"""Actual-source controls for Console::close owned subobjects and call order."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location(
    "console_dispatch_host",
    ROOT / "recovery/tests/test_console_key_click_dispatch_host.py",
)
console_host = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = console_host
spec.loader.exec_module(console_host)


def _actual_console_close():
    declarations = (ROOT / "src/console.h").read_text()
    for declaration in ("Buffer buffer_;", "Time time_;", "AlphaMenu alpha_menu_;"):
        assert declaration in declarations
    assert "class AlphaMenu : public Menu {" in (ROOT / "src/alphamenu.h").read_text()
    assert "class Menu : public GraphicWin {" in (ROOT / "src/menu.h").read_text()
    source = (ROOT / "src/console.cpp").read_text()
    body = console_host._block(source, "void Console::close()")
    assert "reinterpret_cast" not in body
    assert "buffer_.close();" in body
    assert "static_cast<GraphicWin &>(alpha_menu_).close();" in body
    return body


def _harness(body):
    return r'''
#include <assert.h>
#include <stdint.h>

int events[8], event_count;
const void *buffer_receiver;
const void *time_receiver;
const void *graphic_receiver;
const void *clear_receiver;

struct Buffer {
    void close() { events[event_count++] = 1; buffer_receiver = this; }
};
struct Time {
    void close() { events[event_count++] = 2; time_receiver = this; }
};
struct GraphicWin {
    uint32_t close() { events[event_count++] = 3; graphic_receiver = this; return 0; }
};
struct Menu : public GraphicWin {};
struct AlphaMenu : public Menu {};

struct Console {
    uint32_t field_23BC8_;
    uint32_t cursor_active_;
    uint32_t field_23BE4_, field_23BE8_, field_23BF0_, field_23BF4_;
    uint32_t survey_mode_, field_23C04_, field_23BFC_, field_23BF8_;
    int32_t cursor_history_index_;
    int32_t cursor_history_x_[32], cursor_history_y_[32];
    uint32_t field_23D10_, field_23D18_, field_23D88_, field_23D80_;
    uint32_t field_23D8C_, field_23D84_;
    GraphicWin graphic_win_;
    Buffer buffer_;
    Time time_;
    AlphaMenu alpha_menu_;

    void clear(int value) {
        assert(value == 0);
        events[event_count++] = 4;
        clear_receiver = this;
    }
BODY
};

int main() {
    Console subject = {};
    subject.field_23BC8_ = 8;
    subject.cursor_active_ = subject.field_23BE4_ = subject.field_23BE8_ = 1;
    subject.field_23BF0_ = subject.field_23BF4_ = subject.survey_mode_ = 1;
    subject.field_23C04_ = subject.field_23BFC_ = subject.field_23BF8_ = 1;
    subject.field_23D10_ = subject.field_23D18_ = subject.field_23D88_ = 1;
    subject.field_23D80_ = subject.field_23D8C_ = subject.field_23D84_ = 1;
    for (int i = 0; i != 32; ++i) {
        subject.cursor_history_x_[i] = 100 + i;
        subject.cursor_history_y_[i] = 200 + i;
    }
    subject.cursor_history_index_ = 0;

    subject.close();

    assert(subject.field_23BC8_ == 0xffffffffu);
    assert(subject.cursor_active_ == 0 && subject.field_23BE4_ == 0);
    assert(subject.field_23BE8_ == 0 && subject.field_23BF0_ == 0);
    assert(subject.field_23BF4_ == 0 && subject.survey_mode_ == 0);
    assert(subject.field_23C04_ == 0 && subject.field_23BFC_ == 0);
    assert(subject.field_23BF8_ == 0 && subject.cursor_history_index_ == 0);
    assert(subject.cursor_history_x_[0] == 0 && subject.cursor_history_y_[0] == 0);
    for (int i = 1; i != 32; ++i) {
        assert(subject.cursor_history_x_[i] == 100 + i);
        assert(subject.cursor_history_y_[i] == 200 + i);
    }
    assert(subject.field_23D10_ == 0 && subject.field_23D18_ == 0);
    assert(subject.field_23D88_ == 0 && subject.field_23D80_ == 0);
    assert(subject.field_23D8C_ == 0 && subject.field_23D84_ == 0);
    assert(event_count == 4);
    assert(events[0] == 1 && events[1] == 2 && events[2] == 3 && events[3] == 4);
    assert(buffer_receiver == &subject.buffer_);
    assert(time_receiver == &subject.time_);
    assert(graphic_receiver == static_cast<const void *>(&subject.alpha_menu_));
    assert(clear_receiver == &subject);
    return 0;
}
'''.replace("BODY", body.replace("void Console::close()", "void close()", 1))


def _compile_run(tmp_path, compiler, source, name, sanitizer=False):
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source)
    command = [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror"]
    if sanitizer:
        command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    command += [str(cpp), "-o", str(exe)]
    built = subprocess.run(command, text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    environment = os.environ.copy()
    environment.setdefault("ASAN_OPTIONS", "detect_leaks=0")
    return subprocess.run([str(exe)], text=True, capture_output=True, env=environment)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_console_close_actual_body_members_order_and_resets(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_run(tmp_path, compiler, _harness(_actual_console_close()),
                          compiler_name.replace("+", "p"), sanitizer=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_console_close_wrong_base_receiver_is_detected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = _actual_console_close().replace(
        "static_cast<GraphicWin &>(alpha_menu_).close();",
        "graphic_win_.close();",
        1,
    )
    assert body != _actual_console_close()
    result = _compile_run(tmp_path, compiler, _harness(body),
                          compiler_name.replace("+", "p") + "-wrong-receiver")
    assert result.returncode != 0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_console_close_changed_order_is_detected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = _actual_console_close().replace(
        "buffer_.close();\n    time_.close();",
        "time_.close();\n    buffer_.close();",
        1,
    )
    assert body != _actual_console_close()
    result = _compile_run(tmp_path, compiler, _harness(body),
                          compiler_name.replace("+", "p") + "-wrong-order")
    assert result.returncode != 0
