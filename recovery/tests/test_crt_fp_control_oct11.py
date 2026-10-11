"""Bounded FP-control dependency correspondence, not application recovery."""
from collections import Counter
from dataclasses import replace
from pathlib import Path
import csv
import hashlib
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from recovery.environment.wine import VC6_ROOT

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASE = "25e0aa110c85c051a66f91f7bdbbb6aa90820aa1"
EXPECTED = (
    (0x649840, 0x649852, "__setdefaultprecision", "build/intel/mt_obj/fp8.obj", 1),
    (0x64D259, 0x64D28E, "__control87", "../build/intel/mt_obj/ieee87.obj", 3),
    (0x64D28E, 0x64D2A4, "__controlfp", "../build/intel/mt_obj/ieee87.obj", 4),
    (0x64D2A4, 0x64D336, "__abstract_cw", "../build/intel/mt_obj/ieee87.obj", 8),
    (0x64D336, 0x64D3BF, "__hw_cw", "../build/intel/mt_obj/ieee87.obj", 10),
)


def without_fp_control(manifest):
    """Remove only this fully identified addition from historical controls."""
    owned = {row[2] for row in EXPECTED}
    added = tuple(b for b in manifest.bindings if b.symbol in owned)
    if not added:
        return manifest
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in added] == [r[:4] for r in EXPECTED]
    assert all(b.spans == (Span(a, e),) for b, (a, e, *_) in zip(added, EXPECTED))
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.symbol not in owned))


def previous():
    return subprocess.check_output(["git", "show", BASE + ":src/vendor/vc6-runtime.recovery.h"], cwd=ROOT, text=True)


def test_only_five_complete_bindings_added():
    before, after = previous(), SOURCE.read_text()
    old, current = parse_component(before), parse_component(after)
    assert without_fp_control(current) == old
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in current.bindings[-5:]] == [r[:4] for r in EXPECTED]
    assert all("not original archive selection" in b.evidence for b in current.bindings[-5:])
    for delimiter, index in (("// RECOVERY_COMPONENT_BEGIN v1", 0), ("// RECOVERY_COMPONENT_END", 1)):
        assert before.split(delimiter)[index] == after.split(delimiter)[index]


@pytest.fixture(scope="module")
def artifacts():
    image, archive = ROOT / ".opensmacx/game/terranx_original.exe", VC6_ROOT / "LIB/LIBCMT.LIB"
    if not image.is_file() or not archive.is_file():
        pytest.skip("pinned original and external runtime archive required")
    return parse_component(SOURCE.read_text()), image.read_bytes(), archive.read_bytes()


def test_strict_closure_and_all_previous_verdicts(artifacts):
    manifest, image, archive = artifacts
    imports = read_imports(ROOT)
    before = verify_toolchain(without_fp_control(manifest), image, archive, import_bindings=imports)
    after = verify_toolchain(manifest, image, archive, import_bindings=imports)
    assert before["functions"] == after["functions"][:-5]
    assert Counter(r["result"] for r in before["functions"]) == {"pass": 60, "unknown": 26}
    assert Counter(r["result"] for r in after["functions"]) == {"pass": 65, "unknown": 26}
    for row, (start, end, symbol, unit, section) in zip(after["functions"][-5:], EXPECTED):
        assert row["result"] == row["boundary"] == row["bytes"] == row["references"] == "pass", row
        assert row["span"] == {"start": start, "end": end}
        assert row["symbol"] == symbol and row["unit"] == unit and row["section"] == section
        assert row["alignment_fill"] == 0
    assert after["result"] == "unknown" and after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


def test_independent_bounds_and_native_call_closure(artifacts):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, data, _ = artifacts
    ida = ROOT / ".opensmacx/game/ida94-functions.csv"
    ghidra = ROOT / "docs/recovery/ghidra-functions.csv"
    if not ida.is_file() or not ghidra.is_file():
        pytest.skip("independent catalogues required")
    with ida.open() as stream:
        intervals = {int(r["address"], 16): int(r["end"], 16) for r in csv.DictReader(stream)}
    with ghidra.open() as stream:
        rows = list(csv.DictReader(stream))
    pin = manifest.bindings[0].origin.image_sha256
    for start, end, *_ in EXPECTED:
        assert intervals[start] == end
        assert any(r["program_sha256"] == pin and r["ghidra_version"] == "12.1.2"
                   and r["address"] == f"0x{start:08X}" and r["end_address"] == f"0x{end:08X}"
                   and r["body_ranges"] == f"0x{start:08X}-0x{end:08X}" and int(r["size"]) == end-start
                   for r in rows)
    image = PeImage(data, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    calls = []
    for start, end, *_ in EXPECTED:
        insns = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(i.size for i in insns) == end-start
        assert insns[-1].mnemonic == "ret"
        calls.extend((i.address, i.operands[0].imm) for i in insns if i.mnemonic == "call")
    assert calls == [(0x64984A, 0x64D28E), (0x64D265, 0x64D2A4),
                     (0x64D27C, 0x64D336), (0x64D29C, 0x64D259)]


@pytest.mark.parametrize("symbol", [r[2] for r in EXPECTED])
@pytest.mark.parametrize("damage", ("extent", "body"))
def test_damaged_correspondence_refuses(artifacts, symbol, damage):
    manifest, image, archive = artifacts
    target = next(b for b in manifest.bindings if b.symbol == symbol)
    if damage == "extent":
        bindings = tuple(replace(b, spans=(Span(b.origin.address, b.spans[0].end-1),))
                         if b.symbol == symbol else b for b in manifest.bindings)
        manifest = replace(manifest, bindings=bindings)
    else:
        parsed = PeImage(image, target.origin.image_sha256)
        section, offset, _ = next((s, o, n) for s, o, n in parsed.sections
                                 if s.start <= target.origin.address < s.start+n)
        changed = bytearray(image)
        changed[offset+target.origin.address-section.start] ^= 1
        image = bytes(changed)
        pin = hashlib.sha256(image).hexdigest()
        manifest = replace(manifest,
                           bindings=tuple(replace(b, origin=Origin(pin, "pe:va", b.origin.address)) for b in manifest.bindings),
                           data_sections=tuple(replace(d, origin=Origin(pin, "pe:va", d.origin.address)) for d in manifest.data_sections))
    result = verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))
    row = next(r for r in result["functions"] if r["symbol"] == symbol)
    assert row["result"] != "pass", row
