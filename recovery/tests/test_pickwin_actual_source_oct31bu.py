"""Actual PickWin RECT alias, callback reread and signed geometry controls.

Reuses all existing owner/lifetime oracles; doubles expose pinned call boundaries,
not claims that actual renderer/string providers mutate the original object.
"""
from pathlib import Path
import shutil
import subprocess
import pytest
import test_pickwin_owner_behavior as previous

ROOT = Path(__file__).resolve().parents[2]

DECLARATIONS = r'''
struct PickWin;
static PickWin *active;
static int control_mode, box_calls, dialog_calls;
static std::vector<int> vline_x;
static void observe_box(RECT *, int *);
static void observe_label(RECT *);
static void observe_extract(RECT *);
static void observe_dialog(RECT *);
static void observe_text_colour();
static std::vector<int> label_ids;
static std::vector<std::array<int,5>> text_geometry;
static std::vector<std::string> text_names;
static void observe_label_lookup(int);
static void observe_uppercase();
static void observe_text_button(const char *,int,int,int,int,int);
'''
HOOKS = r'''
static void observe_label_lookup(int id) {
  if (!active || control_mode != 6) return;
  assert(StringTemp[0]==0);
  label_ids.push_back(id);
  if (id==0x45E) {
    active->field_9210_=101; active->field_9214_=202;
    active->field_9218_=131; active->field_921C_=242;
    std::strcpy(StringTemp,"prefix:");
  }
}
static void observe_uppercase() {
  if (active && control_mode==6 && label_ids.back()==0x66) {
    active->field_9230_=701; active->field_9234_=802;
    active->field_9238_=771; active->field_923C_=882;
  }
}
static void observe_text_button(const char *name,int id,int x,int y,int width,int height) {
  if (!active || control_mode!=6) return;
  assert(name==StringTemp);
  text_geometry.push_back({id,x,y,width,height});
  text_names.push_back(name);
}
static void observe_box(RECT *area, int *) {
  if (!active || control_mode != 3) return;
  ++box_calls;
  if (box_calls == 1) {
    assert(area == &active->frame_rect_);
    area->right = 901;
    active->header_panel_rect_ = {10,20,110,60};
  } else if (box_calls == 2) {
    assert(area == &active->header_panel_rect_);
    assert(area->left == 10 && area->top == 20 && area->right == 110 && area->bottom == 60);
  } else if (box_calls == 3) {
    assert(area == &active->header_panel_rect_);
    assert(area->left == 13 && area->top == 23 && area->right == 107 && area->bottom == 57);
    *area = {100,200,300,400};
  }
}
static void observe_label(RECT *area) {
  if (!active || control_mode != 3) return;
  assert(area == &active->header_panel_rect_);
  assert(area->left == 100 && area->top == 200 && area->right == 300 && area->bottom == 400);
  *area = {200,300,400,500};
}
static void observe_extract(RECT *area) {
  if (!active || control_mode != 1) return;
  switch (trace.extract_rectangles.size()) {
    case 1: active->field_A14_=40; active->field_A1C_=11; active->field_A18_=8; active->field_A20_=7; area->right=123456; break;
    case 2: active->field_A14_=70; active->field_A1C_=13; active->field_A18_=20; active->field_A20_=9; area->right=654321; break;
    case 3: active->field_A18_=30; active->field_A20_=5; break;
    case 10: active->field_A1C_=5; active->field_A20_=9; active->field_A18_=400; break;
    case 11: active->field_A14_=4; active->field_A1C_=7; active->field_A20_=3; active->field_A18_=600; break;
  }
}
static void observe_dialog(RECT *area) {
  if (!active || control_mode != 2) return;
  assert(area == &active->list_area_rect_);
  assert(area->left == 336 + dialog_calls*7);
  area->left += 7;
  ++dialog_calls;
}
static void observe_text_colour() {
  if (active && control_mode == 5) active->field_A1C_ = uint32_t(-3);
}
'''
MAIN = r'''
int main() {
  assert(sizeof(LONG)==4 && sizeof(RECT)==16);
  assert(baseline_controls()==0);
  PickWin pick; active=&pick;
  reset_trace(); control_mode=1;
  pick.init(selection_callback,0,0);
  assert((trace.extract_rectangles[1] == std::array<LONG,4>{52,8,63,15}));
  assert((trace.extract_rectangles[2] == std::array<LONG,4>{84,20,97,29}));
  assert((trace.extract_rectangles[3] == std::array<LONG,4>{1,36,14,41}));
  assert((trace.extract_rectangles[10] == std::array<LONG,4>{1,410,6,419}));
  assert((trace.extract_rectangles[11] == std::array<LONG,4>{4,604,11,607}));
  assert((trace.extract_xy[0] == std::array<int,5>{4,608,7,13,0}));

  reset_trace(); control_mode=2; dialog_calls=0;
  pick.init(selection_callback,0,0);
  assert(dialog_calls==1 && pick.list_area_rect_.left==343);
  pick.on_button_clicked(0x102);
  assert(dialog_calls==2 && pick.list_area_rect_.left==350);

  reset_trace(); control_mode=3; box_calls=0;
  pick.on_redraw();
  assert(box_calls==4 && pick.frame_rect_.right==901);
  assert(pick.header_panel_rect_.left==197 && pick.header_panel_rect_.top==297);
  assert(pick.header_panel_rect_.right==403 && pick.header_panel_rect_.bottom==503);

  reset_trace(); control_mode=4; pick.buttonGroup_.field_84_=0x105;
  pick.field_9240_=uint32_t(-11); pick.field_9248_=uint32_t(-2); pick.list_area_rect_.left=-8;
  vline_x.clear(); pick.on_redraw();
  assert(vline_x.size()==2 && vline_x[0]==-6 && vline_x[1]==-10);
  pick.field_9240_=0x7fffffffU; pick.field_9248_=0x7ffffff0U; pick.list_area_rect_.left=0x7fffffff;
  vline_x.clear(); pick.on_redraw();
  assert(vline_x.size()==2 && vline_x[0]==-8 && vline_x[1]==-13);

  reset_trace(); control_mode=5;
  pick.init(selection_callback,0,0);
  assert(pick.frame_rect_.right==-3 && pick.header_panel_rect_.right==-7);
  assert(pick.field_9220_==5 && pick.field_9228_==1); // signed(-17/3), then signed(-15/4)-1
  reset_trace(); control_mode=6; label_ids.clear(); text_geometry.clear(); text_names.clear();
  pick.init(selection_callback,0,1);
  assert((label_ids==std::vector<int>{0x45B,0x45C,0x45D,0x45E,0x45F,0x66,0x65,0x460,0x461}));
  assert(text_geometry.size()==6);
  assert((text_geometry[0]==std::array<int,5>{0x106,101,202,30,40}));
  assert((text_geometry[2]==std::array<int,5>{0x103,701,802,70,80}));
  assert(text_names[0]=="prefix:LABEL_45e");
  assert(text_names[2]=="LABEL_66");
  active=0;
}
'''


