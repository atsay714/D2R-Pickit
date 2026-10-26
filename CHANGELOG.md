# Changelog

## 0.10.0 — 2026-10-26

- Add built-in Auto Interact for ordinary loot containers, scenery corpses,
  loose rocks, and weapon/armor racks. Configure categories, locked chests,
  timing and a separate opening range in the Auto Interact tab.
- Set opening range to 1..20 to open eligible containers from that distance
  without moving the character; zero keeps native reach. Collision still
  applies, and normal barrels retain native kick reach.
- Add the Ctrl+P settings window with searchable item checkboxes, bulk actions,
  category switches, live saves, and independent walking/post-cast radii.
- Include post-cast item/gold pickup, retain standalone Teleport compatibility,
  and follow walking/running paths with periodic pickup checks.
- Use the native Windows pointer in an unowned settings window to isolate its
  input queue from the game; cache item rows during routine settings changes.
- Preserve existing item rules and import the separate Auto Interact test
  plugin's preferences once when appropriate. Ship one Pickit DLL and TOML.
- Verified with 26 CTests, seven packaging tests, an independent source-package
  rebuild, and the user's merged-build gameplay retest. Controller navigation,
  TCP/IP and packed-mod object sources remain separately unqualified.

The implementation notes below describe the consolidation and the earlier
unpublished development checkpoints included in this release.

Move the authored Auto Interact 0.3.0 engine into Pickit. One DLL now owns
pickup and ordinary object opening, with one TOML and the existing Auto
Interact tab. Keep independent pickup/opening toggles, extended range 1..20,
all category/key/state/collision checks and performance diagnostics. Remove
the external settings-provider lookup and export; UI callbacks are internal.

Import a legacy sibling Auto Interact TOML once when the new section is absent,
preserving preferences and item rules. Existing object sections win; failed
writes and malformed input leave legacy data intact. Block duplicate opening
when a retired DLL remains. Opening admission failures leave pickup available.
Retain native witnesses, runtime lifecycle, retry/completion tests and notices.
Packages contain only Pickit's DLL/config, with all supporting sources included.

## 0.9.6 (unreleased test build)

Replace the settings software cursor with the native Windows pointer. Keep the
settings window unowned: a cross-thread owner implicitly joins input queues,
sharing game cursor hiding and input stalls with settings. Retain game-relative
positioning and return focus on close, initial dropdown repainting and cached
item rows. Add real Win32 queue/cursor isolation tests with a hidden game cursor,
three open/close cycles, idle visibility, text and resize cursors. These fixtures
do not measure live game mouse latency; user retest remains required.

## 0.9.5 (unreleased test build)

Update the settings cursor immediately on mouse messages, including child and
combo popup messages. Keep the timer for stationary visibility, skip unchanged
positions, and avoid repeated popup repositioning. Reuse item search/catalog
rows for status, tab and Auto Interact changes; preserve item selection/scroll.
Update range help for Auto Interact 0.3.0's extended radius and barrel exception.
Real-control fixtures cover cached 6000-row status updates and existing cursor,
rendering and lifecycle behavior. In-game mouse feel still needs a user retest.

## 0.9.4 (unreleased test build)

Add an optional Auto Interact tab to Ctrl+P, with independent master/category,
locked-chest, range and timing controls for Auto Interact 0.2.0+. The active
tab saves only its own plugin; Pickit's master remains independent. Missing or
older Auto Interact stays optional and is explained in the tab. Settings calls
hold a module reference only for the synchronous operation. No gameplay hook
ownership is shared. Includes the 0.9.3 cursor and initial-control paint fixes.


## 0.9.3 (unreleased)

- Keep the settings pointer visible while stationary using a small, owned, click-through cursor surface. It follows the pointer while the settings window is foreground, including over open radius lists, and hides on close, focus loss or leaving the settings area. No game cursor hooks or global ShowCursor count changes.
- Limit tabs to the header area, clip sibling controls, and repaint the full control tree on opening, activation, tab changes, resize and settings refresh. Radius selectors no longer depend on hover to repaint.
- Focus the first General checkbox when opening General instead of the hidden item-search field. Extend real-control tests to Common Controls v6, initial radius values/layout, stationary pointer drawing, click-through behavior and teardown. Live-game verification of these fixes remains pending.

## 0.9.2 (unreleased)

- Follow walking/running on authoritative game updates every 100 ms, using the live walking radius. Previously only movement commands triggered scans, so items encountered along a path could be missed until another command.
- Keep following long paths while the player moves, then retire after a short arrival grace period. Resolve player IDs each update and cancel work on disable, session changes and unload; retain item cooldowns and native collision/placement.
- Walking follow-ups work independently of built-in post-cast pickup and with older standalone Teleport. No additional movement/cast hooks or config keys are introduced.
- Add walking follow-up counters and sampled item-request diagnostics with actual distance/radius. Cover radii 1â€“20, long paths, burst/refusal fairness, live changes and stale callbacks in runtime fixtures. Live-game verification remains pending.

## 0.9.1 (unreleased)

