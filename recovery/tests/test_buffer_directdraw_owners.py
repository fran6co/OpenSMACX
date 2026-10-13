"""Actual-source controls for Buffer DirectDraw methods after typed migration.

The product methods are extracted whole; only Win32 pointer widths and the
descriptor size are bridged for the 64-bit host. COM calls are not rewritten. This does not
model the complete DirectDraw interface or claim VC6 object equivalence.
"""
from pathlib import Path
import os
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"


def body_after(signature):
    text = SOURCE.read_text()
    start = text.index(signature + " {")
    brace = text.index("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated product body: " + signature)


def actual_host_bodies():
    """Extract the integrated typed methods; bridge only x86 widths below."""
    pixel = body_after("int Buffer::put_pixel(int a1, int a2, int a3)")
    draw = body_after(
        "int Buffer::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7)")
    multi = body_after(
        "int Buffer::draw_multi_table_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7)")

    # Product COM dispatch and receiver ownership must already be typed.
    assert "IDirectDrawSurface *surface;" in pixel
    assert "surface = surface_;" in pixel and pixel.count("surface = surface_;") == 2
    assert "surface->Lock(nullptr, &description, 1, nullptr)" in pixel
    assert "surface->Unlock(locked_bits_)" in pixel
    assert "LockFn" not in pixel and "UnlockFn" not in pixel
    assert "description.dwSize = sizeof(description);" in pixel
    assert "DDSURFACEDESC description =" not in pixel
    assert "memset(&description" not in pixel
    assert pixel.index("surface = surface_;") < pixel.index("surface->Lock(")
    assert pixel.rindex("surface = surface_;") > pixel.index("after_write:")
    assert pixel.index("surface->Unlock(") > pixel.rindex("surface = surface_;")

    for body in (draw, multi):
        assert "IDirectDrawSurface *surface = surface_;" in body
        assert "LPVOID const bits = locked_bits_;" in body
        assert "LPVOID const cachedBits = locked_bits_;" in body
        assert "surface->Unlock(cachedBits)" in body
        assert "reinterpret_cast<int>(locked_bits_)" not in body
        assert "reinterpret_cast<void *>(cachedBits)" not in body
        assert "UnlockFn" not in body
        assert body.index("if (surface_ == 0)") < body.index(
            "IDirectDrawSurface *surface = surface_")
        assert body.index("IDirectDrawSurface *surface = surface_") < body.index(
            "if (surface == 0)") < body.index("surface->Unlock(")
        assert body.index("surface->Unlock(") < body.index("locked_bits_ = nullptr;")

    # Host-only bridges: the product is Win32/x86 but the host compiler is
    # 64-bit. Preserve its DWORD narrowing explicitly through uintptr_t.
    pixel = pixel.replace("reinterpret_cast<int>(dib_bits_)",
        "static_cast<int>(reinterpret_cast<uintptr_t>(dib_bits_))")
    pixel = pixel.replace("reinterpret_cast<int>(locked_bits_)",
        "static_cast<int>(reinterpret_cast<uintptr_t>(locked_bits_))")
    pixel = pixel.replace("(void *)cur",
        "reinterpret_cast<void *>(static_cast<uintptr_t>(cur))")
    pixel = pixel.replace("(char *)cur",
        "reinterpret_cast<char *>(static_cast<uintptr_t>(cur))")
    pixel = pixel.replace("description.dwSize = sizeof(description);",
                          "description.dwSize = 0x6c;")
    draw = draw.replace("reinterpret_cast<int>(locked_bits_)",
        "static_cast<int>(reinterpret_cast<uintptr_t>(locked_bits_))")
    multi = multi.replace("reinterpret_cast<int>(locked_bits_)",
        "static_cast<int>(reinterpret_cast<uintptr_t>(locked_bits_))")
    return pixel, draw, multi


