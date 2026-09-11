"""Native owner publication does not resolve another task's routed finding.

The existing migration fixture supplies synthetic COFF and independent
source/mapping/claim frontend doubles. Restoration, layout, historical claims,
preservation damage controls, fixture product build, admission, and Git
candidate/destination publication execute natively. This is not game recovery.
"""

from pathlib import Path
import hashlib

import pytest

from match_recovery import migration_check as native
from match_recovery.driver import working_state
from match_recovery.integration_prepare import prepare_integration
from match_recovery.integration_publish import publish_integration
from match_recovery.model import Origin
from match_recovery.preservation import check_preservation
from match_recovery.restoration import measure_restoration
from match_recovery.source_obligations import task_obligations
from match_recovery.task_routes import incoming_routes, route_task, target_reference
from match_recovery.tasks import Task, TaskStore
from test_application_commands import project
from test_migration_check import Compiler, migration
from test_task_source import git


@pytest.fixture
def dependency(migration):
    root = migration
    store = TaskStore(root)
    owner = store.read("owner")
    # The second owner remains pending/unverified. It has a distinct original
    # identity and source home; its scope is never folded into the callee task.
    (root / "code/request.hpp").write_text(
        "// ORIGINAL: 0x00401010 _request 0x00401010-0x00401011\nvoid request();\n")
    git(root, "add", "code/request.hpp")
    git(root, "commit", "-m", "Synthetic requesting source home")
    requester = Task("requester", owner.target, owner.author, "Recover independent requesting owner",
                     git(root, "rev-parse", "HEAD"),
                     (Origin(owner.subjects[0].image_sha256, "pe:va", 0x401010),),
                     ("code/request.hpp",), ("Synthetic cross-owner dependency",))
    store.create(requester)
    finding = store.discover("requester", request_id="cross-owner", author="worker",
                             hypothesis="Requesting behavior depends on the other owner's implementation",
                             affects=[requester.subjects[0].entity_id, owner.subjects[0].entity_id],
                             evidence=["Keep both ownership impacts until independently resolved"], expected_revision=0)
    original, = store.journal.read("task:requester")
    decision = {"request_id": "ask-existing-owner", "expected_revision": 1,
                "route": {"kind": "discovery_routed", "author": requester.author,
                          "discovery_id": finding["discovery_id"], "discovery_event_id": original.event_id,
                          "target": target_reference(store, "owner"), "subjects": [owner.subjects[0].entity_id],
                          "evidence": ["Route only the callee identity; grant no resolution or scope"]}}
    route_task(root, "requester", decision)
    git(root, "add", "recovery/tasks", "recovery/history")
    git(root, "commit", "-m", "Preserve original pending cross-owner request")
    return root, owner, requester, finding, decision["route"]


def candidate_for(dependency, *, regression=False):
    root = dependency[0]
    destination = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "incoming-owner")
    unit = root / "code/unit.cc"
    unit.write_text(unit.read_text() + ("// regression\n" if regression else "// recovered owner spelling\n"))
    git(root, "add", "code/unit.cc")
    git(root, "commit", "-m", "Synthetic owned implementation change")
    source = git(root, "rev-parse", "HEAD")
    git(root, "switch", "-c", "destination", destination)
    context = {"author": dependency[1].author, "source_revision": source, "target_revision": destination,
               "evidence": ["Verify this owner change without resolving the requesting task"]}
    prepared = prepare_integration(root, "owner", context, worktree="build/dependency-candidate",
                                   branch="dependency-candidate")
    assert prepared["candidate_created"] and not prepared["ready_for_collection"]
    assert prepared["verification"] == "not_measured"
    candidate = Path(prepared["worktree"])
    permission = {"author": dependency[1].author, "candidate_revision": prepared["commit"],
                  "destination_branch": "refs/heads/destination", "integration": context,
                  "message": "Collect freshly verified independent owner", "allow_migration": False}
    return candidate, permission


def publish(dependency, candidate, permission, monkeypatch, *, fail_destination=False):
    root, owner, _, _, _ = dependency
    calls, reports, compiled = [], [], []

    class ObservedCompiler(Compiler):
        def overlay(self, source, replacements, arguments, flags):
            compiled.append((self.root, source, bool(replacements)))
            data = super().overlay(source, replacements, arguments, flags)
            if fail_destination and self.root == root:
                data = bytearray(data)
                data[100] = 0xCC  # Measured destination byte failure, not a fabricated verifier verdict.
                data = bytes(data)
            return data

    monkeypatch.setattr(native, "measure_restoration", lambda root, name, image, **kwargs:
                        measure_restoration(root, name, image, compiler=ObservedCompiler(root), **kwargs))
    monkeypatch.setattr(native, "check_preservation", lambda root, name, **kwargs:
                        check_preservation(root, name, compiler=ObservedCompiler(root), **kwargs))
    build = native.check_build

    def observed_build(checkout):
        report = build(checkout)
        reports.append((checkout, "build", report))
        return report

    monkeypatch.setattr(native, "check_build", observed_build)

    def command(stage):
        return {"argv": [stage], "timeout": 10}

    config = {"schema_version": 1, "target": owner.target, "acceptance_scope": "change",
              **{stage: command(stage) for stage in ("context", "check", "obligations", "migration_check")}}

    def execute(command, checkout, name, payload):
        stage = command.argv[0]
        calls.append((checkout, stage))
        assert name == "owner"
        if stage == "context":
            return 0, {"incoming_routes": incoming_routes(TaskStore(checkout), name)}
        if stage == "obligations":
            return 0, task_obligations(checkout, name)
        if stage == "check":
            assert payload["context"]["incoming_routes"]
            report = native.measure_restoration(checkout, name, checkout / "original.exe")
            reports.append((checkout, stage, report))
            return {"pass": 0, "fail": 1, "unknown": 3}[report["result"]], report
        if stage == "review":
            assert payload["context"]["incoming_routes"]
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["Independent synthetic owner review; requesting discovery stays pending"],
                       "feedback": "Approve only the measured owner change"}
        assert stage == "migration_check", "Verification-only collection must never launch a worker"
        report = native.measure_migration(checkout, name, checkout / "original.exe", payload)
        reports.append((checkout, stage, report))
        return 0, report

    result = publish_integration(root, "owner", candidate, permission, configuration=config,
                                 worker_configuration={"author": "worker", "command": command("worker")},
                                 reviewer_configuration={"author": "reviewer", "command": command("review")},
                                 execute=execute)
    return result, calls, reports, compiled


