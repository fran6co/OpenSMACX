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
ORIGINAL: 0x00616BC0 BYTE_EXACT
// name      ??0CaviarData@@QAE@XZ
// size      13 bytes
// spans     0x00616BC0-0x00616BCD
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

func_caviar_free_record *CaviarDataFreeRecord =
    (func_caviar_free_record *)0x00638430;

/*
Purpose: Release the record this data slot owns. A slot with no record is left
         entirely untouched; otherwise the helper walks the record and frees
         its members, and the slot forgets it. The two leading fields keep
         whatever they held either way.
ORIGINAL: 0x00616C60
// name      ?close@CaviarData@@QAEXXZ
// size      28 bytes
// spans     0x00616C60-0x00616C7C
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
        CaviarDataFreeRecord(record_);
        record_ = nullptr;
    }
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
ORIGINAL: 0x00616DA0
// name      ??0Caviar@@QAE@XZ
// size      54 bytes
// spans     0x00616DA0-0x00616DD6
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
    memset(bytes + 0x74C, 0, sizeof(CaviarData) * 200);
    const uint32_t distance_limit = 0x461C4000;
    const uint32_t default_scene_scale = 0x3F2AAAAB;
    const uint32_t zero = 0;
    memcpy(bytes + 0x108, &distance_limit, sizeof(distance_limit));
    memcpy(bytes + 0x13CC, &zero, sizeof(zero));
    memcpy(&scene_scale_, &default_scene_scale, sizeof(scene_scale_));
}

/*
Purpose: Copy a camera position and orientation directly into the renderer state.
ORIGINAL: 0x006182A0
// name      ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z
// size      127 bytes
// spans     0x006182A0-0x0061831F
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
ORIGINAL: 0x006183B0
// name      ?set_scaling@Caviar@@QAEXM@Z
// size      13 bytes
// spans     0x006183B0-0x006183BD
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
ORIGINAL: 0x006183C0
// name      ?get_scaling@Caviar@@QAEMXZ
// size      7 bytes
// spans     0x006183C0-0x006183C7
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
ORIGINAL: 0x006176C0 BYTE_EXACT
// name      ?UNK3@Caviar@@QAEHHHH@Z
// size      5 bytes
// spans     0x006176C0-0x006176C5
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
ORIGINAL: 0x006176D0 BYTE_EXACT
// name      ?UNK4@Caviar@@QAEHHHHH@Z
// size      5 bytes
// spans     0x006176D0-0x006176D5
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
ORIGINAL: 0x00618250 BYTE_EXACT
// name      ?UNK5@Caviar@@QAEXHH@Z
// size      3 bytes
// spans     0x00618250-0x00618253
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
ORIGINAL: 0x00618260 BYTE_EXACT
// name      ?UNK6@Caviar@@QAEXHH@Z
// size      3 bytes
// spans     0x00618260-0x00618263
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
ORIGINAL: 0x00618270 BYTE_EXACT
// name      ?UNK7@Caviar@@QAEXHH@Z
// size      3 bytes
// spans     0x00618270-0x00618273
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
ORIGINAL: 0x00618290 BYTE_EXACT
// name      ?UNK9@Caviar@@QAEXHHH@Z
// size      3 bytes
// spans     0x00618290-0x00618293
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
ORIGINAL: 0x006183A0 BYTE_EXACT
// name      ?UNK12@Caviar@@QAEXHHH@Z
// size      3 bytes
// spans     0x006183A0-0x006183A3
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
ORIGINAL: 0x00618280 BYTE_EXACT
// name      ?UNK8@Caviar@@QAEXH@Z
// size      13 bytes
// spans     0x00618280-0x0061828D
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
ORIGINAL: 0x00618320 BYTE_EXACT
// name      ?UNK10@Caviar@@QAEXHHH@Z
// size      24 bytes
// spans     0x00618320-0x00618338
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

func_apply_rotation *CaviarOriginalApplyRotation =
    (func_apply_rotation *)0x00627D00;

/*
Purpose: Set the scene's rotation, handing the three angles and the matrix at
         0x38 to the helper that applies them.
ORIGINAL: 0x00618370
// name      ?set_scene_rotation@Caviar@@QAEXMMM@Z
// size      47 bytes
// spans     0x00618370-0x0061839F
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
    CaviarOriginalApplyRotation(angles,
                                reinterpret_cast<uint8_t *>(this) + 0x38);
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
ORIGINAL: 0x00618340 BYTE_EXACT
// name      ?UNK11@Caviar@@QAEXHHH@Z
// size      42 bytes
// spans     0x00618340-0x0061836A
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
