"""Live paint/draw branch controls and actual emitted ABI; not game acceptance."""

import shutil
from dataclasses import replace

import pytest

from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from test_application_link_recovery import ROOT, references, run_recovered_harness, symbol_at
from test_editbox_recovery import table_targets
import test_application_link_recovery as harness_support


@pytest.fixture(scope="module")
def paint_objects():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    return {unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                   for flags in settings.flag_sets]
            for unit in ("src/buffer.cpp", "src/graphicwin.cpp", "src/editbox.cpp", "src/caviar.cpp")}


def test_paint_override_is_inherited_by_editbox(paint_objects):
    for owner, unit in (("GraphicWin", "src/graphicwin.cpp"), ("EditBox", "src/editbox.cpp")):
        for obj in paint_objects[unit]:
            _, slots = table_targets(obj, f"??_7{owner}@@6BWin@@@")
            assert slots[73] == "?on_paint@GraphicWin@@UAEXPAUtagRECT@@@Z"


def test_missing_override_control_selects_wrong_base_entry(paint_objects):
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    header = (ROOT / "src/graphicwin.h").read_bytes()
    declaration = b"  virtual void on_paint(RECT *area);\n"
    assert header.count(declaration) == 1
    for flags in settings.flag_sets:
        obj = Coff(compiler.overlay(ROOT / "src/editbox.cpp",
                                   {"src/graphicwin.h": header.replace(declaration, b"")},
                                   (), flags + settings.layout_flags))
        _, slots = table_targets(obj, "??_7EditBox@@6BWin@@@")
        assert slots[73] == "?on_paint@Win@@UAEXPAUtagRECT@@@Z"


def test_draw_uses_one_real_sprite_and_correct_six_argument_entry(paint_objects):
    draw = "?draw@Buffer@@QAEHPAV1@HHHHH@Z"
    shared = "?g_BUFFER_SPRITE@@3VSprite@@A"
    for obj in paint_objects["src/buffer.cpp"]:
        assert len([s for s in obj.symbols.values() if s.name == shared and s.section > 0]) == 1
        targets = references(obj, draw)
        assert shared in targets
        assert "?draw@Sprite@@QAEHPAVBuffer@@HHHHH@Z" in targets
        assert not any("BlitSource" in s.name or "BlitClip" in s.name for s in obj.symbols.values())
        for name in ("?setup_buff_sprite@Buffer@@QAEXH@Z", "?release_buff_sprite@Buffer@@SAXXZ",
                     "?draw_dest@Buffer@@QAEHPAV1@HHHH@Z",
                     "?draw_multi_table_dest@Buffer@@QAEHPAV1@HHHH@Z"):
            assert shared in references(obj, name)
    for unit, address in (("src/graphicwin.cpp", 0x5D5CF0), ("src/caviar.cpp", 0x617230)):
        for obj in paint_objects[unit]:
            assert draw in references(obj, symbol_at(unit, address))


def test_actual_buffer_draw_routes_surfaces_and_borrowed_software_storage(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "buffer_draw_harness.cpp", [
        ("// LIVE_DRAW", "src/buffer.cpp", 0x5D84C0),
        ("// LIVE_RELEASE", "src/buffer.cpp", 0x5D8360)], declarations=[
        ("// LIVE_SPRITE", "src/sprite.h", "class Sprite {", "static_assert(sizeof(Sprite)"),
        ("// LIVE_SETUP", "src/buffer.cpp", "void Buffer::setup_buff_sprite(int colour) {",
         "/*\nPurpose: Release the shared Sprite descriptor")])


def test_actual_graphicwin_paint_clipping_routing_and_restoration(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "graphicwin_paint_harness.cpp", [
        ("// LIVE_PAINT", "src/graphicwin.cpp", 0x5D5CF0)])


@pytest.mark.parametrize("address,old,new,control", [
    (0x5D84C0, "return denominator;", "return 0;", "draw"),
    (0x5D5CF0, "ScreenBuffer.set_clip(area);", "Buffer::set_clip(area);", "paint"),
])
def test_live_controls_reject_wrong_return_or_clip_receiver(tmp_path, monkeypatch, address, old, new, control):
    reader = harness_support.read_file

    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == address:
                assert record.region.count(old) == 1
                record = replace(record, region=record.region.replace(old, new))
            result.append(record)
        return result

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        if control == "draw":
            test_actual_buffer_draw_routes_surfaces_and_borrowed_software_storage(tmp_path, monkeypatch)
        else:
            test_actual_graphicwin_paint_clipping_routing_and_restoration(tmp_path, monkeypatch)
