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
#include "spritebox.h"
#include "dialogs.h"
#include "listbox.h"
#include "tutwin.h"
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
#include "fx.h"
#include "hypothesis_layouts.h"
#include "statuswin.h"
#include "console.h"
#include "buttongroup.h"
#include "datalink.h"
#include "gamma.h"
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
 * HOW ONE LEAVES. When a body lands in a compiled unit, DELETE its forwarder.
 * The linker enforces that: two definitions of one symbol is LNK2005, so a
 * stale forwarder cannot survive the promotion it was waiting for. That is
 * the whole maintenance rule, and it is mechanical rather than remembered.
 *
 * WHAT THEY DO AT RUNTIME. Jump into an address that means nothing in this
 * process, and fault. That is deliberate: a forwarder that returned a
 * plausible success would let the startup sequence run past the first thing
 * that is missing, and the point of an entry point is to say exactly how far
 * the recovery reaches.
 */

#define PENDING_BODY(address, signature) \
    reinterpret_cast<signature>(static_cast<unsigned long>(address))

// sub_627d00 at 0x00627D00 - applies three Euler angles to a matrix.
void __cdecl caviar_apply_rotation(float *angles, void *matrix) {
    typedef void(__cdecl *pending)(float *, void *);
    PENDING_BODY(0x00627D00, pending)(angles, matrix);
}

// sub_638430 at 0x00638430 - the helper that walks a Caviar record and frees
// its members. A free function, so the forwarder is one too.
void __cdecl caviar_free_record(void *record) {
    typedef void(__cdecl *pending)(void *);
    PENDING_BODY(0x00638430, pending)(record);
}

// ?draw@Sprite@@QAEHPAUBuffer@@HHH@Z at 0x005E4B9A - 3225 bytes, the blitter
// the six-argument overload wraps with a substituted origin. Called by name,
// so it gets a forwarder rather than a pointer.
//             body in src/unrecovered/005e4b9a.cpp
int Sprite::draw(Buffer *buffer, int a, int b, int c) {
    typedef int(__fastcall *pending)(Sprite *, void *, Buffer *, int, int, int);
    return PENDING_BODY(0x005E4B9A, pending)(this, nullptr, buffer, a, b, c);
}

// ?exec@BasePop@@QAEHHP6AHXZ@Z at 0x00602600 - the modal loop both `exec`
// overloads wrap.
//             body in src/unrecovered/00602600.cpp
int BasePop::exec(int flags, int(__cdecl *callback)()) {
    typedef int(__fastcall *pending)(BasePop *, void *, int, int(__cdecl *)());
    return PENDING_BODY(0x00602600, pending)(this, nullptr, flags, callback);
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

// ?item@Dialog@@QAEHPADH@Z at 0x00609990 and ?item@ListBox@@QAEHPADH@Z at
// 0x0060C920 - the two item adders `Dialogs::item` dispatches to by kind.
int Dialog::item(char *text, int index) {
    typedef int(__fastcall *pending)(Dialog *, void *, char *, int);
    return PENDING_BODY(0x00609990, pending)(this, nullptr, text, index);
}

int ListBox::item(char *text, int index) {
    typedef int(__fastcall *pending)(ListBox *, void *, char *, int);
    return PENDING_BODY(0x0060C920, pending)(this, nullptr, text, index);
}

// 0x00611240 - SpriteBox::on_right_down, dispatched to by kind from dialogs.cpp.
void SpriteBox::on_right_down(int a, int b) {
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x00611240, pending)(this, nullptr, a, b);
}

// 0x00611330 - SpriteBox::on_right_double_click, dispatched to by kind from dialogs.cpp.
void SpriteBox::on_right_double_click(int a, int b) {
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x00611330, pending)(this, nullptr, a, b);
}

// 0x006111A0 - SpriteBox::on_left_up, dispatched to by kind from dialogs.cpp.
void SpriteBox::on_left_up(int a, int b) {
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x006111A0, pending)(this, nullptr, a, b);
}

// 0x00611290 - SpriteBox::on_right_up, dispatched to by kind from dialogs.cpp.
void SpriteBox::on_right_up(int a, int b) {
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x00611290, pending)(this, nullptr, a, b);
}

