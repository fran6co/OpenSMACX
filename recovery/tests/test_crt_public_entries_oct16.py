"""Pinned runtime entry proofs; not application or final-link acceptance."""

import base64
from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.component_targets import component_targets
from match_recovery.model import Span
from match_recovery.source_inventory import source_snapshot


ROOT = Path(__file__).resolve().parents[2]
COMPANION = "src/vendor/vc6-runtime.recovery.h"
MEMBER = "build/intel/mt_obj/strcat.obj"
START = 0x00645460
END = 0x00645550
STRCHR_MEMBER = "build/intel/mt_obj/strchr.obj"
STRCHR_START = 0x00645DC0
STRCHR_END = 0x00645E8C
GENEXCEP_MEMBER = "../build/intel/mt_obj/genexcep.obj"
GENEXCEP_START = 0x0064A970
GENEXCEP_END = 0x0064A9C3

SPARSE_ENTRY_SECTIONS = {
    STRCHR_MEMBER: {
        "span": Span(STRCHR_START, STRCHR_END),
        "entries": {"_strchr": (0x10, STRCHR_START + 0x10),
                    "___from_strstr_to_strchr": (0x16, STRCHR_START + 0x16)},
    },
    GENEXCEP_MEMBER: {
        "span": Span(GENEXCEP_START, GENEXCEP_END),
        "entries": {"__startTwoArgErrorHandling": (0x0, GENEXCEP_START),
                    "__startOneArgErrorHandling": (0x17, GENEXCEP_START + 0x17)},
    },
}


def manifest():
    return parse_component((ROOT / COMPANION).read_text())


def contribution(value):
    return next(item for item in value.code_sections if item.unit == MEMBER)


def section_for(value, unit):
    return next(item for item in value.code_sections if item.unit == unit)


def snapshot_with_manifest(header):
    snapshot = source_snapshot(ROOT)
    snapshot["files"][COMPANION]["content_base64"] = base64.b64encode(
        header.encode("utf-8")).decode("ascii")
    return snapshot


@pytest.fixture
def pinned_inputs():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    settings = json.loads((ROOT / "recovery/project.json").read_text())
    archive_path = Path(settings["components"]["toolchain_archives"]["vc6-runtime"])
    if not archive_path.is_absolute():
        archive_path = ROOT / archive_path
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("pinned executable and installed runtime archive are required")
    return image_path.read_bytes(), archive_path.read_bytes(), archive_path


def test_source_retains_legacy_identity_and_declares_size_free_entries():
    value = manifest()
    section = contribution(value)
    old, = (item for item in value.bindings if item.symbol == "_strcpy")
    assert old.unit == MEMBER and old.origin.address == START
    assert old.spans == (Span(START, START + 0x10),)
    assert not any(item.symbol == "_strcat" for item in value.bindings)
    assert section.span == Span(START, END)
    assert section.section == 1 and section.section_name == ".text"
    assert {(item.name, item.offset) for item in section.entries} == {
        ("_strcpy", 0), ("_strcat", 0x10)}
    assert all(item.evidence.strip() for item in section.entries)


def test_pinned_whole_contribution_qualifies_entries_not_function_extents(pinned_inputs):
    image, archive, _ = pinned_inputs
    value = manifest()
    report = verify_toolchain(value, image, archive)
    row, = (item for item in report["code_sections"] if item["unit"] == MEMBER)
    assert row["result"] == row["boundary"] == row["bytes"] == row["references"] == "pass"
    assert row["callable"] is False and row["link_target"] is False
    assert {item["symbol"]: item["address"] for item in row["entries"]} == {
        "_strcpy": START, "_strcat": START + 0x10}
    for entry in row["entries"]:
        assert entry["result"] == "pass"
        assert entry["callable"] is True and entry["scoped_reference_target"] is True
        assert entry["link_target"] is False and entry["function_extent"] == "unknown"
    # The old function-size interpretation remains visible and unmeasurable;
    # the independently verified contribution/entry is the new evidence.
    old, = (item for item in report["functions"] if item["symbol"] == "_strcpy")
    assert old["result"] == "unknown"
    assert old.get("boundary") != "pass"
    assert not any(item["symbol"] == "_strcat" for item in report["functions"])
    assert report["application_recoveries"] == 0
    assert report["link_verification"] == report["runtime_verification"] == "unknown"
    assert report["ready_for_collection"] is False


