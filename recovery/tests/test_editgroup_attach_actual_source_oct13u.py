"""Dual-compiler host controls for the literal EditGroup::attach body.

The fixture extracts the current C++ definition unchanged and supplies narrow
recording doubles for the real GraphicWin::init argument order, Buffer color /
font calls, EditBox initialization and callbacks. It tests observable source
behavior, not VC6 layout, EH tables, or the original virtual-base ABI. Its
controlled null allocation is a host branch control and does not prove VC6
throw behavior, SEH cleanup, or unwind equivalence.

The child-style observer changes the installed child between the two opaque
calls to distinguish a fresh receiver read from a cached pointer. This is a
source observation-order control, not a claim that Buffer setters reenter the
game. The independently pinned call census supplies the original authority.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
SIGNATURE = "int EditGroup::attach(GraphicWin *parent, int x, int y, int flags)"


def extract(text, signature):
    assert text.count(signature) == 1, f"expected one definition of {signature}"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unterminated EditGroup::attach body")


def actual_body():
    source = (ROOT / "src/editgroup.cpp").read_text(encoding="utf-8")
    body = extract(source, SIGNATURE)
    assert "reinterpret_cast" not in body
    assert "__asm" not in body and "_emit" not in body
    return body


HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using std::vector;
typedef unsigned int uint32_t;
struct Font { int id; };
static vector<int> events;
static int dimension_result, parent_init_result, child_fail_index = -1;
static int dimensions_columns=2,dimensions_rows=3,dimensions_cell_width=31,dimensions_cell_height=15;
static int callback_count;
static int next_child_id;
static bool fail_child_allocation;
static int old_child_deleted;
static void (*show_callback)(int);
static void fail(const char *name) { fprintf(stderr, "EDITGROUP_ATTACH:%s\n", name); exit(98); }
static void check(bool value, const char *name) { if (!value) fail(name); }
struct Buffer;
static void (*color_observer)(Buffer *);
struct Buffer {
  int color_call, color2_calls, color3_calls, font_calls;
  Buffer() : color_call(0),color2_calls(0),color3_calls(0),font_calls(0) {}
  void set_text_color(int a,int b,int c,int d) { check(a==11&&b==12&&c==13&&d==14,"color1-values"); events.push_back(20); ++color_call; if(color_observer)color_observer(this); }
  void set_text_color2(int a,int b,int c,int d) { check(a==21&&b==22&&c==23&&d==24,"color2-values"); events.push_back(21); ++color2_calls; }
  void set_text_color3(int a,int b,int c,int d) { check(a==31&&b==32&&c==33&&d==34,"color3-values"); events.push_back(22); ++color3_calls; }
  void set_font(Font *a,Font *b,Font *c,int d) { check(a&&a->id==1&&b&&b->id==2&&c&&c->id==3&&d==0,"font-values"); events.push_back(23); ++font_calls; }
};
struct Win { uint32_t iFlags_; Win() : iFlags_(0) {} };
struct GraphicWin : Win, Buffer {
  int init_x, init_y, init_width, init_height, init_flags, init_calls;
  Win *init_parent;
  static void *operator new(size_t bytes) noexcept { if(fail_child_allocation)return 0;return malloc(bytes); }
  static void operator delete(void *p) noexcept { free(p); }
  GraphicWin() : init_x(-1),init_y(-1),init_width(-1),init_height(-1),init_flags(-1),init_calls(0),init_parent(0) {}
  // Same parameter order and types as GraphicWin's production declaration.
  int init(int x,int y,int width,int height,char *title,int flags,Win *parent,int a8,int a9) {
    check(title==0&&a8==0&&a9==0,"parent-init-extra-arguments");
    ++init_calls; init_x=x; init_y=y; init_width=width; init_height=height;
    init_flags=flags; init_parent=parent; events.push_back(10); return parent_init_result;
  }
};
struct Dialog {
  uint32_t field_20_,field_34_,field_40_,field_44_;
  int window_x_,window_y_;
  int column_count_,row_count_,field_5C_,field_60_,field_64_,field_68_,field_6C_;
  Font *font1_,*font2_,*font3_;
  uint32_t text_color_a_,text_color_b_,text_color_c_,text_color_d_;
  uint32_t text_color_2a_,text_color_2b_,text_color_2c_,text_color_2d_;
  uint32_t text_color_3a_,text_color_3b_,text_color_3c_,text_color_3d_;
  uint32_t field_AC_,field_B0_,field_B4_;
  Dialog() : field_20_(0),field_34_(0),field_40_(0),field_44_(0),window_x_(0),window_y_(0),
    column_count_(0),row_count_(0),field_5C_(0),field_60_(0),field_64_(0),field_68_(0),field_6C_(0),
    font1_(0),font2_(0),font3_(0),text_color_a_(11),text_color_b_(12),text_color_c_(13),text_color_d_(14),
    text_color_2a_(21),text_color_2b_(22),text_color_2c_(23),text_color_2d_(24),
    text_color_3a_(31),text_color_3b_(32),text_color_3c_(33),text_color_3d_(34),field_AC_(51),field_B0_(52),field_B4_(53) {}
};
class EditGroup;
struct EditBox : GraphicWin {
  uint32_t field_B14_,field_B1C_,field_B28_,field_B2C_,field_B30_,field_B34_,field_B3C_,field_B40_,field_B44_;
  char text_[256]; int id, init_calls, init_x,init_y,init_width,init_height,init_focus, redraw_calls,show_calls;
  Win *init_parent;
  EditBox() : field_B14_(0),field_B1C_(0),field_B28_(0),field_B2C_(0),field_B30_(0),field_B34_(0),field_B3C_(0),field_B40_(99),field_B44_(0),id(next_child_id++),init_calls(0),init_x(0),init_y(0),init_width(0),init_height(0),init_focus(0),redraw_calls(0),show_calls(0),init_parent(0) { text_[0]=0; events.push_back(30); }
  ~EditBox() { events.push_back(31); if(id==9)++old_child_deleted; }
  int init(int x,int y,int width,int height,Win *parent,int focus) {
    ++init_calls; init_x=x;init_y=y;init_width=width;init_height=height;init_parent=parent;init_focus=focus;
    events.push_back(40+id); return id==child_fail_index ? 7 : 0;
  }
  void set_text(char *s) { events.push_back(60+id); if(s) { strncpy(text_,s,255);text_[255]=0; } else text_[0]=0; }
  void redraw() { ++redraw_calls; events.push_back(80+id); ++callback_count; }
  void show(int value) { check(value==0,"show-value"); ++show_calls; events.push_back(100+id); ++callback_count; if(show_callback)show_callback(id); }
};
class EditGroup : public virtual GraphicWin, public virtual Dialog {
 public:
  EditBox *boxes_[10]; char *text_buffers_[10]; uint32_t limits_[10];
  int label_width_; int edit_width_;
  EditGroup() : label_width_(17),edit_width_(19) {
    for(int i=0;i<10;++i){boxes_[i]=0;text_buffers_[i]=0;limits_[i]=10;}
  }
  int calculate_dimensions() {
    events.push_back(1); ++callback_count;
    Dialog::field_34_=7; Dialog::field_40_=41; Dialog::field_44_=6;
    Dialog::column_count_=dimensions_columns; Dialog::row_count_=dimensions_rows; Dialog::field_5C_=211; Dialog::field_60_=127; Dialog::field_64_=dimensions_cell_width; Dialog::field_68_=dimensions_cell_height; Dialog::field_6C_=61;
    return dimension_result;
  }
  int attach(GraphicWin *parent,int x,int y,int flags);
};
static EditGroup *callback_group;
static EditBox *replaced_child, *replacement_child;
static void replace_child_after_color(Buffer *receiver) {
  if(receiver!=static_cast<Buffer *>(callback_group->boxes_[0]))return;
  replaced_child=callback_group->boxes_[0];
  callback_group->boxes_[0]=replacement_child;
  color_observer=0;
}
static void mutate_geometry_from_show(int id) {
  if(id==0){callback_group->Dialog::column_count_=3;callback_group->Dialog::row_count_=3;callback_group->Dialog::field_64_=47;callback_group->Dialog::field_68_=19;}
  if(id==1)callback_group->Dialog::field_68_=23;
  if(id==3){callback_group->Dialog::field_64_=53;callback_group->Dialog::field_68_=29;}
  if(id==4)callback_group->Dialog::field_68_=31;
}
@BODY@
static void init_group(EditGroup &g) {
  next_child_id=0;callback_count=0;child_fail_index=-1;fail_child_allocation=false;show_callback=0;callback_group=0;color_observer=0;
  dimensions_columns=2;dimensions_rows=3;dimensions_cell_width=31;dimensions_cell_height=15;
  dimension_result=0;parent_init_result=0;
  static Font f1={1},f2={2},f3={3}; g.Dialog::font1_=&f1;g.Dialog::font2_=&f2;g.Dialog::font3_=&f3;
  g.Dialog::field_20_=0x1000; g.Dialog::field_44_=6; g.Dialog::column_count_=2;g.Dialog::row_count_=3;
  for(int i=0;i<10;++i){g.limits_[i]=70+i;g.text_buffers_[i]=(i%2)?(char*)"seed":0;}
}
static void cases() {
  { events.clear(); EditGroup g; init_group(g); check(g.attach(0,4,5,6)==3,"null-parent-result"); check(events.empty(),"null-parent-side-effects"); }
  { events.clear(); EditGroup g; GraphicWin p; init_group(g); dimension_result=0;parent_init_result=9;
    check(g.attach(&p,101,202,0x40)==9,"init-failure-result");check(g.GraphicWin::init_calls==1,"init-failure-call-count");
    check(events.size()==2&&events[0]==1&&events[1]==10,"init-failure-order"); }
  { events.clear(); EditGroup g; GraphicWin p; init_group(g); parent_init_result=0;dimension_result=0;p.iFlags_=0x1000;
    check(g.attach(&p,101,202,0x40)==0,"success-result");
    check(g.Dialog::window_x_==101&&g.Dialog::window_y_==202,"stored-position");
    check(g.GraphicWin::init_x==101&&g.GraphicWin::init_y==202&&g.GraphicWin::init_width==211&&g.GraphicWin::init_height==127&&g.GraphicWin::init_flags==0x40,"parent-init-xywh-flags");
    check(g.GraphicWin::init_parent==&p,"parent-init-parent");
    check(g.Buffer::color_call==1&&g.Buffer::color2_calls==1&&g.Buffer::color3_calls==1&&g.Buffer::font_calls==1,"parent-four-style-calls");
    check(g.boxes_[0]&&g.boxes_[1]&&g.boxes_[2]&&g.boxes_[3]&&g.boxes_[4]&&g.boxes_[5],"row-column-count");
    check(next_child_id==6,"row-column-count");
    check(g.boxes_[6]==0,"row-column-overflow");
    check(g.boxes_[0]->init_parent==static_cast<Win*>(&g)&&g.boxes_[0]->init_focus==1,"child-parent-focus");
    for(int i=0;i<6;++i){EditBox *b=g.boxes_[i];check(b->init_calls==1&&b->redraw_calls==1&&b->show_calls==1,"child-lifecycle");
      const int column=i/3,row=i%3;
      check(b->init_x==36+column*31&&b->init_y==3+row*15&&b->init_width==2&&b->init_height==9,"child-grid-geometry");
      check(b->field_B14_==70u+(unsigned)i&&b->field_B1C_==61&&b->field_B28_==51&&b->field_B2C_==52&&b->field_B30_==53&&b->field_B34_==41,"child-state-copy");
      check(b->field_B40_==0&&b->field_B44_==strlen(b->text_),"child-caret-text-length");
      check((b->field_B3C_&2)!=0,"child-checkbox-flag");
      check(b->color_call==1&&b->font_calls==1,"child-two-style-calls");
      check(b->color2_calls==0,"child-extra-color2");
      check(b->color3_calls==0,"child-extra-color3");
    }
    check(g.boxes_[0]->text_[0]==0&&strcmp(g.boxes_[1]->text_,"seed")==0,"child-text-values");
    check(g.boxes_[0]->init_x!=g.boxes_[0]->init_y,"child-axis-distinction");
    vector<int> expected; expected.push_back(1);expected.push_back(10);expected.push_back(20);expected.push_back(21);expected.push_back(22);expected.push_back(23);
    for(int i=0;i<6;++i){expected.push_back(30);expected.push_back(40+i);expected.push_back(20);expected.push_back(23);expected.push_back(60+i);expected.push_back(80+i);expected.push_back(100+i);}
    check(events==expected,"parent-child-callback-order");check(callback_count==13,"callback-count");
  }
  { events.clear();EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;
    dimensions_columns=2;dimensions_rows=2;callback_group=&g;show_callback=mutate_geometry_from_show;
    check(g.attach(&p,1,2,0)==0,"callback-geometry-result");show_callback=0;
    check(next_child_id==9,"callback-live-column-row-count");
    const int expected_x[9]={36,36,36,83,83,83,142,142,142};
    const int expected_y[9]={3,22,45,3,32,63,3,34,65};
    for(int i=0;i<9;++i){EditBox *b=g.boxes_[i];check(b!=0,"callback-child-installed");
      if(!(b->init_x==expected_x[i]&&b->init_y==expected_y[i]&&b->init_width==2&&b->init_height==9))fprintf(stderr,"callback geometry id=%d got=(%d,%d,%d,%d) want=(%d,%d,2,9)\n",i,b->init_x,b->init_y,b->init_width,b->init_height,expected_x[i],expected_y[i]);
      check(b->init_x==expected_x[i]&&b->init_y==expected_y[i]&&b->init_width==2&&b->init_height==9,"callback-grid-fresh-fields-and-reset");}
  }
  { events.clear(); EditGroup g;GraphicWin p;init_group(g); parent_init_result=0;p.iFlags_=0;g.Dialog::field_20_=0;
    check(g.attach(&p,1,2,0x40)==0,"flag-case-result");check(g.GraphicWin::init_flags==0x100040,"checkbox-parent-flag");
    check((g.boxes_[0]->field_B3C_&2)==0,"checkbox-bit-absent"); }
  { events.clear(); EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;g.Dialog::field_20_=0x100000;
    check(g.attach(&p,1,2,0)==0,"different-bit-result");
    check((g.boxes_[0]->field_B3C_&2)==0,"checkbox-different-bit-absent"); }
  { events.clear(); EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;parent_init_result=0;child_fail_index=2;
    check(g.attach(&p,1,2,0)==7,"child-init-failure-propagation");
    check(g.boxes_[2]!=0&&g.boxes_[2]->redraw_calls==0&&g.boxes_[2]->show_calls==0,"failed-child-remains-installed");
    check(g.boxes_[0]->show_calls==1&&g.boxes_[1]->show_calls==1,"prior-child-effects-remain"); }
  { events.clear();EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;parent_init_result=0;fail_child_allocation=true;
    check(g.attach(&p,1,2,0)==4,"child-allocation-null-result");fail_child_allocation=false;
    check(g.boxes_[0]==0,"child-allocation-null-slot");check(events.size()==6,"child-allocation-before-child-effects"); }
  { events.clear(); EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;parent_init_result=0;
    EditBox *old=new EditBox;old->id=9;g.boxes_[0]=old;old_child_deleted=0;
    check(g.attach(&p,1,2,0)==0,"replace-old-child-result");
    check(g.boxes_[0]!=0&&g.boxes_[0]->id!=9&&old_child_deleted==1,"old-child-deleted-new-child-installed");
    check(events.size()>8&&events[0]==30&&events[1]==1&&events[2]==10&&events[3]==20&&events[4]==21&&events[5]==22&&events[6]==23&&events[7]==31&&events[8]==30,"delete-before-new-constructor"); }
  { events.clear(); EditGroup g;GraphicWin p;init_group(g);p.iFlags_=0x1000;
    dimensions_columns=dimensions_rows=1;callback_group=&g;
    replacement_child=new EditBox;replaced_child=0;color_observer=replace_child_after_color;
    check(g.attach(&p,1,2,0)==0,"style-replacement-result");color_observer=0;
    check(replaced_child&&g.boxes_[0]==replacement_child,"style-child-replaced");
    check(replaced_child->color_call==1&&replaced_child->font_calls==0&&replacement_child->color_call==0&&replacement_child->font_calls==1,"child-font-reloads-receiver");
    check(replacement_child->field_B14_==70&&replacement_child->redraw_calls==1&&replacement_child->show_calls==1,"style-followup-reloads-receiver");
    delete replaced_child;delete replacement_child;
  }
}
int main(){cases();return 0;}
'''


def compile_run(compiler, body, mutation=None):
    if mutation:
        original = body
        if callable(mutation):
            body = mutation(body)
        else:
            old, new = mutation
            assert old in body, f"mutation anchor missing: {old}"
            body = body.replace(old, new, 1)
        assert body != original, "mutation did not change the actual source"
    code = HARNESS.replace("@BODY@", body)
    with tempfile.TemporaryDirectory(prefix="editgroup-attach-host-") as td:
        cpp, exe = Path(td) / "attach.cpp", Path(td) / "attach"
        cpp.write_text(code, encoding="utf-8")
        built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                                "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
                               text=True, capture_output=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], text=True, capture_output=True, timeout=10)


def test_literal_attach_body_on_gcc_and_clang():
    body = actual_body()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "both GCC and Clang are required"
    for compiler in compilers:
        result = compile_run(compiler, body)
        assert result.returncode == 0, result.stdout + result.stderr


def test_compiling_mutants_fail_at_named_runtime_controls():
    body = actual_body()
    mutations = (
        ("flags-order", ("GraphicWin::init(x, y, Dialog::field_5C_",
                          "GraphicWin::init(x, flags, Dialog::field_5C_"), "parent-init-xywh-flags"),
        ("row-column", ("column < Dialog::column_count_",
                         "column < Dialog::row_count_"), "row-column-count"),
        ("omit-show", ("boxes_[index]->show(0);", "/* mutant omitted show */"), "child-lifecycle"),
        ("omit-text", ("boxes_[index]->set_text(text_buffers_[index]);",
                        "/* mutant omitted set_text */"), "child-text-values"),
        ("extra-child-color2", ("            boxes_[index]->set_font(",
            "            boxes_[index]->set_text_color2(Dialog::text_color_2a_, Dialog::text_color_2b_, Dialog::text_color_2c_, Dialog::text_color_2d_);\n            boxes_[index]->set_font("), "child-extra-color2"),
        ("extra-child-color3", ("            boxes_[index]->set_font(",
            "            boxes_[index]->set_text_color3(Dialog::text_color_3a_, Dialog::text_color_3b_, Dialog::text_color_3c_, Dialog::text_color_3d_);\n            boxes_[index]->set_font("), "child-extra-color3"),
        ("omit-parent-color2", ("    buffer->set_text_color2(Dialog::text_color_2a_, Dialog::text_color_2b_,\n                            Dialog::text_color_2c_, Dialog::text_color_2d_);",
            "    /* mutant omitted parent color2 */"), "parent-four-style-calls"),
        ("cached-child", lambda source: source.replace(
            "            boxes_[index]->set_text_color(",
            "            EditBox *const style_child = boxes_[index];\n            style_child->set_text_color(", 1).replace(
            "            boxes_[index]->set_font(", "            style_child->set_font(", 1),
            "child-font-reloads-receiver"),
    )
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        assert compiler, compiler_name + " required"
        for name, mutation, diagnostic in mutations:
            result = compile_run(compiler, body, mutation)
            assert result.returncode == 98, f"{name}: {result.stdout}{result.stderr}"
            assert "EDITGROUP_ATTACH:" + diagnostic in result.stderr, (name, result.stderr)


def test_cumulative_column_geometry_mutant_is_caught_by_live_callbacks():
    body = actual_body()
    def cumulative_columns(source):
        replacements = (
            ("    int index = 0;\n    for (int column", "    int index = 0;\n    int column_offset = 0;\n    for (int column"),
            ("        const int column_offset =\n            column * static_cast<int>(Dialog::field_64_);\n", ""),
            ("            ++index;\n        }\n    }\n    return 0;", "            ++index;\n        }\n        column_offset += static_cast<int>(Dialog::field_64_);\n    }\n    return 0;"),
        )
        for old, new in replacements:
            assert old in source, f"cumulative-column mutation anchor missing: {old}"
            source = source.replace(old, new, 1)
        return source

    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        assert compiler, compiler_name + " required"
        result = compile_run(compiler, body, cumulative_columns)
        assert result.returncode == 98, result.stdout + result.stderr
        assert "EDITGROUP_ATTACH:callback-grid-fresh-fields-and-reset" in result.stderr


def test_pinned_parent_and_child_style_call_census():
    import hashlib
    import os
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    raw = path.read_bytes()
    pin = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
    assert hashlib.sha256(raw).hexdigest() == pin
    image = PeImage(raw, pin)
    ops = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x611AF0, 0x611F79)), 0x611AF0))
    style_targets = {'0x5dacb0', '0x5dace0', '0x5dad10', '0x5dac70'}
    assert [(i.address, i.op_str) for i in ops if i.mnemonic == 'call' and i.op_str in style_targets] == [
        (0x611BCF, '0x5dacb0'), (0x611C02, '0x5dace0'),
        (0x611C35, '0x5dad10'), (0x611C63, '0x5dac70'),
        (0x611E0C, '0x5dacb0'), (0x611E39, '0x5dac70'),
    ]
    instructions = {i.address: (i.mnemonic, i.op_str) for i in ops}
    for address in (0x611E03, 0x611E30):
        assert instructions[address] == ('mov', 'ecx, dword ptr [esi]')
    for address in (0x611E06, 0x611E33):
        assert instructions[address] == ('add', 'ecx, 0x444')
