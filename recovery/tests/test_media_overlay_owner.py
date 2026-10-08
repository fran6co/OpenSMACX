"""Actual-source overlay controls with COM doubles, not native ABI/game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body():
    text = (ROOT / "src/movie_media.cpp").read_text()
    start = text.index("int __fastcall MediaRuntime::update_overlay(int show) {")
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated overlay body")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#define __fastcall
typedef uint32_t DWORD; typedef int32_t LONG; typedef int32_t HRESULT;
const HRESULT DD_OK=0, E_INVALIDARG=static_cast<HRESULT>(0x80070057U);
const HRESULT DDERR_SURFACELOST=static_cast<HRESULT>(0x887601C2U);
const DWORD DDOVER_HIDE=0x200, DDOVER_SHOW=0x4000, DDOVER_DDFX=0x80000;
const DWORD DDOVERFX_ARITHSTRETCHY=1;
struct RECT { LONG left,top,right,bottom; };
struct DDOVERLAYFX {
  DWORD dwSize, edge_depth, edge, reserved, dest_depth, dest_const;
  DWORD src_depth, src_const, dest_key_low, dest_key_high, src_key_low, src_key_high;
  DWORD dwDDFX,dwFlags;
};
static_assert(sizeof(DDOVERLAYFX)==0x38,"projected 32-bit effects storage");
struct IDirectDrawSurface2 {
  virtual HRESULT UpdateOverlay(RECT*,IDirectDrawSurface2*,RECT*,DWORD,DDOVERLAYFX*)=0;
  virtual HRESULT Restore()=0;
};
struct MediaDescriptor {
  IDirectDrawSurface2 *surface_14,*surface_1c;
  DWORD output_width_64,output_height_60;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD output_rect_left_14,output_rect_top_18,output_rect_extent_0c;
  DWORD output_rect_or_draw2_bits_10,queue_record_kind;
  int update_overlay(int show);
};
struct Surface : IDirectDrawSurface2 {
  HRESULT result; int restores,calls; RECT source,destination;
  IDirectDrawSurface2 *target; DWORD flags; DDOVERLAYFX effects;
  MediaRuntime *runtime; MediaDescriptor *replacement;
  Surface():result(0),restores(0),calls(0),target(0),flags(0),runtime(0),replacement(0) {}
  HRESULT UpdateOverlay(RECT *s,IDirectDrawSurface2 *t,RECT *d,DWORD f,DDOVERLAYFX *e) {
    assert(s!=d); ++calls; source=*s; destination=*d; target=t; flags=f; effects=*e;
    if (replacement) runtime->descriptor=replacement;
    return result;
  }
  HRESULT Restore() { ++restores; return E_INVALIDARG; }
};
ACTUAL_BODY
int main() {
  Surface overlay,target,rebound;
  MediaDescriptor d={&target,&overlay,64,48}, changed={&target,&rebound,99,77};
  MediaRuntime r={&d,11,13,37,29,2};
  assert(r.update_overlay(1)==0);
  assert(overlay.source.left==11 && overlay.source.top==13);
  assert(overlay.source.right==48 && overlay.source.bottom==42);
  assert(overlay.destination.left==0 && overlay.destination.top==0);
  assert(overlay.destination.right==63 && overlay.destination.bottom==47);
  assert(overlay.target==&target && overlay.flags==0x84000);
  DDOVERLAYFX expected={}; expected.dwSize=0x38; expected.dwDDFX=1;
  assert(std::memcmp(&expected,&overlay.effects,sizeof(expected))==0);
  assert(r.queue_record_kind==2 && overlay.restores==0);
  assert(r.update_overlay(0)==0 && overlay.flags==0x200);
  expected.dwDDFX=0;
  assert(std::memcmp(&expected,&overlay.effects,sizeof(expected))==0);
  overlay.result=E_INVALIDARG;
  assert(r.update_overlay(-1)==1 && r.queue_record_kind==1 && overlay.restores==0);
  r.queue_record_kind=2; overlay.result=DDERR_SURFACELOST;
  assert(r.update_overlay(1)==1 && overlay.restores==1 && r.queue_record_kind==2);
  overlay.runtime=&r; overlay.replacement=&changed;
  assert(r.update_overlay(1)==1 && rebound.restores==1 && overlay.restores==1);
  assert(overlay.target==&target && r.descriptor==&changed);
  r.descriptor=&d; overlay.replacement=0; overlay.result=1;
  assert(r.update_overlay(1)==1 && r.queue_record_kind==2 && overlay.restores==1);
  overlay.result=0; d.output_width_64=0; d.output_height_60=0;
  r.output_rect_left_14=0xFFFFFFF0U; r.output_rect_extent_0c=0x30;
  assert(r.update_overlay(1)==0);
  assert(overlay.source.right==0x20);
  assert(overlay.destination.right==-1 && overlay.destination.bottom==-1);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_overlay_operation_and_failure_receivers(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    text = (ROOT / "src/stringstruct.cpp").read_text()
    assert "BltSlotShim" not in text and "RestoreSlotShim" not in text
    source = tmp_path / "overlay.cpp"
    source.write_text(HARNESS.replace("ACTUAL_BODY", body()))
    exe = tmp_path / "overlay"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    subprocess.run([str(exe)], check=True, timeout=10)
