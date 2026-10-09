/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x00456100 ??1CaviarData@@QAE@XZ 0x00456100-0x00456105 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original destructor span and its direct close target.", "Original model arrays and renderer embedding EH cleanup distinguish loaded-model and instance lifetimes.", "Fresh VC6 observations under both configured flag sets identify this natural emission host and preserve normal versus EH call targets."], "reason": "Genuine header-defined destructor in src/caviar.h; this translation unit naturally emits the original five-byte cleanup tail through real object lifetime use, without synthetic calls or forced exports."}
// body      src/caviar.h
#include "stdafx.h"
#include "caviar.h"
#include "basepop.h"
#include "buffer.h"
#include "palette.h"
#include "general.h"    // mem_get
#include "stringstruct.h"
#include "strings.h"    // StringTemp
#include "vector.h"     // vector_add - the engine's geometry accumulator

#include <io.h>        // _read/_write/_lseek/_tell - the vx_* IO shims
#include <new>
#include <stddef.h>

// Forward declaration: the record dispatchers precede the retained kernel
// definition in this translation unit.
static void caviar_install_record_kernels_candidate(
    VoxRenderRecord *, uint8_t, int);
void __cdecl caviar_original_666fd0(
    int32_t, int32_t, uint32_t, const uint8_t *, int32_t);

// These three contiguous dword slots form a useful typed source grouping.
// All four blitters share the destination cursor. Only the transparent and
// 16-bit scaled loops share destination_stride_bytes_; the two 8-bit scaled
// loops use their stride parameters. Only the transparent loop consumes
// source_row_bytes_. This does not assert original extent or class.
struct CaviarBlitStateCandidate {
    unsigned char * volatile destination_cursor_; // 0x00698B70
    uint32_t source_row_bytes_;                   // 0x00698B74
    uint32_t destination_stride_bytes_;           // 0x00698B78
};
typedef char CaviarBlitStateCandidate_is_12_bytes[
    sizeof(CaviarBlitStateCandidate) == 12 ? 1 : -1];
typedef char CaviarBlitStateCandidate_cursor_at_0[
    offsetof(CaviarBlitStateCandidate, destination_cursor_) == 0 ? 1 : -1];
typedef char CaviarBlitStateCandidate_source_pitch_at_4[
    offsetof(CaviarBlitStateCandidate, source_row_bytes_) == 4 ? 1 : -1];
typedef char CaviarBlitStateCandidate_destination_pitch_at_8[
    offsetof(CaviarBlitStateCandidate, destination_stride_bytes_) == 8 ? 1 : -1];

static uint32_t caviar_blit_offset(int x, int y, int stride) {
    return static_cast<uint32_t>(y) * static_cast<uint32_t>(stride) +
           static_cast<uint32_t>(x);
}

static unsigned char *caviar_blit_address(unsigned char *base,
                                           uint32_t offset) {
    const uint32_t address =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(base)) + offset;
    return reinterpret_cast<unsigned char *>(static_cast<uintptr_t>(address));
}

static const char *caviar_blit_source_address(const char *base,
                                              uint32_t offset) {
    const uint32_t address =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(base)) + offset;
    return reinterpret_cast<const char *>(static_cast<uintptr_t>(address));
}

// Original readonly IEEE754 scalar is0xC61C4000 (-10000.0f), with no relocation.
// Datalink and SelectPartWin both multiply their camera vectors by this storage.
// RECOVERY_OBJECT: v2 0x0066AA40 0x0066AA44 ?CaviarCameraDistance@@3MB 0x00429300
const float CaviarCameraDistance = -10000.0f;

// Shared accessed windows for the 6404F0 producer, raster/scalar/shadow
// consumers, and stage-error writer. Their source types retain the known
// member identities qualified by the fresh standalone Caviar COFF/CodeView
// object. The 0x200 parent remains only an observed access window.
// Historical split-view notes about stores around vox_scene_extent_ remain
// review debt; grouping here proposes typed ownership, not original extent,
// lifetime or final-linked placement.
// RECOVERY_OBJECT: v3 0x009C0840 ?CaviarProjectionCells@@3PAUCaviarProjectionCell@@A member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.cells_
// RECOVERY_OBJECT: v3 0x009C09E0 ?CaviarProjectionScratchE0@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_e0_
// RECOVERY_OBJECT: v3 0x009C09E4 ?CaviarProjectionScratchE4@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_e4_
// RECOVERY_OBJECT: v3 0x009C09E8 ?CaviarProjectionScratchE8@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_e8_
// RECOVERY_OBJECT: v3 0x009C09EC ?CaviarProjectionScratchEC@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_ec_
// RECOVERY_OBJECT: v3 0x009C09F0 ?CaviarProjectionScratchF0@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_f0_
// RECOVERY_OBJECT: v3 0x009C09F4 ?CaviarProjectionScratchF4@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_f4_
// RECOVERY_OBJECT: v3 0x009C09FC ?CaviarProjectionScratchFC@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_fc_
// RECOVERY_OBJECT: v3 0x009C0A00 ?CaviarProjectionScratchA00@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_a00_
// RECOVERY_OBJECT: v3 0x009C0A04 ?CaviarProjectionScratchA04@@3MA member ?CaviarKernelProjectionWindow@@3TCaviarKernelProjectionWindowCandidate@@A current_views_.tail_.scratch_a04_
// RECOVERY_OBJECT: v3 0x009C0B90 ?CaviarProjectionOutputWords@@3PAIA member ?CaviarOutputMessageWindow@@3UCaviarOutputMessageWindowCandidate@@A projection_words_
CaviarKernelProjectionWindowCandidate CaviarKernelProjectionWindow = {0};
CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {0};
// RECOVERY_OBJECT: v1 0x009C0A40 ?CaviarFixedPointProjection@@3UCaviarFixedPointProjectionMatrix@@A
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {0}; // 0x009C0A40
// RECOVERY_OBJECT: v1 0x009C0A7C ?CaviarProjectionScaledMatrix@@3PAMA
float CaviarProjectionScaledMatrix[9] = {0}; // 0x009C0A7C

// Separate direction-helper cells whose consumers share the existing
// projection matrix/state rather than owning an invented scratch aggregate.
// RECOVERY_OBJECT: v1 0x009C0AA0 ?vox_projection_matrix_@@3UVOX_Matrix@@A
VOX_Matrix vox_projection_matrix_ = {0};    // 0x009C0AA0
// RECOVERY_OBJECT: v1 0x009C0A70 ?vox_projection_component_@@3PAHA
int32_t vox_projection_component_[3] = {0}; // 0x009C0A70..0x009C0A78
// RECOVERY_OBJECT: v1 0x009C0AC4 ?vox_projection_render_stage_@@3HA
int32_t vox_projection_render_stage_ = 0;   // 0x009C0AC4
// RECOVERY_OBJECT: v1 0x009C0AC8 ?vox_projection_prepared_@@3HA
int32_t vox_projection_prepared_ = 0;       // 0x009C0AC8
// RECOVERY_OBJECT: v1 0x009C0B74 ?vox_projection_octant_mask_@@3HA
int32_t vox_projection_octant_mask_ = 0;    // 0x009C0B74
// RECOVERY_OBJECT: v1 0x00671038 ?CaviarProjectionDirectionScale@@3MB
const float CaviarProjectionDirectionScale = 256.0f; // 0x00671038
// RECOVERY_OBJECT: v1 0x0067103C ?CaviarProjectionRoundThreshold@@3MB
const float CaviarProjectionRoundThreshold = 0.0f;   // 0x0067103C
// RECOVERY_OBJECT: v1 0x00671040 ?CaviarProjectionRoundBelow@@3MB
const float CaviarProjectionRoundBelow = -0.5f;      // 0x00671040
// RECOVERY_OBJECT: v1 0x00671044 ?CaviarProjectionRoundAbove@@3MB
const float CaviarProjectionRoundAbove = 0.5f;       // 0x00671044
// RECOVERY_OBJECT: v1 0x00670C18 ?CaviarBoundsRadiusScale@@3MB
const float CaviarBoundsRadiusScale = 128.0f;        // 0x00670C18

#include "caviar_projection_state.h"

#pragma once

#include "caviar.h"

// The exact observed C++ view windows used by 63FE00/63FFE0. These are
// separate objects with address starts 0x9C0AA0 (matrix), 0x9C0A70 (three
// rounded coordinates), 0x9C0AC4 (stage word), 0x9C0AC8 (prepared word), and
// 0x9C0B74 (octant mask). Wider ownership/lifetime is not inferred here.
extern VOX_Matrix vox_projection_matrix_;
extern int32_t vox_projection_component_[3];
extern int32_t vox_projection_render_stage_;
extern int32_t vox_projection_prepared_;
extern int32_t vox_projection_octant_mask_;

// Pinned read-only scalar values consumed by these direction helpers.
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
extern const float CaviarProjectionAngleLimit;
extern const float CaviarProjectionAngleWrap;
extern const float CaviarProjectionPolarScale;
extern const float CaviarProjectionAzimuthScale;

// The source owner must provide these 24 immutable byte rows through the
// existing data-preparation mechanism. Their row contents are original data,
// not a formula derivable from the current body. Row cardinality and extent
// come from the pinned relocation table and contiguous target storage.
typedef uint8_t CaviarProjectionAngleRowCandidate[0x860];
extern const CaviarProjectionAngleRowCandidate CaviarProjectionAngleData[24];
extern const CaviarProjectionAngleRowCandidate *CaviarProjectionAngleRows[24];
extern const uint8_t CaviarProjectionZeroByte;
// 0x9BE6A0 stores an actual pointer into the selected row; the index may be
// an interior byte selected by the 0x59-angle subtraction. It is not an
// integer encoding to be reinterpreted by downstream consumers.
extern const uint8_t *vox_projection_selected_angle_;

// Provisional source objects for the direction-helper access windows. The
// native writer must bind each object independently after its fresh compile;
// these declarations do not claim wider original capacities.
// The already mapped direction cells and rounding scalars above remain the
// sole storage owners; these new helpers consume them directly.
// RECOVERY_OBJECT: v1 0x00671048 ?CaviarProjectionAngleLimit@@3MB
const float CaviarProjectionAngleLimit = 1.0f;                 // 0x00671048
// RECOVERY_OBJECT: v1 0x0067104C ?CaviarProjectionAngleWrap@@3MB
const float CaviarProjectionAngleWrap = -6.283185482025146484375f; // 0x0067104C
// RECOVERY_OBJECT: v1 0x00671050 ?CaviarProjectionPolarScale@@3MB
const float CaviarProjectionPolarScale = -14.64225482940673828125f; // 0x00671050
// RECOVERY_OBJECT: v1 0x00671054 ?CaviarProjectionAzimuthScale@@3MB
const float CaviarProjectionAzimuthScale = -14.1647891998291015625f; // 0x00671054

// The existing preparation hook materializes exactly the reviewed 24*0x860
// file-backed bytes. This source-owned byte owner has an accessed-window
// extent, not an asserted original allocation boundary.
// RECOVERY_OBJECT: v1 0x00699100 ?CaviarProjectionAngleData@@3QAY0IGA@$$CBEA
const CaviarProjectionAngleRowCandidate CaviarProjectionAngleData[24] = {
#include "caviar_angle_rows.inc"
};

// RECOVERY_OBJECT: v1 0x006A5A00 ?CaviarProjectionAngleRows@@3PAPAY0IGA@$$CBEA
const CaviarProjectionAngleRowCandidate *CaviarProjectionAngleRows[24] = {
    &CaviarProjectionAngleData[0],  &CaviarProjectionAngleData[1],
    &CaviarProjectionAngleData[2],  &CaviarProjectionAngleData[3],
    &CaviarProjectionAngleData[4],  &CaviarProjectionAngleData[5],
    &CaviarProjectionAngleData[6],  &CaviarProjectionAngleData[7],
    &CaviarProjectionAngleData[8],  &CaviarProjectionAngleData[9],
    &CaviarProjectionAngleData[10], &CaviarProjectionAngleData[11],
    &CaviarProjectionAngleData[12], &CaviarProjectionAngleData[13],
    &CaviarProjectionAngleData[14], &CaviarProjectionAngleData[15],
    &CaviarProjectionAngleData[16], &CaviarProjectionAngleData[17],
    &CaviarProjectionAngleData[18], &CaviarProjectionAngleData[19],
    &CaviarProjectionAngleData[20], &CaviarProjectionAngleData[21],
    &CaviarProjectionAngleData[22], &CaviarProjectionAngleData[23]
};

// Pinned original initial bytes are zero. Runtime direction selection writes
// an interior pointer into one of the owned immutable row windows.
// RECOVERY_OBJECT: v1 0x009BE6A0 ?vox_projection_selected_angle_@@3PBEB
const uint8_t *vox_projection_selected_angle_ = 0;

// Typed literals preserve the original data objects; declarations receive
// fresh RECOVERY_OBJECT mappings only after native compiler measurement.
extern const float CaviarProjectionZero;
extern const float CaviarProjectionMinusOne;
extern const float CaviarProjectionOne;
extern const float CaviarProjectionHalfPixel;
extern const float CaviarProjectionNearNegativeEpsilon;
extern const float CaviarProjectionNearPositiveEpsilon;
// RECOVERY_OBJECT: v1 0x00670FF8 ?CaviarProjectionZero@@3MB
const float CaviarProjectionZero = 0.0f;              // 0x00670FF8
// RECOVERY_OBJECT: v1 0x00670FFC ?CaviarProjectionMinusOne@@3MB
const float CaviarProjectionMinusOne = -1.0f;         // 0x00670FFC
// RECOVERY_OBJECT: v1 0x00671000 ?CaviarProjectionOne@@3MB
const float CaviarProjectionOne = 1.0f;               // 0x00671000
// RECOVERY_OBJECT: v1 0x00671004 ?CaviarProjectionHalfPixel@@3MB
const float CaviarProjectionHalfPixel = -0.5f;        // 0x00671004
// RECOVERY_OBJECT: v1 0x00671008 ?CaviarProjectionNearNegativeEpsilon@@3MB
const float CaviarProjectionNearNegativeEpsilon = -1.0e-6f; // 0x00671008
// RECOVERY_OBJECT: v1 0x0067100C ?CaviarProjectionNearPositiveEpsilon@@3MB
const float CaviarProjectionNearPositiveEpsilon = 1.0e-6f;  // 0x0067100C

// This mutable global starts with native rows (1,0,.1),(0,1,.1),(0,0,0).
// The builder changes cells [0][2] and [2][2], leaving [1][2] untouched.
// RECOVERY_OBJECT: v1 0x00698A78 ?CaviarProjectionShadowBasis@@3UVOX_Matrix@@A
VOX_Matrix CaviarProjectionShadowBasis = {
  {
    {1.0f, 0.0f, 0.1f},
    {0.0f, 1.0f, 0.1f},
    {0.0f, 0.0f, 0.0f}
  }
}; // 0x00698A78

