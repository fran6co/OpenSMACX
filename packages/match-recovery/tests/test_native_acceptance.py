"""Native admission controls with real history/build and a synthetic COFF backend.

Global check results and structure/runtime proofs are controlled independently.
Positive protocol tests explicitly double qualified linked preservation; the
physical compiler fixture alone never supplies full outside-scope verification.
These tests prove orchestration, not Alpha Centauri semantics or a live pilot.
"""

from copy import deepcopy
from dataclasses import asdict, replace
import io
import json
import sys

import pytest

from match_recovery import acceptance_check as native
from match_recovery.build_check import check_build
from match_recovery.cli import main
from match_recovery.driver import Command, drive, working_state
from match_recovery.model import Dimension, Obligation, digest
from match_recovery.reviews import ReviewStore, acceptance_result
from match_recovery.source_obligations import task_obligations
from match_recovery.tasks import TaskStore
from test_application_commands import project
from test_migration_check import migration, request as migration_request, Compiler
from test_task_source import git


@pytest.fixture
def acceptance(migration, monkeypatch):
    root = migration
    store = TaskStore(root)
    task = store.read("owner")
    # Establish this fixture's policy BEFORE authorizing its task. Production
    # acceptance is never allowed to reset a baseline to hide policy changes.
    (root / "recovery/tasks/owner.json").unlink()
    (root / "recovery/gate.json").write_text(json.dumps({
        "schema_version": 1, "checks": {}, "inputs": [], "input_roots": [], "environment": []}))
    path = root / "recovery/project.json"
    config = json.loads(path.read_text())
    config.update(gate="recovery/gate.json", acceptance={"checks": {
        name: {"argv": [name], "timeout": 10} for name in ("structure", "runtime")},
        "inputs": ["build/proof-input"], "environment": ["ACCEPTANCE_TEST_INPUT"]})
    path.write_text(json.dumps(config))
    (root / "build").mkdir(exist_ok=True)
    (root / "build/proof-input").write_text("fixture proof input")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic acceptance policy baseline")
    store.create(replace(task, base_revision=git(root, "rev-parse", "HEAD")))
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize acceptance fixture")

    def gate(root, image, **kwargs):
        assert kwargs["retain_build"] is True
        built = check_build(root, retain=True)
        return {"result": built["result"], "input_fingerprint": digest(native.project_inputs(root, image)),
                "checks": [{"name": "build", "result": built["result"], "report": built}]}

    restore = native.measure_restoration
    preserve = native.check_preservation
    monkeypatch.setattr(native, "check_project", gate)
    monkeypatch.setattr(native, "measure_restoration", lambda root, name, image, **kwargs:
                        restore(root, name, image, compiler=Compiler(root), **kwargs))
    monkeypatch.setattr(native, "check_preservation", lambda root, name:
                        preserve(root, name, compiler=Compiler(root)))
    return root


def request(root):
    value = migration_request(root)
    value.pop("permission")
    value["protected"] = [{key: row[key] for key in ("entity_id", "dimension", "baseline", "obligation_id")}
                          for row in value["protected"]]
    value["verification_id"] = "fresh fixture invocation"
    value["review_policy"] = {}
    value["snapshot"] = value["review"]["snapshot"] = ReviewStore(root, "owner").snapshot(
        working_state(root), value["review_policy"])
    return value


def measure(root, payload=None):
    return native.measure_acceptance(root, "owner", root / "original.exe", payload or request(root))


@pytest.fixture
def proofs(monkeypatch):
    calls = []

    def execute(command, root, name, payload):
        calls.append(deepcopy(payload))
        assert command.argv[0] == payload["dimension"]
        built = payload["build"]
        assert (root / built["directory"] / built["outputs"][0]["path"]).read_bytes() == b"synthetic product"
        return 0, {"request_fingerprint": digest(payload), "result": "pass",
                   "evidence": ["controlled fixture proof"], "measurements": [], "current": {}}

    monkeypatch.setattr(native, "invoke", execute)
    return calls


@pytest.fixture
def qualified_preservation_protocol(acceptance, monkeypatch):
    """Complete-prerequisite protocol double, not physical linked evidence."""
    preserve = native.check_preservation

    def qualified(*args, **kwargs):
        return {**preserve(*args, **kwargs), "outside_scope_verification": "pass",
                "test_fixture": "synthetic complete linked prerequisite for protocol only"}

    monkeypatch.setattr(native, "check_preservation", qualified)


