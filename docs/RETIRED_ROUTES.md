# Retired routes, and what they were for

Byte-matching is the project's only live verification route: write a body, ask
VC6 12.00.8168, compare the bytes. Everything below was built to answer the same
question a different way, and is retired rather than deleted from memory.

**This file used to be a reachability root.** `verify_tool_reachability.py`
globbed `docs/*.md`, so a tool named here stayed reachable and a tool whose
line was struck had to be deleted in the same commit. That tool went with the
suite below; the convention survives it as a convention, and naming a tool here
is still how an idea is kept for one paragraph instead of rebuilt from scratch.

Nothing here is a recommendation to rebuild it as it was. Each section ends with
the measurement that priced it, because the numbers are the part that does not
survive re-derivation.

---

## The ctest suite — retired 2026-08-15

`tests/` was a 1,487-line `CMakeLists.txt` registering 95 tests: ~60 Python
tool-test files, the `recovery-gameplay-tests` C++ fixture target, and the ABI,
vtable, class-layout, span-termination, call-edge and annotation-integrity
gates. Roughly 6,600 lines of test code.

**Why it went. Nothing consumed its results.** The suite was run on 2026-08-15
and reported 21% passing — 75 failures, every one of them
`ModuleNotFoundError: No module named 'pefile'`. The cause was a plain
`cmake -S . -B build`, which reconfigures without `-DOPENSMACX_PYTHON` and
falls back to `/usr/bin/python3` instead of the venv every tool in this tree
runs on. That is not a state somebody sees and tolerates for a while; it is
what a suite nobody runs looks like from the inside.

The corroborating measurement: `test_agent_brief.py`'s prose bound was failing
at `52e624fb`, **the commit that introduced the bound**, and stayed red through
every commit after it. A check that has never once been green is not enforcing
anything, and neither is a green one nobody reads — which is the argument that
settled this. Green and red were equally uninformative here, so the split
between them was not worth preserving either.

**What replaced it: nothing, deliberately.** The gate is
`tools/decomp_status.py --check` — 1,540 BYTE_EXACT claims recompiled against
the shipped image, 0 not reproduced, run every batch and read every time. It
was always the only load-bearing check; the rest was apparatus around it.

**What was kept.** Every verifier whose subject is the IMAGE or the CATALOGUE
still exists and still runs standalone: `verify_member_offsets`,
`verify_class_layouts`, `verify_recovery_abi`, `verify_recovery_pipeline`,
`verify_span_termination`, `verify_call_edges`, `verify_return_agreement`,
`verify_vendor_zlib`, `verify_void_returns`, `verify_documented_counts`.

**What was deleted with it.** Every verifier whose subject was the SUITE:
`verify_checks_can_fail`, `verify_check_tests_observe`,
`verify_tool_test_registration`, `verify_test_registration`,
`verify_tests_all_run`, `verify_tests_do_not_write`, `verify_tool_reachability`,
`verify_build_freshness`, `verify_cmake_paths_exist`, `cmake_sources` and
`run_windows_test`. Ten of the eleven checked a property of the tests
themselves; with no tests, each was vacuously green.

**The cost, stated plainly.** Six invariants now have no automatic enforcement:
class layouts against the image, ABI agreement, span termination, call-edge
completeness, vendored-source integrity and documented counts. A regression in
any of them will be found by someone running the tool, or not at all. The
byte-exactness of 1,540 functions is unaffected — that is what the gate
measures.

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

**Three of those four are still enforced. `def-append-only` is not**, and the
overturn above is the reason it took a second look: the ratchet it was said to
protect was `export-signedness-audit`, and on 2026-08-13 both were retired
together with the file underneath them — see *The module definition and the two
checks that policed it* below. The overturn was not wrong when it was written;
its premise simply stopped holding the moment the surviving ratchet stopped
surviving.

