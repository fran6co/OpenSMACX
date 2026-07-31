# Byte matching as a verification route

Measured 2026-07-31 on this host. Tool: `tools/msvc6_byte_match.py`.
Offline tests: `tools/test_msvc6_byte_match.py`, registered as
`msvc6-byte-match-tests`.

Every verification route this project has is bounded by something. The
differential oracles are bounded by the fault wall: a body that walks a pointer
chain cannot be called with a staged receiver, and lifecycle methods never
guard. The SMT prover is bounded by decidability, and its cost grows with the
body. Byte matching would have neither bound — if a recovered body recompiles
to the original instruction sequence, the cost of believing it is O(1) in the
body's size, its call graph, its loops, its COM and its SEH.

So the question was asked directly, with the compiler SMACX shipped with.

## Pre-registration

Fixed in writing before any measurement, and reproduced here verbatim so the
decision is auditable rather than retrofitted.

* **Population.** Five functions that are already `source_complete` in
  `docs/recovery/functions.csv`, are LEAF (no calls out), and do **no struct
  member access**. The last condition is load-bearing: a wrong `/Zp` packing
  and a wrong `/O` level produce the same symptom, so a body that touches a
  struct field turns an interpretable mismatch into an uninterpretable one.
* **Build.** `cl 12.00.8168 /c /O2 /Gy /GR-`.
* **Comparison.** Normalised capstone mnemonic sequences.
* **Verdict rule.** 5/5 → the route is real. 3–4/5 → partial. **<3/5 → KILL,**
  recorded in `docs/` so it is not rediscovered as untried.

The compiler is Microsoft (R) 32-bit C/C++ Optimizing Compiler Version
12.00.8168 — Visual C++ 6.0 RTM, the build SMACX shipped with; SP5/SP6 would
report 12.00.8804. It lives under `~/opt/vc6` and runs under Wine. One compile
costs **0.234 s**.

## What is normalised away, exactly

Over-normalising would manufacture a match, so this is stated in full.

Discarded: **all operands.** That removes register allocation, absolute
addresses, relocation targets, immediates and branch displacements — none of
which a fresh compile can reproduce, since the original's addresses do not
exist in an object file. Also stripped: trailing `int3` and `nop` COMDAT
alignment padding on the object side.

Not merged: **anything else.** `lea` is not `mov`, `imul` is not `add`, `inc`
is not `add`, `je` is not `jne`. A strength reduction the original performed
and the recompile did not is a mismatch — that is the signal, not noise.

One known blind spot: capstone spells `ret 8` and `ret` with the same mnemonic,
so the callee-pop count — the only ground truth for arity — does not reach the
verdict. Byte lengths are reported next to every result for that reason, and
`test_msvc6_byte_match.py` pins the blind spot rather than papering over it.

The original side reads the body's catalogued spans and concatenates them;
`size` is their **sum**, never `end - address`.

## Result under the pre-registered flags: 0/5

`cl /c /O2 /Gy /GR-`

| address | name | original | rebuilt | first divergence |
|---|---|---|---|---|
| `0x0042A020` | `?UNK1@Datalink@@QAEHHH@Z` | 30 B / 12 mn | 28 B / 9 mn | #0 `push` vs `mov` |
| `0x0042A040` | `?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z` | 54 B / 21 mn | 53 B / 18 mn | #0 `push` vs `mov` |
| `0x0050BA00` | `?bitmask@@YAXHPAHPAH@Z` | 39 B / 17 mn | 32 B / 11 mn | #0 `push` vs `mov` |
| `0x00532A50` | `sub_532a50` | 49 B / 24 mn | 49 B / 20 mn | #0 `push` vs `mov` |
| `0x00559210` | `sub_559210` | 17 B / 9 mn | 15 B / 6 mn | #0 `push` vs `mov` |

**By the pre-registered rule this is a KILL.** It is recorded as one.

