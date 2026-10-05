"""Permanent-test proposal: controls read the integrated product sources only."""

import os
import re
import subprocess
import tempfile
from pathlib import Path


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src" / "win.cpp").is_file() and (parent / "recovery").is_dir():
            return parent
    raise AssertionError("could not locate the OpenSMACX source root")


ROOT = repository_root()


def function_text(text: str, signature: str) -> str:
    match = re.search(re.escape(signature) + r"\s*\{", text)
    assert match, f"missing product-source function: {signature}"
    opening = text.index("{", match.start())
    depth = 0
    for position in range(opening, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start() : position + 1]
    raise AssertionError(f"unterminated product-source function: {signature}")


def win_handlers() -> tuple[str, str]:
    source = (ROOT / "src" / "win.cpp").read_text()
    return (
        function_text(source, "void Win::dispatch_23(int x, int y, int alternate)"),
        function_text(source, "void Win::dispatch_24(int x, int y, int alternate)"),
    )


HOST = r'''#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#define __cdecl

struct Sound {
    explicit Sound(std::vector<std::string> *trace, const char *tag, int active = 0)
        : trace_(trace), tag_(tag), active_(active) {}
    int is_playing() {
        trace_->push_back(tag_ + std::string(".is_playing"));
        return active_;
    }
    int play() {
        trace_->push_back(tag_ + std::string(".play"));
        active_ = 1;
        return 0;
    }
    std::vector<std::string> *trace_;
    std::string tag_;
    int active_;
};

class Win;
static Win *WinCallbackWindow = 0;
static std::vector<std::string> *g_trace = 0;
static void __cdecl record_callback(int x, int y) {
    g_trace->push_back("callback:" + std::to_string(x) + "," + std::to_string(y));
}

class Win {
public:
    Win() : iFlags_(0), iSomeFlag_(0), field_424_(0), field_428_(0),
            val_12_(0), val_13_(0), val_24_(0), val_25_(0) {}
    void dispatch_23(int, int, int);
    void dispatch_24(int, int, int);
    void on_right_down(int x, int y) {
        g_trace->push_back("self23:" + std::to_string(x) + "," + std::to_string(y));
    }
    void vslot_24(int x, int y) {
        g_trace->push_back("self24:" + std::to_string(x) + "," + std::to_string(y));
    }
    void vslot_36(int x, int y) {
        g_trace->push_back("self36:" + std::to_string(x) + "," + std::to_string(y));
    }
    void vslot_37(int x, int y) {
        g_trace->push_back("self37:" + std::to_string(x) + "," + std::to_string(y));
    }
    unsigned int iFlags_;
    unsigned int iSomeFlag_;
    uintptr_t field_424_;
    uintptr_t field_428_;
    void *val_12_;
    void *val_13_;
    void *val_24_;
    void *val_25_;
};

__BODY_23__
__BODY_24__

static uintptr_t callback_bits() {
    return reinterpret_cast<uintptr_t>(&record_callback);
}
static int check(int condition, const char *name) {
    if (!condition) std::fprintf(stderr, "FAIL: %s\n", name);
    return condition ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    std::vector<std::string> trace;
    g_trace = &trace;
    Win win;
    Sound first(&trace, "first");
    Sound second(&trace, "second");
    win.field_424_ = callback_bits();
    win.field_428_ = callback_bits();
    win.val_12_ = &first;
    win.val_13_ = &first;
    win.val_24_ = &second;
    win.val_25_ = &second;
    const std::string test(argv[1]);
    int failures = 0;
    if (test == "route23") {
        win.dispatch_23(11, 22, 0);
        const std::vector<std::string> expected = {
            "callback:11,22", "self23:11,22", "first.is_playing", "first.play"};
        failures += check(trace == expected, "slot23 callback/self/sound order");
        failures += check(WinCallbackWindow == &win, "slot23 callback-window publication");
    } else if (test == "route24") {
        win.dispatch_24(31, 42, 0);
        const std::vector<std::string> expected = {
            "callback:31,42", "self24:31,42", "first.is_playing", "first.play"};
        failures += check(trace == expected, "slot24 callback/self/sound order");
        failures += check(WinCallbackWindow == &win, "slot24 callback-window publication");
    } else if (test == "alternate23") {
        win.dispatch_23(5, 6, 1);
        const std::vector<std::string> expected = {
            "self36:5,6", "second.is_playing", "second.play"};
        failures += check(trace == expected, "slot36 alternate/sound order");
        failures += check(WinCallbackWindow == 0, "alternate route skips publication");
    } else if (test == "alternate24") {
        win.dispatch_24(7, 8, 1);
        const std::vector<std::string> expected = {
            "self37:7,8", "second.is_playing", "second.play"};
        failures += check(trace == expected, "slot37 alternate/sound order");
        failures += check(WinCallbackWindow == 0, "alternate route skips publication");
    } else if (test == "active") {
        first.active_ = 1;
        win.dispatch_24(7, 8, 0);
        const std::vector<std::string> expected = {
            "callback:7,8", "self24:7,8", "first.is_playing"};
        failures += check(trace == expected, "active Sound is not replayed");
    } else if (test == "gates") {
        win.iFlags_ = 0x200000U;
        win.dispatch_23(1, 2, 0);
        failures += check(trace.empty(), "first gate precedes side effects");
        win.iFlags_ = 0;
        win.iSomeFlag_ = 8U;
        win.dispatch_24(1, 2, 0);
        failures += check(trace.empty(), "second gate precedes side effects");
    } else {
        return 3;
    }
    return failures ? 1 : 0;
}
'''


