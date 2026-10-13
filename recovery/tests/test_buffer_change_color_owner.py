"""Actual Buffer color-replacement bodies with host SDK doubles.

Host pointer narrowing models Win32 low-address storage; this is not an SDK
vtable, native ABI, full game or matching proof. IntersectRect is invoked by
the actual product expression, without silently repairing it in the harness.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"


def body_after(signature):
    text = SOURCE.read_text()
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated product function: " + signature)


def actual_bodies():
    bodies = [body_after(signature) for signature in (
        "int Buffer::change_color(int a1, int a2) {",
        "int Buffer::change_color(int a1, int a2, int a3, int a4, int a5, int a6) {")]
    assert "if (!IntersectRect(&rect, &rect1_, &rect))" in bodies[0]
    for body in bodies:
        assert "ordered[" not in body and "DDLockFn" not in body
    assert "LPVOID const storage = dib_bits_;" in bodies[0]
    assert "locked_bits_ = storage;" in bodies[0]
    assert "const uint32_t existing = reinterpret_cast<uint32_t>(locked_bits_);" in bodies[0]
    assert "pixel_base = static_cast<int>(existing);" in bodies[0]
    assert "LPVOID const data = descriptor.lpSurface;" in bodies[0]
    assert "LPVOID const data2 = locked_bits_;" in bodies[0]
    assert "surface2->Unlock(data2);" in bodies[0]
    assert "reinterpret_cast<void *>(storage)" not in bodies[0]
    assert "reinterpret_cast<uint32_t>(descriptor.lpSurface)" not in bodies[0]
    assert "LPVOID const bits = dib_bits_;" in bodies[1]
    assert "locked_bits_ = bits;" in bodies[1]
    # Bridge only the pointer-to-int numeric pixel_base conversions. The
    # low-address mapping below keeps those genuine 32-bit address operations
    # meaningful on the 64-bit host; all storage/Unlock values stay LPVOID.
    for index in range(len(bodies)):
        bodies[index] = bodies[index].replace(
            "reinterpret_cast<uint32_t>(locked_bits_)",
            "static_cast<uint32_t>(reinterpret_cast<uintptr_t>(locked_bits_))",
        )
        for pointer_name in ("storage", "data"):
            bodies[index] = bodies[index].replace(
                f"reinterpret_cast<int>({pointer_name})",
                f"static_cast<int>(reinterpret_cast<uintptr_t>({pointer_name}))",
            )
    return bodies


HARNESS = r'''
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#define __stdcall

static void require(bool value, const char *what) {
    if (!value) { fprintf(stderr, "failed: %s\n", what); abort(); }
}
typedef unsigned char BYTE;
typedef uint32_t DWORD;
typedef void *LPVOID;
struct RECT { int left, top, right, bottom; };
struct DDPIXELFORMAT { uint32_t words[8]; };
struct DDCOLORKEY { uint32_t low, high; };
#pragma pack(push, 4)
struct DDSURFACEDESC {
    uint32_t dwSize, dwFlags, dwHeight, dwWidth;
    int32_t lPitch;
    uint32_t dwBackBufferCount, dwMipMapCount, dwAlphaBitDepth, dwReserved;
    void *lpSurface;
    DDCOLORKEY ddckCKDestOverlay, ddckCKDestBlt, ddckCKSrcOverlay, ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    uint32_t ddsCaps;
};
#pragma pack(pop)
static_assert(offsetof(DDSURFACEDESC, dwSize) == 0x00, "DDSURFACEDESC::dwSize");
static_assert(offsetof(DDSURFACEDESC, lPitch) == 0x10, "DDSURFACEDESC::lPitch");
static_assert(offsetof(DDSURFACEDESC, lpSurface) == 0x24, "DDSURFACEDESC::lpSurface");
static_assert(sizeof(void *) != 4 || sizeof(DDSURFACEDESC) == 0x6c,
              "32-bit SDK DDSURFACEDESC extent");
struct Header { int biWidth; int biHeight; };
struct Dib { Header bmiHeader; };
static int intersect_rect(RECT *out, RECT *a, RECT *b) {
    out->left = a->left > b->left ? a->left : b->left;
    out->top = a->top > b->top ? a->top : b->top;
    out->right = a->right < b->right ? a->right : b->right;
    out->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    return out->left < out->right && out->top < out->bottom;
}
static int IntersectRect(RECT *out, RECT *a, RECT *b) {
    return intersect_rect(out, a, b);
}
struct Event { int kind; void *arg0; void *arg1; unsigned flags; };
static Event events[16];
static int event_count;
static int lock_result;
static bool null_pixels;
class IDirectDrawSurface;
static IDirectDrawSurface **mutated_surface_slot;
static IDirectDrawSurface *mutated_surface_value;
static unsigned char *low_page(size_t bytes) {
    void *p = mmap(0, bytes, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    require(p != MAP_FAILED, "low-address mapping");
    require(reinterpret_cast<uintptr_t>(p) <= 0x7fffffff, "32-bit pointer bridge");
    return static_cast<unsigned char *>(p);
}
class IDirectDrawSurface {
public:
    virtual long Lock(RECT *, DDSURFACEDESC *, DWORD, void *) = 0;
    virtual long Unlock(void *) = 0;
};
class FakeSurface : public IDirectDrawSurface {
public:
    unsigned char *pixels;
    int pitch;
    FakeSurface(unsigned char *p, int s) : pixels(p), pitch(s) {}
    long Lock(RECT *area, DDSURFACEDESC *desc, DWORD flags, void *event) {
        events[event_count++] = Event{1, area, event, flags};
        require(desc != 0, "Lock descriptor");
        require(desc->dwSize == 0x6c, "Lock dwSize initialized as original");
        desc->lPitch = pitch;
        desc->lpSurface = null_pixels ? 0 : pixels;
        if (mutated_surface_slot != 0) {
            *mutated_surface_slot = mutated_surface_value;
            mutated_surface_slot = 0;
        }
        return lock_result;
    }
    long Unlock(void *data) {
        events[event_count++] = Event{2, data, this, 0};
        return 0;
    }
};
static void reset_events() {
    event_count = 0; lock_result = 0; null_pixels = false;
    mutated_surface_slot = 0; mutated_surface_value = 0;
}

class BufferA {
public:
    void *locked_bits_;
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    int surface_lock_count_;
    uint32_t stride_;
    RECT rect1_;
    Dib dib_;
    int change_color(int, int);
};
class BufferB {
public:
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    void *locked_bits_;
    int surface_lock_count_;
    uint32_t stride_;
    RECT rect1_;
    Dib dib_;
    int change_color(int, int, int, int, int, int);
};
'''


def host_source():
    first, second = actual_bodies()
    first = first.replace("int Buffer::change_color", "int BufferA::change_color")
    second = second.replace("int Buffer::change_color", "int BufferB::change_color")
    main = r'''
int main() {
    unsigned char *pixel = low_page(4096);
    FakeSurface *surface = new (low_page(4096)) FakeSurface(pixel, 8);
    FakeSurface *second_surface = new (low_page(4096)) FakeSurface(pixel, 12);
    pixel[0] = 7; pixel[1] = 8; pixel[8] = 7; pixel[9] = 9;

    // First overload: Lock error returns 7 before count/pixel changes.
    BufferA a = {};
    a.rect1_ = RECT{0,0,2,2}; a.dib_.bmiHeader = Header{2,-2};
    a.surface_ = surface;
    reset_events(); lock_result = -1;
    require(a.change_color(7, 3) == 7, "two-arg failed Lock result");
    require(event_count == 1 && events[0].kind == 1 && events[0].arg0 == 0 &&
            events[0].arg1 == 0 && events[0].flags == 1, "two-arg Lock contract");
    require(a.surface_lock_count_ == 0 && pixel[0] == 7, "two-arg Lock failure effects");

    // Success mutates the clipped pixels, then unlocks the descriptor pointer
    // and clears both lock fields in the original order.
    reset_events();
    require(a.change_color(7, 3) == 0, "two-arg successful replacement");
    require(pixel[0] == 3 && pixel[1] == 8 && pixel[8] == 3 && pixel[9] == 9,
            "two-arg row/pixel effects");
    require(event_count == 2 && events[0].kind == 1 && events[1].kind == 2 &&
            events[1].arg0 == pixel, "two-arg Lock then Unlock");
    require(a.surface_lock_count_ == 0 && a.locked_bits_ == 0,
            "two-arg final lock fields");

    // A Lock callback can change surface_; the product uses the captured
    // first receiver for Lock, then re-reads surface_ for the later Unlock.
    reset_events();
    a.surface_ = surface; a.locked_bits_ = 0;
    mutated_surface_slot = &a.surface_;
    mutated_surface_value = second_surface;
    require(a.change_color(7, 3) == 0, "two-arg callback receiver reload");
    require(events[0].arg1 == 0 && event_count == 2 && events[1].kind == 2 &&
            events[1].arg1 == second_surface && events[1].arg0 == pixel,
            "Unlock uses the current surface and captured descriptor pointer");

    // The DIB pointer is stored directly, while only pixel_base remains a
    // genuine 32-bit numeric address in this host bridge.
    BufferA dib_a = {};
    dib_a.dib_bits_ = pixel; dib_a.rect1_ = RECT{0,0,2,2};
    dib_a.dib_.bmiHeader = Header{2,-2}; dib_a.stride_ = 8;
    pixel[0] = 7; reset_events();
    require(dib_a.change_color(7, 6) == 0, "two-arg DIB storage path");
    require(pixel[0] == 6 && dib_a.surface_lock_count_ == 0 &&
            dib_a.locked_bits_ == 0 && event_count == 0,
            "two-arg DIB direct storage and original cleanup");

    // Existing-lock path deliberately performs no COM calls and retains one
    // holder after this operation.
    pixel[0] = 7; a.locked_bits_ = pixel; a.surface_lock_count_ = 1; reset_events();
    require(a.change_color(7, 4) == 0, "two-arg nested holder result");
    require(event_count == 0 && a.surface_lock_count_ == 1 && pixel[0] == 4,
            "two-arg nested holder behavior");

    // Six-argument overload has its own Lock failure path and the source's
    // successful null-pixel quirk (returns 7 after increment, no Unlock).
    BufferB b = {};
    b.surface_ = surface; b.rect1_ = RECT{0,0,2,2};
    b.dib_.bmiHeader = Header{2,-2};
    reset_events(); lock_result = -1;
    require(b.change_color(0,0,2,2,7,5) == 7, "six-arg failed Lock result");
    require(event_count == 1 && events[0].kind == 1 && events[0].arg0 == 0 &&
            events[0].arg1 == 0 && events[0].flags == 1 &&
            b.surface_lock_count_ == 0, "six-arg failed Lock effects");

    reset_events(); null_pixels = true;
    require(b.change_color(0,0,2,2,7,5) == 7, "six-arg null-pixel result");
    require(event_count == 1 && b.surface_lock_count_ == 1 &&
            b.locked_bits_ == 0, "six-arg null-pixel error-path state retained");

    reset_events(); null_pixels = false;
    pixel[0] = 7; pixel[1] = 8; pixel[8] = 7; pixel[9] = 9;
    b.surface_lock_count_ = 0; b.locked_bits_ = 0;
    require(b.change_color(0,0,2,2,7,5) == 0, "six-arg replacement");
    require(pixel[0] == 5 && pixel[1] == 8 && pixel[8] == 5 && pixel[9] == 9,
            "six-arg row/pixel effects");
    require(event_count == 2 && events[0].kind == 1 && events[1].kind == 2 &&
            events[1].arg0 == pixel && b.surface_lock_count_ == 0 &&
            b.locked_bits_ == 0, "six-arg Lock/Unlock and reset");

    // The six-argument overload's already-held path also increments and
    // decrements its own count, without issuing either interface call.
    pixel[0] = 7; b.locked_bits_ = pixel; b.surface_lock_count_ = 1;
    reset_events();
    require(b.change_color(0,0,2,2,7,4) == 0, "six-arg nested holder");
    require(event_count == 0 && b.surface_lock_count_ == 1 &&
            b.locked_bits_ == pixel && pixel[0] == 4,
            "six-arg nested holder behavior");

    // Six-arg no-surface path does not call the interface and restores its
    // own count/bits, unlike the surface path above.
    BufferB dib = {};
    dib.dib_bits_ = pixel; dib.rect1_ = RECT{0,0,2,2};
    dib.dib_.bmiHeader = Header{2,-2}; dib.stride_ = 8;
    pixel[0] = 7; reset_events();
    require(dib.change_color(0,0,2,2,7,6) == 0, "six-arg DIB path");
    require(event_count == 0 && pixel[0] == 6 && dib.surface_lock_count_ == 0 &&
            dib.locked_bits_ == 0, "six-arg DIB lock cleanup");
    return 0;
}
'''
    return HARNESS + "\n" + first + "\n" + second + "\n" + main


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_typed_dispatch_contracts(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    with tempfile.TemporaryDirectory(prefix="buffer-typed-candidate-") as td:
        cpp = Path(td) / "candidate.cpp"
        exe = Path(td) / "candidate"
        generated = host_source()
        assert "ordered[" not in generated
        assert "uint8_t descriptor[0x6C]" not in generated
        assert "unsigned char descBuf[0x6c]" not in generated
        assert "DDSURFACEDESC descriptor;\n            descriptor.dwSize = 0x6C;" in generated
        assert "DDSURFACEDESC descBuf;" in generated
        assert "descBuf.dwSize = 0x6c;" in generated
        cpp.write_text(generated)
        subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
                       check=True)
        subprocess.run([str(exe)], check=True)