It is also the same divergence five times, at instruction zero, for a reason
that is visible in the executable rather than inferred: `/O2` implies `/Oy`,
and **the shipped image keeps its frame pointers.** Every one of the five opens
`push ebp; mov ebp, esp` and closes `pop ebp`. The pre-registration fixed a
flag set that the binary itself contradicts. That is a mis-specified parameter,
not a property of the route, and saying otherwise would be the wrong lesson to
leave behind.

## Result with that one flag corrected: 3/5

`cl /c /O2 /Gy /GR- /Oy-`

| address | name | shape | original | rebuilt | verdict |
|---|---|---|---|---|---|
| `0x0042A020` | `?UNK1@Datalink@@QAEHHH@Z` | `__thiscall`; ×10000 then add | 30 B / 12 mn | 30 B / 12 mn | **MATCH** |
| `0x0042A040` | `?parse_id@Datalink@@QAEXHPAUDatalinkID@@PAH@Z` | `__thiscall`; ÷10000 signed, two stores | 54 B / 21 mn | 54 B / 21 mn | **MATCH** |
| `0x0050BA00` | `?bitmask@@YAXHPAHPAH@Z` | `__cdecl`; ÷8, variable shift, two stores | 39 B / 17 mn | 33 B / 14 mn | mismatch, #4 `and` vs `mov` |
| `0x00532A50` | `sub_532a50` | `__stdcall`; `idiv`, remainder, conditional round up | 49 B / 24 mn | 49 B / 24 mn | mismatch, #14 `je` vs `jne` |
| `0x00559210` | `sub_559210` | `__cdecl`; `idiv` then multiply back | 17 B / 9 mn | 17 B / 9 mn | **MATCH** |

Three exact matches, byte length included, across three calling conventions.
The matches are not trivial shapes: `?UNK1@Datalink@@` requires the compiler to
reproduce the original's strength reduction of `× 10000` into four `lea`s and a
`shl`, and `parse_id` requires it to pick the same magic-number reciprocal
sequence (`imul` by 0x68DB8BAD, `sar 12`, `shr 31`, `add`) for `/ 10000`.

**By the pre-registered rule this is partial.** Both readings are published
because both are true, and which one is "the answer" depends on whether a
wrongly specified flag invalidates the run or the route. It invalidates the
run.

### Flag sensitivity, measured

| flags | matches | note |
|---|---:|---|
| `/O2 /Gy /GR-` | 0/5 | frame pointer omitted; diverges at #0 |
| `/O2 /Gy /GR- /Oy-` | 3/5 | |
| `/O2 /Gy /GR- /Oy- /Ob0` | 3/5 | same code bytes; nothing here is inlinable |
| `/Ox /Gy /GR- /Oy-` | 3/5 | same code bytes on all five |
| `/O1 /Gy /GR- /Oy-` | 0/5 | decisively not it |

Neither `/Ox` nor `/Ob0` changes the emitted code on these five — the objects
differ outside `.text` only — so `/O2` and `/Ox` cannot be told apart here and
this measurement does not choose between them.

`/O1` is worth recording as a refutation rather than a near miss: it leaves
`× 10000` as a single `imul` where the original expanded it into `lea`s, and it
diverges on all five by instruction #3–#4. The shipped build was optimised for
speed, not size.

## Why the two non-matches missed — and neither is the compiler

Both were tested as hypotheses against scratch variants. **The committed source
was not edited**, and neither result is counted in the verdict above. The tool
runs these itself and prints them under a header saying so.

* **`0x0050BA00 ?bitmask@@YAXHPAHPAH@Z` — the recovered source is wrong.** The
  original divides *signed*: `cdq; and edx,7; add eax,edx; sar eax,3`. The
  recovery declares `uint32_t input`, so VC6 emits a single `shr`. The mangled
  name `?bitmask@@YAXHPAHPAH@Z` decodes to `void __cdecl(int, int*, int*)`, and
  `functions.csv` carries exactly that prototype — so the catalogue and the
  committed source already disagreed, and nothing had noticed. Changing the one
  parameter to `int` reproduces the original **exactly**, 39 B / 17 mnemonics.
  This is a real defect in a body already marked `source_complete`: it behaves
  identically for every input below 2³¹ and differently for a negative one, so
  no value-level oracle over plausible inputs would have found it. Byte
  matching found it on first contact. **It is deliberately left unfixed by the
  commit that records this**, because fixing it would move the score from 3/5
  to 4/5 in the same change that reports the score, and a pre-registered
  experiment that edits its own inputs measures nothing. It is a defect to
  repair on its own evidence, in its own change.
