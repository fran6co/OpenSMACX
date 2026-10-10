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
#include "graphicwin.h"
#include "dialogs.h"
#include "checkbox.h"
#include "flatbutton.h"
#include "stringstruct.h"

// ONE OF THE POPUP'S TWO 0x118 TEXT BLOCKS (0x2E64 and 0x2F7C inside the
// object). 0x104 of string plus five numbers: BasePop::start (0x00601BF0)
// strcat's into `text` and atoi/atol's into each of the five, and
// BasePop::close (0x00600F00) restores the whole 0x118 from the file-backed
// defaults. popup_wave_callback consumes +0x104 as a wave index and +0x108
// as an integer enable flag. The generic parser also fills the other fields.
struct BasePopTextBlock {
  char text[0x104];  // 0x0000
  int field_104_;    // 0x0104
  int field_108_;    // 0x0108
  int field_10C_;    // 0x010C
  int field_110_;    // 0x0110
  int field_114_;    // 0x0114
};

static_assert(sizeof(BasePopTextBlock) == 0x118,
              "BasePopTextBlock layout must match the original executable");
static_assert(offsetof(BasePopTextBlock, field_104_) == 0x104,
              "BasePopTextBlock first number must follow the text");
static_assert(offsetof(BasePopTextBlock, field_114_) == 0x114,
              "BasePopTextBlock last number must end the block");

// FX::init installs popup_wave_callback into the default at 0x009B8CF8.
// BasePop::exec passes the complete block, not merely its leading string,
// and cleans two arguments after each call (0x0060369C / 0x00603787).
typedef void (__cdecl *BasePopConfigCallback)(BasePopTextBlock *, int);
extern BasePopConfigCallback BasePopDefaultField3094;

// The generic popup allocation hook is initialized by this BasePop library
// owner, then WinMain may install the application-derived Popup allocator.
class BasePop;
typedef BasePop *(__cdecl func_popup_alloc)();
extern func_popup_alloc *PopupAllocHook;

// Shared startup-caption storage consumed by BasePop configuration/start and
// Popup convenience callers. The existing name and 256-byte extent remain
// provisional; the image proves the address and writes, not the full extent.
extern char PopupStartCaption[256];

 /*
  * BasePop class
  */
class BasePop : public GraphicWin {
 public:
  // homed from 00605130.cpp
  void on_left_down(int x, int y);

 public:
  // homed from 006017a0.cpp
  int set_ok_text(char *text);

 public:
  // homed from 00601ac0.cpp
  void set_height(int height);

 public:
  // homed from 00605010.cpp
  void set_external_window(GraphicWin *external_window, int loc_x, int loc_y,
                           int width, int height);

 public:
  // homed from 00604680.cpp
  // STATIC, and the `QAAX` the catalogue spells is a receiver nothing sets
  // up: jackal_close calls this actual static operation with ecx untouched.
  // The body reads no `this` and the image's own bytes agree. Same case as fallout() and
  // set_def_ok_text above - the `// symbol` fact on the marker records the
  // SAX this tree emits.
  static void __cdecl close_class();

 public:
  // 0x006044D0 reads WinModalFocus, not an incoming receiver, and returns
  // without popping its caller's notification argument.
  static void __cdecl double_click(int button_id);

 public:
  // homed from 00604410.cpp
  int on_key_down(int key);
  // Console::go_to reads the original +0x3100 input-poll state after exec.
  uint32_t input_poll_state() const { return field_3100_; }

 public:
  // homed from 00601980.cpp
  int string(char *text);
  // 0x006019F0, homed from 006019f0.cpp: the item-text twin of string().
  int button(char *text);
  // EVIDENCE-FREE as a name, for the same reason the marker on
  // BasePop::UNK1 gives: the image names it nowhere.
  int UNK1(char *text);  int set_cancel_text(char *text);  // 0x00601820, set_ok_text's twin
  // 0x00601A40, owned_text_'s setter

