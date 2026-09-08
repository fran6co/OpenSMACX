"""Coordinator-only local publication, called inside a fresh driver epoch.

No saved verdict is an input. The driver keeps its lease through verification
and publication. Git objects are built from raw verified files, without filters
or hooks; the original index is leased and the branch is compare-and-swapped.
This is not cross-worktree integration or authentication of actor labels.
"""

import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

from .model import require_text


def authorization(value, coordinator, worker, reviewer):
    if not isinstance(value, dict) or set(value) != {"author", "message", "allow_migration"}:
        raise ValueError("checkpoint authorization needs author, message and allow_migration")
    require_text(value["message"], "checkpoint message")
    if (value["author"] != coordinator or value["author"] in {worker, reviewer}
            or type(value["allow_migration"]) is not bool):
        raise ValueError("checkpoint publication requires independent task coordinator authorization")
    return dict(value)


def _git(root, *args, input=None, index=None):
    env = dict(os.environ)
    if index is not None:
        env["GIT_INDEX_FILE"] = str(index)
    return subprocess.run(["git", "--literal-pathspecs", *args], cwd=root, env=env, input=input,
                          capture_output=True, check=True).stdout


def preflight(root):
    if any(name in os.environ for name in ("GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR")):
        raise ValueError("checkpoint publication refuses Git path overrides")
    if Path(os.fsdecode(_git(root, "rev-parse", "--show-toplevel")).strip()).resolve() != root:
        raise ValueError("checkpoint publication requires the repository root")
    for name in ("MERGE_HEAD", "CHERRY_PICK_HEAD", "REVERT_HEAD", "rebase-merge", "rebase-apply", "sequencer", "BISECT_START"):
        if (root / os.fsdecode(_git(root, "rev-parse", "--git-path", name)).strip()).exists():
            raise ValueError("finish the existing Git operation before checkpoint publication")
    if _git(root, "diff", "--cached", "--ita-visible-in-index", "--name-only", "-z", "HEAD"):
        raise ValueError("checkpoint publication refuses staged changes; preserve the index")
    if any(not entry.startswith(b"H ") for entry in _git(root, "ls-files", "-v", "-z").split(b"\0") if entry):
        raise ValueError("checkpoint publication refuses special index entries; preserve skip-worktree/assume-unchanged flags")
    branch = _git(root, "symbolic-ref", "HEAD").decode().strip()
    if not branch.startswith("refs/heads/"):
        raise ValueError("checkpoint publication requires a local branch")
    return branch


def _content(root, name, state):
    path = root / name
    if path.parent.resolve() != path.parent:
        raise ValueError("checkpoint publication refuses symlinked parent directories")
    if state[0] == "symlink":
        content = os.fsencode(os.readlink(path))
        if os.fsdecode(content) == state[1]:
            return "120000", content
    elif not path.is_symlink() and path.stat().st_mode == state[0]:
        content = path.read_bytes()
        if hashlib.sha256(content).hexdigest() == state[1]:
            return ("100755" if state[0] & 0o111 else "100644"), content
    raise ValueError("verified checkpoint file changed during publication")


def publish(root, expected, allowed, branch, message):
    """Publish only a just-verified state. Caller must hold the driver lease.

    An interruption after ref publication but before index replacement can leave
    the previous index staged against the new commit. Preserve it for inspection;
    never reset, roll back a branch, or rerun a saved acceptance receipt.
    """
    from .driver import working_state

    def current():
        if working_state(root) != expected or preflight(root) != branch:
            raise ValueError("verified checkpoint state changed; preserve work and reverify")

    current()
    baseline = {}
    for row in _git(root, "ls-tree", "-rz", "HEAD").split(b"\0"):
        if row:
            metadata, name = row.split(b"\t", 1)
            mode, kind, blob = metadata.decode().split()
            if kind != "blob":
                raise ValueError("checkpoint publication requires ordinary files, not submodules")
            baseline[os.fsdecode(name)] = (mode, blob)
    algorithm = _git(root, "rev-parse", "--show-object-format").decode().strip()
    candidate = {}
    for name, state in expected["files"].items():
        if state is None:
            continue
        mode, content = _content(root, name, state)
        blob = hashlib.new(algorithm, b"blob " + str(len(content)).encode() + b"\0" + content).hexdigest()
        candidate[name] = (mode, blob)
    # Compare raw files, not `git diff`: clean filters, CRLF conversion and
    # core.fileMode=false must not hide a difference in the verified snapshot.
    names = {name for name in baseline.keys() | candidate.keys() if baseline.get(name) != candidate.get(name)}
    if names - set(allowed):
        raise ValueError("checkpoint contains changes outside authorized source and decision history")
    if not names:
        current()
        return {"published": False, "reason": "no_changes"}
    index = root / os.fsdecode(_git(root, "rev-parse", "--git-path", "index")).strip()
    if index.is_symlink():
        raise ValueError("checkpoint publication refuses a symlinked index")
    lock = index.with_name(index.name + ".lock")
    # Git's own index lease prevents cooperating Git writers, not arbitrary
    # filesystem writers. Freshness checks detect noncooperating source changes.
    with lock.open("xb") as leased:
        owns_lock = True
        try:
            current()
            with tempfile.TemporaryDirectory(prefix="recovery-checkpoint-") as directory:
                temporary = Path(directory) / "index"
                _git(root, "read-tree", expected["head"].decode().strip(), index=temporary)
                for name in sorted(names):
                    path = root / name
                    state = expected["files"].get(name)
                    if path.parent.resolve() != path.parent:
                        raise ValueError("checkpoint publication refuses symlinked parent directories")
                    if state is None:
                        _git(root, "update-index", "--force-remove", "--", name, index=temporary)
                        continue
                    mode, content = _content(root, name, state)
                    blob = _git(root, "hash-object", "-w", "--stdin", "--no-filters", input=content).decode().strip()
                    if (mode, blob) != candidate[name]:
                        raise ValueError("verified checkpoint file changed during publication")
                    _git(root, "update-index", "--add", "--cacheinfo", mode, blob, name, index=temporary)
                tree = _git(root, "write-tree", index=temporary).decode().strip()
                new_index = temporary.read_bytes()
                entries = _git(root, "ls-files", "--stage", "-z", index=temporary)
                current()
                parent = expected["head"].decode().strip()
                commit = _git(root, "commit-tree", tree, "-p", parent, input=message.encode()).decode().strip()
                current()
                leased.write(new_index)
                leased.flush()
                os.fsync(leased.fileno())
                _git(root, "-c", "core.hooksPath=" + directory, "update-ref",
                     "-m", "recovery checkpoint", branch, commit, parent)
            leased.close()
            os.replace(lock, index)
            owns_lock = False
            after = working_state(root)
            if (after["head"].decode().strip() != commit
                    or {k: v for k, v in after["files"].items() if v is not None}
                    != {k: v for k, v in expected["files"].items() if v is not None}
                    or after["index"] != entries or preflight(root) != branch):
                raise ValueError("checkpoint published but checkout changed; preserve work and inspect before continuing")
            return {"published": True, "commit": commit, "parent": parent, "files": sorted(names)}
        finally:
            # Only our exclusively created lock; never remove another writer's.
            if owns_lock and lock.exists():
                lock.unlink()
