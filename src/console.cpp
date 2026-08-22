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
#include "console.h"
#include "game.h"
#include "general.h"
#include "infowin.h"
#include "map.h"
#include "mapwin.h"
#include "statuswin.h"
#include "temp.h"
#include "fx.h"
#include "hypothesis_layouts.h"
#include "graphicwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "menu.h"
#include "sprite.h"
#include "time.h"
#include "texture.h"
#include "vector_teardown.h"
#include <cstring>

// The manual "vtable" pointers `GraphicWin::construct`'s own idiom writes
// when GraphicWin is directly the most-derived object (see graphicwin.cpp),
// repeated here on Console's own embedded Menu and on its virtual base -
// nothing in this chain declares a single `virtual`, so VC6 never refreshes
// them on its own.
static void *const g_0066ef04 = reinterpret_cast<void *>(0x0066EF04);
static void *const g_0066ed88 = reinterpret_cast<void *>(0x0066ED88);
static void *const g_0066ed80 = reinterpret_cast<void *>(0x0066ED80);
static void *const g_0066ec18 = reinterpret_cast<void *>(0x0066EC18);
static void *const g_0066ec10 = reinterpret_cast<void *>(0x0066EC10);
static void *const g_005e37e0 = reinterpret_cast<void *>(0x005E37E0);
static void *const g_00406850 = reinterpret_cast<void *>(0x00406850);
static void *const g_0066a57c = reinterpret_cast<void *>(0x0066A57C);
static void *const g_0066a574 = reinterpret_cast<void *>(0x0066A574);

/*
Purpose: Build the game console - its virtual GraphicWin base, its MapWin
         base, its own extra (non-virtual) embedded GraphicWin, and its own
         Buffer/Time/Menu/Sprite members.
// ORIGINAL: 0x0050F460 ??0Console@@QAE@H@Z 0x0050F460-0x0050F629;0x0065D250-0x0065D2EF
// symbol    ?construct@Console@@QAEXH@Z
// size      616 bytes
// prototype void (__thiscall ??0Console@@QAE@H@Z)(Console* this, int)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004626E0 0x005D4CF0 0x005D7210 0x005E37E0 0x005FAC60 0x006161D0 0x006457C2
Return Value: this

MEASURED: not spelled as a real constructor - see the note in `console.h`
and `mapwin.h`. A plain method never gets VC6's own most-derived-flag
treatment, so the single `a1` here is read and branched on exactly as the
image's `[ebp+8]` is, with no second, compiler-inserted flag arriving
alongside it. Console's own fields past the MapWin base are still opaque
storage (see the class declaration), so everything below reaches its
sub-objects by raw offset.
*/
void Console::construct(int a1) {
    char *const self = reinterpret_cast<char *>(this);

    if (a1) {
        *reinterpret_cast<void **>(self) = g_0066ef04;
        new (reinterpret_cast<GraphicWin *>(self + 0x23d94)) GraphicWin();
    }

    MapWin::construct(0);

    new (reinterpret_cast<GraphicWin *>(self + 0x21a68)) GraphicWin();
    reinterpret_cast<Buffer *>(self + 0x2247c)->Buffer::Buffer();
    reinterpret_cast<Time *>(self + 0x22a04)->Time::Time();

    char *const menu = self + 0x22a2c;
    reinterpret_cast<Menu *>(menu)->Menu::Menu();

    reinterpret_cast<Buffer *>(menu + 0xb64)->Buffer::Buffer();
    reinterpret_cast<Sprite *>(menu + 0x10ec)->Sprite::Sprite();

    VectorCtorIterator(menu + 0x1118, 0x2c, 3, g_005e37e0, g_00406850);

    *reinterpret_cast<void **>(menu) = g_0066ed88;
    *reinterpret_cast<void **>(menu + 0x444) = g_0066ed80;

    reinterpret_cast<Sprite *>(self + 0x23d28)->Sprite::Sprite();
    reinterpret_cast<Sprite *>(self + 0x23d54)->Sprite::Sprite();

    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(self);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(self + off) = g_0066ec18;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(self);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(self + off + 0x444) = g_0066ec10;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(self);
        int32_t const off = vtbl[1];
        *reinterpret_cast<int32_t *>(self + off - 4) = off - 0x23d94;
    }

    field_23BC8_ = static_cast<uint32_t>(-1);
    field_23BD8_ = 0;
    field_23BE4_ = 0;
    field_23BE8_ = 0;
    field_23BF0_ = 0;
    field_23BF4_ = 0;
    field_23C00_ = 0;
    field_23C04_ = 0;
    field_23BFC_ = 0;
    field_23BF8_ = 0;
    field_23C08_ = 0;

    for (int32_t i = 0x20; i != 0; i--) {
        int32_t idx = field_23C08_;
        *reinterpret_cast<int32_t *>(self + 0x23c10 + idx * 4) = 0;
        idx = field_23C08_;
        *reinterpret_cast<int32_t *>(self + 0x23c90 + idx * 4) = 0;
    }

    field_23D10_ = 0;
    field_23D18_ = 0;
    field_23D88_ = 0;
    field_23D80_ = 0;
    field_23D8C_ = 0;
    field_23D84_ = 0;
}

