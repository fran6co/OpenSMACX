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
- Local artifact paths must have no symlink components; configure-time and Python checks reject writes that could escape or alias another artifact.
- Never commit or distribute generated assembly or object files.
- Keep each eligible legacy island as a separate symbol and section so it can be replaced independently.
- Do not revert unrelated worktree changes.

## Recovery State

- Canonical inventory: 6,000 functions.
- Game functions: 5,627.
- Library functions: 338.
- Thunks: 35.
- Current recovery backlog: 5,075 candidates.
- Current local legacy-island count: 127, reduced from 174.
- `DllMain` entry redirects: 55, comprising 53 source recoveries and two inactive-pass-through gameplay hooks. The gameplay gate also installs two call-site hooks; scenario behavior activates only when its environment is configured.
- Runtime redirects are signature-checked, transactional, and rolled back in reverse order.

### Analysis Inputs

- Ghidra executable SHA-256: `01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf`.
- Canonical IDB input SHA-256: `7d1933da68a3326ac97464849a209a5f127606f5bd7a6abfe9985cd3ce044767`.
- Canonical IDB SHA-256: `6ffdcf2d6644f2c1b19c218d3b1b293b4e442d56b8cf1f537b0403608ff866fa`.
- Persistent ignored canonical IDB: `.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`; analysis databases contain proprietary bytes and must never be committed.
- Persistent ignored Ghidra project: `build/ghidra-projects/live-recovery`.
- Historical external-analysis catalog: `docs/recovery/external-analysis-sources.json`; exact snapshots remain ignored and are hypothesis inputs only.
- Local correlation currently maps 88 of 91 Yitzi function-note addresses and all 1,352 Dio disassembly-label addresses to canonical function ranges.
- The exported-first queue covers all 462 DEF exports: no unverified exact-name replacements or mapped unrecovered functions, 415 exact source-complete mappings, and 47 name-ambiguous rows manually resolved as 45 source-complete mappings and two source-only compatibility exports.

## Completed Work

### Inventory and Hybrid Workflow

- `bb77d47 Add decompilation recovery inventory`
- `d4bb283 Correlate independent recovery analyses`
- `eb7432e Classify remaining executable bindings`
- `c619bf3 Normalize IDA analysis metadata`
- `f186b39 Recover base coordinate lookup`
- `cc47ad1 Add local hybrid image bootstrap`
- `6565518 Add verified hybrid executable workflow`
- `2ea0897 Harden recovery verification workflow`
- The canonical IDB now lives only at the ignored, hash-pinned `.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`; IDA databases are prohibited repository artifacts.
- `verify-recovery-metadata` deterministically regenerates and compares every canonical/correlation output, and hybrid staging depends on it.
- `verify-recovery-batch` builds and runs behavioral tests, ABI checks, differential oracles, ordinary island regeneration, metadata verification, staging, and runtime smoke.
- All proprietary-producing paths are restricted to nonsymlinked descendants of `.opensmacx/` or `build/`.

### Legacy Island Tooling

- `099511b Add local legacy leaf islands`
- The 21 classifier and extraction tests pass.
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
- `23a0c86 Recover Time class lifecycle exports`
- `4cec2b6 Recover TextIndex wrapper exports`
- `0fcede4 Recover Text constructors`
- `328a566 Recover Text global lifecycle exports`
- `7b85aa7 Recover AlphaNet identity lookups`
- `ef2086d Recover Win and Scroll helpers`

Recovered source includes:

- Five `Buffer` font and color setters.
- Four `Dialog` font and color setters plus bounded ID/position lookup, selection, and selected-ID retrieval.
- `StringStruct::current_id`, `current_entry`, `next_entry`, and `seek_id`.
- `text_get`, `text_string`, `text_item`, `text_item_string`, `text_item_number`, `text_item_binary`, and `text_item_hex` through the staged hybrid export redirects.
- `text_get_number` through the staged hybrid export redirect.
- `text_close`, `text_set_get_ptr`, and `text_set_item_ptr` through the staged hybrid export redirects.
- `text_open` through the staged hybrid export redirect.
- `Text` construction, destruction, and global lifecycle through staged hybrid export redirects.
- `TextIndex` construction, destruction, `text_make_index`, `text_search_index`, and `text_clear_index` through staged hybrid export redirects.
- `Spot` construction, destruction, and clearing through staged hybrid export redirects.
- Both `Font` constructors and the destructor through staged hybrid export redirects.
- `Time` construction, destruction, modal-state setters, and class counter lifecycle through staged hybrid export redirects.
- `Filemap` construction and destruction through staged hybrid export redirects.
- `Heap` construction and destruction through staged hybrid export redirects.
- `Strings` construction and destruction through staged hybrid export redirects.
- `Random` initialization, lifecycle, reseeding, seed retrieval, and both global generation wrappers through staged hybrid export redirects.
- `Log` initialization, exit cleanup, both constructors, destruction, reset, all four decimal/hexadecimal output wrappers, and state control through staged hybrid export redirects.
- `in_box` and `do_sound`.
- `BasePop::set_loc`, `BasePop::set_string_font`, and `ButtonGroup::add`.
- `ButtonGroup` construction, close, and initialization.
- `CaviarData` and `Caviar` constructors, scaling, camera state, and scaling getter.
- `MainInterface::clear_message`, `desktop_update`, Buffer lifecycle hooks, and `Dialogs::close`.
- `AlphaNet::pid_2_idx`, `pid_2_who`, `who_2_pid`, and `who_2_idx` plus their fastcall-to-thiscall runtime adapters.
- `Win::move`, `set_vert_paging`, and `set_horz_paging`, plus `Scroll::set_border_color` and their fastcall-to-thiscall runtime adapters.
- `Menu::set_menu_proc`, `Menu::id_to_index`, and all six `PullDown` item-state mutators plus `get_selected`, with fastcall-to-thiscall runtime adapters.

Other completed corrections and checks:

- Fixed a latent `BasePop` layout hole with a `0xC94` placeholder. `BasePop` has verified size `0x3230`.
- Corrected the `AlphaNet` layout to `0x14A0`; the constructor initializes its trailing `Heap` at offset `0x148C`.
- Added verified `Win`, `GraphicWin`, and `Scroll` layouts of `0x444`, `0xA14`, and `0x214C` respectively; the two Win rectangles and Scroll thumb rectangle now have explicit `RECT` storage.
- Added verified `MenuEntry`, `Menu`, `PullDownItem`, and `PullDown` layouts of `0x14`, `0xB64`, `0x14`, and `0xF40` respectively.
- Added verified layouts for `CaviarData`, `Caviar`, `ButtonGroup`, `Buffer`, and `Dialog`.
- ButtonGroup lifecycle tests verify the exact preserved `0x84..0x8B` constructor/close hole, complete initialization, object canaries, destructor behavior, and adapter return values.
- BasePop string-font tests cover null and uninitialized primary fonts, all four verified field offsets, return codes, and full-object canaries.
- Dialog ID lookup tests cover stale null-head state, non-dereferenced invalid heads for non-positive counts, matches, duplicates, bounded misses, cyclic lists, and complete object/entry canaries.
- Dialog selection tests cover stale null-head positions, invalid non-positive heads, duplicate matches, bounded invalid successors, and complete object/entry canaries.
- Dialog selected-ID tests cover forward/backward cursor restoration, upper and lower out-of-range positions, null heads, and legacy `INT_MIN` wrapping.
- Dialog position-to-ID tests verify explicit positions ignore selection state while preserving the same bounded forward/backward and wrapping semantics.
- StringStruct accessor tests verify the head-only guard, poison stale cursors, exact ID/payload offsets, ignored count/position fields, and complete object/entry canaries.
- StringStruct advance tests cover empty lists, circular traversal, count/position mismatch, exact-equality reset, and signed-position wrapping.
- StringStruct seek tests cover null and nonpositive lists, first/middle/last and duplicate matches, bounded misses, invalid successors, and complete object/entry canaries.
- StringStruct seek uses a C-linkage bridge whose fastcall adapter explicitly restores the original return flags in Debug and Release.
- TextIndex lifecycle tests verify the exact `0x118` layout, constructor/destructor writes, one Heap shutdown, active-only global clearing, preserved filenames, skipped entries, and complete array canaries.
- TextIndex wrapper tests verify first-free insertion, full-array no-ops, sparse ordered searches, continuation after all negative results, zero-valued success, exact argument forwarding, and complete array canaries.
- Text constructor tests verify exact field writes, preserved path storage, first-allocation failure short-circuiting, both allocation sizes and results, complete object canaries, and both constructor return conventions.
- Text global lifecycle tests verify in-place process-owned construction, both 512-byte allocations, preserved object storage, unconditional exit registration after allocation failure, cleanup delegation, and the absence of the previous temporary allocation leak.
- Text destructor tests verify its exact `0x160` layout, one shutdown delegation, complete object canaries, and a non-COMDAT direct tail jump in Debug and Release.
- Text open-wrapper tests verify exact object and argument forwarding, nullable inputs, unnormalized return values, and complete object canaries.
- Spot lifecycle tests verify the exact `0xC` layout, full clear behavior, both destructor branches, and complete object canaries.
- Font lifecycle tests verify the exact `0x28` layout, preserved `0x14` and `0x20` fields, constructor forwarding, both resource-cleanup branches, and complete object canaries.
- Recovered Text, TextIndex, Spot, Font, Time, Filemap, Heap, Strings, Random, and Log constructors explicitly preserve the legacy `EAX = this` return in Debug and Release.
- Time lifecycle tests verify the exact `0x28` layout, complete reset state, constructor return, destructor delegation, modal publication/clearing, signed counter wrapping, and complete object canaries.
- Filemap lifecycle tests verify the exact `0x10` layout, preserved file-size field, conditional handle/view cleanup, constructor return, and complete object canaries.
- Heap lifecycle tests verify the exact `0x14` layout, direct destructor cleanup without shutdown delegation, all five fields, and complete object canaries. ABI checks separately verify byte-only writes preserve the three padding bytes.
- Heap byte fields use explicit volatile writes so optimized inline construction, destruction, and shutdown preserve the three legacy padding bytes in Release as well as Debug.
- Win movement tests cover both rectangle selectors, unrelated flag bits, positive/negative and identity movement, complete object canaries, and 32-bit coordinate/dimension wrapping.
- Win paging tests cover vertical/horizontal, direct/adapter, null, distinct and aliased scroll pointers, raw signed values, exact target offsets, and complete Win/Scroll canaries.
- Scroll border-color tests cover the `-1` sentinel, signed color extremes, zero/one/negative and wrapping thicknesses, poisoned prior rectangles, exact write boundaries, and complete object canaries.
- PullDown item-state tests cover first/middle/last entries, all six mutations, stable hide/show states, first-match duplicates, sentinel termination, ignored counts, visible-count wrapping, exact dirty-byte writes, direct/adapted calls, and complete object canaries.
- PullDown selection tests cover the `-1` sentinel, disabled entries, ordinary and wrapping indices, legacy unchecked 32-bit address arithmetic, direct/adapted calls, and complete object canaries.
- Menu tests cover null and non-null callbacks, preserved callback return residue, first/middle/last and duplicate ID matches, sentinel termination, ignored counts, direct/adapted calls, and complete object canaries.
- Strings lifecycle tests verify the exact `0x18` layout, one Heap shutdown, preserved populated state, constructor return, and complete object canaries.
- Random tests verify the exact four-byte layout, lifecycle writes, signed bound ordering including negative and wrapping ranges, exact LCG seed transitions, bit-identical floating results, clean x87 status, and all six global entry points.
- Log tests verify the exact eight-byte layout, constructor/destructor writes, preserved initialized-constructor state, filename allocation and copying, reset mode, global placement construction, exit cleanup, state inversion, constructor returns, all four output formats, append arguments, open failure, every disable gate, and complete object canaries.
- Random floating generation now transfers the synthesized IEEE single without the previous out-of-bounds eight-byte type pun.
- Caviar scaling uses raw integer transfer to preserve NaN payloads, floating-point exceptions, and `EAX`. Release disassembly contains no unwanted x87 transfer.
- Hybrid staging and launch succeeded for the earlier Buffer and Dialog recovery batches.
- All completed batches passed Debug and Release MinGW builds, metadata regeneration, island removal, and independent review.

