// Recovery leaf tests: win_and_graphic.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "win_and_graphic"

void test_win_is_visible() {
    // Visibility is the conjunction of this window's flag bit 0 and the
    // visibility of every ancestor, so chains exercise the recursion.
    struct WinNode {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(Win) + 32];

        Win *object() { return reinterpret_cast<Win *>(storage + 16); }
    };
    struct ChainCase {
        int depth;
        uint32_t flags[3];   // flag dword written at offset 0x9C per node
        int expected;
    };
    const ChainCase cases[] = {
        {1, {0x00000000U, 0, 0}, 0},
        {1, {0x00000001U, 0, 0}, 1},
        {1, {0xFFFFFFFEU, 0, 0}, 0},
        {1, {0xFFFFFFFFU, 0, 0}, 1},
        {1, {0xA55AA55BU, 0, 0}, 1},
        {2, {0x00000001U, 0x00000001U, 0}, 1},
        {2, {0x00000001U, 0x00000000U, 0}, 0},
        {2, {0x00000000U, 0x00000001U, 0}, 0},
        {3, {0x00000001U, 0x00000001U, 0x00000001U}, 1},
        {3, {0x00000001U, 0x00000001U, 0x00000000U}, 0},
        {3, {0x00000001U, 0x00000000U, 0x00000001U}, 0},
        {3, {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU}, 0},
    };
    for (const ChainCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[3];
            for (int index = 0; index < test.depth; ++index) {
                seed_storage(nodes[index].storage, nodes[index].expected,
                             sizeof(nodes[index].storage));
                write_at(nodes[index].storage, 16 + 0x9C, test.flags[index]);
                // Innermost node terminates the chain with a null parent.
                Win *parent = (index + 1 < test.depth)
                    ? nodes[index + 1].object() : nullptr;
                write_at(nodes[index].storage, 16 + 0xC4, parent);
                std::memcpy(nodes[index].expected, nodes[index].storage,
                            sizeof(nodes[index].storage));
            }
            const int result = use_adapter
                ? win_is_visible_redirect(nodes[0].object(), nullptr)
                : nodes[0].object()->is_visible();
            expect(result == test.expected);
            // Visibility is a pure query: no node may be modified.
            for (int index = 0; index < test.depth; ++index) {
                expect_storage_bytes(nodes[index].storage, nodes[index].expected,
                                     sizeof(nodes[index].storage));
            }
        }
    }

    struct WrappingCase {
        uint32_t flags;
        uint32_t start_x;
        uint32_t start_y;
        uint32_t child_client_x;
        uint32_t child_client_y;
        uint32_t child_outer_x;
        uint32_t child_outer_y;
        uint32_t parent_client_x;
        uint32_t parent_client_y;
        uint32_t parent_outer_x;
        uint32_t parent_outer_y;
        uint32_t expected_x;
        uint32_t expected_y;
    };
    const WrappingCase wrapping_cases[] = {
        // The local client and outer additions wrap independently for both axes.
        {0x00000000U,
         0x7FFFFFFFU, 0x80000000U,
         0x00000001U, 0xFFFFFFFFU,
         0x00000001U, 0xFFFFFFFFU,
         0, 0, 0, 0,
         0x80000001U, 0x7FFFFFFEU},
        // The parent's additions receive already adjusted child coordinates.
        {0x00000020U,
         0x7FFFFFFFU, 0x80000000U,
         0, 0, 0, 0,
         0x00000001U, 0xFFFFFFFFU,
         0x00000001U, 0xFFFFFFFFU,
         0x80000001U, 0x7FFFFFFEU},
        // Parent client/outer terms cancel during recursion, leaving the
        // bit-15 subtraction itself to cross the signed boundary.
        {0x00008020U,
         0x80000000U, 0x7FFFFFFFU,
         0, 0, 0, 0,
         0xFFFFFFFFU, 0x00000001U,
         0x00000001U, 0xFFFFFFFFU,
         0x7FFFFFFFU, 0x80000000U},
    };
    for (const WrappingCase &test : wrapping_cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[2];
            for (WinNode &node : nodes) {
                seed_storage(node.storage, node.expected, sizeof(node.storage));
            }
            write_at(nodes[0].storage, 16 + 0x98, test.flags);
            write_at(nodes[0].storage, 16 + 0x14C, test.child_client_x);
            write_at(nodes[0].storage, 16 + 0x150, test.child_client_y);
            write_at(nodes[0].storage, 16 + 0x13C, test.child_outer_x);
            write_at(nodes[0].storage, 16 + 0x140, test.child_outer_y);
            write_at(nodes[0].storage, 16 + 0xC4,
                     (test.flags & 0x20U) ? nodes[1].object() : nullptr);
            write_at(nodes[1].storage, 16 + 0x98, 0U);
            write_at(nodes[1].storage, 16 + 0x14C, test.parent_client_x);
            write_at(nodes[1].storage, 16 + 0x150, test.parent_client_y);
            write_at(nodes[1].storage, 16 + 0x13C, test.parent_outer_x);
            write_at(nodes[1].storage, 16 + 0x140, test.parent_outer_y);
            write_at(nodes[1].storage, 16 + 0xC4, static_cast<Win *>(nullptr));
            for (WinNode &node : nodes) {
                std::memcpy(node.expected, node.storage, sizeof(node.storage));
            }
            int x = int_from_bits(test.start_x);
            int y = int_from_bits(test.start_y);
            if (use_adapter) {
                win_client_to_screen_redirect(nodes[0].object(), nullptr, &x, &y);
            } else {
                nodes[0].object()->client_to_screen(&x, &y);
            }
            expect(static_cast<uint32_t>(x) == test.expected_x);
            expect(static_cast<uint32_t>(y) == test.expected_y);
            for (WinNode &node : nodes) {
                expect_storage_bytes(node.storage, node.expected,
                                     sizeof(node.storage));
            }
        }
    }
}

// Depth-first search over the child array at 0x1A4, whose count lives at
// 0x3FC. The original never null-checks a slot before recursing into it, so
// every populated slot here is a real object - a defensive null check would be
// unfaithful and is deliberately absent from the recovery.
void test_win_is_descendant() {
    struct WinNode {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(Win) + 32];

        Win *object() { return reinterpret_cast<Win *>(storage + 16); }
    };
    WinNode nodes[6];
    auto reset = [&]() {
        for (WinNode &node : nodes) {
            seed_storage(node.storage, node.expected, sizeof(node.storage));
            write_at(node.storage, 16 + 0x3FC, 0);
            std::memcpy(node.expected, node.storage, sizeof(node.storage));
        }
    };
    auto adopt = [&](int parent, std::initializer_list<int> kids) {
        int slot = 0;
        for (int kid : kids) {
            write_at(nodes[parent].storage, 16 + 0x1A4 + slot * 4,
                     nodes[kid].object());
            ++slot;
        }
        write_at(nodes[parent].storage, 16 + 0x3FC, slot);
        std::memcpy(nodes[parent].expected, nodes[parent].storage,
                    sizeof(nodes[parent].storage));
    };
    auto ask = [&](int adapter, int parent, Win *candidate) {
        return adapter ? win_is_descendant_redirect(nodes[parent].object(),
                                                    nullptr, candidate)
                       : nodes[parent].object()->is_descendant(candidate);
    };
    auto untouched = [&]() {
        for (WinNode &node : nodes) {
            expect_storage_bytes(node.storage, node.expected,
                                 sizeof(node.storage));
        }
    };

    for (int adapter = 0; adapter < 2; ++adapter) {
        // A null candidate is rejected before the count is even read.
        reset();
        adopt(0, {1, 2});
        expect(ask(adapter, 0, nullptr) == 0);
        untouched();

        // An empty list, and a NEGATIVE count: the guard is signed (`jle`),
        // so a negative count must not be treated as "some children".
        reset();
        expect(ask(adapter, 0, nodes[1].object()) == 0);
        write_at(nodes[0].storage, 16 + 0x3FC, -1);
        std::memcpy(nodes[0].expected, nodes[0].storage,
                    sizeof(nodes[0].storage));
        expect(ask(adapter, 0, nodes[1].object()) == 0);
        untouched();

        // Direct hit in the first and in the last slot, and the exact 1.
        reset();
        adopt(0, {1, 2, 3});
        expect(ask(adapter, 0, nodes[1].object()) == 1);
        expect(ask(adapter, 0, nodes[3].object()) == 1);
        expect(ask(adapter, 0, nodes[4].object()) == 0);
        untouched();

        // Depth: 0 -> 1 -> 4 -> 5. Only the recursion can find node 5, and
        // the miss has to walk the whole tree before returning 0.
        reset();
        adopt(0, {1, 2});
        adopt(1, {4});
        adopt(4, {5});
        expect(ask(adapter, 0, nodes[5].object()) == 1);
        expect(ask(adapter, 0, nodes[3].object()) == 0);
        // A descendant of a sibling is not a descendant of that sibling.
        expect(ask(adapter, 2, nodes[5].object()) == 0);
        // ...but it is one of its own parent.
        expect(ask(adapter, 4, nodes[5].object()) == 1);
        untouched();

        // A window is not its own descendant unless it is genuinely parked in
        // its own child list; the search starts at the children, not at this.
        reset();
        adopt(0, {1});
        expect(ask(adapter, 0, nodes[0].object()) == 0);
        untouched();
    }
}

