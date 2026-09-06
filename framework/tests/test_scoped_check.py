import json

import pytest

from match_recovery.adapters.opensmacx_check import check_task
from match_recovery.cli import main
from test_obligations import project
from test_application_binary import project as binary_project
from test_binary import coff_fixture
from match_recovery.tasks import TaskStore


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


def prepare_shared(binary_project):
    root, task = binary_project
    (root / "src/widget.h").write_text("// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n")
    TaskStore(root).expand(task.task_id, request_id="layout", author=task.author, subjects=(),
                          editable_files=("src/widget.h",), evidence=("synthetic",), expected_revision=0)
    return root, task


def test_fresh_objects_are_shared_only_within_one_check(binary_project):
    root, task = prepare_shared(binary_project)
    calls = []

    def compiler(path, command, flags):
        calls.append(flags)
        assert flags[-1] == "/Z7"
        return coff_fixture()  # No debug types: reuse must not invent layout proof.

    for iteration in range(2):
        report = check_task(root, task.task_id, root / "original.exe", compiler=compiler,
                            command_for=lambda _: ("synthetic",))
        assert len(calls) == 2 * (iteration + 1)
        assert report["compilation"]["attempts"] == report["compilation"]["reused_objects"] == 2
        assert report["checks"]["binary"]["result"] == "pass"
        assert report["checks"]["layout"]["result"] == report["result"] == "unknown"
        for row in report["checks"]["binary"]["functions"]:
            assert all(variant["flags"][-1] == "/Z7" for variant in row["variants"])


def test_failed_compiles_are_not_reused(binary_project):
    root, task = prepare_shared(binary_project)

    def compiler(*args):
        raise ValueError("compiler failed")

    report = check_task(root, task.task_id, root / "original.exe", compiler=compiler,
                        command_for=lambda _: ("synthetic",))
    assert report["compilation"]["attempts"] == 4
    assert report["compilation"]["reused_objects"] == 0
    assert report["result"] == "unknown"


def test_different_commands_do_not_share_objects(binary_project):
    root, task = prepare_shared(binary_project)
    commands = iter([("first",), ("second",), ("third",)])
    report = check_task(root, task.task_id, root / "original.exe", compiler=lambda *_: coff_fixture(),
                        command_for=lambda _: next(commands))
    assert report["compilation"]["attempts"] == 4
    assert report["compilation"]["reused_objects"] == 0


def test_changed_header_prevents_cached_object_reuse(binary_project, monkeypatch):
    from match_recovery.adapters import opensmacx_check

    root, task = prepare_shared(binary_project)
    original = opensmacx_check.verify_layout

    def changed(*args, **kwargs):
        (root / "src/new.h").write_text("// newly introduced header\n")
        return original(*args, **kwargs)

    monkeypatch.setattr(opensmacx_check, "verify_layout", changed)
    with pytest.raises(ValueError, match="changed between scoped checks"):
        check_task(root, task.task_id, root / "original.exe", compiler=lambda *_: coff_fixture(),
                   command_for=lambda _: ("synthetic",))
