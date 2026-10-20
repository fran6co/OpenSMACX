"""Actual-source controls for GraphicWin's inherited surface callback."""

from pathlib import Path
import re
import shutil
import subprocess
import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
CALLBACK_ADDRESS = 0x00404290
UPDATE_ADDRESS = 0x005D56B0


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError(signature)


def record_for(path, address):
    rows = [row for row in read_file(path) if row.address == address]
    assert len(rows) == 1
    return rows[0]


def test_actual_callback_and_callee_keep_canonical_mappings_and_tu_boundary():
    header = (ROOT / "src/graphicwin.h").read_text(encoding="utf-8")
    events_path = ROOT / "src/graphicwin_events.cpp"
    events = events_path.read_text(encoding="utf-8")
    cpp_path = ROOT / "src/graphicwin.cpp"
    cpp = cpp_path.read_text(encoding="utf-8")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    callback = record_for(events_path, CALLBACK_ADDRESS)
    update = record_for(cpp_path, UPDATE_ADDRESS)
    assert callback.name == "?update@AlphaMovie@@QAEXXZ"
    assert callback.symbol == "?on_update@GraphicWin@@UAEXXZ"
    assert callback.image_spans == ((CALLBACK_ADDRESS, 0x00404298),)
    assert callback.byte_exact
    assert update.name == "?update@GraphicWin@@QAEXPAUGraphicWin@@@Z"
    assert update.symbol == "?update@GraphicWin@@QAEXPAV1@@Z"
    assert update.image_spans == ((UPDATE_ADDRESS, 0x005D571D),)
    assert update.byte_exact

    assert re.search(r"^\s*virtual void on_update\(\);\s*$", header, re.M)
    callback_body = braced(events, "void GraphicWin::on_update()")
    assert re.sub(r"\s+", " ", callback_body) == \
        "void GraphicWin::on_update() { update(0); }"
    braced(cpp, "void GraphicWin::update(GraphicWin *)")
    assert "void GraphicWin::update(GraphicWin *)" not in events
    assert "src/graphicwin.cpp" in cmake
    assert "src/graphicwin_events.cpp" in cmake

    # Each original identity has exactly one canonical current source record.
    all_sources = (events_path, cpp_path)
    assert sum(row.address == CALLBACK_ADDRESS
               for path in all_sources for row in read_file(path)) == 1
    assert sum(row.address == UPDATE_ADDRESS
               for path in all_sources for row in read_file(path)) == 1

    implementation = (header, events, cpp)
    workarounds = (r"__declspec\s*\(\s*noinline\s*\)",
                   r"__attribute__\s*\(\(\s*noinline\s*\)\)",
                   r"#\s*pragma\s+optimize")
    assert not any(re.search(pattern, text, re.I)
                   for pattern in workarounds for text in implementation)
    assert "graphicwin_update.cpp" not in cmake


def fixture_header():
    return r"""struct RECT { int left, top, right, bottom; };
class Win {
public:
    bool visible;
    int visibility_checks;
    Win() : visible(false), visibility_checks(0) {}
    virtual void on_update() {}
    virtual void visibility_probe() {}
    int is_visible() { ++visibility_checks; visibility_probe(); return visible; }
    void update_window(RECT *rect);
    void client_to_screen(RECT *rect);
    static void flip(RECT *rect);
};
class Buffer { public: RECT rect2_; };
class GraphicWin : public Win, public Buffer {
public:
    void update(GraphicWin *target);
    virtual void on_update();
    virtual void visibility_probe() {
        rect2_.right = rect2_.left + 20;
        rect2_.bottom = rect2_.top + 25;
    }
};
"""


def compile_and_run(tmp_path, *, callback, update, main, name):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    header_path = tmp_path / "fixture.h"
    callback_path = tmp_path / "callback.cpp"
    update_path = tmp_path / "update.cpp"
    main_path = tmp_path / "main.cpp"
    executable = tmp_path / name
    header_path.write_text(fixture_header())
    callback_path.write_text('#include "fixture.h"\n' + callback + "\n")
    update_path.write_text('#include "fixture.h"\n' + update + "\n")
    main_path.write_text('#include "fixture.h"\n' + main + "\n")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(callback_path), str(update_path),
         str(main_path), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


