# Auto Interact native implementation evidence

October 26, 2026. Local test build 0.2.0 for AND-124. Maintainer: atsay714.
This records static analysis, automated fixtures and limited first-test logs.
Version 0.1.0 logged one accepted native chest request; visual opening was not
confirmed and the user reported missed stashes, baskets and logs. Broader gameplay
and real plugin load-order qualification remain pending; TCP/IP is unsupported.

## Capture and admission

Installed D2R.exe reports 3.3.93847; D2RCore reports 1.3.1-beta. The running
executable image was D2RLoader.exe at 0x140000000. The elevated game denied this
session direct read access. The user ran the read-only capture helper three
times, supplying executable, unwind/constant data and the separate static
object-operation array. No game writes, native calls, heap data, saves or config
were involved. Raw captures remain ignored local files under native-capture/.

The reviewed .text range starts at RVA 0x1000, size 30,076,928, SHA256
`9918bc2fb45db42f0b28825a0d4d223b5af6bba6f434d54af203ae67b3e72eb8`.
The immutable per-range hashes are in native-contract.json; native_contract.hpp
contains the corresponding runtime witnesses. Runtime admission checks the
reviewed ranges, all seven operation-table pointers, and the paired object
accessor contract before installing any observer. It does not require a full
process .text hash. Unknown replacements fail admission.

Do not derive the executable base from packet slot 1: Pickit already owns that
slot. The original object packet slot 0x41 provided the independent base witness.
On-disk packed code and old gold-wallet excerpts are unsuitable for reviewing
the native runtime addresses. Unwind records can split one logical function;
the full dispatcher and chest body were reviewed beyond individual fragments.

## Verified ordinary operation path

| Native role | RVA / detail |
| --- | --- |
| Server packet table | 0x1D2A790, 256 pointers |
| Ordinary object packet | slot 0x41 -> 0x4B0F80 |
| Object packet fields | 13 bytes: GUID +1, expected mode +5, cursor GUID +9 |
| Player corpse packet | slot 0x13 -> 0x4B2BE0; excluded, not a generic opener |
| Native object reach | 0x34BC90; actual object shape, separate shape helper 0x34BDE0 |
| Ordinary object dispatcher | 0x593630, logical range through 0x59390B |
| Object operation table | 0x23A51C0, 75 pointers |
| Chest callback | operation 4 -> 0x58E2F0, 1,431 reviewed bytes |
| Scenery corpse / rock callback | operation 14 -> 0x58EED0, 202 bytes |
| Armor rack callback | operation 19 -> 0x58F560, 141 bytes |
| Weapon rack callback | operation 20 -> 0x58F5F0, 141 bytes |
| Basket / urn / jar callback | operation 3 -> 0x58F1D0, 208 bytes |
| Normal barrel callback | operation 5 -> 0x58E890, 395 bytes |
| Jungle stash callback | operation 51 -> 0x5905E0, 249 bytes |

The dispatcher ABI is int32(Game*, Player*, objectGUID, cursorGUID, bool
checkCollision), using Win64 ABI. Auto Interact passes UINT32_MAX cursor GUID
and true collision checking after freshly resolving and rechecking the object.
The dispatcher resolves the object, checks active records, targetability,
state/death/cursor, collision mask 0x804 and a distance backstop. It builds the
ordinary native operation structure and calls the selected callback. It has no
walking fallback. Return zero means accepted; completion is counted only after
observing a subsequent nonzero mode on the same object/class identity.

Version 0.3.0 applies the packet path's shape/reach predicate for range zero
and for the normal-barrel kick callback. Positive ranges 1..20 substitute an
explicit native distance bound, checked in snapshot and immediately before
dispatch. The dispatcher retains its distance-50 backstop and collision path;
it does not initiate walking. Distance also orders eligible candidates. Native callbacks retain responsibility for animation,
lock/key consumption, traps, loot and one-use state. No object mode, drop table,
or inventory contents are written directly.

## Authoritative context and lifecycle

The independent observer at player action dispatcher 0x42D020 forwards all eight
Win64 arguments and the original int32 result once, then validates the observed
Game*/Player* relation. Caller witness 0x4F3BE4 and callee stack accesses establish
the extra arguments. It never attempts opening inside the action callback.
No server packet-table slots are read or written for observer ownership.

GetGameFromUnit is 0x48FF00 (Unit+0xD8). GetUnitRoom at 0x34B440 returns a room,
not a Game*. The observer checks native player type/ID, Game* equality and the
authoritative server-unit lookup before retaining only the session Game* value
and player ID. No native player/object pointer survives a callback.

