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

// The shared popup implementation owns these dialogs in popup.cpp. Keep the
// declarations local to this TU: the public popup header does not expose them.
int __cdecl pop_ask(char *text_file, char *text_label, int value,
                    char *text, int (__cdecl *callback)(), int flags);
int __cdecl pop_ask_number(char *caption, char *label, int value,
                           int (__cdecl *callback)());

// THE SAME BUFFER `popup.cpp` calls `PopupStartCaption` - one array at
// 0x009B8AA8 under two names. Defined there; used here.

/*
Purpose: Raise a popup with caller-supplied override text, using its string
         length and the default pair of enabled flags.
// size      60 bytes
// kind      game
Return Value: whatever the full form returns, or -1 for a null text pointer
Status: Complete
*/
// ORIGINAL: 0x005BF6F0 ?X_pops@@YAHPADPBDPADHPAUSprite@@P6AHXZ@Z 0x005BF6F0-0x005BF72C BYTE_EXACT
// symbol    ?X_pops@@YAHPADPBD0HPAVSprite@@P6AHXZ@Z
int __cdecl X_pops(char *caption, const char *label, char *text, int value,
                   Sprite *sprite, int (__cdecl *callback)()) {
    if (text == nullptr) {
        return -1;
    }
    const int length = static_cast<int>(std::strlen(text));
    return X_pops(caption, label, length, text, value, sprite, 1, 1,
                  callback);
}

/*
Purpose: Raise a popup with caller-supplied override text and flags.
// size      64 bytes
// kind      game
Return Value: whatever the full form returns, or -1 for a null text pointer
Status: Complete
*/
// ORIGINAL: 0x005BF730 ?X_pops@@YAHPADPBDPADHPAUSprite@@HHP6AHXZ@Z 0x005BF730-0x005BF770 BYTE_EXACT
// symbol    ?X_pops@@YAHPADPBD0HPAVSprite@@HHP6AHXZ@Z
int __cdecl X_pops(char *caption, const char *label, char *text, int value,
                   Sprite *sprite, int flag1, int flag2,
                   int (__cdecl *callback)()) {
    if (text == nullptr) {
        return -1;
    }
    const int length = static_cast<int>(std::strlen(text));
    return X_pops(caption, label, length, text, value, sprite, flag1, flag2,
                  callback);
}

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

// Eight fixed strings X_pop reaches for the mod-key check below: two
// filename/mode pairs it fopen/fclose-probes, two override captions/labels
// it compares against, and two override-caption pointers it substitutes.
// REAL LITERALS AND SLOTS since 2026-08-29 - the raw .rdata bindings
// (0x691B7C, 0x691C80, 0x691C8C, 0x691C90, 0x691CA4, 0x691CA8) and the
// three pointer slots (0x691B0C, 0x691B20, 0x691B24, initialized in the
// image to "SCRIPT", "alienIscript", "alienuscript") all pointed into
// terranx.exe's read-only data, unmapped in the standalone build - the
// very first X_pop call faulted reading the second compare string.
// The three SLOT-HELD ones stay behind inline accessors reading through
// a real slot variable, which is the one-instruction double indirection
// the image's `mov eax, [slot]` has.
static const char *const XPopModCheckString1Slot = "SCRIPT";
static const char XPopModCheckString2Text[] = "SCRIPT.txt";
static const char XPopModCheckFile1Text[] = "alienIscript.txt";
static const char XPopModCheckMode1Text[] = "rb";
static const char *const XPopModCheckCaption1Slot = "alienIscript";
static const char XPopModCheckFile2Text[] = "alienuscript.txt";
static const char XPopModCheckMode2Text[] = "rb";
static const char *const XPopModCheckCaption2Slot = "alienuscript";
inline LPCSTR XPopModCheckString1() {
    return XPopModCheckString1Slot;
}
inline LPCSTR XPopModCheckCaption1() {
    return XPopModCheckCaption1Slot;
}
inline LPCSTR XPopModCheckCaption2() {
    return XPopModCheckCaption2Slot;
}

