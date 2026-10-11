from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src/general.cpp"
MOVIE = ROOT / "src/movie_media.cpp"


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        current = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if current == '"':
                state = "string"
            elif current == "'":
                state = "char"
            elif current == "/" and following == "/":
                state = "line"
                index += 1
            elif current == "/" and following == "*":
                state = "block"
                index += 1
            elif current == "{":
                depth += 1
            elif current == "}":
                depth -= 1
                if depth == 0:
                    line_start = source.rfind("\n", 0, start) + 1
                    return source[line_start:index + 1]
        elif state == "string":
            if current == "\\":
                index += 1
            elif current == '"':
                state = "code"
        elif state == "char":
            if current == "\\":
                index += 1
            elif current == "'":
                state = "code"
        elif state == "line" and current == "\n":
            state = "code"
        elif state == "block" and current == "*" and following == "/":
            state = "code"
            index += 1
        index += 1
    raise AssertionError(f"unclosed definition for {signature}")


def _compile_run(tmp_path: Path, unit: str, name: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        raise AssertionError("GCC or Clang is required for the actual-source controls")
    source_file = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source_file.write_text(unit)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-pedantic", str(source_file), "-o", str(binary)],
        check=True, timeout=60,
    )
    subprocess.run([str(binary)], check=True, timeout=30)


def test_actual_decoder_helper_bodies_and_runtime_storage(tmp_path: Path) -> None:
    general = GENERAL.read_text()
    signatures = (
        "static unsigned int media_rotate_right_32(",
        'extern "C" void __cdecl sub_5d439f(',
        'extern "C" void __cdecl sub_5d4448(',
        'extern "C" void __fastcall sub_5d3f50(',
        "int __fastcall sub_5d3df0(",
        "int __fastcall sub_5d3980(",
        "void __fastcall sub_5d4020(",
    )
    bodies = "\n\n".join(_definition(general, signature) for signature in signatures)
    unit = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
