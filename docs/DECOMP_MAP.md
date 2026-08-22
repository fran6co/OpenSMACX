# The source map: src/ is the single record of the decompilation

The decompilation is tracked in exactly one place: the source tree. Every
piece of the binary the project maps carries an annotation in a `.cpp` file
naming the bytes it claims, and one command asks the original compiler what
state each piece is in.

Every annotation carries its own `name`, `size`, `spans`, `prototype`,
`callers`, `kind`, `flags`, direct `calls` and `indirect` call sites. The IDA
database was the SEED - the only thing that could produce these facts in the
first place, since the image carries no symbols - and each fact now travels
with the annotation that needs it, so nothing else has to be consulted to
read the map.

Reader: the `decomp` package (`uv sync` installs it editable, so
`from decomp import read` works anywhere). Driver: `tools/osmx.py`, the
command line over it. Offline tests: `uv run pytest decomp/tests` - the
parse, the read -> write -> read loop over every annotated file, and what
each module may import.

## The grammar

One marker, valid in any comment form (`//` or inside `/* */`); the whole
map is `grep -rn "ORIGINAL: 0x" src/`. The marker NAMES the piece before
saying how to read it: the mangled name and the image spans ride the
marker line, ahead of the keywords — an annotation that cannot say what it
names and where the piece ends in the image cannot be added. (Moved to
this spelling 2026-08-18; the `// name` and `// spans` fact lines came
out of the tree with it.)

**`name` is what the IMAGE calls the piece, and it is not always what this
tree emits for it.** A body that re-expresses a compiler-generated construct
— an adjustor thunk, an atexit destructor, a scalar deleting destructor —
carries a symbol of its own by design, and a catalogued name can simply be
wrong about a return type (`?map_init@@YAXXZ` says void over a body
`src/map.cpp` defines as `BOOL __cdecl map_init()`). Both are recorded, as
two facts rather than one corrected into the other:

```
// ORIGINAL: 0x0041B4C0 ?on_scrolling@BaseWin@@QAEXHH@Z 0x0041B4C0-0x0041B4D7
// symbol    @base_win_on_scrolling_redirect@16
```

A `// symbol` line names the symbol THIS TREE'S compiler emits, measured
from the object rather than guessed, and its value is a single token — the
key alone cannot be trusted, because `src/` carries 2,037 prose lines that
begin `// symbol ` and every one of them is a sentence. 1,543 were written
on 2026-08-19 for the records the build compiles. Every reader goes through
`decomp`, which reads this fact, and an annotation without one is simply a
piece whose emitted name is its `name`.

```
ORIGINAL: 0x00401640 ?init@Game@@QAEHH@Z 0x00401640-0x00401B00
                                  the function definition that follows is
                                  the decompiled body; it is extracted and
                                  measured in generated VC6 scaffolding
ORIGINAL: 0x00403B70 ?f@Win@@QAEXXZ 0x00403B70-0x00403F00 FILE
                                  the WHOLE file is the translation unit
                                  for this address; compiled verbatim
ORIGINAL: 0x0064A123 ?crt_init@@YAXXZ 0x0064A123-0x0064A130 EXCLUDED S1
                                  deliberately not decompiled; the token
                                  cites the ground in docs/EXCLUSIONS.md
ORIGINAL: 0x0046FB10 ?g@@YAXXZ 0x0046FB10-0x0046FB40 BYTE_EXACT
                                  RATCHET CLAIM: this was proved to
                                  recompile to the shipped bytes, and
                                  `--check` fails if it stops
```

`BYTE_EXACT` composes with `FILE` in either order, and it is the ONLY verdict
ever written into the tree. It is a claim, not a status field: every other
tier — MISMATCH, NO_COMPILE, MNEMONIC_ONLY — is measured on demand and
recorded nowhere, because a status field in a comment goes stale the moment
someone edits the body and is believed anyway. `--record-matches` adds the
token for everything a run proves and never removes one, so losing a claim is
a deliberate source edit visible in the diff.

The lookbehind matters: `PROVEN-AGAINST-ORIGINAL: 0x...` provenance
comments end in this spelling and are not map entries.

## What was learned, beside the body it was learned on

Two tokens, read only from the comment run immediately after a marker, so the
same words in prose further down a file are not a claim:

```
ORIGINAL: 0x005E3650 BYTE_EXACT
LEVER: jl/jge  flipped the `if` condition and swapped the arms
```
```
ORIGINAL: 0x0055B760
TRIED: plain immediate; named local; const; x|0xFF;
           register storage; a 1-bit bitfield
```