// 0x006111F0 - SpriteBox::on_right_click, dispatched to by kind from dialogs.cpp.
void SpriteBox::on_right_click(int a, int b) {
    typedef void(__fastcall *pending)(SpriteBox *, void *, int, int);
    PENDING_BODY(0x006111F0, pending)(this, nullptr, a, b);
}

// 0x0060C5D0 - ListBox::on_scrolling, dispatched to by kind from dialogs.cpp.
void ListBox::on_scrolling(int a, int b) {
    typedef void(__fastcall *pending)(ListBox *, void *, int, int);
    PENDING_BODY(0x0060C5D0, pending)(this, nullptr, a, b);
}

// 0x0060CB70 - ListBox::on_mousewheel, dispatched to by kind from dialogs.cpp.
void ListBox::on_mousewheel(int a) {
    typedef void(__fastcall *pending)(ListBox *, void *, int);
    PENDING_BODY(0x0060CB70, pending)(this, nullptr, a);
}

// ?close@BasePop@@QAEXXZ at 0x00600F00 and
// ?start@Popup@@QAEXPADPBDHPADHPAUGraphicWin@@@Z at 0x00406380 - the two
// bodies popup.cpp used to reach through pointers.
void BasePop::close() {
    typedef void(__fastcall *pending)(BasePop *, void *);
    PENDING_BODY(0x00600F00, pending)(this, nullptr);
}

void Popup::start(char *a1, const char *a2, int a3, char *a4, int a5,
                  GraphicWin *owner) {
    typedef void(__fastcall *pending)(Popup *, void *, char *, const char *,
                                      int, char *, int, GraphicWin *);
    PENDING_BODY(0x00406380, pending)(this, nullptr, a1, a2, a3, a4, a5, owner);
}

// ?pops@@YAHPADPADHPADHPAUSprite@@HHP6AHXZ@Z at 0x006276A0 - the full popup
// entry every `pop` wrapper funnels into.
//             body in src/unrecovered/006276a0.cpp
int __cdecl pops(char *caption, char *label, int a3, char *a4, int a5,
                 Sprite *sprite, int a7, int a8, int (__cdecl *callback)()) {
    typedef int(__cdecl *pending)(char *, char *, int, char *, int, Sprite *,
                                  int, int, int (__cdecl *)());
    return PENDING_BODY(0x006276A0, pending)(caption, label, a3, a4, a5,
                                             sprite, a7, a8, callback);
}

// ?X_pops@@... at 0x005BF930 and ?X_pop@@... at 0x005BF480 - the two full
// builders every xpops wrapper funnels into.
int __cdecl X_pops(char *caption, const char *label, int a3, char *a4, int a5,
                   Sprite *sprite, int a7, int a8, int (__cdecl *callback)()) {
    typedef int(__cdecl *pending)(char *, const char *, int, char *, int,
                                  Sprite *, int, int, int (__cdecl *)());
    return PENDING_BODY(0x005BF930, pending)(caption, label, a3, a4, a5,
                                             sprite, a7, a8, callback);
}

int __cdecl X_pop(char *caption, const char *label, int a3, char *a4, int a5,
                  int (__cdecl *callback)()) {
    typedef int(__cdecl *pending)(char *, const char *, int, char *, int,
                                  int (__cdecl *)());
    return PENDING_BODY(0x005BF480, pending)(caption, label, a3, a4, a5,
                                             callback);
}

// Five more bodies the tree called through function pointers. Each is
// declared on its class now, so the call sites emit `call rel32`.
void Dialog::close() {                                  // 0x00608F50
    typedef void(__fastcall *pending)(Dialog *, void *);
    PENDING_BODY(0x00608F50, pending)(this, nullptr);
}

void MainInterface::set_date(char *text) {              // 0x0045BE80
    typedef void(__fastcall *pending)(MainInterface *, void *, char *);
    PENDING_BODY(0x0045BE80, pending)(this, nullptr, text);
}

int Buffer::fill(int left, int top, int width, int height, int color) {
    typedef int(__fastcall *pending)(Buffer *, void *, int, int, int, int,
                                     int);                // 0x005D8240
    return PENDING_BODY(0x005D8240, pending)(this, nullptr, left, top, width,
                                             height, color);
}

