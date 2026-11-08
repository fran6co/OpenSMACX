/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// ORIGINAL: 0x00616AA0 sub_616aa0 0x00616AA0-0x00616AB6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete decode establishes this exclusive span and paired startup/atexit cleanup. Original receivers 9B9108 and 9B96B0 share guard byte9B9104 with independent masks1 and2; cleanup sets its bit before direct complete Buffer destruction.", "Private lifetime storage contains one Buffer at offset zero. The lifetime wrapper is a chosen implementation, not a recovered original class name or evidence for a larger combined allocation. A then B startup implies B then A atexit cleanup; explicit Caviar close retains A then B ordering.", "Physical COFF startup and registered-cleanup references identify the corresponding source-owned Buffer storage. The adjacent symbol fact identifies this operation; the stored object has Buffer type, while CaviarSceneBufferLifetime is a chosen lifetime wrapper, not an original class-name claim. Explicit Buffer-qualified destruction avoids virtual deleting dispatch. Startup byte restoration and full linked/runtime fidelity remain obligations; this binding banks no claim."], "reason": "Bind the original Caviar scene Buffer init to its source-owned guarded C++ lifetime."}
// symbol    _$E16
// ORIGINAL: 0x00616AC0 sub_616ac0 0x00616AC0-0x00616ADF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete decode establishes this exclusive span and paired startup/atexit cleanup. Original receivers 9B9108 and 9B96B0 share guard byte9B9104 with independent masks1 and2; cleanup sets its bit before direct complete Buffer destruction.", "Private lifetime storage contains one Buffer at offset zero. The lifetime wrapper is a chosen implementation, not a recovered original class name or evidence for a larger combined allocation. A then B startup implies B then A atexit cleanup; explicit Caviar close retains A then B ordering.", "Physical COFF startup and registered-cleanup references identify the corresponding source-owned Buffer storage. The adjacent symbol fact identifies this operation; the stored object has Buffer type, while CaviarSceneBufferLifetime is a chosen lifetime wrapper, not an original class-name claim. Explicit Buffer-qualified destruction avoids virtual deleting dispatch. Startup byte restoration and full linked/runtime fidelity remain obligations; this binding banks no claim."], "reason": "Bind the original Caviar scene Buffer fini to its source-owned guarded C++ lifetime."}
// symbol    _$E14
// ORIGINAL: 0x00616AE0 sub_616ae0 0x00616AE0-0x00616AF6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete decode establishes this exclusive span and paired startup/atexit cleanup. Original receivers 9B9108 and 9B96B0 share guard byte9B9104 with independent masks1 and2; cleanup sets its bit before direct complete Buffer destruction.", "Private lifetime storage contains one Buffer at offset zero. The lifetime wrapper is a chosen implementation, not a recovered original class name or evidence for a larger combined allocation. A then B startup implies B then A atexit cleanup; explicit Caviar close retains A then B ordering.", "Physical COFF startup and registered-cleanup references identify the corresponding source-owned Buffer storage. The adjacent symbol fact identifies this operation; the stored object has Buffer type, while CaviarSceneBufferLifetime is a chosen lifetime wrapper, not an original class-name claim. Explicit Buffer-qualified destruction avoids virtual deleting dispatch. Startup byte restoration and full linked/runtime fidelity remain obligations; this binding banks no claim."], "reason": "Bind the original Caviar scene Buffer init to its source-owned guarded C++ lifetime."}
// symbol    _$E21
// ORIGINAL: 0x00616B00 sub_616b00 0x00616B00-0x00616B1F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned complete decode establishes this exclusive span and paired startup/atexit cleanup. Original receivers 9B9108 and 9B96B0 share guard byte9B9104 with independent masks1 and2; cleanup sets its bit before direct complete Buffer destruction.", "Private lifetime storage contains one Buffer at offset zero. The lifetime wrapper is a chosen implementation, not a recovered original class name or evidence for a larger combined allocation. A then B startup implies B then A atexit cleanup; explicit Caviar close retains A then B ordering.", "Physical COFF startup and registered-cleanup references identify the corresponding source-owned Buffer storage. The adjacent symbol fact identifies this operation; the stored object has Buffer type, while CaviarSceneBufferLifetime is a chosen lifetime wrapper, not an original class-name claim. Explicit Buffer-qualified destruction avoids virtual deleting dispatch. Startup byte restoration and full linked/runtime fidelity remain obligations; this binding banks no claim."], "reason": "Bind the original Caviar scene Buffer fini to its source-owned guarded C++ lifetime."}
// symbol    _$E19
#include "stdafx.h"
#include "caviar.h"
#include "caviar_io.h"
#include "caviar_cpu.h"
#include "vox_engine.h"
#include "basepop.h"
#include "buffer.h"
#include "palette.h"
#include "general.h"    // mem_get
#include "stringstruct.h"
#include "strings.h"    // StringTemp
#include "vector.h"     // vector_add - the engine's geometry accumulator
#include "table_storage.h"  // PointerTripletTable, released at 0x00616B40

#include <io.h>        // _read/_write/_lseek/_tell - the vx_* IO shims
#include <new>
#include <stdlib.h>
#include <stddef.h>


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
// RECOVERY_OBJECT: v3 0x009C0B90 0x009C0B94 ?CaviarProjectionOutputWords@@3PAIA 0x006392E0 member ?CaviarOutputMessageWindow@@3UCaviarOutputMessageWindowCandidate@@A io_seek_
CaviarKernelProjectionWindowCandidate CaviarKernelProjectionWindow = {0};
// 6392E0 copies the six callbacks to 9C0B80, the window's start; the separately
// mapped vox_clip_bottom_ at 9C0E60 bounds its 0x2E0 bytes.
// RECOVERY_OBJECT: v2 0x009C0B80 0x009C0E60 ?CaviarOutputMessageWindow@@3UCaviarOutputMessageWindowCandidate@@A 0x006392E0
CaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {0};
// RECOVERY_OBJECT: v1 0x009C0A40 ?CaviarFixedPointProjection@@3UCaviarFixedPointProjectionMatrix@@A
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {0}; // 0x009C0A40
// RECOVERY_OBJECT: v1 0x009C0A7C ?CaviarProjectionScaledMatrix@@3PAMA
float CaviarProjectionScaledMatrix[9] = {0}; // 0x009C0A7C

// Separate direction-helper cells whose consumers share the existing
// projection matrix/state rather than owning an invented scratch aggregate.
// RECOVERY_OBJECT: v1 0x009C0AA0 ?vox_projection_matrix_@@3UVOX_Matrix@@A
VOX_Matrix vox_projection_matrix_ = {0};    // 0x009C0AA0
// Provisional descriptive name for this separately mapped three-float
// output window; the pin establishes its start and accessed bytes, not a wider
// allocation, original symbol, or lifetime.
// RECOVERY_OBJECT: v1 0x009C0A64 ?CaviarProjectionDirectionCandidate@@3UVOX_Vect@@A
VOX_Vect CaviarProjectionDirectionCandidate = {0}; // 0x009C0A64 observed bytes 0x009C0A64..0x009C0A6F
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

