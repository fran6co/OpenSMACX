"""VC6 source-override mechanics, separate from target-independent recovery."""

import json
import base64
import hashlib
import io
import re
import subprocess
from pathlib import Path
import sys

import pytest

from recovery.environment.compile import _variant_arguments, compile_overlay, compile_object
from match_recovery.pe_coff import Coff
from match_recovery.coff_replica import object_signature, function_signature


@pytest.fixture
def variant_project(tmp_path, monkeypatch):
    subprocess.run(["git", "init", "-q", str(tmp_path)], check=True)
    (tmp_path / ".gitignore").write_text("build/\n")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    (tmp_path / "code/detail").mkdir(parents=True)
    (tmp_path / "support").mkdir()
    (tmp_path / "recovery").mkdir()
    (tmp_path / "code/unit.cpp").write_text('#include "detail/value.h"\n#include <value.h>\n'
                                            'const char* location = __FILE__;\n'
                                            'int value() { return LOCAL_VALUE + GLOBAL_VALUE + __LINE__; }\n')
    (tmp_path / "code/detail/value.h").write_text('#pragma once\nstruct Value { int x; };\n#define LOCAL_VALUE 7\n')
    (tmp_path / "support/value.h").write_text('#define GLOBAL_VALUE 9\n')
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/IMAGE_PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "application", "flag_sets": [["/c", "/O2"]],
                        "layout_flags": ["/Z7"], "unit_extensions": [".cpp"], "inputs": [], "input_roots": ["support"]}}))
    return tmp_path


def test_override_rewrites_both_include_forms_and_forced_include(variant_project, monkeypatch):
    root = variant_project
    source = root / "code/unit.cpp"
    candidate = source.read_bytes().replace(b"LOCAL_VALUE + GLOBAL_VALUE", b"GLOBAL_VALUE")
    seen = []
    def compile(source_copy, command, flags, *, temporary_parent):
        seen.append(source_copy)
        replica = source_copy.parent.parent
        assert replica == temporary_parent and replica.is_relative_to(root / "build/recovery/variants")
        assert source_copy.read_bytes().endswith(candidate)
        assert source_copy.read_bytes() == candidate
        assert command == ("compile", "/I" + str(replica / "support"), "/I" + str(replica / "code"))
        assert flags == ("/FI" + str(replica / "code/detail/value.h"), "/c")
        assert (replica / "code/detail/value.h").read_bytes().endswith(b"#define LOCAL_VALUE 7\n")
        assert (root / "code/unit.cpp").read_bytes() != candidate
        return b"object"
    monkeypatch.setattr("recovery.environment.compile.compile_object", compile)
    assert compile_overlay(root, source, {"code/unit.cpp": candidate},
                           ("compile", "-I", str(root / "support"), "/I" + str(root / "code")),
                           ("/FI", str(root / "code/detail/value.h"), "/c")) == b"object"
    assert not seen[0].exists()


@pytest.mark.parametrize("flag", ["/Yupch.h", "-Ycpch.h", "/Fpx.pch", "@flags", "/Irelative", "/FIrelative.h"])
def test_unsupported_compiler_options_are_refused(variant_project, flag):
    root = variant_project
    source = root / "code/unit.cpp"
    with pytest.raises(ValueError):
        compile_overlay(root, source, {"code/unit.cpp": source.read_bytes()}, ("nonexistent-compiler",), (flag,))


@pytest.mark.parametrize("name", ["CL", "_CL_", "VC6_INCLUDE"])
def test_hidden_options_are_refused(variant_project, monkeypatch, name):
    root = variant_project
    monkeypatch.setenv(name, "hidden option")
    with pytest.raises(ValueError, match="explicit include/options"):
        compile_overlay(root, root / "code/unit.cpp", {"code/unit.cpp": b"candidate"}, ("nonexistent-compiler",), ())


def test_include_remapping_does_not_change_external_search_paths(variant_project):
    root = variant_project
    assert _variant_arguments(("/I/opt/compiler/include",), root, root / "replica", ("code",)) == ("/I/opt/compiler/include",)
    with pytest.raises(ValueError, match="outside the declared"):
        _variant_arguments(("/I" + str(root / "recovery"),), root, root / "replica", ("code",))


