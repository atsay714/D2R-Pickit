#pragma once
#include "catalog.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
    // Sorted and deduplicated once when the config is parsed.
    std::vector<std::uint32_t> includeCodes, excludeCodes;
    auto Selects(std::uint32_t code) const noexcept -> bool {
        if (code == PackItemCode("gld")) return false;
        if (std::binary_search(excludeCodes.begin(), excludeCodes.end(), code)) return false;
        if (std::binary_search(includeCodes.begin(), includeCodes.end(), code)) return true;
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
    char quote{};
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (quote == '"' && s[i] == '\\' && i + 1 < s.size()) { ++i; continue; }
        if (quote && s[i] == quote) quote = 0;
        else if (!quote && (s[i] == '"' || s[i] == '\'')) quote = s[i];
        else if (s[i] == '#' && !quote) return s.substr(0, i);
    }
    return s;
}
inline auto ParseBool(std::string_view s, bool& result) noexcept -> bool {
    if (s == "true") { result = true; return true; }
    if (s == "false") { result = false; return true; }
    return false;
}
inline auto ParseItemCodes(std::string_view value, std::vector<std::uint32_t>& codes) -> bool {
    if (value.size() < 2 || value.front() != '[' || value.back() != ']') return false;
    value = Trim(value.substr(1, value.size() - 2));
    while (!value.empty()) {
        const auto quote = value.front();
        if (quote != '"' && quote != '\'') return false;
        const auto end = value.find(quote, 1);
        if (end == std::string_view::npos) return false;
        const auto code = value.substr(1, end - 1);
        if (code.empty() || code.size() > 4) return false;
        for (const auto ch : code) if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))) return false;
        codes.push_back(PackItemCode(code));
        value = Trim(value.substr(end + 1));
        if (value.empty()) break;
        if (value.front() != ',') return false;
        value = Trim(value.substr(1));
    }
    std::sort(codes.begin(), codes.end());
    codes.erase(std::unique(codes.begin(), codes.end()), codes.end());
    return true;
}
inline auto ParseConfig(std::string_view input, Config& output, std::string& error) -> bool {
    Config parsed;
    enum class Section { Root, Vanilla, Reimagined, ItemCodes, D2rl } section{Section::Root};
    std::array<bool, 5> sections{};
    std::array<bool, VanillaGroups.size()> vanillaSeen{};
    std::array<bool, ReimaginedGroups.size()> reimaginedSeen{};
    bool enabledSeen{}, validationSeen{}, includeSeen{}, excludeSeen{};
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
            else if (line == "[item_codes]") section = Section::ItemCodes;
            else if (line == "[d2rl]") section = Section::D2rl;
            else return fail("unknown section; use the current plugin config");
            auto& seen = sections[static_cast<std::size_t>(section)];
            if (seen) return fail("duplicate section");
            seen = true; continue;
        }
        if (section == Section::D2rl) continue;
        const auto eq = line.find('=');
        if (eq == std::string_view::npos) return fail("expected key=value");
        const auto key = Trim(line.substr(0, eq)), value = Trim(line.substr(eq + 1));
        if (section == Section::ItemCodes) {
            if (key != "include" && key != "exclude") return fail("unknown item_codes setting; use include or exclude");
            auto& seen = key == "include" ? includeSeen : excludeSeen;
            if (seen) return fail("duplicate setting");
            std::string list(value);
            // TOML arrays may span lines and contain comments or a trailing comma.
            if (!value.empty() && value.front() == '[') while (list.find(']') == std::string::npos) {
                if (start > input.size()) return fail("unterminated item-code list");
                ++lineNumber;
                const auto nextEnd = input.find('\n', start);
                list += ' ';
                list += Trim(WithoutComment(input.substr(start, nextEnd == std::string_view::npos ? input.size() - start : nextEnd - start)));
                start = nextEnd == std::string_view::npos ? input.size() + 1 : nextEnd + 1;
            }
            if (!ParseItemCodes(Trim(list), key == "include" ? parsed.includeCodes : parsed.excludeCodes))
                return fail("expected an array of quoted item codes (1-4 lowercase letters/digits)");
            seen = true; continue;
        }
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
    output = std::move(parsed); error.clear(); return true;
}
}