  /* THE OBJECT, 0x3230 BYTES, as the recovered bodies now read it. Every
   * region below is named by a member; the offsets are the image's.
   *
   *   0x0000  GraphicWin (Win + Buffer + the canvas pointers) - the base
   *   0x0A14  external_window_   the window the popup draws its dialog into
   *   0x0A18  default_loc_a_ / default_loc_b_ (0x0a1c)   start-up position
   *   0x0A20  dialog_width_ / dialog_height_ (0x0a24)    default dialog size
   *   0x0A28  heap_                    the popup's own allocation heap
   *   0x0A3C  owner_window_            the window the popup was opened for
   *   0x0A40  selected_id_             the entry the popup settled on
   *   0x0A44  value_                   the popup's value (start's 3rd argument)
   *   0x0A48  owned_text_              one heap string from the config script
   *   0x0A4C..0x0A58  four dwords no body in the tree reaches
   *   0x0A5C  flat_button1_ (0xB4C) / 0x15A8 flat_button2_  the two rows
   *   0x20F4  a dword close() zeroes beside sprite_
   *   0x20F8  item_buttons_[8]         eight owned FlatButton pointers
   *   0x2118  sprite_ (0x2C)           the popup's own picture
   *   0x2144  caller_sprite_           pops()' sprite argument, or 0
   *   0x2148  two dwords from BasePopDefaultField2148
   *   0x2150  item_ids_ (0x30)         the item-text list
   *   0x2180  unit_ids_ (0x30)         the unit-name list
   *   0x21B0..0x21CC  seven dwords: four from BasePopDefaultField21B0, two
   *           from BasePopDefaultField21C0, and the pair close() resets to
   *           0 / -1 (0x21c8/0x21cc)
   *   0x21D0  dialogs_ (0xC94)         ListBox + RadioButton + CheckBox +
   *           SpriteBox + EditGroup over one GraphicWin and one Dialog base
   *           - see dialogs.h for the prefixes, and `THE DIALOG VIRTUAL
   *           BASE` below for how these bodies reach into it
   *   0x2E64  text_block_a_ (0x118) / 0x2F7C text_block_b_ (0x118)
   *   0x3094  two dwords from BasePopDefaultField3094
   *   0x3098  spot_                    the click spots
   *   0x30A4..0x30F4  twenty-two dwords of state close() zeroes (see below)
   *   0x30F8  loc_a_ / loc_b_ (0x30fc)      the popup's position
   *   0x3100  the at-key poll flag pops() publishes, then the 0x3104 triple
   *   0x3110  the four string fonts, the four colour slots x four tiers
   *           (0x3120..0x315c), three spare dwords, the three button fonts
   *           and the button colour slots (0x3178..0x319c)
   *   0x31A0  the BasePopDefaultField31A0 row, then two more defaults
   *   0x31B8  default_block_[27]       nine rows of three, ending 0x3224
   *   0x3224  ok_text_ / cancel_text_ (0x3228)   heap captions
   *   0x322C  the dword BasePopDefaultField322C is published into
   *
   * THE DIALOG VIRTUAL BASE. Dialogs carries Dialog as a virtual base at
   * +0xBA0, so the popup's three writes into it - the default width at
   * Dialog+0x2C, the height at +0x30, and the two show-state bits at +0x20 -
   * are spelled as a QUALIFIED MEMBER ACCESS on the member:
   *
   *       dialogs_.Dialog::field_2C_ = width;
   *
   * VC6 walks the vbtable at +0x21d0 for the base's displacement and folds
   * the rest into the store, which is exactly the image's addressing
   * (`mov eax,[ecx+0x21d0]; mov edx,[eax+8]; mov [edx+ecx+0x21fc],..`).
   * set_width, set_external_window, init, close and the two click handlers
   * all use it; the unqualified spelling is a different field (ListBox has
   * its own field_2C_ at the same offset) and binds there silently.
   */
  // `pops()` (popup.cpp) reaches BasePop's fields directly - caller_sprite_,
  // external_window_, field_30B4_..field_30C0_, field_3100_, field_3104_,
  // field_3108_ - the way the image's own free function does, through the
  // same object rather than through an accessor per field.
  friend int __cdecl pops(char *caption, char *label, int value,
                          char *override_text, int flags, Sprite *sprite,
                          int pair_value_a, int pair_value_b,
                          int (__cdecl *callback)());
  // Popup is the derived class, and its own bodies reach this state directly -
  // hide() reads field_30AC_ (the auto-hide seconds) and external_window_ the
  // way the image reads them, `mov ecx,[esi+0x30ac]` / `[esi+0xa14]`, with no
  // accessor call anywhere in the frame. Friendship is access only: it moves
  // no byte of layout and no line of codegen.
  friend class Popup;
  // The image's own free popup drivers do the same - read external_window_,
  // field_3100_ and the dialogs_ member straight off the instance they exec.
  // pop_ask additionally walks dialogs_' Dialog virtual base for the item
  // count, which is what the public dialogs() accessor would cost a call for.
  friend int __cdecl pop_ask(char *text_file, char *text_label, int value,
                             char *override_text, int (__cdecl *callback)(),
                             int flags);
  friend int __cdecl pop_ask_number(char *label, int initial,
                                    int (__cdecl *callback)());
  friend int __cdecl pop_ask_number(char *caption, char *label, int initial,
                                    int (__cdecl *callback)());
  friend int __cdecl pop_ask_number(char *label, int (__cdecl *callback)());
  friend int __cdecl pop_ask_number(char *caption, char *label,
                                    int (__cdecl *callback)());

 public:
  // 0x006015B0, a pending_bodies forwarder.
  int init(int group_id, long flags);

 public:
  // 0x00604AB0, recovered in basepop.cpp beside its marker.
  void on_redraw();

 public:
  // STATIC, AND THE CATALOGUED NAME IS WRONG. It reads
  // `?set_def_ok_text@BasePop@@QAAHPAD@Z` - QAA, a `__cdecl` MEMBER, which
  // would push `this` as the first argument. The image reads its string from
  // [esp+8], the FIRST slot, so there is no receiver: declared as a member
  // the body reads [esp+0xc] and every argument shifts. The `symbol` fact on
  // each marker records what this tree emits instead.
  // 0x00600F00, still a pending_bodies forwarder. Declared HERE because
  // `BasePop` inherits GraphicWin privately, so an undeclared `close`
  // resolves to the inaccessible base one.
  void close();
  // 0x00601BF0, still a pending_bodies forwarder: `pops()` (popup.cpp) calls
  // it BY NAME so it emits the image's `call rel32`.
  int start(char *text_file, const char *text_label, int value,
            char *override_text, int flags, GraphicWin *owner_window);
  // 0x00602600, still a pending_bodies forwarder: the two `exec`
  // overloads below call it BY NAME so they emit the image's `E8`.
  int exec(int flags, int(__cdecl *callback)());
  // Declared MEASURED (dllexport) to match the `inline` definitions in
  // basepop.cpp: the image EXPANDS both of them into init_class (0x00604590)
  // AND keeps the standalone copies at 0x006018A0 / 0x00601910, so the /Ob1
  // inliner has to see `inline` while the dllexport keeps that standalone
  // COMDAT from being dropped once this TU's only caller folds it. game.cpp's
  // callers never see the definition, so they keep the real `call`s the image
  // has there.
  static MEASURED int __cdecl set_def_ok_text(LPSTR text);
  static MEASURED int __cdecl set_def_cancel_text(LPSTR text);
  // 0x006050F0, homed from 006050f0.cpp. `static` for the same reason
  // fallout() and on_modal() are: the catalogued `QAAX` names a __cdecl
  // MEMBER, so its hidden `this` would sit at [esp+0x18] past the five
  // real arguments - and the image reads its five from [esp+4]..[esp+0x14]
  // and never touches a sixth slot. No receiver exists to pass.
  static void set_def_external_window(GraphicWin *external_window, int loc_x,
                                      int loc_y, int width, int height);
  // 0x006043D0, homed from 006043d0.cpp. `static`, and the measurement that
  // settled it is on the marker. The catalogued `QAA` names a __cdecl
  // MEMBER, but the image's only caller (config_popups, 0x00588AEB) pushes
  // exactly ONE argument before the call and the body ends in `ret` rather
  // than `ret 4` - one stack slot, callee pops nothing. A __cdecl member
  // would have `this` at [esp+4] and the string at [esp+8]; the image reads
  // its string from [esp+4], so there is no receiver to pass. Same case as
  // set_def_ok_text above, and the `// symbol` fact records the SAH.
  // MEASURED for the same reason those two carry it: nothing in this TU
  // calls set_def_file (its one caller is config_popups, 0x00588aa0), and
  // without the dllexport VC6 drops the unreferenced COMDAT, which makes
  // the body unmeasurable rather than merely unmatched.
  static int set_def_file(char *text);

