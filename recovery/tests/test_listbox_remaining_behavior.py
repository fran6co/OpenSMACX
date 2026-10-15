"""Actual C++ body controls with host service doubles, not target ABI/game proof."""
import shutil
import pytest

def test_actual_listbox_remaining_handlers(tmp_path):
    import shutil
    if not shutil.which("g++"):
        pytest.skip("host C++ compiler unavailable")
    from pathlib import Path
    import shutil
    import subprocess
    root = Path(__file__).resolve().parents[2]
    packet = tmp_path
    out = packet / 'actual_body_control.cpp'
    source = (root / 'src/listbox.cpp').read_text()
    helper_start = source.index('// Private pure branches')
    helper_source = source[helper_start:source.index('\n/*', helper_start)]
    
    def extract(signature):
        start = source.index(signature)
        opening = source.index('{', start)
        depth = 0
        for i in range(opening, len(source)):
            depth += (source[i] == '{') - (source[i] == '}')
            if depth == 0:
                return source[start:i + 1]
        raise AssertionError(signature)
    hit_helpers = helper_source
    hit_helpers += '\n' + extract('struct ListBoxHitSpanContext') + ';'
    hit_helpers += '\n' + extract('static int listbox_hit_span_at')
    hit_helpers += '\n' + extract('static int listbox_hit_position(ListBox *')
    dispatch_start = source.index('static int listbox_key_dispatch_arm')
    dispatch_end = source.index('// Instruction/caller provenance retained in', dispatch_start)
    dispatch = source[dispatch_start:dispatch_end] + '\n'
    method_signatures = ['void ListBox::on_left_down', 'void ListBox::on_mouse_move', 'int ListBox::on_key_down', 'void ListBox::UNK1', 'void ListBox::on_redraw']
    methods = '\n'.join((extract(s) for s in method_signatures))
    prefix = '#include <cassert>\n#include <cctype>\n#include <cstdint>\n#include <cstring>\n#include <vector>\ntypedef unsigned int UINT;\ntypedef unsigned char BYTE;\ntypedef unsigned int uint32_t;\n#define VK_CONTROL 0x11\n#define VK_SHIFT 0x10\nstruct RECT { int left, top, right, bottom; };\nstatic int key_state_control, key_state_shift;\nstatic UINT mapped_character;\nstatic int GetKeyState(int key) { return key == 0x11 ? key_state_control : key_state_shift; }\nstatic UINT MapVirtualKeyA(UINT, UINT mode) { assert(mode == 2); return mapped_character; }\nclass Win;\nclass GraphicWin;\nstatic Win *WinCallbackWindow;\nstatic int ListBoxDrawOffset;\nstruct StringStructPayload { unsigned int field_C_; unsigned int field_8_; char *text; };\nclass StringList {\npublic:\n    int entry_count_; void *head_; std::vector<StringStructPayload> values; int cursor;\n    StringList() : entry_count_(0), head_(0), cursor(-1) {}\n    bool seek_pos(int p) { cursor = p; return p >= 0 && p < entry_count_; }\n    StringStructPayload *current_entry() { return cursor >= 0 && cursor < entry_count_ ? &values[cursor] : 0; }\n};\nclass Win {\npublic:\n    Win *parent_; int event_log[128]; int event_count;\n    Win() : parent_(0), event_count(0) {}\n    Win *parent_window() { return parent_; }\n    virtual int on_dialog_back_draw(GraphicWin *) { event_log[event_count++] = 11; return 0; }\n    virtual void on_button_clicked(int) { event_log[event_count++] = 12; }\n};\nclass Buffer {\npublic:\n    int copy_source_x, copy_source_y, copy_dest_x, copy_dest_y, copy_width, copy_height;\n    Buffer() : copy_source_x(0), copy_source_y(0), copy_dest_x(0), copy_dest_y(0), copy_width(0), copy_height(0) {}\n    void restore_clip() {}\n    int pixel_width() const { return 640; }\n    int pixel_height() const { return 480; }\n    int copy(Buffer *, int sx, int sy, int dx, int dy, int w, int h) {\n        copy_source_x=sx; copy_source_y=sy; copy_dest_x=dx; copy_dest_y=dy; copy_width=w; copy_height=h; return 0;\n    }\n    int box(RECT *, int, int) { return 0; }\n};\nclass GraphicWin : public Win, public Buffer {\npublic:\n    RECT work; int fills, updates, draws[32], draw_count;\n    GraphicWin() : fills(0), updates(0), draw_count(0) { work.left=2; work.top=3; work.right=102; work.bottom=83; }\n    RECT *buffer_working_rect() { return &work; }\n    void fill(int) { ++fills; }\n    void soft_update() { ++updates; }\n};\nclass Dialog : public GraphicWin {\npublic:\n    StringList item_list_; int selected_position_; uint32_t field_F0_;\n    uint32_t field_54_, field_40_, field_38_, field_58_, field_64_, field_44_, field_68_, field_20_, field_6C_, field_24_, field_28_;\n    Dialog() : selected_position_(-1), field_F0_(0xFFFFFFFFu), field_54_(1), field_40_(0), field_38_(0), field_58_(1), field_64_(10), field_44_(0), field_68_(10), field_20_(0), field_6C_(0), field_24_(0), field_28_(0) {}\n};\ntypedef int (*FillCallback)(GraphicWin *, int, int, void *);\nclass ListBox : public Dialog {\npublic:\n    unsigned int field_4_, field_18_, iFlags_; FillCallback fill_callback_;\n    ListBox() : field_4_(1), field_18_(0), iFlags_(0), fill_callback_(0) {}\n    void set_selected_pos(int p, int) { selected_position_ = p; event_log[event_count++] = 100+p; }\n    void draw_item(int p) { draws[draw_count++] = p; event_log[event_count++] = 200+p; }\n    void on_left_down(int, int);\n    void on_mouse_move(int, int);\n    void on_key_down(int);\n    void UNK1(int);\n    void on_redraw();\n};\n'
    controls = '\nstatic int fill_noop(GraphicWin *, int, int, void *) { return 0; }\nint main() {\n    ListBox box; Win parent; box.parent_ = &parent;\n    box.item_list_.entry_count_ = 5; box.item_list_.head_ = &box.item_list_;\n    box.item_list_.values.resize(5);\n    int spans[] = {1, 2, 1, 1, 1};\n    for (int i=0;i<5;++i) { box.item_list_.values[i].field_C_=spans[i]; box.item_list_.values[i].field_8_=0; box.item_list_.values[i].text=(char*)"x"; }\n    // Literal on_left_down body: mode 1 derives hit from y alone, publishes\n    // parent callback receiver, and applies the no-Shift backward selection.\n    box.field_18_=0; box.field_4_=1; box.field_54_=1; box.selected_position_=4;\n    box.item_list_.values[4].field_8_=1; box.item_list_.values[1].field_8_=1;\n    box.on_left_down(999, 0);\n    assert(WinCallbackWindow == &parent && box.selected_position_ == 0);\n    assert(box.item_list_.values[0].field_8_ == 1 && box.item_list_.values[1].field_8_ == 0 && box.item_list_.values[4].field_8_ == 0);\n    // Ctrl keeps unrelated rows and toggles the clicked row when prior\n    // selection is elsewhere. The handler calls set_selected_pos last.\n    box.selected_position_=4; box.item_list_.values[1].field_8_=1; box.item_list_.values[2].field_8_=1;\n    key_state_control=0x8000; box.event_count=0; box.on_left_down(0, 20);\n    assert(box.item_list_.values[1].field_8_ == 0 && box.item_list_.values[2].field_8_ == 1);\n    assert(box.event_log[box.event_count-1] == 101);\n\n    // The original forward-Shift path starts its walk at previous-1. With\n    // no previous selection that is -2, which wraps through the list tail\n    // before advancing to row 0. It selects the tail and 0..clicked.\n    box.selected_position_=-1; box.field_4_=1; box.field_54_=1;\n    for (int i=0;i<5;++i) box.item_list_.values[i].field_8_=0;\n    key_state_control=0; key_state_shift=0x8000;\n    box.on_left_down(0, 0);\n    assert(box.item_list_.values[4].field_8_ == 1);\n    assert(box.item_list_.values[0].field_8_ == 1);\n    assert(box.item_list_.values[1].field_8_ == 0);\n    box.selected_position_=-1;\n    for (int i=0;i<5;++i) box.item_list_.values[i].field_8_=0;\n    box.on_left_down(0, 30);\n    assert(box.item_list_.values[4].field_8_ == 1);\n    assert(box.item_list_.values[0].field_8_ == 1 && box.item_list_.values[1].field_8_ == 1 && box.item_list_.values[2].field_8_ == 1);\n    assert(box.item_list_.values[3].field_8_ == 0);\n    key_state_shift=0;\n\n    // Literal hover body: old row redraw precedes new row, then soft update.\n    key_state_control=0; box.draw_count=0; box.field_18_=0; box.field_54_=2; box.field_58_=3;\n    box.field_64_=10; box.field_68_=10; box.selected_position_=0; box.field_F0_=2;\n    box.on_mouse_move(10, 10);\n    assert(box.draw_count == 2 && box.draws[0] == 2 && box.draws[1] == 4 && box.updates == 1);\n    box.draw_count=0; box.field_F0_=4; box.selected_position_=4; box.on_mouse_move(10, 10);\n    assert(box.draw_count == 1 && box.draws[0] == 4 && box.field_F0_ == 0xFFFFFFFFu);\n\n    // Literal key handler uses MapVirtualKey mode 2 and selects the matching\n    // next item; arrow branch navigates by row count.\n    box.selected_position_=0; box.field_54_=2; box.field_58_=2;\n    box.on_key_down(0x27); assert(box.selected_position_ == 2);\n    box.item_list_.values[3].text=(char*)"A"; box.selected_position_=0;\n    mapped_character=\'A\'; box.on_key_down(\'A\'); assert(box.selected_position_ == 3);\n\n    // Literal UNK1 body uses the evidenced blit geometry and typed APIs.\n    box.field_64_=100; box.field_40_=5; box.field_54_=3; box.field_58_=4; box.field_68_=10;\n    box.UNK1(1); assert(box.copy_source_x == 105 && box.copy_dest_x == 5 && box.copy_width == 190 && box.copy_height == 40);\n\n    // Literal redraw body: callbacks short-circuit fill; absent list callback\n    // and disabled parent paint falls through to fill and draws visible rows.\n    box.fill_callback_=fill_noop; box.iFlags_=0; box.field_54_=2; box.field_58_=2;\n    box.field_18_=1; box.selected_position_=0; box.item_list_.entry_count_=4;\n    box.draw_count=0; box.fills=0; box.on_redraw();\n    assert(box.fills == 1 && box.draw_count == 3 && box.draws[0] == 1 && box.draws[2] == 3);\n    box.item_list_.head_=0; box.draw_count=0; box.on_redraw(); assert(box.draw_count == 0);\n}\n'
    prefix = prefix.replace('virtual void on_button_clicked(int) { event_log[event_count++] = 12; }',
                            'virtual void on_button_clicked(int) { event_log[event_count++] = 12; }\n    virtual int on_key_down(int) { return 0; }')
    prefix = prefix.replace('    void on_key_down(int);', '    int on_key_down(int) override;')
    controls = controls.replace('box.on_key_down(0x27); assert(box.selected_position_ == 2);',
                                'assert(box.on_key_down(0x27) == 1 && box.selected_position_ == 2);')
    controls = controls.replace("mapped_character='A'; box.on_key_down('A'); assert(box.selected_position_ == 3);",
                                "mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 3);")
    controls = controls.replace("mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 3);",
                                "mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 3); mapped_character='Z'; assert(box.on_key_down('Z') == 0);")
    out.write_text(prefix + hit_helpers + dispatch + methods + controls)
    for name in ('g++', 'clang++'):
        compiler = shutil.which(name)
        if compiler:
            exe = packet / ('actual-body-' + name)
            subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror', str(out), '-o', str(exe)], check=True, timeout=30)
            subprocess.run([str(exe)], check=True, timeout=30)
            print(name + ': literal extracted ListBox handler controls PASS', flush=True)