int TutWin::tut_win(void *owner, const char *text, int a3, int a4,
                    Sprite *sprite, int a6, int a7, int a8) {  // 0x004BDFE0
    typedef int(__fastcall *pending)(TutWin *, void *, void *,
                                     const char *, int, int, Sprite *, int,
                                     int, int);
    return PENDING_BODY(0x004BDFE0, pending)(this, nullptr, owner, text, a3,
                                             a4, sprite, a6, a7, a8);
}

int Buffer::map_colors(int a1, int a2, int a3, int a4, void *table) {
    typedef int(__fastcall *pending)(Buffer *, void *, int, int, int, int,
                                     void *);              // 0x005DA330
    return PENDING_BODY(0x005DA330, pending)(this, nullptr, a1, a2, a3, a4,
                                             table);
}

void GraphicWin::overlay_nonclient(RECT *area) {           // 0x005D6AC0
    typedef void(__fastcall *pending)(GraphicWin *, void *, RECT *);
    PENDING_BODY(0x005D6AC0, pending)(this, nullptr, area);
}

void BaseWin::click(int a1, int a2, int a3, int a4) {       // 0x004165D0
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x004165D0, pending)(this, nullptr, a1, a2, a3, a4);
}

void BaseWin::iface_click(int a1, int a2, int a3, int a4) {                       // 0x004160F0
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x004160F0, pending)(this, nullptr, a1, a2, a3, a4);
}

void BaseWin::draw_supported(int a1) {                       // 0x0040C850
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    PENDING_BODY(0x0040C850, pending)(this, nullptr, a1);
}

void BaseWin::draw_facilities(int a1) {                       // 0x0040FCC0
    typedef void(__fastcall *pending)(BaseWin *, void *, int);
    PENDING_BODY(0x0040FCC0, pending)(this, nullptr, a1);
}

void BaseWin::garrison_click(int a1, int a2, int a3, int a4) {                       // 0x0040B140
    typedef void(__fastcall *pending)(BaseWin *, void *, int, int, int, int);
    PENDING_BODY(0x0040B140, pending)(this, nullptr, a1, a2, a3, a4);
}

