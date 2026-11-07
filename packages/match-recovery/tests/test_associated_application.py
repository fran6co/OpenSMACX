"""Associated-data evidence must survive ordinary checks and source trials."""

from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path

import pytest

from match_recovery.application_binary import check_compiled, verify_records, verify_task
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Result, Span
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


def test_native_multi_span_owner_with_association_remains_unknown():
    binding, objects, image, associated = context()
    primary = binding.spans[0]
    compound = replace(
        binding,
        spans=(primary, Span(primary.end + 0x100, primary.end + 0x108)),
    )
    rows = check_objects([compound], objects, image,
                         associated_bindings=associated)
    row, = rows
    assert row["result"] == Result.UNKNOWN


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
    stale_body = replace(records[0], region=records[0].region + "\n// stale caller body\n")
    with pytest.raises(ValueError, match="current configured source annotation"):
        verify_records(root, [stale_body], root / "image.exe", flag_sets=(("first",),), compiler=compile)


def test_native_record_verifier_returns_same_fresh_report_for_unchanged_source(tmp_path):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    verify = lambda: verify_records(root, records, root / "image.exe",
                                    flag_sets=(("first",),), compiler=lambda *_: raw,
                                    command_for=lambda _path: ())
    first, second = verify(), verify()
    assert first == second


@pytest.mark.parametrize("damage", ["same_mtime_edit", "add_source", "remove_source", "retarget_symlink", "config"])
def test_record_verifier_refuses_captured_source_changes_during_compilation(tmp_path, damage):
    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")
    extra = root / "code/extra.h"
    if damage in ("same_mtime_edit", "remove_source"):
        extra.write_text("// retained configured host\n")
    elif damage == "retarget_symlink":
        first_target, second_target = root / "code/first-target.h", root / "code/second-target.h"
        first_target.write_text("// same target bytes\n")
        second_target.write_text("// same target bytes\n")
        extra.symlink_to(first_target.name)
    original_stat = extra.stat() if extra.exists() else None

    def compile(*_args):
        if damage == "same_mtime_edit":
            extra.write_text("// changed, but mtime restored\n")
            os.utime(extra, ns=(original_stat.st_atime_ns, original_stat.st_mtime_ns))
        elif damage == "add_source":
            extra.write_text("// new configured host\n")
        elif damage == "remove_source":
            extra.unlink()
        elif damage == "retarget_symlink":
            extra.unlink()
            extra.symlink_to(second_target.name)
        elif damage == "config":
            project_config = json.loads((root / "recovery/project.json").read_text())
            project_config["source"]["extensions"] = [".cc", ".h", ".hpp"]
            (root / "recovery/project.json").write_text(json.dumps(project_config))
        return raw

    with pytest.raises(ValueError, match="source|build inputs|inventory"):
        verify_records(root, records, root / "image.exe", flag_sets=(("first",),),
                       compiler=compile, command_for=lambda _path: ())


def test_record_verifier_checks_namespace_collisions_in_unselected_headers(tmp_path):
    root, _task, _raw = project(tmp_path)
    marker = "// RECOVERY_IMPORT: v1 __imp__shared m.dll name:Shared\n"
    (root / "code/unit.cc").write_text((root / "code/unit.cc").read_text() + marker)
    (root / "code/other.h").write_text(marker)
    records = read_file(root / "code/unit.cc")
    with pytest.raises(ValueError, match="duplicate source import"):
        verify_records(root, records, root / "image.exe", flag_sets=(("first",),),
                       compiler=lambda *_: pytest.fail("namespace collision reached compilation"),
                       command_for=lambda _path: ())


def test_captured_source_readers_are_stable_while_standalone_reader_stays_fresh(tmp_path):
    from match_recovery.import_source import read_imports
    from match_recovery.source_inventory import SourceInputs, source_snapshot

    root, _task, _raw = project(tmp_path)
    snapshot = source_snapshot(root)
    inputs = SourceInputs.from_snapshot(root, snapshot)
    before = inputs.text("code/unit.cc")
    snapshot_view = inputs.snapshot
    snapshot_view["files"]["code/unit.cc"]["content_base64"] = ""
    assert inputs.text("code/unit.cc") == before
    with pytest.raises(TypeError):
        inputs.texts["code/unit.cc"] = "mutated"
    source = root / "code/unit.cc"
    source.write_text(source.read_text() + "\n// RECOVERY_IMPORT: v1 __imp__fresh m.dll name:Fresh\n")

    assert read_imports(root) and read_imports(root)[0].name == "Fresh"
    assert read_imports(root, inputs=inputs) == ()


def test_record_verification_keeps_strict_utf8_source_fact_decoding(tmp_path):
    from match_recovery.source_inventory import SourceInputs, source_snapshot

    root, _task, _raw = project(tmp_path)
    (root / "code/invalid.h").write_bytes(b"// invalid byte: \xff\n")
    snapshot = source_snapshot(root)
    with pytest.raises(UnicodeDecodeError):
        SourceInputs.from_snapshot(root, snapshot)


