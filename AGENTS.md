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
- Current recovery backlog: 5,018 candidates.
- Current local legacy-island count: 114, reduced from 174.
- `DllMain` entry redirects: 112, comprising 110 source recoveries and two inactive-pass-through gameplay hooks. The gameplay gate also installs two call-site hooks; scenario behavior activates only when its environment is configured.
- Runtime redirects are signature-checked, transactional, and rolled back in reverse order.

### Analysis Inputs

- Ghidra executable SHA-256: `01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf`.
- Canonical IDB input SHA-256: `7d1933da68a3326ac97464849a209a5f127606f5bd7a6abfe9985cd3ce044767`.
- Canonical IDB SHA-256: `6ffdcf2d6644f2c1b19c218d3b1b293b4e442d56b8cf1f537b0403608ff866fa`.
- Persistent ignored canonical IDB: `.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`; analysis databases contain proprietary bytes and must never be committed.
- Persistent ignored Ghidra project: `build/ghidra-projects/live-recovery`.
- Historical external-analysis catalog: `docs/recovery/external-analysis-sources.json`; exact snapshots remain ignored and are hypothesis inputs only.
- Local correlation currently maps 88 of 91 Yitzi function-note addresses and all 1,352 Dio disassembly-label addresses to canonical function ranges.
- The catalog's `additional_repositories` entry pins the Thinker mod (GPLv2; incompatible with this project's GPLv3-or-later, so its text can never be copied or committed regardless of hypothesis policy). `tools/correlate_thinker_layouts.py` reduces the fetched headers to ignored struct-offset and global-address hypothesis CSVs; every lead still requires independent verification against the hash-bound canonical executable before source, tests, or metadata are committed.
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
- Its stamp hashes the exact source-metadata projection consumed by the exporter, so ordinary implementation, test, and runtime-oracle edits reuse a verified result while annotation or fixed-address-binding changes still force catalog regeneration and byte comparison.
- Its ignored canonical-export checkpoint separately hashes the binary-analysis input closure, binary-only projections of `functions.csv` and `summary.json`, and the complete call graph. Source annotations, bindings, redirects, and overrides are cheaply reapplied before correlation, so normal recovery promotions avoid the multi-minute IDB parse while correlation and final byte comparisons still run; binary drift invalidates the checkpoint and `--force` rebuilds every stage.
- `verify-recovery-batch` builds and runs behavioral tests, ABI checks, differential oracles, ordinary island regeneration, metadata verification, staging, and runtime smoke.
- All proprietary-producing paths are restricted to nonsymlinked descendants of `.opensmacx/` or `build/`.

### Legacy Island Tooling

- `099511b Add local legacy leaf islands`
- The 21 classifier and extraction tests pass.
- The Ghidra interior-reference sidecar contains 2,574 references and is hash-bound in `analysis-summary.json`.
- Initial extraction produced 174 islands and 174 COFF globals in `pe-i386` format with no relocations.

### Static Recompilation Pilot

