#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct CvrShape {
    uint32_t field_0_;
    uint32_t byte_size_;
    float scale_;
    uint8_t middle_[8];
    int16_t axis_x_, axis_y_, axis_z_;
    int16_t offset_x_, offset_y_, offset_z_;
};
#pragma pack(push, 1)
struct CvrRasterShapeViewCandidate {
    CvrShape shape_prefix_;
    int16_t raster_origin_[3];
    uint32_t item_count_;
    uint8_t item_stream_[1];
};
struct CvrRasterItemHeaderCandidate {
    int16_t endpoint_[6];
    int16_t extent_[3];
    int16_t base_[3];
    uint32_t item_count_;
    uint8_t flags_;
    uint8_t unknown_[3];
    uint8_t inline_payload_[1];
};
struct CaviarRenderClipWindowCandidate {
    int32_t left_, top_, right_, bottom_, width_, height_;
};
#pragma pack(pop)
struct CvrStep { unsigned char *shape_; };
struct VoxInstance {
    uint8_t *instance_colour_;
    VOX_Vect *instance_vector_;
    VOX_Matrix *instance_matrix_;
};
struct VoxSceneState {
    uint8_t lead_[4];
    int32_t instance_count_;
    uint32_t draw_flags_;
    VoxInstance *instances_;
    CvrStep *instance_steps_;
    VOX_Vect scene_offset_;
    VOX_Matrix scene_rotation_;
    float scale_;
};
struct CaviarSceneContext {
    VoxSceneState state_;
    VOX_Vect light_vector_;
    VOX_Vect shadow_vector_;
    float field_78_, field_7C_, field_80_, field_84_, field_88_;
    float field_8C_, field_90_, field_94_, field_98_, field_9C_, field_A0_;
    uint8_t field_A4_;
};
struct CaviarCamera { VOX_Vect position; VOX_Matrix rotation; float scaling; };
struct CaviarChannelBounds {
    float lead_[5];
    int32_t values_[6];
};
struct VoxCameraBlock { CaviarCamera camera_; CaviarChannelBounds bounds_; };
struct VoxRenderRecord {
    uintptr_t setup_block_;
    int32_t draw_left_, draw_top_, draw_right_, draw_bottom_;
    int32_t draw_width_, draw_height_;
    int32_t clip_max_a_x_, clip_max_a_y_, clip_min_a_x_, clip_min_a_y_;
    int32_t clip_width_a_, clip_height_a_;
    int32_t clip_max_b_x_, clip_max_b_y_, clip_min_b_x_, clip_min_b_y_;
    int32_t clip_width_b_, clip_height_b_;
};
struct CaviarRenderSetupViewCandidate {
    uint8_t bytes_00_[8];
    uint32_t shadow_data_;
    int32_t half_width_;
    int32_t half_height_;
};
struct CaviarSceneProjectionScratchCandidate {
    VOX_Vect camera_offset_;
    VOX_Matrix view_scene_matrix_;
    float perspective_factor_;
    float projected_scale_;
    float viewport_aspect_;
    CaviarRenderClipWindowCandidate part_clip_;
    VOX_Vect shadow_centre_;
    VOX_Matrix shadow_matrix_;
    float shadow_radius_x_, shadow_radius_y_;
    CaviarRenderClipWindowCandidate shadow_clip_;
};

static CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch;
static VOX_Matrix CaviarProjectionShadowBasis;
static int32_t vox_projection_render_stage_ = 0;
static const uint8_t *vox_projection_selected_angle_ = 0;
static const float CaviarProjectionZero = 0.0f;
static const float CaviarProjectionMinusOne = -1.0f;
static const float CaviarProjectionOne = 1.0f;
static const float CaviarProjectionHalfPixel = -0.5f;
static const float CaviarProjectionNearNegativeEpsilon = -1.0e-6f;
static const float CaviarProjectionNearPositiveEpsilon = 1.0e-6f;
static const float CaviarProjectionRoundThreshold = 0.0f;
static const float CaviarProjectionRoundAbove = 0.5f;
static const float CaviarProjectionAngleLimit = 1.0f;
static const float CaviarProjectionAngleWrap = -6.283185482025146484375f;
static const float CaviarProjectionPolarScale = -14.64225482940673828125f;
static const float CaviarProjectionAzimuthScale = -14.1647891998291015625f;
typedef uint8_t CaviarProjectionAngleRowCandidate[0x860];
static const CaviarProjectionAngleRowCandidate *CaviarProjectionAngleRows[24];
static const float vox_radius_scale_ = 1.0f;
static int32_t vox_part_behind_result = 0;
static int sub_6401b0_call_count = 0;
static int16_t sub_6401b0_centres[8];

