// Private proposal template. Each section is a self-contained C++98 control.
// Permanent pytest reads ONLY this file and current src/newgame.cpp.

// @@CASE_map_menu@@
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t DWORD;
typedef char *LPSTR;
typedef const char *LPCSTR;
#define __cdecl
struct RECT { int left,top,right,bottom; };

static char events[4096];
static int event_len;
static int size_results[8], size_status[8], size_count, size_at;
static int menu_results[8], menu_count, menu_at;
static int custom_results[8], custom_count, custom_at;
static int daemon_results[8], daemon_count, daemon_at, file_result, config_result;
static int random_results[8], random_count, random_at;
static int climate_calls, daemon_calls, file_calls, config_calls, popup_starts;
static int popup_value;
static char first_daemon_path[64];
static void record(const char *s) {
    if (event_len) events[event_len++]='|';
    while (*s && event_len < (int)sizeof(events)-2) events[event_len++]=*s++;
    events[event_len]=0;
}
// @@HELPERS_map_menu@@

struct DefaultPref { uint32_t difficulty,faction_id,pad,map_type,top_menu; } DefaultPrefs;
int MapSetupStatus;
int IsMultiplayerNet;
uint32_t MapOceanCoverage, MapLandCoverage, MapErosiveForces;
uint32_t MapPlanetaryOrbit, MapCloudCover, MapNativeLifeForms;
char PopupStartCaption[256];

class Popup {
public:
    uint32_t field_3100_;
    Popup() : field_3100_(0) {}
    void start(char *, const char *title, int a, char *b, int c, class GraphicWin *owner) {
        if (strcmp(title,"MAPMENU") || a!=-1 || b || c || owner) abort();
        ++popup_starts; record("popup.start");
    }
    void set_value(int value) { popup_value=value; record("popup.set_value"); }
    int input_poll_state() const { return static_cast<int>(field_3100_); }
};
class SetupWin {
public:
    int do_menu(Popup *, int kind, int flags) {
        if (kind!=1) abort();
        (void)flags;
        record("menu");
        return menu_at<menu_count ? menu_results[menu_at++] : -1;
    }
};

static int prefs_save(int) { record("prefs.save"); return 1; }
static int size_of_planet(int) {
    const int i=size_at++;
    MapSetupStatus=size_status[i]; record("size"); return size_results[i];
}
static int custom_planet(int, int) {
    record("custom"); return custom_at<custom_count ? custom_results[custom_at++] : 0;
}
static int load_map_daemon(char *path) {
    if (strcmp(path,"maps\\xplanet.mp") && strcmp(path,"maps\\xplanetx.mp")) abort();
    if (daemon_calls == 0) strcpy(first_daemon_path, path);
    ++daemon_calls; record("daemon");
    return daemon_at<daemon_count ? daemon_results[daemon_at++] : 0;
}
static int load_map() { ++file_calls; record("file"); return file_result; }
static void world_climate() { ++climate_calls; record("climate"); }
static int config_game(int) { ++config_calls; record("config"); return config_result; }
static int host_rand() { return random_at<random_count ? random_results[random_at++] : 0; }

#define rand host_rand
// @@BODY_map_menu@@
#undef rand

static void reset() {
    memset(events,0,sizeof(events)); event_len=0;
    size_count=size_at=menu_count=menu_at=custom_count=custom_at=0;
    random_count=random_at=0; daemon_count=daemon_at=0; file_result=config_result=0;
    climate_calls=daemon_calls=file_calls=config_calls=popup_starts=0;
    first_daemon_path[0]=0;
    MapSetupStatus=0; IsMultiplayerNet=0;
    MapOceanCoverage=MapLandCoverage=MapErosiveForces=0;
    MapPlanetaryOrbit=MapCloudCover=MapNativeLifeForms=0;
    DefaultPrefs.map_type=0;
}
static int contains(const char *text,const char *part) { return strstr(text,part)!=0; }

int main() {
    reset();
    DefaultPrefs.map_type=2; daemon_results[0]=0; daemon_count=1; config_result=0;
    if (map_menu(1)!=0 || daemon_calls!=1 || climate_calls!=1 ||
        strcmp(first_daemon_path,"maps\\xplanet.mp") ||
        !contains(events,"popup.start|popup.set_value|prefs.save|daemon|climate|config")) return 1;

    reset();
    size_count=1; size_results[0]=0; size_status[0]=1;
    random_count=2; random_results[0]=6; random_results[1]=0;
    if (map_menu(1)!=0 || MapOceanCoverage!=0 || MapLandCoverage!=2 ||
        MapErosiveForces!=1 || MapPlanetaryOrbit!=1 || MapCloudCover!=1 ||
        MapNativeLifeForms!=1 || climate_calls!=0) return 2;

    reset();
    DefaultPrefs.map_type=1;
    size_count=2; size_results[0]=size_results[1]=0;
    size_status[0]=size_status[1]=1;
    custom_count=2; custom_results[0]=1; custom_results[1]=0;
    if (map_menu(1)!=0 || popup_starts!=2 || custom_at!=2 || config_calls!=1) return 3;

    reset();
    DefaultPrefs.map_type=4; file_result=1; menu_results[0]=-1; menu_count=1;
    if (map_menu(1)!=1 || file_calls!=1 || popup_starts!=2 || menu_at!=1) return 4;

    reset();
    DefaultPrefs.map_type=3; daemon_results[0]=1; daemon_results[1]=0; daemon_count=2;
    if (map_menu(1)!=0 || daemon_calls!=2 || popup_starts!=2 || climate_calls!=1 ||
        strcmp(first_daemon_path,"maps\\xplanetx.mp")) return 5;

    reset();
    size_count=1; size_results[0]=0; size_status[0]=1;
    IsMultiplayerNet=1; random_count=1; random_results[0]=5;
    if (map_menu(1)!=0 || MapOceanCoverage!=0 || MapLandCoverage!=2 ||
        random_at!=1 || MapNativeLifeForms!=2) return 6;

    puts("map_menu body host control: PASS");
    return 0;
}

// @@ENDCASE_map_menu@@

// @@CASE_custom_planet@@
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
static void fail(int line) { fprintf(stderr, "custom_planet fixture line %d\n", line); exit(91); }
#define CHECK(x) do { if (!(x)) fail(__LINE__); } while (0)
typedef char *LPSTR;
typedef const char *LPCSTR;

static char events[4096];
static int event_size;
static void event(const char *text) {
    if (event_size) events[event_size++]='|';
    while (*text && event_size < (int)sizeof(events)-2) events[event_size++]=*text++;
    events[event_size]=0;
}
static int count_text(const char *text, const char *needle) {
    int count=0;
    const size_t n=strlen(needle);
    for (const char *p=text; (p=strstr(p,needle))!=0; p+=n) ++count;
    return count;
}

struct AlphaIniPrefMock { uint32_t custom_world[7]; } AlphaIniPrefs;
uint32_t MapSizePlanet, MapOceanCoverage, MapLandCoverage, MapErosiveForces;
uint32_t MapPlanetaryOrbit, MapCloudCover, MapNativeLifeForms;
char StringTemp[1032];
char PopupStartCaption[256];
int WinScreenWidth=800;
static int prefs_save_calls, prefs_save_value;
static int popup_starts, popup_exec_calls, setup_menu_calls, setup_closes;
static int popup_value, popup_start_mutation_slot, popup_start_mutation_value;
static int popup_returns[8], popup_return_count, popup_return_at;
static int menu_returns[8], menu_return_count, menu_return_at;
static int field_actions[8], field_action_count, field_action_at;
static char popup_titles[8][128];
static int popup_values[8];
static char loaded[32][48];
static int loaded_count;
static int loaded_colours[32], loaded_ranges[32];
static int mutate_load_index=-1, mutate_screen_width, mutate_life, mutate_clouds;
static int last_overlay_x[3], last_overlay_y[3], overlay_present[3];
static int menu_overlay_x[3], menu_overlay_y[3], menu_overlay_present[3];

static void prefs_save(int value) { ++prefs_save_calls; prefs_save_value=value; event("prefs.save"); }

