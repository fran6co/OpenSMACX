#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <new>
#include <vector>
#include <string>

typedef int BOOL;
typedef uint32_t DWORD;
typedef void *HANDLE;
typedef const char *LPCSTR;
typedef void *LPVOID;
#define TRUE 1
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define VK_ESCAPE 0x1B
#define static_assert(condition, message) \
    typedef char static_assertion_##__LINE__[(condition) ? 1 : -1]

struct AlphaNetProcessSlot {
    uint32_t pid_;
    unsigned char status_bytes_[4];
};
struct LockStateEntry { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; };
struct DeletionRecord { uint8_t acknowledgements_; uint8_t reserved_[3]; int owner_; int16_t count_; int16_t vehicle_ids_[25]; };
struct DeletionList { DeletionRecord records_[24]; int current_owner_; uint32_t sequence_bits() const { uint32_t x; memcpy(&x, &current_owner_, 4); return x; } void set_sequence_bits(uint32_t x) { memcpy(&current_owner_, &x, 4); } uint32_t advance_sequence() { uint32_t x=sequence_bits()+1; set_sequence_bits(x); return x; } };

struct PacketCopy {
    unsigned short type;
    int size;
    int target;
    std::vector<unsigned char> bytes;
};
static std::vector<PacketCopy> g_messages;
static std::vector<PacketCopy> g_group_packets;
static std::vector<int> g_group_members;
static int g_scenario;
static int g_escape_after;
static int g_draw_count;
static int g_process_count;
static int g_find_index;
static int g_find_count;
static int g_free_count;
static int g_findclose_count;
static int g_setcwd_count;
static int g_file_open_count;
static int g_chunk_draws;
static size_t g_chunk_start;
static int g_transfer_ui;
static unsigned int g_test_file_size = 1500;
static unsigned int g_compressed_size = 2050;
static unsigned int g_ack_value;
static char g_cwd[260] = "C:\\Old";
static char g_saved_cwd[260];
static char g_alt_path[260];
static int g_next_handle = 1;
class NetDaemon;
static NetDaemon *g_active_daemon;
static std::vector<const void *> g_method_owners;
static std::vector<int> g_reported_counts;
static std::vector<std::string> g_open_names;
static std::vector<void *> g_retained_tables;
static bool g_rebind_tables;
static bool g_fail_open;
static bool g_observe_invalid_tail;
static uintptr_t g_observed_base;
static uint32_t g_observed_capacity;
static size_t g_observed_copy_count;
static uint32_t g_observed_send_count;
static int g_observed_copies;
static int g_observed_sends;
static bool g_observed_memmove;
static const char *g_popup_file;
static int g_setup_receiver_status;
static int g_setup_local_status;
static int g_setup_global_status;
static bool g_observe_lookup;
static uint32_t g_lookup_abort;
static void producer_callback(int stage);
extern uint32_t NetDaemonFileTransferAckMask;
static void do_draw();

struct FakeFindData {
    DWORD dwFileAttributes;
    DWORD nFileSizeLow;
    char cFileName[260];
};
typedef FakeFindData WIN32_FIND_DATAA;

class Filemap {
public:
    Filemap() : opened_(false) {}
    ~Filemap() {}
    LPVOID open_read(LPCSTR name, BOOL) {
        g_open_names.push_back(name); ++g_file_open_count;
        opened_ = !g_fail_open;
        return opened_ ? reinterpret_cast<LPVOID>(1) : 0;
    }
    HANDLE get_handle() { return reinterpret_cast<HANDLE>(1); }
    LPVOID get_view() { static unsigned char bytes[4096]; return bytes; }
    LPVOID create(LPCSTR, uint32_t, BOOL) { static unsigned char bytes[4096]; return bytes; }
    void close() { opened_ = false; }
private:
    bool opened_;
};

