# Matched bodies with no `src/` home

One file per function, named for its address, holding a body Mizuchi proved
byte-exact against the pinned original.

## Why they are here and not in `src/`

`docs/recovery/functions.csv` gives these functions no `source_locations`:
nothing in the tree owns them, so there is no catalogued span to splice and no
file to append to. Choosing one - a file, a position, an include set, a place
in the class - is a recovery decision, and `tools/mizuchi_writeback.py` does
not make it. Before this directory existed the consequence was that the body
was thrown away: the ledger recorded BYTE_EXACT and the text stayed in
gitignored `build/byte-match/`, so a `git clean` undid a day of proving.

## What they are NOT

Product source. These are written in the style
`tools/emit_translation_unit.py` emits for verification - opaque class shells,
fixed-address globals, offset casts - which `AGENTS.md` deliberately keeps out
of `src/`, where recovered code is written to BEHAVE. A file here compiles
against generated scaffolding, not against the project's headers.

Moving one into the tree, in the tree's own style, with a leaf test, is the
per-recovery loop in `AGENTS.md`. This directory is what that work starts
from, not a substitute for it.

## Keeping them honest

`tools/byte_match_fanout.py --collect` re-verifies every file here the same
way it re-verifies the fan-out work area: it rebuilds the unit from the
current emitter and re-measures against the original. A body that stops
verifying because the scaffolding changed shows up as a ledger regression
rather than as a stale file nobody reads.
