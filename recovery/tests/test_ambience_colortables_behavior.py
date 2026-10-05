"""Execute the exact proposed C++ bodies with narrow host owner doubles.

This test is evidence for control flow and data ownership only. It is neither
the application's compiler nor a native ABI/byte-match claim.
"""

from pathlib import Path
import subprocess
import tempfile


HERE = Path(__file__).parent
ROOT = next(parent for parent in HERE.parents if (parent / "src").is_dir())
FALLBACK_SOURCE = ROOT / "build/recovery/ambience-colortables-owner-oct5/candidate.cpp"


def extract(signature: str) -> str:
    source = None
    for product_file in (ROOT / "src/ambience.cpp", ROOT / "src/game.cpp"):
        if product_file.is_file():
            text = product_file.read_text()
            if signature in text:
                source = text
                break
    if source is None:
        source = FALLBACK_SOURCE.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


HARNESS = r"""
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <string>
#include <utility>
#include <vector>

#define __cdecl
using DWORD = uint32_t;
struct PALETTEENTRY { uint8_t peRed, peGreen, peBlue, peFlags; };
struct HSV { int h, s, v; };
struct Player { char filename[32]; };
Player Players[8] = {{"f0"}};
char StringTemp[256];
uint32_t GamePreferences;
const int WinFillColour = 255;
int InfoFactionTextColors[8], InfoFactionShadowColors[8];
int ChatItemIdTable[8], ChatShadowColors[8];
uint8_t TerritoryColorTables[8][256];
uint8_t BodyTintColorTables[8][24][256];
std::vector<std::string> events;

struct FactionAmbience {
  int calls[14] = {};
  void general() { ++calls[0]; events.push_back("event:general"); }
  void tech() { ++calls[1]; events.push_back("event:tech"); }
  void popup1() { ++calls[2]; events.push_back("event:popup1"); }
  void eot() { ++calls[3]; events.push_back("event:eot"); }
  void hostility() { ++calls[4]; events.push_back("event:hostility"); }
  void basewin_show() { ++calls[5]; events.push_back("event:basewin_show"); }
  void basewin_hide() { ++calls[6]; events.push_back("event:basewin_hide"); }
  void terraform() { ++calls[7]; events.push_back("event:terraform"); }
  void new_base() { ++calls[8]; events.push_back("event:new_base"); }
  void energy_resources() { ++calls[9]; events.push_back("event:energy_resources"); }
  void design_window_show() { ++calls[10]; events.push_back("event:design_window_show"); }
  void design_window_hide() { ++calls[11]; events.push_back("event:design_window_hide"); }
  void base_liberated() { ++calls[12]; events.push_back("event:base_liberated"); }
};
FactionAmbience music_instance;
FactionAmbience *FactionMusic = &music_instance;
int AmbienceCurrentEffect = -1;
int AmbienceTransitionMarker = 0;
struct Effect {
  int playing = 0;
  int id = 0;
  void fade() { events.push_back("fade:" + std::to_string(id)); }
  int is_playing() { events.push_back("playing:" + std::to_string(id)); return playing; }
  void stop() { events.push_back("stop:" + std::to_string(id)); playing = 0; }
};
struct FX {
  Effect effects[256];
  int played = -1;
  FX() { for (int i=0;i<256;++i) effects[i].id=i; }
  Effect &effect(int id) { return effects[id]; }
  void play(int id) { played = id; events.push_back("play:" + std::to_string(id)); }
};
FX g_FX;

std::vector<std::string> calls;
struct ExtractCall { int x,y,w,h; };
std::vector<ExtractCall> extracts;
std::vector<std::pair<int,int>> sample_calls;
struct Buffer {
  Buffer() { calls.push_back("Buffer-ctor"); }
  ~Buffer() { calls.push_back("Buffer-dtor"); }
  std::deque<int> results;
  std::vector<std::string> loaded;
  int load_pcx(const char *name, void *, int, int) {
    loaded.emplace_back(name); calls.push_back(std::string("load:")+name);
    if (results.empty()) return 0;
    int result=results.front(); results.pop_front(); return result;
  }
  int get_pixel(int x, int y) {
    sample_calls.emplace_back(x,y);
    if (x==0xA1 && y==0x2EB) return 200;
    if (x==0xA1 && y==0x2F5) return 5;
    if (x==0x1B3 && y==0x2E9) return 77;
    if (x==4 && y==0x2E3) return 11;
    if (x==4 && y==0x2EB) return 12;
    if (x==4 && y==0x2F3) return 13;
    if (x==4 && y==0x2FB) return 14;
    return 0;
  }
  void close() { calls.push_back("canvas-close"); }
};
Buffer g_PCX_PARSE_TEMP_BUFFER1;
struct Sprite {
  int count = 0;
  void extract(Buffer *, int, int x, int y, int w, int h, void *) {
    ++count; extracts.push_back({x,y,w,h});
  }
};
struct FactionArt {
  Sprite base[2][4][3]; Sprite council_logo[2]; Sprite council_small;
  Sprite council_big; Sprite diplomacy; Sprite diplomacy_logo;
  Sprite diplomacy_landscape; Sprite datalink; Sprite report_logo_small[3];
  Sprite report_logo[2];
};
FactionArt FactionsArt[8];
int colortables_calls=0;
void colortables_init_faction(int id) { assert(id==0); ++colortables_calls; calls.push_back("colortables"); }

struct Palette {
  Palette() { calls.push_back("Palette-ctor"); }
  ~Palette() { calls.push_back("Palette-dtor"); }
};
struct CvrRecord { uint8_t palette_[0x302 + 24*256]; } record_data;
struct CaviarData { CvrRecord *record() { return &record_data; } };
CaviarData g_VBP_CAVIARDATA;
struct PaletteOwner {
  PALETTEENTRY colors[256]; int create_calls=0, nearest_calls=0;
  int last_index=-1, last_low=-1, last_high=-1, last_bg=-1, last_fg=-1;
  PaletteOwner() { for(int i=0;i<256;++i) colors[i]={(uint8_t)i,(uint8_t)i,(uint8_t)i,0}; }
  int create_table_from_color(int index, uint8_t *out, int low, int high, int bg, int fg) {
    ++create_calls; last_index=index; last_low=low; last_high=high; last_bg=bg; last_fg=fg;
    for(int i=0;i<256;++i) out[i]=(uint8_t)(i^index); return 0;
  }
  PALETTEENTRY *entries() { return colors; }
  int get_nearest_palette_index(uint8_t r,uint8_t g,uint8_t b,int) { ++nearest_calls; return (r+g+b)&255; }
} g_PALETTE1;
int compare_result=0;
int compare_palettes() { calls.push_back("compare-palettes"); return compare_result; }
void RGB_to_HSV(const PALETTEENTRY *p, HSV *h) { h->h=p->peRed; h->s=p->peGreen; h->v=p->peBlue; }

uint8_t value_bytes[0x80], table_bytes[0xC100];
bool open_values=false, open_short_table=false;
int values_create_failures=0, table_create_failures=0;
uint32_t opened_size=0;
int clock_value=0;
DWORD timeGetTime() { calls.push_back("clock"); clock_value+=100; return (DWORD)clock_value; }
struct Filemap {
  Filemap() { calls.push_back("Filemap-ctor"); }
  ~Filemap() { calls.push_back("Filemap-dtor"); }
  std::string name; uint32_t size=0; std::vector<std::string> local_ops;
  void *open(char *path, int) {
    name=path; calls.push_back(std::string("open:")+path);
    if (name.find("vals.tmp")!=std::string::npos && open_values) { size=12; return value_bytes; }
    if (name.find(".tmp")!=std::string::npos && open_short_table) { size=16; return table_bytes; }
    return nullptr;
  }
  void *create(char *path, unsigned long extent, int) {
    name=path; calls.push_back(std::string("create:")+path+":"+std::to_string(extent));
    if (name.find("vals.tmp")!=std::string::npos) {
      if(values_create_failures-- > 0) return nullptr;
      size=(uint32_t)extent; return value_bytes;
    }
    if(table_create_failures-- > 0) return nullptr;
    size=(uint32_t)extent; return table_bytes;
  }
  void close() { calls.push_back("file-close"); }
  void close(unsigned char *end) {
    assert(end==value_bytes+12 || end==table_bytes+12);
    opened_size=12; calls.push_back("values-truncate:12");
  }
  uint32_t get_size() const { return size; }
};

"""


