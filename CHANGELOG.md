# Changelog

## 0.7.0

- Restore optional `[item_codes].include` and `.exclude` arrays with empty defaults.
- Allow exact item codes outside the built-in categories; exclusions override both categories and inclusions.
- Apply the same custom selection to movement pickup and Teleport Auto Pickup's V1/V2 scan APIs.
- Accept multiline lists, comments, single or double quotes, and trailing commas; reject malformed codes and duplicate settings.
- Keep existing configs valid, the default categories unchanged, and gold handled by the existing gold collector.
- Document item-code lookup, examples and upgrades, and rebuild native, Reimagined-layout and source packages.

## 0.6.1

- Set displayed plugin author and Windows file company to atsay714.
- Establish an independent repository, portable build, self-contained source packages and release documentation.
- Synchronize Windows file version metadata with the plugin version.
- Preserve the 0.6.0 selection and native pickup behavior.

## 0.6.0

- Add a V2 scan API accepting the teleport plugin's per-call distance; retain V1 compatibility.
- Keep movement pickup distance fixed at 4.

## 0.5.0

- Limit selection to nine item switches, including Reimagined loose gems and stacks.
- Enable gems and runes by default; leave ordinary keys and potions off.

## 0.4.0

- Use a generic plugin identity and support global or mod scope.
- Remove belt routing, destination overrides and arbitrary pickup behavior controls.
