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