TESTS = r"""
static void reset_events() { events.clear(); }
static void test_ambience() {
  GamePreferences=0; AmbienceCurrentEffect=-1; AmbienceTransitionMarker=0;
  reset_events(); assert(ambience(31)==31); assert(events.empty());
  GamePreferences=0x800; FactionMusic=&music_instance;
  assert(ambience(0xC8)==0xC8 && music_instance.calls[0]==1);
  assert(ambience(0xD0)==0xD0 && music_instance.calls[0]==2);
  FactionMusic=nullptr; assert(ambience(0xC9)==0xC9);
  FactionMusic=&music_instance;
  AmbienceCurrentEffect=3; g_FX.effects[4].playing=1;
  reset_events(); assert(ambience(4)==4);
  const std::vector<std::string> expected={"fade:3","playing:4","stop:4","play:4"};
  assert(events==expected && AmbienceCurrentEffect==4 && AmbienceTransitionMarker==-1);
  reset_events(); assert(ambience(4)==4 && events.empty());
  assert(ambience(-1)==-1 && AmbienceCurrentEffect==-1);
}

static void test_art_fallback_and_extractions() {
  calls.clear(); extracts.clear(); colortables_calls=0;
  g_PCX_PARSE_TEMP_BUFFER1.loaded.clear();
  g_PCX_PARSE_TEMP_BUFFER1.results={0};
  load_faction_art(0);
  assert(colortables_calls==1);
  assert(g_PCX_PARSE_TEMP_BUFFER1.loaded.size()==1);
  assert(g_PCX_PARSE_TEMP_BUFFER1.loaded[0]=="f0.pcx");
  assert(extracts.size()==37);
  assert(extracts[0].x==1 && extracts[0].y==1 && extracts[0].w==0x64 && extracts[0].h==0x4B);
  assert(extracts[1].x==0x66 && extracts[1].y==1);
  assert(extracts[24].x==1 && extracts[24].y==0x1C9);
  assert(InfoFactionTextColors[0]==11 && InfoFactionShadowColors[0]==12);
  assert(ChatItemIdTable[0]==13 && ChatShadowColors[0]==14);

  extracts.clear(); g_PCX_PARSE_TEMP_BUFFER1.loaded.clear();
  g_PCX_PARSE_TEMP_BUFFER1.results={1,0};
  load_faction_art(0);
  assert(g_PCX_PARSE_TEMP_BUFFER1.loaded.size()==2);
  assert(g_PCX_PARSE_TEMP_BUFFER1.loaded[0]=="f0.pcx");
  assert(g_PCX_PARSE_TEMP_BUFFER1.loaded[1]=="faction.pcx");
  assert(extracts.size()==37);
  assert(extracts[36].x==0x304 && extracts[36].y==0x3D);
}

static void test_colortable_native_control() {
  calls.clear(); clock_value=0; compare_result=0; open_values=false;
  open_short_table=false; values_create_failures=0; table_create_failures=0;
  g_PALETTE1.create_calls=0; g_PALETTE1.nearest_calls=0;
  sample_calls.clear();
  colortables_real(0);
  auto palette_ctor=std::find(calls.begin(),calls.end(),"Palette-ctor");
  auto filemap_ctor=std::find(calls.begin(),calls.end(),"Filemap-ctor");
  auto buffer_ctor=std::find(calls.begin(),calls.end(),"Buffer-ctor");
  assert(sample_calls.size()==3);
  assert((sample_calls[0]==std::pair<int,int>(0xA1,0x2EB)));
  assert((sample_calls[1]==std::pair<int,int>(0xA1,0x2F5)));
  assert((sample_calls[2]==std::pair<int,int>(0x1B3,0x2E9)));
  assert(g_PALETTE1.create_calls==1 && g_PALETTE1.last_index==200);
  assert(g_PALETTE1.last_low==10 && g_PALETTE1.last_high==0xEC);
  assert(g_PALETTE1.last_bg==2 && g_PALETTE1.last_fg==98);
  assert(g_PALETTE1.nearest_calls==24*256);
  assert(opened_size==12);
  assert(palette_ctor<filemap_ctor && filemap_ctor<buffer_ctor);
  assert(buffer_ctor<std::find(calls.begin(),calls.end(),"compare-palettes"));
  auto buffer_dtor=std::find(calls.begin(),calls.end(),"Buffer-dtor");
  auto filemap_dtor=std::find(calls.begin(),calls.end(),"Filemap-dtor");
  auto palette_dtor=std::find(calls.begin(),calls.end(),"Palette-dtor");
  assert(buffer_dtor<filemap_dtor && filemap_dtor<palette_dtor);
  auto open=std::find(calls.begin(),calls.end(),"open:f0vals.tmp");
  auto clock=std::find(open,calls.end(),"clock");
  assert(open!=calls.end() && clock!=calls.end() && open<clock);
  assert(TerritoryColorTables[0][200]==(200^200));

  // A non-null short .tmp mapping is invalid and must still be recreated.
  calls.clear(); sample_calls.clear();
  const uint32_t key[3]={98,200,77};
  std::memcpy(value_bytes,key,sizeof(key));
  open_values=true; open_short_table=true; compare_result=0;
  colortables_real(0);
  assert(opened_size==12);
  assert(std::find(calls.begin(),calls.end(),"create:f0.tmp:49408")!=calls.end());

  // A failed output create loops back through the original shared filename
  // buffer, creates/truncates the same .tmp path as a 0x80-byte map, and then
  // begins a new output-cache retry window.
  calls.clear(); sample_calls.clear(); clock_value=0;
  open_values=false; open_short_table=false; table_create_failures=1;
  colortables_real(0);
  auto failed_output=std::find(calls.begin(),calls.end(),"create:f0.tmp:49408");
  assert(failed_output!=calls.end());
  assert(std::find(calls.begin(),calls.end(),"create:f0.tmp:128")!=calls.end());
  assert(std::find(failed_output+1,calls.end(),"create:f0.tmp:49408")!=calls.end());
}

int main() { test_ambience(); test_art_fallback_and_extractions(); test_colortable_native_control(); }
"""


