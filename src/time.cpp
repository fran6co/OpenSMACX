#include "stdafx.h"

#include "time.h"
#include "mapwin.h"      // MapWinTable, go_reset's window walk
#include "console.h"     // ConsoleStatusWin
#include "statuswin.h"   // StatusWin::set_title, on the object above
#include "win.h"         // WinPointerOwner3/4
#include "tutwin.h"        // TutWinBaseWindow, TutWinSocWindow, TutorialWindow
#include "game.h"          // ExitTurnLoop, IsMultiplayerNet
#include "map.h"           // WorldClimateSkipTerrainClear
#include "planwin.h"       // PlanWin::on_redraw, MapWinTable
#include "multiwin.h"
#include "councwin.h"      // CouncilWindow, the turn redraw receiver

// The timer callbacks' shared state; declarations in time.h beside the
// callbacks themselves. All four are zero-initialised .data in the image.
// Original 0x0050EE4C/0x0050EE89/0x0050EF19 gate the timer callbacks on this shared value; start only.
// RECOVERY_OBJECT: v1 0x00915620 ?TimerHoldFlag@@3HA
int TimerHoldFlag;      // 0x00915620
// RECOVERY_OBJECT: v1 0x009392B4 ?Blink2Counter@@3HA
int Blink2Counter;      // 0x009392B4
// RECOVERY_OBJECT: v1 0x00939FD4 ?PlanLineCounter@@3HA
int PlanLineCounter;    // 0x00939FD4
// RECOVERY_OBJECT: v1 0x00703DE0 ?TurnRedrawPending@@3HA
int TurnRedrawPending;  // 0x00703DE0

