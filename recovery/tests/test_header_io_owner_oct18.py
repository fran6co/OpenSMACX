"""Actual stream-framing bodies, malformed-input effects and pinned anchors.

Host doubles are bounded behavior evidence, not native FILE ABI/runtime proof.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/header_io.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function {signature}")


def _actual_bodies() -> tuple[str, str]:
    source = SOURCE.read_text(encoding="utf-8")
    return (
        _function_body(source, "void __cdecl header_check(LPSTR header, FILE *file)"),
        _function_body(source, "void __cdecl header_write(LPCSTR header, FILE *file)"),
    )


HARNESS = r"""
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

typedef char *LPSTR;
typedef const char *LPCSTR;
#define __cdecl
struct TestFile {
    std::vector<int> input;
    std::vector<int> output_args;
    std::vector<unsigned char> output_bytes;
    size_t cursor;
    int reads;
    int writes;
    TestFile() : cursor(0), reads(0), writes(0) {}
};

#define FILE TestFile
static int observed_fgetc(TestFile *stream) {
    ++stream->reads;
    if (stream->cursor == stream->input.size()) {
        return EOF;
    }
    return stream->input[stream->cursor++];
}

static int observed_fputc(int value, TestFile *stream) {
    ++stream->writes;
    stream->output_args.push_back(value);
    stream->output_bytes.push_back(static_cast<unsigned char>(value));
    return EOF; // The recovered functions ignore this result.
}

#define fgetc observed_fgetc
#define fputc observed_fputc
"""


TESTS = r"""
static int failures = 0;
static void require(bool condition, const char *label) {
    if (!condition) {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    }
}

static FILE stream_of(const std::vector<int> &bytes) {
    FILE stream;
    stream.input = bytes;
    return stream;
}

static bool prefix_is(const std::vector<unsigned char> &actual,
                      const std::vector<unsigned char> &expected) {
    if (actual.size() < expected.size()) return false;
    for (size_t i = 0; i < expected.size(); ++i) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}

static void test_header_check_text_and_embedded_nul() {
    FILE stream = stream_of(std::vector<int>{'A', 0x80, 0xFF, 0, 'Q', 'R'});
    unsigned char destination[300];
    std::memset(destination, 0xCC, sizeof(destination));
    header_check(reinterpret_cast<char *>(destination), &stream);
    require(destination[0] == 'A' && destination[1] == 0x80 &&
            destination[2] == 0xFF && destination[3] == 0,
            "header_check copies ordinary/high bytes and embedded terminator");
    require(stream.reads == 5 && stream.cursor == 5,
            "header_check consumes exactly one byte after embedded NUL");
    require(destination[4] == 0xCC, "header_check does not copy discarded byte");
}

static void test_header_check_eof_before_nul() {
    FILE stream = stream_of(std::vector<int>{'A', 'B'});
    unsigned char destination[300];
    std::memset(destination, 0xCC, sizeof(destination));
    header_check(reinterpret_cast<char *>(destination), &stream);
    bool exact = destination[0] == 'A' && destination[1] == 'B';
    for (int i = 2; i < 256; ++i) exact = exact && destination[i] == 0xFF;
    require(exact, "EOF is stored as signed-char 0xFF until the 256-byte bound");
    require(stream.reads == 257 && stream.cursor == 2,
            "EOF path performs 256 body reads plus the unconditional extra read");
    require(destination[256] == 0xCC, "EOF bound leaves following output untouched");
}

static void test_header_check_exact_bound_extra_read() {
    std::vector<int> bytes(256, 'X');
    bytes.push_back('Q');
    bytes.push_back('R');
    FILE stream = stream_of(bytes);
    unsigned char destination[300];
    std::memset(destination, 0xCC, sizeof(destination));
    header_check(reinterpret_cast<char *>(destination), &stream);
    bool exact = true;
    for (int i = 0; i < 256; ++i) exact = exact && destination[i] == 'X';
    require(exact, "header_check copies exactly 256 nonzero bytes at the bound");
    require(stream.reads == 257 && stream.cursor == 257,
            "bounded header_check still consumes exactly one extra byte");
    require(stream.input[stream.cursor] == 'R' && destination[256] == 0xCC,
            "extra read is discarded rather than copied");
}

static void test_header_write_bytes_nul_marker_and_arguments() {
    FILE stream = stream_of(std::vector<int>());
    const char header[] = {'A', static_cast<char>(0x80), static_cast<char>(0xFF), 0};
    header_write(header, &stream);
    require(stream.writes == 5, "header_write writes every byte including NUL and marker");
    require(prefix_is(stream.output_bytes,
                      std::vector<unsigned char>{'A', 0x80, 0xFF, 0, 0x1A}),
            "header_write output bytes include NUL followed by 0x1A");
    require(stream.output_args.size() == 5 && stream.output_args[0] == 'A' &&
            stream.output_args[1] == -128 && stream.output_args[2] == -1 &&
            stream.output_args[3] == 0 && stream.output_args[4] == 0x1A,
            "fputc receives signed-char promoted values, then marker");
}

