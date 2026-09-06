"""Real command-protocol controls without a compiler or optional dependencies."""

import json
import base64
import hashlib
import sys

import pytest

from match_recovery.compiler import Compiler


@pytest.fixture
def compiler_project(tmp_path):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "code").mkdir()
    (tmp_path / "code/unit.cc").write_text("synthetic source\n")
    adapter = tmp_path / "recovery/compiler.py"
    adapter.write_text('import json, sys\nr = json.load(sys.stdin)\n'
                       'assert r == {"schema_version": 2, "profile": "default", "source": "code/unit.cc", "arguments": ["compile"], "flags": ["-O2"]}\n'
                       'print(json.dumps({"object_base64": "b2JqZWN0"}))\n')
    configuration = {"schema_version": 1, "compiler": {
        "command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 5},
        "inputs": ["recovery/compiler.py"], "environment": ["FIXTURE_COMPILER_OPTION"]}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(configuration))
    return tmp_path


def compile_fixture(root):
    return Compiler(root)(root / "code/unit.cc", ("compile",), ("-O2",))


def test_configured_compiler_exchanges_object_bytes_not_verdicts(compiler_project):
    assert compile_fixture(compiler_project) == b"object"
    assert not (compiler_project / "build").exists()


@pytest.mark.parametrize("response", [{"object_base64": ""}, {"object_base64": "not base64!"},
                                    {"object_base64": 12}, {"result": "pass"},
                                    {"object_base64": "b2JqZWN0", "verified": True}])
def test_success_without_a_valid_object_is_refused(compiler_project, response):
    (compiler_project / "recovery/compiler.py").write_text("print(" + repr(json.dumps(response)) + ")\n")
    with pytest.raises(ValueError):
        compile_fixture(compiler_project)


@pytest.mark.parametrize("code", [1, 3, 17])
def test_failure_never_supplies_object_evidence(compiler_project, code):
    (compiler_project / "recovery/compiler.py").write_text('print(\'{"object_base64":"b2JqZWN0"}\')\nraise SystemExit(' + str(code) + ')\n')
    with pytest.raises(ValueError):
        compile_fixture(compiler_project)


def test_compiler_timeout_is_unmeasurable(compiler_project):
    path = compiler_project / "recovery/project.json"
    value = json.loads(path.read_text())
    value["compiler"]["command"]["timeout"] = 1
    path.write_text(json.dumps(value))
    (compiler_project / "recovery/compiler.py").write_text("import time\ntime.sleep(30)\n")
    with pytest.raises(ValueError, match="did not complete"):
        compile_fixture(compiler_project)


@pytest.mark.parametrize("damage", ["implementation", "configuration", "environment"])
def test_changed_compiler_inputs_are_refused_before_reuse(compiler_project, monkeypatch, damage):
    root = compiler_project
    compiler = Compiler(root)
    if damage == "implementation":
        (root / "recovery/compiler.py").write_text("different implementation")
    elif damage == "configuration":
        path = root / "recovery/project.json"
        value = json.loads(path.read_text())
        value["compiler"]["inputs"] = []
        path.write_text(json.dumps(value))
    else:
        monkeypatch.setenv("FIXTURE_COMPILER_OPTION", "changed")
    with pytest.raises(ValueError, match="changed"):
        compiler(root / "code/unit.cc", ("compile",), ("-O2",))


def test_compiler_self_modification_invalidates_its_object(compiler_project):
    (compiler_project / "recovery/compiler.py").write_text(
        'from pathlib import Path\nPath(__file__).write_text("changed")\nprint(\'{"object_base64":"b2JqZWN0"}\')\n')
    with pytest.raises(ValueError, match="changed during"):
        compile_fixture(compiler_project)


def test_environment_values_are_not_exposed_in_diagnostic_snapshots(compiler_project, monkeypatch):
    monkeypatch.setenv("FIXTURE_COMPILER_OPTION", "sensitive-fixture-value")
    assert "sensitive-fixture-value" not in json.dumps(Compiler(compiler_project).initial)


def test_compiler_refuses_source_symlinks_outside_project(compiler_project, tmp_path_factory):
    outside = tmp_path_factory.mktemp("outside-compiler") / "unit.cc"
    outside.write_text("synthetic outside source")
    (compiler_project / "code/link.cc").symlink_to(outside)
    with pytest.raises(ValueError, match="inside the project"):
        Compiler(compiler_project)(compiler_project / "code/link.cc", (), ())


@pytest.mark.parametrize("version", [True, 2])
def test_compiler_rejects_unsupported_configuration_schema(compiler_project, version):
    path = compiler_project / "recovery/project.json"
    value = json.loads(path.read_text())
    value["schema_version"] = version
    path.write_text(json.dumps(value))
    with pytest.raises(ValueError, match="configuration version"):
        Compiler(compiler_project)


def test_source_override_protocol_keeps_original_identity_and_bytes(compiler_project):
    root = compiler_project
    source = root / "code/unit.cc"
    original, candidate = source.read_bytes(), b"int example() { return 7; }\n\xff"
    request = {"schema_version": 3, "profile": "application", "source": "code/unit.cc",
               "arguments": [], "flags": ["-O2"], "source_sha256": hashlib.sha256(original).hexdigest(),
               "source_base64": base64.b64encode(candidate).decode()}
    (root / "recovery/compiler.py").write_text(
        'import json, sys\nfrom pathlib import Path\nr = json.load(sys.stdin)\n'
        f'assert r == {request!r}\nassert Path(r["source"]).read_bytes() == {original!r}\n'
        'print(json.dumps({"object_base64": "b2JqZWN0"}))\n')
    before = {p.relative_to(root): p.read_bytes() for p in root.rglob("*") if p.is_file()}
    assert Compiler(root, profile="application").variant(source, candidate, (), ("-O2",)) == b"object"
    assert {p.relative_to(root): p.read_bytes() for p in root.rglob("*") if p.is_file()} == before


@pytest.mark.parametrize("candidate", ["not bytes", bytearray(b"not immutable"), None])
def test_source_override_requires_bytes(compiler_project, candidate):
    with pytest.raises(ValueError, match="must be bytes"):
        Compiler(compiler_project).variant(compiler_project / "code/unit.cc", candidate, (), ())


@pytest.mark.parametrize("variant", [False, True])
def test_compiler_cannot_edit_original_source_and_return_evidence(compiler_project, variant):
    root = compiler_project
    (root / "recovery/compiler.py").write_text(
        'from pathlib import Path\nPath("code/unit.cc").write_bytes(b"concurrent change")\n'
        'print(\'{"object_base64":"b2JqZWN0"}\')\n')
    compiler = Compiler(root)
    with pytest.raises(ValueError, match="source changed during"):
        if variant:
            compiler.variant(root / "code/unit.cc", b"candidate", (), ())
        else:
            compiler(root / "code/unit.cc", (), ())
    # Never restore over the newer write, even if it came from a bad command.
    assert (root / "code/unit.cc").read_bytes() == b"concurrent change"


def test_old_compiler_protocol_does_not_silently_score_original_as_candidate(compiler_project):
    with pytest.raises(ValueError):
        Compiler(compiler_project).variant(compiler_project / "code/unit.cc", b"candidate", ("compile",), ("-O2",))
