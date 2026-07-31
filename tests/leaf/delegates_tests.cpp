// Recovery leaf tests: delegates.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "delegates"

namespace {

BasePop *g_closed_base_pop = nullptr;

int g_base_pop_close_calls = 0;

int g_scroll_order_at_base_close = -1;

void __thiscall observe_base_pop_close(BasePop *self) {
    g_closed_base_pop = self;
    // The scroll must already be closed by the time the base close runs.
    g_scroll_order_at_base_close = scroll_close_probe_state.order;
    ++g_base_pop_close_calls;
}

}  // namespace

void test_popup_close() {
    // 104 callers, the highest fan-in function left. It closes the Scroll at
    // 0x3230 - exactly sizeof(BasePop) - then the popup base. Scroll::close is
    // real recovered code, so it runs for real here against the shared
    // fixture, positioned at the member rather than at the popup: if close()
    // aimed at `this` instead of the member, none of the fixture would be
    // where Scroll::close looks and the probes would not fire.
    ScrollCloseFixture scroll_fixture;
    scroll_fixture.install();
    auto *const saved_close = BasePopOriginalClose;
    BasePopOriginalClose = &observe_base_pop_close;

    std::vector<uint8_t> storage(sizeof(Popup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *popup_object = reinterpret_cast<Popup *>(storage.data() + 16);
    uint8_t *const member = storage.data() + 16 + 0x3230;

    seed_storage(storage.data(), expected.data(), storage.size());
    scroll_fixture.prepare(member);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_base_pop_close_calls = 0;
    popup_object->close();

    expect(scroll_close_probe_state.order == 4);
    expect(g_base_pop_close_calls == 1);
    expect(reinterpret_cast<void *>(g_closed_base_pop) ==
           reinterpret_cast<void *>(popup_object));
    // Ordering: the scroll is fully closed before the base close runs.
    expect(g_scroll_order_at_base_close == 4);
    // Only the Scroll member changed; the BasePop region ahead of it is
    // untouched, which is what shows close() did not aim at the wrong object.
    std::memcpy(expected.data() + 16 + 0x3230, storage.data() + 16 + 0x3230,
                sizeof(Scroll));
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    seed_storage(storage.data(), expected.data(), storage.size());
    scroll_fixture.prepare(member);
    std::memcpy(expected.data(), storage.data(), storage.size());
    popup_close_redirect(popup_object, nullptr);
    expect(scroll_close_probe_state.order == 4);
    expect(g_base_pop_close_calls == 2);
    std::memcpy(expected.data() + 16 + 0x3230, storage.data() + 16 + 0x3230,
                sizeof(Scroll));
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    BasePopOriginalClose = saved_close;
    scroll_fixture.restore();
}

namespace {

Popup *g_hidden_popup = nullptr;

int g_popup_hide_calls = 0;

int g_flag_at_hide = -1;

void __thiscall observe_popup_hide(Popup *self) {
    g_hidden_popup = self;
    std::memcpy(&g_flag_at_hide,
                reinterpret_cast<const uint8_t *>(self) + 0x5384, sizeof(int));
    ++g_popup_hide_calls;
}

void *g_released_interface = nullptr;

int g_release_calls = 0;

int g_status_flag_at_release = -1;

const uint8_t *g_status_object = nullptr;

void __thiscall observe_release_iface_mode(void *self) {
    g_released_interface = self;
    std::memcpy(&g_status_flag_at_release, g_status_object + 0x15D4, sizeof(int));
    ++g_release_calls;
}

}  // namespace

void test_guarded_delegates() {
    // NetMsg::close stops its Time then hides the popup, but only when a
    // message is actually showing.
    auto *const saved_hide = PopupOriginalHide;
    PopupOriginalHide = &observe_popup_hide;
    std::vector<uint8_t> nm(sizeof(NetMsg) + 32), nm_want(nm.size());
    auto *message = reinterpret_cast<NetMsg *>(nm.data() + 16);
    auto set_showing = [&](int32_t value) {
        std::memcpy(nm.data() + 16 + 0x5384, &value, sizeof(value));
    };

    seed_storage(nm.data(), nm_want.data(), nm.size());
    set_showing(0);
    std::memcpy(nm_want.data(), nm.data(), nm.size());
    g_popup_hide_calls = 0;
    message->close();
    expect(g_popup_hide_calls == 0);
    expect_storage_bytes(nm.data(), nm_want.data(), nm.size());

    seed_storage(nm.data(), nm_want.data(), nm.size());
    set_showing(0x1234);
    std::memcpy(nm_want.data(), nm.data(), nm.size());
    g_popup_hide_calls = 0;
    message->close();
    expect(g_popup_hide_calls == 1);
    expect(reinterpret_cast<void *>(g_hidden_popup) ==
           reinterpret_cast<void *>(message));
    // The flag is cleared before the hide, not after.
    expect(g_flag_at_hide == 0);
    // Time::close cleared its own member and nothing else moved besides the
    // flag - in particular the Popup region ahead of 0x537C is untouched.
    std::memcpy(nm_want.data() + 16 + 0x5384, nm.data() + 16 + 0x5384, 4);
    std::memcpy(nm_want.data() + 16 + 0x538C, nm.data() + 16 + 0x538C,
                sizeof(Time));
    expect_storage_bytes(nm.data(), nm_want.data(), nm.size());

    set_showing(-1);
    g_popup_hide_calls = 0;
    net_msg_close_redirect(message, nullptr);
    expect(g_popup_hide_calls == 1);
    PopupOriginalHide = saved_hide;

    // StatusWin::reset releases the interface mode on a global object, and
    // the original tail-jumps into it, so nothing may follow the call.
    auto *const saved_release = SubInterfaceOriginalReleaseIfaceMode;
    void *const saved_global = SubInterfaceGlobal;
    int fake_interface = 0;
    SubInterfaceOriginalReleaseIfaceMode = &observe_release_iface_mode;
    SubInterfaceGlobal = &fake_interface;

    std::vector<uint8_t> sw(sizeof(StatusWin) + 32), sw_want(sw.size());
    auto *status = reinterpret_cast<StatusWin *>(sw.data() + 16);
    g_status_object = sw.data() + 16;
    auto set_held = [&](int32_t value) {
        std::memcpy(sw.data() + 16 + 0x15D4, &value, sizeof(value));
    };

    seed_storage(sw.data(), sw_want.data(), sw.size());
    set_held(0);
    std::memcpy(sw_want.data(), sw.data(), sw.size());
    g_release_calls = 0;
    status->reset();
    expect(g_release_calls == 0);
    expect_storage_bytes(sw.data(), sw_want.data(), sw.size());

    seed_storage(sw.data(), sw_want.data(), sw.size());
    set_held(0x5A5A);
    std::memcpy(sw_want.data(), sw.data(), sw.size());
    g_release_calls = 0;
    status->reset();
    expect(g_release_calls == 1);
    // The global is passed, not the status window - the original loads a
    // fixed address into ecx rather than forwarding `this`.
    expect(g_released_interface == &fake_interface);
    expect(g_status_flag_at_release == 0);
    std::memcpy(sw_want.data() + 16 + 0x15D4, sw.data() + 16 + 0x15D4, 4);
    expect_storage_bytes(sw.data(), sw_want.data(), sw.size());

    set_held(INT_MIN);
    g_release_calls = 0;
    status_win_reset_redirect(status, nullptr);
    expect(g_release_calls == 1);
    SubInterfaceOriginalReleaseIfaceMode = saved_release;
    SubInterfaceGlobal = saved_global;
}

namespace {

Dialog *g_closed_dialog = nullptr;

int g_dialog_close_calls = 0;

void __thiscall observe_dialog_close(Dialog *self) {
    g_closed_dialog = self;
    ++g_dialog_close_calls;
}

const void *g_win_closed = nullptr;

const void *g_buffer_closed = nullptr;

void __thiscall observe_win_close(void *self) { g_win_closed = self; }

void __thiscall observe_buffer_close(void *self) { g_buffer_closed = self; }

}  // namespace

void test_virtual_base_closes() {
    // RadioButton and CheckBox resolve both of their calls through the
    // vbtable - [edx+4] for the virtual GraphicWin, [edx+8] for the Dialog -
    // so the whole point of these tests is which subobject each call reaches.
    // GraphicWin::close is real recovered code needing its two subobject
    // seams and its default. The Scroll fixture cannot serve here - its
    // probes assert Scroll's own call ordering against a Scroll base - so
    // these get plain observers instead.
    uint32_t base_default = 0x0BADF00DU;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_default = GraphicWinFieldA0CDefault;
    WinOriginalClose = observe_win_close;
    BufferSubobjectClose = observe_buffer_close;
    GraphicWinFieldA0CDefault = &base_default;
    auto *const saved_dialog = RadioButtonOriginalDialogClose;
    RadioButtonOriginalDialogClose = &observe_dialog_close;
    auto *const saved_check_dialog = CheckBoxOriginalDialogClose;
    CheckBoxOriginalDialogClose = &observe_dialog_close;

    uint32_t radio_one = 0x11112222U;
    uint32_t radio_two = 0x33334444U;
    uint32_t *const saved_r1 = RadioButtonDefault1;
    uint32_t *const saved_r2 = RadioButtonDefault2;
    RadioButtonDefault1 = &radio_one;
    RadioButtonDefault2 = &radio_two;

    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        uint32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };

    // Two vbtables: the one a most-derived RadioButton uses, and a second
    // placing the same subobjects elsewhere, which is what happens when the
    // class is embedded in a larger one - Dialogs holds a RadioButton at 0x44.
    // Hardcoding this class's own offsets passes the first case and fails the
    // second, which is exactly what reached the game and crashed it.
    const int32_t own_vbtable[3] = {0, 0x18, 0xA30};
    const int32_t embedded_vbtable[3] = {0, 0x30, 0xA60};

    std::vector<uint8_t> rb(sizeof(RadioButton) + 0xA0 + 32);
    auto *radio = reinterpret_cast<RadioButton *>(rb.data() + 16);
    for (size_t i = 0; i < rb.size(); ++i) {
        rb[i] = static_cast<uint8_t>(0x40 + (i * 7));
    }
    const uint32_t zero = 0;
    auto point_at = [&](const int32_t *table) {
        const int32_t *pointer = table;
        std::memcpy(rb.data() + 16, &pointer, sizeof(pointer));
        // GraphicWin::close follows 0xA08 off whichever base the table names.
        std::memcpy(rb.data() + 16 + table[1] + 0xA08, &zero, sizeof(zero));
    };
    point_at(own_vbtable);
    g_dialog_close_calls = 0;
    radio->close();
    expect(g_dialog_close_calls == 1);
    // The Dialog it closed must be the one at 0xA30, not the object.
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == rb.data() + 16 + 0xA30);
    expect(read32(rb, 0x0C) == 0);
    expect(read32(rb, 0x10) == 0);
    expect(read32(rb, 0x08) == 0x33334444U);
    expect(read32(rb, 0x04) == 0x11112222U);
    // GraphicWin::close ran against the base at 0x18, not against the object:
    // it clears 0xA10 there and its two subobject closes see that address.
    expect(read32(rb, 0x18 + 0xA10) == 0);
    expect(g_win_closed == rb.data() + 16 + 0x18);
    expect(g_buffer_closed == rb.data() + 16 + 0x18 + 0x444);

    radio_button_close_redirect(radio, nullptr);
    expect(g_dialog_close_calls == 2);

    // Now the same object described by a vbtable that puts the base at 0x30
    // and the Dialog at 0xA60. Everything must follow the table.
    point_at(embedded_vbtable);
    g_dialog_close_calls = 0;
    radio->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == rb.data() + 16 + 0xA60);
    expect(g_win_closed == rb.data() + 16 + 0x30);
    expect(g_buffer_closed == rb.data() + 16 + 0x30 + 0x444);
    expect(read32(rb, 0x30 + 0xA10) == 0);

    uint32_t check_one = 0x55556666U;
    uint32_t check_two = 0x77778888U;
    uint32_t *const saved_c1 = CheckBoxDefault1;
    uint32_t *const saved_c2 = CheckBoxDefault2;
    CheckBoxDefault1 = &check_one;
    CheckBoxDefault2 = &check_two;

    const int32_t check_own[3] = {0, 0x1C, 0xA34};
    const int32_t check_embedded[3] = {0, 0x40, 0xA70};
    std::vector<uint8_t> cb(sizeof(CheckBox) + 0xA0 + 32);
    auto *check = reinterpret_cast<CheckBox *>(cb.data() + 16);
    for (size_t i = 0; i < cb.size(); ++i) {
        cb[i] = static_cast<uint8_t>(0x90 + (i * 5));
    }
    auto point_check_at = [&](const int32_t *table) {
        const int32_t *pointer = table;
        std::memcpy(cb.data() + 16, &pointer, sizeof(pointer));
        std::memcpy(cb.data() + 16 + table[1] + 0xA08, &zero, sizeof(zero));
    };
    point_check_at(check_own);
    g_dialog_close_calls = 0;
    check->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == cb.data() + 16 + 0xA34);
    expect(read32(cb, 0x04) == 0);
    expect(read32(cb, 0x08) == 0);
    expect(read32(cb, 0x0C) == 0);
    expect(read32(cb, 0x14) == 0x77778888U);
    expect(read32(cb, 0x10) == 0x55556666U);
    expect(read32(cb, 0x1C + 0xA10) == 0);
    expect(g_win_closed == cb.data() + 16 + 0x1C);
    expect(g_buffer_closed == cb.data() + 16 + 0x1C + 0x444);
    check_box_close_redirect(check, nullptr);
    expect(g_dialog_close_calls == 2);

    point_check_at(check_embedded);
    g_dialog_close_calls = 0;
    check->close();
    expect(g_dialog_close_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_closed_dialog) == cb.data() + 16 + 0xA70);
    expect(g_win_closed == cb.data() + 16 + 0x40);
    expect(read32(cb, 0x40 + 0xA10) == 0);

    RadioButtonDefault1 = saved_r1;
    RadioButtonDefault2 = saved_r2;
    CheckBoxDefault1 = saved_c1;
    CheckBoxDefault2 = saved_c2;
    RadioButtonOriginalDialogClose = saved_dialog;
    CheckBoxOriginalDialogClose = saved_check_dialog;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_default;
}

