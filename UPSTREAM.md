# Upstream provenance

Current project author and maintainer: [atsay714](https://github.com/atsay714).

Native function signatures, pickup calls, action-table hooks, and parts of the
compatibility and selection infrastructure were adapted from RuffnecKk Auto
Pickup 2.0.0 and Potion Auto Pickup in RuffnecKk-D2RLoader-Suite.

- Source: https://github.com/RuffDood/RuffnecKk-D2RLoader-Suite
- Pinned commit: `4d070a0ec7f3ea0d83bfdebc332f071cb0d21e2b`
- License: MIT, copyright 2026 RuffnecKk and contributors; retained in LICENSE.
- The radius reference of 4 comes from Potion Auto Pickup's config comment,
  not an independently verified recovery of vanilla auto-gold's full predicate.

The current plugins separate item selection from post-cast scheduling, use
generic identities and both scopes, check fingerprints before activation,
and cooperate over a versioned synchronous scan bridge. Auto Pickup Anything
has a limited nine-switch catalog. Teleport Auto Pickup schedules landing
scans using game/type/player-ID lookup rather than retaining native pointers.
Current changes and versions are documented in CHANGELOG.md.

The vendored PluginSDK headers/config helpers are from D2RLoader/PluginSDK,
tag `v3`, commit `4933e2c42cb2592958cd0df3b6dc5003102252d1`. Its original
MIT notice and file hashes remain in third_party/PluginSDK. The small SDK
CMake adapter came from the RuffnecKk suite. The SDK is included so this
repository and its source ZIP build independently.
