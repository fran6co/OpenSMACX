"""Actual CheckBox layout body: retain the pinned unchecked payload access."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/checkbox.cpp"
CURRENT_GUARD = (
    "char *text = entry == 0 || entry->payload == 0 ? 0 : "
    "entry->string_payload()->text;"
)
PINNED_ACCESS = "char *text = entry == 0 ? 0 : entry->string_payload()->text;"


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, "actual product definition missing: " + signature
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated product definition: " + signature)


HARNESS_PREFIX = r'''#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstring>

struct Font {};
struct StringStructPayload { char *text; };
struct StringStructEntry {
    StringStructEntry *next;
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
};
struct StringList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringList() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
};
struct Dialog {
    StringList item_list_;
    uint32_t field_20_;
    int field_2C_, field_30_;
    uint32_t field_34_;
    int field_40_;
    uint32_t field_44_, field_48_;
    int field_4C_;
    int column_count_, row_count_;
    uint32_t field_5C_, field_60_;
    int field_64_, field_68_;
    Font *font1_, *font2_, *font3_;
    uint32_t text_color_a_, text_color_b_, text_color_c_, text_color_d_;
    uint32_t text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_;
    uint32_t text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_;
    Dialog()
        : field_20_(0), field_2C_(0), field_30_(0), field_34_(1), field_40_(0),
          field_44_(0), field_48_(1), field_4C_(1), column_count_(0), row_count_(0),
          field_5C_(0), field_60_(0), field_64_(0), field_68_(0),
          font1_(0), font2_(0), font3_(0),
          text_color_a_(0), text_color_b_(0), text_color_c_(0), text_color_d_(0),
          text_color_2a_(0), text_color_2b_(0), text_color_2c_(0), text_color_2d_(0),
          text_color_3a_(0), text_color_3b_(0), text_color_3c_(0), text_color_3d_(0) {}
};
struct Buffer {
    int width_calls;
    int height_calls;
    char *last_text;
    Buffer() : width_calls(0), height_calls(0), last_text(0) {}
    void set_text_color(int, int, int, int) {}
    void set_text_color2(int, int, int, int) {}
    void set_text_color3(int, int, int, int) {}
    int set_font(Font *, Font *, Font *, Font *) { return 0; }
    int text_width(char *text) {
        ++width_calls;
        last_text = text;
        std::fputs("TEXT_WIDTH\n", stderr);
        std::fflush(stderr);
        return text != 0 ? 9 : 0;
    }
    int text_height() { ++height_calls; return 1; }
};
struct GraphicWin : Buffer {};
struct Sprite { int width() { return 1; } int height() { return 1; } };
struct CheckBox : GraphicWin, Dialog {
    Sprite *unchecked_sprite_;
    Sprite *checked_sprite_;
    CheckBox() : GraphicWin(), Dialog(), unchecked_sprite_(0), checked_sprite_(0) {}
    int calculate_dimensions();
};
'''


HARNESS_MAIN = r'''
int main(int argc, char **argv) {
    assert(argc == 2);
    const char *mode = argv[1];
    CheckBox box;
    StringStructEntry entry;
    StringStructPayload payload;
    char label[] = "payload-text";
    entry.next = &entry;
    entry.payload = &payload;
    payload.text = label;

    if (std::strcmp(mode, "empty") == 0) {
        box.Dialog::item_list_.entry_count_ = 0;
        assert(box.calculate_dimensions() == 7);
        assert(box.width_calls == 0);
        return 0;
    }

    box.Dialog::item_list_.entry_count_ = 1;
    box.Dialog::field_4C_ = 1;
    box.Dialog::field_34_ = 1;
    box.Dialog::field_48_ = 1;
    box.Dialog::field_44_ = 0;
    box.Dialog::field_40_ = 0;
    box.Dialog::field_2C_ = 0;
    box.Dialog::field_20_ = 0;
    if (std::strcmp(mode, "null-current") == 0) {
        box.Dialog::item_list_.head_ = 0;
        box.Dialog::item_list_.current_ = 0;
    } else {
        box.Dialog::item_list_.head_ = &entry;
        box.Dialog::item_list_.current_ = &entry;
        if (std::strcmp(mode, "null-payload") == 0) {
            entry.payload = 0;
        }
    }

    const int result = box.calculate_dimensions();
    assert(result == 0);
    assert(box.width_calls == 1);
    if (std::strcmp(mode, "valid") == 0) {
        assert(box.last_text == label);
    } else {
        assert(std::strcmp(mode, "null-current") == 0 ||
               std::strcmp(mode, "null-payload") == 0);
        assert(box.last_text == 0);
    }
    return 0;
}
'''


def _disable_core_dump():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def _compile(compiler: str, body: str, output: Path):
    source = output.with_suffix(".cpp")
    source.write_text(HARNESS_PREFIX + body + HARNESS_MAIN)
    subprocess.run(
        [compiler, "-std=c++98", "-O0", str(source), "-o", str(output)],
        check=True,
        capture_output=True,
        text=True,
    )


def test_actual_checkbox_layout_preserves_payload_fault_and_null_current(tmp_path):
    compilers = [candidate for candidate in ("g++", "clang++") if shutil.which(candidate)]
    if not compilers:
        pytest.skip("host C++ compiler unavailable")

    source = SOURCE.read_text()
    body = extract_definition(source, "int CheckBox::calculate_dimensions()")
    assert body.count(PINNED_ACCESS) == 1
    assert CURRENT_GUARD not in body
    mutant = body.replace(PINNED_ACCESS, CURRENT_GUARD)
    assert mutant != body

    for index, compiler in enumerate(compilers):
        candidate = tmp_path / ("candidate-" + str(index))
        safe_mutant = tmp_path / ("safe-mutant-" + str(index))
        _compile(compiler, body, candidate)
        _compile(compiler, mutant, safe_mutant)

        empty = subprocess.run([str(candidate), "empty"], capture_output=True, text=True)
        assert empty.returncode == 0, empty.stderr
        assert "TEXT_WIDTH" not in empty.stderr

        valid = subprocess.run([str(candidate), "valid"], capture_output=True, text=True)
        assert valid.returncode == 0, valid.stderr
        assert valid.stderr.count("TEXT_WIDTH") == 1

        null_current = subprocess.run(
            [str(candidate), "null-current"], capture_output=True, text=True
        )
        assert null_current.returncode == 0, null_current.stderr
        assert null_current.stderr.count("TEXT_WIDTH") == 1

        fault = subprocess.run(
            [str(candidate), "null-payload"], capture_output=True, text=True,
            preexec_fn=_disable_core_dump,
        )
        assert fault.returncode != 0
        assert "TEXT_WIDTH" not in fault.stderr

        mutant_result = subprocess.run(
            [str(safe_mutant), "null-payload"], capture_output=True, text=True
        )
        assert mutant_result.returncode == 0, mutant_result.stderr
        assert mutant_result.stderr.count("TEXT_WIDTH") == 1