def test_native_acceptance_retains_one_fresh_build_for_both_proofs(acceptance, proofs, qualified_preservation_protocol):
    payload = request(acceptance)
    report = measure(acceptance, payload)
    assert acceptance_result(0, report, request=payload) == "pass"
    assert report["measurements"] and all(row["result"] == "pass" for row in report["measurements"])
    assert [row["dimension"] for row in proofs] == ["structure", "runtime"]
    assert proofs[0]["build"] == proofs[1]["build"]
    assert report["artifacts"] == {proofs[0]["build"]["directory"] + "/" + row["path"]: row["sha256"]
                                   for row in proofs[0]["build"]["outputs"]}
    assert len(list((acceptance / "build/products").iterdir())) == 1


def discover(root, name="owner"):
    store = TaskStore(root)
    if name != "owner":
        store.create(replace(store.read("owner"), task_id=name))
    task = store.read(name)
    return store.discover(name, request_id="unresolved-finding", author="worker",
                          hypothesis="Unresolved recovery evidence", affects=[task.subjects[0].entity_id],
                          evidence=["Controlled finding; review alone does not resolve it"],
                          expected_revision=len(store.decisions(name)))


@pytest.mark.parametrize("name", ["owner", "other-task"])
@pytest.mark.parametrize("entry", ["native", "cli"])
def test_direct_full_acceptance_cannot_bypass_any_tasks_pending_discovery(
        acceptance, proofs, monkeypatch, capsys, name, entry):
    discover(acceptance, name)
    payload = request(acceptance)  # Generic approval includes the current finding.
    before = working_state(acceptance)
    decisions = TaskStore(acceptance).decisions(name)
    def no_gate(*args, **kwargs):
        pytest.fail("unresolved discoveries must refuse before the gate or proof commands")
    monkeypatch.setattr(native, "check_project", no_gate)
    if entry == "native":
        with pytest.raises(ValueError, match="unresolved discoveries.*" + name):
            measure(acceptance, payload)
    else:
        monkeypatch.setattr("sys.stdin", io.StringIO(json.dumps(payload)))
        assert main(["tasks", "check-acceptance", "owner", "--root", str(acceptance),
                     "--image", "original.exe"]) == 3
        assert "unresolved discoveries" in capsys.readouterr().err
    assert not proofs
    assert working_state(acceptance) == before
    assert TaskStore(acceptance).decisions(name) == decisions


@pytest.mark.parametrize("name", ["owner", "other-task"])
@pytest.mark.parametrize("disposition", ["incorporated", "rejected"])
def test_explicitly_resolved_findings_do_not_block_full_acceptance(acceptance, proofs, qualified_preservation_protocol, name, disposition):
    finding = discover(acceptance, name)
    store = TaskStore(acceptance)
    store.resolve(name, discovery_id=finding["discovery_id"], request_id="resolve-finding",
                  author=store.read(name).author, disposition=disposition,
                  source_evidence=["Controlled explicit resolution; all acceptance checks still run"],
                  expected_revision=len(store.decisions(name)))
    decisions = store.decisions(name)
    assert measure(acceptance)["result"] == "pass"
    assert len(proofs) == 2
    assert store.decisions(name) == decisions


def test_resolving_another_tasks_finding_requires_a_new_full_review(acceptance, proofs, qualified_preservation_protocol):
    finding = discover(acceptance, "other-task")
    payload = request(acceptance)
    store = TaskStore(acceptance)
    store.resolve("other-task", discovery_id=finding["discovery_id"], request_id="resolve-finding",
                  author=store.read("other-task").author, disposition="rejected",
                  source_evidence=["Controlled explicit rejection after the previous review"],
                  expected_revision=len(store.decisions("other-task")))
    with pytest.raises(ValueError, match="reviewed source snapshot"):
        measure(acceptance, payload)
    assert not proofs
    assert measure(acceptance)["result"] == "pass"