The first move/cast after LocalPlayerReady acquires the actor. SDK ThreadService
then runs one self-requeueing authoritative callback chain, interval 100 ms by
default. Only neutral/town-neutral modes 1/5, alive state and an empty cursor
qualify. Moving or casting delays opening until idle. Lifecycle generations
invalidate tickets and identity caches on session changes; owner unload discards
queued work and restores the context-owned observer. Unavailable game work
retires the chain and reports the limitation. Remote TCP/IP has no authoritative
local game work; neither host nor client gameplay is qualified for this build.

Traversal is bounded to 64 neighboring rooms, 16,384 visited units and 1,024
objects. Faults, overflow and invalid active rows fail closed. The 1,024-entry
retry cache stores player/object/class IDs, arms before native calls, never
evicts an active cooldown and permits other eligible targets during cooldown.

## Layout and classification corrections

Correction from the 0.2.0 live log: Class at +0 is a linker field (descriptor
type 0x13), and can contain a numeric ID rather than an ASCII string. The old
string assumption was not valid in this runtime. The remaining fields are
Selectable0 at +0x85, IsDoor at +0xFA, SubClass at +0x127 and stride 0x168.
InitFn is a BYTE at +0x15C, PopulateFn at +0x15D, OperateFn at +0x15E and
ClientFn at +0x15F. Older atlas uint16 descriptions overlap adjacent fields and
must not be used. The independent 0x34B7B2 identity witness establishes Unit
type+0, class+4 and mode+0xC. Each object record getter must equal the bounds
checked current-bank row (Game bank+0x106).

Ordinary chests require init 3/57 plus operation 4. Racks require init 0 and
operation 19/20. The shared operation 14 requires reviewed stable unlocalized
Class keys, with init 0/2 for scenery corpses and init 0 for RockC/RockD. All
categories require selectable, non-door records and no subclass bits except
the chest bit. No numeric object row indices or localized names are shipped.
Unknown corpse/rock variants and custom callbacks remain excluded. See policy.hpp
for the explicit supported scenery keys and policy_tests.cpp for exclusions.

The 0.1.0 allowlist omitted ordinary scenery loot that the user expected:
hidden stashes (including Rock/HiddenStash), hollow Log, and several other
operation-14 containers. Version 0.1.1 explicitly admits their reviewed Class
keys under the existing `chests` switch. It also admits exact known basket/jar/
ordinary-urn/box keys on operation 3 (init 2 or specific dungeon init 33), normal
Barrel on operation 5 (init 0), and JunglePillar0..3 on operation 51 (init 2).
SewerChest's init-2/operation-4 pair is a specific chest exception. DungeonRockPile
uses init 33/operation 3 and follows `rocks`. No new numeric row IDs are used.
Personal/town stash operation 32, exploding barrel operation 7, Evil Urn
operation 68 and unknown/custom keys on shared callbacks stay excluded.

Current capture disassembly verifies operation 3's logical end at 0x58F2A0;
the immediately adjacent unwind range starts a separate shrine handler. Do not
merge every contiguous unwind record as one function. Operation 3 changes mode,
schedules end animation, retains its native chance of no drop and invokes traps.
Operation 5 performs its native kick via 0x42D020, changes the barrel mode and
preserves trap/loot events. This is called only for an idle player. Operation 51
schedules normal jungle-stash animation, delayed loot and trap events, recording
the ordinary player ID for the native timer callback. The dispatcher ABI is the
same and all three new table pointers and exact bodies are mandatory witnesses.

ObjectData is Unit+0x10; its interaction low byte is +8. High bit is a lock for
chest operation 4 only; the other newly admitted callbacks do not test it as a lock.
low seven bits hold trap data, conservatively requiring values below 10. The
paired getter/setter validator accepts canonical native code or the fully
witnessed loader split-byte expansion. Reading this low byte is justified by
both that pair and the reviewed chest body. Auto Interact never calls the setter.

Locked chests default off. Opt-in availability follows the chest's class-6
Assassin exemption or a carried key (item type 41, mode 0, inventory page 0,
quantity stat 70 > 0). NativeStatCompat reads quantity without mutation. The
native consumption function at 0x4712A0 is never called as a presence test.
0x36CFE0 is the inventory-page getter, not a socket predicate; unknown hooks to
this key-only helper reject locked-chest activation. Native consumption still
performs the final check and spends the key itself.

## Automated qualification

Nine CTests pass: policy, engine, schedule, settings editing, SDK runtime, native adapter,
actual-DLL activation, vendored native-stat contract and paired object contract.
Seven Python packaging regressions pass. Important boundaries exercised:

- Active-row/class replacement, unreachable/blocked/used/locked objects, missing
  keys, Assassin exception, player busy/dead/cursor, SEH and traversal bounds.
- Mandatory witness/pointer corruption refusal before observer installation,
  short/missing services, partial registration/hook failure and disabled or
  validation-only modes. Actual ABI/default resources and DLL metadata agree.
