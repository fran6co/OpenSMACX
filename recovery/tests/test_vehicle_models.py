"""Actual model-assembly bodies with bounded renderer/game-state doubles.

Expected model orders come from pinned branch/call operands, not seed returns
or renderer output. This is semantic coverage, not Win32 layout, native x87,
asset loading, or whole-game proof. Model definitions and record declarations
are selected from current headers; no original-address aliases execute here.
Predicate and renderer callbacks test source read order, not whether the native
compiler preserves the original out-of-line has_tech call boundary.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def clean(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source")


def live_source():
    source = clean((ROOT / "src/veh.cpp").read_text())
    matches = list(re.finditer(r"void\s+__cdecl\s+vehdraw_construct_\w+\([^;{]*\)\s*\{", source))
    assert len(matches) == 7
    bodies = "\n".join(braced(source, m.start()) for m in matches)
    prototypes = "\n".join(source[m.start():source.index("{", m.start())] + ";" for m in matches)
    caviar = clean((ROOT / "src/caviar.h").read_text())
    reset = braced(caviar, caviar.index("void reset_objects()"))
    records = []
    for file, names in (("veh.h", ("Veh", "VehPrototype", "RulesWeapon", "RulesArmor", "RulesAbility")),
                        ("caviar.h", ("VOX_Vect",))):
        header = clean((ROOT / "src" / file).read_text())
        records.extend(braced(header, header.index("struct " + n + " {")) + ";" for n in names)
    models = []
    for name, count in re.findall(r"extern CaviarData (g_\w+)(?:\[(\d+)\])?;", caviar):
        if count:
            values = ",".join('{"' + name + '",' + str(i) + '}' for i in range(int(count)))
            models.append("CaviarData " + name + "[" + count + "]={" + values + "};")
        else:
            models.append('CaviarData ' + name + '={"' + name + '",0};')
    storage = []
    for name in ("VehDrawAnimationStep", "VehInfantryOffset"):
        match, = re.findall(r"(?:int|VOX_Vect) " + name + r"[^;]*;", source)
        storage.append(match)
    return "\n".join(records), "\n".join(models), prototypes, bodies, reset, "\n".join(storage)


HARNESS = r"""
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#define __cdecl
#define UNUSED(x) (void)(x)
typedef char *LPSTR;
typedef int BOOL;
void check_at(bool value,int line) {
    if (!value) { std::fprintf(stderr,"behavior assertion at line %d\n",line); std::exit(91); }
}
#define check(v) check_at((v),__LINE__)
// RECORDS
const int RFLAG_ALIEN=0x80, RULE_FREEABIL=15;
Veh Vehs[4]; VehPrototype VehPrototypes[16];
RulesWeapon Weapon[26]; RulesArmor Armor[16]; RulesAbility Ability[32];
struct Player { int rule_flags,faction_bonus_count,faction_bonus_id[8],faction_bonus_val1[8]; } Players[8];
struct Console { int faction_id_; } ConsoleGlobal;
struct CaviarData { const char *name; int index; };
// MODELS
unsigned char BodyTintColorTables[8][24][256],ArmorTintColorTables[4][24][256];
unsigned char ConstantColorTables[24][256];
VOX_Vect VehGunMountOffset,VehDropOffset,VehHoverGunOffset,VehHoverLightsOffset;
// STORAGE
struct Event {
    std::string name; int index,step;
    unsigned char *colors; VOX_Vect *offset;
};
std::vector<Event> events;
std::vector<std::string> actions;
void (*on_add)(CaviarData *),(*on_tech)(),(*on_morale)(),(*on_visible)();
bool tech_answer=true,visible=false;
int morale_value=0,tech_id_seen,tech_faction_seen;
struct Caviar {
    uint32_t field_13CC_;
    void *allocation;
    float scale;
    // RESET
    float get_scaling() { actions.push_back("get_scale"); return scale; }
    void set_scaling(float v) { actions.push_back("set_scale"); scale=v; }
    int add_object(CaviarData *m,int step) { return add(m,0,0,step); }
    int add_object(CaviarData *m,VOX_Vect *v,int step) { return add(m,0,v,step); }
    int add_object(CaviarData *m,unsigned char *c,int step) { return add(m,c,0,step); }
    int add_object(CaviarData *m,unsigned char *c,VOX_Vect *v,int step) { return add(m,c,v,step); }
    int add(CaviarData *m,unsigned char *c,VOX_Vect *v,int step) {
        events.push_back({m->name,m->index,step,c,v}); actions.push_back("add");
        ++field_13CC_; if(on_add) on_add(m); return 7; // errors are not new early returns
    }
} renderer;
struct DesignWin { bool is_visible() { actions.push_back("visible"); if(on_visible)on_visible(); return visible; } } TutWinDesWindow;
BOOL has_tech(int tech,int faction) {
    actions.push_back("tech"); tech_id_seen=tech; tech_faction_seen=faction;
    if(on_tech) on_tech(); return tech_answer;
}
int morale_veh(int id,int a,int b) {
    check(id==0 && a==0 && b==0); actions.push_back("morale_veh");
    check(renderer.field_13CC_==0); if(on_morale)on_morale(); return morale_value;
}
uint32_t morale_alien(int id,int faction) {
    check(id==0 && faction==ConsoleGlobal.faction_id_); actions.push_back("morale_alien");
    check(renderer.field_13CC_==0); if(on_morale)on_morale(); return static_cast<uint32_t>(morale_value);
}
// PROTOTYPES
// BODIES
void reset() {
    std::memset(Vehs,0,sizeof(Vehs)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Players,0,sizeof(Players)); std::memset(Weapon,0,sizeof(Weapon));
    std::memset(Armor,0,sizeof(Armor)); std::memset(Ability,0,sizeof(Ability));
    events.clear(); actions.clear(); on_add=0; on_tech=0; on_morale=0; on_visible=0;
    tech_answer=true; visible=false; morale_value=0; VehDrawAnimationStep=4;
    renderer.field_13CC_=42; renderer.scale=1.0f; renderer.allocation=&renderer;
    ConsoleGlobal.faction_id_=6;
}
void names(std::initializer_list<const char *> expected) {
    check(events.size()==expected.size()); std::size_t i=0;
    for(const char *name:expected) check(events[i++].name==std::string("g_")+name+"_CAVIARDATA");
}
void event(std::size_t i,CaviarData &m,int step=0,unsigned char *colors=0,VOX_Vect *offset=0) {
    check(i<events.size()); const Event &e=events[i];
    check(e.name==m.name && e.index==m.index && e.step==step && e.colors==colors && e.offset==offset);
}
void mutate_bonus() { Players[3].faction_bonus_val1[0]=13; }
void shrink_bonus() { Players[3].faction_bonus_count=1; }
void grow_bonus() { Players[3].faction_bonus_count=2; on_tech=0; }
void change_alien(CaviarData *m) { if(m==&g_VT_CAVIARDATA) Players[3].rule_flags=0x80; }
void change_step(CaviarData *m) { if(m==&g_VSPTB_CAVIARDATA)VehDrawAnimationStep=50; }
void change_wing(CaviarData *m) { if(m==&g_VJTP_CAVIARDATA)Players[3].rule_flags=0x80; }
void visible_step() { VehDrawAnimationStep=200; }
void reroute_native() { Vehs[0].proto_id=10; renderer.scale=2.0f; }
void dirty_count() { renderer.field_13CC_=37; }
void change_final_mode(CaviarData *m) { if(m==&g_SP_SOPORIFIC_CAVIARDATA)Weapon[2].mode=11; }
void ability_cases() {
    reset(); const uint32_t flags=(1u<<24)|(1u<<25)|(1u<<27)|(1u<<26)|1u|(1u<<13)|
        (1u<<5)|(1u<<7)|(1u<<10)|(1u<<15)|(1u<<21)|(1u<<4)|(1u<<9)|(1u<<18);
    vehdraw_construct_abil(flags,&renderer,3);
    CaviarData *order[]={&g_SP_SOPORIFIC_CAVIARDATA,&g_SP_DISSWAVE_CAVIARDATA,
        &g_SP_NANOO_CAVIARDATA,&g_SP_MARINED_CAVIARDATA,&g_SSF_CAVIARDATA,&g_SFT_CAVIARDATA,
        &g_SAS_CAVIARDATA,&g_SCD_CAVIARDATA,&g_SAGS_CAVIARDATA,&g_VHR_CAVIARDATA1,
        &g_SRB_CAVIARDATA,&g_SDP_CAVIARDATA,&g_SCJ_CAVIARDATA,&g_SHT_CAVIARDATA};
    check(events.size()==14); for(int i=0;i<14;++i)event(i,*order[i]);
    reset(); Players[3].rule_flags=0x80; vehdraw_construct_abil(0x2001,&renderer,3);
    names({"SSFA","SFTA"});
    reset(); Players[3].faction_bonus_count=1; Players[3].faction_bonus_id[0]=15;
    Ability[0].preq_tech=-1; vehdraw_construct_abil(1,&renderer,3);
    names({"SSF","SSF"}); check(tech_id_seen==-1 && tech_faction_seen==3);
    events.clear(); on_tech=mutate_bonus; vehdraw_construct_abil(0x2001,&renderer,3);
    names({"SFT","SSF","SFT"}); check(tech_id_seen==-1);
    reset(); Players[3].faction_bonus_count=2;
    Players[3].faction_bonus_id[0]=Players[3].faction_bonus_id[1]=15;
    on_tech=shrink_bonus; vehdraw_construct_abil(1,&renderer,3); names({"SSF","SSF"});
    reset(); Players[3].faction_bonus_count=1;
    Players[3].faction_bonus_id[0]=Players[3].faction_bonus_id[1]=15;
    on_tech=grow_bonus; vehdraw_construct_abil(1,&renderer,3); names({"SSF","SSF","SSF"});
    reset(); Players[3].faction_bonus_count=1; Players[3].faction_bonus_id[0]=14;
    vehdraw_construct_abil(1,&renderer,3); names({"SSF"}); check(actions.size()==1);
    reset(); Players[3].faction_bonus_count=1; Players[3].faction_bonus_id[0]=15; tech_answer=false;
    vehdraw_construct_abil(1,&renderer,3); names({"SSF"});
}
void reactor_cases() {
    for(int reactor=1;reactor<=4;++reactor) {
        reset(); Weapon[2].mode=11; vehdraw_construct_reactor(3,0,2,reactor,0x8000,&renderer);
        check(events.size()==1);event(0,g_VIPTR_CAVIARDATA[reactor-1],0,0,&VehInfantryOffset);
        events.clear(); vehdraw_construct_reactor(3,8,2,reactor,0x8000,&renderer);
        check(events.size()==1);event(0,g_VPBR0_CAVIARDATA[reactor-1]);
        events.clear(); vehdraw_construct_reactor(3,1,2,reactor,0x8000,&renderer);
        check(events.size()==2);event(0,g_VR_CAVIARDATA[reactor-1]);event(1,g_VHR_CAVIARDATA2);
    }
    reset(); vehdraw_construct_reactor(3,0,-1,0,0x8000,&renderer);check(events.empty());
    check(VehInfantryOffset.values[0]==-400 && VehInfantryOffset.values[1]==200 && VehInfantryOffset.values[2]==0);
}
void chassis_cases() {
    reset(); Weapon[2].mode=7; vehdraw_construct_chassis(3,0,2,0,0,&renderer);
    names({"VB","VBP","VCL","VCLT00","VLIGHTS"});event(1,g_VBP_CAVIARDATA,0,&BodyTintColorTables[3][0][0]);
    reset(); on_add=change_step; vehdraw_construct_chassis(3,1,2,0,0,&renderer);
    names({"VGMC","VGMCP","VB","VBP","VSP","VSPTB","VSPTF","VLIGHTS"});
    event(5,g_VSPTB_CAVIARDATA,4);event(6,g_VSPTF_CAVIARDATA,50);
    reset(); vehdraw_construct_chassis(3,2,2,0,0,&renderer);
    names({"VB","VHT_VBP","VHTP","VHTTP","VHTA0","VHTTPA0","VLIGHTS"});
    event(6,g_VLIGHTS_CAVIARDATA,0,0,&VehHoverLightsOffset);
    reset(); vehdraw_construct_chassis(3,3,2,0,0,&renderer);names({"VGMC","VGMCP","VB","VBP","VFL"});
    reset(); vehdraw_construct_chassis(3,4,2,0,0,&renderer);names({"VB","VBP","VGMC","VGMCP","VCU","VCUP","VCUW","VCUA0"});
    reset(); on_add=change_wing; vehdraw_construct_chassis(3,5,2,0,0,&renderer);
    names({"VB","VBP","VGMC","VGMCP","VJTP","AA_WING"});
    reset(); visible=true; on_visible=visible_step; vehdraw_construct_chassis(3,6,2,0,0,&renderer);
    names({"VB","VBP","VGMC","VGMCP","VCT","VCTP","VCTB","VCT0"});
    event(6,g_VCTB_CAVIARDATA,4);event(7,g_VCT0_CAVIARDATA[0],0,&BodyTintColorTables[3][0][0]);check(VehDrawAnimationStep==201);
    reset(); vehdraw_construct_chassis(3,7,2,0,0,&renderer);names({"VB","VBP","VGMC","VGMCP","VGS","VGSP"});
    reset(); vehdraw_construct_chassis(3,8,24,0,0,&renderer);names({"VM","TECLOAD","VB","VBP"});
    reset(); Armor[2].defense_rating=-128; Weapon[21].mode=11;
    vehdraw_construct_chassis(3,1,21,2,1,&renderer);names({"VPT"});event(0,g_VPT_CAVIARDATA,4,0,&VehInfantryOffset);
    reset(); Players[3].rule_flags=0x80; vehdraw_construct_chassis(3,1,2,10,4,&renderer);
    check(events.size()==10);event(0,g_SCJ_CAVIARDATA);event(1,g_ACP_CAVIARDATA[3],0,&ArmorTintColorTables[0][0][0]);
    reset(); Weapon[2].mode=7; vehdraw_construct_chassis(-1,0,2,0,0,&renderer);
    event(1,g_VBP_CAVIARDATA,0,&BodyTintColorTables[0][0][0]);
}
void weapon_cases() {
    const int slots[]={-1,-1,0,0,1,3,2,2,4,4,5,5,6,7,7,7,8,8,8,8,9,9,9,9,10,13};
    for(int rating=-2;rating<=26;++rating) {
        reset();Weapon[2].offense_rating=rating;vehdraw_construct_weapon(3,2,2,&renderer);
        int index=rating==-1 ? 11 : slots[rating<0 ? 0 : (rating>25 ? 25 : rating)];
        if(index<0)check(events.empty());else {check(events.size()==1);event(0,g_VW_CAVIARDATA[index],0,0,&VehHoverGunOffset);}
    }
    reset();Weapon[2].offense_rating=99;vehdraw_construct_weapon(3,0,2,&renderer);
    names({"VW","VGMT","VGMTP"});event(0,g_VW_CAVIARDATA[12],0,0,&VehGunMountOffset);
    reset();vehdraw_construct_weapon(3,0,12,&renderer);check(events.empty());
    reset();vehdraw_construct_weapon(3,0,13,&renderer);names({"RESBOLT"});event(0,g_RESBOLT_CAVIARDATA,0,0,&VehGunMountOffset);
    reset();Weapon[2].mode=9;on_add=change_alien;vehdraw_construct_weapon(3,0,2,&renderer);names({"VT","AT","ATP"});
    reset();Weapon[2].mode=8;vehdraw_construct_weapon(3,1,2,&renderer);names({"DROPLET"});event(0,g_DROPLET_CAVIARDATA,0,0,&VehDropOffset);
    reset();vehdraw_construct_weapon(3,8,-1,&renderer);check(events.empty());
    reset();vehdraw_construct_weapon(3,0,25,&renderer);names({"VM","FUNLOAD"});
}
void armor_cases() {
    for(int alien=0;alien<2;++alien) for(int tint=0;tint<2;++tint) for(int chassis=-1;chassis<=7;++chassis) {
        reset();Players[3].rule_flags=alien ? 0x80 : 0;Weapon[2].mode=7;
        vehdraw_construct_armor(3,chassis,2,tint ? 2 : 1,0,&renderer);
        CaviarData *base=alien ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        CaviarData *first=base,*second=0;
        if(chassis==1)second=alien ? &g_AA_ROVER_CAVIARDATA : &g_VSPA0_CAVIARDATA[0];
        if(chassis==2){first=&g_VHTA0_CAVIARDATA[1];second=&g_VHTTPA0_CAVIARDATA[1];}
        if(chassis==4)first=&g_VCUA0_CAVIARDATA[1];
        if(chassis==5)second=alien ? &g_AA_WING_CAVIARDATA[1] : &g_VJT0_CAVIARDATA[1];
        if(chassis==6){first=&g_VCT0_CAVIARDATA[1];second=base;}
        unsigned char *colors=tint ? &ArmorTintColorTables[0][0][0] : 0;
        check(events.size()==(second ? 2u : 1u));event(0,*first,0,colors);if(second)event(1,*second,0,colors);
    }
    reset();Weapon[2].mode=0;VehDrawAnimationStep=-1;vehdraw_construct_armor(3,0,2,1,0,&renderer);
    event(0,g_VIPTA_CAVIARDATA[0],-1,0,&VehInfantryOffset);
    reset();Weapon[2].mode=11;vehdraw_construct_armor(3,0,2,4,0,&renderer);
    names({"VIPTAWL"});event(0,g_VIPTAWL_CAVIARDATA,4,0,&VehInfantryOffset);
    reset();Players[3].rule_flags=0x80;vehdraw_construct_armor(3,6,2,4,0,&renderer);
    names({"A","APWALL","VCT0","AA01"});event(2,g_VCT0_CAVIARDATA[0]);
    reset();vehdraw_construct_armor(3,6,2,5,0,&renderer);check(events.empty());
    reset();vehdraw_construct_armor(3,6,2,5,1,&renderer);
    names({"VCT0","VA01"});event(0,g_VCT0_CAVIARDATA[1],0,&ConstantColorTables[0][0]);event(1,g_VA01_CAVIARDATA,0,&ConstantColorTables[0][0]);
}
void vehicle_cases() {
    for(int proto=8;proto<=10;++proto)for(int morale=-1;morale<=8;++morale) {
        reset();Vehs[0].proto_id=proto;Vehs[0].faction_id=2;VehPrototypes[proto].chassis_id=5;
        morale_value=morale;VehDrawAnimationStep=-1;vehdraw_construct_vehicle(0,&renderer);
        int n=morale<0 ? 0 : (morale>(proto==10 ? 7:6) ? (proto==10 ? 7:6) : morale);
        check(renderer.scale==float(n+(proto==8 ? 12 : (proto==9 ? 6:5)))*.125f);
        check(events.size()==1 && renderer.field_13CC_==1 && renderer.allocation==&renderer);
        event(0,proto==8 ? g_NW_CAVIARDATA : (proto==9 ? g_NI_CAVIARDATA : g_NLC_CAVIARDATA),proto==10 ? -1:29);
    }
    reset();on_morale=reroute_native;morale_value=2;vehdraw_construct_vehicle(0,&renderer);
    names({"NLC"});check(renderer.scale==1.75f);check(actions[2]=="morale_alien");
    reset();Vehs[0].proto_id=1;Vehs[0].faction_id=3;VehPrototypes[1].chassis_id=8;
    VehPrototypes[1].weapon_id=24;VehPrototypes[1].ability_flags=1u<<24;
    on_morale=dirty_count;
    vehdraw_construct_vehicle(0,&renderer);names({"VM","TECLOAD","VB","VBP","SP_SOPORIFIC"});
    check(renderer.field_13CC_==5 && renderer.allocation==&renderer);
    reset();vehdraw_construct_vehicle(3,8,24,0,2,1u<<24,&renderer);
    names({"VM","TECLOAD","VB","VBP","SP_SOPORIFIC","VPBR0"});event(5,g_VPBR0_CAVIARDATA[1]);
    check(renderer.field_13CC_==6 && renderer.allocation==&renderer);
    reset();Armor[2].defense_rating=-128;Weapon[21].mode=11;
    vehdraw_construct_vehicle(3,1,21,2,1,0,&renderer);
    names({"VPT","VIPTA","VIPTR"});
    event(0,g_VPT_CAVIARDATA,4,0,&VehInfantryOffset);
    event(1,g_VIPTA_CAVIARDATA[0],4,&ArmorTintColorTables[0][0][0],&VehInfantryOffset);
    event(2,g_VIPTR_CAVIARDATA[0],0,0,&VehInfantryOffset);
    reset();Weapon[2].mode=7;on_add=change_final_mode;
    vehdraw_construct_vehicle(3,0,2,0,1,1u<<24,&renderer);
    names({"VB","VBP","VCL","VCLT00","VLIGHTS","VWNTT","SP_SOPORIFIC","VIPTR"});
    event(7,g_VIPTR_CAVIARDATA[0],0,0,&VehInfantryOffset);
}
int main() { ability_cases();reactor_cases();chassis_cases();weapon_cases();armor_cases();vehicle_cases(); }
"""


def run_host(tmp_path, damage=None):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    records, models, prototypes, bodies, reset, storage = live_source()
    if damage:
        old, new = damage
        assert old in bodies
        bodies = bodies.replace(old, new, 1)
    code = HARNESS
    for marker, value in (("RECORDS", records), ("MODELS", models), ("STORAGE", storage),
                          ("RESET", reset), ("PROTOTYPES", prototypes), ("BODIES", bodies)):
        code = code.replace("// " + marker, value)
    source, binary = tmp_path / "models.cpp", tmp_path / "models"
    source.write_text(code)
    build = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                            "-fno-sanitize-recover=undefined", str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert build.returncode == 0, build.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode == (91 if damage else 0), result.stderr


def test_actual_seven_function_family(tmp_path):
    run_host(tmp_path)


@pytest.mark.parametrize("damage", [
    ("if (Players[faction_id].faction_bonus_id[i] != RULE_FREEABIL)",
     "if (Players[faction_id].faction_bonus_id[i] == RULE_FREEABIL)"),
    ("const uint32_t bonus = abilities &", "const uint32_t bonus = 0u &"),
    ("&g_VIPTR_CAVIARDATA[reactor - 1]", "&g_VIPTR_CAVIARDATA[0]"),
    ("vehdraw_construct_abil(abilities, renderer, faction_id);",
     "vehdraw_construct_abil(faction_id, renderer, faction_id);"),
    ("proto_id = Vehs[veh_id].proto_id;\n    if", "/* stale prototype */\n    if"),
    ("const int faction_id = Vehs[veh_id].faction_id;\n    renderer->reset_objects();",
     "const int faction_id = Vehs[veh_id].faction_id; /* missing second reset */"),
    ("Armor[armor].defense_rating <= 1", "static_cast<unsigned char>(Armor[armor].defense_rating) <= 1"),
    ("renderer->add_object(&g_VLIGHTS_CAVIARDATA, 0);", "/* missing utility lights */"),
    ("renderer->add_object(&g_VSPTF_CAVIARDATA, VehDrawAnimationStep);",
     "renderer->add_object(&g_VSPTF_CAVIARDATA, 4);"),
    ("if (second) {", "if (false && second) {"),
    ("renderer->add_object(&g_RESBOLT_CAVIARDATA, &VehGunMountOffset, 0);",
     "renderer->add_object(&g_RESLASER_CAVIARDATA, &VehGunMountOffset, 0);"),
])
def test_model_family_rejects_runtime_damage(tmp_path, damage):
    run_host(tmp_path, damage)
