// Host projection for actual definitions extracted from movie_media.cpp.
#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

typedef uint32_t DWORD;
typedef uint8_t BYTE;
typedef int32_t HRESULT;
typedef int32_t LONG;
typedef unsigned int UINT;
typedef long long __int64;
typedef void *HMMIO;
typedef char *HPSTR;
#define __cdecl
#define __fastcall

enum {
  DD_OK = 0, DDPF_FOURCC = 4, DDCKEY_SRCBLT = 4,
  DS_OK = 0, DSSCL_PRIORITY = 2, DSCAPS_EMULDRIVER = 0x20,
  DSBPLAY_LOOPING = 1, WAVE_FORMAT_PCM = 1
};
typedef void *HWND;
struct GUID { unsigned char bytes[16]; };
static const GUID IID_IDirectDrawSurface2 = {{0}};
struct DDCAPS { DWORD unused[8]; };
struct DDPIXELFORMAT {
  DWORD dwSize, dwFlags, dwFourCC, dwRGBBitCount;
  DWORD dwRBitMask, dwGBitMask, dwBBitMask, dwRGBAlphaBitMask;
};
struct DDSCAPS { DWORD dwCaps, dwCaps2, dwCaps3, dwCaps4; };
struct DDSURFACEDESC {
  DWORD dwSize, dwFlags, dwHeight, dwWidth, lPitch, dwBackBufferCount;
  DWORD dwMipMapCount, dwAlphaBitDepth, dwReserved;
  void *lpSurface;
  DWORD dwColorKey[8];
  DDPIXELFORMAT ddpfPixelFormat;
  DDSCAPS ddsCaps;
  DWORD dwTextureStage;
};
struct DDCOLORKEY { DWORD dwColorSpaceLowValue, dwColorSpaceHighValue; };
struct DSCAPS { DWORD dwSize, dwFlags; BYTE rest[0x58]; };
struct DSBCAPS { DWORD dwSize, dwFlags, dwBufferBytes; BYTE rest[0x08]; };
struct WAVEFORMATEX {
  unsigned short wFormatTag, nChannels;
  DWORD nSamplesPerSec, nAvgBytesPerSec;
  unsigned short nBlockAlign, wBitsPerSample, cbSize;
};
struct DSBUFFERDESC {
  DWORD dwSize, dwFlags, dwBufferBytes, dwReserved;
  WAVEFORMATEX *lpwfxFormat;
};
struct IDirectSoundBuffer {
  virtual HRESULT GetCaps(DSBCAPS *) = 0;
  virtual HRESULT Play(DWORD, DWORD, DWORD) = 0;
  virtual HRESULT SetFrequency(DWORD) = 0;
  virtual HRESULT SetVolume(LONG) = 0;
  virtual unsigned long Release() = 0;
};
struct IDirectSound {
  virtual HRESULT SetCooperativeLevel(HWND, DWORD) = 0;
  virtual unsigned long Release() = 0;
  virtual HRESULT GetCaps(DSCAPS *) = 0;
  virtual HRESULT CreateSoundBuffer(DSBUFFERDESC *, IDirectSoundBuffer **,
                                    void *) = 0;
};

struct IDirectDrawSurface {
  virtual HRESULT QueryInterface(const GUID &, void **) = 0;
  virtual unsigned long Release() = 0;
};
struct IDirectDrawSurface2 : IDirectDrawSurface {
  virtual HRESULT SetColorKey(DWORD, DDCOLORKEY *) = 0;
};
struct IDirectDraw2 {
  virtual HRESULT CreateSurface(DDSURFACEDESC *, IDirectDrawSurface **,
                                void *) = 0;
};

struct MediaConversionTableStorage {
  DWORD block_scale_entries[64];
  BYTE unresolved[12];
  DWORD packed_component_words[384];
};
MediaConversionTableStorage movie_media_conversion_tables;

