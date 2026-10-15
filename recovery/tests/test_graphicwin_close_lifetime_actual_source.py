"""Exercise GraphicWin close side effects against a focused C++98 host owner."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/graphicwin.cpp"


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated definition: " + signature)


def live_definitions():
    source = SOURCE.read_text(encoding="utf-8")
    close = definition(source, "void GraphicWin::close()")
    caption_result = definition(
        source, "uint32_t GraphicWin::field_A0C_value() const")
    return close, caption_result


def build_fixture(close, caption_result):
    return r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// uintptr_t preserves the independently tested field_A0C_value getter's
// pointer-sized representation in this host control; this tests neither the
// product ABI nor 32-bit layout.
#define uint32_t uintptr_t

struct GraphicWin;
struct Buffer;
typedef int (*GraphicWinCaptionDraw)(GraphicWin *, char *, Buffer *);
static int default_caption_draw(GraphicWin *, char *, Buffer *) { return 17; }
static GraphicWinCaptionDraw GraphicWinFieldA0CDefault = &default_caption_draw;

static char events[16];
static int event_count;
static int scalar_delete_count;
static bool mutate_owner_during_close;
static bool owner_visible_during_dtor;
static bool reset_state_visible_during_dtor;
static GraphicWin *active_window;
static Buffer *win_close_replacement;
static Buffer *buffer_close_replacement;
static Buffer *dtor_close_replacement;

void operator delete(void *memory) throw() {
    ++scalar_delete_count;
    free(memory);
}
void operator delete[](void *memory) throw() {
    ++scalar_delete_count;
    free(memory);
}

struct Win {
    void close();
    virtual ~Win() {}
    int field_134_;
    int field_138_;
};

struct Buffer {
    void close();
    virtual ~Buffer() { events[event_count++] = 'b'; }
};

struct GraphicWin : Win, Buffer {
    GraphicWin();
    void close();
    uintptr_t field_A0C_value() const;

    void (*field_A10_)();
    void *frame_top_left_;
    void *frame_bottom_left_;
    void *frame_top_right_;
    void *frame_bottom_right_;
    void *frame_left_;
    void *frame_right_;
    void *frame_bottom_;
    void *frame_top_;
    void *frame_noncaption_left_;
    void *frame_noncaption_right_;
    void *frame_noncaption_middle_;
    void *overlay_top_left_;
    void *overlay_bottom_left_;
    void *overlay_top_right_;
    void *overlay_bottom_right_;
    Buffer *owned_buffer_;
    GraphicWinCaptionDraw field_A0C_;
};

static void paint_hook() {}
static int alternate_caption_draw(GraphicWin *, char *, Buffer *) { return 23; }
GraphicWin::GraphicWin()
    : field_A10_(&paint_hook), frame_top_left_((void *)1),
      frame_bottom_left_((void *)1), frame_top_right_((void *)1),
      frame_bottom_right_((void *)1), frame_left_((void *)1),
      frame_right_((void *)1), frame_bottom_((void *)1), frame_top_((void *)1),
      frame_noncaption_left_((void *)1), frame_noncaption_right_((void *)1),
      frame_noncaption_middle_((void *)1), overlay_top_left_((void *)1),
      overlay_bottom_left_((void *)1), overlay_top_right_((void *)1),
      overlay_bottom_right_((void *)1), owned_buffer_(0),
      field_A0C_(&default_caption_draw) {
    field_134_ = 3;
    field_138_ = 4;
}

void Win::close() {
    events[event_count++] = 'W';
    if (mutate_owner_during_close)
        active_window->owned_buffer_ = win_close_replacement;
}
void Buffer::close() {
    events[event_count++] = 'B';
    if (mutate_owner_during_close)
        active_window->owned_buffer_ = buffer_close_replacement;
}

class TrackedBuffer : public Buffer {
public:
    explicit TrackedBuffer(int id) : id_(id) {}
    virtual ~TrackedBuffer() {
        events[event_count++] = (char)('0' + id_);
        if (active_window != 0 && active_window->owned_buffer_ == this) {
            owner_visible_during_dtor = true;
            active_window->owned_buffer_ = dtor_close_replacement;
        }
        reset_state_visible_during_dtor =
            active_window != 0 && active_window->field_A10_ == 0
            && active_window->field_134_ == 0
            && active_window->field_138_ == 0
            && active_window->frame_top_left_ == 0
            && active_window->overlay_bottom_right_ == 0
            && active_window->field_A0C_ == GraphicWinFieldA0CDefault;
    }
private:
    int id_;
};

''' + close + "\n\n" + caption_result + r'''

static void assert_reset(const GraphicWin &window) {
    assert(window.field_A10_ == 0);
    assert(window.field_134_ == 0 && window.field_138_ == 0);
    assert(window.frame_top_left_ == 0 && window.frame_bottom_left_ == 0);
    assert(window.frame_top_right_ == 0 && window.frame_bottom_right_ == 0);
    assert(window.frame_left_ == 0 && window.frame_right_ == 0);
    assert(window.frame_bottom_ == 0 && window.frame_top_ == 0);
    assert(window.frame_noncaption_left_ == 0);
    assert(window.frame_noncaption_right_ == 0);
    assert(window.frame_noncaption_middle_ == 0);
    assert(window.overlay_top_left_ == 0 && window.overlay_bottom_left_ == 0);
    assert(window.overlay_top_right_ == 0 && window.overlay_bottom_right_ == 0);
    assert(window.field_A0C_ == GraphicWinFieldA0CDefault);
}

int main() {
    GraphicWin window;
    active_window = &window;
    TrackedBuffer *initial = new TrackedBuffer(1);
    TrackedBuffer *win_replacement = new TrackedBuffer(2);
    TrackedBuffer *buffer_replacement = new TrackedBuffer(3);
    TrackedBuffer *dtor_replacement = new TrackedBuffer(4);
    window.owned_buffer_ = initial;
    win_close_replacement = win_replacement;
    buffer_close_replacement = buffer_replacement;
    dtor_close_replacement = dtor_replacement;
    mutate_owner_during_close = true;
    scalar_delete_count = 0;

    window.close();
    assert(event_count == 4);
    assert(events[0] == 'W' && events[1] == 'B');
    assert(events[2] == '3' && events[3] == 'b');
    assert(owner_visible_during_dtor);
    assert(reset_state_visible_during_dtor);
    assert(scalar_delete_count == 1);
    assert(window.owned_buffer_ == 0);
    assert_reset(window);

    // With no separately owned Buffer, reset still runs and the independent
    // field_A0C_value getter still exposes the default callback's stored value.
    event_count = 0;
    mutate_owner_during_close = false;
    window.field_A0C_ = &alternate_caption_draw;
    window.close();
    assert(event_count == 2 && events[0] == 'W' && events[1] == 'B');
    assert(window.field_A0C_value() == reinterpret_cast<uintptr_t>(GraphicWinFieldA0CDefault));
    assert(window.owned_buffer_ == 0);
    assert_reset(window);
    assert(scalar_delete_count == 1);

    active_window = 0;
    delete initial;
    delete win_replacement;
    delete dtor_replacement;
    return 0;
}
'''


def mutations(close):
    cases = {}

    before = "    delete release_target;"
    after = "    release_target->Buffer::~Buffer();"
    cases["qualified_destructor"] = close.replace(before, after, 1)

    before = "    delete release_target;\n    owned_buffer_ = 0;"
    after = "    owned_buffer_ = 0;\n    delete release_target;"
    cases["early_clear"] = close.replace(before, after, 1)

    before = "    Win::close();\n    Buffer::close();"
    after = "    Buffer::close();\n    Win::close();"
    cases["wrong_base_order"] = close.replace(before, after, 1)

    before = "    Win::close();\n    Buffer::close();"
    after = "    Buffer *const release_target = owned_buffer_;\n"
    after += "    Win::close();\n    Buffer::close();"
    cached = close.replace(before, after, 1)
    before = "    Buffer *const release_target = owned_buffer_;\n    field_A10_ = 0;"
    after = "    field_A10_ = 0;"
    cases["stale_owner_capture"] = cached.replace(before, after, 1)

    for name, mutated in cases.items():
        assert mutated != close, "failed to apply mutant " + name
    return cases


def run_control(compiler, close, caption_result, tmp_path):
    cpp = tmp_path / "graphicwin_close.cpp"
    executable = tmp_path / "graphicwin_close"
    cpp.write_text(build_fixture(close, caption_result))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_graphicwin_close_full_lifetime(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    close, caption_result = live_definitions()
    result = run_control(compiler, close, caption_result, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [
    "qualified_destructor", "early_clear", "wrong_base_order",
    "stale_owner_capture",
])
def test_live_graphicwin_close_rejects_lifetime_mutations(
        compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    close, caption_result = live_definitions()
    result = run_control(compiler, mutations(close)[mutation],
                         caption_result, tmp_path)
    assert result.returncode != 0, mutation + " escaped the lifetime control"