class Buffer {
public:
    int width_;
    Buffer() : width_(968) { event("buffer.ctor"); }
    ~Buffer() { close(); event("buffer.dtor"); }
    void close() { event("buffer.close"); }
    int load_pcx(const char *name, void *, int colours, int range) {
        CHECK(loaded_count < 32);
        strcpy(loaded[loaded_count], name);
        loaded_colours[loaded_count]=colours;
        loaded_ranges[loaded_count]=range;
        ++loaded_count;
        if (loaded_count-1==mutate_load_index) {
            WinScreenWidth=mutate_screen_width;
            MapNativeLifeForms=static_cast<uint32_t>(mutate_life);
            MapCloudCover=static_cast<uint32_t>(mutate_clouds);
        }
        event("buffer.load");
        return 0;
    }
    int pixel_width() const { return width_; }
};
class Popup {
public:
    uint32_t field_3100_;
    Popup() : field_3100_(0) { event("popup.ctor"); }
    ~Popup() { close(); event("popup.dtor"); }
    void start(char *, const char *title, int selected, char *override_text,
               int flags, class GraphicWin *owner) {
        CHECK(title && selected==-1 && !override_text && flags==0x40 && !owner);
        CHECK(popup_starts < 8);
        strcpy(popup_titles[popup_starts], title);
        if (popup_start_mutation_slot >= 0)
            AlphaIniPrefs.custom_world[popup_start_mutation_slot] =
                static_cast<uint32_t>(popup_start_mutation_value);
        ++popup_starts; event("popup.start");
    }
    void set_value(int value) { popup_value=value; popup_values[popup_starts-1]=value; event("popup.set_value"); }
    int exec(int flags, int (*callback)()) {
        CHECK(flags==0 && callback==0);
        ++popup_exec_calls; event("popup.exec");
        if (field_action_at < field_action_count)
            field_3100_=static_cast<uint32_t>(field_actions[field_action_at++]);
        return popup_return_at < popup_return_count ? popup_returns[popup_return_at++] : -1;
    }
    int input_poll_state() const { return static_cast<int>(field_3100_); }
    void close() { event("popup.close"); }
};
class SetupWin {
    Buffer buffer_;
public:
    Buffer *overlay_buffers_[3];
    int overlay_x_[3], overlay_y_[3];
    SetupWin() {
        for (int i=0;i<3;++i) overlay_buffers_[i]=0, overlay_x_[i]=overlay_y_[i]=0;
        event("setup.ctor");
    }
    ~SetupWin() { close(); event("setup.dtor"); }
    int do_menu(Popup *popup, int kind, int flags) {
        CHECK(kind==1 && flags==0);
        for (int i=0;i<3;++i) {
            menu_overlay_present[i]=overlay_buffers_[i]!=0;
            menu_overlay_x[i]=overlay_x_[i]; menu_overlay_y[i]=overlay_y_[i];
        }
        if (field_action_at < field_action_count)
            popup->field_3100_=static_cast<uint32_t>(field_actions[field_action_at++]);
        ++setup_menu_calls; event("setup.menu");
        return menu_return_at < menu_return_count ? menu_returns[menu_return_at++] : -1;
    }
    void close() {
        ++setup_closes;
        for (int i=0;i<3;++i) {
            overlay_present[i]=overlay_buffers_[i]!=0;
            last_overlay_x[i]=overlay_x_[i]; last_overlay_y[i]=overlay_y_[i];
        }
        event("setup.close");
        buffer_.close();
    }
};
class GraphicWin {};

// @@HELPERS_custom_planet@@
// @@BODY_custom_planet@@

static void reset() {
    events[0]=0; event_size=0;
    memset(&AlphaIniPrefs,0,sizeof(AlphaIniPrefs));
    memset(&MapSizePlanet,0,sizeof(MapSizePlanet));
    MapOceanCoverage=MapLandCoverage=MapErosiveForces=0;
    MapPlanetaryOrbit=MapCloudCover=MapNativeLifeForms=0;
    memset(StringTemp,0,sizeof(StringTemp)); memset(PopupStartCaption,0,sizeof(PopupStartCaption));
    prefs_save_calls=prefs_save_value=popup_starts=popup_exec_calls=setup_menu_calls=setup_closes=0;
    popup_value=-1; popup_start_mutation_slot=-1; popup_start_mutation_value=0;
    popup_return_count=popup_return_at=menu_return_count=menu_return_at=0;
    field_action_count=field_action_at=0;
    memset(popup_titles,0,sizeof(popup_titles));
    memset(popup_values,0,sizeof(popup_values));
    memset(popup_returns,0,sizeof(popup_returns)); memset(menu_returns,0,sizeof(menu_returns));
    memset(field_actions,0,sizeof(field_actions));
    loaded_count=0; WinScreenWidth=800;
    mutate_load_index=-1; mutate_screen_width=mutate_life=mutate_clouds=0;
    memset(loaded,0,sizeof(loaded)); memset(loaded_colours,0,sizeof(loaded_colours));
    memset(loaded_ranges,0,sizeof(loaded_ranges));
    memset(last_overlay_x,0,sizeof(last_overlay_x)); memset(last_overlay_y,0,sizeof(last_overlay_y));
    memset(overlay_present,0,sizeof(overlay_present));
    memset(menu_overlay_x,0,sizeof(menu_overlay_x)); memset(menu_overlay_y,0,sizeof(menu_overlay_y));
    memset(menu_overlay_present,0,sizeof(menu_overlay_present));
}

int main() {
    CHECK(custom_planet_clamp_level(0)==1);
    CHECK(custom_planet_clamp_level(1)==2);
    CHECK(custom_planet_clamp_level(2)==3);
    CHECK(custom_planet_clamp_level(3)==3);
    CHECK(custom_planet_clamp_level(0x7FFFFFFFu)==1);
    CHECK(custom_planet_clamp_level(0xFFFFFFFFu)==1);

    reset();
    AlphaIniPrefs.custom_world[1]=2;
    AlphaIniPrefs.custom_world[3]=4;
    AlphaIniPrefs.custom_world[5]=5;
    AlphaIniPrefs.custom_world[6]=3;
    CHECK(custom_planet(1,1)==0);
    CHECK(popup_starts==0 && setup_menu_calls==0 && popup_exec_calls==0);
    CHECK(MapOceanCoverage==2 && MapErosiveForces==4 && MapNativeLifeForms==3 &&
          MapCloudCover==5 && MapSizePlanet==0 && MapLandCoverage==0);
    CHECK(prefs_save_calls==1 && prefs_save_value==0);

    reset();
    AlphaIniPrefs.custom_world[1]=1;
    AlphaIniPrefs.custom_world[1]=2;
    AlphaIniPrefs.custom_world[6]=3;
    AlphaIniPrefs.custom_world[5]=4;
    AlphaIniPrefs.custom_world[0]=5;
    menu_returns[0]=0; menu_returns[1]=1; menu_returns[2]=2; menu_returns[3]=3;
    menu_return_count=4;
    CHECK(custom_planet(1,0)==0);
    CHECK(setup_menu_calls==4 && popup_starts==4 && popup_exec_calls==0);
    CHECK(AlphaIniPrefs.custom_world[1]==0 && AlphaIniPrefs.custom_world[3]==1 &&
          AlphaIniPrefs.custom_world[6]==2 && AlphaIniPrefs.custom_world[5]==3 &&
          AlphaIniPrefs.custom_world[0]==5);
    CHECK(MapSizePlanet==0 && MapLandCoverage==2);
    CHECK(loaded_count==12);
    CHECK(strcmp(loaded[0],"S1L1C1.PCX")==0 && loaded_colours[0]==10 && loaded_ranges[0]==0xEC);
    CHECK(strcmp(loaded[3],"moon1_800.pcx")==0 && strcmp(loaded[4],"moon2_800.pcx")==0 &&
          strcmp(loaded[5],"moon3_800.pcx")==0);
    CHECK(strcmp(loaded[6],"S1L1C1.PCX")==0 && loaded_colours[6]==10);
    CHECK(menu_overlay_present[0] && menu_overlay_x[0]==449 && menu_overlay_y[0]==-168);
    CHECK(setup_closes==1);
    CHECK(strstr(events,"setup.close|buffer.close|setup.dtor")!=0);
    CHECK(strstr(events,"setup.dtor|buffer.close|buffer.dtor")!=0);
    CHECK(strstr(events,"popup.close|popup.dtor")!=0 && count_text(events,"buffer.dtor")==4);

    reset();
    AlphaIniPrefs.custom_world[1]=2;
    popup_return_count=4;
    for (int i=0;i<4;++i) popup_returns[i]=0;
    CHECK(custom_planet(0,0)==0);
    CHECK(popup_exec_calls==4 && setup_menu_calls==0 && loaded_count==0);
    CHECK(strcmp(popup_titles[0],"WORLDLAND")==0);

    reset();
    AlphaIniPrefs.custom_world[1]=1;
    popup_return_count=4;
    for (int i=0;i<4;++i) popup_returns[i]=0;
    field_actions[0]=1; field_action_count=1;
    CHECK(custom_planet(0,0)==0);
    CHECK(MapOceanCoverage==0xFFFFFFFFu && AlphaIniPrefs.custom_world[1]==1);
    CHECK(prefs_save_calls==1 && prefs_save_value==0);

    reset();
    WinScreenWidth=1024;
    AlphaIniPrefs.custom_world[3]=2;
    menu_returns[0]=0; menu_return_count=4;
    CHECK(custom_planet(1,0)==0);
    CHECK(strcmp(loaded[3],"MOON1.PCX")==0 && strcmp(loaded[4],"MOON2.PCX")==0 &&
          strcmp(loaded[5],"MOON3.PCX")==0);
    CHECK(menu_overlay_x[0]==56 && menu_overlay_y[0]==0);

    reset();
    AlphaIniPrefs.custom_world[6]=6;
    AlphaIniPrefs.custom_world[5]=5;
    menu_return_count=4;
    CHECK(custom_planet(1,0)==0);
    CHECK(strcmp(popup_titles[2],"WORLDLIFE")==0);
    CHECK(strcmp(popup_titles[3],"WORLDCLOUDS")==0);
    CHECK(loaded_count==12 && strcmp(loaded[0],"S1L1C1.PCX")==0 &&
          loaded_colours[0]==10 && loaded_ranges[0]==0xEC);

    // Dispatch follows descriptor slot, never the saved preference value.
    reset();
    AlphaIniPrefs.custom_world[1]=5;
    AlphaIniPrefs.custom_world[3]=1;
    AlphaIniPrefs.custom_world[6]=2;
    AlphaIniPrefs.custom_world[5]=3;
    menu_return_count=4;
    CHECK(custom_planet(1,0)==0);
    CHECK(strcmp(popup_titles[0],"WORLDLAND")==0);
    CHECK(strcmp(loaded[0],"S1L1C1.PCX")==0);
    CHECK(strcmp(popup_titles[1],"WORLDTIDES")==0);
    CHECK(strcmp(loaded[3],"moon1_800.pcx")==0);
    CHECK(strcmp(popup_titles[2],"WORLDLIFE")==0);
    CHECK(strcmp(popup_titles[3],"WORLDCLOUDS")==0);

    // Popup.start may mutate the preference; native reloads it after callback
    // and seeds the popup from that current value before do_menu.
    reset();
    AlphaIniPrefs.custom_world[1]=1;
    popup_returns[0]=0; popup_returns[1]=0; popup_returns[2]=0; popup_returns[3]=0;
    popup_return_count=4;
    popup_start_mutation_slot=1; popup_start_mutation_value=4;
    CHECK(custom_planet(0,0)==0);
    CHECK(popup_values[0]==4);
    CHECK(strstr(events,"popup.start|popup.set_value|popup.exec")!=0);

    reset();
    AlphaIniPrefs.custom_world[1]=1;
    menu_return_count=4;
    mutate_load_index=0; mutate_screen_width=1024; mutate_life=2; mutate_clouds=2;
    CHECK(custom_planet(1,0)==0);
    CHECK(strcmp(loaded[0],"S1L1C1.PCX")==0 &&
          strcmp(loaded[1],"S2L3C3.PCX")==0 &&
          strcmp(loaded[2],"S3L3C3.PCX")==0);
    CHECK(menu_overlay_x[0]==56 && menu_overlay_y[0]==0);

    reset();
    AlphaIniPrefs.custom_world[3]=2;
    menu_return_count=6; WinScreenWidth=800;
    mutate_load_index=0; mutate_screen_width=1024;
    CHECK(custom_planet(1,0)==0);
    CHECK(strcmp(loaded[3],"MOON1.PCX")==0 &&
          strcmp(loaded[4],"MOON2.PCX")==0 &&
          strcmp(loaded[5],"MOON3.PCX")==0);

    reset();
    popup_returns[0]=0; popup_returns[1]=1; popup_returns[2]=-1; popup_return_count=3;
    CHECK(custom_planet(0,0)==1);
    CHECK(popup_exec_calls==3 && setup_menu_calls==0 && prefs_save_calls==1 && prefs_save_value==0);
    CHECK(popup_starts==3 && count_text(events,"buffer.dtor")==4 &&
          strstr(events,"popup.close|popup.dtor")!=0);

    puts("custom_planet exact-body host control: PASS");
    return 0;
}

