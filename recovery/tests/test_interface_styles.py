"""Full interface loader observations from original453A70..453F6F.

The original performs431 Sprite extracts,52 Buffer init/copy pairs and51
style publications. Pointer stores453CC5..453D2E precede the mask pixel read;
null publication precedes close, and mask colour follows it. Tests extract the
whole source loader/helper, actual BoxSpriteParams and service declarations.
Graphics/PCX allocation and lifetime are recording doubles, not native ABI,
asset contents, startup closure or working-game proof. WinFillColour retains
its actual const type: every extract argument is observed, but illegal mutation
of const storage is deliberately not used to claim callback reload coverage.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
POPUPS = ("TOP_LEFT", "TOP_MID", "TOP_RIGHT", "MID_LEFT", "MID_RIGHT",
          "BOT_LEFT", "BOT_MID", "BOT_RIGHT")
SPRITES = (tuple("g_IFACE_BOX_SPRITES" + str(n) for n in range(1, 9)) +
           ("g_IFACE_LOCK_SPRITES", "g_IFACE_GREEN_RIGHT_ARROW_SPRITE",
            "g_IFACE_MP_COMBO_ARROW_SPRITES", "g_IFACE_TECH_TREE_ARROW_SPRITES",
            ) +
           tuple("g_IFACE_STD_POPUPS_" + name + "_SPRITE" for name in POPUPS))


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
    source = (ROOT / "src/game.cpp").read_text()
    sh = (ROOT / "src/sprite.h").read_text()
    bh = (ROOT / "src/buffer.h").read_text()
    headers = "\n".join((ROOT / ("src/" + name + ".h")).read_text()
                        for name in ("sprite", "buffer", "newtechwin", "win", "game"))
    objects = "\n".join((ROOT / ("src/" + name + ".cpp")).read_text()
                        for name in ("sprite", "buffer", "newtechwin"))
    definitions = []
    for name in SPRITES:
        definition, = re.findall(r"^Sprite " + name + r"(?:\[\d+\])?;", objects, re.M)
        definitions.append(definition)
    for name in ("g_PCX_PARSE_TEMP_BUFFER1", "g_IFACE_BOX_SPRITE_BUFFERS", "g_IFACE_STD_POPUPS_MIDDLE_BUFFER"):
        definition, = re.findall(r"^Buffer " + name + r"(?:\[\d+\])?;", objects, re.M)
        definitions.append(definition)
    colour, = re.findall(r"^const unsigned char WinFillColour = \d+;", (ROOT / "src/win.cpp").read_text(), re.M)
    definitions.append(colour)
    externs = []
    for definition in definitions:
        declaration = definition.split(" = ")[0] + ";" if " = " in definition else definition
        public, = re.findall(r"^extern " + re.escape(declaration), headers, re.M)
        externs.append(public)
    style, = re.findall(r"^BoxSpriteParams InterfaceBoxStyles\[51\];", source, re.M)
    style_extern, = re.findall(r"^extern BoxSpriteParams InterfaceBoxStyles\[51\];", bh, re.M)
    loader_declaration, = re.findall(r"^void __cdecl load_box_sprites\(\);", headers, re.M)
    # The loader calls the integer-rectangle overload. The RECT overload is
    # a separate API; its presence must not prevent this actual-body control.
    extract, = re.findall(r"  int extract\(Buffer \*buffer, int \w+, int \w+,[^;]+\);", sh)
    methods = []
    for pattern in (r"  int load_pcx\(const char \*[^;]+;", r"  int get_pixel\([^;]+;",
                    r"  int init\(int width, int height, int tgl, ExtDirectDraw \*direct_draw\);",
                    r"  void close\(\);"):
        method, = re.findall(pattern, bh)
        methods.append(method)
    copy, = re.findall(r"  MEASURED int copy\(Buffer \*buffer, int xCoord, int yCoord, int wx, int wy,\s*int width, int height\);", bh)
    methods.append(copy)
    return {
        "INTERFACES": "struct Sprite {" + extract + "};\nstruct Buffer {" + "\n".join(methods) + "};",
        "PARAMS": braced(bh, "struct BoxSpriteParams {") + ";",
        "PUBLIC_DECLARATIONS": "\n".join(externs + [style_extern, loader_declaration]),
        "OBJECTS": "\n".join(definitions),
        "STYLES": style,
        "HELPER": braced(source, "void extract_interface_box_sprite("),
        "LOADER": braced(source, "void __cdecl load_box_sprites()"),
    }


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <utility>
#define __cdecl
#define MEASURED
struct Palette;struct TexHeap;struct ExtDirectDraw;struct Buffer;
INTERFACES
PARAMS
PUBLIC_DECLARATIONS
OBJECTS
STYLES
int scenario,extract_count,init_count,copy_count,pixel_count,mask_closes,canvas_closes;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::exit(93);}}
#define require(x) check((x),__LINE__)
struct Event {char kind;std::vector<int> args;};
std::vector<Event> expected;size_t cursor;
std::map<Sprite*,std::pair<int,int>> identities;
Sprite sentinel_sprite;Buffer sentinel_mask,callback_mask;
Sprite *families[8]={g_IFACE_BOX_SPRITES1,g_IFACE_BOX_SPRITES2,g_IFACE_BOX_SPRITES3,g_IFACE_BOX_SPRITES4,
                     g_IFACE_BOX_SPRITES5,g_IFACE_BOX_SPRITES6,g_IFACE_BOX_SPRITES7,g_IFACE_BOX_SPRITES8};
Sprite *popups[8]={&g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE,&g_IFACE_STD_POPUPS_TOP_MID_SPRITE,&g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE,
    &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE,&g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE,&g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE,
    &g_IFACE_STD_POPUPS_BOT_MID_SPRITE,&g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE};
void expected_event(char kind,std::initializer_list<int> args){Event e={kind,args};expected.push_back(e);}
void observe(char kind,std::initializer_list<int> args={}){require(cursor<expected.size());
    require(expected[cursor].kind==kind&&expected[cursor].args==std::vector<int>(args));++cursor;}
int mask_id(Buffer *buffer){for(int n=0;n<51;++n)if(buffer==&g_IFACE_BOX_SPRITE_BUFFERS[n])return n;
    if(buffer==&g_IFACE_STD_POPUPS_MIDDLE_BUFFER)return 51;require(false);return -1;}
std::vector<Sprite*> frame(const BoxSpriteParams&r){return {r.corner0_,r.corner1_,r.corner2_,r.corner3_,r.edge0_,r.edge1_,r.edge2_,r.edge3_};}
void require_unpublished(int n){const auto&r=InterfaceBoxStyles[n];
    for(auto pointer:frame(r))require(pointer==&sentinel_sprite);
    require(r.mask_==&sentinel_mask&&r.mask_color_==-100-n);}
void require_frame(int n){const auto pointers=frame(InterfaceBoxStyles[n]);for(int k=0;k<8;++k)require(pointers[k]==&families[k][n]);}
int pixel_result(int n){return n%3==0?9:n%3==1?7:-1;}
int Buffer::load_pcx(const char*name,Palette*palette,int mode,int height){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1&&std::strcmp(name,"interface.pcx")==0&&palette==nullptr);
    observe('L',{mode,height});return scenario==0?0:-71;}
int Sprite::extract(Buffer*canvas,int key,int x,int y,int width,int height,TexHeap*heap){
    require(canvas==&g_PCX_PARSE_TEMP_BUFFER1&&heap==nullptr&&key==WinFillColour);
    require(identities.count(this)==1);const auto id=identities[this];
    if(id.first>=10&&id.first<18)require_unpublished(id.second);
    observe('E',{id.first,id.second,x,y,width,height,key});++extract_count;return scenario==0?0:-72;}
int Buffer::init(int width,int height,int mode,ExtDirectDraw*direct_draw){require(direct_draw==nullptr);int n=mask_id(this);
    if(n<51)require_unpublished(n);observe('I',{n,width,height,mode});++init_count;return scenario==0?0:-73;}
int Buffer::copy(Buffer*destination,int x,int y,int dest_x,int dest_y,int width,int height){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1);int n=mask_id(destination);if(n<51)require_unpublished(n);
    observe('C',{n,x,y,dest_x,dest_y,width,height});++copy_count;return scenario==0?0:-74;}
int Buffer::get_pixel(int x,int y){int n=mask_id(this);require(n<51);require_frame(n);
    require(InterfaceBoxStyles[n].mask_==&sentinel_mask&&InterfaceBoxStyles[n].mask_color_==-100-n);
    observe('P',{n,x,y});++pixel_count;
    if(scenario==2){InterfaceBoxStyles[n].corner0_=&sentinel_sprite;InterfaceBoxStyles[n].mask_=&callback_mask;
        InterfaceBoxStyles[n].mask_color_=1000+n;}
    return pixel_result(n);}
void Buffer::close(){
    if(this==&g_PCX_PARSE_TEMP_BUFFER1){observe('Z');++canvas_closes;require(pixel_count==51&&extract_count==431);return;}
    int n=mask_id(this);require(n<51&&pixel_result(n)==9);const auto&r=InterfaceBoxStyles[n];
    require(r.mask_==nullptr&&r.mask_color_==(scenario==2?1000+n:-100-n));
    observe('K',{n});++mask_closes;
    if(scenario==2){InterfaceBoxStyles[n].mask_=&callback_mask;InterfaceBoxStyles[n].edge3_=&sentinel_sprite;
        InterfaceBoxStyles[n].mask_color_=2000+n;}}
HELPER
LOADER
void prepare(){
    cursor=0;extract_count=init_count=copy_count=pixel_count=mask_closes=canvas_closes=0;expected.clear();identities.clear();
    for(int n=0;n<51;++n){auto&r=InterfaceBoxStyles[n];r.corner0_=r.corner1_=r.corner2_=r.corner3_=&sentinel_sprite;
        r.edge0_=r.edge1_=r.edge2_=r.edge3_=&sentinel_sprite;r.mask_=&sentinel_mask;r.mask_color_=-100-n;}
    for(int k=0;k<8;++k)for(int n=0;n<51;++n)identities[&families[k][n]]={10+k,n};
    for(int n=0;n<2;++n)identities[&g_IFACE_LOCK_SPRITES[n]]={1,n};
    identities[&g_IFACE_GREEN_RIGHT_ARROW_SPRITE[0]]={2,0};
    for(int n=0;n<3;++n){identities[&g_IFACE_MP_COMBO_ARROW_SPRITES[n]]={3,n};identities[&g_IFACE_TECH_TREE_ARROW_SPRITES[n]]={4,n};}
    for(int n=0;n<6;++n)identities[&g_NEWTECHWIN_SPRITES[n]]={5,n};
    for(int n=0;n<8;++n)identities[popups[n]]={6,n};
    expected_event('L',{10,236});
    expected_event('E',{1,0,247,71,15,19,WinFillColour});expected_event('E',{1,1,263,71,15,19,WinFillColour});
    expected_event('E',{2,0,615,530,16,15,WinFillColour});
    for(int n=0,x=534;n<3;++n,x+=8)expected_event('E',{3,n,x,537,7,13,WinFillColour});
    // Independent sheet-grid walk; four corners, two horizontal strips and
    // two vertical strips. This does not reuse the product's /3 and %3.
    const int offset_x[8]={-32,0,-32,0,-25,-25,-32,0};
    const int offset_y[8]={0,0,32,32,0,32,7,7};
    const int width[8]={6,6,6,6,24,24,6,6},height[8]={6,6,6,6,6,6,24,24};
    int style=0;
    for(int y=229;y<=853;y+=39)for(int x=33;x<=111;x+=39,++style){
        for(int k=0;k<8;++k)expected_event('E',{10+k,style,x+offset_x[k],y+offset_y[k],width[k],height[k],WinFillColour});
        expected_event('I',{style,24,24,0});expected_event('C',{style,x-25,y+7,0,0,24,24});
        expected_event('P',{style,0,0});if(pixel_result(style)==9)expected_event('K',{style});}
    require(style==51);
    const int popup_x[8]={296,358,420,296,420,296,358,420},popup_y[8]={711,711,711,756,756,801,801,801};
    for(int n=0;n<8;++n)expected_event('E',{6,n,popup_x[n],popup_y[n],61,44,WinFillColour});
    expected_event('I',{51,61,44,0});expected_event('C',{51,358,756,0,0,61,44});
    for(int n=0,x=140;n<3;++n,x+=8)expected_event('E',{4,n,x,37,7,13,WinFillColour});
    int icon=0;for(int x=391;x<=420;x+=29)for(int y=566;y<=636;y+=35,++icon)expected_event('E',{5,icon,x,y,28,34,WinFillColour});
    expected_event('Z',{});
}
void final_state(){
    require(cursor==expected.size()&&extract_count==431&&init_count==52&&copy_count==52&&pixel_count==51&&mask_closes==17&&canvas_closes==1);
    for(int n=0;n<51;++n){const auto&r=InterfaceBoxStyles[n];const auto pointers=frame(r);
        for(int k=0;k<8;++k){bool mutated=scenario==2&&(k==0||(k==7&&pixel_result(n)==9));require(pointers[k]==(mutated?&sentinel_sprite:&families[k][n]));}
        require(r.mask_color_==7);Buffer*want=pixel_result(n)==9?(scenario==2?&callback_mask:nullptr):&g_IFACE_BOX_SPRITE_BUFFERS[n];require(r.mask_==want);}
}
int main(){for(scenario=0;scenario<3;++scenario){prepare();load_box_sprites();final_state();}}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "styles.cpp", tmp_path / "styles"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_interface_style_loader(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("wrong_fill_argument", "HELPER", "WinFillColour,", "0,"),
    ("helper_source", "HELPER", "&g_PCX_PARSE_TEMP_BUFFER1,", "&g_IFACE_STD_POPUPS_MIDDLE_BUFFER,"),
    ("helper_dimensions", "HELPER", "x, y, width, height, nullptr", "x, y, height, width, nullptr"),
    ("pcx_filename", "LOADER", '"interface.pcx"', '"interface2.pcx"'),
    ("pcx_flags", "LOADER", '"interface.pcx", 0, 10, 236', '"interface.pcx", 0, 11, 236'),
    ("pcx_failure_abort", "LOADER", 'canvas.load_pcx("interface.pcx", 0, 10, 236);', 'if (canvas.load_pcx("interface.pcx", 0, 10, 236) != 0) return;'),
    ("lock_sprite_receiver", "LOADER", "g_IFACE_LOCK_SPRITES[1], 263", "g_IFACE_LOCK_SPRITES[0], 263"),
    ("combo_arrow_count", "LOADER", "arrow < 3", "arrow < 2"),
    ("style_count", "LOADER", "style < 51", "style < 50"),
    ("style_column_major", "LOADER", "(style % 3) * 39", "(style / 17) * 39"),
    ("style_row_pitch", "LOADER", "(style / 3) * 39", "(style / 3) * 38"),
    ("corner_geometry", "LOADER", "x - 32, y, 6, 6", "x - 31, y, 6, 6"),
    ("edge_geometry", "LOADER", "x - 25, y, 24, 6", "x - 25, y, 23, 6"),
    ("vertical_edge_geometry", "LOADER", "x, y + 7, 6, 24", "x, y + 6, 6, 24"),
    ("mask_init_dimensions", "LOADER", "mask.init(24, 24, 0, nullptr)", "mask.init(24, 23, 0, nullptr)"),
    ("mask_init_failure_abort", "LOADER", "mask.init(24, 24, 0, nullptr);", "if (mask.init(24, 24, 0, nullptr) != 0) return;"),
    ("copy_direction", "LOADER", "canvas.copy(&mask,", "mask.copy(&canvas,"),
    ("copy_origin", "LOADER", "canvas.copy(&mask, x - 25, y + 7, 0, 0", "canvas.copy(&mask, x - 25, y + 7, 1, 0"),
    ("copy_source", "LOADER", "canvas.copy(&mask, x - 25, y + 7", "canvas.copy(&mask, x - 24, y + 7"),
    ("copy_failure_abort", "LOADER", "canvas.copy(&mask, x - 25, y + 7, 0, 0, 24, 24);", "if (canvas.copy(&mask, x - 25, y + 7, 0, 0, 24, 24) != 0) return;"),
    ("corner_publication", "LOADER", "corner0_ = &g_IFACE_BOX_SPRITES1[style]", "corner0_ = &g_IFACE_BOX_SPRITES2[style]"),
    ("edge_publication", "LOADER", "edge3_ = &g_IFACE_BOX_SPRITES8[style]", "edge3_ = &g_IFACE_BOX_SPRITES7[style]"),
    ("premature_mask_publication", "LOADER", "if (mask.get_pixel(0, 0) == 9)", "InterfaceBoxStyles[style].mask_ = &mask; if (mask.get_pixel(0, 0) == 9)"),
    ("pixel_receiver", "LOADER", "mask.get_pixel(0, 0)", "canvas.get_pixel(0, 0)"),
    ("pixel_coordinates", "LOADER", "mask.get_pixel(0, 0)", "mask.get_pixel(1, 0)"),
    ("pixel_key", "LOADER", "mask.get_pixel(0, 0) == 9", "mask.get_pixel(0, 0) == 7"),
    ("pixel_failure_is_not_empty", "LOADER", "mask.get_pixel(0, 0) == 9", "mask.get_pixel(0, 0) != 7"),
    ("null_before_close", "LOADER", "InterfaceBoxStyles[style].mask_ = nullptr;\n            mask.close();", "mask.close();\n            InterfaceBoxStyles[style].mask_ = nullptr;"),
    ("close_missing", "LOADER", "mask.close();", "/* mask remains open */"),
    ("borrowed_mask", "LOADER", "InterfaceBoxStyles[style].mask_ = &mask;", "InterfaceBoxStyles[style].mask_ = &canvas;"),
    ("colour_before_callback", "LOADER", "if (mask.get_pixel(0, 0) == 9)", "InterfaceBoxStyles[style].mask_color_ = 7; if (mask.get_pixel(0, 0) == 9)"),
    ("colour_value", "LOADER", "mask_color_ = 7", "mask_color_ = 9"),
    ("callback_mask_retention", "LOADER", "mask.close();", "mask.close(); InterfaceBoxStyles[style].mask_ = nullptr;"),
    ("callback_pointer_retention", "LOADER", "InterfaceBoxStyles[style].mask_color_ = 7;", "InterfaceBoxStyles[style].mask_color_ = 7; InterfaceBoxStyles[style].corner0_ = &g_IFACE_BOX_SPRITES1[style];"),
    ("popup_receiver", "LOADER", "g_IFACE_STD_POPUPS_TOP_MID_SPRITE, 358", "g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE, 358"),
    ("popup_middle_init", "LOADER", "g_IFACE_STD_POPUPS_MIDDLE_BUFFER.init(61, 44", "g_IFACE_STD_POPUPS_MIDDLE_BUFFER.init(61, 43"),
    ("popup_middle_copy", "LOADER", "MIDDLE_BUFFER, 358, 756, 0, 0, 61, 44", "MIDDLE_BUFFER, 358, 755, 0, 0, 61, 44"),
    ("tech_arrow_count", "LOADER", "tech_arrow < 3", "tech_arrow < 2"),
    ("newtech_count", "LOADER", "technology < 6", "technology < 5"),
    ("newtech_row_major", "LOADER", "(technology / 3) * 29", "(technology % 2) * 29"),
    ("newtech_row_pitch", "LOADER", "(technology % 3) * 35", "(technology % 3) * 34"),
    ("final_canvas_close", "LOADER", "canvas.close();", "/* no final close */"),
]


@pytest.mark.parametrize("name,part,before,after", DAMAGE, ids=[item[0] for item in DAMAGE])
def test_interface_style_damage_is_observed(tmp_path, name, part, before, after):
    parts = actual_source()
    assert before in parts[part], (name, part)
    parts[part] = parts[part].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 93 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)
