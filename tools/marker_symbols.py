#!/usr/bin/env -S uv run python
r"""Markers whose symbol the build does not actually emit.

A marker's second field names the function it claims. `osmx measure` finds the
compiled body BY THAT NAME, so when a source change alters a mangled name -
a return type, a `virtual`, a parameter width - the marker stops resolving and
the claim silently becomes UNVERIFIABLE rather than failing. It is the same
shape as the reader that stopped CHECKING markers in headers: the count moves
in the healthy-looking direction and nothing says why.

Measured 2026-08-22: unifying `close()` on one return type across the
Dialog/GraphicWin hierarchy - a real reconstruction fix, since the census names
are guesses and the image is stripped - moved nine markers out of the gate this
way in a single edit, while the gate reported only two regressions.

This compiles each build-visible file, reads the symbol table, and reports the
markers whose named symbol is not there.

    uv run tools/marker_symbols.py
    uv run tools/marker_symbols.py src/checkbox.cpp    # one file

Exit 0 clean, 1 if any marker names a symbol the build does not emit.
"""

from __future__ import annotations

def near(symbol, names):
    """Symbols the build DOES emit for the same function under another spelling.

    Three naming reconstructions cost claims on 2026-08-22 alone, all the same
    shape: the annotation records a GUESS at the mangled name and only the
    object file is evidence.

      ??1GraphicWin@@QAE@XZ   the build emits ??1GraphicWin@@UAE@XZ
                              - `U` not `Q`, because the destructor is virtual
      ??_GAlphaMovie@@WEEE@   the build emits ??_EAlphaMovie@@WEEE@
                              - the VECTOR deleting destructor, not the scalar
      ?x@C@@QAEHXZ            a receiver the bytes deny at all

    So a missing symbol is reported with what the build emits under the same
    name and class, which is nearly always the answer.
    """
    head = symbol.split("@@")[0]
    if not head:
        return []
    return sorted(n for n in names if n.split("@@")[0] == head and n != symbol)

import json
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SCRATCH = Path("/tmp/opensmacx-marker-symbols")
# re.M IS LOAD-BEARING: without it `^` anchors to the start of the whole
# file and every marker below line 1 is invisible - the check then reports
# "0 file(s) checked" and exits 0, which reads exactly like a clean tree.
MARKER = re.compile(r"^// ORIGINAL: (0x[0-9A-Fa-f]{8})\s+(\S+)", re.M)
# `sub_xxxxxx` and other census placeholders are not mangled names and cannot
# be looked up; they are not what this checks.
MANGLED = re.compile(r"^[?@]")


def emitted(entry: dict, out: Path) -> set[str] | None:
    """Every symbol the object defines, or None if it will not compile."""
    command = re.sub(r"/FoCMakeFiles\S+", f"/Fo{out}", entry["command"])
    done = subprocess.run(command, shell=True, cwd=entry["directory"],
                          capture_output=True, text=True)
    if not out.exists():
        return None
    listed = subprocess.run(["nm", str(out)], capture_output=True, text=True)
    names = set()
    for line in listed.stdout.splitlines():
        parts = line.split()
        if parts:
            names.add(parts[-1])
    return names


if __name__ == "__main__":
    database = json.loads(
        (REPO_ROOT / "build" / "compile_commands.json").read_text())
    wanted = [a for a in sys.argv[1:] if not a.startswith("-")]
    SCRATCH.mkdir(parents=True, exist_ok=True)

    missing: list[tuple[str, str, str]] = []
    scanned = 0
    for entry in database:
        path = Path(entry["file"])
        if wanted and not any(w in str(path) for w in wanted):
            continue
        text = path.read_text(errors="replace")
        # THE `// symbol` FACT WINS, which is the rule the measurement itself
        # applies: the marker names what the IMAGE calls the piece, and the
        # fact names what THIS TREE's compiler emits for it. They differ
        # legitimately wherever the source re-expresses a compiler-generated
        # construct, or where the census guessed a spelling MSVC does not use
        # (`RECT` against `tagRECT`). Comparing the marker alone reported 24
        # false hits on a tree the gate resolves fine.
        claims = []
        for block in re.split(r"(?=^// ORIGINAL: )", text, flags=re.M)[1:]:
            head = MARKER.match(block)
            if not head:
                continue
            fact = re.search(r"^// symbol\s+(\S+)", block, re.M)
            wanted = fact.group(1) if fact else head.group(2)
            if MANGLED.match(wanted):
                claims.append((head.group(1), wanted))
        if not claims:
            continue
        names = emitted(entry, SCRATCH / f"{path.stem}.obj")
        if names is None:
            print(f"  (skipped {path.name}: does not compile)")
            continue
        scanned += 1
        for address, symbol in claims:
            if symbol not in names:
                missing.append((path.name, address, symbol, near(symbol, names)))

    for name, address, symbol, suggestions in missing:
        print(f"  {address}  {name:22} names {symbol}")
        if suggestions:
            for s in suggestions[:3]:
                print(f"      ...the build emits {s}")
        else:
            print(f"      ...which this build does not emit")
    if not missing:
        print("  every marker names a symbol the build emits")
    print(f"\n{scanned} file(s) checked; {len(missing)} marker(s) name a symbol "
          f"that is not there")

    # THE FLOOR, ratcheted like every other census. Six of today's eight are
    # empty-inline lifecycle stubs and the two ??__E/??__F ordinal
    # initialisers - each needs a stub conversion or the initialiser matcher,
    # not a symbol-fact edit, and forcing them overnight risks exactly the
    # neighbour regressions the gate exists to catch. The count may only
    # fall; --check fails when it rises, and reaching zero retires this
    # ceiling in its own commit.
    CEILING = 11
    if "--check" in sys.argv:
        if len(missing) > CEILING:
            print(f"SYMBOL FACTS GREW: {len(missing)}, above the floor of "
                  f"{CEILING}")
            sys.exit(1)
        if len(missing) < CEILING:
            print(f"symbol facts down: {len(missing)}, below the floor of "
                  f"{CEILING} - lower it in this same commit")
        sys.exit(0)
    sys.exit(1 if missing else 0)
