"""Actual-source contract checks for Buffer clip views and application callers."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def test_buffer_clip_views_preserve_reference_and_restore_alias():
    header = (ROOT / "src/buffer.h").read_text(encoding="utf-8")
    assert "const RECT &current_clip() const { return rect1_; }" in header
    assert "const RECT &clip_rect() const { return rect2_; }" in header
    assert "void restore_clip() { set_clip(&rect2_); }" in header
    for class_name in ("BaseWin", "MapWin", "MultiWin", "DipEdit"):
        assert f"friend class {class_name};" not in header


def test_application_clip_paths_use_typed_views_and_original_restore_order():
    base = (ROOT / "src/basewin.cpp").read_text(encoding="utf-8")
    map_source = (ROOT / "src/mapwin.cpp").read_text(encoding="utf-8")
    garrison = body(base, "void BaseWin::draw_garrison(int update_mode)")
    assert garrison.index("canvas->current_clip()") < garrison.index("canvas->set_clip(&clip_rect);")
    assert garrison.index("canvas->set_clip(&clip_rect);") < garrison.index("font.init(")
    assert garrison.index("MainInterfaceGlobal.update(") < garrison.index("canvas->set_clip(&saved_clip);")

    hit_test = body(base, "void BaseWin::on_left_up(int xCoord, int yCoord)")
    assert "const RECT &interface_clip = MainInterfaceGlobal.Buffer::clip_rect();" in hit_test
    assert "MainInterfaceGlobal.Buffer::rect2_" not in hit_test

    viewport = body(map_source, "void MapWin::draw_vehicles(int x, int y, int radius, int faction)")
    assert "const RECT &viewport = canvas->current_clip();" in viewport
    assert "pixel_to_tile(viewport.left, viewport.top," in viewport
    assert "canvas->rect1_" not in viewport

    for source in (base, map_source):
        assert "canvas->set_clip(&canvas->rect2_)" not in source
