"""Header-owned GraphicWin update callback with its canonical map preserved."""

from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.source import read_file, write_body


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x00404290
CALLBACK = "virtual void on_update() { update(0); }"
UPDATE_ADDRESS = 0x005D56B0
UPDATE_IMPLEMENTATION = """void GraphicWin::update(GraphicWin *) {
    // Four separate field reads of the surface RECT, not a copied RECT: the
    // latter compiles a real `call memcpy` here (unlike redraw()'s own read
    // of the same field, which VC6 happens to inline), where the image loads
    // each of the four dwords directly.
    int height = rect2_.bottom - rect2_.top;
    int width = rect2_.right - rect2_.left;

    if (static_cast<Win *>(this)->is_visible()) {
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        static_cast<Win *>(this)->update_window(&rect);
        static_cast<Win *>(this)->client_to_screen(&rect);
        Win::flip(&rect);
    }
}"""


def mapped_callback():
    records = [row for row in read_file(ROOT / "src/graphicwin.cpp")
               if row.address == ADDRESS]
    assert len(records) == 1
    return records[0]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError(signature)


def test_header_body_keeps_one_canonical_mapping_and_history():
    header = (ROOT / "src/graphicwin.h").read_text(encoding="utf-8")
    cpp_path = ROOT / "src/graphicwin.cpp"
    cpp = cpp_path.read_text(encoding="utf-8")
    record = mapped_callback()

    assert CALLBACK in header
    assert not re.search(r"\bGraphicWin::on_update\s*\(", cpp)
    assert record.body == "src/graphicwin.h"
    assert record.context == ""
    assert record.name == "?update@AlphaMovie@@QAEXXZ"
    assert record.symbol == "?on_update@GraphicWin@@UAEXXZ"
    assert record.image_spans == ((ADDRESS, 0x00404298),)
    assert record.byte_exact

    raw = cpp_path.read_bytes()
    assert write_body(raw, record, "src/graphicwin.h") == raw


def test_split_update_keeps_original_mapping_body_and_source_boundary():
    header = (ROOT / "src/graphicwin.h").read_text(encoding="utf-8")
    cpp = (ROOT / "src/graphicwin.cpp").read_text(encoding="utf-8")
    update_path = ROOT / "src/graphicwin_update.cpp"
    update_cpp = update_path.read_text(encoding="utf-8")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    assert CALLBACK in header
    assert not re.search(r"^\s*void GraphicWin::on_update\s*\(", cpp, re.M)
    assert not re.search(r"^\s*void GraphicWin::on_update\s*\(", update_cpp, re.M)
    assert not re.search(r"^\s*void GraphicWin::update\s*\(\s*GraphicWin\s*\*", cpp, re.M)
    assert braced(update_cpp, "void GraphicWin::update(GraphicWin *)") == UPDATE_IMPLEMENTATION
    assert len(re.findall(r"^\s*src/graphicwin_update\.cpp\s*$", cmake, re.M)) == 1

    update_records = [row for row in read_file(update_path)
                      if row.address == UPDATE_ADDRESS]
    old_tu_records = [row for row in read_file(ROOT / "src/graphicwin.cpp")
                      if row.address == UPDATE_ADDRESS]
    assert len(update_records) == 1 and not old_tu_records
    record = update_records[0]
    assert record.name == "?update@GraphicWin@@QAEXPAUGraphicWin@@@Z"
    assert record.symbol == "?update@GraphicWin@@QAEXPAV1@@Z"
    assert record.image_spans == ((UPDATE_ADDRESS, 0x005D571D),)
    assert record.byte_exact

    # The source boundary, not a compiler switch or method attribute, keeps
    # the implementation invisible to TUs instantiating the inline callback.
    implementation_sources = (header, cpp, update_cpp)
    workarounds = (r"__declspec\s*\(\s*noinline\s*\)",
                   r"__attribute__\s*\(\(\s*noinline\s*\)\)",
                   r"#\s*pragma\s+optimize")
    assert not any(re.search(pattern, text, re.I)
                   for pattern in workarounds for text in implementation_sources)
    assert not re.search(r"set_source_files_properties\s*\([^)]*graphicwin_update\.cpp",
                         cmake, re.S)