def test_source_snapshot_guards_same_content_symlink_target_identity(tmp_path):
    from match_recovery.source_inventory import assert_snapshot_current, source_snapshot

    root, _task, _raw = project(tmp_path)
    first, second = root / "code/first.h", root / "code/second.h"
    alias = root / "code/alias.h"
    first.write_text("// identical bytes\n")
    second.write_text("// identical bytes\n")
    alias.symlink_to(first.name)
    snapshot = source_snapshot(root)
    assert snapshot["files"]["code/alias.h"]["resolved_path"] == "code/first.h"
    alias.unlink()
    alias.symlink_to(second.name)
    with pytest.raises(ValueError, match="source target changed"):
        assert_snapshot_current(root, snapshot)


def test_source_snapshot_refuses_symlink_retarget_during_capture(tmp_path, monkeypatch):
    import match_recovery.source_inventory as source_inventory

    root, _task, _raw = project(tmp_path)
    first, second = root / "code/first.h", root / "code/second.h"
    alias = root / "code/alias.h"
    first.write_text("// identical bytes\n")
    second.write_text("// identical bytes\n")
    alias.symlink_to(first.name)
    original_memoized = source_inventory.memoized
    changed = False

    def retarget_after_parse(cache, key, load):
        nonlocal changed
        result = original_memoized(cache, key, load)
        if not changed:
            changed = True
            alias.unlink()
            alias.symlink_to(second.name)
        return result

    monkeypatch.setattr(source_inventory, "memoized", retarget_after_parse)
    with pytest.raises(ValueError, match="source target changed during import"):
        source_inventory.source_snapshot(root)


@pytest.mark.parametrize("line_ending", [b"\r\n", b"\r"])
def test_captured_fact_reader_text_matches_universal_newlines_and_unselected_collision(tmp_path, line_ending):
    from match_recovery.associated_source import read_associated
    from match_recovery.import_source import read_imports
    from match_recovery.source import read_text
    from match_recovery.source_inventory import (SourceInputs, source_file_texts,
                                                 source_snapshot)

    root, _task, _raw = project(tmp_path)
    source = root / "code/unit.cc"
    source.write_bytes(source.read_bytes().replace(b"\r\n", b"\n").replace(b"\r", b"\n")
                       .replace(b"\n", line_ending))
    marker = b"// RECOVERY_IMPORT: v1 __imp__same m.dll name:Same"
    other_ending = b"\r" if line_ending == b"\r\n" else b"\r\n"
    source.write_bytes(source.read_bytes() + marker + line_ending)
    (root / "code/other.h").write_bytes(marker + other_ending)

    snapshot = source_snapshot(root)
    inputs = SourceInputs.from_snapshot(root, snapshot)
    _, fresh_rows = source_file_texts(root)
    _, captured_rows = source_file_texts(root, inputs)
    fresh_texts = dict(fresh_rows)
    captured_texts = dict(captured_rows)
    assert captured_texts == fresh_texts
    assert read_text(inputs.text("code/unit.cc"), source) == read_file(source)
    assert read_associated(root, snapshot["image_sha256"], inputs=inputs) == read_associated(
        root, snapshot["image_sha256"])
    with pytest.raises(ValueError, match="duplicate source import"):
        read_imports(root)
    with pytest.raises(ValueError, match="duplicate source import"):
        read_imports(root, inputs=inputs)


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
    from match_recovery.source import asm

    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")

    def strict_pass_row(*_args, **_kwargs):
        return {"result": overall,
                "functions": [{"address": f"{records[0].address:08X}",
                               "result": Result.PASS}]}

    monkeypatch.setattr(asm, "verify_records", strict_pass_row)
    compare = compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                           (("first",),), compiler=lambda *_: raw,
                           root=root, strict_command_for=lambda _path: ())
    assert compare.results[0][1].verdict is (
        Tier.MISMATCH if overall == Result.FAIL else Tier.REFUSED)


def test_associated_refusal_keeps_selected_dependency_reason(tmp_path, monkeypatch):
    from match_recovery.source import asm

    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")

    def unknown_dependency(*_args, **_kwargs):
        return {"result": Result.UNKNOWN,
                "functions": [{"address": f"{records[0].address:08X}",
                               "result": Result.PASS}],
                "selected_evidence": {"checks": [{"result": Result.UNKNOWN,
                                                   "reason": "transitive object _g has no bound extent"}]}}

    monkeypatch.setattr(asm, "verify_records", unknown_dependency)
    compare = compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                           (("first",),), compiler=lambda *_: raw,
                           root=root, strict_command_for=lambda _path: ())
    observed = compare.results[0][1]
    assert observed.verdict is Tier.REFUSED
    assert observed.diagnostic == "transitive object _g has no bound extent"


