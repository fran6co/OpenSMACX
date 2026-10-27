"""Associated-data evidence must survive ordinary checks and source trials."""

from dataclasses import replace
import hashlib
import json
from pathlib import Path

import pytest

from match_recovery.application_binary import check_compiled, verify_records, verify_task
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Result
from match_recovery.source_integrity import check_annotation_text
from match_recovery.source import read_file
from match_recovery.source.asm import compare_unit
from match_recovery.source.model import Tier
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


def test_native_record_verifier_accepts_only_current_records_and_explicit_trials(tmp_path):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    calls = []

    def compile(source, arguments, flags):
        calls.append((source.name, flags))
        return raw

    report = verify_records(root, records, root / "image.exe", flag_sets=(("first",), ("second",)),
                            compiler=compile, command_for=lambda _path: ())
    assert report["result"] == Result.PASS, report
    assert calls == [("unit.cc", ("first",)), ("unit.cc", ("second",))]
    assert report["task_id"] is None
    assert report["functions"][0]["result"] == Result.PASS
    stale = replace(records[0], symbol="_stale")
    with pytest.raises(ValueError, match="current configured source annotation"):
        verify_records(root, [stale], root / "image.exe", flag_sets=(("first",),), compiler=compile)


@pytest.mark.parametrize("damage", ["guard", "table", "reference"])
def test_native_record_verifier_does_not_promote_associated_damage(tmp_path, damage):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    if damage == "guard":
        changed = bytearray(raw)
        changed[60] ^= 1
        candidate = bytes(changed)
    elif damage == "table":
        candidate = fixture(selector_byte=1)[0].raw_data
    else:
        candidate = fixture(cell_addend=1)[0].raw_data
    report = verify_records(root, records, root / "image.exe", flag_sets=(("first",),),
                            compiler=lambda *_: candidate, command_for=lambda _path: ())
    assert report["result"] != Result.PASS, report
    assert report["functions"][0]["result"] != Result.PASS


def test_compare_unit_routes_associated_owners_strictly_and_requires_root_context(tmp_path):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    calls = []

    def compile(source, arguments, flags):
        calls.append((source, arguments, flags))
        return raw

    compare = compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                           ("first",), compiler=compile,
                           root=root, strict_command_for=lambda _path: ())
    assert compare.results[0][1].verdict is Tier.BYTE_EXACT
    assert compare.compile_attempts == 1 and compare.reused_attempts == 0
    assert len(calls) == 1 and calls[0][2] == ("first",)

    _, _, _, associated = context()
    with pytest.raises(ValueError, match="root context is required"):
        compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                     (("first",),), compiler=lambda *_: raw,
                     associated_bindings=associated)
    with pytest.raises(ValueError, match="root context is required"):
        compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                     (("first",),), compiler=lambda *_: raw)


def test_mixed_unit_reuses_associated_compile_for_ordinary_owner(tmp_path):
    root, _task, raw = project(tmp_path)
    binding, _objects, original, _associated = context()
    image_bytes = bytearray(original.data)
    body_size = binding.spans[0].end - binding.spans[0].start
    second_rva = 0x1080
    second_raw = 512 + second_rva - 0x1000
    image_bytes[second_raw:second_raw + body_size] = image_bytes[512:512 + body_size]
    image_path = root / "image.exe"
    image_path.write_bytes(image_bytes)
    pin = hashlib.sha256(image_bytes).hexdigest()
    (root / "code/PIN").write_text("sha256 " + pin + "\n")
    source = root / "code/unit.cc"
    source.write_text(source.read_text()
                      + "\n// ORIGINAL: 0x401080 _missing 0x401080-0x401096\n")
    associated, ordinary = read_file(source)
    calls = []

    def compile(source, arguments, flags):
        calls.append((source, arguments, flags))
        return raw

    compare = compare_unit((associated, ordinary), image_path,
                           source, (), ("first",), compiler=compile,
                           root=root, strict_command_for=lambda _path: ())
    assert compare.results[0][1].verdict is Tier.BYTE_EXACT
    assert isinstance(compare.results[1][1], str) and "_missing" in compare.results[1][1]
    assert compare.compile_attempts == 1 and compare.reused_attempts == 1
    assert len(calls) == 1 and calls[0][2] == ("first",)


@pytest.mark.parametrize("overall", [Result.FAIL, Result.UNKNOWN])
def test_associated_compare_requires_whole_native_selection_to_pass(tmp_path, monkeypatch, overall):
    from match_recovery import application_binary

    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")

    def strict_pass_row(*_args, **_kwargs):
        return {"result": overall,
                "functions": [{"address": f"{records[0].address:08X}",
                               "result": Result.PASS}]}

    monkeypatch.setattr(application_binary, "verify_records", strict_pass_row)
    compare = compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                           (("first",),), compiler=lambda *_: raw,
                           root=root, strict_command_for=lambda _path: ())
    assert compare.results[0][1].verdict is (
        Tier.MISMATCH if overall == Result.FAIL else Tier.REFUSED)


def test_ordinary_rooted_comparison_does_not_scan_other_source_files(tmp_path):
    root, _task, raw = project(tmp_path)
    source = root / "code/unit.cc"
    source.write_text(source.read_text().replace(comments(
        tuple(replace(row, unit="code/unit.cc") for row in context()[3])), ""))
    (root / "code/unrelated.cc").write_text(
        "// RECOVERY_ASSOCIATED: malformed annotation in an unrelated unit\n")
    records = read_file(source)
    compare = compare_unit(records, root / "image.exe", source, (),
                           (("first",),), compiler=lambda *_: raw, root=root)
    assert compare.compile_attempts == 1


def test_recording_unit_reuses_its_invocation_compiler_for_strict_associated_owner(tmp_path, monkeypatch):
    from match_recovery import recording

    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    calls = []

    def compile(source, arguments, flags):
        calls.append((source.name, arguments, flags))
        return raw

    monkeypatch.setattr(recording, "Compiler", lambda *_args, **_kwargs: compile)
    unit = recording._unit((root, "synthetic", root / "code/unit.cc", records,
                            root / "image.exe", (), (("first",),), frozenset()))
    assert unit.results[0][1].verdict is Tier.BYTE_EXACT
    assert calls == [("unit.cc", (), ("first",))]


def test_native_record_verifier_observes_changed_association_mapping(tmp_path):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    path = root / "code/unit.cc"
    original = path.read_text()
    assert "0x401018 0x40101C" in original
    path.write_text(original.replace("0x401018 0x40101C", "0x401020 0x401024", 1))
    report = verify_records(root, records, root / "image.exe", flag_sets=(("first",),),
                            compiler=lambda *_: raw, command_for=lambda _path: ())
    assert report["result"] != Result.PASS, report
    assert report["functions"][0]["result"] != Result.PASS


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
