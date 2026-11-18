#include "rules_profile.hpp"
#include "item_picker.hpp"
#include <toml.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace RuffnecKk::AutoPickup;
using namespace RuffnecKk::AutoPickup::Picker;
namespace {
int failures{};
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " #x "\n"; ++failures; } } while (false)
}
int main(int argc, char** argv) {
    Config current;
    current.enabled = false; current.walkingRange = 9999; current.teleportRange = 42;
    current.vanilla.fill(false); current.reimagined.fill(false);
    current.includeCodes = {PackItemCode("jew")}; current.excludeCodes = {PackItemCode("r01")};
    current.potionRouting.enabled = true; current.interact.interactRange = 20;
    Config imported; RulesImportReport report; std::string error;
    CHECK(ImportRules("[vanilla]\nrunes=true\n[item_codes]\ninclude=['cm1', 'zz99', 'cm1']\n", current, imported, report, error));
    CHECK(imported.vanilla[1] && !imported.vanilla[0]);
    CHECK(imported.includeCodes == (std::vector{PackItemCode("cm1"), PackItemCode("zz99")}));
    CHECK(imported.excludeCodes == current.excludeCodes); // Missing field is retained.
    CHECK(!imported.enabled && imported.walkingRange == 9999 && imported.teleportRange == 42);
    CHECK(imported.potionRouting == current.potionRouting && imported.interact.interactRange == 20);
    CHECK(report.fields == 2);
    CHECK(ImportRules("[item_codes]\ninclude=[]\nexclude=[]\n", current, imported, report, error));
    CHECK(imported.includeCodes.empty() && imported.excludeCodes.empty());
    const auto future = R"(
# A full config from a version with added, removed and unrelated properties.
enabled = true
removed_old_option = 3
[walking]
pickup_range = 100000
[vanilla]
gems = true
removed_category = [1, 2, 3]
[future]
description = """unknown text with [vanilla] and runes=false
and a fake [item_codes] section"""
values = [{ x = "#not a comment" }, { y = [1, 2] }]
[[future.nested]]
include = ["ignored"]
[item_codes]
include = [
  "zz99", # Code absent from the current game tables survives.
]
)";
    CHECK(ImportRules(future, current, imported, report, error));
    CHECK(imported.vanilla[0] && !imported.vanilla[1]);
    CHECK(imported.includeCodes == std::vector{PackItemCode("zz99")} && imported.excludeCodes == current.excludeCodes);
    CHECK(imported.walkingRange == 9999 && !imported.enabled && report.ignored > 0);
    // Legacy root metadata is irrelevant to rules. BOM/CRLF and quoted TOML keys work.
    CHECK(ImportRules("\xef\xbb\xbf[\"vanilla\"]\r\n'runes'=true\r\n[d2rl]\r\nmatch=['retired']\r\n", current, imported, report, error));
    CHECK(imported.vanilla[1]);
    // Export/import round trip includes categories as well as both arrays.
    current.vanilla[2] = true; current.reimagined[1] = true;
    const auto exported = ExportRules(current, "Mod \"quoted\" \\ name\nsecond line");
    Config defaults;
    CHECK(ImportRules(exported, defaults, imported, report, error));
    CHECK(imported.vanilla == current.vanilla && imported.reimagined == current.reimagined);
    CHECK(SameSelection(imported, current) && report.fields == 11);
    CHECK(imported.enabled && imported.walkingRange == 4); // Export excludes other settings.
    CHECK(exported == ExportRules(current, "Mod \"quoted\" \\ name\nsecond line"));
    // Metadata must remain a single TOML value, even with controls or content
    // that resembles rule sections. The real parser checks the emitted format.
    std::string controls;
    for (char c = 0; c < 32; ++c) controls += c;
    controls += static_cast<char>(127);
    struct MetadataCase { const char* name; std::string mod; };
    for (const auto& sample : {MetadataCase{"empty", {}},
            MetadataCase{"quoted and multiline", "Mod \"quoted\" \\ name\nsecond line"},
            MetadataCase{"section-like text and quote delimiters", "[vanilla]\nrunes=false\n'''\"\"\""},
            MetadataCase{"ASCII controls", controls},
            MetadataCase{"Unicode", "Mod \xe7\xb9\x81\xe4\xb8\xad \xf0\x9f\x98\x80"}}) {
        const auto profile = ExportRules(current, sample.mod);
        try {
            const auto document = toml::parse(profile);
            CHECK(document["pickit_rules"]["format_version"].value_or(0) == 1);
            CHECK(document["pickit_rules"]["source_mod"].value<std::string>() == sample.mod);
            CHECK(ImportRules(profile, defaults, imported, report, error));
            CHECK(SameSelection(imported, current) && report.fields == 11);
        } catch (const toml::parse_error& failure) {
            std::cerr << "Metadata case " << sample.name << ": " << failure << '\n';
            CHECK(false);
        }
    }
    if (argc == 2) {
        std::ifstream file(argv[1]); const std::string text{std::istreambuf_iterator<char>(file), {}};
        CHECK(ImportRules(text, current, imported, report, error));
        CHECK(imported.vanilla == Config{}.vanilla && imported.reimagined == Config{}.reimagined);
    }
    for (const auto bad : {
        "", "[future]\nnew_setting=1", "[vanilla]\nrunes='true'", "[vanilla]\nrunes=1",
        "vanilla=false", "item_codes=[]", "[item_codes]\ninclude=['UPPER']", "[item_codes]\nexclude=['abcde']",
        "[item_codes]\ninclude=['jew', 1]", "[item_codes]\ninclude='jew'", "[item_codes]\ninclude=['']",
        "[item_codes]\ninclude=['jew'", "[vanilla]\nrunes=true\nrunes=false",
        "[vanilla]\ngems=true\nrunes=1", "[item_codes]\nexclude=['bad-code']"}) {
        imported = current; report = {123, 456};
        CHECK(!ImportRules(bad, current, imported, report, error));
        CHECK(SameSettings(imported, current) && !error.empty()); // No partial draft update.
        CHECK(report.fields == 123 && report.ignored == 456);
    }
    CHECK(!ImportRules(std::string(MaximumRulesBytes + 1, ' '), current, imported, report, error));
    const auto nul = std::string("[vanilla]\nrunes=true\n\0ignored", 29);
    CHECK(!ImportRules(nul, current, imported, report, error));
    std::cout << "Portable rule profiles; failures: " << failures << '\n';
    return failures ? 1 : 0;
}
