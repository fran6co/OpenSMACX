"""Deterministic local agent subprocesses for native acceptance damage controls.

These are test doubles for reasoning only. The driver, coordinator authorization,
compiler, linker, source checks and acceptance verifier are the real package path.
No provider service is called.
"""

import json
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
if role == "worker":
    header = Path("src/owner.h")
    # Reason from current source, including after a new bounded invocation;
    # invocation-local round numbers are not a recovery-state ledger.
    step = 1 if "int raw_[2];" in header.read_text() else 2
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
    finding = request["pending_discoveries"][0]
    extra = sorted(set(finding["affects"]) - set(request["subjects"]))
    response = {"snapshot": request["snapshot"], "action": "expand" if extra else "incorporate",
                "discovery_id": finding["discovery_id"], "subjects": extra, "editable_files": [],
                "evidence": ["Test source expresses the base/member relationship; include the known caller before independent review. This is not acceptance."]}
else:
    assert role == "reviewer"
    response = {"snapshot": request["snapshot"], "decision": "approve",
                "evidence": ["test-owned original Base and Member"], "feedback": "Run native full acceptance",
                "supersessions": [], "retractions": []}
print(json.dumps(response))