### Recovery Verification

- The direct-source `recovery-leaf-tests` harness passes under Wine in Debug and Release. It covers all seven AlphaNet process-ID and identity slots, signed identities, first-match duplicates with distinct payloads, zero IDs, exact scan boundaries, complete object canaries, all four redirect adapters, and `in_box` edge semantics.
- CTest always registers the Windows behavioral test through `tools/run_windows_test.py`, which auto-detects Wine and uses the build's dedicated owned test prefix.
- The `verify-recovery-abi` target and CTest check pass in Debug and Release. They verify i386 COFF, required symbols, thiscall cleanup, fastcall adapter cleanup including optimized PullDown tail jumps, and both gameplay trampolines' overwritten instruction/call, preserved state, callback stack cleanup, and continuations.
- `verify-recovery-oracles` extracts 18 explicitly selected recovered leaves into the ignored build tree and compares the three AlphaNet identity lookups, `Random::reseed`, integer `Random::get`, three Win movement/paging methods, `Scroll::set_border_color`, seven PullDown accessors, and two Menu accessors against source with identical fixtures in Debug and Release.
- Explicit oracle extraction accepts recovered canonical addresses but restricts all proprietary outputs to ignored subdirectories of `.opensmacx/` or `build/`.
- Lifecycle tests verify actual Heap, Strings, Spot, and Log deallocation; Filemap handle/view closure; Log initialization failure paths; and Random/Log exit callback registration.
- The floating `Random::get` body is not eligible for a copied-byte oracle because it contains an absolute image reference; its source-level tests retain bit-pattern and x87-status coverage.
- Regenerated state after the Menu/PullDown recovery is 5,075 priorities, 559 source-complete functions, 5,041 unrecovered functions, and 127 islands.

### Hybrid Runtime Compatibility

- `90a99f8 Use PRACX runtime for hybrid staging`
- The pre-PRACX executable reaches DirectDraw `E_NOTIMPL` on Wine Staging 11.10, then the legacy `DDRAWERROR` path crashes while using popup globals before `BasePop::init_class()`.
- Forcing the bundled native DDrawCompat proxy also fails fast on this Wine version.
- Hybrid staging defaults to the hash-pinned PRACX executable at `.opensmacx/game/terranx.exe` and publishes all 460 expected import redirects.
- The packer labels PRACX `hash_pinned_runtime_build`; all recovery body mappings are `not_analyzed` and unmapped rather than projected from canonical addresses.
- Legacy-island extraction separately remains bound to the independently analyzed pre-PRACX executable and produces 127 islands.
- Always launch through `tools/run_game.py`. On macOS it uses the Wine application bundle, explicitly passes `WINEPREFIX`, and temporarily skips PRACX intro movies unless `--play-intro-movie` is requested.
- The PRACX hybrid loader trace reached DirectDraw rendering and loaded `OpenSMACX.dll`, `prax.dll`, and Wine's built-in `DDRAW.dll` without a main-process unhandled exception.
- `tools/smoke_hybrid_game.py` automates that gate, requires the executable, `OpenSMACX.dll`, `prax.dll`, and builtin `DDRAW.dll` in one Wine loader context, validates process survival and rendering when Wine emits a flip trace, rejects required-module failures and unhandled exceptions, and stops the dedicated owned test prefix while removing its per-run executable alias.
- ImportAdder runs only in the marker-protected build Wine prefix, receives an explicit `WINEPREFIX`, and stops only that prefix after every invocation.
- `tools/run_gameplay_scenario.py` temporarily bypasses intro movies, records Wine SEH/thread diagnostics, loads a local ignored save, deterministically invokes the verified active-turn handler after refresh, inspects legal movement candidates, asserts source `go_to` movement-order state, or resolves the order through legacy `action_go_to` before requesting end turn.
- The gameplay runner waits for a terminal JSON result, rejects fatal Wine diagnostics, and stops its dedicated owned prefix while verifying removal of its per-run executable alias. A passing local fixture used turn 12, vehicle 0, `(22,26)` to `(23,27)`; resolution spent 3 movement points, moved the map stack, and cleared the order.
- Repeated advancement runs can still stall inside legacy `action_go_to` or while awaiting upkeep; the same movement-resolution stall reproduces with the `StringStruct::seek_id` redirect disabled. Hybrid smoke remains the mandatory per-recovery runtime gate until that legacy scenario path is made deterministic.
- The gameplay trampoline at the verified `Console::human_turn` seam preserves registers and flags, executes the overwritten store, and selects either the ordinary or early-exit continuation.
- The post-increment trampoline verifies the single-player `turn_upkeep` caller, turn/year state, and resolved vehicle position, then exits before later popup/script upkeep through the normal epilogues. The local fixture advances turn 12 to 13 and mission year 2113.
- `~/Desktop/backtrace.txt` is manually saved and cannot be used as an automatic crash signal.

