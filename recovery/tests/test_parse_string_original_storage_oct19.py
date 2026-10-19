"""Source and pinned-image controls for parse_string's original table/calls."""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/general.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE_DEFAULT = ROOT / ".opensmacx/game/terranx_original.exe"
PART_SPEECH = (" de ", " le ", " la ", "De ", "Le ", "La ")
PART_SPEECH_BYTES = b"".join(
    value.encode("ascii") + bytes(20 - len(value.encode("ascii")))
    for value in PART_SPEECH
)


def parser_body(source: str) -> str:
    match = re.search(r"int\s+__cdecl\s+parse_string\s*\([^)]*\)\s*\{", source)
    assert match, "missing parse_string definition"
    opening = source.index("{", match.start())
    depth = 0
    quoted = None
    escaped = False
    line_comment = False
    block_comment = False
    for index in range(opening, len(source)):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and following == "/":
                block_comment = False
        elif quoted is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quoted:
                quoted = None
        elif char == "/" and following == "/":
            line_comment = True
        elif char == "/" and following == "*":
            block_comment = True
        elif char in ("'", '"'):
            quoted = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
    raise AssertionError("unterminated parse_string body")


def table_declaration(source: str) -> str:
    match = re.search(
        r"static\s+const\s+char\s+part_speech\s*\[\s*6\s*\]\s*"
        r"\[\s*20\s*\]\s*=\s*\{[^}]*\};",
        parser_body(source),
        re.S,
    )
    assert match, "French patterns must be six inline 20-byte rows"
    declaration = match.group(0)
    strings = re.findall(r'"([^"\\]*)"', declaration)
    assert tuple(strings) == PART_SPEECH
    return declaration


def test_source_keeps_original_direct_calls_and_decimal_hex_behavior():
    body = parser_body(SOURCE.read_text(encoding="utf-8"))
    assert body.count("_itoa(ParseNumTable[number], output_num, 10);") == 3
    assert body.count("strcat(output, output_num);") == 3
    assert body.count("strcat(output, ParseStrBuffer[number].str);") == 4
    assert "_itoa_s(" not in body and "strcat_s(" not in body
    assert table_declaration(SOURCE.read_text(encoding="utf-8"))

    hex_branch = body.split('if (!strncmp(var, "$HEX", 4))', 1)[1].split("} else {", 1)[0]
    assert "_itoa(ParseNumTable[number], output_num, 10);" in hex_branch


def test_actual_table_declaration_has_six_complete_twenty_byte_rows():
    declaration = table_declaration(SOURCE.read_text(encoding="utf-8"))
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is installed for host table controls"
    harness = f"""
#include <cstring>
{declaration}
static_assert(sizeof(part_speech) == 120, "six full rows");
static_assert(sizeof(part_speech[0]) == 20, "row stride");
int main() {{
  const char expected[6][20] = {{
    " de ", " le ", " la ", "De ", "Le ", "La "
  }};
  return std::memcmp(part_speech, expected, sizeof(expected)) != 0;
}}
"""
    with tempfile.TemporaryDirectory(prefix="osmx-parse-string-table-") as temp:
        path = Path(temp) / "table.cpp"
        binary = Path(temp) / "table"
        path.write_text(harness, encoding="utf-8")
        for compiler in compilers:
            built = subprocess.run(
                [compiler, "-std=c++17", "-O0", str(path), "-o", str(binary)],
                capture_output=True, text=True, timeout=45,
            )
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            assert ran.returncode == 0, ran.stdout + ran.stderr


def test_pinned_table_bytes_and_loop_witnesses():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", IMAGE_DEFAULT))
    if not image_path.is_file():
        pytest.skip("pinned original executable unavailable")
    image = PeImage(image_path.read_bytes(), PIN)
    table_span = Span(0x006972D4, 0x0069734C)
    assert image.read(table_span) == PART_SPEECH_BYTES
    assert len(PART_SPEECH_BYTES) == 6 * 20
    assert not image.absolute_sites(table_span)

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    function_span = Span(0x00625880, 0x00625DDD)
    decoded = list(decoder.disasm(image.read(function_span), function_span.start))
    assert len(decoded) == 490
    cursor = function_span.start
    for ins in decoded:
        assert ins.address == cursor, "noncontiguous original parser decode"
        cursor += ins.size
    assert cursor == function_span.end, "original parser decode is incomplete"
    instructions = {ins.address: ins for ins in decoded}

    def fact(address: int) -> tuple[str, str]:
        ins = instructions[address]
        return ins.mnemonic, ins.op_str

    # The three conversions have distinct original stack destinations, all
    # receive radix 10, and call the pinned ordinary _itoa entry.
    itoa_witnesses = (
        (0x00625924, 0x78, 0x00625928, 0x00625939),
        (0x00625996, 0x28, 0x0062599A, 0x006259AB),
        (0x00625A32, 0xC8, 0x00625A39, 0x00625A4A),
    )
    destinations = []
    for lea_at, offset, push_at, call_at in itoa_witnesses:
        mnemonic, operands = fact(lea_at)
        assert mnemonic == "lea" and operands.endswith(f"[esp + 0x{offset:x}]")
        destinations.append(offset)
        assert fact(push_at) == ("push", "0xa")
        assert fact(call_at) == ("call", "0x64fc88")
    assert len(set(destinations)) == 3

    strcat_calls = (0x00625947, 0x00625CE6, 0x00625D0D)
    assert all(fact(address) == ("call", "0x645470") for address in strcat_calls)

    # Pinned table traversal: six calls to strstr, 0x14-byte row steps, and
    # the exclusive upper bound one row beyond the sixth initializer.
    assert fact(0x00625D2D) == ("mov", "esi, 0x6972d4")
    assert fact(0x00625D34) == ("call", "0x6468d0")
    assert fact(0x00625D57) == ("add", "esi, 0x14")
    assert fact(0x00625D5B) == ("cmp", "esi, 0x69734c")
    assert fact(0x00625D61) == ("jl", "0x625d32")