class Net {
public:
    Net() : field_760_(0x111), field_764_(0x222), player_count_(1),
            get_calls_(0), sequence_(0) {
        memset(process_slots_, 0, sizeof(process_slots_));
        memset(&primary_state_, 0, sizeof(primary_state_));
        memset(channels_, 0, sizeof(channels_));
        memset(&deletion_list_, 0, sizeof(deletion_list_));
        process_slots_[1].pid_ = 0x222;
    }
    int pid_2_idx(unsigned long id) {
        const int result = id == field_760_ ? 1 : 2;
        producer_callback(4);
        return result;
    }
    void *get(unsigned long *sender, unsigned long *length) {
        ++get_calls_;
        if (get_calls_ > 100) { fprintf(stderr, "get pump runaway\\n"); ::abort(); }
        *sender = 0x222;
        *length = 0x20;
        if (g_scenario == 4) NetDaemonFileTransferAckMask = g_ack_value;
        return 0;
    }
    int send(void *data, int size, unsigned long who, int mode) {
        g_method_owners.push_back(this);
        PacketCopy packet;
        packet.type = *reinterpret_cast<unsigned short *>(data);
        packet.size = size;
        if (mode != 1) abort();
        packet.target = static_cast<int>(who);
        packet.bytes.resize(size);
        memcpy(&packet.bytes[0], data, size);
        g_messages.push_back(packet); return 1;
    }
    int create_group(char *) { g_method_owners.push_back(this); return 7; }
    void delete_group(int) { g_method_owners.push_back(this); }
    int add_to_group(long, unsigned long member) {
        g_method_owners.push_back(this);
        g_group_members.push_back(static_cast<int>(member)); return 0;
    }
    int send_group(void *data, int size, long, int) {
        g_method_owners.push_back(this);
        if (g_observe_invalid_tail) {
            ++g_observed_sends;
            g_observed_send_count = static_cast<uint32_t>(size);
            return 1;
        }
        PacketCopy packet;
        packet.type = *reinterpret_cast<unsigned short *>(data);
        packet.size = size; packet.target = 0;
        packet.bytes.resize(size);
        memcpy(&packet.bytes[0], data, size);
        g_group_packets.push_back(packet); return 1;
    }
    DeletionList deletion_list_;
    PrimaryState primary_state_;
    LockStateEntry channels_[2];
    struct NetDaemonFileIndexRecord *file_index_records_;
    uint32_t field_1BE4_;
    AlphaNetProcessSlot process_slots_[8];
    unsigned long field_760_;
    unsigned long field_764_;
    int player_count_;
    int get_calls_;
    int sequence_;
};

class NetDaemon : public Net {
public:
    // Host-only view of the source-owned transfer pointer, not an ABI witness.
    void process_message(char *, unsigned long, int) { ++g_process_count; }
    void send_message(char *bytes, unsigned long size, int target);
    int send_files();
    void do_draw();
    void do_all_draws() {}
    void pop_close() {}
};

static NetDaemon NetDaemonNet;
static char SaveNameBuffer[0x104] = "C:\\Game";
char NetDaemonFileTransferPath[0x104] = "assets";
static char *StringTemp;
static char g_script_file[] = "SCRIPT";
static char g_rebound_script_file[] = "CALLBACK_SCRIPT";
static char *ScriptPopupFile = g_script_file;
static int PopupModalState;
static int NetDaemonFileTransferMode;
uint32_t NetDaemonFileTransferAckMask;
static int NetDaemonFileTransferCurrentFile;
static int NetDaemonFileTransferChunkCount;
static int NetDaemonFileTransferProgress;
static unsigned char *NetDaemonFileTransferReceiveBuffer;

static void *mem_get(size_t size) {
    if (g_observe_invalid_tail && size != 0x11C00 && size != 0x820)
        return reinterpret_cast<void *>(g_observed_base);
    return calloc(1, size == 0 ? 1 : size);
}
static int checksum_file(const char *) { producer_callback(1); return 0x5A; }
static void filefind_set_alternative(const char *path) {
    strncpy(g_alt_path, path, sizeof(g_alt_path) - 1);
}
static void parse_says(int, char *, int, int) { producer_callback(3); }
static void parse_num(int, int value) { g_reported_counts.push_back(value); }
static void x_pop_caption_flags(char *file, char *, int, int) {
    g_popup_file = file;
    g_chunk_start = g_group_packets.size();
    g_transfer_ui = 1;
}
static void do_draw();
static void do_all_draws() {}
static void pop_close() {}

