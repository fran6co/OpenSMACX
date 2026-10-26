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
#include "interface_fonts.h"
#include <shellapi.h>
#include <mmsystem.h>
#include <cstring>
#include "popup.h"
#include "popup_fonts.h"
#include "buffer.h"
#include "diplopop.h"
#include "main.h"
#include "wave.h"
#include "fx.h"
#include "ambience.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "time.h"
#include "strings.h"
#include "mapwin.h"
#include "messagewin.h"
#include "base.h"
#include "lock.h"
#include "map.h"
#include "veh.h"
#include "alpha.h"
#include "xpops.h"
#include "console.h"
#include "game.h"
#include "interface_palette.h"
#include "palette.h"
#include "general.h"
#include "faction.h"

// The moved text overload calls the CRT length routine in the original,
// rather than expanding the compiler intrinsic. Keep the ordinary C API spelling.
#pragma function(strlen)

// The moved numeric prompt calls strcat out of line; retain the ordinary
// CRT API call rather than VC6's intrinsic expansion.
#pragma function(strcat)


/*
Purpose: The most minimal pop() form - just a label and callback; shared
         caption buffer, value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
// ORIGINAL: 0x00627130 ?pop@@YAHPADP6AHXZ@Z 0x00627130-0x00627154 BYTE_EXACT
// symbol    ?pop_label_cb@@YAHPADP6AHXZ@Z
// size      36 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_label_cb(char *label, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, nullptr,
                            1, 1, callback);
}
/*
Purpose: Like pop_label_cb but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
// ORIGINAL: 0x00627160 ?pop@@YAHPADPADP6AHXZ@Z 0x00627160-0x00627184 BYTE_EXACT
// symbol    ?pop_caption@@YAHPAD0P6AHXZ@Z
// size      36 bytes
// prototype 
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption(char *caption, char *label,
                        int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, nullptr, 1, 1,
                            callback);
}
/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer and caller-supplied value and title; no override text, both
         flags fixed at 1.
// ORIGINAL: 0x006271D0 ?pop@@YAHPADHHP6AHXZ@Z 0x006271D0-0x006271FA BYTE_EXACT
// symbol    ?pop_value_title@@YAHPADHHP6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_title(char *label, int value, int title,
                            int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            nullptr, 1, 1, callback);
}
/*
Purpose: Like pop_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, null sprite, both flags fixed
         at 1.
// ORIGINAL: 0x00627200 ?pop@@YAHPADPADHHP6AHXZ@Z 0x00627200-0x0062722A BYTE_EXACT
// symbol    ?pop_caption_value_title@@YAHPAD0HHP6AHXZ@Z
// size      42 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_value_title(char *caption, char *label, int value,
                                    int title, int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, nullptr, 1,
                            1, callback);
}
/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer, value defaulted to -1 and only the title exposed; no
         override text, both flags fixed at 1.
// ORIGINAL: 0x00627230 ?pop@@YAHPADHP6AHXZ@Z 0x00627230-0x00627257 BYTE_EXACT
// symbol    ?pop_title@@YAHPADHP6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_title(char *label, int title, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            nullptr, 1, 1, callback);
}
/*
Purpose: Like pop_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, null sprite, both flags
         fixed at 1.
// ORIGINAL: 0x00627260 ?pop@@YAHPADPADHP6AHXZ@Z 0x00627260-0x00627287 BYTE_EXACT
// symbol    ?pop_caption_title@@YAHPAD0HP6AHXZ@Z
// size      39 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_title(char *caption, char *label, int title,
                              int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, nullptr, 1, 1,
                            callback);
}
/*
Purpose: Raise the caption-less, sprite-less popup against the shared
         caption buffer with value, override text and title all
         caller-supplied; both flags fixed at 1.
// ORIGINAL: 0x00627290 ?pop@@YAHPADHPADHP6AHXZ@Z 0x00627290-0x006272BD BYTE_EXACT
// symbol    ?pop_value_text_title@@YAHPADH0HP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_text_title(char *label, int value, char *text,
                                 int title, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            nullptr, 1, 1, callback);
}
/*
Purpose: The full caption-less, sprite-less popup form - caption, value,
         override text and title all caller-supplied; null sprite, both
         flags fixed at 1.
// ORIGINAL: 0x006272C0 ?pop@@YAHPADPADHPADHP6AHXZ@Z 0x006272C0-0x006272ED BYTE_EXACT
// symbol    ?pop_full@@YAHPAD0H0HP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_full(char *caption, char *label, int value, char *text,
                     int title, int (__cdecl *callback)()) {
    return pops(caption, label, value, text, title, nullptr, 1, 1,
                            callback);
}
/*
Purpose: The most minimal pops() form - label, sprite and callback; shared
         caption buffer, value -1, no override text, title 0, both flags
         fixed at 1.
// ORIGINAL: 0x00627310 ?pops@@YAHPADPAUSprite@@P6AHXZ@Z 0x00627310-0x00627337 BYTE_EXACT
// symbol    ?pops_minimal@@YAHPADPAVSprite@@P6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_minimal(char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            1, 1, callback);
}
/*
Purpose: Like pops_minimal but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text,
         title 0.
// ORIGINAL: 0x00627340 ?pops@@YAHPADPAUSprite@@HHP6AHXZ@Z 0x00627340-0x0062736D BYTE_EXACT
// symbol    ?pops_flags@@YAHPADPAVSprite@@HHP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_flags(char *label, Sprite *sprite, int flag_a, int flag_b,
                       int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            flag_a, flag_b, callback);
}
/*
Purpose: Like pops_minimal but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, title 0, both flags
         fixed at 1.
// ORIGINAL: 0x00627370 ?pops@@YAHPADPADPAUSprite@@P6AHXZ@Z 0x00627370-0x00627397 BYTE_EXACT
// symbol    ?pops_caption@@YAHPAD0PAVSprite@@P6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption(char *caption, char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, sprite, 1, 1,
                            callback);
}
/*
Purpose: Like pops_flags but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0.
// ORIGINAL: 0x006273A0 ?pops@@YAHPADPADPAUSprite@@HHP6AHXZ@Z 0x006273A0-0x006273CD BYTE_EXACT
// symbol    ?pops_caption_flags@@YAHPAD0PAVSprite@@HHP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_flags(char *caption, char *label, Sprite *sprite,
                               int flag_a, int flag_b,
                               int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, 0, sprite, flag_a,
                            flag_b, callback);
}
/*
Purpose: Raise the full popup against the shared caption buffer with value
         and title exposed and the sprite caller-supplied; no override
         text, both flags fixed at 1.
// ORIGINAL: 0x006273D0 ?pops@@YAHPADHHPAUSprite@@P6AHXZ@Z 0x006273D0-0x006273FD BYTE_EXACT
// symbol    ?pops_value_title@@YAHPADHHPAVSprite@@P6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title(char *label, int value, int title,
                             Sprite *sprite, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            sprite, 1, 1, callback);
}
/*
Purpose: Like pops_value_title but with both flags caller-supplied instead
         of fixed at 1; shared caption buffer, no override text.
// ORIGINAL: 0x00627400 ?pops@@YAHPADHHPAUSprite@@HHP6AHXZ@Z 0x00627400-0x00627433 BYTE_EXACT
// symbol    ?pops_value_title_flags@@YAHPADHHPAVSprite@@HHP6AHXZ@Z
// size      51 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title_flags(char *label, int value, int title,
                                   Sprite *sprite, int flag_a, int flag_b,
                                   int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}
/*
Purpose: Like pops_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, both flags fixed at 1.
// ORIGINAL: 0x006274D0 ?pops@@YAHPADPADHHPAUSprite@@P6AHXZ@Z 0x006274D0-0x006274FD BYTE_EXACT
// symbol    ?pops_caption_value_title@@YAHPAD0HHPAVSprite@@P6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_value_title(char *caption, char *label, int value,
                                     int title, Sprite *sprite,
                                     int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, sprite, 1,
                            1, callback);
}
/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, value, title, sprite and both flags.
// ORIGINAL: 0x00627500 ?pops@@YAHPADPADHHPAUSprite@@HHP6AHXZ@Z 0x00627500-0x00627533 BYTE_EXACT
// symbol    ?pops_no_text@@YAHPAD0HHPAVSprite@@HHP6AHXZ@Z
// size      51 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_text(char *caption, char *label, int value, int title,
                         Sprite *sprite, int flag_a, int flag_b,
                         int (__cdecl *callback)()) {
    return pops(caption, label, value, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}
/*
Purpose: Raise the full popup against the shared caption buffer with value
         defaulted to -1 and only the title and sprite exposed; no override
         text, both flags fixed at 1.
// ORIGINAL: 0x00627540 ?pops@@YAHPADHPAUSprite@@P6AHXZ@Z 0x00627540-0x0062756A BYTE_EXACT
// symbol    ?pops_title@@YAHPADHPAVSprite@@P6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title(char *label, int title, Sprite *sprite,
                       int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            sprite, 1, 1, callback);
}
/*
Purpose: Like pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text.
// ORIGINAL: 0x00627570 ?pops@@YAHPADHPAUSprite@@HHP6AHXZ@Z 0x00627570-0x006275A0 BYTE_EXACT
// symbol    ?pops_title_flags@@YAHPADHPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title_flags(char *label, int title, Sprite *sprite,
                             int flag_a, int flag_b,
                             int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, -1, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}
/*
Purpose: Like pops_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, both flags fixed at 1.
// ORIGINAL: 0x006275A0 ?pops@@YAHPADPADHPAUSprite@@P6AHXZ@Z 0x006275A0-0x006275CA BYTE_EXACT
// symbol    ?pops_caption_title@@YAHPAD0HPAVSprite@@P6AHXZ@Z
// size      42 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title(char *caption, char *label, int title,
                               Sprite *sprite, int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, sprite, 1, 1,
                            callback);
}
/*
Purpose: Like pops_caption_title but with both flags caller-supplied instead
         of fixed at 1; value -1, no override text.
// ORIGINAL: 0x006275D0 ?pops@@YAHPADPADHPAUSprite@@HHP6AHXZ@Z 0x006275D0-0x00627600 BYTE_EXACT
// symbol    ?pops_caption_title_flags@@YAHPAD0HPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title_flags(char *caption, char *label, int title,
                                     Sprite *sprite, int flag_a, int flag_b,
                                     int (__cdecl *callback)()) {
    return pops(caption, label, -1, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}
/*
Purpose: Raise the full popup against the shared caption buffer with value,
         override text, title and sprite all caller-supplied; both flags
         fixed at 1.
// ORIGINAL: 0x00627600 ?pops@@YAHPADHPADHPAUSprite@@P6AHXZ@Z 0x00627600-0x00627630 BYTE_EXACT
// symbol    ?pops_value_text_title@@YAHPADH0HPAVSprite@@P6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_text_title(char *label, int value, char *text,
                                  int title, Sprite *sprite,
                                  int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            sprite, 1, 1, callback);
}
/*
Purpose: Like pops_value_text_title but with both flags caller-supplied
         instead of fixed at 1; only the caption defaults to the shared
         buffer.
// ORIGINAL: 0x00627630 ?pops@@YAHPADHPADHPAUSprite@@HHP6AHXZ@Z 0x00627630-0x00627666 BYTE_EXACT
// symbol    ?pops_default_caption@@YAHPADH0HPAVSprite@@HHP6AHXZ@Z
// size      54 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_default_caption(char *label, int value, char *text,
                                 int title, Sprite *sprite, int flag_a,
                                 int flag_b, int (__cdecl *callback)()) {
    return pops(PopupStartCaption, label, value, text, title,
                            sprite, flag_a, flag_b, callback);
}
/*
Purpose: Raise the full popup with everything but the flags caller-supplied
         - caption, value, override text, title and sprite; both flags
         fixed at 1.
// ORIGINAL: 0x00627670 ?pops@@YAHPADPADHPADHPAUSprite@@P6AHXZ@Z 0x00627670-0x006276A0 BYTE_EXACT
// symbol    ?pops_no_flags@@YAHPAD0H0HPAVSprite@@P6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006276A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_flags(char *caption, char *label, int value, char *text,
                          int title, Sprite *sprite,
                          int (__cdecl *callback)()) {
    return pops(caption, label, value, text, title, sprite, 1, 1,
                            callback);
}
// WITHDRAWN 2026-08-22. This note used to say the runtime walk stops here
// because "DirectDraw FAILS under wine", that the error path into
// `report_error` -> `pops` was therefore faithful, and that no further source
// would move it. ALL THREE CLAIMS WERE WRONG, and they were wrong because the
// executable was being launched incorrectly, not because of anything here.
//
// Run with the project's own wine prefix, inside a wine virtual desktop, from
// `.opensmacx/game` - BOTH the shipped image and
// this build run with zero page faults, repeatably:
//
//   cd .opensmacx/game
//   WINEPREFIX=$HOME/opt/vc6/.wineprefix DISPLAY=:1 wine explorer /desktop=smac,1024x768 <exe>
//
// Under the DEFAULT prefix with no virtual desktop, the SHIPPED IMAGE faults
// identically - at 0x005F7E90, its own `Win::is_visible`, reading 0x9C off a
// null popup slot. So the crash was never evidence about this recovery at
// all; it reproduced on the original just as readily.
//
// Two singleton BasePop instances, allocated once by BasePop::init_class()
// (basepop.cpp, which stores the same two objects under the names
// PopupInstanceSlotA/B - one storage, one name) and chosen between here by
// which one is not already on screen. The slots are real pointer variables
// now: in the image they are the two zero-initialised pointer dwords at
// 0x009BC074/78, and the old `int *const` bindings into those addresses read
// and wrote unmapped memory in the standalone build - the DDRAWERROR path
// dereferenced slot A's garbage outright.
// The screen location saved across one call to `pops()` and restored on the
// next, then reset to the sentinel 0x2000 (BasePop::set_loc's own "let the
// popup centre itself" value elsewhere in this file). The image's .data
// carries 0x2000 in both dwords, so the objects initialise from it.
// RECOVERY_OBJECT: v1 0x006973DC _PopupSavedLocX
static int PopupSavedLocX = 0x2000;  // 0x006973DC
// RECOVERY_OBJECT: v1 0x006973E0 _PopupSavedLocY
static int PopupSavedLocY = 0x2000;  // 0x006973E0

// The `a5 & 0x8000` tail's rectangle: BasePop's field_30B4_/_30B8_ are an
// origin, field_30BC_/_30C0_ a width and height added to that origin to
// close the rectangle. Inferred from the arithmetic pattern alone.
// Zero-initialised .bss in the image at 0x009BC058..0x009BC064.
static int PopupCloseRectLeft;    // 0x009BC058
static int PopupCloseRectTop;     // 0x009BC05C
static int PopupCloseRectRight;   // 0x009BC060
static int PopupCloseRectBottom;  // 0x009BC064
/*
// ORIGINAL: 0x006276A0 ?pops@@YAHPADPADHPADHPAUSprite@@HHP6AHXZ@Z 0x006276A0-0x006277F9
// TRIED: the pending_bodies scaffold this replaces had the two start()/sprite() success checks inverted (`== 0` where the image's `je` jumps to the CONTINUE label when the callee returns 0, i.e. success is 0 and only a nonzero result is an error), and folded read_check()'s return value into the *(self+0x3100) store instead of keeping them as the two separate globals (0x9BC06C and 0x9BC070) the image writes.
// TRIED: win-selection guard reads `*PopupInstanceSlotB` as the DEFAULT and only re-reads `*PopupInstanceSlotA` inside the `!is_visible()` branch (never binding `*PopupInstanceSlotA` to a local kept live across the call) - 91/110 -> 98/110. The `field_2274_` ternary needed its arms swapped (`== 0` first) to match the image placing the ELSE body as the fallthrough. The 0x8000 tail's four field reads declared in REVERSE offset order (0x30C0 down to 0x30B4, matching the image's load order) fixed its register choice - 87/110 -> 91/110. Plateaued at 98/110: one `add` operand order in that same tail and the prologue's exact register pick for the is_visible receiver did not yield to further reordering attempts.
// LEVER (2026-08-29): SAVE THE FLAG, THEN READ THE DEFAULT - `const BOOL winVisible = ...SlotA->is_visible(); Win *win = *SlotB; if (!winVisible) win = *SlotA;` puts the SlotB load AFTER the call like the image (0x006276AE), 98/110 -> 101/110.
// TRIED (2026-08-29): positive if/else and `c ? B : A` ternary for the win selection - both grow the `jmp` the image does not have (11/110); `if (sprite)` vs `if (sprite != 0)` identical.
// TRIED (2026-08-29): the last two clusters are scheduler placements no source shape reached. (a) The image tests the sprite pointer BEFORE the two 0x2000 stores (0x00627709); ours stores first - the branch is textually after the stores in every structured spelling. (b) The image computes w+x (into w's register edx) BEFORE the Left store; VC6 here sinks w+x below Left and Bottom, which COMMUTES the add into x's register (`add esi,edx`) because x is dead by then - named sums in the image's compute order rescheduled identically. --all-flags: 101/110 is the best across all 20 sets (the /O2 non-/Oy- family ties).
// TRIED (2026-09-04): spelling the Right sum `x + w` to give the add x's register as the DESTINATION instead - measured 103 of 110, IDENTICAL to `w + x`, still first-diverging at image instruction 86 on the same `add edx,esi` against our `add esi,edx`. (b) above still stands; 103/110 is today's census of this body, the two notes above were written at 98 and 101.
// LEVER (2026-08-29, slot conversion): the two instance slots as real `Win *`
// objects (basepop.h/basepop.cpp) instead of `int *const` bindings into
// 0x9BC074/78 - the identical folded loads, and 101/110 -> 103/110 (0.982
// similar, /O2 non-/Oy-): the win-selection `je` now agrees and the first
// divergence is the 0x8000 tail's add commute. Saved locs, write-check state
// and close rect converted the same way; every verdict on this page
// re-measured before and after, no other tier moved.
// symbol    ?pops@@YAHPAD0H0HPAVSprite@@HHP6AHXZ@Z
// size      345 bytes
// prototype
// callers   ?   call targets   9
// kind      game
// calls     0x005F7E90 0x00601BF0 0x00601B80 0x006108E0 0x00601BB0 0x00602600
//           0x005FCBB0 0x00601BD0 0x00600F00
Status: Complete - testing
*/
int __cdecl pops(char *caption, char *label, int value, char *override_text,
                 int flags, Sprite *sprite, int pair_value_a, int pair_value_b,
                 int (__cdecl *callback)()) {
    // SAVE THE FLAG, THEN READ THE DEFAULT: the image's order is
    // `mov ecx,[SlotA]; call is_visible; mov esi,[SlotB]; test eax,eax;
    // jne +6; mov esi,[SlotA]` - the SlotB default load sits AFTER the call,
    // so the visibility result has to be bound to a named local BEFORE the
    // default is read, with the guard testing the saved value. Reading the
    // default first hoisted its load above the call (98/110); if/else and
    // ternary spellings both grew a `jmp` the image does not have.
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    if (pop->start(caption, label, value, override_text, flags, 0)) {
        return -1;
    }

    pop->set_loc(PopupSavedLocX, PopupSavedLocY);
    PopupSavedLocY = 0x2000;
    PopupSavedLocX = 0x2000;

    if (sprite) {
        if (pair_value_a != pair_value_b) {
            pop->field_3104_ = pair_value_a;
            pop->field_3108_ = pair_value_b;
        }
        if (pop->dialogs_.list_.entry_count_ == 0) {
            pop->caller_sprite_ = sprite;
        } else {
            pop->caller_sprite_ = 0;
        }

        if (pop->dialogs().sprite(sprite, 0, 0)) {
            return -1;
        }
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);

    if (pop->external_window_ == 0) {
        do_all_draws();
    }

    if (flags & 0x8000) {
        int h = pop->field_30C0_;
        int w = pop->field_30BC_;
        int y = pop->field_30B8_;
        int x = pop->field_30B4_;
        // NAMED SUMS IN THE IMAGE'S COMPUTE ORDER: the image adds w+x
        // (0x006277B4, into w's register) BEFORE y+h, and only then does it
        // store Left, Bottom, and finally Right - sunk past the pops. The
        // mutate-in-place spelling computed w+x last and commuted the add
        // into x's register.
        PopupCloseRectTop = y;
        const int closeRectRight = w + x;
        const int closeRectBottom = y + h;
        PopupCloseRectLeft = x;
        PopupCloseRectBottom = closeRectBottom;
        PopupCloseRectRight = closeRectRight;
        return execResult;
    }

    PopupWriteCheckState = pop->read_check();
    ConsoleGroupAction = pop->field_3100_;
    if ((flags & 0x300) == 0) {
        pop->close();
    }
    return execResult;
}

