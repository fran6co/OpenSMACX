# The recovery loop, and every tool in it

`tools/` holds around 140 scripts and until this file existed nobody could say
which of them the project still runs. This is the answer, and it is CHECKED:
`tools/verify_tool_reachability.py` fails when a tool named here does not
exist, and fails when a tool exists that nothing — this file, `CMakeLists.txt`,
`.claude/agents/`, `.claude/commands/` — tells anyone to run. Keeping a tool means placing it below.
Deleting one means striking its line. Neither can drift.

---

## The loop

Four questions, in order. Everything else feeds one of them.

### 1. What is left? — `tools/decomp_status.py`

The single arbiter. It parses the `ORIGINAL: 0x...` annotations out of `src/`
and asks the original compiler what state each body is in. There is no second
opinion and no committed ledger; see `DECOMP_MAP.md` for the grammar.

```sh
tools/decomp_status.py                    # every state, drift, verdicts
tools/decomp_status.py --state-only       # the map alone, no VC6 needed
tools/decomp_status.py --gaps             # image bytes in no catalogued span
tools/decomp_status.py --work 0x00405C20  # scaffold one function to work on
tools/decomp_status.py --check            # THE RATCHET
```

`tools/annotation_scan.py` is the reader underneath it, and
`tools/project_catalogue.py` is what makes `src/` the store rather than a view:
every annotation carries its own name, size, spans, prototype, kind, flags and
call edges, and `--check` holds them to the export row by row.

### 2. What shape is this function? — read the image

```sh
tools/disasm.py 0x00405C20 --length 120   # the bytes, always the first move
tools/agent_brief.py 0x00405C20           # everything known, in one message
```

`agent_brief.py` is the one to reach for. It assembles every reading below
into a single document, and the sections that pay are at the END of it.

Three of them answer questions the disassembly alone cannot, and each exists
because agents were measured guessing the answer:

```sh
tools/jump_tables.py 0x00490B00      # the case targets behind a computed jump
tools/frame_objects.py 0x004D61A0    # which C++ objects the frame holds, and
                                     # how big the original's frame is
tools/member_map.py InfoWin          # where this class's fields are, read off
                                     # its own [ecx + N] accesses
```

A switch's targets are DATA and appear in no disassembly; three agents in one
batch guessed a case order and all three said so. A `Popup` local carries a
`Scroll`, two `FlatButton`, a `Spot`, a `Sprite`, a `Heap` and a `Dialogs`
region, and from the disassembly each looks like an independent local —
declaring them separately gets the frame wrong at instruction zero. And a
header's caveat about `sizeof` is not a caveat about offsets: two functions
were deferred in one batch against headers that named every offset they
needed, which is what `member_map` now settles.

The catalogue's own claims are not trusted where the bytes disagree:
`tools/catalogue_corrections.py` records the mangled names IDA reconstructed
wrongly and the instruction that says so, and `tools/recovery_symbols.py` is
the one place that decides what symbol a recovered function carries.
`tools/recover_conventions.py` and `tools/derive_prototypes_from_names.py` read
the convention and the prototype the mangled name already states; and
`tools/indirect_call_sites.py` reports the call sites the call graph cannot
see.

### 3. What is the class? — the layout half

This is the half that gates everything else: a method cannot be replaced until
its field offsets are known. Three independent readings, none a model.

```sh
tools/verify_member_offsets.py --check    # declared boundaries vs access widths
tools/verify_member_offsets.py --pins     # which pinned sizes the image backs
tools/verify_member_offsets.py --check-names   # every field_<HEX>_ IS at that
                                          # offset, against the compiler. The
                                          # brief tells agents to trust the
                                          # naming convention; this is what
                                          # makes that safe to say, and it
                                          # found six names that lied.
tools/derive_array_strides.py             # a struct size read off the code
                                          # that indexes a global array
tools/derive_class_vtables.py             # which vtable belongs to which class
the retired `verify_subobjects`                # what a ctor/dtor builds, and where
the retired `split_proved_fields` --apply      # name the fields inside a slab
```

The evidence they rest on is derived from the image and published under
`docs/recovery/`: `tools/derive_class_layout.py` (sizes the image states
exactly), `tools/derive_access_bounds.py` (how far a class's own code reaches —
a floor), `tools/derive_class_size_bounds.py` (the next global — a ceiling),
`tools/derive_agreed_sizes.py` (where two independent sources meet),
`tools/derive_base_edges.py` (inheritance, stated by one source and checked
against another), the retired `derive_vtables` (where the vtable pointers live).

Outside opinions arrive through `tools/export_idb_members.py` (the IDA
database, read without IDA), `tools/correlate_thinker_layouts.py`,
the retired `correlate_pracx_layouts`, `tools/correlate_yitzi_notes.py`,
the retired `run_ghidra_analysis`,
the retired `correlate_external_analysis` and `tools/fetch_external_analysis.py`.
Every one of them is a hypothesis until the image agrees.

