"""Actual presentation body controls; not COM ABI or full codec/game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_caviar_draw_callbacks_actual_source import extract
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "21651a88042981bae291f49ca649ef7d1f269d1b"
SIGNATURE = "void MediaRuntime::sub_5cb6c0()"


def test_source_identity_and_bounded_change():
    path = ROOT / "src/movie_media.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/movie_media.cpp"], cwd=ROOT, text=True)
    after = path.read_text()
    assert before.replace(extract(before, SIGNATURE), "PRESENT_BODY").replace(
        "output_rect_or_draw2_bits_10", "output_rect_height_10") == after.replace(
        extract(after, SIGNATURE), "PRESENT_BODY")
    assert [(r.address, _continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(before, path)] == [
        (r.address, _continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(after, path)]
    old_header = subprocess.check_output(["git", "show", BASE + ":src/movie_media.h"], cwd=ROOT, text=True)
    assert (ROOT / "src/movie_media.h").read_text() == old_header.replace(
        "DWORD output_rect_or_draw2_bits_10;             // +010; rect height or COM pointer bits",
        "DWORD output_rect_height_10;                   // +010; destination rectangle height")


def test_pinned_descriptor_receiver_height_and_saved_palette():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x5CB6C0, 0x5CBAD1)), 0x5CB6C0)}
    for address, instruction in {
        0x5CB6D1: ("mov", "ebx, dword ptr [eax + 0x28]"),
        0x5CB700: ("mov", "eax, dword ptr [eax + 0x10]"),
        0x5CB70D: ("mov", "eax, dword ptr [esi]"),
        0x5CB738: ("mov", "ecx, dword ptr [esi + 0x10]"),
        0x5CB759: ("mov", "eax, dword ptr [esi]"),
        0x5CB76C: ("mov", "eax, dword ptr [esi]"),
        0x5CBA60: ("mov", "eax, dword ptr [esi]"),
        0x5CBA86: ("mov", "eax, dword ptr [esi]"),
        0x5CBA88: ("mov", "edx, dword ptr [esp + 0x3c]"),
    }.items():
        assert instructions[address] == instruction


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
typedef uint32_t DWORD; typedef int32_t LONG; typedef int32_t HRESULT;
typedef unsigned char BYTE; typedef void *LPPALETTEENTRY;
const HRESULT DD_OK=0, DDERR_SURFACELOST=static_cast<HRESULT>(0x887601C2U);
const DWORD DDWAITVB_BLOCKBEGIN=1, DDBLT_ASYNC=0x1000000, DDFLIP_WAIT=1, DDLOCK_WAIT=1;
struct RECT { LONG left,top,right,bottom; };
struct DDSURFACEDESC { DWORD dwSize; void *lpSurface; BYTE unused[100]; };
struct MediaDescriptor;
struct MediaRuntime;
static MediaRuntime *active;
struct IDirectDraw2 { virtual HRESULT WaitForVerticalBlank(DWORD,void*)=0; };
struct IDirectDrawSurface2 {
  virtual HRESULT IsLost()=0;
  virtual HRESULT Restore()=0;
  virtual HRESULT Blt(RECT*,IDirectDrawSurface2*,RECT*,DWORD,void*)=0;
  virtual HRESULT Flip(IDirectDrawSurface2*,DWORD)=0;
  HRESULT Lock(void*,DDSURFACEDESC*,DWORD,void*) { assert(false); return -1; }
  HRESULT Unlock(void*) { assert(false); return -1; }
};
struct IDirectDrawPalette { virtual HRESULT SetEntries(DWORD,DWORD,DWORD,LPPALETTEENTRY)=0; };
struct MediaDescriptor {
  DWORD flags; IDirectDraw2 *draw_device_10;
  IDirectDrawSurface2 *surface_14,*surface_18,*surface_1c;
  IDirectDrawPalette *palette_24; BYTE *palette_entries_28;
  struct { DWORD total_component_bits; } pixel_format;
  BYTE palette_rgb_triplets_b0[0x300]; DWORD output_width_64,output_height_60;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD output_handle_candidate,queue_record_kind,output_rect_left_14,output_rect_top_18;
  DWORD output_rect_extent_0c,output_rect_height_10; BYTE *owned_heap_638;
  int update_overlay(int) { assert(false); return 0; }
  void sub_5cb6c0();
};
static int errors;
static const char *sub_5cf120(HRESULT status) { assert(status==-7); return "error"; }
static void OutputDebugStringA(const char *message) { assert(!std::strcmp(message,"error")); ++errors; }
struct Draw : IDirectDraw2 {
  int calls=0; MediaDescriptor *replacement=0;
  HRESULT WaitForVerticalBlank(DWORD flags,void *event) {
    assert(flags==1 && !event); ++calls;
    if(replacement) active->descriptor=replacement;
    return 0;
  }
};
struct Palette : IDirectDrawPalette {
  int calls=0; void *entries=0;
  HRESULT SetEntries(DWORD flags,DWORD start,DWORD count,LPPALETTEENTRY p) {
    assert(!flags && !start && count==256); ++calls; entries=p; return 0;
  }
};
struct Surface : IDirectDrawSurface2 {
  int lost_calls=0,restore_calls=0,blt_calls=0,flip_calls=0;
  HRESULT lost_result=0,restore_result=0,blt_result=0,flip_result=0;
  MediaDescriptor *lost_replacement=0,*restore_replacement=0,*blt_replacement=0,*flip_replacement=0;
  RECT destination={},source={}; IDirectDrawSurface2 *blt_source=0;
  HRESULT IsLost() { ++lost_calls; if(lost_replacement) active->descriptor=lost_replacement; return lost_result; }
  HRESULT Restore() { ++restore_calls; if(restore_replacement) active->descriptor=restore_replacement; return restore_result; }
  HRESULT Blt(RECT *d,IDirectDrawSurface2 *s,RECT *r,DWORD flags,void *fx) {
    assert(flags==DDBLT_ASYNC && !fx); ++blt_calls; destination=*d; source=*r; blt_source=s;
    if(blt_replacement) active->descriptor=blt_replacement;
    return blt_result;
  }
  HRESULT Flip(IDirectDrawSurface2 *target,DWORD flags) {
    assert(!target && flags==DDFLIP_WAIT); ++flip_calls;
    if(flip_replacement) active->descriptor=flip_replacement;
    return flip_result;
  }
};
ACTUAL_BODY
int main() {
  Draw draw; Surface a,b,c,target,final_target; Palette old_palette,final_palette;
  BYTE original_entries[1024]={},new_entries[1024]={};
  MediaDescriptor initial={},after_wait={},after_lost={},after_restore={},after_blt={},after_flip={};
  initial.flags=0x1000; initial.draw_device_10=&draw; initial.palette_entries_28=original_entries;
  after_wait.surface_1c=&a; after_wait.output_width_64=64; after_wait.output_height_60=48;
  after_lost.surface_1c=&b; after_lost.output_width_64=99; after_lost.output_height_60=77;
  after_restore.surface_1c=&c; after_restore.surface_14=&target;
  after_blt.flags=0x08000000; after_blt.pixel_format.total_component_bits=8;
  after_blt.surface_14=&final_target; after_blt.palette_24=&old_palette;
  after_flip.palette_24=&final_palette; after_flip.palette_entries_28=new_entries;
  MediaRuntime runtime={&initial,0,4,11,13,37,29,0}; active=&runtime;
  draw.replacement=&after_wait; a.lost_result=DDERR_SURFACELOST; a.lost_replacement=&after_lost;
  b.restore_replacement=&after_restore; target.blt_result=-1; target.blt_replacement=&after_blt;
  final_target.flip_replacement=&after_flip;
  runtime.sub_5cb6c0();
  assert(draw.calls==1 && a.lost_calls==1 && b.restore_calls==1);
  assert(target.blt_calls==1 && target.blt_source==&c);
  assert(target.source.left==0 && target.source.top==0 && target.source.right==63 && target.source.bottom==47);
  assert(target.destination.left==11 && target.destination.top==13 && target.destination.right==48 && target.destination.bottom==42);
  assert(runtime.queue_record_kind==1 && final_target.flip_calls==1);
  assert(final_palette.calls==1 && old_palette.calls==0 && final_palette.entries==original_entries);
  runtime.descriptor=&initial; runtime.queue_record_kind=4; b.restore_result=-1;
  runtime.sub_5cb6c0(); assert(target.blt_calls==1 && final_target.flip_calls==1);
  runtime.descriptor=&initial; runtime.queue_record_kind=4; b.restore_result=0;
  after_blt.pixel_format.total_component_bits=16; final_target.flip_replacement=0; final_target.flip_result=-7;
  runtime.sub_5cb6c0(); assert(errors==1 && final_palette.calls==1);
  initial.flags=0x800; runtime.descriptor=&initial; runtime.queue_record_kind=4;
  runtime.sub_5cb6c0(); assert(draw.calls==3);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("none", "stale_surface", "late_rectangle", "stale_flags", "changed_palette_buffer", "stale_palette"))
def test_actual_kind4_and_final_tail(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    body = extract((ROOT / "src/movie_media.cpp").read_text(), SIGNATURE)
    mutations = {
        "stale_surface": ("HRESULT status = descriptor->surface_1c->IsLost();", "HRESULT status = d.surface_1c->IsLost();"),
        "late_rectangle": ("source.right = (LONG)descriptor->output_width_64 - 1;", "source.right = (LONG)99 - 1;"),
        "stale_flags": ("if ((descriptor->flags & 0x08000000) != 0", "if ((flags & 0x08000000) != 0"),
        "changed_palette_buffer": ("(LPPALETTEENTRY)saved_palette_entries", "(LPPALETTEENTRY)descriptor->palette_entries_28"),
        "stale_palette": ("descriptor->palette_24->SetEntries", "d.palette_24->SetEntries"),
    }
    if damage != "none":
        old, new = mutations[damage]
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    source = tmp_path / "present.cpp"
    source.write_text(HARNESS.replace("ACTUAL_BODY", body))
    exe = tmp_path / "present"
    built = subprocess.run([compiler, "-std=c++11", str(source), "-o", str(exe)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage == "none")
