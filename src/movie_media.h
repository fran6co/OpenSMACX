// Source-owned movie/media storage and interface views.
// Explicit unresolved ranges are storage evidence, not recovered semantics.
#pragma once

#include <stddef.h>
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <ddraw.h>

struct MediaRuntime;
struct MediaQueueRecord;
#if defined(_MSC_VER)
class __single_inheritance UV2Player;
#else
class UV2Player;
#endif
typedef int (__cdecl UV2Player::*MediaKeyPollMember)();

// Verified storage chain produced by the media color setup and consumed by
// movie conversion routines in general.cpp. The coefficient subviews are
// aliases into the second halves of the 512-entry contribution arrays.
struct MediaTransformBitState {
  const BYTE *cursor;                                // 9C5920
  LONG available_bits;                               // 9C5924
  DWORD accumulator;                                 // 9C5928
};

struct MediaConversionTableStorage {
  DWORD block_scale_entries[64];                    // 9C5820..9C591F; decoder scale table
  MediaTransformBitState transform_bit_state;        // 9C5920..9C592B
  DWORD packed_component_words[3 * 128];             // 9C592C..9C5F2B
  struct {
    DWORD words[1024];                               // 9C5F2C..9C6F2B
  } remap_or_lookup_storage;
  DWORD contribution_byte3[512];                     // 9C6F2C..9C772B
  DWORD contribution_byte1[512];                     // 9C772C..9C7F2B
  DWORD contribution_byte0[512];                     // 9C7F2C..9C872B
  DWORD blend_high[306];                             // 9C872C..9C8BF3
  DWORD blend_mid[262];                              // 9C8BF4..9C900B
  DWORD blend_low[354];                              // 9C900C..9C9593
  // These are proven producer windows, not full consumer-domain proofs.
  // Some general.cpp readers use nominal nine-bit indexes; their valid-input
  // bounds remain obligations. Do not infer wider storage or add a clamp.
};

extern MediaConversionTableStorage movie_media_conversion_tables;

struct MediaPixelFormat {
  DWORD red_bits;                                    // descriptor +2C
  DWORD green_bits;                                  // +30
  DWORD blue_bits;                                   // +34; popcount of blue mask
  DWORD red_shift;                                   // +38
  DWORD green_shift;                                 // +3C
  DWORD blue_shift;                                  // +40
  DWORD combined_component_mask;                     // +44
  DWORD total_component_bits;                        // +48

  void sub_5cc450();                                 // ORIGINAL 005CC450
};

struct MediaPaletteLookupStorage {
  DWORD entries[256];                              // descriptor +3b0..+7af
  BYTE trailing_original_storage_7b0_7b3[4];       // descriptor +7b0..+7b3
};
typedef char media_palette_lookup_storage_extent_check[
    sizeof(MediaPaletteLookupStorage) == 0x404 ? 1 : -1];

// Existing seed identity at 0x005CC940 is an unmangled fastcall free
// function. A product migration can narrow its int* parameters to these
// evidence-backed object views while preserving the C-linkage identity.
extern "C" void __fastcall sub_5cc940(MediaRuntime *runtime,
                                     MediaQueueRecord *record);
extern "C" int __fastcall sub_5cbc00(int ecx_arg, int edx_arg,
                                     int stack_1, int stack_2, int stack_3,
                                     int stack_4, int stack_5, int stack_6,
                                     int stack_7, int stack_8);
extern "C" int __fastcall sub_5cc150(int ecx_arg, int edx_arg,
                                     int stack_1, int stack_2, int stack_3,
                                     int stack_4, int stack_5);
extern "C" void __fastcall sub_5cc2b0(int selected_component);
extern "C" char *__fastcall sub_5cf120(int code);

typedef int (__cdecl *MediaCallback)(void);

struct MediaDescriptor {
  void sub_5cb220(); // ORIGINAL 0x005CB220: receiver is this descriptor

