#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define __cdecl
typedef uint32_t DWORD;
typedef unsigned char BYTE;
typedef BYTE *LPBYTE;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef void *HKEY;
typedef long LONG;
typedef DWORD REGSAM;
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002u)
#define ERROR_SUCCESS 0L
#define REG_BINARY 3u
#define MaxPlayerNum 8

/* ACTUAL_TAMPER_RECORD */
struct ConsoleGlobalMock { int faction_id_; };
static ConsoleGlobalMock ConsoleGlobal;
static int TamperState93A9AC;
static int TamperState93A9B0;
static int TamperState93A9B4;
static int TamperDialogFaction[8];
static int TamperDialogValue[8];
static TamperRecord TamperRegistryRecords[50];
static int TamperRegistryCursor;
struct GameDataStateWindow { struct Fields { union LockMasks { struct Bytes { uint8_t human_mask_, alive_mask_, unassigned_[2]; } bytes_; uint32_t enable_mask_; } lock_masks_; int turn_current_num_; } fields_; };
static GameDataStateWindow GameDataStateStorage;
static int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
static int IsMultiplayerPBEM;

static char TamperReadApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
static char TamperWriteApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
static char TamperReadValueMVM[] = "MVM";
static char TamperReadValueMVB[] = "MVB";
static char TamperWriteValueMVM[] = "MVM";
static char TamperWriteValueMVB[] = "MVB";
static char TamperDialogLabel[] = "HOTNOTAMPER";
static char TamperReadClass[1];
static char TamperWriteClass[1];

static int create_count;
static LONG create_results[4];
static int query_count;
static bool query_write_cursor;
static bool query_write_records;
static int set_count;
static int close_count;
static int popup_count;
static int query_status;
static char set_names[2][4];
static DWORD set_types[2];
static DWORD set_sizes[2];
static BYTE set_snapshots[2][0x4B0];

static void reset_control() {
    create_count = query_count = set_count = close_count = popup_count = 0;
    query_write_cursor = query_write_records = false;
    query_status = 5;
    memset(create_results, 0, sizeof(create_results));
    memset(set_names, 0, sizeof(set_names));
    memset(set_types, 0, sizeof(set_types));
    memset(set_sizes, 0, sizeof(set_sizes));
    memset(set_snapshots, 0, sizeof(set_snapshots));
}

static LONG RegCreateKeyExA(HKEY root, LPCSTR subkey, DWORD reserved,
                            LPSTR class_name, DWORD options, REGSAM access,
                            void *security, HKEY *out, DWORD *disposition) {
    assert(root == HKEY_LOCAL_MACHINE);
    assert(reserved == 0 && options == 0 && access == 0xF003F);
    assert(security == 0 && disposition == 0);
    if (create_count == 0) {
        assert(strcmp(subkey, TamperReadApplicationKey) == 0);
        assert(class_name == TamperReadClass);
    } else {
        assert(strcmp(subkey, TamperWriteApplicationKey) == 0);
        assert(class_name == TamperWriteClass);
    }
    LONG result = create_results[create_count];
    *out = (HKEY)(uintptr_t)(create_count + 1);
    ++create_count;
    return result;
}
static LONG RegQueryValueExA(HKEY key, LPCSTR name, DWORD *reserved,
                             DWORD *type, LPBYTE data, DWORD *byte_count) {
    assert((uintptr_t)key == 1 && reserved == 0 && type != 0);
    if (query_count == 0) {
        assert(strcmp(name, "MVM") == 0 && data == (LPBYTE)&TamperRegistryCursor);
        assert(*byte_count == 4);
        if (query_write_cursor) memcpy(data, &query_status, 4);
    } else {
        assert(strcmp(name, "MVB") == 0 && data == (LPBYTE)TamperRegistryRecords);
        assert(*byte_count == 0x4B0);
        if (query_write_records) {
            memset(data, 0xA5, 0x4B0);
        }
    }
    ++query_count;
    return 5; // The native caller ignores both query return codes.
}
static LONG RegSetValueExA(HKEY key, LPCSTR name, DWORD reserved,
                           DWORD type, const BYTE *data, DWORD size) {
    assert((uintptr_t)key == 2 && reserved == 0 && type == REG_BINARY);
    assert(set_count < 2);
    strncpy(set_names[set_count], name, 3);
    set_types[set_count] = type;
    set_sizes[set_count] = size;
    memcpy(set_snapshots[set_count], data, size);
    ++set_count;
    return 0;
}
static LONG RegCloseKey(HKEY key) {
    assert((uintptr_t)key == (close_count == 0 ? 1u : 2u));
    ++close_count;
    return 0;
}
static int X_pop(const char *label, int (__cdecl *callback)()) {
    assert(strcmp(label, "HOTNOTAMPER") == 0 && callback == 0);
    ++popup_count;
    return 0;
}

