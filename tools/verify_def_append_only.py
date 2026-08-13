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
compare the exported names in the working tree against every set this file has
ever held, and refuse any that disappeared. Corrections are still possible -
alias the old and the new decoration to the same symbol, which is additive.

WHY THE WHOLE HISTORY, AND NOT `--base HEAD`, WHICH IS WHAT THIS DEFAULTED TO
UNTIL 2026-08-13. CMakeLists registers this check with no arguments at all, so
it ran against a base of HEAD - and on a clean tree `HEAD:src/OpenSMACX.def` is
byte-identical to the working copy, so `base - current` was empty by
construction and the comparison could not fail. It was a pre-commit diff: real
while you are editing, silent in CI.

Worse for the invariant it names, a base of HEAD forgets. Once a removal is
committed it becomes the new baseline and the check goes quiet about it
permanently, which is precisely the state the frozen import table cannot
tolerate. Measured before the fix, on a throwaway history that drops
`"?init@Heap@@QAEHI@Z"` in a commit and is then left clean:

    def-append-only: 11 exports, none removed since HEAD        (exit 0)

"No export ever published may disappear" is a property of the whole history, so
the whole history is what is read - the UNION of every set the file has held,
not the set at any single revision. An alias added in the middle of history and
dropped later is invisible to every single-revision comparison, including this
one's old default.

COST, because a gate nobody waits for is a gate somebody deletes. The walk is
two `git cat-file` batches for the entire history rather than one `git show`
per revision: 28 revisions in 7 ms instead of 35, and the process count stays
flat no matter how long the history gets.

WHAT STILL CANNOT FAIL HERE. `def-append-only` is listed in the EXEMPT set of
tools/verify_checks_can_fail.py, so no damage case exercises it; the proof that
this refuses lives only in the test file beside this one, which now runs the
no-flags invocation the gate actually uses rather than only the flagged one.
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


def relative_to_repo(path, repo_root):
    """The git-visible path of `path`, or None when it lies outside the repo.

    A def file outside the repository is not a comparison anyone can make; it
    returns None so every caller can refuse, rather than raising ValueError out
    of Path.relative_to - a traceback is not a refusal, and the gate's own
    auditor (NOT_A_REFUSAL in verify_checks_can_fail.py) counts it as neither.
    """
    try:
        return Path(path).resolve().relative_to(
            Path(repo_root).resolve()).as_posix()
    except ValueError:
        return None


def names_at(revision, path, repo_root):
    relative = relative_to_repo(path, repo_root)
    if relative is None:
        return None
    result = subprocess.run(
        ["git", "-C", str(repo_root), "show", f"{revision}:{relative}"],
        capture_output=True, text=True)
    if result.returncode != 0:
        return None
    return exported_names(result.stdout)


def revisions_touching(path, repo_root):
    """Every commit that changed this file, newest first.

    --full-history because default history simplification prunes one side of a
    merge, and an alias published on the pruned side is exactly one this check
    must still remember. On this repository the two agree (27 revisions against
    28) and the union they produce is identical, so the wider walk costs one
    more blob and closes a hole rather than moving the answer.
    """
    relative = relative_to_repo(path, repo_root)
    if relative is None:
        return []
    result = subprocess.run(
        ["git", "-C", str(repo_root), "log", "--format=%H", "--full-history",
         "--", relative],
        capture_output=True, text=True)
    if result.returncode != 0:
        return []
    return result.stdout.split()


def _batch_payloads(data):
    """Split `git cat-file --batch` output into its payloads.

    Each record is `<oid> <type> <size>\\n<size bytes>\\n`. The size is a byte
    count, which is why this handles bytes and never decodes the stream whole.
    A malformed or truncated record stops the scan; the caller compares the
    payload count against what it asked for, so a short read is reported as an
    unreadable history rather than quietly narrowing the union.
    """
    payloads, at = [], 0
    while at < len(data):
        end = data.find(b"\n", at)
        if end < 0:
            break
        header = data[at:end].split()
        if len(header) != 3 or not header[2].isdigit():
            break
        size = int(header[2])
        payloads.append(data[end + 1:end + 1 + size])
        at = end + 1 + size + 1
    return payloads


