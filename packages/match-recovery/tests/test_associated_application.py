"""Associated-data evidence must survive ordinary checks and source trials."""

from dataclasses import replace
import hashlib
import json
from pathlib import Path

import pytest

from match_recovery.application_binary import check_compiled, verify_task
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Result
from match_recovery.source_integrity import check_annotation_text
from match_recovery.tasks import Task, TaskStore
from match_recovery.variants import _require_associated_unchanged, measure_variants
from test_associated_binary import fixture


def context():
    obj, subject, span, image, associated, targets = fixture()
    binding = FunctionBinding(associated[0].owner, (span,), subject.name, associated[0].unit)
    return binding, {binding.unit: obj}, image, associated


def comments(bindings):
    return "".join(
        f"// RECOVERY_ASSOCIATED: v1 {row.kind} 0x{row.owner.address:X} "
        f"0x{row.span.start:X} 0x{row.span.end:X} 0x{row.evidence.address:X}\n"
        for row in bindings)


def project(tmp_path):
    binding, objects, image, associated = context()
    unit = "code/unit.cc"
    associated = tuple(replace(row, unit=unit) for row in associated)
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    (tmp_path / "image.exe").write_bytes(image.data)
    (tmp_path / "code/PIN").write_text("sha256 " + binding.origin.image_sha256 + "\n")
    body = (comments(associated)
            + f"// ORIGINAL: 0x{binding.origin.address:X} {binding.symbol} "
            + f"0x{binding.spans[0].start:X}-0x{binding.spans[0].end:X}\n"
            + "void f() { /* baseline */ }\n")
    (tmp_path / unit).write_text(body)
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va",
                   "roots": ["code"], "include_paths": ["code"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
                        "flag_sets": [["first"], ["second"]], "layout_flags": [],
                        "unit_extensions": [".cc"], "inputs": [], "input_roots": []},
    }))
    task = Task("switch-owner", "synthetic", "coordinator", "Recover the switch owner",
                "a" * 40, (binding.origin,), (unit,), ("synthetic original switch evidence",))
    TaskStore(tmp_path).create(task)
    return tmp_path, task, next(iter(objects.values())).raw_data


def test_bound_and_compiled_checks_consume_only_explicit_associated_relations():
    binding, objects, image, associated = context()
    old = check_objects([binding], objects, image)
    assert old == check_objects([binding], objects, image, associated_bindings=())
    assert old[0]["result"] != Result.PASS  # No implicit tail truncation.
    rows = check_objects([binding], objects, image, associated_bindings=associated)
    assert rows[0]["result"] == Result.PASS, rows
    proofs = check_compiled([binding], (), (), objects, image, {}, associated_bindings=associated)
    assert proofs[0] == rows
    assert all(not group for group in proofs[1:])
    proof = rows[0]
    assert proof["span"] == proof["source_body_span"]
    assert proof["comparison_span"]["end"] > proof["span"]["end"]
    assert proof["linked_selection"] == proof["runtime"] == Result.UNKNOWN


@pytest.mark.parametrize("damage", ["unit", "source_spans", "pin", "duplicate"])
def test_invalid_associated_population_cannot_borrow_ordinary_code_proof(damage):
    binding, objects, image, associated = context()
    if damage == "unit":
        associated = tuple(replace(row, unit="different.cc") for row in associated)
    elif damage == "source_spans":
        binding = replace(binding, source_spans=binding.spans)
    elif damage == "pin":
        associated = tuple(replace(row, owner=replace(row.owner, image_sha256="a" * 64),
                                   evidence=replace(row.evidence, image_sha256="a" * 64))
                           for row in associated)
    else:
        associated = (*associated, associated[0])
    try:
        rows = check_objects([binding], objects, image, associated_bindings=associated)
    except ValueError:
        return
    assert rows[0]["result"] != Result.PASS


