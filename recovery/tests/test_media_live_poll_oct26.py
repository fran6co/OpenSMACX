"""Execute the live media poller at pin-observed descriptor read boundaries."""

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
                return source[start : position + 1]
    raise AssertionError("unterminated definition: " + signature)


PRELUDE = r"""
#include <cassert>
#include <stdint.h>
#include <cstring>

typedef uint32_t DWORD;
typedef int32_t LONG;
typedef void *HMMIO;
#define __fastcall
#define SEEK_SET 0

struct MediaDescriptor {
  DWORD flags;
  DWORD stream_gate_7c;
  DWORD stream_gate_80;
  HMMIO stream;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  LONG channel_state;
  LONG channel_seek_position;
  LONG pending_output_records;
  LONG pending_read_records;
  LONG channel_poll_result;
};

static MediaRuntime *active_runtime;
static MediaDescriptor *sleep_replacement;
static MediaDescriptor *helper_replacement[4];
static int helper_results[4];
static int helper_calls;
static int pending_after_poll;
static int sleep_calls;
static int seek_calls;
static HMMIO sought_stream;
static LONG sought_offset;

static void Sleep(DWORD) {
  ++sleep_calls;
  if (sleep_replacement) active_runtime->descriptor = sleep_replacement;
}
static LONG mmioSeek(HMMIO stream, LONG offset, LONG) {
  ++seek_calls;
  sought_stream = stream;
  sought_offset = offset;
  return 0;
}
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime) {
  int index = helper_calls++;
  if (helper_replacement[index]) runtime->descriptor = helper_replacement[index];
  if (index == 0 && pending_after_poll >= 0)
    runtime->pending_output_records = pending_after_poll;
  return helper_results[index];
}
"""


MAIN = r"""
static void reset_hooks(MediaRuntime &runtime) {
  active_runtime = &runtime;
  sleep_replacement = 0;
  pending_after_poll = -1;
  for (int i = 0; i != 4; ++i) {
    helper_replacement[i] = 0;
    helper_results[i] = 0;
  }
  helper_calls = sleep_calls = seek_calls = 0;
  sought_stream = 0;
  sought_offset = -99;
}

static void test_entry_flags_are_captured_but_seek_stream_is_reloaded() {
  MediaDescriptor entry = {}, after_sleep = {};
  entry.flags = 0x00008000;
  entry.stream = reinterpret_cast<void *>(0x11);
  after_sleep.flags = 0;  // The entry flag still controls whether Sleep occurs.
  after_sleep.stream = reinterpret_cast<void *>(0x22);
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.channel_state = -1;
  runtime.channel_seek_position = 37;
  reset_hooks(runtime);
  sleep_replacement = &after_sleep;
  helper_results[0] = 9;  // Initial channel-state acquisition.
  helper_results[1] = 0;  // Poll result.

  sub_5ca970(&runtime);
  assert(sleep_calls == 1);
  assert(seek_calls == 1);
  assert(sought_stream == after_sleep.stream);
  assert(sought_offset == 37);
  assert(helper_calls == 2);
  assert(runtime.channel_state == 9);
  assert(runtime.channel_poll_result == 0);
}

static void test_poll_result_uses_current_stream_flag_and_stream() {
  MediaDescriptor entry = {}, after_poll = {}, after_sleep = {};
  entry.flags = 0;
  entry.stream = reinterpret_cast<void *>(0x31);
  after_poll.flags = 0x00008000;
  after_poll.stream = reinterpret_cast<void *>(0x32);
  after_sleep.stream = reinterpret_cast<void *>(0x33);
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.channel_state = 4;
  runtime.channel_seek_position = 51;
  reset_hooks(runtime);
  helper_replacement[0] = &after_poll;
  helper_results[0] = -1;
  helper_results[1] = 6;
  sleep_replacement = &after_sleep;

  sub_5ca970(&runtime);
  assert(runtime.channel_poll_result == -1);
  assert(sleep_calls == 1);
  assert(seek_calls == 1);
  assert(sought_stream == after_sleep.stream);
  assert(sought_offset == 51);
  assert(runtime.channel_state == 6);
  assert(helper_calls == 2);
}

static void test_poll_gate_decision_and_clear_reload_owner() {
  MediaDescriptor entry = {}, after_poll = {}, after_second_helper = {};
  entry.stream_gate_7c = 0;
  after_poll.stream_gate_7c = 1;
  after_second_helper.stream_gate_7c = 1;
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.channel_state = 7;
  reset_hooks(runtime);
  helper_replacement[0] = &after_poll;
  helper_results[0] = 1;
  helper_replacement[1] = &after_second_helper;
  helper_results[1] = 1;

  sub_5ca970(&runtime);
  assert(helper_calls == 2);
  assert(runtime.channel_state == 1);
  assert(entry.stream_gate_7c == 0);
  assert(after_poll.stream_gate_7c == 1);
  assert(after_second_helper.stream_gate_7c == 0);
}

static void test_result_two_observes_current_gate_after_poll() {
  MediaDescriptor entry = {}, after_poll = {};
  entry.stream_gate_80 = 0;
  after_poll.stream_gate_80 = 1;
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.channel_state = 7;
  reset_hooks(runtime);
  helper_replacement[0] = &after_poll;
  helper_results[0] = 2;
  // The helper changes the queue after the entry limit check.
  pending_after_poll = 0x16;
  sub_5ca970(&runtime);
  assert(helper_calls == 1);
  assert(runtime.channel_state == 7 && runtime.channel_poll_result == 2);
  assert(entry.stream_gate_80 == 0 && after_poll.stream_gate_80 == 1);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  if (std::strcmp(argv[1], "entry") == 0)
    test_entry_flags_are_captured_but_seek_stream_is_reloaded();
  else if (std::strcmp(argv[1], "poll") == 0)
    test_poll_result_uses_current_stream_flag_and_stream();
  else if (std::strcmp(argv[1], "gate") == 0)
    test_poll_gate_decision_and_clear_reload_owner();
  else if (std::strcmp(argv[1], "result_two") == 0)
    test_result_two_observes_current_gate_after_poll();
  else
    assert(!"unknown case");
  return 0;
}
"""


def projected_source(damaged=False) -> str:
    body = definition(SOURCE.read_text(), 'extern "C" void __fastcall sub_5ca970(')
    if damaged:
        # Cache only the later reads, retaining the actual entry flags alias.
        assert body.count("runtime->descriptor->") == 6
        body = body.replace("runtime->descriptor->", "d.")
    return "\n".join((PRELUDE, body, MAIN))


@pytest.mark.parametrize("damaged", [False, True])
@pytest.mark.parametrize("case", ["entry", "poll", "gate", "result_two"])
def test_actual_source_observes_pinned_live_descriptor_boundaries(tmp_path, case, damaged):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    unit = tmp_path / "movie_media_live_poll.cpp"
    binary = tmp_path / "movie_media_live_poll"
    unit.write_text(projected_source(damaged))
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", str(unit), "-o", str(binary)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary), case], capture_output=True, text=True)
    assert (result.returncode != 0 if damaged else result.returncode == 0), result.stdout + result.stderr
