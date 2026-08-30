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
#include "original_seam.h"
#include "popup.h"
#include "wave.h"
#include "fx.h"
#include "win.h"
#include "spritebox.h"
#include "net_class.h"
#include "temp.h"


/*
Purpose: Close the popup - its scroll bar first, then the popup base.
// ORIGINAL: 0x00404900 ?close@Popup@@QAEXXZ 0x00404900-0x00404917 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?close@Popup@@QAEXXZ)(Popup* this)
// callers   104   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00600F00 0x00605370
Return Value: n/a
Status: Complete
*/
void Popup::close() {
    scroll_.close();
    BasePop::close();
}



/*
Purpose: The five-argument start form, forwarding to the six-argument one with
         no parent graphic window.
// ORIGINAL: 0x0043EF70 ?start@Popup@@QAEXPADPBDHPADH@Z 0x0043EF70-0x0043EF92 BYTE_EXACT
// symbol    ?start@Popup@@QAEXPADPBDH0H@Z
// size      34 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPADPBDHPADH@Z)(Popup* this, int8* srcFileID, int8* sectionID, int, int8*, int)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(char *a1, const char *a2, int a3, char *a4, int a5) {
    start(a1, a2, a3, a4, a5, nullptr);
}


// AN ARRAY, NOT A POINTER. 0x009B8AA8 is in .bss - zero-filled, in no
// file section - and the image pushes its ADDRESS as an immediate
// (`push 0x9b8aa8`), which is a decayed array. Through a `char *`
// variable it is a load and a push instead.
// EXTENT IS A FLOOR: nothing recovered bounds the writes into it, so
// 256 is this tree's usual caption size rather than a measured one.
char PopupStartCaption[256];  // 0x009B8AA8

/*
Purpose: Start a popup from just a label, defaulting the rest.
// ORIGINAL: 0x005A5990 ?start@Popup@@QAEXPBD@Z 0x005A5990-0x005A59AD BYTE_EXACT
// size      29 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPBD@Z)(Popup* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label) {
    start(PopupStartCaption, label, -1, nullptr, 0, nullptr);
}

/*
Purpose: Start a popup from a label and a value, defaulting the rest.
// ORIGINAL: 0x00559040 ?start@Popup@@QAEXPBDH@Z 0x00559040-0x0055905F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?start@Popup@@QAEXPBDH@Z)(Popup* this, int8* sectionID, int)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label, int value) {
    start(PopupStartCaption, label, -1, nullptr, value, nullptr);
}



/*
Purpose: Draw a popup's dialog background; the legacy implementation does
         nothing and returns 0.
// ORIGINAL: 0x00405B10 ?on_dialog_back_draw@Popup@@QAEHPAUGraphicWin@@@Z 0x00405B10-0x00405B15 BYTE_EXACT
// symbol    ?on_dialog_back_draw@Popup@@QAEHPAVGraphicWin@@@Z
// size      5 bytes
// prototype int (__thiscall ?on_dialog_back_draw@Popup@@QAEHPAUGraphicWin@@@Z)(Popup* this, GraphicWin*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Popup::on_dialog_back_draw(::GraphicWin *) {
    return 0;
}



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

/*
Purpose: Sound a popup's wave. Nothing sounds unless bit 0x400 of the flag
         word is set, the popup is armed, and its index is inside the
         forty-five entry bank. Wave 0x19 becomes 0x25 when the context
         field at 0x50 is below -0x46; wave 0x2B only sounds when the
         millisecond clock modulo three is one. With both the voiceover
         wave and the last-played bank wave idle, the index is remembered
         and its bank entry loads and plays. Afterwards - whether or not
         anything sounded - wave 0x19 fires the owner's virtual at 0x138
         and wave 0x10 plays effect 0x38.
// ORIGINAL: 0x004456B0 ?popup_wave_callback@@YAXPAUPopupWave@@H@Z 0x004456B0-0x004457BE SEMANTIC
// TRIED     the two differing instructions are the vtable temp - image `mov eax, [ecx]` against the compiled `mov edx, [ecx]` - and the register is chosen by the SEAM, not by spelling. VC6 reserves `__thiscall` (C4234), so a plain `(*(Slot **)owner)[78](owner)` cannot be written at all, which is why the ORIGINAL()/pointer-to-member machinery exists; that machinery is what takes eax and pushes the vtable load into edx. Byte-exactness needs a REAL class with a no-argument virtual at slot 78. It is not Win: 0x138 is Win's on_key slot (0x005F5D10) and on_key takes four arguments while this call pushes none, so naming Win here would be invention.
// size      270 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446A00 0x004C6920 0x004C6B10 0x004C6CE0
// indirect  0x00445715 0x004457A0
Return Value: n/a
Status: Complete
*/
// The last-played entry of the wave bank (popup.h). Zero-initialised, as
// the image's dword at 0x0074DAA4 is.
int PopupWaveLastIndex;  // 0x0074DAA4

