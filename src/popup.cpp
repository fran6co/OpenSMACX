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

void __fastcall popup_close_redirect(Popup *self, void *) {
    self->close();
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

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5) {
    self->start(a1, a2, a3, a4, a5);
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

void __fastcall popup_start_label_redirect(Popup *self, void *, const char *label) {
    self->start(label);
}

void __fastcall popup_start_label_value_redirect(Popup *self, void *,
                                                 const char *label, int value) {
    self->start(label, value);
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

int __fastcall popup_on_dialog_back_draw_redirect(Popup *self, void *, ::GraphicWin *window) {
    return self->on_dialog_back_draw(window);
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

func_popup_wave_query PopupWaveIsPlaying = original_method<func_popup_wave_query>(0x004C6B10);
func_popup_wave_query PopupWaveLoad = original_method<func_popup_wave_query>(0x004C6CE0);
func_popup_wave_query PopupWavePlay = original_method<func_popup_wave_query>(0x004C6920);

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
// ORIGINAL: 0x004456B0 ?popup_wave_callback@@YAXPAUPopupWave@@H@Z 0x004456B0-0x004457BE
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
void __cdecl popup_wave_callback(PopupWave *popup, int) {
    if (!(*PopupWaveFlags & 0x400)) {
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
    if (*PopupWaveFlags & 0x400) {
        if (!(ORIGINAL(PopupWaveVoice)->*PopupWaveIsPlaying)() &&
            !(ORIGINAL(PopupWaveBank + *PopupWaveLastIndex)->*PopupWaveIsPlaying)()) {
            *PopupWaveLastIndex = chosen;
            Wave *const wave = PopupWaveBank + chosen;
            (ORIGINAL(wave)->*PopupWaveLoad)();
            (ORIGINAL(wave)->*PopupWavePlay)();
        }
    }
    if (popup->wave_index_ == 0x19) {
        void *const owner = PopupWaveOwnerSlot();
        if (owner) {
            (ORIGINAL(owner)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(owner) + 0x138)))();
        }
    }
    if (popup->wave_index_ == 0x10) {
        PopupWaveFx->FX::play(0x38);
    }
}

void __cdecl popup_wave_callback_redirect(PopupWave *popup, int a2) {
    popup_wave_callback(popup, a2);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00404F80 ?on_redraw_nc@Popup@@QAEXPAURECT@@H@Z 0x00404F80-0x00404F83 BYTE_EXACT
// symbol    ?on_redraw_nc@Popup@@QAEXPAUtagRECT@@H@Z
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

void __fastcall popup_on_redraw_nc_redirect(
        Popup *self, void *, RECT *a1, int a2) {
    self->on_redraw_nc(a1, a2);
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

void __fastcall popup_on_adjust_button_width_redirect(Popup *self, void *) {
    self->on_adjust_button_width();
}

/*
// ORIGINAL: 0x00404F90 ?on_nc_hittest@Popup@@QAEHHH@Z 0x00404F90-0x00404FA4 BYTE_EXACT
// size      20 bytes
// prototype int (__thiscall ?on_nc_hittest@Popup@@QAEHHH@Z)(Popup* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005F5AD0
Status: Complete
*/
int Popup::on_nc_hittest(int a1, int a2) {
    return reinterpret_cast<Win *>(this)->on_nc_hittest(a1, a2);
}

/*
// ORIGINAL: 0x00404FB0 ?alloc@Popup@@QAAHXZ 0x00404FB0-0x0040501C;0x00650772-0x0065078F
// symbol    ?alloc@Popup@@SAHXZ
// RULED-OUT: spelling the allocation as manual operator-new + placement-new of BasePop diverges at the EH state-byte store and runs longer than the original; plain `new Popup` is the shape that matches.
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
// ORIGINAL: 0x004048A0 ??0Popup@@QAE@XZ 0x004048A0-0x004048F7;0x00650760-0x00650772
// body      src/popup.h
// size      105 bytes
// prototype void (__thiscall ??0Popup@@QAE@XZ)(Popup* this)
// callers   104   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00600860 0x006051D0
*/
