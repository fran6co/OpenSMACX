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
#pragma once

#include "original_seam.h"
#include "mapwin.h"
#include "menu.h"     // menu_
#include "sprite.h"   // menu_sprite_, menu_sprites_[3], sprite_a_/sprite_b_
#include "time.h"     // time_

class StatusWin;

class PrefWin;

class InfoWin;

 /*
  * Console class
  *
  * The largest unrecovered class in the image - 132KB across 148 methods -
  * and the reason the layout campaign exists.
  *
  * The base is MapWin, not GraphicWin. ??0Console@@QAE@H@Z reaches
  * ??0MapWin@@QAE@H@Z at 0x0050F4AE with `mov ecx, esi` and no adjustment -
  * esi is the unmodified `this` saved at 0x0050F483 - so a MapWin subobject
  * begins at offset 0, and GraphicWin is reached THROUGH it. The header used
  * to say `: virtual GraphicWin`, which is a level too shallow; the edge is
  * still virtual, just one link further down (`MapWin : virtual GraphicWin`).
  *
  * The two comments this replaces were both withdrawn by measurement:
  *
  *   - "that cannot be written as `: virtual GraphicWin` here ... the base is
  *     held as a member at the offset MSVC put it" was already false when it
  *     was read: the class it sat on top of spelled `: virtual GraphicWin`
  *     and held no such member. VC6 puts a virtual base after the derived
  *     members exactly where MSVC's vbtable says, so the declaration and the
  *     image agree and the warning had nothing left to warn about.
  *   - "MapWin's own fields occupy the front of derived_storage_" is now the
  *     declaration rather than a note about it.
  *
  * The size is read rather than inferred: the vbtable at 0x0066EF04 - stored
  * into [this] at 0x0050F49A - reads {0, 0x23D94}, and GraphicWin is pinned
  * at 0xA14, summing to the 0x247A8 asserted below. The constructor builds
  * that GraphicWin at this+0x23D94 (0x0050F4A0) under the most-derived flag,
  * which is what makes it the virtual base rather than a member.
  * Independently g_CONSOLE's global slot bounds the object above at 0x247D8,
  * consistent with 0x30 to spare - two readings sharing no evidence.
  *
  * WHERE THE BASE SUBOBJECT ENDS, and the one place this spelling is four
  * bytes off. A standalone MapWin puts its virtual base at 0x21A6C (vbtable
  * 0x0066C870, {0, 0x21A6C}), so mapwin.h declares storage right up to
  * 0x21A6C. Inside a Console the virtual base has moved to the end, and the
  * MapWin subobject stops at 0x21A68: the constructor builds an
  * unconditional GraphicWin at this+0x21A68 (0x0050F4BC), 0xA14 bytes, which
  * ends at 0x2247C - exactly where the next member starts (Buffer at
  * this+0x2247C, 0x0050F4CB). Those four bytes are the vtordisp MSVC reserves
  * immediately ahead of a virtual base, and it belongs to the MOST DERIVED
  * object, so in a Console it moves to 0x23D90: 0x0050F57E reads the vbase
  * displacement out of the vbtable and writes `[esi + 0x23D94 - 4]` at
  * 0x0050F584.
  * field_23D90_ below WAS that slot; the compiler owns it now (see the
  * correction below), and MapWin no longer declares a counterpart either.
  *
  * SUPERSEDED 2026-08-25, and the correction is the whole point of the
  * Win pass. This block used to read "VC6 will not emit a vtordisp of its
  * own - it only does so where a virtual base's virtual functions are
  * overridden, and nothing in this chain declares a single `virtual`", and
  * concluded that mapwin.h therefore had to hold the four bytes as a
  * declared member. The premise was true when it was written and is false
  * now: win.h declares its 88 vtable slots, so the chain overrides plenty
  * and VC6 emits the vtordisp itself.
  *
  * What that changes here: the compiler emits ONE vtordisp per most-derived
  * object. In a Console the MapWin base is not most-derived, so it gets
  * none and the subobject stops at the image's 0x21A68 - the slab below
  * starts there rather than at 0x21A6C. Console's own vtordisp, at 0x23D90,
  * is likewise emitted rather than declared. Both edits are four bytes in
  * opposite directions, which is exactly why sizeof(Console) never moved
  * and the static_assert could not see the damage; `cursor_next` could,
  * reading [eax*4 + 0x23C08] against the image's 0x23C0C. The recorded
  * lesson `sizeof-cannot-pin-a-layout` is this incident's second instance.
  *
  * Fields must be carved out of the slabs as methods are recovered, keeping
  * the total fixed. Appending would move the virtual base and break every
  * offset in the class.
  */
