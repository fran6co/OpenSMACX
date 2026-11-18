/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// The VOX library's raster and projection bodies, 0x0063E950..0x00640A42 in
// the image, grouped by address (CPU detection at 0x0063E860 keeps
// caviar_cpu.cpp). This grouping does not assert the original
// translation-unit boundary.
#include "stdafx.h"
#include "caviar.h"
#include "caviar_io.h"
#include "caviar_cpu.h"
#include "vox_engine.h"
#include "vector.h"
// This library keeps its intrinsics (built /O2 on its own; the image has
// inline block copies, fsqrt/fsin/fcos here): undo stdafx.h's game-wide list.
#pragma intrinsic(memcpy, memset, sqrt, atan, atan2, pow, fabs, labs, exp, log, log10, tan, acos, asin, cosh, sinh, tanh, fmod)

#include <stddef.h>
#include <string.h>
#include "caviar_projection_state.h"

static const char vox_msg_no_address_tables[] = "No room for address tables";



// Native 63F86C/63ECF3/63F344: a record's bit 0 selects either three bytes
// per item or a byte extent stored at the payload start. That dword remains
// part of the callback payload and need not be aligned. No padding is added.
// The image computes it in line at each site (the library is /Ob1).
#define VOX_PAYLOAD_EXTENT(item) \
    (((item)->flags_ & 1) \
         ? *(const uint32_t *)(item)->inline_payload_ \
         : (item)->item_count_ * 3u)


#include "caviar_raster_kernels.inc"


// Caviar projection and scaled sprite owner bodies. Their shared state is
// defined above in this translation unit; original callers and formats stay
// distinct. Exact x87 exceptional behavior and indirect blitter reachability
// remain explicit restoration obligations.


// Scaled projection coordinate rounded half away from the threshold, written
// out in line at every site in the image (the library is /Ob1). Same shape as
// ROUND_VIEW_SCALED further down the file, applied to a scaled value.
#define VOX_ROUND_PROJECTED(v) \
    ((int)((v) * CaviarProjectionDirectionScale + \
           ((v) * CaviarProjectionDirectionScale < CaviarProjectionRoundThreshold \
                ? CaviarProjectionRoundBelow \
                : CaviarProjectionRoundAbove)))

// 0x0063E950. Eight arguments: the record, the walked node, the part's
// position and its scaled matrix, camera rotation, state direction, shape scale
// and the visibility flag the draw computed. The matrix/direction inputs are
// not a pair of clip rectangles.
OPENSMACX_NOINLINE
// SEED HISTORY: archived draw seed mapping used FILE mode; current source ownership is a BODY mapping.
// ORIGINAL: 0x0063E950 sub_63e950 0x0063E950-0x0063EFFF BYTE_EXACT
// symbol    ?vox_draw_part@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@32MH@Z
// kind      game
// TRIED: 8-int __cdecl signature transcribed faithfully from both branches (bit2==0 single-shot with nested bit4 split vs bit2-set do/while loop over pbVar7), following the sibling 0x0063F000's shape; prologue diverges at instr #11 (sub esp/push framing) - not chased further, large FP-heavy function, landed for coverage.
void __cdecl vox_draw_part(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    VOX_Matrix *camera_rotation, const VOX_Vect *state_direction,
    float shape_scale, int visible) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int16_t delta[3];
    float point[3];
    float transformed[3];
    int box[6];
    int clip[6];
    int flags;
    int px, py, pz;
    int ox, oy;

    sub_63fa80(part_matrix);
    caviar_direction_to_projection_index(part_matrix, state_direction,
                                         shape_scale, camera_rotation);

    // 63E994: the record's draw rectangle and extents, six dwords from +0x38.
    memcpy(clip, &record->draw_, sizeof(clip));
    if (scratch.part_clip_.width_ != 0 && scratch.part_clip_.height_ != 0) {
        if (clip[0] < scratch.part_clip_.left_) clip[0] = scratch.part_clip_.left_;
        if (clip[1] < scratch.part_clip_.top_) clip[1] = scratch.part_clip_.top_;
        if (clip[2] > scratch.part_clip_.right_) clip[2] = scratch.part_clip_.right_;
        if (clip[3] > scratch.part_clip_.bottom_) clip[3] = scratch.part_clip_.bottom_;
    }
    if (clip[2] >= 0 && clip[3] >= 0) {
        clip[4] = clip[2] - clip[0] + 1;
        clip[5] = clip[3] - clip[1] + 1;
    } else {
        clip[4] = 0;
        clip[5] = 0;
    }

    caviar_build_projection_tables();
    flags = visible != 0;
    vox_projection_prepared_ = 0;

    if (shape->field_0_ & 2) {
        const CvrRasterItemHeaderCandidate *item;
        uint32_t visited_items;

        box[0] = 0x7fff;
        box[1] = 0x7fff;
        box[2] = -0x7fff;
        box[3] = -0x7fff;
        box[4] = 0;
        box[5] = 0;
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            shape->item_stream_);
        for (visited_items = 0; visited_items < shape->item_count_;) {
            point[0] = static_cast<float>(item->endpoint_[0] + item->base_[0]);
            point[1] = static_cast<float>(item->endpoint_[1] + item->base_[1]);
            point[2] = static_cast<float>(item->endpoint_[2] + item->base_[2]);
            vox_matrix_vector_mul(part_matrix->values, point, transformed);
            vector_add(position->values, transformed, point);
            delta[0] = item->endpoint_[3] - item->base_[0];
            delta[1] = item->endpoint_[4] - item->base_[1];
            delta[2] = item->endpoint_[5] - item->base_[2];
            sub_6401b0(delta, item->extent_, box);
            px = VOX_ROUND_PROJECTED(point[0]);
            py = VOX_ROUND_PROJECTED(point[1]);
            pz = VOX_ROUND_PROJECTED(point[2]);
            ox = static_cast<int>(point[0]);
            oy = static_cast<int>(point[1]);
            if (box[4] != 0 && box[5] != 0) {
                box[0] += ox;
                box[1] += oy;
                box[2] += ox;
                box[3] += oy;
            }
            if (item->flags_ & 1) {
                if (vox_projection_prepared_ == 0) vox_direction_to_octant(record);
                flags |= 0x10;
            } else {
                flags &= ~0x10;
            }
            if (box[2] >= clip[0] && box[3] >= clip[1] &&
                box[0] <= clip[2] && box[1] <= clip[3]) {
                if (shape->field_0_ & 8)
                    flags |= 0x20;
                else
                    flags &= ~0x20;
                if (box[0] >= clip[0] && box[1] >= clip[1] &&
                    box[2] <= clip[2] && box[3] <= clip[3]) {
                    record->part_contained_(px, py, pz, item->item_count_,
                                            item->inline_payload_, flags);
                } else {
                    record->part_clipped_(px, py, pz, clip, item->item_count_,
                                          item->inline_payload_, flags);
                }
            }
            visited_items += item->item_count_;
            if (item->flags_ & 1)
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + *(const uint32_t *)item->inline_payload_);
            else
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + item->item_count_ * 3);
        }
    } else {
        if (shape->field_0_ & 4) {
            px = VOX_ROUND_PROJECTED(position->values[0]);
            py = VOX_ROUND_PROJECTED(position->values[1]);
            pz = VOX_ROUND_PROJECTED(position->values[2]);
        } else {
            point[0] = static_cast<float>(shape->raster_origin_[0]);
            point[1] = static_cast<float>(shape->raster_origin_[1]);
            point[2] = static_cast<float>(shape->raster_origin_[2]);
            vox_matrix_vector_mul(part_matrix->values, point, transformed);
            vector_add(position->values, transformed, point);
            px = VOX_ROUND_PROJECTED(point[0]);
            py = VOX_ROUND_PROJECTED(point[1]);
            pz = VOX_ROUND_PROJECTED(point[2]);
        }
        // 63EEB5: the flat box offset is the untransformed position.
        ox = static_cast<int>(position->values[0]);
        oy = static_cast<int>(position->values[1]);
        sub_6401b0(&shape->offset_x_, &shape->axis_x_, box);
        if (box[4] != 0 && box[5] != 0) {
            box[0] += ox;
            box[1] += oy;
            box[2] += ox;
            box[3] += oy;
        }
        if (shape->field_0_ & 4) {
            vox_direction_to_octant(record);
            flags |= 0x10;
        } else {
            flags &= ~0x10;
        }
        if (box[2] >= clip[0] && box[3] >= clip[1] &&
            box[0] <= clip[2] && box[1] <= clip[3]) {
            if (shape->field_0_ & 8)
                flags |= 0x20;
            else
                flags &= ~0x20;
            if (box[0] >= clip[0] && box[1] >= clip[1] &&
                box[2] <= clip[2] && box[3] <= clip[3]) {
                record->part_contained_(px, py, pz, shape->item_count_,
                                        shape->item_stream_, flags);
            } else {
                record->part_clipped_(px, py, pz, clip, shape->item_count_,
                                      shape->item_stream_, flags);
            }
        }
    }
}



