/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "original_seam.h"
#include "tutwin.h"

// The tutorial overlay used by InfoWin and MapWin; a real process-lifetime
// object rather than a pointer into the original executable's BSS.
// RECOVERY_OBJECT: v2 0x008C6E68 0x008CC240 ?TutorialWindow@@3VTutWin@@A 0x00458420
TutWin TutorialWindow;
#include "win.h"
#include "mapwin.h"
#include "temp.h"     // do_all_draws
#include "game.h"     // IsMultiplayerNet, GamePreferences - tut_check's gates
#include "faction.h"  // Players/LocalFaction/RFLAG_ALIEN - the expansion gate
#include "console.h"  // ConsoleGlobal.faction_id_ supplies the active faction
#include "veh.h"      // the recovered vehicle painter
#include "strings.h"  // StringTemp and the tutorial's decimal label suffixes
#include "map.h"
#include "base.h"
#include "general.h"
#include "technology.h"
#include "worldwin.h"
#include "multiwin.h"
#include "random.h"
#include "text.h"
#include "alpha.h"      // Labels and the typed tutorial-label table


// ORIGINAL: 0x004BDF20 ?terrain_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDF20-0x004BDF66 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BDF20-0x004BDF66; seed:src/recovered/004bdf20.cpp; source:src/mapwin.h:gen_terrain_poly; source:src/tutwin.h:TutorialWindow"], "reason": "The draw callback takes no receiver and reads the singleton tutorial tile state while invoking the typed terrain renderer."}
// symbol    ?tut_terrain_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl tut_terrain_draw(Sprite *, GraphicWin *window, int,
                              int pixel_x, int pixel_y) {
    Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
    gen_terrain_poly(canvas, 0, 0, ConsoleGlobal.faction_id_, -1, 0x621,
                     TutorialWindow.field_537C_, TutorialWindow.field_5380_,
                     pixel_x, pixel_y);
}

// ORIGINAL: 0x004BDF70 ?veh_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDF70-0x004BDFA0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BDF70-0x004BDFA0; seed:src/recovered/004bdf70.cpp; source:src/veh.h:veh_draw; source:src/graphicwin.h:Buffer base"], "reason": "The tutorial vehicle draw callback has no receiver and forwards a typed GraphicWin canvas to the recovered renderer."}
// symbol    ?tut_veh_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl tut_veh_draw(Sprite *, GraphicWin *window, int vehicle,
                          int pixel_x, int pixel_y) {
    Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
    veh_draw(canvas, vehicle, pixel_x, pixel_y, 0, 0, 0);
}

// Reset at 0x004BDDE0 clears this process-wide marker after the tutorial's
// own state; it is separate from TutorialWindow, whose extent ends at 8CC240.
// RECOVERY_OBJECT: v2 0x008CC244 0x008CC248 ?TutWinShownFlag@@3IA 0x004BDDE0
uint32_t TutWinShownFlag;
// The adjacent process flags are independently owned by the tutorial
// window. Their fixed-image identities are recorded by the coordinator.
// RECOVERY_OBJECT: v1 0x008CC248 ?TutWinTourPending@@3HA
int TutWinTourPending;
// RECOVERY_OBJECT: v1 0x009A64BC ?TutWinFlags9A64BC@@3HA
int TutWinFlags9A64BC;
// Tutorial tour's original 0x945830 gate controls WorldClimateWorldWin's
// restoration. It is source-owned, writable state with loader-zero initial
// storage; no writer or larger object owner is established.
// RECOVERY_OBJECT: v1 0x00945830 ?TutWinClimateGate945830@@3HA
int TutWinClimateGate945830;

// THE TWO TUTORIAL-SHOWN BIT SETS, one per checker. tut_check and tut_check2
// each OR their argument in and refuse to fire again while the bit is set;
// the split into two words is what makes them independent hint sequences.
// Both must be real globals (not const-address bindings) or VC6 caches the
// word across the table read and the `|=` degrades to a load/or/store.
int TutorialFlags;   // 0x009A64AC
int TutorialFlags2;  // 0x009A64B0