 public:
  static void fallout();
  // EVIDENCE-FREE as a name is NOT the case here - the body is one load of
  // the fallout flag - but the catalogue's QAA still names a receiver
  // nothing passes, so it is static like fallout() and set_def_ok_text.
  static int on_modal();  // 0x00604A80

  // `static`, because the receiver its catalogued name claims is a receiver
  // NOTHING PASSES AND NOTHING READS. The body at 0x00604E40 is a factory -
  // `operator new(0x3230)` then `BasePop::BasePop()` on the result - and its
  // `push ecx` at 0x00604E55 is VC6 reserving four stack bytes, overwritten
  // by `mov [esp], eax` two instructions later; every later `mov ecx, ...`
  // WRITES ecx, for the constructor call and for the SEH unlink. All seven
  // `call dword ptr [0x696ecc]` sites reach it with no ecx set up at all.
  //
  // So `QAE` in `?basepop_alloc@BasePop@@QAEHXZ` describes a `this` that does
  // not exist, and declaring it static is what lets WinMain take its ADDRESS
  // - which is the only thing WinMain does with it.
  static int init_class();          // 00604590
  static BasePop *basepop_alloc();
  void on_button_clicked(int button_id);  // 0x00604300, defined in basepop.cpp
  // 0x00604EA0, UNK2. EVIDENCE-FREE as a name: the image names it nowhere
  // and no recovered caller reaches it (catalogue callers 0, no CSV entry,
  // no UNK string in the shipped binary), so the body's own reading has to
  // carry the meaning - it measures the widest string the popup is going to
  // draw (the two captions and every item), and answers 0 when the caller
  // never opened the popup's text at all.
  int UNK2();
  int on_key_click(int a, int b);
  int on_key_up(int a) override;
  void set_string_color(int c1, int c2, int c3, int c4);
  void set_string_color2(int c1, int c2, int c3, int c4);
  void set_string_color3(int c1, int c2, int c3, int c4);
  void set_string_color_hyper(int c1, int c2, int c3, int c4);
  // The FIRST parameter is a `char` - `D` in
  // ?set_button_color@BasePop@@QAEXDHHH@Z - and the bytes agree: 0x00604830
  // is `mov al, byte ptr [esp+4]` into `mov byte ptr [ecx+0x3178], al`,
  // while parameters 2-4 are dword loads into dword fields. `char`, not
  // `signed char`: they are distinct MSVC types mangling D and C.
  void set_button_color(char c1, int c2, int c3, int c4);
  void set_button_color2(char c1, int c2, int c3, int c4);
  void set_button_color3(char c1, int c2, int c3, int c4);
  static void set_def_string_color(int c1, int c2, int c3, int c4);
  static void set_def_string_color2(int c1, int c2, int c3, int c4);
  static void set_def_string_color3(int c1, int c2, int c3, int c4);
  static void set_def_string_color_hyper(int c1, int c2, int c3, int c4);
  static void set_def_button_color(int c1, int c2, int c3, int c4);
  static void set_def_button_color2(int c1, int c2, int c3, int c4);
  static void set_def_button_color3(int c1, int c2, int c3, int c4);
  // Static defaults shared by every popup; __cdecl in the original because
  // they take no instance.
  static int set_def_string_font(Font *font1, Font *font2, Font *font3,
                                 Font *font4);
  uint32_t read_check();
  int item(char *text, int index);
  int add_asterisk_item();
  int set_button_font(Font *font1, Font *font2, Font *font3);
  static int set_def_button_font(Font *font1, Font *font2, Font *font3);
  // DECLARED, NOT DEFINED INLINE, and that is what lets `basepop_alloc` be
  // recovered. Its body is `return (int)new BasePop();` and the image calls
  // `??0BasePop@@QAE@XZ` at 0x00604E75; an empty inline constructor is
  // inlined away, the call never appears, and every instruction after it
  // shifts. Measured on that body with this line as the only difference:
  //
  //     BasePop();        BYTE_EXACT
  //     BasePop() { ; }   MISMATCH at #1
  //
  // src/*.h has 145 empty inline constructors and destructors across 75
  // classes, so the same question is waiting on most of them.
  BasePop();
  // DEFINED IN-CLASS, the ~FlatButton move, because the image INLINES this
  // destructor at the popup's destruction sites and CALLS it out of line 75
  // times - and only an in-class definition lets both happen.
  // ??1BasePop@@QAE@XZ (0x004064D0, claimed in basepop.cpp) keeps being
  // emitted out-of-line by the MEASURED storage class; the expansion is what
  // Caviar::init_class (0x006185A0) lays down at each of its early returns
  // and what sound_version_warning (0x004C9080) lays down at its one scope
  // exit. Out-of-line here those sites can only emit `call ??1BasePop`, and
  // init_class measured 16/392 with those 200 instructions of teardown
  // MISSING outright rather than divergent.
  //
  // The body is `close()` and nothing else: the rest of the image's body -
  // Spot at +0x3098, the Dialogs slab at +0x21d0, unit_ids_ and item_ids_,
  // sprite_, both FlatButtons, heap_ and the GraphicWin base, in exactly
  // that reverse-declaration order - is the compiler's, generated from the
  // member declarations below, the same way ??1DipEdit is generated from
  // dipedit.h. An empty body produces the identical chain; the one call the
  // member list cannot produce is the image's opening `call ?close`.
  MEASURED ~BasePop() { close(); }
  void set_loc(int x, int y);
  // NetMsg preserves the original local-popup auto-hide value at +0x30AC.
  void set_auto_hide_value(uint32_t value) { field_30AC_ = value; }
  void set_width(int width);
  // 0x00603800, homed from 00603800.cpp beside its marker. Lays the popup's
  // text out and hands the result to Dialogs::calculate_dimensions twice.
  // Original primary table 0x6698D4 extends GraphicWin slots 0..89 with
  // these seven callbacks, in this order. Popup table 0x6695C8 overrides
  // slots 90 and 92..96; exec/layout/redraw dispatch through this hierarchy.
  virtual int calculate_dimensions();
  // Provisional role names for the shared defaults; their native table
  // entries and call sites establish the signatures, not catalogue names.
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_pre_window_init@BasePop@@UAEXXZ 0x00602600
  virtual void on_pre_window_init() {}
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_post_window_init@BasePop@@UAEXXZ 0x00602600
  virtual void on_post_window_init() {}
  // RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_adjust_button_width@BasePop@@UAEXXZ 0x00603800
  virtual void on_adjust_button_width() {}
  // Primary original 0x00406B20 is bound in basepop.cpp to this definition.
  // Other classes retain their independently bound folded bodies.
  virtual void on_adjust_button_placement(int *, int *, int *) {}
  // Primary original 0x00406840 is bound in basepop.cpp to this definition.
  // Its BasePop table/lifecycle evidence anchor remains 0x00602600.
  virtual int on_exec() { return 1; }
  // RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?on_draw_background@BasePop@@UAEHXZ 0x00604AB0
  virtual int on_draw_background() { return 0; }
  // Native487DFD reads Popup+0xA48: its script-owned caption text.
  const char *caption_text() const { return owned_text_; }
  void write_check(long value);
  int exec();
  int exec(int (__cdecl *callback)());
  int set_string_font(Font *font1, Font *font2, Font *font3, Font *font4);

