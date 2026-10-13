"""Bounded source control for SelectPartWin's captured parent DIB surface.

This compiles the literal frame/scroll phase from SelectPartWin::exec. It does
not compile the complete modal or prove the game's ABI/runtime behavior.
"""
import os
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def source_phase(source):
    begin = source.index('    const int width = TutWinDesWindow.pixel_width();')
    end = source.index('    field_2B60_ = 0;\n    if (selected >=', begin)
    return source[begin:end]


HARNESS = r'''
#include <cstdio>
#include <cstdlib>

struct Win {};
struct RECT { int left, top, right, bottom; };
struct FakeScreen {
    int width = 911;
    int height = 613;
};
struct DesignSurface : Win {
    int width = 503;
    int height = 307;
    int pixel_width() const { return width; }
    int pixel_height() const { return height; }
};
FakeScreen Screen;
DesignSurface TutWinDesWindow;
const int WinScreenWidth = 911;
const int WinScreenHeight = 613;
int InterfaceBoxStyles[1];

void check(bool condition, const char *diagnostic) {
    if (!condition) {
        std::fprintf(stderr, "%s\n", diagnostic);
        std::exit(98);
    }
}

struct Buffer {
    int width = 701;
    int pixel_width() const { return width; }
    void box_sprite(RECT *frame, int *) {
        check(frame->left == 0x17 && frame->top == 0 &&
              frame->right == 503 - 0x37 && frame->bottom == 307,
              "frame did not use captured parent DIB dimensions");
    }
};

struct GraphicWin : Win, Buffer {
    int init(int, int, int width, int height, int, int, Win *parent,
             int, int) {
        check(width == 503 && height == 307,
              "init did not use captured parent DIB dimensions");
        check(parent == static_cast<Win *>(&TutWinDesWindow),
              "GraphicWin::init parent is not TutWinDesWindow");
        // Model callbacks changing the parent after dimensions were captured.
        TutWinDesWindow.width = 1201;
        TutWinDesWindow.height = 1001;
        return INIT_RESULT; // The original intentionally ignores this result.
    }
    void fill(int) {
        // Filling can change this window's own DIB width before scroll setup.
        width = 809;
    }
};

struct Scroll {
    int paging_ = 0;
    int field_A30_ = 0;
    void init(int x, int y, int width, int height, Win *parent,
              int, int);
    void set_range(int, int) {}
    void redraw() {}
    void hide() {}
};

struct SelectPartWin : GraphicWin {
    Scroll scroll_;
    int field_2B60_ = 0;
    int field_2B6C_ = 5;
    int field_2B70_ = 4;
    int field_2B74_ = 19;
    void exercise() {
PHASE
    }
};
SelectPartWin *active_picker;
void Scroll::init(int x, int y, int width, int height, Win *parent,
                  int, int) {
    check(x == 809 - 0x2b,
          "scroll x did not reload own Buffer width after fill");
    check(y == 0x15 && width == 0x18 && height == 0xf6,
          "scroll geometry changed");
    check(parent == static_cast<Win *>(active_picker),
          "scroll parent is not this SelectPartWin");
}

int main() {
    SelectPartWin picker;
    active_picker = &picker;
    const int screen_width = Screen.width;
    const int screen_height = Screen.height;
    check(screen_width != TutWinDesWindow.width &&
          screen_height != TutWinDesWindow.height &&
          picker.pixel_width() != TutWinDesWindow.width,
          "test fixture dimensions are not distinct");
    picker.exercise();
    check(TutWinDesWindow.width == 1201 && TutWinDesWindow.height == 1001,
          "init callback did not mutate parent dimensions");
    check(picker.pixel_width() == 809,
          "fill callback did not mutate own DIB width");
}
'''


def run_control(tmp_path, compiler_name, phase, damage, init_result):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f'{compiler_name} unavailable')
    if damage == 'parent_self':
        phase = phase.replace(
            'static_cast<Win *>(&TutWinDesWindow)',
            'static_cast<Win *>(this)')
    elif damage == 'screen_dimensions':
        phase = phase.replace(
            'TutWinDesWindow.pixel_width()', 'WinScreenWidth', 1)
        phase = phase.replace(
            'TutWinDesWindow.pixel_height()', 'WinScreenHeight', 1)
    elif damage == 'stale_scroll_width':
        phase = phase.replace(
            'static_cast<Buffer *>(this)->pixel_width()',
            'TutWinDesWindow.pixel_width()', 1)

    executable = tmp_path / f'{compiler_name}-{damage or "control"}'
    built = subprocess.run(
        [compiler, '-std=c++17', '-O2', '-x', 'c++', '-', '-o', str(executable)],
        input=HARNESS.replace('PHASE', phase).replace('INIT_RESULT', str(init_result)),
        text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run(
        [str(executable)], cwd=tmp_path, capture_output=True,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    if not damage:
        assert result.returncode == 0, result.stderr
        return
    diagnostics = {
        'parent_self': b'GraphicWin::init parent is not TutWinDesWindow',
        'screen_dimensions': b'init did not use captured parent DIB dimensions',
        'stale_scroll_width': b'scroll x did not reload own Buffer width after fill',
    }
    assert result.returncode == 98, (result.returncode, result.stderr)
    assert diagnostics[damage] in result.stderr


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('init_result', [0, -7, 11])
@pytest.mark.parametrize('damage', ['', 'parent_self', 'screen_dimensions',
                                     'stale_scroll_width'])
def test_literal_exec_parent_frame_and_scroll_phase(
        tmp_path, compiler_name, damage, init_result):
    source = (ROOT / 'src/selectpartwin.cpp').read_text()
    phase = source_phase(source)
    assert 'const int width = TutWinDesWindow.pixel_width();' in phase
    assert 'const int height = TutWinDesWindow.pixel_height();' in phase
    assert 'static_cast<Win *>(&TutWinDesWindow)' in phase
    assert 'RECT frame = {0x17, 0, width - 0x37, height};' in phase
    assert 'GraphicWin::fill(9);' in phase
    assert 'static_cast<Buffer *>(this)->pixel_width() - 0x2b' in phase
    run_control(tmp_path, compiler_name, phase, damage, init_result)


def test_pinned_parent_surface_and_live_scroll_width_instructions():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        'OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    if not image_path.is_file():
        pytest.skip('pinned image unavailable')
    image = PeImage(image_path.read_bytes(), PIN)
    instructions = {
        ins.address: (ins.mnemonic, ins.op_str)
        for ins in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
            image.read(Span(0x434220, 0x4342b0)), 0x434220)
    }
    expected = {
        0x434229: ('mov', 'esi, dword ptr [0x71f778]'),
        0x43422f: ('mov', 'edi, dword ptr [0x71f774]'),
        0x434239: ('push', '0x71f2b0'),
        0x43423e: ('neg', 'esi'),
        0x434247: ('push', 'esi'),
        0x434248: ('push', 'edi'),
        0x43424f: ('call', '0x5d4ef0'),
        0x434293: ('mov', 'eax, dword ptr [ebx + 0x4c4]'),
        0x4342ab: ('add', 'eax, -0x2b'),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction
