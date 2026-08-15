#!/usr/bin/env python3
"""Everything an agent needs for one function, in one message.

Measured on the first coordinated run: a sonnet agent spent 79,111 tokens and
20 tool calls to fix a SIXTEEN BYTE function, and almost all of it went on
finding things - opening the findings file, the prompt, the current body,
hunting for the verify command. The reasoning was a handful of lines at the
end.

So the brief is assembled here instead. The agent gets the disassembly, the
body as it stands, the verdict it has to beat, the rules that apply to THIS
divergence rather than all of them, and the one command that scores it. Nothing
to look up, nothing to guess at, and no repository to wander around in.

    tools/agent_brief.py 0x005E3630

The output is the prompt. Paste it as the whole task.
"""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import emit_translation_unit as emit  # noqa: E402
import verify_recovered_function as verifier  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent

# Only the rules that bear on the divergence actually seen. The full findings
# file is a page and a half; a body whose branch polarity is wrong does not
# need the paragraph about strength reduction, and everything an agent reads
# and does not use is budget it cannot spend on the answer.
TARGETED = (
    (("jl", "jge", "jg", "jle", "je", "jne", "jz", "jnz"),
     "BRANCH POLARITY. `if (c) { A } return B;` jumps over `A`, so the jump\n"
     "carries the NEGATED condition - source `<` gives `jge`. Flipping the\n"
     "condition and swapping the arms flips the emitted jump. A ternary is a\n"
     "third shape again. Try all three; they are not interchangeable.\n"
     "Measured: on 0x005E3650 the ternary did NOT help and flipping the `if`\n"
     "did."),
    (("jbe", "jae", "jb", "ja"),
     "SIGNEDNESS. `jbe`/`jae`/`jb`/`ja` are unsigned, `jle`/`jge`/`jl`/`jg`\n"
     "are signed. A diff between the two families is a type error: the\n"
     "recovery used `uint32_t` where the original used `int`. AGENTS.md:87\n"
     "says parameters are `int`."),
    (("sar", "shl", "shr", "cdq", "idiv", "imul"),
     "STRENGTH REDUCTION. `sar`/`cdq`/`shl` in the original against\n"
     "`cmp`/`and`/`div` in the rebuild means the original divided or\n"
     "multiplied by a power of two on a SIGNED type. `cdq` before `idiv` is\n"
     "signed division; an unsigned recovery emits neither."),
    (("push", "mov"),
     "If this is `push` vs `mov` at index 0-2 it is REGISTER ALLOCATION - the\n"
     "original dedicates a callee-saved register the rebuild does not.\n"
     "docs/BYTE_MATCH_ROUTE.md:305-330 measured this as the hard class with\n"
     "two levers already refuted. Timebox it and report what you tried.\n"
     "Also measured: VC6 does not reclaim a register freed earlier in the same\n"
     "body - whichever value is computed FIRST lands in EAX and the second in\n"
     "EDX, whatever order the source puts them in. Six functions have stalled\n"
     "there. If that is your only divergence, stop.\n"
     "WHICH WAY ROUND YOUR DIVERGENCE IS decides the fix, and the two fixes\n"
     "are opposites - see the DIRECTION paragraph above, which reads the side\n"
     "off your own note. This entry is only what holds either way."),
    # WALLS. Ruled out exhaustively; each is keyed like every other entry so an
    # agent meets the one that applies rather than a catalogue of all of them.
    (("stosd", "rep"),
     "REP STOSD SETUP ORDER. The original emits `xor eax,eax` before\n"
     "`mov ecx,N`; every source form emits the count first. memset, ascending\n"
     "and descending hand loops, scoped loop variables and a hoisted const\n"
     "zero were all measured. Stop."),
    (("fnclex", "fninit", "fldcw", "fnstcw"),
     "A BARE x87 OPCODE with no preceding `call` was inlined in the original's\n"
     "source. `_clear87()` and `_fpreset()` both emit a real `call`, and\n"
     "`__asm` is barred. Unreachable - stop."),
    (("jmp",),
     "If the original TAIL-JUMPS an indirect thiscall dispatch where you emit\n"
     "`call; ret N`, stop: VC6 does not emit the sibling jump even when the\n"
     "argument slot is untouched and the cleanup matches exactly.\n"
     "If instead the original's span carries no `ret` AT ALL, check whether the\n"
     "branch target is the ENTRY of another catalogued symbol - that is a `/Gy`\n"
     "fold onto a different function, which no per-function compile can\n"
     "reproduce, and an MSVC vtordisp adjustor thunk looks the same way."),
    # FROM mizuchi.yaml, which carried its own hand-transcribed subset for as
    # long as both existed. These five were in that copy and NOT in this one -
    # the two tables had diverged in both directions, so an agent got whichever
    # third its route happened to carry.
    # Keyed on `loop` alone. `lea`/`inc`/`dec` were tried and reverted: they
    # appear all over ordinary arithmetic, so the loop advice fired on an
    # `add` vs `lea` divergence that has nothing to do with a loop. A rule that
    # matches too widely is worse than one that matches nothing, because the
    # agent cannot tell it was mis-selected.
    (("loop",),
     "LOOP FORM IS A REAL LEVER AND NOT A GENERAL ONE - three refusals in one\n"
     "batch. `sub_5ad450` went exact as `do { ...; p += 3; } while (--count)`,\n"
     "and its IMMEDIATE NEIGHBOUR `sub_5ad4c0`, same table and stride, got\n"
     "WORSE from the same edit (divergence moved #4 -> #2). Also measured: VC6\n"
     "never unrolls a constant fill - `AutoSound::init` is 132 B of straight-\n"
     "line stores in the original and every loop spelling collapses to a 31-byte\n"
     "loop, so the stores must be written out; and an indexed loop needs one\n"
     "more live value, spills a callee-saved register, and THAT spill is the\n"
     "`push` you see at index 0."),
    (("adc", "sbb"),
     "CARRY-FLAG ACCUMULATION IS UNREACHABLE FROM C++ WITH THIS COMPILER, and\n"
     "this is the one measured exhaustively: 13 spellings of a bit-count loop -\n"
     "do-while, while, for, shift-in-condition, extract-to-temp, `%2`,\n"
     "`if (v&1) c++` and variants - crossed with 6 optimisation levels, 4\n"
     "processor targets, /Oi on and off and both frame-pointer settings, about\n"
     "340 compilations. Every one emits and/add/shr or test/je/inc. NONE emits\n"
     "`adc` or `sbb`. The original was almost certainly hand-written assembly.\n"
     "Do NOT hunt a spelling. Get the rest of the body exact in ordinary C++;\n"
     "this is the one case where a small `__asm` for the carry sequence is the\n"
     "right answer rather than a shortcut, and say which instructions you wrote."),
    (("C4234",),
     "VC6 REJECTS `__thiscall` ON A FREE FUNCTION POINTER (C4234). For an\n"
     "indirect virtual call use the generated VCall shim in the context file;\n"
     "for a thiscall function POINTER, take a member-function pointer of a\n"
     "dummy class instead of spelling the convention."),
    (("BOOL", "LPSTR", "C2146", "C2061"),
     "A WINDOWS TYPEDEF IN THE SIGNATURE IS A NO_COMPILE, NOT A MISMATCH. The\n"
     "scaffolding forward-declares only what is reachable from the decoded\n"
     "signature, so `BOOL` and `LPSTR` arrive undeclared. That is a fact about\n"
     "the unit, not about the body - do not rewrite the body to chase it."),
    (("C2065", "C2039"),
     "AN UNDECLARED IDENTIFIER IS USUALLY THE SCAFFOLDING, NOT YOUR BODY.\n"
     "Measured over every NO_COMPILE note in the map: of 2,865 C2065\n"
     "occurrences, 2,455 are game constants and globals - GameAtexit (388),\n"
     "ScrollOperatorDelete (56), RadiusOffsetX/Y, TRIAD_AIR, MaxPlayerNum,\n"
     "BIT_FUNGUS, DTREATY_PACT, even NULL - and only 79 are `field_*` members.\n"
     "The standalone unit declares what the decoded signature reaches and no\n"
     "more, so a body written against the real headers loses everything\n"
     "project-wide. Reach the value the way the emitter can supply it rather\n"
     "than assuming a class layout is missing."),
    (("<end>",),
     "A LENGTH MISMATCH where the ORIGINAL ends first is usually a span that\n"
     "excludes its own `ret` because the linker folded the tail onto another\n"
     "function. Check whether the branch target is the ENTRY of a different\n"
     "catalogued symbol; if it is, this is unreachable."),
    # ------------------------------------------------------------------ the
    # levers batches 3-6 paid for. They lived in the COORDINATOR'S PROMPT for
    # four batches, retyped eight times each, which is the hand-maintained-list
    # defect this repository names as its highest-yield shape - and it defeats
    # the whole design of this table, which is that a rule reaches the agent
    # whose divergence SELECTS it. Keyed here so they are selected, counted by
    # `--audit`, and survive the coordinator.
    (("C2733",),
     "A CALLEE REDECLARATION COLLIDES WITH THE SCAFFOLD'S NULLARY ONE. The\n"
     "emitter declares each callee with the arity it could decode, often zero,\n"
     "and redeclaring it with the real signature is C2733. Declare a\n"
     "DIFFERENTLY NAMED extern with the true signature and call through that -\n"
     "the comparison is over code, and the symbol you invent is never linked.\n"
     "Paid on _itoa, _strcat, fseek, strcmpi, GetOpenFileNameA and more."),
    (("C4234",),
     "`__thiscall` IS A RESERVED WORD VC6 WILL NOT ACCEPT on a free function\n"
     "or a function-pointer typedef. Spell an indirect thiscall as a MEMBER\n"
     "FUNCTION POINTER plus a union to convert the address, or as a method on\n"
     "a shim class. Paid three times in one batch."),
    (("setne", "sete", "setge", "setg", "setl", "setle"),
     "A `set<cc>` AGAINST YOUR BRANCH is the comparison's FORM, not its\n"
     "meaning. Measured: `(x >= 0) - 1` emits `test; setge` where the\n"
     "algebraically identical `(-1 < x) - 1` emits `cmp; setg`. Write the\n"
     "comparison whose operand ORDER matches the original's."),
    (("movsd", "stosd", "scasb", "movsb", "movsw"),
     "A `rep` STRING OPERATION IS AN INTRINSIC, and it cuts both ways.\n"
     "`memcpy`/`memset` REPRODUCE `rep movsd`/`rep stosd` where a hand-rolled\n"
     "loop does not - so if the original has one, call the intrinsic. If the\n"
     "original does NOT and yours does, VC6 inlined a CRT call: stop it with\n"
     "`#pragma function(memcpy)` (also `abs`, `strcpy`), or by calling through\n"
     "a differently-named extern. Both directions measured."),
    (("pop", "ret"),
     "A DIVERGENCE IN THE TAIL IS THE EXIT SHAPE. Three measured forms:\n"
     "(1) read whether the original sets `eax` before `ret` at all - if it\n"
     "never does the function is `void`, and a `return 0` adds a spurious\n"
     "`xor eax,eax`; if it ENDS in `xor eax,eax` the explicit `return 0` is\n"
     "what reproduces it. Read the tail, do not assume either way.\n"
     "(2) fall-through-then-`return 0` AT THE END, with the guard's false path\n"
     "falling into the tail, where an early return does not match.\n"
     "(3) one shared epilogue via `goto fail;` where separate early returns\n"
     "duplicate it."),
    (("push", "mov", "sub", "test", "cmp", "xor", "lea", "and", "call"),
     "AT INDEX 0-6, READ THE FOUR-FLAG-SET TABLE BEFORE CHANGING A LINE. The\n"
     "verdict is chosen by mnemonic SIMILARITY before first divergence\n"
     "(`byte_match._better`), so the `#N` above may come from the frameless\n"
     "build while the FRAMED build of the same source is already several\n"
     "instructions deeper. Ten bodies swept on 2026-08-14 all reported `#0`;\n"
     "on FIVE the framed build was already at #2, #3, #4 or #7 and the frame\n"
     "was never the defect. `verify_recovered_function.py --dir` prints all\n"
     "four - work the row whose prologue class matches the original (framed if\n"
     "it opens `push ebp; mov ebp, esp`, frameless otherwise).\n"
     "Then, IN THIS ORDER, from ten controlled sweeps:\n"
     "(1) If the body calls `strcat`/`strcpy`/`strlen`/`memset`/`memcpy`, put\n"
     "`#pragma function(<name>)` above the definition. /O2 implies /Oi and the\n"
     "inline expansion changes the PROLOGUE, not just the call: 0 -> 4 and\n"
     "0 -> 22 on two bodies, and removing ONLY the pragma from a byte-exact\n"
     "body returned it to #0. Worked on 3 of 3 applicable bodies.\n"
     "(2) Replace the scaffold`s `*(T *)0xADDR` casts and pointer-to-member\n"
     "call shims with real `extern` globals and real member functions. The\n"
     "shim spells `mov reg,ADDR; call reg` where the original has one\n"
     "`call rel32`; that extra scratch register moves the whole allocation.\n"
     "Reverting only the call form on a byte-exact body cost BYTE_EXACT -> #22.\n"
     "(3) Only then touch locals, and expect NO default direction: adding a\n"
     "live local was the fix 4 times, deleting one 3 times, one body needed\n"
     "both in different branches, and two were inert to all six forms. Pick\n"
     "the sign by counting `push ebx/esi/edi/ebp` in the original prologue\n"
     "against yours - too few means make a value live across the region, too\n"
     "many means recompute it at each use.\n"
     "TWO THINGS MEASURED NOT TO WORK. Do NOT declare N/4 locals to reproduce\n"
     "`sub esp,N`: eleven ints, an unreferenced array and an array with a dead\n"
     "store all left the frame at 0x18 where the original wanted 0x2c, because\n"
     "N is bytes of AGGREGATE storage - change a type`s SIZE, not the count.\n"
     "And do NOT add a destructor-carrying local to force an SEH frame unless\n"
     "the original prologue actually registers a handler: five attempts, zero\n"
     "improvements, two measurably worse."),
    (("jae", "jbe", "ja", "jb", "jge", "jle", "jg", "jl"),
     "A SIGNED COMPARE THAT CAME OUT UNSIGNED IS A LITERAL'S TYPE, not your\n"
     "comparison. MSVC types a bare hex literal like `0xfffffc18` as\n"
     "`unsigned int`, so `x >= 0xfffffc18` silently compiles to `jae` where\n"
     "the original has `jge` - no warning, and the body is WRONG rather than\n"
     "merely unmatched. Cast the operand: `x >= (int)0xfffffc18`. Measured\n"
     "2026-08-15: moved one function's divergence from #19 to #33. Check\n"
     "every negative constant written in hex, and read the original's own\n"
     "jCC to see which signedness it wanted."),
)


