"""Pinned and actual-source checks for the fixed-point cosine wrapper.

The host harness checks the observed 32-bit phase adjustment and exact
forwarding to a typed sin stub. It does not prove the game's compiler/ABI or
runtime implementation of sin.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())
SOURCE = ROOT / "src/general.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
COS_SIGNATURE = "int __cdecl cos(int a1, int a2)"
COS_ORIGINAL = (
    "// ORIGINAL: 0x0063BAC0 ?cos@@YAHHH@Z "
    "0x0063BAC0-0x0063BAD9 BYTE_EXACT"
)
SIN_ORIGINAL = (
    "// ORIGINAL: 0x0063B9B0 ?sin@@YAHHH@Z "
    "0x0063B9B0-0x0063BABB BYTE_EXACT"
)
TOKEN = re.compile(
    r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]',
    re.S,
)


def definition(text: str, signature: str) -> str:
    hits = [match for match in re.finditer(re.escape(signature), text)]
    assert len(hits) == 1, (signature, len(hits))
    start = hits[0].start()
    opened = False
    depth = 0
    for token in TOKEN.finditer(text, hits[0].end()):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated actual body: " + signature)


def replace_call(body: str, replacement) -> str:
    marker = "return sin("
    start = body.index(marker)
    args_start = start + len(marker)
    depth = 0
    comma = None
    end = None
    for index in range(args_start, len(body)):
        char = body[index]
        if char == "(":
            depth += 1
        elif char == ")":
            if depth == 0:
                end = index
                break
            depth -= 1
        elif char == "," and depth == 0 and comma is None:
            comma = index
    assert comma is not None and end is not None
    first = body[args_start:comma].strip()
    second = body[comma + 1:end].strip()
    statement_end = end + 1
    if statement_end < len(body) and body[statement_end] == ";":
        statement_end += 1
    return body[:start] + replacement(first, second) + body[statement_end:]


def actual_body(source_text: str, mutant: str | None = None) -> str:
    body = definition(source_text, COS_SIGNATURE)
    if mutant in ("wrong_phase", "omitted_offset"):
        old = "0x3fffffffU"
        assert body.count(old) == 1, "phase expression changed; review test mutation"
        new = "0x40000000U" if mutant == "wrong_phase" else "0U"
        body = body.replace(old, new, 1)
    elif mutant == "swapped_args":
        body = replace_call(body, lambda first, second:
                            f"return sin({second}, {first});")
    elif mutant == "lost_return":
        body = replace_call(body, lambda first, second:
                            f"sin({first}, {second}); return 0;")
    elif mutant is not None:
        raise AssertionError("unknown mutation: " + mutant)
    return body


HOST = r"""
#include <cassert>
#include <cstdint>
#include <cstring>

#define __cdecl
using uint32_t = std::uint32_t;
using int32_t = std::int32_t;

static int call_count;
static int observed_angle;
static int observed_scale;
static int sin_result;
static int sin(int angle, int scale) {
    ++call_count;
    observed_angle = angle;
    observed_scale = scale;
    return sin_result;
}

@@COS_BODY@@