  MediaRuntime *runtime;                         // +00
  DWORD flags;                                   // +04
  DWORD capability_options_08;                    // +08; tested before DirectDraw GetCaps
  IUnknown *draw_query_interface_0c;              // +0c; QueryInterface receiver
  IDirectDraw2 *draw_device_10;                   // +10; QI for IID_IDirectDraw2 at 66FC38
  IDirectDrawSurface2 *surface_14;                // +14; DirectDraw consumer
  IDirectDrawSurface2 *surface_18;                // +18; DirectDraw consumer
  IDirectDrawSurface2 *surface_1c;                // +1c; created/QI surface
  IDirectDrawSurface2 *surface_20;                // +20; created/QI surface
  IDirectDrawPalette *palette_24;                 // +24; CreatePalette/SetEntries
  BYTE *palette_entries_28;                       // +28; allocated 0x400 bytes
  MediaPixelFormat pixel_format;                // +2c..+4b; passed as 5CC450 receiver
  BYTE *provided_output_base_4c;                 // +4c; selected without Lock
  // 5CBBC0 swaps this address into +4c before the next record conversion.
  // The provided-buffer stride is derived from width/pixel mode, not +50.
  BYTE *provided_output_alternate_50;            // +50; alternating output buffer
  IDirectSound *sound_device;                    // +54
  // Setup5CE340 queries this optional buffer; teardown5CE450 releases it.
  // The parsed playback rate is MediaRuntime+58, a different object's field.
  IDirectSoundBuffer *source_buffer_58;          // +58; optional caller buffer
  IDirectSoundBuffer *play_buffer;               // +5c
  DWORD output_height_60;                        // +60; DirectDraw loop/rectangle bound
  DWORD output_width_64;                         // +64; DirectDraw rectangle bound
  LONG geometry_offset_x_68;                     // +68; added to centered left edge
  LONG geometry_offset_y_6c;                     // +6c; added to centered top edge
  DWORD geometry_width_override_70;               // +70; replaces computed width
  DWORD geometry_height_override_74;              // +74; replaces computed height
  DWORD setup_value_78;                           // +78; initialized to 100 when flag clear
  DWORD stream_gate_7c;                          // +7c
  DWORD stream_gate_80;                          // +80
  LONG volume_argument;                          // +84
  BYTE unresolved_original_storage_88_8b[4];      // +88..+8b
  DWORD elapsed_ticks;                           // +8c
  LONG progress_counter;                         // +90; Interlocked increments
  LONG progress_value;                           // +94
  LONG state;                                    // +98; mixed atomic/plain use
  HWND window;                                   // +9c
  HMMIO stream;                                  // +a0
  DWORD priority;                                // +a4
  DWORD beginthread_result;                      // +a8; not assumed HANDLE
  MediaCallback callback;                       // +ac
  // Exec stores UV2Player::on_key here before palette data is populated.
  // Native palette production/consumption later uses the same bytes.
  union {
    MediaKeyPollMember key_poll_member_b0;     // exec-time member callback
    BYTE palette_rgb_triplets_b0[0x300];       // +b0..+3af; 256 RGB triplets
  };
  MediaPaletteLookupStorage palette_lookup_3b0; // +3b0..+7b3
};

struct MediaQueueRecord {
  BYTE *payload; // +00
  DWORD field_04; // +04; size-like on output records, producer unresolved on read records
  DWORD field_08; // +08; producer-stored counter snapshot
  DWORD field_0c; // +0c; ring-specific kind/snapshot role
};

// Native 5D0AAB/5D0D82 share this exact context from ESI: 64 weight DWORDs,
// three delta predictors, unresolved bytes through context+1FF, then input
// and bit cursor at +200/+204. The decoder calls receive this subobject, not
// the MediaRuntime base.
struct MediaDecoderContext {
  DWORD block_transform_weights[64];       // +000..0FF
  DWORD predictor_4f8;                     // +100
  DWORD predictor_4fc;                     // +104
  DWORD predictor_500;                     // +108
  BYTE unresolved_original_storage_10c_1ff[0xf4]; // +10C..1FF
  const BYTE *compressed_input_200;         // +200
  DWORD bit_position_204;                   // +204
};
typedef char media_decoder_context_extent_check[
    sizeof(MediaDecoderContext) == 0x208 ? 1 : -1];

// Descriptive recovered types, not original debug names. 5D3980 decodes
// signed ten-bit x/y pairs; 5D4448 consumes their signed pixel offsets.
struct MediaMotionCoordinate {
  int x;
  int y;
};

