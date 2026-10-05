#include <cassert>
#include <cstddef>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <string>

#define SAVE_TEST_JOIN_INNER(a, b) a##b
#define SAVE_TEST_JOIN(a, b) SAVE_TEST_JOIN_INNER(a, b)
#define static_assert(expression, message) \
    typedef char SAVE_TEST_JOIN(static_assertion_, __LINE__)[(expression) ? 1 : -1]

typedef uint32_t DWORD;
typedef const char *LPCSTR;
typedef int BOOL;
#ifndef __cdecl
#define __cdecl
#endif
enum { RULES_IRONMAN = 0x400 };

// This host-only projection represents the target's 32-bit borrowed pointer
// word. The production slot and save-view declarations below are extracted
// verbatim by the pytest harness; product-source assertions guard this shim.
// ACTUAL_STORAGE_DECLARATIONS
typedef AlphaNetProcessSlot AlphaNetProcessSlotHost;

struct NetDaemonHost {
    int player_count_;
    AlphaNetProcessSlotHost process_slots_[8];
    NetDaemonSaveStorageView save_storage_;
} NetDaemonNet;
uint32_t GameRules;
uint32_t GameState;
BOOL IsMultiplayerPBEM;
BOOL IsMultiplayerNet;
int TamperState93A9B4;
int SaveDaemonStateWord;
uint32_t NetTurnFlags;
int GameDataPlayerCountSnapshot;

static const char PathDot[] = ".";
static const char SaveGameFileExtension[] = "SAV";
static const char SaveFileMode[] = "wb";
static const char SaveHeaderText[] = "TERRAN";
static FILE fake_file_storage;
static FILE *const fake_file = &fake_file_storage;
static bool open_succeeds;
static int version_write_result;
static int tick_write_result;
static int serializer_result;
static DWORD tick_value;
static int seed_calls;
static int seed_value;
static int game_data_calls;
static int close_calls;
static int remove_calls;
static std::string opened_path;
static std::string removed_path;
static std::string calls;

static void reset_fixture() {
    std::memset(static_cast<void *>(&NetDaemonNet), 0, sizeof NetDaemonNet);
    GameRules = GameState = NetTurnFlags = 0;
    IsMultiplayerPBEM = IsMultiplayerNet = 0;
    TamperState93A9B4 = SaveDaemonStateWord = 0;
    GameDataPlayerCountSnapshot = -1;
    open_succeeds = true;
    version_write_result = tick_write_result = 1;
    serializer_result = 0;
    tick_value = 0xFEDCBA98u;
    seed_calls = seed_value = game_data_calls = close_calls = remove_calls = 0;
    opened_path.clear(); removed_path.clear(); calls.clear();
}

static DWORD GetTickCount() { calls += "T"; return tick_value; }
static void my_srand(int value) { ++seed_calls; seed_value = value; calls += "S"; }
static FILE *env_open(const char *path, const char *mode) {
    assert(std::strcmp(mode, "wb") == 0);
    opened_path = path; calls += "O";
    return open_succeeds ? fake_file : 0;
}
static void header_write(const char *header, FILE *) {
    assert(std::strcmp(header, "TERRAN") == 0); calls += "H";
}
static int game_data(FILE *, int writing) {
    assert(writing == 1); ++game_data_calls; calls += "G";
    return serializer_result;
}
static size_t fixture_fwrite(const void *buffer, size_t size, size_t count, FILE *) {
    assert(size == 4 && count == 1);
    int word = 0; std::memcpy(&word, buffer, 4);
    if (word == 0x56) { calls += "V"; return version_write_result; }
    calls += "K";
    assert(static_cast<uint32_t>(word) == tick_value);
    return tick_write_result;
}
static int fixture_fclose(FILE *file) {
    assert(file == fake_file); ++close_calls; calls += "C"; return -1;
}
static int fixture_remove(const char *path) {
    ++remove_calls; removed_path = path; calls += "R"; return -1;
}

#define fwrite fixture_fwrite
#define fclose fixture_fclose
#define remove fixture_remove
// ACTUAL_SAVE_DAEMON_BODY
#undef fwrite
#undef fclose
#undef remove

static void test_filename_and_success_path() {
    reset_fixture();
    NetDaemonNet.player_count_ = 4;
    IsMultiplayerNet = 1;
    for (int i = 0; i < 8; ++i) {
        std::memset(static_cast<void *>(&NetDaemonNet.process_slots_[i]), i + 1,
                    sizeof(AlphaNetProcessSlotHost));
    }
    assert(save_daemon(const_cast<char *>("autosave")) == 0);
    assert(opened_path == "autosave.SAV");
    assert(GameDataPlayerCountSnapshot == 4);
    assert(GameState == 0x10000000u && seed_calls == 0);
    assert(std::memcmp(NetDaemonNet.process_slots_, NetDaemonNet.save_storage_.process_slots_,
                       sizeof NetDaemonNet.process_slots_) == 0);
    assert(calls == "TOHVGKC");
    assert(close_calls == 1 && remove_calls == 0);
}

static void test_open_failure_keeps_prior_side_effects_and_skips_cleanup() {
    reset_fixture(); open_succeeds = false;
    GameRules = RULES_IRONMAN; IsMultiplayerPBEM = 1;
    assert(save_daemon(const_cast<char *>("autosave")) == 1);
    assert(TamperState93A9B4 == 1 && NetTurnFlags == 0x20);
    assert(GameState == 0x10000000u && GameDataPlayerCountSnapshot == 0);
    assert(seed_calls == 1 && seed_value == static_cast<int>(tick_value));
    assert(close_calls == 0 && remove_calls == 0 && calls == "TSO");
}

static void test_any_dot_suppresses_extension_and_postopen_failures_remove() {
    reset_fixture();
    version_write_result = 0;
    assert(save_daemon(const_cast<char *>("dir.with.dot/name")) == 2);
    assert(opened_path == "dir.with.dot/name");
    assert(close_calls == 1 && remove_calls == 1 && removed_path == opened_path);
    assert(calls == "TOHVCR");

    reset_fixture(); serializer_result = 1;
    assert(save_daemon(const_cast<char *>("bad")) == 3);
    assert(game_data_calls == 1 && close_calls == 1 && remove_calls == 1);

    reset_fixture(); tick_write_result = 0;
    assert(save_daemon(const_cast<char *>("bad")) == 3);
    assert(game_data_calls == 1 && close_calls == 1 && remove_calls == 1);
}

int main() {
    test_filename_and_success_path();
    test_open_failure_keeps_prior_side_effects_and_skips_cleanup();
    test_any_dot_suppresses_extension_and_postopen_failures_remove();
}
