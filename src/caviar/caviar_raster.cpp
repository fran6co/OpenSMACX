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

#include <stddef.h>
#include <string.h>
#include "caviar_projection_state.h"

static const char vox_msg_no_address_tables[] = "No room for address tables";



// Native 63F86C/63ECF3/63F344: a record's bit 0 selects either three bytes
// per item or a byte extent stored at the payload start. That dword remains
// part of the callback payload and need not be aligned. No padding is added.
static uint32_t caviar_raster_payload_extent(const CvrRasterItemHeaderCandidate *item) {
    if ((item->flags_ & 1) == 0) return item->item_count_ * 3u;
    uint32_t extent;
    memcpy(&extent, item->inline_payload_, sizeof(extent));
    return extent;
}


#include "caviar_raster_kernels.inc"


// Caviar projection and scaled sprite owner bodies. Their shared state is
// defined above in this translation unit; original callers and formats stay
// distinct. Exact x87 exceptional behavior and indirect blitter reachability
// remain explicit restoration obligations.


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
        const uint32_t extent = caviar_raster_payload_extent(item);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}


// 0x0063F000. Five arguments: the shadow branch's counterpart of the above.
OPENSMACX_NOINLINE
// SEED HISTORY: archived draw seed mapping used FILE mode; current source ownership is a BODY mapping.
// ORIGINAL: 0x0063F000 sub_63f000 0x0063F000-0x0063F5D7
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x0063F000 reserves 0x58 bytes and saves four registers; after helper-argument cleanup at 0x0063F026, load 0x0063F0C1 from ESP+0x7C is entry ESP+0x14, the fifth cdecl argument. XOR EDI,EDI at 0x0063F02B and CMP ESI,EDI / SETNE BL at 0x0063F0CB / 0x0063F0D5 establish 0/1 normalization.", "The original caller supplies its scalar LOD local, not camera bounds. Flat/tiled and contained/clipped paths preserve this low bit while independently setting 0x10/0x20 flags. This corrects a source type/value interpretation without changing original identities, layout or quirks."], "reason": "Interpret the fifth shadow argument as scalar visibility and normalize its nonzero value into the dispatch low bit."}
// symbol    ?vox_draw_shadow@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@H@Z
// kind      game
// TRIED: 5-int __cdecl signature transcribed faithfully from the two branches (bit2==0 single-shot vs bit2-set do/while loop over pbVar6); prologue diverges at instr #0 (sub esp,0x58 vs push) - not chased further, large FP-heavy function, landed for coverage.
void __cdecl vox_draw_shadow(
    VoxRenderRecord *record, const CvrShape *shape,
    const VOX_Vect *position, VOX_Matrix *part_matrix,
    int visible) {
    CaviarSceneProjectionScratchCandidate &scratch = CaviarSceneProjectionScratch;
    int clip[4];
    int flags = visible != 0;
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
        const uint32_t extent = caviar_raster_payload_extent(item);
        item = reinterpret_cast<const CvrRasterItemHeaderCandidate *>(
            item->inline_payload_ + extent);
    } while (visited_items < shape->item_count_);
}

// ORIGINAL: 0x0063F5E0 sub_63f5e0 0x0063F5E0-0x0063F9A8
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?caviar_clip_shape@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PBUVOX_Matrix@@PBUCaviarRenderClipWindowCandidate@@QAH@Z
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
            const uint32_t payload_size = caviar_raster_payload_extent(item);
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
// Deleted seed provenance: git:1ed59304110fc2098fb1dd11457fa5b0074169bd:src/unrecovered/0063fa80.cpp
Status: the four .rdata words stay bindings rather than literals. The
        words they feed are named from
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
    return (int)(value + (value < CaviarProjectionRoundThreshold
                              ? CaviarProjectionRoundBelow
                              : CaviarProjectionRoundAbove));
}
}  // namespace

