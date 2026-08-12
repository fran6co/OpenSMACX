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

## 2026-07-31 — the staging route pays: 83 proven becomes 89

Both mandatory numbers moved, in the right order. The reachable set grew first
(108 oracles -> 118) and then was spent: **proven_recovered 83 -> 89 functions,
7,680 -> 7,879 bytes, 3.92% -> 4.02% of recovered bytes.**

### What the bounded oracles returned

All ten produced a verdict: **6 PASS, 4 INCONCLUSIVE-no-effect, 0 FAIL.** Six
were addresses no route had ever proven:

| address | function |
| --- | --- |
| 0x00483820 | `?UNK5@NetWin@@QAEXXZ` |
| 0x004B9F90 | `?set_loc@StatusWin@@QAEXHH@Z` |
| 0x004BA720 | `?UNK1@TutWin@@QAEXXZ` |
| 0x004BDDD0 | `?UNK3@TutWin@@QAEXH@Z` |
| 0x004C5920 | `?is_disabled@Midi_Device@@QAEHXZ` |
| 0x004C5A80 | `?set_codec@Wave_In_Device@@QAEHK@Z` |

Every one belongs to a class with NO pinned `sizeof` - NetWin, StatusWin,
TutWin, Midi_Device, Wave_In_Device - which is to say they are precisely the
functions the exact-size requirement had been excluding. A bounded PASS is no
weaker than a pinned one: both compare the original and the recovered body on
the same staged buffer, and the bound decides only how much zero-filled room
they were given.

Whole run: 118 verdicts, 50 PASS, 45 INCONCLUSIVE-no-effect, 22
INCONCLUSIVE-original-faulted, 1 FAIL.

### The gate still fails, and is left failing

The one FAIL is `?clear_links@Buffer@@QAEXXZ` at 0x005DEF90. `Buffer`'s size is
PINNED, so it is not a staging artifact, and it has been open since the
fault-guard session. A FAIL fails the suite by design, so the gate reports
failed and stays that way rather than being quieted.

### Correction: the smoke duration is not a measurement variable

Both CMake caches were handing `--duration 20` while the source has said 60
since the gate was widened, and the previous commit here warned that fixing it
would make runs incomparable, "because it changes how far the game gets before
the deferred phase".

Re-run at 60 s, the distribution is **identical**: 118 verdicts, 50 / 45 / 22 /
1, the same in every bucket. The caution was wrong. The deferred phase runs
from `scenario_opening_movie` at a fixed point in startup, so the extra forty
seconds happen entirely AFTER the verdicts exist, and runs at 20 s and 60 s are
directly comparable. Worth knowing in both directions: the duration cannot buy
more coverage either.

## 2026-07-31 — 2.3 was closed against the wrong evidence; the IDB has the layouts

Phase 2.3 was closed here on seven mechanisms, each with a control, all
near-zero: allocation, vbtable, array stride, constructor zeroing, copy
constructor / `operator=` (which do not exist in this image at all), `imul` by
the class size (0 right, 2 wrong), and the stride between global instances
(0 right, 1 wrong). The structural argument looked sound - for a single global
object the compiler needs a size at allocation and at array destruction, and
there is neither, so it never appears in code.

All seven searched the EXECUTABLE. The answer was in the analysis directory.

`.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb`, the original
repository's database, carries IDA struct definitions for these classes.
`python-idb` is already pinned and can reach them through the raw ID0 b-tree,
even though its IDC shim exposes no struct API at all.

The discriminator is the netnode id. A FUNCTION name resolves to a netnode
whose id is its address - `??0StringStruct@@QAE@H@Z` gives 0x401000. A CLASS
name resolves into the 0xFF000000+ struct-id space - `Console` gives
0xff034e43. Lookups for invented names raise, so the distinction is real and
not an artifact of the reader.

Under tag `M` the member table is IDA packed dwords: flags, member count, then
five-tuples whose third element is the member's size. Summing those and
aligning to 4 reproduces the struct size.

**Not usable yet, and the control says why.** On the 40 classes with a pinned
size: **31 right, 2 WRONG, 7 silent.** `Console` misses by exactly 4 and
`PullDown` reads 0xa14 against a true 0xf40, so the five-tuple stride is wrong
for some member kind - nested structs, unions or arrays carrying an extra
packed field. The bar here is ZERO wrong, because a wrong layout compiles
perfectly and corrupts memory at runtime, so this stays a lead until the parse
is tightened.

