"""Raw committed worktree materialization shared by workers and integration.

Never run checkout hooks/filters, repurpose an existing path or erase a failed
candidate. Creating a checkout is not recovery or acceptance.
"""

from contextlib import contextmanager
import hashlib
import os
from pathlib import Path
import posixpath
import subprocess
import tempfile

from .components import relative_path
from .driver import working_state
from .model import require_text
from .source_history import _blobs, _tree, git


def run_git(root, options, *args, input=None, codes=(0,)):
    result = subprocess.run(["git", "--literal-pathspecs", *options, *args], cwd=root, input=input,
                            capture_output=True, timeout=120)
    if result.returncode not in codes:
        raise ValueError("worktree Git command failed: " + result.stderr.decode(errors="replace")[-2000:])
    return result


def committed_files(root, revision):
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


def assert_raw_checkout(root, revision):
    entries, blobs = committed_files(root, revision)
    state = working_state(root)
    if set(state["files"]) != set(entries):
        raise ValueError("worktree preparation requires a clean tracked checkout")
    for name, (mode, _, blob) in entries.items():
        actual, data = state["files"][name], blobs[blob]
        expected = (("symlink", os.fsdecode(data)) if mode == "120000" else
                    (mode, hashlib.sha256(data).hexdigest()))
        normalized = (actual if actual is None or actual[0] == "symlink" else
                      ("100755" if actual[0] & 0o111 else "100644", actual[1]))
        if normalized != expected:
            raise ValueError("worktree preparation refuses dirty or filtered source bytes")
    return state


def destination_state(root, revision, options):
    entries, _ = committed_files(root, revision)
    state = working_state(root)
    if (set(state["files"]) != set(entries) or any(value is None for value in state["files"].values())
            or run_git(root, options, "diff", "--name-only", "-z", revision).stdout):
        raise ValueError("worktree preparation requires a clean destination checkout")
    # Preserve lossless Git-normalized text on the coordinator; newly created
    # worktrees still receive exact committed bytes, never converted copies.
    return state


@contextmanager
def git_options(root):
    with tempfile.TemporaryDirectory(prefix="recovery-git-hooks-") as hooks:
        options = ["-c", "core.hooksPath=" + hooks, "-c", "core.fsmonitor=false"]
        drivers = run_git(root, options, "config", "--name-only", "--get-regexp", r"^merge\..*\.driver$", codes=(0, 1))
        if drivers.stdout:
            raise ValueError("worktree preparation does not execute external merge drivers")
        filters = run_git(root, options, "config", "--name-only", "--get-regexp", r"^filter\..*\.(clean|smudge|process|required)$", codes=(0, 1))
        for prefix in sorted({line.rsplit(".", 1)[0] for line in filters.stdout.decode().splitlines()}):
            for field, value in (("clean", ""), ("smudge", ""), ("process", ""), ("required", "false")):
                options.extend(["-c", prefix + "." + field + "=" + value])
        yield options


def new_destination(root, options, worktree, branch, *, excluded_roots=()):
    require_text(branch, "candidate branch")
    path = Path(worktree)
    path = Path(os.path.abspath(path if path.is_absolute() else root / path))
    if path.exists() or path.is_symlink() or path.resolve() != path:
        raise ValueError("candidate worktree must be a new, non-symlinked path")
    run_git(root, options, "check-ref-format", "refs/heads/" + branch)
    if run_git(root, options, "show-ref", "--verify", "--quiet", "refs/heads/" + branch, codes=(0, 1)).returncode == 0:
        raise ValueError("candidate branch already exists; no branches are overwritten")
    for key in ("--git-dir", "--git-common-dir"):
        metadata = (root / git(root, "rev-parse", key).decode().strip()).resolve()
        if path.is_relative_to(metadata):
            raise ValueError("candidate worktree must be outside Git metadata")
    if any(path.is_relative_to(root / name) for name in excluded_roots):
        raise ValueError("worker worktree must be outside configured source roots")
    # Absolute paths avoid pathspec-magic tokens without interpreting globs.
    if path.is_relative_to(root) and run_git(root, options, "--no-literal-pathspecs", "check-ignore", "--quiet", "--no-index", "--",
                                            str(path), codes=(0, 1)).returncode != 0:
        raise ValueError("a nested candidate worktree must be in ignored storage")
    return path


def raw_inputs(root, options, revision):
    entries, blobs = committed_files(root, revision)
    attributes = run_git(root, options, "check-attr", "--source=" + revision, "-z", "--stdin", "filter",
                         input=b"".join(os.fsencode(name) + b"\0" for name in entries)).stdout.split(b"\0")
    if attributes[-1:] != [b""] or len(attributes[:-1]) != 3 * len(entries):
        raise ValueError("unsupported Git attribute response")
    if any(value not in (b"unspecified", b"unset") for value in attributes[2::3]):
        raise ValueError("active checkout filters require coordinator resolution before candidate preparation")
    return entries, blobs


def materialize(root, options, path, branch, commit, entries, blobs):
    # Recheck the new-path boundary immediately before any mutation. Failures
    # preserve partial output for inspection rather than attempting a reset/reap.
    new_destination(root, options, path, branch)
    path.mkdir(parents=True)
    run_git(root, options, "worktree", "add", "--no-checkout", "-b", branch, str(path), commit)
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
    run_git(path, options, "read-tree", commit)
    assert_raw_checkout(path, commit)
