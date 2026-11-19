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
- `analysis/unitpad.py <unit> [--write] [--at ADDR] [--only a,b]` whole-unit sweep of the unit's pad block
  (`CANARY-PADS` / `HEADER-PADS`, function or extern kind) 0..63: per count, which markers of the unit are not
  fe-exact (markers never exact at any count are left out). Use it after a header batch so a pad that restores
  one canary does not flip another function of the unit. `--at ADDR` sweeps pads inserted before that marker
  instead (no block needed). `analysis/padext.py <unit> <addr[,addr]> [--write]` is the single-function form
  for an extern-kind block.
- `analysis/spellsweep.py <unit> <addr> [--write] [--rules ...]` raw-vs-helper screen of one function for the
  invented budget-only helpers (is_human, is_alive, is_alien_faction, map_tiles(), MapAbstract()): every site
  alone, all sites, then greedy; key = (named call multiset distance to the image, rows), no ratchet on rows
  when the call distance drops.
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
  (r) a field adjusted through a local copy, directly (`f = g(f)`) or through `T &r = f` (the double_cross clamp,
  MATCHING_GUIDE 3.7). The ranking key starts with the named callee difference (analysis/callees.py), so a
  variant that fixes which callee is called ranks above one that only fixes the call count.
- `analysis/callees.py <unit> <addr> [--src ALT] [--pfx N]` named call multisets: image direct calls named
  through the ORIGINAL markers (plus a CRT table learned from aligned call sequences), `call [IAT]` through the
  image's import directory, ours through relocations; prints the callees on one side only.
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
- `analysis/giantcallee.py [--giants N] [--add NAMES] [--max-size B] [--addr A,..] [--copies-only] [--json F] | --probes`
  header-inline screen (MATCHING_GUIDE 2.8, "Header inlines behind dead probes"; 18 s, no compiler): image call graph over
  every marker span; giants = the N largest spans plus --add; lists every small function whose image callers are all giants,
  its address neighbours (G giant, C candidate, H header inline) and the run of C/H copies it sits in, whether our source
  already defines it as a header inline (HDR, `// body` pointer) or in its .cpp (PLAIN), and the hand-written copies of its
  body elsewhere (parameters match any balanced expression, locals any identifier; `x` exact host, `G` giant host).
  `--probes` counts the dead budget-probe call sites in the tree. First pass 2026-11-19: is_port, is_known, rnd and the
  communicate cluster's adjective/insult forms became header inlines (the enemy cluster 0x00579770-0x00579A30 and the
  communicate cluster 0x00558FC0-0x00559221 are runs of such copies).
- `analysis/litgap.py <unit> [--src ALT] [--all] | --tree [unit ...] [--json F]` string-literal gap screen (13 s tree-wide):
  our object's .data literals in order against the image's literals over the same span; prints the image literals we lack
  (`U` = no code reference: the label of an expanded `rnd(b, "LABEL")` or another ignored inline argument) with their
  neighbours, and our literals the image lacks. Units whose literals are spread (base.cpp) anchor poorly; read their
  labels by neighbours instead. MATCHING_GUIDE 2.8.
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
- `analysis/slotflow.py <unit> <addr> [--src ALT.cpp] [--pfx N]` transcription-error screen (MATCHING_GUIDE 3.5): for every
  image instruction that reads a frame slot and is aligned with one of ours reading a named variable v, it takes the
  image's reaching definitions of that slot, maps each through the fe alignment to our instruction and the variable it
  writes, and prints `READ v image value written as w` when none of them is v (our source may read the wrong variable of a
  packed slot). Misaligned regions give false hits; read each one against the source.
- `analysis/pairscan.py <unit> <addr> [--src ALT] [--pfx N] [--sites]` frame-slot pairing by source variable: for every
  aligned row where both sides access a frame slot with the same instruction shape, the variable our /FAsc listing
  names at our slot on that line and the image slot it pairs with (`?a/b` = a temporary or packed block-local, `T-0xN`
  = unnamed slot). `--sites` lists the source lines and image addresses of each minority pairing: the places where
  the frames pack differently. Complements `slotmap.py` (slots only) and feeds reusesweep's rename candidates.