- A time-boxed local-only pilot statically lowers `Wave_Device::is_group_disabled` at `0x004C5460` into explicit CPU-state C++ basic blocks under ignored per-preset build directories.
- Debug and Release verification pass deterministic fresh regeneration, complete source/oracle provenance checks, pe-i386 and `thiscall` ABI checks, standalone execution, and original-byte differential fixtures for full `EAX`, partial `EDX`, defined flags, stack movement, object memory, and canaries.
- The compiler-generated boundary is deliberately a result-only adapter; exact volatile-register and flag equivalence belongs to the explicit-state core.
- The pilot does not change recovery state and generated C++, assembly, objects, manifests, and executables remain local proprietary-derived artifacts.
- The value gate stopped expansion: the leaf duplicates the simpler exact-island mechanism, while validating call-bearing `Wave_Device::enable` at `0x004C51C0` would require a disproportionate separate canonical-process injection harness because PRACX recovery bodies are unmapped.

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
- `12fb52d Recover Menu and PullDown helpers`
- `dda4970 Recover Scroll thumb rectangle`
- `eef482b Recover Scroll sprite setters`
- Recovered the five Scroll init wrappers, with the primary initializer retained as an explicit classified original dependency.
- Recovered three wrapping geometry leaves and six `Vector` lifecycle/arithmetic leaves.
- Recovered `GraphicWin::~GraphicWin` at `0x005D4DD0` (185 callers), the highest-fan-in function in the binary, with its Win subobject destructor retained as a classified temporary original dependency; the Buffer side is source-owned.
- Recovered `GraphicWin::close` at `0x005D4E40` (66 callers), closing through the original `Win::close` dependency and source-owned `Buffer::close`, then preserving the ordered field reset, process default, optional scalar-deleting virtual call, and both `EAX` residues.
- Recovered `Scroll::close` at `0x00605370` (136 callers), restoring every Scroll-owned default from the two process tables before closing its embedded FlatButtons in left/right order and delegating to source-owned `GraphicWin::close`.
- Recovered the BaseButton/FlatButton teardown cluster: `BaseButton::~BaseButton` at `0x00607040` (134 callers), `BaseButton::close` at `0x006070C0`, `FlatButton::close` at `0x00607DA0` (134 callers), and `FlatButton::~FlatButton` at `0x00406880` (93 callers). The source preserves both vtable stages, Time teardown order, process-default reloads, two executable-CRT frees and their return residue, and the derived-to-base close chain reached by every recovered `Scroll::close`.
- Recovered the derived two-stage close at `0x004066C0` (61 callers, the highest remaining fan-in), which closes under its own tables before closing its StringStruct base.
- Recovered `StringStruct::close` at `0x00401060` (25 callers), entered through a virtual-base adjustor that hands the redirect a pointer `0x1C` bytes into the object; it installs both virtual tables and reuses the recovered entry walk.
- Recovered `spying` at `0x0055BC00` (19 callers), a pure intelligence-visibility check over four original tables.
- Recovered `vector_add` at `0x00628150` (17 callers), a self-contained x87 vector sum verified by a copied-byte oracle across special values, overlapping layouts, and all four rounding modes.
- Recovered `Buffer::destroy` at `0x005D7410`, which **retires the `BufferOriginalDestructor` temporary dependency**: the GraphicWin destructor now reaches a source-owned Buffer teardown through a bindable seam instead of the original body. `Win::close`, `Win::~Win`, and `Scroll::init` remain as the three explicitly preflighted UI dependencies.
- Recovered `Buffer::close` at `0x005D7470` (44 callers), covering the twenty-entry release loop, both DirectDraw and device-context teardown paths, and the full field reset; `buffer-release` verifies the release loop against real allocations in the deferred phase.
- Recovered `find_font` at `0x005882F0` (31 callers), whose 9999 seed doubles as a match threshold, and `Buffer::text_line_height` at `0x005DCAB0` (21 callers).
- Recovered `StringStruct::remove_all` at `0x00402970` (79 callers), which walks the entry list through the owner callback and MSVC virtual-base scalar deleting destructors.
- Recovered `Buffer::get_data` at `0x005E3373` (48 callers) and `Buffer::free_data` at `0x005E34A3` (43 callers), the reference-counted DirectDraw surface lock pair feeding `Buffer::close`.
- Recovered `Sprite::close` at `0x005E3820` (111 callers), completing the Sprite lifecycle pair.
- Recovered the two highest-fan-in bottleneck primitives: `Win::is_visible` at `0x005F7E90` (120 callers) and `Sprite::Sprite` at `0x005E37E0` (154 callers), both verified by new in-process runtime-oracle suites because they carry a recursive call and an absolute accounting global respectively.
- Recovered `Win::client_to_screen` at `0x005ED240` (71 callers), preserving recursive parent-chain translation, bit-15 parent-origin subtraction, output alias ordering, and defined 32-bit wrapping. Its original-address Win oracle exercises the call-bearing body before redirects install.
- Recovered the constructor closure: `AutoSound::AutoSound` (`0x0062BA80`), `Win::Win` (`0x005EB3D0`), `Palette::get_rgbquad` (`0x005FE560`), `Buffer::Buffer` (`0x005D7210`, 80 callers), `GraphicWin::GraphicWin` (`0x005D4CF0`, 52 callers), and `BaseButton::BaseButton` (`0x00606F30`), verified by a new in-process `constructor` oracle suite. Both `verify-recovery-batch` gates pass 11/11 with both hybrid smokes, and all six mutation sweeps are triaged: Buffer kills 198/203, BaseButton 61/63, GraphicWin 21/24, and every survivor is equivalent by construction and recorded in a `Verification note:` on its function. Linking the oracle suite required adding `src/autosound.cpp` and `src/palette.cpp` to `recovery-oracle-tests`, because `Win::construct` builds its AutoSound member and `Buffer::construct` resolves default colours through `Palette::get_rgbquad`.

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
- Both `in_box` overloads, the origin-and-dimensions RECT constructor, the TutWin rectangle-center helper, and `do_sound`.
- `BasePop::set_loc`, `BasePop::set_string_font`, and `ButtonGroup::add`.
- `ButtonGroup` construction, close, and initialization.
- `CaviarData` and `Caviar` constructors, scaling, camera state, and scaling getter.
- `MainInterface::clear_message`, `desktop_update`, Buffer lifecycle hooks, and `Dialogs::close`.
- `AlphaNet::pid_2_idx`, `pid_2_who`, `who_2_pid`, and `who_2_idx` plus their fastcall-to-thiscall runtime adapters.
- `Win::move`, recursive `client_to_screen`, `set_vert_paging`, and `set_horz_paging`, plus the five Scroll init wrappers, `Scroll::set_border_color`, all four sprite-triplet setters, `set_range`, `set_pos`, four style setters, both thumb resetters, `compute_thumb_rect`, and their fastcall-to-thiscall runtime adapters.
- The named-field RECT expansion helper used by Scroll initialization.
- `Menu::set_menu_proc`, `Menu::id_to_index`, and all six `PullDown` item-state mutators plus `get_selected`, with fastcall-to-thiscall runtime adapters.
- `Vector` construction, clearing, subtraction, in-place addition/subtraction, and scaling, with fastcall-to-thiscall runtime adapters.

Other completed corrections and checks:

- Fixed a latent `BasePop` layout hole with a `0xC94` placeholder. `BasePop` has verified size `0x3230`.
- Corrected the `AlphaNet` layout to `0x14A0`; the constructor initializes its trailing `Heap` at offset `0x148C`.
- Added verified `Win`, `GraphicWin`, and `Scroll` layouts of `0x444`, `0xA14`, and `0x214C` respectively; the two Win rectangles and Scroll thumb rectangle now have explicit `RECT` storage.
- Added verified `MenuEntry`, `Menu`, `PullDownItem`, and `PullDown` layouts of `0x14`, `0xB64`, `0x14`, and `0xF40` respectively.
- Added verified layouts for `CaviarData`, `Caviar`, `ButtonGroup`, `Buffer`, `Dialog`, `BaseButton`, and `FlatButton`; the button sizes are `0xAB8` and `0xB4C`.
- Added a verified `Vector` layout of `0xC`.
- ButtonGroup lifecycle tests verify the exact preserved `0x84..0x8B` constructor/close hole, complete initialization, object canaries, destructor behavior, and adapter return values.
- BaseButton/FlatButton teardown tests cover direct and adapted closes, all four allocation shapes, exact process-default tables, ordered field writes and frees, complete object canaries, Time member order, transient derived/base vtables, two-stage destruction, and all legacy return residues.
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
- GraphicWin close tests cover direct/adapted calls, Win-then-Buffer delegation, late release-target reads through dependency-side installation/removal, exact ordered reset boundaries, process-default publication, scalar-deleting flag `1`, both return residues, and complete object canaries. Its 67 compiling statement/constant mutants are all killed; 27 malformed mutants do not compile and prove nothing.
- Win client-to-screen tests cover one- through three-node parent chains, both controlling flag bits, null parents, unrelated flags, distinct and aliased outputs, local/recursive/subtractive signed-boundary wrapping, direct/adapted calls, and complete object canaries.
- Win paging tests cover vertical/horizontal, direct/adapter, null, distinct and aliased scroll pointers, raw signed values, exact target offsets, and complete Win/Scroll canaries.
- Scroll init-wrapper tests cover null rectangles and parents, zero and signed-extreme lengths, zero and wrapping process-default thicknesses, wrapping RECT subtraction, exact primary-init argument forwarding, unnormalized return values, `_nc` write ordering and persistence, direct/adapted calls, unchanged inputs, and complete object canaries.
- Scroll border-color tests cover the `-1` sentinel, signed color extremes, zero/one/negative and wrapping thicknesses, poisoned prior rectangles, exact write boundaries, and complete object canaries.
- Scroll sprite-triplet tests cover left/right, direct/adapted calls, null/distinct/duplicate pointers, horizontal/vertical/equal geometry, signed-comparison divergence, negated `INT_MIN`, first-pointer return residue, exact primary/button offsets, and complete object canaries. Volatile stores and reads retain the legacy access order in optimized builds, which ABI checks verify in Debug and Release disassembly.
- Scroll state tests cover signed range normalization, reversed and equal ranges, signed position clamping/reflection, null parents, current-window publication, redraw callbacks, exact object writes, complete canaries, and return residue.
- Scroll style tests cover signed-extreme values, primary and embedded-button propagation, two ordered redraw callbacks, reset geometry, exact write boundaries, and complete object canaries.
- Scroll vertical-sprite tests cover up/down, direct/adapted calls, null/distinct/duplicate pointers, signed geometry branches including `INT_MIN`, ordered stores, first-pointer return residue, exact primary/button offsets, and complete object canaries.
- Scroll thumb-rectangle tests cover horizontal, vertical, and equal-dimension orientation; button and border geometry; reversed/equal ranges; signed truncation and wrapping; drag clamping; crossed bounds; nonsquare templates; exact and partial object aliases; output canaries; and the legacy internal-rectangle return residue. The signed division helper is forced to a source-owned x86 `IDIV` and verified in Debug and Release disassembly.
- RECT expansion tests cover zero, positive/negative, signed-extreme, and wrapping amounts; exact named-field access order; cdecl cleanup; pointer return residue; and complete input/output canaries.
- Geometry tests cover wrapping RECT construction, signed inclusive/exclusive origin-and-dimensions bounds, reversed and signed-extreme rectangle centers, ordered output aliases into every late-read field, return residue, and complete canaries.
- Vector tests cover zeroing lifecycle, exact `0xC` writes, direct/adapted calls, all arithmetic operations, output/self/right aliases and partial overlaps, raw special-value bit patterns, all x87 rounding modes, x87 status, ordered binary32 stores, return residue, and complete canaries.
- PullDown item-state tests cover first/middle/last entries, all six mutations, stable hide/show states, first-match duplicates, sentinel termination, ignored counts, visible-count wrapping, exact dirty-byte writes, direct/adapted calls, and complete object canaries.
- PullDown selection tests cover the `-1` sentinel, disabled entries, ordinary and wrapping indices, legacy unchecked 32-bit address arithmetic, direct/adapted calls, and complete object canaries.
- Menu tests cover null and non-null callbacks, preserved callback return residue, first/middle/last and duplicate ID matches, sentinel termination, ignored counts, direct/adapted calls, and complete object canaries.
- Strings lifecycle tests verify the exact `0x18` layout, one Heap shutdown, preserved populated state, constructor return, and complete object canaries.
- Random tests verify the exact four-byte layout, lifecycle writes, signed bound ordering including negative and wrapping ranges, exact LCG seed transitions, bit-identical floating results, clean x87 status, and all six global entry points.
- Log tests verify the exact eight-byte layout, constructor/destructor writes, preserved initialized-constructor state, filename allocation and copying, reset mode, global placement construction, exit cleanup, state inversion, constructor returns, all four output formats, append arguments, open failure, every disable gate, and complete object canaries.
- Random floating generation now transfers the synthesized IEEE single without the previous out-of-bounds eight-byte type pun.
- Caviar scaling uses raw integer transfer to preserve NaN payloads, floating-point exceptions, and `EAX`. Release disassembly contains no unwanted x87 transfer.
- Hybrid staging and launch succeeded for the earlier Buffer and Dialog recovery batches.
- Debug and Release DLLs are self-contained MinGW builds so both presets can execute the complete staged-hybrid smoke gate.
- All completed batches passed Debug and Release MinGW builds, metadata regeneration, island removal, staged-hybrid runtime smoke, and independent review.