// @@ENDCASE_custom_planet@@

// @@CASE_size_of_planet@@
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
static void host_fail(int line) { fprintf(stderr,"fixture abort at line %d\n",line); exit(90); }
#undef abort
#define abort() host_fail(__LINE__)
typedef char *LPSTR;
typedef const char *LPCSTR;
static char event_log[4096];
static int event_n;
static void record(const char *s) {
    if (event_n) event_log[event_n++]='|';
    while (*s && event_n<(int)sizeof(event_log)-2) event_log[event_n++]=*s++;
    event_log[event_n]=0;
}
static int text_open_result;
static int text_numbers[64], text_number_count, text_number_at;
static int text_item_at;
static const char *text_items[8];
static int popup_results[8], popup_result_count, popup_result_at;
static int menu_result;
static int custom_width_text, custom_height_text;
static int edit_count, edit_labels[2];
static int popup_start_count, popup_close_count, setup_close_count;
static int expected_start_flags;
static int prefs_save_count, prefs_put_count, map_init_count, paths_init_count;
static int xpop_result, xpop_calls, random_values[8], random_count, random_at;
static int prefs_x=40, prefs_y=80;
static int last_put_x, last_put_y;
// @@HELPERS_size_of_planet@@

struct DefaultPrefsMock { uint32_t custom_world[7]; } AlphaIniPrefs;
uint32_t MapSizePlanet, MapOceanCoverage, MapLandCoverage, MapErosiveForces;
uint32_t MapPlanetaryOrbit, MapCloudCover, MapNativeLifeForms;
int MapLongitudeBounds, MapLatitudeBounds;
int MapSetupStatus;
uint32_t UnkBitfield1;
static const char AlphaxFileID[]="ALPHAX";
char StringTemp[1032];
struct ParseStringMock { char str[256]; } ParseStrBuffer[10];
char PopupStartCaption[256];
struct PathMock { void init() { ++paths_init_count; record("paths.init"); } } Paths;

static char *label_get(int index) {
    return const_cast<char *>(index==0x1108/4 ? "World X" : "World Y");
}
static int text_open(LPCSTR file, LPCSTR section) {
    if (strcmp(file,"ALPHAX") || strcmp(section,"WORLDSIZE") || MapSetupStatus!=1) abort();
    record("text.open"); return text_open_result;
}
static char *text_get() { record("text.get"); return (char *)"item"; }
static char *text_item() {
    const char *s=text_items[text_item_at++]; return const_cast<char *>(s);
}
static int text_item_number() {
    return text_number_at<text_number_count ? text_numbers[text_number_at++] : 0;
}
static int prefs_get(LPCSTR key, int def, int fallback) {
    if (fallback==0x28 && strcmp(key,"Custom Map X")==0) return prefs_x;
    if (fallback==0x50 && strcmp(key,"Custom Map Y")==0) return prefs_y;
    abort(); return def;
}
static void prefs_put(LPCSTR key, int value, int toggle) {
    if (toggle!=0) abort();
    if (strcmp(key,"Custom Map X")==0) last_put_x=value;
    else if (strcmp(key,"Custom Map Y")==0) last_put_y=value;
    else abort();
    ++prefs_put_count; record("prefs.put");
}
static void prefs_save(int) { ++prefs_save_count; record("prefs.save"); }
static int map_init() { ++map_init_count; record("map.init"); return 1; }
static int X_pop(const char *label, int callback) {
    if (strcmp(label,"VERYLARGEMAP") || callback) abort();
    ++xpop_calls; record("xpop"); return xpop_result;
}
static char *host_itoa(int value, char *out, int radix) {
    if (radix!=10) abort();
    sprintf(out,"%d",value);
    return out;
}
static int host_rand() {
    return random_at<random_count ? random_values[random_at++] : 0;
}

class EditGroup {
public:
    int edit(char *label, char *, int limit) {
        if (limit!=8 || edit_count>=2) abort();
        edit_labels[edit_count]=strcmp(label,"World X")==0 ? 0 : 1;
        sprintf(ParseStrBuffer[edit_count].str, "%d",
                edit_count==0 ? custom_width_text : custom_height_text);
        ++edit_count; record("edit"); return 1;
    }
};
class DialogsMock : public EditGroup {
public:
    int item(char *, int id) { (void)id; record("item"); return 1; }
};
class Popup {
    DialogsMock dialogs_;
public:
    ~Popup() { close(); record("popup.dtor"); }
    void start(char *, const char *label, int value, char *override_text,
               int flags, class GraphicWin *owner) {
        if (value!=-1 || override_text || owner) abort();
        if (popup_start_count==0 &&
            (strcmp(label,"WORLDSIZE") || flags!=expected_start_flags)) abort();
        if (popup_start_count==1 && (strcmp(label,"CUSTOMMAP") || flags!=0x44)) abort();
        ++popup_start_count; record("popup.start");
    }
    DialogsMock &dialogs() { return dialogs_; }
    int exec(int flags, int (*callback)()) {
        if (flags || callback) abort();
        record("popup.exec");
        return popup_result_at<popup_result_count ? popup_results[popup_result_at++] : -1;
    }
    void set_value(int) {}
    void close() { ++popup_close_count; record("popup.close"); }
};
class SetupWin {
public:
    ~SetupWin() { close(); record("setup.dtor"); }
    int do_menu(Popup *, int kind, int flags) {
        if (kind!=1 || flags) abort();
        record("menu");
        return menu_result;
    }
    void close() { ++setup_close_count; record("setup.close"); }
};

