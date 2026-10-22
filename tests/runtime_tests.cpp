#include "../src/plugin.cpp"
#include <iostream>
namespace {
int failures{}, requests{};
std::uint32_t ExpectedDistance = 4;
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " #x "\n"; ++failures; } } while (false)
struct Unit { std::uint32_t type{}, id{}, code{}; std::int32_t distance{}; bool blocked{}; } Player, Item;
int Room{};
void* Rooms[]{&Room};
bool NativeAccepts{true};
void* __fastcall RoomOf(void*) { return &Room; }
void __fastcall Neighbours(void*, void*** out, std::uint32_t* count) { *out = Rooms; *count = 1; }
void* __fastcall First(void*) { return &Item; }
void* __fastcall Next(void*) { return nullptr; }
std::uint32_t __fastcall Type(void* unit) { return static_cast<Unit*>(unit)->type; }
std::uint32_t __fastcall Id(void* unit) { return static_cast<Unit*>(unit)->id; }
std::uint32_t __fastcall Code(void* unit) { return static_cast<Unit*>(unit)->code; }
std::uint32_t __fastcall Mode(void*) { return 3; }
std::int32_t __fastcall Distance(void*, void* item) { return static_cast<Unit*>(item)->distance; }
std::int32_t __fastcall Collision(void*, void* item, std::uint32_t) { return static_cast<Unit*>(item)->blocked ? 1 : 0; }
bool __fastcall Request(void* player, std::uint32_t id, bool ordinary, std::uint32_t distance, bool normal, bool extra) {
    CHECK(player == &Player && id == Item.id && ordinary && normal && !extra && distance == ExpectedDistance);
    ++requests; return NativeAccepts;
}
}
int main() {
    using namespace RuffnecKk::AutoPickup;
    GetUnitRoom = RoomOf; GetRoomNeighbours = Neighbours; GetRoomFirstUnit = First; GetNextUnit = Next;
    UnitType = Type; UnitId = Id; UnitMode = Mode; GetItemCode = Code;
    UnitDistance = Distance; UnitCollision = Collision; Pickup = Request;
    Player = {0, 1}; Item = {4, 2, PackItemCode("r01")};
    Settings = {}; Scan(&Player); CHECK(requests == 1 && Metrics.picked.load() == 1);
    Item.code = PackItemCode("ka3"); Scan(&Player); CHECK(requests == 1);
    Item.code = PackItemCode("hp5"); Scan(&Player); CHECK(requests == 1);
    std::string error;
    CHECK(ParseConfig("[vanilla]\nhealth_potions = true", Settings, error));
    Scan(&Player); CHECK(requests == 2); // Same ordinary native request as a rune.
    Item.blocked = true; Scan(&Player); CHECK(requests == 2);
    Item.blocked = false; Item.distance = 5; Scan(&Player); CHECK(requests == 2);
    Item.distance = 4; NativeAccepts = false; Scan(&Player);
    CHECK(requests == 3 && Metrics.refused.load() == 1); // No forced placement or fallback retry.
    NativeAccepts = true; ScanApiReady = true; Item.distance = 8;
    CHECK(ExternalScan(&Player)); CHECK(requests == 3); // V1 stays at normal movement radius.
    ExpectedDistance = 8;
    CHECK(ExternalRangeScan(&Player, 8)); CHECK(requests == 4);
    Item.distance = 9; CHECK(ExternalRangeScan(&Player, 8)); CHECK(requests == 4);
    CHECK(!ExternalRangeScan(&Player, 0) && !ExternalRangeScan(&Player, 21));
    CHECK(requests == 4);
    ExpectedDistance = 4; Item.distance = 4; Scan(&Player); CHECK(requests == 5); // External call never changes movement range.
    CHECK(ParseConfig("[item_codes]\ninclude = [\"jew\"]\nexclude = [\"r01\"]", Settings, error));
    Item.code = PackItemCode("jew"); Scan(&Player); CHECK(requests == 6);
    Item.code = PackItemCode("r01"); Scan(&Player); CHECK(requests == 6);
    CHECK(ExternalScan(&Player)); CHECK(requests == 6); // The V1 bridge honors exclusions too.
    Item.code = PackItemCode("jew"); CHECK(ExternalScan(&Player)); CHECK(requests == 7);
    Item.distance = 8; ExpectedDistance = 8;
    CHECK(ExternalRangeScan(&Player, 8)); CHECK(requests == 8); // Teleport's V2 bridge uses the custom policy.
    CHECK(ParseConfig("[item_codes]\ninclude = [\"jew\"]\nexclude = [\"jew\"]", Settings, error));
    CHECK(ExternalRangeScan(&Player, 8)); CHECK(requests == 8);
    Item.distance = 4; ExpectedDistance = 4; Scan(&Player); CHECK(requests == 8);
    CHECK(ParseConfig("[item_codes]\ninclude = [\"jew\"]", Settings, error));
    NativeAccepts = false; Scan(&Player); CHECK(requests == 9 && Metrics.refused.load() == 2);
    NativeAccepts = true; Settings.enabled = false; Scan(&Player); CHECK(requests == 9);
    std::cout << "Selected items use the same native request; unselected/mod items, collisions and distance rejected; native refusal preserved; failures: " << failures << '\n';
    return failures ? 1 : 0;
}
