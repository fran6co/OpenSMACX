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
#include "popup.h"   // PopupStartCaption, the shared caption buffer
#include "faction.h"  // PopupDialogFactionID, ExpansionEnabled
#include "spying_recovery.h"  // SpyingFactionFlagBytes
#include <cstdio>
#include <cstring>

// THE SAME BUFFER `popup.cpp` calls `PopupStartCaption` - one array at
// 0x009B8AA8 under two names. Defined there; used here.

/*
Purpose: Raise a popup with the defaults the short form implies - the shared
         caption buffer, no title, no override text, and both flags set.
// ORIGINAL: 0x005BF7D0 ?X_pops@@YAHPBDHPAUSprite@@P6AHXZ@Z 0x005BF7D0-0x005BF7FA BYTE_EXACT
// symbol    ?x_pops_short@@YAHPBDHPAVSprite@@P6AHXZ@Z
// size      42 bytes
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
    return X_pops(PopupStartCaption, label, -1, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: The three-argument popup form: like the short form but with the value
         defaulted to zero and no override text.
// ORIGINAL: 0x005BF5D0 ?X_pops@@YAHPBDPAUSprite@@P6AHXZ@Z 0x005BF5D0-0x005BF5F8 BYTE_EXACT
// symbol    ?x_pops_minimal@@YAHPBDPAVSprite@@P6AHXZ@Z
// size      40 bytes
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
    return X_pops(PopupStartCaption, label, -1, nullptr, 0,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the caption-less popup with its own caption and label supplied
         by the caller, rather than the shared buffer - value, override text
         and flags all take their defaults (-1, none, 0).
// ORIGINAL: 0x005BF330 ?X_pop@@YAHPADPBDP6AHXZ@Z 0x005BF330-0x005BF34F BYTE_EXACT
// symbol    ?x_pop_caption_label@@YAHPADPBDP6AHXZ@Z
// size      31 bytes
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
    return X_pop(caption, label, -1, nullptr, 0, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer, with
         both the value and the flags supplied by the caller. No override
         text.
// ORIGINAL: 0x005BF390 ?X_pop@@YAHPBDHHP6AHXZ@Z 0x005BF390-0x005BF3B4 BYTE_EXACT
// symbol    ?x_pop_value_flags@@YAHPBDHHP6AHXZ@Z
// size      36 bytes
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
    return X_pop(PopupStartCaption, label, value, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption, value
         and flags. No override text.
// ORIGINAL: 0x005BF3C0 ?X_pop@@YAHPADPBDHHP6AHXZ@Z 0x005BF3C0-0x005BF3E3 BYTE_EXACT
// symbol    ?x_pop_caption_value_flags@@YAHPADPBDHHP6AHXZ@Z
// size      35 bytes
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
    return X_pop(caption, label, value, nullptr, flags, callback);
}

/*
Purpose: The short form of the caption-less popup - shared caption buffer,
         value defaulted to -1, no override text, only the flags exposed.
// ORIGINAL: 0x005BF3F0 ?X_pop@@YAHPBDHP6AHXZ@Z 0x005BF3F0-0x005BF412 BYTE_EXACT
// symbol    ?x_pop_short@@YAHPBDHP6AHXZ@Z
// size      34 bytes
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
    return X_pop(PopupStartCaption, label, -1, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption and
         flags; value defaults to -1 and there is no override text.
// ORIGINAL: 0x005BF420 ?X_pop@@YAHPADPBDHP6AHXZ@Z 0x005BF420-0x005BF441 BYTE_EXACT
// symbol    ?x_pop_caption_flags@@YAHPADPBDHP6AHXZ@Z
// size      33 bytes
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
    return X_pop(caption, label, -1, nullptr, flags, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer with
         everything else - value, override text and flags - supplied by the
         caller.
// ORIGINAL: 0x005BF450 ?X_pop@@YAHPBDHPADHP6AHXZ@Z 0x005BF450-0x005BF476 BYTE_EXACT
// symbol    ?x_pop_default_caption@@YAHPBDHPADHP6AHXZ@Z
// size      38 bytes
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
    return X_pop(PopupStartCaption, label, value, text, flags,
                            callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1, no override text and value forced to 0; the sprite
         and both flags are caller-supplied. Unlike x_pops_short/minimal, the
         flags are not fixed to 1/1 here.
// ORIGINAL: 0x005BF600 ?X_pops@@YAHPBDPAUSprite@@HHP6AHXZ@Z 0x005BF600-0x005BF62C BYTE_EXACT
// symbol    ?x_pops_flags@@YAHPBDPAVSprite@@HHP6AHXZ@Z
// size      44 bytes
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
    return X_pops(PopupStartCaption, label, -1, nullptr, 0,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: The minimal popup form with a caller-supplied caption instead of the
         shared buffer - title -1, no override text, value 0, both flags
         fixed at 1.
// ORIGINAL: 0x005BF630 ?X_pops@@YAHPADPBDPAUSprite@@P6AHXZ@Z 0x005BF630-0x005BF657 BYTE_EXACT
// symbol    ?x_pops_caption_minimal@@YAHPADPBDPAVSprite@@P6AHXZ@Z
// size      39 bytes
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
    return X_pops(caption, label, -1, nullptr, 0, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_flags but with a caller-supplied caption in place of the
         shared buffer - title -1, no override text, value forced to 0, both
         flags caller-supplied.
// ORIGINAL: 0x005BF660 ?X_pops@@YAHPADPBDPAUSprite@@HHP6AHXZ@Z 0x005BF660-0x005BF68B BYTE_EXACT
// symbol    ?x_pops_caption_flags@@YAHPADPBDPAVSprite@@HHP6AHXZ@Z
// size      43 bytes
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
    return X_pops(caption, label, -1, nullptr, 0, sprite, flag1,
                             flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with both
         title and value exposed (unlike the short form, which only exposes
         value); no override text, both flags fixed at 1.
// ORIGINAL: 0x005BF690 ?X_pops@@YAHPBDHHPAUSprite@@P6AHXZ@Z 0x005BF690-0x005BF6BC BYTE_EXACT
// symbol    ?x_pops_title@@YAHPBDHHPAVSprite@@P6AHXZ@Z
// size      44 bytes
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
    return X_pops(PopupStartCaption, label, title, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Like x_pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, no override text.
// ORIGINAL: 0x005BF6C0 ?X_pops@@YAHPBDHHPAUSprite@@HHP6AHXZ@Z 0x005BF6C0-0x005BF6F0 BYTE_EXACT
// symbol    ?x_pops_title_flags@@YAHPBDHHPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
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
    return X_pops(PopupStartCaption, label, title, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_title but with a caller-supplied caption instead of the
         shared buffer; no override text, both flags fixed at 1.
// ORIGINAL: 0x005BF770 ?X_pops@@YAHPADPBDHHPAUSprite@@P6AHXZ@Z 0x005BF770-0x005BF79B BYTE_EXACT
// symbol    ?x_pops_caption_title@@YAHPADPBDHHPAVSprite@@P6AHXZ@Z
// size      43 bytes
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
    return X_pops(caption, label, title, nullptr, value, sprite, 1,
                             1, callback);
}

/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, title, value, sprite and both flags.
// ORIGINAL: 0x005BF7A0 ?X_pops@@YAHPADPBDHHPAUSprite@@HHP6AHXZ@Z 0x005BF7A0-0x005BF7CF BYTE_EXACT
// symbol    ?x_pops_no_text@@YAHPADPBDHHPAVSprite@@HHP6AHXZ@Z
// size      47 bytes
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
    return X_pops(caption, label, title, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1 and no override text; value and both flags are
         caller-supplied.
// ORIGINAL: 0x005BF800 ?X_pops@@YAHPBDHPAUSprite@@HHP6AHXZ@Z 0x005BF800-0x005BF82E BYTE_EXACT
// symbol    ?x_pops_value_flags@@YAHPBDHPAVSprite@@HHP6AHXZ@Z
// size      46 bytes
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
    return X_pops(PopupStartCaption, label, -1, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_value_flags but with a caller-supplied caption in place
         of the shared buffer and both flags fixed at 1; title defaults to
         -1, no override text.
// ORIGINAL: 0x005BF830 ?X_pops@@YAHPADPBDHPAUSprite@@P6AHXZ@Z 0x005BF830-0x005BF859 BYTE_EXACT
// symbol    ?x_pops_caption_value@@YAHPADPBDHPAVSprite@@P6AHXZ@Z
// size      41 bytes
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
    return X_pops(caption, label, -1, nullptr, value, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_caption_value but with both flags caller-supplied
         instead of fixed at 1; title defaults to -1, no override text.
// ORIGINAL: 0x005BF860 ?X_pops@@YAHPADPBDHPAUSprite@@HHP6AHXZ@Z 0x005BF860-0x005BF88D BYTE_EXACT
// symbol    ?x_pops_caption_value_flags@@YAHPADPBDHPAVSprite@@HHP6AHXZ@Z
// size      45 bytes
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
    return X_pops(caption, label, -1, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title,
         override text and value all caller-supplied; both flags fixed at 1.
// ORIGINAL: 0x005BF890 ?X_pops@@YAHPBDHPADHPAUSprite@@P6AHXZ@Z 0x005BF890-0x005BF8BE BYTE_EXACT
// symbol    ?x_pops_override_text@@YAHPBDHPADHPAVSprite@@P6AHXZ@Z
// size      46 bytes
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
    return X_pops(PopupStartCaption, label, title, text, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with every
         other argument - title, override text, value, sprite and both flags
         - caller-supplied. Only the caption defaults.
// ORIGINAL: 0x005BF8C0 ?X_pops@@YAHPBDHPADHPAUSprite@@HHP6AHXZ@Z 0x005BF8C0-0x005BF8F2 BYTE_EXACT
// symbol    ?x_pops_default_caption@@YAHPBDHPADHPAVSprite@@HHP6AHXZ@Z
// size      50 bytes
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
    return X_pops(PopupStartCaption, label, title, text, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Raise the full popup with caption, title, override text, value and
         sprite all caller-supplied; only the two flags default, fixed at 1.
// ORIGINAL: 0x005BF900 ?X_pops@@YAHPADPBDHPADHPAUSprite@@P6AHXZ@Z 0x005BF900-0x005BF92D BYTE_EXACT
// symbol    ?x_pops_no_flags@@YAHPADPBDH0HPAVSprite@@P6AHXZ@Z
// size      45 bytes
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
    return X_pops(caption, label, title, text, value, sprite, 1, 1,
                             callback);
}

// Eight fixed-address strings X_pop reaches for the mod-key check below, no
// established identity beyond what this one function shows: two
// filename/mode pairs it fopen/fclose-probes, two override captions/labels
// it compares against, and two override-caption pointers it substitutes.
// XPopModCheckString1 lives AT the address, not pointed at from it - a
// variable holding a string pointer, read INLINE through a cast so the
// dereference folds to one instruction rather than materialising the
// address in a register first (the shape console_map_win() already uses).
// string2 is used at the address level - a literal sitting right there.
inline LPCSTR XPopModCheckString1() {
    return *reinterpret_cast<LPCSTR *>(0x00691B0C);
}
LPCSTR const XPopModCheckString2 = (LPCSTR)0x00691C80;
LPCSTR const XPopModCheckFile1 = (LPCSTR)0x00691C90;
LPCSTR const XPopModCheckMode1 = (LPCSTR)0x00691C8C;
// Both caption substitutes are variables holding a string pointer, like
// XPopModCheckString1 above.
inline LPCSTR XPopModCheckCaption1() {
    return *reinterpret_cast<LPCSTR *>(0x00691B20);
}
LPCSTR const XPopModCheckFile2 = (LPCSTR)0x00691CA8;
LPCSTR const XPopModCheckMode2 = (LPCSTR)0x00691CA4;
inline LPCSTR XPopModCheckCaption2() {
    return *reinterpret_cast<LPCSTR *>(0x00691B24);
}

/*
Purpose: The full builder every xpops wrapper funnels into. When the label
         matches one of two fixed override strings, probe for a mod-key file
         (opening and immediately closing it) gated on the human's own or
         the popup dialog's faction having its spying flag set, and on
         expansions being enabled; on success substitute a fixed caption for
         the popup. Either way, fall through to the shared pop builder.
// ORIGINAL: 0x005BF480 ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z 0x005BF480-0x005BF5C2 SEMANTIC
// RULED-OUT: MNEMONIC_ONLY 100/127, best across all flag sets. The remaining gap is a pure register-rotation (eax/ecx/edx swapped) at three near-identical pop_full(cap, label, a3, a4, a5, callback) call sites reading [ebp+0xc] (label); caching label in a local, reordering it against the caption expression via separate statements, and reading the two fixed-address caption substitutes through an inline accessor (fixed a genuine double-indirection bug first) all leave the same rotation.
// symbol    ?X_pop@@YAHPADPBDH0HP6AHXZ@Z
// size      322 bytes
// prototype int (__cdecl ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z)(int8*, int8*, int length, int8*, int, int (__cdecl *)())
// callers   9   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006272C0 0x00645598 0x00645646 0x00645660
Return Value: whatever the shared pop builder returns
Status: Complete
*/
int __cdecl X_pop(char *caption, const char *label, int a3, char *a4, int a5,
                  int (__cdecl *callback)()) {
    char *const label2 = const_cast<char *>(label);
    if (strcmp(caption, XPopModCheckString1()) == 0 ||
        strcmp(caption, XPopModCheckString2) == 0) {
        if ((SpyingFactionFlagBytes[LocalFaction * 0x59C] & 0x80) != 0) {
            if (ExpansionEnabled != 0) {
                FILE *const fp = fopen(XPopModCheckFile1, XPopModCheckMode1);
                if (fp != nullptr) {
                    fclose(fp);
                    char *const lbl = label2;
                    char *const cap = const_cast<char *>(XPopModCheckCaption1());
                    int const result = pop_full(cap, lbl, a3, a4, a5, callback);
                    if (result != -1) {
                        return result;
                    }
                }
            }
        } else if (ExpansionEnabled != 0 && 0 < PopupDialogFactionID &&
                  PopupDialogFactionID < 8 &&
                  (SpyingFactionFlagBytes[PopupDialogFactionID * 0x59C] &
                   0x80) != 0) {
            FILE *const fp = fopen(XPopModCheckFile2, XPopModCheckMode2);
            if (fp != nullptr) {
                fclose(fp);
                char *const lbl = label2;
                char *const cap = const_cast<char *>(XPopModCheckCaption2());
                int const result = pop_full(cap, lbl, a3, a4, a5, callback);
                if (result != -1) {
                    return result;
                }
            }
        }
        return pop_full(caption, label2, a3, a4, a5, callback);
    }
    return pop_full(caption, label2, a3, a4, a5, callback);
}
