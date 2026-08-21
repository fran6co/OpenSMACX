/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "caviar.h"

#include <new>

namespace {

OPENSMACX_NOINLINE
uint32_t load_scaling_bits(const void *source) {
    uint32_t value;
    memcpy(&value, source, sizeof(value));
    return value;
}

} // namespace

/*
Purpose: Initialize an empty Caviar object-data record.
// ORIGINAL: 0x00616BC0 ??0CaviarData@@QAE@XZ 0x00616BC0-0x00616BCD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ??0CaviarData@@QAE@XZ)(CaviarData* this)
// callers   82   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
CaviarData::CaviarData() : field_0_(0), fileDescriptor_(0), record_(nullptr) {
}

/*
Purpose: Release the record this data slot owns. A slot with no record is left
         entirely untouched; otherwise the helper walks the record and frees
         its members, and the slot forgets it. The two leading fields keep
         whatever they held either way.
// ORIGINAL: 0x00616C60 ?close@CaviarData@@QAEXXZ 0x00616C60-0x00616C7C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?close@CaviarData@@QAEXXZ)(CaviarData* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638430
Return Value: n/a
Status: Complete
*/
void CaviarData::close() {
    if (record_) {
        caviar_free_record(record_);
        record_ = nullptr;
    }
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
// ORIGINAL: 0x00616DA0 ??0Caviar@@QAE@XZ 0x00616DA0-0x00616DD6
// size      54 bytes
// prototype void (__thiscall ??0Caviar@@QAE@XZ)(Caviar* this)
// callers   7   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
Caviar::Caviar() {
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    memset(slots_, 0, sizeof(slots_));
    const uint32_t distance_limit = 0x461C4000;
    const uint32_t default_scene_scale = 0x3F2AAAAB;
    const uint32_t zero = 0;
    memcpy(bytes + 0x108, &distance_limit, sizeof(distance_limit));
    memcpy(bytes + 0x13CC, &zero, sizeof(zero));
    memcpy(&scene_scale_, &default_scene_scale, sizeof(scene_scale_));
}

/*
Purpose: Copy a camera position and orientation directly into the renderer state.
// ORIGINAL: 0x006182A0 ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z 0x006182A0-0x0061831F
// symbol    ?set_camera_direct@Caviar@@QAEXPBUVOX_Vect@@PBUVOX_Matrix@@@Z
// size      127 bytes
// prototype void (__thiscall ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z)(Caviar* this, VOX_Vect*, VOX_Matrix*)
// callers   10   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Caviar::set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix) {
    if (camera && matrix) {
        uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
        memcpy(bytes + 0xA5, camera, sizeof(*camera));
        memcpy(bytes + 0xB1, matrix, sizeof(*matrix));
    }
}

/*
Purpose: Set the Caviar renderer scaling factor.
// ORIGINAL: 0x006183B0 ?set_scaling@Caviar@@QAEXM@Z 0x006183B0-0x006183BD
// size      13 bytes
// prototype void (__thiscall ?set_scaling@Caviar@@QAEXM@Z)(Caviar* this, float)
// callers   24   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Caviar::set_scaling(float scaling) {
    set_scaling_bits(load_scaling_bits(&scaling));
}

void Caviar::set_scaling_bits(uint32_t scaling_bits) {
    memcpy(reinterpret_cast<uint8_t *>(this) + 0xD5,
           &scaling_bits, sizeof(scaling_bits));
}

/*
Purpose: Read the Caviar renderer scaling factor.
// ORIGINAL: 0x006183C0 ?get_scaling@Caviar@@QAEMXZ 0x006183C0-0x006183C7
// size      7 bytes
// prototype float (__thiscall ?get_scaling@Caviar@@QAEMXZ)(Caviar* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Current scaling factor
Status: Complete
*/
float Caviar::get_scaling() {
    float scaling;
    memcpy(&scaling, reinterpret_cast<uint8_t *>(this) + 0xD5, sizeof(scaling));
    return scaling;
}

void __fastcall caviar_data_close_redirect(CaviarData *self, void *) {
    self->close();
}

CaviarData *__fastcall caviar_data_construct_redirect(CaviarData *self, void *) {
    return new (self) CaviarData;
}

Caviar *__fastcall caviar_construct_redirect(Caviar *self, void *) {
    return new (self) Caviar;
}

void __fastcall caviar_set_camera_direct_redirect(
    Caviar *self, void *, const VOX_Vect *camera, const VOX_Matrix *matrix) {
    self->set_camera_direct(camera, matrix);
}

uint32_t __fastcall caviar_set_scaling_redirect(
    Caviar *self, void *, uint32_t scaling_bits) {
    self->set_scaling_bits(scaling_bits);
    return scaling_bits;
}

float __fastcall caviar_get_scaling_redirect(Caviar *self, void *) {
    return self->get_scaling();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x006176C0 ?UNK3@Caviar@@QAEHHHH@Z 0x006176C0-0x006176C5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK3@Caviar@@QAEHHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Caviar::UNK3(int, int, int) {
    return 0;
}

int __fastcall caviar_unk3_redirect(Caviar *self, void *, int a1, int a2, int a3) {
    return self->UNK3(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x006176D0 ?UNK4@Caviar@@QAEHHHHH@Z 0x006176D0-0x006176D5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@Caviar@@QAEHHHHH@Z)(Caviar* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Caviar::UNK4(int, int, int, int) {
    return 0;
}

int __fastcall caviar_unk4_redirect(Caviar *self, void *, int a1, int a2, int a3, int a4) {
    return self->UNK4(a1, a2, a3, a4);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618250 ?UNK5@Caviar@@QAEXHH@Z 0x00618250-0x00618253 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK5@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK5(int, int) {
}

void __fastcall caviar_unk5_redirect(Caviar *self, void *, int a1, int a2) {
    self->UNK5(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618260 ?UNK6@Caviar@@QAEXHH@Z 0x00618260-0x00618263 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK6(int, int) {
}

void __fastcall caviar_unk6_redirect(Caviar *self, void *, int a1, int a2) {
    self->UNK6(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618270 ?UNK7@Caviar@@QAEXHH@Z 0x00618270-0x00618273 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK7@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK7(int, int) {
}

void __fastcall caviar_unk7_redirect(Caviar *self, void *, int a1, int a2) {
    self->UNK7(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618290 ?UNK9@Caviar@@QAEXHHH@Z 0x00618290-0x00618293 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK9@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK9(int, int, int) {
}

void __fastcall caviar_unk9_redirect(Caviar *self, void *, int a1, int a2, int a3) {
    self->UNK9(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x006183A0 ?UNK12@Caviar@@QAEXHHH@Z 0x006183A0-0x006183A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK12@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK12(int, int, int) {
}

void __fastcall caviar_unk12_redirect(Caviar *self, void *, int a1, int a2, int a3) {
    self->UNK12(a1, a2, a3);
}

/*
Purpose: Store a single value at 0x108.
// ORIGINAL: 0x00618280 ?UNK8@Caviar@@QAEXH@Z 0x00618280-0x0061828D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?UNK8@Caviar@@QAEXH@Z)(Caviar* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK8(int a1) {
    field_108_ = a1;
}

/*
Purpose: Store three values into the triplet at 0x2C.
// ORIGINAL: 0x00618320 ?UNK10@Caviar@@QAEXHHH@Z 0x00618320-0x00618338 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?UNK10@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK10(int a1, int a2, int a3) {
    field_2C_ = a1;
    field_30_ = a2;
    field_34_ = a3;
}

void __fastcall caviar_unk8_redirect(Caviar *self, void *, int a1) {
    self->UNK8(a1);
}

void __fastcall caviar_unk10_redirect(Caviar *self, void *, int a1, int a2, int a3) {
    self->UNK10(a1, a2, a3);
}

/*
Purpose: Set the scene's rotation, handing the three angles and the matrix at
         0x38 to the helper that applies them.
// ORIGINAL: 0x00618370 ?set_scene_rotation@Caviar@@QAEXMMM@Z 0x00618370-0x0061839F BYTE_EXACT
// size      47 bytes
// prototype void (__thiscall ?set_scene_rotation@Caviar@@QAEXMMM@Z)(Caviar* this, float, float, float)
// callers   13   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627D00
Return Value: n/a
Status: Complete
*/
void Caviar::set_scene_rotation(float x, float y, float z) {
    float angles[3] = {x, y, z};
    caviar_apply_rotation(angles, reinterpret_cast<uint8_t *>(this) + 0x38);
}

void __fastcall caviar_set_scene_rotation_redirect(Caviar *self, void *,
                                                   float x, float y, float z) {
    self->set_scene_rotation(x, y, z);
}

/*
Purpose: Read back the three values at 0x2C, 0x30 and 0x34, skipping any output
         whose pointer is null.

             test eax,eax / je next / mov edx,[ecx+0x2C] / mov [eax],edx

         repeated per output. Each null check guards only its OWN store, so a
         caller passing the middle pointer null still receives the other two -
         which is what the three independent branches encode and what a single
         guard around all three would get wrong.

         The mangled name declares three ints; they are used as pointers, and
         the parameters are typed that way here because that is what the body
         does with them.
// ORIGINAL: 0x00618340 ?UNK11@Caviar@@QAEXHHH@Z 0x00618340-0x0061836A BYTE_EXACT
// symbol    ?UNK11@Caviar@@QAEXPAH00@Z
// size      42 bytes
// prototype void (__thiscall ?UNK11@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK11(int *out1, int *out2, int *out3) {
    if (out1) {
        *out1 = field_2C_;
    }
    if (out2) {
        *out2 = field_30_;
    }
    if (out3) {
        *out3 = field_34_;
    }
}

void __fastcall caviar_unk11_redirect(Caviar *self, void *, int *out1,
                                      int *out2, int *out3) {
    self->UNK11(out1, out2, out3);
}

/*
Purpose: Release every allocation the renderer holds and reset the scene.
// ORIGINAL: 0x00617020 ?close@Caviar@@QAEXXZ 0x00617020-0x0061707F BYTE_EXACT
// size      95 bytes
// prototype void (__thiscall ?close@Caviar@@QAEXXZ)(Caviar* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
//
// PROMOTED from src/recovered/00617020.cpp, which was byte-exact but not
// compiled - so it proved the artifact and not this tree, and `class Caviar`
// did not even declare `close`. Its body reached the slots as
// `reinterpret_cast<char *>(this) + 0x750` walked by a raw `record += 0xc`;
// `slots_` says the same thing in the type, and see the note on it in the
// header for why the arithmetic reconciles.
//
// A DOWN-COUNTING do/while OVER A WALKING POINTER, because that is what the
// image emits: it never materialises an index, and the loop cannot run zero
// times.
Return Value: n/a
Status: Complete
*/
void Caviar::close() {
    // THE CURSOR AIMS AT THE MIDDLE MEMBER. The image computes
    // `lea esi, [ebp + 0x750]` - `fileDescriptor_` of slot zero, not
    // `field_0_` at 0x74C - and reaches the other two as `[esi - 4]` and
    // `[esi + 4]`. Walking from `slots_` bases at 0x74C instead and reads
    // `[esi]`, `[esi+4]`, `[esi+8]`: the same three words, a different
    // instruction at every site.
    void **cursor = &slots_->fileDescriptor_;
    int count = 0xc8;
    do {
        if (cursor[-1] != nullptr) {
            free(cursor[-1]);
            cursor[-1] = nullptr;
        }
        if (cursor[0] != nullptr) {
            free(cursor[0]);
            cursor[0] = nullptr;
        }
        if (cursor[1] != nullptr) {
            free(cursor[1]);
            cursor[1] = nullptr;
        }
        cursor += 3;
    } while (--count);

    field_4_ = 0;
    scene_scale_ = 2.0f / 3.0f;
}
