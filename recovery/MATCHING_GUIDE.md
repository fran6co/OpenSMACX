# VC6 matching guide

This guide distils the lean match loop's batches 1–9 (2026-11-06 to 2026-11-12).
Over that period byte-exact functions rose from 3,938 to about 5,090 of 5,661.
Part 1 is a reusable brief for a matching worker. Parts 2–4 are compiler rules
measured with micro-experiments, a lookup table from symptom to cause, and
levers that were measured and proved neutral. `AGENTS.md` and
`recovery/WORKFLOW.md` own policy and win any conflict. `COMPILER_EVIDENCE.md`
keeps the older historical observations.

Update this file when a micro-experiment establishes a rule or disproves a
lever. Do not append per-function logs; commit messages carry those.

## 1. Worker brief (paste into a worker prompt; fill in `<N>`, prefixes, files)

You are making functions byte-exact in OpenSMACX, a faithful, bug-for-bug C++
reconstruction of Sid Meier's Alpha Centauri (terranx.exe v2.0, VC6), kept for
preservation. Never fix an original bug. Read `AGENTS.md` ("Current mode" and
"Working rules") and `recovery/MATCHING_GUIDE.md` (this file) first.

- **Checkout.** Work only in `.opensmacx/worktrees/lean-w<N>`, which is
  configured and prepared, and never touch the main checkout.
- **Compiler prefixes.** Prefix every `recover` command with
  `VC6_PREFIX=<prefix>`, run one recover command per prefix at a time, and
  give `recover permute --jobs K` one `--slot VC6_PREFIX=<p>` for each prefix.
- **Scope.** Edit only the assigned files. Report header changes as proposals
  with evidence unless the prompt allows them. Make no git state changes; the
  coordinator collects `git diff`. Don't run `record`, `prepare`, a full
  `check-matches` or the whole pytest suite.
- **Loop.**
  - `recover measure <addrs> --in <file> --image .opensmacx/game/terranx_original.exe`
    takes about 5 s. Refused rows carry `diagnostic_similarity`, and EH-topology
    refusals print both state tuples (see 2.4).
  - `recover diff <addr> --in <file> --image ...`. Delete
    `build/recovery/diffs/<addr>` first: a failed compile leaves the stale HTML
    behind.
  - `recover permute <addr> --jobs K`.
    - `--operator continue_guard|shared_return|break_goto|budget_probe` limits
      the search.
    - `--candidates deck.json` ranks your own spellings, which is much faster
      than random search in big units.
    - `--probe-sweep --time-limit S` searches dead budget probes.
  - `recover check-matches --unit <tu> --summary` is the scoped regression check.
- **Priority (user decision, 2026-11-12).** First bring every MISMATCH to
  "within registers" (verdict MNEMONIC_ONLY or SHAPE_EXACT: the image's
  instruction sequence with only register/slot choices differing), which a
  rewrite usually achieves in one session and which captures the structure
  and behaviour. Register/slot ties are a separate later pass: time-box them
  (about 20 minutes per function) and move on, reporting what was tried.
  Track both counts: byte-exact and within-registers.
- **Targets.** Measure every `// ORIGINAL:` line in your files that lacks
  `BYTE_EXACT`. Work near-misses first, then refused rows by their diagnostic,
  then rewrites of distant functions.
- **Large functions.** Rewrite in the image's block order from
  `llvm-objdump -d -M intel --no-show-raw-insn --start-address=A --stop-address=B`.
  You may split the listing into chunks of about 2,000 instructions, translated
  in parallel by at most 4 read-only sub-agents. Share one table of frame slots
  to variables across the chunks. Keep a rewrite only if it measures better or is
  clearly more faithful.
- **Rules.**
  - Where our code deviates from the image, restore the original behaviour and
    report it as "restored original behaviour".
  - OpenSMAC is an aid for understanding intent only. Use its
    `BUG(original)`/compat branches, and never copy its code or names.
  - No `_emit`, copied machine code or fixed-address calls. `__asm` is for
    original hand-written assembly only.
  - A stand-in is kept if it reproduces a structural property of the image
    (frame, calls, layout), exact or not. Tag it `// WORKAROUND(<inline|noinline|type|decl|volatile>): <evidence>`.
  - Never edit `ORIGINAL:` lines or `BYTE_EXACT` tokens or compiler flags. Add no
    tests or JSON.
  - Time-box each hypothesis. Check `git log -p -S'<name>'` for earlier attempts
    and exact seeds first.
- **Report what broke; do not revert for it.** At the end, measure every
  `BYTE_EXACT` address in every file you edited and list the ones that no
  longer match. A structurally correct change stays even when it breaks
  exact functions (AGENTS.md, "Structure first, no ratchet"); revert only
  what the image shows to be wrong. `cmake --build build` must link.
- **Report**, concise:
  - per file, the claimed count and how many claims are still exact;
  - newly exact addresses, each with a one-line cause;
  - restored original behaviour;
  - proposed header changes with evidence;
  - new general rules;
  - tried and reverted.

Tooling traps:
- `pgrep -f <pat>` and `pkill -f <pat>` match the waiting shell itself; wait on a
  PID or an output file.
- `llvm-objdump` mis-decodes VC6 objects at `.lf`/`.ef` symbols: disassemble a
  *copy* stripped of them (`llvm-objcopy -w --strip-symbol=.lf
  --strip-symbol=.ef in out`, never in place on a qualified input).
- In callers of thousands of instructions, flat probes grow the budget as much
  as they spend; use branchy probes to measure missing spend.
- `RECOVERY_ASSOCIATED` switch lines may sit at the end of the file, away
  from their function: grep before adding one; a duplicate makes `measure`
  print nothing.
- Run only one `recover` command at a time per worktree, even on different
  prefixes: concurrent measure/diff runs in one worktree fail silently.
- A greedy per-hunk search over a rewrite finds real gains but also keeps
  half-applied pairs that change behaviour (a store without its read, a
  dropped assignment, a duplicated call): review every kept hunk.
- Wine reads stdin: run batch compiles inside `while read` loops with
  `</dev/null`, or the loop stops silently after a few units.
- Wine is case-insensitive: object names differing only in case overwrite
  each other; use unique lowercase names in screening scripts.
- For functions with in-text jump tables, `measure` similarity counts the
  table bytes past `ret`; compare body-only mnemonic ratios.
- `/tmp` is a small tmpfs; keep large output under the worktree's `build/`.
- `permute --candidates` can give identical distances for variants that
  `measure` scores differently (extent-mismatched functions); confirm with
  `measure`.
- Editing *any* source file in the worktree mid-run stops `permute`
  (`inputs_changed`), not only the permuted one.
- Body comments must not contain the word `ORIGINAL`.
- `recover measure` refuses backslash-continued macros.
- `measure` similarity uses difflib autojunk, which underrates functions above
  about 200 instructions; compare differing-line counts from `diff` instead.

### 1.1 Judging candidates and searching (2026-11-18)

The remaining functions are stuck in local minima, so do not accept or
reject a candidate on similarity. Compare structure in this order and keep
a change that fixes a higher rung even when similarity falls and even when
it breaks byte-exact functions (they were tuned around the wrong structure
and are restored by the next fix, never by a revert): EH states, call multiset, frame size, block layout, stored and
reloaded slots, registers, bytes. ProdPicker::on_scrolled went 0.893 → 0.78
→ 0.91 → exact; the 0.78 step was the one that reproduced the frame.

A raw compile of one unit takes 0.3 to 2 s, against 4 s or more for
`recover measure`. Compile the unit directly (the command is in
`build/compile_commands.json`), disassemble the function's COMDAT and the
image span, and diff them with relocations masked; confirm the final
candidate with `recover measure` and bank with `recover record`. At that
speed a small grammar of source forms (which object each read uses, where
each statement sits) can be enumerated exhaustively: 288 forms take 12 s on
14 compiler prefixes. Restrict the score to the image address window being
worked on so that unrelated ties do not hide a structural match.

### 1.2 Rewrite sweeps (2026-11-18)