class Console : public MapWin {
 public:
  // 0x0050F960, a pending_bodies forwarder.
  void on_post_redraw_nc_buffer(void * a1, int a2);

 public:
  // 0x00510110, a pending_bodies forwarder.
  void on_nc_mouse_move(int a1, int a2);

 public:
  // 0x0050FE70, a pending_bodies forwarder.
  void on_nc_left_up(int a1, int a2);

 public:
  // 0x0050FB10, a pending_bodies forwarder.
  void on_nc_left_down(int a1, int a2);

 public:
  // 0x0050F680, a pending_bodies forwarder.
  int on_nc_hittest(int x, int y);

 public:
  // 0x005178C0, a pending_bodies forwarder.
  int on_key_click(int a, int b);

 public:
  void cursor_next(int x_coord, int y_coord);

 public:
  void editor_polar();
  void on_sys_close();
  void menu_update();
  void editor_redo();
  void editor_climate();
  Console() { ; }
  // See the note in `mapwin.h`: a `construct` method rather than a real
  // constructor, because VC6's hidden most-derived flag doubles the push.
  void construct(int input);
  // A genuine destructor is safe here (unlike the constructor): destructors
  // never carry the most-derived flag - `guarded_teardowns.cpp`'s own
  // already-matching `->Console::~Console()` proves it, the same way it did
  // for MapWin's and PlanWin's.
  // MEASURED IN-CLASS 2026-08-30, and the whole point of the member carve
  // above: the image's 0x0051D9F0 is the BASE-object destructor
  // (??_DConsole@@QAEXXZ - the complete ??1Console at 0x0051DDF0 calls it
  // and then ??1GraphicWin on the vbase; the catalogue pinned the ??1 name
  // on the base flavor). ??_DConsole's bytes are the EH frame, the two
  // vtable/vtordisp re-stores on the vbase, the body's close(), the
  // compiler's member teardown in the reverse-declaration order of the
  // carve, and MapWin's own base teardown INLINED (clear(0) and MapWin's
  // members) - all of it compiler-emitted from these declarations once the
  // body is the one line below. Out-of-line in console.cpp it measured
  // MISMATCH with every teardown hand-spelled; declared here the funclet
  // spans (0x0065D0E0-0x0065D220) come out of the same EH machinery.
  MEASURED ~Console() { close(); }
  // 0x0051D7D0. Not declared before this landed, which is why the body
  // could only live in an artifact.
  void close();
  void clear_group();
  void set_preferences();
  void set_auto_preferences();
  void set_base_preferences();
  void set_audiovisual();
  void set_map_display();
  int edit_lock();
  void set_adv_preferences();
  void editor_undo();
  void update_data(int a1);
  int focus(int x_coord, int y_coord, int faction_id);