def ledger_row(address: int, tier: str = "", note: str = "") -> dict:
    """The verdict to reason about, MEASURED unless the caller supplies one.

    The ledger `.opensmacx/byte-match.csv` is a gitignored cache, so reading it
    made this brief silently useless in a worktree - the tier came back empty,
    no fingerprint matched, and every targeted rule switched itself off while
    the brief still printed confidently. The coordinator has usually just
    measured the batch, so it passes the verdict in; anything else measures one
    function rather than guessing from a file that may not exist.
    """
    if tier:
        return {"tier": tier, "note": note}
    # SCORE A WHOLE UNIT AS A WHOLE UNIT. `writeback.verify` wraps scaffolding
    # around whatever it is handed, and a FILE-mode landing already carries
    # its own typedefs, globals and callee declarations - so it double-declared
    # them and reported `C2733: second C linkage` as NO_COMPILE on bodies the
    # gate scores MISMATCH or BYTE_EXACT. That verdict is printed at the top of
    # this brief as "Current verdict", so EVERY brief for a FILE-mode address
    # opened by telling the agent its committed body does not compile - and a
    # NO_COMPILE carries no divergence, so every targeted lesson keyed on a
    # divergence pair switched itself off as well. Reported independently by
    # two agents in the 2026-08-14 prologue sweep.
    body = verifier.committed_body(address)[0] or ""
    whole = verifier.complete_unit(body)
    verdict = (verifier.verify_body_verbatim(address, whole) if whole
               else verifier.writeback.verify(address, body))
    return {"tier": verdict.get("tier", ""),
            "note": verdict.get("note") or verdict.get("refusal_reason") or ""}


