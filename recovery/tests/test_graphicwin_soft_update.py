"""Actual soft_update bodies with bounded host/UBSan behavioral doubles.

These controls establish neither VC6 ABI nor live USER32/whole-game behavior.
The rectangle double implements intersection, including aliased output, rather
than supplying a preselected result. Expected coordinates and call sequences
come from the original visible/copy/clip/translate/invalidate paths.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def extract(source, signature):
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated live method")


def live_bodies():
    path = ROOT / "src/graphicwin.cpp"
    records = read_file(path)
    for address, end in ((0x5D5890, 0x5D592D), (0x5D5930, 0x5D5A6E)):
        record, = (record for record in records if record.address == address)
        assert record.image_spans == ((address, end),)
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    return [extract(source, signature) for signature in (
        "void GraphicWin::soft_update()",
        "void GraphicWin::soft_update(RECT *area)",
    )]


HARNESS = r"""
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
void check(bool ok) { if (!ok) std::exit(91); }
struct RECT { std::int32_t left, top, right, bottom; };
typedef void *HWND;
enum { FALSE = 0, TRUE = 1 };
bool same(const RECT &a, const RECT &b) {
    return a.left == b.left && a.top == b.top &&
           a.right == b.right && a.bottom == b.bottom;
}
struct Win { int is_visible(); void client_to_screen(int *, int *); };
struct Buffer { RECT rect2_; };
struct GraphicWin : Win, Buffer {
    void soft_update(); void soft_update(RECT *area);
} window;
int main_token, fresh_token, other_token;
HWND HandleMain = &main_token, OtherHandle = &other_token;
const char *expected_events;
int event_count, visible_calls, visible_answers[2], invalidations;
bool mutate_coordinates;
RECT caller, bounds_before, caller_before, invalid_expected;
const RECT changed_bounds = {101, 102, 103, 104};
const RECT changed_caller = {201, 202, 203, 204};
RECT *borrowed;
void event(char name) {
    check(expected_events[event_count] == name);
    ++event_count;
}
int Win::is_visible() {
    check(this == static_cast<Win *>(&window));
    event('V'); check(visible_calls < 2);
    return visible_answers[visible_calls++];
}
void Win::client_to_screen(int *x, int *y) {
    check(this == static_cast<Win *>(&window));
    event('C'); check(*x == 0 && *y == 0);
    *x = 17; *y = -9;
    if (mutate_coordinates) {
        window.rect2_ = changed_bounds;
        if (borrowed == &caller) caller = changed_caller;
    }
    // The invalidation must read the main handle at the call, not use an
    // unrelated window handle or a value cached before coordinate conversion.
    HandleMain = &fresh_token;
}
int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
    event('I');
    check(out == a && out != &window.rect2_ && out != borrowed);
    check(b == &window.rect2_ && same(*b, bounds_before));
    check(same(*a, borrowed == &window.rect2_ ? bounds_before : caller_before));
    const RECT first = *a, second = *b;
    RECT clipped = {std::max(first.left, second.left),
                    std::max(first.top, second.top),
                    std::min(first.right, second.right),
                    std::min(first.bottom, second.bottom)};
    if (clipped.left >= clipped.right || clipped.top >= clipped.bottom) {
        *out = RECT{0, 0, 0, 0}; return FALSE;
    }
    *out = clipped; return TRUE;
}
int InvalidateRect(HWND handle, const RECT *area, int erase) {
    event('N');
    check(handle == HandleMain && handle == &fresh_token && handle != OtherHandle);
    check(erase == FALSE && area != &window.rect2_ && area != borrowed);
    check(same(*area, invalid_expected));
    ++invalidations; return TRUE;
}

// LIVE_BODIES