def harness(mutation):
    bodies, list_body, global_body = previous.extract_pickwin_sources()
    if mutation == "copied_panel":
        start = bodies.index("void PickWin::on_redraw() {")
        end = bodies.index("\n}", start) + 2
        block = bodies[start:end]
        changed = block.replace("header_panel_rect_", "copy")
        changed = changed.replace("void PickWin::on_redraw() {", "void PickWin::on_redraw() {\n  RECT copy = header_panel_rect_;", 1)
        assert changed != block
        bodies = bodies[:start] + changed + bodies[end:]
    elif mutation == "copied_list":
        old = "static_cast<Dialog &>(listBox_).init(&list_area_rect_, 0);"
        assert bodies.count(old) == 2
        bodies = bodies.replace(old, "{ RECT copy=list_area_rect_; static_cast<Dialog &>(listBox_).init(&copy,0); }")
    elif mutation == "stale_sprite":
        old = "field_A14_ += field_A1C_ + 1;"
        assert bodies.count(old) == 2
        bodies = bodies.replace(old, "field_A14_ = static_cast<uint32_t>(area.right) + 1u;", 1)
    elif mutation == "icon_axes":
        old = "static_cast<LONG>(field_A14_ + field_A1C_),\n                 static_cast<LONG>(field_A18_ + field_A20_)};\n    sprites4_"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, old.replace("field_A1C_", "AXIS").replace("field_A20_", "field_A1C_").replace("AXIS", "field_A20_"), 1)
    elif mutation == "unsigned_half":
        old = "pickwin_geometry_value(field_9240_ + field_9248_) / 2"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, "(field_9240_ + field_9248_) / 2", 1)
    elif mutation == "unsigned_third":
        old = "pickwin_geometry_value(static_cast<uint32_t>(header_panel_rect_.right)\n      - static_cast<uint32_t>(header_panel_rect_.left) - 6u) / 3"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, "(static_cast<uint32_t>(header_panel_rect_.right)\n      - static_cast<uint32_t>(header_panel_rect_.left) - 6u) / 3", 1)
    elif mutation == "stale_text_geometry":
        preparation = "  StringTemp[0] = '\\0';\n  strcat(StringTemp, label_get(0x45E));\n"
        rect = "  area.left = pickwin_geometry_value(field_9210_); area.top = pickwin_geometry_value(field_9214_);\n  area.right = pickwin_geometry_value(field_9218_); area.bottom = pickwin_geometry_value(field_921C_);\n"
        assert bodies.count(preparation + rect) == 1
        bodies = bodies.replace(preparation + rect, rect + preparation, 1)
    elif mutation == "missing_scratch_copy":
        old = "strcat(StringTemp, label_get(0x45E));"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, "(void)label_get(0x45E);", 1)
    elif mutation == "offset_label":
        assert bodies.count("label_get(0x45E)") == 1
        bodies = bodies.replace("label_get(0x45E)", "label_get(0x1178)", 1)
    fixture = (ROOT / "recovery/tests/pickwin_owner_host_control.cpp").read_text()
    fixture = fixture.replace("struct Buffer {", DECLARATIONS + "\nstruct Buffer {", 1)
    fixture = fixture.replace("char *label_get(int id) {", "static void observe_label_lookup(int);\nchar *label_get(int id) {\n  observe_label_lookup(id);", 1)
    fixture = fixture.replace("void CharUpperA(char *s) { for (; *s; ++s) if (*s >= 'a' && *s <= 'z') *s -= 32; }", "static void observe_uppercase();\nvoid CharUpperA(char *s) { for (; *s; ++s) if (*s >= 'a' && *s <= 'z') *s -= 32; observe_uppercase(); }", 1)
    fixture = fixture.replace('int init(const char *, int, int, int, int, int, Win *, int) { note("button_init"); return 0; }', 'int init(const char *n,int id,int x,int y,int w,int h,Win *,int) { note("button_init"); observe_text_button(n,id,x,y,w,h); return 0; }', 1)

    fixture = fixture.replace('int box_sprite(RECT *, int *) { note("box_sprite"); return 0; }', 'int box_sprite(RECT *r, int *s) { note("box_sprite"); observe_box(r,s); return 0; }', 1)
    fixture = fixture.replace('int write_cent_l(char *, RECT *, int) { note("write_cent_l"); return 0; }', 'int write_cent_l(char *, RECT *r, int) { note("write_cent_l"); observe_label(r); return 0; }', 1)
    fixture = fixture.replace('void vline(int, int, int, int) { note("vline"); }', 'void vline(int x,int,int,int) { note("vline"); if(active && control_mode==4)vline_x.push_back(x); }', 1)
    fixture = fixture.replace('void set_text_color(int, int, int, int) { note("set_text_color"); }', 'void set_text_color(int,int,int,int) { note("set_text_color"); observe_text_colour(); }', 1)
    fixture = fixture.replace('note("sprite_extract_rect"); return 0;', 'note("sprite_extract_rect"); observe_extract(area); return 0;', 1)
    fixture = fixture.replace('int init(RECT *, int) { note("dialog_init"); return 0; }', 'int init(RECT *r,int) { note("dialog_init"); observe_dialog(r); return 0; }', 1)
    fixture = fixture.replace(previous.BODY_MARKERS[0], HOOKS + "\n" + bodies).replace(previous.BODY_MARKERS[1],list_body).replace(previous.BODY_MARKERS[2],global_body)
    assert fixture.count("int main() {") == 1
    fixture = fixture.replace("int main() {", "int baseline_controls() {", 1)
    # The original main relies on its implicit return; an ordinary helper cannot.
    closing = fixture.rfind("\n}")
    assert closing >= 0
    fixture = fixture[:closing] + "\n  return 0;" + fixture[closing:]
    return fixture + MAIN


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None,"copied_panel","copied_list","stale_sprite","icon_axes","unsigned_half","unsigned_third","stale_text_geometry","missing_scratch_copy","offset_label"])
def test_pickwin_original_alias_and_geometry_effects(tmp_path, compiler, mutation):
    cxx=shutil.which(compiler)
    if not cxx: pytest.skip(compiler+" unavailable")
    source=tmp_path/"pickwin.cpp";exe=tmp_path/"pickwin"
    source.write_text(harness(mutation))
    built=subprocess.run([cxx,"-std=c++17","-O1","-Wall","-Wextra","-fsanitize=undefined","-fno-sanitize-recover=all",str(source),"-o",str(exe)],capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(exe)],capture_output=True,text=True)
    assert (result.returncode==0)==(mutation is None),result.stderr


