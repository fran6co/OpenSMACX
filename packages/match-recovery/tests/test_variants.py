"""Portable synthetic recovery project: no target modules or proprietary data."""

from dataclasses import replace
import hashlib
import json
import struct
import sys
import threading
import copy

import pytest

from match_recovery.cli import main
from match_recovery.coff_replica import function_signature, object_signature
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff
from match_recovery.tasks import Task, TaskStore
from match_recovery.variants import measure_variants
from test_binary import coff_fixture, pe_fixture
from test_multispan import object_fixture


@pytest.fixture
def project(tmp_path):
    root = tmp_path
    (root / "code").mkdir()
    (root / "recovery").mkdir()
    image = pe_fixture()
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/PIN").write_text("sha256 " + pin + "\n")
    (root / "code/unit.cc").write_text("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nvoid f() { /* baseline */ }\n")
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "arbitrary-profile", "flag_sets": [["-O2"]],
                              "layout_flags": [], "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    task = Task("ownership", "any-project", "coordinator", "Recover this owner", "a" * 40,
                (Origin(pin, "pe:va", 0x401000),), ("code/unit.cc",), ("synthetic ownership evidence",))
    TaskStore(root).create(task)
    return root, task


def deck(project):
    root, task = project
    return {"schema_version": 2, "source": "code/unit.cc", "entity_id": task.subjects[0].entity_id,
            "files": {"code/unit.cc": {"source_sha256": hashlib.sha256((root / "code/unit.cc").read_bytes()).hexdigest(),
                                       "before": "/* baseline */", "control": "for (;;) {}", "entity_id": task.subjects[0].entity_id}},
            "candidates": {"exact": {"code/unit.cc": "/* proposed */"}, "different": {"code/unit.cc": "/* different */"}}}


class Compiler:
    def __call__(self, *_):
        return coff_fixture()

    def variant(self, source, contents, command, flags):
        assert source.read_bytes() != contents or b"baseline" in contents
        return coff_fixture(b"\xeb\xfe" if b"for (;;)" in contents else b"\x90" if b"different" in contents else b"\xc3")

    def overlay(self, source, replacements, command, flags):
        return self.variant(source, replacements["code/unit.cc"], command, flags)


def measure(project, value=None, compiler=None, **kwargs):
    root, task = project
    return measure_variants(root, task.task_id, root / "image.exe", deck(project) if value is None else value,
                            compiler=compiler or Compiler(), **kwargs)


def test_deck_reuses_source_map_and_scores_without_writing_or_banking(project, monkeypatch):
    from match_recovery import variants
    original_view = variants.project_view
    calls = []
    monkeypatch.setattr(variants, "project_view", lambda root: (calls.append(root), original_view(root))[1])
    root, _ = project
    before = {p.relative_to(root): p.read_bytes() for p in root.rglob("*") if p.is_file()}
    report = measure(project)
    assert report["status"] == "scored"
    assert report["compile_attempts"] == 5 and len(calls) == 1
    assert report["controls"][0]["result"] == "pass"
    assert [row["target"]["result"] for row in report["candidates"]] == ["pass", "fail"]
    assert report["rankings"] == [{"flags": ["-O2"], "names": ["exact", "different"]}]
    assert not report["ready_for_collection"] and not report["scope_complete"] and not report["persisted"]
    assert report["structure_verification"] == "unknown"
    assert {p.relative_to(root): p.read_bytes() for p in root.rglob("*") if p.is_file()} == before


def test_multiple_edits_use_original_byte_ranges_without_cascading(project):
    root, _ = project
    path = root / "code/unit.cc"
    original = "// π\r\nint one; int two;\r\n".encode() + path.read_bytes()
    path.write_bytes(original)
    value = deck(project)
    # Deliberately not source order. The first replacement contains another
    # anchor's text, but that new text must not be replaced a second time.
    value["files"]["code/unit.cc"].update(
        before=["/* baseline */", "int two;", "int one;"],
        control=["for (;;) {}", "int two;", "int one;"])
    value["candidates"] = {"coupled": {"code/unit.cc": ["/* proposed */", "int final;", "int two;"]}}
    class Observed(Compiler):
        def __init__(self):
            self.contents = []
        def variant(self, source, contents, *args):
            self.contents.append(contents)
            return super().variant(source, contents, *args)
    compiler = Observed()
    report = measure(project, value, compiler=compiler)
    assert report["status"] == "scored" and report["controls"][0]["result"] == "pass"
    assert compiler.contents[-1].startswith("// π\r\nint two; int final;\r\n".encode())
    assert b"/* proposed */" in compiler.contents[-1]
    assert path.read_bytes() == original


