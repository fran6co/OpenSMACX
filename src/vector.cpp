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
#include "vector.h"

#include <cstring>
#include <new>

/*
Purpose: Initialize a zero vector.
ORIGINAL: 0x006343C0
Status: Complete
*/
Vector::Vector() {
    volatile uint32_t *ordered = values_;
    ordered[0] = 0;
    ordered[1] = 0;
    ordered[2] = 0;
}

/*
Purpose: Clear all vector components.
ORIGINAL: 0x006343D0
Status: Complete
*/
void Vector::close() {
    volatile uint32_t *ordered = values_;
    ordered[0] = 0;
    ordered[1] = 0;
    ordered[2] = 0;
}

/*
Purpose: Subtract another vector into a separate output vector.
ORIGINAL: 0x00634430
Status: Complete
*/
void Vector::__mi(Vector &output, Vector &right) {
    uint32_t results[3] = {0};
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        float result;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        result = left_value - right_value;
        std::memcpy(&results[index], &result, sizeof(result));
    }
    volatile uint32_t *ordered_output = output.values_;
    ordered_output[0] = results[0];
    ordered_output[1] = results[1];
    ordered_output[2] = results[2];
}

/*
Purpose: Add another vector sequentially into this vector.
ORIGINAL: 0x00634480
Status: Complete
*/
void Vector::__apl(Vector &right) {
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        left_value += right_value;
        std::memcpy(&values_[index], &left_value, sizeof(left_value));
    }
}

/*
Purpose: Subtract another vector sequentially from this vector.
ORIGINAL: 0x006344B0
Status: Complete
*/
void Vector::__ami(Vector &right) {
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        left_value -= right_value;
        std::memcpy(&values_[index], &left_value, sizeof(left_value));
    }
}

/*
Purpose: Scale this vector and copy the updated components into an output vector.
ORIGINAL: 0x00634670
Status: Complete
*/
void Vector::scale(Vector &output, float scalar) {
    for (size_t index = 0; index < 3; ++index) {
        float value;
        std::memcpy(&value, &values_[index], sizeof(value));
        value *= scalar;
        std::memcpy(&values_[index], &value, sizeof(value));
    }
    volatile uint32_t *ordered_output = output.values_;
    volatile const uint32_t *ordered_source = values_;
    ordered_output[0] = ordered_source[0];
    ordered_output[1] = ordered_source[1];
    ordered_output[2] = ordered_source[2];
}

Vector *__fastcall vector_construct_redirect(Vector *self, void *) {
    return new (self) Vector;
}

uintptr_t __fastcall vector_close_redirect(Vector *self, void *) {
    self->close();
    return 0;
}

Vector *__fastcall vector_subtract_redirect(
        Vector *self, void *, Vector *output, Vector *right) {
    self->__mi(*output, *right);
    return output;
}

Vector *__fastcall vector_add_assign_redirect(Vector *self, void *, Vector *right) {
    self->__apl(*right);
    return self;
}

Vector *__fastcall vector_subtract_assign_redirect(
        Vector *self, void *, Vector *right) {
    self->__ami(*right);
    return self;
}

Vector *__fastcall vector_scale_redirect(
        Vector *self, void *, Vector *output, uint32_t scalar_bits) {
    float scalar;
    std::memcpy(&scalar, &scalar_bits, sizeof(scalar));
    self->scale(*output, scalar);
    return output;
}

/*
Purpose: Add two vectors component-wise into an output vector.
ORIGINAL: 0x00628150
Status: Complete
*/
void __cdecl vector_add(Vector *left, Vector *right, Vector *output) {
    uint32_t *const left_values = reinterpret_cast<uint32_t *>(left);
    uint32_t *const right_values = reinterpret_cast<uint32_t *>(right);
    uint32_t *const output_values = reinterpret_cast<uint32_t *>(output);
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        float result;
        std::memcpy(&left_value, &left_values[index], sizeof(left_value));
        std::memcpy(&right_value, &right_values[index], sizeof(right_value));
        result = left_value + right_value;
        std::memcpy(&output_values[index], &result, sizeof(result));
    }
}
