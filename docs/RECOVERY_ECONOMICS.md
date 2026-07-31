# Recovery economics

What a recovery costs, measured. This exists because the project's schedule, its
choice of targets and its decisions about which tooling to build all rest on a
rate, and for most of the project's life that rate was inherited rather than
measured. Two estimates that were carried for months are refuted below.

Every figure here was measured on this host on 2026-07-31 unless dated
otherwise. Where a number is derived rather than observed it says so.

## The measured rate

Five zero-seam functions were recovered end to end, each timed:

| function | bytes | s/byte | commit |
|---|---:|---:|---|
| `?stack_veh@@YAHHH@Z` | 1,583 | 2.27 | `0bbe844` |
| `?action_home@@YAHH@Z` | 2,239 | 1.08 | `748220c` |
| `crop_yield` + `mine_yield` + `energy_yield` | 3,905 | 0.97 | `004393d` |

`stack_veh` was first-of-kind and carries the learning cost. **Steady state is
≈1.0 s/byte, or ≈3,600 B/agent-hour.**

### What that does not license

Those five were the *easiest available*: zero seams, every callee already
`source_complete`, prototypes in hand, 1–4 kB. That population is the zero-seam
frontier — 969 functions / 216,328 B — which at this rate is about 60
agent-hours. **The remaining ~2 MB has no measurement of any kind.** It is
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

- **ccache does not help.** Every mutant is by construction a unique source
  file, so it is a guaranteed cache miss. The unchanged ~94 other translation
  units are not recompiled anyway. The one real case is repeated hardening
  passes, where mutant *K*'s object is identical across passes.
- **A faster linker does not help.** Linking is 0.09 s, under 4% of a mutant.

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