 private:
  // THE EXTERNAL WINDOW THE POPUP DRAWS ITS DIALOG INTO, set by
  // set_external_window (0x00605010, whose first parameter is the
  // GraphicWin * stored here) and dispatched through by double_click's
  // vtable call. A pointer, not the uint32_t it was typed before: the
  // image calls its vtable slot 0xe8 with this slot as the receiver.
  // Both setter APIs receive GraphicWin*, retained directly or through the
  // default-layout slot; the observed Diplopop caller supplies that base.
  // GraphicWin's primary Win view supplies modal and visibility dispatch.
  GraphicWin *external_window_;  // 0x0a14
  // The STARTING loc_a_/loc_b_ pair: copied over them unless the value is
  // 0x2000, the keep-current sentinel set_loc uses too.
  uint32_t default_loc_a_;  // 0x0a18
  uint32_t default_loc_b_;  // 0x0a1c
  // The DEFAULT dialog width/height, seeded from the file-backed
  // 0x00696EC4 table; the ctor's tail writes them into the embedded
  // Dialog's width/height slots (the height grown 3/2 for wide screens).
  uint32_t dialog_width_;   // 0x0a20
  uint32_t dialog_height_;  // 0x0a24
  Heap heap_;
  // THE WINDOW THE POPUP WAS OPENED FOR. Two writers, both windows: the ctor
  // stores `this` (image `mov [ebp+0xa3c], ebp` at 0x00600ab0) and
  // BasePop::start (0x00601bf0) stores its sixth argument, the
  // `GraphicWin *owner_window` that pops() passes as 0 - so the slot ends up
  // holding whichever window the popup belongs to, defaulting to itself.
  // Nothing in the tree read it back until on_redraw (0x00604AB0) did, and
  // that body is what types it: it takes the OWNER's Buffer (+0x444) for every
  // text call, and compares the slot against `this` to decide whether the
  // popup draws into itself or into the window that opened it - so the type is
  // the one whose +0x444 is a Buffer, which is GraphicWin and not Win. Same
  // type start's own sixth parameter already carries; the store compiles to
  // the identical `mov`.
  GraphicWin *owner_window_;  // 0x0a3c
  // THE SELECTED ENTRY ID. Three writers agree: close() and init() zero it,
  // and both on_key_down (0x00604410) and double_click (0x006044d0) land the
  // embedded Dialog's selected id in it - `get_selected_id()` for the whole
  // Dialogs, `Dialog::selected_position_` when the kind is 1.
  int selected_id_;  // 0x0a40
  // THE POPUP'S VALUE. start (0x00601bf0) stores its third argument here and
  // its config reader overwrites it with `atoi` of a directive's number;
  // close() resets it to -1. popup.h's gloss for pops() calls that argument
  // the value.
  int value_;  // 0x0a44
  // ONE OWNED HEAP STRING, installed by start's config reader: freed, then
  // `mem_get(len + 1)` and a `strcat` of the directive's text. close() frees
  // it and nulls the slot. Nothing reads it back in the tree yet.
  char *owned_text_;  // 0x0a48
  uint32_t field_A4C_;
  uint32_t field_A50_;
  uint32_t field_A54_;
  uint32_t field_A58_;
  FlatButton flat_button1_;
  FlatButton flat_button2_;
  uint32_t field_20F4_;
  // EIGHT ZEROED DWORDS at 0x20F8, ONE writer and ONE meaning: the ctor's
  // `lea edi,[ebp+0x20f8]; mov ecx,8; xor eax,eax` is a single `rep stosd`
  // (0x006018xx), so this is a block the class clears, not eight separate
  // flags. Typed 2026-09-04; the eight offset names it replaces said nothing
  // the memset does not.
  // Original exec allocates FlatButton at 0x0060303B/0x00603051 and stores
  // the result into this run at 0x0060305A. Close walks eight slots at
  // 0x00601187, invokes virtual scalar deletion with flag 1 at 0x0060119C,
  // then clears each slot. The earlier zero_block_ spelling hid ownership.
  FlatButton *item_buttons_[8];  // 0x20F8
  Sprite sprite_;
  // THE CALLER'S SPRITE, or 0. pops() (popup.cpp) stores its `Sprite *`
  // argument here when the popup's own item list is empty and 0 when it is
  // not, so the popup draws the caller's picture instead of its own. A
  // pointer, not the uint32_t it was typed before: it IS a Sprite *.
  Sprite *caller_sprite_;  // 0x2144
  // THE POPUP'S OWN SECOND PICTURE, typed 2026-09-05 from on_redraw
  // (0x00604AB0), which is the only reader: it is the sprite `Buffer::tile`
  // is handed to fill the popup's background with, ahead of the same body's
  // draw of `field_214C_`. Seeded from BasePopDefaultField2148[0] by the ctor
  // and close() exactly as `field_214C_` below is from [1].
  Sprite *field_2148_;  // 0x2148
  // THE BACKGROUND SPRITE, the partner of field_2148_ above. on_redraw draws
  // it into the popup's buffer when the popup has no external window, and
  // reads its iSpriteWidth_/iSpriteHeight_/iWidth_/iHeight_ to centre it -
  // four Sprite fields, so a Sprite * and not the uint32_t it was typed
  // before. Seeded from BasePopDefaultField2148[1].
  Sprite *field_214C_;  // 0x214c
  // The popup's ITEM-TEXT list, carved 2026-08-30 beside unit_ids_ below on
  // the identical evidence: 0x30 = sizeof(StringList), and the ctor's stores
  // for this block are StringList's two-stage construction (the same
  // 0x0066B0EC vbtable and staged vtables, +0x006693AC virtual-base vtable),
  // which the compiler emits from the declaration. BasePop::start adds the
  // popup's per-item strings here (0x006021A1) as it adds the unit ids to
  // unit_ids_ (0x006024F4).
  StringList item_ids_;  // 0x2150
  public:
  // THE POPUP'S UNIT-NAME LIST, carved 2026-08-30 from what was the twelve
  // dwords field_2180_..field_21AC_: exactly 0x30 = sizeof(StringList), and
  // the image's own ctor writes are StringList's construction, not raw
  // stores - the vbtable (0x0066B0EC) at +0x2184, the StringAllocationBase
  // vftable
  // (0x006693AC) at +0x21A8, the allocator hand-off capture into +0x21AC,
  // and the StringStruct zeroing. The old hand-spelled copies of those
  // stores are deleted from the ctor; the compiler now emits them from this
  // declaration, as it already does for StringBox's stringList_ at 0x2B70.
  // PUBLIC because Caviar::init_class flattens it into StringTemp for the
  // unsupported-CPU popup, and access specifiers do not change layout.
  StringList unit_ids_;  // 0x2180

