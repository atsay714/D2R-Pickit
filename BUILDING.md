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

For a different build or output directory, pass `--build-dir` or `--output-dir`.
`--mod-name` changes the mod folder in the mod-layout ZIP; the default is
`Reimagined`. Choose one installation scope per plugin.

## Release maintenance

Keep `plugin.json`, the CMake project version, `PluginInfo::version`, Windows
VERSIONINFO, config heading and changelog in sync. Keep the plugin ID stable:
D2RLoader records gameplay plugin identities in character environment history.
Do not store publishing credentials in this repository.

See [RELEASING.md](RELEASING.md) for the D2RLoader Hub workflow, repository
secret and Hub project-ID setup. Manual workflow runs build and test only;
publishing a GitHub release triggers the Hub upload.
