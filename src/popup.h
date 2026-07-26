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

 private:
  Scroll scroll_;
};

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
