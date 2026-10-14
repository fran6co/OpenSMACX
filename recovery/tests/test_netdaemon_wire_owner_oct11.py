"""Actual dispatch controls for pinned sequence ownership and five routes."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from test_caviar_draw_callbacks_actual_source import extract
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "a18f9cd7c43aaf731732104bd6db627154578f04"

PREAMBLE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
struct Console { int faction_id_; } ConsoleGlobal;
struct Vehicle { int faction_id; int order; } Vehs[32];
@@LOCK_DECLARATIONS@@
struct DeletionList {
    uint32_t value; int stores;
    void set_sequence_bits(uint32_t next) { value=next; ++stores; }
};
struct NetDaemon { PrimaryLockStateTail primary_state_; LockStateEntry channels_[2]; DeletionList deletion_list_; };
static NetDaemon NetDaemonNet;
static int SpockNotifyPending, order_calls, action_calls, fight_calls, drop_calls, cross_calls;
static int arg0, arg1, arg2, seed, logged_faction, acknowledged_faction;
static bool mutate_clock, mutate_cross;
static unsigned char *live_packet;
static std::vector<std::string> events;
static void put(int offset, int value) { std::memcpy(live_packet+offset,&value,4); }
static void log_say(const char *label, int a, int, int) {
    events.push_back(label);
    if (std::string(label).find("From Server:")==0) logged_faction=a;
    if (std::string(label).find("Acknowledging")==0) acknowledged_faction=a;
}
static void my_srand(int value) {
    seed=value; events.push_back("seed");
    if (mutate_clock) { put(4,3); put(0x10,2); put(0x14,19); put(0x18,23); }
}
static void message_data(int opcode,int,int,int,int,int) {
    assert(opcode==0x2308 || opcode==0x2214); events.push_back("message_data");
}
static void order_veh(int id,int direction,int mode) {
    assert(mode==2); ++order_calls; arg0=id; arg1=direction; events.push_back("order");
}
static void battle_fight(int id,int direction,int a,int b,void *p) {
    assert(a==1 && b==1 && !p); ++fight_calls; arg0=id; arg1=direction; events.push_back("fight");
}
static void action_airdrop(int id,int x,int y,int mode) {
    assert(mode==2); ++drop_calls; arg0=id; arg1=x; arg2=y; events.push_back("drop");
}
static void action(int id) { ++action_calls; arg0=id; events.push_back("action"); }
static void double_cross(int a,int b,int c) {
    ++cross_calls; arg0=a; arg1=b; arg2=c; events.push_back("cross");
    if (mutate_cross) { put(4,3); put(0x0c,777); }
}
'''

MAIN = r'''
static void reset(NetDaemon &daemon, unsigned char *packet) {
    std::memset(&daemon,0,sizeof daemon); std::memset(packet,0,32);
    std::memset(&NetDaemonNet,0,sizeof NetDaemonNet);
    live_packet=packet; put(4,7); put(8,123); put(0xc,456);
    put(0x10,1); put(0x14,11); put(0x18,13);
    ConsoleGlobal.faction_id_=7; Vehs[1].faction_id=7; Vehs[1].order=42;
    SpockNotifyPending=order_calls=action_calls=fight_calls=drop_calls=cross_calls=0;
    seed=logged_faction=acknowledged_faction=-1;
    mutate_clock=mutate_cross=false; events.clear();
}
int main() {
    NetDaemon daemon; alignas(4) unsigned char packet[32];
    // Server rejects a faction mismatch but still acknowledges the request.
    reset(daemon,packet); daemon.primary_state_.leading_words_[0]=1;
    NetDaemonNet.primary_state_.leading_words_[0]=1;
    Vehs[1].faction_id=2;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(SpockNotifyPending==1 && order_calls==0 && acknowledged_faction==7);
    assert(daemon.deletion_list_.stores==0);
    // A client sends the rejection notice, then continues with the order.
    reset(daemon,packet); Vehs[1].faction_id=2;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(order_calls==1 && arg0==1 && arg1==11 && SpockNotifyPending==0);
    assert(events[2]=="message_data" && events[3]=="order");
    assert(daemon.deletion_list_.value==456); // Not transport length or a reread.
    // Ordinary server execution still proceeds when the faction agrees.
    reset(daemon,packet); daemon.primary_state_.leading_words_[0]=1;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(order_calls==1);
    // Payload/faction reloads after RNG are different from entry-owned values.
    reset(daemon,packet); mutate_clock=true; Vehs[1].faction_id=3; put(0xc,999);
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(order_calls==1 && arg0==1 && arg1==19 && acknowledged_faction==7);
    assert(seed==123 && daemon.deletion_list_.value==456);
    reset(daemon,packet); mutate_clock=true;
    assert(netdaemon_handle_five_message_routes(daemon,0x4402,packet,7,456));
    assert(fight_calls==1 && arg0==1 && arg1==19);
    assert(NetDaemonNet.channels_[0].leading_words_[3]==2 && daemon.channels_[0].leading_words_[3]==0 && daemon.deletion_list_.value==456);
    reset(daemon,packet); mutate_clock=true;
    assert(netdaemon_handle_five_message_routes(daemon,0x4403,packet,7,456));
    assert(drop_calls==1 && arg0==1 && arg1==11 && arg2==13);
    assert(daemon.deletion_list_.value==456 && acknowledged_faction==7);
    reset(daemon,packet); mutate_clock=true;
    assert(netdaemon_handle_five_message_routes(daemon,0x4404,packet,7,456));
    assert(action_calls==1 && arg0==2 && daemon.deletion_list_.value==456);
    // DOUBLE CROSS reloads arguments, sequence and faction; it is not the
    // common saved-header path used by the other four operations.
    reset(daemon,packet); mutate_clock=mutate_cross=true;
    daemon.channels_[1].leading_words_[3]=99;
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,packet,7,456));
    assert(cross_calls==1 && arg0==2 && arg1==19 && arg2==23);
    assert(daemon.deletion_list_.value==777 && daemon.channels_[1].leading_words_[3]==99);
    reset(daemon,packet); daemon.channels_[1].trailing_words_[2]=9;
    assert(netdaemon_handle_five_message_routes(daemon,0x4404,packet,7,456));
    assert(daemon.channels_[1].trailing_words_[2]==0 && events.back()=="message_data");
    reset(daemon,packet);
    assert(!netdaemon_handle_five_message_routes(daemon,0x9999,packet,7,456));
    assert(events.empty() && daemon.deletion_list_.stores==0);
    reset(daemon,packet); NetDaemonNet.primary_state_.leading_words_[0]=1;
    Vehs[1].faction_id=2;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(SpockNotifyPending==1 && order_calls==0 && daemon.deletion_list_.stores==1);
    reset(daemon,packet); daemon.primary_state_.leading_words_[0]=1;
    Vehs[1].faction_id=2;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(SpockNotifyPending==0 && order_calls==1 && daemon.deletion_list_.stores==0);
}
'''


WIRE_EDITS = (
    ('        unsigned long message_size) {\n    const int packet_faction = *reinterpret_cast<int *>(message + 4);\n', '        int entry_packet_faction, uint32_t entry_sequence) {\n    // Original stack+8 is the faction captured at534466, not a transport\n    // identity. Stack-10 is the packet sequence captured at534461, not its\n    // byte length. The DOUBLE CROSS arm instead rereads both packet fields.\n', 1),
    ('        log_say("From Server: MOVE VEH", packet_faction, vehicle_id, direction);\n', '        log_say("From Server: MOVE VEH", *reinterpret_cast<int *>(message + 4), vehicle_id, direction);\n', 1),
    ('        if (Vehs[vehicle_id].faction_id != packet_faction) {\n', '        bool execute_order = true;\n        if (Vehs[vehicle_id].faction_id != *reinterpret_cast<int *>(message + 4)) {\n', 1),
    ('                SpockNotifyPending = 1;\n', '                SpockNotifyPending = 1;\n                // Pinned5367CC jumps past order_veh to the acknowledgement.\n                execute_order = false;\n', 1),
    ('        ::order_veh(vehicle_id, direction, 2);\n        log_say("Acknowledging MOVE VEH", packet_faction, 0, 0);\n', '        if (execute_order)\n            ::order_veh(vehicle_id, *reinterpret_cast<int *>(message + 0x14), 2);\n        log_say("Acknowledging MOVE VEH", entry_packet_faction, 0, 0);\n', 1),
    ('            daemon.deletion_list_.set_sequence_bits(static_cast<uint32_t>(message_size));\n        if (packet_faction == ConsoleGlobal.faction_id_)\n', '            daemon.deletion_list_.set_sequence_bits(entry_sequence);\n        if (entry_packet_faction == ConsoleGlobal.faction_id_)\n', 4),
    ('        log_say("From Server: ARTY", packet_faction, vehicle_id, direction);\n', '        log_say("From Server: ARTY", *reinterpret_cast<int *>(message + 4), vehicle_id, direction);\n', 1),
    ('        battle_fight(vehicle_id, direction, 1, 1, nullptr);\n        log_say("Acknowledging ARTY", packet_faction, 0, 0);\n', '        battle_fight(vehicle_id, *reinterpret_cast<int *>(message + 0x14), 1, 1, nullptr);\n        log_say("Acknowledging ARTY", entry_packet_faction, 0, 0);\n', 1),
    ('        log_say("Acknowledging AIRDROP", packet_faction, 0, 0);\n', '        log_say("Acknowledging AIRDROP", entry_packet_faction, 0, 0);\n', 1),
    ('        log_say("From Server: ACTION", packet_faction, vehicle_id, order);\n', '        log_say("From Server: ACTION", *reinterpret_cast<int *>(message + 4), vehicle_id, order);\n', 1),
    ('        ::action(vehicle_id);\n', '        ::action(*reinterpret_cast<int *>(message + 0x10));\n', 1),
    ('        double_cross(faction, partner, victim);\n', '        double_cross(*reinterpret_cast<int *>(message + 0x10),\n                     *reinterpret_cast<int *>(message + 0x14),\n                     *reinterpret_cast<int *>(message + 0x18));\n', 1),
    ('        if (packet_faction == ConsoleGlobal.faction_id_)\n', '        if (*reinterpret_cast<int *>(message + 4) == ConsoleGlobal.faction_id_)\n', 1),
    ('    unsigned char *msg = reinterpret_cast<unsigned char *>(message);\n', '    unsigned char *msg = reinterpret_cast<unsigned char *>(message);\n    // Pinned534458..534466 captures these original packet words before\n    // dispatch. Transport message_size is never the sequence acknowledgement.\n    uint32_t entry_sequence;\n    int entry_packet_faction;\n    std::memcpy(&entry_sequence, msg + 0x0C, sizeof entry_sequence);\n    std::memcpy(&entry_packet_faction, msg + 0x04, sizeof entry_packet_faction);\n', 1),
    ('            *this, type, msg, message_size)) {\n', '            *this, type, msg, entry_packet_faction, entry_sequence)) {\n', 1),
)


COMMAND_VIEW_DECLARATION = "// Borrowed wire view over the layout already owned by message_data's sender.\n// Read one field at a time: MOVE packets have only two payload words, while\n// AIRDROP/DOUBLE CROSS have three. Do not copy a full 32-byte record, impose\n// alignment, cache fields across calls, or initialize original unwritten bytes.\n// This is a serialization boundary, not a claim about a new original class.\nclass MessageCommandView {\npublic:\n    explicit MessageCommandView(const unsigned char *bytes) : bytes_(bytes) {}\n    int faction() const {\n        int value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, faction_), sizeof value);\n        return value;\n    }\n    int timestamp_bits() const {\n        int value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, timestamp_), sizeof value);\n        return value;\n    }\n    uint32_t sequence() const {\n        uint32_t value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, sequence_), sizeof value);\n        return value;\n    }\n    int payload_10() const {\n        int value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_10_), sizeof value);\n        return value;\n    }\n    int payload_14() const {\n        int value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_14_), sizeof value);\n        return value;\n    }\n    int payload_18() const {\n        int value;\n        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_18_), sizeof value);\n        return value;\n    }\nprivate:\n    const unsigned char *bytes_;\n};\n\n"
COMMAND_FIELD_EDITS = [["*reinterpret_cast<int *>(message + 4)","packet.faction()",5],["*reinterpret_cast<int *>(message + 8)","packet.timestamp_bits()",5],["*reinterpret_cast<int *>(message + 0x10)","packet.payload_10()",7],["*reinterpret_cast<int *>(message + 0x14)","packet.payload_14()",7],["*reinterpret_cast<int *>(message + 0x18)","packet.payload_18()",3],["*reinterpret_cast<uint32_t *>(message + 0x0C)","packet.sequence()",1]]


def apply_command_view_migration(source):
    old = extract(source, "static bool netdaemon_handle_five_message_routes(")
    new = old
    for before, after, count in COMMAND_FIELD_EDITS:
        assert new.count(before) == count
        new = new.replace(before, after)
    assert new.count("    switch (type) {") == 1
    new = new.replace("    switch (type) {", "    const MessageCommandView packet(message);\n    switch (type) {", 1)
    assert source.count(old) == 1
    from test_netdaemon_council_wire_oct11 import apply_council_wire_migration
    return apply_council_wire_migration(source.replace(old, COMMAND_VIEW_DECLARATION + new, 1))


def apply_wire_migration(source):
    """Only the reviewed dispatch edits; never derive a runtime source diff."""
    for before, after, count in WIRE_EDITS:
        assert source.count(before) == count
        source = source.replace(before, after)
    return apply_command_view_migration(source)


def product():
    text = SOURCE.read_text()
    record = extract(text, "struct MessageDataRecord {") + ";\n"
    view = extract(text, "class MessageCommandView {") + ";\n"
    return record + view + extract(text, "static void netdaemon_process_local_player_completion(") + "\n" + extract(
        text, "static bool netdaemon_handle_five_message_routes(")


def preamble():
    header=(ROOT/"src/netdaemon.h").read_text()
    declarations=extract(header,"struct PrimaryLockStateTail {")+";\n"
    declarations+=extract(header,"struct LockStateEntry {").replace("int *vehicle_selection_follow;","uint32_t vehicle_selection_follow;")+";"
    # No pointer payload is consumed; this host64 substitution preserves only
    # wire word geometry. The coordinator's native build owns ABI validation.
    return PREAMBLE.replace("@@LOCK_DECLARATIONS@@",declarations)


@pytest.mark.parametrize("damage", ["none", "server_order", "sequence", "direction", "action", "cross", "receiver_move", "receiver_arty"])
@pytest.mark.parametrize("wire_offset", [0, 1])
def test_actual_five_routes_and_call_boundary_reloads(tmp_path, damage, wire_offset):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = product()
    changes = {
        "receiver_move": ("NetDaemonNet.primary_state_.leading_words_[0]", "daemon.primary_state_.leading_words_[0]"),
        "receiver_arty": ("NetDaemonNet.channels_[0].leading_words_[3]", "daemon.channels_[0].leading_words_[3]"),
        "server_order": ("execute_order = false;", "execute_order = true;"),
        "sequence": ("set_sequence_bits(entry_sequence)", "set_sequence_bits(24)"),
        "direction": ("::order_veh(vehicle_id, packet.payload_14(), 2)",
                      "::order_veh(vehicle_id, direction, 2)"),
        "action": ("::action(packet.payload_10())", "::action(vehicle_id)"),
        "cross": ("double_cross(packet.payload_10(),\n                     packet.payload_14(),\n                     packet.payload_18())",
                  "double_cross(faction, partner, victim)"),
    }
    if damage != "none":
        before, after = changes[damage]
        assert before in body
        body = body.replace(before, after)
    source = tmp_path / "routes.cpp"
    executable = tmp_path / "routes"
    main = MAIN.replace("alignas(4) unsigned char packet[32];",
                        "alignas(4) unsigned char storage[36]; unsigned char *packet=storage+" + str(wire_offset) + ";")
    source.write_text(preamble() + body + main)
    subprocess.run([compiler,"-std=c++11","-O1",str(source),"-o",str(executable)],
                   check=True,capture_output=True,text=True,timeout=60)
    result = subprocess.run([str(executable)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_move_packet_reads_only_its_transmitted_24_bytes(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    main = r'''
int main() {
    long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    void *mapping = mmap(0, 2*page, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(mapping != MAP_FAILED);
    unsigned char *base = static_cast<unsigned char *>(mapping);
    assert(mprotect(base+page, page, PROT_NONE)==0);
    unsigned char *packet = base+page-24;
    std::memset(packet,0,24);
    live_packet=packet; put(4,7); put(8,123); put(0xc,456);
    put(0x10,1); put(0x14,11);
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    ConsoleGlobal.faction_id_=7; Vehs[1].faction_id=7;
    assert(netdaemon_handle_five_message_routes(daemon,0x4401,packet,7,456));
    assert(order_calls==1 && arg0==1 && arg1==11);
    assert(daemon.deletion_list_.value==456);
    assert(munmap(mapping,2*page)==0);
}
'''
    source = tmp_path / "bounded.cpp"
    executable = tmp_path / "bounded"
    source.write_text("#include <sys/mman.h>\n#include <unistd.h>\n" + preamble() + product() + main)
    subprocess.run([compiler,"-std=c++11","-O1",str(source),"-o",str(executable)],
                   check=True,capture_output=True,text=True,timeout=60)
    subprocess.run([str(executable)],check=True,capture_output=True,timeout=10)


def test_identity_and_entry_header_ownership():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    before, after = read_text(old,SOURCE), read_file(SOURCE)
    assert [(_continuity_fields(r),r.symbol,r.function_fragment) for r in before] == [
        (_continuity_fields(r),r.symbol,r.function_fragment) for r in after]
    text = SOURCE.read_text()
    body = extract(text,"void NetDaemon::process_message(")
    assert "const MessageCommandView entry_packet(msg);" in body
    assert "uint32_t entry_sequence = entry_packet.sequence();" in body
    assert "unsigned short type = entry_packet.opcode();" in body
    assert "int entry_packet_faction = entry_packet.faction();" in body
    assert "type = entry_packet.opcode();" in body
    assert "*this, type, msg, entry_packet_faction, entry_sequence" in body
    assert body.index("pid_2_who(static_cast<unsigned long>(sender_id))") < body.index(
        "const MessageCommandView entry_packet(msg)")
    assert body.index("entry_packet.faction()") < body.index("bool skip500")
    callback = body.index("message_data(0x1505, entry_packet_faction")
    assert callback < body.index("\n    type = entry_packet.opcode();", callback)


def test_frozen_wire_recipe_rejects_broken_migration_anchors():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    migrated = apply_wire_migration(old)
    assert "const MessageCommandView packet(message);" in migrated
    assert "packet.payload_10()" in migrated
    assert "set_sequence_bits(entry_sequence)" in migrated
    assert "double_cross(packet.payload_10()," in migrated
    with pytest.raises(AssertionError):
        apply_wire_migration(old.replace("        ::action(vehicle_id);", "        ::action(0);"))


def test_pinned_saved_header_skip_and_field_reload_anchors():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                               ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    cs = Cs(CS_ARCH_X86, CS_MODE_32)
    anchors = {}
    for start, end in ((0x534453,0x534469),(0x53456F,0x534572),(0x536781,0x536827),
                       (0x536835,0x536939),(0x536D0F,0x536D64)):
        anchors.update({i.address:(i.mnemonic,i.op_str) for i in cs.disasm(
            image.read(Span(start,end)),start)})
    expected = {
        0x534458:("mov","ecx, dword ptr [ebx + 0xc]"),
        0x53445B:("mov","ax, word ptr [ebx]"),
        0x53445E:("mov","edx, dword ptr [ebx + 4]"),
        0x534461:("mov","dword ptr [ebp - 0x10], ecx"),
        0x534466:("mov","dword ptr [ebp + 8], edx"),
        0x53456F:("mov","cx, word ptr [ebx]"),
        0x5367CC:("jmp","0x5367f4"),
        0x5367E5:("mov","eax, dword ptr [ebx + 0x14]"),
        0x5367EC:("call","0x5947c0"),
        0x5367F4:("mov","edi, dword ptr [ebp + 8]"),
        0x536813:("mov","ecx, dword ptr [ebp - 0x10]"),
        0x536816:("mov","dword ptr [esi + 0x1b2c], ecx"),
        0x536842:("mov","dword ptr [ebp + 0xc], edx"),
        0x536845:("mov","dword ptr [ebp + 0x10], eax"),
        0x53689D:("mov","edx, dword ptr [ebx + 0x14]"),
        0x5368CC:("mov","eax, dword ptr [ebp - 0x10]"),
        0x53690A:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536920:("mov","edx, dword ptr [ebp - 0x10]"),
        0x536D2E:("mov","edx, dword ptr [ebx + 0x18]"),
        0x536D31:("mov","eax, dword ptr [ebx + 0x14]"),
        0x536D34:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536D45:("mov","eax, dword ptr [ebx + 0xc]"),
        0x536D48:("mov","ecx, dword ptr [ebx + 4]"),
    }
    for address, instruction in expected.items():
        assert anchors[address] == instruction
