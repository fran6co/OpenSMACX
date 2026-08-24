---
description: Recover one class family - inheritance, vtable slots, ctors/dtors - instead of one function at a time
---

Run one CLASS recovery batch. `$ARGUMENTS` names the family (a class name,
or a file stem owning one) plus optional explicit addresses.

You are the **coordinator**. You alone touch the map and run `git`; subagents
only edit bodies in their own worktrees. The mechanics - worktrees, collect,
re-measure, gate, reap - are exactly `/recover-batch` steps 3-7. This file
covers only what is DIFFERENT about working a class instead of an address.

## Why a class batch exists

Function batches cannot express the remaining debt. Of 655 reachable
not-matching bodies, 107 are blocked by a shape whose remedy is a DECLARATION
elsewhere in the class: 46 vtable-slot dispatches cured by declaring the
method virtual, 21 VCall shims likewise, 11 `construct()` stand-ins cured by
writing the real constructor. Measured 2026-08-23, 32 of the 46 dispatches
belong to the Midi hierarchy alone. One function per agent hands those to four
agents who all trip over the same missing declaration.

## 1. Assemble the family brief

No tool owns this; join existing outputs:

```sh
uv run tools/osmx.py status --json        # unclaimed members by address/file
uv run tools/compiler_work.py             # which shapes the family carries
uv run tools/osmx.py show <ctor-addr>     # the image's construction order
uv run tools/most_derived_flag.py         # which ctors take the hidden flag
```

The brief MUST state, with evidence:
- **inheritance edges** - base classes and `virtual` or not, read off vbtable
  accesses (`mov eax,[ecx]; mov ecx,[eax+4]`) and vtordisp arithmetic in the
  image, not off any header comment;
- **vtable slot order** - the image's own dispatches spell it out: every
  `call dword ptr [eax + N]` against the family is a slot position. A method
  declared virtual at the wrong position changes the vtable and REGRESSES
  claimed bodies;
- **construction order** - what the image writes before/after each vfptr
  store (see AGENT_BRIEF, VTABLE STORES GO FIRST).

## 2. Fan out ONE agent per family

Worktree isolation as always. The agent works top-down:

1. Fix declarations first: inheritance, then virtuals at their image slots.
   AGENT_BRIEF is emphatic and measured: fixing the declaration often IS the
   recovery - ListBox and CheckBox bodies were short once their bases were
   real.
2. Replace lifecycle stand-ins: a real `X::X()` for `construct()`, real
   `~X()` for free-function destructors. Keep `construct(int)` ONLY where
   most_derived_flag.py proved the class takes a real int.
3. Promote artifact bodies: byte-exact copies living in `src/unrecovered/`
   or address-named files move into the family's translation unit
   (`promotable.py` lists candidates; `orphan_artifacts.py` must stay clean
   afterwards).
4. Delete what the compiler now emits - hand-installed vtables, placement-new
   subobjects, hand-walked vbtable reads. Every deletion was worth claims in
   this project's history; none of it is style.

Two families sharing a header go to the SAME batch or are sequenced; parallel
agents editing one header is how declarations silently vanish.

## 2b. The semantic-fidelity pass: drive the class's `debt` to 0

`uv run tools/class_debt.py` counts what a body can carry while measuring
BYTE_EXACT - the bytes cannot see any of it, which is why palette.cpp
measured clean while carrying all of it (2026-08-24). `--by-class` shows the
per-class total in the `debt` column, and **a class is not done above 0**.
In the same worktree, same agent, after the declarations land:

1. **Name every `UNKn`** from evidence: callers, the image's data,
   `docs/recovery/behaviour-member-names.csv`. A rename changes the mangled
   symbol - update the `// symbol` fact (the gated `marker_symbols.py` holds
   it to what the build emits) and re-measure the body. No `UNKn` survives
   the class's pass.
2. **Fix the types the bytes cannot see.** `int` returning
   `reinterpret_cast<int>(this)` is a pointer. Do not trust an invented
   operator name either way: Palette's `__as` looked like `operator=` and
   the CALLERS refuted it - both run the destructor immediately before it,
   so it is `copy_from`, a reset method. Name from call patterns and
   behaviour; ask `marker_symbols.py` what the build emits. Re-measure every
   touched body: a type change that moves bytes is a finding, not a
   formality.
3. **Delete the class's orphan redirects** - `*_redirect` functions whose
   only references are their own declaration and definition. The gate's
   link step referees; if the link breaks, something non-textual needed it,
   which is a finding to record, not a reason to keep dead code quietly.
4. **Purge `reinterpret_cast<...>(this)` - it is banned.** Palette's
   copy_from proved the member form byte-identical (19/19); a cast to
   another class is an inheritance edge to declare instead. A body that
   cannot lose the cast is a layout finding for the family brief. And do
   not trade a this-pun for a member-pun: a dword pun of a struct feeding
   mask/shift extraction is that struct's fields read directly, an int copy
   of a struct is a struct assignment - Palette's honest forms measured
   CLOSER to the image, not just cleaner.
5. **Convert function-address bindings**: a cast like
   `(void *)0x005FEAD0` whose address lands inside a catalogued function is
   a function reference in data clothes. Declare the callee (forward in
   `pending_bodies.cpp` if unpromoted) and delete the global.
6. **Claim every lowered ceiling** in `class_debt.py` in the same commit -
   the gate fails on slack.

The claim floor must not move through any of this: renames and type fixes
re-measure to the same claims under corrected names, never to fewer.

## 3. Re-measure WIDE

After landing, the coordinator measures beyond the family itself:

```sh
uv run tools/osmx.py record <every family member>
uv run tools/frontier.py --roots crtinit --fresh    # pre-WinMain ctors surface here
```

Then the callers the declarations just unlocked: the seam-blocked bodies
(`compiler_work.py` census before/after) frequently flip to BYTE_EXACT with no
edit of their own. Record those too - they are the yield that pays for the
batch.

## 4. Gate, ceilings, report

Bare `osmx check`, never piped, then `--json` harvest as in `/recover-batch`.
A `compiler_work` shape that DROPPED must have its ceiling lowered in this
same commit. Report: family, declarations changed, shapes removed, claims
banked by your measurement, and anything that regressed with why.
