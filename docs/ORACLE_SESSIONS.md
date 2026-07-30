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

### Proven count by mechanism (was: 54 unchanged)

| mechanism | functions | source |
| --- | --- | --- |
| `legacy_leaf_island` | 32 | `docs/recovery/proven.csv` |
| `hybrid_runtime`, hand-written | 17 | `src/*_oracle.cpp` markers |
| `hybrid_runtime`, generated | 5 | `src/generated_signature_oracle.cpp` |

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

### Shape-addressable set: 5 -> 122 (generated route)

`tools/generate_signature_oracles.py --list`

| gate | candidates |
| --- | --- |
| before: free functions taking (void) | 5 |
| after: + `__thiscall` members, staged receiver | **108** (103 members, 5 free), 6,839 B, after the lifecycle and crash exclusions |

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
