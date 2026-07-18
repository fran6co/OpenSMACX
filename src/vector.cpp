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
Original Offset: 006343C0
Status: Complete
*/
Vector::Vector() : values_{} {
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

/*
Purpose: Clear all vector components.
Original Offset: 006343D0
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
Original Offset: 00634430
Status: Complete
*/
void Vector::__mi(Vector &output, Vector &right) {
    uint32_t results[3] = {};
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__(
        "flds %1\n\t"
        "fsubs %2\n\t"
        "fstps %0"
        : "=m"(results[0])
        : "m"(values_[0]), "m"(right.values_[0])
        : "st", "memory");
    __asm__ __volatile__(
        "flds %1\n\t"
        "fsubs %2\n\t"
        "fstps %0"
        : "=m"(results[1])
        : "m"(values_[1]), "m"(right.values_[1])
        : "st", "memory");
    __asm__ __volatile__(
        "flds %1\n\t"
        "fsubs %2\n\t"
        "fstps %0"
        : "=m"(results[2])
        : "m"(values_[2]), "m"(right.values_[2])
        : "st", "memory");
#else
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        float result;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        result = left_value - right_value;
        std::memcpy(&results[index], &result, sizeof(result));
    }
#endif
    volatile uint32_t *ordered_output = output.values_;
    ordered_output[0] = results[0];
    ordered_output[1] = results[1];
    ordered_output[2] = results[2];
}

/*
Purpose: Add another vector sequentially into this vector.
Original Offset: 00634480
Status: Complete
*/
void Vector::__apl(Vector &right) {
#if defined(__GNUC__) && defined(__i386__)
#define OPENSMACX_VECTOR_ADD(INDEX) \
    __asm__ __volatile__( \
        "flds %1\n\t" \
        "fadds %0\n\t" \
        "fstps %0" \
        : "+m"(values_[INDEX]) \
        : "m"(right.values_[INDEX]) \
        : "st", "memory")
    OPENSMACX_VECTOR_ADD(0);
    OPENSMACX_VECTOR_ADD(1);
    OPENSMACX_VECTOR_ADD(2);
#undef OPENSMACX_VECTOR_ADD
#else
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        left_value += right_value;
        std::memcpy(&values_[index], &left_value, sizeof(left_value));
    }
#endif
}

/*
Purpose: Subtract another vector sequentially from this vector.
Original Offset: 006344B0
Status: Complete
*/
void Vector::__ami(Vector &right) {
#if defined(__GNUC__) && defined(__i386__)
#define OPENSMACX_VECTOR_SUBTRACT(INDEX) \
    __asm__ __volatile__( \
        "flds %0\n\t" \
        "fsubs %1\n\t" \
        "fstps %0" \
        : "+m"(values_[INDEX]) \
        : "m"(right.values_[INDEX]) \
        : "st", "memory")
    OPENSMACX_VECTOR_SUBTRACT(0);
    OPENSMACX_VECTOR_SUBTRACT(1);
    OPENSMACX_VECTOR_SUBTRACT(2);
#undef OPENSMACX_VECTOR_SUBTRACT
#else
    for (size_t index = 0; index < 3; ++index) {
        float left_value;
        float right_value;
        std::memcpy(&left_value, &values_[index], sizeof(left_value));
        std::memcpy(&right_value, &right.values_[index], sizeof(right_value));
        left_value -= right_value;
        std::memcpy(&values_[index], &left_value, sizeof(left_value));
    }
#endif
}

/*
Purpose: Scale this vector and copy the updated components into an output vector.
Original Offset: 00634670
Status: Complete
*/
void Vector::scale(Vector &output, float scalar) {
#if defined(__GNUC__) && defined(__i386__)
#define OPENSMACX_VECTOR_SCALE(INDEX) \
    __asm__ __volatile__( \
        "flds %1\n\t" \
        "fmuls %0\n\t" \
        "fstps %0" \
        : "+m"(values_[INDEX]) \
        : "m"(scalar) \
        : "st", "memory")
    OPENSMACX_VECTOR_SCALE(0);
    OPENSMACX_VECTOR_SCALE(1);
    OPENSMACX_VECTOR_SCALE(2);
#undef OPENSMACX_VECTOR_SCALE
#else
    for (size_t index = 0; index < 3; ++index) {
        float value;
        std::memcpy(&value, &values_[index], sizeof(value));
        value *= scalar;
        std::memcpy(&values_[index], &value, sizeof(value));
    }
#endif
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