Layouts are then written and checked: `tools/emit_hypothesis_layouts.py`
declares the classes `src/` does not, the retired `extend_short_classes` grows a
class the image proves is bigger, `tools/verify_class_layouts.py` records which
extracted layouts survive a compile, the retired `promote_agreed_sizes` turns an
agreed size into an assertion. Names come from
the retired `name_members_from_behaviour` (the method that exists to touch the
member) and the retired `name_members_from_sources` (what the outside sources call
it). `tools/classify_casts.py` reports every `this`-relative cast still in
`src/`, which is the layout gap showing up in source form, and
the retired `structure_observations` records what a recovery turned up on the way.

### 4. Does it reproduce? — the search over source form

Semantics get decided early; the rest is finding which of several equivalent
spellings VC6 lowers the way the original was lowered.

```sh
tools/verify_recovered_function.py 0x00405C20 --dir /tmp/variants   # many, ranked
tools/verify_recovered_function.py 0x00405C20 --body candidate.cpp  # one
```

One command, two modes; exit 0 means BYTE_EXACT and nothing else does, so it
works directly as a loop condition. `--dir` is the one to reach for — it loads
the image once and answers "which of these nine" instead of asking nine times.

`tools/byte_match.py` is the comparison itself, `tools/byte_match_census.py`
recompiles everything already recovered, the retired `byte_match_fanout` prepares
and scores batches, and `tools/emit_translation_unit.py` builds the standalone
unit each is measured in — with declarations from `tools/class_layouts.py`,
`tools/src_declarations.py` and `tools/emit_vtable_shim_header.py`.
the retired `msvc6_byte_match` is the original five-function spike the route grew
out of, kept because `BYTE_MATCH_ROUTE.md` reports its numbers.
the retired `x86_lower`, the retired `x86_smt` and `tools/generator_support.py` are
shared machinery underneath.

A LANDED UNIT FREEZES ITS SCAFFOLDING, so an emitter fix reaches only the
units written after it. `tools/refresh_file_units.py --apply` re-scaffolds the
landed FILE-mode units while keeping their bodies, ratchets by tier so a unit
can only improve, and reverts anything that regressed. One pass banked 20
BYTE_EXACT with no agent time at all; run it after any emitter change.

Landing a match: the retired `harvest_proven_units` and
the retired `preserve_worked_units` rescue proved work out of the build tree,
the retired `integrate_recovery` moves it into a compiled file,
`tools/repair_source_locations.py` re-points the catalogue at it, and
the retired `build_regressions` records the bodies that gave up byte-exactness so
the tree would compile.

---

## The recovery loop, run as subagents

`/recover-batch` (`.claude/commands/recover-batch.md`) is the coordinator and
`.claude/agents/byte-match-recovery.md` is the worker. Six steps: select with
`tools/recovery_frontier.py --max-size`, prepare with `decomp_status.py --work`,
brief with `tools/agent_brief.py`, fan out at most six subagents, collect with
`decomp_status.py --addresses ... --record-matches`, gate with `--check`.

Two properties do the work. **The coordinator re-measures everything** — an
agent's report is a claim about a run nobody observed, produced by the process
that had an incentive to stop. And **subagents run only the scorer and
`disasm.py`**: `decomp_status.py` writes an unlocked shared ledger, so it is the
coordinator's serial step alone. One address is one file, so no two agents can
collide.

Both files are reachability roots, on the same rule as the documentation: they
tell someone to run a tool, and the someone being an agent changes nothing.

## What the retired harness left behind

The external Node harness is gone (see `docs/RETIRED_ROUTES.md`); five of its
tools were never really its own and carry the loop today.
the retired `mizuchi_context` emits the scaffolding-only unit,
`tools/emit_target_object.py` produces the COFF target,
`tools/disassembly.py` holds the disassembler and the definition-head
builder that `tools/agent_brief.py` now imports, `tools/declfix.py`
respells callee declarations so VC6 re-mangles them to the catalogued names,
and `tools/writeback.py` lands a byte-exact match in a tracked file or
refuses and changes nothing. The names are historical; the code is not.

## Wiring a recovery into the DLL — every tool here is retired

Nothing in this section runs. The DLL, the redirects and the module definition
`src/OpenSMACX.def` are all retired (`docs/RETIRED_ROUTES.md`); the section is
kept because it says what the route was made of.

the retired `add_redirect` wired one in a single verified step.
the retired `generate_redirect_signatures` derived the DllMain signature header,
the retired `verify_redirect_patch_fit` checked the jump had room,
the retired `generate_vc6_exports` and the retired `generate_mingw_exports` emitted
the module definition, the retired `verify_def_append_only` refused to drop an
export name, and the retired `generate_imports` derived the import shim table.
Signedness was audited by the retired `audit_export_signedness` and
the retired `audit_recovered_signatures`; the applier that acted on their findings
went first, so a disagreement was reported and fixed by hand before the rest
followed the `.def` out on 2026-08-13.

## Running it

