# Recovery economics

What a recovery costs, measured. This exists because the project's schedule, its
choice of targets and its decisions about which tooling to build all rest on a
rate, and for most of the project's life that rate was inherited rather than
measured. Two estimates that were carried for months are refuted below.

Every figure here was measured on this host on 2026-07-31 unless dated
otherwise. Where a number is derived rather than observed it says so.

## The measured rate

Eleven zero-seam functions were recovered end to end. The first three rows were
timed by hand; the rest are commit-to-commit intervals, which is the harsher
measure because it charges every recovery for its own integration:

| function | bytes | s/byte | commit |
|---|---:|---:|---|
| `?stack_veh@@YAHHH@Z` | 1,583 | 2.27 | `0bbe844` |
| `?action_home@@YAHH@Z` | 2,239 | 1.08 | `748220c` |
| `crop_yield` + `mine_yield` + `energy_yield` | 3,905 | 0.97 | `004393d` |
| `?base_support@@YAXXZ` | 1,530 | 0.46\* | `a02d8cb` |
| `?world_site@@YAHHHH@Z` | 1,509 | 1.29 | `5616b33` |
| `?num_objectives@@YAHHH@Z` | 1,161 | 0.76 | `460fa52` |
| the three `spot_*` leaves | 845 | 2.31 | `8849bcc` |
| `?spot_loc@@YAXHHHH@Z` | 821 | 0.54 | `cc19992` |
| `?reset_territory@@YAXXZ` | 1,074 | 1.56 | `5ad12fa` |

\* understated: its analysis overlapped an unrelated commit that landed between.

`stack_veh` was first-of-kind and carries the learning cost. Over all eleven,
**14,667 B in 17,397 s = 1.19 s/byte, or ≈3,030 B/agent-hour.** The earlier
five-function figure of ≈1.0 s/byte was optimistic by about 20%, and the spread
per function is 0.5–2.3 s/byte — do not price a single function from the mean.

### Blockers are a cost line, not an anomaly

Recovering flushes out defects in already-committed code, and paying for them is
part of the rate. Two of the nine commits in one run were blockers found this
way, costing **2,984 s — 15% of that run's total**:

| blocker | cost | what it was |
|---|---:|---|
| `205a230` | 1,964 s | a `.def` rename broke `stage-hybrid-game`; the staged exe's import table is frozen |
| `8b67ada` | 1,020 s | a test truncated a committed CSV to 0 bytes under the other gate lane |

Charging those to the bytes gives **1.39 s/byte all-in**. That is the number to
plan with; 1.19 is the number to compare authoring approaches with.

### What that does not license

Those eleven were the *easiest available*: zero seams, every callee already
`source_complete`, prototypes in hand, 1–4 kB. That population is the zero-seam
frontier — 969 functions / 216,328 B — which at 1.19 s/byte is **72 agent-hours,
or 84 with blockers charged in**. (The earlier estimate of ~60 h came from the
optimistic 1.0 figure.) **The remaining ~2 MB has no measurement of any kind.** It is
seam-bearing, contains a 170-function SCC with no leaf by construction, and
941 functions reach DirectDraw/DirectPlay/DirectSound (see
[EXCLUSIONS.md](EXCLUSIONS.md)). Publish the frontier figure; do not extrapolate
it across the remainder.

## Two refuted estimates

**147 B/agent-hour is retired.** It was inside the historically measured band,
so it was not invented — but 42.5% of the bytes behind it came from bulk
generators that have returned zero candidates since 2026-07-30. Carrying it
forward projects an exhausted mechanism. The non-generator historical rate is
**73–93 B/agent-hour**, measured over 238 commits and 573 hand-recovered
functions of mean size 63 B.

**Cost is not per-family.** The prediction was that functions sharing a fixture
would be dramatically cheaper per byte, because resolving globals and building a
drivable fixture is a one-off. Three yield functions sharing one fixture came in
at 0.97 s/byte against a single function's 1.08 — **a 10% discount, not the 3×
predicted.** The fixture is not where the time goes.

