"""Actual-source controls for Datalink's scalar category/remainder split."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PARSER_SIGNATURE = "void Datalink::parse_id(int id, uint32_t *out_category, int *out_remainder)"


def extract_body(path: Path, signature: str) -> str:
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


PARSER_BODY = extract_body(ROOT / "src/datalink.cpp", PARSER_SIGNATURE)


HARNESS = r'''
#include <stdint.h>
#include <stdlib.h>

class Datalink {
public:
    void parse_id(int id, uint32_t *out_category, int *out_remainder);
};
Datalink Parser;
void require(bool value) { if (!value) exit(1); }

void check_id(int id) {
    uint32_t category = 0xDEADBEEFU;
    int remainder = 0x12345678;
    Parser.parse_id(id, &category, &remainder);
    const int32_t expected_quotient = id / 10000;
    const uint32_t expected_category = static_cast<uint32_t>(expected_quotient);
    const uint32_t expected_remainder = static_cast<uint32_t>(id) -
        static_cast<uint32_t>(expected_quotient) * 10000U;
    require(category == expected_category);
    require(static_cast<uint32_t>(remainder) == expected_remainder);
}

int main() {
    const int ids[] = {
        0, 1, 9999, 10000, 10001, 19999, 20000,
        -1, -9999, -10000, -10001, -19999, -20000,
        INT32_MIN, INT32_MIN + 1, INT32_MAX - 1, INT32_MAX
    };
    for (unsigned int i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
        check_id(ids[i]);
    }

    // Both original output stores target the same dword when the caller aliases
    // its pointers; the later remainder store must be the final visible value.
    uint32_t shared = 0;
    Parser.parse_id(-12345, &shared, reinterpret_cast<int *>(&shared));
    require(shared == static_cast<uint32_t>(-2345));
    return 0;
}
'''


def compile_and_run(compiler: str, body: str, tmp_path: Path):
    source = tmp_path / "datalink_parse_id.cpp"
    executable = tmp_path / "datalink_parse_id"
    source.write_text(HARNESS + "\n" + body + "\n")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_parse_id_signed_category_wrap_remainder_and_alias(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = PARSER_BODY
    assert "const int32_t quotient = id / 10000;" in body
    assert "*out_category = static_cast<uint32_t>(quotient);" in body
    assert body.index("*out_category =") < body.index("*out_remainder =")
    assert "static_cast<uint32_t>(id) - static_cast<uint32_t>(quotient) * 10000U" in body
    result = compile_and_run(compiler, body, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


def test_newtechwin_exec_uses_separate_scalar_category_and_entry_outputs():
    body = extract_body(ROOT / "src/newtechwin.cpp", "void NewTechWin::exec(int technology_id, int other_faction_id)")
    category_decl = "uint32_t category_id = 0;"
    entry_decl = "int entry_id = 0;"
    parse_call = "DatalinkMain.parse_id(PendingTechnologyLink, &category_id, &entry_id);"
    exec_call = "DatalinkMain.exec(category_id, entry_id);"
    assert category_decl in body and entry_decl in body
    assert body.index(category_decl) < body.index(entry_decl) < body.index(parse_call) < body.index(exec_call)
    assert "reinterpret_cast<DatalinkID" not in body
    header = (ROOT / "src/datalink.h").read_text()
    assert "void parse_id(int id, uint32_t *out_category, int *out_remainder);" in header
    assert "struct DatalinkID;" not in header


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_source_wrong_floor_quotient_mutant_compiles_then_fails(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    damaged = PARSER_BODY.replace(
        "const int32_t quotient = id / 10000;",
        "const int32_t quotient = id / 10000 - (id % 10000 < 0 ? 1 : 0);",
        1,
    )
    assert damaged != PARSER_BODY
    result = compile_and_run(compiler, damaged, tmp_path)
    assert result.returncode != 0, "floor-division damage escaped signed quotient cases"