namespace {

// /O2 implies /Oi, and /Oi expands `memset` into the `rep stosd` form at
// every site. render_mono's image CALLS it (`push 0x1800 / push <byte> /
// push 0x9b9c38 / call 0x6465f0`), so the expansion is routed through a real
// function - path.cpp's `memset_call` idiom, kept file-local here because
// path.cpp already owns the external name.
void *__cdecl caviar_memset_call(void *dst, int value, unsigned count) {
    return memset(dst, value, count);
}

// 0x00639380, the engine's shutdown hook, is a nullsub - a function whose
// whole body is one `ret`. close_class calls it, so a body that does nothing
// is the faithful spelling; kept out of line so the call survives.
OPENSMACX_NOINLINE
// ORIGINAL: 0x00639380 nullsub_16 0x00639380-0x00639381 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00639380-0x00639381 is a single RET", "source:src/caviar.cpp Caviar::close_class calls vox_engine_close for the original shutdown hook", "pinned/current owner facts identify one original caller; the source body remains deliberately empty"], "reason": "Rehome the no-op shutdown-hook identity onto the already-called, out-of-line vox_engine_close owner; no separate adapter body is retained."}
// symbol    ?vox_engine_close@?%src/caviar.cpp@@YAXXZ
// kind      game
void vox_engine_close() {
}

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
// body src/caviar.h

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

// ORIGINAL: 0x00616B20 sub_616b20 0x00616B20-0x00616B2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00616B20-0x00616B2D returns ECX in EAX after zeroing dwords +0/+4/+8", "source:src/caviar.h VoxInstance is exactly 0xC with three pointer members and RECOVERY_MEMBER anchors at +4/+8", "pinned:0x00616DA0 initializes 200 twelve-byte slot records inline at Caviar+0x74C; it does not directly call 0x00616B20"], "reason": "Own the three pointer-zeroing stores on the existing 0xC VoxInstance type. Moving its inline constructor out of the header exposes this source definition but changes the Caviar array-construction code shape; constructor return-register/ABI and all callers remain native-review obligations."}
// symbol    ??0VoxInstance@@QAE@XZ
// kind      game
VoxInstance::VoxInstance()
    : instance_colour_(0), instance_vector_(0),
      instance_matrix_(0) {
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
// ORIGINAL: 0x00616DA0 ??0Caviar@@QAE@XZ 0x00616DA0-0x00616DD6 SEMANTIC
// LEVER: 0 of 16 -> SHAPE_EXACT 12 of 16, on two changes that are one idea. The image never memsets `slots_`: it runs a 200-turn loop writing three zero dwords per element, which is `CaviarData::CaviarData()` FOLDED into the implicit array construction. This body was doing both - the compiler emitted a 200-turn loop of `call ?CaviarData::CaviarData` because that constructor was out-of-line in caviar.cpp, AND the source then memset the same storage with `rep stosd`, 25 instructions against the image's 16. Moving the constructor in-class in caviar.h (`MEASURED`, so 0x00616BC0 keeps its own COMDAT and stays BYTE_EXACT) lets VC6 inline it, and deleting the memset leaves exactly the image's loop.
// TRIED: the residual 4 instructions are a pointer bias inside VC6's array-construction loop and no source form here reaches it. The image walks `[eax + 0x750]` and stores at `[edx - 4]`, `[edx]`, `[edx + 4]`; this tree walks `[eax + 0x74c]` and stores at `[edx]`, `[edx + 4]`, `[edx + 8]`. Same element, same order, same encoded length - only the base is biased by 4. listing_diff reports 0 differing runs, which is why this is SHAPE_EXACT rather than a structural mismatch.
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
    const uint32_t default_scene_scale = 0x3F2AAAAB;
    field_108_ = 0x461C4000;
    field_13CC_ = 0;
    memcpy(&scene_scale_, &default_scene_scale, sizeof(scene_scale_));
}

/*
Purpose: Copy a camera position and orientation directly into the renderer state.
// ORIGINAL: 0x006182A0 ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z 0x006182A0-0x0061831F BYTE_EXACT
// LEVER: TWELVE ELEMENT COPIES, no struct assignment anywhere. Every store in the image is `mov [ecx + disp32], reg` off the incoming `this` - 0xA5, 0xA9, 0xAD for the vector and 0xB1 through 0xD1 for the matrix - so nothing ever takes the address of a subobject. `camera_view_.camera_.position = *camera` (12 bytes) forms `lea edx, [ecx + 0xa5]` and indexes off it, and `camera_view_.camera_.rotation = *matrix` (36 bytes) is over VC6's inline-copy threshold and becomes `rep movsd`, which also costs the EDI/ESI pair and moves the two null tests behind the pushes: 21 compiled instructions against 33, agreeing 0. Assigning the twelve floats one at a time gives 33 of 33, BYTE_EXACT.
// TRIED: MEASURED, all with the same 5 of 33 - three 12-byte `memcpy`s for the matrix rows (42 compiled), a `for (row)` loop over three row memcpys (34), splitting `camera && matrix` into two guard clauses (42), and keeping the struct assignment for `position` while spelling the nine matrix floats out (36). It is the struct assignment that costs it, not the copy width and not the chained condition - the image really does test both and share one `ret 8`.
// TRIED: `record` DROPPED THIS LINE the first time. Re-recording a body whose tier changes rewrites its annotation from the parsed state and keeps only the LEVER; the TRIED text is discarded silently. Re-added after the claim was banked, and verified with `osmx show` rather than by reading the diff.
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
        camera_view_.camera_.position.values[0] = camera->values[0];
        camera_view_.camera_.position.values[1] = camera->values[1];
        camera_view_.camera_.position.values[2] = camera->values[2];
        camera_view_.camera_.rotation.values[0][0] = matrix->values[0][0];
        camera_view_.camera_.rotation.values[0][1] = matrix->values[0][1];
        camera_view_.camera_.rotation.values[0][2] = matrix->values[0][2];
        camera_view_.camera_.rotation.values[1][0] = matrix->values[1][0];
        camera_view_.camera_.rotation.values[1][1] = matrix->values[1][1];
        camera_view_.camera_.rotation.values[1][2] = matrix->values[1][2];
        camera_view_.camera_.rotation.values[2][0] = matrix->values[2][0];
        camera_view_.camera_.rotation.values[2][1] = matrix->values[2][1];
        camera_view_.camera_.rotation.values[2][2] = matrix->values[2][2];
    }
}

/*
Purpose: Set the Caviar renderer scaling factor.
// ORIGINAL: 0x006183B0 ?set_scaling@Caviar@@QAEXM@Z 0x006183B0-0x006183BD BYTE_EXACT
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
    memcpy(&camera_view_.camera_.scaling, &scaling, sizeof(scaling));
}

void Caviar::set_scaling_bits(uint32_t scaling_bits) {
    memcpy(&camera_view_.camera_.scaling, &scaling_bits, sizeof(scaling_bits));
}

/*
Purpose: Read the Caviar renderer scaling factor.
// ORIGINAL: 0x006183C0 ?get_scaling@Caviar@@QAEMXZ 0x006183C0-0x006183C7 BYTE_EXACT
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
    return camera_view_.camera_.scaling;
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
void Caviar::UNK8(int menu_id) {
    field_108_ = menu_id;
}

/*
Purpose: Store three values into the triplet at 0x2C.
// ORIGINAL: 0x00618320 ?UNK10@Caviar@@QAEXHHH@Z 0x00618320-0x00618338 BYTE_EXACT
// symbol    ?UNK10@Caviar@@QAEXMMM@Z
//           (the three arguments are typed float because the fields they fill
//           are the floats render subtracts the camera offset from; the
//           catalogue spells them H, and either way the store is one dword.)
// size      24 bytes
// prototype void (__thiscall ?UNK10@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK10(float offset_x, float offset_y, float offset_z) {
    scene_context_.state_.scene_offset_.values[0] = offset_x;
    scene_context_.state_.scene_offset_.values[1] = offset_y;
    scene_context_.state_.scene_offset_.values[2] = offset_z;
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
    caviar_apply_rotation(angles, &scene_context_.state_.scene_rotation_);
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
// symbol    ?UNK11@Caviar@@QAEXPAM00@Z
//           (the three outputs are typed float*, the same dwords the image
//           copies - its catalogue spells them H, which no pointer can be.)
// size      42 bytes
// prototype void (__thiscall ?UNK11@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK11(float *out1, float *out2, float *out3) {
    if (out1) {
        *out1 = scene_context_.state_.scene_offset_.values[0];
    }
    if (out2) {
        *out2 = scene_context_.state_.scene_offset_.values[1];
    }
    if (out3) {
        *out3 = scene_context_.state_.scene_offset_.values[2];
    }
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
// The image visits all 200 entries in increasing address order, freeing and
// clearing each entry's colour, vector and matrix allocations in that order.
Return Value: n/a
Status: Complete
*/
void Caviar::close() {
    for (int index = 0; index < 200; ++index) {
        if (slots_[index].instance_colour_ != nullptr) {
            free(slots_[index].instance_colour_);
            slots_[index].instance_colour_ = nullptr;
        }
        if (slots_[index].instance_vector_ != nullptr) {
            free(slots_[index].instance_vector_);
            slots_[index].instance_vector_ = nullptr;
        }
        if (slots_[index].instance_matrix_ != nullptr) {
            free(slots_[index].instance_matrix_);
            slots_[index].instance_matrix_ = nullptr;
        }
    }

    field_4_ = 0;
    scene_scale_ = 2.0f / 3.0f;
}

// ====================
// turn_start addresses this renderer at 0x008CC828 and its light/shadow
// vectors at +0x60/+0x6C. It is the existing C++ static owner, not an alias.
// RECOVERY_OBJECT: v1 0x008CC828 ?g_VEHDRAW_CAVIAR@@3VCaviar@@A
Caviar g_VEHDRAW_CAVIAR;
Buffer g_CAVIAR_BUFFER_1;
Buffer g_CAVIAR_BUFFER_2;

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
CaviarData g_UNUSED_CAVIARDATA_VAR1;  // 0x00791D68
// RECOVERY_OBJECT: v1 0x007ACCA0 ?g_SSF_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SSF_CAVIARDATA;  // 0x007ACCA0
// RECOVERY_OBJECT: v1 0x0079A6A0 ?g_SDP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SDP_CAVIARDATA;  // 0x0079A6A0
// RECOVERY_OBJECT: v1 0x00787E48 ?g_SAS_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SAS_CAVIARDATA;  // 0x00787E48
// RECOVERY_OBJECT: v1 0x00799228 ?g_SCD_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SCD_CAVIARDATA;  // 0x00799228
// RECOVERY_OBJECT: v1 0x007ACC90 ?g_SCJ_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SCJ_CAVIARDATA;  // 0x007ACC90
// RECOVERY_OBJECT: v1 0x0077A578 ?g_SAGS_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SAGS_CAVIARDATA;  // 0x0077A578
// RECOVERY_OBJECT: v1 0x0079A6B0 ?g_SFT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SFT_CAVIARDATA;  // 0x0079A6B0
// RECOVERY_OBJECT: v1 0x007765B0 ?g_VHR_CAVIARDATA1@@3UCaviarData@@A
CaviarData g_VHR_CAVIARDATA1;  // 0x007765B0
// RECOVERY_OBJECT: v1 0x0078A5E0 ?g_SHT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SHT_CAVIARDATA;  // 0x0078A5E0
// RECOVERY_OBJECT: v1 0x0078A300 ?g_SRB_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SRB_CAVIARDATA;  // 0x0078A300
// RECOVERY_OBJECT: v1 0x007AC030 ?g_ASAS_CAVIARDATA@@3UCaviarData@@A
CaviarData g_ASAS_CAVIARDATA;  // 0x007AC030
// RECOVERY_OBJECT: v1 0x007ACCC0 ?g_RESLASER_CAVIARDATA@@3UCaviarData@@A
CaviarData g_RESLASER_CAVIARDATA;  // 0x007ACCC0
// RECOVERY_OBJECT: v1 0x007A7900 ?g_RESBOLT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_RESBOLT_CAVIARDATA;  // 0x007A7900
// RECOVERY_OBJECT: v1 0x007ACC10 ?g_FUNLOAD_CAVIARDATA@@3UCaviarData@@A
CaviarData g_FUNLOAD_CAVIARDATA;  // 0x007ACC10
// RECOVERY_OBJECT: v1 0x00788028 ?g_TECLOAD_CAVIARDATA@@3UCaviarData@@A
CaviarData g_TECLOAD_CAVIARDATA;  // 0x00788028
// RECOVERY_OBJECT: v1 0x00799E38 ?g_SP_DISSWAVE_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SP_DISSWAVE_CAVIARDATA;  // 0x00799E38
// RECOVERY_OBJECT: v1 0x007792A0 ?g_SP_MARINED_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SP_MARINED_CAVIARDATA;  // 0x007792A0
// RECOVERY_OBJECT: v1 0x0075AEF0 ?g_SP_NANOO_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SP_NANOO_CAVIARDATA;  // 0x0075AEF0
// RECOVERY_OBJECT: v1 0x007AD130 ?g_SP_SOPORIFIC_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SP_SOPORIFIC_CAVIARDATA;  // 0x007AD130
// RECOVERY_OBJECT: v1 0x007AD040 ?g_AA01_CAVIARDATA@@3UCaviarData@@A
CaviarData g_AA01_CAVIARDATA;  // 0x007AD040
// RECOVERY_OBJECT: v1 0x0075AFC0 ?g_AA_ROVER_CAVIARDATA@@3UCaviarData@@A
CaviarData g_AA_ROVER_CAVIARDATA;  // 0x0075AFC0
// RECOVERY_OBJECT: v1 0x00779288 ?g_AA_WING_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_AA_WING_CAVIARDATA[2];  // 0x00779288, 0xc stride
// RECOVERY_OBJECT: v1 0x0078A610 ?g_ACP_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_ACP_CAVIARDATA[4];  // 0x0078A610, 0xc stride
// RECOVERY_OBJECT: v1 0x0079A6F0 ?g_AX_CAVIARDATA@@3UCaviarData@@A
CaviarData g_AX_CAVIARDATA;  // 0x0079A6F0
// RECOVERY_OBJECT: v1 0x00789A50 ?g_AA_CAVIARDATA@@3UCaviarData@@A
CaviarData g_AA_CAVIARDATA;  // 0x00789A50
// RECOVERY_OBJECT: v1 0x0078A600 ?g_ACOLPOD_CAVIARDATA@@3UCaviarData@@A
CaviarData g_ACOLPOD_CAVIARDATA;  // 0x0078A600
// RECOVERY_OBJECT: v1 0x00787E60 ?g_AT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_AT_CAVIARDATA;  // 0x00787E60
// RECOVERY_OBJECT: v1 0x007880C0 ?g_VTA_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VTA_CAVIARDATA;  // 0x007880C0
// RECOVERY_OBJECT: v1 0x007ABFC0 ?g_ATP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_ATP_CAVIARDATA;  // 0x007ABFC0
// RECOVERY_OBJECT: v1 0x0078A510 ?g_SSFA_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SSFA_CAVIARDATA;  // 0x0078A510
// RECOVERY_OBJECT: v1 0x0079A640 ?g_SFTA_CAVIARDATA@@3UCaviarData@@A
CaviarData g_SFTA_CAVIARDATA;  // 0x0079A640
// RECOVERY_OBJECT: v1 0x007871C0 ?g_VW00_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VW00_CAVIARDATA;  // 0x007871C0
// RECOVERY_OBJECT: v1 0x0079A700 ?g_VIPTAWL_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VIPTAWL_CAVIARDATA;  // 0x0079A700
// RECOVERY_OBJECT: v1 0x007AB3F0 ?g_VIPTASGN_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VIPTASGN_CAVIARDATA;  // 0x007AB3F0
// RECOVERY_OBJECT: v1 0x007795A0 ?g_VIPTAPSI_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VIPTAPSI_CAVIARDATA;  // 0x007795A0
// RECOVERY_OBJECT: v1 0x00799628 ?g_VIPTR_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VIPTR_CAVIARDATA[4];  // 0x00799628, 0xc stride
// RECOVERY_OBJECT: v1 0x007791D0 ?g_VIPTA_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VIPTA_CAVIARDATA[2];  // 0x007791D0, 0xc stride
// RECOVERY_OBJECT: v1 0x0078A500 ?g_PTMOD_CAVIARDATA@@3UCaviarData@@A
CaviarData g_PTMOD_CAVIARDATA;  // 0x0078A500
// RECOVERY_OBJECT: v1 0x00779230 ?g_VB_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VB_CAVIARDATA;  // 0x00779230
// RECOVERY_OBJECT: v1 0x0078E968 ?g_VBP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VBP_CAVIARDATA;
// RECOVERY_OBJECT: v1 0x0075AED0 ?g_VGMC_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGMC_CAVIARDATA;  // 0x0075AED0
// RECOVERY_OBJECT: v1 0x0076EBE0 ?g_VGMCP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGMCP_CAVIARDATA;  // 0x0076EBE0
// RECOVERY_OBJECT: v1 0x0075B848 ?g_VLIGHTS_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VLIGHTS_CAVIARDATA;  // 0x0075B848
// RECOVERY_OBJECT: v1 0x007592D0 ?g_VPT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VPT_CAVIARDATA;  // 0x007592D0
// RECOVERY_OBJECT: v1 0x0075AF00 ?g_A_CAVIARDATA@@3UCaviarData@@A
CaviarData g_A_CAVIARDATA;  // 0x0075AF00
// RECOVERY_OBJECT: v1 0x0078A490 ?g_APWALL_CAVIARDATA@@3UCaviarData@@A
CaviarData g_APWALL_CAVIARDATA;  // 0x0078A490
// RECOVERY_OBJECT: v1 0x007AD120 ?g_ASGEN_CAVIARDATA@@3UCaviarData@@A
CaviarData g_ASGEN_CAVIARDATA;  // 0x007AD120
// RECOVERY_OBJECT: v1 0x007ACCB0 ?g_APSID_CAVIARDATA@@3UCaviarData@@A
CaviarData g_APSID_CAVIARDATA;  // 0x007ACCB0
// RECOVERY_OBJECT: v1 0x007AD0E0 ?g_VA01_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VA01_CAVIARDATA;  // 0x007AD0E0
// RECOVERY_OBJECT: v1 0x0076E560 ?g_VR_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VR_CAVIARDATA[4];  // 0x0076E560, 0xc stride
// RECOVERY_OBJECT: v1 0x00779560 ?g_VHR_CAVIARDATA2@@3UCaviarData@@A
CaviarData g_VHR_CAVIARDATA2;  // 0x00779560
// RECOVERY_OBJECT: v1 0x0075ADB8 ?g_VRC_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VRC_CAVIARDATA[4];  // 0x0075ADB8, 0xc stride
// RECOVERY_OBJECT: v1 0x0078B740 ?g_VI_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VI_CAVIARDATA;  // 0x0078B740
// RECOVERY_OBJECT: v1 0x0077A5B8 ?g_VGMT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGMT_CAVIARDATA;  // 0x0077A5B8
// RECOVERY_OBJECT: v1 0x0078B720 ?g_VGMTP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGMTP_CAVIARDATA;  // 0x0078B720
CaviarData g_UNUSED_CAVIARDATA_VAR2;  // 0x007ACF90
// RECOVERY_OBJECT: v1 0x007796A0 ?g_VWNTU_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VWNTU_CAVIARDATA;  // 0x007796A0
// RECOVERY_OBJECT: v1 0x0075AD78 ?g_VT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VT_CAVIARDATA;  // 0x0075AD78
// RECOVERY_OBJECT: v1 0x007ACD60 ?g_DROP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_DROP_CAVIARDATA;  // 0x007ACD60
// RECOVERY_OBJECT: v1 0x0076EBD0 ?g_DROPLET_CAVIARDATA@@3UCaviarData@@A
CaviarData g_DROPLET_CAVIARDATA;  // 0x0076EBD0
// RECOVERY_OBJECT: v1 0x007AD170 ?g_VCL_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCL_CAVIARDATA;  // 0x007AD170
CaviarData g_UNUSED_CAVIARDATA_VAR3[2];  // 0x007795F0, 0xc stride
// RECOVERY_OBJECT: v1 0x00789B58 ?g_VCLT00_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCLT00_CAVIARDATA;  // 0x00789B58
// RECOVERY_OBJECT: v1 0x00779608 ?g_VHT_VBP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VHT_VBP_CAVIARDATA;  // 0x00779608
// RECOVERY_OBJECT: v1 0x0078A640 ?g_VHTP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VHTP_CAVIARDATA;  // 0x0078A640
// RECOVERY_OBJECT: v1 0x00779648 ?g_VHTA0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VHTA0_CAVIARDATA[2];  // 0x00779648, 0xc stride
// RECOVERY_OBJECT: v1 0x00759310 ?g_VHTTP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VHTTP_CAVIARDATA;  // 0x00759310
// RECOVERY_OBJECT: v1 0x0078B750 ?g_VHTTPA0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VHTTPA0_CAVIARDATA[3];  // 0x0078B750, 0xc stride
// RECOVERY_OBJECT: v1 0x007A6820 ?g_VSP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VSP_CAVIARDATA;  // 0x007A6820
// RECOVERY_OBJECT: v1 0x00779270 ?g_VSPA0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VSPA0_CAVIARDATA[2];  // 0x00779270, 0xc stride
// RECOVERY_OBJECT: v1 0x007A7290 ?g_VSPTF_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VSPTF_CAVIARDATA;  // 0x007A7290
// RECOVERY_OBJECT: v1 0x007A7280 ?g_VSPTB_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VSPTB_CAVIARDATA;  // 0x007A7280
// RECOVERY_OBJECT: v1 0x007ACC50 ?g_VFL_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VFL_CAVIARDATA;  // 0x007ACC50
// RECOVERY_OBJECT: v1 0x007A7240 ?g_VGS_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGS_CAVIARDATA;  // 0x007A7240
// RECOVERY_OBJECT: v1 0x00798BF0 ?g_VGSP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VGSP_CAVIARDATA;  // 0x00798BF0
// RECOVERY_OBJECT: v1 0x007795B0 ?g_VJTP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VJTP_CAVIARDATA;  // 0x007795B0
// RECOVERY_OBJECT: v1 0x0078A2B8 ?g_VJT0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VJT0_CAVIARDATA[2];  // 0x0078A2B8, 0xc stride
// RECOVERY_OBJECT: v1 0x0075AEE0 ?g_VCU_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCU_CAVIARDATA;  // 0x0075AEE0
// RECOVERY_OBJECT: v1 0x0075B440 ?g_VCUP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCUP_CAVIARDATA;  // 0x0075B440
// RECOVERY_OBJECT: v1 0x0078CAB8 ?g_VCUA0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VCUA0_CAVIARDATA[2];  // 0x0078CAB8, 0xc stride
// RECOVERY_OBJECT: v1 0x0075B088 ?g_VCUW_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCUW_CAVIARDATA;  // 0x0075B088
// RECOVERY_OBJECT: v1 0x0079A680 ?g_VCT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCT_CAVIARDATA;  // 0x0079A680
// RECOVERY_OBJECT: v1 0x007ACFA0 ?g_VCTP_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCTP_CAVIARDATA;  // 0x007ACFA0
// RECOVERY_OBJECT: v1 0x0076E890 ?g_VCTB_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VCTB_CAVIARDATA;  // 0x0076E890
// RECOVERY_OBJECT: v1 0x0079A628 ?g_VCT0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VCT0_CAVIARDATA[2];  // 0x0079A628, 0xc stride
// RECOVERY_OBJECT: v1 0x0078B730 ?g_VWNTT_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VWNTT_CAVIARDATA;  // 0x0078B730
// RECOVERY_OBJECT: v1 0x0078DE20 ?g_VWNST_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VWNST_CAVIARDATA;  // 0x0078DE20
// RECOVERY_OBJECT: v1 0x007AC1A0 ?g_VWNAA_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VWNAA_CAVIARDATA;  // 0x007AC1A0
// RECOVERY_OBJECT: v1 0x00787100 ?g_VW_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VW_CAVIARDATA[16];  // 0x00787100, 0xc stride
// RECOVERY_OBJECT: v1 0x00791D78 ?g_VM_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VM_CAVIARDATA;  // 0x00791D78
// RECOVERY_OBJECT: v1 0x0078A5F0 ?g_VM13_CAVIARDATA@@3UCaviarData@@A
CaviarData g_VM13_CAVIARDATA;  // 0x0078A5F0
// RECOVERY_OBJECT: v1 0x0076E9B0 ?g_VPBR0_CAVIARDATA@@3PAUCaviarData@@A
CaviarData g_VPBR0_CAVIARDATA[4];  // 0x0076E9B0, 0xc stride
CaviarData g_UNUSED_CAVIARDATA_VAR4[2];  // 0x007791E8, 0xc stride
// RECOVERY_OBJECT: v1 0x0079A690 ?g_NW_CAVIARDATA@@3UCaviarData@@A
CaviarData g_NW_CAVIARDATA;  // 0x0079A690
// RECOVERY_OBJECT: v1 0x007791C0 ?g_NI_CAVIARDATA@@3UCaviarData@@A
CaviarData g_NI_CAVIARDATA;  // 0x007791C0
// RECOVERY_OBJECT: v1 0x00779660 ?g_NLC_CAVIARDATA@@3UCaviarData@@A
CaviarData g_NLC_CAVIARDATA;  // 0x00779660
CaviarData g_UNUSED_CAVIARDATA_VAR5[5];  // 0x00787E08, 0xc stride

// RECOVERY_OBJECT: v1 0x009C0ACC ?CaviarSceneProjectionScratch@@3UCaviarSceneProjectionScratchCandidate@@A
CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch = {0};

// ---------------------------------------------------------------------------
// The voxel renderer's workspace, set up once by init_class below. These are
// REAL OBJECTS now - they used to be `T *const` bindings naming terranx.exe
// data addresses, which every use read or wrote through, unmapped in a
// standalone build. The image's addresses travel in the comments; the image's
// own layout lines up behind them exactly: BufferA (sizeof 0x588) ends at
// CaviarViewportState, BufferB ends at the colour-table block, and the block's
// 0x1800 read extent ends at CaviarScene. They stay file-local because nothing
// else in the tree reaches them.
// ---------------------------------------------------------------------------

namespace {

// 0x009B9100. The 0x20000-byte colour table mem_get allocates here, filled
// with 0xffff, copied into the render record and walked into its two ramps.
// One scene row: 0x100 16-bit pixels, 0x200 bytes - 0x100 rows make the
// scene's own 0x20000-byte surface, which vox_fill_colour_table clears to
// 0xffff and the render record's ramp A walks in 0x100-byte (half-row)
// steps.
struct CaviarSceneRow { uint16_t pixel[0x100]; };

static_assert(sizeof(CaviarSceneRow) == 0x200,
              "scene row must be 0x200 bytes so 0x100 rows are 0x20000");

CaviarSceneRow *CaviarSceneMemory;  // 0x009B9100

// 0x009B9108 and 0x009B96B0. The two 256x256 scene surfaces this function
// creates (init) and clears (fill 9 / fill 0). Their `dib_bits_` (+0x54) and
// `stride_` (+0x4A8) are what the descriptor block below reads - the old
// CaviarSceneBufferABits/CaviarSceneBufferBBits/CaviarRecordBitsSource/
// CaviarRecordOwnerSource bindings were those very fields under fixed
// addresses (0x9B915C, 0x9B9704, 0x9B95B0 = BufferA+0x4A8, 0x9B9B58 =
// BufferB+0x4A8).
Buffer CaviarSceneBufferA;  // 0x009B9108
Buffer CaviarSceneBufferB;  // 0x009B96B0

// 0x009B9690..0x009B969C: four slots zeroed once the record is built. The
// image clears them in the order 0, 2, 3, 1 - not slot order.
int CaviarViewportState[4];  // 0x009B9690

// 0x009B9C38. Passed twice to vox_create_record, which reads it as the
// colour-table source (0x1800 bytes for pixel size 1) and the shadow-table
// source (0x100 bytes) - the old "record's name" reading was the binding's,
// not the body's. Zero at load in the image, as here; whatever filled it in
// the shipped game is not recovered. Sized to the 0x1800 the call reads.
uint8_t CaviarColourSource[0x1800];  // 0x009B9C38

// ForceOldVoxelAlgorithm, the PREFERENCES byte the ini read seeds. Real
// storage - the image's slot is 0x009C0830, unmapped in the standalone
// build, and both init_class and the engine's algo dispatch read it.
uint8_t CaviarVoxelAlgoFlag = 0;

// ---------------------------------------------------------------------------
// THE ENGINE'S PER-DRAW SCRATCH. Every one of these is written at the top of
// the engine's own draw (vox_draw_scene, homed at the foot of this file) from
// the render record and the descriptor block it is handed, and read back by
// that draw and by the helpers it calls. Their image addresses are named on
// each; the rebuilt program gives them its own, which is what every other
// global in this file does too.
// ---------------------------------------------------------------------------

// 0x009BE6C4. Set to 1 at the draw's entry and cleared on the way out; if the
// draw is entered while it is set the image spins on `jmp $+0` forever.
uint8_t vox_scene_busy = 0;

// 0x009BE6A4..0x009BE6BB, six dwords copied out of the descriptor block at
// the top of the draw. The image writes them in the order +0x18, +0x14, +0x10,
// +0x04, +0x00, +0x08 - that is, back to front - and each slot's comment names
// the source offset it takes.
uint32_t vox_setup_copy_[6];  // [0]=0x009BE6A4, [5]=0x009BE6B8

// 0x009BE6BC. The render record's colour table, copied to where the helpers
// read it.
void *vox_active_colour_table_ = 0;  // 0x009BE6BC

// 0x009C09F8. setup_block[0] minus setup_block[4] - the same two dwords the
// clip rectangle the draw starts from is built out of.

// 0x009C0A18, 0x009C082C, 0x009C0A1C. The record's two ramp tables and its
// pixel-size code, copied to where the per-part walkers read them.
void *vox_active_ramp_a_ = 0;   // 0x009C0A18
void *vox_active_ramp_b_ = 0;   // 0x009C082C
uint8_t vox_active_size_code_ = 0;  // 0x009C0A1C


// 0x00671010. The visibility cut the part's scaled extent is compared against
// to decide whether it is drawn at all.
const float vox_visibility_cut_ = 0.0f;  // 0x00671010

// 0x00671014. Scales the walked node's extent into world units for the clip
// test, and the result is stored and re-read - a real variable, not a
// folded constant.
const float vox_radius_scale_ = 0.0f;  // 0x00671014

// The scene render record this function fills and hands to vox_create_record;
// the created handle lands at the tail (+0x40, 0x009BB478).
struct CaviarSceneRecord {
  uint32_t field_00_;      // 0x9BB438, zeroed on success
  uint32_t field_04_;      // 0x9BB43C, zeroed on success
  uint32_t field_08_;      // 0x9BB440, zeroed on success
  uint32_t field_0C_;      // 0x9BB444, zeroed on success
  void *buffer_a_bits_;    // 0x9BB448
  void *scene_memory_;     // 0x9BB44C
  void *buffer_b_bits_;    // 0x9BB450
  uint32_t width_a_;       // 0x9BB454
  uint32_t height_a_;      // 0x9BB458
  void *record_bits_;      // 0x9BB45C
  uint32_t width_b_;       // 0x9BB460
  void *record_owner_;     // 0x9BB464
  uint8_t record_flag_;    // 0x9BB468
  uint8_t pad_[0x38 - 0x31];
  // 0x9BB470. The renderer object the engine's per-object callbacks dispatch
  // through: object_start's own body opens `mov ecx, [0x9bb470]` rather than
  // using the receiver it was passed. Nothing recovered writes this slot, so
  // it is null until whatever sets it in the image is recovered.
  Caviar *renderer_;       // 0x9BB470
  // 0x9BB474. The caller's borrowed colour-table source lands here before
  // render; object_start reads it for a 0x1800-byte copy and never writes it.
  const void *render_argument_;
  VoxRenderRecord *created_;  // 0x9BB478
};

CaviarSceneRecord CaviarScene;  // 0x009BB438


} // namespace

// Narrow Caviar-owned reads preserve the original live Buffer field
// value without acquiring/releasing pixels or exposing Buffer layout.
void *Caviar::render_color_dib_bits() {
    return CaviarSceneBufferA.dib_bits_;
}

void *Caviar::render_shadow_dib_bits() {
    return CaviarSceneBufferB.dib_bits_;
}

/*
Purpose: Boot the CAVIAR voxel renderer. Hands the vx_* file-IO callbacks to
         the engine and, when the machine's CPU report needs showing, opens a
         popup over BasePop's infrastructure and flattens its entry list into
         StringTemp. Then reads the ForceOldVoxelAlgorithm preference, creates
         the two 256x256 scene surfaces plus the shared 0x20000-entry colour
         table, and builds the render record from them. Nonzero returns make
         WinMain abort the boot; 0x17 is the render-record failure.
// ORIGINAL: 0x006185A0 ?init_class@Caviar@@QAAHXZ 0x006185A0-0x00618D16;0x00663150-0x006632F1
// symbol    ?init_class@Caviar@@SAHXZ
//           (static member: WinMain's `call 0x6185a0` sets up no ecx, so a
//           plain member would C2352 - the same rule as palette.h's
//           set_active_window, which is why the emitted name spells S not Q)
// TRIED: explicit __try/__except(EXCEPTION_EXECUTE_HANDLER) returning 0x17
//        around the post-construction body - C2712, VC6 refuses __try in a
//        function that requires object unwinding, and pop needs it. Measured
//        2026-08-29; reverted. The image's own shape is out of C++ reach: a
//        byte scan of .text and SELFMOD finds exactly ONE executable branch
//        into the teardown block at 0x618763 (the `jmp 0x618873` tail of the
//        block above it), so that block is referenced ONLY by the scope
//        table at 0x6632e7 - it IS an __except handler - and the image's
//        per-path teardown chains (state variable [esp+0x325c] walking
//        9..1, 0x12..0xa, 0x1b..0x13, 0x24..0x1c) are its unwind actions.
//        Those chains destroy pop's MEMBERS directly - 0x406820 (StringStruct
//        dtor) at +0x2150/+0x2180, 0x4066c0 on the second one's virtual base
//        at +0x21a8, 0x406910+0x608e10+0x5d4dd0 on the Dialogs slab at
//        +0x21d0 - where the out-of-line ??1BasePop (0x4064D0) spells the
//        same teardown as remove_all (0x402970) calls. This body's returns
//        all run ??1BasePop instead, which is the same work at runtime and
//        not reproducible from any C++ spelling that also links.
// TRIED: int return on vox_init_callbacks tested `test eax, eax` against the
//        image's `test al, al`; the forwarder now returns char and the test
//        agrees (18 -> 19 of 392 at /O2 /GX).
// LEVER  2026-09-03: the CAVIAR_INVALIDCPU guard is the RANGE form on the
//        global DIRECTLY - `CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag`,
//        no local. The image reads `mov al, byte ptr [0x9c0830]` straight from
//        storage and branches `76 08` (jbe) - an unsigned `<= 0`. The old
//        `const uint8_t algo_flag` local both forced a register copy the image
//        does not have AND made the front end fold `<= 0` back to `== 0`, so
//        the branch emitted `je`. Measured: the compiled branch is now the
//        image's own `76 08` byte-for-byte at 0x00618625/0x00618626, one byte
//        of alignment earlier. vox_init_callbacks (0x6392E0) carries the same
//        check spelled the same way.
// TRIED: holding the popup StringList in a named local versus re-deriving
//        `&pop + 0x2180` at every use - identical scores (19/392, 195
//        compiled). The image's prologue pushes one register fewer than
//        either spelling; kept the named local for readability.
// TRIED: the image INLINES seek_pos(-1), next_entry and current_entry of the
//        popup's StringList at +0x2180 (~60 of the ~200 remaining divergent
//        instructions); those are BYTE_EXACT out-of-line members in
//        stringstruct.cpp, so __forceinline-ing them for this one site would
//        regress their own claims and every other call site. Ceiling noted,
//        not chased.
Return Value: 0 on success, 4 when a scene surface or the colour table could
              not be created, 0x17 when the render record could not.
Status: REWRITTEN under RAII - the body is faithful; the notes above carry
        what measurement showed about the remaining divergence.
*/
int Caviar::init_class() {
    // The popup infrastructure, its heap, buttons and sprite. Its real
    // constructor (0x00600860, in basepop.cpp) builds every subobject the
    // image's teardown paths name; the destructor (0x004064D0, still a
    // pending forwarder) reverses all of it, so a plain local under RAII
    // replays each early exit's cleanup - and its nontrivial destructor is
    // what makes VC6 emit the image's fs:[0] unwind frame for this function.
    BasePop pop;

    // The six vx_* file-IO callbacks the engine is handed: vx_malloc,
    // vx_free, vx_read, vx_write, vx_seek, vx_tell. Real functions homed
    // below (0x00618E10..0x00618E90 in the image) - the raw literals this
    // array used to carry pointed into the original image's .text, which
    // the standalone build does not map.
    VoxIoCallbacks vx_callbacks = {
        &Caviar::vx_malloc, &Caviar::vx_free, &Caviar::vx_read,
        &Caviar::vx_write, &Caviar::vx_seek, &Caviar::vx_tell};

    if (vox_init_callbacks(&vx_callbacks, 1) != 0) {
        // The engine sized up this machine's CPU. Without a stored
        // preference - or with one above the known range - tell the player
        // this is not a CPU the renderer supports, by listing the popup's
        // entries as one space-separated line in StringTemp.
        // RANGE form on the global DIRECTLY - no local, and not `== 0`. The
        // image reads `mov al, byte ptr [0x9c0830]` straight from storage and
        // branches `cmp al,bl; jbe`, which is an unsigned `<= 0`; a
        // `const uint8_t algo_flag` local makes the front end fold `<= 0`
        // back to `== 0` and the branch becomes `je`. Same rule as
        // vox_init_callbacks' copy of this check.
        if (CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag) {
            pop.start("jackal", "CAVIAR_INVALIDCPU", -1, 0, 0, 0);

            // The popup's unit-name list, which start() filled.
            StringList &unit_ids = pop.unit_ids_;
            StringTemp[0] = 0;
            unit_ids.seek_pos(-1);
            for (int i = 0; i < unit_ids.entry_count_; ++i) {
                unit_ids.next_entry();
                const char *entry_text = 0;
                if (unit_ids.current_ != 0) {
                    // The payload node's second word is the display name.
                    entry_text = unit_ids.current_entry()->text;
                }
                strcat(StringTemp, entry_text);
                strcat(StringTemp, " ");
            }
        }
    }

    if (GetPrivateProfileIntA("PREFERENCES", "ForceOldVoxelAlgorithm", 0,
                              ".\\Alpha Centauri.ini") != 0) {
        CaviarVoxelAlgoFlag = 1;
    } else {
        WritePrivateProfileStringA("PREFERENCES", "ForceOldVoxelAlgorithm",
                                   "0", ".\\Alpha Centauri.ini");
    }

    if (CaviarSceneBufferA.init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }
    if (CaviarSceneBufferB.init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }

    CaviarSceneMemory = static_cast<CaviarSceneRow *>(mem_get(0x100 * sizeof(CaviarSceneRow)));
    if (CaviarSceneMemory == 0) {
        return 4;
    }

    CaviarSceneBufferA.fill(9);
    CaviarSceneBufferB.fill(0);
    vox_fill_colour_table(CaviarSceneMemory, 0xffff, 0x10000);

    // The descriptor block, in the image's store order.
    CaviarScene.scene_memory_ = CaviarSceneMemory;
    CaviarScene.record_bits_ = (void *)CaviarSceneBufferA.stride_;
    CaviarScene.buffer_a_bits_ = (void *)CaviarSceneBufferA.dib_bits_;
    CaviarScene.width_a_ = 0x100;
    CaviarScene.height_a_ = 0x100;
    CaviarScene.record_flag_ = 1;
    CaviarScene.buffer_b_bits_ = (void *)CaviarSceneBufferB.dib_bits_;
    CaviarScene.record_owner_ = (void *)CaviarSceneBufferB.stride_;
    CaviarScene.width_b_ = 0x100;

    CaviarScene.created_ = vox_create_record(
        0, CaviarColourSource, CaviarColourSource,
        (void *)&CaviarScene.buffer_a_bits_, 0x80);
    if (CaviarScene.created_ == 0) {
        return 0x17;
    }

    VoxRenderRecord *const record = CaviarScene.created_;
    record->zero_b_[2] = 0;             // 0x38
    record->zero_b_[3] = 0;             // 0x3C
    record->draw_right_ = 0xff;
    record->draw_bottom_ = 0xff;
    record->draw_width_ = 0x100;
    record->draw_height_ = 0x100;
    record->object_start_callback_ = &Caviar::object_start;

    // The record's two 256-entry ramp tables. Table A's entries are the
    // addresses of successive 0x100-byte shade rows of the colour table;
    // table B's are the same addresses measured from zero, which the engine
    // rebases when it runs. The slots are dwords in the record's own layout.
    // Table A holds the shade-row addresses: successive 0x100-byte steps
    // of the scene memory - half a scene row per shade level, the engine's
    // own ramp granularity. Table B holds the same rows ADDRESSED FROM
    // ZERO; the engine adds the scene base when it renders.
    void **const ramp_a = record->ramp_a_ptr_;
    void **const ramp_b = record->ramp_b_ptr_;
    uint8_t *shade_row = reinterpret_cast<uint8_t *>(CaviarSceneMemory);
    for (int index = 0; index < 0x100; ++index) {
        ramp_a[index] = shade_row;
        shade_row += 0x100;
        ramp_b[index] = reinterpret_cast<void *>(index * 0x100);
    }

    CaviarViewportState[0] = 0;
    CaviarViewportState[2] = 0;
    CaviarViewportState[3] = 0;
    CaviarViewportState[1] = 0;
    CaviarScene.field_00_ = 0;
    CaviarScene.field_08_ = 0;
    CaviarScene.field_0C_ = 0;
    CaviarScene.field_04_ = 0;
    return 0;
}

// ---------------------------------------------------------------------------
// The six vx_* IO shims init_class hands the engine. Each is one pure
// forwarding call; the image bodies shuffle their stack operands into the
// callee's order and nothing else. Homed 2026-08-29 from the image bytes
// (the source the array above used to spell as raw .text literals).
// ---------------------------------------------------------------------------

/*
Purpose: The engine's allocation callback - return mem_get's pointer with
         the engine's unsigned-long size.
// ORIGINAL: 0x00618E10 ?vx_malloc@Caviar@@SAXK@Z 0x00618E10-0x00618E1E
// symbol    ?vx_malloc@Caviar@@SAPAXK@Z
// size      14 bytes
// calls     0x005D4510
// TRIED: 5/5 real instructions agree; the image cleans the pushed argument
//        with `add esp, 4` where this tree's VC6 emits `pop ecx` - the same
//        stack-cleanup heuristic auto_save's marker documents as not a
//        source-form lever (src/general.cpp, 0x005ABD20).
//        RE-READ 2026-09-03 and the rule is narrower than "heuristic": the
//        image uses `pop ecx` for exactly ONE of this file's three 1-argument
//        __cdecl cleanups - vx_free's, whose callee `free` returns VOID - and
//        `add esp, 4` for this one and vx_tell's, whose callees (mem_get,
//        _tell) both return a value that the body discards. So the branch the
//        image took is conditioned on the callee's return type, and this
//        tree's VC6 always picks `pop ecx` at a function's last statement.
//        Reaching `add esp, 4` therefore needs a VOID-returning declaration
//        for 0x005D4510, which would be a forwarder at an address the image
//        does not call (call_diff resolves E8 targets by address). Four body
//        spellings measured, all 4 of 5: `(void)mem_get(size)`, an explicit
//        `return;`, a discarded `const void *block` local, and
//        `static_cast<size_t>` on the argument - identical bytes every time.
*/
void *Caviar::vx_malloc(unsigned long size) {
    return mem_get(size);
}

/*
Purpose: The engine's release callback - free behind a null test, so the
         engine may hand back a slot it never filled.
// ORIGINAL: 0x00618E20 ?vx_free@Caviar@@SAXPAX@Z 0x00618E20-0x00618E2F
// size      16 bytes
// calls     0x00644EF2
// TRIED: 6/6 real instructions agree; the unit's link binds the header's
//        `free` through an incremental-link pad (`call $+5; jmp`) that the
//        image does not have - the archive spelling this homing replaced
//        declared its own never-defined `free`, which the harness resolves
//        straight at the image's import thunk, but a real CRT call cannot
//        spell that and stay callable in the standalone build.
*/
void Caviar::vx_free(void *block) {
    if (block != 0) {
        free(block);
    }
}

/*
Purpose: The engine's read callback - the CRT's _read with the operands
         forwarded in call order.
// ORIGINAL: 0x00618E30 ?vx_read@Caviar@@SAXHPADJ@Z 0x00618E30-0x00618E48 BYTE_EXACT
// symbol    ?vx_read@Caviar@@SAHHPADJ@Z
// size      24 bytes
// calls     0x0064A178
*/
int Caviar::vx_read(int fd, char *buffer, long count) {
    return _read(fd, buffer, count);
}

/*
Purpose: The engine's write callback - the CRT's _write, same forwarding
         shape as vx_read.
// ORIGINAL: 0x00618E50 ?vx_write@Caviar@@SAXHPAXJ@Z 0x00618E50-0x00618E68 BYTE_EXACT
// symbol    ?vx_write@Caviar@@SAHHPAXJ@Z
// size      24 bytes
// calls     0x00649EAC
*/
int Caviar::vx_write(int fd, void *buffer, long count) {
    return _write(fd, buffer, count);
}

/*
Purpose: The engine's seek callback - the CRT's _lseek.
// ORIGINAL: 0x00618E70 ?vx_seek@Caviar@@SAXHJH@Z 0x00618E70-0x00618E88 BYTE_EXACT
// symbol    ?vx_seek@Caviar@@SAJHJH@Z
// size      24 bytes
// calls     0x0064BF67
*/
long Caviar::vx_seek(int fd, long offset, int origin) {
    return _lseek(fd, offset, origin);
}

/*
Purpose: The engine's tell callback - the CRT's _tell.
// ORIGINAL: 0x00618E90 ?vx_tell@Caviar@@SAXH@Z 0x00618E90-0x00618E9E
// symbol    ?vx_tell@Caviar@@SAJH@Z
// size      14 bytes
// calls     0x00650220
// TRIED: both of the other shims' residuals at once - `pop ecx` against
//        the image's `add esp, 4` (auto_save's documented heuristic) and
//        the incremental-link pad on the CRT `_tell` call.
*/
long Caviar::vx_tell(int fd) {
    return _tell(fd);
}

// ---------------------------------------------------------------------------
// The voxel engine's own entry points, called by init_class above through
// caviar.h's declarations. None is a Caviar member; they are the engine
// (0x6392E0..0x63F9B0 in the image) operating on its own fixed globals.
// ---------------------------------------------------------------------------

namespace {

// The six typed vx_* IO callback slots installed at 0x009C0B80..0x009C0B94,
// ordered alloc/free/read/write/seek/tell in the original contiguous span.
// Historical compiler notes below preserve the earlier array experiment.
// Ownership caveat: this span overlaps the existing projection_words_ binding
// at 0x009C0B90 (the seek/tell slots). The shared-storage alias and lifetime
// remain unresolved; neither interpretation proves simultaneous independent
// globals. Keep this callback storage separate and provisional, with no
// callback-object binding, until the overlap is independently resolved.
// The image's .data holds them at
// 0x009C0B80..0x009C0B94 and the engine calls through the memory operand
// (`call dword ptr [0x9C0B80]`); the typed object remains six contiguous
// function pointers, and the installer copies its complete Win32 24-byte span.
VoxIoCallbacks vox_callback_slots = {0};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(VoxIoCallbacks) == 24,
              "voxel callback table must remain six contiguous Win32 dwords");
static_assert(offsetof(VoxIoCallbacks, allocate) == 0 &&
              offsetof(VoxIoCallbacks, release) == 4 &&
              offsetof(VoxIoCallbacks, read) == 8 &&
              offsetof(VoxIoCallbacks, write) == 12 &&
              offsetof(VoxIoCallbacks, seek) == 16 &&
              offsetof(VoxIoCallbacks, tell) == 20,
              "voxel callback order must match the original table");
#endif
#define vox_alloc_slot (vox_callback_slots.allocate)
#define vox_free_slot (vox_callback_slots.release)
#define vox_read_slot (vox_callback_slots.read)
#define vox_seek_slot (vox_callback_slots.seek)

// The engine's fixed-address data bindings live at FILE SCOPE, below, as
// plain static consts rather than inside this anonymous namespace: the
// census counts anonymous fixed-address globals against a tight ceiling and
// file-scope statics are how sprite.cpp binds 0x00698CAC.

} // namespace

// The engine's data slots, real storage now - each was a binding on the
// image address in its name, which the standalone build does not map.
typedef char (__cdecl *vox_user_chunk_fn)(int, CvrRecord *, CvrChunkHeader *);
// RECOVERY_OBJECT: v2 0x009C0834 0x009C0838 ?vox_user_chunk@@3P6ADHPAUCvrRecord@@PAUCvrChunkHeader@@@ZA 0x006381F0
vox_user_chunk_fn vox_user_chunk = 0; // 0x009C0834, zeroed on accepted CPU report
uint32_t vox_unk_9c0b78 = 0;    // 0x009C0B78, zeroed on accepted CPU report
// 0x009C0B70, the CPU VENDOR byte vox_detect_cpu fills: 0 unknown,
// 1 GenuineIntel, 2 AuthenticAMD.
uint8_t vox_cpu_vendor = 0;
uint32_t vox_record_count = 0;  // 0x009BE69C, records created, bumped per create

// The pixel-size dispatch tables the engine's two dispatchers (sub_63ad60
// for CPU ranks 1/2/4, sub_63ae20 for ranks 3/5) store into the record's
// five slots (+0x6c, +0x70, +0x74, +0x78, +0x7c), selected by pixel size
// and the record's alignment flag byte. Carried as address integers named by
// slot until the engine's behaviour names the tables themselves. The two
// dispatchers are byte-identical twins in the shipped image, so both bodies
// store the SAME five tables.

// The engine's message strings. Real literals now - the image keeps them in
// .rdata at the addresses each name carried, and vox_stage_error copies from the
// pointer it is handed, so a raw binding dereferenced unmapped memory the
// moment any error arm ran. "Unsupported pixel size" is different in the
// image: a data SLOT at 0x00698A9C holds the string's pointer and the body
// loads through it - one indirection the literal here does not need.
static const char vox_msg_no_setup[] = "No setup parameter";        // 0x00698AFC
static const char vox_msg_setup_empty[] = "Setup is empty";         // 0x00698AD8
static const char vox_msg_cpu_unsupported[] = "CPU not supported";  // 0x00698AE8
static const char vox_msg_no_shadow[] = "No room for shadow table"; // 0x00698B10
static const char vox_msg_no_colour[] = "No room for color table";  // 0x00698B2C
static const char vox_msg_no_handler[] = "No room for handler";     // 0x00698B44
static const char vox_msg_no_colortab[] = "No ColorTab parameter";  // 0x00698B58
static const char vox_msg_unsupported_pixel[] = "Unsupported pixel size"; // 0x00698AC0

// Three more of the same pool, named 2026-09-04 by reading the image's own
// pointer table: 0x006989CC..0x006989E0 is ONE run of six message slots -
// "File seeking", "Bad file format", "Reading file", "No room", "Invalid
// color buffer depth", "Error reading user data chunk" - and the CVR loader
// (0x00638620..0x00638DE0) hands the last five of them to vox_stage_error.
// These three are POINTER VARIABLES, not literals, and that is measured, not
// stylistic: the image loads the slot then pushes it (`mov edx,
// [0x6989d8] / push edx`), where a literal array compiles to one `push
// <address>` and costs vox_alloc_block its match. The eight above stay
// literals because their callers push the address straight out.
static const char *const vox_msg_bad_file_format = "Bad file format";  // 0x006989D0
static const char *const vox_msg_reading_file = "Reading file";        // 0x006989D4
static const char *const vox_msg_no_room = "No room";                  // 0x006989D8
static const char *const vox_msg_user_chunk = "Error reading user data chunk"; // 0x006989E0

// The loader's two float constants, .rdata 0x00670FE8 and 0x00670FEC. The
// first is the chain walk's cut: vox_walk_scaled_chain keeps walking while a
// node's extent, scaled by the caller's factor, stays below it. 1.4142f is
// the exact float the image holds (0x3FB50481).
static const float vox_sqrt_two = 1.4142f;       // 0x00670FE8
// The 7.0f vox_read_chunks tests the record's float at +0x8 against to
// choose between loading the 0x1020000 chunk and expanding it in place.
static const float vox_threshold_seven = 7.0f;   // 0x00670FEC

// The contiguous .data window at 0x00698B70..0x00698B78 is zero in the image.
// vox_blit_transparent initializes and consumes all three words; the three
// scaled blitters all share the cursor; only the 16-bit scaled blitter shares
// the destination-stride word. The two 8-bit loops use their stride parameters.
// The source-row-byte word is used only by vox_blit_transparent.
// The record's clip bounds as the engine hands them to its callers, .data
// 0x009B96A0..0x009B96AC. Named 2026-09-04 from the two bodies that fill it:
// all four come off the created VoxRenderRecord's own clip fields
// (+0x8/+0xc/+0x10/+0x14), the min edges incremented by one.
uint32_t vox_clip_rect[4];     // 0x009B96A0
// RECOVERY_OBJECT: v3 0x00698B70 0x00698B74 ?vox_blit_cursor@@3IA 0x0063B11C member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A destination_cursor_
// RECOVERY_OBJECT: v3 0x00698B74 0x00698B78 ?vox_blit_row_bytes@@3IA 0x0063B072 member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A source_row_bytes_
// RECOVERY_OBJECT: v3 0x00698B78 0x00698B7C ?vox_blit_dst_stride@@3IA 0x0063B11C member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A destination_stride_bytes_
CaviarBlitStateCandidate CaviarBlitState = {0, 0, 0};

// The three engine callees, all HOMED to the foot of this file. The forward
// declarations keep the call sites above them compiling; being same-file
// definitions, the calls stay `call rel32` like the image.
extern "C" void __cdecl vox_stage_error(const char *message);  // stage a message string
extern "C" void __cdecl sub_63ad60(unsigned char *record);
extern "C" char __cdecl sub_63f9b0(unsigned char *record);

// The CVR loader, homed below (0x006385D0..0x00638720, 0x00638CC0, 0x00638DE0,
// 0x0063B072). Declarations first so the loader's own call graph - which is
// mutually recursive through the chunk readers - compiles in any order.
extern "C" unsigned char *__cdecl vox_walk_scaled_chain(unsigned char *node,
                                                        float scale);
extern "C" int __cdecl vox_read_exact(int fd, void *buffer, int count);
extern "C" int __cdecl vox_alloc_block(int size, void **out);
extern "C" int __cdecl vox_read_length_field(int fd, CvrRecord *record,
                                             int length);
extern "C" int __cdecl vox_read_chunks(int fd, CvrRecord *record, int size);
extern "C" int __cdecl vox_read_colour_chunks(int fd, CvrStep *out, int size);
extern "C" int __cdecl vox_read_geometry_chunks(int fd, CvrPart *slots, int size,
                                                int count);
extern "C" int __cdecl vox_read_dword(int fd, CvrRecord *record);
int __cdecl vox_read_parts(int fd, CvrRecord *record, int size);
int __cdecl vox_read_part(int fd, CvrRecord *record, int index, int size);
int __cdecl vox_read_shadow(int fd, CvrRecord *record, int index, int size);
int __cdecl vox_read_shape_patches(int fd, CvrRecord *record, int size);

// Publish the typed slot before reporting allocation failure, just as the
// original allocator does. A void-pointer temporary wrapper would delay that
// observable write until after vox_stage_error.
template<class T>
inline int vox_alloc_typed(int size, T **out) {
    T *block = reinterpret_cast<T *>(vox_alloc_slot((unsigned long)size));
    *out = block;
    if (block == 0) {
        vox_stage_error(vox_msg_no_room);
        return -1;
    }
    return 0;
}


// Original616C80. The second stack argument is present but unused.
// ORIGINAL: 0x00616C80 sub_616c80 0x00616C80-0x00616D33
// symbol    ?apply_palette@CaviarData@@QAEHPAVPalette@@H@Z
// kind      game
int CaviarData::apply_palette(Palette *destination, int) {
    CvrRecord *record = record_;
    if (record == 0) return 7;
    if (destination == 0) return 0x10;
    if (record->palette_ == 0) return 0x10;

    const int first = record->palette_[0];
    const int count = record->palette_[1] - first;
    RGBQUAD colours[256];
    if (count > 0) {
        RGBQUAD *output = colours;
        int source_index = first * 3;
        int remaining = count;
        do {
            output->rgbRed = record->palette_[source_index + 2];
            output->rgbGreen = record->palette_[source_index + 3];
            output->rgbBlue = record->palette_[source_index + 4];
            // rgbReserved remains unwritten, as in the pinned original.
            source_index += 3;
            ++output;
        } while (--remaining != 0);
    }
    return destination->set_rgbquad(colours, first, count);
}

// Original616D40. This releases one optional chunk, not the loaded record.
// ORIGINAL: 0x00616D40 sub_616d40 0x00616D40-0x00616D66 BYTE_EXACT
// symbol    ?release_palette_chunk@CaviarData@@QAEXXZ
// kind      game
void CaviarData::release_palette_chunk() {
    if (record_ != 0 && record_->field_18_ != 0) {
        free(record_->field_18_);
        // The original reloads the owner's record after the free call.
        record_->field_18_ = 0;
    }
}
// Old CVR palettes contain sixteen 256-byte shade rows after a two-byte
// prefix and RGB table. Expand them to twenty-four rows, in column-first
// order; failed reads leave the already-read prefix intact and no new rows.
// ORIGINAL: 0x006391F0 ?vox_expand_palette@@YAHHPAE@Z 0x006391F0-0x006392DC
// RECOVERY_BINDING: v1 {"evidence": ["Original6391F0..6392DC reads 1,1,768,4096 bytes in order, returns -1 on each read failure and expands 24 shade rows from source row floor(2*row/3) in column-first order. Existing seed provides compatible bounds; fresh VC6 emits the declared symbol."], "reason": "Recover old-format CVR palette expansion as genuine C++ in its loader owner, replacing the original-address forwarder."}
int __cdecl vox_expand_palette(int fd, uint8_t *palette) {
    uint8_t shades[0x1000];
    if (vox_read_exact(fd, palette, 1) != 0) return -1;
    if (vox_read_exact(fd, palette + 1, 1) != 0) return -1;
    if (vox_read_exact(fd, palette + 2, 0x300) != 0) return -1;
    if (vox_read_exact(fd, shades, sizeof(shades)) != 0) return -1;

    for (int column = 0; column < 0x100; ++column) {
        for (int shade = 0; shade < 0x30; shade += 2) {
            palette[0x302 + (shade / 2) * 0x100 + column] =
                shades[(shade / 3) * 0x100 + column];
        }
    }
    return 0;
}

