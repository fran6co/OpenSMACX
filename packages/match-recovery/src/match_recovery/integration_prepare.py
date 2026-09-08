"""Create unverified merge candidates without moving the destination branch.

Only committed source is materialized. Hooks, checkout filters and external
merge drivers cannot participate. Compiler resources and verification are
separate; a clean text merge is never an acceptance result.
"""

import hashlib
import os
from pathlib import Path
import posixpath
import re
import subprocess
import tempfile

from .checkpoints import preflight
from .components import relative_path
from .driver import working_state
from .integration import decision, validate
from .locking import file_lock
from .model import canonical, require_text
from .records import parse_json
from .source_history import _blobs, _tree, git
from .tasks import Task, task_name


def _run(root, options, *args, input=None, codes=(0,)):
    result = subprocess.run(["git", "--literal-pathspecs", *options, *args], cwd=root, input=input,
                            capture_output=True, timeout=120)
    if result.returncode not in codes:
        raise ValueError("integration preparation Git command failed: " + result.stderr.decode(errors="replace")[-2000:])
    return result


def _files(root, revision):
    entries = _tree(root, revision, ())
    for name, (mode, kind, _) in entries.items():
        relative_path(name)
        if (any(part.casefold() == ".git" for part in Path(name).parts) or kind != "blob"
                or mode not in ("100644", "100755", "120000")):
            raise ValueError("candidate needs ordinary tracked files, not Git metadata or submodules")
        if any(parent.as_posix() in entries for parent in Path(name).parents):
            raise ValueError("candidate path traverses a tracked file or symlink")
    blobs = _blobs(root, [entry[2] for entry in entries.values()])
    for name, (mode, _, blob) in entries.items():
        if mode != "120000":
            continue
        seen, current = set(), name
        while entries[current][0] == "120000":
            if current in seen:
                raise ValueError("candidate contains a symlink cycle")
            seen.add(current)
            target = os.fsdecode(blobs[entries[current][2]])
            if Path(target).is_absolute() or "\\" in target or ":" in target:
                raise ValueError("candidate symlink must name an internal tracked file")
            current = relative_path(posixpath.normpath(str(Path(current).parent / target)))
            if current not in entries:
                raise ValueError("candidate symlink must resolve to an internal tracked file")
    return entries, blobs


def _assert_raw_checkout(root, revision):
    entries, blobs = _files(root, revision)
    state = working_state(root)
    if set(state["files"]) != set(entries):
        raise ValueError("integration preparation requires a clean tracked checkout")
    for name, (mode, _, blob) in entries.items():
        actual, data = state["files"][name], blobs[blob]
        expected = (("symlink", os.fsdecode(data)) if mode == "120000" else
                    (mode, hashlib.sha256(data).hexdigest()))
        normalized = (actual if actual is None or actual[0] == "symlink" else
                      ("100755" if actual[0] & 0o111 else "100644", actual[1]))
        if normalized != expected:
            raise ValueError("integration preparation refuses dirty or filtered source bytes")
    return state