static VOX_Matrix &caviar_scene_shadow_matrix_view(
    CaviarSceneProjectionScratchCandidate &scratch) {
    return scratch.shadow_matrix_;
}

static void set_identity(VOX_Matrix &matrix) {
    memset(&matrix, 0, sizeof(matrix));
    matrix.values[0][0] = matrix.values[1][1] = matrix.values[2][2] = 1.0f;
}

static void vox_matrix_vector_mul(const float matrix[3][3], const float vector[3],
                                  float *out) {
    float result[3];
    for (int row = 0; row != 3; ++row)
        result[row] = (matrix[row][1] * vector[1] + matrix[row][2] * vector[2])
                    + matrix[row][0] * vector[0];
    memcpy(out, result, sizeof(result));
}
static void vox_matrix_matrix_mul(const float left[3][3], const float right[3][3],
                                  float out[3][3]) {
    float result[3][3];
    for (int row = 0; row != 3; ++row)
        for (int col = 0; col != 3; ++col)
            result[row][col] = left[row][0] * right[0][col] +
                left[row][1] * right[1][col] + left[row][2] * right[2][col];
    memcpy(out, result, sizeof(result));
}
static void vector_add(const float *left, const float *right, float *out) {
    float result[3] = {left[0] + right[0], left[1] + right[1], left[2] + right[2]};
    memcpy(out, result, sizeof(result));
}
static void leaf_00628180_redirect(const float *left, const float *right,
                                   float *out) {
    float result[3] = {left[0] - right[0], left[1] - right[1], left[2] - right[2]};
    memcpy(out, result, sizeof(result));
}
static void vox_matrix_inverse(const float source[3][3], float output[3][3]) {
    float copy[3][3];
    memcpy(copy, source, sizeof(copy));
    memcpy(output, copy, sizeof(copy));
}
static void sub_63fa80(float *) { }
static unsigned char *__cdecl vox_walk_scaled_chain(unsigned char *node, float) {
    return node;
}
static bool __cdecl vox_part_behind(const VOX_Vect *, float, const VoxCameraBlock *) {
    return vox_part_behind_result != 0;
}
static void sub_6401b0(const int16_t *centre, const int16_t *, int *out) {
    sub_6401b0_centres[sub_6401b0_call_count++] = centre[0];
    out[0] = 2; out[1] = 2; out[2] = 2; out[3] = 2;
    out[4] = 1; out[5] = 1;
}
static double _hypot(double x, double y) { return sqrt(x * x + y * y); }
void __cdecl caviar_clip_shape(
    VoxRenderRecord *, const CvrRasterShapeViewCandidate *, const VOX_Vect *,
    const VOX_Matrix *, const CaviarRenderClipWindowCandidate *, int[6]);

/* PRODUCT_EXTRACTS */

