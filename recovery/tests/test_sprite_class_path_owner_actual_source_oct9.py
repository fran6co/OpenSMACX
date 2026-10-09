"""Host controls for Sprite::init_class extracted from its mapped owner."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_text


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/sprite.cpp"
SIGNATURE = "int Sprite::init_class()"


def extract_braced(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state == "string":
            if ch == "\\":
                i += 1
            elif ch == '"':
                state = "code"
        elif state == "char":
            if ch == "\\":
                i += 1
            elif ch == "'":
                state = "code"
        elif ch == "/" and nxt == "/":
            state = "line"
            i += 1
        elif ch == "/" and nxt == "*":
            state = "block"
            i += 1
        elif ch == '"':
            state = "string"
        elif ch == "'":
            state = "char"
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1
    raise AssertionError("unbalanced Sprite::init_class definition")


def installed_source() -> tuple[str, str, str]:
    text = PRODUCT.read_text(encoding="utf-8")
    records = [row for row in read_text(text, PRODUCT.resolve())
               if row.address == 0x0063CE20]
    assert len(records) == 1 and records[0].mode is Mode.BODY
    assert records[0].byte_exact, "historical BYTE_EXACT obligation must remain intact"
    assert text.count(SIGNATURE) == 1
    function = extract_braced(text, text.index(SIGNATURE))
    path_declaration = 'static const char JackalPcxPath[] = "jackal.pcx";'
    assert text.count(path_declaration) == 1
    return text, function, path_declaration


PREFIX = r'''#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#ifndef __cdecl
#define __cdecl
#endif

struct Event {
    std::string name;
    const void *pointer;
    int a, b, c, d, e, f;
};
std::vector<Event> events;
int mock_load_result;
std::string last_path;
static void event(const char *name, const void *pointer = 0,
                  int a = 0, int b = 0, int c = 0,
                  int d = 0, int e = 0, int f = 0) {
    Event value = {name, pointer, a, b, c, d, e, f};
    events.push_back(value);
}

struct Palette {};
class Buffer {
public:
    Buffer() { event("buffer-ctor", this); }
    ~Buffer() { event("buffer-dtor", this); }
    int load_result;
    int load_pcx(const char *path, Palette *palette, int tgl, int height) {
        event("load", this, palette == 0, tgl, height);
        last_path = path;
        return mock_load_result;
    }
};
class Sprite {
public:
    static int init_class();
    int blank_result, extract_result;
    int create_blank(int width, int height, int depth) {
        event("create_blank", this, width, height, depth);
        return blank_result;
    }
    int extract(Buffer *buffer, int transparent, int x, int y,
                int width, int height, int heap) {
        event("extract", this, transparent, x, y, width, height, heap);
        extract_buffer = buffer;
        return extract_result;
    }
    Buffer *extract_buffer;
};

Sprite &blank_sprite();
Sprite blank_sprite_value = {};
Sprite &blank_sprite() { event("blank_sprite", &blank_sprite_value); return blank_sprite_value; }
'''

SUFFIX = r'''
static int fail(int id) {
    fprintf(stderr, "FAIL init_class control %d\n", id);
    return id;
}
static void check_case(int load_result, int operation_result,
                       bool load_failure, int expected_result, int id) {
    events.clear();
    mock_load_result = load_result;
    blank_sprite_value.blank_result = operation_result;
    blank_sprite_value.extract_result = operation_result;
    const int result = Sprite::init_class();
    if (result != expected_result) { fail(id); assert(result == expected_result); }
    if (events.size() != 5) { fail(id + 10); assert(events.size() == 5); }
    assert(events[0].name == "buffer-ctor");
    assert(events[0].pointer != 0);
    assert(events[1].name == "load");
    assert(events[1].a == 1 && events[1].b == 10 && events[1].c == 0xEC);
    assert(last_path == "jackal.pcx");
    if (load_failure) {
        assert(events[2].name == "blank_sprite");
        assert(events[3].name == "create_blank");
        assert(events[3].a == 0x10 && events[3].b == 0x10 && events[3].c == 9);
    } else {
        assert(events[2].name == "blank_sprite");
        assert(events[3].name == "extract");
        assert(events[3].a == 9 && events[3].b == 1 && events[3].c == 0x12 &&
               events[3].d == 0x10 && events[3].e == 0x10 && events[3].f == 0);
        assert(blank_sprite_value.extract_buffer ==
               static_cast<Buffer *>(const_cast<void *>(events[1].pointer)));
    }
    // Buffer's automatic destructor runs after the selected operation and
    // before init_class returns on both the nonzero and zero result paths.
    assert(events.back().name == "buffer-dtor");
    assert(events.back().pointer == events[0].pointer);
}

int main() {
    assert(strcmp(JackalPcxPath, "jackal.pcx") == 0);
    check_case(1, 27, true, 27, 1);
    check_case(-1, 0, true, 0, 2);
    check_case(0, -13, false, -13, 3);
    check_case(0, 0, false, 0, 4);
    puts("Sprite::init_class actual-source controls PASS");
    return 0;
}
'''


def compile_run(compiler: str, text: str, function: str, path_decl: str,
                tmp_path: Path, name: str) -> subprocess.CompletedProcess:
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(PREFIX + path_decl + "\n" + function + "\n" + SUFFIX,
                      encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], text=True, capture_output=True,
        check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True,
                          check=False)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_sprite_init_class_source_paths_lifetimes_and_mutants(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    text, function, path_decl = installed_source()
    baseline = compile_run(compiler, text, function, path_decl, tmp_path, "baseline")
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr
    assert "Sprite::init_class actual-source controls PASS" in baseline.stdout

    path_mutant = path_decl.replace('"jackal.pcx"', '"wrong.pcx"', 1)
    assert path_mutant != path_decl
    mutants = [(function, path_mutant, "path")]

    branch_mutant = function.replace("load_pcx(JackalPcxPath, 0, 10, 0xec) != 0",
                                     "load_pcx(JackalPcxPath, 0, 10, 0xec) == 0", 1)
    assert branch_mutant != function
    mutants.append((branch_mutant, path_decl, "branch"))

    argument_mutant = function.replace("load_pcx(JackalPcxPath, 0, 10, 0xec)",
                                       "load_pcx(JackalPcxPath, 0, 11, 0xec)", 1)
    assert argument_mutant != function
    mutants.append((argument_mutant, path_decl, "arguments"))

    return_mutant = function.replace("return result;", "return 0;", 1)
    assert return_mutant != function
    mutants.append((return_mutant, path_decl, "return"))

    destructor_mutant = function.replace(
        "    Buffer buf;", "    Buffer *buf = new Buffer();", 1
    ).replace("buf.load_pcx(", "buf->load_pcx(", 1).replace(
        "blank_sprite().extract(&buf,", "blank_sprite().extract(buf,", 1
    )
    assert destructor_mutant != function
    mutants.append((destructor_mutant, path_decl, "destructor-sequence"))

    for mutant_body, mutant_path, name in mutants:
        result = compile_run(compiler, text, mutant_body, mutant_path,
                             tmp_path, f"mutant-{name}")
        assert result.returncode != 0, (
            f"{compiler} control missed {name} mutant: {result.stdout}{result.stderr}"
        )
