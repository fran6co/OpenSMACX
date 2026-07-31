// Recovery leaf tests: text_and_resources.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus one LEAF_CASE line at the bottom, and
// bumping this family's count in tests/leaf/leaf_case_manifest.h.
#include "leaf_shared.h"

#define LEAF_FAMILY "text_and_resources"

namespace {

void __cdecl menu_proc_fixture(int) {}

namespace {
int g_menu_repaints;
int g_menu_poisons;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
void __thiscall menu_observe_repaint(void *) { ++g_menu_repaints; }
void __thiscall menu_poison_repaint(void *) { ++g_menu_poisons; }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}  // namespace

void test_menu_accessors() {
    for (MenuProc proc : {static_cast<MenuProc>(nullptr), &menu_proc_fixture}) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(expected, 16 + 0xA14, proc);
            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            if (adapter) {
                expect(menu_set_menu_proc_redirect(menu, nullptr, proc) == proc);
            } else {
                menu->set_menu_proc(proc);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    struct LookupCase {
        int target_index;
        int id;
    };
    const LookupCase lookup_cases[] = {
        {0, INT_MIN}, {7, 17}, {14, INT_MAX},
    };
    for (const LookupCase &fixture : lookup_cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            for (int index = 0; index < 15; ++index) {
                const int id = 1000 + index;
                write_at(storage, 16 + 0xA38 + index * 0x14, id);
            }
            write_at(storage, 16 + 0xA38 + fixture.target_index * 0x14,
                     fixture.id);
            const int ignored_count = INT_MIN;
            write_at(storage, 16 + 0xA18, ignored_count);
            std::memcpy(expected, storage, sizeof(storage));
            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            const int result = adapter
                ? menu_id_to_index_redirect(menu, nullptr, fixture.id)
                : menu->id_to_index(fixture.id);
            expect(result == fixture.target_index);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    for (int miss_case = 0; miss_case < 4; ++miss_case) {
        alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        for (int index = 0; index < 15; ++index) {
            const int id = 1000 + index;
            write_at(storage, 16 + 0xA38 + index * 0x14, id);
        }
        int requested = 77;
        if (miss_case == 0) {
            const int sentinel = -1;
            write_at(storage, 16 + 0xA38 + 2 * 0x14, sentinel);
            write_at(storage, 16 + 0xA38 + 3 * 0x14, requested);
        } else if (miss_case == 1) {
            requested = -1;
        } else if (miss_case == 2) {
            write_at(storage, 16 + 0xA38 + 1 * 0x14, requested);
            write_at(storage, 16 + 0xA38 + 8 * 0x14, requested);
        } else {
            requested = 999;
        }
        std::memcpy(expected, storage, sizeof(storage));
        auto *menu = reinterpret_cast<Menu *>(storage + 16);
        const int result = menu->id_to_index(requested);
        const int expected_result = miss_case == 2 ? 1 : -1;
        expect(result == expected_result);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // The seven menu-item operations. They are 84-byte clones of one another
    // differing only in which PullDown method they end on, so the table drives
    // all seven through the same three questions: a hit dispatches to THAT
    // method on the entry's own PullDown, a miss answers 0xB without
    // dispatching, and neither writes through the Menu.
    struct ItemOp {
        int (Menu::*method)(int, int);
        int (__fastcall *redirect)(Menu *, void *, int, int);
        // What a successful dispatch answers. Six of the seven end in a
        // PullDown method that returns 0 once it finds the item; UNK3 ends in
        // PullDown::UNK2, which is itself a stub returning 8 whatever it is
        // handed. Both are "dispatched" and neither is 0xB, which is what a
        // search miss answers.
        int hit_answer;
    };
    const ItemOp item_ops[] = {
        {&Menu::UNK3, &menu_unk3_redirect, 8},
        {&Menu::hide_menu_item, &menu_hide_menu_item_redirect,
         0},
        {&Menu::show_menu_item, &menu_show_menu_item_redirect,
         0},
        {&Menu::disable_menu_item, &menu_disable_menu_item_redirect,
         0},
        {&Menu::enable_menu_item, &menu_enable_menu_item_redirect,
         0},
        {&Menu::check_menu_item, &menu_check_menu_item_redirect,
         0},
        {&Menu::uncheck_menu_item, &menu_uncheck_menu_item_redirect,
         0},
    };
    // The four flag operations. Same inlined search, but they end by writing
    // ONE bit of the entry's flag byte and repainting through the Menu's own
    // vtable slot 0xF8. The fixture installs a fake vtable so that dispatch
    // is observable, and poisons every other slot so reaching the delegate at
    // all proves the slot literal.
    struct FlagOp {
        int (Menu::*method)(int);
        int (__fastcall *redirect)(Menu *, void *, int);
        uint8_t set;      // bits this operation turns on
        uint8_t cleared;  // bits it turns off
    };
    const FlagOp flag_ops[] = {
        {&Menu::UNK6, &menu_unk6_redirect, 0x00, 0x01},
        {&Menu::UNK7, &menu_unk7_redirect, 0x01, 0x00},
        {&Menu::UNK8, &menu_unk8_redirect, 0x02, 0x00},
        {&Menu::UNK9, &menu_unk9_redirect, 0x00, 0x02},
    };
    for (const FlagOp &op : flag_ops) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            void *vtable[0x100] = {};
            vtable[0xF8 / sizeof(void *)] =
                reinterpret_cast<void *>(&menu_observe_repaint);
            for (size_t slot = 0; slot < 0x100; ++slot) {
                if (slot != 0xF8 / sizeof(void *)) {
                    vtable[slot] = reinterpret_cast<void *>(&menu_poison_repaint);
                }
            }
            auto *menu = reinterpret_cast<Menu *>(storage + 16);
            auto build = [&](int match_index) {
                seed_storage(storage, expected, sizeof(storage));
                write_at(storage, 16, static_cast<void *>(vtable));
                for (int index = 0; index < 15; ++index) {
                    write_at(storage, 16 + 0xA38 + index * 0x14,
                             index == match_index ? 777 : 500 + index);
                    // Every flag byte starts 0xFF, so a set is invisible and
                    // only a CLEAR shows - and then 0x00, so only a SET does.
                    storage[16 + 0xA40 + index * 0x14] = 0xFF;
                }
                std::memcpy(expected, storage, sizeof(storage));
            };
            auto invoke = [&](int menu_id) {
                return adapter ? op.redirect(menu, nullptr, menu_id)
                               : (menu->*op.method)(menu_id);
            };

            // Entry 4 matches. From all-ones, only the cleared bits move.
            build(4);
            expected[16 + 0xA40 + 4 * 0x14] =
                static_cast<uint8_t>(0xFF & ~op.cleared);
            g_menu_repaints = 0;
            g_menu_poisons = 0;
            expect(invoke(777) == 0);
            expect(g_menu_repaints == 1);
            expect(g_menu_poisons == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // From all-zeroes, only the set bits move - which is what tells
            // the two `or` siblings apart from the two `and` ones.
            build(4);
            for (int index = 0; index < 15; ++index) {
                storage[16 + 0xA40 + index * 0x14] = 0x00;
            }
            std::memcpy(expected, storage, sizeof(storage));
            expected[16 + 0xA40 + 4 * 0x14] = op.set;
            g_menu_repaints = 0;
            expect(invoke(777) == 0);
            expect(g_menu_repaints == 1);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // Match on entry ZERO. Starting the walk anywhere else misses
            // it, which is the only thing that pins the initial index - and
            // the reason this case exists is that the sibling fixture above
            // already needed it and this one was written without it.
            build(0);
            expected[16 + 0xA40] = static_cast<uint8_t>(0xFF & ~op.cleared);
            g_menu_repaints = 0;
            expect(invoke(777) == 0);
            expect(g_menu_repaints == 1);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // A -1 SENTINEL at entry 0, with the match sitting behind it.
            // Nothing else exercises the sentinel test or the 0xB it exits
            // through; a table of ordinary ids reaches neither.
            build(3);
            write_at(storage, 16 + 0xA38, -1);
            std::memcpy(expected, storage, sizeof(storage));
            g_menu_repaints = 0;
            expect(invoke(777) == 0xB);
            expect(g_menu_repaints == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // A miss writes no flag and does not repaint.
            build(-1);
            g_menu_repaints = 0;
            expect(invoke(999) == 0xB);
            expect(g_menu_repaints == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // The overread trap again: a sixteenth entry at sizeof(Menu).
            build(-1);
            write_at(storage, 16 + 0xB64, 777);
            storage[16 + 0xB6C] = 0xFF;
            std::memcpy(expected, storage, sizeof(storage));
            g_menu_repaints = 0;
            expect(invoke(777) == 0xB);
            expect(g_menu_repaints == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // The PullDown these dispatch into is seeded with ONE item whose id is 7
    // and a -1 sentinel behind it. That matters: every one of the six
    // PullDown methods answers 11 when it cannot find the item, and 11 is
    // also what Menu answers on a search miss - so an unseeded PullDown makes
    // "dispatched" and "did not dispatch" indistinguishable. With the item
    // present the dispatch answers 0 instead.
    // PullDownItem is 0x14 bytes with id at +8; items_ starts at 0xA18.
    constexpr size_t kItems = 0xA18;
    constexpr size_t kItemId = 8;
    auto seed_pulldown = [&](uint8_t *raw) {
        std::memset(raw, 0, sizeof(PullDown));
        write_at(raw, kItems + kItemId, 7);
        write_at(raw, kItems + 0x14 + kItemId, -1);
    };
    for (const ItemOp &op : item_ops) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            alignas(PullDown) uint8_t pull_storage[sizeof(PullDown)];
            auto *pull = reinterpret_cast<PullDown *>(pull_storage);

            // A. Match on entry 0. Starting the walk anywhere but zero misses
            //    it, so this is what pins the initial index.
            alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
            uint8_t expected[sizeof(storage)];
            auto build = [&](int match_index, bool sentinel_after) {
                seed_storage(storage, expected, sizeof(storage));
                for (int index = 0; index < 15; ++index) {
                    int id = 500 + index;
                    if (index == match_index) { id = 777; }
                    else if (sentinel_after && index == match_index + 1) { id = -1; }
                    write_at(storage, 16 + 0xA38 + index * 0x14, id);
                    write_at(storage, 16 + 0xA48 + index * 0x14,
                             index == match_index ? pull : nullptr);
                }
                std::memcpy(expected, storage, sizeof(storage));
            };
            auto invoke = [&](int menu_id) {
                auto *menu = reinterpret_cast<Menu *>(storage + 16);
                return adapter ? op.redirect(menu, nullptr, menu_id, 7)
                               : (menu->*op.method)(menu_id, 7);
            };

            build(0, true);
            seed_pulldown(pull_storage);
            expect(invoke(777) == op.hit_answer);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // B. Match on the LAST entry, no sentinel anywhere. The walk has
            //    to reach index 14 without tripping the bound.
            build(14, false);
            seed_pulldown(pull_storage);
            expect(invoke(777) == op.hit_answer);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // C. No match and no sentinel: the walk runs off the end of the
            //    fifteen entries. Every entry's PullDown is null, so a body
            //    that dispatched anyway would fault rather than answer.
            //
            //    A SIXTEENTH entry is planted immediately past the array -
            //    entries_[15] lands at exactly sizeof(Menu), 0xB64 - and it
            //    matches. The correct body never reads it; a bound of `> 15`
            //    instead of `>= 15` reads one entry too far, finds it, and
            //    dispatches. Without this the two answer 0xB alike and the
            //    off-by-one survives, which is how it survived the first
            //    version of this fixture.
            build(-1, false);
            write_at(storage, 16 + 0xB64, 777);
            write_at(storage, 16 + 0xB74, pull);
            std::memcpy(expected, storage, sizeof(storage));
            seed_pulldown(pull_storage);
            expect(invoke(777) == 0xB);
            expect_storage_bytes(storage, expected, sizeof(storage));

            // D. Sentinel at entry 0 ends the walk before the match behind
            //    it. Reading the sentinel test backwards would find entry 2.
            build(2, false);
            write_at(storage, 16 + 0xA38, -1);
            std::memcpy(expected, storage, sizeof(storage));
            expect(invoke(777) == 0xB);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // The loop bound is 15 real entries; index 15 sits just past entries_[14]
    // in the trailing canary region, which sizeof(Menu) + 32 keeps safely
    // addressable. Planting the requested ID only there distinguishes the
    // correct `< 15` bound from an off-by-one `<= 15`: the former must return
    // -1 having never read that slot, while the latter would wrongly report
    // it found.
    {
        alignas(Menu) uint8_t storage[sizeof(Menu) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        for (int index = 0; index < 15; ++index) {
            const int id = 1000 + index;
            write_at(storage, 16 + 0xA38 + index * 0x14, id);
        }
        const int requested = 55;
        write_at(storage, 16 + 0xA38 + 15 * 0x14, requested);
        std::memcpy(expected, storage, sizeof(storage));
        auto *menu = reinterpret_cast<Menu *>(storage + 16);
        expect(menu->id_to_index(requested) == -1);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
}

void expect_text_constructor_state(uint8_t *expected) {
    const uint32_t zero = 0;
    expected[16] = 0;
    write_at(expected, 16 + 0x150, zero);
    write_at(expected, 16 + 0x154, zero);
    write_at(expected, 16 + 0x158, zero);
    write_at(expected, 16 + 0x15C, zero);
}

void test_text_constructors() {
    static_assert(sizeof(Text) == 0x160, "Text fixture requires the legacy layout");
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expect_text_constructor_state(expected);
    auto *text = new (storage + 16) Text;
    expect_storage_bytes(storage, expected, sizeof(storage));
    text->~Text();

    struct AllocationScenario {
        bool first_succeeds;
        bool second_succeeds;
        int expected_calls;
    };
    const AllocationScenario scenarios[] = {
        {false, false, 1},
        {true, false, 2},
        {true, true, 2},
    };
    constexpr size_t requested_size = 0x1357;
    for (const AllocationScenario &scenario : scenarios) {
        LPVOID first = scenario.first_succeeds ? std::malloc(1) : nullptr;
        LPVOID second = scenario.second_succeeds ? std::malloc(1) : nullptr;
        mem_get_scripted = true;
        mem_get_calls = 0;
        mem_get_sizes[0] = 0;
        mem_get_sizes[1] = 0;
        mem_get_results[0] = first;
        mem_get_results[1] = second;

        seed_storage(storage, expected, sizeof(storage));
        expect_text_constructor_state(expected);
        write_at(expected, 16 + 0x158, first);
        write_at(expected, 16 + 0x15C, second);
        text = new (storage + 16) Text(requested_size);

        expect(mem_get_calls == scenario.expected_calls);
        expect(mem_get_sizes[0] == requested_size);
        if (scenario.expected_calls == 2) {
            expect(mem_get_sizes[1] == requested_size);
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
        text->~Text();
        std::free(first);
        std::free(second);
    }
    mem_get_scripted = false;
    mem_get_calls = 0;
}

void test_text_global_lifecycle() {
    alignas(Text) static uint8_t exit_storage[sizeof(Text)];
    uint8_t expected[sizeof(exit_storage)];
    seed_storage(exit_storage, expected, sizeof(exit_storage));
    expected[0] = 0;
    const uint32_t zero = 0;
    write_at(expected, 0x150, zero);
    write_at(expected, 0x154, zero);

    LPVOID first = std::malloc(1);
    LPVOID second = std::malloc(1);
    write_at(expected, 0x158, first);
    write_at(expected, 0x15C, second);
    Txt = reinterpret_cast<Text *>(exit_storage);
    mem_get_scripted = true;
    mem_get_calls = 0;
    mem_get_results[0] = first;
    mem_get_results[1] = second;
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    atexit_calls = 0;
    atexit_callback = nullptr;

    text_txt();
    expect(mem_get_calls == 2);
    expect(mem_get_sizes[0] == 512);
    expect(mem_get_sizes[1] == 512);
#if defined(__MINGW32__)
    expect(atexit_calls == 1);
    expect(atexit_callback == text_txt_exit);
#endif
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));

    text_txt_exit();
    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == Txt);
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));

    std::free(first);
    std::free(second);

    seed_storage(exit_storage, expected, sizeof(exit_storage));
    expected[0] = 0;
    write_at(expected, 0x150, zero);
    write_at(expected, 0x154, zero);
    write_at(expected, 0x158, zero);
    write_at(expected, 0x15C, zero);
    mem_get_calls = 0;
    mem_get_results[0] = nullptr;
    mem_get_results[1] = reinterpret_cast<LPVOID>(0x13572468U);
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    atexit_calls = 0;
    atexit_callback = nullptr;

    text_txt();
    expect(mem_get_calls == 1);
    expect(mem_get_sizes[0] == 512);
#if defined(__MINGW32__)
    expect(atexit_calls == 1);
    expect(atexit_callback == text_txt_exit);
#endif
    expect_storage_bytes(exit_storage, expected, sizeof(exit_storage));
    text_txt_exit();
    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == Txt);

    mem_get_scripted = false;
    mem_get_calls = 0;
    new (exit_storage) Text;
}

void test_text_destructor_thunk() {
    static_assert(sizeof(Text) == 0x160, "Text fixture requires the legacy layout");
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    auto *text = new (storage + 16) Text;
    std::memcpy(expected, storage, sizeof(storage));
    text_shutdown_calls = 0;
    text_shutdown_this = nullptr;
    text->~Text();

    expect(text_shutdown_calls == 1);
    expect(text_shutdown_this == text);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_text_open_wrapper() {
    alignas(Text) uint8_t storage[sizeof(Text) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    auto *text = new (storage + 16) Text;
    std::memcpy(expected, storage, sizeof(storage));
    Txt = text;
    char source[] = "fixture.txt";
    char section[] = "SECTION";
    text_open_calls = 0;
    text_open_this = nullptr;
    text_open_source_arg = nullptr;
    text_open_section_arg = nullptr;
    text_open_result = static_cast<BOOL>(0x13572468U);

    expect(text_open(source, section) == text_open_result);
    expect(text_open_calls == 1);
    expect(text_open_this == text);
    expect(text_open_source_arg == source);
    expect(text_open_section_arg == section);
    expect_storage_bytes(storage, expected, sizeof(storage));

    text_open_result = FALSE;
    expect(text_open(nullptr, nullptr) == FALSE);
    expect(text_open_calls == 2);
    expect(text_open_this == text);
    expect(text_open_source_arg == nullptr);
    expect(text_open_section_arg == nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Txt = nullptr;
    text->~Text();
}

}  // namespace

void expect_heap_clear(uint8_t *expected, size_t heap_offset) {
    expected[heap_offset] = 0;
    LPVOID null_pointer = nullptr;
    const size_t zero = 0;
    write_at(expected, heap_offset + 4, null_pointer);
    write_at(expected, heap_offset + 8, null_pointer);
    write_at(expected, heap_offset + 12, zero);
    write_at(expected, heap_offset + 16, zero);
}

namespace {

void test_text_index_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Heap) == 0x14, "Heap fixture requires the legacy layout");
    static_assert(sizeof(TextIndex) == 0x118, "TextIndex fixture requires the legacy layout");

    alignas(TextIndex) uint8_t storage[sizeof(TextIndex) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    const uint32_t zero_count = 0;
    write_at(expected, 16 + 0x100, zero_count);
    expected[16] = 0;
    expect_heap_clear(expected, 16 + 0x104);

    auto *index = new (storage + 16) TextIndex;
    int phase_failures = failures;
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != phase_failures) {
        report_storage_mismatch("TextIndex constructor", storage, expected, sizeof(storage));
    }

    seed_storage(storage, expected, sizeof(storage));
    const uint32_t section_count = 3;
    LPVOID base = std::malloc(8);
    LPVOID current = base;
    const size_t base_size = 8;
    const size_t free_size = 4;
    write_at(storage, 16 + 0x100, section_count);
    write_at(storage, 16 + 0x108, base);
    write_at(storage, 16 + 0x10C, current);
    write_at(storage, 16 + 0x110, base_size);
    write_at(storage, 16 + 0x114, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    write_at(expected, 16 + 0x100, zero_count);
    expect_heap_clear(expected, 16 + 0x104);
    heap_shutdown_calls = 0;
    index->~TextIndex();
    expect(heap_shutdown_calls == 1);
    phase_failures = failures;
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != phase_failures) {
        report_storage_mismatch("TextIndex destructor", storage, expected, sizeof(storage));
    }
    if (failures != failures_before) {
        std::fprintf(stderr, "TextIndex lifecycle fixture failed\n");
    }
}

void test_text_clear_index() {
    const int failures_before = failures;
    constexpr size_t entry_size = 0x118;
    const uint32_t zero_count = 0;
    alignas(TextIndex) uint8_t storage[MaxTextIndexNum * entry_size + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        new (storage + 16 + static_cast<size_t>(i) * entry_size) TextIndex;
    }
    const uint32_t counts[MaxTextIndexNum] = {0, 1, UINT32_MAX, 0};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        uint8_t *entry = storage + 16 + static_cast<size_t>(i) * entry_size;
        std::memset(entry, 0x41 + i, 0x100);
        write_at(entry, 0x100, counts[i]);
        entry[0x104] = static_cast<uint8_t>(0x51 + i);
        LPVOID null_pointer = nullptr;
        LPVOID poison_current = reinterpret_cast<LPVOID>(
            0x11110000U + static_cast<uint32_t>(i));
        const size_t base_size = static_cast<size_t>(0x2000 + i);
        const size_t free_size = static_cast<size_t>(0x1000 + i);
        write_at(entry, 0x108, null_pointer);
        write_at(entry, 0x10C, poison_current);
        write_at(entry, 0x110, base_size);
        write_at(entry, 0x114, free_size);
    }
    std::memcpy(expected, storage, sizeof(storage));
    for (int i = 1; i <= 2; ++i) {
        const size_t entry_offset = 16 + static_cast<size_t>(i) * entry_size;
        write_at(expected, entry_offset + 0x100, zero_count);
        expect_heap_clear(expected, entry_offset + 0x104);
    }

    heap_shutdown_calls = 0;
    text_clear_index_source(reinterpret_cast<TextIndex *>(storage + 16));
    expect(heap_shutdown_calls == 2);
    expect_storage_bytes(storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "TextIndex clear fixture failed\n");
    }
}

void test_text_index_wrappers() {
    constexpr size_t entry_size = sizeof(TextIndex);
    alignas(TextIndex) uint8_t storage[MaxTextIndexNum * entry_size + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    TxtIndex = reinterpret_cast<TextIndex *>(storage + 16);
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        new (&TxtIndex[i]) TextIndex;
    }

    const uint32_t make_counts[MaxTextIndexNum] = {1, UINT32_MAX, 0, 0};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 make_counts[i]);
    }
    std::memcpy(expected, storage, sizeof(storage));
    char source[] = "fixture";
    text_index_make_calls = 0;
    text_index_make_this = nullptr;
    text_index_make_source = nullptr;
    text_make_index(source);
    expect(text_index_make_calls == 1);
    expect(text_index_make_this == &TxtIndex[2]);
    expect(text_index_make_source == source);
    expect_storage_bytes(storage, expected, sizeof(storage));

    const uint32_t full_counts[MaxTextIndexNum] = {1, 2, 3, 4};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 full_counts[i]);
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_make_calls = 0;
    text_make_index(nullptr);
    expect(text_index_make_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    const uint32_t search_counts[MaxTextIndexNum] = {0, 1, UINT32_MAX, 1};
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 search_counts[i]);
        text_index_search_results[i] = -1;
        text_index_search_this[i] = nullptr;
        text_index_search_source[i] = nullptr;
        text_index_search_section[i] = nullptr;
    }
    text_index_search_results[1] = INT_MIN;
    text_index_search_results[2] = 0;
    text_index_search_results[3] = 0x2468;
    std::memcpy(expected, storage, sizeof(storage));
    char section[] = "SECTION";
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == 0);
    expect(text_index_search_calls == 2);
    expect(text_index_search_this[0] == &TxtIndex[1]);
    expect(text_index_search_this[1] == &TxtIndex[2]);
    expect(text_index_search_source[0] == source);
    expect(text_index_search_source[1] == source);
    expect(text_index_search_section[0] == section);
    expect(text_index_search_section[1] == section);
    expect_storage_bytes(storage, expected, sizeof(storage));

    text_index_search_results[1] = 0x2468;
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == 0x2468);
    expect(text_index_search_calls == 1);
    expect(text_index_search_this[0] == &TxtIndex[1]);
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100,
                 full_counts[i]);
        text_index_search_results[i] = i == 0 ? INT_MIN : -i;
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_search_calls = 0;
    expect(text_search_index(source, section) == -1);
    expect(text_index_search_calls == MaxTextIndexNum);
    for (int i = 0; i < MaxTextIndexNum; ++i) {
        expect(text_index_search_this[i] == &TxtIndex[i]);
        expect(text_index_search_source[i] == source);
        expect(text_index_search_section[i] == section);
    }
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        const uint32_t zero = 0;
        write_at(storage, 16 + static_cast<size_t>(i) * entry_size + 0x100, zero);
    }
    std::memcpy(expected, storage, sizeof(storage));
    text_index_search_calls = 0;
    expect(text_search_index(nullptr, nullptr) == -1);
    expect(text_index_search_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    for (int i = 0; i < MaxTextIndexNum; ++i) {
        TxtIndex[i].~TextIndex();
    }
    TxtIndex = nullptr;
}

