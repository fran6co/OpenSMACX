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
#include "autosound.h"
#include "buffer.h"
#include "palette.h"
#include "heap.h"
#include "spot.h"
#include "sprite.h"

class Menu; // forward declaration
class ImageButton;
// The ninth Win::init / GraphicWin::init argument, spelled PAUBorderSizing@@
// in both mangled names - a struct, hence the tag.
//
// TWELVE BYTES, THREE INTS, proved by BasePop::exec (0x00602600): its frame
// `sub esp, 0x20` holds 8 locals plus exactly one 12-byte object at
// [esp+0x24] whose three dwords come from this+0x114 / +0x118 / +0x11c, i.e.
// Win's caption_height_, border_thickness_ and bottom_border_thickness_ in
// that order. A fourth field would not fit the frame, and no other body
// reads or writes the struct, so these three are the whole of it.
struct BorderSizing {
    int caption_height;
    int border_thickness;
    int bottom_border_thickness;
};
class Scroll; // forward declaration

// The embedded Win+0xC8 object, in its own TU since 2026-08-26. The class
// is carried VERBATIM - in-class destructor included, which is what makes
// `??_GWinNodeList` at 0x005F8770 measure 54/54 - see winnodelist.h.
#include "winnodelist.h"

 /*
  * Win class: Most basic window class.
  */
// Forward declaration: `Win::set_base_window` takes one, and GraphicWin
// derives from Win, so the definition has to come after this class.
class GraphicWin;
// WIN IS AN AUTOSOUND. The layout said so before the inheritance did: this
// class held `AutoSound auto_sound_` at offset 0, AutoSound's own first member
// was the vtable dword, so Win's vtable and AutoSound's were the same four
// bytes - and `iFlags_` sat immediately after AutoSound's 0x98, exactly where
// a base subobject puts it. tools/flat_classes.py flags the embedded-base
// shape independently.
//
// Declaring it is what gives Win, and through it GraphicWin, a real vfptr.
// The two per-window hooks Win carries as members, typed as what the
// image dispatches through them: `mouse_move_hook_` is called with the
// position on every non-parent mouse move, `key_hook_` with the virtual
// key on every key-down. Both default to null and the bodies null-test.
typedef void (__cdecl *WinMouseMoveHookFn)(int x, int y);
// Control notifications pass one selected id with caller-owned stack cleanup
// (RadioButton 0x0060E20C-0x0060E210, CheckBox 0x0060FAA6-0x0060FAA9).
// The installed BasePop callback returns with plain ret at 0x00604580.
typedef void (__cdecl *WinNotificationCallback)(int selected_id);
// resize_event's installable width/height callback, stored in Win::field_400_.
typedef void (__cdecl *WinResizeCallback)(int width, int height);
typedef int (__cdecl *WinKeyHookFn)(int key);

class Win : public AutoSound {
 public:
  const RECT &client_rect() const { return client_rect_; }
  const RECT &cached_outer_rect() const { return cached_outer_rect_; }

 public:
  // FileWin's button callback restores the modal window's focus-list cursor
  // when the filename editor had focus. The list remains owned by Win.
  friend class FileWin;
  // Credits configures its embedded GraphicWin's multi-table draw path.
  friend class Credits;
  // Popup configures the style of its embedded Dialogs' shared Win base.
  friend class Popup;
  // Reaches Win's fields directly: the byte-exact body for 0x005F5080
  // is a free function (both member spellings were measured and
  // refuted), and it reads `iFlags_` the way the image does.
  friend void __cdecl add_parent(Win *);
  // Homed into win.cpp 2026-08-25. It walks a window's children and
  // reads the same private fields the class's own z-order code does;
  // the image treats it as part of Win even though it is not a member.
  friend void __cdecl recurse_zorder(Win *window);
  // Homed into win.cpp 2026-09-06. Walks the parent chain accumulating each
  // window's client + outer origin into the caller's pair; reads the same
  // private fields the class's own coordinate converters do.
  friend void __cdecl add_ancestor_origin(int *x, int *y, Win *window);
  // 0x005ED490, homed into win.cpp 2026-09-06. The exact mirror of the one
  // above: every add is a subtract, the walk and the correction identical.
  friend void __cdecl remove_ancestor_origin(int *x, int *y, Win *window);
  // The window class's WM_SYSKEY router, homed into win.cpp. It reads
  // poWinBase_, iSomeFlag_ and win_parent_ off whichever window it
  // resolves as active; the image treats it as part of Win even though
  // its receiver is an HWND, not a `this`.
  friend void __cdecl OnSysKey(void *, unsigned int, long, int, unsigned int);
  friend int __stdcall win_reparent_window(HWND, int);
 public:
  // homed from 005f1340.cpp
  // STATIC, not the `QAA` member the catalogue reconstructs: the body reads
  // its argument at [esp+0x58] with no receiver below it - the same
  // argument-offset shift that made OnPaletteChanged, init_class and
  // set_display_mode statics above.
  static bool __cdecl OnPaint(HWND hwnd);

 public:
  // homed from 005ee330.cpp
  // Resize notification; all three original callers ignore the result.
  void resize_event(int width, int height);

 public:
  // homed from 005f83d0.cpp
  void set_bubble_text(char * text, RECT * rect);

 public:
  // homed from 005f6230.cpp
  // `is_down` gates which pair of messages this posts/dispatches: WM_KEYDOWN
  // + WM_SYSKEYDOWN when set, WM_KEYUP + WM_SYSKEYUP when clear. `repeat_count`
  // and `key_flags` pack into the dispatched message's lParam exactly as
  // Windows itself packs WM_SYSKEYDOWN/UP, low word and high word.

 public:
  // homed from 005f2760.cpp
  void redo_caption_buttons();

 public:
  // homed from 005f1150.cpp
  // `(HWND hWnd, LPARAM lParam)` per the catalogue's prototype: the WM_
  // PALETTECHANGED handler, comparing them raw (both read as generic
  // words) to skip a self-triggered change.
  // `static`: the mangled name ends in `QAA`, same as `set_display_mode`
  // and `init_class` below - a public member declared __cdecl, taking no
  // receiver. A non-static `__cdecl` member still takes `this` on the
  // stack, which is exactly the four-byte argument-offset shift a
  // non-static declaration measured here.
  static int __cdecl OnPaletteChanged(void * hwnd, void * lparam);

 public:
  // homed from 005f54e0.cpp
  void client_to_nonclient(RECT * rect);
  void client_to_nonclient(int * points, int * count);
  void bring_child_to_top(Win * child);

 public:
  // homed from 005ed170.cpp
  void nonclient_to_screen(RECT * rect);

 public:
  // homed from 005ed0a0.cpp
  void nonscreen_to_client(RECT * rect);

 public:
  // homed from 005ecec0.cpp
  // NAMED from the body: true when `this` IS the focus window, or is one of
  // its direct children, or is a descendant of one of them.
  int is_in_focus_chain();

 public:
  // homed from 005ecdc0.cpp
  // NAMED from the body: it walks `ancestor`'s children looking for
  // `this`, recursing - the mirror of is_descendant, which asks whether a
  // candidate is among MY descendants.
  int is_descendant_of(Win *ancestor);

 public:
  // homed from 005ecb60.cpp
  void remove_parent_dialog();

 public:
  // homed from 005ec8a0.cpp
  void get_mouse_pos(int * x, int * y);

 public:
  // homed from 005ec800.cpp
  void set_mouse_pos(int x, int y);

 public:
  // 0x005EEF60, a pending_bodies forwarder.
  void nonclient_to_client(int * x, int * y);
  // The RECT overload the homed bodies call - same conversion, one
  // rectangle instead of a coordinate pair.
  void nonclient_to_client(RECT *rect);

 public:
  // 0x005EBD80, a pending_bodies forwarder.
  int init(int x, int y, int width, int height, LPSTR caption, int flags, Win * parent, Menu * menu, BorderSizing * border);

  // 0x005F4D90, homed into win.cpp 2026-09-06. Installs the window this one
  // treats as its base display window: resizes the incoming window to the
  // current base's outer rectangle when the two disagree, records it in
  // poWinBase_, re-walks the root windows and repaints. The argument is the
  // GraphicWin whose working rectangle (Buffer::rect2_, 0x474) the comparison
  // reads - `Win` is a friend of `Buffer`, which is what makes it legal here.
  int set_base_window(GraphicWin *window);

  // 0x005EE4A0, homed into win.cpp 2026-09-06. The same gates as
  // send_wheel_delta (slot 77), one value straight through slot 15 or slot 30
  // with no division in front of it.
  void send_to_window(int value, int which);

  // 0x005EE460, homed into win.cpp 2026-09-06. Publishes `this` as
  // WinCallbackWindow, runs the field_404_ hook, raises slot 14, then offers
  // the contextual Sound receiver at val_6_ its status/play pair.
  void publish_callback_window();