int main() {
    static_assert(sizeof(int) == 4, "the original wrapper uses 32-bit int");
    const int angles[] = {
        0, 1, -1, 0x3fffffff, 0x40000000, 0x7fffffff,
        (-2147483647 - 1), (-2147483647), -0x40000000,
    };
    const int scales[] = {
        0, 1, -1, 0x3fffffff, 0x40000000, 0x7fffffff,
        (-2147483647 - 1), 0x12345678, -0x1234567,
    };
    const int returns[] = {
        (-2147483647 - 1), 0x7fffffff, -1, 0, 1,
        -0x1234567, 0x31415926, -0x31415926,
    };
    int case_index = 0;
    for (int angle : angles) {
      for (int scale : scales) {
        const uint32_t wrapped = static_cast<uint32_t>(angle) + 0x3fffffffU;
        int expected_angle;
        std::memcpy(&expected_angle, &wrapped, sizeof(expected_angle));

        call_count = 0;
        observed_angle = 0x11223344;
        observed_scale = 0x55667788;
        sin_result = returns[case_index % 8];
        const int result = cos(angle, scale);
        assert(call_count == 1);
        assert(observed_angle == expected_angle);
        assert(observed_scale == scale);
        assert(result == sin_result);
        ++case_index;
      }
    }

    // Deterministically exercise the full signed 32-bit input domain without
    // relying on implementation-defined unsigned-to-signed conversions.
    uint32_t state = 0x6d2b79f5u;
    for (int index = 0; index != 4096; ++index) {
        state = state * 1664525u + 1013904223u;
        int angle;
        std::memcpy(&angle, &state, sizeof(angle));
        state = state * 1664525u + 1013904223u;
        int scale;
        std::memcpy(&scale, &state, sizeof(scale));
        state = state * 1664525u + 1013904223u;
        int returned;
        std::memcpy(&returned, &state, sizeof(returned));

        const uint32_t wrapped = static_cast<uint32_t>(angle) + 0x3fffffffU;
        int expected_angle;
        std::memcpy(&expected_angle, &wrapped, sizeof(expected_angle));
        call_count = 0;
        observed_angle = 0x11223344;
        observed_scale = 0x55667788;
        sin_result = returned;
        const int result = cos(angle, scale);
        assert(call_count == 1);
        assert(observed_angle == expected_angle);
        assert(observed_scale == scale);
        assert(result == returned);
    }
    return 0;
}
"""


def pinned_image() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        return Path(configured)
    local = ROOT / ".opensmacx/game/terranx_original.exe"
    if local.is_file():
        return local
    if len(ROOT.parents) > 2:
        coordinator = ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
        if coordinator.is_file():
            return coordinator
    return local


def test_pinned_cos_complete_span_and_call_target() -> None:
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    image = pinned_image()
    if not image.is_file():
        pytest.skip("pinned image unavailable; host controls do not replace original anchors")
    pe = PeImage(image.read_bytes(), PIN)
    span = Span(0x0063BAC0, 0x0063BAD9)
    code = pe.read(span)
    assert len(code) == 25
    rows = Listing(code, span.start).lines
    assert len(rows) == 8
    instructions = [re.split(r"\s{2,}", row.strip())[-1] for row in rows]
    assert instructions == [
        "mov ecx, dword ptr [esp + 4]",
        "mov eax, dword ptr [esp + 8]",
        "add ecx, 0x3fffffff",
        "push eax",
        "push ecx",
        "call 0x63b9b0",
        "add esp, 8",
        "ret",
    ]

    source = SOURCE.read_text(encoding="utf-8")
    assert COS_ORIGINAL in source
    assert SIN_ORIGINAL in source


def compile_and_run(compiler: str, source_text: str, mutant: str | None,
                    optimization: str) -> bool:
    generated = HOST.replace("@@COS_BODY@@", actual_body(source_text, mutant))
    with tempfile.TemporaryDirectory(prefix="osmx-general-cos-host-") as directory:
        cpp = Path(directory) / "general_cos.cpp"
        executable = Path(directory) / "general_cos"
        cpp.write_text(generated, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++17", optimization, "-Wall", "-Wextra", "-Werror",
             "-fsanitize=undefined", "-fno-sanitize-recover=all", str(cpp), "-o",
             str(executable)],
            capture_output=True, text=True, timeout=60,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
        return result.returncode == 0


def test_actual_cos_body_is_extracted_once() -> None:
    source_text = SOURCE.read_text(encoding="utf-8")
    body = actual_body(source_text)
    assert body.startswith(COS_SIGNATURE)
    assert body.count("return sin(") == 1


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("optimization", ("-O0", "-O2"))
def test_actual_cos_wrap_and_exact_sin_forwarding(compiler_name: str,
                                                   optimization: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    source_text = SOURCE.read_text(encoding="utf-8")
    assert compile_and_run(compiler, source_text, None, optimization)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("optimization", ("-O0", "-O2"))
@pytest.mark.parametrize("mutant", (
    "wrong_phase", "omitted_offset", "swapped_args", "lost_return",
))
def test_compiling_semantic_damage_witnesses(compiler_name: str,
                                              optimization: str,
                                              mutant: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    source_text = SOURCE.read_text(encoding="utf-8")
    assert not compile_and_run(compiler, source_text, mutant, optimization), (
        compiler_name, optimization, mutant,
    )
