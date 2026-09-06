"""Read-only import of the old OpenSMACX annotation system.

This is a migration boundary, not the new framework's reader or ledger. All raw
files are preserved, including comments the old parser cannot interpret. A
legacy match token becomes a historical obligation, never a fresh measurement.
"""

from __future__ import annotations

import base64
from collections import Counter, defaultdict
from dataclasses import asdict
import hashlib
from pathlib import Path
import re
import subprocess
from typing import Any

from ..model import Dimension, Obligation, Origin, Span, require_digest


def image_pin(root: Path) -> str:
    text = (root / "src" / "IMAGE_PIN").read_text(encoding="utf-8")
    matches = re.findall(r"^sha256\s+([0-9a-f]{64})\s*$", text, re.MULTILINE)
    if len(matches) != 1:
        raise ValueError("src/IMAGE_PIN must contain exactly one SHA-256 pin")
    return matches[0]


def _revision(root: Path) -> str | None:
    result = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root,
                            capture_output=True, text=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def _sources(root: Path) -> list[Path]:
    return sorted(path for path in (root / "src").rglob("*")
                  if path.is_file() and path.suffix in (".cpp", ".c", ".h"))


def legacy_snapshot(root: Path, image: Path | None = None) -> dict[str, Any]:
    # The standalone package works without `decomp`. Only this one-time adapter
    # needs the old parser, installed by the OpenSMACX workspace.
    from decomp.reader import read_text

    root = root.resolve()
    pin = image_pin(root)
    revision = _revision(root)
    if image is not None:
        with image.open("rb") as stream:
            actual = hashlib.file_digest(stream, "sha256").hexdigest()
        if actual != pin:
            raise ValueError(f"image digest mismatch: expected {pin}, got {actual}")
    files, records, obligations = {}, [], []
    paths = _sources(root)
    for path in paths:
        if not path.resolve().is_relative_to(root):
            raise ValueError(f"source resolves outside the worktree: {path}")
        raw = path.read_bytes()
        file_digest = hashlib.sha256(raw).hexdigest()
        relative = path.relative_to(root).as_posix()
        files[relative] = {"sha256": file_digest,
                           "content_base64": base64.b64encode(raw).decode("ascii")}
        # The base64 snapshot is lossless even if the historical source has an
        # invalid UTF-8 comment; parser replacement cannot destroy that evidence.
        for record in read_text(raw.decode("utf-8", errors="replace"), path):
            origin = Origin(pin, "pe:va", record.address)
            invalid_spans = []
            for low, high in record.image_spans:
                try:
                    Span(low, high)
                except ValueError:
                    invalid_spans.append([low, high])
            seed = path.relative_to(root / "src").parts[0] in ("recovered", "unrecovered")
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
                "legacy_exclusion": record.exclusion,
                "legacy_byte_exact": record.byte_exact,
                "legacy_semantic": record.semantic,
                "levers": [list(item) for item in record.levers],
                "tried": list(record.ruled_out), "deferred": list(record.deferred),
                "extract_error": record.extract_error,
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
    if not records:
        raise ValueError("no legacy annotations found; refusing an empty migration")
    # Refuse a racing import rather than quietly dropping concurrently moved files.
    if paths != _sources(root) or pin != image_pin(root) or revision != _revision(root):
        raise ValueError("source inventory changed during import; retry at a checkpoint")
    for name, info in files.items():
        if hashlib.sha256((root / name).read_bytes()).hexdigest() != info["sha256"]:
            raise ValueError(f"source changed during import: {name}")
    return {"schema_version": 1, "provider": "opensmacx-legacy",
            "image_sha256": pin, "image_verified": image is not None,
            "revision": revision, "files": files, "records": records,
            "historical_obligations": obligations}


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
    return {
        "schema_version": 1, "revision": snapshot["revision"],
        "image_sha256": snapshot["image_sha256"],
        "image_verified": snapshot["image_verified"],
        "source_files": len(snapshot["files"]), "annotations": len(records),
        "entities": len(by_entity),
        "candidates": dict(sorted(Counter(row["candidate_kind"] for row in records).items())),
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
