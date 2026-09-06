"""Durable task decisions; current ownership, impacts and verdicts stay derived.

A task fixes what the coordinator authorized. New structural discoveries do not
disappear at a function boundary: they remain pending until explicitly resolved,
and out-of-scope discoveries require a new scope decision before editing.

This is a trusted-coordinator API. Actor labels provide attribution, not process
authentication; worker isolation and independent-review execution are separate
runner responsibilities. A recorded decision never constitutes a passing gate.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re
from typing import Iterable

from .components import relative_path
from .journal import Journal
from .model import Origin, require_digest, require_text
from .records import publish_json, read_json


def task_name(value: str) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[a-z0-9][a-z0-9_-]{0,79}", value):
        raise ValueError("task id must be a short lowercase name, not a path")
    return value


def entity_id(value: str) -> str:
    if not isinstance(value, str) or not value.startswith("entity-"):
        raise ValueError("expected an original entity identity")
    require_digest(value[7:])
    return value


@dataclass(frozen=True)
class Task:
    task_id: str
    target: str
    author: str
    intent: str
    base_revision: str
    subjects: tuple[Origin, ...]
    editable_files: tuple[str, ...]
    evidence: tuple[str, ...]

    def __post_init__(self):
        task_name(self.task_id)
        for value in (self.target, self.author, self.intent):
            require_text(value, "task decision")
        if not isinstance(self.base_revision, str) or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", self.base_revision):
            raise ValueError("task needs a committed source baseline")
        if not self.subjects or len({subject.entity_id for subject in self.subjects}) != len(self.subjects):
            raise ValueError("task subjects must be nonempty and unique")
        if not self.editable_files or len(set(self.editable_files)) != len(self.editable_files):
            raise ValueError("editable files must be nonempty and unique")
        for name in self.editable_files:
            relative_path(name)
        if not self.evidence:
            raise ValueError("task requires evidence for its scope/intent")
        for item in self.evidence:
            require_text(item, "task evidence")

    def to_dict(self) -> dict:
        return {"schema_version": 1, **asdict(self)}

    @classmethod
    def from_dict(cls, value: dict):
        fields = {"schema_version", "task_id", "target", "author", "intent", "base_revision",
                  "subjects", "editable_files", "evidence"}
        if not isinstance(value, dict) or set(value) != fields:
            raise ValueError("invalid task fields")
        if type(value["schema_version"]) is not int or value["schema_version"] != 1:
            raise ValueError("unsupported task schema")
        for name in ("subjects", "editable_files", "evidence"):
            if not isinstance(value[name], list):
                raise ValueError(f"task {name} must be an array")
        subjects = []
        for item in value["subjects"]:
            if not isinstance(item, dict) or set(item) != {"image_sha256", "address_space", "address"}:
                raise ValueError("invalid task original identity")
            subjects.append(Origin(**item))
        return cls(*(value[name] for name in ("task_id", "target", "author", "intent", "base_revision")),
                   tuple(subjects), tuple(value["editable_files"]), tuple(value["evidence"]))


class TaskStore:
    def __init__(self, root: Path, *, private: str = "build/recovery"):
        self.root = Path(root)
        self.journal = Journal(self.root / "recovery" / "history",
                               lock_directory=self.root / relative_path(private) / "locks")

    def create(self, task: Task) -> bool:
        return publish_json(self.root / "recovery" / "tasks" / (task.task_id + ".json"),
                            task.to_dict(), allow_identical=True)

    def read(self, name: str) -> Task:
        task = Task.from_dict(read_json(self.root / "recovery" / "tasks" / (task_name(name) + ".json")))
        if task.task_id != name:
            raise ValueError("task filename disagrees with its identity")
        return task

    def decisions(self, name: str) -> tuple[dict, ...]:
        task = self.read(name)
        allowed = {subject.entity_id for subject in task.subjects}
        images = {(subject.image_sha256, subject.address_space) for subject in task.subjects}
        discovered, resolved, payloads = {}, set(), []
        fields = {
            "discovery": {"kind", "discovery_id", "author", "hypothesis", "affects", "evidence"},
            "scope_expanded": {"kind", "author", "subjects", "editable_files", "evidence"},
            "discovery_resolved": {"kind", "discovery_id", "author", "disposition", "source_evidence"},
        }
        for event in self.journal.read("task:" + name):
            payload = event.payload
            kind = payload["kind"]
            if kind not in fields or set(payload) != fields[kind]:
                raise ValueError("unsupported or invalid task decision fields")
            require_text(payload["author"], "decision author")
            evidence = payload["source_evidence" if kind == "discovery_resolved" else "evidence"]
            if not isinstance(evidence, list) or not evidence:
                raise ValueError("task decision evidence must be a nonempty array")
            for item in evidence:
                require_text(item, "decision evidence")
            if kind == "discovery":
                key = payload["discovery_id"]
                require_text(key, "discovery id")
                require_text(payload["hypothesis"], "hypothesis")
                if key in discovered or not isinstance(payload["affects"], list) or not payload["affects"]:
                    raise ValueError("duplicate discovery or invalid affected entities")
                for item in payload["affects"]:
                    entity_id(item)
                discovered[key] = payload
            elif kind == "scope_expanded":
                if (payload["author"] != task.author or not isinstance(payload["subjects"], list)
                        or not isinstance(payload["editable_files"], list)):
                    raise ValueError("invalid coordinator scope expansion")
                for item in payload["subjects"]:
                    if not isinstance(item, dict) or set(item) != {"image_sha256", "address_space", "address"}:
                        raise ValueError("invalid expansion subject")
                    subject = Origin(**item)
                    if (subject.image_sha256, subject.address_space) not in images:
                        raise ValueError("expansion contains an unrelated original")
                    allowed.add(subject.entity_id)
                for item in payload["editable_files"]:
                    relative_path(item)
            else:
                key = payload["discovery_id"]
                require_text(key, "discovery id")
                if key not in discovered or key in resolved or payload["disposition"] not in ("incorporated", "rejected"):
                    raise ValueError("invalid or conflicting discovery resolution")
                if payload["disposition"] == "incorporated" and not set(discovered[key]["affects"]) <= allowed:
                    raise ValueError("discovery was incorporated outside its authorized scope")
                resolved.add(key)
            payloads.append(payload)
        return tuple(payloads)

    def scope(self, name: str) -> tuple[frozenset[str], frozenset[str]]:
        return self._scope(self.read(name), self.decisions(name))

    @staticmethod
    def _scope(task: Task, decisions: tuple[dict, ...]) -> tuple[frozenset[str], frozenset[str]]:
        subjects = {subject.entity_id for subject in task.subjects}
        files = set(task.editable_files)
        for decision in decisions:
            if decision["kind"] == "scope_expanded":
                subjects.update(Origin(**item).entity_id for item in decision["subjects"])
                files.update(decision["editable_files"])
        return frozenset(subjects), frozenset(files)

    def expand(self, name: str, *, request_id: str, author: str, subjects: Iterable[Origin],
               editable_files: Iterable[str], evidence: Iterable[str], expected_revision: int) -> dict:
        """Coordinator editing authorization, NOT approval to relax any proof."""
        task = self.read(name)
        if author != task.author:
            raise ValueError("only the task coordinator may expand editing scope")
        subjects = tuple(subjects)
        files, evidence = tuple(sorted(set(editable_files))), tuple(evidence)
        if not (subjects or files) or not evidence:
            raise ValueError("scope expansion needs additional scope and evidence")
        images = {(subject.image_sha256, subject.address_space) for subject in task.subjects}
        if any((subject.image_sha256, subject.address_space) not in images for subject in subjects):
            raise ValueError("scope expansion refers to an unrelated original image/address space")
        if len({subject.entity_id for subject in subjects}) != len(subjects):
            raise ValueError("duplicate expansion subjects")
        for name_in_source in files:
            relative_path(name_in_source)
        for item in evidence:
            require_text(item, "scope evidence")
        payload = {"kind": "scope_expanded", "author": author, "subjects": [asdict(subject) for subject in subjects],
                   "editable_files": list(files), "evidence": list(evidence)}
        return self.journal.append("task:" + name, request_id=request_id, payload=payload,
                                   expected_revision=expected_revision).payload

    def discover(self, name: str, *, request_id: str, author: str, hypothesis: str,
                 affects: Iterable[str], evidence: Iterable[str], expected_revision: int) -> dict:
        self.read(name)
        require_text(author, "discovery author")
        require_text(hypothesis, "structural hypothesis")
        affects, evidence = tuple(sorted(set(affects))), tuple(evidence)
        if not affects or not evidence:
            raise ValueError("a discovery needs affected entities and supporting evidence")
        for item in affects:
            entity_id(item)
        for item in evidence:
            require_text(item, "discovery evidence")
        payload = {"kind": "discovery", "discovery_id": request_id, "author": author,
                   "hypothesis": hypothesis, "affects": list(affects), "evidence": list(evidence)}
        return self.journal.append("task:" + name, request_id=request_id, payload=payload,
                                   expected_revision=expected_revision).payload

    def resolve(self, name: str, *, discovery_id: str, request_id: str, author: str,
                disposition: str, source_evidence: Iterable[str], expected_revision: int) -> dict:
        decisions = self.decisions(name)
        if not any(item.get("kind") == "discovery" and item.get("discovery_id") == discovery_id for item in decisions):
            raise ValueError("unknown discovery")
        if any(item.get("kind") == "discovery_resolved" and item.get("discovery_id") == discovery_id for item in decisions):
            # A retry with the same journal request id is handled by append;
            # a new contradictory decision needs a new discovery, not rewriting history.
            previous = next(event for event in self.journal.read("task:" + name)
                            if event.payload.get("kind") == "discovery_resolved"
                            and event.payload.get("discovery_id") == discovery_id)
            if previous.request_id != request_id:
                raise ValueError("discovery is already resolved; record new evidence as a new discovery")
        if disposition not in ("incorporated", "rejected"):
            raise ValueError("discovery must be incorporated into source or rejected with evidence")
        if disposition == "incorporated":
            allowed, _ = self._scope(self.read(name), decisions)
            found = next(item for item in decisions if item.get("kind") == "discovery"
                         and item.get("discovery_id") == discovery_id)
            if not set(found["affects"]) <= allowed:
                raise ValueError("cannot incorporate a discovery beyond the approved task scope")
        require_text(author, "resolution author")
        evidence = tuple(source_evidence)
        if not evidence:
            raise ValueError("discovery resolution needs source evidence")
        for item in evidence:
            require_text(item, "resolution evidence")
        payload = {"kind": "discovery_resolved", "discovery_id": discovery_id,
                   "author": author, "disposition": disposition, "source_evidence": list(evidence)}
        return self.journal.append("task:" + name, request_id=request_id, payload=payload,
                                   expected_revision=expected_revision).payload

    def status(self, name: str, current_entities: Iterable[str]) -> dict:
        task = self.read(name)
        decisions = self.decisions(name)
        allowed, files = self._scope(task, decisions)
        discoveries = {item["discovery_id"]: item for item in decisions if item["kind"] == "discovery"}
        resolved = {item["discovery_id"] for item in decisions if item["kind"] == "discovery_resolved"}
        pending = [item for key, item in discoveries.items() if key not in resolved]
        missing = sorted(allowed - set(current_entities))
        expansion = sorted({subject for item in pending for subject in item["affects"]} - allowed)
        return {"task_id": name, "decision_revision": len(decisions), "missing_subjects": missing,
                "editable_files": sorted(files),
                "pending_discoveries": pending, "scope_expansion_required": expansion,
                "next_action": ("repair_missing_source_mapping" if missing else
                                "review_scope_expansion" if expansion else
                                "evaluate_structural_discoveries" if pending else
                                "evaluate_current_source_and_checks"),
                "verification": "not_measured", "ready_for_collection": False}