struct MediaDescriptor {
  DWORD flags;
  HMMIO stream;
  DWORD stream_gate_7c;
  HWND window;
  IDirectSound *sound_device;
  IDirectSoundBuffer *source_buffer_58;
  IDirectSoundBuffer *play_buffer;
  LONG volume_argument;
};
struct MediaQueueRecord { BYTE *payload; };
struct MediaRuntime {
  MediaDescriptor *descriptor;
  BYTE *input_ring_base;
  BYTE *decoded_ring_base;
  MediaQueueRecord output_records[24];
  MediaQueueRecord read_records[24];
  int format_parameter_5c;
  int format_parameter_54;
  DWORD playback_frequency;
  int format_dispatch_60;
  DWORD release_sound_device, emulated_device, source_buffer_bytes;
  LONG release_optional_buffer;
  DWORD synthetic_cursor_mode, playback_buffer_bytes, playback_cursor;
  DWORD synthetic_playback_cursor;
  LONG buffer_setup_finished;
  int setup_sound_device();
  int create_playback_buffer();
  int release_session_interfaces();
  int release_playback_buffer();
  int parse_initial_format(int);
  int sub_5cae00();
};
struct MediaHResultText { HRESULT result; const char *text; };
static BYTE mmio_bytes[0xc000];
static LONG mmio_result_override = -2;
static int mmio_close_calls;
static LONG mmioRead(HMMIO, HPSTR output, LONG count) {
  if (mmio_result_override != -2) return mmio_result_override;
  memcpy(output, mmio_bytes, count);
  return count;
}
static LONG mmioClose(HMMIO, UINT) { ++mmio_close_calls; return 0; }
static IDirectSound *direct_sound_result;
static HRESULT direct_sound_create_result;
static HRESULT DirectSoundCreate(void *, IDirectSound **out, void *) {
  if (direct_sound_create_result == DS_OK) *out = direct_sound_result;
  return direct_sound_create_result;
}

// ACTUAL_TABLES
// ACTUAL_HRESULT_TABLE
// ACTUAL_HRESULT_FALLBACK
// ACTUAL_HELPERS
// ACTUAL_SUB_5CF120
// ACTUAL_SUB_5CD2D0
// ACTUAL_SUB_5CC860
// ACTUAL_SUB_5CD560
// ACTUAL_SOUND_SETUP
// ACTUAL_SOUND_CREATE
// ACTUAL_SOUND_RELEASE
// ACTUAL_PARSE_FORMAT
// ACTUAL_CLEANUP