/*
Purpose: Tear down the game console - its own Buffer/Time/Menu/Sprite
         members, its own extra embedded GraphicWin, then the MapWin
         fields it holds directly (no separate call into MapWin's own
         destructor).
// ORIGINAL: 0x0051D9F0 ??1Console@@QAE@XZ 0x0051D9F0-0x0051DC10;0x0065D0E0-0x0065D220
// symbol    ??1Console@@UAE@XZ
// size      864 bytes
// prototype void (__thiscall ??1Console@@QAE@XZ)(Console* this)
// callers   2   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x0051D7D0 0x005D4DD0 0x005D7410 0x005E3820 0x005FAD00 0x00616200 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

`this` IS NOT Console's own front, matching `guarded_teardowns.cpp`'s
already-matching `->Console::~Console()` on `TeardownObject00939444` -
`g_CONSOLE`'s front plus 0x23D94, the offset of the embedded virtual-base
GraphicWin, the same convention PlanWin's and MapWin's destructors use (see
the note in `planwin.cpp`).
*/
Console::~Console() {
    char *const self = reinterpret_cast<char *>(this);

    // Console's own tail, ahead of the virtual base at `self`.
    char *const front = self - 0x23d94;
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(front);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(front + off) = g_0066ec18;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(front);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(self - 0x23950 + off) = g_0066ec10;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(front);
        int32_t const off = vtbl[1];
        *reinterpret_cast<int32_t *>(self - 0x23d98 + off) = off - 0x23d94;
    }

    close();

    reinterpret_cast<Sprite *>(self - 0x40)->close();
    reinterpret_cast<Sprite *>(self - 0x6c)->close();

    char *const menu = self - 0x1368;
    VectorDtorIterator(menu + 0x1118, 0x2c, 3, g_00406850);
    reinterpret_cast<Sprite *>(menu + 0x10ec)->close();
    reinterpret_cast<Buffer *>(menu + 0xb64)->~Buffer();
    reinterpret_cast<Menu *>(menu)->Menu::~Menu();

    reinterpret_cast<Time *>(self - 0x1390)->~Time();
    reinterpret_cast<Buffer *>(self - 0x1918)->~Buffer();
    reinterpret_cast<GraphicWin *>(self - 0x232c)->GraphicWin::~GraphicWin();

    // Rebase onto the MapWin base's own embedded (virtual) GraphicWin.
    char *const mapwin_vbase = self - 0x2328;
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(mapwin_vbase - 0x21a6c);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(mapwin_vbase - 0x21a6c + off) = g_0066a57c;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(mapwin_vbase - 0x21a6c);
        int32_t const off = vtbl[1];
        *reinterpret_cast<void **>(mapwin_vbase - 0x21628 + off) = g_0066a574;
    }
    {
        int32_t *const vtbl = *reinterpret_cast<int32_t **>(mapwin_vbase - 0x21a6c);
        int32_t const off = vtbl[1];
        *reinterpret_cast<int32_t *>(mapwin_vbase - 0x21a70 + off) = off - 0x21a6c;
    }

    reinterpret_cast<MapWin *>(mapwin_vbase)->clear(0);

    VectorDtorIterator(mapwin_vbase - 0x2b18, 0xabc, 4, reinterpret_cast<const void *>(0x00625310));

    reinterpret_cast<Font *>(mapwin_vbase - 0x2b4c)->~Font();
    reinterpret_cast<Font *>(mapwin_vbase - 0x2b78)->~Font();
    reinterpret_cast<Font *>(mapwin_vbase - 0x2ba4)->~Font();
    reinterpret_cast<Buffer *>(mapwin_vbase - 0x3134)->~Buffer();
    reinterpret_cast<Buffer *>(mapwin_vbase - 0x36bc)->~Buffer();
    reinterpret_cast<Buffer *>(mapwin_vbase - 0x3c44)->~Buffer();
    reinterpret_cast<TextureStore *>(mapwin_vbase - 0x3f60)->~TextureStore();

    VectorDtorIterator(mapwin_vbase - 0x210e0, 0x260, 0xc4, reinterpret_cast<const void *>(0x006252B0));
    VectorDtorIterator(mapwin_vbase - 0x21a60, 0x260, 4, reinterpret_cast<const void *>(0x006252B0));
}

