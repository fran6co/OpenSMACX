"""Exercise Scroll's actual default, reset, and sprite-setter bodies."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_scroll_actual_typed_owner_defaults_and_setters(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source_text = (ROOT / "src/scroll.cpp").read_text()
    header = (ROOT / "src/scroll.h").read_text()
    assert "Sprite *sprite_left_[3];" in header
    assert "Sprite *sprite_up_[3];" in header
    assert "Sprite *sprite_right_[3];" in header
    assert "Sprite *sprite_down_[3];" in header
    assert "uint32_t *const object = reinterpret_cast<uint32_t *>(this)" not in source_text
    assert "reinterpret_cast<uint32_t *>(this)" not in source_text

    constructor = _definition(source_text, "Scroll::Scroll()")
    close = _definition(source_text, "uint32_t Scroll::close()")
    pointer_conversion = _definition(source_text, "inline Sprite *sprite_from_default_bits(")
    triplet = _definition(source_text, "__forceinline void set_sprite_triplet(")
    setters = "\n".join(
        _definition(source_text, signature)
        for signature in (
            "void Scroll::set_sprite_left(",
            "void Scroll::set_sprite_right(",
            "void Scroll::set_sprite_up(",
            "void Scroll::set_sprite_down(",
        )
    )

    fixture = r'''
#include <cassert>
#include <cstdint>
#define __forceinline inline
using uint32_t = std::uint32_t;
using uintptr_t = std::uintptr_t;
using LONG = std::int32_t;
struct Sprite {};
struct RECT { LONG left, top, right, bottom; };
struct BitmapHeader { LONG biWidth, biHeight; };
struct Dib { BitmapHeader bmiHeader; };
static int close_order[3];
static int close_count;
struct FlatButton {
    Sprite *config[27] = {};
    int order;
    FlatButton(int value) : order(value) {}
    void close() { close_order[close_count++] = order; }
    void set_scroll_icons(Sprite *, Sprite *, Sprite *) {}
};
struct GraphicWin {
    Dib dib_{};
    uint32_t close() { close_order[close_count++] = 3; return 0xCAFE; }
};
uint32_t ScrollCloseStaticDefaults[11];
uint32_t ScrollCloseDynamicDefaults[17];
struct Scroll : GraphicWin {
    uint32_t field_A14_ = 99, field_A18_ = 98;
    int border_color_ = 0, range_minimum_ = 0, range_maximum_ = 0;
    uint32_t range_reversed_ = 0;
    int position_ = 0;
    uint32_t field_A30_ = 0, field_A34_ = 0, field_A38_ = 0, field_A3C_ = 0;
    int paging_ = 0;
    uint32_t field_A44_ = 0, field_A48_ = 0;
    RECT thumb_rect_{};
    uint32_t color_ = 0;
    int bar_thickness_ = 0, field_A64_ = 0;
    uint32_t bevel_thickness_ = 0, bevel_upper_ = 0, bevel_lower_ = 0;
    Sprite *thumb_sprite_ = nullptr, *field_A78_ = nullptr;
    Sprite *sprite_left_[3]{}, *sprite_up_[3]{}, *sprite_right_[3]{}, *sprite_down_[3]{};
    FlatButton flat_button_left_{1}, flat_button_right_{2};
    void (*field_2144_)(int,int) = nullptr;
    void (*field_2148_)(int,int) = nullptr;
    Scroll();
    uint32_t close();
    void set_sprite_left(Sprite *, Sprite *, Sprite *);
    void set_sprite_right(Sprite *, Sprite *, Sprite *);
    void set_sprite_up(Sprite *, Sprite *, Sprite *);
    void set_sprite_down(Sprite *, Sprite *, Sprite *);
};
''' + pointer_conversion + "\n" + constructor + "\n" + close + "\n" + triplet + "\n" + setters + r'''
int main() {
    for (uint32_t i = 0; i < 11; ++i) ScrollCloseStaticDefaults[i] = 100 + i;
    for (uint32_t i = 0; i < 17; ++i) ScrollCloseDynamicDefaults[i] = 200 + i;
    ScrollCloseDynamicDefaults[14] = 0xDEADBEEF;
    ScrollCloseDynamicDefaults[16] = 0x12345678;
    ScrollCloseDynamicDefaults[15] = 0x87654321;
    Scroll value;
    assert(value.field_A14_ == 200 && value.border_color_ == 103);
    assert(value.range_reversed_ == 0 && value.range_minimum_ == 201);
    assert(value.range_maximum_ == 104 && value.position_ == 201);
    assert(value.field_A30_ == 102 && value.field_A3C_ == 0xFFFFFFFFu);
    assert(value.field_A38_ == 0 && value.field_A34_ == 101 && value.paging_ == 100);
    assert(value.thumb_rect_.left == 0 && value.thumb_rect_.top == 0 &&
           value.thumb_rect_.right == 0 && value.thumb_rect_.bottom == 0);
    assert(value.field_A48_ == 105 && value.color_ == 106 && value.field_A64_ == 107);
    assert(value.bevel_thickness_ == 108 && value.bevel_upper_ == 109 && value.bevel_lower_ == 110);
    for (int i = 0; i < 3; ++i) {
        assert(reinterpret_cast<uintptr_t>(value.sprite_left_[i]) == 202u + i);
        assert(reinterpret_cast<uintptr_t>(value.sprite_up_[i]) == 205u + i);
        assert(reinterpret_cast<uintptr_t>(value.sprite_right_[i]) == 208u + i);
        assert(reinterpret_cast<uintptr_t>(value.sprite_down_[i]) == 211u + i);
    }
    assert(reinterpret_cast<uintptr_t>(value.thumb_sprite_) == 0x87654321u);
    assert(reinterpret_cast<uintptr_t>(value.field_A78_) == 0x12345678u);
    assert(value.field_2144_ == nullptr && value.field_2148_ == nullptr);

    Sprite a, b, c;
    value.dib_.bmiHeader.biWidth = 80;
    value.dib_.bmiHeader.biHeight = -20;
    value.set_sprite_left(&a, &b, &c);
    assert(value.sprite_left_[0] == &a && value.sprite_left_[1] == &b && value.sprite_left_[2] == &c);
    assert(value.flat_button_left_.config[12] == &a && value.flat_button_left_.config[13] == &b && value.flat_button_left_.config[14] == &c);
    value.dib_.bmiHeader.biWidth = 20;
    value.dib_.bmiHeader.biHeight = -80;
    value.set_sprite_up(&c, &a, &b);
    assert(value.sprite_up_[0] == &c && value.flat_button_left_.config[12] == &c);
    value.dib_.bmiHeader.biWidth = 80;
    value.dib_.bmiHeader.biHeight = -20;
    value.set_sprite_right(&b, &c, &a);
    assert(value.flat_button_right_.config[12] == &b && value.flat_button_right_.config[13] == &c && value.flat_button_right_.config[14] == &a);
    value.dib_.bmiHeader.biWidth = 20;
    value.dib_.bmiHeader.biHeight = -80;
    value.set_sprite_down(&a, &c, &b);
    assert(value.sprite_down_[0] == &a && value.flat_button_right_.config[12] == &a);
    value.dib_.bmiHeader.biWidth = 20;
    value.dib_.bmiHeader.biHeight = -20;
    value.flat_button_left_.config[12] = &b;
    value.flat_button_right_.config[12] = &c;
    value.set_sprite_left(&a, &b, &c);
    value.set_sprite_up(&a, &b, &c);
    value.set_sprite_right(&a, &b, &c);
    value.set_sprite_down(&a, &b, &c);
    assert(value.flat_button_left_.config[12] == &b && value.flat_button_right_.config[12] == &c);

    value.field_2144_ = reinterpret_cast<void (*)(int,int)>(1);
    value.field_2148_ = reinterpret_cast<void (*)(int,int)>(2);
    close_count = 0;
    assert(value.close() == 0xCAFE);
    assert(close_count == 3 && close_order[0] == 1 && close_order[1] == 2 && close_order[2] == 3);
    assert(value.field_A14_ == 200 && value.border_color_ == 103 && value.range_reversed_ == 0);
    assert(value.range_minimum_ == 201 && value.range_maximum_ == 104 && value.position_ == 201);
    assert(value.sprite_left_[2] == reinterpret_cast<Sprite *>(202u + 2));
    assert(value.sprite_up_[2] == reinterpret_cast<Sprite *>(205u + 2));
    assert(value.sprite_right_[2] == reinterpret_cast<Sprite *>(208u + 2));
    assert(value.sprite_down_[2] == reinterpret_cast<Sprite *>(211u + 2));
    assert(value.field_2144_ == nullptr && value.field_2148_ == nullptr);
}
'''
    cpp_file = tmp_path / "scroll_actual_owner.cpp"
    binary = tmp_path / "scroll_actual_owner"
    cpp_file.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp_file), "-o", str(binary)],
        check=True,
        timeout=30,
    )
    subprocess.run([str(binary)], check=True, timeout=30)
