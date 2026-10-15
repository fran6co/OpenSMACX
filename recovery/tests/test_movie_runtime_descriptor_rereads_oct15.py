"""Pinned descriptor reload boundaries, exercised from actual movie_media.cpp bodies."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/movie_media.cpp"


def definition(source: str, signature: str) -> str:
    start = source.rindex(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated definition: " + signature)


def declaration(source: str, signature: str) -> str:
    start = source.index(signature)
    return source[start:source.index(";", start) + 1]


def initializer(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:source.index(";", position) + 1]
    raise AssertionError("unterminated initializer: " + signature)


PRELUDE = r"""
#include <cassert>
#include <cstring>

typedef unsigned int DWORD;
typedef int HRESULT;
typedef void *HMMIO;
typedef unsigned int UINT;
#define __fastcall
enum { DSOUND_GATE = 0x00800000, STREAM_CLOSE_GATE = 0x00080000 };

struct IDirectSoundBuffer { virtual unsigned long Release() = 0; };
struct IDirectSound { virtual unsigned long Release() = 0; };
struct MediaDescriptor {
  DWORD flags;
  IDirectSoundBuffer *source_buffer_58;
  IDirectSound *sound_device;
  IDirectSoundBuffer *play_buffer;
  DWORD stream_gate_7c;
  HMMIO stream;
};
struct MediaQueueRecord { void *payload; };
struct MediaRuntime {
  MediaDescriptor *descriptor;
  int release_optional_buffer;
  int release_sound_device;
  void *input_ring_base;
  void *decoded_ring_base;
  MediaQueueRecord read_records[24];
  MediaQueueRecord output_records[24];
  int release_session_interfaces();
  int release_playback_buffer();
  int sub_5cae00();
};

static MediaRuntime *callback_runtime;
static MediaDescriptor *callback_descriptor;
static bool mutate_from_mmio_close;
static bool mutate_from_free;
static int mmio_close_calls;
static int mmioClose(HMMIO, UINT) {
  ++mmio_close_calls;
  if (mutate_from_mmio_close) callback_runtime->descriptor = callback_descriptor;
  return 0;
}
static void test_free(void *) {
  if (mutate_from_free) {
    callback_runtime->descriptor = callback_descriptor;
    mutate_from_free = false;
  }
}

struct MockBuffer : IDirectSoundBuffer {
  int releases;
  MediaRuntime *runtime_to_mutate;
  MediaDescriptor *replacement;
  MockBuffer() : releases(0), runtime_to_mutate(0), replacement(0) {}
  unsigned long Release() {
    ++releases;
    if (runtime_to_mutate) runtime_to_mutate->descriptor = replacement;
    return 0;
  }
};
struct MockDevice : IDirectSound {
  int releases;
  MediaRuntime *runtime_to_mutate;
  MediaDescriptor *replacement;
  MockDevice() : releases(0), runtime_to_mutate(0), replacement(0) {}
  unsigned long Release() {
    ++releases;
    if (runtime_to_mutate) runtime_to_mutate->descriptor = replacement;
    return 0;
  }
};
"""


MAIN = r"""
static void test_session_release_reloads_owner_after_buffer_callback() {
  MediaDescriptor old_owner = {}, live_owner = {};
  MockBuffer source;
  MockDevice old_device, live_device;
  old_owner.flags = DSOUND_GATE;
  old_owner.source_buffer_58 = &source;
  old_owner.sound_device = &old_device;
  live_owner.source_buffer_58 = &source;
  live_owner.sound_device = &live_device;
  source.runtime_to_mutate = 0;
  source.replacement = &live_owner;
  MediaRuntime runtime = {};
  runtime.descriptor = &old_owner;
  runtime.release_optional_buffer = 1;
  runtime.release_sound_device = 1;
  source.runtime_to_mutate = &runtime;

  assert(runtime.release_session_interfaces() == 0);
  assert(source.releases == 1);
  assert(old_owner.source_buffer_58 == &source);
  assert(live_owner.source_buffer_58 == 0);
  assert(old_device.releases == 0 && old_owner.sound_device == &old_device);
  assert(live_device.releases == 1 && live_owner.sound_device == 0);
}

static void test_playback_release_reloads_owner_after_release() {
  MediaDescriptor old_owner = {}, live_owner = {};
  MockBuffer playback;
  old_owner.play_buffer = &playback;
  live_owner.play_buffer = &playback;
  MediaRuntime runtime = {};
  runtime.descriptor = &old_owner;
  playback.runtime_to_mutate = &runtime;
  playback.replacement = &live_owner;

  assert(runtime.release_playback_buffer() == 0);
  assert(playback.releases == 1);
  assert(old_owner.play_buffer == &playback);
  assert(live_owner.play_buffer == 0);
}

static void test_device_release_clears_the_owner_it_released() {
  MediaDescriptor owner = {}, replacement = {};
  MockDevice device, replacement_device;
  owner.flags = DSOUND_GATE;
  owner.sound_device = &device;
  replacement.sound_device = &replacement_device;
  MediaRuntime runtime = {};
  runtime.descriptor = &owner;
  runtime.release_sound_device = 1;
  device.runtime_to_mutate = &runtime;
  device.replacement = &replacement;

  assert(runtime.release_session_interfaces() == 0);
  assert(device.releases == 1);
  assert(owner.sound_device == 0);
  assert(replacement.sound_device == &replacement_device);
}

