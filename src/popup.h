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
#include "basepop.h"
#include "graphicwin.h"
#include "scroll.h"

 /*
  * Popup class
  *
  * Derives from BasePop on an unadjusted `this`, and holds a Scroll at 0x3230
  * - exactly sizeof(BasePop), so the member follows the base with nothing
  * between. Both are pinned here, at 0x3230 and 0x214C, so this much of the
  * layout is exact; where the object ends is not established.
  */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
class Popup : public BasePop {
 public:
  // 0x00404A80, a pending_bodies forwarder.
  void hide();

 public:
  int on_nc_hittest(int a1, int a2);

  // `static`, because the image's name ends in `QAA` - a public member
  // declared __cdecl, taking no receiver. WinMain stores its ADDRESS into the
  // popup allocator hook and never calls it from there, and it has to be a
  // real symbol for that store to be the `mov eax, OFFSET` the image emits
  // rather than a load of a constant a header made up.
  static int alloc();

  Popup();
  // DECLARED, NOT DEFINED EMPTY. The real destructor is 0x00406C00 and ends
  // `ret`, not `ret 4`, so it takes no vbase flag and a direct call reaches
  // it. An empty inline body compiles `??_GPopup`'s call away. Forwarded in
  // `pending_bodies.cpp`.
  ~Popup();
  void close();
  void on_adjust_button_width();
  // 0x00406380, still a pending_bodies forwarder: the three overloads
  // below call it BY NAME so they emit the image's `call rel32`.
  void start(char *a1, const char *a2, int a3, char *a4, int a5,
             GraphicWin *owner);
  void start(char *a1, const char *a2, int a3, char *a4, int a5);
  void start(const char *label);
  void start(const char *label, int value);
  int on_dialog_back_draw(::GraphicWin *window);
  // PAURECT@@H: the first parameter is a RECT *, per
  // ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z. The body is `ret 8` either way,
  // so only the decorated name moves - toward the one the catalogue states.
  void on_redraw_nc(RECT *, int);

 private:
  Scroll scroll_;
};

// Where the object ends IS established, by the class that derives from it.
// ??0TutWin@@QAE@XZ at 0x004BA6B0 builds this Popup on an unadjusted `this`
// and then writes TutWin's own fields; the lowest of those is 0x537C
// (`mov dword ptr [esi + 0x537c], ecx` at 0x004BA6F1), and below it the
// constructor stores nothing but the two GraphicWin vptrs at 0x0 and 0x444.
// So the Popup base subobject spans exactly [0, 0x537C) - which is also
// sizeof(BasePop) 0x3230 plus sizeof(Scroll) 0x214C, both pinned and both
// `reached` by verify_member_offsets.py --pins, with scroll_ at 0x3230
// confirmed by ?close@Scroll@@QAEXXZ on this+0x3230 at 0x004BE82E. Nothing
// in this chain is a virtual base (BasePop : GraphicWin : public Win are all
// non-virtual), and MSVC never reuses tail padding, so the subobject's extent
// is sizeof.
//
// Popup's OWN methods do not reach that far - verify_member_offsets.py --pins
// prints `short Popup 0x537C image reaches 0x4C70` - so this pin rests on the
// derived class and on the BasePop + Scroll sum, not on Popup's own accesses.
// `short` means the image reaches less than the pin, which does not refute it;
// it means Popup's own code never touches its last 0x70C bytes.
static_assert(sizeof(Popup) == 0x537C,
              "Popup layout must match the original executable");

void __fastcall popup_on_adjust_button_width_redirect(Popup *self, void *);
void __fastcall popup_on_redraw_nc_redirect(
    Popup *self, void *, RECT *a1, int a2);

// BasePop::close is not recovered yet.

void __fastcall popup_close_redirect(Popup *self, void *);

// The six-argument Popup::start is not recovered; the five-argument form
// forwards to it with a null final GraphicWin argument.

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5);

// The two short start forms share the caption buffer at a fixed address.
extern char PopupStartCaption[256];

void __fastcall popup_start_label_redirect(Popup *self, void *, const char *label);
void __fastcall popup_start_label_value_redirect(Popup *self, void *,
                                                 const char *label, int value);
int __fastcall popup_on_dialog_back_draw_redirect(Popup *self, void *, ::GraphicWin *window);

/*
 * pop/pops forwarder family
 *
 * The nine-argument popup builder pops() (caption, label, value, override
 * text, title, sprite, two flags, callback) is the common target every
 * forwarder below reduces to. Each defaults a different subset of those
 * nine arguments to whatever its own, shorter argument list does not
 * expose. The pop_* family always passes a null sprite; the pops_* family
 * always forwards its own sprite argument. Note both caption and label are
 * plain char* here (PAD in the mangled names), unlike the const char*
 * label used by the XPopsOriginalFull family in xpops.h.
 */
// 0x006276A0, still a pending_bodies forwarder. Declared as a name rather
// than reached through `func_pops_full *`, which compiled `call [ptr]` at
// every one of the `pop` wrappers below.
int __cdecl pops(char *caption, char *label, int a3, char *a4, int a5,
                 Sprite *sprite, int a7, int a8, int (__cdecl *callback)());

