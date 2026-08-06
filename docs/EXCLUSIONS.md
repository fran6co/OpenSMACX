# What will not be recovered

Everything else in `docs/` says how to recover a function. This file says which
functions the project has decided not to, and on what grounds — because until
now those decisions lived in a plan file outside the repository, and a new
agent rediscovered each wall by walking into it.

Read the grounds, not just the list. Three different things are called
"excluded" here and they are not interchangeable:

- **Availability.** The code can be *linked* instead of rewritten. Nothing is
  given up; the obligation moves from "write it" to "prove the substitute
  behaves the same".
- **Expressibility.** There is no C++ expression for what the body does. A
  platform layer replaces it, and the acceptance criterion has to be stated
  before the replacement is written, not after.
- **Verifiability.** The source can be recovered; no differential oracle in
  this repository can *check* it. These are not excluded from recovery. They
  are excluded from the proof mechanism, and their acceptance is behavioural.

An exclusion is a promise about a population, and a population is a number.
Every figure below is re-derived by `tools/measure_exclusions.py`, and
`--check` fails when this document and the image disagree:

```
.opensmacx/venv/bin/python tools/measure_exclusions.py --check
```

The `measured` block at the bottom of this file is what it checks. Do not edit
a number there by hand to make the check pass; re-run the tool.

## 1. The MSVC 6 CRT — availability, and now nearly moot

327 functions, 45,155 bytes, `recovery_state == external_library`. 278 of them
carry the leading-underscore C convention, which is the cheapest evidence that
this is a library and not the game.

The exclusion has always rested on the claim that this code can be linked
rather than rewritten. That is a claim about *behaviour*, and three groups
inside it are not generic enough for a modern libc to satisfy it silently:

- **`__CIsin`, `__CIcos`, `__CIsqrt`, `__CIatan`, `__CIpow`, `__CIacos`** —
  the x87 intrinsic helpers. This project already treats x87 rounding as
  load-bearing rather than incidental: `tools/lifted_x87.h` exists precisely
  because the control word decides the answer, and `docs/HANDOVER.md` records
  that a SLEIGH interpreter was rejected for emitting `trunc(round(ST0))` and
  never reading it — wrong at every `fistp` site in the image.
- **`__output` and `_$I10_OUTPUT`** — printf formatting and long-double
  conversion. Their output reaches saved games and displayed text, so a
  difference is user-visible and file-format-visible, not internal.
- **`___sbh_heap_init`, `___sbh_find_block`, `___sbh_free_block`,
  `___sbh_alloc_block`, `___sbh_alloc_new_region`, `___sbh_alloc_new_group`,
  `___sbh_resize_block`** — the MSVC 6 small-block heap. No other allocator
  reproduces its size classes or its reuse order, and code that survives on
  a particular allocator's layout survives nowhere else.

**This is now largely solvable and should be, before anyone writes a line of
CRT replacement.** MSVC 6.0 Professional is installed at `~/opt/vc6`;
`~/opt/vc6/BIN/CL.EXE` reports FileVersion `12.00.8168.0`, the exact compiler
the Rich header pins (`docs/HANDOVER.md`), and `~/opt/vc6/LIB/LIBC.LIB` is
present. Linking the ISO's own `LIBC.LIB` makes these functions **identical**
rather than assumed-equivalent, which retires the entire behavioural argument
above instead of arguing it.

## 2. MSVC structured exception frames — the wall that was not there

**Corrected on 2026-07-31.** The earlier reading of this image was that 1,072
SEH prologues existed, 387 of them C++ EH thunks and the remaining ~685
`_except_handler3` frames that the EH-omission argument does not cover. That
was a byte-pattern artifact and the population does not exist.

`push -1; push imm32` occurs 1,072 times, and 687 of those are ordinary
argument pairs — `push -1; push 0xa0; call sub_5DACB0`. Subtracting 387 from
1,072 subtracted one census from a different one. Anchoring on the frame
*store* instead, `mov fs:[0], esp`, and reading the handler back out of the
nearest preceding push:

