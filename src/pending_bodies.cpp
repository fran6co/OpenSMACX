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
#include "alphasave.h"
#include "caviar.h"
#include "font.h"
#include "game.h"
#include "general.h"
#include "palette.h"
#include "basepop.h"
#include "buffer.h"
#include "checkbox.h"
#include "checkbutton.h"
#include "dialog.h"
#include "stringstruct.h"
#include "spritebox.h"
#include "dialogs.h"
#include "listbox.h"
#include "tutwin.h"
#include "deletionlist.h"
#include "maininterface.h"
#include "basewin.h"
#include "cursor.h"
#include "filewin.h"
#include "radiobutton.h"
#include "sprite.h"
#include "time.h"
#include "win.h"
#include "map.h"
#include "mapwin.h"
#include "popup.h"
#include "sound.h"
#include "netdaemon.h"
#include "alphanet.h"
#include "deletionlist.h"
#include "lock.h"
#include "fx.h"
#include "hypothesis_layouts.h"
#include "famewin.h"
#include "statuswin.h"
#include "console.h"
#include "buttongroup.h"
#include "datalink.h"
#include "gamma.h"
#include "texture.h"
#include "infowin.h"
#include "stringbox.h"
#include "worldwin.h"
#include "basebutton.h"
#include "dipedit.h"
#include "designwin.h"
#include "councwin.h"
#include "graphicwin.h"
#include "sounddevice.h"
#include "menu.h"
#include "reportwin.h"
#include "reportif.h"
#include "setupwin.h"
#include "scroll.h"
#include "replaywin.h"
#include "net_class.h"
#include "temp.h"
#include "alphamenu.h"
#include "alphamovie.h"
#include "credits.h"
#include "diplopop.h"
#include "diplowin.h"
#include "netmsg.h"
#include "netwin.h"
#include "newtechwin.h"
#include "pickwin.h"
#include "popmenu.h"
#include "pulldown.h"
#include "prodpicker.h"
#include "pushbutton.h"
#include "socialwin.h"
#include "editbox.h"
#include "wave.h"
#include "wave_device.h"
#include "editgroup.h"
#include "planwin.h"
#include "uv2player.h"
#include "vector_teardown.h"
#include "xpops.h"

/*
 * THE FRONTIER.
 *
 * Every function the compiled tree CALLS BY NAME but has not promoted into
 * the build yet gets one forwarder here, aimed at its address in the shipped
 * image. Nothing else belongs in this file.
 *
 * WHY IT EXISTS. Adding the recovered `WinMain` to the link added six
 * undefined symbols, and their transitive closure is 1,489 catalogued
 * addresses - the whole remaining recovery, arriving in one link error. The
 * alternative to a forwarder was to keep spelling callees as raw addresses at
 * the call site, the way the injected DLL did:
 *
 *     typedef int(__cdecl fn)(Palette *, Font *, LPSTR, int, int, int, int);
 *     ((fn *)0x0062D3A0)(&g_PALETTE1, &g_JACKAL_FONT, ...);
 *
 * which compiles a `call dword ptr [...]` where the image has `call rel32`,
 * and leaves the caller reading as finished when it is not. A forwarder keeps
 * the call site honest C++, emits the `E8` the image emits, and puts every
 * not-yet-promoted edge in ONE place that can be counted.
 *
 * HOW ONE LEAVES. When a body lands in a compiled unit, DELETE its forwarder,
 * AND REPOINT ITS CALLERS.
 *
 * This used to claim the linker enforced that - "two definitions of one
 * symbol is LNK2005, so a stale forwarder cannot survive the promotion it
 * was waiting for" - and that was false, in the way that matters. A
 * forwarder is a FREE FUNCTION (`?win_close_class@@YAXXZ`); the promotion is
 * a MEMBER (`?close_class@Win@@QAAXXZ`). Different mangled names. Nothing
 * collides, nothing fires, and both definitions link happily side by side
 * while every caller still routed through the forwarder jumps into a raw
 * image address and faults - with the real body sitting in the same binary.
 * `jackal_close` did exactly that for Win and Palette until 2026-08-24.
 *
 * WHAT ENFORCES IT NOW is `tools/address_index.py`, gated: it reports a
 * LANDMINE for any address whose claim is COMPILED while a forwarder still
 * defines it, and that floor is a hard count that may only fall. It also
 * distinguishes the case that looks identical and must NOT be touched - a
 * claim living only in src/recovered/ or src/unrecovered/, which the build
 * does not compile, so the forwarder is the ONLY definition and deleting it
 * breaks the link. Conflating those two is why a floor of nine sat
 * untouched: five of them could not be fixed the way the others had to be.
 *
 * WHAT THEY DO AT RUNTIME. Jump into an address that means nothing in this
 * process, and fault. That is deliberate: a forwarder that returned a
 * plausible success would let the startup sequence run past the first thing
 * that is missing, and the point of an entry point is to say exactly how far
 * the recovery reaches.
 */

#define PENDING_BODY(address, signature) \
    reinterpret_cast<signature>(static_cast<unsigned long>(address))

// ??_M@YGXPAXIHP6EX0@Z@Z at 0x006456E4 and ??_L@YGXPAXIHP6EX0@Z1@Z at
// 0x006457C2 - the CRT's vector destructor and constructor iterators, reached
// by 204 array-thunk call sites. They were bound as `*const` pointers, which
// REAL NOW, not seams: the image's two iterators are plain loops - the
// constructor walk front to back (0x006457C2), the destructor walk back to
// front, last element first (0x006456E4) - and jumping into the original for
// them is what crashed the standalone build at startup. The element callback
// is __fastcall: the element arrives in ECX, thiscall's own shape.
void __stdcall VectorDtorIterator(void *array, unsigned int element_size, int count,
                                  func_vector_element_callback teardown) {
    char *element = static_cast<char *>(array) + element_size * count;
    for (int index = 0; index < count; ++index) {
        element -= element_size;
        teardown(element);
    }
}

void __stdcall VectorCtorIterator(void *array, unsigned int element_size, int count,
                                  func_vector_element_callback ctor,
                                  func_vector_element_callback dtor) {
    char *element = static_cast<char *>(array);
    for (int index = 0; index < count; ++index) {
        ctor(element);
        element += element_size;
    }
}

// ?do_video@@YAXXZ at 0x00636300 - a message-loop pump. See the note in
// `temp.h`: it was bound as a pointer, which cost every site the image's `E8`.
//
// `do_net` and `check_net` WERE HERE and are gone: both are promoted into
// `net_class.cpp` now. The linker is what enforced that - LNK2005, two
// definitions of one symbol - exactly as the note at the top of this file
// says it would.
void __cdecl do_video() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00636300, pending)();
}

// ?process_voice@Net@@QAEXXZ at 0x00631A60 and ?check_polling@Net@@QAEXXZ at
// 0x006320E0 are in src/net_class.cpp now, with bodies - as is drop_player
// (0x00632350), whose forwarder lives beside its caller check_polling there.

// Six callees `temp.h` bound as pointers. Each one cost its callers the image's
// `E8`; see the note there. wave_it's forwarder is gone - the body is homed
// in popup.cpp (2026-09-06), and free functions have no class for
// home_bodies.py to clear the forwarder against, so it came out by hand.

// ?terrain_fixup@@YAXPAUMapWin@@@Z at 0x004711A0 - mapwin_terrain_fixup
// (src/mapwin.cpp) calls it BY NAME, once per live MapWin slot and once
// more for each of three fixed-address windows.


void __cdecl world_rainfall() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005C4470, pending)();
}

// Four Time callbacks `temp.h` bound as pointers - start_timers (time.cpp)
// calls each by name now, reaching the image's `push 0x...` immediate
// instead of loading the pointer from memory.
// blink2_timer, line_timer and turn_timer WERE HERE, forwarding to
// 0x0050EE30, 0x0050EE80 and 0x0050EF10. Homed into time.cpp 2026-08-29;
// blink_timer's forwarder below remains.

void __cdecl blink_timer(int a1) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x0050EA40, pending)(a1);
}

void __cdecl social_set(uint32_t faction_id) {
    typedef void(__cdecl *pending)(uint32_t);
    PENDING_BODY(0x005B4600, pending)(faction_id);
}

void __cdecl consider_designs(uint32_t faction_id) {
    typedef void(__cdecl *pending)(uint32_t);
    PENDING_BODY(0x00581260, pending)(faction_id);
}

// `Win::~Win` WAS HERE, forwarding to 0x005EBC90. Homed into `win.cpp`
// 2026-08-29; the linker (LNK2005) enforced it exactly as the note at the
// top of this file says it would.

// sub_4c86d0 at 0x004C86D0 - the teardown the Video deleting thunks call. See
// the note in `deleting_thunks.h`. (sub_633010 at 0x00633010, the teardown
// the 0x633160 deleting thunk calls, WAS HERE and is gone: its body is homed
// into src/leaf_recoveries.cpp 2026-09-27.)
void __fastcall sub_4c86d0(void *self) {
    typedef void(__fastcall *pending)(void *);
    PENDING_BODY(0x004C86D0, pending)(self);
}

// ?clear@MapWin@@QAEXH@Z at 0x00462870 - HOMED into src/mapwin.cpp
// 2026-09-04, so this file no longer carries it. MapWin's own
// destructor and PlanWin's/Console's each reach it directly.


// ?close@FileWin@@QAEXXZ at 0x00613900 WAS HERE: the body is homed into
// src/filewin.cpp 2026-09-27 (wave-5 sweep), beside ~FileWin which calls
// it first. No caller repointing was needed - the member's mangled name is
// the one the forwarder defined.


// ?close@UV2Player@@QAEXXZ at 0x004BF400 - reached by ~UV2Player. See the
// note in uv2player.h.
void UV2Player::close() {
    typedef void(__fastcall *pending)(UV2Player *, void *);
    PENDING_BODY(0x004BF400, pending)(this, nullptr);
}

// ?close@SpriteBox@@QAEXXZ at 0x00610280 already has a forwarder below
// (added when the on_* thunks were promoted); ~SpriteBox reaches it there.

// The game's own `operator new` at 0x0064558A, reached from `Wave::init` and
// `Wave::load`. See the note in `vector_teardown.h`.
void *__cdecl WaveOperatorNew(unsigned int size) {
    typedef void *(__cdecl *pending)(unsigned int);
    return PENDING_BODY(0x0064558A, pending)(size);
}

