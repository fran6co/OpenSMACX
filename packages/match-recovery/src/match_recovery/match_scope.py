"""Translation units a source change can affect, for scoped match checks.

A scope is advisory selection, never a verdict: a scoped check measures only
the selected units and stays incomplete. Selection is conservative. A unit is
in scope when it changed, when a changed file's basename is in its lexical
include closure (``source_replica.include_closure``), or when that closure is
unknown. Changes to measurement policy or build inputs, and changes to mapping
or binding annotations (whose reference targets are global), widen the scope
to every unit.
"""

from pathlib import Path, PurePosixPath
import re

from .compilation_database import entries
from .source_history import git
from .source_replica import include_closure, replica_snapshot

_FORCED = re.compile(r"(?:^|\s)[/-]FI\"?([^\"\s]+)")
_ANNOTATION = re.compile(r"^[-+](?![-+]).*//\s*(?:ORIGINAL:|symbol\b|RECOVERY_)")


def changed_files(root: Path, since: str) -> tuple[str, ...]:
    """Tracked differences from ``since`` plus untracked, unignored files."""
    git(root, "rev-parse", "--verify", "--quiet", since + "^{commit}")
    tracked = git(root, "diff", "--name-only", "-z", since, "--").split(b"\0")
    untracked = git(root, "ls-files", "-o", "--exclude-standard", "-z").split(b"\0")
    return tuple(sorted({name.decode() for name in (*tracked, *untracked) if name}))


def _annotation_changes(root: Path, since: str, names) -> list[str]:
    if not names:
        return []
    diff = git(root, "diff", "-U0", since, "--", *names).decode("utf-8", "replace")
    current, touched = None, []
    for line in diff.splitlines():
        if line.startswith("+++ b/"):
            current = line[6:]
        elif current and _ANNOTATION.match(line) and current not in touched:
            touched.append(current)
    return touched


def _forced(entry, root: Path) -> tuple[str, ...]:
    words = entry.arguments if entry.arguments is not None else ()
    text = " ".join(words) if words else (entry.command or "")
    result = []
    for match in _FORCED.finditer(text):
        path = Path(match.group(1))
        if path.is_absolute() and path.resolve().is_relative_to(root):
            result.append(path.resolve().relative_to(root).as_posix())
    return tuple(result)


def affected_units(root: Path, settings, database_bytes: bytes, changed, *, since: str | None = None) -> dict:
    """Return ``{"units": set | None, "widened": reason | None, ...}``; None means every unit."""
    root = root.resolve()
    roots = tuple(dict.fromkeys((*settings.source.roots, *settings.input_roots)))
    changed = tuple(sorted(set(changed)))
    in_roots = [name for name in changed if any(PurePosixPath(name).is_relative_to(item) for item in roots)]
    policy = {"recovery/project.json", settings.source.image_pin, *settings.inputs}
    outside = [name for name in changed if name not in in_roots]
    widening = [name for name in outside if name in policy
                or name.startswith(("recovery/environment/", "cmake/", "packages/match-recovery/src/"))
                or PurePosixPath(name).name in ("CMakeLists.txt", "pyproject.toml", "uv.lock")]
    if widening:
        return {"units": None, "widened": "measurement policy or build input changed: " + ", ".join(widening),
                "changed": list(changed)}
    if since is not None:
        annotated = _annotation_changes(root, since, [name for name in in_roots if (root / name).is_file()])
        if annotated:
            return {"units": None, "widened": "mapping or binding annotations changed in: " + ", ".join(annotated),
                    "changed": list(changed)}
    if not in_roots:
        return {"units": set(), "widened": None, "changed": list(changed)}
    files = replica_snapshot(root, roots)
    changed_names = {PurePosixPath(name).name.casefold() for name in in_roots}
    changed_set = set(in_roots)
    units, unknown = set(), []
    for entry in entries(database_bytes, root):
        if not entry.file.is_relative_to(root):
            continue
        unit = entry.file.relative_to(root).as_posix()
        forced = _forced(entry, root)
        if unit in changed_set or changed_set.intersection(forced):
            units.add(unit)
            continue
        closure = include_closure(files, unit, forced)
        if closure is None:
            units.add(unit)
            unknown.append(unit)
        elif changed_names & closure:
            units.add(unit)
    return {"units": units, "widened": None, "changed": list(changed), "unknown_closure": sorted(unknown)}
