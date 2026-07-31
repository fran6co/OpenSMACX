// Recovery leaf tests: stubs_and_locks.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "stubs_and_locks"

namespace {

// draw_tile's and draw_tiles' only dependency is MapWin::draw_radius
// (0x0046A2A0), which is still an original body reaching gen_radius,
// Texture::draw_trans, compute_clip, GraphicWin::soft_update and do_all_draws.
// The seam replaces it wholesale with this recorder, so the whole of the
// observable behaviour - which slots dispatch, in what order, with what
// arguments - is captured here.
struct DrawTileCall {
    const MapWin *self;
    int x;
    int y;
    int radius;               // the literal at 0x0046AF6D / 0x0046B16D
    int draw_type;
    uint32_t active_at_call;  // [self + 0x1DD74] observed inside the probe
};

DrawTileCall draw_tile_calls[16];

int draw_tile_call_count = 0;

void __thiscall observe_draw_radius(MapWin *self, int x, int y, int radius,
                                    int draw_type) {
    const int index = draw_tile_call_count++;
    if (index >= static_cast<int>(ARRAYSIZE(draw_tile_calls))) {
        return;
    }
    DrawTileCall &call = draw_tile_calls[index];
    call.self = self;
    call.x = x;
    call.y = y;
    call.radius = radius;
    call.draw_type = draw_type;
    std::memcpy(&call.active_at_call,
                reinterpret_cast<const uint8_t *>(self) + MapWinActiveOffset,
                sizeof(call.active_at_call));
}

struct DrawTileShape {
    const char *name;
    int occupant[MapWinTableSlots];       // window index per slot, -1 = empty
    uint32_t active[MapWinTableSlots];    // 0x1DD74 dword, per window index
    int x;
    int y;
    int draw_type;
    int expected[MapWinTableSlots + 1];   // slots expected to draw, -1 ends
};

// Both originals are run through every shape. `radius` is the one byte that
// separates them, so passing it in is what makes a body that copied the wrong
// discriminator fail.
void run_draw_tile_shape(const DrawTileShape &shape,
                         void(__cdecl *broadcast)(int, int, int),
                         int expected_radius) {
    // One arena backs all eight stand-in windows. The window pointers are four
    // bytes apart, so their 0x1DD74 activity dwords land in one contiguous,
    // individually addressable band at the far end - a real 0x22480-byte
    // MapWin each would be 1.1MB of fixture for two fields nobody reads. The
    // windows deliberately overlap: the body only ever reads +0x1DD74, and
    // distinct pointers with distinct gates are exactly what must be pinned.
    const size_t flag_band = 16 + MapWinActiveOffset;
    std::vector<uint8_t> arena(flag_band + MapWinTableSlots * 4 + 16);
    std::vector<uint8_t> arena_want(arena.size());
    seed_storage(arena.data(), arena_want.data(), arena.size());

    // The table gets its own storage with 16 seeded bytes on each side. The
    // trailing canary is deliberately NOT null: a loop that runs past slot 7
    // reads a nonzero garbage pointer and faults on its 0x1DD74 gate, which is
    // how an over-long bound gets killed rather than silently tolerated.
    std::vector<uint8_t> table(16 + MapWinTableSlots * 4 + 16);
    std::vector<uint8_t> table_want(table.size());
    seed_storage(table.data(), table_want.data(), table.size());

    MapWin *windows[MapWinTableSlots];
    for (size_t w = 0; w < MapWinTableSlots; ++w) {
        windows[w] = reinterpret_cast<MapWin *>(arena.data() + 16 + w * 4);
        std::memcpy(arena.data() + flag_band + w * 4, &shape.active[w],
                    sizeof(shape.active[w]));
    }
    MapWin **const slots = reinterpret_cast<MapWin **>(table.data() + 16);
    for (size_t s = 0; s < MapWinTableSlots; ++s) {
        slots[s] = shape.occupant[s] < 0
            ? nullptr
            : windows[shape.occupant[s]];
    }

    // The reference image. Neither body writes anywhere - they contain no store
    // instruction at all - so the byte-exact expectation is the input itself,
    // snapshotted once the shape is installed. Both the window arena and the
    // table, canaries included, must come back untouched.
    std::memcpy(arena_want.data(), arena.data(), arena.size());
    std::memcpy(table_want.data(), table.data(), table.size());

    MapWinTable = slots;
    draw_tile_call_count = 0;
    std::memset(draw_tile_calls, 0, sizeof(draw_tile_calls));

    broadcast(shape.x, shape.y, shape.draw_type);

    expect_storage_bytes(arena.data(), arena_want.data(), arena.size());
    expect_storage_bytes(table.data(), table_want.data(), table.size());
    // The walk is over the table, not over the global: the body must not
    // repoint it.
    expect(MapWinTable == slots);

    int expected_count = 0;
    while (expected_count < static_cast<int>(MapWinTableSlots)
           && shape.expected[expected_count] >= 0) {
        ++expected_count;
    }
    expect(draw_tile_call_count == expected_count);

    for (int index = 0; index < expected_count; ++index) {
        if (index >= draw_tile_call_count) {
            break;
        }
        const int slot = shape.expected[index];
        const DrawTileCall &call = draw_tile_calls[index];
        // Call order and target: slot order, ascending, with the exact slot
        // value as `this` (0x0046AF51 loads it, 0x0046AF71 dispatches on it).
        expect(call.self == windows[shape.occupant[slot]]);
        // Arguments forwarded verbatim, in the order the pushes at
        // 0x0046AF6C..0x0046AF70 build the frame.
        expect(call.x == shape.x);
        expect(call.y == shape.y);
        expect(call.draw_type == shape.draw_type);
        // The hardcoded discriminator: `push 0` at 0x0046AF6D for draw_tile,
        // `push 1` at 0x0046B16D for draw_tiles. This single assertion is what
        // separates the two otherwise byte-identical bodies.
        expect(call.radius == expected_radius);
        // Nothing was written before the dispatch: the gate the loop had just
        // read still holds the value the fixture seeded.
        expect(call.active_at_call == shape.active[shape.occupant[slot]]);
    }
}

}  // namespace