- `analysis/reusesweep.py <unit> <addr> [--write] [--iter N] | --from FEALL.json [--min 100] [--skip A,..] [--write]`
  reused function-scope variable screen (MATCHING_GUIDE 3.3 "A block-local that the image keeps in a function-scope
  variable's slot"): renames each block-local int/BOOL into each depth-1 int/BOOL (or int parameter) that is DEAD over
  the block-local's whole scope and ranks the renames by frame, slot map (|image refs - our refs| per offset), candset
  and rows. Deadness is proved by the compiler, never guessed: a variant with every depth-1 int made `volatile` (each
  source read is a slot read) is compiled with /FAsc and slot liveness is read at the scope's instructions (the unit is
  moved down 20,000 lines so inlined header lines cannot collide), then every candidate is confirmed by the kill probe:
  `T = 0x2468ACF;` at the declaration must compile to the unchanged function (dead-store removal). Packed slots,
  address-taken targets and variants that do not compile fall back to the kill probe alone. `--write` keeps a rename
  only when it improves frame or slots, or at equal frame and slots lowers rows with no worse candset.
- `analysis/uninitscan.py [unit ...] [--first] [--json F]` uninitialized-read screen (MATCHING_GUIDE 3.5, wrong variable of
  a packed frame slot; about 3.5 min tree-wide with FE_JOBS=5): compiles every optimised unit with /W4 and lists every C4701/C4700
  site (VC6 reports one per variable; the tool inserts `V = 0;` at each reported statement in a throw-away copy and recompiles
  until no new site appears), with the function's fe rows or EXACT. A read whose only defs sit in an exclusive branch while a
  sibling local set on that path shares the image slot is the base_build `l_34`/`w` shape. Sites in exact functions are original.
- `analysis/unitlits.py <unit> [--src ALT.cpp] [-q] | --who LABEL|0xVA [-n N]` a unit's literal run against the image's
  (2026-11-19, par-1): the image strings referenced from the unit's functions in address order, with the unreferenced
  ones between them (`U`: rnd labels and other ignored inline arguments), against the object's `$SG` order; prints the
  differing stretches and `LITS ... differing D`. Unlike litgap.py it anchors by owning function, so spread units
  (base.cpp) read cleanly once they are in address order. An insert/delete pair of the same strings inside one
  function is a source-order fact (base_growth, base_production, encounter: arm order / negated test, code-neutral);
  an unreferenced image string names a missing ignored argument. `--who` prints the image's run around one label with
  the functions referencing each neighbour.
- `analysis/reorder.py <unit> [--hoist] [--write] [--force]` puts a unit's annotation blocks in image address order and
  proves the move is code-neutral (reader records equal, object code sections and relocation targets equal); see its
  docstring for the block rules, the repairs it applies and what it does not check (`_$E<n>` symbol facts,
  RECOVERY_LITERAL numbers). 73 of the src/a-l, src/jackal and src/caviar units were ordered with it.
