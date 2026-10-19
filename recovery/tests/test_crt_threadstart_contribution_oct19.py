"""Pinned CRT thread-start contribution attribution; not runtime acceptance."""

from dataclasses import replace
import csv
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.components import CodeEntryAnchor
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASELINE = "e30e2bf17a1fb423ab52abfb131b060b5275e0d4"
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARCHIVE_SHA256 = "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b"
MEMBER = "build/intel/mt_obj/thread.obj"
MEMBER_SHA256 = "5cce4c839d1764f275e326b85b388f98e40622a9f2f2030843082c9023b2e596"
ENTRY = 0x00646FD4
PRIMARY_END = 0x00647048
SECTION_END = 0x00647067
SCOPE_TABLE = 0x006710A8


def baseline_manifest_and_text():
    text = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/vendor/vc6-runtime.recovery.h"],
        cwd=ROOT, text=True)
    return parse_component(text), text


def image_pin():
    rows = [line.split() for line in (ROOT / "src/IMAGE_PIN").read_text(encoding="utf-8").splitlines()
            if line.strip().startswith("sha256 ")]
    assert len(rows) == 1
    fields = rows[0]
    assert len(fields) == 2 and fields[0] == "sha256"
    assert fields[1] == IMAGE_SHA256
    return fields[1]


def repository_root():
    common = subprocess.check_output(["git", "rev-parse", "--git-common-dir"],
                                     cwd=ROOT, text=True).strip()
    return (ROOT / common).resolve().parent


def resolve_pinned_image():
    """Explicit override is authoritative; absent override uses outer repo first."""
    explicit = os.environ.get("OPENSMACX_IMAGE")
    if explicit is not None:
        path = Path(explicit)
        if not path.is_file():
            raise ValueError(f"explicit OPENSMACX_IMAGE is missing: {path}")
    else:
        outer_root = repository_root()
        candidates = (outer_root / ".opensmacx/game/terranx_original.exe",
                      ROOT / ".opensmacx/game/terranx_original.exe")
        path = next((candidate for candidate in candidates if candidate.is_file()), None)
        if path is None:
            raise ValueError("pinned original image is unavailable in outer repository or worktree")
    try:
        image = path.read_bytes()
    except OSError as error:
        raise ValueError(f"cannot read pinned original image: {path}") from error
    if hashlib.sha256(image).hexdigest() != image_pin():
        raise ValueError(f"original image does not match src/IMAGE_PIN: {path}")
    return path, image


def inputs():
    manifest = parse_component(SOURCE.read_text(encoding="utf-8"))
    image_path, image = resolve_pinned_image()
    settings = json.loads((ROOT / "recovery/project.json").read_text())
    archive_path = Path(settings["components"]["toolchain_archives"]["vc6-runtime"])
    if not archive_path.is_absolute():
        archive_path = ROOT / archive_path
    if not archive_path.is_file():
        raise ValueError(f"configured LIBCMT archive is unavailable: {archive_path}")
    archive = archive_path.read_bytes()
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_SHA256
    return manifest, image_path, image, archive


def repin(manifest, image):
    digest = hashlib.sha256(image).hexdigest()
    return replace(
        manifest,
        bindings=tuple(replace(item, origin=replace(item.origin, image_sha256=digest))
                       for item in manifest.bindings),
        data_sections=tuple(replace(item, origin=replace(item.origin, image_sha256=digest))
                            for item in manifest.data_sections),
        code_sections=tuple(replace(item, origin=replace(item.origin, image_sha256=digest))
                            for item in manifest.code_sections))


def code_section(manifest):
    matches = [item for item in manifest.code_sections
               if item.unit == MEMBER and item.section == 3]
    assert len(matches) == 1
    return matches[0]


def run_check(manifest, image, archive):
    return verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))


def raw_offset(image, address):
    parsed = PeImage(image, hashlib.sha256(image).hexdigest())
    matches = [(offset + address - region.start)
               for region, offset, size in parsed.sections
               if region.start <= address < region.start + size]
    assert len(matches) == 1
    return matches[0]