// Defined at the foot of this file, promoted from its artifact.
void __cdecl sub_63ae20(unsigned char *record);

/*
Purpose: The engine's CPU DETECTION PROBE, named vox_detect_cpu 2026-08-30
         from the decode below. The shipped body toggles EFLAGS.ID (0x40000)
         to test for the CPUID instruction at all, then EFLAGS.AC (0x200000)
         to separate 386-class from 486-class, then runs CPUID: leaf 0's
         vendor string selects vox_cpu_vendor - 1 for "GenuineIntel"
         (0x00698DD8), 2 for "AuthenticAMD" (0x00698DE4) - and leaf 1's
         family ((eax>>8)&0xf) with the MMX feature bit (EDX 0x800000)
         picks the renderer rank in CaviarVoxelAlgoFlag: family 5 = rank 2,
         family 5+MMX = 3, family 6 = 4, family 6+MMX = 5, anything else 0.
         A flags-register dance with no C++ spelling, so this stand-in
         leaves rank 1 (the fallback the AC test itself produces) and skips
         detection.
// ORIGINAL: 0x0063E860 sub_63e860 0x0063E860-0x0063E94E
// symbol    ?vox_detect_cpu@@YAHXZ
// TRIED: byte-exact is unreachable under the inline-assembly rule - the body
//        is the flags push/pop pair toggling EFLAGS bit 0x40000 (and
//        0x200000) plus CPUID itself, and this address calls nothing (from
//        the promoted artifact).
*/
int __cdecl vox_detect_cpu() {
    // 2026-08-29: rank 1, not 0. This stand-in cannot run CPUID (the flags
    // dance needs __asm, which the tree bans), and a probe that leaves rank
    // 0 makes vox_create_record's algo switch take its default arm - which
    // aborts Caviar::init_class with 0x17 and the whole standalone boot
    // with it. Rank 1 is the old-algorithm rank the preferences path itself
    // forces via ForceOldVoxelAlgorithm, so the engine proceeds exactly as
    // it does on a machine whose user pinned the old path.
    CaviarVoxelAlgoFlag = 1;
    vox_cpu_vendor = 0;
    return 0;
}

/*
Purpose: Install the six vx_* file-IO callbacks and vet the machine's CPU rank.
         0 = accepted, 1 = rejected (init_class then shows CAVIAR_INVALIDCPU
         when the stored preference is also out of range).
// ORIGINAL: 0x006392E0 sub_6392e0 0x006392E0-0x0063937F FILE
// symbol    ?vox_init_callbacks@@YADPBUVoxIoCallbacks@@H@Z
// The LEVER/TRIED measurements below are historical notes from the earlier
// integer-array/void-slot source model; this typed table supersedes that model.
// LEVER: the callback-table address must reach the memcpy intrinsic as a
//        LITERAL - a file-scope `void *const` variable compiles
//        `mov edi, dword ptr [var]` where the image folds `mov edi, 0x9c0b80`
//        (listing_diff, /O2 /GX). Macros, not const variables, for the slots.
// TRIED: `algo_flag == 0 || 5 < algo_flag` compiles `test al,al; je` against
//        the image's `test al,al; jbe` - SOLVED 2026-09-03: the test is the
//        RANGE form `CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag` on
//        the global directly (init_class's copy of the same check reads
//        `cmp al,bl; jbe`, also an unsigned `<= 0`). With the old
//        `const uint8_t algo_flag` local the front end folded `<= 0` back to
//        `== 0`; testing the global does not. Remaining after that: ONLY the
//        edi save placement (6 differing runs, all edi).
// TRIED: the image pushes edi in the save block (instruction 3, hoisted above
//        the first `jne` by scheduling) and pops it on all four return paths,
//        while this tree's memcpy intrinsic push/pops edi around its own
//        rep movsd - image 58 instructions, this tree 55. No spelling tried
//        made VC6 dedicate edi as a callee-saved register here: caching the
//        table pointer in a local (`void *table = vox_callback_table;`)
//        folds away and changes nothing.
// TRIED: at the frameless sets this body is 8 of 58 with only those edi runs
//        differing, against 1 of 58 at the /Oy- sets - but the /Oy- variant
//        scores 0.941 similar against 0.920 and `measure` picks it, so the
//        banked tier reads worse than the body is. Same pattern on 0x63AF60
//        (7 of 96 frameless, 0 of 96 /Oy-).
*/
char __cdecl vox_init_callbacks(const VoxIoCallbacks *callbacks, int flag) {
    if (callbacks == 0) {
        vox_stage_error(vox_msg_no_setup);
        return 1;
    }
    if (callbacks->allocate != 0 && callbacks->release != 0 &&
        callbacks->read != 0 && callbacks->write != 0 &&
        callbacks->seek != 0 && callbacks->tell != 0) {
        memcpy(&vox_callback_slots, callbacks, sizeof(vox_callback_slots));
        vox_detect_cpu();
        // The RANGE form, not `== 0`: the image branches `test al,al; jbe`
        // here and `cmp al,bl; jbe` in init_class - an unsigned `<= 0`
        // comparison, not an equality test. Spelled `== 0`, VC6 emits `je`
        // and the byte differs.
        if (CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag) {
            vox_stage_error(vox_msg_cpu_unsupported);
            return 1;
        }
        vox_user_chunk = 0;
        vox_unk_9c0b78 = 0;
        return 0;
    }
    vox_stage_error(vox_msg_setup_empty);
    return 1;
}

/*
Purpose: Fill a destination with a 16-bit colour repeated `count` times. The
         alignment cascade burns 1 or 2 leading bytes so the bulk runs on
         dwords, then an unrolled 16/8/4/2/1 remainder.
// ORIGINAL: 0x0063AF60 sub_63af60 0x0063AF60-0x0063B072
// symbol    ?vox_fill_colour_table@@YAXPAXKK@Z
// LEVER: the fold is spelled on the RAW parameter, `(value << 16) +
//        (value & 0xffff)`, with `+` not `|` - that is what reproduces the
//        image's mov-copy/shl/and/add sequence (an earlier 16-bit pun of the
//        parameter widened back to a 32-bit load and moved the `and` ahead of
//        the copy; similarity 0.534 -> 0.613 on the rewrite).
// LEVER: `_rotr(fill, 1)` gives the image's single `ror eax, 1`; the
//        shift-or spelling `(fill >> 1) | (fill << 31)` compiles to
//        shl/shr/or across two registers.
// TRIED: the image's 16-bit argument load `mov ax, word ptr [esp+0xc]` with
//        its `and eax, 0xffff` DEFERRED to the fold is not reachable from
//        ANSI C++: an `unsigned short` parameter movzx's (prior artifact
//        note), and a pun through the parameter's address widened anyway.
//        MEASURED 2026-09-03 and still not reachable: narrowing the parameter
//        through an `unsigned short` local (`unsigned short v = (unsigned
//        short)value;` then `(v << 16) + v`, the promotion supplying the mask
//        the image's `and eax,0xffff` performs) - const and non-const both -
//        still compiles `mov ecx, dword ptr [esp+8]; and ecx,0xffff`, a FULL
//        dword load with the mask folded in. VC6 narrows the mask, not the
//        load. It did cut the /O2 body 106 -> 102 instructions, so the fold
//        is a shade closer, but the 16-bit load itself stays out of reach.
// TRIED: the odd-path dword loop as a plain `while` and as a guarded
//        `do/while` both drift from the image's shr/je + dec/jne loop - VC6
//        converts the store loop to rep stosd (+ push/pop edi) or lea forms,
//        and the pointer lands in esi rather than the image's ebx.
*/
void __cdecl vox_fill_colour_table(void *table, unsigned long value,
                                   unsigned long count) {
    // The fold runs on the RAW argument: the image loads only the low 16
    // bits (`mov ax, word ptr [esp+0xc]`, unreachable from ANSI C++ - a
    // 16-bit pun widens back to a 32-bit load) and cleans the high half at
    // the fold, so the closest spelling keeps `value` whole and truncates
    // in the second term. `+`, not `|`: the terms do not overlap and the
    // image emits `add`.
    const unsigned long fill = (value << 16) + (value & 0xffff);
    unsigned char *cursor = (unsigned char *)table;

    if ((unsigned long)cursor & 1) {
        // Odd destination: burn one byte, then run the rest aligned.
        unsigned long bytes = count + count;
        *cursor = (unsigned char)fill;
        bytes -= 1;
        cursor += 1;
        // BUG IN THE ORIGINAL? One BIT of rotation, not one byte, so after
        // the leading byte the aligned stores repeat a bit-rotated pattern.
        // Invisible to the only caller (0xffff fills to 0xffffffff) and left
        // alone deliberately. _rotr: the shift-or spelling compiles to
        // shl/shr/or; the image has one `ror eax, 1`.
        const unsigned long rotated = _rotr(fill, 1);
        const unsigned long tail = bytes & 3;
        unsigned long blocks = bytes >> 2;
        if (blocks != 0) {
            do {
                *(unsigned long *)cursor = rotated;
                cursor += 4;
                blocks -= 1;
            } while (blocks != 0);
        }
        if (tail & 2) {
            *(unsigned short *)cursor = (unsigned short)rotated;
            cursor += 2;
        }
        if (tail & 1) {
            *cursor = (unsigned char)rotated;
        }
        return;
    }

    if ((unsigned long)cursor & 2) {
        *(unsigned short *)cursor = (unsigned short)fill;
        count -= 1;
        cursor += 2;
    }
    const unsigned long tail_word = count & 1;
    // SIGNED, and the whole 16/8/4/2/1 cascade hangs on it: the image's
    // stage guards are jl/jge (0x63AFDD..0x63B066), so the count is meant
    // to go negative and stop. An unsigned spelling makes every `n >= 0`
    // tautologically true and the 16-wide loop wraps straight past the
    // buffer - the page fault this body produced on its first live run.
    int n = (int)(count >> 1);  // dword count
    if (n != 0) {
        unsigned long *p = (unsigned long *)cursor;
        n -= 16;
        if (n >= 0) {
            do {
                p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
                p[4] = fill; p[5] = fill; p[6] = fill; p[7] = fill;
                p[8] = fill; p[9] = fill; p[10] = fill; p[11] = fill;
                p[12] = fill; p[13] = fill; p[14] = fill; p[15] = fill;
                p += 16;
                n -= 16;
            } while (n >= 0);
        }
        n += 8;
        if (n >= 0) {
            p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
            p[4] = fill; p[5] = fill; p[6] = fill; p[7] = fill;
            p += 8;
            n -= 8;
        }
        n += 4;
        if (n >= 0) {
            p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
            p += 4;
            n -= 4;
        }
        n += 2;
        if (n >= 0) {
            p[0] = fill; p[1] = fill;
            p += 2;
            n -= 2;
        }
        n += 1;
        if (n >= 0) {
            p[0] = fill;
        }
        cursor = (unsigned char *)p;
    }
    if (tail_word != 0) {
        *(unsigned short *)cursor = (unsigned short)fill;
    }
}

/*
Purpose: Create the scene render record: allocate it, copy the colour and
         shadow tables for the descriptor's pixel size, fill the clip bounds
         and viewport extents, then run the per-CPU-rank init handler. The
         record pointer, or 0 on any failure.
// ORIGINAL: 0x006393C0 sub_6393c0 0x006393C0-0x00639612 FILE
// symbol    ?vox_create_record@@YAPAUVoxRenderRecord@@HPAX00H@Z
// NAMED RETURN 2026-08-30: the catalogue's K (unsigned long) decorates a
// pointer - the body allocates a VoxRenderRecord and every caller stores it
// as one; the return register carries the same dword either way.
// LEVER: the alloc/free slots must be MACROS on the raw addresses: a const
//        pointer variable compiles `mov eax,[var]; call dword ptr [eax]`
//        where the image calls `call dword ptr [0x9c0b80]` straight through
//        the slot (listing_diff, /O2 /GX).
// LEVER: the pixel-size dispatch is a switch with grouped cases - case 1/2
//        and 3/5 reproduce the image's dec/je/dec/je chain (an if/else-if
//        chain compiles cmp/jne; 0x0063AE20's marker says the same), and the
//        algo switch's five-entry jump table with one range check falls out
//        of the same spelling on *CaviarVoxelAlgoFlag.
// TRIED: the image keeps `xor ebp, ebp` live for EVERY null test and zero
//        store (cmp reg,ebp / mov [ebx+N],ebp); every spelling tried emits
//        test/imm forms instead - the persistent zero register is the main
//        remaining divergence and no source shape reached it.
// TRIED: per-case constant memcpys duplicate the shadow-table rep movsd the
//        image merges across the two pixel sizes (`mov ecx,0x8000; jmp L;
//        mov ecx,0x40; L: mov edi,eax; rep movsd`); a merged variable-size
//        memcpy would call the CRT instead - neither matches.
// TRIED: pixel_size as uint8_t and as unsigned int; the uint8_t spelling
//        reads the byte straight into al, the image widens through edx
//        (xor edx,edx / mov dl,[eax+0x20] / mov eax,edx).
*/
VoxRenderRecord *__cdecl vox_create_record(int setup_id, void *setup_data,
                                           void *shadow_data,
                                           void *setup_block, int size_code) {
    if (setup_data == 0) {
        vox_stage_error(vox_msg_no_colortab);
        return 0;
    }
    VoxRenderRecord *record = (VoxRenderRecord *)vox_alloc_slot(0x80);
    if (record == 0) {
        vox_stage_error(vox_msg_no_handler);
        return 0;
    }
    memset(record, 0, sizeof(VoxRenderRecord));
    record->setup_id_ = setup_id;
    record->setup_block_ = (uint32_t)setup_block;
    record->setup_size_code_ = (uint8_t)size_code;

    // unsigned int, not uint8_t: the image zeroes a register and reads the
    // byte into its low half (xor edx,edx / mov dl,[eax+0x20]) before the
    // dispatch, which is the widening of a 32-bit local.
    const unsigned int pixel_size =
        *(const unsigned char *)((const unsigned char *)setup_block + 0x20);
    switch (pixel_size) {
    case 1:
        record->colour_table_ = (void *)vox_alloc_slot(0x1800);
        if (record->colour_table_ == 0) {
            vox_stage_error(vox_msg_no_colour);
            return 0;
        }
        memcpy(record->colour_table_, setup_data, 0x1800);
        if (shadow_data != 0) {
            record->shadow_table_ = (void *)vox_alloc_slot(0x100);
            if (record->shadow_table_ == 0) {
                vox_stage_error(vox_msg_no_shadow);
                vox_free_slot(record->colour_table_);
                return 0;
            }
            memcpy(record->shadow_table_, shadow_data, 0x100);
        }
        break;
    case 2:
        record->colour_table_ = (void *)vox_alloc_slot(0x3000);
        if (record->colour_table_ == 0) {
            vox_stage_error(vox_msg_no_colour);
            return 0;
        }
        memcpy(record->colour_table_, setup_data, 0x3000);
        if (shadow_data != 0) {
            record->shadow_table_ = (void *)vox_alloc_slot(0x20000);
            if (record->shadow_table_ == 0) {
                vox_stage_error(vox_msg_no_shadow);
                vox_free_slot(record->colour_table_);
                return 0;
            }
            memcpy(record->shadow_table_, shadow_data, 0x20000);
        }
        break;
    default:
        vox_stage_error(vox_msg_unsupported_pixel);
        return 0;
    }

    const uint8_t *desc = (const uint8_t *)setup_block;
    record->draw_right_ = (int)*(const short *)(desc + 0xc) - 1;
    const int height_minus_1 = (int)*(const short *)(desc + 0x10) - 1;
    record->draw_bottom_ = height_minus_1;
    if (record->draw_right_ >= 0 && height_minus_1 >= 0) {
        record->draw_width_ = record->draw_right_ + 1;
        record->draw_height_ = height_minus_1 + 1;
    } else {
        record->draw_width_ = 0;
        record->draw_height_ = 0;
    }

    switch (CaviarVoxelAlgoFlag) {
    case 1:
    case 2:
    case 4:
        sub_63ad60((unsigned char *)record);
        break;
    case 3:
    case 5:
        sub_63ae20((unsigned char *)record);
        break;
    default:
        vox_stage_error(vox_msg_cpu_unsupported);
        return 0;
    }
    record->ramp_a_ptr_ = 0;
    record->ramp_b_ptr_ = 0;
    if (*(const uint32_t *)(desc + 0xc) != 0 &&
        *(const uint32_t *)(desc + 0x10) != 0 &&
        sub_63f9b0((unsigned char *)record) != 0) {
        return 0;
    }
    vox_record_count += 1;
    return record;
}

/*
Purpose: Fill a render record's pixel-size dispatch slots for CPU ranks 3 and
         5, choosing the table set by the record's odd-alignment flag byte.
// ORIGINAL: 0x0063AE20 sub_63ae20 0x0063AE20-0x0063AED2 BYTE_EXACT
// symbol    ?sub_63ae20@@YAXPAE@Z
// LEVER: switch(type){case 1: case 2: default:} reproduces the dec/je/dec/jne
//        idiom; if/else-if compiled to cmp/jne instead (from the promoted
//        artifact, measured BYTE_EXACT there).
*/
void __cdecl sub_63ae20(unsigned char *record) {
    VoxRenderRecord *const typed =
        reinterpret_cast<VoxRenderRecord *>(record);
    const unsigned char type = static_cast<unsigned char>(
        *reinterpret_cast<const unsigned char *>(typed->setup_block_ + 0x20));
    caviar_install_record_kernels_candidate(
        typed, type, (typed->setup_id_ & 1u) != 0);
}

/*
Purpose: Stage the engine's ERROR MESSAGE: copy the caller's string into
         the shared buffer vox_message_buffer (0x9C0D60). Named
         vox_stage_error 2026-08-30 from the buffer's readers - the engine
         clears it to NUL before each use (0x0063830E), strlen-tests it and
         stages a default when empty (0x006383F1, "Error reading user data
         chunk"), and copies a staged default out of it wholesale
         (0x0063FA61) - every vox_* error path stages through here first.
         The
         catalogued nullary-int contract is wrong on both counts - the body
         reads a real stack argument, and `return 0;` would insert an
         `xor eax,eax` ahead of the expansion's `and ecx,3`.
// ORIGINAL: 0x00639390 sub_639390 0x00639390-0x006393B9 BYTE_EXACT
// symbol    _vox_stage_error
// size      41 bytes
// callers   12   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// LEVER: stdafx.h's `#pragma function(strcpy)` makes every strcpy a call,
//        but the image INLINES this one (`repne scasb` + `rep movsd/movsb`
//        is VC6's /Oi expansion). A TU-local `#pragma intrinsic(strcpy)`
//        just above the body restores the expansion at exactly this one
//        site - destination as a literal `(char *)0x009C0D60` so the
//        immediate folds - and BYTE_EXACT fell out first try. The pragma
//        sits below every other body in this file, none of which calls
//        strcpy, so nothing else can move.
//
// PROMOTED from src/recovered/00639390.cpp.
Return Value: n/a
*/
// The image INLINES this one strcpy - the `repne scasb` + `rep movsd/movsb`
// tail is VC6's own /Oi expansion, and calling strcpy while discarding the
// result is what reproduces it. Every other string op in the shipped image
// is an out-of-line call, which is why stdafx.h turns the four string
// routines into calls tree-wide - so this one TU-local pragma restores the
// expansion exactly where the image has it. Placed below every other body
// in this file, none of which calls strcpy, so nothing else can move.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma intrinsic(strcpy)
#endif
// The engine's staged-message slot, 0x009C0D60 in the image - real storage
// now. Every error arm writes it via vox_stage_error; the engine clears it,
// strlen-tests it and reads it back (0x0063830E/0x006383F1/0x0063FA61 - the
// last copies 27 bytes, so the slot holds at least that; the image's extent
// is unpinned, 0x100 is comfortable).

extern "C" void __cdecl vox_stage_error(const char *message) {
    strcpy(CaviarOutputMessageWindow.message_, message);
}

/*
Purpose: Fill a render record's pixel-size dispatch slots for CPU ranks 1, 2
         and 4 - vox_create_record's other dispatch target beside sub_63ae20.
         The shipped image's two dispatchers are byte-identical twins: same
         178 bytes, same five tables, same dec/je/dec/jne chain, so this body
         is sub_63ae20's transcription unchanged.
// ORIGINAL: 0x0063AD60 sub_63ad60 0x0063AD60-0x0063AE12 BYTE_EXACT
// symbol    _sub_63ad60
// size      178 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// LEVER: the shipped image's two pixel-size dispatchers (this and 0x63AE20)
//        are byte-identical twins - same 178 bytes, same five tables - so
//        sub_63ae20's proven transcription transplants unchanged: the
//        switch(type) spelling, the shared file-scope table integers, and
//        `unsigned char type` read through the zero-extended byte load.
//        BYTE_EXACT first try on the homing.
//
// PROMOTED from src/unrecovered/0063ad60.cpp, whose artifact spelling
// (if/else-if chain, const-pointer globals) measured NOT_MATCHING in its
// scaffold; the switch and the shared table integers are sub_63ae20's
// measured levers.
Return Value: n/a
*/
void __cdecl sub_63ad60(unsigned char *record) {
    VoxRenderRecord *const typed =
        reinterpret_cast<VoxRenderRecord *>(record);
    const unsigned char type = static_cast<unsigned char>(
        *reinterpret_cast<const unsigned char *>(typed->setup_block_ + 0x20));
    caviar_install_record_kernels_candidate(
        typed, type, (typed->setup_id_ & 1u) != 0);
}

/*
Purpose: Allocate a render record's two per-entry ramps. Frees whatever the
         record still held at +0x60 and +0x64, allocates count entries of
         each from the setup descriptor at +4, and when both succeed fills
         them: ramp A with base stepped by the descriptor's step, ramp B with
         a running accumulation of its delta. On either allocation failing,
         stages the engine's 27-byte failure message into the shared buffer
         and reports failure to vox_create_record.
// ORIGINAL: 0x0063F9B0 sub_63f9b0 0x0063F9B0-0x0063FA72
// symbol    _sub_63f9b0
// size      194 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0063F9C0 0x0063F9D1 0x0063F9E4 0x0063F9FA
//
// PROMOTED from src/unrecovered/0063f9b0.cpp, whose artifact spelling
// (cached alloc locals, const-pointer globals, a `char *self` copy of the
// parameter) measured NOT_MATCHING in its scaffold.
// TRIED: MISMATCH #0 'push ebx' vs 'push esi' - the image saves ebx/ebp in
//        the prologue (eager saves, one save set covering the base==0 early
//        return) while every spelling here saves them LAZILY, in the loop
//        preheader (push ebx / mov ebx,[edx+0x18] / push ebp / mov
//        ebp,[edx+0x10]). Two consequences ride along: the image emits
//        `add esp,4` after EACH alloc call where this tree merges both into
//        one `add esp,8` after the second, and the loop's accum/index bind
//        to the opposite pair (image accum=eax,index=ecx; tree the reverse).
//        Everything else agrees: the frees (load-once, test, push), the
//        interleaved alloc-arg recomputes (`mov edx,[esi+4]` before the
//        alloc1 store), the guard, the loop body, the memcpy tail including
//        `mov al,1` ahead of `rep movsd`, and both `xor al,al` epilogues.
//        /c /O2 /Gy /GR- /GX: 78 of 83 instructions, 0.919 similar.
// TRIED: eight spellings, all measured, all within 2 of 83: volatile on the
//        two alloc stores; cached ramp_a/ramp_b locals; swapped
//        accum/index declaration order; a named count local for the guard;
//        count+step declared BEFORE the base test so their live ranges span
//        the early exit (the eager-save shape; 80 instr, 2/83); C89
//        declarations at block top (80 instr, 2/83). /Ox compiles identical
//        to /O2; /Oy- sets reach 23/83 but carry the `mov ebp,esp` frame the
//        image does not have. The eager-vs-lazy save decision is upstream of
//        everything tried; same register ASSIGNMENT both sides (esi=record,
//        edi=base, ebx=step, ebp=scratch), only its placement differs.
Return Value: 0 when both ramps are filled, 1 on failure.
*/
extern "C" char __cdecl sub_63f9b0(unsigned char *record) {
    if (*(int *)(record + 0x60) != 0) {
        vox_free_slot(reinterpret_cast<void *>(*(unsigned long *)(record + 0x60)));
    }
    if (*(int *)(record + 0x64) != 0) {
        vox_free_slot(reinterpret_cast<void *>(*(unsigned long *)(record + 0x64)));
    }
    *(void **)(record + 0x60) =
        vox_alloc_slot(*(int *)(*(int *)(record + 4) + 0x10) << 2);
    *(void **)(record + 0x64) =
        vox_alloc_slot(*(int *)(*(int *)(record + 4) + 0x10) << 2);
    if (*(int *)(record + 0x60) != 0 && *(int *)(record + 0x64) != 0) {
        int info = *(int *)(record + 4);
        int base = *(int *)(info + 4);
        if (base == 0) {
            return 0;
        }
        int step = *(int *)(info + 0x18) << 1;
        int accum = 0;
        int index = 0;
        if (*(int *)(info + 0x10) > 0) {
            do {
                *(int *)(*(int *)(record + 0x60) + index * 4) = base;
                base += step;
                *(int *)(*(int *)(record + 0x64) + index * 4) = accum;
                accum += *(int *)(*(int *)(record + 4) + 0x14);
                ++index;
            } while (index < *(int *)(*(int *)(record + 4) + 0x10));
        }
        return 0;
    }
    memcpy((void *)0x009C0D60, (const void *)0x00698DF0, 27);
    return 1;
}

/*
Purpose: Free a CVR record: five plain pointer fields (0xc, 0x10, 0x14, 0x18,
         0x1c), then four arrays sized by the shared count at +0x24 - a
         pointer array at +0x28, a 12-byte-element array at +0x30 and an
         8-byte-element array at +0x2c, and a second 12-byte array at +0x34,
         each element's leading dwords being pointers - then the record
         itself. Every release goes through the engine's free slot.
// ORIGINAL: 0x00638430 sub_638430 0x00638430-0x006385CD
// TRIED: teardown - frees five plain pointer fields (0xc/0x10/0x14/ 0x18/0x1c), three arrays of small structs at 0x28/0x2c/0x30/ 0x34 sized by a shared count field at 0x24, then the object itself through the free callback at g_009c0b84. Diverges at #6 - the original re-reads each array-base field inside the loop rather than caching it once; landed at 0.64 similarity.
// symbol    ?caviar_free_record@@YAXPAUCvrRecord@@@Z
// size      413 bytes
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638447 0x00638458 0x00638469 0x0063847A 0x0063848B 0x006384AB 0x006384C0 0x006384E6 0x006384FB 0x00638510 0x00638528 0x0063854D 0x00638562 0x00638589 0x0063859E 0x006385B6 0x006385C0
//
// NAMED 2026-09-04: caviar.h already reached this body by this name through a
// pending_bodies forwarder, and CaviarData::close is its one caller. The
// binding it freed through (g_009c0b84) is the engine's free slot, which this
// file already models as vox_free_slot.
//
// HOMED from src/unrecovered/00638430.cpp.
// LEVER: the array bases (+0x28, +0x30, +0x34) and the count (+0x24) are
//        RE-READ inside every loop - the image loads [esi+0x30] at 0x6384DB,
//        0x6384EF and 0x638504 within one iteration - so the loops below
//        spell those field reads out in full. Caching either in a local is
//        what the artifact did, and what diverged at instruction 6.
// BUG IN THE ORIGINAL: the +0x28 pointer array's elements are freed WITHOUT a
//        null test (image 0x6384A4..0x6384AB pushes whatever it finds) while
//        the other three arrays' elements are tested first. Reproduced as
//        written - a NULL element there would fault inside the allocator.
// DEFECT FIXED 2026-09-04: the scaffold this came from had NO +0x2c array
//        walk at all - it freed +0x28, +0x30 and +0x34 and silently dropped
//        the 8-byte-element array between them, so a third of every record's
//        allocations leaked. The image walks it at 0x638531..0x638568; the
//        loop is restored here. 55 of 154 -> 102 of 154 with the same
//        instruction count on both sides.
// TRIED: flipping the SIB base - `offset + *(int *)(base + 0x30)` against the
//        image's [edi+eax], where this tree emits [eax+edi] - measured no
//        change at all; VC6 normalises the sum before register allocation, so
//        the base/index split is codegen's, not the source's. 6 of the 19
//        remaining differing runs.
// TRIED: the image pushes edi at instruction 5 and ebx at 7, inside the first
//        field test's guard, and pops them in that order at the single exit;
//        every spelling here saves both at the first array loop instead (and
//        pops ebx first). The eager-save placement is upstream of the body -
//        same wall the Palette receivers hit. 8 of the 19 remaining runs.
//        102 of 154, /c /O2 /Gy /GR- /GX.
Return Value: n/a
*/
void __cdecl caviar_free_record(CvrRecord *record) {
    if (record == 0) {
        return;
    }
    char *first = record->description_;
    if (first != 0) {
        vox_free_slot(first);
    }
    char *second = record->palette_name_;
    if (second != 0) {
        vox_free_slot(second);
    }
    uint8_t *third = record->palette_;
    if (third != 0) {
        vox_free_slot(third);
    }
    uint8_t *fourth = record->field_18_;
    if (fourth != 0) {
        vox_free_slot(fourth);
    }
    char *fifth = record->parts_name_;
    if (fifth != 0) {
        vox_free_slot(fifth);
    }

    if (record->slot_table_ != 0) {
        if (record->part_count_ > 0) {
            int index = 0;
            do {
                // No null test - the image frees the slot's answer whatever
                // it holds.
                char *element = record->slot_table_[index];
                vox_free_slot(element);
                ++index;
            } while (index < record->part_count_);
        }
        vox_free_slot(record->slot_table_);
    }

    if (record->parts_ != 0) {
        if (record->part_count_ > 0) {
            int index = 0;
            do {
                uint8_t *lead = record->parts_[index].colours_;
                if (lead != 0) {
                    vox_free_slot(lead);
                }
                VOX_Vect *mid = record->parts_[index].vectors_;
                if (mid != 0) {
                    vox_free_slot(mid);
                }
                VOX_Matrix *last = record->parts_[index].matrices_;
                if (last != 0) {
                    vox_free_slot(last);
                }
                ++index;
            } while (index < record->part_count_);
        }
        vox_free_slot(record->parts_);
    }

    if (record->steps_ != 0) {
        if (record->part_count_ > 0) {
            int index = 0;
            do {
                CvrShape *lead = record->steps_[index].shape_;
                if (lead != 0) {
                    vox_free_slot(lead);
                }
                ++index;
            } while (index < record->part_count_);
        }
        vox_free_slot(record->steps_);
    }

    if (record->shadows_ != 0) {
        if (record->part_count_ > 0) {
            int index = 0;
            do {
                uint8_t *lead = record->shadows_[index].field_4_;
                if (lead != 0) {
                    vox_free_slot(lead);
                }
                uint8_t *mid = record->shadows_[index].field_8_;
                if (mid != 0) {
                    vox_free_slot(mid);
                }
                ++index;
            } while (index < record->part_count_);
        }
        vox_free_slot(record->shadows_);
    }

    vox_free_slot(record);
}

/*
Purpose: Walk a node chain and return the node the walk stops at. Each node
         is {uint8_t flags; ...; float extent; int next_offset}: bit 0 of the
         flags byte says a next node exists, +8 is the extent and +0xc the
         byte offset to it. The walk continues while the extent, scaled by
         the caller's factor, stays at or under the cut, and stops at the
         node BEFORE one whose scaled extent is past it.
// ORIGINAL: 0x006385D0 sub_6385d0 0x006385D0-0x00638611 FILE BYTE_EXACT
// symbol    _vox_walk_scaled_chain
// size      65 bytes
// callers   3   call targets   0
// kind      game
// flags     sp_ready;purged_ok
//
// HOMED from src/recovered/units/006385d0.cpp.
// NAMED 2026-09-04 from the body's own arithmetic: a flags/offset chain walked
// while a scaled float stays under a constant. Its binding g_00670fe8 is the
// 1.4142f at 0x00670FE8, vox_sqrt_two here.
// LEVER: BYTE_EXACT on the FPU operand order. Each extent is read into its
//        OWN local first and multiplied afterwards - `float extent =
//        *(float *)(cursor + 8);` then `extent * scale` - which is what makes
//        VC6 load the node's extent into st(0) and `fmul` the caller's
//        factor, the image's order. Writing the product as one expression
//        (`*(float *)(cursor + 8) * scale`, either operand order, named local
//        or not) loads [esp+8] first and fmul's the node: 15 of 23.
Return Value: the node the walk stopped at.
*/
extern "C" unsigned char *__cdecl vox_walk_scaled_chain(unsigned char *node,
                                                        float scale) {
    unsigned char *cursor = node;
    unsigned char flags = *cursor;
    while (flags & 1) {
        // The extent is read into its own local first: the image loads the
        // node's extent into st(0) and multiplies by the caller's factor, so
        // the read has to be a statement of its own ahead of the multiply.
        float extent = *(float *)(cursor + 8);
        if (!(extent * scale < vox_sqrt_two)) {
            break;
        }
        unsigned char *next = cursor + *(int *)(cursor + 0xc);
        float next_extent = *(float *)(next + 8);
        if (!(next_extent * scale <= vox_sqrt_two)) {
            break;
        }
        flags = *next;
        cursor = next;
    }
    return cursor;
}

/*
Purpose: Read EXACTLY count bytes through the engine's read slot: loop until
         the accumulated total reaches the request, and stage "Reading file"
         the moment the slot answers negative.
// ORIGINAL: 0x00638650 sub_638650 0x00638650-0x006386A8 FILE
// symbol    _vox_read_exact
// size      88 bytes
// callers   15   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638670
//
// HOMED from src/recovered/units/00638650.cpp. Its two bindings are named
// from the image: g_009c0b88 is the read slot - slot 2 of the six vx_*
// callbacks Caviar::init_class installs, this file's vox_read_slot - and
// g_006989d4 is the "Reading file" message slot.
// TRIED: the image dedicates ALL FOUR callee-saved registers here - ebx/ebp
//        hold the buffer and the fd for the whole loop, esi the running
//        total, edi the count - and reaches for them in the order
//        count/total/buffer/fd. Every spelling of this body allocates two
//        fewer registers and loads the arguments at their use, 39
//        instructions against the image's 45 at /O2 (17 agreeing, first
//        divergence at instruction 7, the loop's register set); the /Oy- set
//        scores a higher SIMILARITY (0.739) with a frame the image does not
//        have, so `measure` reports 1 of 45. No spelling tried made VC6
//        dedicate ebp as a callee-saved register - the receiver-spill wall.
//        /c /O2 /Gy /GR- /GX.
Return Value: 0 when the full count was read, -1 when the slot went negative.
*/
extern "C" int __cdecl vox_read_exact(int fd, void *buffer, int count) {
    int total = 0;
    if (count > 0) {
        do {
            int read = vox_read_slot((unsigned long)fd,
                                     (char *)buffer + total, count - total);
            if (read < 0) {
                vox_stage_error(vox_msg_reading_file);
                return -1;
            }
            total += read;
        } while (total < count);
    }
    return 0;
}

/*
Purpose: Allocate through the engine's alloc slot into *out, staging "No room"
         and reporting failure when the slot returns nothing.
// ORIGINAL: 0x00638620 sub_638620 0x00638620-0x0063864E FILE BYTE_EXACT
// symbol    _vox_alloc_block
// size      46 bytes
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638625
//
// CLAIMED BYTE_EXACT in its scaffold, homed 2026-09-04 and re-measured here.
// Its two bindings are named from the image: g_009c0b80 is the engine's alloc
// slot - slot 0 of the six vx_* callbacks, this file's vox_alloc_slot - and
// g_006989d8 is the "No room" message slot, vox_msg_no_room here.
//
// HOMED from src/recovered/units/00638620.cpp.
Return Value: 0 with *out filled, or -1 with the message staged.
*/
extern "C" int __cdecl vox_alloc_block(int size, void **out) {
    return vox_alloc_typed(size, out);
}

