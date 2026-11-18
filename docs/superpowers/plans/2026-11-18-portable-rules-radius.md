# Portable Rules and Extended Radius Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Transfer pickup rules across config versions and support pickup radii 1..9999.

**Architecture:** Parse import files with pinned toml++, project recognized rules
onto the current draft, and retain the existing conflict-checked save path.
Win32 handles file selection and bounded file I/O; one shared radius constant
governs configuration, UI and scans.

**Tech Stack:** C++20, Win32, D2RLoader ABI 3, toml++ 3.4.0, CMake/CTest.

**Spec:** `docs/superpowers/specs/2026-11-18-portable-rules-radius-design.md`

## Global Constraints

- Keep default radius 4, allow 1..9999; Auto Interact stays 0..20.
- Import only categories and exact Include/Exclude lists; preserve other settings.
- Ignore unknown fields; missing fields retain destination values; invalid known fields reject atomically.
- Import stages a draft; Apply & Save backs up before replacing active-scope config.
- Retain plugin ID, ABI, scope and native pickup safeguards; no publishing.

## Review Focus

- Future TOML syntax such as nested tables, multiline strings and arrays must not corrupt extraction.
- Missing one of the two lists must retain that destination list; explicit empty arrays clear it.
- Export overwrite and failed import/backup/write must preserve prior files and active policy.
- Pending file dialogs and stale queued actions must be safe during close/unload.
- Radius typing and refresh must not lose drafts, silently clamp invalid input or bypass legacy ownership.

### Task 1: Portable rule model

**Files:** `src/rules_profile.hpp`, `src/rules_profile.cpp`, `tests/rules_profile_tests.cpp`, `third_party/tomlplusplus/`, `CMakeLists.txt`.

**Interfaces:** `ImportRules(text, current, output, report, error) -> bool`;
`ExportRules(config, sourceMod) -> string`. Parser lives in one translation unit.

- [ ] Add tests for old/full/preset files, added/removed fields, partial missing fields, arrays, invalid types, empty/no rules and oversized input. Run to observe missing feature failures.
- [ ] Vendor pinned parser/license and implement atomic extraction plus deterministic export.
- [ ] Run portable tests with clang++; all assertions pass. Register in Windows CTest.

### Task 2: Extended pickup distance

**Files:** `src/pickup_bridge.hpp`, `src/policy.hpp`, `src/picker_ui.hpp`, `src/picker_window.*`, existing policy/runtime/walking/post-cast/window tests.

**Interfaces:** Shared `MaximumPickupDistance = 9999`; numeric edit events emit validated `WindowActionKind::Range` integers.

- [ ] Add range boundary and native propagation tests; observe 21/9999 rejected by the old parser.
- [ ] Replace duplicated caps and dropdowns with shared validation and numeric entry; add fallback presets.
- [ ] Verify 0/10000 rejected, 21/9999 accepted, native calls retain the chosen radius, and Auto Interact remains unchanged.

### Task 3: Rule files and settings integration

**Files:** `src/rules_files.hpp`, `src/rules_files.cpp`, `src/picker_window.*`, `src/picker_ui.hpp`, `src/picker_layout.hpp`, UI/file tests, CMake.

**Interfaces:** Bounded file reads, atomic export, unique backup helper; import/export actions carry selected Unicode paths to the loader UI thread.

- [ ] Write file/UI tests for successful import staging, cancellation, failed I/O, backups, conflicts, save failures, exported drafts, native fallback and Unicode paths.
- [ ] Implement file controls/dialog cancellation, transfer actions, import status, and backup-before-save tracking.
- [ ] Run Windows build/CTest/packaging in build-only mode; fix failures and retain concrete output.

### Task 4: Documentation and review

**Files:** README, BUILDING, CHANGELOG, provenance/notices/package license list and tests.

- [ ] Document upgrades, import/export semantics, backup path and effective-radius limits. Keep release version unchanged with Unreleased notes.
- [ ] Run all appropriate portable and Windows checks; inspect diff and request one fresh whole-branch review.
- [ ] Address material review findings, reverify affected suites, and leave a concrete reviewable branch/PR without merging or publishing.