#define __fastcall
typedef std::uint32_t DWORD;
typedef unsigned char BYTE;
struct MediaPixelFormat {
  DWORD red_bits, green_bits, blue_bits;
  DWORD red_shift, green_shift, blue_shift;
  DWORD combined_component_mask, total_component_bits;
};
struct MediaPaletteLookupStorage {
  DWORD entries[256];
  BYTE trailing_original_storage_7b0_7b3[4];
};
struct MediaDescriptor {
  DWORD flags;
  MediaPixelFormat pixel_format;
  DWORD output_height_60, output_width_64;
  BYTE palette_rgb_triplets_b0[0x300];
  MediaPaletteLookupStorage palette_lookup_3b0;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD unresolved_word_61c;
  DWORD decoder_packet_value_620, decoder_packet_value_624;
  DWORD decoder_packet_value_628, decoder_packet_value_62c;
  int decoder_capacity_630, decoder_capacity_634;
  BYTE *owned_heap_638, *owned_heap_63c, *owned_heap_640;
  int *motion_row_offsets;
  MediaDecodedPixelBlock4x4 *decoded_pixel_blocks;
  MediaMotionCoordinate *motion_coordinates;
  int *motion_pixel_offsets;
};
static int allocation_calls;
static int fail_allocation_call;
static std::size_t allocation_sizes[32];
static int free_calls;
static void *freed_addresses[32];
static void *tracked_malloc(std::size_t size) {
  allocation_sizes[allocation_calls] = size;
  ++allocation_calls;
  if (allocation_calls == fail_allocation_call) return 0;
  return std::malloc(size);
}
static void tracked_free(void *address) {
  freed_addresses[free_calls++] = address;
  std::free(address);
}
#define malloc tracked_malloc
#define free tracked_free
static unsigned short *last_samples;
static unsigned char *last_steps;
static int last_sample_count;
extern "C" int __cdecl sub_5d427c(unsigned short *samples,
                                   unsigned char *steps, int count) {
  last_samples = samples; last_steps = steps; last_sample_count = count;
  return 0;
}
extern "C" void __cdecl sub_5d271d(unsigned int *, unsigned char *, int);
static int raw_copy_count;
extern "C" void __cdecl sub_5d271d(unsigned int *, unsigned char *, int count) {
  raw_copy_count = count;
}
/* ACTUAL_HELPERS */
#undef malloc
#undef free
int main() {
  MediaDescriptor descriptor = {};
  descriptor.output_width_64 = 2;
  descriptor.output_height_60 = 4;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  BYTE frame[128] = {};
  allocation_calls = free_calls = 0;
  assert(sub_5d3df0(&runtime, frame) == 1);
  assert(allocation_calls == 4);
  assert(allocation_sizes[0] == 16 && allocation_sizes[1] == 16 &&
         allocation_sizes[2] == 32 && allocation_sizes[3] == 4);
  assert(last_samples == reinterpret_cast<unsigned short *>(frame + 12));
  assert(last_steps == runtime.owned_heap_638 && last_sample_count == 1);
  int *row_steps = reinterpret_cast<int *>(runtime.motion_row_offsets);
  assert(row_steps[0] == -8 && row_steps[1] == -6 && row_steps[4] == 0 &&
         row_steps[7] == 6);
  assert(reinterpret_cast<std::uintptr_t>(runtime.owned_heap_638) > 0xffffffffu);
  std::free(runtime.owned_heap_638); std::free(runtime.owned_heap_63c);
  std::free(runtime.owned_heap_640); std::free(runtime.motion_row_offsets);

  BYTE *old_638 = static_cast<BYTE *>(std::malloc(4));
  BYTE *old_63c = static_cast<BYTE *>(std::malloc(4));
  BYTE *old_640 = static_cast<BYTE *>(std::malloc(4));
  int *old_644 = static_cast<int *>(std::malloc(4));
  runtime.owned_heap_638 = old_638; runtime.owned_heap_63c = old_63c;
  runtime.owned_heap_640 = old_640; runtime.motion_row_offsets = old_644;
  allocation_calls = free_calls = 0; fail_allocation_call = 2;
  assert(sub_5d3df0(&runtime, frame) == 0);
  assert(free_calls == 4 && freed_addresses[0] == old_638 &&
         freed_addresses[1] == old_63c && freed_addresses[2] == old_644 &&
         freed_addresses[3] == old_640);
  assert(allocation_calls == 4 && runtime.owned_heap_638 != 0 &&
         runtime.owned_heap_63c == 0 && runtime.motion_row_offsets != 0 &&
         runtime.owned_heap_640 != 0);
  std::free(runtime.owned_heap_638); std::free(runtime.motion_row_offsets);
  std::free(runtime.owned_heap_640);

  descriptor.pixel_format.red_bits = 5;
  descriptor.pixel_format.green_bits = 6;
  descriptor.pixel_format.blue_bits = 5;
  descriptor.pixel_format.red_shift = 11;
  descriptor.pixel_format.green_shift = 5;
  descriptor.pixel_format.blue_shift = 0;
  descriptor.flags = 0x40;
  descriptor.palette_rgb_triplets_b0[0] = 255;
  descriptor.palette_rgb_triplets_b0[1] = 128;
  descriptor.palette_rgb_triplets_b0[2] = 64;
  memset(descriptor.palette_lookup_3b0.trailing_original_storage_7b0_7b3,
         0xA5, 4);
  sub_5d3f50(&runtime);
  const DWORD color = ((31u << 11) | (32u << 5) | 8u);
  assert(descriptor.palette_lookup_3b0.entries[0] ==
         (color | (color << 16)));
  for (int i = 0; i < 4; ++i)
    assert(descriptor.palette_lookup_3b0.trailing_original_storage_7b0_7b3[i] == 0xA5);

  BYTE raw_source[16]; BYTE raw_dest[32] = {};
  for (int i = 0; i < 16; ++i) raw_source[i] = static_cast<BYTE>(i + 1);
  runtime.owned_heap_638 = raw_source;
  descriptor.flags = 0; descriptor.pixel_format.total_component_bits = 8;
  descriptor.output_width_64 = 4; descriptor.output_height_60 = 2;
  sub_5d4020(&runtime, raw_dest, raw_dest, 8);
  assert(memcmp(raw_dest, raw_source, 4) == 0 &&
         memcmp(raw_dest + 8, raw_source + 4, 4) == 0);
  descriptor.flags = 0x40;
  sub_5d4020(&runtime, raw_dest, raw_dest, 8);
  assert(raw_copy_count == 8);

  BYTE indices[8] = {0,1,2,3,4,5,6,7}; BYTE converted[64] = {};
  runtime.owned_heap_638 = indices;
  descriptor.pixel_format.total_component_bits = 16;
  descriptor.flags = 0; descriptor.output_width_64 = 8;
  descriptor.output_height_60 = 1;
  for (DWORD i = 0; i != 256; ++i)
    descriptor.palette_lookup_3b0.entries[i] = 0xA1B20000u + i;
  sub_5d4020(&runtime, converted, converted, 16);
  for (int i = 0; i < 8; ++i)
    assert(reinterpret_cast<unsigned short *>(converted)[i] == 0x0000u + i);
  descriptor.flags = 0x40;
  descriptor.pixel_format.total_component_bits = 32;
  sub_5d4020(&runtime, converted, converted, 32);
  for (int i = 0; i < 8; ++i)
    assert(reinterpret_cast<DWORD *>(converted)[i] == 0xA1B20000u + i);

  DWORD compressed_words[8] = {0xFEDCBA98u, 0x76543210u};
  DWORD decoded[8] = {};
  sub_5d439f(decoded, reinterpret_cast<BYTE *>(compressed_words), 1, 0);
  assert(decoded[0] == 0);
  sub_5d439f(decoded, reinterpret_cast<BYTE *>(compressed_words), 1, 32);
  assert(decoded[0] == 0);
  sub_5d439f(decoded, reinterpret_cast<BYTE *>(compressed_words), 1, 31);
  assert(decoded[0] == 0x7edcba98u);
  sub_5d439f(decoded, reinterpret_cast<BYTE *>(compressed_words), 4, 1);

  int motion_indices[2] = {0, 1}; int motion_offsets[2] = {0, 0};
  BYTE previous[32], current[32] = {};
  MediaDecodedPixelBlock4x4 block;
  for (int i = 0; i < 32; ++i) previous[i] = static_cast<BYTE>(i + 1);
  sub_5d4448(motion_indices, previous, current, 1, motion_offsets,
             &block, 4, 1, 1);
  assert(memcmp(previous, current, 16) == 0);
  for (int i = 0; i < 16; ++i) block.pixels[i] = static_cast<BYTE>(0x80 + i);
  memset(current, 0, sizeof(current)); motion_indices[0] = 1;
  sub_5d4448(motion_indices, previous, current, 1, motion_offsets,
             &block, 4, 1, 1);
  assert(memcmp(block.pixels, current, 16) == 0);

  // Two signed coordinate pairs and both raw/compressed pixel blocks.
  // No guards, capacity normalization or extra initialization in the body.
  MediaDescriptor motion_descriptor = {};
  motion_descriptor.output_width_64 = motion_descriptor.output_height_60 = 4;
  MediaRuntime motion = {};
  motion.descriptor = &motion_descriptor;
  BYTE motion_frame[128] = {};
  allocation_calls = free_calls = 0; fail_allocation_call = 0;
  assert(sub_5d3df0(&motion, motion_frame) == 1);
  int *motion_indices_word = reinterpret_cast<int *>(motion.owned_heap_640);
  *motion_indices_word = 2; // Choose raw block, not the previous-frame branch.
  BYTE motion_packet_storage[128] = {};
  BYTE *motion_packet = motion_packet_storage + 2;
  unsigned short motion_fields[4] = {2, 1, 1, 0};
  memcpy(motion_packet, motion_fields, sizeof(motion_fields));
  const unsigned coordinate_words[4] = {1023, 1, 2, 1023};
  for (unsigned component = 0; component < 4; ++component)
    for (unsigned bit = 0; bit < 10; ++bit)
      if (coordinate_words[component] & (1u << bit)) {
        const unsigned position = component * 10 + bit;
        motion_packet[12 + position / 8] |= 1u << (position % 8);
      }
  for (unsigned i = 0; i < 16; ++i) motion_packet[20 + i] = 0x70 + i;
  motion_packet[36] = 0x10; motion_packet[37] = 0x20;
  motion_packet[38] = 0x30; motion_packet[39] = 0x40;
  const DWORD packed_pixels = 0x1b1b1b1b;
  memcpy(motion_packet + 40, &packed_pixels, sizeof(packed_pixels));
  assert(sub_5d3980(&motion, motion_packet) == 1);
  assert(motion.motion_coordinates[0].x == -1 && motion.motion_coordinates[0].y == 1);
  assert(motion.motion_coordinates[1].x == 2 && motion.motion_coordinates[1].y == -1);
  assert(motion.motion_pixel_offsets[0] == 3 && motion.motion_pixel_offsets[1] == -2);
  assert(memcmp(motion.decoded_pixel_blocks[0].pixels, motion_packet + 20, 16) == 0);
  for (unsigned i = 0; i < 16; ++i)
    assert(motion.decoded_pixel_blocks[1].pixels[i] ==
           motion_packet[36 + ((packed_pixels >> ((15 - i) * 2)) & 3)]);
  std::free(motion.owned_heap_638); std::free(motion.owned_heap_63c);
  std::free(motion.owned_heap_640); std::free(motion.motion_row_offsets);
  std::free(motion.motion_coordinates); std::free(motion.motion_pixel_offsets);
  std::free(motion.decoded_pixel_blocks);

  MediaDescriptor video_descriptor = {};
  video_descriptor.output_width_64 = 4;
  video_descriptor.output_height_60 = 4;
  MediaRuntime video = {};
  video.descriptor = &video_descriptor;
  BYTE *frame_a = static_cast<BYTE *>(std::malloc(16));
  BYTE *frame_b = static_cast<BYTE *>(std::malloc(16));
  BYTE *scratch = static_cast<BYTE *>(std::malloc(4));
  int *rows = static_cast<int *>(std::malloc(64));
  for (int i = 0; i < 16; ++i) frame_a[i] = static_cast<BYTE>(i + 10);
  memset(frame_b, 0, 16); memset(scratch, 0, 4); memset(rows, 0, 64);
  video.owned_heap_638 = frame_a; video.owned_heap_63c = frame_b;
  video.owned_heap_640 = scratch; video.motion_row_offsets = rows;
  BYTE packet_storage[128] = {}; BYTE *packet = packet_storage + 2;
  unsigned short fields[4] = {1, 1, 0, 8};
  memcpy(packet, fields, sizeof(fields));
  allocation_calls = free_calls = 0; fail_allocation_call = 0;
  assert(sub_5d3980(&video, packet) == 1);
  assert(video.decoder_packet_value_620 == 1 &&
         video.decoder_packet_value_624 == 1 &&
         video.decoder_packet_value_628 == 0 &&
         video.decoder_packet_value_62c == 8);
  assert(allocation_calls == 3 && allocation_sizes[0] == 8 &&
         allocation_sizes[1] == 4 && allocation_sizes[2] == 16);
  assert(video.owned_heap_638 == frame_b && video.owned_heap_63c == frame_a);
  assert(memcmp(frame_a, frame_b, 16) == 0);
  MediaDecodedPixelBlock4x4 *saved_block = video.decoded_pixel_blocks;
  const int saved_allocations = allocation_calls;

  unsigned short smaller_fields[4] = {1, 0, 0, 0};
  memcpy(packet, smaller_fields, sizeof(smaller_fields));
  assert(sub_5d3980(&video, packet) == 1);
  assert(video.decoded_pixel_blocks == saved_block &&
         video.decoder_capacity_634 == 1 && allocation_calls == saved_allocations);

  unsigned short larger_fields[4] = {1, 2, 0, 0};
  memcpy(packet, larger_fields, sizeof(larger_fields));
  allocation_calls = free_calls = 0; fail_allocation_call = 1;
  assert(sub_5d3980(&video, packet) == 0);
  assert(free_calls == 1 && freed_addresses[0] == saved_block);
  assert(video.decoded_pixel_blocks == 0 && video.decoder_capacity_634 == 2 &&
         allocation_calls == 1 && allocation_sizes[0] == 32);
  std::free(video.owned_heap_638); std::free(video.owned_heap_63c);
  std::free(video.owned_heap_640); std::free(video.motion_row_offsets);
  std::free(video.motion_coordinates); std::free(video.motion_pixel_offsets);
}
'''
    header = (ROOT / "src/movie_media.h").read_text()
    types = (_definition(header, "struct MediaMotionCoordinate") + ";\n" +
             _definition(header, "struct MediaDecodedPixelBlock4x4") + ";\n")
    unit = unit.replace("struct MediaRuntime {", types + "struct MediaRuntime {")
    unit = unit.replace("/* ACTUAL_HELPERS */", bodies)
    with tempfile.TemporaryDirectory(prefix="movie-runtime-helpers-") as directory:
        _compile_run(Path(directory), unit, "runtime_helpers")


def test_actual_caller_case_one_two_dispatch_passes_typed_runtime_and_buffers(
    tmp_path: Path,
) -> None:
    source = MOVIE.read_text()
    switch_start = source.index("switch (runtime->selected_record_kind_39c)")
    end = source.index("  case 3:", switch_start)
    dispatch = source[switch_start:end] + "  default: break;\n  }"
    unit = r'''#include <cassert>