def disassembly(address: int) -> str:
    """The target's own instructions, read from the IMAGE.

    This used to scrape a fenced block out of `mizuchi/prompts/<hex>/prompt.md`.
    That directory is gitignored and 426 MB of local generation, so in a fresh
    worktree - the isolation model the fan-out depends on - the brief degraded
    to "(no generated prompt)" and handed the agent nothing to reason from. The
    same function that wrote those prompts is imported instead, so the brief now
    depends on the pinned executable and nothing else.
    """
    import pefile
    import disassembly as prompts

    functions = _cached("functions", emit.load_functions)
    row = functions.get(address)
    if row is None:
        return f"(0x{address:08X} is not catalogued)"
    pe = _cached("pe", lambda: pefile.PE(str(verifier.byte_match.DEFAULT_EXE),
                                         fast_load=True))
    body = prompts.disassemble(pe, address, row.get("body_ranges", ""), functions)
    text = "```asm\n" + body + "\n```"
    # THE SWITCH TABLES, which are DATA and so are absent from everything
    # above. Three agents in one batch guessed a case order without them and
    # all three said so; a guessed case order compiles, reads correctly and
    # dispatches to the wrong branch.
    import jump_tables
    spans = emit.parse_body_ranges(row.get("body_ranges") or "")
    tables = jump_tables.render(pe, spans, functions)
    # WHICH C++ OBJECTS THE FRAME HOLDS, read off the destructor receivers.
    # Guessing which locals a large RAII function declares, and how they nest,
    # was the single biggest source of wasted effort in batches 18-20 - a
    # `Popup` carries a `Scroll`, two `FlatButton`, a `Spot`, a `Sprite`, a
    # `Heap` and a `Dialogs` region, and from the disassembly every one of
    # them looks like an independent local.
    import frame_objects
    frame = frame_objects.render(pe, spans, functions)
    return text + ("\n" + tables if tables else "") \
                + ("\n" + frame if frame else "")


def prompt_section(address: int, heading: str) -> str:
    """The Contract, or the Ghidra hypothesis, derived rather than scraped."""
    if heading == "Contract":
        import pefile
        import disassembly as prompts
        try:
            head = prompts.signature_head(
                address, _cached("functions", emit.load_functions),
                _cached("derived", emit.load_derived),
                _cached("callees", emit.load_callees),
                _cached("pe", lambda: pefile.PE(
                    str(verifier.byte_match.DEFAULT_EXE), fast_load=True)))
        except Exception as error:
            return f"## Contract\n\n(no scaffolding: {error})"
        return ("## Contract\n\nDefine exactly this, out of line, using the "
                "definition head VERBATIM:\n\n```cpp\n" + head + "\n```")
    import disassembly as prompts
    text = prompts.ghidra_hypothesis(address)
    if not text:
        return ""
    return ("## Ghidra decompilation (hypothesis only)\n\n```c\n"
            + text + "\n```")


def evidence(src: Path = None) -> dict:
    """{mnemonic: [(address, kind, prose)]} from LEVER/RULED-OUT in `src/`.

    THE GLOSSES ABOVE ARE CURATED AND THE EVIDENCE IS NOT, and that division is
    the point. "`jbe`/`jae` are unsigned, `jle`/`jge` are signed, so a diff
    between the families is a type error" is a human sentence; no amount of
    counting annotations produces it. What counting DOES produce is which
    addresses actually paid for it, and that accumulates mechanically as agents
    record what worked and what did not.

    So a gloss is selected by fingerprint and shown whether or not evidence
    exists - withholding it until someone had annotated a body would have
    deleted nine working rules on the day this landed. `--audit` reports the
    glosses nothing yet backs, as a worklist rather than a gate.
    """
    import annotation_scan
    root = src or (REPO_ROOT / "src")
    if not root.is_dir():
        return {}
    found = {}
    for annotation in annotation_scan.scan_tree(root):
        for key, prose in annotation.levers:
            for part in re.split(r"[/,\s]+", key):
                if part:
                    found.setdefault(part, []).append(
                        (annotation.address, "WORKED", prose))
        for prose in annotation.ruled_out:
            found.setdefault("*", []).append(
                (annotation.address, "RULED OUT", prose))
    return found


# `#0: original 'push' vs rebuilt 'mov'` - the two sides of one divergence.
DIVERGENCE_PAIR = re.compile(r"original '([\w.]+)' vs rebuilt '([\w.]+)'")

# Keyed on the PAIR, in order, because the same two mnemonics mean opposite
# things depending on which side carries which. Measured over the 1,537 scored
# mismatches whose rebuild is a real attempt rather than an empty stub:
# `push` stands on the ORIGINAL side 438 times and on the REBUILT side 514,
# and the fix is the reverse of itself between them. The single `push`/`mov`
# entry in TARGETED gave both populations the same paragraph.
DIRECTIONAL = {
    ("push", "mov"): (
        "REGISTER ALLOCATION, and the direction says which way. The ORIGINAL\n"
        "pushes where you move: it keeps MORE values live than your source.\n"
        "MAKE A VALUE LIVE - hoist a repeatedly recomputed expression into one\n"
        "local held across the calls, or index through a pointer you keep\n"
        "re-deriving. 311 bodies are on this side and the other side's fix is\n"
        "the exact reverse, so read the note. If the register is saved and\n"
        "never reused, stop: two levers are already refuted there."),
    ("mov", "push"): (
        "REGISTER ALLOCATION, and the direction says which way. YOU push where\n"
        "the original moves, so your body spills a register it never needed.\n"
        "DELETE YOUR CACHED LOCALS: the original recomputes the expression at\n"
        "each use, and hoisting it into a temp forces an EBP frame and a\n"
        "spill. Paid three times on 2026-08-13 - 0x0063D4D0, 0x00611730,\n"
        "0x005EEE70 - and 271 bodies are on this side. It is the opposite of\n"
        "what reads well, so try it deliberately.\n"
        "AN EBP FRAME YOU HAVE AND THE ORIGINAL DOES NOT IS NOT A WALL. Three\n"
        "agents in one batch reported it as a systematic one; measured over\n"
        "the whole tree, frameless originals score BYTE_EXACT at 31% (1,073 of\n"
        "3,469) against 17% for framed ones. They are EASIER. The frame is a\n"
        "symptom of the live values above, not a separate problem."),
    ("sub", "push"): (
        "REGISTER ALLOCATION, and the direction says which way. The original\n"
        "allocates with `sub esp,N` and saves NO register; you save one first.\n"
        "Same cause as `mov` vs `push` and the same cure: drop the cached\n"
        "temporaries and let the expression recompute. 173 bodies here."),
}

