"""Actual-source controls for the remaining Win DirectDraw HDC calls."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/win.cpp"
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


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


SIGNATURES = (
    "void Win::window_line_raw(int x2, int y2, int x1, int y1, int colour,",
    "void Win::sub_5ef1e0(int x1, int y1, int x2, int y2, void *pen, int unused6)",
    "long __cdecl Win::OnActivate(void *hwnd, unsigned int state, void *other,",
)

PRELUDE = r"""
#include <cassert>
typedef void *HDC;
typedef void *HGDIOBJ;
typedef void *HPEN;
typedef void *HPALETTE;
typedef void *HWND;
struct HWND__ {};
#define __cdecl
#define __stdcall
#define FALSE 0
#define TRANSPARENT 1
struct LOGPEN { unsigned int lopnStyle; struct { int x; int y; } lopnWidth; unsigned int lopnColor; };
struct Win;
struct IDirectDrawSurface2;
static IDirectDrawSurface2 *DirectDrawSurface;
static int WinHdcRefCount;
static HDC WinSharedHdc;
static HWND HandleMain;
static void *PaletteInitialized;
static int BufferDirectDraw;
static void *WinPointerOwner1, *WinPointerOwner2;
static int get_calls[2], release_calls[2], gdi_get_calls, gdi_release_calls;
static int palette_calls, realize_calls, draw_calls;
static bool mutate_global_on_acquire;
static IDirectDrawSurface2 *surface_after_acquire;
static HDC supplied_hdc;
static HDC received_hdc;
static int fail_acquire;
static int hdc_token;
struct Palette {
    int seed_;
    void set_active_window(Win *);
};
static Palette palette_active = {7};
static Palette *PaletteActive = &palette_active;
struct IDirectDrawSurface2 {
    int identity;
    int GetDC(HDC *out) {
        ++get_calls[identity];
        if (!fail_acquire) *out = supplied_hdc;
        if (mutate_global_on_acquire) DirectDrawSurface = surface_after_acquire;
        return fail_acquire ? -1 : 0;
    }
    int ReleaseDC(HDC hdc) {
        ++release_calls[identity];
        received_hdc = hdc;
        return -1;
    }
};
static IDirectDrawSurface2 surface_a = {0}, surface_b = {1};
void Palette::set_active_window(Win *) {
    if (mutate_global_on_acquire) DirectDrawSurface = surface_after_acquire;
}
struct Win {
    int palette_seed_cache_;
    void window_line_raw(int, int, int, int, int, int, unsigned int);
    void sub_5ef1e0(int, int, int, int, void *, int);
    long OnActivate(void *, unsigned int, void *, long);
};
static HDC GetDC(HWND) {
    ++gdi_get_calls;
    if (mutate_global_on_acquire) DirectDrawSurface = surface_after_acquire;
    return fail_acquire ? 0 : supplied_hdc;
}
static int ReleaseDC(HWND, HDC hdc) {
    ++gdi_release_calls; received_hdc = hdc; return 1;
}
static HPEN CreatePenIndirect(const LOGPEN *) { ++draw_calls; return &hdc_token; }
static HGDIOBJ SelectObject(HDC, HGDIOBJ object) { ++draw_calls; return object; }
static int MoveToEx(HDC, int, int, void *) { ++draw_calls; return 1; }
static int LineTo(HDC, int, int) { ++draw_calls; return 1; }
static int DeleteObject(HGDIOBJ) { ++draw_calls; return 1; }
static int SelectPalette(HDC, HPALETTE, int) { ++palette_calls; return 1; }
static int RealizePalette(HDC) { ++realize_calls; return 1; }
static int InvalidateRect(HWND, void *, int) { return 1; }
static long DefWindowProcA(HWND, unsigned int, unsigned long, long) { return 0x55; }
static void reset() {
    WinHdcRefCount = 0; WinSharedHdc = 0; HandleMain = reinterpret_cast<HWND>(&hdc_token);
    DirectDrawSurface = &surface_a; BufferDirectDraw = 0;
    WinPointerOwner1 = WinPointerOwner2 = &hdc_token;
    get_calls[0] = get_calls[1] = release_calls[0] = release_calls[1] = 0;
    gdi_get_calls = gdi_release_calls = palette_calls = realize_calls = draw_calls = 0;
    mutate_global_on_acquire = false; surface_after_acquire = 0;
    supplied_hdc = reinterpret_cast<HDC>(&hdc_token); received_hdc = 0;
    fail_acquire = 0; palette_active.seed_ = 7;
}
"""

CASES = r"""
int main() {
    Win win = {7};
    reset();
    // Acquisition uses its local surface snapshot; release rereads the global
    // after GetDC's callback replaced it.
    mutate_global_on_acquire = true; surface_after_acquire = &surface_b;
    win.window_line_raw(4, 5, 1, 2, 3, 1, 0);
    assert(get_calls[0] == 1 && get_calls[1] == 0);
    assert(release_calls[0] == 0 && release_calls[1] == 1);
    assert(received_hdc == supplied_hdc && WinHdcRefCount == 0 && !WinSharedHdc);

    // Existing reference: no acquire/release, and the per-call count balances.
    reset(); WinHdcRefCount = 2; WinSharedHdc = supplied_hdc;
    win.sub_5ef1e0(1, 2, 3, 4, &hdc_token, 0);
    assert(WinHdcRefCount == 2 && get_calls[0] == 0 && release_calls[0] == 0);

    // DirectDraw null output preserves the early return and zero count.
    reset(); fail_acquire = 1;
    assert(win.OnActivate(HandleMain, 1, 0, 0) == 0x55);
    assert(get_calls[0] == 1 && palette_calls == 0 && realize_calls == 0);
    assert(WinHdcRefCount == 0 && release_calls[0] == 0);

    // No surface at acquisition selects GDI. A callback then installs the
    // DirectDraw surface, which is re-read and used for final release.
    reset(); DirectDrawSurface = 0;
    mutate_global_on_acquire = true; surface_after_acquire = &surface_b;
    assert(win.OnActivate(HandleMain, 1, 0, 0) == 0x55);
    assert(gdi_get_calls == 1 && get_calls[0] == 0);
    assert(release_calls[1] == 1 && gdi_release_calls == 0);
    assert(received_hdc == supplied_hdc && WinHdcRefCount == 0 && !WinSharedHdc);

    // When callbacks leave the global null, the original fallback release is
    // retained even if acquisition used DirectDraw.
    reset(); mutate_global_on_acquire = true; surface_after_acquire = 0;
    win.sub_5ef1e0(1, 2, 3, 4, &hdc_token, 0);
    assert(get_calls[0] == 1 && gdi_release_calls == 1);
    assert(received_hdc == supplied_hdc && WinHdcRefCount == 0);
    return 0;
}
"""


def _run(compiler: str, source: str, tmp_path: Path, *, should_pass: bool) -> None:
    cpp = tmp_path / "win_hdc_actual.cpp"
    exe = tmp_path / "win_hdc_actual"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                            "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_remaining_hdc_methods_and_executed_mutant(tmp_path: Path, compiler: str | None) -> None:
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    product = SOURCE.read_text(encoding="utf-8")
    bodies = [extract_body(product, signature) for signature in SIGNATURES]
    for body in bodies:
        assert "ComSlot017" not in body and "ComSlot026" not in body
        assert "reinterpret_cast<void ***>" not in body
    actual = PRELUDE + "\n".join(bodies) + CASES
    _run(compiler, actual, tmp_path, should_pass=True)

    damaged = bodies[0].replace("iface->GetDC(&WinSharedHdc);",
                                "iface->ReleaseDC(WinSharedHdc);", 1)
    assert damaged != bodies[0]
    mutant = PRELUDE + "\n".join([damaged] + bodies[1:]) + CASES
    _run(compiler, mutant, tmp_path, should_pass=False)
