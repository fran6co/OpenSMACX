"""Actual four-vertex Gouraud polygon with independent rational observations.

Uses the entire polygon source, real EdgeScan and actual fixed_div. Python
Fraction arithmetic independently models direct PC24/chop division, then
WORD conversion; it is not Texture's reciprocal/multiply oracle. Geometry
uses closed-form directed edge intersections and fixed-point light planes.
FPU calls and Buffer are explicit observers, not native x87 status, locking,
ABI or game proof. Pointer comparisons stay in a nonwrapping same-order
allocation domain, including bounded zero/negative pitches.
Host function-entry instrumentation observes real fixed_div calls during
edge transitions, without replacing that dependency or altering its body.
"""

from fractions import Fraction
from pathlib import Path
import os
import random
import shutil
import subprocess

import pytest

from test_polygon import actual_source


def direct_word_step(delta, width):
    if not delta:
        return 0
    ratio = Fraction(abs(delta), width * 65536)
    exponent = ratio.numerator.bit_length() - ratio.denominator.bit_length()
    power = Fraction(2) ** exponent
    if power > ratio:
        exponent -= 1
    quantum = Fraction(2) ** (exponent - 23)
    rounded = (ratio // quantum) * quantum
    magnitude = int(rounded * 256)
    signed = -magnitude if delta < 0 else magnitude
    return signed & 65535 if -32768 <= signed <= 32767 else 32768


def trunc_div(numerator, denominator):
    return (-1 if numerator < 0 else 1) * (abs(numerator) // denominator)


def edge_at(vertices, lights, direction, y):
    top = min(range(4), key=lambda i: vertices[i][1])
    bottom = max(range(4), key=lambda i: vertices[i][1])
    index = top
    while index != bottom:
        following = (index + direction) % 4
        x0, y0 = vertices[index]
        x1, y1 = vertices[following]
        if y0 <= y < y1:
            numerator = x0 * (y1 - y0) + (y - y0) * (x1 - x0)
            x = -((-numerator) // (y1 - y0))
            light = lights[index] * 65536 + (y - y0) * trunc_div(
                (lights[following] - lights[index]) * 65536, y1 - y0)
            return x, light
        assert y1 >= y0, "oracle covers monotone chains only"
        index = following
    raise AssertionError("row beyond selected maximum")


def raster_case(vertices, lights, clip, pitch):
    top = min(y for _, y in vertices)
    bottom = max(y for _, y in vertices)
    # Independent initial poison values distinguish untouched scratch slots.
    span_left, span_right, light, step = 82, 80, 81, 83
    row = top
    writes = []
    if top * pitch < clip[3] * pitch:
        for y in range(top, bottom):
            row = y
            span_left, light = edge_at(vertices, lights, -1, y)
            span_right, right_light = edge_at(vertices, lights, 1, y)
            if span_right > span_left:
                step = direct_word_step(right_light - light, span_right - span_left)
            if y * pitch >= clip[3] * pitch:
                break
            if (y * pitch >= clip[1] * pitch and span_right > clip[0]
                    and span_left < clip[2] and span_left < span_right):
                end = min(span_right, clip[2])
                if span_left < clip[0]:
                    # Literal original: unsigned WORD * INTEGER skipped
                    # pixels, >>8, added to16.16. Not the usual <<8.
                    light += (step * (clip[0] - span_left)) // 256
                    span_left = clip[0]
                for x in range(span_left, end + 1):
                    value = (((light // 256) + (x - span_left) * step) & 65535) // 256
                    offset = 1024 + y * pitch + x
                    assert 0 <= offset < 4096
                    writes.append((offset, value))
    return (row, span_left, span_right, light, step, writes)


def generated_cases():
    geometry = [
        ([(1, 1), (4, 1), (4, 4), (1, 4)], [0, 3, 8, 5]),
        ([(1, 1), (4, 1), (4, 4), (1, 4)], [3, 0, -5, -2]),
        ([(2, -2), (8, 0), (11, 8), (-2, 6)], [-3, 17, 25, 4]),
        ([(1, 1), (2, 1), (2, 4), (1, 4)], [200, -200, -200, 200]),
    ]
    cases = []
    for vertices, lights in geometry:
        for reverse in (False, True):
            pairs = list(zip(vertices, lights))
            if reverse:
                pairs.reverse()
            for rotation in (0, 1, 3):
                rotated = pairs[rotation:] + pairs[:rotation]
                v, z = [p[0] for p in rotated], [p[1] for p in rotated]
                for clip in [(-4, -4, 16, 12), (3, 2, 9, 5), (-4, 3, 16, 3), (-4, -4, 16, -3)]:
                    cases.append((v, z, clip, 40))
    rectangle, lights = geometry[0]
    cases.extend((rectangle, lights, clip, pitch) for clip, pitch in [
        ((0, 0, 12, 6), 0), ((0, 0, 12, 6), -40),
        ((0, 4, 12, 0), -40), ((0, 2, 12, 0), -40),
    ])
    # Long clipped prefix requires edge phase to advance without constructing
    # or dereferencing out-of-allocation pointers for invisible rows.
    cases.append(([(1, -80), (4, -80), (4, 3), (1, 3)], [0, 3, 7, 4], (0, 0, 12, 6), 40))
    return cases


def case_cpp(case):
    vertices, lights, clip, pitch = case
    row, left, right, light, step, writes = raster_case(*case)
    bottom = max(range(4), key=lambda i: vertices[i][1])
    points = ",".join("{%d,%d}" % point for point in vertices)
    updates = ",".join("{%d,%d}" % item for item in writes)
    return "{{%s},{%s},{%s},%d,%d,%d,%d,%d,%d,%d,{%s}}" % (
        points, ",".join(map(str, lights)), ",".join(map(str, clip)),
        pitch, row, left, right, light, step, bottom, updates)


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <vector>
#include <string>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(89);}}
#define require(x) check((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
ACTUAL_VERTEX
unsigned char pixels[4096],expected[4096],dummy_lut[256];
unsigned char *const origin=pixels+1024;
int __cdecl fixed_div(int,int);
bool trace_enabled;
int trace_counts[16],trace_size;
extern "C" void __cyg_profile_func_enter(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_exit(void *,void *) __attribute__((no_instrument_function));
extern "C" void __cyg_profile_func_enter(void *function,void *){
    if(trace_enabled && function==reinterpret_cast<void *>(fixed_div) && trace_size<16){
        int changed=0;
        for(unsigned i=0;i<sizeof(pixels);++i)changed+=pixels[i]!=231;
        trace_counts[trace_size++]=changed;
    }
}
extern "C" void __cyg_profile_func_exit(void *,void *){}
RECT live_clip;
int live_pitch;
bool fail_lock,rows_are_addresses;
std::string events;
const unsigned _MCW_EM=0x003f,_MCW_PC=0x0300,_MCW_RC=0x0c00,_MCW_IC=0x1000;
const unsigned _PC_64=0x0300,_PC_24=0,_RC_NEAR=0,_IC_PROJECTIVE=0,_RC_CHOP=0x0c00;
void acquire_observer();void release_observer(int);void reset_observer();
void _fpreset(){reset_observer();}
unsigned _control87(unsigned value,unsigned mask){
    if(mask==(_MCW_PC|_MCW_RC)){
        require(value==(_PC_24|_RC_CHOP));events+='Q';
    }else{
        require(mask==(_MCW_EM|_MCW_PC|_MCW_RC|_MCW_IC));
        require(value==(_MCW_EM|_PC_64|_RC_NEAR|_IC_PROJECTIVE));events+='C';
    }
    return 0;
}
struct Buffer {
    ACTUAL_FIELDS
    intptr_t get_data(){acquire_observer();rect1_=live_clip;stride_=live_pitch;
        return fail_lock ? 0 : reinterpret_cast<intptr_t>(origin);}
    void free_data(int n){release_observer(n);}
} buffer;
ACTUAL_SOURCE
enum {COLOR,PIXELS,BOTTOM,VERTICES,LEFT,TOP,REMAP,COUNT,Y,PITCH,RIGHT,CLIP_BOTTOM,
      SPAN_RIGHT,LIGHT,SPAN_LEFT,LIGHTS,LIGHT_STEP};
typedef std::vector<intptr_t> State;
State before,released;
std::vector<unsigned char> released_pixels;
intptr_t row_value(const PolygonRowPosition &row,bool address){
    // Inspect the selected representation without reading an inactive member.
    if(address){uintptr_t value;std::memcpy(&value,&row,sizeof(value));return value;}
    int value;std::memcpy(&value,&row,sizeof(value));return value;
}
State snapshot(bool addresses){return {PolygonColor,reinterpret_cast<intptr_t>(PolygonPixels),
    PolygonBottomVertex,reinterpret_cast<intptr_t>(PolygonVertices),PolygonClipLeft,
    row_value(PolygonClipTop,addresses),reinterpret_cast<intptr_t>(PolygonRemap),PolygonVertexCount,
    row_value(PolygonY,addresses),PolygonPitch,PolygonClipRight,row_value(PolygonClipBottom,addresses),
    PolygonSpanRight,PolygonLight,PolygonSpanLeft,reinterpret_cast<intptr_t>(PolygonLights),PolygonLightStep};}
void acquire_observer(){require(events.empty() && snapshot(false)==before);events+='G';}
void release_observer(int n){
    require(n==1 && (events=="GRCQ" || events=="GRC" || events=="G"));
    events+='F';released=snapshot(rows_are_addresses);
    released_pixels.assign(pixels,pixels+sizeof(pixels));
}
void reset_observer(){
    require(events=="G" || (!events.empty() && events.back()=='F'));
    if(events=="G"){
        State published=before;published[PIXELS]=reinterpret_cast<intptr_t>(origin);
        require(snapshot(false)==published);
    }
    events+='R';
}
void start(){
    std::memset(pixels,231,sizeof(pixels));std::memset(expected,231,sizeof(expected));
    live_clip={0,0,12,12};live_pitch=40;fail_lock=rows_are_addresses=false;
    buffer.rect1_={90,91,92,93};buffer.stride_=63;
    PolygonColor=301;PolygonPixels=pixels+5;PolygonBottomVertex=71;PolygonVertices=nullptr;
    PolygonClipLeft=72;PolygonClipTop.coordinate=73;PolygonRemap=dummy_lut;
    PolygonVertexCount=74;PolygonY.coordinate=75;PolygonPitch=76;
    PolygonClipRight=77;PolygonClipBottom.coordinate=78;
    PolygonSpanRight=80;PolygonLight=81;PolygonSpanLeft=82;PolygonLights=nullptr;PolygonLightStep=83;
    events.clear();released.clear();released_pixels.clear();before=snapshot(false);
}
struct Write {int offset,value;};
struct Case {Vert vertices[4];int lights[4];RECT clip;int pitch,row,left,right,light,step,bottom;
             std::vector<Write> writes;};
void normal(Case &c){
    start();live_clip=c.clip;live_pitch=c.pitch;rows_are_addresses=true;
    for(const auto &write:c.writes)expected[write.offset]=write.value;
    State final=before;
    final[PIXELS]=reinterpret_cast<intptr_t>(origin);final[VERTICES]=reinterpret_cast<intptr_t>(c.vertices);
    final[LIGHTS]=reinterpret_cast<intptr_t>(c.lights);final[BOTTOM]=c.bottom;
    final[PITCH]=c.pitch;final[LEFT]=c.clip.left;final[RIGHT]=c.clip.right;
    final[TOP]=reinterpret_cast<intptr_t>(origin)+c.pitch*c.clip.top;
    final[CLIP_BOTTOM]=reinterpret_cast<intptr_t>(origin)+c.pitch*c.clip.bottom;
    final[Y]=reinterpret_cast<intptr_t>(origin)+c.pitch*c.row;
    final[SPAN_LEFT]=c.left;final[SPAN_RIGHT]=c.right;final[LIGHT]=c.light;final[LIGHT_STEP]=c.step;
    require(polygon_gouraud(&buffer,c.vertices,c.lights)==0);
    require(events=="GRCQFRC" && released==final && snapshot(true)==final);
    require(std::memcmp(pixels,expected,sizeof(pixels))==0);
    require(std::equal(released_pixels.begin(),released_pixels.end(),expected));
}
int main(){
    const int slopes[][3]={SLOPE_CASES};
    for(const auto &s:slopes)require(polygon_light_step(s[0],s[1]*65536)==s[2]);
    require(polygon_light_step(196608,3*65536)==256);
    require(fixed_div(-65536,196608)==-21845);
    require(fixed_div(65536,196608)==21845);
    Case cases[]={RASTER_CASES};
    for(auto &c:cases)normal(c);
    // Right changes segment while advancing row1, before that row's four
    // pixels; left changes on row2, before its seven pixels. Pure image
    // comparison alone would not observe this internal call ordering.
    Case traced=TRACE_CASE;
    trace_size=0;trace_enabled=true;normal(traced);trace_enabled=false;
    require(trace_size==4 && trace_counts[0]==0 && trace_counts[1]==0
            && trace_counts[2]==0 && trace_counts[3]==4);
    Vert v[]={{1,1},{4,1},{4,4},{1,4}};int z[]={0,3,3,0};
    start();fail_lock=true;
    require(polygon_gouraud(&buffer,v,z)==0);
    State final=before;final[PIXELS]=0;
    require(events=="GFRC" && snapshot(false)==final && released==final);
    start();require(polygon_gouraud(&buffer,nullptr,z)==16);
    require(events.empty() && snapshot(false)==before);
    require(polygon_gouraud(&buffer,v,nullptr)==16);
    require(events.empty() && snapshot(false)==before);
    // A null Buffer with valid geometry has no recovered original contract.
    require(polygon_gouraud(nullptr,nullptr,z)==16 && events.empty());
    start();for(auto &p:v)p.y=2;
    require(polygon_gouraud(&buffer,v,z)==0);
    final=before;final[PIXELS]=reinterpret_cast<intptr_t>(origin);final[BOTTOM]=0;
    final[VERTICES]=reinterpret_cast<intptr_t>(v);final[LIGHTS]=reinterpret_cast<intptr_t>(z);
    final[PITCH]=40;final[LEFT]=0;final[RIGHT]=12;final[TOP]=0;final[CLIP_BOTTOM]=12;
    require(events=="GRCFRC" && released==final && snapshot(false)==final);
    require(std::memcmp(pixels,expected,sizeof(pixels))==0);

    // Actual 15-DWORD helper leaves its four UV slots untouched.
    start();Vert edge_vertices[]={{2,1},{6,1},{1,5},{0,3}};
    int edge_lights[]={10,6,-2,-6};PolygonVertices=edge_vertices;PolygonLights=edge_lights;
    PolygonBottomVertex=2;
    EdgeScan edge={1,91,92,101,102,103,104,93,94,95,96,97,98,99,100};
    require(setup_gouraud_polygon_edge(&edge,0)==1);
    const int wanted[]={1,4,2,101,102,103,104,6,-1,-1,-3,1,4,393216,-131072};
    static_assert(sizeof(edge)==15*sizeof(int),"real EdgeScan shape");
    require(std::memcmp(&edge,wanted,sizeof(edge))==0);
    EdgeScan old=edge;require(setup_gouraud_polygon_edge(&edge,2)==0);
    require(std::memcmp(&edge,&old,sizeof(edge))==0);
    edge.f0=-1;require(setup_gouraud_polygon_edge(&edge,0)==1);
    require(edge.accumD==655360 && edge.deltaD==-524288 && edge.counter==2);
    return 0;
}
'''


def harness(source, fields, vertex):
    slopes = set()
    for width in (1, 2, 3, 7, 31, 255, 32767):
        for delta in (0, 1, -1, 196608, -196608, 2147483647, -2147483648):
            slopes.add((delta, width))
        for boundary in (-32769, -32768, -1, 0, 1, 32767, 32768):
            for offset in (-1, 0, 1):
                delta = boundary * width * 256 + offset
                if -2147483648 <= delta <= 2147483647:
                    slopes.add((delta, width))
    rng = random.Random(626)
    for _ in range(200):
        slopes.add((rng.randint(-2147483648, 2147483647), rng.randint(1, 32767)))
    slope_cpp = ",".join("{%d,%d,%d}" % (delta, width, direct_word_step(delta, width))
                         for delta, width in sorted(slopes))
    return (HARNESS.replace("ACTUAL_FIELDS", fields).replace("ACTUAL_VERTEX", vertex)
            .replace("ACTUAL_SOURCE", source).replace("SLOPE_CASES", slope_cpp)
            .replace("RASTER_CASES", ",".join(case_cpp(case) for case in generated_cases()))
            .replace("TRACE_CASE", case_cpp(([(2, 0), (7, 2), (4, 6), (0, 3)],
                                              [0, 4, 12, 6], (-4, -4, 16, 12), 40))))


def run_host(tmp_path, source, fields, vertex):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    cpp = tmp_path / "gouraud.cpp"
    cpp.write_text(harness(source, fields, vertex))
    binary = tmp_path / "gouraud"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-finstrument-functions", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_gouraud_polygon(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("static_cast<int64_t>(delta) * 256 / width_fixed", "static_cast<int64_t>(delta) * 255 / width_fixed"),
    ("step < -32768", "step < -65536"),
    ("step > 32767", "step > 65535"),
    ("return 0x8000;", "return 0;"),
    ("static_cast<uint64_t>(PolygonLightStep) * skip",
     "static_cast<int64_t>(static_cast<short>(PolygonLightStep)) * skip"),
    ("static_cast<unsigned int>(product >> 8)", "static_cast<unsigned int>(product << 8)"),
    ("--remaining >= 0", "--remaining > 0"),
    ("PolygonLight = backward.accumD;", "PolygonLight = forward.accumD;"),
    ("PolygonSpanRight = forward.accumC;", "PolygonSpanRight = forward.accumC - 1;"),
    ("if (pending_width > 0) {\n                                PolygonLightStep",
     "if (done) break;\n                            if (pending_width > 0) {\n                                PolygonLightStep"),
    ("PolygonY.address += PolygonPitch;", "PolygonY.address += 2 * PolygonPitch;"),
    ("edge->fracAccum = 1 - edge->counter;", "edge->fracAccum = 0;"),
    ("backward.fracAccum > 0", "backward.fracAccum >= 0"),
    ("static_cast<unsigned int>(backward.accumD) + backward.deltaD",
     "static_cast<unsigned int>(backward.accumD)"),
    ("_PC_24 | _RC_CHOP", "_PC_64 | _RC_CHOP"),
    ("polygon_reset_fpu();", ";"),
    ("buffer->free_data(1);\n    polygon_reset_fpu();",
     "polygon_reset_fpu();\n    buffer->free_data(1);"),
    ("((int64_t)numerator * 65536)", "((int64_t)numerator * 65535)"),
])
def test_gouraud_polygon_damage_is_observed(tmp_path, before, after):
    source, fields, vertex = actual_source()
    assert before in source
    result = run_host(tmp_path, source.replace(before, after), fields, vertex)
    assert result.returncode == 89, result.stderr
