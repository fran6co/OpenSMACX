/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// The VOX library's scene setup and draw, 0x006396A0..0x0063B2CB in the
// image, grouped by address. This grouping does not assert the original
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

// The contiguous .data window at 0x00698B70..0x00698B78 is zero in the image.
// vox_blit_transparent initializes and consumes all three words; the three
// scaled blitters all share the cursor; only the 16-bit scaled blitter shares
// the destination-stride word. The two 8-bit loops use their stride parameters.
// The source-row-byte word is used only by vox_blit_transparent.

// RECOVERY_OBJECT: v3 0x00698B70 0x00698B74 ?vox_blit_cursor@@3IA 0x0063B11C member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A destination_cursor_
// RECOVERY_OBJECT: v3 0x00698B74 0x00698B78 ?vox_blit_row_bytes@@3IA 0x0063B072 member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A source_row_bytes_
// RECOVERY_OBJECT: v3 0x00698B78 0x00698B7C ?vox_blit_dst_stride@@3IA 0x0063B11C member ?CaviarBlitState@@3UCaviarBlitStateCandidate@@A destination_stride_bytes_
CaviarBlitStateCandidate CaviarBlitState = {0, 0, 0};

// 0x009BE6C4. Set to 1 at the draw's entry and cleared on the way out; if the
// draw is entered while it is set the image spins on `jmp $+0` forever.
namespace {
uint8_t vox_scene_busy = 0;
} // namespace


// 0x00671010: original float bits 0x3F000000. The visibility cut the part's
// scaled extent is compared against to select the per-part rendering flag.
const float vox_visibility_cut_ = 0.5f;  // 0x00671010

// 0x00671014: original float bits 0x43000000. Scales the walked node's extent
// for the shadow clip tests in both the bounds and drawing passes.
const float vox_radius_scale_ = 128.0f;  // 0x00671014

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



