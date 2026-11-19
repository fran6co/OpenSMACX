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
- `analysis/candset.py <unit> <addr> [--src ALT.cpp] [--min N] [--quiet]` the register candidates VC6 actually
  formed, read from the code of both sides: every callee-saved web (reaching definitions over the CFG) named by
  its role (`this`, `lea:this+K`, `[slot]{name}` piece, `#K`, `ret@line`), scored by rule 4/4b at instruction
  level and checked against its own side's registers; prints the roles on one side only and the pairs the sides
  order differently (MATCHING_GUIDE 2.7 "Candidate set"). `regprio.py ... --candset` appends the same report.
- `analysis/mergescan.py <unit> <addr[,addr]> [--src ALT.cpp] | --from FEALL.json [--json F]` tail-merged statements
  (MATCHING_GUIDE 2.7 "Tail-merged references count"; 50 s tree-wide with FE_JOBS=5): our source lines whose call left
  no `call` instruction while the callee is called elsewhere in the function (`MERGED`, with the identifiers of its
  arguments and the lines that kept a copy), and every `goto`. Register allocation runs before tail merging, so each
  merged copy still counts for its operands: compare with candset.py's disputed pairs.
- `analysis/gotowo.py <unit> <addr>` rule (b) screen: writes each label's tail block (ending in a return or the
  function end) out at subsets of its goto sites and fes every variant (`OUT=` keeps the best). Tree-wide over the
  85 goto functions on 2026-11-19 it found stack_veh 0x005B8EE0 (67 -> 52); farm_click 0x0040A1D0 (269 -> 54) was
  the same lever by hand. Long or mid-loop targets are skipped.
- `analysis/armwo.py <unit> <addr> [--write] [--show KEY] [--only d,t,c]` arm-structure screen, the successor of gotowo.py:
  parses the body into statements and fes four spellings: (d) `if (c) goto L; S; L:` as a negated block or an empty first
  arm per site (adjacent sites joined with `&&`), (t) a shared tail (after a label, or a list suffix ending in a return or the
  function end) written out at subsets of its predecessors (goto sites and the fallthrough arms of the preceding chain,
  implicit else included), (c) statements repeated at the end of every arm collapsed after the join, (s) the statements after
  a chain sunk into every arm. Top variants by rows are fingerprinted (call-count diff, frame diff, candset one-side roles +
  disputed pairs) and ranked calls, frame, candset, rows; `--write` keeps the best when it beats the base. Tree pass of
  2026-11-19 (par-2, about 200 functions >= 60 rows): 29 functions moved, 70,754 -> 69,009 rows, none lost; the sink (s)
  was the most frequent winner (ReportIf::draw_sat 197 -> 57, base_production 515 -> 335, ListBox::on_mouse_move exact).
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
- `analysis/loopsweep.py [--rule rebind|sibling] --json loops.json | --unit U --addr A [--write] [--min N]`; rebind:
  drops a `T &r = A[i]` / `T *p = &A[i]` local and writes `A[i]` at each use (skipped when the name or a
  subscript variable is reassigned), measures each site alone and with --write keeps the best site per
  function that lowers its rows without moving the frame or loop tests away from the image. Rerun per
  function until it writes nothing (first pass: 9 functions, 2,429 -> 2,304 rows, none exact lost).
  sibling: one counter name declared by several non-nested `for (int i ...)` loops and unused outside them
  becomes one function-scope `int i;` (per name, and all names together).
- `analysis/cdeck.py` (`cdeck.run(unit, addr, [(key, text), ...], focus=('on_map',))`) deck runner that prints, per
  variant, rows, the callee counts that differ from the image, our offsets of focus calls and our frame size: the
  quick screen for /Ob2 (MATCHING_GUIDE 2.8) experiments. `analysis/ccu.py <unit.cpp> ...` lists every function of
  the units whose call multiset differs from the image's.
- `analysis/runs.py DIFF.txt [min]` unpaired row runs (image-only = code we lack, ours-only = code we add) of a
  saved `fe.py --all` diff; `analysis/slotmap.py DIFF.txt` pairs our `[ebp+-N]` slots with the image's and lists
  our slots that serve several image slots (packed or shared locals).
- `analysis/unitexact.py <unit.cpp> [ALT.cpp ...]` fe rows of every marker of a unit for the unit and each
  whole-file variant side by side; prints the exact count per column and only the functions that moved (what a
  variant breaks elsewhere in its unit).
- `analysis/refscan.py [unit ...] [--addr A,..] [--claimed] [--unbound N] [--json F]` reference scan (50 s tree-wide
  with FE_JOBS=5): fe masks relocations, so a body that names the wrong global, the wrong offset into an object
  (`G[2]` for `G[3]`, another member) or the wrong literal reads exact by fe and SHAPE_EXACT by `recover measure`.
  For every fe-exact marker it runs the package's strict `reference_mismatches` with the measurement's source-bound
  targets and prints each wrong field with the symbol the image's address is bound to. `--unbound N` lists the
  symbols left masked (no single binding: vtables, asm-defined data, imports), i.e. the scan's blind spot.
  Operand order matters: VC6 loads the LATER operand of `a - b - c` / `a - b + c` first, so a swapped pair of
  globals at one expression is a source operand-order fix (PrefWin::init 0x0048D840).
