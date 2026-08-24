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
  // homed from 005f54e0.cpp
  void on_paint(RECT * a1);
  void client_to_nonclient(RECT * rect);
  void client_to_nonclient(int * points, int * count);
  void bring_child_to_top(Win * child);

 public:
  // homed from 005ed170.cpp
  void nonclient_to_screen(RECT * a1);

 public:
  // homed from 005ed0a0.cpp
  void nonscreen_to_client(RECT * a1);

 public:
  // homed from 005ecec0.cpp
  int UNK4();

 public:
  // homed from 005ecdc0.cpp
  int UNK2(int a1);

 public:
  // homed from 005ecb60.cpp
  void remove_parent_dialog();

 public:
  // homed from 005ec8a0.cpp
  void get_mouse_pos(int * a1, int * a2);

 public:
  // homed from 005ec800.cpp
  void set_mouse_pos(int a1, int a2);

 public:
  // 0x005EEF60, a pending_bodies forwarder.
  void nonclient_to_client(int * a1, int * a2);

 public:
  // 0x005EBD80, a pending_bodies forwarder.
  int init(int a1, int a2, int a3, int a4, LPSTR a5, int a6, Win * a7, Menu * a8, BorderSizing * a9);

  friend class Scroll;
  // BaseButton's colour setters test the parent link before drawing.
  friend class BaseButton;

 public:
  // 0x005F6320  ?on_mouse_move@Win@@QAEXHHIH@Z - public, __thiscall,
  // void(int, int, unsigned int, int). GraphicWin::on_mouse_move is a pure
  // forwarder to it and is its only caller.
  void on_mouse_move(int a1, int a2, unsigned int a3, int a4);
  void on_mousewheel_up_vert(int a1);
  void on_mousewheel_down_horz(int a1);
  int get_lbutton_state();
  void on_mousewheel_up_horz(int a1);
  void on_mousewheel_down_vert(int a1);
  int UNK1(int a, int b, int c, int d, int e, int f, int g, int h, int i);
  int UNK5();
  int UNK6(int a);
  int on_set_cursor(void *a, unsigned int b, unsigned int c);
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
  void UNK8(int value);
  void UNK9(int value);
  void reset_window_clip();
  void sync_palette();
  int UNK3(int value);
  static int OnSetCursor(void *a1, void *a2, unsigned int a3, unsigned int a4);
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
  int on_query_new_palette();
  int get_vert_pos();
  int get_horz_pos();
  void set_vert_paging(int paging);
  void set_horz_paging(int paging);

  void on_move(int a1, int a2);
  void on_size(unsigned int a1, int a2, int a3);
  void on_size_nc(unsigned int a1, int a2, int a3);
  // ?on_sys_command@Win@@QAEHIHH@Z returns H. The body is empty either way
  // - VC6 emits the same `ret 0xc` - so only the decorated name moves.
  int on_sys_command(unsigned int a1, int a2, int a3);

  // Three base handlers the derived windows reach with a direct `call rel32`,
  // never through a vtable slot - PullDown::hide, DiploPop::hide,
  // Popup::on_nc_hittest and ReportIf::done all encode one - so they are
  // declared non-virtual on purpose. All three are unrecovered.
  void hide();                          // 0x005EDCD0  ?hide@Win@@QAEXXZ
  int on_nc_hittest(int a1, int a2);    // 0x005F5AD0  ?on_nc_hittest@Win@@QAEHHH@Z
  void release_modal();                 // 0x005EE280  ?release_modal@Win@@QAEXXZ
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
  uint32_t field_C8_;
  uint32_t field_CC_;
  uint32_t field_D0_;
  uint32_t field_D4_;
  uint32_t field_D8_;
  // RAW STORAGE, NOT A TYPED Heap, and the image is what says so. The
  // marker on ??0Win@@QAE@XZ records exactly one call target -
  // 0x0062BA80, the AutoSound base constructor - so Win's constructor
  // never builds a Heap here. Declared `Heap heap_`, VC6 emits
  // `lea ecx, [esi + 0xdc]; call 0x5d4560` for Heap's real constructor
  // and the body stops matching. `uint32_t[]` keeps the size and the
  // 4-byte alignment; nothing in the tree reads Win::heap_ by name.
  // 0xDC is SEPARATE because the constructor zeroes it in its member
  // initialiser list - the image writes it at 0x005EB3FC, one
  // instruction BEFORE the vfptr store, and only the list runs there.
  uint32_t heap_head_;
  uint32_t heap_[sizeof(Heap) / 4 - 1];
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
};