// form: 0 no argument, 1 borrowed caller rectangle, 2 null, 3 owned alias.
void run(int form, RECT bounds, RECT input, int first, int second,
         const char *events, bool mutate, RECT expected) {
    window.rect2_ = bounds; bounds_before = bounds;
    caller = input; caller_before = input;
    borrowed = form == 1 ? &caller : form == 3 ? &window.rect2_ : 0;
    event_count = visible_calls = invalidations = 0;
    expected_events = events;
    visible_answers[0] = first; visible_answers[1] = second;
    mutate_coordinates = mutate; invalid_expected = expected;
    HandleMain = &main_token;
    if (form == 0) window.soft_update();
    else window.soft_update(borrowed);
    check(expected_events[event_count] == '\0');
    const bool converted = std::strchr(events, 'C') != 0;
    check(invalidations == (std::strchr(events, 'N') != 0 ? 1 : 0));
    check(same(window.rect2_, mutate && converted ? changed_bounds : bounds));
    check(same(caller, mutate && converted && form == 1 ? changed_caller : input));
}
int main() {
    const RECT bounds = {-4, 6, 30, 40}, input = {-20, 10, 8, 100};
    const RECT full = {13, -3, 47, 31}, clipped = {13, 1, 25, 31};
    const RECT unused = {0, 0, 0, 0};
    run(0, bounds, input, 0, 1, "V", false, unused);
    run(0, bounds, input, 1, 0, "VCN", true, full);
    // Unlike the clipping overload, noarg still invalidates degenerate bounds.
    run(0, RECT{3, 5, 3, 9}, input, 1, 0, "VCN", false, RECT{20, -4, 20, 0});
    run(1, bounds, input, 0, 1, "V", false, unused);
    run(2, bounds, input, 0, 1, "V", false, unused);
    run(2, bounds, input, 1, 0, "VV", false, unused);
    run(2, bounds, input, 1, 1, "VVCN", true, full);
    run(1, bounds, input, 1, 0, "VICN", true, clipped);
    const RECT empty[] = {{31, 10, 50, 20}, {30, 10, 50, 20},
                          {7, 10, 6, 20}, {0, 10, 8, 10}};
    for (unsigned i = 0; i < sizeof(empty) / sizeof(empty[0]); ++i)
        run(1, bounds, empty[i], 1, 0, "VI", false, unused);
    run(3, bounds, input, 1, 0, "VICN", true, full);
    return 0;
}
"""


def replace_once(text, old, new):
    assert text.count(old) == 1, old
    return text.replace(old, new)


def damage(bodies, kind):
    noarg, area = bodies
    if kind == "wrong_handle":
        assert "InvalidateRect(HandleMain," in noarg
        noarg = noarg.replace("InvalidateRect(HandleMain,", "InvalidateRect(OtherHandle,")
    elif kind == "late_copy":
        noarg = replace_once(noarg,
            "RECT local = rect2_;\n    Win::client_to_screen(&x, &y);",
            "Win::client_to_screen(&x, &y);\n    RECT local = rect2_;")
    elif kind == "borrowed_not_copied":
        area = replace_once(area, "RECT local = *area;", "RECT &local = *area;")
    elif kind == "skip_intersection":
        area = replace_once(area, "if (!IntersectRect(", "if (false && !IntersectRect(")
    elif kind == "missing_second_visibility":
        branch = extract(area, "if (!area)")
        if "soft_update();" in branch:
            interior = noarg[noarg.index("{") + 1:noarg.rindex("}")]
            interior = replace_once(interior, "if (!Win::is_visible()) return;", "")
            damaged = replace_once(branch, "soft_update();", interior)
        else:
            damaged = replace_once(branch, "if (!Win::is_visible()) return;", "")
        area = replace_once(area, branch, damaged)
    else:
        raise AssertionError(kind)
    return [noarg, area]


def run_host(tmp_path, bodies):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required for bounded semantic control")
    source = tmp_path / "soft_update.cpp"
    binary = tmp_path / "soft_update"
    source.write_text(HARNESS.replace("// LIVE_BODIES", "\n\n".join(bodies)))
    compiled = subprocess.run([
        compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
        "-fno-sanitize-recover=undefined", str(source), "-o", str(binary),
    ], text=True, capture_output=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)


def test_live_soft_update_paths(tmp_path):
    result = run_host(tmp_path, live_bodies())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("kind", [
    "wrong_handle", "late_copy", "borrowed_not_copied", "skip_intersection",
    "missing_second_visibility",
])
def test_soft_update_behavior_damage(tmp_path, kind):
    result = run_host(tmp_path, damage(live_bodies(), kind))
    # A mutation must compile and reach the explicit behavioral assertion,
    # rather than count a compiler error, signal, or sanitizer crash as proof.
    assert result.returncode == 91, result.stdout + result.stderr
