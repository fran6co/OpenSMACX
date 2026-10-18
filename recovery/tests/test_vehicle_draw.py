"""Actual veh_draw behavior with bounded renderer/API doubles, not GUI/ABI proof.

Pinned 559865/559881 retain raw body coordinates; overlay offsets are separate.
559DBD onward consumes the RECT passed to black fill; 559DF6 publishes the font.
559F40 copies the complete label into its original four-byte scratch before
559FEC/559FF0 truncate it and 55A082 may duplicate brackets. Ordinary host
behavior cases use short labels; malformed-label controls spy on requested
source bytes while bounding host writes, not original adjacent-stack effects.
Record declarations are live, but host pointer widths and callback substitution
do not establish native layout, linked targets, or whole-game rendering.
"""

from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def clean(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if not depth:
            return text[start:end + 1]
    raise AssertionError("unterminated live declaration")


def body():
    text = clean((ROOT / "src/veh.cpp").read_text())
    match, = re.finditer(r"int\s+__cdecl\s+veh_draw\([^;{]+\)\s*\{", text)
    return braced(text, match.start())


def declarations():
    text = clean((ROOT / "src/veh.h").read_text())
    return "\n".join(braced(text, text.index("struct " + name + " {")) + ";"
                     for name in ("Veh", "VehPrototype", "RulesChassis", "RulesWeapon", "RulesOrder"))


HARNESS = r'''
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
void check(bool yes) { if(!yes) { std::fputs("behavior assertion\n",stderr); std::exit(91); } }
// DECLARATIONS
struct RECT { int left,top,right,bottom; };
bool same(RECT a,RECT b) { return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom; }
Veh Vehs[3]; VehPrototype VehPrototypes[32]; RulesChassis Chassis[19];
RulesWeapon Weapon[32]; RulesOrder Order[30];
struct { int faction_id_; } ConsoleGlobal;
struct Player { char search_key[64]; } Players[8];
int MapWinCaptionScale=-1,MapWinCaptionDivisor=-1;
int SpriteScaleNumerator=7,SpriteScaleDenominator=9,VehDrawSuppressedVehicle=-1;
struct GameDataFields {
    unsigned game_state_;
    struct { unsigned enable_mask_; } lock_masks_;
};
struct GameDataStorage { GameDataFields fields_; } GameDataStateStorage={};
int IsMultiplayerNet=0,InfoFactionTextColors[8],InfoFactionShadowColors[8];
unsigned char ShadowColorTable[256];
int scenario,zoom_value=0,flags_value=0,morale_value=3,stack_value=1,speed_value=6;
int expected_order=0,expected_faction=2;
std::string events,label;
struct CopyRequest { std::string source; std::size_t requested; };
std::vector<CopyRequest> copy_requests;
struct Draw { char kind; int index,x,y,key,n,d; };
std::vector<Draw> draws;
struct Fill { RECT rect; int color; };
std::vector<Fill> fills,boxes;
std::vector<int> colors;
RECT text_rect;
struct Font {} font;
Font *VehDrawFont=nullptr;
void record_copy(char *destination,const char *source,std::size_t requested,std::size_t capacity) {
    copy_requests.push_back({std::string(source,std::strlen(source)+1),requested});
    const std::size_t written=requested<capacity?requested:capacity;
    if(written) std::memcpy(destination,source,written);
}
void record_strcpy(char *destination,const char *source,std::size_t capacity) {
    record_copy(destination,source,std::strlen(source)+1,capacity);
}
const char *first_character(const char *source) {
    static char only[2]; only[0]=source[0]; only[1]=0; return only;
}
struct Buffer {
    void box(RECT*,int,int); void fill(RECT*,int);
    void set_font(Font*,int,int,int); void set_text_color(int,int,int,int);
    void write_cent_l(const char*,RECT*,int);
} buffer;
struct FontQueue { Font *get_zoom(int,int,int,int); } g_FONTQUEUE_VAL2;
struct Strings { char *get(char *p) { events+='T'; check(p==Order[expected_order].letter); return p; } } StringTable;
struct Sprite {
    unsigned char cTransparentIndex_;
    void draw(Buffer*,int,int,int); void draw_mono(Buffer*,int,int,int,int);
    void draw_dest(Buffer*,int,int,int,unsigned char*);
} g_VEH_SPRITES[152],g_FLAGS_VEH_SPRITES[112];
void sprite_call(Sprite *s,Buffer*b,int key,int x,int y,char kind) {
    check(b==&buffer && key==s->cTransparentIndex_);
    int index=kind=='D'?int(s-g_VEH_SPRITES):int(s-g_FLAGS_VEH_SPRITES);
    events+=kind; draws.push_back({kind,index,x,y,key,SpriteScaleNumerator,SpriteScaleDenominator});
}
void Sprite::draw(Buffer*b,int k,int x,int y) { sprite_call(this,b,k,x,y,'S'); }
void Sprite::draw_dest(Buffer*b,int k,int x,int y,unsigned char*t) { check(t==ShadowColorTable); sprite_call(this,b,k,x,y,'D'); }
void Sprite::draw_mono(Buffer*b,int k,int x,int y,int color) {
    check(color==InfoFactionShadowColors[expected_faction]); sprite_call(this,b,k,x,y,'H');
    if(scenario==3) { MapWinCaptionScale=2; MapWinCaptionDivisor=1; }
}
void Buffer::box(RECT*r,int a,int b) {
    check(this==&buffer && a==b); events+='Q'; boxes.push_back({*r,a});
    if(scenario==20) { *r={5,7,19,23}; InfoFactionTextColors[2]=99; }
}
void Buffer::fill(RECT*r,int c) {
    check(this==&buffer); events+='B'; fills.push_back({*r,c});
    if(scenario==2 && fills.size()==1) { *r={100,200,104,240}; Vehs[0].proto_id=1; VehPrototypes[1].reactor_id=2; Vehs[0].dmg_incurred=5; }
}
void Buffer::set_font(Font*f,int a,int b,int c) { check(this==&buffer && f==&font && VehDrawFont==f && a==0 && b==0 && c==0); events+='N'; }
void Buffer::set_text_color(int c,int a,int b,int d) {
    check(this==&buffer && a==-1 && b==1 && d==1); events+='C'; colors.push_back(c);
    if(scenario==14 && c==InfoFactionShadowColors[2]) ConsoleGlobal.faction_id_=5;
}
void Buffer::write_cent_l(const char*s,RECT*r,int len) {
    check(this==&buffer && len==int(std::strlen(s)) && len<=2); events+='W'; label=s; text_rect=*r;
}
Font *FontQueue::get_zoom(int z,int a,int b,int c) { check(this==&g_FONTQUEUE_VAL2 && z==zoom_value && a==1 && b==4 && c==1); events+='F'; return &font; }
int pick_top_veh(int id) { check(id==0); events+='P'; return 0; }
int morale_veh(int id,int a,int b) { check(id==0 && !a && !b); events+='M'; if(scenario==6) Vehs[0].proto_id=17; return morale_value; }
int morale_alien(int id,int faction) { check(id==0 && faction==ConsoleGlobal.faction_id_); events+='A'; return morale_value; }
int speed(int id,BOOL zero) { check(id==0 && !zero); events+='E'; if(scenario==7) Vehs[0].moves_expended=6; return speed_value; }
int stack_check(int id,int a,int b,int c,int d) { check(id==0 && a==1 && b==-1 && c==-1 && d==-1); events+='K'; return stack_value; }
void body_call(Buffer*b,int x,int y,int frame,int z,char kind) {
    check(b==&buffer && x==100 && y==200 && frame==7 && z==zoom_value);
    check(SpriteScaleNumerator==1 && SpriteScaleDenominator==1); events+=kind;
    if(scenario==1) { MapWinCaptionScale=2; MapWinCaptionDivisor=1; Vehs[0].faction_id=5; }
}
void vehdraw_draw(Buffer*b,int id,int x,int y,int f,int z,int frame) { check(id==0 && f==flags_value); body_call(b,x,y,frame,z,'V'); }
void vehdraw_turn(Buffer*b,int x,int y,int frame,int z) { body_call(b,x,y,frame,z,'R'); }
void vehdraw_scoot(Buffer*b,int x,int y,int frame,int z) { body_call(b,x,y,frame,z,'O'); }
// BODY
void expect_fill(unsigned i,RECT r,int c) { check(i<fills.size() && same(fills[i].rect,r) && fills[i].color==c); }
int main(int argc,char**argv) {
    check(argc==2); scenario=std::atoi(argv[1]);
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=255;
    ConsoleGlobal.faction_id_=2; Vehs[0].faction_id=2; Vehs[0].dmg_incurred=5;
    for(int i=0;i<32;++i) { VehPrototypes[i].icon_offset=-1; VehPrototypes[i].reactor_id=2; Weapon[i].icon=-1; }
    for(int i=0;i<19;++i) for(int j=0;j<8;++j) { Chassis[i].sprite_flag_x_off[j]=20; Chassis[i].sprite_flag_y_off[j]=12; }
    for(int i=0;i<152;++i) g_VEH_SPRITES[i].cTransparentIndex_=static_cast<unsigned char>(i+70);
    for(int i=0;i<112;++i) g_FLAGS_VEH_SPRITES[i].cTransparentIndex_=static_cast<unsigned char>(i+30);
    for(int i=0;i<8;++i) { InfoFactionTextColors[i]=80+i; InfoFactionShadowColors[i]=100+i; }
    for(int i=0;i<30;++i) Order[i].letter=const_cast<char*>("X");
    int expected_result=1,fx=120,fy=212,index=34;
    std::string expected_label="X";
    if(scenario==2) { VehPrototypes[0].reactor_id=1; Vehs[0].dmg_incurred=9; }
    if(scenario==3) { flags_value=2; stack_value=2; Chassis[0].sprite_flag_x_off[0]=60; fx=166; }
    if(scenario==4 || scenario==5) { Weapon[0].icon=scenario==4?0:1; expected_result=0; }
    if(scenario==6) { VehPrototypes[0].icon_offset=6; expected_result=0; }
    if(scenario==7) ++index;
    if(scenario==8) { Chassis[0].sprite_flag_x_off[0]=255; fx=355; }
    if(scenario==9) { Order[0].letter=const_cast<char*>("AB"); expected_label="A"; }
    if(scenario==10) { Order[0].letter=const_cast<char*>("[x]"); expected_label="[["; }
    if(scenario==11) { Order[0].letter=const_cast<char*>(""); expected_label=""; }
    if(scenario==12) { ConsoleGlobal.faction_id_=3; expected_label="-"; }
    if(scenario==13) { Order[0].letter=const_cast<char*>("["); Vehs[0].state=0x200; GameDataStateStorage.fields_.game_state_=0x1040; Vehs[0].move_to_ai_type='Z'; expected_label="Z["; }
    if(scenario==14) { Vehs[0].state=0x8000200; }
    if(scenario==15) { Vehs[0].state=0x8000200; Vehs[0].order_auto_type=10; expected_label="B"; }
    if(scenario==16) { Vehs[0].order=-128; Order[0].letter=const_cast<char*>("low"); expected_label="l"; }
    if(scenario==17) { Vehs[0].order=127; expected_order=29; Order[29].letter=const_cast<char*>("hig"); expected_label="h"; }
    if(scenario==18) { flags_value=1; VehDrawSuppressedVehicle=0; }
    if(scenario==19 || scenario==20) { zoom_value=-9; std::strcpy(Players[2].search_key,"SPARTANS"); }
    if(scenario==21) flags_value=0x40;
    if(scenario==22) { flags_value=0x800; expected_result=0; }
    if(scenario==23) flags_value=0x10;
    if(scenario==24) flags_value=0x20;
    if(scenario==25) flags_value=0x80;
    const int damage_boundaries[]={5,6,12,13};
    if(scenario>=26 && scenario<=29) Vehs[0].dmg_incurred=damage_boundaries[scenario-26];
    if(scenario==30 || scenario==31) { VehPrototypes[0].plan=12; Vehs[0].dmg_incurred=scenario-30; }
    if(scenario==32) { Vehs[0].state=0x8000000; Vehs[0].faction_id=3; ConsoleGlobal.faction_id_=3; expected_faction=3; index=48; }
    if(scenario==33) { Vehs[0].state=0x1004200; expected_label="P"; }
    if(scenario==34) { Vehs[0].state=0x200; Vehs[0].order_auto_type=11; expected_label="*"; }
    if(scenario==35) Order[0].letter=const_cast<char*>("Malformed overlong label");
    int result=veh_draw(&buffer,0,100,200,zoom_value,flags_value,7);
    if(scenario==35) {
        const std::string expected("Malformed overlong label\0",25);
        check(result==1 && copy_requests.size()==1);
        check(copy_requests[0].source==expected && copy_requests[0].requested==expected.size());
        return 0;
    }
    if(scenario==18) { check(result==0 && events=="P" && SpriteScaleNumerator==7 && SpriteScaleDenominator==9); return 0; }
    if(scenario==19 || scenario==20) {
        check(result==0 && events=="QB" && boxes.size()==1 && boxes[0].color==44 && fills.size()==1);
        check(same(boxes[0].rect,{114,206,129,221}));
        expect_fill(0,scenario==20?RECT{6,8,18,22}:RECT{115,207,128,220},scenario==20?99:82);
        check(SpriteScaleNumerator==7 && SpriteScaleDenominator==9); return 0;
    }
    check(result==expected_result && SpriteScaleNumerator==1 && SpriteScaleDenominator==1);
    if(scenario==21) { check(events=="MV" && draws.empty() && fills.empty()); return 0; }
    if(scenario==22 || scenario==25) check(events.find('V')==std::string::npos);
    if(scenario==23) check(events.find('R')!=std::string::npos);
    if(scenario==24) check(events.find('O')!=std::string::npos);
    if(scenario==4 || scenario==5 || scenario==6) {
        check(draws.size()==2 && draws[0].kind=='D' && draws[0].x==100 && draws[0].y==200);
        check(draws[0].index==(scenario==4?72:scenario==5?83:121));
    }
    if(scenario==3) { check(draws.size()==2 && draws[0].kind=='H' && draws[0].x==154 && draws[0].y==212 && draws[0].index==34); }
    check(!draws.empty()); Draw last=draws.back();
    check(last.kind=='S' && last.index==index && last.x==fx && last.y==fy);
    check(last.n==(scenario==1?2:16) && last.d==(scenario==1?1:16));
    check(label==expected_label && VehDrawFont==&font);
    int scale=(scenario==1 || scenario==3)?2:1;
    RECT black={fx+6*scale,fy+6*scale,fx+8*scale,fy+27*scale};
    expect_fill(0,black,0);
    if(scenario==31) check(fills.size()==1);
    else {
        check(fills.size()==2);
        int hp=scenario==30?1:20,damage=Vehs[0].dmg_incurred;
        RECT foreground=black;
        if(scenario==2) foreground={100,200,104,240};
        foreground.bottom=foreground.top+(foreground.bottom-foreground.top)*(hp-damage)/hp;
        // Fixed independently decoded threshold boundaries, not the source's clamp expressions.
        int color=scenario==27 || scenario==28?251:scenario==29?249:250;
        expect_fill(1,foreground,color);
    }
    check(same(text_rect,{fx+10*scale,fy+5*scale,fx+21*scale,fy+19*scale}));
    check(!colors.empty() && colors[0]==0);
    if(scenario==14) check(colors==std::vector<int>({0,102}));
    if(scenario==15) check(colors==std::vector<int>({0,102,255}));
    if(scenario==32) check(colors==std::vector<int>({0,250}));
    if(scenario==0) check(events=="MVESBBFNCTW");
    return 0;
}
'''


def compile_body(tmp_path, live):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "draw.cpp"
    source.write_text(HARNESS.replace("// DECLARATIONS", declarations()).replace("// BODY", live))
    exe = tmp_path / "draw"
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                             "-fno-sanitize-recover=undefined", str(source), "-o", str(exe)],
                            text=True, capture_output=True)
    assert result.returncode == 0, result.stderr
    return exe