HARNESS = r'''
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#define __stdcall
#define STDMETHODCALLTYPE __stdcall
static void require(bool condition, const char *what) {
    if (!condition) { fprintf(stderr, "failed: %s\n", what); abort(); }
}
typedef uint32_t DWORD;
typedef void *LPVOID;
struct DDSURFACEDESC { DWORD dwSize; int32_t lPitch; void *lpSurface; };
struct RECT { int left, top, right, bottom; };
struct Header { int biWidth, biHeight; };
struct Dib { Header bmiHeader; };
class IDirectDrawSurface {
public:
    virtual long STDMETHODCALLTYPE Lock(RECT *, DDSURFACEDESC *, DWORD, void *) = 0;
    virtual long STDMETHODCALLTYPE Unlock(void *) = 0;
};
struct Buffer;
struct Sprite {
    char *pcBits_;
    uint32_t iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_;
    uint32_t iWidth_, iHeight_, iLeftOffset_, iTopOffset_;
    char cTransparentIndex_;
    uint32_t fObj1Exists_;
    int call_kind, call_result;
    Buffer *seen_buffer;
    int args[5];
    unsigned char *table;
    char *seen_pc_bits;
    uint32_t seen_pitch, seen_width, seen_height, seen_transparent;
    void snapshot() {
        seen_pc_bits=pcBits_; seen_pitch=iSpriteWidth2_;
        seen_width=iSpriteWidth_; seen_height=iSpriteHeight_;
        seen_transparent=static_cast<unsigned char>(cTransparentIndex_);
    }
    int draw_dest(Buffer *b, int a2, int a3, int a4, int a5, int a6,
                  unsigned char *a7) {
        snapshot();
        call_kind = 1; seen_buffer = b;
        args[0]=a2; args[1]=a3; args[2]=a4; args[3]=a5; args[4]=a6;
        table=a7; return call_result;
    }
    int draw_multi_table_dest(Buffer *b, int a2, int a3, int a4, int a5,
                              int a6, unsigned char *a7) {
        snapshot();
        call_kind = 2; seen_buffer = b;
        args[0]=a2; args[1]=a3; args[2]=a4; args[3]=a5; args[4]=a6;
        table=a7; return call_result;
    }
};
static Sprite g_BUFFER_SPRITE = {};
struct BufferSpriteOwnerAdapter {
    Sprite &sprite() { return g_BUFFER_SPRITE; }
};
static BufferSpriteOwnerAdapter BufferSpriteOwner;
static int WinFillColour = 13;
struct Buffer {
    void *dib_bits_;
    IDirectDrawSurface *surface_;
    void *locked_bits_;
    int surface_lock_count_;
    uint32_t stride_;
    Dib dib_;
    int put_pixel(int,int,int);
    int draw_dest(Buffer *,int,int,int,int,int,unsigned char *);
    int draw_multi_table_dest(Buffer *,int,int,int,int,int,unsigned char *);
};
static Buffer *lock_mutation_buffer;
static IDirectDrawSurface *lock_mutation_target;
static bool mutate_surface_on_lock;
static int lock_status;
static int lock_calls, unlock_calls;
static void *last_unlock_receiver, *last_unlock_bits;
static unsigned char *low_page(size_t size) {
    void *p = mmap(0, size, PROT_READ|PROT_WRITE,
                   MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
    require(p != MAP_FAILED, "low-address pixel allocation");
    require(reinterpret_cast<uintptr_t>(p) < 0x80000000u,
            "Win32 DWORD pointer bridge range");
    return static_cast<unsigned char *>(p);
}
class FakeSurface : public IDirectDrawSurface {
public:
    unsigned char *pixels; int pitch;
    FakeSurface(unsigned char *p, int s) : pixels(p), pitch(s) {}
    long STDMETHODCALLTYPE Lock(RECT *rect, DDSURFACEDESC *desc,
                                DWORD flags, void *event) {
        ++lock_calls;
        require(rect == 0 && event == 0 && flags == 1, "Lock argument ABI");
        require(desc != 0 && desc->dwSize == 0x6c,
                "Lock sees only initialized dwSize contract");
        desc->lPitch = pitch;
        desc->lpSurface = pixels;
        if (mutate_surface_on_lock) {
            lock_mutation_buffer->surface_ = lock_mutation_target;
        }
        return lock_status;
    }
    long STDMETHODCALLTYPE Unlock(void *bits) {
        ++unlock_calls; last_unlock_receiver=this; last_unlock_bits=bits;
        return -77; // Original ignores this HRESULT.
    }
};
static void reset_fake() {
    lock_calls=unlock_calls=0; last_unlock_receiver=last_unlock_bits=0;
    lock_status=0; mutate_surface_on_lock=false;
    lock_mutation_buffer=0; lock_mutation_target=0;
    g_BUFFER_SPRITE.call_kind=0; g_BUFFER_SPRITE.call_result=0x2468;
    g_BUFFER_SPRITE.pcBits_=reinterpret_cast<char *>(1);
    g_BUFFER_SPRITE.seen_pc_bits=0; g_BUFFER_SPRITE.seen_pitch=0;
    g_BUFFER_SPRITE.seen_width=0; g_BUFFER_SPRITE.seen_height=0;
    g_BUFFER_SPRITE.seen_transparent=0;
}
'''


