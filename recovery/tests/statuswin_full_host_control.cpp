#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string.h>

typedef char *LPSTR;
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[51];
static int TextColourTable[195];
static char StringTemp[1032];
static const char BufferJoinSpace[] = " ";
static int rockiness_label_calls;
static const char *label_get(int id) {
    if (id >= 89 && id <= 92) ++rockiness_label_calls;
    return "label";
}
struct Strings { char *get(int) { return const_cast<char *>("name"); } };
static Strings StringTable;

struct Font {
    int unk_1_, line_height_value_, height_;
    int width(char *s) { assert(std::strcmp(s, "#") == 0); return 5; }
    int pixel_height() const { return height_; }
    int line_height() const { return line_height_value_; }
    int descent() const { return 3; }
};
static int HostRenderEvent;
static void copy_text(char *target, const char *source, int count) {
    int length = count < 255 ? count : 255;
    std::memcpy(target, source, length);
    target[length] = '\0';
}
struct Spot {
    int kills[16], adds;
    int records[128][6];
    int events[128];
    void kill_type(int type) { ++kills[type]; }
    void add(int id, int type, int x, int y, int w, int h) {
        assert(adds < 128);
        records[adds][0]=id; records[adds][1]=type; records[adds][2]=x;
        records[adds][3]=y; records[adds][4]=w; records[adds][5]=h;
        events[adds]=HostRenderEvent++;
        ++adds;
    }
};
struct Buffer {
    RECT current_clip, full_clip, wrap_rect;
    int box_calls, outline_calls, clip_calls, color_calls, writes, wraps;
    RECT clips[128];
    int clip_events[128];
    RECT boxes[64];
    int box_events[64];
    int write_x[1024], write_y[1024];
    char write_text[1024][256];
    int update_events;
    void record_write(char *s, int x, int y, int length) {
        assert(writes < 1024);
        write_x[writes]=x; write_y[writes]=y;
        copy_text(write_text[writes], s, length);
        ++writes;
    }
    void set_font(Font *, Font *, Font *, int) {}
    int box_sprite(RECT *, BoxSpriteParams *) { ++box_calls; return 1; }
    int box(RECT *r, int, int) {
        assert(outline_calls < 64);
        boxes[outline_calls]=*r; box_events[outline_calls]=HostRenderEvent++;
        ++outline_calls; return 1;
    }
    int set_clip(RECT *r) {
        assert(clip_calls < 128);
        clips[clip_calls] = *r; clip_events[clip_calls] = HostRenderEvent++;
        ++clip_calls; current_clip = *r; return 1;
    }
    const RECT &clip_rect() const { return full_clip; }
    void restore_clip() { set_clip(&full_clip); }
    void set_text_color(int, int, int, int) { ++color_calls; }
    int write_l(char *s, int x, int y, int length) {
        record_write(s,x,y,length); return 8;
    }
    int write_cent_l(char *s, RECT *r, int length) {
        record_write(s,r->left,r->top,length); return 8;
    }
    int write_cent_l(char *s, int x, int y, int, int length) {
        record_write(s,x,y,length); return 8;
    }
    int write_right_l(char *s, int x, int y, int length) {
        record_write(s,x,y,length); return 8;
    }
    int wrap(char *, RECT *r) { ++wraps; wrap_rect=*r; return 40; }
};
struct FlatButton { int hides, shows; void hide() { ++hides; } void show(int) { ++shows; } };
struct MainInterface : Buffer {
    RECT status_right, status_left, hit, stack, adjacent;
    FlatButton buttons[11];
    int updates;
    RECT update_rects[32];
    const RECT &status_right_rect() const { return status_right; }
    const RECT &status_left_rect() const { return status_left; }
    const RECT &status_hit_rect() const { return hit; }
    const RECT &stack_button_rect() const { return stack; }
    const RECT &adjacent_panel_right_rect() const { return adjacent; }
    FlatButton &flat_button(int i) { return buttons[i]; }
    void soft_update(RECT *r) {
        assert(updates < 32); update_rects[updates]=*r;
        ++updates; ++HostRenderEvent;
    }
};
static MainInterface MainInterfaceGlobal;
static int IfaceIdToSlotStorage[32];
static int ActiveIfaceIdStorage;
struct BattleWin {
    int calls, args[5];
    void battle_report(int a, int b, int c, int d, int e) {
        ++calls; args[0]=a; args[1]=b; args[2]=c; args[3]=d; args[4]=e;
    }
};
static BattleWin g_BattleWin;