namespace {

struct TutShowRecord {
    int calls; void *self; void *window; const char *text;
    int x, y; void *sprite; int flag, a7, a8;
};

TutShowRecord g_tut_show;

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

int __thiscall observe_tut_win_show(void *self, void *window,
                                    const char *text, int x, int y,
                                    void *sprite, int flag, int a7, int a8) {
    ++g_tut_show.calls;
    g_tut_show.self = self;   g_tut_show.window = window;
    g_tut_show.text = text;   g_tut_show.x = x;
    g_tut_show.y = y;         g_tut_show.sprite = sprite;
    g_tut_show.flag = flag;   g_tut_show.a7 = a7;
    g_tut_show.a8 = a8;
    return 0;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

}  // namespace

void test_tut_win_rects() {
    // Four 67-byte clones that centre a rectangle and convert the result
    // through a FIXED window - never through `this`. All four windows default
    // to addresses in the original image that are unmapped here, so each is
    // rebound to its own Win arena, and each arena carries DISTINCT offsets:
    // that is what proves each sibling reaches its own window rather than
    // whichever one happened to be bound last.
    struct RectOp {
        void (TutWin::*method)(RECT *, int *, int *);
        void (__fastcall *redirect)(TutWin *, void *, RECT *, int *, int *);
        Win **window;
    };
    const RectOp rect_ops[] = {
        {&TutWin::iface_rect, &tut_win_iface_rect_redirect, &TutWinIfaceWindow},
        {&TutWin::base_rect, &tut_win_base_rect_redirect, &TutWinBaseWindow},
        {&TutWin::soc_rect, &tut_win_soc_rect_redirect, &TutWinSocWindow},
        {&TutWin::des_rect, &tut_win_des_rect_redirect, &TutWinDesWindow},
    };
    const size_t op_count = sizeof(rect_ops) / sizeof(rect_ops[0]);

    std::vector<std::vector<uint8_t>> arenas(op_count);
    std::vector<Win *> saved(op_count);
    for (size_t index = 0; index < op_count; ++index) {
        arenas[index].assign(sizeof(Win), 0);
        // client_rect_.left/top at 0x14C/0x150, outer_rect_.left/top at
        // 0x13C/0x140 - the same four the client_to_screen fixture uses. Bit 5
        // of 0x98 stays clear so the walk stops here and no parent is needed.
        write_at(arenas[index].data(), 0x14C,
                 static_cast<int32_t>(1000 * (index + 1)));
        write_at(arenas[index].data(), 0x150,
                 static_cast<int32_t>(2000 * (index + 1)));
        write_at(arenas[index].data(), 0x13C,
                 static_cast<int32_t>(10 * (index + 1)));
        write_at(arenas[index].data(), 0x140,
                 static_cast<int32_t>(20 * (index + 1)));
        saved[index] = *rect_ops[index].window;
        *rect_ops[index].window =
            reinterpret_cast<Win *>(arenas[index].data());
    }

    struct Centre { int32_t l, t, r, b; int cx; int cy; };
    const Centre centres[] = {
        {0, 0, 10, 20, 5, 10},
        {10, 20, 30, 40, 20, 30},
        // The centre is `near + (far - near) / 2`, and that division
        // truncates toward ZERO because the original halves with
        // `cdq` / `sub` / `sar` rather than with a bare arithmetic shift. The
        // consequence is that an odd extent biases the answer toward the NEAR
        // edge in both directions - the centre of [-5, 0] is -3, not -2, so
        // an expectation computed as "the true centre truncated toward zero"
        // disagrees with the bytes on exactly these cases.
        {0, 0, 5, 7, 2, 3},
        {-5, -7, 0, 0, -3, -4},
        {-11, -13, -2, -4, -7, -9},
        // A width that overflows a signed subtraction: the original wraps.
        {INT_MIN, INT_MIN, INT_MAX, INT_MAX, INT_MIN + 0, INT_MIN + 0},
        // INVERTED rectangles, where right < left. These are the only cases
        // that exercise the sign bit carried back over the halving: the
        // original shifts with `sar`, and emulating that on an unsigned value
        // needs the top bit restored. Drop it and a negative width halves to
        // an enormous positive one instead, which every non-inverted case
        // above is blind to.
        {10, 20, 0, 0, 5, 10},
        {0, 0, -9, -21, -4, -10},
        {100, 200, 1, 3, 51, 102},
    };
    for (size_t index = 0; index < op_count; ++index) {
        const RectOp &op = rect_ops[index];
        for (const Centre &c : centres) {
            for (int adapter = 0; adapter < 2; ++adapter) {
                alignas(TutWin) uint8_t storage[sizeof(TutWin)];
                uint8_t expected[sizeof(TutWin)];
                seed_storage(storage, expected, sizeof(storage));
                std::memcpy(expected, storage, sizeof(storage));
                auto *tut = reinterpret_cast<TutWin *>(storage);
                RECT rect = {c.l, c.t, c.r, c.b};
                const RECT untouched = rect;
                int x = 0x5A5A;
                int y = 0x3C3C;
                if (adapter) {
                    op.redirect(tut, nullptr, &rect, &x, &y);
                } else {
                    (tut->*op.method)(&rect, &x, &y);
                }
                // The incoming *x and *y are OVERWRITTEN, not accumulated -
                // the centre is stored before client_to_screen adds to it.
                const int32_t shift_x = 1000 * static_cast<int32_t>(index + 1)
                                      + 10 * static_cast<int32_t>(index + 1);
                const int32_t shift_y = 2000 * static_cast<int32_t>(index + 1)
                                      + 20 * static_cast<int32_t>(index + 1);
                expect(x == c.cx + shift_x);
                expect(y == c.cy + shift_y);
                // The rectangle is read, never written.
                expect(rect.left == untouched.left && rect.top == untouched.top);
                expect(rect.right == untouched.right
                       && rect.bottom == untouched.bottom);
                // And the TutWin receiver is not touched at all.
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }

    // The four do_* helpers share those same windows and centring, then hand
    // the result to TutWin::tut_win against the primary map window, virtual-
    // base adjusted. Reusing the arenas above is the point: a sibling that
    // centred through the wrong window fails on the shift, exactly as in the
    // rect cases.
    struct ShowOp {
        void (TutWin::*method)(RECT *, const char *, int);
        void (__fastcall *redirect)(TutWin *, void *, RECT *, const char *, int);
    };
    // ORDERED TO MATCH rect_ops, not to match the addresses. Each entry here
    // must line up with the arena its window was bound to above - do_base uses
    // TutWinBaseWindow, which is rect_ops[1] - and getting that wrong makes
    // every centre assertion fail against the neighbour's offsets.
    const ShowOp show_ops[] = {
        {&TutWin::do_iface, &tut_win_do_iface_redirect},
        {&TutWin::do_base, &tut_win_do_base_redirect},
        {&TutWin::do_soc, &tut_win_do_soc_redirect},
        {&TutWin::do_des, &tut_win_do_des_redirect},
    };
    func_tut_win_show *const saved_show = TutWinOriginalShow;
    MapWin **const saved_table = MapWinTable;
    TutWinOriginalShow = &observe_tut_win_show;

    // A map window whose vbtable displacement is deliberately NON-ZERO, so a
    // body that skipped the adjustment and passed the raw pointer fails.
    std::vector<uint8_t> primary(0x200, 0);
    const int32_t vbtable[2] = {0, 0x40};
    const int32_t *vbtable_pointer = vbtable;
    std::memcpy(primary.data(), &vbtable_pointer, sizeof(vbtable_pointer));
    MapWin *table[MapWinTableSlots] = {};
    table[0] = reinterpret_cast<MapWin *>(primary.data());
    MapWinTable = table;

    for (size_t index = 0; index < op_count; ++index) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            for (int empty_slot = 0; empty_slot < 2; ++empty_slot) {
                table[0] = empty_slot
                    ? nullptr : reinterpret_cast<MapWin *>(primary.data());
                alignas(TutWin) uint8_t storage[sizeof(TutWin)];
                uint8_t expected[sizeof(TutWin)];
                seed_storage(storage, expected, sizeof(storage));
                std::memcpy(expected, storage, sizeof(storage));
                auto *tut = reinterpret_cast<TutWin *>(storage);
                // TWO rectangles, and the second is INVERTED. The centring
                // here is the same code as the rect helpers above, so it has
                // the same blind spot: with only a positive width the
                // sign-carry term is unobserved and its mutants survive. That
                // was learned one batch ago and re-learned here; the lesson
                // is that a clone family needs the clone fixture's CASES, not
                // just its shape.
                const RECT rects[2] = {{10, 20, 30, 40}, {30, 40, 10, 20}};
                RECT rect = rects[empty_slot];
                const char *text = "tutorial";
                g_tut_show = TutShowRecord{};
                if (adapter) {
                    show_ops[index].redirect(tut, nullptr, &rect, text, 5);
                } else {
                    (tut->*show_ops[index].method)(&rect, text, 5);
                }
                expect(g_tut_show.calls == 1);
                expect(g_tut_show.self == tut);
                // Null slot passes a null through; a live one is adjusted by
                // the vbtable displacement rather than passed raw.
                expect(g_tut_show.window ==
                       (empty_slot ? nullptr : primary.data() + 0x40));
                expect(g_tut_show.text == text);
                expect(g_tut_show.x ==
                       20 + 1000 * static_cast<int>(index + 1)
                          + 10 * static_cast<int>(index + 1));
                expect(g_tut_show.y ==
                       30 + 2000 * static_cast<int>(index + 1)
                          + 20 * static_cast<int>(index + 1));
                expect(g_tut_show.sprite == nullptr);
                expect(g_tut_show.flag == 5);
                expect(g_tut_show.a7 == -1 && g_tut_show.a8 == -1);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
    MapWinTable = saved_table;
    TutWinOriginalShow = saved_show;

    for (size_t index = 0; index < op_count; ++index) {
        *rect_ops[index].window = saved[index];
    }
}

void test_win_client_to_screen() {
    // Each node contributes client_rect_.left/top (0x14C/0x150) plus
    // outer_rect_.left/top (0x13C/0x140); bit 5 of the dword at 0x98 continues
    // the walk to the parent at 0xC4, and bit 15 then backs the parent's outer
    // origin out again. Nodes carry distinct values so a dropped or misrouted
    // term changes the total.
    struct WinNode {
        alignas(Win) uint8_t storage[sizeof(Win) + 32];
        uint8_t expected[sizeof(Win) + 32];

        Win *object() { return reinterpret_cast<Win *>(storage + 16); }
    };
    struct ChainCase {
        int depth;
        uint32_t flags[3];
        int start_x;
        int start_y;
        int expected_x;
        int expected_y;
    };
    // Per-node geometry: client (100,200)*(n+1), outer (10,20)*(n+1).
    const ChainCase cases[] = {
        // No walk: bit 5 clear stops at this node regardless of a parent.
        {1, {0x00000000U, 0, 0}, 0, 0, 110, 220},
        {2, {0x00000000U, 0x00000020U, 0}, 0, 0, 110, 220},
        // The start point is accumulated, not replaced.
        {1, {0x00000000U, 0, 0}, 7, -9, 117, 211},
        // Bit 5 with a null parent still stops.
        {1, {0x00000020U, 0, 0}, 0, 0, 110, 220},
        // Two-node walk: 110+220 = 330, 220+440 = 660.
        {2, {0x00000020U, 0x00000000U, 0}, 0, 0, 330, 660},
        // Bit 15 without bit 5 is inert - no walk, so nothing to back out.
        {1, {0x00008000U, 0, 0}, 0, 0, 110, 220},
        // Bit 15 with the walk subtracts the parent's outer origin (20,40).
        {2, {0x00008020U, 0x00000000U, 0}, 0, 0, 310, 620},
        // Three-node walk: 110+220+330 = 660, 220+440+660 = 1320.
        {3, {0x00000020U, 0x00000020U, 0x00000000U}, 0, 0, 660, 1320},
        // Innermost node also backs out its own parent's outer origin (30,60).
        {3, {0x00000020U, 0x00008020U, 0x00000000U}, 0, 0, 630, 1260},
        // Both levels back out: -(20,40) at the root, -(30,60) one level in.
        {3, {0x00008020U, 0x00008020U, 0x00000000U}, 0, 0, 610, 1220},
        // Unrelated flag bits must not gate either behaviour.
        {2, {0xFFFF7FDFU, 0U, 0}, 0, 0, 110, 220},
        {2, {0xFFFFFFFFU, 0U, 0}, 0, 0, 310, 620},
    };
    for (const ChainCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            WinNode nodes[3];
            for (int index = 0; index < test.depth; ++index) {
                seed_storage(nodes[index].storage, nodes[index].expected,
                             sizeof(nodes[index].storage));
                write_at(nodes[index].storage, 16 + 0x98, test.flags[index]);
                const int scale = index + 1;
                write_at(nodes[index].storage, 16 + 0x14C, 100 * scale);
                write_at(nodes[index].storage, 16 + 0x150, 200 * scale);
                write_at(nodes[index].storage, 16 + 0x13C, 10 * scale);
                write_at(nodes[index].storage, 16 + 0x140, 20 * scale);
                Win *parent = (index + 1 < test.depth)
                    ? nodes[index + 1].object() : nullptr;
                write_at(nodes[index].storage, 16 + 0xC4, parent);
                std::memcpy(nodes[index].expected, nodes[index].storage,
                            sizeof(nodes[index].storage));
            }
            int x = test.start_x;
            int y = test.start_y;
            if (use_adapter) {
                win_client_to_screen_redirect(nodes[0].object(), nullptr, &x, &y);
            } else {
                nodes[0].object()->client_to_screen(&x, &y);
            }
            expect(x == test.expected_x);
            expect(y == test.expected_y);
            // The translation is a pure query: no node may be modified.
            for (int index = 0; index < test.depth; ++index) {
                expect_storage_bytes(nodes[index].storage, nodes[index].expected,
                                     sizeof(nodes[index].storage));
            }
        }
    }
}

void test_sprite_construct() {
    static_assert(sizeof(Sprite) == 0x2C, "Sprite tests require the legacy layout");
    int *const saved_total = SpriteMemoryUsed;
    const int32_t starting_totals[] = {
        0, 1, -1, 0x7FFFFFFF, static_cast<int32_t>(0x80000000U),
    };
    for (int32_t starting_total : starting_totals) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            // Every field the constructor writes, in legacy layout order.
            write_at(expected, 16 + 0x00, 0U);
            write_at(expected, 16 + 0x04, 0U);
            expected[16 + 0x08] = 9;
            write_at(expected, 16 + 0x0C, 0U);
            write_at(expected, 16 + 0x10, 0U);
            write_at(expected, 16 + 0x14, 0U);
            write_at(expected, 16 + 0x18, 0U);
            write_at(expected, 16 + 0x1C, 0U);
            write_at(expected, 16 + 0x20, 0U);
            write_at(expected, 16 + 0x24, 0U);
            write_at(expected, 16 + 0x28, 0U);

            int32_t total = starting_total;
            SpriteMemoryUsed = &total;
            auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
            if (use_adapter) {
                expect(sprite_construct_redirect(sprite, nullptr) == sprite);
            } else {
                new (sprite) Sprite();
            }
            // Offsets 0x09-0x0B stay untouched: only a byte is written at 0x08.
            expect_storage_bytes(storage, expected, sizeof(storage));
            // The accounting total advances by exactly one object, wrapping.
            const int32_t advanced = static_cast<int32_t>(
                static_cast<uint32_t>(starting_total) + 0x2CU);
            expect(total == advanced);
        }
    }
    SpriteMemoryUsed = saved_total;
}

GraphicWinStubRecord graphic_win_stub_record = {};

bool button_lifecycle_capture = false;

uint8_t *button_lifecycle_base = nullptr;

int button_lifecycle_close_calls = 0;

int button_lifecycle_close_time_counts[4] = {};

uint32_t button_lifecycle_close_primary_vtables[4] = {};

uint32_t button_lifecycle_close_buffer_vtables[4] = {};

uint32_t button_lifecycle_close_a14[4] = {};

uint32_t button_lifecycle_close_a18[4] = {};

uint32_t button_lifecycle_close_ab8[4] = {};

uint32_t button_lifecycle_close_abc[4] = {};

int button_lifecycle_destructor_time_count = -1;

uint32_t button_lifecycle_read(size_t offset) {
    uint32_t value;
    std::memcpy(&value, button_lifecycle_base + offset, sizeof(value));
    return value;
}

void reset_button_lifecycle_capture(uint8_t *base) {
    button_lifecycle_capture = true;
    button_lifecycle_base = base;
    button_lifecycle_close_calls = 0;
    std::memset(button_lifecycle_close_time_counts, 0,
                sizeof(button_lifecycle_close_time_counts));
    std::memset(button_lifecycle_close_primary_vtables, 0,
                sizeof(button_lifecycle_close_primary_vtables));
    std::memset(button_lifecycle_close_buffer_vtables, 0,
                sizeof(button_lifecycle_close_buffer_vtables));
    std::memset(button_lifecycle_close_a14, 0,
                sizeof(button_lifecycle_close_a14));
    std::memset(button_lifecycle_close_a18, 0,
                sizeof(button_lifecycle_close_a18));
    std::memset(button_lifecycle_close_ab8, 0,
                sizeof(button_lifecycle_close_ab8));
    std::memset(button_lifecycle_close_abc, 0,
                sizeof(button_lifecycle_close_abc));
    button_lifecycle_destructor_time_count = -1;
    time_close_calls = 0;
    std::memset(time_close_targets, 0, sizeof(time_close_targets));
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall graphic_win_stub_buffer_destructor(void *target) {
    if (button_lifecycle_capture) {
        button_lifecycle_destructor_time_count = time_close_calls;
    }
    graphic_win_stub_record.buffer_calls++;
    graphic_win_stub_record.buffer_target = target;
    graphic_win_stub_record.sequence =
        (graphic_win_stub_record.sequence << 4) | 2;
}

void __thiscall graphic_win_stub_win_destructor(void *target) {
    graphic_win_stub_record.win_calls++;
    graphic_win_stub_record.win_target = target;
    graphic_win_stub_record.sequence =
        (graphic_win_stub_record.sequence << 4) | 1;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void test_graphic_win_destructor() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "GraphicWin tests require the legacy layout");
    // The destructor stores both vtables, clears the trailing field, then
    // delegates to the Buffer and Win subobject destructors. The delegation
    // targets are original dependencies, so the source test substitutes
    // recording stubs and asserts the observable stores and call order.
    for (int null_self = 0; null_self < 2; ++null_self) {
        alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        auto *object = reinterpret_cast<GraphicWin *>(storage + 16);

        graphic_win_destructor_probe_reset();
        graphic_win_stub_record = GraphicWinStubRecord{};
        func_subobject_destructor *const saved_buffer = BufferSubobjectDestructor;
        func_subobject_destructor *const saved_win = WinOriginalDestructor;
        BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
        WinOriginalDestructor = graphic_win_stub_win_destructor;
        GraphicWin *const target = null_self ? nullptr : object;
        expect(graphic_win_destructor_redirect(target, nullptr) == target);
        BufferSubobjectDestructor = saved_buffer;
        WinOriginalDestructor = saved_win;

        if (null_self) {
            // Nothing may be written when the instance pointer is null.
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(graphic_win_destructor_probe_buffer_calls() == 0);
            expect(graphic_win_destructor_probe_win_calls() == 0);
            expect(graphic_win_stub_record.buffer_calls == 0);
            expect(graphic_win_stub_record.win_calls == 0);
            continue;
        }
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);
        write_at(expected, 16 + 0xA10, 0U);
        expect_storage_bytes(storage, expected, sizeof(storage));
        // Buffer subobject first, then the Win base, matching legacy order.
        expect(graphic_win_destructor_probe_buffer_calls() == 1);
        expect(graphic_win_destructor_probe_win_calls() == 1);
        expect(graphic_win_destructor_probe_buffer_target()
               == reinterpret_cast<void *>(storage + 16 + 0x444));
        expect(graphic_win_destructor_probe_win_target()
               == reinterpret_cast<void *>(storage + 16));
        expect(graphic_win_destructor_probe_order() == 0x21);
        // The stubs prove the delegation calls were made, not merely
        // recorded as intended by the inline probe.
        expect(graphic_win_stub_record.buffer_calls == 1);
        expect(graphic_win_stub_record.win_calls == 1);
        expect(graphic_win_stub_record.buffer_target
               == reinterpret_cast<void *>(storage + 16 + 0x444));
        expect(graphic_win_stub_record.win_target
               == reinterpret_cast<void *>(storage + 16));
        expect(graphic_win_stub_record.sequence == 0x21);
    }
}

struct GraphicWinCloseStubRecord {
    int win_calls;
    void *win_target;
    int buffer_calls;
    void *buffer_target;
    int release_calls;
    void *release_target;
    uint32_t release_flags;
    uint32_t sequence;
    uint32_t dependency_seen_a10[2];
    uint32_t dependency_seen_134[2];
    uint32_t dependency_seen_9cc[2];
    uint32_t release_seen_a10;
    uint32_t release_seen_134;
    uint32_t release_seen_9cc;
    uint32_t release_seen_a08;
    uint32_t release_seen_a0c;
    bool replace_a08;
    void *replacement_a08;
};

