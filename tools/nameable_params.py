#!/usr/bin/env python3
"""Split the scaffold-name debt into what can be named NOW and what cannot.

`class_debt`'s "scaffold name" shape counts every `aN` identifier, which
makes it look like one big naming backlog. It is not. Measured 2026-08-26:

    409   in pending_bodies.cpp itself - a forwarder's parameters
    289   in headers, on a method PENDING_BODY still stands in for
    482   in headers, on a method whose body IS recovered

The first two are 59% of the total and are NOT work: a placeholder's
parameters cannot be named from evidence that does not exist yet, and
naming them would decorate a body nobody has recovered. They fall out on
their own when the body lands. Only the third group is actionable, and this
prints it so a pass can start there instead of at the top of a census.

    uv run tools/nameable_params.py            # the actionable ones, by file
    uv run tools/nameable_params.py --summary  # just the three totals
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from class_debt import SCAFFOLD_NAME, code_lines  # noqa: E402

PENDING = Path("src/pending_bodies.cpp")


def pending_method_names(path: Path = PENDING) -> set[str]:
    if not path.exists():
        return set()
    text = path.read_text(errors="replace")
    return {m for _cls, m in re.findall(r"\b(\w+)::(\w+)\s*\(", text)}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=Path("src"))
    ap.add_argument("--summary", action="store_true")
    args = ap.parse_args()

    pending = pending_method_names()
    in_pending = blocked = 0
    actionable: collections.Counter = collections.Counter()
    lines_by_file: dict[str, list[str]] = collections.defaultdict(list)

    for line in code_lines(PENDING) if PENDING.exists() else []:
        in_pending += len(SCAFFOLD_NAME.findall(line))

    for header in sorted(args.root.glob("*.h")):
        for line in code_lines(header):
            found = SCAFFOLD_NAME.findall(line)
            if not found:
                continue
            hit = re.search(r"\b(\w+)\s*\(", line)
            if hit and hit.group(1) in pending:
                blocked += len(found)
            else:
                actionable[header.name] += len(found)
                lines_by_file[header.name].append(line.strip()[:88])

    if not args.summary:
        for name, n in actionable.most_common():
            print(f"\n{name}  ({n})")
            for line in lines_by_file[name][:6]:
                print(f"    {line}")
            if len(lines_by_file[name]) > 6:
                print(f"    ... {len(lines_by_file[name]) - 6} more")
    print(f"\n{in_pending:5d}  in pending_bodies.cpp itself (forwarder parameters)")
    print(f"{blocked:5d}  in headers, on a method PENDING_BODY still stands in for")
    print(f"{sum(actionable.values()):5d}  ACTIONABLE - the body is recovered, so evidence exists")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
