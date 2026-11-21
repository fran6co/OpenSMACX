# Mac witness tools (2026-11-21)

The Mac SMACX Carbon build (CodeWarrior, PowerPC, 2003) is a second compile of
the game's source. Its traceback tables name 3,754 functions with their C++
signatures. These tools read it; the binaries stay in ignored
`.opensmacx/witness/mac/` (see `.opensmacx/witness/mac/src.sha`).

- `pef.py`: PEF reader. Code/data sections (pattern-initialized data unpacked),
  loader imports and relocations, TOC, traceback names, ordered `bl` targets
  (`calls()`), cross-TOC glue resolved to import names, unnamed targets (the
  statically linked runtime) as `@0xOFFSET`. `demangle()` gives `Owner::name`.
- `w1.py [unit ..] [--addr A] [--all] [--json OUT]`: call-site report. For each
  non-exact marker with a Mac twin, the twin's named callees against ours
  compiled at `/Ob0`, with our callees that have no Mac function expanded into
  their own callees (CodeWarrior inlines small inlines too). `MAC-ONLY` = the
  source calls a helper we do not; `OURS-ONLY` = mostly CodeWarrior inlining,
  read with care. 50 s over the whole tree on one lane (`FE_PFX_BASE`).

Limits: the Mac port is the 2003 source, so a difference is a candidate until
the Windows image distinguishes it; helpers CodeWarrior inlined are invisible;
it says nothing about VC6 register choices. Known name mismatches: the Mac's
`ClassList<T>` / `PointerList<T>` templates against our `StringStruct` family.
