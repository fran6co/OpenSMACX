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
- **Giant callers: the budget falls with caller size (2026-11-19 par-2).**
  Probe (scratch `g2.py`/`forms.py` pattern: M stores `g3 = K;`, then N
  calls of an inline 2-if or 4-if callee, count the expansions): with 600
  4-if sites the count rises with M (29 at M=0, 128 at 1000, 286 at 2000),
  then falls by one 4-if expansion per ~20 stores (240 at 3000, 141 at
  5000, 42 at 7000) down to the B0 floor (12 at 7500 and beyond). So
  B ≈ min(k·S, L − S') with a floor: past the peak every node the caller
  writes removes room, live or dead, wherever it sits. battle_fight
  (19.9 KB) and the other giants are on the falling side; that is 2.8's
  "In a giant, caller size takes room away".
- **What a statement costs (the caller-size side, 2026-11-19 par-2).**
  Measured on the falling side (M=4000, N=900 2-if sites, 200-400 copies
  of each form; resolution about 0.05 store), in units of one
  `g3 = K;` store. The steps come in quarters, so a quarter is about one
  front-end node. Forms that compile to the same code can still cost
  different amounts, and in a giant each difference moves late sites:
  | form | cost | cheaper equivalent | cost |
  | --- | --- | --- | --- |
  | `;` / declaration without initializer | 0 | | |
  | `{}` / `{ int t; }` | 0.5 | | |
  | `g = K;` | 1.0 | | |
  | `g = g2;` | 1.25 | | |
  | `use(K);` / `g = f();` / `g = -x;` | 1.5 | | |
  | `use(g2);` / `use(((g2)))` (parentheses are free) | 1.75 | | |
  | `use((int)g2)` (int to int) | 2.0 | `use(g2)` | 1.75 |
  | `use((int)gs)` (short) | 2.0 | `use(gs)`: the implicit conversion costs the same | 2.0 |
  | `use((short)g2)` | 2.25 | | |
  | `t = g2; use(t);` (local copy) | 3.0 | `use(g2);` | 1.75 |
  | `{ int t = g2; use(t); }` | 3.5 | `use(g2);` | 1.75 |
  | `g6 = g6 + 1;` | 1.75 | `g6 += 1;` / `g6++;` / `++g6;` | 1.0 |
  | `if (g4 != 0) use(K);` / `if (g4 == 0) ...` | 4.0 | `if (g4)` / `if (!g4)` (`!` is free) | 3.5 |
  | `if (f() != 0) use(K);` | 4.2 | `if (f())` | 3.7 |
  | `if (c) { use(1); }` | 4.5 | `if (c) use(1);` | 4.0 |
  | `if (c) { use(1); } else { use(2); }` | ≥7.9 | `if (c) use(1); else use(2);` | 6.5 |
  | `if (a) if (b) use(1);` | 6.5 | `if (a && b) use(1);` | 5.25 |
  | `if (a) use(1); else if (b) use(1);` | 8.85 | `if (a \|\| b) use(1);` | 5.4 |
  | `if (c) g3 = 1; else g3 = 2;` | 5.5 | `g3 = c ? 1 : 2;` | 2.75 |
  | `if (c) { use(1); return; }` | 4.75 | (`if (c) return;` alone 2.75) | |
  | `t = a * b; g3 = t + K; g7 = t + 2;` | 5.5 | the product written twice | 5.0 |
  | `g3 = s.x;` (one `.` or `->`) | 1.75 | `g3 = g2;` | 1.25 |
  | `g3 = a.b.c.d.x;` (four levels; a union member costs the same) | 3.2 | | |
  | `if (a.b.c.d.x & (1 << k)) use(K);` | 6.65 | the same on a plain global | 4.7 |
  | `g3 = ga[K];` / `ga[g2]` / `ga[g2 + 1]` / `ga[g2 * 4 + 1]` | 1.75 / 2.5 / 3.0 / 3.5 | | |
  | `g3 = gv[g2].a;` | 3.0 | | |
  | `p = &gv[g2]; g3 = p->a;` | 4.2 | `g3 = gv[g2].a;` | 3.0 |
  | `p = &gv[g2]; g3 = p->a + p->b;` | 5.65 | `gv[g2].a + gv[g2].b` | 5.4 |
  | `g3 = g5 + g6;` / `+ g7` / `g5 << 2` / `g5 * 4` | 2.0 / 2.75 / 1.75 / 1.75 | | |
  So a decompiler-shaped body is dearer than a hand-written one: named
  copies (+1.25 each), split `if`s (+1.25), if/else assignments instead of
  `?:` (+2.75), `!= 0` (+0.5), `x = x + 1` (+0.75), braces (+0.5 per arm)
  and pointer locals that are used once or twice. Each member level costs
  0.5, so a long `a.b.c.d` chain is expensive. Screened 2026-11-19: making
  every `GameDataStateStorage.fields_.X` a separate plain global, tree-wide,
  loses 25 fe-exact functions and gains none (15 lost with the lock masks
  kept in the union). The codegen changes too: at_climax's
  `test byte ptr [human_mask], al` becomes a load into a register. The
  struct spelling is image-supported; do not retry it as a budget lever.
  Decisive evidence (2026-11-20 par-6): setup_game 0x005B3A7A clears two
  arrays with ONE induction pointer (`mov eax, 0x9A6644; mov [eax-0x2C], esi;
  mov [eax], -9999; add eax, 4`), which VC6 can only form when both arrays
  are members of one object at a known distance; separate globals cannot
  give it. GameDataStateStorage stays a struct.
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
  (StatusWin::redraw) is never auto-inlined. Caveat (2026-11-19 par-2): in
  the image many copies sit in an end-of-unit cluster instead, after the
  unit's destructors and before its `??_E` adjustor thunks: datalink's
  ~ButtonGroup/~StringBox/~Flic copies (0x4325B0..) follow ~Datalink
  although ??0Datalink, the unit's first function, jumps to them from its
  unwind funclets; Popup::start's 5-argument copy 0x43EF70 follows
  ~SelectPartWin although select_special calls it. A copy at a unit's end
  is not evidence for a plain function defined there. The 5-argument start
  stays a header inline: made a plain member (defined in designwin.cpp,
  every other site spelled with the sixth argument), popp 0x0048C0A0,
  destroy, editor_diplomacy and seven more exact functions lose their
  teardown decisions, so its expansions are load-bearing spend.
- **Out of line at one site:** `(obj.*&C::f)()` still expands a header inline;
  a non-const local member pointer `void (C::*p)() = &C::f; (obj.*p)();`
  keeps the call (NetDaemon::reset in menu_proc).
- **Definition order and keywords.** Definition order is not a reliable
  lever: plain functions defined after their caller usually stay calls, but
  window_proc's static handlers and free functions defined after it were
  auto-inlined (C4711), and reordering compiled identically; always check
  C4711. An explicit `inline` *member* defined later still inlines.
  `__forceinline` and `inline` compile identically for `label_get` across
  all 5,574 markers; its named LPSTR local, not the keyword, is load-bearing.
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
- **Hand-stepped pointer against the plain nested index (2026-11-19).** A
  pointer stepped by hand (`*out++`, `out += W`) is a basic IV whose
  increment sits where the source puts it; VC6's own IV derived from a plain
  `dst[K + row*W + col] = src[f(row)*W + col]` is stepped at the end of the
  enclosing loop body, after the counters (`inc edi; inc ebx` before the
  back-edge). Image increments the column pointer last: write the linear
  subscript, not the pointer (vox_expand_palette 0x006391F0, 2 -> 0 rows).
- **Cached field against a late element pointer (2026-11-19).** A field read
  into an `int` local is one load into a register or slot; the image
  re-reading it at each use through a pointer register anchored at that field
  (`[ebx]`, neighbours at `[ebx-0xC]`, `[ebx+0xC]`) is `T *p = &A[i];` taken
  late in the body (after the calls that would otherwise kill it) with every
  use spelled `p->field`. It can also change /Ob2 spend: the int local had
  made the closing StringList teardown expand where the image calls it
  (DesignWin::select_special 0x0043AC70, 30 -> 5 rows).
- **One counter for sibling loops.** Several non-nested `for (int i ...)` loops
  were often one function-scope `int i;` (game_data 298 -> 240,
  world_rainfall 311 -> 271, eight more).
- `cmp reg, K; jl` is `i < K`; `i <= K - 1` gives `cmp reg, K-1; jle`
  (ProdPicker::calculate).
- **Counter subscripts `A[p][k++]` leave the inner product unfolded** (par-5
  2026-11-19, InfoWin::setup_text 0x00456300). A table filled row by row with
  a running counter (`strcpy(rows[pane][k++], ...)`) is lowered to
  `(pane*12 + k)*80` while `k` is still a variable; constant propagation then
  replaces `k` by 0, 1, 2.. per site, but `pane*12` survives as a common
  subexpression. Image signature: `lea r,[r+r*2]; shl r,2` held in a
  callee-saved register through the block, then the outer multiply per site
  (`lea t,[r+r*4]; shl t,4` for x80, with the folded `k*80` in the
  displacement), and a late `mov [k],N` INSIDE the last call's argument
  evaluation (the post-increment's store, 0x45696A). Literal row numbers
  (`rows[pane][3]`) fold the whole subscript to `pane*960 + K` and cache
  `this + pane*960` instead: other frame, a function-wide zero register the
  image lacks, `this` spilled. The fix was the counter at every row
  (2013 -> 1309 rows, frame 0x508 -> 0x504 = image); resets of the counter
  between sections (`k = 2;` after an optional row) are visible as immediate
  stores to its home. `analysis/findpat.py` finds the pair tree-wide.
- `recovery/tools/analysis/loopscan.py` lists the loop tests of both sides and
  the source loops with these shapes; `loopsweep.py --rule rebind|sibling|
  derive|basic` tries each rewrite site by site.
- **Open: NetDaemon::synch 0x00532E00, case 0x8D** (par-6 2026-11-19; do not
  repeat these). The image walks the social-category compaction with a
  staging pointer and reads `k` inside the inner loop; what keeps `k` read
  there is unknown. Tried in the window 0x533EE7-0x533F72, none reaching the
  image: indexed source and destination; a `RulesSocialCategory *stage`
  stepped in the `for` header, at the body end, or rebuilt as `&staging[k]`
  per iteration; `stage[0].` and `(*stage).` forms; destination as index,
  reference and pointer to `SocialCategories[k]`; an `int *` stage stepped by
  0x35 dwords; both store orders; `j++`/`++j`; `*(a + j)`, `[j + 0]` and
  `(&x)->` destination forms; `memcpy` of the preq dword and of the
  SocialEffect (about 80 variants in 4 decks). The stage pointer comes with a
  down-counter and frame 0x80 against the image's 0x7C.

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
  and none of faction's 278 across 16 shifts. base_energy 0x004EB560 does
  not move with pads behind the includes because the symbol it follows is a
  header global ("Bucket order of referenced symbols" below); give_a_base
  0x0054CF40 moves at no pad count in any place, so its flips come from the
  header content it uses (field types, arrays).
  The absolute count is not recoverable: each function only constrains the
  original's count modulo 64, to its residue set, and the whole header stack
  (vc6_compat.h alone is 1,334 symbols, windows.h 16,655 lean or 41,878 full)
  only matters modulo 64. So pads are alignment, not missing declarations.
- **Unit boundaries from the image (measured 2026-11-19, `analysis/tumap.py`).**
  Our files are already in image order, so "adjacent in .text and in the
  literal run" holds for every neighbouring pair and proves nothing. What
  does show a boundary:
  - a static-initializer thunk (`mov ecx, obj; call ctor; push thunk; call
    atexit`, listed in the CRT table 0x00682004-0x006826CC) sits at its
    object's definition point, so the first one of a run is a unit start and
    whatever lies between it and the next function is in that unit
    (sub_584d30 between the Wave thunk 0x00584D00 and tech_name);
  - a small callee the image calls where /Ob2 expands it once both share a
    unit: merging enemy into veh expands into enemy_capabilities,
    enemy_strategy and compute_odds (11,235 -> 13,792 rows); my_srand in
    netdaemon's unit is expanded into process_message (27 image call sites);
    danger beside add_site is expanded there. The same test rejected
    aggression+battle, base_build+base, netdaemon_message_data+orders,
    uv2player+tutwin, picktech+newtechwin, console_commands+terraforming;
  - a file whose functions lie in runs far apart is not one unit (alpha.cpp
    0x00584D60 and 0x0059D980; map.cpp's eleven runs).
  A merge that changes no function is not evidence for it. wants_to_attack
  flips with any unit change (header-position canary), so it is not either.
  Further measured cases (2026-11-19, par-1): bitmask 0x0050BA00 is expanded
  in battle.cpp (get_basic_offense, shoot, battle_compute) and at the head of
  aggression.cpp (sea_coast, port_to_coast, capture_base) where the image
  calls it, so bitmask + bit_count are a two-function unit between them;
  council.cpp merged into turn.cpp expands into control_turn (313 -> 543).
  A unit's static-initializer thunks are emitted in definition order, so
  `// symbol _$E<n>` numbers must rise with thunk address inside a unit
  (`analysis/esym.py` refreshes them after a move). An `extern T obj;`
  redeclaration AFTER the definition of a class object makes VC6 emit that
  object's dynamic initializer and atexit pair a second time (setup.cpp's
  g_CRASH_LANDING_WAVE: `_$E20/_$E18` and `_$E25/_$E23` for one object):
  never leave a local extern behind a definition. A thunk run between two
  units does not say which side it is on (last definition of the first or
  first of the second); only its contiguity with the next unit's own thunks,
  or a name, decides. Small functions in the row after a unit's last large
  function are mostly out-of-line COMDAT copies of header inlines kept from
  the first unit in link order that calls them out of line; their marker
  belongs in that unit (it must emit the copy, or fe reports `nosymbol`).
- **Data order is unit evidence too (measured 2026-11-19).** A unit's `.bss`
  (and `.data`) contribution is contiguous and the units follow link order,
  as in `.text`. Inside one unit the order is by symbol name (2.6), so only
  the unit's interval matters.
  - *Hard form, `analysis/initorder.py`:* a static-initializer thunk sits in
    its unit's `.text` and names an object of that unit's `.bss`. Taking the
    units in `.text` order, the intervals of the objects their thunks build
    never overlap: 288 thunks, 48 units, no exception once alpha,
    interface_fonts and newgame are one unit. That was the one violation:
    the thunk at the head of alpha's run (0x00584D00) builds
    `g_TOP_MENU_WAVE` 0x00945780, above `ConfigFactionPreview` 0x009403E0
    built by newgame's thunk 0x00589870, so both thunks, and all the code
    between them, are one unit (the wave is used by top_menu 0x0058E360
    alone). A thunk is a definition point, not a unit start: only the first
    thunk of a *unit* starts one. The merge changes no function, which the
    inline test could not have decided.
  - *Soft form, `analysis/datarun.py`, `analysis/datawho.py`:* ranked by
    the link position of the file that defines them, the bound objects of
    each data section must not fall. In `.data` the string literals that
    one unit's functions reference are anchors as well: 4,686 of them, 11
    out of order, so a unit's `.data` contribution is one run, its
    initialised globals and function literals in source order (globals
    defined above the first function come first; an object between two
    function literals was defined between those functions). The first
    units' `.bss` lies inside the raw data of the section (it starts at
    0x006A708C, after the last non-zero byte).
    `datawho.py` names the defining unit of every out-of-order object:
    `H` inside the interval of the objects that unit's initializer thunks
    build (a grouped declaration `Time a, b, c;` is one thunk with several
    `mov ecx, obj`), `D` both `.data` neighbours are that unit's, `S` both
    `.bss` neighbours are and the unit references the object, `N` the
    neighbours differ and exactly one unit between them references it.
    `analysis/objmove.py` moves the definitions (2026-11-19: 289 out of
    order -> about 85, mostly Caviar and undecided pairs).
  - *The game state is one unit's.* The unit we call xpops.cpp (X_pop,
    do_unity_crash, 0x005BEC40-0x005BFE8E) is the original file of global
    definitions: its `.data` head 0x00691870-0x00691BF7 is a list of
    `char *name = "text";` globals (version, date, `ALPHAX`, `HELPX`,
    `SCRIPT`, font faces, `SAV`/`SC`/`MP`) with `Terraforming[]` between
    them, directly in front of its first function literal, and its `.bss`
    run 0x00945F48-0x009B2277 (around its two thunk objects `g_PALETTE1`
    and `g_PALETTE2`) holds the rules tables, MapStorage, Players,
    PlayersData, Bases, Vehs, VehPrototypes and the save window. Other
    measured cases: the popup and interlude font pointers, `FastUnitAnim`
    and `TutWinClimateGate945830` are the alpha unit's; map_grid's
    altitude templates and `AltBonus` arrays lie in mapwin's `.data` and
    `.bss`; colortables' tint tables and vehicle_render's hover offsets
    lie inside interface_assets' thunk interval; fifteen turn-state words
    (0x0093A008-0x0093A98C) are turn.cpp's.
  - *Contiguity alone decides a sandwich (second pass, 2026-11-19).* An
    object whose bound neighbours on both sides are one unit's is that
    unit's definition whether or not the unit reads it (`S` asked for a
    reference and left these open): `DiploRedrawFlag` is fx.cpp's, the map
    and info panel rectangles console.cpp's, the pending-diplomacy words
    netdaemon.cpp's, `DesignGridButtonIndex` 0x009B7B10 a jackal/win.cpp
    global.
  - *Shared texts are COMDAT literals of header inlines (2026-11-19).* The
    image is unpooled, yet a few short texts are pushed from many units:
    `" "` 0x00682820 (43 units, Jackal's buffer.cpp, caviar.cpp and
    net_class.cpp among them), `", " ": " ")" "(" "%"` 0x00682E90, `">" "="
    "$LINK<"` 0x0068353C, `". "` 0x006867E0. Each group sits at the *end*
    of the `.data` contribution of the *first unit in link order that
    references it* (popup, basewin, datalink, multiwin), and the following
    unit often does not reference it at all (battlewin has its own `"%"`
    literals). That is where LINK keeps a COMDAT: from the first object
    that carries it, behind that object's own data. VC6 emits the string
    literals of an inline function as `??_C@` COMDATs even without /Gf
    (the tree's `help_create_link` reproduces the `$LINK<` trio that way),
    so these texts were literals inside header inlines, not globals; the
    named arrays (`BufferJoinSpace`, `SocialWin...Text`, `PercentText`) are
    stand-ins, now defined in the unit that keeps them. A library function
    that references a text in the game's `.data` is the tell.
    `analysis/sharedlits.py` lists them with the first referencing unit.
  - *`char *p = "text";` globals (`analysis/litptr.py`).* A pointer cell
    among a unit's initialised globals whose text lies in one block behind
    the last global, in cell order, and is referenced by nothing but its
    cell, was initialised with a literal. xpops.cpp's 19 cells
    (0x00691870-0x00691B3C -> 0x00691B40-0x00691BF7) all are; the named
    text arrays that stood in for them are gone (RECOVERY_LITERAL beside
    the pointer). One more in the image, Caviar's "Unsupported pixel size"
    slot 0x00698A9C, is left with that library's named messages.
  - *Function alignment phase.* VC6 gives every /Gy COMDAT 16-byte
    alignment, but in two Jackal units the function after one with an
    `__asm` block starts at that function's last byte and the unit's
    following functions keep the phase (sprite.cpp from 0x005E4ADA, the
    end of draw_mono, to its end; buffer.cpp 0x005E2DD3, the end of
    copy_mask, to 0x005E3563), while a COMDAT copy from a header is
    realigned (buffer.cpp's 0x005E35D0). Two
    functions at 0x005EB29F and 0x005EB32F with sprite.cpp's phase
    (SpriteList::on_insert/on_remove) are therefore sprite.cpp's own, not a
    run of spritebox.cpp.
  - *Jackal (2026-11-19).* `g_BUFFER` 0x009B6080 lies inside sprite.cpp's
    `.bss` run and its thunk 0x005E37B0 directly precedes Sprite::Sprite:
    sprite.cpp starts there (buffer.cpp's /Oi- is a no-op, no Jackal unit
    has /Oi, so flags do not separate the two). The blank cell sprite
    0x009BEAE8 with its compiler-generated cleanup guard 0x009BEAE0 is a
    class static of net_session_picker.cpp (thunk 0x0063BAE0 starts that
    unit). net_lists.cpp is one run 0x005D47D0-0x005D4CE7. Every Jackal
    unit now has one `.text` run apart from COMDAT copies in the image's
    leading inline area.
  - *Left undecided (adjacent units, neither names the object):*
    `MapEditorPaintId/BrushId` (map_grid or scale_adjustment), `Compass`
    (vehicle_cost or game), `TurnUpkeepPhase` (technology or xpops; moved
    to xpops with a note), `CursorHotspotX/Y` 0x0093AA10 (turn or
    netdaemon), `MsgStatus` 0x009B7B9C (win_input or timer_messages),
    `MapWinCaptionScale/Divisor` 0x00691E6C (world, game_calendar, map or
    gamma), `vox_projection_selected_angle_` 0x009BE6A0 (caviar_loader or
    caviar_scene). The popup theme colours 0x0068A5A0 (aggression's tail or
    console's head, neither reads them) were moved to console.cpp as its
    leading globals; a tail definition exists elsewhere (EditBox's defaults
    0x006971F0 follow editbox.cpp's literal), so that one is the likelier
    reading, not a proof. The `.rdata` float and const objects that read
    out of order (0x0066AA40, 0x0066EB38, 0x0066EF28, 0x00670A84,
    0x00670C00, 0x00670C50, 0x00670FE8) and Caviar's end-of-image `.bss`
    block 0x009C0828-0x009C0E64 are not unit evidence: pooled constants
    and a VC5 library whose objects do not follow unit order.
  Where an object is defined does not change code (every reference is
  absolute): 180 definitions moved without one function changing, although
  mapwin's symbol counter moved by 13, console's by 2 and popup's by -2.
  It is a structural record, not a matching lever.
- **The emitting unit of an inline copy (`analysis/ctxscan.py`).** For a
  marker in a short run between two other units, compile both neighbours:
  the one that emits the symbol and reads exact is the `compile_context`
  (range 0x00422F00 -> battlewin.cpp, NetDaemon::reset 0x0051DD00 ->
  console.cpp, veh_put/veh_health 0x005A59B0 -> probe.cpp). A dead function
  no unit calls (divide_and_increment 0x00532A50, point_in_inclusive_rect
  0x00592DB0, the two scale_adjustment functions) is no inline copy (an
  unused inline is not emitted): it is a real definition of the unit
  around it, and nothing in the image says which neighbour when it sits on
  a boundary.
- **Build list order.** `OPENSMACX_SOURCES` mirrors the image's unit order
  (each unit's main `.text` run); a unit without code keeps the position
  its data run gives it.
- **Pad sweeps do not fix general register ties (measured 2026-11-18).**
  Full 64-step `extern int` pad sweeps over about 33 units holding
  MNEMONIC_ONLY and register-cause functions flipped no non-exact function,
  and lost no claim. The counter governs only the listed canaries. Don't
  sweep pads to try to close an ordinary register tie; look for the spelling.
- **Header-position ties (measured 2026-11-19, par-5).** A second family
  of ties depends on how many declarations precede a particular *header*,
  not on the symbol counter at the function. Pads after the last `#include`
  (the CANARY-PADS position) never move them, at any count or kind; the same
  pads placed after `stdafx.h` (or before it) do, with a short period
  (set_cat_social: exact at 19-22 and 51-54 of 0-63 `void f();` pads).
  Moving the pad block down include by include names the header whose
  declarations matter (game.h for set_cat_social, caviar.h for
  vehdraw_construct_armor, faction.h for wants_to_attack, map.h for
  DesignWin::draw_armor). Fingerprint: a function that flips on several
  unrelated header commits (a new Buffer overload, a union member, an
  ungated inline elsewhere) and that no after-include pad count restores.
  Removing one unused inline overload restores it while renaming it,
  declaring it without a body, or adding members, blank lines or bodies
  does not; so the header edit is not wrong for it. Restore with a
  `// HEADER-PADS:` block right after `#include "stdafx.h"`, tagged
  `WORKAROUND(decl)`, swept over 0-63 against every function of the unit;
  re-tune after header batches like the counter canaries.
  Follow the block down the include chain and then line by line inside the
  header (a mirrored tree with the pads written into the header copy): the
  boundary lands on the `extern` of a global the function references
  (set_cat_social: `InterfaceBoxStyles` in buffer.h, via game.h, console.h,
  mapwin.h, graphicwin.h; draw_armor: the colour tables in game.h;
  wants_to_attack: the `Players`/`PlayersData` block in faction.h). Only the
  count matters: `void f();`, `extern int x;` and other names give the same
  windows, one extra declaration never flips it, and reordering same-kind
  externs among themselves (93 `CaviarData` externs by address, definition
  order or name) changes nothing. Moving that one `extern` is the natural
  fix when it lands somewhere sensible: `InterfaceBoxStyles` beside
  `g_IFACE_BOX_SPRITE_BUFFERS` restores set_cat_social with no pads and moves
  no other function tree-wide. When several referenced globals are involved
  (vehdraw_construct_armor) the windows are irregular and only pads work.
  The tree screen (2026-11-19, 84 units, pads 0-63 after stdafx.h against
  after the last include) found these ties only in already-exact functions
  that sit on one (the list to re-check after header batches is in the
  commit message); none of the non-exact functions under 60 rows flips.
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
- **An uncalled static function still emits its literals** (measured
  2026-11-19, par-1). VC6 drops the code of a `static` function nothing
  calls, but its string literals stay in the unit's `.data` as `$SG`
  objects, in source order, each taking a counter slot (a probe.cpp control:
  `static void f() { g("X"); }` with no caller leaves `X` in the object and
  no `f` symbol). A dead helper therefore still adds literals to the unit's
  literal run (seen by `analysis/litgap.py` as strings the image lacks) and
  still moves the counter for every later function, besides its own
  declaration's slot. Leftover non-original helpers that nothing calls are
  deleted, not kept as harmless; dead non-static helpers additionally emit
  their code.
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
- **Bucket order of referenced symbols: one counter, read at each symbol's
  own declaration (par-5 2026-11-19, `analysis/declpad.py`).** The
  "header-position ties" above, the round-6 "second counter with period 112"
  (base_energy 0x004EB560) and the mod-64 canaries are one mechanism. What
  the tie reads is the front-end symbol NUMBER (the `$SG` counter) of each
  file-scope symbol the function references, modulo 64: a header global has
  the number the counter had at its `extern`, a string literal the number it
  had at the function. So a pad moves a tie only when it sits before the
  declaration of a symbol of that tie, and "behind the includes" is simply
  after every header global.
  - *Units (read off the first `$SG` of a unit, one declaration added
    before its includes).* 1: `void f();`, `extern int x;`, `int x;`,
    `static const int k = 3;`, `typedef int T;`, a forward `struct S;`.
    Each parameter of a prototype is 1 more, named or not (`int f(int,
    char *);` is 3), each enumerator 1 (`enum E { a };` 2, three
    enumerators 4). `struct S {};` is 8, each data member or member
    prototype 1 (`struct S { int a; };` 9), an in-class body 2 more than the
    prototype and 1 per parameter and per local (`void f() {}` 3,
    `void f(int) {}` 4, with one local 5); a free `inline int f(int a)
    { return a + 1; }` is 3; a class with a virtual destructor and one
    member 17; a `#define` 0. So ADDING A LOCAL TO A HEADER INLINE BODY, or a
    parameter to a prototype, renumbers every later symbol of every
    includer. The round-6 table (struct 16, parameterless member 5,
    one-parameter member 7, period 112) is this counter in units of 4/7
    symbol: 112 of them are 64 symbols. Confirmed on base_energy with pads
    of every value 0..111 built from those three kinds (one contiguous exact
    window, 41..101) and with prototypes (34 of 64); "6 prototypes or
    externs never flip it" was only the width of the window.
  - *Reading a number.* `char *p = "zq";` at any place takes two numbers,
    the pointer and then its literal, so the `$SG<n>` of that literal in the
    object is the counter there plus one (`declpad.py syms <unit> <addr>`
    lists the number of every global a function references; a unit's first
    header symbol is about 1,519, its literals start near 60,000).
  - *Which symbols.* `declpad.py at <unit> <addr>` sweeps 0..63 prototypes
    before and after the declaration of each referenced global in a shadow
    copy of src/ and bisects on the pattern: the pattern changes exactly
    across the declarations the tie follows and is flat after the last one.
    base_energy follows one symbol, `GameDataStateStorage` (exact while its
    number mod 64 is 0..33; pads right after its `extern` do nothing, so the
    round-6 "everything before faction.h's externs" was this one line).
    DesignWin::draw_armor 0x00435400 follows two, `BufferJoinSpace` (b) and
    `ArmorTransparentColorTables` (a); the full map of its one swapped pair
    of loads (`mov edx,[ebp-0x10]; mov eax,[ebp-8]` before an `add`) is
    piecewise constant with FIXED thresholds: for a in 6..41 wrong iff b in
    6..41; for a < 6 wrong iff a < b <= 41; for a in 42..45 wrong iff
    b >= 42; for a >= 46 wrong iff b in 42..45. The thresholds (6, 42, 46;
    34 for base_energy) move with no pad anywhere, with no added local and
    with no added literal, and bucket 0 is an edge: the table is walked from
    bucket 0 and something of the function itself holds fixed buckets. The
    two loads that swap are locals; the symbols that decide are globals the
    function merely passes as arguments.
  - *Per-function table (`declpad.py who`, wip 6a088e1769; residue = symbol
    number mod 64 in that unit, with the unit's pads in place).*

    | function | symbols the tie follows | allowed |
    |---|---|---|
    | base_energy 0x004EB560 | GameDataStateStorage (12) | 0..33 |
    | wants_to_attack 0x0055BC80 | PlayersData (48) | 33..63, 0 |
    | world_volcano 0x005C6DB0 | StringTable (12) | 7..63 |
    | planet_busting 0x00500B60 | ConsoleGlobal (25) | all but 11..15 and 59 |
    | Datalink::set_cat_social 0x0042B630 | InterfaceBoxStyles (36), SocialCategories (0) | top shift 0..27, 32..62 |
    | DesignWin::draw_armor 0x00435400 | BufferJoinSpace (60), ArmorTransparentColorTables (11) | map above |
    | vehdraw_construct_armor 0x004C3090 | eleven `g_*_CAVIARDATA` externs of caviar.h (47..62, 7..26) | top shift 15, 27..34, 43..44, 47..52, 56..3 |
    | faction_upkeep 0x00527290 | Bases (53) + its literals | - |
    | end_of_game 0x00524870 | Players (27), NetDaemonNet (57) + literals | - |
    | corner_the_market 0x004E2000 | ConsoleGlobal (61), NetMsgPrimary (63) + literals | - |
    | action_airdrop 0x004CC360 | NetMsgPrimary (51) + literals | - |
    | action_give 0x004CEDE0 | ConsoleGlobal (25), NetMsgPrimary (51) + literals | - |
    | PrefWin::init 0x0048D840 | PrefWinCaption (unit global) + literals | - |
    | on_iface_button_clicked 0x004A6330 | ReportIfGlobal (30), Bases (15), MainInterfaceGlobal (10) + literals | - |
    | load_box_sprites 0x00453A70 (6 rows) | eleven sprite and buffer globals of sprite.h / buffer.h + literals | never exact on any cut |
    | design_workshop 0x00583E30 (13 rows) | TutWinDesWindow (14) | never exact |

    No global is shared by two of these functions inside one unit except
    NetMsgPrimary and ConsoleGlobal in terraforming_actions.cpp (action_airdrop
    and action_give agree on a region: both exact with the unit's two pad
    blocks emptied and 26, 27, 36, 38..41 or 46 prototypes at the top), so
    the "several functions must agree on one offset" test has nothing else to
    bite on; across units the same header global has a different number in
    each unit (ConsoleGlobal 25, 61, 5), as it had in the original.
  - *What a top-of-unit sweep proves.* n prototypes before the first line
    shift every symbol of the unit together, so every pair of symbols with
    different residues changes its bucket order for part of n = 0..63. A
    function that does not change at any n has no tie of this kind, whatever
    its residue looks like. Tree pass (`declpad.py top`, 175 units, 5,473
    markers, 64 compiles each): 55 functions move at all, 19 of them claimed
    (the canaries of 3.6) and most of the rest large (enemy_move, probe,
    battle_fight, order_veh: a few rows of several thousand). Of the 190
    non-exact functions of at most 60 rows, 9 move and none becomes exact:
    upgrade_prototype 27 -> 24, sat_attack 17 -> 15, load_interface 18 -> 17,
    vox_matrix_matrix_mul 54 -> 22 (VC5 library); load_box_sprites 6,
    design_workshop 13, veh_pick 27, draw_sat 54 and sub_6401b0 33 change
    which rows differ without improving. The one- to three-row residue
    (world_site, SpriteBox::draw_item, Popup::on_draw_background,
    DesignWin::on_iface_right_click, load_voxels, load_palette,
    vehdraw_compute_facet_normals, compute_odds, Win::update_screen,
    FileWin::init, Credits::exec, Console::zoom, tech_achieved) and
    give_a_base are flat at every n. Do not sweep pads at those.
  - *The image constrains a set, not a number.* Each function allows a
    region of its symbols' residues (half of the 64 values for base_energy),
    and the symbols themselves are often our stand-ins: `BufferJoinSpace` is
    a named object where the original had a literal numbered at the
    function, `GameDataStateStorage` one struct where the original had
    dozens of separate globals. So the offsets do not say what a header has
    too much of. One shift for every unit (a count missing from the shared
    prefix, stdafx.h) was tested with every pad block emptied
    (`declpad.py padfree`, `common`): no n keeps all sixteen canaries; n = 49
    keeps all but action_airdrop (and gives buy_council_vote and action_give),
    n = 7 and n = 57 lose one each. Three such n of 64 is what independent
    windows of this width give by chance, and the non-exact functions do not
    prefer 49 (load_interface 18 -> 31, Console::upgrade 147 -> 167 against
    probe 1292 -> 1251). The pad blocks stay per unit.
  - *Placing pads.* A tie on header globals needs the block before the
    includes (HEADER-PADS); one on literals or unit statics after them
    (CANARY-PADS); a function that needs both can have two blocks. Use
    prototypes: struct pads work only through their symbol count (9 each).
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
- **A byte test of `lock_masks_.human_mask_` against `1 << f`** is `is_human(f)`
  (human_mask_ is byte 0 of the two status bytes at 0x9A64E8).
- **`mov r, dword ptr [g]; and r, 0xff` is a uint8_t load, not a dword
  field.** VC6 widens a zero-extended byte read (struct field or plain
  global) to a dword load plus a mask, and serves an adjacent byte of the
  same object from `ah`/`ch` of that load; union, struct-of-bytes, array and
  pointer-cast spellings compile identically (2026-11-20 par-5, rankings
  test units). Judge a field's width only by reads that use bits it would
  not have. The lock masks are two bytes for this reason, and they alias
  GameDataStateStorage stores: rankings reloads `alive` per iteration of
  the faction_rankings_ store loop (0x5AC94B), which a separate object or a
  separate rankings array would hoist.
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
- **Arrays of objects: the vector constructor iterator is an ordinary /Ob2
  inline** (par-3 2026-11-19, `analysis/probes/vecctor/`, high on probes,
  medium on the one real case). For `T a[N]` (local, member, `new T[N]`)
  whose element has a constructor and *no destructor*, VC6 generates the
  `??_H` (`vector constructor iterator`) COMDAT in every unit and treats it
  like an auto-inline callee: expanded, it is a `dec/jne` loop that calls (or
  expands) the element constructor; refused, it is `push ctor; push N; push
  size; push ptr; call ??_H` (stdcall, no `add esp`). Element count (1..16),
  ctor inline/out of line/compiler-generated/extern, a default argument, a
  virtual table, a local vs member vs `new` array and an EH frame elsewhere
  never refuse it (`run.py`, `more.py`, `eh.py`); only the caller's budget
  does (`budget.py`: 16 two-if inline sites before the array refuse it, none
  after it does, a bigger caller restores it). With a destructor and /GX the
  EH iterators `??_L`/`??_M` are always called, never expanded. An array
  member makes the iterator a depth-2 site inside the class's generated
  constructor. The image's only out-of-line `??_H` call (NetWin::exec
  0x4824D5) is such a depth-2 site, and 96 if-units of added pass-1 charge
  did not refuse it in ours, so it is not that caller's budget: the image
  expands nothing below depth 1 in exec, and `#pragma inline_depth(1)`
  around exec (WORKAROUND(inline)) reproduces its call multiset exactly
  (266 -> 233 rows). Tree-wide, the call multisets of `??_H/??_L/??_M` agree
  everywhere else (calldiff, 2026-11-19).

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
4e. **No allocation cursor for callee-saved registers** (high, 2026-11-19
   par-1; probes `cur_*`, `after_*`, `two_before`, `ovl_a` beside g_split's
   `disjoint3`). Each candidate takes the first of esi, edi, ebx that no
   interfering, already served candidate holds; an earlier web that does not
   interfere (disjoint, at any priority, before or after) shifts nothing, and
   disjoint webs reuse esi. The scratch cursor of 3.2 S1 has no callee-saved
   counterpart. The FlatButton vtable census that suggested one (90
   functions with both stage constants 0x669754/0x66974c in registers:
   esi/edi 62, edi/ebx 16, ebx/esi 3, edi/esi 3, ebx/edi 1) is first-free
   plus 4f: the non-cyclic cases are a longer web of one constant that some
   esi/edi holder interrupts while the other constant splits into short
   pieces (DesignWin::on_button_clicked, draw_terraform). Our compiles give
   the image's pair on 85 of the 90.
4f. **Two constants that tie go to the LATER first reference** (high,
   probes `hi_first`/`lo_first`, `small_first`/`big_first`, `afl`, `blkAB`,
   `three`). The opposite of 4c's in-block order for variables, and
   independent of the values, of the last reference and of the order the
   constants first appear in the unit. Three tied constants: the last to
   appear takes esi, the first ebx. In the ~FlatButton stage (`[obj] =
   0x669754; [obj+K] = 0x66974c`) 0x66974c therefore wins. `candset.py`
   uses it (exact functions with both constants: rank constraints its own
   scores explain 275 -> 331 of 402; a random 120 exact functions 709 ->
   717 of 869). Open: base_editor 0x0041C7A0's last 10 rows are this pair
   ordered the other way in the image with identical code around it (candset
   flags the image's `#<r>` as NOT EXPLAINED); `return` -> `break`, the
   switch under `if (action >= 0)` and declaration order did not flip it.
4g. **A constant reference weighs half a variable reference** (medium; probe
   grid `t_cNN_kMM`: p and q hold esi/edi, a variable with c references
   against one constant with k stores for ebx, one block). The constant
   wins from k = 2c + 1 (c = 2..5: k = 5, 7, 9, 11); at k = 2c the variable
   keeps it. Constant pushes (`f(100000)`) never formed a candidate. In
   candset, `--opt constw=50` lowers the explained count (331 -> 314), so the
   instruction-level count is not simply halved; keep constw=100 there.
4h. **`__asm` blocks do not touch the ranking; they only remove ebp** (high,
   2026-11-20 par-5, probes `pr1`-`pr3`: member stores through `this` (T)
   against an int parameter `p` with 8 references, a/b on esi/edi). The
   crossover is T = 8 (tie to `this`) with no asm, an `__asm { nop }` at the
   top, one inside an `if` arm, `p` assigned before or after it in the arm,
   and `p` read or written by name inside it. The only effect is the ebp
   frame: the loser goes to ebp without asm and to memory with it (this
   spilled: `mov [ebp-4],ecx`, reloaded through scratch registers).
4i. **Two whole-function webs can swap with the register count** (high on
   the real function, 2026-11-20 par-5, Buffer::line 0x005E09D0). Same C
   (asm swap written as C): with ebp free, `y1` takes ebx (31 of the image's
   34 ebx reads aligned) and `this` takes ebp; with ebp taken by a frame
   (`__asm { nop }` at the top, or an `_alloca` in a dead arm) `this` takes
   ebx for the whole function and `y1` goes to memory. With served-in-order
   greedy that is impossible (this would take ebx at k = 4 too), so for long
   webs the serving order depends on how many registers there are (as in a
   simplify/spill colouring); rule 4 still decides the short probes.
   Consequences: (a) a "regprio prio 1.5" whole-function web such as
   `y1` is not dead weight: the 4b penalty over-charges webs that span
   everything; read it as "competes for the last register", not "loses".
   (b) The image's Buffer::line (k = 3: y1 ebx, `this` spilled to [ebp-4],
   `along` packed into y1's dead home [ebp+0xC]) is the outcome our source
   gives only at k = 4, so the original weighs `this` less, or `y1` more,
   than ours at k = 3. Measured single-reference probes on our source: the
   loop `stride_` reads moved off `this` spill `this` exactly like the image,
   but ebx then goes to `x1` pieces; dropping any one non-loop `this` use
   (the closing free_data, the rect1_ copy, acquire_pixels, a pixel base)
   also loses ebx for `this`, to `x1`/`y2`. So `this` sits on the threshold
   and `y1` is below `x1`/`y2` there; the missing weight is on `y1`'s side.
   Whole-function member counts are equal (0x4a8 11/11, +0x50 10/10, +0x6c
   11/11), so look for y1 reads the original had (clip tests, the pixel
   bases) rather than for `this` differences.
   **Superseded by 4j** (2026-11-20 par-5): the swap is not a k-dependent
   serving order. At k = 4 `this` leaves ebx to the byte webs
   (`first_color`/`second_color`, stored through `al`) and takes ebp, and
   the next web (`y1`, or `x1` in a one-octant cut) inherits ebx. So the
   k = 4 compile says nothing about `y1` against `this`; at k = 3 the plain
   order holds, and the image (y1 ebx, this spilled) still needs `y1` above
   `this` where ours has `this` above `y1`.
4j. **Under /Oy a web leaves ebx to a lower byte web and takes ebp** (high,
   2026-11-20 par-5; `regalloc/asmprobe/gen_byte.py`, Buffer::line cut to
   one octant). Only ebx has byte sub-registers, so a web whose value is
   used as a byte (`gch[i] = (char)x`, `*pixel = (unsigned char)c`) can only
   take ebx. When a non-byte web is served and its first free register is
   ebx, ebp is free over its range, and an interfering byte web ranks below
   it, it takes ebp instead. The look-ahead is one step: the next non-byte
   web can still take the ebx it left (probe `t10_z5`: `this` ebp, `z` ebx,
   the byte `x` in memory; `t16`: `b` ebp below `this`/`a` on esi/edi).
   With ebp unavailable (/Oy-, any ebp frame: `__asm`, `_alloca`, EH) the
   rule is silent, which is all of 4i. Census of the fe-exact functions
   (`candset.py --census`, 5,164 functions, 13,049 rank constraints):
   9,968 explained without 4j, 10,005 with it; 41 ebp webs explained, 4
   ebx webs against it (two of them in hand-written `uv/` decoders). The
   byte demand of a loser is read from the code (a dword stack load whose
   low byte is stored or compared); byte reads of stack slots
   (`bytepref=2`) also catch local char arrays and explain less (9,997).
   Reading it: an image web in ebp that its score would put in ebx, in an
   /Oy function that stores a value as a byte nearby, is 4j and not a
   priority gap; the lever is the byte web (its type, its cast, whether it
   is a candidate at all), not the ebp web.
   Refuted on the way, with probes in this round's scratch (`g1`-`g4`):
   interference degree does not move a single-block threshold (6 short webs
   interfering with one candidate leave its crossover at 8 of 8 references,
   so no Chaitin cost/degree spill choice); an unconstrained high-priority
   web (degree 1 at k = 3) is still served first and takes esi (no
   Chow-Hennessy deferral of unconstrained ranges); a 256-byte frame that
   pushes the parameters past disp8 does not move `this` against a
   parameter. Served-in-order greedy plus 4j stands.
   4j from source (2026-11-20 par-5, Buffer::write_vertical 0x005DB450,
   a3295b4331, 51 -> 44): when the image's ebp web (and its `push ebp; push
   esi` save class) needs a byte web that ours lacks, the byte web is often
   a char LOCAL the original read the value into (`char c = *p; ... buf[0] =
   c`), not a cast: a direct `buf[0] = *p++` copy forms no byte web, so `this`
   keeps ebx.

Using it (regprio.py uses 4b, 4c and 4j by default; `--model old` is rule 4 only, `--no-bytepref` drops 4j;
candset.py `--opt bytepref=0` drops 4j there, `candset.py --census [units] --opts A --opts B` compares option
sets over the fe-exact functions):

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
- Check the class first: `regalloc/tiescan.py <feall.json>` lists the
  functions whose whole diff is one renaming of esi/edi/ebx/ebp. On
  2026-11-19 that was 5 of 387 non-exact functions (popp 0x0048C0A0,
  say_orders 0x004B43E0, compute_score 0x005ACBE0, Win::flip 0x005EFD20,
  Palette::fade_to 0x005FF930 with two more rows). Of the "register tie" list
  of the 2026-11-18 screen, Menu::on_key_click, base_editor_fac, DiploPop::init
  and double_cross differ mostly in eax/ecx/edx (scratch choice and
  scheduling, not this model); Dialog::init, draw_garrison and compute_score
  are mostly callee-saved; fade_to's swapped rows are two frame slots of
  strength-reduced array differences, with the registers equal.
  Model-guided tries that moved nothing: fade_to `weight` hoisted next to
  `frame` (4c; worse, 4 -> 14: the image's deferred `push ebp` puts `weight`
  inside the `if`), term and declaration order; Dialog::init 36 orders of the
  geometry locals and member stores; compute_score's tail (`*total`
  spellings, named sum, pointer loop).
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
- **A copy splits a web; a reassignment keeps one.** `const int mask = f & 0x10;
  … uint32_t sel = mask; if (n > 1) { …; int r = popup.exec(); if (r < 0)
  return; sel = r; } if (sel & 0x10) …` makes `mask` and `sel` two webs; `sel`
  spans the popup and lost its register to a frame slot (an extra 4-byte
  local, frame 0x538C against 0x5388). The image has one variable: `int
  ability = f & 0x10; … ability = popup.exec(); if (ability < 0) return;`
  (`and ebx,0x10` … `mov ebx,eax; test ebx,ebx`): the mask and the result reach
  the same `& 0x10` test, so they are one web in ebx, and the freed slot is
  shared by entry_x and the teardown temporaries (Console::use_ability
  0x004D7F60, 208 -> 27 after re-tuning its budget probes). candset.py shows
  it as an ours-only `wb[-0x10]{selected_ability}` role (the copy, written
  back to a slot) and an image-only `dwordptr[ecx+<r>` ebx role marked
  "source variable home" (the mask load itself owning the register to the
  final test): one source name too many.
- **A byte member at +1 and a dword read at +0 of the same union are one
  web** (par-5 2026-11-19): when a union overlays a dword with bytes, VC6's
  web builder joins a read of the byte member at offset +1 and a read of the
  dword at +0 of the same object into one allocation web, so they compete
  for (and share) one register home instead of two. Count them as one
  candidate when reading register priorities.
- **A global cached across a loop** (2026-11-19 par-5; probes in
  `analysis/regalloc` style, game flags; base_support, CouncWin::on_left_click).
  Rule 10's "globals across calls are not candidates" is too strong: VC6 forms
  one web for a global's loads over the whole function and gives it a register
  like any candidate. Measured:
  - Kills are calls only. Stores to other globals do not kill it (a direct
    store, `gcnt[i]++` into an int or unsigned array, a store into a global
    struct array element): the load is hoisted out of a call-free loop.
  - After each kill VC6 inserts a reload so the value is available again:
    in the call's own block, at the merge of the killed paths (it splits a
    `continue` target into a reload block and the plain latch), after an
    inner loop that calls, or on the loop's back edge (a block at the loop top
    that the preheader skips with a `jmp`: base_support 0x004E9609).
  - Formation is a cost trade: weighted uses against reload sites. With 3 uses
    (entry, one in an earlier loop, two in the loop) one or two reload sites
    keep the web (callee-saved register when it spans a call, a SCRATCH
    register such as edx when every call is followed by a reload); three
    conditional calls (three sites) drop it, unless one more use follows the
    calls; five sites drop it even then. Without a use inside an earlier loop
    (the entry read alone) no web forms at all.
  - Where the reloads go is a per-function decision we do not predict:
    base_support's Veh loop with `if (faction != f) continue;` reloads after
    each kill (too many sites: the web is dropped, the loop's use loads into
    eax); with the body nested under `if (faction == f) { ... }` VC6 takes the
    back-edge site and the image's ebx web appears (29 -> 11 rows). In synthetic
    loops `continue` and the nested form compile alike, so try both spellings
    of an early `continue` when the image shows a loop-top reload block.
  - A web that wins a register can push a local out of it: CouncWin::on_left_click
    keeps ConsoleGlobal.faction_id_ in esi through both popup arms (reloads after
    X_pop and exec) because the single-player buy_council_vote reads it with no
    call between; the image has the vote (`y`) in esi across the join and loads
    the global at that call. A separate `vote` local (`vote = y` on the
    is_alive path; VC6 parks it in y's dead home, as the image's [ebp+0xc]) does
    not change the winner (30 -> 33): the deciding reference is still unknown.
- **Hoisted loop-invariant globals are candidates.** In can_terraform's
  radius census the image hoists MapLongitudeBounds (four references per
  iteration) into edi and rereads MapLongitude; the statement form of the
  private wrap helper (a static function /Ob2 expands) hoisted MapLongitude
  instead. The flat-first ternary body gives the image's map (336 -> 325).
  The same body change in course_xrange/territory_xrange moves nothing: the
  form matters where the wrap's arms reach the caller's variable or invariant.
- **Call-free loop counters, hoisted globals and the scratch registers**
  (2026-11-20 par-2; `regalloc/gen_loopctr.py`, `analysis/regalloc` probes
  `zc`/`ec`/`tr` in the round's scratch, about 230 probes, /Oy-).
  - G1 (high). A loop counter (strength-reduced to a byte offset, `add r,4;
    cmp r,0x20`) of a call-free loop takes a SCRATCH register with its own
    `xor eax,eax` when one scratch register is free over the whole loop,
    even with esi/edi/ebx unused (`z4call_ilocal`: edi never saved). It then
    never joins a live zero web. A value live through the loop without a
    call (a mask such as tour's `elevated`) also takes a scratch register
    there (`ecx`, `ec` family). Bisection of tour's dir-loop body over six
    parts (wrap, bounds test against globals, tile load `MT[(x>>1)+y*ML]`,
    compare with `tile->climate`, `!dir || dir >= 6`, `index` tests): the
    counter stays in eax/ecx/edx in every body without the tile load, and
    with the tile load alone; tile load + wrap or tile load + bounds test
    (24 of 64 bodies) leave no scratch register free at some point and the
    counter goes to esi/edi. The other three parts never matter.
  - G2 (high). A counter that goes to a callee-saved register takes the
    register a zero web held up to the loop entry, with no `xor` of its own:
    the zero's last stores (`higher = 0; lower = 0;`) and the counter are one
    candset `init#0` web (Z4). So "the image's counter in eax with its own
    `xor`, ours in edi after `mov [m],edi` zero stores" is a G1 question
    (what keeps a scratch register busy over our loop), not a zero question.
  - G3 (medium). A loop-invariant global of a call-free loop is hoisted into
    whatever register is still free over the loop, scratch ones included:
    MapLongitude (one `imul` per iteration) into eax or esi, MapLongitudeBounds
    into eax/edi/ebx, `MapIsFlat & 1` into edi; with none free it is read at
    each use (`mov ecx,[B]` once in the wrap arm, `cmp edx,[B]`). A hoisted
    global therefore competes with the counter for the last scratch register:
    in the full tour-shaped body MapLongitude takes eax and the counter esi.
  - Census (candset.py, 5059 fe-exact non-asm functions, 2026-11-20): with
    call-crossing webs ranked above call-free ones (G1's consequence) and
    call-free pairs ranked by raw references, then range length, the rank
    model explains 9578 of 11402 constraints (9313 before); 3733 of 4287
    call-free callee-saved webs have every scratch register busy over their
    range (G1/G3 hold), and 29 of 45 split zero webs have full pressure
    between the pieces (Z10). `ctrscan.py` pairs loops and flags counter
    class / hoist differences (no BYTE_EXACT function flags).
  - A loop-invariant global that is NOT hoisted can be aliasing, not
    pressure: rankings 0x005AC690 rereads the alive mask (`dword` + `ah`)
    in the faction_rankings_ store loop because both are fields of
    GameDataStateStorage; the image hoists `mov dl, byte ptr [mask]` before
    the loop. A probe with the store into a separate extern array hoists
    the mask and gives the image's esi counter / edx `humans` (22 -> 17):
    in the original the rankings table and the lock masks are separate
    objects (header question, not taken).
  - Console::editor_natural 0x004E14D0: the probe loop's wrap through
    xrange_inline hoists MapLongitudeBounds into ebx as the image does
    (wrap_map_x's ternary put it in scratch ecx; 116 -> 105).
  - TutWin::tour 0x004BC6E0 (open). Image dir loop: counter eax (own `xor`),
    MapTiles in edi (reloaded after crop_yield for `tile` and kept through the
    loop), `elevated` in ebx, esi for pieces (x, y_adj, tile, index);
    MapLongitude/MapLongitudeBounds read per use. Ours: counter + zero web in
    edi, MapLongitudeBounds in ebx, `elevated` in eax. A probe copy of the
    outer body (`tr` family, 36 variants) never lets the counter leave the
    callee-saved register: `elevated` read after a call (it then takes ebx as
    in the image, but a hoisted global takes eax), its later uses removed,
    the bounds/MapLongitude references removed, nested ifs instead of
    `continue`, fungus/step/higher/lower spellings, `while`. In tour itself
    xrange_inline, the statement wrap, nested bounds (299), `dir != 8` and
    the tile-compare spellings move nothing (303). The image's body holds one
    scratch-holding value fewer than ours; which one is still unknown.
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
- **A losing member-address candidate reserves nothing** (high, 2026-11-20
  par-5, `regalloc/asmprobe/gen_addr.py`: `dd.m()` receiver calls on a
  member at +8192 against 0-4 int candidates of 12 references, /Oy and
  /Oy-). With registers to spare the address takes one (`lea edi,[esi+8192]`,
  `mov ecx,edi` per call, from N = 2 calls). When it loses it is
  rematerialised at every site (`lea ecx,[ebx+8192]`, or `[esi+K]` folded
  into a load) and the other candidates still take esi, edi, ebx in order:
  no register is skipped. From N = 4 calls with `this` otherwise used only
  for a few small offsets, VC6 rebases `this` itself (`add ebx,8192`, the
  other members at negative displacements) instead of keeping two webs.
  So an image register that is never used (~PullDown's edi) is not a dropped
  address CSE; look for a winning constant (Z2 below) instead. BasePop::exec
  0x00602600: ours forms `&dialogs_` (ebp, 8 vbptr loads `[ebp]`) and a zero
  (ebx, 12 `cmp [m],ebx`); the image has neither, reads the vbptr as
  `[esi+0x21d0]` and leaves ebx/ebp unused until 0x602C74. With both
  registers free there, a formed address would have won one, so the
  original did not form these candidates at all: a source-reference
  question (fewer `dialogs_` receiver/vbptr uses in the first block, fewer
  zero compares), not a reservation.
- A loser constant still forms short pieces: two adjacent zero stores in one
  block always become `xor eax,eax` + two register stores (both flag sets). The
  image's immediates mean the zero *won* a callee-saved register (and was then
  not materialised). Credits::unload 0x00428830 and Credits::exec 0x004289F0:
  under /Oy the zero wins ebp and the stores are immediates (a
  `#pragma optimize("y", on)` copy is exact); under the unit's /Oy- it loses.
  Exec's frame proves /Oy- for exec, so its source must give the zero more
  weight or one rival less; not found.

Constant pieces and immediates (2026-11-19 par-5; `regalloc/c_zero.spec`,
`python3 cand.py c_zero.spec --grep 'xor|PTR .*, (0|e..)$|^cmp'`; about 400
probes, /Oy- unless stated, the same for 0, 5 and -1)

The four-store rule above decides when a constant *pays* for a register. Below
four, what the stores look like depends on whether the constant is a candidate
of its own and whether it found a free callee-saved register:

- Z1 (high). A constant with no other reference in the function: two to six
  adjacent stores in one block become a scratch piece, `xor eax,eax` (or `mov
  eax,K`) and register stores, whatever the register pressure. Stores in
  separate blocks or separated by calls stay immediates up to three; from four
  they take a callee-saved register when one is free (`spread`, `ifs`,
  `pairs` in the family).
- Z2 (high). Any other reference to the same constant anywhere in the
  function makes it a candidate: a `push 0`, a memory compare `cmp [m],0`, a
  store in another block. A `test reg,reg` of a call result or of a loaded
  value does not (`pair_memcmp` against `pair_callcmp`). Such a candidate
  that finds a callee-saved register free over its whole range, but has fewer
  than four stores, takes that register without materialising it: every store
  and push is an immediate and the register is not even saved (`pair_push/0..2`).
  This is the image's "two immediate stores where we have `xor eax,eax`".
- Z3 (high). The same candidate with no free register over its range (three
  rivals live across it, `pair_push/3`) loses and forms the scratch piece of
  Z1 again. Under /Oy ebp counts as the free register: Credits::unload is
  exactly Z2 under /Oy and Z3 under /Oy-.
- Z4 (high). If the register it wins is one another web also uses (a variable
  born after the constant's last use, or dead before its first), it is
  materialised there: `xor esi,esi`, `push esi`, `mov [m],esi`
  (`pair_born`; s7 `dead_pa`). So "immediates in the image, `xor esi`
  + register stores in ours" means our constant shares a register the image's
  does not, and the reverse.
- Z5 (high). Typed constants are separate webs: a float zero is always an
  immediate; a char or bool zero is its own byte web (`gz0 = 0; gc0 = 0;`
  stays two immediates even at full pressure, `pair_int_char`); short goes
  with int (`mov [m],ax` in the piece); pointer, unsigned, enum, long and
  `NULL` are the same web as int 0. Two adjacent char zero stores alone form
  an `al` piece.
- Z6 (medium). Compares. A true constant register gives `cmp r,zreg` for an
  enregistered local and `cmp [m],zreg` for memory, but a call result in eax
  keeps `test eax,eax`. A zero-initialised *variable* in a callee-saved
  register replaces the 0 of pushes and stores while it is known zero, but its
  compares stay `test r,r`, until the constant 0 is itself a candidate merged
  with the variable. In the `slot2_*` probes one live memory store of 0 in the
  variable's known-zero stretch (before the compares or in one arm) does it;
  a dead store, a store to a parameter, and the same store in the lighter
  `slot_*` shape (one loop memory compare fewer) do not. This is Console::zoom
  0x005150D0 (`cmp eax,ebx` / `cmp esi,ebx` against our `test`): the image's
  zero is merged with `slot`; no source store that would do it was found
  (an explicit `window_index = 0` in the world-map arm, `MapWinTable[slot]`
  for `[0]`, `window_index = slot` per arm: 3 -> 3..13 rows).

Applied (2026-11-19):
- Credits::exec 0x004289F0: under Z2/Z3 the image's two immediates need a
  callee-saved register free over the zero's range, and the range covers the
  inlined free loop where esi (`this`), edi (pointer) and ebx (counter) are all
  live, as in the image. Under /Oy- that is impossible for every body tried,
  so the open question is still what gives exec /Oy-style allocation with a
  frame; types (Z5) are ruled out (both counts are int in every reader).
- TexHeap::configure 0x00635050: the inlined shutdown in the loop-failure arm
  has immediates and `test` in the image while ours materialises the zero in
  edi (Z4: edi, the dead loop index, is shared and the shutdown loop counter
  then also starts from it). The image's other two copies agree with ours.
- DiploWin::exec 0x00443F10 (`constreg.py`: OURS_ONLY zero in edi, 0x444646,
  2 stores, 2 pushes): in the image edi holds a rectangle edge (`mov
  edi,[ebx+4]; sub edx,edi`, then `[eax+0xc]`) that outranks the zero, which
  then loses and stays immediate (spread stores, Z3 without a piece). The
  lever is that edge's candidate (a named copy), not the zero.
- save_game 0x005A9EB0: the image's edi/esi constants in the close tail are the
  two vtable pointers of a second inlined destructor (objects at -0x14b4 and
  -0x2000); ours does not expand the second one there. It is the teardown
  depth question (2.3), not constant allocation.
- mon_facility_built 0x00476FE0: the zero is in eax on both sides; the residue
  is the else arm storing `owner_faction` through eax at a shared join. Moving
  the store after the if/else (and four other spellings) loses the eax zero
  web (3 -> 6..16).
- `constreg.py --pieces` lists store runs that are a scratch piece on one side
  and immediates on the other (tree pass: Credits::exec, gen_terrain_poly,
  world_rainfall, enemy_strategy).

Zero webs on one side only (2026-11-20 par-1; candset.py over the 288 rung
4-7 functions, then single-reference probes)

- Z7 (high, BasePop::exec 0x00602600). **`mov r,[m]; test r,r` in the image
  against `cmp [m],zreg` in ours is a consequence of our zero web, not a
  spelling of the compare.** Ours holds 0 in ebx over 0x3E-0x194 (`xor
  ebx,ebx`, stores and compares through it); the image has no zero web there.
  Removing any ONE zero reference in a probe copy (the entry store `int
  pump_result = 0`, the `external_window_ = 0` store, or the
  `item_list_.entry_count_ == 0` memory compare) removes our web, and every
  compare in the stretch then compiles to the image's `mov eax,[m]; test
  eax,eax`. An inline accessor, a named `const` local, `!x` and `x == 0` all
  compile identically with the web present. So a zero that shows up on one
  side only is usually one reference (or one block weight) over or under the
  threshold; do not respell the compares, find what weighs the zero. Our
  likely extra weight in exec: the `lea ebp,[esi+0x21D0]` address temporary
  (&dialogs_, rule (a)) referenced in the zero's blocks (raises their `n_b`);
  the image forms that temporary only around `attach`/`show` (0x6025C7). Not
  yet resolved; the receiver spellings of set_width/set_height that give the
  per-arm `lea ecx,[edx+esi+0x21D0]` (58eb11f165) are the place to look.
- Z8 (high, PickTech::draw_tech 0x00484C60). **The image's zero register can
  be the loser's absence, not the zero's strength.** Image ebx = 0 over
  0x484C82-0x484DBB (10 refs: `cmp eax,ebx` for `tech < 0`, `cmp edx,ebx` for
  Font::line_advance's `adjustment >= 0`, two stores, six pushes); ours has
  the same ten uses as immediates and `test`, because the `rect` parameter
  wins ebx for its whole range (16 refs, ours 0x7D-0x689). The image never
  enregisters `rect` (it reloads `[ebp+0xC]` for each group of field reads).
  Tried without a fix: an inline layout helper for the five header stores,
  a `RECT *const` copy, a struct copy of `*rect` (294 -> 240, but a new
  frame RECT: rejected), a named wrap width, braces; `page_width_ - 4` as
  wrap_height's width gives 260 but reads the member where the image reads
  the rect fields (rejected).
- Z9 (high, 2026-11-20 par-2; `regalloc/c_zform.spec` and the generated
  grids behind it, about 400 probes, /Oy-, values 0, 5, -1; MapWin::init
  0x00470A90). **Formation, materialisation and rank are three different
  counts.**
  - *Formation* (does the constant become a candidate at all; seen as two
    adjacent stores turning from the Z1 `xor eax,eax` piece into Z2
    immediates): one more reference of any kind anywhere. Counted: a memory
    compare in every spelling (`!= 0`, `!x`, `if (x)`, `< 0`, `> 0`, char,
    pointer or field operands, even though without a zero register they
    compile to `mov r,[m]; test r,r`), a compare of an enregistered variable
    (`a != 0`, `test esi,esi`), a push, a store in another block, a char,
    short, pointer or field store, a parameter store, a propagated `int x =
    0` read once, and every one of these inside an inlined helper (inline
    copies count like direct ones). Not counted: a compare of a call result
    (`cond() != 0`, `< 0`), `return 0`, `a > 0 ? a : 0`, and the
    initialiser of a variable that is redefined later (`int x = 0; if (..) x
    = f();`: that is the variable's web, Z6).
  - *Materialisation* (`xor reg,reg` in a callee-saved register when one is
    free over the range): only **stores** of the constant count, each
    weighted 2^loop-depth, and the threshold is exactly **4** (S stores
    separated by calls, in if-arms, else-arms, `&&` arms, nested ifs or
    inline helpers: S = 3 never, S = 4 always; one store in a loop plus two
    outside = 4: yes; plus one outside: no). Switch-case stores count one
    each beside other stores (two stores + two cases, or one + four, do), but
    four or eight case stores alone did not. char stores count with
    the int ones and then force ebx (byte register). A `for (i = 0; ...)`
    counter's `i = 0` counts as one, and the counter then shares the zero
    (Z4). Compares and pushes never count toward the four: nine memory
    compares, nine pushes, or three stores with seven compares stay
    immediates. Four or more stores in one straight block with no call
    between them stay a scratch piece (Z1), and two if/else pairs each
    storing in both arms (4 stores) did not materialise where three pairs
    (6) did: the probes do not explain that one shape.
  - *Rank* against rivals (4g): stores count as in 4g; **memory compares add
    to the constant's score** (rival with 3 references: 5 stores + 2
    compares win ebx, 4 stores + 4 compares do not, + 8 do); **pushes add
    nothing** (5 stores + 8 pushes still lose; this is 4g's "constant pushes
    never formed a candidate" measured as rank); compares of the rival
    itself (`c == 0`) count for the rival (7 stores win against c, 7 stores
    + 2 `c == 0` lose).
  - Reading a one-side zero register: count the stores first. An image zero
    register over fewer than four visible stores means the original wrote a
    store more that cross-jumping merged afterwards (tail-merged references
    count, "Candidate set" above). MapWin::init: three byte stores of 0 in
    the image, ebx = 0 with 18 references; the inlined standard_caption
    (its `StringTemp[0] = 0` is one of the three) is shared by the
    `map_view_index_ == 1` arm and the other arm. Writing
    `standard_caption()` in both arms (`else if (map_view_index_ == 1) {
    ...; standard_caption(); } else { standard_caption(); }`) gives the
    fourth store, the image's ebx zero and every `cmp r,ebx` (53 -> 15; the
    rest is scratch eax/ecx choice). A ternary that the image computes
    branch-free (`neg; sbb; and`) is not the hidden store: the if/else
    spelling materialises the zero too but stores it as a branch (36).
  - `candset.py <unit> <addr>` prints a "constants whose formation differs"
    block: per constant and side, the stores (and their 2^depth weight,
    counting scratch pieces and register stores), memory compares (including
    `mov r,[m]; test r,r`) and pushes, and flags a register on one side whose
    store weight is under four.
  - Credits::exec 0x004289F0 (probe `unload` shape, c_zform-style): a
    pair of adjacent zero stores stays two immediates when one of the two
    stores has no displacement (`mov dword ptr [reg],0`, a field at offset
    0); when both stores carry a displacement (any offsets, 4 to 0xA14) the
    pair becomes the Z1 `xor eax,eax` piece. The earlier "offsets 0/4"
    reading was the `[reg]` store, not the offsets: Credits' own offsets are
    not the cause of its piece/immediate difference (still open). Under /Oy
    both are immediates.
  - `x = a && b && c` (stack_veh 0x005B8EE0, 46 -> 10). An assignment of a
    short-circuit chain writes one FALSE store (the chain's shared false
    target), so it adds one store to the Z9 count. When the image has a zero
    register the visible stores cannot explain, and the assignment's FALSE
    arm is the only zero store near it, write it as nested if/else with a
    store per failed test (`if (a) { if (b) { if (c) x = TRUE; else x =
    FALSE; } else x = FALSE; } else x = FALSE;`): three stores before
    cross-jumping (VC6 merges them back into the single store the image
    shows), which materialises the zero and gives every `cmp r,ebx` /
    `cmp [m],ebx` of the range.
    Do not nest where the image shows the chain's own codegen: `mov
    [x],1; <last test>; jcc over; mov [x],0` (or `mov r,1; jmp; xor r,r`)
    is `x = a && b` into a memory (register) variable, and the nested form
    compiles to `setcc` or to separate stores there (make_gift 0x00547C10
    `favored`, TutWin::tour 0x004BC6E0 `ridge`: the 151 -> 140 and 303 ->
    300 sweep gains are slot/register reshuffles with the wrong block shape).
  - Shared case labels (vehdraw_construct_chassis 0x004C2470, 140 -> 0).
    `case 0: case 9: body; break;` beside an identical `default: body;`
    lets VC6 merge the two blocks before jump threading: the jump table
    sends 0/9 to the default's code, the merged block loses its threaded
    jump past the following `armor == 10 ...` tests, and a case whose tail
    matches another (`mov ebx,2; xor edi,edi` against case 1's) jumps into
    it. The image kept every case block apart (table entries for 0 and 9
    equal, but a separate threaded block for them, and case 4/8's own
    `xor edi,edi`): writing `case 0: ...; case 9: ...;` and `case 4: ...;
    case 8: ...;` apart gives that, and the zero-test pushes of
    `armor_grade` (`test ebx,ebx; jne; push ebx`) then keep armor_grade in
    ebx instead of folding to `push 0` (one folded push alone hands ebx to
    faction_id). The table still shows one target per pair.
  - Census caveat (candset.py): `#0` is the zero *constant* web (every
    definition is `xor r,r`); `init#0` is a *variable* whose first
    definition is 0 and that is redefined later (Z6). Only `#0` rows are Z9
    material; an `init#0` on one side only is a variable-web question
    (rule 4 priority of that variable), and the store count does not apply.
  - DiploPop::draw_info 0x0043FA90 (constant 3 in edi in ours only, from the
    entry `+ 3` through the four loop-top adjustments): both sides have the
    same constant-3 references; the image keeps `r.right`'s reload in edi
    there. Reordering the four adjustments, `x = x + 3`, `RECT r; r = box;`
    and four spellings of the half-width expression do not move it (29).
- Z10 (high, 2026-11-20 par-2; `regalloc/gen_zsplit.py`, 135 probes, /Oy-).
  **A zero web splits only under register pressure, never at a gap.** Two
  groups of zero stores (1, 2 or 4 stores each, calls between) separated by
  a stretch without zero references stay ONE web (one `xor`, every store
  through the register, or all immediates under Z2) across: one or three
  calls, three conditional calls, an early `return`, a loop with a call, a
  call-free loop, two variables live across the stretch (the zero then
  takes ebx), and two variables followed by a third. It splits (a second
  `xor r,r` after the stretch) when three other webs are live together in the
  stretch and each outranks the zero there (`pqr`, `pqrs`: the variables
  take esi/edi/ebx, the zero's pieces both land in esi). If the zero
  outranks the weakest of them (8 stores against three variables of 2
  references, `f4_pqr_weak_l4`) it keeps its register over the stretch and
  the weak variable loses instead. One zero reference inside the stretch
  keeps the web whole only if it lets the zero outrank the third rival
  (`f2_pqr_z1_l4` whole, `f2_pqr_z1_l2` split with the inner store an
  immediate). Each piece then follows Z2/Z4 on its own: a piece whose
  register another web also uses is materialised even with two stores
  (`f1_pqr_l2`: first store immediate, last two through a fresh `xor esi`).
  Reading it: "ours one zero web from entry to teardown, the image local
  webs" means the image has three webs outranking the zero somewhere in the
  middle (or ours has one fewer, or a heavier zero); "two zero webs in ours,
  one in the image" (DiploWin::exec, ProdPicker::calculate in the screen
  below) is the reverse. Look for the third rival over the zero-free
  stretch, not for a call or a loop boundary. make_gift 0x00547C10: after
  the `gift_value = 0` and in-place base_value fixes (151 -> 112) both sides
  have the same three `#0` webs (candset); the residue is parameter-home
  packing (3.3 "Dead parameter homes are taken first": packrev wants
  gift_value(10) in receiver's +0xc, read first, where the image has the
  mode-8 `favored` web(5) and gift_value in giver's +8).
- Screen (2026-11-20, rung 4-7 functions): image-only `#0` webs in
  CouncWin::make_resolution 0x00424410, MapWin::init 0x00470A90 (ebx, 18
  refs), draw_tech 0x00484C60, vehdraw_construct_chassis 0x004C2470 (ebx, 22
  refs); ours-only `#0` in MapWin::right_menu 0x0046C0D0 (edi, 24 refs),
  ReportWin::on_left_click 0x00496E10 (edi, 18), energy_yield 0x004E7750
  (edi, 11), ReportWin::draw_labs 0x00498B30, ReportIf::draw_sat 0x004AA970;
  two zero webs in ours against one in DiploWin::exec 0x00443F10 and
  ProdPicker::calculate 0x00492FA0. Rerun: `candset.py <unit> <addr>` and
  read the "one side only" block for `#0`; then probe single references as
  in Z7 to find which side of the threshold the source sits on.

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

Candidate set (2026-11-19 par-2; `analysis/candset.py`, `regalloc/cand.py` with `c_*.spec`)

The order rules above are right on probes; real functions miss because the
*candidate set* differs from the source variables. `candset.py <unit>
<0xADDR>` (or `regprio.py ... --candset`) reads it from the code of both
sides: every callee-saved register web (reaching definitions over the CFG, so
a loop variable redefined at the bottom is one web), named by what its first
definition loads (`this`, `lea:this+K`, `lea:[R+R*k]`, `[slot]{name}`,
`wb[slot]`, `#K`, `init#K`, `ret@line`), scored by rule 4/4b at instruction
level, then checked: does each web's own side explain its register (every
lower register held by an interfering web that scores higher)? It prints the
roles present on one side only and the pairs the two sides order differently.

- **Instruction-level rule 4 explains 86% of the exact functions' rank
  constraints** (611 of 707 on the 90-function set; 663 of 903 interfering
  pairs). Two register-class constraints are part of it (high, every
  instance): a byte value (`bl`/`bh` operand) can only take ebx, and a value
  live across a `rep movs`/`stos` (struct copy) cannot take esi/edi
  (Caviar::add_object keeps `this` in ebx under eight lower-scored pieces).
  Constants weighed up or down, immediates counted as references and scratch
  webs in `n_b` all lowered the count; the residue is mostly cached
  memory loads (`mov r,[base+K]` kept for two uses), `this` and scaled
  addresses.
- **Reading a non-exact function.** If the image's allocation is explained by
  its own instruction-level scores and ours is not, the code has the image's
  candidates and our *source* weighs one of them differently (say_orders: the
  `&Vehs[veh_id]` temporary 90 against `text` 88, image explained, ours not;
  Win::flip: the `WinBubbleRect.left` load 18 against `split` 16, same). If the
  image is *not* explained by its own code (draw_garrison: `this` 19 in esi
  above the `this+0x40c2c` temporary 84 and two frame_rect pieces 30; popp:
  the `caption` piece 1 above the `label`/`flags` pieces 10), the original
  source references the winner in ways that leave no instruction. Of those,
  only tail-merged copies count (CSE and dead code do not): see "Tail-merged
  references count" below. Both popp and say_orders became exact that way.

How VC6 forms each kind (probes in `regalloc/c_*.spec`, `python3 cand.py c_X.spec`):

(a) **Address temporaries of a member** (high, c_cse, c_duel). `&member` used
    twice, or a struct copy of the member used twice, forms one `lea
    r,[this+K]` web at every offset tried (0x10 to 0x40000); field reads
    (`f[0]`, `g.l`) never do: they stay `[this+K]` and reload after calls.
    Once formed, the temporary takes every use of the member and `this` keeps
    only its prologue copy, the `lea` and its other uses (with no other
    use `this` is not a candidate at all). Score: def + uses for address uses
    (M uses of `&g` against K member calls: `this` = K+3 wins from K = M-2,
    ties to `this`, the earlier first reference). Struct copies are not
    additive and depend on order: `&g` first then a copy scores 6, a copy
    first then `&g` at most 4, two copies alone 6, two copies then two `&g`
    5; two adjacent copies (`R a = g; R b = g;`) form no temporary. So a copy
    statement moved before or after the first `&member` changes the
    temporary's rank without changing its code shape much.
(b) **Strength-reduced loop pointers** (medium, c_iv). `for (i = 0; i < 10;
    i++) use(garr[i])` becomes a pointer web compared with the end address
    (no counter). Against a variable referenced only outside the loop it
    weighs about 20 (it beats `d` with 18 references, loses to 22), far more
    than its three loop references; against a variable referenced in the
    same loop body it weighs its references (d wins with 3 uses per
    iteration). An index used besides the subscript stays a counter web (`i`
    used twice more takes esi above the pointer).
(c) **Memory-homed variables** are rule 7 losers; their pieces (rule 8) are
    webs of their own and appear as `[slot]{name}` roles. A parameter's whole
    web also starts with a slot load, so `[slot]` alone does not mean memory
    home: it is a piece when the same slot has several webs or is written
    back (`wb`).
(d) **`this`**: prologue copy + every use not routed through an address
    temporary (a). A named copy (`Buffer *const canvas = this;`) coalesces
    and moves nothing (MultiWin::draw_player).
(e) **Constants**: see "Constant candidates" above; `init#K` (a variable that
    starts as a constant and is redefined) is a variable, not a constant.
    A loser constant can get a frame home: MultiWin::draw_player parks the 1 of
    `1 << faction_id` in the dead `bounds` slot (`mov ecx,1; mov
    [ebp+0x10],ecx`) where the image recomputes `mov edx,1` per site; the
    signedness of the 1 (`1u <<`, per site or all) does not separate it.
(f) **Kept call results** are ordinary webs (`ret@line`); a call result
    copied into a named local coalesces with it.

regprio.py now builds its temporaries from these webs (`T:expr` takes every
instruction of its web, not only its address-ordered tenure; `--tenure-refs`
is the old count) and drops an index used only as `table[i]` when a temporary
replaces it (rule 10; `--keep-index`). It also stopped reading the EH unwind
funclets (`text$x`, offsets restarting at 0) as part of the CFG and building
a self loop's body from the whole function (both gave entry blocks loop depth
3). Pair accuracy on the 90-function set: 260 of 412 before, 263 of 410 with
the CFG fixes, 269 of 419 with web references.

Tried on the candidate-set functions, nothing moved (each a deck of natural
spellings, registers unchanged): draw_garrison 0x004103B0 (pointer, reference
or accessor for `garrison_rect_`, `this->`, assignment instead of
initialisation, `expand_rect`, end-of-function arms; field-by-field
`frame_rect` gives the image's `this`/temp order but reads `[esi+K]` per field,
12 rows); Win::flip 0x005EFD20 (loop forms, split declared inside, named
`left`, `rect_width`, `y` update as if/else, named delta, operand order);
say_orders 0x004B43E0 and popp 0x0048C0A0 (both now exact through their merged
statements, below); MultiWin::draw_player 0x00478630
(no `canvas` copy, unsigned shift constants); compute_score 0x005ACBE0
(`*total = *total + ...`, `total[0]`, the difference written to `items[5]`,
`other` loop forms).

Tail-merged references count; CSE'd ones do not (2026-11-19 par-2, high;
`regalloc/duel.py` with `m_*.spec`, mergescan.py)

Measured with duels (two candidates, the threshold N of uses of `c` at which
`c` outranks `d`; one reference in the shared block moves N by one) with an
extra statement added for `d` or for `this`:

| extra source (for d / for this) | instructions left | counts |
| --- | --- | --- |
| `(void)d;`, `{ int z = d; }`, `d = d;`, `if (0) use(d);` | none | 0 |
| `sink_a = d; sink_a = d;` (dead global store) | one store | 1 |
| `sink_a = d*3; sink_b = d*3;` against `d*3`, `d*5` | `d*3` once | as one `d*3` (+2 against +4) |
| `int e = d; use(e); use(e);` against `use(d); use(d);` | same | same (+2): a copy coalesces, no split |
| `if (f0 > 0) use(f0);` against `int v = f0; if (v > 0) use(v);` | one load | 1 each: the CSE'd reread of a member is free |
| `f0 = x; use(f0);` against `f0 = x; use(x);` | same | same: the reread after a store is free |
| `if (c) { call(); use(d); } else { use(d); }` (merged to one `push d; call`) against `use_a`/`use_b` arms (not merged) | 1 against 2 copies | same threshold: both copies count |
| three `use(d)` per arm, merged / not merged | 3 against 6 | same (+4 both) |
| three switch cases ending `use_a(d); break;` (one copy left) | 1 against 3 | same |

So the order of VC6's work is: CSE, copy propagation and dead-code removal
**before** register allocation (their removed references count nothing; a
named local copied from another candidate adds nothing and takes nothing),
tail merging and cross-jumping **after** it (every written-out copy of a
statement that the image shows once counts in full for each candidate it
references, in its own block's `n_b`). Caching a member or array read in a
local therefore changes the ranking only where it changes the code (a reload
after a call); the decompiler-style cached locals of this tree are not a
hidden weight. The hidden weight is the number of source copies of a merged
statement:

- **Ours writes a statement in several arms and VC6 merges them; the image
  ranks an operand of it lower than ours** -> the original wrote it once after
  the join, choosing the differing part in the arms. say_orders 0x004B43E0:
  `strcat(text, ...)` in the order_auto_type 11 / plan 9 / default arms (one
  `push text; call strcat` after merging) gave `text` three references and
  esi above the `&Vehs[veh_id]` temporary; `char *label; ... label =
  label_get(...); strcat(text, label);` (or a ternary, or the LPSTR id with
  one `table_text`) is exact, 90 -> 0. The code is the same; only the source
  count changed.
- **Ours reaches a statement through `goto` or a shared join; the image ranks
  an operand of it higher than ours** -> the original wrote the statement in
  each arm and VC6 merged the copies. popp 0x0048C0A0: `goto plain` / `goto
  missing` to one `start(caption, label, -1, 0, flags)` left `caption` at score
  1 below the `label`/`flags` pieces; a `start(caption, ...)` in every failing
  arm of a nested if/else (one `goto shown` each) is exact, 13 -> 0; the
  extra copies cross-jump into the image's two kept copies. A copy that
  does not merge in full changes the layout (a `goto` replaced by the whole
  target block costs rows when the block is not cross-jumped whole:
  MultiWin::draw_player's `goto not_in_turn` written out gives the image's
  `this` = ebx and faction_id above it, but 130 -> 180 rows; the original's
  arm structure there is still open).
- Screen: `recovery/tools/analysis/mergescan.py <unit> <addr>` (or `--from
  FEALL.json`, 50 s tree-wide) lists our source lines whose call left no
  `call` instruction although the callee is called elsewhere in the function
  (`MERGED`: a tail merged into another copy) with the identifiers of its
  arguments, and every `goto`. Read it beside candset.py's "register order the
  sides disagree on": a disputed role that is an argument of MERGED lines
  (fewer copies in the original) or of a goto target (more copies) is the
  lever. Inline expansions are not reported; repeated global-only statements
  (`strcat(StringTemp, ...)`) are listed but move nothing.
- More cases (2026-11-19 par-2): BaseWin::farm_click 0x0040A1D0, the
  `unchanged:` exit (recompute, unlock_base) written out in three of its five
  rejections gives the image's `this` rank, 269 -> 54 (all 32 subsets
  measured: writing it out everywhere is 59, so not every site was a copy);
  stack_veh 0x005B8EE0, one of two `goto lift` written out, 67 -> 52;
  MultiWin::draw_player 0x00478630, the `goto not_in_turn` replaced by one
  else-if chain whose first arm is the empty "in turn and answered" case,
  130 -> 57 (the chain, not a written-out copy: copies there cost layout).
  `analysis/gotowo.py` screens the tail-block form tree-wide.

Named copies of a field against the base-offset temporary (2026-11-19 par-4;
`regalloc/c_field.spec` with cand.py, `analysis/copycensus.py`,
`analysis/localinplace.py`)

The shape: `int v = Obj[i].f;` with M later uses of `v`, beside N other reads
`Obj[i].g` that go through the address temporary `T` (`lea r,[i+i*2]; lea
r,[i+r*4]; shl r,2`, reads `[r + Obj.g]`). Which of `v` and `T` gets the
callee-saved register, and what happens to the other.

Probes (one function per translation unit; `p` and `q` hold two registers, `T`
and `v` compete for the third; calls between the reads):

- **Counts decide and the temporary starts three ahead** (high; `whole_N*`,
  `loser_N*`, 14 families of 15 probes). `v` takes the register from
  M = N + 3 uses: 5, 6, 7, 8, 9 and 11 for N = 2, 3, 4, 5, 6 and 8. Below
  that `T` takes it, `v` gets a frame slot and each stretch of uses reloads it
  into a scratch register (`mov eax,[v]`, three compares share the load): the
  image's enemy_move entry. `T` scores its N reads, the copy's own load and
  its formation; `v` its M uses and the copy.
- **The pieces rank by their own references** (high). The thresholds are the
  same when `T` is a whole web and when it is a whole-function loser holding
  the register as its first written-back piece (rule 9: `shl esi,2; mov
  [slot],esi`). 16 later uses of `v` (`latepr`), 16 later reads through `T`
  (`latetmp`, until it wins the whole function) and 64 blocks that `T` lives
  through afterwards (`holes`) leave the first piece where it was.
- **A use spelled through the field again counts like the local** (high;
  `spell_*`, `push_*`): `garr[Obj[i].f]` for `garr[v]` with no call between
  compiles and ranks the same at every count, across a `&&` too. The copy
  does not split the web there.
- **A named pointer is another program** (`ptr`): `V *v = &Vs[i]` forms the
  absolute pointer (`lea esi,[Vs + r*4]`, reads `[esi+K]` without a
  relocation), never the offset temporary.

Census of the exact functions (`copycensus.py`, 4 minutes; ours is the image
there): 484 single-assignment locals initialised from a pure field or global
read, in 339 functions. Home of the copy by number of uses and by whether a
call lies between the copy and its last use (`x`):

| uses | call | register | register + slot | slot | neither |
| --- | --- | --- | --- | --- | --- |
| 1 | - | 19 | 0 | 14 | 40 |
| 1 | x | 17 | 1 | 3 | 3 |
| 2 | - | 38 | 2 | 12 | 125 |
| 2 | x | 10 | 0 | 4 | 22 |
| 3 | - | 14 | 0 | 4 | 25 |
| 3 | x | 8 | 0 | 2 | 4 |
| 4-7 | - | 11 | 1 | 5 | 24 |
| 4-7 | x | 19 | 0 | 3 | 9 |
| 8+ | - | 2 | 0 | 11 | 4 |
| 8+ | x | 18 | 1 | 1 | 4 |

"Neither" without a call is the copy living in a scratch register or folded
into its uses (218 of 351: a named copy that crosses no call usually costs
nothing and proves nothing). "Neither" across a call is the screen's blind
spot (42: the register move listed several lines below the copy, ebp under
/Oy), not a reread. Of the 87 copies it resolves across a call, 72 own a
callee-saved register, 2 a written-back register and 13 a slot; the slot
homes with many uses are the texture loops' accumulators and
`Vehs[veh_id].y` in a function whose three registers hold loop state. So a
real named copy that crosses calls normally shows a home in the image, and
the home is a register five times in six.

The rule for the non-exact functions:

- **A named field copy whose image has no home is an invention** (medium;
  the screen's kept changes below). If the image reloads the field at each
  use (`[T + Obj.f]`, or the field's own address) and never stores a slot or
  defines a callee-saved register at the copy, read the field in place. The
  local was taking a register, or adding a web to the entry block, that the
  image gives to something else. A copy with one use is the same case when
  the image loads the field lazily (after an earlier `||` operand failed).
- **A copy the image homes in a slot stays a local** even when our compile
  gives it a register: that is a ranking question (the first bullet), not a
  spelling. Reading it in place there changes the code (the image reads the
  slot, the variant reads the field: 16-bit compares on a `short` field).
  Exception: **a slot written once before a loop and read only inside it
  can be VC6's own loop-invariant hoist** (high, set_time_controls
  0x00523C60, 2026-11-20 par-2). The image's `mov eax,[human_mask]; and
  eax,0xff; mov [ebp-0x14],eax` before the faction loop comes out of the
  plain in-loop `G & (1u << f)` read; our `const unsigned active_mask = G;`
  at the function head was instead a named web with an entry definition,
  which won ebx as a write-back piece and kept the loop's counter out of
  it. Tell them apart by where the store sits: a hoist is stored in the
  loop preheader (after the loop's own initialisation), a named local at
  its source position. The reverse holds for a value read twice per
  iteration: when the image loads it once per iteration into a scratch
  register and ours CSEs the two reads into a callee-saved piece, a
  per-iteration local (`int refresh = TimeControl[sel].refresh;` as the
  loop body's first statement) gives the image's one load (72 -> 24 rows
  with both). Screen: `analysis/localinplace.py` (`ALL:v`) for the first,
  `ctrscan.py` hoist counts for both. Tree sweep 2026-11-20 (localinplace
  over all 309 non-exact functions under 800 rows, a per-statement and a
  loop-body-top per-iteration-local deck over 114 functions, ctrscan
  `hoist 0/1` loops): kept draw_energy_alloc (exact) and design_workshop
  (13 -> 8); every other rows gain was a slot the image does home
  (StringBox width, draw_farm base_y, draw_supported faction), an fe
  realignment in a giant (consider_designs 950 -> 607 with the image's
  CSE'd read already matching ours at the site, air_power) or slot
  numbering (draw_player_names). Read the image's rows at the copy before
  keeping a rows gain.
- **The in-place read is another program wherever the field can change**
  between the copy and the use (a store, a call). Only the image's rows at
  the site decide; never respell a statement that names both the copy and
  the field (`if (Count != count_at_entry)`).
Screen and results (`analysis/localinplace.py`, 2026-11-19). Per function it
lists the single-assignment locals initialised from a pure field or global
read and measures `ALL:v` (every use in place, local gone), `S:v@L` (one
use), and the reverse `R:v@L` / `RALL:v` (a repeated read spelled through
the local); key: named callee difference, frame, candset count, slot map,
rows. An automatic keep never raises the rows and a rows-only keep needs
three. Pass over the non-exact functions of 100+ rows (other workers'
skipped): 122 functions, 54 with no such local, 46 neutral or worse, 22
kept, no exact function lost: BaseWin::draw_supported 228 -> 171,
veh_draw 159 -> 115 (candset 18 -> 8: the icon test rereads
`Vehs[veh_id].proto_id`), consider_designs 1059 -> 958 (the chassis copy
with eight uses), diplomacy_check 787 -> 749, game_data 240 -> 216,
gen_terrain_poly 1585 -> 1562, battle_fight 2138 -> 2120 (named callee
difference 12 -> 8), drone_riot 332 -> 317, PickTech::pick 825 -> 811,
world_rainfall 264 -> 251, draw_farm 386 -> 374, base_production
301 -> 289, and ten of 3 to 9 rows. All but five are the `ALL` form; the
reverse (`R`) never won. order_veh's one candidate is a saved count compared
with the live one (excluded); enemy_strategy has none; time_warp,
base_build and probe are neutral.

enemy_move 0x0056B5B0 stays open, with the question narrowed. The image is
the probe's natural outcome (`chain`, `mini` in c_field.spec: `T` in the
free register with write-back, `proto_id` in its slot, three compares on one
`mov eax,[slot]`, `remote_own` stored as immediates). Ours is not a count
race: by the probe law `proto_id` would need twelve uses to beat `T`'s nine
reads, and it wins with a copy and one compare. Diagnostics (volatile
locals and deletions, not source): `T` takes ebx only when nothing else
wants it. It loses to `remote_own`'s piece, to `proto_id`'s and, with both
volatile, to `faction_id`'s written-back piece; every variant that showed
`T` in ebx (`has_abil(proto_id, ..)`, the plan through `proto_id`, the
planet_buster statement removed, `dmg` read in place, x or y removed) did so
by moving `proto_id` to edi or esi. `proto_id` owes its piece to the
`has_abil` statement (without it `proto_id` is memory-homed with the
image's `mov eax,[slot]` compares). Measured without effect on that order:
about 30 single-site respellings of proto_id/faction_id/x/y in the entry, nine
placements of the copy statement, the `k` test as if/else, three
`remote_own` forms, the compares nested or negated, a truncated function
(another regime: everything wins a register). So something outside the
entry's spelling ranks our `T` below every named piece; the address
temporary itself is the same web on both sides (one store to [ebp-0x10],
167 loads in the image, 175 in ours).

Homed addresses (2026-11-19 par-6; `analysis/addrhome.py`,
`regalloc/c_addrhome.spec`)

The shape: the image stores an address into a frame slot where it is first
formed (`lea ecx,[esi+K]; mov [slot],ecx`), reloads it much later and lets
the base (`this`, an index) die in between, where ours keeps the base in its
register and re-forms the address. What it is, by kind of address:

- **It is not a named pointer local** (high). `T *p = &member;` and
  `T *p = &arr[i];` are forward-substituted: BaseWin::init with `Font *font`
  / `FlatButton *button` locals compiles to the same code as without them;
  custom_planet with two pointer locals reads 84 rows where the plain
  subscripts read 11. Do not add pointer locals for a homed address.
- **VC6 forms the temporary itself, always** (high; `mid/N`, `fold/W`). The
  second use of `this+K` (a member's address) or of `&arr[i]` with an element
  of 1/2/4/8 bytes (`lea r,[i*4+G]`) is a CSE of the first. With a register
  free the temporary keeps it; without one it is stored at its definition and
  reloaded at the reuse, and the base ends in a destructive `add edi,K`. The
  probe gives the image's shape from the plain source, at any distance (64
  loops or 29 distinct members between definition and reuse).
- **The temporary survives only while nothing later names the base**
  (high; BaseWin::init 0x0041E8D0, `fold/W`). One later expression that needs
  the base itself keeps the base live through the region, and then every
  temporary without a register is re-formed from it instead of homed, and the
  loop counter loses its register to the base. In BaseWin::init that
  expression was `flatButton_.set_font(..)`: a direct call of a base-class
  member on a member object folds the two offsets into `this+0xC2C4`. The
  image has `lea ecx,[ebx+0x444]`, the base formed from the register that
  holds the button: the `this` of an expanded header inline member
  (`set_font_redraw`). So when the image homes addresses and lets `this` die,
  look for the LAST expression of ours that still names `this` after the
  image's last use, and for the image's `[reg+C]` form of it.
- **An expanded inline member on a member object unfolds the offsets; on a
  global array element it does not** (high; `idx/W`). `gob[i].set(a,b,c)`
  expanded is the same code as three field stores (the offset temporary,
  reads with the relocation). An OUT-OF-LINE member call on `gob[i]` forms
  the absolute pointer for `ecx` from the offset (`lea edi,[edi+G]`), and the
  reads after the call go through that pointer without a relocation. A named
  pointer local forms the absolute pointer at once (`lea esi,[i*4+G]` after
  the scaling). So in the image `lea r,[off+G]` followed by pointer reads
  means a member call on the element or a pointer local, and offset reads
  with the relocation mean subscripts (or an expanded inline).
- **Whether a `&arr[i]` temporary is homed can be a symbol-counter tie**
  (custom_planet 0x0058C2A0: 11 rows at 44 pad counts with both addresses
  homed and `slot` dead, 373-412 at the other 20 with `slot` kept).
  BaseWin::init does not move with the counter.

Screen (`addrhome.py`, 50 s tree-wide). Slots: address or scaled-offset
values stored to the frame and reloaded, by kind (Pg `lea [i*k+G]`, Po
`lea [off+G]`, Pm `lea [base+d]`, OFF) and by how the reloads are used
(pointer, offset with relocation, index, `this` of a call, argument),
paired with ours through the fe alignment. The slot rows are noisy (shared
slots, linear tracking): of 14 IMG-PTR and 22 OURS-PTR rows on 2026-11-19
none was a source pointer. The useful rows are **UNFOLD**: the image forms
`lea r,[b+Kt]` and reaches the parts as `[r+d]`, ours has the folded
`[b+Kt+d]`. 21 rows in 8 functions led to: BaseWin::init 147 -> 90
(`set_font_redraw`), MainInterface::init 348 -> 321 (seven
`set_report_icons` triples written as member stores), NewTechWin::on_redraw
83 -> 0 (`scratch = rect; expand_rect(&scratch, 3, 3)` for four folded reads,
then `rect_width(&scratch)` / `rect_height(&scratch)` in the arguments),
NetWin::exec and NetSessionPickerWin::initialize (`set_font_redraw`),
DiploWin::exec (a `Buffer &` cast of a member compiles a null test; the image
calls the member directly). Left as ranking questions (the temporary exists
on both sides, the image keeps it in a callee-saved register that ours gives
to a constant): CouncWin::init `this+0xD04` in ebx from 0x427632 to 0x427C8D
(ours `xor ebx,ebx`), DiploWin::exec `this+0xF3C` in ebx at 0x444640,
ReportWin::labs_line `this+0xE258`. Left as an inline-body question:
SocialWin::init 0x4B28C0 and NetWin::exec 0x48285E set three rectangles
through a pointer with every value in a register (`mov ebx,0x9a; mov
[edi],ebx; .. mov ebx,edi; mov edi,0x13c; mov [ebx+8],edi`); ten bodies of
a four-argument rectangle inline (modified parameters, two levels, returned
pointer, pointer copy, reference, `__stdcall`, non-static) all fold to
immediate stores.

Member-address temporaries that live across calls (2026-11-19 par-6, round 11)

- **Field reads end the web; address uses extend it** (high, DiploWin::exec
  0x00443F10). The image held `lea ebx,[esi+0xF3C]` (formed for the struct
  copy into rect_F3C_) across GraphicWin::init, fill and box_sprite and read
  `[ebx+0xC]-[ebx+4]`, `[ebx+8]-[ebx]`; ours folded those reads to
  `[esi+K]` and ended the web at the copy. `rect_width(&rect_F3C_)` /
  `rect_height(&rect_F3C_)` (the win.h inlines, a pointer argument) give the
  image's web; `rect_F3C_.left + 3` then reads `[ebx]` (same value number as
  the temporary) and `.top + 3` `[esi+0xF40]`, both as in the image. A
  `[ebx+K]` read after calls where ours has `[esi+base+K]` is the screen.
- **A pointer local's later reads fold when it loses the register** (high,
  CouncWin::init 0x00427230). `bottom_row = &leader_rects_[19]` stays a
  pointer (`[ebx+K]`) through the vote loops in the image; ours keeps the
  piece only up to `*bottom_row = rect` and re-forms the rest as
  `[esi+0xD08]`, while `first` (same kind) keeps edi. rect_width/rect_height
  on the pointer changes nothing (it is already an address); this is a
  ranking question. Count lever confirmed: writing the `buttons_[5]` block
  in both arms of `if (!field_A1C_)` (more source copies of bottom_row) keeps
  bottom_row and first both live across the loops as in the image, but with
  the names swapped (bottom_row edi, first ebx) and arm1 not cross-jumped as
  the image (the image's arm1 merges only the final `call`): right
  direction, too much weight, not the original statement. The vote loops'
  counter is homed in [ebp+8] in the image: the loops count in `proposal`.
- **A Dialog member setter called on a virtual base keeps the full pointer
  two inline levels down and folds at one** (high, BasePop::exec
  0x00602600). `dialogs_.set_field_2C(v)` inside set_width: the standalone
  set_width stays `mov [edx+ecx+0x21FC],eax` (exact), the copy expanded in
  exec forms `lea ecx,[edx+esi+0x21D0]` per arm and shares `mov
  [ecx+0x2C],eax` after the join, as the image. The qualified field store
  (`dialogs_.Dialog::field_2C_ =`) folds at both levels. The two setters
  are declared for every includer, as an original header would; the
  counter-sensitive functions that move with them are re-tuned in the pad
  pass, not protected by a per-unit guard.
- **A parameter cached across a loop in ours but read from memory inside it
  in the image is usually a candidate-set error elsewhere, not a piece rule**
  (high, setup_game 0x005B3920, 132 -> 0 rows, `regalloc/c_sr.spec`,
  `regalloc/c_looppiece.spec`, 2026-11-19 par-6). Ours kept `reload` in esi
  from before the first setup_player loop to the `reload == 0` test (candset
  16 refs, 56.5, above the zero constant); the image reads `[ebp+8]` into eax
  inside the first loop and caches it in ebx only after it. No spelling of
  the loops, the call argument (`~r`, `-(r + 1)`, a named temporary, an inline
  helper taking `r`), the loop test (`is_human`, the raw mask, `continue`,
  `while`) or the counter moved the piece. The cause was the social loop
  earlier in the function: the image steps a pointer to `Players[1]` (`mov
  edi,&Players[1].soc_ideology_model; ... add edi,0x59c`), ours recomputed
  `social_faction * 0x59c` per iteration (`lea edi,[ebx+ebx*4]` ..., an
  11-reference address CSE). With the pointer, `reload` falls into the
  image's pieces (entry piece in edi, none in loop 1, ebx after it) with no
  change to the loops themselves.
  - **Strength reduction is a candidate-set question** (high, c_sr.spec).
    VC6 replaces `T[i].f` by a pointer stepped by the stride only when `i`
    is not read inside an inner loop of the body: `++rejected < i + 1` in a
    do-while blocks it (base), and dropping that read (nouse_sf) or the
    inner loop (nodo) restores it (and then deletes `i` if nothing else reads
    it). The image keeping both `i` (ebx) and the stepped pointer means a
    source pointer stepped beside the index: `T *p = &T[1]; for (i = 1; ...;
    i++, p++)` (ptrinc). The pointer's base is the middle field the body
    touches (`[edi-8]`, `[edi]`, `[edi+8]`), in source and SR alike.
    `candset.py` prints "pointer induction in the image only" with the
    stride when the image steps a callee-saved register by a stride ours
    lacks, beside our `lea:idx` webs.
  - **SR blocking: one induction variable per stride class** (high, par-1
    2026-11-20, `analysis/probes/srblock/run.py`, 70 variants). VC6 forms
    at most one derived IV per element stride among the arrays a loop
    subscripts by `i`, and turns it into a table pointer only when exactly
    one array uses that stride:
    - one `int` table `tab[i]`: a pointer (`mov esi,OFFSET tab ... add
      esi,4`), whatever else reads `i`. A call argument, `i / 8`, a compare,
      a modified `i`, an unknown bound, `char`/`short`/`unsigned` counters,
      `*(tab + i)`, an inline `at(i)`, `while`/backward `goto` loops, a
      function-scope `i` read after the loop, a `switch` in the body, a
      `goto` out or an inner loop reading `i` all keep the pointer (then
      `i` is a second register beside it: so the pointer-plus-index pair is
      NOT by itself evidence of a source pointer, contrary to the ptrinc
      note above). 64 earlier loops in the function do not stop it.
    - two or more arrays of the SAME stride (`tab[i]` and `tab2[i]`, read
      or written, global, local or through an `int *q`): no pointer. With
      `i` read elsewhere they are `[i*4+tab]`, `[i*4+tab2]` off the counter
      (IDX); with `i` used only as the subscript, one byte offset
      `[off+tab]`, `[off+tab2]` stepped by 4 (OFF). Arrays of different
      strides (`int` and `short`, `int` and a 32-byte struct) each get their
      own pointer; `tab[i]` and `tab[i+1]` are one array.
    - `&i` taken anywhere, or an irreducible loop (a `goto` into the body):
      IDX.
    - a constant-left compare of `i` in the body (`9 <= i`) turns the
      two-table IDX back into OFF and compares the offset (`cmp esi,0x24`)
      while keeping `i`; `i >= 9` compares `i` (guide 3.2, world_site, for
      the loop test itself).
    Reading the image: `[i*4+table]` with `i` live elsewhere and no stepped
    register means a second same-stride array indexed by `i` in that loop
    (or `&i`); look for one the recovery moved out. base_build 0x004F8931:
    `a_694[i] = 0` belongs in the project census loop (VC6 distributes the
    store into the rep stosd ahead of it, so the clearing loop looks
    separate), census window exact except a slot. spot_all 0x005B6A0D: the
    image's `cmp [i],9` against our `cmp ebx,0x24` was `9 <= i`. A dead
    `and r,K` beside a mask test (spot_all 0x005B69FC `and edx,0x80001000`)
    is a second `(v & K) == C` test whose consumer was dropped: a dead
    counter `if ((bit2 & 0x80001000) == 0x1000) n++;` reproduces it (not yet
    applied: the extra local moves counter-sensitive ties later in the
    function). Correction (par-1 2026-11-20, `analysis/probes/deadand/run.py`,
    18 spellings): VC6 deletes a dead counter's test whole, `and` included
    (a dead `d++`, an empty arm, identical arms, a `continue`/`return`/
    `goto` to the fall-through, a case- or `k = 2`-folded guard); every live
    consumer keeps a `cmp`. In real spot_all the dead counter (`else if`
    or sequential, local or not) removes the `and` too and moves the
    callee-saved allocation of the whole function (1701 -> 1981 rows). No
    measured source leaves the image's lone `and edx,0x80001000`; it is
    open.
  - **Index or address CSE: which form a repeated subscript keeps** (high,
    par-1 2026-11-20, `analysis/probes/idxcse/run.py`, 70 variants). For
    repeated `t[E].f` / `t[E*k+c]` with no store to E between the uses, VC6
    CSEs one of three values:
    - the uses read DIFFERENT addresses (two fields of one row, two arrays
      with the same subscript, `x[r*3+2]` and `y[r*3+2]`, a load and a
      store of different members): one byte offset, `lea`s then `shl r,2`,
      each use `[r+tab+disp]`. Spelling (2-D array, row struct, pointer
      arithmetic, `2+a*3`, `3*a`, a short/char/unsigned index, an inline
      `at(t,r,k)`, a call between the uses, a copied local `int r = a`)
      does not change it, nor does function size (4096 filler statements).
    - every use reads the SAME address (`t[E].f` twice, `++` / `x = x+1`
      of one element): the row index E*k unscaled in a register and
      `[r*4+tab]` at each use, no `shl` (make_gift 0x548516 `inc
      [ebx*4+K]`).
    - a reference or pointer to the element (`int &c = t[a].f[b]`,
      `R &r = rows[a]`, `V *p = &vs[G]`): the address, `lea r,[idx*4+tab]`
      and `[r]` / `[r+f]`. The image's `[r*4+K]` against ours `[r]` means
      the original subscripted at every use (make_gift: the gift counter
      and the patience byte; `+= -2 - favored` on the subscript).
    The scaled form with DIFFERENT addresses appears only when the row
    product is the value of a variable, not an expression in the subscript:
    `int r = a*3; x[r+2] + y[r+2]`, `a *= 3`, or (the common case) a
    variable VC6 knows is constant only after the CSE: `x[k + a*3]` with
    `k = 2` assigned, inside `case 2:` of `switch (k)`, or under `if (k ==
    2 && ...)` (but not `||`). base_build 0x4FA13E/0x4FA3F6/0x4FA894 are
    `a_3c4[l_20 + l_2c*3]` in the triad switch's cases; a literal `+2`
    gives `shl eax,2`. The same constant written literally where the image
    has `shl` (0x4FADF5, outside any known-l_20 region) stays literal.
    Two uses split across a different index expression do not CSE at all:
    gen_terrain_poly 0x463888 computes v*13 twice (owner byte through the
    global `Vehs[VehDrawPositionVehicle]`, then a local copy's v*13 in
    place in esi for both proto_id reads); one local for all three gives
    one byte offset. Reading the image: `shl r,2` before two different
    displacements = repeated subscript expression; `[r*4+K]` at different
    displacements = a named or case-constant row variable; one `lea
    r,[i*4+K]` then `[r]` = a reference/pointer local. The commutative
    operand order of a later CSE'd sum can flip when an earlier site stops
    creating the same subscript (base_build 0x4FADF5: write the earlier
    order, `a_2ec[..] + a_3c4[..]`).
  - A loser's single reference per loop iteration (low, c_looppiece.spec):
    with no call between two such loops, each read is a scratch load
    (`mov eax,[r]` right before its use, and the loop test loads the mask
    into a scratch register: `mov cl,[mask]; test al,cl`); one call between
    the loops turns both into a per-iteration callee-saved load (`mov
    ebx,[r]`, and the test reads memory: `test byte ptr [mask],al`). The
    test's form tells which of the two the image has.
  - Same function: two `strcpy(x, label_get(245))` reads are written out as
    `StringTable.get(Labels.strings_ptr[245])` (the image loads the label
    pointer into a fresh register and pushes it before loading `this`; the
    inline's named local loads `this` between).

Home reload of a register-held value (2026-11-20 par-1; standalone probes of
the mon_* shape, about 45 spellings, plus decks on make_gift)

The shape: the image reads a variable from its home (`mov ebx,[ebp+8]`) where
VC6 knows a register holds it, ours copies the register. Instances:
mon_* (eleven producers, the parameter in esi/edi everywhere else, reloaded
once as the peer arm's default owner), make_gift 0x547DDE (`amount = value`
right after `value = popup.exec()` was stored to giver's dead home and tested
in eax; ours `mov ebx,eax`).

- R1 (high). **No plain source form of a copy `x = param` reads the home.**
  Copy before/after a store, in each arm or hoisted, declaration at the top,
  after the call, before the if; a local copy of the parameter (int,
  unsigned, long, enum, `const int &`) used everywhere but the arm; `const`
  parameter; `f = f;` web splits; a pointer to the parameter; by-value,
  `const int &` or `int &` inline helpers (scan only, or the whole body);
  for-init, do-while, inverted if, a ternary: all are a register move, and
  the extra copy relation also moves the parameter from esi to edx (it is
  live across no call). A real address-take (`if (0) take(&f)`, a store of
  `&f` to a global, a call after the last use) makes every use a memory read.
- R2 (high). **`*(volatile int *)&param` reproduces the image exactly** (ten
  mon_* producers byte-exact): the parameter keeps its register elsewhere and
  only that read is memory. It stays as `WORKAROUND(volatile)`.
- R3 (medium). The same stand-in does not carry to make_gift: a volatile read
  of the homed `value` gives the reload but drops `amount`'s write-through
  home (frame 4 smaller, 300 rows). Splitting `value` into selection/divisor
  variables, `== 0`, a late `amount` declaration and wrapping the rest of the
  arm in `if (value)` leave the 3 rows or worse.
- mon_secrets_of_tech 0x00476DA0 (5 rows: the image defines the owner in each
  arm, eax, reloaded in the peer arm) and mon_facility_built 0x00476FE0 (3
  rows: the image stores the owner once at the join) stay at their minimum:
  every arm-defined or join-stored spelling, plain or volatile, moves the
  year/zero webs (12-47 rows).

Write-through pieces (2026-11-19 par-5 round 14; `regalloc/c_wthru.spec`, about
300 probes, and a scan of the image)

The shape: a callee-saved register holds a variable that also has a frame home,
every definition in the stretch is `mov reg,K; mov [home],reg` (or `mov
reg,eax; mov [home],reg`), and the register is reloaded from the home at joins.
What is established, and what is not:

- W1 (high). **A prefix piece writes every definition through.** Rule 9's loser
  holds the register from its first definition until the winner's web starts,
  and every definition inside the prefix is stored, redefinitions and
  read-modify-writes included (tech_val 0x005BCBE0, exact: `add edi,ebx; mov
  [ebp-4],edi` twice before the address temporary takes edi; ListBox::on_key_down
  0x0060AF90 image: `count` in edi with `mov [esp+0x10],edi` until `old_top`'s
  web starts). Probe `prefix/nolate`: c gets the stored prefix when d is first
  defined after it.
- W2 (high). **A constant initialiser never starts a prefix.** `int d = 0;`
  early makes d's web start there: d either wins the register from the `xor`
  (`prefix/lateinit`) or stays in memory with immediate stores; it never
  hands the stretch before its later definitions to another loser.
- W3 (high). **An ordinary memory-homed loser never writes through in the
  middle of its range.** With a callee-saved register free over a stretch of
  its references (rival dead before, next winner born after), its reads are
  cached (rule 8: `nd/k_d4_r8`, `dr/d4_r8`) but each definition stays a memory
  store (`mov [o],K` or `mov [o],eax`) followed by a reload. Measured with 2-8
  constant, call-result and unconditional definitions, 1-8 reads, the region
  repeated up to 8 times, local or parameter, init early or late: no probe
  formed a mid-range write-through piece. A repeated constant takes the free
  register instead (`#6` piece, Z1).
- W4 (high). **Address-taken locals are cached with write-through only between
  calls.** Win::center 0x005ED5C0 (exact): `x`, passed as `&x`, is `mov edi,eax;
  sar edi,1; mov [esp+0xc],edi`, updated in edi with a store per definition,
  and reloaded after the call that receives its address. With a call between
  definition and use (`at/self`) it is never cached. Sharing a frame slot with
  an address-taken local does not make a variable address-taken (`at/noat`,
  `at/press`: can_terraform's `order` lives in x's slot, which the inlined
  `bitmask(.., &offset, ..)` also uses; that is slot packing, not the cause).
- Open. A mid-range write-through piece of a constant-assigned local with a
  reload at the join is in two image functions only (`mov reg,imm; mov
  [slot],reg` twice or more plus `mov reg,[slot]`, scanned over every
  function): can_terraform 0x00565320 (`order` in ebx from the solar test to
  the enricher test, 15 stores, reloads at 0x565A54 and 0x565BE3; the last
  definition 0x565D67 and the final read are memory) and enemy_strategy
  0x00561080 (ebx, [ebp-0x38]). No exact function has it, so no measured source
  produces it; under W2/W3 the image's piece behaves like a web that starts at
  0x565A4A, i.e. the original's `order` before the solar test is a different
  web or memory-only (W2 forbids the plain `int order = 0;` there). The
  region's rival is `solar_tech` (image: stored prefix in ebx up to the piece,
  W1; ours: whole ebx to 0x81B), not `faction_id`. Moving the declaration
  next to the region (400 rows from 325), dropping the enricher definition,
  the result copy or a solar_tech read (diagnostics) never gave the piece.
- Applied without a flip (decks in the commit message): ListBox::on_key_down
  `field_18_` read through the member in the DOWN case (172 -> 162 rows; the
  image's frame 0x14 needs `count` memory-homed, ours keeps it whole in ebx
  and `old_top` in memory); repair_phase 0x00526030 (frame 0x28 against 0x24:
  ours caches `repair_rate` in ebx from the nano-factory assignment to the
  `reactor_id *` multiply, so `reactor_id` reuses its slot; the image keeps it
  in memory, `imul edi,[ebp-4]`, and starts the `faction_id` piece in ebx at the
  join; operand order, a split multiply, a late `repair_total`, a cast on the
  assignment: all 16 rows, frame unchanged).

Open terms (2026-11-19 par-5 round 16). Patterns seen in several non-exact
functions that no rule above explains, with what has been ruled out.

- O1. **Hidden references: dead and folded copies are not them** (high for the
  negative; `regalloc/m_hidden.spec`, `m_hidden2.spec`, `python3 duel.py`).
  candset.py shows image winners ranked above their own instruction-level
  score (shoot_it 0x004CDAF0: `defender` 45.5 in esi above the `mask`
  write-back piece 56.5; DiploPop::start 0x0043F2B0: `faction_id` 33.0 in ebx
  with edi held by lower webs, around `alternate_size`). The idea that the
  original had copies the optimizer removed after counting them is refuted:
  each of these, added for `d` in a c/d duel (threshold 8), moves nothing: a
  local copy overwritten before use (`int t = d; t = def();`), a dead store
  in one arm, `d - d`, `d * 0`, `d && 0`, statements after a `return`, an
  expanded inline that ignores its parameter (one or three calls), `k ? x : 7`
  with constant k, `d == d`, a copy loop that never reads it. A second
  identical `if (d > 0)` (sequential, nested or in the else) is merged before
  allocation and counts once; an inline that uses its parameter twice counts
  like two direct uses; a while test counts once per loop weight and a `do`
  with a guard test twice. So the only reference that leaves no instruction
  and still counts is a tail-merged copy (table above): for these two
  functions read mergescan.py and the goto/arm structure, not dead code.
- O2. **Commutative operand order follows a web property, not the spelling**
  (medium; probes in this round's scratch `tp/`). PrefWin::get_preferences
  0x00490B00 has the same registers on both sides but 16 `test value,bit`
  rows in the image against `test bit,value` in ours, and three member tests
  `mov eax,[this+0xA50]; test eax,edi` (image) against the folded `test
  [ebx+0xA50],edi` (ours). Only the second operand of the tree folds, so the
  image's tree has the value first. In a four-case probe of the same loop:
  `prefs & bit` and `bit & prefs` compile the same; 1-6 extra references of
  `prefs` or 3 of `bit` do not flip it; int/unsigned on either side does not;
  per-iteration copies (`u32 prefs = member;` in the loop) give the image's
  `test prefs,bit`, the same copies made once before the loop give ours'
  `test bit,prefs`, and a direct member read folds (ours' third-loop form);
  an `unsigned short bit` gives the unfolded load. In the real function the
  per-iteration spelling still gives ours' order, so the order encodes some
  other state of the value's web (dropping the `id`/`bit` initialisers: 58 ->
  55 rows, no flip). Treat a swapped `test`/`and`/`add` operand pair with
  equal registers as a candidate-set signal, not a spelling to permute.
- O3. **Scratch-held loop accumulator with a constant initialiser** (high;
  this round's scratch `pm/`). polygon 0x00626620 (and polygon_dest,
  polygon_gouraud, Texture::sub_61f0d0 by the same `0x7fff` shape): the image
  keeps `minimum_y` in edx with `mov edx,0x7fff` scheduled between `test
  esi,esi` and the loop's `jle`, and the loaded `y` in edi; ours has `mov
  edi,0x7fff` early and `y` in ecx. The image holds the constant, so the
  original did not start from the first vertex. A plain probe of the loop
  (no volatile) reproduces the image's minimum exactly (edx, same schedule);
  the `*(volatile int *)&vertices[vertex].y` stand-in for the max arm's
  reload (0x6266D1 `mov ebx,[ecx]`) is what promotes `minimum_y` to a
  callee-saved register. So W2 is not involved; the open term is the reload:
  the image compares through the cached `y` and assigns from memory. Not
  reproduced by a named `y`, a named `y` for the compares with the field for
  the assignments, the compare reversed, a pointer walk, or a global store
  between (it reloads both reads). In polygon: late, for-init and early
  `minimum_y`, named `y`, pointer walk: 25 rows each; any variant without
  the volatile: 68-77.

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
     (`force_if4`, `force_big`). Probes only: in DesignWin::select_special
     `__forceinline` and `inline` helpers measure identically (2026-11-20,
     see "`__forceinline` spends the same budget" below).
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
   **Depth cap 8** (high; turn_upkeep 2026-11-20 par-1): a chain of free
   one-line forwarders expands up to depth 8 and the depth-9 call stays out
   of line, wherever the chain sits (no leak between sites). Arguments nest
   too: `idf(idf(..veh_health(v)..))` seven deep changes range decisions.
   A refused deep site also changes its sibling site's decision (suffix
   regime), so screen a whole call pair, not one site. Nesting the sites
   one level deeper inside a natural owner (predicate, crash helper) is how
   turn_upkeep's two out-of-line reactor ranges were reached without a
   function-pointer twin; dead spend alone needed about 72 if-units in the
   loop.
6. **Teardown with real headers** (high; `real_popup.py`). An isolated
   `Popup p;` with M returns expands ~Popup at every site up to the budget.
   Its ~Scroll then falls in the periodic regime: 5 returns give
   `SSSSSs`, 10 give `sssssssSsss`. From 20 returns on, the closing sites
   call ~Popup itself.
7. **Total-size ceiling: the hard stop** (high; `inline/giant.py`,
   2026-11-19 par-2 round 16). Rule 2's B = k × caller size has a cap: the
   function's running total (its own size plus every expansion accepted so
   far, free ones included) may not pass a fixed ceiling T. Measured with
   one caller of N filler `if (ga[i] == i) use(i);` and a charged 4-if site
   after every tenth filler (`giant.py N 10 if --charged`):

   | N fillers | 1300 | 1400 | 1500 | 1600 | 1700 | 1800 | 2000-6500 |
   | --- | --- | --- | --- | --- | --- | --- | --- |
   | 4-if sites expanded | all 129 | 110 | 88 | 65 | 42 | 12 | 12 |

   N + 4.4 × expansions stays at T ≈ 1885 filler ifs (about 5300 global
   stores with `store` filler): every 100 fillers more take about 23
   expansions away. The cut is a running total in source order, but it
   counts the *whole* caller's own size from the start: `--front` (every
   site first, then the fillers) stops at the same 65. Consequences:
   - **Everything after the stop is refused, free inlines too**
     (`--mixed`, free `fr(k)` sites between the charged ones: N=1600 gives
     43 charged and 43 free expansions, then every site of both kinds
     called). This is battle_fight's and order_veh's round-15 "hard stop".
   - **Only the B0 floor survives** a caller that is over T by itself
     (N ≥ 1800): about 58 units of charged sites, and free sites expand
     without limit (N=3000 `--mixed`: 12 charged, all 300 free). So a free
     site is refused only when both the B0 floor and the ceiling are spent.
   - **Not a budget**: free probes, dead blocks and helper spellings cannot
     buy room past the stop. Every statement anywhere in the function, and
     every byte of every expansion before the stop (an inline body's dead
     temporaries, a deeper member chain), moves the stop earlier. So the
     original giant was smaller by at least the overshoot; dead budget
     probes in a ceiling-bound function cost twice (their nested spend and
     their size).
   - **Screen**: `analysis/ceilscan.py [unit ...]` puts a free probe at the
     top and at the end of every non-exact function, one compile per unit
     (18 s tree-wide with `--all`). `Ec` = ceiling-bound; `--bisect --addr A`
     spreads `CEIL_PROBES` (default 12) probes over the body and prints the
     last expanded one (each expanded probe adds size: use few). At head
     a61619e09f + round 16: exactly four functions are ceiling-bound, the
     four largest giants: base_build 0x004F81A0 (its has_fac_built/range
     calls from 0x4FC6xx on are this stop, in the image too), battle_fight
     0x00506AF0 (stop between source lines 3763 and 3838 with 24 probes),
     order_veh 0x005947C0 (after the vehicle-count return since its two
     probes were retired) and probe 0x0059F120. No exact function is
     ceiling-bound (Win::on_l_button_down's `cc` is a no-inline unit).

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
- **Image refuses a run of late pass-1 sites that ours expands: put the
  dead spend right before the image's FIRST refused pass-1 site** (high in
  probe 0x0059F120, 6ae5d5819b, 2026-11-20 par-4). A ceiling-bound or
  budget-tight giant whose image calls a run of later first-level sites
  (probe: parse_set, get_name, BasePop::item, BasePop::exec in the
  accomplice loop 0x005A302B..0x005A30A4) while ours expands them takes a
  charged dead probe (three branchy one-line ifs on a constant argument,
  folded to no code) placed immediately before the first site the image
  refuses: all four flip together and nothing earlier moves (named calls
  59 -> 52). Placed earlier it refuses sites the image expands (the FRAME
  popup's start and first item); placed later, between the run and the
  next refusals (six NetMsg::pop), it moves nothing. So sweep position
  first (the first refused site, from inlinemap/xjcalls) and count second.
  Then **re-check every existing stand-in of the function**: the new probe
  supplies spend an older stand-in was tuned to supply. probe's
  amphibious-test is_human stand-in measured identical to the raw
  HUMAN_BIT with the probe in place, every rung and row, and was retired.
  Second case (2026-11-20 par-4, base_build 0x004F81A0, ceiling-bound):
  the image calls range from the military arm's l_18 line (0x004FA280)
  and has_fac_built from 0x004FA9E0 on, ours expanded four of those sites.
  Four probe calls right before the l_18 line give the image's call
  multiset (distance 5 -> 0, rung 2 -> 4, rows 4099 -> 3641); 1-3 leave
  sites expanded, 5-8 refuse one too many; the same size as pointer-local
  pads at the top or right before the line moves nothing, so it is the
  expansions' charge at that point, not caller size. Re-check: the six
  size-pad pointer locals tuned earlier were all dead and were retired.
  Screens where it did NOT apply (the refused sites were not budget):
  spot_all's on_map (depth-3 sites; probes before the first refused
  spot_base site refuse veh_top instead).
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
- **In a giant, caller size takes room away** (high; battle_fight
  0x00506AF0, 2026-11-19 par-2). Rule 2's "B grows with the caller" stops
  at the top: in battle_fight every added statement *removes* expansions,
  wherever it is placed (top or last block) and whether it is live (`g = K;`)
  or dead (`if (0) { g = K; }`, which counts about twice a store). 1-12
  stores move nothing, 16 refuse is_ocean/base_who/range/veh_top sites,
  32 dead blocks change a dozen callees. Deleting about 20 call-free
  statements closes the image's last pass-1 sites (map_loc, veh_health x2,
  veh_top at 0x50AAE0..0x50AB9B). So the original giant was *smaller* (or
  its statements cheaper) than ours: never look for compiled-out debug
  code or `if (0)` blocks to explain missing room in a giant; look for
  statements our source has and the original did not, or that the
  original spelled with fewer nodes. Writing repeated chassis/triad reads
  as the veh.h getters (is_missile, get_triad) charges more than it saves.
  Follow-up (2026-11-19 par-2): fewer nodes alone cannot reach the image's
  decision vector in battle_fight. Call multiset now: ours calls map_loc,
  veh_health x2 and veh_top at 0x50AAE0..0x50AB9B, and the range call
  sits at our 0x251C instead of the image's 0x50AB85. Two node-only
  screens, both codegen-neutral: (a) dropping the braces of up to 44
  single-statement ifs (0.5 store each), (b) spelling the battle_fight
  uses of `GameDataStateStorage.fields_.*`, `NetDaemonNet.action_mode_` and
  `ConsoleGlobal.faction_id_` as plain globals (a compile-only screen).
  (b) expands every late pass-1 site as the image does. But then range is
  called at three sites in the combat rounds (our 0x251D/0x25FB/0x2659)
  that the image expands: the new late first-level expansions reserve
  pass-2 room. Adding up to 35 brace strips on top fixes one of them. At
  36 the pass-1 tail breaks: has_fac and is_ocean at 0x50ACB1.. expand,
  and the image calls them. The image therefore has more pass-2 room for
  range relative to its pass-1 tail than any uniform shrink of ours. The
  remaining difference is how the round-region veh_health sites are
  owned or spelled (or what range costs inside veh_health), not the size
  of the caller. Rewriting the round loop as
  `while (veh_health(veh_id) && veh_health(def_id))` with merged
  `continue` guards has no effect on the calls. `ownerhunt.py` over lines
  3478-3830 (the outcome display through the base consequences, 400
  ranges up to 25 statements) found no single owner that lowers the call
  distance below 3. The best ones are one-statement owners such as
  `clear()` (rows 2038 -> 2030), and none moves the round-region range
  site. Next: owners of the round-loop and pre-round veh_health sites
  (lines 3296-3336), and the spelling of veh_health/range themselves.
  Round 14 (2026-11-19 par-2). The image's veh_health vector in
  battle_fight: 29 sites, every one expanded; the inner (reactor) range is
  called at the first 8 and expanded from the rounds' boom (0x509022) on, a
  suffix as 2.8 rule 4 predicts for a free reactor_power over a charged
  range; the last site 0x50AB1E expands veh_health and the reactor range but
  calls the outer range (pass-2 refusal, deep expansion; draw_active
  0x4B5A39 shows the same split). Copy-exact alternative bodies
  (named hp/power local in veh_health, named result in reactor_power,
  guard-clause proto_power, dmg-first operand order, proto-id reactor_power,
  `__forceinline` levels) are all equal or worse there and in every other
  caller; the current three-level body stays. The lever was an invented
  level elsewhere: get_triad/is_missile/get_offense_rating/get_defense_rating
  forwarded to their proto twins, an extra /Ob2 level at 19+4 battle_fight
  sites. Written on the fields (landed, veh.h) the call distance falls
  5 -> 3 and only battle_fight and probe move tree-wide. Open-coding the
  triad reads in battle_fight instead gives too much deep room (inner range
  called at 4-6 sites) and too little pass-1 room. Screens on top (not
  source): without the nerve-gas owner, plain-global spelling and 16 brace
  strips, the vector is the image's except the 0x50AB1E veh_health (called);
  the nerve-gas owner's level-2 spend is the deep room the rounds lack. So
  the remaining gap is caller size against the owner's nested spend, not the
  veh_health/range spelling.
  Round 15 (2026-11-19 par-2): the giant's tail is a hard stop, not room.
  From some source point on, pass 1 refuses *everything*, including a free
  `x = x;` probe: in battle_fight a free probe expands before the stack-damage
  `veh_health(v)` (0x50A9E0) and is called right after it, so 0x50AB1E's
  veh_top/veh_health are refused there whatever the owner's spelling (by-value
  or const-reference parameters, the state local inside the owner, has_treaty
  written raw, is_human: all identical). Deleting the 11-statement GOODIDEA
  block at 0x507E7E moves the stop past 0x50AB1E (both expand, frame 0x138 as
  the image); deleting any one of its statements does not. order_veh shows the
  same stop right after its vehicle-count return teardown. Screen for it:
  one free probe before and after the refused site; if both are called, look
  for statements before the stop that the original did not have or spelled
  smaller, not for spend.
  Round 28 (2026-11-20 par-2): the nerve-gas owner's parameters by value
  (att_faction or def_faction by value is enough; all 32 ref/value
  combinations screened) make the rounds' boom site (0x509013) expand the
  reactor range as the image does: named call multiset equal (range
  12/12), rung 2 -> rung 3 (frame 0x134 against 0x138), rows 2349 -> 2414.
  A by-reference owner parameter is not free spend: it changes how much
  nested room the owner leaves for later sites, so screen an invented
  owner's parameter passing before its body.
  order_veh 0x005947C0 (same round): its four early Popup returns expanded
  ~FlatButton where the image calls it. 20 raw treaty tests, 12 raw human-mask
  tests and 11 raw triad reads spelled as has_treaty/is_human/get_triad (free
  inlines after those returns, rule 4 reservations) give the image's EH map,
  frame and call multiset except one ??_DStringList at the vehicle-count return.
  The `triad = ...` initialiser stays raw: through get_triad it adds a frame slot.

Closing a teardown function with code-identical spellings (2026-11-19 par-4;
PickTech::pick 0x00486510 and popb 0x0048C650, both recorded). Once the EH map
and the call multiset are right except for one or two late sites, the last
moves are spellings that leave the code alone and change only the caller's
size or the order of its decisions:
- **Exact-code size from the 2.1 cost table.** `if (noted != 0)` compiles to
  the same bytes as `if (noted)` but costs about 0.5 store unit more (2.1).
  Eleven note tests in pick gave back about 5.5 units of pass-2 room that the
  eax-first table_text/label_get helpers had taken, and the last ~FlatButton
  site expanded again as in the image. Use such spellings (`!= 0`, braces,
  `x = x + 1`) to tune caller size by fractions of a unit without a probe.
- **A field read back through a const local gives a homed value with a
  cache.** `row_count_ = items / columns + (...); const int rows = row_count_;`
  keeps rows in a home slot with a cached register copy, as the image does
  ([ebp+0x14]). Writing the expression into the local first
  (`const int rows = ...; row_count_ = rows;`) lets VC6 copy-propagate the
  local into a register with no home.
- **A named local decides argument evaluation order.** VC6 evaluates call
  arguments right to left; `const int width = (cell_width + 1) * columns + 1;`
  before `GraphicWin::init(0, 0, width, ...)` computes the width first (in
  ecx, evicting the rows cache) and the height afterwards from the rows home,
  as in the image.
- **Structure that removes a phantom closing-brace teardown.** popb's
  `if (input_poll_state) { ... } else { result chain }` added the
  closing-brace teardown (2.2) after the else arm: 16 extra EH states. With
  the test returning 0 in its arm and the result chain at top level ending
  in `return result`, the unwind map has the image's two post-exec teardowns.
  Before tuning spend in a function whose EH distance is a block of states
  after the last statement, rewrite an if/else whose one arm returns as a
  guard clause.
- popb then needed about 20 if-units of pass-1 charge after the early
  returns (a marked dead probe, `popb_budget_probe`); the original spend
  there is not identified. Owner screens of the focus block and the staple
  items give EH 0 but leave ??_DDialogs called.
  Round 16 (par-2): the remaining ??_DStringList was the ceiling (rule 7), not
  room. Two of the four supply_options_budget_probe calls retired: EH, named
  call multiset and frame all equal to the image (three or none leave the final
  teardown 16 deep). The 10 sites accspell.py still finds (8 offense ratings,
  1 triad, 1 human) all keep or worsen that vector.
- **Accessor spelling screen, tree-wide** (round 16 par-2;
  `analysis/accspell.py`). `list` names, for every rung-1/2 function of a
  rungs.json, each raw test that has a header accessor (has_treaty,
  has_agenda, is_human/is_alive including a local `b = 1 << f` bit,
  get_triad, get_proto_triad, is_missile, get_offense/defense_rating,
  temp_at, altitude_at, owner_at, code_at, is_ocean) with the return it
  follows; `deck <unit> <addr>` measures BASE, ALL, ALL:<family>,
  AFTER_R<k>, ONLY_R<k> and FROM_S<k> (every site from the k-th on) as
  (EH distance, named call distance, inlinemap site distance, frame, rows).
  It adds `#include "faction_inline.h"` where a variant needs has_treaty
  (faction.h declares the out-of-line copy). Results on the rung-1/2 list
  not held by par-4: none of farm_click, battle_report, CouncWin::init,
  draw_score, use_ability, editor_edit_veh, menu_proc, propose_attack,
  communicate, call_council or spot_all moves toward the image (the
  console_commands and call_council variants break their Popup teardowns;
  spot_all's 21 treaty sites change on_map/veh_top both ways). tech_trade,
  propose_pact and Net::session_picker_dialog have no raw site left (faction.cpp
  is already accessor-spelled; the picker has no field tests), so no accessor
  spelling can replace their probes. battle_fight's 34 sites only add calls
  (it is ceiling-bound, rule 7).
- **Raw against accessor in the exact functions** (round 16 census,
  `accspell.py census`, 5199 fe-exact functions). Counts of raw / accessor
  spellings of the same test: treaty 325/459, agenda 16/50, human 48/82,
  alive 24/35, triad 65/36, proto triad 42/28, offense 18/12, missile 2/7,
  is_ocean 22/142. Per unit the split is a recovery-era convention more than
  evidence: faction.cpp is the only unit where most treaty/agenda tests go
  through the accessor (414 against 173 raw), council.cpp (27/0),
  console_commands.cpp (21/0), terraforming_actions.cpp (13/0) and the
  window units write them raw; the human test is accessor-spelled in
  technology.cpp (13/0), base.cpp (13/6), console.cpp (7/0), raw in
  basewin.cpp (6/0); triad reads are raw in console_commands.cpp (23/2),
  mapwin.cpp (14/0), console.cpp (10/0), main.cpp (7/0), accessor in
  battle.cpp (8/0), veh_stack.cpp (6/0), enemy.cpp (8/2). Exact functions
  that mix both spellings of one family are rare: two in faction.cpp, one
  in setup.cpp (human), and ten mapwin.cpp functions mix raw and is_ocean
  (20/66). Because a free accessor
  expands to exactly the raw code, an exact function constrains the spelling
  only when it has inline decisions after the site (teardowns, charged
  inlines); so read a unit's split as the spelling to try first in its
  non-exact functions, not as proof.
- **When to stop.** Dead probes that change other teardowns before they move
  the target mean that pass 1 is the binding limit, not pass 2. Look for a
  structural difference: which object owns the teardown, or a per-TU header
  guard (2.3).
- **Teardown decisions move with sibling-evidenced spellings of earlier
  statements, not with object order or scope** (2026-11-20 par-1;
  BaseWin::base_editor_proj 0x0041BBC0, commit d696e5d5f7). Object order,
  declaration scope, probe counts and owner screens left base_editor_proj's
  unwind map 2 states off (37 against 35). What moved it was copying two
  spellings from its exact sibling base_editor_copy 0x0041C240, which builds
  the same Popup: the PICKAPLACE labels read through `label_get(...)`
  instead of a written-out `StringTable.get(Labels.strings_ptr[...])`, and
  the secret-project owner stored straight into the raw array
  (`secret_project_.project_base[project] = owner_base`) instead of through
  the header's `operator[]` inline. EH distance 2 -> 0, rows 47 -> 29, and no
  exact function in the unit lost. Neither statement is a teardown, but each
  changes pass-1 size and the inline decisions in front of the teardowns, so
  the front end numbers the later states differently.
  Workflow: for a teardown function, list the exact functions that build the
  same objects (Popup/BasePop/Dialogs/StringList; the same `item`, `label`,
  `start` and `exec` calls), usually in the same unit or a sibling command
  (base_editor_*, the other NetWin or Console dialogs). Diff their spelling of
  each statement kind the target shares with them: label through `label_get`
  or written out, `popup.item` or `popup.dialogs().item`, accessor or raw
  field, `exec()` or `exec(0, 0)`, array store or `operator[]`. Screen each
  difference alone and all together with `analysis/sdeck.py` and keep moves
  toward the image on the rung ladder. An exact sibling is evidence of the
  original header's spelling. A probe count is not.
  Classify a teardown residue before you pick a lever (`inlinemap.py`
  against the image):
  - **A, late site expanded where the image calls it** (our pass-2 room is
    larger at the close): call_council 0x0052C880 (the last ~FlatButton),
    Net::session_picker_dialog sub_631050 (two ??_DAlphaNetLedger sites),
    control_turn 0x005275B0 (~RadioButton), base_editor_proj (??_DDialogs,
    ??_DStringList). The image spent more in front: look for a cheaper
    spelling here that is more expensive in the sibling (an inline the
    sibling expands, `label_get`, an accessor).
  - **B, late site called where the image expands it** (our room is
    smaller): MapWin::click 0x0046D5D0 (~FlatButton site 42). The opposite
    move: the sibling's cheaper spelling.
  - **C, the unwind map differs** (EH distance > 0, a scope or teardown
    site missing or extra): NetWin::pick_player 0x0047BBF0 (??_DStringList
    sites 3-4), order_veh 0x005947C0 (one state), and base_editor_proj
    before d696e5d5f7. Fix rung 1 first. As base_editor_proj shows, the
    cause can be a non-teardown statement, so screen the sibling spellings
    before any restructuring.
  - **D, teardowns equal but other callees or layout differ**:
    propose_attack 0x00545E80 (remove_all, indirect calls) and tech_trade
    0x00541100 (the cross-jump host of its ~BasePop exits, a layout
    question; see 3.7). A teardown lever does not apply.
  Round results (2026-11-20 par-1, sdeck):
  - base_editor_proj: the PICKAPLACE `label_get` results passed straight to
    `popup.item(label_get(..), id)`, not through a block-scoped local:
    EH stays 0, call distance 10 -> 8, rows 29 -> 25. The base_editor_copy
    noun spelling (parse_set + noun_faction) costs 1-3 EH states at every
    probe count.
  - pick_player (C): the two extra states are at the closing teardown
    (ours 36,34,33,31,30 against 36,31,30), so one ~StringList level
    expands there where the image calls it. label_get / raw / argument /
    `!_strnicmp` spellings leave EH at 2. Charged one-site probes, placed
    anywhere, add expansions (B grows with caller size), so no probe is
    the lever either.
  - call_council (A, one extra state at the close): the CouncWin GOVVOTE
    picture block (`int picture_size[2]`, `== 0`), `popup.item`,
    `exec()`, the short `start` overload and `table_text` all cost 9-12
    EH states. `popup.item` against `popup.dialogs().item` compiles the
    same. spellsweep's is_human -> raw costs +9 calls at any single site.
  - tech_trade (D): the image keeps its four `ret`s with every return-0
    copy merged into the reject_later tail. Ours has six: the 0x21, 0x25
    and 0x2f exits keep their own epilogue (`lea; mov [ebp-4],esi; call
    ~BasePop; ret`) and REJTECHSECOND hosts. Hence ??1BasePop 2/5. The
    reject_later block cannot move in source. After `goto again` under
    its label it costs 25 EH states, because the front end numbers its
    states 0x35-0x42, between the exits at 0x30 and 0x44, as written.
    `goto speak; reject_later: ...; speak:`, `if (0) { reject_later: ... }`,
    one `&&` condition, and `do/while` or `while (1)` for the `again`
    loop all compile byte-identically to the current source. An `else {}`
    or `{}` around the rest, or `for (;;)` with the return inside, adds 16
    phantom states (2.2). So the host comes from block order after the
    front end, not from where the block is written. Still open.

What the model does not predict (2026-11-19)
- **MapWin::click 0x0046D5D0** (solved 2026-11-19 par-6). The image calls
  `::draw_map` in the first editor arm (0x46D977) and expands it in the
  second. The block is MapWin::UNK4 (0x0046C020, no callers in the image,
  same instructions): `UNK4(tile_x, tile_y); return;` expands UNK4 at pass 1
  and decides its two draw_map sites at pass 2, first called, second
  expanded. Screen for this elsewhere: a plain same-unit function with no
  image callers whose body equals a block another function writes out is
  an auto-inlined owner (2.8 "Missing owner").
- **Orphan owners** (2026-11-19 par-6, `analysis/orphanowner.py`). The
  click/UNK4 screen run tree-wide: 667 markers of >= 8 instructions have no
  image reference of any kind. Matched against their units' image bodies,
  the clean hits were real owners: DesignWin::on_iface_right_click calls
  main_interface_bubble_callback_4396b0 (2 rows -> exact, the `mov ecx`
  placement the written-out copy missed), DiploWin::exec calls sub_441040
  (279 -> 227, block layout fixed), battle_fight's dead-unit sweep is
  battle_kill_chaff (2409 -> 2346 under a marked __forceinline: a plain call
  is refused by the giant's ceiling), base_terraform's arid tests are crappy
  (an invented static twin, 893 -> 873, but the borehole/condenser add_goal
  sites stop cross-jumping: img 4 calls, ours 6), StatusWin::redraw calls
  UNK1 and is then auto-inlined at on_right_click's three sites without the
  `inline`/MEASURED stand-ins. Eight more (set_cursor/sub_510c90,
  cursor_key/base_me, toggle_patrol twice, tech_heck/sub_44b340 replacing
  the emit_help twin, clear_terrain/UNK7, init_sound/unload_sound_dll,
  load_vehicles/sub_450e80, monolith/veh_site_scan, DiploPop::start/
  portrait_anim) expand to the same bytes. proposal_menu 0x0054DCF0 is the
  counter-case: its two PlanWindow blocks are sub_54dc90 (0x0054DC90) in
  the image, but calling it moves ~Scroll/close/~FlatButton/~GraphicWin
  decisions (named call distance 0 -> 5, 86 -> 102 rows), so the written-out
  copies are tuned around another missing charge in that function. Found
  (r24): the ten has_treaty spellings; written raw, the sub_54dc90 calls give
  the image's EH map and call multiset (landed). Read `show` before editing: below score 0.9 shared parse_says/pop
  runs and show_all/hide_all pairs score as hits.
- **Referenced owners too: an invented helper is often a real header inline**
  (high; 2026-11-19 par-4). orphanowner only tries unreferenced markers, so it
  misses header inlines that also have an out-of-line copy. Reverse the scan
  (every marker's image body against ONE host's image body, min 3 tokens) and
  compare each invented `static inline` helper's body with what comes back.
  Console::go_to 0x004D61A0: its go_to_refuse helper was netmsg.h's
  veh_battle_msg (0x0050B9C0, `pop(label, 5000, 0, icon)`); with all four
  refusals through it and the waypoint/sort-button helpers written out, EH,
  named calls, jcc/jmp/call/ret counts and the closing teardown's
  remove_all call/expansion order all equal the image's (181 -> 118 rows),
  where every invented-helper combination left EH 3-6. The invented helpers
  were tuned to imitate the real inline's charge with a different
  parameter list. BaseWin's compute_selected_base was the real focus()
  0x00415DD0 (expanded at 24 sites; spell the real body with the twin's node
  count: `!= 0` tests charged at every expansion moved production/hurry).
- **Orphan refused by the per-callee cap: shrink its own body with an inner
  inline** (2026-11-20 par-6 r24). A plain function with no image references
  that the image expands, but that ours calls even from a one-statement
  caller (`capprobe`: append `void p() { f(); g = 1; }` to the unit), is over
  the auto-inline cap (2.1), not out of budget. The cap counts only the
  callee's own body: an inline call inside it counts as one call whatever
  its body (synthetic g{7 ifs}/f{g(); 7 ifs} expands f; f{14 ifs} is
  called). So the original orphan held part of its body in an inner inline
  helper. Found: net_not_my_turn (skip test as a helper; net_control_turn
  exact), prev_cursor (survey clear as a helper, cursor block as
  reset_turn_view(); retires two WORKAROUND copies), sub_5a6510 (a terser
  spelling alone suffices; compass_move exact), set_parent_dialog (the
  WinNodeList ring append as a helper passes the cap; Win::init's hand copy
  becomes `if (iFlags_ & 0x100000) set_parent_dialog(parent);`, same 48
  rows). rscan finds no real owner for any of these helpers. Definition order does not matter.
  Mark invented helpers WORKAROUND(inline). Note: GameDataStateStorage.fields_
  member chains count more nodes than plain globals; spelling every one of
  them as a plain extern also puts net_not_my_turn under the cap, but the
  tree-wide screen (66 files, 3058 uses) loses 28 exact functions and gains
  1, so it is not landed.
  Tree-wide census (2026-11-20 par-4, `helperscan.py`, 76 invented helpers,
  thr 0.85): no other invented helper of a non-exact host has a real marker
  playing its role; the remaining ones are twins of known functions kept for
  an inline/out-of-line split in one TU (stringbox/picker/popmenu next/
  current entries, net_next_stack_id, xrange_inline) or have no image copy.
  The scan's real yield was a written-out header inline instead: say_num
  (0x0050B8A0, `_itoa(v, buf, 10); strcat(StringTemp, buf)`) in ten hosts
  (StatusWin::draw_active calls 1->0, cf 5->0, 161->60; draw_unit 104->41;
  net_upkeep 65->31 with only its first site). Screen per site: in
  draw_commerce, production, upgrade, draw_tech and QuayleWin every subset
  is worse, so those runs stay written out. get_adjective/get_title/get_name/
  get_noun spellings of written-out parse pairs were neutral in all 34 hosts.
  Console::terraform's four invented helpers against written-out bodies (all
  16 combinations): with all four EH 3 / named calls 9 / 119 rows, all
  written out EH 27 / calls 34 / 347; every single write-out is worse, so the
  owners stay until a real spelling with the same charge is found.
- **Popup::start, five arguments (0x0043EF70), editor_who copies**
  (2026-11-19 par-6, screen only). With editor_diplomacy calling the plain
  `editor_who` (auto-inlined, as in the image) instead of the
  editor_who_inline stand-in, a dead charge of 12 one-line ifs in the
  five-argument body makes editor_diplomacy fe-exact (0: 58 rows, 8: 51,
  10: 62, 12 and 16: 0): the image's refused nested start is a charged
  inline refused at pass 2, and the room it frees deepens the later
  teardowns as the image shows. Not landable: no natural forwarder spelling
  is that dear (`this->`, a `(GraphicWin *)0` cast and a named null owner
  local are free or change codegen), popp 0x0048C0A0 refuses the charged
  start from 10 ifs on (the image expands it there) and standalone
  editor_who loses a teardown expansion. select_special and diplomacy_menu
  call it at first-level sites that no charge up to 32 ifs refuses, so
  those two sites still need an owner or another declaration.
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
- **Console::terraform 0x004D3470** (2026-11-19, par-1). Dead 8-64-if
  probes at the top or at five later positions move ~Scroll by at most 3
  sites (spend leaves ~FlatButton first), yet turning two written-out
  `StringTemp[0] = 0; say_tech; parse_says` runs and the two payment
  refunds into inline helpers refuses ~Scroll at two early sites and makes
  the old 16-if probe inert. Helpers holding real calls act on level 2
  where dead ifs do not; screen hand-written repeated statement runs as
  helpers before reaching for probes. Still E17c13 against E13c17.
- **Same suffix shortfall elsewhere.** go_to 0x004D61A0 (image c,c,c
  with sites 2/3 cross-jumped into one ~Scroll call, then E,E; ours E×5),
  PickTech::pick (image refuses the four first-return sites, ours none)
  and terraform all err the same way: the image refuses ~Scroll at the
  earliest teardowns. queue_click and pick_pass instead swap which of two
  early sites expands (image E,c / c,E against ours c,E / E,c); pick_pass
  also keeps `payload` in memory where ours enregisters it in edx and
  gives its slot [ebp+8] to faction_mode (image: to verify_hash).
  (Solved for go_to and pick by late free helpers, below; terraform open.)
- **Helper screen: real runs retire probes, but not the suffix**
  (2026-11-19, par-1; `analysis/runscan.py --try`). Each statement run
  repeated inside a function, as a template inline helper with and without
  the function's dead probe lines, over 83 probe-carrying functions:
  - Whole retirements: base_editor_copy 0x0041C240 (label/item run, five
    probes), introduce 0x0053D360 (leader and noun pairs, eight probes),
    propose_attack 0x00545E80 (friction/X_pop run, one probe), go_to
    0x004D61A0 (refusal and waypoint runs, 20-if probe). Partial:
    size_of_planet 0x0058CED0 (six of sixteen probes, `--keep` sweeps the
    probe count beside each helper). Most other probes are only partly
    replaced (8-64% of their rows).
  - **A helper's parameter form changes its cost.** In introduce and
    base_editor_copy the natural form (int by value, label index) falls
    short (119 and 42 rows); `const int &` parameters, or a byte offset
    divided inside the helper, reproduce the image. Each reference or
    expression inside the helper adds charge, so try those forms before
    concluding a run was not a helper. Mark the form WORKAROUND(inline).
  - No helper the screen finds refuses ~Scroll at the earliest teardowns of
    go_to or PickTech::pick (pick: seven windows, also the score/best,
    say_tech, box and modal runs; all leave ~Scroll E at every site). The
    suffix shortfall above stays open. (Superseded: the existing header
    helpers, not new runs, close it; see "late free helpers" below.)
  - A run can be a helper at one site and not another: the label/item
    helper breaks DiploWin::on_button_clicked 0x004415C0 (refuses its
    ~RadioButton), so keep such helpers file-local to the sites that gain.
- **Early refusals with late third-level expansions: late free helpers**
  (high; PickTech::pick 0x00486510 and Console::go_to 0x004D61A0,
  2026-11-19 par-4). When the image refuses a level-2 callee (~Scroll) at
  the *earliest* teardowns yet keeps level-3 expansions (~FlatButton) at
  later ones, caller size cannot supply both: more statements raise B and
  add level-3 room, dead spend leaves level 3 first (rule 5). What the
  original had is reservation (rule 4): more first-level FREE inlines
  *after* the refused sites. Our source writes those expansions raw. The
  usual ones are the header helpers that already exist: strings.h `says`,
  `clear`, `say_num`, alpha.h `say_label`/`table_text`/`label_get`,
  faction_inline.h `has_treaty`, map.h accessors, rect_width/height. A free
  helper expands to exactly the raw code, so the image cannot refute the
  spelling at the site; the evidence is the decisions at the refused sites
  (and a matching frame, e.g. say_num's `char[0x50]` is the 80-byte local
  buffer go_to wrote out). Single sites rarely move anything: screen the
  whole family per function (`analysis/spellsweep.py <unit> <addr> --rules
  says_fn,sayl_fn,clear_fn,table_fn,human_fn` prints `ALL:<rule>` rows),
  then greedy on top. Results: pick ten `say_label` + `says("+")` + one
  table_text: EH max 198 -> 191 (image 192), calls 4 -> 3 differences,
  rows 653 -> 395, and the popup frame slots back in image order. go_to
  `says`/`clear`/`say_label`/`table_text` then two `has_treaty`: ~Scroll
  called at the first three returns, EH 43 -> 37 (= image), named call
  multiset equal, frame equal. Its "site 2 has no ~Scroll" was only
  cross-jumping: once sites 2 and 3 both call ~Scroll, their identical
  tails merge (jmp 0x4D64CE) with no special source shape. No effect in
  terraform, config_game, queue_click, pick_player, base_editor_proj,
  popb or save_game (all single-site and ALL rows equal or worse).
- **A helper with a literal inside its body is refuted by per-site
  literals** (high; 2026-11-19 par-4). An argument-free inline such as
  `say_plus() { strcat(StringTemp, "+"); }` emits ONE literal
  (`??_C@_01PJOB@?$CL?$AA@`, a COMDAT shared by every expansion and every
  unit), while a literal argument (`says("+")`) or a raw strcat stays a
  `$SG` of the caller per site. fe masks relocations and cannot tell them
  apart; check `tumap.literal_owners` (or the image's pushes): pick pushes
  eleven distinct "+" (0x686A60..0x686A88, plus an unreferenced twelfth
  for the constant-false first joiner), and every "+" in the image is
  per-site and per-unit. So no "+" helper belongs in strings.h; the
  shared-literal family (`" "`, `", "`, `": "`, `"("`, `")"`, `"%"`,
  `". "`) is exactly the texts the image shares across units.
- **A pushed literal's address decides inline against written out** (high;
  2026-11-20 par-4, `analysis/litaddr.py`). The image is unpooled, so the
  address a site pushes tells which source form emitted it: a raw
  `strcat(x, ", ")`, or a literal passed as an argument, is a `$SG` of the
  calling unit, laid out in its literal run in source order; a text written
  inside an inline function's body is a `??_C@` COMDAT, one address for
  every unit (the separators at 0x00682820, 0x00682E90-0x00682EA0 and
  0x006867E0). So a site that pushes the shared address MUST be the header
  inline (`say_X()` / `strcat_X(dest)` / `space()`), and a site that pushes
  a unit-local copy of the same text MUST be written out, whatever rows,
  call distance or a probe screen says. fe masks relocations and cannot see
  this; compare the push sequence, in code order, with `litaddr.py
  FEALL.json` (20 s tree-wide). It is evidence, not a screen: apply it and
  keep it when rows rise. 2026-11-20 tree pass after the separator rounds:
  ReportIf::draw_sat's first paragraph `" "` pushes a draw_sat-local copy
  (0x4AAB18), so it is `strcat(paragraph, " ")` (6cad1c0b04's rows-based
  `strcat_space` reverted, 54 -> 55 rows). The same test reaches beyond the
  separators: AlphaNetStringStruct::add pushes the shared out-of-heap texts
  (0x00682700/0x00682724, referenced by three units) at all four sites, so
  AlphaNetLedgerEntry's placement `operator new` pair is `inline` (ours were
  out-of-line definitions whose expansions kept `$SG` copies); battle_fight's
  `"biohazd_sm.pcx"`/`"USENERVE"` sit in its own literal run, so the
  WORKAROUND(decl) inline owner takes them as arguments rather than writing
  them in its body. Named stand-in arrays (`BufferJoinSpace`,
  `SocialWin*Text`, shown as `N`) at a shared push are header-inline sites
  too; `--shared` also lists other texts referenced from two units, where
  a data read that happens to point at a string (`mov eax, [0x0068FB58]`)
  is the usual false hit.
- **A shared-COMDAT separator that costs what the raw literal costs is a
  text-RETURNING inline in the caller's own strcat** (high in one real
  function; 2026-11-20 par-4, DesignWin::select_special 0x0043AC70,
  f6fcf7075b). The rule above says a push of the shared address is an
  inline; it does not say which inline. select_special pushes the shared
  " " ": " "(" ")" at all 28 separator sites, yet every destination-taking
  form (`strcat_X(StringTemp)` from strings.h, a unit-local copy, a
  returning-strcat variant, the no-argument `say_X()`) overspends its
  budget: three ~StringList teardowns stop expanding (sdeck EH 3, named
  calls 11, tokens 3, hunks 2, 63 rows). `strcat(StringTemp, f())` with
  `inline const char *f() { return ": "; }` (free function or static
  member) gives (0, 2, 0, 0, 5): the raw-literal structure with the shared
  address, only the unrelated five-argument Popup::start site left. So
  when a host's separator change to `strcat_X`/`say_X` moves EH or
  teardown calls away from the image, try the text-returning spelling
  before concluding the site is written out. The original helper names are
  unknown: the image holds NO out-of-line literal-returning copy
  (`mov eax, offset; ret` after padding occurs 12 times in .text, none
  returning a string; 2026-11-20 scan), which neither confirms nor rules
  out such header helpers (only space() of the strcat forms has a copy
  either). Mark them WORKAROUND(inline).
  Other hosts (2026-11-20 par-4): editor_edit_veh takes the raw structure
  with it (1,7,1,4,147 -> 0,4,0,3,122); upgrade gets (9,13,5,6,473) and
  then the raw structure once its tuned reservation probes are re-swept
  (10 -> 1-7 probes, (0,0,0,5,302)); in draw_active it costs exactly what
  say_X costs (every subset), and the cost is a suffix effect: making the
  last 11 sites raw restores the image while any single site or the first
  23 do not. There the separators were right and the stand-in that was
  tuned against the raw form was wrong (status_damage_percent level ->
  status_damage_taken, 418 -> 60). So after a separator change, re-sweep
  the host's existing budget stand-ins before blaming the separators.
  select_special's five-argument Popup::start (0x0043AED5): ours now
  expands it with no structural cost; refusing it (member pointer or any
  owner) frees room that deepens a ~FlatButton or loses a ~StringList
  teardown (EH 1-2), so the image refused it AND spent its cost
  elsewhere. Left expanded (call distance 2, 5 rows).
- **`__forceinline` spends the same budget as `inline` in a real caller**
  (2026-11-20 par-4, same deck). Rule 3's "`__forceinline` sites expand and
  charge nothing" holds for the synthetic probes only: in select_special,
  `__forceinline` strcat_X copies give exactly the plain inline's
  (3, 11, 3, 2, 63) and `__forceinline` text-returning copies exactly the
  plain (0, 2, 0, 0, 5). Together with `label_get` (identical over all
  5,574 markers) and the 2026-11-19 census (4.1), do not use the keyword
  to save budget; COMPILER_EVIDENCE's "does not spend the /Ob2 budget"
  (and the AGENTS.md summary of it) is not borne out in real functions.
- **Frame-slot order follows the expansion counts: judge a spelling by its
  decisions, not its rows** (high; 2026-11-19 par-4, PickTech::pick
  64a5fb2ca3). Every expanded destructor or helper brings its own locals and
  temporaries, and VC6 numbers frame slots by first appearance in the
  expanded tree, so one more or one fewer expansion at an early site shifts
  every later slot. fe aligns those shifted `[ebp-N]` operands as differing
  rows: a spelling one decision closer to the image can read dozens of rows
  WORSE, and a spelling one decision further can read better because its
  wrong slots happen to line up. In pick, the deck whose ~FlatButton E/c
  string equalled the image's ranked ~40th by rows; once committed, the
  popup slots fell into image order by themselves and rows went 395 -> 146.
  Rank decks by (EH states, named call multiset, inlinemap E/c distance,
  frame size, control-flow tokens) and only then rows
  (`analysis/sdeck.py`, `analysis/jseq.py`); never read a row loss on a
  decision gain as a regression, and never compare slot maps
  (`framescan.py --slots`) until the decision strings agree.
- **Two decisions that flip at one B threshold are not a budget problem**
  (2026-11-19 par-4, Console::go_to 0x004D61A0). go_to's closing teardown
  expands ~StringList's remove_all and calls the base ~StringStruct's (the
  image the reverse, call 0x402970 at 0x4D6897); call multiset and EH are
  equal, so only the site's order shows it. Any spend (veh_moves(subject)
  at L2452, get_triad(subject) at L2271, or 4 dead stores anywhere) gives
  the image's closing shape AND refuses ~Scroll at the action_home return
  (L2307) where the image expands it; 5 raw stores anywhere undo both at
  once, at every position tried. Neither spend nor its position separates
  them, so look for a reservation or a statement layout change, not for
  more or less budget. Screen: `sdeck.py` with a per-site flag
  (state store followed by a call).
- **A suffix shortfall can need two owners: each restores half** (draft,
  medium; Console::terraform 0x004D3470, 2026-11-19 par-4, caf3479a1a).
  The image called ~Scroll 17 times then expanded 13; ours 14/16 (three
  ~Scroll costs of pass-2 room too many). `ownerhunt` found two inline
  owners (the closing "issue the orders" block, the unavailable-tech
  message run), each giving EH distance 5 / call distance 6, one ~Scroll
  site short; together they give the image's ~Scroll and close@Scroll
  vectors. So when a single owner closes about half the shortfall, keep
  it and hunt the second rather than enlarging the first. Calibration in
  that function: 1-if dead probes anywhere are free (inside the free
  threshold, nothing moves); a 2-if probe overshoots (~FlatButton moves
  first); the owner's own charge, set by what it holds (the message owner
  written over an existing say_tech helper is right, the same run
  open-coded overshoots to EH 9), is the only knob fine enough. Caller
  size (stripping `!= 0` tests, accessor spellings) does not move it.
- **Text helpers per site, jointly with the probe count** (high; 2026-11-20
  par-1, BaseWin::base_editor_proj 0x0041BBC0, 4f532784dc). Whole-family
  screens (`spellsweep.py ... ALL:<rule>`) and single-site flips both read
  "no effect" in base_editor_proj, because each helper site changes the
  pass-2 room and the dead budget probes were tuned for the raw spelling.
  Search the sites one by one AND the probe count in the same greedy pass:
  one option per site (raw / `says` / `say_label` / `clear`, and `exec()`
  against `exec(0, 0)`), plus one option "k probes" for k = 0..8. The
  winning state (say_label at the first two owner-status labels, raw
  elsewhere, probes 4 -> 2) gave the image's unwind map and named call
  multiset (25 -> 7 rows); neither the helpers nor the probe change alone
  moves a decision. Tool: a greedy coordinate descent over `sdeck.evaluate`
  (OPTS = {site: [(label, [(old, new)...]), ...]}, first option current);
  include every remaining dead probe of the host as its own "count" site.
- **A ceiling-bound giant's closing teardown that the image calls needs
  caller size, in a narrow window** (high; 2026-11-20 par-1, order_veh
  0x005947C0, 2507df4823). order_veh is past rule 7's size stop; ours
  expanded the closing-brace ??_DStringList where the image calls
  ??1StringList, while an earlier return (vehicle count) expands it in
  both. Probes alone cannot separate the two sites (an expanded probe moves
  the stop for both). What works is a third probe before the vehicle-count
  return (two -> three) plus plain dead size placed *after the last site
  that must expand* and before the final return: 22..24
  `if (0) { order_veh_size_pad = K; }` blocks (marked WORKAROUND(inline);
  they fold to no code but count as caller size). 21 or fewer leave the
  closing site expanded; 25 or more refuse the vehicle-count site too. So
  for a giant whose last teardown is the only wrong decision, sweep a
  size pad at the end of the body in steps of one block; the window is
  a few blocks wide, and the real fix is the original's longer tail
  (the missing statements), which the pad marks.
- **A header change that shortens member paths shrinks every caller; restore
  its decisions with caller statements, not new probes** (high; 2026-11-20
  par-5, the lock-mask batch: call_council 1018b42c1e, order_veh
  ecbf2ffe8f, probe a5da0294c2). Removing one member level at each mask
  test lowers the caller's own size, so B (rule 2) drops and pass 2 refuses
  sites the image expands (call_council's ~Scroll at the SIMULNO return) or
  a ceiling-bound giant's closing teardown flips (order_veh). Before
  re-tuning probes, add plain caller statements: ANY statement counts the
  same wherever it stands (top or end of the body), including `if (0) {}`,
  a dead store or a dead call; splitting a declaration into declaration +
  assignment and empty statements (`;`) count nothing. The window is
  narrow: call_council needs exactly one (two leave three ~FlatButton
  expansions against four), probe 6..9 blocks with its three probes, order_veh
  24..26 with three probes. Mark the blocks WORKAROUND(inline) `<fn>_size_pad`;
  they stand for the original's statements that our recovery lacks.
  The lever is coarse: call_council's rows step 268 (0 statements) -> 243
  (fractional `!= 0` spellings) -> 142 (1 statement, or 0 + four `!= 0`) ->
  200 (2 statements, or 1 + four `!= 0`), with no state between 142 and 200.
  Its last wrong site (the closing ~Scroll copy's second ~FlatButton,
  image 0x0052DA61 calls it; the four preceding teardowns are `Ec` in both)
  did not move under any size or under one to three dead budget probes
  (`static` four-test step/probe helpers) at eight positions from the top to
  the IsMultiplayerNet tail: position never mattered and every probe broke
  earlier ~Scroll sites (par-5 round 41).
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
- **Destructor chain pinned by its own copies** (2026-11-19, par-1). When
  the out-of-line `??1X` copies of a class are byte-exact and their unwind
  maps equal the image's, declaration-level variants of the class
  (`virtual`, qualified `close()`, member order of trivially destructible
  fields, in-class against out-of-class inline definitions) leave every
  inline decision unchanged. Do not search the class further: look at the
  callers (missing inline helpers or wrappers, hand-copied helper bodies,
  teardown-site order, block layout).

- **An inline helper's by-value argument is evaluated once and spilled**
  (par-5 2026-11-19, base_build 0x004F81A0). A helper `f(int tech)` called
  with `Chassis[c].preq_tech` stores the argument to a slot before the first
  use inside the body and reloads that slot for the second use. When the
  image re-reads the source expression at each use (0x4FC395, 0x4FC3CE,
  0x4FC401: the preq_tech field loaded again for the second `has_tech`), the
  original wrote the expression out at each use or used a macro. A
  `const short &` parameter re-reads, but only where every caller's field has
  that type; a helper that restores the call multiset with the wrong reload
  shape is not the original (2858bb1c47 reverted fd3923ed2a).

Header inlines behind dead probes (2026-11-19, par-1; clear/says/space/
say_label/say_tech batch, 86cc2f1fd4..39c52fa497, 89 probe calls retired)
- **A small helper whose out-of-line copy is called only by budget-exhausted
  giants, and whose copy sits next to their code, is a header inline** (high).
  The giants (base_build, battle_fight, communicate, probe, ...) refuse it for
  lack of budget; every other unit expanded it, and the out-of-line copy is
  the one a giant's object contributed. The hand-written body of that helper
  at other call sites is what the dead `*_budget_probe` calls stood in for:
  write the call, make the helper a `MEASURED inline` in its header (the
  ORIGINAL marker stays in its .cpp with a `// body <header>` pointer, as
  clear/says do), and the probes become unnecessary. Screen:
  `analysis/giantcallee.py`.
- **Judge a probe removal by inline decisions, not rows** (high). Keep the
  removal when `inlinemap.py`'s E/c strings and the call multiset stay equal
  to (or closer to) the image and the unit's exact set holds; fe rows can
  stay equal while a decision moves, and move while none does.
- **Deleting an unused probe definition shifts the unit's front-end counter**
  (high). A probe's definition is a declaration like any other: removing the
  last call and its definition flips the unit's count canaries (2.5, 3.6) and
  header-position ties (custom_planet 11 -> 379 from one strings.h
  declaration). Re-tune the canaries once, after the batch.
- **An expanded inline keeps the literals of arguments it ignores** (high;
  2026-11-19, par-1). VC6 emits every string literal into the unit's .data
  in source order (one `$SG` counter slot each) even when the only use is a
  parameter the expanded body never reads. The original's `rnd(bounds,
  "LABEL")` (crt_random.h, a header inline whose label is unused) left its
  labels in the image as literals no instruction references, between the
  literals around each expanded site (ACTIONDESTROY, GOODY7, TREES0, ...;
  about 100 in the image). A source that writes `rand() % K` or another
  stand-in lacks both the literal and its counter slot: restoring the
  labels moved terraforming_actions' canary pads 17 -> 5 and veh_init
  0x005C03D0 67 -> 18. rnd returns unsigned; where the image compares a
  roll signed (`jge`), the source reads `(int)rnd(...)`. Screens:
  `analysis/litgap.py <unit>` (image literals our object lacks, `U` =
  unreferenced) and `analysis/giantcallee.py` (helpers called only by the
  giants). Ambiguous spans (more sites than labels) stay as written until
  something else orders them.
- **A unit's literal run is its definition order** (high; 2026-11-19,
  par-3 literal sweep). VC6 numbers `$SG` literals and lays them out in
  .data in source order, so the image's literal run is in the address
  (definition) order of the unit's functions. A file whose functions are
  defined in another order gives every literal another counter slot.
  Putting the whole file in address order was codegen-neutral in eleven
  units (netwin, netdaemon, net_class, tutwin, video, filewin, newtechwin,
  uv2player, multiwin, replaywin, net_session_picker); move static helpers
  ahead of their first user, but keep a helper defined *before its own
  callee's definition* and give it a forward declaration if hoisting it
  changes what expands (tutwin's tut_callout: 0 -> 236 rows when hoisted).
  Keep `#pragma auto_inline`/`inline_depth` pairs around the same
  functions: a function compiled inside an `auto_inline(off)` region gets
  its own .data section, linked after the deferred functions' section,
  and the region changes codegen (veh prune_protos 167 -> 172 when
  name_proto slipped inside). Within a call, literal arguments are
  emitted right to left (`MessageBoxA(0, text, caption, 0)` emits caption
  first; `popp(.., label, 0, picture, 0)` picture first). A unit's
  initialised globals precede all its literals, so a global between two
  literal runs marks an original TU boundary (veh.cpp: 0x0068F0F0 after
  study_artifact; newgame.cpp: 0x0068F9F0 before sub_58f4f0). Single-use
  named `static char X[] = "..."` arrays are pooled-era stand-ins: unpooled
  units spell them as literals (with a `RECOVERY_LITERAL` at the same
  extent); only addresses pushed from several functions are named objects
  (one address pushed twice inside one function can also be a loop-top
  call duplicated by loop rotation: connect_session's NETCONNECT_CREATE).
  Restoring the literals re-tuned PrefWin::init (exact, 0 pads) and
  action_give (exact at 17 pads). Screens (scratch tools, not in tree):
  compare the unit's referenced image strings in address order with the
  object's `$SG` order.

enemy_move 0x0056B5B0, call multiset (2026-11-19 par-4)

- **A named local takes a small header inline out of the free class** (high in
  enemy_move, one function; `rnd`, crt_random.h). The image calls rnd at
  enemy_move's five constant-bound sites. With the guard-clause body
  (`if (bounds - 1 <= 0) return 0; return rand() % bounds;`) ours expanded all
  five and no dead pass-1 spend at the top refused them (ownerscan `--probes
  1,2,4,8,16,32`: at 16 the probes themselves are refused, at 8 all 53 on_map
  sites are, rnd never), so the body was free. `int value = rand() % bounds;
  return value;` (or `int limit = bounds - 1;`, or one `result` assigned in
  both arms) keeps rnd's own 32 bytes and makes it a charged inline: the five
  sites become calls and rnd's expanded/called decision equals the image's in
  every function of the 17 units that call it (ccu.py). A ternary stays free.
  Cost: a function balanced around the free body breaks (enemies_war
  0x0055C840: its second treaty_off is now refused instead of the last
  set_treaty). Symptom to look for: an image that calls a tiny header inline
  only inside its budget-exhausted giants while ours never calls it.
- **Pass 1 of a giant is a prefix, and plain caller statements do not move
  it** (high in enemy_move). Dead inline probes at the top turn expanded
  sites into calls in exactly reverse source order (veh_who L4648, base_who,
  base_who, go_to, on_map+anything_at, veh_top, ..., proto_power), so the
  budget runs out about 450 lines into the body; 128 dead stores or dead
  `if`s before that point, after it or at the end move nothing (B is not
  growing with this caller). Sites further down that stay expanded on both
  sides (on_map in the radius scans, veh_moves, anything_at) are not touched
  by 38 if-units of extra spend either: they do not draw on that budget.
- **The image's last expansion is one site later than ours** (open). Image:
  veh_who expanded at 0x56CA61 (its map_loc shared with the following
  is_ocean), on_map called at 0x56CC83; ours calls veh_who and expands
  on_map. So at that point the image has more room than ours, by more than
  nothing and less than one base_who/proto_power/wrap_map_x expansion:
  taking veh_moves' expansion out (function-pointer call, a measurement)
  gives exactly the image's two decisions, taking base_who, one proto_power
  or one raw wrap out releases too much (on_map still expands, then go_to).
  One 2-if probe at the top makes on_map a call without making veh_who
  expand. Not the cause: named locals in veh_who/anything_at/base_who
  (`tile`, `owner` dropped or the parameter reused: same decisions, copies
  exact), filler statements. A veh_who without `owner` gives the image's
  whole call sequence but its out-of-line copy 0x00500250 loses 6 rows.
- **Called x_dist/vector_dist beside expanded base_who: map_dist** (kept).
  A site the image calls while pass 1 still expands dearer callees below it
  was a nested site: `dist = map_dist(x, y, tx, ty)` at 0x56C1E3 replaces two
  function-pointer stand-ins (5935 -> 5625 rows). The same reading says the
  first go_to (0x56BF4E, called; the second site at 0x56C21E is expanded)
  sat inside an inline owner; none found yet, its stand-in stays.

Header-inline cost classes, tree census (2026-11-19 par-4, round 6)

Tools: `analysis/inlineclass.py` (per callee, the functions whose image call
count differs from ours; `--base` for a header hypothesis),
`analysis/probesweep.py` (each probe-carrying function without its probes).
Baseline: 34 header-inline callees differ somewhere, 134 site-units, almost
all in ten giants and in the Popup teardown chain; every other function's
counts equal the image's. So a wrong cost class shows only where a giant's
pass 1 ends, and the test of a body spelling is: copy exact, that giant's
call multiset, then the tree.

- **What a body costs, by spelling** (high; each measured with the copy
  byte-exact). Cheapest first:
  - one return expression, ternaries included (`return c ? a : b;`,
    proto_power);
  - one condition with an assignment inside it (`if (a && (v = e) < K) return
    v; return -1;`);
  - a guard clause plus a final ternary (`if (!a) return -1; int v = e;
    return v >= K ? -1 : v;`, veh_who);
  - two guard clauses or nested ifs with a named local (base_who,
    anything_at); a named result (`int value = ...; return value;`, rnd) or
    even an expression statement naming a parameter (`input;`) takes a
    one-line body out of the free class;
  - every further named local (`int index = b; A[index]...; index = a;`).
  Each step is about one small expansion in a giant's pass 1: it moves the
  last expanded site by one (enemy_move: veh_who at 0x56CA61 against on_map
  at 0x56CC83).
- **Named index locals in small writers are stand-ins** (high;
  faction_treaty.cpp). treaty_off/treaty_on/agenda_on/agenda_off were tuned
  alone with `int index = faction_id_with; ...[index]...; index =
  faction_id;` pairs. Written as `PlayersData[a].diplo_x[b]` at each use all
  four copies stay byte-exact and treaty_off is cheap enough for
  enemies_war 0x0055C840 to expand it twice and refuse the last set_treaty,
  as the image does under the charged rnd. When a caller refuses a small
  same-unit function the image expands, strip the callee's named locals
  first and check its copy.
- **The same body can show one or two `-1` stores** (medium). Of the image's
  expansions of base_who / veh_who / anything_at whose result is kept, 2 / 2
  / 2 store -1 in two separate copies (Console::hold, enemy_strategy;
  Path::find twice; enemy_move 0x56BB35, 0x56C194) and 6 / 8 / 4 in one. A
  body with one `return -1` can never give two, so all three have two
  returns; whether VC6 cross-jumps the two stores depends on the caller
  (enemy_move's pair at 0x56BB3B/0x56BB4F is separate with veh_who's
  one-condition body and merged again with its two-return body, though
  veh_who is not expanded there). Do not read a single store as evidence
  for the nested-if body.
- **Stand-ins are budget compensation: change bodies together** (high).
  anything_at's two guard clauses alone keep enemy_move's multiset; veh_who's
  two returns alone keep it; both together refuse veh_who at 0x56CA61 again.
  The pair needs one cheaper body before that site (proto_power as one
  return expression, or veh_top without its named local: both copies exact,
  the first also moves battle_fight closer). Test header spellings as a set
  against the giant, one deck row per combination.
- **Tested and rejected** (do not retry without new evidence):
  - range 0x00422F00 dearer (`int result`, assigned parameter, three guard
    clauses; copy exact): fixes base_build (46/45) and StatusWin::draw_active
    without their stand-ins, but 38-70 exact functions lose their expansion
    shape and battle_fight calls range 6 times too often. The image's
    expansions are the current guard-clause body.
  - has_fac 0x00421670: base_psych calls it at its first five sites and
    expands the last four (image `C C C C C b b b b`): called early, expanded
    late is an owner (2.8 "Missing owner"), not a class (found in round 7,
    below: an inline facility block around has_fac_built). The
    function-pointer stand-ins in staple_the_bastards, end_of_game and
    base_editor_fac stay until their owners are found; dropping the `queued`
    local makes base_doctors and mind_control expand one site too many.
  - go_to 0x00560AD0 dearer (pointer or reference local: copy loses 12 rows;
    proto_id local, dead local: copy exact): enemy_move's first site still
    expands without its stand-in. Owner, not class.
  - `int knob = 0;` in BasePop::exec/item/set_value/dialogs(), Popup::start,
    clear, says, get_noun, parse_set, map_loc, on_map, wrap_map_x, measured
    on the tree with every dead probe call removed: only BasePop::item moves
    more than one function (5 probe functions better by 20 site-units, 12
    other functions worse by 54). No tiny forwarder is a class candidate;
    the 39 probe-carrying functions do not share one under-charged callee.
  - pact_attacks through get_title/get_name (copy exact): enemies_war then
    expands one get_noun more than the image in pass 2.
- **Probe sweep** (`probesweep.py`, 39 functions): with the current headers
  no function keeps its call multiset and rows without its probes; the
  callees that flip are the teardown chain (~Scroll, ~FlatButton, ~BasePop,
  ~Sprite expanded where the image calls them) in 14 functions. The probes
  stand for spend in the callers, not for a header class.

Expression forms of shared header inlines (2026-11-20 par-1; the find_font
`?:` of config_popups generalised; tree-wide `feall.py --all --base`, 35 s
per variant)

- Screened on the header inlines with an expression choice that expand in
  two or more rung 4-7 functions: font_line_advance (`?:`, no `adjustment`
  local, inverted guard), zoom_scale (if/return, if/else into a named
  result), StringStruct::seek_pos (`for` walks, `while (i-- > 0)`), rnd
  (`?:`, no `value` local). None is a clean gain. The `for` forms of
  seek_pos compile identically; `while (i-- > 0)` breaks the out-of-line
  copy 0x004015B0. font_line_advance without the local moves only the
  canary ReportIf::on_iface_button_clicked (and -1/-1 rows elsewhere); the
  inverted guard breaks 27 exact draw functions. zoom_scale as if/return
  loses no exact function (net -75 rows) but the rows that move most are in
  units that never expand it (order_veh 1960 -> 1813, probe +45,
  enemy_move +29): unit-counter noise, and its real hosts split (veh_draw
  -12, draw_tech +9, boom +6). rnd as `?:` breaks four exact functions.
- So the config_popups lever was the function-local find_font (one host
  per TU, an out-of-line copy to keep exact), not a class of header bodies:
  the header inlines' forms are already pinned by their exact hosts (see
  the joint solve below). Look for the same lever in TU-local helpers that a
  non-exact host expands several times, and judge it by the host's candset
  (one web per expansion on each side), not by tree rows.

Joint solve of the header-inline bodies (2026-11-19 par-4, round 7)

Tool: `analysis/jointsolve.py` (README). Objective: the image's call count
of every header-inline callee in every function, with the function-pointer
stand-ins removed so that a body is not judged against its own
compensation. Start: 156 site-units (128 as written). Deck: 22 inlines, 2
to 8 spellings each, every one with its out-of-line copy byte-exact
(`copies`); 9 spellings fail the copy and are out (range guard+ternary,
on_map ternary, x_dist with an assignment or two returns, has_project as
one `&&` or nested, is_known with a guard clause, is_port and has_fac with
a named `base` pointer, has_fac with `is_built`/`queue_count` as two guard
clauses).

- **The current set is a fixed point of single moves** (high). 41 single
  moves, tree-wide: none lowers the site-units without making another
  (function, callee) pair worse or breaking an exact caller. Every move
  that changes a decision changes it in battle_fight 0x00506AF0, enemy_move
  0x0056B5B0 (measured without its go_to stand-in) or probe 0x0059F120 and
  nowhere else; the copies and all other callers are indifferent. So the
  remaining header-inline differences are not a property of the header
  bodies: they are what these three functions' own structure (invented
  helpers, hand-copied owners, stand-ins) leaves open.
- **A broken exact caller is shape evidence and outranks a site-unit**
  (high). Spellings rejected by exact callers although the copy is exact
  and the decisions are equal or better: base_who/veh_who/anything_at
  without the `tile` local or through owner_at (14-16 exact functions),
  veh_top with the local declared after the guard (7), vector_dist(2) with
  `dx`/`dy` locals (18), has_project as guard + ternary (33), on_map as
  guard clauses (12-13), has_treaty with a named local (5), has_fac without
  `queued` (4). on_map as `if (..) return true; return false;` gains one
  spot_all site (6/3 -> 6/4) and costs Path::move 0x0059BC10 a commutative
  `lea` operand order at its wrap_map_x + on_map expansion: rejected.
- **has_fac_built 0x00500290 is a header inline** (high). base_build
  expands it at its first seven selected-base sites and calls it at the
  sixteen after its pass-1 budget runs out (the same place where its range
  sites turn into calls). "Called only by a giant, copy next to the giant"
  (above) applies to a function that looks like a plain helper defined
  after its only caller. base.cpp's two invented forwarders were that
  function. Tree: 156 -> 151 site-units, nothing lost.
- **Called early, expanded late, and no budget moves it: the early sites
  are one inline level deeper** (high in base_psych 0x004EA540). The image
  calls has_fac at base_psych's first five sites and expands the last four,
  with two has_project sites in between expanded. Dead spend at the top (1
  to 8 ifs) moves nothing; has_fac_built at all nine sites expands all
  nine; the facility block as an inline owner with has_fac written
  directly expands the first has_fac and calls the rest. With the block as
  an inline owner AND has_fac_built inside it, has_fac is a depth-3 site
  and all five are refused while has_project (depth 2) expands: byte-exact
  without the stand-in. The same recipe does not transfer blindly:
  staple_the_bastards 0x0041B4F0 expands has_fac at depth 2 and at depth 3
  (image: call) whatever the spend, and base_minerals 0x004E9CB0 refuses
  has_fac at depth 2 at all seven sites where the image expands the last
  two, so those two stay written as `has_fac(id, BaseIDCurrentSelected,
  0)`. Depth decides before budget does, but the depth at which a callee is
  refused differs per caller; test depth 1, 2 and 3 per function.
- **Every out-of-line `range(reactor, 1, 100)` copies the reactor first**
  (high; 13 of 13 image sites: battle_fight 8, enemy_move 0x56B7B8 and
  0x56B80C, interceptor 0x5067B7, battle_report 0x4221B9, draw_active
  0x4B5A17: `xor r,r; mov rl,[reactor]; mov eax,r; push eax`). Ours pushes
  the zero-extended register directly at 11 of them. So proto_power passes
  the reactor byte through an int: a named `int reactor` after the guard
  clause, or an inline level `reactor_power(int reactor) { return
  range(reactor, 1, 100) * 10; }` (both keep 0x005799A0 exact; a cast, a
  `proto_id` local, a ternary with the local declared first or a named
  result do not give the copy). Not landed, because both change the cost:
  the named local is one class dearer (enemy_move then needs anything_at or
  base_who as guard + ternary to keep its multiset; battle_fight calls
  range 18 times against the image's 12, 2119 -> 3366 rows), the inline
  level adds a depth (enemy_move keeps its multiset, 5688 -> 5681 rows;
  battle_fight 2119 -> 3047 rows with eleven more wrong sites) and no other
  function of the tree moves. battle_fight's call multiset (a higher rung)
  is balanced around the copy-less body through its invented veh_cur_hp;
  land the reactor copy together with battle_fight's helpers, not before.
- **An unused function-pointer local is a budget probe, not a call forcer**
  (high in base_build 0x004F81A0). With the current headers each of the 15
  `range_call(...)` / `project_call(...)` uses can be spelled as the plain
  call, one at a time and all together, with identical rows (3640) and an
  equal call multiset, as long as the six `T (__cdecl *const x)(..) = f;`
  declarations stay; deleting any one declaration (then unused) gives range
  46/45 and 3754 rows. So base_build's pass 1 already refuses those sites,
  and what the declarations supply is one unit of pass-1 spend each. Before
  reading a function-pointer stand-in as "this site needs forcing", spell
  the use plainly and keep the declaration: if nothing moves, the site is a
  budget question and the stand-in is a dead probe.
- **A body one class dearer can make a caller's FRAME right** (medium).
  veh_who as guard + ternary against one condition with an embedded
  assignment: air_power 0x005678C0 goes 955 -> 1083 rows with an identical
  call multiset, and its frame goes 0x44 -> 0x48, the image's. The rows are
  slot renumbering inside the right frame; judge such a move by the frame.
- **Where the giants' remaining sites are** (for their owners).
  battle_fight: ours never exhausts pass 1; the image calls range 0x50AB85,
  is_ocean 0x50ACB1, has_fac 0x50ACE0 and 0x50ACF4, has_project 0x50AD0A
  and veh_who 0x50AEC7, all within its last 0x400 bytes of pass-1 sites.
  spot_all 0x005B6260: the three on_map calls ours lacks (0x5B6D2C,
  0x5B7200, 0x5B8295) are depth-3 sites inside spot_base/spot_stack ->
  spot_tile expansions, and the image's sequence over those owners is not
  monotone (C C C C E E C E E), so it is not an on_map class.
- **enemy_move's direction loop 0x56BA68** matches (21 -> 7 rows) in two
  builds: veh_who with one condition, and `remote_own` forced to memory.
  Both do it by the same wrong equilibrium: faction_id takes ebx for the
  whole function, which frees edi for the loop counter. The image is a
  third state: ebx holds the `veh_id * 52` temporary from 0x56B642 (also
  homed at [ebp-0x10]), proto_id and remote_own are memory only ([ebp-0xAC],
  [ebp-0xA0]; `remote_own = 1` is stored through ecx at 0x56B619 before
  the faction compare), faction_id is homed at [ebp-0xC] and loaded into
  ebx at the loop's pre-header, and edi is the counter. Ours gives ebx to
  the constant 1 / remote_own at 0x56B5DC and to proto_id at 0x56B694,
  keeps the temporary in memory and caches it in edi from 0x56B75A across
  the loop. The order of the four entry assignments (24 orders), a `Veh *`
  local, five spellings of the remote_own test and a damage-block owner do
  not move the entry (46 rows); the owner lowers the function to 5547 rows
  but calls proto_power at 0x56B7B8/0x56B80C where the image expands it.


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

- **Auditing a header helper (par-5, 2026-11-19).** Three screens, in order:
  - *Out-of-line copy as a wrapper.* A small image body whose local sits in a
    dead parameter's home is often an expanded inline, not `&param` aliasing:
    has_fac_built(int) 0x00500290 is byte-exact as
    `return has_fac(facility_id, BaseIDCurrentSelected, 0);` (offset at
    `[ebp+8]`, the base id loaded before the range test as the argument
    temporary). Defined plainly after its only caller it stays a call at all
    16 sites without `auto_inline(off)`.
  - *Macro screen.* Replace a single-return header inline by a macro of the
    same expression and run `feall.py --all --base`. Helpers whose expanded
    sites show named-parameter temporaries lose exact functions as macros and
    are real inlines: wrap_map_x (76 lost), map_range/map_dist (18),
    rect_width/rect_height (10), table_text (7), map_loc (7), the veh.h
    triad/rating accessors (Console::destroy, spot_all 1359 -> 3904),
    compass_name (say_orders). Helpers that only spend or reserve /Ob2 budget
    move functions both ways (is_human: base_build 4527 -> 3690, control_turn
    313 -> 850; map_tiles(): gen_terrain_poly -51, order_veh +55): decide
    those per function, by call multiset. Subtract the counter effect: any
    definition removed from a shared header moves the count canaries.
  - *Removal cost.* Deleting a never-called inline or in-class member, or
    replacing a reference accessor (`T &f() { return G; }`) by its global,
    changes no code; it flips 3-7 count canaries per removal, most of them
    restored by a whole-unit pad sweep against the final headers.
- **A dead budget probe may be standing in for a costlier original callee.**
  popb 0x0048C650 needed `popb_budget_probe4` only while its Brood Pit test
  used an invented cheap bit-test helper; written as the real
  `has_fac(id, base, 0)` the probe is redundant (35 -> 23 rows). Before
  tuning probes, check that every helper the function expands exists in the
  image.

### 3.2 Registers and order
Scratch registers and scheduling (2026-11-19 par-5; probes in
`recovery/tools/analysis/sched/`, see its README; diagnosis with
`recovery/tools/analysis/schedscan.py <unit> <0xADDR>`). Measured with
single-factor probe families under the game flags; every family gave one
result over 64 symbol-counter pads, so none of these is a counter canary.

- **S1. Scratch registers are handed out round-robin** (high in straight-line
  probes; p05, p06, p07). VC6 keeps one cursor per function and gives each new
  register-held value the next register of eax -> ecx -> edx after the one it
  handed out last, continuing across calls and statements. One more such value
  anywhere earlier in the function rotates every later choice by one step:
  `f1(ga + 1);` added in front turns `a18 eax, a14 ecx, wfc edx, lea eax`
  into `ecx, edx, eax, ecx`, a second one into `edx, eax, ecx, edx` (p05).
  Counted (one step each): a load into a register, a `lea`, an arithmetic
  result, a byte extension (`xor r,r; mov r8,[m]`), an index register
  (`garr[ga]` is two), a compare that loads one side (`ga > gb`). Not counted:
  immediate stores and pushes, `cmp [m], 0`, call results (eax), the thiscall
  receiver in ecx, `this` copies to callee-saved registers. A register still
  busy is skipped. Call arguments allocate right to left (`f3(a+1, b+2, c+3)`
  gives c eax, b ecx, a edx).
  Reading a diff: when one run of the image and ours differs only by the
  rotation eax->ecx->edx (or its inverse) and the rows before it are in step,
  our source creates one register value more (ours = image rotated by one) or
  one fewer than the original somewhere *before* that run, often far earlier
  and in code that is itself exact. Typical one-value differences: a named
  temporary against a re-read, `p++->f()` (the old pointer is a temporary)
  against `p->f(); ++p`, a byte global read once or per use, an accessor
  inline against the field, an address formed with `lea` against folded into
  the operand. schedscan.py's cursor track prints the offset (ours - image,
  mod 3) of every paired allocation and where it changes. In real functions
  a replay of the rule over the image listings of the BYTE_EXACT functions of
  five units predicts 73% of allocations, no better than "lowest free
  register" (75%): the scheduler reorders instructions after allocation and
  liveness read from a listing is approximate. Use the rule to locate the
  window where the count diverges, not to predict a single row. Much of the
  rest is S7: values used twice, zero tests and named values take the lowest
  free register and do not move the cursor.
- **S2. Evaluation order sets allocation order.** Call arguments are
  evaluated right to left, and the receiver temporary of `p++->f(...)` is
  created before the arguments that follow it in the push order (p08): the
  image's `push a; xor eax,eax; push b; mov al,[g]` reuse of eax means its
  byte argument was allocated after the earlier arguments had been pushed.
- **S3. Order of independent loads and stores is the scheduler's**, with
  dependences and the source's tuple order as tie-break. Statement order of
  two unrelated stores does not move them when the scheduler has a reason to
  reorder (Win::update_screen's RECT stores: 24 orders tried). A final
  `xor eax,eax` goes before a store of a value loaded through a pointer
  (`ga = *p; return 0;`) and after a store of a frame-local field
  (`ga = buf[2]; return 0;`) (p10). Unresolved: MediaRuntime::setup_sound_device
  (image `xor eax,eax` before a store from a local DSBCAPS: if/else, goto and
  named-result forms compile identically), DesignWin::on_iface_right_click
  (thiscall receiver hoisted above two address-taken local stores: statement
  order and label_get forms do not move it). The uV library unit
  (src/uv/movie_media.cpp) has a family of these: the image sets the return
  value (`xor eax,eax`) between the epilogue pops (`pop edi; pop esi; pop
  ebp; xor eax,eax; pop ebx`) where VC6 sets it before them (sub_5cae80,
  setup_sound_device); /G4-/G6, /Oy-, /Oi, /Ob2, /Os, /Oa, /Ow, /Op are no
  closer for the unit. Its compiler identity is open (VC5 needs the unit's
  forced-include header adapted before it can be tried).
- **S4. Which operand a two-operand expression allocates first follows the
  order the operands were first referenced, not the spelling** (high for
  members, locals and fields; p03, p04, p04c, p01_fieldorder). For `x + y`,
  `x * y`, `x & y` the operand referenced *later* in the function is loaded
  first and becomes the destination; the one referenced earlier is the source
  (`f1(x); f1(y); ... x + y` gives `mov eax,[y]; add eax,[x]`; swap the two
  earlier calls and it becomes `mov eax,[x]`). Spelling `y + x`, regrouping,
  casts and a named sum compile identically (this replaces the older
  "a + b of globals loads b first"). With no earlier reference: members and
  locals take the right-hand operand as destination, globals the left-hand
  one. Declaration order does not count. Non-commutative operators keep their
  operand order (`x - y`, `cmp x, y`), but which of the two gets eax and which
  ecx follows the same reference order: in SocialWin::draw_*_alloc the
  `title.left < title.right` block loads left into eax only when title.right
  is referenced (directly) before title.left earlier in the function.
  References made inside an inline helper's body do not count as direct
  references of the caller's struct fields: `expand_rect(&title, -0x17, 0)`
  instead of `title.left += 0x17; title.right -= 0x17;` gave the image's
  order without touching the earlier code, and made all three draw_*_alloc
  functions exact (p01_inline_helper). References in an `if` arm are
  unreliable (p04b: some count, some do not). Lever: when the image swaps the
  two registers of a pair of member/field loads and the instruction order is
  otherwise yours, look for the pair's *first* references earlier in the
  function: a statement order, or an inline helper (expand_rect, offset_rect,
  rect_width) the original used for one of them.
- **S5. Post-increment around a call.** `p++->f(args)` copies p to a
  temporary (ecx) and adds before the call; `p->f(args); ++p;`,
  `for (...; ++p)` and `int r = p->f(); ++p;` add after the call. The forms
  differ by one allocation (S1), so fixing the add's position rotates the
  registers of everything after it; FameWin::init 0x00448400 is such a case
  (the image adds after the call but is in step with the `p++->` form
  downstream: the original saves the allocation somewhere else, not found).
- **S6. Two struct copies from one source** (p09): writing the members
  directly (`ctx.shadow = d; ctx.light = d;`) interleaves the stores field by
  field (`[edi],[esi],[edi+4],[esi+4]...`) with the second address formed
  first; through reference-returning accessor inlines
  (`shadow_vector() = d; light_vector() = d;`, or named references) VC6
  emits one whole copy, then the other. StatusWin::init became exact with the
  direct members (Caviar friend stand-in).

- **S7. Named values sit outside the cursor** (par-2 2026-11-19; high on the
  probe classes below, high on five real functions; probes p13, p14, p16,
  p20). Three kinds of value take the lowest free scratch register (eax
  first) and leave the S1 cursor where it was: a value used more than once
  (`gb = gc; gd = gc;` loads gc into eax whatever the cursor says, and the
  next single-use value continues the cursor as if gc had not been
  allocated, p14), a load only tested against zero (`if (gc) return;` ->
  `mov eax, gc; test eax, eax`, p20), and a named local or inline parameter
  that VC6 cannot forward-substitute into its use (a thiscall argument read
  before the receiver is set: `char *v = gpp[5]; gst.get(v)` or label_get's
  `label_value`, p16). Single-use anonymous expression temporaries
  (arguments, `x + 14`, `lea` of a buffer) take the cursor. This is the rule
  behind most of the "arguments in the opposite order" ties: when a pair of
  argument registers is the image's rotated by one, the image either made a
  named value of something ours computes as a temporary (or the reverse),
  in the window or before it. Measured fixes: get_noun_phrase 0x0058F8B0
  (`phrase_id = phrase_id + 9; if (phrase_id > 0) { i = phrase_id; do ... }`
  -> `for (i = 0; i < phrase_id + 9; i++)`: the reassigned parameter was a
  named value, the image's bound is a temporary; exact), PickWin::
  on_button_clicked 0x00489BA0 (`const int rows = field_5DB0_;` then
  `item(StringTemp, rows + 14)`: exact), Menu::add_menu 0x005FAEF0 (the
  strcat destination `entries_[slot].text` read into a block local at three
  sites: exact; an inline `cat_to(d, s)` helper is the same), ProdPicker::
  on_right_click 0x00493EA0 (written-out `StringTable.get(Labels...)`
  instead of label_get at one site, and the faction read inside the `|=`
  instead of a named const: 8 -> 4), QuayleWin::on_redraw 0x004956D0
  (`_itoa` of the chapter through a named local: 13 -> 9), read_rules
  0x005873C0 (three `table_text(x)` -> `StringTable.get(x)` together: the
  image's four bubble labels are one cursor run ecx, edx, eax, ecx; exact),
  size_of_planet 0x0058CED0 (label_get -> written out at one site: exact),
  MultiWin::draw_player 0x00478630 (written out -> label_get: 57 -> 47).
  label_get, table_text and every other inline whose parameter or local
  holds the label are named values; `StringTable.get(expr)` written at the
  call is a temporary. The choice is per site and can need several sites
  flipped together (read_rules): --try adds an all-sites candidate for
  these rules. A flip can also move the /Ob2 budget (DiploWin::on_redraw:
  one label_get written out, 4 -> 243 rows), so check the call multiset.
  Caution: in isolated probes a named local of a plain load is
  forward-substituted and compiles exactly like the expression (p15, p17,
  p18, p19, p21, p24: about 60 pairs, globals, members, params, pointer
  reads, cdecl and thiscall, pending or flushed stack pops, callee-saved
  bases), so whether a site is affected depends on its function; the rule
  says what to try, not where. Screen with `schedscan.py <unit> <addr> --try`:
  it wraps one argument (nvarg) or one operand inside an argument (nvop,
  `nv(count) + 14`) in an identity inline `nv(x)` (a named value without
  knowing the type), and tries label_get <-> written out and unname, one
  site per candidate. A hit is a WORKAROUND(inline) until the original's
  spelling (a named local, an accessor, a helper's parameter) is chosen.
  An inline member accessor's result is such a named value too, and a
  far-apart pair of "scheduling ties" can be one cursor step: Caviar::
  init_class 0x006185A0 had 14 rows in two straight-line store blocks 40
  instructions apart (2026-11-19). The image's first block holds three
  values in eax one after another (lowest free) and two in edx, ecx
  (cursor), so the second block starts at edx; ours started it at ecx.
  Reading the two row pitches through `Buffer::stride()` and the two pixel
  pointers directly fixed both blocks; named locals for any subset of the
  five loads in every order (326 forms), statement orders and 0..15 dead
  locals (3.6) did not. Screen: when a later block is the image's rotated
  by one, try each load of the earlier block through an identity inline
  (2^n forms), then replace the hits by the class's accessor. Each
  expansion is a free inline for the /Ob2 budget (2.8), so re-check the
  teardown depths of an EH function afterwards.
  Whole-set pass (par-2 2026-11-19, second round; 347 non-exact functions,
  every rule plus nvany on those of at most 150 rows or 700 instructions):
  how to read a `--try` hit.
  - *Type of the named local.* `nv()` is a template, so it names the value
    with the operand's own type. The local must have that type: mind_control
    0x0059EA80 is exact with `short y = Bases[base_id].y;` in front of
    `veh_at(Bases[base_id].x, y)`, an `int y` is 37 rows and naming x as
    well 153 (MessageWin::on_selected's set_cursor is the same `short y`).
  - *An accessor is a named value.* EditBox::on_redraw 0x00615270 read
    `text_parameters_[0][0]` raw; Buffer::foreground_color() at the three
    reads took 87 rows to 3 (then exact with the background as one ternary).
    `contract` misses accessors whose body carries a cast, so check the
    class for one whenever nv() hits a member read.
  - *A hit must fix its own site.* Keep a hit when the differing image rows
    it removes include the site and it adds none (make_gift's portrait,
    control_turn's proposal name, veh_scoot's `my`, PickWin::init's `x`,
    Win::init's `left`/`top`, the `_itoa` operands of draw_tech and
    draw_commerce: 5 to 140 rows each). When several unrelated sites give
    the same row count (process_message: three `packet->payload_14_` sites,
    all 550 -> 460; drone_riot's veh_name argument, 332 -> 157 while its own
    site gets worse; draw_pie's `BYTE red`; slide_show, where the best
    "local" was an `unsigned char` that only moves frame slots) the probe
    moved the callee-saved or frame allocation of the whole function: it
    says a register or slot priority is wrong, not that this operand was a
    local. MonuWin::init, MonuWin::on_redraw and BasePop::exec are cursor
    cases of the same kind: any one of several sites flips the rotation, so
    the missing allocation is earlier (on_redraw: at the switch join
    0x00475E50).
  - *Inline budget.* label_get, table_text and `contract` spend /Ob2 budget,
    a plain local does not. spot_all 0x005B6260 read 1359 -> 1183 rows with
    four has_treaty contractions, but each became an out-of-line has_treaty
    call the image has not got (reverted); base_editor_fac 0x0041B790 needs
    three named strings and is 7 rows with `char *` locals, while the same
    three through table_text/label_get drop its later inline has_fac
    (235 instructions for 263). Compare callees.py before and after.
  - *More than one site.* `--pairs K --triples K` combine the K single sites
    that moved the rows at all (base_editor_fac was a triple; of 64
    functions with two or more cursor changes and at most 40 rows it was
    the only one that combinations moved). For label/table sites every
    subset matters (read_rules): `analysis/labelsubsets.py <unit> <addr>`
    compiles all 2^n subsets of a function's label_get / table_text sites.
    Over the 52 non-exact functions with 2 to 10 such sites it found three
    pairs no single flip reaches: ReportIf::draw_projects 0x004AA0B0
    (51 -> 9), ProdPicker::exec 0x00494700 (62 -> 29), BaseWin::
    draw_commerce 0x00410870 (60 -> 40), each with equal instruction counts
    and call multisets. When two or three subsets tie within a few rows
    (MessageWin::on_selected: 25, 27, 28 for any one written-out site of
    three) only the rotation parity is fixed, not the sites: read the image
    (receiver set before or after the push) before choosing.
  - *Bookkeeping.* A local added or removed renumbers the unit's later `$SG`
    labels (PickWin::init's `int x` moved on_button_clicked's ten literals
    by one): run `recover rebind-literals` on every unit touched this way.
  Yield of the pass: 2 functions exact by fe (mind_control, EditBox::
  on_redraw; PickWin::on_button_clicked's literals bound), 21 more moved,
  65,802 -> 64,634 differing rows tree-wide, none lost (name_proto
  396 -> 196, make_gift 341 -> 177, control_turn 313 -> 173, Win::init
  187 -> 57, draw_projects 51 -> 9, use_ability 27 -> 6, base_editor_fac
  20 -> 7, CouncWin::on_button_clicked 38 -> 14 and smaller ones). About
  nine in ten functions have no single-site S7 hit: for the 1 to 10 row
  near-misses the remaining rotation is not a named/anonymous choice that
  nv() can reach from inside the function.
- **Locating the one-load difference** (par-2 2026-11-19, `analysis/rotscan.py`). Tree pass on 347
  non-exact functions: 150 phase-divergent, 197 phase-equal (the remaining difference is not a cursor
  count: callee-saved choice, order, layout). For a divergent function the window between the last
  in-sync load and the first off one is where the extra/missing scratch load sits; `schedscan.py --try
  --near A-B` restricted to that window is fast enough for functions of 300+ rows, which the whole-set S7
  pass skipped. Hits of the first round: a named copy of a 16-bit id for one argument (drone_riot 0x004F4FB0,
  317 -> 119, phase-equal afterwards; the image holds the id as a word in cx), an inline helper written
  out at one site of two (veh_draw 0x005594F0 zoom_scale for x only, 165 -> 59; the image reads the x
  table entry once per arm), a table_text written out (name_proto 0x0057E040), a named `int16_t y` for
  map_range (time_warp 0x005AEDE0), rect_width for a re-read edge (DesignWin::init 0x0043C270). When
  several unrelated window sites give the same small gain (BasePop::exec: eleven nv() sites, 303 -> 258..266;
  NetDaemon::synch, 432 -> 430) it is a priority shift, not the cause (S7 caution above).
  Second round (same day): the rest of the 148 divergent functions swept at their windows gave one more
  real hit (popb 0x0048C650, a label_get written out, 23 -> 9); the cursor is per function (probe: one to
  three extra loads in the preceding function leave the next one's first register unchanged), so a
  divergence at entry is a named-value or callee-saved choice, not a neighbour. `rotscan.py` now also
  reports the first callee-saved occupant divergence; 70 of the 199 phase-equal functions differ there
  first. The S7 single-site rules aimed at those lines (57 functions, `--near` = line-15..line+1) found
  nothing that fixes the site: the few row drops were priority shifts. Callee-saved and register/slot
  divergences need the rule-4 levers of 2.7 (reference counts and loop depth of the competing roles:
  regprio.py, candset.py), and most of them start with a frame-slot difference (slide_hide, reset_territory,
  draw_ops), i.e. rung 5 first.
  Callee-saved cases are not rotation: ReportWin::labs_line keeps `half` in a frame slot [ebp-8] and the
  host's Buffer view in ebx (`lea ebx,[eax+0x444]` once, the null-checked `neg/sbb/and eax,ebx` at every
  Sprite::draw), ours gives ebx to half and spills the view to [ebp+0x14]: a priority question for 2.7.
  BasePop::exec's set_width/set_height arms form a full Dialog pointer (`lea ecx,[vb+esi+0x21d0]`, then
  `mov [ecx+0x2c],eax`): the shape of an inline Dialog member call on `dialogs_` (receiver conversion,
  no null test); the standalone BasePop::set_width (exact) folds the conversion into the store, so exec's
  arms do not expand that body. A free inline taking `Dialog &` adds a null test (322..341 rows); the
  Dialog member setter needs a dialog.h change (header hypothesis, not tried).
- **S8. Constant folding and subtraction order in a chain are canonical**
  (par-2 2026-11-19, p22, p23; high). `a - b - c` with b a field and c a
  named local compiles to `a - b - c` for every spelling tried: the
  operands in either order, `a - (b + c)`, `-c + w`, unsigned casts on each
  term, an inline `rect_width`, `w -= c` as a separate statement; only a
  store in between (`out = a - c; out -= b;` to a member) changes it. The
  same holds for constants: redo_caption_buttons 0x005F2760's `- 2` folds
  into `lea eax, [ebx + ebx + 2]` whether written as `2 * w`, `w + w`,
  `w * 2U`, `2 * (w + 1)`, through `nv()`, an inline `twice(w)`, a local
  holding 2 or a separate `x -= 2` (about 40 forms; only `w << 1` keeps
  the `sub esi, 2`, with the shift scheduled last). draw_labs 0x004A7000
  (image `right - inset - left`, ours `right - left - inset`; 48 forms) and
  redo_caption_buttons stay open: the order is not a spelling choice, so
  look for a different data flow (a value stored and re-read, a different
  type for one field, a value computed in another statement that is used
  twice).

- **Operand order of commutatives.** See S4 above: the destination is the
  operand referenced later in the function. For `f(a) OP f(b)` it calls the
  right side first. It canonicalises
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
- **Two zero stores as immediates or through `xor eax,eax`.** Rule: 2.7
  "Constant pieces and immediates" (Z1-Z4): immediates mean the zero has
  another reference (a push, a memory compare) and a callee-saved register
  free over its whole range; the scratch piece means no other reference or no
  free register. Measured on a
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
  outer call's other pushes, write `StringTable.get(Labels.strings_ptr[k])`
  at that site: written out, the later arguments are pushed before the label
  load (PrefWin::init, interlude's CARETAKE parse_says). The `inline` vs
  `__forceinline` keyword is irrelevant (identical over all 5,574 markers);
  the named `LPSTR label_value` local inside label_get carries its shape
  (label offset loaded before ecx is set, as through an LPSTR parameter).
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
- **A global load between the compare and the jcc (`cmp al,3; mov
  eax,[Labels]; jae`) is a head merge** (high, 2026-11-19 par-1, probes
  `a_both`/`e_cmp3`/`f_else_store`; say_weapon 0x005905C0 exact). It is the
  same pass as the hoisted pushes: when the first instruction of both arms
  is identical *including the register*, VC6 moves one copy into the
  predecessor after the compare (a `mov` leaves the flags alone). Three
  consequences:
  - Both arms must begin, in source order, with the same read: an arm
    whose first statement is something else (`g = 1;` before the label)
    does not merge even when the scheduler later puts the label load first.
  - Both copies must get the same scratch register. The else arm continues
    the S1 cursor from the end of the then arm, so a then arm with one
    register value more or fewer than the original gives the else arm
    another register and no merge (a_both: `push esi` shrink-wrapped into
    the then arm, ecx against eax). In draw_unit 0x0042C9C0 ours has eax in
    the then arm and ecx in the else arm; the cursor track (schedscan)
    first leaves the image in the then arm's `_itoa` branch (0x42CC31).
  - The image's `mov al,[m]; cmp al,3` is a *consequence*: VC6 folds `mov
    al,[m]; cmp al,K` into `cmp byte ptr [m],K` only when the next
    instruction does not touch eax, and the merged `mov eax,[Labels]` does.
    So a memory compare in ours against a register compare in the image
    before a hoisted load is not a type or named-local question (a
    `const uint8_t mode` local, int/unsigned/char, inline or cast forms all
    gave `cmp byte ptr`; only a second use of the value loads it). Fix the
    merge and the load follows.
  battle_compute 0x005032A2 and custom_my_faction 0x004DD2F4 are not this
  shape: their Labels load sits several instructions above a `test` in the
  same block (ordinary scheduling of the next statement's load).
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
  patches**: write the call; if only those bytes then differ, declare the
  span with `RECOVERY_IMAGE_PATCH` (docs/DECOMP_MAP.md) and keep the call in
  the source (filefind_init 0x00600400 measures exact outside the declared
  span). Stale base-relocation entries inside the span say where the
  replaced code had absolute fields (Caviar::init_class: 0x0061874F and
  0x00618756, a `push offset` and a `call [import]`). An `__asm`
  nop block is not a stand-in: any `__asm` forces an ebp frame in VC6 and
  stops enregistering locals (measured 2026-11-19 on filefind_init
  0x00600400: original `popup->exec(0, 0)` call 0.957, only the nine
  patched bytes 0x600545..0x60054D differ, frame 0x24C; nine `nop`s in an
  `__asm` block 0.703, `[ebp-4]` popup). Moving the block into a
  `__forceinline`, `inline` or `static` helper changes nothing: /Ob2
  expands the helper and the caller inherits the ebp frame (0.703 each).
  Caviar::init_class 0x006185A0 (18 nops, `jmp 0x618873`, nop at
  0x61874B..0x618762) reacts the same way: one `__asm { nop }` turns its
  esp frame into an ebp frame. The user allowed an `__asm` stand-in for
  filefind_init on 2026-11-19; VC6 cannot express it, so the patch is
  declared to the comparator instead.
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
- **Global register equilibrium in very large members is a symptom, not a
  rule.** If the image keeps only `this` in a callee-saved register and every
  named local in memory (no zero register) while ours settles the other way
  (zero register, a hot local enregistered, `this` spilled and
  `this + base + i*K` shared), look for a subscript the source folds and the
  original did not. InfoWin::setup_text 0x00456300 (corrected 2026-11-19): the
  cause was literal row indices where the original wrote `row_text_[pane][k++]`
  (2.4, counter subscripts); with the counter `this` returned to esi, the
  zero register (88 uses) disappeared and the frame matched. The earlier
  note here ("index spellings fold identically, so they do not move it") was
  wrong: it had only compared spellings with constant indices.
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
- **Residue census: what is outside the body for the 1-15 row near-misses**
  (2026-11-19, par-2; `analysis/residue.py`, `analysis/outside.py`,
  `analysis/sitek.py`; 98 functions of at most 15 fe rows, 95 after the three
  fixed below). Classes of the residual windows (a function can hold
  several): scratch renaming 19 functions, callee-saved or mixed renaming 11,
  register reuse without a renaming 9, schedule (same instructions, other
  order) 16, instructions more or fewer 32 (register copy 5, folded load 4,
  zero register 6), layout 23, operand / address-term order 9, immediate,
  slot or pointer bias 11, with an unclassified window 29. Levers that do not
  change the body text, one at a time:
  - **Unit position and counters: nothing.** Function-declaration pads before
    the marker, 0..63 (`unitpad.py --at`): 0 of 98 move. The function moved
    to the end of its unit: 86 of 88 unchanged (two VC5 Caviar functions gain
    one row). So none of these is a count canary, and definition order relative
    to callers and callees is not a lever under /Ob2.
  - **Frame-symbol and budget pads: nothing gained.** `locpad.py` 0..7: 14
    move, all one-way up at a threshold (the 2.1 caller budget), none
    periodic, none lower. `sympad.py`: 10 move, all up. Every parameter
    counts one frame symbol (3.6), so an unused extra parameter is the same
    screen and the same answer.
  - **Signature: fixed by the image, not a hidden lever.** `int` -> `short` /
    `char` parameters and return types always add rows (movsx, byte
    registers: 259 of 270 variants higher, none lower); `int` -> `unsigned
    int` is invisible in 106 of 135 variants (the rest change a compare or a
    division) and never lowers a function. Member / static / free is read
    off the image (`ret N`, ecx read before written); residue.py prints the
    one case where a member never reads `this` (Console::set_ai, still
    `ret 4`). An exact direct caller confirms the pushes (printed per
    function).
  - **Declared signedness of the globals and fields the body names**
    (`typ`, 1,324 variants): 226 move, one lowers (draw_base_names 10 -> 6,
    a function with another cause). Signedness shows as jl/jb, sar/shr,
    movsx/movzx rows, which residue.py would class `type(sign)`: none of the
    98 has such a window. The type census (typescan.py) is the right tool;
    near-miss registers are not a type effect.
  - **Header inline bodies** (`inl`: each called inline with a named result,
    a bare return, or its parameters copied to locals; 859 variants for the
    98 functions, 1,426 more with one parameter copied at a time, 1,307 for
    the 95 functions of 16..60 rows): 85 of the 859 move, two reach zero
    rows, and both turned out to be site facts, not header facts: tree-wide
    the header spelling loses more than it gains, and the same effect written
    at the one call site is exact. The single-parameter forms and the 16..60
    group add nothing beyond counter effects.
  **A header variant that adds a local shifts every including unit's
  front-end symbol counter**, so the count canaries (3.6) move with any such
  variant (PrefWin::init, action_airdrop, farm_click 54 -> 46, sat_attack
  17 -> 15 moved identically under unrelated variants of general.h, map.h and
  buffer.h). Read a tree-wide header result against a control variant that
  adds a dead local to another inline of the same header.
- **A literal argument of a header inline is not the same as a local holding
  that value** (cost_factor 0x004E4430, propose_treaty 0x00544E40; found by
  the `inl` screen, reproduced at the site by `sitek.py`). `range(g, -2, 5)`
  with literals lets the front end fold the constants into the expansion.
  `int lo = -2; range(g, lo, 5)` passes a variable that the optimiser
  propagates later: the instruction count is the same or two higher, and
  what changes is how *other* values in the caller are materialized (the
  has_fac bit result kept as `neg/sbb/neg` and stored instead of tested in
  place; a field loaded into ebx before a call instead of folded into the
  later add). Symptoms: `fold` or `copy` windows away from the call, equal or
  +1/+2 instruction counts, no response to pads. A `const` local, a dead
  local, a named copy of a non-constant argument and an enum constant all
  compile like the literal. In propose_treaty nine of 25 literal sites give
  the image's code and the others do not move, so the image shows the effect
  but not which argument the original named: keep one, tagged
  `WORKAROUND(decl)`. `sitek.py` over the 290 functions up to 250 rows (2,458
  literal sites): two exact, Buffer::draw_pie 101 -> 34 (the zero `dx` of one
  `offset_rect`: `thickness` moves from its parameter slot to ebx), farm_click
  54 -> 45 (any of the `range(.., 1, 99)` bounds), boom 127 -> 114
  (`tile_in_view(x, y, 0)`), nothing else. Decided from the image
  (2026-11-19): farm_click and boom are kept (the rows that move are folds
  the image does not have, 0x40A60A..0x40A629 and the rectangle sums at
  0x504FDE / 0x5051CD / 0x5052C3; a dead local and a local passed to an
  out-of-line call are neutral controls; boom's is the only one of 14 sites
  that moves them); PullDown::on_redraw 24 -> 22 is rejected (only the load
  order of the two paddings shifts, the window stays wrong and the image
  holds no zero). Above 250 rows (42 of the 54 functions, 1,060 sites): no
  site hit. What moves there moves for EVERY site of the function by the
  same amount (base_build -4 at 113 of 113 sites, order_veh -3 at 87 of
  103, give_hints -5 at 6 of 6, draw_expenses -1), and a dead local gives
  the same result (give_hints: one dead local -5, two 0): that is the
  frame-symbol count (`locpad.py`), not this lever. Always run a dead-local
  control before reading a sitek hit in a large function.
  The inverse screen (`analysis/foldk.py`: each plain single-assignment
  literal local of a non-exact body replaced by its literal) finds six such
  locals tree-wide and every fold is worse (farm_click 45 -> 54, boom 114 ->
  129, draw_pie 34 -> 101, shoot 46 -> 175, shoot_it 95 -> 180,
  SocialWin::init 319 -> 335): no site where ours names a value the image
  folded.
- **A struct copied from an accessor that returns a reference**
  (MapWin::draw_base_names 0x00467DE0): `RECT saved = current_clip();` and
  `const RECT &clip = current_clip(); RECT saved = clip;` differ in the order
  of later, unrelated field loads (`pixel_x + field_1DDBC_` loaded as
  `[ebp-0x10]` then the field, five sites). Same family as S7: the named
  reference is a named value.
- **fe-exact is not reference-exact.** Two of this round's fe-exact results
  read SHAPE_EXACT under `recover measure` because fe masks relocations:
  mind_control had the operands of `energy_reserves + corner_market_active`
  swapped, draw_base_names read the InfoWin colour tables where the image
  reads ChatItemIdTable / ChatShadowColors. Run `analysis/refscan.py` (or
  `recover measure`) on every function that becomes fe-exact.

### 3.3 Frame and locals
- **A RECT member written through `this` while an address temporary of it is
  live** (2026-11-20 par-2, ReportIf::draw_ops 0x004A9050). When the image
  forms `&member` in a register (`lea edi,[esi+K]`, passed to calls) but
  reads or writes some of the member's fields as `[esi+K+f]`, those fields
  were spelled through the member (`draw_workspace_rect_.bottom = ...`) and
  the others through the pointer (`workspace->top = ...`). VC6 does not
  route a direct member access through the live temporary, so the mix in
  the image is the mix in the source; spell each access the way the image
  addresses it (draw_ops 14 -> 4). Changing one more access to the member
  can also drop a copy elsewhere (draw_ops' top store: 105), so measure each
  access separately. Rule (a) in 2.7 "How VC6 forms each kind" (a struct
  copy or `&member` forms the temporary) still decides whether it forms.
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
- **An `_itoa` buffer's frame home tells its scope** (par-2 2026-11-20). A
  say_num expansion's `text[0x50]` is a block-local that VC6 places at the
  frame bottom; two expansions share one home. A function-scope `char
  number[N]` is packed with other address-taken locals whose lifetimes do not
  overlap it. colortables_init 0x00423570: the explosions create loop's
  `_itoa(i, buf, 10)` has its own 80 bytes at [ebp-0x188] beside the shared
  say_num home [ebp-0x138] -- the function-scope `number[80]` (a block-scoped
  `name[80]` overlapped the say_num home: frame 0x50 short, 101 -> 37 rows).
  NewTechWin::exec 0x00483F90: the image's lowest object at [ebp-0x74] is the
  say_num home; ours wrote `_itoa` into a function-scope `number[16]` packed
  with two RECTs. Screen: `framescan.py --addr` rows `<-` with `&- x2/x4`
  at the frame bottom or a frame short by 0x50/0x4C.
- **colortables_init's transparent block** (open): a 12-byte `int[3]`
  {weights[0], weights[1], color} reproduces [ebp-0x48..-0x40] and the
  image's `regenerate` above it at [ebp-0x3c], but reorders the 32-byte and
  energy arrays (37 -> 88 rows). As separate `int[2]` + `int`, VC6 orders
  transparent_color/regenerate by reference count: one extra surviving
  `regenerate` read flips them (folded reads do not count).
  2026-11-20 par-2, current source (37 rows, only these two slots and the
  RGB_to_HSV argument order differ): none of these flip it, and all compile
  to the same 37 rows: `++regenerate`, `regenerate++` or `regenerate |= 1`
  at the armorgrads fallback or at make_explosions (both: the known-value
  load folds), `regenerate &= 1` after the zero store, and regenerate as the
  colorvals match flag (`regenerate = 1` on a mismatch then `if
  (regenerate)`, or `if (!matched) regenerate = 1;`: VC6 threads the flag
  away, as it does `matched`). Starting at `regenerate = 1` and clearing it
  on a full match does flip the order (the clearing store and test survive)
  but restructures the cache path (183 rows). So the missing reference is
  one that survives into code; the form that leaves the code unchanged is
  still unknown. A screen of every non-exact function for address-taken
  blocks of >= 0x40 bytes at different offsets (scratch `bufscan.py`,
  framescan slots) found no other `_itoa`/say_num buffer host: battle_fight's
  NERVEGAS name and say_num homes differ only by its 4-byte frame shortfall;
  queue_click, base_editor_proj, select_special and call_council differ in
  PopMenu/dialog object blocks, not number buffers.
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
- **A RECT dissolves into registers unless its left edge is assigned late
  and the shift goes through `offset_rect`** (par-5 2026-11-19,
  NetSessionPickerWin::initialize 0x0063BF10). The image keeps a 16-byte
  RECT in the frame (`sub esp,0x14`, left/right homed at +4/+0xC, top and
  bottom in registers). Written with `rect.left = ...` first and the shift
  as four `+=`, VC6 scalarised the whole RECT into registers and dead
  parameter slots (frame 8). Assigning left after the dx/dy computation and
  shifting with `offset_rect(&rect, dx, dy)` gives the frame and the
  image's right/bottom/dx order (173 -> 78 rows; a 1134-order deck of the
  ten statements ties at 78). Rung 3: an image RECT frame home with partly
  enregistered edges means the RECT's address reached an inline helper.
- **A member read back through an address-taking inline keeps an earlier
  dead store** (par-5 2026-11-19, same function 0x63C240). The image stores
  `tail_.rows_rect_.bottom` (`mov [esi+0x3758],eax`) and immediately
  overwrites it after computing `row_height_` from it; ours, written
  `(rows_rect_.bottom - k - rows_rect_.top) / rows`, drops the first store
  as dead. `rect_height(&tail_.rows_rect_)` (the header inline that takes
  the RECT's address) keeps the store and uses the value from the register
  (78 -> 68). An image store that a later store of the same field
  overwrites with no read between them is the trace of an address-taking
  inline (`rect_height`, `rect_width`, `offset_rect`) in the source.
- **Chained rectangles: an edge computed with `add r,K` / `lea r2,[r+K]`
  from a register that holds another rectangle's edge means the source wrote
  that edge from the other rectangle** (par-2 2026-11-20, ReportWin::draw_ops
  0x0049D540, 233 -> 170). The image forms `icon.top` as `lower.top + 3`
  (lower.top = split + 2 already in a register), `text.left = icon.right + 2`
  and `text.top = icon.top` from registers; ours, written with every edge as
  `split + k` from one shared value, folded the offsets onto split and
  produced a different register and slot map. Write each rectangle from the
  previous one (`RECT lower = {l, upper.bottom + 2, ...}; RECT icon = {l + 3,
  lower.top + 3, ...}; RECT text = {icon.right + 2, icon.top, ...,
  icon.bottom}`): VC6 /Og does not reassociate the folded `split + 5` back into
  `(split + 2) + 3` once the chain is broken, nor break a written chain. The
  counter-case on the same function: `icon.right = icon.left + 40` written
  inside the initializer or via `set_rectangle_dimensions` (right = x + w) is
  worse (170 -> 194/241), because the image's `add eax,0x28` there is a
  register reuse of `left + 3`, not a source chain. Screen:
  `analysis/chainscan.py [unit ...] [--kind f|m|x]` lists, per non-exact
  function, store -> `+K` -> store chains present on one side only. Tree-wide
  (2026-11-20) it lists 8 functions; the frame-to-frame ones besides
  draw_ops (draw_labs, AlphaSave::init, NetWin::exec, draw_farm,
  DiploWin::exec) are slot or register-reuse differences on inspection, not
  missing source chains (draw_labs' `field_A34_ = field_A2C_ + 0xAE` is
  already chained in the source; ours reloads A2C). Read each listed chain
  against the source before rewriting.
- **Address-taken local arrays: smaller size nearer ebp, then descending
  reference count, ties by first reference; declaration order, names, loop
  depth and dead code do not count** (par-2 2026-11-20, probe
  `analysis/probes/tempslot/g16.py`). Three `int[8]` arrays: reordering the
  declarations or renaming leaves the frame unchanged; taking C's address
  first puts C nearest ebp; counts 1/3/2 order them B, C, A. The count is
  taken after CSE (`A[1] + A[1] + A[1]` is one reference; three distinct
  elements are three) and before strength reduction does not add a loop's
  subscripts (a loop store `B[i] = ...` adds nothing, a reference inside a
  loop weighs like one outside). Constant-index element stores do not count
  where an aggregate initialiser counts one. Size wins over count: `int
  S[4]` (1 reference) sits above `int L[8]` (3), `int E[3]` above `int
  L[8]`; but colortables_init's uninitialised `int[3]` energy arrays sit
  below its 32-byte ones in both image and ours (not yet explained).
  colortables_init 0x00423570: the image's 32-byte order is
  territory_weights, body, territory_colors; ours is body, territory_colors,
  territory_weights with every final-listing count equal to the image's
  (body 5 named, the other two 4). One extra read of territory_weights
  anywhere moves it to the top (an inserted dummy read: 129 -> 115 rows), so
  the original has one more counted territory_weights reference that the
  final code does not show. Tried without effect: declaration order, names,
  a named weight temporary in the territory loop, `&territory_weights[0]`,
  a named value in the boundarypercents loop, eight element stores instead
  of the initialiser, and duplicating the make_territory body (213 rows).
  Resolved (par-2 2026-11-20, 129 -> 38): the two transparent weights are
  one `int transparent_weights[2]` (only [0] is compared and cached). The
  8-byte array takes the slot above the 32-byte ones and the 32-byte and
  energy orders then follow without any extra territory_weights read; the
  same change stops VC6 strength-reducing `i * second` in the transparent
  loop (the image keeps `i` in edi and does `imul eax,[ebp-0x44]`: an
  array element is aliased memory, a scalar whose address is taken through
  a propagated pointer is not). Dead reads, `if (0)` uses, self-stores and
  `sizeof` add no counted reference; a folded `if (array)` does but emits
  `lea/test/je`. So when a "missing reference" decides an address-taken
  order, look for a scalar neighbour that was really an array element.
- **An expanded has_fac's offset/mask/is_built slots are the host's packing,
  not the inline's spelling** (par-2 2026-11-20, draw_supported 0x0040C850
  175 rows, draw_production 0x0040E360 137). Image vs ours: draw_supported
  offset -0x74/-0x88 (mask -0x78 both); draw_production offset -0x50/-0x6c,
  and the image's dead is_built store joins offset's slot (cell_width +
  offset + is_built at -0x50) where ours puts it in row's slot (-0x2c).
  Eight body spellings tested in place of the header body (mask declared
  first, `int offset, mask;`, is_built declared before bitmask, the
  offset/mask pair in an inner block, `& (uint8_t)mask`, `if (is_built)
  return` split from `!queue_count`) give identical code in both hosts;
  `if (built & mask) return true` (no is_built), `bool is_built`, offset
  reused as is_built and a named `Base *` are worse in both. No has_fac
  header hypothesis; the slot differences are host-level reference-count
  questions (packrev.py on draw_production: is_built's slot is the only
  has_fac unit off).
- **Two same-type locals in disjoint block scopes share one slot**
  (on_scrolled's two shifted rectangles in a 0x10 frame).
- **Slot sharing follows the source's variable structure: read the image's
  homes, then find the declarations that share or split them the same way**
  (par-1 2026-11-19, `analysis/varset.py`, `analysis/imgmerge.py`). For each
  value, list the image slot it is stored to and reloaded from; two values
  in one image slot are two variables with disjoint scopes (or one reused
  name), one value in two image slots is two variables. Two shapes closed
  rung 5 outright:
  - *A census loop variable sharing a slot with a later row variable.*
    BaseWin::draw_facilities 0x0040FCC0: the image puts the census counter
    `facility` and the drawing loop's `row` in one slot (-0x10) and `count`
    with `y` (-0x24). Ours declared `facility` at function scope with a
    `while` loop, so it had a home of its own and the slot ranks shifted.
    `for (int facility = 0; facility < 134; ++facility)` with one ternary
    test (`facility < 70 ? has_fac(..) || redundant(..) : base_project(..)
    == base`) scopes it to the loop, it packs with `row`, and the function
    went fe-exact (recorded BYTE_EXACT). A function-scope loop counter
    whose image slot is shared with a later variable is a `for`-scoped one.
  - *A chain of section bounds through one `next` variable.*
    NetWin::draw_session_info 0x00480900: from the rules line on, the image
    homes each wrap result at one slot ([ebp-0x14]) and reads every later
    section's top from it, closing each section with one store back
    (`mov [ebp-0x14],ebx`), while `row`'s own slot is dead over those
    sections and reused by another local. Ours copied `bottom` into `row`
    at each section end (`row = bottom`), so `row` stayed live and one home.
    The original shape is `top = bottom` implicit: each section draws at
    `bottom`, computes `next`, then `bottom = next`; `row` is written once
    more after the last section. Slot map one-to-one with the image, 137 ->
    79 rows. A per-section `row = bottom` copy in ours where the image
    keeps writing one home is this chain.
  - *A value homed until a dead `test` is live until that test.*
    SocialWin::init 0x004B2740: the image stores the screen-width flag to
    its own home ([ebp-0x2C], 0x4B281B) and reloads it for a branchless
    `test eax,eax` right before the first Font::init call (0x4B2FB0). Ours
    (a size ternary on a constant local) kept the `setne` but lost the
    test, so the flag died at once, took faction_id's dead parameter slot,
    and pushed a loop counter into memory. A ternary on the receiver,
    `(wide ? font1_ : font1_).init(...)`, keeps the test where the image
    has it (both arms merge after the pushes, the branch goes, the test
    stays) and the flag in its home: 319 -> 268 rows. An if/else of two
    identical calls keeps the home but not the merge (372).
  - *A seed copied into a working local is one variable.*
    BaseWin::draw_production 0x0040E360: `int columns = turn_cost;` after
    the last use of `turn_cost`. The image keeps cost_factor's result in edi
    from 0x40EB31 through the column search and homes it at [ebp-0x28]
    (shared with the vehicle arm's `display`); one variable `columns`
    assigned from cost_factor and used for the gross cost gives slot-map
    difference 21 -> 8 and 163 -> 149 rows. `webscan.py` lists such copies
    as `COPY REGCOPY ... handoff`; most are neutral, try each.
  - *Read the member, not a local copy or a pointer alias.* BaseWin::init
    0x0041E8D0, 90 -> 12 rows in three edits: `RECT *production =
    &production_content_rect_;` let VC6 CSE `rect_height(production)`
    where the image re-reads bottom (0x41ED79); after `r = member;` the
    image forms `r.top = member.bottom + 3` from a member load and keeps the
    copy's bottom store (0x41EEB6), where `r.top = r.bottom + 3` forwards
    the copy; and one `rect_height(&rect6_)` among written-out siblings
    loaded bottom first where the image loads top first (0x41FB8D). A
    statement-order permutation (`perm.py`) then fixed the last block.
  Not every rank difference is structure. ProdPicker::draw_item 0x004927C0
  (id/color/veh_id ranked color > veh_id > id in the image, id > color >
  veh_id in ours): the image spills `item - first` and the row's y into
  color's and veh_id's homes at entry (0x4927F7, 0x49280F) because ebx holds
  column_count there and `item` is reloaded into ebx only after set_clip
  (0x492864); ours gives ebx to `item` from the prologue and spills
  nothing. Spellings of the rectangle (inline args, named w/h, field
  stores, an inline helper with the bounds test) all compile alike; the
  cause is the register decision for `item` (2.7), not the declarations.
- **A RECT packs with a later address-taken array** (par-5 2026-11-19,
  `analysis/probes/rectpack/run.py`). A RECT local whose address is never
  taken out of line shares its frame slot with a later `char[16]` whose
  address is passed to a call, whenever their live ranges do not overlap:
  declaration order, function or block scope, a `RECT *` alias, an inline
  helper taking `&rect`, a whole-struct copy out and a loop around the array
  alone all keep the shared 16-byte slot. Only two shapes keep them apart (a
  32-byte frame): a loop around both uses, or an out-of-line call taking the
  RECT's address. So an image frame with the RECT and the buffer in separate
  slots means one of those two, not a declaration change; a shared slot in
  ours where the image has two means our source lost the loop or the call.
  NetDaemon::synch's block-local `stage` pointer reached the image frame by
  sharing one loop counter between source and destination where the image
  has two: an accidental match, not taken.
- **One name reused for unrelated values costs a slot or reorders slots,
  even when the live ranges do not overlap** (par-3 2026-11-19). VC6 gives a
  named local one home per name; two disjoint webs under one name are
  written back and reloaded as one variable, and the second value's slot
  sits where the first's did. Decompiler-style sources reuse locals freely
  (iVar/local_ renamed, reuse kept); the original often had a separate
  name. BaseWin::draw_expenses 0x00412AA0 parked `line.left` in the
  screen-width flag `wide`: a separate `left` moved our slots onto the
  image's one-to-one (1212 -> 1078 rows).
  **Split only on image evidence** (par-5 2026-11-19): a store of the second
  value to a home the image uses for nothing else is a separate variable; a
  loop counter written to ANOTHER variable's home is an original reuse and
  must stay. InfoWin::setup_text 0x00456300 stores its land-census counter in
  the viewing `faction`'s home (prologue store at 0x456312, `mov [ebp-0x7C],1`
  at 0x457691): the `owner` split cited here before (frame 0x50C -> 0x508)
  was a frame gain inside the wrong register equilibrium; restoring the reuse
  once the rows were counted with `k++` gave 1309 -> 934 rows, because the
  reassignment is what keeps `faction` memory-homed. `analysis/varslot.py
  <unit> <addr> ''` lists, per named local, the image slots its aligned
  stores use; `analysis/varmerge.py` fes `v>w` merges.
  Not every reuse matters: draw_expenses' reused count variable was
  neutral, so judge each split by frame and slot map. The converse is the
  use_ability rule in 2.7 (Console::use_ability 0x004D7F60): sometimes the
  original had ONE reassigned variable where ours copies into a second
  local. `analysis/webscan.py` lists both shapes per function.
- **A block-local that the image keeps in a function-scope variable's slot**
  (par-4 2026-11-19, `analysis/reusesweep.py`, measured tree-wide). The other
  direction of the rule above: a short-lived counter, flag or scratch value
  that our source declares in an inner block, and that VC6 gives a
  callee-saved register (or a slot of its own), was often written by the
  original into an existing function-scope variable that is dead over the
  whole block. The value then lives in that variable's slot, the register
  goes to a loop's pointer IV or another web, and the slot map moves onto the
  image's. Only rename into a variable that is provably dead over the
  block-local's entire scope: reusesweep.py proves it with the compiler (a
  `volatile` variant gives slot liveness; the kill probe `T = K;` at the
  declaration must compile away), because a source-level guess clobbers live
  values (enemy_move's y2 -> n and healthy -> n renames read well and were
  wrong). Tree pass over the non-exact functions of 100+ rows (par-3's
  functions skipped): of 108 functions, 44 had no block-local/target pair,
  16 had no dead target, 35 were neutral or worse, 13 improved and were kept
  (proposal_menu 129 -> 86 with the slot map exact, has_base in
  commlink_count; NewTechWin::exec frame 0x10 -> 0x0C off; consider_designs
  1155 -> 1059; gen_terrain_poly 1648 -> 1578; base_terraform 1050 -> 953;
  Path::find 969 -> 895; air_power 1195 -> 1130; base_ecology 280 -> 242;
  capture_base 327 -> 294; TutWin::tour 302 -> 276; propose_pact 401 -> 388;
  SocialWin::init 330 -> 319; random_events 328 -> 327). Each kept rename
  improved the slot map or the frame, or lowered rows at an unchanged slot
  map with no worse candset. The screen and the probe disagree in a few
  pairs (enemy_move: 16 of 450 screen-dead pairs kept their store), and those
  are dropped, so the tool under-reports rather than clobbers.
  Second pass after the rebase (2026-11-19, every non-exact function of 40+
  rows except other workers' and enemy_move): 189 functions, 86 with no
  pair, 11 with no dead target, 76 neutral or worse, 16 kept, no exact
  function lost (rows as measured on the wip of that day). Of 115 functions
  of 100+ rows 14 were kept: random_events 328 -> 92 (slot difference
  65 -> 8, one rename `tx` -> `mode` on top of the first pass),
  battle_compute 131 -> 90, base_terraform 953 -> 922, CouncWin::init
  251 -> 237, editor_natural 128 -> 116, battle_fight 2147 -> 2138 (frame
  12 -> 8 bytes off), battle_plans 118 -> 113, setup_player slot difference
  30 -> 16 at equal rows, and four of 1 to 7 rows. Of 74 functions of 40-99
  rows 2 were kept (interceptor 80 -> 77, draw_active 96 -> 93). Two kept
  renames trade rows for the slot map (base_ecology 174 -> 184, slot
  difference 45 -> 41; Buffer::write_multi_font_raw_l 104 -> 110, 8 -> 7):
  the tool ranks the slot map above the candidate set and the rows, so read
  those two as open. Two more (veh_scoot, diplomacy_check; one or two rows
  at an unchanged slot map) stopped improving when the tree moved under
  them and were undone: a rename kept on rows alone is only as good as the
  base it was measured on. The yield falls with size because a small
  function has few function-scope variables that are dead where a
  block-local lives.
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
- **Slot sharing follows live ranges, not scopes or inline boundaries**
  (par-3 2026-11-19, `analysis/probes/slotshare/`, high on probes). After
  register allocation VC6 packs every memory-resident value whose live range
  is disjoint from another's into the same slot: a spilled scalar (no address
  taken) goes into the slot of an inlined callee's address-taken local
  (`bitmask(id, &offset, &mask)` in has_fac), of an earlier spilled loop
  counter, or of a dead parameter, and the image's out-of-line has_fac
  (0x00421670) even puts its address-taken `offset` into the dead
  `facility_id` slot. Two address-taken locals with disjoint lives stayed
  apart in every probe (`run.py` for_addr). Where several slots are free, a
  spilled counter took the inline's local's slot ahead of an earlier
  counter's (`pick.py`), which is what ours does in draw_intel 0x004A2210
  (`faction` on has_fac's `mask`); the image puts `faction` with the earlier
  loop counters and keeps `mask` alone (frame 0x9C vs ours 0x98), so in the
  original `mask` was live where `faction` is, or `faction` was not a fresh
  spill. Sharing one function-scope counter among the base/category/faction
  loops, or declaring `faction` at the top, leaves the frame at 0x98.
  NewTechWin::exec 0x00483F90 is the same family (`number[16]` packed with
  the RECT locals in ours, alone in the image: frame 0x64 vs 0x74). Not
  solved; the lever is the live range, not the declaration's scope.
- **A 4-byte aggregate is never enregistered** (par-3 2026-11-19,
  `analysis/probes/aggreg/`, high). A union of DWORD/WORD (any member order,
  `WORD half[2]`, int/short, named or anonymous), a struct of two shorts,
  `*(WORD *)&dword` and bitfields all stay in a frame slot under the game and
  the uv flags, with or without an `__asm` helper in the function; the
  scalar spellings (`p = h; p <<= 16; p |= h`, masks) fold to
  `and/shl/or` and are hoisted out of the loop. VC6 writes a 16-bit register
  only to load a WORD variable (`mov dx, word ptr [m]; and edx, 0xFFFF`). So
  sub_5cd560 0x005CD560's per-iteration `mov dx,ax; mov bx,dx; shl ebx,16;
  mov bx,dx; mov edx,ebx` (not hoisted, partial-register merges, the counter
  and stride spilled to dead parameter slots) is not a C spelling we can
  find; read it as a hand-written `__asm` loop body (AGENTS.md asm policy)
  before trying more union forms.

- **Slot order follows weighted priority** (par-5 2026-11-19, base_build
  0x004F81A0). Memory-resident locals get their frame offsets in the order of
  the same `n_b * 2^depth` reference weight that ranks register candidates
  (2.7, rule 4), not in declaration order. A slot that sits at another offset
  in ours than in the image therefore means one variable has too many or too
  few weighted references: before reordering declarations, compare the image's
  slot at each read with the variable our source reads there. base_build read
  `l_34` (the supply-convoy counter that shares `w`'s slot by live-range
  packing) where the image read `w` at 0x4FC356; reading `w` restored its
  weight and the slot map (4821 -> 3657 rows).

- **A dead-looking store to a frame home in a small inner loop is a
  function-scope scratch variable** (par-5 2026-11-19, air_power 0x005678C0).
  The explore scan stores the wrapped neighbour x/y of its 9-tile loop to the
  homes of `target_x`/`target_y` (0x567C6B, 0x567C70) and never reloads them
  there: the scan used the function-scope pair as scratch, so the stores stay
  live (the variables are read later on other paths). Block-local `x_k`/`y_k`
  live in registers only (1059 -> 1023 rows).
- **Declaration order of function-scope scalars does not set frame offsets**
  (par-5 2026-11-19, setup_text: mode/pane at -0x10 against a pointer temp at
  -0x14, row_pitch/width/base among -0x88..-0x90; declaration order, block
  scope and merges tried, none moves them). Offsets follow weighted priority
  (previous entry); leave a pure slot permutation once the homes hold the
  right variables.
- **Parameter copies: declaration order decides which home each copy takes**
  (par-5 2026-11-19, Buffer::write_multi_font_raw_l 0x005DCAE0; an exception
  to the entry above and to "packs locals into dead parameter slots in an
  order declarations do not control"). A local initialised from a parameter
  (`int x = x_coord;`) packs into that parameter's dead home only if no other
  local's definition precedes it while the parameter is still read. A
  parameter that a LATER copy still reads is live at an earlier copy's
  definition, so the earlier copy cannot take that home and, by the last-fit
  rule below, takes the most recently created free home instead. The image
  keeps `str` in `text`'s home and `x` in `x_coord`'s (0x5DCAF0, 0x5DCAF9):
  with `int x = x_coord; LPSTR str = text;` x interferes with `text` (still
  read by str's initialiser), str then fits `x_coord`'s home, and x opens a
  slot of its own; `LPSTR str = text; int x = x_coord;` gives the image's
  homes (104 -> 81 rows with the strchr fix). Read the image's homes of the
  copies and order the copy declarations so that each parameter's last read
  is its own copy's initialiser.

- **Frame offsets follow the packed slot's total, and spilled induction
  temporaries join a named variable's free slot** (par-4 2026-11-19; which
  free slot is the next entry's last-fit rule, not the first free one), enemy_move
  0x0056B5B0, `analysis/varset.py`). The image's 49 scalar slots are in
  descending order of accesses (-4: 253 ... -0xC4: 4), counted per slot
  after packing. VC6's own spilled temporaries (strength-reduced table
  pointers and counters of the radius and base loops, `mov [slot],0 ...
  mov eax,[slot]; add; mov [slot],eax`) have no slot of their own: each goes
  into a named variable's slot that is dead over its loop. In the image
  they sit in `j`'s slot (12 sites), the garrison/ring slot -0x48 (10),
  `tile_owner`'s (9), `k`'s (7), `hint`'s (8); in ours 68 of them share
  `tile_owner`'s slot, which therefore ranks 9th (image: 23rd), and `j`
  keeps a slot alone (ours 18th, image 11th). `j` cannot take them in ours
  because it is live across three of those loops (written L5822, L6589 and
  L7584 and read after the scans), where the image's `j` slot is free
  (its temporaries at 0x56D9F9, 0x56E365, 0x56FBD5, 0x572739, 0x57602D lie
  outside j's ranges 0x5729B7-0x57419F etc.). The lever is the named
  variable's live range, as in the slot-sharing rule above; a slot-rank
  difference of several places in varset.py's OUR LOCALS table names the
  variables to read. A heavy spilled temporary is itself a register
  symptom: the drop-pod base loop (0x56E6D8) keeps its `Bases` pointer in
  ebx and `d` in esi in the image, ours gives ebx to `best_id` and esi to
  `faction_id` and spills the pointer (same family as the entry's `T`).

- **Frame slots are packed last-fit in descending raw reference count; offsets
  then follow each slot's total** (par-2 2026-11-19,
  `analysis/probes/tempslot/` g2-g12, `analysis/packsim.py`, high). The model:
  1. Every memory-resident symbol is one packing unit: a named local, a
     front-end temporary (`$T..`), or ONE web of an optimizer temporary (a
     spilled strength-reduced pointer/counter; the temporaries of two loops
     are two units even when both loops count with the same variable, g12).
  2. Units are taken in descending RAW reference count (frame accesses; loop
     depth does not weigh: a depth-3 temporary with 4 accesses ranks exactly
     like a depth-1 one against a named X, g6/g7/g9).
  3. Each unit joins the MOST RECENTLY CREATED slot none of whose occupants
     interferes with it, else opens a new slot. Not first-fit and not the
     smallest slot: X(10) and Y(6) overlap, Z joins Y at count 2 and X at
     count 8 (g3); Z joins Y's slot even when Y's slot total (13) exceeds
     X's (11) (g4).
  4. Frame offsets are then assigned by slot TOTAL (sum of occupants'
     counts), largest nearest ebp. Address-taken locals keep creation order
     (g8: X(17) above a 36-access group of block-locals).
  5. Interference is liveness of the HOME, by flow: a value reloaded into a
     callee-saved register for the rest of its range leaves its home dead
     (p3 in g5 hosts a later block-local while p3 itself is still used from
     ebx), a reassigned variable's dead stretch is free, a goto path keeps it
     live, and an uninitialised read keeps the home live from entry.
  On enemy_move 0x0056B5B0 the model reproduces our compiler's slot of 105 of
  124 units (2025 of 2095 accesses; first-fit 69, smallest-total 91), and our
  offsets have 12 order inversions against slot totals (946 pairs) against
  118 against founder counts. Consequences:
  - A light unit (most temporaries) lands in the slot of the LIGHTEST
    compatible earlier variable, which then gains its accesses and climbs in
    the frame: ours piles 64 temporary accesses into `tile_owner`'s slot
    (founder 14 references, the last-created slot free over those loops) and
    ranks it 9th; the image ranks it 23rd.
  - Reading the image backwards: a temporary in slot S while a deeper slot D
    is free over its whole web (check with `live.image` and the web's own
    liveness, not the slot's linear range) means D was created before S, so
    S's founder has FEWER references than D's. The image's j slot (-0x30)
    takes the 0x56D9F9 radius temporary while -0x54 (m) and -0x60 are free
    there: its occupants are light variables, not one 59-reference `j`.
    Giving the automatic-terraformer flag (`j` over L6616-L7060) its own name
    took enemy_move 5790 -> 5702 rows (slot-map difference 136 -> 134);
    splitting every `j` role (5973), or other single roles with it (5711-5814),
    lost.
  - So a slot-rank difference is a reference-count question twice over:
    which units are light enough to pack late, and which slot totals they
    inflate. `packsim.py <unit> <addr> --why` prints the model's slots, the
    offset-order check and, per temporary, the deeper slots and the
    occupant that blocks each one in ours.
  - **Interference is live AND defined, not plain flow liveness** (par-2
    2026-11-19, probe `tempslot/g13`). A variable stored on one arm and read
    later under the same test is maybe-uninitialised by flow, yet its home is
    free on the other arm: an address-taken block-local there shares its slot
    (`maybe_uninit`), and two variables stored on exclusive arms and each read
    after the join share ONE slot (`exclusive`: A(2)/B(2)). This corrects
    rule 5's "an uninitialised read keeps the home live from entry" (true only
    where a store of the variable reaches). Consequence for reading the image:
    one slot whose def-use web carries two of our names is NOT proof of one
    variable. Console::go_to 0x004D61A0 slot -0x18 (`base_id` stored at
    0x4D621C, read 0x4D778C/0x4D793A; `max_range` stored 0x4D6C78, read
    0x4D7308; both stores reach every read by flow) is two packed variables:
    base_id's store never reaches max_range's, so they do not interfere.
  - `analysis/packrev.py <unit> <addr>` runs the model on the IMAGE (units =
    image webs labelled with our variables by the fe alignment, split where
    labels change, live-and-defined interference) and greedily searches
    split/merge moves that reproduce the image's slots. MUST-MERGE (two names
    whose homes interfere yet share an image slot) and SPLIT ACROSS SLOTS are
    the certain findings; the search moves are screens (alignment labels are
    noisy on big functions, and ties between equal counts are not modelled).
    esp-frame functions (most of jackal/caviar) have no ebp slots to read.
  - **Equal counts: the unit whose LAST reference comes first is created
    first** (par-2 2026-11-19, probe `tempslot/g14`). X and Y with equal
    counts overlap, a light Z joins the later-created one: declaration order,
    nesting (outer/inner scope), the order of `take(&X)`/`take(&Y)` and the
    first use do not decide; a sandwich `use(X); Y x5; X x4` creates Y
    first. packsim.py/packrev.py now break ties this way (`--tie last`,
    default; `--tie actual`/`slot` is the old oracle): packsim agreement on
    our own compiles enemy_move 102 -> 117 of 125 units (2071 of 2097
    accesses), ReportIf::draw_ops 13 -> 18 of 18, setup_text 24 -> 30 of 34,
    base_ecology 21 -> 23, process_message 12 -> 13; first-reference order
    is worse on all of them. Reading the image: two block-locals of one name
    in one image slot are usually ONE function-scope variable whose summed
    count packs it before the loop temporaries (ReportIf::draw_ops: the
    vehicle and facility `count`s, 2 + 4 references, slot-map difference
    8 -> 0).
  - **A spilled temporary interferes over its reload tail; boom's residue is
    two light temporaries, not a third tie key** (par-2 2026-11-20, boom
    0x00504AA0, screen only, no probe yet). The &pixel_y[slot] address CSE
    is reloaded into eax and stored through after the new pixel_y value was
    homed, and the two never share a slot: interference of an optimizer
    temporary is its VALUE's liveness (home live range plus the scratch
    register stretch after each reload), named variables keep home liveness.
    packsim/packrev now model it (`live.Func.reload_tail`, `--no-tail`
    restores the old model): boom 18 -> 20 of 27 units, enemy_move,
    draw_ops, setup_text, base_ecology unchanged. With it, boom's remaining
    misses are all explained (24 of 24 scalar units) if two units pack
    after every count-2 unit: the inlined zoom_scale ?: result homed at the
    join (T-0x8@4f2, 2 accesses, one a memory operand `sub ebx,[ebp-8]`)
    and the frame loop's strength-reduced &rects[slot] pointer (T-0x8@e0b,
    4 accesses: init store, reload, `mov/add 0x10/mov` step). No first-,
    last-, span- or IL-reference-count order reproduces both arms (arm 1
    needs the outer value temp created first, arms 2/3 the inner one); a
    probe should test whether a ?: join result and an IV pointer count as
    lighter than their frame accesses.
    Probe `tempslot/g15` (2026-11-20): they do not. An inline ?: result spilled
    on each arm (two arm stores + one reload) packs as 3, a loop's spilled
    struct-pointer IV (init, two reloads, step store) as 4: both exactly their
    frame accesses. The probe never produced boom's single store after the
    join (0x504F97), so that form's count is still open; if it counts its
    pre-merge arm stores it would pack EARLIER, not later, than a count-2
    unit.
  - **Equal counts, one side must win: BaseWin::init 0x0041E8D0** (par-2
    2026-11-20, open). Image -0x10 = sprite(24) + window_height(3), -0x14 =
    load_inverts' dest pointer(4) + the six sprite-loop temporaries(3) + yy.
    The image needs window_height packed before dest (then dest, live inside
    window_height's range, opens -0x14 and yy follows it); ours packs dest(4)
    first. window_height must count 5 (4 ties and dest's last reference,
    L7163, comes first). Measured not to move it: dest as an index or IV
    temporary (four loop spellings), `= 0`/`= -1` initialisers (dead stores
    drop before counting), clamp `wh = avail; if (wh > 398) wh = 398;` (4,
    still a tie lost), ?: and raw-expression arms. The image has 3 frame
    accesses (the 0x18E store, the tail-merged eax store 0x41EE92, one read
    0x41F4C7 shared by `top` and the init argument), so the missing count is
    in references the final code merges away.
  - **g14 exception: two loop-carried founders, NetWin::draw_custom_rules
    0x0047F960 (par-5 2026-11-20, open).** Image -0x14 = line(6) + top(2),
    -0x18 = row(6) + left's temporary(2); ours has the same two groups with
    the offsets swapped (16 rows, nothing else differs). In ours line's
    last access (0x4B2, `line = text_get()`) precedes row's (0x4B9, the
    bottom `++row` store), yet row gets the shallower (first-created) slot,
    so "last reference first" in final-code order mispredicts our own
    compile: both live to the loop's back edge. Measured not to move it:
    declaring row or line at function top, `int row = 0` before line's
    first store (row's first reference first), `line = text_get()` moved
    before the rect updates (line's last reference earlier), a `while`
    loop, IL-only references (`line = line`, `row = row`, a dead read, an
    extra `line ?:` test: removed before counting), the for-increment
    `++row, line = text_get()` (loses the tail duplication, slots kept).
    A synthetic probe did not reproduce two homed loop variables (the
    char* stays in a register). Note that packsim's "model slot = actual"
    maps model groups to slots by majority, so it confirms grouping, not
    offset order: do not read its tie modes as validated on offsets.
  - **One variable for two exclusive-arm roles packs as one heavy unit**
    (par-2 2026-11-20, Console::go_to 0x004D61A0, verdict for par-4).
    Image -0x18 holds base_id (order == -1 arm, 3 accesses), max_range
    (order == 128 arm, 2) and has_fac's mask (-3 arm, 2); centre_x/centre_y
    get their own slots. Separate names make base_id a light count-3 unit
    that exclusive-arm packing (g13) drops into centre_x's slot, as ours
    does. Spelling max_range as base_id (one function-scope variable for
    both roles) puts both at -0x18 like the image: slot-map difference
    8 -> 6, rows 118 -> 119, packrev 32 of 33 units. The merged unit (5)
    is created before the count-3 centres, and centre_x/centre_y interfere
    with its max_range role in the 128 arm, so they open their own slots.
  - **Dead parameter homes are taken first, the parameter read first first**
    (par-2 2026-11-20, probe `tempslot/g17`, packsim default). Every unit, in
    the usual descending-count order, tries the homes of the dead parameters
    BEFORE any local slot: a light Z joins a dead parameter's home although a
    local slot that ended before Z is free (`x3_p1`, `x3_p12`), and of two
    dead parameters it takes the one whose read comes first (`pq_lone`
    against `qp_lone`; an early-return branch's read counts where it stands,
    `qret2`). Heavy or light, address-taken or not; interference is flow
    liveness (a read lexically after Z on a path Z never reaches does not
    block, `lex_goto`). packsim.py models it (`--no-params` = old model,
    `--param-order first|last|oldest`): agreement on our compiles battle_fight
    48 -> 53 of 58 units (last 45, oldest 43), Console::go_to 30 -> 46 of 51,
    boom 20 -> 25, enemy_move 111 -> 113; first beats last everywhere.
    Consequence: a dead home is never "left over". In battle_fight 0x00506AF0
    allow_interceptor (+0x1c, read 0x507007) is tried before odds_out (+0x20,
    0x507098): the heaviest compatible unit, `display` (11), takes +0x1c and
    the next one that interferes with display takes +0x20. Ours gives +0x20 to
    nerve_gas (8); the image keeps nerve_gas at -0x3c and +0x20 holds only
    light units (has_fac's mask 2, rx 5, the target_x>>1 CSE 4, the inlined
    battle_kill_chaff's killer_id 3), and att_bit (7, live from 0x5070D6) is
    not there either. So in the original some unit ranked between display
    and nerve_gas took +0x20 first: rx (its range lies inside display's). The
    count that ranks rx is NOT its frame accesses: spelling the withdraw loop
    with `rx = wrap_map_x(...)` and rx at every site (instead of `int x =
    ...; rx = x;`, which copy propagation turns into x everywhere, rx = 1
    reference) gives rx 9 frame accesses against display's 11, yet rx is
    packed FIRST (rx at +0x1c, display at +0x20, nerve_gas at -0x3c as the
    image, mask at +0x20 as the image; frame 0x134 -> 0x130, rows 2442 ->
    2768). So VC6's count includes a variable's register-resident references
    (after copy propagation), and the image needs display > rx >= nerve_gas
    in that count: the all-rx loop overshoots (rx ~13 references against
    display's ~12), the x-copy loop undershoots (rx ~6). Spellings that move
    one reference (`stack_put(.., x, ..)` at L_509923 through a copy, four
    `display |= any_visible` forms) change nothing. Open: which original
    spelling puts rx between them (the image's loop keeps x in esi with two
    `mov [ebp+0x20],esi` stores, one per wrap_map_x arm).
    The count is taken AFTER copy propagation (par-2 2026-11-20, 18-variant
    deck): `int x = wrap(..); rx = x;` with rx written at the first k loop
    sites (k = 1..6) compiles exactly like the x-copy base (rows 2442, frame
    0x134: copy propagation turns every rx use back into x), and `rx =
    wrap(..);` followed by `int x = rx;` at any point, `int x = rx =
    wrap(..)` or the all-rx loop compile exactly like each other (2768,
    0x130: x becomes rx). `rx = x = wrap(..)` = base; the copy after the
    loop body (`rx = x;` last) 2781/0x134. So a lexical (pre-copy-propagation)
    count is refuted, and no spelling with ONE copy direction lands rx between
    nerve_gas and display: the intermediate count needs references that
    copy propagation cannot fold (a second definition of rx, or uses on a
    path where the copy does not dominate). Note the image's L_509923 block
    also stores ry to its home (-0x34, 0x509965) where ours keeps ry in ebx.
    No general register-inclusive count reproduces our own compiles either:
    packsim `--count` (lexical source references `src`, register uses after
    loads/stores `reg1..reg4`, with or without temporaries) lowers agreement
    on battle_fight/go_to/boom/enemy_move from 53/46/25/113 units (frame
    accesses) to at best 49-51/46/24-25/95-103. Keep frame accesses.
  - packrev.py models the dead parameter homes too (2026-11-20, default;
    `--no-params`): an image parameter slot that is stored to is a
    pre-existing slot founded by the parameter (the reads only its entry
    value reaches, plus webs of the reassigned parameter's own name), tried
    before every local slot in first-read order; address-taken scalars there
    are ordinary units. Console::go_to (current source) reads 40 of 43 units
    right at H0; its residue is has_fac's mask (L2291) joining base_id's
    -0x18 (the search merges them) and the two L2606 front-end temporaries
    at +0x10 where the model tries +0xc (index's home) first. can_terraform
    0x00565320 has the same kind of residue: mine_count/solar_count/base_x
    sit in y/force_improve/x homes (+0x10/+0x14/+0xc) where the model, in
    first-read order, offers +0xc first; so "read first" in final-code
    address order is not always the original's home order (open).

- **A loop counter the image keeps in memory while a callee-saved register
  holds a per-iteration conversion** (par-2 2026-11-19, ReportWin::draw_ops
  0x0049D540). The image's specialist loops store the counter in [ebp-0x10]
  and keep `this ? &this->buffer : 0` (neg/sbb/and) in ebx across
  `best_specialist()`; ours formed the conversion first in a scratch register
  and gave ebx to the counter. Routing the sprite index through an inline
  helper (`table[type_of(n - i - 1)].draw(this, x, y)`, the twin of basewin's
  psych_specialist_type) moves the conversion into ebx and the counter into
  its home (319 -> 253). A named index local, a `Buffer *dest` local, or
  helper bodies with if/else, `<` or `/ %` were neutral or worse. Loop
  counters the image homes in another variable's slot (NetDaemon::
  process_message: the notify and name loops count with the function-scope
  `slot`, image [ebp+0x10]) are the reuse rule above, not packing.
- **Two equal radii squared** (ReportWin::draw_sat 0x0049F350): the image's
  `imul eax,eax / lea ecx,[eax+eax]` is closest to `2 * r * r` (45 -> 25);
  `rx*rx + ry*ry` with `ry = rx` folds to `add eax,eax`, and no spelling tried
  (helpers, locals, `<< 1`, `r*r + r*r`, mixed rx/ry) gives the `lea`.

- **One reused function-scope variable per image home** (par-6 2026-11-19,
  air_power 0x005678C0, `analysis/imgmerge.py`). A body transcribed from a
  decompiler declares one block-local per role (`unseen`, `pick`, `score`,
  `nearest`, `close` ...); the original usually had a handful of
  function-scope scratch variables that every block reused. VC6 gives one
  home per name and ranks the home by the sum of the name's references, so
  the image shows such a variable as ONE slot near the top of the frame that
  many of our low-ranked block-locals pair with. air_power: image slot
  [ebp-0x0C] (39 accesses, second in the frame, above `best` and `i`) is
  stored by the unseen count, both base picks, the tectonic and fungal base
  scores, the drop-pod weight and the bombing score; [ebp-0x24] by the explore
  x, three nearest-base distances, the best cost, the escort x and the bombing
  target x; [ebp-0x2C] by the explore pick, both `close` flags and `risky`.
  Three function-scope variables for those (and `best` for the threshold and
  two more distances) took the frame 0x44 -> 0x48 (the image's), 15 of 18
  slots onto the image's variables and 955 -> 753 rows.
  How to tell a reused name from VC6's own packing of separate block-locals:
  - **The rank decides.** Packed block-locals sit in a slot whose offset is
    that of its first, low-priority occupant; a reused name ranks by the sum.
    order_veh 0x005947C0 keeps `attached`/`cost`/`tries`/`arty_count` in one
    slot on both sides at the same rank (9th) WITHOUT a merge; merging them
    moved the slot to rank 5 and the slot map away from the image (294 -> 306).
    So merge when the image's slot outranks every single member, and leave a
    slot that already sits at the image's rank.
  - **Small helpers pack by themselves.** After the three merges above,
    `half_range` went into `odds`' home and `explore_y` into `i`'s exactly as
    in the image, with no rename. Merge the heavy slots first, re-read the
    slot map, and only then look at what is left.
  - **A parameter slot that hosts locals is packing, not reuse**, when the
    parameter is still read beside them: probe 0x0059F120 keeps `bonus`,
    `pop_loss` and `reloc_base` at [ebp+0xC] while `base_id` (the parameter)
    is read from a register in the same statements (0x5A3EEA..0x5A4026); ours
    packs all three there too, site for site.
  `imgmerge.py --plan` prints, per image slot, our function-scope and
  block-local variables whose aligned accesses use it; `--sites SLOT` prints
  the slot's whole timeline (image address, our aligned instruction, variable
  and source line: read it for the stores the alignment could not pair); the
  default run applies each slot's merges alone, then the improving ones
  cumulatively. Every rename is proved with reusesweep.py's kill probe (the
  owner must be dead at the block-local's declaration), so a refused member
  names a variable that is still live there in OUR source: either the image's
  owner is another variable, or our source keeps a value alive that the
  original did not.
  Yield of the screen on the other large functions (2026-11-19): it is a
  lever for bodies that still carry one block-local per role, not a general
  one. enemy_strategy 0x00561080 (122 named frame variables against 36 image
  slots): of 13 shared slots only the unit x ([ebp-0x40]) and the triad/row/
  cost scratch ([ebp-0x2C]) improve the slot map (102 -> 75); the other eleven,
  merged alone or together, move it away. time_warp 0x005AEDE0: one slot
  (best score / scan index / best quality at [ebp-0x1C], 150 -> 142), a
  second one on the next round (coast flag / path counter, 142 -> 132; the
  first round had measured it neutral, so rerun after every kept merge).
  Tree pass over the 89 non-exact functions of 120+ rows: 14 had two or more
  block-locals in one image slot, 5 kept a merge, all at an unchanged frame:
  spot_all 0x005B6260 (match flag / base id, then the stack id: slot map
  104 -> 70, 1357 -> 1222 rows), ReportWin::draw_ops (item cost / citizen
  loop counters, 41 -> 29, 371 -> 323), base_yield (scan index / best
  position, 39 -> 25, 263 -> 229), world_rainfall (altitude of both passes,
  251 -> 232), ReportWin::draw_labs (rate / scan index, 613 -> 605).
  order_veh, consider_designs, setup_text, social_ai, communicate,
  can_terraform, battle_fight, base_build, goody_box, prune_protos,
  base_terraform, base_production, base_ecology, setup_player,
  Console::on_key_click, config_game, DesignWin::draw_vehicle: nothing kept,
  their shared slots already sit at the image's rank.
  `--fscope` extends the proof to two function-scope variables in one image
  slot; its refusals name the lines where our source keeps both alive. In
  base_build 0x004F81A0 `w` and the convoy counter `l_34` share [ebp-0x34]
  in the image although our source updates `w` inside the convoy loop
  (0x4FC5EB..0x4FC649): the image holds `w` in ebx from 0x4FC371 on, so its
  home is free. A shared image slot that our liveness refuses is therefore a
  register finding (which variable the image enregisters there), not a
  naming one.
- **A scan whose candidate is stored to a function-scope pair every
  iteration and never reloaded there is a block-local pair plus a copy**
  (par-6 2026-11-19, battle_fight 0x00506AF0). The withdraw scan holds the
  candidate tile in esi/edi for all 8 directions and only stores it to
  `rx`/`ry`'s homes (0x509422, 0x50944D); with `rx`/`ry` used directly ours
  kept `ry` in ebx with reloads, `rx` in memory and the result `best_dir` in
  edi. `int x = ...; rx = x; int y = ...; ry = y;` with x/y in the tests gave
  the image's registers, `ry` sharing `intercepted`'s home as in the image
  and `best_dir` in memory (frame 0x130 -> 0x134 of 0x138, 2494 -> 1856
  rows). This is the converse of the air_power explore scan above (stores to
  a function-scope pair that stay live and ARE the working copy): there the
  image reads the homes back, here it never does.
- **`a = b = f()` stores b first, and reads of either name then use b's
  home** until one of them is reassigned (copy propagation). The image's
  store order therefore names the inner variable: battle_fight stores
  best_defender() to [ebp-0x18] then [ebp-0x40] and its bombard walker
  rewrites [ebp-0x18], so the statement was `stack_id = def_id = ...`; the
  reverse order homed every later `def_id` read in `stack_id`'s slot and gave
  the walker a third slot.

- **A 4-byte frame difference is often one register decision, not a missing
  variable** (par-6 2026-11-19; read the two slot maps with `framescan.py
  --addr` and the slot timeline with `imgmerge.py --sites` before renaming
  anything). Measured causes, none of them solved by a variable-set edit:
  - *ours spills a temporary the image keeps in a callee-saved register.*
    order_veh 0x005947C0 (+4): the bound of the inlined
    `rnd(turn / 5 + 100, "ORDER2")` lives in edi across `rand()` (0x595DB1,
    `idiv edi`); ours has edi on the `Vehs[target_veh]` offset (the image has
    ebx there and reloads `target_veh` after) and spills the bound to the slot
    that should take has_fac's `is_built` and a later temporary, which then
    need a slot of their own. enemy_strategy 0x00561080 (+4): `sites` stays in
    bl from 0x562D6F to 0x562E6C because the image takes edi/esi (cached,
    reloadable homes) as scratch for the tile counts; ours takes ebx and
    stores `sites`. social_ai 0x005B4790 (+4): one unnamed temporary.
  - *ours keeps in a register a value the image homes.* can_terraform
    0x00565320 (-4): `solar_tech` is stored to has_fungus' dead home
    (0x5659F0) and ebx is reused for `order` from 0x565A4A (`mov ebx,7; mov
    [order],ebx`, reload at the join); ours keeps `solar_tech` in ebx for its
    whole range and `order` in memory, so `mine_tech` takes has_fungus' home
    and `bonus` the inlined mask's. repair_phase 0x00526030 (-4):
    `repair_rate` is stored on the Nano Factory arm and multiplied from
    memory (0x526548, 0x526565) while ebx already holds `faction_id`; ours
    forwards it through ebx on both arms, so `reactor_id` packs into
    `repair_rate`'s home. battle_fight 0x00506AF0 (-4 after the two fixes
    above): the withdraw scan index is homed and cached in ebx with x/y in
    esi/edi; ours has x in esi, the index in edi and y in ebx, so
    `best_angle` packs into the index's free home. Writing both copies after
    both computations (`x; y; rx = x; ry = y;`) gives the image's frame and
    loop tail but the wrong x/y registers and store order (2405 rows against
    1856): an exact tie between y and the index, left open.
  The packing itself is first-fit over live ranges (see "Slot sharing
  follows live ranges"), so one extra or one missing memory value shifts
  every later low-priority local by a slot; fix the register decision and the
  slot map follows.
  Second list (par-6 2026-11-19, the rest of the framescan list; `d` = ours
  minus image). Each line names the value and the image addresses, for the
  register work:
  - custom_planet 0x0058C2A0 (-8): `&AlphaIniPrefs.custom_world[slot]` homed at
    [ebp-0x1C] (0x58C381, reload 0x58C765) and `&planet_rules_[slot]` at
    [ebp-0x14] (0x58C390, 0x58C79A); `slot` dies at the switch (`dec esi`,
    0x58C3BF). Ours keeps `slot` reloadable through the slots walker and
    re-forms both addresses, also when they are written as pointer locals.
    Resolved as a counter tie (2.7 "Homed addresses"): 31 header pads in
    alpha.cpp give the image's frame and 11 rows; the three homes are permuted.
  - Buffer::write_pcx 0x005DF590 (+4): the output cursor stays in esi from
    0x5DF6E2 to the end, `this` is in ebp, the pixel row in ebx (home
    [esp+0x10]), the column in edi. Ours has `this` in esi, the cursor in ebp
    AND homed at [esp+0x18] (ours +0x244, +0x2D3).
  - ListBox::on_key_down 0x0060AF90 (-4): `count` homed at [esp+0x10]
    (0x60AFBA) and reloaded per case (0x60B052, 0x60B08A, 0x60B17C, 0x60B1DE)
    with old_top in edi, old_selected in ecx and the Dialog base in edx across
    the switch jump; ours keeps `count` in ebx for the whole function.
  - BaseWin::init 0x0041E8D0 (-4): two address temporaries are homed,
    `&font1_` at [ebp-0x2C] (0x41F291, reloads 0x4209B8, 0x4209F1) and
    `&flatButtons_[0]` at [ebp-0x28] (0x41FBBD, reload 0x4209A9 into esi),
    because `this` dies at 0x420990 (`add esi,0xcaa8` in place) and the redraw
    loop takes esi for the walker and edi for the down-counter. Ours keeps
    `this` in esi to the end and homes the counter instead; a named
    `Font *font = &font1_` is folded back into `lea`.
  - MapWin::draw_map 0x0046A550 (-4): `dy` is homed at [ebp-0x28] (0x46A8BA)
    and reloaded after the horizontal fill (0x46AA0B); `x1` is spilled to the
    home that later takes `current_pixel_y` (0x46A6A0/0x46A6B9, reload
    0x46A6D7). Ours keeps both in registers (ebx, ecx).
  - ReportWin::draw_intel 0x004A2210 (-4): `subject` is in ebx (two stores,
    two loads at [ebp-0x1C]) and the `Players[subject]` byte offset is the
    spilled value ([ebp-0x10], stored 0x4A2456, reloaded 0x4A2485, 0x4A24F1,
    0x4A2F44; `shown` and `faction` reuse that home later). Ours gives ebx to
    the offset and keeps `subject` in memory (12 accesses), so `shown` packs
    with `priorities` and `faction` with has_fac's mask. This replaces the
    "mask live range" reading in the slot-sharing entry above.
  - social_ai 0x005B4790 (+4): ours forms the `PlayersData[faction_id]` offset
    temporary at its first use (ours +0x38C, [ebp-0x34]) and reloads it at
    +0x82A and +0xBEE; the image recomputes it at 0x5B4FAB and homes it only
    from there, in the home `j`/`effic_val`/`f` share ([ebp-0x18], reload
    0x5B5361).
  - NetDaemon::synch 0x00532E00 (-4): a loop-form difference (2.4), not a
    variable: the social category case (0x533EE7..0x533F64) keeps three
    induction temporaries in homes, the staging row pointer [ebp-0x28]
    (+0xD4), the destination prerequisite pointer [ebp-0x20] (+4) and the
    source effect pointer [ebp-0x24] (+0x2C), with the destination byte offset
    in [ebp+0xC] (`cmp ecx,0x350`) and the inner offset in eax
    (`cmp eax,0xB0`); ours runs the inner loop on one down-counter. A
    `stage` pointer walked like the neighbouring cases gave frame +4.
  - BaseWin::draw_expenses 0x00412AA0 (-4): one more temporary home in the
    image, [ebp-0x3C], shared by the `&production_outer_rect_` address
    temporary (0x412B23, reload 0x412E48), a never-reloaded copy of tile.top
    (0x413297, 0x413B2F, 0x4143FA) and the three sections' loop counters
    (0x4132CA, 0x413B62, 0x41442D); ours spreads the same temporaries over
    the homes of `left`, `mineral_cost` and `energy_cost`. `wide` and `left`
    share [ebp-0x8C] in the image by packing; merging the names in the source
    is worse (1024 -> 1301 rows).
  - caviar_apply_rotation 0x00627D00 (+4, VC5 x87): the shared `cz * sy`
    product stays on the FPU stack from 0x627D8A to the closing
    `fstp st(0)` (0x627E13); ours spills it. `sz * sy` IS a float variable
    in both (rounded through [esp+0x10]).
  - NewTechWin::exec 0x00483F90 (-12): `number[16]` sits alone at [ebp-0x74]
    and the two RECT scratch areas at [ebp-0x24] and [ebp-0x14]; ours packs
    `number` with `strip`/`inner` and keeps a spill at [ebp-4]. Changing which
    rectangle `inset_rect` writes, the array size or the declaration scope
    does not separate them (the packing takes any dead 16-byte area).
  - NetSessionPickerWin::initialize 0x0063BF10 (-12): `rect` is a RECT in
    memory ([esp+0x14], left/right stored at 0x63C06A/0x63C064, top in ebx,
    bottom in edi) and margin_y is in ebp. Solved (4aa216e1f5, 173 -> 78):
    see "A RECT dissolves into registers" below.
- **An allocated slot that nothing reads or writes, between homes of one
  type, is an array element** (par-6 2026-11-19, slide_show 0x0045D440).
  The image keeps the four sprite x positions at [ebp-0x38], [ebp-0x34],
  (nothing at [ebp-0x30]) and [ebp-0x2C]: the third value lives in ebx and
  never reaches memory, but its slot is still allocated. VC6 enregisters
  single elements of a small local array (or struct) and keeps the whole
  object's frame space; four separate scalars would have been packed without
  the gap and in priority order. `int frame_x[4]` gave the image's frame
  (0xB5C -> 0xB60) and 145 -> 105 rows. Read `framescan.py --addr` for holes
  inside a run of same-width homes before looking for a missing variable.
- **A scratch struct copied into several destinations with ONE store per
  field and a running value in a register was mutated between the copies**
  (par-6 2026-11-19, NewTechWin::exec 0x00483F90). `dst = scratch;
  dst.top = v;` stores top twice (the copy's store, then the patch; the image
  shows that form once, at category_rect_). `scratch.top = v; dst = scratch;`
  stores it once and keeps `v` in the register that carries the cursor from
  one rectangle to the next (`add edi,3; mov [ebx+4],edi`, then the member is
  read back because the store went through a pointer). 331 -> 297 rows.
- **A pointer formed once per iteration and homed, with the index dead after
  it, was a named pointer or an address temporary our allocation does not
  form** (custom_planet above). Writing the pointers lowers the rows
  (404 -> 369) without changing the frame when the index stays reloadable;
  the remaining lever is what keeps the index alive in ours.
- **One height, reassigned** (NetSessionPickerWin::initialize): a store of a
  second value to the home of an earlier variable, followed by reads of that
  home where our source names the first variable, is one reassigned variable
  (`button_height = rect.bottom - rect.top` twice, 0x63C086 and 0x63C0EF).
  Track pending pushes when reading esp-relative homes: the third init's
  width argument is the home of button_width (0x63C138), which an earlier
  reading had taken for the create button's height.
- **One name over two image homes is two variables** (par-6 2026-11-19,
  `analysis/splitaudit.py`, the reverse of the imgmerge screen). When the
  stores of one of our names land in home A over one line range and in home
  B over a later, disjoint range, the image had two variables: VC6 keeps one
  home per name. Three sources of such names, all found by the tree pass:
  - *a decompiler reuse.* battle_compute 0x00501DA0 kept is_bombard at
    [ebp-0x1C] and wrote the bunker/base/open-ground multiplier to
    [ebp+0xC], the home the terrain multiplier used just before: the second
    multiplier is the terrain multiplier reassigned, not is_bombard
    (90 -> 35 rows, every named home on the image's slot).
    consider_designs 0x00581260: weapon_id was the first loop's counter
    (image: weapon's home) and the upgrade arm's armour (a block-local's
    home).
  - *a reusesweep rename the image contradicts.* The sweep proves the target
    dead, not that the image uses it: base_ecology's scan tile in
    shown/report_terraform, base_terraform's special flag in rain_sum and its
    height step in best_region, CouncWin::init's done flag in proposal,
    gen_terrain_poly's direction counter in local_94. Undoing them lowered
    the rows in each (184 -> 174, 922 -> 945 and 893 with the altitude in
    rain_sum, 237 -> 258 and 227 with the next scan counting in proposal,
    1516 -> 1508).
  - *the right value in the wrong home.* The other occupants of home B name
    the variable: Path::find 0x0059A530 had the trace-back step cost in
    x_cur; the image's home also holds cur_cost, dead after the search loop,
    and the rename into cur_cost reads 876 -> 858.
  The slot-map difference of `imgmerge.py --plan` can go UP when a correct
  split is applied (consider_designs 70 -> 78, base_ecology 41 -> 45): the
  freed block-local is packed by first fit into another slot than the
  image's, and every access of it counts. Read the two homes' timelines
  (`imgmerge.py --sites A,B`) for the fact; use the metric and the rows to
  choose between spellings. Where both get worse the rename stays although
  the image has two homes (TutWin::tour's `dir` in `lower` 273 -> 299,
  capture_base 253 -> 281, probe's speed in morale 1292 -> 1811 with the
  frame off, setup_player's ring scan 16 -> 28): the home the image gives
  the value is then some OTHER variable's, not found yet.
- **imgmerge `--fscope` tree pass** (par-6 2026-11-19, 214 non-exact
  functions of 30+ rows, 69 shared homes tried). Function-scope pairs are
  rare and mostly already packed: kept air_power (region scratch / stack
  owner, slot map 83 -> 71, rows 1051 -> 1078), communicate (the
  [ebp-0x48] scratch, 46 -> 36, rows 1513 -> 1553), TutWin::tour (step /
  special flag, 273 -> 261 at an equal slot map) and setup_game (the two
  retry counters, 157 -> 151, slot map exact). load_vehicles' source_y /
  border_y already share the image's home without a merge (the merge reads
  97 -> 110). Refusals are register findings, the image holds one of the
  pair in a register where our source keeps both alive: BaseWin::production
  i/sel (sel live at L4467), colortables_init palette_changed/i, load_vehicles
  x/y (x live at L4737), InfoWin::setup_text k/best, base_build w/l_34,
  communicate's loan_factor, report_rival, tech_price and loan_amount against
  the other owner of their home.

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
- **Field and return types are not a tree-wide lever any more (census
  2026-11-19, par-1).** Three independent screens agree. `imgtypes.py`
  (image alone, 46,597 accesses of 1,596 fields of bound globals): no field
  whose operand width or extension contradicts its declaration; what is left
  is per site (a `(char)` cast: scenario_setup's `movsx` of `is_flat`,
  diplomacy_check's of a treaty word). `accdiff.py` (our compile against the
  image inside differing blocks): 91 access-form differences in 40 of the
  non-exact functions, no operand above 4 functions, and those are register
  rows. `retscan.py`: 2,636 callees, no caller consumes a result at another
  width or extension than the declared return type. What remained were 32-bit
  fields declared `uint32_t` by default and cast to int at their signed uses
  (`retype.py`); retyping them changes no user of the field. A cast dropped
  from a *header inline* does move functions that never touch the field:
  it shifts every includer's symbol counter, with one signature per shift
  (MainInterface::panel_height: PrefWin::init exact, probe 1250 -> 1299,
  enemy_move 5625 -> 5619, order_veh, goody_box, boom, give_hints,
  capture_base, battle_plans, draw_sat, setup_player). So even the giant
  non-exact functions carry counter-dependent ties; judge a header change by
  the functions that use what changed.
- **`field & K` with K in the second byte: three forms, none of them a
  spelling** (par-5 2026-11-19, replaces "the dword test is the helper").
  For a 32-bit field and a mask in bits 8..15 (`flags & 0x200`) VC6 emits
  - `mov r,[m]; test rh,2` when eax/ecx/edx has a free register at the test
    (the usual form);
  - `test dword ptr [m],0x200` when none is free and the test is NOT inside
    a loop the optimiser processed;
  - `test byte ptr [m+1],2` when none is free and the test is inside a loop
    (`for`, `while`, `do`, a backward `goto`; a `for` that always breaks is
    not a loop).
  Masks in the low byte always narrow (`test byte ptr [m],0x80`), masks in
  bits 16..31 never do. Measured on 60 probe functions: literal, enum, `!=
  0`/`== 0`, `!`, an int cast, a pointer or reference to the struct, an
  inline helper taking the mask (`hasf(i, 0x200)`) or the whole test, the
  field's signedness, a store to the same field in the loop and a call in
  the loop change nothing but the register pressure. In the real function
  ten spellings of spot_all's site (0x005B6A60) compile identically. Image
  census: PlayersData flags & 0x200 is 7 times the byte form and 6 times the
  dword form, both forms in spot_all, gen_terrain_poly and Console::zoom;
  exact prototype_factor, morale_alien and goody_at have the dword form from
  plain source, exact spot_stack and action_tectonic the byte form.
  So a dword-against-byte difference says the image's test sits in a loop
  VC6 did not treat as one, or the reverse (spot_all's first reveal loop:
  the image keeps `i` in its slot, reloads MapLongitude at the latch and
  has a dead `and edx,0x80001000`, ours strength-reduces `i`), and a
  load-against-memory difference is register pressure. Fix the loop or the
  variable set; do not respell the test. `accdiff.py`'s remaining rows of
  this kind (spot_all, Path::find, enemy_move 0x0056BCBD) are symptoms.
- **MapLongitude is `int`** (par-3 2026-11-19). The catalogue name
  `?MapLongitude@@3IA` (unsigned) is an inferred type: the image's 132 uses
  outside multiplies are x_dist's signed `cmp; jle`, none is unsigned
  (no `jb/ja`, `shr`, `div` or unsigned-to-float), and declaring it unsigned
  loses 37 exact functions and gains none (feall). The stride's signedness
  never changes code in `G * y` index multiplies.
- **Operand order of `global * value` is register allocation, not spelling**
  (`analysis/probes/mulorder/`, high). `G*y`, `y*G`, casts and an unsigned G
  compile identically. The form follows where y lives: y already in the
  destination register (a fresh, dying temp) gives `imul r,[G]`; a y that
  stays live gives `mov r,[G]; imul r,y` when the caller passed a *named
  local* to the inline, and `mov r,y; imul r,[G]` when it passed the *field
  expression* itself (CSE'd with its later use; `inl.py`). balance
  0x005B0420 is the latter: `map_loc(Vehs[v].x, Vehs[v].y)` and
  `veh_find(Vehs[v].x, Vehs[v].y, ...)` instead of `const int x/y` locals
  (108 -> 106). `mulorder/scan.py FEALL.json` lists functions whose counts of
  the two forms differ (22 on 2026-11-19); most (alien_fauna, enemy_move,
  Path::find) differ because y sits in a different register or slot, a
  register-allocation symptom (2.7), not a source spelling.
- **`lea` base/index order of `this + field` is a counter tie: the number of
  pointer memory references created before it** (par-5 2026-11-20,
  `analysis/probes/leaorder/run.py`, high). `&text_[field]`, `text_ + field`,
  `field + text_`, `&field[text_]` compile alike; whether the lea prints
  `[idx+esi+K]` or `[esi+idx+K]` is fixed per field for the whole function
  (every site of `this + field_B44_` agrees) and moves only with the memory
  references through a pointer (`this->f`, `q->f`; loads, stores, and ones
  the optimizer folds away such as `f = f` or the load of `f = 0; f |= 1`)
  that precede the field's first use: with one extra distinct load before,
  the pattern cycles with period 4 (TiTi, iTiT, iiii, iiii, TiTi; stores
  period 2). Global and parameter loads, local-struct stores, dead `{int t
  = f;}`, `if (f) {}` and code after the site do not count. Reading the
  image: a wrong base/index order with equal registers means the original
  made a different number of member references earlier in the function,
  usually one that leaves no code. Applied: EditBox::on_key_down
  0x00615840 `field_B38_ |= 1` (as the `|= 2` beside it; the folded load of
  the just-zeroed flag is the missing reference, 7 -> 3 rows).
  SpriteBox::draw_item 0x00610C50 needs the same (+1 or +2 references before
  `area.left`: exact with a stand-in self-assignment), no natural source
  found yet (operand orders, `win_parent_` reread, extent locals tried).
  The reassociation of `a + (b - c)` (which pair is summed first) is NOT
  this counter (ListBox::UNK1, EditBox 0x00615A24: 9 prefix counts, 5
  positions, no change). There, a direct member read in the inner pair
  (`item_width + (Dialog::field_40_ - (Dialog::field_40_ << 1))` instead of
  the `inset` local) keeps the image's grouping, with a register swap left
  (ListBox::UNK1 7 -> 8 rows, not applied).
  **What counts (2026-11-20 par-5, `probes/leaorder/forms.py`, phase mod 4
  from `X;T` and `X;u(z9);T`).** The counter is IL nodes, not only member
  references: a parameter `u(k+1)` is +3 and `if (k<0) u(1)` +3 although
  plain global and parameter loads are 0. Measured phases (none = 0):
  +1: `u(f1)`, `f1 = f1`, `u(get1())` (an accessor that returns f1), `this->`
  or a cached `E *p = this` alike, `q->f1`, `RC r = rc` (copying a member
  struct to a local), `u(f1+f2)`, `u(f1+f2+f3)`, a reload after a store
  `f1 = k; u(f1)`;
  +2: any plain member store (`f1 = 1`, `f1 = k`, `set1(k)`, `ref1() = k`,
  a dead store overwritten by a second one), member struct copy `rc = GR`,
  `u(f1 ? f2 : f3)`, `u(&f1 == 0)`, `{int c = k/f1; u(c); u(k-f1*c);}`;
  +3: `f1 = f2; f1 = k`, `rc = rc2`, passing `&rc` (`ur(&rc)`,
  `memset(&rc,...)`), `if (flag1())` (`f1 & 1` accessor), `u(k+1)`,
  `if (k > f1-1)`;
  0 (mod 4): compound `f1 |= 1`, `|= k`, `&= ~1`, `+= k`, `++`; `f1 = 0;
  f1 |= 1`; `f1 = f2 = k`; `u(f1 && f2)`; `u(f1 + k)`, `u(f1 * k)`,
  `u(f1 >> 1)`; per-field `rc.l = GR.l; ...` (four stores); dead reads
  (`(void)f1`, `get1();`, `{int t = f1;}`, `if (f1) {}`); a CSE'd reload
  (`u(f1); u(f1)` = `u(f1)`). So swapping a plain store for a compound one
  (`= 1` against `|= 1`) is a -2 step, a struct copy against per-field
  stores is +2, and member arithmetic folded into one expression differs
  from the same loads in two statements. Spelling-level changes do NOT move
  it (measured on SpriteBox::draw_item and FameWin::on_redraw: operand order,
  statement order of the four RECT stores (720 orders), `!x` against
  `x == 0`, `||`/`&&` of two early returns, commutative factor order,
  an inline helper against the written-out expression, `rect_width()`
  against `r - l`, `RECT area = {...}` (changes code), `set_rect()`).
  Self-assignments locate the residue but are invented code, never source.
- **The commutative load order of `this->a + this->b` follows the same
  counter** (par-5 2026-11-20). FameWin::on_redraw 0x00448D90 (2 rows,
  `field_A14_ + field_A1C_` loaded A1C first): +3 self-assignment stand-ins
  anywhere between the function entry and `field_A1C_`'s first store make
  it exact, +1/+2 do not, and nothing after that store does. In isolation
  (a probe with only the add) no prefix moves it: the counter is sampled at
  the field's first reference, so look for the missing node between the
  function entry and that reference. SpriteBox::draw_item 0x00610C50:
  +2 or +3 anywhere from the entry, or +1 after the two `seek_pos`
  expansions (SpriteListBase::seek_pos header variants: per-arm tail,
  `>=`, operand order, `for`, `i-- > 0` - none). No natural form found for
  either (the decks above, about 40 variants). Screen for the class:
  `analysis/leascreen.py FEALL.json` (non-exact functions whose only
  differences are lea base/index order or one block of the same
  instructions in another order): on 2026-11-20 only draw_item and
  CouncWin::draw_leader 0x00425DB0 (2 leas, councwin.cpp). tech_achieved
  0x005BB000 (`[edi+edx]` of two register locals) is NOT this counter:
  global self-assignment stand-ins at its entry do not move it.
- **The vtordisp store order of an inlined ~RadioButton is NOT the
  pointer-reference counter either; it follows the size of the whole
  function, in irregular plateaus** (par-6 2026-11-20,
  `analysis/probes/vtordisp/run.py`, medium). An expanded ~RadioButton
  (inside ~Dialogs inside a final ~Popup teardown) writes two vtordisp slots,
  `lea edx,[eax-0x18]; mov [X+Y-4],edx` and `lea edx,[eax-0xa30]; mov
  [X+Y-4],edx`; per store `T` = `[esi+eax-4]` (this first), `i` =
  `[eax+esi-4]`. Census of the 11 image expansions (`run.py census`): ii in
  DesignWin/DiploWin::on_button_clicked, process_message, tech_achieved; TT
  in go_to, base_production, tech_trade, propose_attack, config_game,
  order_veh; iT only in proposal_menu 0x0054DCF0. Ours agree in 9; tech_trade
  0x00541100 (ours ii) and proposal_menu (ours TT, its only differing row)
  do not.
  Measured: (1) it is function-local: preceding or following functions,
  `$SG` literals, externs and identifier lengths do not move it; (2) in a
  one-Popup probe it follows the function's total size, not the position of
  the code (N lines before or after the Popup give the same result), as
  plateaus of irregular length (N get_title lines: 74-75 ii, 76 iT, 77-119
  TT, 120-121 Ti, 122 ii, 123 TT, 124-130 ii, 131-154 TT, 155-185 ii,
  186-201 TT, ...), with the teardown's own expansions unchanged throughout;
  so it is not periodic like the lea counter (mod 4) or frame symbols (mod
  8); (3) a mixed pair appears only on a plateau edge, the second store
  switching first (iT on an ii -> TT edge, Ti on TT -> ii); (4) at an edge,
  code-neutral nodes count: a dead `{ int t = k; }`, `pp->f = pp->f`,
  `PlayersData[k].f = PlayersData[k].f`; global self-assignments, `if (k)
  {}` and `(void)k` do not. In proposal_menu nothing body-level moves it: 0
  to 117 dead block locals, 0 to 58 member self-assignments, 0 to 39 calls at
  the top, one call before each of its 170 statements, the 24 single
  has_treaty/getter swaps, local declaration orders, the
  STRINGSTRUCT_REMOVE_ALL_OUT_OF_LINE guard, and ~RadioButton or ~BasePop
  header self-assignments all keep TT. Only a different set of earlier
  expanded teardowns moves it: the ATTACK exit losing its ~Scroll expansion
  (has_treaty at the commlink loop or the counter tests, get_noun at the
  commlink refusal; 44 rows) gives ii, and a ~Scroll body with an extra
  statement gives ii. No variant gave iT (one ii -> TT edge away), so the
  original's spelling is still unknown; no model of the plateau positions
  exists. Do not retry body-level stand-ins on proposal_menu; tech_trade's
  pair is structural (358 rows) and should be read again once its
  teardowns match.
- **Runtime constant arithmetic is almost absent** (par-6 2026-11-19,
  `analysis/constfold.py`: every straight-line `op r,K2` whose register input
  is a known constant from `mov r,K`/`xor r,r`, both sides, all 5,600
  markers, 70 s). VC6 folds literal arithmetic, named constant locals,
  struct fields set and read back (`set_rect(&b, ...); b.right += 0x12`),
  `static const`, single- and two-return inline results, ternaries and
  if/else merges of one value, loop-invariant locals, char/short/double
  detours and `a[0] + a[1]` of a local array (probes). The image has twelve
  pairs in ten functions, and ours reproduces all but one: merges after tail
  duplication or an inline's multi-return result (arm_val's `mov eax,1; add
  eax,eax` is arm_strat's `return 1` doubled by the caller; `mov eax,1; or
  eax,-1` shared returns) and constant registers (`mov ecx,4; lea
  edx,[ecx-4]`, base_energy). The one unexplained site is NetWin::exec 0x004826CD (`mov
  esi,0x14D; add esi,0x12`, stored to `box.right`): no spelling folded into
  a RECT local reproduces it (14 variants: named locals before or after
  setup_rects_[4], `box.right += 0x12`, a second set_rect from box's own
  fields, an offset loop body, `setup_rects_[36].right + 0x12`, short and
  double detours). The source of 0x14D is something VC6 propagates only
  after folding, most likely an inline's merged return or a tail-duplicated
  join, not a variable holding a constant. 2026-11-20 par-1, eleven more,
  all folded: aggregate initialisers for `r`, `box` or both, a static inline
  taking (rows, left, top, right, bottom) that builds r/box with `right +
  0x12` or `r.right + 0x12`, `int right`/`const int right` passed to
  set_rect, a `RECT *const` source, `r.left + (r.right - r.left) + 0x12`,
  `box = r; box.right += 0x12` after a separate declaration. Note for the
  next try: the image keeps `box` as a whole frame RECT (all four fields
  stored at 0x48272C.., `box.right` reread from [ebp-0x1C] every
  iteration), so in the original `box.right` was not a known constant at
  the loop either.
- **Narrow locals: retype the local, not the expression** (par-6 2026-11-19,
  `analysis/widthscan.py` plus a sweep of every char/short/bool local of a
  non-exact function retyped to int). The image's `setg cl; mov edi,ecx;
  test edi,edi` (or `and eax,0xff; push eax` after a `uint8_t` call) is an
  int/BOOL local; ours `setg bl; test bl,bl` (or a byte spill and reload)
  is `bool`/`uint8_t`. Fixed: BaseWin::queue_click (`BOOL empty`, 119 ->
  82), make_gift (`BOOL existing_pact_or_treaty`, 177 -> 151),
  Console::on_key_click (`int value` for the checksum chain, 633 -> 593).
  Read the image before keeping a sweep gain: an int local that lowers the
  rows can still be wrong when the image keeps the narrow form at the use
  (design_workshop's `mov ax,word ptr [esi]` is the uint16_t flags local;
  Buffer::write_pcx's `mov al,1; inc al; cmp al,1; ja` is an unsigned byte
  `count`, whose int or char retypes lower the rows but change the use).
  Narrow ARITHMETIC on a promoted byte (MapWin::get_alt 0x470377, image
  `mov dl,[m]; shr edx,3; and edx,~3; ...; add eax,4; and eax,0xf` against
  ours `shr al,3; and eax,0xfffc; add dl,4`) is not a declared type: the
  `& 0xF` of set_alt_detail lets VC6 keep only the low bits in both, and 20
  spellings (parameter types, casts, `% 16`, a named detail, the add inside
  the helper, the order of the masked halves) leave it unchanged.
  scenario_setup's `movsx eax,byte [is_flat]; and eax,1; shl eax,2` is the
  same: every char spelling tried (casts, char local, char inline return or
  parameter, volatile) gives a narrow load without the movsx.
  **bool census (par-6 2026-11-19): exhausted.** Every `bool` in src/ (59
  groups: locals, invented inline helpers, header accessors, virtual and
  free signatures with their `// symbol` lines) retyped to `BOOL` and fed
  tree-wide: no non-exact function moves except basepop.h
  `picture_enabled` (proposal_menu 86 -> 85, which does not call it), and every exact user
  breaks (is_trackset_playing loses ten ambience functions, is_3d, is_eax,
  is_group_disabled, sub_4c7fa0, Vector::operator==, Matrix3::equals,
  vox_part_behind, BAmbience::general's `playing`): those are real bools.
  The marker names (`QAEHXZ` vs `_N`) are IDA catalogue names of a stripped
  image, not evidence. The reverse sweeps (48 `BOOL` locals to `bool`, 190
  int locals read from narrow fields to the field's type) give row drops
  that the image refutes: enemy_strategy's `ocean` is `setl bl; cmp
  ebx,ecx` (int), action_terraform's and air_power's x/y are `movsx` at
  the load (int). The one image-backed case was enemy_strategy's
  `goal_triad` (`mov al,[triad]; cmp al,2; ... cmp al,1`, uint8_t).
  `or al,K` / `and al,K` rows on dword fields are register choice (al
  free), not a type.
- **`rand() & 0x80000001; jns; dec; or -2; inc`** is `rand() % 2`.
- **A magic divide then multiply-subtract** is `x - x/C*C`, not `%`.
- **Ternary setcc forms.** `c ? K : 0` gives `set!c; dec; and K`;
  `!c ? 0 : K` gives `setc; dec; and -K; add K` (probe's `setle; dec; and 4`
  is `pop > 3 ? 4 : 0`).
  Reading the image back (par-6 2026-11-20): `c ? A : B` compiles to
  `set(!c); dec; and A-B; add B`, so the image's setcc is the NEGATED source
  condition, its `and` constant is A-B and its `add` constant is B. VC6 does
  not normalise the comparison: `t >= w ? 25 : 50` gives `setl; dec; and
  al,0xe7; add 0x32` where the image's `setge; dec; and 0x19; add 0x19` is
  `t < w ? 50 : 25` (base_terraform 0x4EED7A). A compare against a constant
  with a zero arm has a second form: `x != K ? 0 : -5` lowers through
  `sub K; neg; sbb; and 5; add -5`, while the image's `cmp K; setne; dec;
  and -5` is `x == K ? -5 : 0` (BattleWin::battle_report 0x422153).
- **Bool-to-flag forms differ.** `x ? 1 : 2` gives `neg/sbb/add 2`, while
  `2 - (x != 0)` gives `setne/sub`. `(c ? 1 : 0) + K` avoids `and ecx,0xff`.
  More pairs (par-6 2026-11-20, `analysis/selscan.py`): a bool or select plus
  a constant folds the constant into the consumer, the single ternary does not.
  `t += 1 + (c)` gives `setcc; lea [t + c + 1]`, `t += c ? 2 : 1` gives the
  image's `setcc; inc; add` (draw_supported); `s += (c ? 24 : 0) + 8` gives
  `neg; sbb; and 0x18; lea [s + eax + 8]`, `s += c ? 32 : 8` the image's
  `and 0x18; add 8; add` (enemy_move); `(c != 0) - 2` gives `setne; sub 2`,
  `c ? -1 : -2` gives `setne; add -2` (air_power); `(f != 0) + 1 - h`
  reassociates to `setne; sub; inc`, `(f ? 2 : 1) - h` keeps `setne; inc;
  sub` (base_terraform). `A[(c) + 2]` folds the 2 into the displacement and
  `A[c ? 3 : 2]` keeps `setl; add 2` (draw_ops, not applied: reportif.cpp).
- **A negation folded into its use: name it** (par-6 2026-11-20). VC6
  rewrites `Facility[-p]` into `Facility - p*48` (`shl; sub`) and
  `(-s + n - 1) / -s` into `1 - (n - s - 1) / s`; the image's `neg r` before
  the index or divisor is a named `const int f = -p` (MapWin::right_menu,
  BaseWin::draw_nutrients). Not every site: StatusWin::on_right_click's
  ternary arm and queue_click keep the fold or flip registers with the local;
  base_yield's `-i_c - 1` (image `neg; ...; dec` across an inline) does not
  move with any spelling.
- **`!(c ? a : b)` folds into the jump; per-arm negations materialise**
  (base_terraform 0x4EE36A). The image's `xor eax; test edx; sete al; test
  eax; jne` after two arms that only load is `rocky ? (food ? !x : !y) : !y`
  (each arm a negation, tail-merged); `!((rocky && food) ? x : y)` lowers to
  a plain `test; je`.
- **`g -= c + 1` with a bool `c` folds to `or reg,-1; sub`** (par-5
  2026-11-19): VC6 rewrites `c + 1` of a 0/1 value as a branchless
  `neg/sbb`-style constant and subtracts it. Only the ternary
  `g -= c ? 2 : 1` keeps the image's `setcc; inc; sub` sequence; `c + 1`,
  `1 + c`, `(c != 0) + 1` and a named temporary all give the `or -1` form.
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
- **`lea r,[r+r*2+K]; shl r,4; [r+Array]` is a named index variable in an EH
  function** (par-3 2026-11-19, `analysis/probes/idxfold/`). VC6 folds the
  constant of `Array[K + id]` into the displacement (`[r+Array+K*size]`)
  for every expression spelling: order, enum/#define/const, inline helpers
  returning the pointer, index or field, pointer arithmetic, casts,
  unsigned/short ids, address-taken or volatile ids, a named index in a
  function without EH. It keeps K inside the scaled index only when the
  index is a named variable (`const int fid = FacilitySPStart + id;
  Facility[fid]`) in a function with an EH object and `id` stays live; when
  `id` dies at the add, it gives `add eax,K; lea [eax+eax*2]` instead. The
  image has the shape at three sites only (BaseWin::base_editor_proj
  0x0041BD85, NetDaemon::process_message 0x00537781/0x0053780E), all now
  reproduced; a tree-wide `lea [x+n*x+K]` + shift census finds no other
  address use.

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
  - missing dialog callbacks;
  - the wrong variable of a packed frame slot. Decompiler transcriptions name
    locals by slot, so where the original packed two variables into one slot a
    read can name the one that is dead there (base_build 0x004F81A0 read the
    convoy counter `l_34` where the image reads the unit score `w` from their
    shared `[ebp-0x34]`: 4821 -> 3657 rows). That read has no definition on
    its path, so VC6 /W4 reports C4701: `analysis/uninitscan.py` lists every
    such site tree-wide. The 2026-11-19 pass (about 120 sites in non-exact
    functions) found no second case: the rest are flow false positives (def
    and read under the same test, a switch without default over a closed
    range) or original bugs already annotated.

  Restore the image's behaviour.

### 3.6 Canaries (measure after any edit to their unit or a shared header)
Counter-sensitive canaries carry `CANARY` lines; `uv run recover canary`
re-aligns their pad blocks (2.5). base_energy 0x004EB560 follows the
symbol number of `GameDataStateStorage` (2.5 "Bucket order of referenced
symbols"; pads before the first include of base.cpp). give_a_base 0x0054CF40
does not move at any top-of-unit pad count (declpad.py): it is not a tie of
this kind.
- **reportif.cpp:** `ReportIf::on_iface_button_clicked` 0x004A6330.
- **turn.cpp:** `end_of_game` 0x00524870.
- **Round of 2026-11-19 (after the EditBox setters; one pass, windows by
  whole-unit fe sweep):** turn.cpp function pads 38 -> 50 (all exact at 1,
  5-17, 21-23, 28, 30-35, 40, 42-63); faction.cpp 50 -> 31 (0-3, 5-6, 8-9,
  12-13, 15-16, 18-19, 21-23, 26, 28-33, 35-36, 38-39, 41-42, 44, 46, 60,
  62); terraforming_actions.cpp: no count of the extern block gives
  action_give any more, a function-kind HEADER-PADS block before its marker
  does (6, 25-28; set to 26, extern block stays 22). base_energy 0x004EB560
  (one reload-order row) is exact at no count of function pads before its
  marker (0..63) and at no count of dead locals (0..7): not a counter canary.
- **Round of 2026-11-19, second pass (declpad.py, top-of-unit sweeps with
  the blocks emptied):** faction_treaty.cpp 12 -> 0 (wants_to_attack exact at
  49..16, territory at 55..53) and world.cpp 20 -> 0 (world_volcano at
  59..51): both blocks are empty again. base.cpp: the four struct pads are
  36 prototypes (same symbol numbers; exact at 24..57). Still needed:
  datalink 36 (4..34, 36..63), designwin 16 (2..25, 47..61), prefwin 57,
  turn 50, vehicle_render 21 (exact at 0 and 1 too, but on the edge of a
  window), console_commands 3 (corner_the_market is exact without them;
  Console::upgrade is 147 rows with them and 167 without). For the owner of
  the next canary pass: faction.cpp with its 31 pads removed gives
  buy_council_vote 0x0053EB50 by fe and moves no other marker;
  terraforming_actions.cpp with both blocks emptied and 26 prototypes before
  the first line gives action_give 0x004CEDE0 and keeps action_airdrop.
- **console_commands.cpp:** `corner_the_market` 0x004E2000.
- **Header vetting:** `action_give`, `capture_base`, `setup_player`,
  `base_production`, `battle_fight`, `enemy_move`, `game_data`,
  `load_interface`, `SocialWin::init`, `tech_achieved`.
- **Function-local ties: the function's frame symbols mod 8** (2026-11-19;
  ReportIf::draw_intel 0x004AC140, draw_labs 0x004A7000, Gamma::exec
  0x005C8E60). An operand-order tie (`a - b - c` emitted as `a - c - b`,
  `h - bottom + top`, a swapped `sub`/`add` pair) that no unit pad moves
  (function, extern, data or literal pads before the marker, 0..69) follows
  the function's own frame symbols, period 8. Measured with code-free pads
  on draw_intel and Gamma::exec (`analysis/padkinds.py`, same table on both):
  - counts 1: every parameter, referenced or not (draw_intel with 0..9 added
    parameters: exact at 0 and 8); a local that is stored at least once: int,
    pointer, double, `const int`, `register int`, also when it is read,
    stored twice or has its address taken; a local array or struct that is
    only passed by address, indexed by a variable or copied whole. Where it
    is declared (first line, last line, own block, one shared block) does
    not matter.
  - counts 2: a local aggregate with a constant-offset store (`char q[4];
    q[0] = 0;`, `RECT r; r.left = 0;`, also when it is passed afterwards);
    each further field stored that way adds 2 (two fields 4, three 6); a
    local class object with a constructor (`Font f;`).
  - counts 0: an unreferenced local, a static local, extern and literal
    references (0..40 extra distinct symbols, file or block scope), labels,
    empty blocks, typedefs, enums, calls, file-scope declarations, and the
    parameters and locals of inline expansions (member setters included).
  - code-neutral ways to change the count by one: a constant-valued local
    used in place of the literal (`int width = 236;`), a plain copy of a
    member or global used in place of it inside one block (`int faction =
    report_faction_id_;`, draw_security), a loop index or pointer shared by
    two loops instead of one each. Dropping a local that the code needs is
    not neutral (Gamma::exec without `ok`/`cancel`/`screen_width`: 21 -> 30+).
  Working model (fits all three functions, not proven): parameters and locals
  take the first serial numbers and the compiler's temporaries follow, and a
  commutative/associative operand pair is ordered by serial mod 8; so every
  frame symbol rotates all the function's ties together, a pair of adjacent
  temporaries is the image's way round for one residue in eight, and a
  spelling change that adds or removes a temporary earlier in the function
  (a named pointer instead of a CSE'd index) changes which residues are
  right (draw_security with `VehPrototype *proto`: 5 of 8). The old /GF
  observation (draw_intel exact with pooled literals, not unpooled) is
  probably the same thing: a pooled literal used twice is one symbol and a
  CSE candidate, an unpooled pair is two symbols and none.
  Screen: `analysis/locpad.py` sweeps 0..7 dead locals; one exact (or much
  lower) count k means the original had k more frame symbols mod 8, or 8-k
  fewer. Do not keep the pads; find the locals. Tree-wide on 2026-11-19
  (277 non-exact functions up to 150 rows) only four functions are periodic:
  Gamma::exec (21 -> 6 at 6: six layout constants named as locals plus three
  setter parameters for the store scheduling: exact, WORKAROUND(decl) since
  any six constants do), draw_security 0x004AB600 (10 -> 8 at 3), ReportIf::
  draw_sat 0x004AA970 (54 -> 52 at 2), and draw_intel (fixed earlier).
  Every other function that moves under these pads moves once and stays
  (mind_control 3 -> 100 at 3, sat_attack 15 -> 90 at 1, custom_planet
  11 -> 458 at 1): that is the /Ob2 caller budget of 2.1, which grows with
  the caller's source size. It has the same threshold for a dead local, a
  dead static or literal store, a block-scope extern reference or a dead
  call (mind_control: 3 of any), 4 labels, 5 `(void)&extern` statements, 9
  empty blocks or unreferenced declarations, and none for file-scope pads,
  so it is told apart from the tie by using `(void)&X` pads
  (`analysis/sympad.py`): they move the budget and never the tie.
  draw_security also has a second tie (the `inset + left` add order) that
  frame pads do not move; two extra `(void)&extern` statements flip it
  together with 59 rows of the prototype loop (one-way, 2..40), so it sits
  just under a budget step and the image's combination is not found.
- **A member setter's parameter schedules the store early** (Gamma::exec):
  `scroll_.set_paging(10)` puts the store between the next call's pushes
  where `scroll_.paging_ = 10` leaves it after them; a free inline helper
  taking the field by pointer or reference compiles like the direct store.
  Same family as S7 (the callee's `this` is the named value).
  Tree screen `analysis/storesched.py` (2026-11-19, par-5): 117 non-exact
  functions emit a store at another place than the image; once frame slots
  are left out, only 8 functions have a store that moved across argument
  pushes alone (`SETTER?`). Confirmed there: EditBox's B1C/B20/B24 (new
  one-value setters; AlphaSave::init 0x0040786B, NetWin::exec, DiploWin::exec
  0x00444720; EditGroup::attach 0x00611AF0 is exact with either spelling, so
  a setter is neutral where the store is not next to a call). Not the setter
  shape although they look like it: MonuWin::init 0x004722C4 (the image
  stores LATER than a raw store does, and member helpers taking x/y or the
  rect helpers make it worse), Console::activate 0x004D2FBD (`mov edx, ecx`
  copy of `this` before the second store; a member setter on `this` is the
  same value and changes nothing), BaseWin::init 0x0041F3C3 (one store and
  one push swapped; the order StatusWin::init is exact with does not carry
  over). The confirmed cases are all stores into a member OBJECT; the one
  setter tried on the function's own `this` (Console::activate) compiled
  like the raw store.

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
- An invented inline helper's boundary decides how much nested (pass-2)
  room its inner calls get: the level that holds an inner call, not the
  budget, decides whether that call expands. Move the boundary (what the
  helper wraps) before trying budget probes. In draw_active 0x004B4CF0 a
  helper around the whole damage percentage called all four ranges; a
  helper around power minus health, with the multiply and divide written
  in the caller, expands the outer proto_power sites and calls
  veh_health's two ranges as the image does (418 -> 60 rows).

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
- A field clamp written through a reference, `int &x = field; x = x - 1 < 0
  ? 0 : x - 1;`, is one load, `lea`, `test`+`setl`, one store. A local copy
  `int v = field - 1; field = v < 0 ? 0 : v;` uses `dec`'s flags and can
  spill the value (double_cross 0x0050BFCA, 0x0050C0F1).

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
- A spill-home store the image places right before the next call (after
  that block's own computation), where ours stores the same value at the end
  of the previous block, is an assignment written later in source. VC6 does
  not sink a store to a homed variable across statements: the store sits
  where the assignment is. Move the assignment next to its next real use
  (behaviour-preserving when nothing in between reads it).
  NetWin::draw_session_info 0x00480900: `bottom = next;` at each section's
  end -> after the next section's `avail = ...`, just before
  Buffer::text_width (11 -> 3 rows). Screen: `analysis/stmovescan.py`.
- A store of a loop's bound (`mov [home], 0x19`) to a homed variable right
  after the strength-reduced counter's `xor ebx,ebx`, before the loop body,
  is that variable used as the loop counter: VC6 runs the loop on the
  register and stores the exit value up front. Path::find 0x0059A530: an
  `i = 25;` written before `for (int j = 1; j < 25; j++)` is
  `for (i = 1; i < 25; i++)` (two sites, 858 -> 854; the store moves after
  the `xor`).
  The same store beside a separate down-counter slot names the counter's
  variable even when ours has none: load_vehicles 0x00454270 stores 8 to
  variant's home ahead of the XF column loop (its own down-counter in
  another slot), so the column loops ran on variant, not a dead `column`
  (82 -> 10 rows). Screen: `analysis/loopexitscan.py`.

Rectangles
- Stores through one register from single loads are
  `set_rectangle_dimensions(ptr, x, y, w, h)`; field statements reread the
  sources. An address held across a loop is a `RECT *` local (draw_sat).
- Paired `r.top += d; r.bottom += d` that load both edges first are
  `offset_rect(&r, 0, d)` expanded (BaseWin::init).

Layout
- A boolean or int flag set in one arm and tested once later to skip a
  shared block (`skip = 1; ... if (!skip) { tail; }`) was an original
  `goto` past the block when the image jumps from the setting arm straight
  to the code after the block. The flag keeps the arm's live values in
  frame homes across the block and frees callee-saved registers early;
  `goto past;` gives the image's register regions (energy_yield 0x004E7750:
  297 -> 7 rows, then exact). This is the converse of the council_action
  rule in 2.6: there the image tests a join flag, here it never does.
  Screen: `recovery/tools/flagscan.py`.
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
  source statement). The no-op else is a dead-store stand-in and is not
  allowed. What the image has (par-4 2026-11-19): after the loop the
  `Vehs[veh_id]` offset temporary ([ebp-0x10]) is cached in **ecx**, loaded
  in the `!hint` arm (0x574CA7) and again on the other edge (0x574E4D), and
  the arm jumps past that reload (`jmp 0x574E50`); `i` is in edx. Ours caches
  the temporary in **esi**, loaded once above the test, so no jmp exists and
  the held-back arms land 230 rows later. The image has no callee-saved
  register for it there because the block's registers went to `plan` (esi,
  0x574A88-0x574AAC, `push esi` for del_site where ours folds `push 6`) and
  `veh_id` (edi from 0x574AC7, through the loop); ours gives neither a
  register. And that follows from the label's register state: in the image
  the five `goto L_574a88` sites jump straight to 0x574A88 and the loads
  follow the label; in ours they pass through a compensation block
  (`mov edi,[dist]; mov esi,[temp]; jmp` into the label's middle), because
  upstream the temporary sits in edi/esi where the image has it in
  eax/esi. So the passenger loop cannot be fixed locally: enemy_move's
  register pieces (2.7 rule 8) have to be put right from the top down.
- **enemy_move's first divergence is at its top** (par-4 2026-11-19). In the
  image the `Vehs[veh_id]` offset temporary takes ebx for the whole entry
  region (0x56B642-0x56B83A, nine reads), `proto_id` ([ebp-0xAC]) is never in
  a callee-saved register (all twelve uses in the function are frame reads;
  the three `== 0x10/0x11/0x12` compares share one `mov eax,[ebp-0xAC]`),
  the constant 1 is in ecx and `remote_own` is stored straight from ecx/edi
  (`mov [m],ecx` between the two tests, `mov [m],edi` on the failing edge).
  Ours gives ebx to `proto_id` from its definition to the three compares,
  so the temporary falls back to scratch reloads, `remote_own` is
  materialised in ebx and the entry stores differ. Probe (not the source):
  reading `Vehs[veh_id].proto_id` at the three compares reproduces the
  image's entry region (differing rows 37+111 -> 17+49 in 0x56B5B0-0x56B900)
  while the function total rises (5948 -> 6038), since everything below was
  tuned around the wrong pieces. Neutral there: `remote_own` as if/else,
  `?:` or pre-set and cleared; a `switch` for the three compares;
  `VehPrototypes[proto_id]` for plan/triad; x/y statement order;
  `has_abil(proto_id, ...)` (moves proto_id to edi). What ranks the
  temporary's piece above `proto_id`'s in the image: section 2.7, "Named
  copies of a field" (the image is the natural outcome; ours ranks the
  temporary below every named piece, cause still open).
  Slot side of the same function: pairing each aligned frame access shows
  the image spilling compiler temporaries (the `abs()` kept across x_dist in
  map_range, a `Bases[i].x` copy, a faction byte) into the slots of
  `own_ground` (-0x4C) and `tile_owner` (-0x60) where ours uses `alt`'s and
  `no_moves`' slots, at about 60 rows. Splitting the reused `own_ground`
  into one name per role (tile bits, pact flag, goal flag, sensor flag, turn
  counter, ray x) leaves the frame at 0x2C4 and moves no temporary (slot
  difference 163-169 against 164), so the temporaries' slots follow the
  packing order, not the names.
  One site of that kind is a real order difference: at 0x56D1B4 (the
  plan-10 base loop, `stack_check(veh_at(...), 2, 12, -1, -1) > (x ==
  Bases[i].x && y == Bases[i].y)`) the image calls first and forms the 0/1
  after the call in ecx; ours forms it first and keeps it in a frame
  temporary across the call. The sibling `stack_check(other_id, 2, 6, ...)
  > (...)` at 0x56F15D forms the 0/1 first on both sides (edi is free
  there), so the image's order here is the allocator's (the loop's
  registers are esi = `&Bases[i].x` cursor, edi = score, ebx = base_id;
  ours anchors the cursor on `.y` and has esi = base_id), not a spelling:
  the reversed compare, `?:`, an inline helper for the 0/1, a `Base &`
  local and four spellings of the region lookup leave the window as it is.
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
- **Switch tail-merge direction (measured, `analysis/probes/switchtail/const.py`,
  par-3 2026-11-19).** With register-free tails (`use(1)..use(7)`), VC6 lays
  case blocks out in source order whatever the case values, and when two
  cases ending in `break` share a tail and none of them falls through into
  the join, the *later* case jumps back into the *first* copy (3,7 -> 7
  jumps into 3; 1,3,5,7 -> 3, 5 and 7 jump into 1). The copy survives in the
  later case only when that case is the join's fall-through predecessor (no
  else arm after the switch, `no guard`) or the tails end in a void
  `return;` (`3,7 return`; for `f(); return K;` tails see 3.2). Case
  values, a default, the tail length and short identical bodies elsewhere
  do not change it; text order does (7 written first keeps 7). Real tails
  merge only where register allocation made them identical, so even cases
  that look identical can stay separate (vehdraw's 0/2/4/6 differ in the
  image by registers; ours merges 6 into 4). vehdraw_scoot_start
  0x004BFE00: the image's case 3 jumps forward into case 7 with the else arm
  (`midpoint = 0,0,1`) between case 7 and the join, which contradicts both
  rules for an if/else around a switch. Putting the else stores into the
  switch itself (`switch (uses ? direction : -1) { ... default: stores }`)
  reproduces the image's merges and layout (171 -> 7 rows) but changes
  behaviour for an out-of-range direction (the `ja` goes to the stores, not
  the join), and a `case 8` selector gives 14 rows; the inverted
  `if (!uses) {stores} else switch` gives 81. No faithful spelling found
  yet; the evidence is that the stores were a block of the switch region.
  3.2's "the last branch keeps the copy" holds for `return` tails and the
  fall-through case only.
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
- **Toggle-tail JJ merge ground truth** (2026-11-20 par-6,
  `analysis/tailmerge/menutail/`, a switch of three inline toggles
  `f ^= bit; if (f & bit) m.check(..) else m.uncheck(..); ip = gp;
  psave(0); dmap(1);`, base_names' chain and toggle_dest; `psave-calls 1` =
  merged as ours, 4 = the image's menu_proc). Each probe is one factor:
  - Not blockers (always merged): (a) the tail written in the case or
    inside an inline (explicit `static inline`, /Ob2 auto-inline, an explicit
    `return;` at the inline's end); (b) the copies ending in different inline
    scopes (toggles inline with base_names written in the case and the
    reverse; `psave` or `dmap` or both moved into a further inline helper);
    (c) `break`, `return`, `goto END` with `END:` after the switch, the tail
    case last in the switch (falling into the join), no code after the
    switch; (d) a label before, inside or after the tail, block scopes,
    `do {} while (0)`, `int z = 0; psave(z)`; also 300 extra cases before or
    after, 3,000 filler blocks, 300 live locals.
  - Blockers: a third pushing call in the tail (`add esp,12`; JJ needs the
    two-pushing-call `add esp,8` shape, tm3), a branch between the calls,
    and a switch-end predecessor laid out as the join's fall-through host
    whose tail ends in the same `add esp,8` (one `case 1000: h2(100, 1);`
    reached through the compare tree, or `psave(K); dmap(K)` in another
    case): then every toggle cross-jumps only `add esp,8` into that host and
    no JJ merge happens (p10-p12). That shape (shared `add esp,8`) is not the
    image's either: the image keeps four complete
    `push edi; call prefs_save; push 1; call draw_map; add esp,8; jmp 51C23B`
    tails, and its fall-through host of 0x51C23B is case 0x106's
    `add esp,4; mov ebx,[ebp+8]` in ours too.
  - Real code: the original's toggles ARE mergeable. on_key_click's image
    (same TU, same toggles) JJ-merges them: base_names' three arms and
    flatten's two arms jump into base_grid's tail at `push ebx` (0x51B046),
    and grid cross-jumps whole into base_grid's (0x51B026). So menu_proc's
    unmerged tails come from something outside the toggles. In ours,
    deleting case 0x106, base_names or the second switch, `default: break;`,
    a label or `goto` after the first switch, or `if (state & 0x40) {switch}`
    for the early return all keep the merge. Still open; the scratch
    rotation is separate (next item).
  - menu_proc's toggle scratch registers (image ecx/edx/eax, ours edx/eax/ecx)
    follow the case 0x106 readback: dropping one of its four `(char)S`
    reloads aligns them (not faithful); the CSE'd spellings (char local,
    `int v` local, direct `PlayersData[f].x =` stores) load once but add
    register copies and stay one phase off (71 rows against 60).
  - That readback idiom is open in set_ai 0x00514F50 (11 rows) and
    editor_personality 0x004DBEBD alike: the image loads PopupWriteCheckState
    once (set_ai `mov cl, byte`; editor_personality `mov eax, dword`) and
    re-does `movsx` per use with `and K; shr k` in source order. Every
    spelling that loads once also lets VC6 CSE the `movsx` and canonicalise
    to `shr k; and 1` (20 rows); pointer stores reload per use (ours, 11).
    Measured on set_ai, none closer than ours: `char`/`signed char`/
    `unsigned char`/`int` locals, `char &` / `char *` aliases, an inline
    getter or `bit(c, k)` helper, direct or `PlayerData &`/`*` stores,
    signed masks, `!= 0`, `volatile char` (15), `#pragma optimize` a/w/p/g.
    editor_personality's own `const char low` spelling is one of these, so
    there is no sibling spelling to transfer.
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
- **A written-out copy of a shared exit loses its cross-jump to one scratch
  register** (2026-11-20 par-6, `analysis/xjcalls.py`). Rung 2 call-count
  gaps are often cross-jump outcomes, not inlining. BaseWin::farm_click
  0x0040A1D0 reaches its unchanged exit (0x0040A7A5, `Win::is_visible`
  inside) by `jmp` from five rejections; ours wrote the exit in each of
  three middle rejections, and those copies merged only partly because the
  first copy loads the focus id into eax and the host into ecx, so one more
  `is_visible` call stayed. With `goto` to one exit label the named call
  multiset, EH and frame equal the image. The gotos are not yet the final
  spelling: over all 32 goto/copy subsets of the five exits, every subset
  with two or more written-out copies gives `this` the image's esi and
  [ebp-0x10] (rows 45..60, is_visible 3), and only zero or one copy gives
  edi/[ebp-4] (is_visible 2 with all gotos). Tail-merged copies still count
  for allocation (2.7), so the original most likely wrote the copies and
  VC6 merged them whole. Ours merges them only from the `push` on: the
  copies load `focusedBaseID_` into eax and the host into ecx. The nearest
  earlier difference is the specialist loop, where ours forms
  `lea edx,[edx*4+0x84]` and addresses `[edx+esi]` while the image
  addresses `[esi+edx*4+0x84]` three times. Resolved (par-6 round 30):
  `spec[i >> 3] &= ..; spec[i >> 3] |= ..` lets VC6 CSE the scaled offset
  plus displacement of the repeated subscript EXPRESSION; with the word in a
  local (`int word = i >> 3; spec[word] &= ..; spec[word] |= ..`, as base.cpp's
  specialist setter) every access keeps `[base+idx*4+disp]` and the second
  site's RMW `and [m],r` becomes the image's load/and/store. With that, the
  written-out middle exits (subset 01110) merge whole: calls, jmp count and
  `this` (esi, [ebp-0x10]) equal the image, rows 269 -> 19. So an image that
  repeats `[b+i*4+K]` where ours forms `lea r,[i*4+K]` once wants the index
  in a named local, not the subscript expression repeated (pop_click, which
  writes `spec[position >> 3]` twice, shows the image's own lea CSE). So when the image shows several
  `jmp`s into one tail holding the call and ours writes that tail at each
  site, replace the copies with `goto` to one label. In the reverse case
  (ours merges what the image keeps) look for a missing source jump or for
  a register difference just before the call. `xjcalls.py` lists, for
  every callee whose count differs, each call site on both sides with the
  jumps landing in its run (`j@-k` = `jmp` landing k instructions before
  the call), its continuation and the per-line site counts. On the 26
  rung-2 functions of 2026-11-20 farm_click was the only written-out copy
  that a `goto` fixes. The rest fall into three groups:
  - *A register difference before the call* (fix the allocation, 2.7):
    find_line_break_l 0x5DC410 (image `this` edi, segment [ebp-4]; ours the
    reverse, so the in-loop memrchr tails cannot jump into the post-loop
    ones at the shared `je`), Net::send 0x630080 (ours holds `packet` in esi
    in the L2716 MessageBoxA tail and in edi in L2813's, so they do not
    merge; the image merges L2655, L2716 and L2813 at the final `push 0`),
    base_terraform 0x4ECBF0 (the image jumps the L5283/L5298 `add_goal`
    copies into L5308's at `push esi`, which falls into `relocate:`; ours
    reloads esi on another path into relocate, so L5308 is not the join's
    fall-through host), battle_report 0x421B50 (image psi_label in ebx; ours
    loads first_vehicle_id into ebx inside the psi arm's tail, so that arm
    cannot jump into the shared `push eax; mov ecx; call get` and
    `table_text`/`StringTable.get` spellings of the label do not change it),
    propose_pact, MapWin::draw_map, write_strings_height (above).
  - *Expansion or teardown, not tails*: tech_trade (~BasePop 2/5, return
    teardowns), editor_edit_veh and propose_attack (remove_all against an
    expanded ~StringStruct), base_editor_proj, session_picker_dialog,
    control_turn, CouncWin::init (CharUpperA), send_files (GetFileSize),
    select_special (Popup::start overload), filefind_init.
  - *Ours merges a tail that the image keeps*: communicate 0x54FFD0 L8497
    (the image copies `DiploScratchF7B8 = DiploScratchF7E8;
    diplomacy_caption(); goto` into both arms of the expanded net_treaty_off
    and schedules each arm's pending pop, `add esp,0x28` or `0xc`, inside it;
    ours flushes the pops and joins once. Probe (par-6 round 30, an inline
    `if (G) { f4(..); f6(..); } else f3(..);` followed by a tail and a
    `goto`/`continue`/return): VC6 copies the join into both arms, pending
    pop scheduled inside, only for a ONE-statement tail (`cap(a,b)`, `H = K`,
    or two global copies `H = K; M = K`); `H = K; cap(a,b)` and `H = K;
    cap(0,0)` stay joined at every jump distance, loop or forward target,
    with the tail in an inline helper, or after two expansions. The copies
    are made after cross-jumping (the image's two copies are identical from
    `push esi` and still not merged). Both image copies store F7B8 after
    the treaty call, so the store is not hoisted; what makes the original's
    two-statement tail cheap enough to copy is still open. par-5 round 41
    (`analysis/probes/tailcopy/run.py tails.txt`, nto inline + tail + `goto`
    in a loop): the
    limit is the join's CODE SIZE, not its statement count. The join is
    copied whole when its instructions before the final `jmp` take at most
    15 bytes: `cap0(); cap0(); cap0();` (15), `cap1(a); cap1(b);` (15),
    `H = K; cap0();` (15), `H = capr(a, b);` (15), `cap(a, K);` (15) copy;
    `cap(a, b); cap1(a);` (16), `cap3(a, b, K);` (16), `H = 0; cap(a, b);`
    (20), `cap0()` x4 (20) and the one-statement struct copy `s2a = s2b;`
    (22) stay joined. A tail whose call cross-jumps into an identical call
    falling into the target first (`H = K; cap1(a)` before a `cap1(i);
    cont:`) shrinks to 11 bytes and is then copied, so the copies end in
    `push esi; jmp` (cross-jump first, copy second). Jump distance does not
    raise the limit. The image's two copies here are 20-22 bytes, over the
    limit, and its arm-B order `call treaty_off; mov edx,[F7E8]; add esp,0xc;
    mov [F7B8],edx` shows no label in front of the store; a hand-written
    tail in both arms of an open-coded IsMultiplayerNet test merges the
    pops (`add esp,0x30`) instead, so the original's shape is still open.
    Tested in communicate with no effect: comma statement, the store inside
    either argument (`diplomacy_caption(f, (F7B8 = F7E8, fw))`), an inline
    or `__forceinline` caption helper, a pointer store, an inline
    treaty_off+store helper (changes the budget, 1868 rows)), MapWin::editor 0x46B510 L831 (the
    image keeps range's `2` arm with its own `rocky_set` call and `jmp`; the
    same arm of the erase branch's range merges; neither a spelling of the
    `changed` tests without `goto` nor the operand order changes it),
    use_ability L2853, menu_proc, vehdraw_scoot_start (above).
  - *In situ, and prefix copies* (par-5 round 42, `analysis/tailcopyscan.py`).
    In communicate's own compile the 15-byte limit holds at L8497: with the
    tail cut to `diplomacy_caption(f, fw);` (10 bytes) or to the store alone
    (11) ours copies it into the message_data arm (the `add esp,0x28` flushed
    in front); with both (22) it stays joined. So the image's 22-byte copies
    are not explained by the source tail's size. (The scan's other
    communicate row is not a copy: the written-out `FA8C = F7D4; F7E8 = 0;
    F7D4 = 0` ends of both arms of L8551/L8580 stay two identical 22-byte
    runs in ours, each `jmp` to the join, while the image cross-jumps arm 2's
    into arm 1's, which falls into the join's three stores at 0x552321: an
    open cross-jump miss, see "Cross-jumping ground truth". Round 43: fixed
    by writing `FA8C = F7D4; F7D4 = 0; F7E8 = 0;` plus the join and its
    `goto` in both arms (the image's store order clears F7D4 first); the
    dead budget probes in arm 2 do not block it. The image then hosts the
    shared `F7E8 = 0; join` in arm 1 and the two BETRAYFRIEND sites jump
    into it (0x552331); ours hosts it in the last (F7C8) site, though a
    four-site skeleton hosts first as the image does.) The image also
    copies a PREFIX of a join, its first statement,
    and jumps to the next statement: MapWin::editor 0x46B791 copies the
    `rocky_set` call (11 bytes) into range's `2` arm and jumps to the
    `cmp ebx,2` of `if (rocky < 2)`; enemy_planet_buster 0x56AE62 copies the
    `x = wrap_map_x(..)` store into the `x + bounds` arm and jumps to the
    `y = ..` statement. The copied store there uses ecx and the original
    eax (`add ecx,eax; mov [ebp-0x10],ecx` against the shared `mov
    [ebp-0x10],eax`), so the copy was made before register allocation; that
    copy is also why the image's add is `add ecx,eax` and ours `add eax,ecx`
    (not the 3.4 counter: 0..3 self-assignment pads of the bounds, the base
    or the vehicle and dead locals leave it; any dead local moves the /Ob2
    budget, 45 rows). Ours makes neither prefix copy. Tested without effect
    (enemy_planet_buster 20 rows, editor 6): a label, `goto` to the next
    statement or a block around the next statement; x declared at function
    scope beside y; wrap_map_x as single- or multi-return inline, `bounds +
    x`, xrange_inline (52); editor's case 1 as two breaks, label outside
    the `if`, `if`/else swapped, a range local, both `goto`s. A one-function
    probe of editor's case 1 (`probe` in the round-42 scratch: switch in a
    loop, range inline, two rocky_set arms) copies neither arm either, as ours.
    Round 43 (par-5): the copy is a natural VC6 output, not a source form.
    A one-function loop `int x = wrap_map_x(B[k].x + OX[i]); y = B[k].y +
    OY[i]; if (!on_map(x, y)) continue; ...` makes it (`lea edi,[ecx+eax];
    jmp` past the join's `mov edi,eax`), with the ternary or the
    multi-return wrap alike. In enemy_planet_buster itself it comes and
    goes with deletions far from the site: removing the base_id_hit
    scoring after the HQ test gives the image's shape (x in esi), removing
    the two counting loops at the top or the defense block copies the
    store into all three arms (the flat test inverted to `je`), while
    removing the whole base_id_hit block, the vehicle block, the treaty
    filter, or declaring y in the loop gives none. Not a size threshold
    (the base_id_hit deletion is a superset of the one that copies); a
    function-wide condition, still unknown. Do not respell the site.
- **Cross-jumping ground truth** (par-2 2026-11-19,
  `analysis/probes/crossjump/run.py`, one-function probes with `use(K)`
  markers; names below are its variants).
  - *Per instruction, after allocation, across statements and calls.* The
    merge walks back from the common successor as long as the instructions
    are byte-identical, ignoring statement boundaries: `if (a) { use(1);
    g = 5; } else { use(2); g = 5; }` keeps only `push 1` / `push 2` private
    and shares `call use; add esp,4; mov g,5` (store_k; f0_tail, ret_goto1
    the same). Two arms calling one function with different arguments merge
    at the `call` (argdiff_k: `push c; push 1 | push c; push 2` then one
    `call f2`), so a jump that lands between a push and its call is the
    ordinary result, not a ternary argument necessarily.
  - *Identical statements rarely stay identical: scratch registers rotate.*
    A value loaded from a frame slot or a global into a scratch register
    gets eax, ecx, edx, eax, ... in source order, per load, across the whole
    function (rot6g: `f1(b)` in six arms loads b into eax, ecx, edx, eax,
    ecx, edx; rot_2stmt: `f1(b); f1(c)` rotates b and c together). Two
    copies of a statement merge whole only when their operands live in
    callee-saved registers or are constants (jd_noret: b in ebx, both
    `g = b` arms merge; jd_esi), or when they sit a multiple of three loads
    apart (rot6r: switch cases 1/4, 2/5, 3/6 share one block each; the jump
    table points twice at each). Otherwise the merge starts after the loads.
    So one scratch load added or removed anywhere earlier re-pairs every
    later site, which is the mechanism behind "tail cross-jumping depends on
    the whole function" (3.2) and facility_avail's non-monotonic merging
    above. A merge the image lacks or adds far from any edit: count the
    scratch loads between the sites (a multiple of three = same register).
  - *Pending pops must match.* Arms whose deferred `add esp` differ (one more
    pushing call earlier in one arm: `add esp,12` against `add esp,16`)
    never merge their tails (X_both, X_else). One flushed `add esp` in front
    of the tail is enough to align them (pop_diff, pop_flush).
  - *A one-statement return join is copied into each arm* when the arms do
    not merge (jd1: `use(9); return 1;` after the if/else appears twice;
    jd_retv `return c;` too). From two calls on (jd2..jd6) the join stays
    one block behind a `jmp`. A join that does not end the function is never
    copied (jd_if, mid_goto2).
  - *Same successor required.* Identical tails before different successors
    never merge (difffollow).
  - *`if (a) { X; } else { Y; X; }` is not `if (!a) Y; X;`.* With pushing
    calls in X the first keeps two copies of X (pending pops differ) and
    copies a small return join into both arms (X_both, X_else); the second,
    the `goto` form and the if/else with an empty arm all give one X after a
    `jmp` join (X_after, X_elseonly, X_goto). Two copies of a multi-call tail
    in the image = the statements were written in both arms.
  - *`return f(x)` per arm and `r = f(x); ... return r;` are the same code*
    (ret_val = ret_var); identical `return r1(b)` in two arms still keep two
    copies because b rotates (ret_valsame).
  - *Whole return blocks* are cross-jumped into the LAST identical copy, the
    earlier guards becoming `je` to it (thr_f1kk: four `f1(1); f1(2);
    return 1;` arms, one copy; rot_2stmt, thr_bk pair the rotation-equal
    sites), but only above a size that is not an instruction count:
    `f1(1); f1(2);` and `f1(1); f1(2); f0();` merge, `f1(1);`, `f1(1); f0();`,
    up to eight argless `f0();` and four constant stores do not; two calls
    with two stores between them do (thr_f0g). Measured, not explained.
  - *Destructor teardown* (dtor_*): return sites with one value at one depth
    share a single teardown (dtor_ret: three `return 1` arms merge at
    `call use`, one `~D`); different values give each site its own teardown
    and epilogue (dtor_retk); a return from an inner scope runs the inner
    destructor in place and jumps into the outer teardown of the normal path
    (dtor_ret2, dtor_nest).
  - *Census* (`crossjump/census.py`, image only, 3,060 BYTE_EXACT game
    functions): 692 have a `jmp` into fall-through code, 3,312 sites (most
    are plain if/else joins). 348 jumping blocks end with a `push` (an arm
    that leaves inside a call's argument list: a ternary argument or a
    cross-jumped call), 122 targets are a `call` and 55 an `add esp`; the
    image cross-jumps at the call as the probes do.
  - Real cases read with these rules (2026-11-19): ReportWin::labs_line
    0x0049B450's first layout difference (0x49B550) is allocation, not arm
    structure: the image keeps `host` in ebx, so its two `box_sprite` arms
    are identical but have no host to merge into (the third site uses edx),
    while ours spills host to [ebp+0x14] and the arm cross-jumps into the
    third site after its push. BasePop::exec 0x00602600 (jmp 26/22): the
    image converts to the Dialog virtual base in each set_width/set_height
    arm (`lea ecx,[vb+esi+0x21d0]`, scheduled into the arithmetic of one
    arm, so only the store merges); ours holds `&dialogs_` in ebp for the
    whole function, the conversions become identical and merge into one
    after the join. Binding `Dialogs &d` in the attach block does not remove
    ebp (303 -> 306..314 rows): the candidate comes from the vbase accesses.
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
  store after each pop moves the host (1877 -> 1778 rows). 2026-11-19
  (par-4): the then-arm is not the cause. GOT and TOOK as two lone ifs
  (`if (f == me) {...} if (f != me && t == me) {...}`) host TOOK; a store
  after GOT alone hosts TOOK (an else-arm lone if); stores after GOT and TOOK
  host action 4's DRONEINCITE0 (a lone if in a then-arm); only stores after
  all six pops of actions 3-4 give draw_tile (ASSASSINATED, alone in action
  5, is then unpartnered). So inside probe's chain every partnered `add
  esp,8` pop group beats the earlier draw_tile/popp pair, unlike the tm7
  families (QBnnB, QBWWB host B). Still open.
- **Rule 0: the source fallthrough hosts; the image's branch order shows
  which arm was last** (2026-11-19, par-4, `tailscan.py` HOST lines). When
  the source's last arm falls into the join, it hosts. A join the image
  hosts elsewhere means that arm was not last in the original, and the
  image's compare order shows the original if/else:
  - Console::go_to 0x004D61A0 (991 -> 454): the image's finish compares -2
    first (`je` join), then -1 with `jne` into the first Psi Gate unlock_veh
    copy, unlock_base falling through. `if (order != -2) { if (order == -1)
    unlock_base; else unlock_veh; }` makes unlock_veh the last (else) arm;
    VC6 then hosts the redraw join after the first partnered arm (GATENOBASE's
    threaded unlock_veh) and cross-jumps the other copies into it. The old
    `if (order == -1) ... else if (order != -2) unlock_veh` made the finish
    the host; the inverted inner test (`order != -1`) hosts unlock_base.
  - vehdraw_scoot_start 0x004BFE00 (171 -> 81): `if (!terrain) { flat }
    else { switch }` makes case 7 the last arm and host (case 3 cross-jumps
    into it); `if (terrain) { switch } else { flat }` hosted the flat arm.
  - communicate 0x0054FFD0 (host only, 1568 -> 1566): the pay code written
    after the bribe arm (half-pay falls into it), the non-bribe test as
    `if (result != 2) { if (!result) goto refuse; }`, refusal as a goto-only
    block.
  So inverting an if/else is NOT normalized when it changes which arm falls
  into a join shared with other predecessors (go_to, vehdraw). It is
  normalized for two plain arms (enemy_diplomacy's `attack_type = 3` arm:
  identical code both ways; save_game above).
- **Rule 0 refinements** (2026-11-19, par-1):
  - No `continue` on an arm the image lays out as the fallthrough. A then-arm
    ending in `continue` is held back and the else-arm laid first; the image's
    fallthrough arm ending in `jmp` to the loop end was one `&&`/`||`
    condition with the rest in the else (boom 0x00504AA0, 178 -> 127).
  - Arms that jump to a test instead of threading past it mean the original
    tested a different symbol there (a field reread, not the local the chain
    just assigned), so VC6 had no known value to thread (social_ai
    0x005B4790: `region_base_plan[...]` instead of the chain's `plan`).
  - `board = a && b == K` compiles as branches and threads both outcomes into
    the next test; the image's `sete` arm joining the shared test is
    `if (a) board = b == K; else board = FALSE;` (a ternary or a preset FALSE
    measure the same; stack_veh 0x005B8EE0, 67 -> 63).
  - `!(x <= 0) ? A : B` is not normalised to `if (x <= 0) B else A` when the
    arms meet at a join shared with other predecessors: the last-written arm
    hosts. Write the arm order the image's `jg`/`jle` shows (battle_fight
    0x00506AF0: kill arm first, spot_base last falling into the join).
- **Constant-type inline subscripts fold per subscript** (2026-11-19,
  par-1): add_bat inlined with a constant `type` folds `type * 4` into each
  array base only where each subscript is written `type * 4 + offset`; one
  shared `int index` local computes `(offset + 4) * 80` (`lea r,[r+r*4+0x14]`)
  instead (battle_compute 158 -> 131, add_bat exact both ways).
- **Open, not probe-solvable here** (2026-11-19, par-1): PickTech::pick
  0x00486510 and Console::terraform 0x004D3470 call `??1Scroll` out of line
  at their first 4 / 17 ~Popup return sites and expand it later; ours expands
  it at every early site. 1-24 dead probes after the early returns (pick) or
  at the switch / before the lock tail (terraform) never reach the image's
  call multiset (best: terraform Scroll 14 of 17 with other teardowns
  wrong). Same family as editor_edit_veh / custom_my_faction (2.3).
  NetDaemon::synch 0x00532E00 (frame 0x78 vs 0x7C): the image keeps `this`
  in edi with its home at [ebp-0x18], recomputes `total - 1` (`dec eax`) at
  every chunk_done test where ours keeps a CSE slot, and walks the 0x8D
  social-category compaction with a stage pointer and a `j * 0x2C` counter
  (`cmp eax, 0xB0`); a `RulesSocialCategory *stage` like the other cases
  gives the stage pointer but a down-counter and frame 0x80, and no natural
  spelling tried reaches 0x7C.

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
- **`xor eax,eax` just before the last pop is a shared exit, not another
  compiler.** A `return 0` exit whose `xor eax,eax` sits immediately before
  the final `pop`s (rather than right after the test that chose it) is the
  merged return block that other branches jump to; VC6 schedules the zeroing
  into the shared tail. Spell the returns so they share one exit (a common
  `return 0` at the end, or a result variable), do not suspect a different
  compiler or flags (uV study, par-5: the uV functions are VC6, built with
  the unit's FE_OPT/FE_COMPILER options).

- **Hoisted global loads before the loop init and again at a `jge`-exit
  latch is a do-while** (par-5 2026-11-19, base_build 0x004F81A0, frontier
  scan at 0x4F9E89/0x4F9FCB). The image loads the loop-invariant globals once
  ahead of the counter stores and again at the latch, which exits with `jge`
  and jumps back past the loads. A `for` whose first test is constant-true
  keeps the loads at the top of the body and loops with `jl`;
  `do { ... } while (++k < N);` gives the image's preheader/latch layout
  (3660 -> 3624 rows).

Arm layout and operand sources (par-3, 2026-11-19)
- **Flat `if (A && B) X else if (!A) Y` is not the nested form.**
  `if (A) { if (B) X } else Y` hosts the join after the inner then-arm and
  lays the arms the other way; the flat chain with the same outcomes hosts
  the join after the last arm and cross-jumps the first arm's tail into it.
  Inverting the condition or swapping the arms is normalised, so only the
  flat/nested choice moves the blocks (ReportWin::draw_labs 0x00498B30,
  one-prerequisite arm at 0x49972B first: 763 -> 626).
- **An inline fed by fields read just before it takes the fields, not named
  locals.** When the image reloads the record fields (`movsx` from the
  record) right before an expanded inline and reads a global once, pass the
  fields as arguments: `y`/`x` locals holding them gave the value a home slot
  and a callee-saved register and reshuffled the loop (Console::give_hints
  0x00511BB0, tile_in_view at 0x5129FD: 682 -> 618, frame equal).
- **An else-arm `goto` into a label after `break` lays that block last.**
  `if (!(p > 1)) {Y} else if (p == 2) goto L; ... break; L: X` lays Y before
  X; the image's X-then-Y order is the plain nesting
  `if (p >= 0) { if (p > 1) { if (p == 2) {X} } else {Y} }`
  (NetDaemon::process_message 0x00534400, CHANGEPROJECT block 0x5377E5
  before BEGIN/HALT 0x537876).
- **`int &m = arr[i]; m = m > x ? m : x;` changes candidate weights for the
  whole function**, not only the statement: the reference is one web (the
  element address) where the written-out `arr[i] = arr[i] > x ? arr[i] : x`
  forms the address twice. diplomacy_check 0x0053A980 (census clamp at
  faction.cpp:2591): 910 -> 788 rows, with registers moving in loops far
  from the clamp. Try it (armwo (r)) when disputed pairs sit outside the
  statement.

Switch layout (par-3 2026-11-19, `analysis/probes/switchtree/`)
- **Case bodies of a sparse switch are emitted behind the last tree node that
  references them**, in source order inside that node; the order of case
  groups in the text does not move them. Console::on_key_click's final switch
  (nodes `cmp 0x61` + table 0x21..0x28, `cmp 0x10025` + table 0x62..0x69, a
  byte-indexed table 0x10026..0x10068): the 0x61 body follows node 1, the
  direction stubs (in both small tables) follow node 2, the pans (0x10025 is
  node 2's `je` but also in node 3's table) follow node 3. All six textual
  orders of these groups compile to one layout. Only the order inside a group
  moves blocks (and which pan hosts the shared `set_center` tail).
- **A block reached only by `goto` or fall-through follows its last emitted
  jumper**: the direction body after the last stub when the stubs `goto` it,
  after the 0x61 block when that is its only predecessor; it stays contiguous
  either way. `(unsigned)key`, `default: break`, the body after the switch or
  after the join, `break` into it with the pans `goto` the join, all leave
  the layout unchanged. A nested switch or an `if (key >= 0x10000)` split with
  the pans first reproduces a first-pan host with the join head behind it,
  but changes the tree. The image's split body (head after the 0x61 block,
  rest after the pans) is not reproduced; it implies the stubs joined the
  head only after layout (a cross-jump), and written-out bodies per case
  constant-fold `a2` and never merge.

Teardown reservation (par-3 2026-11-19, `analysis/probes/upgrade_eh/`)
- **A periodic `~FlatButton` pattern at middle return sites (ours
  `EcEcEc`, image all `c`) wants reservations, not charges.** Console::upgrade
  0x004D06C0: flat one-test probes after the last middle return refused at
  most two of the three sites; seven folded two-level probes (free outer
  inline around a one-test inner) there refused all of them and matched the
  image's ~FlatButton multiset (256 -> 147 rows) without touching the later
  teardowns. The natural part was the header-inline three-argument `popp`
  wrapper at the two confirmation popups (sites 6 and 7). Neither moves a
  later teardown deeper (return 1 keeps calling ~StringList where the image
  expands it).

New-expressions and EH states (par-2 2026-11-19, AlphaNetStringStruct::add
0x00401D80, FuncInfo 0x00671810 `[-1,0,-1,2,-1,4]`)
- **A new-expression whose constructor expands inline and is nothrow has no
  EH state at all**, its placement-delete cleanup included. When every
  callee of the expanded constructor is a same-TU nothrow function
  (NetUnk72C() over NetTextListBase), the site loses the state the called
  form has. A state count one above the image's at a `new` site is therefore
  an /Ob2 budget question (the image expanded the constructor there: look for
  its inner callee called out of line), not an unwind refusal.
- **An lvalue placement argument is re-read in the unwind funclet; a value
  argument is spilled by the body.** `new(member_) T` makes the cleanup
  funclet reload the member through `this`; `new(owner(this)) T` (any rvalue,
  e.g. an inline accessor's return) stores it to a frame slot or a dead
  parameter slot before the call (0x00401F4A into `[ebp-0x14]`, 0x0040220B
  into `[ebp+8]`) and the funclet pushes that slot. Same shape as
  JackalVoiceRx::add_player's `voice_owner`.
- **A placement `operator new` without a matching placement `operator
  delete` (same extra parameter types) gets no cleanup state**: the
  constructor call is not covered (C4291). If the image's FuncInfo has a
  state at a placement-new site, the class declares the matching delete;
  if not, the delete is absent or the constructor expanded nothrow (first
  rule).

Expression residues (par-5 2026-11-19, `analysis/probes/bytecse/`, `addsub/`,
`deadstore/`; scan with `analysis/findpat.py bytecse|addsub|negcopy`)
- **VC6 CSEs a char's sign extension in straight-line code, always.** Over
  about 60 standalone spellings of a four-flag unpack (char local, `signed
  char`, `(char)` cast of an int global or int local, `*(char *)&g`, union
  byte member, `char[0]`, char global, `char &`, struct char through a
  pointer, `register`, comma statements, `do{}while(0)`, labels, inline
  getters taking `char`, `const char &`, `char *` or `int`, the getter
  shaped like popup_set_write_check_bit's twin, `#pragma optimize` p/s/w/a,
  the global's address taken elsewhere in the unit) the result is one
  `movsx r,byte [g]` folded into the load and reused, or (when the stores
  may alias: stores through `int *`) a narrowed reload `mov cl,[g]; and
  ecx,K` at each use with no movsx. Only three shapes hold the byte once and
  convert per use: uses in blocks that do not dominate each other (an `if`
  around each use; enemies_trade_tech 0x0055D499, exact, is this), a `switch
  (c & K)` controlling expression (`movsx ecx,al` again) and an inline whose
  `char` parameter has its address taken (`char *p = &f`); the first and
  third narrow to `mov dl,cl; and edx,K` rather than the image's `movsx
  eax,cl; and eax,K`. `#pragma optimize("g", off)` keeps the movsx per use
  but reloads memory each time. No source reproduces Console::set_ai
  0x00514F50's block (the same block is in menu_proc 0x0051C1F1 and
  on_key_click 0x00518499): confidence that it is not a body spelling is
  moderate; the remaining candidates are a compiler-internal difference or a
  header type not probed. Do not retry body spellings of it without a new idea.
- **`(b - t) + t - 1` survives only when `b - t` is not forward-substituted.**
  VC6 substitutes a single-definition, single-use local into its use and then
  cancels `t + (b - t)` to `b` (`dec`), whatever the operand order,
  parenthesisation, unsigned type or block placement (12 spellings, probe
  `addsub/run.py`). The form `sub r,t; lea r,[r+t-1]` (image's GraphicWin::fill
  0x005D53FE) appears when the difference is a value VC6 must keep: used twice,
  used inside a loop (Gamma::on_redraw 0x005C9452, exact: `top + height` in the
  loop body), or stored in a member of a local aggregate (`RECT box;
  box.right = r->right - box.left;` or `int v[4]`). An inline parameter or a
  local-struct `left` gives a third shape (`mov r,t; sub r,t; lea
  r,[r+b-1]`). In fill the aggregate spelling reproduces the arithmetic but
  costs registers (36 -> 42 rows); not applied. Confidence high for the
  mechanism, open for fill's source.
- **VC6 removes the last store to a local or parameter before `return`**,
  spilled (`_width$`), address-taken and passed to a call, aliased onto a
  parameter by reference or pointer, reassigned parameter, referenced from an
  `__asm` block in the same function, accumulated through an inline taking
  `int &` / `int *`, or written as `return w += x` (probe `deadstore/ret.py`,
  16 spellings). The image's `mov [ebp+8],ecx; mov eax,ecx` before
  Buffer::text_width's return (0x005DC9F6) therefore comes from storage VC6
  treats as live after the return, not from a body spelling of a local. A
  function with an EH object drops it too; only static storage keeps the
  store (and then reloads it for the return). A `neg esi; mov eax,esi; push eax`
  (SocialWin::draw_info 0x004B0289) was not reproduced by `-u`, `u = -u`, a
  named negated local, a shared function-scope local or an inline taking the
  value (`deadstore/neg.py`): all give `neg esi; push esi`.
Arithmetic, prologue and arm order (par-2 2026-11-19)
- **`x -= q * K` written in place gives `shl/sub/neg/lea`**: the image forms
  a remainder as `x + q*-K` (`shl` by 4, `sub`, `neg`, `lea [x+q*4]` for
  K = 60) into the slot of the variable that held the dividend, then
  subtracts in place. A separate temporary (`int s = x - q*60`) gives the
  `lea`-based `q*60` and a register copy instead. Source:
  `s = (t + 999) / 1000; m = s / 60; s -= m * 60;` (MultiWin::draw_time
  0x00478BE0, exact).
- **Deferred callee-saved pushes mean a single exit.** When the image pushes
  only one register before an early guard and saves the others after it
  (Dialog::init 0x00609486..0x0060948E: esi, then ebx/ebp/edi past the
  null-rect test), the function has one `return`; any second `return`
  makes VC6 push all of them up front. Keep the single-exit form even when
  a two-return spelling scores fewer rows.
- **The fallthrough arm is the first `if` arm.** When the image tests
  `cmp a, b; jge` and lays the `a < b` work in the fallthrough, the source
  is `if (a < b) {that} else {other}`; the negated test with the arms
  exchanged emits the inverse jump and swaps the blocks (base_build
  0x004F81A0, two sites: `l_e0 < unk_28 / 2`, `l_20 == FAC_COMMAND_CENTER`).

Large-function region pass (par-5 2026-11-19)
- **Sites sharing a cross-jumped tail flip together.** Three lookups whose
  calls the image merges into one tail (setup_text's three Natural brush
  names) must all use the same spelling (`table_text(x)` vs
  `StringTable.get(x)`): toggling one site alone un-merges the tail and reads
  as a loss. Toggle them as a group (`analysis/sitedeck.py` does single
  sites; a site that loses alone next to siblings in one tail is retried
  together by hand).
- **An un-threaded second test of a just-tested register means the
  subexpression was written at each test.** air_power 0x005678C0 re-tests
  `eax = state & 0x200` at 0x567AF6 and 0x567B28 without jump threading and
  re-reads `order_auto_type` from memory each time: the source repeats
  `(Vehs[veh_id].state & 0x200)` in each condition. A bool local (`automated`)
  is threaded by VC6 (the second test disappears) or kept in a register.
- **One division tail joined by several arms is one statement after the
  if-chain**, not a copy per arm (air_power's two `/ Rules.move_rate_roads`
  at 0x567A65, entered with different eax): arms assign the operands, the
  division follows the chain (1195 -> 1059 rows with the previous rule).
- **A lookup whose offset is loaded into a register and pushed is a named
  inline parameter** (`table_text(Mood[get_mood(...)])`), not a local plus
  `StringTable.get` (setup_text 0x456E04).
Accessors, folded helpers and unit order (par-1 2026-11-19)
- **Accessor inlines in tests take pass-2 room.** A nested inline the image
  keeps as a call while ours expands it (or the reverse) is often decided by
  how the surrounding tests are spelled, not by the nested site. enemies_war
  0x0055C840: with the treaty tests as raw `PlayersData[a].diplo_treaties[b]
  & K` reads, `get_noun` expanded inside both `pact_attacks` copies (1 call
  against the image's 3; 101 rows). With every test written
  `has_treaty(a, b, K)` the call multiset is the image's and the function is
  byte-exact with no `#pragma inline_depth` region. When a call multiset is
  off by a nested inline, respell the function's raw field tests and writes
  through their header accessors first (`analysis/callees.py` NAMED 0 is the
  target), before any pragma or budget probe.
- **A plain function called with a constant can be a whole statement run.**
  `treaty_off(a, b, DTREATY_WANT_REVENGE)` expanded and folded on the
  constant is exactly four statements (the two agenda clears and the two
  treaty clears). A run of adjacent field writes on a faction pair whose
  constants line up with one helper's switch arm is that helper called with a
  constant, not hand-written stores (enemies_war, both revenge blocks).
- **A local copy of a global inside an inline helper shows as a frame slot
  kept across calls.** `spying` 0x0055BC00 read `ConsoleGlobal.faction_id_`
  into a local; at its two expansions in enemies_war the copy lived in
  `[ebp+8]` across `has_project` where the image reloads the global
  (0x0055CA2A, 0x0055CA7C; 47 rows). The helper reads the global at each
  use. A slot ours keeps and the image reloads from a global, inside an
  expanded helper, is a local in the helper's body.
- **Literal order inside a function gives source arm order.** The unit's
  literal run follows first textual use. encounter 0x0055E370: GUYMEETSAL
  before GUYMEETSALNOPSYCH (0x0068E6A4..) means the psych test is written
  positively with the NOPSYCH message in the else arm, although both arm
  orders compile to the same code. Unreferenced literals in the run are
  arguments of expanded inlines that ignore them (`rnd(n, "ENEMIESWAR")`,
  `rnd(bound, "RANKINGS")`, the six console preference-page labels):
  `analysis/unitlits.py --who` names the owning function by address order.
- **Whole-file moves renumber `_$E<n>` thunks.** A static object's dynamic
  initialiser and atexit thunks are numbered by the front-end counter at the
  object's definition, so reordering the definitions of a unit changes their
  names (battlewin.cpp: `_$E12/_$E10` became `_$E14/_$E12`; 0x004219B0 and
  0x004219D0 read inexact until the symbol facts were rewritten through
  `match_recovery.source.writer.write_symbols`). After any reorder of a unit
  with file-scope objects, compare the thunk names. A `#pragma` that closes a
  region also travels with the chunk that followed it: check that each
  region still wraps the same functions (veh.cpp name_proto, netwin.cpp
  exec, tutwin.cpp tour*).
- **A function inside an `auto_inline(off)` region puts its literals in a
  trailing .data section.** VC6 compiles such a function at once instead of
  deferring it, and its literals go to a separate .data section laid after
  the deferred functions' section, so they leave the unit's literal run
  (tutwin.cpp tour6/7/5/1; console.cpp chat, ~Console, governor_directives).
  A region must wrap only the function it is evidence for; literals that are
  in address order in the image argue against the function being in such a
  region at all.

- **/Ob1 (or /Ob0) with /Gy puts a function's string literals in a COMDAT
  .data section; /Ob2 keeps them in the unit's ordinary .data** (par-1
  2026-11-19, danger.cpp). The unit's /Ob1 recipe compiled danger 0x00538F30
  exact but its two literals left the ordinary section, so
  `recover rebind-literals` found no `$SG` label with ordinary storage ("no
  fresh $SG label holds exactly the original bytes": the bytes were right).
  The image has them in ordinary order between netdaemon's literals and
  my_srand's, so the unit is /Ob2. Check a per-unit /Ob exception against the
  literal run before keeping it.
- **A binding-order refusal after an address-order move is not a source
  fact.** `rebind-literals` keeps the relative label order of a unit's
  existing bindings; moving definitions (reorder.py) changes that order, and
  one crossing pair refuses every binding of the unit ("no assignment keeps
  the TU's existing binding order": basewin, console, datalink, designwin,
  27 bindings). When each binding has one content-equal candidate (duplicates
  narrowed by `context_relocation`), set the symbols from the candidates and
  re-run with `--unit`: all valid.
- **A pair of prompts with one test after them is one call in the image.**
  shoot_it 0x004CDAF0: `if (c) r = popp(A); else r = popp(B); if (r) goto
  fail;` lets VC6 cross-jump the two calls (one arm pushes and jumps into the
  other's call). The test written inside each arm keeps two calls, because the
  then-arm copy ends `jne fail; jmp next` and the else-arm copy `je next; jmp
  fail`: different tails, no merge.
- **A sum passed to an inline as an anonymous argument and as a named local
  can differ in the `lea` operand order of the expansion** (planet_busting
  0x00500B60: `wrap_map_x(RadiusOffsetX[i] + x)` gave `lea esi, [eax + edx]`,
  `const int column = RadiusOffsetX[i] + x; wrap_map_x(column)` the image's
  `[edx + eax]`, exact for 59 of 64 counter pads). Per site: the same
  expression in the function's first loop is exact unnamed.
- **A helper's own getters decide whether pass 1 expands it** (par-1
  2026-11-19, enemies_treaty 0x0055D760). pact_truce with its leader and noun
  fields written out (`GenderDefault = ..; PluralityDefault = ..;
  parse_says(.., Players[f].title_leader, ..)`) is too dear for pass 1 and
  stays a call at both sites; written `parse_says(0, get_title(f), ..)`,
  `get_name`, `get_noun` it expands, its out-of-line copy is still
  byte-exact, and the last get_noun of each copy is the image's call. Respell
  the helper before the caller.
- **Expression inlines before a nested site grow the budget; after it they
  reserve.** In enemies_treaty three dead `has_treaty` tests anywhere after
  the treaty arm refuse has_project in the truce arm's spying copies; the
  same three before the truce arm expand one more site.
- **has_treaty is inline only in faction.cpp, faction_treaty.cpp and
  console_commands.cpp.** Respelling raw `diplo_treaties[..] & K` reads as
  has_treaty elsewhere adds calls the image has not got (accessor sweep over
  the 362 non-exact functions, 2026-11-19: every such case raised the call
  distance; a row drop there, action_terraform 90 -> 48, is an accident of
  the call). The hand-expanded get_name/get_title/get_noun respelling gained
  only base_production (335 -> 301) and broke teardown decisions in
  control_turn; it is neutral in propose_pact.
- **Two-way treaty/agenda writes equal the writer folded on a constant** in
  the writers' own unit: `treaty_off(f, i, 0x200)` and the hand-written pair
  compile to the same code (enemy_diplomacy, eight sites; enemies_unpact's
  0x01100001 clear is `treaty_off(f, w, DTREATY_PACT)`).
Prologue saves and return tails (par-5 2026-11-19; `analysis/prosave.py`,
`analysis/probes/prosave/run.py`)
- **A callee-saved push is deferred only when the code that needs the
  register reaches the epilogue through ONE edge.** VC6 places each save at
  the entry of the region that uses the register and its pop at that region's
  single exit; a second edge from the region to the exit moves the save into
  the prologue. The guard's spelling does not matter: `if (!c) return; body`,
  `if (c) { body }`, a trailing explicit `return;`, `goto end` and
  `if (!c) return 0; body; return k;` all defer the same way (probes
  er_var, wrap_var, er_trail, goto_var, er_val; two guards in a row and a
  call before the guard defer too). What forces every push up front, each
  measured on the same body:
  - a `return` inside the body (er_inret; also `goto end` or an `else` arm,
    which VC6 turns into a second epilogue copy);
  - a body whose LAST statement is conditional: an `if` without `else`
    (er_var_if), a `for`/`while` whose entry test can skip it (er_loop), a
    `switch` (er_sw), a compound `||`/`&&` test (er_tmp, check_veh
    0x0041A8B0). Putting one unconditional statement after it defers again
    (er_if_then, er_loop_then, er_sw_then); so does `do { } while`, a
    `for (;;)` left by `break`, or a `return` written inside the loop when
    the loop is the last statement (er_do_ret, er_for_ret, er_for_brk);
  - a second early-out after the register is in use (er_er_reg);
  - a result variable initialised before the guard (`int r = 0; if (c) {..}
    return r;`, res_val): the register is live across the guard.
  A deferred save is per register and nests (sub_441040 0x00441040: edi
  after the first test, ebx/esi inside the loop). Census of the 2,338 exact
  functions that save a register: 285 defer; 231 of them (81%) have exactly
  one edge into the deferred pops, the other 54 are an epilogue copied after
  allocation (farm_get 0x00409100: the if/else arms each end in their own
  copy). Of the 287 that push everything up front although the entry block
  leaves a saved register untouched and its first branch is an early exit,
  278 have three or more edges into their epilogues. This replaces the
  "single exit" reading of 2.6 "Deferred register saves" and of "Deferred
  callee-saved pushes mean a single exit" above: the count that matters is
  edges from the register's region, not `return` statements (control_game
  0x0052AA30: the labelled failure tail is entered by `goto` from the
  guard, so the loop needed its own `return` to add the second edge).
- **Push order is not a lever.** Within the prologue, and within each
  deferred group, the order is always ebx, ebp, esi, edi (2,049 of 2,054
  exact functions without a deferred save; the five others set up ebp
  late). A different order always means a different register set or a
  different deferral: fix the allocation, not the order.
- **Tree screen, 2026-11-19 (354 non-exact functions):** a (image up front,
  ours deferred) 4, b (the reverse) 2, c (different number of `ret` tails)
  13, d (order only) 0, saved set differs 8. None of the a/b cases is a
  guard spelling: each has a value the image keeps in a callee-saved
  register that ours keeps in ecx/edx or in memory (Console::chat
  0x00515770: `field_1BD0_` in esi for one `and`; success_rates 0x0059EEE0:
  `morale` in ebx where ours enregisters `diff_modifier`; Buffer::
  write_vertical 0x005DB450 and polygon_gouraud 0x00626A00: ebx/ebp/esi
  roles swapped), so the save follows the allocation (2.7) and the prologue
  is the symptom. The c cases are tail-merge cases: read them with
  `analysis/tailscan.py` (Net::drop_player 0x00632350: the image keeps a
  lone `return 0` copy after the search loop that ours folds into the
  switch arms' shared tail; NetDaemon::init 0x0052DF30: the image's
  fall-off path has its own epilogue, ours tests between the shared pops).
- **`return K` copies.** An unconditional path to the exit gets its own copy
  of the epilogue (er_inret: 2 `ret`); two paths ending in the same
  statements are cross-jumped back into one (er_inret_same: 1 `ret`); a
  conditional branch to the exit never gets a copy, it targets a surviving
  one. An if/else as the last statement gives two copies (er_ifelse).

- **Return-tail copies: ground truth** (par-5 2026-11-19;
  `analysis/retcensus.py`, one /FAsc compile per unit, 2,613 `return`
  statements in 880 exact functions; `analysis/probes/rettail/run.py`).
  COPY = the statement has its own `pop..ret`, FOLD = no code (its branch
  targets another statement's copy), FALL/JMP = value set, shared epilogue.
  - *Same expression as the function's last `return`:* folded into the last
    return's copy. Guards 8 COPY against 358 folded (192 FOLD, 144 FALL, 22
    JMP), `S; return x;` 12 against 253. A copy there is a red flag.
  - *A different value, single condition (`if (c) return K;`):* its own copy
    (`jncc skip; mov eax,K; pops; ret`), also when several guards return the
    same K (prototype_factor: two `return 0` copies in a row): unique value
    171 COPY / 7 FOLD, first of several equal ones 72 / 26, a later one
    166 / 36. `&&` 60 / 25 and `||` 46 / 12 follow, but the terms before the
    last one jump to the copy the last term falls into (or_guard).
  - *`S; return K;`* (an unconditional path): own copy, 120 of 172; two
    paths ending in the same statements are cross-jumped (see above).
  - *Probe results:* the number of equal returns alone never folds a guard
    (`if (c) return 0;` followed by 1..8 `if (ask(n)) return 0;` or by
    switch arms `a = n; return 0;`: the guard keeps its copy every time). A
    guard is folded when the last return has the same value (g_end0), when
    the guard is an `else`-less wrap (`if (c) {..} return 0;`), and when
    switch arms written `S; return K;` are cross-jumped into a common tail:
    that tail then also takes one bare conditional `return K` (dp_ret 7
    `ret`, dp_brk 8). Net::drop_player 0x00632350: both tolerance switches
    with `poll_timeout = N; break;` and the one `return 0` after the switch
    give the image's guard copy; `poll_timeout = N; return 0;` in the arms
    folds it (7 rows -> exact). The arms get their epilogue copies either
    way. `analysis/retbrk.py` tries both spellings per switch and for
    every subset (only four non-exact functions have a switch directly
    followed by `return`, no other gain).
  - *Layout `jcc RET; jmp JOIN`* (veh_init 0x005C080A) against ours `jncc
    JOIN; mov eax,v; pops; ret`: the then-arm ended in `if (p || q) return
    v;`. Solved by par-6 (2026-11-20, next bullet): the source tests p and
    q separately and branches with `goto done;` to a label on the final
    `return v;` (exact). The `||` form and `return` in either form all
    keep the copy (probes vi_* above and dup.py).
- **Epilogue copy or branch: the statement shape decides, not the epilogue**
  (par-6 2026-11-20; `analysis/probes/rettail/dup.py`, 40 shapes, each with
  the value constant / register / address-taken local / expression, 0-3
  saved registers, with and without a 64-byte frame). The returned value,
  the number of pops and the frame never change the choice; the cells of
  one shape agree except where noted.
  - A nested `return V` that ends the then-arm of an if/else gets its own
    copy (`jncc JOIN | V; pops; ret`), even when V is the final return's
    value: `if (A) { if (B) return V; } else { X; } J; return V;` (T1, with
    `||` T2, with an `else if` that also returns T5/T13). Without the outer
    `else` (T3), or with any statement after the inner `if` (T6), it
    branches to the final return's block instead.
  - `goto out;` to a label on the final `return V` branches (`jcc RET;
    jmp JOIN`, T9), and so do separate `if (p) goto out; if (q) goto out;`
    tests (T19). `if (p || q) goto out;` still gets the copy (T14), as do
    `if (p) return V; if (q) return V;` (T20) and every `&&`/negated/
    `else { goto join; }` spelling of the arm (W_* rows). So an image
    `jcc RET; jcc RET; jmp JOIN` where ours has `jncc JOIN; <copy>` is
    separate gotos to the final return (veh_init 0x005C03D0, 8 -> 0).
  - In the probe, an `else` arm that itself ends in `return` is moved
    after the join once the then-arm ends in a plain `jmp` (T21, V_two_g_*:
    the then-arm then falls into the join); in veh_init the larger else arm
    stays in line, so check the real function before rejecting the goto.
  - The arms swapped (`if (!A) { X; } else { if (B) return V; }`, T7): a
    branch with 0-1 saved registers, a copy with 2-3; the one register
    count dependency of the matrix.
  - `analysis/retmap.py` lists the functions whose return tails differ
    (2026-11-20: 13 of the non-exact set; the others are inline/EH or
    cross-jump host questions: AlphaNetStringStruct::add, Buffer::copy,
    monolith, shoot).
- **An innermost two-constant if/else is setcc; explicit `mov K1; jmp | mov
  K2; jmp` stores mean the pair was not innermost** (par-6 2026-11-20,
  micro-probe and PickTech::draw_tech 0x00484C60). `if (c) m = K1; else m
  = K2;`, `m = c ? K1 : K2` and an inline helper `if (c) return K1; return
  K2;` all give `xor; cmp; setcc; dec; and; add`. A three-way chain whose
  last two arms store the same constant, `if (c1) m = K1; else if (c2) m =
  K2; else m = K2;`, keeps c1 as branches: its identical arms are merged
  and the c2 compare is deleted, so the middle threshold leaves no trace.
  In draw_tech the same spelling also hoists the shared `cell_width_` load
  above the outer `jne` (image `mov eax,[W]; cmp eax,800; mov eax,[cw];
  jne`); 290 -> 277. Three distinct constants keep c1 as a branch and make
  the c2 pair setcc.
  - *A fall-off path with its own epilogue* (NetDaemon::init 0x0052E02A:
    `test eax,eax; jne L; pops; ret; L: pops; mov eax,1; ret`) against ours
    `test; je; mov eax,1` inside one epilogue: sixteen spellings (result
    local, negated guard, if/else, switch, loop forms, `&&`) compile to our
    form; not found.
  - facility_avail 0x005BA0E0: the first guard's copy is folded in ours
    only while the Tachyon Field statement is present (deleting any other
    statement keeps `je`); that statement's expanded has_fac writes its
    `offset`/`mask` into the homes of facility_id and base_id. Seven
    spellings of the statement, eight of the guard and four of the Ascent
    arm leave the 8 rows.
Block order: which arm is in line, which is moved after the `ret` (par-2
2026-11-19; `analysis/probes/blockorder/run.py`, 79 one-function probes whose
skeleton keeps only branches, `ret` and numbered `use(K)` markers)
- **Source arm order is emitted order.** `if (c) A else B` emits A, then B
  (`je B`); negating the test and swapping the arms swaps the emitted blocks
  (ie, ie_neg). An arm that ends in `return` does not move: `if (c) { A;
  return 0; } rest` keeps A in line behind a `jncc` over it, with its own
  epilogue copy (ie_ret_then = ie_ret_noelse, same code with or without
  `else`). So an in-line arm that the image jumps over is the *then* arm of
  the test as written; fix the test's sense, not the layout.
- **A `goto` to a join copies the join.** `if (c) { A; goto done; } B; done:
  C; return` gives `A C ret | B C ret` (ie_goto), the same code as the
  if/else whose arms each end the function (ie_last). A labelled block
  placed after the function and entered by `goto` (`if (c) goto away; ...
  away: A; goto back;`) is pulled back in line as the then arm (if_goto_far
  = ie): label position is not a layout lever for a small arm.
- **Inside a loop only an arm that leaves the loop is moved out of line.**
  `if (c) break;` and `if (c) continue;` are plain branches (lp_break; lp_cont
  = the wrapped `if (!c) { body }`, lp_wrap). An arm with a body that ends in
  `break` or `return` is emitted AFTER the function's `ret`, and the code
  after the loop is duplicated: the normal exit falls into one copy, the
  moved arm falls into the other (lp_brk_body `je X | body | jl loop | tail
  ret | X: arm | tail ret`; lp_ret_body, lp_ret, lp_while). In a `do { }
  while` the same arm sits directly behind the loop and the normal exit
  jumps over it (lp_do). An arm that ends in `continue` stays in line with a
  `jmp` to the step, identical to the if/else (lp_cont_body = lp_ie). So an
  out-of-line block behind the `ret` that falls into a second copy of the
  post-loop code is `{ ...; break; }`; the same block in line with a `jmp`
  to the step is `continue` or an else arm.
- **Switch against if-chain.** An if-chain tests and emits each body in
  source order (`jne next | body | jmp end`, chain; with returns `jne next |
  body ret`, chain_ret). A `switch` emits all compares first (`je c1 | je c2
  | je c3`), then the default or the code after the switch, then the case
  bodies in REVERSE source order (sw, sw_ret, sw_sparse). Bodies in reverse
  order behind a compare cluster are a switch; bodies in test order are an
  if-chain. (Cases ending in `return` each keep their epilogue copy.)
- **Early returns: copy or branch.** With a final statement `return K1`, an
  early `if (c) return K1;` is a conditional branch to the final block,
  while `if (c) return K0;` (another value) gets its own epilogue copy in
  line behind `jncc over` (ret2, reg0; with one saved register or three and
  a frame). When the final statement returns K0 too, every early `return K0`
  branches to one shared block, which sits at the first guard that cannot be
  inverted (reg0_end0), and wrapping the body in `if (a) { ... } return 0;`
  is the same code (end0_wrap, last0_neg). `if (a || b) return 0;` at the
  head puts its one `return 0` block after the function (ret2_or): an
  `||` guard is the only head guard whose return leaves the line. An `&&`
  guard whose last test is threaded past the next statements (`jg` beyond
  the following tests that the first operand already decides) can never be
  inverted, so its copy always stays (facility_avail 0x005BA0E0 second
  guard, `jg 0x5BA13D`).
- **Loop tests: VC6 duplicates the test, it never jumps to it** (par-2
  2026-11-19; `analysis/probes/looptest/run.py`, 54 one-loop probes). Every
  `for` and `while` whose test is not a compile-time constant becomes `guard
  jncc OUT | body | step | test jcc body` (class DUP): constant, parameter,
  global, member, array and expression bounds, a call in the test (the call
  is emitted twice: `for_call`, `wh_call`), `&&` chains (the guard is the
  first term, the later terms sit at the top of the body with the first
  term at the bottom: `for_and`, `for_andcall`), pointer walks, nested
  loops, loops in an if or else arm, `continue`/`break` inside. A constant
  trip (`i < 8`, `p < &gt[8]`) and `do { } while` have no guard (NONE); a
  hand-written `if (c) do { } while (c);` is the `for`'s code (do_guard).
  Only three spellings keep the test at the top with a `jmp` back (TOP):
  `while (1) { if (x) break; ... }`, a `||` test (`for_or`), and a test
  that is an inline function returning `a && b` (`for_inl2`:
  `jge OUT | je OUT | body | jmp top`; the same test written in the `for`
  is rotated). A break test at the bottom of `while (1)` is a do-while
  (`wh_1b`), and `for (i = 1; ; i++) { if (i >= K) break; ...` is the plain
  `for` (enemy_diplomacy, three spellings, same bytes). No probe gives
  `jmp TEST` into the loop. A `do { ... if (c) break; } while (++i <= n);
  if (i <= n) A; B` always closes with `jg B | jmp HEAD` and lays the
  re-test after the loop; `&&`/nested/`continue`/goto spellings of the
  break, `while (1)`/`for (;;)` with a bottom break and a goto loop are the
  same code (looptest rd_*). Only a rotated `for`/`while` (with its guard)
  ends `jle HEAD`, falls into B and moves the re-test and A after the `ret`
  (rd_for, rd_whi). drone_riot 0x004F4FB0's image has that rotated latch
  with no guard before the loop: not found. Where the image has a `jmp` over the first
  instructions of a loop, they are register loads the entry already holds
  (`mov ebx, [BaseCurrent]` in BaseWin::draw_pop 0x414CF2 / 0x414E80,
  `mov edx, [ebp+8]` in ours for wants_prototype): the value is reloaded on
  the back edge because a register was taken from it in the body. That is
  an allocation fact (2.7), not a loop form; `rungs.py` counts these
  separately (`+r`) from real loop-set differences (`+d`).
- **Switch lowering** (par-2 2026-11-19;
  `analysis/probes/switchlower/run.py`). One to three cases are a subtract
  chain (`dec eax | je | dec eax | je`; `sub eax, K` across a gap, `sub eax,
  0` for case 0), with or without `default`. From four cases on, a dense
  set is a jump table (`dec eax | cmp eax, span | ja | jmp [eax*4+T]`): four
  cases over a span of up to 9 values and five over 9 use the direct table;
  a wider span (4 cases over 10..20, 5 over 13..30, 8 over 200, two runs of
  four 100 apart) gets a byte index table in front (`mov dl, [eax+B] | jmp
  [edx*4+T]`), also when all the cases share one body. Outliers split a
  tree instead: `1,2,3,4,500` is `cmp eax, 3 | jg | je | dec | dec | ... |
  cmp eax, 4 | cmp eax, 500` (the run of four is a chain, not a table), and
  decades (`1, 20, 300, 4000, ...`) are a binary tree on the median with
  subtract chains in the leaves. So the lowering names the case set: a
  direct table of span n has at least four cases within 9; a chain of three
  `dec`/`sub` cannot have had a fourth case in range; an index table means
  the smallest and largest case are more than 9 apart.
- **Block placement that no spelling moved** (enemy_diplomacy 0x0055F930, 8
  rows): the satellite loop's shared `attack_type = 3` block sits behind the
  `ret` in the image and jumps back to the `action_sat_attack` call, with
  the loop latch as `jge OUT | jmp HEAD`; ours lays it in front of the call
  and the latch as `jl HEAD | jmp OUT`. Nine CFG-equal spellings (goto
  to a label at either site, either site hosting the label, the last arm
  negated or nested, the block after the `break`, `for (;;)` with a break
  test, the arm with its own call and `break`) compile to our bytes or
  worse: VC6 normalizes them. The cause is not in the arm structure.
- **Rung 4 class c, worked (par-6 2026-11-19; probes `gt_*`, `thr_*`,
  `abs_*`, `ob_*` in `analysis/probes/blockorder/run.py`).** Read the first
  `!!` conditional jump whose sense is inverted against ours, or the first
  `!>` whose image target is a different *kind* of block (another return's
  teardown, a block after a call), not the `first` block rungs.py names:
  that is often a jump-target shift caused further down.
  - *Inverted test, arms swapped:* the image `jne`s to an arm laid out
    after the other one: write the arms in the image's order. Three of the
    old `if (!(!x))` / `if (!(x))` spellings were tuned to the wrong layout
    (action_terraform 0x004C9D60 `if (!do_it) {contribution} else {merge}`,
    BattleWin::battle_report 0x00422671 and 0x0042225A). The "arm placed
    after the join and jumped back" in action_terraform was this, not an
    out-of-line form.
  - *A constant arm threaded into the next test:* `v = c ? -1 : f(); if (v
    < 0) { ...; return; }` compiles `c` to a jump straight into that
    return's block (thr_tern = thr_if). An image `jne` from a guard into the
    teardown of a later early return is this shape; an `if (!c) { v = f();
    if (v < 0) return; ... }` wrapper sends it to the final return instead
    (base_production 0x004F107F, 289 -> 243, rung 4 -> 5).
  - *Two-arm abs:* `sub; js A; mov edx, ecx; jmp J; A: mov edx, esi; sub
    edx, ebx; J:` is the ternary `d = x - y >= 0 ? x - y : y - x`; the
    one-arm `d = x - y; if (d < 0) d = y - x;` is `sub; jns J; ...`
    (abs_tern/abs_if; capture_base 0x0050DC1E has both forms, dx and dy).
  - *`goto` polarity:* `if (d) goto F;` as the last statement of an arm can
    compile to `je JOIN; jmp F` in a large function; the image's `jne F;
    jmp JOIN` came from adding `goto JOIN;` with the label at the join
    (shoot_it 0x004CDF47, rung 4 -> 5). In the small probe (gt_arm) both
    spellings give `jne F; jmp J`, so the inversion is whole-function.
    Writing the test inside each popup arm instead gives fewer rows (33)
    but hosts the cross-jumped call/test in the first arm, the image in the
    last: keep the layout-correct form.
  - *A jump that skips a call ours always makes is behaviour:* battle_report
    0x0042260D/0x00422618 `jl` past the wrap call at 0x00422795, so the
    weapon line is wrapped only inside the odds test. It moved the function
    to rung 2 (allocation change, lost cross-jump); kept.
  - *A test that guards a whole run of statements, transcribed as guarding
    the first one:* game_data 0x005A8651 `test [net_turn_flags_], 0x20; je
    0x005A91B2` jumps over every transfer from OfferResetCells through
    TamperDialogValue to the screen-size block; the source tested the flag
    in the first `&&` only. behavscan showed it as one IMG-ONLY predicate
    on 126 sites; rows 218 -> 147 once the block was braced.
  - *Behaviour screen precision (behavscan, 2026-11-19):* a hit is a lead,
    not a verdict; read it against the image. Not behaviour, and now
    filtered or marked: cross-jumped call copies (`COUNT tail`: the surplus
    copies share their continuation, jumps followed), a test read through
    an operand one side cannot resolve (`INVERT?`/`CONST?`/`OP?`), two
    constants both sides test (`CONST` dropped: the sites pair across
    arms), the same value compared another way (`lea r,[x+1]; test r,r`,
    `inc`/`dec`/`sub K` chains, `neg`, `test ah,K` = `test eax,K<<8`,
    `setge; dec; and 0x19; add 0x19` against `setl; dec; and 0xe7; add
    0x32`). Still read as hits, all layout: a re-test after jump threading
    (draw_labs `a == -1` lands on the `a >= 0 && b >= 0` re-test), an
    induction variable chosen differently (load_vehicles counts `row`, ours
    `chassis`: `lea eax,[edx+9]; cmp eax,9` against `cmp ecx,2` on the
    derived source_y), an inline expanded on one side only (MapWin::click
    draw_map, DesignWin::select_special's five-argument Popup::start),
    teardown arms (destructor INVERT/COUNT: EH layout), ICF class twins
    (`AlphaNetStringStruct` against `StringStruct`).
  - *Not found* (each tried with 3..5 spellings): base_yield 0x004E8930,
    where range()'s two clamp arms sit after the NEXT range() expansion's
    `jmp` (ob_inl_loop puts them right after the in-line arm); action_destroy
    0x004CAF94, where the BOMBFAILED block (reached only from the
    `is_assault` test, after a `goto`) is laid out after found_terra's first
    `jmp`, ours at the end; time_warp 0x005AF155, where the first expanded
    get_plan's two results share one store (`or eax,-1; mov [ebp-0x70],
    eax`, the found path `je`s onto the store; ours stores -1 directly);
    ReportWin::on_key_click 0x004983FC, where the shared set_modal tail is
    hosted in the F-key chain copy (ours: the rotation switch's case 7);
    Net::join_session 0x0062EA1F, where `return local_player_id_` is laid
    out after close() and the return-0 teardown (the if/else spelling gets
    that order but moves the return-0 host to an earlier site, 22 -> 42).
- **Open: when identical epilogue copies are merged in a large function**
  (facility_avail, 16 `ret` in the image, 15 in ours; hand-over to the
  return-tail work). There every invertible `jncc over | copy` is turned into
  `jcc shared`, the non-invertible ones keep their copy, and the image keeps
  one more copy than ours: the first guard `if (!facility_id) return false;`
  (`jne +9 | copy` at 0x5BA0EC, where ours has `je` to the last block). What
  was measured on our source: (1) no spelling of that guard changes it
  (`== 0`, `return 0`, `return facility_id`, `else`, a named local, the
  guard doubled); (2) the merging is all-or-nothing per function and not
  monotonic in size: cutting statements off the END of the function gives
  merged (full, and minus the last statement), then 26..15 separate copies
  for the next nine cuts, then merged again; the three-statement head alone
  gives separate copies, as do all the probes; (3) appending guards after
  the last statement leaves the outer `return false` guards (33-34 branches)
  on the copy of the last *original* statement, moves the eight branches
  that come out of has_fac / has_project expansions to the new last block,
  and gives the appended guards own copies, although all these tails are
  byte-identical. So the merge is not driven by the tail's bytes, by the
  register holding the tested value (ebx, esi and edi variants on both
  sides) or by distance. Unit pads do not move the function (census above).

Finish-line round (par-1 2026-11-19, round 14):
- **A `goto` into a shared case tail is one copy at allocation; the original
  wrote the tail out** (high; ProdPicker::on_right_click 0x00493EA0, exact).
  The obsolete case ended `goto refresh;` into the loop case's `calculate();
  redraw(); break;`. The image cross-jumps that tail too (0x494112 jumps into
  the refresh block), but cross-jumping runs after allocation: the written-out
  copy's scratch loads were allocated and rotate the cursor (S1), so the next
  case's vtable load sits in edx (0x494114) where ours had eax, and that case
  no longer merges into the last case's identical call. When an image keeps
  a call ours cross-jumped and the scratch phase differs only after a block
  that ends in a `goto` to a shared tail, write the tail out at the goto
  site. armwo (t) and gotowo cover tails ending in `return`; tails ending in
  `break` inside a switch are not in either (base_editor's refresh_population,
  netdaemon's move_veh_ack/next_found and base_build's L_4fef0c are the other
  sites in non-exact functions; base_editor's moves the wrong way).
- **The named-sum `lea` order (planet_busting rule) also closed world_site
  0x005C4FD0**: `const int column = x + RadiusOffsetX[i]; wrap_map_x(column)`
  in the inner ring gives the image's `lea esi,[ecx+eax]` (0x5C50F7); the
  outer ring stays anonymous. Neutral elsewhere: CouncWin::draw_leader's two
  `lea` (`rank = preceding + rankBase + 1`: six orders, groupings, `rankBase
  + 1` folded, an unnamed rankBase, a non-const one), SpriteBox::draw_item's
  `lea ecx,[edx+ebx+4]` (`field_54_ + x + 4`: seven orders and two named
  forms) and CouncWin::set_vote's `lea ecx,[ebx+0x4610]` against ours `add
  ebx,0x4610; mov ecx,ebx` (`buttons_[5].show(0)` as a reference, a pointer,
  `buttons_ + 5`, an early return, a function-scope counter).
- **BaseWin::base_editor 0x0041C7A0's fifteen label sites are all written
  out in the image** (every Labels load/index pair follows the S1 cursor:
  ecx/edx, eax/ecx, edx/eax ... from 0xD40 to 0xE18). Our six label_get
  sites are budget stand-ins: writing them out fixes the label region but
  the freed /Ob2 budget expands two later teardowns that the image calls
  (24 -> about 60 rows). The open question is the original's spend, a 2.8
  budget problem, not an S7 choice; the best pair of flips is 18 rows.
- **A shared arm tail entered by `jmp` means every arm computed its input
  the same way** (par-1 2026-11-19, BaseWin::base_editor_fac 0x0041B790, 7
  rows -> exact). Image: three if/else arms each end `mov eax,<label>; jmp
  T` (the last falls into T), T = `push eax; mov ecx,StringTable; call get;
  push eax; push StringTemp; call strcat`. VC6 cross-jumps identical arm
  tails only when the instructions are identical, including the register:
  ours passed `StringTable.get(Labels.strings_ptr[k])` in arms 2/3, whose
  label load went to edx with a push per arm. Passing the label through
  `table_text(...)` (alpha.h, an inline returning `StringTable.get(p)`) in
  every arm puts the value in eax and the tail merges. The same arm also
  needed its other string in a named local like its sibling (`char *again
  = Facility[f].name;`) so the common `StringTemp[0]=0; get` prefix hoisted
  above the arm test. Rule: when the image's arms end in the same register
  and `jmp` to one tail, spell the tail's input with the same expression
  shape in every arm (same helper, same named local); when ours merges a
  tail the image repeats per arm, the inverse: the arms differ in spelling.
- **BaseWin::init 0x0041E8D0's last slot tie is packing order** (12 rows).
  Image: [ebp-0x10] = window_height + the sprite-run pointer, [ebp-0x14] =
  load_inverts' inlined `dest` + the camera's `yy` + the strength-reduced
  loop counter. Ours packs dest and yy with the sprite pointer and
  window_height with the counter. Neutral: all 720 orders of the six
  function-scope declarations, the clamp of window_height as a ternary,
  `<=`, a self clamp, no `available`; left/top named or inline in the
  GraphicWin::init call; x/yy/z declared together or as self-negations
  (StatusWin's spelling gives 113); declaration order inside load_inverts;
  the sprite loops indexed instead of walked (92-112: y then takes the
  pointer's slot). The remaining push/store schedule at 0x41F3C2 is neutral
  to all 24 orders of the camera-angle stores and four pointer/cast forms.
Re-tests and stack loops (par-2 2026-11-19, enemy_move 0x0056B5B0)
- **A re-test the image keeps was spelled differently from the test before
  it.** When two tests of the same value are textually identical (same
  operands, same operator, same constant), VC6 threads the jump of the first
  past the second and the re-test disappears; a non-identical spelling of the
  same fact (`home_dist < 4` then `home_dist <= 3`, a field against its local,
  a helper against its expansion) keeps the second compare and its block.
  So a compare in the image that ours threads away names a second spelling,
  not a reordered condition (the radius_move guard after the order-9 block,
  7710690f98).
- **A stack loop that tests the id for `< 0` again before reading the next
  link is `veh_next_stack_id(id)`**, not `id = Vehs[id].next_veh_id_stack`:
  the inline's own `id >= 0 ? … : id` adds the re-test (`test reg,reg; jl`
  before the `mov reg,[reg*0x34+next]`). Look for it in every walk the image
  shows with two sign tests per step (enemy_move's sea-transport scan
  0x56F0BD and coast-region passenger check 0x576183, 3134ae26b7).
- **`jcc TARGET | jmp JOIN` at the end of an if-arm is a separate last
  statement; `jncc JOIN | jmp TARGET` is an `||` chain.** When an arm ends
  in `if (A || B || C) continue;` (or `goto X;`) and its join is not the
  next block, VC6 emits the chain's last term inverted over a `jmp` to the
  target; written as `if (A || B) continue; if (C) continue;` the last
  term jumps straight to the target and the arm ends in `jmp JOIN`
  (pc probe: `base` against `sep`/`base1`; enemy_move 0x56F190, 0x57518D,
  0x57196F). Read the image's last jcc of the arm: if it goes to the
  continue/goto target, split off the last term.
- **A loop that leaves through `jge EXIT | jmp TOP` with a guard, where ours
  is top-tested (`jge EXIT ... jmp TEST`), had a test after the loop**
  that VC6 threads: a scan whose found path `goto`s out and whose
  fall-through runs the not-found code was written `for (...) { if (f)
  goto next; } if (i >= n) { not_found; }` (enemy_move 0x572BE1).
- **A return that the image `jne`s to the function's final return block
  is `goto` to that label**, not a `return` of the same value in line
  (enemy_move 0x575B1C). The final block's `xor eax,eax` before the pops
  with a second entry after it (`jne 0x579201`) is a `return 1` path that
  arrives with eax = 1 already.
- **Equilibria: a correct split can flip the whole function's register and
  slot state.** Judge such a flip by the slot agreement of `slotmap.py`
  (aligned rows whose image and our `[ebp-N]` agree), after block layout:
  enemy_move's 0x577C67 split closes four layout hunks and opens five, and
  swaps the two hottest slots (1661 -> 1345 of ~2800); it was not kept.
- **Switch on a byte (par-4 2026-11-19).** `if (x == 1)` on a `uint8_t`
  field compiles to `cmp byte ptr [..],1; jne`. The image's `xor edx,edx;
  mov dl,[..]; mov ecx,edx; dec ecx; je` (or `sub ecx,K; je`) is a switch
  with `case 1:` (or `case K:`): the switch widens the byte into a register
  copy and lowers to a subtract chain ("Switch lowering" above). Write
  `switch (field) { case 1: ...; break; }` even for a single case
  (Console::go_to 0x004D6C4E, bbc27d1969: 286 -> 266 rows).
Accessors, ternaries, accumulation, shared exits (par-5 2026-11-19, library)
- **A null-testing accessor costs one `test; je` per call site.** An inline
  like `current_entry()` (`head ? head->payload : 0`) re-tests the pointer
  at every use even when the caller already tested it; where the image
  tests once per arm, read the member through the already-tested local
  (BasePop::calculate_dimensions 0x603F44/0x603F64, jcc 70 -> 68).
- **`x = c ? a : 0` has a `jmp` over the zero arm; `x = 0; if (c) x = a;`
  has none.** Count the jumps around a conditional load: `test; je L0;
  mov eax,[..]; jmp J; L0: xor eax,eax; J:` is the ternary
  (BasePop::calculate_dimensions 0x603DAC wrap_kind).
- **A member stored after every partial sum is accumulated in place.**
  `m = a; if (c) m += b; m += d;` stores the field after each step; a local
  sum assigned once stores it once at the end (BasePop::calculate_dimensions
  0x6041D4-0x604262, field_30C0_; with the two rules above 263 -> 235 rows).
- **One expansion of a cleanup inline that two paths reach with a `jmp`
  is one call at a shared label.** When the image expands an inline once
  at the end and another branch jumps into it, the source called it once
  after a `done:` (or a common fall-through), not once per branch: two
  calls give two expansions (Buffer::line 0x005E15F0 free_data(1), jseq
  hunks 2 -> 0).
Counters and tie arms (par-5 2026-11-20, councwin)
- **The operand order of `lea r,[a+b+K]` over two register locals follows
  variable identity, not the source operand order.** CouncWin::draw_leader
  0x00425DB0 printed `lea [edi+eax+1]` where the image has `[eax+edi+1]`
  (rankBase + count) under all six operand orders, groupings, member
  stand-ins and every pad count; writing the three sequential faction-count
  loops with one reused counter variable and one reused loop index (instead
  of a fresh local per loop) gave the image order and made it exact
  (1f82d2697e). When a `lea` or `add` over two locals is mirrored and
  reordering the expression does nothing, merge sequential same-role locals.
- **A `do { } while (++i < N)` array clear supplies the 3.4 node counter.**
  CouncWin::make_resolution 0x00424410's remaining PlayersData loop printed
  the add order of `[a1c] + i*0x833` mirrored; any member stand-in before
  the earlier `option_votes` clear fixed it, and the original-looking fix is
  that clear written `i = 0; do { votes[i] = 0; } while (++i < MaxPlayerNum);`
  instead of a `for` (5f28ecbb58). Before adding a stand-in node, try the
  do-while form of a nearby constant-count loop.
- **A tie arm that jumps into the middle of the other arm's store is nested
  under `count == greatest` and stores the same value.** make_resolution's
  `governor == candidate` test jumps to `mov edx,ecx | mov [a20],eax` inside
  the new-leader arm: VC6 cross-jumped two identical `field_A20_ =
  candidate` stores. It needs `if (count == greatest) { if (gov ==
  candidate) A20 = candidate; else if (gov != A20) A20 = 0; } else {
  greatest = count; A20 = candidate; }` (21 -> 8 rows); a flat chain
  (`count > greatest` first, tie as `else if`) stores a different value and
  keeps two copies. Read a cross-jump into an arm's tail as a statement the
  source repeated, and arrange the arms so the repeated stores end both.
- **Cross-jump host: the copy LAST in block order before layout hosts, and
  every other copy merges only into the host** (probe
  `analysis/probes/xjhost/run.py`, 2026-11-20). Three arms with the tail
  `f1(7); f1(8); return 1;` (ret3, join3, fallend, loop, ifelse/ifelse_n):
  the last arm in source keeps the tail, the earlier arms `jmp` into it.
  Block order is source order, except:
  - a block reached by `goto` keeps its source position for the choice of
    host, then layout pulls it into its test's fall-through, so the host
    can sit FIRST in the image (goto_late, goto_mid, goto_early: the `one:`
    or `two:` block written after the final return hosts and is laid out
    right after its `jne`);
  - a compare-chain switch (`sub eax,K; je`) lays its case blocks out in
    descending case value and hosts in the lowest value's case, whatever
    the source order (switch3, switch3r, sw_529, sw_952); a jump-table
    switch keeps source order and hosts in the last case written (sw_dense).
  Each other copy jumps into the host at the start of its longest
  identical suffix WITH THE HOST; copies never merge with each other: in
  sfx12 arms 1 and 2 are identical (`use; g2 = 6; T`) and both keep their
  `use; g2 = 6` privately because the host (arm 3) lacks the store; sfx13,
  sfx23, sfx_only1/3 the same. Unequal pending pops never merge (suffix12).
  Reading the image: the copy that holds the shared code is the last one
  in source (or the lowest case of a compare-chain switch); when the
  image's host is a different arm than ours, the arm order in the source
  differs (or the host arm was reached by a goto), and when two non-host
  copies in the image share code with each other, they were one statement
  reached by goto, not two copies. Open counter-example: ReportWin::
  on_key_click 0x00498280 hosts the shared `set_modal` arm of nine
  report_* expansions in the F7 (report_security) copy of the F-key
  if-chain (0x498402), not in the later F8 copy or the rotation switch's
  copies; ours hosts it in the switch's case 7 (22 rows, jcc/jmp counts
  equal). An F-key `switch` costs 110 rows; not resolved.
- **A switch arm that falls into a shared tail the other arms jump to is
  a flag-tested tail after the switch, not a goto label** (par-4
  2026-11-20, Console::on_key_click 0x005178C0, 418b0cfc68). Eight
  direction cases set `a2` and `goto move_direction;` into a label after
  the first case: VC6 lays the label after the last goto's block (case 7)
  whatever the case order or label position (five orders measured
  identical). The image's first case (0x0051B0B2) falls into the tail.
  `a2 = K; break;` in every direction case, `scroll = 1; break;` in the
  other cases and `default`, and `if (!scroll) { tail }` after the switch
  give it (7 -> 2 control-flow hunks, 555 -> 239 rows with one operand
  order fix); a `goto map_done` from the other cases instead of the flag
  compiles like the label form. Same mechanism as 2.6's council_action
  rule, inside a jump-table switch.
- **A switch whose out-of-range `ja` lands past a case's last store
  ends that case with the store and `break`s into the code after it**
  (par-4 2026-11-20, communicate 0x0054FFD0, `analysis/switchtail.py`
  HOST). Ours had `if ((unsigned)c <= 7u) switch (c) {...}` with gotos to
  two labels after it (`after: FAD0 = 0; loop: ...`), so the default
  reached the store. The image's default goes to `loop` and the store
  is case 7's tail (cases 1/2 `if (mode == 6)` jump into that copy):
  plain `switch` with `break` arms and the store written in both cases.
  Same family: MainInterface::init 0x00459680's ten label cases share
  one `push eax; mov ecx,StringTable; call get` tail only when every
  case passes its offset through an inline (`label_get`, `table_text`);
  a `StringTable.get(expr)` case pushes from its own register and keeps
  the push.
- **`if (A || B) { ...; return; } C;` lays the arm after C; the image's
  arm in place (second test `jne` to C) is the test computed as a value**
  (par-4 2026-11-20, Console::on_key_click 0x00517C74, 239 -> 217, jseq
  0 hunks). Else-if, else-C, negated tests and gotos either way all keep
  the arm last (the layout is local: truncating the function keeps it);
  `int go = A || B; if (go)`, the same `||` returned by an inline helper,
  or `A ? 1 : B` give the image's layout.
- **Flag tests: threaded or materialised** (par-4 2026-11-20, probe
  `analysis/probes/flagtest/run.py`, 60 variants). VC6 threads a flag set
  to a constant on paths and tested at their join (`int shown = 0; if (a)
  { if (b) { ...; shown = 1; } } if (shown) {...}`): each set jumps into
  the arm, each miss past it, and the test disappears. Type (int, bool,
  char), `== 1` / `!= 0`, else-assignments, `++`, a preset 1 cleared on
  the misses, a label, an enclosing block or switch, `do { } while (0)`,
  `for (;;) { ... break; }`, an inline helper returning 0/1, a global or a
  static all thread the same. It is kept, as `mov r,1; test r,r; je` with
  every miss jumping to the test, only when (a) a real loop sits between
  the stores and the test: a one-trip `for (i = 0; i < 1; i++)` (or
  while/do with `i < 1`) is removed after threading and leaves exactly
  that shape in a scratch register, `xor r,r` hoisted before the first
  test; (b) a call or other code separates the join from the test, or the
  flag is read again later (callee-saved register; the FIRST test after
  the stores is still threaded); (c) volatile or address-taken (reloaded
  from the slot). A computed bool (`f = m != 0; if (f)`, `c ? (m != 0) :
  0`) is `setcc; test; je`. Image fingerprint `mov ecx,1; test ecx,ecx;
  je` after an arm whose misses all land on the test is (a): ReportWin::
  draw_intel 0x004A2BE9 and social_ai 0x005B4B8B both read
  `soc_ideology_*[0]` inside a one-trip loop (draw_intel 332 -> 249 rows
  and the frame, social_ai 256 -> 197 and the frame). `analysis/mattest.py`
  screens `mat`/`set` and the `||` layouts tree-wide (40 s).
  Else-if re-tests of the same compare (`if (p == g && v > 0) {} else if
  (p == g && f())`) are threaded in every spelling nested under `if (net)`
  (inline compare helpers, `!= 0`, `>= 1`). An image that keeps the
  re-test (double_cross 0x0050BE1A: both misses land on `cmp ebx,eax`)
  repeats the outer flag in each condition: `if (net && p == g && v > 0)
  {} else if (net && p == g && f())`. VC6 threads the misses past the
  known `net` test only, one hop, and stops at the compare (49 -> 24
  rows, jseq 0 hunks; probe e_hop).
- **An `||` in an else-if chain whose arm the image lays in place
  (`je arm; je arm; jne else; arm:`) is the same value form**: `(A || B ||
  C) != 0`, a bool local or `A ? 1 : B` all compile alike (stack_veh
  0x005B93D8, 10 -> 3 rows). `analysis/orsweep.py <unit.cpp> <addr>
  [--and]` tries the `!= 0` form at every `||` (and `&&`) test of a
  function alone, ranked by sdeck's structural key.
- **Host orientation: the last-copy rule flips to first-copy with
  constructs elsewhere in the function** (probe
  `analysis/probes/xjhost/orient.py`, 2026-11-20 par-5). Three copies of
  `pop(label, K, 0, pcx); gs |= 4;` before returns (monolith's shape):
  the last copy hosts by default, but the FIRST copy hosts (the others
  jmp into it) when the function also has one bare `if (A || B) return;`
  (anywhere: before or after the copies, `||` of calls too), or when the
  first two copies are the then/else arms of one if/else. Each such `||`
  bare return toggles the orientation (two: last again, three: first);
  separate `if (A) return; if (B) return;`, `&&`, a `||` return with a
  statement before it, or one nested in another `if` do not toggle.
  Without a toggle, a last copy that is not the function's fallthrough end
  (an else-arm with a then-arm laid after it) loses to the copy before it.
  So "the image hosts last, ours first" (`xjhostscan.py` ORIENT-LAST) is
  not an arm-order fix: monolith 0x0057A050 hosts its first MONOLITHHEAL
  copy and loses its StringTemp host, and only deleting the proto `||`
  return, the order-0x18 block and the human/heal middle together (not any
  one or two of them) restores the image's last-copy host; separate proto
  ifs, an if/else wrapping, returns in the heal arms and an early-return
  MONOLITHNO arm are all neutral (27 rows). The toggles in a real function
  are therefore not all known; Buffer::copy 0x005DFF00 (arms 2-3 keep
  their own `free_data; return 3` in the image, jump into arm 1's in
  ours) and Net::join_session 0x0062E540 (the image hosts return 0 after
  close() and the epilogue after the success return; every spelling that
  lays the success block after close() moves the return-0 host to the
  first unconditional return 0, the Open failure) look like the same
  family. Open.
- **A loop whose test failure and whose last arm end in the same tail:
  `for (;;) { if (!c) { T } ... T }` and `while (c) { ... break; } T` give
  different cross-jump hosts** (par-5 2026-11-20, Net::send_packet_type
  0x0062F8A0, 615ec1a082). With `while (c)` and `break` there is one copy
  of T after the loop, every exit jumps to it and it hosts. The image's
  `free(packet); return 0;` is hosted in the MIDDLE copy (the retry
  failure inside the loop) and the loop-top `NetDirectPlay` test jumps to
  the post-switch copy: the source wrote T at the top test and again at
  the bottom of the body, `for (;;) { if (NetDirectPlay == 0) { free;
  return 0; } ...; switch; free; return 0; }`, as in the exact sibling
  sub_62F390. 42 -> 0 (exact); Net::send 0x00630080 107 -> 35 with the
  same form plus the sibling's `case 0: continue; case 1: ...; break;`
  switch with one shared tail after it. Read an image whose loop test
  jumps forward into a copy of the tail that also ends the loop body as
  T written twice inside `for (;;)`, not a `while` exit.
- **A jcc-target arm after an exiting arm, when a `goto` target follows
  it in source, is laid out LAST** (probe `analysis/probes/fwdjmp/run.py`,
  par-5 2026-11-20). `for (...) { if (hit) goto found; } A0; if (f) { A;
  goto fin; } NA; goto fin; found: F; if (x) {T} else if (y) {E1} else
  {E2}; ...; fin: ...`: VC6 emits `A0 | A | F T jmp E1 E2 ... | NA | fin`
  (found is pulled to the first unconditional jump after the loop, the
  queued NA waits for the end). It does so in all 45 shapes tried: NA as
  `else`, `else if`, negated, nested or `goto`-inverted test; NA and A
  ending in `return` with distinct values (no shared join); found's
  then-arm ending in an explicit `goto j`; the scan as `break` + `if (i >=
  19)`/`i == 19` (fail first or found first), as a flag, with found inside
  the loop, or written after the final return; extra `goto fin` sites
  before and after; no loop (`if (g) goto found;`); one or two bare `if
  (A || B) return 0;` toggles in front (tg1..tg5; an `||` in the flag test
  itself, `if (a || g8)`, only swaps A and NA). action_destroy
  0x004CAA50's image instead lays its NA (`triad == AIR && faction ==
  player` BOMBFAILED, 0x4CAF94) right after found's FIRST `jmp` (the
  world_climate/draw_map arm, 0x4CAF8F) and before that if's else-if, and
  its ATESTUFF arm, not NA, falls into `finish_destroy`; the ten real-source
  spellings of the same list keep NA at the end (50 rows best, the
  current source). The unpooled literal order (Arty Failed, ARTYBOMBFAILED,
  BOMBFAILED, BOMBSUCCEEDED) confirms NA precedes found in source. So the
  image's placement is not a spelling of the fail path or the scan; the
  remaining candidates are whole-function. Do not retry fail-path
  spellings without new evidence.
- **Label block placement: the LAST site in source that reaches a label
  decides where VC6 lays it out, and only some site forms move it** (probe
  `analysis/probes/failplace/run.py`, par-6 2026-11-20, on upgrade_prototype
  0x004EFE80's and proposal_menu 0x0054DCF0's skeletons). A label block
  (`fail: return 0;`, `fail: x = 0; done: ...`) with a fall-through
  predecessor and later gotos is laid at its fall-through predecessor (its
  source position) unless the last site in source is a *claiming* form, in
  which case VC6 inverts that site's last test and lays the label right
  after it; a structural predecessor that then has to jump to it gets a
  private copy of a pure return block (tail duplication: one extra `ret`).
  Claiming: `if (c) goto L;`, `if (A && B) goto L;`, `{ S; goto L; }`,
  `if (A || B) { S } else goto L;`, a bare `L2: goto L;` label block, and
  backward gotos of these forms (written after L). Not claiming: `if (c) {
  S } else goto L;` with a single test and an `S` that falls on, `if (A ||
  B) goto L;` (every term jumps), and an if/else join that falls into L.
  Only the last site counts: earlier claiming sites keep direct jumps when
  the last one does not claim (q10/q11/gfail), and `if (c) return 0;` copies
  never merge into the label (the `r*` variants keep one epilogue each).
  A chain `fail: ...; done: ...` moves as one unit with fail's claim; a
  later claiming site for `done` splits it (q9: `fail: ... jmp done`).
  Reading the image: when every site jumps straight to a label that sits at
  its fall-through predecessor, the original's last site was a non-claiming
  form; when ours lays the label after its last site, rewrite that site
  (e.g. `if (x < 0) goto fail; S` -> `if (x >= 0) { S } else goto fail;`).
  Applied: proposal_menu's commlink `FA5C < 0` site (85 -> 78 rows, `fail:`
  back at the ATTACK fall-through). `done:` then went to the end of the
  function (after the final return) although its last site is a
  non-claiming `||` goto; the probe reproduces it (d2, e1-e7) only when the
  fail site is nested in an `if (c == 13) { ... }` whose false edge joins at
  the done site (k1/k3, without that nesting or without fail:, keep it).
  Un-nesting it fixes it: `if (c != 13) { goto skip; } ... skip:` keeps
  done: at its fall-through (proposal_menu 78 -> 1 rows). An if-block
  wrapping a fail site whose false edge joins the label's last site is the
  form to remove; an empty then-arm (`if (!f()) {} else goto L;`) is not a
  non-claiming site (shoot 0x00500560: no change).
  upgrade_prototype: our last site is the no_energy `if (set_flag && ...)
  goto fail;` (claims; 27 rows, one extra `ret`); every rewrite of those
  sites tried (else-goto, `||` else, nested, returns, no_energy inline or
  before fail:) still claims or worsens the function, so the image's
  source order around no_energy is not known yet.
- **The loop counter VC6 keeps is the variable the body's tests read.**
  load_vehicles 0x00454270 loops `chassis` 0..18 with `row = chassis - 9`;
  the image counts `row` (edx from -9, skip test `lea eax,[edx+9]; cmp
  eax,9`, latch `cmp eax,0x13`, `cmp edx,9` for `row == 9`). `if (chassis
  < 9) continue;` keeps chassis as the counter and VC6 folds both tests
  into the derived source_y IV (`cmp ecx,2`, `cmp ecx,0x304`); `if (row +
  9 < 9)` gives the image's loop (cb1f79b9be; `row < 0`, `chassis - 9 <
  0`, row-counted `for` headers are worse). When the image compares a
  derived `lea` against a constant where ours compares another IV, write
  the test on the variable the image counts.
- **A flag re-tested in an else-if chain is not threaded; a nested test
  is.** MultiWin::draw_player 0x00478630: `if (in_turn && !F && (A || B))
  {} else if (in_turn && F) {X} else if (F) {Y} else {W}` re-tests F after
  B fails (`cmp [F],0`); every polarity spelling and a local copy of F
  keep the re-test. The image's `jne X | A | B | je W | jmp past` is
  `if (in_turn) { if (!F) { if (!A && !B) goto W; } else {X} } else if
  (F) {Y} else { W: ... }` (78b0474751; W gets two entries, one reloading
  a register).
- **A test that the image makes and then ignores is a ternary with equal
  arms** (high in one real function; ReportWin::draw_labs 0x00498B30,
  2026-11-20 par-4). The image loads `datalink`, `test`s it and pushes the
  same value on both paths with no branch (0x0049AFEF..0x0049AFFB): VC6
  folds `c ? X : X` inside an expression but keeps the test. Two identical
  `if (c) S; else S;` arms keep their branches (test; jne; S; jmp; S). So
  write `f(c ? X : X)` at the argument the arms share (here inside
  StringTable.get), not around the call (blocks 282 -> 279 = image, 0
  control-flow hunks). Checked for a plainer spelling: `char *n; if (c) n =
  X; else n = X; f(n);` compiles to the same bytes (either is fine); an
  empty `if (c) {}` / `(void)c;` beside the plain call drops the test (105
  rows worse).
- **An else-if whose test the image never re-tests is a goto target laid
  after the first arm** (draw_labs, same day). The image tests
  `ids[1] >= 0 && ids[2] >= 0` once, jumps to the two-arm, and lays the
  one-arm first with only its own `ids[1] + 1 != 0` test. `if ((A || B) &&
  C) one; else if (!A && !B) two;` re-tests both ids; a nested if/else or
  the chain in the other order lays two first; `goto two; ... goto done;
  two: ...` is not it either. Only `if (!A && !B) goto two; if (C) { one }
  if (0) { two: two-arm }` gives the image's tests and order (blocks
  284 -> 282, first diverging block 3383 -> 9407). Rechecked for a plainer
  spelling: `if (A && B) two; else if (C) one;`, `if (!(A && B)) { if (C)
  one } else two` and `if (A < 0 || B < 0) ... else two` compile to one
  and the same code, 10 rows from the goto form: the source order of a
  plain if/else does not reach the image's layout, so the goto stays.
- **Small polarity / width checks worth a jseq pass on every rung-4
  function** (2026-11-20 par-4; `analysis/jseq.py <unit.cpp> <src> <addr>`
  prints the jcc/jmp hunks): `mov eax, A; je; mov eax, B` against ours
  `jne` is the ternary written with the other polarity (Console::upgrade's
  `Language ? .. : ..` -> `!Language ? .. : ..`); `dec ecx; js` against
  `dec; test; jl` is an int-promoted byte (`range(byte - 1, ..)`) against a
  uint32_t copy; `cmp esi, [edx]; jle` against `cmp; jge` is the
  comparison written with its operands swapped (diplomacy_check
  `other > faction`).

### 3.8 VC5 (Caviar/VOX) x87 rules (2026-11-19 par-5)

The Caviar/VOX units compile with VC5 /O2 /Ob1 (COMPILER_EVIDENCE.md,
"Caviar library compiler"). Probes: `analysis/probes/vc5x87/` (`v5.py`
compiles probe text with that recipe; `x87sym.py <unit> <addr> [--ours]
[--names reg+off=name,..]` replays a function's x87 code symbolically and
prints every store as an expression tree in the operand order the code used,
with `{...}` around values reloaded from a slot the function stored, so the
image's and our expression trees and spills can be compared directly).

- **Commutative x87 operand order is not the spelling.** `a*b`, `b*a`, a
  named local copy of either operand and the declaration order of the
  locals all give the same `fld X; fmul Y`; so do `+`. VC5 even
  reassociates: `G1*G2*G3`, `G3*G2*G1` and `G1*(G2*G3)` compile to the
  same `fld G3; fmul G2; fmul G1`, and `G1*G2 + G3*H1` equals
  `G3*H1 + G1*G2`. In vox_matrix_matrix_mul all 48 spellings of a cell
  (3 orders of the products x operand flips) collapse to four results, set
  only by which product is added last. `-` and `/` keep source order.
- **What decides the order:** two loads off the same base go higher
  displacement first (`s->a * s->b` with a at +0 loads +4 first; `p[0] *
  p[2]` loads `[eax+8]`; a reversed struct loads its +8 member first).
  Loads of different symbols are ordered by a symbol key that is neither
  the declaration order nor the spelling (`extern float G1, G2` loads G2
  first; `extern float H2, H1` loads H1 first; parameters `p, q` and `q, p`
  both load q first). So read an image operand order as a compiler key,
  not a source fact, and do not search spellings for it.
- **The key moves with earlier declarations in bands.** Unused
  `extern int f(int);` / `extern int v;` declarations placed before
  vox_matrix_matrix_mul flip its rows in bands of about eight
  declarations (54 -> 22..30, period 16), and vox_matrix_inverse cycles
  through four states (139/128/121/126, period 32); a probe's
  `x = x * p->scale` flips once (load scale first -> load x first) after
  about 90 earlier symbols and stays. caviar_apply_rotation does not move
  with 0..39 declarations before it, nor with any of the 720 orders of its
  six sine/cosine locals (best 87 rows, frame always 0x18 against the
  image's 0x14), so its residual is not the key.
- **Every named float local is stored.** VC5 writes each named float
  local to its home with `fst` as soon as it is computed, even when the
  value stays on the FPU stack and is used from there; a common
  subexpression without a name is kept on the stack or spilled to an
  unnamed slot by stack pressure. Dead parameter slots are reused as
  float homes (caviar_apply_rotation's image stores two of its six trig
  values at `[esp+0x18]`/`[esp+0x1c]`, the angles/matrix argument slots).
  So the count of stored float slots in the image is the count of named
  float locals plus spilled common subexpressions: apply_rotation's image
  has 7 (six trig values and `sz*sy`) and keeps `cz*sy` on the stack;
  ours spills `cz*sy` too (8 slots, frame +4). Naming `cz*sy` (93 rows),
  writing the trig values without locals (154), or double locals (101) do
  not give the image's frame.
- **Scheduling around integer code is not the key either.** In
  vox_draw_scene the nine `part[i][j] * node->scale_` products interleave
  with the callback test (`mov eax,[edi+0x58]; add esp,0xc; test`) one
  load earlier in the image than ours. Hoisting the callback load (before,
  inside or after the products), `*=`, a scale-first spelling, a loop, and
  0..30 declarations before the function are all neutral or worse; a probe
  of the same block (with and without the loop around it) always places the
  integer code after the ninth load, like ours. Unresolved.
- **uV is VC6, not VC5.** movie_media.cpp under VC5 (/O2 /Ob1 or the
  unit's /Og /Oi /Ot /Oy /Gs /Gy /Ob1) gives 7 exact functions against
  VC6's 29 (three bodies patched to return a value, which VC5 requires:
  C2561/C2202 on int functions without a return).

## 4. Measured and neutral (do not retry without new evidence)

### 4.0 Rung-ladder census: the work list (2026-11-19, `analysis/rungs.py`)

`uv run python recovery/tools/analysis/rungs.py` (75 s tree-wide with
FE_JOBS=5; table in `build/recovery/rungs/rungs.txt`, records in
`rungs.json`) gives every function that is not fe-exact its FIRST differing
rung in the AGENTS.md order: 1 EH (FuncInfo states, parents, funclets, try
blocks), 2 calls (named call multiset), 3 frame (size, ebp/esp, EH
registration), 4 layout (block count, return tails, block-order signature,
prologue-save class), 5 slots (frame-slot map), 6 regs (saved set, or rows
that vanish with register names masked), 7 bytes (same registers: operand
order, selection, schedule). Rerun it after a batch; the numbers below are
head fbbf5b225e.

Non-exact functions (347 functions, 59227 rows):

| rung | functions | rows | largest |
|---|---|---|---|
| 1 EH | 16 | 6159 | order_veh 0x005947C0 (2133), probe 0x0059F120 (1292), PickTech::pick 0x00486510 (825), Console::go_to 0x004D61A0 (443), call_council 0x0052C880 (259), Console::terraform 0x004D3470 (250) |
| 2 calls | 26 | 6215 | battle_fight 0x00506AF0 (1992), spot_all 0x005B6260 (1224), propose_pact 0x00543BC0 (388), tech_trade 0x00541100 (358), MapWin::draw_map 0x0046A550 (345), ReportWin::draw_score 0x004A1140 (303) |
| 3 frame | 16 | 6035 | enemy_strategy 0x00561080 (1889), BaseWin::draw_expenses 0x00412AA0 (1024), NetDaemon::synch 0x00532E00 (432), custom_planet 0x0058C2A0 (404), ReportWin::draw_intel 0x004A2210 (333), NewTechWin::exec 0x00483F90 (331) |
| 4 layout | 113 | 32705 | enemy_move 0x0056B5B0 (5690), base_build 0x004F81A0 (3640), gen_terrain_poly 0x004632D0 (1516), communicate 0x0054FFD0 (1513), air_power 0x005678C0 (1051), consider_designs 0x00581260 (958) |
| 5 slots | 43 | 4480 | NetDaemon::process_message 0x00534400 (546), caviar_build_projection_tables 0x006404F0 (364), ReportWin::draw_ops 0x0049D540 (319), SocialWin::init 0x004B2740 (319), NetWin::exec 0x00482490 (231), base_ecology 0x004F67F0 (184) |
| 6 regs | 106 | 3496 | name_proto 0x0057E040 (196), MapWin::right_menu 0x0046C0D0 (162), DipEdit::init 0x004DB080 (124), CheckBox::draw_item 0x0060F030 (111), RadioButton::draw_item 0x0060D750 (111), MonuWin::init 0x00472230 (110) |
| 7 bytes | 27 | 137 | NetWin::draw_custom_rules 0x0047F960 (16), vox_load_scene_view 0x0063A280 (15), vox_draw_scene 0x00639E60 (10), TexHeap::configure 0x00635050 (9), base_hurry 0x004F3FE0 (8), time_controls_dialog 0x00589330 (8) |

Count-sensitive canaries (4 functions, 30 rows):

| rung | functions | rows | largest |
|---|---|---|---|
| 1 EH | 0 | 0 |  |
| 2 calls | 0 | 0 |  |
| 3 frame | 0 | 0 |  |
| 4 layout | 2 | 19 | sat_attack 0x004A0260 (17), buy_council_vote 0x0053EB50 (2) |
| 5 slots | 0 | 0 |  |
| 6 regs | 0 | 0 |  |
| 7 bytes | 2 | 11 | load_box_sprites 0x00453A70 (6), action_give 0x004CEDE0 (5) |

Broken claims (4 functions, 42 rows):

| rung | functions | rows | largest |
|---|---|---|---|
| 1 EH | 0 | 0 |  |
| 2 calls | 2 | 29 | Caviar::init_class 0x006185A0 (20), filefind_init 0x00600400 (9) |
| 3 frame | 0 | 0 |  |
| 4 layout | 0 | 0 |  |
| 5 slots | 0 | 0 |  |
| 6 regs | 1 | 10 | ReportIf::draw_security 0x004AB600 (10) |
| 7 bytes | 1 | 3 | Credits::unload 0x00428830 (3) |

Rung 4 by cause (`rungs.py`, the `4x` column and `layout.sub`; head
47c61fb0bc + this split). a = return-tail count; b = block count with
equal tails (`jcc` a conditional branch more or less, `jmp` an unconditional
one = a tail merged or duplicated, `mix` both, `tgt` the same branches but a
jump lands inside a block = a tail cross-jumped at another depth); c = same
counts, another order; p = prologue-save class only; `+d` the loop set
differs (headers, back edges, `jmp`-closed back edges, entry by a jump to
the test); `+r` only loop-head reload skips differ (allocation):

| cause | functions | rows | largest |
|---|---|---|---|
| a | 6 | 117 | Buffer::copy 0x005DFF00 (53), upgrade_prototype 0x004EFE80 (27), AlphaNetStringStruct::add 0x00401D80 (13), NetDaemon::init 0x0052DF30 (8) |
| a+r | 1 | 1051 | air_power 0x005678C0 (1051) |
| b:jcc | 4 | 243 | Buffer::line 0x005E15F0 (92), ReportIf::draw_sat 0x004AA970 (54), double_cross 0x0050BCC0 (49), stack_veh 0x005B8EE0 (48) |
| b:jmp | 20 | 5025 | communicate 0x0054FFD0 (1513), ReportWin::draw_energy 0x0049BCD0 (506), ReportWin::labs_line 0x0049B450 (485), BasePop::exec 0x00602600 (303) |
| b:jmp+d | 1 | 167 | prune_protos 0x0057F4B0 (167) |
| b:jmp+r | 12 | 3858 | consider_designs 0x00581260 (958), Path::find 0x0059A530 (876), diplomacy_check 0x0053A980 (704), BaseWin::draw_farm 0x0040F0F0 (386) |
| b:mix | 6 | 3025 | gen_terrain_poly 0x004632D0 (1516), ReportWin::draw_labs 0x00498B30 (605), PickTech::draw_tech 0x00484C60 (303), DiploWin::exec 0x00443F10 (291) |
| b:mix+d | 1 | 3640 | base_build 0x004F81A0 (3640) |
| b:mix+r | 1 | 232 | world_rainfall 0x005C4470 (232) |
| b:tgt | 30 | 4691 | base_terraform 0x004ECBF0 (922), Console::on_key_click 0x005178C0 (633), Console::give_hints 0x00511BB0 (570), Buffer::line 0x005E09D0 (393) |
| b:tgt+d | 4 | 973 | StatusWin::draw_status 0x004B6570 (331), drone_riot 0x004F4FB0 (317), goody_box 0x0057A660 (317), enemy_diplomacy 0x0055F930 (8) |
| b:tgt+r | 2 | 743 | setup_player 0x005B0E00 (591), ReportWin::draw_projects 0x0049EB10 (152) |
| c | 20 | 2865 | time_warp 0x005AEDE0 (675), BattleWin::battle_report 0x00421B50 (357), base_production 0x004F07E0 (289), base_yield 0x004E80B0 (229) |
| c+d | 1 | 5690 | enemy_move 0x0056B5B0 (5690) |
| c+r | 1 | 271 | world_rivers 0x005C38B0 (271) |
| p | 3 | 114 | success_rates 0x0059EEE0 (53), Buffer::write_vertical 0x005DB450 (51), Console::chat 0x00515770 (10) |

No rung-4 function differs in its jump-table count (`sw` equal in all 113),
and only 7 differ in their loop set; 17 more differ only in loop-head
reload skips. The bulk is b:jmp / b:tgt (69 functions, 15,457 rows): the
same branches with a tail merged, duplicated or cross-jumped at another
depth, i.e. the cross-jump and join-host rules of 3.7, which follow
register allocation as often as source shape. Real loop-set cases to read
first: StatusWin::draw_status (image 8 loops, ours 7), prune_protos and
goody_box (the image closes one more loop with `jmp`), drone_riot (ours
closes one with `jmp`).

queue_click 0x00409200, second reading (2026-11-19 par-2): the code between
return sites 1 and 2 is the image's instruction for instruction (only the
`BaseIDCurrentSelected` temporary is ecx/eax swapped at the lock and unlock
calls), so no expansion is missing or extra there. Measured on the site
pattern: every first-level inline instance after a site counts one unit
whatever its size (expression, ternary, one or two ifs, void: same step);
`queue_change_blocked()` between the sites is one such unit for site 1
only (with the field read raw, sites 1 and 2 flip together: `D D` at three
probes, `s s` at four); a two-level dead inline (free outer around 1, 2 or 3
inners of 1, 3 or 6 ifs) between the sites never refuses site 2 alone
(36 variants). The image's closing teardown also expands one small
destructor more than ours (0x409555: the member at `[ebp-0x4184]` restores
a global from its saved field in line; ours calls the destructor of the
object at `[ebp-0x41AC]`), so the image has room left at the end while it
refuses site 2: not a shortage, a reservation held against site 2 only.

Reading rung 1 (all 16 looked at, 2026-11-19 par-2): none is a class or
lifetime fact. Every image/our FuncInfo pair has the same teardown sites
(same root-state sequence, same try count 0); they differ only in how deep
one or more `~Popup`/`~PopMenu` teardowns expand, i.e. /Ob2 pass-2 room
(2.8), which is rung 2 seen through the unwind map:
- ours deeper at k sites, none shallower (too much pass-2 room; the original
  caller spent or reserved more): order_veh 4 of 7, probe 9 of 39,
  PickTech::pick 5 of 8, go_to 3 of 7, terraform 5 of 33, config_game 2 of
  5, call_council (2 late sites at full depth 16 where the image has 6),
  pick_player, base_editor_proj, save_game, POP2 (1 each);
- ours shallower at one site (too little room there): Console::upgrade,
  editor_edit_veh (the first full teardown lacks the image's two
  ~StringList stage states);
- same state count, one site swapped: queue_click (image `D s D D D`, ours
  `s D D D D` with its three probes, `D D D D D` with fewer; a 48-variant
  sweep of probe count x three positions only ever refuses a prefix of the
  sites, never site 2 alone) and pick_pass (image `D D D s D s`, ours
  `D D D D s s`). A refused site between two accepted ones is not reachable
  with dead spend: the original's site 2 had less room than site 1 for a
  reason inside the source between them (queue_click: the `!right` arm up to
  the `lock_base` return).
- popb 0x0048C650 is the one phantom (2.2): ours has 16 unreferenced states
  (55 against 39) from the final if/else whose arms both return. Ending in a
  top-level `return result;` removes them but the freed room makes the final
  teardown full depth (49 states; image final teardown is 7 states, the
  `return 0` before it 16), and a single shared return gives 36. The
  phantom stands in for about one full `~Popup` of spend that the original
  body had before its last return.
POP2 with POP3's exact `popup_history_text` helper is refused one level too
far (teardowns merge, 54 rows): the helper is too dear for POP2 as written,
the raw loop slightly too cheap.

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

- **Counter-subscript signature: no other site** (par-5 2026-11-19).
  `findpat.py ctrsub` and `latectr` over all 5,574 marker spans hit only
  InfoWin::setup_text; do not look for `A[p][k++]` tables elsewhere.
- **Merging two names whose disjoint values already share a packed slot is
  neutral** (par-5 2026-11-19): time_warp 0x005AEDE0 `smallest_pop` into
  `best_score` (733 = 733 rows, same code), setup_text `mode` into `pane`
  (60 = 60), give_hints' sibling 8-direction counters into one `i` (570,
  575 or 581). VC6 allocates registers per web and packs block- and
  function-scope scalars by live range, so a reuse only shows where the
  second value keeps the FIRST one's memory home alive (setup_text's
  `faction` census counter, 3.3). `varslot.py` hits of that kind are noise.
- **Large-function equilibria not moved by local spellings** (par-5
  2026-11-19; image facts for the next attempt, none a one-line fix):
  enemy_strategy 0x00561080 keeps `sites` in bl through the settle block
  (ours spills it: the frame's +4), `path`/`reach` memory-homed with
  their_combat in ebx, `region` reloaded at the loop top with the latch in
  edi (loop header, bad_reg written out, old_plan type, reach/path scope and
  seven `full |=` spellings: 1911 rows each or worse). base_build 0x004F81A0
  at 0x4FC2E6 spills the `PlayersData[target]` product to -0x48 and keeps
  the tech divisor in ebx (ours spills the divisor and holds the product in
  edi across the Reactor loop, whose pointer then lives in memory; a
  block-local divisor and six Reactor-loop spellings are neutral). time_warp
  caches faction_id in edi from 0x5AF8EF through the base-founding arm, so
  the inlined get_plan counters and `smallest_pop` stay in memory (-0x1c).
  order_veh 0x005947C0: four statement orders of the opening assignments
  out of 24 are within 33 rows, the present one is best. repair_phase
  0x00526030 (16 rows, frame 0x28 against 0x24): the image multiplies
  `reactor_id` by repair_rate from memory and gives reactor_id the slot of
  has_fac's `is_built` (-0x24); seven spellings of the product and four
  positions of `has_repair_bay` do not move it (initialising has_repair_bay
  early gives the frame and 60 rows: the image's store is at 0x526439, the
  present position). ListBox::on_key_down 0x0060AF90 (frame 0x14 against
  0x10): the image evicts `count` to [esp+0x10] and holds the saved top in
  edi, the switch index in ebx; writing the DOWN/RIGHT arms on the saved top
  costs rows (165 -> 172..181) and one mixed spelling gives the frame at
  174, with no image evidence that separates the local from the CSE'd
  member, so nothing was kept.
- **A local read through an inline's reference or pointer parameter is
  not address-taken** once the inline is expanded (ListBox::on_key_down
  0x0060AF90, 2026-11-19: `n / d` through `lbx_div(const int &n, int d)`,
  the count written through an `int &out` parameter, a `const int *` local
  and a two-reference helper all compile to the same code as the plain
  local, which stays in ebx). In that function every named local of the
  image has a home slot (count, columns, last visible, old top, old
  selection at [esp+0x10..0x20]) and registers only cache them and the
  members per block: ecx (selection) and edx (the Dialog base) stay live
  across the jump-table dispatch, `key` takes ebx for the switch, edi
  caches `field_18_` and is incremented in place. The count is eligible
  and simply loses ebx; the missing lever is what gives `key` and the
  cached member the registers, not a property of the count.
- **Inline-local slots look last-freed-first-reused** (hypothesis, one
  observation): order_veh's two has_fac expansions use (offset, mask) =
  (-0x64, -0x50) and (-0x50, -0x64); ours with three locals rotates the
  same way. Not probed.

### 4.1 Stand-in debt (2026-11-19 par-1, `analysis/workarounds.py`)

`uv run python recovery/tools/analysis/workarounds.py --feall F [--table]
[--rules] [--json OUT]` (5 s; F from `feall.py --all --out F`) lists every
stand-in with its file, line, kind, the functions it serves, its evidence
comment and whether those functions are fe-exact now: `WORKAROUND(<k>)` tags
(one item per comment block or trailing tag), budget-probe definitions and
call lines (names with budget/probe/spend, and macros that expand to them),
function-pointer locals (`range_call`, `project_call`, ...), `#pragma
auto_inline/inline_depth/optimize` regions, CANARY-PADS/HEADER-PADS blocks,
`volatile` casts/locals in marked bodies and `__forceinline`. `--rules`
reads each evidence comment for the rule most likely to retire it. A
`wide` item is a header/type stand-in whose identifier more than 25
functions use.

Counts at e62b283824 + this batch (all-exact = every served function
fe-exact; the stand-in may be tuning debt or dead):

| kind | items | all-exact | mixed | none-exact |
|---|---|---|---|---|
| probe-call lines (223 calls, 55 probe names, 62 functions, 27 exact) | 166 | 86 | 0 | 80 |
| probe definitions | 57 | 26 | 12 | 17 |
| function-pointer locals | 17 | 2 | 0 | 15 |
| pragma regions | 23 | 20 | 0 | 1 |
| `__forceinline` | 13 | 10 | 1 | 2 |
| `volatile` (untagged casts/locals) | 31 | 28 | 0 | 3 |
| WORKAROUND(inline) | 169 | 61 | 42 | 57 |
| WORKAROUND(noinline) | 51 | 31 | 1 | 14 |
| WORKAROUND(decl) | 146 | 48 | 38 | 30 |
| WORKAROUND(type) | 28 | 11 | 4 | 13 |
| WORKAROUND(volatile) | 3 | 1 | 1 | 1 |
| pad blocks (known count stand-ins) | 15 | 11 | 1 | 1 |

463 functions are served by a non-pad, non-wide stand-in (318 fe-exact).
By likely retiring rule (`--rules`, keyword reading, a screen): probe /
dead spend 265 items (2.8 owner or cost ladder), teardown depth 87 (2.3 and
2.8; the missing level is often a constructor or an owner), call forcers
92 (2.8 "called early, expanded late": one inline level deeper), registers
and reloads 85 (2.7, Z1-Z6, named values), counter 30 (2.5), naming only
28, narrow types 5; 127 need reading.

Retirement pass (`analysis/retire.py WA.json [--write]`, one unit per lane,
every marker of the unit measured with each mechanical stand-in removed
alone: probe statements per function and per line, function-pointer
locals spelled as the plain call, pragma regions, `__forceinline` ->
`inline`, volatile casts dropped). 2026-11-19 result: 15 `__forceinline`
dead (all in .cpp static helpers; each unit unchanged), 6 volatile casts
dead and 2 better (Console::focus, draw_status's `alt`: 331 -> 319),
top_menu's two probes and 15 slack probe calls elsewhere dead (window_proc
12 -> 9, do_tracking 20 -> 15, order_veh, size_of_planet, base_editor_proj),
the unused top_menu_spend definition removed. No pragma region and no
function-pointer local is dead: every one moves its unit. Two cautions:
- **fe masks relocation operands, so a load-order stand-in can read as
  dead** (base_upkeep 0x004F79C0: the volatile turn read removed is fe-equal
  but `recover measure` says SHAPE_EXACT, the two globals swap). Confirm
  every retirement with `recover measure` of the served functions.
- **A `volatile`-qualified cast type is a front-end symbol**: removing the
  last `static_cast<const volatile MapWin *>` of a unit renumbers its `$SG`
  literals (console.cpp, mapwin.cpp); run `rebind-literals` after.

Shared separator texts (strings.h, 2026-11-19): of the seven functions that
kept the named stand-ins, the literal form now holds in save_game (all 3
sites), PickTech::draw_tech (23 sites, 303 -> 294) and 13 of
Console::upgrade's 17 sites (147 -> 145), spelled `strcat_X(StringTemp)`;
`say_X()` at the same sites moves them (save_game 19 named-callee distance,
draw_tech 458). The difference is the parameter, not the budget: draw_tech's
call multiset is the same under both. Still named: top_menu (its one " "
site; with any helper the two init_opening expansions of the loop's
if/else are cross-jumped into one, 0 -> 57 rows, a layout effect and not
a call decision), Console::editor_edit_veh (all three sites; any one
helper gives 4 -> 7 named-callee distance), StatusWin::draw_active (any
site: 161 -> 395, calls unchanged: allocation), upgrade's four sites of
its first label block, and DesignWin::select_special (par-2).
Update 2026-11-20 (par-4): select_special's row above is stale. Since
f6fcf7075b its 28 separator sites push the shared texts through
text-returning inlines (`strcat(StringTemp, f())`, WORKAROUND(inline)),
and editor_edit_veh, draw_active and upgrade were re-measured with that
form (2.8, "A shared-COMDAT separator that costs what the raw literal
costs").