@pytest.mark.parametrize("ignored", [False, True])
def test_unrelated_discovery_during_proof_invalidates_full_acceptance(acceptance, proofs, monkeypatch, ignored):
    store = TaskStore(acceptance)
    store.create(replace(store.read("owner"), task_id="other-task"))
    if ignored:
        path = acceptance / ".gitignore"
        path.write_text(path.read_text() + "recovery/history/\n")
    original = native.invoke
    def changed(*args):
        result = original(*args)
        before = working_state(acceptance)
        discover(acceptance, "other-task")
        if ignored:
            assert working_state(acceptance) == before  # Explicit journal freshness covers this gap.
        return result
    monkeypatch.setattr(native, "invoke", changed)
    with pytest.raises(ValueError, match="task changed|must participate in review snapshot"):
        measure(acceptance)
    assert len(proofs) == 1
    assert store.status("other-task", [store.read("other-task").subjects[0].entity_id])["pending_discoveries"]


@pytest.mark.parametrize("name", ["owner", "other-task"])
def test_ignored_resolved_findings_cannot_bypass_review_before_invocation(acceptance, proofs, monkeypatch, name):
    store = TaskStore(acceptance)
    if name != "owner":
        store.create(replace(store.read("owner"), task_id=name))
    path = acceptance / ".gitignore"
    path.write_text(path.read_text() + "recovery/history/\n")
    payload = request(acceptance)
    before = working_state(acceptance)
    finding = discover(acceptance, name)
    store.resolve(name, discovery_id=finding["discovery_id"], request_id="hidden-resolution",
                  author=store.read(name).author, disposition="rejected",
                  source_evidence=["Ignored resolution is absent from the independent review snapshot"],
                  expected_revision=len(store.decisions(name)))
    assert working_state(acceptance) == before
    def no_gate(*args, **kwargs):
        pytest.fail("unreviewed decisions must refuse before the gate")
    monkeypatch.setattr(native, "check_project", no_gate)
    with pytest.raises(ValueError, match="task decisions must participate in review snapshot"):
        measure(acceptance, payload)
    assert not proofs


def test_ignored_task_definition_cannot_bypass_full_review(acceptance, proofs, monkeypatch):
    path = acceptance / ".gitignore"
    path.write_text(path.read_text() + "recovery/tasks/other-task.json\n")
    payload = request(acceptance)
    before = working_state(acceptance)
    store = TaskStore(acceptance)
    store.create(replace(store.read("owner"), task_id="other-task"))
    assert working_state(acceptance) == before
    def no_gate(*args, **kwargs):
        pytest.fail("ignored task decisions must refuse before the gate")
    monkeypatch.setattr(native, "check_project", no_gate)
    with pytest.raises(ValueError, match="task decisions must participate in review snapshot"):
        measure(acceptance, payload)
    assert not proofs


def test_malformed_unrelated_task_journal_refuses_before_full_gate(acceptance, proofs, monkeypatch):
    discover(acceptance, "other-task")
    stream = TaskStore(acceptance).journal.stream_directory("task:other-task")
    (stream / "00000001.json").write_text("{}")
    payload = request(acceptance)
    def no_gate(*args, **kwargs):
        pytest.fail("invalid task journals must refuse before the gate")
    monkeypatch.setattr(native, "check_project", no_gate)
    with pytest.raises(ValueError, match="journal schema"):
        measure(acceptance, payload)
    assert not proofs


@pytest.mark.parametrize("damage", ["source", "untracked", "build-policy", "review-policy"])
def test_native_acceptance_refuses_source_changed_after_review_before_entry(acceptance, proofs, monkeypatch, damage):
    payload = request(acceptance)
    if damage == "review-policy":
        payload["review_policy"]["different"] = "policy after independent review"
    else:
        name = {"source": "code/unit.cc", "untracked": "not-reviewed.txt",
                "build-policy": "recovery/build.json"}[damage]
        path = acceptance / name
        path.write_text((path.read_text() if path.exists() else "") + "\n")
    def no_gate(*args, **kwargs):
        pytest.fail("a stale review must be refused before running the gate")
    monkeypatch.setattr(native, "check_project", no_gate)
    with pytest.raises(ValueError, match="reviewed source snapshot"):
        measure(acceptance, payload)
    assert not proofs


