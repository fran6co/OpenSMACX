"""Referenced global owners are compiler inputs, not fabricated symbol aliases."""

import hashlib
import json
import struct
from dataclasses import replace

import pytest

from match_recovery.application_binary import verify_task
from match_recovery.model import Origin
from match_recovery.object_source import ObjectBinding, referenced_object_units
from match_recovery.pe_coff import Coff
from match_recovery.tasks import Task, TaskStore
from test_binary import coff_fixture, pe_fixture


def data_object(name=b"_g", reference=None):
    raw = bytearray(coff_fixture(bytes(4), relocation=(0, 1, 6) if reference else None))
    struct.pack_into("<I", raw, 56, 0xC0300040)  # data, not executable code
    symbols = struct.unpack_from("<I", raw, 8)[0]
    raw[symbols:symbols + 8] = name.ljust(8, b"\0")
    struct.pack_into("<H", raw, symbols + 14, 0)
    raw[symbols + 18:symbols + 26] = (reference or b"_unused").ljust(8, b"\0")
    return bytes(raw)


@pytest.fixture
def project(tmp_path):
    root = tmp_path
    (root / "code").mkdir()
    (root / "recovery").mkdir()
    image = pe_fixture(b"\xa1\x20\x10\x40\0\xc3", absolute_site=1)
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/PIN").write_text("sha256 " + pin)
    (root / "code/caller.cc").write_text("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\nint f();\n")
    (root / "code/global.cc").write_text("// RECOVERY_OBJECT: v1 0x00401020 _g\nint g;\n")
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va",
              "roots": ["code"], "include_paths": ["code"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
              "flag_sets": [["first"], ["second"]], "layout_flags": [], "unit_extensions": [".cc"],
              "inputs": [], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    task = Task("owner", "synthetic", "coordinator", "Recover the caller", "a" * 40,
                (Origin(pin, "pe:va", 0x401000),), ("code/caller.cc",), ("synthetic",))
    TaskStore(root).create(task)
    return root


def check(root, owner=data_object(), failure=None):
    calls = []
    def compile_source(path, command, flags):
        calls.append((path.name, flags))
        if path.name == "caller.cc":
            return coff_fixture(b"\xa1\0\0\0\0\xc3", relocation=(1, 1, 6))
        if failure:
            failure(path, flags)
        return owner
    return verify_task(root, "owner", root / "image.exe", compiler=compile_source), calls


def test_unselected_global_owner_compiles_once_per_trial_without_expanding_edit_scope(project):
    report, calls = check(project)
    assert report["result"] == "pass" and report["compile_attempts"] == 4
    assert calls == [(name, (flag,)) for flag in ("first", "second") for name in ("caller.cc", "global.cc")]
    assert all(row["units"] == ["code/global.cc"] and not row["errors"] for row in report["object_dependencies"])
    assert TaskStore(project).scope("owner")[1] == frozenset({"code/caller.cc"})
    assert not report["ready_for_collection"]


def test_dependency_compile_failure_does_not_borrow_another_trials_object(project):
    def failure(path, flags):
        if flags == ("second",):
            raise ValueError("synthetic missing compiler result")
    report, calls = check(project, failure=failure)
    assert [row["result"] for row in report["functions"][0]["variants"]] == ["pass", "unknown"]
    assert report["object_dependencies"][1]["errors"] == {"code/global.cc": "synthetic missing compiler result"}
    assert len(calls) == 4


def test_wrong_global_binding_fails_instead_of_using_original_operand(project):
    path = project / "code/global.cc"
    path.write_text(path.read_text().replace("0x00401020", "0x00401024"))
    assert check(project)[0]["result"] == "fail"


def test_successful_dependency_compile_without_definition_stays_unknown(project):
    assert check(project, owner=coff_fixture())[0]["result"] == "unknown"


def test_uncompilable_owner_path_is_reported_without_calling_compiler(project):
    (project / "code/global.cc").rename(project / "code/global.h")
    report, calls = check(project)
    assert report["result"] == "unknown" and len(calls) == 2
    assert all(row["errors"] == {"code/global.h": "object owner is not a configured translation unit"}
               for row in report["object_dependencies"])


def test_dependency_cycle_terminates_and_shared_owner_is_not_recompiled(project):
    (project / "code/other.cc").write_text("// RECOVERY_OBJECT: v1 0x00401024 _h\nint h;\n")
    calls = []
    def compile_source(path, command, flags):
        calls.append((path.name, flags))
        if path.name == "caller.cc":
            return coff_fixture(b"\xa1\0\0\0\0\xc3", relocation=(1, 1, 6))
        return data_object(b"_g", b"_h") if path.name == "global.cc" else data_object(b"_h", b"_g")
    report = verify_task(project, "owner", project / "image.exe", compiler=compile_source)
    assert report["result"] == "pass" and report["compile_attempts"] == len(calls) == 6
    assert all(row["units"] == ["code/global.cc", "code/other.cc"] for row in report["object_dependencies"])


@pytest.mark.parametrize("storage,kind,expected", [(2, 6, True), (2, 20, True), (3, 6, False), (105, 6, False), (2, 0, False)])
def test_only_referenced_external_symbols_select_owners(storage, kind, expected):
    binding = ObjectBinding(Origin("a" * 64, "pe:va", 0x402000), "_g", "code/global.cc", 1)
    obj = Coff(coff_fixture(relocation=(0, 1, 6)))
    obj.symbols[1] = replace(obj.symbols[1], storage=storage)
    obj.sections = (replace(obj.sections[0], relocations=((0, 1, kind),)),)
    assert bool(referenced_object_units((binding,), {"code/caller.cc": obj})) == expected
    obj.sections = (replace(obj.sections[0], relocations=()),)
    assert not referenced_object_units((binding,), {"code/caller.cc": obj})


def test_changed_owner_source_invalidates_the_whole_measurement(project):
    def change_source(path, flags):
        path.write_text(path.read_text() + "// raced\n")
    with pytest.raises(ValueError, match="changed during measurement"):
        check(project, failure=change_source)


@pytest.mark.parametrize("bad_replica", [False, True])
def test_candidate_new_global_reference_gets_its_own_owner_replica_control(project, bad_replica):
    from match_recovery.variants import measure_variants
    task = TaskStore(project).read("owner")
    original = (project / "code/caller.cc").read_bytes()
    deck = {"schema_version": 2, "source": "code/caller.cc", "entity_id": task.subjects[0].entity_id,
            "files": {"code/caller.cc": {"source_sha256": hashlib.sha256(original).hexdigest(),
                      "before": "int f();", "control": "int f(); // damaged", "entity_id": task.subjects[0].entity_id}},
            "candidates": {"reference": {"code/caller.cc": "int f(); // proposed"}}}
    calls = []
    class Compiler:
        def __call__(self, source, command, flags):
            calls.append((source.name, flags, "direct"))
            return data_object() if source.name == "global.cc" else coff_fixture(b"\x90" * 5 + b"\xc3")

        def overlay(self, source, replacements, command, flags):
            calls.append((source.name, flags, "overlay"))
            if source.name == "global.cc":
                # No candidate may borrow a dependency from the actual tree
                # without proving the same overlay mechanism works for it.
                raw = bytearray(data_object())
                if bad_replica:
                    raw[60] = 1
                return bytes(raw)
            content = replacements["code/caller.cc"]
            if b"proposed" in content:
                return coff_fixture(b"\xa1\0\0\0\0\xc3", relocation=(1, 1, 6))
            return coff_fixture(b"\xeb\xfe" if b"damaged" in content else b"\x90" * 5 + b"\xc3")
    report = measure_variants(project, "owner", project / "image.exe", deck, compiler=Compiler())
    assert report["compile_attempts"] == len(calls)
    assert all(not row["object_dependencies"] for row in report["baseline"])
    for row in report["candidates"]:
        assert row["object_dependencies"][0]["unit"] == "code/global.cc"
        assert row["target"]["result"] == ("unknown" if bad_replica else "pass")
        if bad_replica:
            assert "replica differs" in row["compile_errors"]["code/global.cc"]
        else:
            assert row["object_dependencies"][0]["result"] == "pass"
    assert report["status"] == ("incomplete" if bad_replica else "scored")
    assert (project / "code/caller.cc").read_bytes() == original
