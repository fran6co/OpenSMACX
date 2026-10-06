"""Installed CRT source binding controls.

The ordinary test reads src/vendor/vc6-runtime.recovery.h directly and does
not import an ignored emitter or treat a proposal as installed source. The
archive test is explicitly opt-in because LIBCMT.LIB is an external,
proprietary compiler artifact.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.components import ToolchainDataSectionBinding, ToolchainManifest
from match_recovery.import_source import read_imports
from match_recovery.model import Result, Span
from match_recovery.source.reader import read_text


def project_root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "CMakeLists.txt").is_file() and (candidate / "recovery/project.json").is_file():
            return candidate
    raise RuntimeError("could not find the OpenSMACX project root")


ROOT = project_root()
COMPANION = ROOT / "src/vendor/vc6-runtime.recovery.h"
PINNED_IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PINNED_ARCHIVE_SHA256 = "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b"

SEED_IDENTITIES = {
    0x0064D725: ("sub_64d725", Span(0x0064D725, 0x0064D73B)),
    0x0064D73B: ("sub_64d73b", Span(0x0064D73B, 0x0064D751)),
    0x0064D751: ("sub_64d751", Span(0x0064D751, 0x0064D77E)),
    0x0064D77E: ("sub_64d77e", Span(0x0064D77E, 0x0064D7AB)),
}
COMPONENT_FUNCTIONS = {
    0x0064D5B9: (0x0064D725, "__ld12cvt", "../build/intel/mt_obj/intrncvt.obj"),
    0x0064D725: (0x0064D73B, "__ld12tod", "../build/intel/mt_obj/intrncvt.obj"),
    0x0064D73B: (0x0064D751, "__ld12tof", "../build/intel/mt_obj/intrncvt.obj"),
    0x0064D751: (0x0064D77E, "__atodbl", "../build/intel/mt_obj/intrncvt.obj"),
    0x0064D77E: (0x0064D7AB, "__atoflt", "../build/intel/mt_obj/intrncvt.obj"),
    0x0064F524: (0x0064F9F5, "___strgtold12", "../build/intel/mt_obj/strgtold.obj"),
}


def installed_manifest() -> ToolchainManifest:
    text = COMPANION.read_text(encoding="utf-8")
    manifest = parse_component(text)
    assert isinstance(manifest, ToolchainManifest)
    return manifest


def test_installed_companion_retains_four_seed_identities_without_claims() -> None:
    text = COMPANION.read_text(encoding="utf-8")
    source_records = read_text(text, COMPANION)
    records = {record.address: record for record in source_records
               if record.address in SEED_IDENTITIES}
    assert set(records) == set(SEED_IDENTITIES)

    for address, (name, span) in SEED_IDENTITIES.items():
        record = records[address]
        assert record.name == name
        assert record.image_spans == ((span.start, span.end),)
        # The 0x64D73B seed once described itself as BYTE_EXACT in old prose;
        # none of these four source markers grants a parsed current claim.
        assert not record.byte_exact
        assert not record.semantic


def test_installed_companion_binds_wrappers_and_complete_descriptor_section() -> None:
    manifest = installed_manifest()
    bindings = {binding.origin.address: binding for binding in manifest.bindings}
    for address, (end, symbol, unit) in COMPONENT_FUNCTIONS.items():
        binding = bindings[address]
        assert binding.spans == (Span(address, end),)
        assert binding.symbol == symbol
        assert binding.unit == unit

    assert manifest.archive_sha256 == PINNED_ARCHIVE_SHA256
    assert "Proprietary Microsoft runtime" in manifest.license_evidence
    assert "not redistributed" in manifest.license_evidence

    section_bindings = [item for item in manifest.data_sections
                        if isinstance(item, ToolchainDataSectionBinding)]
    assert len(section_bindings) == 1
    section = section_bindings[0]
    assert (section.unit, section.section, section.section_name) == (
        "../build/intel/mt_obj/intrncvt.obj", 1, ".data")
    assert section.origin.image_sha256 == PINNED_IMAGE_SHA256
    assert section.origin.address == 0x006A6928
    assert section.span == Span(0x006A6928, 0x006A6958)
    assert [(symbol.name, symbol.offset, symbol.storage)
            for symbol in section.symbols] == [
                ("_DoubleFormat", 0, 3), ("_FloatFormat", 0x18, 3)]


@pytest.mark.skipif(
    os.environ.get("OPENSMACX_VERIFY_PINNED_CRT_ARCHIVE") != "1",
    reason="external pinned LIBCMT archive verification is opt-in",
)
def test_opted_in_external_archive_matches_four_wrappers_and_data_layout() -> None:
    project = json.loads((ROOT / "recovery/project.json").read_text(encoding="utf-8"))
    configured_archive = project["components"]["toolchain_archives"]["vc6-runtime"]
    archive_path = Path(os.environ.get("OPENSMACX_CRT_ARCHIVE", configured_archive))
    if not archive_path.is_file():
        pytest.skip("the external pinned LIBCMT archive is unavailable")
    archive_bytes = archive_path.read_bytes()
    assert hashlib.sha256(archive_bytes).hexdigest() == PINNED_ARCHIVE_SHA256
    image_bytes = (ROOT / ".opensmacx/game/terranx_original.exe").read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PINNED_IMAGE_SHA256

    report = verify_toolchain(installed_manifest(), image_bytes, archive_bytes,
                              import_bindings=read_imports(ROOT))
    assert report["archive"]["result"] == Result.PASS
    rows = {int(row["address"], 16): row for row in report["functions"]}
    for address in (0x0064D725, 0x0064D73B, 0x0064D751, 0x0064D77E):
        row = rows[address]
        assert row["result"] == row["boundary"] == row["bytes"] == row["references"] == Result.PASS

    data_rows = [row for row in report["data_sections"]
                 if row["address"] == "0x006A6928"]
    assert len(data_rows) == 1
    assert data_rows[0]["result"] == Result.PASS

    # This physical helper boundary includes the associated table. Its unresolved
    # helper references are intentionally not elevated to a whole-component pass.
    helper = rows[0x0064F524]
    assert helper["boundary"] == Result.PASS
    assert helper["span"] == {"start": 0x0064F524, "end": 0x0064F9F5}