// 0x009BE6C0: the record's shadow table, copied where the shadow kernels read
// it (image 0x63F0B4 `mov eax,[ebx+0x54]; mov [0x9be6c0],eax`). Its only
// in-image reference outside _SELFMOD is this store. Defined beside its only
// C writer until its owner is settled (vox_active_colour_table_ 0x9BE6BC
// lives in caviar.cpp).
// RECOVERY_OBJECT: v1 0x009BE6C0 ?vox_active_shadow_table_@@3PAXA
void *vox_active_shadow_table_ = 0;  // 0x009BE6C0

// Scale a projected coordinate by 256 and round half away from zero
// (image 0x63F1CF..0x63F200 and each sibling: fmul 671038, fcom 67103C,
// fld 671040/671044, fadd, _ftol). Written out at every site in the image.
#define VOX_PROJECT_ROUND(dst, value) \
    { \
        float scaled_ = (value) * CaviarProjectionDirectionScale; \
        (dst) = (int)(scaled_ + (scaled_ < CaviarProjectionRoundThreshold \
                                     ? CaviarProjectionRoundBelow \
                                     : CaviarProjectionRoundAbove)); \
    }

// 0x0063F000. Five arguments: the shadow branch's counterpart of the above.
OPENSMACX_NOINLINE
// SEED HISTORY: archived draw seed mapping used FILE mode; current source ownership is a BODY mapping.
// ORIGINAL: 0x0063F000 sub_63f000 0x0063F000-0x0063F5D7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x0063F000 reserves 0x58 bytes and saves four registers; after helper-argument cleanup at 0x0063F026, load 0x0063F0C1 from ESP+0x7C is entry ESP+0x14, the fifth cdecl argument. XOR EDI,EDI at 0x0063F02B and CMP ESI,EDI / SETNE BL at 0x0063F0CB / 0x0063F0D5 establish 0/1 normalization.", "The original caller supplies its scalar LOD local, not camera bounds. Flat/tiled and contained/clipped paths preserve this low bit while independently setting 0x10/0x20 flags. This corrects a source type/value interpretation without changing original identities, layout or quirks."], "reason": "Interpret the fifth shadow argument as scalar visibility and normalize its nonzero value into the dispatch low bit."}
// symbol    ?vox_draw_shadow@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@H@Z
// kind      game
// TRIED: 5-int __cdecl signature transcribed faithfully from the two branches (bit2==0 single-shot vs bit2-set do/while loop over pbVar6); prologue diverges at instr #0 (sub esp,0x58 vs push) - not chased further, large FP-heavy function, landed for coverage.
void __cdecl vox_draw_shadow(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    int visible) {
    uint32_t visited_items;               // esp+0x10
    int px;                               // esp+0x14 (tiled) / ebp (flat)
    int py;
    int ox;
    int oy;
    int16_t delta[3];                     // esp+0x18
    float point[3];                       // esp+0x20, also the translated point
    float rotated[3];                     // esp+0x2C
    int box[6];                           // esp+0x38
    // WORKAROUND(type): record+0x38..+0x4F is copied whole as one six-dword
    // window (rep movsd, ecx=6 at 0x63F029).
    CaviarRenderClipWindowCandidate clip; // esp+0x50
    int flags;                            // the dead `visible` slot
    const CvrRasterItemHeaderCandidate *item;

    sub_63fa80(part_matrix);
    clip = record->draw_;
    if (CaviarSceneProjectionScratch.shadow_clip_.width_ != 0 &&
        CaviarSceneProjectionScratch.shadow_clip_.height_ != 0) {
        if (clip.left_ < CaviarSceneProjectionScratch.shadow_clip_.left_)
            clip.left_ = CaviarSceneProjectionScratch.shadow_clip_.left_;
        if (clip.top_ < CaviarSceneProjectionScratch.shadow_clip_.top_)
            clip.top_ = CaviarSceneProjectionScratch.shadow_clip_.top_;
        if (clip.right_ > CaviarSceneProjectionScratch.shadow_clip_.right_)
            clip.right_ = CaviarSceneProjectionScratch.shadow_clip_.right_;
        if (clip.bottom_ > CaviarSceneProjectionScratch.shadow_clip_.bottom_)
            clip.bottom_ = CaviarSceneProjectionScratch.shadow_clip_.bottom_;
    }
    if (clip.right_ >= 0 && clip.bottom_ >= 0) {
        clip.width_ = clip.right_ - clip.left_ + 1;
        clip.height_ = clip.bottom_ - clip.top_ + 1;
    } else {
        clip.width_ = 0;
        clip.height_ = 0;
    }
    vox_active_shadow_table_ = record->shadow_table_;
    caviar_build_projection_tables();
    flags = visible != 0;
    vox_projection_prepared_ = 0;

    if (shape->field_0_ & 2) {
        box[0] = 0x7fff;
        box[1] = 0x7fff;
        box[2] = -0x7fff;
        box[3] = -0x7fff;
        box[4] = 0;
        box[5] = 0;
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            shape->item_stream_);
        for (visited_items = 0; visited_items < shape->item_count_;) {
            point[0] = (float)(item->endpoint_[0] + item->base_[0]);
            point[1] = (float)(item->endpoint_[1] + item->base_[1]);
            point[2] = (float)(item->endpoint_[2] + item->base_[2]);
            vox_matrix_vector_mul(part_matrix->values, point, rotated);
            vector_add(position->values, rotated, point);
            delta[0] = item->endpoint_[3] - item->base_[0];
            delta[1] = item->endpoint_[4] - item->base_[1];
            delta[2] = item->endpoint_[5] - item->base_[2];
            sub_6401b0(delta, item->extent_, box);
            VOX_PROJECT_ROUND(px, point[0]);
            VOX_PROJECT_ROUND(py, point[1]);
            ox = (int)point[0];
            oy = (int)point[1];
            if (box[4] != 0 && box[5] != 0) {
                box[0] += ox;
                box[1] += oy;
                box[2] += ox;
                box[3] += oy;
            }
            if (item->flags_ & 1) {
                if (vox_projection_prepared_ == 0)
                    vox_inverse_direction_to_octant(part_matrix);
                flags |= 0x10;
            } else {
                flags &= ~0x10;
            }
            if (box[2] >= clip.left_ && box[3] >= clip.top_ &&
                box[0] <= clip.right_ && box[1] <= clip.bottom_) {
                if (shape->field_0_ & 8)
                    flags |= 0x20;
                else
                    flags &= ~0x20;
                if (box[0] >= clip.left_ && box[1] >= clip.top_ &&
                    box[2] <= clip.right_ && box[3] <= clip.bottom_) {
                    record->shadow_contained_(px, py, item->item_count_,
                                              item->inline_payload_, flags);
                } else {
                    record->shadow_clipped_(px, py, &clip.left_,
                                            item->item_count_,
                                            item->inline_payload_, flags);
                }
            }
            visited_items += item->item_count_;
            if (item->flags_ & 1)
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + *(const uint32_t *)item->inline_payload_);
            else
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + item->item_count_ * 3);
        }
    } else {
        if (shape->field_0_ & 4) {
            VOX_PROJECT_ROUND(px, position->values[0]);
            VOX_PROJECT_ROUND(py, position->values[1]);
        } else {
            point[0] = (float)shape->raster_origin_[0];
            point[1] = (float)shape->raster_origin_[1];
            point[2] = (float)shape->raster_origin_[2];
            vox_matrix_vector_mul(part_matrix->values, point, rotated);
            vector_add(position->values, rotated, point);
            VOX_PROJECT_ROUND(px, point[0]);
            VOX_PROJECT_ROUND(py, point[1]);
        }
        // The flat box is offset by the untransformed position in both cases
        // (0x63F49E/0x63F4A7 read [edi], [edi+4] with edi = position).
        ox = (int)position->values[0];
        oy = (int)position->values[1];
        sub_6401b0(&shape->offset_x_, &shape->axis_x_, box);
        if (box[4] != 0 && box[5] != 0) {
            box[0] += ox;
            box[1] += oy;
            box[2] += ox;
            box[3] += oy;
        }
        if (shape->field_0_ & 4) {
            vox_inverse_direction_to_octant(part_matrix);
            flags |= 0x10;
        } else {
            flags &= ~0x10;
        }
        if (box[2] >= clip.left_ && box[3] >= clip.top_ &&
            box[0] <= clip.right_ && box[1] <= clip.bottom_) {
            if (shape->field_0_ & 8)
                flags |= 0x20;
            else
                flags &= ~0x20;
            if (box[0] >= clip.left_ && box[1] >= clip.top_ &&
                box[2] <= clip.right_ && box[3] <= clip.bottom_) {
                record->shadow_contained_(px, py, shape->item_count_,
                                          shape->item_stream_, flags);
            } else {
                record->shadow_clipped_(px, py, &clip.left_,
                                        shape->item_count_,
                                        shape->item_stream_, flags);
            }
        }
    }
}

