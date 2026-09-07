"""Whole-source historical match obligations through configured compilation.

This is the current-measurement half of claim verification. Source-history
identity checks, strict references, structural checks and linking are separate.
Seeds remain evidence candidates, but their historical claims are never omitted.
Nothing here banks, demotes, edits, or reuses a previous invocation's verdicts.
"""

from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
import hashlib
from pathlib import Path

from .compiler import Compiler
from .mapping_check import build_units
from .measurement import Measurement
from .model import digest
from .records import read_json
from .source import read_file
from .source.asm import AsmComparison, compare_unit, shared_spans
from .source.model import Tier
from .source_history import git
from .source_inventory import image_pin


def _unit(job):
    root, profile, path, records, image, flags, shared = job
    return compare_unit(records, image, path, (), flags, shared,
                        compiler=Compiler(root, profile=profile))


def _key(record):
    # Different source/seed attempts and repeated markers must not overwrite
    # each other. Source identity/host consistency is also checked separately.
    return record.path, record.address, record.line


def _observation(root, record, observed, *, built, source):
    row = {"address": record.address_hex, "path": record.path.relative_to(root).as_posix(),
           "line": record.line, "name": record.name, "symbol": record.symbol or record.name,
           "byte_obligation": record.byte_exact, "shape_obligation": record.semantic}
    row["seed"] = source.is_seed(row["path"])
    row["built"] = row["path"] in built
    if isinstance(observed, str):
        row.update(verdict="UNRESOLVED", diagnostic=observed)
        held, unmeasured = False, True
    else:
        if not isinstance(observed, AsmComparison):
            raise ValueError("invalid unit comparison observation")
        row.update(verdict=str(observed.verdict), flags=observed.flags,
                   diagnostic=observed.diagnostic,
                   matching_instructions=observed.matching_instructions,
                   original_instructions=observed.original_instructions,
                   compiled_instructions=observed.compiled_instructions,
                   mnemonic_similarity=observed.mnemonic_similarity,
                   allocation_only=observed.allocation_only == "")
        # A weaker shape claim cannot satisfy a simultaneous byte obligation.
        held = observed.verdict == Tier.BYTE_EXACT or (
            not record.byte_exact and record.semantic and observed.allocation_only == "")
        unmeasured = observed.verdict in (Tier.NO_COMPILE, Tier.SHARED_TAIL, Tier.REFUSED)
    claimed = record.byte_exact or record.semantic
    row["result"] = ("pass" if held else "fail" if not unmeasured or row["built"] else "unknown") if claimed else None
    return row


