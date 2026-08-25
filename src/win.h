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
// The ninth Win::init / GraphicWin::init argument, spelled PAUBorderSizing@@
// in both mangled names - a struct, hence the tag. Nothing on the recovered
// surface dereferences it, so it stays incomplete.
struct BorderSizing;
class Scroll; // forward declaration

// THE EMBEDDED OBJECT AT Win+0xC8. Only `??0Win@@QAE@XZ`, `??1Win@@QAE@XZ`
// and this class's own out-of-line scalar deleting destructor (0x005F8770)
// reference its vtable at 0x0066FF30 anywhere in .text, and that table is
// exactly ONE slot - the destructor's own address - so it belongs to Win
// alone. It is a small singly-linked list: `head_` walks nodes whose own
// +0x8 is a payload `free()`s and +0xC is the next pointer, proven by
// 0x005F8770 and by `Win::~Win()` (0x005EBC90) performing the identical
// walk INLINE at Win+0xCC.._0xDC rather than calling it. `external_` is a
// "do not own this list" guard: nonzero skips the free loop entirely.
// `current_` is scratch during that loop, but is read live elsewhere too -
// `Win::is_dialog_focus` (0x005F2CA0) reads it as the top-of-stack node,
// whose own +0x4 holds the `Win *` currently holding focus.
//
// NOT A SECOND BASE: it is a plain member (`Win::list_`), so Win's own
// vtable at offset 0 stays exactly as it is - hand-installed, not
// compiler-generated.
class WinNodeList {
 public:
  WinNodeList() : head_(0), current_(0), count_(0), tail_(0), external_(0) {}
  // DEFINED IN THE CLASS so VC6 can inline it into `??_GWinNodeList`,
  // which is what the image is: ONE function at 0x005F8770 that restores
  // the vfptr and walks the list. Out-of-line, the deleting destructor
  // tail-CALLS `??1WinNodeList@@UAE@XZ` instead and measures 14
  // instructions against the image's 54.
  virtual ~WinNodeList() {
      // IMAGE ORDER: the head is read before the vfptr-restore that every
      // scalar deleting destructor opens with, matching `Win::~Win()`
      // performing the same walk inline. `external_` guards the whole loop -
      // nonzero means this list does not own its nodes.
      void *node = head_;
      if (node != 0) {
          if (external_ == 0 && count_ > 0) {
              int i = 0;
              do {
                  Win *next = *reinterpret_cast<Win **>(
                      reinterpret_cast<char *>(node) + 0xC);
                  current_ = next;
                  void *payload = *reinterpret_cast<void **>(
                      reinterpret_cast<char *>(node) + 8);
                  if (payload != 0) {
                      std::free(payload);
                  }
                  *reinterpret_cast<void **>(
                      reinterpret_cast<char *>(head_) + 8) = 0;
                  if (head_ != 0) {
                      std::free(head_);
                  }
                  node = current_;
                  head_ = node;
                  ++i;
              } while (i < count_);
          }
          head_ = 0;
          tail_ = 0;
          count_ = 0;
      }
      tail_ = 0;
  }

  void *head_;      // 0x4  (Win+0xCC)
  void *current_;   // 0x8  (Win+0xD0)
  int count_;         // 0xC  (Win+0xD4)
  // AN INDEX, not a pointer: the node walk does `++tail_ == count_`,
  // which is a ring position. Same four bytes either way, so the
  // layout is unchanged and the assert still holds.
  int tail_;          // 0x10 (Win+0xD8)
  int external_;      // 0x14 (Win+0xDC)
};

 /*
  * Win class: Most basic window class.
  */
