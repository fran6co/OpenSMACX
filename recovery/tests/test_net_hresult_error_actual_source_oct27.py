"""Actual-source and pinned controls for the DirectPlay error interpreter."""
from pathlib import Path
import shutil
import subprocess
import os

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/net_class.cpp"
HOST_FIXTURE = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __stdcall
#define __stdcall
#endif
typedef int32_t HRESULT;

static const char *observed_message;
static int parse_count;
static int popup_count;
static int cd_count;
static int popup_result;
static int cd_result;

int parse_says(int slot, char *text, int arg2, int arg3) {
    assert(slot == 0 && arg2 == -1 && arg3 == -1);
    assert(popup_count == 0 && cd_count == 0);
    ++parse_count;
    observed_message = text;
    return 0;
}
int pop_caption_title(char *title, char *caption, int flags, int reserved) {
    assert(strcmp(title, "jackal") == 0);
    assert(strcmp(caption, "DPLAYERROR") == 0);
    assert(flags == 0x10040 && reserved == 0);
    assert(cd_count == 0);
    ++popup_count;
    return popup_result;
}
int cd_check() {
    assert(popup_count == 1);
    ++cd_count;
    return cd_result;
}

// ACTUAL_NET_ERROR_BODY

int main(int argc, char **argv) {
    assert(argc == 4);
    const uint32_t bits = static_cast<uint32_t>(strtoul(argv[1], 0, 0));
    popup_result = atoi(argv[2]);
    cd_result = atoi(argv[3]);
    const int result = sub_631ba0(static_cast<HRESULT>(bits));
    printf("%d|%s|%d|%d|%d\n", parse_count,
           observed_message ? observed_message : "-", popup_count, cd_count,
           result);
    return 0;
}
'''


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth, state, index = 0, "code", opening
    while index < len(text):
        c = text[index]
        n = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "/" and n == "/": state = "line"; index += 1
            elif c == "/" and n == "*": state = "block"; index += 1
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        elif state in ("string", "char"):
            if c == "\\": index += 1
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif state == "line" and c == "\n": state = "code"
        elif state == "block" and c == "*" and n == "/": state = "code"; index += 1
        index += 1
    raise AssertionError("unclosed production definition")


def compile_body(tmp_path: Path, body: str, compiler_name: str) -> Path:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    fixture = HOST_FIXTURE
    assert fixture.count("// ACTUAL_NET_ERROR_BODY") == 1
    cpp = tmp_path / "actual_net_error.cpp"
    binary = tmp_path / "actual_net_error"
    cpp.write_text(fixture.replace("// ACTUAL_NET_ERROR_BODY", body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return binary


def run(binary: Path, error: str, popup: int, cd: int) -> str:
    return subprocess.run([str(binary), error, str(popup), str(cd)], check=True,
                          capture_output=True, text=True, timeout=5).stdout.strip()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_helper_vectors_and_popup_paths(tmp_path: Path, compiler_name: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = definition(source, "static int __stdcall sub_631ba0(HRESULT error)")
    binary = compile_body(tmp_path, body, compiler_name)
    assert run(binary, "0x88770136", -1, 73) == "1|DPERR_SESSIONLOST|1|0|0"
    assert run(binary, "0x88770132", -1, 73) == "0|-|1|0|0"
    assert run(binary, "0x887700AA", 3, 41) == "1|DPERR_NOCONNECTION|1|1|41"
    assert run(binary, "0x80004002", 3, -9) == "1|DPERR_NOINTERFACE|1|1|-9"
    assert run(binary, "0xDEADBEEF", -1, 73) == "0|-|1|0|0"
    assert run(binary, "0xDEADBEEF", 3, 73) == "0|-|1|1|73"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_value_mutant_fails_expected_session_lost_output(tmp_path: Path, compiler_name: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = definition(source, "static int __stdcall sub_631ba0(HRESULT error)")
    expected_case = "case 0x88770136UL: message = \"DPERR_SESSIONLOST\"; break;"
    assert body.count(expected_case) == 1
    wrong_value = body.replace(expected_case,
        "case 0x88770132UL: message = \"DPERR_SESSIONLOST\"; break;")
    binary = compile_body(tmp_path, wrong_value, compiler_name)
    expected_correct_output = "1|DPERR_SESSIONLOST|1|0|0"
    mutant_output = run(binary, "0x88770136", -1, 73)
    assert mutant_output != expected_correct_output
    assert mutant_output == "0|-|1|0|0"
    assert run(binary, "0x88770132", -1, 73) == "1|DPERR_SESSIONLOST|1|0|0"


def test_pinned_session_lost_compare_and_token() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned executable unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    rows = {i.address: (i.mnemonic, i.op_str)
            for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
                image.read(Span(0x631E1A, 0x631E33)), 0x631E1A)}
    assert rows[0x631E1A] == ("cmp", "eax, 0x88770136")
    assert rows[0x631E1F] == ("jne", "0x631eb0")
    assert rows[0x631E29] == ("push", "0x697dac")
    assert image.read(Span(0x697DAC, 0x697DBE)) == b"DPERR_SESSIONLOST\0"
