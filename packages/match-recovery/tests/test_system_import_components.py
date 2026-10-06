from dataclasses import replace
import hashlib
import struct
import subprocess
import sys
from pathlib import Path

import pytest

from match_recovery.component_original_routing import (component_claim_result,
    component_function_observations, exact_component_record_bindings)
from match_recovery.component_source import parse_component, render_component
from match_recovery.components import SystemImportBinding, SystemImportManifest
from match_recovery.import_source import parse_imports
from match_recovery.model import Component, Delivery, Origin, Ownership, Span
from match_recovery.match_check import check_matches
from match_recovery.source import read_text, write
from match_recovery.source_inventory import source_snapshot
from match_recovery.source_measure import measure_source
from match_recovery.recording import record
from test_coff_inventory import imported
from test_imports import image_imports
from test_toolchain_components import archive


def fixture(*, import_record=None, thunk=b"\xff\x25\x60\x11\x40\0", relocation=0x3002,
            extra_relocation=0, member="DEMO.dll", occurrence=1, archive_override=None):
    raw_image = bytearray(image_imports())
    struct.pack_into("<I", raw_image, 0x178 + 36, 0x60000020)
    raw_image[512:518] = thunk
    # image_imports() has a relocation block for the call operand at +2.
    struct.pack_into("<H", raw_image, 0x308, relocation)
    struct.pack_into("<H", raw_image, 0x30A, extra_relocation)
    image = bytes(raw_image)
    record = imported() if import_record is None else import_record
    raw_archive = archive([(member + "/", imported("_Other@4")), (member + "/", record)])
    if archive_override is not None:
        raw_archive = archive_override
    pin = hashlib.sha256(image).hexdigest()
    binding = SystemImportBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401006),),
          "OriginalTime", member, occurrence, "__imp__Paint@4", "exact original thunk and SDK record")
    component = Component("demo-system", Ownership.SYSTEM, Delivery.SYSTEM_IMPORT,
                          ("Pinned PE import table", "Pinned SDK import library"), "SDK test")
    manifest = SystemImportManifest(component, hashlib.sha256(raw_archive).hexdigest(),
                                    "SDK archive license evidence", (binding,))
    declaration = parse_imports("// RECOVERY_IMPORT: v1 __imp__Paint@4 DEMO.dll name:Paint\n", "src/imports.h")
    return manifest, image, raw_archive, declaration


def test_component_roundtrip_and_physical_duplicate_member_selection():
    manifest, image, raw, declaration = fixture(occurrence=1)
    text = render_component(manifest)
    assert parse_component(text) == manifest
    report = manifest.verify(image, raw, declaration)
    assert report["result"] == "pass"
    row, = report["functions"]
    assert (row["bytes"], row["boundary"], row["references"]) == ("pass", "pass", "pass")
    assert row["import"] == {"module": "DEMO.dll", "name": "Paint", "ordinal": None,
                              "address": "0x00401160"}
    assert "iat" not in row and "selected" not in row


@pytest.mark.parametrize("damage", ["wrong_dll", "wrong_name", "wrong_ordinal", "wrong_occurrence", "wrong_member",
                                     "archive_pin", "extra_relocation", "missing_relocation",
                                     "wrong_relocation", "thunk_bytes", "data_import"])
def test_damage_never_becomes_a_system_import_pass(damage):
    record = imported()
    if damage == "wrong_dll": record = imported(module="OTHER.dll")
    if damage == "wrong_name": record = imported("_Other@4")
    if damage == "wrong_ordinal": record = imported(naming=0)
    kwargs = {}
    if damage in ("wrong_dll", "wrong_name", "wrong_ordinal", "data_import"):
        kwargs["import_record"] = imported(kind=1) if damage == "data_import" else record
    manifest, image, raw, declarations = fixture(**kwargs)
    if damage == "wrong_occurrence":
        manifest = replace(manifest, bindings=(replace(manifest.bindings[0], occurrence=0),))
    elif damage == "wrong_member":
        manifest = replace(manifest, bindings=(replace(manifest.bindings[0], unit="NO.dll"),))
    elif damage == "archive_pin":
        manifest = replace(manifest, archive_sha256="0" * 64)
    elif damage in ("extra_relocation", "missing_relocation", "wrong_relocation"):
        relocation = {"extra_relocation": 0x3002,
                      "missing_relocation": 0,
                      "wrong_relocation": 0x3001}[damage]
        manifest, image, raw, declarations = fixture(relocation=relocation,
            extra_relocation=0x3000 if damage == "extra_relocation" else 0)
    elif damage == "thunk_bytes":
        manifest, image, raw, declarations = fixture(thunk=b"\xff\x15\x60\x11\x40\0")
    report = manifest.verify(image, raw, declarations)
    assert report["result"] != "pass"
    assert report["functions"][0]["bytes"] != "pass" or report["functions"][0]["references"] != "pass"


def test_ambiguous_loader_export_and_wrong_manifest_classification_refuse():
    manifest, image, raw, declarations = fixture()
    dup = bytearray(image)
    # Duplicate descriptor + matching loader identity is a malformed/ambiguous import population.
    struct.pack_into("<IIIII", dup, 0x334, 0x1150, 0, 0, 0x1180, 0x1160)
    with pytest.raises(ValueError):
        manifest.verify(bytes(dup), raw, declarations)
    with pytest.raises(ValueError, match="system/system-import"):
        SystemImportManifest(Component("bad", Ownership.SYSTEM, Delivery.TOOLCHAIN, ("evidence",)),
                             manifest.archive_sha256, "license", manifest.bindings)


