"""Network clock import identity and operands; not network protocol acceptance."""

import os
from pathlib import Path
import shutil

import pytest
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from match_recovery.compiler import Compiler
from match_recovery.import_source import import_targets, read_imports
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, symbol_at
from test_wave_clock_import import CLOCK, decode


NETDAEMON_CLOCKS = (0x592E10, 0x592E70, 0x592F50, 0x592FF0, 0x593090,
                   0x593110, 0x593190, 0x532940, 0x5303B0, 0x5304A0,
                   0x530630, 0x530920, 0x530AA0, 0x530E50, 0x531300,
                   0x531480, 0x5315C0)
CONSUMERS = (("src/net_class.cpp", 0x6320E0),
             *(("src/netdaemon.cpp", address) for address in NETDAEMON_CLOCKS))


@pytest.fixture(scope="module")
def network_clocks():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned original image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    trials = [{unit: Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
               for unit in ("src/net_class.cpp", "src/netdaemon.cpp")}
              for flags in settings.flag_sets]
    return compiler, settings, trials, PeImage(path.read_bytes(), image_pin(ROOT))


def clock_operands(obj, unit, address):
    code, relocations, _ = obj.subject(obj.function(symbol_at(unit, address)))
    clocks = [r for r in relocations if obj.symbols[int(r.target)].name == CLOCK]
    assert clocks, "no source-bound multimedia clock import in this body"
    imports = {obj.symbols[int(r.target)].name for r in relocations
               if obj.symbols[int(r.target)].name.startswith("__imp_")}
    assert imports == {CLOCK}, "unexpected direct SDK import in this clock consumer"
    ops = decode(code)
    assert not any((operand.type == X86_OP_MEM and not operand.mem.base and not operand.mem.index
                    and operand.mem.disp == 0x669368)
                   or (operand.type == X86_OP_IMM and operand.imm == 0x669368)
                   for op in ops for operand in op.operands), "stale executable clock-slot address"
    for relocation in clocks:
        op, = (op for op in ops if op.address <= relocation.offset < op.address + op.size)
        # A dllimport pointer is read or called indirectly, never called as code.
        assert op.mnemonic in ("mov", "call")
        source = op.operands[0] if op.mnemonic == "call" else op.operands[1]
        assert source.type == X86_OP_MEM
        assert not source.mem.base and not source.mem.index and source.mem.disp == 0
    return clocks


@pytest.mark.parametrize("unit,address", CONSUMERS)
def test_original_and_compiler_reach_the_same_import_pointer(network_clocks, unit, address):
    _, _, trials, image = network_clocks
    binding, = (b for b in read_imports(ROOT) if b.symbol == CLOCK)
    original, = (i for i in image.imports() if i.module.lower() == "winmm.dll" and i.name == "timeGetTime")
    row, = (r for r in read_file(ROOT / unit) if r.address == address)
    assert len(row.image_spans) == 1
    start, end = row.image_spans[0]
    original_reads = [op for op in decode(image.read(Span(start, end)), start)
                      if op.mnemonic in ("mov", "call") and any(
                          operand.type == X86_OP_MEM and not operand.mem.base and not operand.mem.index
                          and operand.mem.disp == original.address for operand in op.operands)]
    assert original_reads, "no independently decoded original IAT access"
    all_slots = {i.address for i in image.imports()}
    original_slots = {operand.mem.disp for op in decode(image.read(Span(start, end)), start)
                      for operand in op.operands if operand.type == X86_OP_MEM
                      and not operand.mem.base and not operand.mem.index and operand.mem.disp in all_slots}
    assert original_slots == {original.address}
    for objects in trials:
        targets, reports = import_targets([binding], objects, image)
        assert targets == {CLOCK: original.address}
        assert reports[0]["identity"] == "pass" and reports[0]["link_selection"] == "unknown"
        clock_operands(objects[unit], unit, address)


@pytest.mark.parametrize("unit,address,before,after", [
    ("src/net_class.cpp", 0x6320E0, b"= timeGetTime;", b"= GetTickCount;"),
    ("src/netdaemon.cpp", 0x592E10, b"message.time_ = timeGetTime();",
     b"message.time_ = (*reinterpret_cast<DWORD (WINAPI **)()>(0x00669368))();"),
    ("src/netdaemon.cpp", 0x5303B0, b"= timeGetTime;", b"= GetTickCount;"),
])
def test_network_clock_controls_reject_wrong_api_and_old_address(network_clocks, unit, address, before, after):
    compiler, settings, _, _ = network_clocks
    path = ROOT / unit
    source = path.read_bytes()
    assert before in source
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {unit: source.replace(before, after)}, (), flags + settings.layout_flags))
        with pytest.raises(AssertionError, match="no source-bound multimedia clock"):
            clock_operands(obj, unit, address)
    assert path.read_bytes() == source


@pytest.mark.parametrize("replacement,error", [
    (b"GetTickCount()", "unexpected direct SDK import"),
    (b"(*reinterpret_cast<DWORD (WINAPI **)()>(0x00669368))()", "stale executable clock-slot address"),
])
def test_one_good_clock_call_cannot_hide_a_bad_sibling(network_clocks, replacement, error):
    compiler, settings, _, _ = network_clocks
    unit = "src/netdaemon.cpp"
    path = ROOT / unit
    source = path.read_bytes()
    before = b"unsigned long t0 = timeGetTime();"
    assert source.count(before) == 1
    damaged = source.replace(before, b"unsigned long t0 = " + replacement + b";")
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(path, {unit: damaged}, (), flags + settings.layout_flags))
        with pytest.raises(AssertionError, match=error):
            clock_operands(obj, unit, 0x530630)
    assert path.read_bytes() == source