GraphicWinCloseStubRecord graphic_win_close_stub_record = {};

uint32_t graphic_win_field(void *self, size_t offset) {
    uint32_t value;
    std::memcpy(&value, static_cast<uint8_t *>(self) + offset, sizeof(value));
    return value;
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall graphic_win_stub_win_close(void *target) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    if (button_lifecycle_capture) {
        const int call = button_lifecycle_close_calls++;
        if (call < static_cast<int>(
                ARRAYSIZE(button_lifecycle_close_time_counts))) {
            button_lifecycle_close_time_counts[call] = time_close_calls;
            button_lifecycle_close_primary_vtables[call] =
                button_lifecycle_read(0x000);
            button_lifecycle_close_buffer_vtables[call] =
                button_lifecycle_read(0x444);
            button_lifecycle_close_a14[call] =
                button_lifecycle_read(0xA14);
            button_lifecycle_close_a18[call] =
                button_lifecycle_read(0xA18);
            button_lifecycle_close_ab8[call] =
                button_lifecycle_read(0xAB8);
            button_lifecycle_close_abc[call] =
                button_lifecycle_read(0xABC);
        }
    }
    record.win_calls++;
    record.win_target = target;
    record.sequence = (record.sequence << 4) | 1U;
    record.dependency_seen_a10[0] = graphic_win_field(target, 0xA10);
    record.dependency_seen_134[0] = graphic_win_field(target, 0x134);
    record.dependency_seen_9cc[0] = graphic_win_field(target, 0x9CC);
}

void __thiscall graphic_win_stub_buffer_close(void *target) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    auto *const self = static_cast<uint8_t *>(target) - 0x444;
    record.buffer_calls++;
    record.buffer_target = target;
    record.sequence = (record.sequence << 4) | 2U;
    record.dependency_seen_a10[1] = graphic_win_field(self, 0xA10);
    record.dependency_seen_134[1] = graphic_win_field(self, 0x134);
    record.dependency_seen_9cc[1] = graphic_win_field(self, 0x9CC);
    if (record.replace_a08) {
        write_at(self, 0xA08, record.replacement_a08);
    }
}

