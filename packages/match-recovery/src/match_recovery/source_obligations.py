"""Derive task restoration requirements from source, including removed claims.

History records the obligation; none of these historical claims is a current
measurement. The returned report is disposable and never written as a ledger.
"""

from dataclasses import asdict
import base64
from pathlib import Path

from match_recovery.model import Dimension, Obligation, Origin, digest
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.generated_source import parse_generated
from match_recovery.object_source import parse_objects
from .literal_source import parse_literals
from .shared_source import parse_shared
from match_recovery.source_history import HistoryConfiguration, git, source_versions
from match_recovery.tasks import TaskStore
from match_recovery.source_inventory import assert_snapshot_current, source_snapshot
from .source_config import SourceConfiguration


def task_obligations(root: Path, name: str) -> dict:
    from match_recovery.source.reader import read_text

    root = root.resolve()
    store = TaskStore(root)
    task = store.read(name)
    decisions = store.decisions(name)
    scope, _ = store._scope(task, decisions)
    head = git(root, "rev-parse", "HEAD").decode().strip()
    history = HistoryConfiguration.read(root)
    snapshot = source_snapshot(root)
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    pin = snapshot["image_sha256"]
    if any(subject.image_sha256 != pin or subject.address_space != configuration.address_space for subject in task.subjects):
        raise ValueError("task original does not match the source image pin")
    versions = source_versions(root, task.base_revision, head, configuration=configuration, history=history,
                               image_sha256=pin)
    found: dict[tuple[str, Dimension], list[dict]] = {}

    def add(entity: str, dimension: Dimension, evidence: dict) -> None:
        if entity in scope:
            found.setdefault((entity, dimension), []).append(evidence)

    def add_structure(text: str, path: str, evidence: dict, source: SourceConfiguration) -> None:
        if source.is_seed(path):
            return
        # The original evidence anchor, not a current class/member spelling,
        # owns the obligation. Share interpretation identities with the fresh
        # verifiers so a rename/rehome cannot accidentally borrow another proof.
        for parser in (parse_members, parse_generated, parse_bases, parse_objects, parse_shared, parse_literals):
            for item in parser(text, path, pin, address_space=source.address_space):
                if item.evidence is None:
                    continue  # A v1 object start does not assert original initial-data bounds.
                # A member offset/table bound is a concrete source requirement,
                # not an assertion that the entire class/lifetime is recovered.
                # Full STRUCTURE obligations, if supplied, remain distinct.
                add(item.evidence.entity_id, Dimension.DECLARED_STRUCTURE,
                    {**evidence, "line": item.line, "interpretation": item.interpretation})

    for version in versions:
        text = version.content.decode("utf-8", errors="replace")
        add_structure(text, version.path, {"revision": version.revision, "path": version.path, "blob": version.blob},
                      version.configuration)
        for record in read_text(text, root / version.path):
            entity = Origin(pin, version.configuration.address_space, record.address).entity_id
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
                   {"revision": "working-tree", "path": path, "sha256": row["sha256"]}, configuration)
    if (head != git(root, "rev-parse", "HEAD").decode().strip()
            or task != store.read(name) or decisions != store.decisions(name)
            or history != HistoryConfiguration.read(root)):
        raise ValueError("task or source history changed during obligation inventory")
    assert_snapshot_current(root, snapshot)
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
                                         "source_configuration": configuration, "source_history": history,
                                         "files": {name: row["sha256"] for name, row in snapshot["files"].items()}}),
            "obligations": obligations, "interpretations": [interpretations[key] for key in sorted(interpretations)],
            "verification": "not_measured"}