/*
Purpose: Compute a rectangle center with wrapping subtraction and truncation toward zero.
// ORIGINAL: 0x004BA830 ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z 0x004BA830-0x004BA863 BYTE_EXACT
// LEVER: MORE (2 calls vs image's 0: midpoint/int_from_bits helpers never inline) - the image writes the signed halving directly (`sub;cdq;sub;sar;add`), not through any helper, and the tail is `ret 0xc` (callee-cleans), not `ret` - the ORIGINAL is a thiscall member ignoring `this` entirely, and this free-function stand-in needs `__stdcall` (not `__cdecl`) to match that stack cleanup. Rewriting the body as plain field arithmetic and switching the calling convention took 0/25 (0.553 best) to 24/25 (0.960 best, /Oy- flag set) with 0 calls disagreeing.
// symbol    ?rect_center@@YGHPAUtagRECT@@PAH1@Z
// size      51 bytes
// prototype void (__thiscall ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z)(TutWin* this, RECT*, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int __stdcall rect_center(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    int center_y = rect->top + (rect->bottom - rect->top) / 2;
    *y = center_y;
    return center_y;
}

/*
Purpose: Reset the tutorial window - four fields to zero, four to -1, and the
         shared shown-marker cleared with them.
// ORIGINAL: 0x004BA720 ?UNK1@TutWin@@QAEXXZ 0x004BA720-0x004BA761 BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK1@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK1() {
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

/*
Purpose: Store a single value at 0x53D4.
// ORIGINAL: 0x004BDDD0 ?UNK3@TutWin@@QAEXH@Z 0x004BDDD0-0x004BDDE0 BYTE_EXACT
// symbol    ?UNK3@TutWin@@QAEXPAVWin@@@Z
// size      16 bytes
// prototype void (__thiscall ?UNK3@TutWin@@QAEXH@Z)(TutWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK3(Win *a1) {
    field_53D4_ = a1;
}

// The active path clears the arrow state before virtual hide. Hide may
// change the owner, so the owner's update dispatch must reload field_53D4_.
// Drawing completes before either the owner or the shared marker is reset.
// ORIGINAL: 0x004BDDE0 ?reset@TutWin@@QAEHXZ 0x004BDDE0-0x004BDE97 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4BDDEE..4BDE07 clears53A4, calls this slot2, reloads owner53D4 for slot63 and then calls do_all_draws5FCBB0. The initial owner test selects the return value; callbacks precede all final reset stores.", "Both original tails clear owner53D4 and53A4, store minus one to5380/537C/539C, zero53A8, minus one53AC, zero53B8/53C4 and finally real TutWinShownFlag8CC244. Existing TutorialWindow and typed Win owner are preserved; no fixed-address aliases or VCall shim are required."], "reason": "Recover tutorial dismissal and owner notification through real members and virtual Win methods, retaining its original reset ordering and result."}
int TutWin::reset() {
    if (field_53D4_ != 0) {
        field_53A4_ = 0;
        hide();
        field_53D4_->on_update();
        do_all_draws();
        field_53D4_ = 0;
        field_53A4_ = 0;
        field_5380_ = -1;
        field_537C_ = -1;
        field_539C_ = -1;
        field_53A8_ = 0;
        field_53AC_ = -1;
        field_53B8_ = 0;
        field_53C4_ = 0;
        TutWinShownFlag = 0;
        return 1;
    }
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
    return 0;
}

// ORIGINAL: 0x004BDCC0 ?on_button_clicked@TutWin@@QAEXH@Z 0x004BDCC0-0x004BDDA2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bdcc0.cpp", "image:0x004bdcc0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?on_button_clicked@TutWin@@UAEXH@Z
void TutWin::on_button_clicked(int button_id) {
    const uint32_t tour_was_shown = TutWinShownFlag;

    if (field_53B8_ != 0) {
        release_modal();
    }
    if (field_53D4_ != 0) {
        field_53A4_ = 0;
        hide();
        field_53D4_->on_update();
        do_all_draws();
    }

    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;

    field_53BC_ = button_id < 0 ? button_id + 1 : button_id;
    if (button_id < -1) {
        field_53B4_ = 0;
    }
    if (button_id <= 0) {
        return;
    }

    if (field_53B4_ == 0) {
        if (tour_was_shown != 0) {
            TutWinTourPending = 1;
        }
        if (field_53B4_ == 0) {
            goto finish_button;
        }
    }
    field_53B4_ = 0;

finish_button:
    if (field_53D0_ != 0) {
        TutWinFlags9A64BC |= field_53D0_;
    }
}

// ORIGINAL: 0x004BE650 ?tut_map@TutWin@@QAEHPBDHHHPAUSprite@@HHH@Z 0x004BE650-0x004BE7C6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004be650.cpp", "image:0x004be650; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?tut_map@TutWin@@QAEHPBDHHHPAVSprite@@HHH@Z
int TutWin::tut_map(const char *text, int x_coord, int y_coord, int pixel_x,
                    Sprite *sprite, int forwarded_6, int arrow_x,
                    int arrow_y) {
    int arrow_pixel_x = -1;
    int arrow_pixel_y = -1;
    const int original_y_coord = y_coord;
    int tutorial_pixel_x = pixel_x;
    int tutorial_pixel_y = forwarded_6;

    if (forwarded_6 < 0) {
        if (sprite == 0) {
            field_537C_ = x_coord;
            field_5380_ = y_coord;
        }
    } else {
        field_53AC_ = forwarded_6;
    }

    MapWin *const map_window = MapWinTable[0];
    map_window->focus(x_coord, y_coord);
    map_window->tile_to_pixel(x_coord, original_y_coord, &tutorial_pixel_x,
                              &tutorial_pixel_y);
    tutorial_pixel_x += map_window->field_1DDBC_;
    tutorial_pixel_y += map_window->field_1DDC0_;

    if (arrow_x >= 0 &&
        arrow_y >= map_window->field_1DDA8_ &&
        arrow_y < map_window->field_1DDD8_ + map_window->field_1DDD0_ +
                      map_window->field_1DDA8_) {
        int adjusted_arrow_x = arrow_x;
        if ((MapIsFlat & 1) == 0) {
            if (adjusted_arrow_x < map_window->field_1DDA4_) {
                adjusted_arrow_x += MapLongitudeBounds;
            }
            if (map_window->field_1DDD4_ + map_window->field_1DDCC_ +
                    map_window->field_1DDA4_ <= adjusted_arrow_x) {
                adjusted_arrow_x -= MapLongitudeBounds;
            }
        }
        if (map_window->field_1DDA4_ <= adjusted_arrow_x &&
            adjusted_arrow_x < map_window->field_1DDD4_ +
                                  map_window->field_1DDCC_ +
                                  map_window->field_1DDA4_) {
            map_window->tile_to_pixel(arrow_x, arrow_y, &arrow_pixel_x,
                                      &arrow_pixel_y);
            arrow_pixel_x += map_window->field_1DDBC_;
            arrow_pixel_y += map_window->field_1DDC0_;
        }
    }

    GraphicWin *const owner = static_cast<GraphicWin *>(map_window);
    return tut_win(owner, text, tutorial_pixel_x, tutorial_pixel_y, sprite,
                   forwarded_6,
                   arrow_pixel_x, arrow_pixel_y);
}

// The old callout helpers all transform a panel rectangle into screen
// coordinates, then host the popup in MapWin's virtual GraphicWin base.
// Keeping the transformation receiver explicit preserves which window owns
// each rectangle while using the declared virtual-base conversion.
static int tut_callout(TutWin &tutorial, Win &coordinates, RECT &rect,
                       const char *label) {
    int x = rect.left + (rect.right - rect.left) / 2;
    int y = rect.top + (rect.bottom - rect.top) / 2;
    coordinates.client_to_screen(&x, &y);
    MapWin *const map = MapWinTable[0];
    GraphicWin *const owner = map != 0 ? static_cast<GraphicWin *>(map) : 0;
    return tutorial.tut_win(owner, label, x, y, 0, 2, -1, -1);
}

static int tutorial_button_label_index(unsigned int byte_offset) {
    return reinterpret_cast<const int *>(Labels.strings_ptr)[byte_offset / 4];
}

// ORIGINAL: 0x004BDFE0 ?tut_win@TutWin@@QAEHPAUGraphicWin@@PBDHHPAUSprite@@HHH@Z 0x004BDFE0-0x004BE643;0x00659CE0-0x00659CF2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bdfe0.cpp", "image:0x004bdfe0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?tut_win@TutWin@@QAEHPAVGraphicWin@@PBDHHPAVSprite@@HHH@Z
int TutWin::tut_win(GraphicWin *owner, const char *text, int x, int y,
                    Sprite *picture, int flags, int arrow_x, int arrow_y) {
    Sprite local_sprite;
    const int original_x = x;
    const int original_y = y;
    int mode;
    int normalized_mode;
    int screen_width;
    int screen_height;
    int placement_x;
    int placement_y;
    int width_eighth;
    int height_eighth;
    int horizontal_anchor;
    int vertical_anchor;

    field_53D0_ = flags & 0xFFFF0000;
    normalized_mode = flags & 0xFFFF;
    field_53C4_ = 0;
    mode = normalized_mode & 0xFF;
    const int popup_flags = mode == 0 ? 0x4000 : (mode == 1 ? 0 : 0x40);
    field_5384_ = x;
    field_5388_ = y;
    screen_width = WinScreenWidth;
    screen_height = WinScreenHeight;

    popups_tutorial();
    reset();
    field_53D4_ = owner;
    if ((normalized_mode & 0xFF00) != 0) {
        TutWinShownFlag = 1;
        normalized_mode = mode;
    }

    start(const_cast<char *>("EDITOR"), text, -1, 0, popup_flags, 0);
    if (mode < 2) {
        set_ok_text(StringTable.get(
            tutorial_button_label_index(0x36C)));
    } else {
        set_ok_text(StringTable.get(
            tutorial_button_label_index(0x3B0)));
        set_cancel_text(StringTable.get(
            tutorial_button_label_index(0x3AC)));
    }

    if (picture != 0) {
        set_caller_picture(picture_enabled() ? 0 : picture);
        dialogs_.SpriteBox::sprite(picture, 0, 0);
    } else if (field_53AC_ >= 0) {
        set_caller_picture(picture_enabled() ? 0 : &g_VEH_SPRITES[0]);
        dialogs_.SpriteBox::sprite(&g_VEH_SPRITES[0], 0, 0);
        set_redraw_callback(tut_veh_draw);
    } else if (field_53AC_ >= -1) {
        set_caller_picture(picture_enabled() ? 0 : &g_VEH_SPRITES[0]);
        dialogs_.SpriteBox::sprite(&g_VEH_SPRITES[0], 0, 0);
        set_redraw_callback(TutWin::base_draw);
    } else if (field_537C_ >= 0) {
        set_caller_picture(picture_enabled() ? 0 : &g_TER1_TUT_BLANK_SPRITE);
        dialogs_.SpriteBox::sprite(&g_TER1_TUT_BLANK_SPRITE, 0, 0);
        set_redraw_callback(tut_terrain_draw);
    }

    exec(1, 0);

    vertical_anchor = (screen_height * 5 +
                       (screen_height * 5 < 0 ? 7 : 0)) >> 3;
    horizontal_anchor = (screen_width -
                         (screen_width < 0 ? 7 : 0)) >> 1;
    height_eighth = (screen_height + (screen_height < 0 ? 7 : 0)) >> 3;
    width_eighth = (screen_width + (screen_width < 0 ? 7 : 0)) >> 3;
    if (mode < 2 || mode == 2 || mode == 5) {
        horizontal_anchor += width_eighth;
    }
    if (mode == 3 || mode == 4) {
        horizontal_anchor -= width_eighth;
    }
    if (mode == 4 || mode == 5) {
        vertical_anchor -= height_eighth;
    }

    if (y > vertical_anchor) {
        placement_y = screen_height - static_cast<int>(field_30C0_) - 8;
        y = placement_y;
        field_5390_ = placement_y;
    } else {
        placement_y = (static_cast<int>(field_5388_) +
                       (static_cast<int>(field_5388_) < 0 ? 7 : 0)) >> 3;
        y = placement_y;
        field_5390_ = placement_y + field_30C0_;
    }

    if (x > horizontal_anchor) {
        placement_x = (static_cast<int>(field_30BC_) +
                       (static_cast<int>(field_30BC_) < 0 ? -1 : 0)) >> 1;
        field_538C_ = placement_x + 5;
        x = 5;
    } else {
        const int dimension = static_cast<int>(field_30BC_);
        const int half = (dimension - (dimension < 0 ? -1 : 0)) >> 1;
        field_538C_ = screen_width - half - 5;
        x = screen_width - dimension - 5;
    }

    if (arrow_x >= 0) {
        field_5394_ = arrow_x;
        field_5398_ = arrow_y;
        field_539C_ = field_538C_;
        field_53A0_ = field_5390_;
    }
    field_53B8_ = normalized_mode;
    field_53A4_ = 1;
    field_53D4_->window_line_raw(field_538C_, field_5390_, field_5384_,
                                  field_5388_, PopupThemeWindowColor, 2, 2);
    if (field_539C_ >= 0) {
        field_53D4_->window_line_raw(field_539C_, field_53A0_, field_5394_,
                                      field_5398_, PopupThemeWindowColor, 2, 2);
    }

    start(const_cast<char *>("EDITOR"), text, -1, 0, popup_flags, 0);
    if (normalized_mode == 0) {
        set_ok_text(StringTable.get(
            tutorial_button_label_index(0x36C)));
    } else {
        set_ok_text(StringTable.get(
            tutorial_button_label_index(0x3B0)));
        set_cancel_text(StringTable.get(
            tutorial_button_label_index(0x3AC)));
    }
    if (normalized_mode != 6) {
        set_loc(x, y);
    }

    if (picture != 0) {
        set_caller_picture(picture_enabled() ? 0 : picture);
        dialogs_.SpriteBox::sprite(picture, 0, 0);
    } else if (field_53AC_ >= 0) {
        set_caller_picture(picture_enabled() ? 0 : &local_sprite);
        dialogs_.SpriteBox::sprite(&local_sprite, 0,
                                   static_cast<int>(field_53AC_));
        set_redraw_callback(tut_veh_draw);
    } else if (field_53AC_ >= -1) {
        set_caller_picture(picture_enabled() ? 0 : &local_sprite);
        dialogs_.SpriteBox::sprite(&local_sprite, 0,
                                   static_cast<int>(field_53AC_));
        set_redraw_callback(TutWin::base_draw);
    } else if (field_537C_ >= 0) {
        set_caller_picture(picture_enabled() ? 0 : &local_sprite);
        dialogs_.SpriteBox::sprite(&local_sprite, 0, 0);
        set_redraw_callback(tut_terrain_draw);
    }

    exec(0, 0);
    popups_normal();
    field_53A8_ = 1;
    if (normalized_mode == 1) {
        const int run_tour = field_53BC_ != 0;
        const int send_key = field_53C4_ != 0;
        const int key = static_cast<int>(field_53C8_);
        const int key_state = static_cast<int>(field_53CC_);
        if (run_tour) {
            tour();
        }
        if (send_key) {
            ConsoleGlobal.on_key_click(key, key_state);
        }
    }

    return static_cast<int>(field_53B4_);
}

// ORIGINAL: 0x004BC6E0 ?tour@TutWin@@QAEXXZ 0x004BC6E0-0x004BDCBE FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bc6e0.cpp", "image:0x004bc6e0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour() {
    int tour_step = static_cast<int>(field_53C0_);
    int offsets[32];
    int positions[24];
    int scores[32];
    int center_x;
    int center_y;
    int cursor_x;
    int cursor_y;
    int neighbor_x;
    int neighbor_y;
    int ring;
    int index;
    int direction;
    int random_start;
    int flat_map;
    int best_direction_score;
    int best_direction;
    int best_river_x;
    int best_river_y;
    int best_rain_x;
    int best_rain_y;
    int best_forest_x;
    int best_forest_y;
    int best_rock_x;
    int best_rock_y;
    int best_altitude_x;
    int best_altitude_y;
    int best_fungus_x;
    int best_fungus_y;
    int best_bonus_x;
    int best_bonus_y;
    int best_pod_x;
    int best_pod_y;
    int best_site_x;
    int best_site_y;
    int best_target_x;
    int best_target_y;
    int score;
    int score2;
    int score3;
    int saved_score;
    int elevation_score;
    int moisture_score;
    int fungus_score;
    int fungus_yield;
    int terrain_kind;
    int terrain_value;
    int visibility_override;
    int local_player;
    int selected_vehicle;
    int selected_proto;
    int technology_type;
    int current_cursor;
    int screen_x;
    int screen_y;
    MapWin *map_window;
    Map *tile;
    RECT rect;
    int result;

    if (field_53B4_ != 0) {
        return;
    }
    field_53B4_ = 1;

    if (tour_step == 1) {
        tour1();
        goto tour_cleanup;
    }
    if (tour_step == 5) {
        tour5();
        goto tour_cleanup;
    }
    if (tour_step == 6) {
        tour6();
        goto tour_cleanup;
    }
    if (tour_step == 7) {
        tour7();
        goto tour_cleanup;
    }

    WorldClimateWorldWin.hide();
    current_cursor = ConsoleGlobal.cursor_history_index_;
    map_window = MapWinTable[0];
    cursor_x = ConsoleGlobal.cursor_history_x_[current_cursor];
    cursor_y = ConsoleGlobal.cursor_history_y_[current_cursor];
    map_window->set_center(cursor_x, cursor_y, 1);

    best_direction_score = 0;
    best_direction = 0;
    random_start = random(0, 8);
    flat_map = MapIsFlat & 1;
    for (direction = 0; direction < 8; ++direction) {
        int candidate_direction = (direction + random_start) & 7;
        neighbor_x = cursor_x + VehMoveDeltaX[candidate_direction];
        if (!flat_map) {
            if (neighbor_x < 0) {
                neighbor_x += MapLongitudeBounds;
            } else if (neighbor_x >= MapLongitudeBounds) {
                neighbor_x -= MapLongitudeBounds;
            }
        }
        neighbor_y = cursor_y + VehMoveDeltaY[candidate_direction];
        if (neighbor_y >= 0 && neighbor_y < MapLatitudeBounds &&
            neighbor_x >= 0 && neighbor_x < MapLongitudeBounds) {
            tile = MapTiles + static_cast<int>(MapLongitude) * neighbor_y +
                   (neighbor_x >> 1);
            terrain_value = tile->climate & 0xE0;
            if ((terrain_value < 0x61 ||
                 static_cast<int>(map_window->field_1DDA8_ + 2) <=
                     (neighbor_y - (tile->climate >> 5)) + 3) &&
                static_cast<int>(map_window->field_1DDA8_ + 2) <= neighbor_y &&
                neighbor_y < static_cast<int>(map_window->field_1DDD8_ +
                                               map_window->field_1DDD0_ - 2 +
                                               map_window->field_1DDA8_)) {
                int wrapped_x = neighbor_x;
                if (!flat_map) {
                    if (wrapped_x < map_window->field_1DDA4_ + 2) {
                        wrapped_x += MapLongitudeBounds;
                    }
                    if (wrapped_x >= map_window->field_1DDD4_ +
                                         map_window->field_1DDCC_ - 2 +
                                         map_window->field_1DDA4_) {
                        wrapped_x -= MapLongitudeBounds;
                    }
                }
                if (wrapped_x >= map_window->field_1DDA4_ + 2 &&
                    wrapped_x < map_window->field_1DDD4_ +
                                   map_window->field_1DDCC_ - 2 +
                                   map_window->field_1DDA4_ &&
                    terrain_value > 0x5F) {
                    score = 8;
                    if ((tile->bit & 0x20) != 0 && terrain_value > 0x3F) {
                        score = 4;
                    }
                    if (VehMoveDeltaX[candidate_direction] == 0 ||
                        VehMoveDeltaY[candidate_direction] == 0) {
                        ++score;
                    }
                    if (best_direction_score <= score) {
                        best_direction_score = score;
                        best_direction = candidate_direction;
                    }
                }
            }
        }
    }

    if (tour_step == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "TOUR1");
        selected_vehicle = ConsoleGlobal.selected_vehicle_;
        if (ConsoleGlobal.cursor_active_ == 0 || selected_vehicle < 0) {
            strcat(StringTemp, "B");
        } else {
            selected_proto = Vehs[selected_vehicle].proto_id;
            technology_type = VehPrototypes[selected_proto].preq_tech;
            if (technology_type == 2) {
                parse_says(0, VehPrototypes[2].veh_name, -1, -1);
            } else {
                parse_says(0, VehPrototypes[selected_proto].veh_name, -1, -1);
                strcat(StringTemp, "A");
            }
        }
        index = ConsoleGlobal.cursor_history_index_;
        result = tut_map(StringTemp,
                         ConsoleGlobal.cursor_history_x_[index],
                         ConsoleGlobal.cursor_history_y_[index],
                         ConsoleGlobal.selected_vehicle_, 0, 2, -1, -1);
        if (result == 0) {
            goto tour_cleanup;
        }
        map_window->field_1DD70_ |= 0x20000;
        map_window->draw_map(1);

        index = best_direction;
        parse_num(0, MapDirectionPermutation[index]);
        if (text_open(TextBufferGetPtr, "DIRECTIONS") == 0) {
            for (int text_index = 0; text_index <= index; ++text_index) {
                text_get();
            }
            text_close();
            parse_says(0, TextBufferGetPtr, -1, -1);
        }
        index = ConsoleGlobal.cursor_history_index_;
        result = tut_map("TOUR2", ConsoleGlobal.cursor_history_x_[index],
                         ConsoleGlobal.cursor_history_y_[index],
                         ConsoleGlobal.selected_vehicle_, 0, 2, -1, -1);
        if (result == 0) {
            map_window->field_1DD70_ &= ~0x20000;
            map_window->draw_map(1);
            goto tour_cleanup;
        }
    } else if (tour_step == 3 && ConsoleGlobal.selected_vehicle_ >= 0 &&
               Vehs[ConsoleGlobal.selected_vehicle_].proto_id == 1) {
        result = tut_map("TERRAFORMER", cursor_x, cursor_y,
                         ConsoleGlobal.selected_vehicle_, 0, 2, -1, -1);
        if (result == 0) {
            goto tour_cleanup;
        }
    }

    map_window->field_1DD70_ &= ~0x20000;
    map_window->draw_map(1);

    for (index = 0; index < 32; ++index) scores[index] = -1;
    for (index = 0; index < 24; ++index) positions[index] = -1;
    scores[0x19] = scores[7] = scores[0x1E] = 3;
    best_direction_score = -1;
    best_river_x = best_river_y = -1;
    best_rain_x = best_rain_y = -1;
    best_forest_x = best_forest_y = -1;
    best_rock_x = best_rock_y = -1;
    best_altitude_x = best_altitude_y = -1;
    best_fungus_x = best_fungus_y = -1;
    best_bonus_x = best_bonus_y = -1;
    best_pod_x = best_pod_y = -1;
    best_site_x = best_site_y = -1;
    best_target_x = best_target_y = -1;
    scores[10] = scores[0x0D] = scores[0x11] = -1;
    scores[0x10] = scores[0x12] = -1;
    scores[0x0C] = scores[0x14] = scores[0x1B] = -1;
    scores[0x16] = scores[0x18] = scores[0x0B] = -1;
    scores[9] = scores[0x1A] = scores[0x1C] = scores[0x1D] = -1;
    scores[6] = scores[8] = scores[0x0F] = scores[0x17] = -1;
    scores[0x13] = scores[0x15] = scores[0x0E] = -1;
    scores[0] = scores[1] = scores[2] = scores[3] = -1;
    scores[4] = scores[5] = scores[0x1F] = -1;

    local_player = ConsoleGlobal.faction_id_;
    visibility_override = (GameState & 0x80) != 0;
    for (ring = 0; ring < 2; ++ring) {
        for (index = 1; index < 0x15; ++index) {
            neighbor_x = cursor_x + RadiusOffsetX[index];
            flat_map = MapIsFlat & 1;
            if (!flat_map) {
                if (neighbor_x < 0) neighbor_x += MapLongitudeBounds;
                else if (neighbor_x >= MapLongitudeBounds) neighbor_x -= MapLongitudeBounds;
            }
            neighbor_y = cursor_y + RadiusOffsetY[index];
            if (neighbor_y < 0 || neighbor_y >= MapLatitudeBounds || neighbor_x < 0 ||
                neighbor_x >= MapLongitudeBounds) continue;

            tile = MapTiles + static_cast<int>(MapLongitude) * neighbor_y + (neighbor_x >> 1);
            terrain_value = tile->climate & 0xE0;
            if ((terrain_value >= 0x61 && map_window->field_1DDA8_ + 2 <=
                     (neighbor_y - (tile->climate >> 5)) + 3) ||
                map_window->field_1DDA8_ + 2 > neighbor_y ||
                neighbor_y >= map_window->field_1DDD8_ + map_window->field_1DDD0_ - 2 +
                                  map_window->field_1DDA8_) continue;

            int visible_x = neighbor_x;
            if (!flat_map) {
                if (visible_x < map_window->field_1DDA4_ + 2) visible_x += MapLongitudeBounds;
                if (visible_x >= map_window->field_1DDD4_ + map_window->field_1DDCC_ - 2 +
                                     map_window->field_1DDA4_) visible_x -= MapLongitudeBounds;
            }
            if (visible_x < map_window->field_1DDA4_ + 2 ||
                visible_x >= map_window->field_1DDD4_ + map_window->field_1DDCC_ - 2 +
                                 map_window->field_1DDA4_ ||
                (((PlayersData[local_player].flags & 0x200) == 0) &&
                 (tile->visibility & (1 << (local_player & 0x1F))) == 0 &&
                 !visibility_override)) continue;

            if (terrain_value < 0x60) {
                score = 2;
                if ((tile->bit & 0x20) != 0 && terrain_value < 0x40) score = 1;
                if (scores[0x0D] < score) {
                    scores[10] = neighbor_x;
                    scores[0x0D] = score;
                    scores[0x11] = neighbor_y;
                }
                continue;
            }
            if ((tile->bit & 0x20) != 0 && terrain_value >= 0x40) {
                if (best_target_y < 0 || best_target_y >= MapLatitudeBounds ||
                    best_target_x < 0 || best_target_x >= MapLongitudeBounds ||
                    (tile->val3 & 0xC0) < 0x80) {
                    best_target_x = neighbor_x;
                    best_target_y = neighbor_y;
                }
                continue;
            }

            score = goody_at(neighbor_x, neighbor_y);
            if (score == 0) score = bonus_at(neighbor_x, neighbor_y, 0);
            saved_score = (score != 0 || (tile->bit & 0x2000) != 0) ? 1 : 0;
            fungus_yield = crop_yield(local_player, -1, neighbor_x, neighbor_y, 0);
            tile = MapTiles + static_cast<int>(MapLongitude) * neighbor_y + (neighbor_x >> 1);
            const unsigned int tile_bits = tile->bit;
            const int elevation = (tile_bits & 0x80) != 0;
            const int fungus = ((tile_bits & 0x20) != 0 && (tile->climate & 0xE0) < 0x40);
            int north_higher = 0;
            int north_lower = 0;
            for (direction = 0; direction < 8; ++direction) {
                int adjacent_x = neighbor_x + VehMoveDeltaX[direction];
                if (!flat_map) {
                    if (adjacent_x < 0) adjacent_x += MapLongitudeBounds;
                    else if (adjacent_x >= MapLongitudeBounds) adjacent_x -= MapLongitudeBounds;
                }
                int adjacent_y = neighbor_y + VehMoveDeltaY[direction];
                if (adjacent_y < 0 || adjacent_y >= MapLatitudeBounds || adjacent_x < 0 ||
                    adjacent_x >= MapLongitudeBounds) continue;
                const unsigned int adjacent_climate =
                    (MapTiles + static_cast<int>(MapLongitude) * adjacent_y +
                     (adjacent_x >> 1))->climate & 0xE0;
                if (adjacent_climate > 0x5F) {
                    if ((tile->climate & 0xE0) < adjacent_climate) {
                        if (direction == 0) north_higher = 1;
                        if (index > 1 && index < 5) north_lower = 1;
                    }
                    if (adjacent_climate < (tile->climate & 0xE0)) {
                        if (direction == 0) north_lower = 1;
                        if (index > 1 && index < 5) north_higher = 1;
                    }
                }
            }
            const int river = (north_higher != 0 && north_lower == 0);
            score2 = (8 - north_lower) * 2;
            score = river - ((fungus + saved_score) * 3) - elevation + score2;
            if (!saved_score) {
                terrain_value = tile->val3 & 0xC0;
                if (terrain_value == 0 && scores[0x10] < score) {
                    scores[0x10] = score;
                    best_rain_y = neighbor_y;
                    scores[0x12] = neighbor_x;
                } else if (terrain_value == 0x40) {
                    score = fungus_yield + score * 8;
                    if (scores[0x0C] < score && (ring == 0 || best_forest_x < 0)) {
                        scores[0x0C] = score;
                        best_forest_y = neighbor_y;
                        best_forest_x = neighbor_x;
                        scores[0x14] = neighbor_y;
                    }
                } else if (terrain_value == 0x80 && scores[0x1B] < score) {
                    scores[0x1B] = score;
                    best_rock_y = neighbor_y;
                    best_rock_x = neighbor_x;
                    scores[0x16] = neighbor_y;
                    scores[0x18] = neighbor_x;
                }
            }

            elevation_score = tile->climate >> 5;
            if (elevation_score == 3) {
                score3 = fungus_yield +
                    ((((4 - north_lower) * 2 - fungus - saved_score) * 2 -
                      north_higher) - elevation) * 8;
                if ((tile->val3 & 0xC0) > 0x40) score3 = 0;
                if (scores[0x0B] < score3) {
                    scores[0x0B] = score3;
                    best_altitude_y = neighbor_y;
                    best_altitude_x = neighbor_x;
                    scores[0x1A] = neighbor_y;
                    scores[9] = neighbor_x;
                }
            }
            if (scores[0x19] < elevation_score) {
                scores[0x19] = elevation_score;
                scores[0x1D] = neighbor_x;
                scores[0x1C] = neighbor_y;
            }
            const unsigned int land_type = tile->climate & 7;
            const unsigned int moisture_type = (tile->climate >> 3) & 3;
            score2 = (((fungus + saved_score) * -3 - (tile->val3 >> 6)) - elevation_score) +
                     ((north_higher != 0 && north_lower == 0) ? 1 : 0) + 16;
            if (scores[land_type + 3] < score2) {
                positions[land_type + 6] = neighbor_x;
                positions[land_type + 3] = neighbor_y;
                positions[land_type] = land_type;
                scores[land_type + 3] = static_cast<int>(land_type);
            }
            if (scores[moisture_type] < score2) {
                positions[moisture_type + 12] = neighbor_x;
                positions[moisture_type + 9] = neighbor_y;
                positions[moisture_type + 15] = moisture_type;
                scores[moisture_type] = score2;
            }
            if (best_direction_score < static_cast<int>(moisture_type) ||
                (moisture_type == static_cast<unsigned int>(best_direction_score) &&
                 scores[moisture_type] <= score2)) {
                best_direction_score = moisture_type;
                scores[8] = neighbor_x;
                scores[6] = neighbor_y;
            }
            if (static_cast<int>(moisture_type) < scores[7] ||
                (moisture_type == static_cast<unsigned int>(scores[7]) &&
                 scores[moisture_type] <= score2)) {
                scores[7] = moisture_type;
                scores[0x0F] = neighbor_x;
            }
            if (scores[0x17] < static_cast<int>(land_type) ||
                (land_type == static_cast<unsigned int>(scores[0x17]) &&
                 scores[land_type + 3] <= score2)) scores[0x17] = land_type;
            if (static_cast<int>(land_type) < scores[0x1E] ||
                (land_type == static_cast<unsigned int>(scores[0x1E]) &&
                 scores[land_type + 3] <= score2)) scores[0x1E] = land_type;
            if (!saved_score && (tile_bits & 0xA000) == 0) {
                if (fungus_yield == 1 && scores[0x15] < score2) scores[0x15] = score2;
                else if (fungus_yield == 2 && scores[0x0E] < score2) scores[0x0E] = score2;
                result = world_site(neighbor_x, neighbor_y, 0);
                if (scores[0x13] < result) {
                    best_site_x = neighbor_x;
                    best_site_y = neighbor_y;
                    scores[0x13] = result;
                }
            }
        }
    }

    if (tour_step == 0) {
        if (scores[10] < 0 &&
            tut_map("TOUROCEAN", scores[10], scores[0x11], -1, 0, 2, -1, -1) == 0) {
            goto tour_cleanup;
        }
        if (best_target_x < 0 &&
            tut_map("TOURFUNGUS", best_target_x, best_target_y, -1, 0, 2, -1, -1) == 0) {
            goto tour_cleanup;
        }
        if (TutWinClimateGate945830 != 0) WorldClimateWorldWin.show_all();
        do_all_draws();
        if (!ConsoleGlobal.alpha_menu_.is_visible()) {
            ConsoleGlobal.alpha_menu_.show();
        }

        if (!tut_callout(*this, MainInterfaceGlobal,
                         MainInterfaceGlobal.menu_button_rect_, "TOURMENU")) {
            goto tour_cleanup;
        }
        if (MultiWindowOpen == 0) MultiWindow.show();
        if (!tut_callout(*this, MainInterfaceGlobal,
                         MainInterfaceGlobal.multiplayer_button_rect_, "TOURCOMMLINK")) {
            goto tour_cleanup;
        }
        MultiWindow.hide();

        // These panels are MainInterface's own stored RECTs. Their centers
        // are transformed through the same live interface receiver as in the
        // original sequence, and each popup is hosted by the active map.
        RECT *const interface_rects[] = {
            &MainInterfaceGlobal.status_left_rect_,
            &MainInterfaceGlobal.status_right_rect_,
            &MainInterfaceGlobal.status_hit_rect_,
            &MainInterfaceGlobal.message_bounds_,
            &MainInterfaceGlobal.scroll_up_rect_,
            &MainInterfaceGlobal.scroll_down_rect_,
            &MainInterfaceGlobal.tutorial_bounds_,
            &MainInterfaceGlobal.date_rect_
        };
        const char *const interface_labels[] = {
            "TOURUNITSINFO", "TOURTERRAININFO", "TOURSTACKINFO",
            "TOURMESSAGEINFO", "TOURBUTTONMESSAGE", "TOURBUTTONINFO",
            "TOURWORLDINFO", "TOURDATEINFO"
        };
        for (index = 0; index < 8; ++index) {
            if (!tut_callout(*this, MainInterfaceGlobal, *interface_rects[index],
                             interface_labels[index])) {
                goto tour_cleanup;
            }
        }
        if (ConsoleGlobal.cursor_active_ != 0 && TurnCurrentNum < 2) {
            if (field_53D4_ != 0) {
                field_53A4_ = 0;
                hide();
                field_53D4_->on_update();
                do_all_draws();
            }
            field_53D4_ = 0;
            field_53A4_ = 0;
            field_5380_ = -1;
            field_537C_ = -1;
            field_539C_ = -1;
            field_53A8_ = 0;
            field_53AC_ = -1;
            field_53B8_ = 0;
            field_53C4_ = 0;
            TutWinShownFlag = 0;
            field_53B4_ = 0;
            if (best_site_x < 1) {
                index = ConsoleGlobal.cursor_history_index_;
                best_site_x = ConsoleGlobal.cursor_history_x_[index];
                best_site_y = ConsoleGlobal.cursor_history_y_[index];
            }
            tut_map("TOUROVER", best_site_x, best_site_y, ConsoleGlobal.selected_vehicle_,
                    0, 0, -1, -1);
            return;
        }
        goto tour_cleanup;
    }

    index = ConsoleGlobal.cursor_history_index_;
    result = tut_map("TOURMAP", ConsoleGlobal.cursor_history_x_[index],
                     ConsoleGlobal.cursor_history_y_[index], -1, 0, 2, -1, -1);
    if (result == 0) goto tour_cleanup;
    if (scores[8] >= 0 && scores[0x0F] >= 0 &&
        tut_map("TOURFUNGUS", scores[8], scores[6], -1, 0, 2, -1, -1) == 0) {
        goto tour_cleanup;
    }
    for (index = 2; index >= 0; --index) {
        if (positions[index + 0x0F] >= 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, "TOURRAIN");
            if (index == 0) {
                strcat(StringTemp, best_direction_score == 0 ? "DRY2" : "DRY");
            } else if (index == 1) {
                strcat(StringTemp, "MOD");
            } else {
                strcat(StringTemp, "WET");
            }
            if (tut_map(StringTemp, positions[index + 0x0C], positions[index + 9],
                        -1, 0, 2, -1, -1) == 0) {
                goto tour_cleanup;
            }
        }
    }
    if ((scores[9] >= 0 && scores[0x1D] >= 0 &&
         tut_map("TOURALT", scores[9], scores[0x1A], -1, 0, 2,
                 scores[0x1D], scores[0x1C]) != 0) ||
        (scores[0x18] >= 0 &&
         tut_map("TOURTWOROCK", scores[0x18], scores[0x16], -1, 0, 2, -1, -1) != 0) ||
        (best_forest_x >= 0 &&
         tut_map("TOURONEROCK", best_forest_x, scores[0x14], -1, 0, 2, -1, -1) != 0)) {
        if (scores[0x12] >= 0 &&
            tut_map("TOURFLAT", scores[0x12], best_rain_y, -1, 0, 2, -1, -1) == 0) {
            goto tour_cleanup;
        }
        if (best_site_x < 1) {
            index = ConsoleGlobal.cursor_history_index_;
            best_site_x = ConsoleGlobal.cursor_history_x_[index];
            best_site_y = ConsoleGlobal.cursor_history_y_[index];
        }
        if (tut_map("TOURFOREST", best_site_x, best_site_y, -1, 0, 2, -1, -1) == 0 ||
            tut_map("TOURREVIEW", best_site_x, best_site_y, -1, 0, 2, -1, -1) == 0 ||
            tut_map("TOURADVANCED", best_site_x, best_site_y, -1, 0, 2, -1, -1) == 0) {
            goto tour_cleanup;
        }
        if (field_53D4_ != 0) {
            field_53A4_ = 0;
            hide();
            field_53D4_->on_update();
            do_all_draws();
        }
        field_5380_ = -1;
        field_537C_ = -1;
        field_539C_ = -1;
        field_53AC_ = -1;
        field_53D4_ = 0;
        field_53A4_ = 0;
        field_53A8_ = 0;
        field_53B8_ = 0;
        field_53C4_ = 0;
        TutWinShownFlag = 0;
        field_53B4_ = 0;
        tut_map("TOURFORMDONE", best_site_x, best_site_y, -1, 0, 2, -1, -1);
    }
tour_cleanup:
    desktop_update();
    if (field_53D4_ != 0) {
        field_53A4_ = 0;
        hide();
        field_53D4_->on_update();
        do_all_draws();
    }
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
    field_53B4_ = 0;
}

// ORIGINAL: 0x004BAF20 ?tour7@TutWin@@QAEXXZ 0x004BAF20-0x004BB373 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004baf20.cpp", "image:0x004baf20; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour7() {
    const int was_visible = TutWinDesWindow.is_visible();
    int x;
    int y;

    if (!was_visible) {
        TutWinDesWindow.exec(ConsoleGlobal.faction_id_, -2);
    }

    RECT temporary;

    temporary = (TutWinDesWindow.chassis_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURCHAS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.weapon_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURWEAP", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.armor_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURARM", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.reactor_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURREACTOR", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.special_one_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURABILITY", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.field_B2C_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURCOST", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary.left = TutWinDesWindow.field_B3C_;
    temporary.top = TutWinDesWindow.field_B40_;
    temporary.right = TutWinDesWindow.field_B44_;
    temporary.bottom = TutWinDesWindow.field_B48_;
    temporary = (temporary);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURSTATS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.field_B0C_rect_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURBAR", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.field_CBC_rect_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURARROWS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.caption_rect_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURPRESETS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.lower_panel_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURDONE", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    temporary = (TutWinDesWindow.field_B5C_rect_);
    des_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "DESTOURAPPLY", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.lower_panel_extra_rect_, "DESTOURRENAME", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.field_BAC_rect_, "DESTOUROBS", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.field_BBC_rect_, "DESTOURUPGRADE", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.field_B4C_rect_, "DESTOURRETIRE", 2))
        goto tour7_cleanup;
    do_des(&TutWinDesWindow.lower_panel_end_rect_, "DESTOURCANCEL", 2);

tour7_cleanup:
    if (!was_visible) {
        TutWinDesWindow.shut_that_badboy_down();
    }
}

// ORIGINAL: 0x004BB380 ?tour5@TutWin@@QAEXXZ 0x004BB380-0x004BB791 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004bb380.cpp", "image:0x004bb380; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour5() {
    const int was_visible = TutWinSocWindow.is_visible();
    int x;
    int y;
    int i;
    RECT temporary;

    if (!was_visible) {
        TutWinSocWindow.init(-ConsoleGlobal.faction_id_);
        TutWinSocWindow.show(0);
    }


    temporary = (TutWinSocWindow.tutRect4_);
    soc_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURMAIN", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tooltipFactionPower_);
    soc_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURFACTION", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tooltipEffects_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOUROVERVIEW", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tutRect1_);
    soc_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOUREFFECTS", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    for (i = 0; i < 10; ++i) {
        StringTemp[0] = 0;
        strcat(StringTemp, "SOCTOUR");
        say_num(i);
        temporary = TutWinSocWindow.tutRects2_[i];
        soc_rect(&temporary, &x, &y);
        if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
                    StringTemp, x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }

    temporary = (TutWinSocWindow.tooltipEconomy_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURECON", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tooltipPsych_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURPSYCH", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tooltipLabs_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURLABS", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tutRect14_);
    iface_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURINFO", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tutRect11_);
    soc_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURCANCEL", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = (TutWinSocWindow.tutRect12_);
    soc_rect(&temporary, &x, &y);
    if (tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
    "SOCTOURRESET", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    temporary = TutWinSocWindow.tutRect13_;
    do_soc(&temporary, "SOCTOUROK", 2);

tour5_cleanup:
    if (!was_visible) {
        TutWinSocWindow.hide();
        TutWinSocWindow.release_iface_mode();
        TutWinSocWindow.close();
    }
}

// ORIGINAL: 0x004BAA70 ?tour6@TutWin@@QAEXXZ 0x004BAA70-0x004BAF12 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004baa70.cpp", "image:0x004baa70; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour6() {
    if (!TutWinBaseWindow.is_visible()) {
        for (int i = 0; i < BaseCurrentCount; ++i) {
            if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                TutWinBaseWindow.zoom(i, 0);
                break;
            }
        }
        if (!TutWinBaseWindow.is_visible()) return;
    }

    TutWinBaseWindow.focus();
    parse_says(0, BaseCurrent->name_string, -1, -1);
    int x;
    int y;
    RECT temporary = TutWinBaseWindow.population_rect_;
    x = temporary.left + 5;
    y = temporary.top + (temporary.bottom - temporary.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (!tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
                 "DRONETOUR1", x, y, 0, 2, -1, -1)) return;
    temporary = TutWinBaseWindow.garrison_rect_;
    if (!tut_callout(*this, MainInterfaceGlobal, temporary, "DRONETOUR2")) return;
    temporary = TutWinBaseWindow.rect12_;
    if (!tut_callout(*this, MainInterfaceGlobal, temporary, "DRONETOUR3")) return;
    temporary = TutWinBaseWindow.flash_rect_a_;
    if (!tut_callout(*this, TutWinBaseWindow, temporary, "DRONETOUR4")) return;
    temporary = TutWinBaseWindow.population_rect_;
    x = temporary.left + 5;
    y = temporary.top + (temporary.bottom - temporary.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (!tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,
                 "DRONETOUR5", x, y, 0, 2, -1, -1)) return;

    TutWinBaseWindow.draw_energy_alloc(2);
    if (!TutWinBaseWindow.flatButtons_[17].is_visible()) {
        temporary = TutWinBaseWindow.expenses_help_rects_[5];
        if (!tut_callout(*this, TutWinBaseWindow, temporary, "DRONETOUR6")) return;
    } else {
        temporary = TutWinBaseWindow.rect13_;
        if (!tut_callout(*this, TutWinBaseWindow, temporary, "DRONETOUR6")) return;
    }
    TutWinBaseWindow.draw_expenses(1);
    TutWinBaseWindow.draw_farm(2);
    temporary = TutWinBaseWindow.rect11_;
    if (tut_callout(*this, TutWinBaseWindow, temporary, "DRONETOUR6")) {
        TutWinBaseWindow.draw_farm(1);
    }
}

// ORIGINAL: 0x004BB7A0 ?tour1@TutWin@@QAEXXZ 0x004BB7A0-0x004BC591 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bb7a0.cpp", "image:0x004bb7a0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour1() {
    if (!TutWinBaseWindow.is_visible()) {
        for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
            if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
                TutWinBaseWindow.zoom(base_id, 0);
                break;
            }
        }
        if (!TutWinBaseWindow.is_visible()) return;
    }

    TutWinBaseWindow.focus();
    parse_says(0, BaseCurrent->name_string, -1, -1);
    TutWinBaseWindow.draw_farm(4);

    int x;
    int y;
    RECT rect;
    MapWin *const map = MapWinTable[0];
    GraphicWin *const owner = map != 0 ? static_cast<GraphicWin *>(map) : 0;

    rect = (TutWinBaseWindow.rect8_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1GOVERNOR", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect14_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1EXPLORE", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect15_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1DISCOVER", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect18_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1BUILD", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect19_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1CONQUER", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect8_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1GOVERNOR2", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect16_);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1ADVANCED", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.info_title_rect_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1CAPTION", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect5_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1ECO", x, y, 0, 2, -1, -1) == 0) return;

    rect = TutWinBaseWindow.population_rect_;
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1POP", x, y, 0, 2, -1, -1) == 0) return;
    if (tut_win(owner, "TOUR1POP2", x, y, 0, 2, -1, -1) == 0) return;

    rect = (TutWinBaseWindow.flash_rect_a_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1STAPLE", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.queue_help_rect_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1PRODUCTION", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.queue_content_rect_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1QUEUE", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.flash_rect_b_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1CHANGE", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.garrison_rect_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1HURRY", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect1_);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1GARRISON", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect2_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1SUPPORTED", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.commerce_report_rect_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1FACILITIES", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.nutrient_content_rect_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1COMMERCE", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect1_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1NUTRIENTS", x, y, 0, 2, -1, -1) == 0) return;

    TutWinBaseWindow.draw_expenses(2);
    rect = (TutWinBaseWindow.expenses_help_rects_[1]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1AREA", x, y, 0, 2, -1, -1) == 0) return;
    parse_num(0, static_cast<int>(Rules.nutrient_req_citizen));
    rect = (TutWinBaseWindow.expenses_help_rects_[2]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1NUT", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.expenses_help_rects_[0]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1MIN", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_energy_alloc(2);

    if (TutWinBaseWindow.flatButtons_[17].is_visible()) {
        rect = (TutWinBaseWindow.rect13_);
        base_rect(&rect, &x, &y);
        if (tut_win(owner, "TOUR1ENERGY", x, y, 0, 2, -1, -1) == 0) return;
    } else {
        rect = (TutWinBaseWindow.expenses_help_rects_[5]);
        x = rect.left + (rect.right - rect.left) / 2;
        y = rect.top + (rect.bottom - rect.top) / 2;
        TutWinBaseWindow.client_to_screen(&x, &y);
        if (tut_win(owner, "TOUR1ALLOC", x, y, 0, 2, -1, -1) == 0) return;
    }
    rect = (TutWinBaseWindow.expenses_help_rects_[5]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1ALLOC2", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.expenses_help_rects_[3]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1ECON", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.expenses_help_rects_[4]);
    x = rect.left + (rect.right - rect.left) / 2;
    y = rect.top + (rect.bottom - rect.top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(owner, "TOUR1PSYCH", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_expenses(1);
    TutWinBaseWindow.draw_farm(3);
    rect = (TutWinBaseWindow.rect9_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1LABS", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_farm(2);
    rect = (TutWinBaseWindow.rect11_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1SUPPORTBUTTON", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_farm(4);
    rect = (TutWinBaseWindow.rect10_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1PSYCHBUTTON", x, y, 0, 2, -1, -1) == 0) return;

    rect.left = TutWinBaseWindow.previous_button_rect_.left;
    rect.top = TutWinBaseWindow.previous_button_rect_.top;
    rect.right = TutWinBaseWindow.previous_button_rect_.right;
    rect.bottom = TutWinBaseWindow.previous_button_rect_.bottom;
    rect = (rect);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1RESOURCEBUTTON", x, y, 0, 2, -1, -1) == 0) return;
    rect.left = TutWinBaseWindow.next_button_rect_.left;
    rect.top = TutWinBaseWindow.next_button_rect_.top;
    rect.right = TutWinBaseWindow.next_button_rect_.right;
    rect.bottom = TutWinBaseWindow.next_button_rect_.bottom;
    rect = (rect);
    iface_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1PREV", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect7_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1NEXT", x, y, 0, 2, -1, -1) == 0) return;
    rect = (TutWinBaseWindow.rect6_);
    base_rect(&rect, &x, &y);
    if (tut_win(owner, "TOUR1RENAME", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_farm(1);

}



/*
 * Four 67-byte clones differing only in which fixed window they finish on.
 * Each centres the rectangle on both axes and hands the pair to that window's
 * client_to_screen. The centring is inlined four times rather than shared,
 * matching the original, which carries four copies.
 *
 * The halving is the compiler's signed divide-by-two - `sub eax,esi; cdq;
 * sub eax,edx; sar eax,1` - so it truncates toward zero rather than flooring,
 * and the subtraction wraps. Modelled here the way win.cpp's own midpoint()
 * models the identical idiom: unsigned arithmetic throughout, with the sign
 * bit carried back over the shift.
 */


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the iface window.
// ORIGINAL: 0x004BC5A0 ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5A0-0x004BC5E3 BYTE_EXACT
// LEVER: plain `left + (right - left) / 2` signed division - the image
//        already emits the `cdq; sub; sar` fixup itself, so the hand-rolled
//        unsigned-with-sign-carried-over-the-shift trick this body used to
//        model the SAME idiom compiled to different instructions. Matches
//        the sibling `base_rect`/`soc_rect`/`des_rect`, already byte-exact
//        on this same lever.
// symbol    ?iface_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x007AE820,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::iface_rect(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    MainInterfaceGlobal.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the base window.
// ORIGINAL: 0x004BC5F0 ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5F0-0x004BC633 BYTE_EXACT
// symbol    ?base_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x006A7628,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::base_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the soc window.
// ORIGINAL: 0x004BC640 ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC640-0x004BC683 BYTE_EXACT
// symbol    ?soc_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x008A6270,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::soc_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the des window.
// ORIGINAL: 0x004BC690 ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC690-0x004BC6D3 BYTE_EXACT
// symbol    ?des_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x0071F2B0,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::des_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow.client_to_screen(x, y);
}