def test_publishing_and_committing_the_review_does_not_invalidate_it(acceptance, proofs, qualified_preservation_protocol):
    payload = request(acceptance)
    ReviewStore(acceptance, "owner").append(payload["review"], expected_revision=0)
    git(acceptance, "add", "recovery/history")
    git(acceptance, "commit", "-m", "Publish the independent review without editing source")
    assert measure(acceptance, payload)["result"] == "pass"
    assert len(proofs) == 2


@pytest.mark.parametrize("result", ["fail", "unknown"])
def test_incomplete_whole_gate_prevents_acceptance_proofs(acceptance, proofs, monkeypatch, result):
    monkeypatch.setattr(native, "check_project", lambda *a, **k: {
        "result": result, "checks": [{"name": "build", "result": result, "report": {"result": result}}]})
    report = measure(acceptance)
    assert report["result"] == result and not proofs
    assert report["structure"] == report["runtime"] == "unknown"


def test_failed_build_keeps_actionable_linker_evidence(acceptance, proofs, monkeypatch):
    monkeypatch.setattr(native, "check_project", lambda *a, **k: {
        "result": "fail", "checks": [{"name": "build", "result": "fail", "report": {
            "result": "fail", "commands": [{"result": "fail", "stderr": "undefined reference to Owner::add"}]}}]})
    report = measure(acceptance)
    assert report["result"] == "fail"
    assert "undefined reference to Owner::add" in report["findings"][0]["evidence"][0]
    assert not proofs


@pytest.mark.parametrize("damage,dimension", [("regression", "obligations"), ("wrong-layout", "structure")])
def test_passing_proofs_cannot_waive_native_binary_or_layout(acceptance, proofs, damage, dimension):
    path = acceptance / "code/unit.cc"
    path.write_text(path.read_text() + "// " + damage + "\n")
    report = measure(acceptance)
    assert report["result"] == report[dimension] == "fail"


@pytest.mark.parametrize("body,result", [("__asm { _emit 0x90 }", "fail"), ('R"x(unclosed', "unknown")])
def test_passing_proofs_cannot_waive_native_source_form(acceptance, proofs, body, result):
    path = acceptance / "code/unit.cc"
    path.write_text(path.read_text() + "\n" + body + "\n")
    report = measure(acceptance)
    assert report["result"] == report["structure"] == result
    assert any("cpp-source-form" in str(row["evidence"]) for row in report["findings"])
    assert len(proofs) == 2


@pytest.mark.parametrize("result", ["fail", "unknown"])
def test_outside_preservation_is_mandatory(acceptance, proofs, monkeypatch, result):
    monkeypatch.setattr(native, "check_preservation", lambda *a: {"result": result})
    report = measure(acceptance)
    assert report["result"] == report["obligations"] == result


def test_passing_full_proofs_cannot_promote_actual_compiled_preservation_to_linked_proof(acceptance, proofs, monkeypatch):
    preserve = native.check_preservation
    observed = []

    def physical(*args, **kwargs):
        report = preserve(*args, **kwargs)
        observed.append(report)
        return report

    monkeypatch.setattr(native, "check_preservation", physical)
    payload = request(acceptance)
    before = working_state(acceptance)
    report = measure(acceptance, payload)
    assert len(observed) == 1 and observed[0]["result"] == "pass"
    assert observed[0]["outside_scope_verification"] == "unknown" and observed[0]["scope_complete"] is False
    assert report["build"] == report["references"] == report["structure"] == report["runtime"] == "pass"
    assert [row["dimension"] for row in proofs] == ["structure", "runtime"]
    assert report["measurements"] and all(row["result"] == "pass" for row in report["measurements"])
    assert report["result"] == report["obligations"] == "unknown"
    assert acceptance_result(3, report, request=payload) == "unknown"
    assert any("Structure/runtime proof commands cannot waive" in row["message"]
               and "baseline/current linked selection" in row["message"] for row in report["findings"])
    assert working_state(acceptance) == before


@pytest.mark.parametrize("qualification,expected", [(None, "unknown"), (False, "unknown"),
                                                    ("unverified", "unknown"), ("unknown", "unknown"), ("fail", "fail")])
