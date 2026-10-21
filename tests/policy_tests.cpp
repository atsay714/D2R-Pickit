#include "policy.hpp"
#include "compatibility.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
namespace {
int failures{};
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " #x "\n"; ++failures; } } while (false)
}
int main(int argc, char** argv) {
    using namespace RuffnecKk::AutoPickup;
    if (argc != 2) return 2;
    std::ifstream input(argv[1]);
    const std::string text{std::istreambuf_iterator<char>(input), {}};
    Config c; std::string error;
    CHECK(ParseConfig(text, c, error) && c.enabled && !c.validateOnly);
    CHECK(VanillaGroups.size() == 6 && ReimaginedGroups.size() == 3);
    for (const auto& row : Catalog) {
        const bool expected = row.reimagined || VanillaGroups[row.group] == "gems" || VanillaGroups[row.group] == "runes";
        CHECK(c.Selects(PackItemCode(row.code)) == expected);
    }
    for (const auto code : {"ka3", "1ka", "pk1", "dhn", "tes", "toa", "std", "jew", "cm1", "cap", "xa1", "ua1", "rup", "jwp", "1gc", "tsc", "aqv", "gld"}) CHECK(!c.Selects(PackItemCode(code)));
    CHECK(ParseConfig("", c, error) && c.enabled && !c.validateOnly);
    for (const auto code : {"gmm", "gmo", "1am", "1so", "s01", "s33"}) CHECK(c.Selects(PackItemCode(code)));
    CHECK(ParseConfig("[reimagined]\ngems = false", c, error));
    CHECK(!c.Selects(PackItemCode("gmm")) && !c.Selects(PackItemCode("gmo")));
    CHECK(c.Selects(PackItemCode("gpv")) && c.Selects(PackItemCode("1so")));
    CHECK(ParseConfig("[reimagined]\ngem_stacks = false", c, error));
    CHECK(!c.Selects(PackItemCode("1so")) && !c.Selects(PackItemCode("1am")));
    CHECK(c.Selects(PackItemCode("gmo")));
    CHECK(ParseConfig("[reimagined]\nrune_stacks = false", c, error));
    CHECK(!c.Selects(PackItemCode("s01")) && c.Selects(PackItemCode("r01")));
    CHECK(ParseConfig("[vanilla]\ngems = false", c, error));
    CHECK(!c.Selects(PackItemCode("gpv")) && c.Selects(PackItemCode("gmm")));
    for (const auto group : {"keys", "health_potions", "mana_potions", "rejuvenation_potions"}) {
        CHECK(ParseConfig(std::string("[vanilla]\n") + group + " = true", c, error));
        for (const auto& row : Catalog) if (!row.reimagined && VanillaGroups[row.group] == group) CHECK(c.Selects(PackItemCode(row.code)));
    }
    for (const auto bad : {
        "[vanilla]\ngems = true\ngems = false", "[vanilla]\nuber_keys = true",
        "[vanilla]\ncharms = true", "[reimagined]\npower_orbs = true",
        "[reimagined]\nungraded_gems = true", "[reimagined]\nchaos_onyx = true",
        "[item_codes]\ninclude = [\"jew\"]", "pickup_range = 4",
        "scan_every_player_actions = 2", "[health_potions]\nbelt_columns = [1]"}) CHECK(!ParseConfig(bad, c, error));
    CHECK(AcceptPickupEntry(true, false, 0, ""));
    CHECK(AcceptPickupEntry(false, true, 1, "d2rl-auto-deposit"));
    CHECK(!AcceptPickupEntry(false, true, 2, "d2rl-auto-deposit"));
    CHECK(!AcceptPickupEntry(false, true, 1, "unknown"));
    std::cout << "Nine switches, requested defaults, Chaos Onyx grouping, independent vanilla/mod selection and manual-only items; failures: " << failures << '\n';
    return failures ? 1 : 0;
}
