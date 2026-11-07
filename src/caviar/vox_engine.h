/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

// Internal declarations shared by the AnimaTek Caviar/VOX library's own
// translation units (src/caviar/): the engine callback-slot views, the
// engine's cross-TU entry points and the _SELFMOD kernel identities. The game
// side reaches the engine through caviar.h.
#include "caviar.h"
#include "caviar_render_types.h"

#include <stddef.h>

void __cdecl caviar_original_666fd0(
    int32_t, int32_t, uint32_t, const uint8_t *, int32_t);

namespace {

// The six vx_* IO callback slots installed at 0x009C0B80..0x009C0B94 are
// ordered alloc/free/read/write/seek/tell. Projection output begins at
// 0x009C0BA0, so all six slots are live typed fields disjoint from that output.
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
#else
static_assert(offsetof(VoxIoCallbacks, allocate) == 0 &&
              offsetof(VoxIoCallbacks, release) == sizeof(void *) &&
              offsetof(VoxIoCallbacks, read) == 2 * sizeof(void *) &&
              offsetof(VoxIoCallbacks, write) == 3 * sizeof(void *) &&
              offsetof(VoxIoCallbacks, seek) == 4 * sizeof(void *) &&
              offsetof(VoxIoCallbacks, tell) == 5 * sizeof(void *),
              "host callback view follows host pointer width");
#endif
#define vox_alloc_slot (CaviarOutputMessageWindow.io_allocate_)
#define vox_free_slot (CaviarOutputMessageWindow.io_release_)
#define vox_read_slot (CaviarOutputMessageWindow.io_read_)
#define vox_write_slot (CaviarOutputMessageWindow.io_write_)
#define vox_seek_slot (CaviarOutputMessageWindow.io_seek_)
#define vox_tell_slot (CaviarOutputMessageWindow.io_tell_)

// The engine's fixed-address data bindings live at FILE SCOPE, below, as
// plain static consts rather than inside this anonymous namespace: the
// census counts anonymous fixed-address globals against a tight ceiling and
// file-scope statics are how sprite.cpp binds 0x00698CAC.

} // namespace


// The three engine callees, all HOMED to the foot of this file. The forward
// declarations keep the call sites above them compiling; being same-file
// definitions, the calls stay `call rel32` like the image.
extern "C" void __cdecl vox_stage_error(const char *message);  // stage a message string
extern "C" void __cdecl sub_63ad60(VoxRenderRecord *record);
extern "C" char __cdecl sub_63f9b0(VoxRenderRecord *record);

// The CVR loader, homed below (0x006385D0..0x00638720, 0x00638CC0, 0x00638DE0,
// 0x0063B072). Declarations first so the loader's own call graph - which is
// mutually recursive through the chunk readers - compiles in any order.
extern "C" CvrShape *__cdecl vox_walk_scaled_chain(CvrShape *node, float scale);
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
int __cdecl vox_expand_palette(int fd, uint8_t *palette);


// Defined at the foot of this file, promoted from its artifact.
void __cdecl sub_63ae20(VoxRenderRecord *record);



// The dispatchers install these typed callback identities before their
// definitions later in this translation unit.
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


// 0x0063A280. Four arguments, the draw's four.
void __cdecl caviar_clip_shape(
    VoxRenderRecord *, const CvrShape *, const VOX_Vect *,
    const VOX_Matrix *, const CaviarRenderClipWindowCandidate *, int[6]);



// Candidate source body for 0x63F5E0. Uses the real record/shape/vector
// owners plus a non-owning variable-tail view; not product source yet.
extern "C" void __cdecl sub_63fa80(const VOX_Matrix *);
extern "C" void __cdecl sub_6401b0(const short *, const short *, int *);



void __cdecl caviar_original_666989();
void __cdecl caviar_original_66638f();
void __cdecl caviar_original_6646fb();
void __cdecl caviar_original_664d51();
void __cdecl caviar_original_668069();
void __cdecl caviar_original_667a6f();
void __cdecl caviar_original_665ba1();
void __cdecl caviar_original_66554b();
void __cdecl caviar_original_666f68();
// 0x0066731C: the contained-shadow patcher in src/caviar_shadow_contained.asm,
// the shared tail the four setup stubs below jump into. Its bytes are the
// stubs' second span; it has no mapping of its own.
void __cdecl caviar_shadow_contained_patch();


// 0x00666FD0: assembled from src/caviar_shadow_contained.asm.
void __cdecl caviar_original_666fd0( int32_t x, int32_t y, uint32_t count, const uint8_t *stream, int32_t flags);


// 0x00666989: assembled from src/caviar_part_s2a_contained.asm.
void __cdecl caviar_original_666989();

// 0x0066638F: assembled from src/caviar_part_s2a_clipped.asm.
void __cdecl caviar_original_66638f();

// 0x006646FB: assembled from src/caviar_part_s1a_clipped.asm.
void __cdecl caviar_original_6646fb();

// 0x00664D51: assembled from src/caviar_part_s1a_contained.asm.
void __cdecl caviar_original_664d51();

// 0x00668069: assembled from src/caviar_part_s2o_contained.asm.
void __cdecl caviar_original_668069();

// 0x00667A6F: assembled from src/caviar_part_s2o_clipped.asm.
void __cdecl caviar_original_667a6f();

