#pragma once
#include "interact_settings.hpp"
#include <Windows.h>
#include <string>

namespace Reimagined::InteractSettings {
struct View {
    Values values;
    bool available{}, dirty{};
    std::string status{"Auto Interact settings are unavailable."};
};
class Editor {
public:
    View view;
    void Bind(const Api* value) noexcept { api = value; }
    void Open() {
        view = {};
        Values value;
        if (!Valid(api) || api->read(&value) != Result::Success || !Valid(value)) return;
        original = value; view.values = value; view.available = true;
        view.status = value.state & StartupBlocked ? "Auto Interact could not activate. Check the Pickit log for a retired DLL or compatibility failure. Item pickup remains available."
            : value.state & ValidationOnly ? "Validation mode: opening is disabled. Change validate_only in Pickit's [auto_interact] config and restart."
            : value.state & RestartRequired ? "Auto Interact needs a restart to activate. Saved preferences can still be edited here."
            : "Auto Interact is built into Pickit. Its toggle is independent of item pickup; Apply & Save changes this tab only.";
    }
    void Toggle(std::uint32_t index, bool value) {
        if (!view.available || index >= ToggleLabels.size()) return;
        const auto flag = 1U << index;
        view.values.flags = (view.values.flags & ~flag) | (value ? flag : 0U); Changed();
    }
    void Number(std::uint32_t index, std::uint32_t value) {
        if (!view.available || index > 2) return;
        auto draft = view.values;
        (index == 0 ? draft.range : index == 1 ? draft.intervalMs : draft.retryMs) = value;
        if (Valid(draft)) { view.values = draft; Changed(); }
    }
    void Save() {
        if (!view.available || !view.dirty) return;
        char message[512]{};
        if (!Valid(api)) { view.status = "Auto Interact is unavailable. Nothing was saved."; return; }
        const auto result = api->save(&original, &view.values, message, sizeof(message));
        message[sizeof(message) - 1] = 0;
        if (result == Result::Success || result == Result::RestartRequired) {
            Open();
            if (view.available) view.status = message[0] ? message : "Auto Interact settings saved.";
        } else view.status = message[0] ? message : "Auto Interact settings could not be saved. Close and reopen before retrying.";
    }
private:
    void Changed() { view.dirty = !Same(original, view.values); view.status = view.dirty ? "Unsaved Auto Interact changes" : "No Auto Interact changes"; }
    Values original;
    const Api* api{};
};
}