def compile_and_run(tmp_path, callback, *, name):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        return None
    source = f"""
class GraphicWin;
class Win {{ public: virtual void on_update() {{}} }};
class GraphicWin : public Win {{
public:
    void update(GraphicWin *target);
    {callback}
}};
static int calls;
static GraphicWin *observed;
void GraphicWin::update(GraphicWin *target) {{ ++calls; observed = target; }}
int main() {{
    GraphicWin window;
    Win *receiver = &window;
    receiver->on_update();
    return calls == 1 && observed == 0 ? 0 : 1;
}}
"""
    path = tmp_path / f"{name}.cpp"
    path.write_text(source)
    executable = tmp_path / name
    built = subprocess.run([compiler, "-std=c++98", "-O0", str(path), "-o",
                            str(executable)], capture_output=True, text=True,
                           timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


def test_actual_header_callback_delegates_null_and_mutations_fail(tmp_path):
    header = (ROOT / "src/graphicwin.h").read_text(encoding="utf-8")
    match = re.search(r"^\s*(virtual void on_update\(\) \{ update\(0\); \})$",
                      header, re.M)
    assert match is not None

    result = compile_and_run(tmp_path, match.group(1), name="actual")
    if result is None:
        return
    assert result.returncode == 0, result.stderr

    wrong_receiver = compile_and_run(
        tmp_path, match.group(1).replace("update(0)", "update(this)"),
        name="wrong_receiver")
    assert wrong_receiver is not None and wrong_receiver.returncode != 0

    base_default = compile_and_run(
        tmp_path, match.group(1).replace("update(0)", "Win::on_update()"),
        name="base_default")
    assert base_default is not None and base_default.returncode != 0


def test_actual_split_update_visibility_and_rectangle_calls(tmp_path):
    update_cpp = (ROOT / "src/graphicwin_update.cpp").read_text(encoding="utf-8")
    body = braced(update_cpp, "void GraphicWin::update(GraphicWin *)")
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        return

    header = r"""
struct RECT { int left, top, right, bottom; };
class Win {
public:
    bool visible;
    int visibility_checks;
    Win() : visible(false), visibility_checks(0) {}
    virtual void on_update() {}
    int is_visible() { ++visibility_checks; return visible; }
    void update_window(RECT *rect);
    void client_to_screen(RECT *rect);
    static void flip(RECT *rect);
};
class Buffer { public: RECT rect2_; };
class GraphicWin : public Win, public Buffer {
public:
    void update(GraphicWin *target);
    virtual void on_update() { update(0); }
};
"""
    implementation = '#include "fixture.h"\n' + body + "\n"
    main = r"""
#include "fixture.h"
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
    receiver->on_update();
    if (window.visibility_checks != 2 || call_count != 3) return 2;
    if (calls[0] != 1 || calls[1] != 2 || calls[2] != 3) return 3;
    if (updated.left != 0 || updated.top != 0 || updated.right != 76 || updated.bottom != 81) return 4;
    if (converted.left != 0 || converted.top != 0 || converted.right != 76 || converted.bottom != 81) return 5;
    if (flipped.left != 20 || flipped.top != 30 || flipped.right != 96 || flipped.bottom != 111) return 6;
    return 0;
}
"""
    header_path = tmp_path / "fixture.h"
    impl_path = tmp_path / "update_impl.cpp"
    main_path = tmp_path / "main.cpp"
    executable = tmp_path / "split_update"
    header_path.write_text(header)
    impl_path.write_text(implementation)
    main_path.write_text(main)
    built = subprocess.run([compiler, "-std=c++98", "-O0", str(impl_path),
                            str(main_path), "-o", str(executable)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=10)
    assert result.returncode == 0, result.stderr