/*
Purpose: Allocate (length - 8) + 1 bytes into the record's slot at +0xc, read
         that many bytes into it and NUL-terminate: the CVR loader's reader
         for the chunks it keeps as text. Its callers inline it
         (vox_read_chunks' 0x1010000 case is this body verbatim against
         +0x10 instead of +0xc).
// ORIGINAL: 0x006386D0 sub_6386d0 0x006386D0-0x0063871D FILE
// symbol    _vox_read_length_field
// size      77 bytes
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
//
// HOMED from src/recovered/units/006386d0.cpp.
Return Value: 0 when the field is read and terminated, -1 on either failure.
*/
extern "C" int __cdecl vox_read_length_field(int fd, CvrRecord *record,
                                             int length) {
    // Three statements, in this order. The image reads the record into eax
    // first (0x6386D0), the payload count into esi second (0x6386D5..DA) and
    // only then forms the slot address in edi (0x6386DD) - so `base` has to
    // be its own local or the first load lands on the wrong argument, and
    // `payload` has to precede `slot` or the two registers swap (measured:
    // 11 of 36 either way round without the base local).
    CvrRecord *base = record;
    int payload = length - 8;
    // The slot lives at +0xc of the record the caller hands over - the same
    // field caviar_free_record releases first.
    char **slot = &base->description_;
    if (vox_alloc_typed(payload + 1, slot) != 0) {
        return -1;
    }
    if (vox_read_exact(fd, (void *)*slot, payload) != 0) {
        return -1;
    }
    (*slot)[payload] = 0;
    return 0;
}

/*
Purpose: Walk a CVR record's chunk list. Each chunk is an 8-byte {tag,
         length} header followed by length-8 payload bytes, and `size` is the
         whole block's extent; the walk stops when the remaining count runs
         out. Three tags are read into the record - 0x1010000 (a NUL-
         terminated text field, into +0x10), 0x1020000 (the palette, into
         +0x14, expanded in place when the record's float at +8 is under 7),
         0x1030000 (into +0x18) - and anything else is seeked past.
// ORIGINAL: 0x00638720 sub_638720 0x00638720-0x006388B2
// TRIED: chunked-record reader (tag/length header, three named chunk tags 0x1010000/0x1020000/0x1030000 plus a default skip through the indirect callback at g_009c0b90) - diverges at #3 with the loop-entry `lea` vs `push`, register/stack layout differs from here on; landed at 0.73 similarity.
// symbol    _vox_read_chunks
// size      402 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0063878D
//
// HOMED from src/unrecovered/00638720.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (slot 4 of the six vx_* callbacks, this
// file's vox_seek_slot - the payload size and SEEK_CUR's 1 are its two
// arguments), g_006989d0 the "Bad file format" message slot and g_00670fec
// the 7.0f at 0x00670FEC.
// LEVER: the tag dispatch is a switch. The image compares 0x1010000/
//        0x1020000/0x1030000 with three cmp/je pairs because the values are
//        not sequential; an if/else-if chain spells the same thing here and
//        a switch spells it too - the two were measured identical on
//        0x00638DE0, whose tags ARE sequential and whose chain is sub/dec/je.
// BUG IN THE ORIGINAL: the 0x1020000 arm's condition is `record->float_at_8
//        < 7.0` measured against the 7.0f constant, i.e. an OLD file whose
//        palette has to be expanded; the 7.0 is data, not a limit, and no
//        overflow is checked. Reproduced as written.
Return Value: 0 when the walk consumed the block, -1 on any failure.
// TRIED: the image loads the record pointer (ebx) and the fd (ebp) into
//        callee-saved registers before the loop; this tree dedicates ebp
//        only and re-loads the record inside each case, 29 of 144 at /O2
//        (first divergence at instruction 3). No spelling tried - a local
//        for the record, a pointer walked by case, per-case casts - made
//        VC6 dedicate ebx here. /c /O2 /Gy /GR- /GX.
*/
extern "C" int __cdecl vox_read_chunks(int fd, CvrRecord *record, int size) {
    struct ChunkHeader {
        int tag;
        int length;
    };
    int remaining = size - 8;
    if (remaining == 0) {
        return 0;
    }
    do {
        ChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x1010000:
            if (vox_alloc_typed(header.length - 8 + 1,
                                &record->palette_name_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, record->palette_name_,
                               header.length - 8) != 0) {
                return -1;
            }
            record->palette_name_[header.length - 8] = 0;
            break;
        case 0x1020000:
            if (record->version_ < vox_threshold_seven) {
                if (vox_alloc_typed(0x1b02, &record->palette_) != 0) {
                    return -1;
                }
                if (vox_expand_palette(fd, record->palette_) != 0) {
                    return -1;
                }
            } else {
                if (vox_alloc_typed(header.length - 8,
                                    &record->palette_) != 0) {
                    return -1;
                }
                if (vox_read_exact(fd, record->palette_,
                                   header.length - 8) != 0) {
                    return -1;
                }
            }
            break;
        case 0x1030000:
            if (vox_alloc_typed(header.length - 8,
                                &record->field_18_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, record->field_18_,
                               header.length - 8) != 0) {
                return -1;
            }
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    } while (remaining != 0);
    return 0;
}

/*
Purpose: Read a CVR record's colour chunks. Two tags: 0x4040201 is a colour
         table whose expected size arrives in *out (the caller's slot is
         {requested size, filled pointer} - an entry of zero means take the
         chunk's own size), and 0x4040202 is a table read straight into the
         caller's array. Anything else is seeked past.
// ORIGINAL: 0x00638CC0 sub_638cc0 0x00638CC0-0x00638DD4
// TRIED: only one source-form tried; the callee-save register allocation (ebx/ebp/esi/edi across the whole loop) at the prologue diverges early. Landing the closest structural translation (mnemonic_similarity 0.635).
// symbol    _vox_read_colour_chunks
// size      276 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638D1B
//
// HOMED from src/unrecovered/00638cc0.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (vox_seek_slot here) and g_006989d0 the
// "Bad file format" message slot.
Return Value: 0 when the walk consumed the block, -1 on any failure.
// TRIED: the image dedicates ebx to the caller's {size, pointer} array for
//        the whole walk, loaded in the same breath as the fd (0x638CD6 then
//        0x638CDA); this tree re-loads it inside the 0x4040201 case, and
//        every spelling tried (a local alias, a subscripted read, a read at
//        the top of the loop) left it deferred. 53 of 116 at /O2, first
//        divergence at instruction 8 - the missing ebx load. /c /O2 /Gy
//        /GR- /GX.
*/
extern "C" int __cdecl vox_read_colour_chunks(int fd, CvrStep *out, int size) {
    struct ChunkHeader {
        int tag;
        int length;
    };
    int remaining = size - 8;
    if (remaining == 0) {
        return 0;
    }
    do {
        ChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        const int payload = header.length - 8;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x4040201:
            if (out->lead_ == 0) {
                if (vox_alloc_typed(payload, &out->shape_) != 0) {
                    return -1;
                }
            } else {
                if (vox_alloc_typed(out->lead_, &out->shape_) != 0) {
                    return -1;
                }
            }
            if (vox_read_exact(fd, out->shape_, payload) != 0) {
                return -1;
            }
            break;
        case 0x4040202:
            if (vox_read_exact(fd, out, payload) != 0) {
                return -1;
            }
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    } while (remaining != 0);
    return 0;
}

/*
Purpose: Read a CVR record's geometry chunks. Three tags name the three
         arrays the record keeps - 0x4040301 into slots[0] (count bytes),
         0x4040302 into slots[1] (count*12 bytes, 3 dwords per element) and
         0x4040303 into slots[2] (count*36 bytes, 9 dwords per element) -
         and anything else is seeked past. The whole block spans `size`
         bytes and `count` is the element count the chunk headers repeat.
// ORIGINAL: 0x00638DE0 sub_638de0 0x00638DE0-0x00638F2A
// TRIED: calling sub_638620/sub_638650/sub_639390 by name - all three are catalogued unmangled with no CRT_SIGNATURES entry, so declfix leaves them nullary (C2660 direct, C2733 if redeclared); routed through function-pointer casts. The fallback branch's indirect call goes through `*(int*)0x9c0b90`, a stored function pointer value (not a vtable), called the same way. Sharing ONE `sub_638650` call site between the 0x4040302/0x4040303 cases via goto (matching the disassembly's single shared call at 0x638e99) was not attempted - the two branches' locals would cross the jump into each other's scope, which C++ refuses; each case is expanded with its own call instead. Best reached: MISMATCH, edit_count 17, 315 bytes vs 330, mnemonic similarity ~0.67.
// symbol    _vox_read_geometry_chunks
// size      330 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638E3E
//
// HOMED from src/unrecovered/00638de0.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (vox_seek_slot here) and g_006989d0 the
// "Bad file format" message slot.
// LEVER: the tag dispatch is a switch spelled in ascending order - the
//        image's 0x4040301/0x4040302/0x4040303 chain is sub eax,0x4040301 /
//        je / dec eax / je / dec eax / je, which only sequential case values
//        produce (0x00638720's non-sequential tags compare with cmp/je).
Return Value: 0 when the walk consumed the block, -1 on any failure.
// TRIED: the image loads the element count (edi) and the fd (ebp) before
//        the loop; this tree defers the count to the 0x4040302/0x4040303
//        arms where it is first used, so the prologue is one load short and
//        everything after shifts (75 of 134 at /O2, first divergence at
//        instruction 8). Same callee-save wall as vox_read_chunks and
//        vox_read_colour_chunks. /c /O2 /Gy /GR- /GX.
*/
extern "C" int __cdecl vox_read_geometry_chunks(int fd, CvrPart *slots, int size,
                                                int count) {
    struct ChunkHeader {
        int tag;
        int length;
    };
    int remaining = size - 8;
    if (remaining == 0) {
        return 0;
    }
    do {
        ChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x4040301:
            if (vox_alloc_typed(count, &slots->colours_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->colours_, count) != 0) {
                return -1;
            }
            break;
        case 0x4040302:
            if (vox_alloc_typed(count * sizeof(VOX_Vect), &slots->vectors_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->vectors_, count * sizeof(VOX_Vect)) != 0) {
                return -1;
            }
            break;
        case 0x4040303:
            if (vox_alloc_typed(count * sizeof(VOX_Matrix), &slots->matrices_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->matrices_, count * sizeof(VOX_Matrix)) != 0) {
                return -1;
            }
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    } while (remaining != 0);
    return 0;
}

/*
Purpose: Blit a 16-bit source rectangle into the scene, skipping the
         transparent key. The destination is the framebuffer at
         framebuffer + (dest_y * dest_width + dest_x) * 2, the source is
         walked with a 16.16 fixed-point step whose whole part indexes the
         row (source_width*2 bytes per source row) and whose fraction
         accumulates in a scratch pair. The three .data slots at
         0x00698B70..0x00698B78 hold the cursor and the two strides, so the
         next call resumes where this one stopped.
// ORIGINAL: 0x0063B072 sub_63b072 0x0063B072-0x0063B11C FILE
// symbol    _vox_blit_transparent
// size      170 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
//
// HOMED from src/unrecovered/0063b072.cpp. Its three bindings are the scratch
// slots named from the body's own arithmetic - g_00698b70 is the cursor the
// inner loop stores through and the outer loop advances by the stride in
// g_00698b78, and g_00698b74 is the source row length in bytes (the source
// argument's width, shifted once).
// NAMED 2026-09-04 from the same arithmetic: the per-pixel `if (value !=
//        key) store` is a colour-key blit, and the render record's function
//        slots (vox_size1_aligned_6c and friends) point into this run of
//        code, so this is one of the engine's inner loops.
// TRIED: `unsigned int acc_x` / `unsigned acc_y` are the image's own ecx/edx
//        16.16 accumulators; spelling them `int` emits sar where the image
//        has shr.
// TRIED: 0 of 53 - the image pushes ebx/esi/edi/ebp and keeps the row count,
//        the cursor and both accumulators in them for the whole double loop,
//        while this tree spills the cursor and re-reads the two stride slots
//        per iteration (62 instructions against 53). The callee-save wall
//        again: no spelling - cached stride locals, a cursor advanced without
//        the global, `short` against `unsigned short` for the pixel read -
//        made VC6 dedicate the four registers. /c /O2 /Gy /GR- /GX.
Return Value: n/a
*/
extern "C" void __cdecl vox_blit_transparent(
    unsigned char *framebuffer, const unsigned short *source, int dest_x, int dest_y,
    int count_x, int count_y, unsigned source_step, int source_width,
    int dest_width, short transparent_key) {
    if (count_x > 0) {
        if (count_y > 0) {
            CaviarBlitState.source_row_bytes_ =
                static_cast<uint32_t>(source_width) << 1;
            CaviarBlitState.destination_stride_bytes_ =
                static_cast<uint32_t>(dest_width) << 1;
            CaviarBlitState.destination_cursor_ =
                caviar_blit_address(
                    framebuffer,
                    caviar_blit_offset(dest_x, dest_y, dest_width) << 1);
            unsigned acc_y = 0;
            do {
                // The cursor is re-read at each row start and the stores run
                // through the local copy; the global only takes the stride
                // step at the row's end.
                unsigned short *destination = reinterpret_cast<unsigned short *>(
                    CaviarBlitState.destination_cursor_);
                int row = count_x;
                unsigned acc_x = 0;
                do {
                    unsigned short value = source[acc_x >> 16];
                    if (value != transparent_key) {
                        *destination = value;
                    }
                    ++destination;
                    acc_x += source_step;
                    --row;
                } while (row != 0);
                CaviarBlitState.destination_cursor_ = caviar_blit_address(
                    CaviarBlitState.destination_cursor_,
                    CaviarBlitState.destination_stride_bytes_);
                acc_y += source_step;
                const uint32_t source_rows = acc_y >> 16;
                source = reinterpret_cast<const unsigned short *>(
                    caviar_blit_source_address(
                        reinterpret_cast<const char *>(source),
                        source_rows * CaviarBlitState.source_row_bytes_));
                acc_y &= 0xffff;
                --count_y;
            } while (count_y != 0);
        }
    }
}

/*
Purpose: Render through the scene record with a colour-table override: store
         the table into the scene record's slot at +0x3C, set field_4_'s bit
         4 - the bit object_start tests to decide where a draw reads its
         colour table from - call render, and drop the bit again. A null
         table is rejected with 0x10.
// ORIGINAL: 0x00617670 ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z 0x00617670-0x006176BA BYTE_EXACT
// symbol    ?render_with_colour_table@Caviar@@QAEHPAVBuffer@@HHPBXH@Z
// size      74 bytes
// prototype int (__thiscall ?UNK2@Caviar@@QAEHPAUBuffer@@HHHH@Z)(Caviar* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00617230
//
// CLAIMED BYTE_EXACT, homed from src/recovered/00617670.cpp 2026-09-04 and
// re-measured here. The binding it carried (g_009bb474) is the scene record
// this file already models as CaviarScene - the slot at +0x3C, named
// render_argument_ - so the store is a member store, not a fixed address.
// Caviar::render (0x00617230) is not homed yet; pending_bodies.cpp forwards
// it and caviar.h declares it, which keeps this call `call rel32`.
Return Value: 0, or 0x10 when the fourth argument is zero.
*/
int Caviar::render_with_colour_table(Buffer *buffer, int centre_x,
                                     int centre_y, const void *colour_table,
                                     int render_flags) {
    if (colour_table == 0) {
        return 0x10;
    }
    CaviarScene.render_argument_ = colour_table;
    field_4_ |= 0x10;
    int result = render(buffer, centre_x, centre_y, render_flags);
    field_4_ &= 0xffffffef;
    return result;
}

/*
Purpose: Render with the engine's flat monochrome table: fill the shared
         0x1800-byte colour source with one byte repeated, raise field_4_'s
         bit 3 so the draw reads its colours from that source, call render,
         and drop the bit again.
// ORIGINAL: 0x00617620 ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z 0x00617620-0x0061766F
// symbol    ?render_mono@Caviar@@QAEXPAVBuffer@@HHHH@Z
//           (this tree spells Buffer a `class` and the image's own name spells
//           it `U`, a struct - so the emitted name carries V. Same rule as
//           render_with_colour_table above, whose symbol fact does the same
//           thing. Fixing it tree-wide means re-keying `class Buffer` in
//           buffer.h and every PAVBuffer fact with it, which is not this
//           file's edit to make.)
// size      79 bytes
// prototype void (__thiscall ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z)(Caviar* this, Buffer*, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006465F0 0x00617230
//
// The colour source it fills is this file's CaviarColourSource (0x009B9C38),
// the same 0x1800 bytes init_class hands vox_create_record as the record's
// colour table - so the "monochrome" draw is the ordinary draw with every
// palette entry forced to one value.
// LEVER: caviar_memset_call, not memset - /Oi expanded the intrinsic into
//        `rep stosd` (32 instructions against the image's 27) and burned EDI
//        with it. Same lever as path.cpp's memset_call.
// TRIED: the last divergence is the bit clear's width. The image's set is a
//        byte `or al, 8` and its clear a dword `and ecx, 0xfffffff7`; this
//        tree reproduces the byte or and gives the clear as a byte
//        `and al, 0xf7` - 27 of 27 instructions, one differing run of three.
//        Measured `&= 0xfffffff7`, `&= ~0x08`, `= x & ~0x08` and a named
//        local for the clear, all identical: VC6 proves the mask byte-local
//        and shrinks it. render_with_colour_table (0x00617670) has the same
//        pair with masks 0x10/0xef and takes the dword form on both sides,
//        so the width is the allocator's choice, not the source's.
// NOTE 2026-09-06: Buffer::free_data moved out of line in buffer.h/buffer.cpp
//        (see render_multi_table_dest below). This body does not call it, and
//        the move left it byte-for-byte where it was - one differing run of
//        three, the mask width above - while taking the tier up to
//        MNEMONIC_ONLY on the re-measure.
Return Value: n/a
Status: Complete
*/
void Caviar::render_mono(Buffer *buffer, int centre_x, int centre_y,
                         int colour, int flags) {
    caviar_memset_call(CaviarColourSource, colour & 0xff, 0x1800);
    field_4_ |= 8;
    render(buffer, centre_x, centre_y, flags);
    field_4_ &= ~0x08;
}

/*
Purpose: Append every part of one loaded .cvr definition to the renderer's
         instance table, at the caller's rotation step. Each part contributes
         one colour byte, one accumulated vector and one matrix, copied out of
         the part's three parallel arrays at `step % step_count`, and the
         instance's palette and step entry come off the record.
// ORIGINAL: 0x00617920 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUVOX_Vect@@H@Z 0x00617920-0x00617A9F
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAUVOX_Vect@@H@Z
// size      383 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAUVOX_Vect@@H@Z)(Caviar* this, CaviarData*, VOX_Vect*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628150
Return Value: 0, 1 when the instance table is full, 7 when the definition has
              no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, VOX_Vect *offset, int step) {
    if (definition == 0 || offset == 0) {
        return 0x10;
    }
    CvrRecord *record = definition->record_;
    if (record == 0) {
        return 7;
    }
    int count = record->part_count_;
    int remainder = step % record->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = definition->record_->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            vector_add(part.vectors_[remainder].values, offset->values,
                       instance.instance_vector_->values);
            *instance.instance_matrix_ = part.matrices_[remainder];

            instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
            instance_steps_[field_13CC_] = definition->record_->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table using a
         second definition as the geometry source. The count and the step entry
         come off the first record, the vectors and the rotation step off the
         second; nothing writes the instance's colour or matrix.
// ORIGINAL: 0x006176E0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@PAUVOX_Vect@@H@Z 0x006176E0-0x006177FE
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@0PAUVOX_Vect@@H@Z
//           (VC6 back-references a repeated identical argument type to `0`;
//           the catalogued name spells the second CaviarData out in full,
//           which is IDA's rendering and not a linker record.)
// size      286 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@PAUVOX_Vect@@H@Z)(Caviar* this, CaviarData*, CaviarData*, VOX_Vect*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628150
Return Value: 0, 1 when the instance table is full, 7 when either definition
              has no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, CaviarData *geometry,
                       VOX_Vect *offset, int step) {
    if (definition == 0 || geometry == 0) {
        return 0x10;
    }
    if (definition->record_ == 0 || geometry->record_ == 0) {
        return 7;
    }
    int count = definition->record_->part_count_;
    int remainder = step % geometry->record_->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            vector_add(geometry->record_->parts_[index].vectors_[remainder].values,
                       offset->values, instance.instance_vector_->values);
            instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
            instance_steps_[field_13CC_] = definition->record_->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table using a
         second definition as the geometry source and a raw object pointer as
         the instance's id. The count and the step entry come off the first
         record, the vectors and the rotation step off the second.
// ORIGINAL: 0x00617800 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@PAUVOX_Vect@@@Z 0x00617800-0x00617916
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAE0PAUVOX_Vect@@H@Z
//           (the back-reference again, and the fifth argument the image's
//           own `ret 0x14` admits - see the note above the body.)
// size      278 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@PAUVOX_Vect@@@Z)(Caviar* this, CaviarData*, uint8*, CaviarData*, VOX_Vect*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628150
//           The catalogued name has four stack arguments and the body pops
//           five (`ret 0x14`, and the fifth is read as the rotation step), so
//           the declaration says five - the artifact reached the same answer
//           and Ghidra agreed with it.
Return Value: 0, 1 when the instance table is full, 7 when either definition
              has no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, uint8_t *object,
                       CaviarData *geometry, VOX_Vect *offset, int step) {
    if (definition == 0 || geometry == 0) {
        return 0x10;
    }
    if (definition->record_ == 0 || geometry->record_ == 0) {
        return 7;
    }
    int count = definition->record_->part_count_;
    int remainder = step % geometry->record_->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            vector_add(geometry->record_->parts_[index].vectors_[remainder].values,
                       offset->values, instance.instance_vector_->values);
            instance_palettes_[field_13CC_] = object;
            instance_steps_[field_13CC_] = definition->record_->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, accumulated vector, matrix - with a raw object pointer as the
         instance's id.
// ORIGINAL: 0x00617AA0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUVOX_Vect@@H@Z 0x00617AA0-0x00617C2E
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUVOX_Vect@@H@Z
// size      398 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUVOX_Vect@@H@Z)(Caviar* this, CaviarData*, uint8*, VOX_Vect*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628150
//
// THE FAMILY'S WALL, measured on all eight: the image opens
// `sub esp, 8 / push ebx / push ebp / push esi / push edi` - one 8-byte frame
// for exactly two locals, all four callee-saved pushes in one block - and its
// loop RE-READS `definition->record_` off the argument at every use, keeping
// only the part count and the index in the frame and the remainder in ebp.
// Every body below opens a bigger frame (0x10 and more) because VC6 hoists
// the scaled step index (`remainder * 0xc`, `remainder * 0x24`) into frame
// slots the image does not need, and every stack operand after the prologue
// therefore disagrees by that difference - which is why the agreement count
// reads 5 of 111 rather than "everything but the prologue". Measured both
// ways on 0x00617F20: spelling `definition->record_` out at every use
// (5 of 111) scores WORSE than a `record` local (7 of 111), so this is not
// the re-read lever caviar_free_record documented, it is frame shape.
Return Value: 0, 1 when the instance table is full, 7 when the definition has
              no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, uint8_t *object,
                       VOX_Vect *offset, int step) {
    if (definition == 0 || object == 0 || offset == 0) {
        return 0x10;
    }
    CvrRecord *record = definition->record_;
    if (record == 0) {
        return 7;
    }
    int count = record->part_count_;
    int remainder = step % record->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = record->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            vector_add(part.vectors_[remainder].values, offset->values,
                       instance.instance_vector_->values);
            *instance.instance_matrix_ = part.matrices_[remainder];
            instance_palettes_[field_13CC_] = object;
            instance_steps_[field_13CC_] = record->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - using a second definition as the geometry
         source. The count, the palette and the step entry come off the first
         record, the part arrays and the rotation step off the second.
// ORIGINAL: 0x00617C30 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@H@Z 0x00617C30-0x00617DAE
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@0H@Z
//           (VC6 back-references the repeated CaviarData to `0`; the
//           catalogued name spells it out in full.)
// size      382 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@H@Z)(Caviar* this, CaviarData*, CaviarData*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, 1 when the instance table is full, 7 when either definition
              has no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, CaviarData *geometry, int step) {
    if (definition == 0 || geometry == 0) {
        return 0x10;
    }
    if (definition->record_ == 0 || geometry->record_ == 0) {
        return 7;
    }
    int count = definition->record_->part_count_;
    int remainder = step % geometry->record_->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = geometry->record_->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            *instance.instance_vector_ = part.vectors_[remainder];
            *instance.instance_matrix_ = part.matrices_[remainder];
            instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
            instance_steps_[field_13CC_] = definition->record_->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - using a second definition as the geometry
         source and a raw object pointer as the instance's id. The count and
         the step entry come off the first record, the part arrays and the
         rotation step off the second.
// ORIGINAL: 0x00617DB0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@H@Z 0x00617DB0-0x00617F12
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAE0H@Z
//           (VC6 back-references the repeated CaviarData to `0`.)
// size      354 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@H@Z)(Caviar* this, CaviarData*, uint8*, CaviarData*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, 1 when the instance table is full, 7 when either definition
              has no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, uint8_t *object,
                       CaviarData *geometry, int step) {
    if (definition == 0 || geometry == 0) {
        return 0x10;
    }
    if (definition->record_ == 0 || geometry->record_ == 0) {
        return 7;
    }
    int count = definition->record_->part_count_;
    int remainder = step % geometry->record_->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = geometry->record_->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            *instance.instance_vector_ = part.vectors_[remainder];
            *instance.instance_matrix_ = part.matrices_[remainder];
            instance_palettes_[field_13CC_] = object;
            instance_steps_[field_13CC_] = definition->record_->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - the whole record the instance comes from
         being the caller's first argument.
// ORIGINAL: 0x00617F20 ?add_object@Caviar@@QAEHPAUCaviarData@@H@Z 0x00617F20-0x0061807D
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@H@Z
// size      349 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@H@Z)(Caviar* this, CaviarData*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, 1 when the instance table is full, 7 when the definition has
              no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, int step) {
    if (definition == 0) {
        return 0x10;
    }
    CvrRecord *record = definition->record_;
    if (record == 0) {
        return 7;
    }
    int count = record->part_count_;
    int remainder = step % record->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = record->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            *instance.instance_vector_ = part.vectors_[remainder];
            *instance.instance_matrix_ = part.matrices_[remainder];
            instance_palettes_[field_13CC_] = record->palette_ + 0x302;
            instance_steps_[field_13CC_] = record->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - with a raw object pointer as the instance's
         palette source and the whole record taken from the definition.
// ORIGINAL: 0x00618080 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEH@Z 0x00618080-0x006181D0
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAEH@Z
// size      336 bytes
// prototype int (__thiscall ?add_object@Caviar@@QAEHPAUCaviarData@@PAEH@Z)(Caviar* this, CaviarData*, uint8*, int)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, 1 when the instance table is full, 7 when the definition has
              no record, 0x10 on a null argument.
Status: Complete
*/
int Caviar::add_object(CaviarData *definition, uint8_t *object, int step) {
    if (definition == 0) {
        return 0x10;
    }
    CvrRecord *record = definition->record_;
    if (record == 0) {
        return 7;
    }
    int count = record->part_count_;
    int remainder = step % record->step_count_;
    if (count > 0) {
        int index = 0;
        do {
            if (field_13CC_ == 200) {
                return 1;
            }
            VoxInstance &instance = slots_[field_13CC_];
            CvrPart &part = record->parts_[index];
            *instance.instance_colour_ = part.colours_[remainder];
            *instance.instance_vector_ = part.vectors_[remainder];
            *instance.instance_matrix_ = part.matrices_[remainder];
            instance_palettes_[field_13CC_] = object;
            instance_steps_[field_13CC_] = record->steps_[index];
            ++index;
            ++field_13CC_;
        } while (index < count);
    }
    return 0;
}

/*
Purpose: Clear the two scene surfaces and the shared colour table over the
         viewport rectangle. The renderer's own surfaces are filled 9 and 0,
         and when the caller asks for it every row of the shared 16-bit table
         inside the viewport is filled 0xffff and the viewport state is reset.
// ORIGINAL: 0x00618470 ?clear_buffers@Caviar@@QAEXH@Z 0x00618470-0x0061851D BYTE_EXACT
// symbol    ?clear_buffers@Caviar@@QAEXH@Z
// size      173 bytes
// prototype void (__thiscall ?clear_buffers@Caviar@@QAEXH@Z)(Caviar* this, int)
// callers   8   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240 0x005DFB50 0x0063AF60
//
// Every global it touches is one this file already models: the shared 0x20000
// -entry table (CaviarSceneMemory, 0x009B9100), the two scene surfaces
// (CaviarSceneBufferA / CaviarSceneBufferB), the viewport rectangle
// (CaviarViewportState, 0x009B9690 in the order left, top, right, bottom) and
// the scene record CaviarScene. The rows it fills are the table's own 0x200
// -byte rows, so the walk is `row_span` rows of `col_span` 16-bit entries
// starting at the viewport's top-left corner.
Return Value: n/a
Status: Complete
*/
void Caviar::clear_buffers(int fill) {
    if (CaviarSceneMemory != 0) {
        int height = CaviarViewportState[1];
        int width = CaviarViewportState[0];
        int bottom = CaviarViewportState[3];
        int row_span = bottom - height;
        uint16_t *row = reinterpret_cast<uint16_t *>(CaviarSceneMemory) +
                        height * 0x100 + width;
        int col_span = CaviarViewportState[2] - width;
        CaviarSceneBufferA.fill(width, height, col_span, row_span, 9);
        CaviarSceneBufferB.fill(0);
        if (fill == 0) {
            if (row_span > 0) {
                int count = row_span;
                do {
                    vox_fill_colour_table(row, 0xffff, col_span);
                    row += 0x100;
                    --count;
                } while (count != 0);
            }
            CaviarViewportState[0] = 0;
            CaviarViewportState[2] = 0;
            CaviarViewportState[3] = 0;
            CaviarViewportState[1] = 0;
            CaviarScene.field_00_ = 0;
            CaviarScene.field_08_ = 0;
            CaviarScene.field_0C_ = 0;
            CaviarScene.field_04_ = 0;
        }
    }
}

/*
Purpose: Render a scene and blit it into the renderer's own surface as a
         multi-table destination. Fills surface A with the caller's fill
         value, draws the scene with bit 3 forced on so it reads the shared
         monochrome table, blits the 0x100-deep box centred on the caller's
         coordinates into surface A, frees the draw's surface lock, fills
         surface A 9 again and clears the buffers the caller asks for.
// ORIGINAL: 0x006171A0 ?render_multi_table_dest@Caviar@@QAEXPAUBuffer@@HHHPAEH@Z 0x006171A0-0x00617226
// symbol    ?render_multi_table_dest@Caviar@@QAEXPAVBuffer@@HHHPAEH@Z
//           (this tree spells Buffer a `class` and the image's own name spells
//           it `U`, a struct - see the note on render_mono above.)
// size      134 bytes
// prototype void (__thiscall ?render_multi_table_dest@Caviar@@QAEXPAUBuffer@@HHHPAEH@Z)(Caviar* this, Buffer*, int, int, int, uint8*, int)
// callers   0   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFB50 0x00617230 0x005D94B0 0x005E34A3 0x00618470
// TRIED: the 23 extra instructions were one inlining. The image CALLS
//        Buffer::free_data (0x005E34A3, a real COMDAT); this tree's
//        definition was in-class in buffer.h, so VC6 folded it into this body
//        - surface_ read, lock count decremented, the surface's own release
//        through vtable slot 0x80 and both fields nulled, 20 instructions
//        where the image has one `call`. Measured `buffer->free_data(1)` and
//        the qualified `buffer->Buffer::free_data(1)`: both inline, 75/76
//        compiled instructions either way. VC6 has no noinline, so the fix is
//        moving free_data out of line in buffer.h - a buffer.h edit that
//        belongs to whoever owns that class, not to this file.
// FIXED 2026-09-06 by that move (buffer.cpp, and the declaration in buffer.h
//        dropped its in-class body): 76 compiled instructions against the
//        image's 53 became 38 of 53 agreeing, and call_diff now reports the
//        image's six calls with no disagreement. Measured across the tree
//        before and after: no claim regressed anywhere in buffer.cpp,
//        sprite.cpp or texture.cpp, whose free_data call sites re-inlined
//        nothing they had not already inlined.
Return Value: n/a
Status: Complete
*/
void Caviar::render_multi_table_dest(Buffer *buffer, int centre_x, int centre_y,
                                     int fill, unsigned char *bits, int flags) {
    CaviarSceneBufferA.fill(fill);
    if (render(buffer, centre_x, centre_y, flags | 8) != 0) {
        return;
    }
    CaviarSceneBufferA.draw_multi_table_dest(buffer, fill, centre_x - 0x80,
                                             centre_y - 0x80, 1, 1, bits);
    buffer->free_data(1);
    CaviarSceneBufferA.fill(9);
    clear_buffers(flags & 1);
}

/*
Purpose: Release a render record and the four tables it owns, then drop the
         count of records in existence. The colour and shadow tables and the
         two ramp pointer arrays are each freed only when non-null; the record
         itself is freed whatever it holds.
// ORIGINAL: 0x00639630 sub_639630 0x00639630-0x00639694
// symbol    ?vox_release_record@@YAXPAUVoxRenderRecord@@@Z
// size      100 bytes
// prototype void (__cdecl sub_639630)(int param_1)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0063963D 0x0063964E 0x0063965F 0x00639670 0x0063967A
//
// HOMED 2026-09-05: the record it tears down is the one VoxRenderRecord
// models and its only caller is Caviar::close_class, so its home is this file
// beside them. Every release goes through the engine's free slot, the same
// vox_free_slot binding caviar_free_record walks.
Return Value: n/a
Status: Complete
*/
void __cdecl vox_release_record(VoxRenderRecord *record) {
    if (record->colour_table_ != 0) {
        vox_free_slot(record->colour_table_);
    }
    if (record->shadow_table_ != 0) {
        vox_free_slot(record->shadow_table_);
    }
    if (record->ramp_a_ptr_ != 0) {
        vox_free_slot(record->ramp_a_ptr_);
    }
    if (record->ramp_b_ptr_ != 0) {
        vox_free_slot(record->ramp_b_ptr_);
    }
    vox_free_slot(record);
    if (vox_record_count != 0) {
        --vox_record_count;
    }
}

/*
Purpose: Tear the scene down: close the two scene surfaces, release the shared
         scene memory, empty the render record's state and release the record,
         then run the engine's own shutdown hook.
// ORIGINAL: 0x00618D20 ?close_class@Caviar@@QAAXXZ 0x00618D20-0x00618D95 BYTE_EXACT
// symbol    ?close_class@Caviar@@SAXXZ
//           (the header declares this `static` - the same rule as
//           init_class above, whose emitted name spells S against the
//           image's QAA for the same reason: a __cdecl member takes no
//           receiver, and a static member is how a call site with no
//           receiver can spell it at all.)
// size      117 bytes
// prototype void (__thiscall ?close_class@Caviar@@QAAXXZ)(Caviar* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x00639380 0x00639630 0x00644EF2
//
// HOMED 2026-09-05 from the pending_bodies forwarder, whose definition this
// one replaces. Everything it touches is a real body here: Buffer::close for
// the two scene surfaces, free for the shared scene memory, vox_release_record
// for the record the scene's created_ slot points at. The engine's shutdown
// hook at 0x00639380 is a nullsub - a function whose whole body is a `ret` -
// so a local empty function stands in for it and the call keeps the shape.
//
// The three state fields it empties are reached through the record's own
// +0x04 descriptor block, not through CaviarScene directly: that block is
// CaviarScene itself, but the image re-reads the record's pointer for each of
// the three stores, so the body spells the same path.
Return Value: n/a
Status: Complete
*/
void Caviar::close_class() {
    if (CaviarScene.created_ != 0) {
        CaviarSceneBufferA.close();
        CaviarSceneBufferB.close();
        if (CaviarSceneMemory != 0) {
            free(CaviarSceneMemory);
            CaviarSceneMemory = 0;
        }
        ((uint32_t *)CaviarScene.created_->setup_block_)[0] = 0;
        ((uint32_t *)CaviarScene.created_->setup_block_)[1] = 0;
        ((uint32_t *)CaviarScene.created_->setup_block_)[2] = 0;
        vox_release_record(CaviarScene.created_);
        CaviarScene.created_ = 0;
    }
    vox_engine_close();
}

// ---------------------------------------------------------------------------
// The five engine helpers the scene draw reaches that are not recovered yet.
// Their argument counts are read off the draw's own pushes and stack cleans
// (0x639E97 `call 0x6396a0` / `add esp, 0x18` and so on); the stand-ins keep
// the draw's calls `call rel32`, the same device vox_draw_scene uses.
// ---------------------------------------------------------------------------

// 0x006396A0. Six arguments, two of them the same output buffer - the draw
// computes one address twice and pushes both. Leaves a float on the FPU stack
// that the draw discards (`fstp st(0)` at 0x639E9F).