  // 0x005EE4D0, homed into win.cpp 2026-09-06. The shared dispatcher shape -
  // input gates, publish WinCallbackWindow, run mouse_move_hook_, raise slot
  // 17 (slot 31 when the third argument is set), then notify the child.
  void dispatch_17(int a, int b, int alternate);

  // 0x005EE570 / 0x005EE610, homed into win.cpp 2026-09-06. The same
  // dispatcher shape around slot 28 (field_40C_ hook, children val_17_ /
  // val_26_) and slot 29 (field_410_ hook, children val_18_ / val_27_).
  void dispatch_28(int a, int b, int alternate);
  void dispatch_29(int a, int b, int alternate);

  // 0x005EE6B0, homed into win.cpp 2026-09-06. The same shape around slot 19
  // (field_414_ hook, children AutoSound+0x20 / val_20_) - see the layout
  // note in win.cpp about that 0x20 read.
  void dispatch_19(int a, int b, int alternate);

  // 0x005EE810 / 0x005EE8B0, homed into win.cpp 2026-09-06. The same shape
  // around slot 21 (field_41C_ hook, children val_10_ / val_22_) and slot 22
  // (field_420_ hook, children val_11_ / val_23_).
  void dispatch_21(int a, int b, int alternate);
  void dispatch_22(int a, int b, int alternate);
  // Seed identities 0x5EE950/0x5EE9F0; these source labels are provisional.
  void dispatch_23(int x, int y, int alternate);
  void dispatch_24(int x, int y, int alternate);

  friend class Scroll;
  // Control attachment reads the parent window's original flag word.
  friend class RadioButton;
  friend class CheckBox;
  friend class SpriteBox;
  friend class EditGroup;
  // BaseButton's colour setters test the parent link before drawing.
  friend class BaseButton;
  // EditBox::pass_dialog_focus resets its parent's child-list iterator.
  friend class EditBox;
  // GraphicWin's paint clips to a parent's client/outer rectangle.
  friend class GraphicWin;

 public:
  // 0x005F6320  ?on_mouse_move@Win@@QAEXHHIH@Z - public, __thiscall,
  // void(int, int, unsigned int, int). GraphicWin::on_mouse_move is a pure
  // forwarder to it and is its only caller.
  void on_mousewheel_up_vert(int delta);
  void on_mousewheel_down_horz(int delta);
  int get_lbutton_state();
  void on_mousewheel_up_horz(int delta);
  void on_mousewheel_down_vert(int delta);
  // THE FIVE UNK METHODS ARE EVIDENCE-FREE, and that is a measured result
  // rather than an unfinished job. Searched 2026-08-25, all of it empty:
  //   - the image records NO caller for UNK5, UNK6, UNK7 or UNK9, and the
  //     one it records for UNK1 is this declaration's own reference;
  //   - none of the five occupies a slot in Win's 88-entry vtable at
  //     0x0066FDD0 (read with tools/image_data.py), so no dispatch names
  //     them either;
  //   - docs/recovery/behaviour-member-names.csv has no entry for any;
  //   - UNK9's only effect is writing Scroll::field_2148_ on both bars,
  //     and NOTHING in the tree reads that field, so its behaviour names
  //     neither the method nor the member.
  // UNK1, UNK5 and UNK6 are stubs in the image too - UNK1 is `xor eax,eax;
  // ret 0x24`, nine arguments discarded. A name invented to satisfy the
  // census would be a false statement that outlives whoever wrote it; the
  // house rule is that a body which cannot lose its scaffold name is a
  // finding, not a style problem. This is the finding.
  int UNK1(int a, int b, int c, int d, int e, int f, int g, int h, int i);
  int UNK5();
  int UNK6(int a);
  Win();

  // STATIC, not the `QAA` member the census reconstructs: the body at
  // 0x005F04E0 reads no receiver, and its one caller sets up no ecx.
  static void close_class();   // 0x005F04E0
  // The real destructor is at 0x005EBC90, and it ends `ret` rather than
  // `ret 4` - it takes no vbase flag - so a direct call to it is reachable,
  // unlike RadioButton's. An empty inline body compiles every call away,
  // which is why `??_GWin` emitted `call dword ptr [...]` through a seam
  // where the image emits `call rel32`. Body homed in `win.cpp`.
  ~Win();


  // Returns `this`: the image's own real `??0Win@@QAE@XZ` sets `eax = this`
  // before its epilogue, which a void-returning method never emits.
  int move(int x, int y);
  int is_visible();
  // The production editor temporarily suppresses mouse/key input on the
  // desktop's flat buttons while its own modal controls are active. Preserve
  // the unrelated window-state bits when toggling that existing flag.
  void set_input_suppressed(int value) {
    if (value != 0) iSomeFlag_ |= 8U;
    else iSomeFlag_ &= ~8U;
  }
  const RECT &outer_rect() const { return outer_rect_; }
  int is_descendant(Win *candidate);
  int is_dialog_focus();
  int set_cursor(int name);
  // `static`, because the image's name ends in `QAA` - a public member
  // declared __cdecl, taking no receiver. jackal_init_real calls it with
  // no ecx set up, so `Class::method()` is the only legal spelling.
  static int set_display_mode(int width, int height, int depth, int tgl);
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class(LPSTR window_name);   // 005F01F0
  static void flip(RECT *area);              // 005EFD20
  // What `window_proc` routes to. Every one of these is `QAA` or `QAG` in
  // the catalogue - a member declared __cdecl or __stdcall, taking no
  // receiver - and the procedure calls them with no ecx set up, so
  // `Class::method()` is the only legal spelling. `do_tracking` is the one
  // exception: it is `QAE`, and the original loads its receiver from
  // `WinTrackingWindow` (`mov ecx, [0x9b7aac]`) rather than having one.
  static void OnLButtonDown(HWND window, LONG dbl, int x, int y,
                            WPARAM keys);          // 005F2330
  // CORRECTED from `int`. Both are catalogued returning `H`, and every
  // caller in `window_proc` immediately treats the result as a `Win *` -
  // tests it for null, dispatches through its vtable, stores it in
  // `WinHoverWindow`. `H` and `PAV1@` are the same eax either way, so this
  // costs no byte and stops the call sites casting.
  static Win *get_key_window();                    // 005F6A50
  static Win *get_mouse_window(int *x, int *y);    // 005F6F10
  static int update_cursor(Win *window, int tgl);  // 005F1820
  static int update_screen(RECT *area, Win *window);  // 005F7320
  void do_tracking(int x, int y);                  // 005F7580

  // The window procedure `init_class` registers. `static` and `__stdcall`
  // so `&Win::window_proc` is a plain `WNDPROC`: the image's name ends in
  // `QAG`, a public member declared __stdcall, but it is installed as a
  // callback and never called with a receiver, so a non-static member
  // could not be spelled at the one place that uses it.
  static LRESULT __stdcall window_proc(HWND window, UINT message,
                                       WPARAM wparam, LPARAM lparam);
  static void clear_bubble_text();
  // `Win *`, not int: the image's own name is
  // ?set_def_focus@Win@@QAAXPAUWin@@@Z - PAUWin@@ in the parameter list.
  // Declared static here for the same measured reason as every other `QAA`
  // image name above: the body reads its argument at [esp+4], with no
  // receiver on the stack.
  static void set_def_focus(Win *focus);
  void set_scroll_sprite(int value);
  void UNK9(int value);
  void reset_window_clip();
  void sync_palette();
  int is_child(int value);
  static int OnSetCursor(void *hwnd, void *window, unsigned int hittest,
                         unsigned int message);
  void set_vert_pos(int position);
  void set_horz_pos(int position);
  void set_vert_range(int minimum, int maximum);
  void set_horz_range(int minimum, int maximum);

  // The process-wide device context every window shares. The legacy bodies
  // take no instance and clean no stack, so they are statics here.
  static HDC get_hdc();
  static void release_hdc();
  void client_to_screen(int *x, int *y);
  Win *parent_window() const { return win_parent_; }
  bool has_dialog_focus_flag() const { return (iFlags_ & 0x1000) != 0; }
  // ListBox row painting tests this existing Win flag to select parent-surface copying.
  bool has_listbox_backfill_flag() const { return (iFlags_ & 0x80000u) != 0; }
  Win *event_window() const { return event_window_; }
  // ?client_to_screen@Win@@QAEXPAURECT@@@Z at 0x005ECFE0 - the RECT
  // overload, distinct from the (int*, int*) one above. Not yet recovered,
  // a pending_bodies forwarder.
  void client_to_screen(RECT *area);
  void screen_to_client(int *x, int *y);  // 005ED2D0
  // ?update_window@Win@@QAEHPAURECT@@@Z at 0x005F74A0. Not yet recovered,
  // a pending_bodies forwarder.
  int update_window(RECT *area);
  int get_vert_pos();
  int get_horz_pos();
  void set_vert_paging(int paging);
  void set_horz_paging(int paging);

  // ?on_sys_command@Win@@QAEHIHH@Z returns H. The body is empty either way
  // - VC6 emits the same `ret 0xc` - so only the decorated name moves.

