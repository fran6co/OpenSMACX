# What previous agents learned

Read this before writing a body. Every line is something an agent lost
attempts to, or a measurement over the whole corpus. Append to it rather than
rediscovering.

Mizuchi runs one agent per function with no channel between them, so two agents
hitting the same idiom each pay for it separately. This file is that channel.

## The check

```
tools/verify_recovered_function.py <address> --body -   < your-candidate.cpp
```

Exit 0 means BYTE_EXACT and nothing else does. It prints the first differing
mnemonic and its index. Your own opinion of a candidate is not evidence; this
is.

## Source FORM decides the branch, not semantics

Two spellings that behave identically compile differently, and this is the
single most common cause of a near-miss. Measured across the hand-written
near-misses: branch polarity is the largest fixable group.

- `if (c) { A } return B;` tests `c` and jumps over `A` — the jump carries the
  NEGATED condition. Source `<` gives `jge`.
- `return c ? A : B;` tests `c` and jumps TO the `B` arm. Source `<` gives
  `jl`.

So a diff reading `original 'jl' vs rebuilt 'jge'` usually means the original
was a ternary and the recovery used `if`. `je` vs `jne` is the same fault.

### Condition polarity and ternary-vs-if are DIFFERENT levers

Measured on 0x005E3650, where the diff was `jl` against `jge`. The ternary form
`index < 0 ? low : high + index` compiled to `jge` as well - the general
ternary rule did not help. What fixed it was flipping the condition and
swapping the arms: `if (index >= 0) { high + index } return low;` negates to
`jl`. Try both; they are not the same lever.

## Instruction scheduling: bind an operand to a local to reorder a pair

Measured on 0x006281B0, `result[i] = source[i] * scale` three times over.
Every straight-line spelling - including `scale * source[i]`, parenthesised
and temp-copied variants - compiled to

    fld [esp+8]      ; the shared operand first
    fmul [eax+N]

while the original loads the component first. All mnemonics agree, so the
verdict is MNEMONIC_ONLY and only the operands differ.

The multiply is memory-by-memory, and VC6 schedules those by loading the
operand shared across the statements first. Binding the component to a local
makes it local-by-memory, which flips the schedule:

    const float x = source[0];
    result[0] = x * scale;      // fld [eax+N]; fmul [esp+8]

A temp introduced for this reason is LOAD-BEARING. Say so in a comment, or the
next reader inlines it and silently loses the match.

## Signed vs unsigned is visible in the branch

`jle`/`jge`/`jl`/`jg` are signed; `jbe`/`jae`/`jb`/`ja` are unsigned. A diff of
`jle` against `jbe` is a type error, not a shape error - the recovery used
`uint32_t` where the original used `int`. `AGENTS.md:87` says parameters are
`int`, never `uint32_t`, for exactly this reason.

## Strength reduction

`sar`, `cdq`, `shl` in the original against `cmp`/`and`/`xor` in the rebuild
means the original divided or multiplied by a power of two on a SIGNED type and
the recovery wrote something else. `cdq` immediately before `idiv` is signed
division; an unsigned recovery emits neither.

## `push` vs `mov` at index 0-2 is register allocation - deprioritise

The original dedicates a callee-saved register the rebuild does not.
`docs/BYTE_MATCH_ROUTE.md:305-330` measured this as the hard class and records
two levers already tried and refuted. It is 6 of the near-misses. Do not spend
a long time here; report what you tried.

## Size predicts everything

Hand-written bodies that reached BYTE_EXACT: 301 of 830 under 64 bytes, and
**1 of 505 above it**. A body over ~64 bytes is a research project. If you are
given one, say so rather than burning attempts.

## Never buy a match with `__asm`

`__asm` pastes the original's instructions instead of deriving them, so it
proves nothing about the source and `AGENTS.md:5` forbids copied machine code
in distributable builds. `mizuchi_writeback.py` has NO content policy, so an
`__asm` answer that happens to be byte-exact WILL land in `src/` unchallenged.
It has happened once: an agent replaced `StringStruct::remove_all` with a naked
asm free function, the ledger recorded NO_COMPILE -> BYTE_EXACT as progress,
and the method was left declared, called twice, and undefined.

