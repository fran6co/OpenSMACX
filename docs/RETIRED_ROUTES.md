# Retired routes, and what they were for

Byte-matching is the project's only live verification route: write a body, ask
VC6 12.00.8168, compare the bytes. Everything below was built to answer the same
question a different way, and is retired rather than deleted from memory.

**This file is a reachability root.** `tools/verify_tool_reachability.py` globs
`docs/*.md`, so a tool named here stays reachable and a tool whose line is struck
must be deleted in the same commit. That is the graded retirement path: keeping
an idea costs one paragraph, and dropping one is a visible edit.

Nothing here is a recommendation to rebuild it as it was. Each section ends with
the measurement that priced it, because the numbers are the part that does not
survive re-derivation.

---

## The direct-source leaf suite — retired 2026-08-12

`tests/leaf/` was 20 family files, 229 registered cases and 32,001 lines. It
linked an explicit closure of `src/` translation units and exercised recovered
functions directly, with file-scope stand-ins for `map.cpp` and `general.cpp`
symbols — a scriptable `mem_get`, call-recording `lock_map`/`unlock_map`,
`xrange`, `purge_spaces` — that its own fixtures asserted against.

**Why it went.** Byte equality subsumes behavioural equivalence: for any body
that reaches `BYTE_EXACT`, a behavioural test over the same function adds no
evidence. The leaf suite was built for *small* functions — `find_leaf_testable.py`
framed it as "which functions are cheap to VERIFY, not cheap to recover" — and
small functions are exactly the population byte-matching settles. Measured
against `docs/recovery/observability.json`, of the 63 recovered functions of
≥200 B that behavioural testing observes at all, **`recovery-leaf-tests` was the
only possible observer for 9**; `recovery-gameplay-tests` covered 37 alone and
17 jointly. Keeping it alive under VC6 required a C++98 port of a C++11 harness
(`<array>` was only the first error) — a project, spent on 9 functions.

**What that cost, named, because a silent deletion would be the dishonest form.**
These nine now have no observer at all, and byte-matching says nothing about any
of them:

| address | tier | size | function |
|---|---|---:|---|
| `0x00406910` | NO_COMPILE | 590 B | `??1Dialogs@@QAE@XZ` |
| `0x004456B0` | NO_COMPILE | 270 B | `?popup_wave_callback@@YAXPAUPopupWave@@H@Z` |
| `0x004C6120` | SHARED_TAIL | 205 B | `??1Sound@@QAE@XZ` |
| `0x004C6280` | NO_COMPILE | 225 B | `?load@Sound@@QAEHPBD@Z` |
| `0x004C66E0` | SHARED_TAIL | 239 B | `??0Wave@@QAE@XZ` |
| `0x004C67C0` | SHARED_TAIL | 333 B | `??1Wave@@QAE@XZ` |
| `0x004C69B0` | NO_COMPILE | 293 B | `?init@Wave@@QAEXPADK@Z` |
| `0x00590300` | NO_COMPILE | 361 B | `?lock@Lock@@QAEHHHHHHHH@Z` |
| `0x00608E10` | NO_COMPILE | 351 B | `??1Dialog@@QAE@XZ` |

A smaller loss is unmeasured and should be stated as unmeasured: the census only
covers functions ≥200 B, so whatever the suite uniquely evidenced *below* that
size was never counted. The measurement that would have priced it — of the 229
leaf subjects, how many are not `BYTE_EXACT` — was not run before removal.

### Getting that value back, which is mostly a worklist rather than a loss

Six of the nine are `NO_COMPILE`, and none of them for an interesting reason.
The scaffolding will not build them, and the specific defect is named in each
case:

| address | what actually blocks it |
|---|---|
| `0x00406910` | `C2061: syntax error : identifier 'uintptr_t'` — a shim reaching the unit |
| `0x004456B0` | `C2027: use of undefined type 'PopupWave'` — an opaque shell where a layout is needed |
| `0x004C6280` | `C2664: 'filefind_get' : cannot convert parameter 1` — a callee signature |
| `0x004C69B0` | `C2440: cannot convert from 'int' to 'char *'` — a signature |
| `0x00590300` | `C2065: 'field_E0_' : undeclared identifier` — a `Lock` layout gap |
| `0x00608E10` | `C2039: 'destroy' : is not a member of 'Dialog'` — a missing header declaration |

