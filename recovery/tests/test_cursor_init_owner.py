"""Actual-source cursor interaction controls, not native/game acceptance."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition

FIXTURE = Path(__file__).with_name("cursor_init_owner_control.cpp").read_text()


def product_control():
    text = (ROOT / "src/cursor.cpp").read_text()
    arithmetic = "\n\n".join(definition(text, r"static int " + name + r"\([^)]*\)")
                               for name in ("cursor_signed_word", "cursor_subtract", "cursor_add", "cursor_half"))
    body = definition(text, r"int Cursor::init\([^)]*\)")
    assert "PENDING_BODY" not in body and "BODY GOES HERE" not in body
    blocks = {"// @@CURSOR_CONSTRUCTOR@@": definition(text, r"Cursor::Cursor\(\)"),
              "// @@CURSOR_DESTRUCTOR@@": definition(text, r"Cursor::~Cursor\(\)"),
              "// @@CURSOR_ARITHMETIC@@": arithmetic,
              "// @@CURSOR_CLIP@@": definition(text, r"static int cursor_clip_hotspot\([^)]*\)"),
              "// @@CURSOR_INIT_BODY@@": body}
    result = FIXTURE
    for token, block in blocks.items():
        assert result.count(token) == 1
        result = result.replace(token, block.replace("nullptr", "0"))
    return result


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_cursor_lifetime_native_arithmetic_and_callback_snapshots(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = product_control()
    mutants = {
        "floor_not_truncation": source.replace("return value / 2;", "return value >= 0 ? value / 2 : -((-(value + 1) / 2) + 1);", 1),
        "cached_color_center": source.replace("color_x, color_y);", "draw_x, draw_y);", 1),
        "global_blit_size": source.replace("blit_width, blit_height, source_dc", "CursorWidth, CursorHeight, source_dc", 1),
        "post_callback_blit_size": source.replace("blit_width, blit_height, source_dc", "color.pixel_width(), color.pixel_height(), source_dc", 1),
        "cached_hotspot": source.replace("cursor_add(cursor_half(cursor_subtract(CursorWidth, sprite->iWidth_)), hotspot_x)", "draw_x + hotspot_x", 1),
        "wrong_palette": source.replace("color.dib_.bmiColors", "mono_info.colors", 1),
        "wrong_copy_receiver": source.replace("sprite->copy(owned_sprite);", "owned_sprite->copy(sprite);", 1),
        "constructor_wrong_hotspot": source.replace("hotspot_x_ = 0;", "hotspot_x_ = 1;", 1),
        "destructor_missing_sprite_close": source.replace("delete sprite;", "operator delete(sprite);", 1),
    }
    with tempfile.TemporaryDirectory(prefix="cursor-actual-source-") as temp:
        directory = Path(temp)
        for name, text in dict(actual=source, **mutants).items():
            if name != "actual":
                assert text != source, name
            cpp, executable = directory / (name + ".cpp"), directory / name
            cpp.write_text(text)
            build = subprocess.run([compiler, "-std=c++98", "-Wno-int-to-pointer-cast", str(cpp), "-o", str(executable)],
                                   capture_output=True, text=True, timeout=45)
            assert build.returncode == 0, build.stdout + build.stderr
            run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (name == "actual"), run.stdout + run.stderr