| gate | why it went |
|---|---|
| `vtables-current` | kept `vtables.csv` current. The FILE is now deleted too, 2026-08-14: it said 0x006698C4 belonged to 15 classes at offset 0, which distinct classes cannot. Both readings of what it might have meant instead were refuted against the image, and `tools/derive_class_vtables.py` reads the answer off the constructors with no collisions at all — 70 classes, 70 distinct primary vtables. A contradicted file that nothing reads is still available to believe |
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
`mizuchi_context.py`, `declfix.py`, `writeback.py`,
`emit_target_object.py` and `disassembly.py`, the last of which now
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
  entire life. The `LEVER:`/`TRIED:` grammar exists because of this.
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

  Those 47 names were corrected on 2026-08-13 and are no longer misleading: each
  is now spelled as what it is, an MSVC this-adjusting thunk into the class's own
  `??_G` scalar deleting destructor, with the displacement the body subtracts
  written into the name (`??_GAlphaMovie@@WEEE@AEPAXI@Z`, `adjustor{1092}` ==
  `0x444`). See the annotation's own `// CORRECTED from` note, which carries the byte evidence
  per address. The lesson stands unchanged — a name that survived being wrong for
  this long did so because nothing held it to the bytes, and what finally caught
  it was the callee-purge gate, a byte-driven check, exactly as this bullet
  predicts.
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

## class-size-bounds.csv — retired 2026-08-13, tool kept as a query

Retired the FILE, not the derivation. `derive_class_size_bounds.py` still runs
and still refuses; it just no longer commits what it found.

**Why it could not move into `src/` as `static_assert`s, which was the obvious
idea and is what happened to the class SIZES.** Measured: of the 24 classes
carrying a bound, **1 is declared in a real header** (BaseWin), 5 exist only in
the generated `hypothesis_layouts.h`, and **18 are declared nowhere at all**. A
`static_assert(sizeof(X) <= N)` needs an X to `sizeof`, so 23 of them would need
a synthetic placeholder - and asserting `sizeof(Ambience) <= 0xB8` against a
struct we padded to 0xB8 ourselves checks our own declaration against itself.
That is the vacuous shape this tree keeps catching, dressed as consolidation.

The evidence in a bound is about the ORIGINAL image - `bounded above by
??__Eg_FAMEWIN@@YAXXZ at 0x0074DAF8` - so it belongs measured, not claimed.

**What the file was doing was nothing.** No code read it: the two tools whose
docstrings mention `derive_class_size_bounds` only NAME it in prose. Its sole
consumer was the `--check` that regenerated it and compared it to itself.

**What survives is the half with content.** The control runs before the
comparison ever did, and unconditionally: on the classes whose true size IS
pinned by a `static_assert`, every derived bound must sit at or above it, and a
bound BELOW one is refused - that is the direction that would under-allocate a
receiver and corrupt memory. Currently 23 at or above, 0 below.

**And the file was self-retiring already.** `derive_class_size_bounds.py:61`
drops a class the moment `src/` pins its size, so the list shrank from 34 to 24
as ten classes graduated into headers. The finish line was always the empty
file; deleting it only skips to the end.

`ProvenanceTests` in its test file goes too - two tests asserting that
`generate_signature_oracles` never presents a bound as a pin. That generator
retired with the runtime-oracle route, so both had been erroring on the import,
and the property has no consumer left to guard.

## The module definition and the two checks that policed it — retired 2026-08-13

Deleted: `src/OpenSMACX.def` (490 lines, 488 export aliases, 27 commits of
history), `verify_def_append_only.py`, `audit_export_signedness.py`, both test
suites, and all four CTest registrations — the gate checks `def-append-only` and
`export-signedness-audit` plus their two `-tests` lanes.

**The ground, and it was already written down.** `CMakeLists.txt` said in so
many words, at the top of the compile-options block, that `src/OpenSMACX.def` is
NOT linked: every line in it was an ALIAS bridging an Itanium-mangled GCC symbol
to the MSVC name the patched image imported, and cl 12.00.8168 emits the MSVC
name directly, so the aliases described symbols this build does not produce.
Nothing builds a DLL — there is no `add_library`, no `SHARED` and no `MODULE`
anywhere in `CMakeLists.txt`, only `add_executable(OpenSMACX …)`. The generator
that wrote the file, `add_redirect.py`, no longer exists. And *The DLL, the
redirects and the whole runtime route* above retired the consumer on 2026-08-12.
The file outlived its reader by a day and its writer by longer.