- Eight-argument observer forwarding and return, forwarding after Stop,
  stale tickets/session reset and unload cleanup.
- Synthetic foreign packet ownership in both states; the sole owned hook is
  0x42D020. This is not an actual Pickit/Teleport/Auto Deposit load-order test.

Independent capture admission reconstructs the captured sections at their RVAs
in a local image, relocates only the seven operation pointers to the test base,
and asks the actual DLL to activate with validate_only=true. It admits the real
captured native code and loader accessor relays without calling native gameplay
functions. This caught the difference between synthetic canonical code and the
installed loader expansion. Raw reconstructed images are ignored build files.

Before release, record real chest/corpse/rock/rack results, keys/traps, busy and
blocked states, delayed loot behavior, retries, session/unload, and actual plugin
coexistence in both load orders. Packages and standalone source builds are
development artifacts until that gameplay evidence is recorded.

## First live smoke report, October 26 at 13:08â€“13:10 Pacific

The loader log reports Auto Interact 0.1.0 active at 13:08:20.357. Its log
contains one accepted request at 13:09:39.369: object 47, class 181 (the active
Objects TXT identifies JungleChest), kind chest, distance 1, result 0. The old
log did not record post-call mode or session completion counters, so visual
success cannot be established from this entry. The user reported stashes,
baskets and logs failing; inspection identified the missing classes/callbacks
above. Version 0.1.1 adds request Class/mode and end-of-session counters.

Pickit 0.9.2 and Teleport 0.3.5 loaded correctly; Pickit's existing master config
was false, last modified 12:52:57, before the test. The user confirmed that was
their mistake and asked to ignore Pickit. No Pickit code/config was changed.
Loader also recorded StatsPool/object-pool assertions during session/engine
shutdown; those are not attributed to Auto Interact by the available stack.

## Independent range and optional settings UI, 0.2.0 (historical)

`interact_range=0` preserves native shape/reach. Values 1..20 additionally cap
the already eligible native distance. Both engine selection and the immediate
native request recheck use this cap; it never substitutes for reach/collision.
Legacy configs without the key retain zero. The key is in embedded defaults
and the shared gameplay match list.

Pickit 0.9.4 supplies an optional third tab. A versioned settings-only ABI
exposes copied values, optimistic revisions and synchronous save results.
Consumer module references exist only during calls; providers serialize calls
with unload and game scans. No native game pointer crosses the interface.
The pinned v3 SDK has no communication-service declarations, so this uses the
existing module-reference compatibility pattern, not the newer SDK lease API.
Auto Interact saves through its own PluginContext config methods. It does not
write Pickit's config and remains independent of Pickit's master switch.

Writes preserve comments, diagnostics and loader data, re-read expected values,
and reject stale drafts or refused writes. Preparing optional key access reads
only code/stat contracts; it runs under the scan lock before writing. Settings
commit only after the loader accepts the write. Live disable keeps the observer
and callback chain available for re-enable, while skipping all native snapshots.
Startup-disabled/validation-only instances save enabling for restart instead.
Tests cover those boundaries on the actual Auto Interact DLL, plus synthetic
provider absence/legacy/unload, the themed three-tab window and native-panel
fallback. These are not live-game UI or opening qualification.

## Extended range and performance, 0.3.0

The user explicitly requested opening at range 20 without moving, superseding
the native-only reach limitation. Positive range replaces only packet shape
reach; dispatcher 0x593630, collision 0x804, native callbacks and all other
checks remain. Normal Barrel/op5 retains shape reach because its callback
initiates a kick action. No coordinate spoof, walking command or mode write is
introduced. Immediate request rechecks use the supplied current config.

Live 0.2.0 request logs had class 104 rendered as `h` and class 181 as byte B5.
Those are numeric linker IDs, so shared Class allowlists could never reliably
match. A startup Objects TXT catalog now maps row ordinals (skipping Expansion)
to Class keys. Every lookup checks active row count and init/op/subclass/
selectable/door fields. Active mod roots and `<mod>.mpq` loose data are checked;
conflicts, BIN shadows without -txt, malformed/missing mod sources fail closed.
Unmodded compiler source uses the context-reported version. No source reading
occurs in a game scan, and no mod-specific numeric object list is shipped.
The actual Reimagined expansion source parses as 587 rows (excluding header).

The native adapter skips expensive reach/collision on used, disabled-category,
untargetable and out-of-range rows, and checks key inventory lazily. The actor
observer performs native validation once per session. Engine retry/completion
lookups use a fixed 2048-slot hash index for 1024 identity entries. Every 100
scans, average/max microseconds cover snapshot, selection and request, excluding
SDK queueing and unrelated plugin/UI work. This is not a game FPS measurement.
