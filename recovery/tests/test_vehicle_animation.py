"""Bounded animation-body controls, not a complete renderer/game-runtime proof.

Product bodies/helpers and record declarations are extracted unchanged. Map,
geometry and renderer dependencies are explicit doubles. Host UBSan exercises
array ownership and sequencing; it does not establish VC6 layout/x87 identity.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def uncomment(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if not depth:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source")


def product():
    source = uncomment((ROOT / "src/veh.cpp").read_text())
    names = ("vehdraw_scale", "vehdraw_centre_offset", "vehdraw_uses_terrain",
             "vehdraw_facet_index", "vehdraw_average_facets", "vehdraw_finish_frame",
             "vehdraw_turn", "vehdraw_scoot_start", "vehdraw_scoot", "vehdraw_draw")
    bodies = []
    for name in names:
        match, = re.finditer(r"^(?:static )?(?:float|int|bool|void)\s+(?:__cdecl\s+)?"
                             + name + r"\([^;{]*\)\s*\{", source, re.M)
        bodies.append(braced(source, match.start()))
    for name in ("VehMoveDeltaX", "VehMoveDeltaY"):
        declaration, = re.findall(r"int " + name + r"\[\d+\]\s*=\s*\{[^}]*\};", source)
        bodies.insert(0, declaration)
    return "\n".join(bodies)


def declarations():
    result = []
    for file, names in (("veh.h", ("Veh", "VehPrototype", "RulesWeapon")),
                        ("map.h", ("Map",)), ("caviar.h", ("VOX_Vect", "VOX_Matrix"))):
        header = uncomment((ROOT / "src" / file).read_text())
        for name in names:
            result.append(braced(header, header.index("struct " + name + " {")) + ";")
    map_header = (ROOT / "src/map.h").read_text()
    result.append("extern Map *MapTiles;")
    result.append(braced(map_header, map_header.index("inline Map *&map_tiles()")))
    return "\n".join(result)


def test_pinned_animation_control_anchors():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def decode(start, end):
        return list(decoder.disasm(image.read(Span(start, end)), start))

    calls = lambda start, end: [int(i.op_str, 16) for i in decode(start, end)
                               if i.mnemonic == "call" and i.op_str.startswith("0x")]
    assert calls(0x4BFC70, 0x4BFDFA) == [0x6183B0, 0x618370, 0x4C1D20,
        0x617230, 0x4C3090, 0x6171A0, 0x618470]
    assert calls(0x4C0F50, 0x4C1296) == [0x6281B0, 0x628150, 0x628220,
        0x4BF740, 0x627F50, 0x6280E0, 0x618430, 0x6183B0, 0x6182A0,
        0x6183B0, 0x618370, 0x4C1D20, 0x617230, 0x4C3090, 0x6171A0, 0x618470]
    assert calls(0x4C14EF, 0x4C1732) == [0x4BF740, 0x627F50, 0x6280E0,
        0x618430, 0x6183B0, 0x6182A0, 0x4C1D20, 0x618370,
        0x617230, 0x4C3090, 0x6171A0, 0x618470]
    import struct
    assert struct.unpack("<9i", image.read(Span(0x66EF50, 0x66EF74))) == (1,2,1,0,-1,-2,-1,0,0)
    assert struct.unpack("<9i", image.read(Span(0x66EF74, 0x66EF98))) == (-1,0,1,2,1,0,-1,-2,0)
    assert struct.unpack("<8I", image.read(Span(0x4C0F24, 0x4C0F44))) == (
        0x4C02C1,0x4C0384,0x4C05CC,0x4C068F,0x4C0845,0x4C0908,0x4C0B4C,0x4C0C0F)
    boundary = decode(0x4C0F75, 0x4C0F8E)
    assert boundary[0].mnemonic == "cmp" and boundary[-1].mnemonic == "jle"
    loop = decode(0x4BFEE8, 0x4BFEF7)
    assert [(i.mnemonic, i.op_str) for i in loop] == [
        ("add", "ecx, 4"), ("cmp", "ecx, 0x24"),
        ("mov", "dword ptr [ebp + 0x14], ecx"), ("jl", "0x4bfe48")]


HARNESS = r"""
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
void check_at(bool ok,int line) {
    if (!ok) { std::fprintf(stderr,"behavior assertion line %d\n",line); std::exit(91); }
}
#define check(x) check_at((x),__LINE__)
bool near(float a,float b) { return std::fabs(a-b)<0.0001f; }
// DECLARATIONS
void same(const float *a,const VOX_Vect &b) {
    for(int i=0;i<3;++i) check(near(a[i],b.values[i]));
}
VOX_Vect plus(VOX_Vect a,const VOX_Vect &b) {
    for(int i=0;i<3;++i) a.values[i]+=b.values[i]; return a;
}
VOX_Vect scaled(VOX_Vect a,float f) {
    for(int i=0;i<3;++i) a.values[i]*=f; return a;
}
VOX_Vect unit(VOX_Vect a) {
    const float n=std::sqrt(a.values[0]*a.values[0]+a.values[1]*a.values[1]+a.values[2]*a.values[2]);
    for(int i=0;i<3;++i) a.values[i]/=n; return a;
}
Veh Vehs[8]; VehPrototype VehPrototypes[8]; RulesWeapon Weapon[8];
Map maps[2][256]; Map *MapTiles; uint32_t MapLongitude;
int VehLiftX,VehLiftY,MapLongitudeBounds,MapIsFlat;
int VehDrawTurnVehicle,VehDrawAnimationStep,VehDrawScootFacing,VehDrawScootSteps;
float VehDrawTurnAngle,VehDrawTurnIncrement;
VOX_Vect VehLightDirMap,VehDrawCameraAngles,VehDrawScootNormalStart,VehDrawScootNormalMid;
VOX_Vect VehDrawScootFirstIncrement,VehDrawScootSecondIncrement;
VOX_Vect VehDrawFacetNormals0[2401],VehDrawFacetNormals1[2401];
VOX_Vect VehDrawFacetNormals2[2401],VehDrawFacetNormals3[2401];
VOX_Vect *tables[]={VehDrawFacetNormals0,VehDrawFacetNormals1,VehDrawFacetNormals2,VehDrawFacetNormals3};
int VehDrawPositionX,VehDrawPositionY,VehDrawPositionVehicle;
int MapWinCaptionScale,MapWinCaptionDivisor;
using ColorTable = unsigned char[256];
unsigned char ShadowColorTable[256],ArmorTransparentColorTables[24][256];
struct Buffer { int tag; } destination;
struct Console { int get_relative_alt(int,int,int); } ConsoleGlobal;
struct Caviar {
    VOX_Vect light_vector_,shadow_vector_; int field_13CC_;
    void reset_objects() { field_13CC_=0; }
    void set_scaling(float); void set_scene_rotation(float,float,float);
    void set_shadow_table(unsigned char *);
    void set_camera_direct(const VOX_Vect *,const VOX_Matrix *);
    void render(Buffer *,int,int,int);
    void render_multi_table_dest(Buffer *,int,int,int,ColorTable *,int);
    void clear_buffers(int);
} g_VEHDRAW_CAVIAR;
int scenario,chosen,frame_arg,zoom_arg,flags_arg,armor_case,scale_calls,final_scale;
int alt_calls,normal_calls,add_calls,scalar_calls,sub_calls,direction_arg;
bool sampled_slope,mutate_after_end,shift_lift;
std::string events;
VOX_Vect expected_normal,expected_start,expected_end,expected_mid;
VOX_Vect expected_increment,expected_base;
VOX_Matrix *camera_seen; float (*inverse_seen)[3];
VOX_Vect saved[4][9]; int heights[4][9],indices[9];
std::vector<VOX_Vect> mid_inputs;
float *normal_output;
int xrange(int x) {
    if (!(MapIsFlat&1)) {
        if(x<0) x+=MapLongitudeBounds;
        else if(x>=MapLongitudeBounds) x-=MapLongitudeBounds;
    }
    return x;
}
int pick_top_veh(int id) {
    events+='P'; check(id==1); VehLightDirMap=VOX_Vect{{91,92,93}};
    return chosen;
}
int Console::get_relative_alt(int x,int y,int corner) {
    check(this==&ConsoleGlobal); events+='Q';
    if(scenario==3) {
        static const int dx[]={1,2,1,0,-1,-2,-1,0,0};
        static const int dy[]={-1,0,1,2,1,0,-1,-2,0};
        int cell=alt_calls/4; check(cell<9 && corner==alt_calls%4+1);
        const int base_x=(shift_lift && cell>0)?15:0;
        const int base_y=(shift_lift && cell>0)?5:4;
        int ex=base_x+dx[cell];
        if(!(MapIsFlat&1)) { if(ex<0) ex+=16; else if(ex>=16) ex-=16; }
        check(x==ex && y==base_y+dy[cell]);
        if(shift_lift && alt_calls==0) { VehLiftX=15; VehLiftY=5; }
        ++alt_calls; return heights[corner-1][cell];
    }
    check(scenario==2 && x==6 && y==4 && corner==++alt_calls);
    return heights[corner-1][8];
}
void vector_add(const float *a,const float *b,float *out) {
    events+='a';
    if(scenario==1) {
        check(add_calls==0 && a!=out && b!=out);
        same(a,expected_increment); same(b,expected_base);
    } else if(scenario==3) {
        const std::vector<VOX_Vect> *inputs=0;
        std::vector<VOX_Vect> cell_inputs;
        if(normal_calls<2) {
            for(int t=0;t<4;++t) cell_inputs.push_back(saved[t][normal_calls==0?8:direction_arg]);
            inputs=&cell_inputs;
        } else inputs=&mid_inputs;
        check(add_calls<int(inputs->size())-1);
        if(add_calls==0) {
            same(a,(*inputs)[0]); normal_output=out; check(a!=out && b!=out);
            // Every midpoint/cell facet must already be a value snapshot.
            for(int t=0;t<4;++t) for(int i=0;i<2401;++i)
                tables[t][i]=VOX_Vect{{-100,-200,-300}};
        } else check(a==normal_output && out==normal_output);
        same(b,(*inputs)[add_calls+1]);
    }
    for(int i=0;i<3;++i) out[i]=a[i]+b[i];
    ++add_calls;
}
void vox_normalise_vector(const float *in,float *out) {
    events+='n'; check(in==out);
    if(scenario==3) {
        if(sampled_slope) {
            const VOX_Vect want=normal_calls==0?expected_start:(normal_calls==1?expected_end:expected_mid);
            same(in,want);
        } else same(in,VOX_Vect{{0,0,1}});
        // Restore the global tables for the next phase, after proving each
        // preceding phase copied every input before its first add callback.
        for(int t=0;t<4;++t) for(int c=0;c<9;++c) tables[t][indices[c]]=saved[t][c];
        if(mutate_after_end && normal_calls==1) {
            Weapon[2].mode=2; expected_mid=VOX_Vect{{0,0,1}};
        }
        add_calls=0;
    } else if(scenario==1) same(in,expected_normal);
    VOX_Vect v={{in[0],in[1],in[2]}}; v=unit(v);
    for(int i=0;i<3;++i) out[i]=v.values[i];
    ++normal_calls;
}
void leaf_00628180_redirect(const float *a,const float *b,float *out) {
    events+='u'; check(scenario==3);
    same(a,unit(sub_calls==0?expected_mid:expected_end));
    same(b,unit(sub_calls==0?expected_start:expected_mid));
    check(out==(sub_calls==0?VehDrawScootFirstIncrement.values:VehDrawScootSecondIncrement.values));
    for(int i=0;i<3;++i) out[i]=a[i]-b[i]; ++sub_calls;
}
void leaf_006281b0_redirect(const float *in,float factor,float *out) {
    events+='f'; check(in==out);
    if(scenario==1) {
        check(scalar_calls==0); same(in,expected_increment);
        int frame=frame_arg; if(frame>VehDrawScootSteps/2) frame-=VehDrawScootSteps/2;
        check(factor==float(frame)); expected_increment=scaled(expected_increment,factor);
        expected_normal=plus(expected_increment,expected_base);
    } else {
        check(scenario==3 && scalar_calls<2);
        const int half=VehDrawScootSteps>>1;
        const int count=scalar_calls==0?half:VehDrawScootSteps-half;
        check(factor==1.0f/count);
        check(out==(scalar_calls==0?VehDrawScootFirstIncrement.values:VehDrawScootSecondIncrement.values));
    }
    for(int i=0;i<3;++i) out[i]=in[i]*factor; ++scalar_calls;
}
void compute_camera(VOX_Vect *normal,VOX_Matrix *matrix) {
    events+='C'; same(normal->values,unit(expected_normal)); camera_seen=matrix;
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) matrix->values[r][c]=float(r*3+c+1);
    VehLightDirMap=VOX_Vect{{81,82,83}}; VehDrawCameraAngles.values[0]=0.25f;
}
void vox_matrix_inverse(const float (*matrix)[3],float (*out)[3]) {
    events+='I'; check(matrix==camera_seen->values && matrix!=out); inverse_seen=out;
    same(g_VEHDRAW_CAVIAR.light_vector_.values,VOX_Vect{{11,12,13}});
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) out[r][c]=float(101+r*3+c);
}
void vox_matrix_vector_mul(const float (*matrix)[3],const float *v,float *out) {
    events+='M'; check(matrix==inverse_seen && matrix[2][2]==109);
    same(v,VOX_Vect{{-0.57735f,0.57735f,0.57735f}});
    out[0]=31; out[1]=32; out[2]=33;
}
void Caviar::set_shadow_table(unsigned char *table) {
    events+='H'; check(this==&g_VEHDRAW_CAVIAR && table==ShadowColorTable);
    same(shadow_vector_.values,VOX_Vect{{31,32,33}});
}
void Caviar::set_scaling(float value) {
    events+='S'; check(this==&g_VEHDRAW_CAVIAR);
    check(near(value,scale_calls==0?0.0525f:0.14f)); ++scale_calls;
    MapWinCaptionScale=60; MapWinCaptionDivisor=20;
    VehDrawTurnVehicle=2; VehDrawTurnAngle=4; VehDrawTurnIncrement=2;
    VehDrawCameraAngles.values[0]=0.75f; VehDrawScootFacing=5;
}
void Caviar::set_camera_direct(const VOX_Vect *position,const VOX_Matrix *matrix) {
    events+='D'; check(this==&g_VEHDRAW_CAVIAR && matrix==camera_seen);
    same(position->values,VOX_Vect{{-70000,-80000,-90000}});
    MapWinCaptionScale=80; MapWinCaptionDivisor=40;
}
void Caviar::set_scene_rotation(float a,float b,float c) {
    events+='R'; check(this==&g_VEHDRAW_CAVIAR && b==0 && c==0);
    float expected=scenario==0?frame_arg*2.0f+4.0f-0.75f:
        (scenario==1?5*0.785375f-0.75f:5*0.785375f-0.5f);
    check(near(a,expected)); VehDrawTurnVehicle=3;
    MapWinCaptionScale=final_scale; MapWinCaptionDivisor=2;
}
void vehdraw_construct_vehicle(int id,Caviar *renderer) {
    events+='V'; check(renderer==&g_VEHDRAW_CAVIAR && id==chosen);
    check(renderer->field_13CC_==(scenario==0?0:77));
    renderer->field_13CC_=66; VehDrawTurnVehicle=0;
    if(scenario==1) VehDrawAnimationStep=40;
    if(scenario==2) { VehDrawCameraAngles.values[0]=0.5f; Vehs[id].facing=6; }
}
void Caviar::render(Buffer *buffer,int x,int y,int flags) {
    events+='B'; check(this==&g_VEHDRAW_CAVIAR && buffer==&destination && field_13CC_==66);
    const int offset=final_scale?75:-9;
    check(x==4+offset && y==7+offset);
    check(flags==(scenario==2?((flags_arg>>12)&2)|1:1));
    if(scenario==1) check(VehDrawAnimationStep==41);
    Vehs[chosen].proto_id=4; field_13CC_=77;
}
void vehdraw_construct_armor(int faction,int chassis,int weapon,int armor,int transparent,Caviar *renderer) {
    events+='A'; check(renderer==&g_VEHDRAW_CAVIAR && renderer->field_13CC_==0);
    check(faction==6 && chassis==7 && weapon==3 && armor==5 && transparent==1);
}
void Caviar::render_multi_table_dest(Buffer *buffer,int x,int y,int count,ColorTable *table,int flags) {
    events+='T'; const int offset=final_scale?75:-9;
    check(this==&g_VEHDRAW_CAVIAR && buffer==&destination && x==4+offset && y==7+offset);
    check(count==24 && table==ArmorTransparentColorTables);
    check(flags==(scenario==2?(flags_arg>>12)&2:0));
}
void Caviar::clear_buffers(int fill) { events+='X'; check(this==&g_VEHDRAW_CAVIAR && fill==0); }
// PRODUCT
void reset() {
    events.clear(); alt_calls=normal_calls=add_calls=scalar_calls=sub_calls=scale_calls=0;
    std::memset(Vehs,0,sizeof(Vehs)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Weapon,0,sizeof(Weapon)); std::memset(maps,0,sizeof(maps));
    for(int i=0;i<8;++i) { Vehs[i].proto_id=1; Vehs[i].faction_id=6; }
    VehPrototypes[1].chassis_id=0; VehPrototypes[1].weapon_id=2; Weapon[2].mode=255;
    Weapon[2].offense_rating=-1;
    VehPrototypes[4].chassis_id=7; VehPrototypes[4].weapon_id=3;
    VehPrototypes[4].armor_id=armor_case;
    VehDrawTurnVehicle=1; VehDrawAnimationStep=5;
    VehDrawTurnAngle=91; VehDrawTurnIncrement=92;
    MapWinCaptionScale=30; MapWinCaptionDivisor=40;
    VehDrawCameraAngles=VOX_Vect{{90,0,0}}; VehLightDirMap=VOX_Vect{{11,12,13}};
    g_VEHDRAW_CAVIAR.field_13CC_=77;
    MapLongitude=8; MapLongitudeBounds=16; MapIsFlat=0; VehLiftX=0; VehLiftY=4;
    MapTiles=maps[1];
    VehDrawPositionX=6; VehDrawPositionY=4; VehDrawPositionVehicle=2;
    VehDrawScootNormalStart=VOX_Vect{{10,20,30}}; VehDrawScootNormalMid=VOX_Vect{{100,200,300}};
    VehDrawScootFirstIncrement=VOX_Vect{{1,2,4}}; VehDrawScootSecondIncrement=VOX_Vect{{4,5,6}};
    VehDrawScootSteps=5;
    for(int c=0;c<9;++c) {
        heights[0][c]=c%7+1; heights[1][c]=(c/7)+1; heights[2][c]=2; heights[3][c]=3;
        indices[c]=(heights[0][c]-1)*343+(heights[1][c]-1)*49+7+2;
        for(int t=0;t<4;++t) {
            saved[t][c]=VOX_Vect{{float(1+t*9+c),float(20+t*3-c),float(7+t+c)}};
            tables[t][indices[c]]=saved[t][c];
        }
    }
}
VOX_Vect cell_sum(int cell) {
    VOX_Vect out=saved[0][cell]; for(int t=1;t<4;++t) out=plus(out,saved[t][cell]); return out;
}
void run_frame(int which,int frame,int armor,int scale,int flags,bool slope) {
    scenario=which; frame_arg=frame; zoom_arg=-19; flags_arg=flags; armor_case=armor; final_scale=scale;
    reset(); chosen=which==0?1:(which==1?3:2);
    expected_normal=VOX_Vect{{0,0,1}};
    if(which==0) vehdraw_turn(&destination,4,7,frame,-19);
    else if(which==1) {
        expected_base=frame>2?VehDrawScootNormalMid:VehDrawScootNormalStart;
        expected_increment=frame>2?VehDrawScootSecondIncrement:VehDrawScootFirstIncrement;
        vehdraw_scoot(&destination,4,7,frame,-19);
    } else {
        Vehs[2].facing=-1; Vehs[2].x=2; Vehs[2].y=2;
        maps[1][35].climate=0x60;
        if(slope) expected_normal=cell_sum(8);
        else Weapon[2].mode=2;
        vehdraw_draw(&destination,1,4,7,flags,-19,1);
    }
    std::string expected=which==0?"SRVB":(which==1?"fanCIMHSDSRVB":
        (slope?"PQQQQaaanCIMHSDVRB":"PCIMHSDVRB"));
    expected+=armor==5?"AT":"X";
    if(events!=expected) std::fprintf(stderr,"events %s expected %s\n",events.c_str(),expected.c_str());
    check(events==expected);
}
void run_start(int direction,bool slope,int mode,bool flat,bool move,bool mutate) {
    scenario=3; direction_arg=direction; armor_case=4; reset(); chosen=2;
    sampled_slope=slope && mode>=3; mutate_after_end=mutate; shift_lift=move;
    MapIsFlat=flat; Weapon[2].mode=uint8_t(mode); VehPrototypes[1].chassis_id=0;
    for(int i=0;i<256;++i) maps[1][i].climate=slope?0x60:0x5F;
    expected_start=sampled_slope?cell_sum(8):VOX_Vect{{0,0,1}};
    expected_end=sampled_slope?cell_sum(direction):VOX_Vect{{0,0,1}};
    mid_inputs.clear();
    if(!(direction&1)) {
        mid_inputs.push_back(saved[3][direction]); mid_inputs.push_back(saved[1][8]);
    } else {
        // Explicit original corner/facet order, independently of the product switch.
        static const int cells[4][8]={{8,8,0,0,1,1,2,2},{8,8,2,2,3,3,4,4},
                                     {8,8,4,4,5,5,6,6},{8,8,6,6,7,7,0,0}};
        static const int facets[4][8]={{1,2,2,3,3,0,0,1},{2,3,3,0,0,1,1,2},
                                      {3,0,0,1,1,2,2,3},{0,1,1,2,2,3,3,0}};
        for(int i=0;i<8;++i) mid_inputs.push_back(saved[facets[direction/2][i]][cells[direction/2][i]]);
    }
    expected_mid=VOX_Vect{{0,0,1}};
    if(sampled_slope) {
        expected_mid=mid_inputs[0]; for(unsigned i=1;i<mid_inputs.size();++i) expected_mid=plus(expected_mid,mid_inputs[i]);
    }
    vehdraw_scoot_start(1,direction,5,1);
    std::string expected="P"+std::string(36,'Q');
    if(sampled_slope) expected+="aaanaaan";
    if(sampled_slope && !mutate) expected+=std::string((direction&1)?7:1,'a');
    expected+="nuuff";
    if(events!=expected) std::fprintf(stderr,"start events %s expected %s\n",events.c_str(),expected.c_str());
    check(events==expected && alt_calls==36 && sub_calls==2 && scalar_calls==2);
    check(VehDrawTurnVehicle==chosen && VehDrawScootSteps==5 && VehDrawScootFacing==((direction-3)&7));
    check(VehDrawAnimationStep==1);
    same(VehDrawScootNormalStart.values,unit(expected_start));
    same(VehDrawScootNormalMid.values,unit(expected_mid));
    VOX_Vect first=plus(unit(expected_mid),scaled(unit(expected_start),-1));
    VOX_Vect second=plus(unit(expected_end),scaled(unit(expected_mid),-1));
    same(VehDrawScootFirstIncrement.values,scaled(first,0.5f));
    same(VehDrawScootSecondIncrement.values,scaled(second,1.0f/3));
}
int main() {
    scenario=2; armor_case=5; reset(); vehdraw_draw(0,1,4,7,1,-19,1); check(events.empty());
    for(int armor=4;armor<=5;++armor) for(int scale=0;scale<=3;scale+=3) {
        run_frame(0,3,armor,scale,0,false);
        for(int f=-1;f<=5;++f) run_frame(1,f,armor,scale,0,false);
        run_frame(2,0,armor,scale,1,false); run_frame(2,0,armor,scale,0x2001,true);
    }
    for(int direction=0;direction<8;++direction) {
        run_start(direction,true,255,false,false,false);
        run_start(direction,true,2,false,false,false);
        run_start(direction,false,255,false,false,false);
    }
    run_start(3,true,3,false,true,false);
    run_start(7,true,3,true,false,false);
    run_start(1,true,3,false,false,true);
}
"""


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source, executable = tmp_path / "animation.cpp", tmp_path / "animation"
    source.write_text(HARNESS.replace("// DECLARATIONS", declarations()).replace("// PRODUCT", body))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
        "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
        text=True, capture_output=True, timeout=45)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True, timeout=15)


def test_actual_animation_bodies(tmp_path):
    result = run_host(tmp_path, product())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("damage", ["eight_samples", "wrong_midpoint", "half_boundary",
    "stale_scale", "early_scoot_selection", "stale_prototype", "wrong_overlay_flags", "unsigned_mode"])
def test_animation_damage(tmp_path, damage):
    body = product()
    replacements = {
        # Omit the first of nine samples so the explicit call-order assertion
        # fails before an unread centre local could trigger unrelated UB.
        "eight_samples": ("for (int cell = 0; cell < 9; ++cell)", "for (int cell = 1; cell < 9; ++cell)"),
        "wrong_midpoint": ("n[4] = VehDrawFacetNormals3[b]", "n[4] = VehDrawFacetNormals2[b]"),
        "half_boundary": ("if (frame > half)", "if (frame >= half)"),
        "stale_scale": ("g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));\n    g_VEHDRAW_CAVIAR.set_scene_rotation(\n        VehDrawScootFacing",
                        "g_VEHDRAW_CAVIAR.set_scaling(0.0525f);\n    g_VEHDRAW_CAVIAR.set_scene_rotation(\n        VehDrawScootFacing"),
        "stale_prototype": ("VehPrototypes[Vehs[veh_id].proto_id];\n    if (prototype.armor_id == 5)",
                            "VehPrototypes[1];\n    if (prototype.armor_id == 5)"),
        "wrong_overlay_flags": ("vehdraw_finish_frame(buffer, veh_id, x, y, (flags >> 12) & 2)",
                                "vehdraw_finish_frame(buffer, veh_id, x, y, 0)"),
        "unsigned_mode": ("Weapon[prototype.weapon_id].mode >= 3", "(int8_t)Weapon[prototype.weapon_id].mode >= 3"),
    }
    if damage == "early_scoot_selection":
        old = "void __cdecl vehdraw_scoot(Buffer *buffer, int x, int y, int frame, int zoom) {"
        body = body.replace(old, old + "\n    const int cached_vehicle = VehDrawTurnVehicle;", 1)
        old = "const int veh_id = VehDrawTurnVehicle;\n    vehdraw_construct_vehicle"
        assert body.count(old) == 1
        body = body.replace(old, "const int veh_id = cached_vehicle;\n    vehdraw_construct_vehicle", 1)
    else:
        old, new = replacements[damage]
        assert body.count(old) == 1, damage
        body = body.replace(old, new)
    result = run_host(tmp_path, body)
    # Require an explicit behavioral failure, not compilation, UBSan or a signal.
    assert result.returncode == 91, result.stdout + result.stderr
