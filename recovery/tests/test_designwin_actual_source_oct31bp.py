"""Pinned switch mappings and callback lifetime controls for actual owner bodies."""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def definition(signature, address):
    path = ROOT / 'src/designwin.cpp'
    record, = (row for row in read_file(path) if row.address == address)
    assert record.image_spans
    source = path.read_text()
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual definition')


def run_cpp(tmp_path, source):
    compiler = shutil.which('clang++') or shutil.which('g++')
    assert compiler
    cpp = tmp_path / 'actual.cpp'
    exe = tmp_path / 'actual'
    cpp.write_text(source)
    result = subprocess.run([compiler, '-std=c++11', '-fsanitize=address,undefined',
                             '-fno-sanitize-recover=all', str(cpp), '-o', str(exe)],
                            text=True, capture_output=True)
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True,
                          env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=0'})


KEY_HARNESS = r'''
#include <cassert>
#include <vector>
struct Tutorial { int resets; void reset() { ++resets; } } TutorialWindow;
int ProtoSortCount = 40;
struct DesignWin {
 int prototype_stack_start_;
 int draws;
 std::vector<int> calls;
 virtual void on_button_clicked(int action) { calls.push_back(action); }
 void draw_stack() { ++draws; }
 int on_key_click(int, int);
};
// ACTUAL_KEY
int main() {
 for (int first = -5; first < 128; ++first) {
  for (int second = -5; second < 128; ++second) {
   DesignWin w; w.prototype_stack_start_ = 12; w.draws = 0;
   int resets = TutorialWindow.resets;
   assert(w.on_key_click(first, second) == 1);
   assert(TutorialWindow.resets == resets + 1);
   std::vector<int> expected;
   if (second == 13) expected.push_back(4);
   if (second == 27) expected.push_back(8);
   if (second == 37 || second == 100) expected.push_back(23);
   if (second == 39 || second == 102) expected.push_back(24);
   if (first == 77 || first == 79 || first == 109 || first == 111) expected.push_back(6);
   if (first == 85 || first == 117) expected.push_back(7);
   assert(w.calls == expected);
   int anchor = 12, draws = 0;
   if (second == 33 || second == 105) { anchor = 3; draws = 1; }
   if (second == 34 || second == 99) { anchor = 21; draws = 1; }
   if (second == 35 || second == 97) { anchor = 30; draws = 1; }
   if (second == 36 || second == 103) { anchor = 0; draws = 1; }
   assert(w.prototype_stack_start_ == anchor && w.draws == draws);
  }
 }
}
'''


@pytest.mark.parametrize('damage', ['none', 'left_action', 'wrong_argument', 'lost_reset'])
def test_exhaustive_keyboard_mapping_and_independent_dispatch(tmp_path, damage):
    body = definition('int DesignWin::on_key_click(', 0x439A20)
    changes = {'left_action': ('on_button_clicked(0x17)', 'on_button_clicked(0x18)'),
               'wrong_argument': ('switch (a1)', 'switch (a2)'),
               'lost_reset': ('TutorialWindow.reset();', '')}
    if damage != 'none':
        old, new = changes[damage]
        assert body.count(old) == 1
        body = body.replace(old, new)
    result = run_cpp(tmp_path, KEY_HARNESS.replace('// ACTUAL_KEY', body))
    assert (result.returncode == 0) == (damage == 'none'), result.stdout + result.stderr


CAPTURE_HARNESS = r'''
#include <cassert>
struct SelectPartWin;
SelectPartWin *WinPointerOwner3, *WinPointerOwner4;
struct SelectPartWin {
 int field_4648_, field_2B80_, losses, releases, observed;
 SelectPartWin *replacement;
 void on_mouse_move(int, int) {}
 void on_lose_mouse_capture() {
  ++losses; observed = field_2B80_;
  assert(WinPointerOwner3 == 0);
  WinPointerOwner3 = this;
  WinPointerOwner4 = replacement;
 }
 void release_modal() { ++releases; }
 void on_left_up(int, int);
};
// ACTUAL_CAPTURE
int main() {
 SelectPartWin other;
 for (int selected = 0; selected != 2; ++selected) {
  for (int replace = 0; replace != 2; ++replace) {
   SelectPartWin w;
   w.field_4648_ = selected; w.field_2B80_ = 9;
   w.losses = w.releases = 0; w.replacement = replace ? &other : &w;
   WinPointerOwner3 = WinPointerOwner4 = &w;
   w.on_left_up(3, 7);
   assert(w.losses == 1 && w.releases == 1);
   assert(w.observed == (selected ? 9 : -1));
   assert(WinPointerOwner3 == &w);
   assert(WinPointerOwner4 == (replace ? &other : 0));
  }
 }
}
'''


def test_capture_callback_preserves_new_owner_and_reloads_secondary_owner(tmp_path):
    body = definition('void SelectPartWin::on_left_up(', 0x433940)
    result = run_cpp(tmp_path, CAPTURE_HARNESS.replace('// ACTUAL_CAPTURE', body))
    assert result.returncode == 0, result.stdout + result.stderr


def test_pinned_keyboard_tables_and_two_id_extent():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    import struct
    path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    if not path.is_file():
        pytest.skip('pinned image unavailable')
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    for low, high, table_at, indices_at, handlers in [
        (13, 105, 0x439B3C, 0x439B60,
         [0x439ADB, 0x439AE9, 0x439A68, 0x439A7A, 0x439AB9, 0x439AA6, 0x439A4E, 0x439A57, 0x439AF5]),
        (77, 117, 0x439BC0, 0x439BCC, [0x439B25, 0x439B0F, 0x439B31]),
    ]:
        table = struct.unpack('<' + 'I' * len(handlers), image.read(Span(table_at, table_at + 4 * len(handlers))))
        assert list(table) == handlers
        indices = image.read(Span(indices_at, indices_at + high - low + 1))
        expected = ({13:0,27:1,33:2,105:2,34:3,99:3,35:4,97:4,36:5,103:5,37:6,100:6,39:7,102:7}
                    if low == 13 else {77:0,79:0,109:0,111:0,85:1,117:1})
        assert list(indices) == [expected.get(key, len(handlers) - 1) for key in range(low, high + 1)]
    instructions = {i.address: (i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x439EB0, 0x439F18)), 0x439EB0)}
    assert instructions[0x439EC8] == ('lea', 'edi, [esi + 0x14218]')
    assert instructions[0x439EFB] == ('add', 'edx, 4')
    assert instructions[0x439EFE] == ('cmp', 'eax, 2')
    assert instructions[0x439F05] == ('mov', 'dword ptr [esi + eax*4 + 0x14218], ecx')
