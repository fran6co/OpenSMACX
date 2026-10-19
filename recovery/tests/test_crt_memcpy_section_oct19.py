"""Pinned memcpy contribution and size-free entry; not original link acceptance."""

import base64
import csv
from dataclasses import replace
import hashlib
import struct
import subprocess

import pytest

from match_recovery.component_source import parse_component, render_component
from match_recovery.component_targets import component_targets
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import parse_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, CoffRecords, PeImage
from match_recovery.source_inventory import source_snapshot

# Reuse the established authoritative pin/outer-worktree lookup and native
# verifier invocation, rather than introduce another project evidence loader.
from test_crt_threadstart_contribution_oct19 import (
    ROOT, SOURCE, IMAGE_SHA256, inputs, raw_offset, repin,
    repository_root, resolve_pinned_image, run_check,
)


BASELINE = "bc78bc9457e5517872349208e96353f48a8200a1"
MEMBER = "build/intel/mt_obj/memcpy.obj"
MEMBER_SHA256 = "3a93e620dba7b163ced3dc5565b74eb1665791309e804d612322bff7af29497c"
START, END = 0x00645930, 0x00645C65


def contribution(manifest):
    matches = [item for item in manifest.code_sections
               if (item.unit, item.section) == (MEMBER, 1)]
    assert len(matches) == 1
    return matches[0]


def require_entry(section):
    assert section.section_name == ".text" and section.comdat_selection is None
    assert section.span == Span(START, END)
    assert section.origin.address == START
    assert section.origin.address_space == "pe:va"
    assert section.origin.image_sha256 == IMAGE_SHA256
    assert [(entry.name, entry.offset) for entry in section.entries] == [("_memcpy", 0)]


def changed_section(manifest, section):
    old = contribution(manifest)
    return replace(manifest, code_sections=tuple(
        section if item == old else item for item in manifest.code_sections))


def row_for(manifest, image, archive):
    rows = [row for row in run_check(manifest, image, archive)["code_sections"]
            if row["unit"] == MEMBER]
    assert len(rows) == 1
    return rows[0]


def require_prior_cohort(current, prior):
    keys = {(item.unit, item.section) for item in prior.code_sections}
    assert replace(current, code_sections=tuple(
        item for item in current.code_sections
        if (item.unit, item.section) in keys)) == prior


def test_actual_manifest_preserves_complete_prior_cohort_and_size_free_entry():
    current = parse_component(SOURCE.read_text())
    prior_text = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/vendor/vc6-runtime.recovery.h"],
        cwd=ROOT, text=True)
    prior = parse_component(prior_text)
    require_prior_cohort(current, prior)
    current_imports = parse_imports(SOURCE.read_text(), "src/vendor/vc6-runtime.recovery.h")
    for binding in parse_imports(prior_text, "src/vendor/vc6-runtime.recovery.h"):
        assert sum(replace(item, line=binding.line) == binding
                   for item in current_imports) == 1
    require_entry(contribution(current))
    assert not any(item.symbol == "_memcpy" or item.origin.address == START
                   for item in current.bindings)
    assert parse_component(render_component(current)) == current

    # The historical projection must not conceal mutations to old entry fields.
    old = next(item for item in prior.code_sections if item.entries)
    mutated = replace(old, entries=(replace(old.entries[0], offset=old.entries[0].offset + 1),)
                      + old.entries[1:])
    damaged = replace(current, code_sections=tuple(
        mutated if item == old else item for item in current.code_sections))
    with pytest.raises(AssertionError):
        require_prior_cohort(damaged, prior)


def test_pinned_unique_archive_definition_complete_member_and_local_relocations():
    manifest, _, image_data, archive_data = inputs()
    archive = CoffArchive(archive_data)
    member = archive.member(MEMBER)
    assert hashlib.sha256(member).hexdigest() == MEMBER_SHA256
    obj = Coff(member)
    section = obj.sections[0]
    assert section.name == ".text" and len(section.data) == END - START == 821
    assert section.flags & 0x20000020 == 0x20000020
    assert not section.flags & 0x1000
    assert 1 not in obj.associations
    named = [symbol for symbol in obj.symbols.values() if symbol.name == "_memcpy"]
    assert [(symbol.value, symbol.section, symbol.type, symbol.storage) for symbol in named] == [
        (0, 1, 0x20, 2)]
    providers = []
    for name, members in archive.members.items():
        for body in members:
            records = CoffRecords(body)
            providers.extend((name, symbol.value, symbol.section, symbol.type)
                             for symbol in records.symbols.values()
                             if symbol.name == "_memcpy" and symbol.storage == 2
                             and symbol.section > 0)
    assert providers == [(MEMBER, 0, 1, 0x20)]
    assert len(section.relocations) == 46
    assert all(kind == 6 and obj.symbols[target].section == 1
               and obj.symbols[target].storage == 3
               for _, target, kind in section.relocations)
    image = PeImage(image_data, IMAGE_SHA256)
    assert image.absolute_sites(Span(START, END)) == frozenset(
        offset for offset, _, _ in section.relocations)
    require_entry(contribution(manifest))


