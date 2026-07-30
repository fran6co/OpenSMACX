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

**The block below is the arm64 BASELINE, not this host.** It reproduces
line-for-line from `baseline-arm64/report.tsv`, which is committed precisely so
the comparison survives; every current-host report lives under gitignored
`build/oracle/` and is therefore never in the repository. On x86-64 the same
control arm measures `PASS 180,308 B / 1,472 fn` (180,528 / 1,476 in the
default sweep) against the 178,248 / 1,465 printed here. Regenerate with
`tools/lifted_oracle_sweep.sh` and diff with `tools/lifted_oracle_compare.py`
rather than reading these figures as current.

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

**1,220 Python tests across 42 files, 42 ctest targets.** Python tests are
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

**The mask's justification has changed, and the new one is permanent.** It was
"the host gives three different answers, so no lowering can match all three".
Real silicon is self-consistent - that part of the bet paid - but
self-consistent is not the same as agreeing with `lifted_x86.h`'s arbitrary
choice. Note `fixed_div` blames a **SHL, not the IDIV**: the plan comment's
claim that shift OF "was measured to agree with lifted_x86.h's uniform rule in
651 of 651 cases" was measured under Rosetta and does not hold here.

Both bits were then measured directly on this silicon (i9-11980HK), which is
the step that turns "the answers differ" into a decision:

* **IMUL ZF is ALWAYS CLEAR.** 400,000 random cases, 166,234 of them with a
  zero result, ZF set in **zero** of them, regardless of entry flags. The lift
  writes ZF from the truncated result, so the two differ on exactly the zero
  products - which is precisely `speed_proto` and `sub_559210`. SF and PF agree
  everywhere; ZF is the whole disagreement.
* **SHL OF for count != 1 is deterministic but is none of the obvious rules.**
  200,000 vectors: 0 unrepeatable, 0 dependent on the entry value of OF, and 0
  consistent with "left unmodified". It is a genuine function of (value, count)
  belonging to this microarchitecture.

**So do not "fix" the lowerings to match.** It is tempting because the IMUL rule
is one line, and it would be overfitting: these bits are undefined precisely so
that implementations may differ, this image shipped for Pentium II/III, and a
lift tuned to a 2021 Intel part would be wrong on the hardware the program was
written for - and unverifiable on an AMD one. There is no host-independent
answer to match, which is why the mask is not a workaround for a bad host but
the correct treatment of an undefined bit. Keep it, and keep `--no-undef` as
the experiment that re-checks that reasoning on any future host.

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

## The recovery gate works here now, and did not before

`cmake --build --preset mingw-i686-release` stopped at **12 of 284 targets**
until `src/stdafx.h` was corrected to `<sdkddkver.h>`: the Windows SDK ships
that header capitalised, mingw-w64 does not, and only Linux cares. Every target
including `stdafx.h` - the DLL and all three test executables - failed, so the
only workflow that can move a function to `source_complete` could not be run at
all, and nothing said so. Third case-sensitivity defect the move has exposed.

Two of the gate's own checks then failed, correctly, and neither was relaxed:

* **1,081 `.eh_frame` sections** in `autosound.cpp.obj`. `-fno-exceptions` does
  not remove unwind tables and never did; g++ 13 defaults to emitting them.
  Fixed by not emitting them (`-fno-asynchronous-unwind-tables`,
  `-fno-unwind-tables`), because MSVC 6 emitted none and the check exists to
  say so.
* **Palette RGBQUAD order.** g++ 13 emits a third codegen shape. Read against
  the disassembly the alias-sensitive order is intact, so a third pattern was
  added - written pair by pair, and verified to REJECT three permutations
  (byte-swap dropped, reads exchanged, reserved written first). A looser regex
  would pass the exact defect the check exists to catch.

**Two operational facts, both required:**

```
cmake --preset mingw-i686-release -DOPENSMACX_PYTHON=$PWD/.opensmacx/venv/bin/python
ctest            # SERIALLY. Not -j.
```

Without the venv, 24 of 38 fail on a system `python3` that deliberately has no
capstone. Under `-j 4`, `recovery-gameplay-tests` fails and passes alone: the
Wine prefix is a single-instance shared resource, as this file already says,
and the tests do not declare that to ctest. `recovery-oracle` additionally
needs its opt-in `recovery-oracle-tests` target built first.

**Now 38 of 38.**

### The leaf-testable closure, and how its tail should be cleared

`tools/find_leaf_testable.py` reports the queue: unrecovered functions that
touch no absolute global, whose callees are all `source_complete` AND compiled
into `recovery-leaf-tests`, whose declared arity matches their stack cleanup,
and which are not EH unwind funclets. `--show-rejected` gives the reason for
everything it refuses. **The count is the progress metric.** It moves in both
directions: recovering a callee unblocks its callers, and tightening the filter
removes candidates that were never recoverable.

**Queue as of 2026-07-29: 0 candidates. The closure is exhausted.**

That is not the same as "there is nothing left to recover" - it is the
scanner's closure, and it grows again every time a recovery unblocks a caller
or a condition is loosened. Re-run `tools/find_leaf_testable.py` after any
batch; it went 69 -> 0 over one session but rose twice along the way when a
recovered callee made its callers eligible.

The first five tightenings were about whether a body can be TESTED in
isolation. The sixth is about whether it can be WRITTEN at all - a
distinction the other conditions never ask, and one that matters because a
candidate nothing could ever recover makes the count a target that cannot
reach zero.

It has been tightened seven times, each after it offered something unsafe:

| fix | what it was about to allow |
| --- | --- |
| read the target's object dir, not CMakeLists | `BattleWin::stop_timer`, which cannot link where it is tested |
| indexed absolute addresses count as globals | `Dialog::set_def_dialog_text_color`, which writes global arrays |
| declared arity must match the `ret` | `Win::on_redraw`, which would corrupt its callers |
| exclude branch targets from binding flags | four false "needs classification" reports |
| reject bodies that read EBP with no prologue | 13 EH unwind funclets, which are not callable at all |
| reject instructions with no C++ expression | `set_palette` at 0x005d4240, which uploads a VGA palette with `cli` / `out dx,al` / `sti` |
| an image address in a REGISTER is a global too | seven candidates that reach image memory as `mov edi,0x90db24 / rep stosd`, which the displacement check cannot see |

**The tail is not more of the same, and should not be ground out by hand.**
Of the 97 remaining, only about 5 have a class that already exists; ~21 need a
class declared first, and **~71 are unnamed `sub_xxx` bodies**, mostly in a few
identical shapes - a `mov [ecx+N]` field getter accounts for 13 on its own.

The convention for those already exists and it is GENERATION, not hand
recovery. `src/global_arith.cpp` holds 26 unnamed absolute-operand leaves
emitted by `tools/generate_global_arith.py`, named `global_arith_<address>_
redirect(void *, void *)` after the address rather than invented, grouped in
one family file, with the generator emitting the header, the source, a test
fragment and a wire list, and verifying itself by simulation. Follow that
shape for the remaining families rather than writing 71 names by hand; the
same pattern is already used by the nullsub, atexit, init, adjustor, deleting
and delegation thunk generators.

### What `generate_field_accessors.py` now understands, and what it will not

Nine shapes, each accepted only when the instruction fully determines the
meaning. The three added most recently are worth naming because of what they
taught:

| shape | why it was worth adding |
| --- | --- |
| `ret` / `ret N` alone | its ONLY content is the cleanup, which is exactly what I had already shipped wrong once |
| trailing `nop` stripping | IDA sizes a function to its whole 16-byte slot, so `nullsub_185` read as "16 bytes ending in a nop" and was refused as un-understood |
| `this->field = argN` | the first shape with parameters, so the first that has to say WHICH argument goes WHERE |

The refusals matter more than the acceptances. `sub_57dee0` reads `[ebp+8]`
and cleans nothing, so it is `__cdecl`; emitting `__fastcall` for it would
account for its argument twice. A body containing a stray `push` is refused
because that moves ESP and the `[esp+4]` slot arithmetic would then name the
wrong argument while still looking reasonable.

**A do-nothing body has no statements, so the mutation sweep says nothing
about it.** 254/254 killed is not evidence for those; the evidence is objdump
on the emitted `ret N` against the original, plus a manual poison. Say which.

### Nothing checked that a redirect's jump fits until 2026-07-29

`install_redirect` in `src/dllmain.cpp` does an unconditional
`memcpy(original, patch, PatchSize)` with a five-byte `E9 rel32` and never
looks at the length of the function it is replacing. **280 of the wired
redirects are shorter than five bytes**; the shortest are a single `ret`. They
are safe only because MSVC pads every function into a sixteen-byte slot, so
the overrun lands in padding - a property of this image, not of the mechanism.

`tools/verify_redirect_patch_fit.py` now checks it (ctest: `redirect-patch-fit`),
reading `PatchSize` from dllmain.cpp so widening the patch re-checks every
entry instead of silently invalidating the check. Current measurement: 1,994
redirects, 0 overruns, minimum room 11 bytes.

This is the same defect shape as the stack-cleanup bug it followed: a
mechanism with no guard, correct today by luck of the surrounding data. When
adding a redirect for a function under ~16 bytes, that check is the thing
standing between you and four silently rewritten bytes of an unrelated
function.

### When a fixture will not hold still, make the callee a seam

`sub_4080b0` and `sub_406af0` each destroy three subobjects. Running those
destructors for real needs three class teardown fixtures composed, and three
attempts at that failed in a way worth remembering:

1. a zeroed object faults on ListBox's own vbtable
2. publishing it reveals `Dialog::~Dialog` executing the image's
   `DialogOriginalClose`
3. binding that reveals the destructor reading pointer fields out of a seeded
   Dialog subobject
4. giving the Dialog its own minimum reveals `BufferFree` at 0x00644ef2
5. binding THAT made it non-deterministic - three consecutive runs of one
   binary gave fail, fail, PASS

**A single run would have reported success at step 5, and did at attempt one.**
Run any fixture of this kind at least three times before believing it.

The answer was not more setup. These functions ARE three calls with three
pointers, so the four callees became SEAMS defaulting to the real redirects -
the same mechanism `WinOriginalClose`, `DialogOriginalClose` and `BufferFree`
already are. Nothing about the shipped behaviour changes, and the fixture
checks the thing the function actually does. Baseline passes three runs out of
three; all five poisons fail three out of three.

Reach for this when a body's content is *which callee, which pointer, what
order* and the callees are expensive to stand up. It is not a way to avoid
testing a body that computes something - there, the state is the point.

### Checking generated code needs the REAL compile flags

`leaf_006281e0_redirect` is a vector length: `sqrt((y*y + z*z) + x*x)`, summed
and rooted entirely in x87 EXTENDED precision. Written the obvious way with
float operands it compiles, under this tree's actual flags, to `fstps` and a
tail call to `sqrtf` - the sum ROUNDED TO FLOAT32 before the root, which is a
different function in the last bit. Spelled with `long double` it becomes an
inline `fsqrt` on the extended sum, which is what the original does.

A standalone `-O2` check produced the inline form and would have passed the
wrong body. The flags that matter are in
`build/<preset>/compile_commands.json`; use those, not a plausible subset.

### The fault wall does not come down by seeding globals

`INCONCLUSIVE-original-fault` is the largest bucket in the measurement -
1,641,560 B, 68.11% of scope, 2,651 functions. Seeding the objects those
functions dereference was tried properly and moved it to **1,624,247 B, a
1.05% reduction**. Five experiments, each measured, all say the same thing:

| attempt | result |
| --- | --- |
| exclude zero from arena value words | no verdict changed in 40 functions |
| seed the Win global at 0x9bc074 | 7 of 8 sampled victims got PAST `Win::is_visible` - and faulted in `ListBox::attach` instead |
| fill all 3.25 MB of .bss with a valid pointer | no better than six targeted globals |
| widen from 6 to 45 object globals | **no gain, and one regression** |
| seed a vbtable below `this` for ListBox::on_key_down | 2 of 8 moved; 6 unchanged |
| make the whole arena an object graph, ATTEMPT 1 - changed only `draw_node_pointer`/`draw_leaf_pointer` | **a no-op, and read as a result**: see below |
| make the whole arena an object graph, ATTEMPT 2 - every `this` AND every arena pointer word names a slot+0x48 object with a vbtable below it | **the first attempt to move it materially: -39,666 B** - and it buys breadth, not proof |

**The wall moves; it does not fall.** Nine seeding experiments have moved it
from 1,641,560 B to **1,581,830 B - 59,730 B, 3.6%**. The goal that prompted
them asked for below 1,100,000 B, which needs 541,560 B. One experiment
produced 39,666 B of that and the other eight produced 20,064 B between them.

Where the remaining wall is, measured on `report-vbptr.tsv`:

| fault site | victim bytes | functions |
| --- | --- | --- |
| `0x0060afe5` in `ListBox::on_key_down` | 214,341 B | 141 |
| `0x006454fb` in `_strcat` | 94,102 B | 43 |
| `0x0060a8a1` in `ListBox::attach` | 72,126 B | 504 |
| `0x0060afdb` in `ListBox::on_key_down` | 61,130 B | 19 |
| `0x0060a02c` in `ListBox::set_selected_pos` | 55,083 B | 38 |

**ListBox owns roughly 519,050 B of the wall - a third of it - across seven
methods.** That is the arithmetic that makes the target look reachable and is
exactly why it is not: 1,584,581 - 519,050 = 1,065,531 B, just under the
1,100,000 B goal. Every remaining byte of headroom is in one widget's call
chains, and nine attempts at seeding have not made them run. They are not
reached with a seeded `this`; they are reached from inside chains that build
their own, which is what an initialised widget tree does and a seeded arena
does not.

