# OpenSMACX Recovery Handoff

## Objective

Finish OpenSMACX as a standalone source-owned executable. Local proprietary x86 islands are only a temporary recovery mechanism. Final distributable builds must require no original executable, copied machine code, fixed-address redirects, or proprietary assembly.

## Working Rules

- Continue incrementally and commit each completed recovery batch.
- Before starting another replacement, every replacement must pass behavioral tests, ABI/disassembly checks, Debug and Release builds, metadata regeneration, island removal, staged-hybrid build, and runtime smoke testing.
- Write recovery tests before implementing a function. When possible, run identical fixtures against a locally extracted original implementation and the source replacement, then require equivalent return values, state changes, memory boundaries, and relevant CPU/FPU behavior.
- Use extracted COFF symbols as the differential oracle only for conservative self-contained legacy leaves. Functions with calls, absolute globals, relocations, or process state must be exercised at their original address inside the verified hybrid process.
- Keep all proprietary oracle assembly, objects, fixtures derived from original bytes, and runtime artifacts local and ignored. Commit only source-owned tests and tooling.
- Smoke launches do not exercise uncommon paths such as multiplayer. Add isolated source-level tests.
- Runtime tests use a dedicated marker-protected Wine prefix and stop only that prefix. The scenario runner additionally tracks its per-run random executable alias. Never issue a global Wine shutdown or terminate unrelated processes.
- Keep proprietary runtime and tool data ignored under `.opensmacx/` and `build/`.
- Never commit or distribute generated assembly or object files.
- Keep each eligible legacy island as a separate symbol and section so it can be replaced independently.
- Do not revert unrelated worktree changes.

## Recovery State

- Canonical inventory: 6,000 functions.
- Game functions: 5,627.
- Library functions: 338.
- Thunks: 35.
- Current recovery backlog: 5,125 candidates.
- Current local legacy-island count: 150, reduced from 174.
- Source-recovery `DllMain` redirects: 25. The gameplay gate installs one inactive-pass-through active-turn hook and two call-site hooks; scenario behavior activates only when its environment is configured.
- Runtime redirects are signature-checked, transactional, and rolled back in reverse order.

### Analysis Inputs

- Ghidra executable SHA-256: `01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf`.
- Canonical IDB input SHA-256: `7d1933da68a3326ac97464849a209a5f127606f5bd7a6abfe9985cd3ce044767`.
- Canonical IDB SHA-256: `6ffdcf2d6644f2c1b19c218d3b1b293b4e442d56b8cf1f537b0403608ff866fa`.
- Persistent ignored Ghidra project: `build/ghidra-projects/live-recovery`.

## Completed Work

### Inventory and Hybrid Workflow

- `bb77d47 Add decompilation recovery inventory`
- `d4bb283 Correlate independent recovery analyses`
- `eb7432e Classify remaining executable bindings`
- `c619bf3 Normalize IDA analysis metadata`
- `f186b39 Recover base coordinate lookup`
- `cc47ad1 Add local hybrid image bootstrap`
- `6565518 Add verified hybrid executable workflow`

### Legacy Island Tooling

- `099511b Add local legacy leaf islands`
- The 12 classifier tests pass.
- The Ghidra interior-reference sidecar contains 2,574 references and is hash-bound in `analysis-summary.json`.
- Initial extraction produced 174 islands and 174 COFF globals in `pe-i386` format with no relocations.

### Source Recoveries

- `4d9361e Recover Buffer font and color setters`
- `2073b6e Recover Dialog font and color setters`
- `a6a77cf Recover rectangle and sound helpers`
- `730846d Recover UI location and button helpers`
- `11f32bd Recover Caviar constructors and scaling`
- `bff2dd7 Recover Caviar camera state`
- `051c045 Recover legacy null operation hooks`
- `d169b50 Recover multiplayer process lookup`

Recovered source includes:

- Five `Buffer` font and color setters.
- Four `Dialog` font and color setters.
- `in_box` and `do_sound`.
- `BasePop::set_loc` and `ButtonGroup::add`.
- `CaviarData` and `Caviar` constructors, scaling, camera state, and scaling getter.
- `MainInterface::clear_message`, `desktop_update`, Buffer lifecycle hooks, and `Dialogs::close`.
- `AlphaNet::pid_2_idx` and its fastcall-to-thiscall runtime adapter.