## Next Steps

1. Recover the larger arithmetic-heavy `Scroll::compute_thumb_rect` as a separate batch.
2. Keep pixel or accessibility-based UI automation limited to menu, new-game/load-game, and map-entry integration coverage.

## Relevant Files

- `src/alphanet.h`: verified `0x14A0` `AlphaNet` layout and lookup adapter declarations.
- `src/alphanet.cpp`: recovered four process-ID and identity lookup implementations.
- `src/dllmain.cpp`: transactional signature-checked redirects; 53 source recoveries plus the gameplay gate's active-turn, post-increment upkeep, and call-site hooks.
- `src/scenario.h`, `src/scenario.cpp`: opt-in gameplay fixture loading, inspection, command assertions, result writing, and verified active-turn trampoline.
- `src/caviar.h`: recovered `CaviarData`, `Caviar`, `VOX_Vect`, and `VOX_Matrix` layouts.
- `src/caviar.cpp`: recovered Caviar constructors, camera, and scaling behavior.
- `src/buffer.h`, `src/buffer.cpp`: recovered Buffer setters and lifecycle hooks.
- `src/dialog.h`, `src/dialog.cpp`: recovered Dialog setters, bounded ID/position lookup, selection, and selected-ID retrieval.
- `src/random.h`, `src/random.cpp`: verified Random layout, lifecycle, signed-range generation, and exact floating transfer.
- `src/log.h`, `src/log.cpp`: verified Log layout, lifecycle, global initialization/cleanup, reset, decimal/hexadecimal output wrappers, and state control.
- `src/stringstruct.h`, `src/stringstruct.cpp`: verified standalone string-list layout and current ID/payload/advance/seek accessors.
- `src/text_recovery.h`, `src/text_recovery.cpp`: verified Text constructors, destructor, open wrapper, get, and numeric-item source helpers.
- `src/dialogs.h`, `src/dialogs.cpp`: recovered empty `Dialogs::close`.
- `src/basepop.h`, `src/basepop.cpp`: corrected layout and recovered location setter.
- `src/basepop_font.cpp`: recovered BasePop string-font setter in an isolated testable source unit.
- `src/buttongroup.h`, `src/buttongroup.cpp`: recovered button-group insertion.
- `src/win.h`, `src/win.cpp`: verified Win layout, `move`, both paging setters, adapters, and `in_box`.
- `src/scroll.h`, `src/scroll.cpp`: verified Scroll layout, border-color/thumb-rectangle reset, and adapter.
- `src/menu.h`, `src/menu.cpp`: verified Menu layout, callback setter, bounded ID lookup, and adapters.
- `src/pulldown.h`, `src/pulldown.cpp`: verified PullDown layout, six item-state mutators, unchecked selected-index accessor, and adapters.
- `src/autosound.h`, `src/autosound.cpp`: recovered `do_sound` hook.
- `src/maininterface.h`, `src/maininterface.cpp`: recovered null interface hooks.
- `docs/recovery-overrides.csv`: runtime-integrated `source_complete` overrides.
- `docs/recovery/functions.csv`: canonical 6,000-function inventory.
- `docs/recovery/priorities.csv`: currently regenerated to 5,075 candidates.
- `docs/recovery/analysis-correlation.csv`: canonical, IDA, and Ghidra correlation.
- `docs/recovery/analysis-summary.json`: analyzer identities and bound input hashes.
- `docs/recovery/external-analysis-sources.json`: hash-pinned historical-analysis identities and local-only handling policy.
- `docs/recovery/summary.json`: canonical recovery-state counts.
- `tools/fetch_external_analysis.py`: verified local fetcher for ignored historical-analysis snapshots.
- `tools/correlate_external_analysis.py`: address-only correlation for local Yitzi and Dio inputs.
- `tools/build_export_recovery_queue.py`: exported-first queue generator combining recovery and external-lead evidence.
- `tools/test_external_analysis.py`: source-owned parser, correlation, provenance, and queue-tier tests.
- `tools/extract_legacy_leaves.py`: conservative local-only island extractor.
- `tools/test_extract_legacy_leaves.py`: 21 classifier, explicit-selection, symlink-containment, and output-ownership regression tests.
- `tools/local_artifact.py`: shared nonsymlinked `.opensmacx/`/`build/` output ownership enforcement.
- `tools/smoke_hybrid_game.py`: non-destructive Wine launch, diagnostics, and rendering smoke gate.
- `tools/movie_skip.py`: transactional PRACX movie-command override for owned launch tools.
- `tools/test_smoke_hybrid_game.py`: source-owned loader-context, diagnostics, prefix-ownership, and movie-skip tests.
- `tools/owned_wine_prefix.py`: marker-protected initialization and shutdown for the dedicated runtime-test prefix.
- `tools/runtime_process.py`: random executable aliases and exact owned-wrapper discovery/termination.
- `tools/run_gameplay_scenario.py`: deterministic scenario launcher, result validator, and owned-process cleanup.
- `tools/test_run_gameplay_scenario.py`: source-owned fixture, result, diagnostics, and process-alias tests.
- `tools/ghidra/DecompileFunction.java`: exact-entry decompiler used with the persistent project.
- `tools/ghidra/ExportInteriorReferences.java`: exports external references entering function interiors.
- `docs/recovery/ghidra-interior-references.csv`: committed 2,574-row interior-reference sidecar.
- `docs/LEGACY_ISLANDS.md`: ownership, eligibility, lifecycle, and zero-island release rules.
- `docs/HYBRID.md`: local hybrid workflow.
- `tests/recovery_oracle_tests.cpp`: source-versus-original AlphaNet, Random, Win, Scroll, Menu, and PullDown fixtures.
- `CMakeLists.txt`: source list, hybrid targets, legacy-island targets, and local differential-oracle target.
- `build/ghidra-projects/live-recovery`: ignored persistent Ghidra project.
- `build/mingw-i686-release/legacy-leaves/manifest.json`: current ignored 127-island manifest.
- `build/mingw-i686-release/recovery-oracles/manifest.json`: ignored explicit 18-function AlphaNet/Random/Win/Scroll/Menu/PullDown oracle manifest.
- `build/mingw-i686-release/legacy-leaves.obj`: ignored local i386 COFF object.
- `.opensmacx/game/terranx.exe`: ignored hash-pinned PRACX runtime executable used by hybrid staging.
- `.opensmacx/game/terranx_original.exe`: ignored pre-PRACX executable retained as an analysis input.
- `.opensmacx/game/terranx_hybrid.exe`: ignored staged hybrid executable.

## Current Blocker

There is no recovery or tooling blocker. The gameplay gates verify movement-order issuance, resolved adjacent movement, end-turn request state, and the next turn/year increment before later upkeep. Do not force native DDrawCompat on Wine Staging 11.10, and do not terminate pre-existing Wine/game processes to make room for a test.