#undef VOX_PROJECT_ROUND
#undef VOX_ROUND_PROJECTED

// ORIGINAL: 0x0063F5E0 sub_63f5e0 0x0063F5E0-0x0063F9A8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?caviar_clip_shape@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PBUVOX_Matrix@@PBUCaviarRenderClipWindowCandidate@@QAH@Z
// TRIED: 6-arg __cdecl signature derived from esp+N reads at the anchor (post-prologue esp) rather than IDA's untyped guess; the per-vertex bbox loop, sub_6280e0/sub_628150/sub_6401b0 arg order, and the final outBox/localBox clamp (which Ghidra's pseudocode also shows) are transcribed from the raw asm since Ghidra's __ftol() calls hide their FPU source operands.
void __cdecl caviar_clip_shape(VoxRenderRecord *record,
                               const CvrShape *shape,
                               const VOX_Vect *position,
                               const VOX_Matrix *orientation,
                               const CaviarRenderClipWindowCandidate *clip,
                               int output[6]) {
    uint32_t visited_items;  // the dead `clip` slot (esp+0x78)
    int ox;
    int oy;
    int16_t delta[3];        // esp+0x10
    float point[3];          // esp+0x18
    float transformed[3];    // esp+0x24
    int box[6];              // esp+0x30
    int local_box[6];        // esp+0x48
    const CvrRasterItemHeaderCandidate *item;

    output[4] = 0;
    output[5] = 0;
    output[0] = 0x7fff;
    output[1] = 0x7fff;
    output[2] = -0x7fff;
    output[3] = -0x7fff;

    sub_63fa80(orientation);
    // 63F61E: the record's draw rectangle and extents, six dwords from +0x38
    // (rep movsd, ecx=6). Unlike the draw siblings, width/height are not
    // recomputed after the clamp.
    memcpy(local_box, &record->draw_, sizeof(local_box));
    if (clip->width_ != 0 && clip->height_ != 0) {
        if (local_box[0] < clip->left_) local_box[0] = clip->left_;
        if (local_box[1] < clip->top_) local_box[1] = clip->top_;
        if (local_box[2] > clip->right_) local_box[2] = clip->right_;
        if (local_box[3] > clip->bottom_) local_box[3] = clip->bottom_;
    }

    if (shape->field_0_ & 2) {
        box[0] = 0x7fff;
        box[1] = 0x7fff;
        box[2] = -0x7fff;
        box[3] = -0x7fff;
        box[4] = 0;
        box[5] = 0;
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            shape->item_stream_);
        for (visited_items = 0; visited_items < shape->item_count_;) {
            point[0] = (float)(item->endpoint_[0] + item->base_[0]);
            point[1] = (float)(item->endpoint_[1] + item->base_[1]);
            point[2] = (float)(item->endpoint_[2] + item->base_[2]);
            vox_matrix_vector_mul(orientation->values, point, transformed);
            vector_add(position->values, transformed, point);
            delta[0] = item->endpoint_[3] - item->base_[0];
            delta[1] = item->endpoint_[4] - item->base_[1];
            delta[2] = item->endpoint_[5] - item->base_[2];
            sub_6401b0(delta, item->extent_, box);
            // 63F75D: the offset is the translated point, not `transformed`.
            ox = (int)point[0];
            oy = (int)point[1];
            if (box[4] != 0 && box[5] != 0) {
                box[0] += ox;
                box[1] += oy;
                box[2] += ox;
                box[3] += oy;
            }
            if (box[2] >= local_box[0] && box[3] >= local_box[1] &&
                box[0] <= local_box[2] && box[1] <= local_box[3]) {
                if (box[0] < local_box[0]) box[0] = local_box[0];
                if (box[1] < local_box[1]) box[1] = local_box[1];
                if (box[2] > local_box[2]) box[2] = local_box[2];
                if (box[3] > local_box[3]) box[3] = local_box[3];
                if (box[2] >= 0 && box[3] >= 0) {
                    box[4] = box[2] - box[0] + 1;
                    box[5] = box[3] - box[1] + 1;
                } else {
                    box[4] = 0;
                    box[5] = 0;
                }
                if (box[4] != 0 && box[5] != 0) {
                    if (output[0] > box[0]) output[0] = box[0];
                    if (output[1] > box[1]) output[1] = box[1];
                    if (output[2] < box[2]) output[2] = box[2];
                    if (output[3] < box[3]) output[3] = box[3];
                }
            }
            visited_items += item->item_count_;
            if (item->flags_ & 1)
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + *(const uint32_t *)item->inline_payload_);
            else
                item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
                    item->inline_payload_ + item->item_count_ * 3);
        }
    } else {
        ox = (int)position->values[0];
        oy = (int)position->values[1];
        sub_6401b0(&shape->offset_x_, &shape->axis_x_, output);
        if (output[4] != 0 && output[5] != 0) {
            output[0] += ox;
            output[1] += oy;
            output[2] += ox;
            output[3] += oy;
        }
        if (output[2] < local_box[0] || output[3] < local_box[1] ||
            output[0] > local_box[2] || output[1] > local_box[3]) {
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
        if (output[2] > local_box[2]) output[2] = local_box[2];
        if (output[3] > local_box[3]) output[3] = local_box[3];
    }

    // 63F947: both paths finish here (the flat path too, after its clamp).
    if (output[2] >= 0 && output[3] >= 0) {
        output[4] = output[2] - output[0] + 1;
        output[5] = output[3] - output[1] + 1;
        return;
    }
    output[4] = 0;
    output[5] = 0;
}