void test_draw_tile_broadcast() {
    // Both constants live in src/mapwin.h, where tools/mutate_and_verify.py
    // cannot reach them (it only derives mutants from literals inside the
    // annotated function bodies), so pin them here at compile time and cover
    // them behaviourally in the shapes below.
    static_assert(MapWinTableSlots == 8,
                  "draw_tile walks 0x007D3C3C..0x007D3C5C exclusive, 8 slots");
    static_assert(MapWinActiveOffset == 0x1DD74,
                  "draw_tile gates on the MapWin dword at +0x1DD74");

    func_map_win_draw_radius *const saved_draw_radius = MapWinOriginalDrawRadius;
    MapWin **const saved_table = MapWinTable;

    MapWinOriginalDrawRadius = &observe_draw_radius;

    static const DrawTileShape shapes[] = {
        // 1. Full table, gates strictly alternating. Slot 0 draws even though
        //    its gate is clear (the `cmp esi, 0x7d3c3c` exemption at
        //    0x0046AF57); every other slot obeys its gate, and nonzero means
        //    nonzero - 0x80000000 and 0xFFFFFFFF pass just as 1 does, because
        //    the original tests with `test eax,eax`, not a comparison. The
        //    alternation is the point: shifting the gate offset by one dword
        //    inverts the entire drawn set.
        {
            "draw_tile full-table-alternating-gates",
            { 0, 1, 2, 3, 4, 5, 6, 7 },
            { 0u, 1u, 0u, 0x80000000u, 0u, 1u, 0u, 0xFFFFFFFFu },
            0x12345678, -1, 3,
            { 0, 1, 3, 5, 7, -1 },
        },
        // 2. Holes, and an EMPTY slot 0. The null test at 0x0046AF53 precedes
        //    the slot-0 exemption, so the primary slot draws nothing here, and
        //    the walk still continues past it. Signed extremes in the
        //    coordinates and a negative draw type.
        {
            "draw_tile holes-and-null-primary",
            { -1, -1, 2, 3, -1, 5, 6, -1 },
            { 1u, 1u, 0u, 7u, 1u, 0u, 0xFFFFFFFFu, 1u },
            INT_MIN, INT_MAX, -5,
            { 3, 6, -1 },
        },
        // 3. The exemption in isolation: only slot 0 is occupied and its gate
        //    is clear, so a correct body makes exactly one call and a body
        //    that lost the exemption makes none.
        {
            "draw_tile primary-only-gate-clear",
            { 0, -1, -1, -1, -1, -1, -1, -1 },
            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
            0, 0, 0,
            { 0, -1 },
        },
        // 4. The SAME window pointer in slot 0 and slot 4, both reading the
        //    one gate, which is clear. Slot 0 draws, slot 4 does not: the
        //    exemption is keyed on the slot index, not on pointer identity or
        //    on "the primary window object". A body that compared the window
        //    against MapWinTable[0] instead of comparing the cursor against
        //    the table base would draw twice here and pass every other shape.
        {
            "draw_tile primary-aliased-into-slot-4",
            { 0, -1, -1, -1, 0, -1, -1, -1 },
            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u },
            7, -7, 1,
            { 0, -1 },
        },
        // 5. Last slot only. Shape 1 also draws slot 7, but here it is the
        //    only call, so a bound short of eight drops the call count to zero
        //    rather than merely shortening a list.
        {
            "draw_tile last-slot-only",
            { -1, -1, -1, -1, -1, -1, -1, 7 },
            { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u },
            -3, 4, 2,
            { 7, -1 },
        },
        // 6. Empty table: eight null slots, every gate set. No dispatch at
        //    all, and still not one byte written.
        {
            "draw_tile empty-table",
            { -1, -1, -1, -1, -1, -1, -1, -1 },
            { 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u },
            1, 2, 3,
            { -1 },
        },
    };

    for (size_t index = 0; index < ARRAYSIZE(shapes); ++index) {
        run_draw_tile_shape(shapes[index], &draw_tile, 0);
        run_draw_tile_shape(shapes[index], &draw_tiles, 1);
    }

    MapWinOriginalDrawRadius = saved_draw_radius;
    MapWinTable = saved_table;
    expect(MapWinOriginalDrawRadius == saved_draw_radius);
    expect(MapWinTable == saved_table);
    draw_tile_call_count = 0;
    std::memset(draw_tile_calls, 0, sizeof(draw_tile_calls));
}