### Recovery Verification

- The direct-source `recovery-leaf-tests` harness passes under Wine in Debug and Release. It covers all seven AlphaNet process-ID and identity slots, signed identities, first-match duplicates with distinct payloads, zero IDs, exact scan boundaries, complete object canaries, all four redirect adapters, and `in_box` edge semantics.
- CTest always registers the Windows behavioral test through `tools/run_windows_test.py`, which auto-detects Wine and uses the build's dedicated owned test prefix.
- The `verify-recovery-abi` target and CTest check pass in Debug and Release. They verify i386 COFF, required symbols, thiscall cleanup, fastcall adapter cleanup including BaseButton, FlatButton, GraphicWin, and Scroll close, the recursive Win queries, five Scroll wrappers, three geometry leaves, six Vector leaves, and optimized PullDown tail jumps; exact Vector x87 operation counts; BaseButton, GraphicWin, Scroll, and both Win runtime-oracle original-address calls; button vtables, teardown order, executable free dispatches and return residues; Scroll close's two ordered slot-`0x168` virtual dispatches and source-owned base-close relocation; the Scroll primary dependency's raw thiscall dispatch; and both gameplay trampolines' overwritten instruction/call, preserved state, callback stack cleanup, and continuations.
- `verify-recovery-oracles` extracts 32 explicitly selected recovered leaves into the ignored build tree and compares the three AlphaNet identity lookups, `Random::reseed`, integer `Random::get`, three Win movement/paging methods, three geometry leaves, six Vector leaves, three Scroll setters, `Scroll::compute_thumb_rect`, RECT expansion, seven PullDown accessors, and two Menu accessors against source with identical fixtures in Debug and Release.
- Explicit oracle extraction accepts recovered canonical addresses but restricts all proprietary outputs to ignored subdirectories of `.opensmacx/` or `build/`.
- Lifecycle tests verify actual Heap, Strings, Spot, and Log deallocation; Filemap handle/view closure; Log initialization failure paths; and Random/Log exit callback registration.
- The floating `Random::get` body is not eligible for a copied-byte oracle because it contains an absolute image reference; its source-level tests retain bit-pattern and x87-status coverage.
- Regenerated state is 5,018 priorities, 616 source-complete functions, 33 original dependencies, 4,981 unrecovered functions, and 114 islands.

### Hybrid Runtime Compatibility