**`def-append-only` was BROKEN, and it was repaired hours before it was
retired.** That sequence is the lesson worth more than the deletion. The check
guarded "no export name this file ever published may disappear", because the
staged `terranx_hybrid.exe` imported 462 names from a frozen table and a dropped
alias broke the game while ctest stayed green. It defaulted to `--base HEAD`,
and CMake registered it with no arguments at all — so on a clean tree
`HEAD:src/OpenSMACX.def` is byte-identical to the working copy and
`base − current` was empty by construction. It was a pre-commit diff: real while you
were editing, silent in CI, and permanently quiet about any removal already
committed. It could not fail, and nothing said so, because it also sat in the
EXEMPT set of `tools/verify_checks_can_fail.py` back when that set held fourteen
names — the one place that would have asked was told not to.

Commit `e8306017` fixed it: read the UNION of every set the file has held across
the whole history via two `git cat-file` batches (28 revisions in 7 ms), and
give it a real damage case — a throwaway 13-revision repository that adds twelve
aliases one per commit, drops one in a thirteenth commit, and leaves the tree
clean. It refused correctly, exit 1, on exactly the state `--base HEAD`
structurally cannot see. Its last live run in this tree:

    def-append-only: 488 exports, none removed since any of 28 revisions   (exit 0)

Then the subject turned out to be dead. **Nobody asked whether the thing it
guarded still existed until after it was repaired** — the frozen import table
belongs to a hybrid image retired the day before the fix landed. Codify that as
a question, not a resolution: before repairing a check, price its subject.

**What `export-signedness-audit` measured, and why it cannot apply.** It was the
one instrument here that could see a defect class no oracle can: the original
divides signed, the recovery declared the parameter unsigned, and the two agree
on every non-negative input. `bitmask` was found that way by byte-matching, not
by any test. The audit worked because two *independent* records of each
signature existed — the catalogue's MSVC decoration in
`docs/recovery/functions.csv`, read out of the SP3 IDB, and the GCC symbol each
`.def` alias mapped to, produced by the compiler from committed source. Neither
was derived from the other, so a disagreement was evidence. Its last live run:

    exports compared: 315
    signedness disagreements: 198   (signed_divide 44, signed_shift 61, bounded 93)
    audit-export-signedness: within baseline (198 <= 199, 44 <= 44)        (exit 0)

**The paragraph that stood here was wrong twice, and an adversarial pass caught
both. Both corrections are recorded rather than edited away, because the shape
of the error is the useful part.**

It argued: *"Deleting the `.def` destroys the second record, so the audit has
one source and nothing to compare it against. This is a real loss, and it is
not a stale sentence."*

**WRONG ABOUT THE SECOND RECORD.** It is not the `.def`. It is the committed
declarations in `src/*.h`, of which the `.def` was a *compiler-side mirror* —
and a stale one. The comparison the audit performed is reconstructible today
with no `.def` at all: the catalogue's decoration, still live and now read from
`src/` through `emit_translation_unit.load_functions`, against the declared
parameter types in `src/*.h`. The verifier rebuilt it in one script;
`tools/declfix.py` already carries the MSVC type-char table it needs. So the
FILE was correctly retired and the RATCHET was not — it went for a reason that
does not hold, and nothing now stops the population growing again.

**WRONG ABOUT THE FINDING BEING LIVE.** 198 was the reading of a frozen file.
Measured against the tree on 2026-08-13, by restoring the tool and joining
every finding to `src/*.h`: **180 of the 198 are already fixed**, zero agree
with the `.def` alias, 10 are genuinely live and 8 are undetermined (ambiguous
stems: `init`, `get`, `territory`, `has_fac_built`, `base_lose_minerals`).
Commit 78038809 (2026-08-05) *"Correct 174 recovered signatures against their
own mangled names"* changed 174 declarations in `src/` and never touched the
`.def`, whose aliases were last written 2026-07-11 (c6f5a1f0). The audit's
premise — "the GCC symbol each alias maps to, which the compiler produced from
the committed source" — stopped holding on 2026-08-05, and **its `BASELINE`
ratchet could not notice, because a frozen file cannot move.** The work the 44
named was done eight days before this retirement priced it as a loss.

THE TEN THAT ARE STILL LIVE, enumerated here because they exist nowhere else
and are NOT recoverable from the deleted file — against the `.def` they are
indistinguishable from the 180 stale ones:

