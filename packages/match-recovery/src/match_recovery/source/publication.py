"""Guarded publication of source edits prepared from measured input bytes.

This is not a measurement API: callers must measure and authorize the changes.
All outputs are staged before replacing any input. Each replacement is atomic;
a multi-file batch is not a filesystem transaction. On ordinary failure we undo
only our own replacements, never a subsequent editor's change. Backups survive
an interrupted process or an unsafe rollback. Use a coordinator-owned worktree:
uncooperative writers can still race the last check and filesystem replacement.
"""

from collections.abc import Callable, Mapping
import os
from pathlib import Path
import stat
import tempfile


class PublicationError(OSError):
    """Publication failed; retained backups are identified in the diagnostic."""


def publish(expected: Mapping[Path, bytes], replacements: Mapping[Path, bytes], *,
            validate: Callable[[], None], staging_directory: Path) -> tuple[Path, ...]:
    """Publish existing regular files, refusing stale bytes or changed modes.

    ``validate`` rechecks the caller's complete measurement inputs before any
    staging and again immediately before publication. ``expected`` may include
    untouched files; all are rechecked. Missing, aliased, symlinked and hardlinked
    output paths are refused. The caller supplies a private disposable staging
    directory outside measurement inputs, on the same filesystem as outputs.
    No-op outputs retain their inode and timestamp.
    The return value names changed files, not accepted recovery subjects.
    """
    before, after = dict(expected), dict(replacements)
    if not callable(validate) or not before or set(after) - set(before):
        raise ValueError("publication requires expected inputs and an input validator")
    if any(not isinstance(path, Path) or not isinstance(raw, bytes)
           for values in (before, after) for path, raw in values.items()):
        raise ValueError("publication requires Path keys and byte contents")
    resolved = [path.resolve() for path in before]
    if len(set(resolved)) != len(resolved):
        raise ValueError("aliased publication paths")
    changes = {path: raw for path, raw in after.items() if raw != before[path]}
    modes = {}
    for path in before:
        info = path.lstat()
        if not stat.S_ISREG(info.st_mode):
            raise ValueError(f"publication input is not a regular file: {path}")
        if path in changes and (info.st_nlink != 1 or path.absolute() != path.resolve()):
            raise ValueError(f"publication output is linked or aliased: {path}")
        modes[path] = stat.S_IMODE(info.st_mode)

    def current():
        for path, raw in before.items():
            info = path.lstat()
            if (not stat.S_ISREG(info.st_mode) or stat.S_IMODE(info.st_mode) != modes[path]
                    or path.read_bytes() != raw
                    or (path in changes and (info.st_nlink != 1 or path.absolute() != path.resolve()))):
                raise ValueError(f"source changed before publication: {path}")
        validate()

    current()
    if not changes:
        return ()
    if staging_directory.absolute() != staging_directory.resolve():
        raise ValueError("publication staging must be unaliased")
    staging_directory.mkdir(parents=True, exist_ok=True)
    if any(path.stat().st_dev != staging_directory.stat().st_dev for path in changes):
        raise ValueError("publication staging must be unaliased and on the output filesystem")
    staged, backups, written = {}, {}, []
    retained = set()

    def stage(path, raw, suffix):
        fd, name = tempfile.mkstemp(prefix=path.name + ".", suffix=suffix, dir=staging_directory)
        temporary = Path(name)
        try:
            with os.fdopen(fd, "wb") as stream:
                stream.write(raw)
                os.chmod(temporary, modes[path])
                stream.flush()
                os.fsync(stream.fileno())
        except BaseException:
            temporary.unlink(missing_ok=True)
            raise
        return temporary

    try:
        for path, raw in changes.items():
            backups[path] = stage(path, before[path], ".before")
            staged[path] = stage(path, raw, ".after")
        current()
        for path, temporary in staged.items():
            # Do not overwrite an edit observed after the batch-wide check.
            if (path.is_symlink() or path.read_bytes() != before[path]
                    or stat.S_IMODE(path.stat().st_mode) != modes[path]
                    or path.stat().st_nlink != 1 or path.absolute() != path.resolve()):
                raise ValueError(f"source changed during publication: {path}")
            # Register intent first: interruption can happen just after replace
            # has succeeded, before the next Python statement executes.
            written.append(path)
            os.replace(temporary, path)
        for path, raw in changes.items():
            if path.is_symlink() or path.read_bytes() != raw:
                raise ValueError(f"source changed after publication: {path}")
    except BaseException as error:
        for path in reversed(written):
            try:
                if (path.is_symlink()
                        or stat.S_IMODE(path.stat().st_mode) != modes[path]
                        or path.stat().st_nlink != 1 or path.absolute() != path.resolve()):
                    raise ValueError("a subsequent edit must not be overwritten")
                content = path.read_bytes()
                if content == before[path]:
                    continue  # Replacement did not happen, or was already undone.
                if content != changes[path]:
                    raise ValueError("a subsequent edit must not be overwritten")
                os.replace(backups[path], path)
            except (OSError, ValueError):
                retained.add(backups[path])
        if retained:
            raise PublicationError("publication failed; subsequent edits were preserved; "
                                   "original source backups retained at "
                                   + ", ".join(str(path) for path in sorted(retained))) from error
        raise
    finally:
        for path in (*staged.values(), *backups.values()):
            if path not in retained:
                path.unlink(missing_ok=True)
    return tuple(changes)