/*
 * The four do_* helpers: 123-byte clones whose ONLY real difference is which
 * fixed window they centre through. The other differing bytes are the two
 * call displacements, which move because the calls are relative and the four
 * bodies sit 0x80 apart - not because either target differs. Both targets are
 * the same for all four.
 */

/*
Purpose: Centre the rectangle, convert it through the base window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA870 ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA870-0x004BA8EB SEMANTIC
// LEVER: two levers stacked, 4/56 -> 45/56 MNEMONIC_ONLY. (1) plain `left + (right - left) / 2` signed division instead of the unsigned-with-sign-carried trick, same as `iface_rect`. (2) `primary` read BEFORE `window` is given any value, and its null-check written `if (primary == nullptr) { window = nullptr; } else { ...compute... }` (primary-first declaration order, and the null case as the FIRST arm) - the image reads the global first and only zeroes the result register on the branch that skips the vtable dereference, both of which a `window = nullptr;`-then-test body reorders.
// TRIED: 45/56 plateau - the remainder is a consistent eax/ecx role swap through the vbtable-dereference block and the final argument push order, tried: `uint8_t *` instead of `void *` for `window`, and folding the `vbtable` local into the expression directly - neither changes it. VC6 register allocation, not a source-form lever found here.
// symbol    ?do_base@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_base(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary != 0
        ? static_cast<GraphicWin *>(primary) : 0;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the iface window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA8F0 ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA8F0-0x004BA96B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_iface@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_iface(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary != 0
        ? static_cast<GraphicWin *>(primary) : 0;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the soc window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA970 ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA970-0x004BA9EB SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_soc@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_soc(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary != 0
        ? static_cast<GraphicWin *>(primary) : 0;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the des window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA9F0 ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA9F0-0x004BAA6B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_des@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_des(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary != 0
        ? static_cast<GraphicWin *>(primary) : 0;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Tell the attached object the tutorial window moved.
// ORIGINAL: 0x004BDDB0 ?on_move@TutWin@@QAEXHH@Z 0x004BDDB0-0x004BDDC5 BYTE_EXACT
// symbol    ?on_move@TutWin@@UAEXHH@Z
// size      21 bytes
// prototype void (__thiscall ?on_move@TutWin@@QAEXHH@Z)(TutWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004BDDBC
Return Value: n/a
Status: Complete
*/
void TutWin::on_move(int x, int y) {
    // field_53D4_ holds a Win-family object pointer (now typed as one); the
    // repaint is its declared on_update virtual.
    if (field_53D4_) {
        field_53D4_->on_update();
    }
}