- `analysis/varslot.py <unit> <addr> <name-prefix|''>` named-local home screen (MATCHING_GUIDE 3.3 "Split only on image
  evidence"): for every source line that assigns exactly one named local, the frame slot our store uses and the slot of the
  image's aligned store. A local whose image stores land in another local's home is a candidate original reuse; `none` =
  no image store there. Misaligned regions give false hits. Prefix FE_PFX_BASE.
- `analysis/varmerge.py <unit> <addr> <start-text> v>w [...]` fes the function with local v merged into w, one variant per
  pair (text rewrite between <start-text> and the next marker; check live ranges by hand).
- `analysis/sitedeck.py <unit> <addr> lraw,lget,traw,tget [--min N] [--write]` greedy per-site toggles of the label/table
  lookup spellings (`label_get(i)` <-> `StringTable.get(Labels.strings_ptr[i])`, `table_text(x)` <-> `StringTable.get(x)`)
  inside one function; sites that share a cross-jumped tail must be flipped together (MATCHING_GUIDE 3.7).
- `analysis/offline.py <unit> <addr> [src]` offset -> source line map of our function from /FAsc.
- `analysis/info.sh <unit> <addr> <src>...` one line per whole-file variant: rows, frame, `this` register, ebx uses.
- `analysis/findpat.py ctrsub|latectr [--nonexact]` (with addsub, bytecse, negcopy) image-side signature scans over every
  marker span: `ctrsub` = the unfolded inner product of a counter subscript `A[p][k++]` (MATCHING_GUIDE 2.4), `latectr` =
  rising `mov [slot],K` stores between the pushes of calls (the post-increment's store). 2026-11-19: both hit only
  InfoWin::setup_text, tree-wide, exact functions included.
- `analysis/unitpad.py <unit> [--write] [--at ADDR] [--only a,b]` whole-unit pad sweep (45 s; FE_PFX_BASE.., FE_JOBS lanes):
  sweeps the unit's `// CANARY-PADS:` / `// HEADER-PADS: function NAME` block 0..63 and groups the counts by the set of
  markers that are not fe-exact (`ALL OTHERS EXACT` = only the never-exact ones remain). `--at ADDR` sweeps declarations
  inserted before one marker instead, so a count-sensitive function (AGENTS.md canaries) is told apart from one that
  needs a source cause: a function that no count fixes is not a canary flip. `--write` keeps the best count nearest
  the current one.
- `analysis/accsweep.py FEALL.json [unit-substr ...]` accessor respelling screen over the non-exact functions of a
  `feall.py --out` report: families T (`diplo_treaties[..] & K` -> has_treaty), A (has_agenda), N (hand-expanded
  get_noun/get_title/get_name, `analysis/nat.py`), H (is_human), L (is_alive), X all; prints rows and the named-call
  distance (analysis/cdeck.py) per family where either moves. `analysis/foldsite.py <unit> <addr>` replaces each two-way
  treaty/agenda write pair with treaty_off/treaty_on/agenda_off/agenda_on (each site alone and all, MATCHING_GUIDE 3.7
  "folded writers"); `analysis/foldunit.py <unit>` runs that greedily over a unit. Variants go to build/recovery/fe/.
- `analysis/tumap.py [--text] [--lits] [--units] [--cands [--strong]] [--inside] [--unit U]` translation-unit evidence
  map from the image alone (3 s, no compile): every unit's .text runs in address order with its flag set, the span of
  the literals only its functions reference, and `INIT n` where the run starts with static-initializer thunks (the
  CRT table's `$E` functions; VC6 emits each at its object's definition, so the first one marks a unit start);
  literal runs in .data order; units with several runs (split candidates or misplaced functions); short runs
  enclosed by another unit. Adjacency alone proves nothing (the files are already in image order): a boundary is
  shown by an INIT, by different flags, or by a small same-unit callee the image calls where /Ob2 would expand it.
  `tumap.py --merge A.cpp [B.cpp] --into C.cpp [--top] [--dry]` and `--move 0xADDR,.. --into C.cpp [--include '"x.h"']`
  do the mechanical regroup (whole annotation blocks, build lists, project.json flag sets, compile_context); see
  `analysis/tumerge.py`. Judge the result with `feall.py --all <units>` against a base and `recover check-claims`.
- `analysis/localinplace.py <unit> <addr> [--write] [--iter N] [--only NAME] [--list] [--show KEY] | --from FEALL.json [--min 100]
  [--skip A,..] [--write] [--json F]` named-copy screen (MATCHING_GUIDE 2.7 "Named copies of a field"): every
  single-assignment local initialised from a pure field/global read (`int v = Vehs[id].proto_id;`, `v = p->f;`,
  `v = Global;`; address never taken, no local of the read assigned later) is respelled both ways: `ALL:v` every use reads
  the field in place and the local goes, `S:v@Lnnn` one use in place, `R:v@Lnnn` / `RALL:v` a later textual repeat of the
  same read spelled through the local. Ranking key: named callee difference, frame, candset count, slot map, rows; the
  `image` column says whether the image's aligned rows store a slot or define a callee-saved register at the copy.
  Statements naming both the copy and the read (saved against live value) are never touched; an automatic keep never
  raises the rows and a rows-only keep needs 3 rows, because an in-place read after the field changed is another program.
- `analysis/copycensus.py [unit ...] [--pfx N] [--list] [--json F] [--all]` census of those copies over the exact
  functions (ours = image there): home of the copy (callee-saved register, frame slot, neither) by use count and by
  whether a call lies between the copy and its last use. 4 minutes tree-wide on one prefix.
- `analysis/regalloc/c_field.spec` (`python3 cand.py c_field.spec`) the probes behind that guide section.