// 5D3980 copies/expands sixteen byte pixels; 5D4448 copies four rows.
struct MediaDecodedPixelBlock4x4 {
  BYTE pixels[16];
};

typedef char media_motion_coordinate_extent_check[
    sizeof(MediaMotionCoordinate) == 8 ? 1 : -1];
typedef char media_decoded_pixel_block_extent_check[
    sizeof(MediaDecodedPixelBlock4x4) == 16 ? 1 : -1];

struct MediaRuntime {
  // Original-mapped family bodies proposed for this owner.
  static int __fastcall initialize_stream(LPSTR filename,
                                          MediaRuntime *runtime); // 005CB050
  int setup_sound_device();            // 005CE340
  int create_playback_buffer();        // 005CE4B0
  int release_session_interfaces();    // 005CE450
  int release_playback_buffer();       // 005CE620
  int __fastcall parse_initial_format(int byte_count); // 005CA6C0; byte count in EDX
  int __fastcall sub_5ca230(int payload_bytes, int record_kind);
  // Original 005CBAE0: runtime in ECX, show/hide selection in EDX.
  int __fastcall update_overlay(int show);

  // Existing mapped/seed dependencies, deliberately declarations only here.
  // Their original identities and current source homes are listed in the
  // companion dependency note; no placeholder implementations are supplied.
  int sub_5ce2b0();
  void sub_5cb6c0();
  int sub_5ce2f0();
  int sub_5cae00();
  void sub_5cbe30();
  void sub_5cbbc0();
  void sub_5cc710();

