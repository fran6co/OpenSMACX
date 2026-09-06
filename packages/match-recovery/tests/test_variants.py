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
    return {"schema_version": 1, "source": "code/unit.cc", "entity_id": task.subjects[0].entity_id,
            "source_sha256": hashlib.sha256((root / "code/unit.cc").read_bytes()).hexdigest(),
            "before": "/* baseline */", "control": "for (;;) {}", "candidates": {"exact": "/* proposed */", "different": "/* different */"}}


class Compiler:
    def __call__(self, *_):
        return coff_fixture()

    def variant(self, source, contents, command, flags):
        assert source.read_bytes() != contents or b"baseline" in contents
        return coff_fixture(b"\xeb\xfe" if b"for (;;)" in contents else b"\x90" if b"different" in contents else b"\xc3")


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


@pytest.mark.parametrize("damage", ["stale", "absent-anchor", "ambiguous-anchor", "scope", "file", "invalid-schema", "empty"])
def test_invalid_decks_never_reach_compilation(project, damage):
    value = deck(project)
    if damage == "stale": value["source_sha256"] = "0" * 64
    elif damage == "absent-anchor": value["before"] = "absent"
    elif damage == "ambiguous-anchor": value["before"] = " "
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


@pytest.mark.parametrize("selection, expected_same", [(2, True), (4, False), (5, True)])
def test_checksum_normalization_retains_exact_match_policy_and_actual_contents(selection, expected_same):
    original = Coff(object_fixture(selection=selection, parent=1 if selection == 5 else 0))
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
        'data = base64.b64decode(r["source_base64"]) if r["schema_version"] == 3 else b"baseline"\n'
        f'obj = {coff_fixture(b"\xeb\xfe")!r} if b"for (;;)" in data else {coff_fixture(b"\x90")!r} if b"different" in data else {coff_fixture()!r}\n'
        'print(json.dumps({"object_base64":base64.b64encode(obj).decode()}))\n')
    monkeypatch.chdir(root.parent)
    assert main(["tasks", "variants", task.task_id, "--root", str(root), "--image", "image.exe", "--deck", "deck.json", "--jobs", "2"]) == 0
    assert json.loads(capsys.readouterr().out)["status"] == "scored"