| registrations | handler | where |
| ---: | --- | --- |
| 387 | a per-function `__ehhandler` thunk to `__CxxFrameHandler` (`0x00644FD6`) | game and CRT |
| 14 | `__except_handler3` (`0x00646DF8`) directly | CRT only |
| 1 | `__local_unwind2`'s own handler (`0x0064524C`) | CRT only |
| **402** | | |

**No game function in this image registers an `_except_handler3` frame.** All
14 are CRT: `start`, `__threadstart@4`, `___FrameUnwindToState`,
`CallCatchBlock`, `BuildCatchObject`, `terminate`, `__ArrayUnwind`, the three
`??_L`/`??_M`/`??_N` array helpers, `___crtLCMapStringA`,
`___crtGetStringTypeA`, and two unnamed CRT bodies. The largest is 548 bytes;
none is anywhere near a kilobyte.

Confirmed independently: the four-byte value `0x00646DF8` appears exactly 14
times in the whole image, at the 14 sites above and nowhere else.

**Consequence: there is no `_except_handler3` residue to schedule last and
none to declare.** The EH-omission argument in `AGENTS.md` — that the frames
cannot execute because the image contains no C++ throw entry point — covers
every frame a recovery will ever meet. The instruction to re-examine any
function whose prologue targets a handler other than `0x00644FD6` still
stands, and is now known to fire only inside the CRT, which is excluded
anyway.

## 3. Port I/O — expressibility, and it is one function

Exactly one body in this image does port I/O on a directly reachable path:
`sub_5d4240`, 60 bytes at `0x005D4240`, `unrecovered`, no catalogued callers.

```
push ebp / mov ebp, esp / push esi / push edi / pushal / cli
mov edx, 0x3C8 / out dx, al          ; VGA DAC write index
mov edx, 0x3C9
  mov al, [ebx] / shr al, 2 / out dx, al   ; R, G, B, 6 bits each
  ...  256 times
sti / popal / ...
```

It programs the VGA palette DAC directly, with interrupts disabled. There is
no C++ expression for `out dx, al`; a hosted process cannot execute it at all.
The replacement is a platform-layer palette call, and **the acceptance
criterion is stated here in advance**: the replacement is accepted when the
256 palette entries the display reports afterwards equal the 256 entries this
loop would have written, each channel shifted right by two. Not "the screen
looks right".

Thirteen other functions — `battle_report`, `top_menu`, `probe`,
`editor_natural`, `process_message` and others — appear to contain `out`/`in`
on a first pass and do not. Their hits are inside MSVC switch tables in
`.text`: a table of addresses into a function at `0x0040E6xx` carries the byte
`0xE6` in every entry, and a sweep that desynchronises decodes it as
`out 0x40, al`. `tools/measure_exclusions.py` separates the two by asking
whether the dwords around a hit read as a table of addresses into the same
function, and reports the impostor count so that it cannot quietly become zero.

## 4. DirectDraw, DirectPlay, DirectSound, MSVFW32 — verifiability

941 functions, 1,187,932 bytes, **48.4% of catalogued bytes**, transitively
reach one of those four libraries along direct call edges from the six DirectX
and three MSVFW32 import slots.

The IAT surface is nine imports. The real surface is every
`call [vtable+n]` on a returned COM interface pointer, so no import-shim work
touches it and the closure above is a floor rather than a measurement of the
true reach.

These functions are **recoverable as source and unverifiable by any
differential oracle in this repository**. `docs/HANDOVER.md` puts the oracle's
honest ceiling at ~49% of bytes testable with COM skipped; the other half is
reachable only by booting the lifted executable and comparing observable
behaviour. Acceptance for anything in this population is behavioural, and a
recovery here that reports "no oracle available" is complete, not deficient.

## 5. Indirect call sites — every seam count here is a floor