@pytest.mark.parametrize("damage", [None, "digest", "encoding", "version", "unknown-field"])
def test_local_protocol_checks_original_identity_before_dispatch(variant_project, monkeypatch, capsys, damage):
    from recovery.environment import adapter
    root = variant_project
    original = (root / "code/unit.cpp").read_bytes()
    request = {"schema_version": 4, "profile": "application", "source": "code/unit.cpp", "arguments": ["compiler"], "flags": ["/c"],
               "overrides": {"code/unit.cpp": {"sha256": hashlib.sha256(original).hexdigest(), "base64": base64.b64encode(b"candidate").decode()}}}
    if damage == "digest": request["overrides"]["code/unit.cpp"]["sha256"] = "0" * 64
    elif damage == "encoding": request["overrides"]["code/unit.cpp"]["base64"] = "!"
    elif damage == "version": request["schema_version"] = True
    elif damage == "unknown-field": request["claim"] = "pass"
    calls = []
    monkeypatch.setattr(adapter, "compile_overlay", lambda *args: (calls.append(args), b"object")[1])
    monkeypatch.chdir(root)
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(request)))
    assert adapter.main() == (0 if damage is None else 3)
    output = json.loads(capsys.readouterr().out)
    assert len(calls) == (1 if damage is None else 0)
    if damage is None:
        assert calls[0] == (root, root / "code/unit.cpp", {"code/unit.cpp": b"candidate"}, ("compiler",), ("/c",))
        assert output == {"object_base64": "b2JqZWN0"}
    else:
        assert set(output) == {"error"}


@pytest.mark.parametrize("path_sensitive", [False, True])
@pytest.mark.parametrize("debug_flags", [(), ("/Z7",)])
def test_real_vc6_replica_preserves_includes_lines_and_refuses_file_data_difference(variant_project, path_sensitive, debug_flags):
    root = variant_project
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo", "/I" + str(root / "support"), "/I" + str(root / "code"))
    flags = ("/c", "/O2", "/Gy", "/FI" + str(root / "code/detail/value.h"), *debug_flags)
    source = root / "code/unit.cpp"
    if not path_sensitive:
        source.write_bytes(source.read_bytes().replace(b"__FILE__", b'"stable"'))
    original = Coff(compile_object(source, command, flags))
    replica = Coff(compile_overlay(root, source, {"code/unit.cpp": source.read_bytes()}, command, flags))
    assert (object_signature(original) == object_signature(replica)) is not path_sensitive
    assert function_signature(original, "?value@@YAHXZ") == function_signature(replica, "?value@@YAHXZ")
    variant = Coff(compile_overlay(root, source, {"code/unit.cpp": source.read_bytes().replace(b"LOCAL_VALUE + GLOBAL_VALUE", b"GLOBAL_VALUE")}, command, flags))
    assert [(s.name, s.data) for s in original.sections] != [(s.name, s.data) for s in variant.sections]


def test_real_vc6_atomic_header_body_overlay_reaches_both_translation_units(variant_project):
    root = variant_project
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    (root / "code/model.h").write_text("struct Model { int value; };\n")
    (root / "code/first.cpp").write_text('#include "model.h"\nint first(Model* m) { return m->value; }\n')
    (root / "code/second.cpp").write_text('#include "model.h"\nint second(Model* m) { return m->value; }\n')
    originals = {name: (root / name).read_bytes() for name in ("code/model.h", "code/first.cpp", "code/second.cpp")}
    replacements = {"code/model.h": b"struct Model { int prefix; int value; };\n",
                    "code/first.cpp": originals["code/first.cpp"].replace(b"return m->value;", b"return m->prefix + m->value;")}
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy")
    for name, symbol in (("code/first.cpp", "?first@@YAHPAUModel@@@Z"), ("code/second.cpp", "?second@@YAHPAUModel@@@Z")):
        source = root / name
        baseline = Coff(compile_object(source, command, flags))
        replica = Coff(compile_overlay(root, source, originals, command, flags))
        assert object_signature(baseline) == object_signature(replica)
        proposed = Coff(compile_overlay(root, source, replacements, command, flags))
        assert function_signature(baseline, symbol) != function_signature(proposed, symbol)
    assert {name: (root / name).read_bytes() for name in originals} == originals
    assert not list((root / "build/recovery/variants").iterdir())


@pytest.mark.parametrize("debug_flags", [(), ("/Z7",)])
def test_real_vc6_unnamed_namespace_replica_keeps_code_and_external_edges(variant_project, debug_flags):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    (root / "code/helper.h").write_text("struct Guard { ~Guard(); };\ninline int identity(int x) { return x; }\n")
    source.write_text('#include "helper.h"\nint external(int);\n'
                      "namespace { int stored = 3; int helper(int x) { return x + stored; } }\n"
                      "int value(int x) { Guard g; return external(helper(identity(x))); }\n")
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Ob0", "/Gy", "/GX", *debug_flags)
    original = Coff(compile_object(source, command, flags))
    replica = Coff(compile_overlay(root, source, {}, command, flags))
    assert any("?helper@?%" in s.name for s in original.symbols.values())
    if debug_flags:
        assert sum(s.storage == 103 for s in original.symbols.values()) > 1
    assert object_signature(original) == object_signature(replica)
    # A different helper nonce/path is not evidence that a damage edit reached
    # the selected caller. Ordinary external edge changes must still count.
    symbol = "?value@@YAHH@Z"
    assert function_signature(original, symbol) == function_signature(replica, symbol)
    for before, after in ((b"stored = 3", b"stored = 4"), (b"external", b"other_external")):
        damaged = Coff(compile_overlay(root, source, {"code/unit.cpp": source.read_bytes().replace(before, after)}, command, flags))
        assert object_signature(original) != object_signature(damaged)
        if before == b"external":
            assert function_signature(original, symbol) != function_signature(damaged, symbol)


