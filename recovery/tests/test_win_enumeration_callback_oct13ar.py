"""Typed EnumThreadWindows callback behavior from actual Win source bodies.

Host SDK-shaped function pointers and Win/Menu doubles exercise the callback
contract only; they are not a native 32-bit ABI, complete Win layout, or game
runtime proof.
"""

from dataclasses import fields
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "60f4793915344e356389f03e8abfd158eb3765f8"
WIN_CPP = ROOT / "src/win.cpp"
WIN_H = ROOT / "src/win.h"
UPDATE_SIGNATURE = "void Win::update_menus()"
CALLBACK_SIGNATURE = "int __stdcall Win::adjust_menus(HWND hwnd, LPARAM unused)"
OLD_DECL = "static int __stdcall adjust_menus(HWND hwnd, void *unused);"
NEW_DECL = "static int __stdcall adjust_menus(HWND hwnd, LPARAM unused);"
OLD_CALLBACK = "int __stdcall Win::adjust_menus(HWND hwnd, void *unused) {"
NEW_CALLBACK = "int __stdcall Win::adjust_menus(HWND hwnd, LPARAM unused) {"
OLD_SYMBOL = "// symbol    ?adjust_menus@Win@@SGHPAUHWND__@@PAX@Z"
NEW_SYMBOL = "// symbol    ?adjust_menus@Win@@SGHPAUHWND__@@J@Z"
OLD_ENUM_CALL = (
    "    EnumThreadWindows(GetCurrentThreadId(),\n"
    "        reinterpret_cast<WNDENUMPROC>(Win::adjust_menus), 0);"
)
NEW_ENUM_CALL = "    EnumThreadWindows(GetCurrentThreadId(), Win::adjust_menus, 0);"


def _baseline(path: Path) -> str:
    relative = path.relative_to(ROOT).as_posix()
    result = subprocess.run(
        ["git", "show", f"{BASE}:{relative}"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _extract(text: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated source function: {signature}")


def _protected_history(row):
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines)) for lesson in row.origin)


def _protected_metadata(row):
    return tuple((item.name, getattr(row, item.name)) for item in fields(row)
                 if item.name not in {"line", "region", "origin", "symbol"})


def test_callback_and_enumerator_regions_preserve_contract_and_mapping_history():
    old_cpp = _baseline(WIN_CPP)
    old_header = _baseline(WIN_H)
    expected_update = _extract(old_cpp, UPDATE_SIGNATURE)
    expected_callback = _extract(old_cpp, OLD_CALLBACK.removesuffix(" {"))
    for before, after in ((OLD_ENUM_CALL, NEW_ENUM_CALL),
                          (OLD_CALLBACK, NEW_CALLBACK)):
        if before == OLD_ENUM_CALL:
            assert expected_update.count(before) == 1, before
            expected_update = expected_update.replace(before, after, 1)
        else:
            assert expected_callback.count(before) == 1, before
            expected_callback = expected_callback.replace(before, after, 1)
    current_cpp = WIN_CPP.read_text(encoding="utf-8")
    current_header = WIN_H.read_text(encoding="utf-8")
    assert _extract(current_cpp, UPDATE_SIGNATURE) == expected_update
    assert _extract(current_cpp, CALLBACK_SIGNATURE) == expected_callback
    assert OLD_SYMBOL not in current_cpp
    assert current_cpp.count(NEW_SYMBOL) == 1
    assert OLD_DECL not in current_header
    assert current_header.count(NEW_DECL) == 1
    assert old_header.count(OLD_DECL) == 1

    before_rows = {row.address: row for row in read_text(old_cpp, WIN_CPP)}
    after_rows = {row.address: row for row in read_text(current_cpp, WIN_CPP)}
    assert set(after_rows) == set(before_rows)
    for address, old in before_rows.items():
        current = after_rows[address]
        assert _protected_metadata(old) == _protected_metadata(current)
        assert _protected_history(old) == _protected_history(current)
        expected_symbol = (NEW_SYMBOL.removeprefix("// symbol    ")
                           if address == 0x5F0540 else old.symbol)
        assert current.symbol == expected_symbol


HOST_PREFIX = r"""
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#define __stdcall
#define CALLBACK __stdcall
struct HWND__ { int tag; };
typedef HWND__ *HWND;
typedef long LPARAM;
typedef int BOOL;
typedef unsigned long DWORD;
typedef BOOL (CALLBACK *WNDENUMPROC)(HWND, LPARAM);

static int events[12];
static int event_count;
static void record(int event) {
    if (event_count >= 12) {
        std::fprintf(stderr, "CONTROL_FAIL:event-overflow\n");
        std::exit(98);
    }
    events[event_count++] = event;
}
static int control_failure(const char *name) {
    std::fprintf(stderr, "CONTROL_FAIL:%s\n", name);
    return 98;
}
static HWND expected_hwnd;
static long user_data;
static long last_userdata_value;
static int last_userdata_index;
static HWND last_userdata_hwnd;
static WNDENUMPROC observed_callback;
static LPARAM observed_context;
static int observed_callback_result;

class Menu {
public:
    void adjust() { record(5); }
};
class Win {
public:
    Menu *menu_;
    void set_rects() { record(4); }
    static void update_menus();
    static int __stdcall adjust_menus(HWND hwnd, LPARAM unused);
};

static DWORD GetCurrentThreadId() {
    record(1);
    return 37;
}
static long GetWindowLongA(HWND hwnd, int index) {
    record(3);
    last_userdata_hwnd = hwnd;
    last_userdata_index = index;
    last_userdata_value = user_data;
    return user_data;
}
static BOOL EnumThreadWindows(DWORD thread, WNDENUMPROC callback, LPARAM context) {
    if (thread != 37 || callback != &Win::adjust_menus || context != 0)
        std::exit(control_failure("enumeration-typed-callback-and-context"));
    record(2);
    observed_callback = callback;
    observed_context = context;
    observed_callback_result = callback(expected_hwnd, context);
    return 1;
}
"""


