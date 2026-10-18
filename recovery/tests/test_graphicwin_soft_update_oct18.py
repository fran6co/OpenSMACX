"""Whole actual soft_update overloads with independent bounded host geometry.

The typed C++98 Win/USER32 doubles distinguish Buffer bounds from Win client
bounds and expose callback ordering. They do not prove VC6 ABI, native USER32,
signed-overflow behavior, byte equality, linked preservation or game behavior.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from test_infowin_setup_text import _function_end


ROOT=Path(__file__).resolve().parents[2]
SIGNATURES=("void GraphicWin::soft_update()",
            "void GraphicWin::soft_update(RECT *area)",
            "void GraphicWin::soft_update(int x, int y, int width, int height)")


def actual():
    path=ROOT/"src/graphicwin.cpp"
    records=read_file(path)
    for start,end in ((0x5D5890,0x5D592D),(0x5D5930,0x5D5A6E),(0x5D5720,0x5D588C)):
        record,=(r for r in records if r.address==start)
        assert record.image_spans==((start,end),)
    text=path.read_text()
    bodies=[]
    for signature in SIGNATURES:
        assert text.count(signature)==1
        start=text.index(signature)
        bodies.append(text[start:_function_end(text,start)])
    return bodies


# Previous source body at coordinator2203ac498: compile this complete wrong
# provider implementation as a damage control, rather than testing a grep.
OLD_COORDINATE=r'''
void GraphicWin::soft_update(int x, int y, int width, int height) {
    if (!Win::is_visible()) return;
    RECT local = {x, y, x + width, y + height};
    if (!IntersectRect(&local, &local, &client_rect_)) return;
    int origin_x = 0;
    int origin_y = 0;
    Win::client_to_screen(&origin_x, &origin_y);
    OffsetRect(&local, origin_x, origin_y);
    InvalidateRect(HandleMain, &local, FALSE);
}
'''


HARNESS=r'''
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
struct RECT {int left,top,right,bottom;};
typedef void* HWND;
enum {FALSE=0,TRUE=1};
struct Win {RECT client_rect_;int is_visible();void client_to_screen(int*,int*);};
struct Buffer {RECT rect2_;};
struct GraphicWin:Win,Buffer {
 void soft_update();void soft_update(RECT*);void soft_update(int,int,int,int);
} window;
int original_handle,fresh_handle;
HWND HandleMain;
RECT caller,caller_before,clip_bounds,expected_invalidation;
const RECT visibility_bounds={-2,-1,12,17},conversion_bounds={91,92,93,94};
int visibility_answers[2],visibility_calls,mutate_on_visibility,mutate_on_conversion;
int invalidations;
std::string expected_events,events;
void require(bool value,const char*witness){if(!value){fprintf(stderr,"WITNESS: %s\n",witness);exit(43);}}
bool same(const RECT&a,const RECT&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
void event(char c){
 require(events.size()<expected_events.size()&&expected_events[events.size()]==c,"visibility/clip/convert/invalidate order");
 events+=c;
}
int Win::is_visible(){
 require(this==static_cast<Win*>(&window),"visibility Win receiver");event('V');
 require(visibility_calls<2,"original visibility call count");++visibility_calls;
 if(mutate_on_visibility==visibility_calls)window.rect2_=visibility_bounds;
 return visibility_answers[visibility_calls-1];
}
void Win::client_to_screen(int*x,int*y){
 require(this==static_cast<Win*>(&window),"conversion Win receiver");event('C');
 require(*x==0&&*y==0,"translate from zero client origin");
 *x=-13;*y=21;
 HandleMain=&fresh_handle;
 if(mutate_on_conversion)window.rect2_=conversion_bounds;
}
int IntersectRect(RECT*out,const RECT*a,const RECT*b){
 event('I');require(b==&window.rect2_,"Buffer extent provider instead of Win client rectangle");
 require(out==a&&out!=b&&out!=&caller,"clip local copy preserves borrowed rectangle");
 require(same(*b,clip_bounds),"Buffer bounds read after visibility callback");
 RECT r={std::max(a->left,b->left),std::max(a->top,b->top),std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
 if(r.left>=r.right||r.top>=r.bottom){out->left=out->top=out->right=out->bottom=0;return FALSE;}
 *out=r;return TRUE;
}
int OffsetRect(RECT*r,int x,int y){r->left+=x;r->right+=x;r->top+=y;r->bottom+=y;return TRUE;}
int InvalidateRect(HWND handle,const RECT*r,int erase){
 event('N');require(handle==HandleMain&&handle==&fresh_handle,"HandleMain reload after client_to_screen");
 require(erase==FALSE,"invalidate without erase");
 require(r!=&caller&&r!=&window.rect2_,"invalidate local rectangle copy");
 require(same(*r,expected_invalidation),"clipped screen invalidation coordinates");
 ++invalidations;return FALSE; // original discards the API result
}
@BODIES@

// Independent expected interval clipping and translation; source types are
// bounded integers, so none of the fixture cases invokes signed overflow.
bool oracle_intersection(RECT&r,const RECT&b){
 int left=r.left>b.left?r.left:b.left,top=r.top>b.top?r.top:b.top;
 int right=r.right<b.right?r.right:b.right,bottom=r.bottom<b.bottom?r.bottom:b.bottom;
 if(right<=left||bottom<=top)return false;
 r.left=left;r.top=top;r.right=right;r.bottom=bottom;return true;
}
// Forms0 coordinates,1 borrowed pointer,2 null pointer,3 no-args,4 Buffer alias.
void scenario(int form,int x,int y,int width,int height,int first,int second,int visibility_mutation,int conversion_mutation){
 const RECT initial_bounds={-5,3,31,29};
 window.rect2_=initial_bounds;
 window.client_rect_.left=100;window.client_rect_.top=200;window.client_rect_.right=130;window.client_rect_.bottom=220;
 caller.left=x;caller.top=y;caller.right=x+width;caller.bottom=y+height;caller_before=caller;
 visibility_answers[0]=first;visibility_answers[1]=second;visibility_calls=invalidations=0;
 mutate_on_visibility=visibility_mutation;mutate_on_conversion=conversion_mutation;
 events.clear();expected_events="V";HandleMain=&original_handle;
 RECT expected_bounds=visibility_mutation==1?visibility_bounds:initial_bounds;
 bool invalidate=false;RECT projected=caller;
 if(first){
  if(form==2){expected_events+='V';if(visibility_mutation==2)expected_bounds=visibility_bounds;}
  bool visible_again=form!=2||second;
  if(visible_again){
   if(form==2||form==3){projected=expected_bounds;invalidate=true;}
   else {
    expected_events+='I';if(form==4)projected=expected_bounds;
    invalidate=oracle_intersection(projected,expected_bounds);
   }
   if(invalidate){expected_events+="CN";projected.left-=13;projected.right-=13;projected.top+=21;projected.bottom+=21;}
  }
 }
 clip_bounds=expected_bounds;expected_invalidation=projected;
 if(form==0)window.soft_update(x,y,width,height);
 else if(form==1)window.soft_update(&caller);
 else if(form==2)window.soft_update(static_cast<RECT*>(0));
 else if(form==3)window.soft_update();
 else window.soft_update(&window.rect2_);
 require(events==expected_events,"complete original event sequence");
 require(invalidations==(invalidate?1:0),"invalidation count");
 require(same(caller,caller_before),"caller rectangle remains unchanged");
 if(conversion_mutation&&invalidate)expected_bounds=conversion_bounds;
 require(same(window.rect2_,expected_bounds),"only explicit service callbacks change Buffer bounds");
 require(window.client_rect_.left==100&&window.client_rect_.top==200&&window.client_rect_.right==130&&window.client_rect_.bottom==220,"Win client rectangle remains independent");
}
int main(){
 const int rectangles[][4]={
  {-20,-10,70,60}, {-7,1,12,12}, {0,5,5,7}, {31,5,4,4},
  {25,29,4,4}, {-9,3,4,4}, {0,-1,4,4}, {-5,3,36,26},
  {0,5,0,6},{0,5,6,0},{7,9,-2,6},{7,9,6,-2}
 };
 for(int form=0;form<5;++form)for(unsigned r=0;r<sizeof(rectangles)/sizeof(rectangles[0]);++r)
  for(int visible=0;visible<2;++visible)for(int mutation=0;mutation<3;++mutation)
   for(int convert=0;convert<2;++convert){
    if(form!=2&&mutation==2)continue;
    scenario(form,rectangles[r][0],rectangles[r][1],rectangles[r][2],rectangles[r][3],visible,1,mutation,convert);
   }
 scenario(2,0,0,4,4,1,0,0,0);
 scenario(2,0,0,4,4,1,0,2,0);
 puts("GraphicWin actual three-overload controls passed");
}
'''


def run(tmp_path,compiler_name,bodies):
    compiler=shutil.which(compiler_name)
    if compiler is None:pytest.skip(compiler_name+" unavailable")
    cpp,binary=tmp_path/"soft_update.cpp",tmp_path/"soft_update"
    cpp.write_text(HARNESS.replace("@BODIES@","\n\n".join(bodies)))
    build=subprocess.run([compiler,"-std=c++98","-O0","-Wall","-Wextra","-Werror",
                          str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert build.returncode==0,build.stdout+build.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)


@pytest.mark.parametrize("compiler_name",("g++","clang++"))
def test_complete_actual_three_overload_paths(tmp_path,compiler_name):
    result=run(tmp_path,compiler_name,actual())
    assert result.returncode==0,result.stdout+result.stderr
    assert "GraphicWin actual three-overload controls passed" in result.stdout


@pytest.mark.parametrize("damage,witness",[
    ("old_client_inline","Buffer extent provider instead of Win client rectangle"),
    ("wrong_origin","translate from zero client origin"),
    ("erase_true","invalidate without erase"),
    ("convert_before_clip","visibility/clip/convert/invalidate order"),
    ("handle_cached","HandleMain reload after client_to_screen"),
])
def test_compiling_wrong_provider_geometry_and_order_damage(tmp_path,damage,witness):
    bodies=actual()
    if damage=="old_client_inline":bodies[2]=OLD_COORDINATE
    else:
        pointer=bodies[1]
        if damage=="wrong_origin":good,bad="int x = 0, y = 0;","int x = 1, y = 0;"
        elif damage=="erase_true":good,bad="InvalidateRect(HandleMain, &local, FALSE);","InvalidateRect(HandleMain, &local, TRUE);"
        elif damage=="handle_cached":
            good="Win::client_to_screen(&x, &y);"
            bad="HWND saved_handle = HandleMain;\n    Win::client_to_screen(&x, &y);"
            assert pointer.count("InvalidateRect(HandleMain,")==1
            pointer=pointer.replace("InvalidateRect(HandleMain,","InvalidateRect(saved_handle,")
        else:
            good="if (!IntersectRect(&local, &local, &rect2_)) return;\n\n    int x = 0, y = 0;\n    Win::client_to_screen(&x, &y);"
            bad="int x = 0, y = 0;\n    Win::client_to_screen(&x, &y);\n    if (!IntersectRect(&local, &local, &rect2_)) return;"
        assert pointer.count(good)==1,damage
        bodies[1]=pointer.replace(good,bad)
    result=run(tmp_path,"g++",bodies)
    assert result.returncode==43 and f"WITNESS: {witness}" in result.stderr,result.stdout+result.stderr


def test_pinned_complete_overloads_and_buffer_rect_anchors():
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
    for start,end in ((0x5D5720,0x5D588C),(0x5D5890,0x5D592D),(0x5D5930,0x5D5A6E)):
        cursor=start
        for ins in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start):
            assert ins.address==cursor;cursor+=ins.size;instructions[ins.address]=(ins.mnemonic,ins.op_str)
        assert cursor==end
    anchors={
        0x5D57FA:("lea","eax, [esi + 0x474]"),
        0x5D581B:("call","dword ptr [0x669338]"),
        0x5D5864:("mov","eax, dword ptr [0x9b7b28]"),
        0x5D587C:("call","dword ptr [0x669304]"),
        0x5D58A3:("lea","eax, [esi + 0x474]"),
        0x5D590D:("mov","eax, dword ptr [0x9b7b28]"),
        0x5D5916:("push","0"),
        0x5D5922:("call","dword ptr [0x669304]"),
        0x5D5A01:("call","dword ptr [0x669338]"),
        0x5D5A1F:("call","0x5ed240"),
        0x5D5A59:("mov","eax, dword ptr [0x9b7b28]"),
        0x5D5A60:("call","dword ptr [0x669304]"),
    }
    for address,expected in anchors.items():assert instructions[address]==expected,hex(address)