/*
Purpose: Allocate a render record's two per-entry ramps. Frees whatever the
         record still held at +0x60 and +0x64, allocates count entries of
         each from the setup descriptor at +4, and when both succeed fills
         them: ramp A with base stepped by the descriptor's step, ramp B with
         a running accumulation of its delta. On either allocation failing,
         stages the engine's 27-byte failure message into the shared buffer
         and reports failure to vox_create_record.
// ORIGINAL: 0x0063F9B0 sub_63f9b0 0x0063F9B0-0x0063FA72 BYTE_EXACT
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
extern "C" char __cdecl sub_63f9b0(VoxRenderRecord *record) {
    VoxRenderRecord *const typed = record;
    if (typed->ramp_a_ptr_ != 0) {
        vox_free_slot(typed->ramp_a_ptr_);
    }
    if (typed->ramp_b_ptr_ != 0) {
        vox_free_slot(typed->ramp_b_ptr_);
    }
    typed->ramp_a_ptr_ = static_cast<uint16_t **>(
        vox_alloc_slot(static_cast<uint32_t>(
            typed->setup_block_->height_) << 2));
    typed->ramp_b_ptr_ = static_cast<int32_t *>(
        vox_alloc_slot(static_cast<uint32_t>(
            typed->setup_block_->height_) << 2));
    if (typed->ramp_a_ptr_ != 0 && typed->ramp_b_ptr_ != 0) {
        CaviarSceneSetupDescriptorCandidate *const info = typed->setup_block_;
        uint32_t base_bits = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(info->depth_buffer_));
        if (base_bits == 0) {
            return 0;
        }
        const uint32_t step_bits =
            static_cast<uint32_t>(info->depth_row_step_) << 1;
        uint32_t accum_bits = 0;
        int32_t index = 0;
        if (info->height_ > 0) {
            do {
                typed->ramp_a_ptr_[index] = reinterpret_cast<uint16_t *>(
                    static_cast<uintptr_t>(base_bits));
                base_bits += step_bits;
                typed->ramp_b_ptr_[index] = static_cast<int32_t>(accum_bits);
                accum_bits += static_cast<uint32_t>(
                    typed->setup_block_->colour_row_step_);
                ++index;
            } while (index < typed->setup_block_->height_);
        }
        return 0;
    }
    memcpy(CaviarOutputMessageWindow.message_,
           vox_msg_no_address_tables, 27);
    return 1;
}


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
// ORIGINAL: 0x0063FA80 sub_63fa80 0x0063FA80-0x0063FC92 BYTE_EXACT
// symbol    _sub_63fa80
// size      530 bytes
// Deleted seed provenance: git:123267b4afb63f4fd3396772db0fcf89d35259c9:src/unrecovered/0063fa80.cpp
Status: the four .rdata words stay bindings rather than literals. The
        words they feed are named from
        the two bodies' use of the cluster: 0x009C0A40..0x4A are the six
        matrix words 0x006401B0 multiplies by, 0x009C0A4C..0x54 the three
        int words rounded beside them, 0x009C0A7C the float scratch, and
        0x009C0AC4/0x009C0AC8 the two trailing words this body zeroes.
*/
// 63FAB3 and each sibling compare select 671040 (-0.5) when x87 C0
// is set, otherwise 671044 (+0.5). The old address aliases had these
// two names/polarities reversed; this follows the pinned instruction edges.
// The image rounds each word in line (the library is /Ob1), so this is a
// macro rather than a function.
#define ROUND_VIEW_SCALED(value) \
    ((int)((value) + ((value) < CaviarProjectionRoundThreshold \
                          ? CaviarProjectionRoundBelow \
                          : CaviarProjectionRoundAbove)))

