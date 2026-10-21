#include "../src/plugin.cpp"
#include "loader_fixture.hpp"
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
    auto* context = LoaderFixture::Setup(text, true);
    CHECK(D2RLoaderLoadPlugin(context));
    CHECK(!Settings.validateOnly && Settings.enabled);
    CHECK(LoaderFixture::Patches == 4 && LoaderFixture::Checks > 10);
    CHECK(LoaderFixture::InlineHooks == 0);
    CHECK(!D2RL::HasFlag(D2RLoaderGetPluginInfo()->flags, D2RL::PluginFlags::ModScopedOnly));
    D2RLoaderUnloadPlugin();
    // Out-of-box activation must not bypass build/signature admission.
    context = LoaderFixture::Setup(text, false);
    CHECK(!D2RLoaderLoadPlugin(context));
    CHECK(LoaderFixture::Patches == 0 && LoaderFixture::InlineHooks == 0);
    D2RLoaderUnloadPlugin();
    auto diagnostic = text;
    const auto p = diagnostic.find("validate_only = false");
    CHECK(p != std::string::npos);
    if (p != std::string::npos) diagnostic.replace(p, 21, "validate_only = true");
    context = LoaderFixture::Setup(diagnostic, true);
    CHECK(D2RLoaderLoadPlugin(context));
    CHECK(Settings.validateOnly && LoaderFixture::Patches == 0 && LoaderFixture::Checks > 10);
    D2RLoaderUnloadPlugin();
    std::cout << "Default config activates unchanged; incompatible native fingerprint blocks patches; diagnostic-only mode remains optional; failures: " << failures << '\n';
    return failures ? 1 : 0;
}
