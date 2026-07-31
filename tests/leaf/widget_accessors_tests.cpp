// Recovery leaf tests: widget_accessors.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "widget_accessors"

namespace {

void test_texture_store_construct() {
    // Two fields, 3 and 0, and the object returned. The 3 is the point: a
    // constructor that zeroed both - the shape every other one here has -
    // would pass a fixture that only checked field_4_.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(TextureStore) uint8_t storage[sizeof(TextureStore) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(expected, storage, sizeof(storage));
        const uint32_t three = 3, zero = 0;
        std::memcpy(expected + 16 + 0, &three, sizeof(three));
        std::memcpy(expected + 16 + 4, &zero, sizeof(zero));
        auto *store = reinterpret_cast<TextureStore *>(storage + 16);
        if (use_adapter) {
            expect(texture_store_construct_redirect(store, nullptr) == store);
        } else {
            store->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_cursor_construct() {
    // Four fields cleared and the object returned. The fixture checks the
    // exact bytes - all four zero, every other byte and both canaries
    // untouched - and that the adapter hands back `this`, which is the legacy
    // EAX residue.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Cursor) uint8_t storage[sizeof(Cursor) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(expected, storage, sizeof(storage));
        const uint32_t zero = 0;
        for (size_t offset : {0x0, 0x4, 0x8, 0xC}) {
            std::memcpy(expected + 16 + offset, &zero, sizeof(zero));
        }
        auto *cursor = reinterpret_cast<Cursor *>(storage + 16);
        if (use_adapter) {
            expect(cursor_construct_redirect(cursor, nullptr) == cursor);
        } else {
            cursor->construct();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_g_ambience_basewin_show() {
    // Raises the flag byte only when it is CLEAR. The seeds 0x02 and 0x5A are
    // the whole point: an unconditional `field_6C_ = 1`, which is what the
    // sibling basewin_hide does in the other direction, agrees for 0 and 1 and
    // overwrites every other value. Byte-exact comparison also holds it to
    // touching one byte - 0x6D beside it belongs to another event.
    struct Case { uint8_t before, after; };
    const Case cases[] = {
        {0x00, 0x01},   // clear -> raised
        {0x01, 0x01},   // already raised -> unchanged
        {0x02, 0x02},   // NOT overwritten
        {0x5A, 0x5A},
        {0xFF, 0xFF},
    };
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (const Case &one : cases) {
            alignas(GAmbience) uint8_t storage[sizeof(GAmbience) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            storage[16 + 0x6C] = one.before;
            std::memcpy(expected, storage, sizeof(storage));
            expected[16 + 0x6C] = one.after;

            auto *ambience = reinterpret_cast<GAmbience *>(storage + 16);
            if (use_adapter) {
                g_ambience_basewin_show_redirect(ambience, nullptr);
            } else {
                ambience->basewin_show();
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_net_win_unk5() {
    // Five bytes at the start of the object plus a cleared dword at 0x178, and
    // nothing else. The fixture establishes the resulting BYTES and that
    // nothing else in the object moves.
    //
    // What it does NOT establish, and cannot: the first four stores could be a
    // single dword write of 0xFF0000FF and no test could tell, because
    // little-endian that IS the same four bytes. A dword mutant was tried here
    // and survived, correctly. The byte form is kept because it is what the
    // original encodes, not because this fixture can distinguish it.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        std::vector<uint8_t> storage(0x200 + 32);
        std::vector<uint8_t> expected(storage.size());
        for (size_t i = 0; i < storage.size(); ++i) {
            storage[i] = static_cast<uint8_t>(0x35 + i * 17);
        }
        std::memcpy(expected.data(), storage.data(), storage.size());
        const uint32_t zero = 0;
        std::memcpy(expected.data() + 16 + 0x178, &zero, sizeof(zero));
        expected[16 + 0] = 0xFF;
        expected[16 + 1] = 0x00;
        expected[16 + 2] = 0x00;
        expected[16 + 3] = 0xFF;
        expected[16 + 4] = 0x02;

        auto *win = reinterpret_cast<NetWin *>(storage.data() + 16);
        if (use_adapter) {
            net_win_unk5_redirect(win, nullptr);
        } else {
            win->UNK5();
        }
        expect_storage_bytes(storage.data(), expected.data(), storage.size());
    }
}

void test_datalink_parse_id() {
    // The inverse of Datalink::UNK1. The pairs below are chosen so truncation
    // DIRECTION is observable: for a negative id that is not a multiple of
    // 10000, C++ division truncates toward zero and the remainder carries the
    // dividend's sign, where an arithmetic shift would floor and give a
    // different pair. UNK1 is then applied to the two halves and must rebuild
    // the original id, which is the property the two functions share.
    const int ids[] = {
        0, 1, 9999, 10000, 10001, 123456789,
        -1, -9999, -10000, -10001, -123456789,
        2147483647, -2147483647 - 1,
    };
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int id : ids) {
            alignas(Datalink) uint8_t storage[sizeof(Datalink) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));

            int32_t quotient = 0x11111111;
            int32_t remainder = 0x22222222;
            auto *link = reinterpret_cast<Datalink *>(storage + 16);
            if (use_adapter) {
                datalink_parse_id_redirect(
                    link, nullptr, id,
                    reinterpret_cast<DatalinkID *>(&quotient), &remainder);
            } else {
                link->parse_id(id, reinterpret_cast<DatalinkID *>(&quotient),
                               &remainder);
            }
            expect(quotient == id / 10000);
            expect(remainder == id % 10000);
            // Round trip through the inverse, except where the product
            // overflows and the identity cannot hold.
            if (id != 2147483647 && id != -2147483647 - 1) {
                expect(link->UNK1(quotient, remainder) == id);
            }
            // parse_id touches no field of its own.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_base_pop_read_check() {
    // The word CheckBox::UNK1/UNK2/set_state_pos edit, read whole, through the
    // CheckBox embedded at 0x2228. The Dialog displacement comes from THAT
    // subobject's own vbtable, so the fixture drives both tables the CheckBox
    // family is tested under - a body that hardcoded 0xA34 is right under the
    // first and wrong under the second.
    const int32_t own[3] = {0, 0x1C, 0xA34};
    const int32_t embedded[3] = {0, 0x40, 0xA70};
    const int32_t *const tables[2] = {own, embedded};

    for (int table_index = 0; table_index < 2; ++table_index) {
        const int32_t *const table = tables[table_index];
        const size_t word = 0x2228 + static_cast<size_t>(table[2]) + 0xEC;
        for (uint32_t seed : {0x00000000U, 0xFFFFFFFFU, 0xA55AA55AU, 0x00004000U}) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                std::vector<uint8_t> storage(word + 0x100 + 32);
                std::vector<uint8_t> expected(storage.size());
                for (size_t i = 0; i < storage.size(); ++i) {
                    storage[i] = static_cast<uint8_t>(0x35 + i * 17);
                }
                const int32_t *pointer = table;
                std::memcpy(storage.data() + 16 + 0x2228, &pointer, sizeof(pointer));
                std::memcpy(storage.data() + 16 + word, &seed, sizeof(seed));
                std::memcpy(expected.data(), storage.data(), storage.size());

                auto *pop = reinterpret_cast<BasePop *>(storage.data() + 16);
                const uint32_t got = use_adapter
                    ? base_pop_read_check_redirect(pop, nullptr)
                    : pop->read_check();
                expect(got == seed);
                // A reader writes nothing.
                expect_storage_bytes(storage.data(), expected.data(), storage.size());
            }
        }
    }
}

void test_sprite_box_id_to_pos() {
    // A list walk over four raw fields. Four behaviours have to be separated,
    // and three of them are edge cases the middle of the walk never reaches:
    //   * a NULL head returns the existing position and writes NOTHING;
    //   * a non-null head with count <= 0 still zeroes position and parks the
    //     cursor, because both stores precede the count check;
    //   * a match stops WITHOUT counting that entry;
    //   * no match stops after `count` entries, not at the end of the list.
    struct Entry { uint32_t pad0; int32_t id; uint32_t pad8; uint32_t next; };
    static Entry entries[4];
    for (int i = 0; i < 4; ++i) {
        entries[i].pad0 = 0xAAAA0000U + static_cast<uint32_t>(i);
        entries[i].id = 100 + i;
        entries[i].pad8 = 0xBBBB0000U + static_cast<uint32_t>(i);
        entries[i].next = (i < 3)
            ? static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&entries[i + 1]))
            : 0;
    }

    struct Case { uint32_t head; int32_t count; int id; uint32_t seed_pos;
                  uint32_t want_pos; int want_cursor; bool writes; };
    const uint32_t first = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(&entries[0]));
    const Case cases[] = {
        // null head: nothing written, the seeded position comes back
        {0,     4, 100, 0x1234U, 0x1234U, -1, false},
        {0,     0, 999, 0x4321U, 0x4321U, -1, false},
        // headed but empty: position zeroed, cursor parked at head
        {first, 0, 100, 0x9999U, 0,        0, true},
        {first, -1, 100, 0x9999U, 0,       0, true},
        // match at each position
        {first, 4, 100, 0x9999U, 0,        0, true},
        {first, 4, 101, 0x9999U, 1,        1, true},
        {first, 4, 103, 0x9999U, 3,        3, true},
        // no match: stops after `count` entries
        {first, 4, 555, 0x9999U, 4,       -2, true},
        {first, 2, 555, 0x9999U, 2,       -2, true},
        // count <= 0 with a NON-matching first entry. Both cases above used a
        // matching one, so `<= 0` and `< 0` returned the same answer and the
        // sweep survived a mutant that walks when it must not. Here the walk
        // is observable: `<= 0` leaves position 0 and the cursor at the head,
        // while entering the loop would advance both.
        {first, 0, 555, 0x9999U, 0,        0, true},
        // count == 1, which separates `<= 0` from `<= 1`: one entry IS
        // examined, so position lands on 1 and the cursor on the successor.
        {first, 1, 555, 0x9999U, 1,        1, true},
    };

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (const Case &one : cases) {
            alignas(SpriteBox) uint8_t storage[sizeof(SpriteBox) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x2C, one.head);
            write_at(storage, 16 + 0x34, one.count);
            write_at(storage, 16 + 0x38, one.seed_pos);
            const uint32_t parked = 0xCCCCCCCCU;
            write_at(storage, 16 + 0x30, parked);
            std::memcpy(expected, storage, sizeof(storage));
            if (one.writes) {
                write_at(expected, 16 + 0x38, one.want_pos);
                uint32_t cursor;
                if (one.want_cursor >= 0) {
                    cursor = static_cast<uint32_t>(
                        reinterpret_cast<uintptr_t>(&entries[one.want_cursor]));
                } else {
                    // walked off: cursor is whatever the last `next` gave
                    cursor = entries[one.count - 1].next;
                }
                write_at(expected, 16 + 0x30, cursor);
            }

            auto *box = reinterpret_cast<SpriteBox *>(storage + 16);
            const uint32_t got = use_adapter
                ? sprite_box_id_to_pos_redirect(box, nullptr, one.id)
                : box->id_to_pos(one.id);
            expect(got == one.want_pos);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_map_win_is_console() {
    // `cmp ecx, 0x9156B0 / sete al`: an identity test against the process-wide
    // Console, normalised to 0 or 1. Nothing is dereferenced, so the fixture
    // repoints ConsoleGlobal at storage it owns rather than touching
    // 0x009156B0, and checks both answers plus the normalisation - a body
    // returning the raw comparison would still be non-zero for "yes" and pass
    // a weaker assertion.
    void *const saved = ConsoleGlobal;

    alignas(MapWin) uint8_t storage[sizeof(MapWin) + 32];
    alignas(MapWin) uint8_t other[sizeof(MapWin) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    auto *win = reinterpret_cast<MapWin *>(storage + 16);
    auto *not_console = reinterpret_cast<MapWin *>(other + 16);

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        ConsoleGlobal = win;
        expect((use_adapter ? map_win_unk1_redirect(win, nullptr) : win->UNK1()) == 1);
        expect((use_adapter ? map_win_unk1_redirect(not_console, nullptr)
                            : not_console->UNK1()) == 0);
        ConsoleGlobal = not_console;
        expect((use_adapter ? map_win_unk1_redirect(win, nullptr) : win->UNK1()) == 0);
        ConsoleGlobal = nullptr;
        expect((use_adapter ? map_win_unk1_redirect(win, nullptr) : win->UNK1()) == 0);
        // A pure comparison writes nothing.
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    ConsoleGlobal = saved;
}

void test_base_pop_item() {
    // A delegation to the Dialogs at 0x21D0. The offset is the whole content,
    // so the fixture seeds a Dialogs THERE and nowhere else: a body using a
    // different offset reads an unseeded kind_, falls outside one-to-sixteen,
    // and returns 0 instead of the probe's value.
    func_dialog_item *const saved = DialogOriginalItem;
    DialogOriginalItem = &probe_base_pop_item;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int kind : {1, 4, 0, 17}) {
            std::vector<uint8_t> storage(0x21D0 + 0xC00 + 32);
            std::vector<uint8_t> expected(storage.size());
            for (size_t i = 0; i < storage.size(); ++i) {
                storage[i] = static_cast<uint8_t>(0x35 + i * 17);
            }
            uint8_t *const dialogs = storage.data() + 16 + 0x21D0;
            // Its own vbtable: {0, 0x188, 0xBA0}, so the Dialog is at 0xBA0.
            static const int32_t vbtable[3] = {0, 0x188, 0xBA0};
            const int32_t *pointer = vbtable;
            std::memcpy(dialogs, &pointer, sizeof(pointer));
            const int32_t kind_value = kind;
            std::memcpy(dialogs + 0x180, &kind_value, sizeof(kind_value));
            std::memcpy(expected.data(), storage.data(), storage.size());

            char text[8] = "abc";
            g_base_pop_item_calls = 0;
            g_base_pop_item_dialog = nullptr;
            auto *pop = reinterpret_cast<BasePop *>(storage.data() + 16);
            const int got = use_adapter
                ? base_pop_item_redirect(pop, nullptr, text, 7)
                : pop->item(text, 7);

            const bool in_range = (kind >= 1 && kind <= 16);
            if (in_range) {
                expect(got == 0x5A5A1234);
                expect(g_base_pop_item_calls == 1);
                // Dialog taken from the EMBEDDED object's own vbtable.
                expect(reinterpret_cast<uint8_t *>(g_base_pop_item_dialog)
                       == dialogs + 0xBA0);
                expect(g_base_pop_item_text == text);
                expect(g_base_pop_item_index == 7);
            } else {
                expect(got == 0);
                expect(g_base_pop_item_calls == 0);
            }
            // A delegation writes nothing of its own.
            expect_storage_bytes(storage.data(), expected.data(), storage.size());
        }
    }

    DialogOriginalItem = saved;
}

void test_base_pop_button_font_and_caviar_readback() {
    // BasePop::set_button_font distinguishes THREE outcomes that the return
    // code alone does not: a null primary refuses and stores nothing; an
    // uninitialised primary (Font's object pointer at +8 null) is skipped
    // while the other two are still installed; an initialised primary is
    // stored. A body that treated "uninitialised" as "refuse" returns the same
    // 0 and leaves different memory, so the fields are what separate them.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int primary_state = 0; primary_state < 3; ++primary_state) {
            std::vector<uint8_t> storage(0x3200 + 32);
            std::vector<uint8_t> expected(storage.size());
            for (size_t i = 0; i < storage.size(); ++i) {
                storage[i] = static_cast<uint8_t>(0x35 + i * 17);
            }
            alignas(Font) uint8_t font_one[sizeof(Font)] = {};
            alignas(Font) uint8_t font_two[sizeof(Font)] = {};
            alignas(Font) uint8_t font_three[sizeof(Font)] = {};
            // Font::is_initialized() reads the object pointer at +8.
            const uint32_t live = 0xDEADBEEFU;
            if (primary_state == 2) {
                std::memcpy(font_one + 8, &live, sizeof(live));
            }
            auto *one = primary_state == 0
                ? nullptr : reinterpret_cast<Font *>(font_one);
            auto *two = reinterpret_cast<Font *>(font_two);
            auto *three = reinterpret_cast<Font *>(font_three);

            std::memcpy(expected.data(), storage.data(), storage.size());
            if (primary_state != 0) {
                if (primary_state == 2) {
                    std::memcpy(expected.data() + 16 + 0x316C, &one, sizeof(one));
                }
                std::memcpy(expected.data() + 16 + 0x3170, &two, sizeof(two));
                std::memcpy(expected.data() + 16 + 0x3174, &three, sizeof(three));
            }

            auto *pop = reinterpret_cast<BasePop *>(storage.data() + 16);
            const int got = use_adapter
                ? base_pop_set_button_font_redirect(pop, nullptr, one, two, three)
                : pop->set_button_font(one, two, three);
            expect(got == (primary_state == 0 ? 3 : 0));
            expect_storage_bytes(storage.data(), expected.data(), storage.size());
        }
    }

    // Caviar::UNK11 reads three fields into three outputs, each null check
    // guarding only ITS OWN store. All eight null combinations are driven,
    // because a single guard around all three passes whenever they agree.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int mask = 0; mask < 8; ++mask) {
            alignas(Caviar) uint8_t storage[sizeof(Caviar) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            const int32_t v1 = 0x11112222, v2 = 0x33334444, v3 = 0x55556666;
            write_at(storage, 16 + 0x2C, v1);
            write_at(storage, 16 + 0x30, v2);
            write_at(storage, 16 + 0x34, v3);
            std::memcpy(expected, storage, sizeof(storage));

            int out1 = -1, out2 = -1, out3 = -1;
            int *p1 = (mask & 1) ? &out1 : nullptr;
            int *p2 = (mask & 2) ? &out2 : nullptr;
            int *p3 = (mask & 4) ? &out3 : nullptr;
            auto *caviar = reinterpret_cast<Caviar *>(storage + 16);
            if (use_adapter) {
                caviar_unk11_redirect(caviar, nullptr, p1, p2, p3);
            } else {
                caviar->UNK11(p1, p2, p3);
            }
            expect(out1 == ((mask & 1) ? v1 : -1));
            expect(out2 == ((mask & 2) ? v2 : -1));
            expect(out3 == ((mask & 4) ? v3 : -1));
            // A reader writes nothing to the object.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_popup_button_width_and_diplo_clear() {
    // Popup::on_adjust_button_width: 0x30AC / 10000 compared against 1000, and
    // 20 stored at 0x30D0 only when they differ. The values below straddle the
    // threshold exactly - 10000000 scales to 1000 and is the ONLY case that
    // must leave the field alone - so a body comparing before dividing, or
    // using the wrong constant, is caught.
    const int scales[] = {
        0, 1, 9999, 10000, 9999999, 10000000, 10000001, 10009999, 10010000,
        -10000000, 2147483647, -2147483647 - 1,
    };
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int scale : scales) {
            std::vector<uint8_t> storage(0x3100 + 32);
            std::vector<uint8_t> expected(storage.size());
            for (size_t i = 0; i < storage.size(); ++i) {
                storage[i] = static_cast<uint8_t>(0x35 + i * 17);
            }
            std::memcpy(storage.data() + 16 + 0x30AC, &scale, sizeof(scale));
            std::memcpy(expected.data(), storage.data(), storage.size());
            const int32_t quotient = scale / 10000;
            if (quotient != 1000) {
                const int32_t twenty = 20;
                std::memcpy(expected.data() + 16 + 0x30D0, &twenty, sizeof(twenty));
            }
            auto *popup = reinterpret_cast<Popup *>(storage.data() + 16);
            if (use_adapter) {
                popup_on_adjust_button_width_redirect(popup, nullptr);
            } else {
                popup->on_adjust_button_width();
            }
            expect_storage_bytes(storage.data(), expected.data(), storage.size());
        }
    }

