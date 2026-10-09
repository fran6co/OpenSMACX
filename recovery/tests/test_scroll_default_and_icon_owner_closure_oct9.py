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
    popup_source = (ROOT / "src/popup.cpp").read_text()
    assert "Sprite *sprite_left_[3];" in header
    assert "Sprite *sprite_up_[3];" in header
    assert "Sprite *sprite_right_[3];" in header
    assert "Sprite *sprite_down_[3];" in header
    assert "uint32_t *const object = reinterpret_cast<uint32_t *>(this)" not in source_text
    assert "reinterpret_cast<uint32_t *>(this)" not in source_text

    constructor = _definition(source_text, "Scroll::Scroll()")
    close = _definition(source_text, "uint32_t Scroll::close()")
    record_start = header.index("struct ScrollCloseDynamicDefaultsData {")
    record_end = header.index("\n};", record_start) + 3
    record = header[record_start:record_end]
    popup_default_stores = "\n".join(
        line.strip()
        for line in popup_source.splitlines()
        if "ScrollCloseDynamicDefaults." in line
        or "ScrollCloseStaticDefaults[" in line
        or "ScrollDefaultThickness =" in line
    )
    normal_setter = _definition(source_text, "void __cdecl scroll_normal()")
    small_setter = _definition(source_text, "void __cdecl scroll_small()")
    assert "ScrollCloseDynamicDefaults[" not in popup_source
    producer_order = [
        "ScrollCloseStaticDefaults[8]", "ScrollCloseStaticDefaults[3]",
        "ScrollCloseStaticDefaults[7]", "ScrollCloseStaticDefaults[5]",
        "ScrollCloseStaticDefaults[6]",
        "sprite_up[0]", "sprite_up[1]", "sprite_up[2]",
        "sprite_down[0]", "sprite_down[1]", "sprite_down[2]",
        "sprite_left[0]", "sprite_left[1]", "sprite_left[2]",
        "sprite_right[0]", "sprite_right[1]", "sprite_right[2]", ".thumb",
        "ScrollDefaultThickness = 20",
    ]
    producer_positions = [popup_default_stores.index(member) for member in producer_order]
    assert producer_positions == sorted(producer_positions)
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
#define __cdecl
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
''' + record + r'''
ScrollCloseDynamicDefaultsData ScrollCloseDynamicDefaults;
Sprite g_SCROLL_BAR_ARROW_ICON_SPRITES[12];
Sprite g_SCROLL_BAR_FILLER_ICON_SPRITES;
Sprite g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[12];
Sprite g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE;
int ScrollDefaultThickness;
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
''' + constructor + "\n" + close + "\n" + triplet + "\n" + setters + "\n" + _definition(source_text, "static __forceinline void set_scroll_icon_defaults(") + "\n" + normal_setter + "\n" + small_setter + r'''
int main() {
    for (uint32_t i = 0; i < 11; ++i) ScrollCloseStaticDefaults[i] = 100 + i;
    Sprite defaults[12];
    ScrollCloseDynamicDefaults.flags = 200;
    ScrollCloseDynamicDefaults.position = 201;
    for (int i = 0; i < 3; ++i) {
        ScrollCloseDynamicDefaults.sprite_left[i] = &defaults[i];
        ScrollCloseDynamicDefaults.sprite_up[i] = &defaults[3 + i];
        ScrollCloseDynamicDefaults.sprite_right[i] = &defaults[6 + i];
        ScrollCloseDynamicDefaults.sprite_down[i] = &defaults[9 + i];
    }
    Sprite thumb_default, background_default;
    ScrollCloseDynamicDefaults.reserved_38 = 0xDEADBEEF;
    ScrollCloseDynamicDefaults.thumb = &thumb_default;
    ScrollCloseDynamicDefaults.background = &background_default;
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
        assert(value.sprite_left_[i] == &defaults[i]);
        assert(value.sprite_up_[i] == &defaults[3 + i]);
        assert(value.sprite_right_[i] == &defaults[6 + i]);
        assert(value.sprite_down_[i] == &defaults[9 + i]);
    }
    assert(value.thumb_sprite_ == &thumb_default);
    assert(value.field_A78_ == &background_default);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xDEADBEEF);
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
    assert(value.sprite_left_[2] == &defaults[2]);
    assert(value.sprite_up_[2] == &defaults[5]);
    assert(value.sprite_right_[2] == &defaults[8]);
    assert(value.sprite_down_[2] == &defaults[11]);
    assert(value.field_2144_ == nullptr && value.field_2148_ == nullptr);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xDEADBEEF);

    value.field_A14_ = 0;
    value.sprite_left_[0] = nullptr;
    close_count = 0;
    assert(value.close() == 0xCAFE);
    assert(close_count == 3 && close_order[0] == 1 && close_order[1] == 2 && close_order[2] == 3);
    assert(value.field_A14_ == 200 && value.sprite_left_[0] == &defaults[0]);
    assert(value.field_A78_ == &background_default);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xDEADBEEF);

    ScrollCloseDynamicDefaults.background = &a;
    ScrollCloseDynamicDefaults.thumb = &b;
    ScrollCloseDynamicDefaults.reserved_38 = 0xCAFEBABE;
    Sprite normal[12];
    set_scroll_icon_defaults(normal, &c, 20);
    assert(ScrollDefaultThickness == 20);
    assert(ScrollCloseDynamicDefaults.sprite_up[0] == &normal[0]);
    assert(ScrollCloseDynamicDefaults.sprite_down[2] == &normal[5]);
    assert(ScrollCloseDynamicDefaults.sprite_left[0] == &normal[6]);
    assert(ScrollCloseDynamicDefaults.sprite_right[2] == &normal[11]);
    assert(ScrollCloseDynamicDefaults.thumb == &c);
    assert(ScrollCloseDynamicDefaults.background == &a);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xCAFEBABEu);

    Sprite *before_left = ScrollCloseDynamicDefaults.sprite_left[0];
    Sprite *before_up = ScrollCloseDynamicDefaults.sprite_up[0];
    Sprite *before_right = ScrollCloseDynamicDefaults.sprite_right[0];
    Sprite *before_down = ScrollCloseDynamicDefaults.sprite_down[0];
    Sprite *before_thumb = ScrollCloseDynamicDefaults.thumb;
    Sprite *before_background = ScrollCloseDynamicDefaults.background;