def host_source():
    pixel, draw, multi = actual_host_bodies()
    main = r'''
int main() {
    unsigned char *pixels = low_page(4096);
    unsigned char *dib = low_page(4096);
    FakeSurface first(pixels, 8), second(pixels, 12);
    Buffer b = {};
    b.dib_.bmiHeader = Header{8, -4}; b.stride_ = 8;

    // DIB-backed pixel write: no COM calls; the per-pixel hold is dropped.
    reset_fake(); b.dib_bits_=dib; b.surface_=0; b.surface_lock_count_=0;
    require(b.put_pixel(2,1,0x5a)==0, "DIB put_pixel result");
    require(dib[10]==0x5a && lock_calls==0 && unlock_calls==0,
            "DIB put_pixel store/no COM");
    require(b.locked_bits_==0 && b.surface_lock_count_==0,
            "DIB put_pixel releases holder");

    // Successful DirectDraw acquisition writes using Lock pitch, then
    // releases the exact data pointer. Unlock HRESULT is intentionally ignored.
    reset_fake(); b.dib_bits_=0; b.surface_=&first; b.locked_bits_=0;
    b.surface_lock_count_=0; first.pitch=9;
    require(b.put_pixel(3,2,0x61)==0, "surface put_pixel result");
    require(pixels[21]==0x61, "surface Lock pitch used for write");
    require(lock_calls==1 && unlock_calls==1 && last_unlock_receiver==&first &&
            last_unlock_bits==pixels, "surface lock/unlock pairing");
    require(b.locked_bits_==0 && b.surface_lock_count_==0,
            "successful unlock publishes reset");

    // A failed Lock still enters the post-write decrement path; no fake
    // balancing Unlock/reset is allowed when locked_bits_ stayed null.
    reset_fake(); b.surface_=&first; b.locked_bits_=0;
    b.surface_lock_count_=0; lock_status=-5;
    require(b.put_pixel(0,0,7)==0, "failed Lock retains original return");
    require(lock_calls==1 && unlock_calls==0 && b.surface_lock_count_==-1 &&
            b.locked_bits_==0, "failed Lock preserves decrement/no-reset path");

    // Lock's receiver is the value captured before the call; end cleanup
    // rereads Buffer::surface_ after Lock returns, as the original does.
    reset_fake(); b.surface_=&first; b.locked_bits_=0;
    b.surface_lock_count_=0; lock_mutation_buffer=&b;
    lock_mutation_target=&second; mutate_surface_on_lock=true;
    require(b.put_pixel(1,1,0x72)==0, "reloaded receiver result");
    require(unlock_calls==1 && last_unlock_receiver==&second &&
            last_unlock_bits==pixels, "post-Lock surface_ reload for Unlock");

    Buffer target = {};
    unsigned char table[4] = {1,2,3,4};
    // A preexisting surface takes the untouched forward path; Sprite receives
    // exact arguments and the wrapper clears only pcBits_ after the call.
    reset_fake(); b.surface_=&first; b.dib_bits_=dib; b.locked_bits_=pixels;
    b.surface_lock_count_=17; b.stride_=19;
    g_BUFFER_SPRITE.call_result=0x3456;
    require(b.draw_dest(&target, -1, -2, 3, 4, 5, table)==0x3456,
            "draw_dest forwards result");
    require(g_BUFFER_SPRITE.call_kind==1 && g_BUFFER_SPRITE.seen_buffer==&target &&
            g_BUFFER_SPRITE.args[0]==-1 && g_BUFFER_SPRITE.args[1]==-2 &&
            g_BUFFER_SPRITE.args[2]==3 && g_BUFFER_SPRITE.args[3]==4 &&
            g_BUFFER_SPRITE.args[4]==5 && g_BUFFER_SPRITE.table==table,
            "draw_dest exact Sprite arguments");
    require(g_BUFFER_SPRITE.pcBits_==0 && b.surface_lock_count_==17 &&
            b.locked_bits_==pixels && unlock_calls==0,
            "draw_dest surface path remains passthrough");
    require(g_BUFFER_SPRITE.seen_pc_bits==reinterpret_cast<char *>(1),
            "draw_dest invokes Sprite before only pcBits_ is cleared");

    // DIB branch stages the global Sprite, decrements the hold, and only
    // clears lock state after its original count condition.
    reset_fake(); b.surface_=0; b.dib_bits_=dib; b.locked_bits_=pixels;
    b.surface_lock_count_=0; b.stride_=31; b.dib_.bmiHeader=Header{6,-3};
    g_BUFFER_SPRITE.call_result=0x4567;
    require(b.draw_multi_table_dest(&target, -1, 8, 9, 10, 11, table)==0x4567,
            "multi-table draw result");
    require(g_BUFFER_SPRITE.call_kind==2 && g_BUFFER_SPRITE.seen_buffer==&target &&
            g_BUFFER_SPRITE.args[0]==-1 && g_BUFFER_SPRITE.args[1]==8 &&
            g_BUFFER_SPRITE.args[2]==9 && g_BUFFER_SPRITE.args[3]==10 &&
            g_BUFFER_SPRITE.args[4]==11 && g_BUFFER_SPRITE.table==table,
            "multi-table exact Sprite arguments");
    require(g_BUFFER_SPRITE.iSpriteWidth2_==31 &&
            g_BUFFER_SPRITE.iSpriteWidth_==6 &&
            g_BUFFER_SPRITE.iSpriteHeight_==3 &&
            g_BUFFER_SPRITE.cTransparentIndex_==WinFillColour,
            "multi-table staged Sprite metadata and color fallback");
    require(g_BUFFER_SPRITE.seen_pc_bits==reinterpret_cast<char *>(dib) &&
            g_BUFFER_SPRITE.seen_pitch==31 && g_BUFFER_SPRITE.seen_width==6 &&
            g_BUFFER_SPRITE.seen_height==3 &&
            g_BUFFER_SPRITE.seen_transparent==13,
            "multi-table Sprite observes metadata before post-call reset");
    require(g_BUFFER_SPRITE.pcBits_==0 && b.locked_bits_==0 &&
            b.surface_lock_count_==0 && unlock_calls==0,
            "multi-table DIB cleanup order");
    require(munmap(pixels, 4096)==0 && munmap(dib, 4096)==0,
            "release owned host pixel allocations");
}
'''
    return HARNESS + "\n" + pixel + "\n" + draw + "\n" + multi + "\n" + main


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_typed_directdraw_actual_source_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    cpp = tmp_path / "buffer_directdraw.cpp"
    exe = tmp_path / "buffer_directdraw"
    cpp.write_text(host_source())
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                    str(cpp), "-o", str(exe)], check=True)
    # LSan's ptrace probe is unsupported in this isolated runner; retain ASan
    # and UBSan runtime checks while disabling only leak-process inspection.
    child_env = dict(os.environ)
    child_env["ASAN_OPTIONS"] = "detect_leaks=0"
    subprocess.run([str(exe)], check=True, env=child_env)