void __cdecl popup_wave_callback(PopupWave *popup, int) {
    if (!(GamePreferences & 0x400)) {
        return;
    }
    if (!popup) {
        return;
    }
    const int32_t index = popup->wave_index_;
    if (index < 0) {
        return;
    }
    if (!popup->armed_108_) {
        return;
    }
    if (index >= 0x2D) {
        return;
    }
    if (index == 0x19) {
        const uint8_t *const context =
            static_cast<const uint8_t *>(PopupWaveContext());
        if (*reinterpret_cast<const int32_t *>(context + 0x50) < -0x46) {
            popup->wave_index_ = 0x25;
        }
    }
    if (popup->wave_index_ == 0x2B && (PopupWaveTimeSlot())() % 3 != 1) {
        return;
    }
    const int32_t chosen = popup->wave_index_;
    if (GamePreferences & 0x400) {
        if (!g_WAVE_GENERAL.Wave::is_playing() &&
            !(g_CPU_WAVES + PopupWaveLastIndex)->Wave::is_playing()) {
            PopupWaveLastIndex = chosen;
            Wave *const wave = g_CPU_WAVES + chosen;
            wave->Wave::load();
            wave->Wave::play();
        }
    }
    if (popup->wave_index_ == 0x19) {
        void *const owner = PopupWaveOwnerSlot();
        if (owner) {
            (ORIGINAL(owner)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(owner) + 0x138)))();
        }
    }
    if (popup->wave_index_ == 0x10) {
        g_FX.FX::play(0x38);
    }
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00404F80 ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z 0x00404F80-0x00404F83 BYTE_EXACT
// symbol    ?on_redraw_nc@Popup@@UAEXPAUtagRECT@@H@Z
// size      3 bytes
// prototype void (__thiscall ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z)(Popup* this, RECT*, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Popup::on_redraw_nc(RECT *, int) {
}


/*
Purpose: Reset the button width to 20 unless the field at 0x30AC scales to
         exactly 1000.

             mov edx,[ecx+0x30AC] / mov eax,0x68DB8BAD / imul edx
             sar edx,0xC / mov eax,edx / shr eax,0x1F / add edx,eax
             cmp edx,0x3E8 / je done
             mov dword [ecx+0x30D0],0x14

         The magic multiply is a signed divide by 10000 - the same idiom
         Datalink::UNK1 encodes in the other direction - checked against C++
         truncating division over 200,000 random dividends and the boundaries.
         0x3E8 is 1000 and 0x14 is 20.

         Popup models fields only as far as its Scroll member, so both offsets
         are reached through documented raw offsets, as BasePop::UNK3 does.
// ORIGINAL: 0x00405020 ?on_adjust_button_width@Popup@@QAEXXZ 0x00405020-0x0040504A BYTE_EXACT
// size      42 bytes
// prototype void (__thiscall ?on_adjust_button_width@Popup@@QAEXXZ)(Popup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Popup::on_adjust_button_width() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    const int32_t scaled =
        *reinterpret_cast<const int32_t *>(self + 0x30AC) / 10000;
    if (scaled != 1000) {
        *reinterpret_cast<int32_t *>(self + 0x30D0) = 20;
    }
}


/*
// ORIGINAL: 0x00404F90 ?on_nc_hittest@Popup@@QAEHHH@Z 0x00404F90-0x00404FA4 BYTE_EXACT
// symbol    ?on_nc_hittest@Popup@@UAEHHH@Z
// size      20 bytes
// prototype int (__thiscall ?on_nc_hittest@Popup@@QAEHHH@Z)(Popup* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F5AD0
Status: Complete
*/
int Popup::on_nc_hittest(int x, int y) {
    return reinterpret_cast<Win *>(this)->Win::on_nc_hittest(x, y);
}