/*
Purpose: Open the shared preferences window to the preferences page.
// ORIGINAL: 0x00514EF0 ?set_preferences@Console@@QAEXXZ 0x00514EF0-0x00514EFD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_preferences() {
    ConsolePrefWin->PrefWin::display(0);
}

/*
Purpose: Open the shared preferences window to the automation page.
// ORIGINAL: 0x00514F10 ?set_auto_preferences@Console@@QAEXXZ 0x00514F10-0x00514F1D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_auto_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_auto_preferences() {
    ConsolePrefWin->PrefWin::display(3);
}

/*
Purpose: Open the shared preferences window to the base page.
// ORIGINAL: 0x00514F20 ?set_base_preferences@Console@@QAEXXZ 0x00514F20-0x00514F2D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_base_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_base_preferences() {
    ConsolePrefWin->PrefWin::display(2);
}

/*
Purpose: Open the shared preferences window to the audiovisual page.
// ORIGINAL: 0x00514F30 ?set_audiovisual@Console@@QAEXXZ 0x00514F30-0x00514F3D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_audiovisual@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_audiovisual() {
    ConsolePrefWin->PrefWin::display(4);
}

/*
Purpose: Open the shared preferences window to the map-display page.
// ORIGINAL: 0x00514F40 ?set_map_display@Console@@QAEXXZ 0x00514F40-0x00514F4D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_map_display@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_map_display() {
    ConsolePrefWin->PrefWin::display(5);
}

void __fastcall console_set_preferences_redirect(Console *self, void *) {
    self->set_preferences();
}

void __fastcall console_set_auto_preferences_redirect(Console *self, void *) {
    self->set_auto_preferences();
}

void __fastcall console_set_base_preferences_redirect(Console *self, void *) {
    self->set_base_preferences();
}

void __fastcall console_set_audiovisual_redirect(Console *self, void *) {
    self->set_audiovisual();
}

void __fastcall console_set_map_display_redirect(Console *self, void *) {
    self->set_map_display();
}


/*
Purpose: Clear the console's active-group field and drop the highlight bit
         (0x08000000) from every entry of the group table, at its 0x34 stride.
// ORIGINAL: 0x0050F650 ?clear_group@Console@@QAEXXZ 0x0050F650-0x0050F67B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?clear_group@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
Verification note: the sweep's surviving swaps reorder the object-field clear
against the group-count load and one declaration against another; both pairs
touch disjoint state, so the order is unobservable and the mutants are
equivalent by construction.
*/
void Console::clear_group() {
    const int32_t zero = 0;
    std::memcpy(reinterpret_cast<uint8_t *>(this) + 0x23D1C, &zero, sizeof(zero));
    const int32_t count = *ConsoleGroupCount;
    for (int32_t index = 0; index < count; ++index) {
        uint8_t *const entry = ConsoleGroupTable + index * 0x34;
        uint32_t value;
        std::memcpy(&value, entry, sizeof(value));
        value &= 0xF7FFFFFFu;
        std::memcpy(entry, &value, sizeof(value));
    }
}

void __fastcall console_clear_group_redirect(Console *self, void *) {
    self->clear_group();
}