  private:
  uint32_t field_21B0_;
  uint32_t field_21B4_;
  uint32_t field_21B8_;
  uint32_t field_21BC_;
  uint32_t field_21C0_;
  uint32_t field_21C4_;
  uint32_t field_21C8_;
  uint32_t field_21CC_;
  // Split so `field_2274_` - the flag `pops()` (popup.cpp) tests, right
  // after the SpriteBox call below, to decide whether a caller-supplied
  // sprite replaces the popup's own - has a name.
  // THE EMBEDDED DIALOGS, real now: the three spans it replaces
  // (dialogs_[0xA4] + field_2274_ + dialogs_tail_[0xBEC]) total exactly
  // sizeof(Dialogs) = 0xC94 at 0x21D0..0x2E64. Its construction is the
  // compiler's (the image's `push 1; call 0x612830` mid-body), and the
  // CheckBox at 0x2228 (+0x58), SpriteBox at 0x2240 (+0x70) and EditGroup at
  // 0x22C8 (+0xF8) are its own base subobjects - see dialogs() below.
  Dialogs dialogs_;  // 0x21D0

 public:
  // THE POPUP'S SUBOBJECTS ARE REACHED THROUGH THE REAL MEMBER `dialogs_`,
  // and this returns it - the same type, so no conversion. The three former
  // offset accessors (check_box 0x2228, sprite_box 0x2240, edit_group
  // 0x22C8) are GONE: each was `reinterpret_cast<Subclass *>(
  // reinterpret_cast<uint8_t *>(this) + OFF)`, a self-pun re-deriving by hand
  // what `dialogs_`'s own base list already states - CheckBox is at
  // 0x21D0+0x58, SpriteBox at +0x70, EditGroup at +0xF8 (dialogs.h).
  //
  // THEY CANNOT COME BACK AS TYPED SUBOBJECT ACCESSORS, measured 2026-09-04
  // against 0x00601BB0 (image 5 instructions):
  //
  //   dialogs_.set_state_flag(value)        BYTE_EXACT 5/5
  //   CheckBox *p = &dialogs_; p->...       MISMATCH 0/5, compiled 13
  //   CheckBox &r = dialogs_; r....         MISMATCH 0/5, compiled 13
  //   check_box() -> CheckBox &, r....      MISMATCH 0/5, compiled 13
  //
  // Binding a base SUBOBJECT to a pointer or reference makes VC6 emit its
  // adjustor's null guard - `test eax,eax / je` with an `xor ecx,ecx` on the
  // skipped arm - and the two constants stop folding: `lea eax,[ecx+0x21d0]`
  // then `+0x58` never becomes the image's one `add ecx, 0x2228`. Calling the
  // INHERITED method straight on the member names no pointer at all, so the
  // compiler folds the base offset into the receiver itself. Same-type
  // `dialogs()` costs nothing; `pop->dialogs().sprite(...)`,
  // `popup.dialogs().edit(...)` and `dialogs_.set_state_flag(...)` are the
  // three former call sites.
  Dialogs &dialogs() {
    return dialogs_;
  }
  // Application pickers sort this same owned list before executing the
  // dialog. Keep the original StringList virtual operation and receiver.
  void sort_item_ids(int mode) { item_ids_.sort(mode); }
  // Calculated popup dimensions at original +0x30BC/+0x30C0. BasePop uses
  // these for GraphicWin::init; tutorial placement reads the same values.
  // Return the stored type so existing signed casts and unsigned additions
  // at each call site retain their original behavior.
  uint32_t layout_width() const { return field_30BC_; }
  uint32_t layout_height() const { return field_30C0_; }
  // Satellite target selection initializes the owned picture, then selects
  // that same member only after Sprite::init succeeds (4A067F..4A0694).
  Sprite &owned_picture() { return sprite_; }
  // Native popb configures this owned SpriteBox, then its two state fields.
  void configure_sprite_dialog(Sprite *picture) {
    dialogs_.sprite(picture, 0, 0);
    field_3104_ = 2;
    field_3108_ = 1;
  }
  void select_owned_picture() { caller_sprite_ = &sprite_; }
  bool picture_enabled() const { return dialogs_.SpriteBox::list_.entry_count_ != 0; }
  void set_caller_picture(Sprite *picture) { caller_sprite_ = picture; }
  void set_redraw_callback(SpriteBox::DrawCallback callback) { redraw_callback_ = callback; }
  void set_sprite_click_callback(SpriteBox::ClickCallback callback) {
    sprite_click_callback_ = callback;
  }
  void set_sprite_double_click_callback(SpriteBox::ClickCallback callback) {
    sprite_double_click_callback_ = callback;
  }
  // Modal editor pickers set their default answer after start configures
  // the popup. This is the value word at original +0xA44.
  void set_value(int value) { value_ = value; }
  int value() const { return value_; }