def check_matches(root: Path, image: Path, *, jobs: int = 1) -> dict:
    """Ask every current claim and discover unclaimed matches in product TUs.

    Compiler recipe selection stays behind the configured compiler boundary.
    Missing seed recipes are unknown, not an implicit borrowed environment.
    A built claim that loses its compiled subject fails the obligation check;
    unsupported non-TU hosts remain visible and unmeasurable.
    """
    root = root.resolve()
    if type(jobs) is not int or jobs < 1:
        raise ValueError("match-check jobs must be a positive integer")
    settings = Measurement.read(root)
    image = (image if image.is_absolute() else root / image).resolve()
    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("match checking requires a configured compilation database")
    database = settings.source.path(root, mappings["compile_commands"])

    def inputs():
        with image.open("rb") as stream:
            actual = hashlib.file_digest(stream, "sha256").hexdigest()
        if actual != image_pin(root, settings.source):
            raise ValueError("match checking requires the configured pinned original image")
        snapshot = settings.snapshot(root, {
            "head": git(root, "rev-parse", "HEAD").decode().strip(),
            "image": {"path": str(image), "sha256": actual},
            "database": hashlib.sha256(database.read_bytes()).hexdigest()})
        if any(value is None for value in snapshot["inputs"].values()):
            raise ValueError("declared match-check input is missing")
        for name in (*settings.source.roots, *settings.input_roots):
            directory = settings.source.path(root, name)
            if not directory.is_dir():
                raise ValueError(f"match-check input root is missing: {name}")
            if any(path.is_symlink() for path in (directory, *directory.rglob("*"))):
                raise ValueError("match checking cannot enumerate source/input-root symlinks")
        return snapshot

    before = inputs()
    database_bytes = database.read_bytes()
    built, external = build_units(root, database_bytes)
    records = [row for path in settings.source.files(root) for row in read_file(path)]
    shared = shared_spans(records)
    groups, unsupported = defaultdict(list), []
    for record in records:
        claimed = record.byte_exact or record.semantic
        name = record.path.relative_to(root).as_posix()
        if not claimed and not (name in built and record.name and record.image_spans):
            continue
        if record.path.suffix not in settings.unit_extensions:
            row = _observation(root, record, "mapping host is not a configured translation unit",
                               built=built, source=settings.source)
            row["result"] = "unknown" if claimed else None
            unsupported.append(row)
        else:
            groups[record.path].append(record)
    work = [(root, settings.compiler_profile, path, group, image, settings.flag_sets, shared)
            for path, group in sorted(groups.items())]
    if inputs() != before:
        raise ValueError("match inputs changed before compilation")
    if jobs == 1 or len(work) < 2:
        measurements = [_unit(job) for job in work]
    else:
        with ProcessPoolExecutor(max_workers=min(jobs, len(work))) as pool:
            measurements = list(pool.map(_unit, work))
    if inputs() != before or database.read_bytes() != database_bytes:
        raise ValueError("source, image, history or compiler inputs changed during match checking")
    rows = list(unsupported)
    for job, unit in zip(work, measurements, strict=True):
        # The selection, not a worker's report length, defines coverage.
        expected = job[3]
        if (len(unit.results) != len(expected)
                or [_key(row) for row, _ in unit.results] != [_key(row) for row in expected]
                or any(row != selected for (row, _), selected in zip(unit.results, expected))):
            raise ValueError("unit comparison did not cover the selected source mappings")
        rows.extend(_observation(root, row, result, built=built, source=settings.source)
                    for row, result in unit.results)
    rows.sort(key=lambda row: (row["path"], row["line"], row["address"]))
    claims = [row for row in rows if row["byte_obligation"] or row["shape_obligation"]]
    counts = Counter(row["result"] for row in claims)
    result = "fail" if counts["fail"] else "unknown" if counts["unknown"] or not claims else "pass"
    unclaimed = [row for row in rows if not row["byte_obligation"] and not row["shape_obligation"]]
    return {
        "schema_version": 1, "check": "historical_source_matches", "result": result,
        "verification": "historical_masked_bytes_only", "claims": len(claims), "counts": dict(counts),
        "verified": sum(row["result"] == "pass" and row["verdict"] == "BYTE_EXACT" for row in claims),
        "semantic": sum(row["result"] == "pass" and row["verdict"] != "BYTE_EXACT" for row in claims),
        "regressed": [row for row in claims if row["result"] == "fail"],
        "unverifiable": [row for row in claims if row["result"] == "unknown"],
        "free": [row for row in unclaimed if row["verdict"] == "BYTE_EXACT"],
        "near_miss": [row for row in unclaimed if row["verdict"] != "BYTE_EXACT"
                      and 0 < row.get("original_instructions", 0) - row.get("matching_instructions", 0) <= 3],
        "allocation_only": [row for row in unclaimed if row["verdict"] != "BYTE_EXACT" and row.get("allocation_only")],
        "measurements": rows, "translation_units": len(groups), "unsupported_hosts": unsupported,
        "compile_attempts": sum(unit.compile_attempts for unit in measurements),
        "reused_attempts": sum(unit.reused_attempts for unit in measurements),
        "external_build_inputs": external, "input_fingerprint": digest(before),
        "persisted": False, "ready_for_collection": False,
        "limitations": ["historical masked-byte obligations, not strict reference or structural correctness",
                        "source-history identity coverage, source integrity and intended build ownership are separate checks",
                        "no implicit donor recipe for seed evidence; unresolved historical claims remain unknown",
                        "no claims is unverified, not an empty successful recovery",
                        "no final-link selection or runtime behavior proof"],
    }
