"""Source-owned rationale for original-function bindings, separate from run logs."""

import json

from .model import require_text
from .records import parse_json


PREFIX = "// RECOVERY_BINDING: v1 "


def validate(value):
    if not isinstance(value, dict) or set(value) != {"reason", "evidence"}:
        raise ValueError("binding rationale requires reason and evidence")
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("binding rationale requires nonempty evidence references")
    for text in [value["reason"], *value["evidence"]]:
        require_text(text, "binding evidence text")
        if (any(ord(char) < 32 or char in "\x7f\x85\u2028\u2029" for char in text)
                or "ORIGINAL:" in text):
            raise ValueError("binding evidence must be single-line text without embedded markers")
    return value


def render(reason, evidence):
    return PREFIX + json.dumps(validate({"reason": reason, "evidence": list(evidence)}),
                               ensure_ascii=False, sort_keys=True)


def parse(text, records):
    """Attach only to the immediately preceding marker; never float to a neighbor."""
    markers = {record.line for record in records}
    result = {}
    for line, content in enumerate(text.splitlines(), 1):
        stripped = content.strip()
        if not stripped.startswith("// RECOVERY_BINDING:"):
            continue
        if not stripped.startswith(PREFIX) or line - 1 not in markers:
            raise ValueError(f"binding evidence at line {line} requires v1 syntax immediately after its ORIGINAL marker")
        result[line - 1] = validate(parse_json(stripped[len(PREFIX):]))
    return result