// Engine lifetime API; the definition is owned by caviar_loader.cpp.
void vox_engine_close();

namespace {

// Historical: /O2 expands direct `memset` into `rep stosd`; render_mono used
// this helper when the intrinsic expansion differed from the pinned call.
// The selected body now scopes VC6 `#pragma function(memset)` around its
// direct call. This helper remains as an unused legacy form until its
// ownership and removal are reviewed.
void *__cdecl caviar_memset_call(void *dst, int value, unsigned count) {
    return memset(dst, value, count);
}


OPENSMACX_NOINLINE
uint32_t load_scaling_bits(const void *source) {
    uint32_t value;
    memcpy(&value, source, sizeof(value));
    return value;
}

} // namespace

// ORIGINAL: 0x00616B20 sub_616b20 0x00616B20-0x00616B2D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00616B20-0x00616B2D returns ECX in EAX after zeroing dwords +0/+4/+8", "source:src/caviar.h VoxInstance is exactly 0xC with three pointer members and RECOVERY_MEMBER anchors at +4/+8", "pinned:0x00616DA0 initializes 200 twelve-byte slot records inline at Caviar+0x74C; it does not directly call 0x00616B20"], "reason": "Own the three pointer-zeroing stores on the existing 0xC VoxInstance type. Moving its inline constructor out of the header exposes this source definition but changes the Caviar array-construction code shape; constructor return-register/ABI and all callers remain native-review obligations."}
// symbol    ??0VoxInstance@@QAE@XZ
// kind      game
VoxInstance::VoxInstance()
    : instance_colour_(0), instance_vector_(0),
      instance_matrix_(0) {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 8 bytes.
// ORIGINAL: 0x00616B30 sub_616b30 0x00616B30-0x00616B35 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00616B30-0x00616B35 zero EAX and return; callee stack cleanup 8 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x00616B20 sub_616b20 (src/caviar.cpp) and 0x00616B40 sub_616b40 (src/table_storage.cpp); caviar.cpp functions continue at 0x00616BC0"]}
// symbol    ?sub_616b30@@YGHHH@Z
// kind      game
int __stdcall sub_616b30(int, int) {
    return 0;
}

/*
Purpose: Free a table of records holding three pointers each, then the
         table itself. The original-owner, producer, and full-allocation
         extent remain unresolved; the typed view represents only the
         observed receiver and row prefixes.

         The original reloads the table before each row field and reloads the
         signed count at loop end. Keep these reads across external free calls.
         The initial null-table gate, row order, table-free/clear order, and
         unconditional count/word-8 clears are all load-bearing behavior.

// RECOVERY_ROLE: SCAFFOLD Bounded pointer-table cleanup method; original class, producer and complete storage lifetime remain unresolved.
// ORIGINAL: 0x00616B40 sub_616b40 0x00616B40-0x00616BB6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the observed table/count/word receiver as an ordinary member cleanup in the table-storage component, preserving callback-visible rereads without asserting VoxInstance, CvrPart or an original class name.","evidence":["pinned:0x00616B40-0x00616BB6 reads receiver+0 table and signed +4 count, frees three pointer positions per twelve-byte row, reloads table per field and count per iteration, then frees the final table and clears +0/+4/+8","source:PointerTripletTable models these accessed receiver words only; producer and complete original allocation remain unknown"]}
// symbol   ?release_contents@PointerTripletTable@@QAEXXZ
// size      118 bytes
// prototype unresolved; original ECX receiver with unused EDX
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void PointerTripletTable::release_contents() {
    if (table_ != 0) {
        for (int index = 0; index < count_; ++index) {
            if (table_[index].first_ != 0) {
                free(table_[index].first_);
            }
            if (table_[index].second_ != 0) {
                free(table_[index].second_);
            }
            if (table_[index].third_ != 0) {
                free(table_[index].third_);
            }
        }
        if (table_ != 0) {
            free(table_);
        }
        table_ = 0;
    }
    count_ = 0;
    field_08_ = 0;
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
// ORIGINAL: 0x00616DA0 ??0Caviar@@QAE@XZ 0x00616DA0-0x00616DD6 SEMANTIC
// LEVER: 0 of 16 -> SHAPE_EXACT 12 of 16, on two changes that are one idea. The image initializes the 200-element `VoxInstance` array through its three pointer members and does not memset `slots_`. The residual is a pointer bias in VC6's array-construction loop; see TRIED below. CaviarData is a separate loaded-record type and is not the element type of this member array.
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
    field_108_ = 0x461C4000;
    field_13CC_ = 0;
    scene_scale_ = 0.6666667f;
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

// Original pointer cell at 0x009B9100; the allocation it holds is separate.
// The colour table contains 0x10000 16-bit entries (0x20000 bytes), filled
// with 0xffff and shared with the render record. Ramp A advances 0x80 words
// per shade level; viewport clearing advances 0x100 words per scene row.
// RECOVERY_OBJECT: v1 0x009B9100 ?CaviarSceneMemory@?%src/caviar.cpp@@3PAGA
uint16_t *CaviarSceneMemory;

// 0x009B9108 and 0x009B96B0. The two 256x256 scene surfaces this function
// creates (init) and clears (fill 9 / fill 0). Their `dib_bits_` (+0x54) and
// `stride_` (+0x4A8) are what the descriptor block below reads - the old
// CaviarSceneBufferABits/CaviarSceneBufferBBits/CaviarRecordBitsSource/
// CaviarRecordOwnerSource bindings were those very fields under fixed
// addresses (0x9B915C, 0x9B9704, 0x9B95B0 = BufferA+0x4A8, 0x9B9B58 =
// BufferB+0x4A8).
// The registered exits 616AC0/616B00 test and set separate bits of this
// shared byte before directly destroying the Buffers. Ordinary implicit
// destruction would bypass that observed guard; a Buffer member would also
// be destroyed unconditionally after an early-returning owner destructor.
// RECOVERY_OBJECT: v1 0x009B9104 ?CaviarSceneBuffersCleanupGuard@?%src/caviar.cpp@@3EA
uint8_t CaviarSceneBuffersCleanupGuard;

// Private lifetime implementation, not a recovered original class name.
// Each slot owns exactly one real Buffer, starting at offset zero. The
// trivial storage prevents a second implicit teardown and adds no fields
// to Buffer or a speculative parent spanning the gap between the globals.
union CaviarSceneBufferStorage {
    char bytes[sizeof(Buffer)];
    uint32_t alignment_[sizeof(Buffer) / sizeof(uint32_t)];
};

template<unsigned char Mask> struct CaviarSceneBufferLifetime {
    CaviarSceneBufferStorage storage_;

    CaviarSceneBufferLifetime() { new (storage_.bytes) Buffer(); }
    ~CaviarSceneBufferLifetime() {
        if ((CaviarSceneBuffersCleanupGuard & Mask) != 0) return;
        CaviarSceneBuffersCleanupGuard |= Mask;
        // Qualification preserves the original direct complete destructor
        // call, rather than virtual deleting-destructor dispatch.
        buffer().Buffer::~Buffer();
    }
    Buffer &buffer() {
        return *reinterpret_cast<Buffer *>(storage_.bytes);
    }
};

static_assert(sizeof(CaviarSceneBufferLifetime<1>) == sizeof(Buffer),
              "guarded Buffer storage extent");
static_assert(sizeof(CaviarSceneBufferLifetime<2>) == sizeof(Buffer),
              "guarded Buffer storage extent");
static_assert(offsetof(CaviarSceneBufferLifetime<1>, storage_) == 0,
              "first guarded Buffer starts at its owner identity");
static_assert(offsetof(CaviarSceneBufferLifetime<2>, storage_) == 0,
              "second guarded Buffer starts at its owner identity");
// RECOVERY_OBJECT: v3 0x009B9108 ?CaviarSceneBufferA@?%src/caviar.cpp@@3VBuffer@@A member ?CaviarSceneBufferA@?%src/caviar.cpp@@3U?$CaviarSceneBufferLifetime@$00@1@A storage_.bytes
CaviarSceneBufferLifetime<1> CaviarSceneBufferA;  // 0x009B9108
// RECOVERY_OBJECT: v3 0x009B96B0 ?CaviarSceneBufferB@?%src/caviar.cpp@@3VBuffer@@A member ?CaviarSceneBufferB@?%src/caviar.cpp@@3U?$CaviarSceneBufferLifetime@$01@1@A storage_.bytes
CaviarSceneBufferLifetime<2> CaviarSceneBufferB;  // 0x009B96B0

// 0x009B9690..0x009B969C: the viewport rectangle accumulated by both render
// entry points. The image clears it in left, right, bottom, top order.
// RECOVERY_OBJECT: v1 0x009B9690 ?CaviarViewportState@?%src/caviar.cpp@@3UtagRECT@@A
RECT CaviarViewportState;

// 0x009B9C38. Passed twice to vox_create_record, which reads it as the
// colour-table source (0x1800 bytes for pixel size 1) and the shadow-table
// source (0x100 bytes) - the old "record's name" reading was the binding's,
// not the body's. render_mono (0x00617620) fills the complete 0x1800-byte
// window; object_start (0x00618DA0) copies it when renderer bit 3 is set.
// These full-width accesses bound the source backing window, whose end is
// the independently mapped CaviarScene at 0x009BB438. This is not an original
// symbol-table or allocation-provenance claim. The loader's virtual zero-fill
// supplies its initial zeros; subsequent draws deliberately replace them.
// RECOVERY_OBJECT: v2 0x009B9C38 0x009BB438 ?CaviarColourSource@?%src/caviar.cpp@@3PAEA 0x00617620
uint8_t CaviarColourSource[0x1800];

// The CPU rank at original0x009C0830 is owned by caviar_cpu.cpp. Preferences
// can force its fallback rank; detection and renderer selection share it.

// ---------------------------------------------------------------------------
// THE ENGINE'S PER-DRAW SCRATCH. Every one of these is written at the top of
// the engine's own draw (vox_draw_scene, homed at the foot of this file) from
// the render record and the descriptor block it is handed, and read back by
// that draw and by the helpers it calls. Their image addresses are named on
// each; the rebuilt program gives them its own, which is what every other
// global in this file does too.
// ---------------------------------------------------------------------------


} // namespace

// THE SELF-MODIFYING RENDERERS' SHARED STATE. The `_SELFMOD` MASM modules
// (src/caviar_part_*.asm, src/caviar_shadow_*.asm) read and patch through
// these, so they have external linkage: an assembler cannot name a symbol in
// an anonymous namespace.

// The draw currently accesses the 24-byte x86 window at 0x009BE6A4
// (CaviarDrawSetupPointerStorageCandidate, src/caviar/vox_engine.h).
// RECOVERY_OBJECT: v1 0x009BE6A4 ?vox_setup_copy_@@3UCaviarDrawSetupPointerStorageCandidate@@A
CaviarDrawSetupPointerStorageCandidate vox_setup_copy_;

// 0x009BE6BC. The render record's colour table, copied to where the helpers
// read it. 0x639EB2 stores it; every _SELFMOD patcher entry (0x6646FC,
// 0x664D52, ...) loads it first.
// RECOVERY_OBJECT: v1 0x009BE6BC ?vox_active_colour_table_@@3PAXA
void *vox_active_colour_table_ = 0;  // 0x009BE6BC

// 0x009C09F8. setup_block[0] minus setup_block[4] - the same two dwords the
// clip rectangle the draw starts from is built out of.

// 0x009C0A18, 0x009C082C, 0x009C0A1C. The record's two ramp tables and its
// pixel-size code, copied to where the per-part walkers read them.
uint16_t **vox_active_ramp_a_ = 0; // 0x009C0A18
// 0x639EC3 stores ramp B; the shadow patchers (0x666F69, 0x66731D) and the
// part patchers (0x664715, ...) read it.
// RECOVERY_OBJECT: v1 0x009C082C ?vox_active_ramp_b_@@3PBHB
const int32_t *vox_active_ramp_b_ = 0; // 0x009C082C
uint8_t vox_active_size_code_ = 0;  // 0x009C0A1C

// The clipped renderers' bounds, written at each clipped entry from its
// rect argument [left, top, right, bottom] and read only inside _SELFMOD.
// 0x009C0828: rect.right * 2, the right bound of the two-pixel renderers
// (0x665CD2 and 0x6673B2 `shl eax,1; mov [0x9C0828],eax`; compared at
// 0x665E8B). The one-pixel renderers keep rect.right at 0x009C0A10.
// RECOVERY_OBJECT: v1 0x009C0828 ?vox_clip_right_doubled_@@3HA
int32_t vox_clip_right_doubled_ = 0;
// 0x009C0E60: rect.bottom (0x664031 `mov eax,[edi+0Ch]; mov [0x9C0E60],eax`).
// RECOVERY_OBJECT: v1 0x009C0E60 ?vox_clip_bottom_@@3HA
int32_t vox_clip_bottom_ = 0;
// 0x009C0E64: rect.left - 1 (0x66400E..0x664016 `mov eax,[edi]; ...;
// dec eax; mov [0x9C0E64],eax`).
// RECOVERY_OBJECT: v1 0x009C0E64 ?vox_clip_left_minus_one_@@3HA
int32_t vox_clip_left_minus_one_ = 0;

namespace {


// The scene render record this function fills and hands to vox_create_record;
// the created handle lands at the tail (+0x40, 0x009BB478).
struct CaviarSceneRecord {
  RECT scene_rect_;        // 0x9BB438, accumulated scene bounds
  CaviarSceneSetupDescriptorCandidate setup_; // 0x9BB448, borrowed by the record
#if defined(_M_IX86) || defined(__i386__)
  uint8_t setup_to_renderer_gap_[4]; // 0x9BB46C..0x9BB46F
#endif
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
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(CaviarSceneRecord, setup_) == 0x10,
              "the borrowed setup view begins at scene+0x10");
static_assert(offsetof(CaviarSceneRecord, renderer_) == 0x38,
              "renderer storage remains at scene+0x38");
static_assert(offsetof(CaviarSceneRecord, created_) == 0x40,
              "created record remains at scene+0x40");
#endif

// RECOVERY_OBJECT: v1 0x009BB438 ?CaviarScene@?%src/caviar.cpp@@3UCaviarSceneRecord@1@A
CaviarSceneRecord CaviarScene;


} // namespace

// Narrow Caviar-owned reads preserve the original live Buffer field
// value without acquiring/releasing pixels or exposing Buffer layout.
void *Caviar::render_color_dib_bits() {
    return CaviarSceneBufferA.buffer().dib_bits_;
}

void *Caviar::render_shadow_dib_bits() {
    return CaviarSceneBufferB.buffer().dib_bits_;
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

    if (CaviarSceneBufferA.buffer().init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }
    if (CaviarSceneBufferB.buffer().init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }

    CaviarSceneMemory = static_cast<uint16_t *>(mem_get(0x10000 * sizeof(*CaviarSceneMemory)));
    if (CaviarSceneMemory == 0) {
        return 4;
    }

    CaviarSceneBufferA.buffer().fill(9);
    CaviarSceneBufferB.buffer().fill(0);
    vox_fill_colour_table(CaviarSceneMemory, 0xffff, 0x10000);

    // The descriptor block, in the image's store order.
    CaviarScene.setup_.depth_buffer_ = CaviarSceneMemory;
    CaviarScene.setup_.colour_row_step_ = CaviarSceneBufferA.buffer().stride_;
    CaviarScene.setup_.colour_buffer_ = CaviarSceneBufferA.buffer().dib_bits_;
    CaviarScene.setup_.colour_width_ = 0x100;
    CaviarScene.setup_.height_ = 0x100;
    CaviarScene.setup_.pixel_size_ = 1;
    CaviarScene.setup_.shadow_buffer_ = CaviarSceneBufferB.buffer().dib_bits_;
    CaviarScene.setup_.shadow_row_step_ = CaviarSceneBufferB.buffer().stride_;
    CaviarScene.setup_.depth_row_step_ = 0x100;

    CaviarScene.created_ = vox_create_record(
        0, CaviarColourSource, CaviarColourSource,
        &CaviarScene.setup_, 0x80);
    if (CaviarScene.created_ == 0) {
        return 0x17;
    }

    VoxRenderRecord *const record = CaviarScene.created_;
    record->draw_left_ = 0;
    record->draw_top_ = 0;
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
    uint16_t **const ramp_a = record->ramp_a_ptr_;
    int32_t *const ramp_b = record->ramp_b_ptr_;
    uint16_t *shade_row = CaviarSceneMemory;
    for (int index = 0; index < 0x100; ++index) {
        ramp_a[index] = shade_row;
        shade_row += 0x80;
        ramp_b[index] = index * 0x100;
    }

    CaviarViewportState.left = 0;
    CaviarViewportState.right = 0;
    CaviarViewportState.bottom = 0;
    CaviarViewportState.top = 0;
    CaviarScene.scene_rect_.left = 0;
    CaviarScene.scene_rect_.right = 0;
    CaviarScene.scene_rect_.bottom = 0;
    CaviarScene.scene_rect_.top = 0;
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
// ORIGINAL: 0x00618E10 ?vx_malloc@Caviar@@SAXK@Z 0x00618E10-0x00618E1E BYTE_EXACT
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
// ORIGINAL: 0x00618E20 ?vx_free@Caviar@@SAXPAX@Z 0x00618E20-0x00618E30 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Include the release callback's final return in its original span; retain the existing source implementation and historical findings without granting a match claim.","evidence":["pinned:0x00618E26 branches to the return at0x00618E2F; the return ends0x00618E30, which is the next mapped callback entry","source:Caviar::vx_free preserves the null test and ordinary CRT free call"]}
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
// ORIGINAL: 0x00618E90 ?vx_tell@Caviar@@SAXH@Z 0x00618E90-0x00618E9E BYTE_EXACT
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


// The record's clip bounds as the engine hands them to its callers, .data
// 0x009B96A0..0x009B96AC. Named 2026-09-04 from the two bodies that fill it:
// all four come off the created VoxRenderRecord's own clip fields
// (+0x8/+0xc/+0x10/+0x14), the min edges incremented by one.
// RECOVERY_OBJECT: v1 0x009B96A0 ?vox_clip_rect@@3PAIA
uint32_t vox_clip_rect[4];     // 0x009B96A0













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
// ORIGINAL: 0x00617620 ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z 0x00617620-0x0061766F BYTE_EXACT
// symbol    ?render_mono@Caviar@@QAEHPAVBuffer@@HHHH@Z
//           The catalogued input spelling uses struct Buffer (`U`), while
//           this tree declares class Buffer and emits `PAVBuffer`. The pinned
//           PE has no symbol record to establish original spelling; retain
//           the original label as historical mapping metadata. Changing the
//           Buffer key would re-key other owners, outside this function's
//           scope.
// size      79 bytes
// prototype int (__thiscall ?render_mono@Caviar@@QAEHPAVBuffer@@HHHH@Z)(Caviar* this, Buffer*, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006465F0 0x00617230
//
// Return type is inferred from the preserved render result in EAX; the pinned PE has no symbol/debug record for this method. The original decorated label above remains a catalogued identity, not return-type proof.
// The colour source it fills is this file's CaviarColourSource (0x009B9C38),
// the same 0x1800 bytes init_class hands vox_create_record as the record's
// colour table - so the "monochrome" draw is the ordinary draw with every
// palette entry forced to one value.
// LEVER: scoped VC6 `#pragma function(memset)` keeps the direct call; forwarding
//        render's int result keeps EAX live through the dword clear in ECX.
//        The earlier helper expanded to `rep stosd` and produced 32 instructions.
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
Return Value: the result returned by Caviar::render.
Status: Complete
*/
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(memset)
#endif
int Caviar::render_mono(Buffer *buffer, int centre_x, int centre_y,
                         int colour, int flags) {
    memset(CaviarColourSource, colour & 0xff, 0x1800);
    field_4_ |= 8;
    int result = render(buffer, centre_x, centre_y, flags);
    field_4_ &= ~0x08;
    return result;
}
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma intrinsic(memset)
#endif

/*
Purpose: Append every part of one loaded .cvr definition to the renderer's
         instance table, at the caller's rotation step. Each part contributes
         one colour byte, one accumulated vector and one matrix, copied out of
         the part's three parallel arrays at `step % step_count`, and the
         instance's palette and step entry come off the record.
// ORIGINAL: 0x00617920 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUVOX_Vect@@H@Z 0x00617920-0x00617A9F BYTE_EXACT
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
    step %= record->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = definition->record_->parts_[index].colours_[step];
        vector_add(definition->record_->parts_[index].vectors_[step].values, offset->values,
                   slots_[field_13CC_].instance_vector_->values);
        *slots_[field_13CC_].instance_matrix_ = definition->record_->parts_[index].matrices_[step];

        instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table using a
         second definition as the geometry source. The count and the step entry
         come off the first record, the vectors and the rotation step off the
         second; nothing writes the instance's colour or matrix.
// ORIGINAL: 0x006176E0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@PAUVOX_Vect@@H@Z 0x006176E0-0x006177FE BYTE_EXACT
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
    step %= geometry->record_->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        vector_add(geometry->record_->parts_[index].vectors_[step].values,
                   offset->values, slots_[field_13CC_].instance_vector_->values);
        instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table using a
         second definition as the geometry source and a raw object pointer as
         the instance's id. The count and the step entry come off the first
         record, the vectors and the rotation step off the second.
// ORIGINAL: 0x00617800 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@PAUVOX_Vect@@@Z 0x00617800-0x00617916 BYTE_EXACT
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
    step %= geometry->record_->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        vector_add(geometry->record_->parts_[index].vectors_[step].values,
                   offset->values, slots_[field_13CC_].instance_vector_->values);
        instance_palettes_[field_13CC_] = object;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, accumulated vector, matrix - with a raw object pointer as the
         instance's id.
// ORIGINAL: 0x00617AA0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUVOX_Vect@@H@Z 0x00617AA0-0x00617C2E BYTE_EXACT
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
    if (definition == 0 || offset == 0 || object == 0) {
        return 0x10;
    }
    CvrRecord *record = definition->record_;
    if (record == 0) {
        return 7;
    }
    int count = record->part_count_;
    step %= record->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = definition->record_->parts_[index].colours_[step];
        vector_add(definition->record_->parts_[index].vectors_[step].values, offset->values,
                   slots_[field_13CC_].instance_vector_->values);
        *slots_[field_13CC_].instance_matrix_ = definition->record_->parts_[index].matrices_[step];
        instance_palettes_[field_13CC_] = object;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - using a second definition as the geometry
         source. The count, the palette and the step entry come off the first
         record, the part arrays and the rotation step off the second.
// ORIGINAL: 0x00617C30 ?add_object@Caviar@@QAEHPAUCaviarData@@PAUCaviarData@@H@Z 0x00617C30-0x00617DAE BYTE_EXACT
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
    step %= geometry->record_->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = geometry->record_->parts_[index].colours_[step];
        *slots_[field_13CC_].instance_vector_ = geometry->record_->parts_[index].vectors_[step];
        *slots_[field_13CC_].instance_matrix_ = geometry->record_->parts_[index].matrices_[step];
        instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - using a second definition as the geometry
         source and a raw object pointer as the instance's id. The count and
         the step entry come off the first record, the part arrays and the
         rotation step off the second.
// ORIGINAL: 0x00617DB0 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEPAUCaviarData@@H@Z 0x00617DB0-0x00617F1F BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Map the complete instance-append body including its null-argument epilogue, not only its success/full/missing-record exits; no new implementation or match claim.","evidence":["pinned:the null-argument branches enter0x00617F12; that epilogue returns0x10 and ends after ret16 at0x00617F1F; the following byte is alignment padding before mapped0x00617F20","source:Caviar::add_object already returns0x10 for either null definition"]}
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@PAE0H@Z
//           (VC6 back-references the repeated CaviarData to `0`.)
// size      367 bytes
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
    step %= geometry->record_->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = geometry->record_->parts_[index].colours_[step];
        *slots_[field_13CC_].instance_vector_ = geometry->record_->parts_[index].vectors_[step];
        *slots_[field_13CC_].instance_matrix_ = geometry->record_->parts_[index].matrices_[step];
        instance_palettes_[field_13CC_] = object;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - the whole record the instance comes from
         being the caller's first argument.
// ORIGINAL: 0x00617F20 ?add_object@Caviar@@QAEHPAUCaviarData@@H@Z 0x00617F20-0x00618079 BYTE_EXACT
// symbol    ?add_object@Caviar@@QAEHPAUCaviarData@@H@Z
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
    step %= record->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = definition->record_->parts_[index].colours_[step];
        *slots_[field_13CC_].instance_vector_ = definition->record_->parts_[index].vectors_[step];
        *slots_[field_13CC_].instance_matrix_ = definition->record_->parts_[index].matrices_[step];
        instance_palettes_[field_13CC_] = definition->record_->palette_ + 0x302;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
    }
    return 0;
}

/*
Purpose: Append one .cvr definition's parts to the instance table in full -
         colour, vector, matrix - with a raw object pointer as the instance's
         palette source and the whole record taken from the definition.
// ORIGINAL: 0x00618080 ?add_object@Caviar@@QAEHPAUCaviarData@@PAEH@Z 0x00618080-0x006181D0 BYTE_EXACT
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
    step %= record->step_count_;
    for (int index = 0; index < count; ++index) {
        if (field_13CC_ == 200) {
            return 1;
        }
        *slots_[field_13CC_].instance_colour_ = definition->record_->parts_[index].colours_[step];
        *slots_[field_13CC_].instance_vector_ = definition->record_->parts_[index].vectors_[step];
        *slots_[field_13CC_].instance_matrix_ = definition->record_->parts_[index].matrices_[step];
        instance_palettes_[field_13CC_] = object;
        instance_steps_[field_13CC_] = definition->record_->steps_[index];
        ++field_13CC_;
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
// -byte table (CaviarSceneMemory, 0x009B9100), the two scene surfaces
// (CaviarSceneBufferA / CaviarSceneBufferB), the viewport rectangle
// (CaviarViewportState, 0x009B9690 with left, top, right, bottom coordinates) and
// the scene record CaviarScene. The rows it fills are the table's own 0x200
// -byte rows, so the walk is `row_span` rows of `col_span` 16-bit entries
// starting at the viewport's top-left corner.
Return Value: n/a
Status: Complete
*/
void Caviar::clear_buffers(int fill) {
    if (CaviarSceneMemory != 0) {
        int height = CaviarViewportState.top;
        int width = CaviarViewportState.left;
        int bottom = CaviarViewportState.bottom;
        int row_span = bottom - height;
        uint16_t *row = CaviarSceneMemory +
                        height * 0x100 + width;
        int col_span = CaviarViewportState.right - width;
        CaviarSceneBufferA.buffer().fill(width, height, col_span, row_span, 9);
        CaviarSceneBufferB.buffer().fill(0);
        if (fill == 0) {
            if (row_span > 0) {
                int count = row_span;
                do {
                    vox_fill_colour_table(row, 0xffff, col_span);
                    row += 0x100;
                    --count;
                } while (count != 0);
            }
            CaviarViewportState.left = 0;
            CaviarViewportState.right = 0;
            CaviarViewportState.bottom = 0;
            CaviarViewportState.top = 0;
            CaviarScene.scene_rect_.left = 0;
            CaviarScene.scene_rect_.right = 0;
            CaviarScene.scene_rect_.bottom = 0;
            CaviarScene.scene_rect_.top = 0;
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
// ORIGINAL: 0x006171A0 ?render_multi_table_dest@Caviar@@QAEXPAUBuffer@@HHHPAEH@Z 0x006171A0-0x00617226 BYTE_EXACT
// symbol    ?render_multi_table_dest@Caviar@@QAEHPAVBuffer@@HHHPAY0BAA@EH@Z
//           (this tree spells Buffer a `class` and the image's own name spells
//           it `U`, a struct - see the note on render_mono above.)
// size      134 bytes
// prototype int (__thiscall ?render_multi_table_dest@Caviar@@QAEHPAVBuffer@@HHHPAY0BAA@EH@Z)(Caviar* this, Buffer*, int, int, int, ColorTable*, int)
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
Inferred return contract: the original exits with the renderer's nonzero
status on failure and explicitly clears EAX after success. This C++ return type
follows that observed dataflow; it is not a pinned source declaration.
Return Value: render status on failure; zero on success
Status: Complete
*/
int Caviar::render_multi_table_dest(Buffer *buffer, int centre_x, int centre_y,
                                    int fill, ColorTable *bits, int flags) {
    CaviarSceneBufferA.buffer().fill(fill);
    int result = render(buffer, centre_x, centre_y, flags | 8);
    if (result != 0) return result;
    CaviarSceneBufferA.buffer().draw_multi_table_dest(buffer, fill, centre_x - 0x80,
                                             centre_y - 0x80, 1, 1, bits);
    buffer->free_data(1);
    CaviarSceneBufferA.buffer().fill(9);
    clear_buffers(flags & 1);
    return 0;
}

void __cdecl vox_release_record(VoxRenderRecord *record);

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
        CaviarSceneBufferA.buffer().close();
        CaviarSceneBufferB.buffer().close();
        if (CaviarSceneMemory != 0) {
            free(CaviarSceneMemory);
            CaviarSceneMemory = 0;
        }
        CaviarScene.created_->setup_block_->colour_buffer_ = 0;
        CaviarScene.created_->setup_block_->depth_buffer_ = 0;
        CaviarScene.created_->setup_block_->shadow_buffer_ = 0;
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



/*
Purpose: Draw the scene. Resets the six channel bounds to their defaults,
         records the Buffer, the instance count and the draw's flags, hands the
         render record, the viewport state and the camera to the engine's draw
         with this renderer published as the callback's object, then unions the
         clip rectangle the draw produced into the viewport and clears the
         buffers the caller asks for.
// ORIGINAL: 0x00617080 ?UNK1@Caviar@@QAEHHH@Z 0x00617080-0x0061719E BYTE_EXACT
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
// kept - which is why it lands in scene_context_.shadow_enabled_, the byte init seeds with 1.
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
    scene_context_.projection_offset_x_ = 0;
    scene_context_.projection_offset_y_ = 0;
    scene_context_.projection_offset_z_ = 0;
    if (flags & 2) scene_context_.shadow_enabled_ = 0;
    else scene_context_.shadow_enabled_ = 1;
    CaviarScene.renderer_ = this;
    vox_draw_scene(CaviarScene.created_, &scene_context_, &camera_view_, 0);
    CaviarScene.renderer_ = 0;
    RECT drawn;
    drawn.left = CaviarScene.created_->primary_clip_.left_;
    drawn.top = CaviarScene.created_->primary_clip_.top_;
    drawn.right = CaviarScene.created_->primary_clip_.right_ + 1;
    drawn.bottom = CaviarScene.created_->primary_clip_.bottom_ + 1;
    UnionRect(&CaviarViewportState, &drawn, &CaviarViewportState);
    clear_buffers(flags & 1);
    return 0;
}


/*
Purpose: The renderer's own draw. Rejects a draw with the renderer switched
         off or no instances, doubles the camera scaling when bit 5 asks for
         it, resets the channel bounds, clips the 0x100-deep box the caller
         centred on the scene surface, hands the rectangle to the render
         record, transforms the camera position by the scene rotation, draws
         through the engine, unions what was covered into the viewport and the
         scene rectangle, blits the colour table the draw filled, and clears.
// ORIGINAL: 0x00617230 ?render@Caviar@@QAEHPAUBuffer@@HHH@Z 0x00617230-0x00617620 BYTE_EXACT
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
    int top;
    int left;
    VOX_Vect extent;
    VOX_Vect position;
    RECT area;
    RECT clip;
    RECT scene;
    VOX_Matrix saved;
    VoxRenderRecord *record;

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

    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    camera_view_.bounds_.values_[4] = 0x100;
    camera_view_.bounds_.values_[5] = 0x100;
    // DEFECT FIXED 2026-09-06: the image stores the instance count here
    // (0x617292), not the Buffer.
    scene_context_.state_.instance_count_ = field_13CC_;
    buffer_ = buffer;
    scene_context_.state_.draw_flags_ = 1;
    if (flags & 2) {
        scene_context_.shadow_enabled_ = 0;
    } else {
        scene_context_.shadow_enabled_ = 1;
    }

    clip = buffer->rect1_;
    top = centre_y - 0x80;
    left = centre_x - 0x80;
    area.left = left;
    area.top = top;
    area.right = left + 0x100;
    area.bottom = top + 0x100;
    IntersectRect(&area, &area, &clip);
    int dy = 0x80 - centre_y;
    int dx = 0x80 - centre_x;
    area.left += dx;
    area.right += dx;
    area.top += dy;
    area.bottom += dy;

    // The record's fields are re-read through the global at every store.
    CaviarScene.created_->draw_left_ = area.left;
    CaviarScene.created_->draw_top_ = area.top;
    CaviarScene.created_->draw_right_ = area.right;
    CaviarScene.created_->draw_bottom_ = area.bottom;
    CaviarScene.created_->draw_width_ = area.right - area.left;
    CaviarScene.created_->draw_height_ = area.bottom - area.top;

    if (flags & 4) {
        saved = scene_context_.state_.scene_rotation_;
        // Unrotated centering is temporary: the transform always samples step
        // zero, then the saved scene rotation is restored before scene drawing.
        if (flags & 0x10) {
            scene_context_.state_.scene_rotation_.values[0][0] = 1.0f;
            scene_context_.state_.scene_rotation_.values[0][1] = 0.0f;
            scene_context_.state_.scene_rotation_.values[0][2] = 0.0f;
            scene_context_.state_.scene_rotation_.values[1][0] = 0.0f;
            scene_context_.state_.scene_rotation_.values[1][1] = 1.0f;
            scene_context_.state_.scene_rotation_.values[1][2] = 0.0f;
            scene_context_.state_.scene_rotation_.values[2][0] = 0.0f;
            scene_context_.state_.scene_rotation_.values[2][1] = 0.0f;
            scene_context_.state_.scene_rotation_.values[2][2] = 1.0f;
        }
        vox_transform_camera(&scene_context_.state_, 0,
                             &position, &extent);
        scene_context_.state_.scene_rotation_ = saved;
        position.values[0] = (position.values[0] + extent.values[0]) * 0.5f;
        position.values[1] = (position.values[1] + extent.values[1]) * 0.5f;
        position.values[2] = (position.values[2] + extent.values[2]) * 0.5f;
        scene_context_.state_.scene_offset_.values[0] -= position.values[0];
        scene_context_.state_.scene_offset_.values[1] -= position.values[1];
        scene_context_.state_.scene_offset_.values[2] -= position.values[2];
    }

    CaviarScene.renderer_ = this;
    vox_draw_scene(CaviarScene.created_, &scene_context_, &camera_view_, 0);
    record = CaviarScene.created_;
    CaviarScene.renderer_ = 0;

    area.left = record->primary_clip_.left_;
    area.top = record->primary_clip_.top_;
    area.right = record->primary_clip_.right_ + 1;
    area.bottom = record->primary_clip_.bottom_ + 1;
    UnionRect(&CaviarViewportState, &area, &CaviarViewportState);

    if (scene_context_.shadow_enabled_ != 0) {
        record = CaviarScene.created_;
        scene.left = record->secondary_clip_.left_;
        scene.top = record->secondary_clip_.top_;
        scene.right = record->secondary_clip_.right_ + 1;
        scene.bottom = record->secondary_clip_.bottom_ + 1;
        UnionRect(&CaviarScene.scene_rect_, &scene,
                  &CaviarScene.scene_rect_);
        if (scene_context_.shadow_enabled_ != 0) {
            if (field_4_ & 0x20) {
                CaviarSceneBufferB.buffer().draw_dest_source(
                    buffer, 0, centre_x - 0x40, centre_y - 0x40, 1, 2,
                    static_cast<unsigned char *>(CaviarScene.created_->shadow_table_));
            } else {
                CaviarSceneBufferB.buffer().draw_dest_source(
                    buffer, 0, left, top, 1, 1,
                    static_cast<unsigned char *>(CaviarScene.created_->shadow_table_));
            }
        }
    }

    buffer->free_data(1);

    if ((flags & 8) == 0) {
        if (field_4_ & 0x20) {
            CaviarSceneBufferA.buffer().draw(buffer, 9, centre_x - 0x40, centre_y - 0x40, 1, 2);
        } else {
            CaviarSceneBufferA.buffer().draw(buffer, 9, left, top, 1, 1);
        }
        buffer->free_data(1);
        clear_buffers(flags & 1);
    }

    return 0;
}


/*
Purpose: Fill the engine's clip rectangle from the created render record and
         return its address, or null when no record has been created.
// ORIGINAL: 0x00618520 ?UNK13@Caviar@@QAEXXZ 0x00618520-0x00618556 BYTE_EXACT
// symbol    ?clip_rect@Caviar@@QAEPAIXZ
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
uint32_t *Caviar::clip_rect() {
    VoxRenderRecord *record = CaviarScene.created_;
    if (record == 0) {
        return 0;
    }
    vox_clip_rect[0] = record->primary_clip_.left_;
    vox_clip_rect[2] = record->primary_clip_.right_ + 1;
    vox_clip_rect[1] = record->primary_clip_.top_;
    vox_clip_rect[3] = record->primary_clip_.bottom_ + 1;
    return vox_clip_rect;
}

/*
Purpose: Copy the created render record's clip bounds into a caller's array:
         {max_x, max_y, min_x + 1, min_y + 1}. A null array or a null record
         leaves the caller's memory alone.
// ORIGINAL: 0x00618560 ?UNK14@Caviar@@QAEXH@Z 0x00618560-0x006185A0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:the RET 4 instruction begins at 0x61859D and ends at 0x6185A0; the former exclusive bound cut that instruction."], "reason": "Retain the complete original rectangle operation, including all bytes of its callee-pop return."}
// symbol    ?fill_clip_rect@Caviar@@QAEXPAI@Z
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
void Caviar::fill_clip_rect(uint32_t *destination) {
    uint32_t *out = destination;
    if (out != 0) {
        if (CaviarScene.created_ != 0) {
            out[0] = CaviarScene.created_->primary_clip_.left_;
            out[2] = CaviarScene.created_->primary_clip_.right_ + 1;
            out[1] = CaviarScene.created_->primary_clip_.top_;
            out[3] = CaviarScene.created_->primary_clip_.bottom_ + 1;
        }
    }
}



/*
Purpose: Point the renderer at a Buffer and reset the six channel bounds to
         their defaults {0, 0, 0xff, 0xff, 0x100, 0x100}. 7 means the
         renderer is not switched on (field_4_'s bit 0 is clear), 0x10 means
         the caller passed no Buffer.
// ORIGINAL: 0x006183D0 ?set_buffer@Caviar@@QAEHPAUBuffer@@@Z 0x006183D0-0x00618426 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:the RET 4 instruction begins at 0x618423 and ends at 0x618426; the former exclusive bound cut that instruction."], "reason": "Retain the complete original set_buffer operation, including all bytes of its callee-pop return."}
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
    // VC6 schedules the Buffer store between the 0xff stores when it is
    // written after the first 0x100 bound.
    camera_view_.bounds_.values_[0] = 0;
    camera_view_.bounds_.values_[1] = 0;
    camera_view_.bounds_.values_[2] = 0xff;
    camera_view_.bounds_.values_[3] = 0xff;
    camera_view_.bounds_.values_[4] = 0x100;
    buffer_ = buffer;
    camera_view_.bounds_.values_[5] = 0x100;
    return 0;
}

/*
Purpose: Set the shadow direction from an elevation and an azimuth:
         {cos(azimuth)*sin(elevation),
         sin(azimuth)*sin(elevation), cos(elevation)}, the vector a shadow
         falls along for a light at those angles.
// ORIGINAL: 0x006181D0 ?set_shadow_vector@Caviar@@QAEXMM@Z 0x006181D0-0x0061824B FILE
// symbol    ?set_shadow_vector@Caviar@@QAEXMM@Z
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
Purpose: Build the renderer's per-instance table and seed the scene state.
         Allocates the three destinations each of the 200 instances draws
         through - one colour byte, one vector, one matrix - seeds the
         rotation, the clipping defaults and the channel bounds, and marks the
         renderer switched on.
// ORIGINAL: 0x00616DE0 ?init@Caviar@@QAEXXZ 0x00616DE0-0x00617013
// symbol    ?init@Caviar@@QAEHXZ
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
int Caviar::init() {
    close();

    for (int index = 0; index < 200; ++index) {
        VoxInstance &instance = slots_[index];
        instance.instance_colour_ = static_cast<uint8_t *>(mem_get(1));
        if (instance.instance_colour_ == 0) {
            close();
            return 4;
        }
        instance.instance_vector_ =
            static_cast<VOX_Vect *>(mem_get(0xC));
        if (instance.instance_vector_ == 0) {
            close();
            return 4;
        }
        instance.instance_matrix_ =
            static_cast<VOX_Matrix *>(mem_get(0x24));
        if (instance.instance_matrix_ == 0) {
            close();
            return 4;
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

    scene_context_.projection_offset_x_ = 0;
    scene_context_.projection_offset_y_ = 0;
    scene_context_.projection_offset_z_ = 0;
    scene_context_.screen_offset_x_ = 0;
    scene_context_.screen_offset_y_ = 0;
    scene_context_.projection_scale_ = 1.0f;
    scene_context_.shadow_screen_offset_x_ = 0;
    scene_context_.shadow_screen_offset_y_ = 0;
    scene_context_.shadow_scale_x_ = 1.0f;
    scene_context_.shadow_scale_y_ = 1.0f;

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
    scene_context_.shadow_origin_adjustment_ = 0;

    camera_view_.camera_.rotation.values[0][0] = 1.0f;
    camera_view_.camera_.rotation.values[1][1] = 1.0f;
    camera_view_.camera_.rotation.values[2][2] = 1.0f;
    camera_view_.camera_.rotation.values[0][1] = 0;
    camera_view_.camera_.rotation.values[0][2] = 0;
    camera_view_.camera_.rotation.values[1][0] = 0;
    camera_view_.camera_.rotation.values[1][2] = 0;
    camera_view_.camera_.rotation.values[2][0] = 0;
    camera_view_.camera_.rotation.values[2][1] = 0;


    scene_context_.shadow_enabled_ = 1;
    scene_context_.shadow_vector_.values[2] = scene_context_.light_vector_.values[2];
    field_4_ |= 1;
    return 0;
}





/*
Purpose: Install a shadow table into the created render record: the record's
         own 0x54 slot names the 0x100-byte block, and the caller's bytes are
         copied over it. 7 means no record, 0x10 means the record has no
         shadow table.
// ORIGINAL: 0x00618430 ?set_shadow_table@Caviar@@QAEHPAE@Z 0x00618430-0x00618468 BYTE_EXACT
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
Purpose: The engine's per-object callback. Fills 0x1800 bytes of the handle's
         colour table from wherever field_4_'s bits say the next draw reads:
         the shared monochrome source when bit 3 is on, the table the caller
         of render_with_colour_table handed over when bit 4 is on, otherwise
         this renderer's own entry for the instance the engine names.
// ORIGINAL: 0x00618DA0 ?object_start@Caviar@@QAAXPAUVOX_ScnRenderHnd@@JPAUVOX_Vect@@PAUVOX_Matrix@@PAUVOX_Rect@@PAUVOX_Vect@@@Z 0x00618DA0-0x00618E06 BYTE_EXACT
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

// The two methods above require out-of-line memcpy. Restore the ordinary
// intrinsic context for following ordinary copies. The original callbacks
// are direct live member reads rather than out-of-line representation copies.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma intrinsic(memcpy)
#endif
// The five CaviarData methods form one mapped neighborhood at 0x616BC0-
// 0x616D66, between VoxInstance at 0x616B20 and Caviar at 0x616DA0, beside
// the Caviar scene Buffer lifetimes at 0x616AA0. This source grouping is an
// inferred contribution, not a proven historical TU boundary. CaviarData
// remains its own type; global storage and CRT lifetime ownership stay elsewhere.
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
CaviarData::CaviarData()
    : field_0_(0), fileDescriptor_(0), record_(nullptr) {
}

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

// Original616C80. The second stack argument is present but unused.
// ORIGINAL: 0x00616C80 sub_616c80 0x00616C80-0x00616D33 BYTE_EXACT
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
    for (int index = 0; index < count; ++index) {
        colours[index].rgbRed = record->palette_[(first + index) * 3 + 2];
        colours[index].rgbGreen = record->palette_[(first + index) * 3 + 3];
        colours[index].rgbBlue = record->palette_[(first + index) * 3 + 4];
        // rgbReserved remains unwritten, as in the pinned original.
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

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 12 bytes.
// ORIGINAL: 0x00616D80 sub_616d80 0x00616D80-0x00616D85 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00616D80-0x00616D85 zero EAX and return; callee stack cleanup 12 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x00616D40 sub_616d40 (src/caviar.cpp) and 0x00616DA0 ??0Caviar@@QAE@XZ (src/caviar.cpp)"]}
// symbol    ?sub_616d80@@YGHHHH@Z
// kind      game
int __stdcall sub_616d80(int, int, int) {
    return 0;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x00616D90 nullsub_195 0x00616D90-0x00616D91 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00616D90-0x00616D91 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x00616D40 sub_616d40 (src/caviar.cpp) and 0x00616DA0 ??0Caviar@@QAE@XZ (src/caviar.cpp)"]}
// symbol    ?nullsub_195@@YAXXZ
// kind      game
void __cdecl nullsub_195() {
}