/*
// ORIGINAL: 0x00404FB0 ?alloc@Popup@@QAAHXZ 0x00404FB0-0x0040501C;0x00650772-0x0065078F
// symbol    ?alloc@Popup@@SAHXZ
// TRIED: spelling the allocation as manual operator-new + placement-new of BasePop diverges at the EH state-byte store and runs longer than the original; plain `new Popup` is the shape that matches.
// TRIED: manual operator-new + placement-new of Popup itself (2026-08-21) - measured 7/32 vs plain `new Popup`'s 8/32; the ternary null-check adds a `sub esp,8` the image never emits. The image inlines ??0Popup@@QAE@XZ's body here while keeping it a real out-of-line call at its other 104 sites; that per-callsite inlining split is not reachable by resurfacing the new-expression, only by a compiler heuristic this tree cannot force without breaking the other 104 callers. Left as plain `new Popup` (8/32, WRONG CALLEE stands).
// size      137 bytes
// prototype
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00600860 0x006051D0 0x0064557F 0x0064558A
//
// Promoted 2026-08-15 from src/unrecovered/00404fb0.cpp to retire its
// pending_bodies forwarder. WinMain stores this function's address into the
// popup allocator hook, so it must stay a real symbol.
Status: Complete
*/
// `int`, not `Popup *`: the image's own name is ?alloc@Popup@@SAHXZ - H, an
// int return - and WinMain stores this address in the popup allocator hook,
// which is typed to match. The value carried is really the new object (the
// same wire-format lesson as Win::move); the H is what the image decreed.
int __cdecl Popup::alloc() {
    return (int)new Popup;
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004048A0 ??0Popup@@QAE@XZ 0x004048A0-0x004048F7;0x00650760-0x00650772 BYTE_EXACT
// body      src/popup.h
// size      105 bytes
// prototype void (__thiscall ??0Popup@@QAE@XZ)(Popup* this)
// callers   104   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00600860 0x006051D0
*/

// LEVER: 0x00600860 is `??0BasePop@@QAE@XZ`, a REAL constructor (unlike
// GraphicWin/BaseButton's `construct()` seam), and 0x006051D0 is
// `??0Scroll@@QAE@XZ` for the real `scroll_` member at 0x3230 - so both
// calls are the ordinary implicit base+member construction sequence, not
// explicit body-level calls. This is why the image needs an SEH frame here
// (Scroll's construction must be able to unwind the already-built BasePop
// base) where FlatButton/PullDown/NetWin - which only call a construct()
// METHOD from the body - do not pay for one at all.
Popup::Popup() {
    // EMPTY ON PURPOSE. Both vtable stores the body used to make by hand -
    // 0x006695C8 at offset 0 and 0x006695C0 at the Buffer subobject's 0x444 -
    // are the compiler's to emit now that Buffer is a real base of GraphicWin,
    // and it puts them where the image does rather than where a body would.
}

// The shared scratch slot at 0x009BC070 base_at() (base.cpp) already names -
// its own comment there says the purpose is "inferred only from this one
// call site", so it is reused here by the same storage rather than declared
// a second time under a name this function would have to guess too.
extern int BaseAtKeyPollFlag;  // 0x009BC070

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
//   WINEPREFIX=$HOME/opt/vc6/.wineprefix DISPLAY=:1 \
//       wine explorer /desktop=smac,1024x768 <exe>
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
static int PopupSavedLocX = 0x2000;  // 0x006973DC
static int PopupSavedLocY = 0x2000;  // 0x006973E0

// Read before write_check() and overwritten from read_check()'s result at
// the bottom of the non-modal-close tail - a request/response pair the two
// calls share across invocations. Zero-initialised .bss in the image at
// 0x009BC06C.
static int PopupWriteCheckState;  // 0x009BC06C

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
int __cdecl pops(char *caption, char *label, int a3, char *a4, int a5,
                 Sprite *sprite, int a7, int a8, int (__cdecl *callback)()) {
    // SAVE THE FLAG, THEN READ THE DEFAULT: the image's order is
    // `mov ecx,[SlotA]; call is_visible; mov esi,[SlotB]; test eax,eax;
    // jne +6; mov esi,[SlotA]` - the SlotB default load sits AFTER the call,
    // so the visibility result has to be bound to a named local BEFORE the
    // default is read, with the guard testing the saved value. Reading the
    // default first hoisted its load above the call (98/110); if/else and
    // ternary spellings both grew a `jmp` the image does not have.
    const BOOL winVisible = PopupInstanceSlotA->is_visible();
    Win *win = PopupInstanceSlotB;
    if (!winVisible) {
        win = PopupInstanceSlotA;
    }
    BasePop *pop = reinterpret_cast<BasePop *>(win);

    if (pop->start(caption, label, a3, a4, a5, 0)) {
        return -1;
    }

    pop->set_loc(PopupSavedLocX, PopupSavedLocY);
    PopupSavedLocY = 0x2000;
    PopupSavedLocX = 0x2000;

    if (sprite) {
        if (a7 != a8) {
            pop->field_3104_ = a7;
            pop->field_3108_ = a8;
        }
        if (pop->field_2274_ == 0) {
            pop->field_2144_ = reinterpret_cast<uint32_t>(sprite);
        } else {
            pop->field_2144_ = 0;
        }

        if (pop->sprite_box()->sprite(sprite, 0, 0)) {
            return -1;
        }
    }

    pop->write_check(PopupWriteCheckState);
    int execResult = pop->exec(0, callback);

    if (pop->field_A14_ == 0) {
        do_all_draws();
    }

    if (a5 & 0x8000) {
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
    BaseAtKeyPollFlag = pop->field_3100_;
    if ((a5 & 0x300) == 0) {
        pop->close();
    }
    return execResult;
}

/*
Purpose: Step the receiver back to the subobject ??_GPopup@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004070D0 ??_GPopup@@WEEE@AEPAXI@Z 0x004070D0-0x004070DB BYTE_EXACT
// symbol    ??_EPopup@@WEEE@AEPAXI@Z
// CORRECTED from ??3Popup@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00406BD0` into
//   ??_GPopup@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
