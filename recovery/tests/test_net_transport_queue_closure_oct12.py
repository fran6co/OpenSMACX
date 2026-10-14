"""Coupled actual-owner transport controls, not native ABI/runtime acceptance.

Net, its full sixteen-row array, FIFO, allocation and voice declarations are
extracted from the product headers. Access labels are opened in the private
host probe, never in the product. Host pointer/layout differences are explicit:
the allocator backs the original 0x18 request with a complete host FIFO node.
SDK/event/time doubles model specified nonfaulting callbacks only. Native pins
are independently checked through PeImage/Capstone using the absolute image.
"""
from __future__ import annotations

import os
import json
from pathlib import Path
import re
import resource
import shutil
import struct
import subprocess

import pytest

from test_mapwin_cache import braced

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
NET_METHODS = (
    "int NetThreadInfo::internal_receive()",
    "int Net::check_message(unsigned long player, unsigned long stamp)",
    "int Net::drop_player(unsigned long key, int reason)",
    "void *Net::sub_630a40(int key, int *length_out)",
    "void *Net::read_queue_key_until_timeout(int key, int *length_out)",
    "int Net::send_packet_type(void *data, int data_size, unsigned long target,",
)
FIFO_METHODS = (
    "int NetFifo::add(void *data, unsigned long size, unsigned long key, int type,",
    "int NetFifo::get(void *data, int key, int *type, int *len)",
)


def declaration(text: str, head: str) -> str:
    return braced(text, head) + ";"