 private:
  // THE TWO 0x118 TEXT BLOCKS at 0x2E64 and 0x2F7C, typed 2026-09-04. These
  // 140 dwords used to be spelled individually; the runs below replace them.
  // Evidence (all of it from the two bodies that reach the region, neither yet
  // recovered - src/unrecovered/00601bf0.cpp and 00600f00.cpp):
  //
  //   * BasePop::start (0x00601BF0) walks BOTH blocks with `idx * 0x118`,
  //     writing a STRING at block +0 (`*dst = 0; strcat(dst, v)`) and numbers
  //     at block +0x104, +0x108, +0x10C, +0x110 and +0x114.
  //   * BasePop::close (0x00600F00) restores both from the file-backed
  //     defaults 70 dwords at a time - the same 0x118 the ctor memcpy's.
  //   * BasePopDefaultBlock2E64 / BasePopDefaultBlock2F7C (basepop.cpp) are
  //     the 0x118 defaults, and member-accesses.csv records nothing else in
  //     the image touching either block.
  //
  // `text` is 0x104 because the first number sits at +0x104; nothing in the
  // image bounds what strcat may write into it, so the buffer is the size the
  // next field forces rather than a capacity the code checks. The five
  // numbers stay offset-named: start() parses each from its own token with
  // atoi/atol. The sound callback interprets the first two; the remaining
  // fields retain offset names until their consumers establish meanings.
  BasePopTextBlock text_block_a_;  // 0x2E64
  BasePopTextBlock text_block_b_;  // 0x2F7C
  // THE PER-POPUP CONFIG CALLBACK, typed from its only caller. BasePop::exec
  // (0x00602600) reads the slot twice, tests it against null and calls it
  // straight (`call eax`, 0x0060369C and 0x00603787) with a text block and a
  // flag, cleaning eight bytes afterwards. FX::init supplies the void
  // popup_wave_callback; neither call consumes a return value.
  BasePopConfigCallback field_3094_;  // 0x3094
  Spot spot_;
  uint32_t field_30A4_;
  uint32_t field_30A8_;
  uint32_t field_30AC_;
  uint32_t field_30B0_;
  uint32_t field_30B4_;
  uint32_t field_30B8_;
  uint32_t field_30BC_;
  uint32_t field_30C0_;
  uint32_t field_30C4_;
  uint32_t field_30C8_;
  uint32_t field_30CC_;
  uint32_t field_30D0_;
  uint32_t field_30D4_;
  uint32_t field_30D8_;
  uint32_t field_30DC_;
  uint32_t field_30E0_;
  uint32_t field_30E4_;
  uint32_t field_30E8_;
  uint32_t field_30EC_;
  uint32_t field_30F0_;
  uint32_t field_30F4_;
  uint32_t loc_a_;
  uint32_t loc_b_;
  uint32_t field_3100_;
  uint32_t field_3104_;
  uint32_t field_3108_;
  uint32_t field_310C_;
  // THE FOUR STRING FONTS, 0x3110 + 4*n: close() and init() seed them from
  // BasePopDefaultStringFonts[n] and set_string_font installs a caller's four.
  Font *string_font1_;  // 0x3110
  Font *string_font2_;  // 0x3114
  Font *string_font3_;  // 0x3118
  Font *string_font4_;  // 0x311c
  // THE FOUR COLOUR SLOTS x FOUR TIERS, 0x3120 + 0x10*slot + 4*tier: slot is
  // the a/b/c/d below (0x3120, 0x3130, 0x3140, 0x3150) and the tier stride is
  // 4, which is the same [slot][tier] shape as BasePopDefaultStringColors -
  // close() and init() copy the table straight across, one dword at a time.
  int string_color_a_;         // 0x3120 = StringColors[0][0]
  int string_color_2a_;        // 0x3124 = StringColors[0][1]
  int string_color_3a_;        // 0x3128 = StringColors[0][2]
  int string_color_hyper_a_;   // 0x312c = StringColors[0][3]
  int string_color_b_;         // 0x3130 = StringColors[1][0]
  int string_color_2b_;        // 0x3134 = StringColors[1][1]
  int string_color_3b_;        // 0x3138 = StringColors[1][2]
  int string_color_hyper_b_;   // 0x313c = StringColors[1][3]
  int string_color_c_;         // 0x3140 = StringColors[2][0]
  int string_color_2c_;        // 0x3144 = StringColors[2][1]
  int string_color_3c_;        // 0x3148 = StringColors[2][2]
  int string_color_hyperc_;    // 0x314c = StringColors[2][3]
  int string_color_d_;         // 0x3150 = StringColors[3][0]
  int string_color_2d_;        // 0x3154 = StringColors[3][1]
  int string_color_3d_;        // 0x3158 = StringColors[3][2]
  int string_color_hyper_d_;   // 0x315c = StringColors[3][3]
  // veh_pick installs an int(int) cdecl item callback here; dimensions
  // transfers it to SpriteBox+0x68, whose left handler consumes its result.
  // RECOVERY_MEMBER: v1 BasePop sprite_click_callback_ 0x3160 0x0048D0D0
  SpriteBox::ClickCallback sprite_click_callback_;  // 0x3160
  // calculate_dimensions installs this into the embedded SpriteBox's
  // double-click callback; that handler tests its int return value.
  // RECOVERY_MEMBER: v1 BasePop sprite_double_click_callback_ 0x3164 0x00603800
  SpriteBox::ClickCallback sprite_double_click_callback_;  // 0x3164
  // 0x3168. THE POPUP'S DRAW CALLBACK, typed from its one reader. on_redraw
  // (0x00604AB0) calls it through the slot with five arguments - the caller
  // sprite, the owner window, the SpriteBox's position for item 0, and the
  // popup's right/bottom - and skips it when the slot is null, drawing the
  // caller sprite itself instead. `__cdecl`, because the image's `add
  // esp, 0x14` after the indirect call is the caller cleaning five stack
  // slots. The ctor zeroes it.
  SpriteBox::DrawCallback redraw_callback_;  // 0x3168
  // THE THREE BUTTON FONTS, 0x316c + 4*n, seeded from
  // BasePopDefaultButtonFonts[n] the same way.
  Font *button_font1_;  // 0x316c
  Font *button_font2_;  // 0x3170
  Font *button_font3_;  // 0x3174
  // THE BUTTON COLOUR SLOTS. Slot a is FOUR BYTES at 0x3178 (its tiers are
  // the bytes 0x3178/0x3179/0x317a and the spare 0x317b), slots b/c/d are
  // dwords at 0x317c/0x3188/0x3194 - the same four slots x four tiers as the
  // string block, at the button table's 0xC tier stride. close() and init()
  // copy BasePopDefaultButtonColors across: slot a from the table's low
  // bytes, tiers 1-3 as whole dwords.
  uint8_t button_color_a_;    // 0x3178 = ButtonColors[0][0]
  uint8_t button_color_2a_;   // 0x3179 = ButtonColors[0][1]
  uint8_t button_color_3a_;   // 0x317a = ButtonColors[0][2]
  uint8_t field_317B_;        // 0x317b, the fourth byte of slot a
  uint32_t button_color_b_;   // 0x317c = ButtonColors[1][0]
  uint32_t button_color_2b_;  // 0x3180 = ButtonColors[1][1]
  uint32_t button_color_3b_;  // 0x3184 = ButtonColors[1][2]
  uint32_t button_color_c_;   // 0x3188 = ButtonColors[2][0]
  uint32_t button_color_2c_;  // 0x318c = ButtonColors[2][1]
  uint32_t button_color_3c_;  // 0x3190 = ButtonColors[2][2]
  uint32_t button_color_d_;   // 0x3194 = ButtonColors[3][0]
  uint32_t button_color_2d_;  // 0x3198 = ButtonColors[3][1]
  uint32_t button_color_3d_;  // 0x319c = ButtonColors[3][2]
  // THE 0x31A0 ROW, from BasePopDefaultField31A0 = {247, 7, 248, 2} at
  // 0x00696F54: the first dword's low byte lands at 0x31a0 and the other
  // three dwords at 0x31a4/0x31a8/0x31ac. The three bytes 0x31a1..0x31a3 are
  // never written, so they stay zero from the ctor.
  uint8_t field_31A0_;   // 0x31a0 = (uint8_t)Field31A0[0] = 247
  uint8_t field_31A1_;   // 0x31a1
  uint8_t field_31A2_;   // 0x31a2
  uint8_t field_31A3_;   // 0x31a3
  uint32_t field_31A4_;  // 0x31a4 = Field31A0[1] = 7
  uint32_t field_31A8_;  // 0x31a8 = Field31A0[2] = 248
  uint32_t field_31AC_;  // 0x31ac = Field31A0[3] = 2
  uint32_t field_31B0_;  // 0x31b0 = BasePopDefaultField31B0[0]
  uint32_t field_31B4_;  // 0x31b4 = BasePopDefaultField31B0[1]
  // Nine groups of three borrowed Sprite pointers. Constructor/close copy
  // matching slots from the defaults; apply_button_config publishes all27
  // slots to FlatButton, whose redraw supplies the pointer consumers.
  Sprite *default_block_[27];  // 0x31B8..0x3224
  LPSTR ok_text_;
  LPSTR cancel_text_;
  // THE FALLBACK PARENT WINDOW. Zeroed by the ctor and republished from
  // BasePopDefaultField322C, which close_opening fills with the Console's
  // GraphicWin virtual base - so the slot holds a window pointer at run time,
  // and exec (0x00602600) reads it as the 0x5D4EF0 init call's seventh
  // argument, whose type is `Win *`. Same typing argument field_A3C_ above
  // carries; the four bytes do not move.
  Win *field_322C_;
  // THE TWO BUTTON-ROW HELPERS exec (0x00602600) runs at its three button
  // sites, DEFINED IN-CLASS because the image has all 33 stores inline at all
  // three - an out-of-line definition turns each site into a call the image
  // does not make. Both now name their runs through the headers: FlatButton's
  // public config[27] over 0xAE0..0xB48 and BaseButton's public colour/bevel
  // run over 0xA84..0xA9C, so the two `reinterpret_cast` walks this used to
  // carry are gone.
  void apply_button_config(FlatButton *button) {
    button->config[0] = default_block_[0];
    button->config[1] = default_block_[1];
    button->config[2] = default_block_[2];
    button->config[3] = default_block_[3];
    button->config[4] = default_block_[4];
    button->config[5] = default_block_[5];
    button->config[6] = default_block_[6];
    button->config[7] = default_block_[7];
    button->config[8] = default_block_[8];
    button->config[9] = default_block_[9];
    button->config[10] = default_block_[10];
    button->config[11] = default_block_[11];
    button->config[12] = default_block_[12];
    button->config[13] = default_block_[13];
    button->config[14] = default_block_[14];
    button->config[15] = default_block_[15];
    button->config[16] = default_block_[16];
    button->config[17] = default_block_[17];
    button->config[18] = default_block_[18];
    button->config[19] = default_block_[19];
    button->config[20] = default_block_[20];
    button->config[21] = default_block_[21];
    button->config[22] = default_block_[22];
    button->config[23] = default_block_[23];
    button->config[24] = default_block_[24];
    button->config[25] = default_block_[25];
    button->config[26] = default_block_[26];
  }
  void apply_button_bevel(FlatButton *button) {
    button->color_ = 0;
    button->bevel_upper_ = field_31A4_;
    button->bevel_lower_ = field_31A8_;
    button->bevel_thickness_ = field_31AC_;
    button->field_A94_ = field_31B0_;
    button->field_A98_ = field_31B4_;
  }
};



