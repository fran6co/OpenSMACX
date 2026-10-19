"""Header-owned GraphicWin update callback with its canonical map preserved."""

from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.source import read_file, write_body


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x00404290
CALLBACK = "virtual void on_update() { update(0); }"


def mapped_callback():
    records = [row for row in read_file(ROOT / "src/graphicwin.cpp")
               if row.address == ADDRESS]
    assert len(records) == 1
    return records[0]


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