uint32_t __thiscall graphic_win_stub_release(void *target, uint32_t flags) {
    GraphicWinCloseStubRecord &record = graphic_win_close_stub_record;
    record.release_calls++;
    record.release_target = target;
    record.release_flags = flags;
    record.sequence = (record.sequence << 4) | 3U;
    record.release_seen_a10 = graphic_win_field(record.win_target, 0xA10);
    record.release_seen_134 = graphic_win_field(record.win_target, 0x134);
    record.release_seen_9cc = graphic_win_field(record.win_target, 0x9CC);
    record.release_seen_a08 = graphic_win_field(record.win_target, 0xA08);
    record.release_seen_a0c = graphic_win_field(record.win_target, 0xA0C);
    return 0x7B3D19E5U;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

namespace {

struct GwFillCall { void *self; int color; int calls; };

GwFillCall g_gw_fill;

struct GwMapCall { void *self; int a, b, c, d; void *table; int calls; };

GwMapCall g_gw_map;

struct GwCopyCall { void *self; void *buffer; int x, y, e, f, w, h; int calls; };

GwCopyCall g_gw_copy;

struct GwOverlayCall { void *self; RECT *rect; int calls; };

GwOverlayCall g_gw_overlay;

int g_paint_calls;

int g_hook_calls;

// Nibble-packed call order, so the sequence hook -> paint -> overlay is
// asserted rather than just the three counts.
uint32_t g_gw_order;

struct GwInvalidateCall { HWND window; RECT rect; BOOL erase; int calls; };

GwInvalidateCall g_gw_invalidate;

// The paint hook sets a bit on 0x1A0 so the test can prove the clear re-reads
// the field instead of writing back the value latched before the hook ran.
void *g_hook_object;

// When set, the paint hook calls redraw again on the same window. The latch
// at bit 0 of 0x1A0 is what makes that second entry a no-op; without it the
// recursion would paint twice.
GraphicWin *g_hook_reenters;

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

int __thiscall observe_fill_color(void *self, int color) {
    g_gw_fill = {self, color, g_gw_fill.calls + 1};
    return 0;
}

int __thiscall poison_fill_color(void *, int) { expect(false); return 0; }

int __thiscall observe_map_colors(void *self, int a, int b, int c, int d,
                                  void *table) {
    g_gw_map = {self, a, b, c, d, table, g_gw_map.calls + 1};
    return 0;
}

int __thiscall poison_map_colors(void *, int, int, int, int, void *) {
    expect(false);
    return 0;
}

int __thiscall observe_gw_copy_full(void *self, Buffer *buffer, int x, int y,
                                 int e, int f, int w, int h) {
    g_gw_copy = {self, buffer, x, y, e, f, w, h, g_gw_copy.calls + 1};
    return 0;
}

int __thiscall poison_gw_copy_full(void *, Buffer *, int, int, int, int, int,
                                int) {
    expect(false);
    return 0;
}

void __thiscall observe_overlay(void *self, RECT *rect) {
    g_gw_overlay = {self, rect, g_gw_overlay.calls + 1};
    g_gw_order = (g_gw_order << 4) | 3;
}

void __thiscall observe_paint(void *) {
    ++g_paint_calls;
    g_gw_order = (g_gw_order << 4) | 2;
}

void *__thiscall parent_says_transparent(void *self) { return self; }

void *__thiscall parent_says_opaque(void *) { return nullptr; }

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void __cdecl observe_paint_hook() {
    ++g_hook_calls;
    g_gw_order = (g_gw_order << 4) | 1;
    if (g_hook_reenters != nullptr) {
        GraphicWin *const again = g_hook_reenters;
        g_hook_reenters = nullptr;   // one level is enough to prove it
        again->redraw();
    }
    if (g_hook_object != nullptr) {
        uint32_t state;
        std::memcpy(&state,
                    static_cast<uint8_t *>(g_hook_object) + 0x1A0,
                    sizeof(state));
        state |= 0x40U;
        std::memcpy(static_cast<uint8_t *>(g_hook_object) + 0x1A0, &state,
                    sizeof(state));
    }
}

BOOL __stdcall observe_invalidate(HWND window, const RECT *rect, BOOL erase) {
    g_gw_invalidate.window = window;
    g_gw_invalidate.rect = *rect;
    g_gw_invalidate.erase = erase;
    ++g_gw_invalidate.calls;
    return TRUE;
}

void reset_graphic_win_paint_probes() {
    g_gw_fill = GwFillCall{};
    g_gw_map = GwMapCall{};
    g_gw_copy = GwCopyCall{};
    g_gw_overlay = GwOverlayCall{};
    g_paint_calls = 0;
    g_hook_calls = 0;
    g_gw_order = 0;
    g_hook_object = nullptr;
    g_hook_reenters = nullptr;
    g_gw_invalidate = GwInvalidateCall{};
}

}  // namespace

// GraphicWin::fill(int). Three conditions gate the transparent copy path and
// all three must hold: flag bit 19, a non-null parent, and a nonzero answer
// from the parent's vtable slot 0xF4.
void test_graphic_win_fill_color() {
    auto *const saved_fill = BufferOriginalFillColor;
    auto *const saved_map = BufferOriginalMapColors;
    auto *const saved_copy = BufferCopyFull;
    // The seam is repointed at a local slot, never written through: its
    // default target 0x009B3390 is an address in the original image and is
    // not mapped in the standalone leaf process.
    void **const saved_table = GraphicWinColorMapTable;
    void *table_slot = nullptr;
    GraphicWinColorMapTable = &table_slot;

    alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
    alignas(GraphicWin) uint8_t parent_storage[sizeof(GraphicWin) + 32];
    uint8_t expected[sizeof(storage)];
    auto *const object = storage + 16;
    auto *const window = reinterpret_cast<GraphicWin *>(object);
    auto *const parent = parent_storage + 16;

    uintptr_t parent_vtable[64];
    struct Gate { const char *label; uint32_t flags; bool has_parent;
                  bool answers; bool copies; };
    const Gate gates[] = {
        {"opaque",          0x00000000U, true,  true,  false},
        {"bit19 no parent", 0x00080000U, false, true,  false},
        {"bit19 says no",   0x00080000U, true,  false, false},
        {"bit19 says yes",  0x00080000U, true,  true,  true},
        {"other bits only", 0xFFF7FFFFU, true,  true,  false},
        {"all bits",        0xFFFFFFFFU, true,  true,  true},
    };
    for (const Gate &gate : gates) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            std::memset(parent_storage, 0, sizeof(parent_storage));
            for (size_t slot = 0; slot < 64; ++slot) {
                parent_vtable[slot] = 0;
            }
            parent_vtable[0xF4 / 4] = reinterpret_cast<uintptr_t>(
                gate.answers ? &parent_says_transparent : &parent_says_opaque);
            uintptr_t *const table_pointer = parent_vtable;
            std::memcpy(parent, &table_pointer, sizeof(table_pointer));
            write_at(storage, 16 + 0x98, gate.flags);
            uint8_t *const parent_value = gate.has_parent ? parent : nullptr;
            std::memcpy(object + 0xC4, &parent_value, sizeof(parent_value));
            write_at(storage, 16 + 0x14C, 100);
            write_at(storage, 16 + 0x150, 200);
            write_at(storage, 16 + 0x13C, 7);
            write_at(storage, 16 + 0x140, 9);
            write_at(storage, 16 + 0x4C4, 40);
            write_at(storage, 16 + 0x4C8, 25);
            std::memcpy(expected, storage, sizeof(storage));
            reset_graphic_win_paint_probes();
            table_slot = nullptr;
            BufferOriginalFillColor =
                gate.copies ? &poison_fill_color : &observe_fill_color;
            BufferCopyFull = gate.copies ? &observe_gw_copy_full
                                         : &poison_gw_copy_full;
            BufferOriginalMapColors = &poison_map_colors;

            if (adapter) {
                graphic_win_fill_color_redirect(window, nullptr, 0x5A);
            } else {
                window->fill(0x5A);
            }

            if (gate.copies) {
                expect(g_gw_copy.calls == 1);
                expect(g_gw_copy.self == parent + 0x444);
                expect(g_gw_copy.buffer ==
                       reinterpret_cast<Buffer *>(object + 0x444));
                expect(g_gw_copy.x == 107);          // 100 + 7
                expect(g_gw_copy.y == 209);          // 200 + 9
                expect(g_gw_copy.e == 0 && g_gw_copy.f == 0);
                expect(g_gw_copy.w == 40);
                expect(g_gw_copy.h == -25);          // negated height
                expect(g_gw_fill.calls == 0);
            } else {
                expect(g_gw_fill.calls == 1);
                expect(g_gw_fill.self == object + 0x444);
                expect(g_gw_fill.color == 0x5A);
                expect(g_gw_copy.calls == 0);
            }
            expect(g_gw_map.calls == 0);             // null table remaps nothing
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // With a table present the copy path remaps, and the bounds are
    // INCLUSIVE: width-1 and -1-height, re-read after the blit.
    int table_object = 0;
    seed_storage(storage, expected, sizeof(storage));
    std::memset(parent_storage, 0, sizeof(parent_storage));
    parent_vtable[0xF4 / 4] =
        reinterpret_cast<uintptr_t>(&parent_says_transparent);
    uintptr_t *const table_pointer = parent_vtable;
    std::memcpy(parent, &table_pointer, sizeof(table_pointer));
    write_at(storage, 16 + 0x98, 0x00080000U);
    uint8_t *const parent_value = parent;
    std::memcpy(object + 0xC4, &parent_value, sizeof(parent_value));
    write_at(storage, 16 + 0x14C, 0);
    write_at(storage, 16 + 0x150, 0);
    write_at(storage, 16 + 0x13C, 0);
    write_at(storage, 16 + 0x140, 0);
    write_at(storage, 16 + 0x4C4, 40);
    write_at(storage, 16 + 0x4C8, 25);
    std::memcpy(expected, storage, sizeof(storage));
    reset_graphic_win_paint_probes();
    table_slot = &table_object;
    BufferOriginalFillColor = &poison_fill_color;
    BufferCopyFull = &observe_gw_copy_full;
    BufferOriginalMapColors = &observe_map_colors;
    window->fill(3);
    expect(g_gw_copy.calls == 1);
    expect(g_gw_map.calls == 1);
    expect(g_gw_map.self == object + 0x444);
    expect(g_gw_map.a == 0 && g_gw_map.b == 0);
    expect(g_gw_map.c == 39);        // width - 1
    expect(g_gw_map.d == -26);       // -1 - height
    expect(g_gw_map.table == &table_object);
    expect_storage_bytes(storage, expected, sizeof(storage));

    GraphicWinColorMapTable = saved_table;
    BufferCopyFull = saved_copy;
    BufferOriginalMapColors = saved_map;
    BufferOriginalFillColor = saved_fill;
}

// GraphicWin::redraw. The re-entrancy latch, and the fact that the clear
// re-reads 0x1A0 so bits the paint hook set survive it.
void test_graphic_win_redraw() {
    auto *const saved_overlay = GraphicWinOverlayNonclient;
    auto *const saved_invalidate = GraphicWinInvalidateRect;
    HWND *const saved_handle_slot = WinHdcWindow;
    Win **const saved_current = ScrollCurrentWin;

    alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
    uint8_t expected[sizeof(storage)];
    auto *const object = storage + 16;
    auto *const window = reinterpret_cast<GraphicWin *>(object);
    uintptr_t vtable[64];
    HWND handle = reinterpret_cast<HWND>(0x1234);
    Win *current = nullptr;
    WinHdcWindow = &handle;
    ScrollCurrentWin = &current;

    auto arrange = [&](uint32_t state, uint32_t visible_flags,
                       bool with_hook) {
        seed_storage(storage, expected, sizeof(storage));
        for (size_t slot = 0; slot < 64; ++slot) {
            vtable[slot] = 0;
        }
        vtable[0x30 / 4] = reinterpret_cast<uintptr_t>(&observe_paint);
        uintptr_t *const table_pointer = vtable;
        std::memcpy(object, &table_pointer, sizeof(table_pointer));
        write_at(storage, 16 + 0x1A0, state);
        write_at(storage, 16 + 0x9C, visible_flags);
        write_at(storage, 16 + 0xC4, static_cast<uint32_t>(0));
        func_graphic_win_paint_hook *const hook =
            with_hook ? &observe_paint_hook : nullptr;
        std::memcpy(object + 0xA10, &hook, sizeof(hook));
        RECT area = {10, 20, 110, 61};
        std::memcpy(object + 0x474, &area, sizeof(area));
        // Win::client_to_screen ADDS client_rect_+outer_rect_ (0x14C/0x13C
        // and 0x150/0x140, read straight off 0x005ED249) to what it is
        // handed, so non-zero values here make both the translation and the
        // zero-initialisers observable in the rectangle asserted below. A
        // null parent at 0xC4 stops it after the first step.
        write_at(storage, 16 + 0x14C, 5);
        write_at(storage, 16 + 0x13C, 3);   // x offset 8
        write_at(storage, 16 + 0x150, 2);
        write_at(storage, 16 + 0x140, 4);   // y offset 6
        std::memcpy(expected, storage, sizeof(storage));
        reset_graphic_win_paint_probes();
        GraphicWinOverlayNonclient = &observe_overlay;
        GraphicWinInvalidateRect = &observe_invalidate;
    };

    // Already inside a redraw: nothing runs, nothing is published.
    arrange(1, 1, true);
    current = nullptr;
    window->redraw();
    expect(g_paint_calls == 0 && g_hook_calls == 0);
    expect(g_gw_overlay.calls == 0 && g_gw_invalidate.calls == 0);
    expect(current == nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // No window handle at all: the guard is checked before the latch.
    HWND absent = nullptr;
    WinHdcWindow = &absent;
    arrange(0, 1, true);
    window->redraw();
    expect(g_paint_calls == 0 && g_hook_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    WinHdcWindow = &handle;

    // Visible, with a hook: the hook runs, then the virtual paint, then the
    // nonclient overlay with a null rect, and the latch is cleared.
    arrange(0, 1, true);
    g_hook_object = object;
    write_at(expected, 16 + 0x1A0, 0x40U);   // the hook's bit survives
    window->redraw();
    expect(g_hook_calls == 1);
    expect(g_paint_calls == 1);
    expect(g_gw_overlay.calls == 1);
    // hook, then the virtual paint, then the nonclient overlay.
    expect(g_gw_order == 0x123U);
    expect(g_gw_overlay.self == object);
    expect(g_gw_overlay.rect == nullptr);
    expect(current == reinterpret_cast<Win *>(window));
    expect(g_gw_invalidate.calls == 1);
    expect(g_gw_invalidate.window == handle);
    expect(g_gw_invalidate.erase == FALSE);
    // The rectangle is the client rect at 0x474 translated to screen space.
    expect(g_gw_invalidate.rect.left == 18);      // 10 + 8
    expect(g_gw_invalidate.rect.top == 26);       // 20 + 6
    expect(g_gw_invalidate.rect.right == 118);    // 110 + 8
    expect(g_gw_invalidate.rect.bottom == 67);    // 61 + 6
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Re-entrancy: the paint hook redraws the same window. The latch set on
    // entry is what makes that inner call return immediately - the window is
    // painted once, and the screen area is invalidated once.
    arrange(0, 1, true);
    g_hook_reenters = window;
    window->redraw();
    expect(g_hook_calls == 1);
    expect(g_paint_calls == 1);
    expect(g_gw_overlay.calls == 1);
    expect(g_gw_invalidate.calls == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Invisible: everything up to and including the clear still runs, but no
    // screen area is invalidated.
    arrange(0, 0, false);
    window->redraw();
    expect(g_paint_calls == 1);
    expect(g_gw_overlay.calls == 1);
    expect(g_gw_invalidate.calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // No hook installed: the null is simply skipped.
    arrange(0, 1, false);
    window->redraw();
    expect(g_hook_calls == 0);
    expect(g_paint_calls == 1);
    expect(g_gw_invalidate.calls == 1);
    expect(g_gw_invalidate.rect.left == 18 && g_gw_invalidate.rect.top == 26);
    expect(g_gw_invalidate.rect.right == 118);
    expect(g_gw_invalidate.rect.bottom == 67);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Through the redirect adapter.
    arrange(0, 1, true);
    g_hook_object = object;
    write_at(expected, 16 + 0x1A0, 0x40U);
    graphic_win_redraw_redirect(window, nullptr);
    expect(g_hook_calls == 1 && g_paint_calls == 1);
    expect(g_gw_invalidate.calls == 1);
    expect(g_gw_invalidate.rect.left == 18 && g_gw_invalidate.rect.top == 26);
    expect(g_gw_invalidate.rect.right == 118);
    expect(g_gw_invalidate.rect.bottom == 67);
    expect_storage_bytes(storage, expected, sizeof(storage));

    ScrollCurrentWin = saved_current;
    WinHdcWindow = saved_handle_slot;
    GraphicWinInvalidateRect = saved_invalidate;
    GraphicWinOverlayNonclient = saved_overlay;
}

namespace {

// GraphicWin::init fixture. Every dependency is a recorder: the four original
// seams, the process allocator, and the two closes init reaches through
// GraphicWin::close.
struct InitRecord {
    int close_win_calls;
    int close_buffer_calls;
    int alloc_calls;
    unsigned int alloc_size;
    int base_calls;
    void *base_self;
    int base_args[9];
    int min_size_calls;
    uint32_t min_size_seen_448;
    int nonclient_calls;
    void *nonclient_self;
    int nonclient_in_width;
    int nonclient_in_height;
    int surface_calls;
    void *surface_self;
    int surface_width;
    int surface_height;
    int surface_third;
    void *surface_fourth;
    uint32_t sequence;
};

InitRecord g_init_rec;

void init_step(uint32_t step) { g_init_rec.sequence = g_init_rec.sequence * 16 + step; }

int g_init_base_result;

int g_init_surface_result;

void *g_init_alloc_block;

int g_init_nonclient_out_width;

int g_init_nonclient_out_height;

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall init_stub_win_close(void *) {
    ++g_init_rec.close_win_calls;
    init_step(1);
}

void __thiscall init_stub_buffer_close(void *) {
    ++g_init_rec.close_buffer_calls;
    init_step(2);
}

void *__cdecl init_stub_alloc(unsigned int size) {
    ++g_init_rec.alloc_calls;
    g_init_rec.alloc_size = size;
    init_step(3);
    return g_init_alloc_block;
}

int __thiscall init_stub_base(void *self, int a1, int a2, int a3, int a4,
                              LPSTR a5, int a6, Win *a7, Menu *a8,
                              BorderSizing *a9) {
    ++g_init_rec.base_calls;
    g_init_rec.base_self = self;
    const int seen[9] = {a1, a2, a3, a4, static_cast<int>(reinterpret_cast<intptr_t>(a5)),
                         a6, static_cast<int>(reinterpret_cast<intptr_t>(a7)),
                         static_cast<int>(reinterpret_cast<intptr_t>(a8)),
                         static_cast<int>(reinterpret_cast<intptr_t>(a9))};
    for (int index = 0; index < 9; ++index) { g_init_rec.base_args[index] = seen[index]; }
    init_step(4);
    return g_init_base_result;
}

void __thiscall init_stub_min_size(void *self) {
    ++g_init_rec.min_size_calls;
    // Reads the back pointer, so a body that wrote 0x448 afterwards would
    // show up here as zero rather than as the window.
    g_init_rec.min_size_seen_448 =
        graphic_win_field(self, 0x448);
    init_step(5);
}

void __thiscall init_stub_nonclient(void *self, int *width, int *height) {
    ++g_init_rec.nonclient_calls;
    g_init_rec.nonclient_self = self;
    g_init_rec.nonclient_in_width = *width;
    g_init_rec.nonclient_in_height = *height;
    *width = g_init_nonclient_out_width;
    *height = g_init_nonclient_out_height;
    init_step(6);
}

int __thiscall init_stub_surface(void *self, int width, int height, int third,
                                 void *fourth) {
    ++g_init_rec.surface_calls;
    g_init_rec.surface_self = self;
    g_init_rec.surface_width = width;
    g_init_rec.surface_height = height;
    g_init_rec.surface_third = third;
    g_init_rec.surface_fourth = fourth;
    init_step(7);
    return g_init_surface_result;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// The eleven table slots, each a distinct sentinel so the slot-to-field map is
// pinned exactly rather than up to a permutation.
const uint32_t g_init_default_slots[11] = {
    0xD0000001U, 0xD0000002U, 0xD0000003U, 0xD0000004U, 0xD0000005U,
    0xD0000006U, 0xD0000007U, 0xD0000008U, 0xD0000009U, 0xD000000AU,
    0xD000000BU,
};

// Read straight off the stores at 0x005D4F1E..0x005D4F98: field offset, then
// the table slot that lands in it. Slots 4..7 and 8..9 are crossed.
const struct { size_t offset; int slot; } g_init_default_map[11] = {
    {0x9CC, 0}, {0x9D0, 1}, {0x9D4, 2}, {0x9D8, 3},
    {0x9DC, 6}, {0x9E0, 7}, {0x9E4, 5}, {0x9E8, 4},
    {0x9EC, 9}, {0x9F0, 8}, {0x9F4, 10},
};

}  // namespace

void test_graphic_win_init() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "GraphicWin init tests require the legacy layout");

    func_subobject_close *const saved_win_close = WinOriginalClose;
    func_subobject_close *const saved_buffer_close = BufferSubobjectClose;
    uint32_t *const saved_a0c_default = GraphicWinFieldA0CDefault;
    func_win_init *const saved_base = WinOriginalInit;
    func_graphic_win_compute_min_size *const saved_min =
        GraphicWinOriginalComputeMinSize;
    func_win_nonclient_to_client *const saved_nonclient =
        WinOriginalNonclientToClient;
    func_buffer_init *const saved_surface = BufferOriginalInit;
    uint32_t *const saved_defaults = GraphicWinInitDefaults;
    func_operator_new *const saved_alloc = WaveOperatorNew;
    int *const saved_thickness = ScrollDefaultThickness;
    Palette **const saved_palette = WinActivePalette;

    WinOriginalClose = &init_stub_win_close;
    BufferSubobjectClose = &init_stub_buffer_close;
    WinOriginalInit = &init_stub_base;
    GraphicWinOriginalComputeMinSize = &init_stub_min_size;
    WinOriginalNonclientToClient = &init_stub_nonclient;
    BufferOriginalInit = &init_stub_surface;
    WaveOperatorNew = &init_stub_alloc;
    uint32_t a0c_default = 0x11223344U;
    GraphicWinFieldA0CDefault = &a0c_default;
    uint32_t defaults[11];
    for (int slot = 0; slot < 11; ++slot) { defaults[slot] = g_init_default_slots[slot]; }
    GraphicWinInitDefaults = defaults;
    int thickness = 9;
    ScrollDefaultThickness = &thickness;
    // The allocation arm runs the real Buffer::construct, which reads three
    // process globals that still default to addresses in the original image -
    // and the palette one it dereferences. Nothing maps those here, so they
    // are bound before the first call rather than after the first fault. A
    // null palette sends construct down its deterministic ramp arm.
    uint32_t *const saved_reset_520 = BufferResetValue520;
    Font **const saved_font_default = FontDefaultPtr;
    Palette **const saved_buffer_palette = BufferPalette;
    uint32_t reset_520_slot = 0;
    Font *font_slot = nullptr;
    Palette *construct_palette_slot = nullptr;
    BufferResetValue520 = &reset_520_slot;
    FontDefaultPtr = &font_slot;
    BufferPalette = &construct_palette_slot;
    alignas(Palette) uint8_t palette_storage[sizeof(Palette)] = {};
    auto *const palette = reinterpret_cast<Palette *>(palette_storage);
    Palette *palette_slot = palette;
    WinActivePalette = &palette_slot;

    alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
    uint8_t expected[sizeof(storage)];
    auto *const self = reinterpret_cast<GraphicWin *>(storage + 16);
    char title[] = "t";
    auto *const parent = reinterpret_cast<Win *>(0x71000000);
    auto *const menu = reinterpret_cast<Menu *>(0x72000000);
    auto *const border = reinterpret_cast<BorderSizing *>(0x73000000);

    // Everything GraphicWin::close writes, applied to the expected image
    // before a case layers its own effects on top.
    auto apply_close = [&](uint8_t *image) {
        const uint32_t zero = 0;
        write_at(image, 16 + 0xA10, zero);
        write_at(image, 16 + 0x134, zero);
        write_at(image, 16 + 0x138, zero);
        for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
            write_at(image, 16 + offset, zero);
        }
        write_at(image, 16 + 0xA08, static_cast<void *>(nullptr));
        write_at(image, 16 + 0xA0C, a0c_default);
    };
    auto arrange = [&](int flags) {
        seed_storage(storage, expected, sizeof(storage));
        // The seed leaves garbage at 0xA08, and GraphicWin::close dispatches a
        // release through whatever sits there. Every case enters with it empty
        // so close() is the no-op arm its own test already covers in full.
        write_at(storage, 16 + 0xA08, static_cast<void *>(nullptr));
        std::memcpy(expected, storage, sizeof(storage));
        apply_close(expected);
        g_init_rec = InitRecord{};
        g_init_base_result = 0;
        g_init_surface_result = 0;
        g_init_alloc_block = nullptr;
        g_init_nonclient_out_width = 0;
        g_init_nonclient_out_height = 0;
        (void)flags;
    };

    // --- the eleven defaults, and the proof close() runs before them ---
    // close() zeroes 0x9CC..0xA04 outright. If the republish ran first, every
    // one of these fields would read back zero instead of its sentinel, so
    // this single assertion pins both the slot-to-field map and the ordering.
    for (const int style : {0x10000000, 0x20000000, 0x30000000}) {
        arrange(style);
        g_init_base_result = 0x5A5A5A5A;  // stop after Win::init
        for (int index = 0; index < 11; ++index) {
            write_at(expected, 16 + g_init_default_map[index].offset,
                     g_init_default_slots[g_init_default_map[index].slot]);
        }
        expect(self->init(1, 2, 3, 4, title, style, parent, menu, border)
               == 0x5A5A5A5A);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(g_init_rec.sequence == 0x124U);
        expect(g_init_rec.alloc_calls == 0);
        expect(g_init_rec.min_size_calls == 0);
        expect(g_init_rec.surface_calls == 0);
    }

    // Neither style bit: the block is not republished, so close()'s zeroes
    // stand. Same arrangement, opposite outcome.
    arrange(0);
    g_init_base_result = 0x5A5A5A5A;
    expect(self->init(1, 2, 3, 4, title, 0x0F000000, parent, menu, border)
           == 0x5A5A5A5A);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // --- the owned buffer ---
    alignas(Buffer) uint8_t owned_storage[sizeof(Buffer)];
    for (int allocated = 0; allocated < 2; ++allocated) {
        arrange(0x40000000);
        g_init_base_result = 0x5A5A5A5A;
        std::memset(owned_storage, 0xEE, sizeof(owned_storage));
        g_init_alloc_block = allocated ? owned_storage : nullptr;
        write_at(expected, 16 + 0xA08, g_init_alloc_block);
        expect(self->init(1, 2, 3, 4, title, 0x40000000, parent, menu, border)
               == 0x5A5A5A5A);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(g_init_rec.alloc_calls == 1);
        // 0x588 is sizeof(Buffer) and the allocation runs before Win::init.
        expect(g_init_rec.alloc_size == 0x588);
        expect(g_init_rec.sequence == 0x1234U);
        // A block is constructed in place; a null one is not touched at all.
        alignas(Buffer) uint8_t untouched[sizeof(Buffer)];
        std::memset(untouched, 0xEE, sizeof(untouched));
        expect((std::memcmp(owned_storage, untouched, sizeof(untouched)) != 0)
               == (allocated != 0));
    }

    // Without the bit the allocator is never consulted and 0xA08 keeps the
    // null close() left.
    arrange(0);
    g_init_base_result = 0x5A5A5A5A;
    g_init_alloc_block = owned_storage;
    expect(self->init(1, 2, 3, 4, title, 0x20000000, parent, menu, border)
           == 0x5A5A5A5A);
    expect(g_init_rec.alloc_calls == 0);

    // --- nine-argument forwarding, with the allocation arm live ---
    // The original spills the operator-new pointer into its own sixth
    // argument slot at 0x005D4FB3 and survives only because the style was
    // latched into EDI first. Forwarding the spilled slot would show up here
    // as the sixth argument being the Buffer pointer instead of the style.
    arrange(0x40000001);
    g_init_base_result = 0x5A5A5A5A;
    g_init_alloc_block = owned_storage;
    expect(self->init(11, 22, 33, 44, title, 0x40000001, parent, menu, border)
           == 0x5A5A5A5A);
    expect(g_init_rec.base_calls == 1);
    expect(g_init_rec.base_self == storage + 16);
    expect(g_init_rec.base_args[0] == 11);
    expect(g_init_rec.base_args[1] == 22);
    expect(g_init_rec.base_args[2] == 33);
    expect(g_init_rec.base_args[3] == 44);
    expect(g_init_rec.base_args[4] ==
           static_cast<int>(reinterpret_cast<intptr_t>(title)));
    expect(g_init_rec.base_args[5] == 0x40000001);
    expect(g_init_rec.base_args[6] ==
           static_cast<int>(reinterpret_cast<intptr_t>(parent)));
    expect(g_init_rec.base_args[7] ==
           static_cast<int>(reinterpret_cast<intptr_t>(menu)));
    expect(g_init_rec.base_args[8] ==
           static_cast<int>(reinterpret_cast<intptr_t>(border)));

    // --- the back pointer, and that it precedes compute_min_size ---
    arrange(0);
    g_init_surface_result = 0x0BADF00D;
    write_at(expected, 16 + 0x448, static_cast<void *>(self));
    g_init_nonclient_out_width = 55;
    g_init_nonclient_out_height = 66;
    expect(self->init(1, 2, 3, 4, title, 0, parent, menu, border)
           == 0x0BADF00D);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(g_init_rec.min_size_seen_448 ==
           static_cast<uint32_t>(reinterpret_cast<uintptr_t>(self)));
    expect(g_init_rec.sequence == 0x124567U);
    // The non-0x800 arm converts the size in place and the converted values
    // are what reach the surface.
    expect(g_init_rec.nonclient_calls == 1);
    expect(g_init_rec.nonclient_self == storage + 16);
    expect(g_init_rec.nonclient_in_width == 3);
    expect(g_init_rec.nonclient_in_height == 4);
    expect(g_init_rec.surface_calls == 1);
    expect(g_init_rec.surface_self == storage + 16 + 0x444);
    expect(g_init_rec.surface_width == 55);
    expect(g_init_rec.surface_height == 66);
    expect(g_init_rec.surface_third == 0);
    expect(g_init_rec.surface_fourth == nullptr);

    // --- the 0x800 arm: bit 3 of 0x98 is the width, bit 2 is the height ---
    struct ThicknessCase { uint32_t nonclient_flags; int width; int height; };
    const ThicknessCase thickness_cases[] = {
        {0x0, 3, 4}, {0x8, 12, 4}, {0x4, 3, 13}, {0xC, 12, 13},
        // Only the two bits count; the rest of the dword must not leak in.
        {0xFFFFFFF3U, 3, 4},
    };
    for (const ThicknessCase &test : thickness_cases) {
        arrange(0x800);
        g_init_surface_result = 0x0BADF00D;
        write_at(storage, 16 + 0x98, test.nonclient_flags);
        std::memcpy(expected, storage, sizeof(storage));
        apply_close(expected);
        write_at(expected, 16 + 0x448, static_cast<void *>(self));
        expect(self->init(1, 2, 3, 4, title, 0x800, parent, menu, border)
               == 0x0BADF00D);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(g_init_rec.nonclient_calls == 0);
        expect(g_init_rec.surface_width == test.width);
        expect(g_init_rec.surface_height == test.height);
    }

    // --- the success path, and that the palette comes from the seam ---
    // The surface is arranged so sync_to_palette takes its cheap arm: pixel
    // storage present and the generation tag already matching, which leaves
    // only the trailing pair to publish. That pair is the observable - it
    // proves the call ran AND which palette it was handed.
    const uint32_t tag = 0x1234ABCDU;
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        arrange(0);
        LPVOID pixels[1] = {};
        write_at(storage, 16 + 0x444 + 0x54, static_cast<LPVOID *>(pixels));
        write_at(storage, 16 + 0x444 + 0x4A4, tag);
        write_at(palette_storage, 0x400, tag);
        std::memcpy(expected, storage, sizeof(storage));
        apply_close(expected);
        write_at(expected, 16 + 0x448, static_cast<void *>(self));
        write_at(expected, 16 + 0x444 + 0x57C, 1U);
        write_at(expected, 16 + 0x444 + 0x584, palette);
        g_init_nonclient_out_width = 55;
        g_init_nonclient_out_height = 66;
        const int result = use_adapter
            ? graphic_win_init_redirect(self, nullptr, 1, 2, 3, 4, title, 0,
                                        parent, menu, border)
            : self->init(1, 2, 3, 4, title, 0, parent, menu, border);
        expect(result == 0);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(g_init_rec.sequence == 0x124567U);
    }

    WinOriginalClose = saved_win_close;
    BufferSubobjectClose = saved_buffer_close;
    GraphicWinFieldA0CDefault = saved_a0c_default;
    WinOriginalInit = saved_base;
    GraphicWinOriginalComputeMinSize = saved_min;
    WinOriginalNonclientToClient = saved_nonclient;
    BufferOriginalInit = saved_surface;
    GraphicWinInitDefaults = saved_defaults;
    WaveOperatorNew = saved_alloc;
    ScrollDefaultThickness = saved_thickness;
    WinActivePalette = saved_palette;
    BufferResetValue520 = saved_reset_520;
    FontDefaultPtr = saved_font_default;
    BufferPalette = saved_buffer_palette;
}

namespace {

// BaseButton::init's two vtable dispatches. Both are recorded, and the close
// recorder can restage the table so the show's reload is observable.
struct ButtonInitRecord {
    int close_calls;
    void *close_self;
    int show_calls;
    void *show_self;
    int show_arg;
    void **show_vtable_seen;
    uint32_t sequence;
};

ButtonInitRecord g_button_rec;

void **g_button_close_installs;

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

uint32_t __thiscall button_stub_close(void *self) {
    ++g_button_rec.close_calls;
    g_button_rec.close_self = self;
    g_button_rec.sequence = g_button_rec.sequence * 16 + 8;
    // Restage the table if the case asked for it, so a body that reused the
    // pointer it loaded before this call would dispatch the show through the
    // OLD table and be caught.
    if (g_button_close_installs) {
        *reinterpret_cast<void ***>(self) = g_button_close_installs;
    }
    return 0;
}

void __thiscall button_stub_show(void *self, int arg) {
    ++g_button_rec.show_calls;
    g_button_rec.show_self = self;
    g_button_rec.show_arg = arg;
    g_button_rec.show_vtable_seen = *reinterpret_cast<void ***>(self);
    g_button_rec.sequence = g_button_rec.sequence * 16 + 9;
}

void __thiscall button_stub_show_other(void *self, int arg) {
    button_stub_show(self, arg);
    // Distinguishable from the first table's entry by the recorded vtable.
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

}  // namespace

void test_base_button_init() {
    static_assert(sizeof(BaseButton) > 0xA80,
                  "BaseButton init tests reach name_ at 0xA7C");
    constexpr size_t kBuffer = 0x444;
    constexpr size_t kCloseSlot = 0x168 / sizeof(void *);
    constexpr size_t kShowSlot = 0x04 / sizeof(void *);

    // GraphicWin::init runs for real here - it is source-owned - so every one
    // of ITS seams has to be bound too. This is the same set
    // test_graphic_win_init binds, and it reuses those recorders directly.
    func_subobject_close *const saved_win_close = WinOriginalClose;
    func_subobject_close *const saved_buffer_close = BufferSubobjectClose;
    uint32_t *const saved_a0c_default = GraphicWinFieldA0CDefault;
    func_win_init *const saved_base = WinOriginalInit;
    func_graphic_win_compute_min_size *const saved_min =
        GraphicWinOriginalComputeMinSize;
    func_win_nonclient_to_client *const saved_nonclient =
        WinOriginalNonclientToClient;
    func_buffer_init *const saved_surface = BufferOriginalInit;
    uint32_t *const saved_defaults = GraphicWinInitDefaults;
    func_operator_new *const saved_alloc = WaveOperatorNew;
    int *const saved_thickness = ScrollDefaultThickness;
    Palette **const saved_win_palette = WinActivePalette;
    uint32_t *const saved_reset_520 = BufferResetValue520;
    Font **const saved_font_default = FontDefaultPtr;
    Palette **const saved_buffer_palette = BufferPalette;
    uint32_t *const saved_button_colors = BaseButtonDefaultTextColors;
    Font **const saved_button_fonts = BaseButtonDefaultFonts;

    WinOriginalClose = &init_stub_win_close;
    BufferSubobjectClose = &init_stub_buffer_close;
    WinOriginalInit = &init_stub_base;
    GraphicWinOriginalComputeMinSize = &init_stub_min_size;
    WinOriginalNonclientToClient = &init_stub_nonclient;
    BufferOriginalInit = &init_stub_surface;
    WaveOperatorNew = &init_stub_alloc;
    uint32_t a0c_default = 0x11223344U;
    GraphicWinFieldA0CDefault = &a0c_default;
    uint32_t win_defaults[11] = {};
    GraphicWinInitDefaults = win_defaults;
    int thickness = 0;
    ScrollDefaultThickness = &thickness;
    uint32_t reset_520_slot = 0;
    Font *font_default_slot = nullptr;
    Palette *construct_palette_slot = nullptr;
    BufferResetValue520 = &reset_520_slot;
    FontDefaultPtr = &font_default_slot;
    BufferPalette = &construct_palette_slot;
    alignas(Palette) uint8_t palette_storage[sizeof(Palette)] = {};
    Palette *win_palette_slot = reinterpret_cast<Palette *>(palette_storage);
    WinActivePalette = &win_palette_slot;

    // Twelve distinct sentinels: slot s tier t at index s * 3 + t. Anything
    // less than twelve distinct values would let an index permutation pass.
    uint32_t colors[12];
    for (int index = 0; index < 12; ++index) {
        colors[index] = 0xC0000000U + static_cast<uint32_t>(index);
    }
    BaseButtonDefaultTextColors = colors;
    alignas(Font) uint8_t unready_font[sizeof(Font)] = {};
    Font *fonts[3] = {reinterpret_cast<Font *>(unready_font),
                      reinterpret_cast<Font *>(0x64000000),
                      reinterpret_cast<Font *>(0x65000000)};
    BaseButtonDefaultFonts = fonts;

    std::vector<uint8_t> storage(sizeof(BaseButton) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *const button = reinterpret_cast<BaseButton *>(storage.data() + 16);
    auto *const parent = reinterpret_cast<Win *>(0x71000000);

    void *vtable[0x160] = {};
    void *restaged[0x160] = {};
    vtable[kCloseSlot] = reinterpret_cast<void *>(&button_stub_close);
    vtable[kShowSlot] = reinterpret_cast<void *>(&button_stub_show);
    restaged[kCloseSlot] = reinterpret_cast<void *>(&button_stub_close);
    restaged[kShowSlot] = reinterpret_cast<void *>(&button_stub_show_other);

    auto arrange = [&]() {
        seed_storage(storage.data(), expected.data(), storage.size());
        write_at(storage.data(), 16 + 0xA08, static_cast<void *>(nullptr));
        write_at(storage.data(), 16, static_cast<void *>(vtable));
        // GraphicWin::init ends in a real Buffer::sync_to_palette, which on a
        // seeded buffer would take its republish arm and dispatch GetDC
        // through a garbage surface vtable. Entering with no pixel storage
        // stops it at its own first guard, writing nothing - what
        // sync_to_palette does is test_buffer_sync_to_palette's business, and
        // GraphicWin::init already pins that it is reached with the seam's
        // palette.
        write_at(storage.data(), 16 + kBuffer + 0x54,
                 static_cast<LPVOID *>(nullptr));
        std::memcpy(expected.data(), storage.data(), storage.size());
        g_init_rec = InitRecord{};
        g_init_base_result = 0;
        g_init_surface_result = 0;
        g_init_alloc_block = nullptr;
        g_init_nonclient_out_width = 0;
        g_init_nonclient_out_height = 0;
        g_button_rec = ButtonInitRecord{};
        g_button_close_installs = nullptr;
    };
    // GraphicWin::close's writes, which run inside GraphicWin::init.
    auto apply_graphic_close = [&](uint8_t *image) {
        const uint32_t zero = 0;
        write_at(image, 16 + 0xA10, zero);
        write_at(image, 16 + 0x134, zero);
        write_at(image, 16 + 0x138, zero);
        for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
            write_at(image, 16 + offset, zero);
        }
        write_at(image, 16 + 0xA08, static_cast<void *>(nullptr));
        write_at(image, 16 + 0xA0C, a0c_default);
    };

    // --- no parent: the close still runs, and nothing else does ---
    // The close precedes every guard, so even a rejected init leaves the
    // button torn down. Nothing is written because the virtual close is a
    // stub and GraphicWin::init is never reached.
    arrange();
    expect(button->init(nullptr, 7, 1, 2, 3, 4, nullptr, 0) == 3);
    expect(g_button_rec.close_calls == 1);
    expect(g_button_rec.close_self == storage.data() + 16);
    expect(g_init_rec.base_calls == 0);
    expect(g_button_rec.show_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- GraphicWin::init's code is passed straight through ---
    // Nothing after it runs: no id, no colours, no fonts, no show.
    arrange();
    g_init_base_result = 0x5A5A5A5A;
    apply_graphic_close(expected.data());
    expect(button->init(nullptr, 7, 1, 2, 3, 4, parent, 0) == 0x5A5A5A5A);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    expect(g_button_rec.show_calls == 0);

    // --- the style word, and the nine forwarded arguments ---
    struct StyleCase { int flag; int style; };
    const StyleCase style_cases[] = {
        {0, 0x01000320}, {1, 0x01008320}, {-1, 0x01008320},
        {INT_MIN, 0x01008320}, {0x8000, 0x01008320},
    };
    for (const StyleCase &test : style_cases) {
        arrange();
        g_init_base_result = 0x5A5A5A5A;
        expect(button->init(nullptr, 7, 11, 22, 33, 44, parent, test.flag)
               == 0x5A5A5A5A);
        expect(g_init_rec.base_calls == 1);
        // The four geometry values arrive as THIS function's arguments 3..6
        // and land in GraphicWin::init's 1..4; the name is not forwarded at
        // all and the caption, menu and border slots take literal nulls.
        expect(g_init_rec.base_args[0] == 11);
        expect(g_init_rec.base_args[1] == 22);
        expect(g_init_rec.base_args[2] == 33);
        expect(g_init_rec.base_args[3] == 44);
        expect(g_init_rec.base_args[4] == 0);
        expect(g_init_rec.base_args[5] == test.style);
        expect(g_init_rec.base_args[6] ==
               static_cast<int>(reinterpret_cast<intptr_t>(parent)));
        expect(g_init_rec.base_args[7] == 0);
        expect(g_init_rec.base_args[8] == 0);
    }

    // --- the private name copy ---
    // The copy is a fresh allocation, not the caller's pointer, and it holds
    // the caller's bytes. A null name leaves 0xA7C exactly as it was.
    char name_source[] = "button-name";
    arrange();
    g_init_base_result = 0x5A5A5A5A;
    const uintptr_t seeded_name =
        graphic_win_field(storage.data() + 16, 0xA7C);
    expect(button->init(nullptr, 7, 1, 2, 3, 4, parent, 0) == 0x5A5A5A5A);
    expect(graphic_win_field(storage.data() + 16, 0xA7C) == seeded_name);

    arrange();
    g_init_base_result = 0x5A5A5A5A;
    // Hand the allocator a dirty block of exactly the size the body will ask
    // for, so that the body emptying it before the concatenation is
    // observable. Without this the fresh block is usually already zero, the
    // `allocated[0] = '\0'` store looks redundant, and both mutants of it
    // survive. mem_get_old goes straight to malloc and is not a rebindable
    // seam, so recycling a block is the only lever available here.
    {
        void *const dirty = std::malloc(sizeof(name_source));
        std::memset(dirty, 'X', sizeof(name_source));
        std::free(dirty);
    }
    expect(button->init(name_source, 7, 1, 2, 3, 4, parent, 0) == 0x5A5A5A5A);
    LPSTR copied = nullptr;
    std::memcpy(&copied, storage.data() + 16 + 0xA7C, sizeof(copied));
    expect(copied != nullptr && copied != name_source);
    expect(std::strcmp(copied, name_source) == 0);
    std::free(copied);

    // --- the success path ---
    for (int restage = 0; restage < 2; ++restage) {
        arrange();
        if (restage) { g_button_close_installs = restaged; }
        apply_graphic_close(expected.data());
        write_at(expected.data(), 16 + 0x448,
                 static_cast<void *>(button));
        write_at(expected.data(), 16 + 0xA78, 0x1234U);
        // Twelve sentinels at 0x53C + slot * 0x10 + tier * 4, tier-major in
        // the table: index slot * 3 + tier.
        for (size_t slot = 0; slot < 4; ++slot) {
            for (size_t tier = 0; tier < 3; ++tier) {
                write_at(expected.data(),
                         16 + kBuffer + 0x53C + slot * 0x10 + tier * 4,
                         colors[slot * 3 + tier]);
            }
        }
        // set_font sees a primary that is present but not initialized, so it
        // leaves font1_ alone and stores the rest - including the literal
        // null fourth, which is not a fourth table entry.
        write_at(expected.data(), 16 + kBuffer + 0x530, fonts[1]);
        write_at(expected.data(), 16 + kBuffer + 0x534, fonts[2]);
        write_at(expected.data(), 16 + kBuffer + 0x538,
                 static_cast<Font *>(nullptr));
        if (restage) {
            write_at(expected.data(), 16, static_cast<void *>(restaged));
        }
        expect(button->init(nullptr, 0x1234, 1, 2, 3, 4, parent, 0) == 0);
        expect_storage_bytes(storage.data(), expected.data(), storage.size());
        expect(g_button_rec.show_calls == 1);
        expect(g_button_rec.show_self == storage.data() + 16);
        expect(g_button_rec.show_arg == 0);
        // Close first, GraphicWin::init's own sequence, then the show last.
        expect(g_button_rec.sequence == 0x89U);
        // The show's vtable is RE-READ: when the close restaged the table the
        // show must come from the new one, which a hoisted pointer would miss.
        expect(g_button_rec.show_vtable_seen ==
               (restage ? restaged : vtable));
    }

    // --- a null primary font short-circuits set_font, writing nothing ---
    // This is what pins fonts[0] as the FIRST argument: were the order
    // permuted, a non-null entry would reach the guard and the secondaries
    // would land.
    Font *null_first[3] = {nullptr, fonts[1], fonts[2]};
    BaseButtonDefaultFonts = null_first;
    arrange();
    apply_graphic_close(expected.data());
    write_at(expected.data(), 16 + 0x448, static_cast<void *>(button));
    write_at(expected.data(), 16 + 0xA78, 0x1234U);
    for (size_t slot = 0; slot < 4; ++slot) {
        for (size_t tier = 0; tier < 3; ++tier) {
            write_at(expected.data(),
                     16 + kBuffer + 0x53C + slot * 0x10 + tier * 4,
                     colors[slot * 3 + tier]);
        }
    }
    expect(button->init(nullptr, 0x1234, 1, 2, 3, 4, parent, 0) == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    BaseButtonDefaultFonts = fonts;

    // --- through the redirect adapter ---
    arrange();
    g_init_base_result = 0x5A5A5A5A;
    expect(base_button_init_redirect(button, nullptr, nullptr, 7, 11, 22, 33,
                                     44, parent, 1) == 0x5A5A5A5A);
    expect(g_init_rec.base_args[5] == 0x01008320);

    // --- ImageButton::init, which wraps this one ---
    //
    // It keeps its FIRST argument at 0xab8 and forwards the other eight
    // unchanged. Reusing the arrangement above is the point: everything
    // BaseButton::init needs is already standing, so this fixture is about
    // the wrapper and nothing else.
    arrange();
    g_init_base_result = 0x3C3C3C3C;
    expect(image_button_init_redirect(button, nullptr, 0x0BADCAFE, nullptr, 7,
                                      11, 22, 33, 44, parent, 1)
           == 0x3C3C3C3C);
    {
        uint32_t kept;
        std::memcpy(&kept,
                    reinterpret_cast<uint8_t *>(button) + 0xAB8, sizeof(kept));
        expect(kept == 0x0BADCAFEU);
    }
    // The eight forwarded arguments arrive exactly as the direct call's did.
    // They are checked INDIVIDUALLY and with distinct values: base_args[5]
    // alone is a composite that survives an x/y swap, which is exactly the
    // mistake a nine-argument forwarder is likely to make.
    expect(g_init_rec.base_args[0] == 11);      // x
    expect(g_init_rec.base_args[1] == 22);      // y
    expect(g_init_rec.base_args[2] == 33);      // width
    expect(g_init_rec.base_args[3] == 44);      // height
    expect(g_init_rec.base_args[5] == 0x01008320);

    // --- ImageButton::construct and ::close ---
    arrange();
    expect(image_button_construct_redirect(button, nullptr) == button);
    {
        uint32_t primary, buffer, field;
        auto *const bytes = reinterpret_cast<uint8_t *>(button);
        std::memcpy(&primary, bytes + 0x0, sizeof(primary));
        std::memcpy(&buffer, bytes + 0x444, sizeof(buffer));
        std::memcpy(&field, bytes + 0xAB8, sizeof(field));
        // Both vtables, and they are NOT the same address - a body publishing
        // one of them twice would pass a test that only checked offset 0.
        expect(primary == ImageButtonPrimaryVtable);
        expect(buffer == ImageButtonBufferVtable);
        expect(ImageButtonPrimaryVtable != ImageButtonBufferVtable);
        // Published AFTER the base constructor, so they are not BaseButton's.
        expect(primary != BaseButtonPrimaryVtable);
        expect(buffer != BaseButtonBufferVtable);
        expect(field == 0U);

        // close clears the field again, having been given something to clear
        // - AND delegates. Without the second assertion, dropping the
        // BaseButton::close call entirely passes: clearing 0xab8 is the only
        // thing this function does on its own.
        const uint32_t marker = 0x5EED5EEDU;
        std::memcpy(bytes + 0xAB8, &marker, sizeof(marker));
        const int closes_before = g_init_rec.close_win_calls;
        image_button_close_redirect(button, nullptr);
        std::memcpy(&field, bytes + 0xAB8, sizeof(field));
        expect(field == 0U);
        expect(g_init_rec.close_win_calls > closes_before);
    }

    WinOriginalClose = saved_win_close;
    BufferSubobjectClose = saved_buffer_close;
    GraphicWinFieldA0CDefault = saved_a0c_default;
    WinOriginalInit = saved_base;
    GraphicWinOriginalComputeMinSize = saved_min;
    WinOriginalNonclientToClient = saved_nonclient;
    BufferOriginalInit = saved_surface;
    GraphicWinInitDefaults = saved_defaults;
    WaveOperatorNew = saved_alloc;
    ScrollDefaultThickness = saved_thickness;
    WinActivePalette = saved_win_palette;
    BufferResetValue520 = saved_reset_520;
    FontDefaultPtr = saved_font_default;
    BufferPalette = saved_buffer_palette;
    BaseButtonDefaultTextColors = saved_button_colors;
    BaseButtonDefaultFonts = saved_button_fonts;
}

void test_graphic_win_close() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "GraphicWin close tests require the legacy layout");
    struct FakeReleaseObject {
        uintptr_t *vtable;
    };
    uintptr_t release_vtable[] = {
        reinterpret_cast<uintptr_t>(&graphic_win_stub_release),
    };
    FakeReleaseObject release_object = {release_vtable};

    // mode 0: null throughout; mode 1: present throughout; modes 2 and 3
    // prove field_A08_ is loaded after both dependency calls by having the
    // Buffer close stand-in install or remove the release target.
    for (int mode = 0; mode < 4; ++mode) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(GraphicWin) uint8_t storage[sizeof(GraphicWin) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            auto *const self = reinterpret_cast<GraphicWin *>(storage + 16);
            void *const initial_target = (mode == 1 || mode == 3)
                ? &release_object : nullptr;
            void *const final_target = (mode == 1 || mode == 2)
                ? &release_object : nullptr;
            write_at(storage, 16 + 0xA08, initial_target);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t initial_a10 = graphic_win_field(storage + 16, 0xA10);
            const uint32_t initial_134 = graphic_win_field(storage + 16, 0x134);
            const uint32_t initial_9cc = graphic_win_field(storage + 16, 0x9CC);

            const uint32_t zero = 0;
            write_at(expected, 16 + 0xA10, zero);
            write_at(expected, 16 + 0x134, zero);
            write_at(expected, 16 + 0x138, zero);
            for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
                write_at(expected, 16 + offset, zero);
            }
            uint32_t default_a0c = 0xC6A51F73U;
            write_at(expected, 16 + 0xA08, static_cast<void *>(nullptr));
            write_at(expected, 16 + 0xA0C, default_a0c);

            func_subobject_close *const saved_win = WinOriginalClose;
            func_subobject_close *const saved_buffer = BufferSubobjectClose;
            uint32_t *const saved_default = GraphicWinFieldA0CDefault;
            WinOriginalClose = graphic_win_stub_win_close;
            BufferSubobjectClose = graphic_win_stub_buffer_close;
            GraphicWinFieldA0CDefault = &default_a0c;
            graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
            graphic_win_close_stub_record.replace_a08 = mode >= 2;
            graphic_win_close_stub_record.replacement_a08 = final_target;

            const uint32_t result = use_adapter
                ? graphic_win_close_redirect(self, nullptr)
                : self->close();

            WinOriginalClose = saved_win;
            BufferSubobjectClose = saved_buffer;
            GraphicWinFieldA0CDefault = saved_default;

            const GraphicWinCloseStubRecord &record =
                graphic_win_close_stub_record;
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(record.win_calls == 1);
            expect(record.win_target == self);
            expect(record.buffer_calls == 1);
            expect(record.buffer_target == storage + 16 + 0x444);
            expect(record.sequence == (final_target ? 0x123U : 0x12U));
            for (int dependency = 0; dependency < 2; ++dependency) {
                expect(record.dependency_seen_a10[dependency] == initial_a10);
                expect(record.dependency_seen_134[dependency] == initial_134);
                expect(record.dependency_seen_9cc[dependency] == initial_9cc);
            }
            if (final_target) {
                expect(result == 0x7B3D19E5U);
                expect(record.release_calls == 1);
                expect(record.release_target == &release_object);
                expect(record.release_flags == 1U);
                expect(record.release_seen_a10 == 0);
                expect(record.release_seen_134 == 0);
                expect(record.release_seen_9cc == 0);
                expect(record.release_seen_a08
                       == reinterpret_cast<uintptr_t>(&release_object));
                expect(record.release_seen_a0c == default_a0c);
            } else {
                expect(result == default_a0c);
                expect(record.release_calls == 0);
            }
        }
    }
}

int button_lifecycle_free_calls = 0;

void *button_lifecycle_free_targets[2] = {};

uint32_t button_lifecycle_free_a74[2] = {};

uint32_t button_lifecycle_free_a84[2] = {};

uint32_t button_lifecycle_free_a7c[2] = {};

uint32_t button_lifecycle_free_a80[2] = {};

uint32_t button_lifecycle_free_aa8[2] = {};

void reset_button_lifecycle_frees() {
    button_lifecycle_free_calls = 0;
    std::memset(button_lifecycle_free_targets, 0,
                sizeof(button_lifecycle_free_targets));
    std::memset(button_lifecycle_free_a74, 0,
                sizeof(button_lifecycle_free_a74));
    std::memset(button_lifecycle_free_a84, 0,
                sizeof(button_lifecycle_free_a84));
    std::memset(button_lifecycle_free_a7c, 0,
                sizeof(button_lifecycle_free_a7c));
    std::memset(button_lifecycle_free_a80, 0,
                sizeof(button_lifecycle_free_a80));
    std::memset(button_lifecycle_free_aa8, 0,
                sizeof(button_lifecycle_free_aa8));
}

void *button_lifecycle_free_probe(void *block) {
    const int call = button_lifecycle_free_calls++;
    if (call < static_cast<int>(ARRAYSIZE(button_lifecycle_free_targets))) {
        button_lifecycle_free_targets[call] = block;
        button_lifecycle_free_a74[call] = button_lifecycle_read(0xA74);
        button_lifecycle_free_a84[call] = button_lifecycle_read(0xA84);
        button_lifecycle_free_a7c[call] = button_lifecycle_read(0xA7C);
        button_lifecycle_free_a80[call] = button_lifecycle_read(0xA80);
        button_lifecycle_free_aa8[call] = button_lifecycle_read(0xAA8);
    }
    return reinterpret_cast<void *>(
        0xC1050000U + static_cast<uint32_t>(call + 1));
}

void write_button_graphic_close_expected(
        uint8_t *expected, uint32_t graphic_default) {
    write_at(expected, 16 + 0x134, 0U);
    write_at(expected, 16 + 0x138, 0U);
    for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA08, 0U);
    write_at(expected, 16 + 0xA0C, graphic_default);
    write_at(expected, 16 + 0xA10, 0U);
}