struct MockSurface2 : IDirectDrawSurface2 {
  int releases;
  HRESULT query_result, color_key_result;
  IDirectDrawSurface2 *query_result_pointer;
  DWORD last_color_key_flags;
  DDCOLORKEY last_key;
  MockSurface2()
      : releases(0), query_result(DD_OK), color_key_result(DD_OK),
        query_result_pointer(this), last_color_key_flags(0), last_key() {}
  HRESULT QueryInterface(const GUID &, void **out) {
    if (query_result == DD_OK) *out = query_result_pointer;
    return query_result;
  }
  unsigned long Release() { ++releases; return (unsigned long)releases; }
  HRESULT SetColorKey(DWORD flags, DDCOLORKEY *key) {
    last_color_key_flags = flags;
    last_key = *key;
    return color_key_result;
  }
};
struct MockSurface1 : IDirectDrawSurface {
  int releases;
  HRESULT query_result;
  IDirectDrawSurface2 *query_result_pointer;
  MockSurface1()
      : releases(0), query_result(DD_OK), query_result_pointer(0) {}
  HRESULT QueryInterface(const GUID &, void **out) {
    if (query_result == DD_OK) *out = query_result_pointer;
    return query_result;
  }
  unsigned long Release() { ++releases; return (unsigned long)releases; }
};
struct MockDraw2 : IDirectDraw2 {
  HRESULT create_result;
  IDirectDrawSurface *created;
  int calls;
  DDSURFACEDESC observed;
  MockDraw2() : create_result(DD_OK), created(0), calls(0), observed() {}
  HRESULT CreateSurface(DDSURFACEDESC *description,
                        IDirectDrawSurface **out, void *) {
    ++calls;
    observed = *description;
    if (create_result == DD_OK) *out = created;
    return create_result;
  }
};
static char sound_events[128];
static int sound_event_count;
static void sound_event(char value) { sound_events[sound_event_count++] = value; }
static void reset_sound_events() { sound_event_count = 0; }
struct MockBuffer : IDirectSoundBuffer {
  HRESULT caps_result, play_result, frequency_result, volume_result;
  DWORD actual_bytes, frequency;
  LONG volume;
  int releases;
  MockBuffer()
      : caps_result(DS_OK), play_result(DS_OK), frequency_result(DS_OK),
        volume_result(DS_OK), actual_bytes(0x4000), frequency(0), volume(0),
        releases(0) {}
  HRESULT GetCaps(DSBCAPS *caps) {
    sound_event('b');
    caps->dwBufferBytes = actual_bytes;
    return caps_result;
  }
  HRESULT Play(DWORD, DWORD, DWORD) { sound_event('p'); return play_result; }
  HRESULT SetFrequency(DWORD value) {
    sound_event('f'); frequency = value; return frequency_result;
  }
  HRESULT SetVolume(LONG value) {
    sound_event('v'); volume = value; return volume_result;
  }
  unsigned long Release() { sound_event('r'); ++releases; return releases; }
};
struct MockSound : IDirectSound {
  HRESULT cooperative_result, caps_result, create_result;
  DWORD cap_flags;
  MockBuffer *buffer;
  MediaRuntime *frequency_owner;
  DWORD frequency_after_create;
  int releases;
  DSBUFFERDESC observed_description;
  WAVEFORMATEX observed_format;
  MockSound()
      : cooperative_result(DS_OK), caps_result(DS_OK), create_result(DS_OK),
        cap_flags(DSCAPS_EMULDRIVER), buffer(0), frequency_owner(0),
        frequency_after_create(0), releases(0),
        observed_description(), observed_format() {}
  HRESULT SetCooperativeLevel(HWND, DWORD level) {
    sound_event('c');
    assert(level == DSSCL_PRIORITY);
    return cooperative_result;
  }
  unsigned long Release() { sound_event('R'); ++releases; return releases; }
  HRESULT GetCaps(DSCAPS *caps) {
    sound_event('g');
    caps->dwFlags = cap_flags;
    return caps_result;
  }
  HRESULT CreateSoundBuffer(DSBUFFERDESC *desc, IDirectSoundBuffer **out,
                            void *) {
    sound_event('C');
    observed_description = *desc;
    observed_format = *desc->lpwfxFormat;
    if (frequency_owner != 0)
      frequency_owner->playback_frequency = frequency_after_create;
    if (create_result == DS_OK) *out = buffer;
    return create_result;
  }
};

static void test_5cc860_success() {
  MockSurface2 surface;
  MockSurface1 created;
  created.query_result_pointer = &surface;
  MockDraw2 draw;
  draw.created = &created;
  IDirectDrawSurface2 *out = 0;
  assert(sub_5cc860(&draw, &out, 320, 200, 0x32595559) == 0);
  assert(out == &surface && draw.calls == 1 && surface.releases == 0);
  assert(draw.observed.dwSize == 0x6c && draw.observed.dwFlags == 0x1007);
  assert(draw.observed.dwWidth == 320 && draw.observed.dwHeight == 200);
  assert(draw.observed.ddsCaps.dwCaps == 0x80);
  assert(draw.observed.ddpfPixelFormat.dwSize == 0x20);
  assert(draw.observed.ddpfPixelFormat.dwFlags == DDPF_FOURCC);
  assert(draw.observed.ddpfPixelFormat.dwFourCC == 0x32595559);
  assert(surface.last_color_key_flags == DDCKEY_SRCBLT);
  assert(surface.last_key.dwColorSpaceLowValue == 1);
  assert(surface.last_key.dwColorSpaceHighValue == 1);
}

static void test_5cc860_failure_ownership() {
  MockSurface2 surface;
  MockSurface1 created;
  created.query_result_pointer = &surface;
  MockDraw2 draw;
  draw.created = &created;
  IDirectDrawSurface2 *out = 0;
  draw.create_result = -1;
  assert(sub_5cc860(&draw, &out, 1, 2, 3) == 1);
  assert(created.releases == 0 && surface.releases == 0 && out == 0);

  draw.create_result = DD_OK;
  created.query_result = -1;
  assert(sub_5cc860(&draw, &out, 1, 2, 3) == 1);
  assert(created.releases == 1 && surface.releases == 0 && out == 0);

  created.query_result = DD_OK;
  surface.color_key_result = -1;
  assert(sub_5cc860(&draw, &out, 1, 2, 3) == 1);
  // Native releases the CreateSurface interface on this failure path.
  assert(created.releases == 2 && surface.releases == 0 && out == 0);
}

