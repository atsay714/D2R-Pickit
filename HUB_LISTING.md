# D2R Pickit - Auto Pickup Anything

Published listing: https://d2rloader.net/hub/plugins/auto-pickup-anything

Snapshot saved: 2026-10-26T22:48:59.267Z

Listing last updated: October 26, 2026 (as displayed)

Type: plugin

D2RLoader version: 1.3.1

Tags: Quality of life, Single player

## Summary

Auto-pickup gems, runes, and custom items by item code.

## Description

[![bWxG9.gif](https://s15.gifyu.com/images/bWxG9.gif)](https://gifyu.com/image/bWxG9)


[![bWvOe.gif](https://s15.gifyu.com/images/bWvOe.gif)](https://gifyu.com/image/bWvOe)


Automatically pick up gems and runes, with optional support for ordinary keys, health potions, mana potions, and rejuvenation potions.


For maximum convenience, pair with [Auto Pickup after Teleport](https://d2rloader.net/hub/plugins/teleport-auto-pickup) to collect your selected items after teleporting, without having to take an extra walk/run step.


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

### 0.7.2

Follow-up packaging hotfix for reports that the previous releases still conflicted on plugins/plugin.json.

Remove build-only plugin.json and catalog JSON from installation ZIPs. The source ZIP retains the build files.
Give every supporting document, dependency license, and checksum file a plugin-specific filename, so the two packages remain distinct even when an importer flattens directories.
Move supporting documents outside the runtime plugins/ tree. The DLL and TOML keep their normal installation paths.
Add packaging regressions for generic JSON, flattened filename conflicts, missing files/licenses, and stale checksums.
Preserve pickup behavior, the shared per-item retry cooldown, Auto Deposit integration, custom item lists, plugin identity, defaults, and configuration keys.

Use alongside Teleport Auto Pickup 0.3.4. Import the new packages in place of the old package entries; an importer may retain cached contents from an earlier import.

Validation: all five runtime test suites and seven packaging tests pass. The actual native and Reimagined ZIPs have no conflicting paths or filenames under a simulated flattened import. The reporter's installer has not yet been identified, so testing with that installer is still needed.

Restart the game after updating. Existing configurations remain valid.

Three older releases are collapsed on the live page.
