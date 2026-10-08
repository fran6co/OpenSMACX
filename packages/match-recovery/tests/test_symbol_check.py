"""Synthetic source/COFF controls; no executable-derived fixtures."""

import base64
from dataclasses import replace
import json
import struct
import sys

import pytest

from match_recovery.cli import main
from match_recovery.pe_coff import Coff
from match_recovery.symbol_check import check_symbols, function_definition
from test_binary import coff_fixture


def mapped(symbol="_f", address=0x401000):
    return (f"// ordinary leading comment\n// ORIGINAL: 0x{address:08X} sub_{address:x} "
            f"0x{address:08X}-0x{address+1:08X} FILE\n// symbol    {symbol}\nvoid f() {{}}\n")


@pytest.fixture
def project(tmp_path):
    for name in ("code", "recovery", "build"):
        (tmp_path / name).mkdir()
    (tmp_path / "code/PIN").write_text("sha256 " + "a" * 64)
    (tmp_path / "code/f.cc").write_text(mapped())
    (tmp_path / "build/compile_commands.json").write_text(json.dumps([
        {"file": "code/f.cc", "directory": str(tmp_path), "arguments": ["fake", "code/f.cc"]}]))
    config = {"schema_version": 1,
              "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"],
                         "seed_roots": ["code/seeds"], "include_paths": ["code"]},
              "mappings": {"compile_commands": "build/compile_commands.json", "forwarder_macros": []},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "test",
                              "flag_sets": [["first"], ["second"]], "layout_flags": [],
                              "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    return tmp_path


def compile_source(*_):
    return coff_fixture()


@pytest.mark.parametrize("storage", [2, 3])
def test_external_and_static_executable_definitions_are_observed(storage):
    assert function_definition(Coff(coff_fixture(storage=storage)), "_f")["result"] == "pass"


@pytest.mark.parametrize("damage,expected", [("undefined", "unknown"), ("weak", "unknown"),
                                           ("duplicate", "unknown"), ("data", "fail"),
                                           ("not_executable", "fail"), ("end_label", "fail")])
def test_references_data_and_ambiguous_names_are_not_function_definitions(damage, expected):
    obj = Coff(coff_fixture())
    symbol = obj.symbols[0]
    if damage in ("undefined", "weak"):
        obj.symbols[0] = replace(symbol, section=0, storage=105 if damage == "weak" else 2)
    elif damage == "duplicate":
        obj.symbols[1] = replace(symbol, index=1)
    elif damage == "data":
        obj.symbols[0] = replace(symbol, type=0)
    elif damage == "not_executable":
        obj.sections = (replace(obj.sections[0], flags=obj.sections[0].flags & ~0x20000000),)
    else:
        obj.symbols[0] = replace(symbol, value=len(obj.sections[0].data))
    assert function_definition(obj, "_f")["result"] == expected


def test_native_reader_and_one_compile_cover_siblings_without_writing(project):
    path = project / "code/f.cc"
    path.write_text(mapped() + mapped(address=0x401010))
    before = path.read_bytes()
    calls = []
    def compiler(path, args, flags):
        calls.append((path.name, args, flags))
        return compile_source()
    report = check_symbols(project, compiler=compiler)
    assert report["result"] == "pass" and report["counts"] == {"pass": 2}
    assert calls == [("f.cc", (), ("first",))]
    assert report["compile_attempts"] == 1 and report["annotations"] == 2
    assert not report["persisted"] and not report["ready_for_collection"]
    assert path.read_bytes() == before


@pytest.mark.parametrize("first", ["missing", "compile_error", "malformed", "data"])
def test_later_trial_can_emit_the_symbol_without_borrowing_a_failed_object(project, first):
    def compiler(path, args, flags):
        if flags == ("second",):
            return compile_source()
        if first == "compile_error":
            raise ValueError("compiler failed")
        if first == "malformed":
            return b"truncated"
        raw = bytearray(coff_fixture())
        if first == "data":
            at = struct.unpack_from("<I", raw, 8)[0]
            struct.pack_into("<H", raw, at + 14, 0)
        else:
            raw = raw.replace(b"_f\0", b"_h\0")
        return bytes(raw)
    report = check_symbols(project, compiler=compiler)
    assert report["result"] == "pass" and report["compile_attempts"] == 2


