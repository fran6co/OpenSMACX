Base Bases[512];
PlayerData PlayersData[8];
Player Players[8];
Veh Vehs[8192];
RulesFacility Facility[134];
Map MapTiles[4096];
GameDataStateWindow GameDataStateStorage;
unsigned int MapLongitude, UnkBitfield1;
int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat, IsMultiplayerNet,
    BaseIDCurrentSelected;
Console ConsoleGlobal;
NetMessage NetMsgPrimary;
Window WorldClimateWorldWin, TutWinBaseWindow;
char *ScriptPopupFile;
int GenderDefault, PluralityDefault;
int BaseKillCalls, BaseResetCalls, SetBaseCalls, ComputeCalls;
int OwnerSetCalls, DrawCalls, EliminateCalls, StealTechCalls;
const int RadiusOffsetX[289] = {0};
const int RadiusOffsetY[289] = {0};

int base_project(int id) {
    return GameDataStateStorage.fields_.secret_project_[id];
}
void treaty_off(int, int, int) {}
void treaty_on(int, int, int) {}
void log_say(const char *, const char *, int, int, int) {}
void parse_says(int, const char *, int, int) {}
void parse_say(int, int, int, int) {}
void parse_num(int, int) {}
int popp(const char *, const char *, int, const char *, int (*)()) { return 0; }
int X_pop(const char *, int (*)()) { return 0; }
void bitmask(int id, int *index, int *mask) {
    *index = id >> 5;
    *mask = 1 << (id & 31);
}
void set_fac(int, int, int) {}
void base_reset(int, int) { ++BaseResetCalls; }
int is_objective(int) { return 0; }
void base_kill(int) { ++BaseKillCalls; }
void draw_map(int) { ++DrawCalls; }
void replay_base(int, int, int, int) {}
void using_set(int, int, int) {}
void synch_bit(int, int, int) {}
int veh_at(int, int) { return -1; }
void spot_base(int, int) {}
void reset_territory() {}
int base_find(int, int, int) { return -1; }
void kill(int) {}
int has_tech(int, int) { return 0; }
void base_first(int) {}
void owner_set(int, int, int) { ++OwnerSetCalls; }
void draw_tile_fixup(int, int, int, int) {}
void ambience(int) {}
int spying(int) { return 0; }
void draw_tile(int, int, int) {}
void interlude(int, char *, int, int) {}
void veh_init(int, int, int, int) {}
void steal_tech(int, int, int) { ++StealTechCalls; }
void bases_reset(int, int, int) {}
void mon_conquer_base(int, char *) {}
void make_base_unique(int) {}
void text_close() {}
void eliminate_player(int, int) { ++EliminateCalls; }
void set_base(int) { ++SetBaseCalls; }
void base_compute(int) { ++ComputeCalls; }
char *label_get(int) { static char direction[] = "direction"; return direction; }

static void reset_fixture() {
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(Vehs, 0, sizeof(Vehs));
    memset(Facility, 0, sizeof(Facility));
    memset(MapTiles, 0, sizeof(MapTiles));
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    memset(&GameDataStateStorage.fields_.secret_project_, 0xff,
           sizeof(GameDataStateStorage.fields_.secret_project_));
    GameDataStateStorage.fields_.base_current_count_ = 2;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    MapLongitude = 16;
    MapLongitudeBounds = 32;
    MapLatitudeBounds = 32;
    MapIsFlat = 0;
    IsMultiplayerNet = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 7;
    GameDataStateStorage.fields_.game_preferences_ = 0;
    GameDataStateStorage.fields_.game_rules_ = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    UnkBitfield1 = 0;
    BaseIDCurrentSelected = 0;
    ConsoleGlobal.faction_id_ = 0;
    GameDataStateStorage.fields_.secret_project_.universal_translator() = -1;
    GameDataStateStorage.fields_.secret_project_.network_backbone() = -1;
    ScriptPopupFile = const_cast<char *>("popup.txt");
    BaseKillCalls = BaseResetCalls = SetBaseCalls = ComputeCalls = 0;
    OwnerSetCalls = DrawCalls = EliminateCalls = StealTechCalls = 0;
    strcpy(Bases[0].name_string, "Alpha");
    strcpy(Bases[1].name_string, "Beta");
    Bases[0].x = 8; Bases[0].y = 8;
    Bases[1].x = 12; Bases[1].y = 8;
    Bases[0].faction_id_current = 1;
    Bases[0].faction_id_former = 0;
    Bases[0].population_size = 3;
    Bases[0].queue_production_id[0] = -69;
    Bases[0].queue_size = 0;
    Bases[1].faction_id_current = 1;
    PlayersData[1].energy_reserves = 100;
    PlayersData[1].pop_total = 5;
    PlayersData[1].current_num_bases = 1;
    PlayersData[2].current_num_bases = 0;
}

int main() {
    reset_fixture();
    capture_base(0, 2, 1);
    if (Bases[0].faction_id_current != 2 || Bases[0].population_size != 3) return 1;
    if (PlayersData[2].energy_reserves != 50 || PlayersData[1].energy_reserves != 50) return 2;
    if (PlayersData[1].current_num_bases != 0 || PlayersData[2].current_num_bases != 1) return 3;
    if (OwnerSetCalls != 1 || EliminateCalls != 1 || SetBaseCalls != 1 || ComputeCalls != 1) return 4;

    reset_fixture();
    Bases[0].population_size = 1;
    Bases[0].visibility = 1;
    capture_base(0, 2, 0);
    if (Bases[0].population_size != 0 || BaseKillCalls != 1) return 5;
    if (OwnerSetCalls != 1 || EliminateCalls != 1 || SetBaseCalls != 0) return 6;

    reset_fixture();
    GameDataStateStorage.fields_.game_rules_ = 0x4000;
    GameDataStateStorage.fields_.secret_project_.universal_translator() = 0;
    GameDataStateStorage.fields_.secret_project_.network_backbone() = 1;
    capture_base(0, 2, 1);
    if (StealTechCalls != 2) return 7;
    return 0;
}