Every one is a scaffolding or class-layout defect, not a property of the
function. Fix it and the body becomes byte-matchable, at which point the
evidence is **stronger** than the leaf test ever was rather than merely
replaced. `C2065` is the largest such family in the tree, so this is work
already on the route rather than work invented to compensate.

*Corrected 2026-08-12:* an earlier draft of this paragraph called `C2065`
"951 of 1,703 NO_COMPILE rows" and attributed it to missing class fields. Both
halves were wrong. Measured across every `NO_COMPILE` note in the map, there are
**2,865** `C2065` occurrences, of which **2,455 are game constants and globals**
(`GameAtexit` 388, `ScrollOperatorDelete` 56, `RadiusOffsetX/Y`, `TRIAD_AIR`,
`MaxPlayerNum`, `BIT_FUNGUS`, even `NULL`) and only **79** are `field_*`
members. The scaffolding declares what the decoded signature reaches and no
more, so a body written against the real headers loses everything project-wide.
Of the six above, exactly one — `0x00590300` — is really a layout gap.

The remaining three — `0x004C6120`, `0x004C66E0`, `0x004C67C0`, the `Sound` and
`Wave` constructors and destructors — are `SHARED_TAIL`: the linker folded their
tails onto another catalogued function under `/Gy`, so no per-function verdict
is well defined and **byte-matching is structurally incapable of ever speaking
about them**. Behavioural evidence is the only kind that exists for these, and
the retired `generate_signature_oracles` with the differential
`recovery-oracle` is where it belongs. `0x00590300` already appears in the
oracle sources, so the route is not hypothetical.

So the honest summary of the nine: six are a byte-match worklist, three need an
oracle, and none needs a 32,001-line C++11 harness ported to a 1998 compiler.

### The ideas worth keeping

**Append-only case registration.** A new case was added by appending
`LEAF_CASE(LEAF_APPEND, test_*);` to the family file that owned the subject —
`LEAF_APPEND` bare, never `LEAF_APPEND + n`. Appended cases ran after the
baseline and tie-broke by name, so no number had to be coordinated between
agents and two recoveries in two families shared zero files. This is a genuinely
good answer to parallel work and is worth reaching for again, independently of
the suite it served.

**Counting the manifest instead of trusting it.** Per-family case counts were
derived at build time by the retired `generate_leaf_manifest` (counting
`^LEAF_CASE(` lines) and asserted again at run time, so a case that stopped
registering made the binary refuse to run rather than quietly shrink. The same
shape as this tree's other "a check that cannot fail is worth nothing" controls.

**Splitting by link closure, not by subject.** The gameplay suite exists because
the leaf binary's stand-ins collide with the real `map.cpp`/`general.cpp`
translation units. Splitting on what links rather than on what is being tested
gave ~1,800 gameplay free functions a home without disturbing the existing
fixtures.

**Reverse-order execution as a control.** The suite ran a second time as
`recovery-leaf-tests-reversed`, which is what catches a case that only passes
because an earlier one left state behind.

### What was already known to be weak

Worth recording so it is not rebuilt on optimism. `docs/RECOVERY_ECONOMICS.md`
measured that most recovered code is watched by nothing: 5 of 8 sampled
recovered functions ≥200 B were killed by no mutant at all, and a full sweep
killed 102 of 1,217 mutants — 8.4%. A suite with that kill rate is a smoke test,
not a proof, and it was competing against a route that produces exact evidence.

---

## Seven gates, retired 2026-08-12 — the tools stay, the enforcement goes

These were audited one at a time against a single question: does this contribute
to the STATUS of the recoveries, to the byte-match LOOP, or to BUILDING the
project? Twenty-eight gates answered yes. Seven did not, and their `add_test`
registrations are gone. **The tools and the CSVs they guard are untouched** —
they remain runnable on demand and named in `docs/TOOLS.md`. What is retired is
the claim that ctest must keep their output current.

