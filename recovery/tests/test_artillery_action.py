"""Actual artillery command with service doubles, not combat/runtime proof."""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]

def body():
    source = (ROOT / "src/veh.cpp").read_text()
    start = source.index("void __cdecl action_arty(int veh_id, int x, int y) {")
    return source[start:source.index("\n}", start) + 2]

def live_boom_body():
    source = (ROOT / "src/veh.cpp").read_text()
    tables = []
    for name in ("BoomIndexedSpriteBankByWeapon", "BoomColorGroupByWeapon"):
        start = source.index(f"static const int {name}[MaxWeaponNum] = {{")
        end = source.index("\n};", start) + 3
        tables.append(source[start:end])
    helper_start = source.index("static int boom_zoomed_extent(int zoom, int extent) {")
    helper_end = source.index("\n}", helper_start) + 2
    body_start = source.index("int __cdecl boom(int x, int y, int flags) {")
    body_end = source.index("\n}", body_start) + 2
    return "\n".join(tables) + "\n" + source[helper_start:helper_end] + "\n" + source[body_start:body_end]

HARNESS = r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#define __cdecl
struct Veh {int faction_id,x,y;} Vehs[8];
struct {int faction_id_;}ConsoleGlobal;
struct {unsigned diplo_treaties[8];}PlayersData[8];
struct {unsigned artillery_max_rng;}Rules;
const int RadiusRange[]={1,9,25,49,81,121,169,225,289};
int MapIsFlat,MapLongitude=10,MapLongitudeBounds=20,IsMultiplayerNet;
int ready=1,target=1,direction=4,lock_result,maybe_result,fight_result;
int calls,observed_mode,observed_id,opcode,notify,destroyed,unlocked,awaited;
struct NetDaemon {
 struct {int dword_[4];}lock_state_[2];
 int lock_veh(int*id,int mode,int x,int y,int zero){
  assert(mode==4&&x==2&&y==0&&zero==0);*id=3;return lock_result;}
 int maybe_lock(int*id,int a,int b,int x,int y,int mode){
  assert(a==0&&b==0&&x==2&&y==0&&mode==4);*id=3;return maybe_result;}
 void await_exec(int n){assert(n==1);++awaited;}
 void unlock_veh(){++unlocked;}
}NetDaemonNet;
struct Message {void pop(const char*key,int t,int zero,const char*p){
 assert(t==5000&&zero==0&&!p);notify=!std::strcmp(key,"UNITMOVED")?1:2;
}}message;Message*NetMsgPopup=&message;
int veh_ready(int){return ready;}
int veh_at(int,int){return target;}
int stack_fix(int n){return n;}
void action_destroy(int id,int bits,int x,int y){assert(id==0&&bits==0&&x==2&&y==0);++destroyed;}
void message_data(int op,int zero,int id,int x,int y,int end){
 assert(zero==0&&id==3&&x==2&&y==0&&end==0);opcode=op;}
void message_veh(int op,int id,int dir,int zero){assert(id==3&&dir==4&&zero==0);opcode=op;}
int radius_move(int x,int y,int,int,int range){assert(x==0&&y==0&&range==25);return direction;}
int battle_fight(int id,int dir,int a,int b,int*p){
 assert(dir==4&&a==1&&b==1&&!p);++calls;observed_id=id;
 observed_mode=NetDaemonNet.lock_state_[1].dword_[3];return fight_result;}
BODY
void reset(){std::memset(PlayersData,0,sizeof PlayersData);std::memset(Vehs,0,sizeof Vehs);
 Vehs[1].faction_id=1;ready=1;target=1;direction=4;lock_result=maybe_result=fight_result=0;
 MapIsFlat=1;IsMultiplayerNet=0;Rules.artillery_max_rng=2;
 calls=observed_mode=observed_id=opcode=notify=destroyed=unlocked=awaited=0;
 NetDaemonNet.lock_state_[1].dword_[3]=17;}
int main(){
 reset();ready=0;action_arty(0,2,0);assert(notify==1&&calls==0);
 reset();action_arty(0,6,0);assert(notify==2&&calls==0);
 reset();MapIsFlat=0;action_arty(0,15,0);assert(notify==0&&calls==1);
 reset();Vehs[1].faction_id=0;action_arty(0,2,0);assert(calls==0);
 reset();PlayersData[0].diplo_treaties[1]=1;action_arty(0,2,0);assert(calls==0);
 reset();PlayersData[0].diplo_treaties[1]=2;action_arty(0,2,0);assert(calls==1);
 reset();direction=-1;action_arty(0,2,0);assert(calls==0);
 reset();target=-1;action_arty(0,2,0);assert(destroyed==1&&calls==0);
 reset();target=-1;IsMultiplayerNet=1;action_arty(0,2,0);assert(opcode==0x2413&&awaited==1);
 reset();target=-1;IsMultiplayerNet=1;lock_result=1;action_arty(0,2,0);assert(opcode==0&&awaited==0);
 reset();action_arty(0,2,0);assert(calls==1&&observed_mode==3&&observed_id==0);
 reset();IsMultiplayerNet=1;maybe_result=1;action_arty(0,2,0);
 assert(calls==0&&NetDaemonNet.lock_state_[1].dword_[3]==17);
 reset();IsMultiplayerNet=1;fight_result=1;action_arty(0,2,0);
 assert(calls==1&&observed_mode==1&&observed_id==3&&unlocked==1&&opcode==0&&awaited==0);
 reset();IsMultiplayerNet=1;action_arty(0,2,0);
 assert(calls==1&&observed_mode==1&&observed_id==3&&unlocked==0&&opcode==0x2402&&awaited==1);
}
'''

def execute(tmp_path, candidate, label):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
                   input=HARNESS.replace("BODY", candidate), text=True,
                   capture_output=True, check=True)
    return subprocess.run([str(executable)], capture_output=True).returncode

def test_artillery_branch_and_network_protocol(tmp_path):
    assert execute(tmp_path, body(), "artillery") == 0

@pytest.mark.parametrize("before,after", [
    ("diplo_treaties[target_faction] & 1U", "diplo_treaties[target_faction] & 2U"),
    ("dword_[3] = 3;", "dword_[3] = 1;"),
    ("message_veh(0x2402", "message_veh(0x2413"),
])
def test_artillery_damage_is_detected(tmp_path, before, after):
    candidate = body()
    assert before in candidate
    assert execute(tmp_path, candidate.replace(before, after), "damage") != 0


BOOM_HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
struct RECT { int left, top, right, bottom; };
struct Buffer;
struct CopyCall {
    Buffer *source;
    Buffer *destination;
    int sx, sy, dx, dy, width, height;
};
struct DrawCall { int sprite; unsigned char *table; };
struct InitCall { Buffer *buffer; int width, height; };
static std::vector<CopyCall> copies;
static std::vector<DrawCall> sprite_draws;
static std::vector<InitCall> inits;
static int next_buffer_id, non_input_calls, update_calls, tile_draw_calls;
static int fake_tick, shift_state, clock_waits, flic_init_calls;
static int flic_init_result, mutate_on_initial_fx;
static int fx_calls[128];
struct Buffer {
    int id;
    bool map_window_buffer;
    int width, height;
    Buffer() : id(++next_buffer_id), map_window_buffer(false), width(0), height(0) {}
    int init(int w, int h, int, void *) {
        width = w; height = h; inits.push_back(InitCall{this,w,h}); return 0;
    }
    int copy(Buffer *dst, int sx, int sy, int dx, int dy, int w, int h) {
        copies.push_back(CopyCall{this,dst,sx,sy,dx,dy,w,h}); return 0;
    }
    void close() {}
    void set_clip(RECT *) {}
    int draw_multi_table_dest(Buffer *, int, int, int, int, int, unsigned char *) {
        return 0;
    }
};
struct Flic {
    int init(Buffer *, char *, int, int, int) { ++flic_init_calls; return flic_init_result; }
    int frame_count() const { return 4; }
    void decode_stored_frame() {}
    void close() {}
};
struct Veh { int faction_id, x, y, proto_id; } Vehs[8];
struct VehPrototype { int chassis_id, weapon_id, reactor_id; } VehPrototypes[32];
const int CHSI_MISSILE = 8, BSC_SPORE_LAUNCHER = 15;
const int MaxWeaponNum = 26;
int VehDrawSelection689F28, VehDrawSelection689F2C, VehDrawSuppressedVehicle;
int BoomAnimationLevel;
static unsigned char color_table_bytes[8];
unsigned char *ExplosionColorTables[8][1];
struct Sprite {
    int index, sprite_width, sprite_height;
    int width() const { return sprite_width; }
    int height() const { return sprite_height; }
    int transparent_index() const { return 0xff; }
    int draw_multi_table_dest(Buffer *, int, int, int, unsigned char *table) {
        sprite_draws.push_back(DrawCall{index,table}); return 0;
    }
};
Sprite g_XF_BOOM_VEH_SPRITES[64];
Sprite g_XI_BOOM_VEH_SPRITES[144];
struct GraphicWin : Buffer {
    RECT work_rect;
    void update(RECT *, int) { ++update_calls; }
    RECT *buffer_working_rect() { return &work_rect; }
};
struct MapWin : GraphicWin {
    int field_1DD74_, field_1DDA8_, field_1DDD8_, field_1DDD0_;
    int field_1DDA4_, field_1DDD4_, field_1DDCC_;
    int field_1DDB4_, field_1DDB8_, field_1DD98_;
    void draw_tile(int, int, int) { ++tile_draw_calls; }
};
MapWin *MapWinTable[8];
struct Console : GraphicWin {
    void tile_to_pixel(int, int, int *x, int *y) { *x = 100; *y = 50; }
} ConsoleGlobal;
int MapLongitudeBounds=20, MapLongitude=10, MapIsFlat=1;
int MapWinCaptionScale=0, MapWinCaptionDivisor=1;
unsigned GamePreferences;
int IsMultiplayerNet, TimerHoldFlag, SpriteScaleNumerator, SpriteScaleDenominator;
struct FX {
    void play(int sound) {
        ++fx_calls[sound];
        if (sound == 1 && mutate_on_initial_fx) {
            VehPrototypes[Vehs[VehDrawSelection689F28].proto_id].weapon_id = 6;
        }
    }
    void play(int sound, int, int) { ++fx_calls[sound]; }
} g_FX;
typedef short SHORT;
const int VK_SHIFT=0x10;
SHORT GetAsyncKeyState(int) { return static_cast<SHORT>(shift_state); }
unsigned long timeGetTime() { return static_cast<unsigned long>(++fake_tick); }
void do_non_input() { ++non_input_calls; }
void clock_wait(int ms) { assert(ms==50); ++clock_waits; }

BODY

static MapWin window;
void reset_case() {
    copies.clear(); sprite_draws.clear(); inits.clear();
    for (int i=0;i<128;++i) fx_calls[i]=0;
    for (int i=0;i<8;++i) {
        MapWinTable[i]=0; ExplosionColorTables[i][0]=&color_table_bytes[i];
    }
    MapWinTable[0]=&window; window.map_window_buffer=true;
    window.field_1DD74_=1; window.field_1DDA8_=0;
    window.field_1DDD8_=20; window.field_1DDD0_=0;
    window.field_1DDA4_=0; window.field_1DDD4_=20; window.field_1DDCC_=0;
    window.field_1DDB4_=0; window.field_1DDB8_=0; window.field_1DD98_=0;
    for (int i=0;i<8;++i) { Vehs[i]=Veh{0,4,5,0}; }
    for (int i=0;i<32;++i) VehPrototypes[i]=VehPrototype{0,0,0};
    Vehs[0].proto_id=0; Vehs[1].proto_id=1;
    VehDrawSelection689F28=0; VehDrawSelection689F2C=1;
    for (int i=0;i<64;++i) g_XF_BOOM_VEH_SPRITES[i]=Sprite{i,100,40};
    for (int i=0;i<144;++i) g_XI_BOOM_VEH_SPRITES[i]=Sprite{i,80,32};
    MapIsFlat=1; MapLongitudeBounds=20; MapLongitude=10;
    MapWinCaptionScale=0; MapWinCaptionDivisor=1;
    GamePreferences=0; IsMultiplayerNet=0; shift_state=0;
    fake_tick=non_input_calls=update_calls=tile_draw_calls=clock_waits=0;
    flic_init_calls=0; flic_init_result=0; mutate_on_initial_fx=0;
    TimerHoldFlag=0; SpriteScaleNumerator=1; SpriteScaleDenominator=1;
    BoomAnimationLevel=1;
    VehDrawSuppressedVehicle=-1;
}
int main() {
    reset_case();
    assert(boom(4,5,0)==1);
    int capture=-1, restore=-1;
    for (unsigned i=0;i<copies.size();++i) {
        if (copies[i].source->map_window_buffer) capture=i;
        if (copies[i].destination->map_window_buffer) restore=i;
    }
    assert(capture>=0 && restore>=0);
    const CopyCall &c=copies[capture];
    assert(c.sx==50 && c.sy==10 && c.dx==0 && c.dy==0);
    assert(c.width==100 && c.height==-40); // half-width centering, full-width rect
    const CopyCall &r=copies[restore];
    assert(!r.source->map_window_buffer && r.destination->map_window_buffer);
    assert(r.sx==0 && r.sy==0 && r.dx==50 && r.dy==10);
    assert(r.width==100 && r.height==-40); // six-int copy source/destination order

    reset_case(); Vehs[0].proto_id=8;
    assert(boom(4,5,2)==1);
    assert(!sprite_draws.empty());
    assert(sprite_draws[0].sprite>=48 && sprite_draws[0].sprite<64);
    assert(sprite_draws[0].table==&color_table_bytes[6]); // bank 3 => row 6

    reset_case(); Vehs[0].proto_id=3; VehPrototypes[3].reactor_id=2;
    assert(boom(4,5,2)==1);
    assert(!sprite_draws.empty());
    assert(sprite_draws[0].sprite>=16 && sprite_draws[0].sprite<32);
    assert(sprite_draws[0].table==&color_table_bytes[0]); // other bank => row 0

    reset_case(); Vehs[1].proto_id=1; mutate_on_initial_fx=1;
    assert(boom(4,5,1)==1);
    assert(fx_calls[1]>0);
    assert(!sprite_draws.empty() && sprite_draws[0].sprite>=80 &&
           sprite_draws[0].sprite<96);
    assert(sprite_draws[0].table==&color_table_bytes[5]); // callback-updated weapon 6

    reset_case(); GamePreferences=0;
    assert(boom(4,5,0)==1); assert(non_input_calls==16*25);
    reset_case(); GamePreferences=8;
    assert(boom(4,5,0)==1); assert(non_input_calls==16);
    reset_case(); GamePreferences=8;
    assert(boom(4,5,2)==1); assert(non_input_calls==16*50);
    reset_case(); shift_state=0x0100;
    assert(boom(4,5,2)==1); assert(non_input_calls==16);

    reset_case(); IsMultiplayerNet=1;
    assert(boom(4,5,1)==1);
    assert(non_input_calls==8); // even frames skip the entire animation step
    assert(update_calls==9);    // 8 odd-frame updates plus final repaint
    int map_restores=0;
    for (unsigned i=0;i<copies.size();++i)
        if (copies[i].destination->map_window_buffer) ++map_restores;
    assert(map_restores==8);

    reset_case();
    assert(boom(4,5,0x20)==0);
    assert(flic_init_calls==1 && sprite_draws.empty());
    int initial_captures=0;
    for (unsigned i=0;i<copies.size();++i)
        if (copies[i].source->map_window_buffer) ++initial_captures;
    assert(initial_captures==1); // 0x20 prepares only the first snapshot

    reset_case();
    assert(boom(4,5,0x40)==0);
    int window_sources=0, window_destinations=0;
    for (unsigned i=0;i<copies.size();++i) {
        if (copies[i].source->map_window_buffer) ++window_sources;
        if (copies[i].destination->map_window_buffer) ++window_destinations;
    }
    assert(window_sources==1 && window_destinations==1); // reuses first snapshot
}
'''


def test_live_boom_geometry_sounds_snapshots_and_timing(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source = tmp_path / "boom.cpp"
    executable = tmp_path / "boom"
    source.write_text(BOOM_HARNESS.replace("BODY", live_boom_body()))
    compiled = subprocess.run([compiler, "-std=c++17", "-O1", "-fsanitize=undefined",
                               "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
                              text=True, capture_output=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after", [
    ("((flags & 0x100) != 0 ? 6 : 0)", "((flags & 0x100) != 0 ? 0 : 6)"),
    ("pixel_x + full_scaled_width", "pixel_x + scaled_width"),
    ("((GamePreferences & 8u) != 0)", "((GamePreferences & 8u) == 0)"),
    ("if (!skip_multiplayer_frame)", "if (!skip_multiplayer_frame || IsMultiplayerNet)"),
])
def test_boom_damage_controls_are_detected(tmp_path, before, after):
    candidate = live_boom_body()
    assert before in candidate
    candidate = candidate.replace(before, after)
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    source = tmp_path / "boom-damage.cpp"
    executable = tmp_path / "boom-damage"
    source.write_text(BOOM_HARNESS.replace("BODY", candidate))
    compiled = subprocess.run([compiler, "-std=c++17", "-O1", "-fsanitize=undefined",
                               "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
                              text=True, capture_output=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert result.returncode != 0