- `90a99f8 Use PRACX runtime for hybrid staging`
- The pre-PRACX executable reaches DirectDraw `E_NOTIMPL` on Wine Staging 11.10, then the legacy `DDRAWERROR` path crashes while using popup globals before `BasePop::init_class()`.
- Forcing the bundled native DDrawCompat proxy also fails fast on this Wine version.
- Hybrid staging defaults to the hash-pinned PRACX executable at `.opensmacx/game/terranx.exe` and publishes all 460 expected import redirects.
- The packer labels PRACX `hash_pinned_runtime_build`; all recovery body mappings are `not_analyzed` and unmapped rather than projected from canonical addresses.
- Legacy-island extraction separately remains bound to the independently analyzed pre-PRACX executable and produces 114 islands.
- Always launch through `tools/run_game.py`. On macOS it uses the Wine application bundle, explicitly passes `WINEPREFIX`, and temporarily skips PRACX intro movies unless `--play-intro-movie` is requested.
- The PRACX hybrid loader trace reached DirectDraw rendering and loaded `OpenSMACX.dll`, `prax.dll`, and Wine's built-in `DDRAW.dll` without a main-process unhandled exception.
- `tools/smoke_hybrid_game.py` automates that gate, requires the executable, `OpenSMACX.dll`, `prax.dll`, and builtin `DDRAW.dll` in one Wine loader context, validates process survival and rendering when Wine emits a flip trace, rejects required-module failures and unhandled exceptions, and stops the dedicated owned test prefix while removing its per-run executable alias.
- The opt-in Scroll recovery oracle runs sixteen untouched original methods inside the verified PRACX process before redirects are installed, including `Scroll::close` and invalid non-delegating fixtures for all five init wrappers. The close fixture seeds both process default tables, verifies the left/right slot-`0x168` calls before the GraphicWin close chain, compares complete object/canary state and return residue, and restores every seeded global. The suite must write `passed` to an ignored nonsymlinked result path before smoke can pass. All direct redirect signatures and the temporary primary-init dependency are validated before the oracle executes; sprite redirects additionally validate their distinguishing field displacement.
- The Win runtime-oracle suite compares recursive visibility and client-to-screen translation against untouched original bodies before redirects install, including nested parent chains, controlling flags, wrapping arithmetic, aliased outputs, and complete object/canary preservation.
- The GraphicWin runtime-oracle suite calls untouched `GraphicWin::close` at `0x005D4E40` before redirects install and compares both release branches, complete object/canary state, dependency callback traces, and return residue. Its resource-free Win/Buffer fixture validates each dependency's self-publication relative to its own object before normalizing that one pointer for byte comparison; the source-level suite separately observes the two resets that `Win::close` performs first.
- The BaseButton runtime-oracle suite calls all four untouched button teardown bodies before redirects install, seeds and restores all three process-default tables, and compares complete object/canary state, callback traces, and return residues. Its deferred `basebutton-release` phase lifts only the BaseButton close redirect and exercises separate real executable-CRT allocations for the original and source sides.
- ImportAdder runs only in the marker-protected build Wine prefix, receives an explicit `WINEPREFIX`, and stops only that prefix after every invocation.
- `tools/run_gameplay_scenario.py` temporarily bypasses intro movies, records Wine SEH/thread diagnostics, loads a local ignored save, deterministically invokes the verified active-turn handler after refresh, inspects legal movement candidates, asserts source `go_to` movement-order state, or resolves the order through legacy `action_go_to` before requesting end turn.
- The gameplay runner waits for a terminal JSON result, rejects fatal Wine diagnostics, and stops its dedicated owned prefix while verifying removal of its per-run executable alias. A passing local fixture used turn 12, vehicle 0, `(22,26)` to `(23,27)`; resolution spent 3 movement points, moved the map stack, and cleared the order.
- Repeated advancement runs can still stall inside legacy `action_go_to` or while awaiting upkeep; the same movement-resolution stall reproduces with the `StringStruct::seek_id` redirect disabled. Hybrid smoke remains the mandatory per-recovery runtime gate until that legacy scenario path is made deterministic.
- The gameplay trampoline at the verified `Console::human_turn` seam preserves registers and flags, executes the overwritten store, and selects either the ordinary or early-exit continuation.
- The post-increment trampoline verifies the single-player `turn_upkeep` caller, turn/year state, and resolved vehicle position, then exits before later popup/script upkeep through the normal epilogues. The local fixture advances turn 12 to 13 and mission year 2113.
- `~/Desktop/backtrace.txt` is manually saved and cannot be used as an automatic crash signal.

### Release paths are verified by a deferred second oracle phase

The executable's CRT heap cannot be used while `DllMain` runs. Calling the
bound `_malloc` at `0x006470A6` there deadlocks, because the heap lock at
`0x009C0538` is unowned until the executable's CRT startup executes:

```
err:sync:RtlpWaitForCriticalSection section 009C0538 wait timed out, blocked by 0000
```

That made release paths unverifiable from the first oracle phase, because the
two requirements conflict: a differential must call the original body, which
means running before redirects install, while real allocations need the CRT up,
which happens after.

A deferred second phase resolves it. `run_deferred_oracles()` is triggered from
the `scenario_opening_movie` call-site hook, the first startup site reached once
the CRT is running. Each deferred suite lifts the single redirect it needs with
`suspend_redirect_at()`, runs its fixtures against real `_malloc` blocks with
both sides owning separate allocations so each genuinely frees, then restores
the patch with `resume_redirect_at()`. The phase is one-shot, and it rewrites
the oracle result file so phase-one lines and deferred lines form one record;
if it never runs, the earlier record still stands rather than regressing.

`sprite-release`, `buffer-release`, and `basebutton-release` are the suites on
it. The first covers `Sprite::close`'s release branch, the second covers
`Buffer::close`'s real release loop, and the third covers both executable-CRT
free branches in `BaseButton::close`. The split is demonstrable: perturbing the
Sprite release accounting leaves the `sprite` phase-one suite passing while
`sprite-release` fails, because phase-one fixtures cannot reach that branch at
all.

A close that installs a real virtual table before walking cannot be driven
with a populated list: the walk dispatches through original game code rather
than the fixture's stand-in and crashes `DllMain`. Drive only non-walking
shapes there and cover the walk in a suite that leaves the table alone. Where
that leaves a constant unobservable - the derived close at `0x004066C0`
overwrites its own tables before returning - confirm it by reading the
instruction bytes and record that the oracle does not cover it.

Seed any original table a suite indexes before comparing. Game tables are
empty during `DllMain`, so a suite that only reads them compares zero against
zero and passes no matter what the index arithmetic does. The `spying` suite
seeds both the status table and the base faction bytes, then restores them;
without that, deliberately corrupting either stride still passed. Always
confirm a new suite fails when the recovery is perturbed, and treat a poison
that does not fail as evidence the fixtures are vacuous rather than as a
formality.