// Complete typed-source candidate for 0x6396A0.  The seed's `p1` is ESI,
// hence arg2/context; arg1 remains the record and supplies setup_block_.
// Callee return is the final x87 value; the caller discards it.  Its two
// output pointers are intentionally independent formals because native code
// writes both even though the mapped caller passes the same address.
extern const float CaviarProjectionZero;
extern const float CaviarProjectionMinusOne;
extern const float CaviarProjectionOne;
extern const float CaviarProjectionHalfPixel;
extern const float CaviarProjectionNearNegativeEpsilon;
extern const float CaviarProjectionNearPositiveEpsilon;
extern const float CaviarProjectionShadowRadiusScale;
extern const float CaviarProjectionShadowAxisScaleX;
extern const float CaviarProjectionShadowAxisScaleY;
extern VOX_Matrix CaviarProjectionShadowBasis;
extern CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch;

struct CaviarRenderSetupViewCandidate {
    uint8_t bytes_00_[8];
    uint32_t shadow_data_;
    int32_t half_width_;
    int32_t half_height_;
};

static int caviar_projection_floor(float value) {
    return static_cast<int>(floor(static_cast<double>(value)));
}

static void caviar_projection_copy_float_bits(float value, float *out) {
    memcpy(out, &value, sizeof(value));
}

// ORIGINAL: 0x006396A0 sub_6396a0 0x006396A0-0x00639E54
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?vox_stage_scene@@YAMPAUVoxRenderRecord@@PAUCaviarSceneContext@@PAUVoxCameraBlock@@HPAM3@Z
// TRIED: dense FPU tile/sprite-projection math, esp-relative (frameless) args - tracked esp displacement by hand across every sub/add esp to resolve which [esp+N] occurrences are the same real argument vs reused scratch slots (found Ghidra's own decompile conflated two different pointers under one "param_1", corrected to p1+4 vs p1+0xc). Signature changed to float(int,int,int,int,float*,float*) - FPU return, matches trailing fld+ret with no output slot. Compiles, sim 0.61 framed/frameless (tied). fxch-heavy FPU scheduling makes byte-exact implausible by hand; not retried given size (1972 bytes).
float __cdecl vox_stage_scene(
    VoxRenderRecord *record, CaviarSceneContext *context,
    VoxCameraBlock *camera, int step, float *output_a, float *output_b) {
    CaviarSceneProjectionScratchCandidate &scratch =
        CaviarSceneProjectionScratch;
    CaviarRenderSetupViewCandidate *setup =
        reinterpret_cast<CaviarRenderSetupViewCandidate *>(
            static_cast<uintptr_t>(record->setup_block_));

    vox_matrix_matrix_mul(camera->camera_.rotation.values,
                          context->state_.scene_rotation_.values,
                          scratch.view_scene_matrix_.values);
    for (int row = 0; row != 3; ++row) {
        for (int column = 0; column != 3; ++column) {
            scratch.view_scene_matrix_.values[row][column] *=
                context->state_.scale_;
        }
    }

    float camera_relative_scene[3];
    vec3_subtract(context->state_.scene_offset_.values,
                           camera->camera_.position.values,
                           camera_relative_scene);
    vox_matrix_vector_mul(camera->camera_.rotation.values,
                          camera_relative_scene,
                          scratch.camera_offset_.values);

    // Native accesses the first instance's vector stream without an empty
    // table guard. Preserve that precondition and the selected step.
    const VOX_Vect &first_instance_step =
        context->state_.instances_[0].instance_vector_[step];
    float projected_instance[3];
    vox_matrix_vector_mul(scratch.view_scene_matrix_.values,
                          first_instance_step.values, projected_instance);
    vector_add(projected_instance, scratch.camera_offset_.values,
               projected_instance);

    if (projected_instance[2] <= CaviarProjectionZero ||
        camera->camera_.scaling <= CaviarProjectionZero) {
        scratch.perspective_factor_ = CaviarProjectionOne;
    } else {
        scratch.perspective_factor_ = CaviarProjectionOne /
            (projected_instance[2] * camera->camera_.scaling -
             CaviarProjectionMinusOne);
    }
    scratch.projected_scale_ = camera->camera_.scaling *
        context->field_90_ * scratch.perspective_factor_;
    scratch.viewport_aspect_ =
        (camera->bounds_.lead_[4] - camera->bounds_.lead_[3]) /
        (camera->bounds_.lead_[2] - camera->bounds_.lead_[1]);

    // The original scales the first six matrix cells here, not the last row.
    float *matrix_cells = &scratch.view_scene_matrix_.values[0][0];
    for (int index = 0; index != 6; ++index) {
        matrix_cells[index] *= scratch.projected_scale_;
    }

    const float half_width = static_cast<float>(setup->half_width_ >> 1) -
                             CaviarProjectionHalfPixel;
    const float half_height = static_cast<float>(setup->half_height_ >> 1) -
                              CaviarProjectionHalfPixel;
    scratch.camera_offset_.values[0] =
        context->field_7C_ * context->field_90_ + context->field_88_ +
        half_width + scratch.camera_offset_.values[0] * scratch.projected_scale_;
    scratch.camera_offset_.values[1] =
        context->field_80_ * context->field_90_ + context->field_8C_ +
        half_height + scratch.camera_offset_.values[1] * scratch.projected_scale_;
    scratch.camera_offset_.values[2] -= camera->bounds_.lead_[1];

    const uint8_t *instance_pointer_bytes = reinterpret_cast<const uint8_t *>(
        &context->state_.instances_[0].instance_colour_);
    if ((*instance_pointer_bytes & 1) == 0) {
        scratch.view_scene_matrix_.values[2][0] *= scratch.viewport_aspect_;
        scratch.view_scene_matrix_.values[2][1] *= scratch.viewport_aspect_;
        scratch.view_scene_matrix_.values[2][2] *= scratch.viewport_aspect_;
        scratch.camera_offset_.values[2] *= scratch.viewport_aspect_;
    } else {
        scratch.view_scene_matrix_.values[2][0] =
            -scratch.view_scene_matrix_.values[2][0];
        scratch.view_scene_matrix_.values[2][1] =
            -scratch.view_scene_matrix_.values[2][1];
        scratch.view_scene_matrix_.values[2][2] =
            -(scratch.view_scene_matrix_.values[2][2] * scratch.viewport_aspect_);
        scratch.camera_offset_.values[2] =
            -(scratch.camera_offset_.values[2] * scratch.viewport_aspect_);
    }
    scratch.camera_offset_.values[2] += context->field_84_ +
        (((*instance_pointer_bytes & 1) == 0)
            ? camera->bounds_.lead_[3]
            : camera->bounds_.lead_[4]);

    scratch.projected_scale_ =
        context->state_.scale_ * scratch.projected_scale_;
    scratch.viewport_aspect_ =
        context->state_.scale_ * scratch.viewport_aspect_;
    scratch.part_clip_.left_ = caviar_projection_floor(
        static_cast<float>(camera->bounds_.values_[0]) * context->field_90_);
    scratch.part_clip_.top_ = caviar_projection_floor(
        static_cast<float>(camera->bounds_.values_[1]) * context->field_90_);
    scratch.part_clip_.right_ = caviar_projection_floor(
        static_cast<float>(camera->bounds_.values_[4] + 1) * context->field_90_) + 1;
    scratch.part_clip_.bottom_ = caviar_projection_floor(
        static_cast<float>(camera->bounds_.values_[5] + 1) * context->field_90_) + 1;
    scratch.part_clip_.width_ = scratch.part_clip_.left_ - 1 +
                                scratch.part_clip_.right_;
    scratch.part_clip_.height_ = scratch.part_clip_.top_ - 1 +
                                 scratch.part_clip_.bottom_;

    if (context->field_A4_ != 0 && setup->shadow_data_ != 0) {
        float &shadow_z = context->shadow_vector_.values[2];
        if (shadow_z <= CaviarProjectionNearNegativeEpsilon ||
            CaviarProjectionZero <= shadow_z) {
            if (CaviarProjectionZero <= shadow_z &&
                shadow_z < CaviarProjectionNearPositiveEpsilon) {
                caviar_projection_copy_float_bits(
                    CaviarProjectionNearPositiveEpsilon, &shadow_z);
            }
        } else {
            caviar_projection_copy_float_bits(
                CaviarProjectionNearNegativeEpsilon, &shadow_z);
        }

        const float inverse_shadow_z = CaviarProjectionOne / shadow_z;
        const float shadow_x = context->shadow_vector_.values[0] * inverse_shadow_z;
        const float shadow_y = context->shadow_vector_.values[1] * inverse_shadow_z;
        CaviarProjectionShadowBasis.values[0][2] = -shadow_x;
        CaviarProjectionShadowBasis.values[2][2] = -shadow_y;
        vox_matrix_vector_mul(CaviarProjectionShadowBasis.values,
                              context->state_.scene_offset_.values,
                              scratch.shadow_centre_.values);
        scratch.shadow_centre_.values[0] +=
            shadow_x * context->field_78_;
        scratch.shadow_centre_.values[1] +=
            shadow_y * context->field_78_;
        scratch.shadow_centre_.values[2] += context->field_78_;

        float shadow_relative[3];
        vec3_subtract(scratch.shadow_centre_.values,
                               camera->camera_.position.values,
                               shadow_relative);
        vox_matrix_vector_mul(camera->camera_.rotation.values, shadow_relative,
                              scratch.shadow_centre_.values);

        VOX_Matrix shadow_axes;
        vox_matrix_matrix_mul(camera->camera_.rotation.values,
                              CaviarProjectionShadowBasis.values,
                              shadow_axes.values);
        vox_matrix_matrix_mul(shadow_axes.values,
                              context->state_.scene_rotation_.values,
                              shadow_axes.values);
        const float shadow_projected_scale = context->state_.scale_ *
            camera->camera_.scaling * scratch.perspective_factor_;
        const float shadow_x_scale = shadow_projected_scale * context->field_9C_;
        const float shadow_y_scale = shadow_projected_scale * context->field_A0_;
        for (int axis = 0; axis != 3; ++axis) {
            scratch.shadow_matrix_.values[0][axis] =
                shadow_axes.values[axis][0] * shadow_x_scale;
            scratch.shadow_matrix_.values[1][axis] =
                shadow_axes.values[axis][1] * shadow_y_scale;
        }
        scratch.shadow_centre_.values[0] =
            context->field_7C_ * context->field_9C_ + context->field_94_ +
            half_width + scratch.shadow_centre_.values[0] * shadow_x_scale;
        scratch.shadow_centre_.values[1] =
            context->field_80_ * context->field_A0_ + context->field_98_ +
            half_height + scratch.shadow_centre_.values[1] * shadow_y_scale;

        scratch.shadow_radius_x_ = static_cast<float>(_hypot(
            static_cast<double>(scratch.shadow_matrix_.values[0][0]),
            static_cast<double>(scratch.shadow_matrix_.values[0][2])));
        scratch.shadow_radius_y_ = static_cast<float>(_hypot(
            static_cast<double>(scratch.shadow_matrix_.values[1][0]),
            static_cast<double>(scratch.shadow_matrix_.values[1][2])));
        if (scratch.shadow_radius_x_ < CaviarProjectionNearPositiveEpsilon)
            scratch.shadow_radius_x_ = CaviarProjectionNearPositiveEpsilon;
        if (scratch.shadow_radius_y_ < CaviarProjectionNearPositiveEpsilon)
            scratch.shadow_radius_y_ = CaviarProjectionNearPositiveEpsilon;
        *output_a = scratch.shadow_radius_x_;
        *output_b = scratch.shadow_radius_y_;

        scratch.shadow_clip_.left_ = caviar_projection_floor(
            static_cast<float>(camera->bounds_.values_[0]) * context->field_9C_);
        scratch.shadow_clip_.top_ = caviar_projection_floor(
            static_cast<float>(camera->bounds_.values_[1]) * context->field_A0_);
        scratch.shadow_clip_.right_ = caviar_projection_floor(
            static_cast<float>(camera->bounds_.values_[4] + 1) * context->field_9C_) + 1;
        scratch.shadow_clip_.bottom_ = caviar_projection_floor(
            static_cast<float>(camera->bounds_.values_[5] + 1) * context->field_A0_) + 1;
        scratch.shadow_clip_.width_ = scratch.shadow_clip_.left_ - 1 +
                                      scratch.shadow_clip_.right_;
        scratch.shadow_clip_.height_ = scratch.shadow_clip_.top_ - 1 +
                                       scratch.shadow_clip_.bottom_;
    }
    return scratch.projected_scale_;
}


// 0x0063A280. Four arguments, the draw's four.
void __cdecl caviar_clip_shape(
    VoxRenderRecord *, const CvrShape *, const VOX_Vect *,
    const VOX_Matrix *, const CaviarRenderClipWindowCandidate *, int[6]);

// Native 0x63A280 traverses variable-size shapes through the typed shared
// projection owner; original allocation capacities remain unproved.
// ORIGINAL: 0x0063A280 sub_63a280 0x0063A280-0x0063A6F0
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?vox_load_scene_view@@YAXPAUVoxRenderRecord@@PAUCaviarSceneContext@@PAUVoxCameraBlock@@H@Z
// TRIED: __cdecl(int,int,int,int) direct transcription of the Ghidra decompile (per-item bbox accumulation via sub_6280e0/628150/ 6385d0/6282e0/627e20/63f5e0, two parallel min/max passes for two bbox slots at param_1+8 and param_1+0x20); diverges at insn #13, no-frame ESP-relative local layout not reproduced
void __cdecl vox_load_scene_view(
    VoxRenderRecord *record, CaviarSceneContext *context,
    VoxCameraBlock *camera, int step) {
    VoxSceneState &state = context->state_;
    int bounds[12];
    bounds[0] = bounds[1] = bounds[6] = bounds[7] = 0x7fff;
    bounds[2] = bounds[3] = bounds[8] = bounds[9] = -0x7fff;

    int previous_right = -0x7fff;
    int previous_bottom = -0x7fff;
    int shadow_right = -0x7fff;
    int shadow_bottom = -0x7fff;
    int index = 0;

    for (; index < state.instance_count_; ++index) {
        VoxInstance &instance = state.instances_[index];
        if ((instance.instance_colour_[step] & 0x80) != 0) continue;

        const CvrShape *shape = reinterpret_cast<const CvrShape *>(
            vox_walk_scaled_chain(
                reinterpret_cast<unsigned char *>(
                    state.instance_steps_[index].shape_),
                CaviarSceneProjectionScratch.projected_scale_));
        const float extent = shape->scale_;

        VOX_Vect projection_position;
        vox_matrix_vector_mul(CaviarSceneProjectionScratch.view_scene_matrix_.values,
                              instance.instance_vector_[step].values,
                              projection_position.values);
        vector_add(projection_position.values, CaviarSceneProjectionScratch.camera_offset_.values,
                   projection_position.values);

        if (vox_part_behind(
                &projection_position, extent, camera)) {
            goto shadow_pass;
        }

        {
            VOX_Matrix part;
            vox_matrix_matrix_mul(CaviarSceneProjectionScratch.view_scene_matrix_.values,
                                  instance.instance_matrix_[step].values,
                                  part.values);
            for (int row = 0; row < 3; ++row) {
                for (int column = 0; column < 3; ++column) {
                    part.values[row][column] *= extent;
                }
            }

            CaviarRenderClipWindowCandidate object_box;
            caviar_clip_shape(
                record,
                shape,
                &projection_position, &part, &CaviarSceneProjectionScratch.part_clip_,
                &object_box.left_);
            if (object_box.width_ != 0 && object_box.height_ != 0) {
                if (object_box.left_ < bounds[0]) bounds[0] = object_box.left_;
                if (object_box.top_ < bounds[1]) bounds[1] = object_box.top_;
                if (bounds[2] < object_box.right_) bounds[2] = object_box.right_;
                if (bounds[3] < object_box.bottom_) bounds[3] = object_box.bottom_;
            }
        }

shadow_pass:
        const uint32_t *descriptor =
            reinterpret_cast<const uint32_t *>(record->setup_block_);
        if (context->field_A4_ != 0 && descriptor[2] != 0) {
            VOX_Vect shadow;
            vox_matrix_vector_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                  instance.instance_vector_[step].values,
                                  shadow.values);
            vector_add(shadow.values, CaviarSceneProjectionScratch.shadow_centre_.values,
                       shadow.values);
            const float shadow_scale = extent * vox_radius_scale_;
            if (-shadow_scale * CaviarSceneProjectionScratch.shadow_radius_x_ <= shadow.values[0] &&
                shadow.values[0] <= camera->bounds_.values_[4] +
                                      shadow_scale * CaviarSceneProjectionScratch.shadow_radius_x_ &&
                -shadow_scale * CaviarSceneProjectionScratch.shadow_radius_y_ <= shadow.values[1] &&
                shadow.values[1] <= camera->bounds_.values_[5] +
                                      shadow_scale * CaviarSceneProjectionScratch.shadow_radius_y_) {
                VOX_Matrix shadow_part;
                vox_matrix_matrix_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                      instance.instance_matrix_[step].values,
                                      shadow_part.values);
                for (int row = 0; row < 2; ++row) {
                    for (int column = 0; column < 3; ++column) {
                        shadow_part.values[row][column] *= extent;
                    }
                }

                CaviarRenderClipWindowCandidate shadow_box;
                caviar_clip_shape(
                    record,
                    shape,
                    &shadow, &shadow_part, &CaviarSceneProjectionScratch.shadow_clip_,
                    &shadow_box.left_);
                if (shadow_box.width_ != 0 && shadow_box.height_ != 0) {
                    if (shadow_box.left_ < bounds[6]) bounds[6] = shadow_box.left_;
                    if (shadow_box.top_ < bounds[7]) bounds[7] = shadow_box.top_;
                    if (bounds[8] < shadow_box.right_) bounds[8] = shadow_box.right_;
                    if (bounds[9] < shadow_box.bottom_) bounds[9] = shadow_box.bottom_;
                }
            }
        }

        previous_right = bounds[2];
        previous_bottom = bounds[3];
        shadow_right = bounds[8];
        shadow_bottom = bounds[9];
    }

    bounds[4] = previous_right < 0 || previous_bottom < 0
        ? 0 : previous_right - bounds[0] + 1;
    bounds[5] = previous_right < 0 || previous_bottom < 0
        ? 0 : previous_bottom - bounds[1] + 1;
    bounds[10] = shadow_right < 0 || shadow_bottom < 0
        ? 0 : shadow_right - bounds[6] + 1;
    bounds[11] = shadow_right < 0 || shadow_bottom < 0
        ? 0 : shadow_bottom - bounds[7] + 1;

    CaviarRenderClipWindowCandidate *primary =
        reinterpret_cast<CaviarRenderClipWindowCandidate *>(
            &record->clip_max_a_x_);
    CaviarRenderClipWindowCandidate *secondary =
        reinterpret_cast<CaviarRenderClipWindowCandidate *>(
            &record->clip_max_b_x_);
    primary->left_ = bounds[0]; primary->top_ = bounds[1];
    primary->right_ = bounds[2]; primary->bottom_ = bounds[3];
    primary->width_ = bounds[4]; primary->height_ = bounds[5];
    secondary->left_ = bounds[6]; secondary->top_ = bounds[7];
    secondary->right_ = bounds[8]; secondary->bottom_ = bounds[9];
    secondary->width_ = bounds[10]; secondary->height_ = bounds[11];
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
// ORIGINAL: 0x006282E0 sub_6282e0 0x006282E0-0x00628377
// symbol    ?vox_part_behind@@YA_NPBUVOX_Vect@@MPBUVoxCameraBlock@@@Z

// Candidate source body for 0x63F5E0. Uses the real record/shape/vector
// owners plus a non-owning variable-tail view; not product source yet.
extern "C" void __cdecl sub_63fa80(const VOX_Matrix *);
extern "C" void __cdecl sub_6401b0(const short *, const short *, int *);
// ORIGINAL: 0x0063F5E0 sub_63f5e0 0x0063F5E0-0x0063F9A8
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?caviar_clip_shape@@YAXPAUVoxRenderRecord@@PBUCvrRasterShapeViewCandidate@@PBUVOX_Vect@@PBUVOX_Matrix@@PBUCaviarRenderClipWindowCandidate@@QAH@Z
// TRIED: 6-arg __cdecl signature derived from esp+N reads at the anchor (post-prologue esp) rather than IDA's untyped guess; the per-vertex bbox loop, sub_6280e0/sub_628150/sub_6401b0 arg order, and the final outBox/localBox clamp (which Ghidra's pseudocode also shows) are transcribed from the raw asm since Ghidra's __ftol() calls hide their FPU source operands.
void __cdecl caviar_clip_shape(VoxRenderRecord *record,
                               const CvrShape *shape,
                               const VOX_Vect *position,
                               const VOX_Matrix *orientation,
                               const CaviarRenderClipWindowCandidate *clip,
                               int output[6]) {
    output[4] = 0;
    output[5] = 0;
    output[0] = 0x7fff;
    output[1] = 0x7fff;
    output[2] = -0x7fff;
    output[3] = -0x7fff;

    // The helper reads the source matrix and writes separate projection state.
    sub_63fa80(orientation);

    int local_box[6];
    local_box[0] = record->draw_left_;
    local_box[1] = record->draw_top_;
    local_box[2] = record->draw_right_;
    local_box[3] = record->draw_bottom_;
    local_box[4] = record->draw_width_;
    local_box[5] = record->draw_height_;

    if (clip->width_ != 0 && clip->height_ != 0) {
        if (local_box[0] < clip->left_) local_box[0] = clip->left_;
        if (local_box[1] < clip->top_) local_box[1] = clip->top_;
        if (clip->right_ < local_box[2]) local_box[2] = clip->right_;
        if (clip->bottom_ < local_box[3]) local_box[3] = clip->bottom_;
    }

    if ((shape->field_0_ & 2) == 0) {
        const int offset_x = static_cast<int>(position->values[0]);
        const int offset_y = static_cast<int>(position->values[1]);
        sub_6401b0(&shape->offset_x_,
                   &shape->axis_x_, output);

        if (output[4] != 0 && output[5] != 0) {
            output[0] += offset_x;
            output[1] += offset_y;
            output[2] += offset_x;
            output[3] += offset_y;
        }
        if (output[2] < local_box[0] || output[3] < local_box[1] ||
            local_box[2] < output[0] || local_box[3] < output[1]) {
            output[2] = -0x7fff;
            output[3] = -0x7fff;
            output[0] = 0x7fff;
            output[1] = 0x7fff;
            output[4] = 0;
            output[5] = 0;
            return;
        }
        if (output[0] < local_box[0]) output[0] = local_box[0];
        if (output[1] < local_box[1]) output[1] = local_box[1];
        if (local_box[2] < output[2]) output[2] = local_box[2];
        if (local_box[3] < output[3]) output[3] = local_box[3];
        return;
    }

    int box[4];
    box[0] = 0x7fff;
    box[1] = 0x7fff;
    box[2] = -0x7fff;
    box[3] = -0x7fff;
    int accumulated_width = 0;
    int accumulated_height = 0;
    uint32_t visited_items = 0;
    const CvrRasterItemHeaderCandidate *item =
        reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            shape->item_stream_);

    if (shape->item_count_ != 0) {
        do {
            float point[3];
            point[0] = static_cast<float>(static_cast<int>(item->endpoint_[0]) +
                                          static_cast<int>(item->base_[0]));
            point[1] = static_cast<float>(static_cast<int>(item->endpoint_[1]) +
                                          static_cast<int>(item->base_[1]));
            point[2] = static_cast<float>(static_cast<int>(item->endpoint_[2]) +
                                          static_cast<int>(item->base_[2]));

            float transformed[3];
            vox_matrix_vector_mul(orientation->values, point, transformed);
            vector_add(position->values, transformed, point);

            int16_t delta[3];
            delta[0] = static_cast<int16_t>(
                static_cast<int>(item->endpoint_[3]) - item->base_[0]);
            delta[1] = static_cast<int16_t>(
                static_cast<int>(item->endpoint_[4]) - item->base_[1]);
            delta[2] = static_cast<int16_t>(
                static_cast<int>(item->endpoint_[5]) - item->base_[2]);
            sub_6401b0(delta, item->extent_, box);

            const int x = static_cast<int>(transformed[0]);
            const int y = static_cast<int>(point[1]);
            if (accumulated_width != 0 && accumulated_height != 0) {
                box[1] += y;
                box[0] += x;
                box[2] += x;
                box[3] += y;
            }

            if (local_box[0] <= box[2] && local_box[1] <= box[3] &&
                box[0] <= local_box[2] && box[1] <= local_box[3]) {
                if (box[0] < local_box[0]) box[0] = local_box[0];
                if (box[1] < local_box[1]) box[1] = local_box[1];
                if (local_box[2] < box[2]) box[2] = local_box[2];
                if (local_box[3] < box[3]) box[3] = local_box[3];
                if (box[2] < 0 || box[3] < 0) {
                    accumulated_height = 0;
                    accumulated_width = 0;
                } else {
                    accumulated_width = (box[2] - box[0]) + 1;
                    accumulated_height = (box[3] - box[1]) + 1;
                }
                if (accumulated_width != 0 && accumulated_height != 0) {
                    if (box[0] < output[0]) output[0] = box[0];
                    if (box[1] < output[1]) output[1] = box[1];
                    if (output[2] < box[2]) output[2] = box[2];
                    if (output[3] < box[3]) output[3] = box[3];
                }
            }

            visited_items += item->item_count_;
            const uint32_t payload_size = (item->flags_ & 1) == 0
                ? item->item_count_ * 3u
                : *reinterpret_cast<const uint32_t *>(item->inline_payload_);
            item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                item->inline_payload_ + payload_size);
        } while (visited_items < shape->item_count_);
    }

    if (output[2] > -1 && output[3] > -1) {
        output[4] = (output[2] - output[0]) + 1;
        output[5] = (output[3] - output[1]) + 1;
        return;
    }
    output[4] = 0;
    output[5] = 0;
}


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

// Typed candidates for the two direction-to-octant routines. Their shared
// storage declarations are provisional and must be admitted only with the
// accompanying source mapping/consumer closure.

// Native 0x63FCA0 direction selection. The original 24-row table is composed
// from typed source-owned pointers and its pinned bytes come from the existing
// configure-time terrain-data initializer hook.
// ORIGINAL: 0x0063FCA0 sub_63fca0 0x0063FCA0-0x0063FDF6
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?caviar_direction_to_projection_index@@YAXPBUVOX_Matrix@@PBUVOX_Vect@@M0@Z
// TRIED: FPU scheduling of the two-argument sub_627f50 call push sequence (register-save push esi + call-arg pushes) does not reproduce exactly; landed at 0.77 mnemonic similarity. The logic itself (spherical-to-index trig via acos/atan2/table lookup) is transcribed directly from the disassembly.
void __cdecl caviar_direction_to_projection_index(
    const VOX_Matrix *part_matrix, const VOX_Vect *direction,
    float shape_scale, const VOX_Matrix *camera_rotation) {
    const float xy_scale = shape_scale * CaviarSceneProjectionScratch.projected_scale_;
    const float z_scale = shape_scale * CaviarSceneProjectionScratch.viewport_aspect_;

    VOX_Matrix inverse_part;
    vox_matrix_inverse(part_matrix->values, inverse_part.values);
    vox_projection_render_stage_ = 1;

    VOX_Vect camera_direction;
    vox_matrix_vector_mul(camera_rotation->values, direction->values,
                          camera_direction.values);
    camera_direction.values[0] *= xy_scale;
    camera_direction.values[1] *= xy_scale;
    camera_direction.values[2] *= z_scale;

    VOX_Vect local_direction;
    vox_matrix_vector_mul(inverse_part.values, camera_direction.values,
                          local_direction.values);
    if (local_direction.values[2] < CaviarProjectionRoundThreshold) {
        local_direction.values[0] = -local_direction.values[0];
        local_direction.values[1] = -local_direction.values[1];
        local_direction.values[2] = -local_direction.values[2];
    }

    double polar_angle;
    if (local_direction.values[2] < CaviarProjectionAngleLimit) {
        polar_angle = acos(local_direction.values[2]);
    } else {
        polar_angle = CaviarProjectionRoundThreshold;
    }

    double azimuth_angle = atan2(local_direction.values[1],
                                 local_direction.values[0]);
    if (!(azimuth_angle < CaviarProjectionRoundThreshold)) {
        azimuth_angle -= CaviarProjectionAngleWrap;
    }

    const int polar_index = static_cast<int>(
        CaviarProjectionRoundAbove - polar_angle *
            CaviarProjectionPolarScale);
    int azimuth_index = static_cast<int>(
        CaviarProjectionRoundAbove - azimuth_angle *
            CaviarProjectionAzimuthScale);
    if (azimuth_index == 0x59) azimuth_index = 0;

    vox_projection_selected_angle_ =
        (*CaviarProjectionAngleRows[polar_index]) + (0x59 - azimuth_index);
}


// Historical seed proposal; current ABI/field interpretation is stated below.
// PROPOSAL: extern "C" int __cdecl sub_63fe00() ->
//           extern "C" void __cdecl sub_63fe00(unsigned char *param_1) -
//           evidence: `mov eax, [esp+4]` (single stack arg before any
//           push) then `mov cl, byte ptr [eax]` - a pointer, not a plain
//           int, matching IDA's one-argument guess; no explicit `mov eax,`
//           before `ret` (see the sibling 0x0063FFE0's identical tail).
// ORIGINAL: 0x0063FE00 sub_63fe00 0x0063FE00-0x0063FFDD
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.", "The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.", "Fresh native review corrected argument byte0, scaled Y/Z rounding, void builder result and overlapping output streams at offsets0/20/24 with stride16.", "Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."], "reason": "Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations."}
// symbol    ?vox_direction_to_octant@@YAXPBUVoxRenderRecord@@@Z
// TRIED: near-duplicate of 0x0063FFE0 past the preamble (same 628220/6281b0/round-to-nearest/octant/ratio tail) - only the first block differs, negating all three floats when `*param_1 & 1` instead of calling sub_627f50. The three round-to-nearest computations are inlined at each call site rather than factored into a shared helper: a free-function helper ahead of the definition compiles to zero external .text symbols in body-mode extraction (only a helper CLASS is special-cased). Landing the closest control-flow-faithful form (divergence starts at instruction #2, on the flag test).
void __cdecl vox_direction_to_octant(const VoxRenderRecord *render_record) {
    const VOX_Matrix &matrix = vox_projection_matrix_;
    float vector[3];
    if ((render_record->setup_id_ & 1) != 0) {
        vector[0] = -matrix.values[0][2];
        vector[1] = -matrix.values[1][2];
        vector[2] = -matrix.values[2][2];
    } else {
        vector[0] = matrix.values[0][2];
        vector[1] = matrix.values[1][2];
        vector[2] = matrix.values[2][2];
    }

    float unit[3];
    float scaled[3];
    vox_normalise_vector(vector, unit);
    vec3_scale(unit, CaviarProjectionDirectionScale, scaled);

    // The original tests x87 C0: unordered follows the below branch too.
    const float add_x = unit[0] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[0] = static_cast<int>(scaled[0] + add_x);
    const float add_y = scaled[1] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[1] = static_cast<int>(scaled[1] + add_y);
    const float add_z = scaled[2] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[2] = static_cast<int>(scaled[2] + add_z);

    int octant;
    if (vox_projection_component_[1] < 1) {
        octant = (vox_projection_component_[0] > 0) + 2;
    } else {
        octant = vox_projection_component_[0] < 1;
    }
    if (vox_projection_component_[2] < 0) {
        octant += 4;
    }

    const int x = vox_projection_component_[0];
    const int y = vox_projection_component_[1];
    const int z = vox_projection_component_[2];
    const int abs_x = x < 0 ? -x : x;
    const int abs_y = y < 0 ? -y : y;
    const int abs_z = z < 0 ? -z : z;
    const int sum = abs_x + abs_y + abs_z;
    const float ratio = CaviarProjectionDirectionScale / (float)sum;
    const int fx = (int)(abs_x * ratio);
    const int fy = (int)(abs_y * ratio);

    unsigned char bits;
    if (fx > 0x80) {
        bits = 2;
    } else if (fy > 0x80) {
        bits = 3;
    } else {
        bits = (unsigned char)(fy <= 0x80 - fx);
    }

    vox_projection_prepared_ = 1;
    vox_projection_octant_mask_ = 1 << (octant * 4 | bits);
}

// Historical seed proposal; current ABI/field interpretation is stated below.
// PROPOSAL: extern "C" int __cdecl sub_63ffe0() ->
//           extern "C" void __cdecl sub_63ffe0(int a1) - evidence: the
//           single stack read at [esp+0x24] before any push matches IDA's
//           and Ghidra's one-argument guess, and there is no explicit `mov
//           eax,` before `ret` - the value in eax at return is the `shl`
//           result incidentally, not a deliberate return.
// ORIGINAL: 0x0063FFE0 sub_63ffe0 0x0063FFE0-0x006401A8
// RECOVERY_BINDING: v1 {"reason":"Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations.","evidence":["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.","The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.","Fresh native review corrected argument byte0, scaled Y/Z rounding, void builder result and overlapping output streams at offsets0/20/24 with stride16.","Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."]}
// symbol    ?vox_inverse_direction_to_octant@@YAXPBUVOX_Matrix@@@Z
// TRIED: the three callees (sub_627f50/sub_628220/sub_6281b0) are catalogued nullary, so they are reached through explicit cdecl function-pointer casts rather than by name. The first round-to-nearest computation tests the sign of localC[0] (the sub_628220 output) but rounds local18 (the sub_6281b0 output) - an asymmetry the disassembly shows plainly and this keeps, even though axes 1 and 2 are self-referential. A shared `round_nearest` free-function helper ahead of the definition compiled to zero external .text symbols (body-mode extraction only special-cases a helper CLASS, not a helper free function) - inlined the three round computations instead. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the float-copy preamble).
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *source) {
    vox_matrix_inverse(source->values, vox_projection_matrix_.values);
    const float x_source = vox_projection_matrix_.values[0][2];
    const float y_source = vox_projection_matrix_.values[1][2];
    vox_projection_render_stage_ = 1;
    const float z_source = vox_projection_matrix_.values[2][2];
    float vector[3] = {x_source, y_source, z_source};
    float unit[3];
    float scaled[3];
    vox_normalise_vector(vector, unit);
    vec3_scale(unit, CaviarProjectionDirectionScale, scaled);

    // The original tests x87 C0: unordered follows the below branch too.
    const float add_x = unit[0] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[0] = static_cast<int>(scaled[0] + add_x);
    const float add_y = scaled[1] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[1] = static_cast<int>(scaled[1] + add_y);
    const float add_z = scaled[2] >= CaviarProjectionRoundThreshold
                            ? CaviarProjectionRoundAbove
                            : CaviarProjectionRoundBelow;
    vox_projection_component_[2] = static_cast<int>(scaled[2] + add_z);

    int octant;
    if (vox_projection_component_[1] < 1) {
        octant = (vox_projection_component_[0] > 0) + 2;
    } else {
        octant = vox_projection_component_[0] < 1;
    }
    if (vox_projection_component_[2] < 0) {
        octant += 4;
    }

    const int x = vox_projection_component_[0];
    const int y = vox_projection_component_[1];
    const int z = vox_projection_component_[2];
    const int abs_x = x < 0 ? -x : x;
    const int abs_y = y < 0 ? -y : y;
    const int abs_z = z < 0 ? -z : z;
    const int sum = abs_x + abs_y + abs_z;
    const float ratio = CaviarProjectionDirectionScale / (float)sum;
    const int fx = (int)(abs_x * ratio);
    const int fy = (int)(abs_y * ratio);

    unsigned char bits;
    if (fx > 0x80) {
        bits = 2;
    } else if (fy > 0x80) {
        bits = 3;
    } else {
        bits = (unsigned char)(fy <= 0x80 - fx);
    }

    vox_projection_prepared_ = 1;
    vox_projection_octant_mask_ = 1 << (octant * 4 | bits);
}


