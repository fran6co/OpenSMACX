"""Compiler-neutral compilation database identities and lossless argv.

Command-string quoting and compiler option semantics belong to the compiler
environment. In particular, an arguments array must never be shell-split again.
Entries describe intended recipes, not proof that a unit was built.
"""

from dataclasses import dataclass
from pathlib import Path

from .records import parse_json_value


@dataclass(frozen=True)
class CompilationEntry:
    file: Path
    directory: Path
    arguments: tuple[str, ...] | None
    command: str | None


def entries(data: bytes, root: Path) -> tuple[CompilationEntry, ...]:
    rows = parse_json_value(data)
    if not isinstance(rows, list) or not rows:
        raise ValueError("compilation database must be a nonempty array")
    result = []
    for row in rows:
        if (not isinstance(row, dict) or any(not isinstance(row.get(key), str)
                or not row[key] or "\0" in row[key] for key in ("file", "directory"))):
            raise ValueError("compilation database entries require file and directory")
        arguments, command = row.get("arguments"), row.get("command")
        if command is not None and (not isinstance(command, str) or not command or "\0" in command):
            raise ValueError("invalid compilation database command")
        if arguments is not None:
            if (not isinstance(arguments, list) or not arguments
                    or any(not isinstance(word, str) or not word or "\0" in word for word in arguments)):
                raise ValueError("invalid compilation database arguments")
        elif command is None:
            raise ValueError("compilation database entry has no compiler invocation")
        directory = (root / row["directory"]).resolve()
        result.append(CompilationEntry((directory / row["file"]).resolve(), directory,
                                       tuple(arguments) if arguments is not None else None, command))
    return tuple(result)


def read_database(path: Path, *, root: Path | None = None) -> tuple[CompilationEntry, ...]:
    path = Path(path)
    if not path.is_file():
        raise ValueError(f"no {path} - configure the build first")
    return entries(path.read_bytes(), root if root is not None else path.resolve().parent)


def build_inputs(path: Path, *, root: Path | None = None) -> set[Path]:
    return {entry.file for entry in read_database(path, root=root)}
