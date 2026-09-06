"""Dependency commands are native, with only compiler interaction supplied locally."""

import base64
from dataclasses import replace
import hashlib
import io
import json
import sys
import tarfile

import pytest

from match_recovery.cli import main
from match_recovery.component_source import render_component
from match_recovery.component_commands import component_snapshot
from test_binary import pe_fixture, coff_fixture, component_fixture


@pytest.fixture
def component_project(tmp_path):
    for name in ("recovery", "code", "vendor", "hints"):
        (tmp_path / name).mkdir()
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    (tmp_path / "original.exe").write_bytes(image)
    (tmp_path / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    raw = b"int f() { return 1; }\n"
    (tmp_path / "vendor/f.c").write_bytes(raw)
    archive = tmp_path / "upstream.tar"
    with tarfile.open(archive, "w") as stream:
        member = tarfile.TarInfo("f.c")
        member.size = len(raw)
        stream.addfile(member, io.BytesIO(raw))
    manifest = replace(component_fixture(image), archive_sha256=hashlib.sha256(archive.read_bytes()).hexdigest())
    (tmp_path / "code/vendor.recovery.h").write_text(render_component(manifest))
    obj = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20))
    (tmp_path / "recovery/compiler.py").write_text(
        'import json, sys\nr = json.load(sys.stdin)\n'
        'assert r == {"schema_version": 1, "source": "vendor/f.c", "arguments": [], "flags": ["/Gy"]}\n'
        'print(' + repr(json.dumps({"object_base64": base64.b64encode(obj).decode()})) + ')\n')
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code", "hints"], "seed_roots": ["hints"]},
        "compiler": {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 5},
                     "inputs": ["recovery/compiler.py"], "environment": []}}))
    return tmp_path


def test_native_dependency_commands_do_not_require_function_annotations_or_adapter(component_project, capsys):
    root = component_project
    assert main(["components", "list", "--root", str(root)]) == 0
    assert json.loads(capsys.readouterr().out)[0]["bindings"] == 1
    assert main(["components", "check-source", "fixture", "--archive", "upstream.tar", "--root", str(root)]) == 0
    source = json.loads(capsys.readouterr().out)
    assert source["binary_verification"] == "unknown" and source["application_recoveries"] == 0
    assert main(["components", "check-binary", "fixture", "--image", "original.exe", "--root", str(root)]) == 0
    binary = json.loads(capsys.readouterr().out)
    assert binary["compile_attempts"] == 1 and binary["result"] == "pass"
    assert binary["application_recoveries"] == 0 and binary["structure_verification"] == "unknown"


def test_seed_manifests_do_not_override_product_dependency_ownership(component_project):
    root = component_project
    (root / "hints/duplicate.recovery.h").write_text((root / "code/vendor.recovery.h").read_text())
    assert len(component_snapshot(root)[0]) == 1


def test_component_pin_mismatch_is_refused(component_project, capsys):
    root = component_project
    (root / "code/PIN").write_text("sha256 " + "b" * 64 + "\n")
    assert main(["components", "list", "--root", str(root)]) == 3
    assert "image/address space" in capsys.readouterr().err


@pytest.mark.parametrize("target", ["code/vendor.recovery.h", "vendor/f.c", "original.exe", "recovery/project.json"])
def test_component_input_mutation_during_compilation_is_not_reported_as_pass(component_project, capsys, target):
    root = component_project
    adapter = root / "recovery/compiler.py"
    adapter.write_text('from pathlib import Path\np = Path(' + repr(target) + ')\np.write_bytes(p.read_bytes() + b"\\n")\n' + adapter.read_text())
    assert main(["components", "check-binary", "fixture", "--image", "original.exe", "--root", str(root)]) == 3
    assert "changed" in capsys.readouterr().err


def test_missing_compiler_is_unknown_not_a_dependency_pass(component_project, capsys):
    root = component_project
    path = root / "recovery/project.json"
    value = json.loads(path.read_text())
    del value["compiler"]
    path.write_text(json.dumps(value))
    assert main(["components", "check-binary", "fixture", "--image", "original.exe", "--root", str(root)]) == 3
    assert "compiler configuration" in capsys.readouterr().err


@pytest.mark.parametrize("target", ["vendor/f.c", "upstream.tar", "code/vendor.recovery.h"])
def test_source_integrity_report_is_refused_if_inputs_change(component_project, capsys, monkeypatch, target):
    from match_recovery import component_commands
    root = component_project
    check = component_commands.source_integrity
    def changing(*args):
        report = check(*args)
        assert report["result"] == "pass"
        path = root / target
        path.write_bytes(path.read_bytes() + b"\n")
        return report
    monkeypatch.setattr(component_commands, "source_integrity", changing)
    assert main(["components", "check-source", "fixture", "--archive", "upstream.tar", "--root", str(root)]) == 3
    assert "changed during verification" in capsys.readouterr().err