## Where the time actually goes

Cost tracks **mutant count**, which is roughly one per 18–22 bytes (90 mutants
for a 1,583 B body, 103 for 2,239 B), plus branch asymmetry. So it is closer to
per-byte than per-function.

Per mutant, before the fixes below:

| phase | time | share |
|---|---:|---:|
| write the mutated source | 0.000 s | 0% |
| build (compile one file 0.48 s + link 0.09 s) | 0.575 s | 30% |
| `ctest` | 1.330 s | 70% |

The sweep's share of a whole recovery ranged from ~30% (`action_home`, ~2
hardening passes) to **81%** (the yield cluster, 295 mutants × 3 passes). The
implementation is the cheap half: all five recoveries compiled early, and of the
39 mutants that survived the first `stack_veh` sweep, **every one was a hole in
the test, not a defect in the body**.

Two consequences for anyone optimising this:

- **ccache does not help — but not for the reason first given here.** The claim
  was "a guaranteed cache miss". Measured 2026-08-01, a second fresh build
  directory gets **170/170 hits**, so the hits happen; they are just worth
  little, because compiling is not what a fresh build spends its time on. Cold
  build 21 s, with a warm ccache 18 s: **3 s, 14%**. Not worth wiring.
- **A faster linker does not help**, and this now rests on a current number
  rather than the old 0.09 s: a one-file rebuild *and* link of the 5.4 MB
  `recovery-gameplay-tests.exe` is **241 ms** end to end.

### Re-measured 2026-08-01: the build is 11% of a mutant

| step | time | share |
|---|---:|---:|
| rebuild one source file and link | 241 ms | 11% |
| `ctest` run of that one target | 1,989 ms | **89%** |

**Every optimisation proposed for this loop so far — ccache, a faster linker,
sharding across lanes — attacks the 11%.** That is why each measured small. The
test execution is the cost, and it is one Wine-launched executable.

**A lane is far cheaper than recorded, and that correction cuts the other way.**
"~7 s and ~2 GB" omitted the build a fresh lane needs. That build is **21 s**,
not the minutes assumed when auto-sharding was deferred, so lane setup is ~30 s
all-in and parallel across lanes — negligible against any sweep of minutes.

**One number is unreconciled and must not be planned against.** This worktree
measures **~2.2 s per mutant** (0.24 + 2.0). The 2026-08-01 batch reported
~20 min for 55 mutants, i.e. **~21 s per mutant** — 10× apart. Extra hardening
passes and `--confirm-survivors` re-runs account for some of it, not for an
order of magnitude. Whether sharding is worth building depends entirely on
which figure holds: at 21 s a 55-mutant sweep is 20 minutes and 4 lanes are
clearly worth 30 s of setup; at 2.2 s it is two minutes and they are not.
Measure a real sweep in the main tree before building anything.

## What was made faster, and by how much

| change | measured | commit |
|---|---|---|
| Stop the Wine prefix after every test | **1.53×** on a sweep | `364aec8` |
| `RESOURCE_LOCK` the 7 Wine tests, `ctest --parallel 8` | **1.50×** debug, **1.87×** release | `ec24c12` |
| Run the two preset gates concurrently | **1.75×** — 332 s → 190 s | `7ff7912` |
| `--shard I/N` on the mutation sweep | **3.16×** at four lanes, 79% efficiency | `2a0a526` |

The first is the one worth reading about. `--reuse-owned-wine-prefix` existed to
skip teardown, and it made every test **four times slower**: 0.336 s → 1.33 s.
The cause is not Wine and not the teardown it skips (`wineserver -k` on a live
session is 0.053 s). The wineserver and its six service processes inherit the
runner's stdout and stderr, so a *retained* session holds the write end of
CTest's pipe open, and CTest reads that pipe to EOF. The child exits at 0.33 s;
the pipe reaches EOF at 1.38 s. **1.05 s was being bought to save 0.053 s.**