def test_associated_refusal_reports_the_failing_variant_reason(tmp_path, monkeypatch):
    from match_recovery.source import asm

    root, _task, raw = project(tmp_path)
    records = read_file(root / "code/unit.cc")

    def unknown_variant(*_args, **_kwargs):
        return {"result": Result.UNKNOWN,
                "functions": [{"address": f"{records[0].address:08X}",
                               "result": Result.UNKNOWN,
                               "variants": [{"result": Result.PASS, "reason": "unrelated"},
                                            {"result": Result.UNKNOWN,
                                             "reason": "FPO metadata extent differs"}]}]}

    monkeypatch.setattr(asm, "verify_records", unknown_variant)
    compare = compare_unit(records, root / "image.exe", root / "code/unit.cc", (),
                           (("first",),), compiler=lambda *_: raw,
                           root=root, strict_command_for=lambda _path: ())
    observed = compare.results[0][1]
    assert observed.verdict is Tier.REFUSED
    assert observed.diagnostic == "FPO metadata extent differs"


def test_associated_owner_verdict_does_not_depend_on_selected_file_mates(tmp_path, monkeypatch):
    from match_recovery.source import asm

    root, _task, _raw = project(tmp_path)
    _binding, _objects, _original, associated = context()
    source = root / "code/unit.cc"
    shift = 0x80
    mate = tuple(replace(row, unit="code/unit.cc",
                         owner=replace(row.owner, address=row.owner.address + shift),
                         span=Span(row.span.start + shift, row.span.end + shift),
                         evidence=replace(row.evidence, address=row.evidence.address + shift))
                 for row in associated)
    owner = associated[0].owner.address + shift
    source.write_text(source.read_text() + "\n" + comments(mate)
                      + f"// ORIGINAL: 0x{owner:X} _mate 0x{owner:X}-0x{owner + 0x16:X}\n")
    claimed, unclaimed = read_file(source)
    calls = []

    def selection_wide(_root, selection, *_args, **_kwargs):
        # The unclaimed mate's closure refuses; the claimed owner's own
        # closure is complete. Like the native verifier, checks carry no root.
        selection = tuple(selection)
        calls.append(tuple(record.address for record in selection))
        blocked = any(record.address == unclaimed.address for record in selection)
        return {"result": Result.UNKNOWN if blocked else Result.PASS,
                "functions": [{"address": f"{record.address:08X}", "result": Result.PASS}
                              for record in selection],
                "selected_evidence": {"checks": [{"result": Result.UNKNOWN,
                                                   "reason": "mate object has no bound extent"}]
                                      if blocked else []}}

    monkeypatch.setattr(asm, "verify_records", selection_wide)
    measure = lambda rows: dict((record.address, observed) for record, observed in compare_unit(
        rows, root / "image.exe", source, (), (("first",),), compiler=lambda *_: _raw,
        root=root, strict_command_for=lambda _path: ()).results)
    alone = measure((claimed,))
    together = measure((claimed, unclaimed))
    assert alone[claimed.address].verdict is Tier.BYTE_EXACT
    assert together[claimed.address] == alone[claimed.address]
    # The blocked mate keeps its own refusal and reason.
    assert together[unclaimed.address].verdict is Tier.REFUSED
    assert together[unclaimed.address].diagnostic == "mate object has no bound extent"
    assert calls == [(claimed.address,), (claimed.address, unclaimed.address),
                     (claimed.address,), (unclaimed.address,)]


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
                            root / "image.exe", (), (("first",),), frozenset(), None))
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


@pytest.mark.parametrize("groups", [2, 4])
@pytest.mark.parametrize("damage", [None, "code", "shift", "missing-group"])
def test_native_application_preserves_independent_direct_table_results(groups, damage):
    from test_associated_binary import direct_multi_fixture
    options = ({"candidate_change": (groups * 12 + 1, 9)} if damage == "code" else
               {"candidate_shift": 4} if damage == "shift" else
               {"omit": (1,)} if damage == "missing-group" else {})
    obj, subject, span, image, associated, _ = direct_multi_fixture(groups=groups, **options)
    binding = FunctionBinding(associated[0].owner, (span,), subject.name, associated[0].unit)
    rows, *_ = check_compiled([binding], (), (), {binding.unit: obj}, image, {},
                              associated_bindings=associated)
    row, = rows
    assert row["result"] == (Result.PASS if damage is None else
                             Result.UNKNOWN if damage == "missing-group" else Result.FAIL)
    if damage != "missing-group":
        assert row["span"] == row["source_body_span"]
        assert row["comparison_span"]["end"] == associated[-1].span.end
        assert row["linked_selection"] == row["runtime"] == Result.UNKNOWN


@pytest.mark.parametrize("mode", ["direct-single", "direct-multi", "mapped-single", "mapped-multi"])
def test_native_application_refuses_unsupported_candidate_anchor(mode):
    from test_associated_binary import anchor_fixture, changed_anchor
    obj, subject, span, image, associated, _ = anchor_fixture(mode)
    obj = changed_anchor(obj, subject, associated[0], "storage")
    binding = FunctionBinding(associated[0].owner, (span,), subject.name, associated[0].unit)
    rows, *_ = check_compiled([binding], (), (), {binding.unit: obj}, image, {},
                              associated_bindings=associated)
    row, = rows
    assert row["result"] == Result.UNKNOWN
    assert "candidate associated-data anchor proof is unsupported" in str(row)