#define rand host_rand
#define _itoa host_itoa
// @@BODY_size_of_planet@@
#undef _itoa
#undef rand

static void reset() {
    event_n=0; event_log[0]=0;
    text_open_result=0; text_number_count=text_number_at=text_item_at=0;
    popup_result_count=popup_result_at=0; menu_result=0;
    custom_width_text=40; custom_height_text=80; edit_count=0;
    popup_start_count=popup_close_count=setup_close_count=0;
    expected_start_flags=0;
    prefs_save_count=prefs_put_count=map_init_count=paths_init_count=0;
    xpop_result=xpop_calls=random_count=random_at=0;
    prefs_x=40; prefs_y=80; last_put_x=last_put_y=0;
    memset(AlphaIniPrefs.custom_world,0,sizeof(AlphaIniPrefs.custom_world));
    MapSizePlanet=MapLongitudeBounds=MapLatitudeBounds=MapSetupStatus=0;
    UnkBitfield1=0;
    memset(StringTemp,0,sizeof(StringTemp)); memset(ParseStrBuffer,0,sizeof(ParseStrBuffer));
    text_items[0]="small"; text_items[1]="medium";
    text_items[2]="large"; text_items[3]="xlarge";
    text_items[4]="huge"; text_items[5]="unused";
}

int main() {
    reset();
    text_numbers[0]=1; text_numbers[1]=40; text_numbers[2]=80;
    text_number_count=3; popup_results[0]=0; popup_result_count=1;
    if (size_of_planet(2)!=0 || MapSizePlanet!=0 || MapLatitudeBounds!=80 ||
        MapLongitudeBounds!=80 || MapSetupStatus!=1 || prefs_save_count!=1 ||
        map_init_count || paths_init_count || !strstr(event_log,"popup.start|text.open")) return 1;

    reset();
    text_numbers[0]=1; text_numbers[1]=32; text_numbers[2]=48;
    text_number_count=3; popup_results[0]=99; popup_results[1]=0; popup_result_count=2;
    custom_width_text=300; custom_height_text=0; xpop_result=0;
    if (size_of_planet(2)!=0 || MapSizePlanet!=99 || MapLongitudeBounds!=512 ||
        MapLatitudeBounds!=16 || edit_count!=2 || edit_labels[0]!=0 || edit_labels[1]!=1 ||
        last_put_x!=256 || last_put_y!=16 || xpop_calls!=1 || prefs_put_count!=2) return 2;

    reset();
    text_open_result=1;
    if (size_of_planet(2)!=0 || MapSizePlanet!=2 || MapLongitudeBounds!=80 ||
        MapLatitudeBounds!=80 || MapSetupStatus!=1) return 3;

    reset();
    text_numbers[0]=1; text_numbers[1]=40; text_numbers[2]=80;
    text_number_count=3; popup_results[0]=-1; popup_result_count=1;
    if (size_of_planet(2)!=1 || setup_close_count!=1 || popup_close_count!=1) return 4;

    reset();
    text_numbers[0]=1; text_numbers[1]=32; text_numbers[2]=48;
    text_number_count=3; popup_results[0]=99; popup_results[1]=0; popup_result_count=2;
    custom_width_text=160; custom_height_text=80; xpop_result=1;
    if (size_of_planet(2)!=1 || xpop_calls!=1 || setup_close_count!=1 || popup_close_count!=1) return 5;

    reset();
    AlphaIniPrefs.custom_world[0]=3;
    text_numbers[0]=5;
    text_numbers[1]=10; text_numbers[2]=20;
    text_numbers[3]=20; text_numbers[4]=30;
    text_numbers[5]=32; text_numbers[6]=48;
    text_numbers[7]=40; text_numbers[8]=48;
    text_numbers[9]=50; text_numbers[10]=60;
    text_number_count=11; random_values[0]=2; random_count=1;
    popup_results[0]=0; popup_result_count=1;
    if (size_of_planet(2)!=0 || MapSizePlanet!=2 || MapLatitudeBounds!=48 ||
        MapLongitudeBounds!=64 || AlphaIniPrefs.custom_world[0]!=3 || random_at!=1) return 6;

    reset();
    text_numbers[0]=1; text_numbers[1]=40; text_numbers[2]=80;
    text_number_count=3; AlphaIniPrefs.custom_world[0]=0;
    expected_start_flags=0x40;
    if (size_of_planet(1)!=0 || popup_result_at!=0 || MapSizePlanet!=0 ||
        MapLongitudeBounds!=80 || MapLatitudeBounds!=80) return 7;

    puts("size_of_planet body host control: PASS");
    return 0;
}

// @@ENDCASE_size_of_planet@@

// @@CASE_top_menu@@
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#ifndef __cdecl
#define __cdecl
#endif

struct Win { virtual ~Win() {} };
struct GraphicWin : Win {
    int id;
    int init_result;
    int fills;
    int updates;
    GraphicWin() : id(++next_id), init_result(0), fills(0), updates(0) {}
    static int next_id;
    int init(int, int, int, int, char *, int, Win *, void *, void *) {
        return init_result;
    }
    void fill(int) { ++fills; }
    void soft_update() { ++updates; }
};
int GraphicWin::next_id = 0;
static GraphicWin *OpeningWindow = 0;
static GraphicWin *&opening_window() { return OpeningWindow; }
static Win *BasePopDefaultField322C = 0;
static int WinScreenWidth = 1024;
static int WinScreenHeight = 768;
static int ExpansionEnabled = 0;
static int DialogDefaultStyle = 3;
static int TurnUpkeepPhase = 0;
static int GameDataLoadRequest = 0;
static int GameSessionActive = 0;
static int WorldClimateSkipTerrainClear = 1;
struct GameDataStateMockFields {
    int game_preferences_, game_more_preferences_, game_state_, game_rules_;
    int vehicle_current_count_, base_current_count_, turn_current_num_;
    int net_turn_flags_, net_turn_faction_, diff_level_current_;
    int time_controls_selection_;
    struct { int enable_mask_; } lock_masks_;
};
struct GameDataStateMock { GameDataStateMockFields fields_; };
static GameDataStateMock GameDataStateStorage = {};
static int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
static int IsMultiplayerPBEM = 0;
static int IsMultiplayerNet = 0;
static int ExitTurnLoop = 0;
static int ControlTurnPhase = 0;
static int &GamePreferences = GameDataStateStorage.fields_.game_preferences_;
static int &GameState = GameDataStateStorage.fields_.game_state_;
static int &GameRules = GameDataStateStorage.fields_.game_rules_;
static unsigned int MapOceanCoverage = 0;
static unsigned int MapLandCoverage = 0;
static unsigned int MapErosiveForces = 0;
static unsigned int MapPlanetaryOrbit = 0;
static unsigned int MapCloudCover = 0;
static unsigned int MapNativeLifeForms = 0;
struct ScenarioSaveMock { int objective_req_victory; unsigned int objectives_sudden_death_victory; unsigned int ending_mission_year; };
static ScenarioSaveMock ScenarioSave = {};
static int GenderDefault = 0;
static int PluralityDefault = 0;
static int parsed_ids[16];
static int parsed_gender[16];
static int parsed_plurality[16];
static int parsed_count;
static int mutate_narration;
static int objective_faction, objective_mask;
static char PopupStartCaption[256];
static char StringTemp[1032];
struct Label { void *strings_ptr; };
static Label Labels;
struct DefaultPref { unsigned int difficulty, faction_id, pad, map_type, top_menu; };
struct AlphaIniPref { unsigned int preferences, more_preferences, announce, rules, semaphore, time_controls, customize, custom_world[7]; };
static DefaultPref DefaultPrefs = {0,0,0,0,2};
static AlphaIniPref AlphaIniPrefs = {};
struct Player { int is_leader_female; char name_leader[24]; char title_leader[24]; char noun_faction[24]; int noun_gender; int is_noun_plural; };
static Player Players[8];
struct Console { int faction_id_; int move_calls; void set_move(int) { ++move_calls; } };
static Console ConsoleGlobal = {0,0};

