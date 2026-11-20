# Fast matching tools (2026-11-18)

Screens only; confirm with `uv run recover measure` and bank with `uv run recover record`.
Run from the repo root with `uv run python .opensmacx/tools/<tool>`. They use wine prefixes
worker-21..34 (the queue uses 20) and never touch the worktree's build.

Lanes: every tool takes its default wine prefix from `FE_PFX_BASE` (fe.py's `--pfx` included; the
parallel tools use `FE_PFX_BASE..FE_PFX_BASE+FE_JOBS-1`). Export `FE_PFX_BASE` and `FE_JOBS` once per worker and
an explicit `--pfx` is only needed to pick another lane.

- `fe.py <unit> <addr[,addr]> [--src ALT.cpp] [--all] [--ctx N] [--win lo-hi,...] [--quiet]`
  Raw-compiles one unit (0.3-2 s), diffs the function's COMDAT against the image span with
  relocations masked. `!!` = differing row, `!>` = same instruction, different local jump target.
  A marker with `// compile_context <tu>` is measured in that unit's compile, as `recover measure`
  does (unless `--src` replaces the host).
- `feall.py [--all] [--base FILE] [--out FILE] [--root CHECKOUT]` whole-tree pass: 420 non-exact functions in 13 s,
  all 5,574 markers in 17 s (`--all`). `--base` prints LOST/GAINED/MOVED: a header hypothesis
  can be tested tree-wide in under 20 s. `--root` measures another checkout with that checkout's own fe.py (its
  sources and headers; before 2026-11-19 it was parsed and ignored). About 46 claimed functions read inexact (measured in
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
- `analysis/declpad.py top|report|syms|at|who|padfree|common ...` symbol-number ties (MATCHING_GUIDE 2.5 "Bucket order of
  referenced symbols"). `top OUT.jsonl [unit ...]` puts 0..63 prototypes before the first line of each unit and records every
  marker that moves (64 compiles per unit, about 2 h tree-wide on 5 lanes; resumable); `report OUT.jsonl [--max N] [--delta]`
  lists them with their exact pad counts. A function that moves at no n has no such tie. `syms <unit> <addr>` prints the
  front-end symbol number of every global the function references (probe literals in a shadow copy of src/, as outside.py);
  `at <unit> <addr[,addr]> [--full] [--only NAME,..] [--extra file:line,..]` sweeps pads before and after each of those
  declarations and bisects to the ones the tie follows (`DECLPAD_STEP=2` halves the compiles); `who OUT.jsonl WHO.jsonl` runs
  it for every moving function. `padfree PF.jsonl [unit ...]` is the top sweep with the unit's CANARY-PADS / HEADER-PADS blocks
  emptied and `common OUT.jsonl PF.jsonl` asks whether one shift serves every unit (2026-11-19: no).
- `analysis/spellsweep.py <unit> <addr> [--write] [--rules ...]` raw-vs-helper screen of one function for the
  invented budget-only helpers (is_human, is_alive, is_alien_faction, map_tiles(), MapAbstract()): every site
  alone, all sites, then greedy; key = (named call multiset distance to the image, rows), no ratchet on rows
  when the call distance drops.
- `analysis/accspell.py list|deck|census` raw field tests against header accessors (has_treaty, has_agenda,
  is_human/is_alive incl. local `1 << f` bits, get_triad, get_proto_triad, is_missile, offense/defense ratings,
  map.h `*_at`, is_ocean). `list --rungs R.json` sites of every rung-1/2 function with the return they follow;
  `deck <unit> <addr> [--write VARIANT] [--no-im]` BASE/ALL/ALL:family/AFTER_R<k>/ONLY_R<k>/FROM_S<k> keyed by
  (EH distance, named call distance, inlinemap site distance, frame, rows); `census` raw/accessor counts per unit
  over the fe-exact functions. MATCHING_GUIDE 2.8.
- `analysis/ceilscan.py [unit ...] [--all] [--addr A --bisect]` which functions hit the /Ob2 total-size
  ceiling: a free probe at the top and end of each function (`Ec` = ceiling-bound), `--bisect` spreads
  `CEIL_PROBES` probes to locate the stop. One compile per unit, 18 s tree-wide. MATCHING_GUIDE 2.8 rule 7.
- `analysis/inline/giant.py N STRIDE [if|store|call] [--charged] [--mixed] [--front]` synthetic giant caller:
  expansions against caller size (the ceiling T, MATCHING_GUIDE 2.8 rule 7).
- `analysis/labelscan.py [--from FEALL.json] [unit:addr ...] [--moved] [--json F]` label-placement screen
  (MATCHING_GUIDE 3.7 "Label block placement"): per goto label, the last reaching site, its claiming form, the
  predicted placement and the source line laid before the label block in ours and in the image (found through the
  image targets of our goto jumps); `MOVE` = different predecessor. 2 min over the non-exact set; the predecessor line
  is the listing's attribution, so a tail-duplicated return reads as the shared return's line.
- `analysis/tailcopyscan.py <unit> <addr[,..]>|--all [--list]` / `--tree [unit ..] [--from FEALL.json] [--json F] [--list]`
  join-copy screen (MATCHING_GUIDE 3.7 "Ours merges a tail that the image keeps"): identical runs ending in `jmp T`
  (or falling into T) that one side copies into a jumping arm and the other keeps joined, with the copy's bytes and
  the joining side's block bytes (from the label its arms jump to up to its `jmp`); `SIZE` = the joining side's block
  is over 15 bytes and the copy is not, with the copy as the joining block's tail. 70 s over the non-exact set.
  2026-11-20: 16 functions, 3 SIZE rows (menu_proc, pulldown_update, communicate), all known or register noise; the
  image's prefix copies (editor, enemy_planet_buster, MATCHING_GUIDE 3.7) read `tail differs`.
- `objdis.py <unit> <src> <addr>` raw listing of our function with real jump targets.
- `objdis.py <unit> <src> <addr>` raw listing of our function with real jump targets (unit as `picktech`, `picktech.cpp`,
  `src/picktech.cpp`, or a basename found in a subdirectory; lane FE_PFX_BASE).
- `analysis/litaddr.py FEALL.json [--shared] [--json F]` literal-address evidence (MATCHING_GUIDE 2.8 "A pushed literal's
  address decides inline against written out"): per function, the separator literals the image pushes (shared header-inline
  COMDAT `S` or unit-local copy `L`, in code order) against ours (`??_C@` S, `$SG` L, named stand-in N). `--shared` adds
  every other text the image references from two units. 20 s tree-wide on `feall.py --all --out`.
- `analysis/sdeck.py` (`sdeck.run(unit, addr, lo, hi, [(key, text), ...])`) structural deck runner: ranks whole-file
  variants by (EH unwind-depth edit distance, named call multiset distance, jcc/jmp/call/ret count distance,
  control-flow hunks, fe rows), so a
  spelling is judged by its /Ob2 and block decisions before its rows (MATCHING_GUIDE 2.8 "Judge by decisions").
  `sdeck.WATCH = ['remove_all', ...]` adds per-callee image/ours counts to the key.
- `analysis/orsweep.py <unit.cpp> <addr> [--and] [--write KEY]` / `analysis/orsweep2.py <unit.cpp> <addr> [--write KEY]
  [--list]` value-form sweeps (MATCHING_GUIDE 3.7 "Flag tests: threaded or materialised"): orsweep turns each `if (A || B)`
  (with --and also `&&`) alone into `if ((A || B) != 0)`; orsweep2 adds loop conditions with a top-level `&&`/`||`
  (`(cond) != 0`), `if (!(A && B))` -> `if ((A && B) == 0)` and nested else-less ifs merged into `if ((A && B) != 0)`.
  Ranked by sdeck's structural key against BASE; --write applies a variant.
- `analysis/jseq.py <unit> <src> <addr> [<lo> <hi>]` control-flow token diff (jcc/jmp/call/ret) of one function:
  both token counts and the first `SHOW` differing hunks. Equal counts and 0 hunks = rung 4 settled.
- `analysis/ucmp.py <unit.cpp> ...` per-marker fe rows of HEAD's version of each unit against the worktree's:
  the markers a body edit moved in its own unit, before committing (about 1 s per unit).
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
- `analysis/constfold.py [unit ..] [--all|--exact] [--json F]` runtime constant arithmetic on both sides
  (MATCHING_GUIDE 3.4): straight-line `op r,K2` (and `lea r,[s+K]`) whose register input is a known constant from
  `mov r,K`, `xor r,r` or a register copy; image pairs marked `ours-folded` when ours uses the folded value as an
  immediate. 70 s tree-wide with --all on 5 lanes; 2026-11-19: twelve image pairs in ten functions, one unexplained (NetWin::exec).
- `analysis/selscan.py [unit ..] [--all] [--kinds sel,negsbb,cmpsbb,set,abs,babs,mm,neg] [--json F]` branchless
  select/bool/abs/min-max/negation idioms whose multiset differs between the image and ours (MATCHING_GUIDE 3.4
  "Ternary setcc forms"): `setcc; dec; and K; add B` keyed by (K, B) with the condition compared separately (`COND`),
  `neg; sbb; <op>` and `cmp; sbb` with their tails, lone `setcc` with the op that follows (inc/add/sub/lea), `cdq;
  xor; sub` against `test; jge; neg`, `cmp; jcc; mov` min/max and lone `neg`. 6 s tree-wide on 5 lanes; most rows
  are scheduling or register noise, read each with `fe.py --all` before editing. 2026-11-20: 8 fixes (order_veh,
  base_terraform x2, air_power, draw_supported, enemy_move, right_menu, draw_nutrients).
- `analysis/widthscan.py [unit ..] [--json F] [--min N]` byte/word-register arithmetic on one side against the same
  mnemonic on a full register on the other, inside fe's differing blocks (MATCHING_GUIDE 3.4 "Narrow locals").
  40 s tree-wide; most rows are register-allocation noise, the clean ones are a bool/uint8_t local the image keeps
  as int. A per-local retype sweep (every char/short/bool local of a non-exact function to int) finds the same
  cases more directly.
- `analysis/aggscan.py <unit> <addr[,..]> | --from FEALL.json [--json F] [--stores] [--sites]` mixed member/pointer
  aggregate screen (MATCHING_GUIDE 3.3 "VC6 never forwards a stored value between a member access and an access
  through a pointer"): per straight-line block, stores to `[R+K]` overwritten with no read between (`dead`) and reads
  of a field stored earlier in the block (`rel/n` when only same-base stores lie between), image against ours by
  displacement; `--stores` ranks image-only store displacements instead. `AGG_EBB=1` keeps state across a
  conditional jump's fall-through, `AGG_FRAME=1` adds frame slots (local aggregates; spill homes are noise);
  the JSON also carries `lea_img`/`lea_our`, member addresses (`lea r,[R+K]`) formed more often on one side.
  1 min over the non-exact set. 2026-11-20: on the pre-fix PickWin::init it lists the three cursor reloads; tree-wide
  the block screen lists only labs_line and draw_projects (register noise), `--stores` adds ReportIf::draw_ops (the
  top store, an all-member spelling flips a register tie, 105 rows) and otherwise CSE/scheduling differences.
- `analysis/rectscan.py <unit> <addr[,..]> | --from FEALL.json [--json F] [--sites]` rect-helper argument-order screen
  (PickWin::init's set_rect): per straight-line block, groups of four dword stores to K, K+4, K+8, K+0xC (any order,
  within 40 instructions, frame slots included), each with its shape (`C` consecutive / `I` interleaved, plus the field
  store order) and the memory loads of its window in code order; image and our groups are paired by (member K | frame)
  in code order and flagged `ORDER` (same loads, other order), `LOADS` (other loads) or `SHAPE`. 55 s over the non-exact
  set. 2026-11-20: 45 functions flagged, most rows scheduling or constant-register noise; read with `fe.py --all`.
  It led to draw_score (body rect reads the panel edges), draw_ops/draw_projects (an inlined same-unit setter written
  out) and draw_map (|dy| form).
- `analysis/addnegscan.py <unit> <addr[,..]> | --from FEALL.json [--json F]` `add r, -K` against `sub r, K` census: VC6
  writes `x - K` as `sub`, but as `add r, -K` when the same `x - K` is CSE'd from two uses (draw_score's
  `right - 3` in two rects) or reaches the code through an inline parameter (expand_rect). Lists functions whose counts
  move between the two forms. 2026-11-20: 5 functions, NewTechWin::exec and DiploWin::exec the only rect cascades.
- `analysis/aliasscan.py <unit> <addr[,..]> | --from FEALL.json [--json F]` "scalar that was an array element"
  screen (MATCHING_GUIDE 3.3, colortables_init): per natural loop, `imul r,[ebp-N]` and reads of ebp slots the
  loop never writes or address-takes, on both sides, plus the aligned rows where the image reads such a slot and
  ours (same mnemonic) has a register. 30 s tree-wide. 2026-11-20: 22 functions with such pairs; on reading,
  the ones in small functions (set_time_controls, repair_phase, battle_compute) are register pressure or
  operand order, not aliasing. Screen only.
- `analysis/inlparamscan.py <unit> <addr[,..]> | --from FEALL.json [--json F] [--sites]` inline-parameter home screen
  (MATCHING_GUIDE 3.3 "A never-reloaded store ... is an inline function's parameter", draw_expenses): flow-sensitive
  dead frame stores on both sides (`used` = the stored register is read again, `imm`), plus `tern` (`mov [m],K; jcc L;
  mov [m],r; L:`, a select materialised in its home); image events are paired with ours through fe's row alignment
  (IP_WIN rows) and the unpaired ones listed with our nearest source line (header line for an expanded inline).
  Address-taken frame aggregates count as read by calls over IP_AGG bytes (default 0x10); functions without an ebp
  frame are skipped. 75 s over the non-exact set. Validated on the pre-fix draw_expenses (the three tile.top homes,
  the two max homes). 2026-11-20: 24 functions flagged; the clean rows were not inline parameters but local RECT copies
  whose overwritten fields the image still stores (DiploPop::init rect.left, AlphaSave::init frame.top/bottom,
  DiploWin::exec row/rc), the draw_expenses nested-max ternary temporary (-0x24) and NetDaemon::synch's 0x8D loop
  IVs; RECT pointer-copy loops (NewTechWin::exec) read as dead stores (noise).
- `analysis/ivscan.py <unit> <addr[,..]> | --tree [unit ..] [--all] [--json F]` induction-step census of both sides
  (MATCHING_GUIDE 2.7 "Strength reduction is a candidate-set question"): every `add/sub/lea/inc/dec` of a register (or
  `add dword ptr [slot], K`) inside a natural loop, at any stride and sign, whose register is used as an address or
  written back to a home that is, keyed (sign, stride); prints the strides one side steps and the other does not, with
  image addresses and our source lines. Steps in latch blocks only by default (`IV_ANYBLOCK=1` counts every loop
  block); the two filters disagree on loop-rotation and value adds, so trust a stride that both report. 75 s
  tree-wide with --all on 5 lanes; validated: no BYTE_EXACT function reports a difference. 2026-11-19: after
  setup_game no second clean "image steps a struct pointer ours recomputes" case remains outside NetDaemon::synch.
- `analysis/candset.py <unit> <addr> [--src ALT.cpp] [--min N] [--quiet]` the register candidates VC6 actually
  formed, read from the code of both sides: every callee-saved web (reaching definitions over the CFG) named by
  its role (`this`, `lea:this+K`, `[slot]{name}` piece, `#K`, `ret@line`), scored by rule 4/4b at instruction
  level and checked against its own side's registers; prints the roles on one side only and the pairs the sides
  order differently (MATCHING_GUIDE 2.7 "Candidate set"). `regprio.py ... --candset` appends the same report.
  A "constants whose formation differs" block lists, per constant and side, stores (weight 2^depth, scratch pieces
  included), memory compares (`mov r,[m]; test r,r` too) and pushes, flagging a register held on one side with a
  store weight under 4 (2.7 Z9: a merged store is hidden there).
  `candset.py --census [unit ..] [--oy] --opts k=v,.. --opts ..` sums the image-side rank constraints explained over
  the fe-exact functions per option set (70 s tree-wide; 4j `bytepref` validated this way).
  `CANDSET_SITES=K candset.py ...` prints every store of constant K counted in that block (image first, then ours):
  diff the two lists to find the extra store (make_gift's `gift_value = 0` before the mode-7 test).
  Since 2026-11-20 (G1-G3, Z10) the rank model puts call-crossing webs above call-free ones (`callfirst`), orders two
  call-free webs by raw references then range length (`cfrefs`), and the census skips `__asm` bodies (`skipasm`):
  9313 -> 9578 of 11402 constraints over 5059 functions (10066/13126 with the asm bodies and the old model). The census
  also prints the G1/G3 check (call-free callee-saved webs with every scratch register busy over their range,
  3733/4287) and the Z10 check (split zero webs with every register taken between the pieces, 29/45); the per-function
  report lists the call-free callee-saved webs that had a scratch register free.
- `analysis/ctrscan.py <unit> <addr[,..]> [--src ALT] | --tree [unit ..] [--from FEALL.json] [--all] [--json F]` loop-counter
  and hoisted-global screen (MATCHING_GUIDE 2.7 G1-G3; 2 min tree-wide with FE_JOBS=5): pairs the natural loops of both
  sides through the fe alignment, classifies each counter (`S:`/`C:` scratch or callee-saved, `own` init, `zero` = the
  initial `xor` also feeds zero stores before the loop, `shared`, `M` memory), lists globals loaded before the loop and
  read in it (hoisted) and, for call-free loops, the scratch registers live across the back edge with their defining
  instruction and our source line (the values that keep a scratch register busy). Flags loops whose counter class or
  hoist count differs; validated with `--all`: no BYTE_EXACT function flags. Loops whose sizes differ much are paired
  wrongly (a different loop structure): read only pairs with similar `n`.
  `CTR_GREADS=1` also flags paired loops (sizes within 1/8) whose per-use global reads differ, with the
  in-loop non-frame store counts of both sides (`greads I/O stores I/O`): the aliasing-blocks-hoist screen
  (2026-11-20: 27 loops in 17 functions; on reading, pressure or strength reduction, not aliasing).
- `analysis/rotscan.py <unit> <addr[,..]> [--src ALT] [--track] | --tree [unit ..] [--from FEALL.json] [--all] [--json F]`
  scratch-rotation scan (MATCHING_GUIDE 3.2 S1/S7, 3.7 "Cross-jumping ground truth"; 40 s tree-wide with FE_JOBS=5):
  aligns both listings with eax/ecx/edx abstracted, pairs every scratch load (return-value `mov eax, K` excluded) and
  reports per function the FIRST statement where the phase (ours - image, mod 3) leaves 0 and stays off (DIVERGE, with
  image address, our /FAsc line, the window since the last in-sync load and the loads present on one side only), or
  EQUAL. `first disagreement earlier` is the first load the sides disagree on at all (a reuse such as `mov eax,[eax+8]`
  against a fresh register). Each `jmp` on one side only is printed with the rotation between the copy and its host
  on the side without the merge (`rot+k`, loads apart). `callee-saved:` (every function) aligns with callee-saved
  registers and frame slots abstracted to one token and pairs their whole-operand occupants: the first place image and
  ours hold one value in different callee-saved registers or one in a register and the other in a frame slot (`slot`),
  with the instruction that set each side's register, the occupant map (image>ours counts) and the next regions. Tree
  pass 2026-11-19: 70 of the 199 phase-equal functions differ here first; aim `schedscan --try --near` at that line
  (named values move rule-4 priorities too) or read regmap.py/candset.py there. Validated: no BYTE_EXACT function diverges. Aim
  `schedscan.py <unit> <addr> --try --near A-B` at the window it names (2026-11-19: drone_riot 317 -> 119,
  veh_draw 165 -> 59, name_proto 196 -> 162, time_warp 675 -> 660, DesignWin::init 107 -> 84).
- `analysis/mergescan.py <unit> <addr[,addr]> [--src ALT.cpp] | --from FEALL.json [--json F]` tail-merged statements
  (MATCHING_GUIDE 2.7 "Tail-merged references count"; 50 s tree-wide with FE_JOBS=5): our source lines whose call left
  no `call` instruction while the callee is called elsewhere in the function (`MERGED`, with the identifiers of its
  arguments and the lines that kept a copy), and every `goto`. Register allocation runs before tail merging, so each
  merged copy still counts for its operands: compare with candset.py's disputed pairs.
- `analysis/flagscan.py list [unit ...] [--all]` / `deck <unit> <addr> [--var V] [--write]` MATCHING_GUIDE 3.7 Layout
  "a flag skipping a shared tail is a goto": lists bool/int locals set to one constant at some sites and tested once by an
  else-less `if` (form skip/enter; flow=ok when every set site falls straight, through if/else closings or a `break`, into
  the test, so `goto <v>_past;` preserves behaviour); `deck` scores that rewrite with sdeck against the base. Tree-wide
  2026-11-20: 33 flags, 3 flow=ok, none moves (good_sensor is already threaded by VC6); the other leads are flags the image
  keeps (shoot tests `[ebp-0x10]`) or slot/param-home differences (crash_landing keeps tile_y in `[ebp+8]`).
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
- `analysis/rungs.py [unit ...] [--addr A,..] [--out DIR] [--md] [--top K]` rung-ladder census (75 s tree-wide with
  FE_JOBS=5): for every function that is not fe-exact, the first differing rung in the AGENTS.md order (1 EH FuncInfo,
  2 named call multiset, 3 frame size/kind, 4 block count / return tails / block-order signature / prologue-save class,
  5 frame-slot map, 6 registers, 7 bytes), composed from ehcensus, callees, framescan and prosave over one compile per
  unit. Writes `build/recovery/rungs/rungs.txt` (counts per rung, the table sorted by rung then rows, then the same for
  the count-sensitive canaries and the broken claims) and `rungs.json` (every rung's image/our values per function);
  `--md` prints the summary kept in MATCHING_GUIDE 4.0. This table is the work list; use the per-rung tool on one
  function for the detail.
  Rung 4 is split by cause in the `4x` column (`layout.sub`): a return tails, b block count (`b:jcc`, `b:jmp`, `b:mix`,
  `b:tgt` = a jump lands inside a block, `b:sw` jump tables), c order only, p prologue-save class, `+d` loop set (headers,
  back edges by dominance, `jmp`-closed back edges, entry by a jump to the test), `+r` loop-head reload skips only; each
  line ends with the first block where the block sequences stop aligning (image address, our offset, our source line).
- `analysis/probes/looptest/run.py [name ...]` loop-test probes (guard + bottom test, top test with `jmp` back, no guard)
  and `analysis/probes/switchlower/run.py [name ...]` switch lowering by case count and span (subtract chain, direct
  table, byte index table, tree): MATCHING_GUIDE 3.7 "Loop tests" and "Switch lowering".
- `analysis/callees.py <unit> <addr> [--src ALT] [--pfx N]` named call multisets: image direct calls named
  through the ORIGINAL markers (plus a CRT table learned from aligned call sequences), `call [IAT]` through the
  image's import directory, ours through relocations; prints the callees on one side only.
- `analysis/xjcalls.py <unit> <addr[,addr]> [--src ALT] [--all-callees] | --rungs [RUNGS.json] [--only 2] [--json F]`
  cross-jump screen for rung-2 call-count gaps (MATCHING_GUIDE 3.7 "A written-out copy of a shared exit"): for each
  callee whose count differs (callees.py naming), every call site on both sides with the jumps landing up to 12
  instructions before the call (`j@-k` jmp, `c@-k` jcc), jumps landing after it in the same run, its continuation, and
  `lines img/ours` per-source-line site counts. Verdict IMG-SHARES (image has fewer sites and more jmp-ins: our copies
  may want a goto), OURS-SHARES (the reverse), SITES (no jump-in difference: inlining/teardown). `--rungs` takes
  every rung-2 function of rungs.json (10 s on 5 lanes).
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
- `analysis/inline/predict.py <unit> <addr[,..]> [--show] [--leads] | --tree [unit ..]` rule-8 /Ob2 predictor (MATCHING_GUIDE
  2.8 rule 8; 2026-11-20 par-4). Site tree from a /Od /FAs compile of the unit (every call site in source order, one
  destructor call per scope exit, no inlining or cross-jumping); each inline callee's charge measured in the unit's
  own compile by probe callers at the B0 floor (`__cprN(1); <call>;`, binary search over N, a destructor through a
  local object's closing teardown, since an explicit `o->T::~T()` is never expanded; cached per unit text); the
  depth-first walk of rule 8 with S fitted to ours' named call counts, then the image target as ours' fit moved by
  image - ours (so cross-jumped call counts cancel). `--show` prints the walk with each site's own charge, subtree
  spend and child budget; `--leads` tries one site charge +-0.5..4 or one more/fewer later site per body and lists
  the changes that bring the model to the image. 5 s per unit. State: fits ours exactly (distance 0) on most
  functions without destructor teardowns (base.cpp/veh.cpp: 12 of 19 non-exact), not on teardown-heavy giants
  (terraform 80, probe 359): the ~Popup/~BasePop teardowns are cross-jumped into shared tails whose call counts the
  optional suffix-trie merge (`PREDICT_MERGE=1`) over-merges, and ~BasePop (a base-class destructor) expands where
  rule 8 refuses the ~Scroll member before it (`PREDICT_BASE=k` scales the base destructor's charge; neither fixed
  it). Over the 22 rung-2 functions it gave one lead (editor_edit_veh: ~StringList's two sites cheaper by 4) and
  S-only shifts for battle_fight (+1..5) and base_build (-2.5, ceiling-bound, rule 7 not modelled). Screen only.
- `analysis/inlineclass.py [--out F] [--base F] [--fe FEALL.json] [--callee NAME] [--any] [-v] [unit ...]` header-inline
  cost-class census (55 s tree-wide): per callee with a body in a header, the functions whose image call count
  differs from ours (`E+` ours expands where the image calls, `c+` the reverse), exact functions marked `x`;
  `--base` lists the site-units a header hypothesis moved closer to or further from the image. Counts are taken
  after tail merging and an expanded destructor shows as its inner `close` calls, so read one function's sites
  with callseq.py / inlinemap.py. MATCHING_GUIDE 2.8 "Header-inline cost classes".
- `analysis/jointsolve.py eval|copies|descend` joint solve of the header-inline body spellings. The objective is the
  image's per-function call count of every header-inline callee over the whole tree, measured WITHOUT the local
  function-pointer stand-ins (`T (__cdecl *const x)(..) = callee;` deleted, `x(` spelled `callee(` in a scratch copy
  of the unit), so a body is judged against the image and not against its own compensation. `eval [--base F]
  [--set name=rung,.. --deck D.py]` one pass (60-80 s: exact set as written, decision vector bare; lists what moved
  and the exact functions lost beyond the count canaries); `copies D.py` which spellings keep the out-of-line copy
  fe-exact; `descend D.py [--pairs]` coordinate descent over the copy-exact spellings (a move is kept when site-units
  drop, no (function, callee) pair gets worse and no exact caller beyond the canaries breaks). The deck is a Python
  file: `INLINES = {name: dict(file, head=<regex of the definition line>, rungs=[(tag, body), ...], feasible=[...])}`.
  MATCHING_GUIDE 2.8 "Joint solve of the header-inline bodies".
- `analysis/probesweep.py [--write] [--each] [unit ...]` dead budget-probe retirement screen (2.5 min tree-wide):
  each probe-carrying function without its `*_budget_probe*` call lines, by (named call difference, rows) and the
  unit's exact markers it breaks; `--write` applies removals that keep all three.
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
- `analysis/imgtypes.py [--field SUBSTR] [--uses] [--json F] [--refresh]` image-side type census (35 s; the class
  layouts come from one /Z7 compile of every unit, cached in build/recovery/fe/imgtypes_layouts.json): reads the IMAGE
  alone, so it covers non-exact functions. Every memory operand whose displacement lies inside a source-bound global
  (RECOVERY_OBJECT; extent from our link map, a 4-byte `T *p` told from `T a[]` by its size) is named through the
  decorated type and the CodeView layout (struct arrays: offset modulo the element size), and its access form is
  counted (`movsx/movzx b|w`, plain `ld`, `ldzx` for xor+mov or mov+`and 0xff`, `st`, memory-operand arithmetic, and S/U
  for dword loads through typescan's follower). Flags: OVER (operand wider than the declared field), NARROW (extension,
  store, compare or inc/dec through a narrower operand; a narrowed mask test is normal and not counted), SIGN, INNER
  (non-mask access inside a scalar), NOLEAF. Ranked by non-exact functions with a contradicting access.
- `analysis/accdiff.py [unit ...] [--all] [--json F]` access-form differences in the non-exact functions (60 s): inside
  each differing block, image and our instructions that name the same operand (same address expression, a global by
  its relocation symbol) but differ in extension, operand width or the signedness of the following jcc, keyed by
  operand and ranked by the number of functions. A wrongly declared field is one key in many functions; a local cast
  is one function. (2026-11-19: 91 sites in 40 functions, no key above 4 functions once register-only rows are read
  out: field types are not a tree-wide lever any more.)
- `analysis/retscan.py [--all]` return-type census from the image alone (5 s): how each caller consumes eax/ax/al
  after a direct call, against the callee's decorated return type (2026-11-19: 2,636 callees, no contradiction).
- `analysis/retype.py TYPESCAN.json FEALL_BASE.json [--only C::f,..] [--skip FILE] [--dry]` one-field retype runner:
  for each 32-bit field declared unsigned with no unsigned use in the image (typescan). Evidence is a signed use in
  the image, or a load-bearing cast (dropping the int casts while the field stays unsigned moves some function; a cast
  that changes nothing says nothing about the type). With evidence it retypes the declaration to `int`, drops the int
  casts on that field in the units that use it, runs feall on every unit that names the field and commits when
  nothing fails to compile and no user of the field is lost or worse; the other functions that move (a cast dropped
  from a header inline shifts every includer's symbol counter) are listed in the commit. 80 s per field.
- `analysis/addrhome.py [unit ...] [--addr A,..] [--from FEALL.json] [--all] [--min-reloads N] [--json F] [-v]`
  homed-address screen (50 s tree-wide, MATCHING_GUIDE 2.7 "Homed addresses"): frame slots holding an address or a
  scaled offset on either side (kind of definition, how the reloads are used), paired through the fe alignment:
  IMG-PTR / OURS-PTR / IMG-OFF / OURS-OFF / KIND. These slot rows are noisy and none was a source pointer; act on
  the `UNFOLD` rows (the image reaches a member's parts through `lea r,[b+Kt]` + `[r+d]`, ours folds `[b+Kt+d]`:
  an expanded inline member, a struct copy plus a pointer inline, or a temporary that lost its register) and the
  `NULLADJ` rows (a null-tested base cast on one side only).
- `analysis/spellsweep.py ... --rules rw_fn,rh_fn,rw_raw,rh_raw` RECT extents, raw subtraction against
  `rect_width(&r)` / `rect_height(&r)` (not in the default rule set).
- `analysis/datarun.py [--all] [--lo A --hi B]` data-order evidence (MATCHING_GUIDE 2.5): ranks every bound object by
  its owner file's link position and lists the objects outside the longest non-decreasing run of each data section
  (definitions that sit in another unit's .data/.bss contribution), with the units bounding them.
- `analysis/datawho.py [--cmds]` names the unit that defines each out-of-order object of datarun.py (H thunk interval,
  D `.data` neighbours, S `.bss` neighbours plus a reference, N the one referencing unit between differing neighbours,
  ? undecided) and lists in-order objects that sit inside another unit's thunk interval; `--cmds` prints the objmove.py
  commands.
- `analysis/objmove.py --into DEST.cpp (--range LO-HI[,..] | --addr A,B) [--skip A] [--dry]` moves the bound object
  definitions (RECOVERY_OBJECT) in an address range to the unit whose data run holds them: cuts each definition piece from
  its host (an `extern` stays when no header declares the name), puts them after DEST's includes, initialised objects
  first, in image order, and drops DEST's local externs of the moved names. No body is edited; check with feall on
  both sides, `recover check-claims` and `recover rebind-literals --unit` (a moved `char *p = "lit"` renumbers).
- `analysis/sharedlits.py` every `.data` text referenced by functions of two or more units, with the single-unit
  literal before (A) and after (B) it and whether A / B reference it. A real shared text sits at the end of the first
  referencing unit's data (a COMDAT literal of a header inline, MATCHING_GUIDE 2.5); most other rows are data words
  that happen to read as text.
- `analysis/litptr.py <unit.cpp> [--write]` turns `char T[] = "x"; char *p = T;` pairs into `char *p = "x";` where the
  image proves the literal form (the cell holds the text's address, the bytes match, no other `.text` immediate or
  `.data` cell holds that address), replacing the text's RECOVERY_OBJECT by a RECOVERY_LITERAL with a placeholder
  `$SG9000n` label: run `recover rebind-literals --unit <tu> --write` next.
- `analysis/initorder.py` the hard form of that evidence (2 s, no compile): the objects built by the CRT table's
  initializer thunks, per unit in `.text` order; a unit whose objects start below an earlier unit's maximum shares a
  unit with it (alpha + interface_fonts + newgame, the only violation in the image).
- `analysis/ctxscan.py [--pfx N] [--max-run K]` emitting-unit screen for stray markers: for each marker in a short run
  between other units' runs, compiles the preceding and following units and says which emit the symbol and read exact
  (`compile_context` of an out-of-line header-inline copy = the unit whose object keeps it in the image).
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
- `analysis/xjhostscan.py [unit ...] [--from TAILSCAN.json] [--json F]` cross-jump host screen (MATCHING_GUIDE 3.7
  "Cross-jump host", "Host orientation"): reads tailscan's HOST joins and classifies each by where the image's and our
  host copies sit among the predecessors in our source order: `ORDER` (a middle copy hosts on one side: by the last-copy
  rule the image's host was last in the original, move it after the others), `ORIENT-LAST` / `ORIENT-FIRST` (one side
  hosts the first copy, the other the last: an orientation flip from a construct elsewhere in the function, not an arm
  order; see the guide). 50 s tree-wide (tailscan's cost). tailscan's tree pass now skips only the markers with a
  `// compile_context` line instead of their whole unit (veh.cpp, buffer.cpp, net_class.cpp were missing: 251 -> 328
  functions).
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
- `analysis/orphanowner.py census|scan|show` real owners from the image (MATCHING_GUIDE 2.8 "Orphan owners"): every
  marker with no image reference at all (no rel32 call/jmp/jcc, no absolute dword in any section) of >= 8
  instructions (667 on 2026-11-19) was only used where /Ob2 expanded it. `scan [unit ..] [--open] [--lines]
  [--exact-hosts] [--any-unit]` searches the image bodies of the orphan's unit for a window with the orphan's
  instruction shape (registers, frame slots and argument loads masked; callees and globals named), `show <orphan>
  <host>` prints the aligned rows. A hit outranks ownerhunt's invented owners: replace the written-out block (or the
  invented twin) with a call to the orphan. First pass (par-6): DesignWin::on_iface_right_click exact,
  DiploWin::exec 279 -> 227, battle_fight 2409 -> 2346, base_terraform 893 -> 873, the StatusWin::redraw inline and
  MEASURED stand-ins and the terraform_arid_count/emit_help twins retired, eight neutral re-spellings.
- `analysis/capprobe.py <unit> <0xADDR> '<call>' [--decl TEXT] [--src ALT.cpp]` per-callee auto-inline cap probe
  (MATCHING_GUIDE 2.8 "The per-callee cap counts IL nodes"): appends a one-statement caller and prints CALLED (over
  the cap) or EXPANDED, plus the callee's own fe result in that compile. `analysis/capscan.py [--max-callers N]
  [--all] [unit ...]` does it for every orphan (or marker with at most N rel32 references) of every /Ob2 unit, one
  compile per unit (35 s tree-wide); join its CALLED rows with `orphanowner.py scan --exact-hosts --callers N`.
- `analysis/rscan.py <host> [--min N] [--thr R]` reverse owner scan (MATCHING_GUIDE 2.8 "Referenced owners too"):
  every marker's image body, referenced or not, searched inside ONE host's image body (score*prec >= R, default
  0.9; min N tokens, default 4), so header inlines with an out-of-line copy show up too (orphanowner skips them).
  Compare each invented helper of the host with the hits (go_to's refusals were veh_battle_msg 0x0050B9C0,
  BaseWin's recompute twin was focus()). About 14 s per host.
- `analysis/helperscan.py [NAME ..] [--thr R] [--top K] [--json F]` the same scan for every invented helper in the
  tree (a definition with no ORIGINAL marker under a WORKAROUND(inline|decl) tag, budget probes left out): its
  hosts (marked bodies that call it) and each host's rscan hits. `HS_LIST=1` only lists the helpers.
- `analysis/ownerhunt.py <unit> <addr> [--lines A-B] [--maxlen N] [--limit N] [--moved] [--vec] [--fast] [--repeat]
  [--write K]` missing inline OWNER search (MATCHING_GUIDE 2.8 "Missing owner"): parses the function body into
  statements, wraps every liftable contiguous sibling range (a loop, an if-arm, a run of statements; no return/goto/
  label, break/continue only inside their loop, no local that outlives the range) into a file-local `static inline
  void oh_owner(...)` (used locals/parameters by reference, `this` -> `self`, implicit members rewritten from the
  compiler's C2065 errors), compiles each on the FE_PFX_BASE/FE_JOBS lanes and ranks by (EH root-run distance, named
  call multiset distance, inlinemap decision distance, fe rows). `--moved` lists only ranges that move a decision,
  `--fast` skips inlinemap's per-site parse (giants), `--repeat` greps the tree for the range's first two statements.
  First pass 2026-11-19 (par-4): POP2 0x00405140 exact with its picture block as an owner (shared with popp:
  `popup_load_picture`, popup.h); no single-range owner found for PickTech::pick, call_council, config_game,
  NetWin::pick_player, base_editor_proj or enemy_move's entry.
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
  `--try` sweeps single-site rewrites (nvarg/nvop/nvany identity-inline probes, label/table flips, unname,
  expand/contract); `--pairs K --triples K` combine the single sites that moved the rows; `--write SUBSTR` keeps
  the best non-probe candidate whose key contains SUBSTR. A probe hit is a pointer: see MATCHING_GUIDE 3.2 S7,
  "Whole-set pass", for how to turn it into source (typed local, accessor) and when to reject it.
- `analysis/residue.py --from FEALL.json [--max 15] [--show] [--json F] | <unit> <addr[,addr]> [--src ALT]` residue
  classifier for near-misses (MATCHING_GUIDE 3.2 "Residue census"; 8 s for the 98 functions up to 15 rows): groups each
  function's differing rows into windows and names what separates the sides: `scratch` / `saved` / `regs` (a renaming,
  `rot+k` for one eax->ecx->edx rotation), `reuse` (one side reuses a register, no renaming exists), `order`, `operand`,
  `addr-order`, `zero-reg`, `copy(img|ours)` (a register copy = a named value on that side), `fold`, `slot`, `imm`,
  `bias` (pointer displacements off by one delta), `type(sign|width)`, `extra(n)` / `missing(n)`, `layout`, `other`.
  Per function it also prints instruction counts, frame sizes, the image's direct callers and how many sit in BYTE_EXACT
  markers (they confirm the parameter count and pushed widths), and convention notes (`this` unused, ecx read by a
  cdecl symbol).
- `analysis/outside.py --from FEALL.json [--min 1] [--max 15] [--levers sig,pos,inl,typ[,inl1]] [--only A,..] [--json F]
  [--tree BASEALL.json]` outside-the-body lever screens: every variant leaves the body alone and changes one thing
  around it in a shadow copy of src/ (`build/recovery/shadow/<prefix>/`, so header edits never touch the worktree):
  `sig` each int parameter and the int return type as unsigned int / short / char (definition, declarations, COMDAT
  found by prefix), `pos` the function moved to the end of its unit, `inl` one header inline the body calls respelt
  (N named result, B bare return, P parameters copied to locals; `inl1` one parameter at a time), `typ` the declared
  signedness of one global or field the body names. Prints the variants that move the rows. A header variant that adds
  a local shifts the front-end symbol counter of every including unit, so the count canaries move with it: read a
  `--tree` result against a control. A hit on `inl` is a pointer to the call site (see the guide), not a header fix.
- `analysis/sitek.py --from FEALL.json [--max 60] [--only A,..] [--write]` the site form of an `inl` hit: each
  integer-literal argument of a header-inline call becomes a plain local (`int fe_kN = <literal>;`), one site per
  variant (cost_factor 0x004E4430: 8 rows -> exact; propose_treaty 0x00544E40 exact; draw_pie 101 -> 34).
- `analysis/foldk.py --from FEALL.json [--min 1] [--max N] [--only A,..] [--skip A,..] [--json F]` the inverse of
  sitek.py: every plain single-assignment literal local of a non-exact body (`int lo = 1;`, not const) replaced by
  its literal, one local per variant (six such locals tree-wide on 2026-11-19; every fold is worse).
- `analysis/probes/blockorder/run.py [name ...]` block-order probe family (MATCHING_GUIDE 3.7, "Block order"): 79
  one-function variants (early returns, if/else arm order, goto joins, loop break/continue/return arms, if-chain
  against switch); prints each function's skeleton (branches, `ret`, `use(K)` markers); `SHOW=1` for the listing.
- `analysis/labelsubsets.py <unit> <addr> [--max N] [--write]` all 2^n subsets of a function's label_get /
  table_text sites flipped against the written-out form (S7): finds the pairs no single flip reaches
  (draw_projects 51 -> 9, ProdPicker::exec 62 -> 29, draw_commerce 60 -> 40).
- `analysis/sched/` the probe families and runner behind those rules (see its README).
- `analysis/webflow.py <unit> <addr> | --from FEALL.json [--list] [--show KEY] [--write]` value-flow screen
  (Buffer::tile 0x005DA860): a parameter adjusted into a separate local (`T L = P op E;`, P not read after it, or
  overwritten first) is rewritten in place (`P = P op E; T L = P;`, IP0/IPALL/IPS) or a plain copy dropped (CPALL),
  and the reverse, an unconditional in-place `P op= E` split into a new local (SPLIT). Ranked like localinplace.py.
  2026-11-19 tree pass: 10 candidates; MessageWin::on_selected 28 -> 17 the only source-proved gain (the analogous
  Buffer::tile(Sprite) needed the overwrite rule and a height local by hand, 61 -> 29).
- `analysis/webscan.py [unit ...] [--addr A,..] [--from FEALL.json] [--sweep] [-v] [--json F]` one name vs one value
  (MATCHING_GUIDE 3.3, 2.7 use_ability): per non-exact function, named locals/parameters with 2+ disjoint def-use
  webs (REUSE: SPLIT when the image keeps the webs in different slots, SHARED, REG) and hand-off copies `b = a`
  (COPY: MERGE/APART/REGCOPY, `/live` when a is redefined while b is live). `--sweep` compiles one split variant per
  web and one merge per hand-off copy and prints rows and frame (15 min tree-wide with FE_JOBS=5). The sweep does not
  check meaning: listing-based webs miss joins after tail merging, so read every web by hand before keeping one
  (first pass 2026-11-19: veh_scoot, monolith, probe, load_vehicles, draw_expenses real; base_draw, setup_text `k`,
  synch `local_turn` were broken splits that read as large gains).
- `analysis/behavscan.py <unit> <addr[,..]> [-v] | --tree [unit ...] --from FEALL.json [--json F] [--top N]` behaviour-difference
  screen (MATCHING_GUIDE 3.7 "A jump that skips a call ours always makes is behaviour"; 50 s tree-wide with FE_JOBS=5). Every named
  call and every field/global store gets the set of predicates it is control dependent on (post-dominator CDG, transitive, jump
  tables followed), each predicate canonical (`jle 7` = `jl 8`, an inverted jcc is the same test with the other truth, constant
  registers read through, latch tests and predicates the other side never tests dropped). Sites of one key are paired by guard
  distance, then fe alignment; one line per differing predicate: `INVERT` (same test, other arm), `CONST`, `OP` (`<` against `<=`,
  signed against unsigned), `IMG-ONLY` / `OURS-ONLY` (a test one side's site depends on), `COUNT` (missing/extra named call), and
  `TEST-K` / `TEST-J` (aligned cmp with another immediate, jcc with another relation inside fe's differing stretches). `tested at`
  names the jccs. Precision (second pass, 2026-11-19): every predicate carries what it compares (`@G:obj+off` global, `@F+d`
  field, `@R:callee` return value, `@S:off` frame slot, unknown operands kept apart per jcc), so `eq 0` on different
  variables no longer merges; register constants come from constant propagation over the CFG (`mov r,[__imp_X]; call r`
  names X); `lea r,[x+K]`/`inc`/`dec`/`sub K` before the test, switch `dec` chains, `neg`, and `test ah,K` read as the
  same value; our relocation symbols map to the image's marker object by address; vtable calls are `<indirect+0xNN>`.
  `COUNT tail` = the surplus copies of a callee share their continuation (cross-jumping; weight 0.25, their one-side
  guards dropped); `INVERT?`/`CONST?`/`OP?` = one operand unresolved (weak); CONST is dropped when both constants are tested
  on both sides; one-side guards weigh more with the number of calls they govern. `BS_DEBUG=<img addr,..>` prints both
  guard sets of a paired site. Control: 0 hits on 616 exact functions of 10 units. Tree pass over 344 functions: strong
  INVERT/CONST/OP 12 -> 1 (+8 weak), KREG 16 -> 2, COUNT 98 -> 75 (+22 tail); one-side hits 89 -> 209 (they were hidden
  by the `eq 0` merge). Still layout, read and skip: jump-threading re-tests, induction variables chosen differently,
  one-side inline expansions, teardown arms, ICF class twins (MATCHING_GUIDE 3.7). Confirmed transcription errors:
  battle_report wrap call (`IMG-ONLY [!lt 0]`), communicate pact-renewal arm, game_data 0x005A8651 flag block
  (`IMG-ONLY [!bit 32]` on 126 sites), ReportWin::draw_ops 0x0049E26D/0x0049E2F2 Base+0xD4 `mineral_surplus` read as
  `mineral_surplus_final` +0x98 (an IMG-ONLY/OURS-ONLY pair with the same test on different fields),
  NetSessionPickerWin::initialize 0x0063C2BB `Dialog::field_6C_ = 2` written as `field_5C_` (fieldscan FIELD).
- `analysis/fieldscan.py [--from FEALL.json] [unit ...] [--json F] [--strong] | <unit> --addr A[,..] [--src ALT.cpp]` data-flow
  half of the behaviour screen (25 s tree-wide with FE_JOBS=5): on fe-aligned differing rows with the same mnemonic and
  operand shape, a field displacement (FIELD), an immediate (IMM: stored constant, compare constant, pushed argument,
  multiplier, shift count) or an index scale (SCALE) that differs. Frame operands, stack adjustments, EH state stores and
  `fxch` are left out; hits are marked `swap`/`order` (the same operands within 8 rows on the other side: scheduling),
  `moved` (both instructions occur elsewhere on the other side: rows paired across a layout difference) and `ivN` (one
  displacement delta repeated: a pointer induction variable with another base); `--strong` prints only unmarked hits.
  `FS_REGMASK=1` compares with general registers masked (more pairs, more false hits through different base registers
  holding the same address). First pass 2026-11-19: 359 raw hits -> 13 unmarked, one real (NetSessionPickerWin::initialize
  field_6C_); behavscan's paired IMG-ONLY/OURS-ONLY hit on two fields gave ReportWin::draw_ops. The rest were IV bases
  (`lea r,[esi+K]` from a relocated base), misaligned rows and ternary sense (`setg; dec; and -2; add 6` against
  `setle; dec; and 2; add 4`: the same values, the image's `c <= K ? a : b` spelling; setup_player 0x005B28AF).
- `analysis/structscan.py [unit ...] [--addr A[,..]] [--json F] [--claimed]` set screen independent of instruction pairing
  (35 s tree-wide with FE_JOBS=5). Our relocated fields are replaced by their source-bound image addresses (string
  literals keyed by content, `__real@` by bytes, imports by IAT name), so both sides decode to absolute addresses. Prints
  per non-BYTE_EXACT function: `REF` fe-aligned rows with identical text whose relocated target differs (refscan's question
  on non-exact functions; rows whose two targets both balance elsewhere in the function are dropped as load-order swaps,
  `SS_ALLREF=1` keeps them), `G_img`/`G_ours` globals (symbol+offset) one side touches and the other never does, `F_*` field
  offsets by a linear taint type (`this`, a global's address, `*G` loaded pointer; `?` untyped) and `I_*` immediates on one
  side only (byte/word/ah-high spellings folded to one key). Most one-sided rows are folding: a pointer IV biased by
  another field (`Bases+0x810` against `+0x814` loop ends, `[eax-8]` against `[eax-0xc]`), indexed `[i*s + G + k]` (keyed
  without k), load-order swaps and unbound names of ours (vtables, unmapped globals: `G_img` marked `?`). Read a lead against
  the image before acting. 2026-11-19 first pass, four real ones: reset_territory RadiusOffsetX/Y against VehMoveDeltaX/Y
  (G), communicate's tech split at Technology[89] against [87] (an in-loop IV compare, G), process_message TimeControl
  `.extra` against `.event` (REF) and enemy_move `minerals_accumulated_2` against `unused_surplus` (REF).
  Filters (2026-11-19 second pass): `SS_REF=winN` drops a REF row only when the swap balances within N instructions on
  both sides (`SS_REF=block`: the same basic block) instead of anywhere in the function; `SS_CTX=k` keeps only rows inside
  an agreeing run (k equal fe rows each side), which removes fe misalignment (most raw hits). `SS_CALLS=1` adds `CALL`:
  fe-aligned direct calls whose targets differ (ours resolved through the bound marker addresses; an unbound callee is an
  ICF alias such as `remove_all_walk`/`remove_all`). Globals the measurement targets lack are named from
  `RECOVERY_OBJECT` lines and from `// 0xADDR` trailing comments on declarations. Controls: REF and CALL over every
  BYTE_EXACT marker give 0 resolved mismatches. Second pass found InfoWin::setup_text `Might[].adj_start` (REF, win16 +
  CTX=1); CALL with CTX=2 found no sibling-call typos, only inline-budget rows (select_special's 5-argument Popup::start,
  MapWin::click's ::draw_map, probe/enemy_move's x_dist).
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
- `analysis/locpad.py FEALL.json [maxrows] [minrows]` function-local tie screen (MATCHING_GUIDE 3.6, "the number of named
  locals mod 8"): 0..7 dead referenced locals before each near-miss function's closing brace; lists the functions whose rows
  move (2026-11-19: ReportIf::draw_labs exact at 7, Gamma::exec 21 -> 6 rows at 6). Fix by naming, never keep the pads.
  `--move` options: `--deps` also moves the host's unmarked pieces (objects, helpers, `#define` blocks) only the moved
  functions name, and the detached body of a moved marker block; `--include '*'` unites the hosts' includes;
  `--new-like L.cpp [--after P.cpp]` creates the destination as a new unit with L's flags and includes; `--with NAME,..`
  moves named definitions. A `$E` address (static-initializer thunk) moves its marker blocks with the object it
  constructs, removes the destination's own `extern` of it, and places the definition in thunk order among the
  destination's objects. A host left with only its prelude is removed from the tree and the build lists.
- `analysis/esym.py <unit> [--write]` refreshes the `// symbol _$E<n>` lines of a unit's static-initializer markers from
  its compile (bytes outside relocations, named relocation targets at the image's addresses, sibling thunks); run
  automatically after every tumap regroup. `NOT EMITTED` = the object's definition is not in the unit.
- `feall.py --base` compares rows by address, so a marker moved to another unit is compared with its old row; fe/feall
  read a worktree's own `build/compile_commands.json` for units the main database lacks (a unit created by a split).
- `analysis/leascreen.py FEALL.json [--max N]` (lane FE_PFX_BASE) lists non-exact functions whose only differences are lea
  base/index order or a block of the same instructions in another order: the IL-node counter class (MATCHING_GUIDE 3.4).
- `analysis/packsim.py <unit> <addr> [--src ALT.cpp] [--why] [--sym NAME,..] [--fit last|first|smallest]` frame
  packing model check (MATCHING_GUIDE 3.3 "Frame slots are packed last-fit"): rebuilds the memory units of our
  compile (named, `$T`, one unit per anonymous temporary web), runs last-fit in raw-count order on our home
  liveness and prints how many units land in our compiler's slot, the offset-order check (founder against
  total) and, with `--why`, the deeper slots each temporary could not take and their blockers. ~15 s.
- `analysis/varset.py <unit> <addr> [--src ALT.cpp] [--min K] [--sites]` variable-set reconciliation (MATCHING_GUIDE 3.3):
  the image's frame slots (accesses, immediate/callee-saved/scratch stores and loads, `mem` when never cached in a
  callee-saved register) against our locals, each access of ours named from its /FAsc operand (`name`, block-locals
  `name#NNN`, compiler temporaries `$T..`/`T-0xNN`) and paired through the fe alignment. Prints both tables with the
  slot rank on each side (slot order is the packed slot's access total, so a rank difference names a variable with
  too many or too few references, or a slot that hosts the temporaries another variable hosts in the image), MERGE
  candidates (several of our variables in one image slot: an original reuse, or VC6 packing) and SPLIT candidates
  (one of our names over several image slots). First pass: enemy_move (j/k/own_ground/n rank 7-13 places low, the
  spilled induction temporaries all in tile_owner's slot), battle_fight (image slots -0x74/-0x78 have no local of
  ours: the 8 missing frame bytes; stack_id spans -0x18 and -0x40), order_veh, probe, base_build.
- `analysis/imgmerge.py <unit> <addr> [--plan] [--sites SLOT,..|all] [--out FILE | --write] [--only SLOT,..] [--all] [--skip NAME,..]`
  image-slot variable merge (MATCHING_GUIDE 3.3 "One reused function-scope variable per image home"), the rewrite
  step after varset.py. `--plan` lists, per image slot, our function-scope (f) and block-local (b@line) variables whose
  aligned accesses use it; `--sites` prints a slot's timeline (image address and instruction, our aligned instruction,
  variable, source line; `~L` = nearest aligned line for an unpaired image access). The default run takes every slot
  with a function-scope owner (or hoists its first block-local), renames the other block-locals into the owner one at
  a time (owner not named in the scope, no goto into it, reusesweep's kill probe compiled away), measures each slot
  alone (frame difference, slot-map difference, rows) in FE_JOBS lanes and then applies the improving slots
  cumulatively, best first. `--only=SLOT,..`/`--all` apply the named/all slots without the screen. `--fscope` also
  merges function-scope variables that share an image slot (every plain definition of either is probed for the other
  being dead); a refusal lists the source lines where the other variable is still live, which is the finding when the
  image packs two of our variables that our source keeps alive together (base_build's `w`/`l_34`: the image holds
  `w` in ebx over the convoy loop, so its home is free there). A marker followed by inline helpers (enemy_strategy)
  is located by the function the marker's symbol names. Results 2026-11-19: air_power frame 0x44 -> 0x48 and
  955 -> 753 rows (by hand from `--sites`), enemy_strategy slot difference 102 -> 75 (2 of 13 shared slots kept),
  time_warp 150 -> 142; order_veh, consider_designs, setup_text, social_ai, communicate: no slot improves (their
  shared slots are VC6 packing at the image's rank already). Lanes: one slot per prefix FE_PFX_BASE..+FE_JOBS-1; a
  3,000-row unit takes 10-25 s per rename, so run large functions in the background.
- `analysis/padkinds.py <unit> <addr> <signature> <lo-hi> [kinds]` which declarations count for such a tie: k = 0..9 pads of
  ~40 kinds, rows inside the image window per k (period 8 = one frame symbol, 4 = two, one step = /Ob2 caller budget).
- `analysis/sympad.py FEALL.json maxrows OUT.jsonl` the budget twin of locpad.py: block-scope `(void)&extern` pads, which grow
  the caller's size and never move a tie; a function that moves under both at the same count is a budget case.
- `analysis/prosave.py [unit ...] [--addr A,..] [--src ALT] [--class abcd] [--census] [--json F]` prologue-save and
  return-tail screen (MATCHING_GUIDE 3.7 "Prologue saves and return tails"; 60 s tree-wide with FE_JOBS=5): per non-exact
  function the callee-saved pushes before the first branch (`up`), the deferred ones (`def`) and every `pop ..; ret` tail with
  the number of ways into it, image over ours; classes a (image up front, ours deferred), b (reverse), c (tail count), d (order),
  s (saved set). `--census` tabulates the exact functions' shape against the source guard spelling (7 s, no compile).
  `analysis/probes/prosave/run.py` is the probe family behind the rule (one edge from the register's region to the exit).
- `analysis/stmovescan.py <unit> <addr[,..]> [--src ALT] | --tree [unit ..] [--from FEALL.json] [--both] [--json F]` moved
  spill-home store screen (MATCHING_GUIDE 3.7 Frame and slots, draw_session_info; 40 s tree-wide with FE_JOBS=5): pairs
  unaligned `mov [ebp-N], v` stores of both sides (slots mapped by aligned-row votes, a same-row store counts as aligned)
  and flags an image store that sits at least one of our basic blocks later than ours (`LATER blocks +k`, `CALL` = a call
  follows within 8 instructions), with our source line of the store and at the image's position. Read the flag with
  equal register values (`img_ins`/`our_ins` value equal); mismatched values are mostly slot or register permutations.
  2026-11-20: load_vehicles (source_y after the skip test).
- `analysis/loopexitscan.py <unit> <addr[,..]> [--src ALT] | --tree [unit ..] [--from FEALL.json] [--json F] | --src-pass`
  loop-exit counter store screen (MATCHING_GUIDE 3.7 Frame and slots, Path::find; 40 s tree-wide with FE_JOBS=5): per
  non-exact function the `mov [ebp-N], K` stores of small immediates (2..4096), image slots mapped to ours by aligned-row
  votes; `IMG`/`OURS` = one side stores K to the slot more often (a variable the original used as a loop counter, whose
  exit value VC6 stores, or the reverse), `MOVE` = equal counts but an image store has no same-K store on its row, `*` = K
  is a loop bound (`cmp x, K` + backward jcc) on either side. IMG/OURS pairs of one K and excess are cancelled as slot
  permutations. `--src-pass` lists `v = K;` within 6 lines of a `for` over another variable bounded by K (no compile).
  Noisy where slot votes fail (big functions); read each flag with `fe.py --all`. 2026-11-20: load_vehicles (OURS* 8,
  82 -> 3 with two follow-ups); the remaining 21 flags read as slot/register permutations.
- `analysis/storesched.py [unit ...] [--addr A,..] [--src ALT] [--max N] [--json F]` moved-store screen (MATCHING_GUIDE 3.6
  "A member setter's parameter schedules the store early"; 60 s tree-wide): the `mov [reg + K], v` stores both sides emit at
  different places of one differing stretch, rows earlier/later, base register `this` or another object; `SETTER?` = moved
  only across argument pushes (try the class's inline setter, or a one-value setter when two functions need it).
- `analysis/retcensus.py [unit ...] [--json F] [--show CTX/FORM]` return-statement census over the exact functions
  (25 s tree-wide): one /FAsc compile per unit; every `return` statement is COPY (own epilogue), FOLD (no code), JMP or
  FALL against its context (last, guard, guard&&, guard||, after, case, else; `=last` when it returns the last return's
  expression). Ground truth for MATCHING_GUIDE 3.7 "Return-tail copies".
- `analysis/retmap.py [unit ..] [--all] [--addr A,..] [--show] [--json F]` return-site map (MATCHING_GUIDE 3.7 "Epilogue
  copy or branch"; 55 s tree-wide): per non-exact function the `ret` count image/ours and every unpaired return tail,
  `IMG-COPY` (the image has an epilogue copy where ours branches into a shared tail) or `OURS-COPY` (the reverse), with the
  value set, entry (F fallthrough, Jn jumps) and our source line; `--show` lists every tail on both sides. An OURS-COPY on
  a nested `||` return is the veh_init case (separate `if (..) goto done;`). `analysis/probes/rettail/dup.py` is the probe
  matrix behind the rule.
- `analysis/retbrk.py <unit> <addr> [--write] | --from FEALL.json` switch-arm exit screen: for every `switch` directly
  followed by `return X;`, the arms' `return X;` as `break;` (A) or the reverse (B), per switch and every subset
  (Net::drop_player 0x00632350: both inner switches A -> exact).
- `analysis/probes/rettail/run.py [name ...]` probe family for return-tail copies (guards, switch arms, loops, the
  drop_player, veh_init and NetDaemon::init shapes); prints each `ret` copy's value and how it is entered.
- `analysis/rdeck.py <unit> <addr> VARIANTS.py [KEY,..]` replacement deck: VARIANTS.py defines
  `V = {key: [(old, new), ...]}` (each old string unique in the unit); ranks the variants by differing rows, writes the
  named variants under `build/recovery/fe/rd/`, and with `RDF=1` prints each variant's `sub esp` frame line (frame-first
  judging, MATCHING_GUIDE 1.1).
- `analysis/probes/rectpack/run.py [name]` probe family behind MATCHING_GUIDE 3.3 "A RECT packs with a later
  address-taken array": frame and slot map of each RECT/char[16] variant (common loop, out-of-line `&rect` call, block
  scopes, pointer and copy spellings).
- `analysis/splitaudit.py <unit> <addr> [--min K] | --from FEALL.json [--rows N] [--keep DIR]` split screen, the reverse
  of imgmerge.py (MATCHING_GUIDE 3.3 "One name over two image homes"; one compile per function, about 8 s): our named
  locals whose paired accesses fall into two or more image homes over disjoint source line ranges, with the first image
  address of each range. Tree pass of 2026-11-19 over 214 non-exact functions of 30+ rows: 12 functions flagged; kept
  battle_compute (90 -> 35, is_bombard also served the bunker/base multiplier), consider_designs (958 -> 950), CouncWin::init
  (237 -> 227), base_ecology (184 -> 174), base_terraform (922 -> 893), gen_terrain_poly (1516 -> 1508), Path::find
  (876 -> 858, the value belonged to another variable's home); TutWin::tour, capture_base, probe and setup_player read
  best as they are. Seven of the twelve were reusesweep renames. Judge a candidate with `imgmerge.py --plan` and rows.
- `analysis/workarounds.py --feall F [--table] [--rules] [--json OUT] [--kind K,..] [--file S]` stand-in debt inventory
  (MATCHING_GUIDE 4.1): every WORKAROUND tag, budget probe (definitions and call lines), function-pointer local, pragma
  region, pad block, volatile cast/local and `__forceinline`, with the functions it serves, its evidence comment, their
  fe exactness (from a `feall.py --all` result) and, with --rules, the rule most likely to retire it. 5 s.
- `analysis/retire.py WA.json [--write] [--kinds ...] [unit ...]` mechanical retirement screen over workarounds.py
  `--json` items: each probe statement set, function-pointer local, pragma region, `__forceinline` and volatile cast
  removed alone, every marker of the unit measured (DEAD / KEEP / NO); --write applies DEAD and KEEP cumulatively.
  fe masks relocations: confirm with `recover measure` (a load-order volatile reads DEAD and is not).
- `analysis/casescan.py <unit> <addr[,addr]> [--src ALT] [-v] | --from FEALL.json [--json F]` switch case-label screen
  (MATCHING_GUIDE Z9 "Shared case labels"): per jump table (byte index tables included) and per compare tree
  (concrete per-value walk of `cmp/sub/dec/test` + jcc chains), the image's case value -> block partition against
  ours. PART split/merge = ours shares a block the image keeps apart (write the cases apart) or the reverse; DFLT =
  one side joins the bound check's default block; BLK = a case's straight run differs in length (cross-jumped tail);
  TGT/NTAB/NTREE/BIAS as named. 60 s on the non-exact set. 2026-11-20 pass (327 functions, 53 with dispatches):
  PART only in base_build (owned) and vehdraw_scoot_start (ours cross-jumps identical cases 0/6, which the image
  allocated apart: a scratch-register question, not labels); pre-fix vehdraw_construct_chassis reads PART [1,5] +
  DFLT 0/9. The other BLK rows are default blocks, `abs`-style arms beside a tree, or regalloc tails.
- `analysis/switchtail.py <unit> <addr[,addr]> [--src ALT] | --from FEALL.json [--units u,..] [--json F]` shared
  switch-tail screen (MATCHING_GUIDE 3.7 "A switch arm that falls into a shared tail"): per jump table, the joins two or
  more case regions reach by `jmp`, each with its HOST (the case that falls into it), matched to ours through the fe
  alignment (or, unaligned, by shared case values). `HOST ... img-first` = the image's earliest case falls into the
  tail and ours a later one (the on_key_click goto-label shape: write `break` + a flag-tested tail after the switch);
  `HOST` otherwise = the default or another case hosts (communicate: ours `ja` went to a label before the tail);
  `NJOIN` = jumper counts differ (a case that keeps its own copy of part of the tail: an arm-local push or store).
  70 s on the non-exact set. 2026-11-20 pass (327 functions): the img-first shape occurs only in the pre-fix
  on_key_click; communicate (late-contact menu breaks) and MainInterface::init (inline argument in every case)
  fixed; base_build, probe, menu_proc (toggle tails, 3.7) and vehdraw_scoot_start (3.7) open.
- `analysis/cmpscan.py [unit ..] [--all] [--weak] [--addr A,..] [--json F]` compare operand order / jcc sense (VC6 keeps
  both from the source): pairs each image `cmp a,b; jcc` on a differing row with an ours `cmp; jcc` within 8 rows and
  flags `SWAP` (`cmp b,a; j<mirror>`), `INV` (same operands, negated jcc) and `SWAPINV`; `--weak` adds register-renamed
  shapes (RSWAP/RINV, mostly allocation noise). 50 s tree-wide. 2026-11-20: SWAP is the useful class (prune_protos fixed;
  SpriteBox::calculate_dimensions is a ternary whose arm order VC6 picks itself); INV rows are block layout or epilogue
  duplication, and spelling the test the other way compiles identically (Console::activate, veh_init).