/*
Purpose: Report whether editing is locked out. Only meaningful in the scenario
         editor; there editing is locked whenever Scroll Lock is toggled on or
         the game is in editor-only mode.
// ORIGINAL: 0x004E1F40 ?edit_lock@Console@@QAEHXZ 0x004E1F40-0x004E1F70 BYTE_EXACT
// size      48 bytes
// prototype int (__thiscall ?edit_lock@Console@@QAEHXZ)(Console* this)
// callers   8   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004E1F51
Return Value: 1 when editing is locked, 0 otherwise
Status: Complete
*/
int Console::edit_lock() {
    if (!(GameState & STATE_SCENARIO_EDITOR)) {
        return 0;
    }
    if (((ConsoleEditKeyStateSlot())(VK_SCROLL) & 1) ||
        (GameState & STATE_EDITOR_ONLY_MODE)) {
        return 1;
    }
    return 0;
}

int __fastcall console_edit_lock_redirect(Console *self, void *) {
    return self->edit_lock();
}

/*
Purpose: Open the shared preferences window to the advanced page.
// ORIGINAL: 0x00514F00 ?set_adv_preferences@Console@@QAEXXZ 0x00514F00-0x00514F0D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_adv_preferences@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0048FA00
Return Value: n/a
Status: Complete
*/
void Console::set_adv_preferences() {
    ConsolePrefWin->PrefWin::display(1);
}

/*
Purpose: Undo the last scenario-editor change, by asking load_undo for the
         editor's undo slot.
// ORIGINAL: 0x004E1F20 ?editor_undo@Console@@QAEXXZ 0x004E1F20-0x004E1F29 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_undo@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ABE40
Return Value: n/a
Status: Complete
*/
void Console::editor_undo() {
    load_undo(1);
}

void __fastcall console_set_adv_preferences_redirect(Console *self, void *) {
    self->set_adv_preferences();
}

void __fastcall console_editor_undo_redirect(Console *self, void *) {
    self->editor_undo();
}


/*
Purpose: Refresh everything the console shows after a selection or turn change:
         hand the change code to the shared InfoWin, redraw the shared
         StatusWin, then push the map's caption into the main interface.
// ORIGINAL: 0x00514880 ?update_data@Console@@QAEXH@Z 0x00514880-0x005148AA BYTE_EXACT
// size      42 bytes
// prototype void (__thiscall ?update_data@Console@@QAEXH@Z)(Console* this, int)
// callers   33   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458900 0x0046FB10 0x004B9EA0
Return Value: n/a
Status: Complete with a temporary StatusWin::redraw original dependency
Verification note: three properties of this body are unobservable at every
oracle tier, because the original's own callees discard them rather than
because the fixtures are weak.
  1. InfoWin::change at 0x00458900 is a bare `ret 4` - it drops both its
     `this` and its argument - so neither the call nor the value of a1 has any
     effect to observe. src/infowin.cpp models it as an empty body.
  2. MapWin::main_caption at 0x0046FB10 overwrites ecx before using it, so the
     MapWin pointer loaded out of ConsoleMapWinSlot never reaches anything
     that reads it.
  3. The original leaves EAX holding main_caption's residue, whose chain is
     already terminated by the committed `void MapWin::main_caption()`, so
     there is no residue available to return and fabricating a constant would
     be wrong. Kept `void`, matching the mangled name.
Mutation-harness survivors for the dropped `change` call and for a perturbed
a1 are therefore equivalent by construction.
*/
void Console::update_data(int a1) {
    // 0x00514883 mov eax,[ebp+8] / 0x00514886 mov ecx,0x7ad2a0 /
    // 0x0051488B push eax / 0x0051488C call 0x458900.
    reinterpret_cast<InfoWin *>(ConsoleInfoWin)->change(a1);
    // 0x00514891 mov ecx,0x8c5568 / 0x00514896 call 0x4b9ea0.
    ConsoleStatusWin->StatusWin::redraw();
    // 0x0051489B mov ecx,[0x7d3c3c] - a load, not an address-of - then
    // 0x005148A1 call 0x46fb10. The slot is read here, never cached.
    console_map_win()->main_caption();
}

// `ret 4` on the original: one 4-byte stack argument plus the ecx `this`. The
// entry needs no this-adjustment - update_data is entered on the Console
// itself, not through a virtual-base adjustor.
void __fastcall console_update_data_redirect(Console *self, void *, int a1) {
    self->update_data(a1);
}

int CursorLastIndex;    // 0x009392B8
int CursorLastX[32];    // 0x009392C0
int CursorLastY[32];    // 0x00939340

