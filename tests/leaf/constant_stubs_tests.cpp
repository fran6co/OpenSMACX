// Recovery leaf tests: constant_stubs.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "constant_stubs"

void test_constant_return_stubs() {
    // Fifteen legacy stubs whose entire body sets a constant and returns. The
    // shape classifier proposed them; each was confirmed against its own
    // instruction bytes, including that the `ret N` cleanup matches the
    // arity its mangled name implies. Win::on_redraw was rejected from this
    // batch on exactly that check - its name declares no parameters while its
    // body cleans eight bytes.
    //
    // The object must come back untouched: these read nothing and write
    // nothing, so a full byte comparison is the whole specification.
    alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *win = reinterpret_cast<Win *>(win_storage + 16);
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    expect(win->UNK1(1, 2, 3, 4, 5, 6, 7, 8, 9) == 0);
    expect(win->UNK5() == 0);
    expect(win->UNK6(INT_MIN) == 0);
    expect(win->on_set_cursor(nullptr, 0U, 0U) == 1);
    expect(win_unk1_redirect(win, nullptr, -1, -2, -3, -4, -5, -6, -7, -8, -9) == 0);
    expect(win_unk5_redirect(win, nullptr) == 0);
    expect(win_unk6_redirect(win, nullptr, INT_MAX) == 0);
    expect(win_on_set_cursor_redirect(
               win, nullptr, win_storage, 0xFFFFFFFFU, 0xFFFFFFFFU) == 1);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    alignas(Sprite) uint8_t sprite_storage[sizeof(Sprite) + 32];
    uint8_t sprite_expected[sizeof(sprite_storage)];
    auto *sprite = reinterpret_cast<Sprite *>(sprite_storage + 16);
    seed_storage(sprite_storage, sprite_expected, sizeof(sprite_storage));
    std::memcpy(sprite_expected, sprite_storage, sizeof(sprite_storage));
    expect(sprite->UNK1(1, 2, 3, 4, 5, 6, 7) == 0);
    expect(sprite->UNK2(1, 2, 3, 4, 5) == 0);
    expect(sprite_unk1_redirect(sprite, nullptr, -1, -2, -3, -4, -5, -6, -7) == 0);
    expect(sprite_unk2_redirect(sprite, nullptr, -1, -2, -3, -4, -5) == 0);
    expect_storage_bytes(sprite_storage, sprite_expected, sizeof(sprite_storage));

    alignas(PullDown) uint8_t pd_storage[sizeof(PullDown) + 32];
    uint8_t pd_expected[sizeof(pd_storage)];
    auto *pull_down = reinterpret_cast<PullDown *>(pd_storage + 16);
    seed_storage(pd_storage, pd_expected, sizeof(pd_storage));
    std::memcpy(pd_expected, pd_storage, sizeof(pd_storage));
    // These two return 8 rather than zero, which is the distinction a
    // constant-return stub most easily gets wrong.
    expect(pull_down->UNK2(0) == 8);
    expect(pull_down->UNK3(0, 0, 0) == 8);
    expect(pull_down->UNK5() == 1);
    expect(pull_down->UNK6() == 1);
    expect(pull_down_unk2_redirect(pull_down, nullptr, INT_MIN) == 8);
    expect(pull_down_unk3_redirect(pull_down, nullptr, 1, 2, 3) == 8);
    expect(pull_down_unk5_redirect(pull_down, nullptr) == 1);
    expect(pull_down_unk6_redirect(pull_down, nullptr) == 1);
    expect_storage_bytes(pd_storage, pd_expected, sizeof(pd_storage));

    alignas(Menu) uint8_t menu_storage[sizeof(Menu) + 32];
    uint8_t menu_expected[sizeof(menu_storage)];
    auto *menu = reinterpret_cast<Menu *>(menu_storage + 16);
    seed_storage(menu_storage, menu_expected, sizeof(menu_storage));
    std::memcpy(menu_expected, menu_storage, sizeof(menu_storage));
    expect(menu->UNK2(0) == 0);
    expect(menu->UNK4(0, 0, 0) == 0);
    expect(menu_unk2_redirect(menu, nullptr, INT_MAX) == 0);
    expect(menu_unk4_redirect(menu, nullptr, 1, 2, 3) == 0);

    alignas(Font) uint8_t font_storage[sizeof(Font) + 32];
    uint8_t font_expected[sizeof(font_storage)];
    auto *font = reinterpret_cast<Font *>(font_storage + 16);
    seed_storage(font_storage, font_expected, sizeof(font_storage));
    std::memcpy(font_expected, font_storage, sizeof(font_storage));
    expect(font->UNK1(1, 2, 3, 4) == 1);
    expect(font_unk1_redirect(font, nullptr, -1, -2, -3, -4) == 1);
    expect_storage_bytes(font_storage, font_expected, sizeof(font_storage));
    expect_storage_bytes(menu_storage, menu_expected, sizeof(menu_storage));

    alignas(BaseButton) uint8_t bb_storage[sizeof(BaseButton) + 32];
    uint8_t bb_expected[sizeof(bb_storage)];
    auto *button = reinterpret_cast<BaseButton *>(bb_storage + 16);
    seed_storage(bb_storage, bb_expected, sizeof(bb_storage));
    std::memcpy(bb_expected, bb_storage, sizeof(bb_storage));
    button->on_key_click(1, 2);
    button->on_key_down(3);
    button->on_key_up(4);
    base_button_on_key_click_redirect(button, nullptr, -1, -2);
    base_button_on_key_down_redirect(button, nullptr, -3);
    base_button_on_key_up_redirect(button, nullptr, -4);
    expect_storage_bytes(bb_storage, bb_expected, sizeof(bb_storage));

    // MainInterface is far too large to seed on the stack, so its canary
    // lives on the heap. These three are bare returns rather than constant
    // returns - the original bodies are a single `ret` - so leaving every
    // byte alone is the entire behaviour there is to check.
    std::vector<uint8_t> mi_storage(sizeof(MainInterface) + 32);
    std::vector<uint8_t> mi_expected(mi_storage.size());
    auto *interface = reinterpret_cast<MainInterface *>(mi_storage.data() + 16);
    seed_storage(mi_storage.data(), mi_expected.data(), mi_storage.size());
    std::memcpy(mi_expected.data(), mi_storage.data(), mi_storage.size());
    interface->UNK2();
    interface->UNK3();
    interface->UNK4();
    main_interface_unk2_redirect(interface, nullptr);
    main_interface_unk3_redirect(interface, nullptr);
    main_interface_unk4_redirect(interface, nullptr);
    expect_storage_bytes(mi_storage.data(), mi_expected.data(), mi_storage.size());

    alignas(Flic) uint8_t flic_storage[sizeof(Flic) + 32];
    uint8_t flic_expected[sizeof(flic_storage)];
    auto *flic = reinterpret_cast<Flic *>(flic_storage + 16);
    seed_storage(flic_storage, flic_expected, sizeof(flic_storage));
    std::memcpy(flic_expected, flic_storage, sizeof(flic_storage));
    flic->UNK4();
    flic->UNK5();
    flic->UNK6();
    flic->UNK7();
    flic->UNK8();
    flic_unk4_redirect(flic, nullptr);
    flic_unk5_redirect(flic, nullptr);
    flic_unk6_redirect(flic, nullptr);
    flic_unk7_redirect(flic, nullptr);
    flic_unk8_redirect(flic, nullptr);
    expect_storage_bytes(flic_storage, flic_expected, sizeof(flic_storage));

    // Eighteen ambience event hooks across three faction flavours, none of
    // which responds to anything. Where these objects end is not established,
    // so the canary is sized to the layout modelled here rather than to a
    // proven extent; what it checks is that a hook which should do nothing
    // writes nothing, which holds whatever the true trailing size turns out
    // to be.
    alignas(FactionAmbience) uint8_t fa_storage[sizeof(FactionAmbience) + 32];
    uint8_t fa_expected[sizeof(fa_storage)];
    auto *faction_ambience = reinterpret_cast<FactionAmbience *>(fa_storage + 16);
    seed_storage(fa_storage, fa_expected, sizeof(fa_storage));
    std::memcpy(fa_expected, fa_storage, sizeof(fa_storage));
    faction_ambience->begin();
    faction_ambience->tech();
    faction_ambience->terraform();
    faction_ambience->production();
    faction_ambience->general();
    faction_ambience->new_base();
    faction_ambience->popup1();
    faction_ambience->eot();
    faction_ambience->hostility();
    faction_ambience->energy_resources();
    faction_ambience->base_liberated();
    faction_ambience_begin_redirect(faction_ambience, nullptr);
    faction_ambience_tech_redirect(faction_ambience, nullptr);
    faction_ambience_terraform_redirect(faction_ambience, nullptr);
    faction_ambience_production_redirect(faction_ambience, nullptr);
    faction_ambience_general_redirect(faction_ambience, nullptr);
    faction_ambience_new_base_redirect(faction_ambience, nullptr);
    faction_ambience_popup1_redirect(faction_ambience, nullptr);
    faction_ambience_eot_redirect(faction_ambience, nullptr);
    faction_ambience_hostility_redirect(faction_ambience, nullptr);
    faction_ambience_energy_resources_redirect(faction_ambience, nullptr);
    faction_ambience_base_liberated_redirect(faction_ambience, nullptr);
    expect_storage_bytes(fa_storage, fa_expected, sizeof(fa_storage));

    alignas(UAmbience) uint8_t ua_storage[sizeof(UAmbience) + 32];
    uint8_t ua_expected[sizeof(ua_storage)];
    auto *u_ambience = reinterpret_cast<UAmbience *>(ua_storage + 16);
    seed_storage(ua_storage, ua_expected, sizeof(ua_storage));
    std::memcpy(ua_expected, ua_storage, sizeof(ua_storage));
    u_ambience->tech();
    u_ambience->popup1();
    u_ambience->eot();
    u_ambience_tech_redirect(u_ambience, nullptr);
    u_ambience_popup1_redirect(u_ambience, nullptr);
    u_ambience_eot_redirect(u_ambience, nullptr);
    expect_storage_bytes(ua_storage, ua_expected, sizeof(ua_storage));

    alignas(GAmbience) uint8_t ga_storage[sizeof(GAmbience) + 32];
    uint8_t ga_expected[sizeof(ga_storage)];
    auto *g_ambience = reinterpret_cast<GAmbience *>(ga_storage + 16);
    seed_storage(ga_storage, ga_expected, sizeof(ga_storage));
    std::memcpy(ga_expected, ga_storage, sizeof(ga_storage));
    g_ambience->tech();
    g_ambience->production();
    g_ambience->popup1();
    g_ambience->eot();
    g_ambience_tech_redirect(g_ambience, nullptr);
    g_ambience_production_redirect(g_ambience, nullptr);
    g_ambience_popup1_redirect(g_ambience, nullptr);
    g_ambience_eot_redirect(g_ambience, nullptr);
    expect_storage_bytes(ga_storage, ga_expected, sizeof(ga_storage));

    // The five that DO write: a single flag byte each, and nothing else in
    // the object. The two FactionAmbience pairs share a byte apiece, so the
    // show/hide of one must leave the other's byte alone - which a body that
    // wrote the wrong offset would break silently in the other direction.
    struct FlagCase {
        size_t offset;
        int value;
        void (__fastcall *redirect)(FactionAmbience *, void *);
    };
    const FlagCase faction_flags[] = {
        {0x6C, 1, &faction_ambience_basewin_show_redirect},
        {0x6C, 0, &faction_ambience_basewin_hide_redirect},
        {0x6D, 1, &faction_ambience_design_window_show_redirect},
        {0x6D, 0, &faction_ambience_design_window_hide_redirect},
    };
    for (const FlagCase &test : faction_flags) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(fa_storage, fa_expected, sizeof(fa_storage));
            std::memcpy(fa_expected, fa_storage, sizeof(fa_storage));
            fa_expected[16 + test.offset] = static_cast<uint8_t>(test.value);
            if (adapter) {
                test.redirect(faction_ambience, nullptr);
            } else if (test.offset == 0x6C) {
                if (test.value) { faction_ambience->basewin_show(); }
                else { faction_ambience->basewin_hide(); }
            } else {
                if (test.value) { faction_ambience->design_window_show(); }
                else { faction_ambience->design_window_hide(); }
            }
            expect_storage_bytes(fa_storage, fa_expected, sizeof(fa_storage));
        }
    }
    for (int adapter = 0; adapter < 2; ++adapter) {
        seed_storage(ga_storage, ga_expected, sizeof(ga_storage));
        std::memcpy(ga_expected, ga_storage, sizeof(ga_storage));
        ga_expected[16 + 0x6C] = 0;
        if (adapter) { g_ambience_basewin_hide_redirect(g_ambience, nullptr); }
        else { g_ambience->basewin_hide(); }
        expect_storage_bytes(ga_storage, ga_expected, sizeof(ga_storage));
    }

    // Two sound devices that decline to be polled, suspended, or restarted,
    // and one Win clip reset that resets nothing. Their layouts are bounded
    // rather than established, so as with the ambience hooks the canary is
    // sized to what is modelled here; a method that should do nothing writes
    // nothing regardless of where the object really ends.
    alignas(Midi_Device) uint8_t midi_storage[sizeof(Midi_Device) + 32];
    uint8_t midi_expected[sizeof(midi_storage)];
    auto *midi = reinterpret_cast<Midi_Device *>(midi_storage + 16);
    seed_storage(midi_storage, midi_expected, sizeof(midi_storage));
    std::memcpy(midi_expected, midi_storage, sizeof(midi_storage));
    midi->update_sound();
    midi->suspend();
    midi->restart();
    midi_device_update_sound_redirect(midi, nullptr);
    midi_device_suspend_redirect(midi, nullptr);
    midi_device_restart_redirect(midi, nullptr);
    expect_storage_bytes(midi_storage, midi_expected, sizeof(midi_storage));

    alignas(Wave_In_Device) uint8_t wave_storage[sizeof(Wave_In_Device) + 32];
    uint8_t wave_expected[sizeof(wave_storage)];
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(wave_storage + 16);
    seed_storage(wave_storage, wave_expected, sizeof(wave_storage));
    std::memcpy(wave_expected, wave_storage, sizeof(wave_storage));
    wave_in->update_sound();
    wave_in->suspend();
    wave_in->restart();
    wave_in_device_update_sound_redirect(wave_in, nullptr);
    wave_in_device_suspend_redirect(wave_in, nullptr);
    wave_in_device_restart_redirect(wave_in, nullptr);
    expect_storage_bytes(wave_storage, wave_expected, sizeof(wave_storage));

    alignas(Win) uint8_t clip_storage[sizeof(Win) + 32];
    uint8_t clip_expected[sizeof(clip_storage)];
    auto *clip_win = reinterpret_cast<Win *>(clip_storage + 16);
    seed_storage(clip_storage, clip_expected, sizeof(clip_storage));
    std::memcpy(clip_expected, clip_storage, sizeof(clip_storage));
    clip_win->reset_window_clip();
    win_reset_window_clip_redirect(clip_win, nullptr);
    expect_storage_bytes(clip_storage, clip_expected, sizeof(clip_storage));

    // MapWin is the first class here whose size is pinned rather than
    // bounded, so unlike the ambience and sound-device canaries this one
    // spans the whole object the original allocates - 0x22480 bytes, far too
    // large for the stack.
    std::vector<uint8_t> mw_storage(sizeof(MapWin) + 32);
    std::vector<uint8_t> mw_expected(mw_storage.size());
    auto *map_win = reinterpret_cast<MapWin *>(mw_storage.data() + 16);
    seed_storage(mw_storage.data(), mw_expected.data(), mw_storage.size());
    std::memcpy(mw_expected.data(), mw_storage.data(), mw_storage.size());
    map_win->UNK3();
    map_win->do_image_buttons();
    map_win_unk3_redirect(map_win, nullptr);
    map_win_do_image_buttons_redirect(map_win, nullptr);
    expect_storage_bytes(mw_storage.data(), mw_expected.data(), mw_storage.size());

    // Sixteen bare returns across eight window classes. None of these layouts
    // is established, so each canary spans only the GraphicWin base the class
    // is declared with rather than the object's true extent - which is all
    // these methods could reach anyway, since they touch nothing.
    std::vector<uint8_t> base_win_storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> base_win_expected(base_win_storage.size());
    auto *base_win = reinterpret_cast<BaseWin *>(base_win_storage.data() + 16);
    seed_storage(base_win_storage.data(), base_win_expected.data(), base_win_storage.size());
    std::memcpy(base_win_expected.data(), base_win_storage.data(), base_win_storage.size());
    base_win->close();
    base_win->UNK4();
    base_win->UNK6();
    base_win->UNK7();
    base_win_close_redirect(base_win, nullptr);
    base_win_unk4_redirect(base_win, nullptr);
    base_win_unk6_redirect(base_win, nullptr);
    base_win_unk7_redirect(base_win, nullptr);
    expect_storage_bytes(base_win_storage.data(), base_win_expected.data(),
                         base_win_storage.size());
    std::vector<uint8_t> datalink_storage(sizeof(Datalink) + 32);
    std::vector<uint8_t> datalink_expected(datalink_storage.size());
    auto *datalink = reinterpret_cast<Datalink *>(datalink_storage.data() + 16);
    seed_storage(datalink_storage.data(), datalink_expected.data(), datalink_storage.size());
    std::memcpy(datalink_expected.data(), datalink_storage.data(), datalink_storage.size());
    datalink->UNK6();
    datalink->UNK8();
    datalink->UNK9();
    datalink_unk6_redirect(datalink, nullptr);
    datalink_unk8_redirect(datalink, nullptr);
    datalink_unk9_redirect(datalink, nullptr);
    expect_storage_bytes(datalink_storage.data(), datalink_expected.data(),
                         datalink_storage.size());
    std::vector<uint8_t> net_win_storage(sizeof(NetWin) + 32);
    std::vector<uint8_t> net_win_expected(net_win_storage.size());
    auto *net_win = reinterpret_cast<NetWin *>(net_win_storage.data() + 16);
    seed_storage(net_win_storage.data(), net_win_expected.data(), net_win_storage.size());
    std::memcpy(net_win_expected.data(), net_win_storage.data(), net_win_storage.size());
    net_win->UNK1();
    net_win->UNK2();
    net_win->alloc_slots();
    net_win_unk1_redirect(net_win, nullptr);
    net_win_unk2_redirect(net_win, nullptr);
    net_win_alloc_slots_redirect(net_win, nullptr);
    expect_storage_bytes(net_win_storage.data(), net_win_expected.data(),
                         net_win_storage.size());
    std::vector<uint8_t> social_win_storage(sizeof(SocialWin) + 32);
    std::vector<uint8_t> social_win_expected(social_win_storage.size());
    auto *social_win = reinterpret_cast<SocialWin *>(social_win_storage.data() + 16);
    seed_storage(social_win_storage.data(), social_win_expected.data(), social_win_storage.size());
    std::memcpy(social_win_expected.data(), social_win_storage.data(), social_win_storage.size());
    social_win->UNK2();
    social_win->UNK3();
    social_win_unk2_redirect(social_win, nullptr);
    social_win_unk3_redirect(social_win, nullptr);
    expect_storage_bytes(social_win_storage.data(), social_win_expected.data(),
                         social_win_storage.size());
    std::vector<uint8_t> design_win_storage(sizeof(DesignWin) + 32);
    std::vector<uint8_t> design_win_expected(design_win_storage.size());
    auto *design_win = reinterpret_cast<DesignWin *>(design_win_storage.data() + 16);
    seed_storage(design_win_storage.data(), design_win_expected.data(), design_win_storage.size());
    std::memcpy(design_win_expected.data(), design_win_storage.data(), design_win_storage.size());
    design_win->UNK1();
    design_win_unk1_redirect(design_win, nullptr);
    expect_storage_bytes(design_win_storage.data(), design_win_expected.data(),
                         design_win_storage.size());
    std::vector<uint8_t> world_win_storage(sizeof(WorldWin) + 32);
    std::vector<uint8_t> world_win_expected(world_win_storage.size());
    auto *world_win = reinterpret_cast<WorldWin *>(world_win_storage.data() + 16);
    seed_storage(world_win_storage.data(), world_win_expected.data(), world_win_storage.size());
    std::memcpy(world_win_expected.data(), world_win_storage.data(), world_win_storage.size());
    world_win->clear_terrain();
    world_win_clear_terrain_redirect(world_win, nullptr);
    expect_storage_bytes(world_win_storage.data(), world_win_expected.data(),
                         world_win_storage.size());
    std::vector<uint8_t> diplo_win_storage(sizeof(DiploWin) + 32);
    std::vector<uint8_t> diplo_win_expected(diplo_win_storage.size());
    auto *diplo_win = reinterpret_cast<DiploWin *>(diplo_win_storage.data() + 16);
    seed_storage(diplo_win_storage.data(), diplo_win_expected.data(), diplo_win_storage.size());
    std::memcpy(diplo_win_expected.data(), diplo_win_storage.data(), diplo_win_storage.size());
    diplo_win->UNK5();
    diplo_win_unk5_redirect(diplo_win, nullptr);
    expect_storage_bytes(diplo_win_storage.data(), diplo_win_expected.data(),
                         diplo_win_storage.size());
    std::vector<uint8_t> alpha_movie_storage(sizeof(AlphaMovie) + 32);
    std::vector<uint8_t> alpha_movie_expected(alpha_movie_storage.size());
    auto *alpha_movie = reinterpret_cast<AlphaMovie *>(alpha_movie_storage.data() + 16);
    seed_storage(alpha_movie_storage.data(), alpha_movie_expected.data(), alpha_movie_storage.size());
    std::memcpy(alpha_movie_expected.data(), alpha_movie_storage.data(), alpha_movie_storage.size());
    alpha_movie->UNK7();
    alpha_movie_unk7_redirect(alpha_movie, nullptr);
    expect_storage_bytes(alpha_movie_storage.data(), alpha_movie_expected.data(),
                         alpha_movie_storage.size());

    // Three more window classes, each over a base this repository has already
    // pinned - Time, Caviar, FlatButton - so the canary covers an exact base
    // region even though the derived extent is still unknown.
    std::vector<uint8_t> info_win_storage(sizeof(InfoWin) + 32);
    std::vector<uint8_t> info_win_expected(info_win_storage.size());
    auto *info_win = reinterpret_cast<InfoWin *>(info_win_storage.data() + 16);
    seed_storage(info_win_storage.data(), info_win_expected.data(), info_win_storage.size());
    std::memcpy(info_win_expected.data(), info_win_storage.data(), info_win_storage.size());
    info_win->UNK1();
    info_win->on_redraw();
    info_win_unk1_redirect(info_win, nullptr);
    info_win_on_redraw_redirect(info_win, nullptr);
    expect_storage_bytes(info_win_storage.data(), info_win_expected.data(),
                         info_win_storage.size());
    std::vector<uint8_t> status_win_storage(sizeof(StatusWin) + 32);
    std::vector<uint8_t> status_win_expected(status_win_storage.size());
    auto *status_win = reinterpret_cast<StatusWin *>(status_win_storage.data() + 16);
    seed_storage(status_win_storage.data(), status_win_expected.data(), status_win_storage.size());
    std::memcpy(status_win_expected.data(), status_win_storage.data(), status_win_storage.size());
    status_win->close();
    status_win_close_redirect(status_win, nullptr);
    expect_storage_bytes(status_win_storage.data(), status_win_expected.data(),
                         status_win_storage.size());
    std::vector<uint8_t> file_win_storage(sizeof(FileWin) + 32);
    std::vector<uint8_t> file_win_expected(file_win_storage.size());
    auto *file_win = reinterpret_cast<FileWin *>(file_win_storage.data() + 16);
    seed_storage(file_win_storage.data(), file_win_expected.data(), file_win_storage.size());
    std::memcpy(file_win_expected.data(), file_win_storage.data(), file_win_storage.size());
    file_win->UNK6();
    file_win_unk6_redirect(file_win, nullptr);
    expect_storage_bytes(file_win_storage.data(), file_win_expected.data(),
                         file_win_storage.size());

    // Wave::set_asdr is the only member of this sweep that returns a value
    // rather than nothing: a single constant load of 11, which is the part a
    // constant-return stub most easily gets wrong.
    std::vector<uint8_t> asdr_storage(sizeof(Wave) + 32);
    std::vector<uint8_t> asdr_expected(asdr_storage.size());
    auto *wave = reinterpret_cast<Wave *>(asdr_storage.data() + 16);
    seed_storage(asdr_storage.data(), asdr_expected.data(), asdr_storage.size());
    std::memcpy(asdr_expected.data(), asdr_storage.data(), asdr_storage.size());
    expect(wave->set_asdr() == 11);
    expect(wave_set_asdr_redirect(wave, nullptr) == 11);
    expect_storage_bytes(asdr_storage.data(), asdr_expected.data(),
                         asdr_storage.size());

    // Both class-level closers are static and __cdecl in the original, so
    // they take no instance at all and there is nothing to canary.
    CheckButton::close_class();
    Cursor::close_cursor_class();
    check_button_close_class_redirect();
    cursor_close_cursor_class_redirect();

    // Fifty-six stubs that take arguments and ignore them. Every call below
    // passes deliberately hostile values - INT_MIN, INT_MAX, negatives - so a
    // body that actually read an argument, or that returned one rather than
    // its constant, would show up rather than coincide with the expected
    // answer. The canary catches anything written; the return check catches
    // anything computed.
    std::vector<uint8_t> alpha_movie_c_storage(sizeof(AlphaMovie) + 32);
    std::vector<uint8_t> alpha_movie_c_expected(alpha_movie_c_storage.size());
    auto *alpha_movie_c = reinterpret_cast<AlphaMovie *>(alpha_movie_c_storage.data() + 16);
    seed_storage(alpha_movie_c_storage.data(), alpha_movie_c_expected.data(), alpha_movie_c_storage.size());
    std::memcpy(alpha_movie_c_expected.data(), alpha_movie_c_storage.data(), alpha_movie_c_storage.size());
    expect(alpha_movie_c->UNK2(-1, 2147483647, -2147483648) == 0);
    expect(alpha_movie_unk2_redirect(alpha_movie_c, nullptr, -1, 2147483647, -2147483648) == 0);
    expect(alpha_movie_c->UNK4(-1, 2147483647, -2147483648, 3) == 0);
    expect(alpha_movie_unk4_redirect(alpha_movie_c, nullptr, -1, 2147483647, -2147483648, 3) == 0);
    expect(alpha_movie_c->UNK5(-1) == 0);
    expect(alpha_movie_unk5_redirect(alpha_movie_c, nullptr, -1) == 0);
    // UNK3 is the one member of this family that is NOT a constant return: it
    // builds a frame and returns its own argument. UNK5 beside it has the
    // identical signature and returns zero, so a fixture passing 0 or -1 only
    // would pass for both bodies and prove nothing about which one was
    // written. These values are chosen to separate them - and to separate
    // "returns the argument" from "returns the argument truncated or
    // sign-extended", which is why both signed extremes appear.
    expect(alpha_movie_c->UNK3(-1) == -1);
    expect(alpha_movie_unk3_redirect(alpha_movie_c, nullptr, -1) == -1);
    expect(alpha_movie_c->UNK3(2147483647) == 2147483647);
    expect(alpha_movie_unk3_redirect(alpha_movie_c, nullptr, 2147483647) == 2147483647);
    expect(alpha_movie_c->UNK3(-2147483647 - 1) == -2147483647 - 1);
    expect(alpha_movie_unk3_redirect(alpha_movie_c, nullptr, -2147483647 - 1)
           == -2147483647 - 1);
    expect(alpha_movie_c->UNK3(0) == 0);
    expect(alpha_movie_unk3_redirect(alpha_movie_c, nullptr, 0) == 0);
    alpha_movie_c->UNK6(-1, 2147483647);
    alpha_movie_unk6_00404260_redirect(alpha_movie_c, nullptr, -1, 2147483647);
    alpha_movie_c->UNK6(-1);
    alpha_movie_unk6_00404270_redirect(alpha_movie_c, nullptr, -1);
    expect(alpha_movie_c->UNK8(-1, 2147483647) == 1);
    expect(alpha_movie_unk8_redirect(alpha_movie_c, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(alpha_movie_c_storage.data(), alpha_movie_c_expected.data(),
                         alpha_movie_c_storage.size());
    std::vector<uint8_t> base_win_c_storage(sizeof(BaseWin) + 32);
    std::vector<uint8_t> base_win_c_expected(base_win_c_storage.size());
    auto *base_win_c = reinterpret_cast<BaseWin *>(base_win_c_storage.data() + 16);
    seed_storage(base_win_c_storage.data(), base_win_c_expected.data(), base_win_c_storage.size());
    std::memcpy(base_win_c_expected.data(), base_win_c_storage.data(), base_win_c_storage.size());
    base_win_c->UNK5(-1);
    base_win_unk5_redirect(base_win_c, nullptr, -1);
    base_win_c->on_button_toggled(-1, 2147483647);
    base_win_on_button_toggled_redirect(base_win_c, nullptr, -1, 2147483647);
    base_win_c->on_iface_right_down(-1, 2147483647);
    base_win_on_iface_right_down_redirect(base_win_c, nullptr, -1, 2147483647);
    base_win_c->on_iface_selected(-1, 2147483647);
    base_win_on_iface_selected_redirect(base_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(base_win_c_storage.data(), base_win_c_expected.data(),
                         base_win_c_storage.size());
    std::vector<uint8_t> datalink_c_storage(sizeof(Datalink) + 32);
    std::vector<uint8_t> datalink_c_expected(datalink_c_storage.size());
    auto *datalink_c = reinterpret_cast<Datalink *>(datalink_c_storage.data() + 16);
    seed_storage(datalink_c_storage.data(), datalink_c_expected.data(), datalink_c_storage.size());
    std::memcpy(datalink_c_expected.data(), datalink_c_storage.data(), datalink_c_storage.size());
    datalink_c->on_left_click(-1, 2147483647);
    datalink_on_left_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_group_clicked(-1, 2147483647);
    datalink_on_group_clicked_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_mouse_move(-1, 2147483647);
    datalink_on_mouse_move_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_click(-1, 2147483647);
    datalink_on_iface_left_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_click(-1, 2147483647);
    datalink_on_iface_right_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_down(-1, 2147483647);
    datalink_on_iface_left_down_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_down(-1, 2147483647);
    datalink_on_iface_right_down_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_mouse_move(-1, 2147483647);
    datalink_on_iface_mouse_move_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_mouse_leave(-1, 2147483647);
    datalink_on_iface_mouse_leave_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_selected(-1, 2147483647);
    datalink_on_iface_selected_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_left_double_click(-1, 2147483647);
    datalink_on_iface_left_double_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_right_double_click(-1, 2147483647);
    datalink_on_iface_right_double_click_redirect(datalink_c, nullptr, -1, 2147483647);
    datalink_c->on_iface_button_clicked(-1);
    datalink_on_iface_button_clicked_redirect(datalink_c, nullptr, -1);
    datalink_c->on_iface_button_toggled(-1, 2147483647);
    datalink_on_iface_button_toggled_redirect(datalink_c, nullptr, -1, 2147483647);
    expect_storage_bytes(datalink_c_storage.data(), datalink_c_expected.data(),
                         datalink_c_storage.size());
    std::vector<uint8_t> design_win_c_storage(sizeof(DesignWin) + 32);
    std::vector<uint8_t> design_win_c_expected(design_win_c_storage.size());
    auto *design_win_c = reinterpret_cast<DesignWin *>(design_win_c_storage.data() + 16);
    seed_storage(design_win_c_storage.data(), design_win_c_expected.data(), design_win_c_storage.size());
    std::memcpy(design_win_c_expected.data(), design_win_c_storage.data(), design_win_c_storage.size());
    design_win_c->on_iface_left_down(-1, 2147483647);
    design_win_on_iface_left_down_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_right_down(-1, 2147483647);
    design_win_on_iface_right_down_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_selected(-1, 2147483647);
    design_win_on_iface_selected_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_left_double_click(-1, 2147483647);
    design_win_on_iface_left_double_click_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_right_double_click(-1, 2147483647);
    design_win_on_iface_right_double_click_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_button_toggled(-1, 2147483647);
    design_win_on_iface_button_toggled_redirect(design_win_c, nullptr, -1, 2147483647);
    design_win_c->on_iface_group_clicked(-1, 2147483647, -2147483648);
    design_win_on_iface_group_clicked_redirect(design_win_c, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(design_win_c_storage.data(), design_win_c_expected.data(),
                         design_win_c_storage.size());
    std::vector<uint8_t> diplo_win_c_storage(sizeof(DiploWin) + 32);
    std::vector<uint8_t> diplo_win_c_expected(diplo_win_c_storage.size());
    auto *diplo_win_c = reinterpret_cast<DiploWin *>(diplo_win_c_storage.data() + 16);
    seed_storage(diplo_win_c_storage.data(), diplo_win_c_expected.data(), diplo_win_c_storage.size());
    std::memcpy(diplo_win_c_expected.data(), diplo_win_c_storage.data(), diplo_win_c_storage.size());
    diplo_win_c->UNK4(-1);
    diplo_win_unk4_redirect(diplo_win_c, nullptr, -1);
    diplo_win_c->on_iface_left_click(-1, 2147483647);
    diplo_win_on_iface_left_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_up(-1, 2147483647);
    diplo_win_on_iface_left_up_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_click(-1, 2147483647);
    diplo_win_on_iface_right_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_down(-1, 2147483647);
    diplo_win_on_iface_left_down_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_down(-1, 2147483647);
    diplo_win_on_iface_right_down_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_mouse_move(-1, 2147483647);
    diplo_win_on_iface_mouse_move_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_mouse_leave(-1, 2147483647);
    diplo_win_on_iface_mouse_leave_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_selected(-1, 2147483647);
    diplo_win_on_iface_selected_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_left_double_click(-1, 2147483647);
    diplo_win_on_iface_left_double_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_right_double_click(-1, 2147483647);
    diplo_win_on_iface_right_double_click_redirect(diplo_win_c, nullptr, -1, 2147483647);
    diplo_win_c->on_iface_button_clicked(-1);
    diplo_win_on_iface_button_clicked_redirect(diplo_win_c, nullptr, -1);
    diplo_win_c->on_iface_button_toggled(-1, 2147483647);
    diplo_win_on_iface_button_toggled_redirect(diplo_win_c, nullptr, -1, 2147483647);
    expect(diplo_win_c->on_iface_dialog_back_draw(-1) == 0);
    expect(diplo_win_on_iface_dialog_back_draw_redirect(diplo_win_c, nullptr, -1) == 0);
    expect_storage_bytes(diplo_win_c_storage.data(), diplo_win_c_expected.data(),
                         diplo_win_c_storage.size());
    std::vector<uint8_t> net_win_c_storage(sizeof(NetWin) + 32);
    std::vector<uint8_t> net_win_c_expected(net_win_c_storage.size());
    auto *net_win_c = reinterpret_cast<NetWin *>(net_win_c_storage.data() + 16);
    seed_storage(net_win_c_storage.data(), net_win_c_expected.data(), net_win_c_storage.size());
    std::memcpy(net_win_c_expected.data(), net_win_c_storage.data(), net_win_c_storage.size());
    expect(net_win_c->on_key_click(-1, 2147483647) == 1);
    expect(net_win_on_key_click_redirect(net_win_c, nullptr, -1, 2147483647) == 1);
    net_win_c->on_mouse_move(-1, 2147483647);
    net_win_on_mouse_move_redirect(net_win_c, nullptr, -1, 2147483647);
    net_win_c->on_mouse_leave(-1, 2147483647);
    net_win_on_mouse_leave_redirect(net_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(net_win_c_storage.data(), net_win_c_expected.data(),
                         net_win_c_storage.size());
    std::vector<uint8_t> social_win_c_storage(sizeof(SocialWin) + 32);
    std::vector<uint8_t> social_win_c_expected(social_win_c_storage.size());
    auto *social_win_c = reinterpret_cast<SocialWin *>(social_win_c_storage.data() + 16);
    seed_storage(social_win_c_storage.data(), social_win_c_expected.data(), social_win_c_storage.size());
    std::memcpy(social_win_c_expected.data(), social_win_c_storage.data(), social_win_c_storage.size());
    social_win_c->on_iface_left_down(-1, 2147483647);
    social_win_on_iface_left_down_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_right_down(-1, 2147483647);
    social_win_on_iface_right_down_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_selected(-1, 2147483647);
    social_win_on_iface_selected_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_left_double_click(-1, 2147483647);
    social_win_on_iface_left_double_click_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_right_double_click(-1, 2147483647);
    social_win_on_iface_right_double_click_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_button_toggled(-1, 2147483647);
    social_win_on_iface_button_toggled_redirect(social_win_c, nullptr, -1, 2147483647);
    social_win_c->on_iface_group_clicked(-1, 2147483647, -2147483648);
    social_win_on_iface_group_clicked_redirect(social_win_c, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(social_win_c_storage.data(), social_win_c_expected.data(),
                         social_win_c_storage.size());
    std::vector<uint8_t> world_win_c_storage(sizeof(WorldWin) + 32);
    std::vector<uint8_t> world_win_c_expected(world_win_c_storage.size());
    auto *world_win_c = reinterpret_cast<WorldWin *>(world_win_c_storage.data() + 16);
    seed_storage(world_win_c_storage.data(), world_win_c_expected.data(), world_win_c_storage.size());
    std::memcpy(world_win_c_expected.data(), world_win_c_storage.data(), world_win_c_storage.size());
    world_win_c->on_left_double_click(-1, 2147483647);
    world_win_on_left_double_click_redirect(world_win_c, nullptr, -1, 2147483647);
    expect_storage_bytes(world_win_c_storage.data(), world_win_c_expected.data(),
                         world_win_c_storage.size());

    // Eleven more classes, each over a base already pinned here, so every
    // canary below covers an exact base region. Same hostile arguments as
    // above.
    std::vector<uint8_t> battle_win_k_storage(sizeof(BattleWin) + 32);
    std::vector<uint8_t> battle_win_k_expected(battle_win_k_storage.size());
    auto *battle_win_k = reinterpret_cast<BattleWin *>(battle_win_k_storage.data() + 16);
    seed_storage(battle_win_k_storage.data(), battle_win_k_expected.data(), battle_win_k_storage.size());
    std::memcpy(battle_win_k_expected.data(), battle_win_k_storage.data(), battle_win_k_storage.size());
    battle_win_k->on_iface_left_click(-1, 2147483647);
    battle_win_on_iface_left_click_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_right_click(-1, 2147483647);
    battle_win_on_iface_right_click_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_right_down(-1, 2147483647);
    battle_win_on_iface_right_down_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_mouse_move(-1, 2147483647);
    battle_win_on_iface_mouse_move_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_mouse_leave(-1, 2147483647);
    battle_win_on_iface_mouse_leave_redirect(battle_win_k, nullptr, -1, 2147483647);
    battle_win_k->on_iface_selected(-1, 2147483647);
    battle_win_on_iface_selected_redirect(battle_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(battle_win_k_storage.data(), battle_win_k_expected.data(),
                         battle_win_k_storage.size());
    std::vector<uint8_t> counc_win_k_storage(sizeof(CouncWin) + 32);
    std::vector<uint8_t> counc_win_k_expected(counc_win_k_storage.size());
    auto *counc_win_k = reinterpret_cast<CouncWin *>(counc_win_k_storage.data() + 16);
    seed_storage(counc_win_k_storage.data(), counc_win_k_expected.data(), counc_win_k_storage.size());
    std::memcpy(counc_win_k_expected.data(), counc_win_k_storage.data(), counc_win_k_storage.size());
    counc_win_k->on_group_clicked(-1, 2147483647);
    counc_win_on_group_clicked_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_button_passover(-1, 2147483647);
    counc_win_on_button_passover_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_click(-1, 2147483647);
    counc_win_on_iface_left_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_click(-1, 2147483647);
    counc_win_on_iface_right_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_down(-1, 2147483647);
    counc_win_on_iface_left_down_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_down(-1, 2147483647);
    counc_win_on_iface_right_down_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_mouse_move(-1, 2147483647);
    counc_win_on_iface_mouse_move_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_mouse_leave(-1, 2147483647);
    counc_win_on_iface_mouse_leave_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_selected(-1, 2147483647);
    counc_win_on_iface_selected_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_left_double_click(-1, 2147483647);
    counc_win_on_iface_left_double_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_right_double_click(-1, 2147483647);
    counc_win_on_iface_right_double_click_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_button_toggled(-1, 2147483647);
    counc_win_on_iface_button_toggled_redirect(counc_win_k, nullptr, -1, 2147483647);
    counc_win_k->on_iface_group_clicked(-1, 2147483647, -2147483648);
    counc_win_on_iface_group_clicked_redirect(counc_win_k, nullptr, -1, 2147483647, -2147483648);
    expect(counc_win_k->on_button_text_draw(-1, 2147483647, -2147483648) == 0);
    expect(counc_win_on_button_text_draw_redirect(
               counc_win_k, nullptr, -1, 2147483647, -2147483648) == 0);
    expect_storage_bytes(counc_win_k_storage.data(), counc_win_k_expected.data(),
                         counc_win_k_storage.size());
    std::vector<uint8_t> credits_k_storage(sizeof(Credits) + 32);
    std::vector<uint8_t> credits_k_expected(credits_k_storage.size());
    auto *credits_k = reinterpret_cast<Credits *>(credits_k_storage.data() + 16);
    seed_storage(credits_k_storage.data(), credits_k_expected.data(), credits_k_storage.size());
    std::memcpy(credits_k_expected.data(), credits_k_storage.data(), credits_k_storage.size());
    credits_k->on_left_down(-1, 2147483647);
    credits_on_left_down_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_right_down(-1, 2147483647);
    credits_on_right_down_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_mouse_move(-1, 2147483647);
    credits_on_mouse_move_redirect(credits_k, nullptr, -1, 2147483647);
    credits_k->on_button_clicked(-1);
    credits_on_button_clicked_redirect(credits_k, nullptr, -1);
    expect_storage_bytes(credits_k_storage.data(), credits_k_expected.data(),
                         credits_k_storage.size());
    std::vector<uint8_t> diplo_pop_k_storage(sizeof(DiploPop) + 32);
    std::vector<uint8_t> diplo_pop_k_expected(diplo_pop_k_storage.size());
    auto *diplo_pop_k = reinterpret_cast<DiploPop *>(diplo_pop_k_storage.data() + 16);
    seed_storage(diplo_pop_k_storage.data(), diplo_pop_k_expected.data(), diplo_pop_k_storage.size());
    std::memcpy(diplo_pop_k_expected.data(), diplo_pop_k_storage.data(), diplo_pop_k_storage.size());
    diplo_pop_k->on_iface_left_click(-1, 2147483647);
    diplo_pop_on_iface_left_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_click(-1, 2147483647);
    diplo_pop_on_iface_right_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_left_down(-1, 2147483647);
    diplo_pop_on_iface_left_down_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_down(-1, 2147483647);
    diplo_pop_on_iface_right_down_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_mouse_move(-1, 2147483647);
    diplo_pop_on_iface_mouse_move_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_mouse_leave(-1, 2147483647);
    diplo_pop_on_iface_mouse_leave_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_selected(-1, 2147483647);
    diplo_pop_on_iface_selected_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_left_double_click(-1, 2147483647);
    diplo_pop_on_iface_left_double_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_right_double_click(-1, 2147483647);
    diplo_pop_on_iface_right_double_click_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    diplo_pop_k->on_iface_button_toggled(-1, 2147483647);
    diplo_pop_on_iface_button_toggled_redirect(diplo_pop_k, nullptr, -1, 2147483647);
    expect_storage_bytes(diplo_pop_k_storage.data(), diplo_pop_k_expected.data(),
                         diplo_pop_k_storage.size());
    std::vector<uint8_t> gamma_k_storage(sizeof(Gamma) + 32);
    std::vector<uint8_t> gamma_k_expected(gamma_k_storage.size());
    auto *gamma_k = reinterpret_cast<Gamma *>(gamma_k_storage.data() + 16);
    seed_storage(gamma_k_storage.data(), gamma_k_expected.data(), gamma_k_storage.size());
    std::memcpy(gamma_k_expected.data(), gamma_k_storage.data(), gamma_k_storage.size());
    expect(gamma_k->on_key_click(-1, 2147483647) == 1);
    expect(gamma_on_key_click_redirect(gamma_k, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(gamma_k_storage.data(), gamma_k_expected.data(),
                         gamma_k_storage.size());
    std::vector<uint8_t> message_win_k_storage(sizeof(MessageWin) + 32);
    std::vector<uint8_t> message_win_k_expected(message_win_k_storage.size());
    auto *message_win_k = reinterpret_cast<MessageWin *>(message_win_k_storage.data() + 16);
    seed_storage(message_win_k_storage.data(), message_win_k_expected.data(), message_win_k_storage.size());
    std::memcpy(message_win_k_expected.data(), message_win_k_storage.data(), message_win_k_storage.size());
    message_win_k->UNK1(-1, 2147483647, -2147483648, 3, -4);
    message_win_unk1_redirect(message_win_k, nullptr, -1, 2147483647, -2147483648, 3, -4);
    message_win_k->UNK4(-1);
    message_win_unk4_redirect(message_win_k, nullptr, -1);
    expect_storage_bytes(message_win_k_storage.data(), message_win_k_expected.data(),
                         message_win_k_storage.size());
    std::vector<uint8_t> new_tech_win_k_storage(sizeof(NewTechWin) + 32);
    std::vector<uint8_t> new_tech_win_k_expected(new_tech_win_k_storage.size());
    auto *new_tech_win_k = reinterpret_cast<NewTechWin *>(new_tech_win_k_storage.data() + 16);
    seed_storage(new_tech_win_k_storage.data(), new_tech_win_k_expected.data(), new_tech_win_k_storage.size());
    std::memcpy(new_tech_win_k_expected.data(), new_tech_win_k_storage.data(), new_tech_win_k_storage.size());
    new_tech_win_k->on_left_click(-1, 2147483647);
    new_tech_win_on_left_click_redirect(new_tech_win_k, nullptr, -1, 2147483647);
    expect(new_tech_win_k->on_key_click(-1, 2147483647) == 1);
    expect(new_tech_win_on_key_click_redirect(new_tech_win_k, nullptr, -1, 2147483647) == 1);
    expect_storage_bytes(new_tech_win_k_storage.data(), new_tech_win_k_expected.data(),
                         new_tech_win_k_storage.size());
    std::vector<uint8_t> pick_win_k_storage(sizeof(PickWin) + 32);
    std::vector<uint8_t> pick_win_k_expected(pick_win_k_storage.size());
    auto *pick_win_k = reinterpret_cast<PickWin *>(pick_win_k_storage.data() + 16);
    seed_storage(pick_win_k_storage.data(), pick_win_k_expected.data(), pick_win_k_storage.size());
    std::memcpy(pick_win_k_expected.data(), pick_win_k_storage.data(), pick_win_k_storage.size());
    pick_win_k->on_button_passover(-1, 2147483647);
    pick_win_on_button_passover_redirect(pick_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(pick_win_k_storage.data(), pick_win_k_expected.data(),
                         pick_win_k_storage.size());
    std::vector<uint8_t> prod_picker_k_storage(sizeof(ProdPicker) + 32);
    std::vector<uint8_t> prod_picker_k_expected(prod_picker_k_storage.size());
    auto *prod_picker_k = reinterpret_cast<ProdPicker *>(prod_picker_k_storage.data() + 16);
    seed_storage(prod_picker_k_storage.data(), prod_picker_k_expected.data(), prod_picker_k_storage.size());
    std::memcpy(prod_picker_k_expected.data(), prod_picker_k_storage.data(), prod_picker_k_storage.size());
    expect(prod_picker_k->UNK1(-1) == 1);
    expect(prod_picker_unk1_redirect(prod_picker_k, nullptr, -1) == 1);
    prod_picker_k->on_mouse_move(-1, 2147483647);
    prod_picker_on_mouse_move_redirect(prod_picker_k, nullptr, -1, 2147483647);
    prod_picker_k->UNK3(-1);
    prod_picker_unk3_redirect(prod_picker_k, nullptr, -1);
    expect_storage_bytes(prod_picker_k_storage.data(), prod_picker_k_expected.data(),
                         prod_picker_k_storage.size());
    std::vector<uint8_t> report_win_k_storage(sizeof(ReportWin) + 32);
    std::vector<uint8_t> report_win_k_expected(report_win_k_storage.size());
    auto *report_win_k = reinterpret_cast<ReportWin *>(report_win_k_storage.data() + 16);
    seed_storage(report_win_k_storage.data(), report_win_k_expected.data(), report_win_k_storage.size());
    std::memcpy(report_win_k_expected.data(), report_win_k_storage.data(), report_win_k_storage.size());
    report_win_k->on_mouse_move(-1, 2147483647);
    report_win_on_mouse_move_redirect(report_win_k, nullptr, -1, 2147483647);
    report_win_k->on_mouse_leave(-1, 2147483647);
    report_win_on_mouse_leave_redirect(report_win_k, nullptr, -1, 2147483647);
    expect_storage_bytes(report_win_k_storage.data(), report_win_k_expected.data(),
                         report_win_k_storage.size());
    std::vector<uint8_t> setup_win_k_storage(sizeof(SetupWin) + 32);
    std::vector<uint8_t> setup_win_k_expected(setup_win_k_storage.size());
    auto *setup_win_k = reinterpret_cast<SetupWin *>(setup_win_k_storage.data() + 16);
    seed_storage(setup_win_k_storage.data(), setup_win_k_expected.data(), setup_win_k_storage.size());
    std::memcpy(setup_win_k_expected.data(), setup_win_k_storage.data(), setup_win_k_storage.size());
    expect(setup_win_k->UNK3(-1) == 0);
    expect(setup_win_unk3_redirect(setup_win_k, nullptr, -1) == 0);
    expect(setup_win_k->UNK4(-1) == 0);
    expect(setup_win_unk4_redirect(setup_win_k, nullptr, -1) == 0);
    expect_storage_bytes(setup_win_k_storage.data(), setup_win_k_expected.data(),
                         setup_win_k_storage.size());

    // Thirty-seven more, into classes that already had headers, so these went
    // into existing public sections rather than generated files. The calls are
    // generated from the emitted definitions themselves. Deriving them instead
    // by matching redirect names back to methods, as a first attempt did,
    // matches case-insensitively onto pre-existing methods of the same name -
    // which crashed under Wine calling a real one with a seeded `this`.
    // Several take unsigned parameters, occupying the same four-byte stack
    // slot as an int and fed 0xFFFFFFFF for the same reason.
    std::vector<uint8_t> caviar_z_storage(sizeof(Caviar) + 32);
    std::vector<uint8_t> caviar_z_expected(caviar_z_storage.size());
    auto *caviar_z = reinterpret_cast<Caviar *>(caviar_z_storage.data() + 16);
    seed_storage(caviar_z_storage.data(), caviar_z_expected.data(), caviar_z_storage.size());
    std::memcpy(caviar_z_expected.data(), caviar_z_storage.data(), caviar_z_storage.size());
    caviar_z->UNK12(-1, 2147483647, -2147483648);
    caviar_unk12_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648);
    expect(caviar_z->UNK3(-1, 2147483647, -2147483648) == 0);
    expect(caviar_unk3_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648) == 0);
    expect(caviar_z->UNK4(-1, 2147483647, -2147483648, 3) == 0);
    expect(caviar_unk4_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648, 3) == 0);
    caviar_z->UNK5(-1, 2147483647);
    caviar_unk5_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK6(-1, 2147483647);
    caviar_unk6_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK7(-1, 2147483647);
    caviar_unk7_redirect(caviar_z, nullptr, -1, 2147483647);
    caviar_z->UNK9(-1, 2147483647, -2147483648);
    caviar_unk9_redirect(caviar_z, nullptr, -1, 2147483647, -2147483648);
    expect_storage_bytes(caviar_z_storage.data(), caviar_z_expected.data(),
                         caviar_z_storage.size());
    std::vector<uint8_t> file_win_z_storage(sizeof(FileWin) + 32);
    std::vector<uint8_t> file_win_z_expected(file_win_z_storage.size());
    auto *file_win_z = reinterpret_cast<FileWin *>(file_win_z_storage.data() + 16);
    seed_storage(file_win_z_storage.data(), file_win_z_expected.data(), file_win_z_storage.size());
    std::memcpy(file_win_z_expected.data(), file_win_z_storage.data(), file_win_z_storage.size());
    file_win_z->UNK2(-1);
    file_win_unk2_redirect(file_win_z, nullptr, -1);
    expect_storage_bytes(file_win_z_storage.data(), file_win_z_expected.data(),
                         file_win_z_storage.size());
    std::vector<uint8_t> info_win_z_storage(sizeof(InfoWin) + 32);
    std::vector<uint8_t> info_win_z_expected(info_win_z_storage.size());
    auto *info_win_z = reinterpret_cast<InfoWin *>(info_win_z_storage.data() + 16);
    seed_storage(info_win_z_storage.data(), info_win_z_expected.data(), info_win_z_storage.size());
    std::memcpy(info_win_z_expected.data(), info_win_z_storage.data(), info_win_z_storage.size());
    info_win_z->UNK3(-1, 2147483647);
    info_win_unk3_redirect(info_win_z, nullptr, -1, 2147483647);
    info_win_z->change(-1);
    info_win_change_redirect(info_win_z, nullptr, -1);
    expect_storage_bytes(info_win_z_storage.data(), info_win_z_expected.data(),
                         info_win_z_storage.size());
    std::vector<uint8_t> main_interface_z_storage(sizeof(MainInterface) + 32);
    std::vector<uint8_t> main_interface_z_expected(main_interface_z_storage.size());
    auto *main_interface_z = reinterpret_cast<MainInterface *>(main_interface_z_storage.data() + 16);
    seed_storage(main_interface_z_storage.data(), main_interface_z_expected.data(), main_interface_z_storage.size());
    std::memcpy(main_interface_z_expected.data(), main_interface_z_storage.data(), main_interface_z_storage.size());
    main_interface_z->UNK1(-1);
    main_interface_unk1_redirect(main_interface_z, nullptr, -1);
    expect(main_interface_z->on_iface_button_pressed(-1) == 0);
    expect(main_interface_on_iface_button_pressed_redirect(main_interface_z, nullptr, -1) == 0);
    main_interface_z->on_iface_button_toggled(-1, 2147483647);
    main_interface_on_iface_button_toggled_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_right_double_click(-1, 2147483647);
    main_interface_on_iface_right_double_click_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_scrolled(-1, 2147483647);
    main_interface_on_iface_scrolled_redirect(main_interface_z, nullptr, -1, 2147483647);
    main_interface_z->on_iface_scrolling(-1, 2147483647);
    main_interface_on_iface_scrolling_redirect(main_interface_z, nullptr, -1, 2147483647);
    expect_storage_bytes(main_interface_z_storage.data(), main_interface_z_expected.data(),
                         main_interface_z_storage.size());
    std::vector<uint8_t> midi_device_z_storage(sizeof(Midi_Device) + 32);
    std::vector<uint8_t> midi_device_z_expected(midi_device_z_storage.size());
    auto *midi_device_z = reinterpret_cast<Midi_Device *>(midi_device_z_storage.data() + 16);
    seed_storage(midi_device_z_storage.data(), midi_device_z_expected.data(), midi_device_z_storage.size());
    std::memcpy(midi_device_z_expected.data(), midi_device_z_storage.data(), midi_device_z_storage.size());
    expect(midi_device_z->fade(4294967295u) == 0);
    expect(midi_device_fade_redirect(midi_device_z, nullptr, 4294967295u) == 0);
    expect(midi_device_z->select(4294967295u) == 0);
    expect(midi_device_select_redirect(midi_device_z, nullptr, 4294967295u) == 0);
    midi_device_z->set_pan(-1);
    midi_device_set_pan_redirect(midi_device_z, nullptr, -1);
    midi_device_z->set_rate(4294967295u);
    midi_device_set_rate_redirect(midi_device_z, nullptr, 4294967295u);
    midi_device_z->set_volume(4294967295u);
    midi_device_set_volume_redirect(midi_device_z, nullptr, 4294967295u);
    expect_storage_bytes(midi_device_z_storage.data(), midi_device_z_expected.data(),
                         midi_device_z_storage.size());
    std::vector<uint8_t> pull_down_z_storage(sizeof(PullDown) + 32);
    std::vector<uint8_t> pull_down_z_expected(pull_down_z_storage.size());
    auto *pull_down_z = reinterpret_cast<PullDown *>(pull_down_z_storage.data() + 16);
    seed_storage(pull_down_z_storage.data(), pull_down_z_expected.data(), pull_down_z_storage.size());
    std::memcpy(pull_down_z_expected.data(), pull_down_z_storage.data(), pull_down_z_storage.size());
    pull_down_z->on_mouse_leave(-1, 2147483647);
    pull_down_on_mouse_leave_redirect(pull_down_z, nullptr, -1, 2147483647);
    expect_storage_bytes(pull_down_z_storage.data(), pull_down_z_expected.data(),
                         pull_down_z_storage.size());
    std::vector<uint8_t> scroll_z_storage(sizeof(Scroll) + 32);
    std::vector<uint8_t> scroll_z_expected(scroll_z_storage.size());
    auto *scroll_z = reinterpret_cast<Scroll *>(scroll_z_storage.data() + 16);
    seed_storage(scroll_z_storage.data(), scroll_z_expected.data(), scroll_z_storage.size());
    std::memcpy(scroll_z_expected.data(), scroll_z_storage.data(), scroll_z_storage.size());
    scroll_z->on_left_click(-1, 2147483647);
    scroll_on_left_click_redirect(scroll_z, nullptr, -1, 2147483647);
    expect_storage_bytes(scroll_z_storage.data(), scroll_z_expected.data(),
                         scroll_z_storage.size());
    std::vector<uint8_t> sprite_z_storage(sizeof(Sprite) + 32);
    std::vector<uint8_t> sprite_z_expected(sprite_z_storage.size());
    auto *sprite_z = reinterpret_cast<Sprite *>(sprite_z_storage.data() + 16);
    seed_storage(sprite_z_storage.data(), sprite_z_expected.data(), sprite_z_storage.size());
    std::memcpy(sprite_z_expected.data(), sprite_z_storage.data(), sprite_z_storage.size());
    sprite_z->UNK3(-1, 2147483647);
    sprite_unk3_redirect(sprite_z, nullptr, -1, 2147483647);
    sprite_z->UNK4(-1, 2147483647);
    sprite_unk4_redirect(sprite_z, nullptr, -1, 2147483647);
    expect_storage_bytes(sprite_z_storage.data(), sprite_z_expected.data(),
                         sprite_z_storage.size());
    std::vector<uint8_t> wave_z_storage(sizeof(Wave) + 32);
    std::vector<uint8_t> wave_z_expected(wave_z_storage.size());
    auto *wave_z = reinterpret_cast<Wave *>(wave_z_storage.data() + 16);
    seed_storage(wave_z_storage.data(), wave_z_expected.data(), wave_z_storage.size());
    std::memcpy(wave_z_expected.data(), wave_z_storage.data(), wave_z_storage.size());
    expect(wave_z->set_attack(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_attack_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_bufflimit(4294967295u) == 11);
    expect(wave_set_bufflimit_redirect(wave_z, nullptr, 4294967295u) == 11);
    expect(wave_z->set_decay(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_decay_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_release(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_release_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_z->set_sustain(4294967295u, 4294967295u, 4294967295u) == 11);
    expect(wave_set_sustain_redirect(wave_z, nullptr, 4294967295u, 4294967295u, 4294967295u) == 11);
    expect_storage_bytes(wave_z_storage.data(), wave_z_expected.data(),
                         wave_z_storage.size());
    std::vector<uint8_t> wave_in_device_z_storage(sizeof(Wave_In_Device) + 32);
    std::vector<uint8_t> wave_in_device_z_expected(wave_in_device_z_storage.size());
    auto *wave_in_device_z = reinterpret_cast<Wave_In_Device *>(wave_in_device_z_storage.data() + 16);
    seed_storage(wave_in_device_z_storage.data(), wave_in_device_z_expected.data(), wave_in_device_z_storage.size());
    std::memcpy(wave_in_device_z_expected.data(), wave_in_device_z_storage.data(), wave_in_device_z_storage.size());
    expect(wave_in_device_z->get_caps(4294967295u) == 0);
    expect(wave_in_device_get_caps_redirect(wave_in_device_z, nullptr, 4294967295u) == 0);
    expect(wave_in_device_z->select(4294967295u) == 0);
    expect(wave_in_device_select_redirect(wave_in_device_z, nullptr, 4294967295u) == 0);
    wave_in_device_z->set_rate(4294967295u);
    wave_in_device_set_rate_redirect(wave_in_device_z, nullptr, 4294967295u);
    expect_storage_bytes(wave_in_device_z_storage.data(), wave_in_device_z_expected.data(),
                         wave_in_device_z_storage.size());
    std::vector<uint8_t> win_z_storage(sizeof(Win) + 32);
    std::vector<uint8_t> win_z_expected(win_z_storage.size());
    auto *win_z = reinterpret_cast<Win *>(win_z_storage.data() + 16);
    seed_storage(win_z_storage.data(), win_z_expected.data(), win_z_storage.size());
    std::memcpy(win_z_expected.data(), win_z_storage.data(), win_z_storage.size());
    win_z->on_move(-1, 2147483647);
    win_on_move_redirect(win_z, nullptr, -1, 2147483647);
    win_z->on_size(4294967295u, 2147483647, -2147483648);
    win_on_size_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    win_z->on_size_nc(4294967295u, 2147483647, -2147483648);
    win_on_size_nc_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    win_z->on_sys_command(4294967295u, 2147483647, -2147483648);
    win_on_sys_command_redirect(win_z, nullptr, 4294967295u, 2147483647, -2147483648);
    expect_storage_bytes(win_z_storage.data(), win_z_expected.data(),
                         win_z_storage.size());
}

void test_status_win_set_loc() {
    // The first recovery here that writes fields rather than nothing, so the
    // question is no longer whether memory moved but whether the right bytes
    // moved. Caviar is pinned at 0x13D0 and these four dwords sit at 0x15B4
    // through 0x15C0; the check reads them back at those absolute offsets and
    // compares every other byte of the object against its seed, so a field
    // declared at the wrong offset fails twice over - the value is missing
    // where it belongs and present where it does not.
    std::vector<uint8_t> storage(sizeof(StatusWin) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *status = reinterpret_cast<StatusWin *>(storage.data() + 16);
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());

    status->set_loc(0x11223344, -2);

    auto read_at = [&](size_t offset) {
        int32_t value = 0;
        std::memcpy(&value, storage.data() + 16 + offset, sizeof(value));
        return value;
    };
    expect(read_at(0x15B4) == 0x11223344);
    expect(read_at(0x15B8) == -2);
    expect(read_at(0x15BC) == -1);
    expect(read_at(0x15C0) == -1);

    // Everything outside those sixteen bytes must be untouched.
    std::memcpy(expected.data() + 16 + 0x15B4, storage.data() + 16 + 0x15B4, 16);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The redirect must land identically, including the two constants.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    status_win_set_loc_redirect(status, nullptr, INT_MIN, INT_MAX);
    expect(read_at(0x15B4) == INT_MIN);
    expect(read_at(0x15B8) == INT_MAX);
    expect(read_at(0x15BC) == -1);
    expect(read_at(0x15C0) == -1);
    std::memcpy(expected.data() + 16 + 0x15B4, storage.data() + 16 + 0x15B4, 16);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
}

void test_field_store_clears() {
    // Three recoveries that zero specific fields. As with set_loc, the point
    // is which bytes move: each is checked at the absolute offset the original
    // writes, with every other byte compared against its seed, so a field
    // placed even one byte off fails both ways.
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };
    auto read8 = [](const std::vector<uint8_t> &s, size_t off) {
        return s[16 + off];
    };

    std::vector<uint8_t> md(sizeof(MultiDebug) + 32), md_want(md.size());
    auto *debug = reinterpret_cast<MultiDebug *>(md.data() + 16);
    seed_storage(md.data(), md_want.data(), md.size());
    std::memcpy(md_want.data(), md.data(), md.size());
    debug->close();
    expect(read32(md, 0xA3C) == 0);
    std::memcpy(md_want.data() + 16 + 0xA3C, md.data() + 16 + 0xA3C, 4);
    expect_storage_bytes(md.data(), md_want.data(), md.size());
    multi_debug_close_redirect(debug, nullptr);

    std::vector<uint8_t> dl(sizeof(Datalink) + 32), dl_want(dl.size());
    auto *link = reinterpret_cast<Datalink *>(dl.data() + 16);
    seed_storage(dl.data(), dl_want.data(), dl.size());
    std::memcpy(dl_want.data(), dl.data(), dl.size());
    link->close();
    expect(read32(dl, 0x29E0) == 0);
    expect(read32(dl, 0x2A34) == 0);
    expect(read32(dl, 0x2A38) == 0);
    std::memcpy(dl_want.data() + 16 + 0x29E0, dl.data() + 16 + 0x29E0, 4);
    std::memcpy(dl_want.data() + 16 + 0x2A34, dl.data() + 16 + 0x2A34, 8);
    expect_storage_bytes(dl.data(), dl_want.data(), dl.size());
    datalink_close_redirect(link, nullptr);

    // Four of these five are single bytes, so a field widened to a dword by
    // mistake would clear three neighbours the original leaves alone.
    std::vector<uint8_t> fw(sizeof(FileWin) + 32), fw_want(fw.size());
    auto *files = reinterpret_cast<FileWin *>(fw.data() + 16);
    seed_storage(fw.data(), fw_want.data(), fw.size());
    std::memcpy(fw_want.data(), fw.data(), fw.size());
    files->UNK1();
    expect(read8(fw, 0x208) == 0);
    expect(read8(fw, 0x30C) == 0);
    expect(read8(fw, 0x410) == 0);
    expect(read8(fw, 0x531) == 0);
    expect(read32(fw, 0x514) == 0);
    for (size_t offset : {0x208u, 0x30Cu, 0x410u, 0x531u}) {
        fw_want[16 + offset] = fw[16 + offset];
    }
    std::memcpy(fw_want.data() + 16 + 0x514, fw.data() + 16 + 0x514, 4);
    expect_storage_bytes(fw.data(), fw_want.data(), fw.size());
    file_win_unk1_redirect(files, nullptr);
}

void test_field_store_writes() {
    auto read32 = [](const std::vector<uint8_t> &s, size_t off) {
        int32_t v = 0;
        std::memcpy(&v, s.data() + 16 + off, sizeof(v));
        return v;
    };
    auto keep = [](std::vector<uint8_t> &want, const std::vector<uint8_t> &got,
                   std::initializer_list<size_t> offsets) {
        for (size_t off : offsets) {
            std::memcpy(want.data() + 16 + off, got.data() + 16 + off, 4);
        }
    };

    // Caviar's fields were carved out of an opaque span that the class's
    // pinned 0x13D0 protects: if the carving shifted anything the build fails
    // before this runs. What is left to check is that each value lands where
    // the original puts it.
    std::vector<uint8_t> cv(sizeof(Caviar) + 32), cv_want(cv.size());
    auto *caviar = reinterpret_cast<Caviar *>(cv.data() + 16);
    seed_storage(cv.data(), cv_want.data(), cv.size());
    std::memcpy(cv_want.data(), cv.data(), cv.size());
    caviar->UNK10(INT_MIN, -1, INT_MAX);
    caviar->UNK8(0x5A5A5A5A);
    expect(read32(cv, 0x2C) == INT_MIN);
    expect(read32(cv, 0x30) == -1);
    expect(read32(cv, 0x34) == INT_MAX);
    expect(read32(cv, 0x108) == 0x5A5A5A5A);
    keep(cv_want, cv, {0x2C, 0x30, 0x34, 0x108});
    expect_storage_bytes(cv.data(), cv_want.data(), cv.size());
    caviar_unk10_redirect(caviar, nullptr, 1, 2, 3);
    caviar_unk8_redirect(caviar, nullptr, 4);
    expect(read32(cv, 0x2C) == 1);
    expect(read32(cv, 0x30) == 2);
    expect(read32(cv, 0x34) == 3);
    expect(read32(cv, 0x108) == 4);

    // TutWin::UNK1 splits nine fields between zero and -1, which is the part
    // a transcription most easily gets backwards, and clears a shared marker
    // that lives outside the object entirely.
    std::vector<uint8_t> tw(sizeof(TutWin) + 32), tw_want(tw.size());
    auto *tutorial = reinterpret_cast<TutWin *>(tw.data() + 16);
    seed_storage(tw.data(), tw_want.data(), tw.size());
    std::memcpy(tw_want.data(), tw.data(), tw.size());
    uint32_t marker = 0xDEADBEEF;
    uint32_t *const saved_marker = TutWinShownFlag;
    TutWinShownFlag = &marker;
    tutorial->UNK1();
    expect(marker == 0);
    expect(read32(tw, 0x537C) == -1);
    expect(read32(tw, 0x5380) == -1);
    expect(read32(tw, 0x539C) == -1);
    expect(read32(tw, 0x53AC) == -1);
    expect(read32(tw, 0x53A4) == 0);
    expect(read32(tw, 0x53A8) == 0);
    expect(read32(tw, 0x53B8) == 0);
    expect(read32(tw, 0x53C4) == 0);
    expect(read32(tw, 0x53D4) == 0);
    keep(tw_want, tw, {0x537Cu, 0x5380u, 0x539Cu, 0x53A4u, 0x53A8u,
                       0x53ACu, 0x53B8u, 0x53C4u, 0x53D4u});
    expect_storage_bytes(tw.data(), tw_want.data(), tw.size());

    seed_storage(tw.data(), tw_want.data(), tw.size());
    std::memcpy(tw_want.data(), tw.data(), tw.size());
    tutorial->UNK3(0x11223344);
    expect(read32(tw, 0x53D4) == 0x11223344);
    keep(tw_want, tw, {0x53D4u});
    expect_storage_bytes(tw.data(), tw_want.data(), tw.size());
    tut_win_unk3_redirect(tutorial, nullptr, -9);
    expect(read32(tw, 0x53D4) == -9);
    marker = 1;
    tut_win_unk1_redirect(tutorial, nullptr);
    expect(marker == 0);
    TutWinShownFlag = saved_marker;
}

namespace {

ButtonGroup *g_observed_group = nullptr;

int g_observed_button = 0;

int g_observed_flag_during_call = -1;

int g_click_calls = 0;

int g_click_result = 0;

int __thiscall observe_button_click(ButtonGroup *self, int button_id) {
    g_observed_group = self;
    g_observed_button = button_id;
    // The suppression flag must still be set while the click runs - clearing
    // it before the call rather than after would be invisible to a test that
    // only looked at the object afterwards.
    std::memcpy(&g_observed_flag_during_call,
                reinterpret_cast<const uint8_t *>(self) + 0x90, sizeof(int));
    ++g_click_calls;
    return g_click_result;
}

}  // namespace

void test_button_group_set() {
    // The highest fan-in unrecovered function in the image at 35 callers.
    // button_click itself is 471 bytes of virtual dispatch and stays original
    // for now, reached through a rebindable seam that this test replaces.
    auto *const saved = ButtonGroupOriginalButtonClick;
    ButtonGroupOriginalButtonClick = &observe_button_click;

    std::vector<uint8_t> storage(sizeof(ButtonGroup) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *group = reinterpret_cast<ButtonGroup *>(storage.data() + 16);
    auto flag = [&] {
        int value = 0;
        std::memcpy(&value, storage.data() + 16 + 0x90, sizeof(value));
        return value;
    };

    // notify == 0 suppresses: the flag is raised for the duration and cleared.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_click_calls = 0;
    g_click_result = 0x1234;
    expect(group->set(7, 0) == 0x1234);
    expect(g_click_calls == 1);
    expect(g_observed_group == group);
    expect(g_observed_button == 7);
    expect(g_observed_flag_during_call == 1);
    expect(flag() == 0);
    std::memcpy(expected.data() + 16 + 0x90, storage.data() + 16 + 0x90, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // notify != 0 does not raise it, but still clears it afterwards.
    seed_storage(storage.data(), expected.data(), storage.size());
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_click_calls = 0;
    g_click_result = -1;
    expect(group->set(INT_MIN, 1) == -1);
    expect(g_click_calls == 1);
    expect(g_observed_button == INT_MIN);
    expect(g_observed_flag_during_call != 1);
    expect(flag() == 0);
    std::memcpy(expected.data() + 16 + 0x90, storage.data() + 16 + 0x90, 4);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The redirect must behave identically, including passing the result back.
    g_click_calls = 0;
    g_click_result = 99;
    expect(button_group_set_redirect(group, nullptr, 3, 0) == 99);
    expect(g_click_calls == 1);
    expect(g_observed_flag_during_call == 1);
    expect(flag() == 0);

    ButtonGroupOriginalButtonClick = saved;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(116, test_constant_return_stubs);
LEAF_CASE(182, test_status_win_set_loc);
LEAF_CASE(183, test_field_store_clears);
LEAF_CASE(184, test_field_store_writes);
LEAF_CASE(185, test_button_group_set);
}  // namespace
