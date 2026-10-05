
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef const char *LPCSTR;
typedef int BOOL;
struct MapWin {
    uint32_t field_1DD70_, field_1DD74_, field_1DD78_;
    unsigned char gap_[0x1DD98 - 0x1DD7C];
    int field_1DD98_;
    uint32_t field_1DD9C_, field_1DDA0_;
    int closes_;
    void close() { ++closes_; }
};
struct Map { unsigned char visibility; unsigned char pad[3]; uint32_t bit; };
struct PlayerData { uint32_t flags; uint32_t satellites_[4]; };
struct TechnologyData { uint32_t flags; };
struct Base { unsigned char faction_id_current; };
struct Veh { short proto_id; unsigned char pad[2]; unsigned char faction_id; };
struct VehPrototype { unsigned char pad[0x2A]; unsigned char plan; };
struct MapDataTransferHeader { char before[0x380]; int format_version_; char after[0x14]; };
struct ConsoleStub { int faction_id_, cursor_history_index_; int cursor_history_x_[32], cursor_history_y_[32]; uint32_t cursor_active_; int selected_vehicle_; uint32_t field_23BE4_, field_23BE8_; };
struct PathStub { int calls; void init() { ++calls; } };
struct FileBox {
    char file_[0x104];
    void set_def_name(const char *value) { def_name = value; }
    int set_def_ext(const char *value) { def_ext = value; return 0; }
    int add_filter(char *filter, char *) { filters.push_back(filter); return 0; }
    char *save() { ++save_calls; std::strcpy(file_, chosen_file); return save_succeeds ? file_ : 0; }
    static std::string def_name, def_ext;
    static std::vector<std::string> filters;
    static int save_calls;
    static bool save_succeeds;
    static char chosen_file[260];
};
std::string FileBox::def_name, FileBox::def_ext;
std::vector<std::string> FileBox::filters;
int FileBox::save_calls;
bool FileBox::save_succeeds;
char FileBox::chosen_file[260];
static const uint32_t RULES_IRONMAN = 0x400;
static const uint32_t PFLAG_UNK_20000 = 0x20000;
static const int PLAN_COLONIZATION = 8;
static const int MaxPlayerNum = 8, MaxTechnologyNum = 89;
static int VehMoveDeltaX[9] = {1,2,1,0,-1,-2,-1,0,0};
static int VehMoveDeltaY[9] = {-1,0,1,2,1,0,-1,-2,0};
int GameDataWriteMode, GameDataEncryptedMode, GameDataLoadStatusWord;
static uint32_t GameRules, IsMultiplayerNet, SaveDaemonStateWord, NetTurnFlags, GameState;
static int IsMultiplayerPBEM, ControlTurnPhase, tamper_calls, rebuild_vehicle_calls, rebuild_base_calls;
static uint32_t UnkBitfield1;
static char GameSaveMultiplayerMarker[] = "multi";
static uint32_t LockEnableMask;
static PlayerData PlayersData[MaxPlayerNum];
static TechnologyData Technology[MaxTechnologyNum];
static Base Bases[16];
static int BaseCurrentCount, VehCurrentCount, BaseComputedID;
static Veh Vehs[16];
static VehPrototype VehPrototypes[16];
struct ButtonStub { int shown, hidden; void show(int) { ++shown; } void hide() { ++hidden; } };
struct MainInterfaceStub { ButtonStub flat_button_[3]; };
static MainInterfaceStub MainInterfaceGlobal;
static ConsoleStub ConsoleGlobal;
static MapWin *MapWinTable[8];
static const size_t MapWinTableSlots = 8;
static unsigned int MapArea = 2;
static Map MapStorage[2], *MapTiles = MapStorage;
static uint32_t AltContour[11], AltNatural[11];
static PathStub Paths;
static char MapFilePath[80] = "maps\\default";
static char StringTemp[1032] = "fallback.map";
static const char *MapFileExtension = "MP";
static const char *SaveGameFileExtension = "SAV";
static const char MapHeaderText[] = "TERRANMAP", SaveHeaderText[] = "TERRAN";
static const char OpenFileMode[] = "rb", SaveFileMode[] = "wb";
static const char GameSaveReadMode[] = "rb", GameSaveHeaderText[] = "TERRAN";
static const char PathDot[] = ".", MapSaveHeaderText[] = "TERRANMAP";
static const char MapFileFilterPattern[] = "*.", EmptyMapName[] = "?";
static const char LatestMapPreference[] = "Latest Map";
static char mock_header[256] = "TERRAN";
static int mock_version = 5, mock_map_format = 2, mock_game_data_result = 1;
static int open_fails, map_result, fail_io_at, io_calls, close_calls, null_closes, remove_calls;
static int game_data_calls, linearize_calls, random_seed, draws, map_reads, map_writes;
static int preference_calls, label_value;
static std::string last_path, last_mode, last_header, last_filter, saved_name, preference_key, preference_value;

