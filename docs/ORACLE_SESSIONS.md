# Oracle sessions: proven count and shape-addressable set

One entry per session that touched a proof route. Two numbers are mandatory,
because they are the two that can move in opposite directions and hide each
other: how many functions carry a proof, and how many the routes could reach if
the work were done. A session that grows the second without the first has built
capacity; a session that grows the first without the second has spent it.

Numbers here are measured, never carried forward. Where a figure came from a
tool run, the command is named so the next session can re-run it rather than
trust it.

---

## 2026-07-30 — the original builds its own state, and it is not enough

Phase 2.1's `--build-state` is implemented and it works: the original's own
startup runs, 438 initialisers of real machine code execute, and a state file
comes out. Measured against the threshold fixed in advance, **it fails.** No
proof route moved, so both mandatory numbers are unchanged at 53 proven.

### The pre-committed test, and the result

Sweep pair with and without the state, both non-refusing, because the refusing
population is 74,687 B and cannot host a 250,000 B improvement in principle:

| criterion | required | measured | |
| --- | --- | --- | --- |
| `INCONCLUSIVE-original-fault` better | ≥250,000 B | **+19,186 B** | **NOT MET** — 7.7% of it |
| `agreed_full_strength` not worse | — | 42,209 B / 768 fn, unchanged | met |
| kill: full strength worse by >2,000 B or >20 fn | — | not triggered | no revert |

Bought weakened, and reported as the plan requires: `agreed_only_on_paths_taken`
rose 11,360 B (558,043 → 569,403) and `executed` rose 15,375 B. More code runs;
it is not more code that agrees. The wall went 1,584,976 → 1,565,790 B, which is
1.54% of the 1,246,125 B of near-null mass measured as addressable.

### Why — and the first answer given here was wrong

**Correction.** This entry first said the state was full of host-scoped values
because the imports were bound for real, citing a 21.7% in-span residency as
evidence. That number measured every address-shaped word in the dump window,
which is mostly the image's own static `.data`: the "out-of-span" population was
dominated by constants like `0x01000100`, `0x05050505` and `0x0a0a0a0a`, spread
almost uniformly across the low megabytes, which is the signature of data rather
than of pointers. It was not evidence of contamination, or of anything.

Diffing the built state against the pristine image gives the real population —
the **181,426 words this run changed** — and among those:

| | count |
| --- | ---: |
| changed words in any plausible host-pointer range | **58** |
| of which most are float or colour constants (`0x461c4000`, `0xff0000ff`) | |
| changed address-shaped words that are in-span | 2,716 of 5,626 = **48.3%** |
| changed words pointing into the R1 guest heap | 18 |

So there is essentially **no host-pointer contamination**, and binding the
imports for real was not the limit. The instrument now measures only the changed
words, because those are the only ones the run is responsible for.

### What actually limits it

Splitting the fault wall by region, before and after:

| region | before | after | delta |
| --- | ---: | ---: | ---: |
| near-null | 1,246,125 B | 1,194,622 B | **−51,503 B** |
| wild | 134,296 B | 166,859 B | **+32,563 B** |
| top-page | 97,319 B | 97,319 B | 0 |
| stack | 68,143 B | 68,143 B | 0 |

The mechanism works: 51,503 B of near-null faults were genuinely converted. But
**two thirds of that conversion lands in `wild` rather than in agreement** — a
global that held zero now holds a value, and dereferencing it reaches somewhere
else instead of failing at zero. Net movement is the 19,186 B above.

And the shape of what did *not* move is the finding: 1,806 of the original 1,838
near-null-faulting functions are still near-null. Their globals were never
touched. Only 18 changed words point into the heap at all, from 42 allocations
of which the 1 MiB sbh region is most of the bytes.

**Static initialisers are not where this program builds its object graph.** The
`ListBox` cluster and the rest of the faulting mass are filled in later, by
`WinMain` and by gameplay. That makes the next step the boot — running further
into the program — and not more import overrides, which is what the wrong
diagnosis above would have bought.