The structural reason is measured: a
function is INCONCLUSIVE if ANY of its sixteen cases faults, and all 2,651
walled functions compare ZERO cases. Each leaves only when its WHOLE state
dependency is satisfied, and the last experiment shows why that is hard - the
vbtable can be seeded below the ENTRY `this`, but `ListBox::on_key_down` is
reached deep in a chain holding a different `this` taken from an arbitrary
arena word.

That is the difference between plausible state and COHERENT state: an object
graph whose back-references agree with each other. The hybrid smoke gate has
one because a real game built it; the isolated harness deliberately does not.

**That hypothesis has now been tested against a real game, and it is WRONG as
stated.** See "Real state, measured" below before acting on any of the
paragraph above.

### A no-op that looked like a result, and how it was caught

Attempt 1 changed `draw_node_pointer` and `draw_leaf_pointer` and nothing else,
then reported "nothing moved at all - the identical fault split, byte for
byte" and concluded the `this` was not an arena pointer. **That conclusion was
wrong, and the byte-for-byte identity was the tell.** Those two helpers feed
only the DRAWN cases; cases 0-3 are the LEGACY seeder, whose `this` is computed
separately. Nothing the patch touched could have changed them. An experiment
that cannot move its measurement will always agree with itself.

The rule this earns: **when a change produces output identical to the
baseline, suspect the change before believing the finding.** A real effect on a
16-case seed almost never lands on zero-bit difference. Confirm the seed
actually changed - `--dump-seed <case>` prints the registers and the window
around ECX - before writing down what the non-change means.

### What ListBox::on_key_down actually requires

Its first six instructions state the contract, and no amount of guessing at
globals substitutes for reading them:

```
0x0060AF96  mov esi, ecx                     ; esi = this
0x0060AF99  mov eax, dword ptr [esi - 0x48]  ; vbptr: the object BASE is this-0x48
0x0060AF9C  lea ebp, [esi - 0x48]
0x0060AF9F  mov edx, dword ptr [eax + 4]     ; vbtable[1]
0x0060AFA2  mov ecx, dword ptr [eax + 8]     ; vbtable[2]
0x0060AFA5  mov eax, dword ptr [edx + esi + 0x7c]
```

`this` sits 0x48 into its object, `[this-0x48]` must point at a vbtable, and
words 1 and 2 of that table are displacements added back to `this`. With the
`{0, 0x48, 0xa60}` layout `tests/recovery_leaf_tests.cpp` stages for this
class, those two loads land at slot+0x10c and slot+0xb2c - which is why the
object slot is 4 KB and not smaller.

Attempt 2 satisfies all of that for every object, entry or reached: both
seeders' `this`, the arena fill's pointer words, and `draw_node_pointer` /
`draw_leaf_pointer` all name slot+0x48 objects. The entry seed demonstrably
changed - `--dump-seed 0` shows `ecx 009e3048` where it showed `009d02b0`.

**The 20-function sample said it did nothing. The full sweep says otherwise,
and the sample was the thing that was wrong.** Twenty functions around one
call site cannot see a change spread across 5,673:

| figure | 3-seed | coherent arena | delta |
| --- | --- | --- | --- |
| INCONCLUSIVE-original-fault | 1,624,247 B / 2,560 fn | 1,584,581 B / 2,351 fn | **-39,666 B, -209 fn** |
| never_compared | 1,678,050 B | 1,640,428 B | -37,622 B (better) |
| agreed | 184,167 B / 1,517 fn | 191,292 B / 1,615 fn | +7,125 B (better) |
| agreed_full_strength | 42,170 B / 768 fn | 42,209 B / 768 fn | **+39 B, no new functions** |
| agreed_only_on_paths_taken | 529,771 B | 558,803 B | +29,032 B (WORSE) |
| agreed_under_weakened_conditions | 141,997 B | 149,083 B | +7,086 B (WORSE) |

**Read the last three rows together or the first row lies.** Of the 37,622 B
that left `never_compared`, 36,118 B arrive in the two weakened buckets, and
full-strength agreement gains 39 bytes across ZERO new functions. A coherent
arena lets far more bodies run to completion; it does not make the evidence
stronger for a single one of them. Two of the five numbers that must go down
went UP to buy it.

It also cost 11 functions that used to PASS and now fault (2,586 B, mostly
`CheckBox::*`), against 103 that moved fault -> PASS (9,543 B).

**18 new FAILs, and the shim control was run on all of them.** Two moved and
are therefore the harness reading its own memory, not the lowering:
`0x00579840 ?is_known@@YAHHHH@Z` (FAIL -> INCONCLUSIVE under the shim) and
`0x005a63d0 ?proto_sort_2@@YAXH@Z` ("134 words differ at 0x00945b44" ->
"124 words at 0x00945b4c"). **The other 16 are stable across the shim and are
unexamined** - they are the most concrete debt this change leaves. Many are
destructors reporting `ecx original=0xffffffff lifted=0x009eX048`
(`??1VoiceTx`, `??1VoiceRx`, `??_GVideo`, `??_GVoiceRx`, `??_GVoiceTx`), which
smells like one shared cause rather than five.

### Two more sweeps, and what a real gain looks like next to noise

| change | INCONCLUSIVE-original-fault | verdict |
| --- | --- | --- |
| coherent arena (above) | 1,624,247 -> 1,584,581 B | **-39,666 B, kept** |
| 16-word shared vbtable + value words never zero | 1,584,581 -> 1,583,210 B | -1,371 B, and `agreed` fell 226 B: noise |
| a real vbptr under the seeded Win and ListBox | -> 1,581,830 B | -2,751 B, kept as a correctness fix |

**Excluding zero from the arena's value words is settled now, against the
intuition.** 387,924 B of the wall - 24% - faults on exactly `0x00000000`, so
never writing zero looks obviously right. A whole-sweep run of it moved the
wall 1,371 B and cost 226 B of agreement. It had already been rejected once on
a forty-function sample; the full sweep agrees with that rejection. Zero is a
value real programs hold, and a seed that cannot produce it cannot catch a
lowering that mishandles it. It stays.

**The seeded objects were staged in a form the code cannot use.** Part 1 of the
goal put a ListBox and a Win in memory as zeroed blocks with the global
pointing at the block base. `ListBox::on_key_down` opens `mov eax,[esi-0x48]`
and reads `[eax+4]` immediately: `this` sits 0x48 into the object and the word
below it is a vbptr. Zeroed, that vbptr is NULL and the next instruction faults
on address 4 - 116,322 B of the wall, its third-largest fault address. That is
why Part 1 bought only 1.05%: the objects were real but unreachable. Fixed by
writing the shared vbtable pointer below each.

**13 new FAILs, shim control run on all: 10 are artifacts.** Several reported
lifted values that are literally instruction bytes (`0x53565754` is
`push edi/push esi/push ebx`), the signature of the harness reading image
memory through an out-of-span guest address. `0x0050e310 ?transport_base` was
counted as a lost agreement and is a clean PASS under the shim, so that
regression was not one either. **Three survive the control and are unexamined**,
on top of the 16 from the coherent arena.

### Snapshotting real state out of the hybrid: two barriers, one fatal