@pytest.mark.parametrize("damage", ["duplicate", "nested", "partial-overlap", "overlapping-occurrences",
                                    "empty", "empty-anchor", "non-string", "short-control", "short-candidate"])
def test_invalid_multi_edits_refuse_before_compilation(project, damage):
    root, _ = project
    path = root / "code/unit.cc"
    path.write_text(path.read_text() + "// abcdef aaaa\n")
    value = deck(project)
    row = value["files"]["code/unit.cc"]
    row.update(before=["/* baseline */", "abcdef"], control=["for (;;) {}", "abcdef"])
    value["candidates"] = {"coupled": {"code/unit.cc": ["/* proposed */", "changed"]}}
    if damage == "duplicate": row["before"] = ["abcdef", "abcdef"]
    elif damage == "nested": row["before"] = ["abcdef", "bcd"]
    elif damage == "partial-overlap": row["before"] = ["abcd", "cdef"]
    elif damage == "overlapping-occurrences": row["before"] = ["/* baseline */", "aaa"]
    elif damage == "empty": row["before"] = []
    elif damage == "empty-anchor": row["before"][1] = ""
    elif damage == "non-string": row["before"][1] = 12
    elif damage == "short-control": row["control"] = "for (;;) {}"
    else: value["candidates"]["coupled"]["code/unit.cc"] = ["/* proposed */"]
    with pytest.raises(ValueError):
        measure(project, value, compiler=object())


def test_adjacent_anchors_allow_deletion_without_changing_other_bytes(project):
    value = deck(project)
    value["files"]["code/unit.cc"].update(before=["/* baseline", " */"], control=["for (;;) {}", ""])
    value["candidates"] = {"empty": {"code/unit.cc": ["", ""]}}
    report = measure(project, value)
    assert report["status"] == "scored" and report["candidates"][0]["target"]["result"] == "pass"


def test_multi_edits_cannot_redefine_original_evidence(project):
    value = deck(project)
    value["files"]["code/unit.cc"].update(
        before=["/* baseline */", "0x00401001"], control=["for (;;) {}", "0x00401001"])
    value["candidates"] = {"changed-span": {"code/unit.cc": ["/* proposed */", "0x00401002"]}}
    with pytest.raises(ValueError, match="original identities, spans or historical claims"):
        measure(project, value, compiler=object())


@pytest.mark.parametrize("damage", ["replica", "ignored-control", "compile-control"])
def test_bad_replica_and_wrong_target_control_prevent_scoring(project, damage):
    class Bad(Compiler):
        def variant(self, source, contents, *args):
            if damage == "replica" and contents == source.read_bytes():
                return coff_fixture(b"\x90")
            if b"for (;;)" in contents:
                if damage == "compile-control":
                    raise ValueError("damage control did not compile")
                return coff_fixture()
            return super().variant(source, contents, *args)
    result = measure(project, compiler=Bad())
    assert result["status"] == "incomplete" and not result["candidates"]
    assert result["controls"][0]["result"] == "unknown"


def renamed_deck(project, symbol="_r"):
    value = deck(project)
    value["files"]["code/unit.cc"].update(
        before="void f() { /* baseline */ }", control="void f() { for (;;) {} }")
    value["candidates"] = {"renamed": {"code/unit.cc": f"// symbol    {symbol}\nvoid renamed() {{}}"}}
    return value


def test_candidate_cannot_pass_by_emitting_the_old_symbol_after_a_binding_change(project):
    result = measure(project, renamed_deck(project))
    assert result["controls"][0]["result"] == "pass"
    trial = result["candidates"][0]
    assert trial["target"]["symbol"] == "_r"
    assert trial["target"]["result"] == "unknown"