static void test_header_write_empty_string() {
    FILE stream = stream_of(std::vector<int>());
    header_write("", &stream);
    require(stream.writes == 2 && stream.output_args.size() == 2 &&
            stream.output_args[0] == 0 && stream.output_args[1] == 0x1A,
            "empty header still writes NUL and 0x1A");
}

int main() {
    test_header_check_text_and_embedded_nul();
    test_header_check_eof_before_nul();
    test_header_check_exact_bound_extra_read();
    test_header_write_bytes_nul_marker_and_arguments();
    test_header_write_empty_string();
    return failures ? 1 : 0;
}
"""


def _compile_run(tmp_path: Path, compiler: str, check: str, write: str, tag: str):
    fixture = tmp_path / f"header_io_{tag}.cpp"
    executable = tmp_path / f"header_io_{tag}"
    fixture.write_text(HARNESS + "\n" + check + "\n" + write + "\n" + TESTS,
                       encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-fsigned-char", "-Wall", "-Wextra", "-Werror",
         str(fixture), "-o", str(executable)],
        text=True, capture_output=True,
    )
    assert built.returncode == 0, f"{tag} must compile before behavior is judged:\n{built.stderr}"
    return subprocess.run([str(executable)], text=True, capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_header_functions_pass_host_behavior_matrix(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    check, write = _actual_bodies()
    result = _compile_run(tmp_path, compiler, check, write, "actual")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [
    "drop_unconditional_read",
    "read_past_256_bound",
    "omit_terminating_nul_write",
    "wrong_header_marker",
    "unsigned_promoted_character",
])
def test_compiling_behavior_mutants_are_rejected(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    check, write = _actual_bodies()
    changes = {
        "drop_unconditional_read": (check, "    fgetc(file);\n}", "}"),
        "read_past_256_bound": (check, "if (++i >= 256)", "if (++i > 256)"),
        "omit_terminating_nul_write": (write, "        fputc(header_chr, file);",
                                        "        if (header_chr) fputc(header_chr, file);"),
        "wrong_header_marker": (write, "fputc(0x1A, file);", "fputc(0x1B, file);"),
        "unsigned_promoted_character": (write, "fputc(header_chr, file);",
                                         "fputc(static_cast<unsigned char>(header_chr), file);"),
    }
    original, before, after = changes[damage]
    assert before in original, f"mutation anchor missing for {damage}"
    damaged = original.replace(before, after, 1)
    if damage in ("drop_unconditional_read", "read_past_256_bound"):
        check = damaged
    else:
        write = damaged
    result = _compile_run(tmp_path, compiler, check, write, damage)
    assert result.returncode != 0, f"behavior control missed {damage}"
    assert "FAIL:" in result.stderr, (damage, result.stdout, result.stderr)


def test_pinned_header_instruction_and_call_evidence():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def decode(start: int, end: int):
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        cursor = start
        by_address = {}
        for instruction in instructions:
            assert instruction.address == cursor
            cursor += instruction.size
            by_address[instruction.address] = (instruction.mnemonic, instruction.op_str)
        assert cursor == end
        return by_address

    check = decode(0x57D1F0, 0x57D235)
    assert check[0x57D1FC] == ("call", "0x646855")
    assert check[0x57D20A] == ("test", "al, al")
    assert check[0x57D20F] == ("cmp", "ebx, 0x100")
    assert check[0x57D215] == ("jge", "0x57d227")
    assert check[0x57D218] == ("call", "0x646855")
    assert check[0x57D228] == ("call", "0x646855")
    assert check[0x57D234] == ("ret", "")

    write = decode(0x57D240, 0x57D270)
    assert write[0x57D24C] == ("mov", "bl, byte ptr [esi]")
    assert write[0x57D24F] == ("movsx", "eax, bl")
    assert write[0x57D254] == ("call", "0x646886")
    assert write[0x57D25C] == ("test", "bl, bl")
    assert write[0x57D25E] == ("jne", "0x57d24c")
    assert write[0x57D261] == ("push", "0x1a")
    assert write[0x57D263] == ("call", "0x646886")
    assert write[0x57D26F] == ("ret", "")

    # Direct callers bind these recovered routines into the map/daemon streams.
    assert decode(0x5A9C7F, 0x5A9C84)[0x5A9C7F] == ("call", "0x57d1f0")
    assert decode(0x5A97A2, 0x5A97A7)[0x5A97A2] == ("call", "0x57d1f0")
    assert decode(0x5A9653, 0x5A9658)[0x5A9653] == ("call", "0x57d240")
    assert decode(0x5A9BA8, 0x5A9BAD)[0x5A9BA8] == ("call", "0x57d240")