def copy_spy_body(mutant=None):
    live = body()
    original = "strcpy(lineBuf, word);"
    assert live.count(original) == 1, "live full-label copy site changed"
    if mutant == "no-copy-first-character":
        declaration = "char lineBuf[4];"
        assert live.count(declaration) == 1
        live = live.replace(declaration,
                            "char lineBuf[4] = {word[0], 0, 0, 0};", 1)
        return live.replace(original, "", 1)
    replacements = {
        None: "record_strcpy(lineBuf, word, sizeof(lineBuf));",
        "prefix-copy": (
            "record_strcpy(lineBuf, first_character(word), sizeof(lineBuf));"),
        "shifted-source": "record_strcpy(lineBuf, word + 1, sizeof(lineBuf));",
        "truncating-copy": "record_copy(lineBuf, word, 1, sizeof(lineBuf));",
    }
    assert mutant in replacements
    return live.replace(original, replacements[mutant], 1)


@pytest.fixture(scope="module")
def executable(tmp_path_factory):
    return compile_body(tmp_path_factory.mktemp("vehicle-draw"), body())


@pytest.mark.parametrize("scenario", range(35))
def test_live_vehicle_draw(executable, scenario):
    result = subprocess.run([str(executable), str(scenario)], text=True, capture_output=True)
    assert result.returncode == 0, result.stderr