static int MainInterfaceStatusRedrawFlag;
struct GameDataStateFields {
    uint32_t game_state_;
    int turn_current_num_;
    int climate_sea_level_delta_;
    uint32_t game_more_preferences_;
};
struct GameDataStateWindow { GameDataStateFields fields_; };
static GameDataStateWindow GameDataStateStorage;
static int MapWinCaptionScale = -1, MapWinCaptionDivisor = -1;
static int ExpansionEnabled, ClimateSeaLevelDelta, MapSeaLevel;
static int MapIsFlat, MapLongitudeBounds = 16, MapLatitudeBounds = 16;
static uint32_t MapLongitude = 8;
static int VehCurrentCount, BaseCurrentCount;
static int VehMoveDeltaX[9], VehMoveDeltaY[9];
enum { STATE_SCENARIO_EDITOR=0x40, STATE_OMNISCIENT_VIEW=0x80,
       STATE_DEBUG_MODE=0x1000 };
enum { BIT_VEH_IN_TILE = 0x2, BIT_FUNGUS = 0x20, BIT_RIVER = 0x80,
       BIT_FOREST = 0x200000, BIT_CONDENSER = 0x400000,
       BIT_SOIL_ENRICHER = 0x80000, BIT_MAGTUBE = 0x8,
       BIT_MONOLITH = 0x2000, BIT_FARM = 0x8000, BIT_MINE = 0x20000,
       BIT_SOLAR_TIDAL = 0x10000 };
enum { BIT2_CRATER=1, BIT2_VOLCANO=2, BIT2_JUNGLE=4, BIT2_URANIUM=8,
       BIT2_DUNES=0x40, BIT2_FRESH=0x80, BIT2_MESA=0x100,
       BIT2_CANYON=0x200, BIT2_GEOTHERMAL=0x400, BIT2_RIDGE=0x800,
       BIT2_NEXUS=0x2000, BIT2_UNITY=0x4000, BIT2_FOSSIL=0x8000 };
enum { TERRA_FARM=0, TERRA_MINE=2, TERRA_FOREST=4, TERRA_ROAD=5,
       TERRA_REMOVE_FUNGUS=10, TERRA_PLANT_FUNGUS=11 };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1; int8_t territory;
    uint32_t bit, bit2, bit_visible[7]; };
static Map MapTiles[256];
struct Veh { int16_t x,y; uint8_t faction_id; uint8_t pad; uint32_t flags;
    uint8_t visibility; int8_t order; int16_t proto_id; int16_t prev_veh_id_stack,next_veh_id_stack; };
static Veh Vehs[64];
struct Base { int16_t x,y; uint8_t faction_id_current, visibility; char name_string[32]; };
static Base Bases[64];
struct PlayerData { uint32_t flags; uint8_t diplo_treaties[32]; };
static PlayerData PlayersData[8];
struct ConsoleState { int selected_vehicle_,cursor_active_,faction_id_; };
static ConsoleState ConsoleGlobal;
struct RulesData { int tech_three_nutrients_sqr; };
static RulesData Rules;
struct ResourceData { char *name_plural; };
static ResourceData Resource[4];
struct NaturalData { char *name, *name_short; };
static NaturalData Natural[16];
struct TerraformData { uint32_t bit; char *name, *name_sea; };
static TerraformData Terraforming[20];
static uint32_t AltNatural[11];
static int base_draw_calls, base_draw_args[8][7], veh_draw_calls;
static int veh_draw_args[16][7], stack_sort_calls;

static int find_write(const Buffer &buffer, const char *text) {
    for (int i=0;i<buffer.writes;++i)
        if (std::strstr(buffer.write_text[i],text)) return i;
    return -1;
}

static int crop_yield(int,int,int,int,int) { return 2; }
static int mine_yield(int,int,int,int,int bonus) { return bonus ? 3 : 2; }
static int energy_yield(int,int,int,int,int bonus) { return bonus ? 2 : 1; }
static int bonus_at(int,int,int) { return 0; }
static int has_tech(int,int) { return 0; }
static int elev_at(int,int) { return 4; }
static int base_at(int x,int y) {
    for (int i=0;i<BaseCurrentCount;++i) if (Bases[i].x==x && Bases[i].y==y) return i;
    return -1;
}
static int veh_at(int x,int y) {
    for (int i=0;i<VehCurrentCount;++i) if (Vehs[i].x==x && Vehs[i].y==y) return i;
    return -1;
}
static int spying(int) { return 0; }
static int action_terraform(int,int,int) { return 1; }
static int GenTerrainPolyCalls;
static int gen_terrain_poly(Buffer *,int,int,int,int,int,int,int,int,int) {
    ++GenTerrainPolyCalls; return 0;
}
static void base_draw(Buffer *,int id,int x,int y,int dx,int flags) {
    assert(base_draw_calls < 8);
    base_draw_args[base_draw_calls][0]=id; base_draw_args[base_draw_calls][1]=x;
    base_draw_args[base_draw_calls][2]=y; base_draw_args[base_draw_calls][3]=dx;
    base_draw_args[base_draw_calls][4]=flags;
    base_draw_args[base_draw_calls][5]=HostRenderEvent++;
    ++base_draw_calls;
}
static int veh_draw(Buffer *,int id,int x,int y,int dx,int flags,int mode) {
    assert(veh_draw_calls < 16);
    veh_draw_args[veh_draw_calls][0]=id; veh_draw_args[veh_draw_calls][1]=x;
    veh_draw_args[veh_draw_calls][2]=y; veh_draw_args[veh_draw_calls][3]=dx;
    veh_draw_args[veh_draw_calls][4]=flags; veh_draw_args[veh_draw_calls][5]=mode;
    ++veh_draw_calls; return 0;
}
static void say_stats_3(char *out,int) { std::strcpy(out,"stats"); }
static void stack_sort_2(int) { ++stack_sort_calls; }
static void itoa(int value,char *out,int) { std::sprintf(out,"%d",value); }
static int find_font(int,int) { return 0; }
struct RulesResource { };

struct StatusWin {
    Font font1_,font2_,font3_; int field_15A8_,field_15C8_,field_15C4_,field_15CC_,field_15D0_,field_15DC_;
    int iTileX_,iTileY_,iUnitIndex_,iCityIndex_;
    Spot spot_;
    void draw_status(int,int,int,int,int);
    void draw_active(int v,int b,int mode) { last_vehicle=v; last_base=b; last_mode=mode; ++active_calls; }
    int active_calls,last_vehicle,last_base,last_mode;
};

/*__STATUSWIN_DRAW_STATUS_BODY__*/

