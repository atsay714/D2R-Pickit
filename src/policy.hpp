#pragma once
#include "catalog.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace RuffnecKk::AutoPickup {
constexpr auto PackItemCode(std::string_view code) noexcept -> std::uint32_t {
    std::uint32_t packed = 0x20202020;
    for (std::size_t i = 0; i < code.size() && i < 4; ++i) {
        const auto shift = static_cast<std::uint32_t>(i * 8);
        packed = (packed & ~(0xffU << shift)) | (static_cast<std::uint32_t>(static_cast<unsigned char>(code[i])) << shift);
    }
    return packed;
}
struct Config {
    bool enabled{true}, validateOnly{};
    std::array<bool, VanillaGroups.size()> vanilla{true, true};
    std::array<bool, ReimaginedGroups.size()> reimagined{true, true, true};
    constexpr auto Selects(std::uint32_t code) const noexcept -> bool {
        if (code == PackItemCode("gld")) return false;
        for (const auto& row : Catalog) if (PackItemCode(row.code) == code)
            return row.reimagined ? reimagined[row.group] : vanilla[row.group];
        return false;
    }
};
inline auto Trim(std::string_view s) noexcept -> std::string_view {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}
inline auto WithoutComment(std::string_view s) noexcept -> std::string_view {
    bool quoted{};
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '"') quoted = !quoted;
        if (s[i] == '#' && !quoted) return s.substr(0, i);
    }
    return s;
}
inline auto ParseBool(std::string_view s, bool& result) noexcept -> bool {
    if (s == "true") { result = true; return true; }
    if (s == "false") { result = false; return true; }
    return false;
}
inline auto ParseConfig(std::string_view input, Config& output, std::string& error) -> bool {
    Config parsed;
    enum class Section { Root, Vanilla, Reimagined, D2rl } section{Section::Root};
    std::array<bool, 4> sections{};
    std::array<bool, VanillaGroups.size()> vanillaSeen{};
    std::array<bool, ReimaginedGroups.size()> reimaginedSeen{};
    bool enabledSeen{}, validationSeen{};
    std::size_t lineNumber{};
    auto fail = [&](std::string_view message) { error = "line " + std::to_string(lineNumber) + ": " + std::string(message); return false; };
    for (std::size_t start = 0; start <= input.size();) {
        ++lineNumber;
        const auto end = input.find('\n', start);
        auto line = Trim(WithoutComment(input.substr(start, end == std::string_view::npos ? input.size() - start : end - start)));
        start = end == std::string_view::npos ? input.size() + 1 : end + 1;
        if (line.empty()) continue;
        if (line.front() == '[') {
            if (line == "[vanilla]") section = Section::Vanilla;
            else if (line == "[reimagined]") section = Section::Reimagined;
            else if (line == "[d2rl]") section = Section::D2rl;
            else return fail("unknown/old section; use the current vanilla/reimagined config");
            auto& seen = sections[static_cast<std::size_t>(section)];
            if (seen) return fail("duplicate section");
            seen = true; continue;
        }
        if (section == Section::D2rl) continue;
        const auto eq = line.find('=');
        if (eq == std::string_view::npos) return fail("expected key=value");
        const auto key = Trim(line.substr(0, eq)), value = Trim(line.substr(eq + 1));
        bool* seen{}; bool valid{};
        if (section == Section::Root) {
            if (key == "enabled") { seen = &enabledSeen; valid = ParseBool(value, parsed.enabled); }
            else if (key == "validate_only") { seen = &validationSeen; valid = ParseBool(value, parsed.validateOnly); }
            else return fail("unknown setting; this plugin configures item selection only");
        } else if (section == Section::Vanilla) {
            for (std::size_t i = 0; i < VanillaGroups.size(); ++i) if (key == VanillaGroups[i]) { seen = &vanillaSeen[i]; valid = ParseBool(value, parsed.vanilla[i]); break; }
        } else if (section == Section::Reimagined) {
            for (std::size_t i = 0; i < ReimaginedGroups.size(); ++i) if (key == ReimaginedGroups[i]) { seen = &reimaginedSeen[i]; valid = ParseBool(value, parsed.reimagined[i]); break; }
        }
        if (!seen) return fail("unknown item group");
        if (*seen) return fail("duplicate setting");
        if (!valid) return fail("invalid value");
        *seen = true;
    }
    output = parsed; error.clear(); return true;
}
}