def _host_source(callback: str, update: str, mutation: str | None = None) -> str:
    if mutation == "wrong_userdata_index":
        old = "GetWindowLongA(hwnd, -0x15)"
        assert callback.count(old) == 1
        callback = callback.replace(old, "GetWindowLongA(hwnd, -0x14)", 1)
    elif mutation == "drop_set_rects":
        old = "obj->set_rects();"
        assert callback.count(old) == 1
        callback = callback.replace(old, "/* omitted */", 1)
    elif mutation == "drop_menu_adjust":
        old = "menu->adjust();"
        assert callback.count(old) == 1
        callback = callback.replace(old, "/* omitted */", 1)
    elif mutation == "wrong_callback_return":
        assert callback.count("return 1;") == 1
        callback = callback.replace("return 1;", "return 0;", 1)
    elif mutation == "wrong_enum_context":
        old = "Win::adjust_menus, 0);"
        assert update.count(old) == 1
        update = update.replace(old, "Win::adjust_menus, 1);", 1)
    elif mutation == "reverse_member_order":
        old = (
            "        obj->set_rects();\n"
            "        Menu *menu = obj->menu_;\n"
            "        if (menu != 0) {\n"
            "            menu->adjust();\n"
            "        }"
        )
        new = (
            "        Menu *menu = obj->menu_;\n"
            "        if (menu != 0) {\n"
            "            menu->adjust();\n"
            "        }\n"
            "        obj->set_rects();"
        )
        assert callback.count(old) == 1
        callback = callback.replace(old, new, 1)
    return HOST_PREFIX + "\n" + update + "\n" + callback + r"""

static int run_case(Win *window, int expected_count, const char *label) {
    HWND__ raw_window = {71};
    expected_hwnd = &raw_window;
    user_data = static_cast<long>(reinterpret_cast<intptr_t>(window));
    last_userdata_index = 0;
    last_userdata_hwnd = 0;
    last_userdata_value = 0;
    observed_callback = 0;
    observed_context = -1;
    observed_callback_result = -1;
    event_count = 0;
    Win::update_menus();

    if (last_userdata_index != -0x15 || last_userdata_hwnd != expected_hwnd)
        return control_failure("userdata-slot-and-window");
    if (observed_callback != &Win::adjust_menus || observed_context != 0)
        return control_failure("enumerator-function-pointer-boundary");
    if (observed_callback_result != 1)
        return control_failure("callback-return-value");
    const int expected_events[] = {1, 2, 3, 4, 5};
    if (event_count != expected_count)
        return control_failure(label);
    for (int i = 0; i < expected_count; ++i) {
        if (events[i] != expected_events[i]) return control_failure(label);
    }
    return 0;
}

int main() {
    Menu menu;
    Win with_menu = {&menu};
    Win without_menu = {0};
    int result = run_case(0, 3, "null-receiver-path");
    if (result != 0) return result;
    result = run_case(&without_menu, 4, "set-rects-before-null-menu-path");
    if (result != 0) return result;
    return run_case(&with_menu, 5, "member-dispatch-order");
}
"""


def _compile_run(source: str, compiler: str, tmp_path: Path, stem: str):
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-no-pie", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_enumerator_and_callback_paths(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = WIN_CPP.read_text(encoding="utf-8")
    update = _extract(source, UPDATE_SIGNATURE)
    callback = _extract(source, CALLBACK_SIGNATURE)
    result = _compile_run(_host_source(callback, update), compiler, tmp_path,
                          "win_enumeration_callback")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("mutation", "diagnostic"), (
    ("wrong_userdata_index", "userdata-slot-and-window"),
    ("drop_set_rects", "set-rects-before-null-menu-path"),
    ("drop_menu_adjust", "member-dispatch-order"),
    ("wrong_callback_return", "callback-return-value"),
    ("wrong_enum_context", "enumeration-typed-callback-and-context"),
    ("reverse_member_order", "member-dispatch-order"),
))
def test_compile_success_callback_damages_are_observable(
    mutation: str, diagnostic: str, compiler_name: str, tmp_path: Path,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = WIN_CPP.read_text(encoding="utf-8")
    update = _extract(source, UPDATE_SIGNATURE)
    callback = _extract(source, CALLBACK_SIGNATURE)
    result = _compile_run(_host_source(callback, update, mutation), compiler,
                          tmp_path, f"win_enumeration_{mutation}")
    assert result.returncode == 98, result.stdout + result.stderr
    assert f"CONTROL_FAIL:{diagnostic}" in result.stderr