static void test_5cd2d0_tables() {
  DWORD destination[64];
  memset(destination, 0, sizeof(destination));
  assert(sub_5cd2d0(destination, 100) == 0);
  for (int i = 0; i != 64; ++i)
    assert(destination[i] == media_block_transform_weights[i] * 8);

  memset(destination, 0, sizeof(destination));
  assert(sub_5cd2d0(destination, 87) == 0);
  assert(destination[0] == media_block_transform_weights[0] * 8);
  for (int i = 1; i != 64; ++i) {
    const float scale = (107.0f - 87.0f) * 0.0625f;
    const int q = (int)((double)media_block_quant_order[i] *
                        (double)scale * 65536.0);
    const DWORD expected = (DWORD)(((__int64)media_block_transform_weights[i] *
                                    q) >> 16);
    assert(destination[i] == expected);
  }
}

static void test_5cd560_block_layout() {
  const int scales[] = {65536, 32768, 98304, 65535, -32768, -98304,
                        2147483647, (-2147483647 - 1)};
  for (unsigned scale_index = 0; scale_index != sizeof(scales)/sizeof(scales[0]);
       ++scale_index) {
  memset(&movie_media_conversion_tables, 0,
         sizeof(movie_media_conversion_tables));
  movie_media_conversion_tables.block_scale_entries[0] = scales[scale_index];
  // Independent floor-division oracle for the image's round-half-up Q16.
  for (int component = -128; component != 128; ++component) {
    const long long biased = (long long)scales[scale_index] * component + 32768;
    const int expected = (int)(biased >= 0 ? biased / 65536 :
                              -((-biased + 65535) / 65536));
    assert(media_scale_signed_component((signed char)component) == expected);
  }
  DWORD *bank = movie_media_conversion_tables.packed_component_words;
  for (int i = 0; i != 128; ++i) {
    bank[i] = 100 + i;
    bank[128 + i] = 1000 + i;
    bank[256 + i] = 2000 + i;
  }
  const signed char components[6] = {-3, 5, 9, -11, 13, -17};
  BYTE pixels[64 * 16];
  memset(pixels, 0xcc, sizeof(pixels));
  sub_5cd560(components[0], components[1], components[2], components[3],
             components[4], components[5], (int *)pixels, 64);
  int idx[6];
  for (int i = 0; i != 6; ++i) {
    const long long biased = (long long)scales[scale_index] * components[i] + 32768;
    const int rounded = (int)(biased >= 0 ? biased / 65536 :
                             -((-biased + 65535) / 65536));
    const int half = rounded >= 0 ? rounded / 2 : -((-rounded + 1) / 2);
    idx[i] = half & 0x7f;
  }
  const unsigned short block_pixels[4] = {
      (unsigned short)(bank[idx[0]] + bank[128 + idx[5]] + bank[256 + idx[4]]),
      (unsigned short)(bank[idx[1]] + bank[128 + idx[5]] + bank[256 + idx[4]]),
      (unsigned short)(bank[idx[2]] + bank[128 + idx[5]] + bank[256 + idx[4]]),
      (unsigned short)(bank[idx[3]] + bank[128 + idx[5]] + bank[256 + idx[4]])};
  const int base_offsets[4] = {0, 16, 64 * 8, 64 * 8 + 16};
  for (int block = 0; block != 4; ++block) {
    for (int row = 0; row != 8; ++row) {
      for (int column = 0; column != 8; ++column) {
        unsigned short actual;
        memcpy(&actual, pixels + base_offsets[block] + row * 64 + column * 2,
               sizeof(actual));
        assert(actual == block_pixels[block]);
      }
    }
  }
  }
}