// ?set_text@EditBox@@QAEXPAD@Z at 0x006151E0 - WAS HERE and is gone: the body
// is homed into src/leaf_recoveries.cpp 2026-09-27, and EditGroup::set_text
// reaches it by name again. See the note in `editbox.h`.

// ?message_data@@YAXHHHHHH@Z at 0x00592EE0 - broadcasts a game event. It
// cannot be homed into netdaemon.cpp beside its family: the catalogue spells
// the return void, but EAX is live at its ret and unlock_veh (BYTE_EXACT)
// returns that residue, and no body spelled in this tree can pass a callee's
// EAX through - so the forwarder, which is the one spelling that can, stays.
uint32_t __cdecl message_data(int a1, int a2, int a3, int a4, int a5, int a6) {
    typedef uint32_t(__cdecl *pending)(int, int, int, int, int, int);
    return PENDING_BODY(0x00592EE0, pending)(a1, a2, a3, a4, a5, a6);
}

// current_server at 0x0052DBA0 - homed into game.cpp 2026-09-04 with all five
// of its fixed-address bindings renamed to the tree's own models. Its
// forwarder went with it.

void MapWin::click(int a1, int a2, int a3) {  // 0x0046D5D0
    typedef void(__fastcall *pending)(MapWin *, void *, int, int, int);
    PENDING_BODY(0x0046D5D0, pending)(this, nullptr, a1, a2, a3);
}

// sub_638430 at 0x00638430 - the helper that walks a Caviar record and frees
// its members - is HOMED to caviar.cpp as caviar_free_record, which is why
// this forwarder is gone rather than merely stale.

// ?render@Caviar@@QAEHPAUBuffer@@HHH@Z at 0x00617230 is HOMED into
// src/caviar.cpp (2026-09-05, caviar class pass), so this forwarder is gone.

// ?init@Sprite@@QAEXPADHH@Z at 0x005E3890 - WAS HERE and is gone: the body is
// homed into src/sprite.cpp 2026-09-27. BasePop::start (0x00601BF0) calls it
// BY NAME on the popup's own `sprite_` for the "sprite" directive, so the
// call site keeps the member's real address and the image's `E8`.
// Remaining specialized Sprite workers, called by their scale wrappers. Plain
// draw, draw_dest and - since 2026-09-27 - draw_dest_unk2 (0x005E81BC) and
// draw_multi_table_dest (0x005E8FF5) live in sprite.cpp.

// Two __cdecl free functions the editor commands call, still original
// bodies. Declared at the top of console.cpp beside the bodies that reach
// them; their natural header home is the class pass that recovers them.
//             all three bodies homed now: veh_kill and veh_ready in
//             src/veh.cpp (the TU their every other caller compiles from),
//             world_rocky in src/map.cpp - the forwarders are gone rather
//             than duplicated beside the real definitions.

// THE PENDING_BODY CEILING: 252 -> 253, this batch. Three forwarders added -
// battle_kill (0x00505D80), battle_fight's seven-parameter overload
// (0x00506AF0) and the add_deletion below - and two retired, veh_kill
// (0x005C08C0) and veh_ready (0x005C1B70), whose bodies the P2 veh homing
// moved into src/veh.cpp. All three new targets are FILE-tier in the archives
// (no claim, so address_index's archive-only floor stays at 0); tools/
// address_index.py --check reports 0 landmines.
// ?battle_kill@@YAHHPAHPAHPAHHH@Z at 0x00505D80, 938 bytes, FILE-tier in
// src/unrecovered/00505d80.cpp. battle_kill_chaff (veh.cpp) is its one caller
// this batch reached; the declaration is local to veh.cpp, so retiring this
// forwarder when the body is homed cannot strand a stale one.
int __cdecl battle_kill(int cur, int *attacker_out, int *defender_out,
                        int *odds_out, int killer_id, int credit_faction) {
    typedef int(__cdecl *pending)(int, int *, int *, int *, int, int);
    return PENDING_BODY(0x00505D80, pending)(cur, attacker_out, defender_out,
                                             odds_out, killer_id,
                                             credit_faction);
}

// The seven-parameter ?battle_fight@@YAHHHHHHPAH@Z at 0x00506AF0, 19,875
// bytes - the largest body in the veh family - still FILE-tier in
// src/unrecovered/00506af0.cpp. Its five-parameter overload is homed in
// veh.cpp and tail-calls it, so this forwarder is what makes that call link.
int __cdecl battle_fight(int veh_id, int dir, int x, int y, int src_mode,
                         int allow_interceptor, int *odds_out) {
    typedef int(__cdecl *pending)(int, int, int, int, int, int, int *);
    return PENDING_BODY(0x00506AF0, pending)(veh_id, dir, x, y, src_mode,
                                             allow_interceptor, odds_out);
}

// ?init@Scroll@@QAEHHHHHPAUWin@@HH@Z at 0x006054D0 - the primary scrollbar
// init every other overload funnels into.
//             body in src/unrecovered/006054d0.cpp
int Scroll::init(int x, int y, int width, int height, Win *parent,
                 int setting, int options) {
    typedef int(__fastcall *pending)(Scroll *, void *, int, int, int, int,
                                     Win *, int, int);
    return PENDING_BODY(0x006054D0, pending)(this, nullptr, x, y, width,
                                             height, parent, setting, options);
}

// 0x0060C5D0 ListBox::on_scrolling and 0x0060CB70 ListBox::on_mousewheel
// WERE HERE (dispatched to by kind from dialogs.cpp): both bodies are homed
// into src/leaf_recoveries.cpp 2026-09-27, with on_scrolling's entered-
// receiver walk re-taken from the image. draw_item (0x0060B530) and UNK2
// (0x0060C7D0) are DECLARED on the class now (listbox.h) for the follow-up
// pass their large/undecompiled seeds still need.

// ?close@BasePop@@QAEXXZ at 0x00600F00 and
// ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z at 0x00406380 - the two
// bodies popup.cpp used to reach through pointers.




// ?sprite@SpriteBox@@QAEHPAUSprite@@PADH@Z at 0x006108E0 - WAS HERE and is
// gone: the body is homed into src/leaf_recoveries.cpp 2026-09-27. popup.cpp's
// `pops()` still calls it BY NAME, so the call site keeps the image's
// `call rel32`.

// ?X_pops@@... at 0x005BF930 and ?X_pop@@... at 0x005BF480 - the two full
// builders every xpops wrapper funnels into.
int __cdecl X_pops(char *caption, const char *label, int a3, char *a4, int a5,
                   Sprite *sprite, int a7, int a8, int (__cdecl *callback)()) {
    typedef int(__cdecl *pending)(char *, const char *, int, char *, int,
                                  Sprite *, int, int, int (__cdecl *)());
    return PENDING_BODY(0x005BF930, pending)(caption, label, a3, a4, a5,
                                             sprite, a7, a8, callback);
}



// Five more bodies the tree called through function pointers. Each is
// declared on its class now, so the call sites emit `call rel32`.
void MainInterface::set_date(char *text) {              // 0x0045BE80
    typedef void(__fastcall *pending)(MainInterface *, void *, char *);
    PENDING_BODY(0x0045BE80, pending)(this, nullptr, text);
}

// ?redraw_complete@MainInterface@@QAEXXZ at 0x0045C3A0. blink2_timer
// (time.cpp) reaches it on the fixed MainInterface at 0x007AE820.
void MainInterface::redraw_complete() {                 // 0x0045C3A0
    typedef void(__fastcall *pending)(MainInterface *, void *);
    PENDING_BODY(0x0045C3A0, pending)(this, nullptr);
}

// ?draw_edit_pane@MainInterface@@QAEXXZ at 0x0045B550. Console's editor
// commands repaint the pane through it after turning an editor flag.
//             body in src/unrecovered/0045b550.cpp
void MainInterface::draw_edit_pane() {                  // 0x0045B550
    typedef void(__fastcall *pending)(MainInterface *, void *);
    PENDING_BODY(0x0045B550, pending)(this, nullptr);
}

// ?tut_map@TutWin@@QAEHPBDHHHPAUSprite@@HHH@Z at 0x004BE650. Console::survey2
// raises the map-square tutorial popup on the fixed TutWin at 0x008C6E68.
//             body in src/unrecovered/004be650.cpp
int TutWin::tut_map(const char *text, int x_coord, int y_coord, int pixel_x,
                    Sprite *sprite, int forwarded_6, int arrow_x,
                    int arrow_y) {                      // 0x004BE650
    typedef int(__fastcall *pending)(TutWin *, void *, const char *, int, int,
                                    int, Sprite *, int, int, int);
    return PENDING_BODY(0x004BE650, pending)(this, nullptr, text, x_coord,
                                             y_coord, pixel_x, sprite,
                                             forwarded_6, arrow_x, arrow_y);
}


int TutWin::tut_win(void *owner, const char *text, int a3, int a4,
                    Sprite *sprite, int a6, int a7, int a8) {  // 0x004BDFE0
    typedef int(__fastcall *pending)(TutWin *, void *, void *,
                                     const char *, int, int, Sprite *, int,
                                     int, int);
    return PENDING_BODY(0x004BDFE0, pending)(this, nullptr, owner, text, a3,
                                             a4, sprite, a6, a7, a8);
}

void BaseWin::click(int a1, int a2, int right, int is_double) {       // 0x004165D0
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x004165D0, pending)(this, nullptr, a1, a2, right, is_double);
}

void BaseWin::iface_click(int a1, int a2, int right, int is_double) {                       // 0x004160F0
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x004160F0, pending)(this, nullptr, a1, a2, right, is_double);
}

void BaseWin::draw_supported(int a1) {                       // 0x0040C850
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    PENDING_BODY(0x0040C850, pending)(this, nullptr, a1);
}

void BaseWin::draw_facilities(int a1) {                       // 0x0040FCC0
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    PENDING_BODY(0x0040FCC0, pending)(this, nullptr, a1);
}

void BaseWin::garrison_click(int vehID, int a2, int right, int is_double) {                       // 0x0040B140
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x0040B140, pending)(this, nullptr, vehID, a2, right, is_double);
}