### What it took to get __cinit to return

Three blockers, each a measurement rather than a guess:

1. **A hang, not a fault.** `RtlpWaitForCriticalSection section 009C0538
   blocked by 0000` — a `CRITICAL_SECTION` in the guest's own `.bss`, zeroed
   because `_mtinit` runs before `_cinit` and was not being run. A *real*
   `InitializeCriticalSection` would also write a DebugInfo pointer into a
   structure inside the dump window, so the override is required twice over.
2. **A 1 MiB allocation.** `___sbh_alloc_new_region` asks for exactly
   1,048,576 bytes; the scratch window above `.bss` holds 183,904. This is what
   sized the R1 guest heap at 2 MiB. The fault it caused landed three steps
   downstream, in `___initstdio`, so the harness now reports the largest
   *refused* request — the cause, not the symptom.
3. **`__cinit` alone is not startup.** It stopped in `___initstdio` at
   0x00647c06 reading `__pioinfo[]` at 0x009c10a0, which `__ioinit` fills
   earlier. The sequence transcribed from `start` at 0x00646C9D is
   `__heap_init(1)`, `__mtinit`, `__ioinit`, `__cinit`; with all four, all four
   return.

The argv/env steps in that sequence are deliberately skipped:
`GetCommandLineA` and `___crtGetEnvironmentStringsA` return host pointers that
`start` stores into globals inside the dump window. A constructor reading
`__argv` or `_environ` therefore sees NULL here, and anything derived from them
is not built — stated because it is a real hole in the state, not a detail.

---

## 2026-07-30 — a whole-image baseline, and the fault wall names itself

No proof route moved this session, so both mandatory numbers are unchanged: 53
functions carry a proof, and the reachable set is where the last entry left it.
What changed is that there is a current-host baseline to diff against, and the
fault wall has a measured shape instead of a size.

### The baseline: 5,673 rows, no dropped lines

```
REPORT=$PWD/build/oracle/report-base.tsv LOG=$PWD/build/oracle/sweep-base.log \
    ./tools/lifted_oracle_sweep.sh --refuse-blocked --cases 16
```

`--refuse-blocked` because `baseline-arm64/report.tsv` was swept that way and
anything compared against it must be too. Finished with 0 hangs and 1 host
death; `tools/lifted_oracle_summary.py` reads 42,209 B / 1.75% / 768 fn at full
strength and 2,214,194 B / 91.86% machine-carried — the historical figures, on
this host, reproduced.

Against the arm64 baseline the host move is not free, and the direction differs
by figure:

| figure | before | after | delta |
| --- | ---: | ---: | --- |
| `agreed` | 178,248 | 191,129 | +12,881 better, 1,465 -> 1,614 fn |
| `agreed_full_strength` | 42,236 | 42,209 | **-27 worse**, 767 -> 768 fn |
| `agreed_under_weakened_conditions` | 136,012 | 148,920 | +12,908 worse |

Bytes and function count disagree on the only figure that matters, which is why
both are printed. 2,496 B across 14 functions agreed before and do not now.

### 95.3% of the faulting bytes are a dereference of zero

The report's `detail` column already carries `accessing 0x...` on every faulting
row, so this needed no run at all — only someone to read the column:

| where the original's faulting access pointed | fn | bytes | share |
| --- | ---: | ---: | ---: |
| near-null (below 64 KiB) | 244 | 71,130 | 95.3% |
| inside the image span | 42 | 2,165 | 2.9% |
| wild | 11 | 1,081 | 1.4% |
| stack | 1 | 251 | 0.3% |

A near-null access is a zeroed pointer with a field offset added, which is the
signature of a global no constructor ever built. **This is the first direct
evidence that `--build-state` is aimed at the right thing** rather than a
plausible story about why the wall exists.

