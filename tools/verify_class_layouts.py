#!/usr/bin/env python3
"""Prove each extracted class layout against the real one, and record which.

`class_layouts` reads data members out of a header with regexes. That is
enough to be useful and not enough to be trusted: a member it skips - a
nested typedef's type, an anonymous pad - moves every offset after it, and
the recovered body that indexes through the wrong offset still COMPILES. It
fails later, as a byte mismatch nobody traces back to here.

So every candidate is compiled against the real class:

    struct Probe { ...extracted members... };
    static_assert(sizeof(Probe) == sizeof(Buffer), "layout");

and only the ones that survive are written to the verified list the emitter
reads. Measured on the first run: 20 of 25 candidates survived, and the five
that did not (Base, Buffer, Font, ListBox, PlanWin) would each have handed
an agent a layout that is wrong in a way nothing downstream would catch.

Needs VC6 under wine, so it is a generate-and-commit step rather than
something the emitter does per unit.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match as bm  # noqa: E402
import class_layouts  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
VERIFIED = REPO_ROOT / "docs" / "recovery" / "verified-layouts.txt"


def declaring_header(src: Path = SRC) -> dict:
    """{class name: the header that defines it}."""
    home = {}
    for header in sorted(src.glob("*.h")):
        for name, _, _ in class_layouts.class_bodies(
                header.read_text(errors="ignore")):
            home.setdefault(name, header.name)
    return home


def compile_units(work: Path, units: dict, src: Path) -> str:
    """Compile every unit in one batch; return the compiler's output.

    Whether a unit succeeded is read from its `.obj`, not from the exit code:
    `cl` returns non-zero when ANY unit fails and the others still produce
    objects, so the per-unit answer is on disk.
    """
    (work / "cl.rsp").write_text(
        "/c /GR- /GX\n" + "\n".join(f"{stem}.cpp" for stem in units) + "\n")
    environment = bm.wine_environment()
    environment["INCLUDE"] += ";Z:" + str(src).replace("/", "\\")
    finished = subprocess.run(
        ["wine", str(bm.VC6_CL), "/nologo", "@cl.rsp"], cwd=work,
        env=environment, capture_output=True, text=True)
    return finished.stdout + finished.stderr


def errors_by_unit(output: str, units: dict) -> dict:
    """{class name: first compiler error}, from cl's per-file output.

    `cl` announces each translation unit by printing its file name on a line
    of its own before any diagnostic for it, which is the only thing tying an
    error to a unit when they are compiled in one batch.
    """
    found, current = {}, None
    for line in output.splitlines():
        stem = line.strip()[:-4] if line.strip().endswith(".cpp") else None
        if stem in units:
            current = units[stem]
        elif current and "error" in line and current not in found:
            found[current] = line.strip()
    return found


def verify(candidates: dict, src: Path = SRC) -> tuple:
    """(verified, rejected, unbuildable) - one translation unit per class.

    THE PROBE IS BUILT TWICE, AND THE FIRST BUILD CARRIES NO ASSERTION. A
    probe that fails to COMPILE produces no `.obj`, and so did a probe whose
    `static_assert` fired - the two were indistinguishable, and every one of
    them was reported as "the extracted layout is not the real size".

    That is not a cosmetic misattribution. `src/buffer.h` referenced an
    undeclared `Vert` for one commit, and because `graphicwin.h` and `win.h`
    include it, four unrelated classes - ButtonGroup, MenuEntry, PullDownItem,
    TutWin - stopped verifying. Their layouts were never wrong. Regenerating
    the list at that moment would have dropped all four, `supplyable()` would
    have returned False for each, and every recovered body in them would have
    gone NO_COMPILE with nothing pointing back to a header that had a typo.

    So a unit that cannot build is not a verdict about a layout, and it is
    returned separately rather than folded into `rejected`. `main` refuses to
    write the list at all while any exist. `extend_short_classes.py:24-28`
    documents this same control for itself; this tool did not have it.
    """
    home = declaring_header(src)
    work = Path(tempfile.mkdtemp())

    units, bodies = {}, {}
    for index, name in enumerate(sorted(candidates)):
        if name not in home:
            continue
        stem = f"c{index:03d}"
        bodies[stem] = (
            f'#include "stdafx.h"\n#include "{home[name]}"\n'
            f"struct Probe {{\n"
            f"{chr(10).join(class_layouts.unverified_declaration_for(name))}\n"
            f"}};\n")
        (work / f"{stem}.cpp").write_text(
            bodies[stem] + f"int probe{index}() {{ return 0; }}\n")
        units[stem] = name

    # Pass one: the probe alone. Anything that fails here is a broken header,
    # a missing type, or a member `class_layouts` mis-parsed into something
    # that will not compile - none of which is an answer about size.
    output = compile_units(work, units, src)
    reasons = errors_by_unit(output, units)
    buildable = {stem: name for stem, name in units.items()
                 if (work / f"{stem}.obj").is_file()}
    unbuildable = sorted(
        (name, reasons.get(name, "(no diagnostic captured)"))
        for stem, name in units.items() if stem not in buildable)

    # Pass two: the same probes, now asserting. Only these verdicts are about
    # the layout, because the unit is already known to build without them.
    for stem in buildable:
        (work / f"{stem}.obj").unlink()
        (work / f"{stem}.cpp").write_text(
            bodies[stem]
            + f'static_assert(sizeof(Probe) == sizeof({buildable[stem]}), '
              f'"layout");\n'
            + f"int probe_{stem}() {{ return 0; }}\n")
    if buildable:
        compile_units(work, buildable, src)

    verified = sorted(name for stem, name in buildable.items()
                      if (work / f"{stem}.obj").is_file())
    rejected = sorted(name for stem, name in buildable.items()
                      if not (work / f"{stem}.obj").is_file())
    return verified, rejected, unbuildable


def render(verified: list) -> str:
    """The exact bytes of the committed list.

    Extracted from `main` so it can be tested. `--check` compares the file
    against this character for character, so a change to the banner - a
    reworded line, a lost trailing newline - would report every checkout as
    stale without a single layout having moved.
    """
    banner = (
        "# Class layouts tools/class_layouts.py extracts AND that compile\n"
        "# to the same size as the real class. Regenerate with\n"
        "# tools/verify_class_layouts.py. A layout not listed here is not\n"
        "# handed to an agent; the emitter falls back to an opaque shell.\n")
    # An empty list is `"\n".join([]) + "\n"`, which is a stray blank line
    # rather than no lines. Nothing has ever verified zero layouts, so it has
    # never shown - but the comparison is exact, and a file that gains a blank
    # line reads as stale.
    if not verified:
        return banner
    return banner + "\n".join(verified) + "\n"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="fail if the committed list is stale")
    # Named so verify_checks_can_fail.py can point --check at a deliberately
    # damaged copy and prove this gate refuses one. A check with no such proof
    # is a check nobody has seen fail.
    parser.add_argument("--verified", type=Path, default=VERIFIED,
                        help="the list to compare against (default: the "
                             "committed one)")
    # Also for verify_checks_can_fail.py: the build control can only be shown
    # to fire by handing the tool a header that does not compile, and damaging
    # the real src/ to do it is not something a gate check may do.
    parser.add_argument("--src", type=Path, default=SRC,
                        help="the header tree to read and compile against "
                             "(default: the committed one)")
    arguments = parser.parse_args(argv)
    verified_path = arguments.verified

    # Without the compiler every probe fails to build, which reads as "no
    # layout survived" and is indistinguishable from "every layout is wrong".
    # As a CTest gate that would turn a missing toolchain into 23 phantom
    # regressions, so the absence is reported as an absence.
    reason = bm.available()
    if reason:
        print(f"SKIP: {reason}. This needs Visual C++ 6.0 (12.00.8168) under "
              f"Wine.")
        return 0

    candidates = class_layouts.pinned_layouts()
    verified, rejected, unbuildable = verify(candidates, arguments.src)

    # A probe that will not build says nothing about the layout it was meant
    # to test, so neither answer this tool can give is available: writing the
    # list would silently drop the class, and reporting the checkout stale
    # would blame the list for a broken header. Refuse both, and name the
    # header, because that is the only thing here anyone can act on.
    if unbuildable:
        print(f"{len(unbuildable)} probe(s) do not compile. This is a header "
              f"defect, NOT a wrong layout - no verdict is available for "
              f"these classes and the list has not been written:",
              file=sys.stderr)
        for name, reason in unbuildable:
            print(f"    {name}: {reason}", file=sys.stderr)
        return 1

    text = render(verified)

    if arguments.check:
        current = verified_path.read_text() if verified_path.is_file() else ""
        if current != text:
            print("verified-layouts.txt is stale; regenerate it", file=sys.stderr)
            return 1
        print(f"class layouts: {len(verified)} verified (up to date)")
        return 0

    verified_path.parent.mkdir(parents=True, exist_ok=True)
    verified_path.write_text(text)
    print(f"{len(verified)} verified, {len(rejected)} rejected -> {verified_path}")
    for name in rejected:
        print(f"    rejected: {name} (extracted layout is not the real size)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
