from pathlib import Path
import hashlib
import shutil
import subprocess

import pytest

from match_recovery.inspection import inspect_source


ROOT = Path(__file__).resolve().parents[2]
ORIGINAL_IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _pinned_image() -> Path:
    for candidate_root in (ROOT, *ROOT.parents):
        candidate = candidate_root / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    raise AssertionError("pinned original image is unavailable")


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _harness(source: str) -> str:
    helper = _definition(source, "void erase_string_list_current(")
    method = _definition(source, "void StringBox::UNK2(")
    return r"""
#include <stdint.h>
struct StringStructPayload {};
struct StringStructEntry {
    StringStructEntry *previous, *next;
    int id;
    void *payload;
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
};
struct StringList {
    StringStructEntry *head_, *current_;
    int entry_count_, current_position_;
    void on_remove(StringStructPayload *) {}
};
struct StringBox {
    StringList stringList_;
    int fixups;
    StringBox() : fixups(0) {}
    void add_fixup() { ++fixups; }
    void UNK2(int first_bound, int second_bound);
};
namespace {
""" + helper + r"""
}
""" + method + r"""

int main() {
    const int ids[] = {-1, -2, 0, 3, 4, 2};
    StringStructEntry *nodes[6];
    for (int i = 0; i != 6; ++i) {
        nodes[i] = new StringStructEntry;
        nodes[i]->previous = 0;
        nodes[i]->next = 0;
        nodes[i]->id = ids[i];
        nodes[i]->payload = 0;
    }
    for (int i = 0; i != 6; ++i) {
        nodes[i]->previous = nodes[(i + 5) % 6];
        nodes[i]->next = nodes[(i + 1) % 6];
    }
    StringBox box;
    box.stringList_.head_ = nodes[0];
    box.stringList_.current_ = nodes[0];
    box.stringList_.entry_count_ = 6;
    box.stringList_.current_position_ = 0;
    box.UNK2(-1, 3);

    // The pinned CMP/JAE and CMP/JBE sequence is unsigned: with first_bound
    // equal to UINT32_MAX, -2 remains inside the interval but -1 does not.
    // The original changing-count traversal leaves the final out-of-range 2
    // unvisited; caching the initial count would visit and remove that node.
    StringStructEntry *entry = box.stringList_.head_;
    const int expected[] = {-2, 4, 2};
    if (box.stringList_.entry_count_ != 3 || box.stringList_.current_position_ != 2
        || box.fixups != 1) return 1;
    for (int i = 0; i != 3; ++i) {
        if (entry == 0 || entry->id != expected[i]) return 2 + i;
        entry = entry->next;
    }
    return entry == box.stringList_.head_ ? 0 : 6;
}
"""


