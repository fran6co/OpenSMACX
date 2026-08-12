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
ORIGINAL: 0x006343C0 BYTE_EXACT
Status: Complete
*/
Vector::Vector() {
    x_ = 0.0f;
    y_ = 0.0f;
    z_ = 0.0f;
}

/*
Purpose: Clear all vector components.
ORIGINAL: 0x006343D0 BYTE_EXACT
Status: Complete
*/
void Vector::close() {
    x_ = 0.0f;
    y_ = 0.0f;
    z_ = 0.0f;
}

/*
Purpose: Subtract another vector into a separate output vector.
Note:    `output` is the hidden return-object slot of `Vector operator-`, not
         a declared parameter - see the signature note in vector.h. The
         difference is computed into a temporary first because the original
         does, and because the caller may pass the same object twice.
ORIGINAL: 0x00634430
Status: Complete
*/
void Vector::__mi(Vector &output, Vector &right) {
    Vector result;
    result.x_ = x_ - right.x_;
    result.y_ = y_ - right.y_;
    result.z_ = z_ - right.z_;
    output = result;
}

/*
Purpose: Add another vector into this one, component by component.
Note:    Returns `*this`, which is what leaves the receiver in EAX at `ret 4`.
         The catalogued `void` return does not reproduce these bytes; this
         does, exactly.
ORIGINAL: 0x00634480 BYTE_EXACT
Return Value: this vector
Status: Complete
*/
Vector &Vector::__apl(Vector &right) {
    x_ += right.x_;
    y_ += right.y_;
    z_ += right.z_;
    return *this;
}

/*
Purpose: Subtract another vector from this one, component by component.
ORIGINAL: 0x006344B0 BYTE_EXACT
Return Value: this vector
Status: Complete
*/
Vector &Vector::__ami(Vector &right) {
    x_ -= right.x_;
    y_ -= right.y_;
    z_ -= right.z_;
    return *this;
}

/*
Purpose: Scale this vector and copy the updated components into an output
         vector.
Note:    The original scales `this` IN PLACE and then copies it out; `output`
         is the hidden return slot of `Vector scale(float)`. Both effects are
         load-bearing - a caller that ignores the result still sees `this`
         scaled.
ORIGINAL: 0x00634670
Status: Complete
*/
void Vector::scale(Vector &output, float scalar) {
    x_ *= scalar;
    y_ *= scalar;
    z_ *= scalar;
    output = *this;
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
    output->x_ = left->x_ + right->x_;
    output->y_ = left->y_ + right->y_;
    output->z_ = left->z_ + right->z_;
}