void NetDaemon::synch(int16_t opcode, int a, int b, int c, char *text, int d, int16_t flags) {  // 0x00532E00
    typedef void(__fastcall *pending)(NetDaemon *, void *, int16_t, int, int, int, char *, int, int16_t);
    PENDING_BODY(0x00532E00, pending)(this, nullptr, opcode, a, b, c, text, d, flags);
}

// ?order_veh@NetDaemon@@QAEHHHH@Z at 0x00531780 - the client-side half of a
// move, still archived (2485 bytes, src/unrecovered/00531780.cpp). Not claimed,
// so this forwarder is honest: it is the only definition and it faults loudly
// if the body is ever reached before that archive is homed. The veh.cpp action
// family (action_go_to, action_road_to) calls it directly, which is what
// forced the declaration onto the class.
int NetDaemon::order_veh(int veh_id, int payload_10, int payload_1C) {
    typedef int(__fastcall *pending)(NetDaemon *, void *, int, int, int);
    return PENDING_BODY(0x00531780, pending)(this, nullptr, veh_id, payload_10,
                                             payload_1C);
}

void PrefWin::display(int page) {  // 0x0048FA00
    typedef void(__fastcall *pending)(PrefWin *, void *, int);
    PENDING_BODY(0x0048FA00, pending)(this, nullptr, page);
}

// ?draw_status@StatusWin@@QAEXHHHHH@Z at 0x004B6570 - 8885 bytes, still an
// original body. StatusWin::redraw (src/statuswin.cpp) calls it BY NAME.


// ButtonGroup::button_click (0x0062B8A0) WAS HERE and is gone: the body is
// homed into src/leaf_recoveries.cpp 2026-09-27.

void Datalink::exec(unsigned int topic, int index) {  // 0x00429180
    typedef void(__fastcall *pending)(Datalink *, void *, unsigned int, int);
    PENDING_BODY(0x00429180, pending)(this, nullptr, topic, index);
}

void StringBox::add_fixup() {  // 0x00629490
    typedef void(__fastcall *pending)(StringBox *, void *);
    PENDING_BODY(0x00629490, pending)(this, nullptr);
}

void WorldWin::click(int a1, int a2, int button, int is_double) {  // 0x004C3D40
    typedef void(__fastcall *pending)(WorldWin *, void *, int, int, int, int);
    PENDING_BODY(0x004C3D40, pending)(this, nullptr, a1, a2, button, is_double);
}

// ?tile_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z (0x005DA6C0) - homed into
// src/buffer.cpp, along with both `tile` overloads (0x005DA860/0x005DA9F0)
// and tile_mask's per-tile blit ?copy_mask@Buffer@@ (0x005E2B00).

// ?set_selected_id@ListBox@@QAEXH@Z - DipEdit::init lands the list
// selection on the row matching its own faction. WAS HERE: the body is
// homed into src/leaf_recoveries.cpp 2026-09-27 (the id walk over the
// Dialog entry list, selecting through current_position_).

// ?item@CheckBox@@QAEHPADHH@Z - DipEdit::init adds one item per treaty flag.
// WAS HERE and is gone: the body is homed into src/leaf_recoveries.cpp
// 2026-09-27.




// Dialog::init(int) at 0x006095F0 WAS HERE and is gone: the body is homed
// into src/leaf_recoveries.cpp 2026-09-27, beside its sibling overloads.

// Dialog::init(Heap *) at 0x006098D0 and Dialog::init(int, int, int, int,
// Heap *) at 0x00609730 WAS HERE and are gone: the bodies are homed into
// src/leaf_recoveries.cpp 2026-09-27, beside their sibling overloads.
// SpriteBox::init's two tail-forwards reach them by name through the
// embedded Dialog base, unchanged.

void Datalink::draw_entry() {  // 0x0042BF10
    typedef void(__fastcall *pending)(Datalink *, void *);
    PENDING_BODY(0x0042BF10, pending)(this, nullptr);
}

// DesignWin::select_special (0x0043AC70) WAS HERE and is gone: the body is
// homed into src/designwin.cpp 2026-09-27, at the end of the file so no
// recovered body above it shifts. Both byte-exact select_special_N thunks and
// draw_flash still reach it by name.

// ?update_window@Win@@QAEHPAURECT@@@Z at 0x005F74A0 - GraphicWin::update

// ?client_to_screen@Win@@QAEXPAURECT@@@Z at 0x005ECFE0, the RECT overload -
// distinct from client_to_screen(int*, int*) above. GraphicWin::update
// calls it BY NAME.


void FileWin::UNK4() {  // 0x006146A0
    typedef void(__fastcall *pending)(FileWin *, void *);
    PENDING_BODY(0x006146A0, pending)(this, nullptr);
}

void Menu::mouse_move(int a1, int a2) {  // 0x005FBDB0
    typedef void(__fastcall *pending)(Menu *, void *, int, int);
    PENDING_BODY(0x005FBDB0, pending)(this, nullptr, a1, a2);
}

// ReportWin::sat_anim (0x0049FE40) WAS HERE: the body is homed into
// src/reportwin.cpp 2026-09-27 (the sixteen-satellite orbital tick). Its
// daemon wrapper sat_anim_daemon (leaf_recoveries.cpp) already called it
// by name, so no caller repointing was needed.

void ReportWin::exp_anim() {  // 0x004A0100
    typedef void(__fastcall *pending)(ReportWin *, void *);
    PENDING_BODY(0x004A0100, pending)(this, nullptr);
}

void MainInterface::on_iface_button_clicked(int a1) {  // 0x0045D170
    typedef void(__fastcall *pending)(MainInterface *, void *, int);
    PENDING_BODY(0x0045D170, pending)(this, nullptr, a1);
}

int SetupWin::do_menu_rightside(char *a1, int a2) {  // 0x004ADB70
    typedef int(__fastcall *pending)(SetupWin *, void *, char *, int);
    return PENDING_BODY(0x004ADB70, pending)(this, nullptr, a1, a2);
}

uint32_t SpriteBox::close() {  // 0x00610280
    typedef void(__fastcall *pending)(SpriteBox *, void *);
    PENDING_BODY(0x00610280, pending)(this, nullptr);
    return 0;
}

void ReplayWin::timer_callback() {  // 0x005AD9E0
    typedef void(__fastcall *pending)(ReplayWin *, void *);
    PENDING_BODY(0x005AD9E0, pending)(this, nullptr);
}




// ?edit@EditGroup@@QAEHPADPADH@Z at 0x00611F80 - WAS HERE and is gone: the
// body is homed into src/editgroup.cpp 2026-09-27. DipEdit::on_button_clicked
// drives it twice on BasePop's EditGroup subobject at 0x22C8 (see
// BasePop::edit_group), once per atrocity-prompt field.

// ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z at 0x00633F70 WERE HERE and is gone: the
// body is homed into net_class.cpp beside the rest of its class.

// ?init@BasePop@@QAEHHJ@Z at 0x006015B0 is HOMED into src/basepop.cpp.

uint32_t __cdecl save_daemon(LPCSTR save_path) {  // 0x005A94F0
    typedef uint32_t(__cdecl *pending)(LPCSTR);
    return PENDING_BODY(0x005A94F0, pending)(save_path);
}

uint32_t __cdecl load_daemon(LPCSTR a1, BOOL a2) {  // 0x005A9760
    typedef uint32_t(__cdecl *pending)(LPCSTR, BOOL);
    return PENDING_BODY(0x005A9760, pending)(a1, a2);
}

int __cdecl popb(LPCSTR a1, int a2, int a3, LPCSTR a4, Sprite * a5) {  // 0x0048C650
    typedef int(__cdecl *pending)(LPCSTR, int, int, LPCSTR, Sprite *);
    return PENDING_BODY(0x0048C650, pending)(a1, a2, a3, a4, a5);
}

void __cdecl alt_set(int x, int y, uint32_t altitude) {  // 0x00591290
    typedef void(__cdecl *pending)(int, int, uint32_t);
    PENDING_BODY(0x00591290, pending)(x, y, altitude);
}

int __cdecl popp(LPCSTR a1, LPCSTR a2, int a3, LPCSTR a4, int(__cdecl *a5)()) {  // 0x0048C0A0
    typedef int(__cdecl *pending)(LPCSTR, LPCSTR, int, LPCSTR, int(__cdecl *a5)());
    return PENDING_BODY(0x0048C0A0, pending)(a1, a2, a3, a4, a5);
}

BaseWin::~BaseWin() {  // ??1BaseWin@@QAE@XZ at 0x00420A70
    typedef void(__fastcall *pending)(BaseWin *, void *);
    PENDING_BODY(0x00420A70, pending)(this, nullptr);
}

CheckButton::~CheckButton() {  // ??1CheckButton@@QAE@XZ at 0x004B3EC0
    typedef void(__fastcall *pending)(CheckButton *, void *);
    PENDING_BODY(0x004B3EC0, pending)(this, nullptr);
}

Credits::~Credits() {  // ??1Credits@@QAE@XZ at 0x00428E90
    typedef void(__fastcall *pending)(Credits *, void *);
    PENDING_BODY(0x00428E90, pending)(this, nullptr);
}

Datalink::~Datalink() {  // ??1Datalink@@QAE@XZ at 0x00432290
    typedef void(__fastcall *pending)(Datalink *, void *);
    PENDING_BODY(0x00432290, pending)(this, nullptr);
}

DesignWin::~DesignWin() {  // ??1DesignWin@@QAE@XZ at 0x0043ECB0
    typedef void(__fastcall *pending)(DesignWin *, void *);
    PENDING_BODY(0x0043ECB0, pending)(this, nullptr);
}

DiploWin::~DiploWin() {  // ??1DiploWin@@QAE@XZ at 0x00444DB0
    typedef void(__fastcall *pending)(DiploWin *, void *);
    PENDING_BODY(0x00444DB0, pending)(this, nullptr);
}

Gamma::~Gamma() {  // ??1Gamma@@QAE@XZ at 0x00456110
    typedef void(__fastcall *pending)(Gamma *, void *);
    PENDING_BODY(0x00456110, pending)(this, nullptr);
}

MainInterface::~MainInterface() {  // ??1MainInterface@@QAE@XZ at 0x0045EB80
    typedef void(__fastcall *pending)(MainInterface *, void *);
    PENDING_BODY(0x0045EB80, pending)(this, nullptr);
}