def test_native_task_checks_each_recipe_with_its_own_associated_evidence(tmp_path):
    root, task, raw = project(tmp_path)
    calls = []

    def compile(source, arguments, flags):
        calls.append((source.name, flags))
        return raw

    report = verify_task(root, task.task_id, root / "image.exe", compiler=compile)
    assert report["result"] == Result.PASS, report
    assert calls == [("unit.cc", ("first",)), ("unit.cc", ("second",))]
    row, = report["functions"]
    assert len(row["variants"]) == 2
    assert all(trial["result"] == Result.PASS for trial in row["variants"])
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["byte", "compile"])
def test_native_task_never_borrows_another_recipe_success(tmp_path, damage):
    root, task, raw = project(tmp_path)

    def compile(source, arguments, flags):
        if flags == ("second",):
            if damage == "compile":
                raise ValueError("synthetic compiler refusal")
            mutated = bytearray(raw)
            # First code byte, not object metadata or a reference operand.
            mutated[60] ^= 1
            return bytes(mutated)
        return raw

    report = verify_task(root, task.task_id, root / "image.exe", compiler=compile)
    row, = report["functions"]
    first, second = row["variants"]
    assert first["result"] == Result.PASS
    assert second["result"] != Result.PASS
    # Existing flag sets are alternatives: the complete first trial may pass,
    # but its proof cannot be copied into the failed/incomplete second trial.
    assert row["result"] == Result.PASS
    assert not report["ready_for_collection"]


def test_two_partial_recipe_results_never_combine_into_one_success(tmp_path):
    root, task, raw = project(tmp_path)
    damaged_selector = fixture(selector_byte=1)[0].raw_data

    def compile(source, arguments, flags):
        if flags == ("first",):
            changed = bytearray(raw)
            changed[60] ^= 1
            return bytes(changed)
        return damaged_selector

    report = verify_task(root, task.task_id, root / "image.exe", compiler=compile)
    row, = report["functions"]
    assert all(trial["result"] != Result.PASS for trial in row["variants"])
    assert report["result"] != Result.PASS
    assert not report["ready_for_collection"]


def test_body_trials_freeze_interpretations_but_allow_line_number_changes():
    _, _, _, associated = context()
    raw = comments(associated).encode()
    pin = associated[0].owner.image_sha256
    _require_associated_unchanged({"fixture.cpp": raw}, {"fixture.cpp": b"\n" + raw}, pin, "pe:va")
    changed = raw.replace(b"switch_targets", b"switch_selector", 1)
    with pytest.raises(ValueError, match="associated-data interpretations"):
        _require_associated_unchanged({"fixture.cpp": raw}, {"fixture.cpp": changed}, pin, "pe:va")
    with pytest.raises(ValueError):
        _require_associated_unchanged({"fixture.cpp": raw}, {"fixture.cpp": raw + raw}, pin, "pe:va")


def deck(root, task):
    unit = "code/unit.cc"
    return {"schema_version": 2, "source": unit, "entity_id": task.subjects[0].entity_id,
            "files": {unit: {"source_sha256": hashlib.sha256((root / unit).read_bytes()).hexdigest(),
                             "before": "/* baseline */", "control": "/* damaged */",
                             "entity_id": task.subjects[0].entity_id}},
            "candidates": {"same": {unit: "/* same */"}, "bad": {unit: "/* bad */"}}}


class VariantCompiler:
    def __init__(self, raw):
        self.raw = raw

    def __call__(self, *args):
        return self.raw

    def overlay(self, source, replacements, arguments, flags):
        text = replacements["code/unit.cc"]
        if b"/* bad */" in text or b"/* damaged */" in text:
            raw = bytearray(self.raw)
            raw[60] ^= 1
            return bytes(raw)
        return self.raw


def test_variant_measurements_use_frozen_associated_proof_without_dirtying_source(tmp_path):
    root, task, raw = project(tmp_path)
    original = (root / "code/unit.cc").read_bytes()
    report = measure_variants(root, task.task_id, root / "image.exe", deck(root, task),
                              compiler=VariantCompiler(raw))
    assert report["status"] == "scored", report
    assert all(row["result"] == Result.PASS for row in report["controls"])
    assert all(row["target"]["result"] == Result.PASS for row in report["baseline"])
    for row in report["candidates"]:
        assert (row["target"]["result"] == Result.PASS) == (row["name"] == "same")
    assert (root / "code/unit.cc").read_bytes() == original
    assert not report["ready_for_collection"] and not report["persisted"]


def test_source_integrity_checks_new_grammar_without_claiming_physical_proof():
    _, _, _, associated = context()
    text = comments(associated)
    assert check_annotation_text(Path("unit.cc"), text)["findings"] == []
    damaged = text.replace("v1 switch_targets", "v2 switch_targets")
    findings = check_annotation_text(Path("unit.cc"), damaged)["findings"]
    assert any(row["rule"] == "associated_data_bindings" for row in findings)