def _init_harness(source: str) -> str:
    close_method = _definition(source, "void StringBox::close(")
    init_method = _definition(source, "int StringBox::init(")
    return r"""
struct Win {};
struct StringList {
    int remove_calls;
    int current_position_;
    int allocator_;
    StringList() : remove_calls(0), current_position_(9), allocator_(9) {}
    void remove_all() { ++remove_calls; }
};
struct Scroll {
    int close_calls, init_calls, hide_calls;
    int x, y, length, setting;
    Win *parent;
    int range_minimum_, range_maximum_;
    Scroll() : close_calls(0), init_calls(0), hide_calls(0), x(0), y(0),
               length(0), setting(0), parent(0), range_minimum_(9),
               range_maximum_(9) {}
    void close() { ++close_calls; }
    int init_vert(int x_value, int y_value, int length_value,
                  Win *parent_value, int setting_value) {
        ++init_calls;
        x = x_value;
        y = y_value;
        length = length_value;
        parent = parent_value;
        setting = setting_value;
        return 0;
    }
    void hide() { ++hide_calls; }
};
struct DibHeader { int biHeight; };
struct Dib { DibHeader bmiHeader; };
struct GraphicWin : Win {
    int init_result, init_calls, close_calls;
    int init_x, init_y, init_width, init_height, init_flags;
    int mutate_embedded_list;
    int *embedded_position, *embedded_allocator;
    Win *init_parent;
    GraphicWin() : init_result(0), init_calls(0), close_calls(0), init_x(0),
                   init_y(0), init_width(0), init_height(0), init_flags(0),
                   mutate_embedded_list(0), embedded_position(0),
                   embedded_allocator(0), init_parent(0) {}
    void close() { ++close_calls; }
    int init(int x, int y, int width, int height, char *, int flags,
             Win *parent, void *, void *) {
        ++init_calls;
        init_x = x;
        init_y = y;
        init_width = width;
        init_height = height;
        init_flags = flags;
        init_parent = parent;
        if (mutate_embedded_list) {
            *embedded_position = 41;
            *embedded_allocator = 42;
        }
        return init_result;
    }
};
struct StringBox : GraphicWin {
    StringList stringList_;
    StringList *list_override_;
    Scroll scroll_;
    Dib dib_;
    unsigned int visible_rows_;
    int top_line_, per_line_draw_;
    StringBox() : list_override_(0), visible_rows_(0), top_line_(0),
                  per_line_draw_(0) {
        dib_.bmiHeader.biHeight = 0;
        embedded_position = &stringList_.current_position_;
        embedded_allocator = &stringList_.allocator_;
    }
    void close();
    int init(int x, int y, int width, int height, Win *parent,
             StringList *list, int flags);
};
int ScrollDefaultThickness = 12;
""" + close_method + init_method + r"""

int main() {
    Win parent;
    StringList borrowed;
    StringBox box;
    box.dib_.bmiHeader.biHeight = -37;
    box.list_override_ = &borrowed;
    if (box.init(5, 6, 101, 47, &parent, &borrowed, -1) != 0) return 1;
    if (box.close_calls != 1 || box.init_calls != 1) return 2;
    if (box.init_x != 5 || box.init_y != 6 || box.init_width != 101
        || box.init_height != 47 || box.init_flags != 0x1000120
        || box.init_parent != &parent) return 3;
    if (box.list_override_ != &borrowed || box.stringList_.remove_calls != 1)
        return 4;
    if (box.stringList_.current_position_ != 0 || box.stringList_.allocator_ != 9)
        return 5;
    if (box.scroll_.init_calls != 1 || box.scroll_.x != 89 || box.scroll_.y != 0
        || box.scroll_.length != 37 || box.scroll_.parent != &box
        || box.scroll_.setting != 0 || box.scroll_.hide_calls != 1
        || box.scroll_.range_minimum_ != 0 || box.scroll_.range_maximum_ != 0)
        return 6;

    StringBox callback_mutated;
    callback_mutated.dib_.bmiHeader.biHeight = -29;
    callback_mutated.mutate_embedded_list = 1;
    if (callback_mutated.init(3, 4, 97, 29, &parent, &borrowed, 0x2468) != 0)
        return 7;
    if (callback_mutated.stringList_.current_position_ != 41
        || callback_mutated.stringList_.allocator_ != 42
        || callback_mutated.stringList_.remove_calls != 1
        || callback_mutated.list_override_ != &borrowed) return 8;

    StringBox embedded;
    embedded.dib_.bmiHeader.biHeight = -31;
    embedded.mutate_embedded_list = 1;
    if (embedded.init(8, 9, 83, 31, &parent, 0, 0x12345) != 0) return 9;
    if (embedded.init_flags != 0x12345 || embedded.list_override_ != 0
        || embedded.stringList_.remove_calls != 2
        || embedded.scroll_.x != 71 || embedded.scroll_.length != 31
        || embedded.stringList_.current_position_ != 0
        || embedded.stringList_.allocator_ != 0)
        return 10;

    StringBox failed;
    failed.dib_.bmiHeader.biHeight = -23;
    failed.list_override_ = &borrowed;
    failed.init_result = 7;
    failed.scroll_.range_minimum_ = 111;
    failed.scroll_.range_maximum_ = 222;
    if (failed.init(1, 2, 71, 23, &parent, &borrowed, -1) != 7) return 11;
    if (failed.close_calls != 1 || failed.init_calls != 1
        || failed.init_flags != 0x1000120 || failed.list_override_ != 0
        || failed.stringList_.remove_calls != 1) return 12;
    if (failed.scroll_.close_calls != 1 || failed.scroll_.init_calls != 0
        || failed.scroll_.hide_calls != 0 || failed.scroll_.range_minimum_ != 111
        || failed.scroll_.range_maximum_ != 222) return 13;

    StringBox failed_negative;
    failed_negative.init_result = -13;
    if (failed_negative.init(2, 3, 61, 19, &parent, 0, 0) != -13)
        return 14;
    if (failed_negative.close_calls != 1 || failed_negative.init_calls != 1
        || failed_negative.scroll_.init_calls != 0
        || failed_negative.scroll_.hide_calls != 0)
        return 15;
    return 0;
}
"""