// Menu::~Menu (??1Menu@@QAE@XZ at 0x005FAD00) WAS HERE: the body is homed
// into src/menu.cpp 2026-09-27, mirroring the constructor's entry reset
// loop and releasing each pull-down through `delete` (the image's slot-0
// deleting-destructor dispatch). Console::close already called it by name.

NetMsg::~NetMsg() {  // ??1NetMsg@@QAE@XZ at 0x0047ADA0
    typedef void(__fastcall *pending)(NetMsg *, void *);
    PENDING_BODY(0x0047ADA0, pending)(this, nullptr);
}

NetWin::~NetWin() {  // ??1NetWin@@QAE@XZ at 0x004834E0
    typedef void(__fastcall *pending)(NetWin *, void *);
    PENDING_BODY(0x004834E0, pending)(this, nullptr);
}

NewTechWin::~NewTechWin() {  // ??1NewTechWin@@QAE@XZ at 0x00484810
    typedef void(__fastcall *pending)(NewTechWin *, void *);
    PENDING_BODY(0x00484810, pending)(this, nullptr);
}

PickWin::~PickWin() {  // ??1PickWin@@QAE@XZ at 0x0048A8E0
    typedef void(__fastcall *pending)(PickWin *, void *);
    PENDING_BODY(0x0048A8E0, pending)(this, nullptr);
}

ReplayWin::~ReplayWin() {  // ??1ReplayWin@@QAE@XZ at 0x005ADF10
    typedef void(__fastcall *pending)(ReplayWin *, void *);
    PENDING_BODY(0x005ADF10, pending)(this, nullptr);
}

ReportWin::~ReportWin() {  // ??1ReportWin@@QAE@XZ at 0x004AD3B0
    typedef void(__fastcall *pending)(ReportWin *, void *);
    PENDING_BODY(0x004AD3B0, pending)(this, nullptr);
}

SetupWin::~SetupWin() {  // ??1SetupWin@@QAE@XZ at 0x004AE790
    typedef void(__fastcall *pending)(SetupWin *, void *);
    PENDING_BODY(0x004AE790, pending)(this, nullptr);
}

SocialWin::~SocialWin() {  // ??1SocialWin@@QAE@XZ at 0x004B3C80
    typedef void(__fastcall *pending)(SocialWin *, void *);
    PENDING_BODY(0x004B3C80, pending)(this, nullptr);
}

StringBox::~StringBox() {  // ??1StringBox@@QAE@XZ at 0x004325C0
    typedef void(__fastcall *pending)(StringBox *, void *);
    PENDING_BODY(0x004325C0, pending)(this, nullptr);
}

TutWin::~TutWin() {  // ??1TutWin@@QAE@XZ at 0x004BE7D0
    typedef void(__fastcall *pending)(TutWin *, void *);
    PENDING_BODY(0x004BE7D0, pending)(this, nullptr);
}

Interlude::~Interlude() {  // ??1Interlude@@QAE@XZ at 0x0045F740
    typedef void(__fastcall *pending)(Interlude *, void *);
    PENDING_BODY(0x0045F740, pending)(this, nullptr);
}

MonuWin::~MonuWin() {  // ??1MonuWin@@QAE@XZ at 0x00477B10
    typedef void(__fastcall *pending)(MonuWin *, void *);
    PENDING_BODY(0x00477B10, pending)(this, nullptr);
}

// ?draw@MultiWin@@QAEXH@Z at 0x00479330. turn_timer (time.cpp) reaches it on
// the fixed MultiWin at 0x007FD648 with the literal 1.
void MultiWin::draw(int draw_type) {  // 0x00479330
    typedef void(__fastcall *pending)(MultiWin *, void *, int);
    PENDING_BODY(0x00479330, pending)(this, nullptr, draw_type);
}

PickTech::~PickTech() {  // ??1PickTech@@QAE@XZ at 0x00488690
    typedef void(__fastcall *pending)(PickTech *, void *);
    PENDING_BODY(0x00488690, pending)(this, nullptr);
}

PrefWin::~PrefWin() {  // ??1PrefWin@@QAE@XZ at 0x00491DF0
    typedef void(__fastcall *pending)(PrefWin *, void *);
    PENDING_BODY(0x00491DF0, pending)(this, nullptr);
}

QuayleWin::~QuayleWin() {  // ??1QuayleWin@@QAE@XZ at 0x00496710
    typedef void(__fastcall *pending)(QuayleWin *, void *);
    PENDING_BODY(0x00496710, pending)(this, nullptr);
}

VoiceRx::~VoiceRx() {  // ??1VoiceRx@@QAE@XZ at 0x004C8A50
    typedef void(__fastcall *pending)(VoiceRx *, void *);
    PENDING_BODY(0x004C8A50, pending)(this, nullptr);
}

// NO VoiceTx::~VoiceTx forwarder. VoiceTx is now `VoiceTx : public Sound`
// (net_class.h), so the compiler-generated ??1VoiceTx calls the real,
// claimed ~Sound once - the retired 0x004C8DB0 forwarder faulted where the
// image's destructor (which inlines the whole three-stage Sound descent)
// actually tears down. 0x004C8DB0 itself stays unrecovered.


int MapWin::focus(int x_coord, int y_coord) {  // 0x0046B310
    typedef int(__fastcall *pending)(MapWin *, void *, int, int);
    return PENDING_BODY(0x0046B310, pending)(this, nullptr, x_coord, y_coord);
}

// ?calculate_dimensions@CheckBox@@QAEHXZ at 0x0060ED10 - Dialogs
// ::calculate_dimensions (0x006132A0) tail-calls it for kind 1. The body is
// still FILE-only in src/unrecovered/0060ed10.cpp.
int CheckBox::calculate_dimensions() {  // 0x0060ED10
    typedef int(__fastcall *pending)(CheckBox *, void *);
    return PENDING_BODY(0x0060ED10, pending)(this, nullptr);
}

int CheckBox::attach(GraphicWin * a1, int a2, int a3, int a4) {  // 0x0060E800
    typedef int(__fastcall *pending)(CheckBox *, void *, GraphicWin *, int, int, int);
    return PENDING_BODY(0x0060E800, pending)(this, nullptr, a1, a2, a3, a4);
}

// CheckBox::on_key_down (0x0060F8B0) and CheckBox::on_mouse_move (0x0060FBB0)
// WERE HERE and are gone: both bodies are homed into src/leaf_recoveries.cpp
// 2026-09-27.

void CheckBox::on_left_down(int a1, int a2) {  // 0x0060F700
    typedef void(__fastcall *pending)(CheckBox *, void *, int, int);
    PENDING_BODY(0x0060F700, pending)(this, nullptr, a1, a2);
}

void CheckBox::on_redraw() {  // 0x0060FAB0
    typedef void(__fastcall *pending)(CheckBox *, void *);
    PENDING_BODY(0x0060FAB0, pending)(this, nullptr);
}

int Console::on_key_click(int a1, int a2) {  // 0x005178C0
    typedef int(__fastcall *pending)(Console *, void *, int, int);
    return PENDING_BODY(0x005178C0, pending)(this, nullptr, a1, a2);
}

// Console::on_nc_left_down (0x0050FB10) and Console::on_nc_mouse_move
// (0x00510110) WERE HERE and are gone: both bodies are homed into
// src/console.cpp 2026-09-27.

void Console::on_nc_left_up(int a1, int a2) {  // 0x0050FE70
    typedef void(__fastcall *pending)(Console *, void *, int, int);
    PENDING_BODY(0x0050FE70, pending)(this, nullptr, a1, a2);
}

// ?end_my_turn@Console@@QAEXXZ at 0x005169F0 - the turn handoff
// auto_play_callback (console.cpp) makes once AutoPlayCounter reaches 100.
// Its archived transcription still measures MISMATCH 88 of 180, so this is
// the forwarder and not a homed body.
void Console::end_my_turn() {
    typedef void(__fastcall *pending)(Console *, void *);
    PENDING_BODY(0x005169F0, pending)(this, nullptr);
}

int Dialogs::on_scroll_create() {  // 0x00613220
    typedef int(__fastcall *pending)(Dialogs *, void *);
    return PENDING_BODY(0x00613220, pending)(this, nullptr);
}

void Dialogs::pass_dialog_focus() {  // 0x00613180
    typedef void(__fastcall *pending)(Dialogs *, void *);
    PENDING_BODY(0x00613180, pending)(this, nullptr);
}

// ?set_default@EditGroup@@QAEHPADH@Z at 0x006120D0 - WAS HERE and is gone:
// the body is homed into src/editgroup.cpp 2026-09-27. BasePop::start
// (0x00601BF0) still calls it BY NAME with the caller's override text, so the
// call site keeps the image's `E8`.

// ?calculate_dimensions@EditGroup@@QAEHXZ at 0x00612150 - Dialogs
// ::calculate_dimensions (0x006132A0) tail-calls it for kind 4. The body is
// still FILE-only in src/unrecovered/00612150.cpp.
int EditGroup::calculate_dimensions() {  // 0x00612150
    typedef int(__fastcall *pending)(EditGroup *, void *);
    return PENDING_BODY(0x00612150, pending)(this, nullptr);
}

int EditGroup::attach(GraphicWin * a1, int a2, int a3, int a4) {  // 0x00611AF0
    typedef int(__fastcall *pending)(EditGroup *, void *, GraphicWin *, int, int, int);
    return PENDING_BODY(0x00611AF0, pending)(this, nullptr, a1, a2, a3, a4);
}

void EditGroup::on_redraw() {  // 0x00612450
    typedef void(__fastcall *pending)(EditGroup *, void *);
    PENDING_BODY(0x00612450, pending)(this, nullptr);
}

void EditGroup::pass_dialog_focus() {  // 0x006126C0
    typedef void(__fastcall *pending)(EditGroup *, void *);
    PENDING_BODY(0x006126C0, pending)(this, nullptr);
}


void ListBox::on_key_down(int a1) {  // 0x0060AF90
    typedef void(__fastcall *pending)(ListBox *, void *, int);
    PENDING_BODY(0x0060AF90, pending)(this, nullptr, a1);
}

void ListBox::on_left_down(int a1, int a2) {  // 0x0060AA60
    typedef void(__fastcall *pending)(ListBox *, void *, int, int);
    PENDING_BODY(0x0060AA60, pending)(this, nullptr, a1, a2);
}

