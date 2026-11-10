#pragma once
#include "policy.hpp"

namespace RuffnecKk::AutoPickup {
inline void PatchScalar(std::string& text, std::string_view section, std::string_view key, std::string_view replacement, bool array = false) {
    bool inSection = section.empty(), foundSection = inSection;
    std::size_t insert = text.size();
    for (std::size_t start = 0; start < text.size();) {
        const auto end = text.find('\n', start);
        const auto next = end == std::string::npos ? text.size() : end + 1;
        auto line = Trim(WithoutComment(std::string_view(text).substr(start, next - start)));
        if (!line.empty() && line.back() == '\n') line.remove_suffix(1);
        line = Trim(line);
        if (!line.empty() && line.front() == '[') {
            if (inSection) insert = start;
            inSection = line == "[" + std::string(section) + "]";
            foundSection = foundSection || inSection;
        } else if (inSection) {
            const auto eq = line.find('=');
            if (eq != std::string::npos && Trim(line.substr(0, eq)) == key) {
                const auto value = Trim(line.substr(eq + 1));
                const auto offset = static_cast<std::size_t>(value.data() - text.data());
                auto length = value.size();
                std::string comments;
                if (array && !value.empty() && value.front() == '[') {
                    // Input has passed ParseConfig. Find the real closing bracket,
                    // ignoring brackets in comments, and retain interior comments
                    // above the edited key when collapsing a multiline array.
                    for (auto scan = offset; scan < text.size();) {
                        const auto lineEnd = text.find('\n', scan);
                        const auto spanEnd = lineEnd == std::string::npos ? text.size() : lineEnd;
                        const auto raw = std::string_view(text).substr(scan, spanEnd - scan);
                        const auto part = WithoutComment(raw);
                        const auto bracket = part.find(']');
                        if (bracket != std::string_view::npos) {
                            length = scan + bracket + 1 - offset;
                            break;
                        }
                        if (part.size() < raw.size()) {
                            comments += raw.substr(part.size());
                            comments += '\n';
                        }
                        scan = spanEnd + 1;
                    }
                }
                text.replace(offset, length, replacement);
                if (!comments.empty()) text.insert(start, comments);
                return;
            }
        }
        start = next;
    }
    const auto newline = text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
    std::string extra;
    if (insert && text[insert - 1] != '\n') extra += newline;
    if (!foundSection) extra += std::string(newline) + "[" + std::string(section) + "]" + newline;
    extra += std::string(key) + " = " + std::string(replacement) + newline;
    text.insert(insert, extra);
}
}
