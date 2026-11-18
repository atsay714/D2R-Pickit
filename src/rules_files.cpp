#include "rules_files.hpp"
#include <atomic>
#include <chrono>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace RuffnecKk::AutoPickup::Picker {
namespace {
namespace fs = std::filesystem;
std::atomic<unsigned long long> Sequence{};
auto Suffix() -> std::string {
    return std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + std::to_string(++Sequence);
}
auto SameFile(const fs::path& a, const fs::path& b) -> bool {
    std::error_code ec;
    if (fs::equivalent(a, b, ec) && !ec) return true;
    const auto first = fs::weakly_canonical(a, ec);
    if (ec) return true; // Refuse when the active path cannot be established.
    const auto second = fs::weakly_canonical(b, ec);
    if (ec) return true;
#ifdef _WIN32
    return CompareStringOrdinal(first.c_str(), -1, second.c_str(), -1, TRUE) == CSTR_EQUAL;
#else
    return first == second;
#endif
}
// Exclusive creation protects prior backups and other processes' temp files.
auto WriteNew(const fs::path& path, std::string_view text, std::string& error) -> bool {
    bool ok{};
#ifdef _WIN32
    const auto handle = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) { error = "Could not create rules file or backup."; return false; }
    DWORD written{};
    ok = WriteFile(handle, text.data(), static_cast<DWORD>(text.size()), &written, nullptr)
        && written == text.size() && FlushFileBuffers(handle);
    if (!CloseHandle(handle)) ok = false;
#else
    const auto handle = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (handle < 0) { error = "Could not create rules file or backup."; return false; }
    const auto written = ::write(handle, text.data(), text.size());
    ok = written >= 0 && static_cast<std::size_t>(written) == text.size() && ::fsync(handle) == 0;
    if (::close(handle) != 0) ok = false;
#endif
    if (!ok) {
        std::error_code ignored; fs::remove(path, ignored);
        error = "Could not finish writing rules file or backup.";
    }
    return ok;
}
}
auto ReadRulesFile(const std::filesystem::path& path, std::string& output, std::string& error) -> bool {
    std::error_code ec;
    if (!fs::is_regular_file(path, ec) || ec) { error = "Could not open rules file."; return false; }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const auto size = file.tellg();
    if (!file || size < 0 || static_cast<unsigned long long>(size) > MaximumRulesBytes) {
        error = "Could not read rules file, or it exceeds 65535 bytes."; return false;
    }
    std::string text(static_cast<std::size_t>(size), '\0');
    file.seekg(0);
    if (!file.read(text.data(), static_cast<std::streamsize>(text.size())) || file.peek() != std::char_traits<char>::eof()) {
        error = "Could not read the complete rules file."; return false;
    }
    output = std::move(text); error.clear(); return true;
}
auto WriteRulesFile(const std::filesystem::path& path, std::string_view text,
        const std::filesystem::path& activeConfig, std::string& error, bool replace) -> bool {
    if (path.empty() || activeConfig.empty() || SameFile(path, activeConfig)) {
        error = "Choose an export file outside the active config. Nothing was exported."; return false;
    }
    if (text.size() > MaximumRulesBytes) { error = "Rules exceed the 65535-byte file limit. Nothing was exported."; return false; }
    auto temporary = path; temporary += ".pickit-tmp-" + Suffix();
    if (!WriteNew(temporary, text, error)) return false;
#ifdef _WIN32
    const bool moved = MoveFileExW(temporary.c_str(), path.c_str(), (replace ? MOVEFILE_REPLACE_EXISTING : 0U) | MOVEFILE_WRITE_THROUGH) != FALSE;
#else
    std::error_code moveError;
    if (replace) fs::rename(temporary, path, moveError);
    else fs::create_hard_link(temporary, path, moveError); // Atomic publish, refusing an existing name.
    const bool moved = !moveError;
    if (moved && !replace) { std::error_code ignored; fs::remove(temporary, ignored); }
#endif
    if (!moved) {
        std::error_code ignored; fs::remove(temporary, ignored);
        error = "Could not replace the export file. Its previous contents were retained."; return false;
    }
    error.clear(); return true;
}
auto BackupRulesConfig(const std::filesystem::path& activeConfig, std::string_view text,
        std::filesystem::path& backup, std::string& error) -> bool {
    if (activeConfig.empty() || text.size() > MaximumRulesBytes) { error = "Cannot back up the active config. Nothing was applied."; return false; }
    auto path = activeConfig; path += ".before-import-" + Suffix() + ".toml";
    if (!WriteNew(path, text, error)) { error = "Could not back up the active config. Nothing was applied."; return false; }
    backup = std::move(path); error.clear(); return true;
}
}
