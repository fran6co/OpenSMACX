#pragma once

#include "caviar_render_types.h"

#pragma pack(push, 1)
struct CaviarSceneProjectionScratchCandidate {
  VOX_Vect camera_offset_;                      // +0x00, image 0x9C0ACC
  VOX_Matrix view_scene_matrix_;                // +0x0C, image 0x9C0AD8
  float perspective_factor_;                    // +0x30, image 0x9C0AFC
  float projected_scale_;                      // +0x34, image 0x9C0B00
  float viewport_aspect_;                      // +0x38, image 0x9C0B04
  CaviarRenderClipWindowCandidate part_clip_;   // +0x3C, image 0x9C0B08
  VOX_Vect shadow_centre_;                     // +0x54, image 0x9C0B20
  // 6396A0 writes the first two rows; 63A280 consumes all nine cells as
  // one matrix. The third row has no observed producer and starts zero.
  VOX_Matrix shadow_matrix_;                  // +0x60, image 0x9C0B2C
  float shadow_radius_x_;                      // +0x84, image 0x9C0B50
  float shadow_radius_y_;                      // +0x88, image 0x9C0B54
  CaviarRenderClipWindowCandidate shadow_clip_;// +0x8C, image 0x9C0B58
};
#pragma pack(pop)

typedef char caviar_scene_scratch_extent_candidate[
    sizeof(CaviarSceneProjectionScratchCandidate) == 0xA4 ? 1 : -1];
typedef char caviar_scene_scratch_part_clip_offset_candidate[
    offsetof(CaviarSceneProjectionScratchCandidate, part_clip_) == 0x3C ? 1 : -1];
typedef char caviar_scene_scratch_shadow_clip_offset_candidate[
    offsetof(CaviarSceneProjectionScratchCandidate, shadow_clip_) == 0x8C ? 1 : -1];

// Both axis construction and matrix consumption use the same real object.
inline VOX_Matrix &caviar_scene_shadow_matrix_view(
    CaviarSceneProjectionScratchCandidate &scratch) {
  return scratch.shadow_matrix_;
}

extern CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch;