def test_candidate_can_measure_a_new_symbol_without_writing_the_tree(project):
    class Renamed(Compiler):
        def variant(self, source, contents, *args):
            emitted = super().variant(source, contents, *args)
            return emitted.replace(b"_f", b"_r") if b"// symbol" in contents else emitted
    root, _ = project
    before = (root / "code/unit.cc").read_bytes()
    result = measure(project, renamed_deck(project), compiler=Renamed())
    assert result["baseline"][0]["target"]["symbol"] == "_f"
    trial = result["candidates"][0]
    assert trial["target"]["symbol"] == "_r" and trial["target"]["result"] == "pass"
    assert trial["binding_changes"] == [{"entity_id": project[1].subjects[0].entity_id,
                                        "unit": "code/unit.cc", "before": "_f", "after": "_r"}]
    assert (root / "code/unit.cc").read_bytes() == before


@pytest.mark.parametrize("reference,expected", [("_r", "pass"), ("_f", "unknown")])
def test_renamed_candidate_references_cannot_borrow_the_old_external_binding(project, reference, expected):
    root, task = project
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")  # call self
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/PIN").write_text("sha256 " + pin + "\n")
    source = root / "code/unit.cc"
    source.write_text(source.read_text().replace("-0x00401001", "-0x00401006"))
    task = replace(task, task_id="recursive", subjects=(Origin(pin, "pe:va", 0x401000),))
    TaskStore(root).create(task)
    project = root, task

    class Recursive(Compiler):
        def __call__(self, *_):
            return coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)).replace(b"_g", b"_f")

        def variant(self, source, contents, *args):
            if b"for (;;)" in contents:
                return coff_fixture(b"\xeb\xfe")
            if b"// symbol" in contents:
                return coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)).replace(
                    b"_f", b"_r").replace(b"_g", reference.encode())
            return self()

    report = measure(project, renamed_deck(project), compiler=Recursive())
    assert report["baseline"][0]["target"]["result"] == "pass"
    assert report["controls"][0]["result"] == "pass"
    assert report["candidates"][0]["target"]["result"] == expected


@pytest.mark.parametrize("damage", ["remove", "add", "address", "span", "catalogue", "mode", "claim", "body", "kind"])
def test_candidate_cannot_redefine_original_evidence_or_claims(project, damage):
    original = (project[0] / "code/unit.cc").read_text()
    value = deck(project)
    value["files"]["code/unit.cc"].update(before=original, control=original.replace("/* baseline */", "for (;;) {}"))
    after = original
    if damage == "remove": after = "void f() {}\n"
    elif damage == "add": after += "// ORIGINAL: 0x00401010 _g 0x00401010-0x00401011\nvoid g() {}\n"
    elif damage == "address": after = after.replace("ORIGINAL: 0x00401000", "ORIGINAL: 0x00401010")
    elif damage == "span": after = after.replace("-0x00401001", "-0x00401002")
    elif damage == "catalogue": after = after.replace(" _f ", " _r ")
    elif damage == "mode": after = after.replace("0x00401001", "0x00401001 FILE")
    elif damage == "claim": after = after.replace("0x00401001", "0x00401001 BYTE_EXACT")
    elif damage == "body": after = after.replace("void f()", "// body code/elsewhere.h\nvoid f()")
    elif damage == "kind": after = after.replace("void f()", "// kind library\nvoid f()")
    value["candidates"] = {damage: {"code/unit.cc": after}}
    with pytest.raises(ValueError, match="original identities, spans or historical claims"):
        measure(project, value, compiler=object())


def test_damage_control_must_not_satisfy_its_witness_by_changing_its_binding(project):
    value = renamed_deck(project)
    value["files"]["code/unit.cc"]["control"] = "// symbol _r\nvoid f() { for (;;) {} }"
    with pytest.raises(ValueError, match="damage controls"):
        measure(project, value, compiler=object())


def test_renaming_an_unselected_neighbor_requires_scope_expansion(project):
    root, _ = project
    source = root / "code/unit.cc"
    source.write_text(source.read_text() + "// ORIGINAL: 0x00401010 _h 0x00401010-0x00401011\nvoid h() {}\n")
    original = source.read_text()
    value = deck(project)
    value["files"]["code/unit.cc"].update(before=original, control=original.replace("/* baseline */", "for (;;) {}"))
    value["candidates"] = {"outside": {"code/unit.cc": original.replace("void h()", "// symbol _r\nvoid h()")}}
    with pytest.raises(ValueError, match="task authorization"):
        measure(project, value, compiler=object())


