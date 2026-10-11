"""Actual presentation destination/loss/row controls, not COM ABI proof."""
import resource
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract
from test_media_present_descriptor_oct11 import HARNESS, ROOT, SIGNATURE


def no_core():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def test_pinned_retained_destination_receiver_and_row_reload():
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
        0x5CB8CE:("mov","edi, dword ptr [eax + 0x18]"),
        0x5CB8D3:("mov","edi, dword ptr [eax + 0x14]"),
        0x5CB8E1:("mov","edx, dword ptr [esi]"),
        0x5CB926:("dec","ecx"),
        0x5CB934:("dec","edx"),
        0x5CB953:("mov","ecx, dword ptr [edi]"),
        0x5CB955:("call","dword ptr [ecx + 0x60]"),
        0x5CB95F:("mov","edx, dword ptr [edi]"),
        0x5CB962:("call","dword ptr [edx + 0x6c]"),
        0x5CB96D:("mov","eax, dword ptr [esi]"),
        0x5CB96F:("mov","ecx, dword ptr [eax + 4]"),
        0x5CB9B6:("mov","eax, dword ptr [esi]"),
        0x5CBA0A:("mov","eax, dword ptr [esi + 0x18]"),
        0x5CBA20:("mov","ecx, dword ptr [esi]"),
        0x5CBA3A:("mov","eax, dword ptr [esi]"),
    }.items():
        assert instructions[address]==instruction