void write_base_button_close_expected(
        uint8_t *expected, const uint32_t fixed[5],
        const uint32_t dynamic[2], uint32_t graphic_default) {
    write_button_graphic_close_expected(expected, graphic_default);
    write_at(expected, 16 + 0xA44, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA48, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA74, 0U);
    write_at(expected, 16 + 0xA78, 0U);
    write_at(expected, 16 + 0xA7C, 0U);
    write_at(expected, 16 + 0xA80, 0U);
    for (size_t index = 0; index < 4; ++index) {
        write_at(expected, 16 + 0xA84 + index * 4, fixed[index]);
    }
    write_at(expected, 16 + 0xA94, dynamic[0]);
    write_at(expected, 16 + 0xA98, dynamic[1]);
    write_at(expected, 16 + 0xA9C, 0U);
    write_at(expected, 16 + 0xAA0, fixed[4]);
    write_at(expected, 16 + 0xAA4, 0U);
    write_at(expected, 16 + 0xAA8, 0U);
    write_at(expected, 16 + 0xAAC, 0U);
    write_at(expected, 16 + 0xAB0, 0U);
    write_at(expected, 16 + 0xAB4, 0U);
}

void write_time_close_expected(uint8_t *expected, size_t offset) {
    std::memset(expected + 16 + offset, 0, sizeof(Time));
    write_at(expected, 16 + offset + 0x20, 5U);
}