namespace {

void *g_edit_box = nullptr;

char *g_edit_text = nullptr;

int g_set_text_calls = 0;

void __thiscall observe_edit_box_set_text(void *self, char *text) {
    g_edit_box = self;
    g_edit_text = text;
    ++g_set_text_calls;
}

}  // namespace

void test_edit_group_text() {
    auto *const saved = EditBoxOriginalSetText;
    EditBoxOriginalSetText = &observe_edit_box_set_text;

    std::vector<uint8_t> storage(sizeof(EditGroup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *group = reinterpret_cast<EditGroup *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());

    // Two boxes present, one absent - the absent slot is what both accessors
    // guard on, and the original returns nothing rather than an offset from
    // null, which is the difference a missing check would produce.
    uint8_t first_box[0xA20] = {};
    uint8_t second_box[0xA20] = {};
    void *slots[3] = {first_box, nullptr, second_box};
    for (size_t index = 0; index < 3; ++index) {
        std::memcpy(storage.data() + 16 + 4 + index * 4, &slots[index],
                    sizeof(void *));
    }
    std::memcpy(expected.data(), storage.data(), storage.size());

    // The text buffer sits 0xA14 into the box, not at its start.
    expect(group->get_text(0) == reinterpret_cast<char *>(first_box) + 0xA14);
    expect(group->get_text(1) == nullptr);
    expect(group->get_text(2) == reinterpret_cast<char *>(second_box) + 0xA14);
    expect(edit_group_get_text_redirect(group, nullptr, 2) ==
           reinterpret_cast<char *>(second_box) + 0xA14);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    char message[] = "hello";
    g_set_text_calls = 0;
    group->set_text(message, 0);
    expect(g_set_text_calls == 1);
    expect(g_edit_box == first_box);
    expect(g_edit_text == message);
    group->set_text(message, 1);          // absent: must not dispatch
    expect(g_set_text_calls == 1);
    edit_group_set_text_redirect(group, nullptr, message, 2);
    expect(g_set_text_calls == 2);
    expect(g_edit_box == second_box);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // set_text_limits fills exactly ten dwords from 0x54 and nothing beyond.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    group->set_text_limits(0x5A5A5A5A);
    for (size_t index = 0; index < 10; ++index) {
        uint32_t value = 0;
        std::memcpy(&value, storage.data() + 16 + 0x54 + index * 4, sizeof(value));
        expect(value == 0x5A5A5A5AU);
    }
    std::memcpy(expected.data() + 16 + 0x54, storage.data() + 16 + 0x54, 10 * 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    edit_group_set_text_limits_redirect(group, nullptr, -1);
    std::memcpy(expected.data() + 16 + 0x54, storage.data() + 16 + 0x54, 10 * 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    EditBoxOriginalSetText = saved;
}

namespace {

Dialog *g_item_dialog = nullptr;

void *g_item_listbox = nullptr;

char *g_item_text = nullptr;

int g_item_index = 0;

int g_dialog_item_calls = 0;

int g_listbox_item_calls = 0;

int __thiscall observe_dialog_item(Dialog *self, char *text, int index) {
    g_item_dialog = self; g_item_text = text; g_item_index = index;
    ++g_dialog_item_calls;
    return 0x1234;
}

int __thiscall observe_list_box_item(void *self, char *text, int index) {
    g_item_listbox = self; g_item_text = text; g_item_index = index;
    ++g_listbox_item_calls;
    return 0x5678;
}

}  // namespace

void test_dialogs_dispatch() {
    // Both methods switch on the dialog kind through a sixteen entry jump
    // table, so every kind is exercised rather than a representative few -
    // the table is not regular, and a case transcribed into the wrong arm
    // would pass any test that only sampled it.
    auto *const saved_dialog = DialogOriginalItem;
    auto *const saved_listbox = ListBoxOriginalItem;
    DialogOriginalItem = &observe_dialog_item;
    ListBoxOriginalItem = &observe_list_box_item;

    std::vector<uint8_t> storage(sizeof(Dialogs) + 0x100 + 32);
    auto *dialogs = reinterpret_cast<Dialogs *>(storage.data() + 16);
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x30 + (i * 3));
    }
    // A vbtable placing the Dialog somewhere other than a most-derived
    // Dialogs would put it, so a hardcoded offset cannot pass.
    const int32_t vbtable[3] = {0, 0x188, 0xC40};
    const int32_t *pointer = vbtable;
    std::memcpy(storage.data() + 16, &pointer, sizeof(pointer));
    uint8_t *const dialog_at = storage.data() + 16 + 0xC40;
    const int32_t count = 0x0BADBEEF;
    std::memcpy(dialog_at + 0xCC, &count, sizeof(count));
    const int32_t fallback = 0x00C0FFEE;
    std::memcpy(storage.data() + 16 + 0xA4, &fallback, sizeof(fallback));

    auto set_kind = [&](int32_t kind) {
        std::memcpy(storage.data() + 16 + 0x180, &kind, sizeof(kind));
    };
    char text[] = "item";

    // item: kinds 1, 4, 8 and 16 go to the Dialog; kind 2 to the ListBox;
    // everything else, in range or not, yields zero.
    const int dialog_kinds[] = {1, 4, 8, 16};
    for (int kind : dialog_kinds) {
        set_kind(kind);
        g_dialog_item_calls = 0;
        expect(dialogs->item(text, 7) == 0x1234);
        expect(g_dialog_item_calls == 1);
        expect(reinterpret_cast<uint8_t *>(g_item_dialog) == dialog_at);
        expect(g_item_text == text);
        expect(g_item_index == 7);
    }
    set_kind(2);
    g_listbox_item_calls = 0;
    expect(dialogs->item(text, 3) == 0x5678);
    expect(g_listbox_item_calls == 1);
    // The ListBox arm passes the object itself, unadjusted.
    expect(g_item_listbox == reinterpret_cast<void *>(dialogs));
    for (int kind : {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15}) {
        set_kind(kind);
        g_dialog_item_calls = 0;
        g_listbox_item_calls = 0;
        expect(dialogs->item(text, 0) == 0);
        expect(g_dialog_item_calls == 0);
        expect(g_listbox_item_calls == 0);
    }
    for (int kind : {0, -1, 17, INT_MIN, INT_MAX}) {
        set_kind(kind);
        expect(dialogs->item(text, 0) == 0);
    }
    set_kind(1);
    expect(dialogs_item_redirect(dialogs, nullptr, text, 2) == 0x1234);

    // get_num_items: kinds 1, 2, 4 and 16 read the Dialog's count, kind 8
    // reads the object's own field, the rest are zero. Note kind 2 differs
    // between the two methods, which is the kind of asymmetry a shared
    // switch would paper over.
    for (int kind : {1, 2, 4, 16}) {
        set_kind(kind);
        expect(dialogs->get_num_items() == 0x0BADBEEF);
    }
    set_kind(8);
    expect(dialogs->get_num_items() == 0x00C0FFEE);
    for (int kind : {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 0, 17}) {
        set_kind(kind);
        expect(dialogs->get_num_items() == 0);
    }
    set_kind(4);
    expect(dialogs_get_num_items_redirect(dialogs, nullptr) == 0x0BADBEEF);

    DialogOriginalItem = saved_dialog;
    ListBoxOriginalItem = saved_listbox;
}

namespace {

struct XPopsCall {
    char *buffer; const char *label; int title; char *override_text;
    int value; Sprite *sprite; int flag1; int flag2; int (__cdecl *callback)();
    int calls;
} g_xpops = {};

int __cdecl observe_x_pops(char *buffer, const char *label, int title,
                           char *override_text, int value, Sprite *sprite,
                           int flag1, int flag2, int (__cdecl *callback)()) {
    g_xpops = {buffer, label, title, override_text, value, sprite,
               flag1, flag2, callback, g_xpops.calls + 1};
    return 0x4321;
}

int __cdecl xpops_callback() { return 0; }

void *g_date_target = nullptr;

char *g_date_text = nullptr;

int g_set_date_calls = 0;

void __thiscall observe_set_date(void *self, char *text) {
    g_date_target = self; g_date_text = text; ++g_set_date_calls;
}

float g_rotation[3] = {};

void *g_rotation_target = nullptr;

int g_rotation_calls = 0;

void __cdecl observe_apply_rotation(float *angles, void *matrix) {
    std::memcpy(g_rotation, angles, sizeof(g_rotation));
    g_rotation_target = matrix;
    ++g_rotation_calls;
}

}  // namespace

void test_fixed_argument_delegates() {
    // x_pops_short supplies five of the nine arguments itself. Their values
    // and their order are the entire content of the function, so each is
    // checked rather than just that the call happened.
    auto *const saved_full = XPopsOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopsOriginalFull = &observe_x_pops;
    XPopsCaptionBuffer = buffer;
    Sprite sprite_value;
    const char label[] = "label";
    g_xpops.calls = 0;
    expect(x_pops_short(label, 42, &sprite_value, &xpops_callback) == 0x4321);
    expect(g_xpops.calls == 1);
    expect(g_xpops.buffer == buffer);
    expect(g_xpops.label == label);
    expect(g_xpops.title == -1);
    expect(g_xpops.override_text == nullptr);
    expect(g_xpops.value == 42);
    expect(g_xpops.sprite == &sprite_value);
    expect(g_xpops.flag1 == 1);
    expect(g_xpops.flag2 == 1);
    expect(g_xpops.callback == &xpops_callback);
    XPopsOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;

    // main_caption acts on a fixed interface and caption, never on the map
    // window it is called through - passing `this` would compile and be wrong.
    auto *const saved_date = MainInterfaceOriginalSetDate;
    void *const saved_interface = MainInterfaceGlobal;
    char *const saved_caption = MapWinMainCaption;
    int fake_interface = 0;
    char caption[] = "2101.01";
    MainInterfaceOriginalSetDate = &observe_set_date;
    MainInterfaceGlobal = &fake_interface;
    MapWinMainCaption = caption;
    std::vector<uint8_t> mw(sizeof(MapWin) + 32);
    std::vector<uint8_t> mw_want(mw.size());
    auto *map_window = reinterpret_cast<MapWin *>(mw.data() + 16);
    seed_storage(mw.data(), mw_want.data(), mw.size());
    std::memcpy(mw_want.data(), mw.data(), mw.size());
    g_set_date_calls = 0;
    map_window->main_caption();
    expect(g_set_date_calls == 1);
    expect(g_date_target == &fake_interface);
    expect(g_date_target != reinterpret_cast<void *>(map_window));
    expect(g_date_text == caption);
    expect_storage_bytes(mw.data(), mw_want.data(), mw.size());
    map_win_main_caption_redirect(map_window, nullptr);
    expect(g_set_date_calls == 2);
    MainInterfaceOriginalSetDate = saved_date;
    MainInterfaceGlobal = saved_interface;
    MapWinMainCaption = saved_caption;

    // set_scene_rotation passes the three angles in order and the matrix at
    // 0x38, not the object.
    auto *const saved_rotation = CaviarOriginalApplyRotation;
    CaviarOriginalApplyRotation = &observe_apply_rotation;
    std::vector<uint8_t> cv(sizeof(Caviar) + 32);
    std::vector<uint8_t> cv_want(cv.size());
    auto *caviar = reinterpret_cast<Caviar *>(cv.data() + 16);
    seed_storage(cv.data(), cv_want.data(), cv.size());
    std::memcpy(cv_want.data(), cv.data(), cv.size());
    g_rotation_calls = 0;
    caviar->set_scene_rotation(1.5f, -2.25f, 0.125f);
    expect(g_rotation_calls == 1);
    expect(g_rotation[0] == 1.5f);
    expect(g_rotation[1] == -2.25f);
    expect(g_rotation[2] == 0.125f);
    expect(g_rotation_target == cv.data() + 16 + 0x38);
    expect_storage_bytes(cv.data(), cv_want.data(), cv.size());
    caviar_set_scene_rotation_redirect(caviar, nullptr, 3.0f, 4.0f, 5.0f);
    expect(g_rotation[0] == 3.0f);
    expect(g_rotation[2] == 5.0f);
    CaviarOriginalApplyRotation = saved_rotation;
}

namespace {

void *g_fill_target = nullptr;

int g_fill_args[5] = {};

int g_fill_calls = 0;

int __thiscall observe_buffer_fill(void *self, int a, int b, int c, int d, int e) {
    g_fill_target = self;
    g_fill_args[0]=a; g_fill_args[1]=b; g_fill_args[2]=c; g_fill_args[3]=d; g_fill_args[4]=e;
    ++g_fill_calls;
    return 0x9ABC;
}

void *g_check_target = nullptr;

long g_check_value = 0;

int g_check_calls = 0;

void __thiscall observe_set_state_flag(void *self, long value) {
    g_check_target = self; g_check_value = value; ++g_check_calls;
}

struct StartCall {
    Popup *self; char *a1; const char *a2; int a3; char *a4; int a5; void *parent;
    int calls;
} g_start = {};

void __thiscall observe_popup_start_full(Popup *self, char *a1, const char *a2,
                                         int a3, char *a4, int a5, void *parent) {
    g_start = {self, a1, a2, a3, a4, a5, parent, g_start.calls + 1};
}

}  // namespace

void test_offset_delegates() {
    // GraphicWin::fill forwards to Buffer::fill on the buffer at 0x444, with
    // the five arguments in order. The point is the target is the member, not
    // the window, and the order survives.
    auto *const saved_fill = BufferOriginalFill;
    BufferOriginalFill = &observe_buffer_fill;
    std::vector<uint8_t> gw(sizeof(GraphicWin) + 32);
    std::vector<uint8_t> gw_want(gw.size());
    auto *window = reinterpret_cast<GraphicWin *>(gw.data() + 16);
    seed_storage(gw.data(), gw_want.data(), gw.size());
    std::memcpy(gw_want.data(), gw.data(), gw.size());
    g_fill_calls = 0;
    expect(window->fill(11, 22, 33, 44, 55) == 0x9ABC);
    expect(g_fill_calls == 1);
    expect(g_fill_target == gw.data() + 16 + 0x444);
    expect(g_fill_args[0] == 11 && g_fill_args[4] == 55);
    expect(g_fill_args[1] == 22 && g_fill_args[2] == 33 && g_fill_args[3] == 44);
    expect_storage_bytes(gw.data(), gw_want.data(), gw.size());
    graphic_win_fill_redirect(window, nullptr, -1, -2, -3, -4, -5);
    expect(g_fill_args[0] == -1 && g_fill_args[4] == -5);
    BufferOriginalFill = saved_fill;

    // BasePop::write_check forwards to CheckBox::set_state_flag on the member
    // at 0x2228, passing the value through.
    auto *const saved_check = CheckBoxOriginalSetStateFlag;
    CheckBoxOriginalSetStateFlag = &observe_set_state_flag;
    std::vector<uint8_t> bp(sizeof(BasePop) + 32);
    std::vector<uint8_t> bp_want(bp.size());
    auto *popup = reinterpret_cast<BasePop *>(bp.data() + 16);
    seed_storage(bp.data(), bp_want.data(), bp.size());
    std::memcpy(bp_want.data(), bp.data(), bp.size());
    g_check_calls = 0;
    popup->write_check(0x12345678L);
    expect(g_check_calls == 1);
    expect(g_check_target == bp.data() + 16 + 0x2228);
    expect(g_check_value == 0x12345678L);
    expect_storage_bytes(bp.data(), bp_want.data(), bp.size());
    base_pop_write_check_redirect(popup, nullptr, -1L);
    expect(g_check_value == -1L);
    CheckBoxOriginalSetStateFlag = saved_check;

    // Popup::start forwards its five arguments and appends a null parent.
    auto *const saved_start = PopupOriginalStartFull;
    PopupOriginalStartFull = &observe_popup_start_full;
    std::vector<uint8_t> pu(sizeof(Popup) + 32);
    auto *pstart = reinterpret_cast<Popup *>(pu.data() + 16);
    char a1[] = "a"; const char a2[] = "b"; char a4[] = "d";
    g_start.calls = 0;
    pstart->start(a1, a2, 7, a4, 9);
    expect(g_start.calls == 1);
    expect(g_start.self == pstart);
    expect(g_start.a1 == a1 && g_start.a2 == a2 && g_start.a4 == a4);
    expect(g_start.a3 == 7 && g_start.a5 == 9);
    expect(g_start.parent == nullptr);
    popup_start_redirect(pstart, nullptr, a1, a2, -3, a4, -5);
    expect(g_start.a3 == -3 && g_start.a5 == -5 && g_start.parent == nullptr);
    PopupOriginalStartFull = saved_start;
}

void test_win_sync_palette() {
    // sync_palette caches the active palette's generation counter at 0x184 and
    // skips the update when it already matches. set_active_window happens to
    // be an empty stub, but the guard is what this checks: the counter is read
    // again after the call, so a version that cached the stale value would be
    // caught if the counter ever changed across it, and the early return must
    // touch nothing when the generations already agree.
    // Rebind the seam to a local slot rather than writing through the fixed
    // address, which is unmapped here - the recovery only reads *WinActivePalette.
    Palette **const saved = WinActivePalette;
    alignas(Palette) uint8_t palette_storage[sizeof(Palette)] = {};
    auto *palette = reinterpret_cast<Palette *>(palette_storage);
    Palette *palette_slot = palette;
    WinActivePalette = &palette_slot;
    auto set_generation = [&](uint32_t value) {
        std::memcpy(palette_storage + 0x400, &value, sizeof(value));
    };

    std::vector<uint8_t> wn(sizeof(Win) + 32);
    std::vector<uint8_t> wn_want(wn.size());
    auto *window = reinterpret_cast<Win *>(wn.data() + 16);
    auto cached = [&] {
        uint32_t value = 0;
        std::memcpy(&value, wn.data() + 16 + 0x184, sizeof(value));
        return value;
    };
    auto set_cached = [&](uint32_t value) {
        std::memcpy(wn.data() + 16 + 0x184, &value, sizeof(value));
    };

    // Already in step: nothing happens, not even a write of the same value.
    seed_storage(wn.data(), wn_want.data(), wn.size());
    set_generation(0x1111);
    set_cached(0x1111);
    std::memcpy(wn_want.data(), wn.data(), wn.size());
    window->sync_palette();
    expect(cached() == 0x1111);
    expect_storage_bytes(wn.data(), wn_want.data(), wn.size());

    // Behind: the cache is brought up to the palette's generation.
    seed_storage(wn.data(), wn_want.data(), wn.size());
    set_generation(0xABCD);
    set_cached(0x2222);
    std::memcpy(wn_want.data(), wn.data(), wn.size());
    window->sync_palette();
    expect(cached() == 0xABCD);
    std::memcpy(wn_want.data() + 16 + 0x184, wn.data() + 16 + 0x184, 4);
    expect_storage_bytes(wn.data(), wn_want.data(), wn.size());

    set_generation(0x7F7F);
    set_cached(0);
    win_sync_palette_redirect(window, nullptr);
    expect(cached() == 0x7F7F);

    WinActivePalette = saved;
}

void test_self_contained_stores() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };

    // PlanWin::clear_lines zeroes one dword at 0x21FF8, carved out of the
    // pinned derived storage. If the carve moved it, the build's static_assert
    // would have failed; here the check is only that the right dword clears
    // and nothing else does.
    std::vector<uint8_t> pw(sizeof(PlanWin) + 32);
    std::vector<uint8_t> pw_want(pw.size());
    auto *plan = reinterpret_cast<PlanWin *>(pw.data() + 16);
    seed_storage(pw.data(), pw_want.data(), pw.size());
    std::memcpy(pw_want.data(), pw.data(), pw.size());
    plan->clear_lines();
    expect(read32(pw, 0x21FF8) == 0);
    std::memcpy(pw_want.data() + 16 + 0x21FF8, pw.data() + 16 + 0x21FF8, 4);
    expect_storage_bytes(pw.data(), pw_want.data(), pw.size());
    plan_win_clear_lines_redirect(plan, nullptr);

    // PlayerLock::clear resets two three-dword entries and the flag byte at 0.
    // The two -1 sentinels and the zero within each entry are asserted per
    // field: a loop that wrote them in the wrong order, or cleared where it
    // should sentinel, would pass a check that only looked at the whole entry.
    std::vector<uint8_t> pl(sizeof(PlayerLock) + 32);
    std::vector<uint8_t> pl_want(pl.size());
    auto *lock = reinterpret_cast<PlayerLock *>(pl.data() + 16);
    seed_storage(pl.data(), pl_want.data(), pl.size());
    std::memcpy(pl_want.data(), pl.data(), pl.size());
    lock->clear();
    expect((pl[16] & 0xFF) == 0);          // active flag byte at 0
    expect(read32(pl, 0x04) == -1);
    expect(read32(pl, 0x08) == -1);
    expect(read32(pl, 0x0C) == 0);
    expect(read32(pl, 0x10) == -1);
    expect(read32(pl, 0x14) == -1);
    expect(read32(pl, 0x18) == 0);
    // Everything written sits in 0..0x1C; the byte at 0 plus the two entries.
    pl_want[16] = pl[16];
    std::memcpy(pl_want.data() + 16 + 4, pl.data() + 16 + 4, 0x18);
    expect_storage_bytes(pl.data(), pl_want.data(), pl.size());
    player_lock_clear_redirect(lock, nullptr);
}

