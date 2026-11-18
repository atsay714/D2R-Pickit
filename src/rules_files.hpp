#pragma once
#include "rules_profile.hpp"
#include <filesystem>

namespace RuffnecKk::AutoPickup::Picker {
auto ReadRulesFile(const std::filesystem::path& path, std::string& output, std::string& error) -> bool;
// Atomically replace a selected export, but never the active config or an alias.
auto WriteRulesFile(const std::filesystem::path& path, std::string_view text,
    const std::filesystem::path& activeConfig, std::string& error) -> bool;
auto BackupRulesConfig(const std::filesystem::path& activeConfig, std::string_view text,
    std::filesystem::path& backup, std::string& error) -> bool;
}