- Prefer Pickit ownership with cooperative Teleport 0.3.5; keep every pickup control in Pickit's menu and leave standalone loaded but inactive.
- Advertise a versioned resource marker readable before DLL initialization so both load orders make the same decision. Keep legacy ownership for unmodified Teleport 0.3.4, gray out delegated controls, and explain how to edit its own config or optionally update for unified Pickit controls. No Teleport update is required.
- Replace the item dropdown with inline checkboxes, mouse/Space toggling, checked counts, and Pick up results / Leave results / Use category defaults actions.
- Bulk changes cover the full displayed search result set, including offscreen rows, and retain a snapshot so a later query cannot redirect them. Save still uses the same TOML and draft/apply workflow.
- Prefer the checkbox window on newer runtimes too; retain the old native panel only as a window-creation fallback.
- Add cooperative-owner, actual checkbox interaction, seven-result bulk/snapshot, and mixed-version compatibility checks. Test build only; live-game qualification remains pending.

## 0.9.0 (unreleased)

- Rebrand the displayed plugin as D2R Pickit while retaining the existing plugin ID, DLL/config names, item rules and V1/V2 bridge.
- Include post-cast scheduling and gold pickup internally, using the existing authoritative player lookup, carry-cap reader and retry guards.
- Keep Teleport Auto Pickup available as a standalone plugin. If its normal DLL is present or loaded, defer cast-hook ownership to it before installation, independent of load order. Existing 0.3.4 remains compatible.
- Add a General settings tab with the master switch, all category switches, walking/post-cast switches, gold/items switches, and separate radius dropdowns (1-20, default 4).
- Save common settings and item rules to the existing TOML and apply them live. Preserve comments, diagnostic/timing settings, loader metadata and failed-write safety; reject external-edit conflicts.
- Prepare validated hooks even when starting disabled, so the master switch can enable pickup without restarting. Diagnostic mode still installs no hooks.
- Disable built-in post-cast controls when a standalone DLL owns that feature; its own config remains authoritative and untouched.
- Add tests for live category/master/radius settings, built-in scheduling/gold, standalone hook ownership in both load orders, legacy bridge distances, and configuration preservation.
- Test build only: not published; live-game UI/focus, mod table/name resolution, controller and TCP/IP qualification remain pending.

## 0.8.1 (unreleased)

- Fix the missing Ctrl+P action on D2RLoader 1.3.1-beta: its Widget service lacks the newer helpers required by 0.8.0.
- Add an owned Windows overlay with a dedicated message thread, a resizable scrollable item list, search, overrides-only filter and a Default/Include/Exclude dropdown.
- Keep SDK catalog/config operations on loader threads and publish only saved item-list changes between pickup scans.
- Retain the native panel for runtimes that expose its optional helpers; fall back to the Windows frontend when those services or panel registration are unavailable.
- Test real hidden Windows controls with 6,000 rows and test hotkey registration against the older Widget service.
- No Hub publication or live-game/controller/TCP/IP qualification yet.

## 0.8.0 (unreleased)

- Add an optional native in-game item picker, opened with Ctrl+P, a rebindable Controls action, or `auto-pickup-settings`.
- Search loaded game/mod item definitions by name or code, with eight reusable rows and Previous/Next pages.
- Add Default/Include/Exclude choices, an overrides-only view, and retention of saved codes absent from the current tables.
- Read compiled item tables through the SDK and verify code columns through semantic lookup; retain copied codes/names rather than native table pointers or numeric item IDs.
- Save only the two item-code lists, preserve other TOML and loader metadata, reject stale external edits, and apply a complete selection between native scans.
- Keep TOML fallback on loaders without the required optional UI services. Embed the panel resource in the DLL.
- Add picker policy/catalog/config tests and loader fixtures for paging, mod names, failed writes, draft discard and lifecycle cleanup.
- No Hub publication or in-game/controller/TCP/IP qualification yet.

## 0.7.2

- Follow up on reports of `plugins/plugin.json` conflicts persisting with the previous packaging fix.
- Omit build-only `plugin.json` and catalog JSON from installation ZIPs; keep both in source packages.
- Prefix every supporting document, dependency license and checksum filename with this plugin's ID, including when importer directories are flattened.
- Move supporting documents outside the runtime `plugins/` tree and validate the exact installation contents and checksums.
- Add packaging regressions that reject the previous generic filenames and model flattened imports.
- Keep pickup behavior, plugin IDs, defaults, configuration keys and gold limits unchanged.

## 0.7.1

- Fix multi-plugin ZIP installation conflicts by placing metadata, docs, licenses and internal checksums in a plugin-specific folder.
- Add an automatic one-second per-player, per-item retry cooldown shared by walking and Teleport Auto Pickup's V1/V2 scans.
- Temporarily skip attempted items that remain on the ground, including refused requests, native exceptions, and reported successes with no removal.
- Keep other eligible items moving through pickup instead of repeatedly selecting an uncollectable nearest item.
- Reset retry state on session changes, local-player readiness, and unload; retain identities only, never item pointers between scans.
- Preserve native stacking, placement and Auto Deposit integration without a blanket inventory-full check.
- Keep selection defaults, custom include/exclude lists, pickup distances and config keys unchanged; add a retry-skips status counter.
- Add tests for retry timing, item/player isolation, bounded-cache pressure, session cleanup and walking/teleport integration.

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