What it is worth if it lands: the IDB defines a struct for **54 of the 73
blocked classes, covering 299 of the 420 blocked members**.

Two defects in it are now understood rather than guessed at. `PullDown` has
`count = 1`: the IDB defines ONE member of 0xa14 against a true 0xf40, so that
struct is an incomplete hand-reconstruction. `Console` parses 145 of 145
members cleanly and still sums 4 short. So the IDB yields a LOWER bound that is
usually exact - which is the UNSAFE direction for staging, and the reason it
cannot be adopted raw. A class whose IDB sum equals its next-global upper bound
is pinned exactly by two independent sources agreeing, and that is the shape
the campaign should use.

### How far the IDB parse actually gets, and why it stops there

Pushed further with a better reader. python-idb's `get_tag_entries(tag="M")` is
reliable where hand-rolled b-tree cursors silently drop entries - walking the
raw prefix returned only xrefs for `Sprite` and only members for `Console`,
which is a bug in the walking and not in the database.

The header word of the member table varies - 0x1000 for Console and MapWin,
0x1110 for Sprite, 0x1010 for Font - and it is IDA's SF_ALIGN field, bits 7..10,
holding alignment as a power of two. Sprite's 0x1110 decodes to align 4, which
is exactly the 0x29 -> 0x2c it needed. Decoding it moves the control from 31
right to **33 right, 2 wrong, 5 silent**.

It cannot get to zero, and the reason is data rather than parsing. `PullDown`
declares ONE member of 0xa14 against a true 0xf40: the struct is an incomplete
reconstruction, no parser recovers a member that was never entered, and nothing
distinguishes the 54 unvalidated blocked classes from that case. `Console`
remains 4 short with all 145 members parsed.

Combining the two directions does not rescue it either. The IDB gives a LOWER
bound and the next-global gives an UPPER one, so where they meet the size would
be pinned by two independent sources - but they never meet: **0 of the 40 known
classes** have IDB-lower equal to global-upper, because the upper bound measures
to the next CONSTRUCTED global and is far looser than the object.

So the IDB is a good lead that stops below the bar. What survives is the
upper-bound staging route, which needs no IDB at all.

### The other database cannot be opened here

`.opensmacx/game/terranx.exe.i64` is the manually built one and, checked, it
does NOT carry the game layouts: its exported C header defines 66 structs, all
Windows SDK types - `HWND__`, `tagWNDCLASSA`, `_EXCEPTION_RECORD` - with **zero
overlap** with the 40 pinned classes. Bigger meant more auto-analysis, not more
human work.

The database that does have the layouts is the older 32-bit `.idb`, and IDA
Free refuses it: 32-bit databases need IDA Pro. IDA Free also ships no
IDAPython, only IDC.

**Superseded on 2026-08-06, and the premise with it.** Both of those facts are
about getting *into* IDA, and nothing here ever needed to.
`tools/export_idb_members.py` reads the container directly through `python-idb`
and yields member names and OFFSETS - 13,078 members across 124 classes - where
the script-menu route yielded a name and a total size. So the three tools built
around IDA's own doors are gone: `export_idb_struct_sizes.py` and its `.idc`
twin, which had to be run from the script menu, and `sizes_from_ida_header.py`,
which turned a C-header export into sizes and could never receive one here.

The lesson is cheap and worth writing down: a licence looked necessary because
the obvious door was locked, and the container was readable the whole time.

### Thinker was checked, and it is the weaker source

The Thinker mod carries reverse-engineered headers for the same binary, and the
repository policy already covers it: GPLv2 against this project's
GPLv3-or-later, so its text can never be copied or committed regardless, and
`tools/correlate_thinker_layouts.py` reduces fetched headers to ignored
hypothesis CSVs that each require independent verification.

Verified against the 40 known sizes, the two hypothesis sources rank clearly:

| source | right | wrong |
| --- | ---: | ---: |
| Thinker headers | 5 | **7** |
| the IDB | 31 | 2 |

