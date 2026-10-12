"""Actual-source full terrain-loader orchestration (451C50..45370C).

The original-derived oracle enumerates all 486 ordered services, their receivers
and arguments; the complete loader/helpers are extracted unchanged. Real asset
declarations and the remap array/include/static_assert are retained. Its ignored
include is supplied with explicitly SYNTHETIC 53-byte adversarial inputs in /tmp;
these are not the proprietary table or a native initial-data proof.

Graphics services and Sprite/Buffer lifetime are recording doubles. Strict host
ASan/UBSan controls prove only this bounded orchestration, partial C++ unwind and
callback observation, not actual graphics, native ABI/EH, file loading, original
asset contents, startup or game acceptance. The genuine source-remap kernel has
separate tests; this suite checks its call and entire supplied table, not pixels.
LeakSanitizer is disabled because the sandbox uses ptrace.
"""

from collections import Counter
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def one(pattern, text):
    result, = re.findall(pattern, text, re.M)
    return result


def actual_source():
    source = (ROOT / "src/game.cpp").read_text()
    headers = {name: (ROOT / f"src/{name}.h").read_text()
               for name in ("sprite", "texture", "buffer", "game")}
    owners = "\n".join((ROOT / f"src/{name}.cpp").read_text()
                       for name in ("sprite", "texture", "buffer", "game"))
    body = "\n\n".join(braced(source, signature) for signature in (
        "void terrain_extract_texture_grid(", "void terrain_extract_sprite(",
        "void __cdecl load_terrain()"))
    objects, declarations, registrations = [], [], []
    for name in sorted(set(re.findall(r"\bg_[A-Z0-9_]+", body))):
        definition = one(r"^(?:Sprite|Texture|Buffer) " + name + r"(?:\[\d+\])?;", owners)
        objects.append(definition)
        declarations.append(one(r"^extern " + re.escape(definition), "\n".join(headers.values())))
        bound = re.search(r"\[(\d+)\]", definition)
        if bound:
            for index in range(int(bound[1])):
                registrations.append(f'names[&{name}[{index}]] = "{name}[{index}]";')
        else:
            registrations.append(f'names[&{name}] = "{name}";')
    for pattern, header in (
        (r"^unsigned int TextureRasterKey;", "texture"),
        (r"^unsigned char TerrainCoastPalette\[12\];", "game"),
        (r"^IDirectDraw2 \*BufferDirectDraw;", "buffer"),
    ):
        definition = one(pattern, owners)
        objects.append(definition)
        declarations.append(one(r"^extern " + re.escape(definition), headers[header]))
    declarations.append(one(r"^void __cdecl load_terrain\(\);", headers["game"]))
    sprite = headers["sprite"]
    sprite_methods = [
        one(r"  Sprite\(\);", sprite),
        one(r"  ~Sprite\(\) \{ close\(\); \}", sprite),
        one(r"  void close\(\);", sprite),
        one(r"  unsigned char cTransparentIndex_;", sprite),
        one(r"  unsigned char transparent_index\(\) const \{[^}]+\}", sprite),
        one(r"  int extract\(Buffer \*buffer, int a,[^;]+;", sprite),
        one(r"  int draw\(Buffer \*destination, int transparent, int x, int y,\s*unsigned char \*table\);", sprite),
        one(r"  int draw_dest\(Buffer \*destination, int transparent, int x, int y,\s*unsigned char \*table\);", sprite),
    ]
    buffer_methods = [one(pattern, headers["buffer"]) for pattern in (
        r"  Buffer\(\);", r"  virtual ~Buffer\(\);",
        r"  int load_pcx\(const char \*[^;]+;",
        r"  int change_color\(int a1, int a2\);", r"  void close\(\);",
        r"  int init\(int width, int height, int tgl, ExtDirectDraw \*direct_draw\);",
        r"  int get_pixel\(int x, int y\);", r"  int fill\(int color\);")]
    texture_extract = one(r"  int extract\(Buffer \*buffer, int x, int y, int width, int height,\s*TexHeap \*heap\);", headers["texture"])
    inputs = one(r'static const unsigned char TerrainCoastRemapSource\[\] = \{\s*#include "terrain_coast_remap.inc"\s*\};', source)
    inputs += "\n" + one(r'static_assert\(sizeof\(TerrainCoastRemapSource\) == 53, "terrain remap extent"\);', source)
    return {
        "INTERFACES": "struct Sprite {\n" + "\n".join(sprite_methods) + "\n};\nstruct Buffer {\n" +
                      "\n".join(buffer_methods) + "\n};\nstruct Texture {\n" + texture_extract + "\n};",
        "DECLARATIONS": "\n".join(declarations), "OBJECTS": "\n".join(objects),
        "INPUT": inputs, "BODY": body, "REGISTRATIONS": "\n".join(registrations),
    }


