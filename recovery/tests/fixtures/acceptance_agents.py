"""Deterministic local agent subprocesses for native acceptance damage controls.

These are test doubles for reasoning only. The driver, coordinator authorization,
compiler, linker, source checks and acceptance verifier are the real package path.
No provider service is called.
"""

import json
import hashlib
from pathlib import Path
import sys

from match_recovery.source_inventory import source_snapshot


MODEL = """struct Base { int bias; };
struct Member { int value; };
struct Owner : Base {
    Member member;
    void assign(int a, int b) { bias = a; member.value = b; }
    int add(int input);
};
"""


role, request_path = sys.argv[1:]
request = json.loads(Path(request_path).read_text())
sys.stdin.read()  # Consume the bridge prompt; decisions use the structured request.
if role == "planner":
    existing = [item for item in request["tasks"] if item["eligible"]]
    group = next((item for item in request["groups"] if item["eligible"] and any(
        "Owner::add(" in Path(name).read_text() for name in item["source_homes"])), None)
    action = "resume" if existing else "create" if group and request["create_available"] else "defer"
    response = {"snapshot": request["snapshot"], "action": action,
                "task_id": existing[0]["task"]["task_id"] if existing else "planned-owner" if action == "create" else "",
                "group": group["selector"] if action == "create" else "",
                "intent": "Recover the original Owner model and affected caller" if action == "create" else "",
                "editable_files": [], "evidence": ["Test-owned original Owner evidence and current source model"]}
elif role == "worker":
    header = Path("src/owner.h")
    # Reason from current source, including after a new bounded invocation;
    # invocation-local round numbers are not a recovery-state ledger.
    step = 1 if "int raw_[2];" in header.read_text() else 2
    context = request["context"]
    if context.get("discovery_investigations"):
        assert step == 1
        assert [item["discovery_id"] for item in context["discovery_investigations"]] == ["owner-model"]
        assert any(item["discovery_id"] == "owner-model" for item in context["pending_discoveries"])
        Path("build/investigation-worker.json").write_text(json.dumps(context["discovery_investigations"]))
    if step == 2:
        assert header.read_text().endswith(MODEL)  # Structural progress was preserved.
    evidence = "".join(line + "\n" for line in header.read_text().splitlines() if line.startswith("//"))
    header.write_text(evidence + MODEL)
    source = Path("src/owner.cpp")
    body = source.read_text().replace("raw_[0] + raw_[1]", "bias + member.value")
    source.write_text(body.replace("+ input", "- input") if step == 1 else body.replace("- input", "+ input"))
    Path(f"build/worker-{step}.json").write_text(json.dumps(request["check"]))
    response = {"discoveries": []}
    if step == 1:
        response["discoveries"] = [{"hypothesis": "The caller shares this recovered class declaration",
                                     "affects": sorted({row["entity_id"] for row in source_snapshot(Path.cwd())["records"]}),
                                     "evidence": ["src/owner.h and src/entry.cpp"]}]
elif role == "coordinator":
    finding = next(item for item in request["pending_discoveries"]
                   if item["discovery_id"] not in request.get("investigating", []))
    extra = sorted(set(finding["affects"]) - set(request["subjects"]))
    unfinished = "int raw_[2];" in Path("src/owner.h").read_text()
    response = {"snapshot": request["snapshot"], "action": "expand" if extra else "investigate" if unfinished else "incorporate",
                "discovery_id": finding["discovery_id"], "subjects": extra, "editable_files": [],
                "evidence": ["Test source still needs the real base/member model; authorize one in-scope attempt." if unfinished else
                             "Test source expresses the base/member relationship; include the known caller before independent review. This is not acceptance."]}
    product = Path("src/owner.cpp")
    if not extra and not unfinished and "ORIGINAL:" not in product.read_text():
        seed, = [row for row in request["mapped_records"] if row["path"] == "src/seeds/owner.cpp"]
        response.update(action="bind_seed", binding={
            "entity_id": seed["entity_id"], "source": "src/owner.cpp",
            "line": next(i for i, line in enumerate(product.read_text().splitlines(), 1) if line.startswith("int Owner::add")),
            "source_sha256": hashlib.sha256(product.read_bytes()).hexdigest(),
            "name": "?add@Owner@@QAEHH@Z", "reason": "Test-owned original class symbol, seed span and worker-written real Owner method",
            "evidence": ["src/seeds/owner.cpp: original Owner::add span and symbol; src/owner.cpp: corresponding Owner method"]})
else:
    assert role == "reviewer"
    response = {"snapshot": request["snapshot"], "decision": "approve",
                "evidence": ["test-owned original Base and Member"], "feedback": "Run native full acceptance",
                "supersessions": [], "retractions": []}
print(json.dumps(response))