def uncomment(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def default_virtuals(owner: str, text: str, omitted: set[str] | None = None) -> str:
    """Host-only definitions from actual declarations, not positional slots."""
    omitted = omitted or set()
    result = []
    for return_type, name, arguments in re.findall(
        r"virtual\s+(\w+(?:\s*\*)?)\s*(\w+)\s*\(([^()]*)\)\s*;", uncomment(text)
    ):
        if name in omitted:
            continue
        body = "" if return_type == "void" else "return 0;"
        result.append(f"{return_type} {owner}::{name}({arguments}) {{{body}}}")
    return "\n".join(result)


PREFIX = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <string>
#include <vector>
#include <new>
#define __cdecl
using DWORD = uint32_t;
using WORD = uint16_t;
using DPID = DWORD;
using HRESULT = int;
using HANDLE = uintptr_t;
using LONG = int32_t;
using HPSTR = char *;
const int DP_OK = 0, FALSE = 0;
const DWORD WAIT_OBJECT_0 = 0, WAIT_TIMEOUT = 258;
struct CRITICAL_SECTION { int depth; };
struct _GUID { uint32_t a; uint16_t b, c; unsigned char d[8]; };
using GUID = _GUID;
struct DPNAME;
struct Heap {};
Heap *StringAllocationHeap = 0;
class Net;
class AlphaNetStringStruct;
struct IDirectPlay2;
struct _MMIOINFO {
    unsigned char unused[20]; LONG cchBuffer;
    HPSTR pchBuffer, pchNext, pchEndRead;
};
using MMIOINFO = _MMIOINFO;
void *mem_get(int);
void InitializeCriticalSection(CRITICAL_SECTION *p) { p->depth = 0; }
void DeleteCriticalSection(CRITICAL_SECTION *p) { assert(p->depth == 0); }
void EnterCriticalSection(CRITICAL_SECTION *p) { ++p->depth; }
void LeaveCriticalSection(CRITICAL_SECTION *p) { assert(p->depth > 0); --p->depth; }
#define private public
#define protected public
'''

SUPPORT = r'''
#undef private
#undef protected
unsigned char NetGetScratch[0x2000];
uint32_t NetHostChangeSlot;
static Net *active;
static unsigned char *active_packet;
static DWORD clock_tick = 100;
static int clock_step, alloc_count, fail_alloc_at;
static bool alloc_changes_tail, alloc_changes_head, receive_changes_buffer, mutate_event_word;
static bool time_changes_receive_arguments, mutate_voice_count;
static DWORD *retained_from, *retained_size;
static void *replacement_buffer;
static int send_count, send_fail_at, wait_count, wait_mode;
static int caption_choice, tolerance_choice, closes, sound_version;
static bool popup_reorders_rows, popup_changes_rtt, popup_drops_interface;
static std::vector<DWORD> timeouts;
static std::vector<HANDLE> signals;
static std::vector<std::string> captions, boxes;
static std::vector<std::vector<unsigned char> > sent;
static std::string callback_order;
static DWORD sent_from, sent_to;
static int received_calls, stop_calls, start_calls, add_buffer_calls;
static int voice_level, voice_playing;
static int deleted_voices;
static std::string removed_name;
static DWORD removed_id;
static NetFifo::NetFifoNode *alternate_tail;
static NetFifo *allocation_queue;

void *mem_get(int size) {
    ++alloc_count;
    if (alloc_count == fail_alloc_at) return 0;
    // Native requests 0x18; complete host nodes contain wider pointers. Never
    // present this adjusted backing size as original allocation/layout proof.
    size_t backing = static_cast<size_t>(size);
    if (size == 0x18 && backing < sizeof(NetFifo::NetFifoNode))
        backing = sizeof(NetFifo::NetFifoNode);
    void *memory = std::malloc(backing);
    assert(memory);
    std::memset(memory, 0xA5, backing);
    if (alloc_changes_tail && alloc_count == 2) {
        assert(allocation_queue && alternate_tail);
        allocation_queue->tail_ = alternate_tail;
    }
    if (alloc_changes_head && alloc_count == 1) {
        allocation_queue->head_ = allocation_queue->tail_ = alternate_tail;
    }
    return memory;
}
struct IDirectPlay2 {
    std::vector<unsigned char> incoming;
    DWORD incoming_from = 42, incoming_to = 222;
    HRESULT receive_result = DP_OK;
    void *received_buffer = 0;
    virtual HRESULT Receive(DPID *from, DPID *to, DWORD flags, void *buffer, DWORD *size) {
        ++received_calls;
        assert(flags == 3 && *from == 0 && *size == active->field_6E4_);
        received_buffer = buffer;
        if (receive_result != DP_OK) return receive_result;
        assert(incoming.size() <= *size);
        memcpy(buffer, incoming.data(), incoming.size());
        *size = static_cast<DWORD>(incoming.size()); *from = incoming_from; *to = incoming_to;
        retained_from = from; retained_size = size;
        if (receive_changes_buffer) active->thread_info_.buffer_ = replacement_buffer;
        return DP_OK;
    }
    virtual HRESULT Send(DPID from, DPID to, DWORD flags, void *packet, DWORD size) {
        (void)flags;
        sent_from = from; sent_to = to;
        sent.emplace_back(static_cast<unsigned char *>(packet),
                          static_cast<unsigned char *>(packet) + size);
        ++send_count;
        return send_count == send_fail_at ? 1 : DP_OK;
    }
};
static IDirectPlay2 provider;
IDirectPlay2 *NetDirectPlay = &provider;
DWORD timeGetTime() {
    if (time_changes_receive_arguments) {
        time_changes_receive_arguments = false;
        *retained_from = 84; *retained_size = 4;
    }
    DWORD answer = clock_tick;
    clock_tick += static_cast<DWORD>(clock_step);
    return answer;
}
int SetEvent(HANDLE handle) {
    signals.push_back(handle);
    if (mutate_event_word) {
        mutate_event_word = false;
        reinterpret_cast<NetPacketEnvelope *>(active_packet)->type_flags_ = 0x104;
        active->local_player_id_ = 77;
    }
    return 1;
}
int PostThreadMessageA(DWORD thread, unsigned msg, uintptr_t a, intptr_t b) {
    assert(thread == 9 && msg == 0x402 && a == 0 && b == 0);
    callback_order += 'P'; return 1;
}
int ResetEvent(HANDLE) { return 1; }
DWORD WaitForMultipleObjects(DWORD count, const HANDLE *, int all, DWORD) ;
DWORD WaitForMultipleObjects(DWORD count, const HANDLE *, int all, DWORD timeout) {
    assert(count == 2 && all == FALSE);
    timeouts.push_back(timeout); ++wait_count;
    if ((wait_mode == 1 || wait_mode == 2) && wait_count == 1) {
        clock_tick = 20100; return WAIT_TIMEOUT;
    }
    if (wait_mode == 3) { clock_tick = 20100; return WAIT_TIMEOUT; }
    for (int i = 0; i < 16; ++i) active->players_[i].field_8_ = 0;
    clock_tick += 100;
    return WAIT_OBJECT_0 + 1;
}
int MessageBoxA(void *, const char *message, const char *, unsigned) {
    boxes.emplace_back(message); callback_order += 'M'; return 0;
}
void parse_says(int, const char *, int, int) { callback_order += 'G'; }
int pop_caption_value_title(char *, char *caption, int, int, int) {
    captions.emplace_back(caption);
    if (popup_reorders_rows) {
        popup_reorders_rows = false;
        NetPlayer copy = active->players_[0];
        active->players_[0] = active->players_[2]; active->players_[2] = copy;
    }
    return caption_choice;
}
int pop_caption(char *, char *caption, int) {
    captions.emplace_back(caption);
    if (std::strcmp(caption, "NET_ADJUSTTOLERANCE") == 0) return tolerance_choice;
    if (popup_changes_rtt) {
        popup_changes_rtt = false;
        active->players_[1].field_C_ = 7000;
        return 0;
    }
    if (popup_drops_interface) { NetDirectPlay = 0; return 0; }
    return caption_choice;
}
char StringTemp[2048];
int ExitTurnLoop;
int get_sound_version() { return sound_version; }
static SoundDeviceInterface *sound_device(void *p) {
    return static_cast<SoundDeviceInterface *>(p);
}
StringAllocationBase::~StringAllocationBase() { StringAllocationHeap = allocation_owner_; }
SoundFadeState::SoundFadeState() {}
Sound::Sound() { device_ = 0; }
Sound::~Sound() {}
VoiceRx::VoiceRx() {}
VoiceRx::~VoiceRx() {}
VoiceTx::VoiceTx() {}
VoiceTx::~VoiceTx() {}
SoundDeviceInterface::~SoundDeviceInterface() {}
NetTextListBase::NetTextListBase() {
    head_ = current_ = 0; entry_count_ = current_position_ = 0; allocator_ = 0;
}
NetTextListBase::~NetTextListBase() {}
NetUnk72C::NetUnk72C() {}
NetUnk72C::~NetUnk72C() {}
NetAllocationNode::NetAllocationNode() {}
NetAllocationNode::~NetAllocationNode() {}
void NetAllocationNode::operator delete(void *p) { std::free(p); }
int JackalVoiceRx::on_insert(void *) { return 0; }
void JackalVoiceRx::on_remove(void *) { callback_order += 'R'; }
void JackalVoiceRx::clear() { assert(head_ == 0); }
Net::Net() {
    memset(players_, 0, sizeof(players_));
    voice_flags_D8_ = 0; field_E4_ = 1; field_6DC_ = 0; field_6E4_ = 32;
    field_6FC_ = 100; local_player_id_ = host_player_id_ = 99; poll_timeout_E0_ = 20000;
    thread_info_.owner_ = this; thread_info_.buffer_ = 0; thread_info_.direct_play_ = &provider;
    thread_info_.thread_id_ = 9;
    for (int i = 0; i < 5; ++i) thread_info_.events_[i] = 10 + i;
}
Net::~Net() {}
void Net::on_net_event_0(int, int) {}
void Net::on_player_added(char *, unsigned long) {}
void Net::on_player_deleted(char *name, unsigned long key) {
    removed_name = name; removed_id = static_cast<DWORD>(key); callback_order += 'D';
}
void Net::on_player_name_changed(char *, unsigned long) {}
void Net::on_session_lost() {}
void Net::on_host_changed() { callback_order += 'H'; }
void Net::check_polling() {}
void Net::handle_sys_msg() {}
void Net::close() { ++closes; callback_order += 'C'; }
char *Net::get_player_name(unsigned long key) {
    for (int i = 0; i < 16; ++i) if (players_[i].field_0_ == key) return players_[i].name_and_tail_;
    return 0;
}
'''


def host_source(damage: str | None = None) -> str:
    net = (ROOT / "src/net_class.cpp").read_text()
    fifo = (ROOT / "src/net_fifo.cpp").read_text()
    nh = (ROOT / "src/net_class.h").read_text()
    sh = (ROOT / "src/sound.h").read_text()
    vh = (ROOT / "src/voice.h").read_text()
    sound_abi = (ROOT / "src/soundx_sound_abi.h").read_text()
    ah = (ROOT / "src/stringstruct.h").read_text()
    declarations = [
        declaration(ah, "class StringAllocationBase {"),
        declaration(sh, "class SoundFadeState {"),
        declaration(sh, "class Sound :"),
        declaration(sound_abi, "class SoundDeviceInterface {"),
        declaration(vh, "class VoiceRxDeviceInterface :"),
        declaration(vh, "class VoiceRx :"),
        declaration(vh, "class VoiceTx :"),
        declaration(nh, "class VoiceRxAllocation :"),
        declaration(nh, "struct VoiceReceiverEntry :"),
        declaration(nh, "class JackalVoiceRx :"),
        declaration(nh, "class NetTextPayload :"),
        declaration(nh, "struct NetTextListEntry :"),
        declaration(nh, "class NetTextListBase :"),
        declaration(nh, "class NetUnk72C :"),
        declaration(nh, "class NetAllocationNode :"),
        declaration(nh, "class NetPlayer {"),
        declaration(nh, "struct NetThreadInfo {"),
        declaration((ROOT / "src/net_fifo.h").read_text(), "class NetFifo {"),
        "class AlphaNetLedger; class ServiceList; struct ServiceStruct;",
        declaration(nh, "class Net {"),
        declaration(net, "struct NetPacketEnvelope {"),
    ]
    # These dependency bodies are controlled product declarations/calls, not
    # claims about constructors or unscoped list/provider recovery.
    stubs = []
    for owner, header, anchor, omitted in (
        ("SoundFadeState", sh, "class SoundFadeState {", set()),
        ("Sound", sh, "class Sound :", {"is_playing"}),
        ("VoiceRx", vh, "class VoiceRx :", {"start", "stop", "get_nbuffers", "add_buffer"}),
        ("VoiceTx", vh, "class VoiceTx :", set()),
        ("NetTextListBase", nh, "class NetTextListBase :", set()),
        ("NetUnk72C", nh, "class NetUnk72C :", set()),
    ):
        stubs.append(default_virtuals(owner, declaration(header, anchor), omitted))
    device = []
    for header, anchor in ((sound_abi, "class SoundDeviceInterface {"),
                           (vh, "class VoiceRxDeviceInterface :")):
        for return_type, name, args in re.findall(
            r"virtual\s+(\w+(?:\s*\*)?)\s*(\w+)\s*\(([^()]*)\)\s*=\s*0;",
            uncomment(declaration(header, anchor)),
        ):
            if name in {"start", "stop", "get_nbuffers", "add_buffer", "is_playing"}:
                continue
            body = "" if return_type == "void" else "return 0;"
            device.append(f"{return_type} {name}({args}) override {{{body}}}")
    probe_device = r'''
struct ProbeDevice : VoiceRxDeviceInterface {
DEVICE_DEFAULTS
    int start() override { ++start_calls; return 0; }
    int stop() override { ++stop_calls; return 0; }
    int is_playing() override { return voice_playing; }
    int get_nbuffers() override {
        if (mutate_voice_count) active->field_6DC_ = 0x80000001U;
        return voice_level;
    }
    int add_buffer(_MMIOINFO *packet) override {
        ++add_buffer_calls;
        assert(packet->cchBuffer == 8 && packet->pchBuffer == reinterpret_cast<char *>(active_packet + 8));
        assert(packet->pchNext == packet->pchBuffer && packet->pchEndRead == packet->pchBuffer + 8);
        for (unsigned char c : packet->unused) assert(c == 0);
        return 0;
    }
};
'''.replace("DEVICE_DEFAULTS", "\n".join(device))
    definitions = "\n".join(braced(net, head) for head in NET_METHODS)
    definitions += "\n" + "\n".join(braced(fifo, head) for head in FIFO_METHODS)
    for head in (
        "NetFifo::NetFifo()", "NetFifo::~NetFifo()", "void NetFifo::close()",
        "int NetFifo::get(void *data, unsigned int *key, int *type,",
    ):
        definitions += "\n" + braced(fifo, head)
    for head in ("int VoiceRx::start()", "int VoiceRx::stop()", "int VoiceRx::get_nbuffers()",
                 "int VoiceRx::add_buffer(_MMIOINFO *buffer)"):
        definitions += "\n" + braced((ROOT / "src/voice.cpp").read_text(), head)
    definitions += "\n" + braced((ROOT / "src/sound.cpp").read_text(), "int Sound::is_playing()")
    if damage:
        old, new = DAMAGES[damage]
        assert old in definitions, damage
        definitions = definitions.replace(old, new, 1)
    return PREFIX + "\n".join(declarations) + SUPPORT + "\n".join(stubs) + probe_device + definitions + MAIN


MAIN = r'''
static void reset(Net &net) {
    active = &net; NetDirectPlay = &provider;
    clock_tick = 100; clock_step = 0; alloc_count = 0; fail_alloc_at = 0;
    alloc_changes_tail = alloc_changes_head = receive_changes_buffer = mutate_event_word = false;
    time_changes_receive_arguments = mutate_voice_count = false;
    popup_reorders_rows = popup_changes_rtt = popup_drops_interface = false;
    send_count = send_fail_at = wait_count = wait_mode = 0;
    caption_choice = tolerance_choice = closes = sound_version = 0;
    voice_level = voice_playing = stop_calls = start_calls = add_buffer_calls = 0;
    received_calls = 0; ExitTurnLoop = 0;
    deleted_voices = 0;
    timeouts.clear(); signals.clear(); captions.clear(); boxes.clear(); sent.clear();
    callback_order.clear(); removed_name.clear(); removed_id = 0;
    provider.receive_result = DP_OK; provider.incoming_from = 42; provider.incoming_to = 222;
}
static void incoming(Net &net, unsigned char *buffer, WORD type, DWORD sequence = 0, DWORD from = 42) {
    net.thread_info_.buffer_ = buffer; active_packet = buffer;
    provider.incoming.assign(20, 0x55);
    NetPacketEnvelope header = {type, 0xAAAA, sequence};
    memcpy(provider.incoming.data(), &header, sizeof(header));
    provider.incoming_from = from;
}
static DWORD word4(const std::vector<unsigned char> &bytes, size_t offset) {
    assert(offset + 4 <= bytes.size()); DWORD value;
    memcpy(&value, bytes.data() + offset, 4); return value;
}
static WORD word2(const std::vector<unsigned char> &bytes) {
    assert(bytes.size() >= 2); WORD value; memcpy(&value, bytes.data(), 2); return value;
}
static void fifo_controls() {
    Net net; reset(net);
    char data[] = "payload", output[32] = {};
    NetFifo queue;
    assert(queue.add(data, 7, 0x80000001U, 73, 0xFFFFFFFFU) == 0);
    assert(queue.head_ == queue.tail_ && queue.count_ == 1);
    assert(queue.head_->next_ == 0 && queue.head_->result_ == -1);
    assert(queue.add(data, 4, 17, 74, 123) == 0);
    assert(queue.head_->next_ == queue.tail_ && queue.tail_->next_ == 0);
    int length = -1, caller = -1;
    assert(queue.get(output, INT_MIN + 1, &caller, &length) == -1);
    assert(length == 7 && caller == 73 && memcmp(output, data, 7) == 0);
    assert(queue.head_ == queue.tail_ && queue.count_ == 1);
    unsigned int key = 0, size = 0;
    assert(queue.get(output, &key, &caller, &size) == 123);
    assert(key == 17 && size == 4 && caller == 74 && queue.head_ == 0);
    assert(queue.add(data, 2, 0, 0, 0) == 0);
    assert(queue.get(output, 0, 0, &length) == 0 && queue.head_ == 0);

    // Node allocation failures preserve the existing/empty linkage.
    NetFifo empty; alloc_count = 0; fail_alloc_at = 1;
    assert(empty.add(data, 2, 5, 0, 1) == 4 && empty.head_ == 0 && empty.count_ == 0);
    alloc_count = 0; fail_alloc_at = 0;
    assert(empty.add(data, 2, 5, 0, 1) == 0);
    auto *old = empty.tail_; alloc_count = 0; fail_alloc_at = 1;
    assert(empty.add(data, 2, 6, 0, 1) == 4);
    assert(empty.tail_ == old && old->next_ == 0 && empty.count_ == 1);
    NetFifo partial; alloc_count = 0; fail_alloc_at = 2;
    assert(partial.add(data, 2, 7, 9, 77) == 4);
    assert(partial.head_ == partial.tail_ && partial.head_->data_ == 0 && partial.count_ == 0);
    assert(partial.head_->result_ == 77 && partial.head_->next_ == 0);
    NetFifo append_partial; alloc_count = 0; fail_alloc_at = 0;
    assert(append_partial.add(data, 2, 20, 0, 80) == 0);
    auto *prefix = append_partial.head_; alloc_count = 0; fail_alloc_at = 2;
    assert(append_partial.add(data, 2, 21, 0, 81) == 4);
    assert(append_partial.head_ == prefix && prefix->next_ == append_partial.tail_);
    assert(append_partial.tail_->data_ == 0 && append_partial.tail_->result_ == 81);
    assert(append_partial.tail_->next_ == 0 && append_partial.count_ == 1);
    fail_alloc_at = 0; alloc_count = 0;
    NetFifo zero;
    assert(zero.add(0, 0, 8, 0, 91) == 0);
    assert(zero.head_->result_ == 91 && zero.head_->data_size_ == 0xA5A5A5A5U);
    const unsigned char *unwritten = reinterpret_cast<const unsigned char *>(&zero.head_->data_);
    for (size_t i = 0; i < sizeof(zero.head_->data_); ++i) assert(unwritten[i] == 0xA5);
    // Original zero-size data pointer bytes stay untouched; clear solely for
    // fixture cleanup rather than dereference an invalid pointer representation.
    zero.head_->data_ = 0;

    // The allocator changes the owner tail on the payload allocation. Original
    // add stores into the live tail, not a cached local node.
    NetFifo live;
    auto *alternate = static_cast<NetFifo::NetFifoNode *>(std::malloc(sizeof(NetFifo::NetFifoNode)));
    assert(alternate); memset(alternate, 0, sizeof(*alternate));
    alternate_tail = alternate; allocation_queue = &live;
    alloc_count = 0; alloc_changes_tail = true;
    assert(live.add(data, 3, 9, 0, 92) == 0);
    alloc_changes_tail = false;
    assert(live.tail_ == alternate && alternate->data_size_ == 3);
    assert(memcmp(alternate->data_, data, 3) == 0);
    live.head_->data_ = 0; // untouched original node backing is not a payload
    std::free(alternate->data_); std::free(alternate);
    live.tail_ = live.head_;

    // Empty/nonempty branch selection precedes the node allocator callback.
    NetFifo branch;
    alternate = static_cast<NetFifo::NetFifoNode *>(std::malloc(sizeof(NetFifo::NetFifoNode)));
    assert(alternate); memset(alternate, 0, sizeof(*alternate));
    alternate_tail = alternate; allocation_queue = &branch;
    alloc_count = 0; alloc_changes_head = true;
    assert(branch.add(data, 3, 22, 0, 82) == 0);
    alloc_changes_head = false;
    assert(branch.head_ == branch.tail_ && branch.head_ != alternate);
    assert(branch.head_->next_ == 0); std::free(alternate);

    // A fully backed payload aliases the real queue head. Native keyed get
    // reloads that owner field after memcpy; it does not unlink via cached cur.
    NetFifo aliased;
    NetFifo::NetFifoNode *placeholder = 0;
    assert(aliased.add(&placeholder, sizeof(placeholder), 30, 0, 83) == 0);
    assert(aliased.add(data, 2, 31, 0, 84) == 0);
    auto *second_node = aliased.tail_;
    assert(aliased.add(data, 2, 32, 0, 85) == 0);
    auto *third_node = aliased.tail_;
    memcpy(aliased.head_->data_, &third_node, sizeof(third_node));
    assert(aliased.get(&aliased.head_, 30, 0, 0) == 83);
    assert(aliased.head_ == 0 && aliased.count_ == 2);
    // The original alias path orphans these two still-backed nodes.
    std::free(second_node->data_); std::free(second_node);
    std::free(third_node->data_); std::free(third_node);
}
static void scalar_and_stamp_controls() {
    Net net; reset(net); char payload[] = "scalar";
    assert(net.queue_10c_.add(payload, 6, 0x80000001U, 0, 101) == 0);
    int size = -1;
    assert(net.sub_630a40(INT_MIN + 1, &size) == NetGetScratch && size == 6);
    assert(memcmp(NetGetScratch, payload, 6) == 0 && net.queue_10c_.head_ == 0);
    assert(net.queue_10c_.add(payload, 2, 17, 0, 102) == 0);
    assert(net.read_queue_key_until_timeout(17, &size) == NetGetScratch && size == 2);
    assert(net.sub_630a40(0, &size) == 0);
    assert(net.read_queue_key_until_timeout(0, &size) == 0);
    assert(net.queue_10c_.add(payload, 2, 0, 0, 103) == 0);
    unsigned int key = 88, unsigned_size = 0;
    assert(net.queue_10c_.get(NetGetScratch, &key, 0, &unsigned_size) == 103 && key == 0);
    assert(net.queue_10c_.add(payload, 2, 18, 0, 104) == 0);
    clock_step = 100;
    assert(net.read_queue_key_until_timeout(19, &size) == 0);
    assert(net.queue_10c_.count_ == 1);
    NetDirectPlay = 0;
    assert(net.sub_630a40(18, &size) == 0);
    NetDirectPlay = &provider;
    net.players_[15].field_0_ = 42;
    net.players_[15].field_4_ = 0x7FFFFFFFU;
    net.players_[15].stamp_10_ = 0xF0F0F0F0U;
    assert(net.check_message(42, 0x7FFFFFFFU) == 1);
    assert(net.check_message(42, 0x7FFFFFFEU) == 1);
    assert(net.check_message(42, 0x80000000U) == 0);
    assert(net.players_[15].field_4_ == 0x80000000U);
    assert(net.players_[15].stamp_10_ == 0xF0F0F0F0U);
    assert(net.check_message(42, 0xFFFFFFFFU) == 0);
    assert(net.check_message(42, 0) == 1 && net.check_message(43, 10) == 0);
}
static void receive_controls() {
    for (DWORD incoming_sequence : {100U, 99U, 101U, 0x80000000U}) {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        net.players_[15].field_0_ = 42; net.players_[15].field_4_ = 100;
        net.players_[15].stamp_10_ = 777;
        incoming(net, buffer, 4, incoming_sequence);
        assert(net.thread_info_.internal_receive() == 1);
        bool newer = incoming_sequence > 100;
        assert(net.queue_10c_.count_ == (newer ? 1U : 0U));
        assert(net.players_[15].field_4_ == (newer ? incoming_sequence : 100));
        assert(net.players_[15].stamp_10_ == 777 && send_count == 1);
        assert(word2(sent[0]) == 2 && sent[0].size() == 12);
        assert(signals.size() == (newer ? 1U : 0U));
    }
    for (DWORD sequence : {99U, 101U}) {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        net.players_[0].field_0_ = 42; net.players_[0].field_4_ = 100;
        incoming(net, buffer, 5, sequence);
        assert(net.thread_info_.internal_receive() == 1);
        // Original type1|reliable first takes the ordinary queue arm, then
        // independently takes the sequence-gated queue arm. Do not deduplicate.
        assert(net.queue_10c_.count_ == (sequence > 100 ? 2U : 1U));
        assert(net.queue_10c_.head_->result_ == -1 && word2(sent.back()) == 3);
        if (sequence > 100) assert(net.queue_10c_.tail_->result_ == 100);
    }
    {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {}, replacement[32] = {};
        net.players_[0].field_0_ = 42; net.players_[0].field_4_ = 20;
        replacement_buffer = replacement; receive_changes_buffer = true;
        incoming(net, buffer, 4, 21);
        reinterpret_cast<NetPacketEnvelope *>(replacement)->type_flags_ = 0x10;
        mutate_event_word = true;
        assert(net.thread_info_.internal_receive() == 1);
        assert(provider.received_buffer == buffer && net.thread_info_.buffer_ == replacement);
        assert(net.queue_10c_.count_ == 1 && net.queue_130_.count_ == 0);
        assert(net.queue_10c_.head_->data_size_ == 8 && net.queue_10c_.head_->key_ == 42);
        assert(net.queue_10c_.head_->result_ == 100);
        assert(word2(sent[0]) == 0x102 && sent_from == 77 && sent_to == 222);
        assert(word4(sent[0], 4) == 21);
    }
    {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        incoming(net, buffer, 4, 1); time_changes_receive_arguments = true;
        assert(net.thread_info_.internal_receive() == 1);
        assert(net.queue_10c_.head_->key_ == 84 && net.queue_10c_.head_->data_size_ == 4);
        assert(net.queue_10c_.head_->result_ == 100);
        send_fail_at = 2;
        incoming(net, buffer, 4, 2);
        assert(net.thread_info_.internal_receive() == 1); // reply failure ignored
    }
    for (WORD type : {WORD(1), WORD(0x10), WORD(0x80)}) {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        incoming(net, buffer, type);
        assert(net.thread_info_.internal_receive() == 1);
        if (type == 1) assert(net.queue_10c_.head_->data_size_ == 8);
        else assert(net.queue_130_.head_->data_size_ == 20 && callback_order == "P");
    }
    {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        incoming(net, buffer, 1, 0, 0);
        assert(net.thread_info_.internal_receive() == 1 && net.queue_130_.count_ == 1);
        provider.receive_result = 1;
        assert(net.thread_info_.internal_receive() == 0);
        int before = received_calls; NetDirectPlay = 0;
        assert(net.thread_info_.internal_receive() == 0 && received_calls == before);
    }
    for (uint32_t count : {199U, 200U, 0x80000000U}) {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        net.queue_10c_.count_ = count; incoming(net, buffer, 1);
        assert(net.thread_info_.internal_receive() == 1);
        bool accepted = static_cast<int32_t>(count) < 200;
        assert((net.queue_10c_.head_ != 0) == accepted);
    }
    {
        Net net; reset(net); alignas(16) unsigned char buffer[32] = {};
        net.players_[0].field_0_ = 42; net.players_[0].field_8_ = 1;
        net.players_[1].field_0_ = 43; net.players_[1].field_8_ = 1;
        net.players_[2].field_0_ = 0; net.players_[2].field_8_ = 1;
        net.field_E4_ = 8;
        incoming(net, buffer, 2, 7);
        assert(net.thread_info_.internal_receive() == 1 && net.players_[0].field_8_ == 1 && signals.empty());
        incoming(net, buffer, 2, 8);
        assert(net.thread_info_.internal_receive() == 1 && net.players_[0].field_8_ == 0 && signals.empty());
        incoming(net, buffer, 2, 8, 43);
        assert(net.thread_info_.internal_receive() == 1 && signals.size() == 1 && signals[0] == 10);
        net.players_[0].field_4_ = 456;
        incoming(net, buffer, 0x24, 8);
        assert(net.thread_info_.internal_receive() == 1);
        assert(net.players_[0].stamp_10_ == 100 && net.players_[0].field_4_ == 456);
        assert(word2(sent.back()) == 0x22);
    }
}
static void voice_controls() {
    for (bool known : {false, true}) {
        Net net; reset(net); ProbeDevice device;
        auto *voice = new VoiceRxAllocation; voice->device_ = &device;
        auto *entry = new VoiceReceiverEntry;
        entry->id_ = 42; entry->payload_ = voice; entry->next_ = entry->previous_ = entry;
        net.jackal_voice_rx_.head_ = net.jackal_voice_rx_.cursor_ = entry;
        net.jackal_voice_rx_.count_ = 1;
        if (known) { net.players_[0].field_0_ = 42; net.players_[0].field_4_ = 0; }
        net.voice_flags_D8_ = 0x40000000U;
        alignas(16) unsigned char buffer[32] = {}; incoming(net, buffer, 0x200, 0);
        assert(net.thread_info_.internal_receive() == 1);
        assert(start_calls == (known ? 0 : 1) && stop_calls == 0);
        assert(net.voice_flags_D8_ == (known ? 0x40000000U : 0));
        if (known) {
            incoming(net, buffer, 0x204, 1);
            assert(net.thread_info_.internal_receive() == 1);
            assert(start_calls == 1 && net.players_[0].field_4_ == 1 && word2(sent.back()) == 0x202);
        }
        net.jackal_voice_rx_.head_ = net.jackal_voice_rx_.cursor_ = 0;
        delete voice; delete entry;
    }
    for (int version : {0, 1}) for (int level : {4, 5, 15, 16}) {
        Net net; reset(net); ProbeDevice device;
        auto *voice = new VoiceRxAllocation; voice->device_ = &device;
        auto *entry = new VoiceReceiverEntry;
        entry->id_ = 42; entry->payload_ = voice; entry->next_ = entry->previous_ = entry;
        net.jackal_voice_rx_.head_ = net.jackal_voice_rx_.cursor_ = entry;
        net.jackal_voice_rx_.count_ = 1; net.field_6DC_ = 2;
        voice_level = level; sound_version = version;
        alignas(16) unsigned char buffer[32] = {}; incoming(net, buffer, 0x100);
        assert(net.thread_info_.internal_receive() == 1 && add_buffer_calls == 1);
        assert(start_calls == (level >= (version ? 5 : 16) ? 1 : 0));
        voice_playing = 1;
        assert(net.thread_info_.internal_receive() == 1 && add_buffer_calls == 2);
        assert(start_calls == (level >= (version ? 5 : 16) ? 1 : 0));
        voice_level = 2; mutate_voice_count = true;
        assert(net.thread_info_.internal_receive() == 1 && add_buffer_calls == 3);
        // (0x80000001-1)*2 wraps to zero before signed comparison.
        assert(net.field_6DC_ == 0x80000001U);
        net.jackal_voice_rx_.head_ = net.jackal_voice_rx_.cursor_ = 0;
        delete voice; delete entry;
    }
}
static void player(Net &net, int row, DWORD id, const char *name) {
    net.players_[row].field_0_ = id; net.players_[row].flags_14_ = 2;
    net.players_[row].field_C_ = 1000; strcpy(net.players_[row].name_and_tail_, name);
}
static void removal_controls() {
    {
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        net.local_player_id_ = 11; net.host_player_id_ = 99; net.field_6DC_ = 2;
        caption_choice = 1;
        assert(net.drop_player(10, 0) == 0);
        assert(captions.size() == 1 && captions[0] == "NET_OKTODROPHOST");
    }
    for (int answer : {-1, 0, 1, 2, 3}) {
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        net.host_player_id_ = 100; net.field_6DC_ = 2; caption_choice = answer;
        assert(net.drop_player(10, 0) == (answer == 0 ? 1 : 0));
        assert(captions[0] == "NET_OKTODROPHOSTS");
        assert(captions.size() == (answer == 2 ? 2U : 1U));
        assert(net.field_6DC_ == 2 && net.players_[0].field_0_ == 10);
        assert(ExitTurnLoop == (answer == 0 ? 1 : 0));
    }
    {
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        player(net, 2, 12, "twelve"); net.field_6DC_ = 3;
        popup_reorders_rows = true;
        assert(net.drop_player(10, 0) == 1);
        assert(net.players_[0].field_0_ == 12 && net.players_[1].field_0_ == 11);
        assert(net.players_[2].field_0_ == 0 && net.field_6DC_ == 2);
        assert(removed_name == "ten" && removed_id == 10);
        assert(sent.size() == 1 && word2(sent[0]) == 0x84 && word4(sent[0], 8) == 10);
    }
    {
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        net.host_player_id_ = 10; net.field_6DC_ = 2;
        assert(net.drop_player(10, 1) == 1);
        assert(NetHostChangeSlot == 10 && net.host_player_id_ == 11 && callback_order == "HD");
        assert(sent.empty() && net.players_[0].field_0_ == 11);
    }
    for (int tolerance = 0; tolerance < 5; ++tolerance) {
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        caption_choice = 2; tolerance_choice = tolerance;
        assert(net.drop_player(10, 0) == 0);
        DWORD wanted[] = {5000, 10000, 20000, 30000, 20000};
        assert(net.poll_timeout_E0_ == wanted[tolerance]);
    }
    {
        struct CountingVoice : VoiceRxAllocation {
            ~CountingVoice() override { ++deleted_voices; }
        };
        Net net; reset(net); player(net, 0, 10, "ten"); player(net, 1, 11, "eleven");
        net.field_6DC_ = 2;
        auto *first = new VoiceReceiverEntry; auto *second = new VoiceReceiverEntry;
        first->id_ = 10; second->id_ = 11;
        first->payload_ = new CountingVoice; second->payload_ = new CountingVoice;
        first->next_ = first->previous_ = second;
        second->next_ = second->previous_ = first;
        net.jackal_voice_rx_.head_ = net.jackal_voice_rx_.cursor_ = first;
        net.jackal_voice_rx_.count_ = 2;
        assert(net.drop_player(10, 1) == 1 && deleted_voices == 1);
        assert(net.jackal_voice_rx_.head_ == second && net.jackal_voice_rx_.cursor_ == second);
        assert(second->next_ == second && second->previous_ == second);
        assert(net.jackal_voice_rx_.count_ == 1 && net.jackal_voice_rx_.cursor_index_ == 0);
        assert(callback_order == "DR");
        assert(net.drop_player(11, 1) == 1 && deleted_voices == 2);
        assert(net.jackal_voice_rx_.head_ == 0 && net.jackal_voice_rx_.count_ == 0);
        // Native leaves a dangling cursor but sets index to count-1.
        assert(net.jackal_voice_rx_.cursor_ == second && net.jackal_voice_rx_.cursor_index_ == -1);
        assert(callback_order == "DRDR" && net.field_6DC_ == 0 && sent.empty());
    }
}
static void sender_controls() {
    {
        Net net; reset(net); char payload = 'x';
        player(net, 0, 10, "first"); player(net, 1, 10, "last");
        net.players_[1].field_C_ = 2000;
        wait_mode = 1; popup_changes_rtt = true;
        assert(net.send_packet_type(&payload, 1, 10, 1, 1) == 2);
        assert(timeouts.size() == 2 && timeouts[0] == 1000 && timeouts[1] == 1000);
        assert(net.players_[0].field_C_ == 1000 && net.players_[1].field_C_ == 3560);
        assert(word4(sent[0], 4) == 2 && word2(sent[0]) == 5);
    }
    for (int choice : {1, 2, 3}) {
        Net net; reset(net); char payload = 'x'; player(net, 0, 10, "peer");
        caption_choice = choice; wait_mode = 3;
        assert(net.send_packet_type(&payload, 1, 0, 1, 1) == 0);
        assert(boxes.size() == 1 && boxes[0] == "Oh no!! Send failed after many tries!");
        assert(closes == (choice == 1 ? 1 : 0) && ExitTurnLoop == (choice == 2 ? 1 : 0));
        if (choice == 1) assert(callback_order.size() >= 2 && callback_order.substr(callback_order.size() - 2) == "CM");
    }
    {
        Net net; reset(net); char payload = 'x'; player(net, 0, 10, "peer");
        wait_mode = 2; caption_choice = 0; send_fail_at = 3;
        assert(net.send_packet_type(&payload, 1, 10, 1, 1) == 0);
        assert(send_count == 3 && boxes.size() == 1 && boxes[0] == "Oh no!! Send failed!");
    }
    {
        Net net; reset(net); char payload = 'x'; player(net, 0, 10, "peer");
        wait_mode = 3; popup_drops_interface = true;
        assert(net.send_packet_type(&payload, 1, 0, 1, 1) == 0 && boxes.empty());
        NetDirectPlay = &provider;
        reset(net); assert(net.send_packet_type(&payload, 1, 10, 0x200, 0) == 2);
        assert(word4(sent[0], 4) == 0 && word2(sent[0]) == 0x200);
        assert(sent[0][2] == 0xA5 && sent[0][3] == 0xA5 && sent[0].back() == 0xA5);
    }
}
int main() {
    static_assert(sizeof(NetPlayer) == 0x58, "actual complete sixteen-row element");
    static_assert(sizeof(((Net *)0)->players_) / sizeof(NetPlayer) == 16, "actual owner array");
    fifo_controls(); scalar_and_stamp_controls(); receive_controls(); voice_controls();
    removal_controls(); sender_controls();
}
'''

DAMAGES = {
    "fifo_link_result": ("tail_->result_ = static_cast<int>(extra);", "tail_->next_ = reinterpret_cast<NetFifoNode *>(extra);"),
    "fifo_wrong_success": ("++count_;\n  LeaveCriticalSection(&lock_);\n  return 0;", "++count_;\n  LeaveCriticalSection(&lock_);\n  return static_cast<int>(extra);"),
    "fifo_cached_payload": ("tail_->data_ = payload;", "node->data_ = payload;"),
    "scalar_wrong_key": ("cur->key_ != static_cast<uint32_t>(key)", "cur->key_ != static_cast<uint32_t>(key + 1)"),
    "scalar_wrapper_drop_key": ("queue_10c_.get(NetGetScratch, key, 0, length_out)", "queue_10c_.get(NetGetScratch, key + 1, 0, length_out)"),
    "wrong_stamp_word": ("stamp <= record->field_4_", "stamp <= record->stamp_10_"),
    "voice_reverse_sequence": ("net->players_[sequence_player_index].field_4_ >= sequence", "sequence >= net->players_[sequence_player_index].field_4_"),
    "data_reverse_sequence": ("net->players_[ack_player_index].field_4_ >= sequence", "sequence >= net->players_[ack_player_index].field_4_"),
    "post_receive_buffer": ("NetPacketEnvelope *const packet =\n        reinterpret_cast<NetPacketEnvelope *>(bytes);", "NetPacketEnvelope *const packet =\n        reinterpret_cast<NetPacketEnvelope *>(buffer_);"),
    "cached_ack_word": ("packet->type_flags_ = (packet->type_flags_ & 0xFFFB) | 2;", "packet->type_flags_ = (type_flags & 0xFFFB) | 2;"),
    "unsigned_queue_count": ("static_cast<int32_t>(net->queue_10c_.count_) < 0xC8", "net->queue_10c_.count_ < 0xC8"),
    "wrong_voice_stop_slot": ("voice->start();\n                            break;", "voice->stop();\n                            break;"),
    "nonwrapped_voice_load": ("const uint32_t queued_load = (net->field_6DC_ - 1U)\n                        * static_cast<uint32_t>(level);", "const uint32_t queued_load = (static_cast<int32_t>(net->field_6DC_) - 1) * level;"),
    "cached_keyed_unlink": ("head_ = head_->next_;", "head_ = cur->next_;"),
    "drop_key_gate": ("local_player_id_ != host_player_id_ && local_player_id_ != players_[1].field_0_", "local_player_id_ != host_player_id_ && key != players_[1].field_0_"),
    "drop_negative_modal": ("if (answer != 2)", "if (answer == 1 || answer > 2)"),
    "drop_stale_row": ("index = 0;\n    while (index < 0x10 && players_[index].field_0_ != key) ++index;", "// stale original presence-check index retained"),
    "drop_wrong_notice": ("send_packet_type(&key, 4, 0, 0x80, 1);", "unsigned long wrong = 0; send_packet_type(&wrong, 4, 0, 0x80, 1);"),
    "sender_last_rtt": ("if (peer.field_C_ < wait_timeout) wait_timeout = peer.field_C_;", "wait_timeout = peer.field_C_;"),
    "sender_live_rtt": ("// 62FA55 captures the minimum eligible RTT once.", "wait_timeout = players_[selected].field_C_; // 62FA55 captures the minimum eligible RTT once."),
    "sender_no_final_box": ("MessageBoxA(0, \"Oh no!! Send failed after many tries!\",\n                            \"Message from the Net Underworld\", 0);", "// final failure notification removed"),
}


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, *DAMAGES))
def test_actual_complete_transport_owners(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    source = host_source(damage)
    cpp = tmp_path / "transport.cpp"
    cpp.write_text(source)
    executable = tmp_path / "transport"
    built = subprocess.run([compiler, "-std=c++11", "-fsanitize=undefined",
                            "-fno-sanitize-recover=all", str(cpp), "-o", str(executable)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=15, preexec_fn=no_core)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr
    if damage:
        assert "assert" in result.stderr.lower() or "runtime error" in result.stderr.lower(), result.stderr


def test_pinned_complete_transport_native_spans():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned executable unavailable; native evidence not checked")
    assert path.is_absolute(), "native evidence requires an absolute image path"
    image = PeImage(path.read_bytes(), PIN)
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end, count in (
        (0x6327E0, 0x632CB3, 381), (0x632FA0, 0x633007, 35),
        (0x632350, 0x632716, 314), (0x630A40, 0x630A7D, 20),
        (0x630B00, 0x630B7C, 48), (0x633AB0, 0x633B9A, 88),
        (0x633D90, 0x633E75, 99), (0x62F8A0, 0x63007F, 602),
    ):
        rows = list(disassembler.disasm(image.read(Span(start, end)), start))
        assert len(rows) == count and rows[-1].address + rows[-1].size == end
        instructions.update({row.address: (row.mnemonic, row.op_str) for row in rows})
    expected = {
        0x6327FD: ("mov", "ebx, dword ptr [edi + 0x20]"),
        0x632834: ("call", "dword ptr [edx + 0x64]"),
        0x632985: ("jae", "0x632b2b"), 0x632B98: ("jae", "0x632bcc"),
        0x632A13: ("call", "dword ptr [edx + 0x88]"),
        0x632AD9: ("imul", "eax, ebp"), 0x632AE1: ("jge", "0x632af2"),
        0x6328C1: ("jge", "0x632b2b"),
        0x632BCC: ("mov", "ax, word ptr [ebx]"),
        0x632FED: ("lea", "ecx, [ecx + eax*8 + 0x158]"),
        0x632FF6: ("jb", "0x633000"),
        0x633B18: ("mov", "dword ptr [eax + 0x14], 0"),
        0x633B3B: ("mov", "dword ptr [edx + 8], eax"),
        0x633B94: ("xor", "eax, eax"), 0x633B97: ("ret", "0x14"),
        0x633DBC: ("mov", "eax, dword ptr [esp + 0x18]"),
        0x633DC0: ("cmp", "dword ptr [esi + 4], eax"),
        0x630A66: ("push", "eax"), 0x630B40: ("push", "ebp"),
        0x6323AF: ("cmp", "eax, dword ptr [ebx + 0x1ac]"),
        0x63259C: ("xor", "esi, esi"),
        0x6326F4: ("lea", "ecx, [esp + 0x11c]"),
        0x62FA38: ("cmp", "edx, edi"), 0x62FA55: ("mov", "dword ptr [esp + 0x1c], edi"),
        0x62FADA: ("mov", "edx, dword ptr [esp + 0x1c]"),
        0x630010: ("call", "dword ptr [0x669318]"),
        0x62FC26: ("mov", "edx, dword ptr [eax]"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction, hex(address)
    for table in (0x670F48, 0x66E82C):
        assert struct.unpack("<I", image.read(Span(table + 0x88, table + 0x8C)))[0] == 0x4C8C20
        assert struct.unpack("<I", image.read(Span(table + 0x20, table + 0x24)))[0] == 0x4C8C40
    assert struct.unpack("<8I", image.read(Span(0x632718, 0x632738))) == (
        0x63252F, 0x632547, 0x63255F, 0x632577,
        0x63252F, 0x632547, 0x63255F, 0x632577)


def test_coordinator_observed_scalar_bindings_and_actual_declarations():
    # Both configured VC6 profiles were observed before the coordinator used
    # source.writer.write_symbol. These are observed facts, not guessed hints.
    from match_recovery.source import read_file
    rows = {row.address: row for name in ("net_class.cpp", "net_fifo.cpp")
            for row in read_file(ROOT / "src" / name)}
    for address, symbol in {
        0x630A40: "?sub_630a40@Net@@QAEPAXHPAH@Z",
        0x630B00: "?read_queue_key_until_timeout@Net@@QAEPAXHPAH@Z",
        0x633D90: "?get@NetFifo@@QAEHPAXHPAH1@Z",
    }.items():
        assert rows[address].symbol == symbol
    assert rows[0x630A40].byte_exact  # retained historical restoration obligation
    net = (ROOT / "src/net_class.h").read_text()
    fifo = (ROOT / "src/net_fifo.h").read_text()
    assert "void *sub_630a40(int key, int *length_out);" in net
    assert "void *read_queue_key_until_timeout(int key, int *length_out);" in net
    assert "int get(void *data, int key, int *type, int *len);" in fifo
    assert "int get(void *data, unsigned int *key, int *type, unsigned int *len);" in fifo


def scope_bridge(candidate: dict[str, str], baseline: dict[str, str]) -> None:
    """Exact source scope, separate from any old whole-TU acceptance claim."""
    from match_recovery.source import read_text
    for file_name in ("src/net_class.cpp", "src/net_fifo.cpp"):
        before = read_text(baseline[file_name], ROOT / file_name)
        after = read_text(candidate[file_name], ROOT / file_name)
        def identity(row):
            return (row.address, row.name, row.image_spans, row.byte_exact,
                    row.semantic, row.source_role, row.source_role_reason, row.function_fragment,
                    row.mode, row.state, row.body, row.kind, row.exclusion,
                    row.levers, row.ruled_out, row.unrecoverable, row.deferred,
                    tuple((lesson.kind, lesson.key, lesson.prose,
                           tuple(text for _, text in lesson.lines)) for lesson in row.origin))
        assert [identity(row) for row in before] == [identity(row) for row in after]
        text = candidate[file_name]
        if file_name.endswith("net_fifo.cpp"):
            # Reverse only the later authorized two-head closure against its
            # own 908fd8cdc baseline before retaining this original 58563 guard.
            from test_net_fifo_live_head_oct12 import restore_live_head
            text = restore_live_head(text)
        heads = NET_METHODS if file_name.endswith("net_class.cpp") else FIFO_METHODS
        for head in heads:
            old_head = head
            if head.startswith("void *Net::sub_630a40("):
                old_head = "void *Net::sub_630a40(unsigned int *key, int *length_out)"
            elif head.startswith("void *Net::read_queue_key_until_timeout("):
                old_head = "void *Net::read_queue_key_until_timeout(unsigned int *key, int *length_out)"
            elif head.startswith("int NetFifo::get("):
                old_head = "int NetFifo::get(void *data, unsigned int *key, int *type, int *len)"
            body = braced(text, head)
            assert text.count(body) == 1
            text = text.replace(body, braced(baseline[file_name], old_head), 1)
        if file_name.endswith("net_fifo.cpp"):
            evidence = "// 633DBC/633DC0 compares the raw second argument DWORD, not its pointee.\n"
            assert text.count(evidence) == 1
            text = text.replace(evidence, "", 1)
        before_symbols = {row.address: row.symbol for row in before}
        for row in after:
            if row.address in (0x630A40, 0x630B00, 0x633D90):
                text = text.replace("// symbol    " + row.symbol,
                                    "// symbol    " + before_symbols[row.address], 1)
            else:
                assert row.symbol == before_symbols[row.address]
        assert text == baseline[file_name], "outside authorized function bodies/symbols changed"
    for file_name in ("src/net_class.h", "src/net_fifo.h"):
        text = candidate[file_name]
        changes = (
            ("void *sub_630a40(int key, int *length_out);", "void *sub_630a40(unsigned int *key, int *length_out);"),
            ("void *read_queue_key_until_timeout(int key, int *length_out);", "void *read_queue_key_until_timeout(unsigned int *key, int *length_out);"),
        ) if file_name.endswith("net_class.h") else (
            ("int get(void *data, int key, int *type, int *len);", "int get(void *data, unsigned int *key, int *type, int *len);"),
        )
        for current, previous in changes:
            assert text.count(current) == 1
            text = text.replace(current, previous, 1)
        assert text == baseline[file_name], "outside scalar API declarations changed"


@pytest.mark.parametrize("damage", (None, "outside-body", "array-extent", "original-identity"))
def test_exact_task_source_scope_and_identity_controls(damage):
    task = json.loads((ROOT / "recovery/tasks/net-transport-queue-closure-oct12.json").read_text())
    assert task["base_revision"] == "58563bc456781d3224dca06e4d52c5175691a29e"
    assert {subject["address"] for subject in task["subjects"]} == {
        0x6327E0, 0x632FA0, 0x632350, 0x630A40,
        0x630B00, 0x633AB0, 0x633D90, 0x62F8A0,
    }
    files = ("src/net_class.cpp", "src/net_class.h", "src/net_fifo.cpp", "src/net_fifo.h")
    baseline = {name: subprocess.check_output(
        ["git", "show", task["base_revision"] + ":" + name], cwd=ROOT, text=True)
                for name in files}
    candidate = {name: (ROOT / name).read_text() for name in files}
    if damage == "outside-body":
        name = "src/net_class.cpp"
        old = braced(candidate[name], "int Net::send(void *data, int data_size, unsigned long target,")
        new = old.replace("return 0;", "return 1;", 1)
        assert new != old
        candidate[name] = candidate[name].replace(old, new, 1)
    elif damage == "array-extent":
        name = "src/net_class.h"
        candidate[name] = candidate[name].replace("NetPlayer players_[16];", "NetPlayer players_[17];", 1)
    elif damage == "original-identity":
        # Damage an existing canonical identity solely in memory through the
        # native source object's text; no product marker is written or demoted.
        name = "src/net_fifo.cpp"
        from match_recovery.source import read_text
        row = next(row for row in read_text(candidate[name], ROOT / name) if row.address == 0x633AB0)
        candidate[name] = candidate[name].replace(row.name, row.name + "_damaged", 1)
    if damage:
        with pytest.raises(AssertionError):
            scope_bridge(candidate, baseline)
    else:
        scope_bridge(candidate, baseline)