int __cdecl pop_label_cb(char *label, int (__cdecl *callback)());
int __cdecl pop_caption(char *caption, char *label,
                                  int (__cdecl *callback)());
int __cdecl pop_value_title(char *label, int value, int title,
                                      int (__cdecl *callback)());
int __cdecl pop_caption_value_title(char *caption, char *label,
                                              int value, int title,
                                              int (__cdecl *callback)());
int __cdecl pop_title(char *label, int title,
                                int (__cdecl *callback)());
int __cdecl pop_caption_title(char *caption, char *label, int title,
                                        int (__cdecl *callback)());
int __cdecl pop_value_text_title(char *label, int value,
                                           char *text, int title,
                                           int (__cdecl *callback)());
int __cdecl pop_full(char *caption, char *label, int value,
                               char *text, int title,
                               int (__cdecl *callback)());

int __cdecl pops_minimal(char *label, Sprite *sprite,
                                   int (__cdecl *callback)());
int __cdecl pops_flags(char *label, Sprite *sprite, int flag_a,
                                 int flag_b, int (__cdecl *callback)());
int __cdecl pops_caption(char *caption, char *label,
                                   Sprite *sprite, int (__cdecl *callback)());
int __cdecl pops_caption_flags(char *caption, char *label,
                                         Sprite *sprite, int flag_a,
                                         int flag_b,
                                         int (__cdecl *callback)());
int __cdecl pops_value_title(char *label, int value, int title,
                                       Sprite *sprite,
                                       int (__cdecl *callback)());
int __cdecl pops_value_title_flags(char *label, int value,
                                             int title, Sprite *sprite,
                                             int flag_a, int flag_b,
                                             int (__cdecl *callback)());
int __cdecl pops_caption_value_title(char *caption, char *label,
                                               int value, int title,
                                               Sprite *sprite,
                                               int (__cdecl *callback)());
int __cdecl pops_no_text(char *caption, char *label, int value,
                                   int title, Sprite *sprite, int flag_a,
                                   int flag_b, int (__cdecl *callback)());
int __cdecl pops_title(char *label, int title, Sprite *sprite,
                                 int (__cdecl *callback)());
int __cdecl pops_title_flags(char *label, int title,
                                       Sprite *sprite, int flag_a,
                                       int flag_b,
                                       int (__cdecl *callback)());
int __cdecl pops_caption_title(char *caption, char *label,
                                         int title, Sprite *sprite,
                                         int (__cdecl *callback)());
int __cdecl pops_caption_title_flags(char *caption, char *label,
                                               int title, Sprite *sprite,
                                               int flag_a, int flag_b,
                                               int (__cdecl *callback)());
int __cdecl pops_value_text_title(char *label, int value,
                                            char *text, int title,
                                            Sprite *sprite,
                                            int (__cdecl *callback)());
int __cdecl pops_default_caption(char *label, int value,
                                           char *text, int title,
                                           Sprite *sprite, int flag_a,
                                           int flag_b,
                                           int (__cdecl *callback)());
int __cdecl pops_no_flags(char *caption, char *label, int value,
                                    char *text, int title, Sprite *sprite,
                                    int (__cdecl *callback)());

class Wave;
class FX;

// The popup state the wave callback reads. Only the two fields it touches
// are carved: the wave index at 0x104 and the armed marker at 0x108 that
// must be set for anything to sound.
struct PopupWave {
  uint8_t opaque_[0x104];
  int32_t wave_index_;  // +0x104
  void *armed_108_;     // +0x108
};

static_assert(offsetof(PopupWave, wave_index_) == 0x104,
              "the callback reads the index at 0x104");
static_assert(offsetof(PopupWave, armed_108_) == 0x108,
              "the callback reads the armed marker at 0x108");

// The callback's dependencies, every one rebindable: the sound-enable flag
// word (bit 0x400 gates everything), the context whose field at 0x50 turns
// wave 0x19 into 0x25 deep below sea level, the voiceover wave, the
// forty-five entry popup wave bank with its last-played index beside it,
// the owner whose virtual at 0x138 fires after wave 0x19, and the FX bank
// that plays effect 0x38 after wave 0x10.
typedef int (OriginalObject::*func_popup_wave_query)();
typedef unsigned long(__stdcall func_popup_time_source)(void);

uint32_t *const PopupWaveFlags = (uint32_t *)0x009A6490;
inline void *&PopupWaveContext() { return *reinterpret_cast<void **>(0x0090EA30); }
Wave *const PopupWaveVoice = (Wave *)0x00945ED0;
Wave *const PopupWaveBank = (Wave *)0x0074C5F0;
int32_t *const PopupWaveLastIndex = (int32_t *)0x0074DAA4;
inline void *&PopupWaveOwnerSlot() { return *reinterpret_cast<void **>(0x0074DAA0); }
FX *const PopupWaveFx = (FX *)0x00749CF8;
inline func_popup_time_source *&PopupWaveTimeSlot() { return *reinterpret_cast<func_popup_time_source **>(0x00669368); }

void __cdecl popup_wave_callback(PopupWave *popup, int);
void __cdecl popup_wave_callback_redirect(PopupWave *popup, int a2);