def test_fresh_project_targets_include_both_verified_entries(pinned_inputs):
    image, _, _ = pinned_inputs
    targets, reports = component_targets(ROOT, source_snapshot(ROOT), image)
    assert targets["_strcpy"] == START
    assert targets["_strcat"] == START + 0x10
    assert reports and all(report["link_verification"] == "unknown" for report in reports)


def test_unknown_legacy_member_cannot_publish_without_parent_proof(pinned_inputs, tmp_path):
    image, _, archive_path = pinned_inputs
    value = manifest()
    without_parent = replace(value, code_sections=tuple(
        item for item in value.code_sections if item.unit != MEMBER))
    header = render_component(without_parent)
    recovery = tmp_path / "recovery"
    recovery.mkdir()
    (recovery / "project.json").write_text(json.dumps({
        "schema_version": 1,
        "components": {"toolchain_archives": {"vc6-runtime": str(archive_path)}},
    }))
    snapshot = {
        "image_sha256": hashlib.sha256(image).hexdigest(),
        "records": [],
        "source_configuration": {
            "image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
            "seed_roots": ["src/unrecovered"], "symbol_format": "msvc",
        },
        "files": {COMPANION: {"content_base64": base64.b64encode(header.encode()).decode()}},
    }
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert "_strcpy" not in targets and "_strcat" not in targets
    assert targets, "the control must retain other independently measured runtime targets"
    old, = (item for item in reports[0]["functions"] if item["symbol"] == "_strcpy")
    assert old["result"] == "unknown" and old.get("boundary") != "pass"


def test_wrong_anchor_or_archive_cannot_qualify_entries(pinned_inputs):
    image, archive, _ = pinned_inputs
    value = manifest()
    section = contribution(value)
    wrong = replace(section, entries=tuple(
        replace(item, offset=item.offset + 1) if item.name == "_strcat" else item
        for item in section.entries))
    damaged = replace(value, code_sections=tuple(
        wrong if item.unit == MEMBER else item for item in value.code_sections))
    for selected, archive_bytes in ((damaged, archive), (value, archive + b"damaged")):
        report = verify_toolchain(selected, image, archive_bytes)
        row, = (item for item in report["code_sections"] if item["unit"] == MEMBER)
        assert all(item["result"] != "pass" for item in row["entries"])
        assert all(item["scoped_reference_target"] is False for item in row["entries"])


def test_source_adds_only_sparse_size_free_entries_to_existing_contributions():
    value = manifest()
    for unit, expected in SPARSE_ENTRY_SECTIONS.items():
        section = section_for(value, unit)
        assert section.span == expected["span"]
        assert section.section == 1 and section.section_name == ".text"
        assert section.origin.image_sha256 == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
        assert {item.name: (item.offset, section.span.start + item.offset)
                for item in section.entries} == expected["entries"]
        assert all(item.evidence.strip() for item in section.entries)
        assert all(item.name not in {binding.symbol for binding in value.bindings}
                   for item in section.entries)

    baseline_text = subprocess.check_output(
        ["git", "show", "40cb844010428167d760de95780a0891bd95d55f:" + COMPANION],
        cwd=ROOT, text=True)
    baseline = parse_component(baseline_text)
    target_units = set(SPARSE_ENTRY_SECTIONS)
    restored = replace(baseline, code_sections=tuple(
        replace(section, entries=section_for(value, section.unit).entries)
        if section.unit in target_units else section
        for section in baseline.code_sections))
    assert restored == value

    assert section_for(value, MEMBER).span == Span(START, END)
    assert {(item.name, item.offset) for item in section_for(value, MEMBER).entries} == {
        ("_strcpy", 0), ("_strcat", 0x10)}


