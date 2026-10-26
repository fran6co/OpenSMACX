"""Proposal control for pinned live-descriptor accesses in sub_5ca310.

This compiles the complete function extracted from installed src/movie_media.cpp.
The negative control caches the entry owner in the extracted definition.
The decoder branches are compiled but not executed by these owner-boundary cases.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import function_line


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/movie_media.cpp").is_file())
SOURCE = ROOT / "src/movie_media.cpp"


HOST = r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>
#define __fastcall
#define __cdecl
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef unsigned char BYTE;
typedef void *HMMIO;
typedef char *HPSTR;
#define SEEK_CUR 1
#define SEEK_SET 0

struct MediaDescriptor {
  DWORD flags;
  HMMIO stream;
  DWORD stream_gate_80;
};
struct MediaQueueRecord {
  BYTE *payload;
  DWORD field_04, field_08, field_0c;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  int output_counter;
  int channel_seek_position;
  LONG stream_eof;
  int format_dispatch_60;
  int format_parameter_54;
  BYTE *decoded_ring_current;
  BYTE *decoded_ring_base;
  int decoder_state_3d8, decoder_state_3e0, decoder_state_3dc, decoder_state_3e4;
  int decoder_state_3e8, decoder_state_3f0;
  int channel_state;
  int output_producer_cursor;
  MediaQueueRecord output_records[24];
  LONG read_counter;
  LONG pending_output_records;
};

static MediaRuntime *active_runtime;
static MediaDescriptor *replace_after_seek;
static MediaDescriptor *replace_after_read;
static HMMIO seen_seek_stream;
static HMMIO seen_read_stream;
static int seek_calls;
static int read_calls;
static LONG mmioSeek(HMMIO stream, LONG offset, int origin) {
  seen_seek_stream = stream;
  ++seek_calls;
  if (replace_after_seek) active_runtime->descriptor = replace_after_seek;
  return 8;
}
static LONG mmioRead(HMMIO stream, HPSTR buffer, LONG count) {
  seen_read_stream = stream;
  ++read_calls;
  *(DWORD *)buffer = 4; // default format path consumes this encoded byte count
  if (replace_after_read) active_runtime->descriptor = replace_after_read;
  return 4;
}
static LONG InterlockedIncrement(LONG *value) { return ++*value; }
extern "C" void __cdecl sub_5cdd10(int, int *, int *, int *, int *, int *, int, unsigned int *) {}
extern "C" void __cdecl sub_5cdb90(int, int *, int *, int *, int, unsigned int *) {}
extern "C" void __cdecl sub_5ce6e3(unsigned char *, short *, int, int *, int *) {}
__BODY__

static int run_seek_owner_change() {
  BYTE ring[32] = {};
  MediaDescriptor entry = {0x8000, (HMMIO)0x101, 0};
  MediaDescriptor after_seek = {0, (HMMIO)0x202, 0};
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.decoded_ring_current = ring;
  runtime.decoded_ring_base = ring;
  runtime.format_dispatch_60 = 99;
  runtime.output_producer_cursor = 0;
  runtime.channel_state = 0;
  active_runtime = &runtime;
  replace_after_seek = &after_seek;
  replace_after_read = 0;
  seen_seek_stream = seen_read_stream = 0;
  seek_calls = read_calls = 0;
  if (sub_5ca310(&runtime, 4) != 2) return 1;
  if (seek_calls != 1 || seen_seek_stream != entry.stream) return 2;
  if (read_calls != 1 || seen_read_stream != after_seek.stream) return 3;
  return 0;
}

static int run_read_owner_change() {
  BYTE ring[32] = {};
  MediaDescriptor entry = {0, (HMMIO)0x303, 0};
  MediaDescriptor after_read = {0, (HMMIO)0x404, 0};
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.decoded_ring_current = ring;
  runtime.decoded_ring_base = ring;
  runtime.format_dispatch_60 = 99;
  runtime.output_counter = 1; // native skips the optional seek branch
  runtime.channel_state = 2;
  active_runtime = &runtime;
  replace_after_seek = 0;
  replace_after_read = &after_read;
  seen_seek_stream = seen_read_stream = 0;
  seek_calls = read_calls = 0;
  if (sub_5ca310(&runtime, 4) != 2) return 10;
  if (seek_calls != 0 || read_calls != 1 || seen_read_stream != entry.stream) return 11;
  if (entry.stream_gate_80 != 0 || after_read.stream_gate_80 != 1) return 12;
  if (runtime.read_counter != 1) return 13;
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 2) return 90;
  int result = argv[1][0] == 's' ? run_seek_owner_change() : run_read_owner_change();
  if (result) fprintf(stderr, "owner-change control failed at %d\n", result);
  return result;
}
'''


def installed_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    line = function_line(SOURCE, "sub_5ca310")
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
                return source[start:end + 1]
    raise AssertionError("unterminated sub_5ca310 definition")


def cached_owner_mutant(body: str) -> str:
    opening = body.index("{") + 1
    body = body[:opening] + "\n  MediaDescriptor *cached_owner = r->descriptor;" + body[opening:]
    assert body.count("read_descriptor->stream") == 1
    assert body.count("r->descriptor->stream_gate_80") == 2
    return body.replace("read_descriptor->stream", "cached_owner->stream").replace(
        "r->descriptor->stream_gate_80", "cached_owner->stream_gate_80")


def run_host(compiler: str, tmp_path: Path, body: str, case: str) -> subprocess.CompletedProcess[str]:
    exe = shutil.which(compiler)
    if not exe:
        pytest.skip(f"{compiler} is unavailable")
    source_file = tmp_path / f"media-5ca310-{case}-{compiler.replace('+', 'p')}.cpp"
    binary = source_file.with_suffix("")
    source_file.write_text(HOST.replace("__BODY__", body), encoding="utf-8")
    build = subprocess.run(
        # The production is x86 and contains pointer-to-32-bit casts in
        # non-selected decoder branches. On a 64-bit host, GCC's permissive
        # mode only admits those dormant source expressions; the test never
        # executes them. This is a host harness setting, not a project flag.
        [exe, "-std=c++98", "-O0", "-fpermissive", str(source_file), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(binary), case], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("damaged", [False, True])
@pytest.mark.parametrize("case", ["seek", "read"])
def test_actual_producer_observes_live_descriptor_boundaries(case, damaged, tmp_path):
    body = installed_body()
    result = run_host("g++", tmp_path, cached_owner_mutant(body) if damaged else body, case)
    assert (result.returncode != 0 if damaged else result.returncode == 0), result.stdout + result.stderr