def test_proposed_symbol_collision_is_refused_before_compilation(project):
    (project[0] / "code/other.cc").write_text("// ORIGINAL: 0x00401010 _r 0x00401010-0x00401011\nvoid r() {}\n")
    with pytest.raises(ValueError, match="ambiguous bindings"):
        measure(project, renamed_deck(project), compiler=object())


@pytest.mark.parametrize("annotation", [
    "// RECOVERY_OBJECT: v1 0x00401100 _object",
    "// RECOVERY_GENERATED: v1 vtable 0x00401100 ? _table 0x00401000",
    "// RECOVERY_MEMBER: v1 C member 0x10 0x00401000",
])
def test_other_recovery_interpretations_cannot_change_under_baseline_scoring(project, annotation):
    value = deck(project)
    value["candidates"] = {"annotation": {"code/unit.cc": "\n" + annotation + "\n"}}
    with pytest.raises(ValueError, match="recovery interpretations"):
        measure(project, value, compiler=object())


@pytest.mark.parametrize("damage", ["stale", "absent-anchor", "ambiguous-anchor", "scope", "file", "invalid-schema", "empty"])
def test_invalid_decks_never_reach_compilation(project, damage):
    value = deck(project)
    if damage == "stale": value["files"]["code/unit.cc"]["source_sha256"] = "0" * 64
    elif damage == "absent-anchor": value["files"]["code/unit.cc"]["before"] = "absent"
    elif damage == "ambiguous-anchor": value["files"]["code/unit.cc"]["before"] = " "
    elif damage == "scope": value["entity_id"] = "not-authorized"
    elif damage == "file": value["source"] = "../elsewhere"
    elif damage == "invalid-schema": value["schema_version"] = True
    else: value["candidates"] = {}
    with pytest.raises(ValueError):
        measure(project, value, compiler=object())


def test_one_failed_candidate_does_not_erase_the_other_trials(project):
    class Failing(Compiler):
        def variant(self, source, contents, *args):
            if b"different" in contents:
                raise ValueError("candidate does not compile")
            return super().variant(source, contents, *args)
    result = measure(project, compiler=Failing())
    assert result["status"] == "incomplete"
    assert [item["target"]["result"] for item in result["candidates"]] == ["pass", "unknown"]


def test_candidate_compiles_can_overlap_at_requested_width(project):
    barrier = threading.Barrier(2, timeout=5)
    class Concurrent(Compiler):
        def variant(self, source, contents, *args):
            if b"proposed" in contents or b"different" in contents:
                barrier.wait()
            return super().variant(source, contents, *args)
    assert measure(project, compiler=Concurrent(), jobs=2)["status"] == "scored"


def test_default_candidate_compiles_retain_main_thread_signal_cleanup(project):
    class MainThread(Compiler):
        def variant(self, *args):
            assert threading.current_thread() is threading.main_thread()
            return super().variant(*args)
    assert measure(project, compiler=MainThread())["status"] == "scored"


@pytest.mark.parametrize("damage", ["source", "header", "image", "scope"])
def test_inputs_changed_during_trial_invalidate_the_whole_deck(project, damage):
    root, task = project
    class Changing(Compiler):
        def variant(self, source, contents, *args):
            obj = super().variant(source, contents, *args)
            if b"different" in contents:
                if damage == "scope":
                    TaskStore(root).expand(task.task_id, request_id="expand", author=task.author, subjects=(),
                                           editable_files=("code/another.h",), evidence=("discovery",), expected_revision=0)
                else:
                    name = {"source": "code/unit.cc", "header": "code/new.h", "image": "image.exe"}[damage]
                    (root / name).write_bytes(b"newer input")
            return obj
    with pytest.raises(ValueError, match="changed"):
        measure(project, compiler=Changing())


def test_replica_signature_keeps_relocation_identity_and_ignores_timestamp():
    first = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    changed = bytearray(first)
    struct.pack_into("<I", changed, 4, 12345)
    assert object_signature(Coff(first)) == object_signature(Coff(bytes(changed)))
    changed = first.replace(b"_g", b"_h")
    assert object_signature(Coff(first)) != object_signature(Coff(changed))