The same vacuous-seed failure mode also hides inside a fixture that *writes*
through a computed address rather than only reading a table. `verify_close`'s
synthetic vbtable displacement (`src/stringstruct_oracle.cpp`) pointed the
write at offset `0x14` - `current_position_`'s slot - which
`close_with_tables` unconditionally overwrites two lines later; the read of
`vbtable[1]` and the write it drives were unobservable regardless of whether
the displacement was computed correctly. The same fixture separately seeded
`current_position_` to zero before the call, so the explicit reset that clears
it was equally unobservable - a zeroed field can't show that it was zeroed.
Both were caught by `tools/mutate_and_verify.py` (against `recovery-leaf-tests`,
which has no coverage of `close` at all) and confirmed at the oracle level by
poisoning the real constants and watching `stringstruct` flip to `failed`
under the hybrid smoke target. Fixed by retargeting the write at an offset
`close_with_tables` never touches (`0x18`, `allocator_`) and seeding
`current_position_` with a nonzero sentinel (`0x77777777`) instead of zero.

Run `tools/mutate_and_verify.py <source>` to perform that check mechanically
instead of by hand. It derives source-level mutants of each `Original Offset:`
function - dropped stores, perturbed constants, inverted comparisons, swapped
dependent statements - rebuilds, and runs the owning test for each, requiring
every mutant to be killed. Read the three outcomes precisely:

- `killed` - the suite observed the perturbation. This is the only outcome
  that constitutes evidence.
- `SURVIVED` - a coverage hole. Some behaviour of the recovery is not observed
  by any assertion; either extend the fixtures or record the gap in a
  `Verification note:` comment as with the unobservable derived vtable stage.
- `no compile` - the mutant never built and therefore proves nothing. These
  are reported separately and never counted as kills.

The sweep measures one suite at a time - whatever `--target`/`--test` name,
defaulting to `recovery-leaf-tests`. A survivor is therefore unobserved *by
that suite*, which is not the same as unobserved by every gate: the in-process
runtime oracle runs under the hybrid smoke target and is not exercised by a
default sweep. Before recording a gap, check whether the other gate covers it.

A mutant that hangs the suite counts as killed; the harness times it out at
ten times the clean run rather than waiting on the build timeout. The tool
restores the source in a `finally` and on SIGINT/SIGTERM, so an interrupted
sweep does not leave a mutant on disk - but confirm `git status` is clean
before trusting a later build anyway.

For large Wine-backed sweeps, pass `--reuse-owned-wine-prefix`. The option
keeps only `run_windows_test.py`'s dedicated marker-protected build prefix
running between mutants, then performs one ordinary restored-source test to
stop that prefix. It does not change the prefix path, relax ownership checks,
or issue a global Wine shutdown. This removes repeated Wine teardown while
preserving the same executable and CTest selection for every mutant.

Swaps are only emitted where the two statements genuinely interact. Two stores
to distinct lvalues with constant right-hand sides are order-independent in
final state, so swapping them is an equivalent mutant that would survive any
possible suite; reporting those as holes trains you to ignore the output.

As of `2dd742b`, every recovered file has been swept and triaged: all
non-equivalent mutants are killed by the union of `recovery-leaf-tests`, the
`recovery-oracle` ctest (the copied-byte differential executable, registered
as a ctest precisely so the harness can measure it - pair
`--target recovery-oracle-tests` with `--test '^recovery-oracle$'`), and the
in-process runtime oracle under the hybrid smoke. Survivors that remain are
either documented equivalences (`Verification note:` comments at the
function), instrumentation scaffolding, or the known suite-scope gaps:
`Buffer`'s and `spying`'s bodies are observable only in the hybrid process,
so a leaf-suite sweep of src/buffer.cpp or src/spying_recovery.cpp reports
mass survival by design. New recoveries should be swept when their tests
land, not re-audited wholesale. The harness filters literals inside
`[X / 4]` indices whose mutation folds to the same index or to a division by
zero. The remaining known noise class is `Probe`-style dotted increment
targets that the swap-dependence check treats as opaque.

Add a deferred suite when a recovery's release path needs real allocations -
`Buffer::close`, `Win::close`, and the destructor chain all qualify. Keep
non-releasing coverage in phase one, which is cheaper and runs unconditionally.

## Next Steps

1. Replace the wrappers' temporary `Scroll::init` dependency at `0x006054D0` by recovering its remaining `GraphicWin::init`, `BaseButton::init`, and Win dependency closure; its shared RECT-construction helper is already source-owned.
2. Recover the Scroll input and button handlers at `0x006061E0` through `0x00606C43`.
3. Recover the three BaseButton instance colour setters at `0x00607360` through `0x006073E0`. Each is `if (win_parent_) { buffer_.sync_to_palette(*palette_global); buffer_.set_text_color*(c1, c2, c3, c4); }`, where `buffer_` is the GraphicWin member at `+0x444`; both callees are now source-owned. The palette global they read is at `0x009B8180`, which is *not* the already-bound `BufferPalette` at `0x009B8174` - it needs its own binding and classification row.
4. Keep pixel or accessibility-based UI automation limited to menu, new-game/load-game, and map-entry integration coverage.

## Relevant Files

