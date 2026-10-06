#include <cassert>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
typedef unsigned long uLong;
typedef unsigned long uLongf;
typedef int BOOL;
typedef const char *LPCSTR;
typedef void *LPVOID;

static int g_filemap_dtors;
struct Popup {
    int closes;
    void close() { ++closes; trace[trace_n++] = 2; }
    ~Popup() { trace[trace_n++] = 3; }
    static int trace[64], trace_n;
};
int Popup::trace[64];
int Popup::trace_n;
struct Filemap {
    Filemap() {}
    ~Filemap() { ++g_filemap_dtors; Popup::trace[Popup::trace_n++] = 1; }
    LPVOID create(LPCSTR, uint32_t, BOOL) { return target; }
    void close() {}
    static void *target;
};
void *Filemap::target;
struct NetDaemonFileIndexRecord {
    uint32_t ready_flags;
    uint32_t chunk_count;
    uint32_t file_size;
    char filename[260];
};
struct MessageFactionData { unsigned char bytes[64]; };
struct FakeNetWin {
    int setup_calls, visible, pick_calls, picked;
    MessageFactionData *last_data;
    void setup_faction_data(MessageFactionData *data) {
        ++setup_calls; last_data=data;
    }
    int is_visible() { return visible; }
    void pick_pass(int pass) { ++pick_calls; picked=pass; }
} NetWinDialog;
struct Slot { unsigned char status_bytes_[4]; };
struct LockWord { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
class NetDaemonTransferModalCloseGuard;
struct NetDaemonNetType {
    uint32_t field_760_;
    int pid_2_idx(uint32_t) { return 3; }
} NetDaemonNet;
struct NetDaemon {
    LockWord lock_state_[24];
    Slot process_slots_[16];
    int send_calls;
    int last_send_len, last_send_target;
    int fixup_calls;
    int fixup_message(char *) { ++fixup_calls; return 0; }
    void send_message(char *, int length, int target) {
        ++send_calls; last_send_len=length; last_send_target=target;
    }
    bool process_file_transfer_message(char *, unsigned long, int,
                                       NetDaemonTransferModalCloseGuard &,
                                       Filemap &);
};
static NetDaemonFileIndexRecord records[256];
static NetDaemonFileIndexRecord *records_owner = records;
static NetDaemonFileIndexRecord *netdaemon_file_index_records() {
    return records_owner;
}
static void netdaemon_set_file_index_records(NetDaemonFileIndexRecord *p) {
    records_owner=p;
}
int NetDaemonFileTransferMode, NetDaemonFileTransferCurrentFile;
uint32_t NetDaemonFileTransferAckMask;
int NetDaemonFileTransferChunkCount, NetDaemonFileTransferProgress;
unsigned char *NetDaemonFileTransferReceiveBuffer;
char NetDaemonFileTransferPath[260];
int PopupModalState;
static void *mem_get(int size) { return malloc((size_t)size); }
// This modal control doubles the transfer operation; the separate actual-body
// owner controls exercise the recovered wrapper's statuses and size writeback.
static int netdaemon_inflate_buffer(unsigned char *, unsigned long *, unsigned char *, unsigned int) {
    return 0;
}
static void parse_says(int, const char *, int, int) {}
static void parse_num(int, int) {}
static void pop_close() {}
static void x_pop_caption_flags(const char *, const char *, int, int) {}
static unsigned char checksum_file(const char *) { return 0; }

// Source-extracted guard, helper and member definition are inserted verbatim.
@@TRANSFER_CALLBACK_BODIES@@

static void reset_trace() {
    Popup::trace_n = 0;
    memset(Popup::trace, 0, sizeof(Popup::trace));
}
static bool invoke_modal(NetDaemon &daemon, unsigned short type) {
    Popup popup;
    popup.closes = 0;
    NetDaemonTransferModalCloseGuard guard(popup);
    Filemap filemap;
    unsigned char message[128];
    memset(message, 0, sizeof(message));
    *reinterpret_cast<uint16_t *>(message) = type;
    return daemon.process_file_transfer_message(
        reinterpret_cast<char *>(message), sizeof(message), 0x44, guard, filemap);
}
static void check_modal(unsigned short type, bool is_host) {
    NetDaemon daemon;
    memset(&daemon, 0, sizeof(daemon));
    daemon.lock_state_[0].leading_words_[1] = is_host ? 1 : 0;
    reset_trace();
    g_filemap_dtors = 0;
    const bool consumed = invoke_modal(daemon, type);
    assert(consumed);
    assert(Popup::trace_n == 3);
    assert(Popup::trace[0] == 1 && Popup::trace[1] == 2 &&
           Popup::trace[2] == 3);
    assert(g_filemap_dtors == 1);
    assert(daemon.send_calls == 0);
    assert(NetDaemonFileTransferMode == 77);
}
static bool invoke_message(NetDaemon &daemon, unsigned short type,
                           unsigned char flags) {
    Popup popup;
    popup.closes = 0;
    NetDaemonTransferModalCloseGuard guard(popup);
    Filemap filemap;
    unsigned char message[128];
    memset(message, 0, sizeof(message));
    *reinterpret_cast<uint16_t *>(message) = type;
    (void)flags;
    return daemon.process_file_transfer_message(
        reinterpret_cast<char *>(message), 37, 0x44, guard, filemap);
}
int main() {
    NetDaemonFileTransferMode=77;
    NetDaemonFileTransferCurrentFile=19;
    NetDaemonFileTransferProgress=8;
    NetDaemonFileTransferChunkCount=5;
    NetDaemonFileTransferAckMask=0;
    memset(records, 0, sizeof(records));
    records_owner=records;
    check_modal(0xF05, true);
    check_modal(0xF06, true);
    check_modal(0xF07, true);
    check_modal(0xF0B, true);
    check_modal(0xF08, false);
    check_modal(0xF0F, false);

    NetDaemon daemon;
    memset(&daemon, 0, sizeof(daemon));
    daemon.lock_state_[0].leading_words_[0] = 9;
    assert(invoke_message(daemon, 0xF09, 0) == true);
    assert(daemon.fixup_calls == 0 && daemon.send_calls == 0);
    assert(daemon.lock_state_[0].leading_words_[0] == 9);

    MessageFactionData faction_message;
    memset(&faction_message, 0, sizeof(faction_message));
    assert(invoke_message(daemon, 0xF0D, 0) == true);
    assert(NetWinDialog.setup_calls == 1);
    assert(NetWinDialog.last_data != 0);
    NetWinDialog.visible = 0;
    assert(invoke_message(daemon, 0xF0E, 0) == true);
    assert(NetWinDialog.pick_calls == 0);
    NetWinDialog.visible = 1;
    assert(invoke_message(daemon, 0xF0E, 0) == true);
    assert(NetWinDialog.pick_calls == 1 && NetWinDialog.picked == 1);
    puts("transfer modal cleanup controls passed");
}