`recovery/tools/ruleall.py <rule> --write` tries one rewrite at every site
inside the non-exact functions of every unit and keeps a site that lowers
its unit's difference by three rows or more without breaking a frame or
call-count match (it does not protect exact functions). Rules: `ifswap`
(exchange if/else arms), `earlyret` (exchange an early-return block with
the rest of its scope), `declswap` (adjacent local declarations),
`stmtswap` (adjacent independent assignments), `cmpswap` (mirror a
comparison's operands), `ternary` (`v = c ? a : b` as if/else);
`accall.py` replaces inline accessors by the member they return. First
pass: 95,432 differing rows over the non-exact functions -> 90,818, 94
functions closer, Scroll::on_mouse_move exact; statement order (57 sites)
and arm order (62) gave the most. A second pass adds a handful, so rerun
them only after other source work. Register-only differences are still
about a third of all differing rows; the sweeps reach few of them.

Exhausted on the near-misses (do not rerun without new rules): every
single site and every pair of sites of the behaviour-preserving rewrites,
inside each of the 90 functions within 12 differing rows
(`recovery/tools/pairsall.py`), lowered none of them. Dropping a local
that names a simple read (`unname`, exact-only) and hoisting declarations
(`declsplit`) reached nothing either. The remaining near-misses are not a
matter of arm, statement, declaration or comparison order.

## 2. Established VC6 rules (measured with micro-experiments)

These rules apply to the production recipe (`/O2 /Ob2 /Gy /GR- /GX`, with or
without `/Oy-`).

### 2.1 /Ob2 inline budget

- **Who owns it.** Every caller has one budget. Its size grows with the caller's
  own size, which includes dead code, plus a sizeable minimum.
- **What it costs.** Each inline expansion spends budget according to the
  callee's *source spelling* (temporaries, named results, branches), not its
  emitted code. Nested inline decisions inside an expanded callee draw on the
  outer caller's budget.
  - A pure store inline such as `parse_set` is nearly free; a branchy one such as
    `range` or `next_entry` costs a unit.
  - The number of small expansions is a lever that does not depend on position:
    roughly two expansions move a teardown one level.
- **Order.** First-level call sites are decided greedily in source order, and a
  refused site is skipped while cheaper later sites can still inline. Calls
  inside expanded copies are decided in a second pass with reservations;
  see 2.8 for the measured model. Every
  expansion competes, including code after the return sites, and there is a
  per-callee size cap as well.
- **Per-callee auto-inline size cap (2026-11-19).** Separately from the
  caller's budget, /Ob2 refuses to auto-inline a plain (non-`inline`)
  function whose *own source* exceeds a fixed size, at every site and
  whatever the caller's size. Micro-tests: 7 one-line `if`s inline, 8 do
  not; a 12-term sum inlines, 16 does not. What counts is the callee's
  source spelling, not its bytes: each separate statement or guard clause,
  each repeated expression written out (two `Chassis[VehPrototypes[...]]`
  triad reads, `Continents[r].tile_count` at each use instead of a named
  local), member calls on subobjects/globals more than plain calls; calls to
  inline helpers inside the callee do not count. So the same bytes can be
  under or over the cap: tech_avail as one combined condition expands into
  tech_ai, as three guard clauses it is called (image); veh_jail with the
  triads open-coded is called from all eight sites. An explicit `inline`
  (or in-class) function is not held to the cap; only the caller's budget
  limits it. Definition order is not a lever for the cap: a capped callee
  stays a call whether defined before or after its caller, an uncapped one
  expands either way. Diagnosis: a same-band callee that *every* caller in
  its unit calls out of line, while the callers have budget left, is over
  the cap; find the original spelling that puts it there (delete one
  statement at a time and watch C4711) instead of `auto_inline(off)`. A
  callee expanded at some sites and called at others is a budget effect,
  not the cap. When an explicit inline is refused by a budget, its
  out-of-line COMDAT copy is emitted right after the first function that
  calls it (see "Out-of-line copies of explicit inlines" below).
- **Depth before position.** Depth-1 sites expand before depth-2 sites. Every
  invented forwarding layer in a recovered helper (`write_cent` →
  `write_cent_text`) pushes the real body one level deeper and moves the
  cut-off; collapse such layers into the original body. Parameter copies
  (`total`, `cur`) inside a callee can make it unaffordable at a site even when
  its own bytes don't change (buffer.cpp `write_strings`, 0.41 → 0.80).
- **Look for the original helper first.** Before inventing an inline helper
  for a repeated block, search the image for an out-of-line function with
  that exact body; if it exists with one catalogued caller, the original
  called it and /Ob2 decided where to expand it (TutWin::UNK1 is reset's ten
  stores; a header-inline stand-in flipped 14 functions elsewhere).
- **`if (x >> 16)`** gives `test r,0xFFFF0000` and keeps the body's separate
  `x & 0xFFFF0000`; `if (x & 0xFFFF0000)` shares the masked value
  (mov/and/jcc); `HIWORD(x)` gives shr.
- **Duplicated tails block auto-inlining.** /Ob2 refused a function whose two
  arms each end in ~10 duplicated stores and a return (TutWin::reset); a
  single-tail form inlines but its standalone copy loses the duplicated tail.
  Factor the shared stores into an inline helper called from both arms: the
  function then inlines and the standalone copy keeps the image's tails.
- **Out-of-line copies of explicit inlines.** VC6 emits the COMDAT copy right
  after the first function in the unit that calls it out of line; if every
  caller expands it, no copy is emitted. The copy's image position therefore
  names the function that needed it. A 219-byte plain member
  (StatusWin::redraw) is never auto-inlined.
- **Out of line at one site:** `(obj.*&C::f)()` still expands a header inline;
  a non-const local member pointer `void (C::*p)() = &C::f; (obj.*p)();`
  keeps the call (NetDaemon::reset in menu_proc).
- **Definition order and keywords.** Definition order is not a reliable
  lever: plain functions defined after their caller usually stay calls, but
  window_proc's static handlers and free functions defined after it were
  auto-inlined (C4711), and reordering compiled identically; always check
  C4711. An explicit `inline` *member* defined later still inlines.
  `__forceinline` and `inline` compile identically for `label_get` across
  all 48 using files.
- **Seeing decisions.** Compile with the unit's production command from
  `build/compile_commands.json` plus `/W4`, then grep `C4711`. That lists every
  automatic inline in order; explicit `inline` functions are not listed. Treat
  the sequence as a fingerprint: decisions are not monotone across nested sites.
- **Out-of-line at one site.** Call through a local const member-function
  pointer, or for a member, `(this->*&C::f)(...)`. A pointer to a member of a
  class with virtual bases is fat and does not work; use a per-TU
  `#ifndef X_OUT_OF_LINE` guard around the header body. `#pragma inline_depth(0)`
  disables every expansion in the function, and `auto_inline(off)` on the callee
  affects every caller.

### 2.2 Phantom teardown (EH)

The front end emits a teardown of function-scope objects at the closing brace
unless the **last top-level statement is `goto` or `return`**. It never treats
`for(;;)`, `while(1)` or `do … while(true)` as infinite, and it never looks
inside blocks.

That dead teardown emits no code but adds EH states and spends budget.
editor_tech had 40 states against the image's 25; editor_veh had 46 against 29.
The fix is a top-level `again: { … } goto again;`, or ending with a top-level
`return`. Compare the state counts before reaching for budget probes.

Unreferenced EH states: when FuncInfo's maxState exceeds the highest state the
code stores, the missing block is the phantom teardown of a final if/else whose
arms both return; end the function with that if/else, not a trailing return
(upgrade: 13 site groups became the image's 15).

### 2.3 Teardown depth across return sites

- **Order.** EH states are numbered depth-first in source order. /Ob2 decides
  first-level calls greedily in source order, including each site's `~Popup`.
  Deeper teardown levels come afterwards, favouring the final teardown. Early
  return sites lose depth greedy-with-skip, giving non-monotone patterns such as
  5,2,5.
- **What does and doesn't move it.** The nesting depth of a return is not a
  lever; the *kind* of inline spending (one level or two) is. A Popup routed
  through an inlined function such as `editor_who` caps its teardown depth.
- **More states than the image.** The original spent more budget before the
  teardown. Look for an inline the image expands that ours calls, such as
  `seek_id` or `next_entry`. Also look for missing wrapper calls:
  - `BasePop::exec()`/`exec(cb)`/`item`;
  - `Popup::start(label)`/`start(label,value)`;
  - `set_picture`, `veh_battle_msg`, `clear()`, `say_num()`;
  - `has_treaty`/`has_agenda`, `get_noun`/`get_name`/`get_title`.

  These have out-of-line header-inline copies in the image (0x5A5900, 0x558FC0,
  0x558FE0, 0x5A5990, 0x559040, 0x559000, 0x50B9C0). Raw field writes in place of
  these accessors lose their spend.
- **Fewer states than the image.** An invented helper costs too much; write its
  body out instead.
- **Method.**
  1. Fix the diffs that are not teardowns.
  2. Read the per-site depths from the EH predecessor tuple below.
  3. Probe with a dummy inline at a few positions, or use `permute --probe-sweep`.
  4. Replace the probe with a natural spelling (getter, wrapper, `on_map`,
     accessor).

  A probe sweep compiled to an object and read straight from the FuncInfo unwind
  map, with no `recover measure`, answers three things in minutes: where spend
  is missing, how much, and whether spend can fix the function at all. It
  cannot when freed budget flows into the final teardown instead (make_gift:
  the original caller was smaller, not costlier), or when the image calls a
  helper out of line at every budget level (POP2/POP3's `next_entry`).
  Natural spellings that supplied missing spend: `on_map`, `is_alive`,
  `is_human`, per-site `label_get`, and header getters (`get_noun` through
  `parse_set`). These made POP3 and pick_map's topology exact.

  When a sweep leaves almost every placement unchanged (crash_landing: 352 of
  377 variants identical), spend is not the cause; look for an over-sized
  caller or a header-level inline difference (which helper copy a teardown
  calls) instead.

  Cheap flat probes grow the caller's own budget about as much as they spend;
  a teardown they cannot move may still move under one *branchy* dead inline
  (a 3- to 31-way `if (x == k)` chain), with a large jump in states (popp,
  amovie_project, pick_player).

  Probe position matters: dead spend takes depth only from return sites
  *before* it. To make an early site shallower, place spend after it; spend
  before the first return adds caller size and deepens sites. When the image
  expands a per-TU-guarded header inline inside a teardown (e.g. `~Dialogs`
  under `DIALOGS_DTOR_INLINE`) and the unit lacks the guard, no spend reaches
  the image's depth: compare the final teardown's call sequence first.

  Flat zero-code expansions placed after an early return can make that site
  exactly one level shallower once there are at least N of them (more changes
  nothing); branchy probes may hit the right state *count* with the wrong
  distribution, so compare relocation sequences, not just state tuples
  (amovie_project). `popup.item(...)` (BasePop::item) instead of
  `popup.dialogs().item(...)` spends budget even when it stays a call.
  For a deep closing-brace teardown with a shallow teardown laid out last,
  write `if (A) { ... } else { ...; return; }` (NetMsg::pop).

  One-expression inlines (is_human, map_tiles) spend nothing; large single
  branchy chains used once or twice may spend nothing either. What moved
  tech_achieved's five early sites was the *number* of cheap branchy
  expansions after the last early return (about two per site), independent
  of their position.

  Dead probes placed after all expansions still change earlier second-level
  decisions (window_proc). A probe with more than about four chained ifs is
  itself refused and emits calls; use many one-if probes.

  Dead calls in an always-false `if` measure a shortfall; use arguments
  that add no new literals: new literals in a canary unit flip other
  functions (do_checksums vs end_of_game).

  Teardown depth can follow the caller's written size rather than dead
  spend: replacing three written-out Buffer calls with one existing inline
  helper made a drawer's teardown one stage shallower where probes did not.

  Keep a dead probe (a static helper of chained `if (x == k) x = c;` called with
  a constant, which folds to zero code) only as `WORKAROUND(inline)` when no
  natural spelling is found.
- **Two destructor stages walking one inline body** (StringList's ~StringList
  and ~StringStruct): a budget covering one expansion picks the first stage;
  slightly less (about 4 probe units more spent) moves it to the final stage.
  Fix it with a natural spending helper, not a header change.
- **Early return sites shallower than ours in Console Popup functions**
  (editor_edit_veh, custom_my_faction) did not respond to probes: spend at
  the top only deepened the final teardown. Treat them as structural (which
  helper owns the Popup, how many return sites exist). Even brace formatting
  inside an inline helper changes its cost.
- **Hand-copied helper bodies.** Raw `has_treaty` tests or hand-inlined writers
  inflate each caller's own size, so every teardown expands one level too deep
  (call_off_vendetta had 48 states against 22). Restore the helper calls
  file-wide first.

Shared return teardowns: when several `~Popup` return sites share one
teardown copy in the image, the source had one `return` that the other sites
reached by `goto`, placed at the source position of the latest sharing site
(control_turn).

State order follows test order: in Popup functions whose rejection blocks the
image lays out far away, teardown states are numbered in the order of the
tests; write each block inline at its test (`if (cond) { ...; return; }`)
(tech_trade: 35 site groups became the image's 33). Same-TU getters
(get_title, get_name, defined before the caller) give early sites back
without changing their bytes.

Reading per-site depths: a refused measurement's diagnostic contains
`original=(…), compiled=(…)` predecessor tuples. Split each tuple at its `-1`
entries; the run lengths are the teardown depth of each site in state order.

```python
p = [int(x) for x in tuple_text.split(',')]
sites = []
for v in p[1:]:
    if v in (-1, 4294967295):
        sites.append(1)
    else:
        sites[-1] += 1
```

The StringList/StringStruct destructor chain is pinned by its exact
out-of-line copies (0x4066C0, 0x401060, 0x402970: `~X() { close(); }` with an
inline close). Moving those definitions in or out of the class, adding
`virtual`, or dropping MEASURED leaves every inline decision unchanged. When a
Popup caller expands ~StringList/~StringStruct where the image calls it, that
is the caller's budget (2-16 branchy dead probes reproduce the image:
draw_social at 8, pop_click at 2, top_menu at 14): look for the missing
natural spend. A census of 94 Popup-family functions found ours deeper than
the image in 26 and shallower in 2: recovered bodies under-spend, typically
raw field access where the original used inline accessors.

### 2.4 Linear subscripts in nested loops

- **Source shape.** Nested loops over a grid were written with a linear
  subscript of per-level counters: `arr[row*W + col]`, `((a*7+b)*7+c)*7+d`, or
  `arr[base + col]` with `base += W` in the outer for-clause.
- **What VC6 emits.** It carries one pointer through every level. An outer
  counter used only in the subscript has its test replaced by `ptr < &arr[H*W]`,
  a *signed* `jl` because the counter is `int`. Inner subscript-only counters
  become `dec/jne` down-counters or vanish.
- **Spellings that don't match.**
  - A running index (`arr[i++]`) gives a saved base per level.
  - A real `T*` gives `jb`.
  - `(int)p < (int)end` gives `jl` but swaps registers.
- **Column resets.** A grid loop that restarts its column variable in the outer
  increment clause (`…, ++row, x = C`) shows as `mov reg, C` just before the
  outer back-branch, and VC6 reuses that register as the literal C afterwards.
- **Two pointers vs one offset in copy loops.**
  `for (k = 0; k < N; k++, s++) Dst[k].f = s->f;` gives two pointer induction
  variables ending `cmp eax, &Dst[N]; jl`; `Dst[k].f = stage[k].f` with both
  sides indexed from constant addresses gives one byte offset ending
  `cmp eax, N*stride; jl`.
- **Small aggregate copies.** VC6 always splits an 8-byte struct assignment
  into scalar moves. A lea-based 2- or 3-element copy (`lea edx,[src+off];
  lea esi,[dst+off]; mov edi,[edx]; mov [esi],edi; ...`) is a fully unrolled
  constant-trip loop over an array member: write the loop, not a struct type.
  Index spellings (`(&s.a)[j]`, casts, a real array member) compile
  identically, so retyping a field for this alone is not a lever.
- **Finding candidates.** True linear-subscript cases are rare. A census of
  `cmp reg, <data address>; jl` outer back-edges with nested inner loops mostly
  turns up budget, register or missing-code differences. A global table that
  gives `jb` where the image has `jl` was one `int[N]` array declared as
  separate globals; it needs a declaration change (load_interface).
- **Increment order.** The for-clause order sets the order of increments.
  load_terrain became exact with this rule alone. SP3, VC5 and other `/O` flags
  all give the saved base, so this is a matter of source shape.
- **Which IV carries the test (2026-11-19).** VC6 replaces a counter's loop
  test only with an induction variable derived from that counter. A coordinate
  written `x = X0 + i * K` (or used inline) is derived and can become the test
  (`cmp x, end`); one stepped in the for-clause (`x += K`) is a basic IV and
  never does, so a pointer or the counter keeps the test. Image tests the
  coordinate and ours a pointer: derive it (load_box_sprites, load_vehicles'
  sheet_x). Image tests a pointer and ours the coordinate: step it in the
  for-clause (load_vehicles' border_y, 0x004546D7).
- **Per-iteration bindings.** `T &r = A[i];` or `T *p = &A[i];` in a loop body
  gives VC6 one more pointer to carry; the image indexes `A[i]` at each use and
  lets VC6 derive its own (read_rules' social effects: a pointer IV and a
  dec/jne counter became the image's two byte offsets, 59 -> 14 rows; eight
  more functions closer).
- **One counter for sibling loops.** Several non-nested `for (int i ...)` loops
  were often one function-scope `int i;` (game_data 298 -> 240,
  world_rainfall 311 -> 271, eight more).
- `cmp reg, K; jl` is `i < K`; `i <= K - 1` gives `cmp reg, K-1; jle`
  (ProdPicker::calculate).
- `recovery/tools/analysis/loopscan.py` lists the loop tests of both sides and
  the source loops with these shapes; `loopsweep.py --rule rebind|sibling|
  derive|basic` tries each rewrite site by site.

### 2.5 Unit coupling

- **The symbol counter modulo 64 (measured 2026-11-17).** Most "count"
  couplings below are one mechanism. VC6's front end numbers every symbol it
  creates in a TU from one counter; the `$SGn` literal labels are that
  number. Each function declaration and each extern variable adds 1 (names,
  lengths, kind and position before the function don't matter). Each string
  literal adds 1, and so does each local in an earlier function or earlier in
  the same function. Each struct type and each inline body adds several.
  Comments, blank lines and same-length renames add nothing. Some register and
  reload tie-breaks depend on that counter **modulo 64**, presumably through
  64 hash buckets keyed by symbol number:
  - reportif's on_iface_button_clicked repeats its exact pattern every 64
    shifts over 260 shifts, and console_commands' corner_the_market gives
    identical bytes at shifts 1, 65 and 129.
  - Shifting by declarations, by literals in an earlier function or by dummy
    locals in the function itself gives the same result for the same counter
    value.
  - The diffs are pure ties: on_iface_button_clicked reloads `[ebp+8]`
    (button) and `[ebp-8]` (this) in swapped order after
    `ReportWinGlobal.redraw()`; corner_the_market has `add ecx, edx; push ecx`
    (image) against `add edx, ecx; push edx`.

  At one revision, the function is exact when the residue of its first
  relocated `$SGn` (mod 64) is in a set. That set is not fixed across
  revisions (see the drift caveat below). The first sets measured
  (wip 2088402dcf, against each function's own exact bytes):

  | canary | residues |
  |---|---|
  | ReportIf::on_iface_button_clicked 0x004A6330 | 28, 39, 54 |
  | action_give 0x004CEDE0 | 32-35, 40-42 |
  | corner_the_market 0x004E2000 | 0-4, 7-9, 15-18, 27-45, 53-55 |
  | end_of_game 0x00524870 | 1-4, 6-19, 21-23, 27-34, 39-49, 52, 54-60, 62-63 |
  | buy_council_vote 0x0053EB50 | 0, 9, 12, 14-22, 25, 28-33, 35-42, 44-46, 48-55, 57-63 |

  Few functions are counter-sensitive: 2 of 234 in reportif (with sat_attack)
  and none of faction's 278 across 16 shifts. base_energy 0x004EB560 and
  give_a_base 0x0054CF40 are **not** counter-sensitive: 64 declaration
  probes, inline-body probes and struct-type probes leave them unchanged.
  Their flips come from the header content they use (field types, arrays).
  The absolute count is not recoverable: each function only constrains the
  original's count modulo 64, to its residue set, and the whole header stack
  (vc6_compat.h alone is 1,334 symbols, windows.h 16,655 lean or 41,878 full)
  only matters modulo 64. So pads are alignment, not missing declarations.
- **Pad sweeps do not fix general register ties (measured 2026-11-18).**
  Full 64-step `extern int` pad sweeps over about 33 units holding
  MNEMONIC_ONLY and register-cause functions flipped no non-exact function,
  and lost no claim. The counter governs only the listed canaries. Don't
  sweep pads to try to close an ordinary register tie; look for the spelling.
- **Re-tune by sweeping against the image.** Each canary unit has one pad
  block, and the last measured set lives beside it:

  ```
  // CANARY: 0x004A6330 ?on_iface_button_clicked@ReportIf@@UAEXH@Z residues 28,39,54
  // CANARY-PADS: function ReportIfDeclFnPad
  void ReportIfDeclFnPad0();
  ```

  `uv run recover canary --image <image> [<unit>...]` measures every canary
  against the image at the current pad count. When one misses, it predicts a
  count from the recorded sets (each pad moves the counter by 1) and measures
  the prediction as an overlay compile. `--write` publishes only a prediction
  that measures BYTE_EXACT; a stale set refuses and asks for a sweep.
  `--sweep` measures one full period (64 overlay compiles, about 5 minutes per
  unit) and picks the exact count nearest the current one. `--sweep --write`
  writes that count and the measured sets into the `CANARY` lines.

  After each header batch, run `recover canary --image <image> --sweep --write`,
  then `recover rebind-literals --write`. Every pad renumbers the `$SG`
  literals after the block, so the tool counts a body that matches with only
  stale literal bindings as exact. A unit with no pads keeps an empty block
  (`CANARY-PADS` with no pad lines) so the tool can add them.
- **Caveat: residue sets drift when other functions in the unit change.** The
  residue is read at the function's first literal, but the tie depends on
  more than that. Edits to the canary's body, to anything it expands inline,
  and to other functions in the same unit all move the set. Measured on
  0x004A6330 within a day: {28, 39, 54}, then {29-34, 40-45, 55-60} after a
  labs_line edit, then {0-27, 38, 49-53} after the next header batch. In the
  same batch action_give went from {32-35, 40-42} to {17-20, 25-27} and
  corner_the_market from {0-4, 7-9, 15-18, 27-45, 53-55} to
  {0-2, 15-45, 54-55, 58, 60-63}. A `--write` that trusted the stale set moved
  0x004A6330 from an exact count to an inexact one. Treat a recorded set as a
  prediction only, and recalibrate by sweep (above) after each batch.
- **Edits ripple through the unit.** Any edit can flip register or
  operand-order ties in a *later* function of the same TU, because VC6 shifts
  its symbol counter (above). Edits that do this include:
  - removing a redundant cast;
  - adding a file-local static inline;
  - leaving a dead file-local helper behind after a rewrite.

  Delete helpers a rewrite leaves unreferenced, avoid new helpers in canary
  units, and re-measure every claim in the TU.
- **Expansion count of a new helper.** In a canary unit, how many times a
  new file-local helper is *expanded* can flip a later function's tie while
  its definition alone is neutral (console_commands: 0-5 expansions kept
  corner_the_market exact, 6-7 flipped it). Before inventing a helper, look
  for an original function with the same shape and use its exact body
  (popup_set_write_check_bit fixed it).
- **Bistable block layout.** A large goto-target block's placement can flip
  when any new local identifier is added to a *different* function in the
  same unit (order_veh flipped from supply_options edits); reusing an
  existing dead local does not. Diff big near-misses before and after every
  edit elsewhere in their unit.
- **Header vetting depends on counts.** The number of member and declaration
  entries matters, and so does visibility. An unused inline body, or a newly
  visible plain struct, in a class with 30–56 includers flips the same ~12 large
  functions (3.6). Removing an unused body perturbs as much as adding one, and
  removing two unused declarations together can flip a canary that neither does
  alone. Comments, blank lines and same-length renames are neutral.
  Gate new bodies per TU (`#ifdef`, tagged `WORKAROUND(decl)`), and keep a
  header change only when the scoped `check-matches --since` shows gains and no
  losses.
- **Flipping a canary back.** When a rewrite elsewhere in a unit flips a
  canary, look for an equivalent spelling inside the rewritten function
  that compiles to the same code for that function (an if/else assignment
  folded into one `||` expression restored end_of_game).
- **Extern-name count** (the symbol counter modulo 64, above; function
  declarations count the same). Codegen in a TU depends on how many *distinct*
  extern variable names it declares; the names and their lengths don't
  matter, and repeated declarations and enumerators don't count (action_give
  stays exact with 6–7 PopupTheme* names, flips with 1–5; one extra name flips
  base_production, pop_click, sat_attack). When merging globals into an array,
  let the array declaration replace one old declaration and keep the rest as
  `WORKAROUND(decl)` declarations shadowed by element macros, then compare
  every TU's objects function by function (the IfaceColorTable change touched
  only load_interface).
- **One unit both expanding and calling a function** means the original was a
  header inline everywhere and the unit emits a pick-any copy; a strong
  definition elsewhere then gives LNK2005, so make that definition `inline`
  (COMDAT) too. A file-local twin of an inline member is not budget-neutral:
  it may expand where the member would be refused, and masked measurement
  hides wrong call targets at refused sites.
- **Budget probes are non-monotone** (12x1 and 6x2 matched where 8x2, 10x1
  and 16x1 did not): sweep chain size against count.
- **Pick-any copy without a caller:** put `MEASURED` on the class declaration
  and define the body `inline` in the owning unit (dllexport on the
  definition alone fails with C2375); units seeing the body emit their own
  COMDAT copies (StringStruct::current_entry).
- **A local initialised from a dead parameter** is coalesced into that
  parameter's slot; the image's fresh slot needs the parameter to stay live.
- **Moving an out-of-line member body into its header:** guard it per unit
  (`#ifdef X_INLINE` around the body, the plain declaration in `#else`); the
  unit emitting the copy defines the guard too and the copy stays exact.
  Unguarded, Font::line_advance in font.h (148 includers) flipped 29
  functions in 21 units; guarded, only the three target units changed.
- **Turning a header inline into a call per unit.** An explicit `inline`
  member defined at the end of the unit, after its callers, stays a call
  there and is emitted as a pick-any COMDAT (a non-inline definition gives
  LNK2005). Check every function measured in that unit's compile context:
  destructor copies it emits may need an inline twin of the same body.
- **Member count counts declarations** (the symbol counter, above). Turning N data members into one
  struct member perturbs includers; adding back exactly the removed number
  of member declarations of any kind restores identical objects
  (multiwin.h bounds_).
- **Swapping one declaration for one is safe.** Replacing a free declaration
  with a member declaration in win.h (142 includers) changed no function in
  any other unit; the flips above come from adding or removing declarations.
- **Every compiled unit needs a measurement recipe.** `recovery/project.json`
  sets `require_unit_flag_sets`: a new unit must get its production flags in
  `measurement.unit_flag_sets` or every recover command refuses. (Before
  2026-11-12, unpinned units were measured under both global trials and the
  better score kept, so a /Oy- unit could be reported from a frameless compile.)
- **The extern-name count is a window, not a threshold.** Superseded: the
  window is the counter residue set modulo 64 (above), so it recurs every 64
  and moves with every earlier symbol. Use `recover canary` instead of
  sweeping 0-24 extra names.
- **Include position.** A header's position in the include order can flip ties
  even when it arrives later anyway. Dropping an unused direct include is a
  faithful lever.
- **Literals after a header change.** Any header change that shifts counters
  renumbers `$SG` literals. Run `recover rebind-literals --since <rev> --write`
  before judging a loss.
- **TU splits.** Before splitting a TU, find where implicit/COMDAT-only
  constructors and destructors sit. The image keeps one copy, emitted by the
  unit whose atexit handler needs it (the ReportIf split was disproved this way).

### 2.6 Other confirmed rules

- **.bss order follows symbol names.** VC6 orders a unit's uninitialised
  globals in .bss by symbol name (a hash or collation), not by declaration
  or definition order. With popup_api's four names fixed, moving declarations
  in headers or definitions to either end of the file left the offsets
  unchanged. Renaming one object moved it. The image's .bss order is
  therefore a weak hint about original names. Do not rename to match it
  without other evidence.
- **Shared frame slots.** Counters that share one frame slot in the image were
  one function-scope variable; declaration order cannot reproduce the slots.
  This holds across mutually exclusive sections of one function too: walk and
  id variables of different sections sharing a slot were one reused variable
  (prune_protos moved 135 instructions).
- **Nested call results.** A nested cdecl call made before the outer call's
  constant pushes was stored to a local first.
- **Literal order is code-generation order.** VC6 emits string literals in
  statement order, call arguments right to left. If the image's literals of a
  rebuild path precede a load path laid out first, the source had the
  rebuild block first as a label target: `if (flag) { label: ... } else {
  ...; goto label; }` (colortables_init).
- **An unreferenced literal** inside a unit's unpooled literal run, between
  referenced neighbours, is source text in a branch VC6 folded away: evidence
  that the text was written out at that site, not in a helper.
- **`label_get(c ? a : b)`** compiles to setl/dec/and/add index arithmetic;
  two branch loads joined before one get call is
  `StringTable.get(c ? Labels.strings_ptr[a] : Labels.strings_ptr[b])`.
- **`cmp reg,1; jle` back-edges** come from `j <= 1`; `j < 2` gives `cmp 2; jl`.
- **A byte test of `enable_mask_` against `1 << f`** is `is_human(f)`
  (human_mask_ is byte 0 of that union).
- **Writable fixed-stride string slots.** strcat sources at a fixed stride
  (e.g. 0x18) with zero padding, in .data before the unit's other globals,
  are a writable `char[N][stride]` table, not literals (load_music's ambience
  names); pooled literals would let VC6 cross-jump identical arms.
- **One literal address pushed several times** in an unpooled image function
  is a named global string object (SocialWinColonSpaceText, BufferJoinSpace),
  not a literal. Conversely, identical literals the image keeps separate but
  our pooled recipe merges make strict verification refuse a masked-exact
  function; that unit needs the unpooled recipe.
- **Materialised compare after a counting loop** (`xor; cmp; setl; test; je`
  with the count in a dead parameter slot) is an inline helper taking the
  counter by reference and returning the comparison.
- **Shared final `return 0`.** When the image's last `if (x) return 0;` is a
  `jne` into an earlier `pop...ret` copy, write `if (!x) return v; return 0;`:
  every `return 0` then cross-jumps to one epilogue (veh_draw lost 21
  instructions).
- **Clamp stored before tested.** `f = expr; f = max(0, min(N - 1, (int)f));`
  reproduces the image's store-then-overwrite.
- **Fix a unit's literal count as a whole.** Removing a duplicated literal in
  a canary unit flips other functions (shoot_it vs action_give) even when
  the image has fewer literals.
- **Strict refusal with a masked BYTE_EXACT.** First check for `$SG`
  relocations without a `RECOVERY_LITERAL` binding (seed them from the image's
  push operands); import bindings are global, so a second copy is an error.
  "strict selected evidence
  result: unknown" while `masked_verdict` is BYTE_EXACT usually means a system
  import relocation lacks its `RECOVERY_IMPORT` binding; functions with
  `RECOVERY_ASSOCIATED` always take the strict path. Also check
  `rebind-literals` first (renumbered `$SG`).
- **WndProc shape.** `and eax,0xffff` on a return (MAKELRESULT), arguments
  rebuilt from WORDs (FORWARD_WM_*), and x/y kept in incoming argument slots
  mean a windowsx.h `HANDLE_MSG` switch; the handlers are the unreferenced
  statics right after the procedure.
- **Interlocked counters read through pointers.** If the image reads a
  ring cursor through `lea reg,[this+off]; mov eax,[reg]` at each use and
  loads a counter before the slot-index divide, the counter is volatile
  (sub_5ca230 became exact). A `push ecx` frame slot in a fastcall
  function can be the register parameter spilled under pressure.
- **Walking output pointer.** `lea eax,[edx+4]` then `add eax,4` steps with
  stores at negative offsets is `*out++ = ...` with the increments hoisted.
- **Struct fields vs separate globals.** Through a global struct (directly or
  via a pointer) VC6 orders a join's reloads of its fields differently from
  separate objects. If the image reloads them in the other order, the
  original had separate globals; one local reference per field reproduces it
  (sub_5d2e70 became exact).
- **A /G6 sub-range.** Within a /G6 band, measure neighbours with and without
  /G6: functions that match only without it belong to another unit
  (sub_5d2e70/sub_5d35b0 moved out of movie_media_decoder_g6.cpp).
- **Return-type corrections and identity.** Changing `int` (no value) to
  `void` leaves the object code identical. When a mangled name changes, keep
  the historical name on the ORIGINAL line and add or update a `// symbol`
  line with the new one; measure, record and check-claims follow it.
- **Deferred register saves.** When the image saves only the registers it
  needs before an early `return K` and pushes the rest after the guard, the
  rest of the source reached one return: end the arms in a single
  `return x;` with a result variable (VC6 still tail-duplicates the
  epilogue). A return inside an arm puts every push in the prologue.
- **A destructor that does not store its own vptrs at entry** is
  compiler-generated; a user-declared empty destructor always stores them.
- **`if (++p->member == n)` through a pointer** gives `inc edx; mov eax, edx;
  mov [member], edx; cmp eax, ...`; local forms do not.
- **Fill loops.** A plain `for` fill of 0/1 compiles to `rep stosd`, so
  `rep stosd` alone does not prove a memset.
- **`extern "C"` loses EH.** Under /GX an `extern "C"` definition with
  destructible locals compiles with no EH frame; if the image has an fs:[0]
  frame, the original had C++ linkage.
- **A header-inline copy emitted from another unit** that calls a member the
  measured unit defines: wrap that member in `#pragma auto_inline(off)` in
  the measured unit, when the copy is its only caller there (~StringBox).
- **Missing `void`.** An `int` function with a separate epilogue at each exit and
  no zero register was `void`. The catalogue's mangled return type can be wrong.
- **StringList locals.** In drawer functions they call
  `StringStruct::StringStruct` out of line (0x401000), while class-member lists
  expand it. Drawer TUs need `STRINGSTRUCT_CTOR_OUT_OF_LINE`.
- **Header-inline copies.** An out-of-line copy is compiled with the emitting
  unit's flags; use `#pragma optimize("y", off)` around a guarded header
  definition when the host unit's `/Oy` differs. A `MEASURED` (dllexport) inline
  is emitted into every includer and perturbs them; prefer plain `inline` when
  the owner emits the copy through a real call.
- **Tail merging needs `continue`.** In `while (1) { switch (...) {...} }`
  identical case tails ending in `break;` are not cross-jumped; ending in
  `continue;` they are (parse_string).
- **Virtual-base Buffer members.** `Buffer::f()` on `this` has no null check;
  binding `Buffer &b = *this` or passing `*this` to a free helper adds one.
  When the image's inline body is a Buffer member, give it a per-TU-guarded
  Buffer member rather than a free helper.
- **Tail merging follows source form.** VC6 cross-jumps tails that are
  identical in its internal form, not in bytes. When the image keeps identical
  tails separate, spell each differently (`a = b = X;` vs `a = X; b = a;`, or
  statement order); the bytes stay the same and the copies stop merging.
- **Frame too large by N×4.** If the image shows unrelated values sharing a
  slot, move setup-only locals into the mutually exclusive block that uses them
  so they overlap (sub_5cc940's frame became exact).
- **Tail duplication.** VC6 copies a small shared `fail:` tail into a
  predecessor only when that predecessor ends in an unconditional `goto`; a
  conditional `if (x) goto fail;` jumps to the shared copy. For an image's
  private copy with the status store after the receiver set-up, write
  `{ const int r = f(); if (r) { status = r; goto fail; } }` (Video::load).
- **List-node allocation template** (StringStruct, ServiceProviderList,
  SpriteList, JackalVoiceRx add): entries `allocator_ ? new(allocator_) T :
  new(owner) T`; payloads `if (allocator_) x = new(allocator_) P; else x =
  new(owner) P;`. Signature: per-arm EH flag bits in a frame local
  (`or al,N`) and paired cleanup funclets calling placement delete
  0x00402320. A class-scope operator delete hides a global placement delete
  (no cleanup state); `::new(args)` finds it. In cleanup funclets an lvalue
  placement argument is re-read, an rvalue (inline function result) spilled.
- **`new (std::nothrow) X` drops the EH frame.** A `push -1/handler/fs:[0]`
  frame around an image `new X` that ours lacks is plain global `new X`.
- **Signed compares on masked high bytes.** `(x & 0xFF000000) < K` compiles
  unsigned (jae); the image's `and; cmp; jge` is `(x >> 24) < k`,
  `(x & ~0xFFFFFF) < K` or an int cast of the masked value.
- **Invented intermediate bases.** If our constructor stores an extra derived
  vftable stage the image lacks, or a generated deleting destructor calls an
  empty `??1`, the class has an invented intermediate base: derive from the
  real base directly (ListPayloadBase).
- **Moving a member's definition to another unit:** leave an `inline` twin in
  the original unit if callers there expanded it.
- **Class operator new.** A dead store of 0 to a frame local, with no EH
  frame, beside `new` of classes with virtual bases is the construction flag
  of a class `operator new` without a matching placement delete. Global
  `::new(p)` brings a full EH frame back (VC6 `<new>` declares
  `operator delete(void*, void*)`); a non-placement global operator new with
  extra arguments stops inlining the virtual-base constructor.
- **Sprite draw overload.** When every `Sprite::draw(this, s.transparent_index(),
  x, y)` site forms `lea ecx, sprite` before reading `[ecx+8]`, the original
  called an inline draw overload on the sprite that reads its own index.
- **Ten distinct 0x50-byte frame buffers**, one per `_itoa` + strcat, are ten
  expansions of the `say_num(int)` header inline; write say_num per site.
- **`static Sprite *const p = &g_X;`** is not constant-folded: it emits a
  data object and a load. Where the image pushes the address, write `&g_X`.
- **Unwind-only destructors.** A destructor whose only image caller is an
  unwind funclet, with its copy inside a TU's range, is a header-inline
  COMDAT; defining it in its own TU blocks every expansion (~UV2Player).
- **Throwing CRT calls.** VC6 treats CRT `memset` as throwing and a same-TU
  callee without throwing calls as nothrow, which shows in constructor unwind
  states.
- **Block layout.**
  - VC6 lays out blocks from the CFG, so only swapping arms (a negated
    condition) moves layout.
  - Else branches are queued and emitted at the next unconditional jump.
  - Goto-only blocks move next to their jump target.
  - Unreferenced labels change layout.
  - `while(1){ if(x) break; … }` is the only non-rotated loop.
  - `default:` is placed last. VC6 drops an empty `case N: break;` at the top
    of a jump-table range; it does not raise the table bound.
  - `cmp; je A; push x; jmp J; A: push C; J: call` around one shared call is
    `if (x) f(x, ...); else f(C, ...);`; `x ? x : C` gives `jne; mov eax, C`.
  - Only `if (C) {T} else {E; if (!d) {H; goto past;}} J; past:` defers E
    past the next unconditional jump (AlphaMenu::check).
  - A flag tested as `if (avail) A else U` lays out differently from every
    `||` or negated form (which compile identically); the order of unpooled
    literals in the image tells which arm came first in the source.
  - A block reached only by gotos was usually written inline at its *first*
    unconditional goto in source order (order_veh's 59512c block).
  - A loop whose bottom repeats its top guard and jumps past it (`init; je
    body; jmp fail`) was a `goto label` back to a label before the guard;
    an `exit(N)` that falls into the next block is `exit(N); break;`
    inside `while (1) { if (x) break; ... }`.
  - The image's `je target` over a fall-through arm ending in `goto` is
    `if (x != k) { A; goto J; }` followed by the target label.
  - One label per join: two adjacent labels where the second is reached
    only by a backward goto skipping a reload push the return sites between
    them out of line (probe). Write an else arm as `else`, not as a goto to a
    block elsewhere; keep if-chains whole (`else if (x == 2) {...}`).
  - A labeled block reached by fall-through and by later gotos is placed
    after the block holding the last goto, splitting it; a small labeled
    tail is duplicated into the fall-through instead.
  - A goto target is placed after the *last* goto that reaches it, whatever
    the source order. For an image block that an earlier arm falls into and
    later arms jump back to, have the later arms set a flag the join tests
    (`if (!flag) return;`); VC6 threads the jump (council_action).
  - A switch whose result lives in memory with `mov [ebp-4],X; ...; ret`
    sites is `score = X; break;` with one final `return score;` (VC6
    tail-duplicates that epilogue).
  - In a jump-table switch, only case source order moves case blocks;
    rewriting `break`/`goto`/`default` structure compiles identically. A case
    table read straight from the image also catches swapped case bodies
    (CouncWin::draw_leader had five label cases reversed).
  - Switch cases follow source order; out-of-order cases make strict measurement
    refuse with a jump-table cell diagnostic.

### 2.7 Callee-saved register allocation model

Measured 2026-11-18 with about 9,000 generated probe functions
(`recovery/tools/analysis/regalloc/`, flags of a game unit). Each rule names
its probe family; rerun with `python3 run.py <file>` or
`python3 thresh.py g_<family>.cpp` in that directory. Use
`recovery/tools/analysis/regmap.py <unit> <0xADDR>` to see, for the image and
for ours, which role each callee-saved register holds and what the other side
has in the same rows. Confidence: **high** = holds in every probe and in a real
function, **medium** = holds in every probe, **low** = partial.

Candidates and order

1. **The candidates are live ranges (webs), not variables** (high). A variable
   redefined after its last read is a new candidate (`redef` in g_factors2);
   two variables with disjoint ranges reuse one register (`disjoint3`,
   `stagger` in g_split). Declaration order and position have no effect; only
   references do (`c_2222d`, `decl`).
2. **Registers go out in the order esi, edi, ebx, and ebp fourth under /Oy**
   (high; p01_count, both flag sets). Candidates are served in priority order
   and each takes the first of these that no interfering, already served
   candidate holds. So the register *name* tells the rank: a role in ebx in the
   image and edi in ours means one more candidate outranks it and overlaps it
   in the image (or one fewer in ours).
3. **Priority is a weighted reference count** (high for the shapes below).
   Definitions and reads count alike; every textual occurrence counts
   (`garr[d] + d` is two); push arguments, compares, indexes, arithmetic and
   global stores count the same (g_factors). A parameter gets no credit for
   its entry value and no penalty for being a parameter; `this` counts its
   prologue copy as one reference in the entry block (g_factors, g_factors2,
   g_parpos, `this3`/`this4` in g_temps). char and short count like int.
4. **A reference weighs the number of candidates referenced in its block**
   (high, 2026-11-18 par-1; `regalloc/homes.py 40 cd < blockn.spec`, about 60
   thresholds, both flag sets). Priority = sum over the candidate's
   references of `n_b * 2^depth(b)`, where `n_b` is the number of *distinct*
   candidates referenced anywhere in that basic block (the candidate itself
   included; how often each is referenced does not matter) and `depth` the loop
   depth. Calls do not end a block; every branch target and join does, so
   `&&`, `||`, `?:` and each if/else arm are blocks of their own.
   - What counts in `n_b`: locals, parameters and `this` with at least two
     references, compiler temporaries (an address CSE such as
     `gobj[i].f1 .. f3` counts as one more), and a constant used several times
     (`use_a(7)` five times counts). Not counted: a parameter read once,
     globals, a candidate that is only live through the block (`live_p` against
     `ref_p`).
   - A `switch` case block counts half. The first block of `for (;;)` counts
     double against `while (1)` (same code, different weight). A
     `for (n = 0; ...)` body references the counter too, so it has `n_b` one
     higher than the same `while` body.
   - This one rule replaces the old position table and rule 6. The old table
     (entry and last block 1, other blocks 1/4, loop 1/2, for body 3/4) was
     measured with p, q, c, d all referenced in the entry and last blocks
     (`n` = 4) against blocks that reference d alone (`n` = 1, times 2 in a
     loop). The "4/3 advantage of a block-local candidate" was a block with
     four candidates against an entry block with three. "Arm locals take esi
     ahead of a parameter with more references" is a parameter whose blocks
     reference nothing else.
   - Worked check, no other variables: `c` defined with K uses, `if`, then a
     local `d` with eight uses, then a final use of `c`. Entry `{c}` = 1, last
     block `{c,d}` = 2: `d` scores 18, `c` scores K+1+2; `c` takes esi from
     K = 16 (measured 16). Add a parameter read twice in the entry block: entry
     = 2, `c` = 2(K+1)+2, threshold 8 (measured 8). Put those reads in the last
     block instead: last = 3, threshold 24 (measured 24).
5. **Ties: there are few; a live-through penalty decides most** (high,
   2026-11-19 par-2; "Tie-breaking" below). The "later, shorter candidate" of
   the multi-block probes was rule 4b, not a tie-break.
6. **Register names follow the same priority with or without contention**
   (high). Two candidates and three free registers still get esi and edi in
   rule-4 order (blockn.spec `N*`, `L_*`, `W_*`), so a role in edi in the image
   and esi in ours is a priority question even when nothing is spilled.
   Source levers, in order of how often they move a real function: a
   reference moved into a block with more (or fewer) candidates; a candidate's
   first definition moved out of a test block (FlatButton::on_redraw: testing
   the field itself and assigning `state` only in the in-range arm gave the
   image's ebp/ebx order, exact); a block split or merged by `&&`/`?:`; a value
   reread from its source instead of a named copy.

Tie-breaking (2026-11-19 par-2; `regalloc/ties.py`, `tiegen.py`, `tiemodel.py`)

Measured with one probe function per translation unit, so the symbol counter,
the function's position and the earlier functions are controlled. About 3,000
probes: hand families varying one factor at a time, plus 1,800 random
straight/if/else/while programs whose two candidates tie exactly under rule 4,
each checked by adding one reference to either side.
The score of `tiemodel.py` predicts 1,775 of the 1,776 checked random ties
(two, three and four candidates, if/else and while); the miss is register
reuse by a candidate that does not interfere. It also predicts all 19
non-constant blockn.spec thresholds, where plain rule 4 gets 15 (N3, loopd,
loop2d and armd were the "ties"). Rerun: `python3 ties.py ties.spec --n 1-30`.

4b. **Live-through penalty** (high). A candidate loses half the rule-4 weight
   of every block it is live *through* without being referenced, if any other
   candidate is referenced in that block:
   `score = sum over refs n_b * 2^depth - 1/2 * sum over live-through blocks n_b * 2^depth`.
   Blocks where nothing is referenced cost nothing. Parameters pay it like
   locals. It is part of the score, not a tie-break, so it can overturn a
   lead of several references (8 in `L8`). Example: four `if` blocks each referencing `d`
   once, with `c` live through them, cost `c` 2. With the blocks inside loops
   (weight 2) and `d` gaining 2 per loop, `c` needs 3H extra references, not 2H.
   Measured with hole blocks holding 1, 2 or 3 references of `d` (penalty
   independent of the count), and with 1, 2 or 3 candidates referenced (penalty
   scales with n_b). Families `h_*` and `L*` in ties.spec; the random sets
   come from `tiegen.py`.
4c. **Exact ties of the full score** (high). Adding or removing one reference
   always decides (no epsilon; the B/T ladders over 1 to 16 blocks). An exact
   tie goes to the candidate whose first reference is in the **later block**
   (`S7`, `B4`);
   in the same block, to the **earlier first reference** (definition order for
   locals, first read for parameters; `int` declarations, parameter position,
   names and loop-free code between them do not matter; `cd4`, `dc4`, `decl`,
   `par_first_read`).
4d. **The symbol counter does not touch callee-saved ranking** (high). 64-pad
   sweeps of three tie families, 0-40 earlier functions, renamed locals and
   three scratch-register probes gave one result each. The counter canaries
   are other ties: at failing pads buy_council_vote 0x0053EB50 merges one
   `mov edx,[ebp+0x10]` block tail into its jump target and end_of_game
   0x00524870 compares `cmp esi,[mem]` instead of loading `eax`, and drops
   a cached `ecx` reload (block layout and peephole, not register ranking).
   Pad sweeps cannot fix a callee-saved register swap.

Using it (regprio.py uses 4b and 4c by default; `--model old` is rule 4 only):

- A pair swapped between the image and ours is either an exact tie of the
  full score (regprio prints `TIE` and which order 4c picks) or a near tie
  (`NEAR TIE`, with each side's live-through blocks). Levers, by how they
  act:
  - start block (4c): the winner of an exact tie starts later. Defining a
    variable after the branch that precedes its first use, or a parameter's
    first read after it, gives it the tie.
  - first reference in one block (4c): the earlier definition wins, so swap
    two adjacent definitions.
  - live-through penalty (4b): a variable kept across a stretch where other
    candidates work pays 1/2 per such block. Rereading the value from its
    source (a member, a global, a parameter) after the stretch ends the range
    and removes the penalty; one reference inside the stretch removes it in
    that block.
  - otherwise one reference in a heavy block, as before.
- Pair accuracy of regprio on 90 exact functions (two per unit, 0x120-0x600
  bytes): 250 of 413 interfering register pairs with rule 4 alone, 260 of 412
  with 4b/4c. The remaining misses are candidate-set errors (temporaries,
  `this`, cached pieces), not tie order.

Losers: memory home, cached pieces, write-back

7. **A candidate needs one register free over its whole range** (high). It
   never takes a register through another candidate's idle stretch: a winner
   keeps its register across loops in which it is not referenced (`hole_*` in
   g_split). This is why ours can hold a variable in esi over a long stretch
   while the image keeps it in memory: in the image some higher candidate
   overlaps it on each of the three registers.
8. **A loser lives in its frame slot and its reads are cached as new, short
   candidates** (high; g_cache, g_split). A cached copy needs at least two
   references; it is loaded at the first of them; it survives calls; it is
   carried through ifs when every path has it, is reloaded at each loop
   iteration and after a loop (`loop_w`, `loop_multi`, `dead_before`), and it
   takes the first register free over its own stretch. These pieces compete by
   the same priority with every other candidate, including whole variables
   defined later (`cache_vs_web2`: a cached parameter with nine reads takes the
   register from a local with four). This is the image's "kept in memory and
   cached block by block".
9. **Register with write-back is the loser's piece before the winner starts**
   (high; `par4_cd_k1` in g_factors, `short*` in g_rangelen). When the winner's
   range begins later (a parameter first read late, a local defined late), the
   loser holds the register from its definition up to that point and every
   definition is also stored (`mov ebx, eax; mov [slot], ebx`); after it the
   loser is read from memory.
10. **Not candidates** (high): address-taken locals (`addr`), globals and
    members across calls (`global`, `this_field`), a value consumed once from
    eax (`def_use`), a constant initialiser that propagates (`const`). An
    index used only as `table[i].field` is replaced by one address candidate
    that counts the field reads (`cse_addr`), so the index stops competing.

Using it

- Run regmap. If one role sits in a different register on the two sides and
  nothing else moves, compare counts: the image's higher-ranked role needs more
  weighted references than the other, or the first reference.
- Levers that change a count without changing the code: one variable split in
  two where the second starts as the first dies (the second inherits the
  register; parse_string 0x00625880, 129 rows to exact with
  `LPSTR p = var + 2;` for the alternative scan, which dropped `var` below
  `output`); the reverse merge; a value read from its source again instead of
  a named copy; `while (1)` against `for (;;)`; a reference moved between the
  entry block and a later block (statement order across the first branch).
- If the image keeps a variable in memory with cached pieces and ours gives it
  a register (or the reverse), count the candidates that overlap it on each
  register. A constant stored several times (`mov ebx, 6; mov [order], ebx`)
  is a piece of the stored variable, not a constant temporary.

Reading a function with regprio.py (2026-11-19 par-1)

`recovery/tools/analysis/regprio.py <unit> <0xADDR>` applies rule 4 to our
source (blocks and loops from our listing, webs by reaching definitions) and
lists, for every web, the predicted register, ours and the image's, then the
interfering pairs the image orders differently from ours with the blocks that
feed each side. It is a screen: on 60-odd exact functions it orders 62-64% of
interfering register pairs right. The misses teach what the source count does
not see, and those are the levers:

- **An in-place coordinate wrap is three extra references.** `int x = e; if
  (!(MapIsFlat & 1)) { if (x < 0) x += B; else if (x >= B) x -= B; }` defines
  and reads `x` in every arm; the image wraps a temporary and stores each arm
  to `x` (`lea edi,[eax+edx]` / `mov edi,eax`), the shape of `int x =
  wrap_map_x(e);`. The extra references reorder the loop's registers (high,
  three functions): air_power 1639 -> 1272, BaseWin::farm_click 291 -> 269
  (esi score / edi tile_x as the image), BaseWin::draw_farm 413 -> 404.
  `grep -n "+= MapLongitudeBounds"` lists the remaining in-place wraps;
  MapWin::editor, gen_terrain_poly and pixel_to_tile (mapwin.cpp) and
  diplomacy_check (one site, 914 -> 908) are untested or small.
  Tree-wide sweep (2026-11-19 par-1): gen_terrain_poly 1666 -> 1565,
  world_rivers 286 -> 271, energy_yield 301 -> 297, diplomacy_check 914 ->
  908; opticenter/monocenter/find_view/draw_vehicles/draw_legend stay exact.
  A frame or register copy of `MapIsFlat & 1` (`mov [ebp-x], eax` after
  `and eax, 1`, then `test`/`jne` on it per iteration) is VC6's
  loop-invariant hoist, not a `const int flat` source local: the planwin
  scans are exact with wrap_map_x and no flat local, and world_rivers gains
  only when the local goes. Not every written-out wrap is the helper: the
  image adds the bounds to the variable itself (`add ecx, ebx` on near_x,
  no separate result register) in diplomacy_check's second radius loop,
  draw_farm's 21-tile loop (hoisted `longitude` local) and alien_move's
  first neighbour wrap (bounds loaded before the flat test); there
  wrap_map_x costs 6-50 rows. Test each site.
- **Hoisted loop-invariant globals are candidates.** In can_terraform's
  radius census the image hoists MapLongitudeBounds (four references per
  iteration) into edi and rereads MapLongitude; the statement form of the
  private wrap helper (a static function /Ob2 expands) hoisted MapLongitude
  instead. The flat-first ternary body gives the image's map (336 -> 325).
  The same body change in course_xrange/territory_xrange moves nothing: the
  form matters where the wrap's arms reach the caller's variable or invariant.
- **Compiler temporaries the source count misses.** A base-class `this`
  (`this+0x444`, the Buffer of a Win; FlatButton::on_redraw holds it in edi
  with 21 uses), an address CSE that replaces an index (`&rows_[slot].field`
  in esi, NetWin::on_button_clicked), a strength-reduced loop pointer, and a
  global cached across a block. regprio prints them as `T:` webs; when the
  model agrees with the image and ours does not, look there first.
- **A constant can be a memory-homed loser.** MultiWin::draw_player: ours
  computes `1 << faction_id` as `mov ecx,1; mov [ebp+0x10],ecx; mov ecx,ebx;
  mov ebx,[ebp+0x10]; shl ebx,cl`, the constant 1 parked in a dead parameter
  slot, and that splits faction_id into pieces; the image has `mov edx,1`.
  Same instruction-level reference counts on both sides (this 7, faction_id
  18), so the difference is a candidate or block weight we do not see. Nested
  ifs, a named bit, declaration moves and `height` scope do not move it.
- **Exact ties** (rule 5, now "Tie-breaking" 4b/4c): success_rates is morale 17 against
  diff_modifier 17. In the image diff_modifier is a loser whose first piece is
  coalesced with success_rate (edi) and whose second read comes from memory;
  ours keeps diff_modifier in ebx. `int success_rate = diff_modifier;` before
  the test (53 -> 52), an early return, the clamp after the string clear and
  the assignment first in the arm do not flip it.
- **Structure seen through regprio, not yet a register fix.** BaseWin::draw_pop
  forms `&table[index]` (`imul r,r,0x2c; add r,table; mov al,[r+8]; mov
  ecx,r`) at all four sprite draws; ours indexes `[r+table]`. A named pointer
  or an inline helper gives the pointer form but mood still loses ebx to the
  BaseCurrent copy (248 -> 257..272); the loop top in the image is a separate
  reload block (`jle exit; jmp body; reload: mov ebx,[BaseCurrent]`) that ours
  lacks. base_production's morale/BaseIDCurrentSelected (esi/edi swapped),
  set_time_controls and vehdraw_construct_chassis (faction_id winner in ours,
  armor_grade in the image; the model agrees with the image) did not move
  under reference-local, loop-form or clamp spellings.

Constant candidates and phantom registers (2026-11-18 par-1, standalone probes)

- A constant with several references is a candidate (rule 4) and can win a
  register *name* that then holds nothing: the prologue saves the register and
  the body never uses it, or the next candidate skips a name. Freeing a
  destructor's string table (`if (p) { free(p); p = 0; }` twice in a 64-step
  loop) gives esi = pointer, edi = phantom, then the rest; with nonzero
  constants the phantom moves to ebx. In ~PullDown 0x005F88A0 (/Oy) the image
  is pointer esi, phantom edi, `this` ebx, counter ebp; ours ranks `this`
  above the phantom (`this` edi, phantom ebx). Stand-alone copy: the tail
  `dirty = 1; f1 = g1; f2 = g2;` after the loop gives ours; any two of the
  three tail stores give the image's order. No natural spelling with all three
  found (order, struct copy, inline reset, member destructor, `while`/`do`).
- A loser constant still forms short pieces: two adjacent zero stores in one
  block always become `xor eax,eax` + two register stores (both flag sets). The
  image's immediates mean the zero *won* a callee-saved register (and was then
  not materialised). Credits::unload 0x00428830 and Credits::exec 0x004289F0:
  under /Oy the zero wins ebp and the stores are immediates (a
  `#pragma optimize("y", on)` copy is exact); under the unit's /Oy- it loses.
  Exec's frame proves /Oy- for exec, so its source must give the zero more
  weight or one rival less; not found.

What the model does not predict

- success_rates 0x0059EEE0: the image enregisters `morale` (ebx; its home
  slot 0xc is reused for a has_fac temporary), ours `diff_modifier`. Rule 4
  with the constant 1 counted in the clamp blocks predicts the image, so our
  candidate set differs somewhere not visible in the listing; ternary, `max`,
  `<= 0`, reordered arm and a named `morale / 2` do not move it.
- can_terraform 0x00565320: the image holds `order` in ebx as a written-back
  piece and keeps `solar_tech` in memory (`mov ebx,eax; mov [ebp-0x2c],ebx`,
  a rule-9 piece; hence the 0x44 frame); ours gives `solar_tech` the
  register. regprio: order 43 against solar_tech 17, so the model agrees with
  the image; the census-loop registers are fixed (see regprio above).
- MultiWin::draw_player 0x00478630: image `this` ebx, `faction_id` edi (from
  its first read), ours the reverse with `faction_id` reloaded in each arm and
  the constant 1 spilled around a shift. Rule 4 by raw count favours the
  image; the gap is again a candidate set we cannot see.
- PullDown::compute_length 0x005F9D30 (/Oy): the image evaluates the item
  sum as `2*width("  ")`, `width(text)`, `width(right_text)` in the first
  max operand and the other order in the re-evaluated one, holding the partial
  sum in ebp (evicting `this`). All twelve operand orders and groupings of the
  sum compile identically (VC6 orders commutative call operands itself), so
  the lever is not the spelling of the sum.
- set_time_controls 0x00523C60: the image keeps `active_factions` as a
  write-back piece in ebx (it is the zero that initialises the other two
  accumulators) and runs the vehicle loop in scratch registers; ours puts
  the vehicle counter in edi. Declaration order, chained zeroing and the
  increment's position do not move it.
- Residues that look like register ties but are not callee-saved
  allocation: `lea r,[edx+eax]` against `[eax+edx]` in the wrap of a hoisted
  MapLongitudeBounds (action_airdrop 0x004CC360, planet_busting 0x00500B60;
  `MapLongitudeBounds + x` in wrap_map_x changes nothing tree-wide);
  `cmp r,ebx` against `test r,r` with a zero-initialised local in ebx
  (Console::zoom 0x005150D0); operand load order of two members or a member
  and a local (ListBox::attach 0x0060A670, MapWin::draw_base_names 0x00467DE0:
  operand swaps and casts compile identically); push scheduling of
  `veh_drop(veh_lift(id), x, y)` (enemy_planet_buster 0x0056A8A0); a fat
  member-pointer call standing in for a budget-refused inline (MapWin::gen_radius
  0x00469190, see 2.1).

### 2.8 /Ob2 inlining model

Measured 2026-11-19 (par-4) with generated probe families in
`recovery/tools/analysis/inline/` (game unit flags with /GX, see its README).
Each rule names the script that reproduces it. Confidence: **high** = holds in
every probe family, **medium** = holds in the families but is not confirmed in
a real function, **low** = partial. Use
`recovery/tools/analysis/inlinemap.py <unit> <0xADDR>` to see every site of
an inline callee, expanded (E) or called (c), image against ours.

Unit: costs below are in "if units", where one unit is
`if (ga[i] == 1000) mk(k);` inside a callee. Real sizes are IL node counts that
we cannot see, so the numbers are fitted within about 0.3 unit.

1. **Two passes, each in source order** (high; `level1_cutoff.py`,
   `nested_breadth.py`, `dtor_sites.py`). Pass 1 decides every call site in the
   caller's own body. That includes the destructor calls the front end writes
   at each scope exit and `return`, with the closing-brace teardown last. It
   is greedy with skip: a refused site does not stop a cheaper later site
   (`f40` expands, `f16` is refused, the `f2` calls after it expand). Pass 2
   then decides the calls inside the copies that pass 1 made: member and base
   destructors inside an expanded destructor, and a helper's own inline calls.
   Pass 2 comes after *all* of pass 1. In `nested_breadth`, five costly
   top-level calls written after sixteen outer sites still expand, while
   inner sites are refused.
2. **One budget per caller: B = max(B0, k × caller size)** (high in shape,
   medium in numbers; `level1_cutoff.py`, `caller_growth.py`).
   - B0 is about 58 units. Examples: 12 expansions of a 4-if body, 21 of a
     2-if body, one 48-if body, but not one 64-if body.
   - Above the floor, B grows linearly with the caller's own statements,
     dead or alive. Each `g = K;` adds about 0.47 unit, each call statement
     about 0.7, and each `if (x == K) use(1);` about 1.9.
   - No per-callee cap applies to *explicit* inlines (every probe callee
     here is `inline` or in-class): a 128-if body expands in a caller with
     400 stores. Auto-inlined plain functions do have a fixed size cap
     (2.1, "Per-callee auto-inline size cap": 7 one-line ifs inline, 8 do
     not). The two findings agree; the cap is the auto-inline gate, the
     budget is the only limit on an explicit inline.
   - Filler added while B is still at the floor changes nothing.
     `dtor_nested_filler.py` is flat until about 66 stores and then shifts
     one site per two stores.
3. **Cost of an expansion** (high; `costs.py`).
   - **Free inlines.** Below a small size threshold an inline expands and
     charges nothing. That covers one to four calls with constant arguments,
     up to six global stores, one `if` with one call, and expression-only
     bodies. Each parameter reference adds size: `mk(k); use(1)` ×3 is free,
     while `mk(k); use(k)` ×3 costs.
   - **Charged inlines.** Above the threshold the cost is about the size
     plus an overhead of about 0.7 unit (fits: 2-if 2.74, 4-if 4.74, 8-if
     8.74).
   - **Dead code inside the callee counts in full.** `if (k == 12345) {4 ifs}`
     costs more than the 4 ifs.
   - **`__forceinline` sites expand and charge nothing**, even with 8 ifs
     (`force_if4`, `force_big`).
4. **Pass-2 room** (high on the synthetic families; fitted by `fam1.py` and
   `fitftb.py`, with a single solution for the destructor family
   `dtor_nested.py`). At nested site i:

       room(i) = B − (pass-1 charges) − (nested expansions accepted before i)
                   − Σ over first-level expansions j after i of max(0, r_j − c_j)

   The site expands when room(i) ≥ its cost.
   - c_j is the instance's pass-1 charge.
   - r_j is its reservation, about the cost of the inline nested inside it.
     The fits give r = 0.98–1.00 × c_in for c_o = 0, 0.54 and 0.84 × c_in.
   - In other words, every first-level expansion that pass 2 has not reached
     yet holds its nested callee's cost; a visited one holds only what it
     really spent.
   - The shape of the decisions follows from c_o (the outer's charge) against
     c_in (the inner's cost):
     - **Outer free or much cheaper than inner: suffix.** The early sites
       call, every site after the first expansion expands, and the count is
       fixed by B (`nested_explicit.py '' 4`: the last 10 of 12, 16 or 20
       sites). This is the image of probe's ~Popup → ~Scroll (9 calls, then
       29 expansions) and of PickTech::pick (4 calls, then 12 expansions).
     - **Outer close to inner: periodic.** One expansion every
       c_in/(c_in − c_o) sites, starting late, such as `sssssSsssss` (11
       ~P sites, `dtor_nested.py`). These are the non-monotone depths
       (5,2,5) of 2.3.
     - **Outer dearer than inner: prefix.** Plain greedy in source order
       (`fam1.py 12 2`).
   - **Placement matters only in pass 2.** A free inline placed after site i
     holds a reservation but charges nothing. It refuses nested expansions
     at sites before it and never at sites after it (`probe_position.py 6 3
     if1`). This is the mechanism behind 2.3's "dead spend takes depth only
     from return sites before it".
5. **Deeper levels** (medium). A third level, such as ~FlatButton inside an
   expanded ~Scroll, is decided after pass 2 and gets what is left. In
   PickTech::pick, each added dead probe removes one ~FlatButton expansion
   before any ~Scroll changes. Its order within the level was not measured.
6. **Teardown with real headers** (high; `real_popup.py`). An isolated
   `Popup p;` with M returns expands ~Popup at every site up to the budget.
   Its ~Scroll then falls in the periodic regime: 5 returns give
   `SSSSSs`, 10 give `sssssssSsss`. From 20 returns on, the closing sites
   call ~Popup itself.

Using it
- Run inlinemap. For each callee row:
  - The *shape* names the regime.
  - The *count* difference is missing pass-2 room in units of the inner's
    cost.
- **The image has a suffix and ours expands earlier sites too.** Ours has
  more pass-2 room. Supply it from either source:
  - pass-1 charge from an inline the original expanded and ours writes raw,
    such as a wrapper or accessor with a body above the threshold;
  - reservation *after* the refused sites from free inlines (is_human,
    on_map, getters).

  Free inlines written before the first refused site do not move it.
- **A callee is called at an early pass-1 site and expanded at a later one.**
  Pass 1 cannot do that with equal costs, so the called site was a pass-2
  site in the original, inside an inline helper.
  In a small function this usually means the early sites sit inside the
  unit's own inline helpers that our source hand-copied: find the
  hand-copied helper bodies (a callee's first level written out at the
  call site) and rewrite them as calls, so /Ob2 expands the helper and
  leaves its inner call out of line as the image does (par-3, 2026-11-19;
  the `Win::is_visible`/`screen_to_client` pattern in AGENTS.md).
- **Missing owner, not missing budget** (high; battle_fight 0x00506AF0,
  2026-11-19 par-1). In a very large caller, dead pass-1 spend at the top
  (up to 32 if-units in battle_fight) refuses only nested (pass-2) sites,
  never a pass-1 site: B grows with the caller, so pass 1 has ample room.
  So when the image *calls* a pass-1-looking site for a callee that it
  *expands* elsewhere in the same function, the original's site was inside
  an inline owner (predicate, helper, wrapper) that the caller expanded;
  our source writes that owner out. Screen: add a dead probe at the top;
  if the called site does not flip, look for the owner (a natural helper
  boundary, or the same statement run in other functions), not for spend.
  battle_fight's population-loss test as an inline predicate turns
  has_project and veh_who into calls as in the image.
- **When to stop.** Dead probes that change other teardowns before they move
  the target mean that pass 1 is the binding limit, not pass 2. Look for a
  structural difference: which object owns the teardown, or a per-TU header
  guard (2.3).

What the model does not predict (2026-11-19)
- **MapWin::click 0x0046D5D0.** The image calls `::draw_map` in the first
  editor arm (0x46D977) and expands it in the second. Moving the first site
  into a free helper, which makes it a pass-2 site, still expands it. Free
  probes before or after it change the ~Popup teardowns (FlatButton 39 →
  41) before draw_map flips. No natural spelling was found.
- **probe 0x0059F120.** Free probes after the 9th teardown site do not
  refuse ~Scroll at sites 1–9: up to 24 change only the last two teardowns.
  Spelling every human test as is_human does not refuse them either. With
  hundreds of first-level instances, the reservations of all later
  instances are not modelled. The suffix shape needs ~Popup's charge to be
  near zero, whereas the isolated `real_popup` caller is periodic.
- **NetWin::pick_player 0x0047BBF0.** One cheap probe before the first
  return flips one of the two closing-teardown ~StringList sites (image
  calls both). Two to four probes do not flip the second; eight change the
  Dialogs stage instead. This is 2.3's two-stage behaviour, which the
  additive model cannot express.
- **config_game 0x00589D30.** Ours expands ~StringList at its two sites;
  the image calls both. Not attempted beyond the inlinemap reading.
- **PickTech::pick 0x00486510.** The image calls ~Scroll at the four Popup
  teardowns of the first `return tech_ai(...)`; ours expands it at all 16
  sites. Up to 24 dead 4-if probes after that return all expand. Each one
  costs exactly one more ~FlatButton expansion (12 → 24 calls), never a
  ~Scroll. In a real function, extra spend comes out of the deepest pass
  first (level 3, ~FlatButton inside ~Scroll), which confirms rule 5's
  ordering. The image instead keeps level-3 expansions at later sites while
  refusing level 2 at the earliest ones, so its pass-2 reservations are
  larger than ours in a way that dead spend cannot imitate.
- The units are approximate. Costs come from fitted counts, not node
  counts, so a prediction is good to about one expansion. Use it to choose
  the *kind* and *place* of a missing spend, then confirm with fe.

Callee costs, tree-wide (2026-11-19, par-4)

Question: do the image's teardown decisions say that the original's inline
destructors (~Scroll, ~FlatButton, ~BasePop, ~Popup, ~Dialogs, ~StringList,
~Sprite) cost more or less than our header definitions? Method:
`recovery/tools/analysis/inlinetree.py --all` runs inlinemap on every
function of every unit (about 20 s) and records each callee's E/c string,
image against ours. `inline/costknob.py` adds a dead statement to one
callee's body, which leaves every out-of-line copy byte-identical, reruns
the tree and reports which functions' decisions moved toward or away from
the image. Two knob sizes were used: `int k = 0;` (the smallest edit that
moves anything) and `int k = 0; if (k == 12345) close();` (about one
if-unit). Baseline disagreement is 662 site-units in 35 functions; every
other function's decisions match the image.

Disagreement per function under each knob (0 = matches the image; lower
is better; the last rows count the whole tree):

| function | base | ~Scroll +s | ~Scroll +1 | ~BasePop +1 | ~FlatButton +s | ~Popup +s | ~Popup +1 | ~Dialogs +1 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| PickTech::pick 486510 | 86 | 44 | **28** | 34 | 40 | 42 | 32 | 86 |
| probe 59F120 | **55** | 67 | 89 | 55 | 55 | 55 | 79 | 55 |
| Console::terraform 4D3470 | 42 | 37 | **10** | 44 | 44 | 44 | 45 | 42 |
| Console::upgrade 4D06C0 | 20 | 20 | **14** | 22 | 16 | 20 | 34 | 32 |
| save_game 5A9EB0 | 9 | 13 | **0** | 9 | 21 | 9 | 9 | 9 |
| call_council 52C880 | 36 | 39 | 96 | 44 | 44 | 44 | **11** | 36 |
| BaseWin::production 417040 | 2 | 4 | 45 | **0** | **0** | 4 | 45 | 2 |
| BaseWin::queue_click 409200 | **11** | 15 | 21 | 33 | 15 | 15 | 16 | 11 |
| BaseWin::base_editor_proj 41BBC0 | **14** | 14 | 30 | 29 | 14 | 14 | 14 | 14 |
| tech_trade 541100 | **15** | 32 | 42 | 31 | 31 | 38 | 38 | 15 |
| matching functions broken | | 16 | 45 | 24 | 14 | 12 | 40 | 2 |
| tree disagreement | 662 | 789 | 1132 | 796 | 756 | 749 | 1065 | 686 |

(+s = `int k = 0;`, +1 = one dead if.) The same `int k = 0;` in ~Sprite,
~Dialogs and ~StringList moves nothing anywhere. `virtual ~Scroll()`,
`Scroll::close();` and `this->close();` in place of `close();` (and the
same three for ~FlatButton) compile to the same decisions as the current
headers.

Image-implied cost against ours, per callee:

| callee | callers that want it dearer | callers that pin it at ours | verdict |
| --- | --- | --- | --- |
| ~Scroll | pick, terraform, upgrade, save_game (≈ +1 if-unit) | probe, call_council, queue_click, base_editor_proj, tech_trade and 16 matching functions even at +s | ours, within less than `int k = 0;` |
| ~FlatButton | pick, upgrade, production (+s) | save_game and 14 matching functions at +s; +1 breaks 52 | ours |
| ~BasePop | pick, production (+1) | queue_click, base_editor_proj and 24 matching functions | ours |
| ~Popup | call_council, pick (+1) | probe, upgrade, production and 40 matching functions | ours |
| ~Dialogs, ~StringList, ~Sprite | none | insensitive at +s; +1 breaks 2 (~Dialogs) and 3 (~StringList) matching functions and helps none | ours |

Conclusions
- Every one of these callees is pinned at our header's cost by functions
  that already match. Each mismatching caller wants a *different* callee
  made dearer, and some of the mismatching callers (probe, queue_click,
  tech_trade) get worse under every knob. A single header spelling cannot
  supply the image's decisions; no header change was made.
- So the missing spend or the excess budget is in the callers. PickTech::
  pick improves under every knob (86 → 28–44), which means its pass-2 room
  is too large *whatever* takes it: its own body is longer than the
  original's (B too high) or it lacks a pass-1 charge the original had.
  terraform and save_game want ~Scroll-specific reservation (they improve
  only under the ~Scroll knob), which points at extra Popup/Scroll-owning
  objects or teardown sites in the original rather than at more statements.
  call_council wants ~Popup reservation only: its own Popup's teardown, not
  its size.
- The knob is a screen for *which callee's reservation* a caller lacks.
  Run `costknob.py` with the knob in each candidate callee; a caller that
  improves under exactly one callee lacks an object or teardown site of
  that class; a caller that improves under all of them has too much budget
  (find the statements the original wrote as calls or inline helpers, 2.8
  "Using it"). A matching-function count is the price of a real header
  change, and these knobs are not real changes, so never commit one.

## 3. Symptom → first thing to check

### 3.1 Calls and inlining
- **The image calls something we inline (or the reverse).** Run
  `analysis/inlinemap.py` and read the site shape with 2.8; check the budget
  rule (2.1), definition order, and a hand-copied callee body. The most common
  regression cause is a hand-copied first level of a recursive helper such as
  `is_visible` or `screen_to_client`.
- **A hand-written block equals a same-TU function body.** Call the function;
  write it inline before its callers if the image expands it there.
  The same holds for small guard wrappers the image keeps one out-of-line
  copy of (`if (guard == 0) return 1; return hook(args);`, 0x004C6000 and
  0x004C6030): call the wrapper at every site that repeats the guard.
- **An inlined accessor skews register choice through the whole function.**
  Read or write the member directly (the netwin fix covered 10 functions).
  For a member control with a virtual base the difference is larger: the
  image addresses the base's fields as `[vbdisp + this + K]` with one
  `lea esi,[ecx+ebx]` base, and reuses it after calls; through an accessor
  that returns a reference (`file_win_.list_box().item_list_`) VC6 first
  materializes the control's address (`lea esi,[ecx+ebx+0x3280]`, small
  displacements) and reloads the vbptr later. Write the member chain
  (`file_win_.list_box_.item_list_`), making the member public if needed,
  and bind a nested object once with a reference when the image keeps its
  base across calls (AlphaSave::on_selected 0x00407D90).
- **A pointer chase that reuses eax for every link**
  (`mov eax,[list+0xc]; mov eax,[eax+8]; mov eax,[eax+4]` under a head
  test, `xor eax,eax` otherwise) is `list.head_ ? list.current_entry()->text
  : 0` with StringStruct::current_entry expanded (it repeats the head test;
  the unit must define STRINGSTRUCT_CURRENT_ENTRY_INLINE). Every plain
  spelling of the chain routes the middle link through ecx and shifts the
  register rotation of the code after it by one.
- **A register rotation runs through the whole function.** Look for a lost
  inline-helper boundary around a repeated subsequence, or a hand-duplicated
  shared tail.
- **A dead store of a test result before each branch** (`(x>>k)&1` or
  `x & mask` written to one never-reloaded frame temporary) is an inline
  helper's parameter; raw `?:`/`|=` spellings produce no store.
- **The image repeats a compare or reload VC6 would fold.** That is an inline
  helper boundary: wrap the subexpression in a small static inline.
- **Separate jump sites mean separate arms.** Two `mov reg, K; jmp shared`
  sites in the image come from two source arms with the same store, not one
  `||` condition.
- **The diff reads as "a different program" (N instructions against M)** in a
  function with `jmp [4*reg+table]`. The in-body jump table lacks its
  `RECOVERY_ASSOCIATED: v1 switch_targets` annotation. Add it from the image's
  table bounds before reading anything else into the diff.
- **`dec`/`je` on a byte field** with the default arm laid out first is
  `switch (x) { case 1: ...; default: ... }`; `if (x == 1)` gives
  `cmp byte ptr [...],1; jne`.
- **An int local assigned from a `uint8_t` getter** spills (`mov byte ptr
  [tmp],cl; mov ecx,[tmp]; and ecx,0xFF`); write the direct field expression
  where the image shows `xor ecx,ecx; mov cl,...`.
- **Raw `GenderDefault`/`PluralityDefault` store pairs.** Write `parse_set(...)`
  or the header getter instead; the raw pair rotates registers in later code.
  The register of a following `lea` of a leader field follows the spelling:
  raw stores then `strcat(..., Players[f].title_leader)` give edx; a named local
  assigned after the stores gives eax; one assigned before the stores gives
  ecx. An image span equal to an accessor's body does not prove the accessor
  was a header inline (get_title/get_name became faction.h MEASURED inlines on 2026-11-16, measured neutral).
- **`memcpy`/`memset`/`str*`/`abs`/`sqrt` are always calls in the game and
  Jackal** (no /Oi; `stdafx.h` names every intrinsic in `#pragma function`
  until the flag is dropped; Caviar and UV keep theirs). Never add a
  per-site `#pragma function`/`intrinsic`. An image `rep stosd`/`rep movsd`
  there is an element loop (`for (i = 0; i < N; i++) a[i] = 0;` — an odd byte
  count adds `stosb`/`movsb`/`movsw`), a structure assignment, or an
  aggregate initializer; a 4-byte "memcpy" is a cast store. See 3.7.
- **A `vector_dist`-style computation is inlined.** Write a two-level static
  inline helper modelled on the existing ones.

### 3.2 Registers and order
- **Operand order of commutatives.** For `a + b` of globals, VC6 loads `b` first.
  For `f(a) OP f(b)` it calls the right side first. It canonicalises
  `(a + b) / 2`.
- **Which of two member loads comes first** in `a + b` is not set by the
  spelling (`a + b` and `b + a` compile alike) but by which operand the
  function referenced first. Reorder the earlier references instead: reading
  the field into a local before the first use of the count flipped both loop
  bounds of ProdPicker::on_scrolled (`mov eax,[count]; mov edx,[first]`).
- **`this` and a member-address temporary in each other's registers**
  (image `this` in esi and `&member` in edi, ours the reverse), often with
  the real epilogue behind the wrong block. The function returned through
  one result variable and one final `return result;`: each arm assigns it
  (`if (m == 0) r = create(&m); else r = 0xC;`), later work is gated on it
  (`if (r == 0) {...}`, `if (r == 0 && m != 0) {...}`) and nothing returns
  early. VC6 threads the constant arms past the gate into their own
  returns, so the code looks as if it had early returns. Any `return 0xC;`
  or `if (r != 0) return r;` swaps the two registers (Wave::dyna_load
  0x004C6BB0, VoiceTx::init 0x004C8F70).
- **Which variable gets a callee-saved register**: see section 2.7 (order
  esi, edi, ebx by weighted reference count; ties to the first referenced;
  position weights; cached pieces of memory-homed variables). The earlier
  wording here ("declaration order", "a local outranks a parameter") was
  wrong: equal counts go to the candidate referenced first in the code.
- **Two zero stores as immediates or through `xor eax,eax`.** Measured on a
  standalone copy of Credits::unload: with `/Oy-` VC6 emits `xor eax,eax`
  and two register stores; with `/Oy` (and with `/Os`) it emits two
  immediate stores. `/G3`-`/G6`, `/Oa`, `/Ow`, `/Op`, `/Ob1` do not change
  it. credits.cpp is built `/Oy` with `#pragma optimize("y", off)` around
  its framed functions for this reason, and Credits::exec (0x004289F0, 3
  rows) is the case that fits neither: the image has an ebp frame AND
  immediate stores, ours has the frame and `xor` with the pragma, no frame
  and `xor` without it. Every framed function in credits.cpp has stack
  parameters or locals and every frameless one has none, which is plain
  `/Oy-` behaviour, so the unit was most likely built `/Oy-` like the rest
  of the game and the open question is the reverse one: what makes `/Oy-`
  emit immediates in Credits::unload and Credits::exec. Measured so far:
  under `/Oy` the `xor` form appears exactly when ebp is also taken as a
  fourth register; member types (unsigned, int, pointer, enum, float) and
  chained assignment do not matter.
  Also ruled out on the standalone copy: 28 further switches (debug
  info, RTTI, string pooling, calling convention, packing, /Os is the only
  one that changes it and it breaks nine other functions), the SP3 back
  end (same `xor`), loop spellings, block scoping, volatile members, a
  64-bit member, `memset`. VC5 does emit the immediates but keeps `this`
  in eax, unlike the image, and cannot compile the unit's headers.
- **Statement vs conditional expression.** `d = a - b; if (d < 0) d = b - a;`
  and `d = (a - b < 0) ? b - a : a - b;` emit the same instructions with two
  registers swapped (on_scrolled's distance). Try the other form on a
  two-register tie around an absolute difference or a clamp.
- **expand_rect's constants.** For `expand_rect(&r, -N, -N)` the image's
  "+N immediate on left/top, register holding -N added to right/bottom" comes
  from the helper body `left -= h; right += h`; hand-written `+=`/`-=` pairs
  or an inset helper differ.
- **Null-check forms.** `static_cast<Base*>(this)->f()`, a `Base *p = this`
  local and `static_cast<Base&>(obj).f()` all emit a null check; call
  `Base::f()` / `obj.Base::f()`. Passing `this` as a `Base*` argument
  legitimately emits neg/sbb/and. Member data through a virtual base needs no
  check; a reference conversion does.
- **label_get or the written-out lookup is a per-site choice, and many
  sites had the wrong one.** A sweep in both directions (2026-11-18) kept
  8 sites as `label_get(k)` and 110 sites as
  `StringTable.get(Labels.strings_ptr[k])`; 45 functions moved closer
  (battle_compute 895 -> 254 rows, say_orders 90 -> 3,
  CouncWin::on_button_clicked 104 -> 38) and AlphaNet::pick_service
  became exact. The wrapper changes argument evaluation order and spends
  inline budget, so a wrong choice shows as shifted loads and as
  destructor expansions of the wrong depth. Rerun
  `ruleall.py labelget` / `labelraw` after source work on popup code.
- **label_get vs written-out order.** A `label_get(k)` argument is evaluated
  before the other arguments; `StringTable.get(Labels.strings_ptr[k])` at its
  own position. Pick per site from the image.
- **Struct copy after in-place edits.** A whole RECT/vector assignment shows
  as dword moves through a pointer that reload some fields and forward
  others; stores back into a local block before such a copy mean the
  original modified the source in place (MCIVideo::load, Caviar::init).
- **`x = max(a, f())`** evaluates f twice (`cmp; jle; call f again`); a
  single conditional store is `max(a, b)` of plain values.
- **Coordinate wrap with the flat arm first.** An image x-wrap testing
  MapIsFlat with `je` to the wrap code and a fall-through flat arm is a
  ternary: `x2 = (MapIsFlat & 1) ? t : t < 0 ? t + W : t >= W ? t - W : t`
  (wrap_map_x and if-chains give jne layout).
- **`int s = A || B; if (!s && C) return;`** threads both constant stores
  (the `s = 1` block lands at the next unconditional jump).
- **Member stores vs inline setter.** When the image writes a member array's
  fields this-relative right after writes through the member's own register,
  the original stored the fields directly; an inline setter makes its
  receiver a pointer register (BaseWin::init's slice colours).
- **Rect argument order.** A small static inline computing `r->right -
  r->left` loads right then left and puts the width in the first register;
  the written-out expression loads left first. A place-rect helper
  (x, y, w, h) reproduces right-to-left argument loads before ordered stores.
- **Store-to-load forwarding.** VC6 forwards stored rect values into later
  argument loads through the same named pointer local (`RECT *r = &m`);
  writing `&m` at each use gives the image's re-loads and frees a frame temp.
- **A selection kept in an incoming argument slot** is the parameter reused
  (`y = X_pop(...)`), not a new local. Field stores through a receiver
  register (`lea esi,[ebx+off]` then `[esi+..]`) mark an inline accessor or
  a list-pointer local.
- **A named local passed to one call** (`int s = ...; f(x, s)`) can shift
  registers across the function; pass the expression inline (quayle).
- **`cond ? !a() : !b()`** materialises a bool (neg/sbb/inc); the image's
  per-arm `test; je` is `if (!L) { if (!a()) goto found; } else if (!b())
  goto found;`.
- **`if (c) return p; return NULL;` becomes branchless** (neg/sbb/and). When
  the image has separate epilogues with a store after the test, put the store
  in each arm: `if (c) { *out = 0; return p; } *out = 0; return NULL;`.
- **Value reuse across a byte store.** A `T &e = arr[i]` reference lets VC6
  reuse a loaded byte field across a store to a neighbouring field;
  `arr[i].f` at each use reloads, as the image does.
- **`a == b` of two setcc values:** VC6 evaluates the side containing an
  inline call first; keep the getter call on the side the image computes
  first (convoy: `(x >= K) == (get_triad(v) == C)`).
- **A loop counter holding `i + 1` mid-body with no increment at the end**
  is `for (i = 0; ...; ++i) { faction = i + 1; ... }`.
- **Commutative order through a pointer local.** VC6 canonicalises
  `(m.left + m.right) / 2` on directly read members, so swapping operands does
  nothing; reading through `RECT *r = &m;` gives the other load order with
  otherwise identical code (draw_reactor became exact).
- **Locals and declaration order.** Declaration order of locals loaded from one
  struct sets load and slot order. A named local takes a callee-saved register,
  while an unnamed temporary spills.
- **Constants.** `x * 2` gives `add`, `x << 1` gives `shl`. A const local bound
  gives `lea`; writing it at each use gives `inc`.
- **`xor ecx,ecx; cmp; sete cl; test ecx,ecx; jne` inside `(P && X) || (!P && L)`**
  comes from a local flag: `int t = 0; if (P) t = (a == b); if (t || !P && L)`.
  Inline, ternary and inline-function spellings fold to a plain branch.
- **`x - 2*x`** (and `x*2`, `x+x`, through temporaries or helpers) folds to
  `-x`; only `x - (x << 1)` survives as `lea r,[x+x]; sub`.
- **A `jcc` straight to a shared mid-function epilogue** comes from a nested
  `if` whose false edge reaches a common `return`; `if (!x) return 1;` gets its
  own `pop...ret` copy.
- **Register-held zero.** VC6 knows `eax == 0` only on the fall-through of `jne`
  after `test eax,eax`.
- **`cmp eax,<zero register>` where we emit `test eax,eax`.** Store the call
  result in a named local.
- **imul destination.** In `z += f() * y` VC6 makes the moved call result the
  imul destination; `int t = f(); z += t * y` makes y the destination;
  `t = f() * y; z += t;` keeps `imul eax, edx` and schedules the next call's
  push before the stores (supply_options).
- **Nested label_get in an argument list.** When the image calls get after the
  outer call's other pushes, write `StringTable.get(Labels.strings_ptr[k])`;
  the forceinline label_get hoists the call ahead of them (PrefWin::init).
- **The comparison literal survives.** `x >= 4` gives `cmp 4; jge`, `x > 3`
  gives `cmp 3; jg`; an off-by-one constant with a different jcc in an
  aligned diff names the original spelling (fixed 9 sites in base_terraform).
- **`max(0, x - K)` as a condition** gives `add eax,-K; test; jl; je`;
  `x - K > 0` gives `sub; test; jle`, `x > K` gives `cmp; jle`.
- **Tail-merge direction follows source order.** When VC6 cross-jumps
  identical tails, the copy in the branch written last stays in place and
  earlier branches jump to it; the image's merge target names the last
  branch.
- **`lea/inc; test; je` against -1** is `x + 1 != 0`; `x != -1` gives `cmp`.
- **Late sprite address** (formed after every position argument, its
  transparent byte read last) means an inline overload without the index
  argument.
- **A named difference local used twice** (`const int w = r - l; if (w <
  f()) g(w)`) loads the subtrahend into a register first; writing the
  expression at each use gives `sub reg,[mem]` plus a dead-slot store.
- **A probe that only works after the final `return`** is paying for the
  closing-brace phantom teardown: the original had no top-level return,
  typically a `for (;;)` body left only through returns. Wrap the body
  instead of keeping the probe (top_menu).
- **Bistable neighbours.** In battle.cpp any edit to boom (one new goto
  label, two block-scoped const locals) flips battle_fight's kill/damage
  if-else layout (562 -> 751 rows). Diff the next function in the unit
  before keeping an edit.
- **One strcat per arm** of an if/else gives separate `add esp,8`
  cleanups; a `?:` argument merges them.
- **Shared blocks between sibling branches.** When an earlier branch's
  exits jump forward into a later branch's blocks, the labels were in the
  earlier branch and the later one used `goto`s (synch).
- **+/- chains re-associate against available CSEs**: search operand
  order per line rather than reading it from the image (sub_6404f0).
- **Separate identical literals per arm** (distinct string addresses)
  mean an unpooled unit; under /GF VC6 hoists the merged pushes.
- **A widely included header is not a free home for a one-file
  stand-in.** Adding even one or two inline members to sprite.h changed
  code in about 20 unrelated units and broke claims; inside a unit, a new
  static inline helper can flip a canary where reusing an existing helper
  does not.
- **A virtual-base adjustment off a pointer register** (`mov eax,[edi];
  mov eax,[eax+8]; add eax,edi`, then `[eax+off]`), or a virtual call
  whose receiver is formed by `lea ecx` before the vtable load, means
  access through a pointer (`T *p = &member` or an inline parameter);
  direct member access folds to `[edx+edi+off]`.
- **Template RECT with half its fields in the frame.** Only right/bottom
  stored and left/top in separate slots: `RECT base` with named left/top,
  copied per cell and moved with `offset_rect` (EditGroup).
- **`test; js L; cmp K; jle; mov K; jmp; L: xor`** after a `mov eax,edx`
  copy is general.h `range(x, 0, K)`; an if/else clamp misses the copy.
- **A null test returning 3 before `EditBox::init`** is an inline
  `EditBox::init(RECT*, Win*, int)` overload (header candidate).
- **A static inline helper boundary can supply missing spend** like a
  probe, but which calls stay outside it matters (POP3's seek_pos).
- **Tail cross-jumping depends on the whole function.** VC6's merge of
  identical tails switches on or off with changes elsewhere: production's
  duplicated case tails merged only after five hand-written
  `T &r = x; r = cast(r +/- 1)` updates became `x++`/`x--` (size, local
  and reference counts alone do not trigger it). Fix hand-written idioms
  elsewhere before touching the tails; merging also removes CSE temps.
  Write duplicated tails out in full: a shared `goto` label lays out
  differently from the machine cross-jump.
- **`lea reg,[this+off]` at every use** means no pointer local; a pointer
  local holds a callee-saved register for the whole function.
- **`inflate_frame(&member, ...)` keeps a dead first store and reloads
  the right edge**; through a `RECT *` local the stores are forwarded.
- **if/else of two calls vs a ternary argument** is not counter-neutral in
  a canary unit: check the whole unit's objects before keeping either.
- **Switch tail-merge direction.** Cases ending in identical
  `f(); return K;` tails keep the first case's copy; ending them with
  `break` and one `return K` after the switch keeps the later copy; a
  case with its own `return` keeps a private tail.
- **Auto-inline eligibility follows the callee's named locals**: gen_tile
  with two const locals was never auto-inlined at /Ob2, with one it was.
- **A refused later site of an inline depends on caller spend**, not
  callee cost: probes in the callee body do not move it.
- **A member-pointer call on a class with virtual bases** adds a vbtable
  adjustment; it is not an out-of-line stand-in.
- **Natural spend from invented out-of-line helpers.** When early sites
  under-spend, compare the image's call list with ours: invented helpers
  the image expands (make them inline members), repeated blocks the
  image cross-jumps (one inline helper), and raw mask tests where an
  accessor exists (is_alive/is_human) supplied all of process_message's
  missing budget without probes.
- **Dialog-control twins** (CheckBox/RadioButton/SpriteBox): copy the
  exact sibling's spelling first.
- **A local copied from a struct-member global is rematerialized**:
  `int w = MapLongitudeBounds;` is reloaded from the global at each use.
  When the image holds the value in a callee-saved register, read the
  global directly at each use and VC6 CSEs it (editor: 244 -> 57 rows).
- **A cast at the use is not an unsigned field.** On an int16_t field
  the casts give byte ops (`and ah,-0x21`); an unsigned short field gives
  `and eax,0xdfff`. Fix the field type when the image shows the dword form.
- **Score probe grids on the teardown call sequence**, not only the state
  tuple; map call targets through `// symbol` lines first.
- **Out-parameter copied to a plain local.** An address-taken local is
  reloaded after every call; when the image holds the value in a
  callee-saved register, copy the out-param into a plain local
  (on_left_click: 853 -> 805 instructions).
- **`lea r1,[esi+off]; mov r2,r1` then stores through r2** is a RECT
  struct assignment; an inline set_rect helper folds to direct stores.
- **Per-arm Labels loads joining one StringTable.get** is an if/else
  chain assigning an LPSTR, then one get call.
- **A contiguous ascending 8-byte double run is an array** (`double
  v[3]`, `int b[3]`): named scalars coalesce into dead parameter slots
  and interleave the `_ftol` stores (adjust_pal).
- **The shallowest EH depth may need nested spend**: only nested-4 probes
  kept two ??_D StringList stages out of line (supply_options).
- **New helper definitions flip bistable neighbours**: define them at the
  end of the unit behind a forward declaration.
- **Constant-argument dead probes can spend nothing.** When probes show
  no effect, try a natural inline taking a live parameter before deciding
  spend is irrelevant (on_button_clicked: 16 constant probes nothing, two
  real expansions enough).
- **`mov reg,K; cmp field,reg`** repeated across cases with different K
  is an inline helper taking K as a parameter.
- **Pooled literals change tail merging**: identical `X_pop("LABEL",0)`
  arms merge whole in a pooled unit; check pooling before chasing a
  cross-jump difference.
- **Hand-expanded setcc arithmetic is a ternary.** The image's `neg; sbb;
  and; add` is `c != 0 ? -1 : K`; Ghidra's `(-(uint32_t)(c != 0) & M) + K`
  compiles to setne/neg/and/add.
- **A named tile-pointer local** is propagated across calls into a frame
  slot; where the image re-indexes after each call, index at each use.
- **A named local keeps a dead store** (`int t = expr; if (t == 0)` stores
  t into a dead parameter slot under /GX); `if (expr == 0)` does not.
- **A dead `lea reg,[ebp+reg-base]` after a store** marks an inline member
  setter on a virtual base (`obj.dialogs().set_x(v)`); a direct write
  leaves none and a free helper taking `Base&` adds a null check.
- **Every game unit is unpooled** (only the Caviar library pools). A unit
  literal is its own `$SG` object, so a literal the original shared must be
  named: `" "`, `", "` and `": "` appends are BufferJoinSpace (0x682820),
  SocialWinCommaSpaceText (0x682E90) and SocialWinColonSpaceText (0x682E94);
  one literal pushed from one address at two sites is one object (a shared
  inline helper or named string). Check the image's push addresses before
  writing a literal.
- **Canary windows move with the recipe**: an extern-name window or
  file-state redeclaration found under one recipe must be re-swept after a
  recipe or literal change, and may no longer be needed at all
  (corner_the_market's two redeclarations, action_give's 17 names).
- **Never revert a faithful change for a broken claim.** Diagnose and repair
  the break (widening scope is fine), now or later; revert only a change
  the image shows to be wrong.
- **Read a switch's jump-table case order first.** A case-order mismatch
  accounted for most of DiploWin::on_button_clicked's 0.51.
- **When every probe placement gives the same EH topology**, spend is not
  the lever: compare teardown call sequences (a missing
  DIALOGS_DTOR_INLINE there).
- **Guarded class members still change the opting-in unit's codegen**:
  re-measure that unit's near-misses too.
- **A per-TU `#define X_DTOR_INLINE` flips ties in earlier functions**;
  defining the inline body in the unit just before the caller that needs
  it gives the same expansion and leaves earlier functions alone.
- **`f(g() - h())` pushes the outer constants first**; a local
  `int c = g() - h();` evaluates both calls before them.
- **`if (s < 1 || h < 1) v = 1; else if (s > h) v = h; else v = s;`** with
  s its own temporary gives three separate stores; clamping in place gives
  store-then-overwrite.
- **A dead `test` of a flag with no branch** is an if/else with identical
  arms: VC6 folds `c ? K : K` completely.
- **A raw mask test vs an accessor can trade early-site for late-site
  teardown depth** at the same total: compare per-site sequences.
- **Legacy save-format conversions are member-by-member** assignments and
  loops over typed row structs after one whole-row memcpy; memcpy-per-field
  helpers compile to a different program (game_data 0.83 -> 0.956).
- **A counted dword loop over consecutively named scalars** means the
  original had an array; VC6 never folds separate scalars into a loop.
- **Sweep flat probe counts to at least 16** before concluding spend cannot
  fix an early-return EH site (load_map needed ten).
- **A per-arm wrap store** (each arm stores the wrapped coordinate to its
  slot) is a written-out ternary over a temporary; `wrap_map_x` or an
  if-chain stores once after the join (Path::sensors).
- **An inline-only member with an out-of-line copy and no callers**:
  `MEASURED` on the member's own declaration plus an `inline` definition
  emits the copy and lets callers expand it (gen_tile).
- **Masked wrong target from a guarded wrapper.** `popup.item()` /
  `popup.exec(cb)` without POPUP_WRAPPERS_INLINE call BasePop's
  out-of-line copy; masked measurement hides the wrong target. Define the
  guard whenever the wrappers are used.
- **A named index local inside an inline helper blocks cross-expansion
  CSE** (`int proto_id = Vehs[v].proto_id;` in proto_power); indexing at
  each use lets VC6 keep a field in a register across expansions.
- **Constant-left loop compares** (`9 > i`) let VC6 compare the
  strength-reduced subscript and keep a scaled twin of `i`; `i < 9` keeps
  the plain `[reg*4+table]` index (world_site).
- **A materialised `neg/sbb/neg` flag stored before its test** is a header
  inline returning the value (has_fac), not a direct bit test.
- **Message wrappers move teardown depth**: `veh_battle_msg` vs direct
  `NetMsgPrimary.pop` changes every Popup site's depth; try which sites
  originally used the wrapper.
- **Re-sweep budget stand-ins after restoring a natural helper**: the
  helper supplies some of the spend the stand-in covered.
- **Whose setter left the dead lea**: `[vbo+edi+off]` with edi =
  &member is a derived-class member (ListBox); `lea ecx,[vbo+esi+off];
  mov [ecx+x]` is a base-class setter (Dialog).
- **A register copy feeding two stores at each if/else arm's end** is
  each arm assigning only `a`, then one `b = a;` after the if/else;
  chained `b = a = v` uses one register (EditBox::on_key_click).
- **Field types from jcc and operand width**: signed jumps on 32-bit
  accesses mean `int`; a byte cast is not evidence of a 16-bit type
  without movsx/movzx or word operations.
- **A block-scope array takes a slot of its own size**: a larger image
  slot means a disjoint-scope sibling shares it.
- **Folding N scalar members into one array needs N-1 compensating
  member declarations** to stay counter-neutral (typedef names count,
  static members do not); anonymous-union stand-ins cannot be balanced
  reliably, and header-level unions renumber every includer's `$SG`/`$L`
  labels (plan a project-wide rebind).
- **`NetMsgPrimary.pop(label, 0xFFFFEC78, 0, icon)` is the two-argument
  inline `pop(label, icon)`** (body `pop(label, -5000, 0, target)`): the
  inline restores Popup callers' missing spend; check every -5000 pop.
- **Leftover budget after the last early return is conserved** among the
  last few return sites and the final teardown: probes there only move
  it; matching needs real spend placed earlier.
- **Declaration order and scope do not move scalar frame slots or
  callee-saved register choice** (facilities_click, draw_expenses,
  reset_territory, repair_phase: every variant byte-identical); slot
  rotations and register equilibria need a change in the uses.
- **`veh_battle_msg` vs direct pop changes depth by count, not position**:
  shallow early sites with a deeper final teardown means the original
  caller was larger.
- **A shell `timeout` around `recover permute` loses the report**; use
  `--time-limit`.
- **Sibling block-scope arrays share from the high end**: a buffer N bytes
  into a larger array's slot means sibling scopes; give the larger array
  its own `{ }` block around its live range.
- **Unreferenced local arrays get no frame space**, even with `(void)x;`.
- **A shallower first return site than later ones** (a destructor called
  out of line at site 1, expanded later) is structural; probes starve
  later sites first.
- **Other shared punctuation objects**: "(" 0x682E9C, ")" 0x682E98 and
  "$LINK<" / "=" / ">" 0x683544/0x683540/0x68353C are pushed from many
  units; unpooled units cannot share literals, so these are named globals.
  Declaring real shared objects can also replace extern-name pads
  (reportif: 5 real names for 5 pads).
- **Accessor conversion is per site, not wholesale**: the image keeps
  some mask tests raw (with `1 << faction` in a register); converting
  those makes teardown sites shallower than the image.
- **Unlisted in-text jump tables**: when the original span stops before
  in-text tables, add `RECOVERY_ASSOCIATED: v1 switch_targets <func>
  <start> <end> <jmp>` per table.
- **Header inline edits rename every includer's literals**: refused
  claims with "strict selected evidence result: unknown" while masked
  exact need `rebind-literals`, not a revert.
- **Constant member stores interleaved with the next call's pushes** come
  from an inline helper taking them as parameters.
- **A stored `1<<f` tested later** is a named `self_bit` local: keep the
  raw mask test there; `is_human()` fits where the image recomputes it.
- **dec/jne down-counters with separate stepping coordinate slots** are
  `for (i = 0, x = X0; i < N; i++, x += DX)` with i used only in the
  subscript; `X0 + i*DX` at the use gives a compare on the reduced value.
- **`lea r,[eax+K]; cmp r,BOUND` on a counter from 0** is `for (k = K;
  k < BOUND; k++)` with uses written `k - K` (`if (k - K)` gives test).
- **A null-tested this-to-virtual-base conversion before each call**
  means `this` was passed at each site, not a cached base pointer.
- **A nested call inside a call with constant arguments** pushes the
  constants first; `int a = f(k); g(0, T[a].name, -1, -1)` evaluates the
  call before the pushes.
- **`c ? inline_a : inline_b` as a condition materialises setl**;
  `(c && a) || (!c && b)` gives branch-only code.
- **Taking a free function's address stops its auto-inlining** in later
  functions of the unit; it is not a per-site noinline lever.
- **Unpooled tails merge only up to the first distinct literal**: shared
  named objects let identical tails cross-jump fully.
- **Goto into an if body**: `if (t == 1 || t == 2) goto A; ... A: if (t
  == 2) {...}` lets VC6 thread the second test as the image's je pair; a
  goto to the inner label inverts the test instead.
- **`(uint8_t)(1 << f)` shifts a byte register**; the image's dword shift
  is `(1 << f)` without the cast.
- **VC6 folds a switch variable's case constant into array indices**: an
  image with `index*K` hoisted before the jump table is not reproduced by
  a copy local or a reload.
- **A loaded index kept across a byte store between two RMWs** is a named
  local: `int idx = s.f; A[idx]--; s.b = v; B[idx]++;` (give_a_base).
- **A merged early `return 0` can be allocation, not structure**: when
  the image schedules `xor eax,eax` before a final store there is no
  common tail to merge.
- **A canary can flip from a neighbour's loop shape**, independent of
  literals or declarations (action_give vs action_airdrop's is_seen
  loop): re-sweep the canary's window rather than revert the faithful
  neighbour.
- **Induction-pointer bias follows spelling**: `const T &r = arr[i]`
  anchors the pointer at field 0; `arr[i].f` at each use (or passing
  `arr[i]` to an inline taking `const T &`) anchors it where the image
  does.
- **Sibling loops sharing one counter slot** need one function-scope
  variable per role reused by every loop.
- **A flag kept in a dead incoming-argument slot** and re-tested after a
  loop is that parameter reused.
- **/Ob2 has a per-callee size cap separate from caller budget**: a
  function no caller in the unit expands is over the cap by its own
  source size. Find the excess by deleting one statement at a time and
  checking C4711; member calls on subobjects/global objects cost more
  than plain calls.
- **Import vs thunk calls bind to the latest declaration**: a plain
  redeclaration after the SDK's dllimport one sends every later call in
  the unit (inline bodies included) through the thunk; one unit cannot mix
  both by declaration order.
- **A Spot::add reading a partially stored RECT copy** is `RECT r = area;
  r.right += K;` passed to the RECT-form add expanded in place.
- **VC6 packs locals into dead parameter slots in an order declarations
  do not control**: when the image keeps a variable in a specific
  incoming slot, use that parameter directly (draw_status).
- **A frame-resident scalar whose dead stores survive** was not a register
  candidate in the original; `volatile` approximates it (with reloads),
  `(void)&x` does not.
- **`G[i]++` on a loaded counter inlines as `mov r2,r1; inc r1`**;
  `G[i] = loaded + 1` inlines as `lea r2,[r1+1]` (same standalone body):
  pick by the image's inlined copies.
- **A shared `return 0` that keeps its own xor** while other sites jump in
  was reached by goto; VC6 places the block at the last goto in source.
- **`movsx eax, byte [g]; and eax,1`** comes from a named int local
  initialised from the char; casts and char-returning inlines fold.
- **The per-callee inline cap counts invented accessor calls** (an inline
  returning `T *&` costs like a member call); reading the global directly
  can bring a callee under the cap.
- **Inline setter stores pin scheduling**: a store through an inline
  member setter stops VC6 hoisting a later virtual-base pointer load
  above it.
- **A prologue save the image does early** can come from an early arm
  that duplicates the final tail call and returns.
- **A guarded tail cross-jumped from two sibling arms** was written once
  after the arms.
- **VC6 makes inline decisions at a block's written source position**,
  not where it lays the block out: an `||` tested at an early goto site
  whose `= 1` arm sits before a later label was written at that later
  position (communicate: moving it sent has_treaty/has_agenda out of
  line and freed probe budget).
- **Getter spellings are natural spend** (get_title/get_name/get_noun/
  get_adjective instead of parse_set + field): identical bytes at the
  site, but they spend /Ob2 budget; measure per function.
- **The per-callee inline cap counts member-access depth**:
  `obj.a[i].b[j]` can put a body over the cap where `obj.direct_member`
  or a plain global fits with identical code; absolute image accesses to
  a global object's field suggest a direct member.
- **Inline decisions are not forward-greedy**: an earlier site can refuse
  a callee a later site expands.
- **Small callers sit at a minimum budget**: added statements move
  nothing there, while a few dead statements can flip a large caller's
  nested decision.
- **A width computed right-then-left with `sub reg,[left]` inside an
  argument list** is a `RECT*` width helper; a named width local loads
  left first and CSEs it with a later argument.
- **A parameter kept in one register across a clamp** was reassigned in
  place, not copied to a local.
- **An index register equal to a value just compared** means the source
  indexed by the compared expression.
- **label_get vs written-out label reads is per site and greedy**: one
  site can rotate registers through the whole function.
- **Definition order does not gate inlining of explicit inlines**: VC6
  decides at the end of the unit.
- **Spend that refuses a depth-2 call must sit inside the inlined callee,
  before the call**; caller-level probes rarely reach it.
- **A function whose full body is expanded in another unit's callers is a
  header inline** (and keeps its out-of-line copy for other callers).
- **A condition equal to a same-unit predicate's body was a call to it**,
  expanded inline; the raw expression lays out the then-block after the
  join (monolith 0.866 -> 0.973).
- **`if (c) goto L; B; L:` with an earlier goto to L** moves B far away;
  a fall-through B in the image was a structured `if (!c) { B }`.
- **Four rect stores after all four edges are computed** (right-to-left
  argument order) are one inline LTRB setter call.
- **A raw global store pair can change a later load's register** (and
  the short `mov eax,[abs]` encoding shifts every later byte): use the
  accessor (parse_set) and spell the next site as the image does.
- **`f(a) < f(b)` with the left call first and spilled** is
  `int t = f(a); if (t < f(b))`.
- **VC6 drops dead stores to a volatile local after its last read**, so a
  volatile stand-in reproduces kept dead stores only up to that read.
- **An inline helper called out of line at every site of one unit,
  while its owner expands it**, means that unit saw only a declaration
  (or lies across an original TU boundary); check all image call sites
  before blaming budget.
- **A local copy of a byte field in an `a == x || (x == K && ...)` chain**
  lets VC6 thread the known value (an extra jmp); read the field per use.
- **Re-sweep dead-probe counts after restoring a natural out-of-line
  call**: the freed budget changes the count.
- **A jne back to an earlier inline `return 0` with eax == 0 reused** is
  an else arm: `if (!f()) { ... } else { return 0; }` tail-merges into the
  earlier return; `if (f()) return 0;` gets its own epilogue.
- **A frame gap means a used local is larger or missing**, never dead
  padding (unused locals get no slot).
- **A callee's body expanded in a caller with the callee's own static
  pushed** means they shared an original TU (VC6 cannot inline across
  TUs): merge the units and a plain call reproduces the expansion
  (load_game / wipe_undo).
- **An expanded inline's result assigned straight to a global** can give
  one store per return arm; a named local then the global gives one store
  after the join.
- **Shared-literal proof of TU membership**: under unpooling, a function
  can push another function's literal only by expanding it in the same
  object — use this to confirm or rule out TU boundaries.
- **Replacing a raw predicate body with a call to it can be
  byte-identical at the site**: keep it for faithfulness; gains come where
  the raw body differed in register or CSE use.
- **Tooling: compare object sections order-insensitively** — VC6 shuffles
  small COMDAT section order between builds of the same source.
- **An original unit is one unbroken code run and one unbroken literal
  run in the image**; a source file spanning several long runs holds
  several original units, and 1-2 function runs are header/initializer
  copies.
- **Header inline vs merged unit**: if a function's copy sits in unit X's
  code, nothing in X calls it out of line, and a small caller elsewhere
  calls rather than expands it, it was a plain function of X; expansions
  elsewhere mean that code was compiled in X too (game_reload).
- **Window positioning through Buffer getters**: `Win::move(K -
  pixel_width(), ...)` loads width before the divide's cdq; raw
  `dib_.bmiHeader` reads do not (MultiWin::init).
- **The final teardown takes all leftover budget**: when the image's
  final teardown is shallow and ours deep, dead spend cannot fix it.
- **Retry jumps landing after a loop's zero test, action paths on it**:
  the source was `do { ... if (!x) break; } while (!exit);` with continue
  and break, not a for(;;) with gotos.
- **A dead store the image keeps on a member RECT** means the source
  wrote through a `RECT &` reference; direct member stores let VC6 drop
  it (Datalink::init).
- **A push one byte longer than ours (`mov ecx,[abs]` vs `mov eax,[abs]`)**
  after a parse call can come from the previous site's label_get vs
  written-out choice; compare lengths before blaming tables or extents.
- **Tool trap: `pgrep -f`/`pkill -f` in a wait loop match the loop's own
  command line**; wait on an output file instead.
- **Removing a whole original code run from a unit is safe**: the
  remaining functions' bytes do not move, unlike adding or removing
  single helpers.
- **A census "expanded in" flag can be a false positive** (sibling bodies
  of the same shape): confirm with a call-restore measurement.
- **VC6 fixes the evaluation order of a sum of calls**: permutations and
  parenthesisations compile identically.
- **A per-unit flat-member opt-in is safe for other units**: units on
  the old view keep identical code; the opted-in unit needs a literal
  rebind.
- **Read an expansion shortfall from one written-out site**: write out
  one earlier branchy expansion; if the target site then expands with no
  other call change, the shortfall is exactly one expansion.
- **A goto-shared block is laid out at the early goto** when written after
  a returning `if (!x) { ...; return; }`; written inside the early or late
  `if` it is laid out at the late site (propose_attack 648 -> 85 lines).
- **Teardown depth can be zero-sum between two sites**: a probe count
  cannot fix it; it needs more total budget (caller size).
- **A frame a few dwords short with fewer bitmask calls than the image**
  points to has_fac expansions refused for budget; check the inline's
  forwarding layers first.
- **Separate return-0/return-1 epilogues** come from one result variable
  assigned in an if/else and returned once; early returns cross-jump
  (Net::check_message).
- **Cross-jumping happens after register allocation**: identical tails
  merge only when their arguments share registers; respelling the tail
  does not stop a merge.
- **A jump-threaded `||` moves its block to the function end**: test the
  shared term once and `goto` into `if (!s) { reject: ... }` instead
  (propose_pact).
- **Auto-inlining is blocked by a named local pointer in the callee**:
  index the global directly (see_map_check).
- **Struct locals are scalarised** unless an inline takes their address;
  then the frame slots and dead field stores return.
- **`r = src; expand_rect(&r, K, K)`** gives `mov eax,K; add r,eax` with
  `add r,-K` immediates; per-field `src.f +/- k` gives a different load
  order (bottom, right, top, left instead of bottom, top, right, left).
- **Real getter expansions move a final teardown** where dead probes at
  the same spot do not (probe).
- **Declaration order inside an inlined helper can move registers in
  every caller**, unlike declaration order in the caller itself.
- **Refused StringList walks under STRINGSTRUCT_REMOVE_ALL_CALLED call
  `?remove_all_walk`**, which the image does not have; masked measurement
  hides the wrong relocation target.
- **Deleting an unused declaration from a wide header is a header change**:
  it shifts `$SG` numbering and can flip ties; vet it with
  `check-matches --since`, not object compares alone.
- **Shared `return 0` targets follow source order, not layout**: switch
  arms are laid out by case value, but an arm's conditional `return 0`
  joins a shared return-0 block only when the arm is written before that
  block's owning arm. When the image branches backwards into an earlier
  arm's epilogue, reorder the cases in source (window_proc).
- **A nested callee's inline decision follows its own spelling cost**:
  when a depth-2 expansion flips (menu_proc -> quit_game -> reset),
  adjust the inner callee's cost; caller spend does not move it.
  Accessors, chained assignments, memset vs loop and sub-struct
  references cost nothing; cast array-of-struct lvalues do.
- **The image can refuse a callee at an early site and expand it later
  in the same function** (go_to in enemy_move).
- **After merging a callee into its callers' unit, re-measure every
  caller whose hand-copies became calls**: a real call costs more /Ob2
  budget than the written-out body, and nested expansions through
  invented helper layers can flip (base_doctors).
- **Store order sets an induction pointer's anchor**: in a loop writing
  `A = P[f].a; B = P[f].b;`, VC6 anchors the walk on the last store's
  field and emits that store first; write b first to anchor on a.
- **A repeated block's register shape can come from the function, not
  the block**: if sibling functions show the same block in different
  image shapes, spelling the block will not help (ReportIf insets).
- **A helper called out of line at the first N sites and expanded at the
  rest means the source called the inline wrapper at every site**;
  hand-writing its body at some sites moves the cut-off
  (read_basic_rules).
- **`mov reg,K; add left,reg; sub right,reg` is an inline inset helper
  taking K**; hand-written `+= K` gives immediates.
- **A zero register one use short**: when the image keeps 0 in ebx and
  ours uses test/immediates, one dead zero store flips the allocation;
  the original had one more zero use that emits no code (unfound in
  MapWin::init).
- **A per-unit header twin emits its own out-of-line COMDAT where /Ob2
  refuses it**: bind it with RECOVERY_SHARED to the original body it
  duplicates (remove_all_walk -> 0x00402970), or strict reports an
  unbound reference.
- **Functions with RECOVERY_ASSOCIATED switch tables are checked on their
  whole evidence closure**, including the content of every vtable their
  inline teardowns reference; for an unexplained "strict selected
  evidence result: unknown", wrap match_recovery.source.asm.verify_records
  and dump its dict.
- **Separate early-return epilogues need a different final return**: a
  function ending in `return 0` cross-jumps every early `return 0` into
  it; put the shared failure block inside the last test's `if` (goto'd
  from earlier tests) and end with the success return (check_for_lobby).
- **A separate original unit is the only reason a first-site plain
  callee stays a call**: when one caller calls every same-unit plain
  function out of line while all other callers expand them, compile the
  caller alone and compare (base_build).
- **Two unused dwords just below right/bottom-style frame slots are a
  RECT whose left/top were scalarised**: build a RECT local and copy it
  (on_redraw_nc frame 0x5c -> 0x64).
- **A guard `je` to the shared epilogue with every callee-saved register
  pushed in the prologue** is `if (ok) { ... }`, not an early return
  (which lets VC6 defer pushes).
- **Negations computed straight into loop registers** come from
  `for (y = -y; ...)`, not separate negation statements.
- **A wrapped coordinate computed per arm and stored once after the
  join** is a written-out ternary over a temporary; assigning
  wrap_map_x() to a frame-homed variable stores in each arm.
- **An else arm deferred past the next jump** is a ternary assignment
  `v = c ? K : expr;`; if/else forms lay it inline.
- **A `T &r = Global[i]` reference stops VC6 moving a later load ahead of
  a store through r**; index the global directly when the image does.
- **Literal audit**: count `$SG` per unit against the image's literal run
  and list `$SG`s referenced more than once; a shared pair where the image
  has one per site is a helper the original wrote per site (macro or
  written out).
- **Literal order exposes negated chains**: when unpooled literal
  addresses put a later-laid-out arm's texts first, the source tested the
  negated condition first (`if (!A || !B) {X} else {Y}`), and a message
  duplicated across arms was one site (shoot, shoot_it).
- **`switch (v)` constant-propagates v into each case**; the image's
  hoisted `v*size` before the jump table is not reproduced by switching
  on a re-read field (MonuWin::on_redraw, unsolved).
- **Review every permute variant's hunk**: random search can rank a
  behaviour-changing mutation best (a dropped loop increment).
- **A local function-pointer call stand-in is not budget-neutral**:
  replacing it with a plain call changes later /Ob2 decisions; re-sweep
  dead probes after removing one.
- **A shared `return 0` EH teardown is emitted at the first return-0 site
  in layout order**: to place it after the last failure, chain the early
  sites as `if (A) {...} else if (B) {...} else { rest }` falling into one
  final `return 0` (gotos change the frame slot order) (join_session).
- **Goto-terminated then-blocks are deferred like goto targets**:
  `if (c) { msg; goto done; }` lays out like `if (c) goto L; ... L: msg;
  goto done;`, in the source order of the gotos; literal order shows
  where the original wrote them (order_veh).
- **`fld [var]; fmul [rdata]` means a float literal** (`var * 100.0f`):
  a named `const float` is loaded first whatever the operand order.
- **Separate saved values copied into a RECT with an unused dword beside
  them and `xor reg,reg` stored into left** were a `RECT saved` with
  left = 0 copied whole (MultiWin::draw).
- **Where a `= 0` initialisation sits decides whether VC6 keeps zero in a
  register.**
- **A `this`-to-base null check (neg/sbb/and) before an unrelated branch**
  means a named base-pointer local initialised there.
- **No `and ecx,0x1f` before `shl` means plain `1 << x`**; write it at each
  use when the image recomputes it.
- **To call a base-class inline out of line without a null check**, use a
  derived member pointer `int (Derived::*p)(...) = &Base::f;`.
- **A member RECT stored through `lea reg,[this+off]` from four registers
  is a whole copy from a local RECT**; `RECT r = *p` keeps a dead field
  store that a `RECT &` alone does not (DesignWin::init).
- **Merged `label_get` tails with an unfolded `add eax,K` index** are
  `strcat(StringTemp, label_get(...))` written in each arm.
- **Unfolded constants are separate statements** (`v -= C1;`); one
  expression folds them.
- **Comparison operand order is kept**: `cmp call_result, local; jle` is
  `if (f() > local)`.
- **Literal audit tool**: scratchpad wx/litaudit.py counts each
  function's image literal references against the object's `$SG`
  relocations; it caught a function pushing another function's objects
  (pick_map).
- **`#pragma auto_inline(off)` around a plain (non-`inline`) definition
  keeps it out of line** with plain call sites, reproducing a local
  function-pointer stand-in's object exactly; /Ob2 otherwise auto-inlines
  a plain function even when defined after its only caller. It does not
  affect explicit `inline` header functions (base_build/has_fac_built).
- **Byte reads of an enable mask let VC6 fold a later reload into a memory
  operand**; `is_human(f)` gives the image's separate load (chat).
- **BYTE_EXACT claims mask relocation targets**: a claimed function can
  still push the wrong object (another function's literal, a duplicate
  unbound stand-in for a bound pointer cell). Run scratchpad
  wx/litaudit_all.py, which compares each function's image references
  with ours as sets keyed by text or bound base (robust to reordering),
  including its section on exact claims.
- **A literal object referenced from a non-owner function means the
  owner was expanded there**; if the caller's references collapse to one
  site, the original called it at every site and VC6 merged the
  expansions (process_message: 18 end_of_event copies -> calls).
- **Message blocks written at their tests restore early teardown spend
  that probes were hiding** in big Popup functions (order_veh).
- **Under unpooling an expanded callee's literal keeps the callee's place
  in the image's literal run**: a literal ahead of its function's run
  means an earlier same-unit callee was expanded; call it instead of a
  hand copy (tech_achieved/tech_secret).
- **An inline's buffer sharing a slot with a caller buffer** means the
  caller's buffer was block-scoped.
- **Removing even unused declarations from a shared header renumbers
  `$SG` labels in every includer**: pair it with a project-wide
  rebind-literals.
- **A register-held 1 returned once at a failure join** is a flag local
  (`BOOL result = true; ... result = false; if (result) cleanup(); return
  result;`) (map_init).
- **A lone `dec eax` falling into a shared `return K` in a nested switch**
  is an explicit `case N: return K;` beside `default: return K;`.
- **A goto-target block written after the cleanup keeps the image's
  layout while its literals take the later source position** (tour).
- **A whole-map loop walking `add eax,0x2c`** is
  `tile = map_tiles(); for (...; i++, tile++)`; `map_tiles()[i]` reloads
  the base each iteration.
- **When the image enters one block from several arms, write it once**:
  VC6 does not merge our duplicated copies (world_rainfall).
- **`static const` arrays and const local statics go to .rdata**: drop
  `const` when the image object is in .data.
- **Header-inline literals emit as `??_C` COMDATs the linker merges**:
  bind them with `RECOVERY_LITERAL: v2 merged`, one per using unit.
- **Two identical literals in one unpooled function are two `$SG`
  objects**; one image object means the original passed it through one
  expression (an inline helper's parameter or a local pointer).
- **Case order from literal order**: in a big jump-table switch the
  image's unpooled literal order is the original case order (literals are
  emitted in parse order); reordering cases to match can restore
  cross-jumps that only happen in that order (process_message).
- **Shared-literal census** (scratchpad w1b10/census.py): a literal
  referenced by two image functions belongs to one; if our other
  function has its own copy of the text, it is a hand copy.
- **In big functions, rank candidates by image-only/ours-only diff
  lines, not permute distance** (register shifts dominate distance).
- **Shared tail-merge target**: when the image cross-jumps several tails
  into the last copy and ours into the first, write the tail once after the
  last site as a label and goto it (locals skipped by the goto must be
  declare-then-assign for C2362) (give_hints).
- **A fixed store just above a per-faction array inside its loop** is an
  out-of-bounds `arr[MaxPlayerNum] = ...` in the original; VC6 reserves
  that dword (drone_riot).
- **Probes cannot fix a destructor expanded at our early returns that the
  image calls**: if no probe changes the EH state count, spend is not the
  lever (make_gift).
- **Before moving a helper to another unit, check every same-unit caller
  that expands it** (0x402320).
- **Visible but unused header inlines flip ties in a unit**: gate new
  shared helpers per unit and fix flipped functions by spelling (a named
  temporary for a repeated call result).
- **An expanded same-unit function that also keeps an uncalled out-of-line
  copy** is `MEASURED inline` on the definition (plain definitions are not
  auto-inlined there); earlier declarations also need MEASURED (C2375).
- **A census "expansion" at fuzzy similarity (~0.8) needs confirmation**
  from writes to the callee's own globals (clear_council_notify was a
  false positive).
- **Small compare-chain switches are laid out by value**; source case
  order then shows only in literal order.
- **A search loop that breaks and re-tests `i >= N`** was `goto found`
  from inside the loop with the not-found code after it.
- **Call pushes hoisted above `test eax; je`** mean an if/else whose arms
  make the same call with the same trailing arguments.
- **An expanded inline re-reading its argument at each use** had a
  reference parameter (or was a macro); a virtual the image calls
  directly inside an inline was a qualified `Base::f()` call.
- **VC6 numbers a call's literal arguments right to left**: in
  `f("A", "B")`, "B" gets the lower `$SG` and address.
- **A literal inside an inline body is a `??_C` COMDAT even in unpooled
  units**, placed after the caller's own literals.
- **A callee tail `if (r != 0) return 0; return 1;` folds into the
  caller's `if (!f())` test when expanded**; `return r == 0;` leaves a
  sete/test pair.
- **Define new shared inlines in a narrow header**: one inline added to a
  wide header flipped an exact includer.
- **A head re-check after an inline seek survives only when the seek and
  the accessor go through different object expressions** (internal_receive).
- **Two arms ending in the same call with separate tests**:
  `if (c ? A && !f() : B && !f()) g();` keeps both f() calls and one g().
- **A scratch buffer in its own slot beside a block-scoped buffer** is
  function-scoped in the original (block scope lets VC6 overlap them).
- **Declaration count before a function couples to register ties**:
  removing N externs from an included header shifts every later `$SG` by
  N and can flip a tie in an unrelated includer (on_iface_button_clicked
  is exact only at its current count). Treat such flips as counter
  coupling; leave harmless unused declarations rather than pad.
- **Destructor-chain spelling is not a budget lever** (2026-11-16
  investigation): respelling ~Popup/~BasePop/~Scroll bodies changed
  nothing, every out-of-line destructor copy is exact, and 36 of 46 exact
  Popup functions need no probe. Teardown-depth probes are caller-local
  missing spend: look for natural accessor spellings per function.
- **File-local helper twins usually stand in for original header
  inlines** (expand_rect, StringStruct accessors, get_title/get_name,
  Spot::add RECT, has_fac layers): the image's out-of-line copies have
  one or two callers. Move them to a header only after per-unit
  measurement; header edits shift `$SG` counters, so rebind before
  reading losses.
- **has_fac calls bitmask directly** (image 0x00421670); the extra
  has_fac_built layer starved callers' budgets.
- **`if (!a || b) {T} else {E}` and `if (a && !b) {E} else {T}` compile
  identically** but emit literals in source order: let the image's literal
  order choose the arm written first.
- **A function exact only under /GF can be a tie on its count of distinct
  address constants** (ReportIf::draw_security/draw_intel flipped at the
  unpooled switch; sharing any two pairs restores them, but the image has
  distinct addresses, so the real source difference is still open).
- **An arm returning before a flag-tested join loses the shared tail**:
  let it fall to the join's `if (!flag) return;` (council_action).
- **Definition position never gates explicit inlines** (micro-tested
  2026-11-16): an `inline` callee defined after an inline caller or plain
  caller, even if first declared non-inline, is still expanded; VC6
  decides at the end of the unit.
- **Only an if/else assignment keeps a materialised setne**:
  `(x & 1) != 0`, `!!`, `?:` all fold back to the masked value; `if (x & 1)
  f = 1; else f = 0;` gives xor/test/setne at the arm's start.
- **A named call-result local swaps lea base/index**: write the call
  inside the expression, with unrelated stores after the statement.
- **A frame 4 bytes larger with an array fill starting at `[ebp-N+4]`** is
  an array declared one larger and indexed from 1.
- **Step a pointer-held RECT with `offset_rect(cell, 0, rect_height(&src) +
  K)`** when the image computes the step once; separate `+=` reloads.
- **One null check per base conversion through a member**: write
  `static_cast<Base *>(&member)` at each use; a cached two-step cast gives
  two checks; call base members as `member.Base::f()`.
- **A branchy push sequence after shared pushes is an if/else of two
  identical calls**; a `?:` argument compiles branchless (setcc).
- **Dead probes go stale**: re-sweep count and chain length whenever the
  unit changes.
- **/Ob2 per-callee cost excludes the call's argument expressions**: under
  pressure, source order decides which site is refused; member-access
  depth counts only inside the callee's own body. draw_active has spare
  budget, so its two out-of-line range calls (also at 0x004221BC/DB and
  0x0050AB85) are still unexplained.
- **A frame 4 bytes larger with one untouched dword just above an
  array's last element**: the array was declared one larger, indexed from
  0 (base_support).
- **VC6 propagates a constant only from compares on a named local**: if
  the image still runs a later test, the earlier compare read the field.
- **`c == K ? A : B` as a condition materialises sete/setl**; the image's
  branchy form is `(c != K || A) && (c == K || B)`.
- **One added goto shifts later `$SG` numbers**: losses elsewhere after it
  are counter coupling.
- **VC6 hoists a call nested in an argument list ahead of every other
  argument**: an argument the image evaluates before the call and spills
  was computed in an earlier statement.
- **Early-site teardowns DO respond to spend, if the spend is real**
  (corrects an earlier note): probes built as one long if-chain (>~4 ifs)
  are refused by /Ob2 and spend nothing, so older probe sweeps were
  invalid. Use many separate 1-3-if probe calls. Only the gap between two
  return sites matters; teardown decisions run in circular source order
  starting at the first return site after the last spend region, so the
  sites just before the spend lose depth first (pop_click: 8 three-if
  calls after the third return reproduce all five image teardowns).
  One-statement accessors (label_get, input_poll_state, set_value) spend
  budget too: recovery-added wrappers can over-spend (popb). Reproducer:
  scratchpad mx/.
- **Consecutive early returns before any call merge their teardown
  tails**; one combined return gets full per-site depth.
- **N adjacent words cleared with a fresh `xor eax,eax` + N stores although
  a zero register is live** are a short array-clear loop, unrolled (N = 4:
  battle_init inside battle_compute 0x00501DB4, Win::init_class,
  base_init); separate `= 0` stores reuse the live zero register.
- **The last `break` in source order places a loop's after-loop block**:
  end with `if (flag) { label: ...; return; } break;` to lay it last.
- **Mid-body nop runs matching a call sequence's length are post-link
  patches**: write the call; if only those bytes then differ, it is an
  exclusion candidate, not a matching target (filefind_init). An `__asm`
  nop block is not a stand-in: any `__asm` forces an ebp frame in VC6
  (measured 0.90, popup spilled). filefind_init stays unmatched until the
  cleanup pass (user decision, 2026-11-17).
- **A signed `jl` loop test against a strength-reduced element pointer** means
  the source bound was `i < N`; `i != N` gives `jne` (menu_init 0x0045FB20).
- **Block placement follows the successor, not source position.** A shared
  block reached by `goto` and written after the final `return` still lays
  out just before its successor (enemy_diplomacy 0x0055F930, identical bytes).
- **A frameless function (ebp as GPR) keeps a repeated 32-bit constant in a
  spare callee-saved register** where the image uses immediates; respelling
  the constant is folded first (WorldWin::on_redraw 0x004C3EC0, unresolved).
- **A dword-scaled index into a struct array member is a variable
  subscript.** When the image reads `PlayersData[f].satellites_[3]` as
  `[4*idx+disp]` with `idx = f*(stride/4)` in its own slot, separate from the
  byte-offset row used for the record's other fields, the source indexed with
  a non-constant (a local or an accessor parameter). A literal folds into the
  shared row (shoot 252 → 87 differing instructions, shoot_it 0.974 → 0.994).
  An inline accessor called with a constant does not reproduce it (VC6
  substitutes the constant before addressing); use a local, also for `-K`
  kept inside the index (`t = x - K`, base_build).
  Every header spelling folds (literal, `const` local, inline accessor with
  or without an index local inside it, row-pointer accessor: shoot 195
  against 64); only a non-constant subscript in the caller's body keeps it.
- **Null-test register rotation (guarded write_l/write_cent_l).** Each
  inlined `text == 0` test loads the StringTemp address into a register.
  VC6 cycles that load through eax, ecx and edx. With eax it emits
  `test eax,eax; je` (eax already holds the 0 return value); with ecx or edx
  it emits `jne; xor eax,eax; jmp`. One step of phase difference makes every
  later block differ. Compare the image's sequence of these loads against
  ours: the first mismatch shows which statement before it to move. The
  statement order just before a block sets the phase. `title = row;` before
  `expenses_help_rects_[n] = row;` fixed draw_expenses' energy and minerals
  panels, and `text = row;` before `expenses_help_rects_[5] = row;` fixed
  draw_energy_alloc (diff weight 85 -> 39). Declaration order and the order
  of RECT inset statements do not move it.
- **Flat-first ternary wrap inside a static helper.** If the image reloads the
  bound in each arm of an x-wrap, write the helper as the ternary
  `(MapIsFlat & 1) ? x : x < 0 ? x + W : x >= W ? x - W : x` and pass the sum
  directly, not through a stored intermediate (vehdraw_scoot_start).
- **Loop counter spilled to an extra frame slot in the image** where ours
  keeps it in a register: the loop body called an inline helper whose
  parameter copies add pressure. Wrapping a written-out test in a static
  inline made PlanWin::monocenter/opticenter exact.
- **If/else arms share frame slots; an early return does not.** Rewriting
  `if (a) {...; return r;} rest; return r;` as `if (a) {...} else {rest}
  return r;` moved get_nearest_palette_index's slots to the image's.
- **RECT inset/offset into a local** is `rc = src; expand_rect(&rc, -n, -n)`
  (or `offset_rect`); a `dst = src ± n` helper schedules differently
  (DiploWin::exec).
- **Cross-jump direction follows allocation.** When ours keeps the first of
  identical tails and the image the last, case bodies allocate identically in
  ours but not in the image; check in-case registers before respelling tails
  (vehdraw_scoot_start, monolith; observation).
- **A loop's exit tail written inside the loop schedules differently.**
  `for (;;) { if (ok) { tail; return p; } body }` loads and stores the tail
  in a different order from `while (!ok) { body } tail;`. Heap::get
  0x005D4680 became exact this way after six tail respellings tied.
- **Read the field, not a local copy, when the image reloads it.**
  `x = rec->f; if (x >= 0) y = x + 1;` keeps a `mov ecx,eax; inc ecx`
  copy; reading `rec->f` at each use gives the image's reload
  (vox_create_record 0x006393C0).
- **A single-site re-sweep moves one teardown level per step.** When the
  image keeps a different intermediate state rather than more or fewer,
  spend is not the lever (draw_social 0x00430090).
- **Tool trap:** `recover permute --candidates` replacements are scoped to
  the function body; a deck cannot add a helper before the function.
  Edit the file and `measure` instead.
- **VC5 file statics:** caviar's objects name file-static data
  `_name$S<n>`; v2 RECOVERY_OBJECT bindings on those statics do not
  resolve yet (framework gap), so referencing functions measure REFUSED.
- **Function-declaration counter window.** When extern-variable pads stop
  moving a counter-coupled tie, unused *function* declarations can
  (on_iface_button_clicked 0x004A6330: 7-10 work; <=6 and 12 do not).
  Three emitted static/global definitions or three unused `static inline`
  definitions flip it too. Each shifts `$SG` numbering: rebind literals.
- **The distinct-literal tie is in-function.** draw_security's register
  tie follows how many distinct literal constants the function itself
  holds; literal count elsewhere in the unit has no effect. A literal
  owned by an inline helper is one `$SG` shared by every expansion, so
  when the image has a distinct address per site the original did not
  hold that literal in a helper.
- **Fast unit loop without `recover`:** compile an alternate copy of a
  unit on a second prefix and compare masked, function by function,
  against the image (scratchpad/w6/cc.sh + cmpobj.py, ~4 s/variant).
- **An unused inline body can flip block layout in a non-caller.**
  Making expand_rect's body visible in win.h flipped buy_council_vote
  0x0053EB50 (faction.cpp never calls it) at any position in the header;
  gate such bodies per unit (`#define X_INLINE` before stdafx.h).
- **A gate macro alone can flip a tie.** Defining WIN_EXPAND_RECT_INLINE
  (or WIN_RECT_EXTENT_INLINE) in datalink.cpp without other changes flips
  Datalink::set_cat_social 0x0042B630; keep that unit's twin instead.
- **Spend rule, confirmed (control_turn 0x005275B0):** to make one early
  return site shallower, put the spend in the source gap right after that
  site (anywhere before the next return). Spend before it or after the
  next return does nothing. The count of one-if expansions matters (seven
  here), not how branchy each probe is.
- **Masked similarity 1.0 but strict extent fails:** look for a loop
  addressing-mode change, e.g. a pointer cursor in the image against a
  0-based offset plus absolute displacement (4 bytes per access).
- **`(uint8_t)x & bit` and `(x & 0xFF) & bit` differ:** the cast keeps an
  `and 0xff`-first order; the masked form gets reassociated (wants_prop).
- **Check the call sequence, not just the EH map.** A matching predecessor
  tuple can come with a different teardown composition: base_editor_copy
  with 6 x 3-if probes matches the tuple but calls `??_DDialogs` where the
  image calls `??1Dialogs`; 5 x 3-if gets both. Compare ordered call
  targets too.
- **Long single-chain probes can always be shortened** to 1-3-if calls
  with identical map and bytes; count and position decide, not chain
  length.
- **Hand-inlined accessors carry missing spend.** `parse_set(...)` then a
  `name_leader`/`title_leader`/`noun_faction` read is
  `get_name`/`get_title`/`get_noun`; restoring them supplied most of
  load_game's spend.
- **Fast probe loop:** compile the unit with its command from
  build/compile_commands.json and read FuncInfo straight from the object
  (follow the prologue handler push to `mov eax, FuncInfo`); compare with
  the image's and with call sequences (scratchpad/w2/ psweep, objtopo,
  imgtopo, callseq).
- **Tool trap:** editing any source in the worktree fails an in-flight
  `recover measure` on another file ("source changed during
  measurement"); sweep only when no recover command runs.
- **Local-array frame order is not a source lever.** Declaration order
  and names gave identical bytes (colortables_init 0x00423570).
- **A surviving dead store before `return` means a parameter home.**
  `mov [ebp+8],ecx; mov eax,ecx` just before returning: the accumulator
  shared a parameter's home slot (Buffer::text_width). The Buffer family
  (tile, tile_mask, write_raw_l, write_cent_l) shares this residual: the
  image reuses the homes of register-held parameters for temporaries.
  No reproducing spelling known yet.
- **Probes cannot make the first N sites call and later ones expand.**
  A full sweep never refused help_tech_info's early help_create_link
  sites; a call-site stand-in remains the only lever.
- **Tool traps:** `permute --candidates` refuses a replacement occurring
  more than once; editing a running permute's file kills it.
- **In-place global post-increment.** `if (G[t] >= N) return; int o =
  G[t]++;` inlines as `mov ecx,reg; inc reg; mov [G],reg` with the copy
  as index; loading into a local first gives `lea reg,[o+1]`. Separate
  subscripts `A[t*4+o]` (no shared index variable) let a constant t fold
  into the base displacement after inlining (add_bat 0x00501D50).
- **Signedness from the branch:** `jge` after a compare means a signed
  operand; an unsigned global compiles to `jae` (VehBattleModCount).
- **An inline with a call inside a loop flips the induction variable.**
  An expanded inline containing a call (has_fac -> bitmask) inside a loop
  over a global struct array makes VC6 walk it as offset + absolute
  displacement instead of a pointer cursor; the call written at the site,
  or a call-free inline, keeps the pointer (wants_prop 0x0052AEB0). A
  plain `Bases[i].f` loop's cursor is biased to &Bases[0].y (+2)
  whichever field is read first. Narrowed (w6 census): only four image
  facility tests walk a pointer cursor and our build already gets it in
  allocate_energy/proposal_menu, so the flip is local to wants_prop.
- **Dead parameter homes are a slot pool.** VC6 packs locals and
  temporaries into the homes of parameters that are dead or held in
  registers (435 exact functions; base_coast 0x0050DF30, soft_update
  0x005D5720, tile 0x005DA860). An unreferenced parameter's home is never
  reused (`a = a` doesn't count). One extra frame slot in ours usually
  means a live-range difference: move where a variable is assigned
  (declaration order is neutral). E.g. tile's `x = start;` before the
  outer loop and after the inner loop packs one slot fewer than at the
  top of the outer body.
- **A local living in a dead parameter's home can be that parameter**:
  `int &acc = *reinterpret_cast<int *>(&text);` folds to the parameter
  (WORKAROUND(type); text_width).
- **Pen in place:** `w = width(...); right = w + x; x += w;` not
  `next = width(...) + x; ...; x = next;` (write_raw_l 0x005DBD00).
- **if/else of two calls tail-merges** into one call with the argument in
  the image's register; a ternary argument adds a `mov eax, ecx`.
- **An expanded destructor at one later exit makes the -1 state.**
  Before hunting for a missing return, check which exit expands an inline
  destructor (session_picker_dialog 0x00631050).
- **A goto-only block written last** gets the highest EH states, yet VC6
  can lay it out right after the last test that jumps to it.
- **Zero-sum probe placement:** when no single position fixes two sites,
  a dead `if (0) { N stores }` block adds caller size without spend
  (5 stores in tech_trade 0x00541100; 1-4 did nothing).
- **Over-spend after an early exit** can be cut by writing a few header
  getters out at one later site (proposal_menu 0x0054DCF0).
- **REFUSED "unbound compiler artifact relocation(s) $SGnnnn" with
  diagnostic similarity 1.0** is usually exact code missing
  RECOVERY_LITERAL lines: unpooled repeated strings each have their own
  image copy, numbered in source order (garrison_click 0x0040B140).
- **Recovered inflate/inset helpers are expand_rect 0x00606F00**
  (left, right, top, bottom; an inset is a negated amount, emitted
  identically).
- **Accessor restoration is not uniformly a gain.** get_title/get_name/
  get_noun, parse_set and BasePop::item measured neutral at every site
  tried; label_get in Popup callers is real /Ob2 spend and often too much
  (base_editor_fac, popb). Measure each site. Census:
  scratchpad/w2/accessor_census.md (971 sites).
- **A GenderDefault store after the next call's pushes with a `lea eax`
  field load** comes from a named local assigned after parse_set, not
  the header getter (save_game); changing the getter itself loses four
  claims.
- **Tool traps:** psweep needs ANCHOR=<signature line> when the ORIGINAL
  marker sits in a `/* */` block; `pgrep -f pattern` in a wait loop
  whose own command line contains the pattern never exits.
- **An address-taken local is never pooled.** When the image reuses a
  dead slot for a counter our source keeps in an address-taken local
  (passed by `&` elsewhere), the original used a separate plain local
  (enemy_diplomacy 0x0055F930).
- **Read slot assignment from the `/FAs` listing**: VC6 prints
  `_name$ = -N` for every named frame slot, including inlined helper
  temps; unnamed slots are CSE temps. scratchpad/wx/slots.py maps image
  slots to ours; frame_census.md lists 36 frame/home-store candidates.
- **A frame difference can be strength reduction:** an extra slot with
  `cmp reg, 4*N` means a scaled twin of the induction variable; fixing
  the compare can fix the frame yet lose on registers (world_site).
- **ebp as a general register:** without `mov ebp, esp`, `[ebp+N]` is not
  a parameter home.
- **`lea eax,[reg-K]` as an argument from a dying variable** means a
  named local (`int t = v - K; f(t);`); `f(v - K)` gives `add reg,-K`
  (tech_achieved 0x005BB000).
- **`f(a) OP g(b)` evaluates the right call first** whatever the
  operator; name the left one to call it first.
- **A unit-wide out-of-line gate fixes a final teardown probes cannot**:
  when the image calls a header inline in its last teardown and no other
  function in the unit expands it, gate it out of line for the unit
  (STRINGSTRUCT_REMOVE_ALL_OUT_OF_LINE, proposal_menu 0x0054DCF0).
- **Declare loop scalars, then assign them in the image's store order**
  to change frame packing (make_gift 0x00547C10).
- **A goto label plus a missing return is an infinite loop.** In VC6
  (/O2 /GX) an int function containing any label that falls off its end
  compiles the fall-off as `jmp` to the first label, with no C4715
  warning (planet_busting 0x00500B60 looped forever after a strike). A
  void function, or one without labels, gets a normal epilogue. Never
  accept a score that depends on it.
- **Equivalent copy spellings coalesce identically** (`a = b = f()`,
  `a = f(); b = a;`, `int b = a;` at the use); copy spelling cannot
  separate two variables' slots.
- **Doubled-term reassociation:** VC6 turns `a + 2*a + 2*b` into
  `a + 2*(a+b)`; write `3*a + 2*b` for the image's lea chain
  (minerals_at 0x00591F00).
- **A tail-merged `return K` block is laid out at the last case returning
  K in source order;** move that case earlier to pull the block forward.
- **Member call through an embedded member:** `member_.f()` folds into
  `lea ecx,[this+off]`; a local pointer to the member gives
  `lea ecx,[ptr+off]` (the image's CSE'd base). Apply only to the calls
  that need it (ReplayWin::init).
- **Renaming a variable's separate assignments is a no-op:** VC6
  already splits independent live ranges itself. When the image keeps
  them in one slot (battle_fight 0x00506AF0), look for an original
  spelling where some use is reached by both assignments.
- **Fall-off census (2026-11-17):** planet_busting was the only labelled
  non-void function that could fall off its end; scratchpad/w1/
  falloff_census.md.
- **Unit-wide gates are rare (census 2026-11-17).** "Ours expands where
  the image calls" is not enough: the image may expand the same inline
  elsewhere in the unit. Gate only after compiling the whole unit with
  the inline declaration-only and recounting every function; every
  candidate except proposal_menu's remove_all was worse or neutral
  (scratchpad/w5/gate_census.md).
- **has_fac is one level in the original (census 2026-11-17):** bitmask
  0x0050BA00 is never expanded (263 calls); every expanded has_fac site
  is complete and no 2-arg has_fac_built exists out of line, so our
  nested has_fac -> has_fac_built shape produces partial expansions the
  image never has. Shape candidate: scratchpad/w6/hasfac_shape.patch.
- **A record pointer formed before a switch keeps the hoisted index.**
  `T *rec = &A[f].records_[type]; switch (type) { ... rec->field ... }`
  reproduces the image's hoisted type*size and [idx+base+disp]
  addressing (MonuWin::on_redraw); switching on a re-read field is worse.
- **goto vs break pairs duplicated return blocks:** in an inlined search
  loop, `break` groups the head-null exit with the found exit; `goto
  found;` before the shared return groups it with the exhausted exit
  (Dialog::set_selected_id 0x006099D0).
- **A reference local to an array element flips induction registers**
  (`T &b = arr[i]; b.f()` vs `arr[i].f()`; FameWin::init).
- **A mid-function `push ebx` after a loop guard** means the image uses
  ebx only inside the loop.
- **One variable as counter and result rotates spill slots function-wide;**
  split it (facilities_click 0x0040C070). A new local shifts `$SG`
  numbering: rebind the unit.
- **VC5 reorders x87 stores itself;** write them in source order
  (Caviar::set_shadow_vector).
- **A ternary inline lets VC6 thread a stack-walk exit**
  (`return id >= 0 ? next : id;` jumps straight out on a negative id; the
  if-form lands on the loop test). Wrap helpers reading the bound at each
  use give `lea [bounds+x]`. Two condition blocks sharing an exit: one
  `if` plus `else if`, not a goto past the second (order_veh 0x00531780).
- **Unexplained:** early `if (!x) return 0;` kept as a private epilogue
  in the image but merged into the final return-0 block in ours
  (facility_avail, drop_player); `setle al; inc eax` reusing eax=0 after
  has_abil (base_support).
- **Field read before a call, stored after it.** Read it into a const local
  before the call and store it afterwards; VC6 parks the local in a dead
  parameter home across the call, as the image does (SpriteBox::attach).
- **Address-taken RECT extents re-fetch the vbtable.** `r.bottom = a + y;
  r.right = b + x;` re-fetches the vbtable before the second store. Reading
  both extents into consts first gives one fetch, with the stores emitted in
  reverse source order (SpriteBox::draw_item).
- **VC5 `fstp [var]` then `fmul [var]` on an existing local's slot** means the
  original reassigned that local (`scale = axis_z * scale`), not a new
  temporary (vox_transform_camera).
- **Canonicalised forms (not levers).** `f(a) < f(b)` and its mirrored or
  negated forms give the same call order; `a * b` vs `b * a` compile
  identically.
- **Global register equilibrium in very large members.** If the image keeps
  only `this` in a callee-saved register and every named local in memory (no
  zero register), compare the prologue assignments (`mov esi, ecx`,
  `xor ebx, ebx`) first. Ours can settle in the other equilibrium (zero
  register, a hot local enregistered, `this` spilled); once spilled,
  `this + base + i*K` becomes one shared expression and `this` loses its uses.
  Index spellings fold identically, so they don't move it (setup_text).
- **Number-normalising diff filters hide wrong field offsets.** Compare
  relocation symbol plus displacement against the image address (an
  `assistant_city` vs `insult_leader` read only showed up that way).
- **Register with stack write-back is a split live range** (measured
  2026-11-13). When the image keeps a local V in ebx/esi/edi for part of a
  function and stores every assignment to a frame or dead-parameter slot
  (`mov ebx,K; mov [slot],ebx`, reloads at joins), VC6 split V: V had the
  register over its dense part and a shorter, hotter local took it in
  between (rule 9 of section 2.7). A memory-homed local is cached in a
  callee-saved register only where one is free over the whole cached stretch
  (rule 8), so if ours keeps V memory-only, another candidate holds that
  register across V's piece. List each register's occupants in the image
  and ours and fix the balance of uses (a missing or extra re-read, a named
  local the original re-read, an initialisation placed elsewhere): the
  victim flips on 2-4 uses. Declaration order, taking the address (real
  `&x` makes a local memory-only) and address-taking inside inline helpers
  do not produce or remove it. This is a whole-function equilibrium:
  diagnose, don't expect a one-line fix.
- **Spill reloads in exactly reversed order** at loop boundaries: a known
  blocker, so skip.

### 3.3 Frame and locals
- **Locals at `[ebp+8]`/`[ebp+0xc]`** sit in dead parameter slots: write ordinary
  locals and pass the parameters directly.
- **A frame larger than the locals.** Either a block-scoped buffer overlapped a
  later inlined local, or the original read fields directly instead of caching
  them.
- **Function vs block scope** is a lever for frame packing and for VC6's
  merging of identical code: with function-scope temporaries identical case
  bodies merge whole; block-scoped ones stop the merge but pack into a
  separate overlap region with a different slot order.
- **Same-size local arrays** ignore declaration order and names too; putting
  buffers in disjoint block scopes makes them overlap as the image does
  (ProdPicker::calculate's facility loop reusing the vehicle loop's buffers).
- **Same-type objects' slots** (several Popups and a GraphicWin) did not
  depend on declaration order or names across 240 variants.
- **Frame 4 bytes short with an untouched slot below a per-faction array:**
  the array was `[MaxPlayerNum]` indexed by faction id from 1, so element 0
  is never touched (CouncWin::init's support[]).
- **Frame-resident size pairs** (get_pcx_dimensions outputs at
  [ebp-0x14]/[ebp-0x10], width higher) are `int size[2]` read as
  `(&size[1], &size[0])`; named scalars move into dead parameter slots.
- **Buffer sizes.** Read them from the frame gap to the next local.
- **A local the image never stores** was uninitialized in the original.
- **A RECT in memory whose constant edges are stored through a register**
  (`mov ebx,0x15; mov [ebp-0x10],ebx; mov ebx,4; mov [ebp-0xc],ebx`) and
  whose left edge is reloaded while the other edges are folded is a struct
  copy followed by `offset_rect(&copy, 0, dy)`: `copy = rect;
  offset_rect(&copy, 0, dy);`. The source rectangle folds to constants, the
  copy's stores survive, and `left += 0` stops VC6 forwarding the left edge.
  No pointer-setter, reference, aggregate-initializer or POINT form keeps the
  rectangle in memory; a by-value helper return does, but reloads top too.
  After a struct copy, the first field modified with `+=` keeps its copy
  store and later ones lose theirs (ProdPicker::on_scrolled 0x00493CB0).
- **Two same-type locals in disjoint block scopes share one slot**
  (on_scrolled's two shifted rectangles in a 0x10 frame).
- **Frame-size differences: run `analysis/framescan.py` first** (13 s tree-wide);
  `--addr A` prints both slot maps with widths, `&` and our /Fa names. A frame
  that differs is a declaration problem, never a register tie.
- **Address-taken outputs of a call in an inner block, frame 8 short:** the
  original declared them at function scope, where they get their own slots
  instead of packing with the block's other locals (Console::upgrade 0x5400 ->
  0x5408 with get_pcx_dimensions' outputs; ReportWin::draw_score 0x178 ->
  0x180 with most_objectives' leader/pacts).
- **Several contiguous id slots read as one run** (`lea` of a middle element,
  each scalar read from its own slot) are one array: ReportWin::draw_labs keeps
  the tech copy, two prerequisites and four children as `int ids[7]`, not
  `children[4]` plus scalars (0x210 -> 0x218).
- **One slot more in the image for a member read once and reused** (a compiler
  CSE temporary): read the member in place at each use; a named `const` copy
  is propagated back into member reads and loses the slot
  (MapWin::pixel_to_tile 0x14 -> 0x18).

### 3.4 Arithmetic idioms
- **Signedness of a field (movsx/movzx, jl/jb, idiv/div, sar/shr) is its
  declaration, not a cast.** Run `analysis/typescan.py` (about 2 min): it lists
  the fields whose image uses contradict the declared type, with the casts that
  stand in for it. Retype the field (same offset and size) and drop the casts;
  round 9 retyped about 70 uint32_t fields to int this way with no function lost.
  A compare against an unsigned operand is unsigned whatever the field's type,
  `(x >> n) & 1` lowers to `shr` for int too, and a value stored to an int local
  first is signed whatever its source: only signed uses of a dword are strong
  evidence. `(flags & bit) != 0` is an int and lowers to `shr; and 1` like the
  shift (communicate's game-rules bit).
- **`rand() & 0x80000001; jns; dec; or -2; inc`** is `rand() % 2`.
- **A magic divide then multiply-subtract** is `x - x/C*C`, not `%`.
- **Ternary setcc forms.** `c ? K : 0` gives `set!c; dec; and K`;
  `!c ? 0 : K` gives `setc; dec; and -K; add K` (probe's `setle; dec; and 4`
  is `pop > 3 ? 4 : 0`).
- **Bool-to-flag forms differ.** `x ? 1 : 2` gives `neg/sbb/add 2`, while
  `2 - (x != 0)` gives `setne/sub`. `(c ? 1 : 0) + K` avoids `and ecx,0xff`.
- **Altitude tests.**
  - `& ~0x1F` gives a dword compare; `& 0xE0` gives a byte compare.
  - `climate >> 5` against an `ALT_*` level gives an int compare after a byte
    AND.
  - The same holds for every high-bit mask on a byte field. `val3 & 0xC0`
    narrows to `and al; cmp al; jb/jae`, while `val3 & ~0x3F` keeps the
    image's `xor; mov al; and; cmp eax; jl/jge`. Read the image's compare
    form at each site: enemy_move (0.921 -> 0.940), tour and order_veh
    gained, but probe's val3 test is a byte compare in the image.
  - A raw `(map_loc(x, y)->climate & ~0x1F) < 0x60` is `is_ocean(x, y)`
    expanded. Respelling usually compiles identically but can move /Ob2
    spend (base_build 0.9511 -> 0.9523); measure each function.
- **`cmp reg,v; jle; mov reg,other`, or `f()` called twice** is the windows.h
  `max`/`min` macro.
- **Float types.** A float temporary kept in x87 is `float`; `double` makes the
  constants qword. Check `.rdata` operand widths.

### 3.5 Behaviour bugs hidden as near-misses
- **Masking.** Masking hides wrong fields, callees, literals and loop-end
  addends for unbound targets. Check relocation targets before trusting a 0.99,
  because two pointer inductions can swap and still mask-match.
- **Common real deviations:**
  - swapped diplo indices;
  - LSB/MSB order;
  - inverted parent logic;
  - an off-by-one row count;
  - `is_human(f) + K` used without normalisation;
  - missing dialog callbacks.

  Restore the image's behaviour.

### 3.6 Canaries (measure after any edit to their unit or a shared header)
Counter-sensitive canaries carry `CANARY` lines; `uv run recover canary`
re-aligns their pad blocks (2.5). base_energy 0x004EB560 and give_a_base
0x0054CF40 flip from header content, not from the counter.
- **reportif.cpp:** `ReportIf::on_iface_button_clicked` 0x004A6330.
- **turn.cpp:** `end_of_game` 0x00524870.
- **console_commands.cpp:** `corner_the_market` 0x004E2000.
- **Header vetting:** `action_give`, `capture_base`, `setup_player`,
  `base_production`, `battle_fight`, `enemy_move`, `game_data`,
  `load_interface`, `SocialWin::init`, `tech_achieved`.

### 3.7 Rules from the 2026-11-18 parallel rounds

Each line names the image fingerprint, the source that reproduces it and one
function where it was measured. They are per-site choices unless stated.

Helpers and inlines
- All four coordinates loaded into registers before the first call of a
  distance expression: an inline helper. `map_range(x1,y1,x2,y2)` is
  `(abs(y1 - y2) + x_dist(x1, x2)) >> 1`; `map_dist` is
  `vector_dist(x_dist(x1,x2), abs(y1-y2))` (map.h). `!(( … ) & ~1)` and
  `( … & ~1) < 8` are the folded `map_range(...)` compares (enemy_move, 40
  sites; eleven exact functions stay exact with the helper).
- An inline helper written `if (c) { return a; } return b;` differs from the
  `?:` form and from the written-out chain (sort_sessions). For the stack step
  it is the reverse: `return id >= 0 ? Vehs[id].next_veh_id_stack : id;`
  leaves the loop straight from the negative test (veh_next_stack_id).
- A value formed before a call and adjusted after it is an inline taking that
  value as a parameter (FlatButton::on_redraw).
- A 6-byte `jmp [__imp__X]` among a unit's own COMDATs with one caller is an
  inline wrapper, expanded at early sites and called once budget is gone
  (probe, 0x005A58D0).
- Expression-only inlines cost no /Ob2 budget; an inline with an `if`
  statement costs one flat unit. Explicit casts spend budget too: retyping
  `Buffer::locked_bits_` to drop casts made Buffer::copy expand a call the
  image keeps out of line. Nested inline levels decide which `range` stays a
  call (draw_active).
- Accessor against raw member read (`Buffer::pixel_width()` against
  `dib_.bmiHeader.biWidth`) changes CSE and load order at some sites and is
  neutral at most; the image rejects it at several exact sites.
- A virtual-base receiver adjusted once with no null test and reused across
  two calls comes only from a member body of the base class (BasePop::exec).

Registers and scratch choice
- A named local for a global or field read, against the bare argument
  expression, changes scratch-register choice, sometimes far downstream
  (`HWND window = HandleMain;` in UV2Player::play). Dropping a named temporary
  can do the same (show_post_movie_text).
- `f(c ? a : b)` against `if (c) f(a); else f(b);` changes which variable
  gets the callee-saved register (draw_social). `x = m; if (x == -1) x = d;`
  ranks `x` first; `x = m == -1 ? d : m` does not (on_redraw_nc).
- A value known from a branch stays with the tested symbol, not its copy:
  `const int r = f(); v = r; if (r != 0) continue;` keeps `mov v,eax; cmp`
  and the block in place (top_menu).
- `if (c) m |= K;` leaves nothing to forward; `if (c) { p |= K; m = p; }`
  makes later inlined reads use p's register (Win::init).
- A member pointer read at every use, against a local reassigned from the
  member, changes subtraction operand order (on_redraw_nc).
- A named per-iteration index `int k = i + C; A[k].f` gives a byte-offset IV
  off the table (`cmp reg,(N+C)*stride; jl`); `A[i + C].f` at each use gives a
  pointer IV tested against the table address (read_rules 0x00587466).
- x87: `fstp st(N)` followed by pops before a libm call means the argument
  was rounded through a `float` local (`float sq = …; sqrt(sq)`); only
  `fstp st(0)` pops and a direct `fstp qword [esp]` mean it was not
  (Vector::normalize 0x00634520).
- `inc eax; je` after a call is `f() + 1 == 0`; `== -1` gives `cmp eax,-1`.
- A `v = 0` before a large switch can become a function-wide zero register.

Frame and slots
- Slots of scalar locals are ordered by reference count, most used nearest
  ebp; slot-only rows are a symptom of a structural difference elsewhere.
- Address-taken locals with slots of their own are function-scope; declared
  in disjoint blocks they share a slot (base_production, +12 bytes).
- A local stored to its slot at every assignment while cached in a register
  is one function-scope variable shared across blocks (alien_move 194 -> 4).
  The reverse is not safe: check the frame size.
- Separate locals with disjoint live ranges share a slot, and compiler
  temporaries share those slots; a variable reused for several roles and read
  unwritten on some path forces an extra slot (enemy_move).
- A dead `int[3]` is overlaid by a later RECT; a hole plus two stored fields
  identifies a scalarised RECT (show_post_movie_text).
- `mov dl,[slot]; or [mem],dl` is a named `int bit = 1 << n`; `x |= 1 << n`
  on a byte recomputes a narrowed shift.

Rectangles
- Stores through one register from single loads are
  `set_rectangle_dimensions(ptr, x, y, w, h)`; field statements reread the
  sources. An address held across a loop is a `RECT *` local (draw_sat).
- Paired `r.top += d; r.bottom += d` that load both edges first are
  `offset_rect(&r, 0, d)` expanded (BaseWin::init).

Layout
- A far `je` to a fallback call at the function end is
  `if (c) { body } else { fallback(); }`, not an early return.
- A guard that is the negation of a later admission test is written as that
  negation, `if (!((a && b) || (c && d && e))) return;` (sat_attack).
- `switch (x) { case K: …; return; default: break; }` before the closing
  brace gives `je` over the closing teardown; `if (x == K) { …; return; }`
  gives `jne` and the opposite placement (use_ability).
- `while (a && b && c) body` tests `a` first, then re-tests `a` at the bottom
  and jumps back into `b` (write_pcx).
- Two cases ending in the same tail: `return;` in both keeps the code in the
  last copy, `break;` (switch last in the function) keeps it in the first
  (on_left_click).
- `if (!(a && b && c && d)) return 0;` with a teardown is parked behind the
  final `ret`; a helper call `if (!on_map(x, y)) return 0;` stays in place.
- `if (c) goto L; …; L:` and `if (!c) { … }` lay out differently.
  `if (c >= 0) goto L; B; goto M; L:` defers B; `if (c < 0) { B; goto M; }`
  keeps it inline (config_game).
- Blocks reached only through labels are placed by control flow; their source
  order has no effect (order_veh, 24 permutations).
- Loop-body arms that end by jumping to the loop's continue point are held
  back and emitted right after the next unconditional `jmp` in layout order. A
  post-loop `if` whose taken path skips a reload of a cached value supplies
  that `jmp`; without it the arms land much later (enemy_move passenger loop:
  a no-op else after `if (!hint)` moves the group to the image's place,
  5383 -> 4960 rows, so the image's jmp comes from a register cache, not a
  source statement).
- Tail merging follows the closing teardown depth and the per-site scratch
  register, so which copy survives can follow a register tie.
- **Both arms end with their own tail and goto** (communicate 0x0054FFD0,
  1925 -> 1538 rows): when the image ends an else arm with a jump back into
  the then arm's tail, the original wrote the tail and its goto in both arms
  and VC6 cross-jumped them; one shared tail after the if/else puts the join
  after the second arm. It does not extend to tails with a loop: save_game's
  scenario arm with `state |= 0x10; for (...)` written in both arms keeps two
  full copies (49 -> 106 rows).
- **Cross-jump target (measured, `analysis/tailmerge/tm.py`, 2026-11-19).**
  A jumping tail is cross-jumped into the block that falls through into the
  common target, greedily, at any length (one `add esp,4` counts). Tails that
  are identical to each other but not to that block stay separate: six switch
  cases ending `push reg; call f; add esp,8; call g` (reg differs) merge only
  with the case laid out last. Jumper-into-jumper merges happen when the tail
  carries one deferred stack pop over two argument-pushing calls
  (`push 0; call f; push 1; call g; add esp,8`, tm3.py T1/T3/T14); then the
  instructions before the merged point are copied into each predecessor arm,
  so a store after an inline's if/else appears in both arms. One pushing call
  (`push; call f; add esp,4; call g`, T2/T4/T9/T15) stays unmerged.
- **The join's head block follows its most-shared tail.** VC6 moves the
  first block of a join (probe 0x5A36EE: the prb_defense ternary's test and
  has_fac arm, ending `jmp` to the merge at 0x5A42CC) after the predecessor
  whose tail the most other predecessors share, and merges them into it. Ours
  puts it after action 3's GOTENERGY NetMsg::pop and merges six pop tails;
  making the pop tails unmergeable (a distinct store after each) puts it after
  action 2's draw_tile as in the image, where action 7's popp `add esp,0xc`
  merges into draw_tile's (1881 -> 1783 rows even with the stores). The image
  keeps all seven pop tails, so they were not mergeable in the original: not
  explained by a non-inline callee (an extern twin merges too), by distinct
  callees (their shared `add esp,8` still pulls the join), or by the
  ternary's spelling (`(a && b) ? 2 : 0` gives 1825 rows, same placement).
- **Jumper-into-jumper merges in ours are a mismatch signal.**
  `analysis/tailmerge/jjcensus.py` (heuristic: a `jmp` into the middle of a
  statement whose block then ends with `jmp`) finds them in 33 of 110
  non-exact functions but 15 of 626 exact ones in ten large units. The
  original compiler makes them (garrison_click, new_base are exact with
  them), but menu_proc's toggle tails (prefs_save/draw_map, 0x51C4AD..),
  use_ability's first lock-return teardown (0x4D842A), interlude,
  on_key_click and propose_attack merge in ours only. Respelling the site has
  not changed one: the cause is elsewhere in the function ("Tail
  cross-jumping depends on the whole function", 3.2); menu_proc stays merged
  with its three written-out inline stand-ins replaced by calls.
- **Cross-jump direction is not a source-order choice for if/else.** VC6
  gives identical code for `if (c) A else B`, the inverted test, a ternary
  statement, a two-arm switch and goto blocks written after the final
  return (save_game 0x005A9EB0: the image hosts strcpy in the then arm and
  holds the else arm, `push default; jmp back`, after the next unconditional
  jmp; ours hosts it in the else arm; all five spellings measure 49 rows).
- **Cross-jumping merges the longest identical instruction suffix**, after
  allocation (2026-11-19 par-1). battle_fight 0x50ADDC merges two
  `eliminate_player` tails from the `call` on, after pushes from different
  registers (ecx/edx against eax/ecx). So a call-count gap from a tail merge
  is usually a register difference one or two instructions before the call:
  - propose_pact 0x543BC0: the image keeps `pact_of_brotherhood; return 1`
    private in both net_set_treaty arms because myFac is in edi there and
    reloaded into ebx at the final site (edi = 0 there); ours keeps myFac in
    ebx throughout, so all three merge. Fix the allocation (2.7), not the tail.
  - MapWin::draw_map 0x46A550: the image's first seam arm uses ebx (dy) as
    scratch and reloads dy from a spill after its fill call (one more frame
    slot, 0x60 against 0x5C), which keeps that call private.
  - Buffer::write_strings_height 0x5DAD70: the reverse; ours reloads ebp and
    re-zeros edi after arm 2's call, so ours keeps three wrap_height_flying
    calls where the image shares one.
  - Not explained: menu_proc 0x51BC70 keeps four identical
    `call prefs_save; push 1; call draw_map; add esp,8; jmp 51C23B` tails
    (ours merges them into base_names' copy). Neutral in ours: up to 128 dead
    calls, dead locals, 64 extra cases, the toggles written out by hand, a
    `{ int z = 0; prefs_save(z); }` argument. Only `return` in place of `break`
    unmerges (wrong target). Since bytes and target are equal at the end, the
    targets must have differed when VC6 merged. Likewise Console::go_to
    0x4D61A0: the image cross-jumps the three Psi Gate `NetMsgPrimary.pop`
    calls into the first and lays the `finish:` unlock right after it; ours
    keeps three identical `mov ecx; call pop; jmp finish` copies.
- **Which predecessor hosts a join (measured, `analysis/tailmerge/tm7*.py`,
  2026-11-19).** When no arm falls into the join in source order (the last
  arm leaves with `return`/`goto`), VC6 places the join after one
  predecessor and cross-jumps into it only the tails identical to that
  host. Tails identical only to each other stay as separate copies, so
  "the image keeps N identical tails" usually means "the image chose a
  different host", not "something blocked the merge".
  1. Default: the **first arm in source order whose tail has a partner**
     (any other predecessor ending in the same last instruction, one
     `add esp,N` is enough). The partner's length and how many partners there
     are do not matter: `FFPPPPP` hosts F with one 1-instruction partner over
     five 2-instruction P tails (tm7g, tm7h `AAPPP`, `QAPPA`). With no
     partner at all the first arm hosts.
  2. Priority: a tail that ends the then-arm of an if/else inside an arm,
     where the else does not end in that tail, promotes its pair over every
     earlier pair (tm7h `QSsWB`, `QSYWB`, `QSNWB`; `QSWsB` hosts the earlier
     member W of that pair). A tail in a lone `if`, or in the else-arm,
     gets no priority (`QSnWB`, `QSeWB`). Inverting the test does not change
     this (VC6 normalizes the if/else): probe's action-3 GOTENERGY pop
     measured the same both ways.
  3. Deferred pops decide partnership. A call before the tail whose
     `add esp` was deferred into the tail's makes it `add esp,0x10` where a
     tail after a branch has `add esp,0xc` (the branch flushes pending pops),
     so an arm-level straight-line tail is not a partner of a tail inside an
     `if`, but an arm-level tail after an inner if/else is (`QAPPB` hosts P,
     `QSPPB`/`QSWWB` host S). This is the only blocker found that is
     invisible in the source and visible in the bytes.
- **Not blockers (each measured with the tm7 families):** a function-scope
  object with a destructor, an empty inline destructor inside or around
  the tail, frame size / `__chkstk` (12 KB local), function size (2,500 filler
  blocks before or after), the number of merges in the function (400 pairs,
  no cap), an inline wrapper or a forwarding overload around the call, a
  member vs static call, distinct callees (they still merge at `add esp`),
  an ignored return value, a `goto` label at the tail start, codeless
  statements after the tail (`;`, empty `if`, dead local, `do {} while (0)`),
  constant vs global arguments.
- **A source label after an if/else blocks the jumper-into-jumper merge of
  its arms** (tm7m `label_after`, `label_in_then`: `use(k); f2(k, n);` in both
  arms stays as two copies; a switch end, loop end or `break` does not block
  it). It does not block a merge into a fallthrough host (ternary-argument
  pops, tm7h `QSKWB`). Worth a try where ours makes a JJ merge the image
  lacks and the original plausibly had a `goto` target there.
- **probe 0x0059F120 is still open.** The image hosts the join after action
  2's draw_tile (partner: action 7's popp `add esp,0xc`) and keeps seven
  pop tails; by rules 1-2 the original's action-3 GOTENERGY tail (a then-arm,
  partner TOOKENERGY / ASSASSINATED) would have won, so the original's pop
  tails were not partners. Every invisible factor above was tried in place
  (labels, nested else-if, the inverted test, a pcx local); only a distinct
  store after each pop moves the host (1877 -> 1778 rows).

Types
- Wrong declarations hide as casts: a member cast at each use, a byte buffer
  written through `memcpy(bytes + offsetof(...))`, `this + 1`. Declare the
  union, nested record or trailing array; the code is usually unchanged.
- A signed `-1` store into a byte array gives `or r8,0xFF`; the image's
  `mov r8,0xFF` needs `unsigned char` (TerrainCacheEntry::altitude).

Constant registers (measured with standalone probes, 2026-11-18)
- VC6 dedicates a callee-saved register to a constant (zero, -1, 5, …) once
  the function has four memory stores of it. Any width counts; a store in a
  loop counts double, in a nested loop four times. The count is taken before
  head-merging and dead-store elimination, so identical leading stores in both
  arms of an if, a store overwritten later in the block, and a store inside an
  expanded inline all count. Pushes, compares, `test reg,reg` and
  register-local initialisers never count, but use the register once it
  exists (`push ebx`, `cmp [mem],ebx`, `mov [mem],bl`).
- It needs a free callee-saved register: three live locals deny it at four
  stores and it wins at eight. /Oy only adds ebp as a candidate.
- A named `int z = 0` never reassigned equals the literal. A zero-initialised
  local that keeps a callee-saved register is used as the zero while known
  zero.
- So a function-wide `xor ebx,ebx` the source lacks is a missing fourth store
  (BaseWin::on_button_clicked: clearing StringTemp in each arm of a toggle,
  hoisted by VC6 into one store, 62 rows -> exact); a zero register the image
  lacks is one store too many or lost register competition
  (InfoWin::setup_text: 67 immediate zero stores, no register).
- `recovery/tools/analysis/constreg.py` lists every non-exact function where
  the image and ours disagree about a constant register (IMG_ONLY, OURS_ONLY,
  START, REG) with the stretch's uses and both sides' immediate counts.
- Clears that VC6 copies into each return's epilogue are one source store. A
  function whose image repeats `mov [flag],0` before several `ret`s and has
  no zero register had one closing clear reached by fallthrough; spelling
  `flag = 0; return;` in each arm adds stores and buys the register
  (Net::check_polling: four spelled clears -> ebx zero from the prologue,
  one closing clear in an if / else if / else -> exact).
- The count belongs to the stretch where a register is free. In interlude a
  probe store of -1 before the loops gave edi = -1 from the prologue; the same
  store after the loops gave the image's esi = -1 tail.
- EH state stores count: `mov [ebp-4],-1` at each return is a store of -1.
  Once a register holds -1 the last teardown writes `mov [ebp-4],reg`, which
  no longer equals the other returns' immediate stores, so the return tails
  stay unmerged (interlude: four immediate state stores plus one from esi).
- Not counted in MapWin::init trials: the same store twice in a row, a zero
  store followed by a ternary assignment to the same member, pushes (16
  added) and memory compares (8 added). A live extra store did count there.
- VC6 evaluates `a + b - c` as `(b - c) + a` whatever the spelling; a
  `(pos + pos) - neg` tree in an image is not a plain three-term chain.
- `if (c) { …; return 0; } …; return 0;` folds a final
  `if (f() == 0) return 1; return 0;` into neg/sbb/inc; one if/else falling
  to the closing return keeps `test; jne`.
- **No intrinsics (pass 4).** Outside Caviar/UV the image has 125
  `call _memcpy`, 69 `call _memset` and no expansion of any intrinsic (no
  `repne scasb`, `fsqrt`, `fsin`, `fabs`, no variable-size `rep movsd` +
  `rep movsb`). Dropping /Oi with every per-site pragma removed kept 5,052 of
  5,093 exact functions; the 41 others used an expanding `memset`/`memcpy`
  as a stand-in and all became exact as loops/assignments.
  `recovery/tools/crtcalls.py` compares the CRT calls per function in ours
  and in the image; they must be equal.
- **Loop fission order.** One loop storing to two arrays becomes two block
  operations in reverse statement order (`for (...) { A[i] = 0; B[i] = 0; }`
  clears B first: clear_tamper 0x0051E390, clear_council_notify).
- **`do { } while (++k < N)` costs less inline budget than `for`.** map_wipe's
  inner 8-word clear written as `for` made map_resample (0x00592B40) call it
  instead of expanding it; the do/while form is expanded.
- **A new loop variable can shift a budget-sensitive caller**: reuse the
  function's existing counter (BaseWin::production 577 -> 117 rows).
- **`#pragma optimize("y", off)` is a no-op in a /Oy- unit** (51 regions
  removed, no function changed). Where it is load-bearing the function sits
  at a game address but lives in a unit built /Oy: fix the unit's flag or
  move the body to the header that the game unit instantiates.

## 4. Measured and neutral (do not retry without new evidence)

- **String pooling.** The original is unpooled everywhere: identical literals stay
  separate within a function. Converting 117 units to an unpooled recipe
  (`/Og /Oi /Ot [/Oy] /Gs /Gy /Ob2`) gained 0 functions. Pooling is decisive
  only where it changes whether identical pushes are hoisted.
- **Compiler build.** The Rich header shows VC6 C++ builds 8168 (×126) and 8447
  (×66), C 8168/8447, MASM, and untagged VC5 Caviar objects. Rebuilding the
  whole tree with the SP3 backend left the library range (≥0x5C9EC0) unchanged
  and lost 27 game functions. Library identity is evidence for TU and library
  boundaries, not a matching lever.
- **Processor flags per unit.** `/G5`, `/GB`, `/G4` and `/G3` compile
  identically to the recipe in all 182 game units. `/G6` changes 175 units but
  made 0 functions exact and lost 2,390 of 5,015 when applied per unit; only
  movie_media_decoder_g6.cpp is a real `/G6` unit (2026-11-12 sweep).
- **TU counter shifts.** Inserting 1-8 extra extern/function declarations,
  unused literals, unused inlines, referenced externs or helper expansions at
  the top of a unit changed no non-exact function in 6 units (204 variants,
  49 near-misses). Near-misses are not decided by $SG/$L or declaration
  counters; they need per-function body, type or structure work. Such shifts
  only ever *break* exact functions, non-monotonically (end_of_game flips at
  7-8 externs but not 1-6) (2026-11-12 sweep).
- **Flags.** `/Ob1`, `inline_depth` and other `/O` variations either change
  nothing or lose claims.
- **Inline keywords.** `__forceinline` vs `inline` changes nothing. The per-site
  spelling does (written-out `StringTable.get(Labels.strings_ptr[k])` vs
  `label_get(k)`).
- **Declaration scope of a scalar local.** Moving it between block and function
  scope (either order) compiles identically (alien_move). Sharing one variable
  between two scans does change allocation, and that measured worse.
- **Sum term order.** A sum of three member loads is canonicalised: reordering
  the terms, or naming one term, emits the same load order (sub_5cbe30).
- **Early-return else blocks.** `else { if (c) return; }` and `else if (c)
  return;` compile identically (sub_5cc940). Folding two COM calls into one
  HRESULT local merges the calls: a different program.
- **`push imm` around an address `add`.** A cast, parentheses or a literal for
  a constant-propagated variable does not move the push (sub_5cb220).
- **Switch default and case layout.** With a fall-through group plus a small
  separate case, default always targets the epilogue copy of the last laid-out
  case. Default position, `default: return`, a `return` in the small case or a
  goto tail all compile identically (SocialWin::on_button_clicked). Case blocks
  follow source order.
- **Identical code in two cases is merged into the textually last copy.**
  VC6 keeps the last copy, makes the earlier case jump into it, moves that
  last case up behind the jump, lets the code after the switch follow it,
  and emits the cases written in between after the epilogue. So when the
  image shows what looks like a fall-through (`case 2` running into
  `case 7`) followed directly by the code after the switch, with other
  case blocks after the `ret` jumping back, or a `default` that targets
  the first of two epilogue copies, the source did not fall through. Each
  case wrote the shared statements itself and the second one is last in
  the switch:
  `case 2: if (x) return; f = 7; break; case 4: ... case 7: f = 7; break;`.
  The shared code is often an inlined helper (ReportIf::on_status:
  `case 3: close_energy(); ... case 0: close_intel();`, where
  close_energy ends with close_intel's two calls). A real fall-through,
  any case or `default` order, a `break`/`goto` join, a flag, a nested
  switch and a loop all keep every case before the tail. An empty inline
  helper as a case body (`case 8: close_score(); break;`) keeps that case
  in the jump table's range, which a bare `case 8: break;` does not.
  Solved ProdPicker::on_button_clicked 0x004945D0 and
  SocialWin::on_button_clicked 0x004B1E00.
- **`movsx` before `and 1`.** VC6 narrows `(char)x & 1` and
  `*(signed char *)&x & 1` to `mov reg8`, so an image `movsx` there needs
  another explanation (scenario_setup, unresolved).
- **Permute distance is coarse.** Spellings with visibly different code can
  tie; confirm with `recover diff` before discarding. Slow units (socialwin,
  about 8 s per compile) need hand decks rather than random permutes.
- **Commutative operand order.** Swapping or regrouping `+`/`*` operands among
  calls, globals and locals compiled identically in every test (12 orders in
  PullDown::compute_length, calculate_dimensions, PullDown::on_redraw,
  ListBox::UNK1). Read the image's operand order as an allocation outcome;
  this qualifies the "a + b of globals loads b first" note in section 2.
- **Mask-test spellings.** `x & C`, `(x & C) != 0`, `(x >> k) & 1` and an
  accessor all give `test dword ptr [m], C` (world_rainfall). An image
  `test byte ptr [m+k]` comes from the field's declared type or layout.
- **Unresolved blockers** (not proof that no source form exists):
  - **`??_H` inside a struct:** VC6 expands the vector constructor iterator
    inline even under `auto_inline(off)`.
  - **Dead-store counters:** a counter VC6 deletes but the image keeps
    (base_growth).
  - **Reversed spill reloads:** spill reloads in exactly reversed order at loop
    boundaries.
