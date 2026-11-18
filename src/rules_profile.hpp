#pragma once
#include "policy.hpp"
#include <cstddef>

namespace RuffnecKk::AutoPickup::Picker {
inline constexpr std::size_t MaximumRulesBytes = 65'535;
struct RulesImportReport { std::size_t fields{}, ignored{}; };
// Only recognized, present rule fields are transferred. Output/report are
// unchanged on failure; unrelated settings always come from the destination.
auto ImportRules(std::string_view text, const Config& current, Config& output,
    RulesImportReport& report, std::string& error) -> bool;
auto ExportRules(const Config& config, std::string_view sourceMod) -> std::string;
}
