"""CLI routing controls for advisory recovery-speed commands."""

import json
from pathlib import Path
import sys
from types import ModuleType

import pytest

from match_recovery.cli import main, parser


def project(tmp_path):
    root = tmp_path / "project"
    recovery = root / "recovery"
    recovery.mkdir(parents=True)
    (recovery / "project.json").write_text(json.dumps({"schema_version": 1, "source": {}}))
    return root


def fake_module(monkeypatch, name, **attributes):
    module = ModuleType(name)
    for key, value in attributes.items():
        setattr(module, key, value)
    monkeypatch.setitem(sys.modules, name, module)
    return module


def test_public_parser_exposes_advisory_commands_and_local_variant_flag():
    cli = parser()
    assert "prioritize" in cli._subparsers._group_actions[0].choices
    assert "triage" in cli._subparsers._group_actions[0].choices
    assert "throughput" in cli._subparsers._group_actions[0].choices

    args = cli.parse_args([
        "tasks", "variants", "owner", "--root", ".", "--image", "game.exe",
        "--deck", "deck.json", "--local",
    ])
    assert args.action == "variants" and args.local
    assert args.jobs == 1 and not args.compiled_diagnostics


def test_prioritize_dispatches_explicit_targets_and_all_choice(tmp_path, monkeypatch, capsys):
    root = project(tmp_path)
    calls = []

    def recommend(root_arg, image, targets, references, **kwargs):
        calls.append((root_arg, image, targets, references, kwargs))
        return {"operation": "advisory", "count": 0}

    fake_module(monkeypatch, "match_recovery.recovery_examples", recommend_examples=recommend)
    common = ["--root", str(root), "--image", "original.exe", "--reference", "sibling"]
    assert main(["prioritize", "owner-a", *common, "--limit", "7", "--top-k", "2"]) == 0
    assert json.loads(capsys.readouterr().out) == {"operation": "advisory", "count": 0}
    assert calls[-1] == (root, Path("original.exe"), ["owner-a"], ["sibling"],
                         {"limit": 7, "top_k": 2, "all_candidates": False})

    assert main(["prioritize", "--all", *common]) == 0
    capsys.readouterr()
    assert calls[-1][2] == [] and calls[-1][-1]["all_candidates"] is True


@pytest.mark.parametrize("target_args", [[], ["owner-a", "--all"]])
def test_prioritize_refuses_neither_or_both_target_modes(tmp_path, capsys, target_args):
    root = project(tmp_path)
    code = main(["prioritize", *target_args, "--root", str(root), "--image", "x.exe",
                 "--reference", "example"])
    assert code == 3
    assert "prioritize requires explicit targets or --all" in capsys.readouterr().err


def test_triage_dispatch_is_advisory_and_passes_loaded_report(tmp_path, monkeypatch, capsys):
    root = project(tmp_path)
    report_path = root / "report.json"
    report = {"rows": [{"address": "0x401000", "verdict": "MISMATCH"}]}
    report_path.write_text(json.dumps(report))
    calls = []
    fake_module(monkeypatch, "match_recovery.mismatch_triage",
                triage_report=lambda value: calls.append(value) or {"operation": "triage", "advisory": True})

    assert main(["triage", "--root", str(root), "--report", "report.json"]) == 0
    assert calls == [report]
    assert json.loads(capsys.readouterr().out) == {"operation": "triage", "advisory": True}


def test_throughput_dispatch_requires_only_declared_observation_schema(tmp_path, monkeypatch, capsys):
    root = project(tmp_path)
    observation_path = root / "observations.json"
    observation_path.write_text(json.dumps({"before": {}, "after": {}}))
    assert main(["throughput", "--root", str(root), "--observations", "observations.json"]) == 3
    assert "throughput requires explicit paired observations" in capsys.readouterr().err

    valid = {"original_identities": ["entity-a"], "before": {}, "after": {}, "timings": []}
    observation_path.write_text(json.dumps(valid))
    calls = []
    fake_module(monkeypatch, "match_recovery.throughput",
                throughput_report=lambda **value: calls.append(value) or {"operation": "throughput"})
    assert main(["throughput", "--root", str(root), "--observations", "observations.json"]) == 0
    assert calls == [valid]
    assert json.loads(capsys.readouterr().out) == {"operation": "throughput"}

    observation_path.write_text(json.dumps({**valid, "unexpected": True}))
    assert main(["throughput", "--root", str(root), "--observations", "observations.json"]) == 3
    assert "throughput requires explicit paired observations" in capsys.readouterr().err


@pytest.mark.parametrize(
    "extra, message",
    [(["--jobs", "2"], "requires --jobs 1"),
     (["--compiled-diagnostics"], "does not support --compiled-diagnostics")],
)
def test_local_variant_dispatch_rejects_unsupported_options(tmp_path, capsys, extra, message):
    root = project(tmp_path)
    code = main(["tasks", "variants", "owner", "--root", str(root), "--image", "game.exe",
                 "--deck", "deck.json", "--local", *extra])
    assert code == 3
    assert message in capsys.readouterr().err
