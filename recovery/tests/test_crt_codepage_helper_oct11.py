"""One self-contained CRT codepage conversion leaf; no provider-selection claim."""
from collections import Counter
from dataclasses import replace
from pathlib import Path
import csv
import hashlib
import json
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASE = "af91b787101b2eaf99f215797e2aa1289a038e0d"
ADDITION = "db9aaeb81629d511fb9c60f80e9bcb83d76e7cfe"
PINNED_IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PINNED_ARCHIVE_SHA256 = "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b"
UNIT = "build/intel/mt_obj/mbctype.obj"
MEMBER_SHA256 = "74924cfef4395b03a742c0dbcbff475c3ed322e1853cedec84c8e28a48a0874e"
EXPECTED = (0x0064EE16, 0x0064EE49, "_CPtoLCID", UNIT, 7)


def snapshot(revision):
    text = subprocess.check_output(
        ["git", "show", f"{revision}:src/vendor/vc6-runtime.recovery.h"], cwd=ROOT, text=True)
    return parse_component(text), text


def assert_installed_history(installed, historical):
    identity = lambda b: (b.origin.address, b.symbol)
    current = {identity(b): b for b in installed.bindings}
    assert len(current) == len(installed.bindings)
    assert all(current.get(identity(b)) == b for b in historical.bindings)
    assert replace(installed, bindings=historical.bindings,
                   code_sections=historical.code_sections) == historical


def without_codepage_helper(manifest):
    """Remove exactly the checked addition for older immutable projections."""
    start, end, symbol, unit, _ = EXPECTED
    additions = tuple(b for b in manifest.bindings if b.symbol == symbol)
    if not additions:
        return manifest
    assert len(additions) == 1
    binding, = additions
    assert (binding.origin.address, binding.spans, binding.symbol, binding.unit) == (
        start, (Span(start, end),), symbol, unit)
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.symbol != symbol))


def test_exact_component_addition_preserves_existing_history_and_records():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields

    old, prior = snapshot(BASE)
    new, step_text = snapshot(ADDITION)
    current = SOURCE.read_text(encoding="utf-8")
    installed = parse_component(current)
    assert_installed_history(installed, new)
    assert sum(b.symbol == EXPECTED[2] for b in new.bindings) == 1
    assert without_codepage_helper(new) == old
    assert parse_component(render_component(new)) == new
    addition, = (b for b in new.bindings if b.symbol == EXPECTED[2])
    assert addition.origin.address == EXPECTED[0]
    assert (addition.spans, addition.symbol, addition.unit) == (
        (Span(EXPECTED[0], EXPECTED[1]),), EXPECTED[2], EXPECTED[3])
    for delimiter, index in (("// RECOVERY_COMPONENT_BEGIN v1", 0), ("// RECOVERY_COMPONENT_END", 1)):
        assert prior.split(delimiter)[index] == step_text.split(delimiter)[index]
    old_records = {(r.address, r.symbol): (r.address, _continuity_fields(r), r.symbol)
                   for r in read_text(prior, SOURCE)}
    installed_records = {(r.address, r.symbol): (r.address, _continuity_fields(r), r.symbol)
                         for r in read_text(current, SOURCE)}
    assert all(installed_records.get(key) == record for key, record in old_records.items())


def test_projection_is_idempotent_and_rejects_a_different_binding():
    manifest = parse_component(SOURCE.read_text(encoding="utf-8"))
    binding, = (b for b in manifest.bindings if b.symbol == EXPECTED[2])
    projected = without_codepage_helper(manifest)
    assert without_codepage_helper(projected) == projected
    for changed in (replace(binding, unit="wrong.obj"),
                    replace(binding, spans=(Span(EXPECTED[0], EXPECTED[1]-1),)),
                    replace(binding,
                            origin=replace(binding.origin, address=EXPECTED[0] + 1),
                            spans=(Span(EXPECTED[0] + 1, EXPECTED[1]),))):
        damaged = replace(manifest, bindings=tuple(
            changed if b.symbol == EXPECTED[2] else b for b in manifest.bindings))
        with pytest.raises(AssertionError):
            without_codepage_helper(damaged)


@pytest.fixture(scope="module")
def artifacts():
    project = json.loads((ROOT / "recovery/project.json").read_text(encoding="utf-8"))
    archive = Path(project["components"]["toolchain_archives"]["vc6-runtime"])
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image.is_file() or not archive.is_file():
        pytest.skip("pinned original and external runtime archive required")
    installed = parse_component(SOURCE.read_text(encoding="utf-8"))
    historical, _ = snapshot(ADDITION)
    assert_installed_history(installed, historical)
    return historical, image.read_bytes(), archive.read_bytes()


