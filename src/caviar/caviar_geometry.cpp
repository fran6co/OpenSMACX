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
#include "vox_engine.h"
#include "caviar_projection_state.h"
#include "vector.h"
#include <math.h>
#include <cmath>
// VC5 orders commutative x87 operands by translation-unit symbol state.
// With float.h included, vox_matrix_vector_mul and vox_cross_product take the
// image's operand order; the library's real include set is not known.
#include <float.h>

// ---------------------------------------------------------------------------
// The VOX library's vector/matrix math, 0x00627D00..0x00628377 in the image,
// in address order (vector_add and the vec3_* helpers moved here from
// vector.cpp: they sit inside this run, not beside the Vector class).
// Geometry shared by scene drawing and camera transformation. Matrix inputs
// use their real row bounds; output stores are sequential, with fresh input
// reads for each result, preserving the original overlap behavior.
// ---------------------------------------------------------------------------


// This helper uses x87 trig intrinsics, whereas the scene/light routines above
// call the runtime. Restore their function policy after this local body.
#pragma intrinsic(sin, cos)
// ORIGINAL: 0x00627D00 ?caviar_apply_rotation@@YAXPBMPAUVOX_Matrix@@@Z 0x00627D00-0x00627E19
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original body, complete seed extent, and fresh VC6 symbol observation; recovery/tests/test_caviar_geometry.py checks original expression trees and bounded host behavior.", "Inverse snapshots cofactor inputs before stores and has no singular guard; cross preserves sequential overlapping writes; rotation uses scoped x87 intrinsics and reads all angles before stores. Numerical spill precision is not established by these structural bindings."], "reason": "Source-owned voxel geometry with typed matrix rows and original arithmetic/store ordering."}
void __cdecl caviar_apply_rotation(const float *angles, VOX_Matrix *matrix) {
    float sx = (float)sin(angles[0]);
    float cx = (float)cos(angles[0]);
    float sy = (float)sin(angles[1]);
    float cy = (float)cos(angles[1]);
    float sz = (float)sin(angles[2]);
    float cz = (float)cos(angles[2]);
    float sz_sy = sz * sy;

    matrix->values[0][0] = cy * cx;
    matrix->values[0][1] = cx * sz_sy - cz * sx;
    matrix->values[0][2] = cx * (cz * sy) + sz * sx;
    matrix->values[1][0] = cy * sx;
    matrix->values[1][1] = sx * sz_sy + cz * cx;
    matrix->values[1][2] = sx * (cz * sy) - sz * cx;
    matrix->values[2][0] = -sy;
    matrix->values[2][1] = sz * cy;
    matrix->values[2][2] = cz * cy;
}
#pragma function(sin, cos)

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

// All cofactors are formed before any destination write. This preserves the
// inverse's in-place/overlapping behavior, unlike the sequential multiply.
// A zero determinant is not special-cased in the original arithmetic.
// ORIGINAL: 0x00627F50 ?vox_matrix_inverse@@YAXPAY02$$CBMPAY02M@Z 0x00627F50-0x006280DF
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original body, complete seed extent, and fresh VC6 symbol observation; recovery/tests/test_caviar_geometry.py checks original expression trees and bounded host behavior.", "Inverse snapshots cofactor inputs before stores and has no singular guard; cross preserves sequential overlapping writes; rotation uses scoped x87 intrinsics and reads all angles before stores. Numerical spill precision is not established by these structural bindings."], "reason": "Source-owned voxel geometry with typed matrix rows and original arithmetic/store ordering."}
void __cdecl vox_matrix_inverse(const float (*matrix)[3], float (*out)[3]) {
    float c00 = matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1];
    float c10 = matrix[1][0] * matrix[2][2] - matrix[2][0] * matrix[1][2];
    float c20 = matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1];
    float reciprocal = 1.0f / ((matrix[0][0] * c00 - c10 * matrix[0][1])
                              + c20 * matrix[0][2]);
    float c01 = matrix[0][1] * matrix[2][2] - matrix[0][2] * matrix[2][1];
    float c11 = matrix[0][0] * matrix[2][2] - matrix[0][2] * matrix[2][0];
    float c21 = matrix[0][0] * matrix[2][1] - matrix[0][1] * matrix[2][0];
    float c02 = matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1];
    float c12 = matrix[0][0] * matrix[1][2] - matrix[0][2] * matrix[1][0];
    float c22 = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    out[0][0] = reciprocal * c00;
    out[0][1] = -(reciprocal * c01);
    out[0][2] = reciprocal * c02;
    out[1][0] = -(reciprocal * c10);
    out[1][2] = -(reciprocal * c12);
    out[1][1] = reciprocal * c11;
    out[2][0] = reciprocal * c20;
    out[2][1] = -(reciprocal * c21);
    out[2][2] = reciprocal * c22;
}