namespace {

BasePop *g_exec_self = nullptr;

int g_exec_flag = -1;

int (__cdecl *g_exec_cb)() = nullptr;

int g_exec_result = 0, g_exec_calls = 0;

int __thiscall observe_base_pop_exec(BasePop *self, int flag, int (__cdecl *cb)()) {
    g_exec_self = self; g_exec_flag = flag; g_exec_cb = cb; ++g_exec_calls;
    return g_exec_result;
}

int __cdecl exec_probe_cb() { return 0; }

struct StartArgs {
    Popup *self; char *a; const char *b; int c; char *d; int e; void *f;
    int calls;
} g_start2 = {};

void __thiscall observe_start_full(Popup *self, char *a, const char *b, int c,
                                   char *d, int e, void *f) {
    g_start2 = {self, a, b, c, d, e, f, g_start2.calls + 1};
}

}  // namespace

void test_popup_exec_and_start_overloads() {
    // BasePop::exec: two overloads forwarding to the two-argument exec with
    // flag 0 and either no callback or the given one. The callback must pass
    // through, and the result must come back.
    auto *const saved_exec = BasePopExec;
    BasePopExec = &observe_base_pop_exec;
    std::vector<uint8_t> bp(sizeof(BasePop) + 32);
    auto *popup = reinterpret_cast<BasePop *>(bp.data() + 16);

    g_exec_result = 0x1234;
    g_exec_calls = 0;
    expect(popup->exec() == 0x1234);
    expect(g_exec_calls == 1 && g_exec_self == popup);
    expect(g_exec_flag == 0 && g_exec_cb == nullptr);

    g_exec_result = 0x5678;
    expect(popup->exec(&exec_probe_cb) == 0x5678);
    expect(g_exec_flag == 0 && g_exec_cb == &exec_probe_cb);
    expect(base_pop_exec_void_redirect(popup, nullptr) == 0x5678);
    expect(g_exec_cb == nullptr);
    expect(base_pop_exec_callback_redirect(popup, nullptr, &exec_probe_cb) == 0x5678);
    expect(g_exec_cb == &exec_probe_cb);
    BasePopExec = saved_exec;

    // Popup::start: two short forms filling the full start's defaults. The
    // caption comes from the shared buffer, c is -1, and the value lands in
    // slot e - the label-and-value form differs from the label-only form only
    // in that one argument, which the test pins.
    auto *const saved_start = PopupOriginalStartFull;
    char *const saved_caption = PopupStartCaption;
    char caption[4] = {};
    PopupOriginalStartFull = &observe_start_full;
    PopupStartCaption = caption;
    std::vector<uint8_t> pu(sizeof(Popup) + 32);
    auto *pstart = reinterpret_cast<Popup *>(pu.data() + 16);
    const char label[] = "hi";

    g_start2.calls = 0;
    pstart->start(label);
    expect(g_start2.calls == 1 && g_start2.self == pstart);
    expect(g_start2.a == caption && g_start2.b == label);
    expect(g_start2.c == -1 && g_start2.d == nullptr && g_start2.e == 0 &&
           g_start2.f == nullptr);

    pstart->start(label, 0x77);
    expect(g_start2.b == label && g_start2.e == 0x77);   // value in slot e
    expect(g_start2.c == -1 && g_start2.d == nullptr && g_start2.f == nullptr);

    popup_start_label_redirect(pstart, nullptr, label);
    expect(g_start2.e == 0);
    popup_start_label_value_redirect(pstart, nullptr, label, -9);
    expect(g_start2.e == -9);

    PopupOriginalStartFull = saved_start;
    PopupStartCaption = saved_caption;
}

