"""Actual-source controls for the distinct NetDaemon sync header prefix.

These host controls exercise only the explicitly observed 0x24-byte prefix.
They do not establish the enclosing allocation's type, length, lifetime or
initialization of the bytes left unwritten by native field stores.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve()
_root = os.environ.get("OPENSMACX_ROOT")
ROOT = Path(_root).resolve() if _root else HERE.parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"


def _mask(text):
    out = list(text)
    mode = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if mode == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "block"
                continue
            if c == '"':
                out[i] = " "
                mode = "string"
            elif c == "'":
                out[i] = " "
                mode = "char"
        elif mode == "line":
            if c == "\n":
                mode = "code"
            else:
                out[i] = " "
        elif mode == "block":
            out[i] = " "
            if c == "*" and n == "/":
                out[i + 1] = " "
                i += 1
                mode = "code"
        else:
            out[i] = " "
            if c == "\\" and i + 1 < len(text):
                out[i + 1] = " "
                i += 1
            elif (mode == "string" and c == '"') or (mode == "char" and c == "'"):
                mode = "code"
        i += 1
    return "".join(out)


def _declaration(text, kind, name):
    code = _mask(text)
    matches = list(re.finditer(r"\b" + kind + r"\s+" + re.escape(name) + r"\s*\{", code))
    assert len(matches) == 1, "expected one actual " + name + " declaration"
    start = matches[0].start()
    opening = code.find("{", start)
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            cursor = end + 1
            while cursor < len(code) and code[cursor].isspace():
                cursor += 1
            if cursor < len(code) and code[cursor] == ";":
                return text[start:cursor + 1]
            return text[start:end + 1]
    raise AssertionError("unterminated " + name)


def _actual_prefix_source():
    source = SOURCE.read_text()
    assert "struct SyncHeaderPrefix" in source
    parts = [
        _declaration(source, "struct", "SyncHeaderPrefix"),
        _declaration(source, "class", "SyncHeaderPrefixReader"),
        _declaration(source, "class", "SyncHeaderPrefixWriter"),
    ]
    # The explicit offsets and prefix extent are part of the source contract.
    # Keeping these checks tied to actual declarations prevents a test-only
    # schema from silently standing in for production.
    for token in ("0x24", "0x00", "0x04", "0x0C", "0x10", "0x12",
                  "0x14", "0x18", "0x1C", "0x20"):
        assert token in source, "missing sync-prefix layout evidence " + token
    for hole in ("unresolved_2", "unresolved_8", "unresolved_20"):
        assert hole in parts[0], "missing explicitly unresolved interval " + hole
    for accessor in ("flags", "faction", "sequence", "opcode", "index",
                     "argument_2", "argument_3", "argument_4"):
        assert re.search(r"\b" + accessor + r"\s*\(", "\n".join(parts)), accessor
    for accessor in ("store_flags", "store_faction", "store_sequence", "store_opcode",
                     "store_index", "store_argument_2", "store_argument_3", "store_argument_4"):
        assert re.search(r"\b" + accessor + r"\s*\(", "\n".join(parts)), accessor
    return "\n".join(parts)


def test_product_sender_and_receiver_use_the_actual_prefix_views():
    from test_netdaemon_synch_dispatch import _definition

    source = SOURCE.read_text()
    sender = _definition(source, r"static\s+void\s+netdaemon_sync_send_chunk",
                         "sync chunk sender")
    request = _definition(source, r"static\s+void\s+netdaemon_process_sync_request",
                          "sync request receiver")
    assert "SyncHeaderPrefixWriter prefix(bytes);" in sender
    for field in ("flags", "faction", "sequence", "opcode", "index",
                  "argument_2", "argument_3", "argument_4"):
        assert "prefix.store_" + field in sender
    assert "const SyncHeaderPrefixReader request(message);" in request
    assert request.count("request.faction()") == 2
    assert "request.opcode()" in request and "request.argument_2()" in request
    assert "request.argument_3()" in request and "request.argument_4()" in request
    assert "*reinterpret_cast<int16_t *>(message + 0x10)" not in request
    assert "*reinterpret_cast<uint32_t *>(message + 4)" not in request
    assert "memset" not in sender and "sizeof(SyncHeaderPrefix)" not in sender


def _compiler_names():
    requested = os.environ.get("NETDAEMON_HOST_CXX")
    names = [requested] if requested else ["g++", "clang++"]
    return [name for name in names if name and shutil.which(name)]


HARNESS = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
/* ACTUAL SOURCE PREFIX DECLARATIONS */

static int fail(const char *label) {
    std::fprintf(stderr, "sync-prefix assertion: %s\n", label);
    return 98;
}

int main() {
    if (sizeof(SyncHeaderPrefix) != 0x24) return fail("prefix extent");
    unsigned char storage[1 + 0x24 + 8];
    std::memset(storage, 0xA5, sizeof storage);
    unsigned char *unaligned = storage + 1;
    SyncHeaderPrefixWriter writer(unaligned);
    writer.store_flags(static_cast<int16_t>(0x9123));
    writer.store_faction(0x78563412u);
    writer.store_sequence(0xA1B2C3D4u);
    writer.store_opcode(static_cast<int16_t>(-0x1234));
    writer.store_index(static_cast<int16_t>(-2));
    writer.store_argument_2(static_cast<int32_t>(-0x1234567));
    writer.store_argument_3(static_cast<int32_t>(0x12345678));
    writer.store_argument_4(static_cast<int32_t>(-1));

    const int written[][2] = {{0, 2}, {4, 4}, {12, 4}, {16, 2}, {18, 2},
                              {20, 4}, {24, 4}, {28, 4}};
    for (unsigned i = 0; i < sizeof written / sizeof written[0]; ++i) {
        for (int j = 0; j < written[i][1]; ++j) {
            if (unaligned[written[i][0] + j] == 0xA5) return fail("expected field store");
        }
    }
    for (int i = 2; i < 4; ++i) if (unaligned[i] != 0xA5) return fail("hole +2..+3 changed");
    for (int i = 8; i < 12; ++i) if (unaligned[i] != 0xA5) return fail("hole +8..+B changed");
    for (int i = 32; i < 36; ++i) if (unaligned[i] != 0xA5) return fail("hole +20..+23 changed");
    for (int i = 36; i < 0x24 + 8; ++i) if (unaligned[i] != 0xA5) return fail("payload boundary changed");

    SyncHeaderPrefixReader reader(unaligned);
    if (reader.flags() != static_cast<int16_t>(0x9123)) return fail("signed flags");
    if (reader.faction() != 0x78563412u) return fail("faction");
    if (reader.sequence() != 0xA1B2C3D4u) return fail("sequence");
    if (reader.opcode() != static_cast<int16_t>(-0x1234)) return fail("signed opcode");
    if (reader.index() != static_cast<int16_t>(-2)) return fail("signed index");
    if (reader.argument_2() != static_cast<int32_t>(-0x1234567)) return fail("argument 2");
    if (reader.argument_3() != static_cast<int32_t>(0x12345678)) return fail("argument 3");
    if (reader.argument_4() != static_cast<int32_t>(-1)) return fail("argument 4");
    return 0;
}
'''


def _run(tmp_path, compiler, declarations, mutation=None):
    harness = HARNESS.replace("/* ACTUAL SOURCE PREFIX DECLARATIONS */", declarations)
    if mutation:
        before, after = mutation
        assert before in harness
        harness = harness.replace(before, after, 1)
    cpp = tmp_path / "sync-prefix.cpp"
    exe = tmp_path / "sync-prefix"
    cpp.write_text(harness)
    build = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-pedantic", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True, timeout=30)
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=5)
    return run


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_sync_prefix_writer_reader_and_holes(tmp_path, compiler):
    assert SOURCE.is_file(), "set OPENSMACX_ROOT to a checkout containing src/netdaemon.cpp"
    declarations = _actual_prefix_source()
    run = _run(tmp_path, compiler, declarations)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler", _compiler_names())
@pytest.mark.parametrize("mutation", [
    ("int16_t opcode() const {\n        int16_t value;\n"
     "        std::memcpy(&value, bytes_ + offsetof(SyncHeaderPrefix, opcode), sizeof value);",
     "int16_t opcode() const {\n        int16_t value;\n"
     "        std::memcpy(&value, bytes_ + offsetof(SyncHeaderPrefix, index), sizeof value);"),
    ("int16_t index() const {\n        int16_t value;\n"
     "        std::memcpy(&value, bytes_ + offsetof(SyncHeaderPrefix, index), sizeof value);",
     "int16_t index() const {\n        int16_t value;\n"
     "        std::memcpy(&value, bytes_ + offsetof(SyncHeaderPrefix, opcode), sizeof value);"),
    ("void store_argument_4(int32_t value) { write(offsetof(SyncHeaderPrefix, argument_4), value); }",
     "void store_argument_4(int32_t value) { write(offsetof(SyncHeaderPrefix, argument_3), value); }"),
    ("std::memcpy(bytes_ + offset, &value, sizeof value);",
     "std::memcpy(bytes_ + offset + 1, &value, sizeof value);"),
])
def test_sync_prefix_reader_writer_mutants_fail_after_compiling(tmp_path, compiler, mutation):
    run = _run(tmp_path, compiler, _actual_prefix_source(), mutation)
    assert run.returncode == 98, run.stdout + run.stderr
    assert "sync-prefix assertion:" in run.stderr