State the denominator honestly: that is the `--refuse-blocked` population, 298
fn / 74,627 B. The wall Phase 2.1 targets is the non-refusing one, so it was
swept too — same command without `--refuse-blocked`, to
`build/oracle/report-noref.tsv`, finishing with 0 hangs and 8 host deaths.
`INCONCLUSIVE-original-fault` there is **1,584,976 B / 65.76% across 2,354
functions**, and it breaks down:

| where the faulting access pointed | fn | bytes | share |
| --- | ---: | ---: | ---: |
| near-null | 1,838 | 1,246,125 | **79.1%** |
| wild | 208 | 134,296 | 8.5% |
| top-page | 125 | 97,319 | 6.2% |
| stack | 53 | 68,143 | 4.3% |
| inside the image span | 105 | 29,128 | 1.8% |

25 fn / 9,965 B carry a fault code with no access address and are excluded from
the percentages rather than assigned to a bucket.

**1,246,125 B is the mass `--build-state` can address**, and the plan's
pre-committed success threshold of 250,000 B is 20% of it — a target with a
denominator now, instead of a hope. The 79.1% is lower than the 95.3% above
because refusal was hiding the harder faults, which is the direction to expect
and the reason the refusing figure must not be quoted for this purpose.

The offsets say the same thing twice. Of the 1,838 near-null accesses, 813 are
**exactly zero** and 832 more fall in `1..0xff`: a null pointer, then a null
pointer plus a member offset. 174 are in `0x100..0xfff` and 19 above that.

Equally worth saying: 8.5% wild and 6.2% top-page are **not** reachable this
way. A CRT that runs does not populate a pointer holding seed bytes, and the top
64 KiB is the range the harness already documents as unmodellable. Roughly a
fifth of the wall needs a different answer.

### The one-seed-short cohort is the exception, not the rule

94 fn / 11,518 B are exactly one seed short of full strength.
`tools/lifted_oracle_why_not_full.py` replays each under `--verbose` and records
a reason for all 94: 92 `INCONCLUSIVE-original-fault`, 1
`INCONCLUSIVE-lifted-out-of-span`, 1 `INCONCLUSIVE-original-timeout`. By region:
49 near-null, 35 wild, 9 top-page, 1 stack-guard.

Wild leads on bytes here (6,057 vs 4,455) and that is one function:
`?wants_prop@@YAHHHH@Z` is 3,970 B of it, faulting on `0x592dc72c`. So the
cohort does **not** resemble the population above, and it should not be used to
argue about it — a tail of functions that nearly worked is selected for the
faults that are hardest to explain.

### The tool dropped the most important line it could read

First run recorded 91 reasons for 94 functions and reported no error. `%#010x`
of zero prints `0000000000` with no `0x`, because C only adds the prefix for a
non-zero value — so the pattern matched every fault address except **literal
zero**, the single most diagnostic value in the set. Three functions lost their
reason silently.

Two guards now, both with positive controls: an unreadable verdict line is
reported rather than skipped, and a function that is N seeds short must produce
N reasons. The second is what caught it; the judged-seed cross-check stayed
quiet throughout, because the dropped case was not a judged one.

### 6 of 9 FAILs are the harness reading its own memory

Every `FAIL` in the baseline, run against a stock build and against
`OUT=$PWD/build/oracle-shim EXTRA_CXXFLAGS=-DORACLE_LAYOUT_SHIM=0x51000`:

| | functions |
| --- | --- |
| detail MOVED under the shim — not a lowering bug | 6, of which **2 flip to PASS** |
| detail STABLE — candidate real divergence | 3 |

The three survivors are `?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z`,
`sub_634720` (x87 status word, original `0x0200` vs lifted `0`) and
`sub_63a9d0`. The two that flip to PASS, `?is_known@@YAHHHH@Z` and
`?anything_at@@YAHHH@Z`, are the clearest statement of why the control is
mandatory: without it they would have been reported as lowering bugs.

