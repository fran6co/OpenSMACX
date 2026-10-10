"""Execute the actual Net dispatch body; host controls do not prove target ABI.

SDK pointer fields use the host ABI here. Target offsets are separately checked
by the product static assertions and fresh VC6 observations. No original image
pointer is mapped into this harness.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]

PREFIX = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
using DWORD = std::uint32_t;
using DPID = DWORD;
using WORD = std::uint16_t;
struct DPNAME { DWORD dwSize, dwFlags; char *lpszShortNameA, *lpszLongNameA; };
struct DPMSG_GENERIC { DWORD dwType; };
struct DPMSG_CREATEPLAYERORGROUP {
    DWORD dwType, dwPlayerType; DPID dpId; DWORD dwCurrentPlayers;
    void *lpData; DWORD dwDataSize; DPNAME dpnName;
};
struct DPMSG_DESTROYPLAYERORGROUP {
    DWORD dwType, dwPlayerType; DPID dpId; void *lpLocalData;
};
struct DPMSG_SETPLAYERORGROUPDATA {
    DWORD dwType, dwPlayerType; DPID dpId; void *lpData; DWORD dwDataSize;
};
struct DPMSG_SETPLAYERORGROUPNAME {
    DWORD dwType, dwPlayerType; DPID dpId; DPNAME dpnName;
};
alignas(16) unsigned char NetGetScratch[0x2000];
'''

HARNESS = r'''
// Test-owned wire backing, not a fixed bound in the production receiver.
struct PeerWireMessage {
    NetPeerPlayerTableMessage header_;
    NetPeerPlayerRecord players_[32];
};
struct QueuedMessage { unsigned int key; std::vector<unsigned char> bytes; };
struct NetFifo {
    std::vector<QueuedMessage> incoming;
    unsigned int next = 0;
    int calls = 0;
    int get(void *out, unsigned int *key, int *type, unsigned int *size) {
        ++calls;
        assert(type == 0 && size == 0);
        if (next == incoming.size()) return 0;
        const QueuedMessage &message = incoming[next++];
        memcpy(out, message.bytes.data(), message.bytes.size());
        *key = message.key;
        return 1;
    }
    template<class T> void add(const T &message, unsigned int key = 0) {
        QueuedMessage queued;
        queued.key = key;
        const unsigned char *begin = reinterpret_cast<const unsigned char *>(&message);
        queued.bytes.assign(begin, begin + sizeof(message));
        incoming.push_back(queued);
    }
};
struct NetPlayer { DWORD field_0_ = 0; unsigned char flags_14_ = 0; };
struct Addition { char *name; DPID id; unsigned char flag; };
struct IDirectPlay2 {
    int calls = 0, result = 0;
    bool change_id = false;
    DPID requested = 0;
    char *response_name = 0;
    int GetPlayerName(DPID id, void *out, unsigned long *size) {
        ++calls; requested = id;
        assert(*size == 0x100 && out == NetPlayerNameRecord.bytes_);
        if (result == 0) {
            static_cast<DPNAME *>(out)->lpszLongNameA = response_name;
        }
        if (change_id) {
            reinterpret_cast<DPMSG_CREATEPLAYERORGROUP *>(NetGetScratch)->dpId = 88;
        }
        return result;
    }
};
IDirectPlay2 *NetDirectPlay = 0;
struct Net {
    NetFifo queue_130_;
    DWORD field_760_ = 77, field_764_ = 77;
    NetPlayer players_[16];
    std::vector<Addition> additions;
    std::vector<DPID> drops;
    std::vector<Addition> name_changes;
    std::string events;
    bool shrink_count = false;
    int sub_632cc0(char *name, int id, unsigned char flag) {
        additions.push_back({name, static_cast<DPID>(id), flag});
        if (shrink_count) {
            reinterpret_cast<NetPeerPlayerTableMessage *>(NetGetScratch)->count_ = 1;
        }
        return 0;
    }
    int drop_player(unsigned long id, int reason) {
        assert(reason == 1); drops.push_back(static_cast<DPID>(id)); return 0;
    }
    void on_session_lost() { events += 'L'; }
    void close() { assert(events == "L"); events += 'C'; }
    void on_player_name_changed(char *name, unsigned long id) {
        name_changes.push_back({name, static_cast<DPID>(id), 0});
    }
    void handle_sys_msg();
};
'''

MAIN = r'''
int main() {
    char name[] = "borrowed long name";
    unsigned char data = 0xA5;
    IDirectPlay2 provider;
    provider.response_name = name;
    NetDirectPlay = &provider;

    // Full-DWORD dispatch: changing data and name messages were unreachable
    // in the old BYTE switch. Unknown types with matching low bytes stay out.
    Net dispatch;
    dispatch.players_[3].field_0_ = 44;
    DPMSG_SETPLAYERORGROUPDATA update = {0x102, 0, 44, &data, 1};
    DPMSG_SETPLAYERORGROUPNAME rename = {};
    rename.dwType = 0x103; rename.dpId = 44; rename.dpnName.lpszLongNameA = name;
    dispatch.queue_130_.add(update);
    dispatch.queue_130_.add(rename);
    dispatch.handle_sys_msg();
    assert(dispatch.players_[3].flags_14_ == data);
    assert(dispatch.name_changes.size() == 1);
    assert(dispatch.name_changes[0].name == name && dispatch.name_changes[0].id == 44);

    Net ignored;
    update.dwType = 0x10102; ignored.players_[3].field_0_ = 44;
    ignored.queue_130_.add(update);
    DPMSG_CREATEPLAYERORGROUP create = {};
    create.dwType = 0x10003; create.dwPlayerType = 1; create.dpId = 77;
    ignored.queue_130_.add(create);
    ignored.handle_sys_msg();
    assert(ignored.players_[3].flags_14_ == 0 && provider.calls == 0);

    // Creation borrows the provider response and reloads dpId after the API.
    create.dwType = 3; create.lpData = &data;
    Net creation; provider.change_id = true;
    creation.queue_130_.add(create); creation.handle_sys_msg();
    assert(provider.requested == 77 && provider.calls == 1);
    assert(creation.additions.size() == 1);
    assert(creation.additions[0].name == name && creation.additions[0].id == 88);
    assert(creation.additions[0].flag == data);

    // Failed GetPlayerName is ignored, preserving the previous name buffer.
    Net failed; provider.result = -7; provider.change_id = false;
    create.lpData = 0;
    failed.queue_130_.add(create); failed.handle_sys_msg();
    assert(failed.additions.size() == 1 && failed.additions[0].name == name);
    assert(failed.additions[0].flag == 0);
    const int calls = provider.calls;
    Net gated; gated.field_764_ = 99;
    gated.queue_130_.add(create); gated.handle_sys_msg();
    create.dwPlayerType = 0;
    gated.field_764_ = gated.field_760_;
    gated.queue_130_.add(create); gated.handle_sys_msg();
    assert(gated.additions.empty() && provider.calls == calls);

    // Routed peer rows retain their shared-scratch name pointer and a fresh
    // count after every insertion callback, without adding a count clamp.
    Net peer; peer.shrink_count = true;
    PeerWireMessage table = {};
    table.header_.envelope_.type_flags_ = 0x10; table.header_.count_ = 2;
    table.players_[0].id_ = 55; strcpy(table.players_[0].name_, "first");
    table.players_[0].flags_ = 0x123456A5;
    table.players_[1].id_ = 66; strcpy(table.players_[1].name_, "second");
    peer.queue_130_.add(table, 9); peer.handle_sys_msg();
    assert(peer.additions.size() == 1);
    assert(peer.additions[0].id == 55 && peer.additions[0].flag == 0xA5);
    assert(strcmp(peer.additions[0].name, "first") == 0);
    assert(peer.additions[0].name == reinterpret_cast<char *>(NetGetScratch + 0x10));
    Net no_rows; table.header_.count_ = 0; no_rows.queue_130_.add(table, 9);
    no_rows.handle_sys_msg(); assert(no_rows.additions.empty());

    // The unchecked receiver can consume more rows than the local producer
    // normally sends, provided here by complete test-owned packet backing.
    Net oversized; table.header_.count_ = 17;
    for (int i = 0; i < 17; ++i) table.players_[i].id_ = i + 100;
    oversized.queue_130_.add(table, 9); oversized.handle_sys_msg();
    assert(oversized.additions.size() == 17 && oversized.additions.back().id == 116);
    Net low_count; table.header_.count_ = 0x101;
    low_count.queue_130_.add(table, 9); low_count.handle_sys_msg();
    assert(low_count.additions.size() == 1);

    Net priority; table.header_.envelope_.type_flags_ = 0x90; table.header_.count_ = 73;
    priority.queue_130_.add(table, 9); priority.handle_sys_msg();
    assert(priority.additions.empty() && priority.drops.size() == 1);
    assert(priority.drops[0] == 73);

    Net removal;
    DPMSG_DESTROYPLAYERORGROUP destroy = {5, 0, 44, 0};
    removal.queue_130_.add(destroy); destroy.dwPlayerType = 1;
    removal.queue_130_.add(destroy); removal.handle_sys_msg();
    assert(removal.drops.size() == 1 && removal.drops[0] == 44);
    DPMSG_GENERIC lost = {0x31};
    removal.queue_130_.add(lost); removal.handle_sys_msg();
    assert(removal.events == "LC");

    Net null_data;
    null_data.players_[3].field_0_ = 44; update.dwType = 0x102; update.lpData = 0;
    null_data.queue_130_.add(update); update.dpId = 100; update.lpData = &data;
    null_data.queue_130_.add(update); null_data.handle_sys_msg();
    assert(null_data.players_[3].flags_14_ == 0);
    rename.dpnName.lpszLongNameA = 0;
    null_data.queue_130_.add(rename); null_data.handle_sys_msg();
    assert(null_data.name_changes.size() == 1 && null_data.name_changes[0].name == 0);

    Net absent; NetDirectPlay = 0;
    absent.handle_sys_msg(); assert(absent.queue_130_.calls == 0);
}
'''


def _product() -> tuple[str, str]:
    text = (ROOT / "src/net_class.cpp").read_text()
    start = text.index("union NetPlayerNameBuffer {")
    response = text[start:text.index("// Original player insertion", start)]
    start = text.index("struct NetPacketEnvelope {")
    peer_types = text[start:text.index("static_assert(sizeof(NetPacketEnvelope)", start)]
    record, = [r for r in read_file(ROOT / "src/net_class.cpp") if r.address == 0x632DB0]
    body = _definition(record.region, "void Net::handle_sys_msg()")
    return PREFIX + response + peer_types + HARNESS, body


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "byte_type", "cached_count", "cached_id", "failed_api"])
def test_actual_dispatch_and_damage_controls(compiler: str, damage: str | None) -> None:
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    prefix, body = _product()
    if damage == "byte_type":
        body = body.replace("switch (generic->dwType)", "switch (NetGetScratch[0])")
    elif damage == "cached_count":
        body = body.replace("message->count_ & 0xFF", "2")
    elif damage == "cached_id":
        body = body.replace("NetPlayerNameRecord.name_.lpszLongNameA, message->dpId,",
                            "NetPlayerNameRecord.name_.lpszLongNameA, field_760_,")
    elif damage == "failed_api":
        body, count = re.subn(
            r"NetDirectPlay->GetPlayerName\(message->dpId,\s*NetPlayerNameRecord.bytes_, &size\);",
            "if (NetDirectPlay->GetPlayerName(message->dpId, NetPlayerNameRecord.bytes_, &size) != 0) break;",
            body,
        )
        assert count == 1
    if damage:
        assert body != _product()[1], "damage control did not change actual body"
    with tempfile.TemporaryDirectory(prefix="osmx-net-system-owner-") as temp:
        binary = str(Path(temp) / "control")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-x", "c++", "-", "-o", binary],
            input=prefix + body + MAIN, text=True, capture_output=True, timeout=30,
        )
        assert built.returncode == 0, built.stderr
        run = subprocess.run([binary], text=True, capture_output=True, timeout=10)
        assert (run.returncode == 0) == (damage is None), run.stderr