int netdaemon_deflate_buffer(unsigned char *out, unsigned long *out_size,
               unsigned char *, unsigned int) {
    if (g_observe_invalid_tail) { *out_size = g_observed_capacity; return -5; }
    if (*out_size < g_compressed_size) return -5;
    memset(out, 0xA5, g_compressed_size);
    *out_size = g_compressed_size;
    return 0;
}
static void test_free(void *p) {
    if (p != 0) ++g_free_count;
    if (g_observe_invalid_tail && reinterpret_cast<uintptr_t>(p) == g_observed_base)
        return;
    ::free(p);
}
static void *test_memmove(void *destination, const void *source, size_t count) {
    if (g_observe_invalid_tail && reinterpret_cast<uintptr_t>(source) == g_observed_base) {
        ++g_observed_copies; g_observed_copy_count = count;
        g_observed_memmove = true;
        return destination;
    }
    return ::memmove(destination, source, count);
}
static void *test_memcpy(void *destination, const void *source, size_t count) {
    if (g_observe_invalid_tail && reinterpret_cast<uintptr_t>(source) == g_observed_base) {
        ++g_observed_copies; g_observed_copy_count = count;
        g_observed_memmove = false;
        return destination;
    }
    return ::memcpy(destination, source, count);
}

static DWORD GetModuleFileNameA(void *, char *path, DWORD) {
    strcpy(path, "C:\\Game\\app.exe"); return 17;
}
static DWORD GetCurrentDirectoryA(DWORD, char *path) {
    strcpy(path, g_cwd); strcpy(g_saved_cwd, g_cwd); return strlen(path);
}
static BOOL SetCurrentDirectoryA(const char *path) {
    ++g_setcwd_count; strcpy(g_cwd, path); return 1;
}
static int _mkdir(const char *) { return -1; }
static short GetAsyncKeyState(int) {
    if (g_escape_after >= 0 && g_draw_count >= g_escape_after) return static_cast<short>(0x8000);
    return 0;
}
static HANDLE FindFirstFileA(const char *, WIN32_FIND_DATAA *data) {
    g_find_index = 0;
    data->dwFileAttributes = 0;
    data->nFileSizeLow = g_test_file_size;
    strcpy(data->cFileName, "sample.dat");
    return reinterpret_cast<HANDLE>(g_next_handle++);
}
static BOOL FindNextFileA(HANDLE, WIN32_FIND_DATAA *data) {
    if (++g_find_index >= g_find_count) return 0;
    producer_callback(2);
    data->dwFileAttributes = 0;
    data->nFileSizeLow = g_test_file_size;
    strcpy(data->cFileName, "second.dat");
    return 1;
}
static BOOL FindClose(HANDLE) { ++g_findclose_count; return 1; }
static DWORD GetFileSize(HANDLE, DWORD *) { return g_test_file_size; }

static void do_draw() {
    ++g_draw_count;
    if (g_draw_count > 100) { fprintf(stderr, "draw pump runaway\\n"); ::abort(); }
    if (g_transfer_ui && g_group_packets.size() == g_chunk_start)
        ++g_chunk_draws;
    g_active_daemon->process_slots_[1].status_bytes_[0] =
        static_cast<unsigned char>(g_draw_count < 4 ? 1 : 2);
    if (NetDaemonNet.player_count_ > 1) {
        for (int slot = 2; slot <= NetDaemonNet.player_count_; ++slot)
            g_active_daemon->process_slots_[slot].status_bytes_[0] = 2;
    }
    if (g_scenario == 3 && g_draw_count == 1) NetDaemonFileTransferMode = 3;
    if (g_scenario == 5 && g_draw_count == 1)
        NetDaemonNet.channels_[1].trailing_words_[9] = 1;
}
void NetDaemon::do_draw() { ::do_draw(); }

#define free test_free
#define memmove test_memmove
#define memcpy test_memcpy
@@OWNER_SOURCE@@
#undef memcpy
#undef memmove
#undef free

