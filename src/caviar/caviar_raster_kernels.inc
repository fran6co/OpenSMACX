
/* ---- source-owned scalar and shadow raster kernels ---- */
// Shared window types and parent globals are defined in caviar.h/caviar.cpp;
// fresh standalone CodeView/COFF resolves their v3 child paths, but not the
// original allocation bounds, lifetime, or final-linked placement.

// Seed provenance: build/recovery/next-game-coverage-oct5/product-closure-overlay-v6/src/caviar_scalar_kernel_candidate.inc
// Include after the Caviar renderer's real storage definitions. This code
// intentionally uses the current source owners (projection cells/output
// window, setup copies, active angle/palette and scene surfaces); the local
// struct is a non-owning invocation view, not another allocation or ABI type.
#include <float.h>
#include <limits.h>
#include <string.h>
#if !defined(_MSC_VER)
#include <fenv.h>
#endif
#include <math.h>


// The part and shadow renderers and their patchers are the _SELFMOD MASM
// modules (src/caviar_part_*.asm, src/caviar_shadow_*.asm); their former
// scalar C++ stand-ins are gone. Shared projection and raster helpers remain.
extern int32_t vox_projection_prepared_;
extern const float CaviarProjectionDirectionScale;
extern const float CaviarProjectionRoundThreshold;
extern const float CaviarProjectionRoundBelow;
extern const float CaviarProjectionRoundAbove;
extern "C" void __cdecl sub_63fa80(const VOX_Matrix *);
void __cdecl caviar_direction_to_projection_index(
    const VOX_Matrix *, const VOX_Vect *, float, const VOX_Matrix *);
void __cdecl sub_6401b0(const int16_t *, const int16_t *, int *);
void __cdecl caviar_build_projection_tables();
void __cdecl vox_direction_to_octant(const VoxRenderRecord *);
void __cdecl vox_inverse_direction_to_octant(const VOX_Matrix *);
static int caviar_box_is_inside(const int box[6], const int clip[4]);

static int caviar_fistp_qword_low_dword(float value) {
    // Source-level float-to-int conversion intentionally delegates to the
    // selected compiler's ordinary _ftol lowering, which must be measured
    // against 0x6458C8. It is not evidence for out-of-range, NaN, status,
    // precision, or unmasked-trap equivalence.
    return static_cast<int>(value);
}

static int caviar_round_projection_coordinate(float value) {
    const float scaled = value * CaviarProjectionDirectionScale;
    const float adjustment = scaled < CaviarProjectionRoundThreshold
        ? CaviarProjectionRoundBelow : CaviarProjectionRoundAbove;
    return caviar_fistp_qword_low_dword(scaled + adjustment);
}

static int caviar_truncate_projection_coordinate(float value) {
    return caviar_fistp_qword_low_dword(value);
}

static void caviar_project_raster_point(const VOX_Vect *position,
                                        const VOX_Matrix *part_matrix,
                                        const float point[3],
                                        int callback_output[3],
                                        int bounds_offset[2]) {
    float transformed[3];
    float translated[3];
    vox_matrix_vector_mul(part_matrix->values, point, transformed);
    vector_add(position->values, transformed, translated);
    // The native callback tuple and the box offset are different streams:
    // callback x is scaled transformed-x, callback y/z are scaled translated
    // y/z, while the box receives unscaled translated x/y.
    callback_output[0] = caviar_round_projection_coordinate(transformed[0]);
    callback_output[1] = caviar_round_projection_coordinate(translated[1]);
    callback_output[2] = caviar_round_projection_coordinate(translated[2]);
    bounds_offset[0] = caviar_truncate_projection_coordinate(translated[0]);
    bounds_offset[1] = caviar_truncate_projection_coordinate(translated[1]);
}

static int caviar_intersect_raster_box(int box[6], const int clip[4]) {
    return clip[0] <= box[2] && clip[1] <= box[3] &&
           box[0] <= clip[2] && box[1] <= clip[3];
}

static void caviar_clip_record_box(const VoxRenderRecord *record,
                                   const CaviarRenderClipWindowCandidate *scene_clip,
                                   int output[4]) {
    output[0] = record->draw_left_;
    output[1] = record->draw_top_;
    output[2] = record->draw_right_;
    output[3] = record->draw_bottom_;
    if (scene_clip->width_ != 0 && scene_clip->height_ != 0) {
        if (output[0] < scene_clip->left_) output[0] = scene_clip->left_;
        if (output[1] < scene_clip->top_) output[1] = scene_clip->top_;
        if (scene_clip->right_ < output[2]) output[2] = scene_clip->right_;
        if (scene_clip->bottom_ < output[3]) output[3] = scene_clip->bottom_;
    }
}

static int caviar_flat_raster_box(const CvrShape *shape,
                                  const VOX_Vect *position,
                                  const VOX_Matrix *part_matrix,
                                  int box[6]) {
    const float anchor[3] = {
        static_cast<float>(shape->raster_origin_[0]),
        static_cast<float>(shape->raster_origin_[1]),
        static_cast<float>(shape->raster_origin_[2])
    };
    int projected[3];
    int box_offset[2];
    caviar_project_raster_point(position, part_matrix, anchor, projected,
                                box_offset);
    sub_6401b0(&shape->offset_x_,
               &shape->axis_x_, box);
    if (box[4] != 0 && box[5] != 0) {
        box[0] += box_offset[0]; box[2] += box_offset[0];
        box[1] += box_offset[1]; box[3] += box_offset[1];
    }
    return 1;
}

static int caviar_raster_item_box(const CvrRasterItemHeaderCandidate *item,
                                 const VOX_Vect *position,
                                 const VOX_Matrix *part_matrix,
                                 int box[6], int projected[3],
                                 int box_offset[2]) {
    float point[3] = {
        static_cast<float>(static_cast<int>(item->endpoint_[0]) + item->base_[0]),
        static_cast<float>(static_cast<int>(item->endpoint_[1]) + item->base_[1]),
        static_cast<float>(static_cast<int>(item->endpoint_[2]) + item->base_[2])
    };
    caviar_project_raster_point(position, part_matrix, point, projected,
                                box_offset);
    int16_t delta[3];
    delta[0] = static_cast<int16_t>(item->endpoint_[3] - item->base_[0]);
    delta[1] = static_cast<int16_t>(item->endpoint_[4] - item->base_[1]);
    delta[2] = static_cast<int16_t>(item->endpoint_[5] - item->base_[2]);
    sub_6401b0(delta, item->extent_, box);
    return 1;
}

static void caviar_apply_projected_offset(int box[6], const int offset[2]) {
    box[0] += offset[0]; box[2] += offset[0];
    box[1] += offset[1]; box[3] += offset[1];
}

static int caviar_box_is_inside(const int box[6], const int clip[4]) {
    return clip[0] <= box[0] && clip[1] <= box[1] &&
           box[2] <= clip[2] && box[3] <= clip[3];
}