void write_flat_button_close_expected(
        uint8_t *expected, const uint32_t base_fixed[5],
        const uint32_t base_dynamic[2], const uint32_t flat_defaults[27],
        uint32_t graphic_default) {
    write_time_close_expected(expected, 0xA1C);
    write_at(expected, 16 + 0xA14, 0U);
    write_at(expected, 16 + 0xA18, 0U);
    write_at(expected, 16 + 0xAB8, 0xFFFFFFFFU);
    write_base_button_close_expected(
        expected, base_fixed, base_dynamic, graphic_default);
    for (size_t index = 0; index < 3; ++index) {
        write_at(expected, 16 + 0xABC + index * 4, 0xFFFFFFFFU);
        write_at(expected, 16 + 0xAC8 + index * 4, 0U);
        write_at(expected, 16 + 0xAD4 + index * 4, 0U);
        for (size_t group = 0; group < 9; ++group) {
            write_at(expected,
                     16 + 0xAE0 + group * 0xC + index * 4,
                     flat_defaults[group * 3 + index]);
        }
    }
}

void prepare_button_lifecycle_storage(
        uint8_t *storage, uint8_t *expected, size_t size) {
    seed_storage(storage, expected, size);
    write_at(storage, 16 + 0xA08, 0U);
    write_at(storage, 16 + 0xA7C, 0U);
    write_at(storage, 16 + 0xA80, 0U);
    std::memcpy(expected, storage, size);
}

