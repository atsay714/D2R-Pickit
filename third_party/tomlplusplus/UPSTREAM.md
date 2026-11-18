# toml++ provenance

- Repository: https://github.com/marzer/tomlplusplus
- Version: 3.4.0
- Commit: `30172438cee64926dc41fdd9c11fb3ba5b2ba9de`
- License: MIT, Mark Gillard; full upstream notice in LICENSE.
- Unmodified single-header distribution: toml.hpp.
- SHA-256 toml.hpp: `6b5172ad4dd6519aec67b919181fa7a38a2234131e5b2afa232dfe444819783e`
- SHA-256 LICENSE: `529bc3900a9571e49db285b0df432397e70b881cc3bf48de6667ae74ff4b06d8`

Used only to parse portable pickup-rule imports. General-purpose formatters
are disabled with TOML_ENABLE_FORMATTERS=0; the fixed export schema uses basic
string escaping in src/rules_profile.cpp. The active plugin configuration
continues through the existing config parser.