int Win::init(int a1, int a2, int a3, int a4, LPSTR a5, int a6, Win * a7, Menu * a8, BorderSizing * a9) {  // 0x005EBD80
    typedef int(__fastcall *pending)(Win *, void *, int, int, int, int, LPSTR, int, Win *, Menu *, BorderSizing *);
    return PENDING_BODY(0x005EBD80, pending)(this, nullptr, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

void GraphicWin::compute_min_size() {  // 0x005D7030
    typedef void(__fastcall *pending)(GraphicWin *, void *);
    PENDING_BODY(0x005D7030, pending)(this, nullptr);
}

void Win::nonclient_to_client(int * a1, int * a2) {  // 0x005EEF60
    typedef void(__fastcall *pending)(Win *, void *, int *, int *);
    PENDING_BODY(0x005EEF60, pending)(this, nullptr, a1, a2);
}

void NetDaemon::process_message(char *message, unsigned long a, int b) {  // 0x00534400
    typedef void(__fastcall *pending)(NetDaemon *, void *, char *, unsigned long, int);
    PENDING_BODY(0x00534400, pending)(this, nullptr, message, a, b);
}

void NetDaemon::synch(int16_t opcode, int a, int b, int c, char *text, int d, int16_t flags) {  // 0x00532E00
    typedef void(__fastcall *pending)(NetDaemon *, void *, int16_t, int, int, int, char *, int, int16_t);
    PENDING_BODY(0x00532E00, pending)(this, nullptr, opcode, a, b, c, text, d, flags);
}

void FX::play(int effect) {  // 0x00446A00
    typedef void(__fastcall *pending)(FX *, void *, int);
    PENDING_BODY(0x00446A00, pending)(this, nullptr, effect);
}

void PrefWin::display(int page) {  // 0x0048FA00
    typedef void(__fastcall *pending)(PrefWin *, void *, int);
    PENDING_BODY(0x0048FA00, pending)(this, nullptr, page);
}

void StatusWin::redraw() {  // 0x004B9EA0
    typedef void(__fastcall *pending)(StatusWin *, void *);
    PENDING_BODY(0x004B9EA0, pending)(this, nullptr);
}

void Console::cursor_next(int x_coord, int y_coord) {  // 0x005109B0
    typedef void(__fastcall *pending)(Console *, void *, int, int);
    PENDING_BODY(0x005109B0, pending)(this, nullptr, x_coord, y_coord);
}

int ButtonGroup::button_click(int a1) {  // 0x0062B8A0
    typedef int(__fastcall *pending)(ButtonGroup *, void *, int);
    return PENDING_BODY(0x0062B8A0, pending)(this, nullptr, a1);
}

void Datalink::exec(unsigned int topic, int index) {  // 0x00429180
    typedef void(__fastcall *pending)(Datalink *, void *, unsigned int, int);
    PENDING_BODY(0x00429180, pending)(this, nullptr, topic, index);
}

void Gamma::adjust_palette() {  // 0x005C9520
    typedef void(__fastcall *pending)(Gamma *, void *);
    PENDING_BODY(0x005C9520, pending)(this, nullptr);
}

void InfoWin::timer_proc(int a1) {  // 0x00459150
    typedef void(__fastcall *pending)(InfoWin *, void *, int);
    PENDING_BODY(0x00459150, pending)(this, nullptr, a1);
}

void StringBox::add_fixup() {  // 0x00629490
    typedef void(__fastcall *pending)(StringBox *, void *);
    PENDING_BODY(0x00629490, pending)(this, nullptr);
}

void WorldWin::click(int a1, int a2, int button, int is_double) {  // 0x004C3D40
    typedef void(__fastcall *pending)(WorldWin *, void *, int, int, int, int);
    PENDING_BODY(0x004C3D40, pending)(this, nullptr, a1, a2, button, is_double);
}

void BaseButton::timer_callback(int a1) {  // 0x00607B30
    typedef void(__fastcall *pending)(BaseButton *, void *, int);
    PENDING_BODY(0x00607B30, pending)(this, nullptr, a1);
}

void DipEdit::read_check() {  // 0x004DA990
    typedef void(__fastcall *pending)(DipEdit *, void *);
    PENDING_BODY(0x004DA990, pending)(this, nullptr);
}

void DipEdit::do_check() {  // 0x004DADA0
    typedef void(__fastcall *pending)(DipEdit *, void *);
    PENDING_BODY(0x004DADA0, pending)(this, nullptr);
}

int Dialog::init(int a1) {  // 0x006095F0
    typedef int(__fastcall *pending)(Dialog *, void *, int);
    return PENDING_BODY(0x006095F0, pending)(this, nullptr, a1);
}

int Dialog::init(RECT * a1, int a2) {  // 0x00609470
    typedef int(__fastcall *pending)(Dialog *, void *, RECT *, int);
    return PENDING_BODY(0x00609470, pending)(this, nullptr, a1, a2);
}

int Dialog::init(Heap * a1) {  // 0x006098D0
    typedef int(__fastcall *pending)(Dialog *, void *, Heap *);
    return PENDING_BODY(0x006098D0, pending)(this, nullptr, a1);
}

int Dialog::init(int a1, int a2, int a3, int a4, Heap * a5) {  // 0x00609730
    typedef int(__fastcall *pending)(Dialog *, void *, int, int, int, int, Heap *);
    return PENDING_BODY(0x00609730, pending)(this, nullptr, a1, a2, a3, a4, a5);
}

void Datalink::draw_entry() {  // 0x0042BF10
    typedef void(__fastcall *pending)(Datalink *, void *);
    PENDING_BODY(0x0042BF10, pending)(this, nullptr);
}

void DesignWin::select_special(int a1) {  // 0x0043AC70
    typedef void(__fastcall *pending)(DesignWin *, void *, int);
    PENDING_BODY(0x0043AC70, pending)(this, nullptr, a1);
}

void CouncWin::draw_leader(int factionID) {  // 0x00425DB0
    typedef void(__fastcall *pending)(CouncWin *, void *, int);
    PENDING_BODY(0x00425DB0, pending)(this, nullptr, factionID);
}

void GraphicWin::soft_update() {  // 0x005D5890
    typedef void(__fastcall *pending)(GraphicWin *, void *);
    PENDING_BODY(0x005D5890, pending)(this, nullptr);
}

void GraphicWin::update(GraphicWin * a1) {  // 0x005D56B0
    typedef void(__fastcall *pending)(GraphicWin *, void *, GraphicWin *);
    PENDING_BODY(0x005D56B0, pending)(this, nullptr, a1);
}

void FileWin::UNK4() {  // 0x006146A0
    typedef void(__fastcall *pending)(FileWin *, void *);
    PENDING_BODY(0x006146A0, pending)(this, nullptr);
}

int Midi_Device::init(void * a1, unsigned long a2) {  // 0x004C57A0
    typedef int(__fastcall *pending)(Midi_Device *, void *, void *, unsigned long);
    return PENDING_BODY(0x004C57A0, pending)(this, nullptr, a1, a2);
}

int Wave_In_Device::init(void * a1, unsigned long a2) {  // 0x004C5A10
    typedef int(__fastcall *pending)(Wave_In_Device *, void *, void *, unsigned long);
    return PENDING_BODY(0x004C5A10, pending)(this, nullptr, a1, a2);
}

void Menu::mouse_move(int a1, int a2) {  // 0x005FBDB0
    typedef void(__fastcall *pending)(Menu *, void *, int, int);
    PENDING_BODY(0x005FBDB0, pending)(this, nullptr, a1, a2);
}

int Sprite::extract(Buffer * a1, int a2, int a3, int a4, int a5, int a6, TexHeap * a7) {  // 0x005E39A0
    typedef int(__fastcall *pending)(Sprite *, void *, Buffer *, int, int, int, int, int, TexHeap *);
    return PENDING_BODY(0x005E39A0, pending)(this, nullptr, a1, a2, a3, a4, a5, a6, a7);
}

void ReportWin::sat_anim() {  // 0x0049FE40
    typedef void(__fastcall *pending)(ReportWin *, void *);
    PENDING_BODY(0x0049FE40, pending)(this, nullptr);
}

void ReportWin::exp_anim() {  // 0x004A0100
    typedef void(__fastcall *pending)(ReportWin *, void *);
    PENDING_BODY(0x004A0100, pending)(this, nullptr);
}

void MainInterface::on_iface_button_clicked(int a1) {  // 0x0045D170
    typedef void(__fastcall *pending)(MainInterface *, void *, int);
    PENDING_BODY(0x0045D170, pending)(this, nullptr, a1);
}

void ReportIf::bl_anim() {  // 0x004A4060
    typedef void(__fastcall *pending)(ReportIf *, void *);
    PENDING_BODY(0x004A4060, pending)(this, nullptr);
}

void Win::hide() {  // 0x005EDCD0
    typedef void(__fastcall *pending)(Win *, void *);
    PENDING_BODY(0x005EDCD0, pending)(this, nullptr);
}

void Win::on_mouse_move(int a1, int a2, unsigned int a3, int a4) {  // 0x005F6320
    typedef void(__fastcall *pending)(Win *, void *, int, int, unsigned int, int);
    PENDING_BODY(0x005F6320, pending)(this, nullptr, a1, a2, a3, a4);
}

int Win::on_nc_hittest(int a1, int a2) {  // 0x005F5AD0
    typedef int(__fastcall *pending)(Win *, void *, int, int);
    return PENDING_BODY(0x005F5AD0, pending)(this, nullptr, a1, a2);
}

void Win::release_modal() {  // 0x005EE280
    typedef void(__fastcall *pending)(Win *, void *);
    PENDING_BODY(0x005EE280, pending)(this, nullptr);
}

int SetupWin::do_menu_rightside(char *a1, int a2) {  // 0x004ADB70
    typedef int(__fastcall *pending)(SetupWin *, void *, char *, int);
    return PENDING_BODY(0x004ADB70, pending)(this, nullptr, a1, a2);
}

void SpriteBox::close() {  // 0x00610280
    typedef void(__fastcall *pending)(SpriteBox *, void *);
    PENDING_BODY(0x00610280, pending)(this, nullptr);
}

void Scroll::on_mousewheel_down(int a1) {  // 0x00606320
    typedef void(__fastcall *pending)(Scroll *, void *, int);
    PENDING_BODY(0x00606320, pending)(this, nullptr, a1);
}

void Scroll::on_mousewheel_up(int a1) {  // 0x00606440
    typedef void(__fastcall *pending)(Scroll *, void *, int);
    PENDING_BODY(0x00606440, pending)(this, nullptr, a1);
}

void ReplayWin::timer_callback() {  // 0x005AD9E0
    typedef void(__fastcall *pending)(ReplayWin *, void *);
    PENDING_BODY(0x005AD9E0, pending)(this, nullptr);
}

void CheckBox::set_state_flag(long value) {  // 0x0060ECE0
    typedef void(__fastcall *pending)(CheckBox *, void *, long);
    PENDING_BODY(0x0060ECE0, pending)(this, nullptr, value);
}

// ?write_raw_l@Buffer@@QAEHPADHHH@Z at 0x005DBD00 - 1475 bytes, the raster
// writer that puts one single-font run on the surface. Called by name from
// `write_multi_font_raw_l`, which is promoted; a pointer here would cost that
// body the `E8` it now emits.
//             body in src/unrecovered/005dbd00.cpp
int Buffer::write_raw_l(LPSTR text, int x_coord, int y_coord, int len) {
    typedef int(__fastcall *pending)(Buffer *, void *, LPSTR, int, int, int);
    return PENDING_BODY(0x005DBD00, pending)(this, nullptr, text, x_coord,
                                             y_coord, len);
}

// ?jackal_close@@YAXXZ at 0x0062D500 - body in src/recovered/0062d500.cpp
void __cdecl jackal_close() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0062D500, pending)();
}