def test_malformed_label_copy_request_is_observed_with_bounded_host_write(tmp_path):
    exe = compile_body(tmp_path, copy_spy_body())
    result = subprocess.run([str(exe), "35"], text=True, capture_output=True)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutant", [
    "no-copy-first-character", "prefix-copy", "shifted-source", "truncating-copy",
])
def test_copy_request_witness_rejects_nonoriginal_copy_forms(tmp_path, mutant):
    exe = compile_body(tmp_path, copy_spy_body(mutant))
    result = subprocess.run([str(exe), "35"], text=True, capture_output=True)
    assert result.returncode == 91 and "behavior assertion" in result.stderr


def test_pinned_original_order_label_copy_call_and_later_byte_uses():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else []) + [
        root / ".opensmacx/game/terranx_original.exe"
        for root in (ROOT, *ROOT.parents)
    ]
    image_path = next((path for path in candidates if path.is_file()), None)
    if image_path is None:
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN_SHA256
    image = PeImage(raw, PIN_SHA256)
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    full_span = Span(0x005594F0, 0x0055A14E)
    full_rows = list(engine.disasm(image.read(full_span), full_span.start))
    assert full_rows and full_rows[0].address == full_span.start
    assert sum(row.size for row in full_rows) == full_span.end - full_span.start
    assert full_rows[-1].address + full_rows[-1].size == full_span.end
    boundaries = {row.address: row for row in full_rows}

    def decoded(start, end):
        assert start in boundaries
        assert end == full_span.end or end in boundaries
        rows = [row for row in full_rows if start <= row.address < end]
        assert rows and rows[0].address == start
        assert rows[-1].address + rows[-1].size == end
        return {row.address: (row.mnemonic, row.op_str) for row in rows}

    copy = decoded(0x00559F2E, 0x00559F50)
    assert copy == {
        0x00559F2E: ("mov", "ecx, dword ptr [eax*4 + 0x96c880]"),
        0x00559F35: ("push", "ecx"),
        0x00559F36: ("mov", "ecx, 0x9b90d8"),
        0x00559F3B: ("call", "0x6169a0"),
        0x00559F40: ("lea", "edx, [ebp - 4]"),
        0x00559F43: ("push", "eax"),
        0x00559F44: ("push", "edx"),
        0x00559F45: ("call", "0x645460"),
        0x00559F4A: ("mov", "esi, dword ptr [ebp - 0x14]"),
        0x00559F4D: ("add", "esp, 8"),
    }
    clears = decoded(0x00559FEC, 0x00559FF4)
    assert clears == {
        0x00559FEC: ("mov", "byte ptr [ebp - 2], 0"),
        0x00559FF0: ("mov", "byte ptr [ebp - 3], 0"),
    }
    uses = decoded(0x0055A075, 0x0055A085)
    assert uses == {
        0x0055A075: ("mov", "cl, byte ptr [ebp - 4]"),
        0x0055A078: ("cmp", "cl, 0x5b"),
        0x0055A07B: ("je", "0x55a082"),
        0x0055A07D: ("cmp", "cl, 0x5d"),
        0x0055A080: ("jne", "0x55a085"),
        0x0055A082: ("mov", "byte ptr [ebp - 3], cl"),
    }