- `src/alphanet.h`: verified `0x14A0` `AlphaNet` layout and lookup adapter declarations.
- `src/alphanet.cpp`: recovered four process-ID and identity lookup implementations.
- `src/dllmain.cpp`: transactional signature-checked redirects; 110 source recoveries plus the gameplay gate's active-turn, post-increment upkeep, and call-site hooks; direct and temporary-dependency signatures are preflighted before original-address runtime oracles execute.
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
- `src/basebutton.h`, `src/basebutton.cpp`, `src/flatbutton.h`, `src/flatbutton.cpp`: verified button layouts and source-owned close/destructor chain, including process defaults and executable-CRT ownership.
- `src/basebutton_oracle.h`, `src/basebutton_oracle.cpp`: phase-one original-address teardown differentials and the deferred real-allocation release suite.
- `src/win.h`, `src/win.cpp`: verified Win layout, `move`, recursive ancestor-chain `is_visible`, recursive wrapping `client_to_screen`, both paging setters, both `in_box` overloads, wrapping RECT construction, rectangle-center behavior, and adapters.
- `src/vector.h`, `src/vector.cpp`: verified `0xC` Vector layout, lifecycle, ordered x87 arithmetic, alias-sensitive scaling copies, and adapters.
- `src/scroll.h`, `src/scroll.cpp`: verified Scroll layout, five init wrappers with a classified temporary primary dependency, range/position state, style and sprite-triplet setters, thumb reset/computation, named-field RECT expansion, exact alias behavior, signed arithmetic, ordered volatile accesses, and adapters.
- `src/runtime_oracle.h`, `src/runtime_oracle.cpp`: shared descriptor-driven in-process differential oracle machinery and per-suite result reporting; see `docs/RUNTIME_ORACLE.md` for suite authoring.
- `src/scroll_oracle.h`, `src/scroll_oracle.cpp`: Scroll suite of the runtime oracle covering fifteen original Scroll methods that are ineligible for copied-byte oracle execution.
- `src/stringstruct_oracle.h`, `src/stringstruct_oracle.cpp`: StringStruct runtime-oracle suite driving the list walk through stand-in entry and payload objects with recording destructors; entry addresses are side-specific, so payload clearing and head position are compared rather than raw pointers.
- `src/buffer_oracle.h`, `src/buffer_oracle.cpp`: Buffer runtime-oracle suite driving the lock/release pair through a stand-in DirectDraw surface so no real video memory is touched.
- `src/graphicwin.h`, `src/graphicwin.cpp`: verified `0xA14` GraphicWin layout, destructor vtable installation/subobject delegation, recovered close/reset/release behavior, source-owned Buffer teardown, and classified Win close/destructor dependencies.
- `src/graphicwin_oracle.h`, `src/graphicwin_oracle.cpp`: original-address GraphicWin close suite with resource-free Win/Buffer dependencies, optional scalar deletion, return-residue comparison, and fixture-relative Win self-pointer validation.
- `src/sprite.h`, `src/sprite.cpp`: verified `0x2C` Sprite layout, ordered constructor stores, release accounting and field clearing, and the sprite memory and CRT free bindings. The release paths call the executable's free at a fixed address and the executable CRT heap is uninitialised during `DllMain`, so the runtime oracle covers only non-releasing fixtures; the accounting arithmetic and release ordering are verified at source level with a recording free.
- `src/win_oracle.h`, `src/win_oracle.cpp`, `src/sprite_oracle.h`, `src/sprite_oracle.cpp`: Win and Sprite runtime-oracle suites covering recursive visibility and coordinate translation plus the constructor's accounting side effect.
- `src/menu.h`, `src/menu.cpp`: verified Menu layout, callback setter, bounded ID lookup, and adapters.
- `src/pulldown.h`, `src/pulldown.cpp`: verified PullDown layout, six item-state mutators, unchecked selected-index accessor, and adapters.
- `src/autosound.h`, `src/autosound.cpp`: recovered `do_sound` hook.
- `src/maininterface.h`, `src/maininterface.cpp`: recovered null interface hooks.
- `docs/recovery-overrides.csv`: runtime-integrated `source_complete` overrides.
- `docs/recovery-redirects.csv`: committed address/kind catalog of every DllMain redirect and preflight dependency; `tools/generate_redirect_signatures.py` regenerates `src/redirect_signatures.h` from it against the canonical executable with a PRACX byte cross-check, and `verify-redirect-signatures` fails on any drift.
- `docs/recovery/functions.csv`: canonical 6,000-function inventory.
- `docs/recovery/priorities.csv`: currently regenerated to 5,018 candidates.
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
- `tools/static_recompile_pilot.py`, `tools/static_recompile_runtime.h`: local-only static basic-block lowering and minimal explicit x86 state semantics.
- `tools/test_static_recompile_pilot.py`, `tools/verify_static_recompile_pilot.py`: synthetic lowering, containment, deterministic provenance, original-body, fixed-address, relocation, and ABI checks.
- `tools/local_artifact.py`: shared nonsymlinked `.opensmacx/`/`build/` output ownership enforcement.
- `tools/smoke_hybrid_game.py`: non-destructive Wine launch, diagnostics, rendering smoke gate, and required local runtime-oracle result validation.
- `tools/movie_skip.py`: transactional PRACX movie-command override for owned launch tools.
- `tools/test_smoke_hybrid_game.py`: source-owned loader-context, diagnostics, prefix-ownership, and movie-skip tests.
- `tools/owned_wine_prefix.py`: marker-protected initialization and shutdown for the dedicated runtime-test prefix.
- `tools/runtime_process.py`: random executable aliases and exact owned-wrapper discovery/termination.
- `tools/run_gameplay_scenario.py`: deterministic scenario launcher, result validator, and owned-process cleanup.
- `tools/test_run_gameplay_scenario.py`: source-owned fixture, result, diagnostics, and process-alias tests.
- `tools/ghidra/DecompileFunction.java`: exact-entry decompiler used with the persistent project.
- `tools/batch_decompile.py`, `tools/ghidra/DecompileBatch.java`: one-invocation batch decompiler over an address list or priorities-catalog filters into the ignored `build/ghidra-decompile/` cache with a merged manifest; cached results are skipped on rerun and outputs are never committed.
- `tools/mutate_and_verify.py`: mutation harness that mechanises the poison check - derives dropped stores, per-occurrence perturbed constants, inverted comparisons and dependent-statement swaps from each `Original Offset:` function, filters equivalent or invalid divided-index mutants and ABI-only empty compiler barriers, rebuilds, and requires the named suite to kill every mutant; survivors are coverage holes and compile failures are counted as evidence of nothing. Its opt-in owned-prefix reuse avoids repeated Wine teardown and always finishes with the normal marker-protected cleanup path.
- `tools/ghidra/ExportInteriorReferences.java`: exports external references entering function interiors.
- `docs/recovery/ghidra-interior-references.csv`: committed 2,574-row interior-reference sidecar.
- `docs/LEGACY_ISLANDS.md`: ownership, eligibility, lifecycle, and zero-island release rules.
- `docs/HYBRID.md`: local hybrid workflow.
- `tests/recovery_oracle_tests.cpp`: source-versus-original AlphaNet, Random, Win/geometry, Vector, Scroll, Menu, and PullDown fixtures.
- `tests/static_recompile_pilot_tests.cpp`: generated-only and original-byte differential fixtures for the time-boxed static recompilation leaf.
- `docs/STATIC_RECOMPILATION.md`: local-only provenance policy, commands, value gate, and stopped pilot outcome.
- `CMakeLists.txt`: source list, hybrid targets, legacy-island targets, and local differential-oracle target.
- `build/ghidra-projects/live-recovery`: ignored persistent Ghidra project.
- `build/mingw-i686-release/legacy-leaves/manifest.json`: current ignored 114-island manifest.
- `build/mingw-i686-release/recovery-oracles/manifest.json`: ignored explicit 32-function AlphaNet/Random/Win/geometry/Vector/Scroll/Menu/PullDown oracle manifest.
- `build/mingw-i686-release/legacy-leaves.obj`: ignored local i386 COFF object.
- `.opensmacx/game/terranx.exe`: ignored hash-pinned PRACX runtime executable used by hybrid staging.
- `.opensmacx/game/terranx_original.exe`: ignored pre-PRACX executable retained as an analysis input.
- `build/<preset>/staged-game/`: ignored per-preset mirror of the master game data holding the staged `terranx_hybrid.exe` and `OpenSMACX.dll`; `.opensmacx/game` stays read-only during batches so both presets can stage and smoke concurrently.