def test_pinned_rect_aliases_and_four_byte_label_slots():
    from hashlib import sha256
    import os
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image = Path(os.environ.get("OPENSMACX_IMAGE", "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    if not image.is_file(): pytest.skip("pinned original unavailable")
    raw = image.read_bytes()
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest() == pin
    pe = PeImage(raw, pin)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    def instructions(start, end):
        return {i.address: (i.mnemonic, i.op_str) for i in md.disasm(pe.read(Span(start,end)),start)}
    init = instructions(0x4887A0,0x48965F)
    for address, register, offset, slot in (
        (0x48904A,"eax",0x116C,0x45B), (0x4890DF,"ecx",0x1170,0x45C),
        (0x489178,"eax",0x1174,0x45D), (0x4892B0,"edx",0x1178,0x45E),
        (0x48933D,"eax",0x117C,0x45F), (0x48934B,"ecx",0xABC,0x2AF),
        (0x4893D8,"eax",0x198,0x66), (0x48946C,"eax",0x194,0x65),
        (0x489504,"eax",0x1180,0x460), (0x48958B,"eax",0x1184,0x461),
    ):
        assert init[address] == ("mov", f"eax, dword ptr [{register} + {offset:#x}]")
        assert offset == slot * 4  # PE32 LPSTR entries; each original load is DWORD.
    assert init[0x4892A9] == ("mov", "byte ptr [0x9b86a0], 0")
    assert init[0x4892B7] == ("call", "0x6169a0")
    assert init[0x4892C2] == ("call", "0x645470")
    assert init[0x4892C7] == ("mov", "eax, dword ptr [esi + 0x9214]")
    redraw = instructions(0x489660,0x489AF7)
    assert redraw[0x489678] == ("lea", "eax, [esi + 0x623c]")
    assert redraw[0x48968B] == ("lea", "edi, [esi + 0x624c]")
    button = instructions(0x489BA0,0x48A798)
    assert button[0x489DE2] == ("lea", "eax, [esi + 0x86ac]")
    assert button[0x48A214] == ("lea", "eax, [esi + 0x86ac]")
    assert button[0x489EE9] == ("mov", "eax, dword ptr [eax + 0x1188]")
    assert 0x1188 == 0x462 * 4
