#!/usr/bin/env -S uv run python
"""Remove finished agent worktrees and their branches, refusing to lose work.

Recovery agents run in `.claude/worktrees/agent-*` and are told to leave the
worktree DIRTY - the coordinator collects the diff with `git apply --3way` from
the root. Nothing then removes the worktree, so they accumulate: twelve of them
before this existed, nine belonging to agents that had finished hours earlier,
each a full checkout of the tree.

    uv run tools/reap_worktrees.py            # report only
    uv run tools/reap_worktrees.py --reap     # actually remove

FOUR REFUSALS, and they are the point. A worktree is reaped only when all four
pass:

  * NOT LOCKED. A locked worktree has a live agent in it.
  * NOT AHEAD of master. A branch carrying its own commits has work that was
    never collected, whatever its working tree looks like.
  * every DIRTY file is byte-identical to the main checkout's copy. That is
    what "already collected" means here: the coordinator applied the diff and
    committed it, so the two files agree. A dirty file that DIFFERS is
    uncollected work and stops the whole worktree from being reaped.
  * the compared FILE is not itself dirty in the main checkout. If it is,
    "identical to master's copy" compares against something uncommitted and
    proves nothing. This is per-file on purpose: a global clean-tree gate
    refused all nine reapable worktrees over one untracked new file in
    `tools/`, which cannot affect a comparison of `src/base.cpp`.

The branch goes only after its worktree is gone, so a failure between the two
leaves a recoverable state rather than a branch with no checkout.
"""

from __future__ import annotations

import filecmp
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def git(*args: str, cwd: Path | None = None) -> str:
    done = subprocess.run(("git", *args), cwd=str(cwd or REPO_ROOT),
                          capture_output=True, text=True)
    if done.returncode:
        raise RuntimeError(f"git {' '.join(args)}: {done.stderr.strip()}")
    return done.stdout


def worktrees() -> list[dict]:
    """Every worktree but the main one, with its branch and lock state."""
    found, current = [], {}
    for line in git("worktree", "list", "--porcelain").splitlines():
        if not line.strip():
            if current.get("worktree"):
                found.append(current)
            current = {}
            continue
        key, _, value = line.partition(" ")
        current[key] = value or True
    if current.get("worktree"):
        found.append(current)
    # AGENT WORKTREES ONLY. The first draft returned every non-main worktree,
    # so a scratch checkout made somewhere else - or one the user made for
    # their own work - was listed as "ready to reap". This tool removes
    # checkouts and deletes branches; it has no business touching anything it
    # did not come here to manage.
    agents = (REPO_ROOT / ".claude" / "worktrees").resolve()
    keep = []
    for entry in found:
        path = Path(entry["worktree"]).resolve()
        if path == REPO_ROOT.resolve():
            continue
        if path.parent != agents or not path.name.startswith("agent-"):
            continue
        keep.append(entry)
    return keep


def why_not(entry: dict, dirty_here: set) -> str | None:
    """The reason this worktree must be kept, or None if it can go."""
    path = Path(entry["worktree"])
    if entry.get("locked"):
        return "LOCKED - an agent is working in it"
    ahead = git("rev-list", "--count", "master..HEAD", cwd=path).strip()
    if ahead != "0":
        return f"branch is {ahead} commit(s) ahead of master - uncollected"
    for line in git("status", "--porcelain", cwd=path).splitlines():
        name = line[3:].strip().strip('"')
        theirs, ours = path / name, REPO_ROOT / name
        if not ours.exists():
            return f"{name} exists only in the worktree - uncollected"
        if name in dirty_here:
            return (f"{name} is uncommitted in the main checkout too, so "
                    f"comparing them proves nothing")
        if not filecmp.cmp(theirs, ours, shallow=False):
            return f"{name} differs from the main checkout - UNCOLLECTED"
    return None


if __name__ == "__main__":
    reap = "--reap" in sys.argv
    # The files that are uncommitted HERE. A worktree file is only safe to
    # call "already collected" when the main checkout's copy it matches is
    # itself committed.
    dirty_here = {line[3:].strip().strip('"')
                  for line in git("status", "--porcelain").splitlines()
                  if not line.startswith("??")}
    if dirty_here:
        print(f"  {len(dirty_here)} file(s) uncommitted here; any worktree "
              f"touching one of them is kept\n")

    reaped = kept = 0
    for entry in worktrees():
        path = Path(entry["worktree"])
        branch = str(entry.get("branch", "")).replace("refs/heads/", "")
        reason = why_not(entry, dirty_here)
        if reason:
            print(f"  KEEP  {path.name}  ({reason})")
            kept += 1
            continue
        if not reap:
            print(f"  ready {path.name}  ({branch})")
            reaped += 1
            continue
        git("worktree", "remove", "--force", str(path))
        if branch:
            git("branch", "-D", branch)
        print(f"  reaped {path.name} and {branch}")
        reaped += 1

    verb = "reaped" if reap else "ready to reap"
    print(f"\n{reaped} {verb}, {kept} kept")
    if not reap and reaped:
        print("re-run with --reap to remove them")
