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

class Sprite;

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
// 0x006276A0, a mapped forwarder. Declared as a name rather
// than reached through `func_pops_full *`, which compiled `call [ptr]` at
// every one of the `pop` wrappers below.
int __cdecl pops(char *caption, char *label, int a3, char *a4, int a5,
                 Sprite *sprite, int a7, int a8, int (__cdecl *callback)());

int __cdecl pop_label_cb(char *label, int (__cdecl *callback)());
void __cdecl pop_close();
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

int __cdecl pop(char *caption, char *label, char *text, int value,
                int (__cdecl *callback)());
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int (__cdecl *callback)());
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int flag_a, int flag_b,
                 int (__cdecl *callback)());
int __cdecl pop_ask(char *text_file, char *section, int value,
                    int (__cdecl *callback)(), int flags);
int __cdecl pop_ask_number(char *caption, char *label,
                           int (__cdecl *callback)());
int __cdecl set_popup_saved_location(int x, int y);
