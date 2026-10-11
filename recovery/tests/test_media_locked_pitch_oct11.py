"""Actual media copy body: returned pitch, receiver reloads and failure tail.

Host doubles are bounded controls, not native COM ABI or full-game acceptance.
The copy intentionally retains the original eight-dword chunk rounding.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract
from test_media_present_descriptor_oct11 import HARNESS, ROOT, SIGNATURE


def test_pinned_pitch_rounding_and_failed_lock_tail():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        image.read(Span(0x5CB6C0,0x5CBAD1)),0x5CB6C0)}
    for address,instruction in {
        0x5CB7CF:("mov","ecx, dword ptr [esi]"),
        0x5CB7ED:("jne","0x5cba60"),
        0x5CB7F3:("mov","eax, dword ptr [esp + 0x54]"),
        0x5CB7F7:("mov","ecx, dword ptr [esi]"),
        0x5CB80A:("cdq",""),
        0x5CB80B:("and","edx, 3"),
        0x5CB818:("add","eax, edx"),
        0x5CB820:("sar","eax, 2"),
        0x5CB897:("mov","edx, dword ptr [esi]"),
        0x5CB89A:("lea","ebx, [ebx + eax*4]"),
        0x5CB8A8:("mov","eax, dword ptr [esi]"),
    }.items():
        assert instructions[address]==instruction


def test_bounded_kind2_change_and_claim_continuity():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path=ROOT/"src/movie_media.cpp"
    base="6b15ca08d2df432a9ee175bb4ce430c164cf1eea"
    old=subprocess.check_output(["git","show",base+":src/movie_media.cpp"],cwd=ROOT,text=True)
    new=path.read_text()
    def outside_kind2(text):
        start=text.index("  } else if (queue_record_kind == 2) {",text.index(SIGNATURE))
        end=text.index("  } else if (queue_record_kind == 4) {",start)
        return text[:start]+"KIND2"+text[end:]
    assert outside_kind2(old)==outside_kind2(new)
    assert [(r.address,_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(old,path)]==[
        (r.address,_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(new,path)]


def projection():
    preamble = HARNESS[:HARNESS.index("int main()")]
    preamble = preamble.replace(
        "HRESULT Lock(void*,DDSURFACEDESC*,DWORD,void*) { assert(false); return -1; }",
        "virtual HRESULT Lock(void*,DDSURFACEDESC*,DWORD,void*)=0;")
    preamble = preamble.replace(
        "HRESULT Unlock(void*) { assert(false); return -1; }",
        "virtual HRESULT Unlock(void*)=0;")
    preamble = preamble.replace(
        "int update_overlay(int) { assert(false); return 0; }",
        "int update_overlay(int show);")
    preamble = preamble.replace("  HRESULT IsLost() {", r'''
  int lock_calls=0,unlock_calls=0; HRESULT lock_result=0;
  BYTE *locked_data=0; LONG pitch=0; MediaDescriptor *lock_replacement=0;
  HRESULT Lock(void *rectangle,DDSURFACEDESC *out,DWORD flags,void *event) {
    assert(!rectangle && !event && flags==DDLOCK_WAIT && out->dwSize==0x6c);
    ++lock_calls; out->lpSurface=locked_data; out->lPitch=pitch;
    if(lock_replacement) active->descriptor=lock_replacement;
    return lock_result;
  }
  HRESULT Unlock(void *rectangle) { assert(!rectangle); ++unlock_calls; return -3; }
  HRESULT IsLost() {''')
    return preamble + r'''
static MediaDescriptor *overlay_replacement;
static int overlay_calls;
int MediaRuntime::update_overlay(int show) {
  assert(show==1); ++overlay_calls;
  if(overlay_replacement) descriptor=overlay_replacement;
  return 0;
}
int main() {
  alignas(4) BYTE input[512],output[512],expected[512];
  for(unsigned i=0;i<sizeof(input);++i) input[i]=static_cast<BYTE>(i*7+3);
  Surface initial_surface,lock_surface,unlock_surface,flip_surface;
  MediaDescriptor initial={},before_lock={},after_lock={};
  initial.surface_1c=&initial_surface; initial.output_width_64=2; initial.output_height_60=9;
  before_lock.surface_1c=&lock_surface;
  after_lock.surface_1c=&unlock_surface; after_lock.output_width_64=19; after_lock.output_height_60=3;
  MediaRuntime runtime={&initial,1,2,0,0,0,0,input}; active=&runtime;
  overlay_replacement=&before_lock; lock_surface.lock_replacement=&after_lock;
  for(LONG pitch : {84,87,-84,-87}) {
    std::memset(output,0xA5,sizeof(output)); std::memcpy(expected,output,sizeof(output));
    runtime.descriptor=&initial; runtime.output_handle_candidate=1; runtime.queue_record_kind=2;
    const unsigned offset=pitch<0?256:0;
    lock_surface.locked_data=output+offset; lock_surface.pitch=pitch;
    runtime.sub_5cb6c0();
    // Width19 -> nine dwords; native loop copies two full eight-dword chunks.
    for(unsigned row=0;row<3;++row)
      std::memcpy(expected+offset+static_cast<int>(row)*(pitch/4*4),input+row*36,64);
    assert(!std::memcmp(output,expected,sizeof(output)));
    assert(initial_surface.lock_calls==0 && lock_surface.unlock_calls==0);
    assert(runtime.output_handle_candidate==0);
  }
  assert(lock_surface.lock_calls==4 && unlock_surface.unlock_calls==4 && overlay_calls==4);
  // Empty height still unlocks; zero width writes nothing.
  for(DWORD width : {DWORD(0),DWORD(19)}) {
    std::memset(output,0xA5,sizeof(output)); std::memcpy(expected,output,sizeof(output));
    after_lock.output_width_64=width; after_lock.output_height_60=width?0:3;
    runtime.descriptor=&before_lock; runtime.output_handle_candidate=0;
    lock_surface.locked_data=output; lock_surface.pitch=84;
    runtime.sub_5cb6c0(); assert(!std::memcmp(output,expected,sizeof(output)));
  }
  assert(unlock_surface.unlock_calls==6);
  // Lock failure continues to the shared tail, rather than returning early.
  after_lock.flags=0x08000000; after_lock.pixel_format.total_component_bits=16;
  after_lock.surface_14=&flip_surface;
  lock_surface.lock_result=-1; runtime.descriptor=&before_lock; runtime.queue_record_kind=1;
  // A Lock double changes kind while in the kind2 dispatch below.
  runtime.queue_record_kind=2;
  runtime.sub_5cb6c0(); assert(unlock_surface.unlock_calls==6);
  assert(flip_surface.flip_calls==1);
  // The original initial surface-null guard skips Lock, but still reaches tail.
  before_lock.surface_1c=0; runtime.descriptor=&before_lock; runtime.queue_record_kind=2;
  int prior_locks=lock_surface.lock_calls;
  runtime.sub_5cb6c0(); assert(lock_surface.lock_calls==prior_locks);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("none", "width_stride", "wrong_pitch_rounding", "stale_lock", "stale_width", "stale_unlock", "early_failure_return"))
def test_actual_locked_copy(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    body = extract((ROOT / "src/movie_media.cpp").read_text(), SIGNATURE)
    mutations = {
        "width_stride": ("destination_row += destination_stride;", "destination_row += group_count * 4;"),
        "wrong_pitch_rounding": ("(locked.lPitch / 4) * 4", "locked.lPitch"),
        "stale_lock": ("descriptor->surface_1c->Lock", "d.surface_1c->Lock"),
        "stale_width": ("(descriptor->output_width_64 & 0x7fffffff)", "(d.output_width_64 & 0x7fffffff)"),
        "stale_unlock": ("descriptor->surface_1c->Unlock", "d.surface_1c->Unlock"),
        "early_failure_return": (
            "if (descriptor->surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) == DD_OK) {",
            "if (descriptor->surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) return; {"),
    }
    if damage != "none":
        old, new = mutations[damage]
        assert body.count(old)==1
        body = body.replace(old,new,1)
    harness = projection().replace("ACTUAL_BODY",body)
    # Callback changes kind on failed Lock, enabling the original shared tail.
    harness = harness.replace("return lock_result;", "if(lock_result!=0) active->queue_record_kind=1; return lock_result;")
    source=tmp_path/"locked.cpp"; source.write_text("#include <initializer_list>\n"+harness)
    executable=tmp_path/"locked"
    built=subprocess.run([compiler,"-std=c++11",str(source),"-o",str(executable)],capture_output=True,text=True,timeout=30)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(executable)],capture_output=True,timeout=10)
    assert (result.returncode==0)==(damage=="none")