Three hypotheses were measured and killed before that one was found: the failure
path (a failing suite costs the same as a passing one, 0.343 vs 0.348 s), bare
Wine startup (0.042 s whether or not a server is alive), and
`prepare_owned_wine_prefix`'s unconditional teardown (patching it changed
nothing). Recorded because each looked obviously right.

## Sharding a sweep

Mutants are independent; they were serial only because they share one source
file and one binary. A worker therefore needs **its own source tree**, not just
its own build directory. A lane costs ~7 s and ~2 GB:

    git archive HEAD | tar -x -C "$S" --one-top-level=lane$i
    mkdir -p "$S/lane$i/.opensmacx"
    cp -al <repo>/.opensmacx/game "$S/lane$i/.opensmacx/game"
    cd "$S/lane$i" && cmake -S . -B build -G Ninja \
        --toolchain cmake/toolchains/mingw-i686.cmake

Hard links rather than a symlink: the build rejects a symlinked
`OPENSMACX_GAME_DIR` and rejects one outside the lane's own ignored root, and
hard links satisfy both for free. Disk binds before CPU.

**A shard is not a sweep.** Each shard prints its own tally, and `survived 0`
from one shard is indistinguishable from a clean full sweep. The tool announces
this on every sharded run; all N shards must be run and their survivors unioned.

## The calendar

Two cost models remain unfalsified, and the gap between them is the finding
rather than a range to average:

| model | basis | remaining | days @ 6 agents |
|---|---|---:|---:|
| per-byte | 73–93 B/h, historical, on functions of mean 63 B | 21,785–27,753 agent-h | 497–634 |
| flat per function | 0.87 h/fn, same population | 2,443 agent-h | 56 |

**56 to 634 days is an unresolved model choice, not noise.** The five recoveries
above favour the per-byte model, at a rate 39–49× the historical one — but on
the easiest tenth of the work.

The distribution is what makes a single number dishonest: of **2,803**
unrecovered functions / **2,005,187 B**, only **18.9%** of the bytes (2,126
functions / 379,871 B) sit in the ≤621 B class where any agent-era observation
exists. The other **81.1%** has none.

Do not publish a single-number calendar. Publish the frontier estimate and an
explicit unknown for the rest.

## A verification route whose cost does not track size

Everything above prices *recovery*. Verification is priced separately, and both
existing routes scale with the body: the differential oracles by mutant count,
the SMT prover by decidability. **Byte matching would not** — a body that
recompiles to the original instruction sequence costs O(1) to believe,
whatever its size, calls, loops, COM or SEH.

That was tested with the compiler SMACX shipped with, VC6 RTM 12.00.8168, under
a pre-registered rule. Five `source_complete` struct-free leaves scored **0/5**
on the pre-registered flags (`/O2` omits the frame pointer the shipped image
keeps) and **3/5** once `/Oy-` was added — three exact mnemonic-and-length
matches across three calling conventions, including the original's strength
reduction of `× 10000` and its magic-number `/ 10000`. One compile is 0.234 s.

The two misses were both the recovered *source*, not the compiler, and one of
them is a bug the route found: `?bitmask@@YAXHPAHPAH@Z` divides signed in the
original and was recovered as `uint32_t`, a difference invisible to any oracle
over non-negative inputs. Repairing it in its own later change took the score
to **4/5** and also corrected a `.def` export that had never matched any IDB
name. Full result, controls and the pre-registration:
[BYTE_MATCH_ROUTE.md](BYTE_MATCH_ROUTE.md).

**The cost that matters is not the tooling.** It is that a body must be written
to *match* rather than to *behave*, and all 2,557 already-`source_complete`
functions were written to behave.

## Cancelled on this evidence

