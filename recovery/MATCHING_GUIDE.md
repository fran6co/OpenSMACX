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
  was a header inline (get_title/get_name as faction.h inlines were rejected).
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
- **Unresolved blockers** (not proof that no source form exists):
  - **`??_H` inside a struct:** VC6 expands the vector constructor iterator
    inline even under `auto_inline(off)`.
  - **Dead-store counters:** a counter VC6 deletes but the image keeps
    (base_growth).
  - **Reversed spill reloads:** spill reloads in exactly reversed order at loop
    boundaries.
