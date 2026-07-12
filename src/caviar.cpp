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

CaviarData *__fastcall caviar_data_construct_redirect(CaviarData *self, void *) {
    return new (self) CaviarData;
}

Caviar *__fastcall caviar_construct_redirect(Caviar *self, void *) {
    return new (self) Caviar;
}

uint32_t __fastcall caviar_set_scaling_redirect(
    Caviar *self, void *, uint32_t scaling_bits) {
    self->set_scaling_bits(scaling_bits);
    return scaling_bits;
}
