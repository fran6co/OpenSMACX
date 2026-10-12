"""Whole CRT iterator ownership; internal funclets are not missing functions."""
from dataclasses import replace
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage, Coff
from match_recovery.coff_archive import CoffArchive
from recovery.environment.wine import VC6_ROOT

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASE = "1a3e78615"
ADDITION = "c30c6facc4dbb2873f47ccf9acc468c32b86774f"
EXPECTED = {
    "??_N@YGXPAXIHP6EX0@Z1@Z": (0x645F8E, 0x646010, "build/intel/mt_obj/ehveccvb.obj", 1,
        "b4530a93f2d6505fb9ebb0146227d79b2f1f5c7bb3eefe4ec25fa9c5513b22cf"),
    "?__ArrayUnwind@@YGXPAXIHP6EX0@Z@Z": (0x645764, 0x6457C2, "build/intel/mt_obj/ehvecdtr.obj", 3,
        "68d1b7904018d3ad112b2ebbe2ad6ccccca8353d7aba29863d02f27e66418dba"),
}


def test_existing_bindings_metadata_and_notes_preserved():
    before = subprocess.check_output(["git", "show", BASE + ":src/vendor/vc6-runtime.recovery.h"], cwd=ROOT, text=True)
    published = subprocess.check_output(["git", "show", f"{ADDITION}:{SOURCE.relative_to(ROOT)}"], cwd=ROOT, text=True)
    old, new = parse_component(before), parse_component(published)
    # Later bindings are separate discoveries, not changes to this step's evidence.
    owned = {b.symbol for b in old.bindings} | set(EXPECTED)
    new = replace(new, bindings=tuple(b for b in new.bindings if b.symbol in owned))
    assert new.bindings[:-2] == old.bindings
    assert replace(new, bindings=old.bindings, code_sections=old.code_sections) == old
    assert {b.symbol: (b.origin.address, b.spans[0].end, b.unit) for b in new.bindings[-2:]} == {
        symbol: value[:3] for symbol, value in EXPECTED.items()}
    installed = parse_component(SOURCE.read_text())
    assert installed.bindings[:len(new.bindings)] == new.bindings
    for delimiter, index in (("// RECOVERY_COMPONENT_BEGIN v1", 0), ("// RECOVERY_COMPONENT_END", 1)):
        assert before.split(delimiter)[index] == published.split(delimiter)[index]


@pytest.fixture(scope="module")
def artifacts():
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    archive = VC6_ROOT / "LIB/LIBCMT.LIB"
    if not image.is_file() or not archive.is_file():
        pytest.skip("pinned original and external runtime archive required")
    return parse_component(SOURCE.read_text()), image.read_bytes(), archive.read_bytes()


def test_physical_members_and_unknown_reference_obligations(artifacts):
    manifest, image, archive = artifacts
    from match_recovery.import_source import read_imports
    report = verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))
    assert report["result"] == "unknown" and not report["ready_for_collection"]
    assert report["application_recoveries"] == 0
    rows = {r["symbol"]: r for r in report["functions"]}
    members = CoffArchive(archive)
    for symbol, (start, end, unit, section, digest) in EXPECTED.items():
        body = members.member(unit)
        assert hashlib.sha256(body).hexdigest() == digest
        obj = Coff(body)
        row = rows[symbol]
        assert row["boundary"] == "pass" and row["result"] == row["references"] == "unknown"
        assert row["differing_offsets"] == [] and row["alignment_fill"] == 0
        assert row["span"] == {"start": start, "end": end} and row["section"] == section
        assert len(obj.sections[section - 1].data) == end - start
        assert any(r.get("symbol") == "__except_handler3" and r["result"] == "unknown" for r in row["relocations"])
        assert any(r.get("binding", {}).get("kind") == "compiler_artifact" and r["result"] == "unknown" for r in row["relocations"])


def test_complete_native_funclets_and_real_array_callers(artifacts):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, data, _ = artifacts
    image = PeImage(data, manifest.bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for start, end, *_ in EXPECTED.values():
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(i.size for i in instructions) == end - start
        assert instructions[-1].mnemonic == "ret" and instructions[-1].address + instructions[-1].size == end
    for site, target in {0x440F43: 0x645F8E, 0x51DF93: 0x645F8E, 0x645FE2: 0x645FF8,
                         0x64600A: 0x645764, 0x64575E: 0x645764, 0x64583E: 0x645764,
                         0x6457A3: 0x648CE8}.items():
        instruction, = decoder.disasm(image.read(Span(site, site + 5)), site)
        assert (instruction.mnemonic, instruction.op_str) == ("call", hex(target))
    diplo = (ROOT / "src/diplowin.cpp").read_text()
    message = (ROOT / "src/messagewin.cpp").read_text()
    assert "StringList DiplomacyTranscriptLists[8][8];" in diplo and "0x00440F30-0x00440F54" in diplo
    assert "StringList MessageWinDeferredBaseMessages[8];" in message and "0x0051DF80-0x0051DFA4" in message


@pytest.mark.parametrize("symbol", EXPECTED)
@pytest.mark.parametrize("damage", ("extent", "omit_funclet", "member", "body"))
def test_damage_never_passes(artifacts, symbol, damage):
    manifest, image, archive = artifacts
    binding, = tuple(b for b in manifest.bindings if b.symbol == symbol)
    if damage == "extent":
        binding = replace(binding, spans=(Span(binding.origin.address, binding.spans[0].end + 1),))
    elif damage == "omit_funclet":
        end = 0x645FF8 if binding.origin.address == 0x645F8E else 0x6457A0
        binding = replace(binding, spans=(Span(binding.origin.address, end),))
    elif damage == "member":
        binding = replace(binding, unit="build/intel/mt_obj/fpctrl.obj")
    else:
        parsed = PeImage(image, binding.origin.image_sha256)
        span, offset, size = next((s, o, n) for s, o, n in parsed.sections if s.start <= binding.origin.address < s.start + n)
        physical = offset + binding.origin.address - span.start
        changed = bytearray(image)
        changed[physical] ^= 1
        image = bytes(changed)
        manifest = replace(manifest, bindings=tuple(
            replace(candidate, origin=Origin(hashlib.sha256(image).hexdigest(), "pe:va", candidate.origin.address))
            for candidate in manifest.bindings), data_sections=tuple(
            replace(candidate, origin=Origin(hashlib.sha256(image).hexdigest(), "pe:va", candidate.origin.address))
            for candidate in manifest.data_sections), code_sections=tuple(
            replace(candidate, origin=Origin(hashlib.sha256(image).hexdigest(), "pe:va", candidate.origin.address))
            for candidate in manifest.code_sections))
        binding = replace(binding, origin=Origin(hashlib.sha256(image).hexdigest(), "pe:va", binding.origin.address))
    report = verify_toolchain(replace(manifest, bindings=(binding,), data_sections=()), image, archive)
    row = report["functions"][0]
    if damage == "member":
        # A missing symbol in the selected member is incomplete evidence,
        # not a measured mismatch; it must never become an attributed pass.
        assert row["result"] == "unknown" and row.get("boundary") != "pass", report
    else:
        assert row["result"] == "fail", report