def test_manifest_addition_preserves_all_prior_component_state_and_import_prefix():
    prior, prior_text = baseline_manifest_and_text()
    current_text = SOURCE.read_text(encoding="utf-8")
    current = parse_component(current_text)
    section = code_section(current)
    assert current_text.split("// RECOVERY_COMPONENT_BEGIN v1", 1)[0] == \
        prior_text.split("// RECOVERY_COMPONENT_BEGIN v1", 1)[0]
    assert replace(current, code_sections=tuple(
        item for item in current.code_sections if item != section)) == prior
    assert current.bindings == prior.bindings
    assert current.absolute_symbols == prior.absolute_symbols
    assert current.data_sections == prior.data_sections
    assert len(current.code_sections) == len(prior.code_sections) + 1

    assert section.origin.image_sha256 == IMAGE_SHA256
    assert section.origin.address_space == "pe:va"
    assert section.span == Span(ENTRY, SECTION_END)
    assert section.section_name == ".text"
    assert section.comdat_selection == 1
    assert section.entries == ()
    assert "__threadstart@4 at offset 0, storage class 3" in section.evidence
    assert not any(item.symbol == "__threadstart@4" or item.origin.address == ENTRY
                   for item in current.bindings)
    assert any(item.symbol == "__beginthread" and item.spans == (Span(0x646F5D, ENTRY),)
               for item in current.bindings)
    assert any(item.symbol == "__endthread" and item.spans == (Span(SECTION_END, 0x6470A6),)
               for item in current.bindings)
    assert parse_component(render_component(current)) == current


def test_image_resolution_is_root_first_and_explicit_overrides_fail_closed(tmp_path, monkeypatch):
    monkeypatch.delenv("OPENSMACX_IMAGE", raising=False)
    path, image = resolve_pinned_image()
    assert path == repository_root() / ".opensmacx/game/terranx_original.exe"
    assert hashlib.sha256(image).hexdigest() == IMAGE_SHA256

    missing = tmp_path / "missing.exe"
    wrong = tmp_path / "wrong.exe"
    wrong.write_bytes((ROOT / "src/IMAGE_PIN").read_bytes())
    corrupt = tmp_path / "corrupt.exe"
    altered = bytearray(image)
    altered[0] ^= 1
    corrupt.write_bytes(altered)
    for explicit in (missing, wrong, corrupt):
        monkeypatch.setenv("OPENSMACX_IMAGE", str(explicit))
        with pytest.raises(ValueError, match="explicit OPENSMACX_IMAGE|src/IMAGE_PIN"):
            resolve_pinned_image()

    # Exercise an actual fallback with the outer candidate absent, rather than
    # relying on this checkout happening to have a particular parent depth.
    monkeypatch.delenv("OPENSMACX_IMAGE")
    monkeypatch.setitem(resolve_pinned_image.__globals__, "repository_root",
                        lambda: tmp_path / "absent-outer")
    monkeypatch.setitem(resolve_pinned_image.__globals__, "ROOT", tmp_path)
    monkeypatch.setitem(resolve_pinned_image.__globals__, "image_pin", lambda: IMAGE_SHA256)
    fallback = tmp_path / ".opensmacx/game/terranx_original.exe"
    fallback.parent.mkdir(parents=True)
    fallback.write_bytes(image)
    assert resolve_pinned_image() == (fallback, image)
    fallback.unlink()
    with pytest.raises(ValueError, match="unavailable"):
        resolve_pinned_image()


def test_pinned_member_image_and_independent_primary_and_eh_evidence():
    manifest, _, image_data, archive_data = inputs()
    section = code_section(manifest)
    members = CoffArchive(archive_data)
    member = members.member(MEMBER)
    assert hashlib.sha256(member).hexdigest() == MEMBER_SHA256
    obj = Coff(member)
    physical = obj.sections[section.section - 1]
    assert physical.name == ".text" and len(physical.data) == SECTION_END - ENTRY == 147
    assert physical.flags & 0x20 and physical.flags & 0x20000000
    assert physical.flags == 0x60101020
    assert obj.section_definitions[3] == (1, 0)
    assert 3 not in obj.associations and obj.associations[4] == 3
    assert not obj.sections[3].flags & 0x20
    functions = [symbol for symbol in obj.symbols.values()
                 if symbol.section == section.section and symbol.type & 0x20]
    assert [(symbol.name, symbol.value, symbol.type, symbol.storage) for symbol in functions] == [
        ("__threadstart@4", 0, 0x20, 3)]

    image = PeImage(image_data, IMAGE_SHA256)
    with (ROOT / "docs/recovery/ghidra-functions.csv").open(newline="") as stream:
        ghidra = list(csv.DictReader(stream))
    primary = [row for row in ghidra if row["program_sha256"] == IMAGE_SHA256
               and row["ghidra_version"] == "12.1.2"
               and int(row["address"], 16) == ENTRY]
    assert len(primary) == 1
    assert (int(primary[0]["end_address"], 16), int(primary[0]["size"])) == (PRIMARY_END, 116)
    assert primary[0]["body_ranges"] == "0x00646FD4-0x00647048"
    assert not any(row["program_sha256"] == IMAGE_SHA256
                   and int(row["address"], 16) in (PRIMARY_END, 0x0064705C)
                   for row in ghidra)

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    primary_code = list(decoder.disasm(image.read(Span(ENTRY, PRIMARY_END)), ENTRY))
    assert sum(insn.size for insn in primary_code) == PRIMARY_END - ENTRY
    assert (primary_code[-1].mnemonic, primary_code[-1].address) == ("ret", 0x00647045)
    beginthread = list(decoder.disasm(image.read(Span(0x646F5D, ENTRY)), 0x646F5D))
    assert any((insn.address, insn.mnemonic, insn.op_str) ==
               (0x646F81, "push", "0x646fd4") for insn in beginthread)
    assert any((insn.address, insn.mnemonic, insn.op_str) ==
               (0x646F95, "call", "dword ptr [0x6691c0]") for insn in beginthread)
    create_thread, = [item for item in image.imports()
                      if item.address == 0x6691C0]
    assert (create_thread.module.casefold(), create_thread.name) == ("kernel32.dll", "CreateThread")

    scope = struct.unpack("<III", image.read(Span(SCOPE_TABLE, SCOPE_TABLE + 12)))
    assert scope == (0xFFFFFFFF, 0x00647048, 0x0064705C)
    handler = list(decoder.disasm(image.read(Span(0x647048, 0x64705C)), 0x647048))
    cleanup = list(decoder.disasm(image.read(Span(0x64705C, SECTION_END)), 0x64705C))
    assert sum(insn.size for insn in handler) == 20
    assert sum(insn.size for insn in cleanup) == 11
    assert any(insn.mnemonic == "call" and "0x64b5fc" in insn.op_str for insn in handler)
    assert any(insn.mnemonic == "call" and "0x644e10" in insn.op_str for insn in cleanup)


