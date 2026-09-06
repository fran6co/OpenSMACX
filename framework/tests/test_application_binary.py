"""Task execution and refusal tests; all executable bytes are synthetic."""

from dataclasses import replace
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.adapters.opensmacx_binary import select_bindings, verify_task
from match_recovery.adapters.vc6 import FLAG_SETS, compile_object
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.cli import main
from match_recovery.model import Origin, Result, Span
from match_recovery.partition import Candidate, partition
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.tasks import Task, TaskStore

from test_binary import coff_fixture, pe_fixture


@pytest.fixture
def project(tmp_path):
    raw = pe_fixture()
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    (tmp_path / "src").mkdir()
    (tmp_path / "src/IMAGE_PIN").write_text("sha256 " + pin + "\n")
    (tmp_path / "src/widget.cpp").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\nvoid f() {}\n")
    (tmp_path / "original.exe").write_bytes(raw)
    task = Task("widget", "opensmacx", "coordinator", "Recover Widget", "b" * 40,
                (origin,), ("src/widget.cpp",), ("synthetic source evidence",))
    TaskStore(tmp_path).create(task)
    return tmp_path, task


def verify(project, compiler=lambda *_: coff_fixture()):
    root, task = project
    return verify_task(root, task.task_id, root / "original.exe", compiler=compiler,
                       command_for=lambda _: ("synthetic-compiler",))


def test_task_runs_fresh_trials_without_banking_or_claiming_collection(project):
    calls = []
    root, task = project
    source = (root / "src/widget.cpp").read_bytes()

    def compile_source(path, command, flags):
        calls.append((path, command, flags))
        return coff_fixture()

    result = verify(project, compile_source)
    assert result["result"] == Result.PASS
    assert result["compile_attempts"] == len(calls) == len(FLAG_SETS)
    assert result["counts"] == {"pass": 1}
    assert result["structure_verification"] == result["build_verification"] == Result.UNKNOWN
    assert not result["ready_for_collection"] and not result["persisted"]
    assert (root / "src/widget.cpp").read_bytes() == source
    assert not (root / "recovery/history").exists()
    assert verify(project) == result


@pytest.mark.parametrize("content", ["unknown", "fail"])
def test_passing_functions_cannot_hide_unverified_generated_tables(project, monkeypatch, content):
    monkeypatch.setattr("match_recovery.adapters.opensmacx_binary.check_generated",
                        lambda *_: ({}, [{"symbol": "??_7Widget@@6B@", "definition": "pass", "content": content}]))
    result = verify(project)
    assert result["counts"] == {"pass": 1}
    assert result["result"] == content
    assert len(result["generated_tables"]) == len(FLAG_SETS)


def test_many_subjects_share_one_compile_per_tu_and_flag_set(project):
    root, task = project
    other = replace(task.subjects[0], address=0x401010)
    with (root / "src/widget.cpp").open("a") as output:
        output.write("// ORIGINAL: 0x00401010 _g 0x00401010-0x00401011\nvoid g() {}\n")
    TaskStore(root).expand(task.task_id, request_id="expand", author=task.author, subjects=(other,),
                           editable_files=(), evidence=("synthetic",), expected_revision=0)
    result = verify(project)
    assert result["selected_entities"] == 2
    assert result["compile_attempts"] == 2  # not one compile per function
    assert result["counts"] == {"pass": 1, "unknown": 1}  # _g is absent from the fixture object


def test_all_flags_must_fail_before_reporting_failure(project):
    def partially_unmeasurable(path, command, flags):
        if flags == FLAG_SETS[1]:
            raise ValueError("compiler unavailable")
        return coff_fixture(b"\x90")
    result = verify(project, partially_unmeasurable)
    assert result["result"] == Result.UNKNOWN
    assert [item["result"] for item in result["functions"][0]["variants"]] == [Result.FAIL, Result.UNKNOWN]
    assert verify(project, lambda *_: coff_fixture(b"\x90"))["result"] == Result.FAIL


def test_one_complete_passing_trial_is_sufficient_but_never_a_full_gate(project):
    def first_only(path, command, flags):
        if flags == FLAG_SETS[1]:
            raise ValueError("unmeasurable second trial")
        return coff_fixture()
    result = verify(project, first_only)
    assert result["result"] == Result.PASS
    assert not result["ready_for_collection"]


