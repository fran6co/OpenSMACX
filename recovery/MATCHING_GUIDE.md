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
  - A stand-in is kept only if it makes a function exact, or clearly moves it
    with image evidence. Tag it `// WORKAROUND(<inline|noinline|type|decl|volatile>): <evidence>`.
  - Never edit `ORIGINAL:` lines or `BYTE_EXACT` tokens or compiler flags. Add no
    tests or JSON.
  - Time-box each hypothesis. Check `git log -p -S'<name>'` for earlier attempts
    and exact seeds first.
- **No-loss check (required).** At the end, measure every `BYTE_EXACT` address in
  every file you edited, plus your newly exact addresses, and fix or revert any
  regression. `cmake --build build` must link. Each TU has canaries that flip
  first (section 3.6).
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
  refused site is skipped while cheaper later sites can still inline. Every
  expansion competes, including code after the return sites, and there is a
  per-callee size cap as well.
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

### 2.5 Unit coupling

- **Edits ripple through the unit.** Any edit can flip register or
  operand-order ties in a *later* function of the same TU, because VC6 shifts
  its `$SG`/`$L` counters. Edits that do this include:
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
- **Extern-name count.** Codegen in a TU depends on how many *distinct* extern
  variable names it declares; the names and their lengths don't matter, and
  repeated declarations and enumerators don't count (action_give stays exact
  with 6–7 PopupTheme* names, flips with 1–5; one extra name flips
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
- **Member count counts declarations.** Turning N data members into one
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
- **The extern-name count is a window, not a threshold,** and it moves with
  the other function bodies (action_give needed >=4, then >=6, then 14-16
  extra names as action_terraform changed). After rewriting a function in a
  canary unit, sweep 0-24 extra names and keep the middle of the window.
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

## 3. Symptom → first thing to check

### 3.1 Calls and inlining
- **The image calls something we inline (or the reverse).** Check the budget
  rule (2.1), definition order, and a hand-copied callee body. The most common
  regression cause is a hand-copied first level of a recursive helper such as
  `is_visible` or `screen_to_client`.
- **A hand-written block equals a same-TU function body.** Call the function;
  write it inline before its callers if the image expands it there.
- **An inlined accessor skews register choice through the whole function.**
  Read or write the member directly (the netwin fix covered 10 functions).
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
- **The image calls `memcpy`/`memset` out of line.** Wrap the function in
  `#pragma function(...)` … `#pragma intrinsic(...)`.
- **A `vector_dist`-style computation is inlined.** Write a two-level static
  inline helper modelled on the existing ones.

### 3.2 Registers and order
- **Operand order of commutatives.** For `a + b` of globals, VC6 loads `b` first.
  For `f(a) OP f(b)` it calls the right side first. It canonicalises
  `(a + b) / 2`.
- **expand_rect's constants.** For `expand_rect(&r, -N, -N)` the image's
  "+N immediate on left/top, register holding -N added to right/bottom" comes
  from the helper body `left -= h; right += h`; hand-written `+=`/`-=` pairs
  or an inset helper differ.
- **Null-check forms.** `static_cast<Base*>(this)->f()`, a `Base *p = this`
  local and `static_cast<Base&>(obj).f()` all emit a null check; call
  `Base::f()` / `obj.Base::f()`. Passing `this` as a `Base*` argument
  legitimately emits neg/sbb/and. Member data through a virtual base needs no
  check; a reference conversion does.
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
- **Fix before reverting.** When a faithful change breaks a claim, diagnose
  and repair the break (widening scope is fine) before considering a
  revert; revert only if no repair exists and the change is not faithful.
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
- **N adjacent globals cleared with `xor eax,eax` + N stores and no zero
  register** were one block clear (intrinsic memset) in the original;
  separate `= 0` stores count toward VC6's zero-register choice
  (battle_compute/battle_init).
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
- **Flat-first ternary wrap inside a static helper.** If the image reloads the
  bound in each arm of an x-wrap, write the helper as the ternary
  `(MapIsFlat & 1) ? x : x < 0 ? x + W : x >= W ? x - W : x` and pass the sum
  directly, not through a stored intermediate (vehdraw_scoot_start).
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
  between. Memory-homed locals are never cached in callee-saved registers
  across calls, so if ours keeps V memory-only, another local holds that
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

### 3.4 Arithmetic idioms
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
- **reportif.cpp:** `ReportIf::on_iface_button_clicked` 0x004A6330.
- **turn.cpp:** `end_of_game` 0x00524870.
- **console_commands.cpp:** `corner_the_market` 0x004E2000.
- **Header vetting:** `action_give`, `capture_base`, `setup_player`,
  `base_production`, `battle_fight`, `enemy_move`, `game_data`,
  `load_interface`, `SocialWin::init`, `tech_achieved`.

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
  follow source order; image blocks placed after the epilogue were not
  reproduced by any case or default order (ProdPicker::on_button_clicked).
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
