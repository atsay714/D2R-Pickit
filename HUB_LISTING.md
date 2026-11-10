# Prepared listing update for 0.11.0

Prepared 2026-11-10 from the current public listing. This copy has not been applied to the live Hub listing.

## Summary

Auto-pickup loot, open containers, and route potions to your chosen belt columns.

## Description

Automatically pick up the loot you want, open nearby containers, and keep your potions in the right belt columns.

**Pickit**

[![Pickit demonstration](https://s15.gifyu.com/images/bWxG9.gif)](https://gifyu.com/image/bWxG9)

Pick up gems, runes, and your selected items while moving or after teleporting and casting. Teleport onto nearby loot instead of clicking every item. Pickup after teleporting is now built in.

No more arthritis.

**Auto Interact**

[![Auto Interact demonstration](https://s15.gifyu.com/images/bWvOe.gif)](https://gifyu.com/image/bWvOe)

Automatically open nearby chests, hidden stashes, hollow logs, baskets, jars, ordinary urns, crates, barrels, and more. Searchable corpses, loose rocks, and weapon and armor racks are supported too.

Choose which objects to open in **Ctrl+P → Auto Interact**. Objects open when you stop moving or casting.

[![Imgur](https://i.imgur.com/gnsOcZp.png)](https://imgur.com/gnsOcZp)

Auto Interact has its own distance setting, separate from item pickup. Choose **Normal reach** for the game's usual interaction distance, or **1–20** to open eligible objects without moving your character. Walls still block interaction, and normal barrels require close range.

Personal stashes, Evil Urns, exploding barrels, and quest objects stay manual. Opening locked chests is optional and off by default.

**New in 0.11.0: Potion routing**

Send healing, mana, and rejuvenation potions to your chosen belt columns. For example, fill columns 1 and 2 with healing potions, column 3 with mana, and column 4 with rejuvenation.

Open **Ctrl+P → Potion routing** and enable **Use custom potion routing**. Choose columns in your preferred order and set **Inventory overflow** independently for each family.

* Existing columns of the same potion family fill before a new empty allowed column is started. Your equipped belt's height is respected.
* With overflow enabled, potions that cannot fit in an allowed column can go to inventory. Select no columns to send that family directly to inventory.
* With overflow disabled and no available allowed column, the potion stays on the ground.

Custom routing is **off by default**. Choose which potions to collect in General or Item picker first. Your Include/Exclude rules still apply, and manual pickups keep normal placement. **Apply & Save** applies routing, General, and item-selection changes live. Auto Interact saves independently.

Thanks to [RuffDood / RuffnecKk](https://github.com/RuffDood) for contributing this feature in [PR #3](https://github.com/atsay714/D2R-Pickit/pull/3). Stack Manager is optional and is not required or bundled.

**Your loot, your settings**

* **Ctrl+P settings:** change categories, distances, and individual item rules from the menu.
* **Searchable item picker:** find items by name or code, then check what you want to collect.
* **Independent controls:** turn item pickup and Auto Interact on or off separately.
* **Ready out of the box:** gems and runes enabled by default.
* **Reimagined support:** includes its gems, gem stacks, and rune stacks.
* **Custom item lists:** include or exclude exact item codes, including items outside the built-in categories.
* **Your exclusions win:** excluded items stay behind even when their category is enabled.
* **Native pickup rules:** the game decides eligibility, inventory capacity, and stacking. Optional potion routing changes where automatic potion pickups go.

Probably also pair with an auto-deposit plugin, or else your inventory will fill up fast.

**Custom item example**

You can also edit `auto-pickup-anything.toml`. For example, collect jewels and small charms while leaving El runes behind:

```toml
[item_codes]
include = ["jew", "cm1"]
exclude = ["r01"]
```

Item-code rules apply to every quality of that item type. Restart after editing the configuration file manually.

**Upgrading**

Close the game, replace the Pickit DLL, and keep your existing `auto-pickup-anything.toml`. Potion routing stays off until you enable it. To enable Auto Interact after upgrading from an older release, turn it on in its settings tab, save, and restart.

Auto Interact is included in Pickit; no separate Auto Interact plugin is needed. Pickup supports vanilla and Reimagined. Auto Interact is currently verified on Reimagined.

Potion routing has offline test coverage. Live gameplay, full-inventory acceptance, save/reload, multiplayer, and active Stack Manager qualification remain pending. Requires D2RLoader and a compatible Windows x64 D2R runtime; native checks target D2R 3.3.0 build 93847. TCP/IP automatic opening is unsupported.

---

# Published listing snapshot

Published listing: https://d2rloader.net/hub/plugins/auto-pickup-anything

Snapshot saved: 2026-11-10

Listing last updated: 2026-10-30T22:35:32.330Z

Type: plugin

D2RLoader version: 1.3.1

Tags: Quality of life, Single player

## Summary

Auto-pickup gems, runes, and custom items by item code.

## Description

Automatically pick up the loot you want. Now includes **Auto Interact** to open nearby containers for you, too.

**Pickit**

[![Pickit demonstration](https://s15.gifyu.com/images/bWxG9.gif)](https://gifyu.com/image/bWxG9)

Pick up gems, runes, and your selected items while moving or after teleporting and casting. Teleport onto nearby loot instead of clicking every item. Pickup after teleporting is now built in.

No more arthritis.

**✨✨New: Auto Interact ✨✨**

[![Auto Interact demonstration](https://s15.gifyu.com/images/bWvOe.gif)](https://gifyu.com/image/bWvOe)

Automatically open nearby chests, hidden stashes, hollow logs, baskets, jars, ordinary urns, crates, barrels, and more. Searchable corpses, loose rocks, and weapon and armor racks are supported too.

Choose which objects to open in **Ctrl+P → Auto Interact**. Objects open when you stop moving or casting.

[![Imgur](https://i.imgur.com/gnsOcZp.png)](https://imgur.com/gnsOcZp)

Auto Interact has its own distance setting, separate from item pickup. Choose **Normal reach** for the game's usual interaction distance, or **1–20** to open eligible objects without moving your character. Walls still block interaction, and normal barrels require close range.

Personal stashes, Evil Urns, exploding barrels, and quest objects stay manual. Opening locked chests is optional and off by default.

**Your loot, your settings**

* **Ctrl+P settings:** change categories, distances, and individual item rules from the menu.
* **Searchable item picker:** find items by name or code, then check what you want to collect.
* **Independent controls:** turn item pickup and Auto Interact on or off separately.
* **Ready out of the box:** gems and runes enabled by default.
* **Reimagined support:** includes its gems, gem stacks, and rune stacks.
* **Custom item lists:** include or exclude exact item codes, including items outside the built-in categories.
* **Your exclusions win:** excluded items stay behind even when their category is enabled.
* **Normal pickup rules:** the game handles eligibility, stacking, and inventory placement.

Probably also pair with an auto-deposit plugin, or else your inventory will fill up fast.

**Custom item example**

You can also edit `auto-pickup-anything.toml`. For example, collect jewels and small charms while leaving El runes behind:

```toml
[item_codes]
include = ["jew", "cm1"]
exclude = ["r01"]
```

Item-code rules apply to every quality of that item type. Restart after editing the configuration file manually.

**Upgrading**

Keep your existing configuration. To enable Auto Interact after upgrading from an older release, turn it on in its settings tab, save, and restart.

Auto Interact is included in Pickit; no separate Auto Interact plugin is needed. Pickup supports vanilla and Reimagined. Auto Interact is currently verified on Reimagined.
