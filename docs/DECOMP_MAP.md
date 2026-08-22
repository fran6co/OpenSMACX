# The source map: src/ is the single record of the decompilation

The decompilation is tracked in exactly one place: the source tree. Every
piece of the binary the project maps carries an annotation in a `.cpp` file
naming the bytes it claims, and one tool asks the original compiler what
state each piece is in.

**As of 2026-08-12 that is literally true.** `functions.csv` and
`callgraph.json` are deleted. Every annotation carries its own `name`, `size`,
`spans`, `prototype`, `callers`, `kind`, `flags`, direct `calls` and `indirect`
call sites, stamped by `tools/project_catalogue.py` and read back by
`emit.load_functions()`. The IDA database remains the SEED - it is the only
thing that can produce these facts in the first place, since the image carries
no symbols - but it is no longer a runtime dependency, and
`export_recovery_inventory.py` regenerates an export on demand for
`project_catalogue.py --check` to compare against.

Tool: `tools/decomp_status.py` (reader: `tools/annotation_scan.py`).
Offline tests: `tools/test_annotation_scan.py`,
`tools/test_decomp_status.py`. Measured 2026-08-09: 4,095 implemented
pieces, 1,488 placeholders, 376 exclusions — 5,958 of 6,000 catalogue rows
mapped; the remaining 42 are `source_complete` rows the scan cannot locate
and a human must place.

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
on 2026-08-19 for the records the build compiles. **`decomp` reads this
fact; `tools/` does not yet**, and an annotation without one is simply a
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
is `EXCLUDED S<n>` with a population `measure_exclusions.py --check` re-derives,
or it is a long `TRIED` list, and nothing in between. One marker carrying
both "I stopped" and "nobody should try" is the trap that got a `static_assert`
migration reverted.

`--check` enforces three things, all offline and ahead of every early return, so
a checkout with no VC6 still applies them: **state** (`TRIED` on a proved
body, or `LEVER` on an unproved one), **placement** (`TRIED` on a
placeholder — you cannot rule a spelling out of a body that does not exist), and
**syntax** (`LEVER:` with no fingerprint, which is a lesson nothing can group).

That placement rule has a deliberate consequence: an agent whose attempt does
not match must LAND it under `src/unrecovered/<addr>.cpp` before it can record
what it tried. Today such a run leaves nothing behind and the next agent starts
from zero.

`// EXTERN-SYMBOL LEVER:` is a different, older convention that appears inside
recovered bodies; the anchored grammar above does not read it.

**State is measured, not claimed.** The only declared state is EXCLUDED,
because exclusion is a decision. Everything else is derived: a region that
still holds the `// BODY GOES HERE.` sentinel with nothing but the
emitter's own residue after it (comments, the closing brace, the
PLACEHOLDER-tagged return) is a **placeholder**; anything else is
**implemented** and goes to the compiler. A file that claims a match in a
comment and does not hold one in bytes is not a match.

Prose stays as ordinary comments around the marker; the parser keys only on
the marker line. Layout is free: one function per file or many per file,
anywhere under `src/` — the scanner is decoupled from organisation by
construction, and a placeholder may move to its permanent home once the
body is written.

## Placeholders

One file per undecompiled function under `src/unrecovered/<addr>.cpp`,
generated by the tool and stamped with the catalogue's facts so the file
speaks for itself:

```cpp
// ORIGINAL: 0x00405C20 FILE
// placeholder - not yet decompiled
// name      ?bar@@YAXXZ
// size      128 bytes
// spans     0x00405C20-0x00405CA0
// prototype void __cdecl bar(void)
// callers   12   call targets   3
// To start: tools/decomp_status.py --work 0x00405C20

// BODY GOES HERE.
```

Excluded rows get files the same way, carrying their ground instead of a
sentinel. The directory is not in `OPENSMACX_SOURCES` (a hand list) and
never reaches the DLL build.

The decompile loop: `--work <addr>` materialises the emitted scaffold over
the placeholder, keeping the marker and the sentinel; the state stays
placeholder (measured) until a real body replaces it; then
`tools/decomp_status.py <file>` compiles it with VC6 and reports the tier.

## The status tool

