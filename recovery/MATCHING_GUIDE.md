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
  - *Soft form, `analysis/datarun.py`:* ranked by the link position of the
    file that defines them, the 1,291 bound `.bss` objects are in order for
    1,111 (`.data`: 371 of 475). The 293 outside the longest ordered run are
    definitions that sit in another unit's contribution: the popup and
    interlude font pointers 0x0094575C-0x00945810, `FastUnitAnim` and
    `TutWinClimateGate945830` lie among that same alpha/newgame unit's
    objects (0x0093FC58-0x00945834), not at popup.cpp's, veh.cpp's or
    tutwin.cpp's position; `MapEditorPaintId` and `MapEditorBrushId`
    between map_grid's objects and `StoredScaleAdjustment` (not mapwin.cpp);
    `Compass` between vehicle_cost's and game's. The game state proper
    (0x00945F48-0x009B22EC: the rules tables, MapStorage, Players, Bases,
    Vehs, world's counters) lies after technology's thunk object 0x00945ED0
    and before multidebug's 0x009B22F0, and xpops' two thunk objects
    (`g_PALETTE1` 0x0094C590, `g_PALETTE2` 0x009523A0) lie inside it: at
    least 0x0094C590-0x009523A0 (FactionMonuments, TimeControl, Armor,
    Technology, WorldBuilder, ReplayEvents, Morale) is defined by the unit
    we call xpops.cpp, and the rest by it or the units up to gamma, not by
    alpha.cpp, game.cpp and veh.cpp where the tree defines them.
  Where an object is defined does not change code (every reference is
  absolute), so the soft form is a structural record, not a matching lever;
  moving a definition can still shift a unit's symbol counter.
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
    suffix shortfall above stays open.
  - A run can be a helper at one site and not another: the label/item
    helper breaks DiploWin::on_button_clicked 0x004415C0 (refuses its
    ~RadioButton), so keep such helpers file-local to the sites that gain.
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
    late is an owner (2.8 "Missing owner"), not a class. The function-pointer
    stand-ins in base_psych, staple_the_bastards, end_of_game and
    base_editor_fac stay until the owner is found; dropping the `queued`
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

- **Frame offsets follow the packed slot's total, and spilled induction
  temporaries join the first free slot** (par-4 2026-11-19, enemy_move
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
  (best score / scan index / best quality at [ebp-0x1C], 150 -> 142).
  order_veh, consider_designs, setup_text, social_ai, communicate,
  can_terraform, battle_fight, base_build: nothing to merge, their shared
  slots already sit at the image's rank.
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
- **A single-byte treaty mask tested as a dword is the helper.**
  `test dword ptr [PlayersData + ..], 0x200` (spot_all, Path::find) against
  our narrowed `test byte ptr [.. + 1], 2`: VC6 narrows `field & K` written
  in place; battle_fight's `has_treaty(a, b, DTREATY_UNK_800)` gives the
  dword `test .., 0x800` in ours and in the image, so the mask arriving as
  an inline's parameter is the likely spelling (not yet applied at those two
  sites; the named-value sweep's family T covers it).
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
re-aligns their pad blocks (2.5). base_energy 0x004EB560 and give_a_base
0x0054CF40 flip from header content, not from the counter.
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
