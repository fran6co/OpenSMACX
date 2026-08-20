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
#include "sprite.h"

 /*
  * Popup dialog helper
  *
  * A four-argument convenience wrapper over the nine-argument popup builder,
  * supplying a shared caption buffer, the two defaulted flags and the two
  * placeholders the short form does not expose.
  */
// 0x005BF930, still a pending_bodies forwarder. A NAME, not a
// `func_x_pops_full *`: the pointer compiled `call [ptr]` at every wrapper
// below, where the image has `call rel32`.
int __cdecl X_pops(char *caption, const char *label, int a3, char *a4, int a5,
                   Sprite *sprite, int a7, int a8, int (__cdecl *callback)());

int __cdecl x_pops_short(const char *label, int value, Sprite *sprite,
                                   int (__cdecl *callback)());
int __cdecl x_pops_minimal(const char *label, Sprite *sprite,
                                     int (__cdecl *callback)());

/*
 * Popup dialog helper, six-argument builder
 *
 * A sibling of the nine-argument popup builder above, without the sprite and
 * the two trailing flags - the caption-less popup form. Its own forwarders
 * default whichever combination of caption/title/override-text/flags the
 * original call site did not need.
 */
// 0x005BF480, likewise.
int __cdecl X_pop(char *caption, const char *label, int a3, char *a4, int a5,
                  int (__cdecl *callback)());

int __cdecl x_pop_caption_label(char *caption, const char *label,
                                          int (__cdecl *callback)());
int __cdecl x_pop_value_flags(const char *label, int value, int flags,
                                        int (__cdecl *callback)());
int __cdecl x_pop_caption_value_flags(char *caption, const char *label,
                                                int value, int flags,
                                                int (__cdecl *callback)());
int __cdecl x_pop_short(const char *label, int flags,
                                  int (__cdecl *callback)());
int __cdecl x_pop_caption_flags(char *caption, const char *label,
                                          int flags, int (__cdecl *callback)());
int __cdecl x_pop_default_caption(const char *label, int value,
                                            char *text, int flags,
                                            int (__cdecl *callback)());

/*
 * Popup dialog helper, nine-argument builder forwarders
 *
 * Each of these defaults a different subset of XPopsOriginalFull's nine
 * arguments (caption, label, title, override text, value, sprite, the two
 * flags, callback) to the values the original call site did not vary. Named
 * for whichever of caption/title/override-text/value/flags each one exposes
 * rather than defaulting.
 */
int __cdecl x_pops_flags(const char *label, Sprite *sprite,
                                   int flag1, int flag2,
                                   int (__cdecl *callback)());
int __cdecl x_pops_caption_minimal(char *caption, const char *label,
                                             Sprite *sprite,
                                             int (__cdecl *callback)());
int __cdecl x_pops_caption_flags(char *caption, const char *label,
                                           Sprite *sprite, int flag1,
                                           int flag2,
                                           int (__cdecl *callback)());
int __cdecl x_pops_title(const char *label, int title, int value,
                                   Sprite *sprite,
                                   int (__cdecl *callback)());
int __cdecl x_pops_title_flags(const char *label, int title,
                                         int value, Sprite *sprite,
                                         int flag1, int flag2,
                                         int (__cdecl *callback)());
int __cdecl x_pops_caption_title(char *caption, const char *label,
                                           int title, int value,
                                           Sprite *sprite,
                                           int (__cdecl *callback)());
int __cdecl x_pops_no_text(char *caption, const char *label,
                                     int title, int value, Sprite *sprite,
                                     int flag1, int flag2,
                                     int (__cdecl *callback)());
int __cdecl x_pops_value_flags(const char *label, int value,
                                         Sprite *sprite, int flag1, int flag2,
                                         int (__cdecl *callback)());
int __cdecl x_pops_caption_value(char *caption, const char *label,
                                           int value, Sprite *sprite,
                                           int (__cdecl *callback)());
int __cdecl x_pops_caption_value_flags(char *caption,
                                                 const char *label, int value,
                                                 Sprite *sprite, int flag1,
                                                 int flag2,
                                                 int (__cdecl *callback)());
int __cdecl x_pops_override_text(const char *label, int title,
                                           char *text, int value,
                                           Sprite *sprite,
                                           int (__cdecl *callback)());
int __cdecl x_pops_default_caption(const char *label, int title,
                                             char *text, int value,
                                             Sprite *sprite, int flag1,
                                             int flag2,
                                             int (__cdecl *callback)());
int __cdecl x_pops_no_flags(char *caption, const char *label,
                                      int title, char *text, int value,
                                      Sprite *sprite,
                                      int (__cdecl *callback)());