void ListBox::on_mouse_move(int a1, int a2) {  // 0x0060CBC0
    typedef void(__fastcall *pending)(ListBox *, void *, int, int);
    PENDING_BODY(0x0060CBC0, pending)(this, nullptr, a1, a2);
}

void ListBox::on_redraw() {  // 0x0060C350
    typedef void(__fastcall *pending)(ListBox *, void *);
    PENDING_BODY(0x0060C350, pending)(this, nullptr);
}

// ListBox::on_right_double_click, on_right_down and on_scrolled WERE HERE,
// forwarding to 0x0060C6D0, 0x0060AA20 and 0x0060C6A0. Homed into listbox.cpp
// 2026-08-29.

void MapWin::on_button_clicked(int a1) {  // 0x0046F8C0
    typedef void(__fastcall *pending)(MapWin *, void *, int);
    PENDING_BODY(0x0046F8C0, pending)(this, nullptr, a1);
}

void MapWin::on_left_down(int a1, int a2) {  // 0x0046ED30
    typedef void(__fastcall *pending)(MapWin *, void *, int, int);
    PENDING_BODY(0x0046ED30, pending)(this, nullptr, a1, a2);
}

// ?MapWin::on_lose_mouse_capture at 0x0046EB90 - homed into mapwin.cpp on
// 2026-08-29, so the forwarder that stood here is gone.

// MapWin::on_sys_close WAS HERE, forwarding to 0x0046F880. Homed into
// mapwin.cpp 2026-08-29.

// THE GENERATION BATCH SEAM (2026-09-21). gen_tile (0x00468C20) and gen_map
// (0x00469CA0) are homed into src/mapwin.cpp and call the six bodies below
// BY NAME; the declarations live in mapwin.h and these forwarders are their
// definitions, so the application links while the scoped checks keep
// carrying each callee as an unresolved target against the original image.

// ?draw_vehicles@MapWin@@QAEXHHHHH@Z at 0x00466CA0 (seed
// src/recovered/units/00466ca0.cpp). gen_tile's vehicle tail (0x00468CB0)
// and gen_map's vehicle scan (0x0046A035) push (x, y, pixel_x,
// pixel_y - field_1DDC0_, draw_type) - the pushes at 0x00468CA3..AD and
// 0x0046A023..32 fix the five-argument shape.
void MapWin::draw_vehicles(int x, int y, int pixel_x, int pixel_y,
                           int draw_type) {
    typedef void(__fastcall *pending)(MapWin *, void *, int, int, int, int,
                                      int);
    PENDING_BODY(0x00466CA0, pending)(this, nullptr, x, y, pixel_x, pixel_y,
                                      draw_type);
}

// ?draw_base_names@MapWin@@QAEXXZ at 0x00467DE0 (seed
// src/unrecovered/00467de0.cpp). gen_map calls it unconditionally between
// its vehicle and overlay scans (0x0046A079).
void MapWin::draw_base_names() {
    typedef void(__fastcall *pending)(MapWin *, void *);
    PENDING_BODY(0x00467DE0, pending)(this, nullptr);
}

// 0x004688E0 - NO catalogue mapping and no seed: the name gen_overlays and
// the void(int, int) shape rest on its two call sites alone (gen_tile's
// overlay tail at 0x00468CB9 and gen_map's overlay scan at 0x0046A221, both
// pushing (x, y) after `mov ecx, esi` with the callee cleaning ret 8). The
// header declaration carries the same caveat.
void MapWin::gen_overlays(int x, int y) {
    typedef void(__fastcall *pending)(MapWin *, void *, int, int);
    PENDING_BODY(0x004688E0, pending)(this, nullptr, x, y);
}

// ?draw_legend@MapWin@@QAEXXZ at 0x00469720 (seed
// src/recovered/units/00469720.cpp). gen_map's 0x20000-gated legend pass
// (0x0046A271).
void MapWin::draw_legend() {
    typedef void(__fastcall *pending)(MapWin *, void *);
    PENDING_BODY(0x00469720, pending)(this, nullptr);
}

// ?draw_wars@MapWin@@QAEXXZ at 0x004698D0 (seed
// src/recovered/units/004698d0.cpp). gen_map's 0x100000-gated territory-war
// pass (0x0046A284).
void MapWin::draw_wars() {
    typedef void(__fastcall *pending)(MapWin *, void *);
    PENDING_BODY(0x004698D0, pending)(this, nullptr);
}

// ?draw_base_dest@MapWin@@QAEXH@Z at 0x00467970. line_timer (time.cpp) calls
// it per live map-window slot with the literal 1.
void MapWin::draw_base_dest(int draw_type) {  // 0x00467970
    typedef void(__fastcall *pending)(MapWin *, void *, int);
    PENDING_BODY(0x00467970, pending)(this, nullptr, draw_type);
}

void PlanWin::on_redraw() {  // 0x0048AF30
    typedef void(__fastcall *pending)(PlanWin *, void *);
    PENDING_BODY(0x0048AF30, pending)(this, nullptr);
}

// ?calculate_dimensions@RadioButton@@QAEHXZ at 0x0060D380 - Dialogs
// ::calculate_dimensions (0x006132A0) tail-calls it for kind 0x10. The body
// is still FILE-only in src/unrecovered/0060d380.cpp.
int RadioButton::calculate_dimensions() {  // 0x0060D380
    typedef int(__fastcall *pending)(RadioButton *, void *);
    return PENDING_BODY(0x0060D380, pending)(this, nullptr);
}

int RadioButton::attach(GraphicWin * a1, int a2, int a3, int a4) {  // 0x0060D1F0
    typedef int(__fastcall *pending)(RadioButton *, void *, GraphicWin *, int, int, int);
    return PENDING_BODY(0x0060D1F0, pending)(this, nullptr, a1, a2, a3, a4);
}

// RadioButton::on_key_down (0x0060E020) WAS HERE and is gone: the body is
// homed into src/leaf_recoveries.cpp 2026-09-27.

void RadioButton::on_left_down(int a1, int a2) {  // 0x0060DE10
    typedef void(__fastcall *pending)(RadioButton *, void *, int, int);
    PENDING_BODY(0x0060DE10, pending)(this, nullptr, a1, a2);
}

void RadioButton::on_mouse_move(int a1, int a2) {  // 0x0060E320
    typedef void(__fastcall *pending)(RadioButton *, void *, int, int);
    PENDING_BODY(0x0060E320, pending)(this, nullptr, a1, a2);
}

void RadioButton::on_redraw() {  // 0x0060E220
    typedef void(__fastcall *pending)(RadioButton *, void *);
    PENDING_BODY(0x0060E220, pending)(this, nullptr);
}

// ?calculate_dimensions@SpriteBox@@QAEHXZ at 0x00610970 - Dialogs
// ::calculate_dimensions (0x006132A0) tail-calls it for kind 8. The body is
// still FILE-only in src/unrecovered/00610970.cpp.
int SpriteBox::calculate_dimensions() {  // 0x00610970
    typedef int(__fastcall *pending)(SpriteBox *, void *);
    return PENDING_BODY(0x00610970, pending)(this, nullptr);
}

int SpriteBox::attach(GraphicWin * a1, int a2, int a3, int a4) {  // 0x00610540
    typedef int(__fastcall *pending)(SpriteBox *, void *, GraphicWin *, int, int, int);
    return PENDING_BODY(0x00610540, pending)(this, nullptr, a1, a2, a3, a4);
}

int SpriteBox::on_key_down(int a1) {  // 0x00611400
    typedef int(__fastcall *pending)(SpriteBox *, void *, int);
    return PENDING_BODY(0x00611400, pending)(this, nullptr, a1);
}

void SpriteBox::on_left_click(int a1, int a2) {  // 0x00611060
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x00611060, pending)(this, nullptr, a1, a2);
}

// SpriteBox::on_redraw (0x00611380) WAS HERE and is gone: the body is homed
// into src/leaf_recoveries.cpp 2026-09-27.

// ?check@MainMenu@@QAEXH@Z at 0x00460DD0. The catalogue's class name for the
// object at Console+0x22A2C is MainMenu, but that object is AlphaMenu-shaped
// (??0Console builds Menu at +0x22A2C, then Buffer, Sprite and Sprite[3]
// after it - AlphaMenu's own member list), so the method is declared on
// AlphaMenu and this forwarder carries the tree's emitted name.
//             body in src/unrecovered/00460dd0.cpp
void AlphaMenu::check(int veh_id) {  // 0x00460DD0
    typedef void(__fastcall *pending)(AlphaMenu *, void *, int);
    PENDING_BODY(0x00460DD0, pending)(this, nullptr, veh_id);
}

// ?write_raw_l@Buffer@@QAEHPADHHH@Z at 0x005DBD00 - 1475 bytes, the raster
// writer that puts one single-font run on the surface. Called by name from
// `write_multi_font_raw_l`, which is promoted; a pointer here would cost that
// body the `E8` it now emits.
//             body in src/unrecovered/005dbd00.cpp
// ?write_strings@Buffer@@QAEHPAUStringList@@HHHH@Z at 0x005DB040, a
// pending_bodies forwarder. The body is 950 bytes and still FILE-only in
// src/unrecovered/005db040.cpp; BasePop::on_redraw (0x00604AB0) calls it BY
// NAME, so a pointer binding there would cost that body the `E8` it needs.
int Buffer::write_strings(StringList *strings, int left, int top, int width,
                          int height) {
    typedef int(__fastcall *pending)(Buffer *, void *, StringList *, int, int,
                                     int, int);
    return PENDING_BODY(0x005DB040, pending)(this, nullptr, strings, left, top,
                                             width, height);
}

int Buffer::write_raw_l(LPSTR text, int x_coord, int y_coord, int len) {
    typedef int(__fastcall *pending)(Buffer *, void *, LPSTR, int, int, int);
    return PENDING_BODY(0x005DBD00, pending)(this, nullptr, text, x_coord,
                                             y_coord, len);
}

// jackal_close's remaining not-yet-recovered callees (src/general.cpp).
// Two former entries here were PROMOTED and their forwarders retired:
// sub_62d100 (0x0062D100) is now defined in general.cpp above jackal_close,
// and the 0x00635750 thiscall on the object at 0x9BE618 turned out to be
// `DDInit::teardown` on WinDisplayInit - defined in win.cpp beside `init`,
// whose five fields are DDInit's own.

