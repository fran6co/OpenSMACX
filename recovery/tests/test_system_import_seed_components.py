"""Permanent tests proposed for the installed original system-import companions."""
from __future__ import annotations

import os
from pathlib import Path

import pytest

from match_recovery.component_original_routing import (
    component_claim_result,
    component_function_observations,
    exact_component_record_bindings,
)
from match_recovery.component_source import parse_component_entries
from match_recovery.components import SystemImportManifest
from match_recovery.import_source import read_imports
from match_recovery.model import Ownership, Delivery
from match_recovery.records import read_json
from match_recovery.source_inventory import source_snapshot


def _root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "recovery/project.json").is_file():
            return candidate
    raise AssertionError("cannot locate OpenSMACX recovery/project.json")


ROOT = _root()
IMAGE_SHA = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARCHIVES = {
    "vc6-system-import-winmm": ("WINMM.LIB", "bb098679a802afdbc6fdaac03dfe1ea81453a36f6d4768b1c1cd092b1dbe9342"),
    "vc6-system-import-ddraw": ("DDRAW.LIB", "329c7de4664532620fba0e1734e48069a9e8483032efb860db1329d78d91d457"),
    "vc6-system-import-vfw32": ("VFW32.LIB", "0c2c6ebfeded71da862f8b8d772590ae0dc55d826b9871b0a1052ea333ff2c24"),
    "vc6-system-import-dsound": ("DSOUND.LIB", "37679dff670270005be960defafa10fab2ba4eda6706ffed7cbb5818007a6105"),
    "vc6-system-import-comdlg32": ("COMDLG32.LIB", "1d82261a358979f818e7c799be43d57b48a001dfd3da457f2e04d7c9f680ec00"),
    "vc6-system-import-dplayx": ("DPLAYX.LIB", "99ff5ab78c5189fb7090f8c2da08d0da1095833606ca422a051c83f988406739"),
    "vc6-system-import-kernel32": ("KERNEL32.LIB", "cd4a546f71481319aeee455aa0fb51698a094f9d3e6d8339acf445489a263c68"),
}

# Original address -> (symbol, exclusive span end, component id, member key,
# exact repeated-name occurrence, RECOVERY_IMPORT symbol, module, export name,
# ordinal). These are pinned audit facts, not copied from proposal code.
ROWS = {
    0x005A58D0: ("timeGetTime", 0x005A58D6, "vc6-system-import-winmm", "WINMM.dll", 154,
                 "__imp__timeGetTime@0", "WINMM.dll", "timeGetTime", None),
    0x005D44FC: ("DirectDrawCreate", 0x005D4502, "vc6-system-import-ddraw", "DDRAW.dll", 9,
                 "__imp__DirectDrawCreate@12", "DDRAW.dll", "DirectDrawCreate", None),
    0x00644DC0: ("ICClose", 0x00644DC6, "vc6-system-import-vfw32", "MSVFW32.dll", 27,
                 "__imp__ICClose@4", "MSVFW32.dll", "ICClose", None),
    0x00644DC6: ("ICSendMessage", 0x00644DCC, "vc6-system-import-vfw32", "MSVFW32.dll", 9,
                 "__imp__ICSendMessage@16", "MSVFW32.dll", "ICSendMessage", None),
    0x00644DCC: ("ICOpen", 0x00644DD2, "vc6-system-import-vfw32", "MSVFW32.dll", 12,
                 "__imp__ICOpen@12", "MSVFW32.dll", "ICOpen", None),
    0x006501F0: ("DirectSoundCreate", 0x006501F6, "vc6-system-import-dsound", "DSOUND.dll", 6,
                 "__imp__DirectSoundCreate@12", "DSOUND.dll", None, 1),
    0x006501F6: ("GetSaveFileNameA", 0x006501FC, "vc6-system-import-comdlg32", "comdlg32.dll", 15,
                 "__imp__GetSaveFileNameA@4", "comdlg32.dll", "GetSaveFileNameA", None),
    0x006501FC: ("GetOpenFileNameA", 0x00650202, "vc6-system-import-comdlg32", "comdlg32.dll", 13,
                 "__imp__GetOpenFileNameA@4", "comdlg32.dll", "GetOpenFileNameA", None),
    0x00650202: ("DirectDrawEnumerateA", 0x00650208, "vc6-system-import-ddraw", "DDRAW.dll", 11,
                 "__imp__DirectDrawEnumerateA@8", "DDRAW.dll", "DirectDrawEnumerateA", None),
    0x00650208: ("DirectPlayLobbyCreateA", 0x0065020E, "vc6-system-import-dplayx", "DPLAYX.dll", 7,
                 "__imp__DirectPlayLobbyCreateA@20", "DPLAYX.dll", None, 4),
    0x0065020E: ("DirectPlayEnumerateA", 0x00650214, "vc6-system-import-dplayx", "DPLAYX.dll", 5,
                 "__imp__DirectPlayEnumerateA@8", "DPLAYX.dll", None, 2),
    0x00650214: ("DirectPlayCreate", 0x0065021A, "vc6-system-import-dplayx", "DPLAYX.dll", 3,
                 "__imp__DirectPlayCreate@12", "DPLAYX.dll", None, 1),
    0x0065021A: ("RtlUnwind", 0x00650220, "vc6-system-import-kernel32", "KERNEL32.dll", 554,
                 "__imp__RtlUnwind@16", "KERNEL32.dll", "RtlUnwind", None),
}