// The screen width the high-resolution scaling switches on, read at a
// fixed address. Rebindable so tests can drive both sides of the branch.


static_assert(sizeof(BasePop) == 0x3230,
              "BasePop layout must match the original executable");



// Default string and button font slots at 0x009B8D98 and 0x009B8DA8; tests
// outside the hybrid process rebind them.
// ARRAYS, NOT POINTERS. Four string fonts at 0x009B8D98 and three button
// fonts at 0x009B8DA8; the image stores straight into them.
extern Font *BasePopDefaultStringFonts[4];  // 0x009B8D98
extern Font *BasePopDefaultButtonFonts[3];  // 0x009B8DA8
extern Sprite *BasePopDefaultBlock31B8[27];
extern uint32_t BasePopDefaultField21C0[2];
extern uint32_t BasePopDefaultField3104[3];
extern uint32_t BasePopDefaultField31A0[4];
extern uint32_t BasePopDefaultField21B0[4];
extern uint32_t BasePopDefaultField31B0[2];
extern Sprite *BasePopDefaultField2148[2];
extern uint32_t BasePopDefaultField30A4;


// Two interleaved default-colour tables. The string table carries four tiers
// at a 0x10 slot stride; the button table carries three at 0xC. Tests rebind
// both.
extern uint32_t BasePopDefaultStringColors[4][4];  // 0x00696EE4
extern uint32_t BasePopDefaultButtonColors[4][3];  // 0x00696F24



