#!/usr/bin/env -S uv run python
"""Damage every gate sub-check on purpose; require each one to go red.

A check that cannot fail is decoration. This harness existed, was deleted as
an orphan, and its absence is exactly how three defects survived a green
gate in the 2026-08-23 audit: ceiling slack passed as advice, eight stale
symbol facts wired to nothing, two scaffold files nobody ratcheted. So it is
restored as the rule it always stated: every sub-check in `osmx check` gets
a real damage applied to the REAL tree, the FULL gate is run unpiped, and
each case asserts both the exit code and the verdict wording - because an
exit 1 with the wrong message means some OTHER check caught your damage and
yours is still decorative.

    uv run tools/verify_checks_can_fail.py          # ~10 minutes, 7 gates

Every case restores what it touched, and the harness ends on a clean
baseline run: if the last gate is not exit 3, the harness broke the tree,
which is itself a finding.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent


def run_gate() -> tuple[int, str]:
    done = subprocess.run(
        [sys.executable, str(REPO / "tools" / "osmx.py"), "check"],
        cwd=REPO, capture_output=True, text=True)
    return done.returncode, done.stdout + done.stderr


# DAMAGE IS DERIVED FROM THE LIVE TREE, NEVER PINNED. The first version of
# this list hard-coded a ceiling value and an artifact path, and both went
# stale WITHIN A DAY: a legitimate ceiling lowering (1395 -> 1391) made the
# slack case's _patch assert, and the Palette homing deleted the artifact
# file the duplicate case wrote into. A positive control that breaks on every
# legitimate improvement is a control nobody runs - the hand-maintained-list
# defect, inside the tool that exists to catch defects.


def _slack_damage() -> None:
    """Raise one scaffold ceiling above its live count, whatever it is."""
    base = _snap("tools/compiler_work.py")
    found = re.search(r'("artifact files \(recovered/\)": )(\d+)', base)
    assert found, "scaffold ceiling line missing from compiler_work.py"
    _patch("tools/compiler_work.py", found.group(0),
           f"{found.group(1)}{int(found.group(2)) + 5}")


def _first_claim(rel: str) -> str:
    """The file's first BYTE_EXACT marker line, current as of this run."""
    found = re.search(r"^// ORIGINAL: 0x\w+ \S+ \S+ BYTE_EXACT$",
                      _snap(rel), re.M)
    assert found, f"no BYTE_EXACT marker in {rel}"
    return found.group(0)


def _floor_decrement() -> None:
    """Lower the persisted floor by one, whatever it currently says."""
    found = re.search(r"claims (\d+)", _snap("src/CLAIM_FLOOR"))
    assert found, "no claims line in src/CLAIM_FLOOR"
    _patch("src/CLAIM_FLOOR", found.group(0),
           f"claims {int(found.group(1)) - 1}")


def _any_artifact() -> str:
    """A live artifact file to damage, whichever exists today."""
    for candidate in sorted((REPO / "src" / "unrecovered").glob("*.cpp")):
        return str(candidate.relative_to(REPO))
    raise AssertionError("src/unrecovered/ is empty - retire this case "
                         "with the directory")


def _origin_blind() -> None:
    """Make the writer forget every lesson's provenance, whatever it holds.

    CODE-SIDE DAMAGE, DELIBERATELY. After the fix, annotation identity is a
    STRUCTURAL property: no edit under `src/` can break invariant I, because
    the writer no longer regenerates what it did not carry. The honest
    control for a structurally-enforced invariant is to break the structure,
    the same idiom `_slack_damage` uses on a ceiling. `annotation marker
    spelling` below is what keeps this honest by proving the check still
    reads the tree.
    """
    base = _snap("decomp/writer.py")
    found = re.search(r"    if record\.origin is None:\n        return False",
                      base)
    assert found, "writer.py no longer guards on record.origin"
    _patch("decomp/writer.py", found.group(0),
           "    if True:\n        return False")


def _marker_respelling() -> None:
    """Put a second space in a live marker line, wherever one is.

    TREE-SIDE, and it trips nothing else on purpose: same address, same
    name, same spans, same claim count - so `address_index`, `marker_symbols`
    and the claim floor all stay quiet, and only the canonical spelling of
    the marker notices.
    """
    marker = _first_claim("src/palette.cpp")
    _patch("src/palette.cpp", marker,
           marker.replace("// ORIGINAL: ", "// ORIGINAL:  ", 1))


CASES = [
    ("compiler_work slack",
     "FAILED: the tree does more of the compiler's work",
     _slack_damage),
    ("compiler_work growth",
     "FAILED: the tree does more of the compiler's work",
     lambda: _append("src/palette.h",
                     "static int *const g_00dead01 = (int *)0x00DEAD01;\n")),
    # THE CENSUS MUST NOT COUNT PROSE. compiler_work skipped only `//` lines,
    # so a shape named inside a `/* */` block counted as a live site: on
    # 2026-08-25 "anonymous fixed-address global" read 184 where the truth was
    # 103, because Purpose blocks explaining a homing mentioned the bindings
    # they had just removed. This case is the fix stated as a control - the
    # same text that fails as CODE (the growth case above) must be invisible
    # as a COMMENT, so a regression to `//`-only skipping turns this green
    # case red.
    ("compiler_work ignores block comments",
     None,
     lambda: _append("src/palette.h",
                     "/*\n g_00dead02 = (int *)0x00DEAD02;\n*/\n")),
    # DROPPED INITIAL VALUES. A global written `int X;  // 0xADDR` is zero at
    # run time; if the image's .data carries a value there, the recovered
    # program is wrong and NOTHING else sees it - both spellings compile the
    # same load, so the byte ratchet is blind. WinFillColour shipped as 0
    # against the image's 9. This asserts the compare can still fail: strip
    # the initialiser off a value restored from the image and the census must
    # notice, which it cannot do if it ever stops reading the image's bytes.
    ("dropped initial value",
     "DROPPED INITIAL VALUES GREW",
     lambda: _patch("src/sprite.cpp", "SpriteDrawOriginX = 1;",
                    "SpriteDrawOriginX;")),

    # THE RULER ITSELF. P0.8 pinned the image's sha256 and wired the compare
    # into `check`, but shipped no control - and the pin also turned fifteen
    # of decomp/tests red, unnoticed, because nothing runs pytest. This
    # asserts the compare can still fail now that a missing image no longer
    # trips it.
    ("image pin",
     "IMAGE IS NOT THE PINNED ONE",
     lambda: _patch("src/IMAGE_PIN", _snap("src/IMAGE_PIN").split("sha256 ")[1][:8],
                    "deadbeef")),
    ("marker_symbols floor",
     "FAILED: a marker names a symbol the build does not emit",
     lambda: _append("src/palette.cpp",
                     "// symbol    ?totally_bogus_harness@@YAXXZ\n",
                     after=_first_claim("src/palette.cpp"))),
    ("stale_references",
     "FAILED: an instruction names something that is not there",
     lambda: _append("README.md",
                     "\nRun `uv run tools/harness_not_a_tool_9f3a.py` now.\n")),
    ("address_index duplicates",
     "ADDRESS INDEX GREW: duplicate above their floors",
     lambda: _prepend(_any_artifact(),
                      _first_claim("src/palette.cpp") + "\n")),
    ("class_debt growth",
     "SEMANTIC DEBT GREW",
     lambda: _append("src/palette.h", "  void UNK99(int);\n")),
    ("claim floor fell",
     # The one corruption no verdict can catch: the marker stops being a
     # marker, the claim ceases to exist, and only the ledger notices.
     "CLAIMS FELL",
     lambda: _patch("src/palette.cpp",
                    _first_claim("src/palette.cpp"),
                    _first_claim("src/palette.cpp")
                    .replace("// ORIGINAL:", "// ORIGINAl:"))),
    ("claim floor exceeded",
     # Equivalent to a BYTE_EXACT token nothing measured: the tree holds one
     # more claim than `osmx record` ever wrote down.
     "CLAIMS ABOVE THE FLOOR",
     lambda: _floor_decrement()),
    ("annotation identity",
     "FAILED: an annotation does not survive being rewritten",
     _origin_blind),
    ("annotation marker spelling",
     "FAILED: an annotation does not survive being rewritten",
     _marker_respelling),
    ("unresolved guard",
     # Evaluated AFTER the damage lands, so the asserted wording carries the
     # address of whatever claim the damage actually hit - a pinned address
     # here would rot exactly like the pinned ceiling did.
     lambda: ("REGRESSED " + _first_claim("src/palette.cpp").split()[2]
              + " claims BYTE_EXACT, measured UNRESOLVED"),
     lambda: _append("src/palette.cpp",
                     "// symbol    ?second_bogus_harness@@YAXXZ\n",
                     after=_first_claim("src/palette.cpp"))),
]


_SNAPSHOTS: dict = {}


def _read(rel: str) -> str:
    return (REPO / rel).read_text(errors="replace")


def _snap(rel: str) -> str:
    """The pristine text, captured before this run's first touch."""
    if rel not in _SNAPSHOTS:
        _SNAPSHOTS[rel] = _read(rel)
    return _SNAPSHOTS[rel]


def _write(rel: str, text: str) -> None:
    (REPO / rel).write_text(text)


def _patch(rel: str, old: str, new: str) -> None:
    base = _snap(rel)
    assert old in base, f"damage target missing in {rel}: {old[:40]}"
    _write(rel, base.replace(old, new))


def _append(rel: str, snippet: str, after: str | None = None) -> None:
    base = _snap(rel)
    if after is not None:
        i = base.index(after)
        j = base.index("\n", i) + 1
        _write(rel, base[:j] + snippet + base[j:])
    else:
        _write(rel, base + snippet)


def _prepend(rel: str, snippet: str) -> None:
    _write(rel, snippet + _snap(rel))


def restore_all() -> None:
    for rel, text in _SNAPSHOTS.items():
        _write(rel, text)
    _SNAPSHOTS.clear()


def main() -> int:
    # Name cases on the command line to run only those - one new damage case
    # should not cost six full gate runs. The baseline always runs: a
    # filtered run that skips it could leave the tree damaged and call
    # itself green.
    wanted = set(sys.argv[1:])
    selected = [case for case in CASES if not wanted or case[0] in wanted]
    unknown = wanted - {case[0] for case in CASES}
    if unknown:
        print(f"unknown case(s): {', '.join(sorted(unknown))}")
        print(f"have: {', '.join(case[0] for case in CASES)}")
        return 2

    failures = []
    for name, wording, damage in selected:
        _SNAPSHOTS.clear()
        try:
            damage()
            expected = wording() if callable(wording) else wording
            code, out = run_gate()
            if expected is None:
                # A MUST-STAY-GREEN case: the damage is something the check
                # is required to IGNORE. Without this, every control here
                # asserts a check can fail and none asserts it can decline
                # to - which is how a census that counts prose looks healthy.
                ok = "COMPILER WORK GREW" not in out
                print(f"  {'PASS' if ok else 'FAIL'} {name}: exit {code}, "
                      f"expected no growth, "
                      f"{'none reported' if ok else 'GREW'}")
            else:
                ok = code == 1 and expected in out
                print(f"  {'PASS' if ok else 'FAIL'} {name}: exit {code}, "
                      f"{expected[:40]!r} "
                      f"{'found' if expected in out else 'MISSING'}")
            if not ok:
                failures.append(name)
        except Exception as exc:                       # noqa: BLE001
            failures.append(name)
            print(f"  FAIL {name}: damage raised {exc}")
        finally:
            restore_all()

    code, out = run_gate()
    baseline_ok = code in (0, 3)
    print(f"  {'PASS' if baseline_ok else 'FAIL'} baseline: exit {code}")
    if not baseline_ok:
        failures.append("baseline")

    print(f"\n{len(selected) + 1} case(s), "
          f"{len(failures)} failure(s){': ' + ', '.join(failures) if failures else ''}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
