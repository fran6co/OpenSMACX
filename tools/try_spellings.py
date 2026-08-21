#!/usr/bin/env -S uv run python
"""Score several source spellings of ONE body against the shipped bytes.

WHY. A body that reproduces the image's semantics but not its bytes is
usually one source shape away, and finding which shape is a search: write a
candidate, build, measure, put the file back. Doing that by hand is slow and,
worse, leaves the tree edited if anything throws - which is how a half-applied
experiment gets committed.

USAGE. Candidates come from a JSON file: `{"name": "<replacement body>"}`,
where the body is everything BETWEEN the braces of the function.

    uv run tools/try_spellings.py src/buffer.cpp 0x005E1A80 candidates.json

Prints one row per candidate, best first, and RESTORES THE FILE either way.
The winner is not applied: pick it deliberately, because a tie on verdict is
not a tie on whether the source says something true.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TIERS = ("BYTE_EXACT", "SHAPE_EXACT", "MNEMONIC_ONLY", "MISMATCH", "")


def _body_span(text: str, address: str) -> tuple[int, int]:
    """The braces of the body the `ORIGINAL: <address>` marker introduces."""
    marker = re.search(rf"//\s*ORIGINAL:\s*{address}\b", text, re.I)
    if marker is None:
        raise SystemExit(f"{address}: no ORIGINAL marker in the file")
    opening = text.index("{", text.index("\n", marker.end()))
    depth, index = 1, opening + 1
    while index < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[index], 0)
        index += 1
    return opening + 1, index - 1


def measure(address: str) -> tuple[str, str]:
    """(verdict, detail) for one address, or ("", reason) if it cannot say."""
    done = subprocess.run(
        ["uv", "run", "tools/osmx.py", "measure", address],
        cwd=REPO_ROOT, capture_output=True, text=True)
    lines = [line.strip() for line in done.stdout.splitlines() if line.strip()]
    verdict, agreeing = "", ""
    for line in lines:
        for tier in TIERS[:-1]:
            if line.startswith(tier) and not verdict:
                verdict = tier
        # The TIER alone cannot rank two MISMATCHes, and the search is
        # usually a walk through them: 69 of 131 agreeing is progress over 61
        # even though both print the same word.
        if line.startswith("agreeing") or line.startswith("bytes"):
            agreeing += ("  " if agreeing else "") + " ".join(line.split())
    if verdict:
        return verdict, f"{verdict:14s} {agreeing}"
    return "", (lines[0] if lines else done.stderr.strip())[:110]


if __name__ == "__main__":
    source = Path(sys.argv[1])
    address = sys.argv[2]
    candidates = json.loads(Path(sys.argv[3]).read_text())
    original = source.read_text()
    start, end = _body_span(original, address)
    results = []
    try:
        baseline = measure(address)
        print(f"  {'(as committed)':28s} {baseline[1]}")
        # A POSITIVE CONTROL, first, because every candidate scoring exactly
        # what the committed body scores is the signature of a span that was
        # never replaced - and it reads as "nothing helped".
        source.write_text(original[:start] + "\n    return;\n"
                          + original[end:])
        control = measure(address)
        if control[1] == baseline[1]:
            raise SystemExit(
                f"{address}: emptying the body changed nothing, so the span "
                f"this tool replaces is not the body being measured")
        print(f"  {'(control: empty body)':28s} {control[1]}")
        for name, replacement in candidates.items():
            source.write_text(original[:start] + replacement + original[end:])
            verdict, detail = measure(address)
            results.append((TIERS.index(verdict), name, detail))
            print(f"  {name:28s} {detail}")
    finally:
        source.write_text(original)
    results.sort()
    print(f"{len(results)} candidate(s); the file is back as it was")
    if results and results[0][0] < TIERS.index(baseline[0] or ""):
        print(f"best: {results[0][1]}")
    else:
        print("none beat what is committed")