extern "C" void __cdecl sub_63fa80(const VOX_Matrix *sources) {
    // Two walking pointers; VC5 itself forms the image's one moving pointer
    // plus fixed delta.
    const float *cursor = &sources->values[0][0];
    float *out = CaviarProjectionScaledMatrix;
    for (int i = 0; i < 9; ++i) {
        *out++ = *cursor++ * CaviarProjectionDirectionScale;
    }

    CaviarFixedPointProjection.word_00_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[0]);
    CaviarFixedPointProjection.word_02_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[1]);
    CaviarFixedPointProjection.word_04_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[2]);
    CaviarFixedPointProjection.word_06_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[3]);
    CaviarFixedPointProjection.word_08_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[4]);
    CaviarFixedPointProjection.word_0A_ = (short)ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[5]);
    CaviarFixedPointProjection.dword_0C_ = ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[6]);
    CaviarFixedPointProjection.dword_10_ = ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[7]);
    CaviarFixedPointProjection.dword_14_ = ROUND_VIEW_SCALED(CaviarProjectionScaledMatrix[8]);

    vox_projection_render_stage_ = 0;
    vox_projection_prepared_ = 0;
}
#undef ROUND_VIEW_SCALED




// Typed candidates for the two direction-to-octant routines. Their shared
// storage declarations are provisional and must be admitted only with the
// accompanying source mapping/consumer closure.

