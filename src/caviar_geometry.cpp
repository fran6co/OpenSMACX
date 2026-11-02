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

// ---------------------------------------------------------------------------
// Geometry shared by scene drawing and camera transformation. Matrix inputs
// use their real row bounds; output stores are sequential, with fresh input
// reads for each result, preserving the original overlap behavior.
// ---------------------------------------------------------------------------

// 0x006280E0. A 3x3 times a vector: three rows of three multiplies and a
// store, 107 bytes, no calls of its own.
// ORIGINAL: 0x006280E0 ?vox_matrix_vector_mul@@YAXPAY02$$CBMPBMPAM@Z 0x006280E0-0x0062814B
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x006280E0-0x0062814B loads three matrix rows, computes each dot product with its distinct x87 grouping, and stores output components at0x00628108,0x00628127,0x00628147 before reading the next row. Complete extent agrees with src/unrecovered/006280e0.cpp.", "Existing vox_draw_scene and vox_transform_camera call sites supply VOX_Matrix::values, a float[3][3], not a nine-element inner row. Pointer-to-row parameters preserve that C++ object boundary without changing pointer ABI."], "reason": "Multiply a VOX matrix by a three-float vector using actual row ownership, original dot-product grouping and sequential stores. Reload inputs between results to preserve observed overlap behavior."}
void __cdecl vox_matrix_vector_mul(const float (*matrix)[3], const float *vector,
                                  float *out) {
    out[0] = (matrix[0][1] * vector[1] + matrix[0][2] * vector[2])
        + vector[0] * matrix[0][0];
    out[1] = (matrix[1][1] * vector[1] + matrix[1][2] * vector[2])
        + matrix[1][0] * vector[0];
    out[2] = (matrix[2][2] * vector[2] + matrix[2][1] * vector[1])
        + matrix[2][0] * vector[0];
}

// 0x00627E20. A 3x3 times a 3x3, 303 bytes - nine dot products over the same
// rows the vector multiply walks.
// ORIGINAL: 0x00627E20 ?vox_matrix_matrix_mul@@YAXPAY02$$CBM0PAY02M@Z 0x00627E20-0x00627F4F
// RECOVERY_BINDING: v1 {"evidence": ["Original 0x00627E20-0x00627F4F computes nine dot products and stores row-major components one at a time; the x87 stack empties at each store and later products reload input memory. Complete extent agrees with src/unrecovered/00627e20.cpp.", "Existing scene and camera callers own VOX_Matrix records with float values[3][3]. Both input and output matrix parameters therefore point to rows; callers pass values rather than escaping the first three-element row."], "reason": "Multiply two row-major VOX matrices with their genuine three-float row bounds; retain per-cell original expression grouping and row-major sequential output stores, including input/output overlap behavior."}
void __cdecl vox_matrix_matrix_mul(const float (*left)[3], const float (*right)[3],
                                  float (*out)[3]) {
    // Parentheses retain each original x87 dot-product grouping. Do not
    // precompute a whole row or matrix: out may alias either input.
    out[0][0] = (left[0][1] * right[1][0] + left[0][2] * right[2][0])
        + right[0][0] * left[0][0];
    out[0][1] = (left[0][0] * right[0][1] + right[1][1] * left[0][1])
        + right[2][1] * left[0][2];
    out[0][2] = (left[0][0] * right[0][2] + left[0][2] * right[2][2])
        + left[0][1] * right[1][2];
    out[1][0] = (right[0][0] * left[1][0] + right[2][0] * left[1][2])
        + right[1][0] * left[1][1];
    out[1][1] = (right[0][1] * left[1][0] + right[2][1] * left[1][2])
        + right[1][1] * left[1][1];
    out[1][2] = (left[1][2] * right[2][2] + left[1][1] * right[1][2])
        + left[1][0] * right[0][2];
    out[2][0] = (right[0][0] * left[2][0] + left[2][1] * right[1][0])
        + left[2][2] * right[2][0];
    out[2][1] = (left[2][2] * right[2][1] + left[2][1] * right[1][1])
        + left[2][0] * right[0][1];
    out[2][2] = (left[2][0] * right[0][2] + left[2][1] * right[1][2])
        + left[2][2] * right[2][2];
}

/*
Purpose: Normalise a vector in place: divide each component by the vector's
         length. The engine's own normaliser, in the Vector family - its
         neighbours are vector_add (0x00628150) and the scale helper at
         0x006281B0 - and six game bodies call it besides Caviar::init, which
         normalises the scene's down vector with it.
// ORIGINAL: 0x00628220 sub_628220 0x00628220-0x0062828B FILE
// symbol    ?vox_normalise_vector@@YAXPBMPAM@Z
//           (the image carries no name for this address; the fact records
//           what this tree emits. VC6 spells `const float *` and `float *`
//           out in full - PBM and PAM - where a repeated IDENTICAL type
//           would back-reference to `0`.)
// size      107 bytes
// callers   10   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// HOMED 2026-09-06, homed from src/unrecovered/00628220.cpp - which carried
// five thousand scored spellings - and from the file-local stand-in this
// definition replaces (Caviar::init reached the image address through it)
// of the sum without reaching the image's 107 bytes. Nothing in that census
// is contradicted here; the body below is the faithful reading, not a new
// spelling of it.
//
// THE RECIPROCAL IS REAL. The image divides `1.0f` by the length
// (`fdivr dword ptr [0x670c08]`, 0x670C08 being a float 1.0f the engine
// keeps in data) and multiplies all three components by the answer, rather
// than dividing three times - so `scale` below is a real variable, not a
// transcription convenience.
//
// TRIED (archived census, 5760 scored units): no-locals src[i]*src[i] in every
//        order/association/operand-order, 69 of 107 bytes; locals x,y,z with
//        the tail reading source[], 69-76; the tail reading the locals, 85-95
//        with one stack slot too many; const source, /Oa, an __inline helper,
//        an aliasing store before the sum, dead stores, the sum in a variable
//        - none of them reproduce the image's two spill slots. /Op and /O1
//        both lose fsqrt to a call.
// TRIED (this pass, all 0-1 of 37 agreeing against the committed body's 1 of
//        37): `float v[3]` filled from source and read back per term - the
//        array is not enregistered either, and the frame pointer still comes
//        out where the image has `push ecx`; `float pair[2]` for y and z with
//        source[0] direct; y and z as the only locals with the sum's x term
//        reading source[0]; x,y,z locals with the tail reading the locals
//        (33 instructions against the image's 37). The two memory slots the
//        image spills y and z into are one `push ecx` and the CALLER'S OWN
//        ARGUMENT SLOT - VC6 reusing a dead parameter slot for a local - and
//        no spelling here produces it.
*/
void __cdecl vox_normalise_vector(const float *source, float *out) {
    float x = source[0];
    float y = source[1];
    float z = source[2];
    double length = sqrt(static_cast<double>(y) * y
                       + static_cast<double>(z) * z
                       + static_cast<double>(x) * x);
    // The original stores this reciprocal to an m32 stack slot before using it.
    // Volatile preserves that observed float32 conversion without narrowing the
    // preceding square root or changing the sequential source rereads below.
    volatile float scale = 1.0f / length;
    out[0] = source[0] * scale;
    out[1] = source[1] * scale;
    out[2] = source[2] * scale;
}
