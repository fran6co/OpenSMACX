"""Source-bound literals remain measured obligations in every workflow trial."""

import hashlib
import json

import pytest

from match_recovery.application_binary import verify_task
from match_recovery.data_check import check_data
from match_recovery.literal_source import read_literals
from match_recovery.pe_coff import Coff
from match_recovery.tasks import Task, TaskStore
from match_recovery.variants import measure_variants
from test_literal_data import NAME, fixture, literal_object


ANNOTATION = f"// RECOVERY_LITERAL: v1 0x402000 0x402003 {NAME} 0x401000"


@pytest.mark.parametrize("probe_kind,expected", [("unchanged", "pass"), ("changed", "unknown"),
                                               ("missing", "unknown")])
def test_retained_probe_rechecks_its_own_literals_instead_of_borrowing_product_proof(tmp_path, monkeypatch,
                                                                                  probe_kind, expected):
    root, task = project(tmp_path)
    (root / "code/a.cc").write_text(ANNOTATION + "\n"
        '// ORIGINAL: 0x401000 _f 0x401000-0x401010\n'
        '// body code/inline.hpp\n#include "inline.hpp"\n')
    (root / "code/inline.hpp").write_text('inline void f() { /* private retention target */ }\n')
    product = literal_object().replace(b"_f", b"_h")  # Literal exists; f is not ordinarily emitted.
    probe = literal_object(b"wb\0") if probe_kind == "changed" else literal_object(defined=probe_kind != "missing")
    calls = []

    def retain(*args):
        calls.append(args)
        return Coff(probe), {"result": "pass", "compile_attempts": 1}

    monkeypatch.setattr("match_recovery.function_witness.retained_definition", retain)
    report = verify_task(root, task.task_id, root / "image.exe", compiler=lambda *_: product,
                         retained_diagnostics=True)
    assert len(calls) == 2
    assert all(row["content"] == "pass" for row in report["literal_definitions"])
    assert [row["measurement"]["references"] for row in report["retained_definitions"]] == [expected, expected]
    assert report["functions"][0]["result"] == report["result"] == "unknown"
    assert not report["ready_for_collection"]


def project(tmp_path, *, second_owner=False, mapped=True):
    literal, function, _, image = fixture()
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    (tmp_path / "image.exe").write_bytes(image.data)
    (tmp_path / "code/PIN").write_text("sha256 " + literal.origin.image_sha256 + "\n")
    marker = "// ORIGINAL: 0x401000 _f 0x401000-0x401010\n" if mapped else ""
    (tmp_path / "code/a.cc").write_text(ANNOTATION + "\n" + marker + "void f() { /* baseline */ }\n")
    if second_owner:
        (tmp_path / "code/b.cc").write_text(ANNOTATION.replace("0x402000 0x402003", "0x402003 0x402006") + "\n")
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va",
        "roots": ["code"], "include_paths": ["code"]}, "measurement": {
        "backend": "pe-coff-i386", "compiler_profile": "synthetic", "flag_sets": [["first"], ["second"]],
        "layout_flags": [], "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    task = Task("literal-owner", "synthetic", "coordinator", "Recover this literal user", "a" * 40,
                (function.origin,), ("code/a.cc",), ("synthetic original ownership",))
    TaskStore(tmp_path).create(task)
    return tmp_path, task


def test_evidence_anchor_schedules_literal_only_owner_without_expanding_scope(tmp_path):
    root, task = project(tmp_path, second_owner=True)
    calls = []

    def compiler(path, command, flags):
        calls.append((path.name, flags))
        return literal_object()

    report = verify_task(root, task.task_id, root / "image.exe", compiler=compiler)
    assert report["result"] == "pass", report
    assert calls == [(unit, (flag,)) for flag in ("first", "second") for unit in ("a.cc", "b.cc")]
    assert report["translation_units"] == ["code/a.cc", "code/b.cc"]
    assert report["compile_attempts"] == 4
    assert len(report["functions"]) == 1 and report["functions"][0]["unit"] == "code/a.cc"
    assert len(report["literal_definitions"]) == 4
    expected = read_literals(root, task.subjects[0].image_sha256)
    assert report["literal_owners"] == [{"interpretation_id": item.interpretation_id, "unit": item.unit}
                                         for item in expected]
    assert TaskStore(root).scope(task.task_id)[1] == frozenset({"code/a.cc"})
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage,expected", [("contents", "fail"), ("compile", "unknown"), ("definition", "unknown")])
def test_literal_only_owner_cannot_borrow_success_from_another_trial(tmp_path, damage, expected):
    root, task = project(tmp_path, second_owner=True)

    def compiler(path, command, flags):
        if path.name == "b.cc" and flags == ("second",):
            if damage == "compile":
                raise ValueError("literal owner did not compile")
            return literal_object(b"wb\0") if damage == "contents" else literal_object(defined=False)
        return literal_object()

    report = verify_task(root, task.task_id, root / "image.exe", compiler=compiler)
    assert report["functions"][0]["result"] == "pass"  # Independent code is unchanged.
    owners = [row for row in report["literal_definitions"] if row["unit"] == "code/b.cc"]
    assert [(row["flags"], row["content"]) for row in owners] == [(["first"], "pass"), (["second"], expected)]
    assert report["result"] == expected


