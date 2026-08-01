#!/usr/bin/env python3
"""Fail when src/OpenSMACX.def stops exporting a name it used to export.

The staged `terranx_hybrid.exe` carries a frozen import table naming 462
symbols from OpenSMACX.dll. Renaming or removing an alias therefore breaks the
game even when the old name matched no IDB name and nothing in the repository
linked against it. On 2026-08-01 correcting `?bitmask@@YAXIPAI0@Z` to the
signed decoration did exactly that, and `stage-hybrid-game` failed with

    OpenSMACX.dll does not export 1 imported symbols;
    first missing symbol: b'?bitmask@@YAXIPAI0@Z'

That check exists (tools/opensmac_patcher.py) and it caught this - but only
after a build of the DLL, ImportAdder under Wine, and the assembled hybrid
image. `ctest` never sees it, so all 62 tests passed against a DLL the game
could no longer load, and the commit shipped saying so.

This is the same rule enforced in seconds, from git, with nothing built:
compare the exported names in the working tree against those at a base
revision and refuse any that disappeared. Corrections are still possible -
alias the old and the new decoration to the same symbol, which is additive.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DEF = REPO_ROOT / "src" / "OpenSMACX.def"
EXPORT_RE = re.compile(r'^\s*"([^"]+)"\s*=', re.MULTILINE)


def exported_names(text):
    return set(EXPORT_RE.findall(text))


def names_at(revision, path, repo_root):
    relative = Path(path).resolve().relative_to(Path(repo_root).resolve())
    result = subprocess.run(
        ["git", "-C", str(repo_root), "show", f"{revision}:{relative.as_posix()}"],
        capture_output=True, text=True)
    if result.returncode != 0:
        return None
    return exported_names(result.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--def-file", type=Path, default=DEFAULT_DEF)
    parser.add_argument("--repo-root", type=Path, default=REPO_ROOT)
    parser.add_argument("--base", default="HEAD",
                        help="revision to compare against (default HEAD)")
    arguments = parser.parse_args()

    if not arguments.def_file.is_file():
        print(f"def-append-only: {arguments.def_file} is absent, so this check "
              "verified NOTHING", file=sys.stderr)
        return 1

    current = exported_names(arguments.def_file.read_text(encoding="utf-8"))
    if not current:
        print(f"def-append-only: parsed 0 exports out of "
              f"{arguments.def_file}. Either the format changed or the parser "
              "is broken; refusing to compare an empty set against anything.",
              file=sys.stderr)
        return 1

    base = names_at(arguments.base, arguments.def_file, arguments.repo_root)
    if base is None:
        print(f"def-append-only: cannot read {arguments.def_file.name} at "
              f"{arguments.base}, so no comparison was made. This check "
              "verified NOTHING.", file=sys.stderr)
        return 1

    removed = sorted(base - current)
    if removed:
        print(f"def-append-only: {len(removed)} export(s) present at "
              f"{arguments.base} are gone. The staged hybrid imports by name "
              "and its import\ntable is frozen, so a removed or renamed alias "
              "breaks the game - and every\nctest can still pass, because "
              "staging runs before the tests.\nTo correct a decoration, alias "
              "BOTH spellings to the same symbol.", file=sys.stderr)
        for name in removed:
            print(f"    {name}", file=sys.stderr)
        return 1

    added = len(current) - len(base & current)
    print(f"def-append-only: {len(current)} exports, none removed since "
          f"{arguments.base}" + (f", {added} added" if added else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