Other completed corrections and checks:

- Fixed a latent `BasePop` layout hole with a `0xC94` placeholder. `BasePop` has verified size `0x3230`.
- Added verified layouts for `CaviarData`, `Caviar`, `ButtonGroup`, `Buffer`, and `Dialog`.
- Caviar scaling uses raw integer transfer to preserve NaN payloads, floating-point exceptions, and `EAX`. Release disassembly contains no unwanted x87 transfer.
- Hybrid staging and launch succeeded for the earlier Buffer and Dialog recovery batches.
- All completed batches passed Debug and Release MinGW builds, metadata regeneration, island removal, and independent review.

### Recovery Verification

- The direct-source `recovery-leaf-tests` harness passes under Wine in Debug and Release. It covers all seven AlphaNet process-ID slots, duplicate and zero IDs, the redirect adapter, and `in_box` edge semantics.
- CTest always registers the Windows behavioral test through `tools/run_windows_test.py`, which auto-detects Wine and uses the build's dedicated owned test prefix.
- The `verify-recovery-abi` target and CTest check pass in Debug and Release. They verify i386 COFF, required symbols, thiscall cleanup, fastcall adapter cleanup, and the gameplay trampoline's overwritten instruction, preserved state, callback stack cleanup, and continuations.
- Regenerated state after AlphaNet is 5,125 priorities and 150 islands.

### Hybrid Runtime Compatibility

- `90a99f8 Use PRACX runtime for hybrid staging`
- The pre-PRACX executable reaches DirectDraw `E_NOTIMPL` on Wine Staging 11.10, then the legacy `DDRAWERROR` path crashes while using popup globals before `BasePop::init_class()`.
- Forcing the bundled native DDrawCompat proxy also fails fast on this Wine version.
- Hybrid staging defaults to the hash-pinned PRACX executable at `.opensmacx/game/terranx.exe` and publishes all 460 expected import redirects.
- The packer labels PRACX `hash_pinned_runtime_build`; all recovery body mappings are `not_analyzed` and unmapped rather than projected from canonical addresses.
- Legacy-island extraction separately remains bound to the independently analyzed pre-PRACX executable and produces 150 islands.
- Always launch through `tools/run_game.py`. On macOS it uses the Wine application bundle and explicitly passes `WINEPREFIX`.
- The PRACX hybrid loader trace reached DirectDraw rendering and loaded `OpenSMACX.dll`, `prax.dll`, and Wine's built-in `DDRAW.dll` without a main-process unhandled exception.
- `tools/smoke_hybrid_game.py` automates that gate, validates module and rendering markers, rejects unhandled exceptions, and stops the dedicated owned test prefix while removing its per-run executable alias.
- `tools/run_gameplay_scenario.py` loads a local ignored save, inspects legal movement candidates, asserts source `go_to` movement-order state, or resolves the order through legacy `action_go_to` before requesting end turn.
- The gameplay runner waits for a terminal JSON result, rejects fatal Wine diagnostics, and stops its dedicated owned prefix while verifying removal of its per-run executable alias. A passing local fixture used turn 12, vehicle 0, `(22,26)` to `(23,27)`; resolution spent 3 movement points, moved the map stack, and cleared the order.
- The gameplay trampoline at the verified `Console::human_turn` seam preserves registers and flags, executes the overwritten store, and selects either the ordinary or early-exit continuation.
- `~/Desktop/backtrace.txt` is manually saved and cannot be used as an automatic crash signal.

## Next Steps

1. Add a deterministic turn-advancement fixture that exits immediately after the turn/year increment and before popup/script-driven upkeep.
2. Keep pixel or accessibility-based UI automation limited to menu, new-game/load-game, and map-entry integration coverage.
3. Select the next known-layout recovery candidate and repeat the behavioral, ABI, build, metadata, island, staging, and runtime gates.

## Relevant Files

