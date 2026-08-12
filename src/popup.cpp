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

func_base_pop_close BasePopOriginalClose = original_method<func_base_pop_close>(0x00600F00);

/*
Purpose: Close the popup - its scroll bar first, then the popup base.
ORIGINAL: 0x00404900
Return Value: n/a
Status: Complete
*/
void Popup::close() {
    scroll_.close();
    (ORIGINAL(this)->*BasePopOriginalClose)();
}

void __fastcall popup_close_redirect(Popup *self, void *) {
    self->close();
}

func_popup_start_full PopupOriginalStartFull =
    original_method<func_popup_start_full>(0x00406380);

/*
Purpose: The five-argument start form, forwarding to the six-argument one with
         no parent graphic window.
ORIGINAL: 0x0043EF70
Return Value: n/a
Status: Complete
*/
void Popup::start(char *a1, const char *a2, int a3, char *a4, int a5) {
    (ORIGINAL(this)->*PopupOriginalStartFull)(a1, a2, a3, a4, a5, nullptr);
}

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5) {
    self->start(a1, a2, a3, a4, a5);
}

char *PopupStartCaption = reinterpret_cast<char *>(0x009B8AA8);

/*
Purpose: Start a popup from just a label, defaulting the rest.
ORIGINAL: 0x005A5990
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label) {
    (ORIGINAL(this)->*PopupOriginalStartFull)(PopupStartCaption, label, -1, nullptr, 0, nullptr);
}

/*
Purpose: Start a popup from a label and a value, defaulting the rest.
ORIGINAL: 0x00559040
Return Value: n/a
Status: Complete
*/
void Popup::start(const char *label, int value) {
    (ORIGINAL(this)->*PopupOriginalStartFull)(PopupStartCaption, label, -1, nullptr, value, nullptr);
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
ORIGINAL: 0x00405B10 BYTE_EXACT
Return Value: 0, always
Status: Complete
*/
int Popup::on_dialog_back_draw(::GraphicWin *) {
    return 0;
}

int __fastcall popup_on_dialog_back_draw_redirect(Popup *self, void *, ::GraphicWin *window) {
    return self->on_dialog_back_draw(window);
}

func_pops_full *PopsOriginalFull = (func_pops_full *)0x006276A0;

/*
Purpose: The most minimal pop() form - just a label and callback; shared
         caption buffer, value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
ORIGINAL: 0x00627130
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_label_cb(char *label, int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, 0, nullptr,
                            1, 1, callback);
}

/*
Purpose: Like pop_label_cb but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0, null sprite,
         both flags fixed at 1.
ORIGINAL: 0x00627160
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption(char *caption, char *label,
                        int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, 0, nullptr, 1, 1,
                            callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer and caller-supplied value and title; no override text, both
         flags fixed at 1.
ORIGINAL: 0x006271D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_title(char *label, int value, int title,
                            int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, nullptr, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: Like pop_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, null sprite, both flags fixed
         at 1.
ORIGINAL: 0x00627200
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_value_title(char *caption, char *label, int value,
                                    int title, int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, value, nullptr, title, nullptr, 1,
                            1, callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup with the shared caption
         buffer, value defaulted to -1 and only the title exposed; no
         override text, both flags fixed at 1.
ORIGINAL: 0x00627230
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_title(char *label, int title, int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: Like pop_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, null sprite, both flags
         fixed at 1.
ORIGINAL: 0x00627260
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_caption_title(char *caption, char *label, int title,
                              int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, title, nullptr, 1, 1,
                            callback);
}

/*
Purpose: Raise the caption-less, sprite-less popup against the shared
         caption buffer with value, override text and title all
         caller-supplied; both flags fixed at 1.
ORIGINAL: 0x00627290
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_value_text_title(char *label, int value, char *text,
                                 int title, int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, text, title,
                            nullptr, 1, 1, callback);
}

/*
Purpose: The full caption-less, sprite-less popup form - caption, value,
         override text and title all caller-supplied; null sprite, both
         flags fixed at 1.
ORIGINAL: 0x006272C0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pop_full(char *caption, char *label, int value, char *text,
                     int title, int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, value, text, title, nullptr, 1, 1,
                            callback);
}

/*
Purpose: The most minimal pops() form - label, sprite and callback; shared
         caption buffer, value -1, no override text, title 0, both flags
         fixed at 1.
ORIGINAL: 0x00627310
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_minimal(char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            1, 1, callback);
}

/*
Purpose: Like pops_minimal but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text,
         title 0.
ORIGINAL: 0x00627340
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_flags(char *label, Sprite *sprite, int flag_a, int flag_b,
                       int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, 0, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Like pops_minimal but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, title 0, both flags
         fixed at 1.
ORIGINAL: 0x00627370
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption(char *caption, char *label, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, 0, sprite, 1, 1,
                            callback);
}

/*
Purpose: Like pops_flags but with a caller-supplied caption in place of the
         shared buffer; value -1, no override text, title 0.
ORIGINAL: 0x006273A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_flags(char *caption, char *label, Sprite *sprite,
                               int flag_a, int flag_b,
                               int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, 0, sprite, flag_a,
                            flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value
         and title exposed and the sprite caller-supplied; no override
         text, both flags fixed at 1.
ORIGINAL: 0x006273D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title(char *label, int value, int title,
                             Sprite *sprite, int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, nullptr, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_value_title but with both flags caller-supplied instead
         of fixed at 1; shared caption buffer, no override text.
ORIGINAL: 0x00627400
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_title_flags(char *label, int value, int title,
                                   Sprite *sprite, int flag_a, int flag_b,
                                   int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Like pops_value_title but with a caller-supplied caption instead of
         the shared buffer; no override text, both flags fixed at 1.
ORIGINAL: 0x006274D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_value_title(char *caption, char *label, int value,
                                     int title, Sprite *sprite,
                                     int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, value, nullptr, title, sprite, 1,
                            1, callback);
}

/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, value, title, sprite and both flags.
ORIGINAL: 0x00627500
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_text(char *caption, char *label, int value, int title,
                         Sprite *sprite, int flag_a, int flag_b,
                         int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, value, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value
         defaulted to -1 and only the title and sprite exposed; no override
         text, both flags fixed at 1.
ORIGINAL: 0x00627540
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title(char *label, int title, Sprite *sprite,
                       int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, value -1, no override text.
ORIGINAL: 0x00627570
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_title_flags(char *label, int title, Sprite *sprite,
                             int flag_a, int flag_b,
                             int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, -1, nullptr, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Like pops_title but with a caller-supplied caption instead of the
         shared buffer; value -1, no override text, both flags fixed at 1.
ORIGINAL: 0x006275A0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title(char *caption, char *label, int title,
                               Sprite *sprite, int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, title, sprite, 1, 1,
                            callback);
}

/*
Purpose: Like pops_caption_title but with both flags caller-supplied instead
         of fixed at 1; value -1, no override text.
ORIGINAL: 0x006275D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_caption_title_flags(char *caption, char *label, int title,
                                     Sprite *sprite, int flag_a, int flag_b,
                                     int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, -1, nullptr, title, sprite,
                            flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with value,
         override text, title and sprite all caller-supplied; both flags
         fixed at 1.
ORIGINAL: 0x00627600
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_value_text_title(char *label, int value, char *text,
                                  int title, Sprite *sprite,
                                  int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, text, title,
                            sprite, 1, 1, callback);
}

/*
Purpose: Like pops_value_text_title but with both flags caller-supplied
         instead of fixed at 1; only the caption defaults to the shared
         buffer.
ORIGINAL: 0x00627630
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_default_caption(char *label, int value, char *text,
                                 int title, Sprite *sprite, int flag_a,
                                 int flag_b, int (__cdecl *callback)()) {
    return PopsOriginalFull(PopupStartCaption, label, value, text, title,
                            sprite, flag_a, flag_b, callback);
}

/*
Purpose: Raise the full popup with everything but the flags caller-supplied
         - caption, value, override text, title and sprite; both flags
         fixed at 1.
ORIGINAL: 0x00627670
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl pops_no_flags(char *caption, char *label, int value, char *text,
                          int title, Sprite *sprite,
                          int (__cdecl *callback)()) {
    return PopsOriginalFull(caption, label, value, text, title, sprite, 1, 1,
                            callback);
}

uint32_t *PopupWaveFlags = reinterpret_cast<uint32_t *>(0x009A6490);
void **PopupWaveContext = reinterpret_cast<void **>(0x0090EA30);
Wave *PopupWaveVoice = reinterpret_cast<Wave *>(0x00945ED0);
Wave *PopupWaveBank = reinterpret_cast<Wave *>(0x0074C5F0);
int32_t *PopupWaveLastIndex = reinterpret_cast<int32_t *>(0x0074DAA4);
void **PopupWaveOwnerSlot = reinterpret_cast<void **>(0x0074DAA0);
FX *PopupWaveFx = reinterpret_cast<FX *>(0x00749CF8);
func_popup_wave_query PopupWaveIsPlaying = original_method<func_popup_wave_query>(0x004C6B10);
func_popup_wave_query PopupWaveLoad = original_method<func_popup_wave_query>(0x004C6CE0);
func_popup_wave_query PopupWavePlay = original_method<func_popup_wave_query>(0x004C6920);
func_popup_fx_play PopupFxPlay = original_method<func_popup_fx_play>(0x00446A00);
func_popup_time_source **PopupWaveTimeSlot = (func_popup_time_source **)0x00669368;

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
ORIGINAL: 0x004456B0
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
            static_cast<const uint8_t *>(*PopupWaveContext);
        if (*reinterpret_cast<const int32_t *>(context + 0x50) < -0x46) {
            popup->wave_index_ = 0x25;
        }
    }
    if (popup->wave_index_ == 0x2B && (*PopupWaveTimeSlot)() % 3 != 1) {
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
        void *const owner = *PopupWaveOwnerSlot;
        if (owner) {
            (ORIGINAL(owner)->*original_method<void (OriginalObject::*)() >(*reinterpret_cast<unsigned long *>(*reinterpret_cast<uint8_t **>(owner) + 0x138)))();
        }
    }
    if (popup->wave_index_ == 0x10) {
        (ORIGINAL(PopupWaveFx)->*PopupFxPlay)(0x38);
    }
}

void __cdecl popup_wave_callback_redirect(PopupWave *popup, int a2) {
    popup_wave_callback(popup, a2);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
ORIGINAL: 0x00404F80 BYTE_EXACT
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
ORIGINAL: 0x00405020 BYTE_EXACT
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
ORIGINAL: 0x00404F90 BYTE_EXACT
Status: Complete
*/
int Popup::on_nc_hittest(int a1, int a2) {
    return reinterpret_cast<Win *>(this)->on_nc_hittest(a1, a2);
}
