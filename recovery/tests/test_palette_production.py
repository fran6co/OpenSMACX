"""Actual palette producers with bounded API, storage and exception doubles.

No game/VC6/EH-table acceptance is implied. File mappings are audited so the
original unchecked mandatory copies can be observed without crashing the host;
an audit exception is a test signal, not a newly invented product fallback.
Required text sections are supplied; their original uninitialized failure path
is deliberately not executed. Optional-section defaults are exercised.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source")


def live_source():
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", (ROOT / "src/game.cpp").read_text(), flags=re.S)
    bodies = []
    for signature in ("int __cdecl compare_palettes()", "void __cdecl colortables_init()"):
        assert text.count(signature) == 1
        bodies.append(braced(text,text.index(signature)))
    names = ("ShadowColorTable", "ArmorTransparentColorTables", "TerritoryColorTables",
             "ReddishColorTable", "RedTintColorTable", "FadeColorTables", "BlueTintColorTable",
             "ExplosionColorTables", "SpotlightColorTable", "ConstantColorTables",
             "ArmorTintColorTables", "BodyTintColorTables")
    definitions = []
    for name in names:
        definition, = re.findall(r"unsigned char " + name + r"(?:\[\d+\])+;",text)
        definitions.append(definition)
    header = (ROOT / "src/jackal/palette.h").read_text()
    getter = braced(header,header.index("const PALETTEENTRY *entries() const"))
    return "\n".join(definitions), "\n\n".join(bodies), getter


HARNESS = r"""
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <limits>
#include <string>
#include <vector>
#define __cdecl
void check_at(bool ok,int line) {
    if (!ok) { std::fprintf(stderr,"assertion at generated line %d\n",line); std::exit(91); }
}
#define check(ok) check_at((ok),__LINE__)
typedef unsigned char BYTE;
struct PALETTEENTRY { BYTE peRed,peGreen,peBlue,peFlags; };
struct HSV { double h,s,v; };
struct Fault {};
bool tracking=false,optional_defaults=false;
std::vector<std::string> events;
std::map<std::string,std::vector<unsigned char> > files;
std::map<std::string,std::vector<int> > config;
std::string fail_create,throw_site,section;
int token,line,file_id,table_calls,blend_calls,nearest_calls,hsv_calls,pixels,record_reads;
int tint_pixel;
PALETTEENTRY current_tint;
unsigned char expected_body[8][24][256],expected_armor[4][24][256];
bool tint_is_armor;
int tint_ordinal;
void check_section();
void event(const std::string &e) {
    if (tracking) events.push_back(e);
    if (e==throw_site) throw Fault();
}
std::string number(int value) { char text[32]; std::sprintf(text,"%d",value); return text; }
char *_itoa(int value,char *out,int radix) { check(radix==10); std::sprintf(out,"%d",value); return out; }
char StringTemp[1032];
// TABLES
static_assert(sizeof(ShadowColorTable)==256 && sizeof(SpotlightColorTable)==256 &&
              sizeof(ReddishColorTable)==256 && sizeof(RedTintColorTable)==256 &&
              sizeof(BlueTintColorTable)==256 && sizeof(FadeColorTables)==2048,
              "original six basic cache extents");
static_assert(sizeof(ExplosionColorTables)==0xD000 && sizeof(TerritoryColorTables)==0x800 &&
              sizeof(BodyTintColorTables)==0xC000 && sizeof(ArmorTintColorTables)==0x6000 &&
              sizeof(ConstantColorTables)==0x1800 && sizeof(ArmorTransparentColorTables)==0x1800,
              "original row and cache extents");