  // Three base handlers the derived windows reach with a direct `call rel32`,
  // never through a vtable slot - PullDown::hide, DiploPop::hide,
  // Popup::on_nc_hittest and ReportIf::done all encode one - so they are
  // declared non-virtual on purpose. All three are unrecovered.
 // PROTECTED, NOT PRIVATE: GraphicWin derives from Win and the homed window
 // bodies read these directly - which is what the image does, since they are
 // one object (the same reasoning that made AutoSound's fields protected).
 // `private` modelled a containment that the inheritance edge replaced.
 // Access to a member affects no offset and no emitted byte, so the layout
 // and every claim are untouched.
 protected:
  uint32_t iFlags_;         // 0x98, immediately after AutoSound's 0x98 bytes
  uint32_t iSomeFlag_;
  uint32_t field_A0_;
  uint32_t field_A4_;
  // 0xA8. A `Win *`, not a word: `Win::construct` stores `this` here and
  // `Win::get_mouse_window` RETURNS it as the window that owns a position.
  // Nothing else in the tree reads it, so the `uint32_t` cost nothing until
  // now; it is the same four bytes either way.
  Win *poWinBase_;
  uint32_t iVertScaleDenom_;
  uint32_t iVertScaleNum_;
  Buffer *buffer1_;
  Buffer *buffer2_;
  Buffer *buffer3_;
  Buffer *buffer4_;
  Win *win_parent_;
  // 0xC8..0xDF (0x18 bytes). See `class WinNodeList` above for what proves
  // this is an embedded member rather than raw storage: `Win::Win()` stores
  // this member's own vtable FIRST, then zeroes its four remaining fields
  // in declaration order - exactly what an implicitly-constructed member
  // ahead of the constructor BODY produces - and only then installs Win's
  // own vtable at offset 0.
  WinNodeList list_;
  // 0xE0. Zeroed by the constructor; caption lifecycle is owned by Win.
  // Owned duplicate of the window caption; read by both non-client painters.
  char *field_E0_;
  // redo_caption_buttons (5F2760) uses ImageButton's borrowed Sprite at
  // +AB8 and its inherited Buffer DIB width at +444+7C+4. ImageButton
  // init/paint (625380/6253D0) establish that Sprite relationship; ctor
  // 6252E0 and deleting adjustor 6256F0 establish the Buffer base offset.
  // The names follow right-to-left placement and IsZoomed selection.
  // Actual caption-control creation remains an unestablished dependency.
  // RECOVERY_MEMBER: v3 Win minimize_button_ ImageButton* 0xE4 0x005F2760
  ImageButton *minimize_button_;
  // RECOVERY_MEMBER: v3 Win zoom_button_ ImageButton* 0xE8 0x005F2760
  ImageButton *zoom_button_;
  // RECOVERY_MEMBER: v3 Win close_button_ ImageButton* 0xEC 0x005F2760
  ImageButton *close_button_;
  Menu *menu_;
  uint32_t field_F4_;
  // THE WINDOW'S FONT, seeded from WinDynamicDefaults.default_font_ by the
  // constructor and reset by close; `redraw_nc_buffer` hands it straight to
  // Buffer::set_font - the cast that used to dress this up as uint32_t is
  // gone with the type.
  Font *field_F8_;
  uint32_t field_FC_;
  uint32_t field_100_;
  uint32_t field_104_;
  uint32_t field_108_;
  uint32_t field_10C_;
  uint32_t field_110_;
  int caption_height_;
  int border_thickness_;
  int bottom_border_thickness_;
  uint32_t field_120_;
  uint32_t field_124_;
  uint32_t field_128_;
  // 0x12C. PROVEN a `Buffer *`: `on_paint` reads `dib_.bmiHeader.biWidth`
  // (+0x80) and `.biHeight` (+0x84) through it and calls `copy_to_window`
  // (0x005D9BE0) on it with `this` as the target window.
  Buffer *buffer5_;
  // 0x130. Non-zero selects `on_paint`'s edge-fill path over the tiling loop.
  uint32_t field_130_;
  uint32_t field_134_;
  uint32_t field_138_;
  RECT outer_rect_;
  RECT client_rect_;
  RECT cached_outer_rect_;
  uint32_t field_16C_;
  uint32_t field_170_;
  uint32_t field_174_;
  uint32_t field_178_;
  uint32_t field_17C_;
  uint32_t field_180_;
  // 0x184. The palette generation this window last synchronised to: every
  // seed-compare site (`sync_palette`, `minimize`, `update_zorder`, the
  // redraw family) compares it against `PaletteActive->seed_` (Palette's
  // own 0x400) and refreshes through `set_active_window` when it has
  // fallen behind. Was `field_184_`.
  uint32_t palette_seed_cache_;
  Sprite *cursor_sprite_;
  uint32_t field_18C_;
  uint32_t field_190_;
  HCURSOR *cursor_handle_;
  int cursor_name_;
  uint32_t field_19C_;
  uint32_t field_1A0_;
  // The child list. PROVEN from the bytes: Win::add_child 0x005F5200 does
  // `mov [ecx + eax*4 + 0x1a4], esi` with eax = child_count_, so the base
  // is 0x1A4 and the stride is 4; recurse_zorder 0x005F4EC0 and
  // is_descendant 0x005ECE20 walk it the same way, and add_child does NOT
  // bound-check before storing.
  // INFERRED, and the reason the `buffer5_`/`spot_`/`buffer6_` guesses that
  // used to sit in this range are gone: the array runs the whole distance
  // to child_count_, giving 150 slots. Nothing referenced those three, and
  // an unbounded add_child would corrupt anything parked among the slots.
  // If a later recovery proves a real member here, this is what to revisit.
  Win *children_[150];
  // Was `Buffer *buffer6_`. Every caller treats it as a SIGNED count:
  // is_descendant does `test eax,eax / jle` and then `cmp edi, eax / jl`
  // against a loop index.
  int child_count_;
  // 0x400. The RESIZE CALLBACK: `resize_event` null-tests it and calls it
  // with the new width and height; nothing in the recovered tree writes it,
  // so its installer is still an original dependency. Typed here rather
  // than cast at the one use, same rule as key_hook_. The named typedef
  // makes the callback interface explicit, like mouse_move_hook_ above.
  // RECOVERY_MEMBER: v1 Win field_400_ 0x400 0x005EE330
  WinResizeCallback field_400_;
  uint32_t field_404_;
  // 0x408. A CALLBACK, not storage: `on_mouse_move` calls it with the
  // position when the move is not from a parent. Was `uint32_t field_408_`.
  WinMouseMoveHookFn mouse_move_hook_;
  uint32_t field_40C_;
  uint32_t field_410_;
  uint32_t field_414_;
  uint32_t field_418_;
  uint32_t field_41C_;
  uint32_t field_420_;
  uint32_t field_424_;
  uint32_t field_428_;
  uint32_t field_42C_;
  // 0x430. A CALLBACK, not storage: `key_down_event` calls it with the
  // virtual key and keeps its return. Was `uint32_t field_430_`.
  WinKeyHookFn key_hook_;
  uint32_t field_434_;
  uint32_t field_438_;
  // Borrowed nonclient scrollbars; resize_event reloads them after callbacks.
  // RECOVERY_MEMBER: v3 Win scroll_vert_ Scroll* 0x43C 0x005EE330
  Scroll *scroll_vert_;
  // RECOVERY_MEMBER: v3 Win scroll_horz_ Scroll* 0x440 0x005EE330
  Scroll *scroll_horz_;