    // DiploWin::UNK3: clear one indexed entry at 0xA1C, then the pair at 0xA24
    // and 0xA28. Indices 2 and 3 alias the pair, which the exact-byte
    // comparison covers without needing a special case.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int index : {0, 1, 2, 3, 4, 7}) {
            std::vector<uint8_t> storage(0xA80 + 32);
            std::vector<uint8_t> expected(storage.size());
            for (size_t i = 0; i < storage.size(); ++i) {
                storage[i] = static_cast<uint8_t>(0x35 + i * 17);
            }
            std::memcpy(expected.data(), storage.data(), storage.size());
            const int32_t zero = 0;
            std::memcpy(expected.data() + 16 + 0xA1C + index * 4, &zero, sizeof(zero));
            std::memcpy(expected.data() + 16 + 0xA24, &zero, sizeof(zero));
            std::memcpy(expected.data() + 16 + 0xA28, &zero, sizeof(zero));
            auto *diplo = reinterpret_cast<DiploWin *>(storage.data() + 16);
            if (use_adapter) {
                diplo_win_unk3_redirect(diplo, nullptr, index);
            } else {
                diplo->UNK3(index);
            }
            expect_storage_bytes(storage.data(), expected.data(), storage.size());
        }
    }
}

void test_player_lock() {
    func_square_lock_lock *const saved_lock = LockSquareLock;
    func_square_lock_unlock *const saved_unlock = LockSquareUnlock;
    LockSquareLock = &probe_player_lock_lock;
    LockSquareUnlock = &probe_player_lock_unlock;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        // add_lock: entries_[1] at +0x10, and bit 4 forced on regardless of
        // what the caller passed.
        for (int flags : {0, 1, 0x10, 0x2F, -1, 0x7FFFFFFF}) {
            alignas(PlayerLock) uint8_t storage[sizeof(PlayerLock) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            auto *lock = reinterpret_cast<PlayerLock *>(storage + 16);
            g_player_lock_probe = PlayerLockProbe{};
            const int got = use_adapter
                ? player_lock_add_lock_redirect(lock, nullptr, 7, flags, 11, 13)
                : lock->add_lock(7, flags, 11, 13);
            expect(got == 0x1234ABCD);
            expect(g_player_lock_probe.lock_calls == 1);
            expect(g_player_lock_probe.lock_entry == storage + 16 + 0x10);
            expect(g_player_lock_probe.lock_faction == 7);
            expect(g_player_lock_probe.lock_flags == (flags | 0x10));
            expect(g_player_lock_probe.lock_x == 11);
            expect(g_player_lock_probe.lock_y == 13);
            // add_lock writes nothing itself.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }

        // unlock: both entries in order, then the active byte cleared LAST.
        alignas(PlayerLock) uint8_t storage[sizeof(PlayerLock) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        storage[16] = 1;                       // active_
        std::memcpy(expected, storage, sizeof(storage));
        expected[16] = 0;
        auto *lock = reinterpret_cast<PlayerLock *>(storage + 16);
        g_player_lock_probe = PlayerLockProbe{};
        g_player_lock_object = storage + 16;
        if (use_adapter) {
            player_lock_unlock_redirect(lock, nullptr, 5);
        } else {
            lock->unlock(5);
        }
        g_player_lock_object = nullptr;
        expect(g_player_lock_probe.unlock_calls == 2);
        expect(g_player_lock_probe.unlock_entries[0] == storage + 16 + 4);
        expect(g_player_lock_probe.unlock_entries[1] == storage + 16 + 0x10);
        expect(g_player_lock_probe.unlock_faction == 5);
        expect(g_player_lock_probe.active_during_last_unlock == 1);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    LockSquareLock = saved_lock;
    LockSquareUnlock = saved_unlock;
}

void test_setup_win_scaling_and_datalink_combine() {
    // SetupWin::UNK1 and UNK2 are signed divisions by 1024 and 768 written as
    // MSVC idioms - a biased arithmetic shift and a magic-number multiply. The
    // values below are chosen so truncation DIRECTION is observable: a
    // negative dividend that is not a multiple of the divisor distinguishes
    // truncation toward zero from an arithmetic shift, and the two differ by
    // one there. A body using `>> 10` instead of `/ 1024` passes for every
    // non-negative input and fails these.
    const int scales[] = {0, 1, -1, 7, -7, 1024, 768, 100000, -100000,
                          2147483647, -2147483647 - 1};
    const int args[] = {0, 1, -1, 3, -3, 1023, -1023, 65536, -65536};
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int scale : scales) {
            for (int argument : args) {
                alignas(SetupWin) uint8_t storage[sizeof(SetupWin) + 0xA20 + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));
                write_at(storage, 16 + 0xA14, scale);
                write_at(storage, 16 + 0xA18, scale);
                std::memcpy(expected, storage, sizeof(storage));

                const int32_t product = static_cast<int32_t>(
                    static_cast<uint32_t>(scale) * static_cast<uint32_t>(argument));
                auto *setup = reinterpret_cast<SetupWin *>(storage + 16);
                const int one = use_adapter
                    ? setup_win_unk1_redirect(setup, nullptr, argument)
                    : setup->UNK1(argument);
                const int two = use_adapter
                    ? setup_win_unk2_redirect(setup, nullptr, argument)
                    : setup->UNK2(argument);
                expect(one == product / 1024);
                expect(two == product / 768);
                // Both are readers.
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }

    // UNK1 and UNK2 must read DIFFERENT fields: 0xA14 and 0xA18. Seeding them
    // apart is what catches a body that read the other one.
    {
        alignas(SetupWin) uint8_t storage[sizeof(SetupWin) + 0xA20 + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0xA14, 4096);
        write_at(storage, 16 + 0xA18, 7680);
        std::memcpy(expected, storage, sizeof(storage));
        auto *setup = reinterpret_cast<SetupWin *>(storage + 16);
        expect(setup->UNK1(1) == 4);      // 4096 / 1024, not 7680 / 1024
        expect(setup->UNK2(1) == 10);     // 7680 / 768,  not 4096 / 768
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // Datalink::UNK1 is a1 * 10000 + a2, wrapping at 32 bits. The wrapping
    // cases are the point: 300000 * 10000 overflows, and the original's
    // shift-and-add chain wraps exactly as the multiply does.
    {
        alignas(Datalink) uint8_t storage[sizeof(Datalink) + 32];
        uint8_t expected[sizeof(storage)];
        auto *link = reinterpret_cast<Datalink *>(storage + 16);
        const int pairs[][2] = {
            {0, 0}, {1, 0}, {0, 1}, {1, 1}, {-1, 0}, {0, -1}, {-1, -1},
            {7, 42}, {300000, 5}, {-300000, -5}, {214748, 3647},
            {2147483647, 1}, {-2147483647 - 1, -1},
        };
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            for (const auto &pair : pairs) {
                seed_storage(storage, expected, sizeof(storage));
                std::memcpy(expected, storage, sizeof(storage));
                const int32_t wanted = static_cast<int32_t>(
                    static_cast<uint32_t>(pair[0]) * 10000U
                    + static_cast<uint32_t>(pair[1]));
                const int got = use_adapter
                    ? datalink_unk1_redirect(link, nullptr, pair[0], pair[1])
                    : link->UNK1(pair[0], pair[1]);
                expect(got == wanted);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

void test_check_box_state_word() {
    // UNK1, UNK2 and set_state_pos all reach one word at
    // `this + vbtable[2] + 0xEC`. The offset comes from the OBJECT'S OWN
    // vbtable at run time, so the fixture drives each function under TWO
    // different tables - the most-derived {0, 0x1C, 0xA34} and an embedded
    // {0, 0x40, 0xA70} - because a body that hardcoded this class's own 0xA34
    // is correct under the first and silently wrong under the second. That is
    // the defect that passed every suite here once and crashed the game.
    const int32_t own[3] = {0, 0x1C, 0xA34};
    const int32_t embedded[3] = {0, 0x40, 0xA70};
    const int32_t *const tables[2] = {own, embedded};

    for (int table_index = 0; table_index < 2; ++table_index) {
        const int32_t *const table = tables[table_index];
        const size_t word = static_cast<size_t>(table[2]) + 0xEC;
        std::vector<uint8_t> storage(word + 0x100 + 32);
        std::vector<uint8_t> expected(storage.size());

        auto point_at = [&]() {
            for (size_t i = 0; i < storage.size(); ++i) {
                storage[i] = static_cast<uint8_t>(0x35 + i * 17);
            }
            const int32_t *pointer = table;
            std::memcpy(storage.data() + 16, &pointer, sizeof(pointer));
        };
        auto *check = reinterpret_cast<CheckBox *>(storage.data() + 16);
        auto flags_at = [&](std::vector<uint8_t> &buffer) -> uint32_t & {
            return *reinterpret_cast<uint32_t *>(buffer.data() + 16 + word);
        };

        for (int pos : {0, 1, 7, 15, 30, 31}) {
            const uint32_t mask = 1U << pos;
            for (uint32_t seed : {0x00000000U, 0xFFFFFFFFU, 0xA55AA55AU}) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    // UNK2 reads and returns the MASKED WORD, not 0/1.
                    point_at();
                    flags_at(storage) = seed;
                    std::memcpy(expected.data(), storage.data(), storage.size());
                    const int got = use_adapter
                        ? check_box_unk2_redirect(check, nullptr, pos)
                        : check->UNK2(pos);
                    expect(static_cast<uint32_t>(got) == (seed & mask));
                    expect_storage_bytes(storage.data(), expected.data(),
                                         storage.size());

                    // set_state_pos, both directions.
                    for (int state = 0; state < 2; ++state) {
                        point_at();
                        flags_at(storage) = seed;
                        std::memcpy(expected.data(), storage.data(), storage.size());
                        flags_at(expected) = state ? (seed | mask) : (seed & ~mask);
                        if (use_adapter) {
                            check_box_set_state_pos_redirect(check, nullptr, pos,
                                                             state ? -1 : 0);
                        } else {
                            check->set_state_pos(pos, state ? -1 : 0);
                        }
                        expect_storage_bytes(storage.data(), expected.data(),
                                             storage.size());
                    }

                    // UNK1 toggles: set becomes clear, clear becomes set.
                    point_at();
                    flags_at(storage) = seed;
                    std::memcpy(expected.data(), storage.data(), storage.size());
                    flags_at(expected) = (seed & mask) ? (seed & ~mask)
                                                       : (seed | mask);
                    if (use_adapter) {
                        check_box_unk1_redirect(check, nullptr, pos);
                    } else {
                        check->UNK1(pos);
                    }
                    expect_storage_bytes(storage.data(), expected.data(),
                                         storage.size());
                }
            }
        }

        // The shift count is masked to five bits by the hardware, so bit 33 is
        // bit 1. A body shifting by the raw value is undefined in C++ and in
        // practice would touch the wrong bit or none.
        point_at();
        flags_at(storage) = 0;
        std::memcpy(expected.data(), storage.data(), storage.size());
        flags_at(expected) = 2U;
        check->set_state_pos(33, 1);
        expect_storage_bytes(storage.data(), expected.data(), storage.size());
    }
}

void test_win_query_new_palette() {
    // Returns 1 and touches nothing. What this fixture CANNOT establish is the
    // call it makes: Palette::set_active_window is a recovered empty body with
    // no seam to record through, so a version that dropped the call entirely
    // would pass here and is behaviourally identical TODAY. The call is kept
    // because it is what the original encodes, and this note is here so the
    // gap is a stated limit rather than an assumed cover - if
    // set_active_window ever gains a body, this fixture needs a seam.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(expected, storage, sizeof(storage));
        auto *window = reinterpret_cast<Win *>(storage + 16);
        const int got = use_adapter
            ? win_on_query_new_palette_redirect(window, nullptr)
            : window->on_query_new_palette();
        expect(got == 1);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_base_pop_flag_setters() {
    // UNK3 and UNK4 edit bit 0 and bit 1 of the word at +0x20. What this
    // separates is WHICH BIT, WHICH DIRECTION, and that no other byte of the
    // object moves.
    //
    // What it does NOT separate, and cannot: the original reads the dword,
    // edits AL and writes the dword back, which for bits 0 and 1 is
    // indistinguishable from a byte store - the upper three bytes are
    // reloaded unchanged either way, and nothing can observe the difference in
    // a single-threaded body. A byte-store mutant was tried here and survived,
    // correctly. The dword form is kept because it is what the original
    // encodes, not because this fixture can tell.
    const uint32_t seeds[] = {
        0x00000000U, 0xFFFFFFFFU, 0xA55AA55AU, 0x00000003U, 0xFFFFFFFCU,
    };
    for (uint32_t seed : seeds) {
        for (int which = 0; which < 2; ++which) {       // UNK3 / UNK4
            for (int set = 0; set < 2; ++set) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    alignas(BasePop) uint8_t storage[sizeof(BasePop) + 32];
                    uint8_t expected[sizeof(storage)];
                    seed_storage(storage, expected, sizeof(storage));
                    write_at(storage, 16 + 0x20, seed);
                    std::memcpy(expected, storage, sizeof(storage));

                    const uint32_t bit = which ? 2U : 1U;
                    const uint32_t after = set ? (seed | bit) : (seed & ~bit);
                    write_at(expected, 16 + 0x20, after);

                    auto *pop = reinterpret_cast<BasePop *>(storage + 16);
                    const int argument = set ? -1 : 0;
                    if (which == 0) {
                        if (use_adapter) base_pop_unk3_redirect(pop, nullptr, argument);
                        else pop->UNK3(argument);
                    } else {
                        if (use_adapter) base_pop_unk4_redirect(pop, nullptr, argument);
                        else pop->UNK4(argument);
                    }
                    // Exact bytes: the edited word AND every other byte of the
                    // object and both canaries.
                    expect_storage_bytes(storage, expected, sizeof(storage));
                }
            }
        }
    }

    // Any non-zero argument sets, not just -1: the original tests the whole
    // dword, so a body checking only the low byte would mishandle 0x100.
    for (int value : {1, 0x100, 0x10000, 2147483647, -2147483647 - 1}) {
        alignas(BasePop) uint8_t storage[sizeof(BasePop) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x20, 0U);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16 + 0x20, 1U);
        auto *pop = reinterpret_cast<BasePop *>(storage + 16);
        pop->UNK3(value);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_menu_adjust_pulldown_pos() {
    // A `ret 8` and nothing else. The only things a fixture can establish are
    // that it dereferences neither pointer and touches no field, so it is
    // handed pointers to sentinels that must come back unchanged - a body that
    // wrote through either would be caught - and the object plus both canaries
    // are compared byte for byte.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        for (int null_arguments = 0; null_arguments < 2; ++null_arguments) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            int first = 0x11111111;
            int second = 0x22222222;
            int *const a1 = null_arguments ? nullptr : &first;
            int *const a2 = null_arguments ? nullptr : &second;

            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            if (use_adapter) {
                menu_on_adjust_pulldown_pos_redirect(menu, nullptr, a1, a2);
            } else {
                menu->on_adjust_pulldown_pos(a1, a2);
            }
            expect(first == 0x11111111);
            expect(second == 0x22222222);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_base_pop_key_gates() {
    // Both bodies are bit 14 of field_30A8_, INVERTED. Three things can go
    // wrong and the fixture has to separate all of them: the wrong bit, a
    // missing inversion, and reading the wrong offset. So the seeds are chosen
    // so that bit 14 disagrees with its neighbours and with the low bit - a
    // body reading bit 13, bit 15 or bit 0 returns the opposite answer on at
    // least one of them - and every case also checks that nothing was written.
    struct Case { uint32_t field; int wanted; };
    const Case cases[] = {
        {0x00000000U, 1},  // bit 14 clear -> 1
        {0xFFFFFFFFU, 0},  // bit 14 set   -> 0
        {0x00004000U, 0},  // exactly bit 14
        {0xFFFFBFFFU, 1},  // everything BUT bit 14
        {0x00002000U, 1},  // bit 13 only: a body reading 13 would say 0
        {0x00008000U, 1},  // bit 15 only: a body reading 15 would say 0
        {0x00000001U, 1},  // bit 0 only:  a body reading 0 would say 0
        {0xA55AA55AU, 1},  // bit 14 clear inside a hostile pattern
        {0xA55AE55AU, 0},  // same pattern with bit 14 set
    };
    for (const Case &one : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(BasePop) uint8_t storage[sizeof(BasePop) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x30A8, one.field);
            std::memcpy(expected, storage, sizeof(storage));

            auto *pop = reinterpret_cast<BasePop *>(storage + 16);
            const int click = use_adapter
                ? base_pop_on_key_click_redirect(pop, nullptr, -1, 2147483647)
                : pop->on_key_click(-1, 2147483647);
            const int up = use_adapter
                ? base_pop_on_key_up_redirect(pop, nullptr, -1)
                : pop->on_key_up(-1);
            expect(click == one.wanted);
            expect(up == one.wanted);
            // Neither reads its arguments and neither writes anything.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_win_scroll_positions() {
    // get_vert_pos / get_horz_pos are the same five instructions reading two
    // adjacent pointers, so the ONLY thing a fixture has to establish is which
    // one each reads. Seeding both scrolls with the same position would pass
    // for an implementation that read the wrong member, so the two carry
    // deliberately different values and every case checks the one it asked
    // for.
    //
    // The null case is the original's own branch, not a guard invented here:
    // the body tests the pointer and falls to `xor eax, eax` rather than
    // dereferencing. It is exercised per side, because a version that null
    // checked the wrong pointer would still return the right answer whenever
    // both happened to be set.
    const uint32_t values[] = {
        0U, 1U, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xA55AA55AU,
    };
    for (int vertical = 0; vertical < 2; ++vertical) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            for (int null_scroll = 0; null_scroll < 2; ++null_scroll) {
                for (uint32_t bits : values) {
                    alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
                    alignas(Scroll) uint8_t vertical_storage[sizeof(Scroll) + 32];
                    alignas(Scroll) uint8_t horizontal_storage[sizeof(Scroll) + 32];
                    uint8_t win_expected[sizeof(win_storage)];
                    uint8_t vertical_expected[sizeof(vertical_storage)];
                    uint8_t horizontal_expected[sizeof(horizontal_storage)];
                    seed_storage(win_storage, win_expected, sizeof(win_storage));
                    seed_storage(vertical_storage, vertical_expected,
                                 sizeof(vertical_storage));
                    seed_storage(horizontal_storage, horizontal_expected,
                                 sizeof(horizontal_storage));
                    auto *vertical_scroll = reinterpret_cast<Scroll *>(
                        vertical_storage + 16);
                    auto *horizontal_scroll = reinterpret_cast<Scroll *>(
                        horizontal_storage + 16);

                    // The side under test may be null; the OTHER side is
                    // always populated, so a body reading the wrong pointer
                    // returns a value instead of zero and is caught.
                    Scroll *const vertical_field =
                        (vertical && null_scroll) ? nullptr : vertical_scroll;
                    Scroll *const horizontal_field =
                        (!vertical && null_scroll) ? nullptr : horizontal_scroll;
                    write_at(win_storage, 16 + 0x43C, vertical_field);
                    write_at(win_storage, 16 + 0x440, horizontal_field);

                    // Distinct positions, so reading the wrong scroll is a
                    // wrong ANSWER rather than an accidentally equal one.
                    const uint32_t other = bits ^ 0x5A5A5A5AU;
                    write_at(vertical_storage, 16 + 0xA2C,
                             vertical ? bits : other);
                    write_at(horizontal_storage, 16 + 0xA2C,
                             vertical ? other : bits);

                    std::memcpy(win_expected, win_storage, sizeof(win_storage));
                    std::memcpy(vertical_expected, vertical_storage,
                                sizeof(vertical_storage));
                    std::memcpy(horizontal_expected, horizontal_storage,
                                sizeof(horizontal_storage));

                    auto *window = reinterpret_cast<Win *>(win_storage + 16);
                    const int wanted = null_scroll ? 0 : int_from_bits(bits);
                    int got;
                    if (vertical) {
                        got = use_adapter
                            ? win_get_vert_pos_redirect(window, nullptr)
                            : window->get_vert_pos();
                    } else {
                        got = use_adapter
                            ? win_get_horz_pos_redirect(window, nullptr)
                            : window->get_horz_pos();
                    }
                    expect(got == wanted);

                    // A getter writes nothing - not the window, not either
                    // scroll, not the canaries around any of them.
                    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));
                    expect_storage_bytes(vertical_storage, vertical_expected,
                                         sizeof(vertical_storage));
                    expect_storage_bytes(horizontal_storage, horizontal_expected,
                                         sizeof(horizontal_storage));
                }
            }
        }
    }
}

void test_win_paging() {
    const uint32_t values[] = {
        0U, 1U, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xA55AA55AU,
    };
    for (int vertical = 0; vertical < 2; ++vertical) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            for (uint32_t bits : values) {
                alignas(Win) uint8_t win_storage[sizeof(Win) + 32];
                alignas(Scroll) uint8_t vertical_storage[sizeof(Scroll) + 32];
                alignas(Scroll) uint8_t horizontal_storage[sizeof(Scroll) + 32];
                uint8_t win_expected[sizeof(win_storage)];
                uint8_t vertical_expected[sizeof(vertical_storage)];
                uint8_t horizontal_expected[sizeof(horizontal_storage)];
                seed_storage(win_storage, win_expected, sizeof(win_storage));
                seed_storage(vertical_storage, vertical_expected,
                             sizeof(vertical_storage));
                seed_storage(horizontal_storage, horizontal_expected,
                             sizeof(horizontal_storage));
                auto *vertical_scroll = reinterpret_cast<Scroll *>(
                    vertical_storage + 16);
                auto *horizontal_scroll = reinterpret_cast<Scroll *>(
                    horizontal_storage + 16);
                write_at(win_storage, 16 + 0x43C, vertical_scroll);
                write_at(win_storage, 16 + 0x440, horizontal_scroll);
                std::memcpy(win_expected, win_storage, sizeof(win_storage));
                std::memcpy(vertical_expected, vertical_storage,
                            sizeof(vertical_storage));
                std::memcpy(horizontal_expected, horizontal_storage,
                            sizeof(horizontal_storage));
                uint8_t *selected_expected = vertical
                    ? vertical_expected : horizontal_expected;
                write_at(selected_expected, 16 + 0xA40, bits);

                auto *window = reinterpret_cast<Win *>(win_storage + 16);
                const int paging = int_from_bits(bits);
                if (vertical) {
                    if (use_adapter) {
                        win_set_vert_paging_redirect(window, nullptr, paging);
                    } else {
                        window->set_vert_paging(paging);
                    }
                } else if (use_adapter) {
                    win_set_horz_paging_redirect(window, nullptr, paging);
                } else {
                    window->set_horz_paging(paging);
                }
                expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));
                expect_storage_bytes(vertical_storage, vertical_expected,
                                     sizeof(vertical_storage));
                expect_storage_bytes(horizontal_storage, horizontal_expected,
                                     sizeof(horizontal_storage));
            }
        }
    }

    for (int vertical = 0; vertical < 2; ++vertical) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Win) uint8_t null_storage[sizeof(Win) + 32];
            uint8_t null_expected[sizeof(null_storage)];
            seed_storage(null_storage, null_expected, sizeof(null_storage));
            Scroll *null_scroll = nullptr;
            write_at(null_storage, 16 + (vertical ? 0x43C : 0x440), null_scroll);
            std::memcpy(null_expected, null_storage, sizeof(null_storage));
            auto *null_window = reinterpret_cast<Win *>(null_storage + 16);
            if (vertical) {
                if (use_adapter) {
                    win_set_vert_paging_redirect(null_window, nullptr, INT_MIN);
                } else {
                    null_window->set_vert_paging(INT_MIN);
                }
            } else if (use_adapter) {
                win_set_horz_paging_redirect(null_window, nullptr, INT_MAX);
            } else {
                null_window->set_horz_paging(INT_MAX);
            }
            expect_storage_bytes(null_storage, null_expected, sizeof(null_storage));
        }
    }

    alignas(Win) uint8_t alias_win_storage[sizeof(Win) + 32];
    alignas(Scroll) uint8_t alias_scroll_storage[sizeof(Scroll) + 32];
    uint8_t alias_win_expected[sizeof(alias_win_storage)];
    uint8_t alias_scroll_expected[sizeof(alias_scroll_storage)];
    seed_storage(alias_win_storage, alias_win_expected, sizeof(alias_win_storage));
    seed_storage(alias_scroll_storage, alias_scroll_expected,
                 sizeof(alias_scroll_storage));
    auto *alias_scroll = reinterpret_cast<Scroll *>(alias_scroll_storage + 16);
    write_at(alias_win_storage, 16 + 0x43C, alias_scroll);
    write_at(alias_win_storage, 16 + 0x440, alias_scroll);
    std::memcpy(alias_win_expected, alias_win_storage, sizeof(alias_win_storage));
    std::memcpy(alias_scroll_expected, alias_scroll_storage,
                sizeof(alias_scroll_storage));
    const uint32_t alias_value = 0x89ABCDEFU;
    write_at(alias_scroll_expected, 16 + 0xA40, alias_value);
    auto *alias_window = reinterpret_cast<Win *>(alias_win_storage + 16);
    win_set_horz_paging_redirect(
        alias_window, nullptr, int_from_bits(alias_value));
    expect_storage_bytes(alias_win_storage, alias_win_expected,
                         sizeof(alias_win_storage));
    expect_storage_bytes(alias_scroll_storage, alias_scroll_expected,
                         sizeof(alias_scroll_storage));
}