| address | name | class |
|---|---|---|
| `0x0057D510` | `transport_val(chassis_id, ability, reactor_id)` | **signed_divide** |
| `0x005A5A60` | `proto_cost(chassis_id, weapon_id, armor_id, ability, reactor_id)` | **signed_divide** |
| `0x004E4090` | `name_base` | signed_shift |
| `0x0056B480` | `coast_or_border` | signed_shift |
| `0x00593830` | `quick_zoc` | signed_shift |
| `0x0055BB30` | `set_treaty` | bounded |
| `0x0055BBA0` | `set_agenda` | bounded |
| `0x005591E0` | `has_agenda` | bounded |
| `0x00579A30` | `add_goal` | bounded |
| `0x00619370` | `find_line_break_l` | bounded |

Each declares a parameter `uint32_t` where the original treats it as signed.
The two signed divides are the sharp ones: original and recovery agree on every
non-negative input and differ on negatives, which no differential oracle and no
mutation run can see.

**A COUNT RATCHET WAS PROVED BLIND HERE, AND THE PROOF NEARLY WENT WITH THE
TEST FILE.** `test_a_rename_that_keeps_the_count_still_fails` recorded a real
incident (312b5cf3, 2026-08-01): one alias was removed and another added, so
the export COUNT was unchanged at 467 and only a SET DIFFERENCE caught it. This
repository is built out of count ratchets — `BASELINE`, `GATE_CHECK_FLOOR`,
`PREFIX_FAMILY_FLOOR`, `--floor 50` — and this is the one concrete case where a
count was demonstrably blind to a real breakage. It generalises far past the
`.def`: **a ratchet on a total cannot see a substitution.**

That also settles the overturn recorded above under *Seven gates*.
`def-append-only` survived an earlier retire verdict on the grounds that it
"guards the join key of a surviving status ratchet" — and the ratchet was
`export-signedness-audit`, whose join key was the alias name, so a dropped
alias shrank `compared`
toward `COMPARISON_FLOOR` and could hide a disagreement. The argument was sound.
It just required the audit to survive, and the audit is what goes here. Retiring
the pair together is the only internally consistent move; retiring either alone
would have been the mistake that overturn caught.

**What else had to move in the same change, none of it in the original scope.**
Three damage cases and their `CASES` rows in `tools/verify_checks_can_fail.py`
(keeping them would trip the `orphans` structural test, which fires on the table
alone — raising `Skip` is not an escape). Two registered suites that the survey
of references did not name: `tools/test_verify_checks_can_fail.py` pinned
`export-signedness-audit` and `def-append-only` as proof its paren-counting
parser finds deeply nested blocks, and `tools/test_verify_check_tests_observe.py`
asserted a prefix-family population floor of 18 that the two deleted files put at
17. Both would have left ctest red on a change that touched only the files the
`.def` is mentioned in.

The parser pin is now DERIVED rather than repointed. It ran the naive
`(.*?)\n\s*\)` regex's casualties by hand, and after the deletion a *different*
block became the one that regex loses; a hand-repointed list would have been
aimed at whatever still existed, which is how a pin becomes a tautology. It now
runs the naive regex over the real `CMakeLists.txt`, requires the casualty set to
be non-empty, and requires every casualty to be parsed.

`docs/recovery/summary.json` keeps 320 dangling references to the deleted file —
`inputs.definition` plus 319 `"location": "src/OpenSMACX.def:N"` rows under
`redirects.definitions`. It is a committed artifact of the retired IDA-side
pipeline, nothing regenerates it, `recovery_metrics` reads neither field, and
`project_catalogue.py` writes `redirect_exports` as a constant empty string. No
check stales; the dangling line numbers are recorded here rather than rewritten.

Four claims in `AGENTS.md` went with it: the recovery recipe's step 4 told
agents to hand-add a decorated alias to a file that will not exist; the
merge-conflict table's "append-only, one line each" row (the table's "eight
files" is now seven); the standing rule about the frozen 462-symbol import table
and `stage-hybrid-game`; and the provenance line for the retired
`generate_mingw_exports`. `docs/TOOLS.md`'s "Wiring a recovery into the DLL"
section had two live entries left in an otherwise fully retired list — it has
none now, and says so in its heading.

### How to bring it back

The export set is not pinned by anything today, and nothing needs it pinned
because nothing imports by name. If a future route pins one again:

* the file: `git show 72d7ea96:src/OpenSMACX.def` — 490 lines, 488 aliases; its
  27-commit history is `git log --oneline -- src/OpenSMACX.def` before this
  commit;