There are exactly four sanctioned `__asm` sites in product source
(`scroll.cpp`, `scenario.cpp`, `stringstruct.cpp`), each with a justification
and a C++ fallback.

## Some near-misses are catalogue defects, not code defects

Before assuming the body is wrong, check whether the SIGNATURE is.

- `BaseButton::on_key_click/on_key_down/on_key_up` are declared `void` and the
  original emits `xor eax, eax; ret N` - it returns a value. The catalogued
  mangled name says `X` (void), and the scaffolding believes the catalogue, so
  changing the body's return type gives
  `C2556: overloaded function differs only by return type`. The sibling class
  `BasePop` declares the same handlers `int`. Fixing these needs the catalogue
  corrected first; it is not a body change.
- Three bodies in `src/recovered/` are inline asm because the emitter gives
  them `__cdecl`/`__stdcall` signatures with no receiver while the body indexes
  `[ecx + N]`. They are thiscall functions with no mangled name. Real C++
  cannot name the entry value of ECX, so this is an emitter gap.

If the scaffolding's definition head disagrees with what the disassembly needs,
say so and stop. That is a finding, not a failure.

## Whether the signature is free depends on whether the row has a mangled name

Both halves of this were measured, and they point opposite ways:

- A row WITH a catalogued mangled name (`?on_key_click@BaseButton@@QAEXH@Z`)
  gets a class and a member declaration in its scaffolding, so the body has to
  agree with it. That is the `C2556` above, and it is a catalogue fix.
- A row WITHOUT one (`sub_5e3630`, every `leaf_*_redirect`) gets NO declaration
  of itself at all. `byte_match.object_code` compares the object's CODE and
  never looks a symbol up, so the return type, the convention and the parameter
  list are all free, and a class introduced to express `__thiscall` costs no
  bytes. The brief forbade changing them anyway, purely by wording, which put
  every convention error out of reach for no reason.

## Four levers refuted on 0x005E3630, all cheap to repeat and all worthless

Twenty bytes, every mnemonic already agreeing, one register wrong at index 4:
the original chains `mov eax,[eax+8]` -> `mov eax,[eax+4]` and the rebuild
borrows ecx, `mov ecx,[eax+8]` -> `mov eax,[ecx+4]`. What does NOT move it:

- **A real `__thiscall` member** instead of the fake `__fastcall(void *, void *)`.
  Byte-identical output. The fake receiver is not the problem.
- **Reusing one pointer variable** instead of two named temps. The obvious
  reading - one variable, one register - is wrong.
- **Collapsing the chase into a single nested expression.** Also identical.
  Note this is the REVERSE of 0x006281B0, where BINDING a component to a local
  was what flipped fmul scheduling. Operand binding moves some things and not
  this one; it is not a general lever.
- **The whole `/G` processor family.** `/G3 /G4 /G5 /G6 /GB` all emit exactly
  the same bytes here. Instruction scheduling was the best remaining theory for
  the register-allocation class and it is dead for this function.

One real signal did come out of it: at `/O1` the TAIL is exactly right
(`mov eax,[eax+8]; mov eax,[eax+4]`) while the head is wrong (`cmp [ecx+8],0`
instead of a load and `test`), and at `/O2` it is the other way round. The
original is `/O2`'s head with `/O1`'s tail. Whatever explains that is not a
flag in `FLAG_SETS`.

## Levers that DID pay, from the four-agent batch

- **A loop bound the original does not have emits a `cmp`.** `for (i = 0; x &&
  i < 32; i++)` against a plain shift-until-zero. Dropping the bound and
  writing it shift-then-test as a `do/while` was the whole fix (0x00628AB0).
- **A short `for` loop over byte stores gets unrolled AND store-merged.** VC6
  turned eight byte-writes into `or eax,-1` plus two merged dword stores.
  Writing the dword store and the byte stores separately stops the merge
  (0x0057DEE0).
