"""Actual-source host controls for Buffer's two table-draw wrappers.

These controls compile the complete product bodies with narrow field and
Sprite/DirectDraw doubles. They exercise lifecycle and forwarding behavior;
they do not model the VC6 ABI or establish game acceptance.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
METHODS = (
    "draw_dest",
    "draw_multi_table_dest",
)


def actual_methods():
    source = (ROOT / "src/buffer.cpp").read_text()
    methods = {}
    for name in METHODS:
        signatures = list(re.finditer(
            r"\bint\s+Buffer::" + name + r"\s*\([^)]*\)", source))
        signatures = [match for match in signatures
                      if match.group(0).count(",") == 6]
        if name == "draw_dest":
            signatures = [match for match in signatures
                          if "unsigned char *" in match.group(0)]
        else:
            signatures = [match for match in signatures
                          if "ColorTable *" in match.group(0)]
        assert len(signatures) == 1, "missing or ambiguous Buffer::" + name
        signature = signatures[0]
        opening = source.index("{", signature.start())
        depth = 0
        for end in range(opening, len(source)):
            depth += (source[end] == "{") - (source[end] == "}")
            if depth == 0:
                methods[name] = source[signature.start():end + 1]
                break
        else:
            raise AssertionError("unterminated actual Buffer::" + name)
    return methods


HARNESS = r'''#include <cstdint>
#include <cstdlib>

void require(bool ok) { if (!ok) std::abort(); }
using LPVOID = void *;
using ColorTable = unsigned char[256];
const int WinFillColour = 13;

struct Buffer;
struct Observation {
    int route = 0;
    Buffer *destination = nullptr;
    int args[5] = {};
    const void *table = nullptr;
    void *bits = nullptr;
    unsigned int pitch = 0, width = 0;
    int sprite_height = 0, logical_width = 0, logical_height = 0;
    unsigned int left = 0, top = 0, colour = 0, object_exists = 0;
    int result = 0;
} seen;

struct Sprite {
    unsigned char *pcBits_;
    unsigned int iSpriteWidth2_, iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_, iHeight_;
    unsigned int iLeftOffset_, iTopOffset_;
    char cTransparentIndex_;
    unsigned int fObj1Exists_;

    void snapshot(int route, Buffer *destination, int a2, int a3, int a4,
                  int a5, int a6, const void *table) {
        seen.route = route;
        seen.destination = destination;
        seen.args[0] = a2; seen.args[1] = a3; seen.args[2] = a4;
        seen.args[3] = a5; seen.args[4] = a6;
        seen.table = table;
        seen.bits = pcBits_;
        seen.pitch = iSpriteWidth2_; seen.width = iSpriteWidth_;
        seen.sprite_height = iSpriteHeight_;
        seen.logical_width = iWidth_; seen.logical_height = iHeight_;
        seen.left = iLeftOffset_; seen.top = iTopOffset_;
        seen.colour = static_cast<unsigned char>(cTransparentIndex_);
        seen.object_exists = fObj1Exists_;
    }
    int draw_dest(Buffer *destination, int a2, int a3, int a4, int a5,
                  int a6, unsigned char *table) {
        snapshot(1, destination, a2, a3, a4, a5, a6, table);
        return seen.result = 0x2468;
    }
    int draw_multi_table_dest(Buffer *destination, int a2, int a3, int a4,
                              int a5, int a6, ColorTable *table) {
        snapshot(2, destination, a2, a3, a4, a5, a6, table);
        return seen.result = 0x369c;
    }
};
static Sprite g_sprite = {};
struct SpriteOwner { Sprite &sprite() { return g_sprite; } };
static SpriteOwner BufferSpriteOwner;

struct IDirectDrawSurface {
    int unlocks = 0;
    void *unlocked = nullptr;
    void Unlock(void *bits) { ++unlocks; unlocked = bits; }
};
struct Dib { struct { int biWidth, biHeight; } bmiHeader; };
struct Buffer {
    void *locked_bits_;
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    int surface_lock_count_;
    unsigned int stride_;
    Dib dib_;
    int draw_dest(Buffer *, int, int, int, int, int, unsigned char *);
    int draw_multi_table_dest(Buffer *, int, int, int, int, int, ColorTable *);
};

ACTUAL_METHODS

static void reset_sprite() {
    g_sprite = {};
    g_sprite.pcBits_ = reinterpret_cast<unsigned char *>(uintptr_t(0x1110));
    g_sprite.iSpriteWidth2_ = 71; g_sprite.iSpriteWidth_ = 72;
    g_sprite.iSpriteHeight_ = 73; g_sprite.iWidth_ = 74;
    g_sprite.iHeight_ = 75; g_sprite.iLeftOffset_ = 76;
    g_sprite.iTopOffset_ = 77; g_sprite.cTransparentIndex_ = 78;
    g_sprite.fObj1Exists_ = 79;
}

static int invoke(int route, Buffer &owner, Buffer *destination, int a2,
                  int a3, int a4, int a5, int a6,
                  unsigned char *table, ColorTable *multi_table) {
    if (route == 1)
        return owner.draw_dest(destination, a2, a3, a4, a5, a6, table);
    return owner.draw_multi_table_dest(destination, a2, a3, a4, a5, a6,
                                       multi_table);
}

static void check_setup_path(int route, bool has_bits, int initial_count) {
    unsigned char pixels[16] = {};
    unsigned char table_bytes[8] = {};
    ColorTable *multi_table = reinterpret_cast<ColorTable *>(table_bytes);
    IDirectDrawSurface surface;
    Buffer owner = {};
    Buffer destination = {};
    owner.surface_ = nullptr;
    owner.dib_bits_ = has_bits ? pixels : nullptr;
    owner.surface_lock_count_ = initial_count;
    owner.stride_ = 40;
    owner.dib_.bmiHeader.biWidth = 13;
    owner.dib_.bmiHeader.biHeight = 9;
    reset_sprite();
    seen = {};

    const int result = invoke(route, owner, &destination, -1, 21, -22, 33,
                              -44, table_bytes, multi_table);
    require(result == (route == 1 ? 0x2468 : 0x369c));
    require(seen.route == route && seen.destination == &destination);
    require(seen.args[0] == -1 && seen.args[1] == 21 &&
            seen.args[2] == -22 && seen.args[3] == 33 &&
            seen.args[4] == -44);
    require(seen.table == (route == 1 ? static_cast<const void *>(table_bytes)
                                      : static_cast<const void *>(multi_table)));
    require(seen.bits == (has_bits ? static_cast<void *>(pixels) : nullptr));
    require(seen.pitch == 40 && seen.width == 13 && seen.sprite_height == -9);
    require(seen.logical_width == 13 && seen.logical_height == -9);
    require(seen.left == 0 && seen.top == 0 && seen.colour == WinFillColour);
    require(seen.object_exists == 0);
    require(g_sprite.pcBits_ == nullptr);

    const int after_increment = initial_count + (has_bits ? 1 : 0);
    const int after_decrement = after_increment - 1;
    if (after_decrement > 0) {
        require(owner.surface_lock_count_ == after_decrement);
        require(owner.locked_bits_ == (has_bits ? static_cast<void *>(pixels)
                                                : nullptr));
    } else {
        require(owner.surface_lock_count_ == 0 && owner.locked_bits_ == nullptr);
    }
    require(surface.unlocks == 0);
}

static void check_existing_surface_skips_setup(int route) {
    unsigned char pixels[16] = {};
    unsigned char table_bytes[8] = {};
    ColorTable *multi_table = reinterpret_cast<ColorTable *>(table_bytes);
    IDirectDrawSurface surface;
    Buffer owner = {};
    Buffer destination = {};
    owner.surface_ = &surface;
    owner.dib_bits_ = pixels;
    owner.locked_bits_ = pixels;
    owner.surface_lock_count_ = 7;
    owner.stride_ = 40;
    owner.dib_.bmiHeader.biWidth = 13;
    owner.dib_.bmiHeader.biHeight = 9;
    reset_sprite();
    seen = {};

    int result = invoke(route, owner, &destination, 9, 21, -22, 33, -44,
                        table_bytes, multi_table);
    require(result == (route == 1 ? 0x2468 : 0x369c));
    require(seen.route == route && seen.destination == &destination);
    require(seen.table == (route == 1 ? static_cast<const void *>(table_bytes)
                                      : static_cast<const void *>(multi_table)));
    require(seen.bits == reinterpret_cast<void *>(uintptr_t(0x1110)));
    require(seen.pitch == 71 && seen.width == 72 && seen.sprite_height == 73);
    require(seen.logical_width == 74 && seen.logical_height == 75);
    require(seen.left == 76 && seen.top == 77 && seen.colour == 78);
    require(seen.object_exists == 79);
    require(owner.surface_lock_count_ == 7 && owner.locked_bits_ == pixels);
    require(surface.unlocks == 0 && g_sprite.pcBits_ == nullptr);
}

int main() {
    const int routes[] = {1, 2};
    const int counts[] = {2, 1, 0, -1};
    for (int route : routes) {
        for (int bits = 0; bits != 2; ++bits) {
            for (int count : counts)
                check_setup_path(route, bits != 0, count);
        }
        check_existing_surface_skips_setup(route);
    }
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_table_draw_bodies_preserve_setup_and_forwarding(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    methods = actual_methods()
    source = HARNESS.replace("ACTUAL_METHODS", "\n\n".join(
        methods[name] for name in METHODS))
    unit = tmp_path / "buffer_table_draw_actual.cpp"
    executable = tmp_path / "buffer_table_draw_actual"
    unit.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all", str(unit),
         "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    subprocess.run([str(executable)], check=True, timeout=10)