def prepare_integration(root, name, context, *, worktree, branch):
    root = Path(root).resolve()
    task_name(name)
    require_text(branch, "candidate branch")
    if not isinstance(context, dict) or not isinstance(context.get("source_revision"), str) or not re.fullmatch(
            r"[0-9a-f]{40}|[0-9a-f]{64}", context["source_revision"]):
        raise ValueError("integration preparation requires an immutable incoming commit")
    task_path = "recovery/tasks/" + name + ".json"
    entry = _tree(root, context["source_revision"], (task_path,)).get(task_path)
    if entry is None or entry[:2] != ("100644", "blob"):
        raise ValueError("incoming source must contain a regular task authorization")
    task = Task.from_dict(parse_json(git(root, "cat-file", "blob", entry[2])))
    if task.task_id != name:
        raise ValueError("incoming task filename and identity disagree")
    context = decision(task, context)
    path = Path(worktree)
    path = Path(os.path.abspath(path if path.is_absolute() else root / path))
    if path.exists() or path.is_symlink() or path.resolve() != path:
        raise ValueError("candidate worktree must be a new, non-symlinked path")
    with file_lock(root / "build/recovery/driver.lock", timeout=0), tempfile.TemporaryDirectory(prefix="recovery-git-hooks-") as hooks:
        options = ["-c", "core.hooksPath=" + hooks, "-c", "core.fsmonitor=false"]
        drivers = _run(root, options, "config", "--name-only", "--get-regexp", r"^merge\..*\.driver$", codes=(0, 1))
        if drivers.stdout:
            raise ValueError("integration preparation does not execute external merge drivers")
        filters = _run(root, options, "config", "--name-only", "--get-regexp", r"^filter\..*\.(clean|smudge|process|required)$", codes=(0, 1))
        for prefix in sorted({line.rsplit(".", 1)[0] for line in filters.stdout.decode().splitlines()}):
            for field, value in (("clean", ""), ("smudge", ""), ("process", ""), ("required", "false")):
                options.extend(["-c", prefix + "." + field + "=" + value])
        target_branch = preflight(root)
        target, source = context["target_revision"], context["source_revision"]
        if git(root, "rev-parse", "HEAD").decode().strip() != target:
            raise ValueError("destination HEAD has moved from the explicitly authorized target")
        before = _assert_raw_checkout(root, target)
        for revision in (target, source):
            if git(root, "rev-parse", revision + "^{commit}").decode().strip() != revision:
                raise ValueError("integration inputs must be commits")
            git(root, "merge-base", "--is-ancestor", task.base_revision, revision)
        if git(root, "merge-base", source, target).decode().strip() == source:
            raise ValueError("incoming source is already incorporated in the destination")
        _run(root, options, "check-ref-format", "refs/heads/" + branch)
        if _run(root, options, "show-ref", "--verify", "--quiet", "refs/heads/" + branch, codes=(0, 1)).returncode == 0:
            raise ValueError("candidate branch already exists; no branches are overwritten")
        for key in ("--git-dir", "--git-common-dir"):
            metadata = (root / git(root, "rev-parse", key).decode().strip()).resolve()
            if path.is_relative_to(metadata):
                raise ValueError("candidate worktree must be outside Git metadata")
        # check-ignore rejects Git's literal pathspec magic. An absolute path
        # avoids a leading pathspec-magic token without interpreting a glob.
        if path.is_relative_to(root) and _run(root, options, "--no-literal-pathspecs", "check-ignore", "--quiet", "--no-index", "--",
                                             str(path), codes=(0, 1)).returncode != 0:
            raise ValueError("a nested candidate worktree must be in ignored storage")

        def fresh():
            if working_state(root) != before or preflight(root) != target_branch:
                raise ValueError("destination changed during candidate preparation; preserve work and re-evaluate")

        merged = _run(root, options, "merge-tree", "--write-tree", "--name-only", "--no-messages", "-z", target, source, codes=(0, 1))
        parts = [part for part in merged.stdout.split(b"\0") if part]
        if not parts or not re.fullmatch(rb"[0-9a-f]{40}|[0-9a-f]{64}", parts[0]):
            raise ValueError("unsupported Git merge-tree result")
        fresh()
        report = {"task_id": name, "integration_context": context, "destination_branch": target_branch,
                  "candidate_created": False, "verification": "not_measured", "ready_for_collection": False}
        if merged.returncode == 1:
            return {**report, "stop_reason": "merge_conflicts", "conflicts": [os.fsdecode(item) for item in parts[1:]]}
        if len(parts) != 1:
            raise ValueError("clean merge unexpectedly returned unresolved paths")
        tree = parts[0].decode()
        entries, blobs = _files(root, tree)
        attributes = _run(root, options, "check-attr", "--source=" + tree, "-z", "--stdin", "filter",
                          input=b"".join(os.fsencode(name) + b"\0" for name in entries)).stdout.split(b"\0")
        if attributes[-1:] != [b""] or len(attributes[:-1]) != 3 * len(entries):
            raise ValueError("unsupported Git attribute response")
        if any(value not in (b"unspecified", b"unset") for value in attributes[2::3]):
            raise ValueError("active checkout filters require coordinator resolution before candidate preparation")
        message = b"Prepare unverified recovery integration: " + name.encode() + b"\n\nIntegration authorization:\n" + canonical(context)
        commit = _run(root, options, "commit-tree", tree, "-p", target, "-p", source, input=message).stdout.decode().strip()
        fresh()
        # No existing checkout, index or branch is repurposed. Failures retain
        # any newly created candidate for inspection, never a reset/reap.
        path.mkdir(parents=True)
        try:
            _run(root, options, "worktree", "add", "--no-checkout", "-b", branch, str(path), commit)
            for symlinks in (False, True):
                for filename, (mode, _, blob) in entries.items():
                    if (mode == "120000") != symlinks:
                        continue
                    destination = path / filename
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    if destination.parent.resolve() != destination.parent:
                        raise ValueError("candidate materialization encountered a symlinked parent")
                    if symlinks:
                        destination.symlink_to(os.fsdecode(blobs[blob]))
                    else:
                        with destination.open("xb") as stream:
                            stream.write(blobs[blob])
                        destination.chmod(0o755 if mode == "100755" else 0o644)
            _run(path, options, "read-tree", commit)
            _assert_raw_checkout(path, commit)
            validate(path, task, context)
            fresh()
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            raise ValueError(f"candidate preparation incomplete; preserve and inspect {path} and branch {branch}: {error}") from error
        return {**report, "stop_reason": "candidate_prepared", "candidate_created": True,
                "worktree": str(path), "branch": branch, "commit": commit,
                "next_action": "prepare isolated compiler recipes/resources, then run fresh integration-context verification"}
