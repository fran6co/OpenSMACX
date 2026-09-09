"""Both dependency delivery modes use source-owned, reference-checked imports."""

import base64
from dataclasses import replace
import hashlib
import json
import struct
import sys

import pytest

from match_recovery.cli import main
from match_recovery import component_binary
from match_recovery.component_source import render_component
from match_recovery.components import ToolchainManifest
from match_recovery.import_source import parse_imports
from match_recovery.model import Delivery, Span
from test_binary import component_fixture, coff_fixture
from test_imports import DECLARATION, image_imports, with_second_import
from test_toolchain_components import archive


@pytest.fixture(params=["upstream", "toolchain"])
def dependency(tmp_path, request):
    root = tmp_path
    for directory in ("vendor", "code", "code/seeds", "recovery"):
        (root / directory).mkdir()
    (root / "vendor/f.c").write_text("void f() {}\n")
    obj = coff_fixture(b"\xff\x15\0\0\0\0\xc3", relocation=(2, 1, 6))
    return root, request.param, obj


def inputs(dependency, image):
    root, kind, obj = dependency
    manifest = component_fixture(image)
    manifest = replace(manifest, bindings=(replace(manifest.bindings[0], spans=(Span(0x401000, 0x401007),)),))
    raw = archive([("f.c/", obj)])
    if kind == "toolchain":
        manifest = ToolchainManifest(replace(manifest.component, delivery=Delivery.TOOLCHAIN),
                                     hashlib.sha256(raw).hexdigest(), "Synthetic fixture", manifest.bindings)
    return manifest, raw


def verify(dependency, image, text=DECLARATION):
    root, kind, obj = dependency
    manifest, raw = inputs(dependency, image)
    imports = parse_imports(text, "code/system.h")
    if kind == "toolchain":
        return component_binary.verify_toolchain(manifest, image, raw, import_bindings=imports)
    return component_binary.verify_component(root, manifest, image, lambda *_: obj, import_bindings=imports)


def test_component_imports_need_explicit_identity_and_still_check_operand(dependency):
    image = image_imports()
    good = verify(dependency, image)
    assert good["result"] == good["functions"][0]["references"] == "pass"
    assert good["import_bindings"][0]["identity"] == "pass"
    assert good["import_bindings"][0]["address"] == "0x00401160"
    assert good["import_bindings"][0]["link_selection"] == "unknown"
    assert verify(dependency, image, "")["result"] == "unknown"
    assert verify(dependency, image, DECLARATION.replace("DEMO.dll", "WRONG.dll"))["result"] == "unknown"
    wrong = bytearray(image)
    struct.pack_into("<I", wrong, 514, 0x401164)
    assert verify(dependency, bytes(wrong))["functions"][0]["references"] == "fail"


def test_component_import_identity_does_not_select_ambiguous_or_other_dll_slots(dependency):
    duplicate = with_second_import(image_imports(), "DEMO.dll")
    report = verify(dependency, duplicate)
    assert report["result"] == report["import_bindings"][0]["identity"] == "unknown"
    second = with_second_import(image_imports(), "OTHER.dll")
    assert verify(dependency, second)["result"] == "pass"
    assert verify(dependency, second, DECLARATION.replace("DEMO.dll", "OTHER.dll"))["result"] == "fail"


@pytest.mark.parametrize("damage", ["defined", "weak", "common", "static"])
def test_component_imports_cannot_supply_linker_selected_or_defined_symbols(dependency, damage):
    root, kind, raw = dependency
    obj = bytearray(raw)
    at = struct.unpack_from("<I", obj, 8)[0] + 18  # Second synthetic symbol: _g.
    if damage == "defined":
        struct.pack_into("<h", obj, at + 12, 1)
    elif damage == "common":
        struct.pack_into("<I", obj, at + 8, 4)
    else:
        obj[at + 16] = 105 if damage == "weak" else 3
    report = verify((root, kind, bytes(obj)), image_imports())
    assert report["result"] != "pass"
    assert report["import_bindings"][0]["identity"] == "unknown"


def test_component_function_import_namespace_conflict_refuses(dependency):
    with pytest.raises(ValueError, match="both function and import"):
        verify(dependency, image_imports(), DECLARATION.replace("_g", "_f"))
    with pytest.raises(ValueError, match="duplicate"):
        verify(dependency, image_imports(), DECLARATION * 2)


def prepare_cli(dependency):
    root, kind, obj = dependency
    image = image_imports()
    manifest, raw = inputs(dependency, image)
    (root / "code/runtime.recovery.h").write_text(render_component(manifest))
    (root / "code/system.h").write_text(DECLARATION)
    (root / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    (root / "original.exe").write_bytes(image)
    (root / "runtime.lib").write_bytes(raw)
    (root / "recovery/compiler.py").write_text(
        "import json,sys\njson.load(sys.stdin)\nprint(" +
        repr(json.dumps({"object_base64": base64.b64encode(obj).decode()})) + ")\n")
    (root / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "seed_roots": ["code/seeds"]},
        "compiler": {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 5},
                     "inputs": ["recovery/compiler.py"], "environment": []}}))
    command = ["components", "check-binary", "fixture", "--root", str(root), "--image", "original.exe"]
    return command + (["--archive", "runtime.lib"] if kind == "toolchain" else [])


def test_component_cli_uses_current_product_imports_not_seeds(dependency, capsys):
    root, _, _ = dependency
    command = prepare_cli(dependency)
    assert main(command) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["import_bindings"][0]["source"] == "code/system.h"
    (root / "code/system.h").rename(root / "code/seeds/system.h")
    assert main(command) == 3
    assert json.loads(capsys.readouterr().out)["unresolved_symbols"] == ["_g"]


@pytest.mark.parametrize("damage", ["change", "remove", "add"])
def test_component_import_source_epoch_includes_changed_removed_and_new_files(dependency, capsys, monkeypatch, damage):
    root, kind, _ = dependency
    command = prepare_cli(dependency)
    name = "verify_toolchain" if kind == "toolchain" else "verify_component"
    original = getattr(component_binary, name)
    def mutate(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report["result"] == "pass"
        path = root / "code/system.h"
        if damage == "remove":
            path.unlink()
        else:
            if damage == "add":
                path = root / "code/new-import.h"
            path.write_text(DECLARATION.replace("Paint", "Other"))
        return report
    monkeypatch.setattr(component_binary, name, mutate)
    assert main(command) == 3
    assert "changed" in capsys.readouterr().err
