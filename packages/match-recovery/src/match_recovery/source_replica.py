"""Disposable, byte-snapshotted source trees; no compiler or target semantics.

Copy, never hardlink. Originals are only read, including during cleanup. This
helper does not establish equivalence between original and replica builds.
"""

from contextlib import contextmanager
from pathlib import Path, PurePosixPath
import subprocess
import tempfile

from .components import relative_path


def replica_roots(roots):
    """Keep outermost declared roots, preserving no implicit project-root scope."""
    for name in roots:
        relative_path(name)
    return tuple(name for name in sorted(set(roots))
                 if not any(name != other and PurePosixPath(name).is_relative_to(other) for other in roots))


def _files(root, roots):
    result = {}
    for name in roots:
        directory = root / name
        if any(path.is_symlink() for path in (directory, *directory.parents) if path.is_relative_to(root)):
            raise ValueError(f"source replica refuses symlink roots: {name}")
        if not directory.is_dir():
            raise ValueError(f"source replica root is missing: {name}")
        result[name] = None
        for path in sorted(directory.rglob("*")):
            if path.is_symlink():
                raise ValueError(f"source replica refuses symlinks: {path.relative_to(root)}")
            if path.is_file():
                result[path.relative_to(root).as_posix()] = path.read_bytes()
            elif path.is_dir():
                result[path.relative_to(root).as_posix()] = None
            else:
                raise ValueError(f"source replica requires regular files: {path.relative_to(root)}")
    return result


@contextmanager
def source_replica(root: Path, roots: tuple[str, ...], replacements: dict[str, bytes], *, artifacts: str | None = None):
    """Yield a private tree, then reject changed originals even on failure.

    Replacement paths must already exist in explicit roots. Every file in those
    roots is copied, including extensionless/macro include inputs. There are no
    persistent artifacts or edit-and-restore operations on the product tree.
    External dependencies remain the caller's freshness responsibility.
    If artifacts is supplied, temporary copies stay in that verified-ignored
    project directory; otherwise the platform temporary directory is used.
    """
    root = root.resolve()
    roots = replica_roots(roots)
    if not roots:
        raise ValueError("source replica requires explicit roots")
    parent = None
    if artifacts is not None:
        relative_path(artifacts)
        parent = root / artifacts
        if (any(parent.is_relative_to(root / name) for name in roots)
                or any(path.is_symlink() for path in (parent, *parent.parents) if path.is_relative_to(root))):
            raise ValueError("replica artifacts must be outside source roots and not symlinked")
        ignored = subprocess.run(["git", "check-ignore", "-q", artifacts + "/"], cwd=root, capture_output=True, timeout=10)
        tracked = subprocess.run(["git", "ls-files", "--", artifacts + "/"], cwd=root, capture_output=True, timeout=10)
        if ignored.returncode != 0 or tracked.returncode != 0 or tracked.stdout:
            raise ValueError("replica artifact directory must be Git-ignored and contain no tracked files")
        parent.mkdir(parents=True, exist_ok=True)
    for path, contents in replacements.items():
        relative_path(path)
        if not isinstance(contents, bytes):
            raise ValueError("source replica replacements must be bytes")
    before = _files(root, roots)
    if any(before.get(name) is None for name in replacements):
        raise ValueError("source replica replacements must name existing files in its roots")
    try:
        with tempfile.TemporaryDirectory(prefix="recover-source-", dir=parent) as temporary:
            destination = Path(temporary)
            for name, original in before.items():
                if original is None:
                    (destination / name).mkdir(parents=True, exist_ok=True)
                    continue
                data = replacements.get(name, original)
                path = destination / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
            yield destination
    finally:
        if _files(root, roots) != before:
            raise ValueError("source replica inputs changed during compilation")