/*
Purpose: The full builder every xpops wrapper funnels into. When the label
         matches one of two fixed override strings, probe for a mod-key file
         (opening and immediately closing it) gated on the human's own or
         the popup dialog's faction having its spying flag set, and on
         expansions being enabled; on success substitute a fixed caption for
         the popup. Either way, fall through to the shared pop builder.
// ORIGINAL: 0x005BF480 ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z 0x005BF480-0x005BF5C2 SEMANTIC
// TRIED: MNEMONIC_ONLY 100/127, best across all flag sets. The remaining gap is a pure register-rotation (eax/ecx/edx swapped) at three near-identical pop_full(cap, label, a3, a4, a5, callback) call sites reading [ebp+0xc] (label); caching label in a local, reordering it against the caption expression via separate statements, and reading the two fixed-address caption substitutes through an inline accessor (fixed a genuine double-indirection bug first) all leave the same rotation.
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
    // THE ALIEN FLAG, not a "spying" flag: 0x00946F58 is &Players[0].
    // rule_flags (the old binding's own address proves it - Players sits at
    // 0x00946A50 and rule_flags walks to +0x508), and bit 0x80 is
    // RFLAG_ALIEN. That is why the probe files are alienIscript.txt and
    // alienuscript.txt: only an expansion alien faction redirects these
    // two captions.
    if (strcmp(caption, XPopModCheckString1()) == 0 ||
        strcmp(caption, XPopModCheckString2Text) == 0) {
        if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
            if (ExpansionEnabled != 0) {
                FILE *const fp = fopen(XPopModCheckFile1Text, XPopModCheckMode1Text);
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
                  (Players[PopupDialogFactionID].rule_flags &
                   RFLAG_ALIEN) != 0) {
            FILE *const fp = fopen(XPopModCheckFile2Text, XPopModCheckMode2Text);
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

/*
Purpose: The two-argument popup form: caption from the shared buffer, no
         value, no sprite.
// ORIGINAL: 0x005BF310 ?X_pop@@YAHPBDP6AHXZ@Z 0x005BF310-0x005BF330 BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/005bf310.cpp, where the claim proved the artifact and the shipped program held nothing at this address. The artifact reached the caption as `(int8 *)g_009b8aa8`; popup.cpp already models that buffer as PopupStartCaption, and xpops.cpp already includes popup.h for exactly this - the sibling forms above use the same name.
// size      32 bytes
// kind      game
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl X_pop(const char *label, int (__cdecl *callback)()) {
    return X_pop(PopupStartCaption, label, -1, nullptr, 0, callback);
}

// Text-bearing X_pop overload: reject a missing text buffer, otherwise pass
// its actual length to the common six-argument implementation.
// ORIGINAL: 0x005BF350 ?X_pop@@YAHPADPBDPADHP6AHXZ@Z 0x005BF350-0x005BF384 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005bf350.cpp", "src/xpops.h", "src/xpops.cpp"], "reason": "Recover the caption, label, override-text and value overload as a null-checked length-calculating wrapper over the existing typed X_pop core."}
// symbol    ?X_pop@@YAHPADPBD0HP6AHXZ@Z
int __cdecl X_pop(char *caption, const char *label, char *text, int value,
                  int (__cdecl *callback)()) {
    if (text == nullptr) return -1;
    return X_pop(caption, label, static_cast<int>(std::strlen(text)), text,
                 value, callback);
}

// X_pop_ask with shared caption, caller-provided initial value and text.
// ORIGINAL: 0x005BFAB0 ?X_pop_ask@@YAHPBDHPADP6AHXZH@Z 0x005BFAB0-0x005BFAD6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005bfab0.cpp", "src/xpops.h"], "reason": "Recover the shared-caption X_pop_ask overload as a typed delegation to its captioned body."}
int __cdecl X_pop_ask(const char *label, int value, char *text,
                      int (__cdecl *callback)(), int flags) {
    return X_pop_ask(PopupStartCaption, label, value, text, callback, flags);
}

// Captioned X_pop_ask with no initial text buffer.
// ORIGINAL: 0x005BFAE0 ?X_pop_ask@@YAHPADPBDHP6AHXZH@Z 0x005BFAE0-0x005BFB03 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005bfae0.cpp", "src/xpops.h"], "reason": "Recover the captioned X_pop_ask overload that defaults its optional text buffer to null."}
int __cdecl X_pop_ask(char *caption, const char *label, int value,
                      int (__cdecl *callback)(), int flags) {
    return X_pop_ask(caption, label, value, nullptr, callback, flags);
}

// Captioned X_pop_ask overload that derives its initial length from text.
// ORIGINAL: 0x005BFB40 ?X_pop_ask@@YAHPADPBDPADP6AHXZH@Z 0x005BFB40-0x005BFB74 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005bfb40.cpp", "src/xpops.h"], "reason": "Recover the text-bearing X_pop_ask overload with null guard and computed text length."}
// symbol    ?X_pop_ask@@YAHPADPBD0P6AHXZH@Z
int __cdecl X_pop_ask(char *caption, const char *label, char *text,
                      int (__cdecl *callback)(), int flags) {
    if (text == nullptr) return -1;
    return X_pop_ask(caption, label, static_cast<int>(std::strlen(text)),
                     text, callback, flags);
}

// Core text-dialog routing. Match the mod-sensitive caption tokens, probe
// the corresponding optional script file, then fall back to the caller's
// caption if the alternate dialog cannot be opened or accepted.
// ORIGINAL: 0x005BFBC0 ?X_pop_ask@@YAHPADPBDHPADP6AHXZH@Z 0x005BFBC0-0x005BFD02 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005bfbc0.cpp", "src/xpops.h", "src/popup.h", "src/faction.h"], "reason": "Recover the mod-aware shared X_pop_ask core through typed faction, popup-file, and popup-dialog owners."}
// symbol    ?X_pop_ask@@YAHPADPBDH0P6AHXZH@Z
int __cdecl X_pop_ask(char *caption, const char *label, int value,
                      char *text, int (__cdecl *callback)(), int flags) {
    char *const section = const_cast<char *>(label);
    if (std::strcmp(caption, XPopModCheckString1()) != 0 &&
        std::strcmp(caption, XPopModCheckString2Text) != 0) {
        return pop_ask(caption, section, value, text, callback, flags);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        if (ExpansionEnabled != 0) {
            FILE *const file = fopen(XPopModCheckFile1Text,
                                          XPopModCheckMode1Text);
            if (file != nullptr) {
                fclose(file);
                const int result = pop_ask(PopupStartAltFile1, section, value,
                                           text, callback, flags);
                if (result != -1) return result;
            }
        }
    } else if (ExpansionEnabled != 0 && PopupDialogFactionID > 0 &&
               PopupDialogFactionID < 8 &&
               (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) != 0) {
        FILE *const file = fopen(XPopModCheckFile2Text,
                                      XPopModCheckMode2Text);
        if (file != nullptr) {
            fclose(file);
            const int result = pop_ask(PopupStartAltFile2, section, value,
                                       text, callback, flags);
            if (result != -1) return result;
        }
    }

    return pop_ask(caption, section, value, text, callback, flags);
}

/*
Purpose: The three-argument numeric-entry popup form: caption from the shared
         buffer, value and poll callback as given.
// ORIGINAL: 0x005BFD10 ?X_pop_ask_number@@YAHPBDHP6AHXZ@Z 0x005BFD10-0x005BFD2E BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/005bfd10.cpp on the same terms as
//        0x005BF310 above - the claim proved the artifact and nothing in the
//        shipped program owned the address. DipEdit::on_button_clicked is the
//        compiled caller that needed it (the integrity prompt).
// size      30 bytes
// kind      game
// calls     0x005BFD70
Return Value: whatever the four-argument form returns
Status: Complete
*/
int __cdecl X_pop_ask_number(const char *label, int value,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(PopupStartCaption, label, value, callback);
}

