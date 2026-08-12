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
  because the control word decides the answer, and docs/HANDOVER.md (retired) records
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
the Rich header pins (docs/HANDOVER.md (retired)), and `~/opt/vc6/LIB/LIBC.LIB` is
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

## 2a. C++ EH unwind funclets — expressibility, and measured three times

43 functions, 15,970 bytes, all `unrecovered`, all flagged `hidden`, all in
the linker's pool at `0x0065xxxx`–`0x0066xxxx`.

These are a DIFFERENT POPULATION from the frame registrations above and were
being walked into as if they were ordinary small functions. A registration is
a `mov fs:[0], esp` inside a function that has a frame. A funclet is a
separate body with **no frame of its own** that runs during unwinding and
destroys a local belonging to the function that registered — so it borrows
that function's `ebp`:

```
0x0065BFB1   lea ecx, [ebp - 0xA714]   ; the CALLER's frame, 42 KB into it
             jmp 0x00406C00            ; = ??1Popup@@QAE@XZ, an entry point
0x0065C001   mov ecx, [ebp - 0x14]
             jmp <BaseButton dtor>
```

No prologue, no epilogue, no `ret` — a tail `jmp` into another symbol's entry.

**Why a per-function compile cannot produce one, measured rather than argued.**
Three recovery agents reached this independently and tested the alternatives:
a genuine pointer local forces VC6 to emit its own `push ebp; mov ebp, esp`
and a `call` instead of a `jmp`, because the function then has a frame to
establish; only a compile-time-constant `this` reaches a tail `jmp`, and then
the load collapses to `xor ecx, ecx` and the ebp-relative access is gone
entirely. There is no source form with all three of "no prologue",
"ebp-relative access to somebody else's locals" and "no epilogue".

The `-0xA714` displacement settles it without any of that: no standalone
function this harness compiles has a 42 KB frame.

**Consequence.** The recovery unit for a funclet is the ENCLOSING function
that registered it, not the funclet's own address. These addresses should not
be queued by `byte_match_fanout.py --prepare`, and an agent that meets one
should report it and move on rather than spend a wave on it. That is a
catalogue-scope finding; the bodies are not defective and nothing about them
is unknown.

## 2b. Casts that no class declaration can remove

Not functions — CASTS. `tools/classify_casts.py` resolves every
`this`-relative cast in hand-written `src/` against the layout its class
declares and writes `docs/recovery/cast-classification.csv`, gated by the
`cast-classification-current` CTest. Three of its buckets are permanent, and
they are recorded here so the count is not mistaken for a backlog.

**`vtable-load` and `vtable-slot` — correct as written.** `src/wave.h:29-31`
states the model: the vtable pointer at offset 0 is held as opaque storage
"so no C++ vtable is generated; the original installs fixed vtable addresses
itself (the constructor writes one, the destructor republishes three as it
descends the inlined hierarchy)". Declaring these `virtual` would make VC6
own a vptr the original owns by hand, and no C++ destructor republishes three
different tables on the way down. These are not a missing-`virtual` defect;
they are a deliberate refusal to let the compiler own it. The only legitimate
improvement is naming the slot constants, which `src/basebutton.cpp` already
does with `BaseButtonRefreshSlot`.

**`receiver-is-subobject` and `negative-offset` — `container_of`, and C++ has
no `container_of`.** A negative displacement says the class is standing on a
sub-object and reaching its enclosing object. `emit_hypothesis_layouts`
reached the same conclusion for the same reason: "`-0xA14` says this class is
itself a sub-object of something larger. That is the more interesting fact and
still not one a declaration can express." The best form available is
`(char *)this - offsetof(BaseWin, iface_)` — typed and named, and still a
cast. Anything in a body carrying such an offset is disqualified wholesale,
because `this` there is not the declared class and no offset in it resolves.

**Every `ORIGINAL(...)` cast.** `src/original_seam.h:112` is
`#define ORIGINAL(p) (reinterpret_cast<OriginalObject *>(p))`, and the comment
above it records why: VC6 rejects `__thiscall` with C4234, and suppressing
that warning silently emits `__cdecl`, putting the receiver on the stack and
cleaning it twice. These vanish only when the CALLEE becomes source-owned,
which is a recovery task and not a header one.

Anything under `src/hypothesis_layouts.h` or the twelve generated `.cpp`
files is out of scope by construction: those are regenerated, and a hand edit
is reverted by the next build.

## 2c. One-symbol verification against implicit member code

Not a property of the original at all — a property of the harness meeting the
C++ ABI. `byte_match` refuses a candidate that defines more than the subject:
*"expected one external .text symbol, found 2"*. That rule is right, and it
makes two shapes unreachable no matter how the body is written. Both were
found by agents testing an EMPTY body first, so neither is a guess about a
body that was merely hard.

