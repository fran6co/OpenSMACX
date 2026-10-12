"""Two bounded archive helpers, not complete exception/runtime acceptance."""
from collections import Counter
from dataclasses import replace
from pathlib import Path
import csv
import hashlib
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from recovery.environment.wine import VC6_ROOT
from test_crt_codepage_helper_oct11 import without_codepage_helper

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASE = "de298dcd28b4b42598aaa47a12e3e612b9767169"
ADDITION = "eb5295280a402f3b89157f79878ffdca01e4eb3a"
UNIT = "build/intel/mt_obj/frame.obj"
EXPECTED = (
    (0x648BED, 0x648C4A, "?TypeMatch@@YAHPBU_s_HandlerType@@PBU_s_CatchableType@@PBU_s_ThrowInfo@@@Z", UNIT, 5),
    (0x6490E9, 0x64910C, "?AdjustPointer@@YAPAXPAXABUPMD@@@Z", UNIT, 20),
)


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


def without_exception_helpers(manifest):
    """Project later additions in order, preserving this older baseline."""
    manifest = without_codepage_helper(manifest)
    symbols = {r[2] for r in EXPECTED}
    additions = tuple(b for b in manifest.bindings if b.symbol in symbols)
    if not additions:
        return manifest
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in additions] == [r[:4] for r in EXPECTED]
    assert all(b.spans == (Span(a, e),) for b, (a, e, *_) in zip(additions, EXPECTED))
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.symbol not in symbols))


def test_projection_rejects_same_symbol_at_wrong_origin():
    manifest, _ = snapshot(ADDITION)
    target = next(b for b in manifest.bindings if b.symbol == EXPECTED[0][2])
    address = target.origin.address + 1
    wrong = replace(target, origin=replace(target.origin, address=address),
                    spans=(Span(address, target.spans[0].end),))
    damaged = replace(manifest, bindings=tuple(wrong if b == target else b for b in manifest.bindings))
    with pytest.raises(AssertionError):
        without_exception_helpers(damaged)


def test_exact_two_binding_addition_preserves_old_source_obligations():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    old, prior = snapshot(BASE)
    new, step_text = snapshot(ADDITION)
    current = SOURCE.read_text()
    installed = parse_component(current)
    assert_installed_history(installed, new)
    assert without_exception_helpers(new) == old
    pair = [b for b in new.bindings if b.symbol in {r[2] for r in EXPECTED}]
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in pair] == [r[:4] for r in EXPECTED]
    for delimiter, index in (("// RECOVERY_COMPONENT_BEGIN v1", 0), ("// RECOVERY_COMPONENT_END", 1)):
        assert prior.split(delimiter)[index] == step_text.split(delimiter)[index]
    old_records = {(r.address, r.symbol): (r.address, _continuity_fields(r), r.symbol)
                   for r in read_text(prior, SOURCE)}
    installed_records = {(r.address, r.symbol): (r.address, _continuity_fields(r), r.symbol)
                         for r in read_text(current, SOURCE)}
    assert all(installed_records.get(key) == record for key, record in old_records.items())


@pytest.fixture(scope="module")
def artifacts():
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    archive = VC6_ROOT / "LIB/LIBCMT.LIB"
    if not image.is_file() or not archive.is_file():
        pytest.skip("pinned original and external runtime archive required")
    installed = parse_component(SOURCE.read_text())
    historical, _ = snapshot(ADDITION)
    assert_installed_history(installed, historical)
    return historical, image.read_bytes(), archive.read_bytes()


def test_strict_complete_pair_and_unchanged_old_verdicts(artifacts):
    manifest, image, archive = artifacts
    imports = read_imports(ROOT)
    prior, _ = snapshot(BASE)
    before = verify_toolchain(prior, image, archive, import_bindings=imports)
    after = verify_toolchain(manifest, image, archive, import_bindings=imports)
    before_rows = {(r["address"], r["symbol"]): r for r in before["functions"]}
    after_rows = {(r["address"], r["symbol"]): r for r in after["functions"]}
    assert all(after_rows.get(key) == row for key, row in before_rows.items())
    assert Counter(r["result"] for r in before["functions"]) == {"pass": 65, "unknown": 26}
    assert Counter(r["result"] for r in after["functions"]) == {"pass": 67, "unknown": 26}
    for start, end, symbol, unit, section in EXPECTED:
        row = next(r for r in after["functions"] if r["symbol"] == symbol)
        assert row["boundary"] == row["bytes"] == row["references"] == row["result"] == "pass", row
        assert row["span"] == {"start": start, "end": end}
        assert row["symbol"] == symbol and row["unit"] == unit and row["section"] == section
        assert row["alignment_fill"] == 0
    assert after["result"] == "unknown" and after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