// ?init_class@Caviar@@QAAHXZ at 0x006185A0
//             body in src/unrecovered/006185a0.cpp
int Caviar::init_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x006185A0, pending)();
}

// ?close_class@Caviar@@QAAXXZ at 0x00618D20
//             body in src/recovered/units/00618d20.cpp
void Caviar::close_class() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x00618D20, pending)();
}

// ---------------------------------------------------------------------------
// PROMOTED OUT OF temp.h, where they were function POINTERS. A pointer
// binding compiles `call dword ptr [draw_map]` where the image has
// `call rel32`, so every caller of one reads as a mismatch however right its
// body is - that is what held `Console::editor_polar` and
// `Console::editor_climate` off the ratchet. The address is the same one
// src/temp.cpp bound, so nothing about runtime changed; only the call shape.
// ---------------------------------------------------------------------------

// ?draw_map@@YAXH@Z at 0x0046B190 - body in src/recovered/units/0046b190.cpp
void __cdecl draw_map(int draw_type) {
    typedef void(__cdecl *pending)(int);
    PENDING_BODY(0x0046B190, pending)(draw_type);
}

// ?world_climate@@YAXXZ at 0x005C5A30 - body in src/unrecovered/005c5a30.cpp
void __cdecl world_climate() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x005C5A30, pending)();
}