/*
Purpose: Report the cursor's new tile position to the status bar and record
         it in this Console's own ring buffer, unless it is the same tile
         already at the head of the cache. In debug mode, warn when the two
         coordinates have mismatched parity, then normalise them to matching
         parity before anything else runs.
// ORIGINAL: 0x005109B0 ?cursor_next@Console@@QAEXHH@Z 0x005109B0-0x00510A59 BYTE_EXACT
// size      169 bytes
// prototype void (__thiscall ?cursor_next@Console@@QAEXHH@Z)(Console* this, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00517C50 0x00644C40 0x0064B330
Return Value: n/a
Status: Complete
*/
void Console::cursor_next(int x_coord, int y_coord) {
    if (((x_coord ^ y_coord) & 1) != 0) {
        if ((GameState & STATE_DEBUG_MODE) != 0) {
            danger(CursorNextDangerMsg1, CursorNextDangerMsg2, x_coord,
                  y_coord, 0);
        }
        if ((x_coord & 1) != 0) {
            x_coord -= 1;
        }
        if ((y_coord & 1) != 0) {
            x_coord += 1;
        }
    }
    ConsoleStatusWin->set_loc(x_coord, y_coord);
    int idx = CursorLastIndex;
    if (x_coord == CursorLastX[idx]) {
        if (y_coord == CursorLastY[idx]) {
            return;
        }
    }
    int newIdx = (static_cast<int32_t>(field_23C0C_) + 1) % 32;
    field_23C0C_ = static_cast<uint32_t>(newIdx);
    field_23C08_ = static_cast<uint32_t>(newIdx);
    reinterpret_cast<int32_t *>(field_23C10_)[newIdx] = x_coord;
    int reIdx = static_cast<int32_t>(field_23C0C_);
    reinterpret_cast<int32_t *>(field_23C10_)[32 + reIdx] = y_coord;
}

