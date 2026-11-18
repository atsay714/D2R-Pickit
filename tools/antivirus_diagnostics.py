"""Build feature-isolation samples; never publish them as plugin releases.

Run on Windows with --build, or use --prepare-only to inspect source variants.
Every variant is materialized in the same clean path and built by build.cmd.
"""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
OLD = 'v0.11.0'
NEW = 'v0.12.0'
VARIANTS = ('baseline-011', 'radius-core-only', 'import-export-bound20', 'full-012')
COMPONENT_VARIANTS = ('component-control', 'without-parser', 'without-formatter',
                      'without-file-io', 'without-file-dialog')
RADIUS_TESTS = ('policy_tests.cpp', 'runtime_tests.cpp',
                'walking_runtime_tests.cpp', 'post_cast_runtime_tests.cpp')


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


def replace(path, before, after):
    text = path.read_text(encoding='utf-8')
    if before not in text:
        raise ValueError(f'Expected source not found in {path}: {before!r}')
    path.write_text(text.replace(before, after), encoding='utf-8')


def materialize(destination, variant):
    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)
    ref = OLD if variant in ('baseline-011', 'radius-core-only') else NEW
    with tarfile.open(fileobj=io.BytesIO(git('archive', ref))) as archive:
        for member in archive:
            path = destination / member.name
            if not path.resolve().is_relative_to(destination.resolve()):
                raise ValueError(f'Unsafe archive path: {member.name}')
            if member.isdir():
                path.mkdir(parents=True, exist_ok=True)
            elif member.isfile():
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(archive.extractfile(member).read())
            else:
                raise ValueError(f'Unsupported archive member: {member.name}')
    if variant == 'radius-core-only':
        # Deliberately isolate the gameplay/config bound from all new UI code.
        for name in ('src/pickup_bridge.hpp', 'src/policy.hpp'):
            (destination / name).write_bytes(git('show', f'{NEW}:{name}'))
        for name in RADIUS_TESTS:
            (destination / 'tests' / name).write_bytes(git('show', f'{NEW}:tests/{name}'))
    elif variant == 'import-export-bound20':
        # Full new import/export + window lifecycle, with the old range bound.
        replace(destination / 'src/pickup_bridge.hpp',
                'MaximumPickupDistance = 9999;', 'MaximumPickupDistance = 20;')
        replace(destination / 'src/policy.hpp', 'pickup_range must be 1..9999',
                'pickup_range must be 1..20')
        replace(destination / 'src/picker_ui.hpp', 'Change presets up to 9999',
                'Change steps 1-20')
        replace(destination / 'src/picker_ui.hpp',
                'constexpr std::array<std::uint32_t, 22> presets{1,2,3,4,5,6,8,10,12,16,20,30,40,50,75,100,250,500,1000,2500,5000,9999};',
                'constexpr std::array<std::uint32_t, 20> presets{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};')
        replace(destination / 'src/picker_window.cpp', '1-9999 native units', '1-20 native units')
        replace(destination / 'src/picker_window.cpp', 'from 1 to 9999.', 'from 1 to 20.')
        for name in RADIUS_TESTS:
            (destination / 'tests' / name).write_bytes(git('show', f'{OLD}:tests/{name}'))
        # Preserve all new file-dialog and invalid-draft tests at the old bound.
        path = destination / 'tests/picker_window_tests.cpp'
        replace(path, 'L"21"', 'L"19"')
        replace(path, 'Last().value == 21', 'Last().value == 19')
        replace(path, '9999', '20')
        replace(path, '10000', '21')
        replace(destination / 'tests/picker_ui_tests.cpp', 'L"9999"', 'L"20"')
    elif variant in COMPONENT_VARIANTS:
        # These are feature-disabled diagnostic DLLs, never release candidates.
        # Keep this build-only change identical across this round's control.
        replace(destination / 'build.cmd', '-DBUILD_TESTING=ON', '-DBUILD_TESTING=OFF')
        replace(destination / 'build.cmd', 'ctest --test-dir "%~dp0build" --output-on-failure',
                'rem Component isolation: behavioral tests are intentionally not run.')
        if variant == 'without-parser':
            path = destination / 'src/rules_profile.cpp'
            text = path.read_text(encoding='utf-8')
            start = text.index('auto ImportRules(')
            end = text.index('auto ExportRules(')
            stub = ('auto ImportRules(std::string_view, const Config&, Config&, RulesImportReport&, '
                    'std::string& error) -> bool {\n'
                    '    error = "Diagnostic sample: import parser disabled."; return false;\n}\n\n')
            path.write_text(text[:start] + stub + text[end:], encoding='utf-8')
        elif variant == 'without-formatter':
            path = destination / 'src/rules_profile.cpp'
            text = path.read_text(encoding='utf-8')
            start = text.index('auto ExportRules(')
            path.write_text(text[:start] + 'auto ExportRules(const Config&, std::string_view) -> std::string {\n'
                            '    return "# Diagnostic sample: export formatter disabled.\\n";\n}\n}\n', encoding='utf-8')
        elif variant == 'without-file-io':
            (destination / 'src/rules_files.cpp').write_text('''#include "rules_files.hpp"
namespace RuffnecKk::AutoPickup::Picker {
auto ReadRulesFile(const std::filesystem::path&, std::string&, std::string& error) -> bool {
    error = "Diagnostic sample: rule file I/O disabled."; return false;
}
auto WriteRulesFile(const std::filesystem::path&, std::string_view,
    const std::filesystem::path&, std::string& error, bool) -> bool {
    error = "Diagnostic sample: rule file I/O disabled."; return false;
}
auto BackupRulesConfig(const std::filesystem::path&, std::string_view,
    std::filesystem::path&, std::string& error) -> bool {
    error = "Diagnostic sample: rule file I/O disabled."; return false;
}
}
''', encoding='utf-8')
        elif variant == 'without-file-dialog':
            path = destination / 'src/picker_window.cpp'
            text = path.read_text(encoding='utf-8')
            start = text.index('void Window::TransferRules(bool exporting) {')
            end = text.index('DWORD WINAPI Window::ThreadMain(')
            stub = ('void Window::TransferRules(bool) {\n'
                    '    WindowAction event{WindowActionKind::FileError};\n'
                    '    event.generation = activeGeneration;\n'
                    '    event.message = "Diagnostic sample: file dialogs disabled.";\n'
                    '    Emit(std::move(event));\n}\n')
            path.write_text(text[:start] + stub + text[end:], encoding='utf-8')
    return ref


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_changes(destination, ref):
    changes = []
    diff = []
    import difflib
    for item in git('ls-tree', '-r', '--name-only', ref).decode().splitlines():
        path = destination / item
        if not path.is_file():
            continue
        old = git('show', f'{ref}:{item}')
        new = path.read_bytes()
        # git archive applies the repository's CRLF attribute to build.cmd.
        # Do not report that checkout conversion as a source modification.
        if old.replace(b'\r\n', b'\n') != new.replace(b'\r\n', b'\n'):
            changes.append({'path': item, 'sha256': sha(path)})
            diff.extend(difflib.unified_diff(old.decode().splitlines(True),
                                           new.decode().splitlines(True),
                                           fromfile=f'a/{item}', tofile=f'b/{item}'))
    return changes, ''.join(diff)


