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
replaced. `C2065` is the largest such family in the tree — 951 of 1,703
`NO_COMPILE` rows — so this is work already on the route, not work invented to
compensate.

The remaining three — `0x004C6120`, `0x004C66E0`, `0x004C67C0`, the `Sound` and
`Wave` constructors and destructors — are `SHARED_TAIL`: the linker folded their
tails onto another catalogued function under `/Gy`, so no per-function verdict
is well defined and **byte-matching is structurally incapable of ever speaking
about them**. Behavioural evidence is the only kind that exists for these, and
`tools/generate_signature_oracles.py` with the differential
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

## Still live, and not retired by this file

`recovery-gameplay-tests` remains: it is the sole observer for 37 of those 63
functions and joint observer for 17 more, and it builds under VC6 as of
2026-08-12. The runtime oracle, hybrid staging, mutation harness and redirect
machinery are all still in the tree; when they go, they get sections here, with
their numbers, before the code is removed.
