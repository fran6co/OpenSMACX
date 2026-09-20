"""Actual callback on the shared popup block, not sound-stack acceptance."""

import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin

from test_application_link_recovery import ROOT, run_recovered_harness


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    return PeImage(path.read_bytes(), image_pin(ROOT))


def test_original_producer_and_callers_use_the_same_complete_block(original):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def instructions(start, end):
        return [(op.mnemonic, op.op_str) for op in
                decoder.disasm(original.read(Span(start, end)), start)]

    assert instructions(0x445CE6, 0x445CF0) == [
        ("mov", "dword ptr [0x9b8cf8], 0x4456b0")]
    for start, end, offset, flag in (
            (0x603693, 0x6036A1, 0x2E64, 1),
            (0x60377E, 0x60378C, 0x2F7C, 0)):
        assert instructions(start, end) == [
            ("lea", f"ecx, [esi + {hex(offset)}]"), ("push", str(flag)),
            ("push", "ecx"), ("call", "eax"), ("add", "esp, 8")]
    # Independent accesses inside the callback, not inferred from our struct.
    assert instructions(0x4456CD, 0x4456D3) == [("mov", "eax, dword ptr [edi + 0x104]")]
    assert original.read_initial(Span(0x9B8CF8, 0x9B8CFC)) == bytes(4)


def test_live_callback_binding_and_zero_initialized_pointer_storage(original):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    record, = (r for r in read_file(ROOT / "src/popup.cpp") if r.address == 0x4456B0)
    binding, = (r for r in read_objects(ROOT, image_pin(ROOT)) if r.origin.address == 0x9B8CF8)
    assert binding.span == Span(0x9B8CF8, 0x9B8CFC)
    for flags in settings.flag_sets:
        callback = Coff(compiler(ROOT / "src/popup.cpp", (), flags + settings.layout_flags))
        assert callback.function(record.symbol).name == "?popup_wave_callback@@YAXPAUBasePopTextBlock@@H@Z"
        storage = Coff(compiler(ROOT / "src/basepop.cpp", (), flags + settings.layout_flags))
        row, = check_initial_data([binding], {"src/basepop.cpp": storage}, original, {})
        assert row["content"] == row["proof"]["boundary"] == "pass"
        assert row["compiler_storage"]["size"] == 4
    # This is loader state and a type-compatible callback, not the still-pending
    # FX store, owner notification target or linked selection. Clock import
    # operands have separate original/COFF controls in test_wave_clock_import.


def test_actual_callback_consumes_the_shared_block(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "popup_wave_callback_harness.cpp", [
        ("// LIVE_CALLBACK", "src/popup.cpp", 0x4456B0)], declarations=[
        ("// LIVE_TEXT_BLOCK", "src/basepop.h", "struct BasePopTextBlock {",
         "static_assert(sizeof(BasePopTextBlock)"),
        ("// LIVE_CALLBACK_TYPE", "src/basepop.h", "typedef void (__cdecl *BasePopConfigCallback)",
         " /*\n  * BasePop class")])


@pytest.mark.parametrize("before,after", [
    ("popup->field_104_", "popup->field_10C_"),
    ("if (!popup->field_108_)", "if (popup->field_108_)"),
    ("if (index >= 0x2D)", "if (index >= 0x2C)"),
    ("popup->field_104_ = 0x25;", "popup->field_104_ = 0x24;"),
    ("% 3 != 1", "% 3 == 1"),
    ("g_FX.FX::play(0x38);", "g_FX.FX::play(0x37);"),
])
def test_callback_controls_reject_behavior_damage(tmp_path, monkeypatch, before, after):
    original = Path.read_text

    def damaged(path, *args, **kwargs):
        text = original(path, *args, **kwargs)
        if path == ROOT / "src/popup.cpp":
            assert before in text
            return text.replace(before, after)
        return text

    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_actual_callback_consumes_the_shared_block(tmp_path, monkeypatch)