/*
Purpose: Point the map windows at one tile on behalf of one faction. Build a
         view-tag mask from the faction argument, walk the eight map-window
         slots, and hand the coordinate to every window whose tag the mask
         selects. The primary window, slot 0, is handled quite differently from
         the rest, and only its success counts: when it moves, the queued input
         is flushed and 1 is returned.
// ORIGINAL: 0x005108A0 ?focus@Console@@QAEXHHH@Z 0x005108A0-0x005109A3 BYTE_EXACT
// LEVER: four stacked fixes took this from 32 of 85 to BYTE_EXACT, each measured on its own. (1) THE TAG TEST IS THE OUTER ONE. `if (slot == 0) { if (!tagged ...) } else if (!tagged)` makes the tag survive the slot branch, so VC6 materialises it (`and` plus `setne` plus a second `test`) where the image has one `test ecx, edx` at 0x00510908 with two consumers; the chained guard `(mask & flags) == 0 && (slot != 0 || ctrl == 0)` produces the image's own tag-then-slot-then-ctrl order. 32 -> 79/85, and 81/85 written as the single chained guard rather than two nested ifs.
// LEVER: (2) `volatile` ON THE OVERLAY LATCH was the last four instructions. The image hoists `push edi; push ebx` - MapWin::focus's arguments - ABOVE the `mov dword ptr [eax + 0x23c00], 0` at 0x0051093C, and a volatile store cannot be reordered against them. Dropping it is 81/85 -> 85/85; the store survives because a call it cannot see through follows immediately. Ruled out at the same time: moving the `MapWinTable[0]` load ahead of the clear scores 80/85 either way.
// LEVER: (3) `int slot`, not `size_t`: the loop's back edge is `jl 0x5108de` at 0x00510985, and an unsigned induction variable compiles `jb`. `MapWinTableSlots` is a `size_t`, so the bound needs the cast or the comparison promotes back to unsigned.
// LEVER: (4) the repaint went through `MapWinOriginalDrawMap`, a pointer, and compiled `call dword ptr [...]` where 0x0051095D is a `call rel32`. `MapWin::draw_map` is declared in mapwin.h and defined at the end of mapwin.cpp, so naming it directly costs nothing and emits the E8.
// symbol    ?focus@Console@@QAEHHHH@Z
// size      259 bytes
// prototype void (__thiscall ?focus@Console@@QAEXHHH@Z)(Console* this, int xCoord, int yCoord, int factionID)
// callers   32   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550 0x0046B310 0x005109B0 0x005FD120
Return Value: 1 when the primary map window took the coordinate, 0 otherwise
Status: Complete with temporary Console::cursor_next, MapWin::focus,
        MapWin::draw_map and flush_input original dependencies
Verification note: the catalogued mangled name ?focus@Console@@QAEXHHH@Z says
        void and is wrong. EAX is loaded from the [ebp-4] flag slot at
        0x0051098B and 0x0051099A, and 12 of the 61 call sites branch on it two
        instructions later - 0x004F7D7D, 0x00508838 and 0x0050D606 among them.
        Declared int; do not "fix" it back on the strength of the name.
Verification note: slot 0 is special in FOUR distinct ways, each transcribed at
        the address that decides it.
          1. 0x005108ED / 0x005108EF - slot 0 skips the per-window activity gate
             at 0x1DD74 entirely, the same exemption draw_tile makes.
          2. 0x0051090C / 0x0051090E - an UNTAGGED slot 1..7 is dropped, while
             an untagged slot 0 falls through to 0x00510910 and gets a second
             chance from the control-turn global.
          3. 0x0051091B / 0x0051091D - a TAGGED slot 1..7 goes straight to the
             plain focus call at 0x00510964, while a tagged slot 0 runs the
             console cursor and the survey-overlay latch first.
          4. 0x00510976 / 0x00510978 - a successful focus on slots 1..7 does NOT
             raise the flag. Only slot 0 can set it, so only slot 0 can reach
             flush_input or make this function return 1.
Verification note: the reload of the flag slot at 0x0051099A, after flush_input
        returns, is a register-clobber reload of a local this frame owns.
        flush_input is a cdecl no-argument function and cannot touch it, so the
        reload is not observable and is transcribed as one `return focused;`.
Verification note: the mask is computed once at 0x005108C1-0x005108D5 and then
        reloaded from the [ebp+0x10] argument slot at 0x005108FF on every
        iteration, and `this` is reloaded from [ebp-8] at 0x0051092B. Nothing
        this body calls can write a caller-pushed argument slot or another
        frame's local, so both are register-allocation artifacts and the single
        locals below are faithful. The MapWinTable reloads are NOT in that
        class and are kept - see their own comments.
Verification note: the original has no null check other than the slot test at
        0x005108E5. It does not defend against a null `this`, a null
        ConsoleGlobal, a null seam, or a null MapWinTable[0] at 0x00510946 and
        0x00510955 - the last two genuinely reachable if a callee empties the
        slot mid-iteration, and the original would fault. That absence is
        deliberate and is not repaired here.
*/
int Console::focus(int x_coord, int y_coord, int faction_id) {
    // 0x005108AD `cmp eax, esi` against an already-zeroed ESI, then 0x005108B6
    // `jge`: a negative faction takes 0x60000000 and matches both view tags at
    // once. Otherwise the neg/sbb/and/add idiom at 0x005108C1-0x005108D5 folds
    // one equality into a constant, giving 0x40000000 for a different faction
    // and 0x20000000 for the local one. LocalFaction is read only on the
    // non-negative path, exactly as the original's branch structure has it.
    // 0x005108B3 `mov [ebp-4], esi` with ESI zero. The literal 1 at 0x0051097A
    // is the ONLY store into that slot, so this is a plain 0/1 flag and the
    // returns are exactly 0 and 1, never a callee's residue.
    int focused = 0;
    uint32_t mask;
    if (faction_id < 0) {
        mask = 0x60000000U;
    } else {
        mask = (faction_id != LocalFaction) ? 0x40000000U : 0x20000000U;
    }
    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        // 0x005108DE `mov eax, [esi*4 + 0x7d3c3c]`. Re-read every iteration,
        // do NOT hoist: cursor_next, MapWin::focus and MapWin::draw_map below
        // can republish the table, and later slots must see that.
        MapWin *const window = MapWinTable[slot];
        // 0x005108E5 test / 0x005108E7 je. The null test runs FIRST, so an
        // empty slot 0 is skipped despite every exemption below.
        if (window == nullptr) {
            continue;
        }
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x00510902 reads the window's wide flag dword, one dword ahead of the
        // activity field mapwin.h already names. Kept as a documented literal
        // rather than a constant in another class's header: MapWin owns the
        // field, and console.h is not the place to name it. 0x00510908
        // `test ecx, edx` is a bitwise AND against the tag bits, not an
        // equality, so a window carrying 0x60000000 matches either mask.
        const uint32_t flags = *reinterpret_cast<const volatile uint32_t *>(
            reinterpret_cast<const uint8_t *>(window) + 0x1DD70);
        // 0x00510908 `test ecx, edx` is ONE test with two consumers: the
        // image branches on the tag FIRST and only then on the slot, which is
        // this chained guard and not an `if (slot == 0)` outer test. Written
        // the other way round the tag has to survive the slot branch, so VC6
        // materialises it (`and`/`setne` plus a second `test`) and the whole
        // tail shifts - 32 of 85 against 81 of 85 here.
        if ((mask & flags) == 0
            && (slot != 0 || *ConsoleControlTurnActive == 0)) {
            continue;
        }
        if (slot == 0) {
            // Specialisation 3. cursor_next runs on the process-wide Console at
            // 0x009156B0, NOT on `this`. Every call site happens to enter focus
            // with the same object, but the constant is in the instruction
            // stream and is transcribed as one.
            ConsoleGlobal->cursor_next(x_coord, y_coord);
            // 0x0051092B reloads `this`, 0x0051092E reads the survey-overlay
            // latch. Read AFTER the call, do NOT hoist: this comes from `this`
            // while cursor_next was handed ConsoleGlobal, and cursor_next
            // writes its own object's fields in this region. Held as a raw
            // offset into the unmapped derived storage, the way clear_group
            // holds 0x23D1C.
            // NOT `volatile`. The image hoists the two argument pushes for
            // MapWin::focus ABOVE this store (`push edi; push ebx` at
            // 0x0051093A, then the clear at 0x0051093C); a volatile store
            // cannot be reordered against them, which pinned it four
            // instructions early and cost the match at 81 of 85. The store
            // still survives - it is followed by a call VC6 cannot see
            // through - and the read below still cannot be hoisted over
            // cursor_next for the same reason.
            uint32_t *const overlay =
                reinterpret_cast<uint32_t *>(
                    reinterpret_cast<uint8_t *>(this) + 0x23C00);
            if (*overlay != 0) {
                // 0x0051093C clears the latch BEFORE the table load at
                // 0x00510946 and before the call at 0x0051094C. The clear is
                // ordered against MapWin::focus, not folded into it.
                *overlay = 0;
                // 0x00510946 reads slot 0 by absolute address, with no [esi*4]
                // index, and as a fresh load. Do NOT reuse `window`:
                // cursor_next may have republished the table.
                MapWin *const primary = MapWinTable[0];
                if (primary->MapWin::focus(x_coord, y_coord) != 0) {
                    focused = 1;
                    continue;
                }
                // 0x00510955 reads slot 0 a SECOND time, after MapWin::focus
                // returned. Re-read, do NOT hoist and do not share with the
                // load above. The literal 1 is the draw type.
                MapWin *const repaint = MapWinTable[0];
                repaint->MapWin::draw_map(1);
                continue;
            }
            // Latch already clear (0x00510936 je 0x510964): fall through to the
            // plain focus call, exactly as a tagged slot 1..7 does.
        }
        // 0x00510964 - the third table read this iteration can make. Re-read,
        // do NOT hoist: on the slot-0 path cursor_next has already run between
        // this and the load at 0x005108DE.
        MapWin *const target = MapWinTable[slot];
        // Specialisation 4: a successful focus on slots 1..7 is discarded; only
        // the primary window raises the flag.
        if (target->MapWin::focus(x_coord, y_coord) != 0
            && slot == 0) {
            focused = 1;
        }
    }
    // 0x00510991 test / 0x00510993 je / 0x00510995 call - once, after the whole
    // eight-slot walk, never inside it.
    if (focused != 0) {
        flush_input();
    }
    return focused;
}

