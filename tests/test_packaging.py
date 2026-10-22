"""Installation package regressions; does not load DLLs or launch the game."""
import importlib.util
from pathlib import Path, PurePosixPath
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('plugin_packager', ROOT / 'package.py')
packager = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packager)
PLUGIN = __import__('json').loads((ROOT / 'plugin.json').read_text())['id']


def flat_paths(names):
    # Conservative import model: every file is mapped by leaf name only.
    return {'plugins/' + PurePosixPath(name).name.casefold() for name in names}


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / 'plugin.zip'
        self.scope = 'd2rloader'
        self.dll = f'd2rl-{PLUGIN}.dll'
        self.config = f'{PLUGIN}.toml'
        self.docs = f'{self.scope}/docs/{PLUGIN}/'
        self.checksum = f'{self.docs}{PLUGIN}.SHA256SUMS.txt'
        self.files = {
            f'{self.scope}/plugins/{self.dll}': b'dll fixture',
            f'{self.scope}/config/{self.config}': b'enabled = true\n',
            **{self.docs + name: data for name, data in packager.install_support_files(PLUGIN).items()},
        }

    def write(self, files=None, checksum=None):
        packager.write_zip(self.path, self.files if files is None else files,
                           checksums_path=self.checksum if checksum is None else checksum)

    def verify(self):
        packager.verify_install_layout(self.path, self.scope, PLUGIN, self.dll, self.config)

    def test_complete_package_and_each_dependency_license(self):
        self.write()
        self.verify()
        with zipfile.ZipFile(self.path) as archive:
            self.assertEqual(archive.read(self.docs + PLUGIN + '.LICENSE'), (ROOT / 'LICENSE').read_bytes())
            for dependency in ('PluginSDK', 'RuffnecKkNativeStats'):
                path = ROOT / 'third_party' / dependency / 'LICENSE'
                if path.exists():
                    self.assertEqual(archive.read(self.docs + PLUGIN + '.' + dependency + '.LICENSE.txt'), path.read_bytes())
            self.assertFalse(any(name.lower().endswith('.json') for name in archive.namelist()))

    def test_directory_namespace_alone_does_not_protect_flat_import(self):
        old_a = ['d2rloader/plugins/auto-pickup-anything.docs/plugin.json']
        old_b = ['d2rloader/plugins/teleport-auto-pickup.docs/plugin.json']
        self.assertFalse(set(old_a) & set(old_b))
        self.assertEqual(flat_paths(old_a) & flat_paths(old_b), {'plugins/plugin.json'})
        # All new leaf names have their owner's prefix, including dependency
        # licenses and checksums, so this same flattening model cannot collide.
        for other in ('auto-pickup-anything', 'teleport-auto-pickup', 'unrelated-plugin'):
            if other == PLUGIN:
                continue
            others = [f'd2rl-{other}.dll', f'{other}.toml',
                      *packager.install_support_files(other), f'{other}.SHA256SUMS.txt']
            ours = [*self.files, self.checksum]
            self.assertFalse(flat_paths(ours) & flat_paths(others))

    def test_reject_previous_layout(self):
        old_docs = f'{self.scope}/plugins/{PLUGIN}.docs/'
        files = {f'{self.scope}/plugins/{self.dll}': b'dll fixture',
                 f'{self.scope}/config/{self.config}': b'enabled = true\n',
                 old_docs + 'plugin.json': b'{}', old_docs + 'README.md': b'docs'}
        self.write(files, old_docs + 'SHA256SUMS.txt')
        with self.assertRaises(AssertionError):
            self.verify()

    def test_reject_generic_manifest_even_if_other_files_are_correct(self):
        for name in ('plugin.json', 'catalog.json', 'PLUGIN.JSON'):
            with self.subTest(name=name):
                self.write({**self.files, self.docs + name: b'{}'})
                with self.assertRaisesRegex(AssertionError, 'Build JSON'):
                    self.verify()

    def test_reject_leaf_collision_and_case_collision(self):
        for name in ('elsewhere/' + self.dll, f'{self.scope}/plugins/' + self.dll.upper()):
            with self.subTest(name=name):
                self.write({**self.files, name: b'collision'})
                with self.assertRaisesRegex(AssertionError, 'collision'):
                    self.verify()

    def test_reject_missing_license_and_payload(self):
        for name in (self.docs + PLUGIN + '.LICENSE', f'{self.scope}/plugins/{self.dll}',
                     f'{self.scope}/config/{self.config}'):
            with self.subTest(name=name):
                self.write({key: value for key, value in self.files.items() if key != name})
                with self.assertRaises(AssertionError):
                    self.verify()

    def test_reject_changed_content_with_stale_checksums(self):
        self.write()
        with zipfile.ZipFile(self.path) as archive:
            contents = {name: archive.read(name) for name in archive.namelist()}
        contents[f'{self.scope}/plugins/{self.dll}'] = b'modified DLL'
        with zipfile.ZipFile(self.path, 'w') as archive:
            for name, data in contents.items():
                archive.writestr(name, data)
        with self.assertRaisesRegex(AssertionError, 'Invalid internal checksums'):
            self.verify()


if __name__ == '__main__':
    unittest.main()