// Set when the fallout flag is raised, but only while the gate global is
// set. Real storage, defined in basepop.cpp; in the image these are the two
// zero-initialised dwords at 0x009B8D00 and 0x009B8CFC.
extern int BasePopFalloutGate;  // 0x009B8D00
extern int BasePopFalloutFlag;  // 0x009B8CFC

// The two singleton BasePop instances `init_class` allocates (basepop.cpp,
// which defines them) and `pops` (popup.cpp) chooses between: in the image
// these are the two zero-initialised pointer dwords at 0x009BC074/78. Real
// BasePop pointer variables here, matching PopupAllocHook's return type -
// one storage, one name - instead of the per-TU
// pointer bindings into the image's data that read and wrote unmapped
// memory in the standalone build.
extern BasePop *PopupInstanceSlotA;  // 0x009BC074
extern BasePop *PopupInstanceSlotB;  // 0x009BC078

// The two default button captions, heap copies the class owns. `init_class`
// seeds them from the literals "OK" and "Cancel"; the two setters below
// replace them.
extern char *BasePopDefaultOkText;      // 0x009B8D80
extern char *BasePopDefaultCancelText;  // 0x009B8D84

// Defined in basepop.cpp (0x009B8DD0, "copied to +0x322c" of the popup
// defaults). close_opening publishes the address of the Console's GraphicWin
// virtual base here on the way out of the opening, which is why the slot
// holds a window pointer at run time despite the popup-defaults name it
// carried before that call was recovered.
extern Win *BasePopDefaultField322C;  // 0x009B8DD0

// The shipped value of the dword immediately below the class-defaults table
// (0x00696EC4): -1. BasePop::exec (0x00602600) reads it as its answer when
// the modal pump reported a dismissal and the popup is not lockable - so the
// table's own default is "answer -1", the same value the early-return paths
// spell literally.
extern const uint32_t BasePopDefaultExecResult;  // 0x00696EC0

// Where the last modal popup's client area started. exec (0x00602600,
// 0x00603724) publishes both from this->client_rect_ after the pump returns
// and the modal focus has been handed back; nothing in the tree reads either
// yet, so the names say what they hold and no more. Real storage, defined in
// basepop.cpp; in the image these are the two zero-initialised dwords at
// 0x009B8DC4 / 0x009B8DC8.
extern int BasePopModalLeft;  // 0x009B8DC4
extern int BasePopModalTop;   // 0x009B8DC8
