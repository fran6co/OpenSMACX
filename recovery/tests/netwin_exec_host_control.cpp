// Portable host doubles for extracted NetWin::exec and row initializer.
// These exercise source control flow only; they do not prove target ABI or
// runtime behavior.
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <string>
#include <vector>

#define __cdecl
#define __stdcall
#define __fastcall
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef char *LPSTR;
typedef const char *LPCSTR;
struct RECT { int left, top, right, bottom; };
static std::vector<std::string> trace;
static int ExitTurnLoop = 0;
static int Language = 0;
static char temp_string[512];
static char *StringTemp = temp_string;
static const char *AlphaxFileID = "alpha";
static bool text_enabled = true;
static int text_reads = 0;
static int text_count = 2;
static int task_runs = 0;
static int modal_answer = 0;
static int *registered_callback_state = nullptr;
static uint32_t *modal_ready_field = nullptr;
int NetWinPlanetSizeLabelCount = 0;
int NetWinExecAdjacentState = -1;
char NetWinWindowTitleInitialText = '\0';

struct NetWinLobbyPacketHeader {
    uint8_t opcode_, flags_;
    uint16_t field_2_;
    int32_t slot_, field_8_;
    uint32_t field_C_;
};
struct NetWinSessionOptionsState {
    uint8_t status_, player_count_, difficulty_, time_control_;
    uint8_t option_4_, option_5_, planet_size_, ocean_coverage_;
    uint8_t erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
struct NetWinSessionOptionsPacket {
    uint16_t opcode_;
    uint8_t reserved_[0x0E];
    NetWinSessionOptionsState options_;
};
static void *observed_packet_rows = nullptr;
static void *observed_row_addresses[7];
static int observed_row_constructions = 0;
static uint8_t observed_row_bytes[7][0x17C];
struct NetWinLobbyRow {
    uint8_t prefix_, ready_or_occupied_, field_2_;
    int8_t faction_or_color_;
    uint8_t field_4_;
    char other_[0x173];
    uint32_t owner_;
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "row stride");
static_assert(std::is_trivial<NetWinLobbyRow>::value, "passive wire storage");
static NetWinLobbyRow NetWinLobbyRows[8];
struct NetWinConstructedLobbyRow {
    NetWinLobbyRow payload_;
    NetWinConstructedLobbyRow() {
        // Host-only canaries make untouched bytes observable; product code
        // does not initialize these bytes.
        std::memset(&payload_, 0xA5, sizeof(payload_));
        /* ROW_CTOR_BODY_INSERTION */
        if (observed_row_constructions < 7) {
            if (observed_row_constructions == 0) observed_packet_rows = &payload_;
            observed_row_addresses[observed_row_constructions] = &payload_;
            std::memcpy(observed_row_bytes[observed_row_constructions],
                        &payload_, sizeof(payload_));
        }
        ++observed_row_constructions;
        trace.push_back("row-constructor");
    }
};
static_assert(sizeof(NetWinConstructedLobbyRow) == 0x17C, "constructed row stride");
static_assert(std::is_trivially_destructible<NetWinConstructedLobbyRow>::value,
              "original non-EH array has no destructor cleanup");

struct AlphaIniPref { uint32_t preferences, more_preferences, announce, rules, semaphore; };
struct DefaultPref { uint32_t difficulty; };
static AlphaIniPref AlphaIniPrefs = {0, 0, 0, 0x20, 0x1234};
static DefaultPref DefaultPrefs = {3};

struct Buffer {
    RECT rect1_, rect2_;
    void set_text_color(int, int, int, int) { trace.push_back("text-color"); }
    void set_font(void *, int, int, int) { trace.push_back("font"); }
};
struct Win {
    int field_16C_ = 0;
    void center() { trace.push_back("center"); }
    void hide() { trace.push_back("hide"); }
    void show(int) { trace.push_back("show"); }
    void redraw() { trace.push_back("redraw"); }
    void on_redraw() { trace.push_back("on-redraw"); }
    void on_update() { trace.push_back("update"); }
    int set_modal(int, int (__cdecl *callback)(), Win *) {
        assert(registered_callback_state && *registered_callback_state == 1);
        trace.push_back("modal");
        int result = callback();
        if (result > 0 && modal_ready_field) *modal_ready_field = 1;
        return result;
    }
};
struct GraphicWin : Win, Buffer {
    int init(int, int, int, int, LPSTR, int, Win *, void *, void *) {
        trace.push_back("graphic-init"); return 1;
    }
};
struct StringBox : GraphicWin {
    int callback_installed = 0;
    void set_write_strings_callback(void (__cdecl *)(int, int)) {
        callback_installed = 1; trace.push_back("string-callback");
    }
    void init(int, int, int, int, Win *, void *, int) {
        callback_installed = 0; trace.push_back("string-init");
    }
    void set_visible_rows(int) { trace.push_back("visible-rows"); }
    void close() { callback_installed = 0; trace.push_back("string-close"); }
};
struct EditBox : GraphicWin {
    uint32_t field_B14_ = 0, field_B1C_ = 0, field_B20_ = 0;
    uint32_t field_B24_ = 0, field_B30_ = 0, flags_ = 0;
    void add_window_flags(uint32_t bits) { flags_ |= bits; trace.push_back("edit-flags"); }
    int init(int, int, int, int, Win *, int) { trace.push_back("edit-init"); return 1; }
    void close() { trace.push_back("edit-close"); }
};
struct CheckBox { void close() { trace.push_back("checkbox-close"); } };
struct FlatButton : GraphicWin {
    void init(char *, int, int, int, int, int, Win *, int) { trace.push_back("button-init"); }
    void set_text_color(int, int, int, int) { trace.push_back("button-color"); }
    void close() { trace.push_back("button-close"); }
};
struct Spot {
    void init(int) { trace.push_back("spot-init"); }
    void shutdown() { trace.push_back("spot-shutdown"); }
};
struct NetWin : GraphicWin {
    Spot spot_;
    StringBox stringBox_;
    EditBox editBox_;
    CheckBox checkBox_;
    FlatButton flatButton1_, flatButton2_;
    char world_size_labels_[32][0x80];
    uint32_t field_7614_ = 0, field_761C_ = 0, field_7620_ = 0;
    void *faction_records_ = nullptr, *faction_records_copy_ = nullptr;
    uint32_t field_1758_ = 0;
    RECT setup_rects_[50];
    void setup_faction_data(void *) { trace.push_back("setup-factions"); }
    static int __cdecl modal_callback_daemon() { trace.push_back("modal-callback"); return modal_answer; }
    static void __cdecl string_callback(int, int) {}
    void prepare_game() { trace.push_back("prepare-game"); }
    int exec();
};
static NetWin NetWinDialog;
struct LockState { uint32_t leading_words_[4]; struct { uint32_t flags; } payload_; };
struct NetDaemon {
    LockState lock_state_[1];
    int initial_sends = 0, option_sends = 0;
    uint16_t last_options_opcode = 0;
    int last_options_size = 0;
    NetWinSessionOptionsState last_options{};
    NetDaemon() { std::memset(lock_state_, 0, sizeof(lock_state_)); }
    void enable_join() { trace.push_back("enable-join"); }
    void send(void *data, int size, unsigned long, int) {
        if (size == 0xA74) {
            const uint8_t *bytes = static_cast<const uint8_t *>(data);
            assert(bytes[0] == 0x04 && bytes[1] == 0x4F);
            assert(*reinterpret_cast<const int32_t *>(bytes + 4) == -1);
            assert(*reinterpret_cast<const int32_t *>(bytes + 8) == 0);
            // The pinned image does not initialize header +2..3 or +C..F.
            assert(observed_packet_rows == bytes + 0x10);
            assert(std::memcmp(bytes + 0x10, &NetWinLobbyRows[1], 0xA64) == 0);
            ++initial_sends; trace.push_back("send-lobby");
        } else if (size == 0x28) {
            const uint8_t *bytes = static_cast<const uint8_t *>(data);
            last_options_size = size;
            std::memcpy(&last_options_opcode, bytes, sizeof(last_options_opcode));
            std::memcpy(&last_options, bytes + 0x10, sizeof(last_options));
            assert(last_options_opcode == 0x2F02);
            ++option_sends; trace.push_back("send-options");
        } else { assert(false); }
    }
    void pid_2_idx(int) { trace.push_back("pid-to-index"); }
    void set_polling(int) { trace.push_back("polling"); }
    void close_game() { trace.push_back("close-game"); }
};
static NetDaemon NetDaemonNet;
static int NetWinPlayerId = 10, NetWinOtherPlayerId = 20;
static NetWinSessionOptionsState NetWinSessionOptions = {};
static void do_task() {
    ++task_runs; trace.push_back("task");
    if (!NetWinDialog.faction_records_) NetWinDialog.faction_records_ = std::malloc(1);
}
static int text_open(const char *, const char *) { return text_enabled ? 0 : 1; }
static void text_get() { ++text_reads; }
static int text_item_number() { return text_count; }
static char *text_item() { return const_cast<char *>("Tiny|world"); }
static void text_close() { trace.push_back("text-close"); }
static char *label_get(int) { return const_cast<char *>("Localized"); }
static void *find_font(int, int) { return nullptr; }
static char *CharUpperA(char *value) {
    for (char *p = value; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
    return value;
}
static int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

/* EXEC_BODY_INSERTION */

static int index_of(const char *name) {
    for (size_t i = 0; i < trace.size(); ++i) if (trace[i] == name) return static_cast<int>(i);
    return -1;
}
static void reset_fixture() {
    trace.clear(); task_runs = 0; text_reads = 0;
    observed_packet_rows = nullptr;
    observed_row_constructions = 0;
    std::memset(observed_row_addresses, 0, sizeof(observed_row_addresses));
    std::memset(observed_row_bytes, 0, sizeof(observed_row_bytes));
    NetWinPlanetSizeLabelCount = 0; NetWinExecAdjacentState = -1;
    NetDaemonNet.lock_state_[0].leading_words_[1] = 0;
    NetDaemonNet.lock_state_[0].payload_.flags = 0;
    NetDaemonNet.initial_sends = NetDaemonNet.option_sends = 0;
    NetDaemonNet.last_options_opcode = 0;
    NetDaemonNet.last_options_size = 0;
    NetDaemonNet.last_options = {};
    NetWinDialog.field_7614_ = 0;
    std::free(NetWinDialog.faction_records_); NetWinDialog.faction_records_ = nullptr;
    std::free(NetWinDialog.faction_records_copy_); NetWinDialog.faction_records_copy_ = nullptr;
    NetWinDialog.world_size_labels_[0][0] = 0;
    modal_answer = 0; modal_ready_field = &NetWinDialog.field_7614_;
    registered_callback_state = &NetWinDialog.stringBox_.callback_installed;
}
static void test_row_initializer() {
    NetWinConstructedLobbyRow row;
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&row.payload_);
    assert(bytes[0] == 0xFF && bytes[1] == 0 && bytes[2] == 0);
    assert(bytes[3] == 0xFF && bytes[4] == 2);
    for (size_t i = 5; i < 0x178; ++i) assert(bytes[i] == 0xA5);
    for (size_t i = 0x178; i < 0x17C; ++i) assert(bytes[i] == 0);
}
static void test_exec() {
    reset_fixture(); ExitTurnLoop = 1;
    assert(NetWinDialog.exec() == 0 && trace.empty());

    reset_fixture(); ExitTurnLoop = 0;
    NetDaemonNet.lock_state_[0].leading_words_[1] = 1;
    assert(NetWinDialog.exec() == 0);
    assert(task_runs == 1 && NetDaemonNet.initial_sends == 1 && NetDaemonNet.option_sends == 1);
    assert(NetDaemonNet.last_options_opcode == 0x2F02);
    assert(NetDaemonNet.last_options_size == 0x28);
    assert(std::memcmp(&NetDaemonNet.last_options, &NetWinSessionOptions,
                       sizeof(NetWinSessionOptions)) == 0);
    assert(NetDaemonNet.lock_state_[0].payload_.flags == 2);
    assert(NetWinPlanetSizeLabelCount == 2);
    assert(std::strcmp(NetWinDialog.world_size_labels_[0], "Tiny world") == 0);
    assert(NetWinDialog.field_761C_ == 0xFFFFFFFFU && NetWinDialog.field_7620_ == 0xFFFFFFFFU);
    assert(index_of("close-game") < index_of("spot-shutdown"));
    assert(index_of("spot-shutdown") < index_of("string-close"));
    assert(index_of("string-close") < index_of("edit-close"));
    assert(index_of("edit-close") < index_of("checkbox-close"));
    assert(index_of("checkbox-close") < index_of("button-close"));
    assert(NetWinDialog.stringBox_.callback_installed == 0);
    assert(index_of("string-init") < index_of("string-callback"));
    assert(index_of("string-callback") < index_of("modal"));

    reset_fixture(); modal_answer = 1;
    assert(NetWinDialog.exec() == 1);
    assert(index_of("prepare-game") > index_of("close-game"));
    assert(index_of("prepare-game") < index_of("spot-shutdown"));

    reset_fixture(); text_enabled = false;
    assert(NetWinDialog.exec() == 0 && NetWinPlanetSizeLabelCount == 0);
    assert(index_of("text-close") == -1);
    text_enabled = true;
}
static void test_exec_packet_rows_are_payload() {
    reset_fixture();
    std::memset(NetWinLobbyRows, 0x5A, sizeof(NetWinLobbyRows));
    NetDaemonNet.lock_state_[0].leading_words_[1] = 0;
    assert(NetWinDialog.exec() == 0);
    assert(observed_row_constructions == 7);
    assert(index_of("enable-join") < index_of("row-constructor"));
    assert(observed_packet_rows == observed_row_addresses[0]);
    const uintptr_t base = reinterpret_cast<uintptr_t>(observed_packet_rows);
    for (int row = 0; row < 7; ++row) {
        assert(reinterpret_cast<uintptr_t>(observed_row_addresses[row]) ==
               base + row * sizeof(NetWinLobbyRow));
        const uint8_t *bytes = observed_row_bytes[row];
        assert(bytes[0] == 0xFF && bytes[1] == 0 && bytes[2] == 0);
        assert(bytes[3] == 0xFF && bytes[4] == 2);
        for (size_t offset = 5; offset < 0x178; ++offset) assert(bytes[offset] == 0xA5);
        for (size_t offset = 0x178; offset < 0x17C; ++offset) assert(bytes[offset] == 0);
    }
    assert(NetDaemonNet.initial_sends == 0);
    assert(NetDaemonNet.option_sends == 0);
}
int main() {
    const uint8_t *global = reinterpret_cast<const uint8_t *>(NetWinLobbyRows);
    for (size_t i = 0; i < sizeof(NetWinLobbyRows); ++i) assert(global[i] == 0);
    assert(observed_row_constructions == 0);
    test_row_initializer(); test_exec(); test_exec_packet_rows_are_payload();
}
