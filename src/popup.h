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
class DLLEXPORT Popup : BasePop {
 public:
  Popup() { ; }
  ~Popup() { ; }
  void close();
  void start(char *a1, const char *a2, int a3, char *a4, int a5);
  void start(const char *label);
  void start(const char *label, int value);
  int on_dialog_back_draw(::GraphicWin *window);
  void on_redraw_nc(int, int);

 private:
  Scroll scroll_;
};

void __fastcall popup_on_redraw_nc_redirect(
    Popup *self, void *, int a1, int a2);

// BasePop::close is not recovered yet.
typedef void (__thiscall func_base_pop_close)(BasePop *);
extern func_base_pop_close *BasePopOriginalClose;

void __fastcall popup_close_redirect(Popup *self, void *);

// The six-argument Popup::start is not recovered; the five-argument form
// forwards to it with a null final GraphicWin argument.
typedef void (__thiscall func_popup_start_full)(Popup *, char *, const char *,
                                                int, char *, int, void *);
extern func_popup_start_full *PopupOriginalStartFull;

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5);

// The two short start forms share the caption buffer at a fixed address.
extern char *PopupStartCaption;

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
typedef int (__cdecl func_pops_full)(char *, char *, int, char *, int,
                                     Sprite *, int, int, int (__cdecl *)());
extern func_pops_full *PopsOriginalFull;

DLLEXPORT int __cdecl pop_label_cb(char *label, int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_caption(char *caption, char *label,
                                  int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_value_title(char *label, int value, int title,
                                      int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_caption_value_title(char *caption, char *label,
                                              int value, int title,
                                              int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_title(char *label, int title,
                                int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_caption_title(char *caption, char *label, int title,
                                        int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_value_text_title(char *label, int value,
                                           char *text, int title,
                                           int (__cdecl *callback)());
DLLEXPORT int __cdecl pop_full(char *caption, char *label, int value,
                               char *text, int title,
                               int (__cdecl *callback)());

DLLEXPORT int __cdecl pops_minimal(char *label, Sprite *sprite,
                                   int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_flags(char *label, Sprite *sprite, int flag_a,
                                 int flag_b, int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_caption(char *caption, char *label,
                                   Sprite *sprite, int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_caption_flags(char *caption, char *label,
                                         Sprite *sprite, int flag_a,
                                         int flag_b,
                                         int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_value_title(char *label, int value, int title,
                                       Sprite *sprite,
                                       int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_value_title_flags(char *label, int value,
                                             int title, Sprite *sprite,
                                             int flag_a, int flag_b,
                                             int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_caption_value_title(char *caption, char *label,
                                               int value, int title,
                                               Sprite *sprite,
                                               int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_no_text(char *caption, char *label, int value,
                                   int title, Sprite *sprite, int flag_a,
                                   int flag_b, int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_title(char *label, int title, Sprite *sprite,
                                 int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_title_flags(char *label, int title,
                                       Sprite *sprite, int flag_a,
                                       int flag_b,
                                       int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_caption_title(char *caption, char *label,
                                         int title, Sprite *sprite,
                                         int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_caption_title_flags(char *caption, char *label,
                                               int title, Sprite *sprite,
                                               int flag_a, int flag_b,
                                               int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_value_text_title(char *label, int value,
                                            char *text, int title,
                                            Sprite *sprite,
                                            int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_default_caption(char *label, int value,
                                           char *text, int title,
                                           Sprite *sprite, int flag_a,
                                           int flag_b,
                                           int (__cdecl *callback)());
DLLEXPORT int __cdecl pops_no_flags(char *caption, char *label, int value,
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
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef int(__thiscall func_popup_wave_query)(Wave *wave);
typedef void(__thiscall func_popup_fx_play)(FX *fx, int effect);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
typedef unsigned long(__stdcall func_popup_time_source)(void);

extern uint32_t *PopupWaveFlags;
extern void **PopupWaveContext;
extern Wave *PopupWaveVoice;
extern Wave *PopupWaveBank;
extern int32_t *PopupWaveLastIndex;
extern void **PopupWaveOwnerSlot;
extern FX *PopupWaveFx;
extern func_popup_wave_query *PopupWaveIsPlaying;
extern func_popup_wave_query *PopupWaveLoad;
extern func_popup_wave_query *PopupWavePlay;
extern func_popup_fx_play *PopupFxPlay;
extern func_popup_time_source **PopupWaveTimeSlot;

DLLEXPORT void __cdecl popup_wave_callback(PopupWave *popup, int);
void __cdecl popup_wave_callback_redirect(PopupWave *popup, int a2);