def test_replica_control_retains_bss_extent_even_without_raw_data():
    first = bytearray(coff_fixture())
    struct.pack_into("<I", first, 20 + 20, 0)  # uninitialized section has no raw data pointer
    struct.pack_into("<I", first, 20 + 36, 0xC0500080)
    changed = bytearray(first)
    struct.pack_into("<I", changed, 20 + 16, 32)
    assert object_signature(Coff(bytes(first))) != object_signature(Coff(bytes(changed)))


def test_damage_to_an_unrelated_symbol_is_not_a_selected_function_control(project):
    class WrongTarget(Compiler):
        def variant(self, source, contents, *args):
            if b"for (;;)" in contents:
                return coff_fixture().replace(b"_g", b"_h")
            return super().variant(source, contents, *args)
    result = measure(project, compiler=WrongTarget())
    assert result["status"] == "incomplete" and result["candidates"] == []
    assert "selected compiled function" in result["controls"][0]["reason"]


def test_local_symbol_and_section_number_churn_cannot_satisfy_damage_control():
    obj = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    external = function_signature(obj, "_f")
    obj.symbols[1] = replace(obj.symbols[1], name="_other_global")
    assert function_signature(obj, "_f") != external
    obj.symbols[1] = replace(obj.symbols[1], name="$L123", section=1, storage=3)
    local = function_signature(obj, "_f")
    obj.symbols[1] = replace(obj.symbols[1], name="$L456", value=1)
    assert function_signature(obj, "_f") == local


def test_replica_normalizes_explicit_section_symbol_and_association_identities():
    original = Coff(object_fixture())
    replica = copy.deepcopy(original)
    replica.sections = tuple(replace(section, relocations=tuple((offset, target + 20, kind) for offset, target, kind in section.relocations))
                             for section in reversed(original.sections))
    replica.section_sizes = tuple(reversed(original.section_sizes))
    replica.symbols = {index + 20: replace(symbol, index=index + 20, section=3-symbol.section)
                       for index, symbol in original.symbols.items()}
    replica.associations = {1: 2}
    replica.auxiliary = {}
    for index, raw in original.auxiliary.items():
        data = bytearray(raw)
        if data[14] == 5:
            struct.pack_into("<H", data, 12, 2)
        replica.auxiliary[index + 20] = bytes(data)
    assert object_signature(original) == object_signature(replica)
    replica.sections = (replace(replica.sections[0], data=b"\x90" + replica.sections[0].data[1:]), *replica.sections[1:])
    assert object_signature(original) != object_signature(replica)


@pytest.mark.parametrize("selection, expected_same", [(0, True), (1, True), (2, True), (3, True), (4, False), (5, True), (6, True)])
def test_checksum_normalization_retains_exact_match_policy_and_actual_contents(selection, expected_same):
    raw = bytearray(object_fixture(selection=selection, parent=1 if selection == 5 else 0))
    if selection == 0:
        at = 20 + 40 + 36  # Second section header's characteristics.
        struct.pack_into("<I", raw, at, struct.unpack_from("<I", raw, at)[0] & ~0x1000)
    original = Coff(bytes(raw))
    replica = copy.deepcopy(original)
    data = bytearray(replica.auxiliary[3])
    struct.pack_into("<I", data, 8, 123)
    replica.auxiliary[3] = bytes(data)
    assert (object_signature(original) == object_signature(replica)) == expected_same
    replica.sections = (*replica.sections[:1], replace(replica.sections[1], data=b"\x90" + replica.sections[1].data[1:]))
    assert object_signature(original) != object_signature(replica)


def test_replica_weak_aliases_compare_target_identity_not_symbol_table_index():
    from match_recovery.pe_coff import Symbol
    original = Coff(coff_fixture())
    original.symbols[2] = Symbol(2, "_weak", 0, 0, 0, 105)
    original.auxiliary[2] = struct.pack("<II10x", 1, 2)
    replica = copy.deepcopy(original)
    replica.symbols[3] = replace(replica.symbols.pop(1), index=3)
    replica.auxiliary[2] = struct.pack("<II10x", 3, 2)
    assert object_signature(original) == object_signature(replica)
    replica.auxiliary[2] = struct.pack("<II10x", 0, 2)
    assert object_signature(original) != object_signature(replica)