the retired `run_gate` runs the recovery gate in both presets.
the retired `host_doctor` says whether this machine can run the harness at all.
The game side: the retired `setup_game`, the retired `stage_preset_game_dir`,
the retired `prepare_hybrid_image`, the retired `assemble_hybrid_image`,
the retired `opensmac_patcher`, the retired `movie_skip`, the retired `run_game`,
the retired `run_gameplay_scenario`, the retired `smoke_hybrid_game`.
Wine plumbing: `tools/vc6_cl.py` (VC6 as a CMake compiler from Linux),
`tools/owned_wine_prefix.py`, `tools/wine_runtime.py`,
`tools/run_windows_test.py`, the retired `runtime_process`,
`tools/local_artifact.py`.
Oracles: the retired `generate_signature_oracles`,
the retired `extract_legacy_leaves`,
the retired `export_recovery_inventory`, the retired `measure_observability`,
the retired `mutate_and_verify`.

## Static recompilation — research, not the route

the retired `lift_whole_image` emits every game function as C++;
the retired `lifted_boot_loop` drives the boot and reports whether the stop
address moved; the retired `lifted_oracle_plan`,
the retired `lifted_oracle_compare`, the retired `lifted_oracle_summary` and
the retired `lifted_oracle_why_not_full` measure what the differential oracle can
say; the retired `lifted_oracle_mutate` proves it has teeth.
the retired `static_recompile_pilot` and
the retired `verify_static_recompile_pilot` are the pilot and its gate.
The SMT side asks whether a recovered body is PROVABLY equivalent:
the retired `prove_recovered_equivalence`, with the encoding checked against
`lifted_x86.h` by the retired `validate_x86_smt`, against the processor one
instruction at a time by the retired `validate_x86_silicon`, and a whole block at
a time — memory included — by the retired `validate_x86_block`.

## The gates

What `ctest` enforces, beyond `decomp_status.py --check`:

- `tools/verify_member_offsets.py` — no declared member boundary the image
  contradicts
- `tools/verify_class_layouts.py` — which extracted layouts compile to the
  real size
- `tools/verify_recovery_abi.py`, `tools/verify_recovery_pipeline.py` — the
  analysis pipeline says the same thing twice running
- the retired `verify_no_load_time_addresses` — no fixed game address
  dereferenced at load time
- the retired `verify_observability_ratchet` — a new recovery must prove
  something observes it
- `tools/verify_build_freshness.py` — a green ctest against a stale build
  directory means nothing
- `tools/verify_tests_do_not_write.py` — runs every tool test in its own
  subprocess and fails if one edits or deletes a tracked file. It caught
  the since-retired `test_integrate_recovery` deleting a byte-exact proof and
  rewriting the catalogue on every run, and the second test that was passing
  *because* of it
- `tools/verify_test_registration.py`,
  `tools/verify_tool_test_registration.py`,
  `tools/verify_tool_reachability.py`,
  `tools/verify_tests_all_run.py` — no test file defines a test class after its
  `unittest.main()`. One did, and the gate invoking it ran 57 of 73 tests while
  exiting 0. Neither registration gate could see it: the file WAS registered and
  CMake DID run it,
  `tools/verify_cmake_paths_exist.py` — every literal
  `${CMAKE_CURRENT_SOURCE_DIR}/...` names something that exists. A retired
  route's CMake lines outlive it silently, because a target outside `all` is
  never built and so never resolves its own COMMAND,
  `tools/verify_documented_counts.py` — the checks that watch the checks
- `tools/verify_span_termination.py` — no catalogued span stops mid-instruction
  or holds two functions back to back. Nine rows were truncated this way, and
  nothing pointed back at it: the agent is simply told its last instruction
  diverges
- `tools/verify_call_edges.py` — no header drops a call target its own bytes make
- `tools/verify_return_agreement.py` — no name spells `void` over a prototype
  that returns a status
- `tools/verify_vendor_zlib.py` — a vendored source is the upstream one, byte
  for byte
- `tools/verify_void_returns.py` — a review aid rather than a gate: names that
  say `void` over bodies that return a constant. It reads the CALLEE only, and
  four agents have now found returns that only the caller reveals, so it is
  known-incomplete by measurement
- `tools/verify_checks_can_fail.py` — damages real inputs and requires each
  gate to go red
- `tools/verify_check_tests_observe.py` — mutates each check and requires its
  OWN tests to notice. The one diagnostic that found vacuous tests three audits
  in a row, and the only instrument here that can say whether a test is worth
  keeping. It was deleted on 2026-08-13 as an orphan and restored the same day:
  it is reachable from nothing because it is a diagnostic you run deliberately,
  which is a reason to keep it, not evidence that it is dead.
  OWN tests to notice. The one diagnostic that found vacuous tests three
  audits in a row.

Counting is centralised so two numbers cannot disagree:
`tools/recovery_metrics.py` holds the one byte-weighted denominator,
`tools/recovery_frontier.py` says what is reachable next, and
`tools/measure_exclusions.py` measures the populations `EXCLUSIONS.md`
declares out of scope.