@pytest.mark.parametrize("old,new,scenario", [
    ("vehdraw_draw(buffer, veh_id, screen_x, screen_y,", "vehdraw_draw(buffer, veh_id, x, y,", 0),
    ("sprite_category == 12 || sprite_category == 10", "sprite_category == 12 || sprite_category == 9", 5),
    ("strcpy(lineBuf, word);", "strcpy(lineBuf, word + 1);", 9),
    ("lineBuf[1] = lineBuf[0];", "lineBuf[1] = 0;", 10),
    ("VehDrawFont = g_FONTQUEUE_VAL2.get_zoom(zoom, 1, 4, 1);", "g_FONTQUEUE_VAL2.get_zoom(zoom, 1, 4, 1);", 0),
    ("if (faction == (unsigned int)ConsoleGlobal.faction_id_ &&\n                (Vehs[veh_id].state & 0x8000000)", "if (true &&\n                (Vehs[veh_id].state & 0x8000000)", 14),
    ("x += (MapWinCaptionScale * 6) / MapWinCaptionDivisor;", "x += 6;", 3),
    ("(r2.bottom - r2.top) * remaining_health / maximum_health + r2.top", "(rr24 - rr2c) * remaining_health / maximum_health + rr2c", 2),
])
def test_behavior_damage(tmp_path, old, new, scenario):
    live = body()
    assert old in live
    damaged = live.replace(old, new, 1)
    exe = compile_body(tmp_path, damaged)
    result = subprocess.run([str(exe), str(scenario)], text=True, capture_output=True)
    assert result.returncode == 91 and "behavior assertion" in result.stderr, result.stderr
