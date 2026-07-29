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

**Queue as of 2026-07-29: 36 candidates, 1,381 bytes.**

The first five tightenings were about whether a body can be TESTED in
isolation. The sixth is about whether it can be WRITTEN at all - a
distinction the other conditions never ask, and one that matters because a
candidate nothing could ever recover makes the count a target that cannot
reach zero.

It has been tightened six times, each after it offered something unsafe:

| fix | what it was about to allow |
| --- | --- |
| read the target's object dir, not CMakeLists | `BattleWin::stop_timer`, which cannot link where it is tested |
| indexed absolute addresses count as globals | `Dialog::set_def_dialog_text_color`, which writes global arrays |
| declared arity must match the `ret` | `Win::on_redraw`, which would corrupt its callers |
| exclude branch targets from binding flags | four false "needs classification" reports |
| reject bodies that read EBP with no prologue | 13 EH unwind funclets, which are not callable at all |
| reject instructions with no C++ expression | `set_palette` at 0x005d4240, which uploads a VGA palette with `cli` / `out dx,al` / `sti` |

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