- **Order the statements the way the original schedules them.** Materialising
  the address arithmetic into named temps BEFORE computing a wraparound counter
  kept the counter an `inc`; computing the counter first degraded it
  (0x006252C0).

## Naming proposals awaiting a catalogue change

Asking for names produced two the code justifies but that a body edit cannot
carry, because the identifier is catalogue data and the redirect stubs are
exported through the append-only `src/OpenSMACX.def`:

- `0x00448380` `field_accessor_00448380_redirect` -> something like
  `sign_extend_flag_bit0`. The whole body reads bit 0 of `+0x40` and smears it
  with `shl 31; sar 31`; the current name only restates the address.
- `0x00634650` `leaf_00634650_redirect(void *self, void *, const float *other)`
  -> `Vector3::Dot(const float *) const`, with `struct Vector3 { float x, y, z; }`.
  The receiver is indexed at 0, 4 and 8 as floats and `ret 4` pops the single
  stack argument, so the fake `__fastcall` receiver is standing in for a real
  `this`. Verified byte-exact in BOTH spellings, so the rename is free of any
  byte risk and is purely a catalogue and `.def` question.

## The XOR-swap idiom, and a lever that transferred

`?swap@@YAXPAHPAH@Z` at 0x00628A50 was written with a temp; the original is the
three-statement XOR swap with NO aliasing guard. Predicted to transfer to the
byte twin `?swap@@YAXPAEPAE@Z` at 0x00628A80 and it did, first try - the two
had been sitting at MISMATCH with the same `#2 push vs cmp` note.

When two rows share a fingerprint AND a name, fix one and try the other
immediately; that one cost a single compile.

## `BOOL` and `LPSTR` are NO_COMPILE, not a mismatch

`src/alpha.cpp`'s `noun_item` read as `#12 jne vs je` in the ledger but was
actually NO_COMPILE under the writeback recipe: the scaffolding forward-declares
types reachable from the decoded signature and from callee signatures, and a
Windows typedef used only for a LOCAL or spelled into a parameter is not
reachable. `BOOL` is `int` and `LPSTR` is `char *`, so respelling them changes
no type and no byte - but until it is done the body cannot be scored at all.

## Loops: VC6 will not unroll a constant fill, and an indexed loop costs a push

Three results in one batch say the same thing from different directions, and
together they are the most transferable lever found so far.

- **A constant fill stays a loop, always.** `AutoSound::init` zeroes 37 fields
  and the original is 132 bytes of straight-line stores; every loop spelling
  tried collapsed to a 31-byte address-stepping loop. There is no bound, no
  first-N-special-case and no unroll pragma that reaches it - the stores have
  to be written out. Write them against the CLASS MEMBERS rather than an
  index over a `volatile uint32_t *`: both are byte-exact and only one is
  readable.
- **An indexed loop needs one more value live than a pointer walk**, so VC6
  spills a callee-saved register for it. That spill IS the `push` at index 0.
  `sub_5ad450` went exact by walking a pointer with a down-counter
  (`do { ...; p += 3; } while (--count)`) instead of indexing.
- **An extra local can invert the whole register assignment.** `sub_642940`
  was MNEMONIC_ONLY with every mnemonic and every stack offset already
  agreeing: a `remaining = count` copy made VC6 put the counter in ecx and the
  value in edx, exactly the reverse of the original. `while (--count > 0)` on
  the parameter itself flipped it back.

So `push` at index 0 is worth ONE attempt at the loop form before it is filed
under register allocation. It was the answer twice in this batch.

## Where `push` really is register allocation

Two functions in the same batch resisted it, and both are worth leaving alone:

- `?bit_count@@YAHH@Z` (0x0050BA30). ELEVEN structurally distinct variants -
  do-while against for, `if` against `+=`, an added live temp, a guard, a
  writeback through the parameter's stack slot, signed against unsigned - all
  landed on the identical `#2 push vs mov`. VC6 never chose ebx for the loop
  and never folded `and`+`add` into the original's `shr`/`adc` carry chain.
