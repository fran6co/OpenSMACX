"""Installed-style checks for an unregistered project and local compiler profile."""

import base64
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

import pytest

from match_recovery.cli import main
from match_recovery.measurement import Measurement
from match_recovery.model import Origin
from match_recovery.object_source import read_objects
from match_recovery.tasks import Task, TaskStore
from test_binary import coff_fixture, pe_fixture
from test_codeview import debug_types


def combined_object(types=None):
    obj = coff_fixture()
    types = debug_types() if types is None else types
    raw, symbols = obj[60:76], obj[76:]
    text = bytearray(obj[20:60])
    struct.pack_into("<I", text, 20, 100)
    debug = struct.pack("<8sIIIIIIHHI", b".debug$T", 0, 0, len(types), 116, 0, 0, 0, 0, 0x40000040)
    return struct.pack("<HHIIIHH", 0x14C, 2, 0, 116 + len(types), 2, 0, 0) + text + debug + raw + types + symbols


@pytest.fixture
def project(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    image = pe_fixture()
    pin = hashlib.sha256(image).hexdigest()
    (tmp_path / "original.exe").write_bytes(image)
    (tmp_path / "code/PIN").write_text("sha256 " + pin + "\n")
    (tmp_path / "code/unit.cc").write_text('// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nvoid f() {}\n')
    (tmp_path / "code/unit.hpp").write_text('// RECOVERY_MEMBER: v1 Widget slot_ 0x4 0x00401000\n')
    compiler = tmp_path / "recovery/compile.py"
    compiler.write_text('import json, sys\nr = json.load(sys.stdin)\n'
                        'assert r["schema_version"] == 2 and r["profile"] == "custom-profile"\n'
                        'assert r["source"] == "code/unit.cc" and r["arguments"] == []\n'
                        'assert r["flags"] in (["-trial"], ["-trial", "-types"])\n'
                        'print(' + repr(json.dumps({"object_base64": base64.b64encode(combined_object()).decode()})) + ')\n')
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
        "compiler": {"command": {"argv": [sys.executable, "recovery/compile.py"], "timeout": 5},
                     "inputs": ["recovery/compile.py"], "environment": ["CUSTOM_ABI"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "custom-profile", "flag_sets": [["-trial"]],
                        "layout_flags": ["-types"], "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}))
    TaskStore(tmp_path).create(Task("owner", "unregistered-game", "coordinator", "Recover ownership", "b" * 40,
                                  (Origin(pin, "pe:va", 0x401000),), ("code/unit.cc", "code/unit.hpp"), ("synthetic",)))
    return tmp_path


@pytest.mark.parametrize("action", ["check-binary", "check-layout", "check"])
def test_native_checks_need_no_project_dispatcher(project, capsys, action):
    arguments = ["tasks", action, "owner", "--root", str(project), "--summary"]
    if action != "check-layout":
        arguments += ["--image", "original.exe"]
    assert main(arguments) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass" and report["scope_complete"] is False
    assert report["structure_verification"] == "unknown"
    if action == "check":
        assert report["compilation"] == {"attempts": 1, "reused_objects": 1, "debug": ["-types"], "persistent_cache": False}
        assert not report["ready_for_collection"]


def test_strict_joint_check_runs_without_site_packages(project):
    import match_recovery
    package = Path(match_recovery.__file__).resolve().parent.parent
    script = 'import sys; sys.path.insert(0, sys.argv[1]); from match_recovery.cli import main; raise SystemExit(main(sys.argv[2:]))'
    result = subprocess.run([sys.executable, "-I", "-S", "-c", script, str(package), "tasks", "check", "owner",
                             "--root", str(project), "--image", "original.exe", "--summary"],
                            cwd=project, capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["checks"]["binary"]["result"] == "pass"


@pytest.mark.parametrize("damage", [{"backend": "unknown"}, {"flag_sets": []}, {"flag_sets": ["-trial"]},
                                   {"unit_extensions": []}, {"unit_extensions": ["*.cc"]},
                                   {"input_roots": ["../outside"]}, {"layout_flags": "-types"}])
def test_invalid_measurement_configuration_is_refused(project, damage):
    path = project / "recovery/project.json"
    data = json.loads(path.read_text())
    data["measurement"].update(damage)
    path.write_text(json.dumps(data))
    with pytest.raises(ValueError):
        Measurement.read(project)


def test_configured_pch_flags_cannot_supply_standalone_layout_proof(project, capsys):
    path = project / "recovery/project.json"
    data = json.loads(path.read_text())
    data["measurement"]["layout_flags"] = ["/Yuheader.hpp"]
    path.write_text(json.dumps(data))
    assert main(["tasks", "check-layout", "owner", "--root", str(project)]) == 3
    report = json.loads(capsys.readouterr().out)
    assert report["compile_attempts"] == 0
    assert "standalone" in report["members"][0]["reason"]


def test_object_annotations_use_configured_roots_suffixes_and_address_space(project):
    path = project / "recovery/project.json"
    data = json.loads(path.read_text())
    data["source"]["address_space"] = "custom:64"
    path.write_text(json.dumps(data))
    (project / "code/unit.cc").write_text('// RECOVERY_OBJECT: v1 0x100000000 _global\n')
    binding, = read_objects(project, "a" * 64)
    assert binding.origin.address_space == "custom:64" and binding.origin.address == 0x100000000


def test_standalone_binary_check_rejects_an_image_changed_during_compilation(project):
    from match_recovery.application_binary import verify_task
    def changed(*args):
        (project / "original.exe").write_bytes(pe_fixture(b"\x90"))
        return combined_object()
    with pytest.raises(ValueError, match="changed during measurement"):
        verify_task(project, "owner", project / "original.exe", compiler=changed)


def test_non_unit_source_cannot_be_compiled_as_a_guessed_translation_unit(project, capsys):
    (project / "code/unit.cc").rename(project / "code/definition.hpp")
    assert main(["tasks", "check-binary", "owner", "--root", str(project), "--image", "original.exe"]) == 3
    report = json.loads(capsys.readouterr().out)
    assert report["compile_attempts"] == 0
    assert "configured translation unit" in report["functions"][0]["reason"]


@pytest.mark.parametrize("damage", [{"environment": [[]]}, {"inputs": [None]}, {"command": None}])
def test_invalid_compiler_policy_is_a_clear_measurement_error(project, damage):
    path = project / "recovery/project.json"
    data = json.loads(path.read_text())
    data["compiler"].update(damage)
    path.write_text(json.dumps(data))
    with pytest.raises(ValueError):
        Measurement.read(project)
