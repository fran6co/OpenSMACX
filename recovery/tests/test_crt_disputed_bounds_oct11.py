"""Complete physical CRT functions despite missing/disputed catalogue ends."""
from dataclasses import replace
import csv
import hashlib
import json
import os
from pathlib import Path
import subprocess

import capstone
import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source_inventory import source_snapshot
from match_recovery.component_source import read_components
from test_crt_eight_entry_bindings_oct10 import repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
BASE = "9c76904f56b3853366e7c8cef37c967b65a2e3eb"
COMPANION = "src/vendor/vc6-runtime.recovery.h"
EXPECTED = (
    (0x0064513B, 0x006451B0, "?TranslatorGuardHandler@@YA?AW4_EXCEPTION_DISPOSITION@@PAUEHExceptionRecord@@PAUTranslatorGuardRN@@PAX2@Z", "build/intel/mt_obj/trnsctrl.obj", 12, "f9efd58c6a13efa62b50d2b9a7532bc77d511fc1d629cbf635f4e6cec6916f9c", 0x006451B0, 0x006451A8),
    (0x006453AA, 0x006453D9, "___onexitinit", "build/intel/mt_obj/onexit.obj", 6, "88f861d4dbeb26e450537c284442e69e1eceaab59588633656f7c3becdfc303c", 0x006453D9, None),
    (0x00645844, 0x0064584D, "__purecall", "build/intel/mt_obj/purevirt.obj", 1, "3dc99649b19b6dc8631d722cc39bae5cd7ae6d1a3ce9a731d75041fe23da6c26", 0x0064584B, None),
    (0x00647B7B, 0x00647C23, "___initstdio", "build/intel/mt_obj/_file.obj", 5, "cde568d9abdcb6cbb7be79d229deb579e504d9ce01b780076fa9ecba85d37bcf", 0x00647C23, None),
    (0x00647C23, 0x00647C37, "___endstdio", "build/intel/mt_obj/_file.obj", 7, "cde568d9abdcb6cbb7be79d229deb579e504d9ce01b780076fa9ecba85d37bcf", 0x00647C37, None),
    (0x00648E94, 0x00648EBE, "?ExFilterRethrow@@YAHPAU_EXCEPTION_POINTERS@@@Z", "build/intel/mt_obj/frame.obj", 14, "2b87b6fa3f780e75aa5c34ed7a8091416460a13053c1ebff2593df350396c934", 0x00648EBE, None),
    (0x00649913, 0x00649961, "__cropzeros", "../build/intel/mt_obj/cvt.obj", 3, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2", 0x00649961, None),
    (0x00649961, 0x00649979, "__positive", "../build/intel/mt_obj/cvt.obj", 5, "b222cce7bc13671f976e1a77a876d39399b1225cdac96d67740167407358b7c2", 0x00649979, None),
    (0x0064CAB3, 0x0064CB34, "__fcloseall", "build/intel/mt_obj/closeall.obj", 1, "917f9c40727e038d7cf1a82886d5473261aea4065632f0454eda8b6194a0dd6f", 0x0064CB34, None),
    (0x0064CB7A, 0x0064CB8B, "?__CxxSetUnhandledExceptionFilter@@YAXXZ", "build/intel/mt_obj/unhandld.obj", 6, "198280f1f19e4350f7d28346a8a66eed0b063dd4adec776044ae3fa3c723e6b4", 0x0064CB8B, None),
    (0x0064CB8B, 0x0064CB98, "?__CxxRestoreUnhandledExceptionFilter@@YAXXZ", "build/intel/mt_obj/unhandld.obj", 8, "198280f1f19e4350f7d28346a8a66eed0b063dd4adec776044ae3fa3c723e6b4", 0x0064CB98, None),
    (0x0064D934, 0x0064D93D, "__fptrap", "build/intel/mt_obj/crt0fp.obj", 1, "7d3c40aa8e692436c99d2a7d669457230ffb1205f8c018523611f10e728f8537", 0x0064D93B, 0x0064D93D),
)
ADDRESSES = {x[0] for x in EXPECTED}


def prior():
    return subprocess.check_output(["git", "show", BASE + ":" + COMPANION],
                                   cwd=ROOT, text=True)


def test_only_twelve_complete_bindings_added():
    old = prior()
    text = (ROOT / COMPANION).read_text()
    before, after = parse_component(old), parse_component(text)
    assert len(before.bindings) == 232 and len(after.bindings) == 244
    assert replace(after, bindings=after.bindings[:232]) == before
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit)
            for b in after.bindings[232:]] == [x[:4] for x in EXPECTED]
    lines = [l for l in text.splitlines() if l.startswith("//     {") and '"unit"' in l
             and int(json.loads(l[3:].rstrip(","))["origin"], 16) in ADDRESSES]
    assert len(lines) == 12
    for line in lines:
        assert "Catalogue disagreement" in line
        text = text.replace(line + "\n", "", 1)
    last = next(l for l in reversed(old.splitlines()) if '"unit"' in l and '"origin"' in l)
    assert text.replace(last + ",\n", last + "\n", 1) == old