static void producer_callback(int stage) {
    if (stage == 4 && g_observe_lookup) {
        g_lookup_abort = NetDaemonNet.channels_[1].trailing_words_[9];
        NetDaemonNet.channels_[1].trailing_words_[9] = 1;
    }
    if (stage == 4) return;
    if (!g_rebind_tables) return;
    NetDaemonFileIndexRecord *previous = netdaemon_file_index_records();
    NetDaemonFileIndexRecord *replacement = netdaemon_allocate_file_index_records();
    memcpy(replacement, previous, 0x11C00);
    g_retained_tables.push_back(previous);
    netdaemon_set_file_index_records(replacement);
    if (stage == 3) {
        // Keep the old objects alive: this proves reload/capture selection,
        // not whether real callbacks destroy storage or leave dangling rows.
        replacement[0].ready_flags = 1U << 2;
        replacement[1].ready_flags = 1U << 3;
        previous[0].ready_flags = 1U << 4;
        previous[1].ready_flags = 1U << 4;
        strcpy(replacement[0].filename, "rebound-first.dat");
        strcpy(replacement[1].filename, "rebound-second.dat");
        ScriptPopupFile = g_rebound_script_file;
    }
}

void NetDaemon::send_message(char *bytes, unsigned long size, int target) {
    g_method_owners.push_back(this);
    if (*reinterpret_cast<unsigned short *>(bytes) == 0xF05) {
        // F05 precedes every draw/status pump. Preserve the native initial
        // equal-local =>2, unequal-peer =>0 distinction here.
        g_setup_local_status = process_slots_[1].status_bytes_[0];
        g_setup_receiver_status = process_slots_[2].status_bytes_[0];
        g_setup_global_status = NetDaemonNet.process_slots_[2].status_bytes_[0];
    }
    PacketCopy packet;
    packet.type = *reinterpret_cast<unsigned short *>(bytes);
    packet.size = static_cast<int>(size); packet.target = target;
    packet.bytes.resize(size);
    memcpy(&packet.bytes[0], bytes, size);
    g_messages.push_back(packet);
    if (packet.type == 0xF06) {
        // Native callback helper owns this bit; simulate the peer's F08.
        const int index = bytes[0x10];
        NetDaemonFileIndexRecord *records = netdaemon_file_index_records();
        records[index].ready_flags = 1U << 1;
    }
}

static void test_fail(int line) {
    fprintf(stderr, "send_files assertion failed at line %d\n", line);
    ::abort();
}
#define abort() test_fail(__LINE__)

static void reset_fixture() {
    g_find_count = 1;
    g_messages.clear(); g_group_packets.clear(); g_group_members.clear();
    g_scenario = 0; g_escape_after = -1; g_draw_count = 0; g_process_count = 0;
    g_find_index = 0; g_free_count = 0; g_findclose_count = 0;
    g_setcwd_count = 0; g_file_open_count = 0; g_next_handle = 1;
    g_chunk_draws = 0; g_chunk_start = 0; g_transfer_ui = 0;
    g_active_daemon = &NetDaemonNet;
    g_method_owners.clear(); g_reported_counts.clear(); g_open_names.clear();
    g_rebind_tables = false; g_fail_open = false; g_observe_invalid_tail = false;
    g_observed_copies = g_observed_sends = 0;
    g_observed_memmove = false;
    g_observed_copy_count = 0; g_observed_send_count = 0; g_popup_file = 0;
    ScriptPopupFile = g_script_file;
    g_setup_receiver_status = g_setup_global_status = -1;
    g_setup_local_status = -1;
    g_observe_lookup = false; g_lookup_abort = 0xFFFFFFFF;
    g_test_file_size = 4000; g_compressed_size = 2050;
    NetDaemonNet.get_calls_ = 0; g_ack_value = 0;
    strcpy(g_cwd, "C:\\Old"); g_saved_cwd[0] = 0; g_alt_path[0] = 0;
    NetDaemonNet.player_count_ = 1;
    NetDaemonNet.process_slots_[1].status_bytes_[0] = 0;
    NetDaemonNet.process_slots_[1].pid_ = 0x222;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetDaemonNet.channels_[1].trailing_words_[9] = 0;
    NetDaemonNet.file_index_records_ = 0;
    NetDaemonFileTransferMode = 1;
    NetDaemonFileTransferAckMask = 0;
    NetDaemonFileTransferPath[0] = 'a'; NetDaemonFileTransferPath[1] = 0;
    strcpy(SaveNameBuffer, "C:\\Game");
}

