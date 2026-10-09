"""Derived/base Wave destruction stages, not complete audio lifecycle recovery."""

from pathlib import Path

import pytest
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_ESI

from match_recovery.model import Span
from test_application_link_recovery import ROOT, references, run_recovered_harness
from test_sound_chain_storage import (CHAIN_DECLARATIONS, CHAIN_HANDLERS,
                                      SOUNDX_EXPORT_DECLARATIONS)
from test_sound_lifetime import decode
from test_sound_virtual_surface import family


def test_original_wave_destruction_changes_stage_before_release(family):
    _, _, _, image = family
    ops = decode(image.read(Span(0x4C67C0, 0x4C68EC)), 0x4C67C0)
    tables = [(op.address, op.operands[1].imm) for op in ops if op.mnemonic == "mov"
              and op.operands[0].type == X86_OP_MEM and op.operands[0].mem.base == X86_REG_ESI
              and op.operands[0].mem.disp == 0 and op.operands[1].type == X86_OP_IMM]
    assert tables == [(0x4C67E0, 0x66E44C), (0x4C6856, 0x66E3C0), (0x4C68D9, 0x66E444)]
    release, = (op for op in ops if op.mnemonic == "call" and op.operands[0].type == X86_OP_MEM)
    assert release.address == 0x4C6886 and release.operands[0].mem.disp == 0x90DB28
    assert tables[1][0] < release.address < tables[2][0]
    for start, end, target in ((0x659F06, 0x659F0E, 0x4C6120), (0x659F0E, 0x659F16, 0x4C8450),
                               (0x4482C0, 0x4482C5, 0x4C67C0)):
        instructions = decode(image.read(Span(start, end)), start)
        assert instructions[-1].mnemonic == "jmp" and instructions[-1].operands[0].imm == target


def test_wave_body_does_not_release_device_before_its_real_base(family):
    _, _, trials, _ = family
    for objects in trials:
        targets = references(objects["src/wave.cpp"], "??1Wave@@UAE@XZ")
        assert targets.count("??1Sound@@UAE@XZ") == 1, targets
        assert "?SoundDllProcs@@3PAPAXA" not in targets
        assert "?WaveDeviceReleaseGuard@@3HA" not in targets


def run_lifetime(tmp_path, monkeypatch):
    handlers = [(anchor.replace("LIVE_DESTRUCTOR", "LIVE_SOUND_DESTRUCTOR"), unit, address)
                for anchor, unit, address in CHAIN_HANDLERS]
    run_recovered_harness(tmp_path, monkeypatch, "wave_lifetime_harness.cpp",
        [*handlers, ("// LIVE_WAVE_DESTRUCTOR", "src/wave.cpp", 0x4C67C0)], declarations=[
            *SOUNDX_EXPORT_DECLARATIONS,
            *CHAIN_DECLARATIONS,
            ("// LIVE_WAVE_DECLARATION", "src/wave.h", "class Wave : public Sound {", "// The process-wide wave,")])


def test_actual_destructors_have_one_base_stage_and_correct_callback_order(tmp_path, monkeypatch):
    run_lifetime(tmp_path, monkeypatch)


@pytest.mark.parametrize("unit,before,after", [
    ("src/wave.cpp", "Wave::~Wave() {", "Wave::~Wave() { if (device_ && WaveDeviceReleaseGuard) SoundXOrdinal2DeleteSound()(device_);"),
    ("src/wave.cpp", "Wave::~Wave() {", "Wave::~Wave() { this->Sound::~Sound();"),
    ("src/wave.cpp", "WaveDeviceGlobal.pull_from_group(this);", "/* missing group removal */"),
    ("src/sound.cpp", "(SoundXOrdinal2DeleteSound())(device_);", "(SoundXOrdinal2DeleteSound())(device_); if (fname_) { operator delete(fname_); fname_ = 0; }"),
    ("src/wave.cpp", "flags_40_ &= ~2u;", "flags_40_ |= 2u;"),
])
def test_lifetime_rejects_wrong_stage_duplicate_cleanup_and_order(tmp_path, monkeypatch, unit, before, after):
    original = Path.read_text
    path = ROOT / unit
    assert before in original(path)
    def damaged(p, *args, **kwargs):
        text = original(p, *args, **kwargs)
        return text.replace(before, after) if p == path else text
    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError):
        run_lifetime(tmp_path, monkeypatch)
