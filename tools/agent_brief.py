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
LEDGER = REPO_ROOT / ".opensmacx" / "byte-match.csv"
PROMPTS = REPO_ROOT / "mizuchi" / "prompts"

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
     "there. If that is your only divergence, stop."),
    # WALLS. Ruled out exhaustively; each is keyed like every other entry so an
    # agent meets the one that applies rather than a catalogue of all of them.
    (("or", "test", "and"),
     "GUARD-MASK REGISTER. If the original holds the mask in its OWN register\n"
     "across the test - `mov al,1; test al,cl; or cl,al` - stop. VC6 folds the\n"
     "mask into an immediate and reuses one register. Three agents ruled this\n"
     "out separately, across /O1 and /O2, framed and frameless, with a plain\n"
     "immediate, a named local, a const, a volatile-adjacent read, an explicit\n"
     "`x|0xFF`, `register` storage, a 1-bit bitfield, a discarded __fastcall\n"
     "parameter, and a byte-exact sibling's idiom copied verbatim."),
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
    (("<end>",),
     "A LENGTH MISMATCH where the ORIGINAL ends first is usually a span that\n"
     "excludes its own `ret` because the linker folded the tail onto another\n"
     "function. Check whether the branch target is the ENTRY of a different\n"
     "catalogued symbol; if it is, this is unreachable."),
)


def ledger_row(address: int) -> dict:
    with LEDGER.open(newline="", encoding="utf-8-sig") as handle:
        for row in csv.DictReader(handle):
            if row["address"].upper() == f"0X{address:08X}":
                return row
    return {}


def disassembly(address: int) -> str:
    """The target's own instructions, lifted from the generated prompt."""
    path = PROMPTS / f"{address:08x}" / "prompt.md"
    if not path.is_file():
        return "(no generated prompt; run tools/emit_mizuchi_prompts.py)"
    text = path.read_text()
    start = text.find("```asm")
    if start < 0:
        return "(the generated prompt carries no disassembly)"
    end = text.find("```", start + 6)
    return text[start:end + 3]


def prompt_section(address: int, heading: str) -> str:
    """One `## heading` section of the generated prompt, or ''."""
    path = PROMPTS / f"{address:08x}" / "prompt.md"
    if not path.is_file():
        return ""
    text = path.read_text()
    start = text.find(f"## {heading}")
    if start < 0:
        return ""
    end = text.find("\n## ", start + 1)
    return text[start:end if end > 0 else len(text)].strip()


def targeted_rules(note: str) -> str:
    words = set(note.replace("'", " ").split())
    wanted = [advice for triggers, advice in TARGETED
              if words & set(triggers)]
    return "\n\n".join(wanted) if wanted else (
        "No fingerprint matched this divergence. Reason from the disassembly.")


def fresh_recovery_section(address: int) -> str:
    """What to send when NOTHING is recovered yet.

    The ledger still carries a tier for these rows, because the census scores
    every catalogued function and an unrecovered one gets an empty scaffold -
    so the note reads `#0: original 'push' vs rebuilt 'xor'`, which is not a
    divergence at all, it is the shape of a function that does not exist. Left
    alone, the brief hands an agent a blank body, a made-up location and a
    paragraph of register-allocation advice about a stub. There are 278
    unrecovered leaves under 64 bytes, so this is most of the remaining work.

    What a fresh recovery needs instead is the definition head the scaffolding
    will compile against and the decompiler's guess at the body, both of which
    `emit_mizuchi_prompts.py` already writes.
    """
    contract = prompt_section(address, "Contract")
    ghidra = prompt_section(address, "Ghidra decompilation (hypothesis only)")
    if not contract and not ghidra:
        return ("# Nothing is recovered yet\n\nThere is no committed body and "
                "no generated prompt for this address. Run\n"
                "`tools/emit_mizuchi_prompts.py` first - without the contract "
                "there is no way to\nknow what definition head the scaffolding "
                "will accept.\n")
    return f"""# Nothing is recovered yet - write it from scratch

There is no committed body. The ledger still shows a tier for this row
because the census scores every catalogued function and an unrecovered one
gets an empty scaffold, so its "divergence" is just the shape of a function
that does not exist. IGNORE IT and work from the disassembly.

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


def brief(address: int) -> str:
    row = ledger_row(address)
    body, location = verifier.committed_body(address)
    functions = emit.load_functions()
    name = (functions.get(address) or {}).get("name", f"sub_{address:x}")
    note = row.get("note") or row.get("refusal_reason") or ""

    if body is None:
        verdict = "nothing recovered yet"
        middle = fresh_recovery_section(address)
    else:
        verdict = f"{row.get('tier', '?')}{(' - ' + note) if note else ''}"
        middle = (f"# The body as it stands ({location})\n\n"
                  f"```cpp\n{body.strip()}\n```\n\n"
                  f"# What the divergence usually means\n\n"
                  f"{targeted_rules(note)}\n"
                  f"{naming_section(name, body)}")

    return f"""Matching decompilation, MSVC 6.0 x86. One function. Everything you
need is below - do not go looking for anything else.

# Target

`{name}` at 0x{address:08X}, {row.get('size', '?')} bytes.
Current verdict: {verdict}

# What the original compiles to

{disassembly(address)}

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

    cd /home/fran6co/code/OpenSMACX
    /home/fran6co/.venv/bin/python3 tools/verify_recovered_function.py \\
        0x{address:08X} --body /tmp/cand.cpp --against-committed

Exit 0 means BYTE_EXACT. Anything else prints the first differing mnemonic and
its index, and refuses a candidate that is WORSE than what is committed.
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
- Do not edit anything under /home/fran6co/code/OpenSMACX. Write only to /tmp.

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
    parser.add_argument("address")
    arguments = parser.parse_args(argv)
    try:
        address = int(arguments.address, 16)
    except ValueError:
        print(f"error: {arguments.address} is not a hex address",
              file=sys.stderr)
        return 2
    print(brief(address))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