void test_scroll_border_color() {
    const uint32_t colors[] = {
        0xFFFFFFFFU, 0U, 1U, 0x80000000U, 0x7FFFFFFFU,
    };
    const uint32_t thicknesses[] = {
        0xFFFFFFFFU, 0U, 1U, 2U, 0x80000000U, 0x7FFFFFFFU,
    };
    for (uint32_t color : colors) {
        for (uint32_t thickness : thicknesses) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));
                const uint32_t old_color = 0x13579BDFU;
                const uint32_t old_rect[4] = {
                    0x11111111U, 0x22222222U, 0x33333333U, 0x44444444U,
                };
                write_at(storage, 16 + 0xA1C, old_color);
                std::memcpy(storage + 16 + 0xA4C, old_rect, sizeof(old_rect));
                write_at(storage, 16 + 0xA60, thickness);
                std::memcpy(expected, storage, sizeof(storage));
                const uint32_t inset = color == 0xFFFFFFFFU ? 0U : 1U;
                const uint32_t extent = color == 0xFFFFFFFFU
                    ? thickness : thickness - 1U;
                write_at(expected, 16 + 0xA1C, color);
                write_at(expected, 16 + 0xA4C, inset);
                write_at(expected, 16 + 0xA50, inset);
                write_at(expected, 16 + 0xA54, extent);
                write_at(expected, 16 + 0xA58, extent);

                auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                if (use_adapter) {
                    scroll_set_border_color_redirect(
                        scroll, nullptr, int_from_bits(color));
                } else {
                    scroll->set_border_color(int_from_bits(color));
                }
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