* the append-only guard: added in `919a7129`, and **restore the `e8306017`
  version, not the original** — the original could not fail;
* the signedness audit: added in `1e50f588`, with `BASELINE`
  `{disagreements: 199, signed_divide: 44}`, `COMPARISON_FLOOR` 300 and
  `RANKED_FLOOR` 150; its `--exe` must be passed explicitly, because with no
  executable every finding stays `unranked` and half the ratchet compares 0
  against 44;
* the registrations, the damage cases and the doc entries: the diff of this
  commit, reversed.

### The measurement that priced it

Every affected check, run after the change, verdict line and exit code:

| check | before | after |
|---|---|---|
| `tool-reachability` | 59 reached from 15 entry points, exit 0 | **57 reached from 15 entry points, exit 0** |
| `tool-test-registration` | 53 test files, all executed by CMake, exit 0 | **51 test files, all executed by CMake, exit 0** (floor 50) |
| `cmake-paths` | 38 literal source path(s), all resolve, exit 0 | **36 literal source path(s), all resolve, exit 0** |
| `checks-can-fail` | 33 cases across 28 of 29 checks, 1 exempt, exit 0 | **30 cases across 26 of 27 checks, 1 exempt, exit 0** |
| `tests-all-run` | — | **51 test file(s), none stranded, exit 0** |
| `documented-counts` | no per-state count restated, exit 0 | **no per-state count restated, exit 0** |
| `tools/` unittest discover | — | **1,344 tests, OK (13 skipped), exit 0** |

`GATE_CHECK_FLOOR` in `tools/verify_checks_can_fail.py` went 27 → 26. It sat at
exactly the post-removal count of 27, which passes with zero headroom and makes
the *next* honest retirement raise `SystemExit` out of `gate_checks()` for every
caller — including `verify_check_tests_observe.candidate_tools()`, which imports
it. One check of headroom, matching the convention `--floor 50` against 51 test
files already uses; `test_the_floor_is_not_slack` refuses more than two.

## `wine-test-lock-check` — retired 2026-08-13, its subject kept

`verify_wine_test_locks.py` (named bare, not as `tools/…`: this file is a
reachability root, and a qualified path to a deleted tool is a STALE entry
point) required every Wine-backed CTest to hold the
`wineprefix` RESOURCE_LOCK. The hazard it guarded is real and unusually nasty:
the tests that go through `tools/run_windows_test.py` all drive the ONE
marker-protected prefix, and that runner stops the prefix when it finishes, so
two at once means one killing the other's wineserver mid-test. **A wine-backed
test missing from the lock list does not fail — it flakes, in a different test
each run.** Measured on the debug preset when it was written: serial `ctest`
passed 58/58 in 67.9 s; `ctest -j8` finished in 45.6 s and failed
`recovery-leaf-tests`, an earlier lane failing three.

**It was policing seven tests. It is down to one.** `globals-diff-tests` and
`recovery-oracle` went with earlier retirements, leaving only
`recovery-gameplay-tests`, and with one holder a RESOURCE_LOCK serialises
nothing. What survived was the prospective half — the `--expect-at-least`
floor, which refuses when a Wine-backed suite silently loses its lock, and
which is why an earlier retire verdict on this check was **overturned**.

It goes now because that floor had itself gone stale in the way this repository
keeps finding: `--expect-at-least` was derived from
`OPENSMACX_WINE_LOCKED_TESTS`'s LENGTH, the list still named all three, and the
check refused with `expected at least 3, found 1` — red at HEAD, against its
own subject, for an unknown period. That is the second stale-constant red in a
list CMake derives a floor from, after `EXEMPT` in `verify_checks_can_fail`.
Both are hand-maintained lists that outlived their members.

**WHAT REPLACES IT: nothing, and the list is now maintained BY HAND.** Adding a
second Wine-backed test requires adding it to `OPENSMACX_WINE_LOCKED_TESTS`
yourself; if you forget, the suite flakes rather than fails, and no check will
tell you. That is the cost, priced here rather than struck. Restoring it is one
file plus one `add_test`, and the floor should then be derived from the
generated `CTestTestfile.cmake` — what CTest will really run — rather than from
a list in this file, which is what let it drift.

