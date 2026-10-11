// Bounded raster-stream and projection-clip views used by the Caviar renderer.
// The scene context and callback record are owned in caviar.h. These remaining
// views do not establish complete serialized allocations or original type names.
#pragma once

#include <stddef.h>
#include <stdint.h>

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
