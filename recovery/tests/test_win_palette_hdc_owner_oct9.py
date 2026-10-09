"""Host regression controls for Win::OnQueryNewPalette's actual source body."""

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
    body = _body(text, "int __cdecl Win::OnQueryNewPalette(void * hwnd)")
    assert "int eax" not in body
    assert "IDirectDrawSurface2 *surface" in body
    assert "surface->ReleaseDC(WinSharedHdc)" in body
    assert "ReleaseDC(\n            reinterpret_cast<HDC>(&WinSharedHdc))" not in body
    return body


HARNESS = r'''
#include <cassert>
#define __cdecl
using HDC = void *;
using HPALETTE = void *;
using HWND = int;
struct IDirectDrawSurface2 {
    int get_calls, release_calls;
    HDC received;
    HDC get_result;
    int fail_get;
    int GetDC(HDC *out) {
        ++get_calls;
        if (!fail_get) *out = get_result;
        return fail_get ? -1 : 0;
    }
    int ReleaseDC(HDC hdc) { ++release_calls; received = hdc; return -1; }
};
static int BufferDirectDraw, WinHdcRefCount;
static HDC WinSharedHdc, callback_hdc;
static HWND HandleMain = 12;
static IDirectDrawSurface2 *DirectDrawSurface;
static int PaletteInitialized;
static int selected_calls, realized_calls, desktop_releases;
static HDC selected_hdc, realized_hdc, desktop_hdc;
static IDirectDrawSurface2 *replacement_surface;
struct Win { int OnQueryNewPalette(void *); };
static int SelectPalette(HDC hdc, HPALETTE, int) {
    ++selected_calls;
    selected_hdc = hdc;
    if (callback_hdc) {
        WinSharedHdc = callback_hdc;
        DirectDrawSurface = replacement_surface;
    }
    return 0;
}
static int RealizePalette(HDC hdc) {
    ++realized_calls; realized_hdc = hdc; return 0;
}
static HDC GetDC(HWND) { return reinterpret_cast<HDC>(0x1111); }
static int ReleaseDC(HWND, HDC hdc) {
    ++desktop_releases; desktop_hdc = hdc; return 1;
}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_palette_query_preserves_hdc_values_and_callback_rereads(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")

    cases = r'''
int main() {
    Win win{};
    IDirectDrawSurface2 first{}, second{};
    HDC initial = reinterpret_cast<HDC>(0x1234);
    HDC changed = reinterpret_cast<HDC>(0x5678);

    // The surface owns GetDC's output and ReleaseDC receives the handle value.
    first.get_result = initial;
    DirectDrawSurface = &first;
    assert(win.OnQueryNewPalette(nullptr) == 0);
    assert(first.get_calls == 1 && first.release_calls == 1);
    assert(first.received == initial);
    assert(WinHdcRefCount == 0 && WinSharedHdc == nullptr);

    // Failed surface acquisition preserves the null handle and zero count.
    first.fail_get = 1;
    first.get_result = nullptr;
    DirectDrawSurface = &first;
    assert(win.OnQueryNewPalette(nullptr) == 0);
    assert(first.get_calls == 2 && WinHdcRefCount == 0);
    assert(selected_calls == 1 && first.release_calls == 1);
    first.fail_get = 0;

    // SelectPalette may change callback-visible globals. Later calls re-read
    // the shared HDC and surface, while SelectPalette retains the earlier value.
    WinHdcRefCount = 0;
    WinSharedHdc = nullptr;
    first.get_result = initial;
    DirectDrawSurface = &first;
    callback_hdc = changed;
    replacement_surface = &second;
    assert(win.OnQueryNewPalette(nullptr) == 0);
    assert(selected_hdc == initial);
    assert(realized_hdc == changed);
    assert(second.release_calls == 1 && second.received == changed);
    assert(first.release_calls == 1);
    assert(WinHdcRefCount == 0 && WinSharedHdc == nullptr);

    // The desktop API receives the stored handle; null acquisition returns.
    callback_hdc = nullptr;
    DirectDrawSurface = nullptr;
    WinHdcRefCount = 0;
    assert(win.OnQueryNewPalette(nullptr) == 0);
    assert(desktop_releases == 1 && desktop_hdc == reinterpret_cast<HDC>(0x1111));

    // The original null-handle shared-reference path increments before return.
    WinHdcRefCount = 2;
    WinSharedHdc = nullptr;
    assert(win.OnQueryNewPalette(nullptr) == 0);
    assert(WinHdcRefCount == 3 && selected_calls == 3);

    BufferDirectDraw = 1;
    assert(win.OnQueryNewPalette(nullptr) == 1);
}
'''
    source = tmp_path / "win_palette_hdc.cpp"
    source.write_text(HARNESS + _actual_source() + cases)
    binary = tmp_path / "win_palette_hdc"
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                            "-Werror", "-Wno-unused-parameter",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
