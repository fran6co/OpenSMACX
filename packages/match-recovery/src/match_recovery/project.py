"""Non-overwriting local project setup; no target registry or bundled project code."""

from pathlib import Path

from .model import require_text
from .records import publish_json, publish_text


ADAPTER = '''"""Project-owned command adapter. Replace the unimplemented checks locally.

One JSON object arrives on stdin; write one JSON object to stdout. Diagnostics
belong on stderr. No framework imports are required, and any language works.
"""

import json
import sys

request = json.load(sys.stdin)
action = sys.argv[1]
if action == "project":
    print(json.dumps({"output": {"result": "unknown", "reason": "project source operations are not configured"}}))
    raise SystemExit(3)
if action == "context":
    print(json.dumps({"source_homes": request["task"]["editable_files"],
                      "instructions": ["Read source-owned mappings and project evidence before editing."]}))
    raise SystemExit(0)
print(json.dumps({"result": "unknown", "reason": action + " is not configured in this project"}))
raise SystemExit(3)
'''


def initialize(root: Path, target: str, python: str) -> dict:
    require_text(target, "target")
    require_text(python, "Python executable")
    root = root.resolve()
    files = ("driver.json", "project.json", "worker.json", "reviewer.json", "agents.json", "adapter.py")
    directory = root / "recovery"
    if directory.is_symlink() or not directory.resolve().is_relative_to(root):
        raise ValueError("project setup directory must stay inside the project")
    for name in files:
        path = directory / name
        if path.exists() or path.is_symlink():
            raise ValueError(f"project setup refuses to overwrite {path}")
    command = lambda action: {"argv": [python, "recovery/adapter.py", action], "timeout": 300}
    publish_json(directory / "driver.json", {"schema_version": 1, "target": target,
                 "context": command("context"), "check": command("check")})
    publish_json(directory / "project.json", {"schema_version": 1, "adapter": command("project")})
    agents = {"schema_version": 1, "artifacts": "build/recovery/agents"}
    for role in ("worker", "reviewer"):
        publish_json(directory / (role + ".json"), {"author": role,
                     "command": {"argv": [python, "-m", "match_recovery.cli", "agent", role], "timeout": 3600}})
        agents[role] = {"command": {"argv": ["{env:RECOVERY_" + role.upper() + "}"], "timeout": 3300},
                        "instructions": [], "response": "stdout"}
    publish_json(directory / "agents.json", agents)
    publish_text(directory / "adapter.py", ADAPTER)
    return {"created": ["recovery/" + name for name in files], "target": target,
            "ready_for_recovery": False,
            "next_steps": ["Implement local context and verification commands; missing proof must stay unknown.",
                           "Configure independent prompt-driven worker/reviewer commands in recovery/agents.json.",
                           "Create a task decision from source-owned original identities with recover tasks create --file.",
                           "Ignore build/ caches and checkpoint configuration/task decisions in Git before recover run.",
                           "Configure obligations and full acceptance before claiming completed recovery."]}
