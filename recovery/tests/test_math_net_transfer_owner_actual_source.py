"""Self-contained host controls for the installed five-function owner batch.

This candidate is intentionally runnable after promotion to recovery/tests.
It reads only the checked-out owner source and the named historical Git leaf;
it imports no proposal emitter and never compiles an in-memory projected patch.
Host execution checks C++ semantics, not the original VC6 ABI or game runtime.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/netdaemon.cpp").is_file())
BASELINE = "5fe79d193062cacf6b9eb9c5b901f9a0501cd2d8"
ADDRESSES = (0x63BAC0, 0x63BEB0, 0x63BEE0, 0x634280, 0x634330)
OWNER_PATHS = {
    0x63BAC0: "src/general.cpp",
    0x63BEB0: "src/net_session_picker.cpp",
    0x63BEE0: "src/net_session_picker.cpp",
    0x634280: "src/netdaemon.cpp",
    0x634330: "src/netdaemon.cpp",
}


def _reader():
    # Use the installed source grammar. Do not load an ignored packet module.
    from match_recovery.source import read_file, read_text

    return read_file, read_text


def _historical_records():
    _, read_text = _reader()
    raw = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/leaf_recoveries.cpp"],
        cwd=ROOT,
        text=True,
    )
    rows = read_text(raw, ROOT / "src/leaf_recoveries.cpp")
    records = {row.address: row for row in rows if row.address in ADDRESSES}
    assert set(records) == set(ADDRESSES)
    return records


def _current_records():
    read_file, _ = _reader()
    records = {}
    for address, relative in OWNER_PATHS.items():
        path = ROOT / relative
        matches = [row for row in read_file(path) if row.address == address]
        assert len(matches) == 1, f"{relative}: expected one {address:08X} mapping"
        row = matches[0]
        assert not row.extract_error, (relative, row.extract_error)
        records[address] = row
    # These identities must no longer also be carried by the transitional leaf.
    leaf = ROOT / "src/leaf_recoveries.cpp"
    assert not (set(row.address for row in read_file(leaf)) & set(ADDRESSES))
    return records


def test_original_identity_spans_claims_and_lessons_survive_owner_transfer():
    old = _historical_records()
    current = _current_records()
    durable = (
        "address", "name", "mode", "state", "image_spans", "kind",
        "exclusion", "byte_exact", "semantic", "recipe", "levers",
        "ruled_out", "unrecoverable", "deferred",
    )
    for address in ADDRESSES:
        for field in durable:
            assert getattr(current[address], field) == getattr(old[address], field), (
                f"{address:08X} lost historical {field}"
            )
        from match_recovery.source import SourceRole
        assert old[address].source_role is SourceRole.SCAFFOLD
        assert current[address].source_role is SourceRole.UNCLASSIFIED
        assert current[address].source_role_reason == ""
        assert current[address].path == ROOT / OWNER_PATHS[address]
        old_lessons = tuple((item.kind, item.key, item.prose)
                            for item in (old[address].origin or ()))
        new_lessons = tuple((item.kind, item.key, item.prose)
                            for item in (current[address].origin or ()))
        assert new_lessons == old_lessons, f"{address:08X} changed lesson prose"
    # The disproven inflate seed interpretation remains historical evidence,
    # while the current binding retains the pinned correction next to it.
    inflate_old = old[0x634330]
    assert any("second argument is never read" in note
               for note in inflate_old.ruled_out)
    net_text = (ROOT / "src/netdaemon.cpp").read_text()
    marker = net_text.index("// ORIGINAL: 0x00634330")
    envelope = net_text[marker:net_text.find("int __cdecl netdaemon_inflate_buffer", marker)]
    assert "second argument is never read" in envelope
    assert "Pinned correction: 0x00534C81" in envelope


def _function_body(relative: str, signature: str) -> str:
    """Extract a real definition, with the installed lexer locating its line."""
    from match_recovery.source import function_line

    path = ROOT / relative
    source = path.read_text()
    line = function_line(path, signature)
    start = sum(len(part) + 1 for part in source.splitlines()[: line - 1])
    # function_line is code-aware and distinguishes the definition from its
    # prototypes/calls. This small balanced-brace scan handles its body while
    # ignoring braces in comments and quoted literals.
    opening = source.find("{", start)
    assert opening >= 0, signature
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "code"
            index += 1
            continue
        if state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
            else:
                index += 1
            continue
        if state in ("string", "char"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
            index += 1
            continue
        if char == "/" and nxt == "/":
            state = "line"
            index += 2
            continue
        if char == "/" and nxt == "*":
            state = "block"
            index += 2
            continue
        if char == '"':
            state = "string"
            index += 1
            continue
        if char == "'":
            state = "char"
            index += 1
            continue
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
        index += 1
    raise AssertionError(f"unclosed C++ body for {signature}")


def test_real_call_paths_use_both_recovered_transfer_operations():
    send = _function_body("src/netdaemon.cpp", "NetDaemon::send_files")
    dispatch = _function_body("src/netdaemon.cpp", "NetDaemon::process_message")
    transfer = _function_body(
        "src/netdaemon.cpp", "NetDaemon::process_file_transfer_message"
    )
    assert "netdaemon_deflate_buffer(static_cast<unsigned char *>(compressed)," in send
    assert "netdaemon_inflate_buffer(mapped, &decompressed_size," in transfer
    assert "static_cast<unsigned int>(compressed_size)" in transfer
    assert "uncompress(" not in transfer
    assert "process_file_transfer_message(message, message_size, sender_id," in dispatch


def _compile_and_run(source: str) -> None:
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    if not compilers:
        pytest.skip("no host GCC or Clang available")
    with tempfile.TemporaryDirectory(prefix="osmx-five-owner-source-") as td:
        cpp = Path(td) / "actual_owner.cpp"
        cpp.write_text(source)
        for compiler in compilers:
            executable = Path(td) / ("control-" + compiler.replace("+", "p"))
            built = subprocess.run(
                [compiler, "-std=c++98", "-O0", str(cpp), "-o", str(executable)],
                text=True,
                capture_output=True,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(executable)], text=True, capture_output=True)
            assert ran.returncode == 0, ran.stdout + ran.stderr


def test_actual_owner_definitions_preserve_wrap_scale_and_zlib_paths():
    cos_body = _function_body("src/general.cpp", "cos")
    width_body = _function_body(
        "src/net_session_picker.cpp", "NetSessionPickerWin::scale_client_width"
    )
    height_body = _function_body(
        "src/net_session_picker.cpp", "NetSessionPickerWin::scale_client_height"
    )
    deflate_body = _function_body(
        "src/netdaemon.cpp", "netdaemon_deflate_buffer"
    )
    inflate_body = _function_body(
        "src/netdaemon.cpp", "netdaemon_inflate_buffer"
    )
    assert "stream.opaque" not in deflate_body
    assert "stream.opaque" not in inflate_body

    source = r'''#include <cassert>
#include <stdint.h>
#include <cstring>
#include <limits.h>
#define __cdecl
typedef unsigned char Bytef;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef unsigned long uLongf;
struct OpaqueCanary {
  void *value; int writes;
  OpaqueCanary() : value(reinterpret_cast<void *>(0xC0FFEE)), writes(0) {}
  OpaqueCanary &operator=(void *p) { value = p; ++writes; return *this; }
  operator void *() const { return value; }
};
struct z_stream {
  Bytef *next_in; uInt avail_in; uLong total_in;
  Bytef *next_out; uInt avail_out; uLong total_out;
  void *zalloc; void *zfree; OpaqueCanary opaque;
};
#define Z_NULL 0
#define ZLIB_VERSION "1.0.2"
enum { Z_STREAM_END = 1, Z_BUF_ERROR = -5, Z_DATA_ERROR = -3,
       Z_NEED_DICT = 2, Z_FINISH = 4 };
static int init_result, operation_result, end_result;
static int init_calls, operation_calls, end_calls;
static int init_level, deflate_init_level;
static Bytef *expected_in, *expected_out;
static uInt expected_avail_in, expected_avail_out;
static uLong produced;
static int opaque_untouched;
int deflateInit_(z_stream *s, int level, const char *version, int size) {
  assert(strcmp(version, ZLIB_VERSION) == 0);
  assert(size == static_cast<int>(sizeof(z_stream)));
  assert(s->opaque.writes == 0 && s->opaque.value == reinterpret_cast<void *>(0xC0FFEE));
  opaque_untouched = 1; ++init_calls; init_level = level; return init_result;
}
int inflateInit_(z_stream *s, const char *version, int size) {
  assert(strcmp(version, ZLIB_VERSION) == 0);
  assert(size == static_cast<int>(sizeof(z_stream)));
  assert(s->opaque.writes == 0 && s->opaque.value == reinterpret_cast<void *>(0xC0FFEE));
  opaque_untouched = 1; ++init_calls; return init_result;
}
int deflate(z_stream *s, int flush) {
  assert(flush == Z_FINISH); ++operation_calls;
  assert(s->next_in == expected_in && s->next_out == expected_out);
  assert(s->avail_in == expected_avail_in && s->avail_out == expected_avail_out);
  s->total_out = produced; return operation_result;
}
int inflate(z_stream *s, int flush) {
  assert(flush == Z_FINISH); ++operation_calls;
  assert(s->next_in == expected_in && s->next_out == expected_out);
  assert(s->avail_in == expected_avail_in && s->avail_out == expected_avail_out);
  s->total_out = produced; return operation_result;
}
int deflateEnd(z_stream *) { ++end_calls; return end_result; }
int inflateEnd(z_stream *) { ++end_calls; return end_result; }
int sin(int angle, int scale) { assert(scale == 7); return angle; }
// This tiny host receiver exposes only the two accessed fields. It is not a
// product class/layout witness and makes no native member-pointer claim.
struct NetSessionPickerWin {
  struct Tail { int client_width_; int client_height_; } tail_;
  int scale_client_width(int); int scale_client_height(int);
};
'''+cos_body+"\n"+width_body+"\n"+height_body+"\n"+deflate_body+"\n"+inflate_body+r'''
static int signed_product_div(int a, int b, int divisor) {
  const uint32_t product = static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
      int32_t signed_product; std::memcpy(&signed_product, &product, sizeof(product));
  return signed_product / divisor;
}
int main() {
  const int angles[] = {INT_MIN, -1234567, -1, 0, 1, 7654321, INT_MAX};
  for (unsigned i = 0; i < sizeof(angles) / sizeof(angles[0]); ++i) {
    const uint32_t bits = static_cast<uint32_t>(angles[i]) + 0x3fffffffU;
        int32_t wrapped; std::memcpy(&wrapped, &bits, sizeof(wrapped));
    assert(cos(angles[i], 7) == wrapped);
  }
  NetSessionPickerWin picker = {{-2147483647, 2147483647}};
  const int factors[] = {INT_MIN, -17, -1, 0, 1, 17, INT_MAX};
  for (unsigned i = 0; i < sizeof(factors) / sizeof(factors[0]); ++i) {
    assert(picker.scale_client_width(factors[i]) ==
           signed_product_div(-2147483647, factors[i], 600));
    assert(picker.scale_client_height(factors[i]) ==
           signed_product_div(2147483647, factors[i], 440));
  }
  unsigned char dst[64] = {}, src[64] = {};
  unsigned long capacity = 19;
  expected_in = src; expected_out = dst;
  expected_avail_in = 11; expected_avail_out = 19;
  init_result = -4; end_calls = operation_calls = init_calls = opaque_untouched = 0;
  assert(netdaemon_deflate_buffer(dst, &capacity, src, 11) == -4);
  assert(end_calls == 0 && operation_calls == 0 && capacity == 19);
  init_result = 0; operation_result = 0; end_result = 8;
  end_calls = operation_calls = init_calls = opaque_untouched = 0;
  assert(netdaemon_deflate_buffer(dst, &capacity, src, 11) == Z_BUF_ERROR);
  assert(opaque_untouched && init_calls == 1 && init_level == -1);
  assert(end_calls == 1 && operation_calls == 1 && capacity == 19);
  operation_result = Z_DATA_ERROR; end_calls = operation_calls = 0;
  assert(netdaemon_deflate_buffer(dst, &capacity, src, 11) == Z_DATA_ERROR);
  assert(end_calls == 1 && capacity == 19);
  operation_result = Z_STREAM_END; produced = 23; end_result = -6;
  assert(netdaemon_deflate_buffer(dst, &capacity, src, 11) == -6 && capacity == 23);

  capacity = 19; init_result = -5; end_calls = operation_calls = 0;
  assert(netdaemon_inflate_buffer(dst, &capacity, src, 11) == -5);
  assert(end_calls == 0 && operation_calls == 0 && capacity == 19);
  init_result = 0; operation_result = Z_NEED_DICT; end_calls = operation_calls = 0;
  assert(netdaemon_inflate_buffer(dst, &capacity, src, 11) == Z_NEED_DICT);
  assert(end_calls == 1 && operation_calls == 1 && capacity == 19);
  operation_result = Z_STREAM_END; produced = 31; end_result = 5;
  assert(netdaemon_inflate_buffer(dst, &capacity, src, 11) == 5 && capacity == 31);
}
'''
    _compile_and_run(source)


def test_installed_definitions_retain_reviewed_semantic_choices():
    """Check actual field, size-write and call-path choices in installed bodies."""
    width = _function_body(
        "src/net_session_picker.cpp", "NetSessionPickerWin::scale_client_width"
    )
    assert "client_width_" in width and "client_height_" not in width
    assert "*out_size" in _function_body("src/netdaemon.cpp", "netdaemon_inflate_buffer")
    assert "stream.opaque" not in _function_body("src/netdaemon.cpp", "netdaemon_inflate_buffer")
    assert "netdaemon_inflate_buffer(mapped, &decompressed_size," in _function_body(
        "src/netdaemon.cpp", "NetDaemon::process_file_transfer_message"
    )

