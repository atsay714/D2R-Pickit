# Auto Pickup Anything

Automatically pick up selected gems and runes in Diablo II: Resurrected.
Optional switches add ordinary keys and health, mana or rejuvenation potions.
The plugin works with vanilla D2R and includes explicit Reimagined item codes.

**Author and maintainer:** [atsay714](https://github.com/atsay714)  
**Version:** 0.6.1 · **License:** MIT · **Runtime:** D2RLoader / Windows x64

## Installation

Requires D2RLoader and a compatible Windows x64 D2R runtime. Download the
matching ZIP from the project's releases or build it using [BUILDING.md](BUILDING.md).
The native fingerprints currently target D2R 3.3.0 build 93847. Other builds
must pass the plugin's automatic compatibility checks; ABI 3 and the required
loader services must be available. The development environment uses
D2RLoader 1.3.1-beta.

1. Close D2R and D2RLoader.
2. Extract **one** package into your D2R installation directory:
   - `native.zip`: global `d2rloader/plugins/` and `d2rloader/config/`.
   - `reimagined.zip`: the same files under `mods/Reimagined/d2rloader/`.
3. Start the game through D2RLoader. No first-run configuration edit is required.

These are native DLL packages, separate from the Reimagined launcher's JSON
User Plugins importer. The plugin supports global or mod scope. Keep only one
active copy and restart after config changes. Existing configs are retained
by the loader; copy new settings deliberately when upgrading.

DLL: `d2rl-auto-pickup-anything.dll`  
Config: `d2rloader/config/auto-pickup-anything.toml` in the selected scope.

## Configuration

```toml
# Auto Pickup Anything 0.6.1
# Active immediately on a compatible D2RLoader runtime. Restart after edits.
enabled = true
# Optional troubleshooting mode; compatibility checks always run.
validate_only = false

[vanilla]
gems = true
runes = true
keys = false
health_potions = false
mana_potions = false
rejuvenation_potions = false

[reimagined]
# Reimagined gems have a different item code
gems = true
gem_stacks = true
rune_stacks = true

[d2rl]
match = ["enabled", "validate_only", "vanilla.gems", "vanilla.runes", "vanilla.keys", "vanilla.health_potions", "vanilla.mana_potions", "vanilla.rejuvenation_potions", "reimagined.gems", "reimagined.gem_stacks", "reimagined.rune_stacks"]
```

| Setting | Includes |
| --- | --- |
| vanilla.gems | All 35 classic gem codes, chipped through perfect, including skulls |
| vanilla.runes | El through Zod |
| vanilla.keys | Ordinary keys; excludes Uber keys |
| vanilla.health_potions / mana_potions | All five standard sizes of the selected family |
| vanilla.rejuvenation_potions | Small and full rejuvenation potions |
| reimagined.gems | Reimagined's eight loose gem codes, including Chaos Onyx |
| reimagined.gem_stacks | Stacked forms of those eight Reimagined gems |
| reimagined.rune_stacks | Reimagined's El through Zod rune stacks |

The Reimagined switches default to true. Their item codes differ from vanilla
and select no unrelated vanilla items. Power orbs, crafting materials, Uber
keys, jewels, charms, equipment and other categories are left for manual
pickup. The [catalog](catalog.json) lists the 130 supported codes. `[d2rl]`
is loader compatibility metadata; retain it when editing item preferences.

## Pickup behavior

Movement actions request at most one selected nearby item per callback. The
adapter uses native distance cost 4 and checks collision. It calls the normal
pickup entry, preserving the game's acceptance, stacking and placement rules.
There is no movement-radius setting, forced placement or inventory sweep.
This is a separate collector; exact vanilla auto-gold selection parity is
unverified.

[Teleport Auto Pickup](https://github.com/atsay714/d2rl-teleport-auto-pickup)
can request the same item selection after casts. Its V2 bridge supplies a
distance for that scan only; it never changes movement pickup's distance.
Either plugin works independently. The V1 bridge remains available at distance 4.

Auto Deposit remains responsible for Reimagined bag/stash routing. Its 0.1.4
gem-bag path can miss delayed or burst pickups; the separate local pending-ID
queue fix addresses that issue. This repo does not contain or replace Auto
Deposit, and no bag transaction logic is embedded here.

Console command: `auto-pickup`. Log: the selected scope's
`d2rloader/logs/auto-pickup-anything.log`.

## Troubleshooting and removal

The plugin's status command and log report activation and compatibility checks.
`validate_only = true` checks compatibility without installing hooks; normal
play uses the shipped default `false`. An unknown or conflicting native
modification refuses activation rather than guessing addresses.

To disable, set `enabled = false` and restart. To remove, close the game and
move its DLL out of the active plugin folder. It creates no custom items or
inventory pages. D2RLoader may show an environment-history warning after a
plugin change; keep plugin IDs stable and back up characters before changing
your setup. No save-history editing is part of installation or removal.

## Releases

See [RELEASING.md](RELEASING.md) for the D2RLoader Hub publishing workflow.
It builds and tests on Windows, then uploads the native package when a GitHub
release is published. Manual Actions runs build and package without publishing.

## Development and credits

See [BUILDING.md](BUILDING.md) for builds, tests and packaging, and
[CHANGELOG.md](CHANGELOG.md) for changes. The maintainer and release author is
[atsay714](https://github.com/atsay714).

This project includes code adapted from RuffnecKk's MIT-licensed Auto Pickup
plugins and the MIT-licensed D2RLoader PluginSDK. Adapted native signatures and
hook patterns are credited in [UPSTREAM.md](UPSTREAM.md). Original notices are
preserved in [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