size_t host_fread(void *out, size_t size, size_t count, FILE *) {
    ++io_calls;
    if (out != 0 && size * count != 0) std::memset(out, 0, size * count);
    if (size == sizeof(MapDataTransferHeader))
        ((MapDataTransferHeader *)out)->format_version_ = mock_map_format;
    if (size == sizeof(int) && count == 1) *(int *)out = mock_version;
    return io_calls == fail_io_at ? 0 : count;
}
size_t host_fwrite(const void *, size_t, size_t count, FILE *) {
    ++io_calls;
    return io_calls == fail_io_at ? 0 : count;
}
int host_close(FILE *file) { ++close_calls; if (!file) ++null_closes; return 0; }
int host_remove(const char *path) { ++remove_calls; last_path = path; return 0; }
#define fread host_fread
#define fwrite host_fwrite
#define _fread host_fread
#define _fwrite host_fwrite
#define fclose host_close
#define remove host_remove
static int _stricmp(const char *left, const char *right) {
    while (*left && *right) {
        int a = (*left >= 'a' && *left <= 'z') ? *left - 32 : *left;
        int b = (*right >= 'a' && *right <= 'z') ? *right - 32 : *right;
        if (a != b) return a - b;
        ++left; ++right;
    }
    return *left - *right;
}
FILE *env_open(const char *path, const char *mode) {
    last_path = path; last_mode = mode;
    return open_fails ? 0 : reinterpret_cast<FILE *>(1);
}
void header_check(char *buffer, FILE *) { std::strcpy(buffer, mock_header); }
void header_write(const char *header, FILE *) { last_header = header; }
int encrypt_read(void *, unsigned int, unsigned int, FILE *) { ++io_calls; return io_calls == fail_io_at ? 0 : 1; }
int encrypt_write(void *, unsigned int, unsigned int, FILE *) { ++io_calls; return io_calls == fail_io_at ? 0 : 1; }
int map_read(FILE *) { ++map_reads; return map_result; }
int map_write(FILE *) { ++map_writes; return map_result; }
int game_data(FILE *, int) { ++game_data_calls; return mock_game_data_result; }
void my_srand(int seed) { random_seed = seed; }
void world_linearize_contours() { ++linearize_calls; }
void rebuild_vehicle_bits() { ++rebuild_vehicle_calls; }
void rebuild_base_bits() { ++rebuild_base_calls; }
void check_tamper() { ++tamper_calls; }
int bit_count(uint32_t value) { int count = 0; while (value) { count += value & 1U; value >>= 1; } return count; }
int has_tech(int technology, int faction) { return technology == 5 && faction == 2; }
void do_all_draws() { ++draws; }
char *label_get(int value) { label_value = value; return (char *)"Map files"; }
void prefs_put(const char *key, const char *value) { ++preference_calls; preference_key = key; preference_value = value; }

extern "C" int __cdecl sub_5a6510(int x, int y);

/* ACTUAL_SAVE_LOAD_BODIES */

