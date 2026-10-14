"""Pinned output-path controls; host doubles do not prove COM or full codec ABI."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

from test_movie_media_behavior import definition

ROOT = Path(__file__).resolve().parents[2]
BASE = "74795287356356950f454c02e645861d0946d245"
SIGNATURE = 'extern "C" void __fastcall sub_5cc940('


def without_renderer_body(text):
    from test_core_claim_continuity_oct14dt import historical_core_claim_view
    text = historical_core_claim_view(text, 'src/movie_media.cpp')
    from test_media_motion_storage_oct11 import restore_motion_storage
    text=restore_motion_storage(text,'src/movie_media.cpp')
    # Independently verified follow-up correction has its own bounded-source
    # and pinned rounding controls in test_media_component_rounding_oct11.
    text=text.replace('''  const __int64 product =
      (__int64)(int)movie_media_conversion_tables.block_scale_entries[0] * (int)component;
  // Native SHL/SHR/ADC adds product bit 15 before the caller's SAR 1.
  return (int)(product >> 16) + (int)((product >> 15) & 1);''', '''  return (int)(((__int64)movie_media_conversion_tables.block_scale_entries[0] *
                (int)component) >> 16);''')
    replacements=[]
    for signature in ('extern "C" int __fastcall sub_5cc940(', SIGNATURE,
            'void MediaRuntime::sub_5cbbc0()',
            'extern "C" void __fastcall sub_5cd8d0(',
            'extern "C" void __fastcall sub_5cd980(',
            'extern "C" void __fastcall sub_5cda30('):
        if signature in text:
            replacements.append((definition(text,signature),signature.replace('int __fastcall sub_5cc940','void __fastcall sub_5cc940')+' REVIEWED_BODY'))
    for before,after in replacements:
        text=text.replace(before,after)
    for name in ('sub_5cd8d0','sub_5cd980','sub_5cda30'):
        prefix='extern "C" void __fastcall '+name+'(MediaRuntime *, unsigned char *,\n'
        new=prefix+'                                      unsigned char *, unsigned char *, unsigned int);'
        old=prefix+'                                      unsigned char *, int, unsigned int);'
        if prefix in text:
            assert text.count(new)+text.count(old)==1
        text=text.replace(new,old)
    return text.replace(
        '11, then swaps the alternating output-buffer pointers at +0x4c/+0x50.',
        '11, then swaps the pointer-shaped +0x4c and DWORD +0x50 values.')


def test_projection_cannot_hide_unrelated_prototype_changes():
    declaration='extern "C" void unrelated(unsigned char *, unsigned char *, unsigned int);'
    assert without_renderer_body(declaration)==declaration


HEADER_BEFORE = '  DWORD provided_output_pitch_50;                 // +50; byte pitch in same path'
HEADER_AFTER = '''  // 5CBBC0 swaps this address into +4c before the next record conversion.
  // The provided-buffer stride is derived from width/pixel mode, not +50.
  BYTE *provided_output_alternate_50;            // +50; alternating output buffer'''


def restore_output_header(text):
    from test_media_motion_storage_oct11 import restore_motion_storage
    text=restore_motion_storage(text,'src/movie_media.h')
    return text.replace(HEADER_AFTER,HEADER_BEFORE).replace(
        'extern "C" void __fastcall sub_5cc940(', 'extern "C" int __fastcall sub_5cc940(')


def test_bounded_output_owner_change_and_identity_continuity():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    for name in ('movie_media.cpp','movie_media.h'):
        path=ROOT/'src'/name
        old=subprocess.check_output(['git','show',BASE+':src/'+name],cwd=ROOT,text=True)
        new=path.read_text()
        if name.endswith('.cpp'):
            from test_core_claim_continuity_oct14dt import historical_core_claim_view
            new = historical_core_claim_view(new, 'src/movie_media.cpp')
        if name.endswith('.cpp'):
            assert without_renderer_body(old)==without_renderer_body(new)
        else:
            assert HEADER_AFTER in new and restore_output_header(new)==old
        assert [(_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(old,path)]==[
            (_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(new,path)]


def test_pinned_output_bypass_pitch_dispatch_and_unlock():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / '.opensmacx/game/terranx_original.exe'
    if not path.is_file():
        pytest.skip('pinned executable unavailable')
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    rows = {i.address: (i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x5CC940, 0x5CD28C)), 0x5CC940)}
    for address, expected in {
        0x5CCA84: ('test', 'ch, 8'),
        0x5CCA87: ('jne', '0x5ccdde'),
        0x5CCDDE: ('mov', 'edx, dword ptr [eax + 0x4c]'),
        0x5CCDE1: ('mov', 'ebx, dword ptr [eax + 0x64]'),
        0x5CCDE7: ('mov', 'edx, dword ptr [eax + 0x50]'),
        0x5CCDEA: ('shl', 'ebx, 1'),
        0x5CCDF4: ('shl', 'ebx, 1'),
        0x5CCEEF: ('add', 'ebx, ebx'),
        0x5CD001: ('add', 'ebx, ebx'),
        0x5CCF34: ('call', '0x5cfaed'),
        0x5CD046: ('call', '0x5d02cc'),
        0x5CD1FE: ('jne', '0x5cd285'),
        0x5CD217: ('call', 'dword ptr [edx + 0x80]'),
        0x5CD240: ('call', 'dword ptr [ecx + 0x80]'),
        0x5CD25C: ('call', 'dword ptr [edx + 0x80]'),
        0x5CD278: ('call', 'dword ptr [ecx + 0x80]'),
    }.items():
        assert rows[address] == expected
    # Every indirect call after conversion is an Unlock, never IsLost.
    assert [operand for address, (op, operand) in rows.items()
            if address >= 0x5CD1F6 and op == 'call'] == [
                'dword ptr [edx + 0x80]', 'dword ptr [ecx + 0x80]',
                'dword ptr [edx + 0x80]', 'dword ptr [ecx + 0x80]']
    lifecycle={i.address:(i.mnemonic,i.op_str)
        for start,end in ((0x5CBBC0,0x5CBBF5),(0x5CB220,0x5CB60B),(0x5CAAB0,0x5CADF9))
        for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start)}
    for address,expected in {
        0x5CBBE5: ('mov','esi, dword ptr [eax + 0x4c]'),
        0x5CBBE8: ('mov','edx, dword ptr [eax + 0x50]'),
        0x5CBBEB: ('mov','dword ptr [eax + 0x50], esi'),
        0x5CBBEE: ('mov','ecx, dword ptr [ecx]'),
        0x5CBBF0: ('mov','dword ptr [ecx + 0x4c], edx'),
        0x5CB4CF: ('call','0x5cbbc0'),
        0x5CABE8: ('call','0x5cc940'),
        0x5CABED: ('mov','ecx, esi'),
        0x5CABEF: ('call','0x5cb6c0'),
        0x5CAC11: ('call','0x5cc940'),
        0x5CAC16: ('mov','eax, dword ptr [esi]'),
        0x5CB460: ('call','0x5cc940'),
        0x5CB465: ('mov','ecx, esi'),
        0x5CB467: ('call','0x5cb6c0'),
        0x5CB489: ('call','0x5cc940'),
        0x5CB48E: ('mov','eax, dword ptr [esi]'),
    }.items():
        assert lifecycle[address]==expected


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#define __fastcall
typedef unsigned char BYTE; typedef uint32_t DWORD; typedef int32_t LONG;
typedef int32_t HRESULT; typedef int16_t SHORT;
const HRESULT DD_OK=0, DDERR_SURFACELOST=-2;
const DWORD DDLOCK_WAIT=1; const int IID_IDirectDrawSurface2=0;
struct RECT { LONG left,top,right,bottom; };
struct DDBLTFX { DWORD dwSize,dwFillColor; BYTE unused[100]; };
struct DDSURFACEDESC {
  DWORD dwSize,dwFlags,dwWidth,dwHeight; LONG lPitch; void *lpSurface;
  struct { DWORD dwCaps; } ddsCaps; BYTE unused[100];
};
struct IDirectDrawSurface2 {
  int lost_calls=0,restore_calls=0,lock_calls=0,unlock_calls=0;
  HRESULT unlock_result=0; BYTE *data=0; LONG pitch=88;
  HRESULT IsLost() { ++lost_calls; return DD_OK; }
  HRESULT Restore() { ++restore_calls; return DD_OK; }
  HRESULT QueryInterface(int,void**) { assert(false); return -1; }
  HRESULT Blt(RECT*,int,int,DWORD,DDBLTFX*) { assert(false); return -1; }
  HRESULT Lock(void*,DDSURFACEDESC *d,DWORD,void*) {
    ++lock_calls; d->lpSurface=data; d->lPitch=pitch; return DD_OK;
  }
  HRESULT Unlock(void*) { ++unlock_calls; return unlock_result; }
};
typedef IDirectDrawSurface2 IDirectDrawSurface;
struct IDirectDraw2 {
  HRESULT CreateSurface(DDSURFACEDESC*,IDirectDrawSurface**,void*) { assert(false); return -1; }
  HRESULT GetFourCCCodes(DWORD*,DWORD*) { assert(false); return -1; }
};
struct MediaDescriptor {
  DWORD flags=0; IDirectDraw2 *draw_device_10=0;
  IDirectDrawSurface2 *surface_14=0,*surface_18=0,*surface_1c=0,*surface_20=0;
  struct { DWORD total_component_bits=8; } pixel_format;
  BYTE *provided_output_base_4c=0,*provided_output_alternate_50=0;
  DWORD output_width_64=16,output_height_60=16;
  LONG geometry_offset_x_68=0,geometry_offset_y_6c=0;
  DWORD geometry_width_override_70=0,geometry_height_override_74=0;
};
struct MediaRuntime {
  void sub_5cbbc0();
  MediaDescriptor *descriptor; DWORD selected_record_kind_39c=0,queue_record_kind=0;
  DWORD record_scale_byte_048=0,record_option_040=0,record_option_044=0;
  DWORD output_rect_left_14=0,output_rect_top_18=0,output_rect_extent_0c=0,output_rect_height_10=0;
  DWORD surface_width_004=64,surface_height_008=64,output_handle_candidate=0;
  DWORD surface_loss_counter_02c=0,surface_loss_counter_030=0,surface_loss_counter_038=0;
  BYTE *owned_heap_638=0;
  struct { DWORD block_transform_weights[64]; } decoder_context_3f8;
};
struct MediaQueueRecord { BYTE *payload; DWORD field_0c; };
static SHORT media_read_signed_word(const BYTE *p) { SHORT v; memcpy(&v,p,2); return v; }
static void sub_5cd750(MediaRuntime*,BYTE*) {}
static int sub_5cdad0(IDirectDraw2*,IDirectDrawSurface2**,int,int,int) { assert(false); return 0; }
static int sub_5cc860(IDirectDraw2*,IDirectDrawSurface2**,int,int,int) { assert(false); return 0; }
static void sub_5d3df0(MediaRuntime*,BYTE*) {}
static void sub_5d3f50(MediaRuntime*) {}
static void sub_5d3980(MediaRuntime*,BYTE*) {}
static BYTE *seen_destination,*seen_unused; static unsigned seen_pitch,conversions;
static void sub_5d4020(MediaRuntime*,BYTE *d,BYTE *u,int p) {
  seen_destination=d; seen_unused=u; seen_pitch=p; ++conversions;
}
static void media_build_runtime_block_scale_table(int) { assert(false); }
static void sub_5cfaed(BYTE*,BYTE*,int) { assert(false); }
static void sub_5d02cc(BYTE*,void*,int) { assert(false); }
static void sub_5cd3b0(char,char,char,char,char,char,int*,unsigned) { assert(false); }
static void sub_5cd560(signed char,signed char,signed char,signed char,signed char,signed char,int*,unsigned) { assert(false); }
static void sub_5cd2d0(DWORD*,int) {}
static void sub_5cd8d0(MediaRuntime *r,BYTE*,BYTE *d,BYTE *u,unsigned p) { sub_5d4020(r,d,u,p); }
static void sub_5cd980(MediaRuntime *r,BYTE*,BYTE *d,BYTE *u,unsigned p) { sub_5d4020(r,d,u,p); }
static void sub_5cda30(MediaRuntime *r,BYTE*,BYTE *d,BYTE *u,unsigned p) { sub_5d4020(r,d,u,p); }
static void sub_5d35b0(MediaRuntime*,const BYTE*,BYTE*,DWORD,int) { assert(false); }
ACTUAL_BODY
ACTUAL_SWAP
int main() {
  BYTE packet[32]={},provided[8192]={},heap[8192]={},locked[8192]={};
  packet[0]=packet[2]=16;
  MediaQueueRecord record={packet,1};
  for(DWORD record_kind : {DWORD(1),DWORD(2),DWORD(4)}) {
  record.field_0c=record_kind;
  for(DWORD flags : {DWORD(0x800),DWORD(0x840),DWORD(0xC00),DWORD(0xC40)})
    for(DWORD kind : {DWORD(1),DWORD(2),DWORD(4)}) {
      // No surface/device is needed on this path; touching one would crash.
      MediaDescriptor d; d.flags=flags; d.provided_output_base_4c=provided;
      d.provided_output_alternate_50=heap;
      MediaRuntime r; r.descriptor=&d; r.queue_record_kind=kind; r.owned_heap_638=heap;
      conversions=0; sub_5cc940(&r,&record);
      assert(conversions==1 && seen_destination==provided);
      assert(seen_unused==heap);
      assert(seen_pitch==((flags&0x40)?64:32));
      assert(r.queue_record_kind==kind && r.output_handle_candidate==0);
      r.sub_5cbbc0();
      assert(d.provided_output_base_4c==heap && d.provided_output_alternate_50==provided);
      conversions=0; sub_5cc940(&r,&record);
      assert(conversions==1 && seen_destination==heap && seen_unused==provided);
      assert(seen_pitch==((flags&0x40)?64:32));
    }
  }
  record.field_0c=1;
  for(DWORD kind : {DWORD(1),DWORD(4)})
    for(DWORD flags : {DWORD(0),DWORD(0x400),DWORD(0x8000400)})
      for(HRESULT status : {HRESULT(0),HRESULT(-7)}) {
        IDirectDrawSurface2 primary,front,back;
        primary.data=front.data=back.data=locked;
        primary.unlock_result=front.unlock_result=back.unlock_result=status;
        MediaDescriptor d; d.flags=flags;
        d.surface_1c=&primary; d.surface_14=&front; d.surface_18=&back;
        MediaRuntime r; r.descriptor=&d; r.queue_record_kind=kind;
        conversions=0; sub_5cc940(&r,&record);
        auto *target=kind==4 || !(flags&0x400)?&primary:flags&0x8000000?&back:&front;
        assert(conversions==1 && seen_destination==locked && seen_pitch==88);
        assert(seen_unused==locked);
        assert(target->lock_calls==1 && target->unlock_calls==1);
        // Only the pre-conversion cooperative scan queries surface loss.
        assert(primary.lost_calls==1 && front.lost_calls==0);
        assert(back.lost_calls==((flags&0x8000000)?1:0));
        assert(r.surface_loss_counter_038==((status && target==&primary)?1:0));
        assert(r.surface_loss_counter_02c==((status && target==&front)?1:0));
        assert(r.surface_loss_counter_030==((status && target==&back)?1:0));
      }
  // Distinct slots may name one surface; native counter choice is by flags.
  for(DWORD flags : {DWORD(0),DWORD(0x400),DWORD(0x8000400)}) {
    IDirectDrawSurface2 shared;
    shared.data=locked; shared.unlock_result=-7;
    MediaDescriptor d; d.flags=flags;
    d.surface_1c=d.surface_14=d.surface_18=&shared;
    MediaRuntime r; r.descriptor=&d; r.queue_record_kind=1;
    sub_5cc940(&r,&record);
    assert(shared.lock_calls==1 && shared.unlock_calls==1);
    assert(r.surface_loss_counter_038==(!(flags&0x400)?1:0));
    assert(r.surface_loss_counter_02c==((flags&0x400)&&!(flags&0x8000000)?1:0));
    assert(r.surface_loss_counter_030==((flags&0x400)&&(flags&0x8000000)?1:0));
  }
}
'''


@pytest.mark.parametrize('compiler_name', ('g++', 'clang++'))
@pytest.mark.parametrize('damage', ('none','wrong_pitch','wrong_unused','missing_bypass',
    'extra_loss_query','wrong_counter','missing_swap'))
def test_actual_complete_output_path(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    body = definition((ROOT/'src/movie_media.cpp').read_text(), SIGNATURE)
    source = tmp_path/'outputs.cpp'
    swap=definition((ROOT/'src/movie_media.cpp').read_text(),'void MediaRuntime::sub_5cbbc0()')
    projection=HARNESS.replace('ACTUAL_BODY',body).replace('ACTUAL_SWAP',swap)
    changes={
        'wrong_pitch': ('output_pitch = d.output_width_64 << 1;', 'output_pitch = 123;'),
        'wrong_unused': ('unused_output = d.provided_output_alternate_50;', 'unused_output = d.provided_output_base_4c;'),
        'missing_bypass': ('if ((d.flags & 0x800) == 0) {','if (true) {'),
        'extra_loss_query': ('if (unlock_target->Unlock(0) != DD_OK)', '(void)unlock_target->IsLost();\n      if (unlock_target->Unlock(0) != DD_OK)'),
        'wrong_counter': ('if ((unlock_flags & 0x400) == 0) ++runtime->surface_loss_counter_038;', 'if ((unlock_flags & 0x400) == 0) ++runtime->surface_loss_counter_02c;'),
        'missing_swap': ('view->provided_output_base_4c = previous_alternate;', '(void)previous_alternate;'),
    }
    if damage!='none':
        before,after=changes[damage]
        assert before in projection
        projection=projection.replace(before,after,1)
    source.write_text(projection)
    binary = tmp_path/'outputs'
    result = subprocess.run([compiler,'-std=c++17','-fsanitize=undefined',
        '-fno-sanitize-recover=all',str(source),'-o',str(binary)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    result=subprocess.run([str(binary)],capture_output=True,timeout=10,preexec_fn=no_core)
    assert (result.returncode==0)==(damage=='none'),result.stderr.decode()