 private:
  // Storage now starts where the MapWin base ends. Everything this class used
  // to declare below 0x21A6C - the vbtable pointer at 0, the 0x1DDA0 slab
  // after it, the dwords at 0x1DDA4 and 0x1DDA8, the 0x20 slab at 0x1DDAC,
  // the four dwords at 0x1DDCC..0x1DDD8, and the front 0x3C90 of the slab
  // that used to run 0x1DDDC..0x22A2C - is the base's, and mapwin.h
  // declares every one of those bytes itself: MapWin carries its own
  // field_1DDA4_, field_1DDA8_, field_1DDCC_, field_1DDD0_, field_1DDD4_ and
  // field_1DDD8_ at the same offsets, read independently from MapWin's own
  // methods. Two scans of the same storage agreeing is why this edge is safe
  // to declare rather than merely believable.
  //
  // What remains is the tail of that slab: [0x21A6C, 0x22A2C) is 0xFC0 bytes.
  // All but the first four of the unnamed GraphicWin at 0x21A68..0x2247C sit
  // in here - the four the base swallowed are the reason the header comment
  // says it cannot be spelled.
  // 0x21A68, FOUR BYTES WIDER than the address in its old name. MapWin
  // used to hand-declare the dword at 0x21A68 so a standalone MapWin
  // reached sizeof 0x22480; VC6 emits that vtordisp itself now that the
  // Win base declares virtuals, and it only emits one where the base is
  // MOST-DERIVED. Inside a Console it is not, so the subobject stops at
  // 0x21A68 and this slab has to start there or every field below sits
  // four bytes early - measured: cursor_next read [eax*4 + 0x23C08]
  // where the image reads 0x23C0C.
  // THE MANAGED OBJECTS, CARVED 2026-08-30 out of what was the
  // field_21A68_[0xFC4] / field_22A2C_ / field_22A30_[0x440] /
  // field_22E70_ / field_22E74_[0xD54] runs. ??0Console's own call list
  // names every one: ??0GraphicWin at +0x21A68 (0x005D4CF0), ??0Buffer at
  // +0x2247C (0x005D7210), ??0Time at +0x22A04 (0x006161D0), ??0Menu at
  // +0x22A2C (0x005FAC60), then the menu's companions at +0x23590,
  // +0x23B18 and +0x23B44 - a Buffer, a Sprite and Sprite[3] the image's
  // ??_M walk carries with ??1Sprite as its element destructor. The
  // arithmetic closes exactly: 0x21A68+0xA14(GraphicWin)=0x2247C,
  // +0x588(Buffer)=0x22A04, +0x28(Time)=0x22A2C, +0xB64(Menu)=0x23590,
  // +0x588(Buffer)=0x23B18, +0x2C(Sprite)=0x23B44, +3*0x2C=0x23BC8 where
  // the dword run the map already names picks up. Declared in the order
  // destruction reverses, so the compiler's teardown runs sprite_a_,
  // sprite_b_, menu_sprites_ (??_M), menu_sprite_, menu_buffer_, menu_,
  // time_, buffer_, graphic_win_ - ??_DConsole's own order.
  GraphicWin graphic_win_;  // 0x21A68 - Console's own second GraphicWin,
                            // beside the virtual base it inherits (whose
                            // vbtable displacement 0x23D94 is set by these
                            // very members)
  Buffer buffer_;           // 0x2247C
  Time time_;               // 0x22A04
  Menu menu_;               // 0x22A2C
  Buffer menu_buffer_;      // 0x23590
  Sprite menu_sprite_;      // 0x23B18
  Sprite menu_sprites_[3];  // 0x23B44
  uint32_t field_23BC8_;  // 0x23BC8
  uint32_t field_23BCC_;  // 0x23BCC
  uint32_t field_23BD0_;  // 0x23BD0
  uint32_t field_23BD4_;  // 0x23BD4
  uint32_t field_23BD8_;  // 0x23BD8
  uint32_t field_23BDC_;  // 0x23BDC
  uint32_t field_23BE0_;  // 0x23BE0
  uint32_t field_23BE4_;  // 0x23BE4
  uint32_t field_23BE8_;  // 0x23BE8
  uint32_t field_23BEC_;  // 0x23BEC
  uint32_t field_23BF0_;  // 0x23BF0
  uint32_t field_23BF4_;  // 0x23BF4
  uint32_t field_23BF8_;  // 0x23BF8
  uint32_t field_23BFC_;  // 0x23BFC
  uint32_t field_23C00_;  // 0x23C00
  uint32_t field_23C04_;  // 0x23C04
  uint32_t field_23C08_;  // 0x23C08
  uint32_t field_23C0C_;  // 0x23C0C
  uint8_t field_23C10_[0x100];  // 0x23C10
  uint32_t field_23D10_;  // 0x23D10
  uint32_t field_23D14_;  // 0x23D14
  uint32_t field_23D18_;  // 0x23D18
  uint32_t field_23D1C_;  // 0x23D1C
  uint8_t field_23D20_[0x4];  // 0x23D20
  uint32_t field_23D24_;  // 0x23D24
  // THE TWO SPRITES, carved from what was field_23D28_[0x58]: two
  // 0x2C-byte Sprite objects, ??_DConsole's own close() calls at
  // vbase-0x40 and vbase-0x6C (front+0x23D54, front+0x23D28). Declared
  // in the order destruction reverses.
  Sprite sprite_b_;  // 0x23D28
  Sprite sprite_a_;  // 0x23D54
  uint32_t field_23D80_;  // 0x23D80
  uint32_t field_23D84_;  // 0x23D84
  uint32_t field_23D88_;  // 0x23D88
  uint32_t field_23D8C_;  // 0x23D8C
  // The vtordisp for the virtual base at 0x23D94 USED to be declared here,
  // because nothing in the chain spelled `virtual` and VC6 emits one only
  // where a virtual base's virtuals are overridden. Win declares its 88
  // slots now, so the compiler emits this dword itself and naming it too
  // made the class four bytes long - which sizeof hid, because the same
  // four had just been lost from the MapWin subobject above. The image's
  // own idiom is still worth recording: the constructor COMPUTES the
  // value rather than storing a constant, `mov edx,[esi]` / `mov eax,
  // [edx+4]` / `lea ecx,[eax - 0x23d94]` / `mov [eax + esi - 4], ecx` at
  // 0x0050F579..0x0050F584.
};