@pytest.fixture(scope="module")
def inputs():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if "OPENSMACX_IMAGE" in os.environ:
        assert path.is_file(), "explicit pinned image unavailable"
    if not path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned image/runtime archive unavailable")
    manifest = parse_component((ROOT / COMPANION).read_text())
    image, archive = path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == manifest.archive_sha256
    return manifest, image, archive, read_imports(ROOT), path


def verify(inputs, manifest=None, image=None):
    m, i, archive, imports, _ = inputs
    return verify_toolchain(m if manifest is None else manifest,
                            i if image is None else image, archive,
                            import_bindings=imports)


def row(report, address):
    return next(r for r in report["functions"] if int(r["address"], 16) == address)


def test_physical_extent_catalogue_conflicts_and_no_owned_overlap(inputs):
    manifest, image, archive, _, path = inputs
    pe = PeImage(image, hashlib.sha256(image).hexdigest())
    members = CoffArchive(archive)
    ida_path = Path(os.environ.get("OPENSMACX_IDA_CATALOG", path.parent / "ida94-functions.csv"))
    with ida_path.open() as f:
        ida = {int(r["address"], 16): int(r["end"], 16) for r in csv.DictReader(f)}
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as f:
        gh_rows = list(csv.DictReader(f))
    assert gh_rows and all(r["program_sha256"] == hashlib.sha256(image).hexdigest()
                           and r["ghidra_version"] == "12.1.2" for r in gh_rows)
    gh = {int(r["address"], 16): int(r["end_address"], 16) for r in gh_rows}
    spans = [tuple(s) for r in source_snapshot(ROOT)["records"] for s in r.get("spans", [])]
    for component in read_components(ROOT / "src"):
        for binding in component.bindings:
            if binding.origin.address not in ADDRESSES:
                spans.extend((s.start, s.end) for s in binding.spans)
        if hasattr(component, "data_sections"):
            spans.extend((d.span.start, d.span.end) for d in component.data_sections)
    disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    disassembler.detail = True
    indirect_jumps = []
    for address, end, symbol, unit, section, digest, ida_end, gh_end in EXPECTED:
        assert ida[address] == ida_end and gh.get(address) == gh_end
        assert ida_end != gh_end
        assert not any(a < end and address < b for a, b in spans)
        data = members.member(unit)
        assert hashlib.sha256(data).hexdigest() == digest
        obj = Coff(data)
        function = obj.function(symbol, comdat=False)
        assert function.section == section and function.value == 0
        code, _, _ = obj.subject(function)
        assert len(code) == end - address
        native = list(disassembler.disasm(pe.read(Span(address, end)), address))
        assert native and sum(i.size for i in native) == end - address
        assert native[-1].address + native[-1].size == end
        assert native[-1].mnemonic in ("ret", "jmp")
        # Direct branch-target scope, not complete reachability/CFG proof.
        # The translator guard also restores an indirect continuation. Its
        # instruction is verified, but its dynamic target remains unproved.
        for instruction in native:
            if instruction.group(capstone.CS_GRP_JUMP):
                operand = instruction.operands[0]
                if operand.type != capstone.x86_const.X86_OP_IMM:
                    assert instruction.mnemonic == "jmp"
                    assert operand.type == capstone.x86_const.X86_OP_MEM
                    indirect_jumps.append((address, instruction.address,
                                           operand.mem.base, operand.mem.index,
                                           operand.mem.disp))
                    continue
                target = instruction.operands[0].imm
                if not address <= target < end:
                    assert instruction.mnemonic == "jmp"
                    field = instruction.address - address + instruction.imm_offset
                    relocation = next((i for off, i, kind in obj.sections[section - 1].relocations
                                       if off == field and kind == 20), None)
                    assert relocation is not None
                    name = obj.symbols[relocation].name
                    binding = next(b for b in manifest.bindings if b.symbol == name)
                    addend = int.from_bytes(code[field:field + 4], "little", signed=True)
                    assert target == binding.origin.address + addend
        assert pe.absolute_sites(Span(address, end)) == frozenset(
            offset for offset, _, kind in obj.sections[section - 1].relocations if kind == 6)
    assert indirect_jumps == [(0x64513B, 0x6451A5,
                               capstone.x86_const.X86_REG_EBX, 0, 0x18)]


