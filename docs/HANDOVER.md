# Handover

Everything an agent needs to pick this up cold, on a machine that has never
seen it. Read `docs/STATIC_RECOMPILATION.md` and `docs/LEGACY_ISLANDS.md` next,
in that order.

## What this is

A C++ reverse engineering of Sid Meier's Alpha Centauri — a 1999 32-bit x86
Windows PE. 5,673 catalogued functions, **2,410,317 bytes** of code.

The Rich header pins the original toolchain: **cl 12.00.8168 and 12.00.8447,
link 6.00, MASM 6.13 for 36 objects, timestamp 1999-12-20**. MSVC 6.0.

**The strategy is whole-image static recompilation.** Lift every instruction
mechanically into explicit-CPU-state C++, get it compiling, then fix it up
against the original until it matches, then replace lifted code with real
source. This replaced per-function hand recovery, which reached ~8% of bytes in
weeks and was not going to finish. Do not re-open that decision.

The original executable is never redistributed and its bytes never enter a
source file. `opensmacx_image` is zero-filled at build time and the real bytes
are loaded **at runtime** from the user's own copy. Mechanically-derived output
lives under gitignored `build/` and is never committed, never counted as
recovery progress, and never marked `source_complete`.

## Where it stands

Lift: **99.987%** of instructions lowered. It compiles, links, and loads the
image at runtime.

Verification is a differential oracle: the original function runs *for real* at
its canonical address under Wine, beside the lifted version on a separate copy
of the same flat 6.3 MB guest memory, 16 seeded states per function.

```
denominator: lift scope 2,410,317 bytes across 5,673 functions
  (excludes recovery_state == external_library: CRT and Windows library code
   the lift calls and never translates)

SKIP                          2,132,089 B   88.46%   3,732 fn
PASS                            178,248 B    7.40%   1,465 fn
INCONCLUSIVE-original-fault      81,855 B    3.40%     427 fn
SKIP-trap                         8,475 B    0.35%      14 fn
HANG                              4,649 B    0.19%      15 fn
INCONCLUSIVE-original-timeout     2,835 B    0.12%       7 fn
FAIL                              2,166 B    0.09%      13 fn
```

**Quote the tier, never the total.** The 7.40% counted a function that agreed on
one seed of sixteen identically to one that agreed on all sixteen:

| tier | bytes | share of agreed | fn |
| --- | --- | --- | --- |
| single-seed | 41,232 | 23.13% | 119 |
| arbitrated | 13,191 | 7.40% | 50 |
| seeds-incomplete | 81,589 | 45.77% | 529 |
| **full-strength** | **42,236** | **23.70%** | **767** |

**The honest figure is 1.75% of image bytes at full strength.** 76.30% of the
agreeing bytes rest on weakened evidence.

`tools/lifted_oracle_summary.py` also publishes a NUMBERS THAT MUST GO DOWN
block. A dashboard where everything rises cannot report a regression — this
project's defining measurement error was exactly that, so keep it.

## The ceiling — plan against it, do not plan to beat it

**980 functions / ~48.4% of image bytes transitively reach DirectDraw,
DirectPlay, DirectSound or MSVFW32.** The IAT surface there is *two imports*;
the real surface is every `call [vtable+n]` on a returned COM interface
pointer. No IAT work touches it.

