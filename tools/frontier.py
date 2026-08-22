#!/usr/bin/env -S uv run python
"""The WinMain-reachable call graph, in depth-first order, with verdicts.

WHY. The standing goal is to walk depth first from WinMain until everything
reachable is byte exact or semantically equivalent, and nothing in the tree
answers "what is reachable and not yet exact, nearest the root first". `check`
scores every body in the build's own files - claims and candidates alike -
but as a flat list over the whole tree, so it cannot say which of them is ON
THE PATH.

The edges come from the shipped bytes, not from any annotation's `calls` fact,
which is a sorted set produced by a pass that no longer runs.

WHAT "REACHABLE" MEANS HERE. Direct and tail calls only. An indirect call
through a vtable or a bound slot is a runtime fact this cannot resolve, so a
node reached ONLY that way does not appear - the frontier is a lower bound on
the work, never an upper one, and the count is printed so that is visible.

WORKED BODIES ARE MARKED, because handing out eight already-exhausted addresses
costs a whole agent. A body carrying `RULED-OUT:` notes has had someone attack
it and write down what failed; a batch of those comes back "no change, already
plateaued" eight times over, which is exactly what happened to a faction.cpp
batch picked off the raw depth order. `--fresh` drops them.
"""

from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from decomp import read
from decomp.asm import build_inputs
from decomp.asm import original_asm, shared_spans, span_refusal
from decomp.calls import call_sites


REPO_ROOT = Path(__file__).resolve().parent.parent
# HONOURS `OPENSMACX_IMAGE`, because `.opensmacx/` is gitignored and a
# worktree does not have it - an agent working in one must be able to
# point every tool back at the root copy. osmx.py has always taken it;
# these did not, and an agent had to symlink the image to work around it.
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))
ROOT = 0x0045F950  # _WinMain@16


def _blanked(text: str) -> str:
    """Comments replaced by spaces, so brace matching stays honest.

    ORDER MATTERS. Stripping comments AFTER finding the opening brace finds a
    brace inside a comment instead of the function's - a first draft of this
    reported 486 of 487 frontier bodies as stubs, including `read_rules`, which
    is 883 image instructions.
    """
    text = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)),
                  text, flags=re.S)
    return re.sub(r"//[^\n]*", lambda m: " " * len(m.group(0)), text)


_BLANK_CACHE: dict = {}


def body_text(record) -> str | None:
    """The braces-matched source body following a record's marker."""
    where = str(record.location)
    path, _, line = where.rpartition(":")
    path = Path(path)
    if path not in _BLANK_CACHE:
        try:
            _BLANK_CACHE[path] = _blanked(
                path.read_text(errors="replace")).splitlines()
        except OSError:
            _BLANK_CACHE[path] = []
    lines = _BLANK_CACHE[path]
    text = "\n".join(lines[int(line) - 1: int(line) + 600])
    start = text.find("{")
    if start < 0:
        return None
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start + 1:i]
    return None


STUB = re.compile(r"(return\s*(-?\d+|nullptr|false|true)?\s*;)?")


def is_stub(record) -> bool:
    """Empty, or a single bare `return`: a placeholder, not a transcription."""
    inner = body_text(record)
    if inner is None:
        return False
    statements = [s.strip() for s in inner.split("\n") if s.strip()]
    return len(statements) <= 1 and bool(STUB.fullmatch(" ".join(statements)))


def walk(records: list, exe: Path, root: int) -> tuple[list, int, int]:
    """Depth-first from `root`; returns (order, edges seen, unnamed edges)."""
    by_address = {}
    for record in records:
        by_address.setdefault(record.address, record)
    shared = shared_spans(records)

    order, seen, unnamed, edges = [], set(), 0, 0
    stack = [root]
    while stack:
        address = stack.pop()
        if address in seen:
            continue
        seen.add(address)
        record = by_address.get(address)
        if record is None:
            unnamed += 1
            continue
        order.append(record)
        if span_refusal(record, exe, shared) is not None:
            continue
        try:
            listing = original_asm(record, exe)
        except (ValueError, KeyError):
            continue
        targets = []
        for site in call_sites(listing):
            edges += 1
            if site.form in ("direct", "tail") and site.target is not None:
                targets.append(site.target)
        # Reversed, so `pop` takes the FIRST call the image makes: the order
        # of the walk is the order of the code.
        stack.extend(reversed(targets))
    return order, edges, unnamed


if __name__ == "__main__":
    records = read(REPO_ROOT / "src")
    order, edges, unnamed = walk(records, IMAGE, ROOT)
    built = build_inputs(REPO_ROOT / "build" / "compile_commands.json")
    pending = [r for r in order if not r.byte_exact]
    if "--all" not in sys.argv:
        # A record whose file the build does not compile cannot be MEASURED,
        # so it is not work anyone can finish today - it is work that first
        # needs a home in a translation unit.
        pending = [r for r in pending if r.path in built]
    if "--json" in sys.argv:
        print(json.dumps([{"address": r.address_hex, "name": r.name,
                           "file": str(r.path)} for r in pending], indent=2))
        sys.exit(0)
    # How many `RULED-OUT:` notes sit under each marker. The reader already
    # parses lesson lines, so this is the record's own evidence of prior work
    # rather than anything maintained by hand.
    # NO `getattr` DEFAULT. The first draft guessed the field name, and a
    # default of `()` turned the miss into "nothing is worked" - every row
    # unmarked, which reads exactly like a correct answer. Name the field and
    # let a wrong one raise.
    def ruled_out(record) -> int:
        return len(record.ruled_out)

    def levers(record) -> int:
        return len(record.levers)

    if "--fresh" in sys.argv:
        pending = [r for r in pending if not ruled_out(r)]

    # `--untouched` IS THE ONE THAT TARGETS YIELD. `--fresh` only drops bodies
    # carrying a RULED-OUT, and a body can have been worked hard and left a
    # `// LEVER:` line instead - Scroll's three `set_bevel_*` siblings each
    # carry one, and every one reads as "fresh". Picking a batch that way hands
    # an agent work a previous pass already did. Untouched means NEITHER.
    if "--untouched" in sys.argv:
        pending = [r for r in pending if not ruled_out(r) and not levers(r)]

    for depth, record in enumerate(pending):
        worked, moved = ruled_out(record), levers(record)
        mark = ""
        if worked or moved:
            parts = ([f"{worked} ruled-out"] if worked else []) + \
                    ([f"{moved} lever"] if moved else [])
            mark = "  [" + ", ".join(parts) + "]"
        print(f"{depth:4d}  {record.address_hex}  {record.path.name:24s} "
              f"{record.name}{mark}")
    if "--semantic" in sys.argv:
        stubs = [r for r in pending if is_stub(r)]
        granted = [r for r in pending if r.semantic]
        print(f"\n  {len(pending):,} reachable and not byte exact")
        print(f"    {len(pending) - len(stubs):,} carry a real transcription")
        print(f"    {len(stubs):,} are STUBS - empty, or a bare return")
        for record in stubs:
            print(f"      {record.address_hex}  {record.size or 0:,} image "
                  f"bytes   {record.name}")
        print(f"    {len(granted):,} carry a PROVED semantic claim "
              f"(same instructions, different registers)")

        # WHAT IS NOT REACHABLE AT ALL, so the goal's completion criterion can
        # be read honestly. A body whose image bytes are hand-written assembly
        # can never be byte-exact from C++, and `osmx semantic` cannot certify
        # it either - that tier requires the SAME instruction sequence, and a
        # `rep stosd` block is not the sequence any C++ compiles to. Those
        # bodies can only ever be semantically equivalent in the ordinary
        # sense, argued from the source rather than proved by the tool.
        try:
            from subprocess import run
            done = run(["uv", "run", "tools/handwritten_asm.py"],
                       cwd=str(REPO_ROOT), capture_output=True, text=True,
                       timeout=900)
            hand = {int(line.split()[0], 16)
                    for line in done.stdout.splitlines()
                    if line.startswith("  0x")}
        except Exception:                                # noqa: BLE001
            hand = set()
        blocked = [r for r in pending if r.address in hand]
        if blocked:
            print(f"    {len(blocked):,} are HAND-WRITTEN ASSEMBLY in the "
                  f"image - neither tier is reachable:")
            for record in blocked:
                print(f"      {record.address_hex}  {record.name}")
        print("\n  A transcription is EVIDENCE of equivalence, not proof. The"
              "\n  proof this tree can give is `osmx semantic`, and the"
              "\n  strongest cheap check is `call_diff`: a body calling"
              "\n  something the image does not is not equivalent, whatever"
              "\n  its similarity score says.")
        sys.exit(0)

    print(f"{len(order):,} reachable and catalogued, {len(pending):,} not yet "
          f"byte exact; {unnamed:,} direct edges name nothing in the catalogue "
          f"({edges:,} edges walked, indirect ones not followed)")
    # AND THE NUMBER THE GOAL ACTUALLY ASKS FOR. "Byte exact OR semantically
    # the same" is the bar, and a body carrying a PROVED semantic claim has
    # met it - `osmx semantic` grants only where the instruction sequence is
    # identical and the registers differ. Counting those as outstanding
    # overstates the work remaining, so both numbers are printed rather than
    # one being quietly substituted for the other.
    proved = sum(1 for record in pending if record.semantic)
    if proved:
        print(f"  {proved:,} of those carry a PROVED semantic claim, so "
              f"{len(pending) - proved:,} remain against "
              f"\"byte exact or semantically the same\"")
    print("  --all also lists reachable bodies the build does not compile")
