"""Actual-source and pinned controls for the DirectPlay error interpreter."""
from pathlib import Path
import re
import shutil
import subprocess
import os

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/net_class.cpp"
from recovery.environment.wine import VC6_ROOT


def _logical_defines(text: str) -> dict[str, list[str]]:
    """Read only simple #define records, joining continued macro bodies."""
    logical_lines: list[str] = []
    current = ""
    for line in text.splitlines():
        current += line.rstrip().removesuffix("\\") + " "
        if line.rstrip().endswith("\\"):
            continue
        logical_lines.append(current.strip())
        current = ""
    if current.strip():
        logical_lines.append(current.strip())

    defines: dict[str, list[str]] = {}
    for line in logical_lines:
        match = re.match(r"^#\s*define\s+([A-Za-z_]\w*)(?:\s*\(([^)]*)\))?\s*(.*?)\s*$", line)
        if match:
            name, parameters, expression = match.groups()
            if parameters is not None:
                name += "(" + re.sub(r"\s+", "", parameters) + ")"
            defines.setdefault(name, []).append(re.sub(r"\s+", " ", expression).strip())
    return defines


def _win32_error_defines(winerror: str) -> dict[str, list[str]]:
    start = re.search(r"(?m)^#\s*if\s+defined\(_WIN32\)\s*&&\s*!defined\(_MAC\)\s*$", winerror)
    assert start, "VC6 WINERROR.H lacks the expected Win32 error branch"
    end = re.search(r"(?m)^#\s*else\s*$", winerror[start.end():])
    assert end, "VC6 WINERROR.H Win32 error branch has no matching alternate branch"
    return _logical_defines(winerror[start.end():start.end() + end.start()])


def sdk_hresult_environment(body: str, expected_cases: int = 39) -> tuple[str, dict[str, int], int]:
    """Generate named signed32 constants from this configured VC6 SDK only."""
    include = VC6_ROOT / "INCLUDE"
    dplay_path = include / "DPLAY.H"
    if not dplay_path.exists():
        dplay_path = include / "dplay.h"
    winerror_path = include / "WINERROR.H"
    if not winerror_path.exists():
        winerror_path = include / "winerror.h"
    assert dplay_path.is_file() and winerror_path.is_file(), (
        f"configured VC6 SDK headers missing below {include}"
    )
    dplay_text = dplay_path.read_text(encoding="latin-1")
    winerror_text = winerror_path.read_text(encoding="latin-1")
    dplay = _logical_defines(dplay_text)
    winerror_all = _logical_defines(winerror_text)
    win32_errors = _win32_error_defines(winerror_text)

    fac_values = dplay.get("_FACDP", [])
    assert len(fac_values) == 1, "expected one SDK _FACDP definition"
    fac_match = re.fullmatch(r"(0[xX][0-9A-Fa-f]+|[0-9]+)", fac_values[0])
    assert fac_match, f"unsupported SDK _FACDP expression: {fac_values[0]!r}"
    fac_dp = int(fac_match.group(1), 0)

    make_dp_values = dplay.get("MAKE_DPHRESULT(code)", [])
    assert len(make_dp_values) == 1 and re.sub(r"\s+", "", make_dp_values[0]) == "MAKE_HRESULT(1,_FACDP,code)", (
        f"unsupported SDK MAKE_DPHRESULT definition: {make_dp_values!r}"
    )
    make_hresult_values = winerror_all.get("MAKE_HRESULT(sev,fac,code)", [])
    make_hresult_normalized = [re.sub(r"\s+", "", value) for value in make_hresult_values]
    expected_make_hresult = "((HRESULT)(((unsignedlong)(sev)<<31)|((unsignedlong)(fac)<<16)|((unsignedlong)(code))))"
    assert expected_make_hresult in make_hresult_normalized, (
        f"unsupported SDK MAKE_HRESULT definition: {make_hresult_values!r}"
    )
    hresult_typedefs = winerror_all.get("_HRESULT_TYPEDEF_(_sc)", [])
    assert any(re.sub(r"\s+", "", value) == "((HRESULT)_sc)" for value in hresult_typedefs), (
        f"unsupported SDK _HRESULT_TYPEDEF_ definition: {hresult_typedefs!r}"
    )

    def signed32(value: int) -> int:
        value &= 0xFFFFFFFF
        return value if value < 0x80000000 else value - 0x100000000

    def resolve_sdk_error(name: str) -> int:
        values = dplay.get(name, [])
        assert len(values) == 1, f"missing/ambiguous SDK DirectPlay constant {name}: {values!r}"
        expression = values[0]
        make_match = re.fullmatch(r"MAKE_DPHRESULT\(\s*([0-9]+)\s*\)", expression)
        if make_match:
            code = int(make_match.group(1), 10)
            assert 0 <= code <= 0xFFFF, f"out-of-range SDK DirectPlay code in {name}"
            return signed32(0x80000000 | (fac_dp << 16) | code)
        alias = re.fullmatch(r"(E_[A-Za-z0-9_]+)", expression)
        assert alias, f"unsupported SDK DirectPlay HRESULT expression for {name}: {expression!r}"
        error_name = alias.group(1)
        error_values = win32_errors.get(error_name, [])
        assert len(error_values) == 1, (
            f"missing/ambiguous Win32-branch SDK HRESULT {error_name}: {error_values!r}"
        )
        typedef = re.fullmatch(
            r"_HRESULT_TYPEDEF_\(\s*(0[xX][0-9A-Fa-f]+)(?:[uUlL]+)?\s*\)",
            error_values[0],
        )
        assert typedef, f"unsupported Win32 SDK HRESULT expression for {error_name}: {error_values[0]!r}"
        return signed32(int(typedef.group(1), 16))

    names = re.findall(r"(?m)^\s*case\s+([A-Za-z_]\w*)\s*:", body)
    assert len(names) == expected_cases and len(set(names)) == expected_cases, (
        f"expected {expected_cases} distinct named HRESULT cases in extracted body, got {len(names)}"
    )
    constants = {name: resolve_sdk_error(name) for name in names}
    declarations = "\n".join(
        f"static const HRESULT {name} = {value};" for name, value in constants.items()
    )
    return declarations, constants, fac_dp
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

