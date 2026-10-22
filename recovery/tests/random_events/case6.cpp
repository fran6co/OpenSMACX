#include <assert.h>
#include <stdint.h>
#include <string.h>

struct TestBase {
    int x, y, faction_id_current, population_size, visibility;
    unsigned char facilities_built[8];
};
struct TestPlayer { int ranking; };
struct TestConsole {
    int faction_id_, focuses, focus_x, focus_y;
    int focus(int x, int y, int faction) {
        ++focuses; focus_x=x; focus_y=y; assert(faction==faction_id_); return 0;
    }
};
struct TestFacility { const char *name; };
struct TestGameData { struct Fields { int turn_current_num_, base_current_count_; } fields_; };

static TestBase Bases[8];
static TestPlayer PlayersData[8];
static TestConsole ConsoleGlobal;
static TestFacility Facility[128];
static TestGameData GameDataStateStorage;
static int MapLongitudeBounds, MapLongitude, MapIsFlat, IsMultiplayerPBEM, IsMultiplayerNet;
static int ProjectOwners[32];
static int parse_calls, parsed_facility, parsed_num_low, parsed_num_high;
static int draw_calls, drawn_x[8], drawn_y[8], popup_calls, popup_argument;
static char popup_caption[32], popup_image[32];

static bool has_fac_built(int facility, int base) {
    assert(facility >= 0 && facility < 64 && base >= 0 && base < 8);
    return (Bases[base].facilities_built[facility >> 3] & (1u << (facility & 7))) != 0;
}
static int base_project(int slot) { return ProjectOwners[slot]; }
static void parse_say(int, const char *name, int, int) {
    ++parse_calls; parsed_facility = -1;
    for (int i=0; i<128; ++i) if (Facility[i].name == name) { parsed_facility=i; break; }
    assert(parsed_facility >= 0);
}
static int parse_num(int low, int high) {
    ++parse_calls; parsed_num_low=low; parsed_num_high=high; return low;
}
static void draw_tile(int x, int y, int mode) {
    assert(mode==2 && draw_calls<8); drawn_x[draw_calls]=x; drawn_y[draw_calls]=y; ++draw_calls;
}
static int POP2(const char *caption, const char *image, int argument) {
    strcpy(popup_caption,caption); strcpy(popup_image,image); popup_argument=argument; ++popup_calls; return 0;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include "actual_case6.hpp"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

static void reset() {
    memset(Bases,0,sizeof(Bases)); memset(PlayersData,0,sizeof(PlayersData));
    memset(ProjectOwners,0xFF,sizeof(ProjectOwners));
    for (int i=0; i<128; ++i) Facility[i].name = "facility";
    Facility[27].name="facility-27"; Facility[28].name="facility-28";
    Facility[70].name="facility-70"; Facility[84].name="facility-84"; Facility[96].name="facility-96";
    GameDataStateStorage.fields_.turn_current_num_=100;
    GameDataStateStorage.fields_.base_current_count_=3;
    MapLongitudeBounds=100; MapLongitude=50; MapIsFlat=1;
    IsMultiplayerPBEM=0; IsMultiplayerNet=0; ConsoleGlobal.faction_id_=1; ConsoleGlobal.focuses=0;
    parse_calls=draw_calls=popup_calls=0; parsed_facility=-1; parsed_num_low=parsed_num_high=0;
    popup_argument=0; popup_caption[0]=popup_image[0]='\0';
    Bases[0].x=10; Bases[0].y=10; Bases[0].faction_id_current=2; Bases[0].population_size=6;
    Bases[0].visibility=1u<<1;
    Bases[1].x=12; Bases[1].y=10; Bases[1].faction_id_current=1; Bases[1].population_size=7;
    Bases[2].x=18; Bases[2].y=10; Bases[2].faction_id_current=3; Bases[2].population_size=5;
}

static void run_case6(int faction, int base, int roll) {
    random_events_case6_from_mapped_body(6,faction,base,roll,0,0);
}

static void test_population_radius_and_local_focus() {
    reset(); PlayersData[2].ranking=4;
    run_case6(2,0,9);
    assert(parse_calls==1 && parsed_num_low==1 && parsed_num_high==3);
    assert(Bases[0].population_size==3 && Bases[1].population_size==4 && Bases[2].population_size==5);
    assert(draw_calls==2 && drawn_x[0]==10 && drawn_y[0]==10 && drawn_x[1]==12 && drawn_y[1]==10);
    assert(ConsoleGlobal.focuses==2 && ConsoleGlobal.focus_x==12 && ConsoleGlobal.focus_y==10);
    assert(popup_calls==1 && strcmp(popup_caption,"PROMETHEUS0")==0 && popup_argument==0);
}

static void test_rank_threshold_and_round_wrap() {
    reset(); PlayersData[2].ranking=4; GameDataStateStorage.fields_.turn_current_num_=150;
    run_case6(2,0,9);
    assert(parse_calls==0 && draw_calls==0 && Bases[0].population_size==6);

    reset(); PlayersData[2].ranking=4; MapIsFlat=0; MapLongitudeBounds=80; MapLongitude=40;
    Bases[1].x=98;
    run_case6(2,0,9);
    assert(Bases[0].population_size==3 && Bases[1].population_size==4 && draw_calls==2);
}

static void test_facility_and_project_immunity() {
    reset(); IsMultiplayerPBEM=1; Bases[0].facilities_built[18>>3] |= 1u<<(18&7);
    run_case6(2,0,9);
    assert(parse_calls==1 && parsed_facility==28 && draw_calls==0);
    assert(strcmp(popup_caption,"PROMETHEUS1")==0 && popup_argument==9);

    reset(); PlayersData[2].ranking=4; Bases[1].facilities_built[17>>3] |= 1u<<(17&7);
    run_case6(2,0,9);
    assert(Bases[1].population_size==7 && draw_calls==1);

    reset(); Bases[0].facilities_built[18>>3] |= 1u<<(18&7); IsMultiplayerPBEM=1;
    run_case6(2,0,9);
    assert(parse_calls==1 && parsed_facility==28 && popup_calls==1);
    assert(strcmp(popup_caption,"PROMETHEUS1")==0 && popup_argument==9);
    assert(ConsoleGlobal.focuses==0);

    reset(); Bases[0].faction_id_current=1;
    Bases[0].facilities_built[18>>3] |= 1u<<(18&7);
    run_case6(1,0,9);
    assert(parse_calls==1 && parsed_facility==28);
    assert(ConsoleGlobal.focuses==1 && ConsoleGlobal.focus_x==10 && ConsoleGlobal.focus_y==10);
    assert(popup_calls==1 && strcmp(popup_caption,"PROMETHEUS1")==0);
    assert(draw_calls==0 && parsed_num_low==0);

    reset(); PlayersData[2].ranking=4; ProjectOwners[0]=1;
    run_case6(2,0,9);
    assert(Bases[0].population_size==3 && Bases[1].population_size==7);
    assert(draw_calls==1 && popup_calls==1 && strcmp(popup_caption,"PROMETHEUS0")==0);
}

static void test_project_precedence_and_pbem_notification() {
    reset(); IsMultiplayerPBEM=1; ProjectOwners[14]=0; Bases[0].faction_id_current=2;
    run_case6(2,0,9);
    assert(parse_calls==1 && parsed_facility==84 && popup_calls==1);
    assert(strcmp(popup_caption,"PROMETHEUS2")==0 && popup_argument==9);
    assert(draw_calls==0 && parsed_num_low==0); // project branch bypasses radius parse
}

int main() {
    test_population_radius_and_local_focus();
    test_rank_threshold_and_round_wrap();
    test_facility_and_project_immunity();
    test_project_precedence_and_pbem_notification();
}