// ORIGINAL: 0x00627910 ?pop_ask@@YAHPADPADHPADP6AHXZH@Z 0x00627910-0x006279B7 FILE BYTE_EXACT
// symbol    ?pop_ask@@YAHPAD0H0P6AHXZH@Z
// working copy - scaffold materialised by --work
// size      167 bytes
// prototype int (__cdecl ?pop_ask@@YAHPADHPADP6AHXZH@Z)(int8* srcFileID, int8* sectionID, int, int8*, int (__cdecl *)(), int)
// callers   6   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90 0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600
//
// PRESERVED UNIT - scaffold preamble stripped by hand 2026-09-06: the body
// below is spelled against the real BasePop / Popup declarations and the
// tree's own named globals, so the homing carries no fixed-slot bindings and
// no cast. The +0x229c read is Dialogs' Dialog virtual base's
// item_list_.entry_count_, reached through the member the way
// BasePop::on_key_down reaches it.
// symbol    (above) the catalogue spells every PAD out; VC6 emits the `0`
//   back-reference for the repeated char * and that is the name this tree's
//   object really carries. Nothing about the parameters differs.

int __cdecl pop_ask(char *text_file, char *text_label, int value,
                    char *override_text, int (__cdecl *callback)(),
                    int flags) {
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    if (pop->start(text_file, text_label, value, override_text, flags | 0x44,
                   0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    if (pop->dialogs().Dialog::item_list_.entry_count_ == 0) {
        return -1;
    }

    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    pop->close();
    return execResult;
}

// ORIGINAL: 0x00627830 ?pop_ask@@YAHPADPADHP6AHXZH@Z 0x00627830-0x00627854 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["the complete original body forwards arguments 1-3 and 4-5 unchanged and supplies null for the six-argument implementation's override-text slot", "src/popup_api.cpp owns the matching six-argument pop_ask implementation and the other convenience overloads", "no address call, base-slot change, or semantic normalization is required"], "reason": "Move the original five-argument pop_ask overload beside the existing six-argument implementation it forwards to, preserving the null override-text argument and return value."}
// symbol    ?pop_ask@@YAHPAD0HP6AHXZH@Z
// size      36 bytes
// prototype int (__cdecl ?pop_ask@@YAHPADPADHP6AHXZH@Z)(int8* srcFileID, int8* sectionID, int, int (__cdecl *)(), int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627910
// historical Status: Complete
int __cdecl pop_ask(char *a1, char *a2, int a3, int (__cdecl *a4)(), int a5) {
    return pop_ask(a1, a2, a3, 0, a4, a5);
}

// The short popup overloads supply the process-wide default text file and
// delegate to the six-argument implementation above. Their original bodies
// contain no popup lifetime of their own.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627800 ?pop_ask@@YAHPADHPADP6AHXZH@Z 0x00627800-0x00627827 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00627800.cpp; image:0x00627800-0x00627827; source:src/popup_api.cpp:pop_ask"], "reason": "Original forwarding wrapper passes the process-wide PopupStartCaption and all five input arguments unchanged to the six-argument pop_ask; this is the matching product overload."}
// symbol    ?pop_ask@@YAHPADH0P6AHXZH@Z
int __cdecl pop_ask(char *section, int value, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    return pop_ask(PopupStartCaption, section, value, override_text,
                   callback, flags);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627860 ?pop_ask@@YAHPADHP6AHXZH@Z 0x00627860-0x00627884 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00627860.cpp; image:0x00627860-0x00627884; source:src/popup_api.cpp:pop_ask"], "reason": "Original forwarding wrapper supplies PopupStartCaption and a null override-text argument to the six-argument pop_ask; this is the matching product overload."}
int __cdecl pop_ask(char *section, int value,
                    int (__cdecl *callback)(), int flags) {
    return pop_ask(PopupStartCaption, section, value, nullptr,
                   callback, flags);
}

// Text-taking overloads reject a null pointer before computing its length;
// the shorter one supplies the same process-wide default text file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00627890 ?pop_ask@@YAHPADPADPADP6AHXZH@Z 0x00627890-0x006278C4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/00627890.cpp; image:0x00627890-0x006278C4; source:src/popup_api.cpp:pop_ask"], "reason": "Original text-taking popup overload returns -1 on null text, otherwise passes strlen(text) and all caller arguments to the six-argument implementation."}
// symbol    ?pop_ask@@YAHPAD00P6AHXZH@Z
int __cdecl pop_ask(char *text_file, char *section, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    if (override_text == nullptr) {
        return -1;
    }
    return pop_ask(text_file, section, static_cast<int>(strlen(override_text)),
                   override_text, callback, flags);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006278D0 ?pop_ask@@YAHPADPADP6AHXZH@Z 0x006278D0-0x00627904 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/006278d0.cpp; image:0x006278D0-0x00627904; source:src/popup_api.cpp:pop_ask"], "reason": "Original short text-taking popup overload returns -1 on null text, otherwise supplies PopupStartCaption, strlen(text), and caller arguments to the six-argument implementation."}
// symbol    ?pop_ask@@YAHPAD0P6AHXZH@Z
int __cdecl pop_ask(char *section, char *override_text,
                    int (__cdecl *callback)(), int flags) {
    if (override_text == nullptr) {
        return -1;
    }
    return pop_ask(PopupStartCaption, section,
                   static_cast<int>(strlen(override_text)), override_text,
                   callback, flags);
}

// ORIGINAL: 0x006279C0 ?pop_ask_number@@YAHPADHP6AHXZ@Z 0x006279C0-0x00627A8B FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      203 bytes
// prototype int (__cdecl ?pop_ask_number@@YAHPADHP6AHXZ@Z)(int8* sectionID, int, int (__cdecl *)())
// callers   0   call targets   9
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600 0x00645470
//           0x0064FC88
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Popup
// declarations and the tree's own named globals, so the homing carries no
// fixed-slot bindings and no cast.

int __cdecl pop_ask_number(char *label, int initial, int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    StringTemp[0] = 0;
    _itoa(initial, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(PopupStartCaption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// Caption-and-label numeric prompt. The other numeric forms intentionally keep
// their own original identities; this body uses the caller's caption rather
// than the shared default and retains the same popup state/lifetime path.
// ORIGINAL: 0x00627C30 ?pop_ask_number@@YAHPADPADHP6AHXZ@Z 0x00627C30-0x00627CFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f34af2cde626ce68311a71f2a29818e510298d7:src/unrecovered/00627c30.cpp"], "reason": "Home the original caption-and-label numeric popup in its typed popup owner; the X_pop_ask_number routing requires both arguments and this independent original span is established by the retained seed."}
// symbol    ?pop_ask_number@@YAHPAD0HP6AHXZ@Z
int __cdecl pop_ask_number(char *caption, char *label, int initial,
                           int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    StringTemp[0] = 0;
    _itoa(initial, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(caption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// ORIGINAL: 0x00627B60 ?pop_ask_number@@YAHPADP6AHXZ@Z 0x00627B60-0x00627C28 FILE BYTE_EXACT
// size      200 bytes
// prototype
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FCBB0 0x00600F00 0x00601BB0 0x00601BF0 0x00602600 0x00645470
//           0x0064FC88
//
// PRESERVED UNIT - measured MISMATCH. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real BasePop / Popup
// declarations and the tree's own named globals (StringTemp, ParseNumTable,
// ParseStrBuffer, PopupWriteCheckState, BaseAtKeyPollFlag), so the homing
// carries no fixed-slot bindings and no cast. The artifact transcription this
// replaces passed the CALLBACK as start's label argument; the image reads the
// label from [esp+0x6c], which is the first parameter, so a1 it is.

int __cdecl pop_ask_number(char *label, int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    StringTemp[0] = 0;
    _itoa(0, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(PopupStartCaption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// ORIGINAL: 0x006272F0 ?pop_close@@YAXXZ 0x006272F0-0x00627306 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned6272F0..627306; retained git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/recovered/006272f0.cpp historical byte obligation.", "PopupInstanceSlotA/B are the existing init_class-owned singleton pointer cells9BC074/78; BasePop derives through GraphicWin from the nonvirtual Win base."], "reason": "Close both source-owned singleton popups through the genuine BasePop receiver; retain the original sequential reload across the first close callback."}
void __cdecl pop_close() {
    PopupInstanceSlotA->close();
    PopupInstanceSlotB->close();
}

// ORIGINAL: 0x00627190 ?pop@@YAHPADPADPADHP6AHXZ@Z 0x00627190-0x006271CA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627190-0x006271CA", "source:src/popup_api.cpp:typed owner", "source:src/popup_api.h:nine-argument pops core", "source:src/popup_api.cpp:existing popup implementation", "seed:retained original source identity 0x00627190"], "reason": "The pop convenience wrapper preserves the original null-text result and forwards strlen plus the native default sprite and flags to the real popup builder."}
// symbol    ?pop@@YAHPAD00HP6AHXZ@Z
int __cdecl pop(char *caption, char *label, char *text, int value,
                int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    const int text_length = static_cast<int>(std::strlen(text));
    return pops(caption, label, text_length, text, value, 0, 1, 1,
                callback);
}

// ORIGINAL: 0x00627440 ?pops@@YAHPADPADPADHPAUSprite@@P6AHXZ@Z 0x00627440-0x0062747D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627440-0x0062747D", "source:src/popup_api.cpp:typed owner", "source:src/popup_api.h:nine-argument pops core", "source:src/popup_api.cpp:existing popup implementation", "seed:retained original source identity 0x00627440"], "reason": "The sprite-bearing text overload preserves the null-text return and forwards its measured length with the original default flag pair."}
// symbol    ?pops@@YAHPAD00HPAVSprite@@P6AHXZ@Z
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    const int text_length = static_cast<int>(std::strlen(text));
    return pops(caption, label, text_length, text, value, sprite, 1, 1,
                callback);
}

// ORIGINAL: 0x00627480 ?pops@@YAHPADPADPADHPAUSprite@@HHP6AHXZ@Z 0x00627480-0x006274C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627480-0x006274C3", "source:src/popup_api.cpp:typed owner", "source:src/popup_api.h:nine-argument pops core", "source:src/popup_api.cpp:existing popup implementation", "seed:retained original source identity 0x00627480"], "reason": "The sprite-and-flags text overload forwards the measured text length and both caller flags without changing callback or sprite ownership."}
// symbol    ?pops@@YAHPAD00HPAVSprite@@HHP6AHXZ@Z
int __cdecl pops(char *caption, char *label, char *text, int value,
                 Sprite *sprite, int flag_a, int flag_b,
                 int (__cdecl *callback)()) {
    if (text == 0) {
        return -1;
    }
    return pops(caption, label, static_cast<int>(std::strlen(text)), text, value, sprite,
                flag_a, flag_b, callback);
}

// ORIGINAL: 0x00627A90 ?pop_ask_number@@YAHPADPADP6AHXZ@Z 0x00627A90-0x00627B58 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x00627A90-0x00627B58", "source:src/basepop.h:BasePop hierarchy and popup instance slots", "source:src/popup_api.cpp:existing numeric popup lifecycle", "seed:retained original source identity 0x00627A90"], "reason": "Recover the complete zero-default numeric prompt through the real BasePop owner, preserving window selection, startup failure, callback execution, conditional redraw, parsed result and close order."}
// symbol    ?pop_ask_number@@YAHPAD0P6AHXZ@Z
int __cdecl pop_ask_number(char *caption, char *label,
                           int (__cdecl *callback)()) {
    char number_text[80];
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    BasePop *pop = PopupInstanceSlotB;
    if (!winVisible) {
        pop = PopupInstanceSlotA;
    }

    StringTemp[0] = 0;
    _itoa(0, number_text, 10);
    strcat(StringTemp, number_text);

    if (pop->start(caption, label, 9, StringTemp, 0x44, 0)) {
        return -1;
    }
    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);
    if (pop->external_window_ == 0) {
        do_all_draws();
    }
    ConsoleGroupAction = pop->field_3100_;
    ParseNumTable[0] = atoi(ParseStrBuffer[0].str);
    pop->close();
    return execResult;
}

// Original627110, alongside the saved-location objects in this API owner.
// ORIGINAL: 0x00627110 sub_627110 0x00627110-0x00627124 BYTE_EXACT
// symbol    ?set_popup_saved_location@@YAHHH@Z
// kind      game
int __cdecl set_popup_saved_location(int x, int y) {
    PopupSavedLocX = x;
    PopupSavedLocY = y;
    return x;
}
