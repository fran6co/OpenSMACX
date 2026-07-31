// Recovery leaf tests: forwarders.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "forwarders"

namespace {

void *g_struct_add_self = nullptr;

int g_struct_add_index = 0;

int g_struct_add_result = 0;

int g_struct_add_calls = 0;

int __thiscall observe_struct_add(void *self, int index) {
    g_struct_add_self = self; g_struct_add_index = index; ++g_struct_add_calls;
    return g_struct_add_result;
}

StringBox *g_fixup_self = nullptr;

int g_fixup_calls = 0;

void __thiscall observe_add_fixup(StringBox *self) {
    g_fixup_self = self; ++g_fixup_calls;
}

}  // namespace

void test_string_box_add() {
    auto *const saved_add = StringBoxStructAdd;
    auto *const saved_fixup = StringBoxAddFixup;
    StringBoxStructAdd = &observe_struct_add;
    StringBoxAddFixup = &observe_add_fixup;

    std::vector<uint8_t> storage(sizeof(StringBox) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *box = reinterpret_cast<StringBox *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;
    auto read_ptr = [&](size_t off) {
        void *v = nullptr;
        std::memcpy(&v, object + off, sizeof(v));
        return v;
    };
    auto read32 = [&](size_t off) {
        uint32_t v = 0;
        std::memcpy(&v, object + off, sizeof(v));
        return v;
    };

    char text[] = "entry";

    // Struct add reports success (nonzero): the three fields are staged, add
    // is called on the struct member at 0x2B70, and the fixup is skipped.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_struct_add_result = 1;
    g_struct_add_calls = 0;
    g_fixup_calls = 0;
    box->add(text, 7, 0x55);
    expect(g_struct_add_calls == 1);
    expect(g_struct_add_self == object + 0x2B70);
    expect(g_struct_add_index == 7);
    expect(g_fixup_calls == 0);
    expect(read_ptr(0x2B8C) == text);
    expect(read32(0x2B90) == 0x55);
    expect(read32(0x2B94) == 0);
    // Only the three staged fields moved.
    std::memcpy(expected.data() + 16 + 0x2B8C, object + 0x2B8C, 4);
    std::memcpy(expected.data() + 16 + 0x2B90, object + 0x2B90, 8);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Struct add reports failure (zero): the fixup runs, on the box itself.
    g_struct_add_result = 0;
    g_struct_add_calls = 0;
    g_fixup_calls = 0;
    box->add(text, -3, 0);
    expect(g_struct_add_calls == 1);
    expect(g_struct_add_index == -3);
    expect(g_fixup_calls == 1);
    expect(g_fixup_self == box);

    // Redirect: same, and the index passes through.
    g_struct_add_result = 1;
    g_fixup_calls = 0;
    string_box_add_redirect(box, nullptr, text, INT_MIN, 0);
    expect(g_struct_add_index == INT_MIN);
    expect(g_fixup_calls == 0);

    StringBoxStructAdd = saved_add;
    StringBoxAddFixup = saved_fixup;
}

namespace {

void *g_link_freed[24] = {};

int g_link_free_calls = 0;

void *observe_buffer_free(void *pointer) {
    if (g_link_free_calls < 24) g_link_freed[g_link_free_calls] = pointer;
    ++g_link_free_calls;
    return nullptr;
}

}  // namespace

void test_buffer_clear_links() {
    // clear_links reinitialises the spot list, zeroes the count at 0x4AC, and
    // frees the twenty owned link pointers at 0x4BC through the CRT boundary,
    // skipping the null ones. Spot::init is real recovered code, so its
    // allocation and its shutdown-free run for real here.
    auto *const saved_free = BufferFree;
    BufferFree = &observe_buffer_free;

    std::vector<uint8_t> storage(sizeof(Buffer) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *buffer = reinterpret_cast<Buffer *>(storage.data() + 16);
    uint8_t *const object = storage.data() + 16;

    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x30 + (i * 7));
    }
    // The embedded Spot at 0x4B0 must have a null spots_ pointer so its
    // shutdown does not free seeded garbage; Spot::init allocates a fresh one.
    const void *const null_ptr = nullptr;
    std::memcpy(object + 0x4B0, &null_ptr, sizeof(null_ptr));

    // Ten of the twenty links point at real allocations, ten are null.
    void *links[20] = {};
    for (int i = 0; i < 20; ++i) {
        links[i] = (i % 2 == 0) ? std::malloc(8) : nullptr;
        std::memcpy(object + 0x4BC + i * 4, &links[i], sizeof(void *));
    }

    mem_get_calls = 0;
    g_link_free_calls = 0;
    buffer->clear_links();

    // Spot::init ran: it allocated once (the shutdown free of a null pointer
    // is a no-op).
    expect(mem_get_calls == 1);
    // The count at 0x4AC is cleared.
    uint32_t count = 0xFFFFFFFF;
    std::memcpy(&count, object + 0x4AC, sizeof(count));
    expect(count == 0);
    // Exactly the ten non-null links were freed, in order, and each slot is
    // now null; the ten null slots were never freed and stay null.
    expect(g_link_free_calls == 10);
    for (int i = 0; i < 20; ++i) {
        void *slot = reinterpret_cast<void *>(0x1);
        std::memcpy(&slot, object + 0x4BC + i * 4, sizeof(slot));
        expect(slot == nullptr);
    }
    for (int i = 0; i < 10; ++i) {
        expect(g_link_freed[i] == links[i * 2]);
    }

    // Redirect: a fresh object frees its non-null links the same way.
    for (size_t i = 0; i < storage.size(); ++i) {
        storage[i] = static_cast<uint8_t>(0x11 + (i * 3));
    }
    std::memcpy(object + 0x4B0, &null_ptr, sizeof(null_ptr));
    void *one = std::malloc(8);
    for (int i = 0; i < 20; ++i) {
        const void *v = (i == 5) ? one : nullptr;
        std::memcpy(object + 0x4BC + i * 4, &v, sizeof(v));
    }
    g_link_free_calls = 0;
    buffer_clear_links_redirect(buffer, nullptr);
    expect(g_link_free_calls == 1);
    expect(g_link_freed[0] == one);

    BufferFree = saved_free;
}