Thinker is wrong more often than right where it has an opinion, and the
failures are not near-misses: `Buffer` 0x1c against 0x588, `Font` 0x8 against
0x28, `Win` 0xc8 against 0x444. Those are NAME COLLISIONS rather than layout
disagreements - Thinker models the game's data structures (`MapTile`, `BASE`,
`MAP`), and where a name coincides with a UI class it is an unrelated type. It
overlaps only 6 of the 73 blocked classes.

Adopting it would have introduced wrong layouts into a campaign whose whole
risk is that a wrong layout compiles perfectly and corrupts memory at runtime.
The verify-independently rule is what caught it, on the first check. That is the
population no mechanism searching the executable could reach, because the
information was never in the executable.

A cheaper fallback exists for the rest, and it comes from noticing that 2.3 had
been asking the wrong question. The generator gate demands a PINNED EXACT size,
but the oracle only needs enough bytes for a zero-filled receiver, and it
stages into a fresh `static uint8_t staged[ObjectSize]` rather than over the
real global - so over-allocating is safe and only under-allocating corrupts.
Next-global UPPER bounds are therefore usable: 21 of the 73 blocked classes
have one in a sane range, covering 162 members, and on the pinned classes the
bound was at or above the true size 13 times and below it 0 times.

## 2026-07-31 — the best state result on record, and it trips the kill criterion

The hybrid-dump route was rejected long ago on two measurements that are now
known to be unsound: the "only 26-30% of address-shaped words land in-span"
figure came from the whole-window residency metric this project corrected
yesterday, and every sweep that tested it ran through a remapper that was
rewriting 13,046 static-data words per run. Re-run with both fixed, on the same
`state-game.bin` dump, changing ONE variable:

| | fault wall | vs no state |
| --- | ---: | ---: |
| no state | 1,584,976 B | — |
| game state, old remapper | 1,612,851 B | **−27,875 worse** |
| game state, fixed remapper | **1,457,339 B** | **+127,637 better** |

The remapper fix alone is worth 155,512 B on this state, and it flips the
verdict's sign. This is 6.6x the best `--build-state` ever produced and 51.1% of
the 250,000 B threshold - by a wide margin the largest movement any state
experiment in this project has achieved.

Measured with the corrected residency metric, the dump also carries far more
real state than anything the CRT builds: 279,120 changed words of which 68,116
are address-shaped and 28,841 in-span, against `--build-state`'s 5,626 and
2,716. That is the diagnosis from yesterday confirmed from the other side - the
object graph is built by `WinMain` and gameplay, and a dump taken there has ten
times the usable pointer state.

**And it is refused.** `agreed_full_strength` goes 42,209 -> 40,191 B across 768
-> 674 functions. The pre-committed kill criterion is "full strength worse by
>2,000 B or >20 functions"; both are exceeded, by 18 B and by 74 functions.
`agreed_only_on_paths_taken` also rises 60,248 B, so a large part of what looks
like progress is evidence bought weakened.

So the state is not adopted. Recording why that matters: +127,637 B on the
headline debt is an extremely easy thing to call success, and without a
threshold fixed in advance it would have been called that. The trade this makes
- more code reached, less code genuinely agreeing - is exactly the one the
criterion exists to refuse.

What is worth keeping is the remapper finding, which is not about this state at
all: every state experiment this project has ever run was measured through the
corruption, so their results are all suspect in the same direction.

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

### Carrying the heap too, and it does not help either

Of the 24 functions that moved near-null → wild, 20 (27,124 B) fault on a base
inside the R1 guest heap: `0x00b12281`, a real object a constructor built. The
dump window stops at 0x009C2200, so the *pointer* survived into the state and
the *object* did not. That is a harness gap, so it was closed — `--build-state`
now writes a `.heap` beside the dump, `--state` loads it, and the per-case reset
restores the heap as a second range. (Not one contiguous range: side A never
commits the guest stack span between image and heap, and walking through it
takes the harness down inside `memcmp`.)

It works, and it changes nothing worth having. That function's fault moves
deeper — a different instruction, reading a different address — so the object is
genuinely there now. In aggregate:

| | fault wall | vs no state |
| --- | ---: | ---: |
| no state | 1,584,976 B | — |
| state, no heap | 1,565,790 B | +19,186 B |
| state **with** heap | 1,569,464 B | **+15,512 B** |