The recommendation that follows nine failed seeds is "take the state from the
hybrid runtime, which has a real object graph". It was tried far enough to cost
it properly, and it is harder than it sounds.

Wine maps the PE at its preferred base inside an ordinary Linux process, so
guest addresses ARE offsets in `/proc/<pid>/mem` and a snapshot needs no
injection and no write to the game. `tools/` has no such tool now; the throwaway
one worked on the maps parse and died on the read.

1. **`/proc/sys/kernel/yama/ptrace_scope` is 1 on this host**, so
   `/proc/<pid>/mem` is unreadable even for a process this session launched.
   Lowering it needs root, which is a system-wide security setting and not
   something to change for a measurement. The in-process route - a dumper in
   `src/dllmain.cpp`, which is already injected - avoids this entirely.

2. **How much of the object graph is inside the span is OPEN, and an earlier
   version of this section asserted the pessimistic answer without checking.**
   The oracle's guest memory is one flat region, 0x00400000..0x00A0C000, and
   the lift computes host addresses as `opensmacx_image + (guest - 0x00400000)`;
   the image's sections end at 0x009C21F8. If the widget objects are heap
   allocations they fall outside it and a snapshot captures pointers to nothing.
   But at least some are NOT: `0x005FE49A` is `mov ecx, 0x9b7490` - a static
   object at a fixed `.bss` address passed as `this`, immediately below
   `mov [0x9b8180], esi`. That one a snapshot would capture whole.

   **That count has now been done, and the answer is the good one.** Scanning
   `.text` for `mov ecx,<imm32>` where the immediate lands in `.data`/`.bss`
   and a call follows within twelve bytes finds **6,098 sites naming 572
   distinct static objects, and all 572 are inside the flat span.**
   `0x009b90d8` alone is passed as `this` 1,426 times. This program keeps its
   object graph in static storage, which is what a 1990s C++ game does.

   So the snapshot route is a CONTAINED change, not a redesign: an in-process
   dumper in `dllmain.cpp` plus a `--state` overlay on the loaded image, with
   the flat span and the address translation untouched. That is the single
   best-founded next step in this document.

   **It is blocked on a display, and that is the only thing blocking it.** The
   two obstacles that looked structural are not:

   * `--oracle-result` must be under `.opensmacx/` or `build/`; the smoke tool
     refuses any other path so proprietary artefacts cannot leave the ignored
     directories. Point it there and it launches.
   * `ptrace_scope` is 1, which permits an ANCESTOR to read a descendant. A
     reader that launches the smoke itself can read every wine process below
     it, so `/proc/<pid>/mem` needs no `dllmain` change and no privilege - the
     first attempt failed only because the reader was a SIBLING of the game.

   What actually stops it is that this host is headless:

   ```
   err:winediag:nodrv_CreateWindow Application tried to create a window, but
   no driver could be loaded.
   ```

   The game dies at window creation, so it never reaches an initialised state
   worth dumping. The runtime oracles all still pass, because they run inside
   the DLL before that point - `runtime_oracles` being green in
   `hybrid-smoke-result.json` does NOT mean the game ran.

   Neither fix is the harness's call to make. `Xvfb` is not installed, and
   installing it is a system change; `/tmp/.X11-unix/X1` exists, so display
   `:1` is live, but pointing the game at it puts a fullscreen window on
   somebody's desktop. Ask before doing either.

### Giving all 572 static objects a vbptr: rejected, and it says why

The obvious follow-up to that count is to stop inventing objects and seed the
572 real ones instead - each is zero at load, so `[this-0x48]` is NULL and a
virtual-inheritance method faults on its second instruction. Implemented by
scanning the loaded image at startup and caching, then writing the shared
vbtable pointer below each. Full sweep:

| figure | before | after | |
| --- | --- | --- | --- |
| INCONCLUSIVE-original-fault | 1,585,768 B / 2,355 fn | 1,582,708 B / 2,383 fn | -3,060 B but **+28 functions** |
| agreed | 191,292 B / 1,615 fn | 190,607 B / 1,595 fn | **-685 B** |
| agreed_full_strength | 42,209 B / **768 fn** | 41,893 B / **749 fn** | **-316 B, -19 functions** |

**Reverted.** The headline byte count improved and the strongest evidence class
lost nineteen functions. A number that must go down is not worth buying with
the one number that means the most.

And the reason it failed is the useful part: **a vbptr is not an object.** The
572 are still zeroed, so a method reads its vbtable, computes a displacement,
and dereferences a field that is zero - it faults one step later instead of
immediately. Synthetic seeding can supply the SHAPE of state and cannot supply
its CONTENTS, and every function still on the wall needs contents. That is the
argument for the snapshot stated as a measurement rather than a hunch: real
field values are exactly what nine seeds could not fabricate and a dump gets
for free.

### Proving the recovery: what the two mechanisms can actually reach

`unproven_recovered` is 96.9% of recovered bytes - 2,501 functions declared
complete and never executed against the original. Both proof mechanisms were
sized rather than assumed, and both are much smaller than they look.

**Legacy-leaf islands: 131 functions, 4,059 B - 2.1% of unproven bytes.** The
extractor's default census reviews only the 2,808 UNRECOVERED functions, where
islands stand in for code nobody has written; run against the unproven
RECOVERED set instead (patch `select_rows`, pass no addresses so the
"ineligible" raise stays quiet) it accepts 131. What rejects the rest:
`non_exact_analysis` 1,529, `contains_relocation` 529, `contains_call` 280,
`too_large` 17. Only the first is a gate on analysis quality rather than on the
code itself, so that is where any growth would come from.

**Generated hybrid-runtime oracles: 36 functions, 1,476 B.** The mechanism is in
`tools/generate_signature_oracles.py` and it works; the ceiling is what the
selection can reach. Three things had to be right, and each was wrong first:

1. **The redirect table, not `redirect_exports`.** The proof calls one address
   twice - `suspend_redirect_at` for the original, `resume_redirect_at` for the
   recovery - so an address the hybrid never patched has no second body.
   Selecting on the inventory column produced 39 oracles that ALL failed with
   "cannot suspend redirect". The authority is the `specs[]` table in
   `dllmain.cpp`: **2,048 addresses, of which 1,999 are unproven recovered
   functions.** The pool is large; the signature filter is what shrinks it.
2. **The DEFERRED phase, not phase one.** `run_runtime_oracles()` is called
   BEFORE `InstalledSpecs` is populated, so nothing can suspend there - and
   phase one gates DLL init, so a failing oracle stops the game booting.
3. **`@Z` terminates a non-empty parameter list.** Only `(void)` is the bare
   `XZ`. Missing that made every one-argument function invisible and produced a
   measurement of zero where the answer was 36.

**What actually runs, and it is five functions.** The suite is registered in
`DeferredSuites`, reports `generated-signatures passed`, and the game survives -
three runs out of three, plus the final verification. `unproven_recovered` moved
190,101 -> 190,008 B and 2,503 -> 2,498 functions. Small, and real.

