"""Fresh generator/compiler context for one complete counterfactual source tree."""

from contextlib import contextmanager
from pathlib import Path

from .build_check import Build
from .compiler import Compiler
from .compilation_database import read_database
from .measurement import Measurement
from .preparation import prepare_project
from .project_replica import project_replica
from .records import read_json
from .source_replica import replica_roots


def source_roots(root, settings):
    """Historical inputs exclude generated context only when it is regenerated."""
    policy = read_json(root / "recovery/project.json")
    preparation = Build.read(root, key="preparation")
    roots = replica_roots((*settings.source.roots, *policy["measurement"]["input_roots"], *preparation.input_roots))
    if any(Path(preparation.artifacts).is_relative_to(name) or Path(name).is_relative_to(preparation.artifacts) for name in roots):
        raise ValueError("preparation artifacts must be separate from historical input roots")
    return roots


@contextmanager
def prepared_compiler(root, roots, files, settings):
    project = read_json(root / "recovery/project.json")
    preparation = Build.read(root, key="preparation")
    database = project["mappings"]["compile_commands"]
    inputs = tuple(name for name in settings.inputs if name != database
                   and not Path(name).is_relative_to(preparation.artifacts))
    with project_replica(root, roots, files, inputs=inputs) as replica:
        report = prepare_project(replica)
        if report["result"] != "pass" or not report["published"]:
            # A generator failure is not a compiler #error damage witness.
            raise ValueError("counterfactual compiler preparation failed; generated context is unverified")
        rows = read_database(replica / database)
        if any(not row.file.is_relative_to(replica) or not row.directory.is_relative_to(replica) for row in rows):
            raise ValueError("prepared recipes must refer to the private project context")
        current = Measurement.read(replica)
        before = current.snapshot(replica, {})
        try:
            yield Compiler(replica, profile=settings.compiler_profile)
        finally:
            if current.snapshot(replica, {}) != before:
                raise ValueError("prepared compiler context changed during preservation")