def ever_exported(path, repo_root, revisions):
    """The union of every export set this file has held, and what was unreadable.

    Union, not "the set at the oldest revision": an alias added in the middle of
    history and dropped later is exactly the disappearance this guards, and only
    a union sees it.

    Two git processes for the whole walk, not one per revision: the blobs are
    resolved in one `--batch-check`, deduplicated, and read in one `--batch`.
    Anything that leaves the union incomplete - an unresolvable revision, a path
    absent from one, a short read - comes back in `unreadable` so the caller can
    refuse instead of comparing against a set with holes in it.
    """
    revisions = list(revisions)
    relative = relative_to_repo(path, repo_root)
    if relative is None or not revisions:
        return set(), revisions

    resolved = subprocess.run(
        ["git", "-C", str(repo_root), "cat-file",
         "--batch-check=%(objectname) %(objecttype)"],
        input="".join(f"{revision}:{relative}\n" for revision in revisions),
        capture_output=True, text=True)
    lines = resolved.stdout.splitlines()
    if resolved.returncode != 0 or len(lines) != len(revisions):
        return set(), revisions          # git itself failed: nothing was read

    unreadable, blobs = [], []
    for revision, line in zip(revisions, lines):
        fields = line.split()
        # `<spec> missing` when git cannot resolve the revision or the path is
        # absent from it; a `tree` where a blob was expected is as unusable.
        if len(fields) == 2 and fields[1] == "blob":
            if fields[0] not in blobs:
                blobs.append(fields[0])  # identical blobs are read once
        else:
            unreadable.append(revision)

    names = set()
    if blobs:
        content = subprocess.run(
            ["git", "-C", str(repo_root), "cat-file", "--batch"],
            input="".join(f"{blob}\n" for blob in blobs).encode(),
            capture_output=True)
        payloads = _batch_payloads(content.stdout)
        if content.returncode != 0 or len(payloads) != len(blobs):
            return set(), revisions      # a partial read is not a comparison
        for payload in payloads:
            names |= exported_names(payload.decode("utf-8", "replace"))
    return names, unreadable


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--def-file", type=Path, default=DEFAULT_DEF)
    parser.add_argument("--repo-root", type=Path, default=REPO_ROOT)
    parser.add_argument("--base", default=None,
                        help="compare against ONE revision instead of the "
                             "whole history. `--base HEAD` on a clean tree "
                             "compares the file to itself and cannot fail, "
                             "which is what this check used to default to.")
    parser.add_argument("--revision-floor", type=int, default=10,
                        help="refuse to report clean over fewer revisions of "
                             "history than this; a wrong path, a def file "
                             "outside the repo, or a shallow clone finds none "
                             "and every export then looks new (default 10)")
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

    if arguments.base:
        base = names_at(arguments.base, arguments.def_file,
                        arguments.repo_root)
        if base is None:
            print(f"def-append-only: cannot read {arguments.def_file.name} at "
                  f"{arguments.base}, so no comparison was made. This check "
                  "verified NOTHING.", file=sys.stderr)
            return 1
        against = arguments.base
    else:
        revisions = revisions_touching(arguments.def_file,
                                       arguments.repo_root)
        if len(revisions) < arguments.revision_floor:
            print(f"def-append-only: only {len(revisions)} revision(s) of "
                  f"{arguments.def_file.name} found, below the floor of "
                  f"{arguments.revision_floor}. A path git does not know finds "
                  "no history, so every export looks new and nothing can have "
                  "been removed - this check would have verified NOTHING.",
                  file=sys.stderr)
            return 1
        base, unreadable = ever_exported(arguments.def_file,
                                         arguments.repo_root, revisions)
        if unreadable:
            print(f"def-append-only: {len(unreadable)} of {len(revisions)} "
                  "revisions could not be read, so the union is incomplete and "
                  "a removal could hide in the gap. Either git cannot resolve "
                  f"them or {arguments.def_file.name} did not exist there - a "
                  "deletion in history has to be looked at, not skipped: "
                  f"{', '.join(r[:10] for r in unreadable[:5])}",
                  file=sys.stderr)
            return 1
        against = f"any of {len(revisions)} revisions"

    removed = sorted(base - current)
    if removed:
        print(f"def-append-only: {len(removed)} export(s) present at "
              f"{against} are gone. The staged hybrid imports by name "
              "and its import\ntable is frozen, so a removed or renamed alias "
              "breaks the game - and every\nctest can still pass, because "
              "staging runs before the tests.\nTo correct a decoration, alias "
              "BOTH spellings to the same symbol.", file=sys.stderr)
        for name in removed:
            print(f"    {name}", file=sys.stderr)
        return 1

    added = len(current) - len(base & current)
    print(f"def-append-only: {len(current)} exports, none removed since "
          f"{against}" + (f", {added} added" if added else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
