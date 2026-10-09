"""Host controls for the recovered Win DirectDraw HDC owner calls.

The two product bodies and their acquisition macro are extracted verbatim
from src/win.cpp. The harness models only the API surface and GDI calls they
touch; it checks original pointer/refcount/leak/release decisions, not VC6 ABI.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/win.cpp"
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def _actual_source():
    text = SOURCE.read_text()
    macro = next(line for line in text.splitlines()
                 if line.startswith("#define ACQUIRE_HDC("))
    draw = _body(text,
                 "void Win::draw_rect_border(int x1, int y1, int x2, int y2,")
    tiled = _body(text,
                  "void Win::paint_tiled(Buffer *tile, int x_origin, int y_origin,")
    assert "DirectDrawSurface->GetDC(&WinSharedHdc)" in macro
    assert "ComSlot17" not in macro and "reinterpret_cast<void ***>" not in macro
    assert "DirectDrawSurface->GetDC(&WinSharedHdc)" in tiled
    assert "DirectDrawSurface->ReleaseDC(WinSharedHdc)" in tiled
    assert "ComSlot017" not in tiled and "ComSlot026" not in tiled
    return macro + "\n" + draw + "\n" + tiled


HARNESS = r'''
#include <cassert>
using HDC = void *;
using HGDIOBJ = void *;
using HWND = int;
struct RECT { int left, top, right, bottom; };
struct HRGN__ { int unused; };
using HRGN = HRGN__ *;
static int WinHdcRefCount;
static HDC WinSharedHdc;
static HWND HandleMain = 7;
static int get_dc_calls, release_dc_calls, gdi_calls, tile_calls;
static HDC output_hdc, received_hdc;
static int fail_get_dc, fail_region;

struct IDirectDrawSurface2 {
    int GetDC(HDC *out) {
        ++get_dc_calls;
        if (!fail_get_dc) *out = output_hdc;
        return fail_get_dc ? -1 : 0;
    }
    int ReleaseDC(HDC hdc) {
        ++release_dc_calls;
        received_hdc = hdc;
        return -1; // The original ignores this result.
    }
};
static IDirectDrawSurface2 fake_surface;
static IDirectDrawSurface2 *DirectDrawSurface;

struct Win;
struct Palette {
    int seed_;
    static void set_active_window(Win *) {}
};
static Palette active_palette{9};
static Palette *PaletteActive = &active_palette;
struct Win {
    int palette_seed_cache_;
    void release_hdc() {
        if (--WinHdcRefCount == 0) {
            if (DirectDrawSurface) DirectDrawSurface->ReleaseDC(WinSharedHdc);
            WinSharedHdc = nullptr;
        }
    }
    void sync_palette() {}
    void draw_rect_border(int, int, int, int, HGDIOBJ, HGDIOBJ, int);
    void paint_tiled(struct Buffer *, int, int, int, int, int, int, int);
};
static HDC GetDC(HWND) { ++get_dc_calls; return output_hdc; }
static int ReleaseDC(HWND, HDC hdc) {
    ++release_dc_calls; received_hdc = hdc; return 1;
}
static HGDIOBJ SelectObject(HDC, HGDIOBJ object) { ++gdi_calls; return object; }
static int MoveToEx(HDC, int, int, void *) { ++gdi_calls; return 1; }
static int LineTo(HDC, int, int) { ++gdi_calls; return 1; }
static HRGN CreateRectRgnIndirect(RECT *) {
    static HRGN__ region;
    return fail_region ? nullptr : &region;
}
static int SelectClipRgn(HDC, HRGN) { ++gdi_calls; return 1; }
struct BitmapHeader { int biWidth, biHeight; };
struct Dib { BitmapHeader bmiHeader; };
struct Buffer {
    Dib dib_;
    int copy_to_window(Win *, int, int, int, int, int, int) {
        ++tile_calls; return 0;
    }
};
static void reset() {
    WinHdcRefCount = 0; WinSharedHdc = nullptr;
    DirectDrawSurface = &fake_surface;
    get_dc_calls = release_dc_calls = gdi_calls = tile_calls = 0;
    output_hdc = reinterpret_cast<HDC>(0x1234);
    received_hdc = nullptr; fail_get_dc = fail_region = 0;
}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_hdc_bodies_keep_original_state_decisions(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    actual = _actual_source()
    cases = r'''
int main() {
    Win win{};
    Buffer tile{};
    tile.dib_.bmiHeader = {2, -2};

    // The four original border acquisitions pass storage for the output HDC.
    reset();
    win.draw_rect_border(0, 0, 2, 2, nullptr, nullptr, 0);
    assert(get_dc_calls == 4 && release_dc_calls == 4);
    assert(WinHdcRefCount == 0 && WinSharedHdc == nullptr);
    assert(received_hdc == output_hdc);

    // Failed GetDC leaves the count at zero and each original border arm retries.
    reset(); fail_get_dc = 1;
    win.draw_rect_border(0, 0, 2, 2, nullptr, nullptr, 0);
    assert(get_dc_calls == 4 && release_dc_calls == 0);
    assert(WinHdcRefCount == 0 && WinSharedHdc == nullptr);

    // paint_tiled also passes the address; failed acquisition returns early.
    reset(); fail_get_dc = 1;
    win.paint_tiled(&tile, 0, 0, 0, 0, 2, 2, 0);
    assert(get_dc_calls == 1 && WinHdcRefCount == 0);
    assert(tile_calls == 0 && release_dc_calls == 0);

    // The original region-allocation failure leaks its acquired reference.
    reset(); fail_region = 1;
    win.paint_tiled(&tile, 0, 0, 0, 0, 2, 2, 0);
    assert(WinHdcRefCount == 1 && WinSharedHdc == output_hdc);
    assert(tile_calls == 0 && release_dc_calls == 0);

    // Successful completion releases the HDC value, then clears the global.
    reset();
    win.paint_tiled(&tile, 0, 0, 0, 0, 2, 2, 0);
    assert(tile_calls == 1 && release_dc_calls == 1);
    assert(received_hdc == output_hdc);
    assert(WinHdcRefCount == 0 && WinSharedHdc == nullptr);

    // Existing holders are merely decremented; they do not release the surface.
    reset(); WinHdcRefCount = 2; WinSharedHdc = output_hdc;
    win.paint_tiled(&tile, 0, 0, 0, 0, 2, 2, 0);
    assert(WinHdcRefCount == 2 && release_dc_calls == 0);
}
'''
    source = tmp_path / "win_hdc.cpp"
    source.write_text(HARNESS + actual + cases)
    binary = tmp_path / "win_hdc"
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                            "-Werror", "-Wno-unused-parameter",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
