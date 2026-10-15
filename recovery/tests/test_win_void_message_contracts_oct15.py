"""Actual-body checks for three void Win message callback contracts."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"
COMPILERS = [p for name in ("g++", "clang++") if (p := shutil.which(name))]


def extract_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected one current body for {signature}")
    opening = source.find("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError(f"unterminated body: {signature}")


PRELUDE = r"""
#include <cassert>
#define __cdecl
typedef void *HDC;
#define FALSE 0
struct HWND__ {};
typedef HWND__ *HWND;
struct RECT { int left, top, right, bottom; };
struct PAINTSTRUCT { HDC hdc; int fErase; RECT rcPaint; int fRestore, fIncUpdate; unsigned char rgbReserved[32]; };
static HWND HandleMain;
static void *PaletteInitialized;
static void *BufferDirectDraw;
static int WinHdcRefCount;
static HDC WinSharedHdc;
static int palette_calls, realize_calls, surface_get[2], surface_release[2];
static int gdi_get, gdi_release, begin_calls, update_calls, flip_calls, end_calls;
static int paint_events[3], paint_event_count;
static int begin_fails, mutate_surface_on_get;
static HWND paint_hwnd;
static RECT update_rect, flip_rect;
static HDC paint_hdc, acquired_hdc;
struct IDirectDrawSurface2;
static IDirectDrawSurface2 *DirectDrawSurface;
extern IDirectDrawSurface2 surface_b;
struct IDirectDrawSurface2 {
    int id;
    int GetDC(HDC *out) {
        ++surface_get[id];
        *out = acquired_hdc;
        if (mutate_surface_on_get) DirectDrawSurface = &surface_b;
        return 0;
    }
    long ReleaseDC(HDC) { ++surface_release[id]; return -1; }
};
IDirectDrawSurface2 surface_a = {0}, surface_b = {1};
static HDC GetDC(HWND) { ++gdi_get; return acquired_hdc; }
static int ReleaseDC(HWND, HDC) { ++gdi_release; return 1; }
static int SelectPalette(HDC, void *, int) { ++palette_calls; return 1; }
static int RealizePalette(HDC) { ++realize_calls; return 1; }
static HDC BeginPaint(HWND hwnd, PAINTSTRUCT *ps) {
    ++begin_calls; paint_hwnd = hwnd;
    ps->rcPaint = RECT{11, 12, 31, 42};
    ps->hdc = paint_hdc;
    return begin_fails ? 0 : paint_hdc;
}
static void update_screen(RECT *rect, int) { ++update_calls; update_rect = *rect; paint_events[paint_event_count++] = 1; }
static void flip(RECT *rect) { ++flip_calls; flip_rect = *rect; paint_events[paint_event_count++] = 2; }
static int EndPaint(HWND hwnd, PAINTSTRUCT *) { ++end_calls; assert(hwnd == paint_hwnd); paint_events[paint_event_count++] = 3; return 1; }
class Win {
public:
    static void __cdecl OnPaint(HWND);
    static void __cdecl OnPaletteChanged(void *, void *);
};
static void reset() {
    static HWND__ hwnd_token;
    HandleMain = &hwnd_token;
    PaletteInitialized = reinterpret_cast<void *>(0x1234);
    BufferDirectDraw = nullptr; WinHdcRefCount = 0; WinSharedHdc = 0;
    DirectDrawSurface = &surface_a;
    palette_calls = realize_calls = surface_get[0] = surface_get[1] = 0;
    surface_release[0] = surface_release[1] = gdi_get = gdi_release = 0;
    begin_calls = update_calls = flip_calls = end_calls = paint_event_count = 0;
    paint_events[0] = paint_events[1] = paint_events[2] = 0;
    begin_fails = mutate_surface_on_get = 0;
    paint_hwnd = 0; update_rect = flip_rect = RECT{0,0,0,0};
    paint_hdc = reinterpret_cast<HDC>(0x4321);
    acquired_hdc = reinterpret_cast<HDC>(0x5678);
}
"""

PALETTE_CASES = r"""
int main() {
    reset();
    Win::OnPaletteChanged(HandleMain, HandleMain);
    assert(!surface_get[0] && !palette_calls && !WinHdcRefCount);

    reset(); BufferDirectDraw = reinterpret_cast<void *>(1);
    Win::OnPaletteChanged(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2));
    assert(!surface_get[0] && !gdi_get && !palette_calls);

    reset(); WinHdcRefCount = 3; WinSharedHdc = acquired_hdc;
    Win::OnPaletteChanged(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2));
    assert(WinHdcRefCount == 3 && !surface_get[0] && !surface_release[0]);
    assert(palette_calls == 1 && realize_calls == 1);

    reset(); mutate_surface_on_get = 1;
    Win::OnPaletteChanged(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2));
    assert(surface_get[0] == 1 && !surface_get[1]);
    assert(!surface_release[0] && surface_release[1] == 1);
    assert(!WinHdcRefCount && !WinSharedHdc && palette_calls == 1 && realize_calls == 1);

    reset(); acquired_hdc = 0;
    Win::OnPaletteChanged(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2));
    assert(surface_get[0] == 1 && !surface_release[0]);
    assert(!WinHdcRefCount && !WinSharedHdc && !palette_calls && !realize_calls);

    reset(); DirectDrawSurface = 0;
    Win::OnPaletteChanged(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2));
    assert(gdi_get == 1 && gdi_release == 1 && !surface_get[0]);
    assert(!WinHdcRefCount && !WinSharedHdc && palette_calls == 1 && realize_calls == 1);
    return 0;
}
"""

PAINT_CASES = r"""
int main() {
    reset(); begin_fails = 1;
    Win::OnPaint(HandleMain);
    assert(begin_calls == 1 && paint_hwnd == HandleMain);
    assert(!update_calls && !flip_calls && !end_calls);

    reset();
    Win::OnPaint(HandleMain);
    assert(begin_calls == 1 && update_calls == 1 && flip_calls == 1 && end_calls == 1);
    assert(update_rect.left == 11 && update_rect.top == 12 && update_rect.right == 31 && update_rect.bottom == 42);
    assert(flip_rect.left == 11 && flip_rect.top == 12 && flip_rect.right == 31 && flip_rect.bottom == 42);
    assert(paint_event_count == 3 && paint_events[0] == 1 && paint_events[1] == 2 && paint_events[2] == 3);
    return 0;
}
"""


def run(compiler: str, source: str, tmp_path: Path, *, should_pass: bool) -> None:
    cpp, exe = tmp_path / "void_message.cpp", tmp_path / "void_message"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
                           str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0


@pytest.mark.parametrize("compiler", COMPILERS or [None], ids=lambda p: Path(p).name if p else "no-host-cxx")
def test_actual_paint_and_palette_changed_effects_and_mutants(tmp_path: Path, compiler: str | None) -> None:
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    source = SOURCE.read_text(encoding="utf-8")
    palette = extract_body(source, "void __cdecl Win::OnPaletteChanged(void * hwnd, void * lparam)")
    paint = extract_body(source, "void __cdecl Win::OnPaint(HWND hwnd)")
    base = PRELUDE + palette + paint

    run(compiler, base + PALETTE_CASES, tmp_path, should_pass=True)
    changed = palette.replace("if (hwnd != lparam && BufferDirectDraw == nullptr)", "if (true)", 1)
    assert changed != palette
    run(compiler, PRELUDE + changed + paint + PALETTE_CASES, tmp_path, should_pass=False)

    run(compiler, base + PAINT_CASES, tmp_path, should_pass=True)
    dropped_end = paint.replace("EndPaint(hwnd, &ps);", "", 1)
    assert dropped_end != paint
    run(compiler, PRELUDE + palette + dropped_end + PAINT_CASES, tmp_path, should_pass=False)
    reordered = paint.replace("update_screen(&paintRect, 0);\n        flip(&paintRect);",
                              "flip(&paintRect);\n        update_screen(&paintRect, 0);", 1)
    assert reordered != paint
    run(compiler, PRELUDE + palette + reordered + PAINT_CASES, tmp_path, should_pass=False)