def test_independent_catalogues_preserve_extent_disagreement():
    # CSV rows are selected by identity, not frozen physical line numbers.
    with (repository_root() / ".opensmacx/game/ida94-functions.csv").open(newline="") as stream:
        ida = [row for row in csv.DictReader(stream) if int(row["address"], 16) == START]
    assert len(ida) == 1
    assert (int(ida[0]["end"], 16), ida[0]["name"]) == (END, "_memcpy")
    with (ROOT / "docs/recovery/ghidra-functions.csv").open(newline="") as stream:
        ghidra = [row for row in csv.DictReader(stream)
                  if row["program_sha256"] == IMAGE_SHA256
                  and row["ghidra_version"] == "12.1.2"
                  and int(row["address"], 16) == START]
    assert len(ghidra) == 1
    ranges = [Span(*(int(part, 16) for part in interval.split("-")))
              for interval in ghidra[0]["body_ranges"].split(";")]
    assert len(ranges) == 21
    assert sum(span.end - span.start for span in ranges) == int(ghidra[0]["size"]) == 664
    assert ranges[0].start == START and ranges[-1].end == END
    # end_address describes the primary range, not the union of body ranges.
    assert int(ghidra[0]["end_address"], 16) == ranges[0].end == 0x0064598B
    assert 664 != END - START


def test_native_section_and_scoped_entry_pass_without_function_or_link_extent():
    manifest, _, image, archive = inputs()
    row = row_for(manifest, image, archive)
    assert all(row[key] == "pass" for key in
               ("result", "boundary", "bytes", "nonrelocation_bytes", "references", "entry_inventory"))
    assert len(row["relocations"]) == len(row["highlow_offsets"]) == 46
    assert all(item["result"] == "pass" for item in row["relocations"])
    assert row["callable"] is False and row["link_target"] is False
    entry, = row["entries"]
    assert (entry["symbol"], entry["offset"], entry["address"]) == ("_memcpy", 0, START)
    assert entry["result"] == "pass" and entry["scoped_reference_target"] is True
    assert entry["callable"] is True and entry["link_target"] is False
    assert entry["function_extent"] == "unknown"


@pytest.mark.parametrize("damage", ["member", "section", "name", "short", "long",
                                     "entry_name", "entry_offset", "opcode", "local_target", "archive"])
def test_valid_damage_inputs_refuse_for_the_intended_reason(damage):
    manifest, _, image, archive = inputs()
    section = contribution(manifest)
    if damage == "member":
        section = replace(section, unit="build/intel/mt_obj/missing-memcpy.obj")
    elif damage == "section":
        section = replace(section, section=99)
    elif damage == "name":
        section = replace(section, section_name=".rdata")
    elif damage in ("short", "long"):
        section = replace(section, span=Span(START, END + (-1 if damage == "short" else 1)))
    elif damage.startswith("entry_"):
        entry, = section.entries
        entry = replace(entry, **({"name": "_not_memcpy"} if damage == "entry_name" else {"offset": 1}))
        section = replace(section, entries=(entry,))
    elif damage in ("opcode", "local_target"):
        altered = bytearray(image)
        address = START if damage == "opcode" else START + 56
        altered[raw_offset(image, address)] ^= 1
        image = bytes(altered)
        manifest = repin(manifest, image)
        section = contribution(manifest)
    else:
        member = CoffArchive(archive).member(MEMBER)
        assert archive.count(member) == 1
        raw_pointer, = struct.unpack_from("<I", member, 20 + 20)
        altered = bytearray(archive)
        altered[archive.index(member) + raw_pointer] ^= 1
        archive = bytes(altered)
        # Valid archive and COFF records, but no longer the pinned artifact.
        Coff(CoffArchive(archive).member(MEMBER))
    manifest = changed_section(manifest, section)
    rows = run_check(manifest, image, archive)["code_sections"]
    row, = [item for item in rows if item["unit"] == section.unit]
    if not damage.startswith("entry_"):
        assert row["result"] != "pass"
    if damage == "member":
        assert "found 0" in row["reason"]
    elif damage == "section":
        assert "ordinal is absent" in row["reason"]
    elif damage == "name":
        assert "name differs" in row["reason"]
    elif damage in ("short", "long"):
        assert "extent differs" in row["reason"]
    elif damage.startswith("entry_"):
        assert row["entry_inventory"] == "fail"
        assert row["entries"][0]["result"] == "fail"
        assert row["entries"][0]["scoped_reference_target"] is False
    elif damage == "opcode":
        assert row["nonrelocation_bytes"] == "fail"
    elif damage == "local_target":
        assert row["references"] == "fail"
        assert row["relocations"][0]["offset"] == 56
        assert row["relocations"][0]["result"] == "fail"
    else:
        assert row["boundary"] == "unknown"