#define __fastcall
#define BYTE unsigned char
typedef unsigned int DWORD;
struct MediaPixelFormat { DWORD total_component_bits; };
struct MediaDescriptor { MediaPixelFormat pixel_format; };
struct MediaRuntime { DWORD selected_record_kind_39c; };
static int calls[8], call_count;
static MediaRuntime *seen_runtime;
static BYTE *seen_packet, *seen_surface;
static void sub_5d3df0(MediaRuntime *r, BYTE *p) { calls[call_count++]=1; seen_runtime=r; seen_packet=p; }
static void sub_5d3f50(MediaRuntime *r) { calls[call_count++]=2; seen_runtime=r; }
static void sub_5d4020(MediaRuntime *r, BYTE *d, BYTE *u, int pitch) {
  calls[call_count++]=3; seen_runtime=r; seen_surface=d; assert(u == seen_packet+8); assert(pitch == 64);
}
static void sub_5d3980(MediaRuntime *r, BYTE *p) { calls[call_count++]=4; seen_runtime=r; seen_packet=p; }
static void dispatch_actual(MediaRuntime *runtime, MediaDescriptor &d,
                            BYTE *pixel_payload, BYTE *surface_source,
                            DWORD output_pitch) {
  BYTE *unused_output=pixel_payload+8;
''' + dispatch + r'''
}
int main() {
  MediaRuntime runtime = {}; MediaDescriptor d = {};
  BYTE packet[16] = {}, surface[128] = {};
  runtime.selected_record_kind_39c = 1;
  d.pixel_format.total_component_bits = 16;
  dispatch_actual(&runtime, d, packet, surface, 64);
  assert(call_count == 3 && calls[0] == 1 && calls[1] == 2 && calls[2] == 3);
  assert(seen_runtime == &runtime && seen_packet == packet && seen_surface == surface);
  call_count = 0; runtime.selected_record_kind_39c = 1;
  d.pixel_format.total_component_bits = 8;
  seen_packet=packet;
  dispatch_actual(&runtime, d, packet, surface, 64);
  assert(call_count == 2 && calls[0] == 1 && calls[1] == 3);
  call_count = 0; runtime.selected_record_kind_39c = 2;
  dispatch_actual(&runtime, d, packet, surface, 64);
  assert(call_count == 2 && calls[0] == 4 && calls[1] == 3);
  assert(seen_runtime == &runtime && seen_packet == packet && seen_surface == surface);
}
'''
    _compile_run(tmp_path, unit, "caller_dispatch")