static PacketCopy find_message(unsigned short type) {
    for (size_t i = 0; i < g_messages.size(); ++i)
        if (g_messages[i].type == type) return g_messages[i];
    PacketCopy empty; empty.type = 0; empty.size = 0; empty.target = -1;
    return empty;
}

static void test_host_send_chunking_and_order() {
    reset_fixture();
    strcpy(SaveNameBuffer, "C:\\Game\\assets");
    g_test_file_size = 4000;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    const int result = NetDaemonNet.send_files();
    if (result != 0 || g_file_open_count != 1 || g_findclose_count != 1 ||
        g_setup_local_status != 2) abort();
    if (g_group_members.size() != 1 || g_group_members[0] != 0x222) abort();
    if (g_group_packets.size() != 2) abort();
    if (g_group_packets[0].type != 0xF0B || g_group_packets[0].size != 0x820 ||
        g_group_packets[1].size != 0x22) abort();
    if (*reinterpret_cast<const uint32_t *>(&g_group_packets[0].bytes[8]) != 0 ||
        *reinterpret_cast<const uint32_t *>(&g_group_packets[0].bytes[12]) != 0 ||
        *reinterpret_cast<const uint32_t *>(&g_group_packets[1].bytes[12]) != 1) abort();
    const PacketCopy list = find_message(0xF06);
    if (list.size != 0x11C || list.bytes.size() != 0x11C ||
        *reinterpret_cast<const uint32_t *>(&list.bytes[4]) != 1 ||
        list.bytes[0x10] != 0 || list.bytes[0x11] != 0x5A ||
        strcmp(reinterpret_cast<const char *>(&list.bytes[0x18]), "sample.dat") != 0) abort();
    const PacketCopy done = find_message(0xF07);
    if (done.size != 0x20 || done.target != 0) abort();
    const PacketCopy path = find_message(0xF05);
    if (path.size != 0x11C || path.bytes.size() != 0x11C ||
        memcmp(&path.bytes[0x18], "assets", sizeof("assets")) != 0) abort();
    int completion_count = 0;
    for (size_t i = 0; i < g_messages.size(); ++i) {
        if (g_messages[i].type != 0xF07) continue;
        int state;
        memcpy(&state, &g_messages[i].bytes[8], sizeof state);
        if (state != 2 + completion_count) abort();
        ++completion_count;
    }
    if (completion_count != 2) abort();
    if (strcmp(g_cwd, "C:\\Old") != 0 || NetDaemonFileTransferMode != 0) abort();
}

static void test_ack_wait_live_mask_and_status_scan() {
    reset_fixture();
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetDaemonNet.player_count_ = 2;
    NetDaemonNet.process_slots_[2].pid_ = 0x333;
    g_scenario = 4;
    g_ack_value = 4;
    if (NetDaemonNet.send_files() != 0) abort();
    if (NetDaemonNet.get_calls_ == 0) abort();
    if (NetDaemonNet.process_slots_[1].status_bytes_[0] <= 1 ||
        NetDaemonNet.process_slots_[2].status_bytes_[0] <= 1) abort();
}

static void test_exact_chunk_boundary_and_escape_cleanup() {
    reset_fixture();
    g_test_file_size = 4000;
    g_compressed_size = 0x800;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    if (NetDaemonNet.send_files() != 0) abort();
    if (g_group_packets.size() != 1 || g_group_packets[0].size != 0x820) abort();
    // One draw belongs to the full 0x800-byte send iteration; the staged
    // status remains 1 for an additional poll before transitioning to 2.
    if (g_draw_count != 4 || g_chunk_draws != 1) abort();

    reset_fixture();
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    g_escape_after = 0;
    if (NetDaemonNet.send_files() != 1) abort();
    if (!g_group_packets.empty() || NetDaemonNet.channels_[1].trailing_words_[9] != 1 ||
        g_free_count != 1) abort(); // Native early escape leaks the send buffer.
    if (strcmp(g_cwd, "C:\\Old") != 0 || NetDaemonFileTransferMode != 0) abort();
}