**SUPERSEDED 2026-08-15 — `recovery-gameplay-tests` IS retired, with the whole
`tests/` directory (see the section at the top of this file). The argument
below is preserved because it is still the correct account of WHAT was lost:
37 functions whose only behavioural observer it was. What changed is not that
the argument was refuted, but that the suite holding it was not being run at
all, so the observation it offered was never actually made.**

**[Historical] `recovery-gameplay-tests` IS NOT RETIRED, and was proposed for retirement
alongside this check on a framing that turned out to be wrong.** It looked like
the last vestige of the runtime route retired 2026-08-12. It is not: 16,228
lines and 2,217 assertions, and by this document's own measurement at line 418
it is the **sole behavioural observer for 37 recovered functions of >=200 bytes
and joint observer for 17 more** — a class byte-matching proves nothing about.
Retiring it would have deleted the only behavioural evidence for 37 functions,
which is the same error as retiring a capability along with the file that
happened to carry it. Its sources remain compiled by the main `OpenSMACX`
target, so nothing about it is redundant build coverage.

Two latent breakages were fixed in passing, both caused by earlier retirements
leaving names behind: `OPENSMACX_WINE_LOCKED_TESTS` named two tests that no
longer exist, and `add_custom_target(verify-recovery-batch)` still listed
`globals-diff-tests` in `DEPENDS`, a target defined nowhere in the tree.

## A base clause on an EMPTY shell — measured, and a loss

**Tried and reverted 2026-08-14.**

An agent recovering `PushButton::PushButton` (0x0062BF20) reported that the
scaffold declares `class PushButton { public: PushButton(); };` with no base,
though `src/pushbutton.h` derives it from `BaseButton` — so genuine
base-subobject construction is unreachable and it landed the field stores
without the base-constructor call. `class_layouts.layout_bases` gates the base
clause on the DERIVED class being supplyable, and `PushButton` has no pinned
size.

The reasoning for relaxing it looked sound. A shell with no members has no
offsets for a base clause to push down, so emitting `: public BaseButton`
could only add: the base's real members at their real offsets, and an
expressible base. Guarded so a shell that has members from either source — a
pinned layout or a proved offset read out of a byte-exact body — stays flat.

Measured over the whole tree, before and after:

  BYTE_EXACT     1329 -> 1225   (-104)
  NO_COMPILE      686 ->  850   (+164)
  MISMATCH       1716 -> 1686

and the ratchet reported **112 claimed-byte-exact bodies no longer
reproducing**. Reverted the same hour.

The argument was about LAYOUT and the cost is about SCOPE and ORDER: a base
clause makes the base a dependency of the derived class in the unit, and the
emitter's ordering only guarantees that for classes it decided to flatten. It
also brings every base member NAME into the derived scope, where the flat form
had none.

Retrying this needs the ordering fixed first, not the gate relaxed. The
evidence that it is worth doing is unchanged — an agent really did land a
weaker body for want of a base — so this is a "not yet", not a "no".

---

## TU refinement from `.rdata` ordering — measured dead 2026-08-22, tool retired 2026-08-23

The idea: if the shipped image's sections preserve the original link order,
the boundaries between translation units are recoverable, and organising
`src/` around them would speed matched decompilation by putting each body in
a unit whose neighbours - and therefore whose inlining opportunities - match
the original's.

Half of it is TRUE, and proved. `.text` IS in link order: the CRT initialiser
table (434 entries) walks monotonically - **0 descents in 433 steps**. Locate
that table by the NULL `__xc_a`/`__xc_z` sentinels in `.data`, and ONLY that
way: locating it by longest-plausible-run instead finds an 871-entry vtable
in `.rdata` and reports 335 false descents, which is the trap waiting for
anyone re-measuring this.

The refinement route is DEAD. `.rdata` carries no TU signal - walking it the
same way measured **51.9% descents**, coin-flip order, so it cannot sharpen
the `.text` boundaries. And the true half was measured to not be worth
acting on either: the session that proved it also asked whether reorganising
`src/` by TU would make recovery faster, and the answer was no - the match is
decided per body by flags, spelling and class modelling, and `osmx measure`
already compiles each body in the unit it lives in. The user closed it with
"let's not worry about TU for now".

`tools/link_order.py` made both measurements; it was deleted in 56f53bec with
the other unreferenced tools. Git holds it if this route is ever re-opened
with new evidence - and the sentinel-vs-longest-run trap above is the first
thing to re-read if it is.