With every tractable blocker removed and COM left skipped, the oracle's honest
ceiling is **~49% of bytes testable, ~31% agreeing**. The other half of the
image is only ever verified by **booting the lifted executable and comparing
observable behaviour (task #32)**. No seed count reaches it.

## Settled decisions — do not re-litigate

- **Per-import shims are not the answer.** Greedy set-cover, measured: making
  the 40 most valuable imports deterministic unblocks 592 of 2,983 functions —
  127,987 of 2,031,363 bytes, **6.3%**. Flat after step 3. 1,955 of the 2,983
  reach more than twelve distinct imports each. Frequency counts look
  encouraging and are misleading, because they recount the same deep call-graph
  set. The wall is all-or-nothing.
- **Unicorn / QEMU-TCG as the reference side: declined.** Genuinely faster
  (0.084–0.14 ms/case against 5.9 ms) but `lifted_x87.h` gates its exact path on
  `__LDBL_MANT_DIG__ == 64` and delegates to the host FPU, and **76.59% of image
  bytes carry x87**. QEMU's `helper_fsin`/`fcos`/`fptan` also drop to host
  `double`, and its undefined flags are a third arbitrary answer, not a correct
  one. Keep it on the shelf as a tiebreaker on individual FAILs.
- **Byte-matching against MSVC 6.0: a verifier, not a strategy.** Viable —
  no `/OPT:ICF`, no incremental-link thunks, no RTTI, static CRT, and exact
  toolchains are packaged (isledecomp/MSVC600-8168, itsmattkc/MSVC600). But it
  verifies source that *exists*, and **1,909 of the 2,983 import-blocked
  functions (72.10% of bytes) have no C++ at all**. The bounded prize is ~826
  `source_complete` functions the oracle structurally cannot reach: 7.40% →
  ~11.6%. Test it with five `source_complete` **leaf** functions with no struct
  member access (a wrong `/Zp` is indistinguishable from a wrong `/O`), `cl
  12.00.8168 /O2 /Gy /GR-`, diffed as normalised Capstone mnemonics. ~150 lines.
  reccmp is not needed and drags in cvdump.exe, a PDB and pyghidra.
- **Binary Ninja Free: no.** Plugins do not work in Free at all and headless is
  Commercial/Ultimate only. The unit of work here is 5,673 functions.
- **Declined elsewhere:** Qiling (no gdi32/winmm/ddraw/dsound modules),
  decomp-permuter and asm-differ (no x86 backend), Berkeley TestFloat against
  `lifted_x87.h` (it delegates to hardware, so TestFloat would measure the CPU),
  FLIRT/Lumina/BinDiff (library code is ~2% of bytes; 89% already carry mangled
  names), a pypcode/SLEIGH *interpreter* (SLEIGH's `fistp` emits
  `trunc(round(ST0))` and never reads the control word — wrong on 100% of the 69
  `fistp` sites). Read SLEIGH as a specification; do not execute it.

## Traps that have already cost days

1. **`size` is the SUM of `body_ranges` spans, not `end - address`.** MSVC
   outlined cold blocks to `0x0065xxxx`. 402 functions carry a second span, and
   a contiguous read runs into the next function by up to 2,102 bytes. Use
   `body_spans()`.
2. **The harness reads its own memory and it looks exactly like a lowering
   bug.** A guest pointer below the image lands in the harness's `.data` and
   does not fault, so the out-of-span check never fires. Five "lowering bugs"
   were this. **Before calling any FAIL a finding, rebuild with
   `-DORACLE_LAYOUT_SHIM=0x51000`: if the detail string moves, it is the
   harness.** Rebuilding at a different `--image-base` is *inert by
   construction* and proves nothing — relocating the PE moves the wrapped target
   and the image by the same amount.
3. **A test that cannot fail is worth nothing.** `--selfcheck` sat at 20/20
   while real top-page detections fell from 20 to 4. Every assertion needs a
   positive control: damage the code, run the test, confirm it FAILS, revert,
   record the output. A coupling test here was `assertIn("import ", <whole
   file>)` and was satisfied by prose.
4. **Never convert an honest SKIP into a flattering PASS.** If agreement came
   under weakened conditions it gets its own verdict name
   (`PASS-paths-taken`, `AGREED-STUBBED`) and stays out of the headline.
5. **Frequency is not set cover.** See the import measurement above.
6. **Undefined flags after IDIV/DIV/MUL/IMUL — and after any shift of count
   != 1 — are excluded per function via a `undef=<hex>` mask.** The original
   reason was that an arm64 host translating x86 gave three different answers
   for one instruction class. **That reason is now obsolete and the mask is
   still required**: measured on native silicon, dropping it costs 3 FAILs in
   bits the SDM declares undefined, because one consistent hardware answer is
   still not `lifted_x86.h`'s arbitrary one. `--no-undef` runs the experiment.
   See the migration results below; do not assume the shift rule is safe,
   `fixed_div` blames a `shl`.

## Open work, in priority order

- **#32 Boot the lifted image.** The only route to the ~48% of bytes behind COM.
  Now the highest-value item on the list.
- **#41 232 `??__E` thunks fault at `0x0060a8a1`.** Half the `extcall`-only
  cohort. The thunks set ECX to a global object address and call `0x005e37e0`;
  the object is uninitialised in the pristine image, so the *original* faults
  before the lifted side runs. Not a lift bug — the seed never makes that global
  usable. Cheapest remaining widening of the denominator.
  Reproduce: `tools/lifted_oracle_run.sh --only 0x00404440 --list build/oracle/plan.tsv`
- **#35 The 13 FAILs.** Apply trap 2 to each before believing it.
- **#37 The import wall.** Needs a uniform mechanism — one deterministic
  implementation of every import shared by both sides — plus its own verdict
  (`AGREED-STUBBED`). Both sides returning NULL from `GetDC` tests the NULL path
  only. `opensmacx_bind_real_imports` already exists; 129 imports have no shim
  and 40 are implemented but nondeterministic.
- **#36 Give the oracle an owned Wine prefix.** It faults hundreds of times per
  sweep by design and currently runs in the user's personal `~/.wine`.
  `tools/owned_wine_prefix.py` already does this for other tools.
- **#31 Phase 4 traps.** ~70 of the remaining 92 are transcribable from SLEIGH
  (`pushf`/`popf` ×36, `bt` ×13, `pushal`/`popal` ×10, `loop` ×9). Worth 0.01
  points and all sit in already-blocked functions — completeness, not
  throughput. `pushfd` is 36 of the 70 and is a **decision**, not a
  transcription: it is where the undefined-flag choices become observable in
  memory instead of in a maskable register. Derive it from executed behaviour.
- **#28** verification column in `functions.csv`. **#25** duplicate-binding
  rule. **#10 #13 #14 #17 #19** older incremental-recovery batches, low value
  now.

## Layout

```
tools/lift_whole_image.py      the generator: 64 shards + dispatch + runtime
tools/x86_lower.py             the instruction lowerer (capstone -> C++)
tools/lifted_x86.h             integer and flag semantics
tools/lifted_x87.h             x87, 80-bit, mirrors the guest control word
tools/lifted_{muldiv,string,tls,dblshift}.h
tools/lifted_loader.{h,cpp}    loads the user's exe at runtime
tools/lifted_oracle*.{cpp,h}   the differential oracle
tools/lifted_oracle_fold.h     per-function verdict fold (pure, unit-tested)
tools/lifted_oracle_plan.py    blocking flags + the undef= mask
tools/lifted_oracle_summary.py the published numbers
tools/host_doctor.py           run this FIRST on a new machine
docs/HOST_MIGRATION.md         moving the harness between machines
```

**1,008 Python tests across 35 files, 38 ctest targets.** Python tests are
`unittest`, standalone, no pytest. Run them with
`uv run --with-requirements tools/requirements.txt python3 <path>` — the system
`python3` deliberately lacks capstone and pefile.

## The move to native x86-64 has been made, and here is what it bought

Done. The four checks below were run; this section is the result, and the
checklist that follows is kept only because it is what a *third* host would
run. Reports live at `build/oracle/report-{ctl,noundef,masked}.tsv`;
`baseline-arm64/report.tsv` is the old host's, unchanged.

**Compare with `tools/lifted_oracle_compare.py BEFORE.tsv AFTER.tsv`.** Two
summaries side by side cannot answer "same result?" - the totals agree while
the set does not - so it diffs per-function verdict transitions and states
`better`/`worse` per figure rather than printing a signed delta.

**The baseline was swept with `--refuse-blocked` and nothing said so.** Its
3,732 `SKIP` rows are exactly the plan's 3,732 blocked functions. A default
sweep here attempts them and produces no plain `SKIP` at all, so comparing the
two directly attributes a harness setting to the host. Sweep with
`--refuse-blocked` for anything compared against `baseline-arm64/`.

### 1. The old PASSes reproduce, and then some

`agreed` 178,248 -> 180,308 B. **10 of the 15 arm64 HANGs now PASS**, and
**5 of the 13 arm64 FAILs now PASS** - `sub_6282e0`, `sub_634920`,
`sub_628290`, `?get_title@@YAPADH@Z`, `?alt_at@@YAHHH@Z`, every one an x87-code
or flags divergence that was a Rosetta artifact rather than a lowering bug.
No host refusals: the 5 `KILLED-host-refused` deaths are gone.

### 2. The masks are NOT droppable, and that is the finding

Dropping `undef=` cost **3 FAILs, 431 bytes**, all flag-only, all surviving the
`-DORACLE_LAYOUT_SHIM=0x51000` control with byte-identical detail strings:

| function | blamed instruction | flag | mask |
| --- | --- | --- | --- |
| `?speed_proto@@YAHH@Z` | `imul eax, esi` | ZF | `1e` |
| `?fixed_div@@YAHJJ@Z` | `shl eax, 0x10` | OF | `3f` |
| `sub_559210` | `imul eax, ecx` | ZF | `1e` |

These are the very functions this file and `lifted_oracle_plan.py` name as the
measurements that motivated the mask. Every divergence is in a bit the SDM
declares **architecturally undefined** - SF/ZF/AF/PF after IMUL, OF after a
shift of count != 1 - and each sits inside its own function's mask.

**So the mask's justification has changed, and it is now a tractable one.** It
was "the host gives three different answers, so no lowering can match all
three". It is now "the host gives ONE answer and it is not the lift's". Real
silicon is self-consistent, which is what the move was betting on; being
self-consistent is not the same as agreeing with `lifted_x86.h`'s arbitrary
deterministic choice. Note `fixed_div` blames a **SHL, not the IDIV** - the
plan comment's claim that shift OF "was measured to agree with lifted_x86.h's
uniform rule in 651 of 651 cases" was measured under Rosetta and does not hold
here. Deriving those three lowerings from real behaviour would let the mask be
dropped and add those flags to every comparison; that is now a bounded task
rather than an impossible one.

### 3. The top page cannot exist here, so the arbitration is retired

`0xFFFF0000` faults, and it is not merely unmapped: Wine gives a 32-bit process
`lpMaximumApplicationAddress = 0x7ffeffff`, and `VirtualAlloc` there fails with
ERROR_INVALID_PARAMETER. The three-fill arbitration is permanently inert - the
`arbitrated` tier went 13,191 B -> **0** - and every consumer already guards on
`g_top_page_writable`, so nothing else had to change.

It is not free. All 50 arbitrated functions became `seeds-incomplete`, and
**20 functions lost 26 comparable cases** that arm64 had compared against a
page the lift does not model. That is stricter and more honest, and it costs.

### 4. Full strength did NOT rise, and the reason is structural

**41,944 -> 42,024 B after fixing a self-inflicted regression, against the
arm64 42,236.** The gap reconciles exactly, to the byte:

* `?tech_recurse@@YAHHH@Z` -80 B, recursive, `FAIL-lifted-fault 0xc00000fd`
  STATUS_STACK_OVERFLOW - caused by shrinking the image's stack reserve to keep
  the child's stack off the guest image. Fixed by giving the lifted worker its
  own reservation; now 16/16, full strength.
* `?immune@SocialWin@@QAEHHHH@Z` -212 B, 16/16 on arm64 and 16/15 here. Its
  sixteenth seed reads the top page, which arm64 served as stable zeros and
  this host refuses.

42,236 - 212 = **42,024**. Like for like, the two hosts are identical, and the
arm64 figure was 212 B of comparison against unmodellable memory.

**Full strength is gated by the ORIGINAL side surviving all sixteen seeds,
which is a guest-state property and not a host property.** The host fixed
divergences and stability; neither converts a seed the original cannot run.
Do not expect a faster or more honest host to move this number.

The lever, measured: **99 functions / 11,843 B are exactly one seed short**,
led by `?wants_prop@@YAHHH@Z` at 3,970 B - which alone would clear 42,236. 69
more are two short. Same root cause as #32 below.

### What the sweep says the real wall is

Attempting the blocked functions instead of modelling them settles the import
question empirically, and it is not close:

| outcome | bytes | % scope |
| --- | --- | --- |
| `INCONCLUSIVE-original-fault` | 1,641,560 | 68.11% |
| `PASS-paths-taken` (weakened; never counted as agreement) | 500,295 | 20.76% |
| `PASS` | 180,528 | 7.49% |
| `SKIP-reached-blocked` - **the import wall's whole measured cost** | 48,932 | 2.03% |

33:1. Sharper still: the `iat`-only cohort is 238 functions and 44,742 bytes,
and the amount that actually walked into a blocked construct across 16 seeds is
**65 bytes**. Making every import deterministic converts nothing there, because
those functions never reach the imports. This is the same conclusion as the
set-cover measurement under "settled decisions", now with the paths *taken*
rather than the paths that exist.

The faults concentrate hard: 977 distinct sites, but **23 cover half the
faulting bytes**, and the largest cluster is `ListBox` - ~446 KB across 776
functions. `0x0060a8a1` alone accounts for 555 functions, **391 of them `??__E`
dynamic initialisers** (#41 records 232; attempting blocked functions surfaces
391). The globals they touch are uninitialised in a pristine image, so the
ORIGINAL faults before the lifted side runs. **That is #32's case, in bytes.**

## The checklist, for the next host

`python3 tools/host_doctor.py` prints these too.

1. **Confirm the old PASSes reproduce.** Sweep, then compare byte figures — and
   compare `agreed_full_strength` as well as `agreed`, because a move that
   trades large single-seed PASSes for small full-strength ones moves the two in
   opposite directions while the function count barely shifts.
2. **Drop the `undef=` masks.** On real silicon this should cost **zero** new
   FAILs. If it costs some, the masks were hiding a real lowering bug — worth
   more than the migration itself.
3. **Check a guest read at `0xFFFF0000` faults.** If it does, retire the
   three-fill top-page arbitration in `lifted_oracle.cpp`.
4. **Re-run the `iat` cohort.** On the arm64 host, **5 of the first 40 died with
   `rosetta error: unsupported privilege level: 0`** — process gone before any
   guard ran, recorded `KILLED-host-refused`
   (`0x00421b20 0x00421b40 0x00422ed0 0x00447360 0x0045c180`). Those should
   simply run now. If they do, the sweep's default of attempting blocked
   functions is clearly right; if the yield stays low, `--refuse-blocked`
   reverts it.
