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

- Submit the COMPLETE definition, same signature as the body above, nothing
  else. The scaffolding supplies every declaration.
- No `__asm`, no `_emit`. The verifier refuses them before compiling: pasting
  the original's instructions proves nothing about the source, and
  AGENTS.md:5 bars copied machine code from distributable builds.
- Readability counts as well as byte-exactness. Keep the existing shape and
  naming where it is already right; change only what the diff demands.
- Do not edit anything under /home/fran6co/code/OpenSMACX. Write only to /tmp.

# Report

The final body in one ```cpp block, the verdict line the tool printed, and one
sentence on what the source-form change was. If you cannot reach BYTE_EXACT,
say what you ruled out - that is a result, not a failure.
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