## Current Blocker

There is no tooling blocker. The gameplay gates verify movement-order issuance, resolved adjacent movement, end-turn request state, and the next turn/year increment before later upkeep. Do not force native DDrawCompat on Wine Staging 11.10, and do not terminate pre-existing Wine/game processes to make room for a test.

### MSVC SEH frames are present but unreachable

Every destructor and `close` method opens an MSVC structured-exception frame before doing any work, for example `GraphicWin::~GraphicWin` at `0x005D4DD0`:

```
push -1
push 0x662B2A                 ; per-function __ehhandler thunk in the original .text
mov  eax, fs:[0]
push eax
mov  fs:[0], esp              ; register the frame
```

Each pushed thunk loads a FuncInfo pointer and jumps to `__CxxFrameHandler` at `0x00644FD6`; 387 such thunks exist and the unwind funclets follow each thunk inline.

**These frames can never execute.** The canonical executable contains no C++ throw entry point: there is no `_CxxThrowException`, `operator new` at `0x0064558A` is a 14-byte `_nh_malloc(size, 1)` forwarder that returns null instead of raising, `operator delete` at `0x0064557F` forwards to `free`, and all six occurrences of the C++ exception magic `0xE06D7363` sit inside CRT handling code (`__InternalCxxFrameHandler`, `FindHandler`, `__FrameUnwindToState`, `ExFilterRethrow`, `__CxxUnhandledExceptionFilter`) with no game callers. VC6 emitted the frames because exception handling was enabled; nothing reaches them.

Recovered replacements therefore omit the frame, which is behaviourally equivalent rather than a compromise: the only observer of its absence is a C++ throw that cannot occur. Two guardrails hold that invariant:

- The DLL and both test executables compile with `-fno-exceptions`, so no recovered body can raise where the original could not.
- `verify-recovery-abi` fails any recovered object that grows an `.eh_frame` section or imports `__gxx_personality`/`_Unwind_`/`CxxFrameHandler`, which is what a translation unit losing the flag would look like.

Caveat: of 1,072 SEH prologues only 387 are C++ EH thunks. The remaining ~685 are `_except_handler3`-style `__try`/`__except`/`__finally`, which respond to *structured* exceptions such as access violations, and those genuinely can occur. The reasoning above does not transfer to them. Before recovering any function whose prologue targets a handler other than `0x00644FD6`, re-examine it.