void test_base_pop_set_width() {
    // set_width writes into the Dialog subobject of the embedded Dialogs at
    // 0x21D0, located through that Dialogs' own vbtable, and scales the width
    // by three-halves in the high-resolution layout unless the popup opts out.
    // Both the scaling condition (three independent ways to disable it) and
    // the vbtable-relative store location are what the test pins.
    int32_t screen_width = 0x400;
    int32_t *const saved_screen = BasePopScreenWidth;
    BasePopScreenWidth = &screen_width;

    std::vector<uint8_t> storage(sizeof(BasePop) + 0x40);
    std::vector<uint8_t> expected(storage.size());
    auto *popup = reinterpret_cast<BasePop *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;

    // A vbtable whose Dialog offset is not where a most-derived layout would
    // put it, so a hardcoded store offset could not pass.
    const int32_t vbtable[3] = {0, 0x188, 0x400};
    const int32_t *vbtable_pointer = vbtable;
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    const size_t store_at = 0x21D0 + 0x2C + 0x400;

    auto set_u32 = [&](size_t off, uint32_t v) {
        std::memcpy(object + off, &v, sizeof(v));
    };
    auto stored = [&] {
        int32_t v = 0;
        std::memcpy(&v, object + store_at, sizeof(v));
        return v;
    };

    // All three conditions permit scaling: field_A14 zero, style bit clear,
    // screen wide enough. 100 -> 150.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0);
    screen_width = 0x400;
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    popup->set_width(100);
    expect(stored() == 150);
    // Negative widths truncate toward zero, as the asm's cdq/sub/sar does.
    popup->set_width(-100);
    expect(stored() == -150);
    popup->set_width(3);
    expect(stored() == 4);            // (3*3)/2 = 4 truncated
    popup->set_width(-3);
    expect(stored() == -4);           // (-9)/2 = -4 toward zero

    // Each condition alone forces the raw width.
    set_u32(0xA14, 1);
    popup->set_width(100);
    expect(stored() == 100);
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0x400);
    popup->set_width(100);
    expect(stored() == 100);
    set_u32(0x30A8, 0);
    screen_width = 0x3FF;
    popup->set_width(100);
    expect(stored() == 100);
    screen_width = 0x400;

    // The store lands only at the vbtable-derived location - the rest of the
    // object, including the Dialogs vbtable pointer, is left alone.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_u32(0xA14, 0);
    set_u32(0x30A8, 0);
    std::memcpy(object + 0x21D0, &vbtable_pointer, sizeof(vbtable_pointer));
    std::memcpy(expected.data(), storage.data(), storage.size());
    popup->set_width(42);
    expect(stored() == 63);
    std::memcpy(expected.data() + 16 + store_at, object + store_at, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    base_pop_set_width_redirect(popup, nullptr, 200);
    expect(stored() == 300);

    BasePopScreenWidth = saved_screen;
}