def _compile_and_run(tmp_path: Path, compiler: str, source: str, name: str) -> int:
    cpp = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=5).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_stringbox_unk2_keeps_pinned_unsigned_interval_and_loop(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    source = (ROOT / "src/stringbox.cpp").read_text()
    harness = _harness(source)
    assert _compile_and_run(tmp_path, compiler, harness, "faithful") == 0

    signed_method = _definition(source, "void StringBox::UNK2(")
    signed_method = signed_method.replace(
        "const uint32_t id = static_cast<uint32_t>(list.current_->id);",
        "const int id = list.current_->id;",
    ).replace("static_cast<uint32_t>(first_bound)", "first_bound").replace(
        "static_cast<uint32_t>(second_bound)", "second_bound"
    )
    assert "const int id = list.current_->id;" in signed_method
    assert "static_cast<uint32_t>(first_bound)" not in signed_method
    assert "static_cast<uint32_t>(second_bound)" not in signed_method
    signed_source = source.replace(_definition(source, "void StringBox::UNK2("), signed_method)
    assert _compile_and_run(tmp_path, compiler, _harness(signed_source), "signed_mutant") != 0

    changing_count_method = _definition(source, "void StringBox::UNK2(")
    cached_count_method = changing_count_method.replace(
        "int visited = 0;",
        "int visited = 0;\n        const int original_count = list.entry_count_;",
    ).replace("while (visited < list.entry_count_);", "while (visited < original_count);")
    assert cached_count_method != changing_count_method
    assert "const int original_count = list.entry_count_;" in cached_count_method
    assert "while (visited < original_count);" in cached_count_method
    assert "while (visited < list.entry_count_);" not in cached_count_method
    cached_count_source = source.replace(changing_count_method, cached_count_method)
    assert _compile_and_run(tmp_path, compiler, _harness(cached_count_source), "cached_count_mutant") != 0


def test_stringbox_unk2_predicate_matches_pinned_unsigned_branches():
    image = _pinned_image()
    assert hashlib.sha256(image.read_bytes()).hexdigest() == ORIGINAL_IMAGE_SHA256
    report = inspect_source(ROOT, ["0x00629980"], image=image)
    assert report["result"] == "pass" and report["image_verified"]
    subject, = report["subjects"]
    assert subject["source"]["path"] == "src/stringbox.cpp"
    span, = subject["original"]["spans"]
    assert span["result"] == "pass"
    assert span["span"] == [0x00629980, 0x00629A63]
    instructions = {line.split()[0]: line for line in span["disassembly"]}
    assert "cmp ecx, ebp" in instructions["0x006299BC"]
    assert "jae 0x6299ce" in instructions["0x006299BE"]
    assert "cmp ecx, dword ptr [esp + 0x20]" in instructions["0x006299C0"]
    assert "jbe 0x6299ce" in instructions["0x006299C4"]
    assert "mov eax, dword ptr [eax + 0xc]" in instructions["0x006299C6"]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_stringbox_init_uses_width_for_vertical_scroll_x_and_preserves_paths(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    source = (ROOT / "src/stringbox.cpp").read_text()
    assert _compile_and_run(tmp_path, compiler, _init_harness(source), "init_faithful") == 0

    init_method = _definition(source, "int StringBox::init(")
    height_mutant = init_method.replace(
        "scroll_.init_vert(width - ScrollDefaultThickness, 0,",
        "scroll_.init_vert(height - ScrollDefaultThickness, 0,",
    )
    assert height_mutant != init_method
    assert "scroll_.init_vert(height - ScrollDefaultThickness, 0," in height_mutant
    height_mutant_source = source.replace(init_method, height_mutant)
    assert _compile_and_run(
        tmp_path, compiler, _init_harness(height_mutant_source), "init_height_mutant"
    ) != 0

    init_method = _definition(source, "int StringBox::init(")
    failure_normalization_mutant = init_method.replace(
        "return init_result;", "return init_result < 0 ? -1 : 1;"
    )
    assert failure_normalization_mutant != init_method
    failure_normalization_source = source.replace(
        init_method, failure_normalization_mutant
    )
    assert _compile_and_run(
        tmp_path, compiler, _init_harness(failure_normalization_source),
        "init_normalized_failure_mutant",
    ) != 0

    nonzero_success_mutant = init_method.replace("    return 0;", "    return 1;")
    assert nonzero_success_mutant != init_method
    nonzero_success_source = source.replace(init_method, nonzero_success_mutant)
    assert _compile_and_run(
        tmp_path, compiler, _init_harness(nonzero_success_source),
        "init_nonzero_success_mutant",
    ) != 0

    borrowed_clear_mutant = init_method.replace(
        "    } else {\n        stringList_.remove_all();\n"
        "        stringList_.current_position_ = 0;\n"
        "        stringList_.allocator_ = 0;\n    }",
        "    } else {\n        stringList_.remove_all();\n    }\n"
        "    stringList_.current_position_ = 0;\n"
        "    stringList_.allocator_ = 0;",
    )
    assert borrowed_clear_mutant != init_method
    borrowed_clear_source = source.replace(init_method, borrowed_clear_mutant)
    assert _compile_and_run(
        tmp_path, compiler, _init_harness(borrowed_clear_source),
        "init_unconditional_clear_mutant",
    ) != 0


def test_stringbox_init_scroll_x_uses_original_third_argument():
    image = _pinned_image()
    assert hashlib.sha256(image.read_bytes()).hexdigest() == ORIGINAL_IMAGE_SHA256
    report = inspect_source(ROOT, ["0x006292D0"], image=image)
    assert report["result"] == "pass" and report["image_verified"]
    subject, = report["subjects"]
    assert subject["source"]["path"] == "src/stringbox.cpp"
    span, = subject["original"]["spans"]
    assert span["result"] == "pass"
    assert span["span"] == [0x006292D0, 0x00629486]
    instructions = {line.split()[0]: line for line in span["disassembly"]}
    assert "mov edi, dword ptr [esp + 0x20]" in instructions["0x00629395"]
    assert "sub edi, ecx" in instructions["0x00629458"]
    assert "push edi" in instructions["0x00629461"]
    assert "call 0x605890" in instructions["0x00629464"]
    assert "jmp 0x629447" in instructions["0x006293CB"]
    assert "mov dword ptr [esi + 0x14], ebp" in instructions["0x00629441"]
    assert "mov dword ptr [esi + 0x18], ebp" in instructions["0x00629444"]