// WIN IS AN AUTOSOUND. The layout said so before the inheritance did: this
// class held `AutoSound auto_sound_` at offset 0, AutoSound's own first member
// was the vtable dword, so Win's vtable and AutoSound's were the same four
// bytes - and `iFlags_` sat immediately after AutoSound's 0x98, exactly where
// a base subobject puts it. tools/flat_classes.py flags the embedded-base
// shape independently.
//
// Declaring it is what gives Win, and through it GraphicWin, a real vfptr.
class Win : public AutoSound {
 public:
  // Reaches Win's fields directly: the byte-exact body for 0x005F5080
  // is a free function (both member spellings were measured and
  // refuted), and it reads `iFlags_` the way the image does.
  friend void __cdecl add_parent(Win *);
  // Homed into win.cpp 2026-08-25. It walks a window's children and
  // reads the same private fields the class's own z-order code does;
  // the image treats it as part of Win even though it is not a member.
  friend void __cdecl recurse_zorder(Win *window);
  // The window class's WM_SYSKEY router, homed into win.cpp. It reads
  // poWinBase_, iSomeFlag_ and win_parent_ off whichever window it
  // resolves as active; the image treats it as part of Win even though
  // its receiver is an HWND, not a `this`.
  friend void __cdecl OnSysKey(void *, unsigned int, long, int, unsigned int);
 public:
  // homed from 005f1340.cpp
  bool __cdecl OnPaint(HWND a1);

 public:
  // homed from 005ee330.cpp
  int resize_event(int width, int height);

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

  friend class Scroll;
  // BaseButton's colour setters test the parent link before drawing.
  friend class BaseButton;

 public:
  // 0x005F6320  ?on_mouse_move@Win@@QAEXHHIH@Z - public, __thiscall,
  // void(int, int, unsigned int, int). GraphicWin::on_mouse_move is a pure
  // forwarder to it and is its only caller.
  void on_mousewheel_up_vert(int delta);
  void on_mousewheel_down_horz(int delta);
  int get_lbutton_state();
  void on_mousewheel_up_horz(int delta);
  void on_mousewheel_down_vert(int delta);
  int UNK1(int a, int b, int c, int d, int e, int f, int g, int h, int i);
  int UNK5();
  int UNK6(int a);
  Win();

  // STATIC, not the `QAA` member the census reconstructs: the body at
  // 0x005F04E0 reads no receiver, and its one caller sets up no ecx.
  static void close_class();   // 0x005F04E0
  // DECLARED, NOT DEFINED EMPTY. The real destructor is at 0x005EBC90, and it
  // ends `ret` rather than `ret 4` - it takes no vbase flag - so a direct call
  // to it is reachable, unlike RadioButton's. An empty inline body compiles
  // every call away, which is why `??_GWin` emitted `call dword ptr [...]`
  // through a seam where the image emits `call rel32`. Forwarded in
  // `pending_bodies.cpp` until its body lands.
  ~Win();


