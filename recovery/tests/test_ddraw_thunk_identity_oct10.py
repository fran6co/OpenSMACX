"""Live dependency attribution controls; not DLL/runtime or current-link proof."""
from dataclasses import replace
import hashlib
from pathlib import Path
import os
import struct

import pytest
from match_recovery.component_original_routing import component_claim_result
from match_recovery.component_source import parse_component, render_component, read_components
from match_recovery.components import SystemImportManifest
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from match_recovery.records import read_json
from match_recovery.source_history import revision_files
from match_recovery.source_inventory import image_pin
from test_indirect_dependency_frontier_oct10 import NEW_COMPONENTS, without_stored_binding
from test_crt_eight_entry_bindings_oct10 import without_eight_bindings

ROOT = Path(__file__).resolve().parents[2]
BASE = "cc7a89781fec346950ce32bc3c2d8d135bde954a"
COMPANION = "src/vendor/ddraw.recovery.h"
COMPONENT = "ddraw-directdrawcreate-thunk"
ADDRESS = 0x5D44FC
ARCHIVE_PIN = "329c7de4664532620fba0e1734e48069a9e8483032efb860db1329d78d91d457"
RECORD_PIN = "9c932ae550c59d1b07e598bafc1da3924cc59c218836a9095792224d7b88197e"


@pytest.fixture(scope="module")
def live():
    text = (ROOT / COMPANION).read_text()
    manifest = parse_component(text)
    assert isinstance(manifest, SystemImportManifest)
    assert render_component(manifest) == text
    from test_additional_import_thunks_oct11 import without_additional_imports
    manifest = without_additional_imports(manifest)
    assert manifest.component.component_id == COMPONENT
    assert manifest.component.ownership.value == "system"
    assert manifest.component.delivery.value == "system_import"
    assert manifest.archive_sha256 == ARCHIVE_PIN
    binding, = manifest.bindings
    assert binding.origin == Origin(image_pin(ROOT), "pe:va", ADDRESS)
    assert binding.spans == (Span(ADDRESS, ADDRESS + 6),)
    assert (binding.symbol, binding.unit, binding.occurrence, binding.import_symbol) == (
        "DirectDrawCreate", "DDRAW.dll", 9, "__imp__DirectDrawCreate@12")
    declarations = read_imports(ROOT)
    imported, = [d for d in declarations if d.symbol == binding.import_symbol]
    assert (imported.path, imported.module, imported.name, imported.ordinal) == (
        "src/general.cpp", "DDRAW.dll", "DirectDrawCreate", None)
    config = read_json(ROOT / "recovery/project.json")
    archive = Path(config["components"]["system_import_archives"][COMPONENT])
    assert archive.is_absolute()
    return manifest, declarations, archive


def local_inputs(live):
    manifest, declarations, archive_path = live
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file() or not archive_path.is_file():
        pytest.skip("pinned original or configured proprietary SDK archive unavailable")
    image, archive = path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_PIN
    return manifest, declarations, image, archive


def test_preserves_existing_source_components_and_does_not_grant_code_claims(live):
    historical = revision_files(ROOT, BASE, ("src/vendor",))
    prior = {name: parse_component(data.decode()) for name, data in historical.items()
             if name.endswith(".recovery.h")}
    for name, manifest in prior.items():
        observed = parse_component((ROOT / name).read_text())
        if observed.component.component_id == "zlib":
            observed = without_stored_binding(observed)
        elif observed.component.component_id == "vc6-runtime":
            observed = without_eight_bindings(observed)
        assert observed == manifest
    current = read_components(ROOT / "src")
    assert {m.component.component_id for m in current} == (
        {m.component.component_id for m in prior.values()} | {COMPONENT} | NEW_COMPONENTS)
    text = (ROOT / COMPANION).read_text()
    assert "ORIGINAL:" not in text and "BYTE_EXACT" not in text
    assert component_claim_result({"measurement_backend": "system_import_original_thunk"}) == (
        "unknown", "UNRESOLVED")


def test_original_thunk_sdk_record_and_loader_identity_pass_with_explicit_limits(live):
    manifest, declarations, image, archive = local_inputs(live)
    report = manifest.verify(image, archive, declarations)
    assert report["result"] == "pass"
    row, = report["functions"]
    assert row["address"] == "0x005D44FC"
    assert row["bytes"] == row["boundary"] == row["references"] == "pass"
    assert row["import"] == {"module": "DDRAW.dll", "name": "DirectDrawCreate",
                             "ordinal": None, "address": "0x00669018"}
    assert row["physical_record"] == {"member": "DDRAW.dll", "occurrence": 9, "sha256": RECORD_PIN}
    assert row["archive_selection"] == report["link_verification"] == "unknown"
    assert report["application_recoveries"] == 0


def raw_offset(pe, address):
    region, offset, size = next((s, o, n) for s, o, n in pe.sections
                               if s.start <= address < s.start + n)
    return offset + address - region.start


@pytest.mark.parametrize("damage", ("opcode", "target", "relocation"))
def test_physical_image_damage_cannot_pass_even_with_fresh_image_pin(live, damage):
    manifest, declarations, image, archive = local_inputs(live)
    pe = PeImage(image, image_pin(ROOT))
    changed = bytearray(image)
    if damage == "opcode":
        changed[raw_offset(pe, ADDRESS)] ^= 1
    elif damage == "target":
        changed[raw_offset(pe, ADDRESS + 2)] ^= 1
    else:
        rva, length = pe.directories[5]
        table = pe.read(Span(pe.base + rva, pe.base + rva + length))
        at = 0
        matches = []
        while at < len(table):
            page, block_size = struct.unpack_from("<II", table, at)
            for entry_at in range(at + 8, at + block_size, 2):
                entry, = struct.unpack_from("<H", table, entry_at)
                if entry >> 12 == 3 and pe.base + page + (entry & 0xFFF) == ADDRESS + 2:
                    matches.append((entry_at, entry))
            at += block_size
        entry_at, entry = matches[0]
        assert len(matches) == 1
        struct.pack_into("<H", changed, raw_offset(pe, pe.base + rva + entry_at), entry & 0xFFF)
    changed_image = bytes(changed)
    binding = replace(manifest.bindings[0], origin=Origin(hashlib.sha256(changed_image).hexdigest(), "pe:va", ADDRESS))
    report = replace(manifest, bindings=(binding,)).verify(changed_image, archive, declarations)
    assert report["result"] != "pass"


def test_archive_member_and_independent_import_damage_refuses(live):
    manifest, declarations, image, archive = local_inputs(live)
    assert manifest.verify(image, archive + b"\0", declarations)["result"] != "pass"
    binding, = manifest.bindings
    for wrong in (replace(binding, occurrence=8), replace(binding, unit="NOT-DDRAW.dll"),
                  replace(binding, import_symbol="__imp__DirectDrawEnumerateA@8")):
        assert replace(manifest, bindings=(wrong,)).verify(image, archive, declarations)["result"] != "pass"
    missing = [d for d in declarations if d.symbol != binding.import_symbol]
    assert manifest.verify(image, archive, missing)["result"] != "pass"
    with pytest.raises(ValueError, match="exactly six bytes"):
        replace(binding, spans=(Span(ADDRESS, ADDRESS + 7),))