// ORIGINAL: 0x004BA770 ?on_key_click@TutWin@@QAEHHH@Z 0x004BA770-0x004BA82E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BA770-0x004BA82E; seed:src/unrecovered/004ba770.cpp; source:src/win.h:Win::release_modal,Win::hide,Win::on_update"], "reason": "The tutorial key callback owns its modal dismissal, saved key tuple, and TutWin state reset."}
// symbol    ?on_key_click@TutWin@@UAEHHH@Z
int TutWin::on_key_click(int key, int code) {
    if (field_53B8_ != 0) {
        release_modal();
        if (field_53D4_ != 0) {
            field_53A4_ = 0;
            hide();
            field_53D4_->on_update();
            do_all_draws();
        }
        field_53D4_ = 0;
        field_5380_ = -1;
        field_537C_ = -1;
        field_539C_ = -1;
        field_53AC_ = -1;
        field_53A4_ = 0;
        field_53A8_ = 0;
        field_53B8_ = 0;
        field_53C4_ = 0;
        TutWinShownFlag = 0;
        if (code == 0x1b) {
            field_53B4_ = 0;
        }
        field_53C4_ = 1;
        field_53C8_ = key;
        field_53CC_ = code;
        return 1;
    }
    return 0;
}



// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004BA6B0 ??0TutWin@@QAE@XZ 0x004BA6B0-0x004BA71E BYTE_EXACT
// body      src/tutwin.h
// size      110 bytes
// prototype void (__thiscall ??0TutWin@@QAE@XZ)(TutWin* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004048A0
*/

