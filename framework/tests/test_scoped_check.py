import json

import pytest

from match_recovery.adapters.opensmacx_check import check_task
from match_recovery.cli import main
from test_obligations import project


@pytest.mark.parametrize("binary, layout, expected", [
    ("pass", "pass", "pass"), ("pass", "fail", "fail"),
    ("fail", "pass", "fail"), ("pass", "unknown", "unknown"),
    ("unknown", "pass", "unknown"), ("fail", "unknown", "unknown"),
    ("unknown", "fail", "unknown"),
])
def test_both_dimensions_are_checked_and_neither_is_full_acceptance(project, binary, layout, expected):
    root, task = project
    image = root / "image.bin"
    image.write_bytes(b"synthetic image")
    calls = []

    def checker(name, result):
        def run(*args):
            calls.append(name)
            return {"task_id": task.task_id, "result": result, "evidence": name}
        return run

    report = check_task(root, task.task_id, image, binary_checker=checker("binary", binary),
                        layout_checker=checker("layout", layout))
    assert calls == ["binary", "layout"]
    assert report["result"] == expected
    assert report["checks"]["layout"]["evidence"] == "layout"
    assert report["checks"]["binary"]["result"] == binary
    assert report["structure_verification"] == report["runtime_verification"] == "unknown"
    assert not report["ready_for_collection"] and not report["persisted"]


def test_failed_adapter_does_not_discard_other_dimension(project):
    root, task = project
    image = root / "image.bin"
    image.write_bytes(b"synthetic")

    def failed(*args):
        raise ValueError("unsupported binary input")

    report = check_task(root, task.task_id, image, binary_checker=failed,
                        layout_checker=lambda *_: {"task_id": task.task_id, "result": "fail"})
    assert report["result"] == "unknown"
    assert report["checks"]["binary"]["error"] == "unsupported binary input"
    assert report["checks"]["layout"]["result"] == "fail"


@pytest.mark.parametrize("change", ["source", "image", "new_header", "task"])
def test_joint_results_cannot_mix_different_input_snapshots(project, change):
    root, task = project
    image = root / "image.bin"
    image.write_bytes(b"synthetic")

    def first(*args):
        if change == "task":
            target = root / "recovery/tasks" / (task.task_id + ".json")
            data = json.loads(target.read_text())
            data["intent"] = "changed task intent"
            target.write_text(json.dumps(data))
        else:
            target = {"source": root / "src/f.cpp", "image": image,
                      "new_header": root / "src/new.h"}[change]
            target.write_text("changed input")
        return {"task_id": task.task_id, "result": "pass"}

    with pytest.raises(ValueError, match="changed between scoped checks"):
        check_task(root, task.task_id, image, binary_checker=first,
                   layout_checker=lambda *_: {"task_id": task.task_id, "result": "pass"})


@pytest.mark.parametrize("result, expected", [("pass", 0), ("fail", 1), ("unknown", 3)])
def test_joint_cli_summary_keeps_both_verdicts(monkeypatch, capsys, result, expected):
    monkeypatch.setattr("match_recovery.adapters.opensmacx_check.check_task",
                        lambda *_: {"result": result, "ready_for_collection": False,
                                    "checks": {"binary": {"result": result, "functions": []},
                                               "layout": {"result": "unknown", "members": [], "coverage": []}}})
    assert main(["tasks", "check", "fixture", "--image", "fixture.exe", "--summary"]) == expected
    report = json.loads(capsys.readouterr().out)
    assert report["checks"]["binary"] == {"result": result}
    assert report["checks"]["layout"] == {"result": "unknown", "coverage": []}
    assert not report["ready_for_collection"]
