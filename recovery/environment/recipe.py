"""VC6 recipe selection and parse flags from this project's CMake database."""

from pathlib import Path
import shlex

from match_recovery.compilation_database import CompilationEntry, read_database
from match_recovery.records import read_json
from match_recovery.source_config import SourceConfiguration


class MissingRecipe(ValueError):
    """Only absence permits an explicitly chosen donor, not a malformed recipe."""


def application_command(root: Path, source: Path) -> list[str]:
    """Use the product recipe, or this project's explicit seed-only donor.

    The donor reproduces the old sweep's chosen include/define environment;
    it does not claim to recover an original translation-unit recipe. Product
    files and malformed/conflicting database entries can never use this path.
    """
    database = root / "build/compile_commands.json"
    try:
        return build_command(database, source)
    except MissingRecipe:
        settings = SourceConfiguration.read(root)
        if not settings.is_seed(source.relative_to(root).as_posix()):
            raise
        policy = read_json(root / "recovery/environment/recipes.json")
        if (set(policy) != {"schema_version", "seed_donor"}
                or type(policy["schema_version"]) is not int or policy["schema_version"] != 1):
            raise ValueError("invalid project seed compiler recipe policy")
        donor = settings.path(root, policy["seed_donor"])
        if settings.is_seed(policy["seed_donor"]):
            raise ValueError("seed recipe donor must be a product translation unit")
        return build_command(database, donor)


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
    Runtime switches also affect predefines and declaration/link semantics;
    preserve them rather than silently returning to the compiler's default CRT.
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
        if word.startswith("--frontend="):
            if word != "--frontend=intrinsics-i386-no-debug":
                raise ValueError("unsupported explicit compiler frontend")
            kept.append(word)
            continue
        if word.startswith("--repro-root="):
            path = Path(word.split("=", 1)[1])
            if not path.is_absolute() or path.resolve() != path or not path.is_dir():
                raise ValueError("matching recipe requires a canonical reproducible compiler root")
            kept.append(word)
            continue
        if word.startswith("@"):
            raise ValueError("matching recipe requires explicit options, not response files")
        if word.startswith(("/", "-")) and word[1:] in {"MD", "MDd", "MT", "MTd", "ML", "MLd"}:
            kept.append("/" + word[1:])
            continue
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
