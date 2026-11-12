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
- `/tmp` is a small tmpfs; keep large output under the worktree's `build/`.
- Editing the permuted file mid-run stops `permute` (`inputs_changed`).
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
- **Definition order and keywords.** VC6 does not auto-inline a plain function,
  or an explicit free `inline`, defined *after* its caller (check C4711, because
  there are exceptions). An explicit `inline` *member* defined later still
  inlines. `__forceinline` and `inline`
  compile identically for `label_get` across all 48 using files.
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

  Keep a dead probe (a static helper of chained `if (x == k) x = c;` called with
  a constant, which folds to zero code) only as `WORKAROUND(inline)` when no
  natural spelling is found.
- **Hand-copied helper bodies.** Raw `has_treaty` tests or hand-inlined writers
  inflate each caller's own size, so every teardown expands one level too deep
  (call_off_vendetta had 48 states against 22). Restore the helper calls
  file-wide first.

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
- **Header vetting depends on counts.** The number of member and declaration
  entries matters, and so does visibility. An unused inline body, or a newly
  visible plain struct, in a class with 30–56 includers flips the same ~12 large
  functions (3.6). Removing an unused body perturbs as much as adding one, and
  removing two unused declarations together can flip a canary that neither does
  alone. Comments, blank lines and same-length renames are neutral.
  Gate new bodies per TU (`#ifdef`, tagged `WORKAROUND(decl)`), and keep a
  header change only when the scoped `check-matches --since` shows gains and no
  losses.
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
- **Nested call results.** A nested cdecl call made before the outer call's
  constant pushes was stored to a local first.
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
  - `default:` is placed last.
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
- **The image repeats a compare or reload VC6 would fold.** That is an inline
  helper boundary: wrap the subexpression in a small static inline.
- **The image calls `memcpy`/`memset` out of line.** Wrap the function in
  `#pragma function(...)` … `#pragma intrinsic(...)`.
- **A `vector_dist`-style computation is inlined.** Write a two-level static
  inline helper modelled on the existing ones.

### 3.2 Registers and order
- **Operand order of commutatives.** For `a + b` of globals, VC6 loads `b` first.
  For `f(a) OP f(b)` it calls the right side first. It canonicalises
  `(a + b) / 2`.
- **Locals and declaration order.** Declaration order of locals loaded from one
  struct sets load and slot order. A named local takes a callee-saved register,
  while an unnamed temporary spills.
- **Constants.** `x * 2` gives `add`, `x << 1` gives `shl`. A const local bound
  gives `lea`; writing it at each use gives `inc`.
- **Register-held zero.** VC6 knows `eax == 0` only on the fall-through of `jne`
  after `test eax,eax`.
- **`cmp eax,<zero register>` where we emit `test eax,eax`.** Store the call
  result in a named local.
- **Spill reloads in exactly reversed order** at loop boundaries: a known
  blocker, so skip.

### 3.3 Frame and locals
- **Locals at `[ebp+8]`/`[ebp+0xc]`** sit in dead parameter slots: write ordinary
  locals and pass the parameters directly.
- **A frame larger than the locals.** Either a block-scoped buffer overlapped a
  later inlined local, or the original read fields directly instead of caching
  them.
- **Buffer sizes.** Read them from the frame gap to the next local.
- **A local the image never stores** was uninitialized in the original.

### 3.4 Arithmetic idioms
- **`rand() & 0x80000001; jns; dec; or -2; inc`** is `rand() % 2`.
- **A magic divide then multiply-subtract** is `x - x/C*C`, not `%`.
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