void test_base_button_and_flat_button_lifecycle() {
    static_assert(sizeof(BaseButton) == 0xAB8,
                  "BaseButton lifecycle tests require the legacy layout");
    static_assert(sizeof(FlatButton) == 0xB4C,
                  "FlatButton lifecycle tests require the legacy layout");

    uint32_t base_fixed[5];
    uint32_t base_dynamic[2];
    uint32_t flat_defaults[27];
    for (size_t index = 0; index < ARRAYSIZE(base_fixed); ++index) {
        base_fixed[index] = 0x51000000U
            + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(base_dynamic); ++index) {
        base_dynamic[index] = 0xA1000000U
            + static_cast<uint32_t>(index) * 0x011011U;
    }
    for (size_t index = 0; index < ARRAYSIZE(flat_defaults); ++index) {
        flat_defaults[index] = 0xD1000000U
            + static_cast<uint32_t>(index) * 0x010101U;
    }
    uint32_t graphic_default = 0x7B3D19E5U;

    func2 *const saved_free = _free;
    uint32_t *const saved_base_fixed = BaseButtonStaticDefaults;
    uint32_t *const saved_base_dynamic = BaseButtonDynamicDefaults;
    uint32_t *const saved_flat_defaults = FlatButtonDefaults;
    func_subobject_close *const saved_win_close = WinOriginalClose;
    func_subobject_close *const saved_buffer_close = BufferSubobjectClose;
    uint32_t *const saved_graphic_default = GraphicWinFieldA0CDefault;
    func_subobject_destructor *const saved_buffer_destructor =
        BufferSubobjectDestructor;
    func_subobject_destructor *const saved_win_destructor =
        WinOriginalDestructor;
    _free = button_lifecycle_free_probe;
    BaseButtonStaticDefaults = base_fixed;
    BaseButtonDynamicDefaults = base_dynamic;
    FlatButtonDefaults = flat_defaults;
    WinOriginalClose = graphic_win_stub_win_close;
    BufferSubobjectClose = graphic_win_stub_buffer_close;
    GraphicWinFieldA0CDefault = &graphic_default;
    BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
    WinOriginalDestructor = graphic_win_stub_win_destructor;

    // BaseButton::close: all four allocation shapes, through both entry
    // points. The free probe observes the exact name-before-bubble order and
    // the pre-clear pointer values; the second free's return residue survives.
    for (int allocation_mask = 0; allocation_mask < 4; ++allocation_mask) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(BaseButton) uint8_t storage[sizeof(BaseButton) + 32];
            uint8_t expected[sizeof(storage)];
            prepare_button_lifecycle_storage(
                storage, expected, sizeof(storage));
            auto *const self = reinterpret_cast<BaseButton *>(storage + 16);
            void *const name = allocation_mask & 1
                ? reinterpret_cast<void *>(0x11110001U) : nullptr;
            void *const bubble = allocation_mask & 2
                ? reinterpret_cast<void *>(0x22220002U) : nullptr;
            write_at(storage, 16 + 0xA7C, name);
            write_at(storage, 16 + 0xA80, bubble);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t initial_primary = graphic_win_field(self, 0x000);
            const uint32_t initial_buffer = graphic_win_field(self, 0x444);
            const uint32_t initial_a14 = graphic_win_field(self, 0xA14);
            const uint32_t initial_a18 = graphic_win_field(self, 0xA18);
            const uint32_t initial_ab8 = graphic_win_field(self, 0xAB8);
            const uint32_t initial_abc = graphic_win_field(self, 0xABC);
            const uint32_t initial_aa8 = graphic_win_field(self, 0xAA8);
            write_base_button_close_expected(
                expected, base_fixed, base_dynamic, graphic_default);

            reset_button_lifecycle_capture(storage + 16);
            reset_button_lifecycle_frees();
            graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
            const uint32_t result = use_adapter
                ? base_button_close_redirect(self, nullptr) : self->close();

            const int expected_frees = (name ? 1 : 0) + (bubble ? 1 : 0);
            const uint32_t expected_result = bubble
                ? 0xC1050000U + static_cast<uint32_t>(expected_frees) : 0U;
            expect(result == expected_result);
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(button_lifecycle_close_calls == 1);
            expect(button_lifecycle_close_time_counts[0] == 0);
            expect(button_lifecycle_close_primary_vtables[0]
                   == initial_primary);
            expect(button_lifecycle_close_buffer_vtables[0]
                   == initial_buffer);
            expect(button_lifecycle_close_a14[0] == initial_a14);
            expect(button_lifecycle_close_a18[0] == initial_a18);
            expect(button_lifecycle_close_ab8[0] == initial_ab8);
            expect(button_lifecycle_close_abc[0] == initial_abc);
            expect(button_lifecycle_free_calls == expected_frees);
            int free_index = 0;
            if (name) {
                expect(button_lifecycle_free_targets[free_index] == name);
                expect(button_lifecycle_free_a7c[free_index]
                       == reinterpret_cast<uintptr_t>(name));
                expect(button_lifecycle_free_a80[free_index]
                       == reinterpret_cast<uintptr_t>(bubble));
                ++free_index;
            }
            if (bubble) {
                expect(button_lifecycle_free_targets[free_index] == bubble);
                expect(button_lifecycle_free_a7c[free_index] == 0U);
                expect(button_lifecycle_free_a80[free_index]
                       == reinterpret_cast<uintptr_t>(bubble));
            }
            for (int call = 0; call < expected_frees; ++call) {
                expect(button_lifecycle_free_a74[call] == 0U);
                expect(button_lifecycle_free_a84[call] == base_fixed[0]);
                expect(button_lifecycle_free_aa8[call] == initial_aa8);
            }
        }
    }

    // FlatButton::close: Time closes before the Flat fields, Base close sees
    // the orientation fields but not the later table reset, and EAX retains
    // the legacy interior pointer computed by the final loop.
    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(FlatButton) uint8_t storage[sizeof(FlatButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<FlatButton *>(storage + 16);
        const uint32_t initial_primary = graphic_win_field(self, 0x000);
        const uint32_t initial_buffer = graphic_win_field(self, 0x444);
        const uint32_t initial_abc = graphic_win_field(self, 0xABC);
        write_flat_button_close_expected(
            expected, base_fixed, base_dynamic, flat_defaults,
            graphic_default);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        const uint32_t result = use_adapter
            ? flat_button_close_redirect(self, nullptr) : self->close();

        expect(result == reinterpret_cast<uintptr_t>(self) + 0xAECU);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(time_close_calls == 1);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_close_calls == 1);
        expect(button_lifecycle_close_time_counts[0] == 1);
        expect(button_lifecycle_close_primary_vtables[0] == initial_primary);
        expect(button_lifecycle_close_buffer_vtables[0] == initial_buffer);
        expect(button_lifecycle_close_a14[0] == 0U);
        expect(button_lifecycle_close_a18[0] == 0U);
        expect(button_lifecycle_close_ab8[0] == 0xFFFFFFFFU);
        expect(button_lifecycle_close_abc[0] == initial_abc);
        expect(button_lifecycle_free_calls == 0);
    }

    // BaseButton destructor: install its two vtables, close, destroy Time2
    // then Time1, and delegate to GraphicWin teardown. The final GraphicWin
    // tables hide the initial stores, so the close seam snapshots them.
    {
        alignas(BaseButton) uint8_t storage[sizeof(BaseButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<BaseButton *>(storage + 16);
        write_base_button_close_expected(
            expected, base_fixed, base_dynamic, graphic_default);
        write_time_close_expected(expected, 0xA4C);
        write_time_close_expected(expected, 0xA1C);
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        graphic_win_stub_record = GraphicWinStubRecord{};
        expect(base_button_destructor_redirect(self, nullptr) == self);

        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(button_lifecycle_close_calls == 1);
        expect(button_lifecycle_close_time_counts[0] == 0);
        expect(button_lifecycle_close_primary_vtables[0]
               == BaseButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[0]
               == BaseButtonBufferVtable);
        expect(time_close_calls == 2);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA4C));
        expect(time_close_targets[1]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_destructor_time_count == 2);
        expect(graphic_win_stub_record.sequence == 0x21);
        expect(button_lifecycle_free_calls == 0);
    }

    // FlatButton destructor composes Flat close with the complete BaseButton
    // destructor. This intentionally closes Time1 twice, exactly as the
    // original two-stage destructor does.
    {
        alignas(FlatButton) uint8_t storage[sizeof(FlatButton) + 32];
        uint8_t expected[sizeof(storage)];
        prepare_button_lifecycle_storage(storage, expected, sizeof(storage));
        auto *const self = reinterpret_cast<FlatButton *>(storage + 16);
        const uint32_t initial_abc = graphic_win_field(self, 0xABC);
        write_flat_button_close_expected(
            expected, base_fixed, base_dynamic, flat_defaults,
            graphic_default);
        write_time_close_expected(expected, 0xA4C);
        write_time_close_expected(expected, 0xA1C);
        write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
        write_at(expected, 16 + 0x444, GraphicWinBufferVtable);

        reset_button_lifecycle_capture(storage + 16);
        reset_button_lifecycle_frees();
        graphic_win_close_stub_record = GraphicWinCloseStubRecord{};
        graphic_win_stub_record = GraphicWinStubRecord{};
        expect(flat_button_destructor_redirect(self, nullptr) == self);

        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(button_lifecycle_close_calls == 2);
        expect(button_lifecycle_close_time_counts[0] == 1);
        expect(button_lifecycle_close_time_counts[1] == 1);
        expect(button_lifecycle_close_primary_vtables[0]
               == FlatButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[0]
               == FlatButtonBufferVtable);
        expect(button_lifecycle_close_primary_vtables[1]
               == BaseButtonPrimaryVtable);
        expect(button_lifecycle_close_buffer_vtables[1]
               == BaseButtonBufferVtable);
        expect(button_lifecycle_close_abc[0] == initial_abc);
        expect(button_lifecycle_close_abc[1] == 0xFFFFFFFFU);
        expect(time_close_calls == 3);
        expect(time_close_targets[0]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(time_close_targets[1]
               == reinterpret_cast<Time *>(storage + 16 + 0xA4C));
        expect(time_close_targets[2]
               == reinterpret_cast<Time *>(storage + 16 + 0xA1C));
        expect(button_lifecycle_destructor_time_count == 3);
        expect(graphic_win_stub_record.sequence == 0x21);
        expect(button_lifecycle_free_calls == 0);
    }

    button_lifecycle_capture = false;
    button_lifecycle_base = nullptr;
    _free = saved_free;
    BaseButtonStaticDefaults = saved_base_fixed;
    BaseButtonDynamicDefaults = saved_base_dynamic;
    FlatButtonDefaults = saved_flat_defaults;
    WinOriginalClose = saved_win_close;
    BufferSubobjectClose = saved_buffer_close;
    GraphicWinFieldA0CDefault = saved_graphic_default;
    BufferSubobjectDestructor = saved_buffer_destructor;
    WinOriginalDestructor = saved_win_destructor;
    time_close_calls = 0;
    std::memset(time_close_targets, 0, sizeof(time_close_targets));
}