static void test_client_escape_and_live_mode_cancel() {
    reset_fixture();
    NetDaemonNet.primary_state_.leading_words_[0] = 0;
    NetDaemonFileTransferMode = 1;
    g_escape_after = 0;
    const int result = NetDaemonNet.send_files();
    if (result != 1 || g_messages.empty()) abort();
    if (g_messages[0].type != 0xF0F || g_messages[0].size != 0x20) abort();
    int source_player;
    memcpy(&source_player, &g_messages[0].bytes[4], sizeof source_player);
    if (source_player != 1) abort();
    if (g_alt_path[0] == 0 || strcmp(g_cwd, "C:\\Old") != 0) abort();

    reset_fixture();
    NetDaemonNet.primary_state_.leading_words_[0] = 0;
    NetDaemonFileTransferMode = 1;
    g_scenario = 3;
    if (NetDaemonNet.send_files() != 0) abort();
    if (NetDaemonFileTransferMode != 0) abort();
}

static void test_multiple_file_indices_are_not_constant_zero() {
    reset_fixture();
    strcpy(SaveNameBuffer, "C:\\Game\\assets");
    g_find_count = 2;
    g_compressed_size = 2050;
    if (NetDaemonNet.send_files() != 0 || g_file_open_count != 2) abort();
    if (g_group_packets.size() != 4) abort();
    for (size_t i = 0; i < g_group_packets.size(); ++i) {
        int file_index;
        memcpy(&file_index, &g_group_packets[i].bytes[8], sizeof file_index);
        if (file_index != static_cast<int>(i / 2)) abort();
    }
}

static void test_callback_rebinds_and_saved_masks() {
    reset_fixture();
    g_find_count = 2;
    g_rebind_tables = true;
    if (NetDaemonNet.send_files() != 0) abort();
    if (g_reported_counts.size() != 2 || g_reported_counts[0] != 2 ||
        g_reported_counts[1] != 1) abort();
    if (g_open_names.size() != 2 || g_open_names[0] != "rebound-first.dat" ||
        g_open_names[1] != "rebound-second.dat") abort();
    if (g_group_members.size() != 2 || g_group_members[0] != 0x222 ||
        g_group_members[1] != 0x444) abort();
    if (g_popup_file != g_rebound_script_file) abort();
    int lists = 0;
    for (size_t i = 0; i < g_messages.size(); ++i) {
        if (g_messages[i].type != 0xF06) continue;
        if (g_messages[i].bytes[0x11] != 0x5A ||
            g_messages[i].bytes[0x10] != lists) abort();
        if (strcmp(reinterpret_cast<const char *>(&g_messages[i].bytes[0x18]),
                   lists == 0 ? "sample.dat" : "second.dat") != 0) abort();
        ++lists;
    }
    if (lists != 2) abort();
    for (size_t i = 0; i < g_retained_tables.size(); ++i) ::free(g_retained_tables[i]);
    g_retained_tables.clear();

    reset_fixture();
    g_fail_open = true;
    if (NetDaemonNet.send_files() != 0 || !g_group_packets.empty() ||
        g_file_open_count != 1 || g_reported_counts[0] != 1 ||
        strcmp(g_cwd, "C:\\Old") != 0) abort();
}

