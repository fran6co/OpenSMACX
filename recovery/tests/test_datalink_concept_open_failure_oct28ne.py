"""Exercise Datalink's pinned text-open success and failure paths."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
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
    raise AssertionError(f"unterminated body: {signature}")


HARNESS = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[8];
static char StringTemp[256];
static char DatalinkConceptIdSeparator[] = "|";
static char DatalinkConceptSection[] = "CONCEPTSX";
static char DatalinkConceptFile[] = "TITLES";
static const char *TextBufferGetPtr = 0;
static int open_result = 0;
static int reader_step = 0;
static char first_line[] = "Concept";
static std::vector<std::string> events;
static struct { struct { unsigned int game_state_; } fields_; }
    GameDataStateStorage = {{0}};

static int text_open(char *, char *) {
    events.push_back("open");
    return open_result;
}
static void text_get() {
    events.push_back("get");
    TextBufferGetPtr = reader_step++ == 0 ? first_line : const_cast<char*>("");
}
static void text_close() { events.push_back("close"); }
static char *_itoa(int value, char *out, int radix) {
    assert(radix == 10);
    std::sprintf(out, "%d", value);
    return out;
}
static const char *category_label(int) { return "Concepts"; }
#define CATEGORY_LABEL(slot) category_label(slot)

struct ListBox {
    std::vector<std::string> items;
    std::vector<int> ids;
    void item(const char *text, int id) {
        events.push_back("item");
        items.push_back(text);
        ids.push_back(id);
    }
};
struct Datalink {
    ListBox listBox_;
    RECT category_rect_ = {0, 0, 100, 80};
    int font3_ = 0;
    void box_sprite(RECT *, BoxSpriteParams *) { events.push_back("box"); }
    void set_text_color(int, int, int, int) { events.push_back("color"); }
    void set_font(void *, int, int, int) { events.push_back("font"); }
    int write_cent_l(char *, RECT *, unsigned int) {
        events.push_back("write");
        return 0;
    }
    void soft_update(RECT *) { events.push_back("update"); }
    void set_cat_concept();
};

__DATALINK_BODY__

static void reset(int result) {
    events.clear();
    open_result = result;
    reader_step = 0;
    TextBufferGetPtr = 0;
}

int main() {
    Datalink datalink;

    reset(1);
    datalink.set_cat_concept();
    assert((events == std::vector<std::string>{"open"}));
    assert(datalink.listBox_.items.empty());

    reset(0);
    datalink.set_cat_concept();
    const std::vector<std::string> expected = {
        "open", "get", "item", "get", "close", "box", "color",
        "font", "write", "update"
    };
    assert(events == expected);
    assert(datalink.listBox_.items.size() == 1);
    assert(datalink.listBox_.items[0] == "Concept");
    assert(datalink.listBox_.ids[0] == 0);
    return 0;
}
'''


def _compile_and_run(compiler: str, source: str, output: Path) -> subprocess.CompletedProcess[str]:
    generated = output.with_suffix(".cpp")
    generated.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(generated), "-o", str(output)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(output)], capture_output=True, text=True)


def test_concept_open_failure_skips_close_and_drawing_and_success_reads(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    source = (ROOT / "src/datalink.cpp").read_text()
    body = _body(source, "void Datalink::set_cat_concept()")
    assert "if (text_open(DatalinkConceptSection, DatalinkConceptFile) != 0)" in body
    actual = _compile_and_run(
        compiler,
        HARNESS.replace("__DATALINK_BODY__", body),
        tmp_path / "concept_open_failure",
    )
    assert actual.returncode == 0, actual.stdout + actual.stderr

    # Restore the previous shape: failed open skips reading/closing but falls
    # through to category drawing. The executed failure assertion must reject it.
    old_shape = body.replace(
        "if (text_open(DatalinkConceptSection, DatalinkConceptFile) != 0) {\n"
        "        return;\n"
        "    }",
        "if (text_open(DatalinkConceptSection, DatalinkConceptFile) == 0) {",
        1,
    )
    assert old_shape != body
    old_shape = old_shape.replace("    RECT rect = category_rect_;", "    }\n\n    RECT rect = category_rect_;", 1)
    mutant = _compile_and_run(
        compiler,
        HARNESS.replace("__DATALINK_BODY__", old_shape),
        tmp_path / "concept_open_failure_old_shape",
    )
    assert mutant.returncode != 0