// ORIGINAL: 0x0050EA40 ?blink_timer@@YAXH@Z 0x0050EA40-0x0050EE25 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:371deca46d2a09538c2887fcf5c729de7add0065:src/recovered/units/0050ea40.cpp: full original identity/spans and historical source-form findings", "Independent full seed/native semantic review; original three USER32 IAT targets resolve GetAsyncKeyState/GetFocus/GetWindowLongA with GWL_HINSTANCE.", "Win primary virtual slot12 is on_redraw; PlanWin::blink now uses genuine virtual dispatch, not manual vbase/table indexing.", "recovery/tasks/blink-timer-owner-oct5.json", "recovery/tests/test_blink_timer_actual_source.py"], "reason": "Complete timer callback through existing genuine tutorial/Plan/MainInterface/Console/MapWin owners and USER32 APIs. Preserve original guards, parity/wrap geometry, half-open bounds and cursor update ordering. Source family home is not asserted original TU ownership."}
void __cdecl blink_timer(int a1) {
    (void)a1;
    if (WorldClimateSkipTerrainClear != 0) return;
    if (TimerHoldFlag != 0) return;

    if (TutWinBaseWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        TutorialWindow.draw_arrow();
        return;
    }

    PlanWindow.blink();
    MainInterfaceGlobal.string_box_.clip_ids(0x96);

    const int t1 = ConsoleGlobal.field_23BE8_;
    const unsigned char game_state = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
    const int multiplayer = IsMultiplayerNet;
    if (t1 == 0 && (multiplayer == 0 || (game_state & 2) == 0)) {
        ConsoleGlobal.field_23BFC_ = 0;
        return;
    }
    if (ExitTurnLoop != 0) {
        ConsoleGlobal.field_23BFC_ = 0;
        return;
    }
    if (ConsoleGlobal.field_23BE4_ == 0
        && (multiplayer == 0 || (game_state & 2) == 0)) {
        return;
    }

    ConsoleGlobal.field_23BF8_ = ConsoleGlobal.field_23BF8_ == 0 ? 1 : 0;
    draw_cursor();

    const int blink_col = (GetAsyncKeyState(1) >> 8) & 0xFF;
    if (reinterpret_cast<HINSTANCE>(GetWindowLongA(GetFocus(), GWL_HINSTANCE))
        != WinInstance) {
        return;
    }
    if (TutorialWindow.is_visible() != 0) return;
    if (ConsoleGlobal.field_23D80_ == -1) return;
    if (ConsoleGlobal.field_23D84_ == -1) return;
    if (ConsoleGlobal.field_23D88_ == -1) return;
    if (ConsoleGlobal.field_23D8C_ == -1) return;

    const unsigned char preferences_high =
        static_cast<unsigned char>(GameDataStateStorage.fields_.game_preferences_ >> 8);
    if ((preferences_high & 0x10) == 0 && blink_col == 0
        && WinModalDepth == 0) {
        return;
    }

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == 0) continue;
        int col = blink_col;

        if (slot != 0 && window->view_active_ == 0) continue;
        if (col != 0 && window->field_1DD80_ == 0) continue;
        if (window->field_1DDD4_ + window->field_1DDCC_
            >= MapLongitudeBounds) {
            continue;
        }

        int row = window->center_y_;
        col = window->center_x_;

        int mouse_x;
        int mouse_y;
        window->get_mouse_pos(&mouse_x, &mouse_y);
        window->client_to_screen(&mouse_x, &mouse_y);

        const unsigned char flat = static_cast<unsigned char>(MapIsFlat);
        if (mouse_x == 0) {
            col -= (window->field_1DDCC_ + 2) / 3;
            if ((flat & 1) == 0) {
                if (col < 0) {
                    col += MapLongitudeBounds;
                } else {
                    const int width = MapLongitudeBounds;
                    if (col >= width) col -= width;
                }
            }
        }

        if (mouse_x >= WinScreenWidth - 0xA
            && mouse_x < WinScreenWidth + 0xA) {
            int next_col = (window->field_1DDCC_ + 2) / 3 + col;
            if ((flat & 1) != 0) {
                col = next_col;
            } else if (next_col < 0) {
                col = MapLongitudeBounds + next_col;
            } else {
                const int width = MapLongitudeBounds;
                if (next_col >= width) next_col -= width;
                col = next_col;
            }
        }

        if (mouse_y == 0 && window->field_1DDA8_ > -2) {
            const int max_shift = window->field_1DDA8_ + 2;
            const int requested = (window->field_1DDD0_ + 2) / 3;
            int delta;
            if (requested < 0 || max_shift < 0) {
                delta = 0;
            } else if (requested <= max_shift) {
                delta = requested;
            } else {
                delta = max_shift;
            }
            row -= delta;
        }

        if (mouse_y >= WinScreenHeight - 0xA
            && mouse_y < WinScreenHeight + 0xA) {
            const int limit = MapLatitudeBounds + 2;
            const int count = static_cast<int32_t>(window->field_1DDD8_);
            const int start = window->field_1DDA8_;
            if (start + count * 2 - 2 < limit) {
                const int max_shift = (2 - count) * 2 - start
                                    + MapLatitudeBounds;
                const int requested = (window->field_1DDD0_ + 2) / 3;
                int delta;
                if (requested < 0 || max_shift < 0) {
                    delta = 0;
                } else if (requested > max_shift) {
                    delta = max_shift;
                } else {
                    delta = requested;
                }
                row += delta;
            }
        }

        if ((col & 1) != 0) col -= 1;
        if ((row & 1) != 0) col += 1;

        if (col == window->center_x_
            && row == window->center_y_) {
            continue;
        }
        if (row < 0) continue;
        if (row >= MapLatitudeBounds) continue;
        if (col < 0) continue;
        if (col >= MapLongitudeBounds) continue;

        if (window == static_cast<MapWin *>(&ConsoleGlobal)) {
            window->field_21A44_ = 1;
        }
        window->set_center(col, row, 1);
        window->field_21A44_ = 0;
    }
}

// PlanWindow's virtual GraphicWin base begins at the timer's historical
// receiver 0x00856DC0. The real PlanWin object owns that base and lifetime.
// The MultiWin the turn timer redraws (`mov ecx, 0x7fd648`) is the REAL
// object MultiWindow in game.cpp; game.cpp closes its GraphicWin front for
// desktop_close - the same fixed address, one object.



// global

