"""Actual C++ body controls with host service doubles, not target ABI/game proof."""
import os
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
    source_path = Path(os.environ.get(
        'OPENSMACX_LISTBOX_SOURCE', root / 'src/listbox.cpp'))
    source = source_path.read_text()
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
    # Compile the actual private helper block as a unit. Earlier fixtures
    # manually re-extracted a callback-based helper signature that the
    # circular StringList implementation no longer uses.
    hit_helpers = helper_source
    hit_helpers += "\n" + extract("static bool listbox_hit_position(ListBox *")
    dispatch_start = source.index('static int listbox_key_dispatch_arm')
    dispatch_end = source.index('// Instruction/caller provenance retained in', dispatch_start)
    dispatch = source[dispatch_start:dispatch_end] + '\n'
    method_signatures = ['void ListBox::on_left_down', 'void ListBox::on_mouse_move', 'int ListBox::on_key_down', 'void ListBox::UNK1', 'void ListBox::on_redraw']
    methods = '\n'.join((extract(s) for s in method_signatures))
    click_method = extract('void ListBox::on_left_down')
    assert click_method.count('GetAsyncKeyState') == 2
    wrong_import_click_method = click_method.replace('GetAsyncKeyState', 'GetKeyState')
    assert wrong_import_click_method != click_method
    key_method = extract('int ListBox::on_key_down')
    cursor_advance = 'items.current_ = items.current_->next;'
    cursor_advance_at = key_method.rfind(cursor_advance)
    assert cursor_advance_at >= 0
    old_index_mutant = (key_method[:cursor_advance_at]
                        + 'items.seek_pos((position + offset + 1) % search_count);'
                        + key_method[cursor_advance_at + len(cursor_advance):])
    low_byte_mutant = key_method.replace('if (first == 0) {', 'if (character == 0) {', 1)
    stale_selection_mutant = key_method.replace('    position = Dialog::selected_position_;\n', '', 1)
    zero_count_mutant = key_method.replace(
        '    if (position <= search_count - 1) {',
        '    if (search_count <= 0) return 0;\n    if (position <= search_count - 1) {', 1)
    assert all(mutant != key_method for mutant in
               (old_index_mutant, low_byte_mutant, stale_selection_mutant, zero_count_mutant))
    mutant_methods = [methods.replace(key_method, mutant, 1) for mutant in
                      (old_index_mutant, low_byte_mutant,
                       stale_selection_mutant, zero_count_mutant)]
    wrong_import_methods = methods.replace(click_method, wrong_import_click_method, 1)

    def replace_fixture_once(fixture, old, new):
        assert fixture.count(old) == 1, old
        return fixture.replace(old, new, 1)

    prefix = '#include <cassert>\n#include <cctype>\n#include <cstdint>\n#include <cstring>\n#include <stdlib.h>\n#include <vector>\ntypedef unsigned int UINT;\ntypedef unsigned char BYTE;\ntypedef unsigned int uint32_t;\n#define VK_CONTROL 0x11\n#define VK_SHIFT 0x10\nstruct RECT { int left, top, right, bottom; };\nstatic int key_state_control, key_state_shift;\nstatic int async_key_state_control, async_key_state_shift;\nstatic int get_key_state_calls, async_key_state_calls, async_key_args[8];\nstatic UINT mapped_character;\nshort GetKeyState(int key) { ++get_key_state_calls; return key == 0x11 ? key_state_control : key_state_shift; }\nshort GetAsyncKeyState(int key) { assert(async_key_state_calls < 8); async_key_args[async_key_state_calls++] = key; return key == 0x11 ? async_key_state_control : async_key_state_shift; }\nstatic UINT MapVirtualKeyA(UINT, UINT mode) { assert(mode == 2); return mapped_character; }\nclass Win;\nclass GraphicWin;\nstatic Win *WinCallbackWindow;\nstatic int ListBoxDrawOffset;\nstruct StringStructPayload { unsigned int field_C_; unsigned int field_8_; char *text; };\nclass StringList {\npublic:\n    int entry_count_; void *head_; std::vector<StringStructPayload> values; int cursor;\n    StringList() : entry_count_(0), head_(0), cursor(-1) {}\n    bool seek_pos(int p) { cursor = p; return p >= 0 && p < entry_count_; }\n    StringStructPayload *current_entry() { return cursor >= 0 && cursor < entry_count_ ? &values[cursor] : 0; }\n};\nclass Win {\npublic:\n    Win *parent_; int event_log[128]; int event_count;\n    Win() : parent_(0), event_count(0) {}\n    Win *parent_window() { return parent_; }\n    virtual int on_dialog_back_draw(GraphicWin *) { event_log[event_count++] = 11; return 0; }\n    virtual void on_button_clicked(int) { event_log[event_count++] = 12; }\n};\nclass Buffer {\npublic:\n    int copy_source_x, copy_source_y, copy_dest_x, copy_dest_y, copy_width, copy_height;\n    Buffer() : copy_source_x(0), copy_source_y(0), copy_dest_x(0), copy_dest_y(0), copy_width(0), copy_height(0) {}\n    void restore_clip() {}\n    int pixel_width() const { return 640; }\n    int pixel_height() const { return 480; }\n    int copy(Buffer *, int sx, int sy, int dx, int dy, int w, int h) {\n        copy_source_x=sx; copy_source_y=sy; copy_dest_x=dx; copy_dest_y=dy; copy_width=w; copy_height=h; return 0;\n    }\n    int box(RECT *, int, int) { return 0; }\n};\nclass GraphicWin : public Win, public Buffer {\npublic:\n    RECT work; int fills, updates, draws[32], draw_count;\n    GraphicWin() : fills(0), updates(0), draw_count(0) { work.left=2; work.top=3; work.right=102; work.bottom=83; }\n    RECT *buffer_working_rect() { return &work; }\n    void fill(int) { ++fills; }\n    void soft_update() { ++updates; }\n};\nclass Dialog : public GraphicWin {\npublic:\n    StringList item_list_; int selected_position_; uint32_t field_F0_;\n    int column_count_, field_40_, field_38_, row_count_, field_64_, field_44_, field_68_, field_20_, field_6C_, field_24_, field_28_;\n    Dialog() : selected_position_(-1), field_F0_(0xFFFFFFFFu), column_count_(1), field_40_(0), field_38_(0), row_count_(1), field_64_(10), field_44_(0), field_68_(10), field_20_(0), field_6C_(0), field_24_(0), field_28_(0) {}\n};\ntypedef int (*FillCallback)(GraphicWin *, int, int, void *);\nclass ListBox : public Dialog {\npublic:\n    unsigned int field_4_, field_18_, iFlags_; FillCallback fill_callback_;\n    ListBox() : field_4_(1), field_18_(0), iFlags_(0), fill_callback_(0) {}\n    void set_selected_pos(int p, int) { selected_position_ = p; event_log[event_count++] = 100+p; }\n    void draw_item(int p) { draws[draw_count++] = p; event_log[event_count++] = 200+p; }\n    void on_left_down(int, int);\n    void on_mouse_move(int, int);\n    void on_key_down(int);\n    void UNK1(int);\n    void on_redraw();\n};\n'
    prefix = prefix.replace("field_24_", "window_x_").replace("field_28_", "window_y_")
    prefix = replace_fixture_once(prefix,
        '    void draw_item(int p) { draws[draw_count++] = p; event_log[event_count++] = 200+p; }',
        '    void (*draw_mutator)(ListBox *, int); int mutate_once;\n'
        '    void draw_item(int p) { draws[draw_count++] = p; event_log[event_count++] = 200+p; if (draw_mutator != 0) draw_mutator(this, p); }')
    prefix = replace_fixture_once(prefix,
        'ListBox() : field_4_(1), field_18_(0), iFlags_(0), fill_callback_(0) {}',
        'ListBox() : field_4_(1), field_18_(0), iFlags_(0), fill_callback_(0), draw_mutator(0), mutate_once(0) {}')
    prefix = replace_fixture_once(prefix,'static Win *WinCallbackWindow;',
                            'static Win *WinCallbackWindow;\nstatic Win *fill_reparent_target;\nstatic int *box_mutation_target;\nstatic int box_mutation_value;')
    prefix = replace_fixture_once(prefix,'Win *parent_window() { return parent_; }',
                            'Win *parent_window() { return parent_; }\n    void set_parent(Win *parent) { parent_ = parent; }')
    prefix = replace_fixture_once(prefix,
        'int copy_source_x, copy_source_y, copy_dest_x, copy_dest_y, copy_width, copy_height;',
        'int copy_source_x, copy_source_y, copy_dest_x, copy_dest_y, copy_width, copy_height, copy_calls, restore_calls, box_calls;')
    prefix = replace_fixture_once(prefix,
        'Buffer() : copy_source_x(0), copy_source_y(0), copy_dest_x(0), copy_dest_y(0), copy_width(0), copy_height(0) {}',
        'Buffer() : copy_source_x(0), copy_source_y(0), copy_dest_x(0), copy_dest_y(0), copy_width(0), copy_height(0), copy_calls(0), restore_calls(0), box_calls(0) {}')
    prefix = replace_fixture_once(prefix,'void restore_clip() {}', 'void restore_clip() { ++restore_calls; }')
    prefix = replace_fixture_once(prefix,'copy_source_x=sx; copy_source_y=sy; copy_dest_x=dx;',
                            '++copy_calls; copy_source_x=sx; copy_source_y=sy; copy_dest_x=dx;')
    prefix = replace_fixture_once(prefix,'int box(RECT *, int, int) { return 0; }',
                            'int box(RECT *, int, int) { ++box_calls; if (box_mutation_target != 0) { *box_mutation_target = box_mutation_value; box_mutation_target = 0; } return 0; }')
    old_list = 'struct StringStructPayload { unsigned int field_C_; unsigned int field_8_; char *text; };\nclass StringList {\npublic:\n    int entry_count_; void *head_; std::vector<StringStructPayload> values; int cursor;\n    StringList() : entry_count_(0), head_(0), cursor(-1) {}\n    bool seek_pos(int p) { cursor = p; return p >= 0 && p < entry_count_; }\n    StringStructPayload *current_entry() { return cursor >= 0 && cursor < entry_count_ ? &values[cursor] : 0; }\n};'
    linked_list = 'struct StringStructPayload { unsigned int field_C_; unsigned int field_8_; char *text; };\nstruct StringStructEntry { StringStructPayload *payload; StringStructEntry *next, *previous; StringStructPayload *string_payload() { return payload; } };\nclass StringList {\npublic:\n    int entry_count_; StringStructEntry *head_, *current_; int current_position_; std::vector<StringStructPayload> values; std::vector<StringStructEntry> entries; int cursor;\n    StringList() : entry_count_(0), head_(0), current_(0), current_position_(0), cursor(-1) {}\n    void link_items() { entries.resize(values.size()); entry_count_ = static_cast<int>(values.size()); for (int i=0;i<entry_count_;++i) { entries[i].payload=&values[i]; entries[i].next=&entries[(i+1)%entry_count_]; entries[i].previous=&entries[(i+entry_count_-1)%entry_count_]; } head_ = entry_count_ ? &entries[0] : 0; current_=head_; current_position_=0; cursor=entry_count_ ? 0 : -1; }\n    bool seek_pos(int p) { if (p > entry_count_-1) return false; current_=head_; if (p < 0) { if (abs(p) > entry_count_) return false; int i=abs(p); while (i-- > 0) current_=current_->previous; current_position_=p+entry_count_; } else { int i=p; while (i-- > 0) current_=current_->next; current_position_=p; } cursor=current_position_; return true; }\n    StringStructPayload *current_entry() { return current_ ? current_->payload : 0; }\n};'
    assert old_list in prefix
    prefix = prefix.replace(old_list, linked_list)
    prefix = prefix.replace(
        'static UINT MapVirtualKeyA(UINT, UINT mode) { assert(mode == 2); return mapped_character; }',
        'static int map_calls; static int *mapped_selection; static int mapped_selected_value;\nstatic UINT MapVirtualKeyA(UINT, UINT mode) { assert(mode == 2); ++map_calls; if (mapped_selection) *mapped_selection = mapped_selected_value; return mapped_character; }')
    controls = '\nstatic int fill_noop(GraphicWin *, int, int, void *) { return 0; }\nint main() {\n    ListBox box; Win parent; box.parent_ = &parent;\n    box.item_list_.entry_count_ = 5; box.item_list_.head_ = &box.item_list_;\n    box.item_list_.values.resize(5);\n    int spans[] = {1, 2, 1, 1, 1};\n    for (int i=0;i<5;++i) { box.item_list_.values[i].field_C_=spans[i]; box.item_list_.values[i].field_8_=0; box.item_list_.values[i].text=(char*)"x"; }\n    // Literal on_left_down body: mode 1 derives hit from y alone, publishes\n    // parent callback receiver, and applies the no-Shift backward selection.\n    box.field_18_=0; box.field_4_=1; box.column_count_=1; box.selected_position_=4;\n    box.item_list_.values[4].field_8_=1; box.item_list_.values[1].field_8_=1;\n    // Deliberately disagree the synchronous and asynchronous test doubles.\n    key_state_control=0; key_state_shift=0x8000;\n    async_key_state_control=0; async_key_state_shift=0;\n    get_key_state_calls=0; async_key_state_calls=0;\n    box.on_left_down(999, 0);\n    assert(get_key_state_calls == 0 && async_key_state_calls == 2);\n    assert(async_key_args[0] == VK_CONTROL && async_key_args[1] == VK_SHIFT);\n    assert(WinCallbackWindow == &parent && box.selected_position_ == 0);\n    assert(box.item_list_.values[0].field_8_ == 1 && box.item_list_.values[1].field_8_ == 0 && box.item_list_.values[4].field_8_ == 0);\n    // Ctrl keeps unrelated rows and toggles the clicked row when prior\n    // selection is elsewhere. The handler calls set_selected_pos last.\n    box.selected_position_=4; box.item_list_.values[1].field_8_=1; box.item_list_.values[2].field_8_=1;\n    key_state_control=0; key_state_shift=0x8000;\n    async_key_state_control=0x8000; async_key_state_shift=0;\n    get_key_state_calls=0; async_key_state_calls=0;\n    box.event_count=0; box.on_left_down(0, 20);\n    assert(get_key_state_calls == 0 && async_key_state_calls == 2);\n    assert(async_key_args[0] == VK_CONTROL && async_key_args[1] == VK_SHIFT);\n    assert(box.item_list_.values[1].field_8_ == 0 && box.item_list_.values[2].field_8_ == 1);\n    assert(box.event_log[box.event_count-1] == 101);\n\n    // The original forward-Shift path starts its walk at previous-1. With\n    // no previous selection that is -2, which wraps through the list tail\n    // before advancing to row 0. It selects the tail and 0..clicked.\n    box.selected_position_=-1; box.field_4_=1; box.column_count_=1;\n    for (int i=0;i<5;++i) box.item_list_.values[i].field_8_=0;\n    key_state_control=0; key_state_shift=0;\n    async_key_state_control=0; async_key_state_shift=0x8000;\n    get_key_state_calls=0; async_key_state_calls=0;\n    box.on_left_down(0, 0);\n    assert(get_key_state_calls == 0 && async_key_state_calls == 2);\n    assert(async_key_args[0] == VK_CONTROL && async_key_args[1] == VK_SHIFT);\n    assert(box.item_list_.values[4].field_8_ == 1);\n    assert(box.item_list_.values[0].field_8_ == 1);\n    assert(box.item_list_.values[1].field_8_ == 0);\n    box.selected_position_=-1;\n    for (int i=0;i<5;++i) box.item_list_.values[i].field_8_=0;\n    box.on_left_down(0, 30);\n    assert(box.item_list_.values[4].field_8_ == 1);\n    assert(box.item_list_.values[0].field_8_ == 1 && box.item_list_.values[1].field_8_ == 1 && box.item_list_.values[2].field_8_ == 1);\n    assert(box.item_list_.values[3].field_8_ == 0);\n    key_state_shift=0;\n\n    // Literal hover body: old row redraw precedes new row, then soft update.\n    key_state_control=0; box.draw_count=0; box.field_18_=0; box.column_count_=2; box.row_count_=3;\n    box.field_64_=10; box.field_68_=10; box.selected_position_=0; box.field_F0_=2;\n    box.on_mouse_move(10, 10);\n    assert(box.draw_count == 2 && box.draws[0] == 2 && box.draws[1] == 4 && box.updates == 1);\n    box.draw_count=0; box.field_F0_=4; box.selected_position_=4; box.on_mouse_move(10, 10);\n    assert(box.draw_count == 1 && box.draws[0] == 4 && box.field_F0_ == 0xFFFFFFFFu);\n\n    // Literal key handler uses MapVirtualKey mode 2 and selects the matching\n    // next item; arrow branch navigates by row count.\n    box.selected_position_=0; box.column_count_=2; box.row_count_=2;\n    box.on_key_down(0x27); assert(box.selected_position_ == 2);\n    box.item_list_.values[3].text=(char*)"A"; box.selected_position_=0;\n    mapped_character=\'A\'; box.on_key_down(\'A\'); assert(box.selected_position_ == 3);\n\n    // Literal UNK1 body uses the evidenced blit geometry and typed APIs.\n    box.field_64_=100; box.field_40_=5; box.column_count_=3; box.row_count_=4; box.field_68_=10;\n    box.UNK1(1); assert(box.copy_source_x == 105 && box.copy_dest_x == 5 && box.copy_width == 190 && box.copy_height == 40);\n\n    // Literal redraw body: callbacks short-circuit fill; absent list callback\n    // and disabled parent paint falls through to fill and draws visible rows.\n    box.fill_callback_=fill_noop; box.iFlags_=0; box.column_count_=2; box.row_count_=2;\n    box.field_18_=1; box.selected_position_=0; box.item_list_.entry_count_=4;\n    box.draw_count=0; box.fills=0; box.on_redraw();\n    assert(box.fills == 1 && box.draw_count == 3 && box.draws[0] == 1 && box.draws[2] == 3);\n    box.item_list_.head_=0; box.draw_count=0; box.on_redraw(); assert(box.draw_count == 0);\n}\n'
    controls = controls.replace('box.item_list_.entry_count_ = 5; box.item_list_.head_ = &box.item_list_;',
                                'box.item_list_.entry_count_ = 5;')
    controls = controls.replace('box.item_list_.values.resize(5);',
                                'box.item_list_.values.resize(5); box.item_list_.link_items(); box.item_list_.head_ = &box.item_list_.entries[0];')
    if 'struct ListBoxHitSpanContext' in helper_source:
        hit_controls = '''    // The pinned grid path rejects only candidates at/above count. A
    // signed negative candidate remains a hit and leaves its exact value.
    int signed_hit = 99;
    assert(listbox_grid_hit_position(-11, 0, 0, 5, 2, 10, 10,
                                    &signed_hit));
    assert(signed_hit == -2);
    signed_hit = 77;
    assert(!listbox_grid_hit_position(100, 0, 0, 5, 2, 10, 10,
                                     &signed_hit));
    assert(signed_hit == 77);

    // Start at the final variable-height row; a y offset equal to its span
    // advances through the circular link to row 0. The helper is bounded by
    // entry_count and leaves both cursor pointer and ordinal at the hit.
    ListBoxHitSpanContext hit_context = { &box.item_list_ };
    int circular_hit = -99;
    assert(listbox_y_span_hit(10, 4, 5, 10, &hit_context, &circular_hit));
    assert(circular_hit == 0);
    assert(box.item_list_.current_ == &box.item_list_.entries[0]);
    assert(box.item_list_.current_position_ == 0);

'''
        anchor = '    // Literal on_left_down body: mode 1 derives hit from y alone, publishes\n'
        assert controls.count(anchor) == 1
        controls = controls.replace(anchor, hit_controls + anchor, 1)
    controls = replace_fixture_once(controls,
        'static int fill_noop(GraphicWin *, int, int, void *) { return 0; }\nint main() {',
        'static int fill_noop(GraphicWin *, int, int, void *) { return 0; }\n'
        'static int fill_reparent(GraphicWin *window, int, int, void *) { window->set_parent(fill_reparent_target); return 0; }\n'
        'int main() {')
    controls = replace_fixture_once(controls,
        '    box.item_list_.head_=0; box.draw_count=0; box.on_redraw(); assert(box.draw_count == 0);',
        '''    // The entry guard is the Win parent. A nonempty list with no parent
    // returns before clipping or painting.
    box.item_list_.head_=&box.item_list_.entries[0]; box.item_list_.entry_count_=5;
    box.parent_=0; int clips_before_null_parent=box.restore_calls;
    int fills_before_null_parent=box.fills; Win *previous_callback_window=WinCallbackWindow;
    box.draw_count=0; box.on_redraw();
    assert(box.restore_calls == clips_before_null_parent && box.fills == fills_before_null_parent
           && box.draw_count == 0 && WinCallbackWindow == previous_callback_window);

    // An empty list with a parent still paints the background and border,
    // then returns before changing the previous draw offset.
    box.parent_=&parent; box.item_list_.head_=0; box.item_list_.entry_count_=0;
    box.column_count_=2; box.row_count_=2; box.field_18_=0;
    box.field_6C_=2; ListBoxDrawOffset=42;
    int clips_before_empty_list=box.restore_calls; int fills_before_empty_list=box.fills;
    int boxes_before_empty_list=box.box_calls;
    box.draw_count=0; box.on_redraw();
    assert(box.restore_calls == clips_before_empty_list+1 && box.fills == fills_before_empty_list+1);
    assert(box.box_calls == boxes_before_empty_list+2 && box.draw_count == 0);
    assert(ListBoxDrawOffset == 42);

    // The pin reloads field_6C_ after each Buffer::box call.
    box.item_list_.entry_count_=1; box.item_list_.head_=&box.item_list_.entries[0];
    box.column_count_=1; box.row_count_=1; box.field_18_=0; box.field_6C_=3;
    box.box_calls=0; box_mutation_target=&box.field_6C_;
    box_mutation_value=1; box.on_redraw();
    assert(box.box_calls == 1 && box.field_6C_ == 1 && box_mutation_target == 0);

    // A valid negative seek wraps to the final node. An above-end seek fails
    // before changing the cursor and does not abort the grid loop.
    box.item_list_.head_=&box.item_list_.entries[0]; box.item_list_.entry_count_=5;
    box.column_count_=1; box.row_count_=1; box.field_18_=0;
    box.item_list_.current_=&box.item_list_.entries[1]; box.item_list_.current_position_=1;
    box.draw_count=0; box.on_redraw();
    assert(box.item_list_.current_ == &box.item_list_.entries[4]
           && box.item_list_.current_position_ == 4);
    box.field_18_=99; box.item_list_.current_=&box.item_list_.entries[2];
    box.item_list_.current_position_=2; box.draw_count=0; box.on_redraw();
    assert(box.draw_count == 1 && box.draws[0] == 99
           && box.item_list_.current_ == &box.item_list_.entries[2]
           && box.item_list_.current_position_ == 2);
    box.field_18_=static_cast<unsigned int>(-99);
    box.item_list_.current_=&box.item_list_.entries[2];
    box.item_list_.current_position_=2; box.draw_count=0; box.on_redraw();
    assert(box.draw_count == 1 && box.draws[0] == -99
           && box.item_list_.current_ == &box.item_list_.entries[0]
           && box.item_list_.current_position_ == 2);

    // The initial parent snapshot is used for on_dialog_back_draw even when
    // fill_callback reparents the ListBox. The fallback copy reloads parent.
    GraphicWin original_parent, replacement_parent;
    box.parent_=&original_parent; box.item_list_.head_=&box.item_list_.entries[0];
    box.item_list_.entry_count_=1; box.column_count_=1; box.row_count_=1;
    box.field_18_=0; box.iFlags_=0x80000; box.fill_callback_=fill_reparent;
    fill_reparent_target=&replacement_parent;
    original_parent.event_count=0; original_parent.copy_calls=0;
    replacement_parent.copy_calls=0; box.draw_count=0; box.on_redraw();
    assert(original_parent.event_count == 1 && WinCallbackWindow == &original_parent);
    assert(box.parent_ == &replacement_parent && original_parent.copy_calls == 0
           && replacement_parent.copy_calls == 1);

    // If that callback reparents to null, the old parent still receives the
    // callback but the refreshed copy path falls back to fill.
    box.parent_=&original_parent; fill_reparent_target=0;
    original_parent.event_count=0; original_parent.copy_calls=0;
    replacement_parent.copy_calls=0; int fills_before_reparent_null=box.fills;
    box.draw_count=0; box.on_redraw();
    assert(original_parent.event_count == 1 && WinCallbackWindow == &original_parent);
    assert(box.parent_ == 0 && original_parent.copy_calls == 0
           && replacement_parent.copy_calls == 0 && box.fills == fills_before_reparent_null+1);

    box.parent_=&parent; box.fill_callback_=fill_noop; box.iFlags_=0;
    box.item_list_.head_=0; box.item_list_.entry_count_=0;
    box.draw_count=0; box.on_redraw(); assert(box.draw_count == 0);''')
    controls = replace_fixture_once(controls,
        '    // The entry guard is the Win parent. A nonempty list with no parent\n    // returns before clipping or painting.',
        '''    // Callback changes scroll. Pin's equality check breaks only the inner
    // loop, so the next outer pass continues beyond the remaining entries.
    box.item_list_.head_=&box.item_list_.entries[0];
    box.item_list_.entry_count_=5; box.column_count_=2; box.row_count_=2;
    box.field_18_=0; box.draw_count=0; box.mutate_once=1;
    box.draw_mutator = [](ListBox *item, int) {
        if (item->mutate_once) {
            item->mutate_once=0;
            item->field_18_=1;
            item->item_list_.entry_count_=3;
        }
    };
    box.on_redraw();
    assert(box.draw_count == 4 && box.draws[0] == 0 && box.draws[1] == 2
           && box.draws[2] == 3 && box.draws[3] == 4);
    assert(ListBoxDrawOffset == 4);

    // Callback changes both the global offset and scroll. The pin increments
    // the live global and forms the next item index from the refreshed scroll.
    box.item_list_.entry_count_=8; box.column_count_=2; box.row_count_=2;
    box.field_18_=0; box.draw_count=0;
    box.draw_mutator = [](ListBox *item, int) {
        if (item->draw_count == 1) { ListBoxDrawOffset=4; item->field_18_=1; }
    };
    box.on_redraw();
    assert(box.draw_count == 3 && box.draws[0] == 0 && box.draws[1] == 6
           && box.draws[2] == 7);
    assert(ListBoxDrawOffset == 7);

    // Bounds are live too: changing both in draw_item ends after one call.
    box.item_list_.entry_count_=6; box.column_count_=2; box.row_count_=2;
    box.field_18_=0; box.draw_count=0;
    box.draw_mutator = [](ListBox *item, int) {
        if (item->draw_count == 1) { item->row_count_=1; item->column_count_=1; }
    };
    box.on_redraw(); assert(box.draw_count == 1 && box.draws[0] == 0);

    box.draw_mutator=0;
    box.column_count_=0; box.row_count_=2; box.draw_count=0;
    box.on_redraw(); assert(box.draw_count == 0 && ListBoxDrawOffset == 0);
    box.column_count_=2; box.row_count_=-1;
    box.on_redraw(); assert(box.draw_count == 0 && ListBoxDrawOffset == 0);

    // The entry guard is the Win parent. A nonempty list with no parent
    // returns before clipping or painting.''')
    controls = controls.replace("box.item_list_.values[3].text=(char*)\"A\"; box.selected_position_=0;\n    mapped_character='A'; box.on_key_down('A'); assert(box.selected_position_ == 3);",
                                "box.item_list_.values[3].text=(char*)\"A\"; box.selected_position_=0;\n    mapped_character='A'; box.on_key_down('A'); assert(box.selected_position_ == 3);\n    // Above-last selected ordinal skips cursor setup: continue from this\n    // non-head node and wrap its deliberately independent ordinal.\n    box.item_list_.values[2].text=(char*)\"A\"; box.selected_position_=99;\n    box.item_list_.current_=&box.item_list_.entries[1]; box.item_list_.current_position_=4;\n    mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 0);")
    controls = controls.replace(
        "box.item_list_.current_position_=4;\n    mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 0);",
        "box.item_list_.current_position_=4;\n    mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 0);\n    // Too-negative selection resets current to head but leaves the ordinal.\n    box.item_list_.values[1].text=(char*)\"A\"; box.item_list_.values[2].text=(char*)\"x\"; box.selected_position_=-6;\n    box.item_list_.current_=&box.item_list_.entries[3]; box.item_list_.current_position_=4;\n    mapped_character='A'; assert(box.on_key_down('A') == 1 && box.selected_position_ == 0);")
    controls = controls.replace(
        '    // Literal UNK1 body uses the evidenced blit geometry and typed APIs.',
        '''    // MapVirtualKey tests only its low byte, then reloads the live selection.
    box.selected_position_=0; box.item_list_.values[1].text=(char*)""; mapped_character=0x100;
    assert(box.on_key_down('A') == 0 && box.selected_position_ == 0);
    box.item_list_.values[1].text=(char*)"A"; box.item_list_.values[3].text=(char*)"A";
    box.selected_position_=0; mapped_character='A'; mapped_selection=&box.selected_position_; mapped_selected_value=2;
    assert(box.on_key_down('A') == 1 && box.selected_position_ == 3); mapped_selection=0;
    // With no rows, the original still performs cursor setup before its
    // later count check: a negative selected ordinal installs head but leaves
    // current_position_ unchanged.
    box.selected_position_=-2; box.item_list_.entry_count_=0;
    box.item_list_.current_=&box.item_list_.entries[3]; box.item_list_.current_position_=4;
    assert(box.on_key_down('A') == 0);
    assert(box.item_list_.current_ == &box.item_list_.entries[0] && box.item_list_.current_position_ == 4);
    box.item_list_.entry_count_=5;
    int maps_before_empty=map_calls; box.item_list_.entry_count_=0; box.item_list_.head_=0;
    assert(box.on_key_down('A') == 0 && map_calls == maps_before_empty+1);
    box.item_list_.entry_count_=5; box.item_list_.head_=&box.item_list_.entries[0];

    // Literal UNK1 body uses the evidenced blit geometry and typed APIs.''')
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
            for mutant_index, mutant_body in enumerate(mutant_methods):
                mutant_out = packet / ('typeahead-mutant-' + str(mutant_index) + '-' + name + '.cpp')
                mutant_out.write_text(prefix + hit_helpers + dispatch + mutant_body + controls)
                mutant_exe = packet / ('typeahead-mutant-' + str(mutant_index) + '-' + name)
                subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
                                str(mutant_out), '-o', str(mutant_exe)], check=True,
                               capture_output=True, text=True, timeout=30)
                rejected = subprocess.run([str(mutant_exe)], capture_output=True,
                                          text=True, timeout=30)
                assert rejected.returncode != 0, f'typeahead mutant {mutant_index} was not rejected'
            wrong_import_out = packet / ('wrong-key-import-' + name + '.cpp')
            wrong_import_out.write_text(prefix + hit_helpers + dispatch + wrong_import_methods + controls)
            wrong_import_exe = packet / ('wrong-key-import-' + name)
            subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
                            str(wrong_import_out), '-o', str(wrong_import_exe)], check=True,
                           capture_output=True, text=True, timeout=30)
            rejected_import = subprocess.run([str(wrong_import_exe)], capture_output=True,
                                             text=True, timeout=30)
            assert rejected_import.returncode != 0, 'GetKeyState import mutant was not rejected'
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