  // Returns `this`: the image's own real `??0Win@@QAE@XZ` sets `eax = this`
  // before its epilogue, which a void-returning method never emits.
  int move(int x, int y);
  int is_visible();
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
  static void set_def_focus(int focus);
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
 private:
  uint32_t iFlags_;
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
  // 0xE0. Zeroed by the constructor; nothing yet reads it.
  uint32_t field_E0_;
  // 0xE4, 0xE8, 0xEC. PROVEN pointers, not raw storage: `redo_caption_buttons`
  // (0x005F2760) positions each with `move()` and reads a field at +0x4C4 on
  // every one - past `sizeof(Win)`, so each points at a Win-derived object of
  // its own. Processed right-to-left (the standard Windows caption-button
  // order), and the one at +0xE8 alone swaps an icon by `IsZoomed()` -
  // maximize/restore is the only caption button whose icon depends on zoom
  // state. Names are INFERRED from that, not independently confirmed.
  Win *minimize_button_;   // 0xE4
  Win *zoom_button_;       // 0xE8
  Win *close_button_;      // 0xEC
  Menu *menu_;
  uint32_t field_F4_;
  uint32_t field_F8_;
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
  uint32_t field_12C_;
  uint32_t field_130_;
  uint32_t field_134_;
  uint32_t field_138_;
  RECT outer_rect_;
  RECT client_rect_;
  uint32_t field_15C_;
  uint32_t field_160_;
  uint32_t field_164_;
  uint32_t field_168_;
  uint32_t field_16C_;
  uint32_t field_170_;
  uint32_t field_174_;
  uint32_t field_178_;
  uint32_t field_17C_;
  uint32_t field_180_;
  uint32_t field_184_;
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
  uint32_t field_400_;
  uint32_t field_404_;
  uint32_t field_408_;
  uint32_t field_40C_;
  uint32_t field_410_;
  uint32_t field_414_;
  uint32_t field_418_;
  uint32_t field_41C_;
  uint32_t field_420_;
  uint32_t field_424_;
  uint32_t field_428_;
  uint32_t field_42C_;
  uint32_t field_430_;
  uint32_t field_434_;
  uint32_t field_438_;
  Scroll *scroll_vert_;
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
  virtual void vslot_04() {}  // 0x00404280
  virtual int vslot_05(int = 0, int = 0, int = 0) { return 0; }  // 0x00404220
  virtual int vslot_06() { return 0; }  // 0x00406B30
  virtual int vslot_07() { return 0; }  // 21 call sites pass nothing and
                                      // the image pushes nothing  // 0x00404230
  virtual int vslot_08() { return 0; }  // 0x00406B30
  virtual int vslot_09(int value = 0) { return value; }  // 0x00404230
  virtual int vslot_10(int = 0) { return 0; }  // 0x00404250
  virtual int vslot_11(int = 0, int = 0, int = 0, int = 0) { return 0; }  // 0x00404240
  virtual void vslot_12() {}  // 0x00404280
  virtual void vslot_13(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_14() {}  // 0x00404280
  virtual void vslot_15(int = 0) {}  // 0x00404270
  virtual void on_mouse_move(int x, int y, unsigned int keys, int from_parent);  // slot 16  0x005F6320
  virtual void vslot_17(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_18(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_19(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_20(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_21(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_22(int = 0, int = 0) {}  // 0x00404260
  // NO DEFAULT ARGUMENTS. They compiled into `push 0; push 0` at every
  // call, which the image does not emit - measured on 0x005EE750 and
  // 0x005F6320. Twenty-one call sites pass nothing; the one that passed
  // two was the outlier.
  virtual int vslot_23() { return 0; }  // 0x00404260
  virtual void vslot_24(int = 0, int = 0) {}  // 0x00404260
  virtual int vslot_25(int = 0, int = 0) { return 0; }  // 0x00406A80
  virtual int vslot_26(int = 0) { return 0; }  // 0x00404250
  virtual int vslot_27(int = 0) { return 0; }  // 0x00404250
  virtual void vslot_28(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_29(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_30(int = 0) {}  // 0x00404270
  virtual void vslot_31(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_32(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_33(int = 0, int = 0) {}  // 0x00404260
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
  virtual void vslot_44(int = 0) {}  // 0x00404270
  virtual void vslot_45(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_46(int = 0, int = 0, int = 0) {}  // 0x00406B20
  virtual void vslot_47(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_48(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_49(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_50(int = 0) {}  // 0x00404270
  virtual void vslot_51(int = 0) {}  // 0x00404270
  virtual void vslot_52() {}  // 0x00404280
  virtual void vslot_53(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_54(int = 0) {}  // 0x00404270
  virtual void vslot_55(int = 0, int = 0) {}  // 0x00404260
  virtual void vslot_56(int = 0) {}  // 0x00404270
  virtual int set_modal(int flag, int (*poll)(), Win *owner);  // slot 57  0x005EE190
  virtual void release_modal();  // slot 58  0x005EE280
  virtual int vslot_59() { return 1; }  // 0x00406840
  virtual void vslot_60(int = 0, int = 0) {}  // 0x00404260
  virtual int vslot_61() { return 0; }  // 0x00406B30
  virtual void vslot_62() {}  // 0x00404280
  virtual void vslot_63() {}  // 0x00404280
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
  // slot 77  0x005F62D0. NAMED 2026-08-25 from its only dispatch: the
  // WM_MOUSEWHEEL arm of Win::window_proc calls it with the wParam key
  // flags, the wheel delta out of the wParam's high half, the cursor
  // position and the from-parent flag. The five-argument signature is that
  // call site; the body at 0x005F62D0 is still unrecovered.
  // Empty inline, the same placeholder shape the other uncatalogued
  // slots carry: 0x005F62D0's body is not recovered, so this occupies
  // the slot without claiming to be it.
  virtual void on_mouse_wheel(unsigned int keys, unsigned int delta,
                              int x, int y, int from_parent) {}
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
  virtual void on_l_button_down(long flags, int x, int y, unsigned int keys, int dbl);  // slot 81  0x005F63C0
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
  void UNK7(int a1, int a2, int a3, int a4);
  int __stdcall adjust_menus(void *a1);
  void do_caption_buttons();
  int maximize();
  void on_mousewheel_down(int delta);
  void on_mousewheel_up(int delta);
  void set_bottom_border_thickness(int thickness);
  int show_maximize();
  void update_nc_buffer(int a1);

 public:
  void add_child(Win* child);
  void bring_to_top();
  int get_rbutton_state();
  void left_down_event(int a1, int a2, int a3);
  int on_redraw(int, int);
  // Returns Buffer *, not int: every arm of its switch returns one of the
  // four render buffers. The `int` was the raw-offset form's type leaking
  // into the signature. Unclaimed, so no marker moves with this.
  Buffer *redraw_nc_buffer(int index);
  void __cdecl remove_parent(Win* a1);
  void set_border_thickness(int thickness);
  void set_parent_dialog(Win *dialog);
  void __cdecl update();
  void __cdecl update_zorder();
  void window_line_raw(int a1, int a2, int a3, int a4, int a5, int a6, unsigned int a7);

 public:
  void __cdecl OnRButtonDown(void * a1, long a2, int a3, int a4, unsigned int a5);
  void __cdecl OnRButtonUp(void * a1, int a2, int a3, unsigned int a4);
  void __cdecl bring_parent_to_top(Win * a1);
  void draw_rect_border(int x1, int y1, int x2, int y2, HGDIOBJ pen1, HGDIOBJ pen2, int unused7);
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
  int minimize();
  void nonclient_to_screen(int * x, int * y);
  void screen_to_nonclient(int * x, int * y);
  void set_caption(char * text);
  void sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6);
  int sub_63c340();

 public:
  void __cdecl OnKey(void * a1, unsigned int a2, long a3, int a4, unsigned int a5);
  void OnLButtonUp(void *hwnd, int x, int y, unsigned int keys);
  int __cdecl OnSysCommand(HWND a1, unsigned int a2, int a3, int a4);
  void paint_tiled(Buffer *tile, int x_origin, int y_origin, int clip_left, int clip_top, int clip_width, int clip_height, int unused8);
  void update_window_to_buffer(Buffer * buffer);

 public:
  void __cdecl OnMouseMove(void * a1, int a2, int a3, unsigned int a4);

 public:
  int __cdecl OnQueryNewPalette(void * a1);

 public:
  long __cdecl OnActivate(void *a1, unsigned int a2, void *a3, long a4);
  void screen_to_client(RECT * rect);

 public:
  // Tear the window down: clears the focus/tracking globals and walks
  // the child list. Homed from 005eb640.cpp.
  void close();

 public:
  void __cdecl redraw();  // 0x005F4CC0

 public:

 public:
  // A MEMBER: the caller at 0x0063C340 sets up `mov ecx, esi` before
  // this call, which is a thiscall receiver, not a __cdecl free function.
  int sub_63c7c0();  // 0x0063C7C0
};

static_assert(sizeof(Win) == 0x444, "Win layout must match the legacy ABI");


BOOL __cdecl in_box(int x, int y, const RECT *rect);
int __cdecl in_box(
    int x, int y, int left, int top, int width, int height);
void __cdecl offset_rect(RECT *rect, int dx, int dy);
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height);
int __stdcall rect_center(RECT *rect, int *x, int *y);

int __fastcall win_is_dialog_focus_redirect(Win *self, void *);


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
struct IDirectDrawSurface;  // <ddraw.h>, included where it is called
extern IDirectDrawSurface *DirectDrawSurface;

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
// as `BasePopScreenWidth`.
extern int WinScreenWidth;
extern int WinScreenHeight;

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
extern int WinMaximizeIcon;         // 0x009B7B04
extern int WinRestoreIcon;          // 0x009B7B08
extern int WinModalResult;          // 0x009B6EF0
extern int WinSavedAreaWidth;       // 0x009B6F88
extern int WinSavedAreaHeight;      // 0x009B6F8C
extern int WinSizingFlag;           // 0x009B7AD4

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
void __cdecl sub_63c4e0(int a1);

// 0x009B7B3C. Cleared on every WM_MOUSEMOVE and read by
// `Win::update_cursor`, which is the only other function that touches it.
extern int WinCursorMoved;

// 0x009B7A88 and 0x009B7A94. Two `__cdecl` hooks the procedure calls on the
// way out: the second on mouse messages with (hwnd, lParam), the first on
// every message at all, with no arguments. `Effect::init` installs the
// first; `Win::OnKey`, `OnLButtonDown` and `OnRButtonDown` read them too.
extern void(__cdecl *WinMessageHook)();
extern void(__cdecl *WinMouseHook)(HWND window, LPARAM position);

// 0x009B7A8C. A third hook, called after WM_KEYDOWN with the virtual key.
// `Win::OnKey` is its only other reader.
extern void(__cdecl *WinKeyHook)(WPARAM key);

// 0x005F86A0. The one thing `window_proc`'s WM_SYSCOMMAND arm calls that is
// not a `Win` member. Its body was homed from src/recovered/005f86a0.cpp
// into win.cpp and measures BYTE_EXACT there.
// 0x005F86A0, homed into win.cpp.
void __stdcall sub_5f86a0(int a1);


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
extern IDirectDrawSurface *DirectDrawBackBuffer;  // 0x009BC49C
extern RECT DirectDrawClipRect;        // 0x009BC2D0
int *const WinBubbleActive = (int *)0x009B7A50;
// A `Win *`, not a word: `Win::set_bubble_text` (0x005F83D0) stores `this`
// here directly, with no cast - retyped from `int *` so that store needs
// none either.
Win **const WinBubbleCompanion = (Win **)0x009B7A4C;
RECT *const WinBubbleRect = (RECT *)0x009B6E38;

// Both refresh bodies remain original dependencies: update_screen is 383
// bytes with four call targets, flip 1223 bytes with fourteen.


int *const WinDefaultFocus = (int *)0x009B7AEC;


// The active palette lives at a fixed address; rebindable so tests can
// point it at a local rather than requiring the mapped global.
inline Palette *&WinActivePalette() { return *reinterpret_cast<Palette **>(0x009B8180); }




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
  int init(int width, int height, int depth, int tgl);

  // 0x00635870, 1691 bytes - a `switch` over ~99 `DDERR_*` codes that turns
  // a failed HRESULT into a message box. Still a pending_bodies forwarder;
  // `init`'s own call site proves the shape: `this` in ecx, one explicit
  // stack argument, `call rel32`.
  int report_error(int hr);   // 0x00635870, defined in win.cpp

  HWND hwnd_;
  IDirectDraw *surf_;
  uint32_t field_8_;
  IDirectDrawSurface *locked_surface_;
  LPVOID locked_bits_;
};
DDInit *const WinDisplayInit = reinterpret_cast<DDInit *>(0x009BE618);

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

typedef void(__cdecl *FnSetActiveWindow)(Win *);

static int * const WinFocusStack = (int *)0x009B7A1C;
static int * const WinModalDepth = (int *)0x009B7AE4;
static int * const WinFillColour = (int *)0x00696D14;
static int * const WinKeyRingCursor = (int *)0x00696D5C;
static int * const WinMsgIncreaseMaxChildren = (int *)0x00696D60;
static int * const WinMsgTooManyChildren = (int *)0x00696D80;
static int * const WinMsgIncreaseMaxParents = (int *)0x00696D94;
static int * const WinMsgTooManyParents = (int *)0x00696DB4;
static int * const WinMdebugCodeEnd = (int *)0x00696DFD;
static int * const WinBubbleWindow = (int *)0x009B22F0;
static int * const WinTopDialog = (int *)0x009B2300;
static int * const WinDrawFlags = (int *)0x009B238C;
static int * const WinPopupWindow = (int *)0x009B23B4;
static int * const WinDialogList = (int *)0x009B2494;
static int * const WinPopupCount = (int *)0x009B26EC;
static int * const WinScreenClipRect = (int *)0x009B74C0;
static int * const WinGlobalCallback = (int *)0x009B7A90;
static int * const WinMouseCallback = (int *)0x009B7A94;
static int * const WinKeyModifiers = (int *)0x009B7B18;
static int * const WinMainHwnd = (int *)0x009B7B28;
static int * const WinKeyRingStart = (int *)0x009B7B48;
static int * const WinKeyRingEnd = (int *)0x009B7B51;
static int * const WinPalette = (int *)0x009B8180;
static int * const WinModalFocus = (int *)0x009B8D7C;
static int * const WinTitleBarHeight = (int *)0x009B8DD4;
static int * const WinDirtyRect = (int *)0x009B6EE8;
static int * const WinBackBuffer = (int *)0x009B7A68;
static int * const WinViewOriginX = (int *)0x009B7A70;
static int * const WinViewOriginY = (int *)0x009B7A74;
// TRIED 2026-08-25 and REFUTED BY MEASUREMENT: retyping these three to
// `Win **const` - the honest type, and the spelling win.h's
// WinBubbleCompanion uses - cost show_maximize and maximize their claims.
// The binding stopped folding to its immediate (`mov eax, dword ptr [eax]`
// where the image has the address inline) and VC6 also stopped sharing one
// zeroed register across the neighbouring stores, so the image's
// `xor edi,edi` + three `mov [addr], edi` became three immediate stores.
// Spelling the null `0` rather than `nullptr` did not bring it back. The
// `reinterpret_cast<int>(this)` at the use sites is therefore LOAD-BEARING,
// not laziness - it is raw-self-access debt the ratchet has to keep.
// See the recorded lesson `binding-type-decides-folding`.
static int * const WinCallbackWindow = (int *)0x009B7AB8;
static int * const WinInputFocus = (int *)0x009B7AC4;
static int * const WinActiveWindow = (int *)0x009B7AC8;
static int * const WinPendingFocus = (int *)0x009B7B38;
static int * const WinDDSurface = (int *)0x009BC498;
// `int *const`, for the same reason g_GetDC above carries that type: a
// fixed-address binding folds to its immediate only in this spelling. As
// `void **const` it compiled `mov eax, dword ptr [0]` in Win::redraw - the
// array base read from address zero - while two other bodies using the same
// name happened to fold anyway, which is what made it look like a per-body
// register-allocation quirk rather than the type. Callers cast at use.
static int * const g_win_array = (int *)0x009B6630;
static uint32_t * const WinStaticDefaults = (uint32_t *)0x00696D34;
static uint32_t * const WinDynamicDefaults = (uint32_t *)0x009B7AF0;
static Win ** const g_zorder_list = (Win **)0x009B6E48;