def test_extracted_cpp_bodies_execute_under_native_behavior_controls():
    tables = extract("void __cdecl colortables_init_faction(int faction_id)")
    tables = tables.replace("void __cdecl colortables_init_faction(",
                            "void colortables_real(", 1)
    code = (HARNESS + "\n" + extract("int __cdecl ambience(int ambience_id)") + "\n" +
            tables + "\n" +
            extract("void __cdecl load_faction_art(int faction_id)") + "\n" + TESTS)
    with tempfile.TemporaryDirectory(prefix="opensmacx-host-control-") as directory:
        exe = Path(directory) / "control"
        build = subprocess.run(
            ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-x", "c++", "-", "-o", str(exe)],
            input=code, text=True, capture_output=True, check=False)
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True, check=False)
        assert run.returncode == 0, run.stdout + run.stderr

        # Compile and execute damaged versions too: the behavior fixtures must
        # reject real control-flow mutants, not merely find expected substrings.
        mutations = (
            (extract("int __cdecl ambience(int ambience_id)"),
             "GamePreferences & 0x800", "GamePreferences & 0x400"),
            (extract("int __cdecl ambience(int ambience_id)"),
             "g_FX.effect(previous_effect).fade()",
             "g_FX.effect(previous_effect).stop()"),
            (tables, "cache_file.close(saved + 12)",
             "cache_file.close(saved + 8)"),
            (tables, "values_view = cache_file.create(StringTemp, 0x80, 1);",
             "values_view = cache_file.create(StringTemp, 0x100, 1);"),
            (extract("void __cdecl load_faction_art(int faction_id)"),
             "if (canvas.load_pcx(StringTemp, nullptr, 10, 0xEC) != 0)",
             "if (canvas.load_pcx(StringTemp, nullptr, 10, 0xEC) == 0)"),
        )
        for index, (original, before, after) in enumerate(mutations):
            assert before in original
            mutant = code.replace(original, original.replace(before, after), 1)
            mutant_exe = Path(directory) / f"mutant-{index}"
            mutant_build = subprocess.run(
                ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror",
                 "-pedantic", "-x", "c++", "-", "-o", str(mutant_exe)],
                input=mutant, text=True, capture_output=True, check=False)
            assert mutant_build.returncode == 0, mutant_build.stdout + mutant_build.stderr
            mutant_run = subprocess.run(
                [str(mutant_exe)], capture_output=True, text=True, check=False)
            assert mutant_run.returncode != 0, f"behavior mutant {index} escaped"
