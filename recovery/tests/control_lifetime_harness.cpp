// Actual FileWin/EditBox declarations and lifetime bodies with dependency
// doubles. Binary controls separately observe the real ListBox/base layouts.
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <new>
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef int int32_t;
int events[64], count, buttons, fail_time, fail_list, fail_close;
void event(int value) { assert(count < 64); events[count++] = value; }
struct GraphicWin {
    GraphicWin() { event(21); }
    virtual ~GraphicWin() { event(25); }
};
struct Time {
    Time() { event(22); if (fail_time) throw 22; }
    ~Time() { event(24); }
};
// LIVE_EDITBOX_DECL
void EditBox::vslot_12() {}
void EditBox::vslot_17(int, int) {}
void EditBox::vslot_20(int, int) {}
void EditBox::vslot_21(int, int) {}
int EditBox::vslot_25(int, int) { return 0; }
int EditBox::vslot_26(int) { return 0; }
void EditBox::vslot_51(int) {}
void EditBox::pass_dialog_focus() {}
uint32_t EditBoxDefaultB1C = 255, EditBoxDefaultB20 = -1, EditBoxDefaultB24 = -1;
uint32_t EditBoxDefaultB28, EditBoxDefaultB2C, EditBoxDefaultB30 = 1, EditBoxDefaultB34 = 4;
// LIVE_EDITBOX_CTOR
// LIVE_EDITBOX_DTOR
void EditBox::close() { event(23); }
struct BaseButton {
    int id;
    uint32_t field_A14_, field_A18_;
    BaseButton() : id(++buttons) { event(10 + id); }
    ~BaseButton() { event(40 + id); }
    BaseButton *construct() { return new(this) BaseButton; }
};
#define MEASURED
// LIVE_FLATBUTTON_DECL
int FlatButtonDefaultsTable[27];
// LIVE_FLATBUTTON_CTOR
uint32_t FlatButton::close() {
    assert(field_A14_ == 0 && field_A18_ == 0 && field_AB8_ == 0xFFFFFFFFU);
    for (int i = 0; i < 3; ++i) {
        assert(field_ABC_[i] == 0xFFFFFFFFU && field_AC8_[i] == 0 && field_AD4_[i] == 0);
    }
    for (int j = 0; j < 27; ++j) assert(config[j] == static_cast<uint32_t>(FlatButtonDefaultsTable[j]));
    return 0;
}
struct ListBox {
    ListBox() { event(30); if (fail_list) throw 30; }
    ~ListBox() { event(31); }
};
// LIVE_FILEWIN_DECL
const char FileWinTextTableDefault[] = ".";
// LIVE_FILEWIN_CTOR
// LIVE_FILEWIN_DTOR
void FileWin::close() { event(50); if (fail_close) throw 50; }
void expect(const int *values, int size) {
    assert(count == size);
    for (int i = 0; i < size; ++i) assert(events[i] == values[i]);
    count = buttons = 0;
}
int main() {
    for (int i = 0; i < 27; ++i) FlatButtonDefaultsTable[i] = 1000 + i;
    const int normal[] = {11, 12, 21, 22, 30, 50, 31, 23, 24, 25, 42, 41};
    { FileWin win; }
    expect(normal, 12);

    fail_time = 1;
    bool caught = false;
    try { FileWin win; }
    catch (int value) { assert(value == 22); caught = true; }
    assert(caught);
    const int partial_time[] = {11, 12, 21, 22, 25, 42, 41};
    expect(partial_time, 7);
    fail_time = 0;

    fail_list = 1;
    caught = false;
    try { FileWin win; }
    catch (int value) { assert(value == 30); caught = true; }
    assert(caught);
    const int partial_list[] = {11, 12, 21, 22, 30, 23, 24, 25, 42, 41};
    expect(partial_list, 10);
    fail_list = 0;

    void *storage = malloc(sizeof(FileWin));
    FileWin *win = ::new(storage) FileWin;
    fail_close = 1;
    caught = false;
    try { win->~FileWin(); }
    catch (int value) { assert(value == 50); caught = true; }
    assert(caught);
    expect(normal, 12);
    free(storage);
    fail_close = 0;

    storage = malloc(sizeof(EditBox));
    memset(storage, 0xA5, sizeof(EditBox));
    EditBox *edit = ::new(storage) EditBox;
    assert(edit->text_[0] == 0);
    assert(static_cast<unsigned char>(edit->text_[1]) == 0xA5);
    assert(edit->field_B14_ == 10 && edit->field_B34_ == 4);
    edit->~EditBox();
    const int edit_life[] = {21, 22, 23, 24, 25};
    expect(edit_life, 5);
    free(storage);
    return 0;
}