**A destructor for a class with non-trivial members.** `Datalink::~Datalink`
(0x00432290, 1,260 bytes) embeds 18 `FlatButton` members, each holding a
`Buffer` with a declared non-trivial `~Buffer()`. Any user-defined
out-of-line `~Datalink()` — even an empty one — makes VC6 synthesise an
implicit `~FlatButton()` for the ABI-mandated member-destruction epilogue,
`/Gy` gives it its own COMDAT, and the unit now exports two symbols. No
source spelling avoids it: the second symbol is required by the language, not
chosen by the body. It becomes reachable only if those members are ever
remodelled as pointers.

**A local whose type is an opaque shell.** `InfoWin::right_menu`
(0x004589C0) declares a local `PullDown`, which the emitter supplies as a
method-only shell with no members, so the local occupies 1 byte instead of
the original's 0xF40 and `sub esp, 0xF40` never appears. The obvious fix —
`struct PullDownFrame : PullDown { char pad[...]; };` — compiles, and then
VC6 emits `PullDownFrame`'s implicit destructor as a real second `.text`
symbol, which the verifier refuses. The workaround scores NO_COMPILE where
doing nothing scores MISMATCH, so it is strictly worse. This one is not
permanent: it retires when `PullDown` gets a pinned size, which is ordinary
layout work.

The distinction matters for planning. The first shape is a wall. The second
is a missing measurement wearing a wall's clothes.

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

### NO LONGER AN EXCLUSION — measured 2026-08-12

This section's own ground retires it. It says these functions are **recoverable
as source and unverifiable by any differential oracle in this repository** —
which is an exclusion from the ORACLE route, not from byte-matching, where
verification *is* the byte comparison and no oracle is involved. The project now
has exactly one verification route and it is the second one.

Measured rather than argued: 20 DirectX-reaching functions were taken off the
recoverable frontier at <=200 B and their emitted skeletons scored through
`byte_match.match_function`. **All 20 returned a real MISMATCH** — zero REFUSED,
zero SHARED_TAIL, zero NO_COMPILE. The scaffolding emits, the unit compiles, and
the comparison produces a per-instruction verdict; every divergence was at #0,
which is the shape of an empty body and is what any unrecovered function gives.
There is no structural obstacle here at all.

So the 941 functions and 1,187,932 bytes below are **in scope**. The figures stay
because they are a true measurement of what the import closure reaches, and
`measure_exclusions.py --check` still re-derives them — but they measure a
POPULATION, not an exclusion. Nothing in `src/` ever cited this section: only
`EXCLUDED S1` and `EXCLUDED S2a` appear in the tree, so no annotation changes.

The original text is kept below because the reasoning is sound about the route it
was written for, and because 48.4% of the catalogued bytes moving in or out of
scope should be traceable to the sentence that moved them.

> docs/HANDOVER.md (retired) puts the oracle's honest ceiling at ~49% of bytes testable
> with COM skipped; the other half is reachable only by booting the lifted
> executable and comparing observable behaviour. Acceptance for anything in this
> population is behavioural, and a recovery here that reports "no oracle
> available" is complete, not deficient.

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

### NO LONGER AN EXCLUSION — 2026-08-12, for the same reason as §4

The ground here is identical and retires identically: the smoke gate never
exercises any of it, so the evidence available was source-level tests and
nothing else. That was a statement about the runtime route. A `NetDaemon` body
that reproduces the shipped bytes is proved by the bytes, and needs no gate to
have exercised it.

The population figures stay, and stay measured. What goes is the claim that they
are out of scope.

> The smoke gate never exercises any of it. A recovery here can be covered by
> source-level tests and nothing else, and that is weaker evidence than
> everything else in the repository — it must be recorded as such in the commit
> rather than presented alongside gate-verified work.

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

Three figures moved on 2026-08-12 when `src/` became the catalogue's store, and
neither the image nor the definitions changed - only what the rows say.

`eh_funclets` is "unrecovered rows between 0x00650000 and 0x00670000", and it
reads 49 / 15,999 where the export said 43 / 15,970. `src/` carries 49
`EXCLUDED S2a` annotations in that range; six of them were classified some other
way in the export. The annotation is the record now, so 49 is the number.

`netdaemon.original_dependency_bytes` is 0 because `original_dependencies`
recorded which functions still reached the original executable through a
fixed-address binding, and there is no such binding: the DLL and its 2,049
redirects are retired. The column measured a route, and the route is gone.

```measured
external_library.functions = 327
external_library.bytes = 45155
external_library.underscore_names = 278
eh_funclets.functions = 49
eh_funclets.bytes = 15999
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
netdaemon.original_dependency_bytes = 0
```
