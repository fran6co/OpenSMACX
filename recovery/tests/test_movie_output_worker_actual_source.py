"""Pin-backed host controls for sub_5cdfa0's output-lock counter branches."""
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

struct PlayBuffer;
struct MediaDescriptor { PlayBuffer *play_buffer; };
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

enum Event { SYNC_ENTRY=1, POSITION_QUERY, BUFFER_LOCK, BUFFER_UNLOCK,
  OUTPUT_LOCK_INCREMENT, OUTPUT_LOCK_EXCHANGE, CONSUMER_INCREMENT,
  PENDING_DECREMENT, WAKEUP_INCREMENT, SYNC_TAIL };
static MediaRuntime *active_runtime;
static DWORD configured_play_position, configured_safe_position;
static DWORD expected_lock_offset, expected_lock_bytes;
static DWORD expected_cursor_at_output_increment;
static HRESULT configured_lock_result;
static int sync_calls, position_calls, lock_calls, unlock_calls;
static int output_lock_increment_calls, output_lock_exchange_calls;
static int consumer_increment_calls, pending_decrement_calls, wakeup_increment_calls;
static int sleep_calls, endthread_calls, event_log[64], event_count;
static unsigned char audio[1024], payload[128];
static void log_event(int event) {
  assert(event_count < 64);
  event_log[event_count++] = event;
}
static int event_position(int event) {
  for (int i=0;i<event_count;++i) if (event_log[i] == event) return i;
  return -1;
}

struct PlayBuffer {
  HRESULT GetCurrentPosition(DWORD *play, DWORD *safe);
  HRESULT Lock(DWORD offset, DWORD bytes, LPVOID *first, DWORD *first_bytes,
               LPVOID *second, DWORD *second_bytes, DWORD flags);
  HRESULT Restore();
  HRESULT Unlock(LPVOID first, DWORD first_bytes, LPVOID second, DWORD second_bytes);
};
static PlayBuffer play_buffer;
HRESULT PlayBuffer::GetCurrentPosition(DWORD *play, DWORD *safe) {
  log_event(POSITION_QUERY); ++position_calls;
  *play=configured_play_position; *safe=configured_safe_position; return 0;
}
HRESULT PlayBuffer::Lock(DWORD offset, DWORD bytes, LPVOID *first, DWORD *first_bytes,
                         LPVOID *second, DWORD *second_bytes, DWORD) {
  log_event(BUFFER_LOCK); ++lock_calls;
  assert(offset==expected_lock_offset && bytes==expected_lock_bytes);
  if (configured_lock_result != DS_OK) return configured_lock_result;
  *first=audio; *first_bytes=bytes; *second=0; *second_bytes=0; return DS_OK;
}
HRESULT PlayBuffer::Restore() { assert(!"no-pending Lock path does not Restore"); return 1; }
HRESULT PlayBuffer::Unlock(LPVOID first, DWORD first_bytes, LPVOID second, DWORD second_bytes) {
  log_event(BUFFER_UNLOCK); ++unlock_calls;
  assert(first==audio && second==0 && second_bytes==0 && first_bytes==expected_lock_bytes);
  return DS_OK;
}
static void Sleep(DWORD) { ++sleep_calls; }
static LONG InterlockedExchange(LONG *p, LONG value) {
  if (p == &active_runtime->sync_word_3b0) {
    if (sync_calls++ == 0) { log_event(SYNC_ENTRY); *p=value; return 1; }
    log_event(SYNC_TAIL); *p=value; return 0;
  }
  assert(p == &active_runtime->output_lock_word);
  log_event(OUTPUT_LOCK_EXCHANGE); ++output_lock_exchange_calls;
  LONG prior=*p; *p=value; return prior;
}
static LONG InterlockedIncrement(LONG *p) {
  if (p == &active_runtime->output_lock_word) {
    log_event(OUTPUT_LOCK_INCREMENT); ++output_lock_increment_calls;
    // Pin 0x5CE263 increments only after the joined no-pending cursor advance.
    assert(active_runtime->playback_cursor == expected_cursor_at_output_increment);
  } else if (p == &active_runtime->output_consumer_cursor) {
    log_event(CONSUMER_INCREMENT); ++consumer_increment_calls;
  } else if (p == &active_runtime->worker_wakeup_counter) {
    log_event(WAKEUP_INCREMENT); ++wakeup_increment_calls;
  } else assert(!"unexpected increment target");
  return ++*p;
}
static LONG InterlockedDecrement(LONG *p) {
  assert(p == &active_runtime->pending_output_records);
  log_event(PENDING_DECREMENT); ++pending_decrement_calls; return --*p;
}
static void _endthread() { ++endthread_calls; }
'''


MAIN = r'''
extern "C" void __cdecl sub_5cdfa0(void *argument);
static MediaDescriptor descriptor;
static MediaRuntime runtime;