def test_current_functions_and_prior_references_preserved(inputs):
    before = verify(inputs, parse_component(prior()))
    after = verify(inputs)
    assert len(after["functions"]) == 244
    for address in ADDRESSES:
        r = row(after, address)
        assert r["boundary"] == "pass" and not r["differing_offsets"]
        assert r["result"] == ("pass" if address in (0x64513B, 0x648E94) else "unknown")
        assert all(x["result"] in ("pass", "unknown") for x in r["relocations"])
    for old in before["functions"]:
        current = row(after, int(old["address"], 16))
        if old == current:
            continue
        assert current["result"] != "fail"
        assert current["boundary"] == old["boundary"]
        assert current["differing_offsets"] == old["differing_offsets"]
        if old["result"] == "pass":
            assert current["result"] == "pass"
        assert len(current["relocations"]) == len(old["relocations"])
        for a, b in zip(old["relocations"], current["relocations"]):
            assert a["symbol"] == b["symbol"]
            if a != b:
                assert a["result"] == "unknown" and b["result"] == "pass"
                assert a["symbol"] in {x[2] for x in EXPECTED}
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


@pytest.mark.parametrize("address,end,symbol,unit,section,digest,ida_end,gh_end", EXPECTED)
def test_body_damage_detected(inputs, address, end, symbol, unit, section, digest, ida_end, gh_end):
    manifest, image, _, _, _ = inputs
    damaged = bytearray(image)
    damaged[raw_offset(image, address)] ^= 1
    damaged = bytes(damaged)
    assert row(verify(inputs, repin(manifest, damaged), damaged), address)["result"] == "fail"


@pytest.mark.parametrize("address,end,symbol,unit,section,digest,ida_end,gh_end", EXPECTED)
def test_truncated_extent_refused(inputs, address, end, symbol, unit, section, digest, ida_end, gh_end):
    manifest, _, _, _, _ = inputs
    for truncated_end in {end - 1, ida_end, gh_end} - {end, None}:
        assert truncated_end > address
        changed = replace(manifest, bindings=tuple(
            replace(b, spans=(Span(address, truncated_end),)) if b.origin.address == address else b
            for b in manifest.bindings))
        r = row(verify(inputs, changed), address)
        assert r["boundary"] != "pass" and r["result"] != "pass"


def test_known_reference_damage_detected(inputs):
    manifest, image, archive, _, _ = inputs
    # Damage a known named reference in its actual owning function. This is a
    # relocation-field control, separate from the nonrelocation opcode control.
    owner = next(b for b in manifest.bindings if b.origin.address in ADDRESSES
                 and b.symbol.startswith("?TranslatorGuardHandler"))
    obj = Coff(CoffArchive(archive).member(owner.unit))
    fn = obj.function(owner.symbol, comdat=False)
    offset = next(off for off, index, kind in obj.sections[fn.section - 1].relocations
                  if kind == 20)
    damaged = bytearray(image)
    damaged[raw_offset(image, owner.origin.address + offset)] ^= 1
    damaged = bytes(damaged)
    r = row(verify(inputs, repin(manifest, damaged), damaged), owner.origin.address)
    assert r["result"] == "fail" and r["references"] == "fail"

