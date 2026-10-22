// Native contracts adapted from RuffnecKk Auto Pickup; see UPSTREAM.md.
#include <D2RLPlugin/api.h>
#include <Windows.h>
#include "policy.hpp"
#include "compatibility.hpp"
#include "pickup_bridge.hpp"
#include "action_hooks.hpp"
#include <array>
#include <atomic>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>

namespace RuffnecKk::AutoPickup {
namespace {
#include "native.hpp"
// Documented vanilla gold distance from the reference plugin. No config knob.
constexpr std::uint32_t StandardPickupDistance = 4;
const D2RL::PluginContext* Context{};
std::uint8_t* Base{};
Config Settings{};
std::array<TriggerFn, LastTriggerOpcode + 1> OriginalTriggers{};
GetUnitRoomFn GetUnitRoom{};
GetRoomNeighboursFn GetRoomNeighbours{};
UnitFn GetRoomFirstUnit{}, GetNextUnit{};
UnitIntFn UnitType{}, UnitId{}, UnitMode{};
UnitPairFn UnitDistance{};
CollisionFn UnitCollision{};
PickupFn Pickup{};
GetItemCodeFn GetItemCode{};
std::mutex ScanApiMutex;
bool ScanApiReady{};
struct RuntimeMetrics {
    std::atomic<std::uint64_t> actions{}, scans{}, attempts{}, picked{}, refused{}, external{}, exceptions{};
} Metrics;
constexpr D2RL::PluginInfo Info{
    .infoSize = D2RL::PluginInfoSize, .apiVersion = D2RL_PLUGIN_API_VERSION,
    .id = "auto-pickup-anything", .name = "Auto Pickup Anything", .version = "0.7.0",
    .author = "atsay714",
    .description = "Selects items for nearby automatic pickup; normal game stacking and placement.",
    .flags = D2RL::PluginFlags::Shared | D2RL::PluginFlags::NativeHooks,
};
template<class F> auto At(std::uintptr_t rva) noexcept -> F { return reinterpret_cast<F>(Base + rva); }
auto ReadConfiguration() -> bool {
    std::array<char, MaximumConfigBytes> text{}; std::uint32_t required{};
    if (!Context->ReadConfig(text.data(), static_cast<std::uint32_t>(text.size()), &required)) {
        Context->LogError("AutoPickupAnything: configuration could not be read."); return false;
    }
    Config parsed; std::string error;
    if (!ParseConfig(text.data(), parsed, error)) {
        Context->LogError(("AutoPickupAnything: invalid configuration: " + error).c_str()); return false;
    }
    Settings = parsed; return true;
}

void Scan(void* player, std::uint32_t distanceLimit = StandardPickupDistance) noexcept {
    if (!Settings.enabled || Settings.validateOnly || !player) return;
    if (distanceLimit < 1 || distanceLimit > Reimagined::PickupBridge::MaximumPickupDistance) return;
    __try {
        Metrics.scans.fetch_add(1, std::memory_order_relaxed);
        auto* room = GetUnitRoom(player);
        if (!room || UnitType(player) != 0) return;
        void** rooms{}; std::uint32_t count{};
        GetRoomNeighbours(room, &rooms, &count);
        if (!rooms || !count || count > 4096) return;
        void* selected{}; std::int32_t nearest = INT_MAX; std::size_t visited{};
        for (std::uint32_t i = 0; i < count; ++i) for (void* unit = GetRoomFirstUnit(rooms[i]); unit; unit = GetNextUnit(unit)) {
            if (++visited > 262144) return;
            if (UnitType(unit) != ItemType || UnitMode(unit) != GroundMode || !Settings.Selects(GetItemCode(unit))) continue;
            const auto distance = UnitDistance(player, unit);
            if (distance < 0 || static_cast<std::uint32_t>(distance) > distanceLimit
                || distance >= nearest || UnitCollision(player, unit, PickupCollisionMask) != 0) continue;
            selected = unit; nearest = distance;
        }
        if (!selected) return;
        const auto id = UnitId(selected);
        if (id == UINT32_MAX) return;
        Metrics.attempts.fetch_add(1, std::memory_order_relaxed);
        // This is the ordinary native request. No belt hook, destination
        // override, inventory fallback, stack manipulation or deposit call.
        const bool picked = Pickup(player, id, true, distanceLimit, true, false);
        (picked ? Metrics.picked : Metrics.refused).fetch_add(1, std::memory_order_relaxed);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        const auto n = Metrics.exceptions.fetch_add(1, std::memory_order_relaxed) + 1;
        if (Context && (n <= 3 || n % 100 == 0)) Context->LogWarn("AutoPickupAnything: native scan skipped after an exception.");
    }
}
auto ExternalRangeScan(void* player, std::uint32_t distanceLimit) noexcept -> bool {
    std::lock_guard lock(ScanApiMutex);
    if (!ScanApiReady || !Settings.enabled || Settings.validateOnly || !player
        || distanceLimit < 1 || distanceLimit > Reimagined::PickupBridge::MaximumPickupDistance) return false;
    Metrics.external.fetch_add(1, std::memory_order_relaxed);
    Scan(player, distanceLimit); return true;
}
auto ExternalScan(void* player) noexcept -> bool { return ExternalRangeScan(player, StandardPickupDistance); }
constexpr Reimagined::PickupBridge::ApiV1 ScanApi{sizeof(Reimagined::PickupBridge::ApiV1), Reimagined::PickupBridge::Version, ExternalScan};
constexpr Reimagined::PickupBridge::ApiV2 RangeScanApi{sizeof(Reimagined::PickupBridge::ApiV2), Reimagined::PickupBridge::RangeVersion, ExternalRangeScan};
auto __fastcall HookTrigger(void* game, void* player, void* packet, std::int32_t size) noexcept -> std::int64_t {
    const auto opcode = packet && size > 0 ? *static_cast<const std::uint8_t*>(packet) : static_cast<std::uint8_t>(0);
    if (!Reimagined::ActionHooks::IsMovement(opcode) || !OriginalTriggers[opcode]) return 1;
    const auto result = OriginalTriggers[opcode](game, player, packet, size);
    Metrics.actions.fetch_add(1, std::memory_order_relaxed);
    Scan(player); return result;
}
#include "pickup_validation.inc"
auto ValidateRuntime() noexcept -> bool {
    bool valid = true;
    for (std::uint8_t opcode = FirstTriggerOpcode; opcode <= LastTriggerOpcode; ++opcode) {
        const auto expected = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Base + TriggerHandlerRvas[opcode]));
        valid = Context->CheckExpectedBytes(ServerPacketTableRva + opcode * sizeof(std::uintptr_t), &expected, sizeof(expected)) && valid;
    }
    valid = Context->CheckExpectedBytes(GetUnitRoomRva, GetUnitRoomExpected.data(), static_cast<std::uint32_t>(GetUnitRoomExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(GetRoomNeighboursRva, GetRoomNeighboursExpected.data(), static_cast<std::uint32_t>(GetRoomNeighboursExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(GetRoomFirstUnitRva, GetRoomFirstUnitExpected.data(), static_cast<std::uint32_t>(GetRoomFirstUnitExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(GetNextUnitRva, GetNextUnitExpected.data(), static_cast<std::uint32_t>(GetNextUnitExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(UnitTypeRva, UnitTypeExpected.data(), static_cast<std::uint32_t>(UnitTypeExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(UnitIdRva, UnitIdExpected.data(), static_cast<std::uint32_t>(UnitIdExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(UnitModeRva, UnitModeExpected.data(), static_cast<std::uint32_t>(UnitModeExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(UnitDistanceRva, UnitDistanceExpected.data(), static_cast<std::uint32_t>(UnitDistanceExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(UnitCollisionRva, UnitCollisionExpected.data(), static_cast<std::uint32_t>(UnitCollisionExpected.size())) && valid;
    valid = Context->CheckExpectedBytes(GetItemCodeRva, GetItemCodeExpected.data(), static_cast<std::uint32_t>(GetItemCodeExpected.size())) && valid;
    return ValidatePickupEntry() && valid;
}
void ResolveRuntime() noexcept {
    GetUnitRoom = At<GetUnitRoomFn>(GetUnitRoomRva); GetRoomNeighbours = At<GetRoomNeighboursFn>(GetRoomNeighboursRva);
    GetRoomFirstUnit = At<UnitFn>(GetRoomFirstUnitRva); GetNextUnit = At<UnitFn>(GetNextUnitRva);
    UnitType = At<UnitIntFn>(UnitTypeRva); UnitId = At<UnitIntFn>(UnitIdRva); UnitMode = At<UnitIntFn>(UnitModeRva);
    UnitDistance = At<UnitPairFn>(UnitDistanceRva); UnitCollision = At<CollisionFn>(UnitCollisionRva);
    Pickup = At<PickupFn>(PickupRva); GetItemCode = At<GetItemCodeFn>(GetItemCodeRva);
}
auto InstallMutations() noexcept -> bool {
    const auto replacement = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(&HookTrigger));
    for (std::uint8_t opcode = FirstTriggerOpcode; opcode <= LastTriggerOpcode; ++opcode) {
        const auto expected = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Base + TriggerHandlerRvas[opcode]));
        OriginalTriggers[opcode] = reinterpret_cast<TriggerFn>(static_cast<std::uintptr_t>(expected));
        if (!Context->PatchWriteU64(ServerPacketTableRva + opcode * sizeof(std::uintptr_t), &expected, sizeof(expected), replacement)) return false;
    }
    return true;
}

auto Status(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
    if (!command || !command->plugin) return D2RL::ConsoleCommandResult::Failed;
    char message[384]{};
    std::snprintf(message, sizeof(message), "Auto Pickup Anything 0.7.0: enabled=%s; validation-only=%s; scans=%llu; attempts=%llu; picked=%llu; refused=%llu; external-scans=%llu; exceptions=%llu.",
        Settings.enabled ? "true" : "false", Settings.validateOnly ? "true" : "false",
        static_cast<unsigned long long>(Metrics.scans.load()), static_cast<unsigned long long>(Metrics.attempts.load()),
        static_cast<unsigned long long>(Metrics.picked.load()), static_cast<unsigned long long>(Metrics.refused.load()),
        static_cast<unsigned long long>(Metrics.external.load()), static_cast<unsigned long long>(Metrics.exceptions.load()));
    command->plugin->WriteConsoleMessage(message); return D2RL::ConsoleCommandResult::Handled;
}
void ResetRuntime() noexcept {
    std::lock_guard lock(ScanApiMutex); ScanApiReady = false;
    OriginalTriggers.fill(nullptr); Settings = {};
    GetUnitRoom = nullptr; GetRoomNeighbours = nullptr; GetRoomFirstUnit = nullptr; GetNextUnit = nullptr;
    UnitType = nullptr; UnitId = nullptr; UnitMode = nullptr; UnitDistance = nullptr;
    UnitCollision = nullptr; Pickup = nullptr; GetItemCode = nullptr;
    for (auto* n : {&Metrics.actions, &Metrics.scans, &Metrics.attempts, &Metrics.picked, &Metrics.refused, &Metrics.external, &Metrics.exceptions}) n->store(0);
}
}
D2RL_PLUGIN_EXPORT auto AutoPickupAnythingGetScanApi(std::uint32_t version, std::uint32_t size) noexcept -> const void* {
    if (version == Reimagined::PickupBridge::RangeVersion && size >= sizeof(RangeScanApi)) return &RangeScanApi;
    return Reimagined::PickupBridge::SupportedRequest(version, size) ? &ScanApi : nullptr;
}
D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* { return &Info; }
D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
    if (!D2RL::HasContext(context) || context->apiVersion < D2RL_PLUGIN_API_VERSION) return false;
    Context = context; Base = reinterpret_cast<std::uint8_t*>(context->exeBase); ResetRuntime();
    if (!Base || !ReadConfiguration()) return false;
    if (Settings.enabled) {
        if (!ValidateRuntime()) { context->LogError("AutoPickupAnything: compatibility check failed; no hooks installed."); return false; }
        if (Settings.validateOnly) context->LogInfo("AutoPickupAnything: VALIDATION PASSED; diagnostic mode, no hooks installed.");
        else {
            ResolveRuntime(); if (!InstallMutations()) return false;
            { std::lock_guard lock(ScanApiMutex); ScanApiReady = true; }
            context->LogInfo("Auto Pickup Anything 0.7.0 active; item selection only, native stacking and placement.");
        }
    }
    if (!context->RegisterConsoleCommand("auto-pickup", Status, "Show item selection pickup counters.")) context->LogWarn("AutoPickupAnything: status command unavailable.");
    return true;
}
D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept { ResetRuntime(); Base = nullptr; Context = nullptr; }
}