def test_independent_bounds_physical_exclusive_sections_and_native_calls(artifacts):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, image_data, archive_data = artifacts
    ida, ghidra = ROOT / ".opensmacx/game/ida94-functions.csv", ROOT / "docs/recovery/ghidra-functions.csv"
    if not ida.is_file() or not ghidra.is_file():
        pytest.skip("independent catalogues required")
    intervals = {int(r["address"], 16): int(r["end"], 16) for r in csv.DictReader(ida.open())}
    ghidra_rows = list(csv.DictReader(ghidra.open()))
    pin = manifest.bindings[0].origin.image_sha256
    member = CoffArchive(archive_data).member(UNIT)
    assert hashlib.sha256(member).hexdigest() == "2b87b6fa3f780e75aa5c34ed7a8091416460a13053c1ebff2593df350396c934"
    obj = Coff(member)
    image = PeImage(image_data, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for start, end, symbol, _, section in EXPECTED:
        assert intervals[start] == end
        assert any(r["program_sha256"] == pin and r["ghidra_version"] == "12.1.2"
                   and r["address"] == f"0x{start:08X}" and r["end_address"] == f"0x{end:08X}"
                   and r["body_ranges"] == f"0x{start:08X}-0x{end:08X}" and int(r["size"]) == end-start
                   for r in ghidra_rows)
        selected = [s for s in obj.symbols.values() if s.name == symbol]
        assert len(selected) == 1
        selected, = selected
        assert (selected.value, selected.section, selected.storage, selected.type) == (0, section, 3, 32)
        contribution = obj.sections[section-1]
        assert len(contribution.data) == end-start
        insns = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(i.size for i in insns) == end-start and insns[-1].mnemonic == "ret"
        if section == 5:
            assert len(contribution.relocations) == 1
            offset, target, kind = contribution.relocations[0]
            assert (offset, kind, obj.symbols[target].name) == (39, 20, "_strcmp")
            assert [(i.mnemonic, i.op_str) for i in insns if i.mnemonic == "call"] == [("call", "0x645660")]
        else:
            assert not contribution.relocations
    for address, target in ((0x648AAB, 0x648BED), (0x648F4D, 0x6490E9),
                            (0x648FEA, 0x6490E9), (0x649031, 0x6490E9), (0x64904B, 0x6490E9)):
        instruction = next(decoder.disasm(image.read(Span(address, address+5)), address))
        assert (instruction.mnemonic, instruction.op_str) == ("call", hex(target))


@pytest.mark.parametrize("symbol", [r[2] for r in EXPECTED])
@pytest.mark.parametrize("damage", ("extent", "body", "reference"))
def test_damaged_helper_cannot_pass(artifacts, symbol, damage):
    manifest, image, archive = artifacts
    binding = next(b for b in manifest.bindings if b.symbol == symbol)
    if damage == "extent":
        manifest = replace(manifest, bindings=tuple(
            replace(b, spans=(Span(b.origin.address, b.spans[0].end-1),)) if b.symbol == symbol else b
            for b in manifest.bindings))
    else:
        if damage == "reference" and symbol == EXPECTED[1][2]:
            pytest.skip("pointer-adjustment leaf has no reference to damage")
        parsed = PeImage(image, binding.origin.image_sha256)
        section, offset, _ = next((s, o, n) for s, o, n in parsed.sections
                                 if s.start <= binding.origin.address < s.start+n)
        changed = bytearray(image)
        at = offset + binding.origin.address-section.start
        changed[at + (39 if damage == "reference" else 0)] ^= 1
        if damage == "reference":
            original_body = bytearray(parsed.read(binding.spans[0]))
            changed_body = changed[at:at + binding.spans[0].end-binding.origin.address]
            assert original_body != changed_body
            # A displacement-masked lens hides precisely this changed target;
            # the native strict comparator relocates and checks it instead.
            original_body[39:43] = changed_body[39:43] = bytes(4)
            assert original_body == changed_body
        image = bytes(changed)
        pin = hashlib.sha256(image).hexdigest()
        manifest = replace(manifest,
            bindings=tuple(replace(b, origin=Origin(pin, "pe:va", b.origin.address)) for b in manifest.bindings),
            data_sections=tuple(replace(d, origin=Origin(pin, "pe:va", d.origin.address)) for d in manifest.data_sections),
            code_sections=tuple(replace(section, origin=Origin(pin, "pe:va", section.origin.address))
                                for section in manifest.code_sections))
    result = verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))
    row = next(r for r in result["functions"] if r["symbol"] == symbol)
    assert row["result"] != "pass", row
    if damage == "reference":
        assert row["bytes"] == row["references"] == "fail", row