void test_spot_lifecycle() {
    static_assert(sizeof(Spot) == 0xC, "Spot fixture requires the legacy layout");
    alignas(Spot) uint8_t storage[sizeof(Spot) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    auto *spot = new (storage + 16) Spot;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    spot->clear();
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID spots = std::malloc(8);
    const uint32_t max_count = 7;
    const uint32_t add_count = 5;
    write_at(storage, 16, spots);
    write_at(storage, 16 + 4, max_count);
    write_at(storage, 16 + 8, add_count);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    tracked_free_pointer = spots;
    tracked_free_calls = 0;
    spot->~Spot();
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    spot = new (storage + 16) Spot;
    write_at(storage, 16 + 4, max_count);
    write_at(storage, 16 + 8, add_count);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Spot));
    tracked_free_pointer = reinterpret_cast<void *>(0x87654321U);
    tracked_free_calls = 0;
    spot->~Spot();
    expect_tracked_free_calls(0);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void expect_font_close(uint8_t *expected, size_t offset) {
    const int negative_one = -1;
    const int zero = 0;
    HGDIOBJ null_font = nullptr;
    LPSTR null_file = nullptr;
    write_at(expected, offset, negative_one);
    write_at(expected, offset + 8, null_font);
    write_at(expected, offset + 0xC, zero);
    write_at(expected, offset + 0x10, zero);
    write_at(expected, offset + 0x18, zero);
    write_at(expected, offset + 0x1C, zero);
    write_at(expected, offset + 0x24, null_file);
}

void test_font_lifecycle() {
    static_assert(sizeof(Font) == 0x28, "Font fixture requires the legacy layout");
    alignas(Font) uint8_t storage[sizeof(Font) + 32];
    uint8_t expected[sizeof(storage)];
    const int negative_one = -1;
    const int zero = 0;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, negative_one);
    for (size_t offset : {4U, 8U, 0xCU, 0x10U, 0x18U, 0x1CU, 0x24U}) {
        write_at(expected, 16 + offset, zero);
    }
    auto *font = new (storage + 16) Font;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    char font_name[] = "Fixture Font";
    const int height = -17;
    const int style = static_cast<int>(0x80000005U);
    font_init_calls = 0;
    font_init_this = nullptr;
    font_init_name = nullptr;
    font_init_height = 0;
    font_init_style = 0;
    font = new (storage + 16) Font(font_name, height, style);
    expect(font == reinterpret_cast<Font *>(storage + 16));
    expect(font_init_calls == 1);
    expect(font_init_this == font);
    expect(font_init_name == font_name);
    expect(font_init_height == height);
    expect(font_init_style == static_cast<uint32_t>(style));
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    HGDIOBJ font_object = reinterpret_cast<HGDIOBJ>(1);
    LPSTR file_name = static_cast<LPSTR>(std::malloc(8));
    write_at(storage, 16 + 8, font_object);
    write_at(storage, 16 + 0x24, file_name);
    std::memcpy(expected, storage, sizeof(storage));
    expect_font_close(expected, 16);
    font_close_calls = 0;
    font->~Font();
    expect(font_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    font = new (storage + 16) Font;
    HGDIOBJ null_font = nullptr;
    LPSTR null_file = nullptr;
    write_at(storage, 16 + 8, null_font);
    write_at(storage, 16 + 0x24, null_file);
    std::memcpy(expected, storage, sizeof(storage));
    expect_font_close(expected, 16);
    font_close_calls = 0;
    font->~Font();
    expect(font_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void expect_time_closed(uint8_t *expected, size_t offset) {
    const uint32_t resolution = 5;
    std::memset(expected + offset, 0, sizeof(Time));
    write_at(expected, offset + 0x20, resolution);
}

void test_time_lifecycle_and_modal() {
    static_assert(sizeof(Time) == 0x28, "Time fixture requires the legacy layout");
    alignas(Time) uint8_t storage[sizeof(Time) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expect_time_closed(expected, 16);
    auto *timer = new (storage + 16) Time;
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    expect_time_closed(expected, 16);
    time_close_calls = 0;
    timer->~Time();
    expect(time_close_calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    timer = new (storage + 16) Time;
    std::memcpy(expected, storage, sizeof(storage));
    Time::TimeModal = nullptr;
    timer->set_modal();
    expect(Time::TimeModal == timer);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Time::TimeModal = reinterpret_cast<Time *>(1);
    timer->release_modal();
    expect(Time::TimeModal == nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));

    Time::TimeInitCount = 0;
    expect(Time::init_class() == 0);
    expect(Time::TimeInitCount == 1);
    Time::close_class();
    expect(Time::TimeInitCount == 0);

    Time::TimeInitCount = INT_MAX;
    expect(Time::init_class() == 0);
    expect(Time::TimeInitCount == INT_MIN);
    Time::TimeInitCount = INT_MIN;
    Time::close_class();
    expect(Time::TimeInitCount == INT_MAX);
}

void test_filemap_lifecycle() {
    static_assert(sizeof(Filemap) == 0x10, "Filemap fixture requires the legacy layout");
    alignas(Filemap) uint8_t storage[sizeof(Filemap) + 32];
    uint8_t expected[sizeof(storage)];
    LPVOID null_view = nullptr;
    HANDLE invalid_file = INVALID_HANDLE_VALUE;
    HANDLE null_handle = nullptr;

    seed_storage(storage, expected, sizeof(storage));
    write_at(expected, 16, null_view);
    write_at(expected, 16 + 4, invalid_file);
    write_at(expected, 16 + 8, null_handle);
    auto *filemap = new (storage + 16) Filemap;
    expect_storage_bytes(storage, expected, sizeof(storage));

    HANDLE mapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 4096, nullptr);
    LPVOID view = mapping ? MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 4096) : nullptr;
    HANDLE file = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    expect(mapping != nullptr);
    expect(view != nullptr);
    expect(file != nullptr);
    if (mapping && view && file) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16, view);
        write_at(storage, 16 + 4, file);
        write_at(storage, 16 + 8, mapping);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16, null_view);
        write_at(expected, 16 + 4, null_handle);
        write_at(expected, 16 + 8, null_handle);
        filemap->~Filemap();
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(!UnmapViewOfFile(view));
        expect(!CloseHandle(file));
        expect(!CloseHandle(mapping));
    } else {
        if (view) {
            UnmapViewOfFile(view);
        }
        if (mapping) {
            CloseHandle(mapping);
        }
        if (file) {
            CloseHandle(file);
        }
    }

    seed_storage(storage, expected, sizeof(storage));
    filemap = new (storage + 16) Filemap;
    std::memcpy(expected, storage, sizeof(storage));
    filemap->~Filemap();
    expect_storage_bytes(storage, expected, sizeof(storage));
}

