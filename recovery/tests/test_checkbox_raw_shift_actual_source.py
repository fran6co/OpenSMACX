"""Bounded host controls for six current CheckBox variable-shift bodies.

The test compiles complete method definitions extracted from the product TU;
it does not copy their algorithms. All exercised shift counts are 0..31.
Out-of-range shifts, VC6/x86 behavior, ABI/layout, and game acceptance are
explicitly outside this host test. Existing getter-32 and caller tests remain
separate and are not replaced by these controls.
"""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/jackal/checkbox.cpp").is_file())
SOURCE = ROOT / "src/jackal/checkbox.cpp"
COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
METHODS = {
    "toggle": "void CheckBox::UNK1(int pos)",
    "masked_getter": "int CheckBox::UNK2(int pos)",
    "position_setter": "void CheckBox::set_state_pos(int pos, int state)",
    "item": "int CheckBox::item(char *text, int id, int state)",
    "id_getter": "int CheckBox::get_state_id(int id)",
    "mouse_move": "void CheckBox::on_mouse_move(int x_coord, int y_coord)",
}


def _mask_cpp(text):
    out = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            end = text.find("\n", i)
            if end < 0:
                end = len(text)
            for j in range(i, end):
                out[j] = " "
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            assert end >= 0, "unterminated block comment"
            end += 2
            for j in range(i, end):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        elif text[i] in "\"'":
            quote, end = text[i], i + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                elif text[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
            for j in range(i, min(end, len(text))):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        else:
            i += 1
    return "".join(out)


def _definition(source, signature):
    masked = _mask_cpp(source)
    starts = []
    at = 0
    while True:
        at = masked.find(signature, at)
        if at < 0:
            break
        starts.append(at)
        at += len(signature)
    assert len(starts) == 1, f"expected unique definition {signature!r}"
    start = starts[0]
    opening = masked.find("{", start + len(signature))
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated definition {signature!r}")


def _actual_methods():
    text = SOURCE.read_text(encoding="utf-8")
    return {name: _definition(text, signature)
            for name, signature in METHODS.items()}


HOST = r'''
#include <cassert>
#include <cstdint>
#include <vector>

static std::vector<int> events;
struct ItemNode { int id; ItemNode *next; };
class CheckBox;
static int *state_seen_by_add;
static int expected_state_at_add;
static char *expected_text_at_add;

class StringStruct {
public:
    ItemNode *head_;
    ItemNode *current_;
    int current_position_;
    int entry_count_;
    char *pending_text_;
    int field_20_;
    int field_24_;
    int add_result;
    int add_calls;
    int added_id;

    StringStruct()
        : head_(0), current_(0), current_position_(0), entry_count_(0),
          pending_text_(0), field_20_(7), field_24_(9), add_result(0),
          add_calls(0), added_id(-1) {}

    int add(int id) {
        ++add_calls;
        events.push_back(10);
        assert(state_seen_by_add != 0);
        assert(*state_seen_by_add == expected_state_at_add);
        assert(pending_text_ == expected_text_at_add);
        assert(field_20_ == 0 && field_24_ == 0);
        added_id = id;
        return add_result;
    }
};

class Dialog {
public:
    StringStruct item_list_;
    int selected_position_;
    int field_44_;
    int field_64_;
    int field_68_;
    unsigned int row_count_;
    uint32_t field_F0_;
    int redraw_count;

    Dialog() : selected_position_(0), field_44_(1), field_64_(5),
               field_68_(7), row_count_(3), field_F0_(0), redraw_count(0) {}
    void redraw() { ++redraw_count; events.push_back(20); }
};

class CheckBox : public Dialog {
public:
    void UNK1(int pos);
    int UNK2(int pos);
    void set_state_pos(int pos, int state);
    int item(char *text, int id, int state);
    int get_state_id(int id);
    void on_mouse_move(int x_coord, int y_coord);
};

// INSERT_ACTUAL_METHODS

static void test_bit_methods(CheckBox &box) {
    box.selected_position_ = 0;
    box.UNK1(3);
    assert(static_cast<uint32_t>(box.selected_position_) == 8U);
    box.UNK1(3);
    assert(box.selected_position_ == 0);
    box.UNK1(1);
    box.UNK1(5);
    assert(static_cast<uint32_t>(box.selected_position_) == 34U);

    box.set_state_pos(2, 1);
    box.set_state_pos(1, 0);
    assert(static_cast<uint32_t>(box.selected_position_) == 36U);
    box.set_state_pos(5, 0);
    assert(static_cast<uint32_t>(box.selected_position_) == 4U);

    // UNK2 returns the masked word, not a boolean normalization.
    box.selected_position_ = 128;
    assert(box.UNK2(7) == 128);
    assert(box.UNK2(6) == 0);
}

static void test_getter_traversal(CheckBox &box) {
    ItemNode n0{10, 0}, n1{20, 0}, n2{30, 0}, n3{40, 0};
    n0.next = &n1; n1.next = &n2; n2.next = &n3;
    StringStruct &items = box.item_list_;
    items.head_ = &n0;
    items.entry_count_ = 4;
    box.selected_position_ = 8;
    assert(box.get_state_id(40) == 8);
    assert(items.current_position_ == 3 && items.current_ == &n3);

    // A miss walks the declared count and leaves the cursor one past the tail.
    items.current_position_ = 90;
    items.current_ = &n2;
    box.selected_position_ = 16;
    assert(box.get_state_id(99) == 16);
    assert(items.current_position_ == 4 && items.current_ == 0);

    // Count zero takes the early path and preserves a stale, in-range cursor.
    items.entry_count_ = 0;
    items.current_position_ = 2;
    items.current_ = &n1;
    box.selected_position_ = 4;
    assert(box.get_state_id(20) == 4);
    assert(items.current_position_ == 2 && items.current_ == &n1);
}

static void test_item_publication(CheckBox &box) {
    char label_fail[] = "failed-insert";
    char label_ok[] = "successful-insert";
    StringStruct &items = box.item_list_;
    items.entry_count_ = 4;
    items.add_calls = 0;
    items.field_20_ = 8;
    items.field_24_ = 9;
    items.pending_text_ = 0;
    box.selected_position_ = 0;
    state_seen_by_add = &box.selected_position_;
    expected_text_at_add = label_fail;
    expected_state_at_add = 16U;
    items.add_result = 1; // the original expression maps this result to 4
    events.clear();
    assert(box.item(label_fail, 77, 1) == 4);
    assert(items.add_calls == 1 && items.added_id == 77);
    assert(static_cast<uint32_t>(box.selected_position_) == 16U);
    assert((events == std::vector<int>{10}));

    expected_text_at_add = label_ok;
    expected_state_at_add = 0U;
    items.add_result = 0; // success result maps to 0
    events.clear();
    assert(box.item(label_ok, 88, 0) == 0);
    assert(items.add_calls == 2 && items.added_id == 88);
    assert(box.selected_position_ == 0);
    assert((events == std::vector<int>{10}));

    const int prior_calls = items.add_calls;
    items.pending_text_ = label_ok;
    items.field_20_ = 17;
    items.field_24_ = 19;
    box.selected_position_ = 5;
    assert(box.item(0, 99, 1) == 3);
    assert(items.add_calls == prior_calls);
    assert(items.pending_text_ == label_ok);
    assert(items.field_20_ == 17 && items.field_24_ == 19);
    assert(box.selected_position_ == 5);
}

static void test_geometry_hover_and_redraw(CheckBox &box) {
    // Width 6 and height 8; x=6,y=-8 gives signed quotients 1,-1,
    // hence in-range ordinal 2. The selected hover is cleared then redrawn.
    box.selected_position_ = 4;
    box.field_F0_ = 0;
    box.redraw_count = 0;
    events.clear();
    box.on_mouse_move(6, -8);
    assert(box.field_F0_ == 0xFFFFFFFFU);
    assert(box.redraw_count == 1 && (events == std::vector<int>{20}));
    assert(box.selected_position_ == 4);

    // x=-6,y=32 yields ordinal 1 under signed division; unselected hover is
    // updated and redrawn without changing the shared state word.
    box.selected_position_ = 0;
    box.on_mouse_move(-6, 32);
    assert(box.field_F0_ == 1U);
    assert(box.redraw_count == 2);
    assert(box.selected_position_ == 0);

    // Selected position 1 with the already-clear sentinel returns early.
    box.selected_position_ = 2;
    box.field_F0_ = 0xFFFFFFFFU;
    box.on_mouse_move(-6, 32);
    assert(box.field_F0_ == 0xFFFFFFFFU);
    assert(box.redraw_count == 2);
    assert(box.selected_position_ == 2);
}

int main() {
    CheckBox box;
    test_bit_methods(box);
    test_getter_traversal(box);
    test_item_publication(box);
    test_geometry_hover_and_redraw(box);
}
'''


def _compile_and_run(tmp_path, compiler, methods):
    source = tmp_path / "checkbox_state.cpp"
    binary = tmp_path / "checkbox_state"
    joined = "\n\n".join(methods[name] for name in METHODS)
    source.write_text(HOST.replace("// INSERT_ACTUAL_METHODS", joined),
                      encoding="utf-8")
    compile_result = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        cwd=tmp_path, text=True, capture_output=True, timeout=30,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )
    assert compile_result.returncode == 0, compile_result.stderr
    return subprocess.run(
        [str(binary)], cwd=tmp_path, text=True, capture_output=True, timeout=10,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_checkbox_state_methods(tmp_path, compiler):
    result = _compile_and_run(tmp_path, compiler, _actual_methods())
    assert result.returncode == 0, result.stderr


MUTATIONS = (
    ("wrong setter bit",
     "selected_position_ |= 1U << static_cast<uint32_t>(pos);",
     "selected_position_ |= 1U << static_cast<uint32_t>(pos + 1);"),
    ("normalize masked getter",
     "return static_cast<int>(selected_position_ & mask);",
     "return (selected_position_ & mask) != 0;"),
    ("publish text after append",
     "    item_list_.pending_text_ = text;\n"
     "    item_list_.field_20_ = 0;\n"
     "    item_list_.field_24_ = 0;\n"
     "    return -((item_list_.add(id) != 0) ? 1 : 0) & 4;",
     "    item_list_.field_20_ = 0;\n"
     "    item_list_.field_24_ = 0;\n"
     "    const int added = item_list_.add(id);\n"
     "    item_list_.pending_text_ = text;\n"
     "    return -((added != 0) ? 1 : 0) & 4;"),
    ("skip hover redraw",
     "    redraw();", "    /* redraw omitted */"),
)


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("label,before,after", MUTATIONS)
def test_compiling_behavior_mutations_are_rejected(tmp_path, compiler,
                                                   label, before, after):
    methods = _actual_methods()
    owner = {"wrong setter bit": "position_setter",
             "normalize masked getter": "masked_getter",
             "publish text after append": "item",
             "skip hover redraw": "mouse_move"}[label]
    assert methods[owner].count(before) == 1, (label, before)
    methods[owner] = methods[owner].replace(before, after, 1)
    result = _compile_and_run(tmp_path, compiler, methods)
    assert result.returncode != 0, f"compiled mutation escaped: {label}"