// ?close_class@FileWin@@QAAXXZ at 0x00614E30 - recovered as the member it is
// (src/filewin.cpp). jackal_close reaches the address with NO receiver set
// up, which no member call can spell, so this free stand-in forwards to the
// member on a receiver the body never reads. It stays because general.h
// declares this name and jackal_close calls it; the E8 to the member is a
// relocation on both sides either way.
void __cdecl filewin_close_class() {
    reinterpret_cast<FileWin *>(0)->close_class();
}

// ?close_class@BasePop@@QAAXXZ at 0x00604680 - HOMED into src/basepop.cpp,
// so this free stand-in now reaches the real member instead of the raw
// address. It stays a free function because jackal_close calls it with no
// receiver set up, which no member call can spell (filewin_close_class
// above is the same case).
void __cdecl basepop_close_class() {
    BasePop::close_class();
}

// ?init_class@Caviar@@QAAHXZ at 0x006185A0 is HOMED into src/caviar.cpp
// (a real RAII body over BasePop), so its forwarder is gone - the linker
// enforces that. The voxel-engine entry points init_class calls are HOMED
// THERE TOO (0x006392E0 vox_init_callbacks, 0x0063AF60
// vox_fill_colour_table, 0x006393C0 vox_create_record), and so are the
// callees those bodies reach: 0x00639390 sub_639390, 0x0063AD60 sub_63ad60
// and 0x0063F9B0 sub_63f9b0 are all defined at the foot of src/caviar.cpp
// beside their callers, so their forwarders are gone as well.

// ?close_class@Caviar@@QAAXXZ at 0x00618D20 is HOMED into src/caviar.cpp
// (2026-09-05, caviar class pass), so this forwarder is gone - the linker
// enforces that.

// 0x005D8EE0, the seven-argument Buffer::draw_dest_source. Caviar::render
// (homed into src/caviar.cpp the same day) is its first caller in this tree,
// so this stand-in exists for the link; the body in src/unrecovered/005d8ee0.cpp
// is still the only transcription of it. HANDOFF: whoever homes 0x005D8EE0
// deletes this forwarder with it.
int Buffer::draw_dest_source(Buffer *source, int colour, int x, int y, int flag,
                             int mode, unsigned char *bits) {
    // __fastcall with a dead second argument is the __thiscall spelling this
    // file uses throughout: the receiver lands in ECX and the seven real
    // arguments on the stack, exactly where the image's call site puts them.
    typedef int(__fastcall *pending)(Buffer *, void *, Buffer *, int, int, int,
                                     int, int, unsigned char *);
    return PENDING_BODY(0x005D8EE0, pending)(this, nullptr, source, colour, x,
                                             y, flag, mode, bits);
}

// ---------------------------------------------------------------------------
// PROMOTED OUT OF temp.h, where they were function POINTERS. A pointer
// binding compiles `call dword ptr [draw_map]` where the image has
// `call rel32`, so every caller of one reads as a mismatch however right its
// body is - that is what held `Console::editor_polar` and
// `Console::editor_climate` off the ratchet. The address is the same one
// src/temp.cpp bound, so nothing about runtime changed; only the call shape.
// ---------------------------------------------------------------------------

// world_climate's own not-yet-recovered callees (src/map.cpp).
// ?clear_terrain@MapWin@@QAEXXZ at 0x0046FD90


// ?do_checksums@@YAXH@Z at 0x0051E760
void __cdecl do_checksums(int type) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x0051E760, pending)(type);
}

// ?world_rivers@@YAXXZ at 0x005C38B0 - HOMED into src/map.cpp 2026-09-05 (see
// the marker there), so this file no longer carries it.

// ?custom_planet@@YAHHH@Z at 0x0058C2A0 - body in src/unrecovered/0058c2a0.cpp
int __cdecl custom_planet(int a, int b) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x0058C2A0, pending)(a, b);
}

// load_faction_art at 0x00453710 - body in src/unrecovered/00453710.cpp.
// Was a temp.h function POINTER (`func5 *const load_faction_art`); see the
// note above draw_map.
// ORIGINAL: 0x00453710 ?load_faction_art@@YAXH@Z 0x00453710-0x00453A17
// RECOVERY_BINDING: v1 {"evidence": ["Coordinator investigation_deferred 2026-09-19 (run-z7f3353q): obligation-5fbf6e74 read_faction(int) is unknown solely on 'unresolved targets: ?load_faction_art@@YAXH@Z'; src/alpha.cpp:681-686 spells both image call targets with load_faction_art(player_id) at image 0x00453710, and src/alpha.cpp:1210,1224 spell read_factions' two physical sites exactly as the image keeps them - no edit confined to alpha.cpp/alpha.h can resolve a link-time-absent definition.", "Seed src/unrecovered/00453710.cpp line 1 catalogues ORIGINAL 0x00453710 ?load_faction_art@@YAXH@Z 0x00453710-0x00453A17 FILE (775 bytes); the forwarder at src/pending_bodies.cpp:1492-1496 carries the same address in its PENDING_BODY macro and its comment records the function-pointer history."], "reason": "The pending_bodies.cpp forwarder at the annotated line is the only compiled definition of the symbol (a temp.h function-pointer binding historically); the name and extent are the seed's catalogue, adopted verbatim so cross-TU by-name references resolve against the original identity without transcribing the body."}
void __cdecl load_faction_art(int player_id) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x00453710, pending)(player_id);
}

// ?control_game@@YAXXZ at 0x0052AA30 - homed into game.cpp on 2026-08-29,
// so the forwarder that stood here is gone. ITS CALLEES moved in here with
// it: everything below that control_game calls is still an original body.
// desktop_close, close_opening and game_close were promoted the same day
// (bodies now in game.cpp); amovie_project went to alphamovie.cpp.

// ?system_init@@YAHXZ and ?game_init@@YAHHH@Z at 0x0058F040 / 0x0058F2F0 WERE
// HERE. Homed into game.cpp 2026-09-02, and the twelve forwarders the boot
// sequence needed came in with them - the five free functions and the six
// member `init` calls the two bodies drive, all still original bodies. game.h
// declares them; the six member calls are spelled as `__fastcall`
// receiver-first free functions there, because game.h cannot add a member to
// a header it does not own.

// ?load_sprites@@YAXXZ at 0x00455EB0
void __cdecl load_sprites() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00455EB0, pending)();
}

// load_fixed_sprites' callees, declared in game.h when its body landed: the
// fixed-art load names them and nothing else in the tree does.
// ?load_palette@@YAXXZ at 0x00451A10
void __cdecl load_palette() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00451A10, pending)();
}

// ?load_misc@@YAXXZ at 0x00454D10
void __cdecl load_misc() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00454D10, pending)();
}

// ?load_interface@@YAXH@Z at 0x00455610
void __cdecl load_interface(int mode) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x00455610, pending)(mode);
}


// ?load_deswin_sprites@@YAXXZ at 0x00455E50 is homed in leaf_recoveries.cpp
// under its own name - no forwarder, that claim is compiled.

// ?textcolor_init@@YAXXZ at 0x004BA220 is homed in palette.cpp - no forwarder.

// ?config_popups@@YAXXZ at 0x00588AA0
void __cdecl config_popups() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00588AA0, pending)();
}

// ?bung_momma@@YAXXZ at 0x004595C0
void __cdecl bung_momma() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x004595C0, pending)();
}

// ?init@MainInterface@@QAEHH@Z at 0x00459680
int __fastcall maininterface_init(MainInterface *self, int mode) {
    typedef int(__fastcall *pending)(MainInterface *, int);
    return PENDING_BODY(0x00459680, pending)(self, mode);
}

// ?init@BattleWin@@QAEXXZ at 0x004219E0
void __fastcall battlewin_init(BattleWin *self) {
    typedef void(__fastcall *pending)(BattleWin *);
    PENDING_BODY(0x004219E0, pending)(self);
}

// ?init@Console@@QAEXH@Z at 0x0051D890
void __fastcall console_init(Console *self, int mode) {
    typedef void(__fastcall *pending)(Console *, int);
    PENDING_BODY(0x0051D890, pending)(self, mode);
}

// ?init@MultiDebug@@QAEHPAUNet@@PAUWin@@PAE@Z at 0x005C9860 is homed as a
// real member in multidebug.cpp - no forwarder, and no game.h shape either.

// ?init@MessageWin@@QAEXXZ at 0x00471D50
void __fastcall messagewin_init(MessageWin *self) {
    typedef void(__fastcall *pending)(MessageWin *);
    PENDING_BODY(0x00471D50, pending)(self);
}

// ?init@Datalink@@QAEXXZ at 0x00429300
void __fastcall datalink_init(Datalink *self) {
    typedef void(__fastcall *pending)(Datalink *);
    PENDING_BODY(0x00429300, pending)(self);
}

// ?game_reload@@YAHHH@Z at 0x0058F450 WAS HERE. Homed into game.cpp
// 2026-08-29, beside desktop_close / close_opening / game_close.

// ?setup_game@@YAXH@Z at 0x005B3920
void __cdecl setup_game(int reload) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x005B3920, pending)(reload);
}

// ?top_menu@@YAHH@Z at 0x0058E360
int __cdecl top_menu(int mode) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x0058E360, pending)(mode);
}

// ?desktop_init@@YAHH@Z at 0x0058EE60
int __cdecl desktop_init(int fresh) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x0058EE60, pending)(fresh);
}

// ?multiplayer_init@@YAHH@Z at 0x0058DB30
int __cdecl multiplayer_init(int mode) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x0058DB30, pending)(mode);
}

// ?control_turn@@YAXXZ at 0x005275B0
void __cdecl control_turn() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005275B0, pending)();
}

// ?net_control_turn@@YAXXZ at 0x0052A2E0
void __cdecl net_control_turn() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0052A2E0, pending)();
}

// ?desktop_close@@YAXXZ at 0x0058EFF0, ?close_opening@@YAXXZ at 0x00589B20
// and ?game_close@@YAXH@Z at 0x0058F430 - homed into game.cpp on 2026-08-29,
// so the forwarders that stood here are gone.

// ?system_close@@YAXXZ at 0x0058F250
void __cdecl system_close() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0058F250, pending)();
}

// ?amovie_project@@YAXPAD@Z at 0x00403BE0 - homed into alphamovie.cpp on
// 2026-08-29, so the forwarder that stood here is gone.

