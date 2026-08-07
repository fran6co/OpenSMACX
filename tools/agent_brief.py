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
LEDGER = REPO_ROOT / "docs" / "recovery" / "byte-match.csv"
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
     "two levers already refuted. Timebox it and report what you tried."),
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


def targeted_rules(note: str) -> str:
    words = set(note.replace("'", " ").split())
    wanted = [advice for triggers, advice in TARGETED
              if words & set(triggers)]
    return "\n\n".join(wanted) if wanted else (
        "No fingerprint matched this divergence. Reason from the disassembly.")


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

    return f"""Matching decompilation, MSVC 6.0 x86. One function. Everything you
need is below - do not go looking for anything else.

# Target

`{name}` at 0x{address:08X}, {row.get('size', '?')} bytes.
Current verdict: {row.get('tier', '?')}{(' - ' + note) if note else ''}

# What the original compiles to

{disassembly(address)}

# The body as it stands ({location})

```cpp
{(body or '').strip()}
```

# What the divergence usually means

{targeted_rules(note)}
{naming_section(name, body)}
# The signature is yours to change

Nothing pins it. The scaffolding does NOT declare this function, and the
comparison is over the object's CODE rather than a symbol lookup, so a
different return type, convention or parameter list still scores. Change it
when the DISASSEMBLY contradicts the current one:

- reads `[ecx+N]` with no stack access -> the receiver is `this`, so this is a
  `__thiscall` member and needs a class in your unit to say so
- `ret N` with N != 0 -> a callee-pop convention, `__stdcall` or `__thiscall`
- sets `eax` before returning where the signature says `void` -> it returns

Measured, so you do not repeat it: on 0x005E3630 rewriting the fake-thiscall
`__fastcall(void *self, void *)` as a real `__thiscall` member changed NOT ONE
BYTE. Change the signature on evidence from the disassembly, never on style.

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
