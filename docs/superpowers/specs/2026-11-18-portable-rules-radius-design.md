# Portable pickup rules and extended pickup radius

The user authorized both suggestions after feasibility research: raise pickup
distance above 20 (up to 9999), and import/export rules across reinstalls and
config schema additions/removals. Work stays on a feature branch; no release or
gameplay certification is implied.

## Approaches and decisions

Copying an entire config is already possible but fails the stated upgrade use
case. Importing recognized rule fields into the current config meets it without
overwriting unrelated settings or loader metadata. Use a real TOML parser for
imports so unknown future values, tables and multiline syntax can be skipped
reliably. Vendor pinned MIT toml++ to keep builds self-contained.

Exports contain all nine category booleans and exact Include/Exclude item codes,
plus format/source metadata. They reflect the current draft. Imports accept
portable exports and old full Pickit configs. Recognized present fields replace
their corresponding draft field; missing fields retain the current draft value;
unknown fields are ignored. Invalid recognized fields or malformed TOML refuse
the entire import. Require at least one recognized rule. Item codes remain exact
identities; missing game-table codes are retained. No cross-mod translation.

Windows settings expose Import rules / Export rules and native file pickers.
Imports stage a draft, apply nothing until Apply & Save, and back up the current
full active-scope TOML before saving. A failed read, backup, conflict or write
must not publish imported gameplay policy. Existing normal save behavior remains.
The fallback native panel supports rule file paths through its search field.

Pickup distance accepts integers 1..9999 for walking and built-in post-cast
pickup; default stays 4. Use bounded numeric edit fields in the Windows UI;
native fallback steps through sensible presets including 9999. All shared range
guards use one constant. Do not change Auto Interact's separate 0..20 contract,
old standalone Teleport ownership, room discovery, collision, retry or cadence.
9999 means a distance filter over discovered neighboring rooms, not map-wide loot.

## Verification

Test schema additions/removals, missing/unknown fields, old full configs,
empty lists, malformed/oversized files, unavailable codes, partial failures,
backup failures, save conflicts and draft cancellation. Verify radius boundaries
and propagation through walking/post-cast/native-call doubles and real Win32
controls. Run portable policy/import tests locally and the complete Windows
build/CTest/packaging workflow without publishing. Live extended-distance
gameplay remains separately qualified.
