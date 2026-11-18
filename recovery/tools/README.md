# Fast matching tools (2026-11-18)

Screens only; confirm with `uv run recover measure` and bank with `uv run recover record`.
Run from the repo root with `uv run python .opensmacx/tools/<tool>`. They use wine prefixes
worker-21..34 (the queue uses 20) and never touch the worktree's build.

- `fe.py <unit> <addr[,addr]> [--src ALT.cpp] [--all] [--ctx N] [--win lo-hi,...] [--quiet]`
  Raw-compiles one unit (0.3-2 s), diffs the function's COMDAT against the image span with
  relocations masked. `!!` = differing row, `!>` = same instruction, different local jump target.
  A marker with `// compile_context <tu>` is measured in that unit's compile, as `recover measure`
  does (unless `--src` replaces the host).
- `feall.py [--all] [--base FILE] [--out FILE]` whole-tree pass: 420 non-exact functions in 13 s,
  all 5,574 markers in 17 s (`--all`). `--base` prints LOST/GAINED/MOVED: a header hypothesis
  can be tested tree-wide in under 20 s. About 46 claimed functions read inexact (measured in
  another unit, self-calls); the set is stable, so compare against a base.
- `perm.py <unit> <addr> --lines A-B [--max N] [--win ...] [--apply K]` every order of the source
  lines A..B (36 candidates/s). `perm.run_deck(unit, addr, [(key, source_text), ...])` evaluates
  any hand-built deck of whole-file variants.
- `padsweep2.py <unit> <addr> [--write]` sweeps the CANARY-PADS count 0..63 in seconds.
- `objdis.py <unit> <src> <addr>` raw listing of our function with real jump targets.
- `analysis/regmap.py <unit> <addr> [--src ALT.cpp] [--min N] [--ranges]` which callee-saved register holds
  which role (this, a named slot's cached copy, a call result at a source line, a temporary), for the image
  and for ours, with what the other side uses in the same aligned rows. Read it with MATCHING_GUIDE 2.7.
- `analysis/regprio.py <unit> <addr> [--src ALT.cpp] [--const N] [--blocks] [--cand a,b] [--quiet]` rule-4 priorities
  (MATCHING_GUIDE 2.7) of one function: blocks, loops and switch cases from our /FAsc listing, source references
  per block, webs by reaching definitions, `n_b * 2^depth` per reference, the predicted register beside the
  register our compile and the image (through the fe alignment) give each web, then every interfering pair the
  image orders differently from ours with the blocks that contribute most to each side. It also lists our
  compiler temporaries (`T:this+0x444` base-class `this`, `T:tmp[-4]` unnamed frame slots, `T:expr@line:reg`
  CSE'd values) as candidates, since those are usually the candidate a source-only count misses. Screen only:
  on 50 exact functions it orders 64% of interfering register pairs right (strength-reduced loop counters,
  address CSEs that replace an index, and constant pieces are the misses).
- `analysis/regalloc/` the probe sources and runner behind MATCHING_GUIDE 2.7 (see its README).
- `crtcalls.py OUT.json` compares, per function, the calls to memcpy/memmove/memset/strlen/strcpy/
  strcat/strcmp/memcmp in our objects with the image's (20 s). The game and Jackal have no
  intrinsics, so the multisets must be equal; a difference names a wrong spelling.
- `FE_DROP_FLAGS="/Oi"` (fe.py, feall.py) removes compiler flags from game/Jackal commands for a
  hypothesis run without touching the build.
- `analysis/constreg.py [unit ...] [--addr A] [--src ALT] [--weak] [--agree] [--json F]` constant-register
  diagnosis for every non-exact function (48 s tree-wide): where the image and ours dedicate a
  callee-saved register to a constant (IMG_ONLY = our source lacks a store, OURS_ONLY = one store too
  many or the register is not free, START, REG), with the stretch's uses and the function-wide
  immediate store/push/compare counts of that constant on both sides.
- `analysis/inlinemap.py <unit> <addr> [--src ALT.cpp] [--seq] [--sig KEY=a,b] [--callee KEY]` /Ob2 decisions:
  for every inline callee (found from call-count differences and the image's out-of-line copies), each site
  expanded (E) or called (c), image row over ours, with the addresses that disagree. Read with
  MATCHING_GUIDE 2.8. `--sig` supplies a signature for a copy whose own calls are all expanded
  (`??1StringList=?remove_all@StringStruct,??1StringStruct`).
- `analysis/inline/` the probe families and runner behind MATCHING_GUIDE 2.8 (see its README);
  `realdeck.run(unit, addr, deck)` evaluates whole-file variants and reports the call-count differences.
- `analysis/framescan.py [unit ...] [--addr A,..] [--from FEALL.json] [--all] [--slots] [--json F]` frame-size scan
  (13 s tree-wide with FE_JOBS=5): reads N from each prologue (`sub esp,N`, `push ecx`, `mov eax,N; call
  __chkstk`, with or without `push ebp` and the EH registration) in the image and in our compile, and lists
  the functions whose frames differ, largest difference first, with EH and ebp-frame flags. With `--addr`
  (or `--slots`) it prints both slot maps: every frame offset each side uses, access widths, `&` when the
  address is taken, reference counts, and our /Fa variable names. `--all` includes claimed markers (broken
  claims); markers with `// compile_context` are measured in that unit, as fe.py does.
- `analysis/typescan.py [unit ...] [--addr A,..] [--field SUBSTR] [--min N] [--uses] [--json F]` field-type scan
  (about 2 min tree-wide with FE_JOBS=5): compiles every unit with /FAsc, names the field behind each member
  and global operand (the source line's identifiers at that offset in the object's CodeView layouts, or the
  decorated global's type), and classifies the image's paired instruction: S (movsx; jl/jg.. after a dword
  compare; cdq+idiv; sar) or U (movzx or xor+mov r8; jb/ja..; xor edx+div; shr), following a loaded dword
  through moves and add/sub/imul. Lists the fields whose image uses contradict the declared signedness (or
  width), with counts in non-exact/exact functions, sample image addresses and the casts on that field in
  our source. A signed use of a 32-bit field is strong evidence against `unsigned` (int op uint is
  unsigned); an unsigned use can come from the other operand, so read U as a majority. Byte/word fields are
  judged by their extension only. `--uses` prints every attributed use of the `--field` selection.
- `analysis/loopscan.py [unit ...] [--addr A] [--src ALT] [--struct] [--tagged] [--json F]` loop-form census
  (MATCHING_GUIDE 2.4) over the non-exact functions (35 s tree-wide): tags the source loops that hand-write
  what VC6's induction-variable optimiser derives (`ptr+ctr` pointer stepped beside a counter, `clause+=`
  coordinate stepped in the for-clause, `down` trip-count down-counter, `rebind` `T *p = &A[i]` in the body,
  `memloop` memset/memcpy-shaped body, sibling counter names) and prints both sides' loop tests
  (`dec/jne`, `reg<&G jl` linear-subscript pointer, `reg<K jl` index/offset, `[m]<K` counter in a slot,
  signed/unsigned) with the frame size. STRUCT = frame or loop-test multiset differs; those sort first.