/*
Purpose: Cancel the go mode: hide the cover window, stop the go timer, then
         reset the mode state and cursor of every live map window and of the
         selected one.
// ORIGINAL: 0x0050EF50 ?go_reset@@YAXXZ 0x0050EF50-0x0050F166
// size      534 bytes
// prototype
// callers   7   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20 0x005EC7C0 0x005EDCD0 0x00616780
// indirect  0x0050EFF6 0x0050F014 0x0050F126 0x0050F144
// MEASURED 2026-08-29: /O2 best at 10/137 (0.842 similar). Image
//   instructions 0..7 agree - prologue, the cover window and
//   its direct `call Win::hide`. The counted divergence is the catalogued
//   MEASURED-inline ceiling: `GoTimer->close()` is inlined here in full
//   (stop()'s two kill branches plus the nine field clears, writing
//   0x939E60..0x939E84 directly) where the image makes one plain
//   `call 0x616780`. time.h keeps close() in-class BECAUSE ~Time must
//   inline it, so an out-of-line spelling would regress that claim; VC6
//   honours no per-site noinline. Everything after is that block's
//   register-allocation shadow (the walking cursor lands in edi instead of
//   esi), not a spelling defect this file can reach.
// TRIED: the dead cursor guard as a cached local (`cursor_sprite != 0`)
//   - same 10/137. The image loads the sprite value once for the two
//   equality compares (3D imm at 0x0050F097/0x0050F09E) but re-reads the
//   FIELD for the dead arm (39 at 0x0050F0A5), so the body re-reads it.
// MEASURED 2026-09-03, after the four start/pulse ladders landed: now
//   9/137 (0.864) at /O2. Re-derived the ceiling from scratch and it holds,
//   with one new fact - the image DOES inline other helpers into this body
//   (`go_cover_window()` folds to `mov ecx, 0x937118` at 0x0050EF57, and
//   `TutWinBaseWindow.is_visible()` expands inline in every caller), so
//   VC6's refusal here is specific to `Time::close`, not a general
//   no-inlining TU. The shape that explains all four sites - close()
//   inlined into ~Time, called out-of-line from stop_timers AND from here -
//   is that go_reset's original .cpp never saw close()'s body. This tree
//   merged the two units into src/time.cpp, so the body is always visible
//   and VC6 always expands it (+34 instructions, 155 against 137).
// TRIED: `#pragma inline_depth(0)` ahead of go_reset - it suppressed the
//   close() expansion all right but is a per-function switch, so it also
//   un-inlined go_cover_window() (the folded `mov ecx, 0x937118` became a
//   real call pair) and every is_visible() expansion, landing at 3/137.
//   Not a per-site lever, and not source. Reverted.
// TRIED: making close() an out-of-line member defined below go_reset in
//   this file - would hide the body from go_reset while leaving it visible
//   to ~Time, but it also hides it from console.cpp:875 and netmsg.cpp:77,
//   which are not mine to regress, and it does nothing for the prologue
//   below. Not taken.
//   The second, independent wall is the prologue: the image opens
//   `push ebp / mov ebp, esp / push ecx` (catalogue flag `frame`) and this
//   build's /O2 omits the frame pointer, so instructions 0..1 differ before
//   anything else. No flag set in the axis reaches both - the /Oy- sets fix
//   the prologue and lose the body (17/137 best).
Return Value: n/a
Status: Complete
*/
void __cdecl go_reset() {
    CoverWindow.Win::hide();
    GoTimer.close();

    for (int index = 0; index < static_cast<int>(MapWinTableSlots); ++index) {
        MapWin **const slot = &MapWinTable[index];
        MapWin *const window = *slot;
        if (window != nullptr &&
            (index == 0 || window->view_active_ != 0) &&
            index != MapWinSelectedSlot &&
            window->field_1DD80_ != 0) {
            // The FIRST store runs off the window the tests loaded; the rest
            // re-read the slot each time, as the image re-reads [esi] for
            // every store after the first (0x0050EFB6 on).
            window->field_1DD88_ = 1;
            (*slot)->field_1DD80_ = 0;
            (*slot)->field_1DD8C_ = 0;
            (*slot)->field_1DD84_ = 0;
            (*slot)->field_1DE0C_ = -1;

            // Capture-loss dispatch may replace the slot. Owner4 is still
            // compared with the old base, but the next dispatch reloads it
            // (50EFF9 versus 50F007 in the original).
            Win *const base = static_cast<GraphicWin *>(*slot);
            if (WinPointerOwner3 == base) {
                WinPointerOwner3 = nullptr;
                base->on_lose_mouse_capture();
            }
            if (WinPointerOwner4 == base) {
                WinPointerOwner4 = nullptr;
            }
            (*slot)->on_update();
            ConsoleStatusWin.set_title(0);
        }
    }

    // Slot 0's cursor first, unconditionally.
    MapWin *const primary = MapWinTable[0];
    if (primary->cursor_sprite_ != 0 || primary->cursor_resource_id_ != 0x7F00) {
        primary->GraphicWin::set_cursor(0x7F00);
        primary->cursor_resource_id_ = 0x7F00;
        primary->cursor_sprite_ = 0;
    }

    if (MapWinSelectedSlot >= 0) {
        // The slot walks two registers in the image - the window loaded once
        // for the cursor work, the slot's own address kept for the reset
        // stores below (`lea esi, [eax*4 + 0x7d3c3c]` at 0x0050F08A).
        MapWin *const window = MapWinTable[MapWinSelectedSlot];
        MapWin **const slot = &MapWinTable[MapWinSelectedSlot];
        // Only the two go-mode cursor sprites are replaced by the default.
        // The sprite value is loaded once for the two equality tests, but
        // the dead `!= 0` arm re-reads the FIELD from memory (0x0050F0A5
        // compares [edi + 0x1ef4c] again, not the register) - the same
        // re-read-the-lvalue shape as the reset stores below. Reproduced.
        Sprite *const cursor_sprite = window->cursor_sprite_;
        if ((cursor_sprite == &g_CURSOR_SPRITES[1] ||
             cursor_sprite == &g_CURSOR_SPRITES[9]) &&
            (window->cursor_sprite_ != 0 || window->cursor_resource_id_ != 0x7F00)) {
            window->GraphicWin::set_cursor(0x7F00);
            window->cursor_resource_id_ = 0x7F00;
            window->cursor_sprite_ = 0;
        }
        (*slot)->field_1DD88_ = 1;
        (*slot)->field_1DD80_ = 0;
        (*slot)->field_1DD8C_ = 0;
        (*slot)->field_1DD84_ = 0;
        (*slot)->field_1DE0C_ = -1;

        Win *const base = static_cast<GraphicWin *>(*slot);
        if (WinPointerOwner3 == base) {
            WinPointerOwner3 = nullptr;
            base->on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == base) {
            WinPointerOwner4 = nullptr;
        }
        (*slot)->on_update();
        ConsoleStatusWin.set_title(0);
        MapWinSelectedSlot = -1;
    }
}

