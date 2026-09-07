"""Fresh compiler recipe generation through project-owned commands.

No build-system inference or mtime freshness. Generate in a new ignored
directory, validate the database, then atomically publish just that database.
Retain its generated include context as a disposable cache, not a second ledger.
"""

import hashlib
import os
from pathlib import Path

from .build_check import Build, generate_build
from .compilation_database import entries
from .components import relative_path
from .driver import working_state
from .locking import file_lock
from .model import canonical
from .records import parse_json_value, read_json
from .source_replica import artifact_parent


def _contents(path):
    if path.is_symlink() or (path.exists() and (not path.is_file() or path.stat().st_nlink != 1)):
        raise ValueError("prepared database must be an unlinked regular file")
    return path.read_bytes() if path.exists() else None


def prepare_project(root: Path) -> dict:
    root = root.resolve()
    project = read_json(root / "recovery/project.json")
    settings = Build.read(root, key="preparation")
    if len(settings.outputs) != 1 or settings.outputs[0][1] != "compilation-database":
        raise ValueError("preparation requires exactly one compilation-database output")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict):
        raise ValueError("preparation requires the configured mappings compilation database")
    name = relative_path(mappings.get("compile_commands"))
    destination = root / name
    # A publication can neither replace project source nor inhabit an input
    # tree. The parent is verified ignored and untracked, not merely named build.
    if name in settings.inputs or name in (settings.path, "recovery/project.json"):
        raise ValueError("prepared database overlaps preparation inputs")
    artifact_parent(root, destination.parent.relative_to(root).as_posix(), excluded_roots=settings.input_roots)
    parent = artifact_parent(root, settings.artifacts, excluded_roots=settings.input_roots)
    with file_lock(parent / "prepare.lock"):
        before = settings.snapshot(root)
        state = working_state(root)
        original = _contents(destination)

        def unchanged():
            artifact_parent(root, destination.parent.relative_to(root).as_posix(), excluded_roots=settings.input_roots)
            if (settings.snapshot(root) != before or working_state(root) != state
                    or _contents(destination) != original):
                raise ValueError("preparation inputs, Git state or destination changed; preserve edits and retry")

        report = generate_build(root, settings, retain=True)
        report.update(check="compiler_preparation", database=name, published=False)
        if report["result"] != "pass":
            unchanged()
            return report
        directory = root / report["directory"]
        generated = directory / settings.outputs[0][0]
        if any(path.is_symlink() for path in generated.parents if path.is_relative_to(root)):
            raise ValueError("generated database context was replaced by an alias")
        data = _contents(generated)
        if data is None or hashlib.sha256(data).hexdigest() != report["outputs"][0]["sha256"]:
            raise ValueError("generated database changed before publication")
        rows = parse_json_value(data)
        parsed = entries(data, directory)
        # Preserve compiler argv/command quoting. Only database path identities
        # are made absolute so moving the database cannot change their meaning.
        for row, entry in zip(rows, parsed, strict=True):
            row.update(file=str(entry.file), directory=str(entry.directory))
        prepared = canonical(rows)
        unchanged()
        if original is not None:
            with (directory / "previous-database.json").open("xb") as stream:
                stream.write(original)
        staging = directory / "publish-database.json"
        with staging.open("xb") as stream:
            stream.write(prepared)
        unchanged()
        os.replace(staging, destination)
        # Do not blindly restore old bytes over a concurrent editor's work.
        # The previous database and generated context remain available on an
        # interruption; source/Git are never changed or rolled back here.
        if (settings.snapshot(root) != before or working_state(root) != state
                or _contents(destination) != prepared):
            raise ValueError(f"inputs changed during preparation publication; preserve edits; inspect {directory}")
        report.update(published=True, database_sha256=hashlib.sha256(prepared).hexdigest(),
                      entries=len(parsed), reason="fresh compiler recipes published; generated context retained")
        report["limitations"] = [
            "trusted configured generator; recipe syntax and freshness, not intended TU completeness or compiler correctness",
            "generated context is an ignored disposable cache; regenerate after removing it",
            "no build cache is deleted; preparation cannot run concurrently with a measurement epoch",
        ]
        return report