struct Palette {
    PALETTEENTRY entries_[256];
    Palette() { if (tracking) event("+Palette"); std::memset(entries_,0,sizeof(entries_)); }
    ~Palette() { if (tracking) event("-Palette"); }
    // GETTER
    int create_table(unsigned char *,int,int,int);
    int create_table_from_color(int,unsigned char *,int,int,int,int);
    int get_nearest_palette_index(unsigned char,unsigned char,unsigned char,int);
} g_PALETTE1;
struct Filemap {
    int id; std::string active;
    Filemap():id(++file_id) { event("+File"+number(id)); }
    ~Filemap() { event("-File"+number(id)); }
    void *open(const char *name,int sequential) {
        check(sequential==1); active=name; event("open:"+active);
        return files.count(active) ? files[active].data() : 0;
    }
    void *create(const char *name,unsigned size,int sequential) {
        check(sequential==1); active=name; event("create:"+active+":"+number(size));
        if (active==fail_create) return 0;
        files[active].assign(size,0xD7); return files[active].data();
    }
    void close() { event("close:"+active); active.clear(); }
    void close(void *end) {
        check(!active.empty());
        unsigned char *base=files[active].data();
        const std::ptrdiff_t length=static_cast<unsigned char *>(end)-base;
        check(active=="colorvals.tmp" && length==0x710);
        event("truncate:"+active+":"+number(int(length)));
        files[active].resize(size_t(length)); active.clear();
    }
};
struct Buffer {
    Buffer() { event("+Buffer"); }
    ~Buffer() { event("-Buffer"); }
    int load_pcx(const char *name,Palette *palette,int start,int count) {
        check_section();
        check(std::string(name)=="palette.PCX" && palette==0 && start==10 && count==236);
        event("pcx"); return 0;
    }
    int get_pixel(int x,int y) {
        check(x==580+8*pixels && y==56 && pixels<8);
        event("pixel"); return 90+pixels++;
    }
};
struct CvrRecord { unsigned char *palette_; } model_record;
unsigned char model_palette[2][0x302+24*256];
struct CaviarData {
    const CvrRecord *record() const { ++record_reads; return &model_record; }
} g_VBP_CAVIARDATA;
int tokens_per_line() {
    return section=="energyglow" || section.compare(0,10,"explosions")==0 ? 2 : section=="ARMORCOLORING_TRANS" ? 3 : 1;
}
void check_section() {
    if (section.empty()) return;
    if (!config.count(section)) { check(token==0 && line==0); return; }
    check(token==int(config[section].size()) && line*tokens_per_line()==token);
}
int text_open(const char *name,const char *label) {
    check_section();
    check(std::string(name)=="system"); section=label; token=line=0;
    event("text:"+section); return config.count(section) ? 0 : 1;
}
char *text_get() {
    check(token==line*tokens_per_line());
    ++line; event("text_get"); return StringTemp;
}
int text_item_number() {
    check(config.count(section) && token<int(config[section].size()));
    check(line==token/tokens_per_line()+1);
    return config[section][token++];
}
int body_index(int i) {
    const int defaults[8]={96,105,156,0,85,47,89,227};
    return optional_defaults ? defaults[i] : (unsigned(config["VEHICLECOLORING"][i])+10u)&255u;
}
int armor_index(int i) { return optional_defaults ? 0 : (unsigned(config["ARMORCOLORING"][i])+10u)&255u; }
void RGB_to_HSV(const PALETTEENTRY *entry,HSV *out) {
    check(entry!=g_PALETTE1.entries());
    // A body regeneration always precedes armor. Armor-only regeneration has
    // its own first index, selected by whether body cache was just opened.
    bool armor_only=std::find(events.begin(),events.end(),"open:bodygrads.tmp")!=events.end();
    const int index=armor_only ? armor_index(hsv_calls) : hsv_calls<8 ? body_index(hsv_calls) : armor_index(hsv_calls-8);
    tint_is_armor=armor_only || hsv_calls>=8;
    tint_ordinal=armor_only ? hsv_calls : tint_is_armor ? hsv_calls-8 : hsv_calls;
    check(std::memcmp(entry,&g_PALETTE1.entries_[index],4)==0);
    current_tint=*entry; tint_pixel=0; ++hsv_calls;
    out->h=out->s=out->v=0;
}
int Palette::get_nearest_palette_index(unsigned char r,unsigned char g,unsigned char b,int skip) {
    check(this==&g_PALETTE1 && skip==0 && tint_pixel<24*256);
    const unsigned char index=model_record.palette_[0x302+tint_pixel];
    const PALETTEENTRY &source=entries_[index];
    // Preserve the original integer products and float-valued coefficient
    // in the independent oracle; this does not prove x87 scheduling.
    const double coefficient=double(1.0f/255.0f);
    check(r==static_cast<unsigned char>(int(double(current_tint.peRed)*source.peRed*coefficient)));
    check(g==static_cast<unsigned char>(int(double(current_tint.peGreen)*source.peGreen*coefficient)));
    check(b==static_cast<unsigned char>(int(double(current_tint.peBlue)*source.peBlue*coefficient)));
    event("nearest"); ++nearest_calls;
    const unsigned char mapped=static_cast<unsigned char>(r+2*g+3*b+7);
    if (tint_is_armor) expected_armor[tint_ordinal][tint_pixel/256][tint_pixel%256]=mapped;
    else expected_body[tint_ordinal][tint_pixel/256][tint_pixel%256]=mapped;
    // Each 256-byte row reloads the Caviar record/palette pointer. Allocations
    // are separate from the record; treating that pointer as an offset fails.
    if (++tint_pixel%256==0)
        model_record.palette_=model_record.palette_==model_palette[0] ? model_palette[1] : model_palette[0];
    return mapped+256;
}
int Palette::create_table(unsigned char *table,int start,int count,int amount) {
    check(this==&g_PALETTE1 && start==0 && count==256);
    check((table==ShadowColorTable && amount==-43)||(table==SpotlightColorTable && amount==50));
    ++table_calls;
    for (int i=0;i<256;++i) table[i]=static_cast<unsigned char>(i+amount);
    return 0;
}
int Palette::create_table_from_color(int color,unsigned char *table,int start,int count,int keep,int weight) {
    check(this==&g_PALETTE1); ++blend_calls;
    bool located=false;
    if (table==ReddishColorTable) { check(color==131 && start==0 && count==256 && keep==75 && weight==50); located=true; }
    if (table==RedTintColorTable) { check(color==34 && start==0 && count==256 && keep==20 && weight==80); located=true; }
    if (table==BlueTintColorTable) { check(color==179 && start==0 && count==256 && keep==20 && weight==80); located=true; }
    for (int i=0;i<8;++i) {
        if (table==FadeColorTables[i]) { check(color==255 && start==0 && count==256 && keep==100 && weight==100-12*i); located=true; }
        if (table==TerritoryColorTables[i]) {
            const int expected=optional_defaults ? 80 : config["boundarypercents"][i];
            check(color==90+i && weight==expected); located=true;
        }
    }
    for (int e=0;e<13;++e) for (int row=0;row<16;++row) if (table==ExplosionColorTables[e][row]) {
        const std::vector<int> &values=config["explosions"+number(e)];
        check(color==values[2*row]+10 && weight==values[2*row+1]); located=true;
    }
    for (int row=0;row<24;++row) if (table==ArmorTransparentColorTables[row]) {
        const int first=optional_defaults ? 50 : config["ARMORCOLORING_TRANS"][1];
        const int second=optional_defaults ? 100 : config["ARMORCOLORING_TRANS"][2];
        check(color==(optional_defaults ? 0 : (unsigned(config["ARMORCOLORING_TRANS"][0])+10u)&255u));
        check(weight==((24-row)*first+row*second)/24);
        if (first==50 && second==100) {
            const int expected[24]={50,52,54,56,58,60,62,64,66,68,70,72,75,77,79,81,83,85,87,89,91,93,95,97};
            check(weight==expected[row]);
        }
        if (first==-1 && second==0) check(weight==(row==0 ? -1 : 0));
        located=true;
    }
    check(located);
    if (start!=0) check(start==10 && count==236 && keep==100-weight);
    for (int i=0;i<256;++i) table[i]=static_cast<unsigned char>(color+3*weight+5*keep+i);
    return 0;
}
void audit(const void *pointer,size_t count) {
    if (!pointer) { event("invalid-copy"); throw Fault(); }
    const std::uintptr_t p=reinterpret_cast<std::uintptr_t>(pointer);
    for (std::map<std::string,std::vector<unsigned char> >::const_iterator it=files.begin();it!=files.end();++it) {
        const std::uintptr_t base=reinterpret_cast<std::uintptr_t>(it->second.data());
        if (p>=base && p<=base+it->second.size() && count>base+it->second.size()-p) {
            event("short-copy"); throw Fault();
        }
    }
}
void *audited_copy(void *out,const void *in,size_t count) {
    audit(out,count); audit(in,count); return std::memcpy(out,in,count);
}
int audited_compare(const void *a,const void *b,size_t count) {
    audit(a,count); audit(b,count); return std::memcmp(a,b,count);
}
#define memcpy audited_copy
#define memcmp audited_compare
// BODIES
#undef memcpy
#undef memcmp