def debug_object(*, index_shift=0, line_offset=100, storage=2):
    """Synthetic function-debug records; no bytes from a compiler/game object."""
    from match_recovery.pe_coff import Symbol
    obj = Coff(coff_fixture(storage=storage))
    obj.symbols[2] = Symbol(2, ".bf", 0, 1, 0, 101)
    obj.symbols[4] = Symbol(4, "_next", 1, 1, 0x20, storage)
    obj.symbols[6] = Symbol(6, ".bf", 1, 1, 0, 101)
    obj.symbols = {i + index_shift: replace(s, index=i + index_shift) for i, s in obj.symbols.items()}
    obj.auxiliary = {
        index_shift: struct.pack("<IIIIH", 2 + index_shift, 1, line_offset, 4 + index_shift, 0),
        2 + index_shift: struct.pack("<IH6xIH", 0, 10, 6 + index_shift, 0),
        4 + index_shift: struct.pack("<IIIIH", 6 + index_shift, 1, line_offset + 6, 0, 0),
        6 + index_shift: struct.pack("<IH6xIH", 0, 11, 0, 0),
    }
    return obj


@pytest.mark.parametrize("storage", [2, 3])
def test_replica_normalizes_function_debug_links_not_sizes_or_source_data(storage):
    original = debug_object(storage=storage)
    replica = debug_object(index_shift=30, line_offset=200, storage=storage)
    assert object_signature(original) == object_signature(replica)
    replica.sections = (replace(replica.sections[0], data=b"\xcc" * 16),)
    assert object_signature(original) != object_signature(replica)


@pytest.mark.parametrize("symbol, offset, fmt, value", [
    (0, 4, "<I", 2),  # function extent
    (0, 8, "<I", 0),  # absence of line records (not their file offsets)
    (0, 12, "<I", 0),  # next-function identity
    (0, 16, "<H", 1),  # unsupported/unused fields remain conservative
    (2, 4, "<H", 20),  # source line
    (2, 12, "<I", 0),  # next-begin identity
])
def test_replica_debug_normalization_preserves_other_metadata(symbol, offset, fmt, value):
    original = debug_object()
    replica = copy.deepcopy(original)
    raw = bytearray(replica.auxiliary[symbol])
    struct.pack_into(fmt, raw, offset, value)
    replica.auxiliary[symbol] = bytes(raw)
    assert object_signature(original) != object_signature(replica)


@pytest.mark.parametrize("symbol, offset, target", [
    (0, 0, 999), (0, 0, 1), (0, 12, 2), (2, 12, 4), (2, 12, 999),
])
def test_replica_debug_normalization_refuses_missing_or_wrong_kind_links(symbol, offset, target):
    obj = debug_object()
    raw = bytearray(obj.auxiliary[symbol])
    struct.pack_into("<I", raw, offset, target)
    obj.auxiliary[symbol] = bytes(raw)
    with pytest.raises(ValueError, match="function-debug auxiliary reference"):
        object_signature(obj)


@pytest.mark.parametrize("symbol", [0, 2])
def test_replica_debug_normalization_refuses_unknown_record_extents(symbol):
    obj = debug_object()
    obj.auxiliary[symbol] += bytes(18)
    with pytest.raises(ValueError, match="auxiliary extent"):
        object_signature(obj)


def test_package_cli_runs_deck_with_only_local_compiler_configuration(project, capsys, monkeypatch):
    root, task = project
    value = deck(project)
    (root / "deck.json").write_text(json.dumps(value))
    path = root / "recovery/project.json"
    config = json.loads(path.read_text())
    config["compiler"] = {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 5},
                          "inputs": ["recovery/compiler.py"], "environment": []}
    path.write_text(json.dumps(config))
    (root / "recovery/compiler.py").write_text(
        'import json, sys, base64\nr = json.load(sys.stdin)\n'
        'data = base64.b64decode(r["overrides"]["code/unit.cc"]["base64"]) if r["schema_version"] == 4 else b"baseline"\n'
        f'obj = {coff_fixture(b"\xeb\xfe")!r} if b"for (;;)" in data else {coff_fixture(b"\x90")!r} if b"different" in data else {coff_fixture()!r}\n'
        'print(json.dumps({"object_base64":base64.b64encode(obj).decode()}))\n')
    monkeypatch.chdir(root.parent)
    assert main(["tasks", "variants", task.task_id, "--root", str(root), "--image", "image.exe", "--deck", "deck.json", "--jobs", "2"]) == 0
    assert json.loads(capsys.readouterr().out)["status"] == "scored"


