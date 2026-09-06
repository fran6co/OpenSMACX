"""Project-owned inputs to the reusable ORIGINAL-comment source reader."""

from dataclasses import dataclass, fields
from pathlib import Path, PurePosixPath
import re

from .components import relative_path
from .model import require_text
from .records import read_json


@dataclass(frozen=True)
class SourceConfiguration:
    image_pin: str
    address_space: str
    roots: tuple[str, ...] = ("src",)
    extensions: tuple[str, ...] = (".cpp", ".c", ".h", ".cc", ".cxx", ".hpp", ".hh")
    seed_roots: tuple[str, ...] = ()
    include_paths: tuple[str, ...] = ("src",)
    interface_extensions: tuple[str, ...] = (".h", ".hpp", ".hh")
    symbol_format: str = "none"

    def __post_init__(self):
        relative_path(self.image_pin)
        require_text(self.address_space, "source address space")
        if self.symbol_format not in ("none", "msvc"):
            raise ValueError("unsupported source symbol hint format")
        for name in ("roots", "extensions", "seed_roots", "include_paths", "interface_extensions"):
            values = getattr(self, name)
            if not isinstance(values, tuple) or any(not isinstance(value, str) for value in values):
                raise ValueError(f"source {name} must be a sequence of strings")
            if len(set(values)) != len(values):
                raise ValueError(f"duplicate source {name}")
            for value in values:
                if name in ("extensions", "interface_extensions"):
                    if not re.fullmatch(r"\.[A-Za-z0-9]+", value):
                        raise ValueError("source extensions must be literal suffixes")
                else:
                    relative_path(value)
        if not self.roots or not self.extensions:
            raise ValueError("source roots and extensions must not be empty")
        for index, root in enumerate(self.roots):
            if any(PurePosixPath(root).is_relative_to(other) or PurePosixPath(other).is_relative_to(root)
                   for other in self.roots[:index]):
                raise ValueError("source roots must not overlap")
        if any(not any(PurePosixPath(seed).is_relative_to(root) for root in self.roots)
               for seed in self.seed_roots):
            raise ValueError("seed roots must be within configured source roots")

    @classmethod
    def from_dict(cls, value):
        if not isinstance(value, dict) or not {"image_pin", "address_space"} <= value.keys():
            raise ValueError("source configuration needs image_pin and address_space")
        if value.keys() - {field.name for field in fields(cls)}:
            raise ValueError("unknown source configuration field")
        data = dict(value)
        for name in ("roots", "extensions", "seed_roots", "include_paths", "interface_extensions"):
            if name in data:
                if not isinstance(data[name], (list, tuple)):
                    raise ValueError(f"source {name} must be a sequence of strings")
                data[name] = tuple(data[name])
        return cls(**data)

    @classmethod
    def read(cls, root: Path):
        path = root / "recovery/project.json"
        if not path.resolve().is_relative_to(root.resolve()):
            raise ValueError("source configuration resolves outside the worktree")
        configuration = read_json(path)
        if type(configuration.get("schema_version")) is not int or configuration["schema_version"] != 1:
            raise ValueError("unsupported project configuration version")
        return cls.from_dict(configuration.get("source"))

    def path(self, root: Path, relative: str) -> Path:
        relative_path(relative)
        path = root / relative
        if not path.resolve().is_relative_to(root.resolve()):
            raise ValueError(f"source input resolves outside the worktree: {relative}")
        return path

    def is_seed(self, relative: str) -> bool:
        return any(PurePosixPath(relative).is_relative_to(seed) for seed in self.seed_roots)

    def files(self, root: Path) -> list[Path]:
        paths = []
        for name in self.roots:
            directory = self.path(root, name)
            if not directory.is_dir():
                raise ValueError(f"source root is missing or not a directory: {name}")
            for path in directory.rglob("*"):
                if path.is_file() and path.suffix in self.extensions:
                    self.path(root, path.relative_to(root).as_posix())
                    paths.append(path)
        return sorted(paths)