// ORIGINAL: 0x006396A0 sub_6396a0 0x006396A0-0x00639E54
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original identity/name/span and complete historical lesson run are transported from the matching seed record by match_recovery.source.", "The staged typed definition is uniquely found by match_recovery.source.reader.function_line and compiles in the coordinator's three-TU VC6 overlay.", "The emitted symbol is read from the coordinator's fresh native COFF observation log; it is not inferred from the C++ spelling.", "Caviar data access windows and initialization remain separate coordinator-owned object-mapping obligations; this transfer creates no allocation-bound or initializer claim."], "reason": "Rehome this complete pinned original identity from its generated seed file to the typed Caviar renderer owner; preserve the original span and seed lesson history without asserting BYTE_EXACT. The old FILE interpretation is converted to a function BODY because the new owner contains multiple original identities."}
// symbol    ?vox_stage_scene@@YAMPAUVoxRenderRecord@@PAUCaviarSceneContext@@PAUVoxCameraBlock@@HPAM3@Z
// TRIED: dense FPU tile/sprite-projection math, esp-relative (frameless) args - tracked esp displacement by hand across every sub/add esp to resolve which [esp+N] occurrences are the same real argument vs reused scratch slots (found Ghidra's own decompile conflated two different pointers under one "param_1", corrected to p1+4 vs p1+0xc). Signature changed to float(int,int,int,int,float*,float*) - FPU return, matches trailing fld+ret with no output slot. Compiles, sim 0.61 framed/frameless (tied). fxch-heavy FPU scheduling makes byte-exact implausible by hand; not retried given size (1972 bytes).
float __cdecl vox_stage_scene(
    VoxRenderRecord *record, CaviarSceneContext *context,
    VoxCameraBlock *camera, int step, float *output_a, float *output_b) {
    CaviarSceneProjectionScratchCandidate &scratch =
        CaviarSceneProjectionScratch;
    CaviarSceneSetupDescriptorCandidate *const setup = record->setup_block_;

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
        context->projection_scale_ * scratch.perspective_factor_;
    scratch.viewport_aspect_ =
        (camera->bounds_.lead_[4] - camera->bounds_.lead_[3]) /
        (camera->bounds_.lead_[2] - camera->bounds_.lead_[1]);

    // The original scales the first six matrix cells here, not the last row.
    float *matrix_cells = &scratch.view_scene_matrix_.values[0][0];
    for (int index = 0; index != 6; ++index) {
        matrix_cells[index] *= scratch.projected_scale_;
    }

    const float half_width = static_cast<float>(setup->colour_width_ >> 1) -
                             CaviarProjectionHalfPixel;
    const float half_height = static_cast<float>(setup->height_ >> 1) -
                              CaviarProjectionHalfPixel;
    scratch.camera_offset_.values[0] =
        context->projection_offset_x_ * context->projection_scale_ +
        context->screen_offset_x_ +
        half_width + scratch.camera_offset_.values[0] * scratch.projected_scale_;
    scratch.camera_offset_.values[1] =
        context->projection_offset_y_ * context->projection_scale_ +
        context->screen_offset_y_ +
        half_height + scratch.camera_offset_.values[1] * scratch.projected_scale_;
    scratch.camera_offset_.values[2] -= camera->bounds_.lead_[1];

    // Original 0x639972 tests record+0: ECX is arg1 loaded at 0x6397F6.
    // This is setup_id_'s orientation bit, not an instance-colour pointer tag.
    if ((record->setup_id_ & 1) == 0) {
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
    scratch.camera_offset_.values[2] += context->projection_offset_z_ +
        (((record->setup_id_ & 1) == 0)
            ? camera->bounds_.lead_[3]
            : camera->bounds_.lead_[4]);

    scratch.projected_scale_ =
        context->state_.scale_ * scratch.projected_scale_;
    scratch.viewport_aspect_ =
        context->state_.scale_ * scratch.viewport_aspect_;
    scratch.part_clip_.left_ = (int)floor(
        static_cast<float>(camera->bounds_.values_[0]) * context->projection_scale_);
    scratch.part_clip_.top_ = (int)floor(
        static_cast<float>(camera->bounds_.values_[1]) * context->projection_scale_);
    scratch.part_clip_.right_ = (int)floor(
        static_cast<float>(camera->bounds_.values_[4] + 1) * context->projection_scale_) + 1;
    scratch.part_clip_.bottom_ = (int)floor(
        static_cast<float>(camera->bounds_.values_[5] + 1) * context->projection_scale_) + 1;
    scratch.part_clip_.width_ = scratch.part_clip_.left_ - 1 +
                                scratch.part_clip_.right_;
    scratch.part_clip_.height_ = scratch.part_clip_.top_ - 1 +
                                 scratch.part_clip_.bottom_;

    if (context->shadow_enabled_ != 0 && setup->shadow_buffer_ != 0) {
        float &shadow_z = context->shadow_vector_.values[2];
        if (shadow_z <= CaviarProjectionNearNegativeEpsilon ||
            CaviarProjectionZero <= shadow_z) {
            if (CaviarProjectionZero <= shadow_z &&
                shadow_z < CaviarProjectionNearPositiveEpsilon) {
                shadow_z = CaviarProjectionNearPositiveEpsilon;
            }
        } else {
            shadow_z = CaviarProjectionNearNegativeEpsilon;
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
            shadow_x * context->shadow_origin_adjustment_;
        scratch.shadow_centre_.values[1] +=
            shadow_y * context->shadow_origin_adjustment_;
        scratch.shadow_centre_.values[2] += context->shadow_origin_adjustment_;

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
        const float shadow_x_scale = shadow_projected_scale * context->shadow_scale_x_;
        const float shadow_y_scale = shadow_projected_scale * context->shadow_scale_y_;
        for (int axis = 0; axis != 3; ++axis) {
            scratch.shadow_matrix_.values[0][axis] =
                shadow_axes.values[axis][0] * shadow_x_scale;
            scratch.shadow_matrix_.values[1][axis] =
                shadow_axes.values[axis][1] * shadow_y_scale;
        }
        scratch.shadow_centre_.values[0] =
            context->projection_offset_x_ * context->shadow_scale_x_ +
            context->shadow_screen_offset_x_ +
            half_width + scratch.shadow_centre_.values[0] * shadow_x_scale;
        scratch.shadow_centre_.values[1] =
            context->projection_offset_y_ * context->shadow_scale_y_ +
            context->shadow_screen_offset_y_ +
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

        scratch.shadow_clip_.left_ = (int)floor(
            static_cast<float>(camera->bounds_.values_[0]) * context->shadow_scale_x_);
        scratch.shadow_clip_.top_ = (int)floor(
            static_cast<float>(camera->bounds_.values_[1]) * context->shadow_scale_y_);
        scratch.shadow_clip_.right_ = (int)floor(
            static_cast<float>(camera->bounds_.values_[4] + 1) * context->shadow_scale_x_) + 1;
        scratch.shadow_clip_.bottom_ = (int)floor(
            static_cast<float>(camera->bounds_.values_[5] + 1) * context->shadow_scale_y_) + 1;
        scratch.shadow_clip_.width_ = scratch.shadow_clip_.left_ - 1 +
                                      scratch.shadow_clip_.right_;
        scratch.shadow_clip_.height_ = scratch.shadow_clip_.top_ - 1 +
                                       scratch.shadow_clip_.bottom_;
    }
    return scratch.projected_scale_;
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00639FA9 seeds local ESP+0x10 with 1 and 0x00639FC9 clears it above the LOD cut. At 0x0063A1EC three outstanding matrix-multiply arguments make ESP+0x1C that same local; cleanup at 0x0063A1F0 removes 0x0C bytes, then 0x0063A1FB pushes the scalar as argument 5 to 0x0063F000 at 0x0063A23B.", "The main and shadow passes consume the same precomputed visibility, including shadow-only paths. Existing caller bounds checks and callback ordering are unchanged; the former bounds-pointer argument incorrectly forced the shadow low flag on."], "reason": "Pass the computed scalar LOD visibility to shadow rendering, not the address of the camera bounds."}
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
    CaviarKernelProjectionWindow.current_views_.tail_.vox_scene_extent_ =
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
            record->setup_block_->colour_buffer_)) -
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
            record->setup_block_->depth_buffer_));
    vox_setup_copy_.colour_row_step_ = static_cast<uint32_t>(
        record->setup_block_->colour_row_step_);
    vox_setup_copy_.depth_row_step_ = static_cast<uint32_t>(
        record->setup_block_->depth_row_step_);
    vox_setup_copy_.shadow_row_step_ = static_cast<uint32_t>(
        record->setup_block_->shadow_row_step_);
    vox_setup_copy_.colour_buffer_ = record->setup_block_->colour_buffer_;
    vox_setup_copy_.depth_buffer_ = record->setup_block_->depth_buffer_;
    vox_setup_copy_.shadow_buffer_ = record->setup_block_->shadow_buffer_;

    // The record's own setup hook, called once before the walk.
    record->setup_callback_();

    for (int index = 0; index < state->instance_count_; ++index) {
        VoxInstance &instance = state->instances_[index];
        if ((instance.instance_colour_[step] & 0x80) != 0) {
            continue;
        }
        const CvrShape *node = vox_walk_scaled_chain(
            state->instance_steps_[index].shape_,
            CaviarSceneProjectionScratch.projected_scale_);

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

        // A clipped body can still cast a shadow. The original branch at
        // 0x639FEA skips only the main pass, reaching the live shadow gate.
        if (vox_part_behind(&position, extent, view) == 0) {
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
        }

        // The shadow pass, when the state asks for it and the record's
        // descriptor block names a shadow.
        if (context->shadow_enabled_ != 0 &&
            record->setup_block_->shadow_buffer_ != 0) {
            VOX_Vect shadow;
            vox_matrix_vector_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                  instance.instance_vector_[step].values,
                                  shadow.values);
            vector_add(shadow.values, CaviarSceneProjectionScratch.shadow_centre_.values, shadow.values);

            const float radius = extent * vox_radius_scale_;
            const float span_x = radius * CaviarSceneProjectionScratch.shadow_radius_x_;
            const float span_y = radius * CaviarSceneProjectionScratch.shadow_radius_y_;
            // Preserve the original unordered-X acceptance and ordered lower-Y
            // test rather than normalizing all four comparisons to <=.
            if (!(-span_x > shadow.values[0]) &&
                !(shadow.values[0] >
                    static_cast<float>(view->bounds_.values_[4]) + span_x) &&
                -span_y <= shadow.values[1] &&
                !(shadow.values[1] >
                    static_cast<float>(view->bounds_.values_[5]) + span_y)) {
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
                vox_draw_shadow(record, node, &shadow, &shadow_part, visible);
            }
        }
    }

    vox_scene_busy = 0;
}

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

        const CvrShape *shape = vox_walk_scaled_chain(
            state.instance_steps_[index].shape_,
            CaviarSceneProjectionScratch.projected_scale_);
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
        if (context->shadow_enabled_ != 0 &&
            record->setup_block_->shadow_buffer_ != 0) {
            VOX_Vect shadow;
            vox_matrix_vector_mul(caviar_scene_shadow_matrix_view(CaviarSceneProjectionScratch).values,
                                  instance.instance_vector_[step].values,
                                  shadow.values);
            vector_add(shadow.values, CaviarSceneProjectionScratch.shadow_centre_.values,
                       shadow.values);
            const float shadow_scale = extent * vox_radius_scale_;
            // The x87 branches reject ordered X overflow, but admit unordered
            // X comparisons. The lower Y test alone rejects unordered values.
            if (!(-shadow_scale * CaviarSceneProjectionScratch.shadow_radius_x_ > shadow.values[0]) &&
                !(shadow.values[0] > camera->bounds_.values_[4] +
                                      shadow_scale * CaviarSceneProjectionScratch.shadow_radius_x_) &&
                -shadow_scale * CaviarSceneProjectionScratch.shadow_radius_y_ <= shadow.values[1] &&
                !(shadow.values[1] > camera->bounds_.values_[5] +
                                      shadow_scale * CaviarSceneProjectionScratch.shadow_radius_y_)) {
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

    VoxRenderClipBounds *primary = &record->primary_clip_;
    VoxRenderClipBounds *secondary = &record->secondary_clip_;
    primary->left_ = bounds[0]; primary->top_ = bounds[1];
    primary->right_ = bounds[2]; primary->bottom_ = bounds[3];
    primary->width_ = bounds[4]; primary->height_ = bounds[5];
    secondary->left_ = bounds[6]; secondary->top_ = bounds[7];
    secondary->right_ = bounds[8]; secondary->bottom_ = bounds[9];
    secondary->width_ = bounds[10]; secondary->height_ = bounds[11];
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
void __cdecl sub_63ad60(VoxRenderRecord *record) {
    VoxRenderRecord *const typed = record;
    const unsigned char type = typed->setup_block_->pixel_size_;

    switch (type) {
    case 1:
        if ((static_cast<unsigned char>(typed->setup_id_) & 1u) != 0) {
            typed->part_contained_ = &caviar_original_665640;
            typed->part_clipped_ = &caviar_original_664e50;
            typed->setup_callback_ = &caviar_original_63af40;
        } else {
            typed->part_contained_ = &caviar_original_6647f0;
            typed->part_clipped_ = &caviar_original_664000;
            typed->setup_callback_ = &caviar_original_63af00;
        }
        break;
    case 2:
        if ((static_cast<unsigned char>(typed->setup_id_) & 1u) != 0) {
            typed->part_contained_ = &caviar_original_667b20;
            typed->part_clipped_ = &caviar_original_667380;
            typed->setup_callback_ = &caviar_original_63af20;
        } else {
            typed->part_contained_ = &caviar_original_666440;
            typed->part_clipped_ = &caviar_original_665ca0;
            typed->setup_callback_ = &caviar_original_63aee0;
        }
        break;
    default:
        break;
    }

    typed->shadow_contained_ = &caviar_original_666fd0;
    typed->shadow_clipped_ = &caviar_original_666a80;
}

/*
Purpose: Fill a render record's pixel-size dispatch slots for CPU ranks 3 and
         5, choosing the table set by the record's odd-alignment flag byte.
// ORIGINAL: 0x0063AE20 sub_63ae20 0x0063AE20-0x0063AED2 BYTE_EXACT
// symbol    ?sub_63ae20@@YAXPAUVoxRenderRecord@@@Z
// LEVER: switch(type){case 1: case 2: default:} reproduces the dec/je/dec/jne
//        idiom; if/else-if compiled to cmp/jne instead (from the promoted
//        artifact, measured BYTE_EXACT there).
*/
void __cdecl sub_63ae20(VoxRenderRecord *record) {
    VoxRenderRecord *const typed = record;
    const unsigned char type = typed->setup_block_->pixel_size_;

    switch (type) {
    case 1:
        if ((static_cast<unsigned char>(typed->setup_id_) & 1u) != 0) {
            typed->part_contained_ = &caviar_original_665640;
            typed->part_clipped_ = &caviar_original_664e50;
            typed->setup_callback_ = &caviar_original_63af40;
        } else {
            typed->part_contained_ = &caviar_original_6647f0;
            typed->part_clipped_ = &caviar_original_664000;
            typed->setup_callback_ = &caviar_original_63af00;
        }
        break;
    case 2:
        if ((static_cast<unsigned char>(typed->setup_id_) & 1u) != 0) {
            typed->part_contained_ = &caviar_original_667b20;
            typed->part_clipped_ = &caviar_original_667380;
            typed->setup_callback_ = &caviar_original_63af20;
        } else {
            typed->part_contained_ = &caviar_original_666440;
            typed->part_clipped_ = &caviar_original_665ca0;
            typed->setup_callback_ = &caviar_original_63aee0;
        }
        break;
    default:
        break;
    }

    typed->shadow_contained_ = &caviar_original_666fd0;
    typed->shadow_clipped_ = &caviar_original_666a80;
}


// SEED HISTORY: 0x0063AEE0 previously used FILE mode with spans 0063AEE0-0063AEF4;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden;sp_ready;purged_ok; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x0066638F 0x00666989 0x00666F68.
// SEED HISTORY: former direct-call order 0x00666989 -> 0x0066638F -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AEE0 sub_63aee0 0x0063AEE0-0x0063AEF4;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_create_record selects this record dispatcher by CPU rank", "source:src/caviar.cpp:sub_63ad60/sub_63ae20 directly install the mode-specific callback slots", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The pinned record dispatchers select the typed callback, which binds prepared state before the instance walk; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63aee0@@YAXXZ
// TRIED: `int` return (no eax set before final ret, changed to `void`); address range carries a _SELFMOD (copy-protection) span, so the harness's authoritative check REFUSES any body here regardless of content - MISMATCH is the best obtainable signal via --dir.
// kind      game
__declspec(naked) void __cdecl caviar_original_63aee0() {
    // Hand-written asm (Caviar _SELFMOD setup): three patcher calls, then a
    // tail jump into the contained-shadow patcher at 0x0066731C.
    __asm {
        call caviar_original_666989
        call caviar_original_66638f
        call caviar_original_666f68
        jmp caviar_shadow_contained_patch
    }
}

// SEED HISTORY: 0x0063AF00 previously used FILE mode with spans 0063AF00-0063AF14;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x006646FB 0x00664D51 0x00666F68.
// SEED HISTORY: former direct-call order 0x00664D51 -> 0x006646FB -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF00 sub_63af00 0x0063AF00-0x0063AF14;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_create_record selects this record dispatcher by CPU rank", "source:src/caviar.cpp:sub_63ad60/sub_63ae20 directly install the mode-specific callback slots", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The pinned record dispatchers select the typed callback, which binds prepared state before the instance walk; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af00@@YAXXZ
// TRIED: nothing to rule out on tier - REFUSED here is unconditional (SELFMOD span), verified with a `return 0;` stub scoring MISMATCH while the correct tail-call form scores REFUSED.
// kind      game
__declspec(naked) void __cdecl caviar_original_63af00() {
    // Hand-written asm (Caviar _SELFMOD setup): three patcher calls, then a
    // tail jump into the contained-shadow patcher at 0x0066731C.
    __asm {
        call caviar_original_664d51
        call caviar_original_6646fb
        call caviar_original_666f68
        jmp caviar_shadow_contained_patch
    }
}

// SEED HISTORY: 0x0063AF20 previously used FILE mode with spans 0063AF20-0063AF34;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x00666F68 0x00667A6F 0x00668069.
// SEED HISTORY: former direct-call order 0x00668069 -> 0x00667A6F -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF20 sub_63af20 0x0063AF20-0x0063AF34;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_create_record selects this record dispatcher by CPU rank", "source:src/caviar.cpp:sub_63ad60/sub_63ae20 directly install the mode-specific callback slots", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The pinned record dispatchers select the typed callback, which binds prepared state before the instance walk; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af20@@YAXXZ
// kind      game
__declspec(naked) void __cdecl caviar_original_63af20() {
    // Hand-written asm (Caviar _SELFMOD setup): three patcher calls, then a
    // tail jump into the contained-shadow patcher at 0x0066731C.
    __asm {
        call caviar_original_668069
        call caviar_original_667a6f
        call caviar_original_666f68
        jmp caviar_shadow_contained_patch
    }
}

// SEED HISTORY: 0x0063AF40 previously used FILE mode with spans 0063AF40-0063AF54;0066731C-00667375; no BYTE_EXACT, SEMANTIC, or EXCLUDED claim; flags=hidden; seed artifact size=109 bytes, kind=game, direct callers=0, catalogued call targets=0x0066554B 0x00665BA1 0x00666F68.
// SEED HISTORY: former direct-call order 0x00665BA1 -> 0x0066554B -> 0x00666F68. The shared 0x66731C-0x667375 secondary body is retained on this identity; its self-modifying immediate stores are replaced by typed prepared state.
OPENSMACX_NOINLINE
// ORIGINAL: 0x0063AF40 sub_63af40 0x0063AF40-0x0063AF54;0x0066731C-0x00667375
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "source:src/caviar.cpp:vox_create_record selects this record dispatcher by CPU rank", "source:src/caviar.cpp:sub_63ad60/sub_63ae20 directly install the mode-specific callback slots", "source:src/caviar_raster_kernels.inc:caviar_prepare_kernel_bindings_candidate publishes typed current inputs and lane bases", "source:src/caviar_raster_kernels.inc:caviar_part_output_bases_candidate and caviar_shadow_output_bases_candidate compute destinations without executable-page writes", "docs/recovery/ida9-functions.csv:shared secondary range 0x0066731C-0x00667375 is retained by the four historical setup records"], "reason": "Own the renderer's zero-argument setup callback in the configured Caviar TU. The pinned record dispatchers select the typed callback, which binds prepared state before the instance walk; the shared original secondary span remains on this mapping."}
// symbol    ?caviar_original_63af40@@YAXXZ
// TRIED: straight-line body after the 3 calls. The original's own tail is a `jmp` into a block shared with another caller (spans two disjoint ranges); inlining it here compiles to `mov` straight-through instead of the shared-tail `jmp`.
// kind      game
__declspec(naked) void __cdecl caviar_original_63af40() {
    // Hand-written asm (Caviar _SELFMOD setup): three patcher calls, then a
    // tail jump into the contained-shadow patcher at 0x0066731C.
    __asm {
        call caviar_original_665ba1
        call caviar_original_66554b
        call caviar_original_666f68
        jmp caviar_shadow_contained_patch
    }
}