''' + popup_default_stores + r'''
    assert(ScrollCloseDynamicDefaults.sprite_left[0] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[6]);
    assert(ScrollCloseDynamicDefaults.sprite_up[0] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[0]);
    assert(ScrollCloseDynamicDefaults.sprite_right[0] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[9]);
    assert(ScrollCloseDynamicDefaults.sprite_down[0] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[3]);
    assert(ScrollCloseDynamicDefaults.thumb == &g_SCROLL_BAR_FILLER_ICON_SPRITES);
    assert(ScrollCloseStaticDefaults[8] == 0 &&
           ScrollCloseStaticDefaults[3] == 0xFFFFFFFFu &&
           ScrollCloseStaticDefaults[7] == 0x41 &&
           ScrollCloseStaticDefaults[5] == 0x41 &&
           ScrollCloseStaticDefaults[6] == 0x41);
    assert(ScrollDefaultThickness == 20);
    assert(before_left == &normal[6] && before_up == &normal[0]);
    assert(before_right == &normal[9] && before_down == &normal[3]);
    assert(before_thumb == &c);
    assert(ScrollCloseDynamicDefaults.background == before_background);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xCAFEBABEu);
    value.close();
    assert(value.field_A78_ == before_background);
    assert(value.thumb_sprite_ == &g_SCROLL_BAR_FILLER_ICON_SPRITES);
    scroll_normal();
    assert(ScrollDefaultThickness == 20);
    assert(ScrollCloseDynamicDefaults.sprite_left[1] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[7]);
    assert(ScrollCloseDynamicDefaults.sprite_up[2] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[2]);
    assert(ScrollCloseDynamicDefaults.sprite_right[0] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[9]);
    assert(ScrollCloseDynamicDefaults.sprite_down[1] == &g_SCROLL_BAR_ARROW_ICON_SPRITES[4]);
    assert(ScrollCloseDynamicDefaults.thumb == &g_SCROLL_BAR_FILLER_ICON_SPRITES);
    assert(ScrollCloseDynamicDefaults.background == before_background);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xCAFEBABEu);
    scroll_small();
    assert(ScrollDefaultThickness == 12);
    assert(ScrollCloseDynamicDefaults.sprite_left[1] == &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[7]);
    assert(ScrollCloseDynamicDefaults.sprite_up[2] == &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[2]);
    assert(ScrollCloseDynamicDefaults.sprite_right[0] == &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[9]);
    assert(ScrollCloseDynamicDefaults.sprite_down[1] == &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[4]);
    assert(ScrollCloseDynamicDefaults.thumb == &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE);
    assert(ScrollCloseDynamicDefaults.background == before_background);
    assert(ScrollCloseDynamicDefaults.reserved_38 == 0xCAFEBABEu);
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