 public:
  // ============ THE IMAGE'S VTABLE, DECLARED IN SLOT ORDER ==============
  // 88 slots at 0x0066FDD0. Slot 0 is `??_GWin`, emitted from `~Win()` -
  // already virtual because AutoSound's destructor is. Slots 1..87 are
  // Win's own and are declared here IN ORDER: declaration order IS vtable
  // order. Win was ALREADY polymorphic, so this extends the table without
  // moving a field - sizeof(Win) stays 0x444.
  //
  // The `vslot_NN` entries are real slots whose image body is trivial
  // (`ret`, `ret 8`, `xor eax,eax; ret 0xc`, `mov eax,1; ret`, or one
  // returning its first argument): default handlers a derived class
  // overrides. VC6 folds identical bodies program-wide, which is why the
  // catalogue attributes 51 of them to `AlphaMovie` - it saw that owner
  // first. Each arity is read from its own `ret N`.
  // slot  0  0x005F8610  ??_GWin - the compiler emits this
  //                from the virtual destructor below.
  virtual void show(int visible);  // slot 1  0x005ED9D0
  virtual void hide();  // slot 2  0x005EDCD0
  virtual int resize(int width, int height, int repaint);  // slot 3  0x005ED880
  // Slot 4: plain Win has no capture cleanup; MapWin overrides this.
  // Win's constructor at 0x005EB402 installs table 0x0066FDD0, whose
  // slot 4 selects the shared empty body catalogued as AlphaMovie::UNK7.
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_lose_mouse_capture@Win@@UAEXXZ 0x005EB3D0
  virtual void on_lose_mouse_capture() {}
  virtual int vslot_05(int = 0, int = 0, int = 0) { return 0; }  // 0x00404220
  // Win::init asks slot 6 for an external buffer before allocating its
  // own Scroll fallback. The default original body returns zero.
  // RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?on_scroll_create@Win@@UAEHXZ 0x005EB3D0
  virtual int on_scroll_create() { return 0; }  // slot 6, 0x00406B30
  // Win::close passes the exact Scroll* being removed. The default
  // original body returns that pointer unchanged (0x00404230, ret 4).
  virtual int on_scroll_delete(Scroll *scroll) {
    return reinterpret_cast<int>(scroll);
  }  // slot 7
  // RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?vslot_08@Win@@UAEHXZ 0x005EB3D0
  virtual int vslot_08() { return 0; }  // 0x00406B30
  // The parameter is the POINTER the image's one call site passes through
  // it (Dialogs::on_scroll_delete hands a Scroll* over this slot); typed as
  // such rather than as the int the shim era spelled.
  virtual void *vslot_09(void *value = 0) { return value; }  // 0x00404230
  virtual int vslot_10(int = 0) { return 0; }  // 0x00404250
  virtual int vslot_11(GraphicWin *, int, int, RECT *) { return 0; }  // 0x00404240
  // Slot 12 paints the contents; redraw() at slot 62 drives this callback.
  // Constructor 5EB3D0 installs table66FDD0, selecting the shared empty body.
  // The virtual is spelled on_redraw like every original derived override
  // (thunk1/thunk3 pairs in ListBox/SpriteBox/EditGroup prove one original
  // name served both this slot and Dialog slot2); the folded shared body's
  // catalogue name above stays what the image calls it.
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?draw_contents@Win@@UAEXXZ 0x005EB3D0
  virtual void on_redraw() {}  // 0x00404280
  // Slot 13: Win's constructor-installed table66FDD0 selects the shared RET8.
  // MapWin supplies a genuine override; this is not MainInterface's slot13.
  // RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_resize@Win@@UAEXHH@Z 0x005EB3D0
  virtual void on_resize(int, int) {}
  virtual void on_sys_close() {}  // 0x00404280
  virtual void on_mousewheel(int = 0) {}  // 0x00404270
  // The native tables require the router in slot 16 and the two-argument
  // callback in slot 17. VC6 reverses declaration order within this overload
  // group; physical compiler tables verify this order, unlike host mocks.
  // RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_mouse_move@Win@@UAEXHH@Z 0x005F6320
  virtual void on_mouse_move(int, int) {}
  // The router ignores the callback's EAX result.
  virtual void on_mouse_move(int x, int y, unsigned int keys, int from_parent);  // slot 16  0x005F6320
  virtual void on_mouse_leave(int = 0, int = 0) {}  // 0x00404260
  // Slot 19 is the completed left-click callback: on_l_button_up dispatches
  // it after release, and MonuWin's original table66C880 points it at476780.
  // Keep this two-argument callback distinct from the slot20 press callback.
  virtual void on_left_click(int = 0, int = 0) {}  // 0x00404260
  virtual void on_left_down(int = 0, int = 0) {}  // slot 20
  virtual void on_left_up(int = 0, int = 0) {}  // slot 21
  virtual void on_right_click(int = 0, int = 0) {}  // 0x00404260
  // Receiver distinction: Win table66FDD0 slot23 consumes (x,y), with
  // shared default404260 ret8. The nullary/tested-EAX calls in5EE750 and
  // other dispatchers target Sound children (table66E3C0), not Win.
  // Historical nullary-Win inference was disproved by this receiver closure;
  // adding default coordinates to Sound calls would incorrectly emit pushes.
  // Win's primary slot 23 consumes two coordinates (the byte-backed target
  // at 0x00404260 is ret 8); Sound has a separate nullary slot 23 contract.
  virtual void on_right_down(int x, int y) { (void)x; (void)y; }
  virtual void on_right_up(int = 0, int = 0) {}  // 0x00404260
  // Existing slot 25: keyboard input dispatched by key_click_event.
  virtual int on_key_click(int = 0, int = 0);  // slot 25
  virtual int on_key_down(int = 0) { return 0; }  // 0x00404250
  virtual int vslot_27(int = 0) { return 0; }  // 0x00404250
  // Slot 28: Win dispatches left double-clicks here (0x005EE570).
  // RadioButton's table at 0x00669A6C selects its receiver adjustor
  // 0x00407040 at +0x70, reaching the override at 0x0060E1E0.
  virtual void on_left_double_click(int = 0, int = 0) {}  // 0x00404260
  virtual void on_right_double_click(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_30(int = 0) {}  // 0x00404270
  virtual void vslot_31(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_32(int = 0, int = 0) {}  // 0x00404260
  virtual void on_nc_left_down(int = 0, int = 0) {}  // slot 33, 0x00404260
  virtual void vslot_34(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_35(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_36(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_37(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_38(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_39(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_40(int = 0, int = 0) {}  // 0x00404260
  virtual int vslot_41(int = 0) { return 0; }  // 0x00404250
  virtual void vslot_42(int = 0) {}  // 0x00404270
  virtual void vslot_43(int = 0) {}  // 0x00404270
  // SLOTS 44 AND 45 HAVE NAMES. BaseWin's front table holds 0x0041D500 and
  // 0x0041B4E0 at 0x66A4BC/0x66A4C0 and MainInterface's holds 0x0045C800 and
  // 0x0045C870 at 0x66C648/0x66C64C - and every one of those four catalogue
  // names spells on_button_clicked / on_button_toggled, as does CouncWin's own
  // override at 0x00424870 (?on_button_clicked@CouncWin@@QAEXH@Z). That is the
  // slot's real name, not a coincidence. They were `vslot_44`/`vslot_45` until
  // the BaseWin derivation proved who owns them: the interface's
  // button-clicked and button-toggled events both step the receiver back to
  // the front window and dispatch these two. `vslot_13` on the interface is
  // this one plus a name.
  virtual void on_button_clicked(int = 0) {}       // slot 44  0x00404270
  virtual void on_button_toggled(int = 0, int = 0) {}  // slot 45  0x00404260
  virtual void vslot_46(int = 0, int = 0, int = 0) {}  // 0x00406B20
  virtual void vslot_47(int = 0, int = 0) {}  // 0x00404260
  virtual void on_scrolled(int = 0, int = 0) {}  // 0x00404260
  virtual void on_scrolling(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_50(int = 0) {}  // 0x00404270
  virtual void on_dialog_focus(int = 0) {}  // 0x00404270
  virtual void vslot_52() {}  // 0x00404280
  virtual void vslot_53(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_54(int = 0) {}  // 0x00404270
  virtual void vslot_55(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_56(int = 0) {}  // 0x00404270
  virtual int set_modal(int flag, int (*poll)(), Win *owner);  // slot 57  0x005EE190
  virtual void release_modal();  // slot 58  0x005EE280
  virtual int vslot_59() { return 1; }  // 0x00406840
  // Border-paint notification receives the source Buffer and quadrant.
  virtual void vslot_60(Buffer * = 0, int = 0) {}  // 0x00404260
  // Slot 61 (0xF4): ordinary windows have no GraphicWin surface.
  virtual GraphicWin *get_graphic_win();  // original shared no-op target
  // Slot 62: plain Win has no surface; GraphicWin supplies the redraw.
  virtual void redraw() {}  // 0x00404280
  // Slot 63: no surface in plain Win; GraphicWin supplies the update.
  // The same constructor-installed Win table selects the shared empty body.
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_update@Win@@UAEXXZ 0x005EB3D0
  virtual void on_update() {}
  virtual void pass_dialog_focus();  // slot 64  0x005ECD20
  virtual void set_dialog_focus(Win *target);  // slot 65  0x005ECC40
  virtual void set_rects();  // slot 66  0x005F2940
  virtual void on_redraw_nc(RECT *area, int flags);  // slot 67  0x005F2CF0
  virtual long on_window_pos_changing(WINDOWPOS *pos);  // slot 68  0x005F53A0
  virtual long on_window_pos_changed(WINDOWPOS *pos);  // slot 69  0x005F5480
  virtual void on_move(int x, int y);  // slot 70  0x005F54B0
  virtual void on_size(unsigned int flags, int width, int height);  // slot 71  0x005F54C0
  virtual void on_size_nc(unsigned int flags, int width, int height);  // slot 72  0x005F54D0
  virtual void on_paint(RECT *area);  // slot 73  0x005F54E0
  virtual void on_nc_paint(RECT *area, int flags);  // slot 74  0x005F5810
  virtual int on_nc_hittest(int x, int y);  // slot 75  0x005F5AD0
  virtual int on_query_new_palette();  // slot 76  0x005F1060
  // slot 77  0x005F62D0. RECOVERED 2026-09-06 and homed into win.cpp as
  // `send_wheel_delta`, which replaces the empty placeholder that stood here:
  // the slot divides the packed `short` by 120 - WHEEL_DELTA - and forwards
  // the notch count through slot 15 / slot 30, behind the class's usual
  // input gates. The window_proc arm below is its caller.
  virtual void send_wheel_delta(int key, short delta, int x, int y,
                                int from_parent);
  virtual void on_key(unsigned int key, long flags, int repeat, unsigned int scan);  // slot 78  0x005F5D10
  // slot 79  0x005F5FB0. NAMED 2026-08-25, and it was declared TWICE: as
  // this placeholder and again as the non-virtual `sub_5f5fb0(char, int)`
  // below - the same body. It is the real WM_CHAR handler: it writes the
  // character into the 10-byte typed-key ring at WinKeyRingStart and
  // advances WinKeyRingCursor, which is the ring `typed_keys_differ`
  // (0x005F5F60) reads back. The catalogue's `?on_char@Win@@QAAHDH@Z`
  // describes THIS signature, `(char, int)`, and had been filed against
  // that matcher instead.
  virtual void on_char(char ch, int flags);
  // NOT __stdcall, though the catalogue spells the body `QAG`. The image's
  // own DISPATCH decides this, and it passes the receiver in ecx:
  // OnSysKey at 0x005F16D0 does `mov ecx, esi` before the slot-80 call
  // where a __stdcall member would push it. Declaring it __stdcall cost
  // that claim one instruction; 0x005F6230 itself is unclaimed, so the
  // dispatch is the only measurement that speaks here.
  virtual int on_sys_key(unsigned int key, long flags, int repeat, unsigned int scan);  // slot 80  0x005F6230
  virtual void on_l_button_down(long double_click, int x, int y, unsigned int keys, int from_parent);  // slot81,5F63C0
  virtual void on_l_button_up(int x, int y, unsigned int keys, int dbl);  // slot 82  0x005F6550
  virtual void on_r_button_down(long flags, int x, int y, unsigned int keys, int dbl);  // slot 83  0x005F6710
  virtual void on_r_button_up(int x, int y, unsigned int keys, int dbl);  // slot 84  0x005F6880
  virtual int on_set_cursor(void *cursor, unsigned int hit, unsigned int msg);  // slot 85  0x005F6A30
  virtual int on_sys_command(unsigned int command, int x, int y);  // slot 86  0x005F6A40
  virtual long on_activate(unsigned int state, void *other, long minimized);  // slot 87  0x005F5C00

 public:
  // Homed bodies whose declaration the class never carried - which is
  // exactly why their artifacts would not compile in this TU. Each
  // signature is the definition's own, not a guess.
  // `int`, not the catalogue's `D` (char): the image loads a full dword,
  // `mov ecx, [esp+0x10]`, where a char parameter compiles `movsx ecx,
  // byte ptr`. Measured on 0x005F1660.
  // NOT a member, and the parameter names below are the proof. Declared
  // here only until the free form lands; see win.cpp.
  void UNK7(int x, int y, int width, int height);
  // STATIC, and the receiver was the catalogue's invention. `QAG` is a
  // __stdcall member, whose `this` is a STACK parameter at [esp+4] - and
  // the body reads exactly that and passes it to GetWindowLongA while
  // ignoring the declared one at [esp+8]. As a static taking the HWND the
  // stack layout is identical and the window handle stops being `this`.
  static int __stdcall adjust_menus(HWND hwnd, void *unused);
  static void update_menus();
  void do_caption_buttons();
  int maximize();
  void on_mousewheel_down(int delta);
  void on_mousewheel_up(int delta);
  void set_bottom_border_thickness(int thickness);
  int show_maximize();
  void update_nc_buffer(int flags);

 public:
  void add_child(Win* child);
  void bring_to_top();
  int get_rbutton_state();
  void left_down_event(int x, int y, int from_parent);
  int on_redraw(int, int);
  // Returns Buffer *, not int: every arm of its switch returns one of the
  // four render buffers. The `int` was the raw-offset form's type leaking
  // into the signature. Unclaimed, so no marker moves with this.
  Buffer *redraw_nc_buffer(int index);
  void __cdecl remove_parent(Win* window);
  void set_border_thickness(int thickness);

  void set_field_128(int value);
  void set_parent_dialog(Win *dialog);
  void __cdecl update();
  void __cdecl update_zorder();
  void window_line_raw(int x2, int y2, int x1, int y1, int colour, int width, unsigned int style);

 public:
  void __cdecl OnRButtonDown(void * hwnd, long flags, int x, int y, unsigned int keys);
  void __cdecl OnRButtonUp(void * hwnd, int x, int y, unsigned int keys);
  // Original 0x005F5140 consumes only the stack window argument; both
  // callers clean it up. There is no hidden receiver (unlike the child lift).
  static void __cdecl bring_parent_to_top(Win * window);
  void draw_rect_border(int x1, int y1, int x2, int y2, HGDIOBJ pen1, HGDIOBJ pen2, int unused7);
  // Provisional descriptive names; native receiver/argument evidence is retained in the TU.
  void draw_rect_edges(RECT *rect, HGDIOBJ light_pen, HGDIOBJ dark_pen, int unused);
  void fill_palette_color_rect(int x, int y, int width, int height, int palette_index, int unused);
  void copy_edges_to_buffer(Buffer *target, int x, int y);
  void copy_border_buffers_to_window(Win *target, RECT *clip, int mode);
  void remove_child(Win *child);
  void set_caption_height(int height);
  void update_back_to_window(Buffer * buffer);

 public:
  // The RECT overload. `char *` not `signed char *`: the catalogue spells
  // the caption `PAD`, and matching it makes the tree emit the image's
  // own name instead of needing a `symbol` alias.
  int init(RECT *area, char *caption, int style, Win *parent,
           Menu *menu, BorderSizing *sizing);
    int key_click_event(int key, int flags);
  int key_down_event(int key);
  int key_up_event(int key);
  int set_cursor(Sprite* sprite, int hot_x, int hot_y);
  int set_cursor(HCURSOR *cursor);

 public:
  int center();
  int center_on_parent();
  int minimize();
  void nonclient_to_screen(int * x, int * y);
  void screen_to_nonclient(int * x, int * y);
  void set_caption(char * text);
  void sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6);
 public:
  void __cdecl OnKey(void * hwnd, unsigned int key, long flags, int repeat, unsigned int scan);
  void OnLButtonUp(void *hwnd, int x, int y, unsigned int keys);
  int __cdecl OnSysCommand(HWND hwnd, unsigned int command, int x, int y);
  void paint_tiled(Buffer *tile, int x_origin, int y_origin, int clip_left, int clip_top, int clip_width, int clip_height, int unused8);
  void update_window_to_buffer(Buffer * buffer);

 public:
  void __cdecl OnMouseMove(void * hwnd, int x, int y, unsigned int keys);

 public:
  int __cdecl OnQueryNewPalette(void * hwnd);

 public:
  long __cdecl OnActivate(void *hwnd, unsigned int state, void *other, long minimized);
  void screen_to_client(RECT * rect);

 public:
  // Tear the window down: clears the focus/tracking globals and walks
  // the child list. Homed from 005eb640.cpp.
  void close();

 public:
  // Distinct list-wide operation; preserve its recovered calling convention.
  void __cdecl redraw_all();  // 0x005F4CC0

 public:

 public:
  // A MEMBER: the caller at 0x0063C340 sets up `mov ecx, esi` before
  // this call, which is a thiscall receiver, not a __cdecl free function.
};

static_assert(sizeof(Win) == 0x444, "Win layout must match the legacy ABI");


BOOL __cdecl in_box(int x, int y, const RECT *rect);
int __cdecl in_box(
    int x, int y, int left, int top, int width, int height);
void __cdecl offset_rect(RECT *rect, int dx, int dy);
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height);


// Shared device-context state: the reference count, the cached handle, and
// the optional DirectDraw surface that supplies it.
//
// REAL OBJECTS, not pointers to image addresses. They were
// `int *WinHdcRefCount = reinterpret_cast<int *>(0x009B3AB0)` and the
// reason given was that tests rebind them - the ctest suite was retired on
// 2026-08-15, so nothing does. The pointer form also costs a load at every
// use where the original has a direct absolute access, which is
// byte-visible; `palette.cpp` already declares its own globals this way,
// and 0x009B3AB0 / 0x009B7B2C / 0x009BC498 are all past `.data`'s stored
// bytes, so there is no initialiser to preserve.
extern int WinHdcRefCount;      // 0x009B3AB0
extern HDC WinSharedHdc;        // 0x009B7B2C

// 0x009BC498. An IDirectDrawSurface, eight bytes below the
// IDirectDrawPalette `palette.h` binds at 0x009BC4A0. It was a `void *`
// dispatched through raw vtable offsets - `vtable[0x44 / 4]` and
// `vtable[0x68 / 4]` - and those two slots are 17 and 26, which in
// `IDirectDrawSurface` are exactly `GetDC(HDC *)` and `ReleaseDC(HDC)`.
// Calling them by name emits the same `call [reg + 0x44]`, and unlike an
// offset it cannot be silently wrong.
struct IDirectDrawSurface;  // Buffer's SDK ABI-prefix view
struct IDirectDrawSurface2; // Shared display QueryInterface output66FC28
extern IDirectDrawSurface2 *DirectDrawSurface;

// 0x009B6EF8. The modal-window stack, indexed by the depth at 0x009B7AE4:
// `Win::release_modal` reads `[edx*4 + 0x9b6ef8]` and shuffles entries down
// through `[eax*4 + 0x9b6efc]`, so this is an array of `Win *`.
// `Win::init_class` clears the first four entries. Nothing proves the array
// is only four long - that is simply as far as the one function that clears
// it goes.
extern Win *WinModalStack[4];

// 0x009B7B14. The module handle the window class is registered under:
// `Win::close_class` unregisters with it and `FileWin::init` passes it on.
extern HINSTANCE WinInstance;

// STATIC, not a `QAA` member. The catalogued name
// `?close_class@Win@@QAAXXZ` claims a receiver the body never reads.


// 0x009B7B1C and 0x009B7B20. The screen metrics, read once from
// GetSystemMetrics in `Win::init_class` and then from 166 and 51 other
// sites in the image. `basepop.cpp` binds the first of the two separately
// as `WinScreenWidth`.
extern int WinScreenWidth;
extern int WinScreenHeight;
// Shared Menu/PullDown keyboard-origin state at the original 0x009B7B6C.
extern int MenuPullDownInputOrigin;

// 0x009B7AAC. THE WINDOW BEING TRACKED, not a flag. `window_proc`'s
// WM_MOUSEMOVE arm is `mov ecx, [0x9b7aac]` / `test ecx, ecx` / `je` /
// `call ?do_tracking@Win@@QAEXHH@Z` - ecx is the receiver, and
// `do_tracking` is the one `__thiscall` member in that whole procedure.
// `Win::OnLButtonUp` and `OnRButtonUp` clear it.
extern Win *WinTrackingWindow;

// 0x009B7A7C. The window the pointer was last over: WM_MOUSEMOVE calls
// slot 0x48 on it when the pointer moves to a different one, then stores
// the new one. Also read by `Win::OnMouseMove` and cleared by `Win::close`.
extern Win *WinHoverWindow;

// THE FOUR WINDOWS `Win::get_mouse_window` CONSULTS, in this order, before
// it walks the window tree at all. What is PROVEN is the mechanism, not a
// role for each: the function tests each in turn, takes the first whose
// `iSomeFlag_ & 1` (visible) is set, and NULLS any that fails that test
// before moving on. Which is also why they are `Win *` - it dereferences
// each at `+0x9C`. Two of them were `int WinInputStateA/B` here until this
// function was read, on the strength of nothing but `flush_input` and
// `flush_keyboard` clearing both.
//
// The order is the whole of what distinguishes them, so they are numbered
// rather than given roles this tree cannot yet justify. 1 and 2 are cleared
// together by `flush_input`, `flush_keyboard`, `flush_mouse`,
// `Win::OnActivate`, `Win::close` and `do_all_tasks`; 3 has 85 references
// including `BaseWin::click` and `go_reset`; 4 has 70 including
// `Win::hide` and `iface_click`.
extern Win *WinPointerOwner1;  // 0x009B7ACC
extern Win *WinPointerOwner2;  // 0x009B7AD0
extern Win *WinPointerOwner3;  // 0x009B7ABC
extern Win *WinPointerOwner4;  // 0x009B7AC0

int __cdecl do_char();
int __cdecl do_task();
void __cdecl wait_task();
void __cdecl do_all_tasks();
int __cdecl do_mouse();
void __cdecl do_all_mouse();
void __cdecl flush_keyboard();
void __cdecl flush_mouse();

// 0x009B7AE0. The focus window: `Win::get_key_window` returns it, and
// `Win::OnKey`, `OnChar` and `OnSysKey` are the rest of its readers.
// `Win::get_mouse_window` falls back to walking its tree when no pointer
// owner above is live.
extern Win *WinFocusWindow;

// 0x009B7ADC. The modal window, and this one IS established: `Win::set_modal`
// and `Win::release_modal` are its only writers, and they are also what
// maintains `WinModalStack` and its depth at 0x009B7AE4.
extern Win *WinModalWindow;

// 0x009B6E48 and 0x009B7B34. The root windows and how many there are;
// `get_mouse_window` walks `roots[0 .. count)` when there is no focus
// window, recursing into each until one claims the pointer.
extern Win *WinRootWindows[];
extern int WinRootCount;

// 0x009B7AA4. NOT A KEY STATE, which is what it was called here after
// `Win::window_proc` was read: that procedure only ever forwards it, and
// `get_mouse_window` is what SETS it - to 1 on the two owners that return
// the window found, and to 0 on the two that return that window's
// `poWinBase_` instead. The trailing argument every input slot receives is
// therefore "these coordinates are the window's own", not a modifier mask.
extern int WinMouseDirect;

// 0x009B6628 and 0x009B662C. The screen position `get_mouse_window` was
// given, saved on entry so the tree walk can restore it before trying a
// different subtree.
extern int WinMouseScreenX;
extern int WinMouseScreenY;

// The `Win` flag bits `get_mouse_window`, `window_proc` and `OnLButtonDown`
// test, named for what the code does when each is set.
static const uint32_t WinFlagVisible = 0x1;          // iSomeFlag_
static const uint32_t WinFlagClipToParent = 0x20;    // iFlags_
static const uint32_t WinFlagParentOffset = 0x8000;  // iFlags_
// `OnLButtonDown` runs the hit-test slot when the position is the window's
// own (`WinMouseDirect`) OR this is set, and skips the raise when the other
// is. Named for the branch each guards; nothing establishes more.
static const uint32_t WinFlagHitTestIndirect = 0x4000000;  // iFlags_
static const uint32_t WinFlagNoRaise = 0x2000000;          // iFlags_

// 0x009B7AA8, 0x009B7AB0, 0x009B7AB4 - the rest of the drag that
// `WinTrackingWindow` at 0x009B7AAC begins. All four are contiguous,
// `Win::OnLButtonDown` is the only writer, and `Win::do_tracking` - the
// method `window_proc` calls on `WinTrackingWindow` for every WM_MOUSEMOVE
// - is the only other reader.
// Real storage, not a fixed-address binding. These were spelled
// `static int *const X = (int *)0xADDR` and read as `*X`; the address is
// what the image put them at, which is an annotation, not a type.
extern int WinClipWidth;            // 0x009B7A40
extern int WinClipHeight;           // 0x009B7A44
extern int WinDragOffsetX;          // 0x009B7A38
extern int WinDragOffsetY;          // 0x009B7A3C
// Borrowed caption sprites. game_init publishes close/maximize/minimize;
// no original restore-icon writer was found. These do not own Sprite lifetime.
extern Sprite *WinCloseIcon;        // 0x009B7B00
extern Sprite *WinMaximizeIcon;     // 0x009B7B04
extern Sprite *WinRestoreIcon;      // 0x009B7B08
extern Sprite *WinMinimizeIcon;     // 0x009B7B0C
extern int WinSizingFlag;           // 0x009B7AD4

extern int WinModalDepth;           // 0x009B7AE4
extern int WinKeyModifiers;         // 0x009B7B18
extern int WinViewOriginX;          // 0x009B7A70
extern int WinViewOriginY;          // 0x009B7A74
// A BYTE, not an int: the image reads it with `xor ecx,ecx; mov cl,
// byte ptr [0x696d14]` - the zero-extension of a byte-typed object - and
// the int spelling forced every reader to load a dword and mask
// (setup_buff_sprite measured 60/64 that way, 64/64 as a byte).
extern const unsigned char WinFillColour;  // 0x00696D14
// The four child/parent overflow messages MessageBoxA shows, read out of
// the image with tools/image_data.py rather than left as bare addresses.
extern const char WinMsgTooManyChildren[];      // 0x00696D80
extern const char WinMsgIncreaseMaxChildren[];  // 0x00696D60
extern const char WinMsgTooManyParents[];       // 0x00696DB4
extern const char WinMsgIncreaseMaxParents[];   // 0x00696D94
// The debug cheat code on_char matches the typed-key ring against. The
// image's pointer is 0x00696DFD, the code's LAST character, because the
// comparison walks BACKWARD from the cursor.
extern const char WinMdebugCode[];              // 0x00696DF8
extern int WinTrackingMode;  // 0x009B7AA8, the hit-test code that started it
extern int WinTrackingX;     // 0x009B7AB0
extern int WinTrackingY;     // 0x009B7AB4

// The z-order refresh `OnLButtonDown` runs after raising a window, shared
// with `Win::bring_to_top`, `bring_child_to_top`, `bring_parent_to_top`,
// `recurse_zorder`, `Win::show` and `Win::hide`.
//
// Only the first has a settled meaning: it is compared against each root
// window in turn and re-read after every `recurse_zorder`, so it is a
// `Win *`. The other two are cleared together whenever that comparison
// hits, and what they count is not established.
extern Buffer *WinBackBuffer;         // 0x009B7A68
extern Win *WinModalFocus;         // 0x009B8D7C
extern Win *WinPendingFocus;       // 0x009B7B38
extern Win *WinCallbackWindow;   // 0x009B7AB8
extern Win *WinInputFocus;       // 0x009B7AC4
extern Win *WinActiveWindow;     // 0x009B7AC8
extern Win *WinBubbleCompanion;  // 0x009B7A4C
// Indexed by WinZOrderCount. 0x009B6630 to the next named global at
// 0x009B6E38 is 2056 bytes; 512 entries is 2048 of them, and the last 8
// bytes belong to something this tree has not named yet.
extern Win *WinZOrderArray[512]; // 0x009B6630
extern RECT WinBubbleRect;       // 0x009B6E38
extern RECT WinScreenClipRect;   // 0x009B74C0
extern RECT WinDirtyRect;        // 0x009B6EE8
extern Win *WinDefaultFocus;     // 0x009B7AEC
// Parallel to WinModalStack[4] and indexed by the same depth; the four
// entries run from 0x009B7A1C exactly to the next named global at
// 0x009B7A2C, which is where the size comes from.
extern Win *WinFocusStack[4];    // 0x009B7A1C
// The typed-key ring on_char matches WinMdebugCode against, walking
// BACKWARD from the cursor. 0x009B7B51 is the ring's last byte, which is
// what the `WinKeyRingEnd` scaffold pointed at.
extern char WinKeyRing[10];      // 0x009B7B48
extern char *WinKeyRingCursor;   // 0x00696D5C, initialised to the ring
// Ten dwords: the table runs from 0x00696D34 to WinKeyRingCursor at
// 0x00696D5C, and the values are the image's own, read with
// tools/image_data.py rather than left as an address.
extern uint32_t WinStaticDefaults[10];   // 0x00696D34
// THE DYNAMIC WINDOW DEFAULTS, 0x009B7AF0 - four dwords, of which slot 1 is
// named: it is THE DEFAULT WINDOW FONT (a Font* - the Win constructor seeds
// every new window's field_F8_ from it, Win::close resets the same way, and
// config_popups re-points it at the popup 12px font via find_font). It used
// to be a raw dword slot wearing the address name jackal_unk_9b7af4, which
// was a second C++ object over these same bytes - the font never flowed in
// the standalone build until the two became one.
struct WinDynamicDefaultsTable {
  // RECOVERY_MEMBER: v1 WinDynamicDefaultsTable field_0_ 0x0 0x005EB3D0
  uint32_t field_0_;      // was [0]
  // RECOVERY_MEMBER: v3 WinDynamicDefaultsTable default_font_ Font* 0x4 0x005EB3D0
  Font *default_font_;    // 0x4; original address 0x009B7AF4, was [1] - see above
  // Win construction/close read AF8/AFC. The following four words are
  // independent caption pointers, then MenuPopup and WinInstance, not
  // additional defaults. No indexed access establishes a larger record.
  // RECOVERY_MEMBER: v1 WinDynamicDefaultsTable rest_ 0x8 0x005EB3D0
  uint32_t rest_[2];      // 0x8; original addresses 0x009B7AF8..0x009B7AFC
};
extern WinDynamicDefaultsTable WinDynamicDefaults;
extern int WinBubbleActive;      // 0x009B7A50
extern Win *WinZOrderWindow;  // 0x009B7A6C
extern int WinZOrderCount;    // 0x009B7B30
extern int WinZOrderFlag;     // 0x009B7A78

// 0x009B7A90. Called with the click position after the window has been
// dispatched to. `Win::left_down_event` is its only other reader.
extern void(__cdecl *WinLeftDownHook)(int x, int y);

// 0x005F4EC0. Re-walks one root window's subtree after a raise.
void __cdecl recurse_zorder(Win *window);

// 0x005F6AB0. The tree walk `get_mouse_window` delegates to once it has a
// subtree and a position in that subtree's coordinates.
Win *__cdecl get_mouse_window_recurse(Win *window, int *x, int *y);
// 0x0063C4E0, homed into win.cpp.
// 0x009B7B3C. Cleared on every WM_MOUSEMOVE and read by
// `Win::update_cursor`, which is the only other function that touches it.
extern int WinCursorMoved;

// 0x009B7A88 and 0x009B7A94. Two `__cdecl` hooks the procedure calls on the
// way out: the second on mouse messages with (hwnd, lParam), the first on
// every message at all, with no arguments. `Effect::init` installs the
// first; `Win::OnKey`, `OnLButtonDown` and `OnRButtonDown` read them too.
extern void(__cdecl *WinMessageHook)();
extern void(__cdecl *WinOkHook)();
extern void(__cdecl *WinCancelHook)();
extern void(__cdecl *WinButtonHoverHook)();
extern void(__cdecl *WinMouseHook)(HWND window, LPARAM position);

// 0x009B7A8C. A third hook, called after WM_KEYDOWN with the virtual key.
// `Win::OnKey` is its only other reader.
extern void(__cdecl *WinKeyHook)(WPARAM key);

// 0x005F86A0. The one thing `window_proc`'s WM_SYSCOMMAND arm calls that is
// not a `Win` member. Its body was homed from src/recovered/005f86a0.cpp
// into win.cpp and measures BYTE_EXACT there.
// 0x005F86A0, homed into win.cpp.
void __stdcall sub_5f86a0(int window);


// The cursor refresh this setter triggers is a 2528-byte body with six call
// targets, still an original dependency. Tests rebind this seam.


// Bubble state: the pending flag, its companion slot, and the rectangle the
// refresh republishes. Tests rebind all three.
extern void (*WinFlipHook)();          // 0x009B7A48
extern Font *WinBubbleFont;            // 0x009B7A54
extern int WinBubbleFillColour;        // 0x009B7A64
extern const int WinBubbleTextColour;  // 0x00696D2C
extern const int WinBubbleEdgeColour;  // 0x00696D30
extern Sprite *WinFlipSprite;          // 0x009B7A2C
extern int WinFlipSpriteY;             // 0x009B7A30
extern int WinFlipSpriteX;             // 0x009B7A34
extern int WinFlipClipped;             // 0x009B7AD8
extern IDirectDrawSurface2 *DirectDrawBackBuffer;  // 0x009BC49C
extern RECT DirectDrawClipRect;        // 0x009BC2D0

// A `Win *`, not a word: `Win::set_bubble_text` (0x005F83D0) stores `this`
// here directly, with no cast - retyped from `int *` so that store needs
// none either.


// Both refresh bodies remain original dependencies: update_screen is 383
// bytes with four call targets, flip 1223 bytes with fourteen.




// The active palette is `PaletteActive` (palette.h), the mapped global at
// 0x009B8180. The `WinActivePalette()` pun accessor that used to sit here
// had zero callers and read the same address through a cast.




// The four handlers declared above that are still original bodies - hide is
// 390 bytes, on_mouse_move 155, on_nc_hittest 294 and release_modal 167. Each
// is reached by a direct `call rel32` from a derived window, so the class has
// to name them; each is defined at the end of win.cpp as a forwarder through
// one of these seams. Signatures are the mangled names:
//   ?hide@Win@@QAEXXZ           void()
//   ?on_mouse_move@Win@@QAEXHHIH@Z  void(int, int, unsigned int, int)
//   ?on_nc_hittest@Win@@QAEHHH@Z    int(int, int)
//   ?release_modal@Win@@QAEXXZ  void()

// The single DirectDraw device object Win::set_display_mode dispatches to.
// 0x009BE618 IS the object's address - the image loads it straight into ecx
// (`mov ecx, 0x9be618`) rather than reading a pointer stored there - so this
// is a `T *const` object handle, not a callee bound as a pointer: the CALL
// still goes through `WinDisplayInit->init(...)`, a real member-function
// call by name, so it still emits the image's `call rel32`. `init` itself
// (0x00635510, 587 bytes: DirectDraw surface/window teardown and
// re-creation) is defined in win.cpp.
//
// FIELDS, in image order, from `sub_635450` (0x00635450, unreached in this
// build but still in the image) reading the SAME five offsets off the
// fixed `WinDisplayInit` address rather than off a `this`:
//   0x0  hwnd_           the device window `init` creates and destroys
//   0x4  surf_           the IDirectDraw device object DirectDrawCreate fills
//   0x8  field_8_        never read or written by `init` or `sub_635450`
//   0xC  locked_surface_ a secondary IDirectDrawSurface, locked elsewhere
//   0x10 locked_bits_    the pointer `locked_surface_->Unlock` releases
// `locked_surface_`/`locked_bits_` are proven by the vtable slot `init`
// calls through them: offset 0x80 is index 32, which in the real
// `IDirectDrawSurface` vtable (buffer.h's `#include <ddraw.h>`) is
// `Unlock(LPVOID)` - one pointer argument, matching the single value
// pushed beside the receiver.
class DDInit { public:
  DDInit();
  ~DDInit();
  // Native6354F0 retains ECX while forwarding depth to init(0,0,depth,1).
  // Descriptive name; the old free-stdcall seed lost the receiver contract.
  int init_default_mode(int depth);
  int init(int width, int height, int depth, int tgl);

  // 0x00635870, 1691 bytes - a `switch` over ~99 `DDERR_*` codes that turns
  // a failed HRESULT into a message box. Still a pending_bodies forwarder;
  // `init`'s own call site proves the shape: `this` in ecx, one explicit
  // stack argument, `call rel32`.
  int report_error(int hr);   // 0x00635870, defined in win.cpp

  // 0x00635750, PROMOTED out of src/unrecovered/00635750.cpp - the teardown
  // stages of `init`'s own mode switch as their own member: unlock and
  // release the locked surface, restore the display mode and release
  // DirectDraw, destroy the mode's window, then refresh the metrics.
  // jackal_close runs it on the global object below.
  void teardown();            // 0x00635750, defined in win.cpp

  // 0x006357D0, homed into win.cpp 2026-09-06. Locks `locked_surface_` and
  // caches the descriptor's lpSurface in `locked_bits_` and its lPitch in
  // `pitch_`, returning the bits pointer it already holds on a second call.
  LPVOID lock_surface();      // 0x006357D0, defined in win.cpp

  // 0x00635840, homed into win.cpp 2026-09-06. Unlocks `locked_surface_` and
  // clears `locked_bits_` - the lock half of the pair `teardown` runs, as its
  // own entry point.
  void flush();               // 0x00635840, defined in win.cpp

  // SELF-VERIFYING ANCHOR: DDInit has no base and no virtual, so its first
  // member is at 0 - and field_8_, two members later, states its own offset.
  // Annotating hwnd_ at 0x0 is only correct if the walk lands field_8_ on
  // 0x8, which it does.
  // RECOVERY_MEMBER: v1 DDInit hwnd_ 0x0 0x006354D0
  HWND hwnd_;  // 0x0
  // RECOVERY_MEMBER: v1 DDInit surf_ 0x4 0x006354D0
  IDirectDraw *surf_;
  // RECOVERY_MEMBER: v1 DDInit field_8_ 0x8 0x006354D0
  uint32_t field_8_;
  // RECOVERY_MEMBER: v1 DDInit locked_surface_ 0xC 0x006354D0
  IDirectDrawSurface *locked_surface_;
  // RECOVERY_MEMBER: v1 DDInit locked_bits_ 0x10 0x006354D0
  LPVOID locked_bits_;
  // 0x14. The stride `lock_surface` caches out of the descriptor's lPitch,
  // beside the bits pointer it caches in `locked_bits_`. `lock_surface` is
  // the only writer and nothing in the recovered tree reads it, which is why
  // the five-field survey above missed it - 0x006357D0 is what proves it.
  // RECOVERY_MEMBER: v1 DDInit pitch_ 0x14 0x006357D0
  int pitch_;
  // Native6354D0 clears these distinct words after skipping pitch at +14.
  // Their roles and the complete object's upper bound remain unverified.
  // RECOVERY_MEMBER: v1 DDInit unknown_18_ 0x18 0x006354D0
  uint32_t unknown_18_;
  // RECOVERY_MEMBER: v1 DDInit unknown_1C_ 0x1C 0x006354D0
  uint32_t unknown_1C_;
};

// The object itself at 0x009BE618, not a pointer to one.
extern DDInit WinDisplayInit;    // 0x009BE618, the object, not a pointer to it

// 0x005EFD00, 27 bytes - refreshes `WinScreenWidth`/`WinScreenHeight` from
// `GetSystemMetrics`. Defined in win.cpp; it was a forwarder until the body
// it already had, byte-exact and owned by no file, was promoted.
extern "C" int __cdecl DDInitRefreshScreenMetrics();

// 0x00628F30, a pending_bodies forwarder. `DDInit::report_error` tails into
// it once its popup is dismissed: the install-media check, 432 bytes against
// fixed-slot GetDriveTypeA/FindFirstFileA pointers, two callers, unrecovered.
int __cdecl cd_check();

// ===== Win's fixed-address globals, folded in from win_slots.h =====
// A `const` binding DEFINED IN A HEADER folds to the image's immediate;
// the same binding in a .cpp allocates storage this build never
// initialises, and the body then compiles `mov eax, [x]` where the image
// has the address inline. MEASURED on 0x005F5080: 7/29 instructions as a
// .cpp binding, BYTE_EXACT once the address reached the expression as a
// literal. That is the whole reason these live in a header - it was never
// a reason for them to live in a header of their OWN.

// SUPERSEDED - and the part that was wrong is the REMEDY, not the
// diagnosis. This note recorded, correctly and by measurement, that
// retyping these three to `Win **const` cost show_maximize and maximize
// their claims: the binding stopped folding to its immediate
// (`mov eax, dword ptr [eax]` where the image has the address inline) and
// VC6 stopped sharing one zeroed register across the neighbouring stores,
// so `xor edi,edi` + three `mov [addr], edi` became three immediate
// stores. Spelling the null `0` rather than `nullptr` did not bring it
// back.
// From that it concluded the `reinterpret_cast<int>(this)` at the use
// sites was LOAD-BEARING debt the ratchet had to keep. Measured 2026-08-25
// and REFUTED: the casts were not load-bearing, the BINDING was. Given
// real storage - `extern Win *WinCallbackWindow;` defined in win.cpp -
// all three take `this` with no cast at all, and show_maximize
// (0x005EDF00) and maximize (0x005EDE60) both stay BYTE_EXACT.
// Only two spellings were ever compared, and neither was the honest one.
// Same lever as clear_bubble_text; see its LEVER in win.cpp.
// `int *const`, for the same reason g_GetDC above carries that type: a
// fixed-address binding folds to its immediate only in this spelling. As
// `void **const` it compiled `mov eax, dword ptr [0]` in Win::redraw - the
// array base read from address zero - while two other bodies using the same
// name happened to fold anyway, which is what made it look like a per-body
// register-allocation quirk rather than the type. Callers cast at use.

int __cdecl pump_key_message();
void __cdecl pump_all_key_messages();
int __cdecl pump_mouse_move_message();
int __cdecl pump_mouse_button_message();
int __cdecl pump_sound_message();
void __cdecl pump_all_sound_messages();
void __cdecl do_all_sounds();

// Source-owned original helper; identity and behavior evidence are in its TU.
void __cdecl flip_active_bubble_text();

// Original window message adapters; source-owned mappings and evidence in win.cpp.
LRESULT __cdecl win_window_pos_changing(HWND hwnd, LPARAM parameter);
LRESULT __cdecl win_window_pos_changed(HWND hwnd, LPARAM parameter);
void __cdecl win_window_sized(HWND hwnd, unsigned int flags, int width, int height);
void __cdecl win_window_nonclient_sized(HWND hwnd, unsigned int flags, int width, int height);

// Source-owned Win adapters; each mapping stays beside its owner TU.
int __stdcall win_reparent_window(HWND hwnd, int unused);
void __cdecl win_forward_size(HWND hwnd, unsigned int flags, int width, int height);
void __cdecl win_forward_mouse_move(HWND hwnd, int x, int y, unsigned int keys);
int __cdecl win_forward_set_cursor(HWND hwnd, void *cursor, unsigned int hit, unsigned int message);
void __cdecl win_window_moved(HWND hwnd, int x, int y);
int __cdecl win_window_nc_paint(HWND hwnd);


// Win message-pump API and state. Original mappings and historical lessons are in win.cpp.
/*
 * AN OBJECT, NOT A `uint32_t *` TO A FIXED ADDRESS. The pointer form costs a
 * load the image does not perform: `MsgStatus |= 0x3F` is
 * `mov ecx, [0x9b7b9c]` in the image and `mov eax, [MsgStatus]` followed by
 * `mov ecx, [eax]` through a pointer, which is what held `flush_timer` at
 * 4 of 44 instructions. It also cannot work outside the shipped image - the
 * address is in terranx.exe's data, unmapped in a standalone build, so every
 * use through it is a wild pointer.
 */
extern uint32_t MsgStatus;  // 0x009B7B9C
void __cdecl do_all_tasks(Win *window);
BOOL __cdecl do_non_input();
void __cdecl do_all_draws();
void __cdecl do_all_keyboard();
void __cdecl do_video();
void __cdecl check_net();
void __cdecl do_net();

MEASURED inline BOOL __cdecl do_draw() {
    do_video();
    check_net();
    do_net();
    MSG msg;
    if (!PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, WM_CREATE)) {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
}

MEASURED inline BOOL __cdecl do_keyboard() {
    do_video();
    check_net();
    MSG msg;
    if (!PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYLAST, WM_CREATE)) {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
}

MEASURED inline void __cdecl do_all_non_input() {
    MsgStatus = 32;
    while (do_non_input()) {
        MsgStatus = 32;
    }
    MsgStatus = 0;
    do_net();
    check_net();
}