namespace {

void *g_mapwin_freed = nullptr;

int g_mapwin_free_calls = 0;

void *observe_map_win_free(void *pointer) {
    g_mapwin_freed = pointer;
    ++g_mapwin_free_calls;
    return nullptr;
}

}  // namespace

void test_map_win_close() {
    // close() frees the buffer at 0x4 and then closes the GraphicWin base,
    // located through the object's own vbtable. GraphicWin::close is real
    // recovered code needing its two subobject seams and its default; it does
    // not do Scroll's left/right dispatch, so it gets plain observers rather
    // than the Scroll fixture. The vbase offset installed here is not where a
    // most-derived MapWin would put it, so a hardcoded base offset cannot pass.
    uint32_t base_default = 0x0BADF00DU;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_default = GraphicWinFieldA0CDefault;
    WinOriginalClose = observe_win_close;
    BufferSubobjectClose = observe_buffer_close;
    GraphicWinFieldA0CDefault = &base_default;
    auto *const saved_free = MapWinFree;
    MapWinFree = &observe_map_win_free;

    const int32_t vbase = 0x1000;
    const int32_t vbtable[2] = {0, vbase};
    const int32_t *vbtable_pointer = vbtable;

    std::vector<uint8_t> storage(sizeof(MapWin) + 32);
    auto *window = reinterpret_cast<MapWin *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;
    auto prepare_base = [&] {
        std::memcpy(object, &vbtable_pointer, sizeof(vbtable_pointer));
        const uint32_t zero = 0;
        std::memcpy(object + vbase + 0xA08, &zero, sizeof(zero));  // release ptr
    };

    // Owned pointer present: it is freed and the slot cleared, then the base
    // close runs at the vbtable offset.
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x20 + (i * 7));
    }
    prepare_base();
    void *owned = reinterpret_cast<void *>(0xDEADBEEF);
    std::memcpy(object + 4, &owned, sizeof(owned));
    g_win_closed = nullptr;
    g_buffer_closed = nullptr;
    g_mapwin_free_calls = 0;
    window->close();
    expect(g_mapwin_free_calls == 1);
    expect(g_mapwin_freed == owned);
    void *after = reinterpret_cast<void *>(0x1);
    std::memcpy(&after, object + 4, sizeof(after));
    expect(after == nullptr);
    expect(g_win_closed == object + vbase);
    expect(g_buffer_closed == object + vbase + 0x444);

    // No owned pointer: free is skipped, base close still runs.
    prepare_base();
    void *const null_owned = nullptr;
    std::memcpy(object + 4, &null_owned, sizeof(null_owned));
    g_win_closed = nullptr;
    g_mapwin_free_calls = 0;
    window->close();
    expect(g_mapwin_free_calls == 0);
    expect(g_win_closed == object + vbase);

    std::memcpy(object + 4, &owned, sizeof(owned));
    prepare_base();
    g_mapwin_free_calls = 0;
    map_win_close_redirect(window, nullptr);
    expect(g_mapwin_free_calls == 1);

    MapWinFree = saved_free;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_default;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(123, test_popup_close);
LEAF_CASE(124, test_guarded_delegates);
LEAF_CASE(125, test_virtual_base_closes);
LEAF_CASE(126, test_edit_group_text);
LEAF_CASE(127, test_dialogs_dispatch);
LEAF_CASE(128, test_fixed_argument_delegates);
LEAF_CASE(129, test_offset_delegates);
LEAF_CASE(130, test_win_sync_palette);
LEAF_CASE(131, test_self_contained_stores);
LEAF_CASE(132, test_base_pop_set_width);
LEAF_CASE(133, test_map_win_close);
}  // namespace
