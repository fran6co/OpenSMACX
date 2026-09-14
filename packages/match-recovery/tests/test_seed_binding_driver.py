"""Real binding publication in the driver; reasoning/checker subprocesses are doubles."""

import hashlib
import json

from match_recovery.driver import drive
from match_recovery.model import Origin
from match_recovery.source_inventory import source_snapshot
from match_recovery.tasks import Task, TaskStore
from test_binding import project
from test_task_source import git


def test_worker_body_is_bound_then_rechecked_without_resolving_discovery_early(project):
    root = project
    configuration = root / "recovery/project.json"
    value = json.loads(configuration.read_text())
    value["source"]["seed_roots"] = ["src/seeds"]
    configuration.write_text(json.dumps(value))
    (root / "src/seeds").mkdir()
    seed = root / "src/seeds/f.cpp"
    seed_text = "// ORIGINAL: 0x00401000 _old_hint 0x00401000-0x00401001\nint old_hint() { return 0; }\n"
    seed.write_text(seed_text)
    git(root, "add", ".")
    git(root, "commit", "-m", "Known original seed and product declaration")
    row, = source_snapshot(root)["records"]
    task = Task("seed-owner", "binding-control", "coordinator", "Recover a real product body from seed evidence",
                git(root, "rev-parse", "HEAD"), (Origin(**row["origin"]),), ("src/f.cpp",), ("test-owned seed",))
    store = TaskStore(root)
    store.create(task)
    store.discover(task.task_id, request_id="seed-body", author="coordinator", hypothesis="Product body needs recovery",
                   affects=[row["entity_id"]], evidence=["seed original span"], expected_revision=0)
    git(root, "add", "recovery")
    git(root, "commit", "-m", "Authorize seed recovery without changing its baseline")
    head = git(root, "rev-parse", "HEAD")
    command = lambda name: {"argv": [name], "timeout": 10}
    driver = {"schema_version": 1, "target": task.target, "context": command("context"),
              "check": command("check"), "build": command("build")}
    delegation = {"schema_version": 1, "author": task.author, "command": command("coordinate"),
                  "subjects": [], "editable_files": [], "evidence": ["Authorize only existing seed/product handoff"],
                  "seed_bindings": {"image": "original.exe"}, "max_decisions": 3}
    body = root / "src/f.cpp"
    events = []
    def execute(command, root, name, request):
        role = command.argv[0]
        events.append(role)
        if role == "coordinate":
            text = body.read_text()
            action = "incorporate" if "ORIGINAL:" in text else "bind_seed" if "return 2" in text else "investigate"
            result = {"snapshot": request["snapshot"], "action": action, "discovery_id": "seed-body",
                      "subjects": [], "editable_files": [], "evidence": ["Read current product definition against seed evidence"]}
            if action == "bind_seed":
                result["binding"] = {"entity_id": row["entity_id"], "source": "src/f.cpp", "line": 1,
                                     "source_sha256": hashlib.sha256(body.read_bytes()).hexdigest(),
                                     "name": "_f", "reason": "Test-owned seed extent and worker-written product owner",
                                     "evidence": ["src/seeds/seed.cpp: original extent and src/f.cpp: product _f"]}
            if action == "incorporate":
                assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
                assert [item["kind"] for item in store.decisions(task.task_id)] == ["discovery"]
                assert len(source_snapshot(root)["records"]) == 2
            return 0, result
        if role == "context":
            return 0, {"mapped": "ORIGINAL:" in body.read_text()}
        if role == "check":
            mapped = "ORIGINAL:" in body.read_text()
            assert request["context"]["mapped"] == mapped
            return (0, {"result": "pass"}) if mapped else (1, {"result": "fail"})
        if role == "build":
            return 0, {"result": "pass"}
        if role == "worker":
            assert request["context"]["discovery_investigations"][0]["action"] == "investigate"
            body.write_text("int f() { return 2; }\n")
            return 0, {"discoveries": []}
        assert role == "reviewer"
        assert not store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
        assert "ORIGINAL:" in body.read_text() and seed.read_text() == seed_text
        return 0, {"snapshot": request["snapshot"], "decision": "request_changes", "evidence": ["Test-owned handoff"],
                   "feedback": "Continue byte recovery; this control proves the binding handoff, not body correctness.",
                   "supersessions": [], "retractions": []}
    report = drive(root, task.task_id, driver, {"author": "worker", "command": command("worker")}, iterations=1,
                   coordinator_configuration=delegation,
                   reviewer_configuration={"author": "reviewer", "command": command("reviewer")}, execute=execute)
    assert report["stop_reason"] == "iteration_budget_exhausted" and not report["ready_for_collection"]
    assert report["worker_rounds"] == 1 and report["coordinator_decisions"] == 3
    assert events == ["context", "check", "coordinate", "context", "check", "build", "worker",
                      "context", "check", "coordinate", "context", "check", "coordinate",
                      "context", "check", "build", "reviewer"]
    assert store.read(task.task_id).base_revision == task.base_revision and git(root, "rev-parse", "HEAD") == head
    assert seed.read_text() == seed_text
    product, = [item for item in source_snapshot(root)["records"] if item["path"] == "src/f.cpp"]
    assert not product["legacy_byte_exact"] and product["origin"] == row["origin"] and product["spans"] == row["spans"]
