"""Additional shared bodies must each earn their own strict compiler witness."""

from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery.application_binary import check_compiled, verify_task
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.ownership_source import project_view
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.shared_binary import check_shared
from match_recovery.shared_source import (SharedFunctionBinding, parse_shared, read_shared,
                                          referenced_shared_units, validate_shared)
from match_recovery.tasks import Task, TaskStore
from test_binary import coff_fixture, pe_fixture


def code_object(definitions):
    """One independently owned synthetic COMDAT per short external name."""
    payload, headers, symbols = bytearray(), [], bytearray()
    start = 20 + 40 * len(definitions)
    for number, (name, code) in enumerate(definitions.items(), 1):
        raw = code + b"\xcc" * (-len(code) % 16)
        headers.append(b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(raw), start + len(payload), 0, 0, 0, 0, 0x60501020))
        payload.extend(raw)
        symbols.extend(name.encode().ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0x20, 2, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(headers), 0, start + len(payload), len(definitions), 0, 0)
            + b"".join(headers) + payload + symbols + struct.pack("<I", 4))


def fixture(code=b"\xc3"):
    raw = pe_fixture(code)
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    bindings = [SharedFunctionBinding(origin, Span(origin.address, origin.address + len(code)), symbol,
                Origin(origin.image_sha256, "pe:va", 0x401020), "code/methods.cc", i + 1)
                for i, symbol in enumerate(("_a", "_b"))]
    return bindings, {"code/methods.cc": Coff(code_object({"_a": code, "_b": code}))}, PeImage(raw, origin.image_sha256)


def test_each_method_sharing_one_original_requires_its_own_proof():
    bindings, objects, image = fixture()
    targets, rows = check_shared(bindings, objects, image, {})
    assert targets == {"_a": 0x401000, "_b": 0x401000}
    assert len({row["interpretation_id"] for row in rows}) == 2
    assert all(row["result"] == row["bytes"] == row["references"] == "pass" for row in rows)
    ordinary = [FunctionBinding(item.origin, (item.span,), item.symbol, item.unit) for item in bindings]
    with pytest.raises(ValueError, match="ambiguous original"):
        check_objects(ordinary, objects, image)


@pytest.mark.parametrize("code,expected", [(b"\x90", "fail"), (b"\xc3\xc3", "fail"), (None, "unknown")])
def test_one_bad_or_missing_sibling_cannot_borrow_a_pass(code, expected):
    bindings, objects, image = fixture()
    definitions = {"_a": b"\xc3"}
    if code is not None:
        definitions["_b"] = code
    objects["code/methods.cc"] = Coff(code_object(definitions))
    targets, rows = check_shared(bindings, objects, image, {})
    assert targets == {"_a": 0x401000}
    assert [row["result"] for row in rows] == ["pass", expected]


@pytest.mark.parametrize("error", ["unfinished shared source placeholder", "shared source binding has no body region"])
def test_source_unfinished_state_cannot_be_overridden_by_an_exact_ret(error):
    bindings, objects, image = fixture()
    bindings[1] = replace(bindings[1], body_error=error)
    targets, rows = check_shared(bindings, objects, image, {})
    assert "_b" not in targets and rows[1]["result"] == "unknown" and rows[1]["reason"] == error


def test_shared_relocations_use_declared_targets_not_original_operands():
    bindings, _, image = fixture(b"\xe8\x1b\0\0\0\xc3")
    binding = replace(bindings[0], symbol="_f")
    objects = {binding.unit: Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))}
    for external, expected in (({}, "unknown"), ({"_g": 0x401024}, "fail"), ({"_g": 0x401020}, "pass")):
        targets, rows = check_shared([binding], objects, image, external)
        assert rows[0]["result"] == expected
        assert bool(targets) == (expected == "pass")


def test_shared_reference_scheduler_uses_function_owners_without_object_members():
    bindings, _, _ = fixture()
    binding = replace(bindings[0], symbol="_g")
    caller = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    assert referenced_shared_units([binding], {"code/caller.cc": caller}) == frozenset({binding.unit})
    with pytest.raises(ValueError, match="duplicate shared function symbol"):
        referenced_shared_units([binding, binding], {"code/caller.cc": caller})


@pytest.mark.parametrize("storage", [3, 105])
def test_static_or_weak_symbols_cannot_create_a_shared_external_target(storage):
    bindings, _, image = fixture()
    binding = replace(bindings[0], symbol="_f")
    targets, rows = check_shared([binding], {binding.unit: Coff(coff_fixture(storage=storage))}, image, {})
    assert not targets and rows[0]["result"] == "unknown"


def test_conflicting_or_duplicate_symbol_bindings_are_refused():
    bindings, objects, image = fixture()
    for selected, targets in ((bindings, {"_a": 0x401000}), ([bindings[0], bindings[0]], {})):
        with pytest.raises(ValueError, match="conflicting"):
            check_shared(selected, objects, image, targets)
    with pytest.raises(ValueError, match="also has"):
        check_compiled([], (), (), objects, image, {"_a": 0x401000}, shared_bindings=bindings)


def test_source_body_state_is_derived_without_merging_shared_markers():
    source = ("// RECOVERY_SHARED: v1 0x10 0x11 _a 0x20\nvoid a() {}\n"
              "// RECOVERY_SHARED: v1 0x10 0x11 _b 0x20\nvoid b() {\n// BODY GOES HERE.\n}\n")
    a, b = parse_shared(source, "code/methods.cc", "a" * 64, address_space="custom:offset")
    assert a.body_error is None and "placeholder" in b.body_error
    assert "body_error" not in a.interpretation
    assert a.interpretation_id != b.interpretation_id
    missing, = parse_shared(source.split("void a()")[0], "code/methods.cc", "a" * 64, address_space="custom:offset")
    assert "no body region" in missing.body_error


def test_shared_region_uses_real_comment_context_not_prose_braces():
    source = ('/*\n// RECOVERY_SHARED: v1 0x10 0x11 _a 0x20\n'
              'An unmatched { is only prose.\n*/\nvoid a() {}\n')
    binding, = parse_shared(source, "code/methods.cc", "a" * 64, address_space="custom:offset")
    assert binding.body_error is None
    unterminated = source.replace("*/\n", "")
    binding, = parse_shared(unterminated, "code/methods.cc", "a" * 64, address_space="custom:offset")
    assert binding.body_error == "shared source binding has no body region"


def test_shared_region_ignores_a_literal_comment_opener_before_its_marker():
    source = ('const char *label = "/*";\n'
              '// RECOVERY_SHARED: v1 0x10 0x11 _a 0x20\nvoid a() {}\n')
    binding, = parse_shared(source, "code/methods.cc", "a" * 64, address_space="custom:offset")
    assert binding.body_error is None


@pytest.mark.parametrize("line", ["v1 0x10 0x10 _a 0x20", "v1 0x10 ? _a 0x20", "v1 0x10 0x11 _a", "v2 0x10 0x11 _a 0x20"])
def test_malformed_or_unbounded_shared_annotations_are_refused(line):
    with pytest.raises(ValueError):
        parse_shared("// RECOVERY_SHARED: " + line, "code/methods.cc", "a" * 64, address_space="custom:offset")


@pytest.fixture
def project(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    raw = pe_fixture(b"\xe8\x1b\0\0\0\xc3" + bytes(26) + b"\xc3")
    pin = hashlib.sha256(raw).hexdigest()
    (tmp_path / "image.exe").write_bytes(raw)
    (tmp_path / "code/PIN").write_text("sha256 " + pin)
    (tmp_path / "code/caller.cc").write_text("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\nvoid f() {}\n")
    (tmp_path / "code/catalogue.cc").write_text("// ORIGINAL: 0x00401020 _canonical 0x00401020-0x00401021\nvoid canonical() {}\n")
    (tmp_path / "code/methods.cc").write_text("// RECOVERY_SHARED: v1 0x00401020 0x00401021 _g 0x00401020\nvoid g() {}\n"
                                           "// RECOVERY_SHARED: v1 0x00401020 0x00401021 _h 0x00401020\nvoid h() {}\n")
    configuration = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic", "flag_sets": [["first"], ["second"]],
                        "layout_flags": [], "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(configuration))
    TaskStore(tmp_path).create(Task("owner", "synthetic", "coordinator", "Recover callers", "a" * 40,
        (Origin(pin, "pe:va", 0x401000),), ("code/caller.cc",), ("synthetic",)))
    return tmp_path


def compiler(path, command, flags):
    if path.name == "caller.cc":
        return coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    return code_object({"_g": b"\xc3", "_h": b"\xc3"})


def test_task_follows_shared_owner_and_keeps_all_siblings_and_trials(project):
    report = verify_task(project, "owner", project / "image.exe", compiler=compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == 4
    assert len(report["functions"]) == 1 and len(report["shared_functions"]) == 4
    assert all(row["units"] == ["code/methods.cc"] for row in report["object_dependencies"])
    assert TaskStore(project).scope("owner")[1] == frozenset({"code/caller.cc"})


@pytest.mark.parametrize("damage", ["body", "missing", "compile", "placeholder"])
def test_shared_failure_cannot_hide_behind_passing_caller_or_flag(project, damage):
    if damage == "placeholder":
        path = project / "code/methods.cc"
        path.write_text(path.read_text().replace("void h() {}", "void h() {\n// BODY GOES HERE.\n}"))
    def damaged(path, command, flags):
        if path.name == "methods.cc" and flags == ("second",):
            if damage == "compile":
                raise ValueError("deliberate failure")
            if damage == "body":
                return code_object({"_g": b"\xc3", "_h": b"\x90"})
            if damage == "missing":
                return code_object({"_g": b"\xc3"})
        return compiler(path, command, flags)
    report = verify_task(project, "owner", project / "image.exe", compiler=damaged)
    assert report["functions"][0]["result"] == "pass"
    assert report["result"] == ("fail" if damage == "body" else "unknown")


def test_shared_source_race_invalidates_the_whole_measurement(project):
    def raced(path, command, flags):
        if path.name == "methods.cc":
            path.write_text(path.read_text() + "// changed\n")
        return compiler(path, command, flags)
    with pytest.raises(ValueError, match="changed during measurement"):
        verify_task(project, "owner", project / "image.exe", compiler=raced)


@pytest.mark.parametrize("damage", ["extent", "origin", "anchor", "duplicate", "canonical", "reserved"])
def test_shared_bindings_cannot_invent_originals_or_override_existing_symbols(project, damage):
    view, snapshot = project_view(project)
    bindings = list(read_shared(project, snapshot["image_sha256"]))
    item = bindings[0]
    if damage == "extent": bindings[0] = replace(item, span=Span(item.span.start, item.span.end + 1))
    elif damage == "origin": bindings[0] = replace(item, origin=replace(item.origin, address=0x401030), span=Span(0x401030, 0x401031))
    elif damage == "anchor": bindings[0] = replace(item, evidence=replace(item.evidence, address=0x401040))
    elif damage == "duplicate": bindings.append(item)
    elif damage == "canonical": bindings[0] = replace(item, symbol="_canonical")
    with pytest.raises(ValueError):
        validate_shared(bindings, view, reserved=["_g"] if damage == "reserved" else ())


def test_evidence_anchor_requires_owner_even_after_caller_loses_its_reference(project):
    path = project / "code/methods.cc"
    path.write_text(path.read_text().replace("_g 0x00401020", "_g 0x00401000"))
    def no_reference(path, command, flags):
        return coff_fixture(b"\x90" * 5 + b"\xc3") if path.name == "caller.cc" else compiler(path, command, flags)
    report = verify_task(project, "owner", project / "image.exe", compiler=no_reference)
    assert report["translation_units"] == ["code/caller.cc", "code/methods.cc"]
    assert len(report["shared_functions"]) == 4
    assert all(row["result"] == "pass" for row in report["shared_functions"])
    assert report["result"] == "fail"  # The damaged caller is still wrong.


def test_declared_shared_requirements_need_every_method_and_every_trial(project):
    from match_recovery.structural_requirements import check_requirements, declared_outcome
    from test_structural_requirements import inventory

    view, snapshot = project_view(project)
    bindings = read_shared(project, snapshot["image_sha256"])
    report = verify_task(project, "owner", project / "image.exe", compiler=compiler)
    facts = inventory(*((item, True) for item in bindings))
    candidate = {"checks": {"binary": report}}
    checked = check_requirements(facts, candidate)
    assert len(checked) == 2 and all(row["counts"]["pass"] == 2 for row in checked)
    assert declared_outcome(checked, {})[0] == "pass"
    report["shared_functions"][-1]["result"] = "fail"
    assert declared_outcome(check_requirements(facts, candidate), {})[0] == "fail"
    report["shared_functions"].pop()
    assert declared_outcome(check_requirements(facts, candidate), {})[0] == "unknown"
    report["shared_owners"] = []
    assert all(row["result"] == "unknown" for row in check_requirements(facts, candidate))


def test_source_history_retains_removed_shared_interpretations(project):
    from match_recovery.source_obligations import task_obligations
    from test_task_source import git

    path = project / "code/methods.cc"
    path.write_text(path.read_text().replace("_g 0x00401020", "_g 0x00401000").replace("_h 0x00401020", "_h 0x00401000"))
    store = TaskStore(project)
    task = store.read("owner")
    (project / "recovery/tasks/owner.json").unlink()
    git(project, "init", "--initial-branch=work")
    git(project, "add", ".")
    git(project, "commit", "-m", "Shared method source evidence")
    store.create(replace(task, base_revision=git(project, "rev-parse", "HEAD")))
    git(project, "add", ".")
    git(project, "commit", "-m", "Authorize synthetic caller")
    before = task_obligations(project, "owner")
    assert len(before["interpretations"]) == 2 and all(row["current"] for row in before["interpretations"])
    path.write_text("void g() {}\nvoid h() {}\n")
    after = task_obligations(project, "owner")
    assert {row["obligation_id"] for row in before["obligations"]} == {row["obligation_id"] for row in after["obligations"]}
    assert len(after["interpretations"]) == 2 and not any(row["current"] for row in after["interpretations"])


def test_shared_preservation_uses_evidence_scope_without_collapsing_siblings(project):
    from match_recovery.preservation import _bindings, _outside_annotations, _unit_scope
    from match_recovery.source_config import SourceConfiguration

    configuration = SourceConfiguration.read(project)
    view, snapshot = project_view(project)
    pin = snapshot["image_sha256"]
    files = {"code/methods.cc": (project / "code/methods.cc").read_bytes()}
    anchor = Origin(pin, "pe:va", 0x401020).entity_id
    rows = _bindings(project, files, configuration, pin)
    allowed, outside = _unit_scope(rows, frozenset({anchor}), "code/methods.cc")
    assert set(allowed) == {"_g", "_h"} and len(set(allowed.values())) == 2 and not outside
    assert _unit_scope(rows, frozenset(), "code/methods.cc") == ({}, {"_g", "_h"})
    assert len(_outside_annotations(files, configuration, pin, frozenset())) == 2
    assert not _outside_annotations(files, configuration, pin, frozenset({anchor}))


def test_proven_shared_body_resolves_generated_table_but_bad_body_does_not():
    from match_recovery.generated_source import GeneratedBinding
    from test_generated import fixture as table_fixture

    _, table_objects, _ = table_fixture(target=0x401020)
    raw = pe_fixture((0x401020).to_bytes(4, "little") + bytes(28) + b"\xc3", absolute_site=0)
    pin = hashlib.sha256(raw).hexdigest()
    origin, evidence = Origin(pin, "pe:va", 0x401020), Origin(pin, "pe:va", 0x401030)
    binding = SharedFunctionBinding(origin, Span(0x401020, 0x401021), "_method", evidence, "code/methods.cc", 1)
    table = GeneratedBinding(Origin(pin, "pe:va", 0x401000), Span(0x401000, 0x401004), "??_7Widget@@6B@", evidence, "code/widget.h", 1)
    for code, expected in ((b"\xc3", "pass"), (b"\x90", "unknown")):
        objects = {**table_objects, "code/methods.cc": Coff(code_object({"_method": code}))}
        _, _, tables, _, shared, _ = check_compiled([], (), [table], objects, PeImage(raw, pin), {}, shared_bindings=[binding])
        assert tables[0]["content"] == expected
        assert shared[0]["result"] == ("pass" if expected == "pass" else "fail")


@pytest.mark.parametrize("bad_replica", [False, True])
def test_candidate_decks_follow_new_shared_reference_with_private_owner_controls(project, bad_replica):
    from match_recovery.variants import measure_variants

    path = project / "code/caller.cc"
    original = path.read_bytes()
    task = TaskStore(project).read("owner")
    deck = {"schema_version": 2, "source": "code/caller.cc", "entity_id": task.subjects[0].entity_id,
        "files": {"code/caller.cc": {"source_sha256": hashlib.sha256(original).hexdigest(), "before": "void f() {}",
                  "control": "void f() { /* damage */ }", "entity_id": task.subjects[0].entity_id}},
        "candidates": {"with-reference": {"code/caller.cc": "void f() { /* proposed */ }"}}}
    class Compiler:
        def __call__(self, path, command, flags):
            if path.name == "caller.cc":
                return coff_fixture(b"\x90" * 5 + b"\xc3")
            return compiler(path, command, flags)

        def overlay(self, path, contents, command, flags):
            if path.name == "methods.cc":
                return code_object({"_g": b"\x90" if bad_replica else b"\xc3", "_h": b"\xc3"})
            if b"proposed" in contents["code/caller.cc"]:
                return compiler(path, command, flags)
            if b"damage" in contents["code/caller.cc"]:
                return coff_fixture(b"\xeb\xfe")
            return self(path, command, flags)
    report = measure_variants(project, "owner", project / "image.exe", deck, compiler=Compiler())
    assert all(not row["shared_functions"] for row in report["baseline"])
    assert len(report["candidates"]) == 2
    for row in report["candidates"]:
        assert row["target"]["result"] == ("unknown" if bad_replica else "pass")
        assert len(row["shared_functions"]) == 2
        assert row["object_dependencies"][0]["result"] == ("unknown" if bad_replica else "pass")
    assert path.read_bytes() == original
