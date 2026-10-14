"""Project-owned measurement policy and conservative live input fingerprints."""

from dataclasses import asdict, dataclass
import hashlib
import os
from pathlib import Path
import re

from .components import relative_path
from .driver import Command
from .fingerprint import input_snapshot
from .model import digest, require_text
from .records import read_json
from .source_config import SourceConfiguration
from .toolchain import snapshot as toolchain_snapshot


@dataclass(frozen=True)
class Measurement:
    backend: str
    compiler_profile: str
    flag_sets: tuple[tuple[str, ...], ...]
    layout_flags: tuple[str, ...]
    unit_extensions: tuple[str, ...]
    inputs: tuple[str, ...]
    input_roots: tuple[str, ...]
    source: SourceConfiguration
    environment: tuple[str, ...]
    unit_flag_sets: tuple[tuple[str, tuple[int, ...]], ...] = ()
    unit_explicit_flags: tuple[tuple[str, tuple[str, ...]], ...] = ()

    @classmethod
    def read(cls, root: Path):
        project = read_json(root / "recovery/project.json")
        source = SourceConfiguration.read(root)
        value = project.get("measurement")
        fields = {"backend", "compiler_profile", "flag_sets", "layout_flags", "unit_extensions", "inputs", "input_roots"}
        if not isinstance(value, dict) or set(value) not in (fields, fields | {"unit_flag_sets"}):
            raise ValueError("measurement configuration requires backend, compiler_profile, flag_sets, layout_flags, unit_extensions, inputs and input_roots")
        if value["backend"] != "pe-coff-i386":
            raise ValueError("unsupported measurement backend")
        if source.address_space != "pe:va":
            raise ValueError("PE/COFF measurement requires the pe:va address space")
        require_text(value["compiler_profile"], "compiler profile")
        for key in ("flag_sets", "layout_flags", "unit_extensions", "inputs", "input_roots"):
            if not isinstance(value[key], list):
                raise ValueError(f"measurement {key} must be an array")
        if not value["flag_sets"] or not value["unit_extensions"]:
            raise ValueError("measurement needs flag trials and translation-unit suffixes")
        for flags in value["flag_sets"]:
            if not isinstance(flags, list):
                raise ValueError("each flag trial must be an array")
            for flag in flags:
                require_text(flag, "compiler flag")
        for flag in value["layout_flags"]:
            require_text(flag, "layout compiler flag")
        for extension in value["unit_extensions"]:
            if not isinstance(extension, str) or not re.fullmatch(r"\.[A-Za-z0-9]+", extension):
                raise ValueError("unit extensions must be literal suffixes")
            if extension not in source.extensions:
                raise ValueError("unit extension is absent from configured source selection")
        for path in (*value["inputs"], *value["input_roots"]):
            relative_path(path)
        compiler = project.get("compiler", {})
        if "compiler" in project:
            if (not isinstance(compiler, dict) or set(compiler) != {"command", "inputs", "environment"}
                    or not isinstance(compiler["inputs"], list) or not isinstance(compiler["environment"], list)):
                raise ValueError("invalid measurement compiler configuration")
            Command.read(compiler["command"])
            for path in compiler["inputs"]:
                relative_path(path)
            for name in compiler["environment"]:
                if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name):
                    raise ValueError("invalid measurement environment variable")
        environment = {"PATH", *compiler.get("environment", [])}
        for argument in compiler.get("command", {}).get("argv", []):
            environment.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", argument))
        for name in environment:
            if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name):
                raise ValueError("invalid measurement environment variable")
        inputs = tuple(sorted(set(value["inputs"]) | set(compiler.get("inputs", []))))
        roots = set(value["input_roots"])
        if "preparation" in project:
            from .build_check import Build
            preparation = Build.read(root, key="preparation")
            # Recipe generation can supply forced includes or other compiler
            # context. Pin its policy and generated contents, not just the JSON
            # database. Conservative whole-cache invalidation is intentional;
            # an unobserved generated header must never lend stale evidence.
            inputs = tuple(sorted({*inputs, preparation.path, *preparation.inputs}))
            roots.update((*preparation.input_roots, preparation.artifacts))
            environment.update(preparation.environment)
        for path in inputs:
            relative_path(path)
        flag_sets = tuple(tuple(flags) for flags in value["flag_sets"])
        unit_flag_sets = value.get("unit_flag_sets", {})
        if not isinstance(unit_flag_sets, dict):
            raise ValueError("measurement unit_flag_sets must be an object")
        normalized_units = []
        normalized_explicit_flags = []
        folded_paths = set()
        source_roots = tuple(source.roots)
        for path, selection in unit_flag_sets.items():
            relative_path(path)
            if (not any(Path(path).is_relative_to(name) for name in source_roots)
                    or Path(path).suffix not in value["unit_extensions"]
                    or Path(path).suffix in source.interface_extensions):
                raise ValueError(f"unit_flag_sets path is not a configured source translation unit: {path}")
            if path.casefold() in folded_paths:
                raise ValueError("unit_flag_sets contains duplicate or aliased paths")
            folded_paths.add(path.casefold())
            unit_path = source.path(root, path)
            if not unit_path.is_file():
                raise ValueError(f"unit_flag_sets path does not name an existing source file: {path}")
            resolved_name = unit_path.resolve().relative_to(root.resolve()).as_posix()
            if unit_path.is_symlink() or resolved_name != path:
                raise ValueError(f"unit_flag_sets path is not a canonical source file: {path}")
            if isinstance(selection, dict):
                if set(selection) != {"flags"}:
                    raise ValueError(f"unit_flag_sets[{path!r}] explicit recipe must contain only flags")
                flags = selection["flags"]
                if not isinstance(flags, list) or not flags:
                    raise ValueError(f"unit_flag_sets[{path!r}].flags must select a nonempty recipe")
                for flag in flags:
                    require_text(flag, "compiler flag")
                normalized_explicit_flags.append((path, tuple(flags)))
                continue
            indices = selection
            if not isinstance(indices, list) or not indices:
                raise ValueError(f"unit_flag_sets[{path!r}] must select at least one flag-set index")
            if len(indices) != 1:
                raise ValueError(f"unit_flag_sets[{path!r}] must select exactly one compiler profile")
            if any(type(index) is not int or index < 0 or index >= len(flag_sets) for index in indices):
                raise ValueError(f"unit_flag_sets[{path!r}] contains an invalid flag-set index")
            if len(indices) != len(set(indices)):
                raise ValueError(f"unit_flag_sets[{path!r}] contains duplicate selections")
            normalized_units.append((path, tuple(indices)))
        return cls(value["backend"], value["compiler_profile"], flag_sets,
                   tuple(value["layout_flags"]), tuple(value["unit_extensions"]), inputs,
                   tuple(sorted(roots)), source, tuple(sorted(environment)),
                   tuple(sorted(normalized_units)), tuple(sorted(normalized_explicit_flags)))

    def flags_for(self, path):
        """Return this TU's configured trials, or all global trials if unpinned."""
        name = Path(path).as_posix()
        explicit = dict(self.unit_explicit_flags).get(name)
        if explicit is not None:
            return (explicit,)
        selected = dict(self.unit_flag_sets).get(name)
        return self.flag_sets if selected is None else tuple(self.flag_sets[index] for index in selected)

    def witness_grid(self, paths, suffix=()):
        """Serialize the exact per-TU trial set, including a common suffix."""
        return {Path(path).as_posix(): [list(flags + tuple(suffix)) for flags in self.flags_for(path)]
                for path in sorted({Path(item).as_posix() for item in paths})}

    def effective_flags(self, path, requested):
        """Select a pinned TU profile or retain a requesting graph's profile."""
        name = Path(path).as_posix()
        explicit = dict(self.unit_explicit_flags).get(name)
        if explicit is not None:
            return explicit
        selected = dict(self.unit_flag_sets).get(name)
        if selected is None:
            return tuple(requested)
        return self.flag_sets[selected[0]]

    def snapshot(self, root: Path, metadata: dict):
        paths = {"recovery/project.json", self.source.image_pin, *self.inputs}
        root_name = os.fspath(root)
        # Root inventories are re-enumerated, so new/shadowing headers invalidate
        # a check too. No dependency narrowing or persistent verdict cache here.
        for name in dict.fromkeys((*self.source.roots, *self.input_roots)):
            directory = self.source.path(root, name)
            for parent, _, names in os.walk(directory):
                for filename in names:
                    path = os.path.join(parent, filename)
                    if os.path.isfile(path):
                        # Like rglob, do not descend through directory links.
                        # Configured root aliases retain their logical names;
                        # input_snapshot still resolves and hashes every file.
                        paths.add(os.path.relpath(path, root_name).replace(os.sep, "/"))
        package = Path(__file__).parent
        implementation = digest({Path(os.path.relpath(path, package)).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                                 for path in sorted(package.rglob("*.py"))})
        environment = {name: hashlib.sha256(os.environ[name].encode()).hexdigest() if name in os.environ else None
                       for name in self.environment}
        return input_snapshot(root, paths, {"measurement": asdict(self), "implementation": implementation,
                                            "environment": environment, **metadata, "toolchain": toolchain_snapshot(root)})
