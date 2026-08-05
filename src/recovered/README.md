# Matched bodies that no file owns yet

One file per function, named for its address, holding a body Mizuchi proved
byte-exact against the pinned original.

## Why they are here

`docs/recovery/functions.csv` gives these functions no `source_locations`:
nothing in the tree owns them, so there is no catalogued span to splice and no
file to append to. Choosing one - a file, a position, an include set, a place
in the class - is a recovery decision, and `tools/mizuchi_writeback.py` does
not make it. Before this directory existed the consequence was that the body
was thrown away: the ledger recorded BYTE_EXACT and the text stayed in
gitignored `build/byte-match/`, so a `git clean` undid a day of proving.

They live under `src/` because they are source, not documentation. They live
in their own directory because they are not yet PRODUCT source.

## What they are NOT

**Compiled.** `CMakeLists.txt` names every file in `OPENSMACX_SOURCES` by
hand - there is no glob over `src/` - and none of these are in it. Nothing
here reaches the build.

**Written in the tree's style.** These are what `tools/emit_translation_unit.py`
emits for verification: opaque class shells, globals bound to fixed addresses,
fields reached by offset cast. `AGENTS.md` keeps that out of `src/` on purpose,
because recovered code there is written to BEHAVE - rebindable seams, documented
offsets, an EH frame decision. A file here compiles against generated
scaffolding, not against the project's headers, and would not compile if you
added it to the build.

Rewriting one in the tree's own style, with a leaf test, is the per-recovery
loop in `AGENTS.md`. This directory is what that work starts from, not a
substitute for it.

## Keeping them honest

`tools/byte_match_fanout.py --collect` re-verifies every file here the same
way it re-verifies the fan-out work area: it rebuilds the unit from the
current emitter and re-measures against the original. A body that stops
verifying because the scaffolding changed shows up as a ledger regression
rather than as a stale file nobody reads.
