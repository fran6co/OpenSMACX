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