/*
Purpose: Blink timer tick: skip while any window is up or the game holds the
         timers, then advance the blink counter and repaint the interface.
// ORIGINAL: 0x0050EE30 ?blink2_timer@@YAXH@Z 0x0050EE30-0x0050EE7D BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee30 archive unit. Five
//   sequential early-return `if`s (NOT one chained `&&`): the image jumps to a
//   SHARED `ret` epilogue from every test. The counter increment and the
//   MainInterface receiver load interleave in the image (`mov eax,[ctr]; mov
//   ecx,0x7ae820; inc; mov [ctr],eax`) exactly as the two-statement spelling
//   schedules, and the redraw_complete call is the last statement, which VC6
//   tail-jumps (`jmp 0x45c3a0`).
// size      77 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl blink2_timer(int a1) {
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (ExitTurnLoop != 0) {
        return;
    }
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    ++Blink2Counter;
    MainInterfaceGlobal.redraw_complete();
}

/*
Purpose: Plan-line timer tick: count the line counter down and repaint the
         plan lines on every live map window.
// ORIGINAL: 0x0050EE80 ?line_timer@@YAXH@Z 0x0050EE80-0x0050EF01 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ee80 archive unit. The slot
//   walk is a POINTER walk (`slot < &MapWinTable[8]`), not an index loop -
//   the image keeps the slot pointer in esi (`cmp esi, 0x7d3c5c; jl`), and
//   "slot == &MapWinTable[0]" is the primary-window exemption
//   (`cmp esi, 0x7d3c3c; je`), as in draw_tile. The PlanWin::on_redraw call
//   sits between the counter store and the loop with the receiver loaded as
//   an immediate; the two flag reads are plain (non-volatile) dwords off the
//   window pointer, which VC6 keeps in ecx across the guards.
// The pinned loop bounds 0x007D3C3C..0x007D3C5C and four-byte stride
//   identify eight ordered slots. A signed index preserves the live table
//   walk without converting its slot pointers to integers.
// size      129 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00467970 0x0048AF30 0x005F7E90
Return Value: n/a
Status: Complete
*/
void __cdecl line_timer(int a1) {
    if (WorldClimateSkipTerrainClear != 0) {
        return;
    }
    if (TimerHoldFlag != 0) {
        return;
    }
    if (TutWinBaseWindow.is_visible() != 0) {
        return;
    }
    if (TutWinSocWindow.is_visible() != 0) {
        return;
    }
    --PlanLineCounter;
    // The image makes this a DIRECT call (0x50EEBF -> 0x48AF30
    // ?on_redraw@PlanWin@@QAEXXZ), not a slot-12 dispatch; the qualified
    // spelling suppresses the virtual call the shared name now implies.
    PlanWindow.PlanWin::on_redraw();

    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active = window->view_active_;
            if (active == 0) {
                continue;
            }
        }
        const uint32_t draw_flags = window->field_1DD70_;
        if ((draw_flags & 0xC40000) != 0) {
            window->draw_base_dest(1);
        }
    }
}