  MediaDescriptor *descriptor;                   // +000
  DWORD surface_width_004;                        // +004; SurfaceDesc.dwWidth
  DWORD surface_height_008;                       // +008; SurfaceDesc.dwHeight
  DWORD output_rect_extent_0c;                    // +00c; added to left edge
  DWORD output_rect_height_10;                   // +010; destination rectangle height
  DWORD output_rect_left_14;                      // +014
  DWORD output_rect_top_18;                       // +018
  BYTE *input_ring_base;                          // +01c; 1,200,000 bytes
  BYTE *decoded_ring_base;                        // +020; 300,000 bytes
  BYTE *input_ring_current;                       // +024
  BYTE *decoded_ring_current;                     // +028
  LONG surface_loss_counter_02c;                    // +02c; incremented for source +14 loss
  LONG surface_loss_counter_030;                    // +030; incremented for source +18 loss
  LONG surface_loss_counter_034;                    // +034; incremented for special +1c loss
  LONG surface_loss_counter_038;                    // +038; incremented for +1c loss
  LONG teardown_gate_3c;                          // +03c; guards release group
  DWORD record_option_040;                          // +040; payload byte7 bit1 for kind4
  DWORD record_option_044;                          // +044; payload byte7 bit0 for kind4
  DWORD record_scale_byte_048;                      // +048; payload byte4 for kind3/4
  DWORD queue_record_kind;                        // +04c; values 1,2,4 dispatched
  DWORD output_handle_candidate;                  // +050; passed to helper/reset
  DWORD format_parameter_54;                      // +054; parser-written; decoder-read
  DWORD playback_frequency;                       // +058
  DWORD format_parameter_5c;                      // +05c; parser-written; decoder-read
  DWORD format_dispatch_60;                       // +060; parser-written; dispatches 0/7/10
  MediaQueueRecord output_records[24];            // +064..+1e3
  MediaQueueRecord read_records[24];              // +1e4..+363
  LONG buffer_setup_finished;                     // +364
  DWORD playback_buffer_bytes;                    // +368
  DWORD playback_cursor;                          // +36c
  LONG output_consumer_cursor;                    // +370
  LONG output_producer_cursor;                    // +374
  LONG read_consumer_cursor;                      // +378
  LONG read_producer_cursor;                      // +37c
  LONG pending_read_records;                      // +380
  LONG pending_output_records;                    // +384
  LONG read_counter;                              // +388
  LONG output_counter;                            // +38c
  LONG output_lock_word;                          // +390
  BYTE unresolved_original_storage_394_397[4];    // +394..+397
  LONG stream_eof;                                // +398
  DWORD selected_record_kind_39c;                    // +39c; copied from record +0c
  LONG channel_state;                             // +3a0
  LONG channel_poll_result;                       // +3a4
  LONG channel_seek_position;                     // +3a8
  LONG worker_wakeup_counter;                     // +3ac; interlocked increments
  LONG sync_word_3b0;                             // +3b0; Interlocked access
  DWORD beginthread_result_3b4;                  // +3b4; do not assume HANDLE
  DWORD synthetic_playback_cursor;                // +3b8
  DWORD synthetic_cursor_mode;                    // +3bc
  DWORD emulated_device;                          // +3c0
  DWORD source_buffer_bytes;                      // +3c4
  LONG release_optional_buffer;                   // +3c8
  LONG release_sound_device;                      // +3cc
  LONG protocol_gate;                             // +3d0; not thread-mode
  LONG release_interface_10;                      // +3d4; gates +10 Release
  DWORD decoder_state_3d8;                       // +3d8; decoder read/write
  DWORD decoder_state_3dc;                       // +3dc; decoder read/write
  DWORD decoder_state_3e0;                       // +3e0; decoder read/write
  DWORD decoder_state_3e4;                       // +3e4; decoder read/write
  DWORD decoder_state_3e8;                       // +3e8; 5CE6E3 input/output
  DWORD decoder_state_3ec;                       // +3ec; 5CE6E3 input/output
  DWORD decoder_state_3f0;                       // +3f0; 5CE6E3 input/output
  DWORD decoder_state_3f4;                       // +3f4; 5CE6E3 input/output
  MediaDecoderContext decoder_context_3f8;       // +3f8..+5ff
  BYTE unresolved_original_storage_600_607[8];   // +600..+607
  DWORD start_ticks;                              // +608
  DWORD end_ticks;                                // +60c
  BYTE unresolved_original_storage_610_617[8];    // +610..+617
  DWORD unresolved_word_618;                     // +618; native store/read
  DWORD unresolved_word_61c;                     // +61c; native store/loop bound
  DWORD decoder_packet_value_620;                  // +620; 5D3980 payload DWORD >> 16
  DWORD decoder_packet_value_624;                  // +624; 5D3980 payload DWORD >> 16
  DWORD decoder_packet_value_628;                  // +628; 5D3980 payload DWORD >> 16
  DWORD decoder_packet_value_62c;                  // +62c; 5D3980 payload DWORD >> 16
  int decoder_capacity_630;                        // +630; signed 5D3980 capacity comparison
  int decoder_capacity_634;                        // +634; signed 5D3980 capacity comparison
  BYTE *owned_heap_638;                           // +638; freed with free
  BYTE *owned_heap_63c;                           // +63c; freed with free
  BYTE *owned_heap_640;                           // +640; freed with free
  int *motion_row_offsets;                       // +644; 2*height signed entries, starts at -width*height
  MediaDecodedPixelBlock4x4 *decoded_pixel_blocks; // +648; decoder_capacity_634 blocks
  MediaMotionCoordinate *motion_coordinates;     // +64c; decoder_capacity_630 pairs
  int *motion_pixel_offsets;                     // +650; decoder_capacity_630 signed entries
  BYTE *decoder_coefficient_plane_654;            // +654, allocator result
  BYTE *decoder_temporary_plane_a_658;            // +658, allocator result
  BYTE *decoder_temporary_plane_b_65c;            // +65c, allocator result
};

typedef char media_descriptor_extent_check[
    sizeof(MediaDescriptor) == 0x7b4 ? 1 : -1];
#if defined(_MSC_VER) && defined(_M_IX86)
typedef char media_key_poll_member_abi_check[
    sizeof(MediaKeyPollMember) == 4 ? 1 : -1];
#endif
typedef char media_pixel_format_extent_check[
    sizeof(MediaPixelFormat) == 0x20 ? 1 : -1];
typedef char media_conversion_storage_extent_check[
    sizeof(MediaConversionTableStorage) == 0x3d74 ? 1 : -1];
