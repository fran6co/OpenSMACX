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

If `agent_setup.sh` refuses, read what it says: it will not fast-forward over
uncommitted work, and the fix is to hand that diff back first.

Working in a worktree is why you can edit freely: nothing you touch collides
with another agent.

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