def test_missing_entry_does_not_publish_a_scoped_target_and_actual_guard_refuses():
    manifest, _, image, archive = inputs()
    section = replace(contribution(manifest), entries=())
    with pytest.raises(AssertionError):
        require_entry(section)
    row = row_for(changed_section(manifest, section), image, archive)
    assert row["entries"] == [] and row["callable"] is False and row["link_target"] is False


@pytest.mark.parametrize("damage", ["missing", "misplaced"])
def test_structurally_valid_original_highlow_damage_fails_references(damage):
    manifest, _, image_data, archive = inputs()
    image = PeImage(image_data, IMAGE_SHA256)
    rva, size = image.directories[5]
    at = raw_offset(image_data, image.base + rva)
    end = at + size
    altered = bytearray(image_data)
    found = []
    while at < end:
        page, block_size = struct.unpack_from("<II", image_data, at)
        for offset in range(at + 8, at + block_size, 2):
            entry, = struct.unpack_from("<H", image_data, offset)
            if entry >> 12 == 3 and image.base + page + (entry & 0xFFF) == START + 56:
                found.append(offset)
                struct.pack_into("<H", altered, offset,
                                 0 if damage == "missing" else entry + 1)
        at += block_size
    assert len(found) == 1
    changed = bytes(altered)
    # Parse before verifying: this control changes the relocation shape, not PE
    # validity, and preserves every section byte and the relocation table size.
    parsed = PeImage(changed, hashlib.sha256(changed).hexdigest())
    assert parsed.read(Span(START, END)) == image.read(Span(START, END))
    assert START + 56 not in parsed.relocations
    if damage == "misplaced":
        assert START + 57 in parsed.relocations
    row = row_for(repin(manifest, changed), changed, archive)
    assert row["result"] == "fail" and row["references"] == "fail"
    assert row["boundary"] == "pass"
    assert row["entries"][0]["scoped_reference_target"] is False


def test_duplicate_entry_is_rejected_by_native_schema():
    manifest = parse_component(SOURCE.read_text())
    section = contribution(manifest)
    with pytest.raises(ValueError, match="must be unique"):
        replace(section, entries=section.entries * 2)


def test_wrong_manifest_image_pin_refuses_before_comparison():
    manifest, _, image, archive = inputs()
    section = contribution(manifest)
    wrong = replace(section, origin=replace(section.origin, image_sha256="0" * 64))
    with pytest.raises(AssertionError):
        require_entry(wrong)
    with pytest.raises(ValueError, match="one pinned image"):
        run_check(changed_section(manifest, wrong), image, archive)


@pytest.mark.parametrize("entry_kind", ["valid", "missing", "wrong"])
def test_native_target_publication_requires_the_verified_size_free_entry(entry_kind):
    manifest, _, image, _ = inputs()
    section = contribution(manifest)
    if entry_kind == "missing":
        manifest = changed_section(manifest, replace(section, entries=()))
    elif entry_kind == "wrong":
        manifest = changed_section(manifest, replace(
            section, entries=(replace(section.entries[0], offset=1),)))
    snapshot = source_snapshot(ROOT)
    snapshot["files"]["src/vendor/vc6-runtime.recovery.h"]["content_base64"] = base64.b64encode(
        render_component(manifest).encode()).decode("ascii")
    targets, _ = component_targets(ROOT, snapshot, image)
    if entry_kind == "valid":
        assert targets["_memcpy"] == START
    else:
        assert "_memcpy" not in targets


@pytest.mark.parametrize("kind", ["missing", "wrong", "corrupt"])
def test_explicit_bad_image_pin_never_falls_back(tmp_path, monkeypatch, kind):
    path = tmp_path / "original.exe"
    if kind == "wrong":
        path.write_bytes(b"not the original image")
    elif kind == "corrupt":
        _, image = resolve_pinned_image()
        path.write_bytes(image[:-1] + bytes([image[-1] ^ 1]))
    monkeypatch.setenv("OPENSMACX_IMAGE", str(path))
    with pytest.raises(ValueError, match="missing|src/IMAGE_PIN"):
        resolve_pinned_image()