* **`0x00532A50 sub_532a50` — source form, not semantics.** The recovery uses a
  ternary, `remainder == 0 ? quotient : quotient + 1`, which VC6 lowers to
  `jne` with the arms swapped. Writing it as `if (remainder != 0) { ... }`
  reproduces the original **exactly**, 49 B / 24 mnemonics, `je` included. The
  two are the same function; the original's author wrote an if-statement.

## Controls

A byte-match checker that cannot fail proves nothing.

* **Positive control (required).** A matching body, `?UNK1@Datalink@@`, was
  perturbed by one operation — `+ static_cast<uint32_t>(a2)` →
  `- static_cast<uint32_t>(a2)` — recompiled, and compared. The comparator
  **reported a mismatch** at #9, `add` vs `sub`. It runs on every invocation
  against the first matching case.
* **Identity control.** Handed the same object twice the comparator reports a
  match, so a mismatch verdict is not the only thing it can produce.
* **Offline.** `test_msvc6_byte_match.py` pins that the normalisation refuses
  to merge `lea`/`mov`, `add`/`sub`, `je`/`jne`, `inc`/`add`, that the COFF
  reader refuses an object holding two functions rather than guessing, and that
  the skip path names what is missing instead of passing quietly.

## What this route costs, and what it is worth

Per function, the only labour is writing a standalone translation unit: the
recovered body verbatim, plus the declarations it needs and nothing more.
Compile and compare is 0.234 s and no human judgement. There is no fixture, no
staged receiver, no mutant sweep — which is what makes it different in kind
from the routes in [RECOVERY_ECONOMICS.md](RECOVERY_ECONOMICS.md), where cost
tracks mutant count at roughly one per 18–22 bytes.

Against that, three limits are now measured rather than assumed:

1. **The route needs the recovery to be written to MATCH, not merely to
   behave.** Two of five bodies here are behaviourally right and textually
   different, and one of those differences (`?bitmask`) is a genuine bug while
   the other (`sub_532a50`) is pure style. The comparator cannot tell them
   apart, and neither can a reviewer without doing the work. Every existing
   recovery in this repository was written to behave.
2. **So it is a strategy question, not a tooling question.** The tooling took
   one session and works. Adopting the route means changing how bodies are
   authored — preferring the source form the original compiler would have been
   given — and that is materially harder than writing to behave, on 2,557
   already-`source_complete` functions that were not written that way.
3. **The struct-access exclusion is not free.** It was pre-registered to keep
   mismatches interpretable, and it excludes most of the corpus. Whether `/Zp`
   can be pinned independently is untested, and until it is, every struct-
   touching body carries an uninterpretable failure mode.

The honest summary: **the mechanism works and the route is not dead, but the
0/5-then-3/5 result is a statement about the recovered sources, not about the
compiler.** Where a body was authored to match, VC6 12.00.8168 with
`/c /O2 /Gy /GR- /Oy-` reproduces the shipped instruction sequence exactly,
including its strength reductions and its magic-number division.

## Reproducing

```
tools/msvc6_byte_match.py                  # default: the measured flags
tools/msvc6_byte_match.py --preregistered  # the 0/5 run, verbatim
tools/msvc6_byte_match.py --flags "/c /O1 /Gy /GR- /Oy-" --verbose
```

Without `~/opt/vc6` or Wine the tool prints a SKIP naming what is missing and
exits 0, because CI machines have neither. `VC6_ROOT` relocates the install.
Its output is disassembly of proprietary bytes as mnemonic *names* only; no
opcode byte enters a source file or a commit.
