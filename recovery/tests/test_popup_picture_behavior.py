"""Actual Popup picture body controls; service doubles are not class ABI proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

SOURCE = Path(__file__).resolve().parents[2] / "src/popup.cpp"
SIGNATURE = "int Popup::set_picture(Sprite *picture, int mode, char *text) {"


def body():
    text = SOURCE.read_text()
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if not depth:
                return text[start:end + 1]
    raise AssertionError("unterminated Popup picture body")


HARNESS = r'''
#include <assert.h>
struct Sprite { int identity; };
struct SpriteBox {
    Sprite *seen_picture, **caller_source, *picture_at_call;
    char *seen_text;
    int seen_mode, result, calls;
    SpriteBox() : seen_picture(0), caller_source(0), picture_at_call(0),
                  seen_text(0), seen_mode(0), result(0), calls(0) {}
    int sprite(Sprite *picture, char *text, int mode) {
        ++calls;
        seen_picture = picture;
        picture_at_call = *caller_source;
        seen_text = text;
        seen_mode = mode;
        return result;
    }
};
struct Dialogs : SpriteBox {};
class Popup {
public:
    int count;
    Sprite *caller_picture;
    Dialogs dialog;
    Popup() : count(0), caller_picture(0), dialog() {
        dialog.caller_source = &caller_picture;
    }
    bool picture_enabled() const { return count != 0; }
    void set_caller_picture(Sprite *p) { caller_picture = p; }
    Dialogs &dialogs() { return dialog; }
    int set_picture(Sprite *picture, int mode, char *text);
};
BODY
int main() {
    Sprite incoming = {11}, previous = {22};
    char text[] = "picture";
    const int counts[] = {0, 3, -1};
    for (int index = 0; index != 3; ++index) {
        Popup popup;
        popup.count = counts[index];
        popup.caller_picture = &previous;
        popup.dialog.result = index ? -19 : 73;
        const int mode = index ? -7 : 5;
        const int result = popup.set_picture(&incoming, mode, text);
        Sprite *expected = counts[index] ? 0 : &incoming;
        assert(popup.caller_picture == expected);
        assert(popup.dialog.picture_at_call == expected);
        assert(popup.dialog.seen_picture == &incoming);
        assert(popup.dialog.seen_text == text);
        assert(popup.dialog.seen_mode == mode);
        assert(popup.dialog.calls == 1);
        assert(result == popup.dialog.result);
    }
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["none", "stored_picture", "branch_polarity"])
def test_actual_popup_picture(compiler, damage, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip("host compiler unavailable")
    candidate = body()
    if damage == "stored_picture":
        good = "dialogs().sprite(picture, text, mode)"
        assert candidate.count(good) == 1
        candidate = candidate.replace(good, "dialogs().sprite(caller_picture, text, mode)")
    elif damage == "branch_polarity":
        assert candidate.count("!picture_enabled()") == 1
        candidate = candidate.replace("!picture_enabled()", "picture_enabled()")
    source = tmp_path / "picture.cpp"
    binary = tmp_path / "picture"
    source.write_text(HARNESS.replace("BODY", candidate))
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if damage == "none":
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0
        assert "Assertion" in run.stderr or "assertion" in run.stderr