int parse_says(int slot, const char *text, int arg2, int arg3) {
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

// SDK_HRESULT_CONSTANTS
class Net {
public:
    int report_directplay_error(HRESULT error);
};
// ACTUAL_NET_ERROR_BODY

int main(int argc, char **argv) {
    assert(argc == 4);
    const uint32_t bits = static_cast<uint32_t>(strtoul(argv[1], 0, 0));
    popup_result = atoi(argv[2]);
    cd_result = atoi(argv[3]);
    Net net;
    const int result = net.report_directplay_error(static_cast<HRESULT>(bits));
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


def compile_body(tmp_path: Path, body: str, constants: str, compiler_name: str) -> Path:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    fixture = HOST_FIXTURE
    assert fixture.count("// SDK_HRESULT_CONSTANTS") == 1
    assert fixture.count("// ACTUAL_NET_ERROR_BODY") == 1
    fixture = fixture.replace("// SDK_HRESULT_CONSTANTS", constants)
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
    body = definition(source, "int Net::report_directplay_error(HRESULT error)")
    constants, _, _ = sdk_hresult_environment(body)
    binary = compile_body(tmp_path, body, constants, compiler_name)
    assert run(binary, "0x88770136", -1, 73) == "1|DPERR_SESSIONLOST|1|0|0"
    assert run(binary, "0x88770132", -1, 73) == "0|-|1|0|0"
    assert run(binary, "0x887700AA", 3, 41) == "1|DPERR_NOCONNECTION|1|1|41"
    assert run(binary, "0x80004002", 3, -9) == "1|DPERR_NOINTERFACE|1|1|-9"
    assert run(binary, "0xDEADBEEF", -1, 73) == "0|-|1|0|0"
    assert run(binary, "0xDEADBEEF", 3, 73) == "0|-|1|1|73"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_value_mutant_fails_expected_session_lost_output(tmp_path: Path, compiler_name: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = definition(source, "int Net::report_directplay_error(HRESULT error)")
    constants, _, fac_dp = sdk_hresult_environment(body)
    expected_case = "case DPERR_SESSIONLOST:"
    assert body.count(expected_case) == 1
    wrong132 = 0x80000000 | (fac_dp << 16) | 0x132
    if wrong132 >= 0x80000000:
        wrong132 -= 0x100000000
    wrong_value = body.replace(expected_case, f"case {wrong132}:")
    mutant_constants, _, _ = sdk_hresult_environment(wrong_value, expected_cases=38)
    binary = compile_body(tmp_path, wrong_value, mutant_constants, compiler_name)
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