static void setup_case(int setup, int synthetic, int pending, DWORD cursor,
                       DWORD synthetic_cursor, DWORD play, DWORD buffer_bytes,
                       DWORD record_bytes, LONG lock_word, HRESULT lock_result) {
  memset(&runtime,0,sizeof(runtime)); memset(audio,0x5a,sizeof(audio));
  for (unsigned i=0;i<sizeof(payload);++i) payload[i]=(unsigned char)(i+1);
  descriptor.play_buffer=&play_buffer; runtime.descriptor=&descriptor;
  runtime.buffer_setup_finished=setup; runtime.synthetic_cursor_mode=synthetic;
  runtime.synthetic_playback_cursor=synthetic_cursor; runtime.playback_buffer_bytes=buffer_bytes;
  runtime.playback_cursor=cursor; runtime.pending_output_records=pending;
  runtime.output_lock_word=lock_word; runtime.output_records[0].payload=payload;
  runtime.output_records[0].field_04=record_bytes; runtime.output_records[0].field_08=7;
  configured_play_position=play; configured_safe_position=play+8;
  configured_lock_result=lock_result; expected_lock_offset=cursor;
  expected_lock_bytes=pending ? record_bytes : play-cursor;
  expected_cursor_at_output_increment=0xffffffffu; active_runtime=&runtime;
  sync_calls=position_calls=lock_calls=unlock_calls=0;
  output_lock_increment_calls=output_lock_exchange_calls=0;
  consumer_increment_calls=pending_decrement_calls=wakeup_increment_calls=0;
  sleep_calls=endthread_calls=event_count=0;
}
static void run_worker() {
  sub_5cdfa0(&runtime);
  assert(sleep_calls==2 && sync_calls==2 && endthread_calls==1);
  assert(wakeup_increment_calls==1 && runtime.worker_wakeup_counter==1);
  assert(event_position(WAKEUP_INCREMENT)<event_position(SYNC_TAIL));
}
static void test_setup_and_space_guards() {
  // Pin 0x5CDFCA..D2: setup-incomplete skips +0x390 and reaches +0x3AC.
  setup_case(0,0,0,0,0,64,128,16,5,DS_OK); run_worker();
  assert(position_calls==0 && lock_calls==0 && output_lock_increment_calls==0);
  assert(output_lock_exchange_calls==0 && runtime.output_lock_word==5);
  // Pin 0x5CE070..077: insufficient available bytes also skips +0x390.
  setup_case(1,0,0,0,0,10,128,11,5,DS_OK); run_worker();
  assert(position_calls==1 && lock_calls==0 && output_lock_increment_calls==0);
  assert(output_lock_exchange_calls==0 && runtime.output_lock_word==5);
}
static void test_pending_record_paths() {
  // Pending synthetic output does no DirectSound work; pin 0x5CE19F exchanges
  // record.field_08 and must not increment the counter.
  setup_case(1,1,1,900,100,0,1024,8,5,DS_OK); run_worker();
  assert(runtime.synthetic_playback_cursor==982 && runtime.playback_cursor==908);
  assert(position_calls==0 && lock_calls==0 && unlock_calls==0);
  assert(runtime.output_lock_word==7 && output_lock_exchange_calls==1);
  assert(output_lock_increment_calls==0 && consumer_increment_calls==1);
  assert(pending_decrement_calls==1 && runtime.pending_output_records==0);
  assert(runtime.output_records[0].payload==0 && runtime.output_records[0].field_04==0);
  assert(event_position(CONSUMER_INCREMENT)<event_position(OUTPUT_LOCK_EXCHANGE));
  assert(event_position(OUTPUT_LOCK_EXCHANGE)<event_position(PENDING_DECREMENT));
  // Pending physical output keeps the same exchange after copy/unlock.
  setup_case(1,0,1,0,0,64,128,8,5,DS_OK); run_worker();
  assert(position_calls==1 && lock_calls==1 && unlock_calls==1);
  assert(runtime.playback_cursor==8 && runtime.output_lock_word==7);
  assert(output_lock_exchange_calls==1 && output_lock_increment_calls==0);
  assert(memcmp(audio,payload,8)==0);
}
static void test_empty_queue_increment_paths() {
  // Synthetic no-pending path increments once after cursor advance.
  setup_case(1,1,0,900,100,0,1024,4,5,DS_OK);
  expected_cursor_at_output_increment=982; run_worker();
  assert(runtime.playback_cursor==982 && runtime.output_lock_word==6);
  assert(position_calls==0 && lock_calls==0 && output_lock_increment_calls==1);
  assert(output_lock_exchange_calls==0);
  assert(event_position(OUTPUT_LOCK_INCREMENT)<event_position(WAKEUP_INCREMENT));
  // Physical success silences bytes, advances, then increments the nonzero word.
  setup_case(1,0,0,0,0,64,128,4,9,DS_OK);
  expected_cursor_at_output_increment=64; run_worker();
  assert(runtime.playback_cursor==64 && runtime.output_lock_word==10);
  assert(position_calls==1 && lock_calls==1 && unlock_calls==1);
  assert(output_lock_increment_calls==1 && output_lock_exchange_calls==0);
  for (int i=0;i<64;++i) assert(audio[i]==0);
  assert(event_position(BUFFER_UNLOCK)<event_position(OUTPUT_LOCK_INCREMENT));
  // Failed physical Lock skips silence/unlock but still advances and increments.
  setup_case(1,0,0,0,0,64,128,4,12,1);
  expected_cursor_at_output_increment=64; run_worker();
  assert(runtime.playback_cursor==64 && runtime.output_lock_word==13);
  assert(position_calls==1 && lock_calls==1 && unlock_calls==0);
  assert(output_lock_increment_calls==1 && output_lock_exchange_calls==0);
}
static void test_signed_cursor_remainders() {
  // All three pin paths perform a DWORD wrapping addition, then signed IDIV.
  // Zero divisors and LONG_MIN/-1 remain untested compiler-effects obligations:
  // the pin uses signed division, but host C++ does not define these cases.
  // 0x80000330 + 64 is negative as LONG; divisor 0xC0000000 is also negative,
  // so signed remainder is 0xC0000370. Unsigned modulo retains 0x80000370.
  setup_case(1,1,1,0x80000330u,0x7FFFFFFEu,0,0xC0000000u,64,2,DS_OK);
  run_worker();
  assert(runtime.synthetic_playback_cursor==0x80000370u);
  assert(runtime.playback_cursor==0xC0000370u);
  assert(runtime.output_lock_word==7 && output_lock_exchange_calls==1);
  assert(output_lock_increment_calls==0);

  setup_case(1,0,0,0x80000330u,0,0x80000370u,0xC0000000u,4,3,DS_OK);
  expected_cursor_at_output_increment=0xC0000370u; run_worker();
  assert(runtime.playback_cursor==0xC0000370u && runtime.output_lock_word==4);
  assert(output_lock_increment_calls==1);

  setup_case(1,1,0,0x80000330u,0x7FFFFFFEu,0,0xC0000000u,4,5,DS_OK);
  expected_cursor_at_output_increment=0xC0000370u; run_worker();
  assert(runtime.synthetic_playback_cursor==0x80000370u);
  assert(runtime.playback_cursor==0xC0000370u && runtime.output_lock_word==6);
  assert(output_lock_increment_calls==1);

  // The addition itself wraps as DWORD before the signed remainder. These
  // sums become zero in pending, physical no-pending, and synthetic arms.
  setup_case(1,1,1,0xFFFFFF00u,0xFFFFFD8Eu,0,0xFFFFFF00u,0x100u,2,DS_OK);
  run_worker();
  assert(runtime.synthetic_playback_cursor==0x100u && runtime.playback_cursor==0);
  assert(runtime.output_lock_word==7 && output_lock_exchange_calls==1);

  setup_case(1,0,0,0xFFFFFF00u,0,0x100u,0xFFFFFF00u,4,3,DS_OK);
  expected_lock_bytes=0x100u;
  expected_cursor_at_output_increment=0; run_worker();
  assert(runtime.playback_cursor==0 && runtime.output_lock_word==4);
  assert(output_lock_increment_calls==1);

  setup_case(1,1,0,0xFFFFFF00u,0xFFFFFD8Eu,0,0xFFFFFF00u,4,5,DS_OK);
  expected_cursor_at_output_increment=0; run_worker();
  assert(runtime.synthetic_playback_cursor==0x100u && runtime.playback_cursor==0);
  assert(runtime.output_lock_word==6 && output_lock_increment_calls==1);
}
int main() {
  test_setup_and_space_guards();
  test_pending_record_paths();
  test_empty_queue_increment_paths();
  test_signed_cursor_remainders();
  return 0;
}
'''


def extract_worker() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    line = function_line(SOURCE, "sub_5cdfa0")
    start = sum(len(part) for part in source.splitlines(keepends=True)[: line - 1])
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match.group(0)), source
    )
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{": depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0: return source[start : end + 1]
    raise AssertionError("unterminated sub_5cdfa0 definition")


def compile_and_run(tmp_path: Path, compiler: str, body: str, expected: bool) -> None:
    tmp_path.mkdir()
    unit = tmp_path / "movie_output_worker.cpp"
    binary = tmp_path / "movie_output_worker"
    unit.write_text("\n".join((PRELUDE, body, MAIN)), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", "-Werror", str(unit), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert (run.returncode == 0) is expected, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_worker_output_lock_counter_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None: pytest.skip(f"{compiler_name} is unavailable")
    current = extract_worker()
    increment = "InterlockedIncrement(&r->output_lock_word);"
    pending_exchange = "InterlockedExchange(&r->output_lock_word, (LONG)record.field_08);"
    assert current.count(increment) == 2 and current.count(pending_exchange) == 1
    compile_and_run(tmp_path / "actual", compiler, current, expected=True)
    reset_zero = current.replace(increment, "InterlockedExchange(&r->output_lock_word, 0);", 2)
    assert reset_zero != current and reset_zero.count(increment) == 0
    compile_and_run(tmp_path / "reset-zero-mutant", compiler, reset_zero, expected=False)
    unconditional = current.replace(
        "      InterlockedIncrement(&r->worker_wakeup_counter);",
        "      InterlockedIncrement(&r->output_lock_word);\n"
        "      InterlockedIncrement(&r->worker_wakeup_counter);", 1)
    assert unconditional != current
    compile_and_run(tmp_path / "unconditional-mutant", compiler, unconditional, expected=False)
    unsigned = current.replace(
        "(LONG)(r->playback_cursor + record.field_04) %\n                (LONG)r->playback_buffer_bytes",
        "(r->playback_cursor + record.field_04) %\n                r->playback_buffer_bytes",
    )
    assert unsigned.count("(LONG)(r->playback_cursor + record.field_04)") == 0
    unsigned = unsigned.replace(
        "(LONG)(r->playback_cursor + (DWORD)available) %\n                (LONG)r->playback_buffer_bytes",
        "(r->playback_cursor + (DWORD)available) %\n                r->playback_buffer_bytes",
    )
    assert unsigned.count("(LONG)(r->playback_cursor + (DWORD)available)") == 0
    compile_and_run(tmp_path / "unsigned-remainder-mutant", compiler, unsigned, expected=False)
