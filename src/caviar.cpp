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

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
uint32_t load_scaling_bits(const void *source) {
    uint32_t value;
    memcpy(&value, source, sizeof(value));
    return value;
}

} // namespace

/*
Purpose: Initialize an empty Caviar object-data record.
Original Offset: 00616BC0
Return Value: n/a
Status: Complete
*/
CaviarData::CaviarData() : fields_{} {
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
Original Offset: 00616DA0
Return Value: n/a
Status: Complete
*/
Caviar::Caviar() {
    auto *bytes = reinterpret_cast<uint8_t *>(this);
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
Original Offset: 006182A0
Return Value: n/a
Status: Complete
*/
void Caviar::set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix) {
    if (camera && matrix) {
        auto *bytes = reinterpret_cast<uint8_t *>(this);
        memcpy(bytes + 0xA5, camera, sizeof(*camera));
        memcpy(bytes + 0xB1, matrix, sizeof(*matrix));
    }
}

/*
Purpose: Set the Caviar renderer scaling factor.
Original Offset: 006183B0
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
Original Offset: 006183C0
Return Value: Current scaling factor
Status: Complete
*/
float Caviar::get_scaling() {
    float scaling;
    memcpy(&scaling, reinterpret_cast<uint8_t *>(this) + 0xD5, sizeof(scaling));
    return scaling;
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
Original Offset: 006176C0
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
Original Offset: 006176D0
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
Original Offset: 00618250
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
Original Offset: 00618260
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
Original Offset: 00618270
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
Original Offset: 00618290
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
Original Offset: 006183A0
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
Original Offset: 00618280
Return Value: n/a
Status: Complete
*/
void Caviar::UNK8(int a1) {
    field_108_ = a1;
}

/*
Purpose: Store three values into the triplet at 0x2C.
Original Offset: 00618320
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