- **`tools/integrate_batch.py`.** One full gate cycle is 423 s = 0.118
  agent-hours, against 7.7–9.8 agent-hours for one mean remaining function, so
  the gate is **1.2–1.5% of per-function cost** and batching eight saves ~1% of
  the programme. Two premises behind it were also simply wrong:
  `promote-recovery-metadata` is **3.05 s** per recovery, not 312 s (that figure
  is a cold IDB-reopen path a recovery never triggers), and the mutation sweep
  is not dominated by huge mutant counts — the *median* function has 4, because
  that median is computed over already-recovered functions of median size 22 B.
  Revisit only if the flat-cost model wins.
- **Seam-relief as a track.** 969 zero-seam functions / 216,328 B are authorable
  now, which is 53–68 agent-days before a seam binds on anything.

## Most of what has been recovered is not watched by anything

Measured 2026-08-01. A random sample of **eight** recovered functions of ≥200 B
(seed 20260801, so it re-draws identically) was re-swept with
`tools/mutate_and_verify.py`:

| function | bytes | killed | survived |
|---|---:|---:|---:|
| `?social_ai@@YAXHHHHHPAH@Z` | 3,714 | **0/709** | 709 |
| `?sensors@Path@@QAEHHPAHPAH@Z` | 1,418 | **0/114** | 114 |
| `?make_proto@@YAXHHHHHH@Z` | 1,321 | **0/170** | 170 |
| `?set_course@@YAXHDHH@Z` | 754 | 33/43 | 10 |
| `?suggest_plan@@YAHHH@Z` | 733 | 57/61 | 4 |
| `?success_rates@@YAHHHHH@Z` | 574 | **0/77** | 77 |
| `?base_making@@YAHHH@Z` | 258 | **0/30** | 30 |
| `sub_5b5700` | 202 | 12/13 | 1 |

**Five of the eight are observed by nothing.** No assertion anywhere kills a
single mutant, including mutants that delete a whole statement. Over the sample,
**102 of 1,217 mutants die — 8.4%**.

### The whole population, measured 2026-08-01

The sample was then replaced by a census. `tools/measure_observability.py` asks
the weaker question — *does anything observe this at all* — for **one build per
function** instead of ~152, by wrecking every perturbable line at once and
running the suite once. That is what made asking it over the corpus feasible:
~30 minutes rather than ~8 hours.

| | functions | |
|---|---:|---|
| measured (≥200 B, `source_complete`) | **215** | |
| OBSERVED | 71 | at least one assertion depends on the body |
| **UNOBSERVED** | **144 (67%)** | nothing distinguishes the body from a wrecked one |
| not measurable | 9 | the maximal wreck will not compile |

**The first figure published here was 141/193, i.e. 73%, and it was too
pessimistic.** 25 functions were excluded as unmeasurable because their source
compiles into three test executables — `buffer.cpp` reaches the gameplay, leaf
*and* oracle suites — so `resolve_target` correctly refused to guess one. Giving
the prober a `--target` override and pointing it at `recovery-leaf-tests`
resolved 22 of them: **19 OBSERVED**, 3 unobserved, 3 that will not compile
wrecked. That population was hiding real coverage rather than more debt.

Recording them as UNOBSERVED to tidy the census would have published a worse
number than reality with no way to discover the error. This is the second time
in one day that refusing to guess was what made a correction possible.

The largest are `?battle_compute@@YAXHHPAHPAHH@Z` (10,020 B),
`?tech_val@@YAHHHH@Z` (4,133), `?read_rules@@YAHH@Z` (3,804),
`?read_faction@@YAXPAUPlayer@@H@Z` (3,739) and `?social_ai@@YAXHHHHHPAH@Z`
(3,714).

