"""Configured production recording; historical byte obligations, not acceptance.

The installed command owns selection, input capture, measurement and publication.
Compiler commands/recipes stay behind the project's compiler protocol. No report
or object from an earlier invocation can be submitted as recording evidence.
"""

from concurrent.futures import ProcessPoolExecutor
import hashlib
from pathlib import Path
import re
import subprocess

from .claim_integrity import check_claims
from .compiler import Compiler
from .components import relative_path
from .measurement import Measurement
from .model import digest, require_text
from .records import parse_json_value
from .source import read_file
from .source.asm import compare_unit, shared_spans
from .source.mangled import qualified_name
from .source.recording import record_claims
from .source_history import git
from .source_inventory import image_pin


def _unit(job):
    root, profile, path, records, image, arguments, flags, shared = job
    return compare_unit(records, image, path, arguments, flags, shared,
                        compiler=Compiler(root, profile=profile))


def _select(records, targets, host, symbol_format):
    selected = []
    for target in targets:
        require_text(target, "recording target")
        candidates = [row for row in records if host is None or row.path == host]
        matches = [row for row in candidates if target in (row.name, row.symbol)
                   or symbol_format == "msvc" and target == qualified_name(row.name)]
        # Ordinary C++ names such as f/add/face are also hexadecimal strings.
        # Explicit 0x selects an address; otherwise exact names take precedence.
        if (re.fullmatch(r"0[xX][0-9a-fA-F]+", target)
                or not matches and re.fullmatch(r"[0-9a-fA-F]+", target)):
            matches = [row for row in candidates if row.address == int(target, 16)]
        if len(matches) != 1:
            locations = ", ".join(f"{row.path}:{row.line}" for row in matches)
            raise ValueError(f"recording target {target!r} requires one source mapping; "
                             f"found {len(matches)}" + (f": {locations}; select an exact --in path" if matches else ""))
        selected.append(matches[0])
    if len({(row.path, row.line) for row in selected}) != len(selected):
        raise ValueError("the same source mapping was selected more than once")
    return selected