def test_strict_leaf_pass_and_existing_verdicts_unchanged(artifacts):
    manifest, image, archive = artifacts
    assert hashlib.sha256(image).hexdigest() == PINNED_IMAGE_SHA256
    assert hashlib.sha256(archive).hexdigest() == PINNED_ARCHIVE_SHA256
    imports = read_imports(ROOT)
    before = verify_toolchain(without_codepage_helper(manifest), image, archive,
                              import_bindings=imports)
    after = verify_toolchain(manifest, image, archive, import_bindings=imports)
    before_rows = {(r["address"], r["symbol"]): r for r in before["functions"]}
    after_rows = {(r["address"], r["symbol"]): r for r in after["functions"]}
    assert all(after_rows.get(key) == row for key, row in before_rows.items())
    assert Counter(r["result"] for r in before["functions"]) == {"pass": 67, "unknown": 26}
    assert Counter(r["result"] for r in after["functions"]) == {"pass": 68, "unknown": 26}
    row = next(r for r in after["functions"] if r["symbol"] == EXPECTED[2])
    assert row["result"] == row["boundary"] == row["bytes"] == row["references"] == "pass", row
    assert row["span"] == {"start": EXPECTED[0], "end": EXPECTED[1]}
    assert row["symbol"] == EXPECTED[2] and row["unit"] == UNIT and row["section"] == EXPECTED[4]
    assert row["alignment_fill"] == 0 and row["relocations"] == []
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


def test_independent_bounds_physical_contribution_and_original_callers(artifacts):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    manifest, image_data, archive_data = artifacts
    start, end, symbol, unit, section = EXPECTED
    ida = ROOT / ".opensmacx/game/ida94-functions.csv"
    ghidra = ROOT / "docs/recovery/ghidra-functions.csv"
    ida_rows = {int(r["address"], 16): int(r["end"], 16)
                for r in csv.DictReader(ida.open())}
    pin = manifest.bindings[0].origin.image_sha256
    ghidra_rows = list(csv.DictReader(ghidra.open()))
    assert ida_rows[start] == end
    assert any(r["program_sha256"] == pin and r["ghidra_version"] == "12.1.2"
               and r["address"] == f"0x{start:08X}"
               and r["end_address"] == f"0x{end:08X}"
               and r["body_ranges"] == f"0x{start:08X}-0x{end:08X}"
               and int(r["size"]) == end-start for r in ghidra_rows)

    member = CoffArchive(archive_data).member(unit)
    assert hashlib.sha256(member).hexdigest() == MEMBER_SHA256
    obj = Coff(member)
    selected = [s for s in obj.symbols.values() if s.name == symbol]
    assert len(selected) == 1
    selected, = selected
    assert (selected.value, selected.section, selected.storage, selected.type) == (0, section, 3, 32)
    contribution = obj.sections[section-1]
    assert contribution.name == ".text" and len(contribution.data) == end-start
    assert len(contribution.relocations) == 0
    assert [(s.name, s.value, s.storage) for s in obj.symbols.values()
            if s.section == section and s.index not in obj.auxiliary] == [(symbol, 0, 3)]

    image = PeImage(image_data, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    body = list(decoder.disasm(image.read(Span(start, end)), start))
    assert sum(insn.size for insn in body) == end-start and body[-1].mnemonic == "ret"
    callers = []
    for address in (0x0064ED43, 0x0064ED7C):
        insn, = list(decoder.disasm(image.read(Span(address, address+5)), address))
        assert (insn.mnemonic, insn.op_str) == ("call", "0x64ee16")
        callers.append(insn.address)
    assert callers == [0x0064ED43, 0x0064ED7C]


@pytest.mark.parametrize("damage", ("extent", "body"))
def test_damaged_codepage_leaf_cannot_pass(artifacts, damage):
    manifest, image, archive = artifacts
    binding = next(b for b in manifest.bindings if b.symbol == EXPECTED[2])
    if damage == "extent":
        manifest = replace(manifest, bindings=tuple(
            replace(b, spans=(Span(b.origin.address, b.spans[0].end-1),))
            if b.symbol == EXPECTED[2] else b for b in manifest.bindings))
    else:
        parsed = PeImage(image, binding.origin.image_sha256)
        section, offset, _ = next((s, o, n) for s, o, n in parsed.sections
                                  if s.start <= binding.origin.address < s.start+n)
        changed = bytearray(image)
        changed[offset + binding.origin.address-section.start] ^= 1
        image = bytes(changed)
        pin = hashlib.sha256(image).hexdigest()
        manifest = replace(manifest,
            bindings=tuple(replace(b, origin=Origin(pin, "pe:va", b.origin.address))
                           for b in manifest.bindings),
            data_sections=tuple(replace(d, origin=Origin(pin, "pe:va", d.origin.address))
                                for d in manifest.data_sections),
            code_sections=tuple(replace(section, origin=Origin(pin, "pe:va", section.origin.address))
                                for section in manifest.code_sections))
    result = verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))
    row = next(r for r in result["functions"] if r["symbol"] == EXPECTED[2])
    assert row["result"] != "pass", row