int main() {
    std::memset(&MainInterfaceGlobal, 0, sizeof(MainInterfaceGlobal));
    MainInterfaceGlobal.status_right.left=0; MainInterfaceGlobal.status_right.top=0;
    MainInterfaceGlobal.status_right.right=160; MainInterfaceGlobal.status_right.bottom=100;
    MainInterfaceGlobal.status_left.left=0; MainInterfaceGlobal.status_left.top=0;
    MainInterfaceGlobal.status_left.right=160; MainInterfaceGlobal.status_left.bottom=30;
    MainInterfaceGlobal.hit.left=0; MainInterfaceGlobal.hit.top=100;
    MainInterfaceGlobal.hit.right=160; MainInterfaceGlobal.hit.bottom=220;
    MainInterfaceGlobal.stack.left=0; MainInterfaceGlobal.stack.top=0;
    MainInterfaceGlobal.stack.right=40; MainInterfaceGlobal.stack.bottom=40;
    MainInterfaceGlobal.adjacent.left=0; MainInterfaceGlobal.adjacent.top=0;
    MainInterfaceGlobal.adjacent.right=160; MainInterfaceGlobal.adjacent.bottom=40;
    MainInterfaceGlobal.full_clip.left=0; MainInterfaceGlobal.full_clip.top=0;
    MainInterfaceGlobal.full_clip.right=640; MainInterfaceGlobal.full_clip.bottom=480;
    TextColourTable[18]=12; TextColourTable[19]=13;
    TextColourTable[20]=14; TextColourTable[21]=15;
    StatusWin win = {};
    win.field_15DC_=77;
    win.draw_status(1,2,3,4,5);
    assert(win.field_15DC_==0 && g_BattleWin.calls==1);
    assert(g_BattleWin.args[0]==1 && g_BattleWin.args[4]==5);
    IfaceIdToSlotStorage[ActiveIfaceIdStorage]=1;
    win.spot_.kills[0]=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.spot_.kills[0]==0);
    IfaceIdToSlotStorage[ActiveIfaceIdStorage]=0;
    win.font1_.unk_1_=4; win.font1_.height_=12; win.font1_.line_height_value_=13;
    win.iTileX_=4; win.iTileY_=4; win.iUnitIndex_=-1; win.iCityIndex_=-1;
    ConsoleGlobal.faction_id_=0; ConsoleGlobal.selected_vehicle_=-1;
    const int selected_tile = 4*MapLongitude+2;
    GameDataStateStorage.fields_.turn_current_num_=0; MapTiles[selected_tile].visibility=1;
    MapTiles[4*MapLongitudeBounds+2].visibility=0;
    MapTiles[selected_tile].climate=0x60;
    GenTerrainPolyCalls=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.active_calls==1 && win.last_vehicle==-1 && win.last_base==-1);
    assert(win.spot_.kills[0]==1 && win.spot_.kills[11]==1);
    assert(MainInterfaceGlobal.writes>0 && MainInterfaceGlobal.updates==1);
    assert(GenTerrainPolyCalls==1); // half-width MapLongitude, not full bounds.
    assert(MainInterfaceGlobal.update_rects[0].bottom==100);
    assert(win.spot_.records[0][2]==0 && win.spot_.records[0][3]==0 &&
           win.spot_.records[0][4]==160 && win.spot_.records[0][5]==100);
    assert(MainInterfaceGlobal.current_clip.left==MainInterfaceGlobal.full_clip.left &&
           MainInterfaceGlobal.current_clip.right==MainInterfaceGlobal.full_clip.right);

    // Base visibility uses the base owner's treaty row, not that owner's flags.
    Bases[0].x=4; Bases[0].y=4; Bases[0].faction_id_current=1;
    Bases[0].visibility=1; BaseCurrentCount=1;
    Vehs[0].x=4; Vehs[0].y=4; Vehs[0].faction_id=1;
    Vehs[0].flags=0; Vehs[0].visibility=0; Vehs[0].prev_veh_id_stack=-1;
    Vehs[0].next_veh_id_stack=-1; VehCurrentCount=1;
    PlayersData[1].flags=0; PlayersData[1].diplo_treaties[0]=1;
    ConsoleGlobal.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=0;
    GameDataStateStorage.fields_.turn_current_num_=1; win.iUnitIndex_=-1; win.iCityIndex_=-1;
    MapTiles[selected_tile].visibility=1;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.last_vehicle==0);
    PlayersData[1].flags=1; PlayersData[1].diplo_treaties[0]=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.last_vehicle==-1);

    // Land rockiness and rainfall share one output row; water omits that row.
    win.field_15A8_=0; win.iUnitIndex_=-1; win.iCityIndex_=-1;
    ConsoleGlobal.selected_vehicle_=-1; GameDataStateStorage.fields_.turn_current_num_=0;
    MapTiles[selected_tile].visibility=1; MapTiles[selected_tile].bit=0;
    MapTiles[selected_tile].bit2=0; MapTiles[selected_tile].val3=0x40;
    MapTiles[selected_tile].climate=0x60; MainInterfaceGlobal.writes=0;
    rockiness_label_calls=0;
    win.draw_status(-1,-1,-1,-1,0);
    int rocky_rain_row=find_write(MainInterfaceGlobal,"label & label");
    assert(rocky_rain_row>=0);
    int rocky_rows=0;
    for (int i=0;i<MainInterfaceGlobal.writes;++i)
        if (std::strcmp(MainInterfaceGlobal.write_text[i],"label & label")==0)
            ++rocky_rows;
    assert(rocky_rows==1);
    assert(rockiness_label_calls==1);
    MapTiles[selected_tile].climate=0x40; MainInterfaceGlobal.writes=0;
    rockiness_label_calls=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(find_write(MainInterfaceGlobal,"label & label")==-1);
    assert(rockiness_label_calls==0);

    // Native landmark masks require their high guard bit, and names concatenate
    // without an inserted comma.
    MapTiles[selected_tile].climate=0x60;
    MapTiles[selected_tile].bit2=BIT2_CRATER;
    MainInterfaceGlobal.writes=0; win.draw_status(-1,-1,-1,-1,0);
    assert(find_write(MainInterfaceGlobal,"name")>=0);
    MapTiles[selected_tile].bit2=BIT2_CRATER|BIT2_VOLCANO;
    MainInterfaceGlobal.writes=0; win.draw_status(-1,-1,-1,-1,0);
    assert(find_write(MainInterfaceGlobal,"namename")>=0);
    assert(find_write(MainInterfaceGlobal,"name, name")==-1);
    MapTiles[selected_tile].bit2=0x80000001U|0x80000002U;
    MainInterfaceGlobal.writes=0; win.draw_status(-1,-1,-1,-1,0);
    assert(find_write(MainInterfaceGlobal,"name")==-1);

    // The wrap rectangle starts at the current text cursor, not panel top.
    MapTiles[selected_tile].bit2=0; MapTiles[selected_tile].bit=1;
    Terraforming[1].bit=1; MainInterfaceGlobal.wraps=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(MainInterfaceGlobal.wraps==1);
    assert(MainInterfaceGlobal.wrap_rect.top>6);

    // Debug output uses the signed region byte and original "x , y" punctuation.
    GameDataStateStorage.fields_.game_state_=STATE_SCENARIO_EDITOR|STATE_DEBUG_MODE;
    MapTiles[selected_tile].bit=0; MapTiles[selected_tile].region=0xfe;
    MainInterfaceGlobal.writes=0; win.draw_status(-1,-1,-1,-1,0);
    assert(find_write(MainInterfaceGlobal,"(-2)")>=0);
    assert(find_write(MainInterfaceGlobal,"(4 , 4)")>=0);
    GameDataStateStorage.fields_.game_state_=0;

    Bases[0].x=4; Bases[0].y=4; Bases[0].faction_id_current=0;
    Bases[0].visibility=1; BaseCurrentCount=1; VehCurrentCount=0;
    ConsoleGlobal.selected_vehicle_=-1; ConsoleGlobal.cursor_active_=0;
    win.iCityIndex_=0;
    GameDataStateStorage.fields_.turn_current_num_=1;
    win.iCityIndex_=0; win.iUnitIndex_=-1;
    base_draw_calls=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.last_base==0 && win.last_vehicle==-1 && base_draw_calls==2);

    Bases[0].x=10; Bases[0].y=10; win.iCityIndex_=-1;
    Vehs[0].x=4; Vehs[0].y=4; Vehs[0].faction_id=0; Vehs[0].flags=0;
    Vehs[0].visibility=1; Vehs[0].order=0; Vehs[0].proto_id=0;
    Vehs[0].prev_veh_id_stack=-1; Vehs[0].next_veh_id_stack=-1;
    VehCurrentCount=1; ConsoleGlobal.selected_vehicle_=0; ConsoleGlobal.cursor_active_=1;
    win.iUnitIndex_=0; MainInterfaceStatusRedrawFlag=0;
    veh_draw_calls=0; stack_sort_calls=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(win.last_vehicle==0 && win.last_base==-1);
    assert(veh_draw_calls==2 && stack_sort_calls==1 && win.spot_.adds>=2);

    // The inner panel's horizontal width determines capacity; a base consumes
    // one slot. The fit path starts at the inset panel edge, not stack buttons.
    Vehs[0].next_veh_id_stack=-1; VehCurrentCount=1;
    Bases[0].x=4; Bases[0].y=4; Bases[0].faction_id_current=0;
    Bases[0].visibility=1; BaseCurrentCount=1;
    VehCurrentCount=0; ConsoleGlobal.selected_vehicle_=-1;
    ConsoleGlobal.cursor_active_=0; win.iUnitIndex_=-1; win.iCityIndex_=0;
    MainInterfaceGlobal.adjacent.left=0;
    base_draw_calls=veh_draw_calls=0; win.spot_.adds=0;
    for (int i=0;i<128;++i) MainInterfaceGlobal.clips[i]=RECT();
    MainInterfaceGlobal.clip_calls=0; MainInterfaceGlobal.box_calls=0;
    MainInterfaceGlobal.outline_calls=0;
    HostRenderEvent=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(MainInterfaceGlobal.buttons[8].hides>0);
    assert(base_draw_calls==2 && veh_draw_calls==0);
    assert(win.spot_.records[1][1]==11 && win.spot_.records[1][2]==6);
    assert(win.spot_.records[1][3]==106 && win.spot_.records[1][4]==66);
    int base_clip_index=-1;
    for (int i=0;i<MainInterfaceGlobal.clip_calls;++i)
        if (MainInterfaceGlobal.clips[i].left==4 &&
            MainInterfaceGlobal.clips[i].top==104 &&
            MainInterfaceGlobal.clips[i].right==69 &&
            MainInterfaceGlobal.clips[i].bottom==216) base_clip_index=i;
    assert(base_clip_index>=0);
    assert(base_draw_args[0][5] >
           MainInterfaceGlobal.clip_events[base_clip_index]);
    assert(win.spot_.events[1] < MainInterfaceGlobal.box_events[0]);
    assert(MainInterfaceGlobal.boxes[0].right==69);

    // Overflow reserves one of two horizontal cells for the base and moves
    // the list beside the stack controls with the narrower native item width.
    Vehs[0].next_veh_id_stack=1; Vehs[1]=Vehs[0]; Vehs[1].prev_veh_id_stack=0;
    Vehs[1].next_veh_id_stack=2; Vehs[2]=Vehs[0];
    Vehs[2].prev_veh_id_stack=1; Vehs[2].next_veh_id_stack=-1;
    VehCurrentCount=3; ConsoleGlobal.selected_vehicle_=0;
    ConsoleGlobal.cursor_active_=1; win.iUnitIndex_=0; MainInterfaceStatusRedrawFlag=1;
    MainInterfaceGlobal.adjacent.left=150;
    base_draw_calls=veh_draw_calls=0; win.spot_.adds=0;
    win.draw_status(-1,-1,-1,-1,0);
    assert(MainInterfaceGlobal.buttons[8].shows>0 &&
           MainInterfaceGlobal.buttons[10].shows>0);
    assert(base_draw_calls==2 && veh_draw_calls==2);
    assert(win.spot_.records[1][2]==41 && win.spot_.records[1][4]==54);
    assert(win.spot_.records[2][2]==95 && win.spot_.records[2][4]==54);
    int unit_clip_index=-1;
    for (int i=0;i<MainInterfaceGlobal.clip_calls;++i)
        if (MainInterfaceGlobal.clips[i].left==93 &&
            MainInterfaceGlobal.clips[i].top==104 &&
            MainInterfaceGlobal.clips[i].right==146 &&
            MainInterfaceGlobal.clips[i].bottom==216) unit_clip_index=i;
    assert(unit_clip_index>=0);
}
