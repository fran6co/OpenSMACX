"""VC6 source-override mechanics, separate from target-independent recovery."""

import json
import base64
import hashlib
import io
import subprocess
from pathlib import Path
import sys

import pytest

from recovery.environment.compile import _variant_arguments, compile_variant, compile_object
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
    assert compile_variant(root, source, candidate,
                           ("compile", "-I", str(root / "support"), "/I" + str(root / "code")),
                           ("/FI", str(root / "code/detail/value.h"), "/c")) == b"object"
    assert not seen[0].exists()


@pytest.mark.parametrize("flag", ["/Yupch.h", "-Ycpch.h", "/Fpx.pch", "@flags", "/Irelative", "/FIrelative.h"])
def test_unsupported_compiler_options_are_refused(variant_project, flag):
    root = variant_project
    source = root / "code/unit.cpp"
    with pytest.raises(ValueError):
        compile_variant(root, source, source.read_bytes(), ("nonexistent-compiler",), (flag,))


@pytest.mark.parametrize("name", ["CL", "_CL_", "VC6_INCLUDE"])
def test_hidden_options_are_refused(variant_project, monkeypatch, name):
    root = variant_project
    monkeypatch.setenv(name, "hidden option")
    with pytest.raises(ValueError, match="explicit include/options"):
        compile_variant(root, root / "code/unit.cpp", b"candidate", ("nonexistent-compiler",), ())


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
    request = {"schema_version": 3, "profile": "application", "source": "code/unit.cpp", "arguments": ["compiler"], "flags": ["/c"],
               "source_sha256": hashlib.sha256(original).hexdigest(), "source_base64": base64.b64encode(b"candidate").decode()}
    if damage == "digest": request["source_sha256"] = "0" * 64
    elif damage == "encoding": request["source_base64"] = "!"
    elif damage == "version": request["schema_version"] = True
    elif damage == "unknown-field": request["claim"] = "pass"
    calls = []
    monkeypatch.setattr(adapter, "compile_variant", lambda *args: (calls.append(args), b"object")[1])
    monkeypatch.chdir(root)
    monkeypatch.setattr(sys, "stdin", io.StringIO(json.dumps(request)))
    assert adapter.main() == (0 if damage is None else 3)
    output = json.loads(capsys.readouterr().out)
    assert len(calls) == (1 if damage is None else 0)
    if damage is None:
        assert calls[0] == (root, root / "code/unit.cpp", b"candidate", ("compiler",), ("/c",))
        assert output == {"object_base64": "b2JqZWN0"}
    else:
        assert set(output) == {"error"}


@pytest.mark.parametrize("path_sensitive", [False, True])
def test_real_vc6_replica_preserves_includes_lines_and_refuses_file_data_difference(variant_project, path_sensitive):
    root = variant_project
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo", "/I" + str(root / "support"), "/I" + str(root / "code"))
    flags = ("/c", "/O2", "/Gy", "/FI" + str(root / "code/detail/value.h"))
    source = root / "code/unit.cpp"
    if not path_sensitive:
        source.write_bytes(source.read_bytes().replace(b"__FILE__", b'"stable"'))
    original = Coff(compile_object(source, command, flags))
    replica = Coff(compile_variant(root, source, source.read_bytes(), command, flags))
    assert (object_signature(original) == object_signature(replica)) is not path_sensitive
    assert function_signature(original, "?value@@YAHXZ") == function_signature(replica, "?value@@YAHXZ")
    variant = Coff(compile_variant(root, source, source.read_bytes().replace(b"LOCAL_VALUE + GLOBAL_VALUE", b"GLOBAL_VALUE"), command, flags))
    assert [(s.name, s.data) for s in original.sections] != [(s.name, s.data) for s in variant.sections]
