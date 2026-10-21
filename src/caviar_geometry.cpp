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

