// Private source-type proposal for the six remaining Caviar draw identities.
// This file is never included by product source without coordinator review.
#pragma once

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)
struct CaviarSceneContextCandidate {
  VoxSceneState state_;       // +0x00, existing 0x48-byte Caviar prefix
  VOX_Vect light_vector_;     // +0x48
  VOX_Vect shadow_vector_;    // +0x54
  float field_78_;            // +0x60, 6396A0 uses as float
  float field_7C_;            // +0x64
  float field_80_;            // +0x68
  float field_84_;            // +0x6C
  float field_88_;            // +0x70
  float field_8C_;            // +0x74
  float field_90_;            // +0x78
  float field_94_;            // +0x7C, 6396A0 uses as float
  float field_98_;            // +0x80
  float field_9C_;            // +0x84
  float field_A0_;            // +0x88
  uint8_t field_A4_;          // +0x8C
};
#pragma pack(pop)

typedef char caviar_context_prefix_must_be_0x48[
    (sizeof(void *) != 4 || sizeof(VoxSceneState) == 0x48) ? 1 : -1];
typedef char caviar_context_tail_must_end_at_0x8d[
    (sizeof(void *) != 4 || sizeof(CaviarSceneContextCandidate) == 0x8D) ? 1 : -1];
typedef char caviar_context_light_offset_must_be_0x48[
    (sizeof(void *) != 4 ||
     offsetof(CaviarSceneContextCandidate, light_vector_) == 0x48) ? 1 : -1];
typedef char caviar_context_shadow_offset_must_be_0x54[
    (sizeof(void *) != 4 ||
     offsetof(CaviarSceneContextCandidate, shadow_vector_) == 0x54) ? 1 : -1];
typedef char caviar_context_flag_offset_must_be_0x8c[
    (sizeof(void *) != 4 ||
     offsetof(CaviarSceneContextCandidate, field_A4_) == 0x8C) ? 1 : -1];

// 0x63E950 dispatches directly through record callback fields. These are
// plain cdecl function pointers populated by the pixel-size dispatchers; not
// C++ virtual methods. Names intentionally remain protocol descriptions.
typedef void(__cdecl *CaviarPartDrawContainedFn)(int, int, int, int,
                                                const void *, int);
typedef void(__cdecl *CaviarPartDrawClippedFn)(int, int, int, const int *, int,
                                               const void *, int);
typedef void(__cdecl *CaviarShadowDrawContainedFn)(int, int, int,
                                                  const void *, int);
typedef void(__cdecl *CaviarShadowDrawClippedFn)(int, int, const int *, int,
                                                 const void *, int);

#pragma pack(push, 1)
struct CaviarRenderDispatchSlotsCandidate {
  CaviarPartDrawContainedFn part_contained_; // record +0x6C
  CaviarPartDrawClippedFn part_clipped_;     // record +0x70
  CaviarShadowDrawContainedFn shadow_contained_; // record +0x74
  CaviarShadowDrawClippedFn shadow_clipped_;     // record +0x78
};
#pragma pack(pop)

typedef char caviar_render_dispatch_slots_must_be_16[
    (sizeof(void *) != 4 ||
     sizeof(CaviarRenderDispatchSlotsCandidate) == 0x10) ? 1 : -1];

// One variable record in the raster stream. The first twelve shorts provide
// low/high endpoints, extent and base vectors used by 63F5E0; the dword at
// +0x18 is the record's item count, and bit 0 of the byte at +0x1C selects
// whether the inline payload begins with a byte-size dword. The complete
// payload/record extent varies and is not represented as a fixed object.
#pragma pack(push, 1)
struct CvrRasterItemHeaderCandidate {
  int16_t endpoint_[6];       // +0x00..+0x0B
  int16_t extent_[3];         // +0x0C..+0x11
  int16_t base_[3];            // +0x12..+0x17
  uint32_t item_count_;        // +0x18
  uint8_t flags_;              // +0x1C
  uint8_t unknown_1d_[3];      // +0x1D..+0x1F
  uint8_t inline_payload_[1];  // +0x20, variable
};
#pragma pack(pop)

typedef char cvr_raster_item_prefix_must_be_0x20[
    offsetof(CvrRasterItemHeaderCandidate, inline_payload_) == 0x20 ? 1 : -1];

// A 63A280 output rectangle consists of six adjacent dwords at each of the
// render record's +8 and +0x20 windows. Its final two words are extent values.
struct CaviarRenderClipWindowCandidate {
  int32_t left_;
  int32_t top_;
  int32_t right_;
  int32_t bottom_;
  int32_t width_;
  int32_t height_;
};

typedef char caviar_clip_window_must_be_24[
    sizeof(CaviarRenderClipWindowCandidate) == 0x18 ? 1 : -1];

// Proposed owners for the two six-dword clip windows consumed by 63A280 and
// 63F5E0. They are existing shared scratch regions at 0x9C0B08 and 0x9C0B58,
// not newly allocated rectangles.
extern CaviarRenderClipWindowCandidate CaviarCurrentProjectionClip;
extern CaviarRenderClipWindowCandidate CaviarShadowProjectionClip;