// ORIGINAL: 0x006404F0 sub_6404f0 0x006404F0-0x00640A42
// RECOVERY_BINDING: v1 {"reason":"Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations.","evidence":["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.","The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.","Fresh native review corrected argument byte0, scaled Y/Z rounding, void builder result and overlapping output streams at offsets0/20/24 with stride16.","Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."]}
// symbol    ?caviar_build_projection_tables@@YAXXZ
// TRIED: this is SMACX's own isometric-tile offset-table builder, not zlib/CRT - checked, no recognisable library algorithm. Read a 6-short/3-int source record at 0x9C0A40, fanned it out formula- for-formula (per the Ghidra decomp, cross-checked against the disassembly's add/sub/shl/neg chains) into ~80 fixed-address cells declared by the scaffold, then transcribed the closing 26-iteration (0x1A) table walk with raw int*/uint* pointer arithmetic over 0x9C0840/0x9C0BA8. Compiles and is formula- faithful but MISMATCHES at instruction #0 (`sub esp,0x34` vs a `push`-first prologue) - at this size (~200 instructions, 79 scratch cells) the compiler's register/stack-slot allocation diverges from the first instruction, and chasing it further is a search this pass's budget does not cover.
void __cdecl caviar_build_projection_tables() {
    // Build the 26 fixed-point isometric offsets, then pack their three
    // observed output words. The fourth dword in either typed row remains
    // uninterpreted because this function does not access it.
    int w40 = (int)CaviarFixedPointProjection.word_00_;
    int w42 = (int)CaviarFixedPointProjection.word_02_;
    int w44 = (int)CaviarFixedPointProjection.word_04_;
    int w46 = (int)CaviarFixedPointProjection.word_06_;
    int w48 = (int)CaviarFixedPointProjection.word_08_;
    int w4a = (int)CaviarFixedPointProjection.word_0A_;
    int d4c = CaviarFixedPointProjection.dword_0C_;
    int d50 = CaviarFixedPointProjection.dword_10_;
    int d54 = CaviarFixedPointProjection.dword_14_;
    int iVar3;
    int iVar4;

    CaviarKernelProjectionWindow.current_views_.cells_[15].x_ = (int)w42;
    CaviarKernelProjectionWindow.current_views_.cells_[13].x_ = (int)w44;
    CaviarKernelProjectionWindow.current_views_.cells_[24].x_ = CaviarKernelProjectionWindow.current_views_.cells_[15].x_ + w40;
    CaviarKernelProjectionWindow.current_views_.cells_[15].y_ = (int)w48;
    CaviarKernelProjectionWindow.current_views_.cells_[25].x_ = CaviarKernelProjectionWindow.current_views_.cells_[24].x_ + CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[0].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[25].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[24].y_ = CaviarKernelProjectionWindow.current_views_.cells_[15].y_ + w46;
    CaviarKernelProjectionWindow.current_views_.cells_[13].y_ = (int)w4a;
    CaviarKernelProjectionWindow.current_views_.cells_[25].y_ = CaviarKernelProjectionWindow.current_views_.cells_[24].y_ + CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[0].y_ = -CaviarKernelProjectionWindow.current_views_.cells_[25].y_;
    iVar4 = d4c + d54;
    CaviarKernelProjectionWindow.current_views_.cells_[0].z_ = (iVar4 + d50) * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[1].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[24].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[1].y_ = -CaviarKernelProjectionWindow.current_views_.cells_[24].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[1].z_ = (d4c + d50) * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[5].x_ = CaviarKernelProjectionWindow.current_views_.cells_[13].x_ - w40;
    CaviarKernelProjectionWindow.current_views_.cells_[2].x_ = CaviarKernelProjectionWindow.current_views_.cells_[5].x_ - CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[5].y_ = CaviarKernelProjectionWindow.current_views_.cells_[13].y_ - w46;
    CaviarKernelProjectionWindow.current_views_.cells_[2].y_ = CaviarKernelProjectionWindow.current_views_.cells_[5].y_ - CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    iVar3 = d54 - d4c;
    CaviarKernelProjectionWindow.current_views_.cells_[2].z_ = (iVar3 - d50) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[7].x_ = (int)w40;
    CaviarKernelProjectionWindow.current_views_.cells_[22].x_ = CaviarKernelProjectionWindow.current_views_.cells_[13].x_ + CaviarKernelProjectionWindow.current_views_.cells_[7].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[3].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[22].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[22].y_ = w46 + CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[3].y_ = -CaviarKernelProjectionWindow.current_views_.cells_[22].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[3].z_ = iVar4 * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[4].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[7].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[4].y_ = -(int)w46;
    CaviarKernelProjectionWindow.current_views_.cells_[4].z_ = d4c * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[5].z_ = iVar3 * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[7].x_ = CaviarKernelProjectionWindow.current_views_.cells_[15].x_ - CaviarKernelProjectionWindow.current_views_.cells_[7].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[6].x_ = CaviarKernelProjectionWindow.current_views_.cells_[7].x_ - CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[7].y_ = CaviarKernelProjectionWindow.current_views_.cells_[15].y_ - w46;
    CaviarKernelProjectionWindow.current_views_.cells_[6].y_ = CaviarKernelProjectionWindow.current_views_.cells_[7].y_ - CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[6].z_ = ((d50 - d4c) - d54) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[7].z_ = (d50 - d4c) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[8].x_ = (CaviarKernelProjectionWindow.current_views_.cells_[13].x_ + CaviarKernelProjectionWindow.current_views_.cells_[15].x_) - (int)w40;
    CaviarKernelProjectionWindow.current_views_.cells_[8].y_ = (CaviarKernelProjectionWindow.current_views_.cells_[13].y_ + CaviarKernelProjectionWindow.current_views_.cells_[15].y_) - (int)w46;
    CaviarKernelProjectionWindow.current_views_.cells_[8].z_ = (iVar3 + d50) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[9].x_ = -(CaviarKernelProjectionWindow.current_views_.cells_[13].x_ + CaviarKernelProjectionWindow.current_views_.cells_[15].x_);
    CaviarKernelProjectionWindow.current_views_.cells_[9].y_ = -(CaviarKernelProjectionWindow.current_views_.cells_[13].y_ + CaviarKernelProjectionWindow.current_views_.cells_[15].y_);
    CaviarKernelProjectionWindow.current_views_.cells_[9].z_ = (d50 + d54) * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[10].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[10].y_ = -CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[10].z_ = d50 * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[11].x_ = CaviarKernelProjectionWindow.current_views_.cells_[13].x_ - CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[11].y_ = CaviarKernelProjectionWindow.current_views_.cells_[13].y_ - CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[11].z_ = (d54 - d50) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[12].x_ = -CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[12].y_ = -CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[12].z_ = d54 * -0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[13].z_ = d54 << 8;
    CaviarKernelProjectionWindow.current_views_.cells_[14].x_ = CaviarKernelProjectionWindow.current_views_.cells_[15].x_ - CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[14].y_ = CaviarKernelProjectionWindow.current_views_.cells_[15].y_ - CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[14].z_ = (d50 - d54) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[15].z_ = d50 << 8;
    CaviarKernelProjectionWindow.current_views_.cells_[16].x_ = CaviarKernelProjectionWindow.current_views_.cells_[13].x_ + CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[16].y_ = CaviarKernelProjectionWindow.current_views_.cells_[13].y_ + CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[16].z_ = (d50 + d54) * 0x100;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_ec_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[18].x_ = w40 - CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e0_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[17].x_ = CaviarKernelProjectionWindow.current_views_.cells_[18].x_ - CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_fc_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[13].z_;
    CaviarKernelProjectionWindow.current_views_.cells_[18].y_ = w46 - CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[17].y_ = CaviarKernelProjectionWindow.current_views_.cells_[18].y_ - CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_f0_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e4_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[17].z_ = ((d4c - d50) - d54) * 0x100;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_a00_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[15].z_;
    CaviarKernelProjectionWindow.current_views_.cells_[19].x_ = CaviarKernelProjectionWindow.current_views_.cells_[22].x_ - CaviarKernelProjectionWindow.current_views_.cells_[15].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[18].z_ = (d4c - d50) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[19].y_ = CaviarKernelProjectionWindow.current_views_.cells_[22].y_ - CaviarKernelProjectionWindow.current_views_.cells_[15].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[19].z_ = ((d54 - d50) + d4c) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[21].x_ = (int)w40;
    CaviarKernelProjectionWindow.current_views_.cells_[20].x_ = CaviarKernelProjectionWindow.current_views_.cells_[21].x_ - CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[21].z_ = d4c << 8;
    CaviarKernelProjectionWindow.current_views_.cells_[21].y_ = (int)w46;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_f4_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[21].x_;
    CaviarKernelProjectionWindow.current_views_.cells_[22].z_ = iVar4 * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[20].y_ = CaviarKernelProjectionWindow.current_views_.cells_[21].y_ - CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e8_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[21].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[23].x_ = CaviarKernelProjectionWindow.current_views_.cells_[24].x_ - CaviarKernelProjectionWindow.current_views_.cells_[13].x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_a04_ = (float)CaviarKernelProjectionWindow.current_views_.cells_[21].z_;
    CaviarKernelProjectionWindow.current_views_.cells_[23].y_ = CaviarKernelProjectionWindow.current_views_.cells_[24].y_ - CaviarKernelProjectionWindow.current_views_.cells_[13].y_;
    CaviarKernelProjectionWindow.current_views_.cells_[24].z_ = (d4c + d50) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[23].z_ = ((d50 - d54) + d4c) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[20].z_ = (d4c - d54) * 0x100;
    CaviarKernelProjectionWindow.current_views_.cells_[25].z_ = (iVar4 + d50) * 0x100;

    unsigned int outputIndex = 0;
    int localCount = 0x1a;
    CaviarProjectionCell *piVar2 = CaviarKernelProjectionWindow.current_views_.cells_;
    do {
        unsigned int uVar1 = (unsigned int)piVar2->y_;
        int recA = piVar2->x_;
        CaviarOutputMessageWindow.projection_words_[outputIndex + 5] = (uint32_t)(((int)uVar1 >> 8) *
            (int32_t)vox_setup_copy_[4] + (recA >> 8));
        CaviarOutputMessageWindow.projection_words_[outputIndex] =
            (uint32_t)recA * 0x01000000u + (uVar1 & 0xffu);
        CaviarOutputMessageWindow.projection_words_[outputIndex + 6] =
            (((unsigned int)piVar2->z_ >> 0x10) +
            (unsigned int)piVar2->z_ * 0x10000) & 0xff00ffff;
        outputIndex += 4;
        localCount = localCount - 1;
        ++piVar2;
    } while (localCount != 0);

}


void __cdecl caviar_original_6647f0(
    int32_t, int32_t, int32_t, uint32_t, const uint8_t *, int32_t);
void __cdecl caviar_original_664000(
    int32_t, int32_t, int32_t, const int32_t *, uint32_t,
    const uint8_t *, int32_t);
void __cdecl caviar_original_665640(
    int32_t, int32_t, int32_t, uint32_t, const uint8_t *, int32_t);
void __cdecl caviar_original_664e50(
    int32_t, int32_t, int32_t, const int32_t *, uint32_t,
    const uint8_t *, int32_t);
void __cdecl caviar_original_666440(
    int32_t, int32_t, int32_t, uint32_t, const uint8_t *, int32_t);
void __cdecl caviar_original_665ca0(
    int32_t, int32_t, int32_t, const int32_t *, uint32_t,
    const uint8_t *, int32_t);
void __cdecl caviar_original_667b20(
    int32_t, int32_t, int32_t, uint32_t, const uint8_t *, int32_t);
void __cdecl caviar_original_667380(
    int32_t, int32_t, int32_t, const int32_t *, uint32_t,
    const uint8_t *, int32_t);
void __cdecl caviar_original_666a80(
    int32_t, int32_t, const int32_t *, uint32_t,
    const uint8_t *, int32_t);

void __cdecl caviar_original_63aee0();
void __cdecl caviar_original_63af00();
void __cdecl caviar_original_63af20();
void __cdecl caviar_original_63af40();
void __cdecl caviar_original_666989();
void __cdecl caviar_original_66638f();
void __cdecl caviar_original_6646fb();
void __cdecl caviar_original_664d51();
void __cdecl caviar_original_668069();
void __cdecl caviar_original_667a6f();
void __cdecl caviar_original_665ba1();
void __cdecl caviar_original_66554b();
void __cdecl caviar_original_666f68();
#include "caviar_raster_kernels.inc"

// Contained-shadow callback: the pinned CFG reaches its common return at
// 0x0066714D; the terminal jump ends at exclusive 0x0066731C, where the
// separately catalogued shared self-modifying setup helper begins.
OPENSMACX_NOINLINE
// ORIGINAL: 0x00666FD0 sub_666fd0 0x00666FD0-0x0066731C
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "build/recovery/caviar-owner-refresh-oct6/shadow-666fd0-boundary-evidence.md:217-reachable-instructions-831-bytes-13-padding-bytes", "source:src/caviar_raster_kernels.inc:caviar_shadow_kernel_candidate", "docs/recovery/ida9-functions.csv:neighbor-shared-helper-0x0066731C-0x00667375"], "reason": "Recover the contained-shadow callback on the genuine Caviar render record; local wrapper name is descriptive, not an original decorated symbol. Pinned reachable CFG and eight relocated dispatch pointers establish the entry; terminal branch ends exactly where the independently catalogued setup helper begins. Shared source kernel implements the callback, with x87 exceptional and allocation/lifetime fidelity still unresolved."}
// symbol    ?caviar_original_666fd0@@YAXHHIPBEH@Z
void __cdecl caviar_original_666fd0(
    int32_t x, int32_t y, uint32_t count, const uint8_t *stream,
    int32_t flags) {
    caviar_shadow_kernel_candidate(0, x, y, 0, count, stream, flags);
}

// SEED HISTORY: 0x0063AEE0 previously used FILE mode with spans 0063AEE0-0063AEF4;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden;sp_ready;purged_ok; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x0066638F 0x00666989 0x00666F68.
// SEED HISTORY: former direct-call order 0x00666989 -> 0x0066638F -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AEE0 sub_63aee0 0x0063AEE0-0x0063AEF4;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene calls the record setup callback before the instance walk", "source:src/caviar_raster_kernels.inc:caviar_install_record_kernels_candidate selects the mode-specific callback", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The mode-specific wrapper delegates to the existing typed prepared-state callback; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63aee0@@YAXXZ
// TRIED: `int` return (no eax set before final ret, changed to `void`); address range carries a _SELFMOD (copy-protection) span, so the harness's authoritative check REFUSES any body here regardless of content - MISMATCH is the best obtainable signal via --dir.
// kind      game
void __cdecl caviar_original_63aee0() {
    caviar_setup_size2_aligned_candidate();
}

// SEED HISTORY: 0x0063AF00 previously used FILE mode with spans 0063AF00-0063AF14;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x006646FB 0x00664D51 0x00666F68.
// SEED HISTORY: former direct-call order 0x00664D51 -> 0x006646FB -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF00 sub_63af00 0x0063AF00-0x0063AF14;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene calls the record setup callback before the instance walk", "source:src/caviar_raster_kernels.inc:caviar_install_record_kernels_candidate selects the mode-specific callback", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The mode-specific wrapper delegates to the existing typed prepared-state callback; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af00@@YAXXZ
// TRIED: nothing to rule out on tier - REFUSED here is unconditional (SELFMOD span), verified with a `return 0;` stub scoring MISMATCH while the correct tail-call form scores REFUSED.
// kind      game
void __cdecl caviar_original_63af00() {
    caviar_setup_size1_aligned_candidate();
}

// SEED HISTORY: 0x0063AF20 previously used FILE mode with spans 0063AF20-0063AF34;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x00666F68 0x00667A6F 0x00668069.
// SEED HISTORY: former direct-call order 0x00668069 -> 0x00667A6F -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF20 sub_63af20 0x0063AF20-0x0063AF34;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene calls the record setup callback before the instance walk", "source:src/caviar_raster_kernels.inc:caviar_install_record_kernels_candidate selects the mode-specific callback", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The mode-specific wrapper delegates to the existing typed prepared-state callback; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af20@@YAXXZ
// kind      game
void __cdecl caviar_original_63af20() {
    caviar_setup_size2_odd_candidate();
}

// SEED HISTORY: 0x0063AF40 previously used FILE mode with spans 0063AF40-0063AF54;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x0066554B 0x00665BA1 0x00666F68.
// SEED HISTORY: former direct-call order 0x00665BA1 -> 0x0066554B -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF40 sub_63af40 0x0063AF40-0x0063AF54;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene calls the record setup callback before the instance walk", "source:src/caviar_raster_kernels.inc:caviar_install_record_kernels_candidate selects the mode-specific callback", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The mode-specific wrapper delegates to the existing typed prepared-state callback; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af40@@YAXXZ
// TRIED: straight-line body after the 3 calls. The original's own tail is a `jmp` into a block shared with another caller (spans two disjoint ranges); inlining it here compiles to `mov` straight-through instead of the shared-tail `jmp`.
// kind      game
void __cdecl caviar_original_63af40() {
    caviar_setup_size1_odd_candidate();
}

// SEED HISTORY: former 0x00666989 record sub_666989 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00666989..0x00666A7D; 244-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00666989 sub_666989 0x00666989-0x00666A7D
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_666989@@YAXXZ
// TRIED: was `EXCLUDED S2a`; disassembly is a normal `ret`, no tail-jmp, self-contained store sequence - not an EH funclet by docs/EXCLUSIONS.md 2a's own test (see 0x00664D51's note in full). Un-excluded.
void __cdecl caviar_original_666989() {
    caviar_prepare_part_destination_bindings_candidate(2);
}

// SEED HISTORY: former 0x0066638F record sub_66638f was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x0066638F..0x00666437; 168-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x0066638F sub_66638f 0x0066638F-0x00666437
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_66638f@@YAXXZ
void __cdecl caviar_original_66638f() {
    caviar_prepare_part_source_bindings_candidate();
}

// SEED HISTORY: former 0x006646FB record sub_6646fb was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x006646FB..0x006647E3; 232-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x006646FB sub_6646fb 0x006646FB-0x006647E3
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_6646fb@@YAXXZ
void __cdecl caviar_original_6646fb() {
    caviar_prepare_part_source_bindings_candidate();
}

// SEED HISTORY: former 0x00664D51 record sub_664d51 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00664D51..0x00664E43; 242-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00664D51 sub_664d51 0x00664D51-0x00664E43
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_664d51@@YAXXZ
// TRIED: this was carried as `EXCLUDED S2a` (C++ EH unwind funclet) by a prior bulk pass, but the disassembly contradicts that ground: docs/EXCLUSIONS.md 2a's own definition is "no frame of its own ... no prologue, no epilogue, no ret - a tail jmp into another symbol's entry", and this function has a normal `ret`, no tail-jmp into anyone else's entry point, and is fully self-contained (reads six globals, republishes them across table records, returns). It was swept into the funclet population by proximity - it sits in the byte gap between two much larger EH-funclet-bearing functions (0x006647F0 and 0x00664E50) - not by matching the population's own test. Un-excluded here so the body below can be measured.
// TRIED: combining the two `+= g_9be6b4` adds into one expression (`value + g_9be6b4 + g_9be6b4`) vs. splitting them into two statements - VC6 lowers both to the same `lea`/`mov` pair either way (original is two separate `add`); tried both, identical rebuilt bytes. Kept split as the more faithful source form. First divergence #18 add/mov (mnemonic similarity 0.92, 4 edit blocks, none of them affecting this expression's semantics).
void __cdecl caviar_original_664d51() {
    caviar_prepare_part_destination_bindings_candidate(1);
}

// SEED HISTORY: former 0x00668069 record sub_668069 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00668069..0x0066815D; 244-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00668069 sub_668069 0x00668069-0x0066815D
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_668069@@YAXXZ
// TRIED: was `EXCLUDED S2a`; disassembly is a normal `ret`, no tail-jmp, self-contained store sequence - not an EH funclet by docs/EXCLUSIONS.md 2a's own test (see 0x00664D51's note in full). Un-excluded.
void __cdecl caviar_original_668069() {
    caviar_prepare_part_destination_bindings_candidate(2);
}

// SEED HISTORY: former 0x00667A6F record sub_667a6f was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00667A6F..0x00667B17; 168-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00667A6F sub_667a6f 0x00667A6F-0x00667B17
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_667a6f@@YAXXZ
void __cdecl caviar_original_667a6f() {
    caviar_prepare_part_source_bindings_candidate();
}

// SEED HISTORY: former 0x00665BA1 record sub_665ba1 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00665BA1..0x00665C93; 242-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00665BA1 sub_665ba1 0x00665BA1-0x00665C93
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_665ba1@@YAXXZ
// TRIED: was `EXCLUDED S2a`; same shape as 0x00664D51's sibling (see that file's note in full) - the disassembly is a normal `ret`, no tail-jmp, self-contained store sequence, not an EH funclet by docs/EXCLUSIONS.md 2a's own test. Un-excluded.
// TRIED: combining `value + g_9be6b4 + g_9be6b4` into one expression vs. splitting into two statements - identical rebuilt bytes either way (VC6 folds both into one `lea`/`mov` pair; original is two separate `add`). Kept split as the more faithful source form. First divergence #18 add/mov, mnemonic similarity 0.92, 4 edit blocks, unrelated to this expression's semantics.
void __cdecl caviar_original_665ba1() {
    caviar_prepare_part_destination_bindings_candidate(1);
}

// SEED HISTORY: former 0x0066554B record sub_66554b was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x0066554B..0x00665633; 232-byte seed; catalogued direct callers=1, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x0066554B sub_66554b 0x0066554B-0x00665633
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_66554b@@YAXXZ
void __cdecl caviar_original_66554b() {
    caviar_prepare_part_source_bindings_candidate();
}

// SEED HISTORY: former 0x00666F68 record sub_666f68 was BODY/EXCLUDED S2d; flags=hidden;sp_ready;purged_ok; kind=game; span=0x00666F68..0x00666FC1; 89-byte seed; catalogued direct callers=4, call targets=none; no BYTE_EXACT or SEMANTIC claim. The old exclusion note called these EH funclets; the pinned bodies return normally. Measurement still refuses for undocumented _SELFMOD range 0x00664000..0x00669000. The setup writes executable-page immediates; the replacement publishes typed state instead.
OPENSMACX_NOINLINE
// kind      game
// ORIGINAL: 0x00666F68 sub_666f68 0x00666F68-0x00666FC1
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_draw_scene copies descriptor slots 0/1/2, 3/4/5 and active ramp/palette before setup callback", "source:src/caviar.h:VoxRenderRecord::ramp_a_ptr_ is the per-record depth-row pointer table", "source:src/caviar_raster_kernels.inc:typed setup stages bind record depth rows, part inputs/output lanes, and shadow destination", "build/recovery/next-game-coverage-oct5/caviar-kernel-closure-oct6.md:original four setup call orders", "docs/recovery/ida9-functions.csv:shared helper 0x0066731C-0x00667375 remains a separate retained span on setup callback records"], "reason": "Replace this EXCLUDED S2d immediate-patch helper with a mapped typed setup operation. The four public setup callbacks preserve the pinned helper order; no executable-page writes are retained."}
// symbol    ?caviar_original_666f68@@YAXXZ
void __cdecl caviar_original_666f68() {
    caviar_prepare_shadow_bindings_candidate();
}

