"""Lossless snapshots of configured ORIGINAL-comment source and seed evidence.

This is a derived view, not another source ledger. All raw files are preserved
in memory, including comments the annotation parser cannot interpret. A
legacy match token becomes a historical obligation, never a fresh measurement.
"""

from __future__ import annotations

import base64
from collections import Counter, defaultdict
from dataclasses import asdict
import hashlib
import os
from pathlib import Path
import re
import subprocess
import sys
from typing import Any

from match_recovery.model import Dimension, Obligation, Origin, Span, digest, require_digest
from match_recovery.memo import memoized
from .source_config import SourceConfiguration


def image_pin(root: Path, configuration: SourceConfiguration | None = None) -> str:
    configuration = configuration or SourceConfiguration.read(root)
    text = configuration.path(root, configuration.image_pin).read_text(encoding="utf-8")
    matches = re.findall(r"^sha256\s+([0-9a-f]{64})\s*$", text, re.MULTILINE)
    if len(matches) != 1:
        raise ValueError(f"{configuration.image_pin} must contain exactly one SHA-256 pin")
    return matches[0]


def _revision(root: Path) -> str | None:
    result = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root,
                            capture_output=True, text=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def source_files(root: Path, configuration: SourceConfiguration | None = None) -> list[Path]:
    return (configuration or SourceConfiguration.read(root)).files(root)


def _lifetime_binding_hint(record) -> dict | None:
    """Expose real lifetime definitions hidden by old construct/destroy bindings.

    This is a candidate to measure, not permission to change a mapping. A real
    constructor/destructor may still be wrong; never silently select it instead.
    """
    from match_recovery.source.mangled import qualified_name
    from match_recovery.source.reader import function_line

    prefix = record.name[:3]
    if prefix not in ("??0", "??1") or not record.symbol or record.symbol.startswith(prefix):
        return None
    name = qualified_name(record.name)
    if not name:
        return None
    try:
        line = function_line(record.path, name)
    except ValueError:
        return None  # Missing/overloaded definitions require other evidence.
    return {"kind": "lifetime_wrapper_binding", "measured_symbol": record.symbol,
            "candidate_definition": name, "candidate_line": line,
            "next_action": "measure the real lifetime definition before reworking the wrapper or class layout"}


def _parse_file(raw: bytes, path: Path, root: Path, pin: str, configuration: SourceConfiguration) -> dict:
    from match_recovery.source.reader import read_text

    records, obligations = [], []
    file_digest = hashlib.sha256(raw).hexdigest()
    relative = path.relative_to(root).as_posix()
    # Lossless source bytes remain in the snapshot even for invalid UTF-8.
    from .binding_source import parse as parse_binding_evidence
    parsed_records = read_text(raw.decode("utf-8", errors="replace"), path)
    binding_evidence = parse_binding_evidence(raw.decode("utf-8", errors="replace"), parsed_records)
    for record in parsed_records:
        origin = Origin(pin, configuration.address_space, record.address)
        invalid_spans = []
        for low, high in record.image_spans:
            try:
                Span(low, high)
            except ValueError:
                invalid_spans.append([low, high])
        seed = configuration.is_seed(relative)
        row = {
            "entity_id": origin.entity_id, "origin": asdict(origin),
            "path": relative, "line": record.line, "file_sha256": file_digest,
            "candidate_kind": "legacy_seed" if seed else "source",
            "catalogued_symbol": record.name,
            "emitted_symbol_hint": record.symbol or record.name,
            "spans": [list(span) for span in record.image_spans],
            "invalid_spans": invalid_spans,
            "body_path": record.body, "kind_hint": record.kind,
            "legacy_mode": record.mode.value, "legacy_state": record.state.value,
            "source_body_region_present": bool(record.region.strip()),
            "source_role": record.source_role.value,
            "source_role_reason": record.source_role_reason,
            "legacy_exclusion": record.exclusion,
            "legacy_byte_exact": record.byte_exact,
            "legacy_semantic": record.semantic,
            "levers": [list(item) for item in record.levers],
            "tried": list(record.ruled_out), "deferred": list(record.deferred),
            "extract_error": record.extract_error,
            "binding_hint": _lifetime_binding_hint(record) if configuration.symbol_format == "msvc" else None,
            "binding_evidence": binding_evidence.get(record.line),
        }
        records.append(row)
        for present, dimension, token in (
                (record.byte_exact, Dimension.BYTES, "BYTE_EXACT"),
                (record.semantic, Dimension.INSTRUCTION_SHAPE, "SEMANTIC")):
            if present:
                obligation = Obligation(origin.entity_id, dimension,
                                        f"legacy:{file_digest}:{record.line}:{token}")
                obligations.append({**asdict(obligation),
                                    "obligation_id": obligation.obligation_id})
    return {"records": records, "obligations": obligations}


def _parser_key() -> str:
    # Include the installed source parser implementation in disposable memo keys.
    from match_recovery import source as source_records

    package = Path(source_records.__file__).parent
    inputs = {path.relative_to(package).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(package.rglob("*.py"))}
    import match_recovery
    for name in ("model.py", "memo.py", "records.py", "binding_source.py"):
        inputs["framework/" + name] = hashlib.sha256(
            (Path(match_recovery.__file__).parent / name).read_bytes()).hexdigest()
    inputs["source_inventory.py"] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    inputs["source_config.py"] = hashlib.sha256(Path(__file__).with_name("source_config.py").read_bytes()).hexdigest()
    return digest({"implementation": inputs, "python": sys.version})


def source_snapshot(root: Path, image: Path | None = None, *, allow_empty: bool = False) -> dict[str, Any]:
    if type(allow_empty) is not bool:
        raise ValueError("empty source selection policy must be a boolean")
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    cache = None
    if os.environ.get("RECOVERY_PARSE_CACHE") != "off":
        # Read-only commands must not create committable derived artifacts.
        relative_cache = "build/recovery/parse_cache/"
        ignored = subprocess.run(["git", "check-ignore", "-q", relative_cache],
                                 cwd=root, capture_output=True, check=False)
        tracked = subprocess.run(["git", "ls-files", "--", relative_cache],
                                 cwd=root, capture_output=True, check=False)
        if (ignored.returncode == 0 and tracked.returncode == 0 and not tracked.stdout
                and (root / relative_cache).resolve().is_relative_to(root)):
            cache = root / relative_cache
    parser_key = _parser_key() if cache is not None else None
    pin = image_pin(root, configuration)
    revision = _revision(root)
    if image is not None:
        with image.open("rb") as stream:
            actual = hashlib.file_digest(stream, "sha256").hexdigest()
        if actual != pin:
            raise ValueError(f"image digest mismatch: expected {pin}, got {actual}")
    files, records, obligations = {}, [], []
    paths = source_files(root, configuration)
    for path in paths:
        if not path.resolve().is_relative_to(root):
            raise ValueError(f"source resolves outside the worktree: {path}")
        raw = path.read_bytes()
        file_digest = hashlib.sha256(raw).hexdigest()
        relative = path.relative_to(root).as_posix()
        files[relative] = {"sha256": file_digest,
                           "content_base64": base64.b64encode(raw).decode("ascii")}
        # Resolved path shape affects the legacy proved-store body extraction.
        key = digest({"parser": parser_key, "file": file_digest, "path": relative,
                      "resolved_path": path.resolve().relative_to(root).as_posix(), "image": pin,
                      "source_configuration": asdict(configuration)})
        parsed = memoized(cache, key, lambda: _parse_file(raw, path, root, pin, configuration))
        records.extend(parsed["records"])
        obligations.extend(parsed["obligations"])
    if not records and not allow_empty:
        raise ValueError("no legacy annotations found; refusing an empty migration")
    # Refuse a racing import rather than quietly dropping concurrently moved files.
    if (configuration != SourceConfiguration.read(root) or paths != source_files(root, configuration)
            or pin != image_pin(root, configuration) or revision != _revision(root)):
        raise ValueError("source inventory changed during import; retry at a checkpoint")
    for name, info in files.items():
        if hashlib.sha256((root / name).read_bytes()).hexdigest() != info["sha256"]:
            raise ValueError(f"source changed during import: {name}")
    return {"schema_version": 1, "provider": "original-comments", "source_configuration": asdict(configuration),
            "image_sha256": pin, "image_verified": image is not None,
            "revision": revision, "files": files, "records": records,
            "historical_obligations": obligations}


def assert_snapshot_current(root: Path, snapshot: dict) -> None:
    """Recheck source inputs without reparsing or persisting another inventory."""
    root = root.resolve()
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    if (configuration != SourceConfiguration.read(root)
            or set(snapshot["files"]) != {path.relative_to(root).as_posix() for path in configuration.files(root)}
            or snapshot["image_sha256"] != image_pin(root, configuration)
            or snapshot["revision"] != _revision(root)):
        raise ValueError("source inventory changed; retry at a checkpoint")
    for name, row in snapshot["files"].items():
        if hashlib.sha256(configuration.path(root, name).read_bytes()).hexdigest() != row["sha256"]:
            raise ValueError(f"source changed since inventory: {name}")


def inventory(snapshot: dict[str, Any]) -> dict[str, Any]:
    require_digest(snapshot["image_sha256"])
    records = snapshot["records"]
    by_entity = defaultdict(list)
    for record in records:
        by_entity[record["entity_id"]].append(record)
    conflicts = []
    for entity_id, candidates in sorted(by_entity.items()):
        sources = [row for row in candidates if row["candidate_kind"] == "source"]
        if len(sources) > 1:
            conflicts.append({"entity_id": entity_id,
                              "candidates": [{"path": row["path"], "line": row["line"]}
                                             for row in sources]})
    body_presence_by_role = defaultdict(Counter)
    body_states_by_role = defaultdict(Counter)
    for row in records:
        role = row["source_role"]
        body_presence_by_role[role][
            "present" if row["source_body_region_present"] else "absent"] += 1
        body_states_by_role[role][row["legacy_state"]] += 1
    return {
        "schema_version": 1, "revision": snapshot["revision"],
        "image_sha256": snapshot["image_sha256"],
        "image_verified": snapshot["image_verified"],
        "source_files": len(snapshot["files"]), "annotations": len(records),
        "entities": len(by_entity),
        "candidates": dict(sorted(Counter(row["candidate_kind"] for row in records).items())),
        "source_body_states": {
            kind: dict(sorted(Counter(row["legacy_state"] for row in records if row["candidate_kind"] == kind).items()))
            for kind in sorted({row["candidate_kind"] for row in records})},
        "body_state_basis": "source_text_only_not_recovery_acceptance",
        "source_role_counts": dict(sorted(Counter(
            row["source_role"] for row in records).items())),
        "source_body_region_presence_by_role": {
            role: dict(sorted(counts.items()))
            for role, counts in sorted(body_presence_by_role.items())},
        "source_body_states_by_role": {
            role: dict(sorted(counts.items()))
            for role, counts in sorted(body_states_by_role.items())},
        "source_role_basis": "source_authored_non_derivable_role_not_recovery_acceptance",
        "ownership_hints": dict(sorted(Counter(row["kind_hint"] or "unknown"
                                                for row in records).items())),
        "historical_byte_obligations": sum(item["dimension"] == Dimension.BYTES
                                            for item in snapshot["historical_obligations"]),
        "historical_semantic_claims": sum(row["legacy_semantic"] for row in records),
        "invalid_extent_records": sum(bool(row["invalid_spans"]) or not row["spans"]
                                      for row in records),
        "source_conflicts": conflicts,
        "verification": "not_measured",
    }
