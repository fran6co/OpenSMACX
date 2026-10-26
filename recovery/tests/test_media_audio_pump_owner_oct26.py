"""Host control for sub_5cdfa0's pin-observed descriptor reload frontiers.

The test extracts the complete production definition.  No proposal source or model implementation replaces the actual queue logic.
"""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import function_line


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/movie_media.cpp").is_file())
SOURCE = ROOT / "src/movie_media.cpp"


PRELUDE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef int32_t HRESULT;
typedef void *LPVOID;
#define DS_OK 0

struct MediaDescriptor { struct PlayBuffer *play_buffer; };
struct MediaQueueRecord { unsigned char *payload; DWORD field_04, field_08, field_0c; };
struct MediaRuntime {
  MediaDescriptor *descriptor;
  LONG sync_word_3b0;
  LONG buffer_setup_finished;
  DWORD synthetic_cursor_mode;
  DWORD synthetic_playback_cursor;
  DWORD playback_buffer_bytes;
  DWORD playback_cursor;
  LONG output_consumer_cursor;
  LONG pending_output_records;
  LONG output_lock_word;
  LONG worker_wakeup_counter;
  MediaQueueRecord output_records[24];
};

static MediaRuntime *active_runtime;
static MediaDescriptor descriptors[6];
static unsigned char audio_first[8];
static int sleep_calls, sync_calls, lock_calls;
static int receiver_log[16], receiver_count;
static DWORD play_value, safe_value;
static void record_receiver(int id) { receiver_log[receiver_count++] = id; }

struct PlayBuffer {
  int id;
  HRESULT GetCurrentPosition(DWORD *play, DWORD *safe);
  HRESULT Lock(DWORD offset, DWORD bytes, LPVOID *first, DWORD *first_bytes,
               LPVOID *second, DWORD *second_bytes, DWORD flags);
  HRESULT Restore();
  HRESULT Unlock(LPVOID first, DWORD first_bytes,
                 LPVOID second, DWORD second_bytes);
};

static PlayBuffer buffers[6] = {{1},{2},{3},{4},{5},{6}};

HRESULT PlayBuffer::GetCurrentPosition(DWORD *play, DWORD *safe) {
  record_receiver(id);
  *play = play_value = 64;
  *safe = safe_value = 96;
  active_runtime->descriptor = &descriptors[2];
  return 0;
}
HRESULT PlayBuffer::Lock(DWORD, DWORD bytes, LPVOID *first, DWORD *first_bytes,
                         LPVOID *second, DWORD *second_bytes, DWORD) {
  record_receiver(id);
  ++lock_calls;
  if (id == 3 && lock_calls == 1) {
    active_runtime->descriptor = &descriptors[3];
    return 1;  // Original path restores and retries on every non-DS_OK result.
  }
  assert(id == 5 && lock_calls == 2 && bytes == 4);
  *first = audio_first;
  *first_bytes = 4;
  *second = 0;
  *second_bytes = 0;
  active_runtime->descriptor = &descriptors[5];
  return 0;
}
HRESULT PlayBuffer::Restore() {
  record_receiver(id);
  assert(id == 4);
  active_runtime->descriptor = &descriptors[4];
  return 0;
}
HRESULT PlayBuffer::Unlock(LPVOID first, DWORD first_bytes,
                           LPVOID second, DWORD second_bytes) {
  record_receiver(id);
  assert(id == 6);
  // Lock's output pointers/counts must survive the descriptor reloads.
  assert(first == audio_first && first_bytes == 4);
  assert(second == 0 && second_bytes == 0);
  return 0;
}

static void Sleep(DWORD) {
  ++sleep_calls;
  if (sleep_calls == 1) active_runtime->descriptor = &descriptors[1];
}
static LONG InterlockedExchange(LONG *p, LONG value) {
  if (p == &active_runtime->sync_word_3b0) {
    LONG prior = sync_calls++ == 0 ? 1 : 0;
    *p = value;
    return prior;  // Enter one loop pass, then leave at its tail check.
  }
  LONG prior = *p;
  *p = value;
  return prior;
}
static LONG InterlockedIncrement(LONG *p) { return ++*p; }
static LONG InterlockedDecrement(LONG *p) { return --*p; }
static void _endthread() {}
'''


MAIN = r'''
int main() {
  MediaRuntime runtime = {};
  unsigned char payload[4] = {0x31, 0x52, 0x73, 0x94};
  runtime.descriptor = &descriptors[0];
  runtime.buffer_setup_finished = 1;
  runtime.synthetic_cursor_mode = 0;
  runtime.playback_buffer_bytes = 128;
  runtime.playback_cursor = 0;
  runtime.pending_output_records = 1;
  runtime.output_records[0].payload = payload;
  runtime.output_records[0].field_04 = 4;
  runtime.output_records[0].field_08 = 7;
  active_runtime = &runtime;
  for (int i = 0; i != 6; ++i) descriptors[i].play_buffer = &buffers[i];
  sleep_calls = sync_calls = lock_calls = receiver_count = 0;
  memset(audio_first, 0, sizeof(audio_first));

  sub_5cdfa0(&runtime);

  assert(sleep_calls == 2);
  assert(sync_calls == 2);
  assert(lock_calls == 2);
  assert(receiver_count == 5);
  const int expected[] = {2, 3, 4, 5, 6};
  // GetCurrentPosition, first Lock, Restore, retry Lock, Unlock.
  for (int i = 0; i != 5; ++i) assert(receiver_log[i] == expected[i]);
  assert(play_value == 64 && safe_value == 96);
  assert(memcmp(audio_first, payload, 4) == 0);
  assert(runtime.playback_cursor == 4);
  assert(runtime.output_consumer_cursor == 1);
  assert(runtime.pending_output_records == 0);
  assert(runtime.output_lock_word == 7);
  assert(runtime.output_records[0].payload == 0);
  assert(runtime.output_records[0].field_04 == 0);
  assert(runtime.worker_wakeup_counter == 1);
  return 0;
}
'''


def extract_worker() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    line = function_line(SOURCE, "sub_5cdfa0")
    start = sum(len(part) for part in source.splitlines(keepends=True)[:line - 1])
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m.group(0)), source
    )
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated sub_5cdfa0 definition")


def stale_owner_mutant(candidate: str) -> str:
    anchor = "  MediaRuntime *r = (MediaRuntime *)argument;\n"
    assert candidate.count(anchor) == 1
    result = candidate.replace(anchor, anchor + "  MediaDescriptor &d = *r->descriptor;\n", 1)
    result = result.replace("r->descriptor->play_buffer", "d.play_buffer")
    assert result.count("d.play_buffer") == 7
    return result


def compile_and_run(tmp_path: Path, compiler: str, body: str, expected: bool) -> None:
    tmp_path.mkdir()
    unit = tmp_path / "media_worker.cpp"
    binary = tmp_path / "media_worker"
    unit.write_text("\n".join((PRELUDE, body, MAIN)), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", str(unit), "-o", str(binary)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True)
    assert (run.returncode == 0) is expected, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_pump_reloads_owners_and_cached_owner_mutant_fails(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    current = extract_worker()
    stale = stale_owner_mutant(current)
    compile_and_run(tmp_path / "actual", compiler, current, expected=True)
    compile_and_run(tmp_path / "stale-mutant", compiler, stale, expected=False)
