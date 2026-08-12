#!/usr/bin/env python3
"""Every tool has an entry point, and every entry point names a tool that exists.

`tools/` grew to 145 production scripts and nobody could say which of them the
project still runs. The obvious audit - "grep CMakeLists" - is the one that
gets it wrong, and it got it wrong here first: rooting reachability at
`CMakeLists.txt` alone reported `mizuchi_context.py` and `emit_asm_dumps.py` as
dead, and both are invoked by `mizuchi.yaml`, an entry point in a language
CMake has never heard of. A root set that is a hand-written list of one is the
same defect as any other hand-maintained list.

So the roots are EVERY file that could start a tool: the build, the agent
harness and its integrator, and the documentation - because a tool a human runs
by hand is reachable exactly when something tells a human to run it. That is
not a loophole, it is the rule: keeping a tool means saying where it is used.

Two directions, and both are failures:

  stale       a doc or config names `tools/x.py` and there is no such file.
              Always a defect: the instruction cannot be followed.
  unreachable a production tool no entry point names and no reachable tool
              imports. Either it is finished and should go, or somebody
              relies on it and no reader could discover that.

Tests are not roots and are not checked. A test exists to watch a tool; a tool
that exists only to be tested is unreachable, which is the answer this wants.
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS = REPO_ROOT / "tools"

# `tools/x.py` anywhere, and the bare `x.py` spelling the prose sometimes uses
# once the directory is established.
QUALIFIED = re.compile(r"tools/([A-Za-z0-9_][A-Za-z0-9_.\-]*\.py)")
BARE = re.compile(r"`([a-z0-9_]+\.py)`")
IMPORT = re.compile(r"^\s*(?:import|from)\s+([a-z_][a-z0-9_]*)", re.M)


def entry_points(root: Path = REPO_ROOT) -> list:
    """Everything that can start a tool. Not a list of names - a list of KINDS.

    A new tool runner drops in as a new file of one of these kinds and is
    picked up; a new KIND has to be added here, which is the one hand-edit this
    cannot avoid and the one a reviewer would notice.
    """
    found = [root / "CMakeLists.txt", root / "AGENTS.md", root / "CLAUDE.md"]
    found += sorted(root.glob("*.yaml")) + sorted(root.glob("*.yml"))
    found += sorted(root.glob("*.mjs")) + sorted(root.glob("*.js"))
    found += sorted(root.glob("*.md")) + sorted((root / "docs").glob("*.md"))
    found += sorted((root / "docs" / "recovery").glob("*.md"))
    return [path for path in found if path.is_file()]


def named(paths: list, tools: Path = TOOLS) -> set:
    """Every tools/ file the entry points name.

    `tools` is a PARAMETER, not the module constant. Resolving the bare
    backticked spelling against the real repository while `--root` pointed
    somewhere else made the check answer about the wrong tree - which is the
    same wrong-directory mistake that keeps producing false findings here.
    """
    out = set()
    for path in paths:
        text = path.read_text(errors="ignore")
        out.update(QUALIFIED.findall(text))
        out.update(name for name in BARE.findall(text)
                   if (tools / name).is_file())
    return out


def graph(tools: Path = TOOLS) -> dict:
    """{stem: {stems it imports}} over the tool directory."""
    stems = {path.stem for path in tools.glob("*.py")}
    edges = collections.defaultdict(set)
    for path in tools.glob("*.py"):
        for found in IMPORT.finditer(path.read_text(errors="ignore")):
            if found.group(1) in stems and found.group(1) != path.stem:
                edges[path.stem].add(found.group(1))
    return edges


def audit(root: Path = REPO_ROOT) -> tuple:
    """(stale names, unreachable tool stems, how many were reached)."""
    tools = root / "tools"
    present = {path.name for path in tools.glob("*.py")}
    production = {path.stem for path in tools.glob("*.py")
                  if not path.name.startswith("test_")}
    points = entry_points(root)
    mentioned = named(points, tools)
    stale = sorted(mentioned - present)

    edges = graph(tools)
    seen, stack = set(), [name[:-3] for name in mentioned & present]
    while stack:
        stem = stack.pop()
        if stem in seen:
            continue
        seen.add(stem)
        stack.extend(edges.get(stem, ()))
    return stale, sorted(production - seen), len(seen & production)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    args = parser.parse_args(argv)

    stale, unreachable, reached = audit(args.root)
    print(f"tool reachability: {reached} reached from "
          f"{len(entry_points(args.root))} entry points")
    for name in stale:
        print(f"  STALE       an entry point names tools/{name}, which does "
              f"not exist")
    for stem in unreachable:
        print(f"  UNREACHABLE tools/{stem}.py - no entry point names it and "
              f"nothing reachable imports it")
    if stale or unreachable:
        print(f"FAIL: {len(stale)} stale reference(s), "
              f"{len(unreachable)} unreachable tool(s)")
        return 1
    if not reached:
        print("FAIL: nothing was reached, so this run proves nothing")
        return 1
    print("OK: every tool has an entry point, and every entry point resolves")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
