"""Control the product redraw against original 496D00/table496DE4 evidence.

The drawers are observers, not a claim that the unfinished pages work in-game.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


def product_body():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("void ReportWin::on_redraw() {")
    return source[start:source.index("\n}\n", start) + 3]


def harness(body):
    return r'''
struct RECT {};
int events[10], count;
void event(int id) { events[count++] = id; }
struct Buffer {
    RECT working;
    void set_clip(RECT *area) { event(area == &working ? 11 : 10); }
};
struct ReportIfFixture { unsigned int field_8_; } ReportIfGlobal;
struct ReportWin : Buffer {
    int field_59AC_;
    RECT energy_panel_rect_;
    void reset() { event(9); }
    RECT *buffer_working_rect() { return &working; }
    void draw_intel() { event(0); }
    void draw_labs() { event(2); }
    void draw_energy() { event(3); }
    void draw_ops() { event(4); }
    void draw_projects() { event(5); }
    void draw_sat() { event(6); }
    void draw_security() { event(7); }
    void draw_score() { event(8); }
    void on_redraw();
};
''' + body + r'''
int main() {
    // Table entries1/default do not draw. Unsigned ja handles negative IDs.
    const unsigned int categories[] = {0,1,2,3,4,5,6,7,8,9,100,~0u};
    for (unsigned int i=0; i<sizeof(categories)/sizeof(categories[0]); ++i) {
        ReportWin window;
        window.field_59AC_=73;
        ReportIfGlobal.field_8_=categories[i];
        count=0;
        window.on_redraw();
        bool drawn=categories[i]<=8 && categories[i]!=1;
        if(window.field_59AC_!=-1 || count!=(drawn?4:3)) return 1;
        if(events[0]!=9 || events[1]!=10 || events[count-1]!=11) return 2;
        if(drawn && events[2]!=static_cast<int>(categories[i])) return 3;
    }
    return 0;
}
'''


@pytest.mark.parametrize("damage", [None, "dispatch", "reset", "selection", "clip"])
def test_product_reportwin_redraw(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = product_body()
    mutations = {
        "dispatch": ("case 0: draw_intel();", "case 0: draw_labs();"),
        "reset": ("reset();", ""),
        "selection": ("field_59AC_ = -1;", "field_59AC_ = 0;"),
        "clip": ("canvas.set_clip(buffer_working_rect());", ""),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    source = tmp_path / "redraw.cpp"
    source.write_text(harness(body))
    executable = tmp_path / "redraw"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None)


def energy_body():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("void ReportWin::draw_energy() {")
    return source[start:source.index("\n}\n", start) + 3]


def energy_harness(body):
    return r'''
#include <cstdio>
#include <cstring>
struct RECT { int left,top,right,bottom; };
struct BoxSpriteParams {} styles[9];
BoxSpriteParams *InterfaceBoxStyles=styles;
int box_count, line_count, right_count, centered_count, wrap_count;
int parse_sections, parse_numbers[16], parse_number_count, parse_says_count;
int loan_payment_case, facility_case;
char sections[8][64], captured[2048];
char StringTemp[1024];
char *ScriptPopupFile="SCRIPT";
int GenderDefault, PluralityDefault;
char *CharUpperA(char *s) { for(char *p=s;*p;++p) if(*p>='a'&&*p<='z') *p-=32; return s; }
struct Buffer {
    void box_sprite(RECT *,BoxSpriteParams *) { ++box_count; }
    void set_text_color(int,int,int,int) {}
    void set_text_color2(int,int,int,int) {}
    void set_text_color3(int,int,int,int) {}
    void write_cent_l(const char *s,RECT *,int) { ++centered_count; std::strcat(captured,s); }
    void write_l(const char *s,int,int,int) { ++line_count; std::strcat(captured,s); }
    void write_right_l(const char *s,int,int,int w,int) {
        ++right_count; std::strcat(captured,s); if(w!=-15) right_count+=100;
    }
    void wrap_flying(const char *s,int,int,int) { ++wrap_count; std::strcat(captured,s); }
    void wrap_flying_flush() {}
};
struct Dialog {
    int field_48_,field_B4_,field_24_,field_28_;
    int init(RECT *,int) { return 1; }
    int set_dialog_font(void *,void *,void *) { return 1; }
    void set_dialog_text_color(int,int,int,int) {}
    void set_dialog_text_color2(int,int,int,int) {}
    void set_dialog_text_color3(int,int,int,int) {}
};
int list_close_count,list_item_count,list_attach_flags,list_selected_id,list_show_count;
char list_items[16][512]; int list_ids[16];
struct GraphicWin : Buffer { void show(int) { ++list_show_count; } };
struct ListBox : Dialog, GraphicWin {
    void close() { ++list_close_count; }
    void set_report_text_color(int) {}
    int item(char *s,int id) {
        std::strcpy(list_items[list_item_count],s);
        list_ids[list_item_count++]=id; return 1;
    }
    void reset_report_selection() {}
    int attach(GraphicWin *,int,int,int flags) { list_attach_flags=flags; return 1; }
    void set_selected_id(int id) { list_selected_id=id; }
    void show(int) { ++list_show_count; }
};
struct Player {
    const char *adj_name_faction,*noun_faction,*title_leader,*name_leader;
    int noun_gender,is_noun_plural,rule_flags,rule_interest;
};
Player Players[8]={{"Native","Native","Native Leader","Native",0,0,0,0},
 {"Alpha","Alpha","Alpha Leader","Alpha",0,0,0x4000,0},
 {"Beta","Beta","Beta Leader","Beta",0,1,0,0}};
struct PlayerData {
    int energy_reserves,sanction_turns,diplo_treaties[8],loan_balance[8],loan_payment[8];
};
PlayerData PlayersData[8];
struct BaseFixture {
    int faction_id_current;
    unsigned char facilities_built[12];
} Bases[80];
int BaseCurrentCount=2;
int MaxPlayerNum=8, LockEnableMask=0x600;
int EnergyEconomyTotal=12, EnergyMaintenanceTotal=4, EnergyCommerceDelta=8, NetIncomeGlobal=16;
int SecretProjectGrid=0;
struct SecretProjectFixture { int planetary_energy_grid; } SecretProject={0};
int bitmask_calls;
void bitmask(int value,int *offset,int *mask) {
    ++bitmask_calls; *offset=value/8; *mask=1<<(value%8);
}
int maintenance_calls,say_base_calls,say_facility_calls;
int maintenance_facility[16], maintenance_faction[16];
int fac_maint(int facility,int faction) {
    maintenance_facility[maintenance_calls]=facility;
    maintenance_faction[maintenance_calls++]=faction;
    return facility+faction+2;
}
void say_base(char *out,int id) {
    ++say_base_calls; std::strcat(out,"BASE"); char n[16];
    std::sprintf(n,"%d",id); std::strcat(out,n);
}
void say_facility(void *raw,int id,int) {
    ++say_facility_calls; char *out=(char *)raw; std::strcat(out,"FAC"); char n[16];
    std::sprintf(n,"%d",id); std::strcat(out,n);
}
const char *label_get(int id) {
    switch(id) {
    case 0xA98/4:return "ENERGY";
    case 0xB9C/4:return "RESERVES";
    case 0xE4/4:return "CREDITS";
    case 0x5F4/4:return "INCOME";
    case 0x5F8/4:return "SUPPORT";
    case 0x5FC/4:return "COMMERCE";
    case 0x708/4:return "LOAN";
    case 0x604/4:return "PAYMENT";
    case 0xEFC/4:return "NET";
    }
    return "LABEL";
}
void energy_compute(int,int) {}
void parse_num(int,int value) { parse_numbers[parse_number_count++]=value; }
int parse_says(int,const char *,int,int) { ++parse_says_count; return 0; }
void parse_it(char *,const char *section) {
    std::strcpy(sections[parse_sections++],section);
    std::strcpy(StringTemp,section);
}
void reportwin_append_number(char *out,int value) {
    char number[32]; std::sprintf(number,"%d",value); std::strcat(out,number);
}
struct ReportIfFixture {
    void *field_1E184_;
    int report_faction_id() const { return 1; }
} ReportIfGlobal={0};
struct ReportWin : GraphicWin {
    RECT energy_panel_rect_,rect1_;
    int field_E268_,field_59AC_,field_59B0_;
    ListBox listBox_;
    ReportWin():field_E268_(14),field_59AC_(-1),field_59B0_(0) {
        energy_panel_rect_.left=10; energy_panel_rect_.top=20;
        energy_panel_rect_.right=410; energy_panel_rect_.bottom=320;
    }
    void draw_energy();
};
''' + body + r'''
int main() {
    if(loan_payment_case) PlayersData[1].diplo_treaties[2]=0x10;
    PlayersData[1].energy_reserves=100;
    PlayersData[1].loan_balance[2]=10;
    PlayersData[1].loan_payment[2]=2;
    PlayersData[2].loan_balance[1]=8;
    PlayersData[2].sanction_turns=3;
    PlayersData[1].energy_reserves=100;
    Bases[0].faction_id_current=0;
    Bases[1].faction_id_current=1;
    Bases[0].facilities_built[0] |= 1;
    Bases[1].facilities_built[0] |= 4;
    SecretProject.planetary_energy_grid=1;
    if(facility_case) {
        BaseCurrentCount=67;
        for(int i=0;i<BaseCurrentCount;++i) Bases[i].faction_id_current=-1;
        Bases[1].faction_id_current=1;
        Bases[3].faction_id_current=1;
        Bases[66].faction_id_current=1;
        Bases[1].facilities_built[0] |= 4;
        Bases[3].facilities_built[0] |= 4;
        Bases[66].facilities_built[0] |= 4;
    }
    ReportWin w;
    if(facility_case) {
        w.field_59AC_=(facility_case==2?66:2);
        w.field_59B0_=0;
    }
    w.draw_energy();
    if(box_count!=6 || centered_count!=3 || line_count<4 || right_count<4) return 1;
    if(parse_sections!=2 || std::strcmp(sections[0],"FACTBON2")
       || std::strcmp(sections[1],loan_payment_case
            ? "LOANPAYMENT" : "LOANINCOME")) return 2;
    if(wrap_count!=2 || parse_says_count!=4) return 3;
    if(parse_number_count!=3 || parse_numbers[0]!=0 || parse_numbers[1]!=10
       || parse_numbers[2]!=2) return 4;
    if(w.field_59AC_!=(facility_case==2?66:(facility_case?2:1))
       || w.field_59B0_!=(facility_case?0:1) || w.rect1_.right!=183
       || std::strstr(captured,"ALPHA") == 0
       || std::strstr(captured,"NET") == 0) return 5;
    if(list_close_count!=1
       || list_item_count!=(facility_case==2?2:(facility_case?5:3))
       || list_ids[0]!=(facility_case==2?0x05F5E0FE:(facility_case?0x401:2))
       || list_attach_flags!=0x81020 || list_selected_id!=99999999
       || list_show_count!=1 || say_facility_calls!=1
       || say_base_calls!=(facility_case==2?0:(facility_case?3:1)))
        return 6;
    if(facility_case==2) {
        if(list_ids[0]!=0x05F5E0FE || list_ids[1]!=99999999
           || maintenance_calls!=0 || bitmask_calls!=0) return 10;
        return 0;
    }
    if(facility_case) {
        if(list_ids[1]!=0x403 || list_ids[2]!=0x442
           || list_ids[3]!=0x05F5E0FE || list_ids[4]!=99999999
           || maintenance_calls!=3 || maintenance_facility[0]!=2
           || maintenance_facility[1]!=2 || maintenance_facility[2]!=2
           || maintenance_faction[0]!=1 || maintenance_faction[1]!=1
           || maintenance_faction[2]!=1 || bitmask_calls!=3) return 8;
        if(std::strstr(list_items[0],"BASE1")==0
           || std::strstr(list_items[1],"BASE3")==0
           || std::strstr(list_items[2],"BASE66")==0) return 9;
        return 0;
    }
    if(list_ids[1]!=0x05F5E0FE || list_ids[2]!=99999999
       || std::strstr(list_items[0],"FAC2")==0
       || std::strstr(list_items[0],"2.5")==0
       || std::strstr(list_items[2],"LABEL")==0) {
        return 7;
    }
    return 0;
}
'''


@pytest.mark.parametrize("damage", [None, "skip_relationships", "wrong_principal",
                                    "skip_facilities"])
@pytest.mark.parametrize("loan_payment", [False, True])
@pytest.mark.parametrize("facility_case", [0, 1, 2])
def test_production_draw_energy_accounts_and_relationships(tmp_path, damage,
                                                           loan_payment, facility_case):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "energy.cpp"
    body = energy_body()
    mutations = {
        "skip_relationships": (
            "if (outgoing_balance == 0 && incoming_balance == 0) continue;",
            "if (true) continue;"),
        "wrong_principal": (
            "parse_num(1, principal);",
            "parse_num(1, PlayersData[viewer].energy_reserves);"),
        "skip_facilities": (
            "if (Bases[base_id].faction_id_current != viewer) continue;",
            "if (Bases[base_id].faction_id_current == viewer) continue;"),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    harness = energy_harness(body)
    if loan_payment:
        harness = harness.replace("int main() {", "int main() {\n    loan_payment_case=1;")
    if facility_case:
        harness = harness.replace("int main() {", "int main() {\n    facility_case=%d;" % facility_case)
    source.write_text(harness)
    executable = tmp_path / "energy"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    expected_success = damage is None or (damage == "skip_facilities" and facility_case != 0)
    assert (result.returncode == 0) == expected_success, result.stderr


def lab_child_collector():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("static int reportwin_collect_lab_children(")
    return source[start:source.index("\n}", start) + 2]


def test_reportwin_lab_child_slots_follow_native_prerequisite_scan(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "lab_children.cpp"
    source.write_text(r'''
struct RulesTechnology { int preq_tech_1, preq_tech_2; };
const int MaxTechnologyNum = 89;
RulesTechnology Technology[MaxTechnologyNum];
''' + lab_child_collector() + r'''
int main() {
    for (int i=0; i<MaxTechnologyNum; ++i)
        Technology[i].preq_tech_1 = Technology[i].preq_tech_2 = -1;
    Technology[3].preq_tech_1 = 12;
    Technology[4].preq_tech_2 = 12;
    Technology[5].preq_tech_1 = Technology[5].preq_tech_2 = 12;
    Technology[6].preq_tech_1 = 13;
    int child_storage[5] = {-1,-1,-1,-1,5};
    int *children=child_storage;
    int count = reportwin_collect_lab_children(12, children);
    if (count != 4) return 1;
    if (children[0] != 3 || children[1] != 4
        || children[2] != 5 || children[3] != 5) return 2;
    for (int i=0; i<MaxTechnologyNum; ++i)
        Technology[i].preq_tech_1 = Technology[i].preq_tech_2 = 12;
    count = reportwin_collect_lab_children(12, children);
    if (count != 5 || children[0] != 0 || children[1] != 0
        || children[2] != 1 || children[3] != 1) return 3;
    if (child_storage[4] != 5) return 5;
    return reportwin_collect_lab_children(-2, children) == 0 ? 0 : 4;
}
''')
    executable = tmp_path / "lab_children"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, "unlock harness returned " + str(result.returncode) + ": " + result.stderr


def lab_unlock_sections():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("static void reportwin_append_lab_entry(")
    return source[start:source.index(
        "// The three-per-category records", start)]


def test_production_lab_unlock_scans_use_native_bounds_and_link_ids(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "lab_unlocks.cpp"
    source.write_text(r'''
#include <cstdio>
#include <cstring>
#include <stdint.h>
typedef char *LPSTR;
struct RulesReactor { int preq_tech; LPSTR name; } Reactor[4];
struct RulesChassis { int preq_tech; LPSTR offsv_1_name; int speed, range; } Chassis[19];
struct RulesWeapon { int preq_tech; LPSTR name; int offense_rating, mode; } Weapon[26];
struct RulesArmor { int preq_tech; LPSTR name; int defense_rating; } Armor[14];
struct RulesAbility { int preq_tech; LPSTR name; } Ability[29];
struct VehPrototype { int flags; int16_t preq_tech; char veh_name[32]; } VehPrototypes[29];
struct RulesFacility { int preq_tech, free_tech; LPSTR name; } Facility[134];
struct RulesCitizen { LPSTR singular_name; int preq_tech; } Citizen[10];
char StringTemp[4096];
void reportwin_append_number(char *out,int value) {
    char number[80]; std::sprintf(number,"%d",value); std::strcat(out,number);
}
char *label_get(int id) {
    switch(id) {
    case 0x234/4:return (char*)"Reactors";
    case 0x120/4:return (char*)"Chassis";
    case 0x124/4:return (char*)"Weapons";
    case 0x128/4:return (char*)"Armor";
    case 0x12C/4:return (char*)"Abilities";
    case 0x318/4:return (char*)"Vehicles";
    case 0x110/4:return (char*)"Facilities";
    case 0x4DC/4:return (char*)"(free";
    case 0x310/4:return (char*)"N/A";
    case 0x200/4:return (char*)"MODE";
    case 0x118/4:return (char*)"Social";
    case 0x4CC/4:return (char*)"Citizens";
    }
    return (char*)"?";
}
struct Buffer {
    int wrapped, measured;
    char sections[12][512];
    int xs[12],ys[12],widths[12];
    int wrap(char *s,int x,int y,int width) {
        xs[wrapped]=x; ys[wrapped]=y; widths[wrapped]=width;
        std::strcpy(sections[wrapped++],s); return y+10;
    }
    int wrap_height(char *s,int width) {
        widths[wrapped]=width;
        std::strcpy(sections[wrapped++],s); ++measured; return 10;
    }
};
struct Datalink { void construct_id(char *out,int c,int i) {
    std::sprintf(out,"%d",c*10000+i);
} } DatalinkMain;
char *registered_names[32]; int registered_name_count;
void register_name(char *name) { registered_names[registered_name_count++]=name; }
struct TestStringTable {
    int lookup_calls, prototype_lookup_calls;
    TestStringTable() : lookup_calls(0), prototype_lookup_calls(0) {}
    char *get(int id) {
        ++lookup_calls;
        const uint32_t key=static_cast<uint32_t>(id);
        if(key==static_cast<uint32_t>(reinterpret_cast<uintptr_t>(VehPrototypes[2].veh_name))
           || key==static_cast<uint32_t>(reinterpret_cast<uintptr_t>(VehPrototypes[10].veh_name)))
            ++prototype_lookup_calls;
        for(int i=0;i<registered_name_count;++i)
            if(key==static_cast<uint32_t>(reinterpret_cast<uintptr_t>(registered_names[i])))
                return registered_names[i];
        return (char*)"?";
    }
} StringTable;
void help_create_link(char *out,char *name,char *id) {
    std::strcat(out,"$LINK<"); std::strcat(out,name);
    std::strcat(out,"="); std::strcat(out,id); std::strcat(out,">");
}
''' + lab_unlock_sections().replace(
    "reinterpret_cast<int>(name)",
    "static_cast<int>(reinterpret_cast<intptr_t>(name))") + r'''
int main() {
    for(int i=0;i<4;++i) Reactor[i].preq_tech=-1;
    for(int i=0;i<19;++i) Chassis[i].preq_tech=-1;
    for(int i=0;i<26;++i) Weapon[i].preq_tech=-1;
    for(int i=0;i<14;++i) Armor[i].preq_tech=-1;
    for(int i=0;i<29;++i) Ability[i].preq_tech=-1;
    for(int i=0;i<29;++i) { VehPrototypes[i].flags=0; VehPrototypes[i].preq_tech=-1; }
    for(int i=0;i<134;++i) Facility[i].preq_tech=Facility[i].free_tech=-1;
    for(int i=0;i<10;++i) Citizen[i].preq_tech=-1;
    Reactor[1].preq_tech=42; Reactor[1].name=(char*)"R1";
    Chassis[0].preq_tech=42; Chassis[0].offsv_1_name=(char*)"C0";
    Chassis[0].speed=255; Chassis[0].range=0;
    Chassis[8].preq_tech=42; Chassis[8].offsv_1_name=(char*)"C8";
    Chassis[8].speed=2; Chassis[8].range=2;
    Weapon[0].preq_tech=42; Weapon[0].name=(char*)"W0";
    Weapon[0].offense_rating=1; Weapon[0].mode=3;
    Weapon[25].preq_tech=42; Weapon[25].name=(char*)"W25";
    Weapon[25].offense_rating=4; Weapon[25].mode=2;
    Armor[0].preq_tech=42; Armor[0].name=(char*)"A0";
    Armor[0].defense_rating=-1;
    Armor[13].preq_tech=42; Armor[13].name=(char*)"A13";
    Armor[13].defense_rating=3;
    Ability[28].preq_tech=42; Ability[28].name=(char*)"B28";
    VehPrototypes[2].flags=1; VehPrototypes[2].preq_tech=42;
    std::strcpy(VehPrototypes[2].veh_name,"V2");
    VehPrototypes[3].flags=0; VehPrototypes[3].preq_tech=42;
    std::strcpy(VehPrototypes[3].veh_name,"inactive");
    VehPrototypes[4].flags=1; VehPrototypes[4].preq_tech=41;
    std::strcpy(VehPrototypes[4].veh_name,"other-tech");
    VehPrototypes[10].flags=1; VehPrototypes[10].preq_tech=42;
    std::strcpy(VehPrototypes[10].veh_name,"V10");
    Facility[1].preq_tech=42; Facility[1].name=(char*)"F1";
    Facility[70].preq_tech=42; Facility[70].name=(char*)"F70";
    Facility[133].free_tech=42; Facility[133].name=(char*)"F133";
    Citizen[6].preq_tech=42; Citizen[6].singular_name=(char*)"P6";
    Citizen[9].preq_tech=42; Citizen[9].singular_name=(char*)"P9";
    register_name(Reactor[1].name);
    register_name(Chassis[0].offsv_1_name); register_name(Chassis[8].offsv_1_name);
    register_name(Weapon[0].name); register_name(Weapon[25].name);
    register_name(Armor[0].name); register_name(Armor[13].name);
    register_name(Ability[28].name);
    register_name(Facility[1].name); register_name(Facility[70].name);
    register_name(Facility[133].name); register_name(Citizen[6].singular_name);
    Buffer canvas={}; int height=0; uint32_t frame_width=166;
    int y=20;
    reportwin_draw_lab_unlock_sections(canvas,42,true,true,10,y,&frame_width,height);
    if(canvas.wrapped!=8 || height!=0) return 1;
    if(canvas.xs[0]!=10 || canvas.ys[0]!=20 || canvas.widths[0]!=160
       || canvas.xs[7]!=10 || canvas.ys[7]!=90 || canvas.widths[7]!=160
       || y!=100) return 12;
    if(std::strcmp(canvas.sections[0],"Reactors: $LINK<R1=50002>")) return 2;
    if(std::strcmp(canvas.sections[1],"Chassis: $LINK<C0=40000> (N/A), $LINK<C8=40008> (2MODE)")) return 3;
    if(std::strcmp(canvas.sections[2],"Weapons: $LINK<W0=60000> (MODE), $LINK<W25=60025> (4MODE)")) return 4;
    if(std::strcmp(canvas.sections[3],"Armor: $LINK<A0=70000> (N/A), $LINK<A13=70013> (3)")) return 5;
    if(std::strcmp(canvas.sections[4],"Abilities: $LINK<B28=80028>")) return 6;
    if(std::strcmp(canvas.sections[5],"Vehicles: $LINK<V2=30002>, $LINK<V10=30010>")) return 7;
    if(std::strcmp(canvas.sections[6],"Facilities: $LINK<F1=100001>, $LINK<F70=110070>, $LINK<F133=110133> (free)")) return 8;
    if(std::strcmp(canvas.sections[7],"Citizens: P6")) return 9;
    canvas.wrapped=0; height=0;
    y=20;
    reportwin_draw_lab_unlock_sections(canvas,42,false,false,10,y,&frame_width,height);
    if(canvas.wrapped!=8 || height!=80) return 10;
    if(canvas.widths[0]!=160 || canvas.widths[7]!=160) return 13;
    if(std::strcmp(canvas.sections[0],"Reactors: R1")
       || std::strcmp(canvas.sections[1],"Chassis: C0 (N/A), C8 (2MODE)")
       || std::strcmp(canvas.sections[5],"Vehicles: V2, V10")
       || std::strcmp(canvas.sections[6],"Facilities: F1, F70, F133 (free)")
       || std::strcmp(canvas.sections[7],"Citizens: P6")) return 11;
    canvas.wrapped=0; height=0; y=20;
    reportwin_draw_lab_unlock_sections(canvas,42,false,true,10,y,&frame_width,height);
    if(canvas.wrapped!=8 || height!=0 || y!=100) return 14;
    if(std::strcmp(canvas.sections[5],"Vehicles: V2, V10")
       || std::strcmp(canvas.sections[7],"Citizens: P6")) return 15;
    if(StringTable.lookup_calls!=36 || StringTable.prototype_lookup_calls!=0)
        return 16;
    return 0;
}
''')
    executable = tmp_path / "lab_unlocks"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, "unlock harness returned " + str(result.returncode) + ": " + result.stderr


def lab_category_style():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("static BoxSpriteParams *reportwin_lab_category_style(")
    return source[start:source.index("\n}\n", start) + 3]


def labs_body():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("void ReportWin::draw_labs() {")
    return source[start:source.index("\n}\n", start) + 3]


def test_production_draw_labs_skips_unlock_passes_without_selected_technology(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = labs_body()
    status_call = """reportwin_draw_lab_status_rows(canvas, field_FAB8_, faction,
            field_A28_ + 3, field_A34_ + field_A2C_,
            field_59A8_, status_y, measured_height, draw_pass,
            status_child_count == 3,
            ReportIfGlobal.field_1E19C_, ReportIfGlobal.field_1E190_);"""
    assert body.count(status_call) == 1
    # Isolate the unlock phase: native status rendering is not under test here.
    body = body.replace(status_call, "status_y = 0;")
    harness = labs_harness(body)
    harness = harness.replace("w.draw_labs();\n    if(mutate_width_case)",
        "w.field_FAB8_=-1; w.draw_labs();\n"
        "    if(lab_unlock_calls!=0 || wrap_width_writes!=1) return 91;\n"
        "    for(int i=0;i<host_style_count;++i)\n"
        "        if(host_style_order[i]==1 || host_style_order[i]==3\n"
        "            || host_style_order[i]==4) return 92; return 0;\n"
        "    if(mutate_width_case)")
    source = tmp_path / "labs_no_selection.cpp"
    source.write_text(harness)
    executable = tmp_path / "labs_no_selection"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


def lab_button_helper():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("static void reportwin_init_lab_button(")
    return source[start:source.index("\n}\n", start) + 2]


def lab_status_helper():
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/reportwin.cpp").read_text()
    start = source.index("static void reportwin_draw_lab_status_rows(")
    return source[start:source.index("\n}\n", start) + 2]


def labs_harness(body):
    return r'''
#include <cstring>
#include <cstdio>
#include <stdint.h>
struct RECT { int left,top,right,bottom; };
struct BoxSpriteParams {} InterfaceBoxStyles[51];
struct Font {};
struct Win {};
struct Buffer;
int lab_icon_width=32, invalid_tech_sprite_indices;
int icon_draws, icon_x[8], icon_y[8], icon_color[8], icon_numerator[8], icon_denominator[8];
struct Sprite {
    unsigned char transparent_index() const { return 7; }
    int width() const { return lab_icon_width; }
    int draw_mono(Buffer *,int transparent,int x,int y,int color,int numerator,int denominator) {
        icon_x[icon_draws]=x; icon_y[icon_draws]=y; icon_color[icon_draws]=color;
        icon_numerator[icon_draws]=numerator; icon_denominator[icon_draws]=denominator;
        ++icon_draws;
        return transparent==7?0:1;
    }
};
struct SpriteArray {
    Sprite sprites[89];
    Sprite &operator[](int technology) {
        if(technology<0 || technology>=89) ++invalid_tech_sprite_indices;
        return sprites[technology<0 || technology>=89 ? 0 : technology];
    }
} g_TECH_ICON_SPRITES;
struct ButtonRecord { int id,x,y,width,height; Win *parent; char text[128]; };
ButtonRecord button_records[16]; int button_count, recolor_count, font_count;
int button_redraw_calls, button_stage_counts[16], button_stages[16][8];
int earned_children_case;
void button_stage(int record,int stage) {
    button_stages[record][button_stage_counts[record]++]=stage;
}
int mouse_pos_calls, mouse_feedback_calls, mouse_feedback_x[8], mouse_feedback_y[8];
int wrap_width, wrap_width_writes;
int host_measure_calls, host_draw_calls, host_wrap_calls;
int host_pass_order[4], host_pass_count, host_style_order[16], host_style_count;
Font status_font_a, status_font_b;
Font *status_font_first, *status_font_second;
int status_font_first_calls, status_font_second_calls;
struct FlatButton {
    int record, redraws;
    FlatButton() : record(-1), redraws(0) {}
    int init(char *name,int id,int x,int y,int width,int height,Win *parent,int) {
        record=button_count++;
        button_stage(record,1);
        button_records[record].id=id; button_records[record].x=x;
        button_records[record].y=y; button_records[record].width=width;
        button_records[record].height=height; button_records[record].parent=parent;
        std::strncpy(button_records[record].text,name,127);
        button_records[record].text[127]=0; return 0;
    }
    void set_font(Font *,Font *,Font *,Font *) { ++font_count; button_stage(record,2); }
    void set_text_color(int,int,int,int) { ++recolor_count; button_stage(record,4); }
    void set_border_styles(BoxSpriteParams *,BoxSpriteParams *,BoxSpriteParams *) { button_stage(record,5); }
    void get_mouse_pos(int *x,int *y) {
        ++mouse_pos_calls; *x=100+mouse_pos_calls; *y=200+mouse_pos_calls;
    }
    void on_mouse_feedback(int x,int y) {
        mouse_feedback_x[mouse_feedback_calls]=x;
        mouse_feedback_y[mouse_feedback_calls]=y;
        ++mouse_feedback_calls;
    }
    void redraw() { ++button_redraw_calls; button_stage(record,redraws++==0?3:6); }
};
struct RulesTechnology { int preq_tech_1,preq_tech_2; };
RulesTechnology Technology[89];
struct TutorialWindow { Font font1_; } TutWinBaseWindow;
bool has_tech(int technology,int) { return technology==3 || (earned_children_case && technology>=20 && technology<=22); }
int boxes, box_styles[64], box_rects[64][4], box_redraw_counts[64];
int box_uppercase_counts[64], uppercase_calls;
int clip_calls, working_clip_calls;
int clip_rects[16][4], energy_calls, energy_faction, rate_calls, writes;
int write_l_calls, write_l_x[8], write_l_y[8];
int centered_write_count, centered_x, centered_y, centered_width;
int wrap_heights, wrap_cents;
int mutate_width_case, observed_widths[8], observed_cent_widths[8];
uint32_t *live_frame_width;
char status_strings[8][64]; int status_string_count;
struct LineRecord { int a,b,c,d,e,f,g; } line_records[16];
int line_count;
RECT first_clip;
char written[1024], centered_text[512];
char *CharUpperA(char *s) {
    ++uppercase_calls;
    for(char *p=s;*p;++p) if(*p>='a' && *p<='z') *p-=32;
    return s;
}
struct Buffer : Win {
    RECT working;
    void set_clip(RECT *r) {
        if (clip_calls==0) first_clip=*r;
        clip_rects[clip_calls][0]=r->left; clip_rects[clip_calls][1]=r->top;
        clip_rects[clip_calls][2]=r->right; clip_rects[clip_calls][3]=r->bottom;
        if(r==&working) ++working_clip_calls;
        ++clip_calls;
    }
    void set_wrap_width(unsigned int width) {
        wrap_width=static_cast<int>(width); ++wrap_width_writes;
    }
    void set_font(Font *a,Font *,Font *,Font *) {
        if(a==status_font_first) ++status_font_first_calls;
        if(a==status_font_second) ++status_font_second_calls;
        host_style_order[host_style_count++]=a==status_font_first?5
            :a==status_font_second?6:1;
    }
    void box_sprite(RECT *r, BoxSpriteParams *style) {
        const int box=boxes;
        box_rects[boxes][0]=r->left; box_rects[boxes][1]=r->top;
        box_rects[boxes][2]=r->right; box_rects[boxes][3]=r->bottom;
        box_styles[boxes++]=static_cast<int>(style-InterfaceBoxStyles);
        box_redraw_counts[box]=button_redraw_calls;
        box_uppercase_counts[box]=uppercase_calls;
    }
    int write_l(char *s,int x,int y,int len) {
        write_l_x[write_l_calls]=x; write_l_y[write_l_calls]=y;
        ++write_l_calls; std::strncat(written,s,len);
        return x+len;
    }
    void set_text_color(int,int,int,int) { host_style_order[host_style_count++]=2; }
    void set_text_color2(int,int,int,int) { host_style_order[host_style_count++]=3; }
    void set_text_color_hyper(int,int,int,int) { host_style_order[host_style_count++]=4; }
    void write_cent_l(const char *s,RECT *,int) { ++writes; std::strcat(written,s); }
    int write_cent_l(char *s,int x,int y,int width,int len) {
        ++centered_write_count; centered_x=x; centered_y=y;
        centered_width=width; std::strncat(centered_text,s,len); return x;
    }
    int wrap_height(char *s,int width) {
        observed_widths[wrap_heights]=width;
        ++wrap_heights; std::strncpy(status_strings[status_string_count++],s,63);
        if(mutate_width_case && wrap_heights==1) *live_frame_width=104;
        return 10;
    }
    int wrap_cent(char *s,int,int y,int width) {
        observed_cent_widths[wrap_cents]=width;
        ++wrap_cents; std::strncpy(status_strings[status_string_count++],s,63);
        return y+10;
    }
    int wrap(char *,int,int y,int) { ++host_wrap_calls; return y+40; }
};
struct DatalinkFixture : Buffer {
    Font auxiliary_font10_, auxiliary_font11_;
    Font *entry_text_font() { return &auxiliary_font11_; }
    Font *entry_link_font() { return &auxiliary_font10_; }
    RECT entry;
    const RECT &entry_clip_rect() const { return entry; }
    RECT *buffer_working_rect() { return &working; }
    void construct_id(char *out,int category,int index) {
        std::sprintf(out,"%d",category*10000+index);
    }
} DatalinkMain;
void help_create_link(char *out,char *name,char *id) {
    std::strcat(out,"$LINK<"); std::strcat(out,name);
    std::strcat(out,"="); std::strcat(out,id); std::strcat(out,">");
}
int lab_unlock_calls;
void reportwin_draw_lab_unlock_sections(Buffer &canvas,int,bool,bool draw_pass,
                                        int x,int &y,uint32_t *,int &height) {
    ++lab_unlock_calls;
    host_pass_order[host_pass_count++]=draw_pass ? 1 : 0;
    if(draw_pass) { char unlock_text[]="Unlock"; canvas.wrap(unlock_text,x,y,100); ++host_draw_calls; y+=40; }
    else { height+=40; ++host_measure_calls; }
}
int spot_kill_calls, spot_add_calls, spot_left, spot_top, spot_length, spot_width;
struct SpotFixture {
    void kill_type(int type) { if(type==0) ++spot_kill_calls; }
    int add(int position,int type,int left,int top,int length,int width) {
        if(position==0 && type==0x29A) {
            ++spot_add_calls; spot_left=left; spot_top=top;
            spot_length=length; spot_width=width;
        }
        return 0;
    }
};
int IfaceIdToSlotStorage[1]={0}, ActiveIfaceIdStorage=0;
int datalink_case, blind_case, completed_case, wide_case, chart_case;
struct ReportIfFixture { int report_faction_id_, field_1E230_; Font *field_1E190_, *field_1E19C_; } ReportIfGlobal={0,0,0,0};
struct ConsoleFixture { int faction_id_; } ConsoleGlobal={0};
struct PlayerDataFixture { int tech_id_researching, tech_accumulated; } PlayersData[8]={{4,7}};
int GameRules=0, GameState=0, MaxTechnologyNum=89;
int lab_category_case=2;
int WinScreenWidth=800;
int EnergyLabsTotal=9, BreakthroughTurnsGlobal=5, TurnCurrentNum=30;
int mutate_wide_after_energy=1;
const int RULES_BLIND_RESEARCH=0x200;
char StringTemp[1024];
char *ScriptPopupFile=(char*)"SCRIPT";
int native_parse_ids[4], native_parse_values[4], native_parse_count;
int expected_progress_date=1053;
int parse_num(int id,int value) {
    native_parse_ids[native_parse_count]=id;
    native_parse_values[native_parse_count++]=value;
    return value;
}
void parse_it(char *,const char *section) {
    std::sprintf(StringTemp,"%s %d",section,
        native_parse_values[native_parse_count-1]);
}
unsigned int game_year(int turn) { return static_cast<unsigned int>(turn+1000); }
const char *label_get(int id) { return id==0xAC4/4?"Hidden":id==0x254/4?"Research":id==0x209+2?"Discover":"Other"; }
struct LabStringTable { const char *get(int id) { return id==0x20B?"Discover":"Category"; } } StringTable;
int tech_category(int) { return lab_category_case; }
int tech_recurse(int,int) { return 3; }
int mandate_color(int category) { return 0x70+category; }
void say_tech(char *out,int technology,int) { std::strcat(out,technology<0?"NONE":"TECH"); }
void energy_compute(int faction,int) {
    ++energy_calls;
    energy_faction=faction;
    if(mutate_wide_after_energy) ReportIfGlobal.field_1E230_=!ReportIfGlobal.field_1E230_;
}
int tech_rate(int) { ++rate_calls; return 9; }
void reportwin_append_number(char *out,int n) { char b[32]; std::sprintf(b,"%d",n); std::strcat(out,b); }
struct ReportWin : Buffer {
    RECT energy_panel_rect_, rect1_, report_panel_rect_, report_work_rect_;
    int field_59A0_,field_59A4_,field_E268_,field_FAB8_,field_A24_;
    uint32_t field_59A8_;
    int field_A28_,field_A2C_,field_A30_,field_A34_;
    int field_594C_,field_5950_,field_5954_,field_5958_,field_595C_,field_5960_;
    int field_5964_,field_5968_,field_596C_,field_5970_;
    FlatButton flatButtons1_[7];
    SpotFixture spot_;
    ReportWin():field_59A0_(0),field_59A4_(0),field_59A8_(0),field_E268_(12),field_FAB8_(4) {
        energy_panel_rect_.left=0; energy_panel_rect_.top=0;
        energy_panel_rect_.right=400; energy_panel_rect_.bottom=300;
        rect1_.left=10; rect1_.top=20; rect1_.right=400; rect1_.bottom=300;
        report_panel_rect_.right=423;
        working.left=1; working.top=2; working.right=399; working.bottom=298;
    }
    RECT *buffer_working_rect() { return &working; }
    void labs_line(int a,int b,int c,int d,int e,int f,int g) {
        LineRecord &line=line_records[line_count++];
        line.a=a; line.b=b; line.c=c; line.d=d;
        line.e=e; line.f=f; line.g=g;
    }
    void draw_labs();
};
''' + lab_category_style() + lab_child_collector() + lab_button_helper() + lab_status_helper() + body + r'''
int main() {
    DatalinkMain.entry.left=10; DatalinkMain.entry.top=20;
    DatalinkMain.entry.right=410; DatalinkMain.entry.bottom=320;
    DatalinkMain.working.left=1; DatalinkMain.working.top=2;
    DatalinkMain.working.right=399; DatalinkMain.working.bottom=298;
    if(datalink_case) IfaceIdToSlotStorage[0]=8;
    if(blind_case) GameRules=RULES_BLIND_RESEARCH;
    ConsoleGlobal.faction_id_=1;
    ReportIfGlobal.report_faction_id_=2;
    status_font_first=&status_font_a; status_font_second=&status_font_b;
    ReportIfGlobal.field_1E19C_=status_font_first;
    ReportIfGlobal.field_1E190_=status_font_second;
    PlayersData[2].tech_id_researching=4; PlayersData[2].tech_accumulated=7;
    if(completed_case) PlayersData[2].tech_id_researching=MaxTechnologyNum-1;
    for(int i=0;i<89;++i)
        Technology[i].preq_tech_1=Technology[i].preq_tech_2=-1;
    if(chart_case==1) {
        Technology[12].preq_tech_1=3; Technology[12].preq_tech_2=4;
        Technology[20].preq_tech_1=12; Technology[21].preq_tech_2=12;
        Technology[22].preq_tech_1=Technology[22].preq_tech_2=12;
        Technology[23].preq_tech_1=12;
    } else if(chart_case==2) {
        Technology[20].preq_tech_1=12;
        Technology[21].preq_tech_2=12;
        Technology[22].preq_tech_1=12;
    } else if(chart_case==3) {
        Technology[20].preq_tech_1=12;
        Technology[21].preq_tech_2=12;
    }
    if(chart_case==2) earned_children_case=1;
    ReportWin w;
    live_frame_width=&w.field_59A8_;
    if(chart_case==4) mutate_width_case=1;
    if(chart_case) w.field_FAB8_=12;
    w.draw_labs();
    if(mutate_width_case) {
        if(wrap_heights!=2 || wrap_cents!=2) return 101;
        if(observed_widths[0]!=184) return observed_widths[0];
        if(observed_widths[1]!=98) return observed_widths[1];
        if(observed_cent_widths[0]!=98) return 104;
        if(observed_cent_widths[1]!=98) return 105;
        if(w.field_59A8_!=104) return 106;
        return 0;
    }
    const int research_hidden=!datalink_case && blind_case && !(GameState&0x80);
    if(boxes!=(datalink_case?0:(research_hidden?6:6)+button_count+(chart_case==2?2:0)) || writes!=0 || energy_calls!=(datalink_case?0:1)
        || rate_calls!=(datalink_case?0:1)) return 1;
    if(!datalink_case && box_styles[0]!=3) return 41;
    if(!datalink_case) {
        const int chart_style_start=2;
        const int expected_chart_styles[]={44,42,42,44};
        for(int i=0;i<4;++i)
            if(box_styles[chart_style_start+i]!=expected_chart_styles[i]) return 52+i;
        if(box_rects[chart_style_start][0]!=13 || box_rects[chart_style_start][1]!=49
            || box_rects[chart_style_start][2]!=407 || box_rects[chart_style_start][3]!=87)
            return 58;
        if(box_rects[chart_style_start+1][2]!=319
            || box_rects[chart_style_start+2][0]!=312
            || box_rects[chart_style_start+2][1]!=42
            || box_rects[chart_style_start+2][2]!=329
            || box_rects[chart_style_start+2][3]!=94
            || box_rects[chart_style_start+3][0]!=313
            || box_rects[chart_style_start+3][1]!=43
            || box_rects[chart_style_start+3][2]!=328
            || box_rects[chart_style_start+3][3]!=93) return 59;
    }
    if(!datalink_case && box_rects[0][0]!=10) return 47;
    if(!datalink_case && box_rects[0][2]!=410) return 49;
    if(!datalink_case
        && (box_styles[1]!=1 || box_rects[1][0]!=13 || box_rects[1][1]!=23
            || box_rects[1][2]!=407 || box_rects[1][3]!=47)) return 47;
    if(spot_kill_calls!=(datalink_case?0:1)
        || spot_add_calls!=(datalink_case?0:1)) return 39;
    if(!datalink_case && (spot_left!=13 || spot_top!=23
        || spot_length!=394 || spot_width!=294)) return 40;
    if(!datalink_case && energy_faction!=2) return 38;
    if(mouse_pos_calls!=6 || mouse_feedback_calls!=6) return 27;
    if(wrap_heights!=2 || wrap_cents!=2 || status_string_count!=4)
        return 30;
    const int expected_icons=(datalink_case?0:!research_hidden)
        +(chart_case==2?1:0);
    if(icon_draws!=expected_icons) return 33;
    if(chart_case==2) {
        const int status_icon=(datalink_case || research_hidden)?0:1;
        if(icon_x[status_icon]!=w.field_A28_+3+(w.field_59A8_-0x34)/2
            || icon_y[status_icon]!=(w.field_A34_+w.field_A2C_-118)/2+23
            || icon_color[status_icon]!=0x72 || icon_numerator[status_icon]!=0x32
            || icon_denominator[status_icon]!=lab_icon_width) return 33;
    }
    if(!datalink_case && chart_case==2) {
        const int centered_top=(w.field_A34_+w.field_A2C_-118)/2-9;
        const int centered_bottom=(w.field_A34_+w.field_A2C_-118)/2+124;
        int found_centered_frame=0;
        for(int i=0;i<boxes;++i)
            if(box_styles[i]==3 && box_rects[i][1]==centered_top
                && box_rects[i][3]==centered_bottom) {
                if(box_uppercase_counts[i]<(datalink_case?1:2)) return 88;
                found_centered_frame=1;
            }
        if(!found_centered_frame) return 68;
    }
    if(!datalink_case && !research_hidden) {
        if(icon_x[0]!=13 || icon_y[0]!=23 || icon_color[0]!=0x72
            || icon_numerator[0]!=0x40 || icon_denominator[0]!=lab_icon_width) return 42;
        const int expected_writes=(PlayersData[2].tech_id_researching<0
            || completed_case)?1:2;
        if(write_l_calls!=expected_writes+3 || write_l_x[0]!=83 || write_l_y[0]!=28)
            return 43;
        if(expected_writes==2 && (write_l_x[1]!=95 || write_l_y[1]!=28))
            return 51;
        char expected_written[256];
        std::strcpy(expected_written,"[Research  ]");
        std::strcat(expected_written,completed_case?"[TECH 8]":"[TECH][TECH]]");
        std::strcat(expected_written,"DISCOV 5NEXTDISCOV 1053");
        if(std::strcmp(written,expected_written)) return 44;
        if(write_l_x[expected_writes]==0) return 63;
    } else if(write_l_calls!=(datalink_case?0:2)) return 45;
    if(research_hidden && (centered_write_count!=1 || centered_x!=83
        || centered_y!=28 || centered_width!=327
        || std::strcmp(centered_text,"[HiddenDISCOV"))) return 64;
    if(!research_hidden && centered_write_count!=0) return 65;
    if(!datalink_case) {
        const int progress_start=research_hidden?0:completed_case?2:3;
        if(write_l_x[progress_start]!=83 || write_l_y[progress_start]!=43
            || write_l_x[progress_start+1]!=95
            || write_l_y[progress_start+1]!=43) return 60;
        if(native_parse_count!=2 || native_parse_ids[0]!=0
            || native_parse_values[0]!=BreakthroughTurnsGlobal
            || native_parse_ids[1]!=0
            || native_parse_values[1]!=expected_progress_date)
            return 61;
    } else if(native_parse_count!=0) return 62;
    if(w.rect1_.left!=106 || w.rect1_.top!=206) return 34;
    if(std::strcmp(status_strings[0],"[TECH]")
        || std::strcmp(status_strings[1],"[Discover 3]")
        || std::strcmp(status_strings[2],"[TECH]")
        || std::strcmp(status_strings[3],"[Discover 3]")) return 31;
    if(wrap_width!=0 || wrap_width_writes!=(datalink_case?3:1)) return 29;
    for(int i=0;i<6;++i) {
        if(mouse_feedback_x[i]!=101+i || mouse_feedback_y[i]!=201+i)
            return 28;
    }
    if(lab_unlock_calls!=2 || host_measure_calls!=1 || host_draw_calls!=1
        || host_wrap_calls!=1 || host_pass_count!=2
        || host_pass_order[0]!=0 || host_pass_order[1]!=1
        || w.field_A24_!=118) return 26;
    if(datalink_case) {
        if(host_style_count<3 || host_style_order[host_style_count-3]!=1
            || host_style_order[host_style_count-2]!=2
            || host_style_order[host_style_count-1]!=4) return 66;
    } else if(host_style_order[host_style_count-2]!=2
              || host_style_order[host_style_count-1]!=3) return 67;
    if(status_font_first_calls!=2 || status_font_second_calls!=2) return 69;
    if(!datalink_case && ReportIfGlobal.field_1E230_==wide_case) return 35;
    if(datalink_case && ReportIfGlobal.field_1E230_!=wide_case) return 36;
    if(!datalink_case
        && w.field_A2C_!=20+0x63+(wide_case?0x62:0)) return 37;
    if(clip_calls!=(datalink_case?1:7)
        || working_clip_calls!=(datalink_case?1:4)) return 20;
    if(datalink_case) {
        if(first_clip.left!=1 || first_clip.top!=2
            || first_clip.right!=399 || first_clip.bottom!=298) return 25;
    } else if(first_clip.left!=13 || first_clip.top!=49
        || first_clip.right!=319 || first_clip.bottom!=87
        || clip_rects[2][0]!=319 || clip_rects[2][1]!=49
        || clip_rects[2][2]!=322 || clip_rects[2][3]!=87
        || clip_rects[4][0]!=320 || clip_rects[4][1]!=50
        || clip_rects[4][2]!=321 || clip_rects[4][3]!=86
        || clip_rects[clip_calls-1][0]!=1 || clip_rects[clip_calls-1][1]!=2
        || clip_rects[clip_calls-1][2]!=399 || clip_rects[clip_calls-1][3]!=298)
        return 25;
    if(research_hidden && std::strcmp(written,"DISCOV 5NEXTDISCOV 1053"))
        return 21;
    if(w.field_59A0_!=(datalink_case?0x96:0xBE)
        || w.field_59A4_!=0x28 || w.field_59A8_!=(datalink_case?0xAA:0xBE))
        return 3;
    if(datalink_case) {
        if(w.field_A28_!=125 || w.field_A30_!=295
            || w.field_A2C_!=53 || w.field_A34_!=293) return 4;
        if(w.rect1_.right!=394 || w.rect1_.bottom!=200
            || w.field_594C_!=18 || w.field_5950_!=125 || w.field_5954_!=252
            || w.field_5958_!=150 || w.field_595C_!=94 || w.field_5960_!=206
            || w.field_5964_!=66 || w.field_5968_!=122
            || w.field_596C_!=178 || w.field_5970_!=234) return 6;
    } else if(w.field_A28_!=115 || w.field_A30_!=305
              || w.field_A2C_!=(wide_case?217:119)
              || w.field_A34_!=(wide_case?391:293)) return 5;
    else if(w.rect1_.right!=400 || w.rect1_.bottom!=153) return 70;
    else if(w.field_594C_!=15 || w.field_5950_!=115 || w.field_5954_!=215) return 71;
    else if(w.field_5958_!=(wide_case?74:173)) return 81;
    else if(w.field_595C_!=(wide_case?18:117)) return 82;
    else if(w.field_5960_!=(wide_case?130:229)) return 83;
    else if(w.field_5964_!=(wide_case?-10:89)) return 84;
    else if(w.field_5968_!=(wide_case?46:145)) return 85;
    else if(w.field_596C_!=(wide_case?102:201)) return 86;
    else if(w.field_5970_!=(wide_case?158:257)) return 87;
    if(chart_case==1) {
        if(button_count!=6 || font_count!=6 || recolor_count!=1
            || line_count!=6) return 8;
        const int ids[]={0x379,0x37A,0x37B,0x37C,0x37D,0x37E};
        const int xs[]={15,15,215,215,215,215};
        const int ys[]={145,201,89,145,201,257};
        const char *names[]={"TECH","TECH","TECH","TECH","TECH","TECH"};
        for(int i=0;i<6;++i) {
            if(button_records[i].id!=ids[i] || button_records[i].x!=xs[i]
                || button_records[i].y!=ys[i] || button_records[i].width!=190
                || button_records[i].height!=40 || button_records[i].parent!=&w
                || std::strcmp(button_records[i].text,names[i])) return 9+i;
        }
        if(line_records[0].a!=115 || line_records[0].b!=173
            || line_records[0].c!=15 || line_records[0].d!=145
            || line_records[0].e!=190 || line_records[0].f!=40
            || line_records[1].d!=201) return 16;
    } else if(chart_case==2 || chart_case==3) {
        const int count=chart_case==2?3:2;
        const int xs[]={215,215,215};
        const int ys[]={chart_case==2?117:145,chart_case==2?173:201,
                        chart_case==2?229:0};
        if(button_count!=count || font_count!=count
            || recolor_count!=(chart_case==2?3:0)
            || line_count!=count) return 17;
        for(int i=0;i<count;++i) {
            if(button_records[i].id!=0x37B+i || button_records[i].x!=xs[i]
                || button_records[i].y!=ys[i] || button_records[i].width!=190
                || button_records[i].height!=40 || button_records[i].parent!=&w)
                return 18+i;
        }
        if(chart_case==2) {
            if(button_redraw_calls!=6) return 90;
            for(int i=0;i<3;++i) {
                if(button_stage_counts[i]!=6 || button_stages[i][0]!=1
                    || button_stages[i][1]!=2 || button_stages[i][2]!=3
                    || button_stages[i][3]!=4 || button_stages[i][4]!=5
                    || button_stages[i][5]!=6) return 91+i;
                const int expected_frame[]={212,ys[i]-3,408,ys[i]+43};
                int found=0;
                for(int box=0;box<boxes;++box) {
                    if(box_styles[box]==3
                        && box_rects[box][0]==expected_frame[0]
                        && box_rects[box][1]==expected_frame[1]
                        && box_rects[box][2]==expected_frame[2]
                        && box_rects[box][3]==expected_frame[3]) {
                        if(box_redraw_counts[box]!=(i+1)*2) return 94+i;
                        found=1;
                    }
                }
                if(!found) return 97+i;
            }
        }
    }
    return 0;
}
'''


@pytest.mark.parametrize("datalink,blind,completed,wide,chart,reveal",
                         [(False,False,False,False,False,False),(True,False,False,False,False,False),
                          (False,True,False,False,False,False),(False,False,True,False,False,False),
                          (False,False,False,True,False,False),(False,False,False,False,1,False),
                          (False,False,False,False,2,False),(False,False,False,False,3,False),
                          (False,False,False,False,4,False),(False,True,False,False,False,True)])
def test_production_draw_labs_host_path(tmp_path,datalink,blind,completed,wide,chart,reveal):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "labs.cpp"
    harness=labs_harness(labs_body())
    harness=harness.replace("int main() {", "int main() {\n    datalink_case=%d; blind_case=%d; completed_case=%d; wide_case=%d; chart_case=%d; GameState=%d;\n    ReportIfGlobal.field_1E230_=wide_case; WinScreenWidth=wide_case?1024:800;" %
                            (datalink,blind,completed,wide,chart,0x80 if reveal else 0))
    source.write_text(harness)
    executable = tmp_path / "labs"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("category",[0,1,2,3])
def test_production_draw_labs_category_style_dispatch(tmp_path,category):
    compiler=shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness=labs_harness(labs_body())
    harness=harness.replace("int main() {",
        "int main() {\n    lab_category_case=%d;" % category)
    harness=harness.replace("w.draw_labs();\n    if(mutate_width_case)",
        "w.draw_labs();\n    const int start=2; const int expected[]={38+3*lab_category_case,36+3*lab_category_case,36+3*lab_category_case,38+3*lab_category_case}; for(int j=0;j<4;++j) if(box_styles[start+j]!=expected[j]) return 98+j; if(icon_color[0]!=0x70+lab_category_case) return 104; return 0;\n    if(mutate_width_case)")
    source=tmp_path/"labs_category.cpp"
    source.write_text(harness)
    executable=tmp_path/"labs_category"
    compiled=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                            capture_output=True,text=True)
    assert compiled.returncode==0,compiled.stderr
    result=subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr


def test_production_draw_labs_research_icon_size_variant(tmp_path):
    compiler=shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness=labs_harness(labs_body())
    harness=harness.replace("int main() {", "int main() {\n    lab_icon_width=20;")
    source=tmp_path/"labs_icon_width.cpp"
    source.write_text(harness)
    executable=tmp_path/"labs_icon_width"
    compiled=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                            capture_output=True,text=True)
    assert compiled.returncode==0,compiled.stderr
    result=subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr


def test_production_draw_labs_no_research_caption_branch(tmp_path):
    compiler=shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness=labs_harness(labs_body())
    harness=harness.replace("PlayersData[2].tech_id_researching=4; PlayersData[2].tech_accumulated=7;",
        "PlayersData[2].tech_id_researching=-1; PlayersData[2].tech_accumulated=7;")
    harness=harness.replace("w.draw_labs();\n    if(mutate_width_case)",
        "w.draw_labs();\n    if(icon_draws!=0 || write_l_calls!=4 || std::strcmp(written,\"[Research  ][]DISCOV 5NEXTDISCOV 1053\") || write_l_x[1]!=95 || write_l_y[1]!=28) return 107; return 0;\n    if(mutate_width_case)")
    source=tmp_path/"labs_no_research.cpp"
    source.write_text(harness)
    executable=tmp_path/"labs_no_research"
    compiled=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                            capture_output=True,text=True)
    assert compiled.returncode==0,compiled.stderr
    result=subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr


def test_production_draw_labs_no_selected_technology_keeps_status_but_skips_unlocks(tmp_path):
    compiler=shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness=labs_harness(labs_body())
    harness=harness.replace("w.draw_labs();\n    if(mutate_width_case)",
        "w.field_FAB8_=-1; w.draw_labs();\n"
        "    if(lab_unlock_calls!=0 || wrap_heights!=2 || wrap_cents!=2\n"
        "        || status_string_count!=4 || status_font_first_calls!=2\n"
        "        || status_font_second_calls!=2\n"
        "        || std::strcmp(status_strings[0],\"[NONE]\")\n"
        "        || invalid_tech_sprite_indices!=0 || icon_draws!=1) return 90;\n"
        "    return 0;\n    if(mutate_width_case)")
    source=tmp_path/"labs_no_selected_technology.cpp"
    source.write_text(harness)
    executable=tmp_path/"labs_no_selected_technology"
    compiled=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                            capture_output=True,text=True)
    assert compiled.returncode==0,compiled.stderr
    result=subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("labs,accumulated,expected_date",
                         [(0,7,11029),(9,100,1030),(9,1,1119)])
def test_production_draw_labs_progress_turn_edges(tmp_path,labs,accumulated,expected_date):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    harness=labs_harness(labs_body())
    harness=harness.replace("PlayersData[2].tech_id_researching=4; PlayersData[2].tech_accumulated=7;",
        "PlayersData[2].tech_id_researching=4; PlayersData[2].tech_accumulated=%d; EnergyLabsTotal=%d; expected_progress_date=%d;" % (accumulated,labs,expected_date))
    harness=harness.replace("w.draw_labs();\n    if(mutate_width_case)",
        "w.draw_labs();\n    if(native_parse_count!=2 || native_parse_values[1]!=expected_progress_date) { std::fprintf(stderr,\"count=%d value=%d expected=%d\\n\",native_parse_count,native_parse_values[1],expected_progress_date); return 97; } return 0;\n    if(mutate_width_case)")
    source=tmp_path / "labs_progress.cpp"
    source.write_text(harness)
    executable=tmp_path / "labs_progress"
    compiled=subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                            capture_output=True,text=True)
    assert compiled.returncode==0,compiled.stderr
    result=subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr
