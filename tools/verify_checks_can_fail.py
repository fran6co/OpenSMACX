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

import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent


def run_gate() -> tuple[int, str]:
    done = subprocess.run(
        [sys.executable, str(REPO / "tools" / "osmx.py"), "check"],
        cwd=REPO, capture_output=True, text=True)
    return done.returncode, done.stdout + done.stderr


CASES = [
    ("compiler_work slack",
     "FAILED: the tree does more of the compiler's work",
     lambda: _patch("tools/compiler_work.py",
                    '"artifact files (recovered/)": 1395,',
                    '"artifact files (recovered/)": 1390,')),
    ("marker_symbols floor",
     "FAILED: a marker names a symbol the build does not emit",
     lambda: _append("src/palette.cpp",
                     "// symbol    ?totally_bogus_harness@@YAXXZ\n",
                     after="// ORIGINAL: 0x005FE460 ?set@Palette@@QAEHXZ")),
    ("stale_references",
     "FAILED: an instruction names something that is not there",
     lambda: _append("README.md",
                     "\nRun `uv run tools/harness_not_a_tool_9f3a.py` now.\n")),
    ("address_index duplicates",
     "ADDRESS INDEX GREW: duplicate above their floors",
     lambda: _prepend("src/unrecovered/005fed40.cpp",
                      "// ORIGINAL: 0x005FE460 ?set@Palette@@QAEHXZ"
                      " 0x005FE460-0x005FE4EB BYTE_EXACT\n")),
    ("unresolved guard",
     "REGRESSED 0x005FE460 claims BYTE_EXACT, measured UNRESOLVED",
     lambda: _append("src/palette.cpp",
                     "// symbol    ?second_bogus_harness@@YAXXZ\n",
                     after="// ORIGINAL: 0x005FE460 ?set@Palette@@QAEHXZ")),
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
    failures = []
    for name, wording, damage in CASES:
        _SNAPSHOTS.clear()
        try:
            damage()
            code, out = run_gate()
            ok = code == 1 and wording in out
            print(f"  {'PASS' if ok else 'FAIL'} {name}: exit {code}, "
                  f"{wording[:40]!r} {'found' if wording in out else 'MISSING'}")
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

    print(f"\n{len(CASES) + 1} case(s), "
          f"{len(failures)} failure(s){': ' + ', '.join(failures) if failures else ''}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