/*
Purpose: Network turn timer tick: pulse the turn window while a multiplayer
         game is running and the timers are not held.
// ORIGINAL: 0x0050EF10 ?turn_timer@@YAXH@Z 0x0050EF10-0x0050EF42 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0050ef10 archive unit. Three
//   nested `if`s, not chained conditions - the image tests each in turn with
//   its own jump to the shared `ret`. MultiWindow.draw(1) is the
//   last statement, so VC6 tails it after loading the receiver immediate.
// size      50 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00479330 0x005D5A70
Return Value: n/a
Status: Complete
*/
void __cdecl turn_timer(int a1) {
    if (IsMultiplayerNet != 0) {
        if (TimerHoldFlag == 0) {
            if (TurnRedrawPending != 0) {
                CouncilWindow.GraphicWin::redraw();
            }
            MultiWindow.draw(1);
        }
    }
}

/*
Purpose: Start global timers.
// ORIGINAL: 0x0050F3D0 ?start_timers@@YAXXZ 0x0050F3D0-0x0050F431 BYTE_EXACT
// size      97 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616350
// LEVER (2026-08-29, timer conversion): the six timers are real `Time`
// objects (time.h/time.cpp) instead of `Time *const` bindings into their
// image addresses - the image's own receivers are the objects' ADDRESSES
// (`mov ecx, 0x915688` here), which `BlinkTimer.start(...)` on a real
// object reproduces; the claim re-measured BYTE_EXACT after, as did
// stop_timers, the three timer callbacks and game.cpp's inlined stop
// expansions.
Return Value: n/a
Status: Complete
*/
void __cdecl start_timers() {
    BlinkTimer.start(blink_timer, 1, 150, 150);
    Blink2Timer.start(blink2_timer, 2, 100, 100);
    LineTimer.start(line_timer, 3, 100, 100);
    TurnTimer.start(turn_timer, 4, 500, 500);
}

/*
Purpose: Stop global timers.
// ORIGINAL: 0x0050F440 ?stop_timers@@YAXXZ 0x0050F440-0x0050F45E BYTE_EXACT
// body      src/time.cpp
// size      30 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616780
Return Value: n/a
Status: Complete
*/
// Historical: an inline definition plus a third-TU forwarder once matched
// selected caller boundaries. The canonical implementation now lives here.
// LEVER: the `time_close_call` forwarder (declared in time.h, defined in
//   flatbutton.cpp). 0 of 6 agreeing, 90 instructions against the image's 6,
//   because close()'s in-class body was expanded at all three sites - the
//   image emits three `mov ecx, <addr>` + `call Time::close`, the last a tail
//   `jmp`. The body is in a header, so it is the same tokens in time.cpp and
//   in game.cpp, and the image wants the two OPPOSITE things from those two
//   units: `call` here, `expand` in system_init 0x58F040 and game_reload
//   0x58F450. No declaration of close() can give both - moving close()'s
//   definition out of the class into this .cpp (TRIED, see 0x00616780) left
//   this unit still able to see it and changed nothing here. A forwarder
//   whose body lives in a THIRD unit breaks the symmetry: this COMDAT emits
//   the real `call`, and the two game.cpp sites are unchanged by it (35 of
//   135 and 19 of 44 agreeing, before and after). Historical result: 6 of 6,
//   BYTE_EXACT.
void __cdecl stop_timers() {
    Blink2Timer.close();
    BlinkTimer.close();
    LineTimer.close();
    // missing MP TurnTimer. TODO: In future determine if timer should be closed on stop
}



