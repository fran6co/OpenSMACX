// Actual-source receiver controls. Doubles observe calls; they do not emulate
// native unbounded memory access, decompression, transport or reentrancy reachability.
#include <cassert>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>
typedef unsigned long uLongf;
static int mode, parsed_progress, closes, send_calls, inflate_calls;
static unsigned incoming_size, inflate_size;
static unsigned char packet[0x820], output[0x2000];
static std::vector<int> events;
static uint32_t word(unsigned offset) { uint32_t value; std::memcpy(&value,packet+offset,4); return value; }
static void put(unsigned offset,uint32_t value) { std::memcpy(packet+offset,&value,4); }
struct Popup { void close() {} };
class NetDaemonTransferModalCloseGuard;
struct LockWords { uint32_t leading_words_[4]; };
struct Slot { uint8_t status_bytes_[4]; };
struct MessageFactionData {};
struct { void setup_faction_data(MessageFactionData *) {} bool is_visible() { return false; } void pick_pass(int) {} } NetWinDialog;
struct Sequence { uint32_t value; uint32_t sequence_bits() { return value; } uint32_t advance_sequence() { return ++value; } };
@@RECORD@@
static NetDaemonFileIndexRecord *rows;
int NetDaemonFileTransferMode, NetDaemonFileTransferCurrentFile;
uint32_t NetDaemonFileTransferAckMask;
int NetDaemonFileTransferChunkCount, NetDaemonFileTransferProgress;
unsigned char *NetDaemonFileTransferReceiveBuffer;
char NetDaemonFileTransferPath[0x104];
int PopupModalState;
@@RESET_WORD@@
static NetDaemonFileIndexRecord *netdaemon_file_index_records() { return rows; }
static void netdaemon_set_file_index_records(NetDaemonFileIndexRecord *value) { rows=value; }
static void *mem_get(int size) { void *storage=std::malloc(size_t(size)); if(storage) std::memset(storage,0xCD,size_t(size)); return storage; }
struct NetDaemonNetType {
    uint32_t local_player_id_;
    int pid_2_idx(uint32_t) {
        if (mode==7 || mode==8) assert(NetDaemonTransferResetWord==-1);
        if (mode==8) { assert(rows!=0); NetDaemonFileTransferCurrentFile=-1; }
        return 3;
    }
} NetDaemonNet;
struct Filemap {
    void *create(const char *name,uint32_t size,int sequential) {
        events.push_back(3);
        assert(sequential==0);
        if (mode==6) { assert(size==0 && !std::strcmp(name,"received.bin")); }
        else { assert(size==0x1200 && !std::strcmp(name,mode==4?"redirect.bin":"received.bin")); }
        if (mode==5) put(0xC,2);
        return output;
    }
    void close() { events.push_back(5); }
};
struct NetDaemon {
    LockWords primary_state_,channels_[2]; Slot process_slots_[16]; Sequence deletion_list_;
    int fixup_message(char *) { return 0; }
    void send_message(char *,int length,int target) {
        assert(target==0x44); ++send_calls; events.push_back(6);
        if (mode==6) { assert(length==0x14); packet[0x10]=4; }
        else if (mode==7 || mode==8) { assert(length==int(incoming_size)); put(8,97); }
        else { assert(length==0x20); }
    }
    bool process_file_transfer_message(char *,unsigned long,int,NetDaemonTransferModalCloseGuard &,Filemap &);
};
static unsigned char checksum_file(const char *) { return 0; }
static uint32_t timeGetTime() { return 0x13572468; }
static int netdaemon_inflate_buffer(unsigned char *dest,unsigned long *size,unsigned char *input,unsigned int count) {
    assert(dest==output && *size==0x1200 && input==NetDaemonFileTransferReceiveBuffer);
    inflate_size=count; ++inflate_calls; events.push_back(4); return -1; // Native ignores this error.
}
static void parse_says(int,const char *name,int,int) {
    assert(!std::strcmp(name,"received.bin"));
    assert(NetDaemonFileTransferReceiveBuffer[0]==0xA5); // Copy must precede all UI.
    events.push_back(1);
}
static void parse_num(int,int value) { parsed_progress=value; assert(value==8); }
static void pop_close() { ++closes; events.push_back(7); }
static void x_pop_caption_flags(const char *,const char *,int flags,int) {
    assert(flags==0x214200 && PopupModalState==6 && closes==0);
    assert(NetDaemonFileTransferProgress==8); events.push_back(2);
    if (mode==4) { put(8,4); put(0xC,1); NetDaemonFileTransferCurrentFile=4; }
    if (mode==9) NetDaemonFileTransferProgress=20;
}
@@BODIES@@
static void reset(int new_mode,unsigned short opcode,unsigned chunk=0) {
    mode=new_mode; events.clear(); closes=send_calls=inflate_calls=0;
    inflate_size=0; parsed_progress=-1; incoming_size=37;
    std::memset(packet,0,sizeof packet); std::memcpy(packet,&opcode,2);
    put(4,0x801); put(8,3); put(0xC,chunk);
    std::memset(packet+0x20,0xA5,5);
    rows=static_cast<NetDaemonFileIndexRecord *>(std::calloc(256,sizeof *rows));
    assert(rows); rows[3].file_size=0x1200; rows[3].chunk_count=2;
    rows[4].file_size=0x1200; rows[4].chunk_count=2;
    std::strcpy(rows[3].filename,"received.bin"); std::strcpy(rows[4].filename,"redirect.bin");
    NetDaemonFileTransferCurrentFile=3; NetDaemonFileTransferProgress=8;
    NetDaemonTransferResetWord=17;
    NetDaemonFileTransferChunkCount=41; PopupModalState=0;
    NetDaemonFileTransferReceiveBuffer=chunk?static_cast<unsigned char *>(std::calloc(1,0x1200)):0;
}
static void invoke() {
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    Popup popup; NetDaemonTransferModalCloseGuard guard(popup); Filemap filemap;
    assert(daemon.process_file_transfer_message(reinterpret_cast<char *>(packet),incoming_size,0x44,guard,filemap));
    assert(!guard.modal_abort());
}
static void clean() {
    std::free(rows); rows=0; std::free(NetDaemonFileTransferReceiveBuffer); NetDaemonFileTransferReceiveBuffer=0;
}
int main() {
    assert(sizeof(NetDaemonFileIndexRecord)==0x11C);
    reset(0,0xF0B); invoke();
    assert(rows[3].chunk_count==2 && NetDaemonFileTransferChunkCount==41);
    assert(parsed_progress==8 && NetDaemonFileTransferProgress==7 && PopupModalState==0);
    assert(closes==1 && send_calls==0 && inflate_calls==0);
    assert(events.size()==3 && events[0]==1 && events[1]==2 && events[2]==7); clean();
    reset(1,0xF0B,1); invoke();
    assert(inflate_size==0x800+incoming_size && inflate_calls==1 && closes==1 && send_calls==1);
    assert(rows==0 && NetDaemonFileTransferReceiveBuffer==0);
    assert(events.size()==5 && events[0]==3 && events[1]==4 && events[2]==5 && events[3]==6 && events[4]==7); clean();
    reset(2,0xF0B); put(4,0x80000000); invoke();
    assert(rows[3].chunk_count==uint32_t(-1048575) && NetDaemonFileTransferChunkCount==41); clean();
    reset(3,0xF0B); put(4,0xFFFFFFFF); invoke(); assert(rows[3].chunk_count==0); clean();
    reset(4,0xF0B); invoke();
    assert(inflate_calls==1 && inflate_size==0x800+incoming_size && send_calls==1 && rows==0); clean();
    reset(5,0xF0B,1); invoke();
    assert(inflate_calls==1 && inflate_size==0x1000+incoming_size && send_calls==0 && closes==1 && rows!=0); clean();
    reset(6,0xF06); packet[0x10]=3; packet[0x11]=1; put(0x14,0x1200);
    std::strcpy(reinterpret_cast<char *>(packet+0x18),"received.bin");
    rows[3].filename[0]='X'; invoke();
    assert(rows[3].filename[0]=='X' && !std::strcmp(rows[4].filename,"received.bin"));
    assert(rows[4].chunk_count==word(0xC) && rows[4].file_size==0x1200 && send_calls==1); clean();
    reset(7,0xF07); incoming_size=67; invoke();
    uint16_t opcode; std::memcpy(&opcode,packet,2);
    assert(opcode==0xF0A && NetDaemonFileTransferMode==97 && send_calls==1 && rows!=0); clean();
    reset(8,0xF07); incoming_size=67; invoke(); std::memcpy(&opcode,packet,2);
    assert(opcode==0xF0C && rows==0 && NetDaemonFileTransferMode==97 && send_calls==1); clean();
    reset(9,0xF0B); invoke(); assert(NetDaemonFileTransferProgress==19); clean();
    const uint32_t words[]={0,0x7FF,0x800,0x801,0x7FFFF7FF,0x7FFFF800,0x7FFFFFFF,0x80000000,0xFFFFF7FF,0xFFFFF800,0xFFFFFFFF};
    for (unsigned index=0;index<sizeof(words)/sizeof(*words);++index) {
        reset(10,0xF0B); NetDaemonFileTransferCurrentFile=-1; put(4,words[index]); invoke();
        // Independently model native ADD/CDQ/AND/ADD/SAR, including wrapping.
        const uint32_t sum=words[index]+0x800u;
        const uint32_t adjusted=sum+((sum&0x80000000u)?0x7FFu:0u);
        const int64_t signed_adjusted=(adjusted&0x80000000u)?int64_t(adjusted)-0x100000000LL:int64_t(adjusted);
        int64_t quotient=signed_adjusted/0x800;
        if (signed_adjusted<0 && signed_adjusted%0x800!=0) --quotient;
        assert(rows[3].chunk_count==uint32_t(quotient) && NetDaemonFileTransferChunkCount==41); clean();
    }
    reset(11,0xF05); std::strcpy(reinterpret_cast<char *>(packet+0x18),"transfer/path"); invoke();
    assert(NetDaemonTransferResetWord==-1 && NetDaemonFileTransferMode==1 && NetDaemonFileTransferCurrentFile==-1);
    assert(!std::strcmp(NetDaemonFileTransferPath,"transfer/path")); clean();
}
