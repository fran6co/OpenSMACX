"""Actual Buffer copy/palette bodies; host callbacks are not native GDI proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
COPY = "int Buffer::copy(Buffer *destination, int src_x, int src_y, int src_w,"
SYNC = "int Buffer::sync_to_palette(Palette *palette)"


def definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
typedef void *HDC;
typedef uint32_t DWORD;
struct RGBQUAD { unsigned char blue, green, red, reserved; };
struct Dib { RGBQUAD bmiColors[256]; };
class Buffer;
class Palette {
public:
    int seed_;
    int get_rgbquad(RGBQUAD *, int, int);
};
static Palette a, b, c;
static Palette *PaletteActive;
static Buffer *source_buffer, *destination_buffer;
static Palette *rgb_palette[3];
static int mutate_phase, color_calls, blits;
static std::vector<std::string> events;
static HDC handle(int value) { return reinterpret_cast<HDC>(static_cast<intptr_t>(value)); }
static void event(char kind, int id) {
    events.push_back(std::string(1, kind) + static_cast<char>('0' + id));
}
static void mutate(int id, int phase) {
    if (mutate_phase == phase) PaletteActive = id == 1 ? &b : &c;
}
class Buffer {
public:
    int id, palette_seed_, has_palette_;
    int gets, releases, references, fail_get;
    void *dib_bits_;
    HDC hdc2_;
    Palette *palette_;
    Dib dib_;
    HDC get_hdc() {
        ++gets; ++references; event('H', id);
        if (gets > 1) mutate(id, 2);
        if (gets == fail_get) { hdc2_ = 0; return 0; }
        hdc2_ = handle(id * 1000 + gets);
        return handle(id * 100 + gets);
    }
    void release_hdc(int count) {
        assert(count == 1); ++releases; event('U', id);
        references -= count;
        if (references <= 0) { references = 0; hdc2_ = 0; }
    }
    int sync_to_palette(Palette *);
    int copy(Buffer *, int, int, int, int, int, int, int, int);
};
static Buffer *owner_of(RGBQUAD *table) {
    if (table == source_buffer->dib_.bmiColors) return source_buffer;
    assert(table == destination_buffer->dib_.bmiColors);
    return destination_buffer;
}
int Palette::get_rgbquad(RGBQUAD *table, int first, int count) {
    assert(first == 0 && count == 0x100);
    Buffer *owner = owner_of(table);
    rgb_palette[owner->id] = this; event('R', owner->id);
    mutate(owner->id, 1);
    return 77;
}
static unsigned int SetDIBColorTable(HDC dc, unsigned int first,
                                    unsigned int count, RGBQUAD *table) {
    Buffer *owner = owner_of(table);
    assert(first == 0 && count == 0x100 && dc == owner->hdc2_);
    assert(dc == handle(owner->id * 1000 + owner->gets));
    ++color_calls; event('C', owner->id); mutate(owner->id, 3);
    return 0;
}
static const DWORD SRCCOPY = 0x00CC0020;
static int StretchBlt(HDC dest, int x, int y, int width, int height,
                      HDC source, int sx, int sy, int sw, int sh, DWORD operation) {
    assert(dest == handle(destination_buffer->id * 100 + 1));
    assert(source == handle(source_buffer->id * 100 +
                           (source_buffer == destination_buffer ? 2 : 1)));
    assert(x == 20 && y == 21 && width == 22 && height == 23);
    assert(sx == 10 && sy == 11 && sw == 12 && sh == 13 && operation == SRCCOPY);
    ++blits; event('B', 0); return 0;
}
// ACTUAL_BODIES
static void reset(Buffer &source, Buffer &destination) {
    source = Buffer(); destination = Buffer();
    source.id = 1; destination.id = 2;
    source.dib_bits_ = handle(1); destination.dib_bits_ = handle(2);
    source_buffer = &source; destination_buffer = &destination;
    a.seed_ = 11; b.seed_ = 22; c.seed_ = 33; PaletteActive = &a;
    rgb_palette[1] = rgb_palette[2] = 0;
    mutate_phase = color_calls = blits = 0; events.clear();
}
static int copy(Buffer &source, Buffer *destination) {
    return source.copy(destination, 10, 11, 12, 13, 20, 21, 22, 23);
}
int main() {
    Buffer source, destination;
    reset(source, destination);
    assert(copy(source, 0) == 0x10 && events.empty());
    reset(source, destination); destination.fail_get = 1;
    assert(copy(source, &destination) == 7 && source.gets == 0);
    assert(destination.references == 1 && destination.releases == 0);
    reset(source, destination); source.fail_get = 1;
    assert(copy(source, &destination) == 7 && source.references == 1);
    assert(destination.references == 0 && destination.releases == 1 && blits == 0);

    // A refresh keeps its selected palette across each kind of callback.
    // The next buffer captures the new global, independently of the first.
    for (int phase = 0; phase <= 3; ++phase) {
        reset(source, destination); mutate_phase = phase;
        assert(copy(source, &destination) == 0);
        assert(source.palette_ == &a && rgb_palette[1] == &a);
        assert(destination.palette_ == (phase ? &b : &a));
        assert(rgb_palette[2] == destination.palette_);
        assert(PaletteActive == (phase ? &c : &a));
        assert(source.palette_seed_ == a.seed_);
        assert(destination.palette_seed_ == (phase ? b.seed_ : a.seed_));
        assert(source.has_palette_ == 1 && destination.has_palette_ == 1);
        assert(source.references == 0 && destination.references == 0);
        assert(source.gets == 2 && source.releases == 2);
        assert(destination.gets == 2 && destination.releases == 2);
        const char *order[] = {"H2", "H1", "R1", "H1", "C1", "U1",
                               "R2", "H2", "C2", "U2", "B0", "U2", "U1"};
        assert(events.size() == 13);
        for (unsigned int n = 0; n < 13; ++n) assert(events[n] == order[n]);
    }
    reset(source, destination); PaletteActive = 0;
    assert(copy(source, &destination) == 0 && color_calls == 0);
    assert(!source.has_palette_ && !destination.has_palette_);
    assert(source.gets == 1 && destination.gets == 1);
    reset(source, destination); source.dib_bits_ = 0;
    assert(copy(source, &destination) == 0 && !source.has_palette_);
    assert(destination.palette_ == &a && source.gets == 1 && destination.gets == 2);
    reset(source, destination); destination.dib_bits_ = 0;
    assert(copy(source, &destination) == 0 && !destination.has_palette_);
    assert(source.palette_ == &a && source.gets == 2 && destination.gets == 1);
    reset(source, destination);
    source.palette_seed_ = destination.palette_seed_ = a.seed_;
    assert(copy(source, &destination) == 0 && color_calls == 0);
    assert(source.palette_ == &a && destination.palette_ == &a);
    assert(source.gets == 1 && destination.gets == 1);
    reset(source, destination); source.fail_get = 2;
    assert(copy(source, &destination) == 0 && color_calls == 1);
    assert(source.palette_ == &a && source.references == 1 && source.releases == 1);
    assert(destination.references == 0);
    reset(source, destination);
    destination_buffer = &source;
    assert(copy(source, &source) == 0 && source.palette_ == &a);
    assert(source.gets == 3 && source.releases == 3 && source.references == 0);
    return 0;
}
"""


def program(source):
    bodies = definition(source, SYNC) + "\n" + definition(source, COPY)
    return HARNESS.replace("// ACTUAL_BODIES", bodies)


def run(tmp_path, compiler, source, name):
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True)
    return subprocess.run([str(exe)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_copy_palette_capture_and_callback_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = program((ROOT / "src/buffer.cpp").read_text())
    variants = [("owner", source, True)]
    for name, old, new in [
        ("late-publish", "palette_ = palette;", "palette_ = PaletteActive;"),
        ("shared-capture", "destination->sync_to_palette(PaletteActive);",
         "destination->sync_to_palette(palette_);"),
        ("omit-nested-release", "release_hdc(1);", ";"),
    ]:
        assert old in source
        variants.append((name, source.replace(old, new, 1), False))
    for name, candidate, should_pass in variants:
        result = run(tmp_path, compiler, candidate, name)
        assert (result.returncode == 0) == should_pass, result.stderr
        if not should_pass:
            assert b"assert" in result.stderr.lower(), result.stderr