def test_short_import_type_and_archive_digest_are_independently_required():
    manifest, image, raw, declarations = fixture(import_record=imported(kind=1))
    assert manifest.verify(image, raw, declarations)["result"] != "pass"
    manifest, image, raw, declarations = fixture()
    altered_archive = raw + b"\n"
    report = manifest.verify(image, altered_archive, declarations)
    assert report["result"] != "pass"


@pytest.mark.parametrize("field,value", [("module", "OTHER.dll"), ("name", "Other"),
                                          ("ordinal", 1), ("name", None)])
def test_source_recovery_import_identity_is_independent_and_exact(field, value):
    manifest, image, raw, declarations = fixture()
    damaged = replace(declarations[0], **{field: value,
                      "ordinal": value if field == "ordinal" else declarations[0].ordinal,
                      "name": value if field == "name" else declarations[0].name})
    result = manifest.verify(image, raw, (damaged,))
    assert result["result"] != "pass"


def test_original_marker_routes_through_component_backend_without_product_compile(tmp_path):
    manifest, image, raw, declarations = fixture()
    for directory in ("src", "original", "recovery"):
        (tmp_path / directory).mkdir()
    (tmp_path / "original/input.exe").write_bytes(image)
    (tmp_path / "original/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    marker = "// ORIGINAL: 0x00401000 _OriginalTime 0x00401000-0x00401006 BYTE_EXACT\n"
    header = write(marker, read_text(marker, "src/system.recovery.h"))
    header += "// RECOVERY_IMPORT: v1 __imp__Paint@4 DEMO.dll name:Paint\n"
    header += render_component(manifest)
    (tmp_path / "src/system.recovery.h").write_text(header)
    (tmp_path / "system.lib").write_bytes(raw)
    (tmp_path / "build").mkdir()
    (tmp_path / "build/commands.json").write_text(__import__("json").dumps([
        {"directory": str(tmp_path), "file": "src/family.cpp",
         "arguments": ["compiler", "-c", "src/family.cpp"]}]))
    (tmp_path / "recovery/project.json").write_text(__import__("json").dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "pe:va",
                   "roots": ["src"], "extensions": [".h", ".cpp"], "seed_roots": [],
                   "include_paths": ["src"]},
        "components": {"system_import_archives": {"demo-system": "system.lib"}},
        "mappings": {"compile_commands": "build/commands.json", "forwarder_macros": []},
        "compiler": {"command": {"argv": [sys.executable, "-c", "raise SystemExit(99)"], "timeout": 5},
                     "inputs": [], "environment": []},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "fixture",
                        "flag_sets": [[]], "layout_flags": [], "unit_extensions": [".cpp"],
                        "inputs": [], "input_roots": []},
    }))
    (tmp_path / "src/family.cpp").write_text("void unused() {}\n")
    (tmp_path / ".gitignore").write_text("build/recovery/publications/\n")
    subprocess.run(["git", "init", "-b", "main"], cwd=tmp_path, check=True, capture_output=True)
    subprocess.run(["git", "config", "user.name", "System import control"], cwd=tmp_path, check=True)
    subprocess.run(["git", "config", "user.email", "system-import@example.test"], cwd=tmp_path, check=True)
    subprocess.run(["git", "add", "."], cwd=tmp_path, check=True)
    subprocess.run(["git", "commit", "-m", "System import fixture"], cwd=tmp_path, check=True,
                   capture_output=True)
    baseline = subprocess.run(["git", "rev-parse", "HEAD"], cwd=tmp_path, check=True,
                              capture_output=True, text=True).stdout.strip()
    project_path = tmp_path / "recovery/project.json"
    project_config = __import__("json").loads(project_path.read_text())
    project_config["source_history"] = {"baseline_revision": baseline,
                                         "evidence": ["Synthetic preserved source history"]}
    project_path.write_text(__import__("json").dumps(project_config))
    subprocess.run(["git", "add", "recovery/project.json"], cwd=tmp_path, check=True)
    subprocess.run(["git", "commit", "-m", "Declare source history baseline"], cwd=tmp_path,
                   check=True, capture_output=True)
    snapshot = source_snapshot(tmp_path)
    owners = exact_component_record_bindings(snapshot)
    assert len(owners) == 1
    rows, checks = component_function_observations(tmp_path, tmp_path / "original/input.exe", owners)
    result = rows[manifest.bindings[0].origin.entity_id]
    assert checks[0]["result"] == "pass"
    assert result["measurement_backend"] == "system_import_original_thunk"
    assert component_claim_result(result) == ("unknown", "UNRESOLVED")
    assert result["bytes"] == result["boundary"] == result["references"] == "unknown"
    assert result["dependency_verification"]["result"] == "pass"
    assert result["dependency_verification"]["archive_selection"] == "unknown"
    measured = measure_source(tmp_path, ["_OriginalTime"], image=Path("original/input.exe"),
                              in_file="src/system.recovery.h")
    measure_row, = measured["measurements"]
    assert measured["result"] == "unknown"
    assert measure_row["verdict"] == "UNRESOLVED" and measure_row["result"] == "unknown"
    assert measure_row["dependency_verification"]["result"] == "pass"
    checked = check_matches(tmp_path, Path("original/input.exe"))
    check_row, = checked["unverifiable"]
    assert checked["verified"] == 0 and check_row["verdict"] == "UNRESOLVED"
    assert check_row["dependency_verification"]["result"] == "pass"
    before = (tmp_path / "src/system.recovery.h").read_bytes()
    with pytest.raises(ValueError, match="not a configured translation unit"):
        record(tmp_path, ["_OriginalTime"], image=Path("original/input.exe"),
               in_file="src/system.recovery.h")
    assert (tmp_path / "src/system.recovery.h").read_bytes() == before