// 0x00665BA1: assembled from src/caviar_part_s1o_contained.asm.
void __cdecl caviar_original_665ba1();

// 0x0066554B: assembled from src/caviar_part_s1o_clipped.asm.
void __cdecl caviar_original_66554b();

// 0x00666F68: assembled from src/caviar_shadow_clipped.asm.
void __cdecl caviar_original_666f68();

// 0x006647F0: assembled from src/caviar_part_s1a_contained.asm.
void __cdecl caviar_original_6647f0( int32_t x, int32_t y, int32_t z, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00664000: assembled from src/caviar_part_s1a_clipped.asm.
void __cdecl caviar_original_664000( int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00665640: assembled from src/caviar_part_s1o_contained.asm.
void __cdecl caviar_original_665640( int32_t x, int32_t y, int32_t z, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00664E50: assembled from src/caviar_part_s1o_clipped.asm.
void __cdecl caviar_original_664e50( int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00666440: assembled from src/caviar_part_s2a_contained.asm.
void __cdecl caviar_original_666440( int32_t x, int32_t y, int32_t z, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00665CA0: assembled from src/caviar_part_s2a_clipped.asm.
void __cdecl caviar_original_665ca0( int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00667B20: assembled from src/caviar_part_s2o_contained.asm.
void __cdecl caviar_original_667b20( int32_t x, int32_t y, int32_t z, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00667380: assembled from src/caviar_part_s2o_clipped.asm.
void __cdecl caviar_original_667380( int32_t x, int32_t y, int32_t z, const int32_t *rect, uint32_t count, const uint8_t *stream, int32_t flags);
// 0x00666A80: assembled from src/caviar_shadow_clipped.asm.
void __cdecl caviar_original_666a80( int32_t x, int32_t y, const int32_t *rect, uint32_t count, const uint8_t *stream, int32_t flags);

// ---------------------------------------------------------------------------
// Engine state still defined in src/caviar.cpp. The VOX bodies moved into this
// library; these objects keep their current owner until their own placement
// is recovered.
// ---------------------------------------------------------------------------
// The draw currently accesses this 24-byte x86 window at 0x009BE6A4. The
// complete original allocation/aggregate ownership is not established here.
struct CaviarDrawSetupPointerStorageCandidate {
    void *depth_buffer_;
    void *colour_buffer_;
    void *shadow_buffer_;
    uint32_t colour_row_step_;
    uint32_t depth_row_step_;
    uint32_t shadow_row_step_;
};
typedef char CaviarDrawSetupPointerStorageCandidate_x86_size[
    sizeof(void *) != 4 || sizeof(CaviarDrawSetupPointerStorageCandidate) == 0x18
        ? 1 : -1];
extern CaviarDrawSetupPointerStorageCandidate vox_setup_copy_;
extern void *vox_active_colour_table_;
extern uint16_t **vox_active_ramp_a_;
extern const int32_t *vox_active_ramp_b_;
extern uint8_t vox_active_size_code_;
extern int32_t vox_clip_right_doubled_;
extern int32_t vox_clip_bottom_;
extern int32_t vox_clip_left_minus_one_;

extern VOX_Matrix vox_projection_matrix_;
extern int32_t vox_projection_component_[3];
extern int32_t vox_projection_render_stage_;
extern int32_t vox_projection_prepared_;
extern int32_t vox_projection_octant_mask_;
extern VOX_Vect CaviarProjectionDirectionCandidate;
extern CaviarFixedPointProjectionMatrix CaviarFixedPointProjection;
extern float CaviarProjectionScaledMatrix[9];
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
extern const float CaviarProjectionAngleLimit;
extern const float CaviarProjectionAngleWrap;
extern const float CaviarProjectionPolarScale;
extern const float CaviarProjectionAzimuthScale;
extern const float CaviarBoundsRadiusScale;
typedef uint8_t CaviarProjectionAngleRowCandidate[0x860];
extern const CaviarProjectionAngleRowCandidate *CaviarProjectionAngleRows[24];
extern const uint8_t *vox_projection_selected_angle_;
extern const float CaviarProjectionZero;
extern const float CaviarProjectionMinusOne;
extern const float CaviarProjectionOne;
extern const float CaviarProjectionHalfPixel;
extern const float CaviarProjectionNearNegativeEpsilon;
extern const float CaviarProjectionNearPositiveEpsilon;
extern VOX_Matrix CaviarProjectionShadowBasis;
struct CaviarSceneProjectionScratchCandidate;
extern CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch;
extern uint32_t vox_clip_rect[4];

// Scene/raster entry points called across the library's units and by
// src/caviar.cpp's Caviar::render and render_scene.
void __cdecl vox_draw_scene(VoxRenderRecord *record, CaviarSceneContext *context,
                            VoxCameraBlock *view, int step);
void __cdecl vox_load_scene_view(VoxRenderRecord *record, CaviarSceneContext *context,
                                 VoxCameraBlock *camera, int step);
void __cdecl vox_draw_part(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    VOX_Matrix *camera_rotation, const VOX_Vect *state_direction,
    float shape_scale, int visible);
void __cdecl vox_draw_shadow(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    int visible);
extern "C" void __cdecl sub_63a9d0(VOX_Vect *origin, VOX_Vect *edge_b,
                                   VOX_Vect *edge_c, VOX_Vect *edge_d,
                                   VOX_Vect *min_out, VOX_Vect *max_out);
