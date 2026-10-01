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