---

## 2026-07-30 — the generated route learns members and a staged `this`

### Proven count: 54 -> 53, and the DIRECTION is the finding

The suite ran. It produced 9 verdicts before crashing on the 10th function, and
those 9 verdicts cost the project a net proof:

| verdict | functions |
| --- | --- |
| `PASS` | 1 — `?main_caption@MapWin@@QAEXXZ` (0x0046FB10) |
| `INCONCLUSIVE-no-effect` | 8 |

**Two of the 8 inconclusive were already published proofs**: `0x004456A0
?passover_callback@@YAXXZ` and `0x00455E50 ?load_deswin_sprites@@YAXXZ`, both
counted in `proven_recovered`. The effect detector found that no seed made either
side do anything observable — they agreed by both doing nothing. That is the
flattering PASS this route was built to prevent, and it was already inside the
published number.

So a run now demotes as well as promotes, and `proven.csv` went 54 / 6,115 B ->
**53 / 6,086 B**: minus 45 B of vacuous claims, plus 16 B of real one. The one
gain is worth more than the arithmetic suggests — `?main_caption@MapWin@@QAEXXZ`
is the first function this route has ever proven through a STAGED RECEIVER,
which is what the session set out to build.

The remaining 3 of the original 5 keep their markers because the crash stopped
the run before reaching them. They are unverified by this mechanism, not
confirmed by it.

### Proven count by mechanism, after the demotion

| mechanism | functions | source |
| --- | --- | --- |
| `legacy_leaf_island` | 32 | `docs/recovery/proven.csv` |
| `hybrid_runtime`, hand-written | 17 | `src/*_oracle.cpp` markers |
| `hybrid_runtime`, generated | 4 | `src/generated_signature_oracle.cpp` |

After this session: `proven_recovered` **53 fn / 6,086 B**; `unproven_recovered`
**2,499 fn / 190,037 B**. Unrecovered: 2,808 fn / 2,012,914 B.

108 oracles are generated and built; **4 carry markers**. A marker is earned at
RUNTIME, not at generation, so building 104 more oracles bought capacity and not
one point of coverage. Publishing them on the strength of a successful compile is
precisely the mistake that put 37 unearned markers in this tree once.

### Two crashes, both the same lesson about the zero-filled receiver

Run 1 died on its FIRST function: `?close@StringStruct@@QAEXXZ`, unhandled page
fault on read access to `0x00000004` at `0x00401074`. Run 2 got 9 verdicts and
died on the 10th: `?UNK1@PlanWin@@QAEXXZ`, same fault, `0x0048B3C2`.

Zeros make a guard bail safely, and that is a property of the BODY, not of the
seed. A body that walks a pointer chain its constructor guarantees non-null reads
`[this+X]` as 0 and faults on `[0+4]`. Lifecycle methods never have the property
— teardown frees what it finds, construction allocates — so `close`, `init`,
`free`, `destroy`, `release` and their kin are refused by name; `close` alone was
the largest name class in the candidate set, 12 of them. `?UNK1@PlanWin@@QAEXXZ`
is refused individually, with the run that proved it.

Both were identified in one step by the announce-before-calling line, which prints
the name and flushes before each call. Run 1's log contained exactly one
`running` line and no verdicts.

### Shape-addressable set: 5 -> 108 (generated route)

`tools/generate_signature_oracles.py --list`

| gate | candidates |
| --- | --- |
| before: free functions taking (void) | 5 |
| after: + `__thiscall` members, staged receiver | **108** (103 members, 5 free), 6,839 B, after the lifecycle and crash exclusions |

Reach of the route as a whole, from `docs/recovery/functions.csv` against the
`specs[]` table in `src/dllmain.cpp`: 1,994 redirected functions are unproven;
815 are `@@Q` members (25,393 B); **543 are `__thiscall` with arguments (17,803
B)** — the ceiling. 108 is what is buildable today; the gap to 543 is class
sizes, since only 38 classes have a `sizeof` pinned by a `static_assert` in
`src/*.h` and the rest need `tools/derive_class_layout.py` first.

### What the other route can never reach, measured

`legacy_leaf_island` is capped at **41** candidates (6,182 B) at `--max-size
512`, 42 at 1024, **43 uncapped** (8,133 B). Re-measured 2026-07-30 and
unchanged. Raising the cap is nearly worthless: the binding constraints are
`contains_relocation` (**1,059**, down from 1,545 as recoveries left the
unrecovered population) and `non_exact_analysis` (747), not size — `too_large`
rejects only 518.

**Four different candidate counts exist on disk and all four are defensible.
Quote the population, never the number.** Reconciled 2026-07-30 by re-running
the extractor at three caps:

| count | population reviewed | status |
|---:|---|---|
| **41 / 42 / 43** | 2,808 `unrecovered` | current, and what this section means |
| 66 | 2,870 `unrecovered` | a stale `build/<preset>/legacy-leaves/` manifest; the population shrank as recoveries landed |
| 124 | 5,027 `unrecovered` | a much older `.opensmacx/legacy-leaves/` manifest |
| 131 | the unproven **recovered** set, via a patched `select_rows` | a different question, and `docs/HANDOVER.md` labels it as one |

The two stale figures live in gitignored trees and are refreshed by the next
`extract-legacy-leaves`; nothing needs correcting but the habit of quoting a
bare number. `AGENTS.md` used to restate one and now points at the manifest.

A tempting widening was measured and rejected: the extractor gates on
`ghidra_relation != "exact"` (`tools/extract_legacy_leaves.py:430`), excluding
747 functions on metadata rather than code, and 546 of those DO have a second
independent analysis agreeing exactly — IDA9 rather than Ghidra. Allowing it
buys **6 functions / 729 B**, not hundreds; 273 of the 546 have discontiguous
IDA9 bodies. That measurement's own positive control scored 31/32
(`0x004BA830 ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z` is contiguous under Ghidra and
not under IDA9), so even the 6 is soft. Harness kept at
`scratchpad/widen_via_ida9.py`.

### Cross-validation anchor: 26 functions, not the 42 the goal named

The goal proposed validating against the 42 island-ELIGIBLE functions. Those are
all `unrecovered` — no body exists, so they cannot validate anything. The real
anchor is the **26 functions that are already island-PROVEN and argument-taking
`__thiscall` members**: `?move@Win@@QAEHHH@Z`, `?get@Random@@QAEIHH@Z`,
`?compute_thumb_rect@Scroll@@QAEXPAURECT@@@Z`, six `PullDown` item toggles, four
`Vector` operators, three `AlphaNet` id mappers, and the rest. Each has an
independent proof in hand, so a disagreement indicts the new route.

### Bulk recovery: exhausted, and the exhaustion is structural

All ten generators accept **0** new functions (adjustor 130, init 388, atexit
373, delegation 59, nullsub 56, deleting 109, field_accessors 42, global_arith
26, guarded_teardowns 25 — 1,208 functions / 21,771 B already committed).
Residue is 33 functions / 1,146 B, 0.06% of unrecovered bytes.

Of those 1,208 bulk-recovered functions, **1 carries a proof.** Passing all 1,208
to the island extractor with the size cap removed refuses 1,198; only 10 are
eligible (165 B). Thunks are forwarders and the island criteria are exactly *no
calls, no external branches, no relocations*. A leaf-suite unit test is not a
proof — `tools/export_proven_functions.py:29-31` disqualifies it by name.

**2,004 of the 2,808 unrecovered functions are island-ineligible specifically
for containing relocations (1,545), calls (433) or external branches (26).** Any
future bulk generator aimed at that population raises `unproven_recovered` by
construction. That is how the current 2,499 accumulated.

### The gate was never running this suite, and the reason was a display