// These are behavior views over observed fields, not names or complete
// allocation claims for either original class. No connection between their
// construction/use/lifetime is inferred from the nearby addresses.
class RangePrefixInitializationView {
public:
    RangePrefixInitializationView();
    unsigned int unknown_dword_0_;
    unsigned int zeroed_dword_4_;
    unsigned int unknown_dword_8_;
    unsigned int unknown_dword_C_;
};

// RECOVERY_ROLE: SCAFFOLD Provisional four-word initialization view; original containing class, complete extent and constructor-to-instance lifetime flow remain unproved. Matching the endpoint views' offsets is not a receiver ownership link.
// ORIGINAL: 0x004C8110 sub_4c8110 0x004C8110-0x004C813B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8110-0x004C813B: complete thiscall constructor first zeroes four bytes [this+4] through memset, then stores dword [this], dword [this+8], and dword [this+0xC] in this order; [this+4] is zeroed by the actual four-byte memset call at 0x004C811B to 0x006465F0", "git:aee589a5dd0fca997773070668af4e1ff9026684:src/recovered/004c8110.cpp: historical-only seed hint // symbol ??0LegacyBlock@@QAE@XZ; this deleted seed-file spelling is not emitted as the current symbol and is not independently established as the original class", "pinned scan: no direct caller edge or absolute-address data reference to 4C8110 was found"], "reason": "Cover only the constructor's concrete four-dword initialization through a typed source view. Preserve its original span and claim; keep field roles, complete original class name, allocation extent and runtime lifetime unknown. The view is not merged with the separate time-range receiver."}
// symbol    ??0RangePrefixInitializationView@@QAE@XZ
RangePrefixInitializationView::RangePrefixInitializationView() {
    memset(&zeroed_dword_4_, 0, sizeof(zeroed_dword_4_));
    unknown_dword_0_ = 0;
    unknown_dword_8_ = 0;
    unknown_dword_C_ = 0;
}

class TimeRangeReceiverView {
public:
    // Endpoint operations and sample share only the observed fields;
    // original class and lifetime ownership are not established.
    void set_lower_endpoint(int value);
    void set_upper_endpoint(int value);
    int sample();
private:
    unsigned char unobserved_prefix_0_[4];
    unsigned char enabled_flags_4_;
    unsigned char unobserved_bytes_5_7_[3];
    int lower_bound_8_;
    int upper_bound_C_;
};