static void test_stage_scene_alias_and_shadow_z() {
    CaviarSceneContext context;
    VoxInstance instance;
    CvrStep step;
    VOX_Vect instance_vector[1];
    VOX_Matrix instance_matrix[1];
    uint8_t colour[1] = {0};
    CvrShape shape;
    CaviarRenderSetupViewCandidate setup;
    VoxRenderRecord record;
    VoxCameraBlock camera;
    memset(&context, 0, sizeof(context));
    memset(&instance, 0, sizeof(instance));
    memset(&setup, 0, sizeof(setup));
    memset(&record, 0, sizeof(record));
    memset(&camera, 0, sizeof(camera));
    memset(&shape, 0, sizeof(shape));
    set_identity(context.state_.scene_rotation_);
    set_identity(instance_matrix[0]);
    set_identity(camera.camera_.rotation);
    instance_vector[0].values[2] = 1.0f;
    instance.instance_colour_ = colour;
    instance.instance_vector_ = instance_vector;
    instance.instance_matrix_ = instance_matrix;
    context.state_.instance_count_ = 1;
    context.state_.instances_ = &instance;
    context.state_.instance_steps_ = &step;
    context.state_.scale_ = 1.0f;
    context.field_90_ = 1.0f;
    context.field_A4_ = 1;
    context.field_78_ = 2.0f;
    context.field_9C_ = context.field_A0_ = 1.0f;
    context.shadow_vector_.values[0] = 2.0f;
    context.shadow_vector_.values[1] = 3.0f;
    context.shadow_vector_.values[2] = 0.0f;
    camera.camera_.scaling = 1.0f;
    camera.bounds_.lead_[1] = 0.0f;
    camera.bounds_.lead_[2] = 1.0f;
    camera.bounds_.lead_[3] = -1.0f;
    camera.bounds_.lead_[4] = 1.0f;
    camera.bounds_.values_[4] = 10;
    camera.bounds_.values_[5] = 10;
    setup.shadow_data_ = 1;
    setup.half_width_ = setup.half_height_ = 20;
    record.setup_block_ = reinterpret_cast<uintptr_t>(&setup);
    step.shape_ = reinterpret_cast<unsigned char *>(&shape);
    CaviarProjectionShadowBasis.values[0][0] = 1.0f;
    CaviarProjectionShadowBasis.values[1][1] = 1.0f;

    float aliased = 0.0f;
    vox_stage_scene(&record, &context, &camera, 0, &aliased, &aliased);
    assert(aliased == CaviarSceneProjectionScratch.shadow_radius_y_);
    assert(context.shadow_vector_.values[2] == CaviarProjectionNearPositiveEpsilon);

    context.shadow_vector_.values[2] = -CaviarProjectionNearPositiveEpsilon / 2.0f;
    float x = 0.0f, y = 0.0f;
    vox_stage_scene(&record, &context, &camera, 0, &x, &y);
    assert(context.shadow_vector_.values[2] == CaviarProjectionNearNegativeEpsilon);
    assert(x == CaviarSceneProjectionScratch.shadow_radius_x_);
    assert(y == CaviarSceneProjectionScratch.shadow_radius_y_);
}