- `analysis/tailscan.py <unit> <addr[,addr]>|--all [--src ALT.cpp] [--min K] [--agree]`, `tailscan.py --tree [unit ...]
  [--json F] [--hosts]` tail-merge / cross-jump scan (MATCHING_GUIDE 3.7 "Layout"; 50 s tree-wide with FE_JOBS=5). On each side it
  clusters the copies VC6 left unmerged: runs of K+ identical instructions (calls and globals by symbol, local jumps by
  target) that end in the same continuation (`jmp T`, a fallthrough into T, or a `ret` epilogue). A cluster one side keeps
  and the other lacks is printed as `IMG-MERGES` (ours keeps a copy the image has not got: the image jumps into another
  copy) or `OURS-MERGES` (the reverse), with each copy's address and source lines (`L123`, `hdr.h:L45`; image positions
  as `~L123`, our line at the nearest aligned row). `JMP` lines group the unconditional jumps whose image and our targets
  disagree, with both targets' lines and how many instructions apart the image lands: `-k` = the image cross-jumped k
  more instructions. Read with the probe/save_game notes in MATCHING_GUIDE 3.7: the image's choice of the join's
  fallthrough predecessor decides which copy survives. `jj=N` (and `JJ` lines) counts our jumper-into-jumper merges,
  the strongest single sign of a merge the image does not make (33/110 non-exact vs 15/626 exact functions sampled). `HOST` lines: each join whose host (the predecessor it is laid after) differs between the image and ours, both sides' predecessors as `line[last instruction]` in our source order (`H:` host, `x:` cross-jumped into it, `/T` inner then-arm tail), whether rules 1-2 of MATCHING_GUIDE 3.7 predict our host, and the source change they predict gives the image's (pop size: move a call across a branch; image host not first partnered: its tail was an inner then-arm; ours promoted by a then-arm: make it a lone if / else-arm; or reorder arms). `--tree --hosts` prints them all.
- `analysis/runscan.py [unit ...] [--fe FEALL.json] [--level 1|2] [--top N] [--json F]` repeated statement-run screen
  (MATCHING_GUIDE 2.8, "Helper screen"; text only, 20 s tree-wide): 2-5 statement windows of straight-line code (a no-else
  simple `if` counts as one statement) holding a call, normalised (locals V, numbers K, strings S; level 2 also collapses
  argument lists and subscripts), that occur 3+ times in the tree or 2+ times in one function, ranked by sites in non-exact
  functions, sites in exact functions marked `x`. `runscan.py <unit> --try 0xADDR [--wmin 1] [--keep] [--unit-exact]` turns
  every run repeated inside that function into a template inline helper (each alone and all together, implicit members
  retried as `this->`), with and without the function's dead probe lines (calls of `*probe*` or lines of calls tagged
  WORKAROUND(inline)), and prints fe rows, the inline E/c strings that moved, and (`--unit-exact`) the unit's exact
  functions it breaks. `--keep` adds each helper with only the first k probe lines. Templates pass locals by reference,
  which costs more than a hand-written int parameter (see the guide), so confirm a hit with typed spellings.
- `analysis/ownerscan.py [unit ...] [--addr A,..] [--probes 8,32] [--repeat] [--swaps] [--json F]` missing inline
  owner screen (MATCHING_GUIDE 2.8 "Missing owner, not missing budget"): per non-exact function, the inline callees
  the image calls more often than ours, our call counts with a dead 0/8/32-probe pass-1 block at the top of the
  body (one recompile per unit and size, every target function probed at once), and a verdict: `budget` (the probe
  turns ours into the image's calls: find natural spend), `partial`, `owner` (nothing flips: the image's sites sat
  inside an inline helper our source writes out). `--repeat` lists the function's source lines naming an owner
  callee and other functions with the same 2-3 statement window (windows found in more than 4 places are idioms and
  are dropped). `--swaps` adds per-site E/c rows for callees with equal counts (slow on large functions). The probe
  `probe calls` column counts refused probes (pass-1 budget already spent, the probe measures nothing there).
- `analysis/probes/<topic>/` standalone VC6 probe families with a README each (2026-11-19, par-3): `vecctor` (`??_H`
  vector constructor iterator expansion), `mulorder` (index multiply operand order, MapLongitude's type; `scan.py`
  tree screen), `aggreg` (4-byte aggregates in registers), `switchtail` (switch tail-merge direction), `slotshare`
  (frame slot sharing with inlined callees' locals). Rules in MATCHING_GUIDE 2.6, 3.3, 3.4, 3.7.
- `analysis/schedscan.py <unit> <addr> [--src ALT.cpp] [--gap N] [--rows]` scheduling / scratch-register
  diagnosis (MATCHING_GUIDE 3.2, rules S1..S6). Merges differing runs into windows and classifies each with
  eax/ecx/edx abstracted: `REGS` (same instructions, other scratch names; `rot+k` when one eax->ecx->edx
  rotation maps ours onto the image), `SCHED` (same instructions, other order; lists what the image places
  earlier or later), `OTHER`. Then a cursor track (rule S1): every paired allocation's offset (ours - image,
  mod 3) and where it changes; the first change bounds the source window where ours creates one register
  value more or fewer than the original.
- `analysis/sched/` the probe families and runner behind those rules (see its README).
- `analysis/webscan.py [unit ...] [--addr A,..] [--from FEALL.json] [--sweep] [-v] [--json F]` one name vs one value
  (MATCHING_GUIDE 3.3, 2.7 use_ability): per non-exact function, named locals/parameters with 2+ disjoint def-use
  webs (REUSE: SPLIT when the image keeps the webs in different slots, SHARED, REG) and hand-off copies `b = a`
  (COPY: MERGE/APART/REGCOPY, `/live` when a is redefined while b is live). `--sweep` compiles one split variant per
  web and one merge per hand-off copy and prints rows and frame (15 min tree-wide with FE_JOBS=5). The sweep does not
  check meaning: listing-based webs miss joins after tail merging, so read every web by hand before keeping one
  (first pass 2026-11-19: veh_scoot, monolith, probe, load_vehicles, draw_expenses real; base_draw, setup_text `k`,
  synch `local_turn` were broken splits that read as large gains).
