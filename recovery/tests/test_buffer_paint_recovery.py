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
    from match_recovery.object_source import parse_objects
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    binding, = (row for row in parse_objects(
        (ROOT / "src/buffer.cpp").read_text(), "src/buffer.cpp", pin,
        address_space="pe:va") if row.origin.address == 0x9B3A50)
    shared = binding.emitted_symbol
    assert shared == "?BufferSpriteOwner@@3UBufferSpriteLifetime@@A"
    assert binding.member_path == ("storage_", "bytes")
    for obj in paint_objects["src/buffer.cpp"]:
        assert len([s for s in obj.symbols.values() if s.name == shared and s.section > 0]) == 1
        targets = references(obj, draw)
        assert shared in targets
        assert "?draw@Sprite@@QAEHPAVBuffer@@HHHHH@Z" in targets
        assert not any("BlitSource" in s.name or "BlitClip" in s.name for s in obj.symbols.values())
        for name in ("?setup_buff_sprite@Buffer@@QAEXH@Z", "?release_buff_sprite@Buffer@@SAXXZ",
                     symbol_at("src/buffer.cpp", 0x5D8A30),
                     symbol_at("src/buffer.cpp", 0x5D93C0)):
            assert shared in references(obj, name)
        for address, target in ((0x5D8A30, "?draw_dest@Sprite@@QAEHPAVBuffer@@HHHPAE@Z"),
                                (0x5D93C0, "?draw_multi_table_dest@Sprite@@QAEHPAVBuffer@@HHHPAY0BAA@E@Z")):
            assert target in references(obj, symbol_at("src/buffer.cpp", address))
    for unit, address in (("src/graphicwin.cpp", 0x5D5CF0), ("src/caviar.cpp", 0x617230)):
        for obj in paint_objects[unit]:
            assert draw in references(obj, symbol_at(unit, address))


def test_actual_buffer_draw_routes_surfaces_and_borrowed_software_storage(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "buffer_draw_harness.cpp", [
        ("// LIVE_DRAW", "src/buffer.cpp", 0x5D84C0),
        ("// LIVE_FOUR_ARGUMENT_DRAW", "src/buffer.cpp", 0x5D8370),
        ("// LIVE_RELEASE", "src/buffer.cpp", 0x5D8360)], declarations=[
        ("// LIVE_SPRITE", "src/sprite.h", "class Sprite {", "static_assert(sizeof(Sprite)"),
        ("// LIVE_COLOR_TABLE", "src/jackal/color_table.h", "typedef unsigned char ColorTable[256];", None),
        ("// LIVE_PREPARE", "src/buffer.cpp", "inline void Buffer::prepare_shared_sprite(int colour) {",
         "__DEFINITION__"),
        ("// LIVE_SETUP", "src/buffer.cpp", "void Buffer::setup_buff_sprite(int colour) {",
         "__DEFINITION__")])


@pytest.mark.parametrize("first_overload", [6, 4])
def test_actual_buffer_draw_rejects_unconditional_shared_sprite_setup(
        tmp_path, monkeypatch, first_overload):
    build = harness_support.recovered_harness_source

    def damaged(*args, **kwargs):
        source = build(*args, **kwargs)
        guard = ("inline void Buffer::prepare_shared_sprite(int colour) {\n"
                 "    if (surface_ != nullptr) {\n"
                 "        return;\n"
                 "    }\n")
        assert source.count(guard) == 1
        source = source.replace(guard, "inline void Buffer::prepare_shared_sprite(int colour) {\n", 1)
        if first_overload == 4:
            order = "static const bool four_argument_first = false;"
            assert source.count(order) == 1
            source = source.replace(order, "static const bool four_argument_first = true;", 1)
        return source

    monkeypatch.setattr(harness_support, "recovered_harness_source", damaged)
    with pytest.raises(AssertionError, match="Assertion failed") as rejected:
        test_actual_buffer_draw_routes_surfaces_and_borrowed_software_storage(tmp_path, monkeypatch)
    assert "CONTROL_OVERLOAD=" + str(first_overload) in str(rejected.value)
    assert "CONTROL_OVERLOAD=" + str(10 - first_overload) not in str(rejected.value)
    assert "pcBits_ == expected_bits && cTransparentIndex_ == expected_colour" in str(rejected.value)


def test_actual_graphicwin_paint_clipping_routing_and_restoration(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "graphicwin_paint_harness.cpp", [
        ("// LIVE_PAINT", "src/graphicwin.cpp", 0x5D5CF0)])


@pytest.mark.parametrize("address,old,new,control", [
    (0x5D84C0, "return denominator;", "return 0;", "draw"),
    (0x5D5CF0, "ScreenBufferOwner.buffer().set_clip(area);", "Buffer::set_clip(area);", "paint"),
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