static void test_inline_shape_stride_sentinel_and_inclusive_edge() {
    VoxRenderRecord record;
    memset(&record, 0, sizeof(record));
    record.draw_left_ = record.draw_top_ = 2;
    record.draw_right_ = record.draw_bottom_ = 2;
    record.draw_width_ = record.draw_height_ = 1;
    CaviarRenderClipWindowCandidate clip = {2, 2, 2, 2, 1, 1};
    unsigned char storage[0x100];
    memset(storage, 0xA5, sizeof(storage));
    CvrRasterShapeViewCandidate *shape =
        reinterpret_cast<CvrRasterShapeViewCandidate *>(storage);
    shape->shape_prefix_.field_0_ = 2;
    shape->item_count_ = 2;
    CvrRasterItemHeaderCandidate *first =
        reinterpret_cast<CvrRasterItemHeaderCandidate *>(shape->item_stream_);
    memset(first, 0, 0x20);
    first->endpoint_[3] = 10;
    first->item_count_ = 1;
    first->flags_ = 0;
    const size_t second_offset = 0x2A + 0x20 + 3;
    CvrRasterItemHeaderCandidate *second =
        reinterpret_cast<CvrRasterItemHeaderCandidate *>(storage + second_offset);
    memset(second, 0, 0x20);
    second->endpoint_[3] = 20;
    second->item_count_ = 1;
    second->flags_ = 0;
    reinterpret_cast<CvrRasterItemHeaderCandidate *>(storage + 0x2A + 0x20)
        ->endpoint_[3] = 99; // wrong no-payload-advance sentinel
    VOX_Vect position = {{0.0f, 0.0f, 0.0f}};
    VOX_Matrix orientation;
    set_identity(orientation);
    int output[6];
    sub_6401b0_call_count = 0;
    caviar_clip_shape(&record, shape, &position, &orientation, &clip, output);
    assert(sub_6401b0_call_count == 2);
    assert(sub_6401b0_centres[0] == 10 && sub_6401b0_centres[1] == 20);
    assert(output[0] == 2 && output[1] == 2 && output[2] == 2 && output[3] == 2);
    assert(output[4] == 1 && output[5] == 1);

    // Exercise the extracted 0x63A280 caller too: the same variable-sized
    // shape stream is reached through the loader, and its resulting inclusive
    // one-pixel bounds are written back to the two record windows.
    VoxInstance instance;
    CvrStep step;
    uint8_t colour[1] = {0};
    VOX_Vect vectors[1] = {{{0.0f, 0.0f, 0.0f}}};
    VOX_Matrix matrices[1];
    set_identity(matrices[0]);
    instance.instance_colour_ = colour;
    instance.instance_vector_ = vectors;
    instance.instance_matrix_ = matrices;
    step.shape_ = storage;
    CaviarSceneContext context;
    memset(&context, 0, sizeof(context));
    context.state_.instance_count_ = 1;
    context.state_.instances_ = &instance;
    context.state_.instance_steps_ = &step;
    context.state_.scale_ = 1.0f;
    uint32_t descriptor[3] = {0, 0, 0};
    record.setup_block_ = reinterpret_cast<uintptr_t>(descriptor);
    CaviarSceneProjectionScratch.projected_scale_ = 1.0f;
    CaviarSceneProjectionScratch.view_scene_matrix_ = matrices[0];
    CaviarSceneProjectionScratch.camera_offset_.values[0] = 0.0f;
    CaviarSceneProjectionScratch.camera_offset_.values[1] = 0.0f;
    CaviarSceneProjectionScratch.camera_offset_.values[2] = 0.0f;
    CaviarSceneProjectionScratch.part_clip_ = clip;
    vox_part_behind_result = 0;
    sub_6401b0_call_count = 0;
    vox_load_scene_view(&record, &context, reinterpret_cast<VoxCameraBlock *>(0), 0);
    assert(sub_6401b0_call_count == 2);
    assert(sub_6401b0_centres[0] == 10 && sub_6401b0_centres[1] == 20);
    assert(record.clip_max_a_x_ == 2 && record.clip_max_a_y_ == 2);
    assert(record.clip_min_a_x_ == 2 && record.clip_min_a_y_ == 2);
    assert(record.clip_width_a_ == 1 && record.clip_height_a_ == 1);
}

static void test_angle_row_wrap_special_case_and_interior_pointer() {
    CaviarProjectionAngleRowCandidate rows[24];
    for (int row = 0; row != 24; ++row) {
        for (int byte = 0; byte != 0x860; ++byte)
            rows[row][byte] = static_cast<uint8_t>(row ^ byte);
        CaviarProjectionAngleRows[row] =
            reinterpret_cast<const CaviarProjectionAngleRowCandidate *>(
                &rows[row]);
    }
    CaviarSceneProjectionScratch.projected_scale_ = 1.0f;
    CaviarSceneProjectionScratch.viewport_aspect_ = 1.0f;
    VOX_Matrix part, camera;
    set_identity(part);
    set_identity(camera);
    VOX_Vect direction = {{1.0f, 0.0f, 0.0f}};
    caviar_direction_to_projection_index(&part, &direction, 1.0f, &camera);
    assert(vox_projection_render_stage_ == 1);
    assert(vox_projection_selected_angle_ == rows[23] + 0x59);

    // Positive azimuths take the negated-wrap comparison path; the exact
    // 0x59 result wraps to index zero, which selects the row's final byte.
    direction.values[1] = 0.25f;
    caviar_direction_to_projection_index(&part, &direction, 1.0f, &camera);
    assert(vox_projection_selected_angle_ >= rows[0]);
    assert(vox_projection_selected_angle_ < rows[24 - 1] + 0x860);
}

int main() {
    test_stage_scene_alias_and_shadow_z();
    test_inline_shape_stride_sentinel_and_inclusive_edge();
    test_angle_row_wrap_special_case_and_interior_pointer();
    return 0;
}