namespace {

void *g_net_get_net = nullptr;

unsigned long g_net_get_first_out = 0;

unsigned long g_net_get_second_out = 0;

int g_net_get_result = 0;

int g_net_get_calls = 0;

int __thiscall observe_net_get(void *net, unsigned long *a, unsigned long *b) {
    g_net_get_net = net;
    *a = g_net_get_first_out;
    *b = g_net_get_second_out;
    ++g_net_get_calls;
    return g_net_get_result;
}

NetDaemon *g_pm_self = nullptr;

char *g_pm_message = nullptr;

unsigned long g_pm_a = 0;

int g_pm_b = 0;

int g_pm_calls = 0;

void __thiscall observe_process_message(NetDaemon *self, char *message,
                                        unsigned long a, int b) {
    g_pm_self = self; g_pm_message = message; g_pm_a = a; g_pm_b = b;
    ++g_pm_calls;
}

}  // namespace

void test_net_daemon_receive() {
    auto *const saved_get = NetDaemonNetGet;
    auto *const saved_pm = NetDaemonProcessMessage;
    void *const saved_net = NetDaemonNet;
    int fake_net = 0;
    NetDaemonNetGet = &observe_net_get;
    NetDaemonProcessMessage = &observe_process_message;
    NetDaemonNet = &fake_net;

    std::vector<uint8_t> storage(sizeof(NetDaemon) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *daemon = reinterpret_cast<NetDaemon *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    // No message: get returns zero, process_message is not called, receive
    // reports 0, and nothing on the object moves.
    g_net_get_result = 0;
    g_net_get_calls = 0;
    g_pm_calls = 0;
    expect(daemon->receive() == 0);
    expect(g_net_get_calls == 1);
    expect(g_net_get_net == &fake_net);   // reads the global Net, not `this`
    expect(g_pm_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A message: get returns nonzero and fills the two out-parameters, which
    // flow to process_message as (result, second, first) - the order the asm
    // pushes them in, which a naive reading would swap.
    g_net_get_result = 0x00ABCDEF;
    g_net_get_first_out = 0x11111111;
    g_net_get_second_out = 0x22222222;
    g_net_get_calls = 0;
    g_pm_calls = 0;
    expect(daemon->receive() == 1);
    expect(g_net_get_calls == 1);
    expect(g_pm_calls == 1);
    expect(g_pm_self == daemon);
    expect(g_pm_message == reinterpret_cast<char *>(0x00ABCDEF));
    expect(g_pm_a == 0x22222222);     // second out-parameter
    expect(g_pm_b == 0x11111111);     // first out-parameter
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    g_pm_calls = 0;
    expect(net_daemon_receive_redirect(daemon, nullptr) == 1);
    expect(g_pm_calls == 1);

    NetDaemonNetGet = saved_get;
    NetDaemonProcessMessage = saved_pm;
    NetDaemonNet = saved_net;
}

void test_win_scroll_forwarders() {
    // The four forwarders send a position or range to the vertical Scroll at
    // 0x43C or the horizontal one at 0x440, guarding on null. Both Scroll
    // methods are real recovered code that dispatches through the redraw
    // vtable, so each member gets a full Scroll with that probe installed, and
    // the test checks the call landed on the right one - swapping vert and
    // horz would send it to the other scroll.
    Win **const saved_current_win = ScrollCurrentWin;
    Win *published = nullptr;
    ScrollCurrentWin = &published;

    alignas(Scroll) uint8_t vert[sizeof(Scroll) + 32] = {};
    alignas(Scroll) uint8_t horz[sizeof(Scroll) + 32] = {};
    uintptr_t vert_vtable[63];
    uintptr_t horz_vtable[63];
    install_scroll_redraw_probe(vert, vert_vtable);
    install_scroll_redraw_probe(horz, horz_vtable);
    auto *vert_scroll = reinterpret_cast<Scroll *>(vert + 16);
    auto *horz_scroll = reinterpret_cast<Scroll *>(horz + 16);
    // A parent so set_pos does not return early.
    Win *parent = reinterpret_cast<Win *>(0x33334444U);
    write_at(vert, 16 + 0xC4, parent);
    write_at(horz, 16 + 0xC4, parent);

    alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *window = reinterpret_cast<Win *>(win_storage + 16);
    auto point_scrolls = [&](Scroll *v, Scroll *h) {
        write_at(win_storage, 16 + 0x43C, v);
        write_at(win_storage, 16 + 0x440, h);
    };
    auto vert_range_min = [&] {
        uint32_t m = 0; std::memcpy(&m, vert + 16 + 0xA20, 4); return m;
    };
    auto horz_range_min = [&] {
        uint32_t m = 0; std::memcpy(&m, horz + 16 + 0xA20, 4); return m;
    };

    // set_vert_range hits the vertical scroll only.
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    point_scrolls(vert_scroll, horz_scroll);
    reset_scroll_redraw_probe();
    { uint32_t z = 0; std::memcpy(horz + 16 + 0xA20, &z, 4); }
    window->set_vert_range(0x1234, 0x5678);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == vert_scroll);
    expect(vert_range_min() == 0x1234);
    expect(horz_range_min() == 0);

    // set_horz_range hits the horizontal scroll only.
    reset_scroll_redraw_probe();
    { uint32_t z = 0; std::memcpy(vert + 16 + 0xA20, &z, 4); }
    window->set_horz_range(0x4321, 0x8765);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == horz_scroll);
    expect(horz_range_min() == 0x4321);
    expect(vert_range_min() == 0);

    // set_vert_pos and set_horz_pos each dispatch to their own scroll.
    reset_scroll_redraw_probe();
    published = nullptr;
    window->set_vert_pos(5);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == vert_scroll);
    reset_scroll_redraw_probe();
    window->set_horz_pos(9);
    expect(scroll_redraw_calls == 1);
    expect(scroll_redraw_self == horz_scroll);

    // Null scrollbars: every forwarder is a no-op, no dispatch, no crash.
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    point_scrolls(nullptr, nullptr);
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    reset_scroll_redraw_probe();
    window->set_vert_pos(1);
    window->set_horz_pos(1);
    window->set_vert_range(1, 2);
    window->set_horz_range(1, 2);
    expect(scroll_redraw_calls == 0);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    // Redirects reach the same scrolls.
    point_scrolls(vert_scroll, horz_scroll);
    reset_scroll_redraw_probe();
    win_set_vert_range_redirect(window, nullptr, 3, 4);
    expect(scroll_redraw_self == vert_scroll);
    reset_scroll_redraw_probe();
    win_set_horz_pos_redirect(window, nullptr, 2);
    expect(scroll_redraw_self == horz_scroll);

    ScrollCurrentWin = saved_current_win;
}