def test_full_proof_commands_cannot_replace_missing_or_refused_linked_qualification(
        acceptance, proofs, monkeypatch, qualification, expected):
    preservation = {"result": "pass", "scope_complete": False}
    if qualification is not None:
        preservation["outside_scope_verification"] = qualification
    monkeypatch.setattr(native, "check_preservation", lambda *args: preservation)
    report = measure(acceptance)
    assert report["structure"] == report["runtime"] == report["build"] == "pass"
    assert report["obligations"] == report["result"] == expected
    assert len(proofs) == 2
    assert any("linked-preservation" in str(row["evidence"]) for row in report["findings"])


def test_full_acceptance_cli_refuses_compiled_only_evidence_despite_passing_proofs(acceptance, proofs, monkeypatch, capsys):
    payload = request(acceptance)
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(payload)))
    assert main(["tasks", "check-acceptance", "owner", "--root", str(acceptance), "--image", "original.exe"]) == 3
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == report["obligations"] == "unknown"
    assert report["structure"] == report["runtime"] == "pass"
    assert acceptance_result(3, report, request=payload) == "unknown"


def test_changed_build_policy_is_not_a_pass(acceptance, proofs):
    path = acceptance / "recovery/build.json"
    config = json.loads(path.read_text())
    config["commands"][0]["argv"][2] += "; pass"
    path.write_text(json.dumps(config))
    assert measure(acceptance)["obligations"] == "unknown"


@pytest.mark.parametrize("dimension", ["structure", "runtime"])
def test_missing_proof_capability_is_unknown(acceptance, proofs, monkeypatch, dimension):
    # Keep baseline policy unchanged to isolate the absent capability.
    policy = native._policy
    def without(root):
        config, commands, inputs, environment = policy(root)
        commands.pop(dimension)
        return config, commands, inputs, environment
    monkeypatch.setattr(native, "_policy", without)
    report = measure(acceptance)
    assert report["result"] == report[dimension] == "unknown"


@pytest.mark.parametrize("damage", ["stale", "exit", "foreign_measurement", "no_evidence"])
def test_invalid_proof_receipts_never_pass(acceptance, proofs, monkeypatch, damage):
    original = native.invoke
    def damaged(*args):
        code, value = original(*args)
        if damage == "stale":
            value["request_fingerprint"] = "f" * 64
        elif damage == "exit":
            code = 1
        elif damage == "no_evidence":
            value["evidence"] = []
        else:
            row = request(acceptance)["protected"][0]
            value["measurements"] = [{"obligation_id": row["obligation_id"], "fingerprint": "a" * 64,
                                      "result": "pass", "evidence": "not this hook's authority"}]
            value["current"] = {row["obligation_id"]: "a" * 64}
        return code, value
    monkeypatch.setattr(native, "invoke", damaged)
    report = measure(acceptance)
    assert report["result"] == "unknown"
    assert report["structure"] == report["runtime"] == "unknown"


@pytest.mark.parametrize("damage", ["source", "image", "artifact", "symlink", "proof-input", "environment", "index"])
def test_mutations_during_proof_refuse_the_entire_epoch(acceptance, proofs, monkeypatch, damage):
    original = native.invoke
    def damaged(*args):
        response = original(*args)
        payload = args[-1]
        if damage == "environment":
            monkeypatch.setenv("ACCEPTANCE_TEST_INPUT", "concurrent change")
        elif damage == "index":
            git(acceptance, "add", "code/unit.cc")
        else:
            name = {"source": "code/unit.cc", "image": "original.exe", "proof-input": "build/proof-input"}.get(damage)
            path = acceptance / (name or (payload["build"]["directory"] + "/result"))
            if damage == "symlink":
                path.unlink()
                path.symlink_to(acceptance / "build/proof-input")
            else:
                path.write_bytes(path.read_bytes() + b"changed")
        return response
    if damage == "index":
        path = acceptance / "code/unit.cc"
        path.write_text(path.read_text() + "// pending candidate\n")
    monkeypatch.setattr(native, "invoke", damaged)
    with pytest.raises(ValueError, match="changed|pinned original|symlink"):
        measure(acceptance)


@pytest.mark.parametrize("damage", ["task", "protected", "interpretations"])
def test_request_cannot_omit_current_source_history(acceptance, proofs, damage):
    payload = request(acceptance)
    if damage == "task":
        payload["task"]["intent"] = "different task"
    else:
        payload[damage] = []
    with pytest.raises(ValueError, match="current task|omits current|inventory is stale"):
        measure(acceptance, payload)
    assert not proofs