def expected_calls():
    out = []
    canvas = 'g_PCX_PARSE_TEMP_BUFFER1'
    def event(kind, receiver, *args): out.append((kind, receiver, args))
    def load(name, first=10, count=236): event('load', canvas, name, None, first, count)
    def texture(receiver, x, y, width=56, height=56, source=canvas):
        event('texture', receiver, source, x, y, width, height, 0)
    def sheet_rows(stem, rows, xs, indices=None):
        receivers = iter(indices if indices is not None else range(len(rows)*len(xs)))
        for y in rows:
            for x in xs:
                try: index = next(receivers)
                except StopIteration: return
                texture(f'{stem}[{index}]', x, y)
    def sprite(receiver, x, y, width=100, height=62, recolour=True):
        if recolour:
            event('recolour', canvas, 263, 265)
            event('recolour', canvas, 262, 265)
        event('sprite', receiver, canvas, 265, x, y, width, height, None)

    load('TEXTURE.PCX')
    event('init', 'scratch', 28, 28, 0, None)
    texture('g_RADIUS1_TEXTURE', 1, 571)
    texture('g_RADIUS2_TEXTURE', 58, 571)
    sheet_rows('g_ROCKY_TEXTURES', [1], [1, 58, 115, 172], [0, 2, 1, 3])
    sheet_rows('g_OCEAN_TEXTURES', [79, 136], [280])
    texture('g_FLAT_ARID_LAND_TEXTURE', 1, 58)
    sheet_rows('g_MOIST_LAND_TEXTURES', [115, 172, 229, 286], [1, 58, 115, 172])
    sheet_rows('g_RAINY_LAND_TEXTURES', [343, 400, 457, 514], [1, 58, 115, 172])
    sheet_rows('g_JUNGLE_LAND_TEXTURES', [259, 316, 373, 430], [526, 583, 640, 697], range(15))
    texture('g_DUNE_LAND_TEXTURE', 229, 1)
    sheet_rows('g_SUNNY_MESA_TEXTURES', [15, 72], [768, 825, 882, 939])
    sheet_rows('g_ROAD_TEXTURES', [395, 452, 509], [775, 832, 889])
    sheet_rows('g_MAGTUBE_TEXTURES', [566, 623, 680], [775, 832, 889])
    sheet_rows('g_RIVER_TEXTURES', [259, 316, 373, 430], [280, 337, 394, 451])
    sheet_rows('g_MOUNT_PLANET_TEXTURES', [628], [1, 58, 115])
    sheet_rows('g_GARLAND_CRATER_TEXTURES', [685], [1, 58, 115])
    sheet_rows('g_FUNGUS_TEXTURES', [516, 573, 630, 687], [280, 337, 394, 451], range(15))
    sheet_rows('g_FUNGUS_TEXTURES', [516, 573, 630, 687], [508, 565, 622, 679], range(15, 30))
    sheet_rows('g_FARM_TEXTURES', [219, 276, 333], [775, 832, 889])
    sheet_rows('g_FOREST_TEXTURES', [6, 63, 120, 177], [526, 583, 640, 697])

    load('RAINFALL.PCX')
    sprite('g_RAINFALL_DOUBLE_TILE_SPRITES[0]', 1, 52, height=100)
    sprite('local4', 2, 333, 113, 56, False)
    for y in [344, 361, 378]:
        for x in [130, 144, 158, 172]: event('pixel', canvas, x, y)
    event('init', 'scratch', 170, 60, 0, None)
    event('remap', 'local4', 'scratch', 'fresh-key-after-init', 0, 0, '53-entry-map')
    texture('g_RAINFALL_SINGLE_TILE_TEXTURES[0]', 0, 0, source='scratch')
    texture('g_RAINFALL_SINGLE_TILE_TEXTURES[1]', 57, 0, source='scratch')
    sprite('local4', 2, 254, 60, 60, False)
    event('fill', 'scratch', 9)
    event('remap', 'local4', 'scratch', 'fresh-key-after-fill', 0, 0, '9-entry-map')
    texture('g_RAINFALL_SINGLE_TILE_TEXTURE', 0, 0, 60, 60, 'scratch')
    load('glow.pcx', 0, 256)
    sprite('g_GLOW_SPRITES[0]', 1, 1)
    sprite('g_GLOW_SPRITES[1]', 102, 1)
    load('TER1WRECK.PCX')
    for y, receivers in [(21, [13, 10, 14]), (84, [6, 7, 8]),
                         (147, [5, 0, 1]), (210, [4, 3, 2]), (273, [12, 9, 11])]:
        for x, receiver in zip([26, 127, 228], receivers):
            sprite(f'g_TER1WRECK_UNITY_WRECKAGE_SPRITES[{receiver}]', x, y)
    for receiver, x, y in [(0,329,147), (1,430,147), (2,329,210), (3,430,210)]:
        sprite(f'g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[{receiver}]', x, y)
    for y, receivers in [(21, [5, 0, 1]), (84, [4, 3, 2])]:
        for x, receiver in zip([329, 430, 531], receivers):
            sprite(f'g_FOSSIL_FIELD_RIDGE_SPRITES[{receiver}]', x, y)
    load('TER1.PCX')
    event('init', 'scratch', 100, 62, 0, None)
    for x, receivers in [(1,[0,1]), (102,[2,3]), (203,[4,5])]:
        for y, receiver in zip([447,579], receivers):
            sprite(f'g_TER1_WHITE_ORG_YEL_TILE_SPRITES[{receiver}]', x, y)
    for x, receivers in [(1,[0,1,2]), (102,[3,4,5]), (203,[6,7,8])]:
        for y, receiver in zip([510,642,705], receivers):
            sprite(f'g_TER1_BOTTOM_LEFT_TILE_SPRITES[{receiver}]', x, y)
    for suffix, x, y in [
        ('UNUSED_SPRITES2[1]',506,1), ('MINE_SPRITE',607,64),
        ('MINING_PLATFORM_SPRITE',506,64), ('SOLAR_COLLECTOR_SPRITE',607,127),
        ('TIDAL_HARNESS_SPRITE',506,127), ('TUT_BLANK_SPRITE',506,190),
        ('KELP_FARM_SPRITE',607,190), ('BUNKER_SPRITE',506,316),
        ('AIRBASE_SPRITE',607,316), ('SENSOR_ARRAY_SPRITE',708,316)]:
        sprite('g_TER1_'+suffix, x, y)
    for receiver, y in enumerate([453,516,579,642,705]):
        sprite(f'g_TER1_UNUSED_SPRITES1[{receiver}]',620,y)
        sprite(f'g_TER1_SOIL_ENRICHER_SPRITES[{receiver}]',822,y)
        sprite(f'g_TER1_FARM_SPRITES[{receiver}]',923,y)
    for suffix, x, y in [
        ('CONDENSER_SPRITE',506,253), ('ECHELON_MIRROR_SPRITE',607,253),
        ('BOREHOLE_SPRITE',708,253), ('BOREHOLE_CLUSTER_SPRITE',708,190),
        ('LANDMARK_RESOURCE_SPRITES[4]',822,253), ('LANDMARK_RESOURCE_SPRITES[5]',822,190)]:
        sprite('g_TER1_'+suffix,x,y)
    for x, receivers in [(478,[3,2]), (579,[4,0]), (680,[5,1])]:
        for y, receiver in zip([632,695], receivers):
            sprite(f'g_TER1_MANIFOLD_NEXUS_SPRITES[{receiver}]',x,y)
    for y, receivers in [(253,[2,3,0,1]), (316,[6,7,4,5]), (379,[10,11,8,9])]:
        for x, receiver in zip([1,102,203,304], receivers):
            sprite(f'g_TER1_SEA_LAND_RESOURCE_SPRITES[{receiver}]',x,y)
    for x, receiver in zip([418,519,620,721,822,923], [1,0,3,2,5,4]):
        sprite(f'g_TER1_UNITY_POD_SPRITES[{receiver}]',x,379)
    sprite('g_TER1_MONOLITH_SPRITE',304,1)
    event('close',canvas)
    return out


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#define __cdecl
#define MEASURED
struct Palette;struct TexHeap;struct ExtDirectDraw;struct IDirectDraw2 {};
struct Buffer;
INTERFACES
DECLARATIONS
bool armed=false;
OBJECTS
INPUT
int scenario, fail_constructor=-1, fail_service=-1, constructor_attempts;
bool error_results, perturb;
size_t cursor;
unsigned int expected_key;
unsigned char expected_palette[12];
Sprite* local_sprites[8];
Buffer* local_buffer;
std::map<const void*,std::string> names;
std::vector<const void*> live;
std::vector<std::string> lifecycle;
std::vector<std::vector<std::string> > expected;
int pixel_count, draw_count, init_count;
IDirectDraw2 mode;
struct Failure {};
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d event %zu\n",line,scenario,cursor);std::_Exit(100);}}
#define require(x) check((x),__LINE__)
std::string number(int x){return std::to_string(x);}
std::string name(const void*p){auto it=names.find(p);require(it!=names.end());return it->second;}
void check_published(){
  require(TextureRasterKey==expected_key);
  require(std::memcmp(TerrainCoastPalette,expected_palette,12)==0);
}
void observe(const char*kind,const void*receiver,std::initializer_list<std::string>args){
  require(armed&&constructor_attempts==9&&live.size()==9);
  if(cursor==0)expected_key=9; // Original451CD2 after all constructors, before load.
  check_published();
  std::vector<std::string>actual={kind,name(receiver)};
  actual.insert(actual.end(),args.begin(),args.end());
  require(cursor<expected.size());
  if(actual!=expected[cursor]){
    for(auto&s:actual)std::fprintf(stderr,"actual %s\n",s.c_str());
    for(auto&s:expected[cursor])std::fprintf(stderr,"expect %s\n",s.c_str());
    require(false);
  }
  int ordinal=int(cursor++);
  if(ordinal==fail_service)throw Failure{};
  if(perturb){
    TextureRasterKey=expected_key=unsigned(1024+cursor);
    BufferDirectDraw=cursor%2?&mode:nullptr;
  }
}
void begin_lifetime(const void*object,bool buffer){
  require(armed);
  require(buffer ? constructor_attempts==8 : constructor_attempts<8);
  const int attempt=constructor_attempts++;
  require(TextureRasterKey==expected_key);
  lifecycle.push_back(std::string("try:")+(buffer?"scratch":"local"+number(attempt)));
  if(attempt==fail_constructor)throw Failure{};
  if(buffer){local_buffer=(Buffer*)object;names[object]="scratch";}
  else{local_sprites[attempt]=(Sprite*)object;names[object]="local"+number(attempt);}
  live.push_back(object);
  lifecycle.push_back("ctor:"+name(object));
}
void end_lifetime(const void*object){
  require(!live.empty()&&live.back()==object);
  lifecycle.push_back("dtor:"+name(object));live.pop_back();
}
Sprite::Sprite(){cTransparentIndex_=9;if(armed)begin_lifetime(this,false);}
void Sprite::close(){if(armed)end_lifetime(this);}
Buffer::Buffer(){if(armed)begin_lifetime(this,true);}
Buffer::~Buffer(){if(armed)end_lifetime(this);}
int Buffer::load_pcx(const char*file,Palette*palette,int first,int count){
  require(palette==nullptr);
  observe("load",this,{file,"null",number(first),number(count)});
  return error_results?7:0;
}
int Buffer::init(int width,int height,int toggle,ExtDirectDraw*draw){
  require(draw==nullptr);
  observe("init",this,{number(width),number(height),number(toggle),"null"});
  ++init_count;
  if(perturb&&init_count==2)local_sprites[4]->cTransparentIndex_=225;
  return error_results?7:0;
}
int Buffer::change_color(int from,int to){
  observe("recolour",this,{number(from),number(to)});return error_results?7:0;
}
int Texture::extract(Buffer*source,int x,int y,int width,int height,TexHeap*heap){
  require(heap==nullptr);
  observe("texture",this,{name(source),number(x),number(y),number(width),number(height),"0"});
  return error_results?7:0;
}
int Sprite::extract(Buffer*source,int key,int x,int y,int width,int height,TexHeap*heap){
  require(heap==nullptr);
  observe("sprite",this,{name(source),number(key),number(x),number(y),number(width),number(height),"null"});
  cTransparentIndex_=perturb?193:static_cast<unsigned char>(key);
  return error_results?7:0;
}
int Buffer::get_pixel(int x,int y){
  observe("pixel",this,{number(x),number(y)});
  const int at=pixel_count++;
  if(perturb){
    // The original only writes this call's destination afterward. Preserve
    // callback edits to both earlier publications and untouched later bytes.
    if(at)TerrainCoastPalette[at-1]=expected_palette[at-1]=static_cast<unsigned char>(77+at);
    if(at<11)TerrainCoastPalette[at+1]=expected_palette[at+1]=static_cast<unsigned char>(181+at);
  }
  int result=(at%2?-0x30000:0x10000)+(17+at*19);
  expected_palette[at]=static_cast<unsigned char>(result);
  return result;
}
int Buffer::fill(int color){
  observe("fill",this,{number(color)});
  if(perturb)local_sprites[4]->cTransparentIndex_=250;
  return error_results?7:0;
}
int Sprite::draw(Buffer*destination,int key,int x,int y,unsigned char*table){
  const int which=draw_count++;
  require(which<2);
  require(key==(perturb?(which?250:225):9));
  observe("remap",this,{name(destination),which?"fresh-key-after-fill":"fresh-key-after-init",
                       number(x),number(y),which?"9-entry-map":"53-entry-map"});
  unsigned char map[256]={};
  if(!which){
    // Expected input is the independently generated synthetic include fixture,
    // not the product's source array read back as its own expected result.
    const unsigned char synthetic[53]={SYNTHETIC};
    for(int n=0;n<53;++n)map[static_cast<unsigned char>(synthetic[n]+10)]=static_cast<unsigned char>(n);
  }else{
    // Nine immediate bytes at4522EF..45232B, not the proprietary 53-byte table.
    const unsigned char indexes[9]={221,218,51,153,126,161,216,105,202};
    for(int n=0;n<9;++n)map[indexes[n]]=static_cast<unsigned char>(n);
  }
  require(std::memcmp(table,map,256)==0);
  if(perturb)std::memset(table,239,256); // Second table must be wholly rebuilt.
  return error_results?7:0;
}
int Sprite::draw_dest(Buffer*,int,int,int,unsigned char*){require(false);return 7;}
void Buffer::close(){observe("close",this,{});require(cursor==486);}
BODY
void setup(){
  armed=false;names.clear();live.clear();lifecycle.clear();expected.clear();
  REGISTRATIONS
  EXPECTED
  require(expected.size()==486);
  cursor=0;constructor_attempts=0;pixel_count=draw_count=init_count=0;
  std::fill(local_sprites,local_sprites+8,nullptr);local_buffer=nullptr;
  TextureRasterKey=expected_key=77;BufferDirectDraw=&mode;
  for(int n=0;n<12;++n)TerrainCoastPalette[n]=expected_palette[n]=static_cast<unsigned char>(211+n);
  armed=true;
}
void check_lifetimes(int successful,int attempted){
  std::vector<std::string>wanted;
  for(int n=0;n<attempted;++n){
    std::string id=n==8?"scratch":"local"+number(n);
    wanted.push_back("try:"+id);
    if(n<successful)wanted.push_back("ctor:"+id);
  }
  for(int n=successful;n-->0;)wanted.push_back("dtor:"+(n==8?std::string("scratch"):"local"+number(n)));
  require(lifecycle==wanted&&live.empty());
}
void run_case(){
  setup();bool caught=false;
  try{load_terrain();}catch(const Failure&){caught=true;}
  const bool intended=fail_constructor>=0||fail_service>=0;
  require(caught==intended);
  if(fail_constructor>=0){
    require(cursor==0&&constructor_attempts==fail_constructor+1);
    check_lifetimes(fail_constructor,fail_constructor+1);
  }else{
    require(cursor==(fail_service>=0?size_t(fail_service+1):486));
    check_lifetimes(9,9);
  }
  check_published();armed=false;
}
int main(){
  // Repeated calls with success/failure returns and callback state perturbation.
  for(scenario=0;scenario<4;++scenario){error_results=scenario&1;perturb=scenario&2;run_case();}
  perturb=true;error_results=true;
  for(int n=0;n<9;++n){scenario=100+n;fail_constructor=n;run_case();}
  fail_constructor=-1;
  // Every service boundary throws independently: exact prefix and reverse
  // object cleanup, not a guessed subset of one exceptional sheet path.
  for(int n=0;n<486;++n){scenario=200+n;fail_service=n;run_case();}
  fail_service=-1;scenario=700;run_case();
  return 0;
}
'''


def run_host(tmp_path, parts, variant=0):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    synthetic = [(n * 37 + 239 + variant * 11) % 256 for n in range(53)]
    synthetic[7] = synthetic[2]
    synthetic[52] = synthetic[0]
    # Explicitly synthetic data, including unsigned-byte wrap and duplicate keys.
    (tmp_path / "terrain_coast_remap.inc").write_text(",".join(map(str, synthetic)))
    manifest = expected_calls()
    assert Counter(event[0] for event in manifest) == dict(
        texture=163, sprite=101, recolour=198, pixel=12, load=5, init=3,
        remap=2, fill=1, close=1)
    rows = []
    for kind, receiver, args in manifest:
        fields = [kind, receiver] + ["null" if x is None else str(x) for x in args]
        rows.append("\t".join(fields))
    # Parse a literal manifest at runtime; hundreds of vector/string constructor
    # expression trees would needlessly dominate each mutation's compile time.
    cpp_events = '''std::istringstream manifest(R"EVENTS(''' + "\n".join(rows) + ''')EVENTS");
    std::string row;
    while(std::getline(manifest,row)){
      std::istringstream fields(row);std::string field;std::vector<std::string> event;
      while(std::getline(fields,field,'\\t'))event.push_back(field);
      expected.push_back(event);
    }'''
    source = HARNESS
    for key, value in dict(parts, EXPECTED=cpp_events,
                           SYNTHETIC=",".join(map(str, synthetic))).items():
        source = source.replace(key, value)
    path, binary = tmp_path / "loader.cpp", tmp_path / "loader"
    path.write_text(source)
    compiled = subprocess.run([compiler, "-std=c++17", "-O0", "-g",
                               "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(path), "-o", str(binary)], capture_output=True,
                              text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


@pytest.mark.parametrize("variant", [0, 1])
def test_actual_terrain_loader(tmp_path, variant):
    result = run_host(tmp_path, actual_source(), variant)
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("unused_lifetime", "Sprite unused[4];", "Sprite unused[3];"),
    ("second_array_lifetime", "Sprite remapped[4];", "Sprite remapped[3];"),
    ("static_lifetime", "Sprite unused[4];", "static Sprite unused[4];"),
    ("buffer_ctor_order", "Sprite unused[4];", "Buffer early_scratch; Sprite unused[4];"),
    ("local_receiver", "remapped[0].extract", "remapped[1].extract"),
    ("raster_key", "TextureRasterKey = 9;", "TextureRasterKey = 8;"),
    ("key_after_load", "TextureRasterKey = 9;", ""),
    ("recolour_order", "change_color(263, 265)", "change_color(262, 265)"),
    ("recolour_skip", "g_PCX_PARSE_TEMP_BUFFER1.change_color(262, 265);", ""),
    ("recolour_failure_exit", "g_PCX_PARSE_TEMP_BUFFER1.change_color(263, 265);",
     "if (g_PCX_PARSE_TEMP_BUFFER1.change_color(263, 265)) return;"),
    ("extract_failure_exit", "remapped[0].extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, 2, 333, 113, 56, 0);",
     "if (remapped[0].extract(&g_PCX_PARSE_TEMP_BUFFER1, 265, 2, 333, 113, 56, 0)) return;"),
    ("load_failure_exit", 'g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TEXTURE.PCX", 0, 10, 236);',
     'if (g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TEXTURE.PCX", 0, 10, 236)) return;'),
    ("init_failure_exit", "scratch.init(28, 28, 0, 0);", "if (scratch.init(28, 28, 0, 0)) return;"),
    ("new_mode_guard", "Sprite unused[4];", "if (BufferDirectDraw) return; Sprite unused[4];"),
    ("rocky_order", "{0, 2, 1, 3}", "{0, 1, 2, 3}"),
    ("fungus_bank", "g_FUNGUS_TEXTURES + 15", "g_FUNGUS_TEXTURES + 14"),
    ("grid_x_pitch", "(index % columns) * 57", "(index % columns) * 56"),
    ("grid_y_pitch", "(index / columns) * 57", "(index / columns) * 56"),
    ("jungle_last", "g_JUNGLE_LAND_TEXTURES, 15, 4", "g_JUNGLE_LAND_TEXTURES, 14, 4"),
    ("grid_failure_exit", "textures[index].extract(&g_PCX_PARSE_TEMP_BUFFER1,",
     "if (textures[index].extract(&g_PCX_PARSE_TEMP_BUFFER1,"),
    ("glow_palette", '"glow.pcx", 0, 0, 256', '"glow.pcx", 0, 10, 236'),
    ("sheet_boundary", '"TER1WRECK.PCX"', '"TER1.PCX"'),
    ("initial_scratch_size", "scratch.init(28, 28, 0, 0);", "scratch.init(56, 56, 0, 0);"),
    ("third_init", "scratch.init(100, 62, 0, 0);", ""),
    ("remap_sprite_dimensions", "265, 2, 333, 113, 56", "265, 2, 333, 100, 56"),
    ("scratch_texture_owner", "g_RAINFALL_SINGLE_TILE_TEXTURES[1].extract", "g_RAINFALL_SINGLE_TILE_TEXTURES[0].extract"),
    ("wreck_order", "{13,10,14,6,7,8,5,0,1,4,3,2,12,9,11}", "{10,13,14,6,7,8,5,0,1,4,3,2,12,9,11}"),
    ("fossil_order", "{5,0,1,4,3,2}", "{0,5,1,4,3,2}"),
    ("manifold_order", "{3,2,4,0,5,1}", "{2,3,4,0,5,1}"),
    ("interleaved_ter1", "g_TER1_WHITE_ORG_YEL_TILE_SPRITES[n * 2 + 1]", "g_TER1_WHITE_ORG_YEL_TILE_SPRITES[n * 2]"),
    ("resource_permutation", "g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4 + 2]", "g_TER1_SEA_LAND_RESOURCE_SPRITES[n * 4]"),
    ("pod_permutation", "g_TER1_UNITY_POD_SPRITES[n ^ 1]", "g_TER1_UNITY_POD_SPRITES[n]"),
    ("palette_column", "130 + column * 14", "130 + column * 13"),
    ("palette_row", "344 + row * 17", "344 + row * 18"),
    ("palette_publication", "TerrainCoastPalette[row * 4 + column] =", "TerrainCoastPalette[(row * 4 + column + 1) % 12] ="),
    ("delayed_palette_publication", "TerrainCoastPalette[row * 4 + column] =", "palette_batch[row * 4 + column] ="),
    ("remap_offset", "TerrainCoastRemapSource[index] + 10", "TerrainCoastRemapSource[index] + 9"),
    ("remap_last", "index < 53", "index < 52"),
    ("remap_ordinal", "remap[color] = static_cast<unsigned char>(index);", "remap[color] = static_cast<unsigned char>(index + 1);"),
    ("remap_fill", "scratch.fill(9);", "scratch.fill(8);"),
    ("wrong_remap_sibling", "remapped[0].draw(", "remapped[0].draw_dest("),
    ("cached_init_key", "scratch.init(170, 60, 0, 0);", "const int stale_key = remapped[0].transparent_index(); scratch.init(170, 60, 0, 0);"),
    ("cached_fill_key", "scratch.fill(9);", "const int stale_key = remapped[0].transparent_index(); scratch.fill(9);"),
    ("second_table_clear", "memset(remap, 0, sizeof(remap));", ""),
    ("missing_close", "g_PCX_PARSE_TEMP_BUFFER1.close();", ""),
    ("premature_close", 'g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TER1.PCX", 0, 10, 236);',
     'g_PCX_PARSE_TEMP_BUFFER1.close(); g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TER1.PCX", 0, 10, 236);'),
]


@pytest.mark.parametrize("name,before,after", DAMAGE, ids=[row[0] for row in DAMAGE])
def test_compiling_terrain_loader_damage(tmp_path, name, before, after):
    parts = actual_source()
    original = parts["BODY"]
    assert before in original, name
    if name == "second_table_clear":
        assert original.count(before) == 2
        at = original.rindex(before)
        parts["BODY"] = original[:at] + original[at:].replace(before, after, 1)
    else:
        parts["BODY"] = original.replace(before, after, 1)
    if name == "key_after_load":
        anchor = 'g_PCX_PARSE_TEMP_BUFFER1.load_pcx("TEXTURE.PCX", 0, 10, 236);'
        parts["BODY"] = parts["BODY"].replace(anchor, anchor + " TextureRasterKey = 9;", 1)
    if name == "grid_failure_exit":
        parts["BODY"] = parts["BODY"].replace("56, 56, 0);", "56, 56, 0)) return;", 1)
    if name == "delayed_palette_publication":
        parts["BODY"] = parts["BODY"].replace("TextureRasterKey = 9;",
                                                "unsigned char palette_batch[12]; TextureRasterKey = 9;", 1)
        parts["BODY"] = parts["BODY"].replace("unsigned char remap[256];",
                                                "memcpy(TerrainCoastPalette, palette_batch, 12); unsigned char remap[256];", 1)
    if name in ("cached_init_key", "cached_fill_key"):
        draw = "remapped[0].draw(&scratch, remapped[0].transparent_index(), 0, 0, remap);"
        at = parts["BODY"].index(draw) if name == "cached_init_key" else parts["BODY"].rindex(draw)
        parts["BODY"] = parts["BODY"][:at] + parts["BODY"][at:].replace(
            "remapped[0].transparent_index()", "stale_key", 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 100 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)