- `?findnum@@YAHPAD@Z` (0x00628B30). Reshaping the loop took mnemonic
  similarity from 0.49 to 0.667 and reproduced the original's check ordering,
  but the residue is `and reg,reg` against `test reg,reg`, a full-register
  `xor ebx,ebx` before a byte-only loop, and a store/reload to share one `pop`
  across three exits.

Neither was landed: `mizuchi_writeback.py` only accepts BYTE_EXACT, and
replacing a working body with a differently-shaped one that is still not exact
trades a known quantity for a guess.

## `goto` to a shared failure tail is sometimes the honest spelling

`sub_6344e0` compares three floats and is byte-exact ONLY when the three tests
are unrolled and share ONE failure return. A `return false` inside each arm
duplicates the tail; a loop pays a `push`. The last comparison also has to be
spelled `==` where the first two are `!=`, because VC6 lays the final block out
the other way round. The form check flags `goto` as questionable and it should
keep doing so - but here the alternative is not a cleaner body, it is a body
that does not match.

## The loop-form lever is real but NOT general - three refusals in one batch

Told about the `sub_5ad450` pointer-walk fix, three agents applied it and it
did not transfer. Recording them so it does not get overgeneralised the way
"push is register allocation" was:

- `sub_5ad4c0` is the IMMEDIATE NEIGHBOUR of `sub_5ad450`, same table at
  `+0xA20`, same 3-word stride - and the pointer walk made it strictly WORSE,
  moving the divergence from #4 to #2. Address proximity does not imply the
  same register-pressure shape.
- `sub_6348c0` is a genuine nested 3x3 loop (the original has two independent
  `dec`/`jne` pairs, so it is not a flat 9-loop the compiler split). Matching
  that nesting closed most of the gap - 26 of 39 bytes to 36 of 39, every
  mnemonic in order - and the residue is one `push`/`pop` pair. Pointer walks
  were worse here, not better.
- `?checksum@@YAEPADHE@Z`: the reverse indexed walk `seed ^= buffer[--size]`
  moved the divergence from #0 to #3 and matched the byte LENGTH exactly. The
  wall is that VC6 schedules `push esi` after the `size == 0` test while the
  original saves it unconditionally before, then pushes and pops esi AGAIN
  inside the taken branch. No source shape reached a double save.

The pattern across all three: the lever moves the divergence LATER and often
matches the length, which is real progress and still not a match. Ranking by
`first_divergence` alone would call these wins.

## A settled hypothesis is not a hypothesis

`sub_532a50` carried a pre-registered diagnostic in `msvc6_byte_match.py`
arguing its ternary should be an `if`. Half right: the ternary was implicated,
but the `if` does not match either. What matters is the `rounded` TEMPORARY -
either spelling of it keeps one more value live and VC6 stops spilling the
divisor to esi. `++quotient` in place is byte-exact. The diagnostic is retired
and the perturbation re-pinned.

## The four return-type defects need a CATALOGUE change, and here is exactly where

`BaseButton::on_key_click/on_key_down/on_key_up` (0x006077F0, 0x00607800,
0x00607810) and `?do_sound@@YAXXZ` (0x005FD2B0) all emit `xor eax, eax; ret N`
- they return a value - while the catalogued mangled name spells `X`, void.
The sibling class `BasePop` declares the same handler `int on_key_click(int,
int)`, so the `void` is an analysis error rather than a real difference.

`derived-prototypes.csv` is NOT the way in. It carries a `prototype` and a
`convention` per address, but `emit_translation_unit.py:716` then does

    decoded = decode_signature(self.mangled)
    if decoded is not None and len(decoded[1]) == len(self.params):
        self.returns, self.params = decoded[0], list(decoded[1])

and that is deliberate: the derived prototypes are written in IDA's alphabet,
which collapses `char`/`signed char` and `long`/`int`, so the mangled name has
to win or 108 rows emit a symbol no target object holds. The consequence here
is that the RETURN TYPE cannot be overridden per-address at all.

So the fix is to correct `name` in `functions.csv` to `...@@QAEHHH@Z`, and
that file is promoted from the canonical IDB export rather than hand-edited.
Total prize: about 18 bytes across four 3-to-5-byte functions. Left alone
deliberately - it is a change to how the catalogue is derived, and the
unrecovered pool is worth orders of magnitude more.