// Native 0x63FCA0 direction selection. The original 24-row table is composed
// from typed source-owned pointers and its pinned bytes come from the existing
// configure-time terrain-data initializer hook.
// ORIGINAL: 0x0063FCA0 sub_63fca0 0x0063FCA0-0x0063FDF6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?caviar_direction_to_projection_index@@YAXPBUVOX_Matrix@@PBUVOX_Vect@@M0@Z
// TRIED: FPU scheduling of the two-argument sub_627f50 call push sequence (register-save push esi + call-arg pushes) does not reproduce exactly; landed at 0.77 mnemonic similarity. The logic itself (spherical-to-index trig via acos/atan2/table lookup) is transcribed directly from the disassembly.
void __cdecl caviar_direction_to_projection_index(
    const VOX_Matrix *part_matrix, const VOX_Vect *direction,
    float shape_scale, const VOX_Matrix *camera_rotation) {
    VOX_Vect camera_direction;
    const float xy_scale = shape_scale * CaviarSceneProjectionScratch.projected_scale_;
    const float z_scale = shape_scale * CaviarSceneProjectionScratch.viewport_aspect_;

    vox_matrix_inverse(part_matrix->values, vox_projection_matrix_.values);
    vox_projection_render_stage_ = 1;

    vox_matrix_vector_mul(camera_rotation->values, direction->values,
                          camera_direction.values);
    camera_direction.values[0] *= xy_scale;
    camera_direction.values[1] *= xy_scale;
    camera_direction.values[2] *= z_scale;

    vox_matrix_vector_mul(vox_projection_matrix_.values, camera_direction.values,
                          CaviarProjectionDirectionCandidate.values);
    if (!(CaviarProjectionDirectionCandidate.values[2] >=
          CaviarProjectionRoundThreshold)) {
        CaviarProjectionDirectionCandidate.values[0] =
            -CaviarProjectionDirectionCandidate.values[0];
        CaviarProjectionDirectionCandidate.values[1] =
            -CaviarProjectionDirectionCandidate.values[1];
        CaviarProjectionDirectionCandidate.values[2] =
            -CaviarProjectionDirectionCandidate.values[2];
    }

    float polar_angle;
    if (CaviarProjectionDirectionCandidate.values[2] >= CaviarProjectionAngleLimit) {
        polar_angle = CaviarProjectionRoundThreshold;
    } else {
        polar_angle = acos(CaviarProjectionDirectionCandidate.values[2]);
    }

    const float raw_azimuth = atan2(CaviarProjectionDirectionCandidate.values[1],
                                    CaviarProjectionDirectionCandidate.values[0]);
    const float azimuth_angle = raw_azimuth < CaviarProjectionRoundThreshold
        ? raw_azimuth - CaviarProjectionAngleWrap
        : raw_azimuth;

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
// ORIGINAL: 0x0063FE00 sub_63fe00 0x0063FE00-0x0063FFDD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.", "The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.", "The projection builder at 0x640A27 advances its cursor before storing: packed rows begin at 0x9C0BA0, with fields at offsets0/4/8 and stride16, disjoint from callbacks at 0x9C0B80..0x9C0B94. This supersedes the earlier overlapping-output interpretation.", "Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."], "reason": "Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations."}
// symbol    ?vox_direction_to_octant@@YAXPBUVoxRenderRecord@@@Z
// TRIED: near-duplicate of 0x0063FFE0 past the preamble (same 628220/6281b0/round-to-nearest/octant/ratio tail) - only the first block differs, negating all three floats when `*param_1 & 1` instead of calling sub_627f50. The three round-to-nearest computations are inlined at each call site rather than factored into a shared helper: a free-function helper ahead of the definition compiles to zero external .text symbols in body-mode extraction (only a helper CLASS is special-cased). Landing the closest control-flow-faithful form (divergence starts at instruction #2, on the flag test).
#if defined(_MSC_VER) && _MSC_VER <= 1200
// Reconstruction control: stdafx forces CRT abs; these two original bodies
// use inline integer absolute values. This does not recover an original pragma.
#pragma intrinsic(abs)
#endif
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
    vox_normalise_vector(vector, unit);
    vec3_scale(unit, 256.0f, vector);

    // The original tests x87 C0: unordered follows the below branch too.
    // Store the globals first and read them back (VC5 register choice).
    vox_projection_component_[0] = static_cast<int>(vector[0] + (vector[0] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));
    vox_projection_component_[1] = static_cast<int>(vector[1] + (vector[1] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));
    vox_projection_component_[2] = static_cast<int>(vector[2] + (vector[2] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));

    const int x = vox_projection_component_[0];
    const int y = vox_projection_component_[1];
    const int z = vox_projection_component_[2];
    const int abs_x = abs(x);
    const int abs_y = abs(y);
    const int abs_z = abs(z);
    const int sum = abs_x + abs_y + abs_z;
    const float ratio = CaviarProjectionDirectionScale / (float)sum;

    int octant;
    if (y > 0) {
        octant = x <= 0;
    } else {
        octant = (x > 0) + 2;
    }
    if (z < 0) {
        octant += 4;
    }

    const int fx = (int)(abs_x * ratio);
    const int fy = (int)(abs_y * ratio);

    int bits;
    if (fx > 0x80) {
        bits = 2;
    } else if (fy > 0x80) {
        bits = 3;
    } else {
        bits = fy <= 0x80 - fx;
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
// ORIGINAL: 0x0063FFE0 sub_63ffe0 0x0063FFE0-0x006401A8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.", "The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.", "The projection builder at 0x640A27 advances its cursor before storing: packed rows begin at 0x9C0BA0, with fields at offsets0/4/8 and stride16, disjoint from callbacks at 0x9C0B80..0x9C0B94. This supersedes the earlier overlapping-output interpretation.", "Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."], "reason": "Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations."}
// symbol    ?vox_inverse_direction_to_octant@@YAXPBUVOX_Matrix@@@Z
// TRIED: the three callees (sub_627f50/sub_628220/sub_6281b0) are catalogued nullary, so they are reached through explicit cdecl function-pointer casts rather than by name. The first round-to-nearest computation tests the sign of localC[0] (the sub_628220 output) but rounds local18 (the sub_6281b0 output) - an asymmetry the disassembly shows plainly and this keeps, even though axes 1 and 2 are self-referential. A shared `round_nearest` free-function helper ahead of the definition compiled to zero external .text symbols (body-mode extraction only special-cases a helper CLASS, not a helper free function) - inlined the three round computations instead. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the float-copy preamble).
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *source) {
    vox_matrix_inverse(source->values, vox_projection_matrix_.values);
    float vector[3];
    vector[0] = vox_projection_matrix_.values[0][2];
    vector[1] = vox_projection_matrix_.values[1][2];
    vox_projection_render_stage_ = 1;
    vector[2] = vox_projection_matrix_.values[2][2];
    float unit[3];
    vox_normalise_vector(vector, unit);
    vec3_scale(unit, 256.0f, vector);

    // The original tests x87 C0: unordered follows the below branch too.
    // Store the globals first and read them back (VC5 register choice).
    vox_projection_component_[0] = static_cast<int>(vector[0] + (vector[0] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));
    vox_projection_component_[1] = static_cast<int>(vector[1] + (vector[1] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));
    vox_projection_component_[2] = static_cast<int>(vector[2] + (vector[2] < CaviarProjectionRoundThreshold
                                                    ? CaviarProjectionRoundBelow
                                                    : CaviarProjectionRoundAbove));

    const int x = vox_projection_component_[0];
    const int y = vox_projection_component_[1];
    const int z = vox_projection_component_[2];
    const int abs_x = abs(x);
    const int abs_y = abs(y);
    const int abs_z = abs(z);
    const int sum = abs_x + abs_y + abs_z;
    const float ratio = CaviarProjectionDirectionScale / (float)sum;

    int octant;
    if (y > 0) {
        octant = x <= 0;
    } else {
        octant = (x > 0) + 2;
    }
    if (z < 0) {
        octant += 4;
    }

    const int fx = (int)(abs_x * ratio);
    const int fy = (int)(abs_y * ratio);

    int bits;
    if (fx > 0x80) {
        bits = 2;
    } else if (fy > 0x80) {
        bits = 3;
    } else {
        bits = fy <= 0x80 - fx;
    }

    vox_projection_prepared_ = 1;
    vox_projection_octant_mask_ = 1 << (octant * 4 | bits);
}
#if defined(_MSC_VER) && _MSC_VER <= 1200
// Restore the shared CRT policy for all following renderer definitions.
#pragma function(abs)
#endif


// Shorthands for the fixed-point basis (0x9C0A40..0x9C0A57) and the 26
// neighbour cells (0x9C0840, stride 16). The image rereads the basis globals
// (movsx/mov from 0x9C0A40/46/4C) instead of spilling them, so they are read
// directly, not copied into locals.
#define PROJ_X0 ((int)CaviarFixedPointProjection.word_00_)
#define PROJ_X1 ((int)CaviarFixedPointProjection.word_02_)
#define PROJ_X2 ((int)CaviarFixedPointProjection.word_04_)
#define PROJ_Y0 ((int)CaviarFixedPointProjection.word_06_)
#define PROJ_Y1 ((int)CaviarFixedPointProjection.word_08_)
#define PROJ_Y2 ((int)CaviarFixedPointProjection.word_0A_)
#define PROJ_Z0 (CaviarFixedPointProjection.dword_0C_)
#define PROJ_Z1 (CaviarFixedPointProjection.dword_10_)
#define PROJ_Z2 (CaviarFixedPointProjection.dword_14_)
#define PROJ_CELL(i) (CaviarKernelProjectionWindow.current_views_.cells_[i])

// ORIGINAL: 0x006404F0 sub_6404f0 0x006404F0-0x00640A42
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.", "The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.", "The projection builder at 0x640A27 advances its cursor before storing: packed rows begin at 0x9C0BA0, with fields at offsets0/4/8 and stride16, disjoint from callbacks at 0x9C0B80..0x9C0B94. This supersedes the earlier overlapping-output interpretation.", "Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."], "reason": "Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations."}
// symbol    ?caviar_build_projection_tables@@YAXXZ
// TRIED: this is SMACX's own isometric-tile offset-table builder, not zlib/CRT - checked, no recognisable library algorithm. Read a 6-short/3-int source record at 0x9C0A40, fanned it out formula- for-formula (per the Ghidra decomp, cross-checked against the disassembly's add/sub/shl/neg chains) into ~80 fixed-address cells declared by the scaffold, then transcribed the closing 26-iteration (0x1A) table walk with raw int*/uint* pointer arithmetic over 0x9C0840/0x9C0BA8. Compiles and is formula- faithful but MISMATCHES at instruction #0 (`sub esp,0x34` vs a `push`-first prologue) - at this size (~200 instructions, 79 scratch cells) the compiler's register/stack-slot allocation diverges from the first instruction, and chasing it further is a search this pass's budget does not cover.
void __cdecl caviar_build_projection_tables() {
    int i;

    // Cell (a,b,c) = a*basis0 + b*basis1 + c*basis2, a,b,c in {-1,0,1},
    // c fastest, (0,0,0) omitted. z is kept in 24.8 fixed point.
    // VC6 re-associates these sums against the cells already computed, so
    // the operand order written here is a code-generation lever rather than
    // the evaluation order: the orders below were chosen by measuring every
    // permutation of each cell against the image (6404F0..6409E3), not read
    // from it.
    // Executing the image symbolically gives each cell's tree. All match
    // except the three two-plus-one-minus cells: the image forms 8, 19 and
    // 23 as (sum of the positives) - negative in x and y (8 from a fresh
    // lea, 19 and 23 from the pairs cells 3 and 1 spilled) and as
    // (positive - negative) + positive in z (the cell 5, 11 and 14 pairs).
    // VC6 gives the opposite association for every spelling of these cells
    // and of their neighbours 5, 7, 9 and 16 (384 combinations), so their
    // original expressions were not three-term chains of the basis reads.
    // Cell 17 is the image's (b0 - b1) - b2 with the pair shared by cell 18;
    // 19 x and 25 z are spelled to keep the thirteen temporaries.
    PROJ_CELL(0).x_ = -(PROJ_X0 + PROJ_X1 + PROJ_X2);
    PROJ_CELL(0).y_ = -(PROJ_Y0 + PROJ_Y1 + PROJ_Y2);
    PROJ_CELL(0).z_ = -(PROJ_Z0 + PROJ_Z1 + PROJ_Z2) << 8;
    PROJ_CELL(1).x_ = -(PROJ_X0 + PROJ_X1);
    PROJ_CELL(1).y_ = -(PROJ_Y0 + PROJ_Y1);
    PROJ_CELL(1).z_ = -(PROJ_Z0 + PROJ_Z1) << 8;
    PROJ_CELL(2).x_ = PROJ_X2 - PROJ_X0 - PROJ_X1;
    PROJ_CELL(2).y_ = PROJ_Y2 - PROJ_Y0 - PROJ_Y1;
    PROJ_CELL(2).z_ = (PROJ_Z2 - PROJ_Z1 - PROJ_Z0) << 8;
    PROJ_CELL(3).x_ = -(PROJ_X0 + PROJ_X2);
    PROJ_CELL(3).y_ = -(PROJ_Y0 + PROJ_Y2);
    PROJ_CELL(3).z_ = -(PROJ_Z0 + PROJ_Z2) << 8;
    PROJ_CELL(4).x_ = -PROJ_X0;
    PROJ_CELL(4).y_ = -PROJ_Y0;
    PROJ_CELL(4).z_ = -PROJ_Z0 << 8;
    PROJ_CELL(5).x_ = PROJ_X2 - PROJ_X0;
    PROJ_CELL(5).y_ = PROJ_Y2 - PROJ_Y0;
    PROJ_CELL(5).z_ = (PROJ_Z2 - PROJ_Z0) << 8;
    PROJ_CELL(6).x_ = PROJ_X1 - PROJ_X0 - PROJ_X2;
    PROJ_CELL(6).y_ = PROJ_Y1 - PROJ_Y0 - PROJ_Y2;
    PROJ_CELL(6).z_ = (PROJ_Z1 - PROJ_Z2 - PROJ_Z0) << 8;
    PROJ_CELL(7).x_ = PROJ_X1 - PROJ_X0;
    PROJ_CELL(7).y_ = PROJ_Y1 - PROJ_Y0;
    PROJ_CELL(7).z_ = (PROJ_Z1 - PROJ_Z0) << 8;
    PROJ_CELL(8).x_ = PROJ_X1 + PROJ_X2 - PROJ_X0;
    PROJ_CELL(8).y_ = PROJ_Y1 + PROJ_Y2 - PROJ_Y0;
    PROJ_CELL(8).z_ = (PROJ_Z2 - PROJ_Z0 + PROJ_Z1) << 8;
    PROJ_CELL(9).x_ = -(PROJ_X1 + PROJ_X2);
    PROJ_CELL(9).y_ = -(PROJ_Y1 + PROJ_Y2);
    PROJ_CELL(9).z_ = -(PROJ_Z1 + PROJ_Z2) << 8;
    PROJ_CELL(10).x_ = -PROJ_X1;
    PROJ_CELL(10).y_ = -PROJ_Y1;
    PROJ_CELL(10).z_ = -PROJ_Z1 << 8;
    PROJ_CELL(11).x_ = PROJ_X2 - PROJ_X1;
    PROJ_CELL(11).y_ = PROJ_Y2 - PROJ_Y1;
    PROJ_CELL(11).z_ = (PROJ_Z2 - PROJ_Z1) << 8;
    PROJ_CELL(12).x_ = -PROJ_X2;
    PROJ_CELL(12).y_ = -PROJ_Y2;
    PROJ_CELL(12).z_ = -PROJ_Z2 << 8;
    PROJ_CELL(13).x_ = PROJ_X2;
    PROJ_CELL(13).y_ = PROJ_Y2;
    PROJ_CELL(13).z_ = PROJ_Z2 << 8;
    PROJ_CELL(14).x_ = PROJ_X1 - PROJ_X2;
    PROJ_CELL(14).y_ = PROJ_Y1 - PROJ_Y2;
    PROJ_CELL(14).z_ = (PROJ_Z1 - PROJ_Z2) << 8;
    PROJ_CELL(15).x_ = PROJ_X1;
    PROJ_CELL(15).y_ = PROJ_Y1;
    PROJ_CELL(15).z_ = PROJ_Z1 << 8;
    PROJ_CELL(16).x_ = PROJ_X1 + PROJ_X2;
    PROJ_CELL(16).y_ = PROJ_Y1 + PROJ_Y2;
    PROJ_CELL(16).z_ = (PROJ_Z1 + PROJ_Z2) << 8;
    PROJ_CELL(17).x_ = PROJ_X0 - PROJ_X1 - PROJ_X2;
    PROJ_CELL(17).y_ = PROJ_Y0 - PROJ_Y1 - PROJ_Y2;
    PROJ_CELL(17).z_ = (PROJ_Z0 - PROJ_Z2 - PROJ_Z1) << 8;
    PROJ_CELL(18).x_ = PROJ_X0 - PROJ_X1;
    PROJ_CELL(18).y_ = PROJ_Y0 - PROJ_Y1;
    PROJ_CELL(18).z_ = (PROJ_Z0 - PROJ_Z1) << 8;
    PROJ_CELL(19).x_ = -PROJ_X1 + PROJ_X2 + PROJ_X0;
    PROJ_CELL(19).y_ = PROJ_Y0 + PROJ_Y2 - PROJ_Y1;
    PROJ_CELL(19).z_ = (PROJ_Z2 - PROJ_Z1 + PROJ_Z0) << 8;
    PROJ_CELL(20).x_ = PROJ_X0 - PROJ_X2;
    PROJ_CELL(20).y_ = PROJ_Y0 - PROJ_Y2;
    PROJ_CELL(20).z_ = (PROJ_Z0 - PROJ_Z2) << 8;
    PROJ_CELL(21).x_ = PROJ_X0;
    PROJ_CELL(21).y_ = PROJ_Y0;
    PROJ_CELL(21).z_ = PROJ_Z0 << 8;
    PROJ_CELL(22).x_ = PROJ_X0 + PROJ_X2;
    PROJ_CELL(22).y_ = PROJ_Y0 + PROJ_Y2;
    PROJ_CELL(22).z_ = (PROJ_Z0 + PROJ_Z2) << 8;
    PROJ_CELL(23).x_ = PROJ_X0 + PROJ_X1 - PROJ_X2;
    PROJ_CELL(23).y_ = PROJ_Y0 + PROJ_Y1 - PROJ_Y2;
    PROJ_CELL(23).z_ = (PROJ_Z1 - PROJ_Z2 + PROJ_Z0) << 8;
    PROJ_CELL(24).x_ = PROJ_X0 + PROJ_X1;
    PROJ_CELL(24).y_ = PROJ_Y0 + PROJ_Y1;
    PROJ_CELL(24).z_ = (PROJ_Z0 + PROJ_Z1) << 8;
    PROJ_CELL(25).x_ = PROJ_X1 + PROJ_X2 + PROJ_X0;
    PROJ_CELL(25).y_ = PROJ_Y0 + PROJ_Y1 + PROJ_Y2;
    PROJ_CELL(25).z_ = (PROJ_Z0 + PROJ_Z1 + PROJ_Z2) << 8;

    // The three unit cells as floats; fild reads the cells just stored.
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_ec_ = (float)PROJ_CELL(13).x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e0_ = (float)PROJ_CELL(13).y_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_fc_ = (float)PROJ_CELL(13).z_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_f0_ = (float)PROJ_CELL(15).x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e4_ = (float)PROJ_CELL(15).y_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_a00_ = (float)PROJ_CELL(15).z_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_f4_ = (float)PROJ_CELL(21).x_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_e8_ = (float)PROJ_CELL(21).y_;
    CaviarKernelProjectionWindow.current_views_.tail_.scratch_a04_ = (float)PROJ_CELL(21).z_;

    // 6409E3..640A38: a cell cursor (ecx, with edi = -&cells[0] for the
    // [edi+ecx+K] accesses) and a row cursor (esi), with the 26-trip counter
    // kept as a dec/jne slot. Of the 64 pointer/index mixes of the six
    // accesses only both cursors give the counter; an index loop lets VC6
    // test one shared byte offset instead (cmp eax, 0x1A0).
    CaviarProjectionCell *cell = &PROJ_CELL(0);
    CaviarProjectionOutputRowCandidate *row = CaviarOutputMessageWindow.projection_rows_;
    for (i = 0; i < 0x1A; i++, cell++, row++) {
        (*row).depth_offset_ =
            ((*cell).y_ >> 8) * (int32_t)vox_setup_copy_.depth_row_step_ +
            ((*cell).x_ >> 8);
        (*row).packed_x_ =
            ((*cell).x_ << 24) + ((*cell).y_ & 0xff);
        (*row).packed_z_ =
            (((uint32_t)(*cell).z_ >> 16) + ((*cell).z_ << 16)) & 0xff00ffff;
    }
}
#undef PROJ_CELL
#undef PROJ_Z2
#undef PROJ_Z1
#undef PROJ_Z0
#undef PROJ_Y2
#undef PROJ_Y1
#undef PROJ_Y0
#undef PROJ_X2
#undef PROJ_X1
#undef PROJ_X0

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
// symbol    _sub_6401b0
// size      818 bytes
// Deleted seed provenance: git:123267b4afb63f4fd3396772db0fcf89d35259c9:src/unrecovered/006401b0.cpp
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
// Rewritten from the image (2026-11-07). The 8.8 reduction rounds half away
// from zero: SETNS/SETGE, DEC and a clear-low-byte select +0x80 or -0x80.
// The extents narrow to signed short before the corner walk.
#define VOX_FIX8(v) ((((v) < 0 ? -0x80 : 0x80) + (v)) >> 8)
#define VOX_GROW(x, y) \
    if ((x) < out_rect[0]) out_rect[0] = (x); \
    if ((x) > out_rect[2]) out_rect[2] = (x); \
    if ((y) < out_rect[1]) out_rect[1] = (y); \
    if ((y) > out_rect[3]) out_rect[3] = (y)

extern "C" void __cdecl sub_6401b0(const short *centre, const short *extent,
                                   int *out_rect) {
    int x, y, t;
    short a[3], b[3], c[3];

    t = CaviarFixedPointProjection.word_00_ * centre[0] +
        CaviarFixedPointProjection.word_04_ * centre[2] +
        CaviarFixedPointProjection.word_02_ * centre[1];
    x = VOX_FIX8(t);
    t = CaviarFixedPointProjection.word_06_ * centre[0] +
        CaviarFixedPointProjection.word_0A_ * centre[2] +
        CaviarFixedPointProjection.word_08_ * centre[1];
    y = VOX_FIX8(t);
    t = extent[0] * CaviarFixedPointProjection.word_00_;
    a[0] = (short)VOX_FIX8(t);
    t = extent[0] * CaviarFixedPointProjection.word_06_;
    a[1] = (short)VOX_FIX8(t);
    t = extent[1] * CaviarFixedPointProjection.word_02_;
    b[0] = (short)VOX_FIX8(t);
    t = extent[1] * CaviarFixedPointProjection.word_08_;
    b[1] = (short)VOX_FIX8(t);
    t = extent[2] * CaviarFixedPointProjection.word_04_;
    c[0] = (short)VOX_FIX8(t);
    t = extent[2] * CaviarFixedPointProjection.word_0A_;
    c[1] = (short)VOX_FIX8(t);

    out_rect[4] = 0;
    out_rect[5] = 0;
    x += a[0];
    y += a[1];
    out_rect[0] = 0x7fff;
    out_rect[1] = 0x7fff;
    out_rect[2] = -0x7fff;
    out_rect[3] = -0x7fff;
    VOX_GROW(x, y);
    x += b[0];
    y += b[1];
    VOX_GROW(x, y);
    x -= a[0];
    y -= a[1];
    VOX_GROW(x, y);
    y -= b[1];
    x -= b[0];
    VOX_GROW(x, y);
    x += c[0];
    y += c[1];
    VOX_GROW(x, y);
    x += a[0];
    y += a[1];
    VOX_GROW(x, y);
    x += b[0];
    y += b[1];
    VOX_GROW(x, y);
    x -= a[0];
    y -= a[1];
    VOX_GROW(x, y);

    out_rect[0] -= 2;
    out_rect[1] -= 2;
    out_rect[3] += 2;
    out_rect[2] += 2;
    if (out_rect[2] >= 0 && out_rect[3] >= 0) {
        out_rect[4] = out_rect[2] - out_rect[0] + 1;
        out_rect[5] = out_rect[3] - out_rect[1] + 1;
        return;
    }
    out_rect[4] = 0;
    out_rect[5] = 0;
}
#undef VOX_GROW
#undef VOX_FIX8