@pytest.fixture
def coupled_project(project):
    root, task = project
    (root / "code/model.h").write_text("struct Model { int value; };\n")
    (root / "code/other.cc").write_text("// ORIGINAL: 0x00401010 _h 0x00401010-0x00401011\nvoid h() {}\n")
    (root / "code/unmeasured.cc").write_text("// ORIGINAL: 0x00401020 _k 0x00401020-0x00401021\nvoid k() {}\n")
    other = Origin(task.subjects[0].image_sha256, "pe:va", 0x401010)
    TaskStore(root).expand(task.task_id, request_id="coupled", author=task.author, subjects=(other,),
                           editable_files=("code/model.h", "code/other.cc"), evidence=("shared member ownership",), expected_revision=0)
    value = {"schema_version": 2, "source": "code/unit.cc", "entity_id": task.subjects[0].entity_id,
             "files": {
                 "code/unit.cc": {"source_sha256": hashlib.sha256((root / "code/unit.cc").read_bytes()).hexdigest(),
                                  "before": "/* baseline */", "control": "for (;;) {}", "entity_id": task.subjects[0].entity_id},
                 "code/model.h": {"source_sha256": hashlib.sha256((root / "code/model.h").read_bytes()).hexdigest(),
                                  "before": "int value;", "control": "int damaged; int value;", "entity_id": other.entity_id}},
             "candidates": {"coupled": {"code/unit.cc": "/* proposed */", "code/model.h": "int base; int value;"},
                            "original": {"code/unit.cc": "/* baseline */", "code/model.h": "int value;"}}}
    return project, value, other


class CoupledCompiler:
    def __init__(self):
        self.seen = []

    def __call__(self, source, *args):
        return coff_fixture().replace(b"_f", b"_h") if source.name == "other.cc" else coff_fixture()

    def overlay(self, source, replacements, *args):
        self.seen.append((source.name, replacements.copy()))
        code = b"\xc3"
        if b"for (;;)" in replacements["code/unit.cc"] or b"damaged" in replacements["code/model.h"]:
            code = b"\xeb\xfe"
        elif b"base;" in replacements["code/model.h"]:
            # Both header and body are needed for the proposed model to compile.
            if b"proposed" not in replacements["code/unit.cc"]:
                raise ValueError("header and body disagree")
            code = b"\x90"
        obj = coff_fixture(code)
        return obj.replace(b"_f", b"_h") if source.name == "other.cc" else obj


def test_multi_file_candidates_are_atomic_and_cover_all_task_units(coupled_project):
    project, value, other = coupled_project
    root, _ = project
    before = {p: p.read_bytes() for p in root.rglob("*") if p.is_file()}
    compiler = CoupledCompiler()
    result = measure(project, value, compiler=compiler)
    assert result["status"] == "scored"
    assert result["translation_units"] == ["code/other.cc", "code/unit.cc"]
    assert result["files"] == ["code/model.h", "code/unit.cc"]
    assert result["compile_attempts"] == 10  # 2 real + 2 replicas + 2 per-file controls + 4 candidates
    assert len(result["controls"][0]["files"]) == len(result["controls"][0]["replicas"]) == 2
    assert result["candidates"][0]["neighbors"][0]["entity_id"] == other.entity_id
    assert result["candidates"][0]["neighbors"][0]["in_task"]
    assert result["unmeasured_product_units"] == ["code/unmeasured.cc"]
    assert result["unmeasured_task_entities"] == []
    assert not result["scope_complete"] and not result["ready_for_collection"]
    assert all(set(replacements) == set(value["files"]) for _, replacements in compiler.seen)
    assert {p: p.read_bytes() for p in root.rglob("*") if p.is_file()} == before


def test_ignoring_only_the_header_cannot_pass_multi_file_controls(coupled_project):
    project, value, _ = coupled_project
    class IgnoresHeader(CoupledCompiler):
        def overlay(self, source, replacements, *args):
            return super().overlay(source, {**replacements, "code/model.h": b"int value;"}, *args)
    result = measure(project, value, compiler=IgnoresHeader())
    assert result["status"] == "incomplete" and result["candidates"] == []
    assert "code/model.h" in result["controls"][0]["reason"]


