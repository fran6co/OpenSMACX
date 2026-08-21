#!/usr/bin/env -S uv run python
"""Remove finished agent worktrees and their branches, refusing to lose work.

Recovery agents run in `.claude/worktrees/agent-*` and are told to leave the
worktree DIRTY - the coordinator collects the diff with `git apply --3way` from
the root. Nothing then removes the worktree, so they accumulate: twelve of them
before this existed, nine belonging to agents that had finished hours earlier,
each a full checkout of the tree.

    uv run tools/reap_worktrees.py            # report only
    uv run tools/reap_worktrees.py --reap     # actually remove
    uv run tools/reap_worktrees.py --experiments --reap   # also refute-*

`--experiments` additionally reaps `refute-*` worktrees whose files differ from
the main checkout. Those are EXPERIMENTS BY CONSTRUCTION: a Workflow refute
agent is told to apply a proposed fix in its own worktree in order to MEASURE
it, and the measurement comes back in its verdict - the diff is scratch. 57 of
them survived one run, each a full checkout, because the fourth refusal cannot
tell a scratch edit from uncollected work.

It is deliberately not the default and deliberately narrow: `agent-*` worktrees
are where real recovery happens and are never covered by it.

FOUR REFUSALS, and they are the point. A worktree is reaped only when all four
pass:

  * NOT LOCKED. A locked worktree has a live agent in it.
  * NOT RECENTLY TOUCHED. Locking is not universal - a Workflow's refute
    worktrees are not locked, and reaping one mid-run would break the run - so
    a worktree with a file modified in the last RECENT_MINUTES is left alone
    regardless.
  * NOT AHEAD of master. A branch carrying its own commits has work that was
    never collected, whatever its working tree looks like.
  * every DIRTY file is byte-identical to the main checkout's copy - unless
    `--experiments` is given and the worktree is a `refute-*` one, see above. That is
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
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
# How recently a worktree must have been written to count as live. Fifteen
# minutes is longer than any gap between an agent's edits and much shorter than
# the time these sit around afterwards.
RECENT_MINUTES = 15


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
    # `.claude/worktrees/` ONLY. The first draft returned every non-main
    # worktree, so a scratch checkout made somewhere else - or one the user
    # made for their own work - was listed as "ready to reap". This tool
    # removes checkouts and deletes branches; it has no business touching
    # anything it did not come here to manage.
    #
    # The whole directory, not just `agent-*`: Workflow runs put their agents
    # in `wf_*` and their own named worktrees alongside them, and those
    # accumulate exactly the same way. Everything under here is harness
    # scaffolding.
    managed = (REPO_ROOT / ".claude" / "worktrees").resolve()
    keep = []
    for entry in found:
        path = Path(entry["worktree"]).resolve()
        if path == REPO_ROOT.resolve():
            continue
        if path.parent != managed:
            continue
        keep.append(entry)
    return keep


def why_not(entry: dict, dirty_here: set) -> str | None:
    """The reason this worktree must be kept, or None if it can go."""
    path = Path(entry["worktree"])
    if entry.get("locked"):
        return "LOCKED - an agent is working in it"
    newest = 0.0
    for child in path.rglob("*"):
        if ".git" in child.parts or "build" in child.parts:
            continue
        try:
            newest = max(newest, child.stat().st_mtime)
        except OSError:
            continue
    age = (time.time() - newest) / 60 if newest else 1e9
    if age < RECENT_MINUTES:
        return f"touched {age:.0f} minute(s) ago - looks live"
    ahead = git("rev-list", "--count", "master..HEAD", cwd=path).strip()
    if ahead != "0":
        return f"branch is {ahead} commit(s) ahead of master - uncollected"
    experiment = (Path(entry["worktree"]).name.startswith("refute-")
                  and "--experiments" in sys.argv)
    for line in git("status", "--porcelain", cwd=path).splitlines():
        name = line[3:].strip().strip('"')
        if experiment:
            continue
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