typedef char media_conversion_remap_offset_check[
    offsetof(MediaConversionTableStorage, remap_or_lookup_storage) == 0x70c ? 1 : -1];
typedef char media_conversion_byte3_offset_check[
    offsetof(MediaConversionTableStorage, contribution_byte3) == 0x170c ? 1 : -1];
typedef char media_conversion_transform_reader_offset_check[
    offsetof(MediaConversionTableStorage, transform_bit_state) == 0x100 ? 1 : -1];
typedef char media_transform_bit_state_extent_check[
    sizeof(MediaTransformBitState) == 0x0c ? 1 : -1];
typedef char media_conversion_byte1_offset_check[
    offsetof(MediaConversionTableStorage, contribution_byte1) == 0x1f0c ? 1 : -1];
typedef char media_conversion_byte0_offset_check[
    offsetof(MediaConversionTableStorage, contribution_byte0) == 0x270c ? 1 : -1];
typedef char media_conversion_blend_high_offset_check[
    offsetof(MediaConversionTableStorage, blend_high) == 0x2f0c ? 1 : -1];
typedef char media_conversion_blend_mid_offset_check[
    offsetof(MediaConversionTableStorage, blend_mid) == 0x33d4 ? 1 : -1];
typedef char media_conversion_blend_low_offset_check[
    offsetof(MediaConversionTableStorage, blend_low) == 0x37ec ? 1 : -1];
typedef char media_runtime_extent_check[
    sizeof(MediaRuntime) == 0x660 ? 1 : -1];
typedef char media_runtime_decoder_state_offsets_check[
    offsetof(MediaRuntime, decoder_context_3f8) == 0x3f8 &&
    offsetof(MediaRuntime, decoder_context_3f8) +
            offsetof(MediaDecoderContext, predictor_4f8) == 0x4f8 &&
    offsetof(MediaRuntime, decoder_context_3f8) +
            offsetof(MediaDecoderContext, predictor_4fc) == 0x4fc &&
    offsetof(MediaRuntime, decoder_context_3f8) +
            offsetof(MediaDecoderContext, predictor_500) == 0x500 &&
    offsetof(MediaRuntime, decoder_context_3f8) +
            offsetof(MediaDecoderContext, compressed_input_200) == 0x5f8 &&
    offsetof(MediaRuntime, decoder_context_3f8) +
            offsetof(MediaDecoderContext, bit_position_204) == 0x5fc &&
            offsetof(MediaRuntime, unresolved_word_618) == 0x618 &&
            offsetof(MediaRuntime, unresolved_word_61c) == 0x61c
        ? 1
        : -1];
typedef char media_runtime_decoder_allocation_state_offsets_check[
    offsetof(MediaRuntime, decoder_packet_value_620) == 0x620 &&
    offsetof(MediaRuntime, decoder_packet_value_624) == 0x624 &&
    offsetof(MediaRuntime, decoder_packet_value_628) == 0x628 &&
    offsetof(MediaRuntime, decoder_packet_value_62c) == 0x62c &&
    offsetof(MediaRuntime, decoder_capacity_630) == 0x630 &&
    offsetof(MediaRuntime, decoder_capacity_634) == 0x634 &&
    offsetof(MediaRuntime, owned_heap_638) == 0x638
        ? 1 : -1];
typedef char media_descriptor_decoder_palette_offset_check[
    offsetof(MediaDescriptor, palette_lookup_3b0) == 0x3b0 ? 1 : -1];
typedef char media_record_extent_check[
    sizeof(MediaQueueRecord) == 0x10 ? 1 : -1];

// Existing product-owned mapped callees implemented in movie_media.cpp.
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime, void *unused);
extern "C" int __fastcall sub_5ca970(MediaRuntime *runtime, void *unused);

// ORIGINAL 0x005CAE80: returns the same embedded descriptor passed at arg 3.
extern "C" MediaDescriptor *__cdecl sub_5cae80(
    LPSTR filename, HWND window, MediaDescriptor *descriptor);
extern "C" int __cdecl sub_5cb610(MediaDescriptor *descriptor);
extern "C" int __cdecl sub_5cb660(MediaDescriptor *descriptor);
extern "C" void __cdecl sub_5caab0(void *descriptor);
