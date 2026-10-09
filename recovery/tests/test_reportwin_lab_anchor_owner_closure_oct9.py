from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_methods():
    source = (ROOT / "src/reportwin.cpp").read_text()
    helper_start = source.index("static int reportwin_signed_coordinate(")
    helper = source[helper_start:source.index("\n}\n", helper_start) + 2]
    line_start = source.index("void ReportWin::labs_line(")
    line = source[line_start:source.index("\n}\n", line_start) + 3]
    pass_start = source.index("void ReportWin::on_button_passover(")
    passed = source[pass_start:source.index("\n}\n", pass_start) + 3]
    return helper, line, passed


def harness(line, passed):
    helper, _, _ = source_methods()
    return r'''#include <stdint.h>
#include <stdio.h>
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {} InterfaceBoxStyles[8];
struct Buffer;
struct Sprite {
    int transparent_index() { return 7; }
    void draw(Buffer *, int byte, int x, int y);
    static int arrow_draws, arrow_byte, arrow_x, arrow_y, arrow_datalink_receiver;
};
int Sprite::arrow_draws=0; int Sprite::arrow_byte=0;
int Sprite::arrow_x=0; int Sprite::arrow_y=0;
int Sprite::arrow_datalink_receiver=0;
Sprite g_IFACE_TECH_TREE_ARROW_SPRITES[3];
struct Event { int kind, l, t, r, b, style, datalink_receiver; } events[64];
int event_count, mutate_clip_once, mutate_restore_once, mutate_box_once;
struct ReportWin;
ReportWin *callback_owner;
struct Buffer {
    virtual ~Buffer() {}
    virtual void set_clip(RECT *r);
    virtual void box_sprite(RECT *r, BoxSpriteParams *s);
    virtual void restore_clip();
};
struct GraphicWin : Buffer { int soft_updates; GraphicWin():soft_updates(0) {}
    void soft_update();
};
struct FlatButton { RECT r; RECT outer_rect() { return r; } };
struct DatalinkWin : GraphicWin {} DatalinkMain;
int IfaceIdToSlotStorage[1]={0}, ActiveIfaceIdStorage=0;
void Sprite::draw(Buffer *buffer, int byte, int x, int y) {
    ++arrow_draws; arrow_byte=byte; arrow_x=x; arrow_y=y;
    arrow_datalink_receiver=(buffer==static_cast<Buffer *>(&DatalinkMain));
}
void GraphicWin::soft_update() {
    ++soft_updates;
    events[event_count++]=(Event){4,0,0,0,0,0,
        this==static_cast<GraphicWin *>(&DatalinkMain)};
}
struct ReportIfFixture { int field_8_; } ReportIfGlobal={2};
int ActiveIfaceIdStorageUnused;
struct ReportWin : GraphicWin {
    uint32_t lab_prerequisite_x_,lab_center_x_,lab_child_x_,lab_center_y_;
    uint32_t lab_three_child_upper_y_,lab_three_child_lower_y_,lab_four_child_top_y_,lab_upper_inner_y_;
    uint32_t lab_lower_inner_y_,lab_four_child_bottom_y_,field_59A8_;
    uint32_t field_A24_,field_A28_,field_A2C_,field_A34_;
    int field_59A0_,field_59A4_;
    RECT report_work_rect_;
    FlatButton flatButtons1_[7];
    void labs_line(int,int,int,int,int,int,int);
    void on_button_passover(int,int);
};
void Buffer::set_clip(RECT *r) {
    events[event_count++] = (Event){1,r->left,r->top,r->right,r->bottom,0,
        this==static_cast<Buffer *>(&DatalinkMain)};
    if (mutate_clip_once && callback_owner) {
        callback_owner->lab_prerequisite_x_=static_cast<uint32_t>(-31);
        mutate_clip_once=0;
    }
}
void Buffer::box_sprite(RECT *r, BoxSpriteParams *s) {
    events[event_count++] = (Event){2,r->left,r->top,r->right,r->bottom,
        static_cast<int>(s-InterfaceBoxStyles),
        this==static_cast<Buffer *>(&DatalinkMain)};
    if (mutate_box_once && callback_owner) {
        callback_owner->field_59A8_=static_cast<uint32_t>(-77);
        callback_owner->lab_center_x_=static_cast<uint32_t>(-33);
        mutate_box_once=0;
    }
}
void Buffer::restore_clip() {
    events[event_count++] = (Event){3,0,0,0,0,0,
        this==static_cast<Buffer *>(&DatalinkMain)};
    if (mutate_restore_once && callback_owner) {
        callback_owner->field_59A8_=static_cast<uint32_t>(-77);
        callback_owner->lab_center_x_=static_cast<uint32_t>(-33);
        mutate_restore_once=0;
    }
}
''' + helper + line + passed + r'''
static void reset() {
    event_count=0; mutate_clip_once=mutate_restore_once=mutate_box_once=0;
    Sprite::arrow_draws=0;
    Sprite::arrow_datalink_receiver=0;
    DatalinkMain.soft_updates=0;
    IfaceIdToSlotStorage[0]=0; ActiveIfaceIdStorage=0;
    ReportIfGlobal.field_8_=2;
    callback_owner=0;
}
int main() {
    if (reportwin_signed_coordinate(0x7FFFFFFFu)!=2147483647
        || reportwin_signed_coordinate(0x80000000u)!=(-2147483647-1)
        || reportwin_signed_coordinate(0xFFFFFFFFu)!=-1) return 11;
    ReportWin w; callback_owner=&w;
    // Negative 32-bit coordinate words must remain signed in the drawing path.
    w.lab_prerequisite_x_=static_cast<uint32_t>(-40);
    w.lab_center_x_=static_cast<uint32_t>(-20);
    w.lab_child_x_=static_cast<uint32_t>(40);
    w.lab_center_y_=static_cast<uint32_t>(-10);
    w.lab_three_child_upper_y_=static_cast<uint32_t>(-30);
    w.lab_three_child_lower_y_=static_cast<uint32_t>(10);
    w.lab_four_child_top_y_=static_cast<uint32_t>(-50);
    w.lab_upper_inner_y_=static_cast<uint32_t>(-25);
    w.lab_lower_inner_y_=static_cast<uint32_t>(5);
    w.lab_four_child_bottom_y_=static_cast<uint32_t>(25);
    w.field_59A8_=static_cast<uint32_t>(-5);
    w.field_A24_=20; w.field_A28_=static_cast<uint32_t>(-80);
    w.field_A2C_=static_cast<uint32_t>(-12); w.field_A34_=static_cast<uint32_t>(-4);
    w.field_59A0_=15; w.field_59A4_=40;

    // Connector callback mutates the stored left anchor; the later rectangle
    // write must reload it after set_clip, not reuse a pre-callback snapshot.
    mutate_clip_once=1;
    w.labs_line(-20,0,0,-25,15,40,0);
    if (event_count < 4 || events[0].kind!=1 || events[1].kind!=2) return 1;
    if (events[1].l!=-20) return 2; // -31 - 4 + 15 after callback
    if (Sprite::arrow_draws!=1 || Sprite::arrow_x!=-30) return 3;

    reset(); callback_owner=&w;
    // Callback mutation before the common tail must be observed by its later
    // center computation. This path uses the non-special right connector.
    w.lab_prerequisite_x_=12; w.lab_center_x_=20; w.lab_child_x_=80;
    w.lab_center_y_=30; w.field_59A8_=40;
    mutate_box_once=1;
    w.labs_line(1,1,1,1,10,20,0);
    int tail_box=-1;
    for (int i=0;i<event_count;++i)
        if (events[i].kind==2) tail_box=i;
    if (tail_box<0 || events[tail_box].l!= -77-4-33+7-7) return 4;
    if (events[tail_box].style!=3) return 5;

    reset(); callback_owner=&w;
    w.lab_prerequisite_x_=12; w.lab_center_x_=20; w.lab_child_x_=80;
    w.lab_center_y_=30; w.field_59A8_=40;
    mutate_restore_once=1;
    w.labs_line(1,1,1,1,10,20,0);
    tail_box=-1;
    for (int i=0;i<event_count;++i)
        if (events[i].kind==2) tail_box=i;
    if (tail_box<0 || events[tail_box].l!=-114) return 10;

    reset(); callback_owner=&w;
    w.lab_center_x_=static_cast<uint32_t>(-22);
    w.lab_center_y_=static_cast<uint32_t>(-11);
    w.field_59A8_=static_cast<uint32_t>(-6);
    w.field_A28_=static_cast<uint32_t>(-90);
    w.field_A2C_=static_cast<uint32_t>(-20);
    w.field_A34_=static_cast<uint32_t>(-10);
    w.field_A24_=8;
    w.flatButtons1_[0].r=(RECT){-12,-8,18,22};
    w.on_button_passover(0x378,1);
    if (events[0].kind!=2 || events[0].l!=-93 || events[0].t!=-28
        || events[0].r!=-93 || events[0].b!=-5 || events[0].style!=5) return 6;
    if (events[event_count-1].kind!=4 || w.soft_updates!=1) return 7;
    if (events[event_count-3].kind!=2 || events[event_count-2].kind!=3) return 8;
    if (events[event_count-3].l!=-7 || events[event_count-3].t!=9) return 9;

    // Slot 8 draws on Datalink's Buffer, while the receiver remains this
    // ReportWin for its chart anchors. Passover indices 3 and 4 reverse the
    // line endpoint order; state 0/1 still select styles 3/5 respectively.
    reset(); callback_owner=&w; IfaceIdToSlotStorage[0]=8; w.soft_updates=0;
    w.lab_prerequisite_x_=12; w.lab_center_x_=20; w.lab_child_x_=80;
    w.lab_center_y_=30; w.lab_three_child_upper_y_=45;
    w.lab_three_child_lower_y_=65; w.lab_upper_inner_y_=10;
    w.lab_lower_inner_y_=50; w.lab_four_child_top_y_=0;
    w.lab_four_child_bottom_y_=75; w.field_59A8_=40;
    w.field_A24_=10; w.field_A28_=30; w.field_A2C_=20; w.field_A34_=80;
    w.field_59A0_=15; w.field_59A4_=40;
    w.labs_line(1,45,0,0,15,40,0);
    if (!Sprite::arrow_datalink_receiver || Sprite::arrow_x!=70
        || Sprite::arrow_y!=59) return 12;
    for (int i=0;i<event_count;++i)
        if (events[i].datalink_receiver!=1) return 13;

    reset(); callback_owner=&w; IfaceIdToSlotStorage[0]=8; w.soft_updates=0;
    w.flatButtons1_[3].r=(RECT){80,45,100,65};
    w.on_button_passover(0x37B,0);
    if (events[0].style!=3 || events[1].style!=3
        || !Sprite::arrow_datalink_receiver || Sprite::arrow_x!=70
        || Sprite::arrow_y!=59) return 14;
    if (DatalinkMain.soft_updates!=1 || w.soft_updates!=0
        || events[event_count-1].kind!=4
        || events[event_count-1].datalink_receiver!=1) {
        fprintf(stderr,"dl=%d report=%d last=%d/%d count=%d\n",
            DatalinkMain.soft_updates,w.soft_updates,events[event_count-1].kind,
            events[event_count-1].datalink_receiver,event_count);
        return 15;
    }
    for (int i=0;i<event_count;++i)
        if (events[i].kind!=4 && events[i].datalink_receiver!=1) return 16;

    reset(); callback_owner=&w; IfaceIdToSlotStorage[0]=8;
    w.flatButtons1_[4].r=(RECT){80,65,100,85};
    w.on_button_passover(0x37C,1);
    if (events[0].style!=5 || events[1].style!=5
        || !Sprite::arrow_datalink_receiver || Sprite::arrow_x!=70
        || Sprite::arrow_y!=78) return 17;
    if (DatalinkMain.soft_updates!=1 || w.soft_updates!=0
        || events[event_count-1].kind!=4
        || events[event_count-1].datalink_receiver!=1) return 18;
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_lab_anchor_and_passover_views(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    _, line, passed = source_methods()
    assert "reinterpret_cast<int *>(this)" not in line
    assert "RECT *work_rect = &report_work_rect_;" in line
    source = tmp_path / "reportwin_lab_anchor.cpp"
    source.write_text(harness(line, passed))
    executable = tmp_path / "reportwin_lab_anchor"
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_lab_anchor_host_control_detects_neighbor_anchor_mutant(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    _, line, passed = source_methods()
    mutant = line.replace(
        "const int line_id = reportwin_signed_coordinate(lab_center_x_);",
        "const int line_id = reportwin_signed_coordinate(lab_child_x_);",
        1,
    )
    assert mutant != line
    source = tmp_path / "reportwin_lab_anchor_mutant.cpp"
    source.write_text(harness(mutant, passed))
    executable = tmp_path / "reportwin_lab_anchor_mutant"
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode != 0