def source_bodies():
    events = (ROOT / "src/graphicwin_events.cpp").read_text(encoding="utf-8")
    cpp = (ROOT / "src/graphicwin.cpp").read_text(encoding="utf-8")
    return (braced(events, "void GraphicWin::on_update()"),
            braced(cpp, "void GraphicWin::update(GraphicWin *)"))


CALLBACK_FIXTURE_MAIN = r"""
extern int update_calls;
extern GraphicWin *observed_target;
int main() {
    GraphicWin window;
    Win *receiver = &window;
    receiver->on_update();
    return update_calls == 1 && observed_target == 0 ? 0 : 1;
}
"""


def test_actual_virtual_callback_delegates_through_graphicwin(tmp_path):
    callback, _ = source_bodies()
    update_stub = r"""
int update_calls;
GraphicWin *observed_target;
void GraphicWin::update(GraphicWin *target) {
    ++update_calls;
    observed_target = target;
}
"""
    main = CALLBACK_FIXTURE_MAIN
    result = compile_and_run(tmp_path, callback=callback, update=update_stub,
                             main=main, name="actual_callback")
    assert result.returncode == 0, result.stderr

    wrong_receiver = compile_and_run(
        tmp_path, callback=callback.replace("update(0)", "update(this)"),
        update=update_stub, main=main, name="wrong_receiver")
    assert wrong_receiver is not None and wrong_receiver.returncode != 0

    base_default = compile_and_run(
        tmp_path, callback=callback.replace("update(0)", "Win::on_update()"),
        update=update_stub, main=main, name="base_default")
    assert base_default is not None and base_default.returncode != 0


def test_actual_callee_visibility_and_rectangle_mutation_order(tmp_path):
    callback, update = source_bodies()
    main = r"""
static int calls[3], call_count;
static RECT updated, converted, flipped;
void Win::update_window(RECT *rect) { calls[call_count++] = 1; updated = *rect; }
void Win::client_to_screen(RECT *rect) {
    calls[call_count++] = 2; converted = *rect;
    rect->left += 20; rect->right += 20; rect->top += 30; rect->bottom += 30;
}
void Win::flip(RECT *rect) { calls[call_count++] = 3; flipped = *rect; }
int main() {
    GraphicWin window;
    window.rect2_.left = 11; window.rect2_.top = 17;
    window.rect2_.right = 87; window.rect2_.bottom = 98;
    Win *receiver = &window;
    receiver->on_update();
    if (window.visibility_checks != 1 || call_count != 0) return 1;
    window.visible = true;
    window.rect2_.left = 11; window.rect2_.top = 17;
    window.rect2_.right = 87; window.rect2_.bottom = 98;
    receiver->on_update();
    if (window.visibility_checks != 2 || call_count != 3) return 2;
    if (calls[0] != 1 || calls[1] != 2 || calls[2] != 3) return 3;
    if (updated.left != 0 || updated.top != 0 || updated.right != 76 || updated.bottom != 81) return 4;
    if (converted.left != 0 || converted.top != 0 || converted.right != 76 || converted.bottom != 81) return 5;
    if (flipped.left != 20 || flipped.top != 30 || flipped.right != 96 || flipped.bottom != 111) return 6;
    return 0;
}
"""
    result = compile_and_run(tmp_path, callback=callback, update=update,
                             main=main, name="actual_update")
    assert result.returncode == 0, result.stderr

    pre_visibility_reads = (
        "int height = rect2_.bottom - rect2_.top;\n"
        "    int width = rect2_.right - rect2_.left;\n\n"
        "    if (static_cast<Win *>(this)->is_visible()) {")
    delayed_reads = (
        "if (static_cast<Win *>(this)->is_visible()) {\n"
        "        int height = rect2_.bottom - rect2_.top;\n"
        "        int width = rect2_.right - rect2_.left;")
    delayed_mutant = update.replace(pre_visibility_reads, delayed_reads, 1)
    assert delayed_mutant != update
    delayed_result = compile_and_run(
        tmp_path, callback=callback, update=delayed_mutant,
        main=main, name="delayed_dimension_reads")
    assert delayed_result.returncode != 0