void test_replay_win_stubs() {
    // Six constant-return stubs generated in bulk by tools/bulk_recover_stubs.
    // Each is a bare return that touches nothing, so the whole specification is
    // that a seeded object comes back unchanged - checked once for all six,
    // through both the method and its redirect. ReplayWin is large, so the
    // canary lives on the heap.
    std::vector<uint8_t> storage(sizeof(ReplayWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *replay = reinterpret_cast<ReplayWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    replay->on_left_double_click(1, 2);
    replay->on_right_double_click(3, 4);
    replay->on_mouse_move(5, 6);
    replay->on_right_down(7, 8);
    replay->on_left_down(9, 10);
    replay->on_left_up(11, 12);
    replay_win_on_left_double_click_redirect(replay, nullptr, -1, -2);
    replay_win_on_right_double_click_redirect(replay, nullptr, -3, -4);
    replay_win_on_mouse_move_redirect(replay, nullptr, -5, -6);
    replay_win_on_right_down_redirect(replay, nullptr, -7, -8);
    replay_win_on_left_down_redirect(replay, nullptr, -9, -10);
    replay_win_on_left_up_redirect(replay, nullptr, -11, -12);

    expect_storage_bytes(storage.data(), expected.data(), storage.size());
}

void test_bulk_generated_stubs() {
    // Eighteen constant-return stubs across seven classes, all generated by
    // tools/bulk_recover_stubs in one pass. Each touches nothing, so a seeded
    // object returning unchanged - through method and redirect - is the whole
    // check, plus the fixed return value where there is one.

    std::vector<uint8_t> report_if_b_storage(sizeof(ReportIf) + 32);
    std::vector<uint8_t> report_if_b_expected(report_if_b_storage.size());
    auto *report_if_b = reinterpret_cast<ReportIf *>(report_if_b_storage.data() + 16);
    seed_storage(report_if_b_storage.data(), report_if_b_expected.data(), report_if_b_storage.size());
    std::memcpy(report_if_b_expected.data(), report_if_b_storage.data(), report_if_b_storage.size());
    report_if_b->on_iface_right_click(1, 2);
    report_if_on_iface_right_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_left_double_click(1, 2);
    report_if_on_iface_left_double_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_right_double_click(1, 2);
    report_if_on_iface_right_double_click_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_left_down(1, 2);
    report_if_on_iface_left_down_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_right_down(1, 2);
    report_if_on_iface_right_down_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_mouse_move(1, 2);
    report_if_on_iface_mouse_move_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_mouse_leave(1, 2);
    report_if_on_iface_mouse_leave_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->on_iface_button_toggled(1, 2);
    report_if_on_iface_button_toggled_redirect(report_if_b, nullptr, -1, -2);
    report_if_b->close_score();
    report_if_close_score_redirect(report_if_b, nullptr);
    expect(report_if_b->on_iface_dialog_item_back_draw(1, 2, 3, 4) == 1);
    expect(report_if_on_iface_dialog_item_back_draw_redirect(
               report_if_b, nullptr, -1, -2, -3, -4) == 1);
    expect_storage_bytes(report_if_b_storage.data(), report_if_b_expected.data(),
                         report_if_b_storage.size());
    std::vector<uint8_t> dip_edit_b_storage(sizeof(DipEdit) + 32);
    std::vector<uint8_t> dip_edit_b_expected(dip_edit_b_storage.size());
    auto *dip_edit_b = reinterpret_cast<DipEdit *>(dip_edit_b_storage.data() + 16);
    seed_storage(dip_edit_b_storage.data(), dip_edit_b_expected.data(), dip_edit_b_storage.size());
    std::memcpy(dip_edit_b_expected.data(), dip_edit_b_storage.data(), dip_edit_b_storage.size());
    dip_edit_b->on_left_click(1, 2);
    dip_edit_on_left_click_redirect(dip_edit_b, nullptr, -1, -2);
    dip_edit_b->on_redraw();
    dip_edit_on_redraw_redirect(dip_edit_b, nullptr);
    expect_storage_bytes(dip_edit_b_storage.data(), dip_edit_b_expected.data(),
                         dip_edit_b_storage.size());
    std::vector<uint8_t> sound_b_storage(sizeof(Sound) + 32);
    std::vector<uint8_t> sound_b_expected(sound_b_storage.size());
    auto *sound_b = reinterpret_cast<Sound *>(sound_b_storage.data() + 16);
    seed_storage(sound_b_storage.data(), sound_b_expected.data(), sound_b_storage.size());
    std::memcpy(sound_b_expected.data(), sound_b_storage.data(), sound_b_storage.size());
    expect(sound_b->UNK1(1) == 11);
    expect(sound_unk1_redirect(sound_b, nullptr, -1) == 11);
    expect_storage_bytes(sound_b_storage.data(), sound_b_expected.data(),
                         sound_b_storage.size());
    std::vector<uint8_t> wave_device_b_storage(sizeof(Wave_Device) + 32);
    std::vector<uint8_t> wave_device_b_expected(wave_device_b_storage.size());
    auto *wave_device_b = reinterpret_cast<Wave_Device *>(wave_device_b_storage.data() + 16);
    seed_storage(wave_device_b_storage.data(), wave_device_b_expected.data(), wave_device_b_storage.size());
    std::memcpy(wave_device_b_expected.data(), wave_device_b_storage.data(), wave_device_b_storage.size());
    wave_device_b->set_pan(1);
    wave_device_set_pan_redirect(wave_device_b, nullptr, -1);
    expect(wave_device_b->fade(1) == 0);
    expect(wave_device_fade_redirect(wave_device_b, nullptr, -1) == 0);
    expect_storage_bytes(wave_device_b_storage.data(), wave_device_b_expected.data(),
                         wave_device_b_storage.size());
    std::vector<uint8_t> sprite_box_b_storage(sizeof(SpriteBox) + 32);
    std::vector<uint8_t> sprite_box_b_expected(sprite_box_b_storage.size());
    auto *sprite_box_b = reinterpret_cast<SpriteBox *>(sprite_box_b_storage.data() + 16);
    seed_storage(sprite_box_b_storage.data(), sprite_box_b_expected.data(), sprite_box_b_storage.size());
    std::memcpy(sprite_box_b_expected.data(), sprite_box_b_storage.data(), sprite_box_b_storage.size());
    sprite_box_b->on_mouse_move(1, 2);
    sprite_box_on_mouse_move_redirect(sprite_box_b, nullptr, -1, -2);
    sprite_box_b->on_mouse_leave(1, 2);
    sprite_box_on_mouse_leave_redirect(sprite_box_b, nullptr, -1, -2);
    expect_storage_bytes(sprite_box_b_storage.data(), sprite_box_b_expected.data(),
                         sprite_box_b_storage.size());
    std::vector<uint8_t> list_box_b_storage(sizeof(ListBox) + 32);
    std::vector<uint8_t> list_box_b_expected(list_box_b_storage.size());
    auto *list_box_b = reinterpret_cast<ListBox *>(list_box_b_storage.data() + 16);
    seed_storage(list_box_b_storage.data(), list_box_b_expected.data(), list_box_b_storage.size());
    std::memcpy(list_box_b_expected.data(), list_box_b_storage.data(), list_box_b_storage.size());
    list_box_b->on_dialog_focus(1);
    list_box_on_dialog_focus_redirect(list_box_b, nullptr, -1);
    expect_storage_bytes(list_box_b_storage.data(), list_box_b_expected.data(),
                         list_box_b_storage.size());
    std::vector<uint8_t> net_b_storage(sizeof(Net) + 32);
    std::vector<uint8_t> net_b_expected(net_b_storage.size());
    auto *net_b = reinterpret_cast<Net *>(net_b_storage.data() + 16);
    seed_storage(net_b_storage.data(), net_b_expected.data(), net_b_storage.size());
    std::memcpy(net_b_expected.data(), net_b_storage.data(), net_b_storage.size());
    expect(net_b->poll_players(1) == 1);
    expect(net_poll_players_redirect(net_b, nullptr, -1) == 1);
    expect_storage_bytes(net_b_storage.data(), net_b_expected.data(),
                         net_b_storage.size());
}

void test_remaining_constant_stubs() {
    // The last of the constant-return bucket - the ones the bulk tool skipped
    // for pointer or unsigned parameters, done by hand with faithful types.
    // Each touches nothing and returns its constant.
    std::vector<uint8_t> mw(sizeof(MapWin) + 32), mw_e(mw.size());
    auto *map = reinterpret_cast<MapWin *>(mw.data() + 16);
    seed_storage(mw.data(), mw_e.data(), mw.size());
    std::memcpy(mw_e.data(), mw.data(), mw.size());
    map->on_left_double_click(1, 2);
    map->on_left_up(3, 4);
    map_win_on_left_double_click_redirect(map, nullptr, -1, -2);
    map_win_on_left_up_redirect(map, nullptr, -3, -4);
    expect_storage_bytes(mw.data(), mw_e.data(), mw.size());

    std::vector<uint8_t> md(sizeof(Midi_Device) + 32), md_e(md.size());
    auto *midi = reinterpret_cast<Midi_Device *>(md.data() + 16);
    seed_storage(md.data(), md_e.data(), md.size());
    std::memcpy(md_e.data(), md.data(), md.size());
    char buf[8] = {};
    expect(midi->get_description(1u, buf, 2u) == 0);
    expect(midi_device_get_description_redirect(midi, nullptr, 1u, buf, 2u) == 0);
    expect_storage_bytes(md.data(), md_e.data(), md.size());

    std::vector<uint8_t> wd(sizeof(Wave_In_Device) + 32), wd_e(wd.size());
    auto *wave = reinterpret_cast<Wave_In_Device *>(wd.data() + 16);
    seed_storage(wd.data(), wd_e.data(), wd.size());
    std::memcpy(wd_e.data(), wd.data(), wd.size());
    expect(wave->get_description(1u, buf, 2u) == 0);
    expect(wave_in_device_get_description_redirect(wave, nullptr, 1u, buf, 2u) == 0);
    expect_storage_bytes(wd.data(), wd_e.data(), wd.size());

    std::vector<uint8_t> pu(sizeof(Popup) + 32), pu_e(pu.size());
    auto *popup = reinterpret_cast<Popup *>(pu.data() + 16);
    seed_storage(pu.data(), pu_e.data(), pu.size());
    std::memcpy(pu_e.data(), pu.data(), pu.size());
    expect(popup->on_dialog_back_draw(nullptr) == 0);
    expect(popup_on_dialog_back_draw_redirect(popup, nullptr, nullptr) == 0);
    expect_storage_bytes(pu.data(), pu_e.data(), pu.size());

    // Win::OnSetCursor and BaseWin::timer_callback are static; no instance.
    expect(Win::OnSetCursor(nullptr, nullptr, 0u, 0u) == 1);
    expect(win_onsetcursor_redirect(nullptr, nullptr, 1u, 2u) == 1);
    BaseWin::timer_callback(1, 2);
    base_win_timer_callback_redirect(-1, -2);
}

void test_field_store_batch2() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0; std::memcpy(&v, s.data() + 16 + off, sizeof(v)); return v;
    };

    // SquareLock::clear: two -1 sentinels and a zero at 0, 4, 8.
    std::vector<uint8_t> sl(sizeof(SquareLock) + 32), sl_e(sl.size());
    auto *lock = reinterpret_cast<SquareLock *>(sl.data() + 16);
    seed_storage(sl.data(), sl_e.data(), sl.size());
    std::memcpy(sl_e.data(), sl.data(), sl.size());
    lock->clear();
    expect(read32(sl, 0) == -1);
    expect(read32(sl, 4) == -1);
    expect(read32(sl, 8) == 0);
    std::memcpy(sl_e.data() + 16, sl.data() + 16, 12);
    expect_storage_bytes(sl.data(), sl_e.data(), sl.size());
    square_lock_clear_redirect(lock, nullptr);

    // DeletionList::clear(index): the marker byte and word land at index*0x3C,
    // which the test checks at two different indices - a wrong stride would put
    // the second one somewhere the check does not look.
    std::vector<uint8_t> dl(0x3C * 4 + 32), dl_e(dl.size());
    auto *list = reinterpret_cast<DeletionList *>(dl.data() + 16);
    for (int index : {0, 2}) {
        seed_storage(dl.data(), dl_e.data(), dl.size());
        std::memcpy(dl_e.data(), dl.data(), dl.size());
        list->clear(index);
        const size_t base = 16 + static_cast<size_t>(index) * 0x3C;
        expect(dl[base] == 0xFF);
        uint16_t word = 0xFFFF;
        std::memcpy(&word, dl.data() + base + 8, sizeof(word));
        expect(word == 0);
        dl_e[base] = dl[base];
        std::memcpy(dl_e.data() + base + 8, dl.data() + base + 8, 2);
        expect_storage_bytes(dl.data(), dl_e.data(), dl.size());
    }
    deletion_list_clear_redirect(list, nullptr, 1);

    // DiploWin::UNK2: zeroes 0xA24 and 0xA28.
    std::vector<uint8_t> dw(sizeof(DiploWin) + 32), dw_e(dw.size());
    auto *diplo = reinterpret_cast<DiploWin *>(dw.data() + 16);
    seed_storage(dw.data(), dw_e.data(), dw.size());
    std::memcpy(dw_e.data(), dw.data(), dw.size());
    diplo->UNK2();
    expect(read32(dw, 0xA24) == 0);
    expect(read32(dw, 0xA28) == 0);
    std::memcpy(dw_e.data() + 16 + 0xA24, dw.data() + 16 + 0xA24, 8);
    expect_storage_bytes(dw.data(), dw_e.data(), dw.size());
    diplo_win_unk2_redirect(diplo, nullptr);
}