int __fastcall console_focus_redirect(Console *self, void *, int x_coord,
                                      int y_coord, int faction_id) {
    return self->focus(x_coord, y_coord, faction_id);
}

/*
Purpose: Drain the keyboard queue, clear the two pointer-owner slots and let
         the network layer poll, then do the same for the mouse queue - each
         PeekMessage loop PEELED, one check outside before the do-while, as
         the image has it.
// ORIGINAL: 0x005FD120 ?flush_input@@YAXXZ 0x005FD120-0x005FD1BA BYTE_EXACT
// size      154 bytes
// prototype void (__cdecl ?flush_input@@YAXXZ)()
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5D0
// indirect  0x005FD13F 0x005FD157 0x005FD180 0x005FD198
Return Value: n/a
Status: Complete
*/
void __cdecl flush_input() {
    MSG msg;
    if (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
        do {
        } while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                             PM_REMOVE));
    }
    WinPointerOwner1 = nullptr;
    WinPointerOwner2 = nullptr;
    check_net();
    check_net();
}

/*
Purpose: Record that the console was closed from the system menu, by raising
         the flag the turn loop watches.
// ORIGINAL: 0x0051D7C0 ?on_sys_close@Console@@QAEXXZ 0x0051D7C0-0x0051D7CB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?on_sys_close@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Console::on_sys_close() {
    *ConsoleExitTurnLoop = 1;
}

/*
// ORIGINAL: 0x004E0FA0 ?editor_climate@Console@@QAEXXZ 0x004E0FA0-0x004E0FC3 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?editor_climate@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0058C2A0 0x005ABF20 0x005C5A30
Status: Complete
*/
void Console::editor_climate() {
    auto_undo();
    if (!custom_planet(0, 0)) {
        world_climate();
        // QUALIFIED: `Console : MapWin` inherits `MapWin::draw_map`, so the
        // unqualified name binds to the member and compiles a `__thiscall`
        // where the image has a `__cdecl` call to the FREE draw_map at
        // 0x0046B190. The `::` is what makes the two the same call.
        ::draw_map(1);
    }
}