5,159 call sites in `docs/recovery/callgraph.json` have no known target. They
are invisible to the call graph, so **every caller count, every seam count and
every reachability closure in this repository is a lower bound**, including
the DirectX closure above. When sizing work that turns calls into seams,
budget **+20% on methods** over what the graph says; virtual dispatch is where
the missing edges concentrate.

## 6. Multiplayer / `NetDaemon` — weaker evidence, recorded as such

36 functions, 43,599 bytes carry the `NetDaemon` name; 24,378 of those bytes
are still `original_dependency`, and 23,394 of them are two functions:
`NetDaemon::process_message` (18,489 B) and `NetDaemon::synch` (4,905 B).

The smoke gate never exercises any of it. A recovery here can be covered by
source-level tests and nothing else, and that is **weaker evidence than
everything else in the repository** — it must be recorded as such in the commit
rather than presented alongside gate-verified work. `AGENTS.md` already says
smoke does not exercise multiplayer; this is the population that sentence is
about.

## 7. The Thinker mod — measured, not licensed away

The licence objection was withdrawn. The measured position is what decides it,
and the measurement is specific.

Of Thinker's **2,562** address hypotheses (`tools/correlate_thinker_layouts.py`
reduces the fetched headers to an ignored CSV), **2,279 land exactly on
catalogued function entries** — a hit rate no accident produces. Where a name
carries a class prefix, its **class attribution is 100% correct: 1,216 of
1,216** agree with the catalogue's mangled class, including constructors,
destructors, scalar deleting destructors and dynamic-initialiser thunks.

So it understands the binary. On **function** names it still knows almost
nothing the catalogue does not:

- It names **exactly one** function the catalogue leaves as `sub_*`:
  `sub_51dd00`, which Thinker calls `NetDaemon_cleanup`. One.
- Its method names are **its own inventions**, not the real symbols. Only 6 of
  2,279 names match the catalogue's, and this image ships mangled names for
  89% of its functions, so the real name is already known wherever it matters.

### Corrected on 2026-08-06 — that argument was applied to the wrong things

The two conclusions below were both wrong, and both in the direction of
refusing evidence this project has no other source for.

**On names.** The redundancy argument above is measured over *functions* and
was extended to *members*, where it does not hold. `docs/recovery/functions.csv`
carries **4,821 mangled function symbols and zero data symbols** — no
`?x@Class@@2…` static members, no `?x@@3…` globals. MSVC mangling encodes class
scope and function signatures; instance data members never enter the symbol
table at all. There is no real member name for Thinker's to be redundant with,
so for data members Thinker is not a second-best source, it is the **only**
source of semantic names this project has — and one whose field meanings have
been validated for years by the mod working. The same holds for the IDB.

**On layouts.** *"A wrong layout compiles perfectly and corrupts memory at
runtime"* describes a shipping game. Nothing here runs. Every recovered body is
compiled and byte-compared against the original object, so a wrong layout
cannot produce a byte match: it is self-detecting at the objdiff gate and its
cost is wasted attempts, not corruption.

The 5-right/7-wrong figure also tested the one thing Thinker structurally
cannot have. It is a **mod**: it declares only the prefix of each struct it
touches, so `Font` 0x8 against 0x28 is a partial declaration, not a
disagreement. Scoring a partial declaration on total size measures how much of
the struct the mod needed, not whether it is right about what it does declare.

### The bar, by where the data lands

| use | bar |
| --- | --- |
| `static_assert(sizeof(X) == N)` in `src/` | unchanged: **zero wrong**, proved |
| a member or class **name** | adopt; the binary supplies no competing name |
| a class `src/` does not declare at all | adopt as a partial declaration, no size assertion |
| a member name or type on an already-pinned offset | adopt — it cannot move a byte, and the existing `static_assert` fails the build if it somehow does |
| a **size** for a class with none | stage to the agent as labelled hypothesis; objdiff is the acceptance test |