# A REVERSED COMPARISON, and the cheapest real lever there is: swapping the
# operand order of a relational test flips the branch VC6 emits. Measured 66
# bodies whose first divergence is exactly this - `je` for `jne` 54 times,
# `jl` for `jge` 12. `0x0063C6D0` records it as a lever in its own annotation:
# `iVar2 <= *w` where the recovery had `*w >= iVar2`.
BRANCH_FLIP = {
    "je": "jne", "jne": "je", "jl": "jge", "jge": "jl", "jg": "jle",
    "jle": "jg", "ja": "jbe", "jbe": "ja", "jb": "jae", "jae": "jb",
    "js": "jns", "jns": "js",
}


def directional_rules(note: str) -> str:
    """Advice that depends on WHICH SIDE of the divergence a mnemonic is on."""
    hit = DIVERGENCE_PAIR.search(note)
    if not hit:
        return ""
    original, rebuilt = hit.group(1), hit.group(2)
    out = []
    pair = DIRECTIONAL.get((original, rebuilt))
    if pair:
        out.append(pair)
    if BRANCH_FLIP.get(original) == rebuilt:
        out.append(
            "REVERSED COMPARISON. Your branch is the exact inverse of the\n"
            f"original's - `{original}` for `{rebuilt}` - so the condition is\n"
            "right and its polarity is not. Swap the OPERAND ORDER of the\n"
            "relational test (`a <= b` for `b >= a`): that flips the branch\n"
            "without changing what the code means. Do NOT negate the condition\n"
            "and swap the arms - that is a different body. 66 bodies in this\n"
            "tree diverge on nothing else.")
    return "\n\n".join(out)


def landing_mode(scaffolding: str) -> str:
    """What the LANDING FILE is, when the whole file is the translation unit.

    Read off the file's own marker rather than looked up a second way, so the
    brief cannot disagree with the thing that will be measured.

    A `FILE` marker means `decomp_status` compiles that file AS IT STANDS -
    `recipe == "verbatim"`. The Rules section says "submit the COMPLETE
    definition and nothing else", which is right for a body-mode landing and
    wrong here: obeying it replaces a self-contained unit with a bare
    definition and DROPS THE MARKER, and the piece stops being measured at all.

    This exact failure has now been paid twice. The agent definition's own
    example stripped the marker and cost group 5 four recoveries on
    2026-08-13; on 2026-08-14 an agent mis-landed 0x00626900 the same way and
    caught it only by running the verifier before moving on - and reported
    that all twelve of its briefs described a mode none of its files used.
    """
    if "GENERATED SKELETON" not in scaffolding and \
            not re.search(r"^// ORIGINAL: 0x[0-9A-Fa-f]{8} \w+ FILE\b",
                          scaffolding, re.M):
        return ""
    if not re.search(r"^// ORIGINAL: 0x[0-9A-Fa-f]{8}[^\n]*\bFILE\b",
                     scaffolding, re.M):
        return ""
    return (
        "# This landing is FILE mode\n\n"
        "The marker on the landing file says `FILE`: the WHOLE FILE is the\n"
        "translation unit, and the ratchet compiles it exactly as it stands.\n"
        "It carries its own typedefs, fixed-address globals, callee\n"
        "declarations and classes, and nothing is prepended to it.\n\n"
        "So the Rules below read differently for you:\n\n"
        "- Land the COMPLETE FILE, not a bare definition. Replacing the file\n"
        "  with just your definition deletes the declarations the definition\n"
        "  needs, and it compiles for nobody.\n"
        "- KEEP THE `// ORIGINAL:` LINE INCLUDING THE WORD `FILE`, byte for\n"
        "  byte. Drop the marker and the piece leaves the measured set: it is\n"
        "  not scored, not banked, and not reported as missing. Four\n"
        "  recoveries were lost this way on 2026-08-13.\n"
        "- Score it with a bare `verify_recovered_function.py 0x...`, which\n"
        "  compiles the committed file verbatim - the same bytes the gate\n"
        "  will compile. `--dir` understands this too: a candidate carrying\n"
        "  its own `FILE` marker or scaffold banner is compiled as it stands,\n"
        "  and a bare body in the same directory is still scaffolded, so you\n"
        "  can mix both. `--body` always scaffolds.\n"
        "- `--dump-unit` prints the exact text the gate will compile. Use it\n"
        "  instead of writing your own; two agents have already written that\n"
        "  script, and a second spelling of the gate's recipe is how the two\n"
        "  drift apart.\n\n")


# `mov al, 1` then `test al, cl` - the mask held in its OWN register across
# the test, which is the shape the guard-mask wall is about. Two instructions,
# because that is the whole of the idiom.
# BOTH OPERANDS MUST BE REGISTERS. `test al, 1` is the mask FOLDED INTO AN
# IMMEDIATE, which is what VC6 emits and what this wall says cannot be turned
# back into a register - so firing on it would tell an agent to stop on the
# very shape its own compiler produces.
GUARD_MASK = re.compile(
    r"\bmov\s+(?P<reg>[abcd][lh]),\s*(?:0x[0-9a-f]+|\d+)\s*$"
    r"(?:\n[^\n]*){0,2}?\n[^\n]*\b(?:test|or|and)\s+"
    r"(?:(?P=reg),\s*[a-z]{2,3}|[a-z]{2,3},\s*(?P=reg))\s*$",
    re.M)


def guard_mask_section(asm: str) -> str:
    """The guard-mask wall, fired on the BODY instead of on three mnemonics.

    THIS WALL WAS KEYED ON `or`, `test` AND `and`. Those are three of the
    commonest mnemonics in the image, and the entry told an agent that three
    other agents had ruled the case out exhaustively and it should stop.

    Measured 2026-08-14: it fired on 130 scored bodies and NOT ONE of them
    contained the idiom it describes. The idiom is real and lives in exactly
    33 bodies - `mov al, 2; test al, cl` in `0x005D71F0`, `0x005EB370` and
    31 more - and the entry reached none of them. A wall that fires where it
    does not apply is worse than no wall: the agent cannot tell it was
    mis-selected, and the instruction is to stop.

    Keyed on the disassembly the brief already carries, so it fires when the
    shape is present and is silent otherwise.
    """
    if not asm or not GUARD_MASK.search(asm):
        return ""
    return (
        "GUARD-MASK REGISTER. The original holds the mask in its OWN register\n"
        "across the test - `mov al,1; test al,cl` - and VC6 folds a mask into\n"
        "an immediate and reuses one register. Three agents ruled this out\n"
        "separately, across /O1 and /O2, framed and frameless, with a plain\n"
        "immediate, a named local, a const, a volatile-adjacent read, an\n"
        "explicit `x|0xFF`, `register` storage, a 1-bit bitfield, a discarded\n"
        "__fastcall parameter, and a byte-exact sibling's idiom copied\n"
        "verbatim. Only 33 bodies in the image have this shape and yours is\n"
        "one of them: if the mask register is your only divergence, stop.\n\n")


