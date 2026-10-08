import hashlib
import json
import base64
import sys

import pytest

from match_recovery.cli import main
from match_recovery.data_check import check_data
from test_object_data import data_fixture, data_image


@pytest.fixture
def project(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    image = data_image(b"\x07\0\0\0")
    (tmp_path / "image.exe").write_bytes(image)
    (tmp_path / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest())
    (tmp_path / "code/global.cc").write_text("// RECOVERY_OBJECT: v2 0x401000 0x401004 _g 0x401010\nint g = 7;\n")
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va",
              "roots": ["code"], "include_paths": ["code"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "synthetic",
              "flag_sets": [["first"], ["second"]], "layout_flags": ["debug-data"], "unit_extensions": [".cc"],
              "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    return tmp_path


def compiler(*_):
    return data_fixture(b"\x07\0\0\0")


def test_native_cli_and_all_trials_are_read_only(project, monkeypatch, capsys):
    import match_recovery.data_check as module
    calls = []
    def compile_source(path, args, flags):
        calls.append((path.name, args, flags))
        return compiler()
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: compile_source)
    before = {p: p.read_bytes() for p in project.rglob("*") if p.is_file()}
    assert main(["check-data", "--root", str(project), "--image", str(project / "image.exe")]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["selected_objects"] == 1 and report["counts"] == {"pass": 2}
    assert report["compile_attempts"] == 2 and not report["persisted"] and not report["ready_for_collection"]
    assert calls == [("global.cc", (), (flag, "debug-data")) for flag in ("first", "second")]
    assert before == {p: p.read_bytes() for p in project.rglob("*") if p.is_file()}


def test_native_cli_checks_module_local_const_data_without_externalizing_it(project, monkeypatch, capsys):
    import match_recovery.data_check as module
    source = project / "code/global.cc"
    source.write_text(source.read_text().replace("int g = 7;", "const int g = 7;"))
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: lambda *_: data_fixture(b"\x07\0\0\0", storage=3))
    assert main(["check-data", "--root", str(project), "--image", str(project / "image.exe"), "--symbol", "_g"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["counts"] == {"pass": 2} and not report["ready_for_collection"]
    assert "const int g" in source.read_text()


def test_native_cli_uses_configured_compiler_protocol_for_data_only_project(project, capsys):
    response = base64.b64encode(compiler()).decode()
    (project / "recovery/compiler.py").write_text(
        "import json,sys\nr=json.load(sys.stdin)\n"
        "assert r['schema_version']==2 and r['profile']=='synthetic'\n"
        "assert r['source']=='code/global.cc' and r['arguments']==[]\n"
        "assert r['flags'] in [['first','debug-data'],['second','debug-data']]\n"
        "print(json.dumps({'object_base64':" + repr(response) + "}))\n")
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["compiler"] = {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 10},
                          "inputs": ["recovery/compiler.py"], "environment": []}
    path.write_text(json.dumps(config))
    assert main(["check-data", "--root", str(project), "--image", str(project / "image.exe"), "--summary"]) == 0
    assert json.loads(capsys.readouterr().out)["compile_attempts"] == 2


@pytest.mark.parametrize("bad,expected", [("compile", "unknown"), ("data", "fail"), ("debug", "unknown")])
def test_trials_cannot_borrow_a_passing_sibling(project, bad, expected):
    def compile_source(path, args, flags):
        if flags[0] == "second":
            if bad == "compile":
                raise ValueError("failed control")
            if bad == "data":
                return data_fixture()
            return data_fixture(type_index=0xFFFF)
        return compiler()
    report = check_data(project, project / "image.exe", compiler=compile_source)
    assert report["result"] == expected and report["counts"] == {"pass": 1, expected: 1}


@pytest.mark.parametrize("symbols", [[], ["_missing"], ["_g", "_g"], [""], "_g"])
def test_invalid_or_missing_selection_is_refused(project, symbols):
    with pytest.raises(ValueError, match="selection"):
        check_data(project, project / "image.exe", symbols=symbols, compiler=compiler)


def test_unbounded_binding_and_missing_owner_are_not_empty_success(project):
    source = project / "code/global.cc"
    source.write_text("// RECOVERY_OBJECT: v1 0x401000 _g\nint g;\n")
    assert check_data(project, project / "image.exe", compiler=compiler)["result"] == "unknown"
    source.rename(project / "code/global.hpp")
    report = check_data(project, project / "image.exe", compiler=compiler)
    assert report["result"] == "unknown" and report["compile_attempts"] == 0


@pytest.mark.parametrize("damage", ["source", "header", "image", "pin", "policy"])
def test_input_races_refuse_the_entire_initialization_measurement(project, damage):
    def compile_source(*_):
        path = project / {"source": "code/global.cc", "header": "code/new.hpp", "image": "image.exe",
                          "pin": "code/PIN", "policy": "recovery/project.json"}[damage]
        path.write_bytes((path.read_bytes() if path.exists() else b"") + b"\n")
        return compiler()
    with pytest.raises(ValueError, match="changed"):
        check_data(project, project / "image.exe", compiler=compile_source)


def test_wrong_image_is_refused_before_compilation(project):
    (project / "image.exe").write_bytes(data_image())
    with pytest.raises(ValueError, match="digest mismatch"):
        check_data(project, project / "image.exe", compiler=lambda *_: pytest.fail("must not compile"))


def test_selected_initializer_follows_its_data_owner_once_per_trial(project):
    image = data_image((0x401020).to_bytes(4, "little"), absolute=0)
    (project / "image.exe").write_bytes(image)
    (project / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest())
    (project / "code/target.cc").write_text("// RECOVERY_OBJECT: v1 0x401020 _h\nint h;\n")
    calls = []
    def compile_source(path, args, flags):
        calls.append((path.name, flags))
        if path.name == "global.cc":
            return data_fixture(relocations=((0, 1, 6),))
        return data_fixture(definition=b"_h", reference=b"_unseen")
    report = check_data(project, project / "image.exe", symbols=["_g"], compiler=compile_source)
    assert report["result"] == "pass" and report["compile_attempts"] == 4
    assert [name for name, _ in calls] == ["global.cc", "target.cc"] * 2


def test_graph_dependencies_use_their_own_profile_and_record_exact_receipts(project):
    image = data_image((0x401020).to_bytes(4, "little"), absolute=0)
    (project / "image.exe").write_bytes(image)
    (project / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest())
    (project / "code/target.cc").write_text("// RECOVERY_OBJECT: v1 0x401020 _h\nint h;\n")
    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["measurement"]["unit_flag_sets"] = {"code/global.cc": [0]}
    config_path.write_text(json.dumps(config))
    calls = []

    def compile_source(path, args, flags):
        calls.append((path.name, flags))
        if path.name == "global.cc":
            return data_fixture(relocations=((0, 1, 6),))
        return data_fixture(definition=b"_h", reference=b"_unseen")

    report = check_data(project, project / "image.exe", symbols=["_g"], compiler=compile_source)
    assert report["result"] == "pass"
    assert calls == [("global.cc", ("first", "debug-data")),
                     ("target.cc", ("first", "debug-data")),
                     ("target.cc", ("second", "debug-data"))]
    assert report["trials"][0]["unit_flag_sets"] == {
        "code/global.cc": ["first", "debug-data"],
        "code/target.cc": ["first", "debug-data"]}
    assert report["trials"][1]["unit_flag_sets"]["code/global.cc"] == ["first", "debug-data"]
    assert report["trials"][1]["unit_flag_sets"]["code/target.cc"] == ["second", "debug-data"]


def test_task_compiles_declared_data_requirement_even_without_a_code_reference(project):
    from match_recovery.application_binary import verify_task
    from match_recovery.model import Origin
    from match_recovery.tasks import Task, TaskStore
    from test_binary import coff_fixture
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    (project / "code/caller.cc").write_text("// ORIGINAL: 0x401010 _f 0x401010-0x401011\nvoid f();\n")
    task = Task("owner", "synthetic", "coordinator", "Recover data ownership", "a" * 40,
                (Origin(pin, "pe:va", 0x401010),), ("code/caller.cc",), ("synthetic",))
    TaskStore(project).create(task)
    calls = []
    def compile_source(path, args, flags):
        calls.append(path.name)
        return coff_fixture(b"\0") if path.name == "caller.cc" else data_fixture()
    report = verify_task(project, "owner", project / "image.exe", compiler=compile_source)
    assert calls == ["caller.cc", "global.cc"] * 2
    assert report["result"] == "fail"
    assert all(row["content"] == "fail" for row in report["object_definitions"])
    assert len(report["object_owners"]) == 1 and all("debug-data" in flags for flags in report["flag_sets"])


def test_deck_cannot_prefer_matching_code_with_broken_initial_data(project):
    from match_recovery.model import Origin
    from match_recovery.tasks import Task, TaskStore
    from match_recovery.variants import measure_variants
    from test_binary import coff_fixture
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    source = project / "code/caller.cc"
    source.write_text("// ORIGINAL: 0x401010 _f 0x401010-0x401011\nvoid f();\n")
    origin = Origin(pin, "pe:va", 0x401010)
    task = Task("owner", "synthetic", "coordinator", "Recover data ownership", "a" * 40,
                (origin,), ("code/caller.cc",), ("synthetic",))
    TaskStore(project).create(task)
    deck = {"schema_version": 2, "source": "code/caller.cc", "entity_id": origin.entity_id,
            "files": {"code/caller.cc": {"source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                      "before": "void f();", "control": "void f(); // control", "entity_id": origin.entity_id}},
            "candidates": {"a_broken": {"code/caller.cc": "void f(); // broken"},
                           "z_valid": {"code/caller.cc": "void f(); // valid"}}}
    class Compiler:
        def __call__(self, path, args, flags):
            return data_fixture(b"\x07\0\0\0") if path.name == "global.cc" else coff_fixture(b"\0")
        def overlay(self, path, contents, args, flags):
            content = contents["code/caller.cc"]
            if path.name == "global.cc":
                return data_fixture() if b"broken" in content else data_fixture(b"\x07\0\0\0")
            return coff_fixture(b"\x01" if b"control" in content else b"\0")
    report = measure_variants(project, "owner", project / "image.exe", deck, compiler=Compiler())
    assert report["status"] == "scored"
    assert report["translation_units"] == ["code/caller.cc", "code/global.cc"]
    assert all(row["target"]["result"] == "pass" for row in report["candidates"])
    assert all(row["names"] == ["z_valid", "a_broken"] for row in report["rankings"])
    assert all(row["object_definitions"][0]["content"] == ("fail" if row["name"] == "a_broken" else "pass")
               for row in report["candidates"])