def test_pinned_sparse_entries_qualify_only_their_complete_sections(pinned_inputs):
    image, archive, _ = pinned_inputs
    value = manifest()
    report = verify_toolchain(value, image, archive)
    assert report["archive"]["result"] == "pass"
    for unit, expected in SPARSE_ENTRY_SECTIONS.items():
        row, = (item for item in report["code_sections"] if item["unit"] == unit)
        assert row["span"] == {"start": expected["span"].start,
                               "end": expected["span"].end}
        assert (row["result"] == row["boundary"] == row["bytes"]
                == row["references"] == row["entry_inventory"] == "pass")
        assert row["callable"] is False and row["link_target"] is False
        assert {entry["symbol"]: (entry["offset"], entry["address"])
                for entry in row["entries"]} == expected["entries"]
        for entry in row["entries"]:
            assert entry["result"] == "pass"
            assert entry["callable"] and entry["scoped_reference_target"]
            assert entry["link_target"] is False
            assert entry["function_extent"] == "unknown"

    known_units = set(SPARSE_ENTRY_SECTIONS) | {MEMBER}
    qualified = {entry["symbol"] for row in report["code_sections"]
                 if row["unit"] in known_units for entry in row["entries"]}
    assert qualified == {"_strcpy", "_strcat", "_strchr", "___from_strstr_to_strchr",
                         "__startTwoArgErrorHandling", "__startOneArgErrorHandling"}
    assert report["application_recoveries"] == 0
    assert report["link_verification"] == report["structure_verification"]
    assert report["link_verification"] == report["runtime_verification"] == "unknown"
    assert report["ready_for_collection"] is False


def test_fresh_native_targets_include_all_four_sparse_entries(pinned_inputs):
    image, _, _ = pinned_inputs
    targets, reports = component_targets(ROOT, source_snapshot(ROOT), image)
    expected = {symbol: address for section in SPARSE_ENTRY_SECTIONS.values()
                for symbol, (_offset, address) in section["entries"].items()}
    assert {symbol: targets[symbol] for symbol in expected} == expected
    assert reports and all(report["link_verification"] == "unknown" for report in reports)


@pytest.mark.parametrize("unit", tuple(SPARSE_ENTRY_SECTIONS))
def test_malformed_sparse_offset_fails_closed_in_native_target_export(pinned_inputs, unit):
    image, _, _ = pinned_inputs
    value = manifest()
    section = section_for(value, unit)
    damaged_entry = section.entries[0]
    damaged = replace(section, entries=tuple(
        replace(entry, offset=entry.offset + 1) if entry == damaged_entry else entry
        for entry in section.entries))
    changed = replace(value, code_sections=tuple(
        damaged if item.unit == unit else item for item in value.code_sections))
    targets, reports = component_targets(
        ROOT, snapshot_with_manifest(render_component(changed)), image)
    assert not (set(targets) & set(SPARSE_ENTRY_SECTIONS[unit]["entries"]))
    row, = (item for item in reports[0]["code_sections"] if item["unit"] == unit)
    assert row["entry_inventory"] == "fail"
    assert row["boundary"] == row["bytes"] == "pass"
    assert all(entry["result"] == "fail" and entry["address"] is None
               and entry["scoped_reference_target"] is False for entry in row["entries"])


@pytest.mark.parametrize("unit", tuple(SPARSE_ENTRY_SECTIONS))
def test_missing_sparse_parent_cannot_publish_its_entries(pinned_inputs, unit):
    image, _, _ = pinned_inputs
    value = manifest()
    without_parent = replace(value, code_sections=tuple(
        item for item in value.code_sections if item.unit != unit))
    targets, reports = component_targets(
        ROOT, snapshot_with_manifest(render_component(without_parent)), image)
    assert not (set(targets) & set(SPARSE_ENTRY_SECTIONS[unit]["entries"]))
    assert reports and all(report["link_verification"] == "unknown" for report in reports)