// 0x006280E0. A 3x3 times a vector: three rows of three multiplies and a
// store, 107 bytes, no calls of its own.
// ORIGINAL: 0x006280E0 ?vox_matrix_vector_mul@@YAXPAY02$$CBMPBMPAM@Z 0x006280E0-0x0062814B BYTE_EXACT
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

/*
Purpose: Add two vectors component-wise into an output vector.
// ORIGINAL: 0x00628150 sub_628150 0x00628150-0x00628175 BYTE_EXACT
// symbol    ?vector_add@@YAXPBM0PAM@Z
// size      37 bytes
// prototype 
// callers   17   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void __cdecl vector_add(const float *left, const float *right, float *output) {
    float rx = right[0];
    output[0] = rx + left[0];
    output[1] = left[1] + right[1];
    output[2] = left[2] + right[2];
}

// Historical source note: original x87 subtracts each pair and rounds only at the float store.
// ORIGINAL: 0x00628180 sub_628180 0x00628180-0x006281A5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 00628180 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_00628180_redirect@@YAXPBM0PAM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_subtract@@YAXPBM0PAM@Z
// kind      game
void __cdecl vec3_subtract(const float *left, const float *right,
                             float *result) {
    result[0] = left[0] - right[0];
    result[1] = left[1] - right[1];
    result[2] = left[2] - right[2];
}

// Historical source note: bind each source component before its multiply; this preserves
// source-before-store behavior when result aliases source and retains the original operand scheduling.
// ORIGINAL: 0x006281B0 sub_6281b0 0x006281B0-0x006281D5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 006281B0 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_006281b0_redirect@@YAXPBMMPAM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_scale@@YAXPBMMPAM@Z
// kind      game
void __cdecl vec3_scale(const float *source, float scale,
                         float *result) {
    const float x = source[0];
    result[0] = x * scale;
    const float y = source[1];
    result[1] = y * scale;
    const float z = source[2];
    result[2] = z * scale;
}

// Historical body note: the original stores y/z scratch slots before reloading them;
// the LEVER annotation below retains the measured register/spill exploration.
// ORIGINAL: 0x006281E0 sub_6281e0 0x006281E0-0x00628217 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original body 006281E0 is a cdecl three-float vector operation; see its preserved ORIGINAL span and instruction claim","prior transitional source spelling ?leaf_006281e0_redirect@@YAMPBM@Z is retained here as history only, not asserted as an original debug symbol","pinned direct E8 callsites span vehicle setup/drawing and Caviar projection; source counterparts pass float[3]/VOX_Vect::values buffers rather than Vector objects","src/vector.cpp already owns 0x00628150 vector_add with the same cdecl float-array shape; this is module co-location, not a claim of original TU or historical source name","the selected body keeps component/store order and existing overlap behavior; the scale locals preserve source-before-store aliasing; the length expression retains the original extended-precision accumulation order"],"reason":"Rehome the mapped free Vec3 operation into the existing vector-array module. The operation-derived API name does not assert an original class or source spelling; this binding records current ownership evidence only."}
// symbol    ?vec3_length@@YAMPBM@Z
// kind      game
// LEVER: under VC5, plain x/y/z locals summed y*y + z*z + x*x match; VC5
//        itself spills y and z to [esp] and the dead argument slot.
#if defined(_MSC_VER) && _MSC_VER <= 1200
// The pinned free operation uses inline fsqrt; Vector::normalize keeps the
// translation unit's CRT sqrt policy before and after this one definition.
#pragma intrinsic(sqrt)
#endif
float __cdecl vec3_length(const float *vector) {
    float x = vector[0];
    float y = vector[1];
    float z = vector[2];
    return (float)sqrt(y * y + z * z + x * x);
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
    // The image inlines vec3_length; the library is /Ob1, so the original
    // wrote it out. Written out as `float x/y/z` locals with
    // `1.0f / (float)sqrt((double)(y * y + z * z + x * x))`, this body is
    // exact, but the removed call shifts VC5's symbol state and flips
    // vox_cross_product's operand order (recovery/COMPILER_EVIDENCE.md,
    // "Symbol state"). The call stays until the file's state is resolved.
    // The components are re-read from source, so out may alias source.
    float scale = 1.0f / vec3_length(source);
    out[0] = source[0] * scale;
    out[1] = source[1] * scale;
    out[2] = source[2] * scale;
}

// Each component is stored before the next one's input reads; retain that
// order when an output vector overlaps an input vector.
// ORIGINAL: 0x00628290 ?vox_cross_product@@YAXPBM0PAM@Z 0x00628290-0x006282D1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original body, complete seed extent, and fresh VC6 symbol observation; recovery/tests/test_caviar_geometry.py checks original expression trees and bounded host behavior.", "Inverse snapshots cofactor inputs before stores and has no singular guard; cross preserves sequential overlapping writes; rotation uses scoped x87 intrinsics and reads all angles before stores. Numerical spill precision is not established by these structural bindings."], "reason": "Source-owned voxel geometry with typed matrix rows and original arithmetic/store ordering."}
void __cdecl vox_cross_product(const float *left, const float *right, float *out) {
    out[0] = left[1] * right[2] - right[1] * left[2];
    out[1] = right[0] * left[2] - left[0] * right[2];
    out[2] = left[0] * right[1] - right[0] * left[1];
}



// 0x006282E0. Three arguments, a byte answer - the draw tests `al` and skips
// the part when it is non-zero.
OPENSMACX_NOINLINE
// Historical seed proposal; current ABI/field interpretation is stated below.
// PROPOSAL: signature -> extern "C" bool __cdecl sub_6282e0(float *a1,
// float a2, int a3). Evidence: `ret` with no epilogue clears only `al`
// on every branch (`mov al,1`/`xor al,al`), never `eax` - the callee-pop
// convention for a `bool` return under this compiler, not `int` (which
// would clear all of eax). Stack reads at [esp+4]/[esp+8]/[esp+0xc] with
// no prior push are three plain __cdecl arguments (float*, float, int),
// matching the Ghidra hint's arity.
// symbol    ?vox_part_behind@@YA_NPBUVOX_Vect@@MPBUVoxCameraBlock@@@Z
// ORIGINAL: 0x006282E0 sub_6282e0 0x006282E0-0x00628377
// symbol    ?vox_part_behind@@YA_NPBUVOX_Vect@@MPBUVoxCameraBlock@@@Z
bool __cdecl vox_part_behind(const VOX_Vect *position, float extent,
                             const VoxCameraBlock *view) {
    const CaviarChannelBounds &clip = view->bounds_;
    if (position->values[2] < clip.lead_[3]) return true;
    if (position->values[2] > clip.lead_[4]) return true;
    float delta = extent * CaviarSceneProjectionScratch.projected_scale_;
    delta *= CaviarBoundsRadiusScale;
    if (position->values[0] < -delta) return true;
    if (position->values[0] >
        static_cast<float>(clip.values_[4]) + delta) return true;
    if (position->values[1] < -delta) return true;
    if (position->values[1] >
        static_cast<float>(clip.values_[5]) + delta) return true;
    return false;
}