- `src/alphanet.h`: committed aligned `AlphaNet` layout and adapter declaration.
- `src/alphanet.cpp`: committed `AlphaNet::pid_2_idx` implementation.
- `src/dllmain.cpp`: transactional signature-checked redirects; 25 source recoveries plus the gameplay gate's active-turn and call-site hooks.
- `src/scenario.h`, `src/scenario.cpp`: opt-in gameplay fixture loading, inspection, command assertions, result writing, and verified active-turn trampoline.
- `src/caviar.h`: recovered `CaviarData`, `Caviar`, `VOX_Vect`, and `VOX_Matrix` layouts.
- `src/caviar.cpp`: recovered Caviar constructors, camera, and scaling behavior.
- `src/buffer.h`, `src/buffer.cpp`: recovered Buffer setters and lifecycle hooks.
- `src/dialog.h`, `src/dialog.cpp`: recovered Dialog setters.
- `src/dialogs.h`, `src/dialogs.cpp`: recovered empty `Dialogs::close`.
- `src/basepop.h`, `src/basepop.cpp`: corrected layout and recovered location setter.
- `src/buttongroup.h`, `src/buttongroup.cpp`: recovered button-group insertion.
- `src/win.h`, `src/win.cpp`: recovered `in_box`.
- `src/autosound.h`, `src/autosound.cpp`: recovered `do_sound` hook.
- `src/maininterface.h`, `src/maininterface.cpp`: recovered null interface hooks.
- `docs/recovery-overrides.csv`: runtime-integrated `source_complete` overrides.
- `docs/recovery/functions.csv`: canonical 6,000-function inventory.
- `docs/recovery/priorities.csv`: currently regenerated to 5,125 candidates.
- `docs/recovery/analysis-correlation.csv`: canonical, IDA, and Ghidra correlation.
- `docs/recovery/analysis-summary.json`: analyzer identities and bound input hashes.
- `docs/recovery/summary.json`: canonical recovery-state counts.
- `tools/extract_legacy_leaves.py`: conservative local-only island extractor.
- `tools/test_extract_legacy_leaves.py`: 12 island-classifier regression tests.
- `tools/smoke_hybrid_game.py`: non-destructive Wine launch, diagnostics, and rendering smoke gate.
- `tools/test_smoke_hybrid_game.py`: source-owned smoke-diagnostics parser tests.
- `tools/owned_wine_prefix.py`: marker-protected initialization and shutdown for the dedicated runtime-test prefix.
- `tools/runtime_process.py`: random executable aliases and exact owned-wrapper discovery/termination.
- `tools/run_gameplay_scenario.py`: deterministic scenario launcher, result validator, and owned-process cleanup.
- `tools/test_run_gameplay_scenario.py`: source-owned fixture, result, diagnostics, and process-alias tests.
- `tools/ghidra/DecompileFunction.java`: exact-entry decompiler used with the persistent project.
- `tools/ghidra/ExportInteriorReferences.java`: exports external references entering function interiors.
- `docs/recovery/ghidra-interior-references.csv`: committed 2,574-row interior-reference sidecar.
- `docs/LEGACY_ISLANDS.md`: ownership, eligibility, lifecycle, and zero-island release rules.
- `docs/HYBRID.md`: local hybrid workflow.
- `CMakeLists.txt`: source list, hybrid targets, and legacy-island targets.
- `build/ghidra-projects/live-recovery`: ignored persistent Ghidra project.
- `build/mingw-i686-release/legacy-leaves/manifest.json`: current ignored 150-island manifest.
- `build/mingw-i686-release/legacy-leaves.obj`: ignored local i386 COFF object.
- `.opensmacx/game/terranx.exe`: ignored hash-pinned PRACX runtime executable used by hybrid staging.
- `.opensmacx/game/terranx_original.exe`: ignored pre-PRACX executable retained as an analysis input.
- `.opensmacx/game/terranx_hybrid.exe`: ignored staged hybrid executable.

## Current Blocker

There is no recovery or tooling blocker. The gameplay gates verify movement-order issuance, resolved adjacent movement, and end-turn request state, but not next-turn advancement. Do not force native DDrawCompat on Wine Staging 11.10, and do not terminate pre-existing Wine/game processes to make room for a test.