def record(root: Path, targets, *, image: Path, in_file: str | None = None,
           arguments_file: Path | None = None, jobs: int = 1) -> tuple[int, dict]:
    """Measure selected existing annotations and publish only byte-exact tokens.

    The PE/COFF backend currently uses the historical relocation-masked comparer.
    Current strict reference, structure, preservation and full acceptance checks
    are separate. Existing obligations survive regressions; no automatic demotion.
    """
    root = root.resolve()
    if not isinstance(targets, (tuple, list)) or not targets:
        raise ValueError("recording requires an explicit nonempty target selection")
    if type(jobs) is not int or jobs < 1:
        raise ValueError("recording jobs must be a positive integer")
    settings = Measurement.read(root)
    image = image if image.is_absolute() else root / image
    image = image.resolve()
    arguments_path = None
    if arguments_file is not None:
        relative_path(arguments_file.as_posix())
        arguments_path = settings.source.path(root, arguments_file.as_posix())
    staging = root / "build/recovery/publications"

    def inputs():
        # Image may live outside the project, but it must always be the pinned
        # original. Neither caller selection nor a changed file can repin it.
        with image.open("rb") as stream:
            actual = hashlib.file_digest(stream, "sha256").hexdigest()
        if actual != image_pin(root, settings.source):
            raise ValueError("recording requires the configured pinned original image")
        snapshot = settings.snapshot(root, {
            "head": git(root, "rev-parse", "HEAD").decode().strip(),
            "image": {"path": str(image), "sha256": actual},
            "arguments_file": (hashlib.sha256(arguments_path.read_bytes()).hexdigest()
                               if arguments_path is not None else None)})
        snapshot["configuration"]["resolved_inputs"] = {
            name: str((root / name).resolve()) for name in snapshot["inputs"]}
        return snapshot

    initial = inputs()
    if any(value is None for value in initial["inputs"].values()):
        raise ValueError("declared recording input is missing")
    for name in (*settings.source.roots, *settings.input_roots):
        directory = settings.source.path(root, name)
        if not directory.is_dir():
            raise ValueError(f"recording input root is missing: {name}")
        if staging.is_relative_to(directory) or directory.is_relative_to(staging):
            raise ValueError("publication staging overlaps a measured input root")
    if any((root / name).is_relative_to(staging) for name in initial["inputs"]):
        raise ValueError("publication staging overlaps a measured input")

    def staging_policy():
        ignored = subprocess.run(["git", "check-ignore", "-q", "build/recovery/publications/"],
                                 cwd=root, capture_output=True)
        if (ignored.returncode != 0 or git(root, "ls-files", "--", "build/recovery/publications/").strip()
                or staging.resolve() != staging):
            raise ValueError("publication staging must be ignored, untracked and unaliased")

    staging_policy()
    records = [row for path in settings.source.files(root) for row in read_file(path)]
    coverage = check_claims(root)
    if coverage["result"] != "pass":
        missing = ", ".join(row["address"] + "/" + row["dimension"] for row in coverage["missing"])
        raise ValueError("historical source obligations are missing: " + missing)
    host = None
    if in_file is not None:
        relative_path(in_file)
        host = settings.source.path(root, in_file)
    selected = _select(records, targets, host, settings.source.symbol_format)
    if any(row.path.suffix not in settings.unit_extensions for row in selected):
        raise ValueError("selected mapping host is not a configured translation unit")
    arguments = ()
    if arguments_path is not None:
        values = parse_json_value(arguments_path.read_bytes())
        if not isinstance(values, list):
            raise ValueError("compiler arguments file must contain a JSON string array")
        for value in values:
            require_text(value, "compiler argument")
            if "\0" in value:
                raise ValueError("compiler arguments cannot contain NUL")
        arguments = tuple(values)
    shared = shared_spans(records)
    groups = {}
    for row in selected:
        groups.setdefault(row.path, []).append(row)
    work = [(root, settings.compiler_profile, path, group, image, arguments, settings.flag_sets, shared)
            for path, group in groups.items()]
    measurements = []

    def validate():
        if inputs() != initial:
            raise ValueError("source, image, history or compiler inputs changed; no claims written")
        staging_policy()
        if arguments_path is not None and arguments_path.resolve() != arguments_path:
            raise ValueError("compiler arguments input is aliased")

    def measure():
        if jobs == 1 or len(work) == 1:
            measurements.extend(_unit(job) for job in work)
        else:
            with ProcessPoolExecutor(max_workers=min(jobs, len(work))) as pool:
                measurements.extend(pool.map(_unit, work))
        return [row for unit in measurements for row in unit.results]

    publication = record_claims(selected, measure=measure, validate=validate, staging_directory=staging)
    def location(row):
        return {"address": row.address_hex, "path": row.path.relative_to(root).as_posix(),
                "line": row.line, "symbol": row.symbol or row.name}
    measured = [{**location(row), "verdict": result.verdict.value, "flags": result.flags,
                 "matching_instructions": result.matching_instructions,
                 "original_instructions": result.original_instructions}
                for row, result in publication.measured]
    return publication.exit_code, {
        "schema_version": 1, "operation": "record", "exit_code": publication.exit_code,
        "result": "pass" if publication.exit_code == 0 else "fail" if publication.exit_code == 1 else "unknown",
        "verification": "historical_masked_bytes_only", "ready_for_collection": False,
        "selected": len(selected), "measured": measured,
        "unresolved": [{**location(row), "diagnostic": why} for row, why in publication.unresolved],
        "regressed": [location(row) for row, _ in publication.regressed],
        "updated": [location(row) for row in publication.updated],
        "changed_files": [path.relative_to(root).as_posix() for path in publication.changed_files],
        "translation_units": len(measurements),
        "compile_attempts": sum(unit.compile_attempts for unit in measurements),
        "reused_attempts": sum(unit.reused_attempts for unit in measurements),
        "input_fingerprint": digest(initial)}