extern "C" void __cdecl sub_63fa80(const VOX_Matrix *sources) {
    // Two walking pointers; VC5 itself forms the image's one moving pointer
    // plus fixed delta.
    const float *cursor = &sources->values[0][0];
    float *out = CaviarProjectionScaledMatrix;
    for (int i = 0; i < 9; ++i) {
        *out++ = *cursor++ * CaviarProjectionDirectionScale;
    }

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

    vox_matrix_inverse(part_matrix->values, vox_projection_matrix_.values);
    vox_projection_render_stage_ = 1;

    VOX_Vect camera_direction;
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

    double polar_angle = CaviarProjectionDirectionCandidate.values[2] >= CaviarProjectionAngleLimit
        ? CaviarProjectionRoundThreshold
        : acos(CaviarProjectionDirectionCandidate.values[2]);

    double azimuth_angle = atan2(CaviarProjectionDirectionCandidate.values[1],
                                 CaviarProjectionDirectionCandidate.values[0]);
    if (azimuth_angle < CaviarProjectionRoundThreshold) {
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


// ORIGINAL: 0x006404F0 sub_6404f0 0x006404F0-0x00640A42
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry and seed span transported by the native source writer; current symbol observed from the VC6 source overlay.", "The four-helper closure uses real shared Caviar projection storage and existing vector/matrix owners, not executable-address aliases.", "The projection builder at 0x640A27 advances its cursor before storing: packed rows begin at 0x9C0BA0, with fields at offsets0/4/8 and stride16, disjoint from callbacks at 0x9C0B80..0x9C0B94. This supersedes the earlier overlapping-output interpretation.", "Source arrays describe accessed windows, not proven original allocation bounds or original class names; remaining draw/setup callers are still unfinished."], "reason": "Complete typed Caviar helper in the existing renderer owner; preserve native branch/store order and historical restoration obligations."}
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

    CaviarProjectionOutputRowCandidate *output =
        CaviarOutputMessageWindow.projection_rows_;
    int localCount = 0x1a;
    CaviarProjectionCell *piVar2 = CaviarKernelProjectionWindow.current_views_.cells_;
    do {
        unsigned int uVar1 = (unsigned int)piVar2->y_;
        int recA = piVar2->x_;
        output->depth_offset_ = (((int)uVar1 >> 8) *
            (int32_t)vox_setup_copy_.depth_row_step_ + (recA >> 8));
        // 640A0D advances the input cursor before 640A27's packed-X store:
        // its first destination is 9C0BA0, leaving all six callbacks intact.
        output->packed_x_ =
            (uint32_t)recA * 0x01000000u + (uVar1 & 0xffu);
        output->packed_z_ =
            (((unsigned int)piVar2->z_ >> 0x10) +
            (unsigned int)piVar2->z_ * 0x10000) & 0xff00ffff;
        ++output;
        localCount = localCount - 1;
        ++piVar2;
    } while (localCount != 0);

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
// symbol    _sub_6401b0
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
// The Status paragraph above retains the retired seed's measurements. Current
// production uses /O2 /Ob2 /Z7 without /Oy-. The pinned reduction also has the
// sign-dependent bias recovered below; unconditional +0x80 was incomplete.
namespace {

// The original bias depends on the wrapped input's sign, before addition.
// Six extent results later narrow to signed short; both centers stay full32.
static int32_t caviar_i32_from_bits(uint32_t bits) {
    if (bits <= 0x7fffffffu) return (int32_t)bits;
    return -1 - (int32_t)(0xffffffffu - bits);
}

// SETNS/SETGE, DEC and clear-low-byte select +128 or -128. Preserve the
// unsigned addition even when that bias crosses the signed32 boundary.
static uint32_t caviar_sar8_bits(uint32_t bits) {
    const uint32_t bias = (int32_t)bits < 0 ? 0xffffff80u : 0x80u;
    const uint32_t biased = bits + bias;
    return (uint32_t)((int32_t)biased >> 8);
}

// Extent reductions are written through AX in the original. Center reductions
// use the full EAX/EDX value and must not pass through this low-word helper.
static int32_t caviar_fixed8_low16(uint32_t bits) {
    return (int16_t)caviar_sar8_bits(bits);
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