// ?custom_planet@@YAHHH@Z at 0x0058C2A0 - body in src/unrecovered/0058c2a0.cpp
int __cdecl custom_planet(int a, int b) {
    typedef int(__cdecl *pending)(int, int);
    return PENDING_BODY(0x0058C2A0, pending)(a, b);
}

// ?control_game@@YAXXZ at 0x0052AA30 - body in src/unrecovered/0052aa30.cpp
void __cdecl control_game() {
    typedef void(__cdecl *pending)();
    PENDING_BODY(0x0052AA30, pending)();
}

// ??0BasePop@@QAE@XZ at 0x00600860 - body in src/unrecovered/00600860.cpp
//
// THE FRONTIER MOVED HERE when basepop_alloc was recovered into
// src/basepop.cpp: `new BasePop()` needs a constructor to link against, and
// basepop.h no longer defines one inline. The forwarder that used to sit here
// for basepop_alloc itself is gone - the linker enforces that, since two
// definitions of one symbol is LNK2005.
BasePop::BasePop() {
    typedef void(__cdecl *pending)(void *);
    PENDING_BODY(0x00600860, pending)(this);
}

// ---------------------------------------------------------------------------
// THE FRONTIER AFTER jackal_init_real, which was recovered into
// src/general.cpp on 2026-08-15. It calls nineteen functions; five were
// already compiled and these fourteen were not, so promoting one body moved
// the edge outward by fourteen. Each is one `#include` and one body away from
// deleting its line here.
// ---------------------------------------------------------------------------

