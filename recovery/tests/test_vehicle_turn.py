"""Bounded actual turn-setup controls; continuation remains outside this test.

Record declarations are extracted from product headers. Renderer/map callbacks
are doubles, not game-runtime or VC6 layout proof. Angle cases use bounded
finite inputs and speed 13, avoiding integer-conversion threshold ambiguity;
zero distance deliberately preserves the original zero steps/NaN increment.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def uncomment(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated product declaration/body")


def live_body():
    source = uncomment((ROOT / "src/veh.cpp").read_text())
    match, = re.finditer(r"int\s+(?:__cdecl\s+)?vehdraw_turn_start\([^;{]*\)\s*\{", source)
    return braced(source,match.start())


def declarations():
    records = []
    for file, names in (("veh.h", ("Veh","VehPrototype","RulesWeapon")),
                        ("map.h", ("Map",)), ("caviar.h", ("VOX_Vect","VOX_Matrix"))):
        header = uncomment((ROOT / "src" / file).read_text())
        records += [braced(header,header.index("struct " + name + " {")) + ";" for name in names]
    map_header = (ROOT / "src/map.h").read_text()
    records.append("extern Map *MapTiles;")
    records.append(braced(map_header, map_header.index("inline Map *&map_tiles()")))
    return "\n".join(records)


def test_original_weapon_mode_operand():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required for independent operand evidence")
    original = PeImage(path.read_bytes(),image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86,CS_MODE_32); decoder.detail = True
    compare, branch = list(decoder.disasm(original.read(Span(0x4BF9A7,0x4BF9B4)),0x4BF9A7))
    assert compare.mnemonic == "cmp" and branch.mnemonic == "jb"
    memory, threshold = compare.operands
    assert memory.type == X86_OP_MEM and memory.size == 1 and memory.mem.disp == 0x94AE60 + 0xA
    assert threshold.type == X86_OP_IMM and threshold.imm == 3
    # Two original Win32 pointer slots precede +8 offense, +9 icon, +A mode.
    # This checks the actual declaration, without asserting host pointer size.
    header = uncomment((ROOT / "src/veh.h").read_text())
    weapon = braced(header,header.index("struct RulesWeapon {"))
    assert re.search(r"LPSTR\s+name;\s*LPSTR\s+name_short;\s*int8_t\s+offense_rating;"
                     r"\s*int8_t\s+icon;\s*uint8_t\s+mode;",weapon)


def test_turn_start_native_identity():
    record, = (r for r in read_file(ROOT / "src/veh.cpp") if r.address == 0x4BF8D0)
    assert record.image_spans == ((0x4BF8D0,0x4BFC67),)


HARNESS = r"""
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
void check_at(bool ok,int line) {
    if (!ok) { std::fprintf(stderr,"behavior assertion at generated line %d\n",line); std::exit(91); }
}
#define check(ok) check_at((ok),__LINE__)
bool near(float a,float b) { return std::fabs(a-b) < 0.000002f; }
// DECLARATIONS
void vec(const float *v,float x,float y,float z) {
    check(near(v[0],x) && near(v[1],y) && near(v[2],z));
}
Veh Vehs[4];
VehPrototype VehPrototypes[4];
RulesWeapon Weapon[4];
Map terrain[2][64];
Map *MapTiles;
uint32_t MapLongitude;
int VehLiftX,VehLiftY;
VOX_Vect VehLightDirMap, VehDrawCameraAngles;
VOX_Vect VehDrawFacetNormals0[2401],VehDrawFacetNormals1[2401];
VOX_Vect VehDrawFacetNormals2[2401],VehDrawFacetNormals3[2401];
VOX_Vect *tables[] = {VehDrawFacetNormals0,VehDrawFacetNormals1,VehDrawFacetNormals2,VehDrawFacetNormals3};
int VehDrawTurnVehicle;
float VehDrawTurnAngle,VehDrawTurnIncrement;
unsigned char ShadowColorTable[256];
struct Console { int get_relative_alt(int,int,int); } ConsoleGlobal;
struct Caviar {
    VOX_Vect light_vector_,shadow_vector_;
    int set_shadow_table(unsigned char *);
    void set_camera_direct(const VOX_Vect *,const VOX_Matrix *);
} g_VEHDRAW_CAVIAR;
int chosen,old_facing,new_facing,corners[4],table_index,adds,phase;
bool sloped;
std::string sequence;
VOX_Matrix *camera_matrix;
float (*inverse_matrix)[3];
float *sum_output;
float saved_normals[4][3];
void event(char c) {
    check(phase < int(sequence.size()) && sequence[phase++] == c);
    if (c != 'P') {
        check(VehDrawTurnVehicle == chosen);
        check(Vehs[chosen].facing == int8_t(new_facing));
    }
}
int pick_top_veh(int id) {
    event('P'); check(id == 1 && chosen == 2 && VehDrawTurnVehicle == -17);
    VehLightDirMap = VOX_Vect{{91,92,93}};
    Vehs[chosen].facing = int8_t(old_facing);
    VehLiftX = 6; VehLiftY = 2;
    return chosen;
}
int Console::get_relative_alt(int x,int y,int detail) {
    check(this == &ConsoleGlobal && x == 6 && y == 2 && detail >= 1 && detail <= 4);
    event(char('0'+detail)); return corners[detail-1];
}
void vector_add(const float *left,const float *right,float *out) {
    event('A'); check(adds < 3);
    if (adds == 0) {
        vec(left,saved_normals[0][0],saved_normals[0][1],saved_normals[0][2]);
        check(out != left && out != right); sum_output = out;
    } else check(left == sum_output && out == sum_output);
    vec(right,saved_normals[adds+1][0],saved_normals[adds+1][1],saved_normals[adds+1][2]);
    for (int t=0; t<4; ++t) check(left != tables[t][table_index].values && right != tables[t][table_index].values);
    for (int c=0; c<3; ++c) out[c] = left[c]+right[c];
    // All four source vectors were copied before the first callback.
    if (adds++ == 0) for (int t=0; t<4; ++t) tables[t][table_index] = VOX_Vect{{-101,-102,-103}};
}
void vox_normalise_vector(const float *in,float *out) {
    event('N'); check(in == sum_output && out == sum_output && adds == 3);
    float total[3] = {};
    for (int t=0; t<4; ++t) for (int c=0; c<3; ++c) total[c] += saved_normals[t][c];
    vec(in,total[0],total[1],total[2]);
    const float length = std::sqrt(total[0]*total[0]+total[1]*total[1]+total[2]*total[2]);
    for (int c=0; c<3; ++c) out[c] = total[c]/length;
}
void compute_camera(VOX_Vect *normal,VOX_Matrix *matrix) {
    event('C');
    float expected[3] = {0,0,1};
    if (sloped) {
        for (int c=0; c<3; ++c) { expected[c]=0; for (int t=0; t<4; ++t) expected[c]+=saved_normals[t][c]; }
        const float length = std::sqrt(expected[0]*expected[0]+expected[1]*expected[1]+expected[2]*expected[2]);
        for (int c=0; c<3; ++c) expected[c] /= length;
    }
    vec(normal->values,expected[0],expected[1],expected[2]);
    camera_matrix = matrix;
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) matrix->values[r][c] = float(r*3+c+1);
    VehLightDirMap = VOX_Vect{{81,82,83}};
}
void vox_matrix_inverse(const float (*matrix)[3],float (*out)[3]) {
    event('I'); check(matrix == camera_matrix->values && matrix != out);
    vec(g_VEHDRAW_CAVIAR.light_vector_.values,11,12,13);
    inverse_matrix = out;
    for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) out[r][c] = float(101+r*3+c);
}
void vox_matrix_vector_mul(const float (*matrix)[3],const float *input,float *out) {
    event('M'); check(matrix == inverse_matrix && matrix[2][2] == 109);
    vec(input,-0.57735002f,0.57735002f,0.57735002f);
    out[0]=31; out[1]=32; out[2]=33;
}
int Caviar::set_shadow_table(unsigned char *table) {
    event('S'); check(this == &g_VEHDRAW_CAVIAR && table == ShadowColorTable);
    for (int i=0; i<256; ++i) check(table[i] == (i^0x5A));
    vec(light_vector_.values,11,12,13); vec(shadow_vector_.values,31,32,33);
    check(VehDrawTurnAngle == 901 && VehDrawTurnIncrement == 902);
    return 0;
}
void Caviar::set_camera_direct(const VOX_Vect *position,const VOX_Matrix *matrix) {
    event('D'); check(this == &g_VEHDRAW_CAVIAR && matrix == camera_matrix);
    vec(position->values,-70000,-80000,-90000);
    check(VehDrawTurnAngle == 901 && VehDrawTurnIncrement == 902);
}
// BODY
void run(int facing,int direction,int flags,int climate,int chassis,int mode,int offense,int corner_case) {
    static int epoch = 0;
    const int corner_sets[3][4] = {{1,1,1,1},{2,3,4,5},{7,7,7,7}};
    old_facing=facing; new_facing=direction; chosen=(flags&1) ? 2 : 1;
    sloped=(climate&0xE0)>=0x60 && (chassis!=0 || mode>=3);
    sequence = ((flags&1) ? "P" : "");
    sequence += sloped ? "1234AAAN" : ""; sequence += "CIMSD";
    phase=adds=0; VehDrawTurnVehicle=-17; VehDrawTurnAngle=901; VehDrawTurnIncrement=902;
    VehLightDirMap=VOX_Vect{{11,12,13}};
    g_VEHDRAW_CAVIAR.light_vector_=g_VEHDRAW_CAVIAR.shadow_vector_=VOX_Vect{{-1,-1,-1}};
    std::memset(Vehs,0,sizeof(Vehs)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Weapon,0,sizeof(Weapon)); std::memset(terrain,0,sizeof(terrain));
    Vehs[chosen].facing=int8_t((flags&1) ? -55 : facing);
    Vehs[chosen].proto_id=3; VehPrototypes[3].chassis_id=uint8_t(chassis); VehPrototypes[3].weapon_id=2;
    Weapon[2].mode=uint8_t(mode); Weapon[2].offense_rating=int8_t(offense);
    VehLiftX=(flags&1) ? 4 : 6; VehLiftY=(flags&1) ? 1 : 2; MapLongitude=7;
    const int bank=(epoch++)%2;
    MapTiles=terrain[bank];
    terrain[bank][17].climate=uint8_t(climate);
    for (int q=0; q<4; ++q) corners[q]=corner_sets[corner_case][q];
    // Independent flattening of one-based 7-level corner digits.
    table_index=0;
    for (int q=0; q<4; ++q) table_index=table_index*7+(corners[q]-1);
    for (int t=0; t<4; ++t) for (int i=0; i<2401; ++i)
        tables[t][i]=VOX_Vect{{float(1+t*3000+i),float(11+t),float(20-t)}};
    for (int t=0; t<4; ++t) for (int c=0; c<3; ++c) saved_normals[t][c]=tables[t][table_index].values[c];
    for (int i=0; i<256; ++i) ShadowColorTable[i]=uint8_t(i^0x5A);
    const int steps=vehdraw_turn_start(1,direction,13,flags);
    check(phase==int(sequence.size()) && VehDrawTurnVehicle==chosen);
    check(Vehs[chosen].facing==int8_t(direction));
    for (int v=0; v<4; ++v) if (v!=chosen) check(Vehs[v].facing==0);
    check(terrain[bank][17].climate==uint8_t(climate));
    const float current=float(((facing==-1 ? 0 : facing)-3)&7)*0.785375f;
    const float target=float((direction-3)&7)*0.785375f;
    // Positive arc stays wide in the original; the negative arc is stored
    // to float before comparison. Double is enough for these float sums.
    double positive=double(target)-current;
    if (positive<0) positive+=double(6.283f);
    const float negative=float(double(current)-target+(current>target ? 0.0 : double(6.283f)));
    const bool backwards=negative<positive;
    const double distance=backwards ? double(negative) : positive;
    const int expected_steps=int(13.0*distance*double(0.15915963f));
    check(steps==expected_steps && VehDrawTurnAngle==current);
    if (expected_steps==0) check(std::isnan(VehDrawTurnIncrement));
    else {
        const float expected_increment=float((backwards ? -distance : distance)/expected_steps);
        if (!near(VehDrawTurnIncrement,expected_increment))
            std::fprintf(stderr,"facing %d to %d: increment %.9g, expected %.9g; arcs %.17g / %.17g\n",
                         facing,direction,VehDrawTurnIncrement,expected_increment,double(negative),positive);
        check(near(VehDrawTurnIncrement,expected_increment));
    }
}
int main(int argc,char **) {
    if (argc > 1) {
        // Keep the original retained-positive/spilled-negative precision
        // boundary independently visible, rather than letting an existing
        // failure make unrelated damage controls falsely appear effective.
        for (int old=-1; old<8; ++old) for (int next=0; next<8; ++next)
            if ((((old==-1 ? 0 : old)-next)&7)==4) run(old,next,0,0,0,0,7,0);
        return 0;
    }
    for (int old=-1; old<8; ++old) for (int next=0; next<8; ++next)
        run(old,next,0,0,0,0,7,0);
    // Mode, not signed offense, governs the special chassis-zero branch.
    for (int flags=0; flags<4; ++flags) {
        run(1,6,flags,0x60,0,2,7,0);
        run(1,6,flags,0x60,0,3,0,1);
        run(1,6,flags,0x7F,0,255,-1,2);
        run(1,6,flags,0x60,1,0,0,2);
        run(1,6,flags,0x5F,1,255,7,1);
    }
    run(-2,258,1,0x60,1,0,0,1);
    return 0;
}
"""


def run_host(tmp_path,body,precision=False):
    compiler=shutil.which("g++")
    if not compiler: pytest.skip("host g++ required")
    source,executable=tmp_path/"turn.cpp",tmp_path/"turn"
    source.write_text(HARNESS.replace("// DECLARATIONS",declarations()).replace("// BODY",body))
    result=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined",
        "-fno-sanitize-recover=undefined",str(source),"-o",str(executable)],
        text=True,capture_output=True,timeout=30)
    assert result.returncode==0,result.stdout+result.stderr
    return subprocess.run([str(executable)]+(["precision"] if precision else []),
                          text=True,capture_output=True,timeout=10)


def test_live_turn_start(tmp_path):
    result=run_host(tmp_path,live_body())
    assert result.returncode==0,result.stdout+result.stderr


def test_antipodal_precision_branch(tmp_path):
    result=run_host(tmp_path,live_body(),precision=True)
    assert result.returncode==0,result.stdout+result.stderr


def replace_once(body,old,new):
    assert body.count(old)==1,old
    return body.replace(old,new)


@pytest.mark.parametrize("damage",[
    "weapon_offense", "late_light_snapshot", "facet_index", "uncopied_normal",
    "flags_not_bit", "guard_zero_steps", "wrong_wrap_branch", "missing_step_return",
    "rounded_positive_distance",
])
def test_turn_start_runtime_damage(tmp_path,damage):
    body=live_body()
    changes={
        "weapon_offense": ("Weapon[prototype.weapon_id].mode < 3", "Weapon[prototype.weapon_id].offense_rating < 3"),
        "facet_index": ("+ h4 - 400", "+ h4 - 399"),
        "uncopied_normal": ("const VOX_Vect n1 =", "const VOX_Vect &n1 ="),
        "flags_not_bit": ("if (flags & 1)", "if (flags != 0)"),
        "guard_zero_steps": ("VehDrawTurnIncrement = (float)(positive_distance / steps);",
                             "VehDrawTurnIncrement = steps ? (float)(positive_distance / steps) : 0.0f;"),
        "wrong_wrap_branch": ("if (VehDrawTurnAngle > target)", "if (VehDrawTurnAngle < target)"),
        "rounded_positive_distance": ("double positive_distance;", "float positive_distance;"),
    }
    if damage=="late_light_snapshot":
        body=replace_once(body,"const VOX_Vect light = VehLightDirMap;","")
        body=replace_once(body,"compute_camera(&normal, &camera);",
                          "compute_camera(&normal, &camera);\n    const VOX_Vect light = VehLightDirMap;")
    elif damage=="missing_step_return":
        assert body.count("return steps;")==2
        body=body.replace("return steps;","return 0;")
    else:
        body=replace_once(body,*changes[damage])
    result=run_host(tmp_path,body)
    # Only a compiled explicit behavioral failure counts, not UB or a signal.
    assert result.returncode==91,result.stdout+result.stderr
