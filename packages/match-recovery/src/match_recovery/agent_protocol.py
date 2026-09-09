"""Provider-independent response shapes and authoritative syntax validation."""

from .model import require_text
from .reviews import validate_review
from .tasks import entity_id


def _object(properties):
    return {"type": "object", "properties": properties, "required": list(properties), "additionalProperties": False}


def response_schema(role):
    text = {"type": "string", "minLength": 1}
    evidence = {"type": "array", "items": text, "minItems": 1}
    digest = {"type": "string", "pattern": "^[0-9a-f]{64}$"}
    if role == "planner":
        return _object({"snapshot": digest, "action": {"type": "string", "enum": ["create", "resume", "defer"]},
                        "task_id": {"type": "string"}, "group": {"type": "string"}, "intent": {"type": "string"},
                        "editable_files": {"type": "array", "items": text}, "evidence": evidence})
    if role == "coordinator":
        return _object({"snapshot": digest,
                        "action": {"type": "string", "enum": ["expand", "incorporate", "reject", "defer", "investigate", "bind_seed"]},
                        "discovery_id": text,
                        "subjects": {"type": "array", "items": {"type": "string", "pattern": "^entity-[0-9a-f]{64}$"}},
                        "editable_files": {"type": "array", "items": text}, "evidence": evidence,
                        "binding": {"anyOf": [{"type": "null"}, _object({
                            "entity_id": {"type": "string", "pattern": "^entity-[0-9a-f]{64}$"},
                            "source": text, "line": {"type": "integer", "minimum": 1},
                            "source_sha256": digest, "name": text, "reason": text})]}})
    if role == "worker":
        discovery = _object({"hypothesis": text,
                             "affects": {"type": "array", "minItems": 1,
                                         "items": {"type": "string", "pattern": "^entity-[0-9a-f]{64}$"}},
                             "evidence": evidence})
        return _object({"discoveries": {"type": "array", "items": discovery}})
    if role != "reviewer":
        raise ValueError("agent role must be worker, reviewer, coordinator or planner")
    return _object({"snapshot": digest, "decision": {"type": "string", "enum": ["approve", "request_changes"]},
                    "evidence": evidence, "feedback": text,
                    "supersessions": {"type": "array", "items": _object({"previous": digest, "replacement": digest,
                                                                          "evidence": evidence, "reason": text})},
                    "retractions": {"type": "array", "items": _object({"interpretation": digest,
                                                                        "evidence": evidence, "reason": text})}})


def validate_worker_response(response):
    if not isinstance(response, dict) or set(response) != {"discoveries"} or not isinstance(response["discoveries"], list):
        raise ValueError("worker must return a discoveries array, not a verdict")
    for finding in response["discoveries"]:
        if not isinstance(finding, dict) or set(finding) != {"hypothesis", "affects", "evidence"}:
            raise ValueError("invalid worker discovery")
        require_text(finding["hypothesis"], "hypothesis")
        for key in ("affects", "evidence"):
            if not isinstance(finding[key], list) or not finding[key]:
                raise ValueError("discovery requires affected entities and evidence")
        for affected in finding["affects"]:
            entity_id(affected)
        for evidence in finding["evidence"]:
            require_text(evidence, "discovery evidence")


def validate_reviewer_response(response, snapshot):
    fields = {"snapshot", "decision", "evidence", "feedback"}
    if not isinstance(response, dict) or not fields <= set(response) or set(response) - fields - {"supersessions", "retractions"}:
        raise ValueError("invalid reviewer response")
    # Attribution belongs to the driver, never to model-provided JSON.
    validate_review({"kind": "review", "author": "worker", "reviewer": "reviewer", **response})
    if response["snapshot"] != snapshot:
        raise ValueError("review refers to stale source")