**Why only five: arguments cannot be seeded in a LIVE process.** Not the type -
the DOMAIN. `?help_tech@@YAXH@Z` takes a tech id; driven with -1 and
0x7FFFFFFF it reached `?draw_labs@ReportWin@@QAEXXZ`, which divided by zero and
took the game down on the third function of thirty-six. The lifted oracle drives
wild integers safely because it runs against an isolated memory image where a
bad value can only fault the harness; this runs inside the real game, which has
no bounds checks and nowhere safe to fault. Nothing in a mangled name states a
function's domain. So the generator refuses every function that takes an
argument, and 31 of the 36 go with it.

**The diagnostic that found it**, because the failure mode hides itself:
`run_deferred_oracles` writes its report only after every suite returns, so a
function that kills the process takes the evidence with it - the log lists what
PASSED and says nothing about what died. Printing the name before each call, and
flushing, turned "the game crashes" into "the game crashes on `help_tech`" in
one run.

**A circularity worth not repeating.** The generator does NOT filter out
already-proven functions. It did, and publishing its own markers then made its
five functions proven, so the next run selected none and the committed file read
stale. `export_proven_functions.py` unions by address and records both
mechanisms on one row, so a function proven twice is not double-counted.

**The old note, kept because the constraint is still real.** The oracles run, but they cannot yet run to
completion. The comparison restores `.data`/`.bss` between the two calls, which
is right for a function whose effects live there and wrong for one that
allocates or writes a file - `?load_deswin_sprites@@YAXXZ` and
`?auto_save@@YAXXZ` do both. Three runs of three ended in an unhandled division
by zero at `0x004991DD`, inside `?draw_labs@ReportWin@@QAEXXZ`, UNRECOVERED
original code reading state the restore had made inconsistent. So the generator
emits no `PROVEN-AGAINST-ORIGINAL:` marker unless `--claim-proofs` is passed,
and the suite is built but NOT registered in `DeferredSuites`. It lowers
nothing and it breaks nothing.

**Where the next increment is.** Of the 1,999 redirected unproven functions,
**815 are `@@Q` __thiscall members** (25,393 B) and 991 are other mangled forms.
Members need a staged `this`; that is the single change that would take this
from 36 functions to hundreds. The safety problem has to be solved first: a way
to know which functions confine their effects to `.data`/`.bss`, or a harness
whose process does not have to survive the oracle.

### Real state, measured: it is WORSE than the synthetic seeds

Every seeding failure in this document ends by pointing at the hybrid, which has
a real object graph. That is now testable: `--state <dump>` overlays real
`.data`/`.bss` captured out of a running hybrid onto the pristine image, and
`tools/` grew nothing, because the capture needs no dumper - the reader just has
to be an ANCESTOR of the game, which `ptrace_scope=1` permits.

Both dumps were taken from a game running on a real display; a human clicked
through to a live map for the second. They differ from each other in 40,383
words, 10,620 of which are zero at the menu, so the in-game one is genuinely
in-game.

| | INCONCLUSIVE-original-fault | agreed | full-strength |
| --- | --- | --- | --- |
| synthetic seeds | **1,585,768 B / 2,355 fn** | **191,292 B** | **42,209 B / 768 fn** |
| real menu state | 1,600,347 B / 2,660 fn | 186,256 B | 33,210 B / 651 fn |
| real in-game state | 1,612,851 B / 2,684 fn | 189,503 B | 34,602 B / 653 fn |

Real state costs 27,083 B on the wall and **115 full-strength functions**, and
the more real it gets the worse it scores. That is not a subtle effect and it is
the opposite of what nine seeding experiments predicted.

**Why, and it was predicted before the second sweep ran and then confirmed by
it.** Count the address-shaped words in the dump - excluding float bit patterns,
which is why the first count of "88% outside" was too crude to quote:

| | land inside the flat span | low heap < 0x400000 | above the span |
| --- | --- | --- | --- |
| menu | 14,896 (26.4%) | 7,793 | 33,742 |
| in-game | 24,832 (30.0%) | 14,862 | 43,184 |

**Roughly 70% of what real state points at is not in the harness.** The oracle
holds one flat region, 0x00400000..0x00A0C000; the game's heap is below and
above it. A synthetic seed pointed every word into the arena and every
dereference landed somewhere; a real pointer to a real heap object lands on
nothing and faults on the first step. In-game state has MORE heap pointers -
82,878 address-shaped words against 56,431 - so it faults more. The prediction
was written down before that sweep and the sweep matched it.

**Then separate the two properties, and it works.** A dump supplies real
CONTENTS; the arena supplies REACHABILITY; nothing requires taking them from
the same place. `--state` now rewrites only the words that cannot work - those
naming Wine's low heap, or anything above the span - onto arena objects, and
leaves real integers, real floats and real in-span pointers alone. 58,046 words
in the in-game dump qualify, exactly the 14,862 + 43,184 the table above
predicts.

| | INCONCLUSIVE-original-fault | agreed | full-strength |
| --- | --- | --- | --- |
| synthetic seeds | 1,585,768 B / 2,355 fn | 191,292 B | 42,209 B / 768 fn |
| real in-game state | 1,612,851 B / 2,684 fn | 189,503 B | 34,602 B / 653 fn |
| **real contents + remapped pointers** | **1,457,178 B / 2,552 fn** | **206,334 B** | 40,191 B / 674 fn |

**-128,590 B, three times the coherent arena and the largest single move in
this document**, with `agreed` up 15,042 B. It costs 94 full-strength functions,
which is the honest price: they move to `seeds-incomplete`, still agreeing on
fewer cases, rather than losing evidence. Regressions are 1,656 B across 92
functions against that.

The `0x20000000` upper bound on the remap is load-bearing. `0x3F800000` is
`1.0f` and the common float patterns sit above it; rewriting those would corrupt
the very contents the dump exists to supply.

One property to know about: a remapped word that was a FUNCTION pointer sends
control into the arena, which shows up as `at 0x009df048 accessing 0x009df048` -
EIP equal to the fault address. Those were DLL addresses that would have faulted
unmapped anyway, so it is a wash, but a fault whose EIP is an arena address
means this and not a lowering bug.

So coherent state is still what the wall needs, and **a dump alone cannot
deliver it through a single flat span.** The requirement really is multi-region guest
memory: this document asserted that two commits before this one on no evidence,
retracted it on finding 572 static objects inside the span, and it is now
established by measurement. The 572 are real; they are just a minority of what
the graph actually references.

`--state` is kept. It is the instrument that settled this, the next person will
want it before trusting any argument about real state, and it costs nothing when
unused. **It has no unit test** - it was validated by use, cross-checking its
own live-word count against the Python dumper's - and that is a debt.

### Real object CONTENTS in the arena crash the harness

The obvious next move after the remap is to stop pointing remapped pointers at
generic slots and give each slot the contents of a REAL object from the dump -
copied from `object - 0x48` so a slot's `this` at `slot+0x48` lines up, keeping
the object's own vbptr, which points at a real vbtable in `.rdata` inside the
span. Tried twice. **It does not work, and the failure is in the harness, not
the lift:**