static void test_hresult_lookup() {
  assert(strcmp(sub_5cf120((HRESULT)0x88760005U),
                "DDERR_ALREADYINITIALIZED") == 0);
  assert(strcmp(sub_5cf120((HRESULT)0x12345678U),
                "Uknown Error Code") == 0);
}

static void test_format_parser() {
  memset(mmio_bytes, 0, sizeof(mmio_bytes));
  mmio_result_override = -2;
  mmio_bytes[4] = 0xfd;
  mmio_bytes[5] = 0x81; mmio_bytes[6] = 1; mmio_bytes[7] = 0xff;
  mmio_bytes[8] = 0x82; mmio_bytes[9] = 2; mmio_bytes[10] = 0x12; mmio_bytes[11] = 0x34;
  mmio_bytes[12] = 0x83; mmio_bytes[13] = 1; mmio_bytes[14] = 7;
  mmio_bytes[15] = 0x84; mmio_bytes[16] = 1; mmio_bytes[17] = 0x44;
  mmio_bytes[18] = 0x8a;
  MediaDescriptor descriptor = {};
  descriptor.stream = &descriptor;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  assert(runtime.parse_initial_format(19) == 0);
  assert(runtime.format_parameter_5c == -1);
  assert(runtime.format_parameter_54 == 0x1234);
  assert(runtime.format_dispatch_60 == 7);
  assert(runtime.playback_frequency == 0x44);

  mmio_result_override = 0;
  assert(runtime.parse_initial_format(19) == -1);
  mmio_result_override = -1;
  assert(runtime.parse_initial_format(19) == -2);
  mmio_result_override = -2;
}

static void test_ring_cleanup() {
  MediaDescriptor descriptor = {};
  descriptor.stream = &descriptor;
  descriptor.stream_gate_7c = 1;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  runtime.input_ring_base = (BYTE *)malloc(8);
  runtime.decoded_ring_base = (BYTE *)malloc(8);
  for (int i = 0; i != 24; ++i) {
    runtime.read_records[i].payload = (BYTE *)1;
    runtime.output_records[i].payload = (BYTE *)1;
  }
  mmio_close_calls = 0;
  assert(runtime.sub_5cae00() == 0);
  assert(mmio_close_calls == 1);
  for (int i = 0; i != 24; ++i) {
    assert(runtime.read_records[i].payload == 0);
    assert(runtime.output_records[i].payload == 0);
  }
  assert(runtime.input_ring_base != 0 && runtime.decoded_ring_base != 0);

  descriptor.flags = 0x00080000;
  descriptor.stream_gate_7c = 0;
  MediaRuntime runtime_without_stream = {};
  runtime_without_stream.descriptor = &descriptor;
  for (int i = 0; i != 24; ++i) {
    runtime_without_stream.read_records[i].payload = (BYTE *)1;
    runtime_without_stream.output_records[i].payload = (BYTE *)1;
  }
  assert(runtime_without_stream.sub_5cae00() == 0);
  assert(mmio_close_calls == 1);
  for (int i = 0; i != 24; ++i) {
    assert(runtime_without_stream.read_records[i].payload == 0);
    assert(runtime_without_stream.output_records[i].payload == (BYTE *)1);
  }
}