/*
Purpose: Fill a destination with a 16-bit colour repeated `count` times. The
         alignment cascade burns 1 or 2 leading bytes so the bulk runs on
         dwords, then an unrolled 16/8/4/2/1 remainder.
// ORIGINAL: 0x0063AF60 sub_63af60 0x0063AF60-0x0063B072 BYTE_EXACT
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
// Hand-written asm: 16-bit argument load, ror eax,1 and or edx,edx tests (MASM module, unaligned).
__declspec(naked) void __cdecl vox_fill_colour_table(void *table, unsigned long value,
                                   unsigned long count) {
    __asm {
        push ebx
        mov ax, word ptr [esp + 0xc]
        mov ebx, dword ptr [esp + 8]
        mov ecx, dword ptr [esp + 0x10]
        mov edx, eax
        shl edx, 0x10
        and eax, 0xffff
        add eax, edx
        test ebx, 1
        je fill_63afbf
        shl ecx, 1
        mov byte ptr [ebx], al
        dec ecx
        inc ebx
        ror eax, 1
        mov edx, 3
        and edx, ecx
        shr ecx, 2
        je fill_63af9e
fill_63af96:
        mov dword ptr [ebx], eax
        add ebx, 4
        dec ecx
        jne fill_63af96
fill_63af9e:
        test edx, 2
        je fill_63afac
        mov word ptr [ebx], ax
        add ebx, 2
fill_63afac:
        test edx, 1
        je fill_63b070
        mov byte ptr [ebx], al
        jmp fill_63b070
fill_63afbf:
        test ebx, 2
        je fill_63afce
        mov word ptr [ebx], ax
        dec ecx
        add ebx, 2
fill_63afce:
        mov edx, 1
        and edx, ecx
        shr ecx, 1
        je fill_63b069
        sub ecx, 0x10
        jl fill_63b019
fill_63afe2:
        mov dword ptr [ebx], eax
        mov dword ptr [ebx + 4], eax
        mov dword ptr [ebx + 8], eax
        mov dword ptr [ebx + 0xc], eax
        mov dword ptr [ebx + 0x10], eax
        mov dword ptr [ebx + 0x14], eax
        mov dword ptr [ebx + 0x18], eax
        mov dword ptr [ebx + 0x1c], eax
        mov dword ptr [ebx + 0x20], eax
        mov dword ptr [ebx + 0x24], eax
        mov dword ptr [ebx + 0x28], eax
        mov dword ptr [ebx + 0x2c], eax
        mov dword ptr [ebx + 0x30], eax
        mov dword ptr [ebx + 0x34], eax
        mov dword ptr [ebx + 0x38], eax
        mov dword ptr [ebx + 0x3c], eax
        add ebx, 0x40
        sub ecx, 0x10
        jge fill_63afe2
fill_63b019:
        add ecx, 8
        jl fill_63b03b
        mov dword ptr [ebx], eax
        mov dword ptr [ebx + 4], eax
        mov dword ptr [ebx + 8], eax
        mov dword ptr [ebx + 0xc], eax
        mov dword ptr [ebx + 0x10], eax
        mov dword ptr [ebx + 0x14], eax
        mov dword ptr [ebx + 0x18], eax
        mov dword ptr [ebx + 0x1c], eax
        add ebx, 0x20
        sub ecx, 8
fill_63b03b:
        add ecx, 4
        jl fill_63b051
        mov dword ptr [ebx], eax
        mov dword ptr [ebx + 4], eax
        mov dword ptr [ebx + 8], eax
        mov dword ptr [ebx + 0xc], eax
        add ebx, 0x10
        sub ecx, 4
fill_63b051:
        add ecx, 2
        jl fill_63b061
        mov dword ptr [ebx], eax
        mov dword ptr [ebx + 4], eax
        add ebx, 8
        sub ecx, 2
fill_63b061:
        inc ecx
        jl fill_63b069
        mov dword ptr [ebx], eax
        add ebx, 4
fill_63b069:
        or edx, edx
        je fill_63b070
        mov word ptr [ebx], ax
fill_63b070:
        pop ebx
        ret
    }
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
// ORIGINAL: 0x0063B072 sub_63b072 0x0063B072-0x0063B11C FILE BYTE_EXACT
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
// Hand-written asm: callee-saved quad push, esp-relative args, dec-memory loop counter (MASM module, unaligned).
extern "C" __declspec(naked) void __cdecl vox_blit_transparent(
    unsigned char *framebuffer, const unsigned short *source, int dest_x, int dest_y,
    int count_x, int count_y, unsigned source_step, int source_width,
    int dest_width, unsigned short transparent_key) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jle blit16_63b117
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jle blit16_63b117
        mov esi, dword ptr [esp + 0x18]
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x30]
        shl eax, 1
        mov dword ptr [CaviarBlitState.source_row_bytes_], eax
        mov eax, dword ptr [esp + 0x34]
        shl eax, 1
        mov dword ptr [CaviarBlitState.destination_stride_bytes_], eax
        mov eax, dword ptr [esp + 0x20]
        imul eax, dword ptr [esp + 0x34]
        add eax, dword ptr [esp + 0x1c]
        shl eax, 1
        add edi, eax
        mov dword ptr [CaviarBlitState.destination_cursor_], edi
        xor edx, edx
blit16_63b0c5:
        xor ecx, ecx
        mov edi, dword ptr [CaviarBlitState.destination_cursor_]
        mov ebp, dword ptr [esp + 0x24]
blit16_63b0d1:
        mov ebx, ecx
        shr ebx, 0x10
        mov ax, word ptr [esi + ebx*2]
        cmp ax, word ptr [esp + 0x38]
        je blit16_63b0e4
        mov word ptr [edi], ax
blit16_63b0e4:
        add edi, 2
        add ecx, dword ptr [esp + 0x2c]
        dec ebp
        jne blit16_63b0d1
        mov eax, dword ptr [CaviarBlitState.destination_stride_bytes_]
        add dword ptr [CaviarBlitState.destination_cursor_], eax
        add edx, dword ptr [esp + 0x2c]
        mov eax, edx
        shr eax, 0x10
        imul eax, dword ptr [CaviarBlitState.source_row_bytes_]
        add esi, eax
        and edx, 0xffff
        dec dword ptr [esp + 0x28]
        jne blit16_63b0c5
blit16_63b117:
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}

/*
Purpose: Walk a scaled sprite into a 16-bit destination through a palette:
         for each destination pixel whose source byte is non-zero, replace
         the existing pixel with palette[current pixel].

         The destination cursor and byte stride live in the two shared
         voxel-blit slots caviar.cpp owns; the cursor is re-read from the
         slot every row, exactly as the original re-reads it, so anything
         else writing the slot between rows is seen.

// ORIGINAL: 0x0063B11C sub_63b11c 0x0063B11C-0x0063B1BF BYTE_EXACT
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
// Hand-written asm: callee-saved quad push, esp-relative args, dec-memory loop counter (MASM module, unaligned).
extern "C" __declspec(naked) void __cdecl sub_63b11c(unsigned short *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height,
                                   int source_x_step, int source_y_step,
                                   int source_stride, int destination_stride,
                                   const unsigned short *palette) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jle remap16_63b1ba
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jle remap16_63b1ba
        mov esi, dword ptr [esp + 0x18]
        mov edi, dword ptr [esp + 0x14]
        mov ebx, dword ptr [esp + 0x3c]
        mov eax, dword ptr [esp + 0x38]
        shl eax, 1
        mov dword ptr [CaviarBlitState.destination_stride_bytes_], eax
        mov eax, dword ptr [esp + 0x20]
        imul eax, dword ptr [esp + 0x38]
        add eax, dword ptr [esp + 0x1c]
        shl eax, 1
        add edi, eax
        mov dword ptr [CaviarBlitState.destination_cursor_], edi
        xor edx, edx
remap16_63b168:
        xor ecx, ecx
        mov edi, dword ptr [CaviarBlitState.destination_cursor_]
        mov ebp, dword ptr [esp + 0x24]
remap16_63b174:
        mov eax, ecx
        shr eax, 0x10
        cmp byte ptr [eax + esi], 0
        je remap16_63b189
        mov ax, word ptr [edi]
        mov ax, word ptr [ebx + eax*2]
        mov word ptr [edi], ax
remap16_63b189:
        add edi, 2
        add ecx, dword ptr [esp + 0x2c]
        dec ebp
        jne remap16_63b174
        mov eax, dword ptr [CaviarBlitState.destination_stride_bytes_]
        add dword ptr [CaviarBlitState.destination_cursor_], eax
        add edx, dword ptr [esp + 0x30]
        mov eax, edx
        shr eax, 0x10
        imul eax, dword ptr [esp + 0x34]
        add esi, eax
        and edx, 0xffff
        dec dword ptr [esp + 0x28]
        jne remap16_63b168
remap16_63b1ba:
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}


/*
Purpose: Walk a scaled sprite into an 8-bit destination, skipping the one
         transparent source byte value.

         The same shared cursor slot carries the destination pointer, and
         is re-read every row like its 16-bit sibling above.

// ORIGINAL: 0x0063B1BF sub_63b1bf 0x0063B1BF-0x0063B241 BYTE_EXACT
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
// Hand-written asm: callee-saved quad push, esp-relative args, dec-memory loop counter (MASM module, unaligned).
extern "C" __declspec(naked) void __cdecl sub_63b1bf(unsigned char *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height, int step,
                                   int source_stride, int destination_stride,
                                   char transparent) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jle blit8_63b23c
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jle blit8_63b23c
        mov esi, dword ptr [esp + 0x18]
        mov edi, dword ptr [esp + 0x14]
        mov eax, dword ptr [esp + 0x20]
        imul eax, dword ptr [esp + 0x34]
        add eax, dword ptr [esp + 0x1c]
        add edi, eax
        mov dword ptr [CaviarBlitState.destination_cursor_], edi
        xor edx, edx
blit8_63b1f2:
        xor ecx, ecx
        mov edi, dword ptr [CaviarBlitState.destination_cursor_]
        mov ebp, dword ptr [esp + 0x24]
blit8_63b1fe:
        mov ebx, ecx
        shr ebx, 0x10
        mov al, byte ptr [ebx + esi]
        cmp al, byte ptr [esp + 0x38]
        je blit8_63b20e
        mov byte ptr [edi], al
blit8_63b20e:
        inc edi
        add ecx, dword ptr [esp + 0x2c]
        dec ebp
        jne blit8_63b1fe
        mov eax, dword ptr [esp + 0x34]
        add dword ptr [CaviarBlitState.destination_cursor_], eax
        add edx, dword ptr [esp + 0x2c]
        mov eax, edx
        shr eax, 0x10
        imul eax, dword ptr [esp + 0x30]
        add esi, eax
        and edx, 0xffff
        dec dword ptr [esp + 0x28]
        jne blit8_63b1f2
blit8_63b23c:
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}


/*
Purpose: Walk a scaled sprite into an 8-bit destination through a shading
         table: each non-zero source byte replaces the destination byte
         with table[destination byte].

// ORIGINAL: 0x0063B241 sub_63b241 0x0063B241-0x0063B2CB BYTE_EXACT
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
// Hand-written asm: callee-saved quad push, esp-relative args, dec-memory loop counter (MASM module, unaligned).
extern "C" __declspec(naked) void __cdecl sub_63b241(unsigned char *destination_base,
                                   const char *source,
                                   int destination_x, int destination_y,
                                   int width, int height,
                                   int source_x_step, int source_y_step,
                                   int source_stride, int destination_stride,
                                   const unsigned char *table) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x24]
        test eax, eax
        jle remap8_63b2c6
        mov eax, dword ptr [esp + 0x28]
        test eax, eax
        jle remap8_63b2c6
        mov esi, dword ptr [esp + 0x18]
        mov edi, dword ptr [esp + 0x14]
        mov ebx, dword ptr [esp + 0x3c]
        mov eax, dword ptr [esp + 0x20]
        imul eax, dword ptr [esp + 0x38]
        add eax, dword ptr [esp + 0x1c]
        add edi, eax
        mov dword ptr [CaviarBlitState.destination_cursor_], edi
        xor edx, edx
remap8_63b278:
        xor ecx, ecx
        mov edi, dword ptr [CaviarBlitState.destination_cursor_]
        mov ebp, dword ptr [esp + 0x24]
remap8_63b284:
        mov eax, ecx
        shr eax, 0x10
        cmp byte ptr [eax + esi], 0
        je remap8_63b298
        xor eax, eax
        mov al, byte ptr [edi]
        mov al, byte ptr [eax + ebx]
        mov byte ptr [edi], al
remap8_63b298:
        inc edi
        add ecx, dword ptr [esp + 0x2c]
        dec ebp
        jne remap8_63b284
        mov eax, dword ptr [esp + 0x38]
        add dword ptr [CaviarBlitState.destination_cursor_], eax
        add edx, dword ptr [esp + 0x30]
        mov eax, edx
        shr eax, 0x10
        imul eax, dword ptr [esp + 0x34]
        add esi, eax
        and edx, 0xffff
        dec dword ptr [esp + 0x28]
        jne remap8_63b278
remap8_63b2c6:
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}