def assert_pending(dependency, checkout):
    _, owner, requester, finding, route = dependency
    store = TaskStore(checkout)
    assert store.read("owner") == owner and store.read("requester") == requester
    assert store.scope("owner") == (frozenset(item.entity_id for item in owner.subjects), frozenset(owner.editable_files))
    assert store.scope("requester") == (frozenset(item.entity_id for item in requester.subjects), frozenset(requester.editable_files))
    assert store.decisions("owner") == ()
    assert store.decisions("requester") == (finding, route)
    status = store.status("requester", store.scope("requester")[0])
    assert status["pending_discoveries"] == [finding] and status["outgoing_routes"] == [route]
    assert status["scope_expansion_required"] == [owner.subjects[0].entity_id]
    assert not status["ready_for_collection"]
    incoming, = incoming_routes(store, "owner")
    assert incoming["discovery"] == finding and incoming["route"] == route
    before = working_state(checkout)
    with pytest.raises(ValueError, match="beyond the approved task scope"):
        store.resolve("requester", discovery_id=finding["discovery_id"], request_id="not-a-closure",
                      author=requester.author, disposition="incorporated",
                      source_evidence=["Owner publication alone cannot resolve the whole finding"], expected_revision=2)
    assert working_state(checkout) == before


def test_native_owner_candidate_and_destination_verify_while_cross_owner_request_stays_pending(dependency, monkeypatch):
    root = dependency[0]
    candidate, permission = candidate_for(dependency)
    histories = {path.relative_to(root): path.read_bytes()
                 for path in TaskStore(root).journal.stream_directory("task:requester").glob("*.json")}
    result, calls, reports, compiled = publish(dependency, candidate, permission, monkeypatch)
    assert result["destination_updated"] and result["destination_verified"]
    assert result["stop_reason"] == "destination_accepted" and not result["ready_for_collection"]
    for checkout in (candidate, root):
        assert calls.count((checkout, "review")) == calls.count((checkout, "migration_check")) == 1
        assert any(path == checkout for path, _, _ in compiled)
        migration_report, = [report for path, stage, report in reports if path == checkout and stage == "migration_check"]
        assert migration_report["build"] == migration_report["outside_scope"] == migration_report["references"] == "pass"
        assert migration_report["measurements"] and all(row["result"] == "pass" for row in migration_report["measurements"])
        built, = [report for path, stage, report in reports if path == checkout and stage == "build"]
        assert built["fresh_directory"] and built["commands"][0]["exit_code"] == 0
        assert built["outputs"] == [{"path": "result", "format": "file", "result": "pass",
                                     "sha256": hashlib.sha256(b"synthetic product").hexdigest(),
                                     "size": len(b"synthetic product")}]
        assert {name: (checkout / name).read_bytes() for name in histories} == histories
        assert_pending(dependency, checkout)
        assert git(checkout, "status", "--porcelain") == ""
    measured = [report for _, stage, report in reports if stage == "migration_check"]
    assert len(measured) == 2 and measured[0]["snapshot"] != measured[1]["snapshot"]


@pytest.mark.parametrize("failed_at", ["candidate", "destination"])
def test_owner_byte_failure_never_closes_source_finding_or_rolls_back_transferred_code(dependency, monkeypatch, failed_at):
    root = dependency[0]
    candidate, permission = candidate_for(dependency, regression=failed_at == "candidate")
    before = working_state(root)
    result, calls, reports, compiled = publish(dependency, candidate, permission, monkeypatch,
                                              fail_destination=failed_at == "destination")
    assert not result["destination_verified"] and not result["ready_for_collection"]
    assert result["stop_reason"] == failed_at + "_verification_required"
    assert result["destination_updated"] is (failed_at == "destination")
    failed_checkout = candidate if failed_at == "candidate" else root
    assert any(path == failed_checkout and stage == "check" and report["restoration"] == "fail"
               for path, stage, report in reports)
    assert any(path == failed_checkout for path, _, _ in compiled)
    if failed_at == "candidate":
        assert working_state(root) == before
        assert all(checkout == candidate for checkout, _ in calls)
    else:
        assert git(root, "rev-parse", "HEAD") == result["verified_candidate_commit"]
        assert "recovered owner spelling" in (root / "code/unit.cc").read_text()
    assert_pending(dependency, root)
    assert_pending(dependency, candidate)