/*
// ORIGINAL: 0x004E1F30 ?editor_redo@Console@@QAEXXZ 0x004E1F30-0x004E1F39 BYTE_EXACT
// size      9 bytes
// prototype void (__thiscall ?editor_redo@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005ABE40
Status: Complete
*/
void Console::editor_redo() {
    load_undo(-1);
}

/*
// ORIGINAL: 0x0051D740 ?menu_update@Console@@QAEXXZ 0x0051D740-0x0051D753 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?menu_update@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00460DD0
Status: Complete
*/
void Console::menu_update() {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x23bdc);
    reinterpret_cast<MainMenu *>(self + 0x22a2c)->MainMenu::check(v);
}

/*
// ORIGINAL: 0x004E0F80 ?editor_polar@Console@@QAEXXZ 0x004E0F80-0x004E0F98 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?editor_polar@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x005ABF20 0x005C58E0 0x005C5A30
Status: Complete
*/
void Console::editor_polar() {
    auto_undo();
    world_polar_caps();
    world_climate();
    ::draw_map(1);   // the FREE draw_map; see Console::editor_climate above
}

/*
Purpose: Reset the console - clear the message ring, the two 0x20-entry tables,
         and the trailing state, then close the Buffer, Time and GraphicWin
         subobjects and clear the map.
// ORIGINAL: 0x0051D7D0 ?close@Console@@QAEXXZ 0x0051D7D0-0x0051D88E BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0051d7d0.cpp, whose claim proved the ARTIFACT reproduced while the program held a pending_bodies forwarder. `void close()` was not declared on Console at all - that, not the body, is what kept it in an artifact.
// symbol    ?close@Console@@QAEXXZ
// size      190 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
void Console::close() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<uint32_t *>(self + 0x23bc8) = 0xffffffff;
    *reinterpret_cast<uint32_t *>(self + 0x23bd8) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23be4) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23be8) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23bf0) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23bf4) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23c00) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23c04) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23bfc) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23bf8) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23c08) = 0;
    int count = 0x20;
    do {
        count -= 1;
        uint32_t idx1 = *reinterpret_cast<uint32_t *>(self + 0x23c08);
        *reinterpret_cast<uint32_t *>(self + 0x23c10 + idx1 * 4) = 0;
        uint32_t idx2 = *reinterpret_cast<uint32_t *>(self + 0x23c08);
        *reinterpret_cast<uint32_t *>(self + 0x23c90 + idx2 * 4) = 0;
    } while (count != 0);
    *reinterpret_cast<uint32_t *>(self + 0x23d10) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23d18) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23d88) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23d80) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23d8c) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x23d84) = 0;
    reinterpret_cast<Buffer *>(self + 0x2247c)->close();
    reinterpret_cast<Time *>(self + 0x22a04)->close();
    reinterpret_cast<GraphicWin *>(self + 0x22a2c)->close();
    clear(0);
}
