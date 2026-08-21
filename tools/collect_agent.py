#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["typer"]
# ///
"""Take an agent's worktree changes into the main checkout, safely.

Collecting a finished agent is six steps, and doing them by hand went wrong
twice in one day:

  * `git apply --check --3way` EXITS 0 FOR PATCHES THAT CONFLICT. It validates
    that hunks can be located and three-way merged; a merge that leaves
    conflict markers is a successful merge, not a failed application. Four
    patches in a row reported "applies" and then produced `UU`.
  * `git apply ... | head` prints its per-file success BEFORE writing, so the
    SIGPIPE from `head` closing the pipe kills it in between - it says
    "Applied cleanly" and writes nothing.

So this never pipes, never trusts `--check`, and reads the tree afterwards to
find out what actually happened.

    uv run tools/collect_agent.py <agent-id-or-worktree-path>
    uv run tools/collect_agent.py <id> --resolve-ours

Conflicts are reported, not resolved, unless `--resolve-ours` is passed.
`--resolve-ours` keeps the main checkout's side and PRINTS every discarded
line, because agents branch before later corrections land and their side is
usually the superseded one - but "usually" is not a reason to drop it unread.
Three of four discards checked that way were correct and reading them is what
established it.
"""
import pathlib
import re
import subprocess
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
WORKTREES = REPO_ROOT / ".claude" / "worktrees"


def git(*args: str, cwd: pathlib.Path = REPO_ROOT) -> subprocess.CompletedProcess:
    """Never through a pipe, and the result carries its own status."""
    return subprocess.run(("git",) + args, cwd=cwd, capture_output=True,
                          text=True)


def resolve(target: str) -> pathlib.Path:
    candidate = pathlib.Path(target)
    if candidate.is_dir():
        return candidate.resolve()
    for name in (target, f"agent-{target}"):
        guess = WORKTREES / name
        if guess.is_dir():
            return guess.resolve()
    raise SystemExit(f"no worktree for {target!r} under {WORKTREES}")


def conflicts() -> list[str]:
    """Files the index says are unmerged - the only honest conflict test."""
    out = git("status", "--porcelain").stdout.splitlines()
    return [line[3:].strip() for line in out
            if line[:2] in ("UU", "AA", "DD", "AU", "UA", "DU", "UD")]


def resolve_ours(path: pathlib.Path) -> int:
    """Keep our side; print theirs. Returns how many lines were discarded."""
    lines = path.read_text().splitlines(keepends=True)
    out, i, dropped = [], 0, 0
    while i < len(lines):
        if lines[i].startswith("<<<<<<<"):
            i += 1
            ours = []
            while i < len(lines) and not lines[i].startswith("======="):
                ours.append(lines[i]); i += 1
            i += 1
            theirs = []
            while i < len(lines) and not lines[i].startswith(">>>>>>>"):
                theirs.append(lines[i]); i += 1
            i += 1
            out.extend(ours)
            dropped += len(theirs)
            print(f"\n  --- {path.name}: DISCARDING {len(theirs)} agent line(s) "
                  f"---")
            print("".join(f"  | {t}" for t in theirs) or "  | (nothing)")
        else:
            out.append(lines[i]); i += 1
    path.write_text("".join(out))
    return dropped


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        raise SystemExit(__doc__)
    worktree = resolve(args[0])

    dirty = [line for line in git("status", "--porcelain").stdout.splitlines()
             if not line.startswith("??")]
    if dirty:
        raise SystemExit(
            "REFUSING: the main checkout has uncommitted changes. Collecting "
            "on top of them makes the two indistinguishable if the gate then "
            "fails.\n  " + "\n  ".join(dirty[:8]))

    patch = git("diff", "HEAD", cwd=worktree).stdout
    if not patch.strip():
        print(f"{worktree.name}: nothing to collect")
        raise SystemExit(0)
    scratch = REPO_ROOT / ".git" / "collect-agent.patch"
    scratch.write_text(patch)
    markers = len(re.findall(r"^\+.*ORIGINAL: 0x", patch, re.M))
    print(f"{worktree.name}: {len(patch.splitlines()):,} diff line(s), "
          f"{markers} marker line(s) added")

    applied = git("apply", "--3way", str(scratch))
    for line in (applied.stdout + applied.stderr).splitlines():
        print(f"  {line}")

    unmerged = conflicts()
    if not unmerged:
        print("\nno conflicts. Build and gate before committing:\n"
              "  cmake --build build\n"
              "  uv run tools/osmx.py check      # read the GATE EXIT line")
        raise SystemExit(0)

    print(f"\n{len(unmerged)} file(s) conflicted: {', '.join(unmerged)}")
    if "--resolve-ours" not in sys.argv:
        print("re-run with --resolve-ours to keep this checkout's side and "
              "print what the agent's side said")
        raise SystemExit(1)

    dropped = sum(resolve_ours(REPO_ROOT / name) for name in unmerged)
    git("add", *unmerged)
    print(f"\nresolved {len(unmerged)} file(s), {dropped} agent line(s) "
          f"discarded and printed above. READ THEM before committing.\n"
          "Then build and gate:\n  cmake --build build\n"
          "  uv run tools/osmx.py check      # read the GATE EXIT line")
