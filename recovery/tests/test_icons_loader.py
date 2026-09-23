"""Actual full load_icons4547E0..454D0C with original-derived event oracles.

The original attempts86 extracts,172 recolours,three sheet loads and one final
close, regardless of service results or DirectDraw mode. Definitions, public
declarations, service signatures and the whole unchanged loader are extracted.
Graphics/PCX services and their mutable payloads are explicit recording doubles:
this does not prove asset contents, allocation/lifetime, native ABI, service
implementations, startup closure or game acceptance. ASan/UBSan are strict;
LeakSanitizer is disabled because the sandbox uses ptrace.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SPRITES = (
    "g_ICONS_GENERAL_SPRITES", "g_RESOURCE_ICON_SPRITES",
    "g_RED_ALIEN_HEAD_ICON_SPRITE", "g_SILVER_MENU_ICON_SPRITES",
    "g_SILVER_CHECKBOX_ICON_SPRITES", "g_RED_MALE_HEAD_ICON_SPRITE",
    "g_NULL_RESOURCE_ICON_SPRITE", "g_PEACE_SIGN_SPRITES",
    "g_ICON_TILE_SQUARE_SPRITE", "g_SCROLL_BAR_ARROW_ICON_SPRITES",
    "g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES", "g_SCROLL_BAR_FILLER_ICON_SPRITES",
    "g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE",
)


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
    source = (ROOT / "src/sprite.cpp").read_text()
    sh = (ROOT / "src/sprite.h").read_text()
    bh = (ROOT / "src/buffer.h").read_text()
    objects = "\n".join((ROOT / ("src/" + name + ".cpp")).read_text()
                        for name in ("sprite", "buffer"))
    definitions = []
    for name in SPRITES:
        definition, = re.findall(r"^Sprite " + name + r"(?:\[\d+\])?;", objects, re.M)
        definitions.append(definition)
    for pattern in (r"^Buffer g_PCX_PARSE_TEMP_BUFFER1;", r"^IDirectDraw \*BufferDirectDraw;"):
        definition, = re.findall(pattern, objects, re.M)
        definitions.append(definition)
    externs = []
    for definition in definitions:
        public, = re.findall(r"^extern " + re.escape(definition), sh + "\n" + bh, re.M)
        externs.append(public)
    loader, = re.findall(r"^void __cdecl load_icons\(\);", (ROOT / "src/game.h").read_text(), re.M)
    extract, = re.findall(r"  int extract\([^;]+\);", sh)
    methods = []
    for pattern in (r"  int load_pcx\(const char \*[^;]+;",
                    r"  int change_color\(int a1, int a2\);", r"  void close\(\);"):
        method, = re.findall(pattern, bh)
        methods.append(method)
    return {
        "INTERFACES": "struct Sprite { int payload; " + extract + "};\nstruct Buffer { int payload; " + "\n".join(methods) + "};",
        "DECLARATIONS": "\n".join(externs + [loader]),
        "OBJECTS": "\n".join(definitions),
        "BODY": braced(source, "void __cdecl load_icons()"),
    }


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define __cdecl
struct Buffer;struct Palette;struct TexHeap;struct IDirectDraw {};
INTERFACES
DECLARATIONS
OBJECTS
int scenario;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::_Exit(95);}}
#define require(x) check((x),__LINE__)
struct Event {char kind;Sprite *sprite;std::vector<int> arguments;std::string file;};
std::vector<Event> expected;
std::vector<Sprite*> assets;
std::vector<int> payloads;
size_t cursor;
int loads,extracts,recolours,closes,buffer_payload;
IDirectDraw surface;
void verify_payloads(){require(g_PCX_PARSE_TEMP_BUFFER1.payload==buffer_payload);
    for(size_t n=0;n<assets.size();++n)require(assets[n]->payload==payloads[n]);}
Event &observe(char kind){require(cursor<expected.size());Event&e=expected[cursor++];require(e.kind==kind);
    verify_payloads();return e;}
void perturb(){if(scenario!=4)return;
    // Mutations are legal recording-double state, not claims about real PCX
    // callbacks. They distinguish invented mode guards or payload resets.
    BufferDirectDraw=cursor%2?&surface:nullptr;
    g_PCX_PARSE_TEMP_BUFFER1.payload=buffer_payload=1000+int(cursor);
    for(size_t n=0;n<assets.size();++n)assets[n]->payload=payloads[n]=int(cursor*97+n);}
int Buffer::load_pcx(const char*file,Palette*palette,int first,int count){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1&&palette==nullptr);
    Event&e=observe('L');require(e.file==file&&e.arguments==std::vector<int>({first,count}));
    ++loads;perturb();return scenario==1||scenario==4?6:0;}
int Buffer::change_color(int from,int to){require(this==&g_PCX_PARSE_TEMP_BUFFER1);
    Event&e=observe('C');require(e.arguments==std::vector<int>({from,to}));
    ++recolours;perturb();return scenario==3||scenario==4?7:0;}
int Sprite::extract(Buffer*buffer,int key,int x,int y,int width,int height,TexHeap*heap){
    require(buffer==&g_PCX_PARSE_TEMP_BUFFER1&&heap==nullptr);
    Event&e=observe('E');require(this==e.sprite&&e.arguments==std::vector<int>({key,x,y,width,height}));
    ++extracts;perturb();return scenario==2||scenario==4?9:0;}
void Buffer::close(){require(this==&g_PCX_PARSE_TEMP_BUFFER1);observe('X');
    require(loads==3&&extracts==86&&recolours==172);++closes;perturb();}
BODY
void event(char kind,Sprite*s=nullptr,std::initializer_list<int>a={},const char*file=""){
    expected.push_back(Event{kind,s,a,file});}
void cell(Sprite*s,int x,int y,int width,int height){
    event('C',nullptr,{263,265});event('C',nullptr,{262,265});event('E',s,{265,x,y,width,height});
    assets.push_back(s);}
void prepare(bool direct_draw){
    expected.clear();assets.clear();payloads.clear();cursor=0;loads=extracts=recolours=closes=0;
    BufferDirectDraw=direct_draw?&surface:nullptr;
    // Coordinate grids are independently tabulated from original instruction
    // immediates and receiver strides, not generated by the candidate loops.
    const int general_x[]={1,24,47,70},general_y[]={13,36,59,82};
    const int resource_x[]={174,215,256,297,338,379,420,461},resource_y[]={304,345,386,427};
    const int arrow_x[]={104,129,154},arrow_y[]={1,26,51,76};
    const int small_x[]={192,203,214},small_y[]={1,12,23,34};
    const int menu_x[]={320,336,352,368};
    event('L',nullptr,{10,236},"NEWICONS.PCX");
    for(int n=0;n<16;++n)cell(&g_ICONS_GENERAL_SPRITES[n],general_x[n%4],general_y[n/4],22,22);
    cell(&g_NULL_RESOURCE_ICON_SPRITE,1,174,22,22);
    cell(&g_PEACE_SIGN_SPRITES[0],1,220,22,22);cell(&g_PEACE_SIGN_SPRITES[1],1,243,22,22);
    for(int n=0;n<32;++n)cell(&g_RESOURCE_ICON_SPRITES[n],resource_x[n%8],resource_y[n/8],40,40);
    event('L',nullptr,{10,236},"TER1WRECK.PCX");
    cell(&g_RED_ALIEN_HEAD_ICON_SPRITE,768,148,28,42);
    event('L',nullptr,{10,236},"ICONS.PCX");
    cell(&g_ICON_TILE_SQUARE_SPRITE,1,1,100,50);
    cell(&g_RED_MALE_HEAD_ICON_SPRITE,46,303,33,30);
    for(int n=0;n<12;++n)cell(&g_SCROLL_BAR_ARROW_ICON_SPRITES[n],arrow_x[n%3],arrow_y[n/3],24,24);
    cell(&g_SCROLL_BAR_FILLER_ICON_SPRITES,104,101,18,18);
    for(int n=0;n<12;++n)cell(&g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[n],small_x[n%3],small_y[n/3],10,10);
    cell(&g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE,225,1,10,10);
    for(int n=0;n<4;++n)cell(&g_SILVER_MENU_ICON_SPRITES[n],menu_x[n],1,15,15);
    cell(&g_SILVER_CHECKBOX_ICON_SPRITES[0],320,20,16,16);
    cell(&g_SILVER_CHECKBOX_ICON_SPRITES[1],337,20,16,16);
    event('X');require(expected.size()==262&&assets.size()==86);
    for(size_t n=0;n<assets.size();++n){for(size_t k=0;k<n;++k)require(assets[n]!=assets[k]);
        assets[n]->payload=-100-int(n);payloads.push_back(assets[n]->payload);}
    g_PCX_PARSE_TEMP_BUFFER1.payload=buffer_payload=-71;
}
int main(){for(scenario=0;scenario<5;++scenario)for(int direct=0;direct<2;++direct){
    prepare(direct!=0);
    // Repeat against the same mutated receiver objects without reconstructing
    // them: the loader must attempt the entire sequence on every invocation.
    for(int repeat=0;repeat<2;++repeat){cursor=0;loads=extracts=recolours=closes=0;
        load_icons();require(cursor==expected.size());verify_payloads();
        require(loads==3&&extracts==86&&recolours==172&&closes==1);}
}}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "icons.cpp", tmp_path / "icons"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_icons_loader(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("general_owner", "icon->extract(&buffer, 265, source_x, source_y, 22, 22, 0)", "g_ICONS_GENERAL_SPRITES[0].extract(&buffer, 265, source_x, source_y, 22, 22, 0)"),
    ("general_rows", "icon < g_ICONS_GENERAL_SPRITES + 16", "icon < g_ICONS_GENERAL_SPRITES + 12"),
    ("general_pitch", "source_x += 23", "source_x += 22"),
    ("general_y", "source_y = 13", "source_y = 12"),
    ("general_width", "22, 22, 0", "21, 22, 0"),
    ("null_owner", "g_NULL_RESOURCE_ICON_SPRITE.extract", "g_RED_MALE_HEAD_ICON_SPRITE.extract"),
    ("peace_owner", "icon->extract(&buffer, 265, 1, source_y, 22, 22, 0)", "g_PEACE_SIGN_SPRITES[0].extract(&buffer, 265, 1, source_y, 22, 22, 0)"),
    ("peace_count", "icon < g_PEACE_SIGN_SPRITES + 2", "icon < g_PEACE_SIGN_SPRITES + 1"),
    ("resource_owner", "icon->extract(&buffer, 265, source_x, source_y, 40, 40, 0)", "g_RESOURCE_ICON_SPRITES[0].extract(&buffer, 265, source_x, source_y, 40, 40, 0)"),
    ("resource_columns", "columns_left = 8", "columns_left = 7"),
    ("resource_y", "source_y += 41", "source_y += 40"),
    ("resource_size", "40, 40, 0", "40, 39, 0"),
    ("alien_owner", "g_RED_ALIEN_HEAD_ICON_SPRITE.extract", "g_RED_MALE_HEAD_ICON_SPRITE.extract"),
    ("alien_width_height", "768, 148, 28, 42", "768, 148, 42, 28"),
    ("tile_size", "1, 1, 100, 50", "1, 1, 100, 49"),
    ("male_coordinates", "46, 303, 33, 30", "46, 302, 33, 30"),
    ("arrow_owner", "icon->extract(&buffer, 265, source_x, source_y, 24, 24, 0)", "g_SCROLL_BAR_ARROW_ICON_SPRITES[0].extract(&buffer, 265, source_x, source_y, 24, 24, 0)"),
    ("arrow_pitch", "source_x += 25", "source_x += 24"),
    ("filler_reset_x", "265, 104, 101, 18, 18", "265, 179, 101, 18, 18"),
    ("small_arrow_owner", "icon->extract(&buffer, 265, source_x, source_y, 10, 10, 0)", "g_SCROLL_BAR_ARROW_ICON_SPRITES[0].extract(&buffer, 265, source_x, source_y, 10, 10, 0)"),
    ("small_arrow_y", "source_y += 11", "source_y += 10"),
    ("small_filler_x", "265, 225, 1, 10, 10", "265, 224, 1, 10, 10"),
    ("menu_owner", "icon->extract(&buffer, 265, source_x, 1, 15, 15, 0)", "g_SILVER_MENU_ICON_SPRITES[0].extract(&buffer, 265, source_x, 1, 15, 15, 0)"),
    ("menu_size", "1, 15, 15, 0", "1, 16, 16, 0"),
    ("checkbox_owner", "g_SILVER_CHECKBOX_ICON_SPRITES[1]", "g_SILVER_CHECKBOX_ICON_SPRITES[0]"),
    ("checkbox_x", "265, 337, 20", "265, 336, 20"),
    ("recolour_order", "buffer.change_color(263, 265);", "buffer.change_color(262, 265);"),
    ("skip_recolour", "buffer.change_color(262, 265);", ""),
    ("extract_key", "&buffer, 265, source_x", "&buffer, 264, source_x"),
    ("wrong_sheet", '"TER1WRECK.PCX"', '"ICONS.PCX"'),
    ("load_palette_range", '"NEWICONS.PCX", 0, 10, 236', '"NEWICONS.PCX", 0, 0, 236'),
    ("skip_sheet", 'buffer.load_pcx("ICONS.PCX", 0, 10, 236);', ""),
    ("load_failure_exit", 'buffer.load_pcx("NEWICONS.PCX", 0, 10, 236);', 'if (buffer.load_pcx("NEWICONS.PCX", 0, 10, 236)) return;'),
    ("extract_failure_exit", 'g_NULL_RESOURCE_ICON_SPRITE.extract(&buffer, 265, 1, 174, 22, 22, 0);', 'if (g_NULL_RESOURCE_ICON_SPRITE.extract(&buffer, 265, 1, 174, 22, 22, 0)) return;'),
    ("recolour_failure_exit", "buffer.change_color(263, 265);", "if (buffer.change_color(263, 265)) return;"),
    ("invented_mode_guard", "Buffer &buffer =", "if (BufferDirectDraw) return; Buffer &buffer ="),
    ("invented_mid_mode_guard", 'buffer.load_pcx("TER1WRECK.PCX", 0, 10, 236);', 'if (BufferDirectDraw) return; buffer.load_pcx("TER1WRECK.PCX", 0, 10, 236);'),
    ("premature_close", 'buffer.load_pcx("ICONS.PCX", 0, 10, 236);', 'buffer.close(); buffer.load_pcx("ICONS.PCX", 0, 10, 236);'),
    ("missing_close", "buffer.close();", ""),
]


@pytest.mark.parametrize("name,before,after", DAMAGE, ids=[row[0] for row in DAMAGE])
def test_compiling_icons_damage(tmp_path, name, before, after):
    parts = actual_source()
    assert before in parts["BODY"], name
    parts["BODY"] = parts["BODY"].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 95 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)