int count_event(const std::string &s) { return int(std::count(events.begin(),events.end(),s)); }
void reset_observations() {
    events.clear(); file_id=table_calls=blend_calls=nearest_calls=hsv_calls=pixels=record_reads=0;
    throw_site.clear(); fail_create.clear(); section.clear(); token=line=0; model_record.palette_=model_palette[0];
}
void initialize() {
    tracking=false; files.clear(); config.clear(); optional_defaults=false;
    for (int i=0;i<256;++i) g_PALETTE1.entries_[i]=PALETTEENTRY{BYTE(i),BYTE(3*i+7),BYTE(5*i+11),BYTE(i^0xE0)};
    for (int bank=0;bank<2;++bank) for (int i=0;i<0x302+24*256;++i)
        model_palette[bank][i]=BYTE((i<0x302 ? 37 : (i-0x302)/256*17+(i-0x302)%256*3+129)+bank*31);
    config["energyglow"]={2,30,3,40,4,50};
    for (int e=0;e<13;++e) for (int r=0;r<16;++r) {
        config["explosions"+number(e)].push_back(e*16+r);
        config["explosions"+number(e)].push_back((e+r)%80);
    }
    config["VEHICLECOLORING"]={-10,119,245,0,5,240,1,2};
    config["ARMORCOLORING"]={245,119,-10,5};
    config["ARMORCOLORING_TRANS"]={245,30,70};
    config["boundarypercents"]={20,30,40,50,60,70,80,90};
    reset_observations(); tracking=true;
}
void teardown() {
    check(events.size()>=3);
    check(events[events.size()-3]=="-Buffer" && events[events.size()-2]=="-File1" && events.back()=="-Palette");
    check(count_event("-Buffer")==1 && count_event("-File1")==1 && count_event("-Palette")==1);
}
void tables_equal_files() {
    check(files["colorvals.tmp"].size()==0x710);
    for (int e=0;e<13;++e) check(std::memcmp(ExplosionColorTables[e],files["explosions"+number(e)+".tmp"].data(),4096)==0);
    check(std::memcmp(TerritoryColorTables,files["territory.tmp"].data(),2048)==0);
    check(std::memcmp(BodyTintColorTables,files["bodygrads.tmp"].data(),0xC000)==0);
    check(std::memcmp(ArmorTintColorTables,files["armorgrads.tmp"].data(),0x6000)==0);
    check(std::memcmp(BodyTintColorTables,expected_body,sizeof(expected_body))==0);
    check(std::memcmp(ArmorTintColorTables,expected_armor,sizeof(expected_armor))==0);
    check(std::memcmp(ArmorTransparentColorTables,files["armorgrads_trans.tmp"].data(),0x1800)==0);
    for (int row=0;row<24;++row) for (int c=0;c<256;++c) check(ConstantColorTables[row][c]==row);
}
void prime() {
    colortables_init(); teardown(); tables_equal_files();
    check(table_calls==2 && blend_calls==251 && nearest_calls==73728 && hsv_calls==12 && record_reads==288);
    reset_observations();
}
void producer_case(const std::string &scenario) {
    initialize();
    if (scenario=="cold" || scenario=="defaults" || scenario=="indices_wrap" || scenario=="negative_transparent") {
        if (scenario=="negative_transparent") {
            config["ARMORCOLORING_TRANS"][1]=-1;
            config["ARMORCOLORING_TRANS"][2]=0;
        }
        if (scenario=="defaults") {
            optional_defaults=true;
            config.erase("VEHICLECOLORING"); config.erase("ARMORCOLORING");
            config.erase("ARMORCOLORING_TRANS"); config.erase("boundarypercents");
        }
        if (scenario=="indices_wrap") {
            config["VEHICLECOLORING"][0]=std::numeric_limits<int>::max();
            config["VEHICLECOLORING"][1]=std::numeric_limits<int>::min();
            config["ARMORCOLORING"][0]=std::numeric_limits<int>::max();
            config["ARMORCOLORING"][1]=std::numeric_limits<int>::min();
            config["ARMORCOLORING_TRANS"][0]=std::numeric_limits<int>::max();
        }
        prime(); return;
    }
    prime();
    const char *basic[]={"shadow.tmp","spotlight.tmp","reddish.tmp","redtint.tmp","bluetint.tmp","fade.tmp"};
    if (scenario=="palette_changed") g_PALETTE1.entries_[255].peFlags^=1;
    else if (scenario.compare(0,6,"basic:")==0) files.erase(basic[std::atoi(scenario.c_str()+6)]);
    else if (scenario.compare(0,7,"header:")==0) files["colorvals.tmp"][std::atoi(scenario.c_str()+7)]^=1;
    else if (scenario.compare(0,10,"explosion:")==0) files.erase("explosions"+scenario.substr(10)+".tmp");
    else if (scenario=="territory") files.erase("territory.tmp");
    else if (scenario=="armor") files.erase("armorgrads.tmp");
    else if (scenario=="transparent") files.erase("armorgrads_trans.tmp");
    else if (scenario=="omitted_weight") config["ARMORCOLORING_TRANS"][2]=99;
    else if (scenario=="body_missing") files.erase("bodygrads.tmp");
    else if (scenario=="short_palette") files["palcheck.tmp"].resize(13);
    else if (scenario.compare(0,6,"throw:")==0) {
        throw_site=scenario.substr(6);
        if (throw_site=="nearest") files.erase("colorvals.tmp");
    }
    else if (scenario=="guarded_create") { files.erase("shadow.tmp"); fail_create="shadow.tmp"; }
    else if (scenario=="mandatory_create") { files.erase("colorvals.tmp"); fail_create="explosions0.tmp"; }
    bool fault=false;
    try { colortables_init(); } catch (const Fault &) { fault=true; }
    if (throw_site=="+File1") {
        check(events.back()=="-Palette" && count_event("-File1")==0 && count_event("+Buffer")==0);
    } else if (throw_site=="+Buffer") {
        check(events[events.size()-2]=="-File1" && events.back()=="-Palette" && count_event("-Buffer")==0);
    } else teardown();
    if (scenario=="body_missing" || scenario=="short_palette" || scenario=="mandatory_create" || scenario.compare(0,6,"throw:")==0) {
        check(fault);
        if (scenario=="body_missing" || scenario=="mandatory_create") check(count_event("invalid-copy")==1);
        if (scenario=="short_palette") check(count_event("short-copy")==1);
        return;
    }
    check(!fault); tables_equal_files();
    const bool rebuild= scenario=="palette_changed" || scenario=="guarded_create" ||
        scenario.compare(0,6,"basic:")==0 || scenario.compare(0,7,"header:")==0 || scenario.compare(0,10,"explosion:")==0;
    if (rebuild) {
        check(nearest_calls==73728 && hsv_calls==12 && record_reads==288);
        check(count_event("open:territory.tmp")==0 && count_event("create:territory.tmp:2048")==1);
        check(count_event("open:bodygrads.tmp")==0 && count_event("open:armorgrads.tmp")==0);
        check(count_event("open:armorgrads_trans.tmp")==0);
        for (int e=0;e<13;++e) check(count_event("create:explosions"+number(e)+".tmp:4096")==1);
    } else if (scenario=="armor") {
        check(nearest_calls==24576 && hsv_calls==4 && record_reads==96 && blend_calls==24);
        check(count_event("open:bodygrads.tmp")==1 && count_event("open:armorgrads_trans.tmp")==0);
    } else {
        check(nearest_calls==0 && hsv_calls==0 && record_reads==0 && table_calls==0);
        check(blend_calls==(scenario=="territory" ? 8 : scenario=="transparent" ? 24 : 0));
        check(count_event("open:bodygrads.tmp")==1 && count_event("open:armorgrads.tmp")==1 && count_event("open:armorgrads_trans.tmp")==1);
    }
    if (scenario=="palette_changed") for (int i=0;i<6;++i) check(count_event("open:"+std::string(basic[i]))==0);
    if (scenario=="warm" || scenario=="omitted_weight") {
        std::vector<std::string> wanted,observed;
        wanted.push_back("open:palcheck.tmp");
        for (int i=0;i<6;++i) wanted.push_back("open:"+std::string(basic[i]));
        wanted.push_back("open:colorvals.tmp");
        for (int i=0;i<13;++i) wanted.push_back("open:explosions"+number(i)+".tmp");
        wanted.push_back("open:territory.tmp"); wanted.push_back("open:bodygrads.tmp");
        wanted.push_back("open:armorgrads.tmp"); wanted.push_back("open:armorgrads_trans.tmp");
        for (size_t i=0;i<events.size();++i) if (events[i].compare(0,5,"open:")==0) observed.push_back(events[i]);
        check(observed==wanted);
    }
    if (scenario.compare(0,6,"basic:")==0) {
        const int missing=std::atoi(scenario.c_str()+6);
        for (int i=0;i<6;++i) check(count_event("open:"+std::string(basic[i]))==(i<=missing ? 1 : 0));
        check(table_calls==2 && blend_calls==251);
    }
    if (scenario.compare(0,10,"explosion:")==0) {
        const int missing=std::atoi(scenario.c_str()+10);
        for (int i=0;i<13;++i) check(count_event("open:explosions"+number(i)+".tmp")== (i<=missing ? 1 : 0));
        check(count_event("create:colorvals.tmp:2048")==0);
    }
}
void compare_case(const std::string &scenario) {
    initialize();
    if (scenario!="compare_missing" && scenario!="compare_create_failure")
        files["palcheck.tmp"]=std::vector<unsigned char>(reinterpret_cast<unsigned char *>(g_PALETTE1.entries_),reinterpret_cast<unsigned char *>(g_PALETTE1.entries_)+1024);
    if (scenario=="compare_changed") files["palcheck.tmp"][1023]^=1;
    if (scenario=="compare_create_failure") fail_create="palcheck.tmp";
    const int result=compare_palettes();
    check(result==(scenario=="compare_equal" ? 0 : 1));
    check(events.front()=="+File1" && events.back()=="-File1");
    check(count_event("close:palcheck.tmp")== (scenario=="compare_equal" ? 0 : 1));
    if (scenario!="compare_create_failure") check(std::memcmp(files["palcheck.tmp"].data(),g_PALETTE1.entries_,1024)==0);
}
int main(int argc,char **argv) {
    check(argc==2); const std::string scenario=argv[1];
    if (scenario.compare(0,8,"compare_")==0) compare_case(scenario); else producer_case(scenario);
    tracking=false; return 0;
}
"""


def compile_host(directory,bodies=None):
    compiler=shutil.which("g++")
    if not compiler: pytest.skip("host g++ required")
    tables,actual,getter=live_source()
    source=directory/"palettes.cpp"; executable=directory/"palettes"
    source.write_text(HARNESS.replace("// TABLES",tables).replace("// GETTER",getter)
                      .replace("// BODIES",actual if bodies is None else bodies))
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined",
        "-fno-sanitize-recover=undefined",str(source),"-o",str(executable)],text=True,capture_output=True,timeout=30)
    assert compiled.returncode==0,compiled.stdout+compiled.stderr
    return executable


@pytest.fixture(scope="module")
def executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("palette-producers"))


def run(executable,scenario):
    return subprocess.run([str(executable),scenario],text=True,capture_output=True,timeout=20)


@pytest.mark.parametrize("scenario",[
    "compare_equal","compare_changed","compare_missing","compare_create_failure",
    "cold","defaults","indices_wrap","negative_transparent","warm","palette_changed",*("basic:"+str(i) for i in range(6)),
    "explosion:0","explosion:6","explosion:12","territory","armor","transparent",
    "omitted_weight","guarded_create","mandatory_create","body_missing","short_palette",
    "throw:text:energyglow","throw:pixel","throw:nearest","throw:+File1","throw:+Buffer",
])
def test_live_palette_paths(executable,scenario):
    result=run(executable,scenario)
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("offset",[0,12,*range(24,1688,64),1688,1720,1752,1784,1800,1804,1807])
def test_each_serialized_segment_invalidates(executable,offset):
    result=run(executable,"header:"+str(offset))
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("damage,scenario",[
    ("local_palette","cold"), ("stale_territory","explosion:6"),
    ("palette_payload_offset","cold"), ("lifetime_order","cold"),
    ("short_compare","compare_changed"), ("wrong_header_weight","warm"),
    ("hide_missing_body","body_missing"), ("missing_tint_store","cold"),
    ("missing_text_get","cold"),
    ("unguarded_basic_create","guarded_create"),
    ("wrong_divisor12","cold"), ("wrong_divisor3","cold"),
])
def test_runtime_damage(tmp_path,damage,scenario):
    _,bodies,_=live_source()
    changes={
        "wrong_divisor12": ("row * transparent_second) / 24", "row * transparent_second) / 12"),
        "wrong_divisor3": ("row * transparent_second) / 24", "row * transparent_second) / 3"),
        "local_palette": ("g_PALETTE1.create_table(ShadowColorTable", "local_palette.create_table(ShadowColorTable"),
        "stale_territory": ('if (!regenerate) territory_cache = file.open("territory.tmp", 1);',
                            'territory_cache = file.open("territory.tmp", 1);'),
        "palette_payload_offset": ("palette_ + 0x302 + row * 256", "palette_ + 0x301 + row * 256"),
        "lifetime_order": ("Palette local_palette;\n    Filemap file;", "Filemap file;\n    Palette local_palette;"),
        "short_compare": ("256 * sizeof(PALETTEENTRY)", "255 * sizeof(PALETTEENTRY)"),
        "wrong_header_weight": ("memcmp(&transparent_first, cached, sizeof(transparent_first))",
                                "memcmp(&transparent_second, cached, sizeof(transparent_second))"),
        "hide_missing_body": ("memcpy(BodyTintColorTables, cache, sizeof(BodyTintColorTables));",
                              "if (cache) memcpy(BodyTintColorTables, cache, sizeof(BodyTintColorTables));"),
        "missing_tint_store": ("BodyTintColorTables[body][row][color] = (unsigned char)", "(void)"),
        "missing_text_get": ("text_get();\n            energy_colors[energy]", "energy_colors[energy]"),
        "unguarded_basic_create": ("if (cache) memcpy(cache, basic_tables[basic], basic_sizes[basic]);",
                                   "memcpy(cache, basic_tables[basic], basic_sizes[basic]);"),
    }
    old,new=changes[damage]
    assert bodies.count(old)==({"palette_payload_offset":2,"short_compare":4}.get(damage,1))
    bodies=bodies.replace(old,new)
    result=run(compile_host(tmp_path,bodies),scenario)
    assert result.returncode==91,result.stdout+result.stderr