namespace {

void *g_pref_win_target = nullptr;

int g_pref_win_page = -1;

int g_pref_win_calls = 0;

void __thiscall observe_pref_win_display(void *pref_win, int page) {
    g_pref_win_target = pref_win; g_pref_win_page = page; ++g_pref_win_calls;
}

}  // namespace

void test_console_preference_openers() {
    // Five openers, each opening the one PrefWin to a different page. The page
    // number is the only thing that distinguishes them, so every one is
    // checked - a copy-paste that left two openers on the same page would pass
    // a test that only confirmed a call happened.
    auto *const saved_display = ConsolePrefWinDisplay;
    void *const saved_pref = ConsolePrefWin;
    int fake_pref_win = 0;
    ConsolePrefWinDisplay = &observe_pref_win_display;
    ConsolePrefWin = &fake_pref_win;

    std::vector<uint8_t> storage(sizeof(Console) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *console = reinterpret_cast<Console *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    struct Opener {
        void (Console::*method)();
        void (__fastcall *redirect)(Console *, void *);
        int page;
    };
    const Opener openers[] = {
        {&Console::set_preferences, &console_set_preferences_redirect, 0},
        {&Console::set_auto_preferences, &console_set_auto_preferences_redirect, 3},
        {&Console::set_base_preferences, &console_set_base_preferences_redirect, 2},
        {&Console::set_audiovisual, &console_set_audiovisual_redirect, 4},
        {&Console::set_map_display, &console_set_map_display_redirect, 5},
        {&Console::set_adv_preferences, &console_set_adv_preferences_redirect, 1},
    };

    for (const Opener &opener : openers) {
        g_pref_win_calls = 0;
        g_pref_win_page = -1;
        g_pref_win_target = nullptr;
        (console->*(opener.method))();
        expect(g_pref_win_calls == 1);
        // The shared PrefWin, not the Console it is called through.
        expect(g_pref_win_target == &fake_pref_win);
        expect(g_pref_win_target != reinterpret_cast<void *>(console));
        expect(g_pref_win_page == opener.page);

        g_pref_win_calls = 0;
        opener.redirect(console, nullptr);
        expect(g_pref_win_calls == 1);
        expect(g_pref_win_page == opener.page);
    }
    // None of them touched the Console object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    ConsolePrefWinDisplay = saved_display;
    ConsolePrefWin = saved_pref;
}

namespace {

BaseWin *g_iface_base = nullptr;

int g_iface_a1 = 0, g_iface_a2 = 0, g_iface_button = -1, g_iface_double = -1;

int g_iface_calls = 0;

void __thiscall observe_iface_click(BaseWin *self, int a1, int a2, int button,
                                    int is_double) {
    g_iface_base = self; g_iface_a1 = a1; g_iface_a2 = a2;
    g_iface_button = button; g_iface_double = is_double;
    ++g_iface_calls;
}

}  // namespace

void test_base_win_iface_clicks() {
    // Four interface-click handlers, reached through the subobject BaseWin
    // embeds at 0xA14, so their `this` points there and must be adjusted back
    // by 0xA14 before dispatching. Each carries a distinct (button, double)
    // pair, and getting the adjustment or a flag wrong is the whole risk here.
    auto *const saved = BaseWinIfaceClick;
    BaseWinIfaceClick = &observe_iface_click;

    // A BaseWin, plus room past 0xA14 so the interface pointer is in-bounds.
    std::vector<uint8_t> storage(sizeof(BaseWin) + 0xA20);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const real_base = storage.data() + 16;
    // What the interface vtable would pass as `this`: BaseWin + 0xA14.
    auto *iface = reinterpret_cast<BaseWin *>(real_base + 0xA14);

    struct Handler {
        void (BaseWin::*method)(int, int);
        void (__fastcall *redirect)(BaseWin *, void *, int, int);
        int button;
        int is_double;
    };
    const Handler handlers[] = {
        {&BaseWin::on_iface_left_click,
         &base_win_on_iface_left_click_redirect, 0, 0},
        {&BaseWin::on_iface_right_click,
         &base_win_on_iface_right_click_redirect, 1, 0},
        {&BaseWin::on_iface_left_double_click,
         &base_win_on_iface_left_double_click_redirect, 0, 1},
        {&BaseWin::on_iface_right_double_click,
         &base_win_on_iface_right_double_click_redirect, 1, 1},
    };

    for (const Handler &handler : handlers) {
        seed_storage(storage.data(), expected.data(), storage.size());
        std::memcpy(expected.data(), storage.data(), storage.size());
        g_iface_calls = 0;
        (iface->*(handler.method))(0x1111, 0x2222);
        expect(g_iface_calls == 1);
        // Adjusted back to the BaseWin, not left at the interface subobject.
        expect(reinterpret_cast<uint8_t *>(g_iface_base) == real_base);
        expect(g_iface_a1 == 0x1111);
        expect(g_iface_a2 == 0x2222);
        expect(g_iface_button == handler.button);
        expect(g_iface_double == handler.is_double);
        // Nothing on the object moves; the handler only dispatches.
        expect_storage_bytes(storage.data(), expected.data(), storage.size());

        g_iface_calls = 0;
        handler.redirect(iface, nullptr, -1, -2);
        expect(g_iface_calls == 1);
        expect(reinterpret_cast<uint8_t *>(g_iface_base) == real_base);
        expect(g_iface_a1 == -1 && g_iface_a2 == -2);
        expect(g_iface_button == handler.button);
        expect(g_iface_double == handler.is_double);
    }

    BaseWinIfaceClick = saved;
}

namespace {

BaseWin *g_draw_supported_base = nullptr;

int g_draw_supported_arg = -1;

int g_draw_supported_calls = 0;

void __thiscall observe_draw_supported(BaseWin *self, int a1) {
    g_draw_supported_base = self; g_draw_supported_arg = a1;
    ++g_draw_supported_calls;
}

}  // namespace

void test_base_win_iface_scrolled() {
    // Only scroll kind 2 does anything: it stores the position at 0x40100
    // relative to the interface subobject, then redraws on the BaseWin reached
    // by the same 0xA14 adjustment. Any other kind is a no-op.
    auto *const saved = BaseWinDrawSupported;
    BaseWinDrawSupported = &observe_draw_supported;

    // Room for the interface subobject at 0xA14 and the store at +0x40100.
    std::vector<uint8_t> storage(sizeof(BaseWin) + 0xA14 + 0x40108);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const real_base = storage.data() + 16;
    auto *iface = reinterpret_cast<BaseWin *>(real_base + 0xA14);
    auto stored = [&] {
        int v = 0;
        std::memcpy(&v, real_base + 0xA14 + 0x40100, sizeof(v));
        return v;
    };

    // Kind 2: stores the position and redraws on the adjusted base.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_draw_supported_calls = 0;
    iface->on_iface_scrolled(2, 0x1234ABCD);
    expect(g_draw_supported_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_draw_supported_base) == real_base);
    expect(g_draw_supported_arg == 1);
    expect(stored() == 0x1234ABCD);
    std::memcpy(expected.data() + 16 + 0xA14 + 0x40100,
                storage.data() + 16 + 0xA14 + 0x40100, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Any other kind: nothing happens, nothing is stored, no redraw.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    for (int kind : {0, 1, 3, -2, INT_MAX}) {
        g_draw_supported_calls = 0;
        iface->on_iface_scrolled(kind, 0x55555555);
        expect(g_draw_supported_calls == 0);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    g_draw_supported_calls = 0;
    base_win_on_iface_scrolled_redirect(iface, nullptr, 2, -1);
    expect(g_draw_supported_calls == 1);
    expect(stored() == -1);

    BaseWinDrawSupported = saved;
}

namespace {

void *g_dfwd_self = nullptr;

int g_dfwd_a1 = 0, g_dfwd_a2 = 0, g_dfwd_calls = 0;

const char *g_dfwd_which = nullptr;

void __thiscall observe_fwd2(void *self, int a1, int a2) {
    g_dfwd_self = self; g_dfwd_a1 = a1; g_dfwd_a2 = a2; ++g_dfwd_calls;
}

void __thiscall observe_fwd1(void *self, int a1) {
    g_dfwd_self = self; g_dfwd_a1 = a1; ++g_dfwd_calls;
}

}  // namespace

void test_dialogs_forwarders() {
    // Eight event forwarders. Each reads a discriminator 8 bytes before the
    // interface `this`, and only when it matches (8 for the sprite-box family,
    // 2 for the list-box one) adjusts `this` back - 0x8C or 0x140 - and calls
    // the embedded widget's handler. Every seam, both discriminator values,
    // and both adjustments are checked; a wrong adjustment sends the call to
    // the wrong subobject, and a wrong discriminator makes a live event a
    // no-op or vice versa.
    auto *const s_rd = DialogsSpriteBoxOnRightDown;
    auto *const s_rdc = DialogsSpriteBoxOnRightDoubleClick;
    auto *const s_lu = DialogsSpriteBoxOnLeftUp;
    auto *const s_ru = DialogsSpriteBoxOnRightUp;
    auto *const s_rc = DialogsSpriteBoxOnRightClick;
    auto *const l_sc = DialogsListBoxOnScrolling;
    auto *const l_mw = DialogsListBoxOnMousewheel;
    DialogsSpriteBoxOnRightDown = &observe_fwd2;
    DialogsSpriteBoxOnRightDoubleClick = &observe_fwd2;
    DialogsSpriteBoxOnLeftUp = &observe_fwd2;
    DialogsSpriteBoxOnRightUp = &observe_fwd2;
    DialogsSpriteBoxOnRightClick = &observe_fwd2;
    DialogsListBoxOnScrolling = &observe_fwd2;
    DialogsListBoxOnMousewheel = &observe_fwd1;

    std::vector<uint8_t> storage(sizeof(Dialogs) + 0x400);
    // Put the interface `this` 0x200 into the buffer so both the -8
    // discriminator and the negative adjustments stay in bounds.
    uint8_t *const iface = storage.data() + 16 + 0x200;
    auto *dialogs = reinterpret_cast<Dialogs *>(iface);
    auto set_disc = [&](int value) {
        std::memcpy(iface - 8, &value, sizeof(value));
    };

    struct Case2 {
        void (Dialogs::*method)(int, int);
        int disc;
        size_t adjust;
    };
    const Case2 sprite[] = {
        {&Dialogs::on_right_down, 8, 0x8C},
        {&Dialogs::on_right_double_click, 8, 0x8C},
        {&Dialogs::on_left_up, 8, 0x8C},
        {&Dialogs::on_right_up, 8, 0x8C},
        {&Dialogs::on_right_click, 8, 0x8C},
        {&Dialogs::on_scrolled, 2, 0x140},
        {&Dialogs::on_scrolling, 2, 0x140},
    };
    for (const Case2 &c : sprite) {
        // Matching discriminator: forwards to (this - adjust) with both args.
        set_disc(c.disc);
        g_dfwd_calls = 0;
        (dialogs->*(c.method))(0x111, 0x222);
        expect(g_dfwd_calls == 1);
        expect(g_dfwd_self == iface - c.adjust);
        expect(g_dfwd_a1 == 0x111 && g_dfwd_a2 == 0x222);
        // Wrong discriminator: no forward.
        set_disc(c.disc + 1);
        g_dfwd_calls = 0;
        (dialogs->*(c.method))(0x111, 0x222);
        expect(g_dfwd_calls == 0);
    }

    // on_mousewheel takes one argument and uses the list-box discriminator.
    set_disc(2);
    g_dfwd_calls = 0;
    dialogs->on_mousewheel(0x333);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x140);
    expect(g_dfwd_a1 == 0x333);
    set_disc(0);
    g_dfwd_calls = 0;
    dialogs->on_mousewheel(0x333);
    expect(g_dfwd_calls == 0);

    // A redirect from each family, confirming the seam wiring.
    set_disc(8);
    g_dfwd_calls = 0;
    dialogs_on_right_down_redirect(dialogs, nullptr, 1, 2);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x8C);
    set_disc(2);
    g_dfwd_calls = 0;
    dialogs_on_mousewheel_redirect(dialogs, nullptr, 7);
    expect(g_dfwd_calls == 1);
    expect(g_dfwd_self == iface - 0x140);

    DialogsSpriteBoxOnRightDown = s_rd;
    DialogsSpriteBoxOnRightDoubleClick = s_rdc;
    DialogsSpriteBoxOnLeftUp = s_lu;
    DialogsSpriteBoxOnRightUp = s_ru;
    DialogsSpriteBoxOnRightClick = s_rc;
    DialogsListBoxOnScrolling = l_sc;
    DialogsListBoxOnMousewheel = l_mw;
}