static void reset_cleanup(MediaRuntime &runtime, MediaDescriptor &old_owner,
                          MediaDescriptor &live_owner) {
  old_owner = MediaDescriptor();
  live_owner = MediaDescriptor();
  runtime = MediaRuntime();
  runtime.descriptor = &old_owner;
  old_owner.stream = reinterpret_cast<HMMIO>(1);
  old_owner.stream_gate_7c = 0;
  live_owner.stream_gate_7c = 1;
  runtime.input_ring_base = reinterpret_cast<void *>(2);
  runtime.decoded_ring_base = reinterpret_cast<void *>(3);
  for (int i = 0; i != 24; ++i) {
    runtime.read_records[i].payload = reinterpret_cast<void *>(4 + i);
    runtime.output_records[i].payload = reinterpret_cast<void *>(40 + i);
  }
  callback_runtime = &runtime;
  callback_descriptor = &live_owner;
  mutate_from_mmio_close = false;
  mutate_from_free = false;
  mmio_close_calls = 0;
}

static void assert_cleanup_owner(MediaRuntime &runtime, MediaDescriptor &old_owner,
                                 MediaDescriptor &live_owner) {
  assert(mmio_close_calls == 1);
  for (int i = 0; i != 24; ++i) {
    assert(runtime.read_records[i].payload == 0);
    assert(old_owner.stream_gate_7c == 0);
    assert(live_owner.stream_gate_7c == 1);
    assert(runtime.output_records[i].payload == 0);
  }
  // The original frees these allocations without clearing the stored bases.
  assert(runtime.input_ring_base == reinterpret_cast<void *>(2));
  assert(runtime.decoded_ring_base == reinterpret_cast<void *>(3));
}

static void test_cleanup_reloads_after_mmio_close() {
  MediaRuntime runtime;
  MediaDescriptor old_owner, live_owner;
  reset_cleanup(runtime, old_owner, live_owner);
  mutate_from_mmio_close = true;
  assert(runtime.sub_5cae00() == 0);
  assert_cleanup_owner(runtime, old_owner, live_owner);
}

static void test_cleanup_reloads_after_free_callback() {
  MediaRuntime runtime;
  MediaDescriptor old_owner, live_owner;
  reset_cleanup(runtime, old_owner, live_owner);
  mutate_from_free = true;
  assert(runtime.sub_5cae00() == 0);
  assert_cleanup_owner(runtime, old_owner, live_owner);
}

static void test_hresult_lookup_walks_the_full_bounded_table() {
  const unsigned int count = sizeof(media_hresult_texts) / sizeof(media_hresult_texts[0]);
  assert(count == 92);
  for (unsigned int i = 0; i < count; ++i)
    assert(std::strcmp(sub_5cf120(media_hresult_texts[i].result),
                       media_hresult_texts[i].text) == 0);
  assert(std::strcmp(sub_5cf120(0x12345678), media_hresult_unknown) == 0);
}

int main() {
  test_session_release_reloads_owner_after_buffer_callback();
  test_playback_release_reloads_owner_after_release();
  test_device_release_clears_the_owner_it_released();
  test_cleanup_reloads_after_mmio_close();
  test_cleanup_reloads_after_free_callback();
  test_hresult_lookup_walks_the_full_bounded_table();
  return 0;
}
"""


def projected_source(mutant=None) -> str:
    source = SOURCE.read_text()
    bodies = {
        "session": definition(source, "int MediaRuntime::release_session_interfaces()"),
        "playback": definition(source, "int MediaRuntime::release_playback_buffer()"),
        "cleanup": definition(source, "int MediaRuntime::sub_5cae00()"),
        "lookup": definition(source, 'extern "C" char *__fastcall sub_5cf120('),
    }
    if mutant:
        key, old, new = mutant
        assert bodies[key].count(old) == 1, "mutation anchor absent: " + old
        bodies[key] = bodies[key].replace(old, new, 1)
    strings = "\n".join((
        "struct MediaHResultText { HRESULT result; const char *text; };",
        initializer(source, "static const MediaHResultText media_hresult_texts[]"),
        declaration(source, "static const char media_hresult_unknown[]"),
    ))
    cleanup = "#define free test_free\n" + bodies["cleanup"] + "\n#undef free"
    return "\n".join((PRELUDE, strings, bodies["lookup"], bodies["session"],
                      bodies["playback"], cleanup, MAIN))


def build_and_run(tmp_path: Path, mutant=None) -> subprocess.CompletedProcess:
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    unit = tmp_path / ("movie_runtime_mutant.cpp" if mutant else "movie_runtime_actual.cpp")
    binary = tmp_path / ("movie_runtime_mutant" if mutant else "movie_runtime_actual")
    unit.write_text(projected_source(mutant))
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", str(unit), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_bodies_preserve_pinned_descriptor_reload_boundaries(tmp_path):
    result = build_and_run(tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("mutant", [
    ("session", "descriptor->source_buffer_58 = 0;", "source_owner->source_buffer_58 = 0;"),
    ("playback", "descriptor->play_buffer = 0;", "playback_owner->play_buffer = 0;"),
    ("session", "device_owner->sound_device = 0;", "descriptor->sound_device = 0;"),
    ("cleanup", "MediaDescriptor *output_owner = descriptor;",
     "MediaDescriptor *output_owner = stream_owner;"),
])
def test_cached_descriptor_mutants_fail_callback_controls(tmp_path, mutant):
    result = build_and_run(tmp_path, mutant)
    assert result.returncode != 0, "control did not detect cached descriptor mutant"