// ?mapwin_system_shutdown@@YAXXZ at 0x004710E0 - body in src/unrecovered/004710e0.cpp.
// desktop_close tears the map-window system down between the window closes
// and the final hide.
void __cdecl mapwin_system_shutdown() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x004710E0, pending)();
}

// ?close@DesignWin@@QAEXXZ at 0x0043C1A0 - homed into designwin.cpp on
// 2026-08-29, so the forwarder that stood here is gone. desktop_close closes
// the process-wide design window through this member.

// ?exec@UV2Player@@QAEHPADHHH@Z at 0x004BEA50 - body in src/unrecovered/004bea50.cpp.
// amovie_project plays the .wve candidates through it.
int UV2Player::exec(char *movie_file, int width, int height, int flags) {
    typedef int(__fastcall *pending)(UV2Player *, void *, char *, int, int, int);
    return PENDING_BODY(0x004BEA50, pending)(this, nullptr, movie_file, width,
                                             height, flags);
}

// ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z at 0x0062D9E0 - homed into
// net_class.cpp, so the forwarder that stood here is gone.

// ?init@Net@@QAEHPAU_GUID@@HH@Z at 0x0062D8E0 - homed into net_class.cpp too;
// its forwarder is gone with it.

// ?join_session@Net@@QAEHPAUSessionStruct@@PADPAD@Z at 0x0062E540 -
// unrecovered; check_for_lobby hands the lobby's session to it.
int Net::join_session(struct SessionStruct *session, char *player_name,
                      char *address) {
    typedef int(__fastcall *pending)(Net *, void *, SessionStruct *, char *,
                                     char *);
    return PENDING_BODY(0x0062E540, pending)(this, nullptr, session,
                                             player_name, address);
}

// ?send@Net@@QAEHPAXHKH@Z at 0x00630080 - 2232 bytes of DirectPlay send with
// per-peer RTT bookkeeping, still archived (00630080.cpp, FILE-only: no claim
// to landmine). Its receiver has to be real here because
// NetDaemon::send_message, homed into netdaemon.cpp, calls it for every
// packet it builds, and a `T *const` receiver binding would cost that call
// site the image's `E8`.
int Net::send(void *data, int mode, unsigned long who, int size) {
    typedef int(__fastcall *pending)(Net *, void *, void *, int, unsigned long,
                                     int);
    return PENDING_BODY(0x00630080, pending)(this, nullptr, data, mode, who,
                                             size);
}

// ??0BasePop@@QAE@XZ at 0x00600860 - homed into basepop.cpp on 2026-08-29
// (a real field-store constructor, not a forwarder), so the definition that
// used to stand here is gone. THE FRONTIER MOVED HERE when basepop_alloc was
// recovered into src/basepop.cpp: `new BasePop()` needs a constructor to link
// against, and basepop.h no longer defines one inline. The forwarder that
// used to sit here for basepop_alloc itself is gone - the linker enforces
// that, since two definitions of one symbol is LNK2005.

// ---------------------------------------------------------------------------
// THE FRONTIER AFTER jackal_init_real, which was recovered into
// src/general.cpp on 2026-08-15. It calls nineteen functions; five were
// already compiled and these fourteen were not, so promoting one body moved
// the edge outward by fourteen. Each is one `#include` and one body away from
// deleting its line here.
// ---------------------------------------------------------------------------

// `DDInit::init` (0x00635510) is promoted into win.cpp. Its two callees
// that are still unrecovered stay here as forwarders:

// 0x00628F30, what `DDInit::report_error` tails into once its popup is
// dismissed - the install-media check - is homed into win.cpp on 2026-08-29
// as `cd_check`, so the forwarder that used to stand here is gone.


// What `Win::window_proc` routes to, now that it is promoted into
// src/win.cpp. These six are what its recovery still stands on, and the
// first thing that faults if you move the mouse or press a key.
//



// ?get_mouse_window_recurse@@YAHPAUWin@@PAHPAH@Z at 0x005F6AB0 - the tree walk
// `get_mouse_window` delegates to, 1110 bytes.


// ?do_tracking@Win@@QAEXHH@Z at 0x005F7580 - the one __thiscall member of the

// sub_5f86a0 at 0x005F86A0 - homed from src/recovered/005f86a0.cpp into
// win.cpp on 2026-08-25 and BYTE_EXACT in the build, so the forwarder
// that used to stand here is gone and the edge is no longer pending.

// ?trig_init@@YAHXZ at 0x0063B940 - homed into general.cpp on 2026-08-29,
// so the forwarder that used to stand here is gone and the edge is no
// longer pending.

// ?insert@WaveGroupList@@ at 0x004C5BF0 - the wave group list-insert helper
// Wave_Device::add_to_group threads new waves through.
void WaveGroupList::insert(Wave *wave) {
    // NOT a PENDING_BODY any more. 0x004C5BF0 is recovered and BYTE_EXACT in
    // wave_device.cpp as `wave_group_insert_redirect`; jumping to the raw
    // image address while the real body is linked in is a live fault, which
    // is what address_index.py calls a LANDMINE.
    wave_group_insert_redirect(this, nullptr, wave);
}

