"""Permanent live-source controls for the narrow WinMM thunk mapping.

The test never constructs/falls back to a candidate manifest. It reads the
installed companion and project configuration. Set WINMM_TEST_SOURCE_ROOT only
to point at a staged or integrated source tree; WINMM_TEST_HISTORY_ROOT points
at the checkout carrying the configured immutable source history.
"""

from dataclasses import replace
import hashlib
import os
from pathlib import Path

import pytest

from match_recovery.component_original_routing import component_claim_result
from match_recovery.component_source import parse_component
from match_recovery.components import SystemImportManifest
from match_recovery.import_source import read_imports
from match_recovery.model import Origin
from match_recovery.pe_coff import PeImage
from match_recovery.records import read_json
from match_recovery.source import read_text, write
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_history import revision_files
from match_recovery.source_inventory import image_pin


CHECKOUT = next(parent for parent in Path(__file__).resolve().parents
                if (parent / "src/IMAGE_PIN").is_file())
SOURCE_ROOT = Path(os.environ.get("WINMM_TEST_SOURCE_ROOT", CHECKOUT)).resolve()
HISTORY_ROOT = Path(os.environ.get("WINMM_TEST_HISTORY_ROOT", CHECKOUT)).resolve()
BASELINE = "62a0a9b6610fdb7be399b01425fb0d99f63f6ade"
SEED_PATH = "src/unrecovered/005a58d0.cpp"
COMPANION_PATH = "src/vendor/winmm.recovery.h"
COMPONENT_ID = "winmm-timegettime-thunk"
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARCHIVE_SHA256 = "bb098679a802afdbc6fdaac03dfe1ea81453a36f6d4768b1c1cd092b1dbe9342"
ADDRESS = 0x005A58D0


@pytest.fixture(scope="module")
def live_source():
    companion_path = SOURCE_ROOT / COMPANION_PATH
    assert companion_path.is_file(), f"restored source companion is required: {companion_path}"
    companion = companion_path.read_text(encoding="utf-8")
    manifest = parse_component(companion)
    assert isinstance(manifest, SystemImportManifest)
    assert manifest.component.component_id == COMPONENT_ID
    assert manifest.component.ownership.value == "system"
    assert manifest.component.delivery.value == "system_import"
    assert manifest.archive_sha256 == ARCHIVE_SHA256
    binding, = manifest.bindings
    assert (binding.origin.address, binding.origin.address_space, binding.origin.image_sha256) == (
        ADDRESS, "pe:va", IMAGE_SHA256)
    assert binding.spans[0].start == ADDRESS and binding.spans[0].end == ADDRESS + 6
    assert (binding.symbol, binding.unit, binding.occurrence, binding.import_symbol) == (
        "timeGetTime", "WINMM.dll", 154, "__imp__timeGetTime@0")

    config = read_json(SOURCE_ROOT / "recovery/project.json")
    components = config.get("components")
    assert isinstance(components, dict)
    archives = components.get("system_import_archives")
    assert isinstance(archives, dict) and archives.get(COMPONENT_ID)
    archive_path = Path(archives[COMPONENT_ID])
    if not archive_path.is_absolute():
        source_config = SourceConfiguration.read(SOURCE_ROOT)
        archive_path = source_config.path(SOURCE_ROOT, archives[COMPONENT_ID])

    history = revision_files(HISTORY_ROOT, BASELINE, ("src/unrecovered",))
    historical_seed = history[SEED_PATH].decode("utf-8")
    prefix = historical_seed[:historical_seed.index("\n\n") + 1]
    records = read_text(prefix, SEED_PATH)
    assert len(records) == 1
    seed, = records
    assert seed.address == ADDRESS and seed.mode.value == "file" and seed.byte_exact
    assert write(prefix, records) == prefix
    archive_envelope = ("/*\nHistorical seed catalogue: git:" + BASELINE + ":" + SEED_PATH
                        + "\n" + prefix + "*/\n")
    assert archive_envelope in companion, "complete historical seed prefix was not retained"

    source_config = SourceConfiguration.read(SOURCE_ROOT)
    assert image_pin(SOURCE_ROOT, source_config) == IMAGE_SHA256
    declarations = read_imports(SOURCE_ROOT)
    clock_imports = [row for row in declarations if row.symbol == "__imp__timeGetTime@0"]
    assert len(clock_imports) == 1
    assert (clock_imports[0].path, clock_imports[0].module,
            clock_imports[0].name, clock_imports[0].ordinal) == (
                "src/wave.cpp", "WINMM.dll", "timeGetTime", None)
    return companion, manifest, declarations, archive_path


def local_inputs(live_source):
    _, _, declarations, archive_path = live_source
    image_path = Path(os.environ.get("OPENSMACX_IMAGE",
                      HISTORY_ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("pinned PE or configured proprietary WINMM archive is unavailable")
    image, archive = image_path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == IMAGE_SHA256
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_SHA256
    return live_source[1], declarations, image, archive


def test_actual_companion_config_and_seed_history_are_joined(live_source):
    companion, manifest, _, _ = live_source
    assert parse_component(companion) == manifest
    assert "// RECOVERY_COMPONENT_BEGIN v1" in companion
    assert "Historical seed catalogue: git:" + BASELINE + ":" + SEED_PATH in companion
    # A passing original-component record is intentionally not a current code claim.
    assert component_claim_result({"measurement_backend": "system_import_original_thunk"}) == (
        "unknown", "UNRESOLVED")


def test_original_thunk_and_archive_qualify_but_current_selection_stays_unknown(live_source):
    manifest, declarations, image, archive = local_inputs(live_source)
    report = manifest.verify(image, archive, declarations)
    assert report["result"] == "pass"
    row, = report["functions"]
    assert row["address"] == "0x005A58D0"
    assert row["physical_record"] == {
        "member": "WINMM.dll", "occurrence": 154,
        "sha256": "9339bfc75141eeaef1f212167c84527e194c4e726931ccbb357f9d24aae7dbff",
    }
    assert row["archive_selection"] == report["link_verification"] == "unknown"
    assert report["application_recoveries"] == 0


def test_original_operand_archive_digest_and_member_mutations_refuse(live_source):
    manifest, declarations, image, archive = local_inputs(live_source)
    pe = PeImage(image, IMAGE_SHA256)
    region, raw_offset, raw_size = next(
        (region, offset, size) for region, offset, size in pe.sections
        if region.start <= ADDRESS and ADDRESS + 6 <= region.start + size)
    changed = bytearray(image)
    changed[raw_offset + ADDRESS - region.start + 2] ^= 1
    changed_image = bytes(changed)
    changed_pin = hashlib.sha256(changed_image).hexdigest()
    changed_origin = Origin(changed_pin, "pe:va", ADDRESS)
    changed_manifest = replace(manifest, bindings=(replace(manifest.bindings[0],
                               origin=changed_origin),))
    assert changed_manifest.verify(changed_image, archive, declarations)["result"] != "pass"

    assert manifest.verify(image, archive + b"\0", declarations)["result"] != "pass"
    wrong_occurrence = replace(manifest, bindings=(replace(manifest.bindings[0], occurrence=153),))
    assert wrong_occurrence.verify(image, archive, declarations)["result"] != "pass"
    wrong_member = replace(manifest, bindings=(replace(manifest.bindings[0], unit="NOT-WINMM.dll"),))
    assert wrong_member.verify(image, archive, declarations)["result"] != "pass"