def test_undefined_symbol_never_passes_a_missing_name_ceiling(project):
    (project / "code/f.cc").write_text(mapped("_g"))
    report = check_symbols(project, compiler=compile_source)
    assert report["result"] == "unknown" and report["counts"] == {"unknown": 1}
    assert report["compile_attempts"] == 2
    assert all(v["undefined_or_weak"] for v in report["functions"][0]["variants"])


@pytest.mark.parametrize("value", [None, "object", b"", b"broken"])
def test_missing_or_malformed_objects_are_unmeasurable(project, value):
    report = check_symbols(project, compiler=lambda *_: value)
    assert report["result"] == "unknown" and report["counts"] == {"unknown": 1}


def test_other_tu_definition_cannot_satisfy_a_mapping(project):
    (project / "code/g.cc").write_text(mapped(address=0x401010))
    database = project / "build/compile_commands.json"
    entries = json.loads(database.read_text())
    entries.append({**entries[0], "file": "code/g.cc"})
    database.write_text(json.dumps(entries))
    report = check_symbols(project, compiler=lambda path, *_: coff_fixture().replace(b"_f\0", b"_h\0")
                           if path.name == "g.cc" else coff_fixture())
    assert report["result"] == "unknown" and report["counts"] == {"pass": 1, "unknown": 1}


@pytest.mark.parametrize("file", ["code/unbuilt.cc", "code/interface.h"])
def test_unbuilt_and_header_mappings_do_not_disappear(project, file):
    (project / file).write_text(mapped(address=0x401010))
    report = check_symbols(project, compiler=compile_source)
    assert report["counts"] == {"pass": 1, "unknown": 1}
    assert report["compile_attempts"] == 1


def test_seeds_and_declared_exclusions_are_separate_from_product_emission(project):
    (project / "code/seeds").mkdir()
    (project / "code/seeds/guess.cc").write_text(mapped("_absent"))
    (project / "code/excluded.cc").write_text(mapped("_absent", 0x401010).replace(" FILE", " EXCLUDED S1"))
    report = check_symbols(project, compiler=compile_source)
    assert report["result"] == "pass" and report["annotations"] == 1


@pytest.mark.parametrize("files", [[], ["missing.cc"], ["code"], ["code/f.cc", "code/f.cc"], ["../f.cc"], "code/f.cc"])
def test_invalid_or_empty_file_selection_is_refused(project, files):
    with pytest.raises(ValueError):
        check_symbols(project, files=files, compiler=compile_source)


@pytest.mark.parametrize("damage", ["source", "header", "database", "policy", "pin"])
def test_changed_inputs_cannot_mix_symbol_evidence(project, damage):
    path = project / {"source": "code/f.cc", "header": "code/new.h", "database": "build/compile_commands.json",
                      "policy": "recovery/project.json", "pin": "code/PIN"}[damage]
    def compiler(*_):
        path.write_bytes((path.read_bytes() if path.exists() else b"") + b"\n")
        return compile_source()
    with pytest.raises(ValueError, match="changed"):
        check_symbols(project, compiler=compiler)


def test_cli_uses_configured_compiler_protocol_not_build_objects_or_nm(project, capsys):
    response = base64.b64encode(coff_fixture()).decode()
    (project / "recovery/compiler.py").write_text(
        "import json,sys\nr=json.load(sys.stdin)\n"
        "assert r['profile']=='test' and r['source']=='code/f.cc'\n"
        "print(json.dumps({'object_base64':" + repr(response) + "}))\n")
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["compiler"] = {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 10},
                          "inputs": ["recovery/compiler.py"], "environment": []}
    path.write_text(json.dumps(config))
    assert main(["check-symbols", "--root", str(project), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["counts"] == {"pass": 1} and report["functions"] == []


def test_missing_emission_is_exit_three_not_green(project, monkeypatch, capsys):
    import match_recovery.symbol_check as module
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: lambda *_: b"truncated")
    assert main(["check-symbols", "--root", str(project)]) == 3
    assert json.loads(capsys.readouterr().out)["counts"] == {"unknown": 1}


def test_sparse_unit_profile_is_the_only_symbol_trial_for_its_owner(project):
    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["measurement"]["unit_flag_sets"] = {"code/f.cc": [1]}
    config_path.write_text(json.dumps(config))
    calls = []

    def compiler(path, command, flags):
        calls.append((path.name, flags))
        return compile_source()

    result = check_symbols(project, compiler=compiler)
    assert calls == [("f.cc", ("second",))]
    assert result["unit_flag_sets"] == {"code/f.cc": [["second"]]}
