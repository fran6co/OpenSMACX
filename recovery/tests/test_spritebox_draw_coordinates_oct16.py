"""Exercise the literal SpriteBox draw body with asymmetric grid geometry."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(text: str) -> str:
    signature = "void SpriteBox::draw_item(int position)"
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated SpriteBox::draw_item")


HARNESS = r'''
#include <cassert>
#include <cstring>
#include <stdint.h>

struct RECT { int left, top, right, bottom; };
struct GraphicWin;
struct Sprite;
struct StringStructPayload { char *text; };
struct SpriteStruct { Sprite *sprite_; };
struct Entry {
    int id;
    StringStructPayload *string_;
    SpriteStruct *sprite_;
    StringStructPayload *string_payload() { return string_; }
    SpriteStruct *sprite_payload() { return sprite_; }
};
struct ItemList {
    int entry_count_;
    Entry *head_, *current_;
    Entry *entries[8];
    bool seek_pos(int position) {
        if (position < 0 || position >= entry_count_) return false;
        current_ = entries[position]; return true;
    }
};
struct Win { Win *win_parent_; };
struct Buffer {
    Buffer *copy_receiver;
    Buffer *copy_source;
    int copy_sx, copy_sy, copy_dx, copy_dy, copy_width, copy_height;
    int box_left, box_top, box_right, box_bottom, box_color_a, box_color_b;
    RECT written_rect;
    int write_length, color_calls, font_calls;
    char written_text[32];
    Buffer() : copy_receiver(0), copy_source(0), copy_sx(0), copy_sy(0), copy_dx(0), copy_dy(0),
        copy_width(0), copy_height(0), box_left(0), box_top(0), box_right(0),
        box_bottom(0), box_color_a(0), box_color_b(0), write_length(0),
        color_calls(0), font_calls(0) { written_text[0]=0; }
    void copy(Buffer *source, int sx, int sy, int dx, int dy, int w, int h) {
        copy_receiver=this;
        copy_source=source; copy_sx=sx; copy_sy=sy; copy_dx=dx; copy_dy=dy;
        copy_width=w; copy_height=h;
    }
    void box(int l,int t,int r,int b,int ca,int cb) {
        box_left=l; box_top=t; box_right=r; box_bottom=b;
        box_color_a=ca; box_color_b=cb;
    }
    void write_l(char *text, RECT *area, int length) {
        written_rect=*area; write_length=length;
        std::memcpy(written_text,text,length); written_text[length]=0;
    }
    void set_text_color(int,int,int,int) { ++color_calls; }
    void set_text_color2(int,int,int,int) { ++color_calls; }
    void set_text_color3(int,int,int,int) { ++color_calls; }
    void set_font(void *,void *,void *,int) { ++font_calls; }
};
struct GraphicWin : Win, Buffer {};
struct Dialog {
    ItemList item_list_;
    int column_count_, row_count_, window_x_, window_y_, selected_position_;
    int field_44_, field_64_, field_68_;
    int text_color_a_, text_color_b_, text_color_c_, text_color_d_;
    int text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_;
    int text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_;
    void *font1_, *font2_, *font3_;
};
struct Sprite {
    int w,h,transparent,draw_x,draw_y,draw_calls;
    Sprite() : w(8),h(10),transparent(3),draw_x(0),draw_y(0),draw_calls(0) {}
    int width() { return w; }
    int height() { return h; }
    int transparent_index() { return transparent; }
    void draw(Buffer *,int,int x,int y,int,int) { draw_x=x;draw_y=y;++draw_calls; }
};
struct SpriteBox : GraphicWin, Dialog {
    ItemList list_;
    int field_54_, field_1C_, field_20_, field_4_, field_8_;
    void (*redraw_callback_)(Sprite *,GraphicWin *,int,int,int);
    void draw_item(int);
    SpriteBox() : field_54_(11), field_1C_(1), field_20_(1), field_4_(1),
        field_8_(7), redraw_callback_(0) {}
};
int SpriteScaleNumerator, SpriteScaleDenominator;
static int callback_x, callback_y, callback_id, callback_calls;
static SpriteBox *callback_box;
static Entry *callback_entry;
static void redraw(Sprite *,GraphicWin *,int id,int x,int y) {
    callback_id=id; callback_x=x; callback_y=y; ++callback_calls;
    callback_box->list_.current_=callback_entry;
    callback_box->field_1C_=2;
    callback_box->field_20_=1;
}
BODY

int main() {
    SpriteBox box; GraphicWin parent;
    Sprite sprite, callback_sprite; callback_sprite.w=2; callback_sprite.h=4;
    StringStructPayload label={(char*)"grid"};
    SpriteStruct sprite_payload={&sprite};
    SpriteStruct callback_payload={&callback_sprite};
    Entry entries[6];
    for (int i=0;i<6;++i) {
        entries[i].id=100+i; entries[i].string_=&label;
        entries[i].sprite_=&sprite_payload;
        box.list_.entries[i]=&entries[i]; box.item_list_.entries[i]=&entries[i];
    }
    box.list_.entry_count_=6; box.list_.head_=&entries[0]; box.list_.current_=0;
    box.item_list_.entry_count_=6; box.item_list_.head_=&entries[0]; box.item_list_.current_=0;
    box.row_count_=3; box.column_count_=2;
    box.window_x_=2147483642; box.window_y_=2147483647;
    box.field_44_=6; box.field_64_=24; box.field_68_=20;
    box.selected_position_=5; box.win_parent_=&parent;

    box.redraw_callback_=0;
    box.draw_item(5);
    Buffer *parent_buffer=static_cast<Buffer *>(static_cast<GraphicWin *>(&parent));
    Buffer *control_buffer=static_cast<Buffer *>(static_cast<GraphicWin *>(&box));
    assert(parent_buffer->copy_receiver==parent_buffer);
    assert(parent_buffer->copy_source==control_buffer);
    assert(parent_buffer->copy_sx==-2147483624 && parent_buffer->copy_sy==-2147483606);
    assert(parent_buffer->copy_dx==30 && parent_buffer->copy_dy==43);
    assert(parent_buffer->copy_width==24 && parent_buffer->copy_height==20);
    assert(sprite.draw_calls==1 && sprite.draw_x==31 && sprite.draw_y==48);
    assert(box.box_left==31 && box.box_top==49 && box.box_right==38 && box.box_bottom==66);
    assert(box.box_color_a==7 && box.box_color_b==7);
    assert(box.written_rect.left==45 && box.written_rect.top==43);
    assert(box.written_rect.right==54 && box.written_rect.bottom==63);
    assert(box.write_length==4 && std::strcmp(box.written_text,"grid")==0);

    box.redraw_callback_=redraw; callback_calls=0; callback_box=&box;
    Entry callback_item={999,&label,&callback_payload}; callback_entry=&callback_item;
    box.color_calls=box.font_calls=0;
    box.draw_item(5);
    assert(callback_calls==1 && callback_id==105);
    assert(callback_x==31 && callback_y==48);
    assert(SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
    assert(box.field_1C_==2 && box.field_20_==1);
    assert(box.color_calls==3 && box.font_calls==1);
    assert(box.box_left==33 && box.box_top==50 && box.box_right==36 && box.box_bottom==67);
    return 0;
}
'''


def compile_run(tmp_path: Path, compiler: str, body: str, name: str, success: bool):
    source = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    source.write_text(HARNESS.replace("BODY", body), encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                            "-Werror", str(source), "-o", str(executable)],
                           text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True,
                            timeout=10)
    assert (result.returncode == 0) == success, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_spritebox_draw_axes_copy_and_callback_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_body((ROOT / "src/spritebox.cpp").read_text(encoding="utf-8"))
    compile_run(tmp_path, compiler, body, f"spritebox_draw_{compiler_name}", True)

    mutants = (
        ("area.left = static_cast<int>(field_54_) + x + 4;",
         "area.left = static_cast<int>(field_54_) + y + 4;", "wrong-x-axis"),
        ("parent_buffer->copy(\n        control_buffer,\n        static_cast<int32_t>(static_cast<uint32_t>(Dialog::window_x_) +\n                             static_cast<uint32_t>(x)),",
         "control_buffer->copy(\n        parent_buffer,\n        static_cast<int32_t>(static_cast<uint32_t>(Dialog::window_x_) +\n                             static_cast<uint32_t>(x)),", "wrong-copy-receiver"),
        ("int sprite_x = x + (static_cast<int>(field_54_) - sprite_width) / 2;",
         "int sprite_x = y + (static_cast<int>(field_54_) - sprite_width) / 2;",
         "wrong-sprite-axis"),
    )
    for before, after, name in mutants:
        assert body.count(before) == 1, name
        compile_run(tmp_path, compiler, body.replace(before, after, 1), name, False)