def class_section(name: str) -> str:
    """What `src/` already declares about the subject's class.

    THE SCAFFOLD IS NOT THE SUM OF WHAT IS KNOWN, and nothing said so. A class
    whose SIZE is not pinned is emitted as an opaque shell - deliberately,
    because a wrong offset compiles and fails later as a byte mismatch nobody
    traces back - and the brief then presented that shell as if it were the
    whole record. Measured 2026-08-14: 1,080 catalogued functions across 68
    classes are in that state, 162 of them still unrecovered.

    So an agent recovering `PushButton::PushButton` saw `class PushButton {
    public: PushButton(); };` and had no way to learn that `src/pushbutton.h`
    declares it deriving from BaseButton with 137 inherited members. It landed
    the field stores without the base-constructor call and reported the shell
    as the defect. The shell was not the defect; the silence was.

    NAMED AS A HYPOTHESIS, and the offsets are not restated here. The gate
    that keeps unproved layouts out of the scaffold is right and this does not
    route around it: the brief points at the tree's own record and says what it
    is worth, which is the same thing `arity_hypothesis` does for a prototype.
    An agent can read the header, check an offset against the disassembly in
    front of it, and record what it confirms - and a confirmed offset reaches
    the next scaffold through `agent-structure-observations.csv`.
    """
    import class_layouts
    if not name.startswith("?") or "@@" not in name:
        return ""
    # TWO SHAPES. `?init@Buffer@@QAE...` puts the method first and the class
    # second; `??0PushButton@@QAE...` is a constructor and the class IS the
    # first token, after the `??0`/`??1` tag. Reading position 1 for both
    # returned an empty string for every lifecycle function - which is exactly
    # the population that needs this section, since a constructor is where a
    # base subobject gets built.
    # THE TAG IS EXACTLY `??0`, `??1` or `??_X`, never a run of capitals.
    # `[0-9A-Z_]+` is greedy and ate the class's own first letter, so
    # `??0PushButton@@` decoded as a class called `ushButton` - which is
    # declared nowhere, so the section stayed silent and looked correct.
    lifecycle = re.match(r"^\?\?(?:[0-9]|_[A-Z])(?P<klass>\w+)@@", name)
    if lifecycle:
        klass = lifecycle.group("klass")
    else:
        parts = name.split("@")
        klass = parts[1] if len(parts) > 2 else ""
    declared = class_layouts._declared_classes()
    if not klass or klass not in declared:
        return ""
    header = ""
    for candidate in sorted((class_layouts.SRC).glob("*.h")):
        if any(n == klass for n, _, _ in
               class_layouts.class_bodies(candidate.read_text(errors="ignore"))):
            header = f"src/{candidate.name}"
            break
    if class_layouts.supplyable(klass):
        return ""          # the scaffold already carries it; nothing to add
    chain = class_layouts.base_chain(klass)
    members = class_layouts.members_of(class_layouts._body_of(klass))
    lines = [f"# `{klass}` is declared in {header}, and your shell is not it",
             ""]
    if chain:
        lines.append(f"It derives from {' -> '.join(chain)}. The scaffold "
                     f"emits it FLAT or opaque,")
        lines.append("so there is no base subobject in your unit to construct "
                     "or to pass `this` as.")
    lines.append(
        f"{header} declares "
        + (f"{len(members)} data member(s)" if members else "a layout this "
           "cannot read")
        + ", and the scaffold does NOT supply them:")
    lines.append(
        f"`sizeof({klass})` is not pinned against the image, so an offset "
        f"taken from that")
    lines.append(
        "header is a HYPOTHESIS - one missing member shifts every field after "
        "it.")
    lines.append("")
    lines.append(
        "Read it if you need field names, check any offset you use against "
        "the disassembly")
    lines.append(
        "above, and report what you CONFIRM as a `member` observation. A "
        "confirmed offset")
    lines.append(
        "reaches the next agent's scaffold; an assumed one reaches nobody and "
        "costs a match.")
    lines.append("")
    return "\n".join(lines) + "\n"


def targeted_rules(note: str, found: dict = None) -> str:
    """The curated glosses this divergence selects, plus what paid for them."""
    # Punctuation is stripped, not just quotes: a compiler diagnostic arrives as
    # `error C2065: 'x' : undeclared identifier`, so keying on bare words missed
    # every C-number - which is the largest family of blockers in the tree.
    words = set(re.split(r"[^\w<>]+", note.replace("'", " ")))
    wanted = [advice for triggers, advice in TARGETED
              if words & set(triggers)]
    found = evidence() if found is None else found
    cited = []
    for word in sorted(words):
        for address, kind, prose in found.get(word, [])[:3]:
            cited.append(f"  {kind:<9} 0x{address:08X}  {prose}")
    directional = directional_rules(note)
    if directional:
        # IT REPLACES the keyed entry rather than joining it. A brief that
        # grows is a brief that gets skimmed - `test_it_stays_small` is the
        # guard - and where a directional rule fires it knows strictly more
        # than the keyed one: the keyed entry matches on either mnemonic
        # appearing anywhere, and this one has read which side each is on.
        # Only the entries this pair fully explains are dropped; an entry
        # keyed on a third mnemonic still has something to say.
        hit = DIVERGENCE_PAIR.search(note)
        pair = set(hit.groups()) if hit else set()
        wanted = [advice for triggers, advice in TARGETED
                  if (words & set(triggers)) and not set(triggers) <= pair]
        wanted.insert(0, directional)
    block = "\n\n".join(wanted) if wanted else (
        "No fingerprint matched this divergence. Reason from the disassembly.")
    if cited:
        block += ("\n\nWhat bodies in this tree recorded against these "
                  "mnemonics:\n" + "\n".join(cited))
    return block


HYPOTHESES_CSV = REPO_ROOT / "docs" / "recovery" / "prototype-hypotheses.csv"


def arity_hypothesis(address: int) -> str:
    """IDA's argument count for a `sub_*` the mangled name cannot describe.

    A mangled name states its own argument list, so `derive_prototypes_from_names`
    settles every catalogued row that has one, and `emit` already scaffolds
    those correctly. What it cannot help is a function IDA never named: the
    scaffold falls back to a nullary `extern "C" int __cdecl sub_xxxx()`, and
    the agent reconstructs the real arity from `[esp+N]` reads by hand.

    Measured 2026-08-14: 274 placeholders are `sub_*` with no prototype, and
    217 of them ALREADY HAVE an evidenced argument count sitting in
    `prototype-hypotheses.csv` that reached nothing. Batch 4 turned ~24% of its
    addresses into signature proposals, which is that gap priced.

    SHOWN AS A HYPOTHESIS, NEVER AS THE CONTRACT. It is one source - I checked
    whether Ghidra could corroborate it and it cannot: for these addresses
    `ghidra-functions.csv` holds `undefined FUN_xxxxxxxx()` with
    `calling_convention=unknown`, so there is nothing to cross-check against.
    IDA's `guess_type` is a guess, it is right often enough to be worth a look,
    and the disassembly in this brief is the thing that settles it.
    """
    if not HYPOTHESES_CSV.is_file():
        return ""
    rows = _cached("hypotheses", lambda: {
        int(r["address"], 16): r
        for r in csv.DictReader(HYPOTHESES_CSV.open())
        if r.get("address")})
    row = rows.get(address)
    if not row:
        return ""
    try:
        count = int(row.get("argument_count") or 0)
    except ValueError:
        return ""
    if count <= 0:
        return ""
    return f"""
# IDA guessed {count} argument(s) for this function

The scaffolding could not: this address has no mangled name, so the definition
head above is a fallback, not a decoded contract. IDA's own analysis says:

    {row.get('evidence', '').strip()}

**This is a hypothesis from one tool, not a second record.** Ghidra has nothing
for this address (`undefined FUN_{address:08x}()`, convention unknown), so
there is no cross-check. Take it as a starting point and let the DISASSEMBLY
decide: {count} stack argument(s) means reads at `[esp+4]`, `[esp+8]`, ...
before any push, or `[ebp+8]`, `[ebp+0xC]`, ... with a frame. A receiver in
`ecx` with no matching stack slot is a `__thiscall` `this` and is NOT one of
these arguments.
"""