void test_scroll_range() {
    struct RangeCase {
        uint32_t first;
        uint32_t second;
        uint32_t minimum;
        uint32_t maximum;
        uint32_t reverse;
    };
    const RangeCase cases[] = {
        {10U, 20U, 10U, 20U, 0U},
        {20U, 10U, 10U, 20U, 1U},
        {5U, 5U, 5U, 5U, 1U},
        {0xFFFFFFFFU, 0U, 0xFFFFFFFFU, 0U, 0U},
        {0x80000000U, 0x7FFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0U},
        {0x7FFFFFFFU, 0x80000000U, 0x80000000U, 0x7FFFFFFFU, 1U},
    };
    for (const RangeCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            uintptr_t vtable[63];
            seed_storage(storage, expected, sizeof(storage));
            install_scroll_redraw_probe(storage, vtable);
            std::memcpy(expected, storage, sizeof(storage));
            write_at(expected, 16 + 0xA20, fixture.minimum);
            write_at(expected, 16 + 0xA24, fixture.maximum);
            write_at(expected, 16 + 0xA28, fixture.reverse);
            write_at(expected, 16 + 0xA2C, fixture.minimum);

            reset_scroll_redraw_probe();
            scroll_redraw_result = fixture.first ^ fixture.second ^ 0xA55AA55AU;
            scroll_redraw_minimum = fixture.minimum;
            scroll_redraw_maximum = fixture.maximum;
            scroll_redraw_reverse = fixture.reverse;
            scroll_redraw_position = fixture.minimum;
            scroll_redraw_check_range = true;
            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            const uint32_t result = use_adapter
                ? scroll_set_range_redirect(
                    scroll, nullptr, int_from_bits(fixture.first),
                    int_from_bits(fixture.second))
                : scroll->set_range(int_from_bits(fixture.first),
                                    int_from_bits(fixture.second));
            expect(result == scroll_redraw_result);
            expect(scroll_redraw_calls == 1);
            expect(scroll_redraw_self == scroll);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

void test_scroll_style_setters() {
    struct StyleCase {
        size_t primary_offset;
        size_t left_offset;
        size_t right_offset;
    };
    const StyleCase styles[] = {
        {0xA5C, 0x1530, 0x207C},
        {0xA68, 0x153C, 0x2088},
        {0xA6C, 0x1534, 0x2080},
        {0xA70, 0x1538, 0x2084},
    };
    const uint32_t values[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };

    for (size_t style = 0; style < ARRAYSIZE(styles); ++style) {
        for (uint32_t value : values) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                uint8_t expected[sizeof(storage)];
                uintptr_t left_vtable[63];
                uintptr_t right_vtable[63];
                seed_storage(storage, expected, sizeof(storage));
                install_scroll_style_redraw_probes(
                    storage, left_vtable, right_vtable);
                std::memcpy(expected, storage, sizeof(storage));
                write_at(expected, 16 + styles[style].primary_offset, value);
                write_at(expected, 16 + styles[style].left_offset, value);
                write_at(expected, 16 + styles[style].right_offset, value);

                scroll_style_redraw_calls = 0;
                scroll_style_base = storage + 16;
                scroll_style_offsets[0] = styles[style].primary_offset;
                scroll_style_offsets[1] = styles[style].left_offset;
                scroll_style_offsets[2] = styles[style].right_offset;
                scroll_style_value = value;
                scroll_style_results[0] = value ^ 0x13579BDFU;
                scroll_style_results[1] = value ^ 0x2468ACE0U;

                auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                const int argument = int_from_bits(value);
                uint32_t result;
                switch (style) {
                  case 0:
                    result = use_adapter
                        ? scroll_set_button_color_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_button_color(argument);
                    break;
                  case 1:
                    result = use_adapter
                        ? scroll_set_bevel_thickness_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_thickness(argument);
                    break;
                  case 2:
                    result = use_adapter
                        ? scroll_set_bevel_upper_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_upper(argument);
                    break;
                  default:
                    result = use_adapter
                        ? scroll_set_bevel_lower_redirect(
                            scroll, nullptr, argument)
                        : scroll->set_bevel_lower(argument);
                    break;
                }
                expect(result == scroll_style_results[1]);
                expect(scroll_style_redraw_calls == 2);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

void test_scroll_thumb_resetters() {
    const uint32_t colors[] = {
        0xFFFFFFFFU, 0U, 0x80000000U,
    };
    const uint32_t thicknesses[] = {
        0U, 1U, 2U, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t color : colors) {
        for (uint32_t thickness : thicknesses) {
            for (int set_bar = 0; set_bar < 2; ++set_bar) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                    uint8_t expected[sizeof(storage)];
                    seed_storage(storage, expected, sizeof(storage));
                    const uint32_t old_thickness = 0x13579BDFU;
                    const uint32_t selected_thickness = set_bar
                        ? thickness : old_thickness;
                    write_at(storage, 16 + 0xA1C, color);
                    write_at(storage, 16 + 0xA60, old_thickness);
                    std::memcpy(expected, storage, sizeof(storage));
                    if (set_bar) {
                        write_at(expected, 16 + 0xA60, thickness);
                    }
                    const uint32_t inset = color == 0xFFFFFFFFU ? 0U : 1U;
                    const uint32_t extent = color == 0xFFFFFFFFU
                        ? selected_thickness : selected_thickness - 1U;
                    write_at(expected, 16 + 0xA4C, inset);
                    write_at(expected, 16 + 0xA50, inset);
                    write_at(expected, 16 + 0xA54, extent);
                    write_at(expected, 16 + 0xA58, extent);

                    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                    uint32_t result;
                    if (set_bar && use_adapter) {
                        result = scroll_set_bar_thickness_redirect(
                            scroll, nullptr, int_from_bits(thickness));
                    } else if (set_bar) {
                        result = scroll->set_bar_thickness(
                            int_from_bits(thickness));
                    } else if (use_adapter) {
                        result = scroll_set_thumb_rect_redirect(scroll, nullptr);
                    } else {
                        result = scroll->set_thumb_rect();
                    }
                    expect(result == (color == 0xFFFFFFFFU
                        ? (set_bar ? color : selected_thickness)
                        : selected_thickness - 1U));
                    expect_storage_bytes(storage, expected, sizeof(storage));
                }
            }
        }
    }
}

void test_scroll_position() {
    Win **const saved_current_win = ScrollCurrentWin;
    Win *published = reinterpret_cast<Win *>(0x45454545U);
    ScrollCurrentWin = &published;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        Win *null_parent = nullptr;
        write_at(storage, 16 + 0xC4, null_parent);
        std::memcpy(expected, storage, sizeof(storage));
        published = reinterpret_cast<Win *>(0x45454545U);
        reset_scroll_redraw_probe();
        auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
        const uint32_t result = use_adapter
            ? scroll_set_pos_redirect(scroll, nullptr, INT_MIN)
            : scroll->set_pos(INT_MIN);
        expect(result == 0U);
        expect(scroll_redraw_calls == 0);
        expect(published == reinterpret_cast<Win *>(0x45454545U));
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    struct PositionCase {
        uint32_t minimum;
        uint32_t maximum;
        uint32_t reverse;
        uint32_t input;
        uint32_t expected;
    };
    const PositionCase cases[] = {
        {0U, 100U, 0U, 0xFFFFFFFFU, 0U},
        {0U, 100U, 0U, 101U, 100U},
        {0xFFFFFFF6U, 10U, 0U, 0xFFFFFFFDU, 0xFFFFFFFDU},
        {0U, 100U, 1U, 25U, 75U},
        {0U, 100U, 0x80000000U, 1000U, 0U},
        {0x80000000U, 0x7FFFFFFFU, 1U, 0U, 0xFFFFFFFFU},
    };
    alignas(Win) uint8_t parent_storage[sizeof(Win)];
    auto *parent = reinterpret_cast<Win *>(parent_storage);
    for (const PositionCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            uintptr_t vtable[63];
            seed_storage(storage, expected, sizeof(storage));
            install_scroll_redraw_probe(storage, vtable);
            write_at(storage, 16 + 0xC4, parent);
            write_at(storage, 16 + 0xA20, fixture.minimum);
            write_at(storage, 16 + 0xA24, fixture.maximum);
            write_at(storage, 16 + 0xA28, fixture.reverse);
            std::memcpy(expected, storage, sizeof(storage));
            write_at(expected, 16 + 0xA2C, fixture.expected);

            published = reinterpret_cast<Win *>(0x45454545U);
            reset_scroll_redraw_probe();
            scroll_redraw_result = fixture.input ^ 0x5AA55AA5U;
            scroll_redraw_position = fixture.expected;
            scroll_redraw_parent = parent;
            scroll_redraw_check_current = true;
            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            const uint32_t result = use_adapter
                ? scroll_set_pos_redirect(
                    scroll, nullptr, int_from_bits(fixture.input))
                : scroll->set_pos(int_from_bits(fixture.input));
            expect(result == scroll_redraw_result);
            expect(scroll_redraw_calls == 1);
            expect(scroll_redraw_self == scroll);
            expect(published == parent);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
    ScrollCurrentWin = saved_current_win;
}

void test_expand_rect() {
    struct ExpandRectCase {
        uint32_t rect[4];
        uint32_t horizontal;
        uint32_t vertical;
    };
    const ExpandRectCase cases[] = {
        {{0U, 0U, 0U, 0U}, 0U, 0U},
        {{10U, 20U, 30U, 40U}, 3U, 4U},
        {{10U, 20U, 30U, 40U}, 0xFFFFFFFFU, 0xFFFFFFFEU},
        {{0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0U},
         0x80000000U, 0x7FFFFFFFU},
        {{0U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
         0x7FFFFFFFU, 0x80000000U},
    };

    for (const ExpandRectCase &test : cases) {
        alignas(RECT) uint8_t storage[sizeof(RECT) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(storage + 16, test.rect, sizeof(test.rect));
        std::memcpy(expected, storage, sizeof(storage));
        uint32_t expected_rect[4];
        std::memcpy(expected_rect, test.rect, sizeof(expected_rect));
        expected_rect[0] -= test.horizontal;
        expected_rect[2] += test.horizontal;
        expected_rect[1] -= test.vertical;
        expected_rect[3] += test.vertical;
        std::memcpy(expected + 16, expected_rect, sizeof(expected_rect));

        auto *rect = reinterpret_cast<RECT *>(storage + 16);
        expect(expand_rect(rect, int_from_bits(test.horizontal),
                           int_from_bits(test.vertical)) == rect);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void test_scroll_sprite_setters() {
    struct Geometry {
        uint32_t width;
        uint32_t stored_height;
        bool horizontal;
        bool vertical;
    };
    const Geometry geometries[] = {
        {100U, 0xFFFFFFECU, true, false},
        {20U, 0xFFFFFF9CU, false, true},
        {20U, 0xFFFFFFECU, false, false},
        {1U, 1U, true, false},
        {0x80000001U, 0x80000000U, true, false},
        {0x80000000U, 0x80000000U, false, false},
    };
    Sprite *sprite_sets[][3] = {
        {nullptr, nullptr, nullptr},
        {reinterpret_cast<Sprite *>(0x10101010U),
         reinterpret_cast<Sprite *>(0x20202020U),
         reinterpret_cast<Sprite *>(0x30303030U)},
        {reinterpret_cast<Sprite *>(0x45454545U),
         reinterpret_cast<Sprite *>(0x56565656U),
         reinterpret_cast<Sprite *>(0x45454545U)},
    };

    for (const Geometry &geometry : geometries) {
        for (Sprite **sprites : sprite_sets) {
            for (int direction = 0; direction < 4; ++direction) {
                for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
                    uint8_t expected[sizeof(storage)];
                    seed_storage(storage, expected, sizeof(storage));
                    write_at(storage, 16 + 0x4C4, geometry.width);
                    write_at(storage, 16 + 0x4C8, geometry.stored_height);
                    std::memcpy(expected, storage, sizeof(storage));

                    const size_t primary_offsets[] = {
                        0xA7C, 0xA94, 0xA88, 0xAA0,
                    };
                    const size_t primary_offset = primary_offsets[direction];
                    for (size_t index = 0; index < 3; ++index) {
                        write_at(expected, 16 + primary_offset
                            + index * sizeof(Sprite *), sprites[index]);
                    }
                    const bool update_button = direction < 2
                        ? geometry.horizontal : geometry.vertical;
                    if (update_button) {
                        const size_t button_offset = (direction & 1)
                            ? 0x2108 : 0x15BC;
                        for (size_t index = 0; index < 3; ++index) {
                            write_at(expected, 16 + button_offset
                                + index * sizeof(Sprite *), sprites[index]);
                        }
                    }

                    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
                    switch (direction) {
                      case 0:
                        if (use_adapter) {
                            expect(scroll_set_sprite_left_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_left(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      case 1:
                        if (use_adapter) {
                            expect(scroll_set_sprite_right_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_right(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      case 2:
                        if (use_adapter) {
                            expect(scroll_set_sprite_up_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_up(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                      default:
                        if (use_adapter) {
                            expect(scroll_set_sprite_down_redirect(
                                scroll, nullptr, sprites[0], sprites[1], sprites[2])
                                == sprites[0]);
                        } else {
                            scroll->set_sprite_down(
                                sprites[0], sprites[1], sprites[2]);
                        }
                        break;
                    }
                    expect_storage_bytes(storage, expected, sizeof(storage));
                }
            }
        }
    }
}

struct ThumbRectCase {
    const char *name;
    uint32_t flags;
    uint32_t border_color;
    uint32_t width;
    uint32_t stored_height;
    uint32_t minimum;
    uint32_t maximum;
    uint32_t position;
    uint32_t drag_coordinate;
    uint32_t rect[4];
};

uint32_t arithmetic_shift_right_one(uint32_t value) {
    return (value >> 1U) | (value & 0x80000000U);
}

uint32_t signed_min_bits(uint32_t left, uint32_t right) {
    return int_from_bits(left) < int_from_bits(right) ? left : right;
}

void expected_thumb_rect(
        const ThumbRectCase &fixture, uint32_t rect[4], uint32_t &drag_coordinate) {
    rect[2] -= rect[0];
    rect[0] = 0;
    rect[3] -= rect[1];
    rect[1] = 0;
    if (fixture.border_color != 0xFFFFFFFFU) {
        for (int index = 0; index < 4; ++index) {
            ++rect[index];
        }
    }

    const uint32_t extent = rect[2] - rect[0];
    const uint32_t height = 0U - fixture.stored_height;
    const bool horizontal = int_from_bits(fixture.width) > int_from_bits(height);
    const uint32_t axis_length = horizontal ? fixture.width : height;
    const bool no_end_buttons = (fixture.flags & 2U) != 0;
    uint32_t offset;

    if (drag_coordinate == 0xFFFFFFFFU) {
        offset = no_end_buttons ? 0U : extent + 1U;
        uint32_t adjustment = 0xFFFFFFFFU - (no_end_buttons
            ? extent : extent * 3U);
        if (fixture.border_color != 0xFFFFFFFFU) {
            adjustment -= 2U;
        }
        if (fixture.maximum != fixture.minimum) {
            const uint32_t numerator = (fixture.position - fixture.minimum)
                * (axis_length + adjustment);
            const int64_t quotient = static_cast<int64_t>(int_from_bits(numerator))
                / static_cast<int64_t>(
                    int_from_bits(fixture.maximum - fixture.minimum));
            offset += static_cast<uint32_t>(quotient);
        }
    } else {
        const uint32_t far_edge = rect[0] + extent;
        const uint32_t candidate = drag_coordinate
            + arithmetic_shift_right_one(extent);
        const uint32_t upper = axis_length - (no_end_buttons
            ? far_edge : far_edge * 2U);
        const uint32_t limited = signed_min_bits(candidate, upper);
        if (no_end_buttons) {
            offset = int_from_bits(limited) < 0 ? 0U : limited;
        } else if (int_from_bits(far_edge) > int_from_bits(limited)) {
            offset = far_edge;
        } else {
            offset = limited;
        }
        drag_coordinate = 0xFFFFFFFFU;
    }

    if (horizontal) {
        rect[0] += offset;
        rect[2] += offset;
    } else {
        rect[1] += offset;
        rect[3] += offset;
    }
}

void initialize_thumb_case(uint8_t *storage, const ThumbRectCase &fixture) {
    write_at(storage, 16 + 0x4C4, fixture.width);
    write_at(storage, 16 + 0x4C8, fixture.stored_height);
    write_at(storage, 16 + 0xA14, fixture.flags);
    write_at(storage, 16 + 0xA1C, fixture.border_color);
    write_at(storage, 16 + 0xA20, fixture.minimum);
    write_at(storage, 16 + 0xA24, fixture.maximum);
    write_at(storage, 16 + 0xA2C, fixture.position);
    write_at(storage, 16 + 0xA3C, fixture.drag_coordinate);
    std::memcpy(storage + 16 + 0xA4C, fixture.rect, sizeof(fixture.rect));
}

void test_scroll_compute_thumb_rect() {
    const ThumbRectCase cases[] = {
        {"horizontal no-buttons minimum", 0xA5A50002U, 0xFFFFFFFFU,
         200U, 0xFFFFFFECU, 0U, 100U, 0U, 0xFFFFFFFFU,
         {3U, 4U, 13U, 14U}},
        {"horizontal buttons midpoint", 0xA5A50000U, 0U,
         300U, 0xFFFFFFECU, 0xFFFFFFF6U, 10U, 0U, 0xFFFFFFFFU,
         {5U, 7U, 25U, 37U}},
        {"horizontal outside range", 0U, 1U,
         100U, 0xFFFFFFF0U, 0U, 10U, 15U, 0xFFFFFFFFU,
         {0U, 0U, 8U, 12U}},
        {"vertical no-buttons truncation", 2U, 0xFFFFFFFFU,
         20U, 0xFFFFFF38U, 0U, 8U, 3U, 0xFFFFFFFFU,
         {9U, 11U, 19U, 41U}},
        {"equal dimensions choose vertical", 0U, 0U,
         20U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFFFFU,
         {2U, 3U, 12U, 23U}},
        {"equal endpoints", 0U, 0xFFFFFFFFU,
         120U, 0xFFFFFFF0U, 7U, 7U, 0x80000000U, 0xFFFFFFFFU,
         {4U, 8U, 16U, 24U}},
        {"reversed range", 2U, 0U,
         160U, 0xFFFFFFE0U, 10U, 0xFFFFFFF6U, 0U, 0xFFFFFFFFU,
         {6U, 9U, 17U, 30U}},
        {"wrapping static arithmetic", 0U, 0x80000000U,
         0x7FFFFFF0U, 0xFFFFFFE0U, 0x70000000U, 0x90000000U,
         0x80000001U, 0xFFFFFFFFU,
         {0x70000000U, 0x11111111U, 0xF0000010U, 0x33333333U}},
        {"horizontal drag below", 2U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFF80U,
         {3U, 4U, 13U, 14U}},
        {"horizontal drag above", 2U, 0U,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 200U,
         {3U, 4U, 13U, 14U}},
        {"horizontal drag buttons inside", 0U, 0xFFFFFFFFU,
         140U, 0xFFFFFFECU, 0U, 10U, 5U, 37U,
         {2U, 4U, 14U, 20U}},
        {"horizontal drag crossed bounds", 0U, 0U,
         10U, 0xFFFFFFFCU, 0U, 10U, 5U, 0U,
         {0U, 0U, 20U, 6U}},
        {"vertical drag no-buttons", 2U, 0xFFFFFFFFU,
         12U, 0xFFFFFF80U, 0U, 10U, 5U, 57U,
         {1U, 4U, 9U, 40U}},
        {"vertical drag buttons", 0U, 0U,
         12U, 0xFFFFFF80U, 0U, 10U, 5U, 90U,
         {1U, 4U, 9U, 40U}},
        {"vertical nonsquare extent", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF00U, 0U, 10U, 5U, 20U,
         {7U, 9U, 12U, 109U}},
        {"horizontal wrapped lower", 0U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
        {"vertical wrapped lower", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF9CU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
    };

    for (const ThumbRectCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
            uint8_t expected[sizeof(storage)];
            alignas(RECT) uint8_t output_storage[sizeof(RECT) + 32];
            uint8_t output_expected[sizeof(output_storage)];
            seed_storage(storage, expected, sizeof(storage));
            seed_storage(output_storage, output_expected, sizeof(output_storage));
            initialize_thumb_case(storage, fixture);
            std::memcpy(expected, storage, sizeof(storage));

            uint32_t expected_rect[4];
            std::memcpy(expected_rect, fixture.rect, sizeof(expected_rect));
            uint32_t expected_drag = fixture.drag_coordinate;
            expected_thumb_rect(fixture, expected_rect, expected_drag);
            write_at(expected, 16 + 0xA3C, expected_drag);
            std::memcpy(expected + 16 + 0xA4C,
                        expected_rect, sizeof(expected_rect));
            std::memcpy(output_expected + 16,
                        expected_rect, sizeof(expected_rect));

            auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
            auto *output = reinterpret_cast<RECT *>(output_storage + 16);
            const int failures_before = failures;
            if (use_adapter) {
                expect(scroll_compute_thumb_rect_redirect(
                           scroll, nullptr, output)
                       == reinterpret_cast<RECT *>(storage + 16 + 0xA4C));
            } else {
                scroll->compute_thumb_rect(output);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect_storage_bytes(
                output_storage, output_expected, sizeof(output_storage));
            if (failures != failures_before) {
                report_storage_mismatch(
                    fixture.name, storage, expected, sizeof(storage));
                report_storage_mismatch(fixture.name, output_storage,
                                        output_expected, sizeof(output_storage));
            }
        }
    }

    alignas(Scroll) uint8_t alias_storage[sizeof(Scroll) + 32];
    uint8_t alias_expected[sizeof(alias_storage)];
    seed_storage(alias_storage, alias_expected, sizeof(alias_storage));
    initialize_thumb_case(alias_storage, cases[1]);
    std::memcpy(alias_expected, alias_storage, sizeof(alias_storage));
    uint32_t alias_rect[4];
    std::memcpy(alias_rect, cases[1].rect, sizeof(alias_rect));
    uint32_t alias_drag = cases[1].drag_coordinate;
    expected_thumb_rect(cases[1], alias_rect, alias_drag);
    write_at(alias_expected, 16 + 0xA3C, alias_drag);
    std::memcpy(alias_expected + 16 + 0xA4C, alias_rect, sizeof(alias_rect));
    auto *alias_scroll = reinterpret_cast<Scroll *>(alias_storage + 16);
    auto *alias_rect_pointer = reinterpret_cast<RECT *>(
        alias_storage + 16 + 0xA4C);
    expect(scroll_compute_thumb_rect_redirect(
               alias_scroll, nullptr, alias_rect_pointer) == alias_rect_pointer);
    expect_storage_bytes(alias_storage, alias_expected, sizeof(alias_storage));
}

enum class PullDownOperation {
    Hide,
    Show,
    Disable,
    Enable,
    Check,
    Uncheck,
};

int call_pull_down_mutator(
        PullDown *pull_down, PullDownOperation operation, int id, bool adapter) {
    switch (operation) {
      case PullDownOperation::Hide:
        return adapter
            ? pull_down_hide_item_redirect(pull_down, nullptr, id)
            : pull_down->hide_item(id);
      case PullDownOperation::Show:
        return adapter
            ? pull_down_show_item_redirect(pull_down, nullptr, id)
            : pull_down->show_item(id);
      case PullDownOperation::Disable:
        return adapter
            ? pull_down_disable_item_redirect(pull_down, nullptr, id)
            : pull_down->disable_item(id);
      case PullDownOperation::Enable:
        return adapter
            ? pull_down_enable_item_redirect(pull_down, nullptr, id)
            : pull_down->enable_item(id);
      case PullDownOperation::Check:
        return adapter
            ? pull_down_check_item_redirect(pull_down, nullptr, id)
            : pull_down->check_item(id);
      case PullDownOperation::Uncheck:
        return adapter
            ? pull_down_uncheck_item_redirect(pull_down, nullptr, id)
            : pull_down->uncheck_item(id);
    }
    return -1;
}

uint32_t initial_pull_down_flags(PullDownOperation operation) {
    const uint32_t preserved = 0xA5A50004U;
    switch (operation) {
      case PullDownOperation::Hide:
      case PullDownOperation::Enable:
      case PullDownOperation::Uncheck:
        return preserved | (operation == PullDownOperation::Hide ? 1U
            : operation == PullDownOperation::Enable ? 2U : 8U);
      case PullDownOperation::Show:
      case PullDownOperation::Disable:
      case PullDownOperation::Check:
        return preserved;
    }
    return preserved;
}

uint32_t expected_pull_down_flags(
        PullDownOperation operation, uint32_t flags) {
    switch (operation) {
      case PullDownOperation::Hide:
        return flags & ~1U;
      case PullDownOperation::Show:
        return flags | 1U;
      case PullDownOperation::Disable:
        return flags | 2U;
      case PullDownOperation::Enable:
        return flags & ~2U;
      case PullDownOperation::Check:
        return flags | 8U;
      case PullDownOperation::Uncheck:
        return flags & ~8U;
    }
    return flags;
}

void fill_pull_down_ids(uint8_t *storage) {
    for (int index = 0; index < 64; ++index) {
        const int id = 1000 + index;
        write_at(storage, 16 + 0xA20 + static_cast<size_t>(index) * 0x14, id);
    }
}

void test_pull_down_item_state() {
    const PullDownOperation operations[] = {
        PullDownOperation::Hide,
        PullDownOperation::Show,
        PullDownOperation::Disable,
        PullDownOperation::Enable,
        PullDownOperation::Check,
        PullDownOperation::Uncheck,
    };
    const int target_indices[] = {0, 31, 63};
    const int target_ids[] = {INT_MIN, 17, INT_MAX};
    for (PullDownOperation operation : operations) {
        for (int target_case = 0; target_case < 3; ++target_case) {
            for (int adapter = 0; adapter < 2; ++adapter) {
                alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));
                fill_pull_down_ids(storage);
                const int target_index = target_indices[target_case];
                const int target_id = target_ids[target_case];
                const size_t item_offset = 0xA18
                    + static_cast<size_t>(target_index) * 0x14;
                write_at(storage, 16 + item_offset + 8, target_id);
                const uint32_t flags = initial_pull_down_flags(operation);
                write_at(storage, 16 + item_offset + 0xC, flags);
                const int ignored_count = INT_MIN;
                write_at(storage, 16 + 0xF20, ignored_count);
                const uint32_t visible_bits = operation == PullDownOperation::Hide
                    ? 0U : operation == PullDownOperation::Show
                    ? 0x7FFFFFFFU : 0x13579BDFU;
                write_at(storage, 16 + 0xF2C, visible_bits);
                storage[16 + 0xF34] = 0x7E;
                std::memcpy(expected, storage, sizeof(storage));
                write_at(expected, 16 + item_offset + 0xC,
                         expected_pull_down_flags(operation, flags));
                if (operation == PullDownOperation::Hide) {
                    const uint32_t decremented = visible_bits - 1U;
                    write_at(expected, 16 + 0xF2C, decremented);
                } else if (operation == PullDownOperation::Show) {
                    const uint32_t incremented = visible_bits + 1U;
                    write_at(expected, 16 + 0xF2C, incremented);
                }
                expected[16 + 0xF34] = 1;

                auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
                expect(call_pull_down_mutator(
                           pull_down, operation, target_id, adapter != 0) == 0);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }

    for (PullDownOperation operation : operations) {
        for (int miss_case = 0; miss_case < 3; ++miss_case) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            fill_pull_down_ids(storage);
            int requested_id = 77;
            if (miss_case == 0) {
                const int sentinel = -1;
                write_at(storage, 16 + 0xA20 + 0x14, sentinel);
                write_at(storage, 16 + 0xA20 + 2 * 0x14, requested_id);
            } else if (miss_case == 1) {
                requested_id = -1;
            } else {
                requested_id = 999;
            }
            const int ignored_count = INT_MAX;
            write_at(storage, 16 + 0xF20, ignored_count);
            std::memcpy(expected, storage, sizeof(storage));
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            expect(call_pull_down_mutator(
                       pull_down, operation, requested_id, false) == 11);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    for (PullDownOperation operation : {
            PullDownOperation::Hide, PullDownOperation::Show}) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            fill_pull_down_ids(storage);
            const int id = 1002;
            const uint32_t flags = operation == PullDownOperation::Hide
                ? 0xA5A50004U : 0xA5A50005U;
            write_at(storage, 16 + 0xA24 + 2 * 0x14, flags);
            const uint32_t visible = 0x89ABCDEFU;
            write_at(storage, 16 + 0xF2C, visible);
            std::memcpy(expected, storage, sizeof(storage));
            expected[16 + 0xF34] = 1;
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            expect(call_pull_down_mutator(
                       pull_down, operation, id, adapter != 0) == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    alignas(PullDown) uint8_t duplicate_storage[sizeof(PullDown) + 32];
    uint8_t duplicate_expected[sizeof(duplicate_storage)];
    seed_storage(duplicate_storage, duplicate_expected, sizeof(duplicate_storage));
    fill_pull_down_ids(duplicate_storage);
    const int duplicate_id = 12345;
    write_at(duplicate_storage, 16 + 0xA20 + 1 * 0x14, duplicate_id);
    write_at(duplicate_storage, 16 + 0xA20 + 40 * 0x14, duplicate_id);
    const uint32_t duplicate_flags = 0x40U;
    write_at(duplicate_storage, 16 + 0xA24 + 1 * 0x14, duplicate_flags);
    write_at(duplicate_storage, 16 + 0xA24 + 40 * 0x14, duplicate_flags);
    std::memcpy(duplicate_expected, duplicate_storage, sizeof(duplicate_storage));
    const uint32_t disabled_flags = duplicate_flags | 2U;
    write_at(duplicate_expected, 16 + 0xA24 + 1 * 0x14, disabled_flags);
    duplicate_expected[16 + 0xF34] = 1;
    auto *duplicate_pull_down = reinterpret_cast<PullDown *>(duplicate_storage + 16);
    expect(pull_down_disable_item_redirect(
               duplicate_pull_down, nullptr, duplicate_id) == 0);
    expect_storage_bytes(
        duplicate_storage, duplicate_expected, sizeof(duplicate_storage));
}

void test_pull_down_get_selected() {
    struct SelectionCase {
        int index;
        bool disabled;
    };
    const SelectionCase cases[] = {
        {-1, false}, {0, false}, {31, true}, {63, false},
        {-2, false}, {64, true}, {65, false}, {INT_MIN, true}, {INT_MAX, false},
    };
    for (const SelectionCase &fixture : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0xF28, fixture.index);
            if (fixture.index != -1) {
                const uint32_t offset = 0xA24U
                    + static_cast<uint32_t>(fixture.index) * 0x14U;
                const uint32_t flags = fixture.disabled ? 0xA5A50002U : 0xA5A5000CU;
                write_at(storage, 16 + offset, flags);
            }
            std::memcpy(expected, storage, sizeof(storage));
            auto *pull_down = reinterpret_cast<PullDown *>(storage + 16);
            const int result = adapter
                ? pull_down_get_selected_redirect(pull_down, nullptr)
                : pull_down->get_selected();
            expect(result == (fixture.index == -1 || fixture.disabled
                              ? -1 : fixture.index));
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

}  // namespace

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(51, test_texture_store_construct);
LEAF_CASE(50, test_cursor_construct);
LEAF_CASE(49, test_g_ambience_basewin_show);
LEAF_CASE(48, test_net_win_unk5);
LEAF_CASE(47, test_datalink_parse_id);
LEAF_CASE(46, test_base_pop_read_check);
LEAF_CASE(45, test_sprite_box_id_to_pos);
LEAF_CASE(44, test_map_win_is_console);
LEAF_CASE(43, test_base_pop_item);
LEAF_CASE(42, test_base_pop_button_font_and_caviar_readback);
LEAF_CASE(41, test_popup_button_width_and_diplo_clear);
LEAF_CASE(40, test_player_lock);
LEAF_CASE(39, test_setup_win_scaling_and_datalink_combine);
LEAF_CASE(38, test_check_box_state_word);
LEAF_CASE(37, test_win_query_new_palette);
LEAF_CASE(36, test_base_pop_flag_setters);
LEAF_CASE(35, test_menu_adjust_pulldown_pos);
LEAF_CASE(34, test_base_pop_key_gates);
LEAF_CASE(33, test_win_scroll_positions);
LEAF_CASE(32, test_win_paging);
LEAF_CASE(65, test_scroll_border_color);
LEAF_CASE(62, test_scroll_range);
LEAF_CASE(63, test_scroll_style_setters);
LEAF_CASE(64, test_scroll_thumb_resetters);
LEAF_CASE(66, test_scroll_position);
LEAF_CASE(67, test_expand_rect);
LEAF_CASE(68, test_scroll_sprite_setters);
LEAF_CASE(69, test_scroll_compute_thumb_rect);
LEAF_CASE(70, test_pull_down_item_state);
LEAF_CASE(71, test_pull_down_get_selected);
}  // namespace
