"""Six complete actual overload bodies; bounded C++98 owner/service controls.

This is not native ABI/layout, Win32 behavior, signed-overflow coverage or game
acceptance. Win pointers keep their real pointer type: no fake callback/proxy
field is used to force parent replacement. Member aliases and recursive flag
changes are executable controls; original separate parent reloads have pinned
instruction anchors, not a fabricated runtime pointer-lifetime proof.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from test_infowin_setup_text import _function_end

ROOT=Path(__file__).resolve().parents[2]
SPEC=(
 ("graphicwin.cpp","int GraphicWin::update(RECT *area, GraphicWin *source)",0x5D5630,0x5D56AA),
 ("graphicwin.cpp","void GraphicWin::update(int x, int y, int width, int height,",0x5D55D0,0x5D562F),
 ("win_point_coordinates.cpp","void Win::nonclient_to_screen(int * x, int * y)",0x5ED3F0,0x5ED48D),
 ("win_point_coordinates.cpp","void Win::client_to_screen(int *x, int *y)",0x5ED240,0x5ED2C5),
 ("buffer.cpp","int Buffer::dotted_box(RECT * a1, int a2)",0x5E32F3,0x5E3370),
 ("buffer.cpp","void Buffer::dotted_box(int left, int top, int right, int bottom, int color)",0x5E3283,0x5E32E4),
)


def actual():
    bodies=[]
    for filename,signature,start,end in SPEC:
        path=ROOT/"src"/filename
        record,=(r for r in read_file(path) if r.address==start)
        assert record.image_spans==((start,end),)
        text=path.read_text()
        # Select the exact overload, never just the first same-named body.
        assert text.count(signature)==1
        opening=text.index(signature)
        bodies.append(text[opening:_function_end(text,opening)])
    return bodies


def test_win_point_and_rectangle_annotations_bind_their_own_overloads():
    for filename,address,signature in (
        ("win_point_coordinates.cpp",0x5ED3F0,"void Win::nonclient_to_screen(int * x, int * y) {"),
        ("win_coordinates.cpp",0x5ED170,"void Win::nonclient_to_screen(RECT * rect) {"),
    ):
        path=ROOT/"src"/filename
        text=path.read_text()
        records=read_file(path)
        record,=(row for row in records if row.address==address)
        assert text.count(signature)==1
        definition_line=text[:text.index(signature)].count("\n")+1
        next_marker=min((row.line for row in records if row.line>record.line),
                        default=len(text.splitlines())+1)
        # The former point marker sat above the RECT sibling and another
        # marker intervened before its real body. Both bindings must be local.
        assert record.line<definition_line<next_marker


HARNESS=r'''
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
struct RECT {int left,top,right,bottom;};
enum {WinFlagClipToParent=0x20};
void require(bool b,const char*w){if(!b){fprintf(stderr,"WITNESS: %s\n",w);exit(47);}}
bool same(const RECT&a,const RECT&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
struct Win {
 uint32_t iFlags_;Win*win_parent_;RECT outer_rect_,client_rect_;
 void client_to_screen(int*,int*);void nonclient_to_screen(int*,int*);
 int is_visible();void update_window(RECT*);void client_to_screen(RECT*);static void flip(RECT*);
};
struct Buffer {
 RECT rect2_;int dotted_box(RECT*,int);void dotted_box(int,int,int,int,int);
 void dotted_hline(int,int,int,int);void dotted_vline(int,int,int,int);
};
struct GraphicWin:Win,Buffer {int update(RECT*,GraphicWin*);void update(int,int,int,int,GraphicWin*);};
GraphicWin graphic,unused_source;
RECT borrowed,expected_rectangle;RECT*mutable_input;
std::string graphic_events;int visible_answer,graphic_mutation;
const RECT changed={71,72,73,74};
int Win::is_visible(){
 require(this==static_cast<Win*>(&graphic),"GraphicWin visibility receiver");graphic_events+='V';
 if(graphic_mutation==1){if(mutable_input)*mutable_input=changed;graphic.rect2_=changed;}
 return visible_answer;
}
void Win::update_window(RECT*r){
 graphic_events+='U';require(this==static_cast<Win*>(&graphic)&&same(*r,expected_rectangle),"GraphicWin rectangle snapshot before callbacks");
 if(graphic_mutation==2){if(mutable_input)*mutable_input=changed;graphic.rect2_=changed;}
}
void Win::client_to_screen(RECT*r){
 graphic_events+='C';require(this==static_cast<Win*>(&graphic)&&same(*r,expected_rectangle),"GraphicWin unchanged local through update callback");
 r->left+=3;r->right+=3;r->top-=2;r->bottom-=2;
}
void Win::flip(RECT*r){
 graphic_events+='F';RECT projected=expected_rectangle;projected.left+=3;projected.right+=3;projected.top-=2;projected.bottom-=2;
 require(same(*r,projected),"GraphicWin screen projection and flip");
}
struct Edge {char type;int a,b,c,color;Edge(char t,int x,int y,int z,int k):type(t),a(x),b(y),c(z),color(k){}};
std::vector<Edge> edges;
Buffer buffer;
RECT box_input;bool mutate_box;
void Buffer::dotted_hline(int a,int b,int c,int color){
 require(this==&buffer,"dotted Buffer receiver");edges.push_back(Edge('H',a,b,c,color));if(mutate_box)box_input=changed;
}
void Buffer::dotted_vline(int a,int b,int c,int color){
 require(this==&buffer,"dotted Buffer receiver");edges.push_back(Edge('V',a,b,c,color));if(mutate_box)box_input=changed;
}
@BODIES@

void graphic_controls(){
 const RECT rectangles[]={{-7,11,39,51},{3,5,3,5},{13,17,7,9}};
 for(int form=0;form<3;++form)for(unsigned r=0;r<3;++r)for(int visible=0;visible<2;++visible)
  for(int mutate=0;mutate<3;++mutate)for(int source=0;source<2;++source){
   graphic.rect2_=rectangles[r];graphic.client_rect_=changed;borrowed=rectangles[r];
   mutable_input=form==0?&borrowed:form==2?&graphic.rect2_:0;
   expected_rectangle=rectangles[r];visible_answer=visible;graphic_mutation=mutate;graphic_events.clear();
   int result=graphic.update(form==0?&borrowed:form==2?&graphic.rect2_:0,source?&unused_source:0);
   require(result==0,"GraphicWin deliberate zero return");
   require(graphic_events==(visible?"VUCF":"V"),"GraphicWin one visibility then update convert flip");
   require(same(graphic.client_rect_,changed),"GraphicWin null selects Buffer bounds");
  }
}
void box_controls(){
 const RECT rectangles[]={{-7,11,39,51},{3,5,3,5},{13,17,7,9}};
 const int colors[]={0x108,-1,79};
 for(unsigned r=0;r<3;++r)for(unsigned k=0;k<3;++k)for(int mutate=0;mutate<2;++mutate){
  box_input=rectangles[r];edges.clear();mutate_box=mutate!=0;
  int result=buffer.dotted_box(&box_input,colors[k]);
  require(result==0&&edges.size()==4,"dotted RECT success and four edges");
  int l=rectangles[r].left,t=rectangles[r].top,rr=rectangles[r].right-1,b=rectangles[r].bottom-1;
  Edge expected[]={Edge('H',l+1,rr,t,colors[k]),Edge('H',l,rr-1,b,colors[k]),
                   Edge('V',l,t,b-1,colors[k]),Edge('V',rr,t+1,b,colors[k])};
  for(unsigned e=0;e<4;++e)require(edges[e].type==expected[e].type&&edges[e].a==expected[e].a&&
      edges[e].b==expected[e].b&&edges[e].c==expected[e].c&&edges[e].color==expected[e].color,
      "dotted inclusive edge snapshots and order");
 }
 edges.clear();require(buffer.dotted_box(static_cast<RECT*>(0),0x108)==0x10&&edges.empty(),"dotted null return and no edges");
}
Win windows[3];
struct Model {uint32_t flags;int parent;RECT outer,client;} model[3];
int point_values[3],expected_values[3];
int*actual_slot(int code){
 if(code<3)return &point_values[code];
 int node=(code-10)/8,field=(code-10)%8;
 if(field==0)return &windows[node].outer_rect_.left;
 if(field==1)return &windows[node].outer_rect_.top;
 if(field==2)return &windows[node].client_rect_.left;
 if(field==3)return &windows[node].client_rect_.top;
 return reinterpret_cast<int*>(&windows[node].iFlags_); // corresponding signed/unsigned type alias
}
int&expected_slot(int code){
 if(code<3)return expected_values[code];
 int node=(code-10)/8,field=(code-10)%8;
 if(field==0)return model[node].outer.left;
 if(field==1)return model[node].outer.top;
 if(field==2)return model[node].client.left;
 if(field==3)return model[node].client.top;
 return reinterpret_cast<int&>(model[node].flags);
}
// Separate sequential memory model retains pointer aliases and reads state
// again after each write/recursive walk. No zero-origin shortcut is used.
void oracle_client(int node,int x,int y){
 expected_slot(x)=expected_slot(x)+model[node].outer.left+model[node].client.left;
 expected_slot(y)=expected_slot(y)+model[node].outer.top+model[node].client.top;
 if((model[node].flags&0x20)==0||model[node].parent<0)return;
 oracle_client(model[node].parent,x,y);
 if((model[node].flags&0x8000)==0)return;
 expected_slot(x)=expected_slot(x)-model[model[node].parent].outer.left;
 expected_slot(y)=expected_slot(y)-model[model[node].parent].outer.top;
}
void point_controls(){
 const int slots[]={0,1,2,10,11,12,13,14,18,19,20,21,22,26,27};
 const uint32_t flags[]={0,0x20,0x8020,0x7fe0,0xffe0};
 for(unsigned f=0;f<5;++f)for(int recurse=0;recurse<2;++recurse)
  for(unsigned x=0;x<15;++x)for(unsigned y=0;y<15;++y){
   for(int node=0;node<3;++node){
    windows[node].iFlags_=model[node].flags=node==0?flags[f]:node==1&&recurse?0x20:0;
    windows[node].win_parent_=node<2?&windows[node+1]:0;model[node].parent=node<2?node+1:-1;
    RECT outer={10+node*30,20+node*40,110,120},client={2+node*3,3+node*4,90,80};
    windows[node].outer_rect_=model[node].outer=outer;windows[node].client_rect_=model[node].client=client;
   }
   point_values[0]=expected_values[0]=7;point_values[1]=expected_values[1]=11;point_values[2]=expected_values[2]=13;
   oracle_client(0,slots[x],slots[y]);
   expected_slot(slots[x])=expected_slot(slots[x])-model[0].outer.left;
   expected_slot(slots[y])=expected_slot(slots[y])-model[0].outer.top;
   windows[0].nonclient_to_screen(actual_slot(slots[x]),actual_slot(slots[y]));
   for(int i=0;i<3;++i){
    require(point_values[i]==expected_values[i],"Win point shared-pointer and recursive member aliases");
    require(windows[i].iFlags_==model[i].flags&&same(windows[i].outer_rect_,model[i].outer)&&same(windows[i].client_rect_,model[i].client),
            "Win point late own/parent geometry and flags reads");
   }
  }
}
int main(){graphic_controls();box_controls();point_controls();puts("rectangle overload cohort controls passed");}
'''


def run(tmp_path,name,bodies):
    compiler=shutil.which(name)
    if not compiler:pytest.skip(name+" unavailable")
    cpp,binary=tmp_path/"cohort.cpp",tmp_path/"cohort"
    cpp.write_text(HARNESS.replace("@BODIES@","\n\n".join(bodies)))
    compiled=subprocess.run([compiler,"-std=c++98","-O0","-Wall","-Wextra","-Werror","-Wno-unused-parameter",
                             str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)


@pytest.mark.parametrize("name",("g++","clang++"))
def test_complete_six_actual_overloads(tmp_path,name):
    result=run(tmp_path,name,actual())
    assert result.returncode==0,result.stdout+result.stderr
    assert "rectangle overload cohort controls passed" in result.stdout


@pytest.mark.parametrize("damage,witness",[
 ("graphic_wrong_null_bounds","GraphicWin rectangle snapshot before callbacks"),
 ("graphic_swapped_extents","GraphicWin rectangle snapshot before callbacks"),
 ("point_value_copies","Win point shared-pointer and recursive member aliases"),
 ("point_early_outer","Win point shared-pointer and recursive member aliases"),
 ("point_wrong_parent_receiver","Win point late own/parent geometry and flags reads"),
 ("point_stale_flags","Win point shared-pointer and recursive member aliases"),
 ("box_no_decrement","dotted inclusive edge snapshots and order"),
 ("box_wrong_edge_order","dotted inclusive edge snapshots and order"),
 ("box_late_borrowed_read","dotted inclusive edge snapshots and order"),
])
def test_compiling_boundary_damage_controls(tmp_path,damage,witness):
    bodies=actual()
    if damage=="graphic_wrong_null_bounds":i,good,bad=0,"area = &rect2_","area = &client_rect_"
    elif damage=="graphic_swapped_extents":
        i=0
        good="update(area->left, area->top, area->right - area->left,\n           area->bottom - area->top, source)"
        bad="update(area->left, area->top, area->bottom - area->top,\n           area->right - area->left, source)"
    elif damage=="point_value_copies":
        bodies[2]='''void Win::nonclient_to_screen(int * x, int * y) {
            int xx=*x,yy=*y;client_to_screen(&xx,&yy);
            xx-=outer_rect_.left;yy-=outer_rect_.top;*x=xx;*y=yy;
        }'''
        i=good=bad=None
    elif damage=="point_early_outer":
        i=2;good="    client_to_screen(x, y);"
        bad="    int old_left=outer_rect_.left,old_top=outer_rect_.top;\n    client_to_screen(x, y);"
        assert bodies[i].count("*x = *x - outer_rect_.left;")==1
        bodies[i]=bodies[i].replace("*x = *x - outer_rect_.left;","*x = *x - old_left;").replace("*y = *y - outer_rect_.top;","*y = *y - old_top;")
    elif damage=="point_wrong_parent_receiver":i,good,bad=3,"win_parent_->client_to_screen(x, y)","win_parent_->nonclient_to_screen(x, y)"
    elif damage=="point_stale_flags":
        i=3;good="win_parent_->client_to_screen(x, y);"
        bad="const uint32_t before_parent_flags=iFlags_;\n    win_parent_->client_to_screen(x, y);"
        assert bodies[i].count("(iFlags_ & 0x8000U)")==1
        bodies[i]=bodies[i].replace("(iFlags_ & 0x8000U)","(before_parent_flags & 0x8000U)")
    elif damage=="box_no_decrement":i,good,bad=4,"--right;","(void)right;"
    elif damage=="box_wrong_edge_order":
        i=5;good="dotted_hline(left + 1, right, top, color);\n    dotted_hline(left, right - 1, bottom, color);"
        bad="dotted_hline(left, right - 1, bottom, color);\n    dotted_hline(left + 1, right, top, color);"
    else:
        # Reintroduce borrowed-field reads between edge service callbacks.
        bodies[4]='''int Buffer::dotted_box(RECT * a1, int a2) {
            if(!a1)return 0x10;
            dotted_hline(a1->left+1,a1->right-1,a1->top,a2);
            dotted_hline(a1->left,a1->right-2,a1->bottom-1,a2);
            dotted_vline(a1->left,a1->top,a1->bottom-2,a2);
            dotted_vline(a1->right-1,a1->top+1,a1->bottom-1,a2);return 0;
        }'''
        i=good=bad=None
    if i is not None:
        assert bodies[i].count(good)==1,damage
        bodies[i]=bodies[i].replace(good,bad)
    result=run(tmp_path,"g++",bodies)
    assert result.returncode==47 and f"WITNESS: {witness}" in result.stderr,result.stdout+result.stderr


def test_pinned_complete_six_spans_and_boundary_anchors():
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
    for _,_,start,end in SPEC:
        cursor=start
        for ins in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start):
            assert ins.address==cursor;cursor+=ins.size;instructions[ins.address]=(ins.mnemonic,ins.op_str)
        assert cursor==end
    # Operand details below are original facts, not fresh byte-match claims.
    assert instructions[0x5D563F][1].endswith("+ 0x474]")
    for address in (0x5D5658,0x5D5680,0x5D568C,0x5D5696,0x5ED43B,
                    0x5E3326,0x5E333C,0x5E3352,0x5E3362):
        assert instructions[address][0]=="call",hex(address)
    assert instructions[0x5ED43B][1]=="0x5ed240"
    anchors={
        0x5ED440:("mov","eax, dword ptr [esi + 0x98]"),
        0x5ED446:("test","ah, 0x80"),
        0x5ED44B:("mov","edx, dword ptr [esi + 0xc4]"),
        0x5ED453:("mov","eax, dword ptr [edx + 0x13c]"),
        0x5ED45D:("mov","ecx, dword ptr [esi + 0xc4]"),
        0x5ED465:("mov","edx, dword ptr [ecx + 0x140]"),
        0x5ED46F:("mov","eax, dword ptr [esi + 0x13c]"),
        0x5ED47B:("mov","ecx, dword ptr [esi + 0x140]"),
    }
    for address,expected in anchors.items():assert instructions[address]==expected,hex(address)
