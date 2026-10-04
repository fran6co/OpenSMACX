"""Exercise the actual typed helper definitions without the proprietary image."""
import shutil
import subprocess
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_ui_callbacks(tmp_path, compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    selection = definition((ROOT / "src/datalink.cpp").read_text(),
                           r"int __cdecl datalink_entry_selection\([^)]*\)")
    callback = definition((ROOT / "src/maininterface.cpp").read_text(),
                          r"void __stdcall main_interface_bubble_callback\([^)]*\)")
    harness = r'''
#include <cassert>
#include <climits>
#define __cdecl
#define __stdcall
int DatalinkEntrySelection = 0;
struct RECT { int left, top, right, bottom; };
struct Win {
    int stage;
    char *seen_text;
    RECT seen;
    Win() : stage(0), seen_text(0) {}
    void client_to_screen(int *x, int *y) {
        assert(stage == 0); stage = 1; *x += 17; *y -= 31;
    }
    void set_bubble_text(char *text, RECT *rectangle) {
        assert(stage == 1); stage = 2; seen_text = text; seen = *rectangle;
    }
};
struct MainInterface : Win {
    void set_bubble_text(unsigned int, const char *) { assert(false); }
};
MainInterface MainInterfaceGlobal;
'''
    checks = r'''
int main() {
    assert(datalink_entry_selection(-1) == 0);
    assert(datalink_entry_selection(INT_MAX) == INT_MAX);
    assert(datalink_entry_selection(INT_MIN) == INT_MAX);
    assert(DatalinkEntrySelection == INT_MAX);
    assert(datalink_entry_selection(0) == 0);
    assert(datalink_entry_selection(7) == 7);
    assert(datalink_entry_selection(-42) == 7);
    char text[] = "bubble";
    main_interface_bubble_callback(text, -10, 50);
    assert(MainInterfaceGlobal.stage == 2);
    assert(MainInterfaceGlobal.seen_text == text);
    assert(MainInterfaceGlobal.seen.left == 7);
    assert(MainInterfaceGlobal.seen.right == 7);
    assert(MainInterfaceGlobal.seen.top == 19);
    assert(MainInterfaceGlobal.seen.bottom == 19);
}
'''
    mutations = [
        (selection, callback),
        (selection.replace("selected_index < 0", "selected_index <= 0"), callback),
        (selection.replace("DatalinkEntrySelection = selected_index;", "DatalinkEntrySelection = 0;"), callback),
        (selection, callback.replace("rectangle.right = x;", "rectangle.right = y;")),
        (selection, callback.replace("rectangle.bottom = y;", "rectangle.bottom = x;")),
    ]
    for index, (selected, bubble) in enumerate(mutations):
        source = tmp_path / f"control_{index}.cpp"
        binary = tmp_path / f"control_{index}"
        source.write_text(harness + selected + "\n" + bubble + checks)
        subprocess.run([executable, "-std=c++98", "-fsanitize=undefined",
                        "-fno-sanitize-recover=all", str(source), "-o", str(binary)], check=True)
        result = subprocess.run([str(binary)], capture_output=True)
        assert (result.returncode == 0) == (index == 0), result.stderr.decode()