Four further retire-verdicts were **overturned** by an adversarial pass and are
still enforced: `wine-test-lock-check` (the assessor priced only the
`RESOURCE_LOCK` half and missed the `--expect-at-least` floor, which refuses when
a Wine-backed suite silently loses its lock), `class-size-bounds-current` (its
`main()` runs a control unconditionally before the comparison, so the check does
more than its docstring claims), `yitzi-sizes-current` (the ratchet is *not* a
second catcher of a drifted pinned size, which the retire case assumed), and
`def-append-only` (it guards the join key of a surviving status ratchet, not only
the retired hybrid's import table). Recording that four of fourteen were wrong is
the useful part: "I cannot see what this catches" was not evidence.

| gate | why it went |
|---|---|
| `vtables-current` | keeps `vtables.csv` current; its readers are the layout hypothesis path, which reads it on demand |
| `subobject-sites-current` | the tool has no `--check` at all, so the registration compared nothing — it was a worklist wearing a gate's clothes |
| `pracx-members-current` | PRACX is a hypothesis source, barred from size *agreement* by the shared-lineage rule; currency of a hypothesis is not a project invariant |
| `behaviour-member-names-current` | 18 member names inferred from their sole accessor; once written into headers the file is a no-op |
| `recovered-shapes-current` | the shapes only annotate the *differential-oracle* debt via `recovery_metrics.unproven_by_shape`; `load_shapes()` returns `{}` when absent and the headline denominator never reads it |
| `signature-oracles-current` | regenerates the hybrid runtime's oracle harness — and **taxed the loop**: inserting a body *above* a function it verifies, in the same file, without touching that function, staled the provenance comment and failed the gate in both lanes |
| `observability-ratchet` | **the heaviest tax of the seven.** It made every new recovery of ≥200 B fail until someone ran `measure_observability.py` on it — one build per function — and it came back OBSERVED. Payable only in the currency of the route being retired, and satisfiable by editing a JSON string. Its own tool states the limit: "editing a verdict from UNOBSERVED to OBSERVED would satisfy this check while proving nothing." |

Its damage case and `COVERED_CHECKS` entry were struck from
`verify_checks_can_fail.py` in the same commit — a coverage requirement that
names a check which no longer exists fails the run, which is the gate working.

`src/generated_signature_oracle.cpp` and `src/globals_diff.h` **stay**: both are
in the DLL source list, and deleting either breaks the build. Only enforcement of
their currency went.

## Two exclusions retired, and 48.4% of the image came back — 2026-08-12

`docs/EXCLUSIONS.md` §4 put 941 functions and 1,187,932 bytes — **48.4% of
catalogued bytes**, the largest claim in the repository — out of scope, and §6
added `NetDaemon`. Both were excluded on *verifiability*, and §4 said so in a
sentence that decides the question once the route changes:

> "These functions are **recoverable as source and unverifiable by any
> differential oracle in this repository**."

That is an exclusion from the ORACLE route. Byte-matching needs no oracle: the
verification *is* the comparison. So the ground did not survive the route change,
and it was never acted on anyway — only `EXCLUDED S1` and `EXCLUDED S2a` appear
in `src/`, so those 941 functions were sitting in the placeholder population
being counted as remaining work the whole time.

**Measured before moving anything.** Twenty DirectX-reaching functions were taken
off the recoverable frontier at ≤200 B and their emitted skeletons scored through
`byte_match.match_function`: **20 of 20 returned a real MISMATCH**, zero REFUSED,
zero SHARED_TAIL, zero NO_COMPILE. Every divergence was at instruction #0, which
is the shape of an empty body and what any unrecovered function gives. There is
no structural obstacle — the scaffolding emits, the unit compiles, the comparison
speaks.

The measured block stays and still passes `measure_exclusions.py --check` (20 of
20 figures agree with the image). Those numbers are a true measurement of what
the import closure reaches; what retired is the claim that reaching it puts a
function out of scope.

## The Mizuchi harness — retired 2026-08-12

An external Node program (`~/code/mizuchi`), driven by `mizuchi.yaml`,
`mizuchi-integrator.mjs` and `tools/run_mizuchi.sh`, running one Claude Agent SDK
session per function — model `claude-sonnet-5`, 10 concurrent, 25 retries, with
an `objdiff` tool in the loop. Last real run 2026-08-06, 26,906 events. Replaced
by `/recover-batch` and `.claude/agents/byte-match-recovery.md`, which do the same
thing inside the session that already has the repository.

Gone with it: `mizuchi-db.json`, **161 MB** carried through git-lfs, and
the `emit_asm_dumps` tool, which nothing but `mizuchi.yaml` named.

**Five of its tools were never really its own** and carry the loop today —
`mizuchi_context.py`, `mizuchi_declfix.py`, `mizuchi_writeback.py`,
`emit_target_object.py` and `emit_mizuchi_prompts.py`, the last of which now
supplies `agent_brief.py` with its disassembler and definition-head builder. The
names are historical; the code is load-bearing.

### What it got right, and should be copied

- **One agent per function, in its own context.** No cross-talk, no shared
  scratch, and a failure that cannot spread. The subagent loop keeps this.
- **A diff tool inside the loop.** The agent could compile and see its own
  divergence rather than submitting blind. `--dir` is the successor: score nine
  spellings against one loaded image and rank them.
- **The gate did not trust the agent.** Only a BYTE_EXACT verdict measured by
  the integrator was integrated; the agent's own claim was never evidence. That
  rule is now `/recover-batch` step 5, and it is the single most important thing
  carried over.

### What it got wrong

- **No channel between agents.** Its own findings file said so: "Mizuchi runs one
  agent per function with no channel between them, so two agents hitting the same
  idiom each pay for it separately." That file then had zero readers for its
  entire life. The `LEVER:`/`RULED-OUT:` grammar exists because of this.
- **The evidence left the repository.** The integrator ratcheted
  `docs/recovery/byte-match.csv`, which had already moved to a gitignored path,
  so a run's proof lived only in a cache. 122 proved bodies once existed solely
  as gitignored `build/byte-match/*/unit.cpp`.
- **A branch mode that lost work.** One 27-minute run left 42 dangling refs and
  42 `/tmp` worktrees with nothing in `git status`.

## The bulk generators — retired 2026-08-12, 9,262 lines

Eleven tools that each found a family of functions sharing one shape and wrote
every member of it at once: `generate_adjustor_thunks`, `generate_atexit_thunks`,
`generate_delegation_thunks`, `generate_deleting_thunks`,
`generate_guarded_teardowns`, `generate_init_thunks`, `generate_nullsub_thunks`,
`generate_global_arith`, `generate_field_accessors`, `bulk_recover_stubs`, and
`find_constant_returns` — the scan that found the families.

**They were already dead, and dangerous while alive.** Measured 2026-07-30, every
one returns zero new candidates: the families are exhausted. And they are **not
idempotent** — `docs/TOOLS.md` carried the warning "re-running
`generate_deleting_thunks.py` against today's `src/` would silently drop 91
committed recoveries. Read them, do not run them." A tool nobody may run, which
finds nothing when run, is not tooling. What it was is a record, and the record
is the part worth keeping:

- **The families themselves are the finding.** Recovery is not uniformly hard;
  large parts of an image are one shape repeated. Adjustor thunks, atexit
  registrations, deleting destructors, nullsubs, field accessors, guarded
  teardowns and global arithmetic each fell to one template applied N times.
  Looking for the next such family is cheaper than recovering N functions.
- **Mangled names can be actively misleading, and the bytes win.** 47 Family A
  thunks are spelled `??3X@@SAXPAXI@Z` — a caller-cleaned `operator delete` —
  while the bytes take the receiver in `ecx` and the callee pops four. Seven more
  carry no name at all. A name-driven scan misses exactly the rows a byte-driven
  one finds.
- **Decode every value twice.** The deleting-thunk generator decoded each field
  through raw ModRM *and* through Capstone and required agreement, because an
  off-by-one on the guarded call would have named `0x0064557E`, the tail of
  `__alloca_probe`, instead of `operator delete`.
- **Keep literals inline rather than hoisted into a table**, so a mutation sweep
  has something in the body to perturb. Hoisting would have left it with nothing
  to find.
- **Track registers by offset from `this`, not as aliases.** `lea ecx,[eax+0x30c]`
  makes a later `mov byte [ecx],dl` write `0x30c`; calling ECX "an alias of this"
  would have written offset 0 — a real field, a plausible value, and wrong.

The 91+ bodies they wrote are in `src/` and are covered by the ratchet like any
other. Nothing they produced is lost by deleting the producers.

## Static recompilation and the SMT equivalence work — retired 2026-08-12

Fifteen tools and ~10,900 lines: the whole-image lifter, the differential lifted
oracle and its plan/compare/summary/mutate chain, the static-recompile pilot and
its gate, the Z3 x86 encoding (`x86_smt`, `x86_lower`) and the three validators
that checked it against `lifted_x86.h`, against silicon one instruction at a
time, and a block at a time with memory included.

`docs/TOOLS.md` already labelled the whole area **"research, not the route"**.
The cluster imported only itself, so it came out whole.

### What it established, which is the reason it existed

- **The differential oracle has a ceiling, and it is about half the image.**
  ~49% of bytes are testable with COM skipped; the rest is reachable only by
  booting the lifted executable and comparing observable behaviour. That number
  is what put 941 functions out of scope in `EXCLUSIONS.md` §4 — an exclusion
  this project has now retired, because byte-matching needs no oracle.
- **Read a specification, do not execute it.** SLEIGH's `fistp` emits
  `trunc(round(ST0))` and never reads the control word — wrong on 100% of the 69
  `fistp` sites. A pypcode/SLEIGH interpreter was rejected on that.
- **Undefined flags must be masked.** After `IDIV`/`DIV`/`MUL`/`IMUL` and any
  shift with count ≠ 1, the flags are undefined; dropping the `undef=<hex>` mask
  costs 3 FAILs on native silicon even though the reason it was introduced no
  longer applies.
- **The harness reads its own memory, and it looks exactly like a lowering bug.**
  Five "lowering bugs" were that. The diagnostic: rebuild with
  `-DORACLE_LAYOUT_SHIM=0x51000` — if the detail string moves, it is the
  harness. Rebuilding at a different `--image-base` is inert by construction.
- **x87 is why emulation was declined.** 76.59% of image bytes carry x87, and
  QEMU drops to host `double`, so it cannot arbitrate this image.

### Why it does not come back as-is

It was measuring how much of the image a *behavioural* oracle could reach. That
question is settled and the answer is "about half". Byte-matching answers a
stronger question over the whole image at O(1) cost per body, which is why the
project now has one route instead of two.

## The DLL, the redirects and the whole runtime route — retired 2026-08-12

**This is the big one, and it is explicitly reversible.** Until now the buildable
artifact was `OpenSMACX.dll`, injected into the shipped executable: `dllmain.cpp`
carried **2,049 fixed-address redirects** (10,671 lines, almost all of it the
table), each validating a byte signature at process attach before writing an
`E9 rel32` jump into the original's code, plus 488 export aliases in
`OpenSMACX.def`. That is how recovered code ran, and `terranx_hybrid.exe` plus
the DLL was the thing you could actually play.

It is retired **while the recovery is finished by byte-matching, which runs
nothing**. If the recovery turns out not to reach far enough for the tree to
stand alone, the dispatching comes back — that is the explicit plan, not a
consolation.

The tension it resolves is in `AGENTS.md:5`, which has always said final builds
must require "no original executable, copied machine code, **fixed-address
redirects**, or proprietary assembly". Redirects were always temporary. What
made retiring them a real decision is that `recovery_metrics` reports
**machine_carried at 90.56% of scope bytes**: nine-tenths of the image's
behaviour is still supplied by machine-derived code, so a standalone link is a
long way off and the interim has no runnable artifact.

### What replaced it, and why it is a better build check

`add_executable(OpenSMACX)` over the same sources, with `src/main.cpp` in place
of `DllMain`, producing `opensmacx-link-check.exe`. Every object is linked
**directly** rather than pulled from an archive on demand, so an undefined symbol
anywhere in the recovered tree is a link error. The DLL gave that guarantee only
for the 488 names its `.def` exported.

It earned itself immediately: the first link failed on
`?run_deferred_oracles@@YAXXZ`, an oracle hook still called from
`scenario.cpp` after the unit that defined it was removed. `main()` deliberately
does nothing — running the recovered code needs the game's data, its globals
initialised and the original process image, none of which exists without the
injection route. A binary that links and then faults is a worse signal than one
that links and exits.

### Retired with it

Twelve source files (`dllmain.cpp` and eleven `*_oracle.cpp`), 22 tools, and the
targets behind them: hybrid image assembly and staging, the game installer and
runner, the gameplay smoke gate, the in-process differential oracle and its
generator, the mutation harness, the observability census, `legacy-leaves.S`
extraction, and the redirect wiring (`add_redirect`, the signature generator,
the patch-fit check, the import shim and both export emitters).

### The ideas to bring back with it

- **A redirect validates a byte signature before it patches.** A mismatch fails
  DLL loading rather than corrupting a running game. Any future injection route
  should keep that ordering.
- **`verify_redirect_patch_fit`** checked that every wired redirect had room for
  its `E9 rel32`. Nothing checked it until 2026-07-29, and it is the kind of
  defect that only shows as a corrupted neighbour.
- **The deferred oracle phase.** The executable's CRT heap deadlocks during
  `DllMain` (`RtlpWaitForCriticalSection section 009C0538 ... blocked by 0000`),
  so oracle work had to trigger off a later hook — `scenario_opening_movie`, the
  first startup call site reached after the CRT is running. The hook point is
  still marked in `src/scenario.cpp`.
- **Vacuous fixtures are the failure mode.** Game tables are empty during
  `DllMain`, so a suite that only reads them compares zero against zero. The
  `verify_close` case seeded `0x77777777` and retargeted a displacement from
  `0x14` to `0x18` for exactly that reason.
- **`--reuse-owned-wine-prefix` must not come back.** Teardown is `wineserver -k`
  at 0.053 s; a retained session cost **1.05 s per CTest invocation**, because
  the wineserver and its six children inherit the runner's stdout and hold
  CTest's output pipe open. A 12-mutant sweep measured 70.7 s with it against
  45.8 s without, same 12/12 kills.

## Still live, and not retired by this file

`recovery-gameplay-tests` remains: it is the sole observer for 37 of those 63
functions and joint observer for 17 more, and it builds under VC6 as of
2026-08-12. The runtime oracle, hybrid staging, mutation harness and redirect
machinery are all still in the tree; when they go, they get sections here, with
their numbers, before the code is removed.

## The cross-analysis correlation and the metadata gate — retired 2026-08-13

Three committed outputs and the three tools that maintained them:
`docs/recovery/analysis-correlation.csv` (6,000 rows), `priorities.csv` (3,172),
`analysis-summary.json`, plus `correlate_recovery_analyses.py`,
`verify_recovery_metadata.py` and `batch_decompile.py`.

**What it measured, and it was real.** Two independent disassemblers against our
catalogue's function boundaries. IDA9 agreed exactly on 5,264 of 5,347 and could
not see 661; Ghidra agreed on 3,529 of 9,918 and could not see 2,029, with 385
entry-range and 54 start-only near-misses. Three readings of the same image, and
where they agreed a boundary was worth more than any one of them alone.

**Why it went anyway.** Byte-matching is a STRICTLY STRONGER boundary control on
every function it reaches. A body that recompiles to the exact bytes of
`[start, end)` has proved the boundary; tool agreement only votes on it. The
correlation kept its value only for functions nothing has proved — a population
that shrinks with every batch, measured by a route with no second opinion in it.

**And it had stopped being true.** `priorities.csv` carries a `recovery_state`
column sourced from `functions.csv`, deleted when the map moved into `src/`.
Measured before deleting: **1,297 of its 3,172 rows say `unrecovered` for a
function that is implemented in `src/`, and 716 sit below `source_complete` on a
body that is BYTE_EXACT.** Forty-one percent wrong as a worklist, and
unrefreshable, because its input no longer exists. `recovery_frontier.py` reads
`src/` and replaces it.

`verify_recovery_metadata.py` was the gate over these files, and its `--promote`
mode regenerated `functions.csv` and `callgraph.json` — the two files whose
deletion made `src/` the record. A gate that restores what the project
deliberately removed is not a gate, and this one had been failing since its own
inputs went away.

`batch_decompile.py` drove Ghidra headless to bulk-produce decompiled C. Under
VC6-only that dependency retires it on its own, and the byte-match loop never
read decompiler output: an agent writes C++ and the compiler judges it.

**What is worth rebuilding, in some other shape.** Boundary disagreement is a
real signal for the un-proved population — `entry_range` and `start_only`
near-misses are where a span is most likely wrong. Recovered as a QUERY against
the IDA/Ghidra exports, run when a body will not match and the span is suspect,
rather than as three committed files and a gate that regenerates them.

## Twenty-five tools outside the three live paths — retired 2026-08-13

Reachability had held at 83 tools with every one "reached from an entry point",
but reachability only proves nothing is ORPHANED. Classifying instead by whether
a tool is EXECUTED — by a CMake gate, by the agent loop, or by another tool's
import — gave a different picture:

    executed by CMake or the agent loop   34
    library, imported by another tool     24
    test-only: its test runs, it does not 18
    reachable from nothing at all          7

The 18 test-only tools are the interesting group, because they LOOK healthy: a
CMake registration runs a test for each, and both registration gates go green.
Neither gate checks that a test PASSES, which is how `analyze_delegates` sat
failing on a ModuleNotFoundError since 271114cf.

Retired here, with what each was for:

  build_regressions, byte_match_fanout, msvc6_byte_match   superseded by
      decomp_status, which measures the whole map in one pass
  runtime_process, harvest_proven_units, preserve_worked_units   the runtime and
      staging routes, retired above; harvest reported 0 remaining
  derive_vtables, verify_subobjects, split_proved_fields, extend_short_classes,
  promote_agreed_sizes, name_members_from_behaviour, name_members_from_sources,
  correlate_pracx_layouts, structure_observations   the class-layout derivations,
      whose outputs `src/` now carries directly
  jump_tables, indirect_call_sites-adjacent helpers, integrate_recovery,
  classify_recovered_shapes   analysis aids the byte-match loop does not invoke
  run_gate, host_doctor   wrappers; host_doctor's checks moved to the one it
      actually needed (VC6), see below
  audit_recovered_signatures, correlate_external_analysis, run_ghidra_analysis,
  export_recovery_inventory, mizuchi_context   the external-analysis and Ghidra
      routes, retired above

`parse_ranges` moved from classify_recovered_shapes into
derive_prototypes_from_names, its only consumer: it is the helper that reads a
body's SPANS rather than `end_address - address`, and the same arithmetic taken
from the wrong span once cost 241 BYTE_EXACT claims.

**One deletion was WRONG and is reverted in the same breath.**
`verify_check_tests_observe.py` mutates each check and requires that check's own
tests to notice — the only instrument here that can say whether a test is worth
keeping, and the one that found vacuous tests three audits running. It was
retired as an "orphan" because it is reachable from no gate, which is BY DESIGN:
it is a diagnostic somebody runs deliberately. Restored the same day.

Restoring it exposed why it had been silently useless: it imported
`mutate_and_verify`, deleted with the mutation route on 2026-08-12, so it had
been dead on arrival ever since and nothing noticed — an import error only
surfaces when somebody runs the thing. Only `Function`, `build_mutants` and
seven helpers were ever used, ~250 lines of 968, so those are inlined and the
dependency is gone rather than resurrected. Its first run immediately reported
that `test_verify_build_freshness` does not pass on the UNMUTATED tool, so
nothing can be measured from it.

Everything here is recoverable from git. The rule that put them on this list is
"outside all three live paths", not "worthless": bring one back by restoring the
file and giving it a line in docs/TOOLS.md, which is what makes it reachable.