@pytest.mark.parametrize("change", ["source", "new-header", "task"])
def test_racing_source_and_scope_changes_discard_the_measurement(project, change):
    root, task = project
    calls = 0

    def changed(path, command, flags):
        nonlocal calls
        calls += 1
        if calls == 1:
            if change == "task":
                TaskStore(root).expand(task.task_id, request_id="new-scope", author=task.author,
                                       subjects=(), editable_files=("src/new.h",), evidence=("new evidence",),
                                       expected_revision=0)
            else:
                target = "widget.cpp" if change == "source" else "new.h"
                (root / "src" / target).write_text("// changed during compilation\n")
        return coff_fixture()

    with pytest.raises(ValueError, match="changed during"):
        verify(project, changed)


def test_wrong_image_is_refused_before_compiling(project):
    root, _ = project
    (root / "original.exe").write_bytes(pe_fixture(b"\x90"))
    def must_not_compile(*_):
        pytest.fail("wrong image reached compiler")
    with pytest.raises(ValueError, match="digest"):
        verify(project, must_not_compile)


def test_missing_build_entry_is_unknown_not_borrowed(project):
    root, task = project
    def absent(_):
        raise ValueError("not a configured build input")
    result = verify_task(root, task.task_id, root / "original.exe", command_for=absent)
    assert result["result"] == Result.UNKNOWN
    assert result["compile_attempts"] == 0


def test_seeds_and_ambiguous_external_bindings_cannot_become_selected_source():
    origin = Origin("a" * 64, "pe:va", 0x401000)
    candidate = Candidate(origin, (Span(0x401000, 0x401001),), "src/f.cpp", 1, False, "_f")
    other = replace(candidate, origin=replace(origin, address=0x402000),
                    spans=(Span(0x402000, 0x402001),), path="src/g.cpp")
    selected = frozenset({origin.entity_id})
    bindings, refused, targets = select_bindings(partition([candidate, other]), selected)
    assert not bindings and "_f" not in targets
    assert refused[0]["result"] == Result.UNKNOWN
    bindings, refused, targets = select_bindings(partition([replace(candidate, seed=True)]), selected)
    assert not bindings and not targets and "seeds" in refused[0]["reason"]


def test_exact_external_targets_are_applied_without_reading_image_operands():
    raw = pe_fixture(b"\xe8\xfb\x00\0\0\xc3")  # call 0x401100
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    binding = FunctionBinding(origin, (Span(0x401000, 0x401006),), "_f", "src/f.cpp")
    objects = {binding.unit: Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))}
    image = PeImage(raw, origin.image_sha256)
    assert check_objects([binding], objects, image)[0]["result"] == Result.UNKNOWN
    assert check_objects([binding], objects, image, external_targets={"_g": 0x401100})[0]["result"] == Result.PASS
    assert check_objects([binding], objects, image, external_targets={"_g": 0x401200})[0]["result"] == Result.FAIL
    with pytest.raises(ValueError, match="different image"):
        check_objects([replace(binding, origin=replace(origin, image_sha256="b" * 64))], objects, image)


def test_nonzero_compiler_exit_with_an_object_is_still_unmeasurable(monkeypatch, tmp_path):
    def failed(argv, *, cwd, **kwargs):
        (Path(cwd) / "unit.obj").write_bytes(coff_fixture())
        return subprocess.CompletedProcess(argv, 1, "partial object", "compiler error")
    monkeypatch.setattr(subprocess, "run", failed)
    with pytest.raises(ValueError, match="compiler error"):
        compile_object(tmp_path / "f.cpp", ("cl",), FLAG_SETS[0])


def test_timeout_is_unmeasurable_not_a_lost_task(monkeypatch, tmp_path):
    def timeout(argv, **kwargs):
        raise subprocess.TimeoutExpired(argv, 300)
    monkeypatch.setattr(subprocess, "run", timeout)
    with pytest.raises(ValueError, match="exceeded"):
        compile_object(tmp_path / "f.cpp", ("cl",), FLAG_SETS[0])


@pytest.mark.parametrize("result, expected", [(Result.PASS, 0), (Result.FAIL, 1), (Result.UNKNOWN, 3)])
def test_cli_preserves_check_exit_semantics(monkeypatch, capsys, result, expected):
    monkeypatch.setattr("match_recovery.adapters.opensmacx_binary.verify_task",
                        lambda *args: {"result": result, "functions": [], "ready_for_collection": False})
    assert main(["tasks", "check-binary", "widget", "--image", "fixture.exe", "--summary"]) == expected
    assert '"ready_for_collection": false' in capsys.readouterr().out