void test_pull_down_id_to_index() {
    // A linear search over the 64 item slots comparing each item's id, at
    // 0xA18 + index*0x14 + 8. The search's three exits are all checked: a
    // match returns the index, a -1 sentinel id stops the scan and returns -1,
    // and an id absent before the sentinel also returns -1.
    std::vector<uint8_t> storage(sizeof(PullDown) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *pull = reinterpret_cast<PullDown *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    auto set_id = [&](int index, int id) {
        std::memcpy(storage.data() + 16 + 0xA18 + index * 0x14 + 8, &id,
                    sizeof(id));
    };
    // Ids 100, 200, 300 in the first three slots, then the -1 sentinel.
    set_id(0, 100);
    set_id(1, 200);
    set_id(2, 300);
    set_id(3, -1);
    std::memcpy(expected.data(), storage.data(), storage.size());

    expect(pull->id_to_index(100) == 0);
    expect(pull->id_to_index(200) == 1);
    expect(pull->id_to_index(300) == 2);
    // Absent id, scan stops at the sentinel in slot 3.
    expect(pull->id_to_index(999) == -1);
    // The id -1 is the sentinel, so it is never matched as a value.
    expect(pull->id_to_index(-1) == -1);
    expect(pull_down_id_to_index_redirect(pull, nullptr, 200) == 1);
    // A pure search writes nothing.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // First match wins when an id repeats.
    set_id(5, 200);
    expect(pull->id_to_index(200) == 1);
}

void test_loop_store_searches() {
    // Palette::get_pos: find-or-first-free over five slots. A matching value
    // returns its index, an empty (-1) slot returns its index, and a full
    // table with no match returns 5. The find and the free-slot exits both
    // return the current index, so both are checked, as is the full case.
    std::vector<uint8_t> pl(sizeof(Palette) + 32), pl_e(pl.size());
    auto *palette = reinterpret_cast<Palette *>(pl.data() + 16);
    seed_storage(pl.data(), pl_e.data(), pl.size());
    auto set_slot = [&](int i, int32_t v) {
        std::memcpy(pl.data() + 16 + 0x404 + i * 0x10, &v, sizeof(v));
    };
    set_slot(0, 0x100);
    set_slot(1, 0x200);
    set_slot(2, 0x300);
    set_slot(3, 0x400);
    set_slot(4, 0x500);
    std::memcpy(pl_e.data(), pl.data(), pl.size());
    expect(palette->get_pos(0x100) == 0);
    expect(palette->get_pos(0x300) == 2);
    expect(palette->get_pos(0x500) == 4);
    expect(palette->get_pos(0x999) == 5);
    expect(palette_get_pos_redirect(palette, nullptr, 0x400) == 3);
    expect_storage_bytes(pl.data(), pl_e.data(), pl.size());

    set_slot(2, -1);
    expect(palette->get_pos(0x999) == 2);
    expect(palette->get_pos(0x100) == 0);
    expect(palette->get_pos(0x400) == 2);

    // PlayerLock::active: 1 when either entry's flag has the low bit set.
    std::vector<uint8_t> lk(sizeof(PlayerLock) + 32), lk_e(lk.size());
    auto *lock = reinterpret_cast<PlayerLock *>(lk.data() + 16);
    auto set_flag = [&](int entry, int32_t v) {
        std::memcpy(lk.data() + 16 + 0xC + entry * 0xC, &v, sizeof(v));
    };
    seed_storage(lk.data(), lk_e.data(), lk.size());
    set_flag(0, 0);
    set_flag(1, 0);
    std::memcpy(lk_e.data(), lk.data(), lk.size());
    expect(lock->active() == 0);
    set_flag(1, 1);
    expect(lock->active() == 1);                 // second entry engaged
    set_flag(1, 0);
    set_flag(0, 3);                              // low bit set among others
    expect(lock->active() == 1);
    set_flag(0, 2);                              // low bit clear
    expect(lock->active() == 0);
    set_flag(0, 1);
    expect(player_lock_active_redirect(lock, nullptr) == 1);
    std::memcpy(lk_e.data(), lk.data(), lk.size());
    (void)lock->active();
    expect_storage_bytes(lk.data(), lk_e.data(), lk.size());
}

void test_win_unk3_contains() {
    // A linear "contains" over the id table at 0x1A4 with its count at 0x3FC.
    // All four exits are checked: a present value returns 1, an absent one 0,
    // a zero query 0 without scanning, and an empty table 0.
    std::vector<uint8_t> storage(sizeof(Win) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *window = reinterpret_cast<Win *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    auto set_count = [&](int32_t c) {
        std::memcpy(storage.data() + 16 + 0x3FC, &c, sizeof(c));
    };
    auto set_entry = [&](int i, int32_t v) {
        std::memcpy(storage.data() + 16 + 0x1A4 + i * 4, &v, sizeof(v));
    };
    set_count(3);
    set_entry(0, 0x111);
    set_entry(1, 0x222);
    set_entry(2, 0x333);
    std::memcpy(expected.data(), storage.data(), storage.size());

    expect(window->UNK3(0x111) == 1);
    expect(window->UNK3(0x333) == 1);
    expect(window->UNK3(0x444) == 0);          // absent
    expect(window->UNK3(0) == 0);              // zero query, no scan
    expect(win_unk3_redirect(window, nullptr, 0x222) == 1);
    // A pure search writes nothing.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Empty table: everything is absent, and a value that sits in the array
    // past the count must not be found.
    set_count(0);
    expect(window->UNK3(0x111) == 0);
    set_count(1);
    expect(window->UNK3(0x222) == 0);          // 0x222 is at index 1, past count 1
    expect(window->UNK3(0x111) == 1);          // 0x111 at index 0, within count
}

void test_console_clear_group() {
    // Clears the field at 0x23D1C and drops bit 27 from every entry of a group
    // table at 0x34 stride, over a count both read from fixed addresses. The
    // seams point at a local table so the masking is observable: each entry's
    // other bits survive, only 0x08000000 is cleared, and entries past the
    // count are untouched.
    int32_t count = 4;
    std::vector<uint8_t> table(0x34 * 6, 0);
    std::vector<uint8_t> table_before;
    int32_t *const saved_count = ConsoleGroupCount;
    uint8_t *const saved_table = ConsoleGroupTable;
    ConsoleGroupCount = &count;
    ConsoleGroupTable = table.data();

    auto entry = [&](int i) {
        uint32_t v = 0; std::memcpy(&v, table.data() + i * 0x34, 4); return v;
    };
    auto set_entry = [&](int i, uint32_t v) {
        std::memcpy(table.data() + i * 0x34, &v, 4);
    };
    // All bits set in the first four entries, plus one past the count.
    for (int i = 0; i < 6; ++i) set_entry(i, 0xFFFFFFFFu);
    table_before = table;

    std::vector<uint8_t> storage(sizeof(Console) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *console = reinterpret_cast<Console *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    int32_t marker = 0x1234;
    std::memcpy(storage.data() + 16 + 0x23D1C, &marker, sizeof(marker));
    std::memcpy(expected.data(), storage.data(), storage.size());

    console->clear_group();

    // Field cleared.
    int32_t field = -1;
    std::memcpy(&field, storage.data() + 16 + 0x23D1C, sizeof(field));
    expect(field == 0);
    std::memcpy(expected.data() + 16 + 0x23D1C, storage.data() + 16 + 0x23D1C, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Entries 0..3 had only bit 27 cleared; entries 4 and 5 (past count 4)
    // are untouched.
    for (int i = 0; i < 4; ++i) expect(entry(i) == 0xF7FFFFFFu);
    expect(entry(4) == 0xFFFFFFFFu);
    expect(entry(5) == 0xFFFFFFFFu);

    // Count <= 0 leaves the table alone.
    table = table_before;
    count = 0;
    console->clear_group();
    expect(table == table_before);
    count = 2;
    console_clear_group_redirect(console, nullptr);
    expect(entry(0) == 0xF7FFFFFFu);
    expect(entry(2) == 0xFFFFFFFFu);   // index 2 past count 2

    ConsoleGroupCount = saved_count;
    ConsoleGroupTable = saved_table;
}

void test_lock_reset_map() {
    // Clears bits 0x38 of the flag byte at offset 5 of each 0x2C-byte record,
    // over a count both from fixed addresses; the seams point at a local table.
    // The mask keeps bits outside 0x38, the offset-5 placement, the stride, and
    // the count bound are all checked.
    int32_t count = 4;
    std::vector<uint8_t> tbl(0x2C * 6, 0);
    int32_t *const saved_count = LockMapCount;
    uint8_t *const saved_table = LockMapTable;
    LockMapCount = &count;
    LockMapTable = tbl.data();

    // Every bit set in each record's flag byte, plus other bytes set too so a
    // wrong offset would clear the wrong byte.
    for (size_t i = 0; i < tbl.size(); ++i) tbl[i] = 0xFF;
    std::vector<uint8_t> before = tbl;

    std::vector<uint8_t> lk(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(lk.data() + 16);
    lock->reset_map();

    // Records 0..3: only the flag byte at +5 changed, to 0xFF & 0xC7 = 0xC7.
    for (int i = 0; i < 4; ++i) {
        expect(tbl[i * 0x2C + 5] == 0xC7);
        // Neighbouring bytes in the record untouched.
        expect(tbl[i * 0x2C + 4] == 0xFF);
        expect(tbl[i * 0x2C + 6] == 0xFF);
    }
    // Records 4, 5 past count 4 are whole.
    expect(tbl[4 * 0x2C + 5] == 0xFF);
    expect(tbl[5 * 0x2C + 5] == 0xFF);

    // Count <= 0 leaves the table alone.
    tbl = before;
    count = 0;
    lock->reset_map();
    expect(tbl == before);
    count = 2;
    lock_reset_map_redirect(lock, nullptr);
    expect(tbl[0 * 0x2C + 5] == 0xC7);
    expect(tbl[2 * 0x2C + 5] == 0xFF);   // index 2 past count 2

    LockMapCount = saved_count;
    LockMapTable = saved_table;
}

void test_lock_clear() {
    // Initialises eight 0x1C records to two -1 sentinels and a zero each, plus
    // three trailing dwords, then runs reset_map. The map count is set to zero
    // so the global tail is a no-op and this checks only the record init.
    int32_t map_count = 0;
    int32_t *const saved_count = LockMapCount;
    LockMapCount = &map_count;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    lock->clear();

    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, storage.data() + 16 + off, sizeof(v)); return v;
    };
    // Each record: flag byte 0 at record*0x1C, then entries {-1,-1,0} at +4 and
    // +0x10. Every one of the eight is checked, not a sample - a loop bound
    // that stopped early would leave a later record seeded.
    for (int r = 0; r < 8; ++r) {
        const size_t base = r * 0x1C;
        expect((storage[16 + base] & 0xFF) == 0);
        expect(read32(base + 0x04) == -1);
        expect(read32(base + 0x08) == -1);
        expect(read32(base + 0x0C) == 0);
        expect(read32(base + 0x10) == -1);
        expect(read32(base + 0x14) == -1);
        expect(read32(base + 0x18) == 0);
    }
    // The three trailing dwords at 0xE0.
    expect(read32(0xE0) == 0);
    expect(read32(0xE4) == 0);
    expect(read32(0xE8) == 0);

    lock_clear_redirect(lock, nullptr);
    LockMapCount = saved_count;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(134, test_draw_tile_broadcast);
LEAF_CASE(145, test_popup_exec_and_start_overloads);
LEAF_CASE(146, test_replay_win_stubs);
LEAF_CASE(147, test_bulk_generated_stubs);
LEAF_CASE(148, test_remaining_constant_stubs);
LEAF_CASE(149, test_field_store_batch2);
LEAF_CASE(150, test_pull_down_id_to_index);
LEAF_CASE(151, test_loop_store_searches);
LEAF_CASE(152, test_win_unk3_contains);
LEAF_CASE(153, test_console_clear_group);
LEAF_CASE(154, test_lock_reset_map);
LEAF_CASE(155, test_lock_clear);
}  // namespace
