"""Pinned branch/order/return controls; mock fonts do not prove native layout."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = Path(__file__).resolve().parents[2]


def installed_definition():
    source = (ROOT / "src/pulldown.cpp").read_text()
    signature = "int PullDown::compute_length(char *text)"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if not depth:
                return source[start:index + 1]
    raise AssertionError("missing installed definition")


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
struct Event { int operation; const char *text; int result; int font; int length; };
static Event events[16];
static int event_count, consumed;
struct Font {
    int id;
    int width(char *text) { return measure(text, 0, -1); }
    int width(const char *text) { return measure(text, 0, -1); }
    int width(char *text, int length) { return measure(text, 1, length); }
    int measure(const char *, int, int);
};
static Font primary = {1}, replacement = {2};
static Font *FontDefault = &primary;
static Font *PullDownDefaultFont;
struct PullDownItem { char *text; char *right_text; };
struct PullDown {
    int width_;
    PullDownItem items_[64];
    int compute_length(char *text);
};
static PullDown *mutated;
static int mutate_at = -1;
static char *new_left, *new_right;
static bool replace_font;
int Font::measure(const char *text, int operation, int length) {
    assert(consumed < event_count);
    int index = consumed++;
    Event &event = events[index];
    assert(event.operation == operation && event.font == id);
    assert(!std::strcmp(event.text, text));
    assert(event.length == length);
    if (index == mutate_at) {
        mutated->items_[0].text = new_left;
        mutated->items_[0].right_text = new_right;
    }
    if (replace_font && index == 0) PullDownDefaultFont = &replacement;
    return event.result;
}
static void reset() {
    event_count = consumed = 0;
    PullDownDefaultFont = nullptr;
    mutated = nullptr; mutate_at = -1; replace_font = false;
}
static void expect(const char *text, int result, int operation = 0,
                   int font = 1, int length = -1) {
    events[event_count++] = Event{operation, text, result, font, length};
}
static void complete() { assert(consumed == event_count); }
'''


MAIN = r'''
int main() {
    char short_text[] = "ab", left[] = "abcd", right[] = "xy";
    char split[] = "abcd|xy", shorter[] = "x", changed_left[] = "new-left";
    char changed_right[] = "new-right";
    PullDown single = {};
    single.width_ = 100;
    reset(); expect(short_text, 10);
    assert(single.compute_length(short_text) == 100 && single.width_ == 100);
    complete(); assert(PullDownDefaultFont == FontDefault);

    single.width_ = 10;
    reset(); expect(short_text, 10); expect(short_text, 7, 0, 2);
    replace_font = true;
    assert(single.compute_length(short_text) == 7 && single.width_ == 7);
    complete();

    single.width_ = 100;
    reset(); expect(split, 20, 1, 1, 4); expect("  ", 10); expect(right, 10);
    assert(single.compute_length(split) == 100 && single.width_ == 100);
    complete();

    single.width_ = 0;
    reset(); expect(split, 20, 1, 1, 4); expect("  ", 10); expect(right, 10);
    expect("  ", 11); expect(split, 23, 1, 1, 4); expect(right, 13);
    assert(single.compute_length(split) == 13 && single.width_ == 58);
    complete();

    PullDown table = {};
    table.items_[0].text = left;
    table.items_[63].text = shorter;
    reset(); expect(left, 20); expect(left, 20); expect(shorter, 5);
    assert(table.compute_length(nullptr) == 0 && table.width_ == 20);
    complete();

    table = PullDown{};
    table.items_[0] = PullDownItem{left, right};
    reset(); expect("  ", 10); expect(changed_left, 20); expect(right, 10);
    expect("  ", 11); expect(changed_right, 12); expect(changed_left, 13);
    mutated = &table; mutate_at = 0; new_left = changed_left; new_right = changed_right;
    assert(table.compute_length(nullptr) == 0 && table.width_ == 47);
    complete();

    table = PullDown{};
    table.items_[0] = PullDownItem{left, right};
    reset(); expect("  ", INT32_MAX); expect(left, 1); expect(right, 1);
    expect("  ", INT32_MAX); expect(right, 2); expect(left, 3);
    assert(table.compute_length(nullptr) == 0 && table.width_ == 3);
    complete();

    table = PullDown{};
    table.items_[0] = PullDownItem{left, right};
    reset(); expect("  ", INT32_MAX); expect(left, 0); expect(right, 0);
    assert(table.compute_length(nullptr) == 0 && table.width_ == 0);
    complete();
}
'''


def compile_run(definition, compiler, succeeds):
    with tempfile.TemporaryDirectory(prefix="osmx-pulldown-length-") as temporary:
        binary = Path(temporary) / "control"
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
             "-x", "c++", "-", "-o", str(binary)],
            input=PREAMBLE + definition + MAIN, text=True, capture_output=True, timeout=45,
        )
        assert compiled.returncode == 0, compiled.stderr
        executed = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (executed.returncode == 0) == succeeds, executed.stderr


def test_installed_compute_length_machine_behavior():
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers
    definition = installed_definition()
    for compiler in compilers:
        compile_run(definition, compiler, True)


@pytest.mark.parametrize("mutation", [
    "single_max", "table_max", "split_result", "table_result", "field_reread",
    "cached_font", "unsigned_compare", "measurement_order",
])
def test_controls_reject_behavior_mutants(mutation):
    definition = installed_definition()
    if mutation == "single_max":
        mutated = definition.replace("measured = current;", "measured = 0;", 1)
    elif mutation == "table_max":
        prefix, table = definition.split("width_ = 0;", 1)
        mutated = prefix + "width_ = 0;" + table.replace("measured = current;", "measured = 0;", 1)
    elif mutation == "split_result":
        mutated = definition.replace("return right_width;", "return width_;", 1)
    elif mutation == "table_result":
        mutated = definition.replace("return 0;", "return width_;", 1)
    elif mutation == "field_reread":
        mutated = definition.replace("width(items_[i].right_text);", "width(first_right_text);", 1)
    elif mutation == "cached_font":
        mutated = definition.replace("PullDownDefaultFont->", "FontDefault->")
    elif mutation == "unsigned_compare":
        mutated = definition.replace(
            "if (current > total)",
            "if (static_cast<uint32_t>(current) > static_cast<uint32_t>(total))", 1)
    else:
        first = "int left_width = PullDownDefaultFont->width(text, separator - text);"
        second = 'int separator_width = PullDownDefaultFont->width("  ");'
        mutated = definition.replace(first + "\n        " + second, second + "\n        " + first, 1)
    assert mutated != definition
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers
    for compiler in compilers:
        compile_run(mutated, compiler, False)