void test_heap_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Heap) == 0x14, "Heap fixture requires the legacy layout");
    alignas(Heap) uint8_t storage[sizeof(Heap) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    auto *heap = new (storage + 16) Heap;
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap constructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID base = std::malloc(8);
    const size_t base_size = 0x12345678U;
    const size_t free_size = 0x87654321U;
    write_at(storage, 16 + 4, base);
    write_at(storage, 16 + 8, base);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = base;
    tracked_free_calls = 0;
    heap->~Heap();
    expect(heap_shutdown_calls == 0);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap allocated destructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID null_base = nullptr;
    LPVOID poison_current = reinterpret_cast<LPVOID>(0x12345678U);
    write_at(storage, 16 + 4, null_base);
    write_at(storage, 16 + 8, poison_current);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = reinterpret_cast<void *>(0x87654321U);
    tracked_free_calls = 0;
    heap->~Heap();
    expect(heap_shutdown_calls == 0);
    expect_tracked_free_calls(0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Heap empty destructor", storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "Heap lifecycle fixture failed\n");
    }
}

void test_strings_lifecycle() {
    const int failures_before = failures;
    static_assert(sizeof(Strings) == 0x18, "Strings fixture requires the legacy layout");
    alignas(Strings) uint8_t storage[sizeof(Strings) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Strings) - 4);
    auto *strings = new (storage + 16) Strings;
    expect(strings == reinterpret_cast<Strings *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Strings constructor", storage, expected, sizeof(storage));

    seed_storage(storage, expected, sizeof(storage));
    LPVOID base = std::malloc(8);
    const size_t base_size = 0x12345678U;
    const size_t free_size = 0x87654321U;
    const BOOL is_populated = static_cast<BOOL>(0x13572468U);
    write_at(storage, 16 + 4, base);
    write_at(storage, 16 + 8, base);
    write_at(storage, 16 + 0xC, base_size);
    write_at(storage, 16 + 0x10, free_size);
    write_at(storage, 16 + 0x14, is_populated);
    std::memcpy(expected, storage, sizeof(storage));
    expected[16] = 0;
    std::memset(expected + 16 + 4, 0, sizeof(Heap) - 4);
    heap_shutdown_calls = 0;
    tracked_free_pointer = base;
    tracked_free_calls = 0;
    strings->~Strings();
    expect(heap_shutdown_calls == 1);
    expect_tracked_free_calls(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Strings destructor", storage, expected, sizeof(storage));
    if (failures != failures_before) {
        std::fprintf(stderr, "Strings lifecycle fixture failed\n");
    }
}

uint32_t expected_random_seed(uint32_t seed) {
    return seed * 0x19660DU + 0x3C6EF35FU;
}

uint32_t expected_random_int(uint32_t seed, uint32_t min, uint32_t max) {
    if (static_cast<int32_t>(min) > static_cast<int32_t>(max)) {
        const uint32_t temporary = min;
        min = max;
        max = temporary;
    }
    const uint32_t range = max - min;
    return ((range * (expected_random_seed(seed) & 0xFFFFU)) >> 16) + min;
}

double expected_random_double(uint32_t seed) {
    const uint32_t bits = (expected_random_seed(seed) & 0x7FFFFFU) | 0x3F800000U;
    float unit = 0.0f;
    std::memcpy(&unit, &bits, sizeof(unit));
    return static_cast<double>(unit) - 1.0;
}

void test_random_exports() {
    const int failures_before = failures;
    static_assert(sizeof(Random) == 4, "Random fixture requires the legacy layout");
    alignas(Random) uint8_t storage[sizeof(Random) + 32];
    uint8_t expected[sizeof(storage)];

    seed_storage(storage, expected, sizeof(storage));
    std::memset(expected + 16, 0, sizeof(Random));
    auto *generator = new (storage + 16) Random;
    expect(generator == reinterpret_cast<Random *>(storage + 16));
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Random constructor", storage, expected, sizeof(storage));

    const uint32_t destructor_seed = 0x89ABCDEFU;
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16, destructor_seed);
    std::memcpy(expected, storage, sizeof(storage));
    const uint32_t zero = 0;
    write_at(expected, 16, zero);
    generator->~Random();
    expect_storage_bytes(storage, expected, sizeof(storage));
    report_storage_mismatch("Random destructor", storage, expected, sizeof(storage));

    struct IntegerFixture {
        uint32_t seed;
        uint32_t min;
        uint32_t max;
    };
    const IntegerFixture integer_fixtures[] = {
        {0, 0, 100},
        {0xFFFFFFFFU, 100, 0},
        {0x12345678U, static_cast<uint32_t>(-10), 10},
        {0x87654321U, 10, static_cast<uint32_t>(-10)},
        {0xDEADBEEFU, static_cast<uint32_t>(INT_MIN), static_cast<uint32_t>(INT_MAX)},
        {0xCAFEBABEU, 17, 17},
    };
    for (const IntegerFixture &fixture : integer_fixtures) {
        generator->reseed(fixture.seed);
        expect(generator->get(fixture.min, fixture.max)
               == expected_random_int(fixture.seed, fixture.min, fixture.max));
        expect(generator->get_seed() == expected_random_seed(fixture.seed));
    }

    Rand = generator;
    generator->reseed(0xA5A5A5A5U);
    expect(random_get() == 0xA5A5A5A5U);
    expect(generator->get_seed() == 0xA5A5A5A5U);
    const uint32_t wrapper_min = static_cast<uint32_t>(-200);
    const uint32_t wrapper_max = 300;
    expect(random(wrapper_min, wrapper_max)
           == expected_random_int(0xA5A5A5A5U, wrapper_min, wrapper_max));
    expect(generator->get_seed() == expected_random_seed(0xA5A5A5A5U));

    const uint32_t floating_seeds[] = {
        0, 1, 0x12345678U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t seed : floating_seeds) {
        generator->reseed(seed);
        _clearfp();
        const double actual = generator->get();
        const unsigned int status = _statusfp();
        _clearfp();
        const double expected_value = expected_random_double(seed);
        expect(std::memcmp(&actual, &expected_value, sizeof(actual)) == 0);
        expect(status == 0);
        expect(generator->get_seed() == expected_random_seed(seed));

        generator->reseed(seed);
        _clearfp();
        const double wrapper_actual = random();
        const unsigned int wrapper_status = _statusfp();
        _clearfp();
        expect(std::memcmp(&wrapper_actual, &expected_value, sizeof(wrapper_actual)) == 0);
        expect(wrapper_status == 0);
        expect(generator->get_seed() == expected_random_seed(seed));
    }

    static Random exit_generator;
    Rand = &exit_generator;
    exit_generator.reseed(0x55555555U);
    atexit_calls = 0;
    atexit_callback = nullptr;
    random_rand();
    expect(exit_generator.get_seed() == 0);
    expect(atexit_calls == 1);
    expect(atexit_callback == random_rand_exit);
    random_reseed(0xABCDEF01U);
    expect(exit_generator.get_seed() == 0xABCDEF01U);
    random_rand_exit();
    expect(exit_generator.get_seed() == 0);
    if (failures != failures_before) {
        std::fprintf(stderr, "Random export fixture failed\n");
    }
}

}  // namespace

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(72, test_menu_accessors);
LEAF_CASE(75, test_text_constructors);
LEAF_CASE(76, test_text_global_lifecycle);
LEAF_CASE(77, test_text_destructor_thunk);
LEAF_CASE(78, test_text_open_wrapper);
LEAF_CASE(79, test_text_index_lifecycle);
LEAF_CASE(80, test_text_clear_index);
LEAF_CASE(81, test_text_index_wrappers);
LEAF_CASE(82, test_spot_lifecycle);
LEAF_CASE(83, test_font_lifecycle);
LEAF_CASE(84, test_time_lifecycle_and_modal);
LEAF_CASE(85, test_filemap_lifecycle);
LEAF_CASE(86, test_heap_lifecycle);
LEAF_CASE(87, test_strings_lifecycle);
LEAF_CASE(88, test_random_exports);
}  // namespace