// The prior seed classifications were EXCLUDED S2d. These now have typed,
// compiled callback implementations, but no BYTE_EXACT claim is made. Each
// following SEED HISTORY line retains that former classification and flags.
// SEED HISTORY (prior excluded record): 0x006647F0 sub_6647f0 0x006647F0-0x00664D51 EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x006647F0 sub_6647f0 0x006647F0-0x00664D51
// symbol    ?caviar_original_6647f0@@YAXHHHIPBEH@Z
// kind      game
void __cdecl caviar_original_6647f0(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s1a_contained_candidate(x, y, z, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00664000 sub_664000 0x00664000-0x006646FB EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00664000 sub_664000 0x00664000-0x006646FB
// symbol    ?caviar_original_664000@@YAXHHHPBHIPBEH@Z
// kind      game
void __cdecl caviar_original_664000(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s1a_clipped_candidate(x, y, z, rect, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00665640 sub_665640 0x00665640-0x00665BA1 EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00665640 sub_665640 0x00665640-0x00665BA1
// symbol    ?caviar_original_665640@@YAXHHHIPBEH@Z
// kind      game
void __cdecl caviar_original_665640(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s1o_contained_candidate(x, y, z, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00664E50 sub_664e50 0x00664E50-0x0066554B EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00664E50 sub_664e50 0x00664E50-0x0066554B
// symbol    ?caviar_original_664e50@@YAXHHHPBHIPBEH@Z
// kind      game
void __cdecl caviar_original_664e50(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s1o_clipped_candidate(x, y, z, rect, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00666440 sub_666440 0x00666440-0x00666989 EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00666440 sub_666440 0x00666440-0x00666989
// symbol    ?caviar_original_666440@@YAXHHHIPBEH@Z
// kind      game
void __cdecl caviar_original_666440(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s2a_contained_candidate(x, y, z, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00665CA0 sub_665ca0 0x00665CA0-0x0066638F EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00665CA0 sub_665ca0 0x00665CA0-0x0066638F
// symbol    ?caviar_original_665ca0@@YAXHHHPBHIPBEH@Z
// kind      game
void __cdecl caviar_original_665ca0(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s2a_clipped_candidate(x, y, z, rect, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00667B20 sub_667b20 0x00667B20-0x00668069 EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00667B20 sub_667b20 0x00667B20-0x00668069
// symbol    ?caviar_original_667b20@@YAXHHHIPBEH@Z
// kind      game
void __cdecl caviar_original_667b20(
    int32_t x, int32_t y, int32_t z, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s2o_contained_candidate(x, y, z, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00667380 sub_667380 0x00667380-0x00667A6F EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00667380 sub_667380 0x00667380-0x00667A6F
// symbol    ?caviar_original_667380@@YAXHHHPBHIPBEH@Z
// kind      game
void __cdecl caviar_original_667380(
    int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_part_s2o_clipped_candidate(x, y, z, rect, count, stream, flags);
}
// SEED HISTORY (prior excluded record): 0x00666A80 sub_666a80 0x00666A80-0x00666F68 EXCLUDED S2d; hidden;sp_ready;purged_ok
// ORIGINAL: 0x00666A80 sub_666a80 0x00666A80-0x00666F68
// symbol    ?caviar_original_666a80@@YAXHHPBHIPBEH@Z
// kind      game
void __cdecl caviar_original_666a80(
    int32_t x, int32_t y, const int32_t *rect, uint32_t count,
    const uint8_t *stream, int32_t flags) {
    caviar_shadow_clipped_callback_candidate(x, y, rect, count, stream, flags);
}

// 0x0063E950. Eight arguments: the record, the walked node, the part's
// position and its scaled matrix, camera rotation, state direction, shape scale
// and the visibility flag the draw computed. The matrix/direction inputs are
// not a pair of clip rectangles.
OPENSMACX_NOINLINE
// SEED HISTORY: archived draw seed mapping used FILE mode; current source ownership is a BODY mapping.
// ORIGINAL: 0x0063E950 sub_63e950 0x0063E950-0x0063EFFF
// symbol    ?vox_draw_part@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@32MH@Z
// kind      game
// TRIED: 8-int __cdecl signature transcribed faithfully from both branches (bit2==0 single-shot with nested bit4 split vs bit2-set do/while loop over pbVar7), following the sibling 0x0063F000's shape; prologue diverges at instr #11 (sub esp/push framing) - not chased further, large FP-heavy function, landed for coverage.
void __cdecl vox_draw_part(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    VOX_Matrix *camera_rotation, const VOX_Vect *state_direction,
    float shape_scale, int visible) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int clip[4];
    int item_box[6];
    int flags = visible != 0;

    sub_63fa80(part_matrix);
    caviar_direction_to_projection_index(part_matrix, state_direction,
                                         shape_scale, camera_rotation);
    caviar_clip_record_box(record, &scratch.part_clip_, clip);
    caviar_build_projection_tables();
    vox_projection_prepared_ = 0;

    if ((shape->field_0_ & 2) == 0) {
        int projected[3];
        caviar_flat_raster_box(shape, position, part_matrix, item_box);
        if ((shape->field_0_ & 4) != 0) {
            projected[0] = caviar_round_projection_coordinate(position->values[0]);
            projected[1] = caviar_round_projection_coordinate(position->values[1]);
            projected[2] = caviar_round_projection_coordinate(position->values[2]);
            vox_direction_to_octant(record);
            flags |= 0x10;
        } else {
            const float anchor[3] = {
                static_cast<float>(shape->raster_origin_[0]),
                static_cast<float>(shape->raster_origin_[1]),
                static_cast<float>(shape->raster_origin_[2])
            };
            int ignored_offset[2];
            caviar_project_raster_point(position, part_matrix, anchor, projected,
                                        ignored_offset);
        }
        if (!caviar_intersect_raster_box(item_box, clip)) return;
        const int contained = caviar_box_is_inside(item_box, clip);
        if ((shape->field_0_ & 8) != 0) flags |= 0x20;
        VoxRenderRecord *dispatch =
            record;
        if (contained) {
            dispatch->part_contained_(projected[0], projected[1], projected[2],
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        } else {
            dispatch->part_clipped_(projected[0], projected[1], projected[2],
                clip, static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        }
        return;
    }

    uint32_t visited_items = 0;
    const CvrRasterItemHeaderCandidate *item =
        reinterpret_cast<const CvrRasterItemHeaderCandidate *>(shape->item_stream_);
    if (shape->item_count_ == 0) return;
    do {
        int projected[3];
        int box_offset[2];
        caviar_raster_item_box(item, position, part_matrix, item_box,
                               projected, box_offset);
        if (item_box[4] != 0 && item_box[5] != 0)
            caviar_apply_projected_offset(item_box, box_offset);
        if ((item->flags_ & 1) == 0) {
            flags &= ~0x10;
        } else {
            if (vox_projection_prepared_ == 0) vox_direction_to_octant(record);
            flags |= 0x10;
        }
        if (caviar_intersect_raster_box(item_box, clip)) {
            const int contained = caviar_box_is_inside(item_box, clip);
            if ((shape->field_0_ & 8) != 0)
                flags |= 0x20;
            else
                flags &= ~0x20;
            VoxRenderRecord *dispatch =
                record;
            if (contained) {
                dispatch->part_contained_(projected[0], projected[1], projected[2],
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            } else {
                dispatch->part_clipped_(projected[0], projected[1], projected[2],
                    clip, static_cast<int>(item->item_count_), item->inline_payload_, flags);
            }
        }
        visited_items += item->item_count_;
        const uint32_t extent = (item->flags_ & 1) == 0
            ? item->item_count_ * 3u
            : *reinterpret_cast<const uint32_t *>(item->inline_payload_);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}

// 0x0063F000. Five arguments: the shadow branch's counterpart of the above.
OPENSMACX_NOINLINE
// SEED HISTORY: archived draw seed mapping used FILE mode; current source ownership is a BODY mapping.
// ORIGINAL: 0x0063F000 sub_63f000 0x0063F000-0x0063F5D7
// symbol    ?vox_draw_shadow@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@PBX@Z
// kind      game
// TRIED: 5-int __cdecl signature transcribed faithfully from the two branches (bit2==0 single-shot vs bit2-set do/while loop over pbVar6); prologue diverges at instr #0 (sub esp,0x58 vs push) - not chased further, large FP-heavy function, landed for coverage.
void __cdecl vox_draw_shadow(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    const void *camera_nonnull) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int clip[4];
    int flags = camera_nonnull != 0;
    int item_box[6];

    sub_63fa80(part_matrix);
    caviar_clip_record_box(record, &scratch.shadow_clip_, clip);
    caviar_build_projection_tables();
    vox_projection_prepared_ = 0;

    if ((shape->field_0_ & 2) == 0) {
        int projected[3] = {0, 0, 0};
        if ((shape->field_0_ & 4) != 0) {
            projected[0] = caviar_round_projection_coordinate(position->values[0]);
            projected[1] = caviar_round_projection_coordinate(position->values[1]);
            vox_inverse_direction_to_octant(part_matrix);
            flags |= 0x10;
        } else {
            const float anchor[3] = {
                static_cast<float>(shape->raster_origin_[0]),
                static_cast<float>(shape->raster_origin_[1]),
                static_cast<float>(shape->raster_origin_[2])
            };
            int ignored_offset[2];
            caviar_project_raster_point(position, part_matrix, anchor, projected,
                                        ignored_offset);
        }
        caviar_flat_raster_box(shape, position, part_matrix, item_box);
        if (!caviar_intersect_raster_box(item_box, clip)) return;
        const int contained = caviar_box_is_inside(item_box, clip);
        if ((shape->field_0_ & 8) != 0)
            flags |= 0x20;
        else
            flags &= ~0x20;
        VoxRenderRecord *dispatch =
            record;
        if (contained) {
            dispatch->shadow_contained_(projected[0], projected[1],
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        } else {
            dispatch->shadow_clipped_(projected[0], projected[1], clip,
                static_cast<int>(shape->item_count_), shape->item_stream_, flags);
        }
        return;
    }

    uint32_t visited_items = 0;
    const CvrRasterItemHeaderCandidate *item =
        reinterpret_cast<const CvrRasterItemHeaderCandidate *>(shape->item_stream_);
    if (shape->item_count_ == 0) return;
    do {
        int projected[3];
        int box_offset[2];
        caviar_raster_item_box(item, position, part_matrix, item_box,
                               projected, box_offset);
        if (item_box[4] != 0 && item_box[5] != 0)
            caviar_apply_projected_offset(item_box, box_offset);
        if ((item->flags_ & 1) == 0) {
            flags &= ~0x10;
        } else {
            if (vox_projection_prepared_ == 0)
                vox_inverse_direction_to_octant(part_matrix);
            flags |= 0x10;
        }
        if (caviar_intersect_raster_box(item_box, clip)) {
            const int contained = caviar_box_is_inside(item_box, clip);
            if ((shape->field_0_ & 8) != 0)
                flags |= 0x20;
            else
                flags &= ~0x20;
            VoxRenderRecord *dispatch =
                record;
            if (contained) {
                dispatch->shadow_contained_(projected[0], projected[1],
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            } else {
                dispatch->shadow_clipped_(projected[0], projected[1], clip,
                    static_cast<int>(item->item_count_), item->inline_payload_, flags);
            }
        }
        visited_items += item->item_count_;
        const uint32_t extent = (item->flags_ & 1) == 0
            ? item->item_count_ * 3u
            : *reinterpret_cast<const uint32_t *>(item->inline_payload_);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}

/*
Purpose: Draw the scene. Refuses to re-enter, stages the record and the
         renderer's state into the engine's scratch, calls the record's setup
         callback, and walks the instance table: for every part whose byte at
         the caller's step is not flagged, composes its position and rotation
         with the view, walks its level-of-detail chain, and either hands the
         part to the record's per-object callback or, when the shadow byte on
         the state asks for it, draws the part and its shadow through the
         second view.
// ORIGINAL: 0x00639E60 sub_639e60 0x00639E60-0x0063A279 FILE
// symbol    ?vox_draw_scene@@YAXPAUVoxRenderRecord@@PAUCaviarSceneContext@@PAUVoxCameraBlock@@H@Z
// size      1049 bytes
// callers   2   call targets   9
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627E20 0x006280E0 0x00628150 0x006282E0 0x006385D0 0x006396A0 0x0063A280 0x0063E950 0x0063F000
// indirect  0x00639F25 0x0063A0AF 0x0063A0EE
//
// HOMED 2026-09-06 from the file-local stand-in this definition replaces.
//
// THE FOUR ARGUMENTS ARE THE RENDERER'S OWN BLOCKS. The image's call sites
// (0x639E8F and 0x63A207) push the created record, `this + 0x18`, `this + 0xA5`
// and a step, so the three pointers are the render record, the state block
// VoxSceneState models, and the camera plus the channel bounds that follow it
// in Caviar - VoxCameraBlock.
//
// BUG IN THE ORIGINAL: the re-entrancy guard is `je` over a `jmp $+0` - a
//        two-byte jump to itself at 0x639E70 - so a second entry SPINS rather
//        than returning. Reproduced as written: the guard is live whenever
//        the draw is entered re-entrantly, and returning early would be a
//        different program.
// LEVER: 0 to a compiled body, 282 instructions against the image's 332 and
//        call_diff clean - the same nine callees in the same order, three of
//        them indirect off the record's own callback slots. The residual is
//        the FPU scheduler: the image keeps the part's position and its nine
//        scaled matrix elements in registers across the clip test, and the
//        spellings here stage them to memory instead.
*/
void __cdecl vox_draw_scene(VoxRenderRecord *record, CaviarSceneContext *context,
                            VoxCameraBlock *view, int step) {
    VoxSceneState *state = &context->state_;
    // Scratch the two setup helpers fill. The image computes one address and
    // pushes it for both of the first helper's trailing arguments.
    float staged[8];

    if (vox_scene_busy != 0) {
        for (;;) {
            // The image's own `jmp 0x639e70`.
        }
    }
    vox_scene_busy = 1;

    vox_stage_scene(record, context, view, step, staged, staged);
    vox_load_scene_view(record, context, view, step);

    vox_active_colour_table_ = record->colour_table_;
    vox_active_ramp_a_ = record->ramp_a_ptr_;
    vox_active_ramp_b_ = record->ramp_b_ptr_;
    vox_active_size_code_ = record->setup_size_code_;
    CaviarKernelProjectionWindow.current_views_.tail_.vox_scene_extent_ = ((uint32_t *)record->setup_block_)[0] -
                        ((uint32_t *)record->setup_block_)[1];
    vox_setup_copy_[3] = ((uint32_t *)record->setup_block_)[5];
    vox_setup_copy_[4] = ((uint32_t *)record->setup_block_)[6];
    vox_setup_copy_[5] = ((uint32_t *)record->setup_block_)[7];
    vox_setup_copy_[1] = ((uint32_t *)record->setup_block_)[0];
    vox_setup_copy_[0] = ((uint32_t *)record->setup_block_)[1];
    vox_setup_copy_[2] = ((uint32_t *)record->setup_block_)[2];

    // The record's own setup hook, called once before the walk.
    if (VoxRenderRecord::SetupCallback callback = record->setup_callback_) {
        callback();
    }

    // The "keep the scene rectangle too" byte is the context's field_A4_.
    const int want_shadow = context->field_A4_;

    for (int index = 0; index < state->instance_count_; ++index) {
        VoxInstance &instance = state->instances_[index];
        if ((instance.instance_colour_[step] & 0x80) != 0) {
            continue;
        }
        const CvrShape *node = reinterpret_cast<const CvrShape *>(
            vox_walk_scaled_chain(
                reinterpret_cast<unsigned char *>(
                    state->instance_steps_[index].shape_),
                CaviarSceneProjectionScratch.projected_scale_));

        // The part's world position: its own vector through the view's
        // rotation, plus the view's offset.
        VOX_Vect position;
        vox_matrix_vector_mul(CaviarSceneProjectionScratch.view_scene_matrix_.values,
                              instance.instance_vector_[step].values,
                              position.values);
        vector_add(position.values, CaviarSceneProjectionScratch.camera_offset_.values, position.values);

        // The level-of-detail cut decides whether the part is drawn at all:
        // the image seeds the flag with 1 and clears it when the walked
        // node's extent, scaled by the cut, is past the visibility cut.
        const float extent = node->scale_;
        int visible = 1;
        if (CaviarSceneProjectionScratch.projected_scale_ * extent > vox_visibility_cut_) {
            visible = 0;
        }

        if (vox_part_behind(&position, extent, view) != 0) {
            continue;
        }

        // The part's rotation is the view's rotation times its own, scaled
        // by the walked node's extent.
        VOX_Matrix part;
        vox_matrix_matrix_mul(CaviarSceneProjectionScratch.view_scene_matrix_.values,
                              instance.instance_matrix_[step].values,
                              part.values);
        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column) {
                part.values[row][column] *= extent;
            }
        }

        // The slot's second argument is this instance-table index, not the
        // animation step used above to select the instance's vectors/matrix.
        // The bounds pointer is the six-int window at view + 0x48; direction
        // is the context's light vector at +0x48.
        if (VoxRenderRecord::ObjectStartCallback callback =
                record->object_start_callback_) {
            callback(record, index, &position, &part,
                     view->bounds_.values_, &context->light_vector_);
        }
        vox_draw_part(record, node, &position, &part, &view->camera_.rotation,
                      &context->light_vector_, extent, visible);

        // Read +0x5C after drawing: vox_draw_part may change this slot, and
        // the original edge tests the post-draw value before calling it.
        if (VoxRenderRecord::PostPartCallback callback =
                record->post_part_callback_) {
            callback(record, index);
        }

        // The shadow pass, when the state asks for it and the record's
        // descriptor block names a shadow.
        if (want_shadow != 0 &&
            ((uint32_t *)record->setup_block_)[2] != 0) {
            VOX_Vect shadow;
            vox_matrix_vector_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                  instance.instance_vector_[step].values,
                                  shadow.values);
            vector_add(shadow.values, CaviarSceneProjectionScratch.shadow_centre_.values, shadow.values);

            const float radius = extent * vox_radius_scale_;
            const float span_x = radius * CaviarSceneProjectionScratch.shadow_radius_x_;
            const float span_y = radius * CaviarSceneProjectionScratch.shadow_radius_y_;
            if (-span_x <= shadow.values[0] &&
                shadow.values[0] <=
                    static_cast<float>(view->bounds_.values_[4]) + span_x &&
                -span_y <= shadow.values[1] &&
                shadow.values[1] <=
                    static_cast<float>(view->bounds_.values_[5]) + span_y) {
                VOX_Matrix shadow_part;
                vox_matrix_matrix_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                      instance.instance_matrix_[step].values,
                                      shadow_part.values);
                // The shadow renderer scales only the first two rows.
                for (int shadow_row = 0; shadow_row < 2; ++shadow_row) {
                    for (int shadow_column = 0; shadow_column < 3; ++shadow_column) {
                        shadow_part.values[shadow_row][shadow_column] *= extent;
                    }
                }
                vox_draw_shadow(record, node, &shadow, &shadow_part,
                                &view->bounds_);
            }
        }
    }

    vox_scene_busy = 0;
}

/*
Purpose: Draw the scene. Resets the six channel bounds to their defaults,
         records the Buffer, the instance count and the draw's flags, hands the
         render record, the viewport state and the camera to the engine's draw
         with this renderer published as the callback's object, then unions the
         clip rectangle the draw produced into the viewport and clears the
         buffers the caller asks for.
// ORIGINAL: 0x00617080 ?UNK1@Caviar@@QAEHHH@Z 0x00617080-0x0061719E
// symbol    ?render_scene@Caviar@@QAEHPAVBuffer@@H@Z
//           (renamed from the catalogue's UNK1: the body is the renderer's
//           own top-level draw. The catalogued first argument is `int`, but it
//           is stored into the same slot set_buffer writes a Buffer* to, so
//           the declaration says Buffer* and the emitted name carries PAV.)
// size      286 bytes
// prototype int (__thiscall ?UNK1@Caviar@@QAEHHH@Z)(Caviar* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618470 0x00639E60
//
// The catalogue's second argument is the draw's flags: bit 0 is passed on to
// clear_buffers, bit 1 says the colour table the draw filled is not to be
// kept - which is why it lands in scene_context_.field_A4_, the byte init seeds with 1.
Return Value: 0, 7 when the renderer is switched off (field_4_'s bit 0 is
              clear), 0x10 on a null Buffer, and nothing further is drawn when
              no instance has been added.
Status: Complete
*/
int Caviar::render_scene(Buffer *buffer, int flags) {
    if ((field_4_ & 1) == 0) {
        return 7;
    }
    if (buffer == 0) {
        return 0x10;
    }
    if (field_13CC_ == 0) {
        return 0;
    }
    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    camera_view_.bounds_.values_[4] = 0x100;
    camera_view_.bounds_.values_[5] = 0x100;
    buffer_ = buffer;
    scene_context_.state_.instance_count_ = field_13CC_;
    scene_context_.state_.draw_flags_ = 1;
    scene_context_.field_7C_ = 0;
    scene_context_.field_80_ = 0;
    scene_context_.field_84_ = 0;
    scene_context_.field_A4_ = (flags & 2) != 0 ? 0 : 1;
    CaviarScene.renderer_ = this;
    vox_draw_scene(CaviarScene.created_, &scene_context_, &camera_view_, 0);
    CaviarScene.renderer_ = 0;
    RECT drawn;
    drawn.left = CaviarScene.created_->clip_max_a_x_;
    drawn.top = CaviarScene.created_->clip_max_a_y_;
    drawn.right = CaviarScene.created_->clip_min_a_x_ + 1;
    drawn.bottom = CaviarScene.created_->clip_min_a_y_ + 1;
    UnionRect(reinterpret_cast<RECT *>(&CaviarViewportState), &drawn,
              reinterpret_cast<RECT *>(&CaviarViewportState));
    clear_buffers(flags & 1);
    return 0;
}

// ---------------------------------------------------------------------------
// Geometry shared by scene drawing and camera transformation. Matrix inputs
// use their real row bounds; output stores are sequential, with fresh input
// reads for each result, preserving the original overlap behavior.
// The separate AABB walk below remains an original-address dependency.
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
Purpose: The voxel-facet bounding box: start the min/max pair at the
         caller's origin vector, then walk the eight corners of the box the
         three edge vectors span - +edge_b, +edge_c, -edge_b, +edge_d,
         +edge_b, -edge_c, -edge_b - widening min/max after every step.
         The first step is the tree's own vector_add (0x00628150); the
         rest are component adds the original inlines.

// ORIGINAL: 0x0063A9D0 sub_63a9d0 0x0063A9D0-0x0063AD54
// symbol    _sub_63a9d0
// size      900 bytes
// Deleted seed provenance: git:1ed59304110fc2098fb1dd11457fa5b0074169bd:src/unrecovered/0063a9d0.cpp
Status: Coverage transcription of the retained seed (src/unrecovered/0063a9d0.cpp,
        plain FILE). The seed's TRIED wall stands: the max-update predicate
        is written value-first (`local > max` against the original's
        `ah & 0x41` FPU polarity test) and the compare scheduling still
        diverges; with this TU's /Oy- frame against the original's
        frameless one the streams separate early, documented and not
        chased. Parameters keep the seed's reading (VOX_Vect *origin plus
        three edges and the two outputs; `a`/`b`/`c`/`d` in the seed and
        Ghidra).
*/
extern "C" void __cdecl sub_63a9d0(VOX_Vect *origin, VOX_Vect *edge_b,
                                   VOX_Vect *edge_c, VOX_Vect *edge_d,
                                   VOX_Vect *min_out, VOX_Vect *max_out) {
    *min_out = *origin;
    *max_out = *origin;

    VOX_Vect walked;
    vector_add(origin->values, edge_b->values, walked.values);

    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = edge_c->values[0] + walked.values[0];
    walked.values[1] = walked.values[1] + edge_c->values[1];
    walked.values[2] = edge_c->values[2] + walked.values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = walked.values[0] - edge_b->values[0];
    walked.values[1] = walked.values[1] - edge_b->values[1];
    walked.values[2] = walked.values[2] - edge_b->values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = edge_d->values[0] + walked.values[0];
    walked.values[1] = walked.values[1] + edge_d->values[1];
    walked.values[2] = edge_d->values[2] + walked.values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = edge_b->values[0] + walked.values[0];
    walked.values[1] = walked.values[1] + edge_b->values[1];
    walked.values[2] = edge_b->values[2] + walked.values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = walked.values[0] - edge_c->values[0];
    walked.values[1] = walked.values[1] - edge_c->values[1];
    walked.values[2] = walked.values[2] - edge_c->values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];

    walked.values[0] = walked.values[0] - edge_b->values[0];
    walked.values[1] = walked.values[1] - edge_b->values[1];
    walked.values[2] = walked.values[2] - edge_b->values[2];
    if (walked.values[0] < min_out->values[0]) min_out->values[0] = walked.values[0];
    if (walked.values[1] < min_out->values[1]) min_out->values[1] = walked.values[1];
    if (walked.values[2] < min_out->values[2]) min_out->values[2] = walked.values[2];
    if (walked.values[0] > max_out->values[0]) max_out->values[0] = walked.values[0];
    if (walked.values[1] > max_out->values[1]) max_out->values[1] = walked.values[1];
    if (walked.values[2] > max_out->values[2]) max_out->values[2] = walked.values[2];
}
/*
Purpose: The engine's camera transform. Walks the renderer's instance table
         once, and for every part whose shape it can reach builds the world
         position (scene rotation, the instance's own vector, the scene
         offset), the part's own rotation (scene rotation times the instance's
         matrix), the offset the shape's three per-axis values give, and the
         axis-aligned box its three scaled axes cover - accumulating the union
         of all of them into the caller's two vectors. Caviar::render hands it
         the renderer's state block and the step index, and subtracts the
         resulting centre from the scene offset when bit 4 asks for it.
// ORIGINAL: 0x0063A6F0 sub_63a6f0 0x0063A6F0-0x0063A9C9 FILE
// symbol    ?vox_transform_camera@@YAXPAUVoxSceneState@@HPAUVOX_Vect@@1@Z
//           (the image carries no name for this address; the fact records
//           what this tree emits. `VoxSceneState` is a struct, so its pointer
//           spells PAU - a `class` would spell PAV.)
// size      729 bytes
// callers   2   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627E20 0x006280E0 0x00628150 0x006281B0 0x0063A9D0
//
// HOMED 2026-09-06, homed from src/unrecovered/0063a6f0.cpp and from the
// file-local stand-in this definition replaces. THE FIRST ARGUMENT IS THE
// RENDERER: the
// image's call site is `lea eax, [ebp + 0x18]` with `this` in ebp, so the
// block the body walks is Caviar's own +0x18 on and every offset below lands
// on a Caviar member - the count, the instance table, the step table, the
// scene offset, the scene rotation and the scale. VoxSceneState is that
// binding written out.
//
// LEVER: the loop re-reads the count at +0x04 every iteration (image 0x63A9B5
//        `mov eax, [edi + 4]` at the bottom of the body), so the condition
//        below spells the field read out in full rather than caching it.
// TRIED: the artifact's flat-array spelling of the same arithmetic (locals
//        named centerA/centerB/mat/pivot, every field reached through a
//        `char *` offset). It compiles and it agrees nowhere near the image -
//        the 0x7c-byte frame the image fills with named floats is this loop's
//        own intermediates, and it needs the struct below to be reached with
//        member names at all. Byte-exactness here is the FPU scheduler's
//        interleaving of the three axis multiplies, not the source's.
*/
void __cdecl vox_transform_camera(VoxSceneState *state, int step,
                                  VOX_Vect *position, VOX_Vect *extent) {
    if (state->instance_count_ <= 0) {
        return;
    }

    int index = 0;
    do {
        VoxInstance &instance = state->instances_[index];
        CvrShape &shape = *state->instance_steps_[index].shape_;
        float scale = shape.scale_ * state->scale_;

        // The part's world position: its own vector through the scene
        // rotation, scaled by the scene's scale, then shifted by the scene
        // offset. The image reuses the rotated vector's buffer for the answer
        // (0x63A773 re-derives the same local it passed as the multiply's
        // output), and three separate locals below is the same program.
        VOX_Vect rotated;
        VOX_Vect scaled;
        VOX_Vect centre;
        vox_matrix_vector_mul(state->scene_rotation_.values,
                              instance.instance_vector_[step].values,
                              rotated.values);
        vec3_scale(rotated.values, state->scale_, scaled.values);
        vector_add(scaled.values, state->scene_offset_.values, centre.values);

        // The part's rotation is the scene rotation times the instance's own.
        VOX_Matrix part;
        vox_matrix_matrix_mul(state->scene_rotation_.values,
                              instance.instance_matrix_[step].values,
                              part.values);

        VOX_Vect offset;
        offset.values[0] = static_cast<float>(shape.offset_x_) * scale;
        offset.values[1] = static_cast<float>(shape.offset_y_) * scale;
        offset.values[2] = static_cast<float>(shape.offset_z_) * scale;
        VOX_Vect pivot;
        VOX_Vect corner;
        vox_matrix_vector_mul(part.values, offset.values, pivot.values);
        vector_add(pivot.values, centre.values, corner.values);

        // The three axes the box is built from are the part rotation's
        // COLUMNS - image 0x63A81E takes [esp+0x68], 0x63A835 [esp+0x6c] and
        // 0x63A88F [esp+0x9c] of the 9-float local, which are elements 0, 1
        // and 2 of a row-major 3x3, each scaled by its own shape axis.
        float axis_x = static_cast<float>(shape.axis_x_) * scale;
        float axis_y = static_cast<float>(shape.axis_y_) * scale;
        float axis_z = static_cast<float>(shape.axis_z_) * scale;
        VOX_Vect axes[3];
        axes[0].values[0] = part.values[0][0] * axis_x;
        axes[0].values[1] = part.values[1][0] * axis_x;
        axes[0].values[2] = part.values[2][0] * axis_x;
        axes[1].values[0] = part.values[0][1] * axis_y;
        axes[1].values[1] = part.values[1][1] * axis_y;
        axes[1].values[2] = part.values[2][1] * axis_y;
        axes[2].values[0] = part.values[0][2] * axis_z;
        axes[2].values[1] = part.values[1][2] * axis_z;
        axes[2].values[2] = part.values[2][2] * axis_z;

        VOX_Vect low;
        VOX_Vect high;
        sub_63a9d0(&corner, &axes[0], &axes[1], &axes[2], &low, &high);

        if (index == 0) {
            *position = low;
            *extent = high;
        } else {
            if (low.values[0] < position->values[0]) {
                position->values[0] = low.values[0];
            }
            if (low.values[1] < position->values[1]) {
                position->values[1] = low.values[1];
            }
            if (low.values[2] < position->values[2]) {
                position->values[2] = low.values[2];
            }
            if (extent->values[0] < high.values[0]) {
                extent->values[0] = high.values[0];
            }
            if (extent->values[1] < high.values[1]) {
                extent->values[1] = high.values[1];
            }
            if (extent->values[2] < high.values[2]) {
                extent->values[2] = high.values[2];
            }
        }

        ++index;
    } while (index < state->instance_count_);
}

/*
Purpose: The renderer's own draw. Rejects a draw with the renderer switched
         off or no instances, doubles the camera scaling when bit 5 asks for
         it, resets the channel bounds, clips the 0x100-deep box the caller
         centred on the scene surface, hands the rectangle to the render
         record, transforms the camera position by the scene rotation, draws
         through the engine, unions what was covered into the viewport and the
         scene rectangle, blits the colour table the draw filled, and clears.
// ORIGINAL: 0x00617230 ?render@Caviar@@QAEHPAUBuffer@@HHH@Z 0x00617230-0x00617620
// symbol    ?render@Caviar@@QAEHPAVBuffer@@HHH@Z
// size      1008 bytes
// prototype int (__thiscall ?render@Caviar@@QAEHPAUBuffer@@HHH@Z)(Caviar* this, Buffer*, int, int, int)
// callers   2   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063A6F0 0x00639E60 0x005D8EE0 0x005E34A3 0x005D84C0 0x00618470
Return Value: 0, 7 when the renderer is off or the Buffer is null, and 0 when
              there is nothing to draw.
Status: Complete
*/
int Caviar::render(Buffer *buffer, int centre_x, int centre_y, int flags) {
    if ((field_4_ & 1) == 0) {
        return 7;
    }
    if (buffer == 0) {
        return 7;
    }
    if (field_13CC_ == 0) {
        return 0;
    }
    if (field_4_ & 0x20) {
        camera_view_.camera_.scaling = camera_view_.camera_.scaling + camera_view_.camera_.scaling;
    }

    // DEFECT FIXED 2026-09-06: this body stored the BUFFER here, where the
    // image stores the instance count it has just tested - image 0x617266
    // `mov eax, [ebp + 0x13cc]` and 0x617292 `mov [ebp + 0x1c], eax`. The
    // count is what the engine's camera transform reads back out of the same
    // dword (+0x04 of the block it is handed), so the old store handed the
    // transform a pointer to walk as a loop bound. render_scene (0x00617080)
    // already spelled it correctly.
    scene_context_.state_.instance_count_ = field_13CC_;
    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    camera_view_.bounds_.values_[4] = 0x100;
    camera_view_.bounds_.values_[5] = 0x100;
    buffer_ = buffer;
    scene_context_.state_.draw_flags_ = 1;
    scene_context_.field_A4_ = (flags & 2) != 0 ? 0 : 1;

    int left = centre_x - 0x80;
    int top = centre_y - 0x80;
    RECT area;
    area.left = left;
    area.top = top;
    area.right = centre_x + 0x80;
    area.bottom = centre_y + 0x80;
    IntersectRect(&area, &area, &buffer->rect1_);
    area.left += 0x80 - centre_x;
    area.right += 0x80 - centre_x;
    area.top += 0x80 - centre_y;
    area.bottom += 0x80 - centre_y;

    // The engine draws in scene coordinates and reports back what it covered,
    // so the rectangle goes across in the scene's own frame and the extent
    // follows from it.
    VoxRenderRecord *record = CaviarScene.created_;
    record->draw_left_ = area.left;
    record->draw_top_ = area.top;
    record->draw_right_ = area.right;
    record->draw_bottom_ = area.bottom;
    record->draw_width_ = area.right - area.left;
    record->draw_height_ = area.bottom - area.top;

    if (flags & 4) {
        VOX_Matrix saved = scene_context_.state_.scene_rotation_;
        VOX_Vect position;
        VOX_Vect extent;
        vox_transform_camera(&scene_context_.state_, (flags & 0x10) != 0 ? flags : 0,
                             &position, &extent);
        position.values[0] = (extent.values[0] + position.values[0]) * 0.5f;
        scene_context_.state_.scene_rotation_ = saved;
        position.values[1] = (extent.values[1] + position.values[1]) * 0.5f;
        position.values[2] = (extent.values[2] + position.values[2]) * 0.5f;
        scene_context_.state_.scene_offset_.values[0] -= position.values[0];
        scene_context_.state_.scene_offset_.values[1] -= position.values[1];
        scene_context_.state_.scene_offset_.values[2] -= position.values[2];
    }

    CaviarScene.renderer_ = this;
    vox_draw_scene(CaviarScene.created_, &scene_context_, &camera_view_, 0);
    CaviarScene.renderer_ = 0;

    RECT covered;
    covered.left = record->clip_max_a_x_;
    covered.top = record->clip_max_a_y_;
    covered.right = record->clip_min_a_x_ + 1;
    covered.bottom = record->clip_min_a_y_ + 1;
    UnionRect(reinterpret_cast<RECT *>(&CaviarViewportState), &covered,
              reinterpret_cast<RECT *>(&CaviarViewportState));

    if (scene_context_.field_A4_ != 0) {
        RECT scene;
        scene.left = record->clip_max_b_x_;
        scene.top = record->clip_max_b_y_;
        scene.right = record->clip_min_b_x_ + 1;
        scene.bottom = record->clip_min_b_y_ + 1;
        UnionRect(reinterpret_cast<RECT *>(&CaviarScene.field_00_), &scene,
                  reinterpret_cast<RECT *>(&CaviarScene.field_00_));
        if (scene_context_.field_A4_ != 0) {
            unsigned char *colour = static_cast<unsigned char *>(record->colour_table_);
            int mode;
            int origin_x;
            int origin_y;
            if (field_4_ & 0x20) {
                mode = 2;
                origin_x = centre_x - 0x40;
                origin_y = centre_y - 0x40;
            } else {
                mode = 1;
                origin_x = left;
                origin_y = top;
            }
            CaviarSceneBufferB.draw_dest_source(buffer, 0, origin_x, origin_y,
                                                1, mode, colour);
        }
    }

    buffer->free_data(1);

    if ((flags & 8) == 0) {
        int mode;
        int origin_x;
        int origin_y;
        if (field_4_ & 0x20) {
            mode = 2;
            origin_x = centre_x - 0x40;
            origin_y = centre_y - 0x40;
        } else {
            mode = 1;
            origin_x = left;
            origin_y = top;
        }
        CaviarSceneBufferA.draw(buffer, 9, origin_x, origin_y, 1, mode);
        buffer->free_data(1);
        clear_buffers(flags & 1);
    }

    return 0;
}

/*
Purpose: Read a single 4-byte field at offset +8 of the caller's block: the
         CVR loader's fixed-size header reader, vox_read_exact wrapped for one
         dword.
// ORIGINAL: 0x006386B0 sub_6386b0 0x006386B0-0x006386CC BYTE_EXACT
// symbol    _vox_read_dword
// size      28 bytes
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638650
//
// CLAIMED BYTE_EXACT, homed from src/recovered/006386b0.cpp 2026-09-04 and
// re-measured here. NAMED from the body: a four-byte read at +8 through
// vox_read_exact. Its one callee is homed in this file, so the call stays
// `call rel32`.
Return Value: 0 when the four bytes were read, -1 otherwise.
*/
extern "C" int __cdecl vox_read_dword(int fd, CvrRecord *record) {
    return vox_read_exact(fd, &record->version_, sizeof(record->version_)) ? -1 : 0;
}

/*
Purpose: Fill the engine's clip rectangle from the created render record and
         return its address, or null when no record has been created.
// ORIGINAL: 0x00618520 ?UNK13@Caviar@@QAEXXZ 0x00618520-0x00618556 BYTE_EXACT
// symbol    ?clip_rect@Caviar@@QAEPAXXZ
// size      54 bytes
// prototype void (__thiscall ?UNK13@Caviar@@QAEXXZ)(Caviar* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
//
// CLAIMED BYTE_EXACT, homed from src/recovered/units/00618520.cpp 2026-09-04
// and re-measured here. Its five bindings are named from the image:
// g_009bb478 is the created render record, CaviarScene.created_ in the scene
// record this file models, and the g_009b96a0/a4/a8/ac quad is the engine's
// clip rectangle, vox_clip_rect here - all four values come off the record's
// own clip fields, so the name is the body's, not a guess.
// NOTE the catalogue's `X` (void) return: the body leaves the rectangle's
// address in eax, so this tree spells the return type it really has.
Return Value: the clip rectangle, or null.
*/
void *Caviar::clip_rect() {
    VoxRenderRecord *record = CaviarScene.created_;
    if (record == 0) {
        return 0;
    }
    vox_clip_rect[0] = record->clip_max_a_x_;
    vox_clip_rect[2] = record->clip_min_a_x_ + 1;
    vox_clip_rect[1] = record->clip_max_a_y_;
    vox_clip_rect[3] = record->clip_min_a_y_ + 1;
    return vox_clip_rect;
}

/*
Purpose: Copy the created render record's clip bounds into a caller's array:
         {max_x, max_y, min_x + 1, min_y + 1}. A null array or a null record
         leaves the caller's memory alone.
// ORIGINAL: 0x00618560 ?UNK14@Caviar@@QAEXH@Z 0x00618560-0x0061859F
// symbol    ?fill_clip_rect@Caviar@@QAEXH@Z
// size      64 bytes
// prototype void (__thiscall ?UNK14@Caviar@@QAEXH@Z)(Caviar* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
//
// CLAIMED BYTE_EXACT, homed from src/recovered/units/00618560.cpp 2026-09-04
// and re-measured here. The scene record slot is the same one UNK13 reads,
// and the four values are vox_clip_rect's, written into the caller's array
// in the image's own order: max_x, min_x + 1, max_y, min_y + 1.
// TRIED: the created record is spelled CaviarScene.created_ at EVERY use,
//        never a cached local - the image reloads 0x9BB478 before each group
//        (0x618577, 0x618584, 0x618590) because the stores through the
//        caller's int * may alias it, and a local collapses those reloads
//        and undersizes the body.
// TRIED: 19 of 19 instructions agree positionally and the body carries one
//        instruction the image's listing does not - its own closing
//        `ret 4`, which the image's span excludes as a shared tail. Nested
//        guards against `&&` measure identically. Nothing in the source
//        reaches it; /c /O2 /Gy /GR- /GX.
Return Value: n/a
*/
void Caviar::fill_clip_rect(int destination) {
    int *out = (int *)destination;
    if (out != 0) {
        if (CaviarScene.created_ != 0) {
            out[0] = CaviarScene.created_->clip_max_a_x_;
            out[2] = CaviarScene.created_->clip_min_a_x_ + 1;
            out[1] = CaviarScene.created_->clip_max_a_y_;
            out[3] = CaviarScene.created_->clip_min_a_y_ + 1;
        }
    }
}

/*
Purpose: Install a shadow table into the created render record: the record's
         own 0x54 slot names the 0x100-byte block, and the caller's bytes are
         copied over it. 7 means no record, 0x10 means the record has no
         shadow table.
// ORIGINAL: 0x00618430 ?set_shadow_table@Caviar@@QAEHPAE@Z 0x00618430-0x00618467
// symbol    ?set_shadow_table@Caviar@@QAEHPAE@Z
// size      55 bytes
// prototype int (__thiscall ?set_shadow_table@Caviar@@QAEHPAE@Z)(Caviar* this, unsigned char*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0061845B
//
// CLAIMED BYTE_EXACT, homed from src/recovered/units/00618430.cpp 2026-09-04
// and re-measured here. Its binding g_009bb478 is CaviarScene.created_ here,
// and 0x54 is the record's shadow_table_ slot (caviar.h's VoxRenderRecord).
// The copy is a real out-of-line call in the image (0x645930), so it is
// spelled plain `memcpy` - the intrinsic expansion would rep movsd it.
Return Value: 0 on success, 7 without a record, 0x10 without a shadow table.
*/
// TRIED: 17 of 17 instructions agree positionally; the body carries one
//        instruction the image's listing does not - its own closing
//        `ret 4`, which falls one byte past the catalogued span - the ret
//        sits at 0x00618465 and ends at 0x00618468, the span at 0x00618467.
//        The
//        intrinsic spelling, this pragma spelling and the scaffold's
//        `_memcpy` indirection all land identically, so the claim is
//        demoted rather than carried.
// TRIED: the image CALLS memcpy here (0x645930) rather than letting it expand, so
// this one pragma turns the intrinsic back into a call for the rest of the
// file - which is this body alone, every earlier one being above it. The
// mirror of vox_stage_error's `#pragma intrinsic(strcpy)` at the foot, which
// works the other way round.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(memcpy)
#endif
int Caviar::set_shadow_table(unsigned char *table) {
    if (CaviarScene.created_ == 0) {
        return 7;
    }
    if (CaviarScene.created_->shadow_table_ == 0) {
        return 0x10;
    }
    memcpy(CaviarScene.created_->shadow_table_, table, 0x100);
    return 0;
}

/*
Purpose: Load a .cvr file into this slot: release whatever record it still
         holds, open the file, hand the descriptor to the loader, and close
         the descriptor again once the record is built - the slot keeps the
         record, not the file.
// ORIGINAL: 0x00616BD0 ?init@CaviarData@@QAEHPAD@Z 0x00616BD0-0x00616C55 FILE BYTE_EXACT
// symbol    ?init@CaviarData@@QAEHPAD@Z
// size      133 bytes
// prototype int (__thiscall ?init@CaviarData@@QAEHPAD@Z)(CaviarData* this, int8*)
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006381F0 0x00638430 0x006493C6 0x0064CF73
//
// CLAIMED BYTE_EXACT, homed from src/unrecovered/00616bd0.cpp 2026-09-04 and
// re-measured here. Its three helper bindings rename to bodies this file now
// owns or forwards: caviar_sub_638430 is caviar_free_record, homed above;
// caviar_sub_6381f0 is vox_load_record (0x006381F0);
// caviar_open is the CRT's _open, spelled plainly -
// 0x8000 is _O_BINARY.
Return Value: 0, 0x10 without a name, 6 when the open fails, 0x17 when the
              loader returns no record.
*/
int CaviarData::init(char *name) {
    if (name == 0) {
        return 0x10;
    }
    if (record_) {
        caviar_free_record(record_);
        record_ = 0;
    }
    int fd = _open(name, 0x8000);
    fileDescriptor_ = fd;
    if (fd == -1) {
        return 6;
    }
    CvrRecord *record = vox_load_record(fd);
    record_ = record;
    if (record == 0) {
        return 0x17;
    }
    if (fileDescriptor_ != 0) {
        _close(fileDescriptor_);
        fileDescriptor_ = 0;
    }
    return 0;
}

/*
Purpose: The engine's per-object callback. Fills 0x1800 bytes of the handle's
         colour table from wherever field_4_'s bits say the next draw reads:
         the shared monochrome source when bit 3 is on, the table the caller
         of render_with_colour_table handed over when bit 4 is on, otherwise
         this renderer's own entry for the instance the engine names.
// ORIGINAL: 0x00618DA0 ?object_start@Caviar@@QAAXPAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAUVOX_Rect@@PAUVOX_Vect@@@Z 0x00618DA0-0x00618E06 SEMANTIC
// symbol    ?object_start@Caviar@@SAXPAUVoxRenderRecord@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAH1@Z
//           (VC6 back-references the sixth argument - the same VOX_Vect as
//           the third - to `1`; the catalogued name spells it out in full.)
// size      102 bytes
// prototype void (__thiscall ?object_start@Caviar@@QAAXPAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAUVOX_Rect@@PAUVOX_Vect@@@Z)(Caviar* this, VOX_ScnRenderHnd*, long, VOX_Vect*, VOX_Matrix*, VOX_Rect*, VOX_Vect*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645930
//
// The original mapping and recovered prototype are retained above. The
// current callback takes int32_t* for bounds because the draw passes the
// six-int values_ member at view + 0x48. It is static so its address has the
// plain six-argument __cdecl type; the body reads only the handle and index.
//
// PLACED AFTER set_shadow_table so this file's `#pragma function(memcpy)`
// reaches it: the image CALLS memcpy here (0x00645930), and /Oi would expand
// a 0x1800-byte copy inline.
//
// NAMED 2026-09-05: the four unread arguments take the roles their types
// carry in the engine's per-object packet; none of them reaches the table.
//
// UNK1 note: the residual on 0x00617080 is register naming only - the image
// saves esi for `this` and edi for its zero register, this tree saves ebp and
// esi, so the same 86 instructions disagree on operand spellings throughout
// and `osmx semantic` refuses at instruction 2 (`mov` against `push`). No
// source spelling moved it: dropping the record local changed nothing.
Return Value: n/a
Status: Complete
*/
void __cdecl Caviar::object_start(VOX_ScnRenderHnd *handle, long instance,
                                  VOX_Vect *position, VOX_Matrix *orientation,
                                  int32_t *bounds, VOX_Vect *direction) {
    Caviar &self = *CaviarScene.renderer_;
    if (self.field_4_ & 8) {
        memcpy(handle->colour_table_, CaviarColourSource, 0x1800);
    } else if (self.field_4_ & 0x10) {
        memcpy(handle->colour_table_, CaviarScene.render_argument_, 0x1800);
    } else {
        memcpy(handle->colour_table_, self.instance_palettes_[instance], 0x1800);
    }
}

/*
Purpose: Point the renderer at a Buffer and reset the six channel bounds to
         their defaults {0, 0, 0xff, 0xff, 0x100, 0x100}. 7 means the
         renderer is not switched on (field_4_'s bit 0 is clear), 0x10 means
         the caller passed no Buffer.
// ORIGINAL: 0x006183D0 ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z 0x006183D0-0x00618424 FILE
// symbol    ?set_buffer@Caviar@@QAEHPAVBuffer@@@Z
// size      84 bytes
// prototype int (__thiscall ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z)(Caviar* this, Buffer*)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
//
// HOMED from src/recovered/units/006183d0.cpp. The raw offsets it wrote are
// caviar.h's declared members now: 0x14 is buffer_ (named from this very
// store) and 0xED..0x105 is camera_view_.bounds_.values_, the six-int group Caviar::init
// defaults to the same six values.
// TRIED: the byte test is NOT a problem - through the declared uint32_t
//        field_4_, VC6 emits the image's own `test byte ptr [ecx+4],1`.
// TRIED: two assignment orders, the image's own (0xED, 0xF1, 0xF5, 0xF9,
//        Buffer, 0xFD, 0x101) and the constant-grouped one; both land 10 of
//        20 because this tree's VC6 hoists the Buffer store and materialises
//        both constants ahead of the stores they feed, where the image keeps
//        the source order and reuses edx's zero from the null compare. 10 of
//        20, /c /O2 /Gy /GR- /GX.
Return Value: 0, 7 when the renderer is off, 0x10 when the Buffer is null.
*/
int Caviar::set_buffer(Buffer *buffer) {
    if ((field_4_ & 1) == 0) {
        return 7;
    }
    if (buffer == 0) {
        return 0x10;
    }
    // The assignment order is the image's store order (0xED, 0xF1, 0xF5,
    // 0xF9, then the Buffer, then 0xFD, 0x101): VC6 materialises each
    // constant into edx/eax as it reaches it and slides the Buffer store
    // between the two 0xff stores, which is exactly the interleaving the
    // image shows.
    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    buffer_ = buffer;
    camera_view_.bounds_.values_[4] = 0x100;
    camera_view_.bounds_.values_[5] = 0x100;
    return 0;
}

/*
Purpose: Set the shadow direction from an elevation and an azimuth:
         {cos(azimuth)*sin(elevation),
         sin(azimuth)*sin(elevation), cos(elevation)}, the vector a shadow
         falls along for a light at those angles.
// ORIGINAL: 0x006181D0 ?set_shadow_vector@Caviar@@QAEXMM@Z 0x006181D0-0x0061824F FILE
// symbol    ?set_shadow_vector@Caviar@@QAEXMM@Z
// size      127 bytes
// prototype void (__thiscall ?set_shadow_vector@Caviar@@QAEXMM@Z)(Caviar* this, float, float)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006463E4 0x00646494
//
// HOMED from src/unrecovered/006181d0.cpp. The three stores at 0x6C,
// 0x70 and 0x74 are the components of Caviar::scene_context_.shadow_vector_.
Return Value: n/a
*/
// The image CALLS cos here (0x646494) as it calls sin; stdafx's pragma line
// turns `sin` into a call but has no entry for `cos`, which this build
// compiles to the FPU's own `fcos`. One TU-local pragma makes it a call for
// the rest of the file - nothing after this body uses it.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(cos)
#endif
void Caviar::set_shadow_vector(float elevation, float azimuth) {
    // The trig names the angles: cos(elevation) is the vector's whole z
    // component and sin(elevation) scales the in-plane pair, so this angle
    // is measured from the z axis and azimuth around it.
    float sin_elevation = (float)sin(elevation);
    float sin_azimuth = (float)sin(azimuth);
    float cos_elevation = (float)cos(elevation);
    float cos_azimuth = (float)cos(azimuth);
    scene_context_.shadow_vector_.values[2] = cos_elevation;
    scene_context_.shadow_vector_.values[0] = cos_azimuth * sin_elevation;
    scene_context_.shadow_vector_.values[1] = sin_azimuth * sin_elevation;
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
    float length = (float)sqrt(x * x + y * y + z * z);
    float scale = 1.0f / length;
    out[0] = source[0] * scale;
    out[1] = source[1] * scale;
    out[2] = source[2] * scale;
}

/*
Purpose: Build the renderer's per-instance table and seed the scene state.
         Allocates the three destinations each of the 200 instances draws
         through - one colour byte, one vector, one matrix - seeds the
         rotation, the clipping defaults and the channel bounds, and marks the
         renderer switched on.
// ORIGINAL: 0x00616DE0 ?init@Caviar@@QAEXXZ 0x00616DE0-0x00617013
// symbol    ?init@Caviar@@QAEXXZ
// size      563 bytes
// prototype void (__thiscall ?init@Caviar@@QAEXXZ)(Caviar* this)
// callers   7   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00617020 0x00628220 0x006463E4 0x00646494
//
// The three allocations per instance are what name the CaviarData union's
// second view: 1 byte, 0xC bytes and 0x24 bytes are exactly a colour byte, a
// VOX_Vect and a VOX_Matrix, and they land in the same three dwords the .cvr
// loader fills with the engine's own handles.
//
// PLACED LAST so the file's `#pragma function(cos)` reaches it: the image
// CALLS sin and cos here, and /Oi would compile cos to the FPU's own `fcos`.
//
// LEVER: the seven float seeds are typed float, not uint32_t with a memcpy -
// this file's `#pragma function(memcpy)` (which set_shadow_table needs) turns
// every 4-byte memcpy in init into a call, and 254 compiled instructions
// against the image's 151 was that, not the body. With the fields typed for
// what they hold the stores fold back to immediates, and the down vector at
// +0x60 reads back out as the shadow direction with two dword moves, which is
// what the image does too.
//
// TRIED: the residual is store ORDER. The image copies the normalised down
// vector into +0x6c/+0x70 before it zeroes +0x40..+0x54 and the rotation's
// off-diagonal elements; this tree schedules that copy last. Same stores,
// same values, 143 against 151 instructions.
Return Value: n/a
Status: Complete
*/
void Caviar::init() {
    close();

    for (int index = 0; index < 200; ++index) {
        VoxInstance &instance = slots_[index];
        instance.instance_colour_ = static_cast<uint8_t *>(mem_get(1));
        if (instance.instance_colour_ == 0) {
            close();
            return;
        }
        instance.instance_vector_ =
            static_cast<VOX_Vect *>(mem_get(0xC));
        if (instance.instance_vector_ == 0) {
            close();
            return;
        }
        instance.instance_matrix_ =
            static_cast<VOX_Matrix *>(mem_get(0x24));
        if (instance.instance_matrix_ == 0) {
            close();
            return;
        }
    }

    field_13CC_ = 0;
    field_108_ = 0x42C80000;

    float sin_elevation = (float)sin(1.0);
    float sin_zero = (float)sin(0.0);
    float cos_elevation = (float)cos(1.0);
    float cos_azimuth = (float)cos(0.0);

    scene_context_.shadow_vector_.values[0] = cos_azimuth * sin_elevation;
    scene_context_.shadow_vector_.values[1] = sin_zero * sin_elevation;
    scene_context_.shadow_vector_.values[2] = cos_elevation;

    scene_context_.state_.scene_offset_.values[0] = 0.0f;
    scene_context_.state_.scene_offset_.values[1] = 0.0f;
    scene_context_.state_.scene_offset_.values[2] = 0.0f;

    const float negative_distance_limit = -65535.0f;
    const float positive_distance_limit = 65535.0f;
    scene_context_.state_.scale_ = 1.0f;

    camera_view_.camera_.scaling = 1.0f;

    camera_view_.bounds_.lead_[3] = 0;
    camera_view_.bounds_.lead_[4] = positive_distance_limit;

    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    camera_view_.bounds_.values_[4] = 0x100;
    camera_view_.bounds_.values_[5] = 0x100;

    scene_context_.field_7C_ = 0;
    scene_context_.field_80_ = 0;
    scene_context_.field_84_ = 0;
    scene_context_.field_88_ = 0;
    scene_context_.field_8C_ = 0;
    scene_context_.field_90_ = 1.0f;
    scene_context_.field_94_ = 0;
    scene_context_.field_98_ = 0;
    scene_context_.field_9C_ = 1.0f;
    scene_context_.field_A0_ = 1.0f;

    camera_view_.bounds_.lead_[0] = 0;
    camera_view_.bounds_.lead_[1] = negative_distance_limit;
    camera_view_.bounds_.lead_[2] = positive_distance_limit;

    scene_context_.state_.instance_count_ = 0;
    scene_context_.state_.instances_ = &slots_[0];
    scene_context_.state_.draw_flags_ = 0;
    scene_context_.state_.instance_steps_ = &instance_steps_[0];

    scene_context_.light_vector_.values[0] = 0;
    scene_context_.light_vector_.values[1] = 0;
    scene_context_.light_vector_.values[2] = -1.0f;

    vox_normalise_vector(scene_context_.light_vector_.values, scene_context_.light_vector_.values);

    // The normalised down vector is the light: its x and y are the shadow
    // direction's in-plane pair and its z is the elevation term
    // set_shadow_vector writes over.
    scene_context_.shadow_vector_.values[0] = scene_context_.light_vector_.values[0];
    scene_context_.shadow_vector_.values[1] = scene_context_.light_vector_.values[1];

    scene_context_.state_.scene_rotation_.values[0][0] = 1.0f;
    scene_context_.state_.scene_rotation_.values[0][1] = 0;
    scene_context_.state_.scene_rotation_.values[0][2] = 0;
    scene_context_.state_.scene_rotation_.values[1][0] = 0;
    scene_context_.state_.scene_rotation_.values[1][1] = 1.0f;
    scene_context_.state_.scene_rotation_.values[1][2] = 0;
    scene_context_.state_.scene_rotation_.values[2][0] = 0;
    scene_context_.state_.scene_rotation_.values[2][1] = 0;
    scene_context_.state_.scene_rotation_.values[2][2] = 1.0f;
    scene_context_.field_78_ = 0;

    camera_view_.camera_.rotation.values[0][0] = 1.0f;
    camera_view_.camera_.rotation.values[1][1] = 1.0f;
    camera_view_.camera_.rotation.values[2][2] = 1.0f;
    camera_view_.camera_.rotation.values[0][1] = 0;
    camera_view_.camera_.rotation.values[0][2] = 0;
    camera_view_.camera_.rotation.values[1][0] = 0;
    camera_view_.camera_.rotation.values[1][2] = 0;
    camera_view_.camera_.rotation.values[2][0] = 0;
    camera_view_.camera_.rotation.values[2][1] = 0;


    scene_context_.field_A4_ = 1;
    scene_context_.shadow_vector_.values[2] = scene_context_.light_vector_.values[2];
    field_4_ |= 1;
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

// Each component is stored before the next one's input reads; retain that
// order when an output vector overlaps an input vector.
// ORIGINAL: 0x00628290 ?vox_cross_product@@YAXPBM0PAM@Z 0x00628290-0x006282D1
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original body, complete seed extent, and fresh VC6 symbol observation; recovery/tests/test_caviar_geometry.py checks original expression trees and bounded host behavior.", "Inverse snapshots cofactor inputs before stores and has no singular guard; cross preserves sequential overlapping writes; rotation uses scoped x87 intrinsics and reads all angles before stores. Numerical spill precision is not established by these structural bindings."], "reason": "Source-owned voxel geometry with typed matrix rows and original arithmetic/store ordering."}
void __cdecl vox_cross_product(const float *left, const float *right, float *out) {
    out[0] = left[1] * right[2] - right[1] * left[2];
    out[1] = right[0] * left[2] - left[0] * right[2];
    out[2] = left[0] * right[1] - right[0] * left[1];
}

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

// Chunk lengths are 32-bit file words. Keep the original wrapping subtraction
// before its signed-result test, without signed-overflow arithmetic in C++.
static int32_t vox_chunk_payload(int32_t length) {
    return (int32_t)((uint32_t)length - 8u);
}

// Failures return without releasing partially built records, as in the image.
// ORIGINAL: 0x006381F0 ?vox_load_record@@YAPAUCvrRecord@@H@Z 0x006381F0-0x00638425
// RECOVERY_BINDING: v1 {"evidence": ["Original 006381F0..00638425: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
CvrRecord *__cdecl vox_load_record(int fd) {
    CvrChunkHeader header;
    if (vox_read_exact(fd, &header, sizeof(header)) != 0) return 0;
    if (header.tag != 0x00525643) {
        vox_stage_error(vox_msg_bad_file_format);
        return 0;
    }
    uint32_t remaining = (uint32_t)header.length - 8u;
    CvrRecord *record;
    if (vox_alloc_typed(sizeof(CvrRecord), &record) != 0) return 0;
    memset(record, 0, sizeof(*record));
    while (remaining != 0) {
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return 0;
        remaining -= (uint32_t)header.length;
        if ((remaining & 0x80000000u) != 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return 0;
        }
        switch (header.tag) {
        case 0x01000000:
            if (vox_read_dword(fd, record) != 0) return 0;
            break;
        case 0x02000000:
            if (vox_read_length_field(fd, record, header.length) != 0) return 0;
            break;
        case 0x03000000:
            if (vox_read_chunks(fd, record, header.length) != 0) return 0;
            break;
        case 0x04000000:
            if (vox_read_parts(fd, record, header.length) != 0) return 0;
            break;
        case 0x0F000000: {
            vox_user_chunk_fn callback = vox_user_chunk;
            CaviarOutputMessageWindow.message_[0] = 0;
            if (callback == 0) {
                if (vox_seek_slot((unsigned long)fd, vox_chunk_payload(header.length), 1) < 0) {
                    vox_stage_error(vox_msg_bad_file_format);
                    return 0;
                }
            } else {
                if (callback(fd, record, &header) != 0) {
                    if (strlen(CaviarOutputMessageWindow.message_) == 0) vox_stage_error(vox_msg_user_chunk);
                    return 0;
                }
                // The callback may rewrite the header; remaining was already
                // decremented, but this stored payload length is a fresh read.
                record->user_length_ = vox_chunk_payload(header.length);
            }
            break;
        }
        default:
            if (vox_seek_slot((unsigned long)fd, vox_chunk_payload(header.length), 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return 0;
            }
            break;
        }
    }
    record->state_ = 3;
    return record;
}

// ORIGINAL: 0x006388C0 ?vox_read_parts@@YAHHPAUCvrRecord@@H@Z 0x006388C0-0x00638AC2
// RECOVERY_BINDING: v1 {"evidence": ["Original 006388C0..00638AC2: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_parts(int fd, CvrRecord *record, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    // The format supplies the count chunk before entries (or the final count
    // check). The original has no initialized fallback for a missing count.
    int expected_count;
    int index;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((remaining & 0x80000000u) != 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x04010000: {
            int payload = vox_chunk_payload(header.length);
            if (vox_alloc_typed((uint32_t)payload + 1u, &record->parts_name_) != 0) return -1;
            if (vox_read_exact(fd, record->parts_name_, payload) != 0) return -1;
            record->parts_name_[payload] = 0;
            break;
        }
        case 0x04020000: {
            if (vox_read_exact(fd, &expected_count, sizeof(expected_count)) != 0) return -1;
            uint32_t names_bytes = (uint32_t)expected_count * sizeof(*record->slot_table_);
            if (vox_alloc_typed(names_bytes, &record->slot_table_) != 0) return -1;
            memset(record->slot_table_, 0, names_bytes);
            uint32_t parts_bytes = (uint32_t)expected_count * sizeof(*record->parts_);
            if (vox_alloc_typed(parts_bytes, &record->parts_) != 0) return -1;
            index = 0;
            memset(record->parts_, 0, parts_bytes);
            record->part_count_ = expected_count;
            break;
        }
        case 0x04030000:
            if (vox_read_exact(fd, &record->step_count_, sizeof(record->step_count_)) != 0) return -1;
            break;
        case 0x04040000:
            if (vox_read_part(fd, record, index, header.length) != 0) return -1;
            ++index;
            break;
        case 0x04050000:
            if (vox_read_shape_patches(fd, record, header.length) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, vox_chunk_payload(header.length), 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    if (expected_count != index) {
        vox_stage_error("Number of objects mismatch");
        return -1;
    }
    return 0;
}

// ORIGINAL: 0x00638AD0 ?vox_read_part@@YAHHPAUCvrRecord@@HH@Z 0x00638AD0-0x00638CB9
// RECOVERY_BINDING: v1 {"evidence": ["Original 00638AD0..00638CB9: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_part(int fd, CvrRecord *record, int index, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((remaining & 0x80000000u) != 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x04040100: {
            int payload = vox_chunk_payload(header.length);
            if (vox_alloc_typed((uint32_t)payload + 1u, &record->slot_table_[index]) != 0) return -1;
            if (vox_read_exact(fd, record->slot_table_[index], payload) != 0) return -1;
            record->slot_table_[index][payload] = 0;
            break;
        }
        case 0x04040200:
            if (record->steps_ == 0) {
                uint32_t bytes = (uint32_t)record->part_count_ * sizeof(*record->steps_);
                if (vox_alloc_typed(bytes, &record->steps_) != 0) return -1;
                memset(record->steps_, 0, bytes);
            }
            if (vox_read_colour_chunks(fd, &record->steps_[index], header.length) != 0) return -1;
            if (record->steps_[index].lead_ == 0) {
                record->flags_ &= ~1u;
                record->steps_[index].lead_ = record->steps_[index].shape_->byte_size_;
            } else {
                record->flags_ |= 1u;
            }
            break;
        case 0x04040300:
            if (vox_read_geometry_chunks(fd, &record->parts_[index], header.length, record->step_count_) != 0) return -1;
            break;
        case 0x04040400:
            if (vox_read_shadow(fd, record, index, header.length) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, vox_chunk_payload(header.length), 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}

// ORIGINAL: 0x00638F30 ?vox_read_shadow@@YAHHPAUCvrRecord@@HH@Z 0x00638F30-0x00639086
// RECOVERY_BINDING: v1 {"evidence": ["Original 00638F30..00639086: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_shadow(int fd, CvrRecord *record, int index, int size) {
    if (record->shadows_ == 0) {
        if (vox_alloc_typed((uint32_t)record->part_count_ * sizeof(*record->shadows_), &record->shadows_) != 0) return -1;
        // Unlike the name/part/step arrays, the original does not clear this.
    }
    uint32_t remaining = (uint32_t)size - 8u;
    CvrShadow *entry = &record->shadows_[index];
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((remaining & 0x80000000u) != 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        int payload = vox_chunk_payload(header.length);
        switch (header.tag) {
        case 0x04040401:
            if (vox_read_exact(fd, entry, payload) != 0) return -1;
            break;
        case 0x04040402:
            if (vox_alloc_typed(payload, &entry->field_4_) != 0) return -1;
            if (vox_read_exact(fd, entry->field_4_, payload) != 0) return -1;
            break;
        case 0x04040403:
            if (vox_alloc_typed(payload, &entry->field_8_) != 0) return -1;
            if (vox_read_exact(fd, entry->field_8_, payload) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}

// ORIGINAL: 0x00639090 ?vox_read_shape_patches@@YAHHPAUCvrRecord@@H@Z 0x00639090-0x006391E6
// RECOVERY_BINDING: v1 {"evidence": ["Original 00639090..006391E6: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_shape_patches(int fd, CvrRecord *record, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    // The format must select an index before its first patch. The original
    // leaves the index unspecified for a patch lacking that predecessor.
    int index;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((remaining & 0x80000000u) != 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        int payload = vox_chunk_payload(header.length);
        switch (header.tag) {
        case 0x04050001:
            if (vox_read_exact(fd, &index, payload) != 0) return -1;
            break;
        case 0x04050002: {
            CvrShape *shape = record->steps_[index].shape_;
            // A 42-byte serialized prefix of a variable-size shape allocation,
            // not a declaration of a 42-byte complete object or array stride.
            unsigned char prefix[0x2A];
            if (vox_read_exact(fd, prefix, sizeof(prefix)) != 0) return -1;
            uint32_t prefix_size;
            memcpy(&prefix_size, prefix + sizeof(uint32_t), sizeof(prefix_size));
            uint32_t old_size = shape->byte_size_;
            uint32_t new_size = prefix_size + old_size;
            unsigned char *bytes = reinterpret_cast<unsigned char *>(shape);
            // The original CRT target copies backward when these ranges
            // overlap; the patch prefix need not exceed the old shape size.
            memmove(bytes + prefix_size, bytes, old_size);
            memcpy(bytes, prefix, sizeof(prefix));
            if (vox_read_exact(fd, bytes + sizeof(prefix), (uint32_t)payload - sizeof(prefix)) != 0) return -1;
            shape->byte_size_ = new_size;
            break;
        }
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}


// Caviar projection and scaled sprite owner bodies. Their shared state is
// defined above in this translation unit; original callers and formats stay
// distinct. Exact x87 exceptional behavior and indirect blitter reachability
// remain explicit restoration obligations.


/*
Purpose: The isometric view-matrix builder: take the caller's nine source
         floats (a 3x3 transform), scale each by the single factor in
         .rdata, park the scaled values in the nine-float scratch, then
         round each back into the fixed-point view state - the first six
         as 16-bit matrix words (the 2x3 matrix the tile projector at
         0x006401B0 multiplies by), the next three as 32-bit words - and
         clear the two trailing words.

// The three mapped callers supply VOX_Matrix owners at 63F611, 63E95C and
// 63F00C. Native x87 exceptional conversion behavior and complete executable
// caller/reference qualification remain explicit fidelity obligations.
// ORIGINAL: 0x0063FA80 sub_63fa80 0x0063FA80-0x0063FC92
// size      530 bytes
// Deleted seed provenance: git:1ed59304110fc2098fb1dd11457fa5b0074169bd:src/unrecovered/0063fa80.cpp
Status: Coverage transcription of the retained seed (src/unrecovered/0063fa80.cpp,
        plain FILE). The seed's two TRIED walls stand: the do/while keeps the
        original's single moving source pointer with the fixed scratch delta
        (the image threads both arrays through `eax` plus one constant
        displacement), and the four .rdata words stay bindings rather than
        literals - their bit patterns were never read, so writing 0.5 into
        the source would invent a value. The whole-image measurement stays a
        documented MISMATCH for a structural reason the seed did not have to
        face: the original is frameless (`mov eax, [esp+4]` at 0x0063FA80)
        while this TU compiles /Oy-, so the frame and spill shape differ
        from the prologue on. The words they feed are named from
        the two bodies' use of the cluster: 0x009C0A40..0x4A are the six
        matrix words 0x006401B0 multiplies by, 0x009C0A4C..0x54 the three
        int words rounded beside them, 0x009C0A7C the float scratch, and
        0x009C0AC4/0x009C0AC8 the two trailing words this body zeroes.
*/
namespace {
// 63FAB3 and each sibling compare select 671040 (-0.5) when x87 C0
// is set, otherwise 671044 (+0.5). The old address aliases had these
// two names/polarities reversed; this follows the pinned instruction edges.
int round_view_scaled(float value) {
    return (int)(value + (value >= CaviarProjectionRoundThreshold
                              ? CaviarProjectionRoundAbove
                              : CaviarProjectionRoundBelow));
}
}  // namespace

extern "C" void __cdecl sub_63fa80(const VOX_Matrix *sources) {
    const float *cursor = &sources->values[0][0];
    int delta = (int)((char *)CaviarProjectionScaledMatrix - (char *)cursor);
    int n = 9;
    do {
        *(float *)((char *)cursor + delta) = *cursor * CaviarProjectionDirectionScale;
        cursor++;
    } while (--n);

    CaviarFixedPointProjection.word_00_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[0]);
    CaviarFixedPointProjection.word_02_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[1]);
    CaviarFixedPointProjection.word_04_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[2]);
    CaviarFixedPointProjection.word_06_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[3]);
    CaviarFixedPointProjection.word_08_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[4]);
    CaviarFixedPointProjection.word_0A_ = (short)round_view_scaled(CaviarProjectionScaledMatrix[5]);
    CaviarFixedPointProjection.dword_0C_ = round_view_scaled(CaviarProjectionScaledMatrix[6]);
    CaviarFixedPointProjection.dword_10_ = round_view_scaled(CaviarProjectionScaledMatrix[7]);
    CaviarFixedPointProjection.dword_14_ = round_view_scaled(CaviarProjectionScaledMatrix[8]);

    vox_projection_render_stage_ = 0;
    vox_projection_prepared_ = 0;
}


/*
Purpose: The isometric box-projector: project the caller's centre and
         half-extent vectors through the fixed-point view matrix
         (0x009C0A40..0x4A, the words sub_63fa80 builds) and accumulate the
         eight corner combinations into an integer rect - left/top/right/
         bottom in [0..3], inset by two pixels on each side, with the width
         and height in [4..5] once the box lands on screen, or zeroes when
         it is entirely above/left of the origin.

Ownership: Global three-pointer __cdecl helper, not an invented class method.
           Original part calls 0x0063EB24/0x0063EEDB, shadow calls
           0x0063F1CA/0x0063F4C4 and clip calls 0x0063F758/0x0063F8C7
           all target this entry. Typed renderer paths supply their actual
           signed-short centres/extents and six-int result storage; the
           source-owned projection matrix supplies its six coefficients.
// ORIGINAL: 0x006401B0 sub_6401b0 0x006401B0-0x006404E2
// size      818 bytes
// Deleted seed provenance: git:1ed59304110fc2098fb1dd11457fa5b0074169bd:src/unrecovered/006401b0.cpp
Status: Coverage transcription of the retained seed (src/unrecovered/006401b0.cpp,
        plain FILE). The seed's TRIED wall stands: the round-to-fixed helper
        is spelled as the ordinary `+ 0x80 >> 8` form and the multiply/add
        scheduling still diverges early (similarity ~0.6 in the seed's own
        attempt), a compiler-scheduling divergence not chased further; this
        TU also compiles /Oy- where the original is frameless (esp-relative
        args at 0x006401B3), which separates the streams from the prologue
        on. The
        parameter reading is the seed's (three short/short/int pointers,
        Ghidra's own), the matrix words are the CaviarFixedPointProjection binding
        beside sub_63fa80, and the corner walk is the seed's incremental
        add/subtract order. Matrix row 0 lands on the rect's x axis and
        row 1 on its y axis, so the extent products are named per axis
        (extentN_x for the row-0 product, extentN_y for the row-1 one).
*/
namespace {

// Ghidra's bit-trick round, spelled as the arithmetic it performs: the
// image adds 0x80 and shifts the 16.8 fixed-point product down to a short.
static int32_t caviar_i32_from_bits(uint32_t bits) {
    if (bits <= 0x7fffffffu) return (int32_t)bits;
    return -1 - (int32_t)(0xffffffffu - bits);
}

// The pinned code adds in a 32-bit register and uses x86 arithmetic SAR.
static uint32_t caviar_sar8_bits(uint32_t bits) {
    const uint32_t biased = bits + 0x80u;
    uint32_t shifted = biased >> 8;
    if ((biased & 0x80000000u) != 0) shifted |= 0xff000000u;
    return shifted;
}

// Extent reductions are written through AX in the original. Center reductions
// use the full EAX/EDX value and must not pass through this low-word helper.
static int32_t caviar_fixed8_low16(uint32_t bits) {
    const uint16_t low = (uint16_t)caviar_sar8_bits(bits);
    return (low & 0x8000u) != 0
        ? (int32_t)low - 0x10000
        : (int32_t)low;
}

static int32_t caviar_add_i32(int32_t a, int32_t b) {
    return caviar_i32_from_bits((uint32_t)a + (uint32_t)b);
}

static int32_t caviar_sub_i32(int32_t a, int32_t b) {
    return caviar_i32_from_bits((uint32_t)a - (uint32_t)b);
}

int32_t fixed8(uint32_t value_bits) {
    return caviar_fixed8_low16(value_bits);
}

}

extern "C" void __cdecl sub_6401b0(const short *centre, const short *extent,
                                   int *out_rect) {
    uint32_t x_centre_bits =
        (uint32_t)((int32_t)CaviarFixedPointProjection.word_00_ * (int32_t)centre[0]);
    x_centre_bits += (uint32_t)((int32_t)CaviarFixedPointProjection.word_04_ * (int32_t)centre[2]);
    x_centre_bits += (uint32_t)((int32_t)CaviarFixedPointProjection.word_02_ * (int32_t)centre[1]);
    int x_walk;
    uint32_t y_centre_bits =
        (uint32_t)((int32_t)CaviarFixedPointProjection.word_06_ * (int32_t)centre[0]);
    y_centre_bits += (uint32_t)((int32_t)CaviarFixedPointProjection.word_0A_ * (int32_t)centre[2]);
    y_centre_bits += (uint32_t)((int32_t)CaviarFixedPointProjection.word_08_ * (int32_t)centre[1]);
    int y_walk;
    int extent0_x = (int)extent[0] * (int)CaviarFixedPointProjection.word_00_;
    int extent0_y = (int)extent[0] * (int)CaviarFixedPointProjection.word_06_;
    int extent1_x = (int)extent[1] * (int)CaviarFixedPointProjection.word_02_;
    int extent1_y = (int)extent[1] * (int)CaviarFixedPointProjection.word_08_;
    int extent2_x = (int)extent[2] * (int)CaviarFixedPointProjection.word_04_;
    int extent2_y = (int)extent[2] * (int)CaviarFixedPointProjection.word_0A_;

    extent0_y = fixed8((uint32_t)extent0_y);
    out_rect[4] = 0;
    out_rect[5] = 0;
    extent0_x = fixed8((uint32_t)extent0_x);
    x_walk = caviar_add_i32(caviar_i32_from_bits(caviar_sar8_bits(x_centre_bits)), extent0_x);
    y_walk = caviar_add_i32(caviar_i32_from_bits(caviar_sar8_bits(y_centre_bits)), extent0_y);
    out_rect[0] = 0x7fff;
    out_rect[1] = 0x7fff;
    out_rect[2] = -0x7fff;
    out_rect[3] = -0x7fff;
    if (x_walk < 0x7fff) out_rect[0] = x_walk;
    if (-0x7fff < x_walk) out_rect[2] = x_walk;
    if (y_walk < 0x7fff) out_rect[1] = y_walk;
    if (-0x7fff < y_walk) out_rect[3] = y_walk;
    extent1_x = fixed8((uint32_t)extent1_x);
    extent1_y = fixed8((uint32_t)extent1_y);
    x_walk = caviar_add_i32(x_walk, extent1_x);
    y_walk = caviar_add_i32(y_walk, extent1_y);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    x_walk = caviar_sub_i32(x_walk, extent0_x);
    y_walk = caviar_sub_i32(y_walk, extent0_y);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    y_walk = caviar_sub_i32(y_walk, extent1_y);
    x_walk = caviar_sub_i32(x_walk, extent1_x);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    x_walk = caviar_add_i32(x_walk, fixed8((uint32_t)extent2_x));
    y_walk = caviar_add_i32(y_walk, fixed8((uint32_t)extent2_y));
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    x_walk = caviar_add_i32(x_walk, extent0_x);
    y_walk = caviar_add_i32(y_walk, extent0_y);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    x_walk = caviar_add_i32(x_walk, extent1_x);
    y_walk = caviar_add_i32(y_walk, extent1_y);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;
    x_walk = caviar_sub_i32(x_walk, extent0_x);
    y_walk = caviar_sub_i32(y_walk, extent0_y);
    if (x_walk < out_rect[0]) out_rect[0] = x_walk;
    if (out_rect[2] < x_walk) out_rect[2] = x_walk;
    if (y_walk < out_rect[1]) out_rect[1] = y_walk;
    if (out_rect[3] < y_walk) out_rect[3] = y_walk;

    x_walk = out_rect[0];
    y_walk = out_rect[1];
    out_rect[0] = caviar_sub_i32(x_walk, 2);
    out_rect[1] = caviar_sub_i32(y_walk, 2);
    int bottom = caviar_add_i32(out_rect[3], 2);
    int right = caviar_add_i32(out_rect[2], 2);
    out_rect[3] = bottom;
    out_rect[2] = right;
    if ((-1 < right) && (-1 < bottom)) {
        out_rect[4] = (right - (x_walk - 2)) + 1;
        out_rect[5] = (bottom - (y_walk - 2)) + 1;
        return;
    }
    out_rect[4] = 0;
    out_rect[5] = 0;
}


/*
Purpose: Walk a scaled sprite into a 16-bit destination through a palette:
         for each destination pixel whose source byte is non-zero, replace
         the existing pixel with palette[current pixel].

         The destination cursor and byte stride live in the two shared
         voxel-blit slots caviar.cpp owns; the cursor is re-read from the
         slot every row, exactly as the original re-reads it, so anything
         else writing the slot between rows is seen.

// ORIGINAL: 0x0063B11C sub_63b11c 0x0063B11C-0x0063B1BF
// RECOVERY_BINDING: v1 {"evidence":["pinned:0x0063B11C-0x0063B1BF:four callee-saved pushes then plain RET; after saves stack +0x14/+0x18 are destination/source, +0x1C/+0x20 x/y, +0x24/+0x28 width/height, +0x2C/+0x30 16.16 x/y steps, +0x34/+0x38 source-byte/destination-word pitches, +0x3C palette","pinned:0x0063B11C:nonzero source-byte gate; reads destination word, indexes palette by that word, stores the result","pinned:0x0063B11C:reads/writes 0x00698B70 cursor and 0x00698B78 stride; 0x0063B072 uses the same state words","source:src/caviar.cpp:CaviarBlitStateCandidate and vox_blit_transparent own the typed shared state"],"reason":"Own this standalone global C-ABI 16-bit blitter in the Caviar renderer component based on its stack contract and shared-state effects; leave incoming selector/caller routes unresolved and claim no C++ class or complete caller domain."}
// symbol   _sub_63b11c
// size      163 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_63b11c(unsigned short *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height,
                                   int source_x_step, int source_y_step,
                                   int source_stride, int destination_stride,
                                   const unsigned short *palette) {
    if (width > 0 && height > 0) {
        CaviarBlitState.destination_stride_bytes_ =
            static_cast<uint32_t>(destination_stride) << 1;
        CaviarBlitState.destination_cursor_ = caviar_blit_address(
            reinterpret_cast<unsigned char *>(destination_base),
            caviar_blit_offset(destination_x, destination_y,
                               destination_stride) << 1);
        unsigned int carried_y = 0;
        do {
            unsigned int carried_x = 0;
            int row_remaining = width;
            unsigned short *pixel = reinterpret_cast<unsigned short *>(
                CaviarBlitState.destination_cursor_);
            do {
                if (source[carried_x >> 16] != 0) {
                    *pixel = palette[*pixel];
                }
                ++pixel;
                carried_x += static_cast<unsigned int>(source_x_step);
                --row_remaining;
            } while (row_remaining != 0);
            CaviarBlitState.destination_cursor_ = caviar_blit_address(
                CaviarBlitState.destination_cursor_,
                CaviarBlitState.destination_stride_bytes_);
            const uint32_t source_rows =
                (carried_y + static_cast<uint32_t>(source_y_step)) >> 16;
            source = caviar_blit_source_address(
                source, source_rows * static_cast<uint32_t>(source_stride));
            carried_y = (carried_y + source_y_step) & 0xffff;
            --height;
        } while (height != 0);
    }
}


/*
Purpose: Walk a scaled sprite into an 8-bit destination, skipping the one
         transparent source byte value.

         The same shared cursor slot carries the destination pointer, and
         is re-read every row like its 16-bit sibling above.

// ORIGINAL: 0x0063B1BF sub_63b1bf 0x0063B1BF-0x0063B241
// RECOVERY_BINDING: v1 {"evidence":["pinned:0x0063B1BF-0x0063B241:four callee-saved pushes then plain RET; after saves stack +0x14/+0x18 are destination/source, +0x1C/+0x20 x/y, +0x24/+0x28 width/height, +0x2C fixed-point step, +0x30/+0x34 source/destination byte pitches, +0x38 transparent byte","pinned:0x0063B1BF:loads source byte, preserves destination for transparent matches, otherwise stores the source byte","pinned:0x0063B1BF:reads/writes 0x00698B70 cursor; sibling 0x0063B072 initializes and consumes the same cursor state","source:src/caviar.cpp:CaviarBlitStateCandidate and vox_blit_transparent own the typed shared state"],"reason":"Own this standalone global C-ABI 8-bit keyed blitter in the Caviar renderer component based on its stack contract and shared-state effects; leave incoming selector/caller routes unresolved and claim no C++ class or complete caller domain."}
// symbol   _sub_63b1bf
// size      130 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_63b1bf(unsigned char *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height, int step,
                                   int source_stride, int destination_stride,
                                   char transparent) {
    if (width > 0 && height > 0) {
        CaviarBlitState.destination_cursor_ = caviar_blit_address(
            destination_base, caviar_blit_offset(destination_x, destination_y,
                                                 destination_stride));
        unsigned int carried = 0;
        do {
            unsigned int fraction = 0;
            char *pixel = reinterpret_cast<char *>(
                CaviarBlitState.destination_cursor_);
            int row_remaining = width;
            do {
                const char value = source[fraction >> 16];
                if (value != transparent) {
                    *pixel = value;
                }
                ++pixel;
                fraction += static_cast<unsigned int>(step);
                --row_remaining;
            } while (row_remaining != 0);
            CaviarBlitState.destination_cursor_ = caviar_blit_address(
                CaviarBlitState.destination_cursor_,
                static_cast<uint32_t>(destination_stride));
            const uint32_t source_rows =
                (carried + static_cast<uint32_t>(step)) >> 16;
            source = caviar_blit_source_address(
                source, source_rows * static_cast<uint32_t>(source_stride));
            carried = (carried + step) & 0xffff;
            --height;
        } while (height != 0);
    }
}


/*
Purpose: Walk a scaled sprite into an 8-bit destination through a shading
         table: each non-zero source byte replaces the destination byte
         with table[destination byte].

// ORIGINAL: 0x0063B241 sub_63b241 0x0063B241-0x0063B2CB
// RECOVERY_BINDING: v1 {"evidence":["pinned:0x0063B241-0x0063B2CB:four callee-saved pushes then plain RET; after saves stack +0x14/+0x18 are destination/source, +0x1C/+0x20 x/y, +0x24/+0x28 width/height, +0x2C/+0x30 16.16 x/y steps, +0x34/+0x38 source/destination byte pitches, +0x3C lookup table","pinned:0x0063B241:nonzero source-byte gate; reads destination byte, indexes table by that byte, stores the result","pinned:0x0063B241:reads/writes 0x00698B70 cursor; 0x0063B072 initializes and consumes the cursor and pitch words","source:src/caviar.cpp:CaviarBlitStateCandidate and vox_blit_transparent own the typed shared state"],"reason":"Own this standalone global C-ABI 8-bit table blitter in the Caviar renderer component based on its stack contract and shared-state effects; leave incoming selector/caller routes unresolved and claim no C++ class or complete caller domain."}
// symbol   _sub_63b241
// size      138 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
extern "C" void __cdecl sub_63b241(unsigned char *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height,
                                   int source_x_step, int source_y_step,
                                   int source_stride, int destination_stride,
                                   const unsigned char *table) {
    if (width > 0 && height > 0) {
        CaviarBlitState.destination_cursor_ = caviar_blit_address(
            destination_base, caviar_blit_offset(destination_x, destination_y,
                                                 destination_stride));
        unsigned int carried_y = 0;
        do {
            unsigned int carried_x = 0;
            int row_remaining = width;
            unsigned char *pixel = CaviarBlitState.destination_cursor_;
            do {
                if (source[carried_x >> 16] != 0) {
                    *pixel = table[*pixel];
                }
                ++pixel;
                carried_x += static_cast<unsigned int>(source_x_step);
                --row_remaining;
            } while (row_remaining != 0);
            CaviarBlitState.destination_cursor_ = caviar_blit_address(
                CaviarBlitState.destination_cursor_,
                static_cast<uint32_t>(destination_stride));
            const uint32_t source_rows =
                (carried_y + static_cast<uint32_t>(source_y_step)) >> 16;
            source = caviar_blit_source_address(
                source, source_rows * static_cast<uint32_t>(source_stride));
            carried_y = (carried_y + source_y_step) & 0xffff;
            --height;
        } while (height != 0);
    }
}