def test_failure_in_secondary_tu_does_not_hide_primary_diagnostics(coupled_project):
    project, value, other = coupled_project
    class FailsOther(CoupledCompiler):
        def overlay(self, source, replacements, *args):
            if source.name == "other.cc" and b"proposed" in replacements["code/unit.cc"]:
                raise ValueError("dependent caller no longer compiles")
            return super().overlay(source, replacements, *args)
    result = measure(project, value, compiler=FailsOther())
    assert result["status"] == "incomplete"
    trial = result["candidates"][0]
    assert trial["target"]["result"] == "fail"
    assert trial["neighbors"][0]["entity_id"] == other.entity_id and trial["neighbors"][0]["result"] == "unknown"
    assert trial["compile_errors"] == {"code/other.cc": "dependent caller no longer compiles"}
    assert result["rankings"][0]["names"][-1] == "coupled"


@pytest.mark.parametrize("damage", ["scope", "stale", "missing-control", "bad-witness", "partial-candidate", "extra-file", "seed", "symlink"])
def test_each_overlay_file_is_validated_before_any_compilation(coupled_project, damage):
    project, value, _ = coupled_project
    root, _ = project
    if damage == "scope":
        value["files"]["code/unmeasured.cc"] = value["files"].pop("code/model.h")
    elif damage == "stale": value["files"]["code/model.h"]["source_sha256"] = "0" * 64
    elif damage == "missing-control": del value["files"]["code/model.h"]["control"]
    elif damage == "bad-witness": value["files"]["code/model.h"]["entity_id"] = "not-in-scope"
    elif damage == "partial-candidate": del value["candidates"]["coupled"]["code/model.h"]
    elif damage == "extra-file": value["candidates"]["coupled"]["code/outside.h"] = "no"
    elif damage == "seed":
        path = root / "recovery/project.json"
        config = json.loads(path.read_text())
        config["source"]["seed_roots"] = ["code/model.h"]
        path.write_text(json.dumps(config))
    else:
        (root / "code/model.h").unlink()
        (root / "code/model.h").symlink_to("unit.cc")
    with pytest.raises(ValueError):
        measure(project, value, compiler=object())


def test_secondary_tu_replica_must_agree_before_scoring(coupled_project):
    project, value, _ = coupled_project
    class BadOtherReplica(CoupledCompiler):
        def overlay(self, source, replacements, *args):
            if source.name == "other.cc":
                return coff_fixture(b"\x90").replace(b"_f", b"_h")
            return super().overlay(source, replacements, *args)
    result = measure(project, value, compiler=BadOtherReplica())
    assert result["status"] == "incomplete" and result["candidates"] == []
    assert "code/other.cc" in result["controls"][0]["reason"]


def test_compiled_neighbors_are_reported_even_without_task_authorization(project):
    root, task = project
    source = root / "code/unit.cc"
    source.write_text(source.read_text() + "\n// ORIGINAL: 0x00401010 _h 0x00401010-0x00401011\nvoid h() {}\n")
    def object_with_neighbor(damaged=False):
        data = bytearray(object_fixture(selection=2, parent=0))
        offset = struct.unpack_from("<I", data, 8)[0] + 5 * 18
        data[offset:offset + 8] = b"_h\0\0\0\0\0\0"
        struct.pack_into("<H", data, offset + 14, 0x20)
        data[offset + 16] = 2
        return bytes(data).replace(b"\xe8\0\0\0\0\xc3", b"\xe9\0\0\0\0\xc3") if damaged else bytes(data)
    class WithNeighbor(Compiler):
        def __call__(self, *args):
            return object_with_neighbor()
        def variant(self, source, contents, *args):
            return object_with_neighbor(b"for (;;)" in contents)
    result = measure(project, compiler=WithNeighbor())
    neighbor = result["candidates"][0]["neighbors"][0]
    assert neighbor["entity_id"] == Origin(task.subjects[0].image_sha256, "pe:va", 0x401010).entity_id
    assert neighbor["in_task"] is False and neighbor["symbol"] == "_h"
    assert result["status"] == "scored" and not result["scope_complete"]
