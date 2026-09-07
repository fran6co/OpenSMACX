"""VC6 recipe selection and parse flags from this project's CMake database."""

from pathlib import Path
import shlex

from match_recovery.compilation_database import CompilationEntry, read_database


class MissingRecipe(ValueError):
    """Only absence permits an explicitly chosen donor, not a malformed recipe."""


def flag_dependent(diagnostic: str) -> bool:
    """Retry optimizer/internal-memory failures, not repeated VC6 parse errors.

    The historical sweep found identical parse diagnostics across flag trials.
    C1001 (internal error) and C1060 (memory) can still depend on optimization.
    """
    return any(code in diagnostic for code in ("C1001", "C1060"))


def build_command(database: Path, source: Path) -> list[str]:
    """Select one explicit recipe; missing or conflicting entries are refused.

    Seeds require a caller-chosen donor recipe. No nearest-address or filename
    heuristic chooses their include environment.
    """
    source = Path(source).resolve()
    matches = [entry for entry in read_database(database) if entry.file == source]
    if not matches:
        raise MissingRecipe(f"{source.name} is not a build input in {Path(database).name}; "
                         "pass the command of a file that is")
    recipes = [invocation(entry) for entry in matches]
    if any(recipe != recipes[0] for recipe in recipes[1:]):
        raise ValueError(f"conflicting compilation recipes for {source}")
    return recipes[0]


def invocation(entry: CompilationEntry) -> list[str]:
    """Keep VC6 parse flags, resolve includes before using a private output dir.

    CMake's POSIX command strings use shell quoting; argv arrays are already
    tokenized. Matching trials supply optimization/debug flags separately.
    Precompiled headers and build output flags cannot be reused here.
    """
    words = list(entry.arguments) if entry.arguments is not None else shlex.split(entry.command)
    if not words:
        raise ValueError("empty compiler invocation")
    command = words[0]
    if "/" in command and not Path(command).is_absolute():
        command = str((entry.directory / command).resolve())
    kept, iterator = [], iter(words[1:])
    for word in iterator:
        if word.startswith("@"):
            raise ValueError("matching recipe requires explicit options, not response files")
        if word.startswith(("/Fo", "/Fd", "/Fp", "/Yu", "/Yc")):
            if word in ("/Fo", "/Fd", "/Fp", "/Yu", "/Yc"):
                if not next(iterator, ""):
                    raise ValueError(f"missing value for VC6 {word}")
            continue
        if word.startswith(("-I", "-D", "-FI")):
            word = "/" + word[1:]
        prefix = next((prefix for prefix in ("/FI", "/I", "/D") if word.startswith(prefix)), None)
        if prefix is None:
            if not word.startswith(("/", "-")) or Path(word).is_absolute() and Path(word).is_file():
                if (entry.directory / word).resolve() != entry.file:
                    raise ValueError("matching recipe requires a direct compiler invocation, not a launcher chain")
            continue
        value = word[len(prefix):] or next(iterator, "")
        if not value:
            raise ValueError(f"missing value for VC6 {prefix}")
        if prefix == "/FI" and Path(value).name.startswith("cmake_pch"):
            continue
        if prefix != "/D" and not Path(value).is_absolute() and not (len(value) > 1 and value[1] == ":"):
            value = str((entry.directory / value).resolve())
        kept.append(prefix + value)
    return [command, "/nologo", *kept]