```
oracle: HARNESS FAULT 0xc0000005 at 10001b08
  (this is a bug in the oracle, not a finding about the lift)
```

60% of functions came back `KILLED-host-refused` - 1,356 of 2,242 before the
sweep was killed. A report that full of kills is a broken instrument, not a
measurement.

Two hypotheses, one ruled out and one standing:

* **Back-word ordering: ruled out.** The first attempt ran the slot fill AFTER
  the four words below `this` were seeded, overwriting them - and those must
  stay small, because 130-odd functions open `sub ecx,[ecx-4]` and an arbitrary
  value there yields a pointer near 4 GiB whose lifted-side translation reads
  the harness's own memory. That is exactly the documented failure mode, so it
  looked certain. Reordering the fill to run FIRST did not fix it. **A
  mechanism that explains a symptom perfectly is not thereby the cause.**
* **The arena's value discipline is load-bearing, and copies violate it.** With
  `--state` the crash is reliable; without it, the same build runs clean. The
  only difference is arbitrary real 32-bit words - packed fields, floats,
  handles - sitting in the arena. The arena is deliberately built from small
  four-aligned integers and in-arena pointers, and the header says why: it
  keeps every derived address inside the scratch window. Object interiors do
  not obey that contract, and the remap cannot impose it, because it rewrites
  out-of-span ADDRESSES and leaves everything else alone by design.

That contract was then extended and it STILL crashes. `admit_word` was added:
a copied word is kept only if it is a small integer or an address inside the
mapped span, and anything else - float, handle, packed field - becomes an arena
object, exactly as the remap treats an out-of-span pointer. All 14 previously
killed probes still return no row.

A fourth attempt applied the below-`this` invariant to EVERY slot rather than
only the entry object, which the fault address appeared to name outright. It
failed too, and it failed with a byte-identical fault address across a code
change - the same no-op signature recorded earlier in this file.

**THE ONE CLUE WORTH INHERITING.** Decode the fault rather than guessing at it.
The lift computes `host = opensmacx_image + (guest - 0x00400000)`, and the run
prints `opensmacx_image` on every start. With it at `0x10fe8520`, a HARNESS
FAULT at `0x10001630` inverts to **guest `0xFF419110`** - a pointer near 4 GiB,
which is the case the back-word seeding exists to prevent. Whatever produces it
is not the four words below any `this`, because seeding those for all 32 slots
changed nothing. Start there with a debugger attached, not with a fifth theory.

**SOLVED, and it was mine.** `scan_static_this` filtered candidates with
`imm + 0x1000U >= kDataHigh`. For an `imm` near 2^32 that addition WRAPS:
`mov ecx,0xfffffe84` - a negative constant, not an object - computed
`0x00000e84`, passed the bound, and the slot filler copied 4 KB from
`0xfffffe84 - 0x48 - 0x400000`. An out-of-bounds read in the harness's own C++.
It looked like a harness bug because it was one.

What found it was instrumenting the fault instead of theorising about it. The
handler now prints the guest context on a HARNESS FAULT, and the first dump
named the culprit: `esp=0x0039f648` was a HOST stack, `ecx`/`ebx` were a slot
and the next slot, the faulting access was exactly `esi` - the source pointer -
and `eax=0xfffffe84` was the bad object in plain sight. **Four hypotheses about
guest semantics, and the bug was arithmetic in a filter.** Keep that
instrumentation; it costs nothing and it is what ends this kind of hunt.

**Four confident hypotheses, four wrong.** Back-word ordering; then the
`0x3F800000`-style float above the remap ceiling; then the value contract in
general; then the invariant applied per slot. Each explained the symptom completely and none was the cause. The one
piece of real evidence about where it lives: the fault address MOVES with the
build - `0x10001b08`, then `0x10001630` - which puts it in the harness's own
code, not in anything the seeding writes. That is a debugger's job on the
oracle process, not another hypothesis.

What is solid: `--state` crashes and the same build without it runs clean, so
the trigger is real object contents in the arena. What that costs is bounded -
the remap already banks -128,590 B without copying any contents at all.

### Real object contents in slots: still broken, and it printed 15.85%

With the overflow fixed, all 14 previously killed probes return rows and a whole
sweep runs to completion. **The result is still not usable, and the way it fails
is the point:**

```
INCONCLUSIVE-original-fault    382072   15.85%   1050      <- the target, smashed
never compared by the oracle  2296255   95.27%   4981      <- and here is why
```

3,928 of 5,673 report rows have address `0000000000` and name `(null)`. Those
rows name no function, `read_report` drops them - correctly - and the headline
is computed over the 1,745 that remain. **A run that collapsed reads exactly
like the number collapsing.**

**AND THE SWEEP SAID IT WAS FINE.** Observed, without a mechanism attached,
because four wrong mechanisms in one day is enough:

* `sweep: finished, 0 hang(s) and 0 host death(s)`
* exactly ONE `HARNESS FAULT` in the whole log, and ZERO resumes
* the null rows are CONTIGUOUS, row 1747 to the end
* no other report in this project - `report-state-remap`, `-vbptr`,
  `-coherent` - contains a single null row

**The mechanism, found afterwards and fixed at the source.** Those rows are
plan entries whose `address` is ZERO, and `%#010x` emits the `0x` prefix only
for a NONZERO value - so a zero address renders `0000000000`, which
`read_report` cannot parse and therefore drops. **The unparseable format is the
only reason this was caught at all.** Had the driver printed a parseable
`0x00000000`, 3,928 rows priced at zero bytes would have been folded into the
figures as real results.

`lifted_oracle_main.cpp` now refuses to write a report row for an entry with no
address and names the entry on stderr. The guard is a net; below is how far the
diagnosis got.

**RULED OUT: the plan parser.** It reads with `char line[1024]` and `fgets`, so
a line over 1023 bytes would be split and the continuation parsed as a fresh
entry whose `strtoul` finds no hex - address zero. Mangled C++ names get long,
so this looked right. The longest line in `plan.tsv` is **123 bytes**. It also
predicts nulls scattered wherever the long names are, and the observed nulls are
CONTIGUOUS from row 1747 to the end.

**HYPOTHESIS BELOW IS NOW RULED OUT. Kept because being wrong five times in
one area is the finding.** `lifted_oracle_main.cpp` compares every plan entry
against a snapshot taken at startup, before each function, and aborts the run
if one moved. Positive control run: poisoning `plan[5].address` makes it fire,
restoring it makes it silent. Re-running the object-contents reproducer under
that check produced **no corruption report, no harness fault and no null rows**
- it simply stopped at 1,745 functions, which is a TIMEOUT: the reproducer
memcpys 4 MB per function and a fifty-minute cap is not enough for 2,000 of
them.

So the plan is not being rewritten, and the null rows come from somewhere in
the sweep's own kill-and-resume path, which is where the next person should
look. The integrity check stays: it is cheap, it has a positive control, and it
now rules this out for every future run rather than for one experiment.

~~LEADING HYPOTHESIS, NOT VERIFIED: the guest corrupted the plan in memory.~~
The evidence is circumstantial and stated as such:

* the nulls are contiguous from a point onward, which is what a single
  corrupting write looks like and not what a parse bug looks like;
* the one HARNESS FAULT in that run had `esp=0x0039f7a0`, a HOST stack;
* it happens only with `--state` object contents in the arena, never with
  synthetic seeds.

`lifted_oracle_run.sh` already documents this hazard - "a seeded pointer can
land on the HOST's stack and overwrite the registers the runner saved there" -
and a plan vector is no better protected than a saved register. **Do not act on
this without confirming it**; four mechanisms were asserted confidently in this
area and all four were wrong. Putting the plan behind a guard page would confirm
it in one run.

So the run really covered 1,745 of 5,673 functions and the rest are
placeholders, and `lifted_oracle_sweep.sh` reported success. Its supervision
watches for a stalled report and for a process that dies without writing a row;
this was neither. **"finished, 0 hang(s) and 0 host death(s)" is not evidence
that the plan was covered.** Check the row count and the new dropped-line
warning before believing any sweep, including a green one.

`never compared` at 95% gives it away instantly, which is why that figure is
published beside the headline and must never be dropped from a report. But
nothing SAID 3,928 lines had been discarded, so `lifted_oracle_summary.py` now
prints:

```
WARNING: 3928 report line(s) named no function and were dropped. A figure
computed from the rest describes only 1745 of 5673 rows - suspect the RUN,
not the lift.
```

Two tests cover it, and the positive control was run: disabling the counter
makes them fail.

### One of the seeded globals was never a pointer

`0x0087BE24` was seeded with the address of the staged ListBox on the strength
of a scan for `mov ecx,[bss global]` near a Win/ListBox call. The image says it
is an index:

```
0x004A423E  or eax, 0xffffffff
0x004A4243  mov dword ptr [0x87be24], eax      ; reset to -1

0x004A624A  mov eax, dword ptr [ebp + 8]
0x004A624D  cmp eax, 0x400
0x004A6252  jge 0x4a626f                        ; >= 1024 is refused
0x004A6256  mov dword ptr [0x87be24], eax       ; so it is an index < 1024
```

Seeding it with `0x009eb000` puts a nine-megabyte value in a slot every
consumer range-checks. That does not make a caller more realistic - it sends it
down a branch a real run cannot take, and an unchecked consumer indexes an
array with it and computes an address outside anything mapped. Zero at load is
what `.bss` gives it and is a valid index, so the correct seed is no seed.

**The rest of that list has not been re-checked.** It was built by pattern scan,
and the pattern cannot tell an object pointer from a handle or an index. The
same disassembly that settled this one settles each of the others in a minute.
`0x009BC074` is the next to look at: at `0x00604650` it takes the return of an
imported call and the code tests it for zero and returns error code 4, which
reads like a Win32 handle rather than a `this`.

### The part still not understood

Five cases still fault at `0x0060afe5` on `mov eax,[ebx+0xa4]` with `ebx`
zero, while the `[ebx+0x10]` load eleven instructions earlier succeeded on the
same path. **Three explanations have been tested and eliminated - do not spend
the afternoon on them again:**

* **"after 0 steps" does not mean it faulted immediately.** `oracle_steps` only
  increments on single-step exceptions, and TF is set only while blaming, so 0
  is what every ordinary run reports. `at <eip>` is `ExceptionAddress` and the
  accessed address is `ExceptionInformation[1]`; both are exactly what they say.
* **It is not a loop.** The only branch anywhere in that range is the forward
  `jge 0x60afdb` at `0x0060AFD4`. Nothing jumps backwards into the block, so
  `ebx` cannot have been reloaded by `mov ebx,[esp+0x28]` at `0x0060B001` and
  then re-entered.
* **It is not the switch.** `xor ebx,ebx / mov bl,[eax+0x60b4ec]` makes `ebx` a
  small index, which would give exactly a fault on `0xa4` - but the table at
  `0x0060B4D8` holds five targets, `0x0060b126`, `0x0060b038`, `0x0060b170`,
  `0x0060b08a`, `0x0060b34f`, and none is inside the faulting block. (Entries
  5-7 read as `0x03020100` and `0x04040404`: that is the byte-index table at
  `0x0060B4EC` overlapping, not more targets.)

So the contradiction is real and unresolved: on a straight-line path
`[ebx+0x10]` cannot succeed while `[ebx+0xa4]` faults with `ebx` zero, `esi` is
never reassigned, and nothing writes `ebx` between the two. Both cannot hold on a straight line, so control reaches that
instruction some other way inside the 1,349-byte body. The switch at
`0x0060B031` (`jmp dword ptr [ebx*4 + 0x60b4d8]`, byte index table at
`0x60b4ec`) is bounds-checked by the `ja 0x60b353` above it, so it is not
that. Nobody has read the rest of the function.

**What the seeds cost.** They are kept, but the trade is not one-sided:

| figure | delta | direction |
| --- | --- | --- |
| never_compared | -47,138 B | better (must go down) |
| INCONCLUSIVE-original-fault | -17,313 B | better |
| agreed | +3,639 B | better |
| agreed_only_on_paths_taken | **+29,476 B** | **worse (must go down)** |
| agreed_under_weakened_conditions | **+3,413 B** | **worse (must go down)** |
| LOST AGREEMENT | **646 B across 8 fn** | **regression** |

Seeding changes the input to EVERY function, so it moves agreement onto weaker
evidence and can break functions that already agreed. A 25-function spot check
of previously-passing functions found no regression; the full sweep found
eight. **Sample a regression check at 25 of 1,476 and it cannot see a 0.5%
rate.** Run the sweep.

### A FULL-FILE mutation sweep reports survivors that are not real

Running `tools/mutate_and_verify.py` over the whole of
`src/leaf_recoveries.cpp` gives DIFFERENT answers on identical input. Two
consecutive runs, same tree, nothing else changed:

| run | reported coverage holes |
| --- | --- |
| A | 005E3660 x2, 0063BEE0, 00408470, 0063E7F0 |
| B | 005E3630, 005E3660 x2 |

Every disputed entry was then checked two other ways, and all of them die:

* targeted `--address` runs: 0063E7F0 8/8 killed, 0063BEE0 2/2, 00408470 3/3,
  00532A50 5/5, 00642940 14/14, all with zero survivors
* hand poisons: the 0063E7F0 null-guard inversion and the 005E3630 chase
  offset both FAIL the fixture and revert to passing

Only the 005E3660 pair is a genuine equivalence, and that one is proved rather
than assumed - mutating the clamp VALUE on the same line IS killed.

I first suspected my own concurrent edits - the harness rebuilds from the
working tree, so anything touching it mid-run would corrupt the result. That
is NOT the explanation. Two more runs with nothing else touching the tree
still disagreed:

| clean run | survivors |
| --- | --- |
| 1 | 005E3660 x2, 005CBBC0:889 |
| 2 | 005E3660 x2, 005CBBC0:889, **0063E7F0:722** |

The three-survivor core is stable and understood. The extra one is not real:
applying `!=` -> `==` at 0063E7F0:722 by hand FAILS the fixture.