def test_actual_dialogs_propagates_listbox_key_result(tmp_path):
    from pathlib import Path
    import subprocess

    root = Path(__file__).resolve().parents[2]
    source = (root / 'src/dialogs.cpp').read_text()
    signature = 'int Dialogs::on_key_down(int key)'
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError('unterminated Dialogs::on_key_down')

    compiler = shutil.which('g++') or shutil.which('clang++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    harness = r'''
#include <cassert>
struct RadioButton { int result; int on_key_down(int key) { assert(key == 7); return result; } };
struct ListBox { int result; int on_key_down(int key) { assert(key == 7); return result; } };
struct CheckBox { int result; int on_key_down(int key) { assert(key == 7); return result; } };
struct SpriteBox { int result; int on_key_down(int key) { assert(key == 7); return result; } };
struct Dialogs : RadioButton, ListBox, CheckBox, SpriteBox {
    int kind_;
    int on_key_down(int key);
};
BODY
int main() {
    Dialogs dialog;
    dialog.kind_ = 2; dialog.ListBox::result = 1;
    assert(dialog.on_key_down(7) == 1);
    dialog.ListBox::result = 0;
    assert(dialog.on_key_down(7) == 0);
    dialog.kind_ = 16; dialog.RadioButton::result = 3;
    assert(dialog.on_key_down(7) == 3);
    dialog.kind_ = 1; dialog.CheckBox::result = -1;
    assert(dialog.on_key_down(7) == -1);
    dialog.kind_ = 8; dialog.SpriteBox::result = 5;
    assert(dialog.on_key_down(7) == 5);
    dialog.kind_ = 4;
    assert(dialog.on_key_down(7) == 0);
    dialog.kind_ = 99;
    assert(dialog.on_key_down(7) == 0);
}
'''.replace('BODY', body)
    cpp = tmp_path / 'dialogs-key-result.cpp'
    exe = tmp_path / 'dialogs-key-result'
    cpp.write_text(harness)
    subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
                    str(cpp), '-o', str(exe)], check=True, capture_output=True,
                   text=True, timeout=30)
    subprocess.run([str(exe)], check=True, capture_output=True, timeout=30)
