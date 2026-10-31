"""Original 493CC9..493CDF word arithmetic, including negative INT_MIN distance.

Only the actual scroll body is projected. Drawing doubles observe callbacks;
this does not establish native layout, rendering, or linked selection.
"""
from pathlib import Path
import shutil
import subprocess

import pytest
from test_prodpicker_scroll_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
void require(bool ok) { if (!ok) std::exit(109); }
struct RECT { int unused; };
struct ProdPicker {
    int32_t field_A77C_, column_count_, cell_width_ = 0, row_height_ = 0;
    uint32_t field_A814_ = 1;
    RECT rect2_;
    std::vector<char> events;
    std::vector<int> drawn;
    void on_scrolled(int, int position);
    void redraw() { events.push_back('R'); }
    void soft_update() { events.push_back('S'); }
    void set_clip(RECT *area) { require(area == &rect2_); events.push_back('C'); }
    void copy(ProdPicker *source, int sx, int sy, int dx, int dy, int w, int h) {
        require(source == this && sx == 21 && sy == 4 && dx == 21 && dy == 4);
        require(w == 4 && h == 0); events.push_back('B');
    }
    void draw_item(int item, int update) {
        require(update == 0 && drawn.size() < 4);
        drawn.push_back(item); events.push_back('D');
    }
};
ACTUAL_BODY
// Mathematical modulo oracle avoids the product/subtraction's signed UB.
int32_t word(int64_t n) {
    int64_t v = n % INT64_C(4294967296);
    if (v < 0) v += INT64_C(4294967296);
    if (v >= INT64_C(2147483648)) v -= INT64_C(4294967296);
    return static_cast<int32_t>(v);
}
void check(int32_t old, int32_t columns, int position,
           std::vector<char> expected, std::vector<int> items) {
    ProdPicker picker;
    picker.field_A77C_ = old; picker.column_count_ = columns;
    const int32_t published = word(int64_t(columns) * position);
    picker.on_scrolled(0, position);
    require(picker.field_A77C_ == published);
    require(picker.events == expected && picker.drawn == items);
}
int main() {
    // SUB then reverse-SUB both leave 80000000 negative. Signed JLE admits
    // the small-scroll path; saturating/unsigned absolute value would not.
    check(INT32_MIN, 0, 0, {'C','B','S'}, {});
    check(0, 2, INT32_MAX, {'C','B','D','D','S'}, {-2,-1});
    check(INT32_MAX, 2, INT32_MAX, {'R','S'}, {});
    check(INT32_MAX, 0, 0, {'R','S'}, {});
}
'''


def run(tmp_path, compiler_name, damage=None):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + ' unavailable')
    body = extract_function((ROOT / 'src/prodpicker.cpp').read_text())
    if damage:
        before, after = damage
        assert body.count(before) == 1, 'mutation must hit the actual body once'
        changed = body.replace(before, after, 1)
        assert changed != body
        body = changed
    source = tmp_path / 'scroll-word.cpp'
    source.write_text(HARNESS.replace('ACTUAL_BODY', body))
    binary = tmp_path / 'scroll-word'
    built = subprocess.run([compiler, '-std=c++17', '-O1', '-Wall', '-Wextra',
        '-Werror', '-fsanitize=undefined', '-fno-sanitize-recover=all',
        str(source), '-o', str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=5)


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_scroll_native_word_arithmetic(tmp_path, compiler):
    result = run(tmp_path, compiler)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('before,after', [
    ('static_cast<uint32_t>(pageSize)\n        * static_cast<uint32_t>(position)',
     'static_cast<uint32_t>(pageSize * position)'),
    ('int distance = static_cast<int32_t>(newPositionBits\n        - static_cast<uint32_t>(oldPosition));',
     'int distance = newPosition - oldPosition;'),
    ('distance = static_cast<int32_t>(static_cast<uint32_t>(oldPosition)\n            - newPositionBits);',
     'distance = -distance;'),
    ('if (distance > pageSize)',
     'if (static_cast<uint32_t>(distance) > static_cast<uint32_t>(pageSize))'),
    ('if (distance > pageSize)',
     'if (distance == INT32_MIN) distance = INT32_MAX;\n    if (distance > pageSize)'),
])
def test_original_overflow_and_negative_distance_damage(tmp_path, before, after):
    result = run(tmp_path, 'clang++', (before, after))
    assert result.returncode != 0, 'damaged actual-source arithmetic must fail'
