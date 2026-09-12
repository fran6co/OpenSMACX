"""Fresh generator/compiler context for one complete counterfactual source tree."""

from contextlib import contextmanager
from collections.abc import Callable
from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path

from .build_check import Build
from .compiler import Compiler
from .compilation_database import entries
from .measurement import Measurement
from .preparation import prepare_project
from .project_replica import project_replica
from .records import read_json, parse_json_value
from .source_replica import replica_roots


@dataclass(frozen=True)
class PreparedCompiler:
    """Fresh recipe membership, not proof that the application links or works."""

    compiler: Compiler
    units: frozenset[str]
    check_current: Callable[[], None]

    @property
    def root(self):
        return self.compiler.root

    def __call__(self, source, arguments, flags):
        self.check_current()
        source = Path(source)
        if (not source.is_relative_to(self.root) or source.resolve() != source
                or source.relative_to(self.root).as_posix() not in self.units):
            raise ValueError("source has no unique freshly prepared recipe")
        try:
            return self.compiler(source, arguments, flags)
        finally:
            self.check_current()


def source_roots(root, settings):
    """Historical inputs exclude generated context only when it is regenerated."""
    policy = read_json(root / "recovery/project.json")
    preparation = Build.read(root, key="preparation")
    roots = replica_roots((*settings.source.roots, *policy["measurement"]["input_roots"], *preparation.input_roots))
    if any(Path(preparation.artifacts).is_relative_to(name) or Path(name).is_relative_to(preparation.artifacts) for name in roots):
        raise ValueError("preparation artifacts must be separate from historical input roots")
    return roots


@contextmanager
def prepared_compiler(root, roots, files, settings, *, versioned_files=None):
    project = read_json(root / "recovery/project.json")
    preparation = Build.read(root, key="preparation")
    database = project["mappings"]["compile_commands"]
    inputs = tuple(name for name in settings.inputs if name != database
                   and not Path(name).is_relative_to(preparation.artifacts))
    if versioned_files is None:
        versioned_files = {}
        for name in preparation.versioned_inputs:
            path = root / name
            if not path.is_file() or path.resolve() != path or path.stat().st_nlink != 1:
                raise ValueError("versioned project input must be a canonical unaliased regular file: " + name)
            versioned_files[name] = path.read_bytes()
    with project_replica(root, roots, files, inputs=inputs, versioned_inputs=preparation.versioned_inputs,
                         versioned_files=versioned_files) as replica:
        report = prepare_project(replica)
        if report["result"] != "pass" or not report["published"]:
            # A generator failure is not a compiler #error damage witness.
            raise ValueError("counterfactual compiler preparation failed; generated context is unverified")
        database_path = replica / database
        if not database_path.is_file() or database_path.resolve() != database_path:
            raise ValueError("prepared compilation database must be a canonical regular file")
        database_before = database_path.read_bytes()

        def recipe_units(data, directory):
            rows = entries(data, directory)
            if any(not row.file.is_relative_to(replica) or not row.directory.is_relative_to(replica) for row in rows):
                raise ValueError("prepared recipes must refer to the private project context")
            units = []
            for row, raw in zip(rows, parse_json_value(data), strict=True):
                origin = directory / raw["directory"]
                source = origin / raw["file"]
                if (origin != row.directory or source != row.file or origin.resolve() != origin or source.resolve() != source
                        or not row.directory.is_dir() or not row.file.is_file()):
                    raise ValueError("prepared recipe needs a canonical existing source and directory")
                units.append(row.file.relative_to(replica).as_posix())
            if len(set(units)) != len(units):
                raise ValueError("duplicate prepared translation unit recipe")
            return frozenset(units)

        units = recipe_units(database_before, database_path.parent)
        # Publication makes identities absolute/resolved; inspect the actual
        # generator's spellings too, so that normalization cannot hide aliases.
        generated_directory = replica / report["directory"]
        generated = generated_directory / preparation.outputs[0][0]
        if not generated.is_file() or generated.resolve() != generated:
            raise ValueError("generated recipe evidence is unavailable or aliased")
        generated_bytes = generated.read_bytes()
        if (sha256(generated_bytes).hexdigest() != report["outputs"][0]["sha256"]
                or recipe_units(generated_bytes, generated_directory) != units):
            raise ValueError("generated recipe evidence changed after preparation")
        current = Measurement.read(replica)
        before = current.snapshot(replica, {})

        def fresh():
            if (not database_path.is_file() or database_path.resolve() != database_path
                    or database_path.read_bytes() != database_before or current.snapshot(replica, {}) != before):
                raise ValueError("prepared compiler context changed during preservation")

        try:
            fresh()
            yield PreparedCompiler(Compiler(replica, profile=settings.compiler_profile), units, fresh)
        finally:
            fresh()