```
tools/decomp_status.py                     full map: states, drift, verdicts
tools/decomp_status.py --state-only        the map alone; no VC6 needed
tools/decomp_status.py src/stringstruct.cpp
tools/decomp_status.py --addresses 0x401640,0x402530
tools/decomp_status.py --generate-placeholders
tools/decomp_status.py --work 0x00405C20
tools/decomp_status.py --gaps              image bytes in no catalogued span
tools/decomp_status.py --no-cache --verbose --json
```

Measurement reuses the ratchet's machinery VERBATIM — census extractor and
scaffolding for body mode, the writeback recipe for the proved store,
`byte_match.match_functions` (batched response-file compiles, four flag
sets, best-of, early exit on BYTE_EXACT) for the comparison. A cold pass
over the whole tree is ~2 minutes; a content-hash cache
(`.opensmacx/decomp-status-cache.json`, keyed on the BUILT unit) makes warm
reruns seconds. Without VC6/Wine/the pinned executable the tool prints the
reason, exits 0, and still reports the map and drift.

Verdicts merge into the ignored cache `.opensmacx/byte-match.csv` keyed
by address: rows
this run did not measure are preserved verbatim, and **a BYTE_EXACT row is
never downgraded** — a failure to reproduce it is kept and printed as
UNREPRODUCED, because that is either a tooling change or a lost scaffolding
and both need a human.

**THE RATCHET IS `tools/decomp_status.py --check`**, and it reads the claims
in `src/`, not the CSV. The floor is the number of claims, so there is no
constant to bump: every piece a run proves gets `BYTE_EXACT` written onto its
own annotation by `--record-matches`, and a claim that stops reproducing
fails the check by address and file. The previous form compared two hardcoded
totals against the ledger, and counted `0x0064F09C` - whose body had been
reset to `// BODY GOES HERE.` after the proof - for months.

## Drift, gaps, and the two populations

The report names where map and catalogue disagree: implemented pieces whose
catalogue row still says unrecovered, `source_complete` rows with no
annotation, annotations the catalogue does not know, stale
`source_locations`, duplicate addresses. Drift is reported, never failed
on — it is the worklist for inverting the truth direction.

`--gaps` measures the image bytes no catalogued span covers (padding,
switch tables, funclet edges): the map claims completeness, and that claim
stays a measurement.

Generated thunk files (`init_thunks.cpp`, `atexit_thunks.cpp`, the rest of
`byte_match_census.GENERATED_FILES`) are reported in a separate block from
hand-written bodies and never totalled with them — the census rule, carried
over.

## Migration — done 2026-08-09

The whole tree was migrated in one guarded pass
(`tools/decomp_status.py --migrate --apply --rewrite-locations`, a flag
that no longer exists — see below): 1,607
files rewritten, every rewrite proven comment-only (code content
identical) and address-set-preserving before anything was written; the
full VC6 re-measurement preserved every ledger tier; the DLL rebuilt
clean. The legacy spellings (`Original Offset:` blocks, the inline
trailing and opening-brace forms, the two `src/recovered/` header
styles) are still RECOGNISED read-only and flagged `deprecated`, but an
offline test now pins the convergence invariant: no deprecated spelling
may remain in `src/`, and the writers (`writeback.py`,
`preserve_worked_units.py`) emit the marker on every new file.

One rule the migration surfaced: `source_locations` is not the map — it
is the SCORING ROUTE. A row with a location belongs to the census; a
body under `src/recovered/` stays scoreable only while its row is
unowned (`test_collect_ownership.py` pins this); a placeholder is a
promise, not an implementation. Only an IMPLEMENTED annotation in
product source may occupy the column.

`--rewrite-locations` used to enforce that by writing the column back
into `functions.csv`. That export is deleted, so the flag could only
raise `FileNotFoundError`; it and `rewrite_source_locations` were
removed on 2026-08-12. The column is now derived on READ, per row, by
`project_catalogue.from_source()` — `annotation.location` when
IMPLEMENTED, empty otherwise — so it cannot go stale between rewrites.
What did not survive the move is the `src/recovered/` half of the rule:
`from_source` applies no such filter, and 1,477 of the 6,000 rows carry
a `src/recovered/` location today (measured). That is a live question
about what the census scores, not a leftover of the deleted CSV.