namespace {

BaseWin *g_bwclick_self = nullptr;

int g_bwclick_a1 = 0, g_bwclick_a2 = 0, g_bwclick_button = -1, g_bwclick_double = -1;

int g_bwclick_calls = 0;

void __thiscall observe_bw_click(BaseWin *self, int a1, int a2,
                                       int button, int is_double) {
    g_bwclick_self = self; g_bwclick_a1 = a1; g_bwclick_a2 = a2;
    g_bwclick_button = button; g_bwclick_double = is_double;
    ++g_bwclick_calls;
}

}  // namespace

void test_base_win_clicks() {
    // Three primary click handlers, forwarding to the shared click() with no
    // this-adjustment - `this` is the BaseWin itself. Each carries a distinct
    // (button, double) pair, and unlike the iface family the callee gets the
    // object unchanged, which is what the test confirms alongside the flags.
    auto *const saved = BaseWinClick;
    BaseWinClick = &observe_bw_click;

    std::vector<uint8_t> storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *window = reinterpret_cast<BaseWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    struct Handler {
        void (BaseWin::*method)(int, int);
        void (__fastcall *redirect)(BaseWin *, void *, int, int);
        int button;
        int is_double;
    };
    const Handler handlers[] = {
        {&BaseWin::on_left_click, &base_win_on_left_click_redirect, 0, 0},
        {&BaseWin::on_right_click, &base_win_on_right_click_redirect, 1, 0},
        {&BaseWin::on_left_double_click,
         &base_win_on_left_double_click_redirect, 0, 1},
    };

    for (const Handler &handler : handlers) {
        g_bwclick_calls = 0;
        (window->*(handler.method))(0x1111, 0x2222);
        expect(g_bwclick_calls == 1);
        expect(g_bwclick_self == window);        // the object itself, no adjust
        expect(g_bwclick_a1 == 0x1111 && g_bwclick_a2 == 0x2222);
        expect(g_bwclick_button == handler.button);
        expect(g_bwclick_double == handler.is_double);

        g_bwclick_calls = 0;
        handler.redirect(window, nullptr, -1, -2);
        expect(g_bwclick_calls == 1);
        expect(g_bwclick_self == window);
        expect(g_bwclick_a1 == -1 && g_bwclick_a2 == -2);
        expect(g_bwclick_button == handler.button);
        expect(g_bwclick_double == handler.is_double);
    }
    // The forwarders only dispatch; the object is untouched.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    BaseWinClick = saved;
}