def test_cli_reads_driver_request_and_returns_native_receipt(acceptance, proofs, qualified_preservation_protocol, monkeypatch, capsys):
    payload = request(acceptance)
    monkeypatch.setattr("sys.stdin", io.StringIO(json.dumps(payload)))
    assert main(["tasks", "check-acceptance", "owner", "--root", str(acceptance), "--image", "original.exe"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert acceptance_result(0, report, request=payload) == "pass"


def test_actual_json_proof_subprocesses_receive_the_fresh_artifact(acceptance, qualified_preservation_protocol, monkeypatch):
    script = (
        "import hashlib,json,sys; from pathlib import Path; r=json.load(sys.stdin); "
        "o=r['build']['outputs'][0]; p=Path(r['build']['directory'])/o['path']; "
        "assert hashlib.sha256(p.read_bytes()).hexdigest()==o['sha256']; "
        "d=hashlib.sha256((json.dumps(r,sort_keys=True,ensure_ascii=False,separators=(',',':'))+'\\n').encode()).hexdigest(); "
        "print(json.dumps(dict(request_fingerprint=d,result='pass',evidence=['synthetic artifact observed'],measurements=[],current={})))"
    )
    policy = native._policy
    def configured(root):
        config, commands, inputs, environment = policy(root)
        commands = {name: Command((sys.executable, "-c", script), 10) for name in commands}
        return config, commands, inputs, environment
    monkeypatch.setattr(native, "_policy", configured)
    assert measure(acceptance)["result"] == "pass"


@pytest.mark.parametrize("native_result", ["unknown", "fail"])
def test_hook_only_discharges_its_own_unmeasured_protected_dimension(acceptance, proofs, qualified_preservation_protocol, monkeypatch, native_result):
    payload = request(acceptance)
    item = Obligation(payload["protected"][0]["entity_id"], Dimension.STRUCTURE, "fixture structural duty")
    payload["protected"].append({**asdict(item), "obligation_id": item.obligation_id})
    original = native.invoke
    restore = native.measure_restoration
    def measured(*args, **kwargs):
        report = restore(*args, **kwargs)
        report["measurements"].append({"obligation_id": item.obligation_id, "result": native_result,
                                       "fingerprint": "a" * 64, "evidence": "controlled native structure result"})
        report["current"][item.obligation_id] = "a" * 64
        return report
    def proof(*args):
        code, report = original(*args)
        if args[-1]["dimension"] == "structure":
            assert args[-1]["protected"] == [payload["protected"][-1]]
            report["measurements"] = [{"obligation_id": item.obligation_id, "result": "pass",
                                       "fingerprint": digest(args[-1]), "evidence": "controlled structure proof"}]
            report["current"] = {item.obligation_id: digest(args[-1])}
        return code, report
    monkeypatch.setattr(native, "measure_restoration", measured)
    monkeypatch.setattr(native, "invoke", proof)
    assert measure(acceptance, payload)["result"] == ("pass" if native_result == "unknown" else "fail")


def test_driver_reaches_native_acceptance_after_independent_review(acceptance, proofs, qualified_preservation_protocol):
    task = TaskStore(acceptance).read("owner")
    stages = []
    def command(name):
        return {"argv": [name], "timeout": 10}
    def execute(cmd, root, name, payload):
        stage = cmd.argv[0]
        stages.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "obligations":
            return 0, task_obligations(root, name)
        if stage == "check":
            report = native.measure_restoration(root, name, root / "original.exe")
        elif stage == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["controlled independent review"], "feedback": "Native verification required"}
        else:
            assert stage == "acceptance"
            report = native.measure_acceptance(root, name, root / "original.exe", payload)
        return {"pass": 0, "fail": 1, "unknown": 3}[report["result"]], report
    result = drive(acceptance, task.task_id, {"schema_version": 1, "target": task.target,
        **{name: command(name) for name in ("context", "check", "obligations", "acceptance")}},
        {"author": "worker", "command": command("worker")}, iterations=1,
        reviewer_configuration={"author": "independent", "command": command("review")}, execute=execute)
    assert result["ready_for_collection"] and "worker" not in stages
    assert stages.index("review") < stages.index("acceptance")