static_assert(sizeof(Console) == 0x247A8, "Console layout must match terranx.exe");

// All five preference openers drive the one PrefWin the game keeps at a fixed
// address, opening it to a different page. PrefWin::display is not recovered.
// THE OBJECT, at 0x008578D8 in the image - REAL STORAGE, defined in
// console.cpp. It was `PrefWin *const ConsolePrefWin = (PrefWin *)0x008578D8`
// naming terranx.exe data that is unmapped in a standalone build. PrefWin has
// no constructor in this tree (an opaque layout), so the object is the zero
// storage the image leaves until its own init fills it, and every
// `ConsolePrefWin->PrefWin::display(n)` receiver keeps its folded
// `mov ecx, imm32` form, the displacement relocated.
extern PrefWin ConsolePrefWin;  // 0x008578D8


// clear_group masks a bit in each entry of a group table the game keeps at a
// fixed address, counted by another fixed-address field; both are rebindable
// so tests drive them against a local table.
//
// THE GROUP TABLE IS NOT SEPARATE STORAGE: 0x0095282C is Vehs + 4 - the
// `state` field of Vehs[0], veh.cpp's Veh array (0x00952828, 0x34 stride).
// `ConsoleGroupTable + index * 0x34` is `&Vehs[index].state`, and the bit
// clear_group drops (0x08000000) is VSTATE_UNK_8000000 in that state dword.
// clear_group walks it as an offset of Vehs now; no second object exists.
int32_t *const ConsoleGroupCount = (int32_t *)0x009A64C8;


// edit_lock consults the Scroll Lock key through the game's imported
// GetKeyState. The seam is the address of that import slot (the IAT entry the
// original calls indirectly), so it reads the live pointer at run time and
// stays rebindable for tests.
typedef SHORT(__stdcall func_get_key_state)(int virtual_key);
inline func_get_key_state *&ConsoleEditKeyStateSlot() { return *reinterpret_cast<func_get_key_state **>(0x0066932C); }


// update_data drives the three fixed windows the original reaches by absolute
// address and never through `this` - ecx is dead on entry and is overwritten
// by `mov ecx, 0x7ad2a0` before anything reads it. The InfoWin at 0x007AD2A0
// and the StatusWin at 0x008C5568 are the objects themselves (loaded as
// immediates); 0x007D3C3C is a slot holding the current MapWin pointer
// (loaded with `mov ecx, dword ptr [0x7d3c3c]`), published once at startup by
// 0x0058F1C1, so it is read at call time exactly like ConsoleEditKeyStateSlot
// above. All four bindings are rebindable so tests can substitute local
// objects and probes.
//
// InfoWin::change (0x00458900) and MapWin::main_caption (0x0046FB10) are
// source_complete and are called directly; StatusWin::redraw (0x004B9EA0) is
// still original - it pulls in the 8885-byte draw_status, two indirect calls
// through the runtime slot at 0x00669328, and GraphicWin::soft_update - so it
// is reached through a rebindable seam, exactly as ListBox reaches
// Dialog::close.
// THE OBJECTS, defined in console.cpp and statuswin.cpp - REAL STORAGE, not
// the `void *const` / `StatusWin *const` bindings that named terranx.exe
// data. The image's ??__E dynamic initializers construct both at their fixed
// addresses before WinMain (`mov ecx, 0x7ad2a0; call InfoWin::InfoWin` at
// 0x004562C0, `mov ecx, 0x8c5568; call StatusWin::StatusWin` at 0x004B3FA0,
// each registering the deleting destructor with atexit), which is what the
// real objects do through this build's own startup.
extern InfoWin ConsoleInfoWin;    // 0x007AD2A0, the process-wide InfoWin
extern StatusWin ConsoleStatusWin;  // 0x008C5568, the process-wide StatusWin
// AN LVALUE AT A FIXED ADDRESS. The MapWin POINTER lives at 0x007D3C3C and
// the image reads it with one absolute `mov ecx, [0x7d3c3c]`; spelled
// `void **const` it took two loads - the constant, then the pointer.
inline MapWin *&console_map_win() {
  return *reinterpret_cast<MapWin **>(0x007D3C3C);
}