namespace {

struct ListBoxEvent {
    int kind;               // 1 win close, 2 buffer close, 3 dialog close
    const void *target;
    uint32_t graphic_vptr;  // observed [obj + graphic_disp] at call time
    uint32_t dialog_vptr;   // observed [obj + dialog_disp] at call time
};

// Sixteen, not eight: test_report_if_close_energy drives THREE ListBox::close
// calls and each emits a win close, a buffer close and a dialog close, so
// eight slots silently dropped the last one and the third dialog close was
// simply never recorded. list_box_record bounds-checks with ARRAYSIZE, so
// the drop was invisible rather than an overrun.
ListBoxEvent list_box_events[16];

int list_box_event_count = 0;

uint8_t *list_box_obj = nullptr;

int32_t list_box_graphic_disp = 0;

int32_t list_box_dialog_disp = 0;

void list_box_record(int kind, const void *target) {
    const int index = list_box_event_count++;
    if (index >= static_cast<int>(ARRAYSIZE(list_box_events))) {
        return;
    }
    ListBoxEvent &event = list_box_events[index];
    event.kind = kind;
    event.target = target;
    std::memcpy(&event.graphic_vptr, list_box_obj + list_box_graphic_disp, 4);
    std::memcpy(&event.dialog_vptr, list_box_obj + list_box_dialog_disp, 4);
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

// GraphicWin::close's two sub-seams and the Dialog::close seam, all recording.
void __thiscall list_box_win_close_probe(void *self) { list_box_record(1, self); }

void __thiscall list_box_buffer_close_probe(void *self) { list_box_record(2, self); }

void __thiscall list_box_dialog_close_probe(Dialog *self) { list_box_record(3, self); }

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

// Controlled default tables, so every defaulted field is a distinct sentinel.
uint32_t list_box_static_defaults[4];

uint32_t list_box_dynamic_default;

uint32_t list_box_graphic_a0c_default;

// The ListBox-owned field resets close() performs, applied to a reference copy.
void list_box_apply_resets(uint8_t *obj, int32_t dialog_disp) {
    auto set = [&](size_t off, uint32_t value) {
        std::memcpy(obj + off, &value, sizeof(value));
    };
    set(0x04, 0); set(0x08, 0); set(0x18, 0); set(0x1C, 0); set(0x20, 0);
    set(0x24, list_box_static_defaults[0]);   // 0x006970E0
    set(0x28, list_box_dynamic_default);      // 0x009B8EE0
    set(0x2C, list_box_static_defaults[1]);   // 0x006970E4
    set(0x10, list_box_static_defaults[2]);   // 0x006970E8
    set(0x14, list_box_static_defaults[3]);   // 0x006970EC
    set(dialog_disp + 0xB4, 1);               // Dialog::field_B4_
    set(0x30, 0); set(0x34, 0); set(0x38, 0); set(0x3C, 0); set(0x40, 0);
    set(0x0C, 0);
}

// The three vtable installs + two vbase-adjust writes destroy() performs.
void list_box_apply_stages(uint8_t *obj, int32_t g, int32_t d) {
    const uint32_t gw = 0x0067041C, bf = 0x00670414, dl = 0x00670408;
    std::memcpy(obj + g, &gw, 4);
    std::memcpy(obj + g + 0x444, &bf, 4);
    std::memcpy(obj + d, &dl, 4);
    const int32_t ga = g - 0x48, da = d - 0xa60;
    std::memcpy(obj + g - 4, &ga, 4);
    std::memcpy(obj + d - 4, &da, 4);
}

}  // namespace

void test_list_box_teardown() {
    static_assert(sizeof(GraphicWin) == 0xA14,
                  "ListBox teardown test requires the legacy GraphicWin layout");
    static_assert(sizeof(Dialog) == 0xF4,
                  "ListBox teardown test requires the legacy Dialog layout");

    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_a0c = GraphicWinFieldA0CDefault;
    func_dialog_close *const saved_dialog = ListBoxOriginalDialogClose;
    uint32_t *const saved_static = ListBoxCloseStaticDefaults;
    uint32_t *const saved_dynamic = ListBoxCloseDynamicDefault;

    for (int i = 0; i < 4; ++i) {
        list_box_static_defaults[i] =
            0x515A0000U + static_cast<uint32_t>(i) * 0x010203U;
    }
    list_box_dynamic_default = 0xB16B00B5U;
    list_box_graphic_a0c_default = 0x7B3D19E5U;

    WinOriginalClose = list_box_win_close_probe;
    BufferSubobjectClose = list_box_buffer_close_probe;
    GraphicWinFieldA0CDefault = &list_box_graphic_a0c_default;
    ListBoxOriginalDialogClose = list_box_dialog_close_probe;
    ListBoxCloseStaticDefaults = list_box_static_defaults;
    ListBoxCloseDynamicDefault = &list_box_dynamic_default;

    struct Shape { int32_t g; int32_t d; };
    // {most-derived}, {embedded / shifted vbtable}.
    const Shape shapes[2] = { {0x48, 0xa60}, {0x60, 0xB00} };

    for (int s = 0; s < 2; ++s) {
        const int32_t g = shapes[s].g;
        const int32_t d = shapes[s].d;
        int32_t vbtable[3] = { 0, g, d };

        for (int is_dtor = 0; is_dtor < 2; ++is_dtor) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                // Big enough for the shifted Dialog subobject + trailing canary.
                alignas(uint32_t) uint8_t storage[0xC40];
                alignas(uint32_t) uint8_t reference[0xC40];
                seed_storage(storage, reference, sizeof(storage));

                // Point [L] at this shape's vbtable and null the GraphicWin
                // release target (field_A08_) so GraphicWin::close stays on its
                // no-release path. Re-sync the reference to these edits.
                int32_t *const vbptr = vbtable;
                std::memcpy(storage + 16 + 0x00, &vbptr, sizeof(vbptr));
                const uint32_t zero = 0;
                std::memcpy(storage + 16 + g + 0xA08, &zero, sizeof(zero));
                std::memcpy(reference, storage, sizeof(storage));

                uint8_t *const obj = storage + 16;
                uint8_t *const ref_obj = reference + 16;

                // Build the reference bytes with the same source dependency:
                // (destructor) stage the vtables, then run the real
                // GraphicWin::close over the base region, then the ListBox
                // resets. The probe events from this build are discarded.
                list_box_obj = ref_obj;
                list_box_graphic_disp = g;
                list_box_dialog_disp = d;
                if (is_dtor) {
                    list_box_apply_stages(ref_obj, g, d);
                }
                reinterpret_cast<GraphicWin *>(ref_obj + g)->close();
                list_box_apply_resets(ref_obj, d);

                // Run the function under test on the storage copy.
                list_box_obj = obj;
                list_box_event_count = 0;
                auto *const self = reinterpret_cast<ListBox *>(obj);
                uint32_t result;
                if (is_dtor) {
                    result = use_adapter
                        ? list_box_destructor_redirect(
                              obj + ListBoxDestructorAdjustment, nullptr)
                        : self->destroy();
                } else {
                    result = use_adapter
                        ? list_box_close_redirect(self, nullptr)
                        : self->close();
                }

                // Byte-exact object and complete leading/trailing canaries.
                expect_storage_bytes(storage, reference, sizeof(storage));
                expect(result == 0U);

                // Call order: GraphicWin::close (Win then Buffer) then Dialog::close,
                // each on the subobject located through the runtime displacement.
                expect(list_box_event_count == 3);
                expect(list_box_events[0].kind == 1);
                expect(list_box_events[0].target == obj + g);
                expect(list_box_events[1].kind == 2);
                expect(list_box_events[1].target == obj + g + 0x444);
                expect(list_box_events[2].kind == 3);
                expect(list_box_events[2].target == obj + d);

                if (is_dtor) {
                    // The destructor staged the base vtables before any close,
                    // so every dependency observes ListBox's staged values (no
                    // later stage overwrites them here).
                    for (int e = 0; e < 3; ++e) {
                        expect(list_box_events[e].graphic_vptr == 0x0067041CU);
                        expect(list_box_events[e].dialog_vptr == 0x00670408U);
                    }
                    uint32_t buffer_vtable = 0;
                    std::memcpy(&buffer_vtable, obj + g + 0x444, 4);
                    expect(buffer_vtable == 0x00670414U);
                    // vbase-adjust = runtime displacement minus most-derived offset.
                    int32_t graphic_adjust = 0;
                    int32_t dialog_adjust = 0;
                    std::memcpy(&graphic_adjust, obj + g - 4, 4);
                    std::memcpy(&dialog_adjust, obj + d - 4, 4);
                    expect(graphic_adjust == g - 0x48);
                    expect(dialog_adjust == d - 0xa60);
                }
            }
        }
    }

    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_a0c;
    ListBoxOriginalDialogClose = saved_dialog;
    ListBoxCloseStaticDefaults = saved_static;
    ListBoxCloseDynamicDefault = saved_dynamic;
}

ScrollDtorEvent scroll_dtor_events[24];

int scroll_dtor_event_count = 0;

uint8_t *scroll_dtor_base = nullptr;

void test_report_if_close_energy() {
    // close_intel's two subobjects, plus a PROCESS-WIDE ListBox that goes
    // FIRST. The global is repointed at storage this fixture owns rather than
    // touching 0x0087BE84, exactly as the MapWin fixture does for
    // ConsoleGlobal, so the ordering can be observed without the real object.
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_a0c = GraphicWinFieldA0CDefault;
    func_dialog_close *const saved_dialog = ListBoxOriginalDialogClose;
    uint32_t *const saved_static = ListBoxCloseStaticDefaults;
    uint32_t *const saved_dynamic = ListBoxCloseDynamicDefault;
    ListBox *const saved_energy = ReportIfEnergyListBox;

    WinOriginalClose = list_box_win_close_probe;
    BufferSubobjectClose = list_box_buffer_close_probe;
    GraphicWinFieldA0CDefault = &list_box_graphic_a0c_default;
    ListBoxOriginalDialogClose = list_box_dialog_close_probe;
    ListBoxCloseStaticDefaults = list_box_static_defaults;
    ListBoxCloseDynamicDefault = &list_box_dynamic_default;

    const int32_t graphic = 0x48;
    const int32_t dialog = 0xA60;
    static int32_t vbtable[3];
    vbtable[0] = 0; vbtable[1] = graphic; vbtable[2] = dialog;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        std::vector<uint8_t> storage(0xAE24 + 0xC40 + 32);
        std::vector<uint8_t> global(0xC40);
        for (size_t i = 0; i < storage.size(); ++i) {
            storage[i] = static_cast<uint8_t>(0x35 + i * 17);
        }
        for (size_t i = 0; i < global.size(); ++i) {
            global[i] = static_cast<uint8_t>(0x77 + i * 13);
        }
        uint8_t *const object = storage.data() + 16;
        const int32_t offsets[2] = {0x0A2D0, 0x0AE24};
        int32_t *pointer = vbtable;
        const uint32_t zero = 0;
        for (int which = 0; which < 2; ++which) {
            uint8_t *const box = object + offsets[which];
            std::memcpy(box, &pointer, sizeof(pointer));
            std::memcpy(box + graphic + 0xA08, &zero, sizeof(zero));
        }
        std::memcpy(global.data(), &pointer, sizeof(pointer));
        std::memcpy(global.data() + graphic + 0xA08, &zero, sizeof(zero));
        ReportIfEnergyListBox = reinterpret_cast<ListBox *>(global.data());

        list_box_obj = object + offsets[0];
        list_box_graphic_disp = graphic;
        list_box_dialog_disp = dialog;
        list_box_event_count = 0;

        auto *const self = reinterpret_cast<ReportIf *>(object);
        if (use_adapter) {
            report_if_close_energy_redirect(self, nullptr);
        } else {
            self->close_energy();
        }

        const void *dialogs[3] = {nullptr, nullptr, nullptr};
        int seen = 0;
        for (int i = 0; i < list_box_event_count &&
                 i < static_cast<int>(ARRAYSIZE(list_box_events)); ++i) {
            if (list_box_events[i].kind == 3 && seen < 3) {
                dialogs[seen++] = list_box_events[i].target;
            }
        }
        // Three closes, and the GLOBAL one is first.
        expect(seen == 3);
        expect(dialogs[0] == global.data() + dialog);
        expect(dialogs[1] == object + offsets[0] + dialog);
        expect(dialogs[2] == object + offsets[1] + dialog);
    }

    ReportIfEnergyListBox = saved_energy;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_a0c;
    ListBoxOriginalDialogClose = saved_dialog;
    ListBoxCloseStaticDefaults = saved_static;
    ListBoxCloseDynamicDefault = saved_dynamic;
}

void test_report_if_close_intel() {
    // Two ListBox subobjects at 0xA2D0 and 0xAE24, closed in that order.
    // ListBox::close is already covered by test_list_box_teardown, so what
    // this establishes is the two OFFSETS and their ORDER - the dialog-close
    // seam fires once per ListBox, and each firing carries the Dialog address
    // that ListBox resolved from its own vbtable, which identifies which
    // subobject it was.
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_a0c = GraphicWinFieldA0CDefault;
    func_dialog_close *const saved_dialog = ListBoxOriginalDialogClose;
    uint32_t *const saved_static = ListBoxCloseStaticDefaults;
    uint32_t *const saved_dynamic = ListBoxCloseDynamicDefault;

    WinOriginalClose = list_box_win_close_probe;
    BufferSubobjectClose = list_box_buffer_close_probe;
    GraphicWinFieldA0CDefault = &list_box_graphic_a0c_default;
    ListBoxOriginalDialogClose = list_box_dialog_close_probe;
    ListBoxCloseStaticDefaults = list_box_static_defaults;
    ListBoxCloseDynamicDefault = &list_box_dynamic_default;

    const int32_t graphic = 0x48;
    const int32_t dialog = 0xA60;
    static int32_t vbtable[3];
    vbtable[0] = 0; vbtable[1] = graphic; vbtable[2] = dialog;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        std::vector<uint8_t> storage(0xAE24 + 0xC40 + 32);
        for (size_t i = 0; i < storage.size(); ++i) {
            storage[i] = static_cast<uint8_t>(0x35 + i * 17);
        }
        uint8_t *const object = storage.data() + 16;
        const int32_t offsets[2] = {0x0A2D0, 0x0AE24};
        for (int which = 0; which < 2; ++which) {
            uint8_t *const box = object + offsets[which];
            int32_t *pointer = vbtable;
            std::memcpy(box, &pointer, sizeof(pointer));
            // Keep GraphicWin::close on its no-release path.
            const uint32_t zero = 0;
            std::memcpy(box + graphic + 0xA08, &zero, sizeof(zero));
        }

        // list_box_record samples through these; point them at the FIRST box
        // so the sampling stays in bounds.
        list_box_obj = object + offsets[0];
        list_box_graphic_disp = graphic;
        list_box_dialog_disp = dialog;
        list_box_event_count = 0;

        auto *const self = reinterpret_cast<ReportIf *>(object);
        if (use_adapter) {
            report_if_close_intel_redirect(self, nullptr);
        } else {
            self->close_intel();
        }

        // Exactly two dialog closes, in subobject order.
        const void *dialogs[2] = {nullptr, nullptr};
        int seen = 0;
        for (int i = 0; i < list_box_event_count &&
                 i < static_cast<int>(ARRAYSIZE(list_box_events)); ++i) {
            if (list_box_events[i].kind == 3 && seen < 2) {
                dialogs[seen++] = list_box_events[i].target;
            }
        }
        expect(seen == 2);
        expect(dialogs[0] == object + offsets[0] + dialog);
        expect(dialogs[1] == object + offsets[1] + dialog);
    }

    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_a0c;
    ListBoxOriginalDialogClose = saved_dialog;
    ListBoxCloseStaticDefaults = saved_static;
    ListBoxCloseDynamicDefault = saved_dynamic;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(13, test_win_is_visible);
LEAF_CASE(14, test_win_is_descendant);
LEAF_CASE(121, test_tut_win_rects);
LEAF_CASE(122, test_win_client_to_screen);
LEAF_CASE(15, test_sprite_construct);
LEAF_CASE(16, test_graphic_win_destructor);
LEAF_CASE(20, test_graphic_win_fill_color);
LEAF_CASE(21, test_graphic_win_redraw);
LEAF_CASE(18, test_graphic_win_init);
LEAF_CASE(19, test_base_button_init);
LEAF_CASE(17, test_graphic_win_close);
LEAF_CASE(22, test_base_button_and_flat_button_lifecycle);
LEAF_CASE(59, test_list_box_teardown);
LEAF_CASE(55, test_report_if_close_energy);
LEAF_CASE(54, test_report_if_close_intel);
}  // namespace