> **THESE COUNTS ARE BOUNDS, NOT MEASUREMENTS, and the error runs the
> reassuring way.** A third audit found that `measure_observability.py` scores
> OBSERVED on ANY non-zero exit from the test binary, because
> `mutate_and_verify.Harness.check()` returns FAILED for a segfault, an abort
> and an assertion failure alike. A wrecked body that FAULTS - a nulled vtable
> literal, a zeroed divisor - therefore counts as observed with no assertion
> having examined it. So **71 OBSERVED is an upper bound and 144 UNOBSERVED a
> lower bound**: the real coverage is no better than stated and may be worse.
> Fixing it means having `check()` report why the run failed and scoring only an
> assertion failure as OBSERVED. Until then, do not quote 67% as a measurement.

**The instrument was validated in both directions before these numbers were
believed**, because a prober that can only say UNOBSERVED is a broken tool that
looks like a dramatic finding: `?repair_phase@@YAXH@Z` kills 118 of 126 mutants
and is reported OBSERVED; `?social_ai@@YAXHHHHHPAH@Z` kills 0 of 709 and is
reported UNOBSERVED.

It also finds coverage a name grep cannot. `sub_5b5700`, `?is_sensor@@YAHHH@Z`
and the `Dialog`/`Dialogs`/`Wave` destructors are named in no test and come back
OBSERVED, because they are reached through a caller. That is why the 72%
stem-matching estimate is an upper bound and this 73% census is not the same
number arrived at twice.

### THE DEBT IS HISTORICAL, WHICH IS THE ONLY GOOD NEWS HERE

The three functions recovered on 2026-08-01, re-swept under the strengthened
constant operator:

| function | killed | |
|---|---:|---|
| `?repair_phase@@YAXH@Z` | 118/126 | 94% |
| `?alien_base@@YAHHHH@Z` | 46/49 | 94% |
| `?suggest_plan@@YAHHH@Z` | 57/61 | 93% |

So the current process produces observed recoveries; it is the accumulated
corpus that is unwatched. That distinction decides the response. Blocking
progress on ~131 retroactive fixtures would stop the objective dead, and it is
not warranted — but the count must be published and must not be allowed to grow,
which is the same ratchet shape as `BASELINE` in `audit_export_signedness.py`.

Each of the five was checked three ways before being called unobserved: named by
no file under `tests/`, absent from `src/generated_signature_oracle.cpp`, and
absent from `docs/recovery/proven.csv`. `src/veh.cpp` compiles into exactly one
test executable, so no second suite could have been covering the two functions
there.

**Do not read this as an artifact of the off-by-one operator added the same
day.** A function nothing observes kills nothing under any operator, and the
previous zero-only version would have reported the same. Re-running the harness
over already-landed work is simply the first time the question was asked.

**And do not substitute a name grep for the sweep.** `sub_5b5700` is named in no
test and still scores 12/13, because it is reached through a caller. A
stem-matching estimate puts 162 of 224 recovered functions ≥200 B (72%) beyond
the reach of any test, oracle or proof; that is an UPPER BOUND on what is
unobserved, and the measured 5/8 is the number with evidence behind it.

### Why this belongs in an economics document

`machine_carried` counts bytes that have source. It does not ask whether
anything observes that source, so **the objective can be driven to zero with
most of the corpus unwatched**. That is permitted — proof here is explicitly
opportunistic and never a gate on progress — but it has never been priced.

The honest reading is that a recovery has two costs, and the published rate
covers one of them. The functions in this sample that ARE observed
(`set_course`, `suggest_plan`, `sub_5b5700`) each carry a hand-written fixture;
the five that are not were authored and wired without one. Whatever
bytes-per-agent-hour figure is quoted, it is a blend of those two populations,
and the cheaper one delivers source that no assertion checks.

A mechanical companion metric would fix this: the count and byte total of
recovered functions for which **at least one mutant dies**. Unlike a name grep
it cannot be satisfied by a test that merely mentions a symbol. It costs one
sweep per function, so it wants computing incrementally as recoveries land
rather than retroactively over the whole corpus.