// The paired operations update the two bounds in their original order. Their
// unsigned comparisons are pinned behavior, not a claim about endpoint meaning.
/*
 * RECOVERY_HISTORY: v1 address=0x004C80C0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=e0d6eb0a4d32e64334cc1f4cded43e7d701b510f32d7a1a14fb4d75f7ad5a11f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0xc against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 0xc] / cmp edx, eax / jae 0x4c80d0 / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80c0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80C0-0x004C80D7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_lower_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_lower_endpoint(int value) {
    if (static_cast<DWORD>(upper_bound_C_) < static_cast<DWORD>(value)) {
        upper_bound_C_ = value;
    }
    lower_bound_8_ = value;
}

/*
 * RECOVERY_HISTORY: v1 address=0x004C80E0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=8c15061494d8d9b690e02e41f5aa7054bafa4a713979ab8f6841a65deb438b95
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0x8 against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 8] / cmp edx, eax / jbe 0x4c80f0 / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80e0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80E0-0x004C80F7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_upper_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_upper_endpoint(int value) {
    if (static_cast<DWORD>(lower_bound_8_) > static_cast<DWORD>(value)) {
        lower_bound_8_ = value;
    }
    upper_bound_C_ = value;
}

// Historical TRIED: the seed candidate calculated range before the clock call;
// the compiler scheduled the call earlier, retaining both endpoints and using
// another callee-saved register. A volatile range instead spilled through a
// frame. Those were compiler-shape findings, not evidence for a source class.
// The original adds the unsigned remainder to the reloaded upper endpoint.
// memcpy retains x86 32-bit wrap without C++ signed-overflow undefined behavior.
static int range_view_wrapped_upper_add(int upper, DWORD remainder) {
    const DWORD bits = static_cast<DWORD>(upper) + remainder;
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

// Historical seed called this an unknown Subject::roll and kept an IAT alias;
// that is not pinned class/API evidence. The actual source uses the SDK clock.
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C8350 sub_4c8350 0x004C8350-0x004C8396 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8350-0x004C8396: thiscall reads byte[this+4]&1; signed-compares dwords+8/+C; forms a wrapped unsigned inclusive difference before timeGetTime; divides unsigned; reloads +C after the call and adds the remainder", "pinned calls at 0x004C836D and 0x004C8384 target PE import slot 0x00669368, which uniquely resolves to WINMM timeGetTime; the normal SDK API is used, not an executable-address alias", "promoted historical seed src/recovered/units/004c8350.cpp retained the complete70-byte span and MISMATCH state; prior range-before-clock source-shape observation is preserved here for later restoration", "pinned scan: no direct caller edge or absolute-address data reference to 4C8350 was found", "source:src/time.cpp provisional endpoint member operations at 0x004C80C0/0x004C80E0 update the same +8/+C fields read by sample"], "reason": "Implement the observed time-based range read through a provisional receiver view, paired with 0x004C80C0/0x004C80E0 because those endpoint operations write the same +8/+C fields this body reads. Keep the separate RangePrefixInitializationView at 0x004C8110 unmerged. Preserve signed branch comparison, unsigned 32-bit range wrap, zero-divisor failure, the post-clock upper-field reload, and wrapped return addition. This relation does not prove original complete class identity, object extent, constructor, callers, or lifetime."}
// symbol    ?sample@TimeRangeReceiverView@@QAEHXZ
int TimeRangeReceiverView::sample() {
    if ((enabled_flags_4_ & 1) == 0) {
        return 0;
    }

    const int lower = lower_bound_8_;
    const int upper = upper_bound_C_;
    if (lower > upper) {
        const DWORD range = static_cast<DWORD>(lower)
                          - static_cast<DWORD>(upper) + 1u;
        const DWORD tick = timeGetTime();
        const DWORD remainder = tick % range;
        return range_view_wrapped_upper_add(upper_bound_C_, remainder);
    }

    const DWORD range = static_cast<DWORD>(upper)
                      - static_cast<DWORD>(lower) + 1u;
    const DWORD tick = timeGetTime();
    const DWORD remainder = tick % range;
    return range_view_wrapped_upper_add(upper_bound_C_, remainder);
}

// Pump redraws before non-input work at least once. The original captures the
// WINMM timeGetTime import once and compares unsigned elapsed time, including
// wraparound and negative arguments interpreted as unsigned durations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050DD90 ?clock_wait@@YAXH@Z 0x0050DD90-0x0050DDBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0050dd90.cpp retained exact seed and original extent; preserved without copying historical claim", "Pinned PE import669368 resolves WINMM.dll timeGetTime; full disassembly50DD90..50DDBA captures import once, calls5FCBB0 then5FCA30 and uses unsigned jb elapsed compare"], "reason": "Home cooperative unsigned elapsed-time wait to timer integration; preserve import capture, redraw/non-input ordering, mandatory first iteration and modular DWORD subtraction, without fixed import address."}
void __cdecl clock_wait(int milliseconds) {
    DWORD (WINAPI *const read_clock)() = timeGetTime;
    const DWORD started = read_clock();
    do {
        do_all_draws();
        do_non_input();
    } while (read_clock() - started < static_cast<DWORD>(milliseconds));
}