def vcall_section(scaffolding: str) -> str:
    """How to reach the generated vtable shim, shown only when there is one.

    Keyed on the SCAFFOLD, not on a divergence: an agent needs this before it
    has a divergence, and a body that dispatches virtually needs it every time.

    Two measured facts, both of which cost matches while they lived only in the
    coordinator's prompt. 142 of 1,891 landed bodies wrote their OWN shim
    class - the single most common workaround in the tree.
    """
    if "class VCall" not in scaffolding:
        return ""
    return """
# Reaching the vtable shim

**Cast ONE level, not two.** `reinterpret_cast<VCall *>(this)->slot002()` is
the call. Dereferencing to get the vtable and then indexing through it emits an
extra load that the original does not have - two byte-exact matches in one
batch turned on exactly this.

**Retype the slot you need, in this file.** The generated shim types every slot
`void()`, which cannot express a slot that takes arguments or returns a value.
Only DECLARATION ORDER fixes the vtable offset, so changing a slot's signature
moves nothing. Edit `VCall` where it sits in the unit rather than declaring a
second class beside it.

ONE SHIM IS NOT ALWAYS ENOUGH: the same slot INDEX can carry different
signatures on `this` than on a child object the body also dispatches through.
When that happens a second, differently-named shim class is correct.
"""


def thunk_section(asm: str) -> str:
    """The two thunk shapes, told apart by their jump operand.

    A body that is ONE jump is a thunk, and which kind decides whether it is a
    one-candidate match or a wall. Measured across batch 2 (2026-08-13), where
    thunks were 12 of 38 matches and four agents each rediscovered the same
    lever from scratch because no brief carried it.

    `jmp <address>` - a tail call into another function. Expressible, and
    reliably so. `jmp dword ptr [addr]` - a linker import stub. Not
    expressible, and three agents proved it separately before the family was
    counted, which is exactly the spend this section exists to prevent.
    """
    body = [line.strip() for line in asm.splitlines()
            if line.strip().startswith("0x")]
    if len(body) != 1 or " jmp " not in f" {body[0]} ":
        return ""
    if "ptr [" in body[0]:
        return """
# This is an import stub, and it is a known wall

The whole body is `jmp dword ptr [slot]` - a linker-generated stub into the
import table. **VC6 will not tail-call an indirect `__stdcall` callee**, so
every source form lowers to `push`/`call`/`ret` and never the bare 6-byte
jump. A function-pointer cast, an `extern` slot, a forwarding wrapper and a
`__declspec(dllimport)` call were each tried and each failed, by three agents
independently.

The image holds 13 of these, 78 bytes, in four contiguous runs; 8 are already
`EXCLUDED`. **Write the clearest forwarding body, land it, and move on** -
do not search for a spelling. Note in `RULED-OUT:` what you tried.

An import reached by `call` rather than `jmp` IS expressible: cast the fixed
IAT slot to a function-pointer type and call through it. That is a different
shape and it matches.
"""
    return """
# This is a tail-call thunk - use a shim class

The whole body is one `jmp` into another function, so the source is a plain
tail call: `return target(args);` with a matching signature folds to exactly
that jump, and the relocation the jump carries is MASKED by the comparison.

The catch, and the reason this needs saying: the callee is usually a member of
a class the scaffolding already declares as CLOSED, without that method on it.
Do not edit the scaffold's class. Declare your OWN class with the same method
signature and dispatch through it:

    class Shim { public: void close(); };          // your own, in this file
    reinterpret_cast<Shim *>(this)->close();

The shim's class name does NOT have to match the real callee's - only the
signature and the `__thiscall` receiver matter, because the comparison is over
code and the call target is masked. For a virtual call, declare the slots up
to the one you need so declaration ORDER puts it at the right vtable offset.

Twelve byte-exact matches in one batch came from this shape. It is usually a
one-candidate answer.
"""


def lifecycle_section(name: str) -> str:
    """The store-order rule, shown only on a constructor or destructor.

    Keyed on the MANGLED NAME rather than on a divergence, because it is a
    prior about a whole family and not a response to one diff - and because the
    diff it produces, MNEMONIC_ONLY, carries no note to key on: every mnemonic
    matches and only the operands differ, so `targeted_rules` sees an empty
    string and says "no fingerprint matched".

    Paid four times in the batch of 2026-08-13, which is a third of that run's
    hand-written matches: 0x00618EA0 `Font::Font`, 0x005D4560 `Heap::Heap`,
    0x006161D0 `Time::Time`, 0x005FA860 `Spot::Spot`. Constructors and
    destructors are dense in what is left of the frontier, so this fires often.
    """
    if not re.match(r"\?\?[01]", name or ""):
        return ""
    return """
# This is a constructor or destructor - check the STORE ORDER first

Write the field stores as ASSIGNMENTS IN THE BODY, in the order the
disassembly writes them, which is often NOT declaration order. On
0x00618EA0 the first-declared member is stored LAST.

A member-init list CANNOT express this: C++ runs initialisers in declaration
order however you write them, so an out-of-order original is unreachable that
way. Declaration order scores MNEMONIC_ONLY - every mnemonic right, every
operand wrong - which looks like a hard register problem and is not one.

Measured BYTE_EXACT this way on Font, Heap, Time and Spot.
"""


def fresh_recovery_section(address: int) -> str:
    """What to send when NOTHING is recovered yet.

    A tier still exists for these rows, because the census scores
    every catalogued function and an unrecovered one gets an empty scaffold -
    so the note reads `#0: original 'push' vs rebuilt 'xor'`, which is not a
    divergence at all, it is the shape of a function that does not exist. Left
    alone, the brief hands an agent a blank body, a made-up location and a
    paragraph of register-allocation advice about a stub. There are 278
    unrecovered leaves under 64 bytes, so this is most of the remaining work.

    What a fresh recovery needs instead is the definition head the scaffolding
    will compile against and the decompiler's guess at the body, both of which
    `disassembly.py` already writes.
    """
    contract = prompt_section(address, "Contract")
    ghidra = prompt_section(address, "Ghidra decompilation (hypothesis only)")
    if not contract and not ghidra:
        return ("# Nothing is recovered yet\n\nThere is no committed body and "
                "no generated prompt for this address. Run\n"
                "`tools/disassembly.py` first - without the contract "
                "there is no way to\nknow what definition head the scaffolding "
                "will accept.\n")
    return f"""# Nothing is recovered yet - write it from scratch

There is no committed body. Any tier shown above was scored against an EMPTY
scaffold, so its "divergence" is just the shape of a function that does not
exist. IGNORE IT and work from the disassembly.

{contract}

{ghidra}
"""


# Identifiers that carry no meaning: IDA's address-derived placeholders, the
# emitter's own, and the decompiler's positional argument names. A body full of
# these compiles identically to a body that says what it means, so the compiler
# will never ask for better ones - which is exactly why asking has to be part
# of the task rather than left to whoever reads the diff later.
PLACEHOLDER = re.compile(
    r"\b(?:sub_[0-9a-fA-F]{6,}"
    r"|(?:leaf|nullsub|unknown|j)_[0-9a-fA-F]{4,}\w*"
    r"|[aA]\d{1,2}|arg_\w+|v\d{1,2}|dword_\w+|off_\w+|unk_\w+"
    r"|param\d*|result\d*|field_\w+)\b")


