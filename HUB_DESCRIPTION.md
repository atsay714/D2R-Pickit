Automatically pick up the loot you want, open nearby containers, and keep your potions in the right belt columns.

**Pickit**

[![Pickit demonstration](https://s15.gifyu.com/images/bWxG9.gif)](https://gifyu.com/image/bWxG9)

Pick up gems, runes, and your selected items while moving or after teleporting and casting. Teleport onto nearby loot instead of clicking every item. Pickup after teleporting is now built in.

No more arthritis.

**New in 0.12.0: Save your pickup rules**

Keep your selections when you reinstall your mods. Open **Ctrl+P → Item picker** and choose **Export rules...** to save your category switches and Include/Exclude lists to a TOML file.

To restore them, choose **Import rules...**, review the selections, then **Apply & Save**. You can import a previous rules export or an old full `auto-pickup-anything.toml`. Pickit copies the rules it recognizes into your current configuration, skips unrelated or retired settings, and keeps your current selections for any rule fields missing from the import.

Exports include the rules currently shown in the menu, including unsaved changes. Before applying an import, Pickit backs up your full current config beside the active TOML. Keep your exported rules outside the mod folder so they survive a reinstall. Pickup distances, potion routing, and Auto Interact settings are kept as they are.

**New in 0.12.0: Pickup radius up to 9999**

Set separate walking and post-cast pickup distances from **1–9999** in **Ctrl+P → General**. The default is still **4**.

The radius applies to loot in the neighboring game rooms Pickit can search. Walls, inventory space, and the game's normal pickup rules still apply, so 9999 does not mean map-wide pickup. Auto Interact keeps its separate **Normal reach / 1–20** setting.

**Auto Interact**

[![Auto Interact demonstration](https://s15.gifyu.com/images/bWvOe.gif)](https://gifyu.com/image/bWvOe)

Automatically open nearby chests, hidden stashes, hollow logs, baskets, jars, ordinary urns, crates, barrels, and more. Searchable corpses, loose rocks, and weapon and armor racks are supported too.

Choose which objects to open in **Ctrl+P → Auto Interact**. Objects open when you stop moving or casting.

[![Imgur](https://i.imgur.com/gnsOcZp.png)](https://imgur.com/gnsOcZp)

Auto Interact has its own distance setting, separate from item pickup. Choose **Normal reach** for the game's usual interaction distance, or **1–20** to open eligible objects without moving your character. Walls still block interaction, and normal barrels require close range.

Personal stashes, Evil Urns, exploding barrels, and quest objects stay manual. Opening locked chests is optional and off by default.

**Potion routing**

Send healing, mana, and rejuvenation potions to your chosen belt columns. For example, fill columns 1 and 2 with healing potions, column 3 with mana, and column 4 with rejuvenation.

Open **Ctrl+P → Potion routing** and enable **Use custom potion routing**. Choose columns in your preferred order and set **Inventory overflow** independently for each family.

* Existing columns of the same potion family fill before a new empty allowed column is started. Your equipped belt's height is respected.
* With overflow enabled, potions that cannot fit in an allowed column can go to inventory. Select no columns to send that family directly to inventory.
* With overflow disabled and no available allowed column, the potion stays on the ground.

Custom routing is **off by default**. Choose which potions to collect in General or Item picker first. Your Include/Exclude rules still apply, and manual pickups keep normal placement. **Apply & Save** applies routing, General, and item-selection changes live. Auto Interact saves independently.

Thanks to [RuffDood / RuffnecKk](https://github.com/RuffDood) for contributing this feature in [PR #3](https://github.com/atsay714/D2R-Pickit/pull/3). Stack Manager is optional and is not required or bundled.

**Your loot, your settings**

* **Ctrl+P settings:** change categories, distances, and individual item rules from the menu.
* **Portable pickup rules:** export your selections and restore them after a reinstall, even when the config format has changed.
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

Close the game, replace the Pickit DLL, and keep your existing `auto-pickup-anything.toml`. Your rules and enabled options are retained. Export your pickup rules before reinstalling your mods, then import them into the new setup.

Custom potion routing is off by default until you enable it. To enable Auto Interact after upgrading from an older release, turn it on in its settings tab, save, and restart.

Auto Interact is included in Pickit; no separate Auto Interact plugin is needed. Pickup supports vanilla and Reimagined. Auto Interact is currently verified on Reimagined.

Requires D2RLoader and a compatible Windows x64 D2R runtime; native checks target D2R 3.3.0 build 93847. TCP/IP automatic opening is unsupported.
