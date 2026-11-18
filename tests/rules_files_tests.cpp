#include "rules_files.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace RuffnecKk::AutoPickup::Picker;
namespace fs = std::filesystem;
namespace {
int failures{};
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " #x "\n"; ++failures; } } while (false)
auto Read(const fs::path& path) -> std::string { std::ifstream file(path, std::ios::binary); return {std::istreambuf_iterator<char>(file), {}}; }
}
int main() {
    const auto root = fs::temp_directory_path() / ("pickit-files-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directory(root);
    const auto active = root / "active.toml", preset = root / fs::path(L"pickup-\u898f\u5247.toml");
    const std::string old = "# full active config\nenabled=false\n[item_codes]\ninclude=['jew']\n";
    { std::ofstream file(active); file << old; }
    std::string error, text = "unchanged"; fs::path backup;
    CHECK(WriteRulesFile(preset, "[item_codes]\ninclude=[]\n", active, error));
    CHECK(ReadRulesFile(preset, text, error) && text == "[item_codes]\ninclude=[]\n");
    CHECK(WriteRulesFile(preset, "[vanilla]\nrunes=true\n", active, error));
    CHECK(Read(preset) == "[vanilla]\nrunes=true\n");
    CHECK(!WriteRulesFile(active, "bad", active, error) && Read(active) == old);
    CHECK(!WriteRulesFile(root / "." / "active.toml", "bad", active, error) && Read(active) == old);
    std::error_code ec;
    const auto alias = root / "alias.toml";
    fs::create_hard_link(active, alias, ec);
    if (!ec) CHECK(!WriteRulesFile(alias, "bad", active, error) && Read(active) == old);
    CHECK(BackupRulesConfig(active, old, backup, error) && backup != active && Read(backup) == old);
    fs::path second;
    CHECK(BackupRulesConfig(active, "new full config", second, error) && second != backup);
    CHECK(Read(backup) == old && Read(second) == "new full config" && Read(active) == old);
    CHECK(!BackupRulesConfig(root / "missing" / "active.toml", old, second, error));
    CHECK(!WriteRulesFile(root / "missing" / "preset.toml", "test", active, error));
    CHECK(!WriteRulesFile(preset, std::string(MaximumRulesBytes + 1, 'x'), active, error));
    CHECK(Read(preset) == "[vanilla]\nrunes=true\n");
    { std::ofstream file(root / "huge.toml", std::ios::binary); file << std::string(MaximumRulesBytes + 1, 'x'); }
    text = "unchanged";
    CHECK(!ReadRulesFile(root / "huge.toml", text, error) && text == "unchanged");
    CHECK(!ReadRulesFile(root / "missing.toml", text, error) && text == "unchanged");
    CHECK(!ReadRulesFile(root, text, error));
    CHECK(!WriteRulesFile(root, "bad", active, error));
    for (const auto& entry : fs::directory_iterator(root)) CHECK(entry.path().filename().string().find("pickit-tmp") == std::string::npos);
    fs::remove_all(root);
    std::cout << "Rules file I/O and backups; failures: " << failures << '\n';
    return failures ? 1 : 0;
}
