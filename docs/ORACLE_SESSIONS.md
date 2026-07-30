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

## 2026-07-30 — the generated route learns members and a staged `this`

### Proven count: 54 (unchanged this session)

| mechanism | functions | source |
| --- | --- | --- |
| `legacy_leaf_island` | 32 | `docs/recovery/proven.csv` |
| `hybrid_runtime`, hand-written | 17 | `src/*_oracle.cpp` markers |
| `hybrid_runtime`, generated | 5 | `src/generated_signature_oracle.cpp` |

`proven_recovered` 54 fn / 6,115 B (3.12% of recovered bytes);
`unproven_recovered` **2,498 fn / 190,008 B (96.88%)**. Unrecovered: 2,808 fn /
2,012,914 B.

**The count did not move, and that is the honest outcome.** 117 new oracles were
generated and built, but a marker is now earned at RUNTIME, not at generation
time, and the suite has not yet been run to completion. Publishing 117 markers
on the strength of a successful compile is precisely the mistake that put 37
unearned markers in this tree once.

### Shape-addressable set: 5 -> 122 (generated route)

`tools/generate_signature_oracles.py --list`

| gate | candidates |
| --- | --- |
| before: free functions taking (void) | 5 |
| after: + `__thiscall` members, staged receiver | **122** (117 members, 5 free), 8,935 B |

Reach of the route as a whole, from `docs/recovery/functions.csv` against the
`specs[]` table in `src/dllmain.cpp`: 1,994 redirected functions are unproven;
815 are `@@Q` members (25,393 B); **543 are `__thiscall` with arguments (17,803
B)** — the ceiling. 122 is what is buildable today; the gap to 543 is class
sizes, since only 38 classes have a `sizeof` pinned by a `static_assert` in
`src/*.h` and the rest need `tools/derive_class_layout.py` first.

### What the other route can never reach, measured

`legacy_leaf_island` is capped at **41** candidates (6,182 B) at `--max-size
512`, 42 at 1024, **43 uncapped** (8,133 B). Raising the cap is nearly
worthless: the binding constraints are `contains_relocation` (1,545) and
`non_exact_analysis` (747), not size.

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
construction. That is how the current 2,498 accumulated.

### Open, and needed before the next session claims anything

* The suite has not been run. Until it is, the verdict lines are untested code
  and the 117 members are capacity, not coverage.
* `unproven_recovered` may have a floor well above zero: the thunk cohort has no
  observable effect for a runtime differential to latch onto, and the
  `init`/`atexit` bodies push the ORIGINAL address through a shared seam, so the
  recovered twin is not what runs. Measure that floor before treating the number
  as reducible to zero.
* `tools/recovery_frontier.py` is the only script in `tools/` with no
  `test_*.py`, and the census figures come out of it.