@pytest.mark.parametrize("selection", [None, [NAME]])
def test_data_only_declared_literals_are_checked_without_object_annotations(tmp_path, selection):
    root, _ = project(tmp_path, mapped=False)
    calls = []

    def compiler(path, command, flags):
        calls.append((path.name, flags))
        return literal_object()

    report = check_data(root, root / "image.exe", symbols=selection, compiler=compiler)
    assert report["result"] == "pass" and report["counts"] == {"pass": 2}
    assert report["objects"] == [] and report["selected_objects"] == 0
    assert report["selected_literals"] == 1 and len(report["literals"]) == 2
    assert calls == [("a.cc", (flag,)) for flag in ("first", "second")]
    assert not report["persisted"] and not report["ready_for_collection"]


def test_data_selection_refuses_bare_private_name_ambiguity_but_checks_all_owners(tmp_path):
    root, _ = project(tmp_path, second_owner=True, mapped=False)
    with pytest.raises(ValueError, match="ambiguous across literal owners"):
        check_data(root, root / "image.exe", symbols=[NAME], compiler=object())
    report = check_data(root, root / "image.exe", compiler=lambda *_: literal_object())
    assert report["result"] == "pass" and report["selected_literals"] == 2
    assert report["counts"] == {"pass": 4} and report["compile_attempts"] == 4
    assert {row["unit"] for row in report["literals"]} == {"code/a.cc", "code/b.cc"}


@pytest.mark.parametrize("damage,expected", [("contents", "fail"), ("compile", "unknown"), ("definition", "unknown")])
def test_data_literal_trials_keep_failure_and_missing_evidence(tmp_path, damage, expected):
    root, _ = project(tmp_path, mapped=False)

    def compiler(path, command, flags):
        if flags == ("second",):
            if damage == "compile":
                raise ValueError("literal owner did not compile")
            return literal_object(b"wb\0") if damage == "contents" else literal_object(defined=False)
        return literal_object()

    report = check_data(root, root / "image.exe", compiler=compiler)
    assert report["result"] == expected and report["counts"] == {"pass": 1, expected: 1}
    assert [row["content"] for row in report["literals"]] == ["pass", expected]


def deck(root, task):
    return {"schema_version": 2, "source": "code/a.cc", "entity_id": task.subjects[0].entity_id,
        "files": {"code/a.cc": {"source_sha256": hashlib.sha256((root / "code/a.cc").read_bytes()).hexdigest(),
            "before": "/* baseline */", "control": "/* damaged function */", "entity_id": task.subjects[0].entity_id}},
        "candidates": {"same": {"code/a.cc": "/* same */"}, "changed-literal": {"code/a.cc": "/* changed literal */"}}}


class VariantCompiler:
    def __call__(self, source, command, flags):
        return literal_object()

    def overlay(self, source, replacements, command, flags):
        text = replacements["code/a.cc"]
        raw = bytearray(literal_object(b"wb\0" if b"changed literal" in text else b"rb\0"))
        if b"damaged function" in text:
            raw[100] = 0xA3  # Physical code byte; preserve valid section/symbol records.
        return bytes(raw)


@pytest.mark.parametrize("replacement", ["", ANNOTATION.replace("0x402000 0x402003", "0x402003 0x402006"),
                                        ANNOTATION.replace(NAME, "other"), ANNOTATION.replace("0x401000", "0x401001")])
def test_variant_deck_cannot_change_or_remove_literal_interpretations(tmp_path, replacement):
    root, task = project(tmp_path)
    value = deck(root, task)
    value["files"]["code/a.cc"].update(before=["/* baseline */", ANNOTATION],
                                         control=["/* damaged function */", ANNOTATION])
    value["candidates"] = {"rebind": {"code/a.cc": ["/* same */", replacement]}}
    with pytest.raises(ValueError, match="cannot change recovery interpretations"):
        measure_variants(root, task.task_id, root / "image.exe", value, compiler=object())


def test_variants_preserve_literal_failures_in_measurements_and_rankings(tmp_path):
    root, task = project(tmp_path)
    original = (root / "code/a.cc").read_bytes()
    report = measure_variants(root, task.task_id, root / "image.exe", deck(root, task), compiler=VariantCompiler())
    assert report["status"] == "scored" and all(row["result"] == "pass" for row in report["controls"])
    assert all(row["literal_definitions"][0]["content"] == "pass" for row in report["baseline"])
    for trial in report["candidates"]:
        changed = trial["name"] == "changed-literal"
        assert trial["literal_definitions"][0]["content"] == ("fail" if changed else "pass")
        assert trial["target"]["result"] == ("unknown" if changed else "pass")
    for ranking in report["rankings"]:
        assert ranking["names"] == ["same", "changed-literal"]
        bad = next(row for row in ranking["details"] if row["name"] == "changed-literal")
        assert bad["scored_results"]["fail"] == 1
        assert {"kind": "literal", "unit": "code/a.cc", "symbol": NAME,
                "address": "0x00402000", "after": "fail"} in bad["lost_passing_measurements"]
    assert (root / "code/a.cc").read_bytes() == original
    assert not report["ready_for_collection"] and not report["persisted"]
