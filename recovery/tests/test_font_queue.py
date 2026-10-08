"""Actual FontQueue class/bodies with an explicit host Font dependency double.

Expectations follow pinned 5592D0..5593D9, not a second cache implementation.
This checks bounded arithmetic and cache/lifetime behavior, not GDI, VC6 code
generation, signed-overflow inputs, or native layout/whole-game acceptance.
Reentrant init is a dependency double exposing publication order; it does not
assert that the original Font::init calls back into the queue.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source")


def actual_source():
    header = (ROOT / "src/font_queue.h").read_text()
    source = (ROOT / "src/font_queue.cpp").read_text()
    font = (ROOT / "src/interface_fonts.cpp").read_text()
    mapwin = (ROOT / "src/mapwin.cpp").read_text()
    globals_ = []
    for text, pattern in (
        (font, r'^LPCSTR DefaultFontFace\s*=\s*[^;]+;'),
        (mapwin, r'^int MapWinCaptionScale\s*=\s*[^;]+;'),
        (mapwin, r'^int MapWinCaptionDivisor\s*=\s*[^;]+;'),
    ):
        matches = re.findall(pattern, text, re.M)
        assert len(matches) == 1
        globals_.append(matches[0])
    return "\n".join([
        *globals_,
        braced(header, "class FontQueue {") + ";",
        braced(source, "FontQueue::FontQueue()"),
        braced(source, "FontQueue::~FontQueue()"),
        braced(source, "Font *FontQueue::get_zoom("),
    ])


HARNESS = r'''
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
typedef const char *LPCSTR;
void require(bool ok) { if (!ok) std::exit(42); }
struct Font;
struct Call { Font *font; LPCSTR face; int height, style; };
std::vector<Call> calls;
std::vector<Font *> constructed, destroyed;
int init_result = 0;
void (*init_hook)(Font *, int, int) = nullptr;
struct Font {
    Font() { constructed.push_back(this); }
    ~Font() { destroyed.push_back(this); }
    int init(LPCSTR face, int height, int style) {
        Call call = {this, face, height, style};
        calls.push_back(call);
        require(calls.size() < 20); // Turn accidental recursive misses into damage.
        if (init_hook) init_hook(this, height, style);
        return init_result;
    }
};
ACTUAL

Font *height(FontQueue &queue, int value, int style = 0) {
    return queue.get_zoom(0, value, 50, style);
}
void rates() {
    require(MapWinCaptionScale == -1 && MapWinCaptionDivisor == -1);
    require(std::strcmp(DefaultFontFace, "Arial") == 0);
    // Independent literal outcomes: first signed quotient is truncated BEFORE
    // multiplication and the second signed division (5592E5/5592FF/559307).
    const int cases[][7] = {
        {-1, -1, 0, 1, 1, 0, 50},
        {0, 0, 2, 1, 1, 1, 56},
        {-7, 0, -17, 8, 1, 2, -24},
        {-1, -1, -15, 8, 1, 3, 24},
        {-1, -1, -17, 7, 2, 4, -10},
        {-1, -1, -17, -7, 2, 5, 11},
        {-1, -1, -17, 7, -2, 6, 11},
        {1, 3, 99, 8, 1, 7, 128},
        {1, -3, 99, 8, 1, 8, -128},
        {3, 2, -99, -1, 2, 9, -37},
        {-1, -1, 0, 1, 5, 10, 11},
        {-1, -1, -16, 3, 7, 11, 0},
    };
    for (const auto &c : cases) {
        FontQueue queue;
        calls.clear();
        MapWinCaptionScale = c[0]; MapWinCaptionDivisor = c[1];
        Font *got = queue.get_zoom(c[2], c[3], c[4], c[5]);
        require(calls.size() == 1 && got == calls[0].font);
        require(calls[0].height == c[6] && calls[0].style == c[5]);
        require(calls[0].face == DefaultFontFace);
    }
}
void order() {
    FontQueue queue;
    require(constructed.size() == 3);
    Font *a = height(queue, 20), *b = height(queue, 21), *c = height(queue, 22);
    require(a == constructed[0] && b == constructed[1] && c == constructed[2]);
    require(height(queue, 20) == a && calls.size() == 3); // oldest -> newest
    require(height(queue, 23) == b && calls.size() == 4);
    require(height(queue, 22) == c && calls.size() == 4); // oldest -> newest
    require(height(queue, 24) == a && calls.size() == 5);
    require(height(queue, 24) == a && calls.size() == 5); // newest stays newest
    require(height(queue, 25) == b && calls.size() == 6);
    {
        FontQueue second;
        Font *x = height(second, 30), *y = height(second, 31), *z = height(second, 32);
        require(height(second, 31) == y && calls.size() == 9); // middle -> newest
        require(height(second, 33) == x && calls.size() == 10);
        require(height(second, 32) == z && calls.size() == 10);
    }
}
void keys_and_failure() {
    FontQueue queue;
    // Constructor sentinel is observable as a hit without initialization.
    Font *sentinel = height(queue, -999);
    require(calls.empty() && sentinel == constructed[0]);
    Font *first = height(queue, 20, -7);
    require(first == constructed[1]); // sentinel hit promoted slot zero
    require(height(queue, 20, -7) == first && calls.size() == 1);
    Font *other_style = height(queue, 20, 2);
    require(other_style != first && calls.size() == 2);
    Font *eleven = queue.get_zoom(0, 1, 5, 4); // computed ten becomes eleven
    require(height(queue, 11, 4) == eleven && calls.size() == 3);
    const char alternate[] = "Host alternate font";
    DefaultFontFace = alternate;
    require(height(queue, 11, 4) == eleven && calls.size() == 3); // face not key
    init_result = 3; // Font::init reports nonzero errors (null face uses three).
    Font *failed = height(queue, 27, 9);
    require(failed && calls.size() == 4 && calls.back().font == failed);
    require(calls.back().face == alternate);
    require(height(queue, 27, 9) == failed && calls.size() == 4); // no retry
    DefaultFontFace = nullptr; // Init receives the configured value, unmodified.
    require(height(queue, 28, 9) != nullptr && calls.size() == 5);
    require(calls.back().face == nullptr);
}
FontQueue *active;
void observe_publication(Font *font, int h, int style) {
    init_hook = nullptr;
    // A different request during init must use the NEXT slot. Both keys and
    // replacement order were published before call559374. Do this BEFORE a
    // hit: hit promotion could otherwise conceal a delayed miss rotation.
    Font *next = height(*active, h + 1, style);
    require(next == constructed[1] && next != font && calls.size() == 2);
    require(height(*active, h, style) == font && calls.size() == 2);
}
void publication() {
    FontQueue queue;
    active = &queue; init_hook = observe_publication;
    require(height(queue, 40, 5) == constructed[0]);
    require(calls.size() == 2);
    require(height(queue, 41, 5) == constructed[1] && calls.size() == 2);
}
void lifetime() {
    {
        FontQueue queue;
        require(constructed.size() == 3 && destroyed.empty());
        require(height(queue, 17) == constructed[0]);
    }
    require(destroyed.size() == 3);
    require(destroyed[0] == constructed[2] && destroyed[1] == constructed[1]
            && destroyed[2] == constructed[0]);
}
int main(int argc, char **argv) {
    require(argc == 2);
    if (!std::strcmp(argv[1], "rates")) rates();
    else if (!std::strcmp(argv[1], "order")) order();
    else if (!std::strcmp(argv[1], "keys")) keys_and_failure();
    else if (!std::strcmp(argv[1], "publication")) publication();
    else if (!std::strcmp(argv[1], "lifetime")) lifetime();
    else return 43;
}
'''


def compile_host(tmp_path, source):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    unit = tmp_path / "font_queue.cpp"
    executable = tmp_path / "font_queue"
    unit.write_text(HARNESS.replace("ACTUAL", source))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module")
def actual_executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("font-queue"), actual_source())


@pytest.mark.parametrize("scenario", ["rates", "order", "keys", "publication", "lifetime"])
def test_actual_font_queue(actual_executable, scenario):
    result = subprocess.run([str(actual_executable), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after,scenario", [
    ("MapWinCaptionScale > 0", "MapWinCaptionScale > 1", "rates"),
    ("((zoom * 5 + 80) * 10) / 16", "((zoom * 5 + 80) * 10) >> 4", "rates"),
    ("int height = (scale * numerator) / denominator;",
     "int height = scale * (numerator / denominator);", "rates"),
    ("int height = (scale * numerator) / denominator;",
     "int height = static_cast<unsigned int>(scale * numerator) / denominator;", "rates"),
    ("height = 11;", "height = 10;", "rates"),
    (" && styles_[slot] == style", "", "keys"),
    ("heights_[i] = -999;", "heights_[i] = 0;", "keys"),
    ("lru_slots_[i] = i;", "lru_slots_[i] = 2 - i;", "order"),
    ("lru_slots_[next] = lru_slots_[next + 1];",
     "lru_slots_[next] = lru_slots_[next];", "order"),
    ("lru_slots_[position] = lru_slots_[position + 1];",
     "lru_slots_[position] = lru_slots_[position];", "order"),
    ("font->init(DefaultFontFace, height, style);",
     "font->init(\"Arial\", height, style);", "keys"),
    ("font->init(DefaultFontFace, height, style);",
     "if (font->init(DefaultFontFace, height, style) != 0) return 0;", "keys"),
    ("font->init(DefaultFontFace, height, style);",
     "if (font->init(DefaultFontFace, height, style) != 0) heights_[oldest] = -999;", "keys"),
    ("heights_[oldest] = height;", "/* delayed key */", "publication"),
])
def test_font_queue_damage_compiles_but_fails(tmp_path, before, after, scenario):
    source = actual_source()
    assert source.count(before) == 1
    source = source.replace(before, after)
    executable = compile_host(tmp_path, source)
    result = subprocess.run([str(executable), scenario], capture_output=True,
                            text=True, timeout=10)
    # A compilation error, crash or sanitizer diagnostic cannot satisfy this.
    assert result.returncode == 42, result.stderr


def test_miss_rotation_after_init_is_behavioral_damage(tmp_path):
    source = actual_source()
    rotation = """    for (int position = 0; position < 2; ++position) {
        lru_slots_[position] = lru_slots_[position + 1];
    }
    lru_slots_[2] = oldest;"""
    call = "font->init(DefaultFontFace, height, style);"
    assert source.count(rotation) == source.count(call) == 1
    source = source.replace(rotation, "").replace(call, call + "\n" + rotation)
    executable = compile_host(tmp_path, source)
    result = subprocess.run([str(executable), "publication"], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 42, result.stderr