namespace {

WorldWin *g_ww_self = nullptr;

int g_ww_button = -1, g_ww_double = -1, g_ww_calls = 0;

void __thiscall observe_world_win_click(WorldWin *self, int a1, int a2,
                                        int button, int is_double) {
    g_ww_self = self; g_ww_button = button; g_ww_double = is_double;
    ++g_ww_calls; (void)a1; (void)a2;
}

MapWin *g_mw_self = nullptr;

int g_mw_a1 = 0, g_mw_a2 = 0, g_mw_button = -1, g_mw_calls = 0;

void __thiscall observe_map_win_click(MapWin *self, int a1, int a2, int button) {
    g_mw_self = self; g_mw_a1 = a1; g_mw_a2 = a2; g_mw_button = button;
    ++g_mw_calls;
}

}  // namespace

void test_window_click_forwarders() {
    // WorldWin: a plain 4-arg click, no adjustment, no guard.
    auto *const saved_ww = WorldWinClick;
    WorldWinClick = &observe_world_win_click;
    std::vector<uint8_t> ww(sizeof(WorldWin) + 32);
    std::vector<uint8_t> ww_want(ww.size());
    auto *world = reinterpret_cast<WorldWin *>(ww.data() + 16);
    seed_storage(ww.data(), ww_want.data(), ww.size());
    std::memcpy(ww_want.data(), ww.data(), ww.size());
    g_ww_calls = 0;
    world->on_left_click(1, 2);
    expect(g_ww_calls == 1 && g_ww_self == world && g_ww_button == 0 && g_ww_double == 0);
    world->on_right_click(1, 2);
    expect(g_ww_button == 1 && g_ww_double == 0 && g_ww_self == world);
    world_win_on_left_click_redirect(world, nullptr, 3, 4);
    expect(g_ww_button == 0);
    expect_storage_bytes(ww.data(), ww_want.data(), ww.size());
    WorldWinClick = saved_ww;

    // MapWin: reached through the GraphicWin virtual base, so `this` points at
    // 0x21A6C into the object and is adjusted back; gated on an input-enable
    // flag. Both the adjustment and the guard are pinned.
    auto *const saved_mw = MapWinClick;
    int32_t *const saved_flag = MapWinInputEnabled;
    int32_t enabled = 1;
    MapWinClick = &observe_map_win_click;
    MapWinInputEnabled = &enabled;

    std::vector<uint8_t> mw(sizeof(MapWin) + 32);
    auto *real_map = reinterpret_cast<uint8_t *>(mw.data() + 16);
    // What the virtual-base vtable passes as `this`: MapWin + 0x21A6C.
    auto *vbase_this = reinterpret_cast<MapWin *>(real_map + 0x21A6C);

    enabled = 1;
    g_mw_calls = 0;
    vbase_this->on_left_click(0x55, 0x66);
    expect(g_mw_calls == 1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);  // adjusted back
    expect(g_mw_a1 == 0x55 && g_mw_a2 == 0x66 && g_mw_button == 0);
    vbase_this->on_right_click(-1, -2);
    expect(g_mw_button == 1 && g_mw_a1 == -1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);

    // Disabled: the guard blocks both, no dispatch.
    enabled = 0;
    g_mw_calls = 0;
    vbase_this->on_left_click(1, 1);
    vbase_this->on_right_click(1, 1);
    expect(g_mw_calls == 0);

    enabled = 1;
    g_mw_calls = 0;
    map_win_on_right_click_redirect(vbase_this, nullptr, 9, 8);
    expect(g_mw_calls == 1 && g_mw_button == 1);
    expect(reinterpret_cast<uint8_t *>(g_mw_self) == real_map);

    MapWinClick = saved_mw;
    MapWinInputEnabled = saved_flag;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(135, test_string_box_add);
LEAF_CASE(136, test_buffer_clear_links);
LEAF_CASE(137, test_net_daemon_receive);
LEAF_CASE(138, test_win_scroll_forwarders);
LEAF_CASE(139, test_console_preference_openers);
LEAF_CASE(140, test_base_win_iface_clicks);
LEAF_CASE(141, test_base_win_iface_scrolled);
LEAF_CASE(142, test_dialogs_forwarders);
LEAF_CASE(143, test_base_win_clicks);
LEAF_CASE(144, test_window_click_forwarders);
}  // namespace
