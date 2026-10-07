"""Compile live Console::on_nc_mouse_move update calls in a host vbase fixture."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"
CALL_PREFIX = "GraphicWin::update("


def statements(source):
    start = source.index("void Console::on_nc_mouse_move(int x_coord, int y_coord)")
    end = source.index("\n}", start) + 2
    body = source[start:end]
    found = []
    cursor = 0
    while True:
        call = body.find(CALL_PREFIX, cursor)
        if call < 0:
            break
        opening = body.index("(", call)
        depth = 0
        for index in range(opening, len(body)):
            if body[index] == "(":
                depth += 1
            elif body[index] == ")":
                depth -= 1
            elif body[index] == ";" and depth == 0:
                found.append(body[call:index + 1])
                cursor = index + 1
                break
        else:
            raise AssertionError("unterminated update call")
    assert len(found) == 2, "expected exactly two GraphicWin::update calls"
    assert all("nullptr" in call for call in found)
    return found


def fixture(call_statements):
    return r'''#include <assert.h>
#include <stddef.h>

struct UpdateCall {
    int x, y, width, height;
    void *source;
    const void *receiver;
};

struct GraphicWin {
    GraphicWin() : count(0) {}
    virtual ~GraphicWin() {}
    void update(int x, int y, int width, int height, GraphicWin *source) {
        assert(count < 2);
        calls[count].x = x;
        calls[count].y = y;
        calls[count].width = width;
        calls[count].height = height;
        calls[count].source = source;
        calls[count].receiver = this;
        ++count;
    }
    int count;
    UpdateCall calls[2];
};

struct MapWin : public virtual GraphicWin {
    MapWin() {}
    char map_storage_[73];
};

struct DibHeader { int biWidth, biHeight; };
struct Dib { DibHeader bmiHeader; };
static int ConsoleDragLineX = 27;
static int ConsoleDragLineY = 39;

struct Console : public MapWin {
    Console() : graphic_win_() {
        dib_.bmiHeader.biWidth = 640;
        dib_.bmiHeader.biHeight = -48;
    }
    void invoke_extracted_calls() {
@@CALLS@@
    }
    GraphicWin graphic_win_; // Distinct member, not the virtual base view.
    Dib dib_;
};

static void check_results(const Console &console) {
    const GraphicWin *virtual_base = static_cast<const GraphicWin *>(&console);
    assert(reinterpret_cast<const char *>(virtual_base) !=
           reinterpret_cast<const char *>(&console));
    assert(virtual_base != &console.graphic_win_);
    assert(virtual_base->count == 2);
    const UpdateCall &first = virtual_base->calls[0];
    const UpdateCall &second = virtual_base->calls[1];
    assert(first.receiver == virtual_base);
    assert(first.x == 0 && first.y == ConsoleDragLineY);
    assert(first.width == 640 && first.height == 1);
    assert(first.source == 0);
    assert(second.receiver == virtual_base);
    assert(second.x == ConsoleDragLineX && second.y == 0);
    assert(second.width == 1 && second.height == 48);
    assert(second.source == 0);
    assert(console.graphic_win_.count == 0);
}

int main() {
    Console console;
    console.invoke_extracted_calls();
    check_results(console);
    return 0;
}
'''


def compile_run(compiler, calls, tmp_path):
    source = fixture(calls)
    assert source.count("@@CALLS@@") == 1
    # C++98 has no nullptr keyword; 0 is the same null pointer argument here.
    source = source.replace("@@CALLS@@", "\n".join(calls), 1).replace("nullptr", "0")
    cpp = tmp_path / "console_update_calls.cpp"
    executable = tmp_path / "console_update_calls"
    cpp.write_text(source)
    flags = ["-std=c++98", "-Wall", "-Wextra", "-Werror"]
    if "clang" in Path(compiler).name:
        flags.append("-Wno-reinterpret-base-class")
    built = subprocess.run(
        [compiler] + flags + [str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


def mutants(calls):
    out = {}
    out["complete_object_receiver"] = [
        call.replace("GraphicWin::update(",
                     "reinterpret_cast<GraphicWin *>(this)->update(", 1)
        for call in calls
    ]
    out["distinct_member_receiver"] = [
        call.replace("GraphicWin::update(", "graphic_win_.update(", 1)
        for call in calls
    ]
    out["swapped_calls"] = list(reversed(calls))
    wrong_sign = calls[1].replace(
        "-static_cast<int>(dib_.bmiHeader.biHeight)",
        "static_cast<int>(dib_.bmiHeader.biHeight)", 1,
    )
    assert wrong_sign != calls[1]
    out["wrong_height_sign"] = [calls[0], wrong_sign]
    return out


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_calls_use_console_virtual_graphicwin_base(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_run(compiler, statements(SOURCE.read_text(encoding="utf-8")),
                        tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [
    "complete_object_receiver", "distinct_member_receiver",
    "swapped_calls", "wrong_height_sign",
])
def test_live_calls_reject_wrong_receiver_order_and_sign(
        compiler_name, mutation, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    calls = statements(SOURCE.read_text(encoding="utf-8"))
    result = compile_run(compiler, mutants(calls)[mutation], tmp_path)
    assert result.returncode != 0, mutation + " escaped the caller controls"
