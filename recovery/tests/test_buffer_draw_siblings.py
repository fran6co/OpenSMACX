"""Bounded actual-source controls for Buffer's three shared-Sprite siblings.

The test compiles the product method bodies with small field/service doubles.
It covers descriptor publication, forwarding, and cleanup; it does not prove
the 32-bit VC6 ABI, native object layout, real DirectDraw behavior, or game
acceptance. The Sprite zero-return body is compiled separately as a positive
control; the observed service double lets the Buffer harness inspect caller
state at the call boundary.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
METHODS = ("draw_unk2", "draw", "draw_dest_unk2")


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual method: " + signature)


def actual_source():
    source = (ROOT / "src/buffer.cpp").read_text()
    sprite_source = (ROOT / "src/sprite.cpp").read_text()
    methods = {}
    for name in METHODS:
        signatures = list(re.finditer(
            r"\bint\s+Buffer::" + name + r"\s*\([^)]*\)", source))
        if name == "draw":
            signatures = [match for match in signatures
                          if "unsigned char *" in match.group(0)]
        assert len(signatures) == 1, "missing or ambiguous actual Buffer::" + name
        signature = signatures[0]
        methods[name] = braced(source, signature.group(0))

    assert "draw_noop(source" in methods["draw_unk2"]
    assert "reinterpret_cast<int>(source)" not in methods["draw_unk2"]
    noop = braced(sprite_source, "int Sprite::draw_noop(")
    assert "return 0;" in noop
    methods["sprite_noop_actual"] = noop.replace(
        "Sprite::draw_noop(", "SpriteNoop::draw_noop(")
    return methods


HARNESS = r'''#include <cstdint>
#include <cstdlib>
#include <cstring>

void require(bool ok) { if (!ok) std::exit(42); }
const int WinFillColour = 0x5a;

struct Buffer;
struct Seen {
    int route, transparent, x, y;
    Buffer *destination;
    uintptr_t table;
    void *bits;
    int pitch, width, sprite_height, logical_width, logical_height;
    int left, top, colour, object_exists;
    int result;
} seen;
bool replace_bits_during_call;
char *const replacement_bits = reinterpret_cast<char *>(0x1234);

struct Sprite {
    char *pcBits_;
    int iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_;
    int iWidth_, iHeight_, iLeftOffset_, iTopOffset_;
    unsigned char cTransparentIndex_;
    int fObj1Exists_;
    int draw_noop(Buffer *, int, int, int, int);
    int draw(Buffer *, int, int, int, unsigned char *);
    int draw_dest_unk2(Buffer *, int, int, int, unsigned char *);
};
Sprite g_BUFFER_SPRITE;
struct SpriteNoop { int draw_noop(Buffer *, int, int, int, int); };

struct IDirectDrawSurface {
    int unlocks;
    void *unlocked;
    void Unlock(void *bits) { ++unlocks; unlocked = bits; }
};
struct Dib { struct { int biWidth, biHeight; } bmiHeader; };

struct Buffer {
    void *locked_bits_;
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    int surface_lock_count_;
    uint32_t stride_;
    Dib dib_;
    int draw_unk2(Buffer *, int, int, int, int);
    int draw(Buffer *, int, int, int, unsigned char *);
    int draw_dest_unk2(Buffer *, int, int, int, unsigned char *);
};

static void observe(int route, Buffer *destination, int transparent,
                    int x, int y, uintptr_t table) {
    seen.route = route; seen.destination = destination;
    seen.transparent = transparent; seen.x = x; seen.y = y;
    seen.table = table; seen.bits = g_BUFFER_SPRITE.pcBits_;
    seen.pitch = g_BUFFER_SPRITE.iSpriteWidth2_;
    seen.width = g_BUFFER_SPRITE.iSpriteWidth_;
    seen.sprite_height = g_BUFFER_SPRITE.iSpriteHeight_;
    seen.logical_width = g_BUFFER_SPRITE.iWidth_;
    seen.logical_height = g_BUFFER_SPRITE.iHeight_;
    seen.left = g_BUFFER_SPRITE.iLeftOffset_;
    seen.top = g_BUFFER_SPRITE.iTopOffset_;
    seen.colour = g_BUFFER_SPRITE.cTransparentIndex_;
    seen.object_exists = g_BUFFER_SPRITE.fObj1Exists_;
}
int Sprite::draw_noop(Buffer *destination, int transparent, int x, int y,
                      int table) {
    observe(0, destination, transparent, x, y, static_cast<uintptr_t>(table));
    if (replace_bits_during_call) g_BUFFER_SPRITE.pcBits_ = replacement_bits;
    return seen.result = 0x1357;
}
int Sprite::draw(Buffer *destination, int transparent, int x, int y,
                 unsigned char *table) {
    observe(1, destination, transparent, x, y,
            reinterpret_cast<uintptr_t>(table));
    if (replace_bits_during_call) g_BUFFER_SPRITE.pcBits_ = replacement_bits;
    return seen.result = 0x2468;
}
int Sprite::draw_dest_unk2(Buffer *destination, int transparent, int x, int y,
                           unsigned char *table) {
    observe(2, destination, transparent, x, y,
            reinterpret_cast<uintptr_t>(table));
    if (replace_bits_during_call) g_BUFFER_SPRITE.pcBits_ = replacement_bits;
    return seen.result = 0x369c;
}

ACTUAL_METHODS
ACTUAL_SPRITE_NOOP

static int invoke(Buffer &owner, int route, Buffer *destination,
                  int transparent, int x, int y, unsigned char *table,
                  int integer_table) {
    if (route == 0)
        return owner.draw_unk2(destination, transparent, x, y, integer_table);
    if (route == 1) return owner.draw(destination, transparent, x, y, table);
    return owner.draw_dest_unk2(destination, transparent, x, y, table);
}

static void check_route(int route, bool surface_path, bool absent_bits,
                        int transparency = -1, int starting_count = 0,
                        bool negative_height = false,
                        bool replace_bits = false) {
    unsigned char pixels[8] = {};
    unsigned char palette[4] = {3, 7, 11, 15};
    IDirectDrawSurface surface = {0, nullptr};
    Buffer owner = {};
    Buffer destination = {};
    owner.surface_ = surface_path ? &surface : nullptr;
    owner.dib_bits_ = absent_bits ? nullptr : pixels;
    owner.locked_bits_ = nullptr;
    owner.surface_lock_count_ = starting_count;
    owner.stride_ = 40;
    owner.dib_.bmiHeader.biWidth = 13;
    owner.dib_.bmiHeader.biHeight = negative_height ? -9 : 9;

    g_BUFFER_SPRITE = {};
    g_BUFFER_SPRITE.pcBits_ = surface_path ? reinterpret_cast<char *>(pixels) : nullptr;
    g_BUFFER_SPRITE.iSpriteWidth2_ = 71;
    g_BUFFER_SPRITE.iSpriteWidth_ = 72;
    g_BUFFER_SPRITE.iSpriteHeight_ = 73;
    g_BUFFER_SPRITE.iWidth_ = 74;
    g_BUFFER_SPRITE.iHeight_ = 75;
    g_BUFFER_SPRITE.iLeftOffset_ = 76;
    g_BUFFER_SPRITE.iTopOffset_ = 77;
    g_BUFFER_SPRITE.cTransparentIndex_ = 78;
    g_BUFFER_SPRITE.fObj1Exists_ = 79;
    seen = {};
    replace_bits_during_call = replace_bits;

    const int integer_table = 0x2ab;
    int result = invoke(owner, route, &destination, transparency, -23, 41,
                        palette, integer_table);
    require(result == (route == 0 ? 0x1357 : route == 1 ? 0x2468 : 0x369c));
    require(seen.route == route);
    require(seen.destination == &destination);
    require(seen.transparent == transparency && seen.x == -23 && seen.y == 41);
    require(seen.table == (route == 0 ? static_cast<uintptr_t>(integer_table)
                                     : reinterpret_cast<uintptr_t>(palette)));

    if (surface_path) {
        // This sibling's established surface path consumes the existing
        // descriptor. It does not publish DIB geometry or unlock it here.
        require(seen.bits == pixels && seen.pitch == 71 && seen.width == 72);
        require(seen.sprite_height == 73 && seen.logical_width == 74);
        require(seen.logical_height == 75 && seen.left == 76 && seen.top == 77);
        require(seen.colour == 78 && seen.object_exists == 79);
        require(surface.unlocks == 0);
    } else {
        require(seen.bits == (absent_bits ? nullptr : pixels));
        require(seen.pitch == 40 && seen.width == 13);
        require(seen.sprite_height == (negative_height ? 9 : -9));
        require(seen.logical_width == 13 &&
                seen.logical_height == (negative_height ? 9 : -9));
        require(seen.left == 0 && seen.top == 0);
        require(seen.colour == (transparency == -1 ? WinFillColour : transparency));
        require(seen.object_exists == 0);
        if (!absent_bits && starting_count > 0) {
            require(owner.surface_lock_count_ == starting_count);
            require(owner.locked_bits_ == pixels);
        } else {
            require(owner.surface_lock_count_ == 0 && owner.locked_bits_ == nullptr);
        }
    }
    // Every method clears after the callee has observed the published pointer.
    require(g_BUFFER_SPRITE.pcBits_ == nullptr);
}

int main() {
    Buffer probe = {};
    require(SpriteNoop().draw_noop(&probe, -1, 0, 0, 0) == 0);
    for (int route = 0; route != 3; ++route) {
        check_route(route, false, false);
        check_route(route, false, true);
        check_route(route, false, false, 0x31);
        check_route(route, false, false, -1, 2);
        check_route(route, false, false, 0x31, 0, true);
        check_route(route, false, false, -1, 0, false, true);
        check_route(route, true, false);
    }
}
'''


def compile_host(tmp_path, methods):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    unit = tmp_path / "buffer_draw_siblings.cpp"
    executable = tmp_path / "buffer_draw_siblings"
    source = HARNESS.replace("ACTUAL_METHODS", "\n\n".join(methods[name] for name in METHODS))
    source = source.replace("ACTUAL_SPRITE_NOOP", methods["sprite_noop_actual"])
    unit.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module")
def actual_executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("buffer-draw-siblings"), actual_source())


def test_actual_buffer_draw_sibling_controls(actual_executable):
    result = subprocess.run([str(actual_executable)], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("method,old,new", [
    ("draw_unk2", "draw_noop(source,", "draw_noop(this,"),
    ("draw", "source, transparent_in, x, y, table)", "source, transparent, x, y, table)"),
    ("draw_dest_unk2", "g_BUFFER_SPRITE.pcBits_ = 0;", "/* missing clear */"),
    ("draw_dest_unk2", "surface_lock_count_ = surface_lock_count_ - 1;\n            if ((int)surface_lock_count_ > 0) {",
     "if ((int)surface_lock_count_ > 0) {"),
    ("draw_dest_unk2", "g_BUFFER_SPRITE.pcBits_ = 0;",
     "if (g_BUFFER_SPRITE.pcBits_ == nullptr) g_BUFFER_SPRITE.pcBits_ = 0;"),
])
def test_actual_buffer_draw_mutations_fail(tmp_path, method, old, new):
    methods = actual_source()
    assert methods[method].count(old) == 1
    methods[method] = methods[method].replace(old, new)
    executable = compile_host(tmp_path, methods)
    result = subprocess.run([str(executable)], capture_output=True,
                           text=True, timeout=10)
    assert result.returncode == 42, result.stderr