@pytest.mark.parametrize("debug_flags", [(), ("/Z7",)])
def test_real_vc6_preservation_tracks_exception_locations_not_numeric_names(variant_project, debug_flags):
    from match_recovery.coff_preservation import compare_outside_functions
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    header = root / "code/guard.h"
    header.write_text("struct Guard { ~Guard(); };\n")
    source.write_text('#include "guard.h"\nint external(int);\n'
                      "int value(int x) { Guard guard; return external(x); }\n")
    # A new unused inline definition can renumber later exception labels without
    # emitting code or changing the already present int(int) function type.
    candidate = header.read_bytes() + b"inline int unused(int argument) { return argument; }\n"
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/GX", *debug_flags)
    baseline = Coff(compile_object(source, command, flags))
    same = Coff(compile_overlay(root, source, {}, command, flags))
    assert object_signature(baseline) == object_signature(same)
    changed = Coff(compile_overlay(root, source, {"code/guard.h": candidate}, command, flags))
    def locals(obj):
        return {s.name for s in obj.symbols.values()
                if s.section in obj.associations and re.fullmatch(r"\$[LT][0-9]+", s.name)}
    assert locals(baseline) and locals(changed) and locals(baseline) != locals(changed)
    assert compare_outside_functions(baseline, changed, {}, {})["result"] == "pass"
    for damage in (source.read_bytes().replace(b"external(x)", b"external(x + 1)"),
                   source.read_bytes().replace(b"external", b"other_external"),
                   source.read_bytes() + b"int additional_storage = 7;\n"):
        damaged = Coff(compile_overlay(root, source,
            {"code/guard.h": candidate, "code/unit.cpp": damage}, command, flags))
        assert compare_outside_functions(baseline, damaged, {}, {})["result"] == "fail"


def test_real_vc6_same_code_and_storage_cannot_hide_changed_member_type(variant_project):
    from recovery.environment.wine import VC6_ROOT
    from match_recovery.codeview import class_layout
    from match_recovery.codeview_identity import type_identity
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_text("struct Model { int value; };\nModel instance;\n"
                      "int size() { return sizeof(Model); }\n")
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7")
    original = Coff(compile_object(source, command, flags))
    changed = Coff(compile_overlay(root, source,
        {"code/unit.cpp": source.read_bytes().replace(b"int value", b"float value")}, command, flags))
    assert class_layout(original, "Model")["size"] == class_layout(changed, "Model")["size"] == 4
    assert function_signature(original, "?size@@YAHXZ") == function_signature(changed, "?size@@YAHXZ")
    assert [(s.name, s.data) for s in original.sections if not s.name.startswith(".debug")]
    assert [(s.name, s.data) for s in original.sections if not s.name.startswith(".debug")] == [
        (s.name, s.data) for s in changed.sections if not s.name.startswith(".debug")]
    assert type_identity(original) != type_identity(changed)
    assert object_signature(original) != object_signature(changed)