// LEVER: `: public Popup` real inheritance, so the single call is the
// implicit base constructor (now BYTE_EXACT, see popup.cpp) - no explicit
// `Popup::Popup()` call needed, and (matching Popup's own fix) the trailing
// stores use a non-volatile pointer.
TutWin::TutWin() {
    field_53B4_ = 0;
    field_53C0_ = 0;
    field_53D0_ = 0;
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

// The original inline teardown is exactly Popup::close followed by the
// compiler-owned destructors for Popup's Scroll, BasePop members, and
// GraphicWin bases. TutWin owns no non-trivial member after Popup, so letting
// the declared C++ hierarchy run its normal reverse-lifetime chain preserves
// that ordering without replaying subobject offsets or vptr stores.
// ORIGINAL: 0x004BE7D0 ??1TutWin@@QAE@XZ 0x004BE7D0-0x004BE9EE;0x00659BF0-0x00659CD7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004be7d0.cpp", "image:0x004be7d0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ??1TutWin@@UAE@XZ
TutWin::~TutWin() {}

// ORIGINAL: 0x004BDFA0 ?base_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDFA0-0x004BDFDD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004bdfa0.cpp", "image:0x004bdfa0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?base_draw@TutWin@@SAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl TutWin::base_draw(Sprite *, GraphicWin *window, int mode,
                              int pixel_x, int pixel_y) {
    int base_id = mode + 2;
    if (base_id < 0) {
        base_id = -2 - mode;
    }
    Buffer *const destination = window != 0
        ? static_cast<Buffer *>(window) : 0;
    ::base_draw(destination, base_id, pixel_x, pixel_y, 0, 0);
}

