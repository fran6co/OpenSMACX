# Compiler evidence, not workflow policy

Relevant observations extracted from the retired agent brief. They are historical
starting points, not current measurements or universal rules. The original brief
and its complete experiments remain available with
`git show b7f45b19:docs/recovery/AGENT_BRIEF.md`. Current ownership and verification
policy is in `WORKFLOW.md`; no spelling trick overrides that policy.

## Structural questions first

- Virtual-base flags are ABI inputs, not necessarily source parameters. Earlier
  catalogue names and the brief contradicted themselves about constructors and
  destructors. Determine explicit arguments from callers, base construction,
  vbtable accesses and emitted symbols. Do not globally translate constructor
  labels into `construct(int)` methods or infer virtual inheritance from a name.
- The old RadioButton experiments found compiler-emitted vtordisp thunks after
  genuine virtual bases and overrides were declared. Hand-written argument
  forwarding produced a call/frame where the image had a tail jump. Inspect
  generated COMDATs and map the emitted code; do not reproduce a compiler thunk
  with arithmetic over `this`.
- Adding virtual declarations affects callers and table slots. Qualified calls
  can intentionally suppress virtual dispatch, but only the original call and
  target evidence establish whether that is appropriate. Reconstructed Q/U
  catalogue decorations are hints, not a complete proof of virtualness.
- Placement construction of already-declared members duplicates lifecycle work.
  The SocialWin and MultiDebug experiments improved after real member
  construction replaced it. When initializers regress, investigate actual global
  ownership and callers together; do not restore raw storage automatically.
- An exception-frame mismatch can reflect missing bases, embedded objects,
  cleanup funclets, or the wrong selected symbol. The old suggestion to replace
  constructors with methods merely to remove SEH is not a recovery rule.
- Empty inline constructors hide initialization defects; making them real can
  expose additional failures. Track the dependent construction graph rather than
  treating the newly visible fault as evidence against the constructor.
- Old Palette recovery demonstrated that byte-exact bodies can retain wrong
  names, types, raw offsets and incorrect ownership. Names come from behavior and
  callers; a mangled rename must be reconciled with the actual emitted symbol.

## Source-form experiments after structure

OpenSMACX currently selects two release comparison flag sets in its local
compiler configuration: `/c /O2 /Gy /GR- /GX` and the same with `/Oy-`.
There is no `--all-flags` option. Flag-policy changes are coordinator decisions,
not a per-function search for a permissive checker.

- VC6 is pre-modern C++. Use compiler-supported syntax and evidence-backed
  widths/signedness; `int` and unsigned integer parameters mangle differently.
  Do not change a signed interface into an unsigned one merely for instruction
  selection, or apply an all-parameters-must-be-int rule to real pointer types.
- Separate guards versus merged conditions, guard order, branch polarity and
  peeled loops can change epilogues and register allocation. The old
  `prototype_factor`/Buffer measurements differed in direction: measure locally.
- The receiver-spill pattern observed in Palette survived several guard/local
  experiments. Treat previous failed attempts as a reason to seek new context,
  not to repeat the same spellings or declare the function impossible.
- Match a local's width to its underlying field and observe when the image
  widens it. Signed division rounds toward zero; a right shift is not equivalent
  on all negative inputs. Preserve semantics before considering encoding.
- `volatile` aliases can prevent memory-operand folding; genuine volatile stores
  can preserve writes the image performs. Neither adding nor removing volatile
  is a safe tree-wide transformation.
- Observe repeated calls and reads. Hoisting an expression can remove calls
  from a macro expansion or change aliasing behavior. A call-count difference is
  a semantic question before it is a similarity question.
- A function bound through a pointer can change a direct call into an indirect
  one. Recover a real callee/global binding, not an original-address forwarder.
  The old relocation-masked score could not prove that the referenced target was
  correct; strict reference checks are required.
- Never trust a layout/sizeof observation from a failed compile. Likewise,
  a source-integrity match, a byte match and a successful link prove different
  properties. None independently proves a functioning reconstructed application.

## Operational lessons retained

Global initialization/atexit dependencies matter alongside entry-point reachability.
Recovered seeds must be reconciled with product definitions and their callers;
removing a duplicate body does not ensure callers no longer use a redirect.

Git stash is shared across worktrees. Piped patch application can die before
writing, and `git apply --check --3way` is not proof of a conflict-free applied
index. Inspect actual side effects. Isolate compiler state; do not reuse a build
database naming another worktree or kill a shared Wine server.

For runtime investigations, use the same working directory, game assets and
display/Wine conditions for original and candidate controls. A crash caused by
launch conditions is not evidence of a decompilation defect. Runtime acceptance
is not yet wired into the supplied recovery driver.
