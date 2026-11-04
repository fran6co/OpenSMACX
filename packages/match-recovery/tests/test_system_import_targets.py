"""SDK thunk targets must be independent of the application call operand."""

import base64
from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery.application_binary import check_compiled
from match_recovery.bound_objects import FunctionBinding
from match_recovery.component_source import render_component
from match_recovery.component_targets import component_targets
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from test_binary import coff_fixture
from test_coff_inventory import imported
from test_system_import_components import fixture


def project(root, manifest, image, archive, *, declarations=True):
    (root / "recovery").mkdir()
    (root / "src").mkdir()
    library = root / "sdk.lib"
    library.write_bytes(archive)
    config = {"image_pin": "src/PIN", "address_space": "pe:va", "roots": ["src"],
              "seed_roots": [], "symbol_format": "msvc"}
    (root / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1, "source": config,
        "components": {"system_import_archives": {"demo-system": str(library)}}}))
    header = "// RECOVERY_IMPORT: v1 __imp__Paint@4 DEMO.dll name:Paint\n" if declarations else ""
    header += render_component(manifest)
    (root / "src/sdk.recovery.h").write_text(header)
    return {"image_sha256": hashlib.sha256(image).hexdigest(), "source_configuration": config,
            "records": [], "files": {"src/sdk.recovery.h": {
                "content_base64": base64.b64encode(header.encode()).decode()}}}


def test_system_only_project_publishes_physical_function_symbol_not_display_or_iat(tmp_path):
    manifest, image, raw, _ = fixture()
    snapshot = project(tmp_path, manifest, image, raw)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {"_Paint@4": 0x401000}
    report, = reports
    assert report["functions"][0]["physical_record"]["symbol"] == "_Paint@4"
    assert report["functions"][0]["symbol"] == "OriginalTime"  # Deliberately different display identity.
    assert report["application_recoveries"] == 0
    assert report["link_verification"] == report["runtime_verification"] == "unknown"


@pytest.mark.parametrize("damage", ["archive", "missing_import", "data_import", "wrong_dll",
                                    "wrong_name", "thunk", "fixup", "missing_config"])
def test_damaged_independent_thunk_evidence_never_publishes(tmp_path, damage):
    kwargs = {}
    if damage == "data_import": kwargs["import_record"] = imported(kind=1)
    if damage == "wrong_dll": kwargs["import_record"] = imported(module="OTHER.dll")
    if damage == "wrong_name": kwargs["import_record"] = imported("_Other@4")
    if damage == "thunk": kwargs["thunk"] = b"\xff\x15\x60\x11\x40\0"
    if damage == "fixup": kwargs["relocation"] = 0
    manifest, image, raw, _ = fixture(**kwargs)
    if damage == "archive": manifest = replace(manifest, archive_sha256="0" * 64)
    snapshot = project(tmp_path, manifest, image, raw, declarations=damage != "missing_import")
    if damage == "missing_config":
        (tmp_path / "recovery/project.json").write_text('{"schema_version":1}')
    assert component_targets(tmp_path, snapshot, image)[0] == {}


def test_same_sdk_symbol_at_two_original_thunks_remains_ambiguous(tmp_path):
    manifest, image, raw, _ = fixture()
    other = replace(manifest, component=replace(manifest.component, component_id="second"),
                    bindings=(replace(manifest.bindings[0], symbol="SecondTime",
                        origin=replace(manifest.bindings[0].origin, address=0x401010),
                        spans=(Span(0x401010, 0x401016),)),))
    image = bytearray(image)
    image[0x210:0x216] = image[0x200:0x206]
    # Both independent thunk operands need their own original HIGHLOW fixup.
    # Reuse the established fixture's relocation block and add the second site.
    struct.pack_into("<H", image, 0x30a, 0x3012)
    image = bytes(image)
    pin = hashlib.sha256(image).hexdigest()
    manifest = replace(manifest, bindings=(replace(manifest.bindings[0],
                       origin=replace(manifest.bindings[0].origin, image_sha256=pin)),))
    other = replace(other, bindings=(replace(other.bindings[0],
                    origin=replace(other.bindings[0].origin, image_sha256=pin)),))
    snapshot = project(tmp_path, manifest, image, raw)
    header = render_component(other)
    snapshot["files"]["src/other.recovery.h"] = {"content_base64": base64.b64encode(header.encode()).decode()}
    config_path = tmp_path / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["components"]["system_import_archives"]["second"] = str(tmp_path / "sdk.lib")
    config_path.write_text(json.dumps(config))
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert len(reports) == 2 and all(r["result"] == "pass" for r in reports)
    assert targets == {}


def test_direct_caller_requires_correct_verified_thunk_not_the_iat(tmp_path):
    manifest, image, raw, _ = fixture()
    image = bytearray(image)
    image[0x220:0x226] = b"\xe8" + struct.pack("<i", 0x401000 - 0x401025) + b"\xc3"
    image = bytes(image)
    pin = hashlib.sha256(image).hexdigest()
    manifest = replace(manifest, bindings=(replace(manifest.bindings[0],
                       origin=replace(manifest.bindings[0].origin, image_sha256=pin)),))
    snapshot = project(tmp_path, manifest, image, raw)
    targets, _ = component_targets(tmp_path, snapshot, image)
    caller = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 0x14))
                  .replace(b"_g".ljust(8, b"\0"), b"_Paint@4"))
    subject = FunctionBinding(Origin(pin, "pe:va", 0x401020),
                              (Span(0x401020, 0x401026),), "_f", "src/caller.cpp")
    def check(mapping):
        return check_compiled([subject], (), (), {"src/caller.cpp": caller},
                              PeImage(image, pin), mapping)[0][0]
    assert check({})["result"] == "unknown"
    assert check(targets)["result"] == "pass"
    assert check({"_Paint@4": 0x401160})["result"] == "fail"  # IAT is not callable thunk.


def test_conflicting_product_identity_cannot_borrow_sdk_target(tmp_path):
    manifest, image, raw, _ = fixture()
    snapshot = project(tmp_path, manifest, image, raw)
    snapshot["records"] = [{"path": "src/product.cpp", "candidate_kind": "source",
                            "entity_id": "product", "emitted_symbol_hint": "_Paint@4",
                            "catalogued_symbol": "_Paint@4", "origin": {"address": 0x401020}}]
    assert component_targets(tmp_path, snapshot, image)[0] == {}