def placeholders(name: str, body: str) -> list:
    """The meaningless identifiers in this target, in first-seen order."""
    seen = []
    for found in PLACEHOLDER.findall(f"{name}\n{body or ''}"):
        if found not in seen:
            seen.append(found)
    return seen


def naming_section(name: str, body: str) -> str:
    """Ask for better names, but ONLY when there are placeholders to replace.

    An agent that has just read twenty bytes of pointer arithmetic knows more
    about what a function does than anyone who will look at it later, and that
    knowledge is thrown away the moment it returns a body still calling things
    `a1`. But a target whose names are already good does not need the
    paragraph, and every line here is budget not spent on the match.
    """
    found = placeholders(name, body)
    if not found:
        return ""
    return f"""
# Naming

These identifiers say nothing: {', '.join(f'`{f}`' for f in found[:8])}.

You will understand this function better than its next reader. Propose better
names for it and for its parameters, derived from what the disassembly and the
body actually DO - what is being counted, indexed, compared, returned. A name
you cannot justify from the code is worse than the placeholder, so say
"unclear" rather than inventing one.

Renaming is FREE with respect to the match: the comparison is over the emitted
code, and the code does not change when an identifier does. So rename in the
body you submit, and list the renames separately in your report - the function
name is catalogue data and gets applied there, not in the body alone.
"""


_CACHE = {}


def reset_cache() -> None:
    """Drop the per-run loads.

    Anything that REPLACES one of the cached loaders - `emit.load_functions`,
    `landing_path`, the image - has to call this on the way in and on the way
    out, or the cache outlives the replacement. That is not hypothetical: two
    test classes stub `load_functions` to a one-row catalogue, and the first
    brief they built left that row cached for every later test in the process,
    so the real-brief size guard silently measured a brief whose disassembly
    section read "(0x005E3630 is not catalogued)".
    """
    _CACHE.clear()


def _cached(key, build):
    """Memoise a per-run load so `--addresses` pays for it once.

    A single brief costs ~24 s, and almost none of it is the brief: it is
    `emit.load_functions()` walking 3,633 files under src/, `landing_path()`
    walking them again, and `pefile.PE()` on the shipped image - all three
    identical for every address in a batch. Generating 72 briefs one process at
    a time was ~30 minutes of re-reading the same data. Through `--addresses`
    it is one load and then a few hundred milliseconds each.

    Deliberately process-local and never persisted. A cache on disk would have
    to know when src/ changed, and this tool is run BETWEEN edits to src/ by
    the coordinator - a stale brief would send an agent to work that is already
    done, which is the failure `recovery_frontier` was joined against
    `annotation_scan` to stop.
    """
    if key not in _CACHE:
        _CACHE[key] = build()
    return _CACHE[key]


def interpreter() -> str:
    """The python that can actually run the scorer, as the brief should spell it.

    The brief used to print `tools/verify_recovered_function.py <addr> ...`
    bare. That file is not executable and its shebang is `/usr/bin/env
    python3`, which has none of the dependencies: run as printed it exits 126,
    and run under the system python it exits 1. So the one command the brief
    exists to hand over did not work, and every agent paid a few turns
    rediscovering the venv.

    Taken from `sys.executable` rather than written down, so it is right by
    construction in any checkout, and shortened to a repo-relative path when it
    lives inside the tree - which is the form the rest of the repo uses.
    """
    python = Path(sys.executable)
    try:
        return str(python.relative_to(REPO_ROOT))
    except ValueError:
        return str(python)


def landing_path(address: int, src: Path = None) -> str:
    """The ONE repository file an agent on this address may write.

    The brief used to say "write only to /tmp", which is wrong in a way that
    costs a whole batch: the coordinator banks with
    `decomp_status.py --record-matches`, and that measures the SOURCE TREE. A
    body proved in /tmp and never landed is measured as the placeholder it
    replaced, so the batch banks zero and it reads as agents failing rather
    than as a brief contradicting the loop.

    The path is looked up rather than composed. `src/unrecovered/<hex>.cpp` is
    the usual home but not the rule - of 48 addresses prepared on 2026-08-13,
    one already lived in `src/recovered/` - and a composed path would have sent
    that agent to a file nothing measures.
    """
    import annotation_scan
    root = src or (REPO_ROOT / "src")
    if not root.is_dir():
        return ""
    index = _cached(("landing", str(root)), lambda: {
        a.address: str(a.path) for a in annotation_scan.scan_tree(root)})
    return index.get(address, "")