**MEASURED 2026-07-30, and the mechanism above is REFUTED.** The suspected cause
was the hung mutant: it leaves a Wine process holding `recovery-leaf-tests.exe`,
the next mutant's link cannot replace the binary, the build still exits zero, and
the harness tests the PREVIOUS mutant's executable. The check for it is now built
- `build()` compares the artifact's `(mtime_ns, size, inode)` across the build and
returns `STALE` when a successful build replaced nothing - and it **never fired**:
zero STALE across four full sweeps, 1,444 mutant builds. On reflection it could
not have: this is a Linux cross-build, where an open handle does not stop `ld`
replacing its output, and a link that genuinely failed would exit non-zero and
already be counted as `no compile`.

What the four sweeps did establish, comparing outcomes by MUTANT INDEX rather
than by `address:line operator` (which is not unique - the `constant` operator
emits several mutants per line, and keying on it makes unrelated mutants look
like flips):

| | measured |
|---|---|
| indices present in all four runs | 361 |
| indices whose outcome disagreed | **5 (1.39%)** |
| direction of every disagreement | **three runs killed, one let through** |
| which run flipped | different in each case: idx 35 -> run 3, 134 -> run 2, 163 -> run 4, 219 -> run 4, 224 -> run 2 |
| hang position | index 152 in ALL FOUR - deterministic, so not the cause |
| calibrated threshold | 1.4 s baseline, 30 s threshold, identical in all four |

So it is a low-rate per-run flake, not a set of weak fixtures: different mutants
flip in different runs. All five perturb a memory offset or a pointer guard -
`0x8`->`0` in a chase, `==`->`!=` on a null check, `0x4C`->`0` in a `load32` -
where whether the perturbation is observable depends on fixture memory the test
does not pin.

**The asymmetry is the fix.** The flake only ever manufactures survivors, so
re-observing a survivor removes it, and that is nearly free: the mutant is
already built, so a confirmation is one test run, and a sweep has a couple of
dozen survivors against 361 mutants. `--confirm-survivors N` (default 2) re-runs
a surviving mutant before reporting it as a coverage hole and prints how many
were killed on re-run, so the flake rate stays visible rather than absorbed.

**The fix was then validated against the ground truth the four sweeps
established.** 20 mutants survived in ALL FOUR unconfirmed runs; 25 survived in
at least one. Two further sweeps with `--confirm-survivors 2` both reported
exactly **20**, the set was **identical to the always-survived set**, and the
five it excluded were exactly the five flip indices (35, 134, 163, 219, 224).
The two confirmed sweeps also agreed with each other on all 361 outcomes, where
the unconfirmed ones disagreed on five. One of them caught 2 flakes on the fly
(indices 206 and 215, both in `005CBBC0`) and reported them as killed-on-re-run.

Cost of that: about 20 extra test runs, roughly 30 seconds, against the ~90
minutes three extra full sweeps would take to get the same answer.

A full-file `N/N killed` is now worth what it says, provided
`--confirm-survivors` was on. The `sweep N/N` figures in commits before this
change came from unconfirmed full-file runs and carry a ~1.4% false-survivor
rate; a targeted `--address` run and a hand poison remain the strongest forms.

### The mutation harness is blind to two shapes, and that is not a pass

`tools/mutate_and_verify.py` reports `0 mutants` - or refuses the function
entirely with "no recovered functions matched" - for a body it cannot perturb:

* a bare `return a1;` or `return 0;`, which carries no literal, no comparison
  and no droppable store (`AlphaMovie::UNK3`);
* an empty body, which has no statements at all
  (`Menu::on_adjust_pulldown_pos`).

This file already records the same hole for a bare null-pointer guard. Zero
mutants is **no signal**, not a clean sweep, and the working rules are explicit
that a compile failure or an absent mutant "proves nothing". For those
functions the evidence is a MANUAL poison - change the body to a plausible
wrong one, watch the suite fail, revert - and the failure text belongs in the
commit message so the absence of a sweep is not mistaken for the absence of a
problem.

Two of the ten functions recovered on this host were in that class.

### The next recovery, already scouted

`?stop_timer@BattleWin@@QAEXXZ` at **0x00421b40**, 8 bytes:

```
add ecx, 8
jmp ?stop@Time@@QAEXXZ        ; 0x00616730
```

The offset needs no deriving: `BattleWin::~BattleWin` is already recovered and
already does `(uint8_t *)this + 8` as a `Time`, so `stop_timer` adds no layout
assumption that is not already relied on. The body is one line.

**It was attempted and reverted, and the reason is the useful part.** It does
not link into `recovery-leaf-tests`:

```
undefined reference to `Time::stop()'
```

`Time` is split. `src/time_recovery.cpp` holds the recovered lifecycle - the
constructor, destructor, `set_modal`, `release_modal` - and that is the file
the leaf tests link. `Time::stop` is in `src/time.cpp` with `init`, `start`,
`pulse` and `close`, which reach **fixed-address bindings into the original
executable**: `HandleMain` at `0x009B7B28` and `MsgStatus` at `0x009B7B9C`
(`src/temp.cpp`). Pulling that TU into a standalone test executable puts
unmapped absolute addresses one wrong fixture away.

Which is what the working rule at the top of `AGENTS.md` already says: a
function with calls, absolute globals, relocations or process state "must be
exercised at their original address inside the verified hybrid process". So
`stop_timer` belongs in the in-process runtime oracle under the hybrid smoke
gate, not in `recovery-leaf-tests`, and verifying it costs a staged hybrid run
rather than a one-line fixture.

**So the scouting criterion needs a third condition, and this is it.** Checking
that a callee is `source_complete` is not enough; it must also be reachable
from the target that will TEST the caller. Both of the first two conditions
held here and the recovery was still not cheap:

1. the callee is `source_complete`  ✔
2. it adds no new original dependency  ✔
3. **the callee links into the test target that covers the caller**  ✘

Anything failing (3) is not a bad target - it is a hybrid-oracle target, which
is a different and much larger unit of work than its byte count suggests.

Rejected while scouting, with the reasons, so they are not re-picked:

* `?on_redraw@Win@@QAEHXZ` (0x005ed9c0) - `xor eax,eax; ret 8`. The mangled
  name declares no parameters and the body cleans eight bytes.
  `tools/find_constant_returns.py` rejects it for exactly this and is right to.
* `Gamma::on_scrolled`, `FileWin::on_double_clicked`,
  `DesignWin::select_special_1/2` - all thin wrappers whose callee is still
  unrecovered, so each would add an original dependency rather than remove one.
* `MCIVideo::is_playing` (0x00600320) - `mov eax,[ecx]; and eax,1`, a perfect
  leaf, but no `MCIVideo` source exists at all, so it needs a whole verified
  class layout for a six-byte function.

`tools/find_constant_returns.py` now reports **0 candidates**: that shape is
exhausted. 153 unrecovered methods of 4-40 bytes remain on classes that already
have a `src/` file, which is the queue to work from.

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
