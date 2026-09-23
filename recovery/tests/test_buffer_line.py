"""Actual Buffer line rasterization with original-derived bounded controls.

Original 5E15F0..5E1A7F establishes x normalization, strict-positive error
steps, inclusive endpoints, half-open clip edges and acquire/release ordering.
The seed's generic algorithm is not the oracle. A closed-form rational pixel
oracle is independent of the production accumulator/entry-clipping loops.
Actual hline/vline bodies exercise axis pixels; a separate line-only harness
uses recording delegates to observe the otherwise invisible dispatch choice.

The get_data/free_data doubles model their ownership contract, not DirectDraw
or native ABI. Integer overflow, wrapped native addresses, pathological huge
coordinates and concurrent callbacks are outside the bounded pixel controls.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError('unterminated actual body')


def actual_bodies():
    text = (ROOT / 'src/buffer.cpp').read_text()
    return {name: braced(text, f'void Buffer::{name}(')
            for name in ('line', 'hline', 'vline')}


def actual_fields():
    header = (ROOT / 'src/buffer.h').read_text()
    fields = []
    for name in ('rect1_', 'locked_bits_', 'dib_bits_', 'surface_',
                 'surface_lock_count_', 'stride_'):
        field, = re.findall(r'^\s*(?:RECT|LPVOID|IDirectDrawSurface\s*\*|'
                            r'int|uint32_t)\s*' + name + r'\s*;', header, re.M)
        fields.append(field.strip())
    return '\n'.join(fields)


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
typedef void *LPVOID;
struct RECT { int left, top, right, bottom; };
void require_at(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(91); }
}
#define require(x) require_at((x), __LINE__)
struct IDirectDrawSurface { bool failure, null_bits, change_state; };
std::string events;
unsigned char allocation[64 * 64 + 256], expected[64 * 64 + 256];
unsigned char *const pixels = allocation + 128;
struct Delegate { char kind; int a, b, c, colour; };
std::vector<Delegate> delegates;
struct Buffer {
    // ACTUAL_FIELDS
    // Only these DIB dimensions participate in the actual axis primitives.
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
    void line(int, int, int, int, int);
    void hline(int, int, int, int);
    void vline(int, int, int, int);
    // The target returns its 32-bit pointer in int; this host double returns
    // intptr_t because the callers observe only its zero/nonzero value.
    intptr_t get_data() {
        events += 'G';
        if (!surface_) {
            locked_bits_ = dib_bits_;
            if (locked_bits_) ++surface_lock_count_;
            return reinterpret_cast<intptr_t>(locked_bits_);
        }
        if (locked_bits_) {
            ++surface_lock_count_;
            return reinterpret_cast<intptr_t>(locked_bits_);
        }
        events += 'L';
        if (surface_->failure) return 0;
        ++surface_lock_count_;
        locked_bits_ = surface_->null_bits ? nullptr : pixels;
        if (surface_->change_state) {
            rect1_ = {12, 13, 24, 29};
            stride_ = 64;
        }
        return reinterpret_cast<intptr_t>(locked_bits_);
    }
    void free_data(int count) {
        require(count == 1);
        events += 'F';
        --surface_lock_count_;
        if (surface_lock_count_ <= 0) {
            if (surface_ && locked_bits_) events += 'U';
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
    }
};
// ACTUAL_LINE
// AXIS_BODIES

Buffer fresh() {
    std::memset(allocation, 0xa5, sizeof allocation);
    std::memcpy(expected, allocation, sizeof allocation);
    events.clear(); delegates.clear();
    Buffer b = {};
    b.dib_bits_ = pixels;
    b.stride_ = 64;
    b.rect1_ = {8, 8, 40, 40};
    b.dib_.bmiHeader.biWidth = 64;
    b.dib_.bmiHeader.biHeight = -64;
    return b;
}

void expected_pixel(int x, int y, int colour, RECT clip) {
    if (x < clip.left || x >= clip.right || y < clip.top || y >= clip.bottom)
        return;
    require(x >= 0 && x < 64 && y >= 0 && y < 64);
    expected[128 + y * 64 + x] = static_cast<unsigned char>(colour);
}

void oracle(int x1, int y1, int x2, int y2, int colour, RECT clip) {
    if (x1 == x2 && y1 == y2) return; // vertical delegate rejects a point
    if (x1 == x2) {
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y)
            expected_pixel(x1, y, colour, clip);
        return;
    }
    if (y1 == y2) {
        for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x)
            expected_pixel(x, y1, colour, clip);
        return;
    }
    if (x2 < x1) { std::swap(x1, x2); std::swap(y1, y2); }
    const int dx = x2 - x1, dy = std::abs(y2 - y1);
    const int major = std::max(dx, dy), minor = std::min(dx, dy);
    const int sign = y2 > y1 ? 1 : -1;
    if (x1 > clip.right || x2 < clip.left ||
        (sign > 0 ? y1 > clip.bottom || y2 < clip.top
                  : y1 < clip.top || y2 > clip.bottom)) return;
    int first = 0;
    for (;;) {
        const int steps = (2 * first * minor + major - 1) / (2 * major);
        const int x = x1 + (dx >= dy ? first : steps);
        const int y = y1 + sign * (dx >= dy ? steps : first);
        if (x >= clip.left && (sign > 0 ? y >= clip.top : y < clip.bottom))
            break;
        require(++first < 1024);
    }
    // The original writes this entry pixel BEFORE testing the endpoint.
    // In particular descending y2==clip.bottom may advance beyond the
    // supplied segment and still emit one pixel. Do not "fix" that quirk.
    for (int n = first; n <= std::max(first, major); ++n) {
        // Nearest minor coordinate with exact half ties down. Unlike the
        // product, no error accumulator, skip-to-clip walk or endpoint clamp.
        const int steps = (2 * n * minor + major - 1) / (2 * major);
        const int x = x1 + (dx >= dy ? n : steps);
        const int y = y1 + sign * (dx >= dy ? steps : n);
        expected_pixel(x, y, colour, clip);
    }
}

void compare_pixels() {
    require(std::memcmp(allocation, expected, sizeof allocation) == 0);
}

void geometry() {
    // Bounded exhaustive endpoint grid includes every octant, reversed
    // endpoints, equal/near slopes and segments wholly/partly outside clip.
    const int coordinate[] = {-3, 3, 7, 8, 9, 15, 23, 31, 39, 40, 47};
    for (int x1 : coordinate) for (int y1 : coordinate)
    for (int x2 : coordinate) for (int y2 : coordinate) {
        Buffer b = fresh();
        oracle(x1, y1, x2, y2, 0x17d, b.rect1_);
        b.line(x1, y1, x2, y2, 0x17d);
        compare_pixels();
        require(b.surface_lock_count_ == 0 && b.locked_bits_ == nullptr);
    }
    // Explicit original branch anchors for strict-positive ties. Both
    // diagonal directions include the final endpoint and truncate colour.
    for (int descending = 0; descending < 2; ++descending) {
        Buffer b = fresh();
        const int ys[5] = {10, 10, 11, 11, 12};
        for (int i = 0; i < 5; ++i)
            expected_pixel(10 + i, descending ? 22 - ys[i] : ys[i], -2, b.rect1_);
        b.line(10, descending ? 12 : 10, 14, descending ? 10 : 12, -2);
        compare_pixels();
    }
    // Clip entry retains the original phase; clipping must not start a new
    // Bresenham line between independently clipped geometric endpoints.
    for (int left = 8; left <= 13; ++left) {
        Buffer b = fresh(); b.rect1_ = {left, 10, 24, 22};
        oracle(-3, 5, 31, 23, 61, b.rect1_);
        b.line(-3, 5, 31, 23, 61); compare_pixels();
    }
    {
        Buffer b = fresh(); b.rect1_ = {0, 0, 10, 1};
        expected_pixel(2, 0, 73, b.rect1_);
        b.line(0, 2, 1, 1, 73); compare_pixels();
    }
}

void lifecycle() {
    for (int mode = 0; mode < 8; ++mode) {
        Buffer b = fresh();
        IDirectDrawSurface surface = {};
        if (mode != 0) { b.surface_ = &surface; b.dib_bits_ = nullptr; }
        if (mode == 2) { b.locked_bits_ = pixels; b.surface_lock_count_ = 2; }
        if (mode == 3) surface.failure = true;
        if (mode == 4) surface.null_bits = true;
        if (mode == 5) { surface.change_state = true; b.stride_ = 56; }
        if (mode == 6) { b.surface_ = nullptr; b.dib_bits_ = nullptr; }
        if (mode == 7) { b.surface_ = nullptr; b.dib_bits_ = pixels;
                         b.locked_bits_ = pixels; b.surface_lock_count_ = 2; }
        RECT clip = mode == 5 ? RECT{12, 13, 24, 29} : b.rect1_;
        if (mode != 3 && mode != 4 && mode != 6)
            oracle(5, 6, 32, 30, 81, clip);
        b.line(5, 6, 32, 30, 81); compare_pixels();
        const char *want[] = {"GF", "GLFU", "GF", "GL", "GL", "GLFU", "", "GF"};
        require(events == want[mode]);
        require(b.surface_lock_count_ == ((mode == 2 || mode == 7) ? 2 : mode == 4 ? 1 : 0));
        require(b.locked_bits_ == ((mode == 2 || mode == 7) ? pixels : nullptr));
    }
    // Every acquired diagonal rejection releases once, even before drawing.
    const int rejected[][4] = {{1, 1, 3, 4}, {41, 8, 44, 20},
                              {8, 41, 20, 44}, {1, 12, 3, 8}};
    for (const auto &segment : rejected) {
        Buffer b = fresh(); IDirectDrawSurface surface = {};
        b.surface_ = &surface;
        b.line(segment[0], segment[1], segment[2], segment[3], 81);
        require(events == "GLFU"); compare_pixels();
        require(b.surface_lock_count_ == 0 && b.locked_bits_ == nullptr);
    }
}

void dispatch() {
    Buffer b = fresh();
    b.line(12, 20, 12, 9, 0x123);
    require(delegates.size() == 1 && events.empty());
    require(delegates[0].kind == 'V' && delegates[0].a == 12 &&
            delegates[0].b == 20 && delegates[0].c == 9 && delegates[0].colour == 0x123);
    delegates.clear();
    b.line(24, 15, 9, 15, -2);
    require(delegates.size() == 1 && delegates[0].kind == 'H');
    require(delegates[0].a == 24 && delegates[0].b == 9 &&
            delegates[0].c == 15 && delegates[0].colour == -2);
    delegates.clear();
    b.line(12, 12, 12, 12, 91);
    require(delegates.size() == 1 && delegates[0].kind == 'V');
    delegates.clear(); b.dib_bits_ = nullptr;
    b.line(12, 20, 12, 9, 81);
    b.line(24, 15, 9, 15, 81);
    require(delegates.empty() && events.empty());
}

int main(int argc, char **argv) {
    require(argc == 2);
    const std::string mode = argv[1];
    if (mode == "geometry") geometry();
    else if (mode == "lifecycle") lifecycle();
    else if (mode == "dispatch") dispatch();
    else require(false);
}
'''


def compile_host(tmp_path, bodies, *, delegates=False):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    axis = '\n'.join(bodies[name] for name in ('hline', 'vline'))
    if delegates:
        axis = '''
void Buffer::hline(int a, int b, int c, int colour) {
    ::delegates.push_back({'H', a, b, c, colour});
}
void Buffer::vline(int a, int b, int c, int colour) {
    ::delegates.push_back({'V', a, b, c, colour});
}
'''
    text = HARNESS.replace('// ACTUAL_FIELDS', actual_fields())
    text = text.replace('// ACTUAL_LINE', bodies['line']).replace('// AXIS_BODIES', axis)
    cpp, exe = tmp_path / 'buffer_line.cpp', tmp_path / 'buffer_line'
    cpp.write_text(text)
    built = subprocess.run([compiler, '-std=c++11', '-O1', '-g',
                            '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                            '-fno-omit-frame-pointer', '-no-pie', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    return exe


def run(exe, mode):
    # The traced runner prevents LeakSanitizer's thread enumeration. Memory
    # access and UB instrumentation remain enabled for every actual body.
    return subprocess.run([str(exe), mode], capture_output=True, text=True,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'), timeout=30)


@pytest.fixture(scope='module')
def executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp('buffer-line'), actual_bodies())


@pytest.mark.parametrize('mode', ['geometry', 'lifecycle'])
def test_actual_line_pixels_and_lifecycle(executable, mode):
    result = run(executable, mode)
    assert result.returncode == 0, result.stderr


def test_actual_line_axis_dispatch(tmp_path):
    exe = compile_host(tmp_path, actual_bodies(), delegates=True)
    result = run(exe, 'dispatch')
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('kind,mode', [
    ('ties', 'geometry'), ('paired_swap', 'geometry'),
    ('endpoint', 'geometry'), ('right_clip', 'geometry'),
    ('phase', 'geometry'), ('release', 'lifecycle'),
    ('rejected_release', 'lifecycle'), ('failed_acquire_release', 'lifecycle'),
    ('early_clip_snapshot', 'lifecycle'), ('early_pitch_snapshot', 'lifecycle'),
    ('vertical_dispatch', 'dispatch'), ('horizontal_arguments', 'dispatch'),
    ('missing_storage_gate', 'dispatch'),
])
def test_line_damage_is_observed(tmp_path, kind, mode):
    bodies = actual_bodies()
    original = bodies['line']

    def change(old, new, *, count=None):
        assert old in bodies['line'], old
        bodies['line'] = bodies['line'].replace(old, new, count or -1)

    if kind == 'ties':
        change('if (error > 0)', 'if (error >= 0)')
    elif kind == 'paired_swap':
        change('swap = y1;', 'swap = y2;')
    elif kind == 'endpoint':
        change('x2 < clip.right ? x2 : clip.right - 1',
               'x2 < clip.right ? x2 - 1 : clip.right - 1')
    elif kind == 'right_clip':
        change('const RECT clip = rect1_;', 'RECT clip = rect1_; ++clip.right;')
    elif kind == 'phase':
        change('            if (x >= clip.right ||',
               '            error = 2 * dy - dx;\n            if (x >= clip.right ||',
               count=1)
    elif kind == 'release':
        change('    free_data(1);', '')
    elif kind == 'rejected_release':
        change('y2 > clip.bottom))) {\n            break;',
               'y2 > clip.bottom))) {\n            return;')
    elif kind == 'failed_acquire_release':
        change('if (get_data() == 0) {\n        return;',
               'if (get_data() == 0) {\n        free_data(1);\n        return;')
    elif kind in ('early_clip_snapshot', 'early_pitch_snapshot'):
        declaration = ('const RECT clip = rect1_;' if kind == 'early_clip_snapshot'
                       else 'const int pitch = static_cast<int>(stride_);')
        change('    ' + declaration, '')
        change('    if (get_data() == 0)',
               '    ' + declaration + '\n    if (get_data() == 0)')
    elif kind == 'vertical_dispatch':
        change('vline(x1, y1, y2, color);', 'hline(x1, y1, y2, color);')
    elif kind == 'horizontal_arguments':
        change('hline(x1, x2, y1, color);', 'hline(x2, x1, y1, color);')
    elif kind == 'missing_storage_gate':
        change('if (dib_bits_ == nullptr && surface_ == nullptr)', 'if (false)')
    else:
        raise AssertionError(kind)
    assert bodies['line'] != original
    exe = compile_host(tmp_path, bodies, delegates=mode == 'dispatch')
    result = run(exe, mode)
    assert result.returncode == 91, result.stderr
