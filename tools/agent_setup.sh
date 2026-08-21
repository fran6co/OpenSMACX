#!/usr/bin/env bash
# Step 0 for a recovery agent working in a git worktree.
#
# Two agents in a row lost time to the same two things, and both were things
# the brief ASSERTED rather than did:
#
#   * The worktree was branched from a stale base - one was 911 commits behind
#     `master` and had none of the tooling the brief names. The agent had to
#     work out `git merge --ff-only master` for itself.
#   * `.opensmacx/` is gitignored, so the pinned image and the build are not in
#     the worktree. The brief said every tool reads `OPENSMACX_IMAGE`; one
#     agent symlinked the image instead, because at the time that was false.
#
# Prints the exports to eval. Usage, from inside the worktree:
#
#     eval "$(bash <root>/tools/agent_setup.sh <root>)"
#
set -euo pipefail

root="${1:?usage: agent_setup.sh <path-to-main-checkout>}"
root="$(cd "$root" && pwd)"

if [ "$root" = "$(pwd)" ]; then
  echo "agent_setup.sh: run this from INSIDE the worktree, not the root" >&2
  exit 2
fi

# Catch up to the base the brief describes. --ff-only, so this can only ever
# fast-forward: if the worktree already has commits of its own it fails loudly
# rather than merging under the agent.
base="$(git -C "$root" rev-parse HEAD)"
if ! git merge-base --is-ancestor "$base" HEAD; then
  if [ -n "$(git status --porcelain)" ]; then
    echo "agent_setup.sh: this worktree is behind $root's HEAD and has" >&2
    echo "  uncommitted changes, so it cannot be fast-forwarded. Hand the" >&2
    echo "  diff back before catching up:  git diff -- src/ > /tmp/mine.patch" >&2
    exit 2
  fi
  echo "# fast-forwarding worktree to the root's HEAD" >&2
  git merge --ff-only "$base" >&2
fi

image="$root/.opensmacx/game/terranx_original.exe"
[ -f "$image" ] || { echo "agent_setup.sh: no image at $image" >&2; exit 2; }

echo "export OPENSMACX_IMAGE='$image'"
echo "export OPENSMACX_COMPILE_COMMANDS='$root/build/compile_commands.json'"
echo "export OPENSMACX_ROOT='$root'"