/*
Purpose: Step the receiver back to the subobject ??_GBasePop@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004070B0 ??_GBasePop@@WEEE@AEPAXI@Z 0x004070B0-0x004070BB BYTE_EXACT
// symbol    ??_EBasePop@@WEEE@AEPAXI@Z
// CORRECTED from ??3BasePop@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406B40` into
//   ??_GBasePop@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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

/*
Purpose: Step the receiver back to the subobject ??_GCredits@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00428F80 ??_GCredits@@WEEE@AEPAXI@Z 0x00428F80-0x00428F8B BYTE_EXACT
// symbol    ??_ECredits@@WEEE@AEPAXI@Z
// CORRECTED from ??3Credits@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00428E60` into
//   ??_GCredits@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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

/*
Purpose: Step the receiver back to the subobject ??_GSetupWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AE9A0 ??_GSetupWin@@WEEE@AEPAXI@Z 0x004AE9A0-0x004AE9AB BYTE_EXACT
// symbol    ??_ESetupWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3SetupWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AE760` into
//   ??_GSetupWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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

/*
Purpose: Step the receiver back to the subobject ??_GReplayWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x005AE0E0 ??_GReplayWin@@WEEE@AEPAXI@Z 0x005AE0E0-0x005AE0EB BYTE_EXACT
// symbol    ??_EReplayWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3ReplayWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x005AE0B0` into
//   ??_GReplayWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// ---- Forwarders for bodies the Win homing revealed as callable but not yet
// ---- recovered. Each address is the catalogue's, not a guess.

void Buffer::copy_to_window(Win *target, int sx, int sy, int x, int y, int w, int h) {
    typedef void(__fastcall *pending)(Buffer *, void *, Win *, int, int, int, int, int, int);
    PENDING_BODY(0x005D9BE0, pending)(this, nullptr, target, sx, sy, x, y, w, h);
}

// ?send_packet_type@Net@@QAEHPAXHKHH@Z at 0x0062F8A0 - still unrecovered
// (a 2015-byte sender). Net::start_voice and Net::stop_voice, promoted into
// src/net_class.cpp, are gone from here; stop_voice tails into this member,
// and the forwarder keeps that call site's `E8`.
int Net::send_packet_type(void *data, int mode, unsigned long who, int size,
                          int flags) {
    typedef int(__fastcall *pending)(Net *, void *, void *, int, unsigned long,
                                     int, int);
    return PENDING_BODY(0x0062F8A0, pending)(this, nullptr, data, mode, who,
                                             size, flags);
}


int Win::sub_63c7c0() {
    typedef int(__fastcall *pending)(Win *, void *);
    return PENDING_BODY(0x0063C7C0, pending)(this, nullptr);
}

void __cdecl wait_task() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005FC700, pending)();
}

// The four one-directional treaty/agenda writers DipEdit's checkbox handlers
// call - treaty_on (0x0055B870), treaty_off (0x0055B760), agenda_on
// (0x0055BA80) and agenda_off (0x0055B820) - are homed into src/faction.cpp
// now, so the forwarders that stood here are gone.

// ?ambience@@YAHH@Z at 0x00446B40 - the event-sound player treaty_on asks for
// (0xcc) when it sets a vendetta with the local faction involved.
int __cdecl ambience(int ambience_id) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x00446B40, pending)(ambience_id);
}




// Menu's add_menu_item (0x005FB100) and rename_menu_item (0x005FB700) - homed
// into src/menu.cpp - call PullDown::add_item (0x005F89D0) and
// PullDown::rename_item (0x005F8E50). Both are homed in src/pulldown.cpp now,
// beside their sibling add_separator and their shared helper compute_length,
// so the forwarders that stood here are gone.

// ---------------------------------------------------------------------------
// THIRTEEN FREE FUNCTIONS the Console bodies call, none of which src/ had a
// definition for. draw_cursor left for mapwin.cpp, where its body now lives. Each target is an UNCLAIMED archive body (file / verbatim,
// no claim), so a forwarder is the safe spelling for it - no address-index
// landmine. The declarations live at the top of src/console.cpp, beside the
// callers that needed them; homing any of these into its own TU deletes both
// the forwarder here and the declaration there.

int __cdecl save_game(int a1) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x005A9EB0, pending)(a1);
}

void __cdecl base_kill(int base_id) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x004E5250, pending)(base_id);
}

int __cdecl end_of_game(int a1) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x00524870, pending)(a1);
}

int __cdecl load_game(int a1, int a2) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x005AAAB0, pending)(a1, a2);
}

int __cdecl load_map() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x005AB600, pending)();
}

// ?save_map@@YAXXZ at 0x005ABB90, still archived
// (src/recovered/units/005abb90.cpp). Console::editor_save calls it; the
// declaration is beside load_map in console.cpp's handoff block.
void __cdecl save_map() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005ABB90, pending)();
}

// ?tech_pick@@YAHHHHPAD@Z at 0x005BEB70, still archived
// (src/unrecovered/005beb70.cpp - the PickTech EH wall). Console::set_tech
// calls it; the declaration is in console.cpp's handoff block.
int __cdecl tech_pick(int faction_id, int a2, int a3, const char *section) {
    typedef int(__cdecl *pending)(int, int, int, const char *);
    return PENDING_BODY(0x005BEB70, pending)(faction_id, a2, a3, section);
}



int __cdecl size_of_planet(int a1) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x0058CED0, pending)(a1);
}

int __cdecl supply_options(int a1, int a2) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x00593A00, pending)(a1, a2);
}

void __cdecl time_controls_dialog(Popup *popup) {
    typedef void(__cdecl *pending)(Popup *);
    PENDING_BODY(0x00589330, pending)(popup);
}

int __cdecl veh_pick(int a1, const char *label, int a3) {
    typedef int(__cdecl *pending)(int, const char *, int);
    return PENDING_BODY(0x0048D0D0, pending)(a1, label, a3);
}

// ?world_build@@YAXXZ at 0x005C86E0 - HOMED into src/map.cpp 2026-09-05 (see
// the marker there), so this file no longer carries it.

// Two callees the diplomacy bodies homed into src/faction.cpp reach with the
// image's `call rel32`:
//   ?pop@NetMsg@@QAEXPBDHHPBD@Z   0x0047A890
//       (src/recovered/units/0047a890.cpp - measured NO_COMPILE there) - the
//       only one of the two still forwarded here.
//   ?get_he_she@@YAPADHH@Z        0x0058FCE0 - WAS HERE and is gone: the body
//       is homed into src/stringstruct.cpp 2026-09-27.
// The seed under the remaining forwarder is FILE-only, so it is the only
// definition in the build and carries no claim an extra definition could
// landmine.
void NetMsg::pop(const char *label, int duration, int channel,
                 const char *target) {
    typedef void(__fastcall *pending)(NetMsg *, void *, const char *, int, int,
                                      const char *);
    PENDING_BODY(0x0047A890, pending)(this, nullptr, label, duration, channel,
                                      target);
}

// The cross-TU callees faction.h declares for the net_* / diplomacy bodies.
// All six are FILE-only archive bodies carrying no claim, so none of these
// forwarders can landmine.
void __cdecl trade_maps(int faction_id, int faction_id_with) {
    typedef void(__cdecl *pending)(int, int);
    PENDING_BODY(0x00543A20, pending)(faction_id, faction_id_with);
}

void __cdecl tech_achieved(int faction_id, int faction_id_with, int tech_id,
                           int give_freebie) {
    typedef void(__cdecl *pending)(int, int, int, int);
    PENDING_BODY(0x005BB000, pending)(faction_id, faction_id_with, tech_id,
                                      give_freebie);
}

int __cdecl do_withdrawal(int faction_id, int faction_id_with) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x00548F80, pending)(faction_id, faction_id_with);
}

void __cdecl give_a_base(int faction_id, int faction_id_with) {
    typedef void(__cdecl *pending)(int, int);
    PENDING_BODY(0x0054CF40, pending)(faction_id, faction_id_with);
}

void __cdecl double_cross(int faction_id, int faction_id_with, int victim) {
    typedef void(__cdecl *pending)(int, int, int);
    PENDING_BODY(0x0050BCC0, pending)(faction_id, faction_id_with, victim);
}

void __cdecl pact_ends(int faction_id, int faction_id_with) {
    typedef void(__cdecl *pending)(int, int);
    PENDING_BODY(0x0053C650, pending)(faction_id, faction_id_with);
}

void __cdecl wait_loop() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0051F200, pending)();
}



// ?lock@NetDaemon@@QAEHHHHHHH@Z at 0x00530630 - 734 bytes: it broadcasts the
// lock request through message_lock, then polls with the same wait loop
// unlock_base and await_synch spell out inline. Still archived there
// (FILE-only, so no claim is landmined); the declaration in netdaemon.h is
// what lets lock_veh and the Console lock bodies reach it with the image's
// `E8`.


// ?unlock@NetDaemon@@QAEXXZ (0x00530D00) and ?close_channel@NetDaemon@@QAEXXZ
// (0x00530600) WERE HERE and are gone: both are homed into src/netdaemon.cpp,
// whose diplo_unlock and diplomacy_ends callers needed them.

// Three forwarders the daemon's lock and synch bodies need. All three
// targets are FILE-only in the archives, so none of them landmines a claim.
// (check_spock and help_spock were the other two and are homed into
// netdaemon.cpp instead - a forwarder beside a compiled BYTE_EXACT claim is
// the one thing address_index refuses.)
// check_spock (0x0051F1B0) WERE HERE and is gone: the BYTE_EXACT body is
// homed into src/netdaemon.cpp, whose wait loops call it, and a forwarder
// beside a compiled claim is the one thing address_index refuses.

void __cdecl action_airdrop(int faction_id, int x, int y, int mode) {
    // ?action_airdrop@@YAHHHHH@Z at 0x004CC360, still in veh.cpp's region
    typedef void(__cdecl *pending)(int, int, int, int);
    PENDING_BODY(0x004CC360, pending)(faction_id, x, y, mode);
}

int __cdecl order_veh(int veh_id, int x, int y) {
    // ?order_veh@@YAHHHH@Z at 0x005947C0
    typedef int(__cdecl *pending)(int, int, int);
    return PENDING_BODY(0x005947C0, pending)(veh_id, x, y);
}

// NetFifo::sub_634200 (0x00634200) and NetFifo::sub_6340c0 (0x006340C0) WERE
// HERE and are gone: both bodies are homed into src/leaf_recoveries.cpp
// 2026-09-27. Their receiver is the queue at Net+0x10C, and Net::sub_630980 /
// Net::sub_6309c0 still reach them by name.

int ServiceStruct::add(int slot) {
    // ?add@ServiceStruct@@QAEHH@Z at 0x00401760, archived beside
    // stringstruct.cpp's class. Net's enumeration callback appends through it.
    typedef int(__fastcall *pending)(ServiceStruct *, void *, int);
    return PENDING_BODY(0x00401760, pending)(this, nullptr, slot);
}

int AlphaNet::setup(int option) {  // ?setup@AlphaNet@@QAEHH@Z at 0x004E31E0
    typedef int(__fastcall *pending)(AlphaNet *, void *, int);
    return PENDING_BODY(0x004E31E0, pending)(this, nullptr, option);
}



void __cdecl atrocity(int faction_id, int faction_id_victim, int kind, int unk) {
    // ?atrocity@@YAXHHHH@Z at 0x0055EEE0, still archived (1391 bytes,
    // src/recovered/units/0055eee0.cpp). Not claimed, so this forwarder is the
    // only definition and faults loudly if reached before that archive is
    // homed. action_oblit commits the atrocity the obliteration of a base is.
    typedef void(__cdecl *pending)(int, int, int, int);
    PENDING_BODY(0x0055EEE0, pending)(faction_id, faction_id_victim, kind, unk);
}

// --- callees of the console turn/editor bodies, still archived ---

// ?world_fungus@@YAXXZ at 0x005C3440 - HOMED into src/map.cpp 2026-09-05 over
// the real Fractal class (src/fractal.h), so this file no longer carries it.
// Console::editor_fungus still reaches it by name.

// ?set_view@Console@@QAEXH@Z at 0x00511480 - Console's own view-mode setter,
// archived. Console::editor_lock2 calls it when the cursor vehicle is live;
// the forwarder goes when the claimed body homes.
//             body homed into src/console.cpp (set_view, BYTE_EXACT)


// ?set_center@MapWin@@QAEXHHH@Z at 0x0046B1F0 - recenter the map.
//             body in src/recovered/units/0046b1f0.cpp
void MapWin::set_center(int x_coord, int y_coord,
                        int draw_type) {  // 0x0046B1F0
    typedef void(__fastcall *pending)(MapWin *, void *, int, int, int);
    PENDING_BODY(0x0046B1F0, pending)(this, nullptr, x_coord, y_coord,
                                     draw_type);
}

// ?veh_init@@YAHHHHH@Z at 0x005C03D0 - world_fungus (map.cpp) is its only
// caller in the product, seeding a fungus vehicle on the tiles it just
// painted. Archived transcription still measures NOT_MATCHING, so this is
// the forwarder and not a homed body.
int __cdecl veh_init(int unit_type, int faction_id, int x_coord, int y_coord) {
    typedef int(__cdecl *pending)(int, int, int, int);
    return PENDING_BODY(0x005C03D0, pending)(unit_type, faction_id, x_coord,
                                             y_coord);
}

// ?world_erosion@@YAXXZ at 0x005C2CB0 - world_build (map.cpp) calls it between
// build_hills and the polar-cap pass. Its archived transcription is a
// goto/label Ghidra shape that has never been rewritten in tree style, so
// this is the forwarder and not a homed body.
void __cdecl world_erosion() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005C2CB0, pending)();
}

// ?enemy_move@@YAHH@Z at 0x0056B5B0 - the AI's per-vehicle move driver, the
// one callee enemy_veh (general.cpp) was homed without. 56 KB of image span,
// still the original's own bytes, so this is the forwarder and not a homed
// body. base_terraform also reaches it by name (terraforming.cpp).
int __cdecl enemy_move(int veh_id) {
    typedef int(__cdecl *pending)(int);
    return PENDING_BODY(0x0056B5B0, pending)(veh_id);
}

// ?action@NetDaemon@@QAEHHH@Z at 0x00532570 - the daemon's side of running a
// vehicle's order, reached by enemy_veh (general.cpp) once it has awaited the
// synch. Still the image's own bytes, so this is the forwarder and not a homed
// body.
int NetDaemon::action(int veh_id, int mode) {
    typedef int(__fastcall *pending)(NetDaemon *, void *, int, int);
    return PENDING_BODY(0x00532570, pending)(this, nullptr, veh_id, mode);
}


// ?upgrade_prototype@@YAHHHHH@Z at 0x004EFE80 - the single-prototype upgrade
// worker, still unrecovered. Its only homed caller is base.cpp's
// upgrade_prototypes (0x004F0460), which the image calls with FOUR pushes
// (`add esp, 0x10` at 0x004F0620) against a five-H symbol, so the shipped
// image's own fifth parameter is read off the caller's stack. Reproduced as
// four: a fifth declared parameter no caller passes would be a modelled
// argument the image never sends. See base.cpp's note at the call site.
int __cdecl upgrade_prototype(int base_id, int faction_id, int proto_id,
                              int set_flag) {
    typedef int(__cdecl *pending)(int, int, int, int);
    return PENDING_BODY(0x004EFE80, pending)(base_id, faction_id, proto_id,
                                             set_flag);
}
