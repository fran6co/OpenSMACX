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

    @classmethod
    def read(cls, root: Path):
        project = read_json(root / "recovery/project.json")
        source = SourceConfiguration.read(root)
        value = project.get("measurement")
        fields = {"backend", "compiler_profile", "flag_sets", "layout_flags", "unit_extensions", "inputs", "input_roots"}
        if not isinstance(value, dict) or set(value) != fields:
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
        return cls(value["backend"], value["compiler_profile"], tuple(tuple(flags) for flags in value["flag_sets"]),
                   tuple(value["layout_flags"]), tuple(value["unit_extensions"]), inputs,
                   tuple(sorted(roots)), source, tuple(sorted(environment)))

    def snapshot(self, root: Path, metadata: dict):
        paths = {"recovery/project.json", self.source.image_pin, *self.inputs}
        # Root inventories are re-enumerated, so new/shadowing headers invalidate
        # a check too. No dependency narrowing or persistent verdict cache here.
        for name in (*self.source.roots, *self.input_roots):
            directory = self.source.path(root, name)
            for path in directory.rglob("*"):
                if path.is_file():
                    # Enumeration starts at a validated project-local root.
                    # relpath avoids repeatedly constructing ancestor Paths;
                    # input_snapshot still resolves and hashes every input.
                    paths.add(Path(os.path.relpath(path, root)).as_posix())
        package = Path(__file__).parent
        implementation = digest({Path(os.path.relpath(path, package)).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                                 for path in sorted(package.rglob("*.py"))})
        environment = {name: hashlib.sha256(os.environ[name].encode()).hexdigest() if name in os.environ else None
                       for name in self.environment}
        return input_snapshot(root, paths, {"measurement": asdict(self), "implementation": implementation,
                                            "environment": environment, **metadata})
