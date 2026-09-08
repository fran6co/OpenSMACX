"""Read-only selected-body measurement through configured compiler interaction.

Diagnostic trials reuse the same per-TU comparer as recording, but never invoke
publication, replace missing recipes, or require historical obligations to be
currently satisfied before investigating a structural regression.
"""

from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
from dataclasses import asdict
from hashlib import file_digest
from pathlib import Path

from .compiler import Compiler
from .measurement import Measurement
from .model import digest
from .source import State, read_file
from .source.asm import AsmComparison, compare_unit, shared_spans
from .source.model import Tier
from .source_history import git
from .source_inventory import image_pin
from .source_selection import select_records


def _unit(job):
    root, profile, path, records, image, flags, shared = job
    return compare_unit(records, image, path, (), flags, shared, compiler=Compiler(root, profile=profile))


def measure_source(root: Path, targets, *, image: Path, in_file=None, jobs=1):
    root = root.resolve()
    if not isinstance(targets, (list, tuple)) or not targets:
        raise ValueError("measurement requires explicit nonempty targets")
    if type(jobs) is not int or jobs < 1:
        raise ValueError("measurement jobs must be a positive integer")
    settings = Measurement.read(root)
    image = (image if image.is_absolute() else root / image).resolve()

    def inputs():
        with image.open("rb") as stream:
            actual = file_digest(stream, "sha256").hexdigest()
        if actual != image_pin(root, settings.source):
            raise ValueError("measurement requires the configured pinned original image")
        snapshot = settings.snapshot(root, {
            "head": git(root, "rev-parse", "HEAD").decode().strip(),
            "image": {"path": str(image), "sha256": actual},
            "targets": list(targets), "host": in_file})
        if any(value is None for value in snapshot["inputs"].values()):
            raise ValueError("declared measurement input is missing")
        for name in (*settings.source.roots, *settings.input_roots):
            directory = settings.source.path(root, name)
            if not directory.is_dir():
                raise ValueError(f"measurement input root is missing: {name}")
            if any(path.is_symlink() for path in (directory, *directory.rglob("*"))):
                raise ValueError("measurement cannot enumerate source/input-root symlinks")
        return snapshot

    before = inputs()
    records = [row for path in settings.source.files(root) for row in read_file(path)]
    host = settings.source.path(root, in_file) if in_file is not None else None
    selected = select_records(records, targets, host, settings.source.symbol_format)
    groups, unavailable = defaultdict(list), []
    for record in selected:
        if record.path.suffix not in settings.unit_extensions:
            unavailable.append((record, "mapping host is not a configured translation unit"))
        elif record.state is State.PLACEHOLDER:
            unavailable.append((record, "unfinished source placeholder; matching an empty body is not recovery"))
        else:
            groups[record.path].append(record)
    shared = shared_spans(records)
    work = [(root, settings.compiler_profile, path, rows, image, settings.flag_sets, shared)
            for path, rows in sorted(groups.items())]
    if inputs() != before:
        raise ValueError("measurement inputs changed before compilation")
    if jobs == 1 or len(work) < 2:
        units = [_unit(job) for job in work]
    else:
        with ProcessPoolExecutor(max_workers=min(jobs, len(work))) as pool:
            units = list(pool.map(_unit, work))
    if inputs() != before:
        raise ValueError("source, original, history or compiler inputs changed during measurement")
    results = list(unavailable)
    for job, unit in zip(work, units, strict=True):
        expected = job[3]
        if len(unit.results) != len(expected) or [row for row, _ in unit.results] != expected:
            raise ValueError("unit comparison did not cover exactly the selected source mappings")
        results.extend(unit.results)
    rows = []
    for record, value in results:
        row = {"address": record.address_hex, "path": record.path.relative_to(root).as_posix(),
               "line": record.line, "name": record.name, "symbol": record.symbol or record.name,
               "historical_byte_obligation": record.byte_exact, "historical_shape_obligation": record.semantic,
               "source_state": record.state.value}
        if isinstance(value, str):
            row.update(result="unknown", verdict="UNRESOLVED", diagnostic=value)
        elif isinstance(value, AsmComparison):
            row.update(asdict(value))
            row["result"] = ("pass" if value.verdict == Tier.BYTE_EXACT else
                             "unknown" if value.verdict in (Tier.NO_COMPILE, Tier.REFUSED, Tier.SHARED_TAIL) else "fail")
        else:
            raise ValueError("invalid selected-body comparison observation")
        rows.append(row)
    counts = Counter(row["result"] for row in rows)
    result = "fail" if counts["fail"] else "unknown" if counts["unknown"] else "pass"
    return {"schema_version": 1, "operation": "measure", "result": result,
            "verification": "historical_masked_bytes_only", "ready_for_collection": False, "persisted": False,
            "selected": len(selected), "counts": dict(counts),
            "measurements": sorted(rows, key=lambda row: (row["path"], row["line"])),
            "translation_units": len(units), "compile_attempts": sum(unit.compile_attempts for unit in units),
            "reused_attempts": sum(unit.reused_attempts for unit in units), "input_fingerprint": digest(before),
            "limitations": ["selected current spellings only; no source claims are banked, demoted or reverted",
                            "masked-byte comparison is not strict reference, structure, preservation, build/link or runtime proof",
                            "missing recipes remain unavailable; compiler interaction owns any explicitly configured seed policy",
                            "use task variants for private candidate decks with scope and damage controls"]}