def test_real_vc6_deck_follows_nonvirtual_to_virtual_method_binding(variant_project):
    from match_recovery.model import Origin
    from match_recovery.tasks import Task, TaskStore
    from match_recovery.variants import measure_variants
    from recovery.environment.wine import VC6_ROOT
    from test_binary import pe_fixture
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    # Independent synthetic image: mov eax,[ecx+4]; ret. The existing virtual
    # anchor gives C a vptr before its data member in both candidate spellings.
    image = pe_fixture(b"\x8b\x41\x04\xc3")
    pin = hashlib.sha256(image).hexdigest()
    (root / "image.exe").write_bytes(image)
    (root / "code/IMAGE_PIN").write_text("sha256 " + pin + "\n")
    source, header = root / "code/unit.cpp", root / "code/model.h"
    old_symbol, new_symbol = "?get@C@@QAEHXZ", "?get@C@@UAEHXZ"
    header.write_text("class C { public: virtual void anchor(); int value; int get(); };\n")
    source.write_text('#include "model.h"\n'
                      '// ORIGINAL: 0x00401000 ?get@C@@QAEHXZ 0x00401000-0x00401004\n'
                      'int C::get() { return value; }\n')
    path = root / "recovery/project.json"
    settings = json.loads(path.read_text())
    settings["source"]["symbol_format"] = "msvc"
    settings["measurement"].update(flag_sets=[["/c", "/O2", "/Gy", "/GR-"]], layout_flags=[])
    path.write_text(json.dumps(settings))
    task = Task("virtual-binding", "synthetic", "coordinator", "Test coupled virtual method proposal", "a" * 40,
                (Origin(pin, "pe:va", 0x401000),), ("code/unit.cpp", "code/model.h"), ("synthetic compiler control",))
    TaskStore(root).create(task)
    original = {name: (root / name).read_bytes() for name in task.editable_files}
    body = "int C::get() { return value; }"
    value = {"schema_version": 2, "source": "code/unit.cpp", "entity_id": task.subjects[0].entity_id,
             "files": {name: {"source_sha256": hashlib.sha256(data).hexdigest(),
                               "entity_id": task.subjects[0].entity_id,
                               "before": body if name.endswith(".cpp") else ["int value;", "int get();"],
                               "control": body.replace("return value", "return value + 1") if name.endswith(".cpp")
                               else ["int prefix; int value;", "int get();"]} for name, data in original.items()},
             "candidates": {name: {"code/unit.cpp": "// symbol " + new_symbol + "\n" + body,
                                    "code/model.h": spelling} for name, spelling in (
                                        ("virtual", ["int value;", "virtual int get();"]),
                                        ("stale", ["int value;", "int get();"]))}}
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    class LocalCompiler:
        def __call__(self, source, arguments, flags):
            return compile_object(source, command, flags)

        def overlay(self, source, contents, arguments, flags):
            return compile_overlay(root, source, contents, command, flags)

    report = measure_variants(root, task.task_id, root / "image.exe", value, compiler=LocalCompiler())
    assert report["controls"][0]["result"] == "pass", report["controls"]
    assert report["baseline"][0]["target"]["symbol"] == old_symbol
    assert report["baseline"][0]["target"]["result"] == "pass"
    virtual, stale = report["candidates"]
    assert virtual["target"]["symbol"] == stale["target"]["symbol"] == new_symbol
    assert virtual["target"]["result"] == "pass", virtual
    assert stale["target"]["result"] == "unknown", stale
    assert {name: (root / name).read_bytes() for name in original} == original
    assert not list((root / "build/recovery/variants").iterdir())


def test_real_vc6_historical_tree_excludes_new_include_shadow(variant_project):
    from match_recovery.source_replica import replica_snapshot
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_bytes(source.read_bytes().replace(b"__FILE__", b'"stable"'))
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo", "/I" + str(root / "code"), "/I" + str(root / "support"))
    flags = ("/c", "/O2", "/Gy")
    files = {name: data for name, data in replica_snapshot(root, ("code", "support")).items() if data is not None}
    prior = Coff(compile_object(source, command, flags))
    (root / "code/value.h").write_text("#define GLOBAL_VALUE 99\n")
    current = Coff(compile_object(source, command, flags))
    assert function_signature(prior, "?value@@YAHXZ") != function_signature(current, "?value@@YAHXZ")
    historical = Coff(compile_overlay(root, source, files, command, flags, tree_roots=("code", "support")))
    assert object_signature(prior) == object_signature(historical)
    assert (root / "code/value.h").read_text() == "#define GLOBAL_VALUE 99\n"
    assert not list((root / "build/recovery/variants").iterdir())


@pytest.mark.parametrize("damage", ["stale-header", "malformed-header", "symlink-header", "v3", "empty"])
def test_adapter_refuses_bad_overlay_before_compiling_any_file(variant_project, monkeypatch, capsys, damage):
    from recovery.environment import adapter
    root = variant_project
    names = ("code/unit.cpp", "code/detail/value.h")
    request = {"schema_version": 4, "profile": "application", "source": names[0], "arguments": ["compiler"], "flags": [],
               "overrides": {name: {"sha256": hashlib.sha256((root / name).read_bytes()).hexdigest(),
                                    "base64": base64.b64encode(b"candidate").decode()} for name in names}}
    if damage == "stale-header": request["overrides"][names[1]]["sha256"] = "0" * 64
    elif damage == "malformed-header": request["overrides"][names[1]]["base64"] = "!"
    elif damage == "symlink-header":
        (root / names[1]).unlink()
        (root / names[1]).symlink_to("../unit.cpp")
    elif damage == "v3": request["schema_version"] = 3
    else: request["overrides"] = {}
    def forbidden(*args):
        pytest.fail("invalid overlay reached compiler")
    monkeypatch.setattr(adapter, "compile_overlay", forbidden)
    monkeypatch.chdir(root)
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(request)))
    assert adapter.main() == 3
    assert set(json.loads(capsys.readouterr().out)) == {"error"}
