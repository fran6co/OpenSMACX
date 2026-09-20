"""Actual Buffer mono wrapper and descriptor helpers with a Sprite call double.

Pinned 5D8930..5D8A28 supplies the expectations: second argument selects the
descriptor key, all seven original arguments reach Sprite, and the borrowed
pointer is cleared after the call. These are host behavior checks, not native
layout, COM integration, Sprite rasterization, or whole-game acceptance.
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


def declaration(text, name):
    matches = re.findall(r"^\s*(?:LPVOID|IDirectDrawSurface\s*\*|uint32_t|int|"
                         r"char\s*\*|unsigned char)\s*" + name + r"\s*;", text, re.M)
    assert len(matches) == 1, name
    return matches[0].strip()


def actual_source():
    source = (ROOT / "src/buffer.cpp").read_text()
    header = (ROOT / "src/buffer.h").read_text()
    sprite = (ROOT / "src/sprite.h").read_text()
    win = (ROOT / "src/win.cpp").read_text()
    constant = re.findall(r"^const unsigned char WinFillColour\s*=\s*[^;]+;", win, re.M)
    assert len(constant) == 1
    signature = re.findall(r"\bint draw_mono\(Buffer \*destination,[^;]+;", header)
    assert len(signature) == 1
    buffer_fields = "\n".join(declaration(header, n) for n in (
        "locked_bits_", "dib_bits_", "surface_", "surface_lock_count_", "stride_"))
    sprite_fields = "\n".join(declaration(sprite, n) for n in (
        "pcBits_", "cTransparentIndex_", "iSpriteWidth2_", "iSpriteWidth_",
        "iSpriteHeight_", "iWidth_", "iHeight_", "iLeftOffset_", "iTopOffset_",
        "fObj1Exists_"))
    return "\n".join([
        constant[0],
        "struct Buffer; struct Sprite {", sprite_fields,
        "int draw_mono(Buffer *, int, int, int, int, int, int); };",
        "Sprite g_BUFFER_SPRITE;",
        "struct Buffer {", buffer_fields,
        "Dib dib_; void setup_buff_sprite(int); static void release_buff_sprite();",
        signature[0], "};",
        braced(source, "void Buffer::setup_buff_sprite("),
        braced(source, "void Buffer::release_buff_sprite("),
        braced(source, "int Buffer::draw_mono("),
    ])


HARNESS = r'''
#include <cstdint>
#include <cstdlib>
#include <cstring>
typedef void *LPVOID;
void require(bool condition) { if (!condition) std::exit(42); }
struct IDirectDrawSurface {
    int calls = 0;
    void Unlock(void *) { ++calls; }
};
// Explicit host dependency shape; no assertion about the complete Windows ABI.
struct Dib { struct { int32_t biWidth, biHeight; } bmiHeader; };
ACTUAL
Buffer *expected_destination;
Buffer *source_buffer;
Sprite expected_sprite;
void *expected_locked;
int expected_count, expected_args[6], draw_result, calls;
bool replace_borrow;
char replacement[4];

int Sprite::draw_mono(Buffer *destination, int transparent, int x, int y,
                      int colour, int numerator, int denominator) {
    ++calls;
    require(this == &g_BUFFER_SPRITE && destination == expected_destination);
    const int args[] = {transparent, x, y, colour, numerator, denominator};
    for (int i = 0; i < 6; ++i) require(args[i] == expected_args[i]);
    require(pcBits_ == expected_sprite.pcBits_);
    require(cTransparentIndex_ == expected_sprite.cTransparentIndex_);
    require(iSpriteWidth2_ == expected_sprite.iSpriteWidth2_);
    require(iSpriteWidth_ == expected_sprite.iSpriteWidth_);
    require(iSpriteHeight_ == expected_sprite.iSpriteHeight_);
    require(iWidth_ == expected_sprite.iWidth_ && iHeight_ == expected_sprite.iHeight_);
    require(iLeftOffset_ == expected_sprite.iLeftOffset_);
    require(iTopOffset_ == expected_sprite.iTopOffset_);
    require(fObj1Exists_ == expected_sprite.fObj1Exists_);
    require(source_buffer->locked_bits_ == expected_locked);
    require(source_buffer->surface_lock_count_ == expected_count);
    if (replace_borrow) pcBits_ = replacement;
    return draw_result;
}

void check(int scenario) {
    char pixels[40]; std::memset(pixels, 0xA7, sizeof pixels);
    char old_bits[4];
    Buffer source = {}, destination = {};
    IDirectDrawSurface surface;
    source_buffer = &source;
    source.dib_bits_ = pixels;
    source.locked_bits_ = old_bits;
    source.stride_ = 12;
    source.dib_.bmiHeader.biWidth = 9;
    source.dib_.bmiHeader.biHeight = -3;
    source.surface_lock_count_ = 0;
    expected_destination = &destination;
    int transparent = 0x134;
    draw_result = -17;
    if (scenario == 1) { transparent = -1; draw_result = 3; }
    if (scenario == 2) { transparent = -2; source.surface_lock_count_ = 2; }
    if (scenario == 3) { source.dib_bits_ = nullptr; draw_result = 0; }
    if (scenario == 4) { source.dib_bits_ = nullptr; source.surface_lock_count_ = 2; }
    if (scenario == 5) { source.surface_ = &surface; source.surface_lock_count_ = 3; }
    if (scenario == 6) expected_destination = nullptr;
    if (scenario == 7) replace_borrow = true;
    g_BUFFER_SPRITE = {};
    g_BUFFER_SPRITE.pcBits_ = old_bits;
    g_BUFFER_SPRITE.cTransparentIndex_ = 71;
    g_BUFFER_SPRITE.iSpriteWidth2_ = 81;
    g_BUFFER_SPRITE.iSpriteWidth_ = 82;
    g_BUFFER_SPRITE.iSpriteHeight_ = 83;
    g_BUFFER_SPRITE.iWidth_ = 84; g_BUFFER_SPRITE.iHeight_ = 85;
    g_BUFFER_SPRITE.iLeftOffset_ = 86; g_BUFFER_SPRITE.iTopOffset_ = 87;
    g_BUFFER_SPRITE.fObj1Exists_ = 88;
    expected_sprite = g_BUFFER_SPRITE;
    expected_locked = old_bits; expected_count = 3;
    if (scenario != 5) {
        expected_sprite.pcBits_ = static_cast<char *>(source.dib_bits_);
        expected_sprite.cTransparentIndex_ = scenario == 1 ? 9 :
            (scenario == 2 ? 254 : 52);
        expected_sprite.iSpriteWidth2_ = 12;
        expected_sprite.iSpriteWidth_ = expected_sprite.iWidth_ = 9;
        expected_sprite.iSpriteHeight_ = expected_sprite.iHeight_ = 3;
        expected_sprite.iLeftOffset_ = expected_sprite.iTopOffset_ = 0;
        expected_sprite.fObj1Exists_ = 0;
        expected_locked = scenario == 2 ? pixels : nullptr;
        expected_count = scenario == 2 ? 2 : (scenario == 4 ? 1 : 0);
    }
    expected_args[0] = transparent;
    expected_args[1] = -23; expected_args[2] = 41; expected_args[3] = 0x1F9;
    expected_args[4] = 3; expected_args[5] = 7;
    int result = source.draw_mono(expected_destination, transparent, -23, 41, 0x1F9, 3, 7);
    require(result == draw_result && calls == 1);
    require(g_BUFFER_SPRITE.pcBits_ == nullptr);
    require(g_BUFFER_SPRITE.cTransparentIndex_ == expected_sprite.cTransparentIndex_);
    require(g_BUFFER_SPRITE.iSpriteWidth2_ == expected_sprite.iSpriteWidth2_);
    require(source.surface_lock_count_ == expected_count && source.locked_bits_ == expected_locked);
    require(source.dib_bits_ == (scenario == 3 || scenario == 4 ? nullptr : pixels));
    require(surface.calls == 0);
    for (char pixel : pixels) require(static_cast<unsigned char>(pixel) == 0xA7);
}
int main(int argc, char **argv) {
    require(argc == 2); check(std::atoi(argv[1]));
}
'''


def compile_host(tmp_path, source):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    unit = tmp_path / "buffer_mono.cpp"
    executable = tmp_path / "buffer_mono"
    unit.write_text(HARNESS.replace("ACTUAL", source))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module")
def actual_executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("buffer-mono"), actual_source())


@pytest.mark.parametrize("scenario", range(8))
def test_actual_buffer_mono(actual_executable, scenario):
    result = subprocess.run([str(actual_executable), str(scenario)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after,scenario", [
    ("setup_buff_sprite(transparent);", "setup_buff_sprite(colour);", 0),
    ("destination, transparent, x, y, colour, numerator, denominator);",
     "destination, transparent == -1 ? WinFillColour : transparent, x, y, colour, numerator, denominator);", 1),
    ("destination, transparent, x, y, colour, numerator, denominator);",
     "this, transparent, x, y, colour, numerator, denominator);", 0),
    ("destination, transparent, x, y, colour, numerator, denominator);",
     "destination, transparent, y, x, colour, numerator, denominator);", 0),
    ("destination, transparent, x, y, colour, numerator, denominator);",
     "destination, transparent, x, y, colour & 255, numerator, denominator);", 0),
    ("destination, transparent, x, y, colour, numerator, denominator);",
     "destination, transparent, x, y, colour, denominator, numerator);", 0),
    ("    release_buff_sprite();", "    /* missing release */", 0),
    ("    return result;", "    return 0;", 0),
    ("g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;",
     "g_BUFFER_SPRITE.iSpriteWidth2_ = dib_.bmiHeader.biWidth;", 0),
    ("    setup_buff_sprite(transparent);", "    if (!destination) return 3;\n    setup_buff_sprite(transparent);", 6),
    ("    setup_buff_sprite(transparent);", "    if (surface_) return 3;\n    setup_buff_sprite(transparent);", 5),
    ("++surface_lock_count_;", "/* missing acquisition */", 2),
    ("const int count = --surface_lock_count_;",
     "const int count = surface_lock_count_;", 4),
])
def test_damage_compiles_but_fails(tmp_path, before, after, scenario):
    source = actual_source()
    assert before in source
    executable = compile_host(tmp_path, source.replace(before, after))
    result = subprocess.run([str(executable), str(scenario)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 42, result.stderr


def test_release_before_draw_is_behavioral_damage(tmp_path):
    source = actual_source()
    assert source.count("    release_buff_sprite();") == 1
    source = source.replace("    release_buff_sprite();", "")
    source = source.replace("    int result = g_BUFFER_SPRITE.draw_mono(",
                            "    release_buff_sprite();\n    int result = g_BUFFER_SPRITE.draw_mono(")
    executable = compile_host(tmp_path, source)
    result = subprocess.run([str(executable), "0"], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 42, result.stderr