def projection():
    text=HARNESS[:HARNESS.index("int main()")]
    text=text.replace("struct Surface : IDirectDrawSurface2 {", "struct Surface : IDirectDrawSurface2 {\n  std::vector<RECT> destinations,sources;\n  std::vector<IDirectDrawSurface2*> receivers;\n  bool mutate_rectangles=false;")
    text=text.replace("assert(flags==DDBLT_ASYNC && !fx); ++blt_calls; destination=*d; source=*r; blt_source=s;",
        "assert(flags==DDBLT_ASYNC && !fx); ++blt_calls; destination=*d; source=*r; blt_source=s;\n"
        "destinations.push_back(*d); sources.push_back(*r); receivers.push_back(s);\n"
        "if(mutate_rectangles && blt_calls==1) { d->left=27; r->left=7; active->output_rect_top_18+=100; }")
    return "#include <vector>\n"+text+r'''
int main(int argc,char **argv) {
  Surface target,back_target,wrong_target,source_initial,source_after,source_final;
  BYTE entries[1024]; std::memset(entries,0xA5,sizeof(entries));
  MediaDescriptor initial={},after_lost={},after_restore={},after_blt={};
  initial.surface_14=&target; initial.surface_18=&back_target; initial.surface_1c=&source_initial;
  initial.output_width_64=64; initial.output_height_60=48;
  initial.palette_entries_28=entries; initial.pixel_format.total_component_bits=8;
  for(unsigned i=0;i<0x300;++i) initial.palette_rgb_triplets_b0[i]=static_cast<BYTE>(i*11+5);
  after_lost.surface_14=&wrong_target; after_lost.surface_18=&wrong_target;
  after_lost.surface_1c=&source_after; after_lost.output_width_64=99; after_lost.output_height_60=77;
  after_restore.surface_14=&wrong_target; after_restore.surface_18=&wrong_target;
  after_restore.surface_1c=&source_after; after_restore.output_width_64=99; after_restore.output_height_60=5;
  after_blt.surface_14=&wrong_target; after_blt.surface_18=&wrong_target;
  after_blt.surface_1c=&source_final; after_blt.output_width_64=101; after_blt.output_height_60=3;
  MediaRuntime runtime={&initial,0,1,11,13,37,29,0}; active=&runtime;
  if(argc>1) {
    if(!std::strcmp(argv[1],"null_target")) initial.surface_14=0;
    else if(!std::strcmp(argv[1],"null_palette")) initial.palette_entries_28=0;
    else assert(false);
    runtime.sub_5cb6c0(); return 0;
  }
  for(unsigned select_back=0;select_back<2;++select_back) {
    for(DWORD mode : {DWORD(0),DWORD(0x20),DWORD(0x10),DWORD(0x30)}) {
      target=Surface(); back_target=Surface();
      Surface &selected=select_back?back_target:target;
      selected.lost_result=DDERR_SURFACELOST; selected.lost_replacement=&after_lost;
      selected.restore_replacement=&after_restore; selected.blt_replacement=&after_blt;
      selected.mutate_rectangles=true;
      initial.flags=select_back?0x08000000:0;
      after_restore.flags=mode; after_blt.flags=0;
      runtime.descriptor=&initial; runtime.queue_record_kind=1; runtime.output_rect_top_18=13;
      runtime.sub_5cb6c0();
      assert(selected.lost_calls==1 && selected.restore_calls==1);
      assert(source_initial.lost_calls==0 && source_initial.restore_calls==0);
      assert(wrong_target.lost_calls==0 && wrong_target.restore_calls==0 && wrong_target.blt_calls==0);
      unsigned calls=(mode&0x20)?2:(mode&0x10)?4:1;
      assert(selected.blt_calls==static_cast<int>(calls));
      for(unsigned i=0;i<calls;++i) {
        const RECT &d=selected.destinations[i],&s=selected.sources[i];
        assert(d.left==(i?27:11) && d.right==48);
        assert(s.left==(i?7:0) && s.right==63);
        assert(selected.receivers[i]==(i?&source_final:&source_after));
        if(mode==0) { assert(d.top==13 && d.bottom==42 && s.top==0 && s.bottom==47); }
        else {
          unsigned row=(mode&0x20)?i*2:(i/2)*2;
          assert(d.top==static_cast<LONG>((i?113:13)+row+((mode&0x20)?0:i%2)));
          assert(d.bottom==d.top+1 && s.top==static_cast<LONG>(row/2) && s.bottom==s.top+1);
        }
      }
      for(unsigned i=0;i<256;++i) {
        assert(entries[i*4]==initial.palette_rgb_triplets_b0[i*3]);
        assert(entries[i*4+1]==initial.palette_rgb_triplets_b0[i*3+1]);
        assert(entries[i*4+2]==initial.palette_rgb_triplets_b0[i*3+2]);
        assert(entries[i*4+3]==0);
      }
    }
  }
  // Failed Restore exits before Blt even if replacement descriptor requests Flip.
  target=Surface(); target.lost_result=DDERR_SURFACELOST; target.restore_result=-1;
  initial.flags=0; runtime.descriptor=&initial; runtime.queue_record_kind=1;
  runtime.sub_5cb6c0(); assert(target.restore_calls==1 && target.blt_calls==0);
  // Early kind1 suppression retains the initial flag decision.
  initial.flags=0x400; runtime.descriptor=&initial;
  runtime.sub_5cb6c0(); assert(target.lost_calls==1);
  // Unsigned width/height decrement and output-coordinate wrapping.
  target=Surface(); initial.flags=0; initial.pixel_format.total_component_bits=16;
  initial.output_width_64=0x80000000; initial.output_height_60=0;
  runtime.descriptor=&initial; runtime.output_rect_top_18=0xFFFFFFF0;
  runtime.sub_5cb6c0();
  assert(target.sources[0].right==0x7FFFFFFF && target.sources[0].bottom==-1);
  assert(target.destinations[0].bottom==13);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("none", "wrong_loss_receiver", "wrong_restore_receiver", "stale_flags", "stale_source", "late_dimensions", "stale_second_row_origin"))
def test_actual_destination_and_row_callbacks(tmp_path,compiler_name,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    body=extract((ROOT/"src/movie_media.cpp").read_text(),SIGNATURE)
    prefix,rest=body.split("  } else if (queue_record_kind == 2)",1)
    mutations={
        "wrong_loss_receiver":("HRESULT status = target->IsLost();","HRESULT status = d.surface_1c->IsLost();"),
        "wrong_restore_receiver":("target->Restore() != DD_OK","d.surface_1c->Restore() != DD_OK"),
        "stale_flags":("DWORD presentation_flags = descriptor->flags;","DWORD presentation_flags = flags;"),
        "stale_source":("descriptor->surface_1c, &source","d.surface_1c, &source"),
        "late_dimensions":("source.right = (LONG)(descriptor->output_width_64 - 1);","source.right = 98;"),
        "stale_second_row_origin":("destination.top = (LONG)(output_rect_top_18 + row + 1);","destination.top += 1;"),
    }
    if damage!="none":
        old,new=mutations[damage]
        assert old in prefix
        prefix=prefix.replace(old,new)
    body=prefix+"  } else if (queue_record_kind == 2)"+rest
    source=tmp_path/"destination.cpp"
    source.write_text("#include <initializer_list>\n"+projection().replace("ACTUAL_BODY",body))
    exe=tmp_path/"destination"
    built=subprocess.run([compiler,"-std=c++11","-fsanitize=undefined","-fno-sanitize-recover=all",str(source),"-o",str(exe)],capture_output=True,text=True,timeout=30)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(exe)],capture_output=True,timeout=10,preexec_fn=no_core)
    assert (result.returncode==0)==(damage=="none"),result.stderr.decode()
    if damage=="none":
        for case in ("null_target","null_palette"):
            failure=subprocess.run([str(exe),case],capture_output=True,timeout=10,preexec_fn=no_core)
            assert failure.returncode!=0,"native unguarded path must not silently skip"


def test_kind1_and_kind4_bounded_source_migration():
    from recovery.tests.test_media_audio_owner_oct11 import without_audio_owner_functions
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path=ROOT/"src/movie_media.cpp"
    before=subprocess.check_output(["git","show","22a6607d34d6527117d0ff14452b630086ecff78:src/movie_media.cpp"],cwd=ROOT,text=True)
    after=path.read_text()
    def outside_kind1(text):
        start=text.index("  if (queue_record_kind == 1) {",text.index(SIGNATURE))
        end=text.index("  } else if (queue_record_kind == 2) {",start)
        return text[:start]+"KIND1"+text[end:]
    expected=outside_kind1(before).replace("(LONG)descriptor->output_width_64 - 1","(LONG)(descriptor->output_width_64 - 1)").replace(
        "(LONG)descriptor->output_height_60 - 1","(LONG)(descriptor->output_height_60 - 1)")
    assert without_audio_owner_functions(expected)==without_audio_owner_functions(outside_kind1(after))
    assert [(r.address,_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(before,path)]==[
        (r.address,_continuity_fields(r),r.symbol,r.function_fragment) for r in read_text(after,path)]