**They carry different durability, and that is the design.** A `LEVER` sits on a
body that MATCHED, so its divergence is gone and the fingerprint is historical —
it must be written down or the lesson cannot be filed against anything. A
`TRIED` sits on a body that has NOT matched, so its divergence is still live
and the key is **measured rather than written**: a key that is never written can
never be stale. An indented comment line with no token of its own continues the
line above.

**Both are self-refuting, which is what earns them a place under "state is
measured, not claimed".** A `LEVER` is already covered by the ratchet: the body
must keep reproducing. A `TRIED` fails the moment its body *does* match, so
a wall cannot quietly outlive its reason — it must be promoted to a `LEVER` with
its fingerprint, or deleted. There is deliberately **no `WALL:` token**: a wall
is `EXCLUDED S<n>` with its ground in docs/EXCLUSIONS.md,
or it is a long `TRIED` list, and nothing in between. One marker carrying
both "I stopped" and "nobody should try" is the trap that got a `static_assert`
migration reverted.

No automated check enforces LEVER/TRIED placement today; the rule lives here
and in the worker prompt. The convention that does the work: an agent whose
attempt does not match writes what it tried under the body's own marker in
`src/`, so the next attempt starts from what failed instead of from zero.

`// EXTERN-SYMBOL LEVER:` is a different, older convention that appears inside
recovered bodies; the anchored grammar above does not read it.

**State is measured, not claimed.** The only declared state is EXCLUDED,
because exclusion is a decision. Everything else is derived: a record whose
region holds no body at all is **NO_BODY** in `osmx status`; anything with a
body is **implemented** and goes to the compiler. A file that claims a match in
a comment and does not hold one in bytes is not a match.

Prose stays as ordinary comments around the marker; the parser keys only on
the marker line. Layout is free: one function per file or many per file,
anywhere under `src/` — the scanner is decoupled from organisation by
construction.

## Working copies

`src/unrecovered/` holds working-copy units materialised by the retired
scaffold route; they are verification artifacts, not product source, and are
not build inputs. New work happens IN PLACE: the worker edits the body under
its own marker in the `src/` file that claims it.
`uv run tools/osmx.py show <addr> --in unrecovered` still reads an old copy
when it is the best starting point.

## The status tool

```
uv run tools/osmx.py status            # every population, reachable or not
uv run tools/osmx.py show <addr|name>  # annotation + shipped disassembly
uv run tools/osmx.py calls <addr>      # call sites in image order
uv run tools/osmx.py measure <addr | --body F | --dir D>   # exit 0 only BYTE_EXACT
uv run tools/osmx.py record <addrs...> # measure AND stamp annotations
uv run tools/osmx.py semantic <addr> [--withdraw]          # equivalence claim
uv run tools/osmx.py configure         # regenerate the build database
```

Measurement goes through `decomp.asm`: the pinned image on one side, the
record's unit compiled by VC6 on the other, several flag sets compared, and
the best answer kept. `--dir` scores a whole directory of candidate bodies
best-first in one pass. Without VC6, Wine, or the pinned image the commands
print the reason instead of guessing.

**THE RATCHET IS `osmx check`.** It reads the claims in `src/`, recompiles
every one against the pinned image, links the tree, and fails by address and
file if a claim stops reproducing. The floor is the number of claims, so
there is no constant to bump: only `osmx record` writes the token, it stamps
only what it measured, and nothing ever removes one but a deliberate source
edit visible in the diff.

## Drift

There is no second catalogue left to drift against: the annotations ARE the
map, so drift is checked by re-measurement rather than by comparison with an
export. `osmx status` reports how many records the build can compile today;
records in non-build directories are counted separately, and an address
claimed by both product source and a leftover artifact is what
`tools/orphan_artifacts.py` exists to catch.

## Migration — done 2026-08-09

The whole tree was migrated in one guarded pass (a flag of the retired
status tool): 1,607 files rewritten, every rewrite proven comment-only (code
content identical) and address-set-preserving before anything was written;
the full VC6 re-measurement preserved every tier; the build rebuilt clean.
The legacy spellings (`Original Offset:` blocks, the inline trailing and
opening-brace forms, the two `src/recovered/` header styles) are still
RECOGNISED read-only and flagged `deprecated`, and the package's tests pin
the convergence invariant: no deprecated spelling may remain in `src/`, and
the writer emits the marker on every new file.