static void test_receiver_methods_and_global_protocol_state() {
    reset_fixture();
    NetDaemon receiver;
    g_active_daemon = &receiver;
    receiver.primary_state_.leading_words_[0] = 1;
    NetDaemonNet.player_count_ = 2;
    NetDaemonNet.process_slots_[2].status_bytes_[0] = 0;
    receiver.process_slots_[2].status_bytes_[0] = 77;
    g_scenario = 4; g_ack_value = 4;
    receiver.player_count_ = 0; // Native count is the singleton's count.
    receiver.process_slots_[1].pid_ = 0x987;
    receiver.channels_[1].trailing_words_[9] = 77;
    NetDaemonNet.primary_state_.leading_words_[0] = 0;
    if (receiver.send_files() != 0 || g_group_members.size() != 1 ||
        g_group_members[0] != 0x987 || receiver.process_slots_[1].status_bytes_[0] != 2 ||
        NetDaemonNet.process_slots_[1].status_bytes_[0] != 0 ||
        receiver.channels_[1].trailing_words_[9] != 77 ||
        receiver.file_index_records_ != 0 || NetDaemonNet.file_index_records_ != 0 ||
        g_setup_local_status != 2 || g_setup_receiver_status != 0 ||
        g_setup_global_status != 0 ||
        NetDaemonNet.get_calls_ == 0 || receiver.get_calls_ != 0) abort();
    for (size_t i = 0; i < g_method_owners.size(); ++i)
        if (g_method_owners[i] != &receiver) abort();

    reset_fixture();
    NetDaemon client;
    g_active_daemon = &client;
    client.primary_state_.leading_words_[0] = 0;
    client.field_764_ = 0x765;
    g_scenario = 5;
    if (client.send_files() != 1 || client.channels_[1].trailing_words_[9] != 0 ||
        NetDaemonNet.channels_[1].trailing_words_[9] != 1 ||
        g_messages[0].target != 0x765 || g_method_owners[0] != &client) abort();

    reset_fixture();
    NetDaemon escaping_client;
    g_active_daemon = &escaping_client;
    escaping_client.primary_state_.leading_words_[0] = 0;
    g_escape_after = 0;
    if (escaping_client.send_files() != 1 ||
        escaping_client.channels_[1].trailing_words_[9] != 0 ||
        NetDaemonNet.channels_[1].trailing_words_[9] != 1) abort();

    reset_fixture();
    NetDaemon escaping;
    g_active_daemon = &escaping;
    escaping.primary_state_.leading_words_[0] = 1;
    g_escape_after = 0;
    if (escaping.send_files() != 1 || escaping.channels_[1].trailing_words_[9] != 0 ||
        NetDaemonNet.channels_[1].trailing_words_[9] != 1) abort();
}

static void test_observation_only_unsigned_tail_words() {
    // Synthetic allocator/compressor observations, not real native storage
    // or allocation success. Neither fake addresses nor enormous extents are
    // accessed: BOTH copy APIs and send_group are intercepted in this mode.
    const uintptr_t bases[] = {0x1000, 0xFFFFFF00, 0x1000, 0x1000, 0x1000, 4};
    const uint32_t capacities[] = {0x200, 0x200, 0x80000000, 0xFFFFFFFF, 0, 0xFFFFFFF0};
    const int copies[] = {1, 0, 1, 0, 0, 1};
    for (int i = 0; i < 6; ++i) {
        reset_fixture();
        g_observe_invalid_tail = true;
        g_observed_base = bases[i]; g_observed_capacity = capacities[i];
        if (NetDaemonNet.send_files() != 0 || g_observed_copies != copies[i] ||
            g_observed_sends != copies[i]) abort();
        if (copies[i] && (g_observed_copy_count != capacities[i] ||
            g_observed_send_count != capacities[i] + 0x20u ||
            !g_observed_memmove)) abort();
    }
}

static void test_abort_clear_precedes_lookup_observation() {
    reset_fixture();
    NetDaemon receiver;
    g_active_daemon = &receiver;
    receiver.primary_state_.leading_words_[0] = 0;
    g_observe_lookup = true;
    NetDaemonNet.channels_[1].trailing_words_[9] = 9;
    g_scenario = 3;
    // Observe the actual source call prefix. This does not assert that the
    // real pid lookup reenters or establish behavior after a native failure.
    if (receiver.send_files() != 1 || g_lookup_abort != 0 || g_draw_count != 0)
        abort();
}

int main() {
    test_host_send_chunking_and_order();
    test_ack_wait_live_mask_and_status_scan();
    test_exact_chunk_boundary_and_escape_cleanup();
    test_client_escape_and_live_mode_cancel();
    test_multiple_file_indices_are_not_constant_zero();
    NetDaemonNet.process_slots_[3].pid_ = 0x444;
    test_callback_rebinds_and_saved_masks();
    test_receiver_methods_and_global_protocol_state();
    test_observation_only_unsigned_tail_words();
    test_abort_clear_precedes_lookup_observation();
    puts("send_files host controls passed");
    return 0;
}
