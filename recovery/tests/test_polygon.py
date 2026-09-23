"""Actual flat polygon family with independent bounded raster observations.

The whole polygon TU (apart from includes) is compiled unchanged. Buffer
acquisition/release are recording doubles with the real clip/pitch declarations;
Vert and edge records are actual source. Expected convex/monotone pixels use
rational edge intersections rounded upward, not the production recurrence.
Coordinates establish the original extrema sentinels and avoid overflowing
arithmetic. Real locking, native ABI, indirect scratch users and game rendering
are not established here. The coupled Gouraud family is not covered.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    source = (ROOT / "src/polygon.cpp").read_text()
    # Replace only the environment includes, never a function expression.
    source = re.sub(r"^\s*#include[^\n]*\n", "", source, flags=re.M)
    header = (ROOT / "src/buffer.h").read_text()
    fields = []
    for declaration in (r"RECT rect1_;", r"uint32_t stride_;"):
        field, = re.findall(declaration, header)
        fields.append(field)
    vertex = braced((ROOT / "src/vert.h").read_text(), "struct Vert {") + ";"
    return source, "\n".join(fields), vertex


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <vector>
#include <string>
#define __cdecl
void check(bool ok,int line) {
    if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(87);}
}
#define require(x) check((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
ACTUAL_VERTEX
static unsigned char pixels[4096],expected[4096],lookup[256];
static unsigned char *const origin=pixels+1024;
bool fail_lock,release_mutates;
RECT live_clip;
unsigned live_pitch;
std::string events;
void acquire_observer();
void release_observer(int);
struct Buffer {
    ACTUAL_FIELDS
    intptr_t get_data() {
        acquire_observer();rect1_=live_clip;stride_=live_pitch;
        return fail_lock ? 0 : reinterpret_cast<intptr_t>(origin);
    }
    void free_data(int count){release_observer(count);}
} buffer;
ACTUAL_SOURCE
enum {COLOR,PIXELS,BOTTOM,VERTICES,LEFT,TOP,REMAP,COUNT,Y,PITCH,RIGHT,CLIP_BOTTOM};
typedef std::vector<intptr_t> State;
State before,at_release;
std::vector<unsigned char> pixels_at_release;
State snapshot() {
    return {PolygonColor,reinterpret_cast<intptr_t>(PolygonPixels),PolygonBottomVertex,
            reinterpret_cast<intptr_t>(PolygonVertices),PolygonClipLeft,PolygonClipTop,
            reinterpret_cast<intptr_t>(PolygonRemap),PolygonVertexCount,PolygonY,
            PolygonPitch,PolygonClipRight,PolygonClipBottom};
}
void acquire_observer() {
    require(events.empty() && snapshot()==before);events+='G';
}
void release_observer(int count) {
    require(count==1 && events=="G");events+='F';
    at_release=snapshot();pixels_at_release.assign(pixels,pixels+sizeof(pixels));
    if(release_mutates){PolygonY=1234;PolygonColor=-321;PolygonRemap=nullptr;}
}
void start() {
    for(unsigned i=0;i<sizeof(pixels);++i)pixels[i]=expected[i]=(i*37+11)&255;
    for(unsigned i=0;i<256;++i)lookup[i]=(i*13+7)&255;
    live_clip={-4,-4,18,18};live_pitch=40;fail_lock=release_mutates=false;
    buffer.rect1_={90,91,92,93};buffer.stride_=63;
    PolygonColor=301;PolygonPixels=pixels+5;PolygonBottomVertex=71;
    PolygonVertices=nullptr;PolygonClipLeft=72;PolygonClipTop=73;
    PolygonRemap=lookup+1;PolygonVertexCount=74;PolygonY=75;
    PolygonPitch=76;PolygonClipRight=77;PolygonClipBottom=78;
    events.clear();at_release.clear();pixels_at_release.clear();before=snapshot();
}
int ceiling(int numerator,int denominator) {
    require(denominator>0);
    return numerator>=0 ? (numerator+denominator-1)/denominator : numerator/denominator;
}
void oracle(const std::vector<Vert> &vertices,int colour,bool remap,int alias=-1) {
    if(vertices.size()<2)return;
    int bottom=vertices[0].y,top=bottom;
    for(auto v:vertices){top=std::min(top,v.y);bottom=std::max(bottom,v.y);}
    for(int y=std::max(top,live_clip.top);y<std::min(bottom,live_clip.bottom-1);++y) {
        std::vector<int> intersections;
        for(unsigned i=0;i<vertices.size();++i) {
            Vert a=vertices[i],b=vertices[(i+1)%vertices.size()];
            if(a.y>b.y)std::swap(a,b);
            if(a.y<=y && y<b.y) {
                int numerator=a.x*(b.y-a.y)+(y-a.y)*(b.x-a.x);
                intersections.push_back(ceiling(numerator,b.y-a.y));
            }
        }
        if(intersections.empty())continue;
        require(intersections.size()==2);
        int left=std::max(live_clip.left,*std::min_element(intersections.begin(),intersections.end()));
        int right=std::min(live_clip.right-1,*std::max_element(intersections.begin(),intersections.end()));
        for(int x=left;x<right;++x) {
            int index=1024+y*live_pitch+x;require(index>=0 && index<4096);
            if(!remap)expected[index]=static_cast<unsigned char>(colour);
            else expected[index]=alias<0 ? lookup[expected[index]] : expected[alias+expected[index]];
        }
    }
}
void run(std::vector<Vert> &vertices,int count,bool remap,int colour=0x143,int alias=-1) {
    Vert *v=vertices.data();unsigned char *lut=alias<0 ? lookup : pixels+alias;
    State final=before;
    final[PIXELS]=fail_lock ? 0 : reinterpret_cast<intptr_t>(origin);
    if(!fail_lock) {
        final[VERTICES]=reinterpret_cast<intptr_t>(v);
        final[remap ? REMAP : COLOR]=remap ? reinterpret_cast<intptr_t>(lut) : colour;
        final[COUNT]=count;final[PITCH]=live_pitch;
        final[LEFT]=live_clip.left;final[TOP]=live_clip.top;
        final[RIGHT]=live_clip.right-1;final[CLIP_BOTTOM]=live_clip.bottom-1;
        if(count>0) {
            int low=32767,high=-32765;
            for(int i=0;i<count;++i) {
                low=std::min(low,v[i].y);
                if(v[i].y>high){high=v[i].y;final[BOTTOM]=i;}
            }
            if(low<high)final[Y]=low>=live_clip.bottom-1 ? low : std::min(high-1,live_clip.bottom-1);
        }
    }
    int result=remap ? polygon_dest(&buffer,v,count,lut) : polygon(&buffer,v,count,colour);
    require(result==0 && events=="GF");
    require(at_release==final);
    if(release_mutates){final[Y]=1234;final[COLOR]=-321;final[REMAP]=0;}
    require(snapshot()==final);
    require(std::memcmp(pixels,expected,sizeof(pixels))==0);
    require(std::equal(pixels_at_release.begin(),pixels_at_release.end(),expected));
}
void full_polygons() {
    const std::vector<std::vector<Vert>> shapes={
        {{1,1},{11,1},{11,10},{1,10}},
        {{2,1},{12,9},{-2,9}},
        {{-3,-3},{12,2},{8,12},{1,10}},
        {{2,-3},{8,-3},{13,2},{11,9},{3,12},{-3,5}},
        {{1,1},{1,1},{11,1},{11,10},{1,10},{1,10}},
        {{0,0},{6,9},{3,9}},
    };
    for(auto shape:shapes)for(int winding:{0,1})for(unsigned shift=0;shift<shape.size();++shift)
    for(bool remap:{false,true})for(int clipcase:{0,1,2,3}) {
        auto vertices=shape;
        if(winding)std::reverse(vertices.begin(),vertices.end());
        std::rotate(vertices.begin(),vertices.begin()+shift,vertices.end());
        start();live_pitch=clipcase&1 ? 40 : 48;
        if(clipcase==1)live_clip={3,4,10,9};
        if(clipcase==2)live_clip={-4,20,18,24};
        if(clipcase==3)live_clip={-4,-4,18,0};
        oracle(vertices,0x143,remap);run(vertices,vertices.size(),remap);
    }
    // Source-remap bytes alias destination bytes. The first output alters
    // lookup[0], which later pixels must observe; a copied LUT is incorrect.
    for(bool remap:{true}) {
        std::vector<Vert> v={{0,0},{3,0},{3,2},{0,2}};
        start();pixels[1024]=expected[1024]=1;
        pixels[1025]=expected[1025]=0;pixels[1026]=expected[1026]=0;
        oracle(v,0,remap,1024);run(v,4,remap,0,1024);
        require(pixels[1024]==0 && pixels[1025]==0);
    }
    for(bool remap:{false,true}) {
        std::vector<Vert> v={{1,1},{8,1},{4,7}};
        start();release_mutates=true;oracle(v,0x143,remap);run(v,3,remap);
        start();fail_lock=true;run(v,3,remap);
        for(int count:{0,-1}){start();run(v,count,remap);}
        v={{2,2},{3,2},{4,2}};start();run(v,3,remap);
        v={{2,2}};start();run(v,1,remap);
        v={{1,1},{5,7}};start();oracle(v,0x143,remap);run(v,2,remap);
        start();
        require((remap ? polygon_dest(nullptr,v.data(),1,lookup) : polygon(nullptr,v.data(),1,4))==16);
        require(events.empty() && snapshot()==before);
        require((remap ? polygon_dest(&buffer,nullptr,1,lookup) : polygon(&buffer,nullptr,1,4))==16);
        require(events.empty() && snapshot()==before);
    }
}
void helper_controls() {
    static_assert(sizeof(FlatPolygonEdge)==9*sizeof(int),"actual nine-int edge state");
    Vert v[]={{2,1},{6,1},{1,5},{0,3}};
    start();PolygonVertices=v;PolygonVertexCount=4;PolygonBottomVertex=2;
    FlatPolygonEdge edge={1,91,92,93,94,95,96,97,98};
    require(setup_flat_polygon_edge(&edge,0)==1);
    const int expected_setup[]={1,4,2,6,-1,-1,-3,1,4};
    require(std::memcmp(&edge,expected_setup,sizeof(edge))==0);
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==5 && edge.error==-2);
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==4 && edge.error==-1);
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==3 && edge.error==0);
    require(advance_flat_polygon_edge(&edge)==0 && edge.remaining_rows==0 && edge.x==3);
    // Terminal setup changes nothing; descending setup publishes the
    // negative remaining-row difference but leaves other fields untouched.
    FlatPolygonEdge old=edge;require(setup_flat_polygon_edge(&edge,2)==0);
    require(std::memcmp(&edge,&old,sizeof(edge))==0);
    edge.direction=-1;require(setup_flat_polygon_edge(&edge,0)==1);
    require(edge.x==2 && edge.x_step==-1 && edge.error==-1 && edge.remaining_rows==2);
    PolygonBottomVertex=3;edge.direction=1;old=edge;
    require(setup_flat_polygon_edge(&edge,2)==0 && edge.remaining_rows==-2);
    old.remaining_rows=-2;require(std::memcmp(&edge,&old,sizeof(edge))==0);
    // Strictly-positive error correction: zero is a tie, not a step.
    edge={1,3,2,3,0,1,-1,1,4};
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==3 && edge.error==0);
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==4 && edge.error==-3);
    Vert horizontal[]={{0,1},{2,1},{3,1}};
    PolygonVertices=horizontal;PolygonVertexCount=3;PolygonBottomVertex=2;
    edge={1,91,92,93,94,95,96,97,98};old=edge;
    require(setup_flat_polygon_edge(&edge,0)==0);
    old.remaining_rows=0;require(std::memcmp(&edge,&old,sizeof(edge))==0);
    Vert successive[]={{0,0},{2,2},{5,5},{0,5}};
    PolygonVertices=successive;PolygonVertexCount=4;PolygonBottomVertex=2;
    edge.direction=1;require(setup_flat_polygon_edge(&edge,0)==1);
    require(advance_flat_polygon_edge(&edge)==1 && edge.x==1);
    require(advance_flat_polygon_edge(&edge)==1);
    const int expected_transition[]={1,3,2,2,1,1,0,0,3};
    require(std::memcmp(&edge,expected_transition,sizeof(edge))==0);
}
void span_controls() {
    for(bool remap:{false,true})for(int left:{-6,-2,0,4,10})for(int right:{-4,0,2,8,12}) {
        start();PolygonPixels=origin;PolygonPitch=40;PolygonY=-2;
        PolygonClipLeft=-2;PolygonClipRight=8;PolygonColor=0x1ab;PolygonRemap=lookup;
        // Standalone spans have no Y clipping and do not sort endpoints.
        for(int x=std::max(left,-2);x<std::min(right,8);++x) {
            int at=1024-80+x;expected[at]=remap ? lookup[expected[at]] : 0xab;
        }
        State saved=snapshot();
        if(remap)draw_remapped_polygon_span(left,right);else draw_solid_polygon_span(left,right);
        require(snapshot()==saved && events.empty());
        require(std::memcmp(pixels,expected,sizeof(pixels))==0);
    }
}
int main(){full_polygons();helper_controls();span_controls();return 0;}
'''


def run_host(tmp_path, source, fields, vertex):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    cpp = tmp_path / "polygon.cpp"
    cpp.write_text(HARNESS.replace("ACTUAL_FIELDS", fields).replace("ACTUAL_VERTEX", vertex)
                   .replace("ACTUAL_SOURCE", source))
    binary = tmp_path / "polygon"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_flat_polygon_family(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("buffer->rect1_.right - 1", "buffer->rect1_.right"),
    ("buffer->rect1_.bottom - 1", "buffer->rect1_.bottom"),
    ("edge->error > 0", "edge->error >= 0"),
    ("edge->error = 1 - edge->remaining_rows", "edge->error = 0"),
    ("edge->x_step = -(delta / edge->remaining_rows)", "edge->x_step = delta / edge->remaining_rows"),
    ("--edge->remaining_rows == 0", "edge->remaining_rows-- == 0"),
    ("if (vertex == PolygonBottomVertex)", "if (false)"),
    ("vertices[vertex].y > maximum_y", "vertices[vertex].y >= maximum_y"),
    ("PolygonY >= PolygonClipTop", "PolygonY > PolygonClipTop"),
    ("PolygonColor = color;", "PolygonColor = color + 1;"),
    ("static_cast<int>(buffer->stride_)", "48"),
    ("buffer->free_data(1);", "if (PolygonPixels) buffer->free_data(1);"),
    ("buffer->free_data(1);", "buffer->free_data(2);"),
    ("return 16;", "return 0;"),
    ("unsigned char *remap = PolygonRemap;",
     "unsigned char frozen[256]; std::memcpy(frozen, PolygonRemap, 256); unsigned char *remap = frozen;"),
    ("*pixel = remap[*pixel];", "*pixel = remap[static_cast<unsigned char>(*pixel + 1)];"),
    ("if (left < PolygonClipLeft)", "if (false)"),
    ("if (right >= PolygonClipRight)", "if (false)"),
    ("if (!advance_flat_polygon_edge(&backward))", "if (advance_flat_polygon_edge(&backward))"),
    ("++PolygonY;", "PolygonY += 2;"),
])
def test_flat_polygon_damage_is_observed(tmp_path, before, after):
    source, fields, vertex = actual_source()
    assert before in source
    result = run_host(tmp_path, source.replace(before, after), fields, vertex)
    assert result.returncode == 87, result.stderr