/*CHECK_TAMPER_BODY*/

static void reset_state() {
    memset(TamperRegistryRecords, 0, sizeof(TamperRegistryRecords));
    memset(TamperDialogFaction, 0, sizeof(TamperDialogFaction));
    memset(TamperDialogValue, 0, sizeof(TamperDialogValue));
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    TamperState93A9AC = 7;
    TamperState93A9B0 = 9;
    TamperState93A9B4 = 11;
    ConsoleGlobal.faction_id_ = 2;
    TurnCurrentNum = 400;
    IsMultiplayerPBEM = 1;
    TamperRegistryCursor = 49;
    reset_control();
}

static void test_gates() {
    reset_state();
    IsMultiplayerPBEM = 0;
    check_tamper();
    assert(create_count == 0 && query_count == 0 && popup_count == 0);
    reset_state();
    TamperState93A9AC = 0;
    check_tamper();
    assert(create_count == 0 && query_count == 0 && popup_count == 0);
}

static void test_match_and_masked_notification() {
    reset_state();
    TamperRecord &record = TamperRegistryRecords[3];
    record.field_0_ = 7; record.field_4_ = 9; record.field_8_ = 400;
    record.field_C_ = 2; record.count_ = 5; record.field_14_ = 11;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = (1u << 1) | (1u << 3) | (1u << 5) | (1u << 6);
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = (1u << 3) | (1u << 5) | (1u << 6);
    query_write_cursor = false; // failed reads leave shared storage untouched
    check_tamper();
    assert(record.count_ == 6);
    assert(TamperDialogFaction[3] == 2 && TamperDialogValue[3] == 6);
    assert(TamperDialogFaction[5] == 2 && TamperDialogValue[5] == 6);
    assert(TamperDialogFaction[1] == 0);
    assert(TamperDialogFaction[6] == 2 && TamperDialogValue[6] == 6);
    assert(query_count == 2 && create_count == 2 && close_count == 2);
    assert(set_count == 2 && popup_count == 1);
    assert(strcmp(set_names[0], "MVM") == 0 && set_sizes[0] == 4);
    assert(strcmp(set_names[1], "MVB") == 0 && set_sizes[1] == 0x4B0);
    assert(set_types[0] == 3 && set_types[1] == 3);
    assert(memcmp(set_snapshots[0], &TamperRegistryCursor, 4) == 0);
    assert(memcmp(set_snapshots[1], TamperRegistryRecords, 0x4B0) == 0);
}

static void test_new_record_wrap_and_ignored_query_failures() {
    reset_state();
    query_write_cursor = false;
    query_write_records = false;
    check_tamper();
    TamperRecord &record = TamperRegistryRecords[49];
    assert(record.field_0_ == 7 && record.field_4_ == 9);
    assert(record.field_8_ == 400 && record.field_C_ == 2);
    assert(record.count_ == 0 && record.field_14_ == 11);
    assert(TamperRegistryCursor == 0);
    assert(query_count == 2 && create_count == 2 && close_count == 2);
    assert(set_count == 2 && popup_count == 0);
    assert(memcmp(set_snapshots[0], &TamperRegistryCursor, 4) == 0);
    assert(memcmp(set_snapshots[1], TamperRegistryRecords, 0x4B0) == 0);
}

static void test_failed_registry_create_still_updates_table() {
    reset_state();
    create_results[0] = 5;
    create_results[1] = 5;
    check_tamper();
    assert(query_count == 0 && close_count == 0 && set_count == 0);
    assert(TamperRegistryRecords[49].field_0_ == 7);
    assert(TamperRegistryCursor == 0 && popup_count == 0);
}

int main() {
    test_gates();
    test_match_and_masked_notification();
    test_new_record_wrap_and_ignored_query_failures();
    test_failed_registry_create_still_updates_table();
    puts("check_tamper actual-body host controls: PASS");
    return 0;
}
