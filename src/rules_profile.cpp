#include "rules_profile.hpp"
#include "item_picker.hpp"
#include <toml.hpp>
#include <sstream>

namespace RuffnecKk::AutoPickup::Picker {
namespace {
auto QuotedMetadata(std::string_view value) -> std::string {
    constexpr char Hex[] = "0123456789ABCDEF";
    std::string text = "\"";
    for (const auto byte : value) {
        const auto c = static_cast<unsigned char>(byte);
        if (c == '"' || c == '\\') { text += '\\'; text += byte; }
        else if (c < 0x20 || c == 0x7f) {
            text += "\\u00"; text += Hex[c >> 4]; text += Hex[c & 0xf];
        } else text += byte; // Preserve the loader's UTF-8 mod name.
    }
    text += '"'; return text;
}
}
auto ImportRules(std::string_view text, const Config& current, Config& output,
        RulesImportReport& report, std::string& error) -> bool {
    if (text.size() > MaximumRulesBytes || text.find('\0') != std::string_view::npos) {
        error = "Rules file is too large or contains a NUL byte. Nothing was imported."; return false;
    }
    try {
        const auto document = toml::parse(text);
        Config next = current;
        RulesImportReport counts;
        for (const auto& [name, node] : document) {
            const auto section = name.str();
            if (section == "pickit_rules") continue; // Informational export metadata.
            if (section != "vanilla" && section != "reimagined" && section != "item_codes") {
                ++counts.ignored; continue;
            }
            const auto* table = node.as_table();
            if (!table) { error = std::string(section) + " must be a TOML table. Nothing was imported."; return false; }
            for (const auto& [key, value] : *table) {
                const auto field = key.str();
                bool* category{};
                if (section == "vanilla") {
                    for (std::size_t i = 0; i < VanillaGroups.size(); ++i)
                        if (field == VanillaGroups[i]) category = &next.vanilla[i];
                } else if (section == "reimagined") {
                    for (std::size_t i = 0; i < ReimaginedGroups.size(); ++i)
                        if (field == ReimaginedGroups[i]) category = &next.reimagined[i];
                }
                const auto fail = [&] {
                    error = "Invalid " + std::string(section) + "." + std::string(field) + ". Nothing was imported."; return false;
                };
                if (category) {
                    const auto* boolean = value.as_boolean();
                    if (!boolean) return fail();
                    *category = boolean->get(); ++counts.fields;
                } else if (section == "item_codes" && (field == "include" || field == "exclude")) {
                    const auto* array = value.as_array();
                    if (!array) return fail();
                    std::vector<std::uint32_t> codes;
                    for (const auto& item : *array) {
                        const auto* string = item.as_string();
                        if (!string) return fail();
                        const auto& code = string->get();
                        if (code.empty() || code.size() > 4) return fail();
                        for (const auto c : code) if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) return fail();
                        codes.push_back(PackItemCode(code));
                    }
                    std::sort(codes.begin(), codes.end());
                    codes.erase(std::unique(codes.begin(), codes.end()), codes.end());
                    (field == "include" ? next.includeCodes : next.excludeCodes) = std::move(codes);
                    ++counts.fields;
                } else ++counts.ignored;
            }
        }
        if (!counts.fields) { error = "No recognized pickup rules found. Nothing was imported."; return false; }
        output = std::move(next); report = counts; error.clear(); return true;
    } catch (const toml::parse_error&) {
        error = "Invalid TOML rules file. Nothing was imported."; return false;
    }
}

auto ExportRules(const Config& config, std::string_view sourceMod) -> std::string {
    // The export schema is fixed. Escape its only free-form value as a TOML
    // basic string; rule values reuse the config's bounded code serialization.
    std::ostringstream text;
    text << "# D2R Pickit portable pickup rules\n# Import into the current config; review, then Apply & Save.\n\n[pickit_rules]\n"
        << "format_version = 1\nsource_mod = " << QuotedMetadata(sourceMod) << "\n\n[vanilla]\n";
    for (std::size_t i = 0; i < VanillaGroups.size(); ++i)
        text << VanillaGroups[i] << " = " << (config.vanilla[i] ? "true" : "false") << '\n';
    text << "\n[reimagined]\n";
    for (std::size_t i = 0; i < ReimaginedGroups.size(); ++i)
        text << ReimaginedGroups[i] << " = " << (config.reimagined[i] ? "true" : "false") << '\n';
    text << "\n[item_codes]\ninclude = " << ArrayText(config.includeCodes) << "\nexclude = " << ArrayText(config.excludeCodes) << '\n';
    return text.str();
}
}
