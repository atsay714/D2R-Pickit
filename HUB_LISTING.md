# Auto Pickup Anything

Published listing: https://d2rloader.net/hub/plugins/auto-pickup-anything

Snapshot saved: 2026-10-21T22:42:58-07:00

Listing last updated: 2026-10-22T05:37:17.623Z

Type: plugin

D2RLoader version: 1.3.1

Tags: Quality of life, Single player

## Summary

Auto-pickup gems, runes, and custom items by item code.

## Description

Automatically pick up gems and runes, with optional support for ordinary keys, health potions, mana potions, and rejuvenation potions.

For maximum convenience, pair with [Auto Pickup after Teleport](https://d2rloader.net/hub/plugins/teleport-auto-pickup) to collect your selected items after teleporting, too.

Teleport onto nearby gems and runes instead of clicking to pick them up.

No more arthritis.

Probably also pair with an auto-deposit plugin, or else your inventory will fill up fast.

* **Ready out of the box:** gems and runes enabled by default.
* **Reimagined support:** includes its gems, gem stacks, and rune stacks.
* **Custom item lists:** include or exclude exact item codes, even items outside the built-in categories.
* **Your exclusions win:** exclusions override both category toggles and explicit inclusions. Both lists default to empty.
* **Normal pickup rules:** the game handles eligibility, stacking, and inventory placement.

**Custom item example**

Automatically collect jewels and small charms while leaving El runes behind:

```toml
[item_codes]
include = ["jew", "cm1"]
exclude = ["r01"]
```

Item codes select every quality of that item type; they do not filter for rare or unique items. The configuration includes examples for jewels, charms, and full rejuvenation potions. Restart the game after editing.

## Releases

### 0.7.0

- Restored custom `[item_codes]` include and exclude lists, both empty by default.
- Added support for exact item codes outside the built-in categories.
- Exclusions override both category toggles and explicit inclusions.
- Custom selections work with movement pickup and Teleport Auto Pickup.
- Added configuration comments with common item codes and ready-to-copy examples.
- Supports multiline lists, comments, single or double quotes, and trailing commas.
- Existing configurations remain valid; default pickup selections are unchanged.
- Updated documentation and rebuilt release packages.

### 0.6.1

Initial Plugin Hub release.

* Configurable pickup of gems, runes, ordinary keys, and three potion families.
* Vanilla and Reimagined gem/rune support.
* Optional integration with Teleport Auto Pickup.
* Automatic compatibility checks and diagnostic logging.