static_assert(sizeof(Win) == 0x444, "Win layout must match the legacy ABI");

extern const uint32_t WinPrimaryVtable;
extern const uint32_t WinSecondaryVtable;
uint32_t *const WinStaticDefaults = (uint32_t *)0x00696D34;
uint32_t *const WinDynamicDefaults = (uint32_t *)0x009B7AF0;
Win *__fastcall win_construct_redirect(Win *self, void *);
int __fastcall win_move_redirect(Win *self, void *, int x, int y);
int __fastcall win_is_visible_redirect(Win *self, void *);
void __fastcall win_client_to_screen_redirect(
    Win *self, void *, int *x, int *y);
int __fastcall win_on_query_new_palette_redirect(Win *self, void *);
int __fastcall win_get_vert_pos_redirect(Win *self, void *);
int __fastcall win_get_horz_pos_redirect(Win *self, void *);
void __fastcall win_set_vert_paging_redirect(Win *self, void *, int paging);
void __fastcall win_set_horz_paging_redirect(Win *self, void *, int paging);

BOOL __cdecl in_box(int x, int y, const RECT *rect);
int __cdecl in_box(
    int x, int y, int left, int top, int width, int height);
void __cdecl offset_rect(RECT *rect, int dx, int dy);
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height);
int __stdcall rect_center(RECT *rect, int *x, int *y);
int __fastcall tutwin_rect_center_redirect(
    void *self, void *, RECT *rect, int *x, int *y);

int __fastcall win_is_dialog_focus_redirect(Win *self, void *);

HDC __cdecl win_get_hdc_redirect();
void __cdecl win_release_hdc_redirect();

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
// not a `Win` member; byte-exact in src/recovered/005f86a0.cpp.
extern "C" void __stdcall sub_5f86a0(int a1);

int __fastcall win_set_cursor_redirect(Win *self, void *, int name);

// The cursor refresh this setter triggers is a 2528-byte body with six call
// targets, still an original dependency. Tests rebind this seam.

void __cdecl win_clear_bubble_text_redirect();

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
int *const WinBubbleCompanion = (int *)0x009B7A4C;
RECT *const WinBubbleRect = (RECT *)0x009B6E38;

// Both refresh bodies remain original dependencies: update_screen is 383
// bytes with four call targets, flip 1223 bytes with fourteen.

int __fastcall win_unk1_redirect(
    Win *self, void *, int a, int b, int c, int d, int e, int f, int g, int h, int i);
int __fastcall win_unk5_redirect(
    Win *self, void *);
int __fastcall win_unk6_redirect(
    Win *self, void *, int a);
int __fastcall win_on_set_cursor_redirect(
    Win *self, void *, void *a, unsigned int b, unsigned int c);

void __cdecl win_set_def_focus_redirect(int focus);
int *const WinDefaultFocus = (int *)0x009B7AEC;

void __fastcall win_unk8_redirect(Win *self, void *, int value);
void __fastcall win_unk9_redirect(Win *self, void *, int value);
void __fastcall win_reset_window_clip_redirect(Win *self, void *);
void __fastcall win_on_move_redirect(Win *self, void *, int a1, int a2);
void __fastcall win_on_size_redirect(Win *self, void *, unsigned int a1, int a2, int a3);
void __fastcall win_on_size_nc_redirect(Win *self, void *, unsigned int a1, int a2, int a3);
int __fastcall win_on_sys_command_redirect(Win *self, void *, unsigned int a1, int a2, int a3);

// The active palette lives at a fixed address; rebindable so tests can
// point it at a local rather than requiring the mapped global.
inline Palette *&WinActivePalette() { return *reinterpret_cast<Palette **>(0x009B8180); }

void __fastcall win_sync_palette_redirect(Win *self, void *);

void __fastcall win_set_vert_pos_redirect(Win *self, void *, int position);
void __fastcall win_set_horz_pos_redirect(Win *self, void *, int position);
void __fastcall win_set_vert_range_redirect(Win *self, void *, int minimum, int maximum);
void __fastcall win_set_horz_range_redirect(Win *self, void *, int minimum, int maximum);
int __cdecl win_onsetcursor_redirect(void *a1, void *a2, unsigned int a3, unsigned int a4);
int __fastcall win_unk3_redirect(Win *self, void *, int value);

int __fastcall win_is_descendant_redirect(Win *self, void *, Win *candidate);

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