static void test_directsound_setup_create_release_order() {
  MockBuffer buffer;
  MockSound sound;
  sound.buffer = &buffer;
  direct_sound_result = &sound;
  direct_sound_create_result = DS_OK;
  MediaDescriptor descriptor = {};
  descriptor.flags = 0x00800000 | 0x00002000;
  descriptor.window = &descriptor;
  descriptor.source_buffer_58 = 0;
  descriptor.volume_argument = -500;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  runtime.playback_frequency = 22050;
  reset_sound_events();

  assert(runtime.setup_sound_device() == 0);
  assert(descriptor.sound_device == &sound && runtime.release_sound_device == 1);
  assert(runtime.emulated_device == 1 && runtime.source_buffer_bytes == 0x8000);
  assert(sound_event_count == 2 && sound_events[0] == 'c' &&
         sound_events[1] == 'g');

  runtime.source_buffer_bytes = 0x6000;
  assert(runtime.create_playback_buffer() == 0);
  assert(sound.observed_description.dwSize == 0x14);
  assert(sound.observed_description.dwFlags == 0x000100e0);
  assert(sound.observed_description.dwBufferBytes == 0x6000);
  assert(sound.observed_format.wFormatTag == WAVE_FORMAT_PCM);
  assert(sound.observed_format.nChannels == 2);
  assert(sound.observed_format.nSamplesPerSec == 22050);
  assert(sound.observed_format.nAvgBytesPerSec == 22050U << 2);
  assert(sound.observed_format.nBlockAlign == 4 &&
         sound.observed_format.wBitsPerSample == 16);
  assert(buffer.frequency == 22050 && buffer.volume == -500);
  assert(runtime.playback_buffer_bytes == 0x4000 &&
         runtime.buffer_setup_finished == 1);
  assert(sound_event_count == 7);
  assert(sound_events[2] == 'C' && sound_events[3] == 'f' &&
         sound_events[4] == 'v' && sound_events[5] == 'b' &&
         sound_events[6] == 'p');

  runtime.release_optional_buffer = 0;
  assert(runtime.release_session_interfaces() == 0);
  assert(descriptor.sound_device == 0 && sound.releases == 1);
  assert(runtime.release_playback_buffer() == 0);
  assert(descriptor.play_buffer == 0 && buffer.releases == 1);
  assert(sound_events[7] == 'R' && sound_events[8] == 'r');
}

static void test_directsound_create_failure_falls_back() {
  MockSound sound;
  direct_sound_result = &sound;
  direct_sound_create_result = DS_OK;
  sound.cooperative_result = -1;
  MediaDescriptor descriptor = {};
  descriptor.flags = 0x00800000;
  descriptor.source_buffer_58 = 0;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  assert(runtime.setup_sound_device() == 0);
  assert(descriptor.sound_device == 0 && sound.releases == 1);

  descriptor.flags = 0;
  assert(runtime.create_playback_buffer() == 0);
  assert(runtime.synthetic_cursor_mode == 1);
  assert(runtime.synthetic_playback_cursor == 0);
  assert(runtime.playback_buffer_bytes == 0x8000);
  assert(runtime.buffer_setup_finished == 1);
}

static void test_optional_source_buffer_is_not_playback_rate() {
  MockSound sound;
  MockBuffer source, playback;
  source.actual_bytes = 0x2400;
  sound.buffer = &playback;
  MediaDescriptor descriptor = {};
  descriptor.flags = 0x00C00000;
  descriptor.sound_device = &sound;
  descriptor.source_buffer_58 = &source;
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  runtime.playback_frequency = 32000;
  reset_sound_events();
  assert(runtime.setup_sound_device() == 0);
  assert(runtime.source_buffer_bytes == 0x2400);
  assert(source.releases == 0 && descriptor.source_buffer_58 == &source);
  // The original reloads runtime+58 after CreateSoundBuffer. This mock
  // mutation tests the read boundary, not an assertion about real COM events.
  sound.frequency_owner = &runtime;
  sound.frequency_after_create = 48000;
  assert(runtime.create_playback_buffer() == 0);
  assert(sound.observed_format.nSamplesPerSec == 32000);
  assert(sound.observed_format.nAvgBytesPerSec == 32000U * 4);
  assert(playback.frequency == 48000 && source.frequency == 0);
  assert(sound.observed_description.dwBufferBytes == source.actual_bytes);
  runtime.release_optional_buffer = 1;
  assert(runtime.release_session_interfaces() == 0);
  assert(source.releases == 1 && descriptor.source_buffer_58 == 0);
  assert(sound.releases == 0 && descriptor.sound_device == &sound);
  assert(runtime.playback_frequency == 48000);
  assert(runtime.release_playback_buffer() == 0);
  assert(playback.releases == 1);
}

int main() {
  test_5cc860_success();
  test_5cc860_failure_ownership();
  test_5cd2d0_tables();
  test_5cd560_block_layout();
  test_hresult_lookup();
  test_format_parser();
  test_ring_cleanup();
  test_directsound_setup_create_release_order();
  test_directsound_create_failure_falls_back();
  test_optional_source_buffer_is_not_playback_rate();
  std::puts("movie media actual-source host controls passed");
  return 0;
}