int main() {
    assert(sub_5a6510(-4, -9) == 6 && sub_5a6510(1, 1) == 2 && sub_5a6510(0, 0) == 8);
    MapWin record = {};
    GameDataEncryptedMode = 1; GameDataWriteMode = 1; io_calls = fail_io_at = 0;
    assert(game_io(&record, 0) == 0 && io_calls == 6);
    io_calls = 0; fail_io_at = 3;
    assert(game_io(&record, 0) == 1 && io_calls == 3);
    GameDataEncryptedMode = 0; io_calls = fail_io_at = 0;
    assert(map_data((FILE *)1, 0, 1) == 0 && map_reads == 1 && io_calls == 5);
    assert(GameDataWriteMode == 0 && GameDataEncryptedMode == 0);
    io_calls = 0; fail_io_at = 3; map_reads = 0;
    assert(map_data((FILE *)1, 1, 1) == 1 && map_reads == 0 && io_calls == 3);
    io_calls = 0; fail_io_at = 0; map_writes = 0; map_result = 0;
    assert(map_data((FILE *)1, 1, 0) == 0 && map_writes == 1 && io_calls == 0);
    mock_map_format = 3; io_calls = 0;
    assert(map_data((FILE *)1, 0, 1) == 0 && io_calls == 5);

    MapWin w1 = {}, w3 = {};
    w1.field_1DD74_ = 1; w3.field_1DD74_ = 1;
    MapWinTable[1] = &w1; MapWinTable[3] = &w3;
    open_fails = 1;
    assert(load_map_daemon((char *)"map.mp") == 1 && close_calls == 0);
    open_fails = 0; std::strcpy(mock_header, "TERRANMAP"); mock_version = 4;
    assert(load_map_daemon((char *)"map.mp") == 3 && w1.closes_ == 0);
    mock_version = 5; map_result = 0; io_calls = 0;
    for (int map_contour = 0; map_contour < 11; ++map_contour) {
        AltContour[map_contour] = 700u + static_cast<uint32_t>(map_contour);
        AltNatural[map_contour] = 200u + static_cast<uint32_t>(map_contour);
    }
    const int successful_map_load = load_map_daemon((char *)"map.mp");
    assert(successful_map_load == 0 && w1.closes_ == 1);
    for (int copied_contour = 0; copied_contour < 11; ++copied_contour) {
        assert(AltNatural[copied_contour] == 700u + static_cast<uint32_t>(copied_contour));
        assert(AltContour[copied_contour] == 700u + static_cast<uint32_t>(copied_contour));
    }
    assert(ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.selected_vehicle_ == -1);
    assert(BaseCurrentCount == 0 && VehCurrentCount == 0 && BaseComputedID == -1 && Paths.calls == 1);

    // Native loader uses signed tests: an invalid negative area skips the
    // tile walk rather than treating the word as billions of unsigned tiles.
    MapArea = 0xFFFFFFFFu;
    MapTiles[0].visibility = 0xA5;
    MapTiles[0].bit = 0x1234567Bu;
    assert(load_map_daemon((char *)"map.mp") == 0);
    assert(MapTiles[0].visibility == 0xA5 && MapTiles[0].bit == 0x1234567Bu);
    MapArea = 2;

    open_fails = 1; close_calls = null_closes = remove_calls = 0;
    assert(save_map_daemon((char *)"scenario") == 1);
    assert(last_path == "scenario.MP" && close_calls == 1 && null_closes == 1 && remove_calls == 1);
    open_fails = 0; fail_io_at = 1; io_calls = 0;
    assert(save_map_daemon((char *)"scenario.SC") == 2 && last_header == "TERRANMAP");
    assert(close_calls == 2 && remove_calls == 2);

    std::strcpy(mock_header, "TERRAN"); mock_version = 0x56;
    GameRules = RULES_IRONMAN; IsMultiplayerNet = 0; SaveDaemonStateWord = 1;
    mock_game_data_result = 0; fail_io_at = 0; io_calls = 0; game_data_calls = 0;
    BaseCurrentCount = 1; Bases[0].faction_id_current = 4;
    VehCurrentCount = 1; Vehs[0].proto_id = 0; Vehs[0].faction_id = 5;
    VehPrototypes[0].plan = PLAN_COLONIZATION;
    LockEnableMask = 0xA51234FFu;
    const uint32_t preserved = LockEnableMask & 0xFFFF00FFu;
    assert(load_daemon("slot.sav", 1) == 0 && game_data_calls == 1);
    assert(linearize_calls == 1 && random_seed == 0x56 && remove_calls >= 3);
    assert((LockEnableMask & 0xFFFF00FFu) == preserved);
    assert(((LockEnableMask >> 8) & 0xFFu) == 0x30u);
    assert(w3.closes_ == 1 && w3.field_1DD74_ == 0);

    // Cancellation may retain nonempty text: test the real returned pointer,
    // not a fabricated filename-is-empty cancellation contract.
    std::strcpy(FileBox::chosen_file, "cancel.SC");
    FileBox::save_succeeds = false;
    save_map();
    assert(draws == 1 && preference_calls == 0 && label_value == 0x678 / 4);
    assert(FileBox::filters.size() == 1 && FileBox::filters[0] == "*.MP");
    std::strcpy(FileBox::chosen_file, "maps\\Europa.MP");
    FileBox::save_succeeds = true;
    save_map();
    assert(draws == 2 && last_path == "maps\\Europa.MP" && last_mode == "wb");
    assert(preference_key == "Latest Map" && preference_value == "maps\\Europa");
    std::strcpy(StringTemp, "fallback.map"); std::strcpy(FileBox::chosen_file, "?");
    save_map();
    assert(last_path == "fallback.map" && preference_value == "fallback");
    // Native failure status is read from the distinct 0x945D7C owner.
    mock_game_data_result = 1; GameRules = 0; IsMultiplayerNet = 0;
    GameDataLoadStatusWord = 0;
    const int closes_before_failure = close_calls;
    assert(load_daemon("slot.sav", 0) == 4 && close_calls == closes_before_failure + 1);
    GameDataLoadStatusWord = 1; mock_game_data_result = 1;
    assert(load_daemon("slot.sav", 0) == 5 && close_calls == closes_before_failure + 2);

    // Successful tail: cursor history broadcast, button branch, turn owner,
    // PBEM/tamper selection and final transient flag update.
    mock_game_data_result = 0; GameDataLoadStatusWord = 0;
    ConsoleGlobal.cursor_history_index_ = 3;
    for (int contour_seed = 0; contour_seed < 11; ++contour_seed) {
        AltContour[contour_seed] = 900u + static_cast<uint32_t>(contour_seed);
        AltNatural[contour_seed] = 100u + static_cast<uint32_t>(contour_seed);
    }
    ConsoleGlobal.cursor_history_x_[3] = 17; ConsoleGlobal.cursor_history_y_[3] = 29;
    ConsoleGlobal.cursor_active_ = 1; ConsoleGlobal.faction_id_ = 4;
    GameRules = 0; IsMultiplayerNet = 0; NetTurnFlags = 0;
    LockEnableMask = 0; UnkBitfield1 = 0; tamper_calls = 0; GameState = 0;
    assert(load_daemon("slot.multi.sav", 0) == 0);
    assert(MainInterfaceGlobal.flat_button_[2].shown == 1);
    assert((GameState & 0x10C0u) == 0x10C0u && ConsoleGlobal.cursor_active_ == 0);
    assert(ControlTurnPhase == 4 && IsMultiplayerPBEM == 0 && tamper_calls == 0);
    assert(UnkBitfield1 & 4u);
    assert(rebuild_vehicle_calls >= 1 && rebuild_base_calls >= 1);
    for (int contour_result = 0; contour_result < 11; ++contour_result) {
        assert(AltNatural[contour_result] == 900u + static_cast<uint32_t>(contour_result));
        assert(AltContour[contour_result] == 900u + static_cast<uint32_t>(contour_result));
    }
    for (int history_slot = 0; history_slot < 32; ++history_slot) {
        assert(ConsoleGlobal.cursor_history_x_[history_slot] == 17);
        assert(ConsoleGlobal.cursor_history_y_[history_slot] == 29);
    }
    NetTurnFlags = 0x20;
    assert(load_daemon("slot.sav", 0) == 0);
    assert(IsMultiplayerPBEM == 1 && tamper_calls == 1);

    // Pinned5A9C8C..98 selects TERRAN for SAV and TERRANMAP otherwise;
    // the saved-game path also requires version0x56 rather than scenario5.
    std::strcpy(mock_header, "TERRANMAP");
    mock_version = 0x56;
    fail_io_at = 0;
    map_result = 0;
    assert(load_map_daemon((char *)"slot.SAV") == 2);
    std::strcpy(mock_header, "TERRAN");
    mock_version = 0x55;
    assert(load_map_daemon((char *)"slot.SAV") == 3);
    mock_version = 0x56;
    assert(load_map_daemon((char *)"slot.SAV") == 0);
    return 0;
}