def _installed_components():
    snapshot = source_snapshot(ROOT)
    # Use the snapshot's complete source bytes so path/origin joins are tested
    # exactly as match/check-mappings see them.
    import base64
    source_files = {
        path: base64.b64decode(item["content_base64"]).decode("utf-8")
        for path, item in snapshot["files"].items()
    }
    entries = parse_component_entries(source_files)
    components = {path: manifest for path, manifest in entries
                  if isinstance(manifest, SystemImportManifest)}
    return snapshot, components


def test_system_import_seed_companions_preserve_all_original_records_and_imports():
    snapshot, components = _installed_components()
    assert len(components) == 7
    manifests = {manifest.component.component_id: manifest for manifest in components.values()}
    assert set(manifests) == set(ARCHIVES)
    assert all(manifest.component.ownership == Ownership.SYSTEM
               and manifest.component.delivery == Delivery.SYSTEM_IMPORT
               for manifest in manifests.values())
    assert all(manifests[key].archive_sha256 == digest for key, (_, digest) in ARCHIVES.items())
    assert all("redistribution rights are unestablished" in manifest.license_evidence
               for manifest in manifests.values())
    configured_archives = read_json(ROOT / "recovery/project.json")["components"]["system_import_archives"]
    assert set(configured_archives) == set(ARCHIVES)
    assert all(Path(configured_archives[key]).name == archive
               for key, (archive, _) in ARCHIVES.items())

    records_by_address = {}
    for row in snapshot["records"]:
        address = row["origin"]["address"]
        if address in ROWS:
            records_by_address.setdefault(address, []).append(row)
    assert set(records_by_address) == set(ROWS)

    expected_companions = {f"src/system_import_{key.removeprefix('vc6-system-import-')}.recovery.h"
                           for key in ARCHIVES}
    assert set(components) == expected_companions
    owner_rows = exact_component_record_bindings(snapshot)
    system_owner_rows = {key: owner for key, owner in owner_rows.items()
                         if owner[0] in ARCHIVES}
    import_rows = read_imports(ROOT)  # rejects duplicate import-symbol declarations
    imports = {binding.symbol: binding for binding in import_rows}
    assert len(imports) == len(import_rows)
    assert len(system_owner_rows) == 13

    for address, (symbol, end, component, unit, occurrence, import_symbol,
                  module, name, ordinal) in ROWS.items():
        row, = records_by_address[address]
        assert row["origin"]["image_sha256"] == IMAGE_SHA
        assert row["origin"]["address_space"] == "pe:va"
        assert row["catalogued_symbol"] == symbol
        assert row["spans"] == [[address, end]]
        assert row["path"] in components
        assert not row["invalid_spans"]
        assert (row["path"], row["entity_id"]) in system_owner_rows
        component_id, manifest, binding = system_owner_rows[(row["path"], row["entity_id"])]
        assert component_id == component
        assert binding.origin.address == address
        assert binding.spans[0].start == address and binding.spans[0].end == end
        assert binding.symbol == symbol
        assert binding.unit == unit and binding.occurrence == occurrence
        assert binding.import_symbol == import_symbol
        declared = imports[import_symbol]
        assert declared.module == module
        assert declared.name == name and declared.ordinal == ordinal
        # The migrated seeds are replaced, not left as duplicate mappings.
        assert not (ROOT / "src/unrecovered" / f"{address:08x}.cpp").exists()

    time_get_time, = records_by_address[0x005A58D0]
    assert time_get_time["legacy_mode"] == "file"
    assert time_get_time["legacy_byte_exact"] is True
    assert time_get_time["levers"] == [["cast", "the fixed IAT slot to a function-pointer-pointer, call through"]]
    assert sum(row["legacy_byte_exact"] for rows in records_by_address.values() for row in rows) == 1
    assert sum(row["legacy_exclusion"] == "S2a"
               for rows in records_by_address.values() for row in rows) == 8


@pytest.mark.skipif(os.environ.get("OPENSMACX_SYSTEM_IMPORT_ASSET_TESTS") != "1",
                    reason="requires opt-in pinned original image and local proprietary SDK archives")
def test_pinned_original_thunks_verify_without_promoting_historical_matching():
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    options = read_json(ROOT / "recovery/project.json")["components"]["system_import_archives"]
    if not image.is_file() or any(not Path(options[key]).is_file() for key in ARCHIVES):
        pytest.skip("pinned original image or configured local SDK archive is unavailable")
    snapshot = source_snapshot(ROOT, image)
    all_owners = exact_component_record_bindings(snapshot)
    owners = {key: owner for key, owner in all_owners.items()
              if owner[0] in ARCHIVES}
    assert len(owners) == 13
    observations, checks = component_function_observations(ROOT, image, owners)
    assert len(checks) == 7
    assert all(item["result"] == "pass" for item in checks)
    assert len(observations) == 13
    for result in observations.values():
        assert result["measurement_backend"] == "system_import_original_thunk"
        assert result["dependency_verification"]["result"] == "pass"
        assert result["dependency_verification"]["archive_selection"] == "unknown"
        assert (result["bytes"], result["boundary"], result["references"]) == (
            "unknown", "unknown", "unknown")
        assert component_claim_result(result) == ("unknown", "UNRESOLVED")
