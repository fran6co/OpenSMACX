"""Installed command publication uses explicit decisions, not project adapters."""

import json

from match_recovery.cli import main
from match_recovery.model import canonical
from test_task_routes import routed
from test_coordination import setup
from test_source_configuration import configured
from test_task_source import project


def test_cli_exposes_target_pin_and_publishes_only_source_route(routed, capsys, monkeypatch, tmp_path):
    root, store, source, target, _, decision = routed
    assert main(["tasks", "status", target.task_id, "--root", str(root)]) == 0
    status = json.loads(capsys.readouterr().out)
    assert status["reference"] == decision["route"]["target"]
    proposal = root / "build" / "route.json"
    proposal.parent.mkdir(exist_ok=True)
    proposal.write_bytes(canonical(decision))
    target_history = store.decisions(target.task_id)
    monkeypatch.chdir(tmp_path)
    assert main(["tasks", "route", source.task_id, "--root", str(root),
                 "--file", "build/route.json", "--target-root", "."]) == 0
    result = json.loads(capsys.readouterr().out)
    assert result["published"] and not result["ready_for_collection"]
    assert result["verification"] == "not_measured"
    assert store.decisions(target.task_id) == target_history
    assert store.decisions(source.task_id)[-1] == decision["route"]
    assert store.status(source.task_id, store.scope(source.task_id)[0])["pending_discoveries"]


def test_cli_refuses_stale_decision_without_publishing(routed, capsys):
    root, store, source, _, _, decision = routed
    decision["route"]["discovery_event_id"] = "f" * 64
    proposal = root / "build" / "route.json"
    proposal.parent.mkdir(exist_ok=True)
    proposal.write_bytes(canonical(decision))
    before = store.decisions(source.task_id)
    assert main(["tasks", "route", source.task_id, "--root", str(root),
                 "--file", "build/route.json"]) == 3
    captured = capsys.readouterr()
    assert "exact discovery event" in captured.err and not captured.out
    assert store.decisions(source.task_id) == before