def run(variant, destination, output, build):
    ref = materialize(destination, variant)
    changes, diff = source_changes(destination, ref)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'source.patch').write_text(diff, encoding='utf-8')
    metadata = {'variant': variant, 'base_ref': ref,
                'base_commit': git('rev-parse', ref + '^{commit}').decode().strip(),
                'changed_files': changes, 'diagnostic_only': True,
                'source_path': str(destination), 'run_id': os.getenv('GITHUB_RUN_ID'),
                'note': ('Not a release. Component isolation builds deliberately disable functionality and behavioral tests.'
                         if variant in COMPONENT_VARIANTS else
                         'Not a release. Radius-core-only retains the 0.11 settings UI; use TOML for extended values.')}
    (output / 'manifest.json').write_text(json.dumps(metadata, indent=2), encoding='utf-8')
    if not build:
        return metadata
    subprocess.run(['cmd.exe', '/d', '/c', 'call build.cmd'], cwd=destination, check=True)
    subprocess.run(['python', '-m', 'unittest', 'discover', '-s', 'tests', '-p', 'test_packaging.py'],
                   cwd=destination, check=True)
    subprocess.run(['python', 'package.py'], cwd=destination, check=True)
    package = next((destination / 'dist').glob('*-native.zip'))
    dll = destination / 'build/d2rl-auto-pickup-anything.dll'
    shutil.copy2(package, output / f'{variant}-native.zip')
    shutil.copy2(dll, output / 'd2rl-auto-pickup-anything.dll')
    ctest = destination / 'build/Testing/Temporary/LastTest.log'
    if ctest.exists():
        shutil.copy2(ctest, output / 'LastTest.log')
    shutil.copy2(destination / 'build/CMakeCache.txt', output / 'CMakeCache.txt')
    with zipfile.ZipFile(package) as archive:
        embedded = archive.read('d2rloader/plugins/d2rl-auto-pickup-anything.dll')
        assert hashlib.sha256(embedded).hexdigest() == sha(dll)
    metadata.update(zip_sha256=sha(package), dll_sha256=sha(dll), dll_size=dll.stat().st_size,
                    ctest_passed=None if variant in COMPONENT_VARIANTS else True,
                    packaging_tests_passed=True)
    (output / 'manifest.json').write_text(json.dumps(metadata, indent=2), encoding='utf-8')
    return metadata


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--variant', choices=(*VARIANTS, *COMPONENT_VARIANTS, 'all', 'components'), default='all')
    parser.add_argument('--build', action='store_true')
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    if args.build and os.name != 'nt':
        parser.error('Build samples on Windows; use --prepare-only elsewhere.')
    if args.build == args.prepare_only:
        parser.error('Choose exactly one of --build or --prepare-only.')
    scratch = ROOT / 'build-diagnostics'
    variants = (VARIANTS if args.variant == 'all' else COMPONENT_VARIANTS
                if args.variant == 'components' else (args.variant,))
    manifests = []
    for variant in variants:
        print(f'::group::{variant}', flush=True)
        manifests.append(run(variant, scratch / 'source', scratch / 'samples' / variant, args.build))
        print('::endgroup::', flush=True)
    (scratch / 'samples/manifest.json').write_text(json.dumps(manifests, indent=2), encoding='utf-8')


if __name__ == '__main__':
    main()