## The ledger was honest and the extraction was not

The harvest refused 121 of 186 proved recoveries, and the commit that landed
it said those were "ledger rows claiming BYTE_EXACT that the current toolchain
cannot reproduce". THAT WAS WRONG. All 122 unit-only rows were re-measured by
compiling their units WHOLE, and every one is still BYTE_EXACT. The proofs
were fine; the extraction was broken, in two different ways at once.

- 83 units no longer contained the definition head the emitter computes today,
  because the catalogued NAME has since been corrected - including by
  `catalogue_corrections.py` earlier the same day. The head moved; the body
  did not.
- 38 more extracted a body that would not compile, because agents declare shim
  classes and typedefs ABOVE the definition, and cutting at the head throws
  them away. `NO_COMPILE` there was self-inflicted.

Cutting where the SCAFFOLDING ends - at its last real line of code, not its
prose and not the subject's signature - rescues 134 instead of 65.

The wider audit is worth recording too, because it is the reassuring half.
Of the 670 rows the ratchet counts: 465 have a committed `src/` body, 83 are
in the store, 122 existed only as a gitignored `unit.cpp`, and ZERO have no
artifact at all. Nothing in the ledger is fabricated. The exposure was never
false claims - it was that a `git clean` would have erased 122 real proofs
while the ledger went on counting them.

A REFUSAL RATE IS A CLAIM ABOUT THE TOOL BEFORE IT IS A CLAIM ABOUT THE DATA.
121 of 186 was read as rot in the ledger and reported that way, when the
cheaper hypothesis - that a two-day-old extractor was wrong - was both true
and testable in one pass.

## The original's flags are not in FLAG_SETS, and two functions now say so

Two independent recoveries have stalled on the SAME shape: the original is
`/O2`'s output in one place and `/O1`'s in another, and no single flag set in
`byte_match.FLAG_SETS` produces the mixture.

- `sub_5e3630` (20 bytes). `/O1` emits the exact TAIL - `mov eax,[eax+8];
  mov eax,[eax+4]`, chaining through one register - while `/O2` emits the
  exact HEAD, a load and a `test` where `/O1` has `cmp [ecx+8],0`. The
  original is `/O2`'s head with `/O1`'s tail. Seven source shapes, a real
  `__thiscall`, one reused local, a nested expression and the entire
  `/G3 /G4 /G5 /G6 /GB` scheduling family all leave it untouched.
- `sub_52a980` (37 bytes). `/O2` reproduces the loop bound, the `add`/`cmp`/
  `jl`, and the trailing `xor eax,eax` plus two stores - but always
  register-mediates `*p &= 0xfe` instead of leaving the original's direct
  `and byte ptr [eax], 0xfe`. `/O1` keeps that `and` as a direct memory op and
  then gets the rest wrong: it re-encodes the `0xFF` store as a
  memory-immediate every iteration rather than hoisting it into `cl` once, and
  collapses the trailing zero-out to `and dword ptr [...], 0`. Eight source
  shapes ruled out, including a minimal single-field repro that isolates the
  mediation as inherent to `/O2` rather than caused by the second field.

Two functions is not proof, but it is the second time the residue has this
exact character, and it is a different character from register allocation: in
both cases each HALF is reproducible exactly, just never together. That points
at a compiler setting rather than at the source - a per-file `#pragma
optimize`, a flag combination nobody has tried, or a different build of
12.00.8168. Worth an experiment before more agent time goes into either.

THAT EXPERIMENT WAS RUN AND THE HYPOTHESIS IS DEAD. 288 combinations were
swept over `sub_5e3630` - the product of `/Og`, `/Oi`, `/Os` against `/Ot`,
`/Oy` against `/Oy-`, `/Ob0`/`/Ob1`/`/Ob2`, and `/Oa` against `/Ow`, which is
the `/O1` and `/O2` bundles taken apart into their actual components. Not one
reached BYTE_EXACT or even SHAPE_EXACT.

So the residue is NOT a flag setting, at least not anywhere in the `/O`
family, and the "the original used settings we do not have" reading has to be
dropped. What survives is narrower and stranger: each half is reproducible,
the two are never available together, and no optimisation switch selects
between them. That leaves a per-function `#pragma optimize`, a different build
of 12.00.8168, or something about the surrounding translation unit - none of
which a body rewrite can reach, and none worth more agent time until there is
a new reason to look.

The cost of finding this out was one command. The cost of NOT finding it out
would have been every future agent re-deriving "it might be the flags" on
every function in this class.

## The negative offsets ARE virtual inheritance, and that is now measured

Three recovered bodies dereference a negative offset from `this`, and two of
them - `CheckBox` at -0x1C and `SpriteBox` at -0x8C - carry the identical
five-instruction sequence:

    mov eax, [ecx - N]           ; the vbptr, N bytes BEFORE this
    mov edx, [eax + 4]           ; a this-adjustment delta from vbtable[1]
    mov eax, [edx + ecx - N]     ; the adjusted object's vtable
    lea ecx, [edx + ecx - N]     ; the adjusted this
    call [eax + 0xf8]            ; virtual slot 62

That is MSVC's canonical virtual-base access, and it was CONFIRMED rather than
assumed. A declared hierarchy - a non-virtual lead base plus
`public virtual VBase`, with the call written as an ordinary `slot062()` -
compiles to SHAPE_EXACT against 0x0060FB90: all six instructions, every
register, in order. The single difference is the displacement, `+0x18` where
the original has `-0x1C`.

WHAT THAT PROVES AND WHAT IT DOES NOT. It proves the model: these classes are
virtually derived, the compiler generates this sequence for them, and no
hand-written cast chain is needed to reproduce it. It does NOT give the
hierarchy. A negative displacement means `this` sits N bytes PAST the vbptr,
which happens when the function's class is a later base inside a larger
object - and which class encloses `CheckBox`, and at what offset, is not
visible from `CheckBox`'s own code. Two arrangements were tried and ruled out:
a lead base plus a virtual base puts the vbptr at +0x18 (positive), and an
`Owner : virtual VBase` with `CheckBoxImpl : Owner` changes the shape outright
(`add` where the original has `lea`).

So the inheritance is real, the sequence is reproducible, and the missing
piece is one fact about the ENCLOSING class - which a caller of one of these
functions would show. That is a targeted question for whoever recovers a
caller, not a guess to make here.

The cast-based bodies stay as they are meanwhile: they are byte-exact, and a
declared hierarchy would change layout and vtable order for every other body
on the same class. Modelling it is worth doing when the enclosing offset is
known, and not before.

## `hypothesis_layouts.h` reaches no compiled translation unit

Nothing in `OPENSMACX_SOURCES` includes it. The only `#include` anywhere is in
a `src/recovered/` staging file, which is not compiled either. So the header
exists, is regenerated, is checked for staleness - and no build has ever seen
it.

That corrects something claimed here earlier. Feeding proved members into
`emit_hypothesis_layouts.py` was described as fixing "the PRODUCT side" while
leaving verification untouched. It fixed neither: the generated header is not
compiled, so naming a member there changes nothing about any build.

The change that DID work is the separate one, and it works for a different
reason: `emit_translation_unit.proved_member_declaration` calls
`emit_hypothesis_layouts.proved_members()`, which reads
`agent-structure-observations.csv` DIRECTLY. It never reads the header. So the
scaffolding gains the members, recovered bodies can name them, and
`Midi::set_base_path` is byte-exact writing `field_3c_` - all of which is real
and none of which came from the header.

Two things follow. A generated artefact nobody includes cannot be assumed to
have an effect just because it is regenerated and gated; and when two changes
land together and one works, the working one is worth isolating before its
success is attributed to the other.

What the header would be FOR is product code: a recovered body integrated into
`src/` that wants to say `field_3c_` needs the declaration from somewhere the
build reads. That means including it from the files that need it, or promoting
those classes into headers of their own. Neither has been done, and until one
is, integration keeps needing the offset-cast spelling.