def compile_handler_control(body23: str, body24: str, case: str, expect_success: bool) -> None:
    source = HOST.replace("__BODY_23__", body23).replace("__BODY_24__", body24)
    with tempfile.TemporaryDirectory(prefix="osmx-win-sound-source-") as temp:
        temp_path = Path(temp)
        cpp = temp_path / "control.cpp"
        exe = temp_path / "control"
        cpp.write_text(source)
        subprocess.run(
            [os.environ.get("CXX", "g++"), "-std=c++11", "-Wall", "-Wextra",
             "-Werror", str(cpp), "-o", str(exe)],
            check=True, capture_output=True, text=True,
        )
        result = subprocess.run([str(exe), case], capture_output=True, text=True)
        assert (result.returncode == 0) is expect_success, result.stderr


def test_live_product_handlers_preserve_routes_and_order():
    body23, body24 = win_handlers()
    for case in ("route23", "route24", "alternate23", "alternate24", "active", "gates"):
        compile_handler_control(body23, body24, case, expect_success=True)


def test_live_product_handler_mutants_are_detected():
    body23, body24 = win_handlers()
    mutant_gate = body23.replace("child->is_playing() == 0", "child->is_playing() != 0", 1)
    assert mutant_gate != body23
    compile_handler_control(mutant_gate, body24, "route23", expect_success=False)
    mutant_self = body23.replace("on_right_down(x, y);", "/* omitted native self event */", 1)
    assert mutant_self != body23
    compile_handler_control(mutant_self, body24, "route23", expect_success=False)


def test_four_repurposed_autosound_words_are_neutral_pointer_storage():
    header = (ROOT / "src" / "autosound.h").read_text()
    assert re.search(r"void\s*\*\s*val_12_\s*;", header)
    assert re.search(r"void\s*\*\s*val_13_\s*;", header)
    assert re.search(r"void\s*\*\s*val_24_\s*;", header)
    assert re.search(r"void\s*\*\s*val_25_\s*;", header)
    assert 'static_assert(sizeof(AutoSound) == 0x98' in header


def test_autosound_lifecycle_keeps_the_same_default_and_zero_stores():
    source = (ROOT / "src" / "autosound.cpp").read_text()
    for signature in ("AutoSound::AutoSound()", "void AutoSound::close()",
                      "void AutoSound::close2()"):
        body = function_text(source, signature)
        for field, index in (("val_12_", 11), ("val_13_", 12),
                             ("val_24_", 23), ("val_25_", 24)):
            assert re.search(
                rf"{field}\s*=\s*reinterpret_cast<void\s*\*>\(AutoSoundDefaults\[{index}\]\)",
                body,
            ), f"{signature} must preserve {field} default word {index}"
    init_body = function_text(source, "void AutoSound::init()")
    for field in ("val_12_", "val_13_", "val_24_", "val_25_"):
        assert re.search(rf"{field}\s*=\s*0\s*;", init_body)


def test_flatbutton_pressed_sprite_uses_the_same_neutral_slot_as_pointer():
    source = (ROOT / "src" / "flatbutton.h").read_text()
    setter = function_text(source, "void set_scroll_icons(Sprite *normal, Sprite *hover, Sprite *pressed)")
    assert "val_12_ = reinterpret_cast<void *>(pressed);" in setter
    assert "reinterpret_cast<int>(pressed)" not in setter


def test_product_consumer_closure_does_not_call_win_nullary_slot23():
    for filename in ("win.cpp", "basebutton.cpp", "menu.cpp", "scroll.cpp", "listbox.cpp"):
        source = (ROOT / "src" / filename).read_text()
        assert not re.search(r"\bvslot_23\s*\(\s*\)", source), filename
    win_header = (ROOT / "src" / "win.h").read_text()
    assert re.search(r"virtual\s+void\s+on_right_down\s*\(\s*int\s+x\s*,\s*int\s+y\s*\)", win_header)