// focus walks the eight map-window slots and, for the primary window only,
// runs the console cursor and the survey-overlay latch first. Four of its
// callees are still original bodies, so each gets a rebindable seam. Nothing
// here binds an address that is already bound: the table is mapwin.h's
// MapWinTable at 0x007D3C3C (which also carries MapWinTableSlots and
// MapWinActiveOffset) and the faction id is game.h's LocalFaction.
//
// Console::cursor_next (0x005109B0, ret 8) is entered on the process-wide
// Console the original names with `mov ecx, 0x9156b0` at 0x00510921 - never on
// `this`. That object is bound separately so a test can hand focus a Console
// that is NOT the process-wide one and observe which of the two each half of
// the body touches.
// MapWin::focus (0x0046B310, ret 8) reports whether it moved the view;
// MapWin::draw_map (0x0046A550, ret 4) repaints one. Both are unrecovered -
// focus reaches in_box and set_center, draw_map is 2049 bytes - so both stay
// original behind a seam.
// ?flush_input@@YAXXZ at 0x005FD120 - __cdecl, no arguments - drains the queued
// input once the focus walk actually moved the primary view. Still original; it
// pumps the message loop through check_net.
typedef void(__cdecl func_console_flush_input)(void);

// 0x0046A550 is NOT bound here. It is MapWin::draw_map's own address and
// src/mapwin.h owns the seam for it (MapWinOriginalDrawMap). Seams dedupe on
// the ADDRESS, never the name: two variables for one address is not a link
// error, so a fixture rebinding one would leave the other still pointing at
// the original image - which is why tools/test_generator_support.py refuses
// a second binding, and it caught this one.
// 0x005FD120, defined in src/console.cpp.
void __cdecl flush_input();
// THE OBJECT, at 0x009156B0 in the image - REAL STORAGE, defined in
// console.cpp. It was `Console *const ConsoleGlobal = (Console *)0x009156B0`
// naming terranx.exe data that is unmapped in a standalone build. Console's
// constructor here is the empty inline the tree models, so the object is the
// zero storage the image leaves until its own construct() fills it. Every
// `ConsoleGlobal->` receiver keeps its folded `mov ecx, imm32` form, the
// displacement relocated; body text that READS the console's address (the
// vbtable walk in close_opening) now takes `&ConsoleGlobal`.
extern Console ConsoleGlobal;  // 0x009156B0, the process-wide Console

// Console::cursor_next's own last-position cache: the CURRENT index and the
// two coordinate arrays it indexes, distinct from the per-object ring buffer
// at field_23C08_/field_23C0C_/field_23C10_. No established identity beyond
// what this one function shows.
extern int CursorLastIndex;    // 0x009392B8
extern int CursorLastX[32];    // 0x009392C0
extern int CursorLastY[32];    // 0x00939340

// Two fixed-address strings cursor_next passes to danger() when a debug-mode
// coordinate parity mismatch is detected. No established text.
LPCSTR const CursorNextDangerMsg1 = (LPCSTR)0x0068A5D0;
LPCSTR const CursorNextDangerMsg2 = (LPCSTR)0x00939FF0;

// The dword at 0x0093A938 is set while a turn is played out under program
// control rather than interactively. A linear scan of .text finds exactly four
// writers: control_turn stores 1 at 0x00527680 and 0 at 0x0052823B,
// net_end_of_turn stores 1 at 0x0052A1CB, and net_control_turn stores 1 at
// 0x0052A301 and 0 at 0x0052A4ED. Over a hundred sites read it, always as a
// plain zero/non-zero gate. focus consults it only at 0x00510910, to decide
// whether an untagged primary window still gets the cursor path.
int32_t *const ConsoleControlTurnActive = (int32_t *)0x0093A938;  // 0x0093A938

// on_sys_close raises the flag the turn loop watches. The dword at 0x009B2068
// is game.h's ExitTurnLoop now - a real global, defined in game.cpp when
// control_game was homed (2026-08-29); the `int32_t *const` binding that stood
// here pointed at the image's address while the binary's own storage lived
// elsewhere, and on_sys_close stores through the global with the same bytes.

// menu_update hands the console's selected-vehicle id to the MainMenu the
// original embeds at +0x22A2C. MainMenu::check (0x00460DD0, 4782 bytes) is
// unrecovered and MainMenu has no header in src/, so this is a seam rather
// than a declared type.

