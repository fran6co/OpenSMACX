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
typedef unsigned long DWORD;
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
    LPVOID open_read(LPCSTR, BOOL) { opened_ = true; ++g_file_open_count; return reinterpret_cast<LPVOID>(1); }
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
    int pid_2_idx(unsigned long id) { return id == field_760_ ? 1 : 2; }
    void *get(unsigned long *sender, unsigned long *length) {
        ++get_calls_;
        if (get_calls_ > 100) { fprintf(stderr, "get pump runaway\\n"); ::abort(); }
        *sender = 0x222;
        *length = 0x20;
        if (g_scenario == 4) NetDaemonFileTransferAckMask = g_ack_value;
        return 0;
    }
    int send(void *data, int size, unsigned long who, int mode) {
        PacketCopy packet;
        packet.type = *reinterpret_cast<unsigned short *>(data);
        packet.size = size;
        if (mode != 1) abort();
        packet.target = static_cast<int>(who);
        packet.bytes.resize(size);
        memcpy(&packet.bytes[0], data, size);
        g_messages.push_back(packet); return 1;
    }
    int create_group(char *) { return 7; }
    void delete_group(int) {}
    int add_to_group(long, unsigned long member) {
        g_group_members.push_back(static_cast<int>(member)); return 0;
    }
    int send_group(void *data, int size, long, int) {
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
    void *file_index_records_;
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
static int PopupModalState;
static int NetDaemonFileTransferMode;
uint32_t NetDaemonFileTransferAckMask;
static int NetDaemonFileTransferCurrentFile;
static int NetDaemonFileTransferChunkCount;
static int NetDaemonFileTransferProgress;
static unsigned char *NetDaemonFileTransferReceiveBuffer;

static void *mem_get(size_t size) { return calloc(1, size == 0 ? 1 : size); }
static int checksum_file(const char *) { return 0x5A; }
static void filefind_set_alternative(const char *path) {
    strncpy(g_alt_path, path, sizeof(g_alt_path) - 1);
}
static void parse_says(int, char *, int, int) {}
static void parse_num(int, int) {}
static void x_pop_caption_flags(char *, char *, int, int) {
    g_chunk_start = g_group_packets.size();
    g_transfer_ui = 1;
}
static void do_draw();
static void do_all_draws() {}
static void pop_close() {}

int netdaemon_deflate_buffer(unsigned char *out, unsigned long *out_size,
               unsigned char *, unsigned int) {
    if (*out_size < g_compressed_size) return -5;
    memset(out, 0xA5, g_compressed_size);
    *out_size = g_compressed_size;
    return 0;
}
static void test_free(void *p) {
    if (p != 0) ++g_free_count;
    ::free(p);
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
    NetDaemonNet.process_slots_[1].status_bytes_[0] =
        static_cast<unsigned char>(g_draw_count < 4 ? 1 : 2);
    if (NetDaemonNet.player_count_ > 1) {
        for (int slot = 2; slot <= NetDaemonNet.player_count_; ++slot)
            NetDaemonNet.process_slots_[slot].status_bytes_[0] = 2;
    }
    if (g_scenario == 3 && g_draw_count == 1) NetDaemonFileTransferMode = 3;
}
void NetDaemon::do_draw() { ::do_draw(); }

#define free test_free
@@OWNER_SOURCE@@
#undef free

void NetDaemon::send_message(char *bytes, unsigned long size, int target) {
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
    if (result != 0 || g_file_open_count != 1 || g_findclose_count != 1) abort();
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
    if (!g_group_packets.empty() || NetDaemonNet.channels_[1].trailing_words_[9] != 1) abort();
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

int main() {
    test_host_send_chunking_and_order();
    test_ack_wait_live_mask_and_status_scan();
    test_exact_chunk_boundary_and_escape_cleanup();
    test_client_escape_and_live_mode_cancel();
    test_multiple_file_indices_are_not_constant_zero();
    puts("send_files host controls passed");
    return 0;
}