/*
Purpose: The caption-taking numeric-entry form with the value defaulted.
         Preserve the supplied caption and use the original zero default.
// size      27 bytes
// kind      game
Return Value: whatever the four-argument form returns
Status: Complete
*/
// ORIGINAL: 0x005BFD30 ?X_pop_ask_number@@YAHPADPBDP6AHXZ@Z 0x005BFD30-0x005BFD4B BYTE_EXACT
// symbol    ?X_pop_ask_number@@YAHPADPBDP6AHXZ@Z
int __cdecl X_pop_ask_number(char *caption, const char *label,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(caption, label, 0, callback);
}

// Numeric-entry convenience form: shared caption and initial value zero.
// ORIGINAL: 0x005BFD50 ?X_pop_ask_number@@YAHPBDP6AHXZ@Z 0x005BFD50-0x005BFD6C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/005bfd50.cpp", "src/xpops.h", "src/popup.h"], "reason": "Recover the shared-caption numeric X_pop_ask_number convenience overload with its original zero default."}
int __cdecl X_pop_ask_number(const char *label,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(PopupStartCaption, label, 0, callback);
}

// Full numeric-entry routing mirrors X_pop_ask's alien-script fallback,
// with the alternate file/caption pair selected by the same faction gates.
// ORIGINAL: 0x005BFD70 ?X_pop_ask_number@@YAHPADPBDHP6AHXZ@Z 0x005BFD70-0x005BFE8E
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005bfd70.cpp", "src/xpops.h", "src/popup.h", "src/faction.h"], "reason": "Recover numeric X_pop_ask_number mod-caption and alien-script routing using typed popup and faction state."}
int __cdecl X_pop_ask_number(char *caption, const char *label, int value,
                             int (__cdecl *callback)()) {
    char *const section = const_cast<char *>(label);
    if (std::strcmp(caption, XPopModCheckString1()) != 0 &&
        std::strcmp(caption, XPopModCheckString2Text) != 0) {
        return pop_ask_number(caption, section, value, callback);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) == 0) {
        if (ExpansionEnabled != 0 && PopupDialogFactionID > 0 &&
            PopupDialogFactionID < 8 &&
            (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) != 0) {
            FILE *const file = fopen(XPopModCheckFile2Text,
                                          XPopModCheckMode2Text);
            if (file != nullptr) {
                fclose(file);
                const int result = pop_ask_number(PopupStartAltFile2, section,
                                                  value, callback);
                if (result != -1) return result;
            }
        }
    } else if (ExpansionEnabled != 0) {
        FILE *const file = fopen(XPopModCheckFile1Text,
                                      XPopModCheckMode1Text);
        if (file != nullptr) {
            fclose(file);
            const int result = pop_ask_number(PopupStartAltFile1, section,
                                              value, callback);
            if (result != -1) return result;
        }
    }

    return pop_ask_number(caption, section, value, callback);
}