Measured 2026-07-30, and it invalidates the verdict figures recorded above
rather than adding to them. The smoke gate accepted a result file whose last
line was `all passed`, and phase one wrote that terminator as soon as its own
nine suites passed — while four deferred suites, this one included, were still
to run. The last green gate's result file lists twelve suites with
`generated-signatures` absent and zero `GENERATED-ORACLE-VERDICT` lines in the
log beside it. So the 108 oracles had never run under the gate at all, and the
gate had never said so. Fixed in `8606ea0`: phase one writes `deferred pending`,
the deferred phase rewrites the file after every suite, and the validator refuses
a file missing a suite it was told to expect.

With that honest, the gate failed and named the real cause: **the deferred phase
needs an X display.** Phase-one oracles run inside `DllMain` before any window
exists, so all nine pass headless; the deferred phase triggers from
`scenario_opening_movie`, which the game cannot reach without a window
(`err:winediag:nodrv_CreateWindow`). Run runtime gates with `DISPLAY` set.

Run with a display, the suite produced **17 verdicts** before dying on function
18, `?update_data@Console@@QAEXH@Z` — not 9, and not the 108 the file contains:

| verdict | count |
|---|---:|
| `INCONCLUSIVE-no-effect` | 15 |
| `PASS` | 2 |

One PASS is new evidence: `0x004E25E0 ?pid_2_idx@AlphaNet@@QAEHK@Z`, 39 B, whose
three siblings are island-proven while it never was. proven 53 → 54 / 6,125 B.

Three of the fifteen INCONCLUSIVE addresses are themselves published proofs —
the AlphaNet siblings, by the island mechanism — and demoting those would have
been wrong. `INCONCLUSIVE-no-effect` from this route says only that a zero-filled
receiver observed nothing, which is a statement about these fixtures, not
evidence against a mechanism that ran the original's own copied bytes.
`earned_markers()` already gets this right, and the reason is worth keeping:
it seeds only from proven.csv rows whose evidence names THIS generated file, so
an island proof is never in the set and the discard cannot reach it. Verified by
running it both ways — one marker gained, none lost.

### Open, and needed before the next session claims anything

* 91 of the 108 oracles have still never produced a verdict, and the crash is the
  only reason. **The suite is not slow, and the "180 s run" earlier revisions of
  this file costed a cycle at was never measured.** Instrumented 2026-07-30 with
  `GENERATED-ORACLE-TIMING`: the seventeen functions that run take **0.1 s in
  total — mean 5 ms, median 8 ms, max 12 ms**, so all 108 would be about a
  second. What the window pays for is the game booting to
  `scenario_opening_movie`, which needs ~60 s and is now the configured value.
  A fault guard that turns a page fault into a verdict is the only thing left
  between 17 verdicts and 108, and it is still not built.
* 15 of the 17 verdicts are INCONCLUSIVE-no-effect. A zero-filled receiver gets
  past almost no guard, which is the honest cost of the safe seed. The fix is the
  per-function field seed the hand-written suites use - 0x45454545 into Scroll's
  offset 0xC4 is the model - and no signature supplies it. Note where the suite
  runs from: `scenario_opening_movie` fires BEFORE any save is loaded, so these
  oracles meet the emptiest state the process ever has. Running them after
  `refresh_loaded_game()` would give the same fixtures a real object graph, and
  the hook already exists in `src/scenario.cpp`.
* `unproven_recovered` may have a floor well above zero: the thunk cohort has no
  observable effect for a runtime differential to latch onto, and the
  `init`/`atexit` bodies push the ORIGINAL address through a shared seam, so the
  recovered twin is not what runs. Measure that floor before treating the number
  as reducible to zero.
* `tools/recovery_frontier.py` has no `test_*.py` (it is not alone in that —
  about a dozen tools/ scripts lack an exact-name test counterpart), but it is
  the untested one whose output matters: the census figures come out of it.
