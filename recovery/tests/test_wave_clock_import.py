"""Real clock import operands, not full audio/game or DLL-version acceptance."""

from dataclasses import replace
import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM, X86_OP_REG

from match_recovery.compiler import Compiler
from match_recovery.import_source import import_targets, read_imports
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, symbol_at


CLOCK = "__imp__timeGetTime@0"
CONSUMERS = (("src/wave.cpp", 0x4C6B10, False),
             ("src/wave.cpp", 0x4C6920, False),
             ("src/popup.cpp", 0x4456B0, False),
             ("src/game.cpp", 0x58F2F0, True),
             ("src/ambience.cpp", 0x447360, True))


def decode(code, base=0):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return list(decoder.disasm(code, base))


@pytest.fixture(scope="module")
def clock_objects():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned original image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    trials = [{unit: Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
               for unit in sorted({unit for unit, _, _ in CONSUMERS})}
              for flags in settings.flag_sets]
    return compiler, settings, trials, PeImage(path.read_bytes(), image_pin(ROOT))


def assert_two_calls_through_loaded_register(ops, load):
    assert load.mnemonic == "mov"
    assert load.operands[0].type == X86_OP_REG and load.operands[1].type == X86_OP_MEM
    assert not load.operands[1].mem.base and not load.operands[1].mem.index
    register = load.operands[0].reg
    # These original/candidate sites retain one callee-saved pointer, not an
    # arbitrary dataflow proof. Refuse a reload or overwrite before both calls.
    assert load.reg_name(register) in ("ebx", "esi", "edi", "ebp")
    calls = 0
    for op in ops:
        if op.address <= load.address:
            continue
        if op.mnemonic == "call" and op.operands[0].type == X86_OP_REG and op.operands[0].reg == register:
            calls += 1
        if register in op.regs_access()[1]:
            break
    assert calls == 2


def assert_clock_operand(obj, unit, address, cached):
    code, relocations, _ = obj.subject(obj.function(symbol_at(unit, address)))
    clock, = (r for r in relocations if obj.symbols[int(r.target)].name == CLOCK)
    ops = decode(code)
    op, = (op for op in ops if op.address <= clock.offset < op.address + op.size)
    if cached:
        assert_two_calls_through_loaded_register(ops, op)
        assert op.operands[1].mem.disp == 0
    else:
        assert op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM
        assert not op.operands[0].mem.base and not op.operands[0].mem.index
        assert op.operands[0].mem.disp == 0


def test_original_pe_and_callers_distinguish_the_import_slot_from_executable_code(clock_objects):
    _, _, _, image = clock_objects
    original, = (i for i in image.imports() if i.module.lower() == "winmm.dll" and i.name == "timeGetTime")
    assert original.ordinal is None and original.address == 0x669368
    for address in (0x4C6B2B, 0x4C698E, 0x445715):
        op, = decode(image.read(Span(address, address + 6)), address)
        assert op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM
        assert not op.operands[0].mem.base and op.operands[0].mem.disp == original.address
    for start, end, load_address in ((0x58F2F0, 0x58F421, 0x58F2F4),
                                      (0x447360, 0x44746A, 0x44736C)):
        ops = decode(image.read(Span(start, end)), start)
        load, = (op for op in ops if op.address == load_address)
        assert load.operands[1].mem.disp == original.address
        assert_two_calls_through_loaded_register(ops, load)


def test_native_import_identity_resolves_uniquely_but_does_not_claim_link_selection(clock_objects):
    _, _, trials, image = clock_objects
    binding, = (b for b in read_imports(ROOT) if b.symbol == CLOCK)
    for objects in trials:
        targets, reports = import_targets([binding], objects, image)
        assert targets == {CLOCK: 0x669368}
        report, = reports
        assert report["identity"] == "pass" and report["link_selection"] == "unknown"
        for wrong in (replace(binding, module="KERNEL32.dll"), replace(binding, name="GetTickCount")):
            targets, reports = import_targets([wrong], objects, image)
            assert targets == {} and reports[0]["identity"] == "unknown"


@pytest.mark.parametrize("unit,address,cached", CONSUMERS)
def test_compiler_uses_import_pointer_with_original_load_and_call_shape(clock_objects, unit, address, cached):
    _, _, trials, _ = clock_objects
    for objects in trials:
        assert_clock_operand(objects[unit], unit, address, cached)


@pytest.mark.parametrize("unit,address,cached,before,after", [
    ("src/wave.cpp", 0x4C6B10, False, b"timeGetTime()", b"GetTickCount()"),
    ("src/wave.cpp", 0x4C6920, False, b"timeGetTime()", b"(*reinterpret_cast<DWORD (WINAPI **)()>(0x00669368))()"),
    ("src/ambience.cpp", 0x447360, True, b"= timeGetTime;", b"= reinterpret_cast<DWORD (WINAPI *)()>(0x00669368);"),
    ("src/game.cpp", 0x58F2F0, True, b"= timeGetTime;", b"= GetTickCount;"),
])
def test_clock_contract_rejects_wrong_import_and_original_address_alias(clock_objects, unit, address, cached, before, after):
    compiler, settings, _, _ = clock_objects
    path = ROOT / unit
    source = path.read_bytes()
    assert before in source
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {unit: source.replace(before, after)}, (), flags + settings.layout_flags))
        with pytest.raises((AssertionError, ValueError)):
            assert_clock_operand(obj, unit, address, cached)
    assert path.read_bytes() == source