def test_native_whole_contribution_without_entries_keeps_unknowns():
    manifest, _, image, archive = inputs()
    report = run_check(manifest, image, archive)
    row, = [item for item in report["code_sections"] if item["unit"] == MEMBER]
    assert row["section"] == 3 and row["section_name"] == ".text"
    assert row["span"] == {"start": ENTRY, "end": SECTION_END}
    assert row["boundary"] == "pass" and row["nonrelocation_bytes"] == "pass"
    assert row["highlow_offsets"] == [6, 11, 44, 50, 67]
    assert "entry_inventory" not in row
    assert row["callable"] is False and row["link_target"] is False
    assert row["references"] == "unknown" and row["result"] == "unknown"
    assert row["entries"] == []
    assert not any(item["unit"] == MEMBER and item["section"] == 4
                   for item in report["data_sections"])
    relocation_results = {item["symbol"]: item["result"] for item in row["relocations"]}
    assert relocation_results["__endthread"] == "pass"
    assert relocation_results["__XcptFilter"] == "pass"
    assert relocation_results["__exit"] == "pass"
    assert relocation_results["__except_list"] == "pass"
    assert relocation_results["__amsg_exit"] == "unknown"


@pytest.mark.parametrize("damage", ["wrong_section", "wrong_name", "short_span", "ordinary",
                                     "opcode", "relative_target", "archive"])
def test_threadstart_contribution_damage_never_passes(damage):
    manifest, _, image, archive = inputs()
    section = code_section(manifest)
    if damage == "wrong_section":
        changed = replace(section, section=99)
        manifest = replace(manifest, code_sections=tuple(
            changed if item == section else item for item in manifest.code_sections))
    elif damage == "wrong_name":
        changed = replace(section, section_name=".rdata")
        manifest = replace(manifest, code_sections=tuple(
            changed if item == section else item for item in manifest.code_sections))
    elif damage == "short_span":
        changed = replace(section, span=Span(section.span.start, section.span.end - 1))
        manifest = replace(manifest, code_sections=tuple(
            changed if item == section else item for item in manifest.code_sections))
    elif damage == "ordinary":
        changed = replace(section, comdat_selection=None)
        manifest = replace(manifest, code_sections=tuple(
            changed if item == section else item for item in manifest.code_sections))
    elif damage in ("opcode", "relative_target"):
        changed_image = bytearray(image)
        address = ENTRY if damage == "opcode" else 0x0064702D
        changed_image[raw_offset(image, address)] ^= 1
        image = bytes(changed_image)
        manifest = repin(manifest, image)
    else:
        archive += b"archive digest damage"

    row, = [item for item in run_check(manifest, image, archive)["code_sections"]
            if item["unit"] == MEMBER]
    assert row["result"] != "pass", row
    if damage == "relative_target":
        assert row["references"] == "fail"


def test_private_callback_cannot_be_promoted_to_public_anchor():
    manifest = parse_component(SOURCE.read_text(encoding="utf-8"))
    with pytest.raises(ValueError, match="cannot declare entries"):
        replace(code_section(manifest), entries=(CodeEntryAnchor(
            "__threadstart@4", 0, "private symbol, not an external entry"),))
