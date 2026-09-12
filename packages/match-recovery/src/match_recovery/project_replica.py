"""Disposable current-policy projects for independently generated source trees.

Each replica owns its Git HEAD/index/objects. Historical objects are read through
an alternate, never checked out with filters or shared as a writable index.
This is input isolation for trusted compiler commands, not a security sandbox.
"""

from contextlib import contextmanager
import json
import os
from pathlib import Path
import subprocess
import tempfile

from .checkpoints import _content
from .components import relative_path
from .driver import working_state
from .source_replica import artifact_parent, replica_snapshot, validate_tree


def _git(root, *arguments):
    result = subprocess.run(["git", "--literal-pathspecs", "-c", "core.fsmonitor=false",
                             "-c", "core.hooksPath=" + os.devnull, *arguments], cwd=root,
                            capture_output=True, timeout=60)
    if result.returncode:
        raise ValueError("project replica Git operation failed: " + result.stderr.decode(errors="replace")[-2000:])
    return result.stdout


@contextmanager
def project_replica(root, roots, files, *, inputs=(), artifacts="build/recovery/preservation",
                    versioned_inputs=(), versioned_files=None):
    """Keep current policy/Git identity, substitute a complete source file set.

    No original ignored cache, executable, virtual environment or compiler prefix
    is copied implicitly. Explicit non-source inputs are copied when declared.
    Source/history versions are not rewritten: the private working tree carries
    the counterfactual edits relative to the original current HEAD.
    """
    root = Path(root).resolve()
    if any(name in os.environ for name in ("GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR", "GIT_INDEX_FILE",
                                           "GIT_OBJECT_DIRECTORY", "GIT_ALTERNATE_OBJECT_DIRECTORIES", "GIT_NAMESPACE")):
        raise ValueError("project replica refuses Git path overrides")
    roots = validate_tree(roots, files)
    if (not isinstance(versioned_inputs, (tuple, list)) or any(not isinstance(name, str) for name in versioned_inputs)
            or len(set(versioned_inputs)) != len(versioned_inputs)
            or not isinstance(versioned_files if versioned_files is not None else {}, dict)):
        raise ValueError("invalid versioned project inputs")
    versioned_files = dict(versioned_files or {})
    if set(versioned_files) != set(versioned_inputs) or any(not isinstance(raw, bytes) for raw in versioned_files.values()):
        raise ValueError("versioned project inputs require exactly their declared files and bytes")
    for name in (*roots, *files, *inputs, *versioned_inputs):
        relative_path(name)
        if any(part.casefold() == ".git" for part in Path(name).parts):
            raise ValueError("project replica refuses Git metadata as source")
    for name in versioned_inputs:
        if any(Path(name).is_relative_to(other) or Path(other).is_relative_to(name)
               for other in (*roots, artifacts, *(item for item in versioned_inputs if item != name))):
            raise ValueError("versioned project inputs overlap source, artifacts or another versioned path")
        if name == "recovery/project.json":
            raise ValueError("project policy cannot be a versioned source input")
        path = root / name
        if not path.is_file() or path.resolve() != path or path.stat().st_nlink != 1:
            raise ValueError("versioned project input must be a canonical unaliased regular file: " + name)
    versioned_before = {name: (root / name).read_bytes() for name in versioned_inputs}
    before = working_state(root)
    source_before = replica_snapshot(root, roots)
    contents = {}
    for name, state in before["files"].items():
        relative_path(name)
        if any(part.casefold() == ".git" for part in Path(name).parts):
            raise ValueError("project replica refuses Git metadata as source")
        if state is not None and not any(Path(name).is_relative_to(base) for base in roots):
            contents[name] = _content(root, name, state)
    for name in inputs:
        relative_path(name)
        if name in contents or any(Path(name).is_relative_to(base) for base in roots):
            continue
        path = root / name
        if not path.is_file() or path.is_symlink() or path.resolve() != path:
            raise ValueError("project replica needs canonical explicit inputs: " + name)
        contents[name] = ("100755" if path.stat().st_mode & 0o111 else "100644", path.read_bytes())
    for name, content in files.items():
        state = before["files"].get(name)
        mode = "100755" if state is not None and state[0] != "symlink" and state[0] & 0o111 else "100644"
        contents[name] = (mode, content)
    for name, content in versioned_files.items():
        mode = "100755" if (root / name).stat().st_mode & 0o111 else "100644"
        contents[name] = (mode, content)
    explicit = {name: (root / name).read_bytes() for name in inputs if name not in source_before}
    if any(contents[name][1] != data for name, data in explicit.items()
           if name not in versioned_files and contents[name][0] != "120000"):
        raise ValueError("project replica explicit inputs changed while copying")

    def fresh():
        if (working_state(root) != before or replica_snapshot(root, roots) != source_before
                or any((root / name).read_bytes() != data for name, data in explicit.items())
                or any(not (root / name).is_file() or (root / name).resolve() != root / name
                       or (root / name).stat().st_nlink != 1 or (root / name).read_bytes() != data
                       for name, data in versioned_before.items())):
            raise ValueError("project inputs or Git state changed during prepared source replication")

    parent = artifact_parent(root, artifacts, excluded_roots=roots)
    objects = (root / os.fsdecode(_git(root, "rev-parse", "--git-path", "objects")).strip()).resolve()
    algorithm = _git(root, "rev-parse", "--show-object-format").decode().strip()
    try:
        with tempfile.TemporaryDirectory(prefix="project-", dir=parent) as temporary:
            destination = Path(temporary)
            _git(destination, "init", "--quiet", "--template=", "--object-format=" + algorithm)
            (destination / ".git/objects/info/alternates").write_text(json.dumps(str(objects), ensure_ascii=False) + "\n")
            _git(destination, "config", "core.hooksPath", os.devnull)
            _git(destination, "config", "core.fsmonitor", "false")
            _git(destination, "update-ref", "--no-deref", "HEAD", before["head"].decode().strip())
            _git(destination, "read-tree", before["head"].decode().strip())
            for base in roots:
                (destination / base).mkdir(parents=True, exist_ok=True)
            # Files first; aliases may point only to a copied internal file.
            for symlinks in (False, True):
                for name, (mode, content) in contents.items():
                    if (mode == "120000") != symlinks:
                        continue
                    target = destination / name
                    target.parent.mkdir(parents=True, exist_ok=True)
                    if target.parent.resolve() != target.parent:
                        raise ValueError("project replica refuses aliased parent directories")
                    if symlinks:
                        link = os.fsdecode(content)
                        resolved = (target.parent / link).resolve()
                        if Path(link).is_absolute() or not resolved.is_relative_to(destination) or not resolved.is_file():
                            raise ValueError("project replica aliases must name copied internal files")
                        target.symlink_to(link)
                    else:
                        target.write_bytes(content)
                        target.chmod(0o755 if mode == "100755" else 0o644)
            fresh()
            try:
                yield destination
            finally:
                if any(not (destination / name).is_file() or (destination / name).resolve() != destination / name
                       or (destination / name).stat().st_nlink != 1 or (destination / name).read_bytes() != data
                       for name, data in versioned_files.items()):
                    raise ValueError("versioned build input changed in prepared project")
    finally:
        fresh()