/*
Purpose: Step the receiver back to the subobject ??_GTutWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004BEA20 ??_GTutWin@@WEEE@AEPAXI@Z 0x004BEA20-0x004BEA2B BYTE_EXACT
// symbol    ??_ETutWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3TutWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004BE9F0` into
//   ??_GTutWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// Tutorial arrows use game.cpp's PopupThemeWindowColor (original 0x0068A5A0).
// config_popups updates that shared palette cell; no separate arrow copy exists.

/*
Purpose: Draw the tutorial's pointing arrow: its shaft across the rectangle the
         window records, then its head from the recorded corner, both in the one
         colour index and only when the window says it is on screen at all.
// ORIGINAL: 0x004BDEA0 ?draw_arrow@TutWin@@QAEXXZ 0x004BDEA0-0x004BDF16 FILE BYTE_EXACT
// LEVER: the two calls are Win::window_line_raw on the Win-family object at
//   0x53D4, and the second only fires when field_539C_ is non-negative - the
//   arrow's head, drawn from a different corner of the same rectangle.
// size      118 bytes
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F2AC0
Return Value: n/a
Status: Complete
*/
void TutWin::draw_arrow() {
    if (field_53A4_ != 0) {
        field_53D4_->window_line_raw(field_538C_, field_5390_, field_5384_,
                                     field_5388_, PopupThemeWindowColor, 2, 2);
        int head = field_539C_;
        if (head >= 0) {
            field_53D4_->window_line_raw(head, field_53A0_, field_5394_,
                                         field_5398_, PopupThemeWindowColor, 2, 2);
        }
    }
}

