# Building and packaging

Maintainer: [atsay714](https://github.com/atsay714).

## Requirements

- Windows x64.
- Visual Studio 2022 or newer, or Build Tools, with Desktop development with C++ and a Windows SDK.
- CMake 3.28+ and Ninja. The Visual Studio bundled tools work.
- Python 3.11+ for ZIP packaging and optional catalog regeneration.

The pinned D2RLoader SDK is included in `third_party/PluginSDK`. A clean checkout
builds without downloading dependencies or referring to another project.

## Build and test

Run `build.cmd` from a command prompt. It locates Visual Studio when needed,
builds a Release DLL, and runs CTest. From an x64 Native Tools prompt, the
equivalent commands are:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests use injected native functions and loader fixtures. They do not launch
D2R or call game addresses. Live-game behavior and multiplayer need separate
qualification; passing tests is not a claim that every D2R build is supported.

The automated suites cover configuration/selection, DLL and bridge ABI, activation and
session subscriptions, collector behavior, and retry timing. Collector tests
exercise the walking hook and both teleport bridge versions, including a
refused nearest item followed by a different successful pickup.
Picker suites also cover mod item identities, semantic table-column witnesses,
large catalogs, preservation of manual config edits, failed writes, paging,
unsaved drafts and partial UI registration cleanup. These fixtures validate
the plugin's control flow; they do not render D2R's native widgets. The Windows
frontend test creates real hidden controls, scrolls 6,000 rows, changes the
selection, types a search query, and checks action delivery and teardown.
It uses Common Controls v6 like the D2R host, checks initial radius values and
nonoverlapping tab layout before hover, and renders the click-through pointer
twice without mouse events. The pointer popup must be destroyed on teardown.
Compatibility fixtures verify that the older 56-byte Widget service still
registers the picker hotkey and console command through the Windows frontend.

The activation suites cover both load orders: Pickit owns all slots with a cooperative peer; a legacy peer retains the old split. Checkbox tests exercise mouse, Space, bulk-result snapshots and 6,000-row scrolling.
To additionally exercise the actual existing standalone DLL with injected loader
services and a synthetic admitted image (no game addresses are called):

```bat
build\activation_tests.exe config\auto-pickup-anything.toml C:\path\to\d2rl-teleport-auto-pickup.dll build\d2rl-auto-pickup-anything.dll
```

The standalone source is retained in its own repository. The embedded module in
`src/post_cast/` derives from the authored 0.3.4 implementation; source packages
include it and the pinned native-stat adapter without a sibling-repository dependency.

## Package

```bat
python package.py
```

This verifies the built DLL's identity, author, version and flags, then creates
global/native, Reimagined-layout, and self-contained source ZIPs in `dist/`.
Each ZIP includes SHA-256 checksums, documentation, and dependency license
notices. `dist/SHA256SUMS.txt` hashes the ZIPs themselves. The source ZIP can
build and test independently. Packages never include developer build output,
logs, character saves, credentials or publishing tokens.

Each installation ZIP stores text documents under
`<scope>/docs/<plugin-id>/`. Every document and checksum filename is prefixed
with the plugin ID, including dependency licenses. `plugin.json` and the
catalog are build/source files and never enter installation ZIPs. This avoids
generic JSON being interpreted as a runtime patch. The DLL and TOML keep their
normal paths. Verification rejects generic/unexpected files, case-insensitive
path and leaf-filename collisions, missing licenses, and invalid checksums.
The source ZIP retains its normal source-tree layout and root checksum manifest.

Run `python -m unittest discover -s tests -p test_packaging.py` for packaging
regressions. These include a simulated importer that flattens paths: this is
an additional compatibility check, not a claim to reproduce every manager.

For a different build or output directory, pass `--build-dir` or `--output-dir`.
`--mod-name` changes the mod folder in the mod-layout ZIP; the default is
`Reimagined`. Choose one installation scope per plugin.

## Release maintenance

The walking runtime fixture covers a single movement command followed by
position changes on game updates, radius boundaries 1–20, long paths, refusal
cooldowns, live edits, legacy Teleport ownership, and stale-session callbacks.
It models native pickup acceptance; it does not replace live-game testing.

Keep `plugin.json`, the CMake project version, `PluginInfo::version`, Windows
VERSIONINFO, config heading and changelog in sync. Keep the plugin ID stable:
D2RLoader records gameplay plugin identities in character environment history.
Do not store publishing credentials in this repository.

See [RELEASING.md](RELEASING.md) for the D2RLoader Hub workflow, repository
secret and Hub project-ID setup. Manual runs with no release tag build and test only;
publishing a GitHub release triggers the Hub upload.


## Built-in Auto Interact

The authored engine is under `src/auto_interact/`. It compiles into the Pickit
DLL without any sibling repository or runtime plugin dependency. The internal
settings callbacks persist `[auto_interact]` in the Pickit TOML; old cross-DLL
settings exports are absent. The source ZIP includes native research and pinned
object-contract/stat dependencies with their notices.

Tests cover source Class catalogs, explicit/native range, collision/keys,
identity retries, actor/session lifetime, internal UI saves, migration,
legacy-DLL exclusion, and actual combined-DLL load/cleanup. The combined test
uses injected mutation services and never calls native gameplay functions.
For additional capture admission, pass a reconstructed local image:

```bat
build\combined_activation_tests.exe build\d2rl-auto-pickup-anything.dll config\auto-pickup-anything.toml C:\path\capture-admission-image.bin
```

Code bytes come from the independent capture. The fixture normalizes table
relocations and models Auto Deposit's SDK ownership report for its captured
hook. The external D2RCore GetUnitStat thunk is absent from the image, so that
one helper is normalized to the adapter's separately tested native branch. This proves admission/control flow, not gameplay or real load order.
To prepare a migration file offline without modifying either input:

```bat
build\migrate_interact_config.exe old-pickit.toml auto-interact.toml new-pickit.toml
```

The helper refuses an existing output file. Review it before replacing the
active config. Runtime first-launch migration uses the same tested function.
