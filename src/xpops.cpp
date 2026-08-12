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
#include "xpops.h"

func_x_pops_full *XPopsOriginalFull = (func_x_pops_full *)0x005BF930;
func_x_pop_full *XPopOriginalFull = (func_x_pop_full *)0x005BF480;
char *XPopsCaptionBuffer = (char *)0x009B8AA8;

/*
Purpose: Raise a popup with the defaults the short form implies - the shared
         caption buffer, no title, no override text, and both flags set.
ORIGINAL: 0x005BF7D0
// name      ?X_pops@@YAHPBDHPAUSprite@@P6AHXZ@Z
// size      42 bytes
// spans     0x005BF7D0-0x005BF7FA
// prototype 
// callers   27   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_short(const char *label, int value, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: The three-argument popup form: like the short form but with the value
         defaulted to zero and no override text.
ORIGINAL: 0x005BF5D0
// name      ?X_pops@@YAHPBDPAUSprite@@P6AHXZ@Z
// size      40 bytes
// spans     0x005BF5D0-0x005BF5F8
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_minimal(const char *label, Sprite *sprite,
                           int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, 0,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the caption-less popup with its own caption and label supplied
         by the caller, rather than the shared buffer - value, override text
         and flags all take their defaults (-1, none, 0).
ORIGINAL: 0x005BF330
// name      ?X_pop@@YAHPADPBDP6AHXZ@Z
// size      31 bytes
// spans     0x005BF330-0x005BF34F
// prototype 
// callers   17   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_caption_label(char *caption, const char *label,
                                int (__cdecl *callback)()) {
    return XPopOriginalFull(caption, label, -1, nullptr, 0, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer, with
         both the value and the flags supplied by the caller. No override
         text.
ORIGINAL: 0x005BF390
// name      ?X_pop@@YAHPBDHHP6AHXZ@Z
// size      36 bytes
// spans     0x005BF390-0x005BF3B4
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_value_flags(const char *label, int value, int flags,
                              int (__cdecl *callback)()) {
    return XPopOriginalFull(XPopsCaptionBuffer, label, value, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption, value
         and flags. No override text.
ORIGINAL: 0x005BF3C0
// name      ?X_pop@@YAHPADPBDHHP6AHXZ@Z
// size      35 bytes
// spans     0x005BF3C0-0x005BF3E3
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_caption_value_flags(char *caption, const char *label,
                                     int value, int flags,
                                     int (__cdecl *callback)()) {
    return XPopOriginalFull(caption, label, value, nullptr, flags, callback);
}

/*
Purpose: The short form of the caption-less popup - shared caption buffer,
         value defaulted to -1, no override text, only the flags exposed.
ORIGINAL: 0x005BF3F0
// name      ?X_pop@@YAHPBDHP6AHXZ@Z
// size      34 bytes
// spans     0x005BF3F0-0x005BF412
// prototype 
// callers   17   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_short(const char *label, int flags,
                        int (__cdecl *callback)()) {
    return XPopOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption and
         flags; value defaults to -1 and there is no override text.
ORIGINAL: 0x005BF420
// name      ?X_pop@@YAHPADPBDHP6AHXZ@Z
// size      33 bytes
// spans     0x005BF420-0x005BF441
// prototype 
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_caption_flags(char *caption, const char *label, int flags,
                                int (__cdecl *callback)()) {
    return XPopOriginalFull(caption, label, -1, nullptr, flags, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer with
         everything else - value, override text and flags - supplied by the
         caller.
ORIGINAL: 0x005BF450
// name      ?X_pop@@YAHPBDHPADHP6AHXZ@Z
// size      38 bytes
// spans     0x005BF450-0x005BF476
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_default_caption(const char *label, int value, char *text,
                                  int flags, int (__cdecl *callback)()) {
    return XPopOriginalFull(XPopsCaptionBuffer, label, value, text, flags,
                            callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1, no override text and value forced to 0; the sprite
         and both flags are caller-supplied. Unlike x_pops_short/minimal, the
         flags are not fixed to 1/1 here.
ORIGINAL: 0x005BF600
// name      ?X_pops@@YAHPBDPAUSprite@@HHP6AHXZ@Z
// size      44 bytes
// spans     0x005BF600-0x005BF62C
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_flags(const char *label, Sprite *sprite, int flag1,
                         int flag2, int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, 0,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: The minimal popup form with a caller-supplied caption instead of the
         shared buffer - title -1, no override text, value 0, both flags
         fixed at 1.
ORIGINAL: 0x005BF630
// name      ?X_pops@@YAHPADPBDPAUSprite@@P6AHXZ@Z
// size      39 bytes
// spans     0x005BF630-0x005BF657
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_minimal(char *caption, const char *label,
                                   Sprite *sprite,
                                   int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, -1, nullptr, 0, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_flags but with a caller-supplied caption in place of the
         shared buffer - title -1, no override text, value forced to 0, both
         flags caller-supplied.
ORIGINAL: 0x005BF660
// name      ?X_pops@@YAHPADPBDPAUSprite@@HHP6AHXZ@Z
// size      43 bytes
// spans     0x005BF660-0x005BF68B
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_flags(char *caption, const char *label,
                                 Sprite *sprite, int flag1, int flag2,
                                 int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, -1, nullptr, 0, sprite, flag1,
                             flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with both
         title and value exposed (unlike the short form, which only exposes
         value); no override text, both flags fixed at 1.
ORIGINAL: 0x005BF690
// name      ?X_pops@@YAHPBDHHPAUSprite@@P6AHXZ@Z
// size      44 bytes
// spans     0x005BF690-0x005BF6BC
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_title(const char *label, int title, int value,
                         Sprite *sprite, int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, title, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Like x_pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, no override text.
ORIGINAL: 0x005BF6C0
// name      ?X_pops@@YAHPBDHHPAUSprite@@HHP6AHXZ@Z
// size      48 bytes
// spans     0x005BF6C0-0x005BF6F0
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_title_flags(const char *label, int title, int value,
                               Sprite *sprite, int flag1, int flag2,
                               int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, title, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_title but with a caller-supplied caption instead of the
         shared buffer; no override text, both flags fixed at 1.
ORIGINAL: 0x005BF770
// name      ?X_pops@@YAHPADPBDHHPAUSprite@@P6AHXZ@Z
// size      43 bytes
// spans     0x005BF770-0x005BF79B
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_title(char *caption, const char *label, int title,
                                 int value, Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, title, nullptr, value, sprite, 1,
                             1, callback);
}

/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, title, value, sprite and both flags.
ORIGINAL: 0x005BF7A0
// name      ?X_pops@@YAHPADPBDHHPAUSprite@@HHP6AHXZ@Z
// size      47 bytes
// spans     0x005BF7A0-0x005BF7CF
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_no_text(char *caption, const char *label, int title,
                           int value, Sprite *sprite, int flag1, int flag2,
                           int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, title, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1 and no override text; value and both flags are
         caller-supplied.
ORIGINAL: 0x005BF800
// name      ?X_pops@@YAHPBDHPAUSprite@@HHP6AHXZ@Z
// size      46 bytes
// spans     0x005BF800-0x005BF82E
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_value_flags(const char *label, int value, Sprite *sprite,
                               int flag1, int flag2,
                               int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_value_flags but with a caller-supplied caption in place
         of the shared buffer and both flags fixed at 1; title defaults to
         -1, no override text.
ORIGINAL: 0x005BF830
// name      ?X_pops@@YAHPADPBDHPAUSprite@@P6AHXZ@Z
// size      41 bytes
// spans     0x005BF830-0x005BF859
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_value(char *caption, const char *label, int value,
                                 Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, -1, nullptr, value, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_caption_value but with both flags caller-supplied
         instead of fixed at 1; title defaults to -1, no override text.
ORIGINAL: 0x005BF860
// name      ?X_pops@@YAHPADPBDHPAUSprite@@HHP6AHXZ@Z
// size      45 bytes
// spans     0x005BF860-0x005BF88D
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_value_flags(char *caption, const char *label,
                                      int value, Sprite *sprite, int flag1,
                                      int flag2,
                                      int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, -1, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title,
         override text and value all caller-supplied; both flags fixed at 1.
ORIGINAL: 0x005BF890
// name      ?X_pops@@YAHPBDHPADHPAUSprite@@P6AHXZ@Z
// size      46 bytes
// spans     0x005BF890-0x005BF8BE
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_override_text(const char *label, int title, char *text,
                                 int value, Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, title, text, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with every
         other argument - title, override text, value, sprite and both flags
         - caller-supplied. Only the caption defaults.
ORIGINAL: 0x005BF8C0
// name      ?X_pops@@YAHPBDHPADHPAUSprite@@HHP6AHXZ@Z
// size      50 bytes
// spans     0x005BF8C0-0x005BF8F2
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_default_caption(const char *label, int title, char *text,
                                   int value, Sprite *sprite, int flag1,
                                   int flag2, int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, title, text, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Raise the full popup with caption, title, override text, value and
         sprite all caller-supplied; only the two flags default, fixed at 1.
ORIGINAL: 0x005BF900
// name      ?X_pops@@YAHPADPBDHPADHPAUSprite@@P6AHXZ@Z
// size      45 bytes
// spans     0x005BF900-0x005BF92D
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_no_flags(char *caption, const char *label, int title,
                            char *text, int value, Sprite *sprite,
                            int (__cdecl *callback)()) {
    return XPopsOriginalFull(caption, label, title, text, value, sprite, 1, 1,
                             callback);
}
