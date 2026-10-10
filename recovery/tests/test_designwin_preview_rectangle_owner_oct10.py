"""Actual preview geometry fragments and whole-definition preservation controls.

The host executes only rectangle initialization/clip/update fragments, not the
complete GUI methods. Fresh native CodeView/build checks establish the separate
32-bit member layout. No original debug-name or full-game proof is inferred.
"""
from dataclasses import fields
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.source import read_text, write
from match_recovery.layout_source import parse_members
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control


ROOT = Path(__file__).resolve().parents[2]
BASE = "47785ab9c9e9644ac1723b9df9689d0d9e299365"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def source():
    return (ROOT / "src/designwin.cpp").read_text()


def test_complete_definitions_change_only_the_confirmed_rectangle_accesses():
    original = subprocess.check_output(
        ["git", "show", BASE + ":src/designwin.cpp"], cwd=ROOT).decode()
    current = source()
    for signature in ("void DesignWin::init()", "void DesignWin::draw_flash()"):
        old = extract_definition(original, signature)
        new = extract_definition(current, signature)
        new = new.replace("&preview_panel_rect_",
                          "reinterpret_cast<RECT *>(&field_B1C_)")
        new = new.replace("RECT rc = preview_panel_rect_;",
                          "RECT rc = *reinterpret_cast<RECT *>(&field_B1C_);")
        for edge, name in (("left", "field_B1C_"), ("top", "field_B20_"),
                           ("right", "field_B24_"), ("bottom", "field_B28_")):
            new = new.replace("preview_panel_rect_." + edge, name)
        assert new == old
    rows = read_text(current, ROOT / "src/designwin.cpp")
    assert write(current, rows) == current
    previous = {r.address: r for r in read_text(original, ROOT / "src/designwin.cpp")}
    assert {r.address for r in rows} == set(previous)
    for row in rows:
        old = previous[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(old, field.name)
        def lessons(record):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines))
                    for x in record.origin]
        assert lessons(row) == lessons(old)


def test_preview_binding_is_one_real_rectangle_not_four_cast_words():
    header = (ROOT / "src/designwin.h").read_text()
    members = parse_members(header, "src/designwin.h", PIN, address_space="pe:va")
    preview, = [m for m in members if m.class_name == "DesignWin"
                and m.member == "preview_panel_rect_"]
    assert (preview.offset, preview.class_type, preview.evidence.address) == (
        0xB1C, "tagRECT", 0x43DD50)
    assert re.search(r"(?m)^  RECT preview_panel_rect_;", header)
    for old in ("field_B1C_", "field_B20_", "field_B24_", "field_B28_"):
        assert old not in header and old not in source()


def control():
    current = source()
    init = extract_definition(current, "void DesignWin::init()")
    init_fragment = init[init.index("    field_B0C_rect_ ="):
                         init.index("    info_panel_rect_ =")]
    flash = extract_definition(current, "void DesignWin::draw_flash()")
    clip = flash[flash.index("    canvas->set_clip("):
                 flash.index("    rc.left += 3;")]
    update = flash[flash.index("    MainInterfaceGlobal.soft_update("):
                   flash.index("    canvas->restore_clip();")]
    header = (ROOT / "src/designwin.h").read_text()
    declarations = "\n".join(re.search(r"(?m)^  RECT " + name + r";", header)[0]
                              for name in ("field_B0C_rect_", "preview_panel_rect_"))
    return r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
struct tagRECT {int32_t left,top,right,bottom;};
typedef tagRECT RECT;
static_assert(sizeof(RECT)==16,"original edge width");
struct Style {} InterfaceBoxStyles[4];
struct MainInterface {
  RECT frame,hit; RECT *updated=0;
  RECT main_frame_rect(){return frame;}
  RECT status_hit_rect(){return hit;}
  void soft_update(RECT *r){updated=r;}
} MainInterfaceGlobal;
struct Buffer {
  RECT *clip=0;RECT box;int boxes=0;
  void set_clip(RECT *r){clip=r;}
  void box_sprite(RECT *r,Style *style){assert(style==&InterfaceBoxStyles[3]);box=*r;++boxes;}
} Canvas;
struct DesignWin {
DECLARATIONS
  void init_preview(){
INIT
  }
  void draw_preview(){
    Buffer *canvas=&Canvas;
CLIP
UPDATE
  }
};
static bool same(const RECT&a,const RECT&b){return !std::memcmp(&a,&b,sizeof(a));}
int main(){
  DesignWin win;
  const RECT frames[]={{0,0,100,100},{-20,-15,160,120},{10,20,5,3}};
  const int tops[]={25,-5,4};
  for(int i=0;i<3;++i){
    MainInterfaceGlobal.frame=frames[i];MainInterfaceGlobal.hit={1,tops[i],60,80};
    win.init_preview();
    RECT expected={frames[i].left+3,frames[i].top+3,frames[i].right-3,tops[i]-3};
    assert(same(win.preview_panel_rect_,expected));
    assert(win.field_B0C_rect_.left==frames[i].left+4);
    Canvas.clip=0;Canvas.boxes=0;MainInterfaceGlobal.updated=0;
    win.draw_preview();
    assert(Canvas.clip==&win.preview_panel_rect_);
    assert(Canvas.boxes==1&&same(Canvas.box,expected));
    assert(MainInterfaceGlobal.updated==&win.preview_panel_rect_);
    assert(same(win.preview_panel_rect_,expected));
  }
}
'''.replace("DECLARATIONS", declarations).replace("INIT", init_fragment).replace(
        "CLIP", clip).replace("UPDATE", update)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_preview_fragments_and_damage_controls(tmp_path, compiler):
    run_control(tmp_path, compiler, control(), None)
    for old, new in (
        ("preview_panel_rect_.bottom = field_B0C_rect_.top;",
         "preview_panel_rect_.bottom = stat_panel.bottom;"),
        ("preview_panel_rect_.right -= 3;", "preview_panel_rect_.right += 3;"),
        ("canvas->set_clip(&preview_panel_rect_);", "canvas->set_clip(&field_B0C_rect_);"),
        ("MainInterfaceGlobal.soft_update(&preview_panel_rect_);",
         "MainInterfaceGlobal.soft_update(&field_B0C_rect_);"),
    ):
        run_control(tmp_path, compiler, control(), (old,new))
