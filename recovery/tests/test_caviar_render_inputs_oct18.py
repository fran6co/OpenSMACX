"""Complete Caviar::render source with typed host boundaries and pin anchors.

These C++98 controls observe renderer inputs, calls and state transitions.
Service doubles do not establish native ABI/layout, real voxel kernels, x87
precision/status, invalid-pointer safety, byte equality or game acceptance.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual():
    source=(ROOT/"src/caviar.cpp").read_text()
    start=source.index("int Caviar::render(Buffer *buffer, int centre_x, int centre_y, int flags)")
    return source[start:source.index("\n}",start)+2]


HARNESS=r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#define __cdecl
struct RECT {int left,top,right,bottom;};
struct VOX_Vect {float values[3];};
struct VOX_Matrix {float values[3][3];};
struct VoxSceneState {
 int instance_count_,draw_flags_;VOX_Matrix scene_rotation_;VOX_Vect scene_offset_;
};
struct CaviarSceneContext {VoxSceneState state_;unsigned char shadow_enabled_;};
struct VoxCameraBlock {
 struct Camera {float scaling;} camera_;
 struct Bounds {int values_[6];} bounds_;
};
struct Clip {int left_,top_,right_,bottom_;};
struct VoxRenderRecord {
 Clip primary_clip_,secondary_clip_;int draw_left_,draw_top_,draw_right_,draw_bottom_,draw_width_,draw_height_;
 void *colour_table_,*shadow_table_;
};
struct Caviar;
struct Buffer {
 RECT rect1_;int identity;
 void free_data(int);
 int draw_dest_source(Buffer*,int,int,int,int,int,unsigned char*);
 int draw(Buffer*,int,int,int,int,int);
};
struct BufferOwner {Buffer value;Buffer&buffer(){return value;}} CaviarSceneBufferA,CaviarSceneBufferB;
struct Scene {VoxRenderRecord *created_;Caviar *renderer_;RECT scene_rect_;} CaviarScene;
RECT CaviarViewportState;
struct Caviar {
 int field_4_,field_13CC_;CaviarSceneContext scene_context_;VoxCameraBlock camera_view_;Buffer *buffer_;
 int render(Buffer*,int,int,int);void clear_buffers(int);
};
std::string events;
Caviar *active;
Buffer target;
VoxRenderRecord record;
VOX_Matrix initial_rotation;
unsigned char colour[0x1800],shadow[256];
int flags_seen,cx,cy,transform_calls,hide_shadow_after_union;
RECT screen_clip,expected_draw;
void require(bool b,const char*w){if(!b){fprintf(stderr,"WITNESS: %s\n",w);exit(23);}}
bool equal_rect(const RECT&a,const RECT&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
bool is_identity(const VOX_Matrix&m){for(int r=0;r<3;++r)for(int c=0;c<3;++c)if(m.values[r][c]!=(r==c?1.0f:0.0f))return false;return true;}
int IntersectRect(RECT*out,const RECT*a,const RECT*b){
 events+='I';require(b==&target.rect1_,"destination clip receiver");
 RECT r={std::max(a->left,b->left),std::max(a->top,b->top),std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
 bool valid=r.left<r.right&&r.top<r.bottom;if(!valid)r.left=r.top=r.right=r.bottom=0;*out=r;return valid;
}
int UnionRect(RECT*out,const RECT*a,const RECT*b){
 require(CaviarScene.renderer_==0,"renderer cleared before output unions");
 RECT r={std::min(a->left,b->left),std::min(a->top,b->top),std::max(a->right,b->right),std::max(a->bottom,b->bottom)};
 if(out==&CaviarViewportState){events+='P';RECT covered={3,5,8,12};require(equal_rect(*a,covered),"primary inclusive output clip");}
 else {events+='S';require(out==&CaviarScene.scene_rect_,"secondary union owner");RECT covered={13,17,20,24};require(equal_rect(*a,covered),"secondary inclusive output clip");}
 *out=r;
 if(out==&CaviarScene.scene_rect_&&hide_shadow_after_union)active->scene_context_.shadow_enabled_=0;
 return 1;
}
void vox_transform_camera(VoxSceneState*state,int step,VOX_Vect*position,VOX_Vect*extent){
 events+='T';++transform_calls;require(state==&active->scene_context_.state_,"transform state receiver");
 require(step==0,"centering uses animation step zero");
 require((flags_seen&0x10)?is_identity(state->scene_rotation_):memcmp(&state->scene_rotation_,&initial_rotation,sizeof(initial_rotation))==0,"temporary identity only for bit10");
 position->values[0]=2;position->values[1]=4;position->values[2]=6;
 extent->values[0]=10;extent->values[1]=20;extent->values[2]=30;
}
void vox_draw_scene(VoxRenderRecord*r,CaviarSceneContext*context,VoxCameraBlock*camera,int step){
 events+='V';require(r==&record&&r==CaviarScene.created_,"created scene record receiver");
 require(context==&active->scene_context_&&camera==&active->camera_view_&&step==0,"scene typed arguments");
 require(CaviarScene.renderer_==active&&active->buffer_==&target,"published callback renderer and target");
 require(context->state_.instance_count_==active->field_13CC_&&context->state_.draw_flags_==1,"instance count and draw flags publication");
 require(context->shadow_enabled_==((flags_seen&2)?0:1),"shadow enable flags2");
 const int bounds[]={0,0,255,255,256,256};
 for(int i=0;i<6;++i)require(camera->bounds_.values_[i]==bounds[i],"camera bounds publication");
 require(record.draw_left_==expected_draw.left&&record.draw_top_==expected_draw.top&&record.draw_right_==expected_draw.right&&record.draw_bottom_==expected_draw.bottom,"scene-local clip coordinates");
 require(record.draw_width_==expected_draw.right-expected_draw.left&&record.draw_height_==expected_draw.bottom-expected_draw.top,"scene-local clip extents");
 require(memcmp(&context->state_.scene_rotation_,&initial_rotation,sizeof(initial_rotation))==0,"saved rotation restored before scene draw");
 const float offset[]={30,50,70},midpoint[]={6,12,18};
 for(int i=0;i<3;++i)require(context->state_.scene_offset_.values[i]==offset[i]-((flags_seen&4)?midpoint[i]:0),"centering midpoint subtraction");
 record.primary_clip_.left_=3;record.primary_clip_.top_=5;record.primary_clip_.right_=7;record.primary_clip_.bottom_=11;
 record.secondary_clip_.left_=13;record.secondary_clip_.top_=17;record.secondary_clip_.right_=19;record.secondary_clip_.bottom_=23;
}
void check_blit(Buffer*source,Buffer*dest,int key,int x,int y,int numerator,int denominator,bool is_shadow){
 require(source==(is_shadow?&CaviarSceneBufferB.value:&CaviarSceneBufferA.value)&&dest==&target,"blit source and destination owners");
 require(key==(is_shadow?0:9)&&numerator==1,"blit transparency and numerator");
 bool half=(active->field_4_&0x20)!=0;
 require(denominator==(half?2:1)&&x==cx-(half?64:128)&&y==cy-(half?64:128),"blit origin and scale");
}
int Buffer::draw_dest_source(Buffer*dest,int key,int x,int y,int numerator,int denominator,unsigned char*table){
 events+='H';check_blit(this,dest,key,x,y,numerator,denominator,true);
 require(table==shadow&&table!=colour,"shadow table provider is record54");return 17;
}
int Buffer::draw(Buffer*dest,int key,int x,int y,int numerator,int denominator){events+='C';check_blit(this,dest,key,x,y,numerator,denominator,false);return 18;}
void Buffer::free_data(int count){require(this==&target&&count==1,"original target release receiver and count");events+='F';}
void Caviar::clear_buffers(int flag){require(this==active&&flag==(flags_seen&1),"clear flags low bit");events+='K';}
@BODY@
void reset(Caviar&c,int flags,int renderer_flags,bool empty){
 memset(&c,0,sizeof(c));memset(&record,0,sizeof(record));events.clear();transform_calls=hide_shadow_after_union=0;
 active=&c;flags_seen=flags;cx=178;cy=188;c.field_4_=renderer_flags;c.field_13CC_=2;c.camera_view_.camera_.scaling=1.25f;
 for(int r=0;r<3;++r)for(int col=0;col<3;++col)initial_rotation.values[r][col]=(float)(2+r*3+col);
 c.scene_context_.state_.scene_rotation_=initial_rotation;
 c.scene_context_.state_.scene_offset_.values[0]=30;c.scene_context_.state_.scene_offset_.values[1]=50;c.scene_context_.state_.scene_offset_.values[2]=70;
 target.rect1_.left=70;target.rect1_.top=80;target.rect1_.right=250;target.rect1_.bottom=240;
 if(empty){target.rect1_.left=500;target.rect1_.top=500;target.rect1_.right=600;target.rect1_.bottom=600;}
 expected_draw.left=empty?128-cx:20;expected_draw.top=empty?128-cy:20;
 expected_draw.right=empty?128-cx:200;expected_draw.bottom=empty?128-cy:180;
 CaviarScene.created_=&record;CaviarScene.renderer_=0;record.colour_table_=colour;record.shadow_table_=shadow;
 CaviarViewportState.left=1;CaviarViewportState.top=2;CaviarViewportState.right=4;CaviarViewportState.bottom=6;
 CaviarScene.scene_rect_.left=8;CaviarScene.scene_rect_.top=9;CaviarScene.scene_rect_.right=12;CaviarScene.scene_rect_.bottom=15;
}
int main(){
 const int flags[]={0,1,2,3,4,5,6,8,9,0x10,0x14,0x16,0x1c,0x1e};
 for(unsigned f=0;f<sizeof(flags)/sizeof(flags[0]);++f)for(int half=0;half<2;++half)for(int empty=0;empty<2;++empty){
  Caviar c;reset(c,flags[f],1|(half?0x20:0),empty!=0);
  require(c.render(&target,cx,cy,flags[f])==0,"render successful return ignores blit results");
  std::string wanted="I";if(flags[f]&4)wanted+='T';wanted+="VP";if(!(flags[f]&2))wanted+="SH";wanted+='F';if(!(flags[f]&8))wanted+="CFK";
  require(events==wanted,"render callback and release order");
  require(transform_calls==((flags[f]&4)?1:0),"centering gated by bit4");
  require(c.camera_view_.camera_.scaling==(half?2.5f:1.25f),"camera scaling doubled without restore");
  RECT viewport={1,2,8,12};require(equal_rect(CaviarViewportState,viewport),"primary coverage union");
  if(!(flags[f]&2)){RECT bounds={8,9,20,24};require(equal_rect(CaviarScene.scene_rect_,bounds),"secondary coverage union");}
 }
 Caviar c;reset(c,1,0,false);require(c.render(&target,cx,cy,1)==7&&events.empty(),"inactive guard");
 reset(c,1,1,false);require(c.render(0,cx,cy,1)==7&&events.empty(),"null target guard");
 reset(c,1,0x21,false);c.field_13CC_=0;require(c.render(&target,cx,cy,1)==0&&events.empty()&&c.camera_view_.camera_.scaling==1.25f,"zero count precedes scaling");
 reset(c,1,1,false);c.field_13CC_=-1;require(c.render(&target,cx,cy,1)==0&&events=="IVPSHFCFK","negative nonzero count not normalized");
 reset(c,1,1,false);hide_shadow_after_union=1;require(c.render(&target,cx,cy,1)==0&&events=="IVPSFCFK","shadow byte rechecked after secondary union");
 puts("Caviar actual render input controls passed");
}
'''


def run(tmp_path,body):
    compiler=shutil.which("g++")
    if compiler is None:pytest.skip("host GCC unavailable")
    cpp,binary=tmp_path/"caviar_render.cpp",tmp_path/"caviar_render"
    cpp.write_text(HARNESS.replace("@BODY@",body))
    compiled=subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror","-O0",
                             str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)


def test_complete_actual_caviar_render_inputs(tmp_path):
    result=run(tmp_path,actual())
    assert result.returncode==0,result.stdout+result.stderr
    assert "Caviar actual render input controls passed" in result.stdout


@pytest.mark.parametrize("damage,witness",[
    ("colour_for_shadow","shadow table provider is record54"),
    ("flags_for_step","centering uses animation step zero"),
    ("missing_identity","temporary identity only for bit10"),
    ("identity_wrong_gate","temporary identity only for bit10"),
    ("missing_restore","saved rotation restored before scene draw"),
    ("wrong_half_origin","blit origin and scale"),
    ("flags8_reversed","render callback and release order"),
])
def test_compiling_render_input_damage_is_rejected(tmp_path,damage,witness):
    body=actual()
    if damage=="colour_for_shadow":
        good,bad="record->shadow_table_","record->colour_table_"
    elif damage=="flags_for_step":
        good="vox_transform_camera(&scene_context_.state_, 0,"
        bad="vox_transform_camera(&scene_context_.state_, (flags & 0x10) ? flags : 0,"
    elif damage=="missing_identity":
        pattern=r"        if \(flags & 0x10\) \{\n(?:            scene_context_\.state_\.scene_rotation_\.values\[\d\]\[\d\] = [01]\.0f;\n){9}        \}\n"
        body,count=re.subn(pattern,"",body)
        assert count==1
        good=bad=None
    elif damage=="identity_wrong_gate":
        good,bad="if (flags & 0x10)","if ((flags & 0x10) == 0)"
    elif damage=="missing_restore":
        good,bad="scene_context_.state_.scene_rotation_ = saved;","(void)saved;"
    elif damage=="wrong_half_origin":
        good,bad="origin_x = centre_x - 0x40;","origin_x = centre_x - 0x80;"
        assert body.count(good)==2
        body=body.replace(good,bad)
        good=bad=None
    else:
        good,bad="if ((flags & 8) == 0)","if ((flags & 8) != 0)"
    if good is not None:
        assert body.count(good)==1,damage
        body=body.replace(good,bad)
    result=run(tmp_path,body)
    assert result.returncode==23 and f"WITNESS: {witness}" in result.stderr,result.stdout+result.stderr


def test_pinned_complete_render_span_and_input_anchors():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    configured=os.environ.get("OPENSMACX_IMAGE")
    candidates=([Path(configured)] if configured else [])+[
        parent/".opensmacx/game/terranx_original.exe" for parent in (ROOT,*ROOT.parents)]
    path=next((p for p in candidates if p.is_file()),None)
    if path is None:pytest.skip("pinned proprietary image unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions={}
    for start,end in ((0x617230,0x617620),(0x618430,0x618468),(0x618DA0,0x618E06)):
        cursor=start
        for ins in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start):
            assert ins.address==cursor;cursor+=ins.size;instructions[ins.address]=(ins.mnemonic,ins.op_str)
        assert cursor==end
    anchors={
        0x617266:("mov","eax, dword ptr [ebp + 0x13cc]"),
        0x617292:("mov","dword ptr [ebp + 0x1c], eax"),
        0x6173FB:("lea","edx, [ebp + 0x38]"),
        0x617409:("test","al, 0x10"),
        0x61740B:("rep movsd","dword ptr es:[edi], dword ptr [esi]"),
        0x61740F:("mov","ecx, 0x3f800000"),
        0x617414:("xor","eax, eax"),
        0x617416:("mov","dword ptr [edx], ecx"),
        0x61742D:("mov","dword ptr [ebp + 0x58], ecx"),
        0x617432:("xor","eax, eax"),
        0x61743E:("push","eax"),
        0x617443:("call","0x63a6f0"),
        0x617465:("rep movsd","dword ptr es:[edi], dword ptr [esi]"),
        0x6174CE:("call","0x639e60"),
        0x61757E:("mov","edx, dword ptr [ecx + 0x54]"),
        0x61759A:("mov","eax, dword ptr [edx + 0x54]"),
        0x6175AC:("call","0x5d8ee0"),
        0x6175B5:("call","0x5e34a3"),
        0x6175F4:("call","0x5d84c0"),
        0x6175FD:("call","0x5e34a3"),
        0x61760F:("call","0x618470"),
        0x618441:("mov","eax, dword ptr [eax + 0x54]"),
        0x618454:("push","0x100"),
        0x618DBB:("mov","ecx, dword ptr [eax + 0x50]"),
        0x618DF9:("mov","edx, dword ptr [ecx + 0x50]"),
    }
    for address,expected in anchors.items():assert instructions[address]==expected,hex(address)