`derive_class_layout.py --score-csv` is unchanged and still refuses any source
that is wrong about a pinned size. It guards the first row, which is the only
one where a wrong answer is unrecoverable — a `static_assert` is believed by
everything downstream and checked by nothing.

Source *text* still never enters the repository: extracted offsets and names
only, per `hypothesis_only_local_input` in
`docs/recovery/external-analysis-sources.json`.

### 7a. PRACX is the same source as Thinker, and must not be counted twice

Checked on 2026-08-06 because it is the other public mod that describes these
window classes, and its binaries (`prac.dll`, `prax.dll`, present under
`.opensmacx/game/`) carry 39 RTTI type descriptors — all `std::`, no game
types — so only its source could be worth anything.
`DrazharLn/pracx@06726f89`, `shared/terran.h`, declares 42 structs.

**It is not an independent source.** Of its 38 structs carrying members, 26
share member names with a Thinker struct, and many share all of them:

| PRACX | Thinker | shared member names |
| --- | --- | ---: |
| `CSprite` | `Sprite` | 12 of 12 |
| `CImage` | `Texture` | 28 of 28 |
| `_PcxHeader` | `PCXHeader` | 18 of 18 |
| `CClass3B` | `CClass3B` | 25 of 25 |
| `CMenu` | `CMenu` | 13 of 13 |
| `CWinBase` | `Win` | 66 of 76 |
| `CMap` | `Console` | 66 of 79 |
| `CInfoWin` | `StatusWin` | 42 of 43 |

Identical member names in identical order is descent, not convergence:
Thinker's headers and PRACX's share an ancestor. `CCanvas` even carries
Thinker's `pfcnScrollText`, `dwordC`, `dword10` and `iFlags` — including the
function pointer whose unreadability once shifted a whole struct's offsets.

**This matters more than the yield.** `tools/derive_agreed_sizes.py` admits a
size when two INDEPENDENT sources land on the same number, and its control
holds at 24 right / 0 wrong precisely because the IDA database and the observed
access bound fail in unrelated ways. Adding PRACX and treating its agreement
with Thinker as corroboration would be counting one source twice, and the
control would not notice — both would be wrong together or right together.
(The pairing in use today is the IDB against the access bound, so nothing is
damaged; this is written down so the next attempt does not make the mistake.)

What is genuinely PRACX-only is 12 small structs — `CHitBox`, `CMemAllocator`,
`CLabelAllocator`, `MapVtbl` and friends — 39 members between them, 31 named,
and named in PRACX's own vocabulary rather than the game's. Not worth a
pipeline. Its useful residue is a NAME MAP for reading either mod's code:
`CWinBase` is `Win`, `CCanvas` is `Buffer`, `CWinBuffed` is `GraphicWin`,
`CMap` is `Console`, `CInfoWin` is `StatusWin`, `CMain` is `Console`.

Not catalogued in `external-analysis-sources.json`, because nothing here needs
fetching on a schedule.

---

## The measured block

Re-derived by `tools/measure_exclusions.py`; `--check` compares this block
against the pinned executable, `docs/recovery/functions.csv` and
`docs/recovery/callgraph.json`. The Thinker figures are not here: they depend
on an ignored, never-committed hypothesis CSV, so they cannot be gate-checked
and are cited above with their derivation instead.

```measured
external_library.functions = 327
external_library.bytes = 45155
external_library.underscore_names = 278
seh.frames = 402
seh.cxx_frame_handler = 387
seh.except_handler3 = 14
seh.except_handler3_functions = 14
seh.except_handler3_game_functions = 0
port_io.functions = 1
port_io.bytes = 60
port_io.jump_table_impostors = 1
directx.functions = 941
directx.bytes = 1187932
directx.percent_of_catalogued_bytes = 48.4
indirect_call_sites = 5159
netdaemon.functions = 36
netdaemon.bytes = 43599
netdaemon.original_dependency_bytes = 24378
```
