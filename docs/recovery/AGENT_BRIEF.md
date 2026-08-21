Working rules and the levers this tree has already measured. Read before starting.

IF YOU ARE IN A GIT WORKTREE (your own copy of the repo, not
/home/fran6co/code/OpenSMACX), run this FIRST, before anything else:

    eval "$(bash /home/fran6co/code/OpenSMACX/tools/agent_setup.sh /home/fran6co/code/OpenSMACX)"
    uv run tools/osmx.py configure          # builds this worktree's own build/

`agent_setup.sh` exists because two agents in a row lost time to what this
paragraph used to merely ASSERT. It fast-forwards your worktree to the root's
HEAD - one worktree was branched 911 commits back and had none of the tools
named below - and it exports `OPENSMACX_IMAGE`, because `.opensmacx/` is
gitignored and the pinned image is therefore NOT in your worktree. `configure`
then writes this worktree's OWN `build/compile_commands.json`, which every
measurement reads; do not point that at the root's copy, because it names the
root's paths and nothing in your worktree will match them.

RUNNING THIS SCRIPT IS EXPLICITLY PERMITTED even if your instructions say git
commands are the coordinator's. It uses git internally, and only ever
`merge --ff-only` - it cannot rewrite history, cannot discard work, and refuses
outright rather than fast-forwarding over anything uncommitted. An agent that
declined to run it lost a whole dispatch to a worktree 937 commits behind
`master`, reporting the blocker instead of clearing it. Clear it.

If `agent_setup.sh` refuses, read what it says: it will not fast-forward over
uncommitted work, and the fix is to hand that diff back first.

Working in a worktree is why you can edit freely: nothing you touch collides
with another agent.

A BODY MAY ALREADY BE WRITTEN, SOMEWHERE THAT IS NOT COMPILED
- `uv run tools/promotable.py` lists byte-exact bodies that exist only in
  `src/recovered/`, `src/recovered/units/` or `src/unrecovered/`. There are 878,
  and 36 are called by code the build compiles. A claim in one of those files
  proves the ARTIFACT, not this tree: nothing links them, and the product source
  often does not declare the function at all.
- Promoting one is not copying it. The artifact reaches its fields through
  `reinterpret_cast<char *>(this) + 0x...`; you have to model the layout,
  declare the member, delete the artifact, and delete any `pending_bodies.cpp`
  forwarder it had. The linker catches the last one for you - two definitions
  of a symbol is LNK2005.

BEFORE YOU GRIND A BODY
- `uv run tools/handwritten_asm.py` lists bodies whose shipped bytes use
  opcodes VC6 never emits - `loop`, bare `lodsb`, `xlatb`. Those were inline
  assembly in the original, byte-exactness is NOT reachable from C++, and the
  ceiling is semantic equivalence. Seven are already marked in the tree.
  DO NOT answer one of these with `__asm`: a semantic C++ body is worth more
  than a byte-exact assembly one.
- `uv run tools/listing_diff.py <addr>` prints EVERY divergence, aligned and
  relocation-masked. `measure` reports only the first, which on `Buffer::hline`
  was a jump whose target had moved because of something 70 instructions later.

PICKING A BATCH (for whoever hands one out)
- `uv run tools/frontier.py --fresh` lists WinMain-reachable bodies with no
  `RULED-OUT:` notes on them. Rows without `--fresh` carry `[N ruled-out]`.
  A batch picked off the raw depth order sent one agent eight bodies that were
  already exhausted, and it correctly reported eight no-changes.
- `uv run tools/call_diff.py --all` ranks bodies whose CALL COUNT disagrees
  with the image. Those are structural defects, not register noise, and they
  are the highest-yield rows on the board: the FEWER direction means this tree
  skips work the image does.

MEASURING
- `uv run tools/osmx.py measure <addr>` scores one body. `--all-flags` scores every
  flag set; the winner is picked on similarity across sets that answer different
  questions, so a real improvement can look like a no-op without it.
- `uv run tools/osmx.py show <addr>` prints the image's bytes.
- `uv run tools/osmx.py calls <addr>` says what the image REALLY calls. "0 call(s)"
  means every callee is written out inline there.
- `uv run tools/store_order.py <addr>` prints the field-write order in the image
  beside this tree's, normalised to each side's own first store.
- `uv run tools/call_diff.py --all` lists bodies whose CALL COUNT disagrees with the
  image and names the helper being over-called. A call the image does not make is a
  SEMANTIC difference, not an encoding one - fix those first.
- `uv run tools/try_spellings.py <file> <addr> <candidates.json>` scores candidate
  bodies and restores the file. If its control does not score 0 agreeing
  instructions, STOP: the span is not being replaced and every result is meaningless.

