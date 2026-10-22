"""Build deterministic, checksummed D2RLoader ZIPs from an existing Release DLL."""
from pathlib import Path, PurePosixPath
import argparse
import hashlib
import json
import re
import tomllib
import zipfile

ROOT = Path(__file__).resolve().parent


def digest(data):
    return hashlib.sha256(data).hexdigest()


def write_zip(path, files, checksums_path='SHA256SUMS.txt'):
    files = dict(files)
    assert checksums_path not in files
    files[checksums_path] = ''.join(f'{digest(data)}  {name}\n' for name, data in sorted(files.items())).encode()
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(files.items()):
            entry = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED
            entry.external_attr = 0o644 << 16
            archive.writestr(entry, data)
    with zipfile.ZipFile(path) as archive:
        assert archive.testzip() is None
        assert set(archive.namelist()) == set(files)
        for name, data in files.items():
            assert archive.read(name) == data


def install_support_files(plugin_id):
    # These are text documents, never runtime manifests or JSON patches.
    # Give every leaf filename an owner: some importers flatten ZIP directories.
    files = {f'{plugin_id}.{name}': (ROOT / name).read_bytes() for name in
             ('README.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md', 'UPSTREAM.md', 'CHANGELOG.md')}
    for dependency in ('PluginSDK', 'RuffnecKkNativeStats'):
        license_path = ROOT / 'third_party' / dependency / 'LICENSE'
        if license_path.exists():
            files[f'{plugin_id}.{dependency}.LICENSE.txt'] = license_path.read_bytes()
    return files


def verify_install_layout(path, scope, plugin_id, dll_name, config_name):
    docs = f'{scope}/docs/{plugin_id}/'
    payload = {f'{scope}/plugins/{dll_name}', f'{scope}/config/{config_name}'}
    checksum = f'{docs}{plugin_id}.SHA256SUMS.txt'
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        assert len({name.casefold() for name in names}) == len(names), 'Windows path collision'
        leaves = [PurePosixPath(name).name.casefold() for name in names]
        assert len(set(leaves)) == len(leaves), 'Flattened filename collision'
        assert payload <= set(names), 'Missing DLL or config'
        assert not any(name.casefold().endswith('.json') for name in names), 'Build JSON must stay in source ZIPs'
        required = {docs + name for name in install_support_files(plugin_id)} | {checksum}
        assert set(names) == payload | required, 'Unexpected or missing installation file'
        assert all(PurePosixPath(name).name.startswith(plugin_id + '.') for name in names if name.startswith(docs)), 'Unowned supporting filename'
        expected = ''.join(f'{digest(archive.read(name))}  {name}\n' for name in sorted(names) if name != checksum)
        assert archive.read(checksum) == expected.encode(), 'Invalid internal checksums'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'dist')
    parser.add_argument('--mod-name', default='Reimagined')
    args = parser.parse_args()
    assert re.fullmatch(r'[A-Za-z0-9_-]+', args.mod_name), 'Invalid mod folder name'
    manifest = json.loads((ROOT / 'plugin.json').read_text())
    plugin_id, version = manifest['id'], manifest['version']
    dll_name = f'd2rl-{plugin_id}.dll'
    dll = args.build_dir / dll_name
    if not dll.exists():
        dll = args.build_dir / 'Release' / dll_name
    if not dll.is_file():
        parser.error('Build the Release DLL first; use --build-dir for a different build location.')
    config_name = f'{plugin_id}.toml'
    config = (ROOT / 'config' / config_name).read_bytes()
    settings = tomllib.loads(config.decode())
    assert settings['enabled'] and not settings['validate_only']
    assert settings['item_codes'] == {'include': [], 'exclude': []}, 'Custom lists must ship empty'
    assert {'item_codes.include', 'item_codes.exclude'} <= set(settings['d2rl']['match'])
    # Verify metadata against the actual DLL without activating the plugin.
    import ctypes
    class Info(ctypes.Structure):
        _fields_ = [('size', ctypes.c_uint32), ('api', ctypes.c_uint32), ('id', ctypes.c_char_p),
                    ('name', ctypes.c_char_p), ('version', ctypes.c_char_p), ('author', ctypes.c_char_p),
                    ('description', ctypes.c_char_p), ('flags', ctypes.c_uint32), ('reserved', ctypes.c_uint32 * 4)]
    library = ctypes.CDLL(str(dll.resolve()))
    query = library.D2RLoaderGetPluginInfo
    query.restype = ctypes.POINTER(Info)
    info = query().contents
    assert (info.id.decode(), info.name.decode(), info.version.decode(), info.author.decode()) == (
        plugin_id, manifest['name'], version, manifest['author'])
    assert info.api == 3 and info.flags == 14
    args.output_dir.mkdir(parents=True, exist_ok=True)
    common = install_support_files(plugin_id)
    outputs = []
    for variant, scope in [('native', 'd2rloader'), ('reimagined', f'mods/{args.mod_name}/d2rloader')]:
        path = args.output_dir / f'{plugin_id}-v{version}-{variant}.zip'
        # Preserve normal manual-extraction paths, and also avoid collisions
        # if an importer discards the supporting documents' directories.
        docs = f'{scope}/docs/{plugin_id}'
        write_zip(path, {**{f'{docs}/{name}': data for name, data in common.items()},
                        f'{scope}/plugins/{dll_name}': dll.read_bytes(),
                        f'{scope}/config/{config_name}': config},
                  checksums_path=f'{docs}/{plugin_id}.SHA256SUMS.txt')
        verify_install_layout(path, scope, plugin_id, dll_name, config_name)
        outputs.append(path)
    source = {}
    for name in ('CMakeLists.txt', 'README.md', 'BUILDING.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md',
                 'UPSTREAM.md', 'CHANGELOG.md', 'RELEASING.md', 'plugin.json', 'build.cmd', 'package.py', '.gitignore', '.gitattributes'):
        source[name] = (ROOT / name).read_bytes()
    for name in ('prepare.py', 'catalog.json', 'HUB_LISTING.md'):
        if (ROOT / name).exists():
            source[name] = (ROOT / name).read_bytes()
    for folder in ('src', 'config', 'tests', 'third_party', '.github'):
        for file in (ROOT / folder).rglob('*'):
            if file.is_file() and '__pycache__' not in file.parts:
                source[file.relative_to(ROOT).as_posix()] = file.read_bytes()
    path = args.output_dir / f'{plugin_id}-v{version}-source.zip'
    write_zip(path, source)
    outputs.append(path)
    sums = ''.join(f'{digest(path.read_bytes())}  {path.name}\n' for path in outputs)
    (args.output_dir / 'SHA256SUMS.txt').write_text(sums, encoding='utf-8', newline='\n')
    for path in outputs:
        print(path)


if __name__ == '__main__':
    main()