Slightly *worse* than without it, and 6.2% of the 250,000 B threshold. Three
variants of the same idea now agree with each other: the fault wall does not
come from state the CRT builds. 1,806 of 1,838 near-null-faulting functions are
untouched by any of them.

`agreed_full_strength` is unchanged at 42,209 B / 768 fn throughout, so nothing
regressed and the kill criterion was never triggered. `--build-state` stays as a
working instrument — the boot will want it — but it is not the win Phase 2.1
projected, and no further tuning of it is worth a session.

### The remapper was rewriting data, and that part is now fixed

`overlay_state` rewrote every out-of-span word in `[0x10000, 0x20000000)` into
the arena, on the assumption that an address-shaped word is a pointer. The
analysis above disproves it, and the fix follows from the same idea: **only a
word the running program WROTE can be a runtime pointer.** A word identical to
the pristine image is a compile-time constant and cannot name a heap object.

`--remap all|changed|none`, defaulting to `changed`. Counted on the same state:

| mode | words rewritten |
| --- | ---: |
| `all` (the old behaviour) | 15,956 |
| `changed` (the default now) | **2,910** |

So **13,046 words — 81.8% — were static data**, rewritten into arena pointers on
every state run this project has ever done. Swept:

| | fault wall | vs no state | `agreed` |
| --- | ---: | ---: | ---: |
| no state | 1,584,976 B | — | — |
| state, `--remap all` | 1,569,464 B | +15,512 B | **−145 B worse** |
| state, `--remap changed` | 1,566,965 B | +18,011 B | **+509 B better** |

`agreed` improving is the part that matters: every previous state experiment in
this project made it *worse*, and that was the corruption, not the state. It is
still only 7.2% of the 250,000 B threshold, and `agreed_full_strength` is
unchanged at 42,209 B / 768 fn throughout.

So the remapper was a real defect with a real fix, and it does not change the
conclusion: four variants now agree that the fault wall is not built by the CRT.

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
death; the retired `lifted_oracle_summary` reads 42,209 B / 1.75% / 768 fn at full
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
the retired `lifted_oracle_why_not_full` replays each under `--verbose` and records
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

The two that flip to PASS, `?is_known@@YAHHHH@Z` and `?anything_at@@YAHHH@Z`,
are the clearest statement of why the control is mandatory: without it they
would have been reported as lowering bugs.

**All three survivors are one cause, and it is real.** The two that looked like
arena-pointer divergences were not pointers at all — the blamed instruction in
each is `fnstsw ax`, so the compared EAX is the x87 STATUS WORD with stale high
bits. Decoded:

| function | original | lifted | differs in |
| --- | --- | --- | --- |
| `?RGB_to_HSV@@YAX…` | `0x3820` | `0x3800` | PE (inexact) |
| `sub_63a9d0` | `0x4022` | `0x4000` | DE (denormal), PE |
| `sub_634720` | `0x0200` | `0x0000` | C1 |

**The lift never sets the x87 IEEE exception flags.** That is a genuine lowering
gap, not a harness artifact — which is exactly what surviving the shim means.

How much it matters is also measured. Across every catalogued function there are
**206 `fnstsw`/`fstsw` sites**, and of the 152 that mask the result:

| mask | sites | reads |
| --- | ---: | --- |
| `0x0100` | 80 | C0 |
| `0x4100` | 55 | C0, C3 |
| `0x4000` | 16 | C3 |
| `0x0020` | **1** | PE |

151 of 152 read only the comparison codes, which the lift models correctly. The
single site that reads an exception flag is `__check_range_exit` at 0x0064a8c9,
which is `external_library` — CRT code the lift never translates. So **no lifted
function in this image reads the bits the lift fails to set.**

These stay FAIL. The divergence is real, and masking a register the program
happens not to read would be converting an honest failure into a flattering pass
on the grounds that it probably does not matter — which is the move this
harness exists to prevent. Recorded as a known gap with a measured blast radius
of zero reachable sites, so a future session can price the fix rather than
rediscover the cause.

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

`generate_signature_oracles (retired) --list`

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
`ghidra_relation != "exact"` (`extract_legacy_leaves (retired):430`), excluding
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
