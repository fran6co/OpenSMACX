"""Loader-owned export storage and actual consumers; not driver ABI acceptance."""

from pathlib import Path

import pytest
from capstone.x86 import X86_OP_MEM

from match_recovery.object_data import check_initial_data
from match_recovery.object_source import object_targets, read_objects
from match_recovery.pe_coff import Coff
from match_recovery.model import Span
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness
from test_sound_lifetime import decode
from test_sound_virtual_surface import family


EXPORTS = "?SoundDllProcs@@3PAPAXA"


@pytest.fixture(scope="module")
def export_storage(family):
    compiler, settings, trials, image = family
    objects = [{**trial, "src/sound_loader.cpp": Coff(compiler(
        ROOT / "src/sound_loader.cpp", (), flags + settings.layout_flags))}
        for trial, flags in zip(trials, settings.flag_sets)]
    binding, = (b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol == EXPORTS)
    return compiler, settings, objects, image, binding


def test_original_loader_establishes_the_export_array_and_consumer_roles(export_storage):
    _, _, _, image, binding = export_storage
    assert binding.span == Span(0x90DB24, 0x90DB50)
    assert binding.evidence.address == 0x4C5E50
    ops = {op.address: op for op in decode(image.read(Span(0x4C5E50, 0x4C5F68)), 0x4C5E50)}
    assert ops[0x4C5E85].operands[1].imm == binding.span.start
    assert ops[0x4C5EA4].mnemonic == "add" and ops[0x4C5EA4].operands[1].imm == 4
    assert ops[0x4C5EA8].operands[1].imm == binding.span.end
    assert ops[0x4C5EE1].operands[1].imm == 11
    assert ops[0x4C5EE8].operands[1].imm == binding.span.start
    assert ops[0x4C5EF2].mnemonic == "rep stosd"
    assert image.read_initial(binding.span) == bytes(44)
    for address, target in ((0x4C616E, 0x90DB28), (0x4C6E23, 0x90DB24)):
        op, = decode(image.read(Span(address, address + 6)), address)
        assert op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM
        assert not op.operands[0].mem.base and op.operands[0].mem.disp == target


def test_native_data_check_observes_the_real_eleven_slot_owner(export_storage):
    _, _, trials, image, binding = export_storage
    for objects in trials:
        targets, _ = object_targets([binding], objects)
        assert targets[EXPORTS] == binding.span.start
        row, = check_initial_data([binding], objects, image, targets)
        assert row["definition"] == row["content"] == "pass", row
        assert row["compiler_storage"]["size"] == 44
        assert row["lifetime"] == "unknown"


def call_slot_offsets(obj, symbol):
    code, relocations, _ = obj.subject(obj.function(symbol))
    result = []
    for relocation in relocations:
        if obj.symbols[int(relocation.target)].name != EXPORTS:
            continue
        op, = (op for op in decode(code) if op.address <= relocation.offset < op.address + op.size)
        assert op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM
        assert not op.operands[0].mem.base and not op.operands[0].mem.index
        result.append(op.operands[0].mem.disp)
    return result


def test_real_sound_and_wave_consumers_call_the_loader_storage(export_storage):
    _, _, trials, _, _ = export_storage
    for objects in trials:
        assert call_slot_offsets(objects["src/sound.cpp"], "??1Sound@@UAE@XZ") == [4]
        assert call_slot_offsets(objects["src/wave.cpp"], "?load@Wave@@UAEHXZ") == [0]


def test_native_data_check_rejects_a_short_export_owner(export_storage):
    compiler, settings, _, image, binding = export_storage
    header = (ROOT / "src/sound.h").read_bytes()
    assert header.count(b"typedef void *SoundDllProcsTable[11];") == 1
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(ROOT / "src/sound_loader.cpp", {
            "src/sound.h": header.replace(b"SoundDllProcsTable[11]", b"SoundDllProcsTable[10]")
        }, (), flags + settings.layout_flags))
        row, = check_initial_data([binding], {"src/sound_loader.cpp": obj}, image, {})
        assert row["definition"] == "pass" and row["content"] == "fail"
        assert row["proof"]["boundary"] == "fail"


def run_loader(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "sound_export_storage_harness.cpp", [], declarations=[
        ("// LIVE_EXPORT_TYPE", "src/sound.h", "typedef void *SoundDllProcsTable[11];", "extern SoundDllProcsTable SoundDllProcs;"),
        ("// LIVE_EXPORT_ABIS", "src/sound.h", "typedef unsigned int (__cdecl *func_sound_dll_version)();", "extern HMODULE SoundDllModule;"),
        ("// LIVE_EXPORT_STORAGE", "src/sound_loader.cpp", "SoundDllProcsTable SoundDllProcs;", "int __cdecl load_sound_dll()"),
        ("// LIVE_RELEASE_SLOT", "src/wave.h", "inline func_wave_device_release *&WaveDeviceReleaseSlot()", "extern int WaveDeviceReleaseGuard;"),
        ("// LIVE_CREATE_SLOT", "src/wave.h", "typedef int(__cdecl func_wave_device_create)", "// Movement sound, owned and initialized with the other Wave objects."),
        ("// LIVE_LOADER", "src/sound_loader.cpp", "int __cdecl load_sound_dll()", "/*\nPurpose: The sound-DLL version complaint box.")])


def test_actual_loader_populates_rebinds_and_clears_the_actual_role_accessors(tmp_path, monkeypatch):
    run_loader(tmp_path, monkeypatch)


@pytest.mark.parametrize("unit,before,after", [
    ("src/wave.h", "&SoundDllProcs[1]", "&SoundDllProcs[0]"),
    ("src/wave.h", "&SoundDllProcs[0]", "&SoundDllProcs[1]"),
    ("src/sound_loader.cpp", "i < 0xb", "i < 0xa"),
    ("src/sound_loader.cpp", "*slot = proc;", "if (index != 0) *slot = proc;"),
])
def test_loader_role_contract_rejects_wrong_slots_or_incomplete_storage(tmp_path, monkeypatch, unit, before, after):
    original_read = Path.read_text
    target = ROOT / unit
    assert original_read(target).count(before) == 1
    def damaged_read(path, *args, **kwargs):
        text = original_read(path, *args, **kwargs)
        return text.replace(before, after) if path == target else text
    monkeypatch.setattr(Path, "read_text", damaged_read)
    with pytest.raises(AssertionError):
        run_loader(tmp_path, monkeypatch)