def brief(address: int, tier: str = "", note: str = "") -> str:
    # `tier`/`note` come from the coordinator, which has just measured the
    # batch. Left empty, `ledger_row` measures this one function rather than
    # reading a gitignored cache that may not exist.
    row = ledger_row(address, tier, note)
    body, location = verifier.committed_body(address)
    functions = _cached("functions", emit.load_functions)
    name = (functions.get(address) or {}).get("name", f"sub_{address:x}")
    # From the CATALOGUE, not from the verdict: the verdict is now measured or
    # passed in and carries no size, and `? bytes` in a brief is the agent's
    # first cue for how hard this is.
    size = (functions.get(address) or {}).get("size", "?")
    note = row.get("note") or row.get("refusal_reason") or ""
    landing = landing_path(address) or (
        f"NONE - no annotation claims 0x{address:08X}, so there is nowhere to "
        f"land it; report that rather than creating a file")
    python = interpreter()
    asm = disassembly(address)
    # The unit as it stands, so a section can key on what the SCAFFOLD holds
    # rather than on a divergence the agent does not have yet.
    try:
        scaffolding = (REPO_ROOT / landing).read_text(errors="replace") \
            if landing and not landing.startswith("NONE") else ""
    except OSError:
        scaffolding = ""
    # Keyed on the SHAPE of the body rather than on a name or a divergence: a
    # thunk is a one-instruction function, and which instruction it is decides
    # between a one-candidate match and a known wall.
    thunk = thunk_section(asm)

    if body is None:
        verdict = "nothing recovered yet"
        middle = (fresh_recovery_section(address) + landing_mode(scaffolding)
                  + class_section(name) + guard_mask_section(asm) + thunk
                  + arity_hypothesis(address) + vcall_section(scaffolding)
                  + lifecycle_section(name))
    else:
        verdict = f"{row.get('tier', '?')}{(' - ' + note) if note else ''}"
        middle = (f"# The body as it stands ({location})\n\n"
                  f"```cpp\n{body.strip()}\n```\n\n"
                  f"# What the divergence usually means\n\n"
                  f"{targeted_rules(note)}\n"
                  f"{landing_mode(scaffolding)}"
                  f"{class_section(name)}"
                  f"{guard_mask_section(asm)}"
                  f"{thunk}"
                  f"{arity_hypothesis(address)}"
                  f"{vcall_section(scaffolding)}"
                  f"{lifecycle_section(name)}"
                  f"{naming_section(name, body)}")

    return f"""Matching decompilation, MSVC 6.0 x86. One function. Everything you
need is below - do not go looking for anything else.

# Target

`{name}` at 0x{address:08X}, {size} bytes.
Current verdict: {verdict}

# What the original compiles to

{asm}

{middle}
# The signature is yours to change

Nothing pins it. The scaffolding does NOT declare this function, and the
comparison is over the object's CODE rather than a symbol lookup, so a
different return type, convention or parameter list still scores. Change it
when the DISASSEMBLY contradicts the current one:

- reads `[ecx+N]` with no stack access -> the receiver is `this`, so this is a
  `__thiscall` member and needs a class in your unit to say so
- `ret N` with N != 0 -> a callee-pop convention, `__stdcall` or `__thiscall`
- sets `eax` before returning where the signature says `void` -> it returns

Change it on evidence, never on style: rewriting a fake-thiscall as a real
`__thiscall` member on 0x005E3630 changed NOT ONE BYTE.

A changed signature is a proposal about the CATALOGUE, which owns the mangled
name and the declaration every caller sees - so report it as its own item. A
byte-exact body whose signature silently disagrees with the catalogue cannot
land: it fails on `C2371`/`C2556` at integration, which is how an earlier
return-type change turned MISMATCH into NO_COMPILE.

# Your loop

Write a candidate to a file under /tmp and score it. This writes NOTHING to the
repository:

    {python} tools/verify_recovered_function.py 0x{address:08X} --dir /tmp/variants
    {python} tools/verify_recovered_function.py 0x{address:08X} \\
        --body /tmp/cand.cpp --against-committed

Exit 0 means BYTE_EXACT and nothing else does. `--dir` scores every *.cpp in the
directory against one loaded image and ranks them, which is the mode to reach
for: the question is "which of these nine", not "is this one it".
`--against-committed` refuses a candidate that is WORSE than what is there.
Iterate until exit 0, or until you can say what you ruled out.

# The lever that keeps working

The context spells fixed globals `static T *const g_ADDR = (T *)0xADDR;`. That
is right for a plain load or store and WRONG wherever the ADDRESS ITSELF does
work, because `/O2` folds the literal away and the instruction the original
used disappears with it. Measured in three unrelated shapes:

    read-modify-write   `dec [addr]`   became load/modify/store
    indexed table base  `lea`          became `add`
    `guard ? obj : 0`   `neg/sbb/and`  vanished entirely

Declare your own `extern T name;` (or `extern T name[];`) and use it plainly.
BYTE_EXACT in all three. The relocation it needs is MASKED by the comparison,
so it costs nothing, and it reads better than an address. Plain loads and
stores keep the context's global.

Parameter reads exactly 4 too high are the FRAME POINTER, not a missing `this`.

# Rules

- Submit the COMPLETE definition and nothing else. The scaffolding supplies
  every declaration except a class you introduce to express `__thiscall`.
- No `__asm`, no `_emit`. The verifier refuses them before compiling: pasting
  the original's instructions proves nothing about the source, and
  AGENTS.md:5 bars copied machine code from distributable builds.
- Readability counts as well as byte-exactness. Keep the existing shape and
  naming where it is already right; change only what the diff demands.
- Write to /tmp, and to ONE repository file: {landing}. Land your final body
  there, matched or not - the coordinator banks by measuring the source tree,
  so a body left in /tmp banks nothing. Nothing else, and no `git`.

# Report

Four items, each labelled, and omit the ones that do not apply:

1. BODY - the final definition in one ```cpp block.
2. VERDICT - the line the tool printed, verbatim. Not your reading of it.
3. CHANGE - one sentence on what the source-form change was. If you did not
   reach BYTE_EXACT, what you RULED OUT instead; that is a result, not a
   failure, and it stops the next agent paying for it again.
4. PROPOSALS - signature changes and renames, as `old -> new` with the
   evidence for each. These are catalogue edits and are applied separately
   from the body, so they are useless buried in prose.
5. STRUCTURE - what you learned about the DATA that this function does not
   itself need. One line each, in the shape

       kind | subject | offset | what it is | what proves it

   where kind is one of: member, member-type, class, embedded, vtable-slot,
   table, callgraph, emitter. Offsets in hex, and `-` where one does not
   apply. Examples of the real thing, all found this way:

       member      | UAmbience | 0x6C  | one-byte flag, cleared on hide
                   | mov byte ptr [esi+0x6c], 0 with esi as this
       vtable-slot | Win       | 0x128 | slot 74 takes two ints
                   | two pushes then call [eax+0x128]
       table       | -         | 0xA20 | 512 records of three uint32
                   | do/while of 0x200 writing three dwords, advancing by 3
       emitter     | -         | -     | the VCall shim declares every slot
                   | nullary, so any slot taking arguments needs a second shim

   EVIDENCE IS REQUIRED. An observation without it is a guess in citation
   format, and it will be dropped. Say nothing rather than guess a field's
   meaning; an offset and a width with no story is still worth recording.
   These go to `docs/recovery/agent-structure-observations.csv`, which is how
   the next agent on the same class starts knowing what you just worked out.
"""


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("address", nargs="?")
    parser.add_argument("--audit", action="store_true",
                        help="report which glosses have evidence in src/ and "
                             "which fingerprints have evidence but no gloss")
    # The coordinator has just measured the batch; passing the verdict in keeps
    # the brief from measuring the same function a second time. Omitted, it
    # measures - it never guesses from a cache.
    parser.add_argument("--tier", default="",
                        help="the verdict to reason about (default: measure it)")
    parser.add_argument("--note", default="",
                        help="the divergence note that goes with --tier")
    parser.add_argument("--addresses",
                        help="comma-separated hex addresses; writes one brief "
                             "per address and loads the catalogue ONCE")
    parser.add_argument("--out-dir", type=Path,
                        help="where --addresses writes (default build/briefs)")
    arguments = parser.parse_args(argv)

    if arguments.audit:
        found = evidence()
        keyed = {trigger for triggers, _ in TARGETED for trigger in triggers}
        backed = sorted(k for k in keyed if found.get(k))
        bare = sorted(k for k in keyed if not found.get(k))
        loose = sorted(k for k in found if k not in keyed and k != "*")
        walls = len(found.get("*", []))
        print(f"lesson coverage: {len(TARGETED)} glosses over {len(keyed)} "
              f"fingerprints; {len(backed)} backed by an annotation, "
              f"{len(bare)} not yet; {walls} RULED-OUT list(s) recorded")
        print("\n  backed:      " + (", ".join(backed) or "(none yet)"))
        print("  no evidence: " + (", ".join(bare) or "(none)"))
        if loose:
            print("  evidence with NO gloss - candidates for a new rule:")
            print("      " + ", ".join(loose))
        # A report, never a gate: making it one would put every recovery that
        # records a lesson back into a file all of them share, which is the
        # merge point worktree-isolated batches exist to remove.
        return 0

    if arguments.addresses:
        # One process, one catalogue load. See `_cached`.
        out = arguments.out_dir or (REPO_ROOT / "build" / "briefs")
        out.mkdir(parents=True, exist_ok=True)
        addresses = [int(t, 16) for t in
                     arguments.addresses.replace(" ", "").split(",") if t]
        written = failed = 0
        for address in addresses:
            try:
                text = brief(address, arguments.tier, arguments.note)
            except Exception as error:            # one bad address must not
                print(f"0x{address:08X}: {error}", file=sys.stderr)
                failed += 1                        # take the batch down
                continue
            # The trailing newline `print` adds on the single-address path, so
            # the two paths are ONE projection and a test can assert equality
            # rather than equality-modulo-whitespace.
            (out / f"0x{address:08X}.md").write_text(text + "\n")
            written += 1
        print(f"wrote {written} brief(s) to {out}"
              + (f"; {failed} failed" if failed else ""))
        return 1 if failed else 0

    if not arguments.address:
        parser.error("an address is required unless --audit")
    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2
    print(brief(address, arguments.tier, arguments.note))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