// ORIGINAL: 0x004BA5F0 ?tut_check@@YAHH@Z 0x004BA5F0-0x004BA64E FILE BYTE_EXACT
// LEVER: test/push the flag word as a named `extern int`, not a const-address pointer - the pointer spelling lets VC6 prove no alias with the table read, cache it in eax and push/pop esi; a real global may alias, so it reloads and `|=` lowers to load/or/store. Transferred from 0x004BA650, then MEASURED here.
// size      94 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

tut_check(int flag) {
    // Gates, in the image's order: no tutorial hints in multiplayer, none
    // unless GamePreferences keeps the hints switch on, none for a hint bit
    // already shown, and none in an expansion alien faction (bit 0x80 of
    // rule_flags is RFLAG_ALIEN - the same gate X_pop's mod-caption check
    // makes). Players is walked by faction because 0x00946F58 is
    // Players[0].rule_flags and the image scales its index by 0x59C.
    if (IsMultiplayerNet != 0) {
        return 0;
    }
    if ((GamePreferences & 0x20) == 0) {
        return 0;
    }
    if ((flag & TutorialFlags) != 0) {
        return 0;
    }
    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        return 0;
    }
    TutorialFlags |= flag;
    return 1;
}

// ORIGINAL: 0x004BA650 ?tut_check2@@YAHH@Z 0x004BA650-0x004BA6AE FILE BYTE_EXACT
// LEVER: test/push 0x9A64B0 as a named `extern int`, not a const-address pointer: the pointer spelling let VC6 cache the flag word in eax across the table read and push/pop esi for the parameter; a real global may be aliased by the table pointer, so it reloads, keeps `a1` in scratch edx, and `|=` lowers to load/or/store instead of `or [mem], reg`.
// size      94 bytes
// prototype 
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

tut_check2(int flag) {
    // The second gate, identical in shape to tut_check but with its own
    // shown-bit word, which is what makes the two hint sequences independent.
    if (IsMultiplayerNet != 0) {
        return 0;
    }
    if ((GamePreferences & 0x20) == 0) {
        return 0;
    }
    if ((flag & TutorialFlags2) != 0) {
        return 0;
    }
    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        return 0;
    }
    TutorialFlags2 |= flag;
    return 1;
}