LEVERS THAT HAVE PAID, most productive first

- A CALLEE BOUND AS A POINTER costs its caller the `E8`. `T *const F = (T *)0x...`
  compiles `call dword ptr [F]` where the image emits `call rel32`. Declare it as
  an ordinary function and add a forwarder in `src/pending_bodies.cpp`; the target
  of an `E8` is a relocation on both sides and is discounted, so any symbol
  matches. Same defect for a CALLBACK bound as `extern const void *const C;`
  defined in a .cpp - it compiles `push dword ptr [C]` against the image's
  `push 0xADDR`, and the fix is to DEFINE it in the header, where a namespace
  scope `const` has internal linkage and folds to the immediate. Worth 202 claims
  in one change on 2026-08-21, plus the whole message-pump family.

- A CONSTRUCTOR CALL IS NOT THE WAY TO REACH A CONSTRUCTOR. When a `??__E`
  initialiser dispatches through a `func_opaque_ctor_*` seam, the fix is a
  `construct` METHOD on the class, not a real constructor: VC6 adds its hidden
  most-derived flag to an explicit `->Class::Class(n)` call, so the site pushes
  the argument TWICE. `Win::construct` is the existing idiom, and its marker
  carries a `symbol` fact naming it. The call target is a relocation on both
  sides, so the method's own name costs nothing. Five initialisers went
  byte-exact this way on 2026-08-21; the constructor spelling made them worse.

- THE IMAGE PEELS ITS LOOPS. `do { B } while (C);` compiles one copy of the body;
  the image runs `B; while (C) { B }` - same program, and the difference was four
  instructions on each of three message pumps. If the image's loop has TWO calls
  to the condition where you have one, this is why.

- BRANCH POLARITY IS SOURCE-SHAPED. `if (x) { work; return true; } return false;`
  and `if (!x) { return false; } work; return true;` are not the same codegen.
  The image often jumps TO the work and falls through to the early return, which
  means it never emits the `xor eax, eax` the first form needs. If your listing
  has an extra `xor eax, eax` and an inverted jump, rewrite it as a guard clause.

1. A helper the image INLINES that this tree defines in a .cpp. `osmx calls` shows no
   call to it. Move the body to the header - `inline` if it has no ORIGINAL marker,
   `MEASURED inline` if it does (that keeps its own claim measurable) - and LEAVE THE
   MARKER IN THE .cpp, because decomp's reader globs only *.cpp and *.c. Put the
   definition at the END of the header so it can see what it uses.
2. `memcpy`-based accessors and other helpers VC6 refuses to inline: `__forceinline`.
3. Assignments in declaration order where the image uses another. `store_order` shows
   the permutation directly.
4. `uint32_t` where the image's branch is signed: `jl` not `jb`, `sar` not `shr`.
   `uv run tools/signedness.py` finds these.
5. A value read AFTER a store where the image reads it before - hoist it into a local.
6. `&record->field` taken once, rather than the field read and written twice: the
   image folds the member offset into the `lea`.
7. `T *p = &Array[index];` then `p->a`, `p->b` rather than `Array[index].a`,
   `Array[index].b`. This one is PER FUNCTION - it has also made bodies worse - so
   measure it, do not sweep it.
8. `volatile` and hand-rolled bit-casts written to pin store order or wrapping: both
   defeat VC6 and are usually unnecessary. Try the plain spelling.
9. A vtable dispatch spelled `(ORIGINAL(x)->*original_slot<T>(vtable + N))()` emits
   `mov reg,[vtable+N]; call reg` where the image has one `call dword ptr [vtable+N]`.
   `vtable_slot<Fn>(x, N)(x)` from original_seam.h emits the single call; declare `Fn`
   as `__fastcall` with the RECEIVER AS ITS ONLY PARAMETER, or a second one costs a
   `xor edx, edx`.

RULES
- After ANY source change `cmake --build build` must exit 0.
- Bank a body with `uv run tools/osmx.py record <addr>`.
- The gate is `uv run tools/osmx.py check`. NEVER pipe it to `tail` - the exit code is
  the verdict. It must stay at 0 REGRESSED, and you must run it before finishing.
- Record what you learn under each marker: a `// LEVER:` line for what worked, a
  `// RULED-OUT:` line for a spelling you measured and rejected. Never touch the
  `// ORIGINAL:` marker line itself.
- Never write a comment asserting something the measurement does not show. If a body
  plateaus after a handful of measured attempts, leave it and say so.
- Do not commit. Do not change any file outside src/.