// ?set_display_mode@Win@@QAAHHHHH@Z at 0x005F2C40
int Win::set_display_mode(int width, int height, int depth, int tgl) {
    typedef int(__cdecl *pending)(int, int, int, int);
    return PENDING_BODY(0x005F2C40, pending)(width, height, depth, tgl);
}


// What `Win::window_proc` routes to, now that it is promoted into
// src/win.cpp. These six are what its recovery still stands on, and the
// first thing that faults if you move the mouse or press a key.
//
// ?recurse_zorder@@YAXPAUWin@@@Z at 0x005F4EC0
void __cdecl recurse_zorder(Win *window) {
    typedef void(__cdecl *pending)(Win *);
    PENDING_BODY(0x005F4EC0, pending)(window);
}

// ?get_key_window@Win@@QAGHXZ at 0x005F6A50
Win *Win::get_key_window() {
    typedef Win *(__stdcall *pending)();
    return PENDING_BODY(0x005F6A50, pending)();
}

// ?screen_to_client@Win@@QAEXPAHPAH@Z at 0x005ED2D0 - BYTE_EXACT already in
// src/unrecovered/005ed2d0.cpp, and 133 bytes, so this edge is cheap to
// close next.
void Win::screen_to_client(int *x, int *y) {
    typedef void(__fastcall *pending)(Win *, void *, int *, int *);
    PENDING_BODY(0x005ED2D0, pending)(this, nullptr, x, y);
}

// ?get_mouse_window_recurse@@YAHPAUWin@@PAHPAH@Z at 0x005F6AB0 - the tree walk
// `get_mouse_window` delegates to, 1110 bytes.
Win *__cdecl get_mouse_window_recurse(Win *window, int *x, int *y) {
    typedef Win *(__cdecl *pending)(Win *, int *, int *);
    return PENDING_BODY(0x005F6AB0, pending)(window, x, y);
}

// ?update_cursor@Win@@QAAHPAUWin@@H@Z at 0x005F1820
int Win::update_cursor(Win *window, int tgl) {
    typedef int(__cdecl *pending)(Win *, int);
    return PENDING_BODY(0x005F1820, pending)(window, tgl);
}

// ?update_screen@Win@@QAAHPAURECT@@PAVWin@@@Z at 0x005F7320
int Win::update_screen(RECT *area, Win *window) {
    typedef int(__cdecl *pending)(RECT *, Win *);
    return PENDING_BODY(0x005F7320, pending)(area, window);
}

// ?do_tracking@Win@@QAEXHH@Z at 0x005F7580 - the one __thiscall member of the
// set, so the forwarder hands the receiver over explicitly.
void Win::do_tracking(int x, int y) {
    typedef void(__fastcall *pending)(Win *, void *, int, int);
    PENDING_BODY(0x005F7580, pending)(this, nullptr, x, y);
}

// sub_5f86a0 at 0x005F86A0 - byte-exact in src/recovered/005f86a0.cpp, which
// is in no build, so the edge is still pending here.
extern "C" void __stdcall sub_5f86a0(int a1) {
    typedef void(__stdcall *pending)(int);
    PENDING_BODY(0x005F86A0, pending)(a1);
}

// ?init_cursor_class@Cursor@@QAAXXZ at 0x0063B910 - `int` here, see cursor.h
int Cursor::init_cursor_class() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B910, pending)();
}

// ?trig_init@@YAHXZ at 0x0063B940
int __cdecl trig_init() {
    typedef int(__cdecl *pending)();
    return PENDING_BODY(0x0063B940, pending)();
}
