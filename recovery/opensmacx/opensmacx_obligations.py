"""Derive task restoration requirements from source, including removed claims.

History records the obligation; none of these historical claims is a current
measurement. The returned report is disposable and never written as a ledger.
"""

from dataclasses import asdict
import base64
import hashlib
from pathlib import Path
import re

from match_recovery.model import Dimension, Obligation, Origin, digest
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.generated_source import parse_generated
from match_recovery.source_history import git, source_versions
from match_recovery.tasks import TaskStore
from .opensmacx import _sources, image_pin, legacy_snapshot


def task_obligations(root: Path, name: str) -> dict:
    from match_recovery.source.reader import read_text

    root = root.resolve()
    store = TaskStore(root)
    task = store.read(name)
    decisions = store.decisions(name)
    scope, _ = store._scope(task, decisions)
    head = git(root, "rev-parse", "HEAD").decode().strip()
    versions = source_versions(root, task.base_revision, head, required_paths=("src/IMAGE_PIN",))
    snapshot = legacy_snapshot(root)
    pin = snapshot["image_sha256"]
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("task original does not match the source image pin")
    found: dict[tuple[str, Dimension], list[dict]] = {}

    def add(entity: str, dimension: Dimension, evidence: dict) -> None:
        if entity in scope:
            found.setdefault((entity, dimension), []).append(evidence)

    def add_structure(text: str, path: str, evidence: dict) -> None:
        for item in parse_members(text, path, pin):
            # The original evidence address anchors the review obligation;
            # class/member spellings and offsets remain revisable interpretations.
            add(item.evidence.entity_id, Dimension.STRUCTURE,
                {**evidence, "line": item.line,
                 "interpretation": {"kind": "direct_member_offset", "class_name": item.class_name,
                                    "member": item.member, "offset": item.offset,
                                    **({"class_type": item.class_type} if item.class_type is not None else {}),
                                    "original_evidence": asdict(item.evidence)}})

        for item in parse_generated(text, path, pin):
            add(item.evidence.entity_id, Dimension.STRUCTURE,
                {**evidence, "line": item.line,
                 "interpretation": {"kind": "compiler_generated_vtable",
                                    "origin": asdict(item.origin),
                                    "span": asdict(item.span) if item.span is not None else None,
                                    "symbol": item.symbol,
                                    "original_evidence": asdict(item.evidence)}})
        for item in parse_bases(text, path, pin):
            add(item.evidence.entity_id, Dimension.STRUCTURE,
                {**evidence, "line": item.line,
                 "interpretation": {"kind": "nonvirtual_base_offset", "class_name": item.class_name,
                                    "base": item.base, "offset": item.offset,
                                    "original_evidence": asdict(item.evidence)}})

    for version in versions:
        text = version.content.decode("utf-8", errors="replace")
        if version.path == "src/IMAGE_PIN":
            pins = re.findall(r"^sha256\s+([0-9a-f]{64})\s*$", text, re.MULTILINE)
            if pins != [pin]:
                raise ValueError("source image pin changed within task history")
            continue
        add_structure(text, version.path, {"revision": version.revision, "path": version.path, "blob": version.blob})
        for record in read_text(text, root / version.path):
            entity = Origin(pin, "pe:va", record.address).entity_id
            for present, dimension in ((record.byte_exact, Dimension.BYTES),
                                       (record.semantic, Dimension.INSTRUCTION_SHAPE)):
                if present:
                    add(entity, dimension, {"revision": version.revision, "path": version.path,
                                            "blob": version.blob, "line": record.line})
    for row in snapshot["records"]:
        for present, dimension in ((row["legacy_byte_exact"], Dimension.BYTES),
                                   (row["legacy_semantic"], Dimension.INSTRUCTION_SHAPE)):
            if present:
                add(row["entity_id"], dimension, {"revision": "working-tree", "path": row["path"],
                                                 "sha256": row["file_sha256"], "line": row["line"]})
    for path, row in snapshot["files"].items():
        add_structure(base64.b64decode(row["content_base64"]).decode("utf-8", errors="replace"), path,
                   {"revision": "working-tree", "path": path, "sha256": row["sha256"]})
    if (head != git(root, "rev-parse", "HEAD").decode().strip()
            or task != store.read(name) or decisions != store.decisions(name)):
        raise ValueError("task or source history changed during obligation inventory")
    current_files = {path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                     for path in _sources(root)}
    if (current_files != {name: row["sha256"] for name, row in snapshot["files"].items()}
            or image_pin(root) != pin):
        raise ValueError("source changed during obligation inventory")
    obligations = []
    interpretations = {}
    for (entity, dimension), evidence in sorted(found.items()):
        obligation = Obligation(entity, dimension, "git-history:" + task.base_revision)
        obligations.append({**asdict(obligation), "obligation_id": obligation.obligation_id,
                            "source_evidence": evidence})
        for item in evidence:
            if "interpretation" in item:
                value = {"entity_id": entity, "value": item["interpretation"]}
                key = digest(value)
                current = item["revision"] == "working-tree" or interpretations.get(key, {}).get("current", False)
                interpretations[key] = {"interpretation_id": key, **value, "current": current}
    return {"schema_version": 1, "task": name, "base_revision": task.base_revision,
            "head": head, "image_sha256": pin, "scope": sorted(scope),
            "input_fingerprint": digest({"task": task, "decisions": decisions, "head": head,
                                         "files": {name: row["sha256"] for name, row in snapshot["files"].items()}}),
            "obligations": obligations, "interpretations": [interpretations[key] for key in sorted(interpretations)],
            "verification": "not_measured"}
