"""Actual-source controls for Buffer's typed pixel-storage locals.

The host fixture supplies only the DirectDraw and member-layout surface used
by the three extracted methods. Low-address storage is used for the retained
fill_line pointer-bits-in-int return; no native ABI or matching claim follows.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"
SIGNATURES = (
    "int Buffer::fill_line(int x1, int y1, int x2, int y2, char fill_colour) {",
    "int Buffer::put_pixel(int a1, int a2, int a3) {",
)


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated Buffer body: {signature}")


HARNESS = r'''
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#define DDLOCK_WAIT 1
typedef void *LPVOID;
struct RECT { int left, top, right, bottom; };
struct Header { int biWidth, biHeight; };
struct Dib { Header bmiHeader; };
struct DDSURFACEDESC { uint32_t dwSize; int32_t lPitch; void *lpSurface; };

static void require(bool ok, const char *message) {
    if (!ok) { fprintf(stderr, "failed: %s\n", message); abort(); }
}

class IDirectDrawSurface {
public:
    virtual int Lock(RECT *, DDSURFACEDESC *, uint32_t, void *) = 0;
    virtual int Unlock(void *) = 0;
};

class TestSurface : public IDirectDrawSurface {
public:
    unsigned char *pixels;
    int pitch;
    int lock_result;
    int locks;
    int unlocks;
    void *unlocked;
    TestSurface(unsigned char *p, int s)
        : pixels(p), pitch(s), lock_result(0), locks(0), unlocks(0), unlocked(0) {}
    int Lock(RECT *, DDSURFACEDESC *desc, uint32_t flags, void *) {
        ++locks;
        require(flags == DDLOCK_WAIT, "Lock flags");
        require(desc->dwSize == 0x6c || desc->dwSize == sizeof(*desc),
                "original method-specific descriptor size");
        desc->lPitch = pitch;
        desc->lpSurface = pixels;
        return lock_result;
    }
    int Unlock(void *bits) { ++unlocks; unlocked = bits; return 0; }
};

class Buffer {
public:
    void *locked_bits_;
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    int surface_lock_count_;
    uint32_t stride_;
    RECT rect1_;
    Dib dib_;
    int fill_line(int, int, int, int, char);
    int put_pixel(int, int, int);
};

static unsigned char *low_storage(size_t size) {
    void *p = mmap(0, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    require(p != MAP_FAILED, "low-address pixel mapping");
    require(reinterpret_cast<uintptr_t>(p) <= 0x7fffffff,
            "pointer return fits original int carrier");
    return static_cast<unsigned char *>(p);
}

static void test_fill_line() {
    unsigned char *pixels = low_storage(4096);
    Buffer b = {};
    b.dib_bits_ = pixels;
    b.stride_ = 8;
    b.rect1_ = RECT{0, 0, 8, 8};
    require(b.fill_line(1, 1, 3, 3, 0x35) == 0, "DIB fill_line result");
    require(pixels[9] == 0x35 && pixels[18] == 0x35 && pixels[27] == 0x35,
            "positive-pitch diagonal pixels");
    require(b.surface_lock_count_ == 0 && b.locked_bits_ == 0,
            "DIB fill_line releases its holder");

    // Reverse-y traversal uses the signed interpretation of the stored pitch.
    for (int i = 0; i < 64; ++i) pixels[i] = 0;
    b.dib_bits_ = pixels + 24;
    b.stride_ = static_cast<uint32_t>(-8);
    require(b.fill_line(1, 3, 3, 1, 0x46) == 0, "negative-pitch fill_line");
    require(pixels[1] == 0x46 && pixels[10] == 0x46 && pixels[19] == 0x46,
            "negative-pitch descending pixels");

    TestSurface surface(pixels, 8);
    b.surface_ = &surface;
    b.stride_ = 8;
    b.locked_bits_ = pixels;
    b.surface_lock_count_ = 2;
    const int pointer_bits = static_cast<int>(reinterpret_cast<uintptr_t>(pixels));
    require(b.fill_line(1, 1, 2, 2, 0x57) == pointer_bits,
            "nested surface path preserves pointer bits in int result");
    require(surface.locks == 0 && surface.unlocks == 0 &&
            b.surface_lock_count_ == 2 && b.locked_bits_ == pixels,
            "nested surface path retains one holder");

    Buffer failed = {};
    TestSurface failing(pixels, 8);
    failing.lock_result = 1;
    failed.surface_ = &failing;
    require(failed.fill_line(1, 1, 2, 2, 0x68) == 1,
            "failed Lock result returned directly");
    require(failing.locks == 1 && failing.unlocks == 0 &&
            failed.surface_lock_count_ == 0,
            "failed Lock does not acquire or release a holder");
}

static void test_put_pixel() {
    unsigned char *pixels = low_storage(4096);
    Buffer b = {};
    b.dib_bits_ = pixels;
    b.stride_ = 8;
    b.dib_.bmiHeader = Header{8, -8};
    require(b.put_pixel(2, 4, 0xa5) == 0 && pixels[34] == 0xa5,
            "positive-pitch DIB pixel");

    b.dib_bits_ = pixels + 24;
    b.stride_ = static_cast<uint32_t>(-8);
    require(b.put_pixel(1, 2, 0xb6) == 0 && pixels[9] == 0xb6,
            "negative-pitch DIB pixel");

    TestSurface surface(pixels + 24, -8);
    Buffer locked = {};
    locked.surface_ = &surface;
    locked.dib_.bmiHeader = Header{8, -8};
    require(locked.put_pixel(3, 2, 0xc7) == 0 && pixels[11] == 0xc7,
            "surface Lock pixel uses returned pointer and signed pitch");
    require(surface.locks == 1 && surface.unlocks == 1 &&
            surface.unlocked == pixels + 24 && locked.surface_lock_count_ == 0 &&
            locked.locked_bits_ == 0,
            "successful Lock pixel is unlocked and cleared");

    Buffer nested = {};
    nested.surface_ = &surface;
    nested.locked_bits_ = pixels;
    nested.surface_lock_count_ = 2;
    nested.stride_ = 8;
    nested.dib_.bmiHeader = Header{8, -8};
    require(nested.put_pixel(4, 1, 0xd8) == 0 && pixels[12] == 0xd8,
            "nested-lock pixel storage pointer");
    require(surface.locks == 1 && surface.unlocks == 1 &&
            nested.surface_lock_count_ == 2 && nested.locked_bits_ == pixels,
            "nested-lock pixel retains holder");

    TestSurface failing(pixels, 8);
    failing.lock_result = 1;
    Buffer failed = {};
    failed.surface_ = &failing;
    failed.dib_.bmiHeader = Header{8, -8};
    require(failed.put_pixel(1, 1, 0xe9) == 0,
            "put_pixel keeps its historical Lock-error return");
    require(failing.locks == 1 && failing.unlocks == 0 &&
            failed.surface_lock_count_ == -1 && failed.locked_bits_ == 0,
            "put_pixel Lock-error cleanup decrement is retained");
}

int main() { test_fill_line(); test_put_pixel(); }
'''


def _translation_unit(mutations=()):
    source = SOURCE.read_text(encoding="utf-8")
    bodies = [_body(source, signature) for signature in SIGNATURES]
    for index, old, new in mutations:
        assert old in bodies[index]
        bodies[index] = bodies[index].replace(old, new, 1)
    # Host-only bridge for the native 32-bit API's unusual int return.
    bodies[0] = bodies[0].replace(
        "reinterpret_cast<int>(current)",
        "static_cast<int>(reinterpret_cast<uintptr_t>(current))",
    )
    return HARNESS + "\n" + "\n".join(bodies) + "\n"


def _run(tmp_path: Path, compiler_name: str, mutations=()):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / f"buffer_pixel_locals_{compiler_name}.cpp"
    exe = tmp_path / f"buffer_pixel_locals_{compiler_name}"
    cpp.write_text(_translation_unit(mutations), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], check=False, capture_output=True,
                          text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_pixel_methods_keep_typed_storage_and_lock_effects(
    compiler_name: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    fill = _body(source, SIGNATURES[0])
    put = _body(source, SIGNATURES[1])
    assert "unsigned char *locked;" in fill
    assert "locked + stride * y1 + x1" in fill
    assert "reinterpret_cast<int>(locked)" not in fill
    assert "void *cur = dib_bits_;" in put
    assert "const int row_offset = static_cast<int>(stride_ * a2);" in put
    assert "local_48 + row_offset + a1" in put
    assert "reinterpret_cast<int>(dib_bits_)" not in put
    result = _run(tmp_path, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    "mutations",
    (
        ((0, "locked + stride * y1 + x1", "locked + stride * x1 + y1"),),
        ((1, "static_cast<int>(stride_ * a2)",
          "static_cast<int>(stride_ * a1)"),),
    ),
)
def test_actual_pixel_address_mutants_are_detected(
    mutations, tmp_path: Path,
) -> None:
    result = _run(tmp_path, "g++", mutations)
    assert result.returncode != 0, result.stdout + result.stderr