static int event_count;
static char events[512][40];
static void event(const char *text) {
    if (event_count < 512) {
        std::strncpy(events[event_count], text, sizeof events[event_count] - 1);
        events[event_count][sizeof events[event_count] - 1] = '\0';
        ++event_count;
    }
}
static int menu_values[32];
static int menu_count;
static int menu_at;
static int menu_item_count;
static int mapped_result;
static int loaded_result;
static int map_load_result;
static int multiplayer_result;
static int game_init_result;
static int credits_calls;
static int all_draws;
static int prefs_save_calls;
static int map_wipe_calls;
static int setup_game_calls;
static int close_calls;
static int popup_close_calls;
static int setup_close_calls;
static int startup_wave_loads;
static int wave_playing;
static int wave_fades;
static int wave_unloads;
static int opening_change_calls;
static int replace_opening_at;
static GraphicWin replacement_window;

struct Popup {
    int value_;
    int starts;
    Popup() : value_(0), starts(0) {}
    ~Popup() { close(); event("popup.destroy"); }
    void start(char *, const char *, int, char *, int, GraphicWin *) {
        ++starts; event("popup.start");
    }
    void set_value(int value) { value_ = value; }
    void close() { ++popup_close_calls; event("popup.close"); }
};
struct SetupWin {
    ~SetupWin() { close(); event("setup.destroy"); }
    int do_menu(Popup *, int, int) {
        event("menu.popup");
        if (menu_at >= menu_count) return -1;
        return menu_values[menu_at++];
    }
    int do_menu(char *, int, int) {
        event("menu.scenario");
        if (menu_at >= menu_count) return -1;
        return menu_values[menu_at++];
    }
    void close() { ++setup_close_calls; event("setup.close"); }
};
struct Palette { int set() { event("palette.set"); return 0; } };
static Palette g_PALETTE1;
struct Wave {
    void load(char *, int) { ++startup_wave_loads; wave_playing = 1; event("wave.load"); }
    void play() { event("wave.play"); }
    int is_playing() { return wave_playing; }
    void fade(int) { ++wave_fades; event("wave.fade"); }
    void unload() { ++wave_unloads; wave_playing = 0; event("wave.unload"); }
} ;
static Wave g_TOP_MENU_WAVE;
struct Strings {
    char *get(int) { static char label[] = "Commander"; return label; }
    void shutdown() { event("strings.shutdown"); }
};
static Strings StringTable;
static char *label_get(int) {
    return StringTable.get(7);
}

static int __cdecl change_opening(char *filename);
static void close_opening();
static void do_all_draws();
static void prefs_load(int);
static void prefs_use();
static void prefs_save(int);
static void clear_all_player_messages();
static void labels_shutdown();
static void stop_timers();
static void show_credits();
static int map_menu(int);
static int size_of_planet(int);
static int load_map();
static int load_game(int, int);
static int multiplayer_init(int);
static void map_wipe();
static int config_game(int);
static void setup_game(int);
static int game_init(int, int);
static void read_rules(int);
static void parse_says(int, const char *, int, int);
static int parse_num(int, int);
static int num_objectives(int, int);
static void say_year(char *);
static int X_pop(char *, const char *, int, char *, int, int (__cdecl *)());

// @@BODY_top_menu@@

static int all_map[4];
static int all_map_count;
static void reset_fixture() {
    event_count = menu_count = menu_at = menu_item_count = 0;
    mapped_result = loaded_result = map_load_result = multiplayer_result = 0;
    game_init_result = credits_calls = all_draws = prefs_save_calls = 0;
    map_wipe_calls = setup_game_calls = close_calls = popup_close_calls = 0;
    setup_close_calls = startup_wave_loads = wave_fades = wave_unloads = 0;
    opening_change_calls = replace_opening_at = 0;
    wave_playing = 0;
    replacement_window.updates = 0;
    parsed_count = 0;
    mutate_narration = 0;
    objective_faction = objective_mask = -1;
    OpeningWindow = 0;
    BasePopDefaultField322C = 0;
    ExpansionEnabled = DialogDefaultStyle = TurnUpkeepPhase = 0;
    GameDataLoadRequest = GameSessionActive = 0;
    WorldClimateSkipTerrainClear = 1; VehCurrentCount = 1;
    IsMultiplayerPBEM = IsMultiplayerNet = ExitTurnLoop = 0;
    ControlTurnPhase = GamePreferences = 0;
    GenderDefault = PluralityDefault = 0;
    GameState = GameRules = 0;
    DefaultPrefs.top_menu = 2;
    AlphaIniPrefs.rules = 0;
    all_map_count = 0;
    static int label_id = 7;
    Labels.strings_ptr = &label_id;
}
static void check(bool condition, const char *message) {
    if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
static int event_index(const char *needle) {
    for (int i = 0; i < event_count; ++i)
        if (std::strcmp(events[i], needle) == 0) return i;
    return -1;
}

int __cdecl change_opening(char *filename) {
    char text[40] = "opening:";
    std::strncat(text, filename, sizeof text - std::strlen(text) - 1);
    event(text);
    ++opening_change_calls;
    if (replace_opening_at == opening_change_calls)
        OpeningWindow = &replacement_window;
    return 0;
}
static void close_opening() { ++close_calls; OpeningWindow = 0; }
static void do_all_draws() { ++all_draws; event("all.draws"); }
static void prefs_load(int) { event("prefs.load"); }
static void prefs_use() { event("prefs.use"); }
static void prefs_save(int) { ++prefs_save_calls; event("prefs.save"); }
static void clear_all_player_messages() { event("messages.clear"); }
static void labels_shutdown() { event("labels.shutdown"); }
static void stop_timers() { event("timers.stop"); }
static void show_credits() { ++credits_calls; event("credits"); }
static int map_menu(int quick) { all_map[all_map_count++] = quick; return mapped_result; }
static int size_of_planet(int) { event("size.of.planet"); return 0; }
static int load_map() { event("map.load"); return map_load_result; }
static int load_game(int, int continue_mode) { event(continue_mode ? "game.load.continue" : "game.load"); return loaded_result; }
static int multiplayer_init(int) { event("multiplayer.init"); return multiplayer_result; }
static void map_wipe() { ++map_wipe_calls; event("map.wipe"); }
static int config_game(int) { event("config.game"); return 0; }
static void setup_game(int) { ++setup_game_calls; event("setup.game"); }
static int game_init(int, int) { event("game.init"); return game_init_result; }
static void read_rules(int) { event("rules.read"); }
static void parse_says(int id, const char *, int, int) {
    if (parsed_count < 16) {
        parsed_ids[parsed_count] = id;
        parsed_gender[parsed_count] = GenderDefault;
        parsed_plurality[parsed_count] = PluralityDefault;
        ++parsed_count;
    }
    event("parse.says");
    if (mutate_narration && id == 1) ConsoleGlobal.faction_id_ = 1;
    if (mutate_narration && id == 2) ConsoleGlobal.faction_id_ = 2;
}
static int parse_num(int id, int) {
    event("parse.num");
    if (mutate_narration && id == 1) ConsoleGlobal.faction_id_ = 3;
    return 0;
}
static int num_objectives(int faction, int mask) {
    objective_faction = faction;
    objective_mask = mask;
    return 3;
}
static void say_year(char *) { event("say.year"); }
static int X_pop(char *, const char *, int, char *, int, int (__cdecl *)()) { event("x.pop"); return 0; }

int main() {
    reset_fixture();
    menu_values[0] = -1; menu_count = 1;
    check(top_menu(0) == 1, "menu cancel result");
    check(opening_change_calls == 2 && all_draws == 2,
          "native initial double opening presentation");
    check(popup_close_calls == 1 && setup_close_calls == 1,
          "automatic destructors close both locals exactly once");
    check(event_index("messages.clear") < event_index("prefs.load"),
          "mode-zero message clear precedes preference load");
    check(DefaultPrefs.top_menu == 2 && prefs_save_calls == 0,
          "cancel returns before saving a selected preference");
    check(event_index("popup.close") < event_index("setup.close") &&
          event_index("popup.destroy") < event_index("setup.destroy"),
          "reverse declaration-order destruction");

    reset_fixture();
    GamePreferences = 0x800;
    wave_playing = 1;
    AlphaIniPrefs.rules = 0x20000000u;
    menu_values[0] = 0; menu_count = 1; mapped_result = 0;
    check(top_menu(1) == 0, "map success result");
    check(startup_wave_loads == 0 && wave_fades == 1 && wave_unloads == 1,
          "mode-one audio cleanup uses live playing state");
    check((DialogDefaultStyle & 4) != 0, "rules flag sets dialog style bit");
    check(all_map_count == 1 && all_map[0] == 0,
          "choice zero maps to ordinary map_menu mode");

    reset_fixture();
    replace_opening_at = 1;
    menu_values[0] = -1; menu_count = 1;
    check(top_menu(1) == 1, "opening replacement callback fixture exits");
    check(replacement_window.updates == 2 &&
          BasePopDefaultField322C == &replacement_window,
          "soft update and published owner reload the live opening slot");

    reset_fixture();
    menu_values[0] = 1; menu_values[1] = -1; menu_count = 2;
    mapped_result = 1;
    check(top_menu(0) == 1, "retry then cancel result");
    check(all_map_count == 1 && all_map[0] == 1,
          "choice one maps to quick-start mode");
    check(opening_change_calls == 3,
          "failed action retries with one opening presentation");

    reset_fixture();
    menu_values[0] = 2; menu_values[1] = 2; menu_count = 2;
    map_load_result = 0;
    check(top_menu(0) == 0, "map-file new game completes");
    check(event_index("map.load") < event_index("config.game") &&
          event_index("config.game") < event_index("setup.game"),
          "map-file branch ordering");
    check(MapOceanCoverage == 0 && MapNativeLifeForms == 0 &&
          (GameState & 0x080000E0u) == 0x080000E0u && VehCurrentCount == 0,
          "map-file branch preserves map settings and sets game-state bits");
    check(ControlTurnPhase == ConsoleGlobal.faction_id_,
          "local faction published to turn phase");

    reset_fixture();
    menu_values[0] = 2; menu_values[1] = 1; menu_count = 2;
    check(top_menu(0) == 1, "new-game route returns nested selection");
    check(MapOceanCoverage == 1 && MapLandCoverage == 1 &&
          MapErosiveForces == 1 && MapPlanetaryOrbit == 1 &&
          MapCloudCover == 1 && MapNativeLifeForms == 1,
          "native new-game path resets all six separate map globals");
    check(map_wipe_calls == 1 && setup_game_calls == 1,
          "new-game path wipes then sets up map once");

    reset_fixture();
    menu_values[0] = 3; menu_count = 1;
    loaded_result = 0;
    GameSessionActive = 1;
    check(top_menu(0) == 0, "load menu action completes");
    check(event_index("game.load") < event_index("timers.stop") &&
          event_index("timers.stop") < event_index("strings.shutdown"),
          "successful load lifecycle ordering");
    check(GameDataLoadRequest == 0 && WorldClimateSkipTerrainClear == 0,
          "load selector and climate-clear control reset after callbacks");

    reset_fixture();
    Players[0].is_leader_female = 1;
    Players[0].noun_gender = 4;
    Players[0].is_noun_plural = 1;
    menu_values[0] = 1; menu_count = 1;
    check(top_menu(1) == 0, "continue-load setup returns through scenario intro");
    check(parsed_count == 5 && parsed_ids[0] == 1 && parsed_ids[1] == 2 &&
          parsed_ids[2] == 3 && parsed_ids[3] == 4 && parsed_ids[4] == 0,
          "scenario narration parse order");
    check(parsed_gender[0] == 1 && parsed_plurality[0] == 0 &&
          parsed_gender[1] == 1 && parsed_plurality[1] == 0 &&
          parsed_gender[2] == 4 && parsed_plurality[2] == 1 &&
          parsed_gender[3] == 4 && parsed_plurality[3] == 1,
          "leader and faction grammar defaults remain live through year lines");
    check(ConsoleGlobal.move_calls == 1 && event_index("x.pop") > event_index("parse.num"),
          "scenario intro popup follows all numeric substitutions");

    reset_fixture();
    ConsoleGlobal.faction_id_ = 0;
    mutate_narration = 1;
    GameRules = 0x1000;
    Players[0].is_leader_female = 5;
    Players[1].is_leader_female = 6;
    Players[2].noun_gender = 7;
    Players[2].is_noun_plural = 8;
    menu_values[0] = 1; menu_count = 1;
    check(top_menu(1) == 0, "live faction narration succeeds");
    check(parsed_gender[0] == 5 && parsed_gender[1] == 6 &&
          parsed_gender[2] == 7 && parsed_plurality[2] == 8,
          "each narration step reloads callback-changed faction");
    check(objective_faction == 3 && objective_mask == 0x1000,
          "objective call reloads faction and preserves integer rule mask");

    reset_fixture();
    menu_values[0] = 5; menu_values[1] = -1; menu_count = 2;
    check(top_menu(0) == 1 && credits_calls == 1,
          "credits always retry the menu");

    reset_fixture();
    menu_values[0] = 5; menu_count = 1;
    check(top_menu(1) == 6 && credits_calls == 0,
          "nonzero mode increments positive selection before unsigned range exit");

    reset_fixture();
    menu_values[0] = 3; menu_count = 1; multiplayer_result = 0;
    check(top_menu(1) == 0 && event_index("multiplayer.init") >= 0,
          "nonzero menu selection three dispatches to multiplayer init");

    std::puts("top_menu whole-body host control: PASS");
    return 0;
}

// @@ENDCASE_top_menu@@

// @@CASE_multiplayer_init@@
#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <vector>

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef int BOOL;
#define __cdecl

static std::vector<std::string> events;
static std::vector<int> TextNumbers;
static size_t TextNumberIndex;
static int TextOpenResult;
static int ExitTurnLoop;
struct AlphaNetProcessSlot { int8_t identity_; };
struct LockStateEntry {
    uint32_t leading_words_[4];
    union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_;
    uint32_t trailing_words_[10];
};
struct MockLock { void clear() { events.push_back("lock-clear"); } };
struct MockNetDaemon {
    int player_count_;
    AlphaNetProcessSlot process_slots_[9];
    struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; } primary_state_;
    LockStateEntry channels_[2];
    MockLock locks_;
    uint32_t voice_hotkey_48_, voice_hotkey_4c_, voice_flags_D8_;
    int init_result, send_files_result;
    int receive_count;
    int negative_loop_probe;
    void close() { events.push_back("daemon-close"); }
    int init(int) { events.push_back("daemon-init"); return init_result; }
    int pid_2_who(unsigned long) { events.push_back("pid-map"); return 2; }
    int send_files() { events.push_back("send-files"); return send_files_result; }
    int receive() {
        events.push_back("receive");
        ++receive_count;
        if (negative_loop_probe && receive_count == 1)
            primary_state_.trailing_words_[2] = 0xFFFFFFFFu;
        else
            ExitTurnLoop = 1;
        return 0;
    }
    void send_rules() { events.push_back("send-rules"); }
    void send_game() { events.push_back("send-game"); }
};
typedef MockNetDaemon NetDaemon;
struct MockNetWin { int exec_result; void config_players() { events.push_back("config-players"); } int exec() { events.push_back("exec"); return exec_result; } };
struct MockStringTable { void shutdown() { events.push_back("strings-shutdown"); } int get(int) { return 0; } };
struct MockPath { void init() { events.push_back("path-init"); } };
struct MockConsole { int faction_id_; void set_move(int) { events.push_back("set-move"); } };
struct MockWindow { void close() { events.push_back("window-close"); } void hide() { events.push_back("window-hide"); } };
struct MockMainInterface : MockWindow {};
struct MockOptions {
    uint8_t status_, player_count_, difficulty_, time_control_, option_4_, option_5_;
    uint8_t planet_size_, ocean_coverage_, erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
struct MockSession { uint32_t mode_; uint8_t state_[0x164]; };
struct MockLobbyRow { uint8_t bytes_[0x178]; uint32_t owner_; };
struct MockPlayer { int is_leader_female; char title_leader[32], name_leader[32]; int noun_gender, is_noun_plural; char noun_faction[32]; };
struct MockPlayerData { int unk_99; };

static MockNetDaemon NetDaemonNet;
static MockNetWin NetWinDialog;
static MockStringTable StringTable;
static MockPath Paths;
static MockConsole ConsoleGlobal;
static MockWindow TutWinDesWindow, TutWinBaseWindow, ConsoleStatusWin, WorldClimateWorldWin, MultiWindow;
static MockMainInterface MainInterfaceGlobal;
static MockOptions NetWinSessionOptions;
static MockSession NetWinSessionData;
static MockLobbyRow NetWinLobbyRows[8];
static MockPlayer Players[8];
static MockPlayerData PlayersData[8];
struct InitGameDataFields {
    uint32_t game_rules_, game_state_, net_turn_flags_, game_more_preferences_;
    int diff_level_current_, time_controls_selection_, vehicle_current_count_;
    int turn_current_num_, base_current_count_;
    struct { uint32_t enable_mask_; } lock_masks_;
};
struct InitGameData { InitGameDataFields fields_; };
static InitGameData GameDataStateStorage = {};
static uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
static uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
static uint32_t &NetTurnFlags = GameDataStateStorage.fields_.net_turn_flags_;
static uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
static int IsMultiplayerNet, IsMultiplayerPBEM;
static int &DiffLevelCurrent = GameDataStateStorage.fields_.diff_level_current_;
static int GenderDefault, PluralityDefault;
static int &TimeControlsSelection = GameDataStateStorage.fields_.time_controls_selection_;
static int ControlTurnPhase;
static int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
static int MapSizePlanet, MapLongitudeBounds, MapLatitudeBounds;
static uint32_t MapOceanCoverage, MapLandCoverage, MapErosiveForces;
static uint32_t MapPlanetaryOrbit, MapCloudCover, MapNativeLifeForms;
static uint32_t WorldClimateSkipTerrainClear;
struct ScenarioSaveMock { int objective_req_victory; uint32_t objectives_sudden_death_victory; uint32_t ending_mission_year; } ScenarioSave;
static uint32_t UnkBitfield1;
static const char *AlphaxFileID = "ALPHAX";
static uint32_t AltNatural[11], AltContour[11];
static char StringTemp[1032];
static int NetWinPlayerId;
static int MockGameInitResult;
static int multi_parsed_factions[8], multi_parse_count;
static int multi_faction_after[4], multi_faction_mutations;
static int multi_objective_faction;
static uint32_t multi_objective_mask;
static char *label_get(int) { return const_cast<char *>("YEAR"); }

static int text_open(const char *, const char *section) {
    assert(strcmp(section, "WORLDSIZE") == 0);
    events.push_back("text-open");
    return TextOpenResult;
}
static char *text_get() { events.push_back("text-get"); return 0; }
static char *text_item() { events.push_back("text-item"); return 0; }
static int text_item_number() {
    assert(TextNumberIndex < TextNumbers.size());
    events.push_back("text-number");
    return TextNumbers[TextNumberIndex++];
}
static int map_init() { events.push_back("map-init"); return 1; }
static void world_build() { events.push_back("world-build"); }
static void setup_game(int) { events.push_back("setup-game"); }
static void scenario_setup() { events.push_back("scenario-setup"); }
static void log_set_state(int) { events.push_back("log-state"); }
static void log_reset() { events.push_back("log-reset"); }
static int top_menu(int) { events.push_back("top-menu"); return 0; }
static void pop_close() { events.push_back("pop-close"); }
static void do_all_tasks() { events.push_back("tasks"); }
static void mapwin_system_shutdown() { events.push_back("mapwin-shutdown"); }
static void stop_timers() { events.push_back("stop-timers"); }
static void labels_shutdown() { events.push_back("labels-shutdown"); }
static int game_init(int, int) { events.push_back("game-init"); return MockGameInitResult; }
static void desktop_init(int) { events.push_back("desktop-init"); }
static void planetfall(int) { events.push_back("planetfall"); }
static int num_objectives(int faction, int mask) {
    multi_objective_faction=faction; multi_objective_mask=mask;
    events.push_back("objectives"); return faction + 7;
}
static int parse_says(int id, const char *, int, int) {
    if (multi_parse_count<8) multi_parsed_factions[multi_parse_count++]=ConsoleGlobal.faction_id_;
    if (multi_parse_count<=multi_faction_mutations)
        ConsoleGlobal.faction_id_=multi_faction_after[multi_parse_count-1];
    events.push_back(std::string("says-") + char('0' + id)); return 0;
}
static int parse_num(int id, int) { events.push_back(std::string("num-") + char('0' + id)); return 0; }
static void say_year(char *) { events.push_back("say-year"); }
static int X_pop(char *caption, const char *label, int (*)()) {
    assert(strcmp(caption, "INTRO") == 0 && strcmp(label, "SCENARIO") == 0);
    events.push_back("scenario-popup");
    return 0;
}

// @@RESET_BODY@@
// @@BODY_multiplayer_init@@

static void reset() {
    events.clear();
    TextNumbers.clear();
    TextNumberIndex = 0;
    TextOpenResult = 1;
    memset(&NetDaemonNet, 0, sizeof(NetDaemonNet));
    memset(&NetWinSessionData, 0, sizeof(NetWinSessionData));
    memset(&NetWinSessionOptions, 0, sizeof(NetWinSessionOptions));
    memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    memset(Players, 0, sizeof(Players));
    memset(PlayersData, 0, sizeof(PlayersData));
    IsMultiplayerNet = IsMultiplayerPBEM = ExitTurnLoop = 0;
    GameRules = GameState = NetTurnFlags = LockEnableMask = 0;
    DiffLevelCurrent = TimeControlsSelection = ControlTurnPhase = VehCurrentCount = 0;
    UnkBitfield1 = 0;
    NetWinPlayerId = 17;
    memset(multi_parsed_factions,0,sizeof(multi_parsed_factions));
    memset(multi_faction_after,0,sizeof(multi_faction_after));
    multi_parse_count=multi_faction_mutations=0;
    multi_objective_faction=multi_objective_mask=0;
}

int main() {
    reset();
    IsMultiplayerPBEM = 1;
    assert(multiplayer_init(1) == 1);
    assert(IsMultiplayerNet == 0);
    assert(events.size() >= 3 && events[0] == "log-state" && events[1] == "config-players" && events[2] == "top-menu");
    assert(NetDaemonNet.voice_hotkey_48_ == 0xDC);
    assert(NetDaemonNet.voice_flags_D8_ == 1);
    assert(NetDaemonNet.voice_hotkey_4c_ == 0x100DC);

    reset();
    NetDaemonNet.init_result = 4;
    const int failed_init = multiplayer_init(0);
    assert(failed_init == 1);
    assert(IsMultiplayerNet == 0);
    assert(events[2] == "daemon-init");
    assert(events[events.size() - 1] == "daemon-close");

    reset();
    NetWinDialog.exec_result = 1;
    NetWinSessionData.mode_ = 0;
    NetWinSessionOptions.planet_size_ = 2;
    NetWinSessionOptions.ocean_coverage_ = 1;
    NetWinSessionOptions.erosion_ = 2;
    NetWinSessionOptions.cloud_cover_ = 3;
    NetWinSessionOptions.native_life_ = 4;
    NetWinSessionOptions.rule_bits_c_ = 0x10;
    NetDaemonNet.player_count_ = 2;
    NetDaemonNet.process_slots_[1].identity_ = 1;
    NetDaemonNet.process_slots_[2].identity_ = 4;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    ControlTurnPhase = 1;
    assert(multiplayer_init(1) == 0);
    assert(MapSizePlanet == 2 && MapLongitudeBounds == 0x28 && MapLatitudeBounds == 0x50);
    assert(MapOceanCoverage == 1 && MapLandCoverage == 2);
    assert(MapPlanetaryOrbit == 1 && MapErosiveForces == 2);
    assert(MapCloudCover == 3 && MapNativeLifeForms == 4);
    assert((UnkBitfield1 & 8u) != 0);
    assert(events[2] == "exec" && events[3] == "text-open");
    assert(events[4] == "map-init" && events[5] == "path-init");
    assert(events[events.size() - 3] == "send-rules");
    assert(events[events.size() - 2] == "send-game");
    assert(events[events.size() - 1] == "lock-clear");

    reset();
    NetWinDialog.exec_result = 1;
    NetWinSessionData.mode_ = 0;
    NetWinSessionOptions.planet_size_ = 3;
    NetWinSessionOptions.ocean_coverage_ = 1;
    NetWinSessionOptions.erosion_ = 2;
    NetWinSessionOptions.cloud_cover_ = 3;
    NetWinSessionOptions.native_life_ = 4;
    TextOpenResult = 0;
    TextNumbers.push_back(5);
    for (int i = 0; i != 5; ++i) {
        TextNumbers.push_back(10 + i);
        TextNumbers.push_back(20 + i);
    }
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    ControlTurnPhase = 1;
    assert(multiplayer_init(1) == 0);
    assert(TextNumberIndex == TextNumbers.size());
    assert(MapSizePlanet == 3 && MapLongitudeBounds == 26 && MapLatitudeBounds == 23);
    assert(events[2] == "exec" && events[3] == "text-open");
    assert(events[4] == "text-get" && events[5] == "text-number");

    reset();
    NetWinDialog.exec_result = 1;
    NetWinSessionData.mode_ = 2;
    NetWinSessionOptions.difficulty_ = 0xFE;
    NetWinSessionOptions.rule_bits_a_ = 0x11223344;
    NetWinSessionOptions.rule_bits_b_ = 0x01800020;
    GameState = 0x20;
    NetWinSessionOptions.rule_bits_c_ = 0x10;
    NetWinSessionOptions.time_control_ = 0xFD;
    NetDaemonNet.player_count_ = 2;
    NetDaemonNet.process_slots_[1].identity_ = 0;
    NetDaemonNet.process_slots_[2].identity_ = 3;
    NetDaemonNet.negative_loop_probe = 1;
    NetWinLobbyRows[1].owner_ = 0xAABBCCDD;
    NetWinLobbyRows[2].owner_ = 0x1122;
    ControlTurnPhase = 1;
    assert(multiplayer_init(0) == 1);
    assert(DiffLevelCurrent == -2 && GameRules == 0x11223344);
    assert((GameState & 0x01800000) == 0x01800000);
    assert((GameState & 0x20) != 0 && (NetTurnFlags & 0x10));
    assert(TimeControlsSelection == 253);
    assert(static_cast<uint32_t>(PlayersData[0].unk_99) == 0xAABBCCDDu);
    assert(PlayersData[3].unk_99 == 0x1122);
    assert(NetDaemonNet.receive_count == 2);
    assert(events[events.size() - 1] == "daemon-close");

    reset();
    NetWinDialog.exec_result = 1;
    NetWinSessionData.mode_ = 3;
    NetWinSessionOptions.rule_bits_a_ = 0xA5A5;
    NetWinSessionOptions.rule_bits_b_ = 0x02000000;
    NetWinSessionOptions.rule_bits_c_ = 0;
    NetDaemonNet.send_files_result = 1;
    assert(multiplayer_init(1) == 1);
    assert(events[events.size() - 4] == "pop-close");
    assert(IsMultiplayerNet == 0);

    reset();
    NetWinDialog.exec_result = 1;
    NetWinSessionData.mode_ = 3;
    NetWinSessionOptions.rule_bits_a_ = 0x1000;
    NetWinSessionOptions.rule_bits_b_ = 0x01000000;
    NetWinSessionOptions.rule_bits_c_ = 0x10;
    Players[2].is_leader_female=1; Players[2].noun_gender=2; Players[2].is_noun_plural=0;
    Players[3].is_leader_female=0; Players[3].noun_gender=3; Players[3].is_noun_plural=1;
    Players[4].is_leader_female=1; Players[4].noun_gender=4; Players[4].is_noun_plural=0;
    multi_faction_after[0]=3; multi_faction_after[1]=4; multi_faction_after[2]=5;
    multi_faction_mutations=3;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    MockGameInitResult = 0;
    ControlTurnPhase = 1;
    assert(multiplayer_init(1) == 0);
    assert(events[0] == "log-state" && events[1] == "config-players");
    assert(events[events.size() - 1] == "scenario-popup");
    assert(events[events.size() - 2] == "num-3");
    assert(events[events.size() - 3] == "num-2");
    assert(events[events.size() - 4] == "objectives");
    assert(events[events.size() - 5] == "num-1");
    assert(events[events.size() - 6] == "num-0");
    assert(events[events.size() - 7] == "says-4");
    assert(events[events.size() - 8] == "say-year");
    assert(events[events.size() - 9] == "says-3");
    assert(events[events.size() - 10] == "says-2");
    assert(events[events.size() - 11] == "says-1");
    assert(multi_parse_count==4 && multi_parsed_factions[0]==2 &&
           multi_parsed_factions[1]==3 && multi_parsed_factions[2]==4);
    assert(multi_objective_faction==5 && multi_objective_mask==(GameRules & 0x1000));
    assert(WorldClimateSkipTerrainClear == 0);
    return 0;
}

// @@ENDCASE_multiplayer_init@@

// @@CASE_multiplayer_reset@@
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

struct Payload {
    unsigned int flags;
};
struct LockStateEntry {
    unsigned int leading_words_[4];
    Payload payload_;
    unsigned int trailing_words_[10];
};
typedef char lock_state_entry_must_be_0x3c[
    sizeof(LockStateEntry) == 0x3c ? 1 : -1];

struct Lock {
    int calls;
    void clear() { ++calls; }
};
struct NetDaemonDouble {
    struct DeletionListState { unsigned char records_[24][0x3C]; int current_owner_; } deletion_list_;
    struct PrimaryState { unsigned int leading_words_[3]; unsigned int flags_; unsigned int trailing_words_[10]; } primary_state_;
    LockStateEntry channels_[2];
    void *file_index_records_;
    unsigned int field_1BE4_;
    unsigned char saved_process_slots_[8][0x19C];
    Lock locks_;
    int close_calls;
    void close() { ++close_calls; }
};
typedef NetDaemonDouble NetDaemon;
static NetDaemonDouble NetDaemonNet;

// @@RESET_BODY@@

int main() {
    memset(&NetDaemonNet.deletion_list_, 0x5A, sizeof NetDaemonNet.deletion_list_);
    memset(&NetDaemonNet.primary_state_, 0x5A, sizeof NetDaemonNet.primary_state_);
    memset(NetDaemonNet.channels_, 0x5A, sizeof NetDaemonNet.channels_);
    memset(NetDaemonNet.saved_process_slots_, 0x5A, sizeof NetDaemonNet.saved_process_slots_);
    NetDaemonNet.file_index_records_ = reinterpret_cast<void *>(static_cast<uintptr_t>(0x12345678));
    NetDaemonNet.field_1BE4_ = 0x5A5A5A5Au;
    unsigned char deletion_snapshot[sizeof NetDaemonNet.deletion_list_];
    unsigned char saved_slots_snapshot[sizeof NetDaemonNet.saved_process_slots_];
    memcpy(deletion_snapshot, &NetDaemonNet.deletion_list_, sizeof deletion_snapshot);
    memcpy(saved_slots_snapshot, NetDaemonNet.saved_process_slots_, sizeof saved_slots_snapshot);
    NetDaemonNet.locks_.calls = 0;
    NetDaemonNet.close_calls = 0;

    multiplayer_reset_network_state_owner_v1();

    // Independently project the native write-address list to the typed array.
    // Only these words are reset; adjacent fields remain sentinel-filled.
    unsigned int expected[3][15];
    for (int entry = 0; entry != 3; ++entry)
        for (int word = 0; word != 15; ++word)
            expected[entry][word] = 0x5A5A5A5A;
#define ZERO(entry, word) expected[(entry)][(word)] = 0
    ZERO(0, 1); ZERO(0, 2); ZERO(0, 3); ZERO(0, 4);
    ZERO(0, 13); ZERO(0, 12); ZERO(1, 1); ZERO(1, 0);
    ZERO(1, 5); ZERO(2, 1); ZERO(1, 14); ZERO(2, 0);
    ZERO(2, 10); ZERO(2, 9); ZERO(0, 9); ZERO(0, 10);
    ZERO(0, 11); ZERO(2, 8); ZERO(2, 7); ZERO(2, 3);
    ZERO(2, 2); ZERO(2, 4); ZERO(0, 5); ZERO(0, 6);
    ZERO(2, 13); ZERO(2, 12); ZERO(0, 7); ZERO(0, 8);
    expected[2][11] = 0xFF;
    ZERO(1, 4);
    for (int word = 6; word <= 13; ++word) ZERO(1, word);
#undef ZERO

    for (int entry = 0; entry != 3; ++entry) {
        unsigned int actual[15];
        if (entry == 0) {
            actual[0] = static_cast<unsigned int>(NetDaemonNet.deletion_list_.current_owner_);
            for (int i = 0; i != 3; ++i) actual[1 + i] = NetDaemonNet.primary_state_.leading_words_[i];
            actual[4] = NetDaemonNet.primary_state_.flags_;
            for (int i = 0; i != 10; ++i) actual[5 + i] = NetDaemonNet.primary_state_.trailing_words_[i];
        } else {
            const LockStateEntry &channel = NetDaemonNet.channels_[entry - 1];
            for (int i = 0; i != 4; ++i) actual[i] = channel.leading_words_[i];
            actual[4] = channel.payload_.flags;
            for (int i = 0; i != 10; ++i) actual[5 + i] = channel.trailing_words_[i];
        }
        for (int word = 0; word != 15; ++word) {
            if (actual[word] != expected[entry][word]) {
                std::fprintf(stderr,
                             "state[%d].word[%d]: got %08x expected %08x\n",
                             entry, word, actual[word], expected[entry][word]);
                return 1;
            }
        }
    }
    if (memcmp(deletion_snapshot, &NetDaemonNet.deletion_list_, sizeof deletion_snapshot) != 0 ||
        memcmp(saved_slots_snapshot, NetDaemonNet.saved_process_slots_, sizeof saved_slots_snapshot) != 0 ||
        NetDaemonNet.file_index_records_ != reinterpret_cast<void *>(static_cast<uintptr_t>(0x12345678)) ||
        NetDaemonNet.field_1BE4_ != 0x5A5A5A5Au) {
        std::fprintf(stderr, "untouched deletion/save/file-index state changed\n");
        return 1;
    }
    if (NetDaemonNet.locks_.calls != 1 || NetDaemonNet.close_calls != 1) {
        std::fprintf(stderr, "cleanup call counts differ\n");
        return 1;
    }
    std::puts("multiplayer reset-state typed control: PASS");
    return 0;
}

// @@ENDCASE_multiplayer_reset@@
