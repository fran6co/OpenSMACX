"""Actual table-E receiver preserves packet reads across logger/lookup calls."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "e4a7777ba"
EDITS = (
    ("        const int player_index =\n            *reinterpret_cast<int *>(message + 0x10);\n        const int sequence = *reinterpret_cast<int *>(message + 0x0C);\n", ""),
    ("        // selecting the PlayerLock from the unmodified packet word at +0x10.\n        (void)daemon.AlphaNet::who_2_idx(identity);",
     "        // selecting the PlayerLock from the post-log packet word at +0x10.\n        const int player_index = *reinterpret_cast<int *>(message + 0x10);\n        (void)daemon.AlphaNet::who_2_idx(player_index);\n        const int sequence = *reinterpret_cast<int *>(message + 0x0C);"),
    ("        const int third = *reinterpret_cast<int *>(message + 0x18);\n", ""),
    ("daemon.locks_.add_lock(sender_identity, third, first, second)",
     "daemon.locks_.add_lock(sender_identity,\n                *reinterpret_cast<int *>(message + 0x18),\n                *reinterpret_cast<int *>(message + 0x10),\n                *reinterpret_cast<int *>(message + 0x14))"),
)


def apply_table_e_read_migration(source):
    old = extract(source, "static bool netdaemon_process_table_e(")
    body = old
    for before, after in EDITS:
        assert body.count(before) == 1, before
        body = body.replace(before, after, 1)
    assert source.count(old) == 1
    from test_netdaemon_global_lock_signed_oct11 import apply_global_lock_signed_migration
    return apply_global_lock_signed_migration(source.replace(old, body, 1))


def current_wire_types(source):
    return "\n".join(extract(source, marker) + ";" for marker in (
        "struct MessageDataRecord {", "class MessageCommandView {",
        "struct MessageAcknowledgementPrefix {",
        "class MessageAcknowledgementPrefixView {", "struct MessageLockRecord {",
        "class MessageLockView {"))


PREAMBLE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
static unsigned char *packet;
static int logged_identity, logged_first, logged_second, lookup_arg, active_index;
static int sent_type, sent_target, sends, add_calls, add_result, add_args[4];
static bool mutate_log, mutate_lookup, mutate_send;
static int SpockHelpActive, TurnControlWord93A954;
static int NetDaemonProgressInterval, NetDaemonProgressLastTick;
static void put(int offset,int value) { std::memcpy(packet+offset,&value,4); }
struct PlayerLock {
    uint8_t active_; int index;
    int active() { active_index=index; return 1; }
};
struct Locks {
    PlayerLock records_[8]; uint32_t field_E8_;
    void unlock(int) {}
    int add_lock(int a,int b,int c,int d) {
        ++add_calls; add_args[0]=a; add_args[1]=b; add_args[2]=c; add_args[3]=d;
        return add_result;
    }
};
struct DeletionRecord { uint8_t acknowledgements_; int owner_; };
struct DeletionList { DeletionRecord records_[24]; };
struct AlphaNet {
    int who_2_idx(int identity) {
        lookup_arg=identity;
        if (mutate_lookup) { put(0x10,3); put(0x0C,30); }
        return 7; // Original deliberately discards the lookup result.
    }
};
struct Words { uint32_t leading_words_[4]; };
struct NetDaemon : AlphaNet { Locks locks_; DeletionList deletion_list_; Words channels_[2]; };
struct { struct { struct { uint32_t enable_mask_; } lock_masks_; } fields_; } GameDataStateStorage;
static NetDaemon *owner;
static void log_say(const char *label,int sender,int first,int second) {
    const std::string name(label);
    if (name=="From Client: ACKNOWLEDGE") {
        assert(sender==2); logged_identity=first;
        if (mutate_log) { put(0x10,2); put(0x0C,20); }
    }
    if (name=="From Client: ADD LOCK") {
        assert(sender==2); logged_first=first; logged_second=second;
        if (mutate_log) { put(0x10,20); put(0x14,21); put(0x18,22); }
    }
}
static void message_data(int type,int target,int,int,int,int) {
    ++sends; sent_type=type; sent_target=target;
    if (mutate_send && type==0x1206) owner->locks_.records_[target].active_=0x40;
}
'''

MAIN = r'''
static void reset(NetDaemon &daemon,unsigned char *bytes) {
    std::memset(&daemon,0,sizeof daemon); std::memset(bytes,0,32);
    packet=bytes; owner=&daemon; mutate_log=mutate_lookup=mutate_send=false;
    sends=add_calls=add_result=0; lookup_arg=active_index=-1;
    SpockHelpActive=TurnControlWord93A954=0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=6;
    for (int n=0;n<8;++n) { daemon.locks_.records_[n].index=n; daemon.locks_.records_[n].active_=2; }
}
int main() {
    NetDaemon daemon; alignas(4) unsigned char bytes[32];
    reset(daemon,bytes); put(0x10,1); put(0x0C,10);
    daemon.deletion_list_.records_[0].owner_=10;
    daemon.deletion_list_.records_[1].owner_=20;
    daemon.deletion_list_.records_[2].owner_=30;
    daemon.deletion_list_.records_[2].acknowledgements_=2;
    mutate_log=mutate_lookup=mutate_send=true;
    assert(netdaemon_process_table_e(daemon,packet,0x2213,2));
    assert(logged_identity==1 && lookup_arg==2 && active_index==2);
    assert(daemon.deletion_list_.records_[0].acknowledgements_==0);
    assert(daemon.deletion_list_.records_[1].acknowledgements_==0);
    assert(daemon.deletion_list_.records_[2].acknowledgements_==0xFF);
    assert(sends==1 && sent_type==0x1206 && sent_target==2);
    assert(daemon.locks_.records_[2].active_==0x41); // Post-send state is reread too.
    for (int failure=0;failure<2;++failure) {
        reset(daemon,bytes); put(0x10,10); put(0x14,11); put(0x18,12);
        mutate_log=true; add_result=failure;
        assert(netdaemon_process_table_e(daemon,packet,0x2215,2));
        assert(logged_first==10 && logged_second==11 && add_calls==1);
        assert(add_args[0]==2 && add_args[1]==22 && add_args[2]==20 && add_args[3]==21);
        assert(sends==1 && sent_target==2 && sent_type==(failure ? 0x1208 : 0x1207));
    }
    reset(daemon,bytes); SpockHelpActive=1;
    assert(netdaemon_process_table_e(daemon,nullptr,0x2215,2));
    assert(add_calls==0 && sends==1 && sent_type==0x1208);
    assert(!netdaemon_process_table_e(daemon,nullptr,0x9999,2));
}
'''


@pytest.mark.parametrize("damage", ["none", "pre_log_index", "post_lookup_index", "pre_lookup_sequence", "cached_add"])
def test_actual_table_e_callback_read_order(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = extract(SOURCE.read_text(), "static bool netdaemon_process_table_e(")
    sequence = "        const int sequence = packet.sequence();"
    lookup = "        (void)daemon.AlphaNet::who_2_idx(player_index);"
    index = "        const int player_index = packet.player_index();"
    changes = {
        "pre_log_index": (index, "        const int player_index = identity;"),
        "post_lookup_index": (index, "        const int player_index = identity;"),
        "pre_lookup_sequence": (lookup+"\n"+sequence, sequence+"\n"+lookup),
            "cached_add": ("daemon.locks_.add_lock(sender_identity,\n                packet.payload_18(), packet.payload_10(),\n                packet.payload_14())", "daemon.locks_.add_lock(sender_identity, cached_third, first, second)"),
    }
    if damage != "none":
        if damage == "cached_add":
            declaration = "        const int second = packet.payload_14();"
            assert body.count(declaration) == 1
            body = body.replace(declaration, declaration + "\n        const int cached_third = packet.payload_18();", 1)
            before, after = changes[damage]
            before = before.replace("*reinterpret_cast<int *>(message + 0x18)", "packet.payload_18()")
            before = before.replace("*reinterpret_cast<int *>(message + 0x10)", "packet.payload_10()")
            before = before.replace("*reinterpret_cast<int *>(message + 0x14)", "packet.payload_14()")
            changes[damage] = (before, after)
        before, after = changes[damage]
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    source = tmp_path / "table.cpp"
    binary = tmp_path / "table"
    source.write_text(PREAMBLE + current_wire_types(SOURCE.read_text()) + body + MAIN)
    subprocess.run([compiler,"-std=c++11",str(source),"-o",str(binary)],
                   check=True,capture_output=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_historical_fixed_read_recipe_is_preserved_separately_from_current_source():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    migrated = apply_table_e_read_migration(old)
    migrated_body = extract(migrated, "static bool netdaemon_process_table_e(")
    assert "who_2_idx(player_index)" in migrated_body
    assert "const int player_index = *reinterpret_cast<int *>(message + 0x10);" in migrated_body
    assert "daemon.locks_.add_lock(sender_identity," in migrated_body
    current_body = extract(SOURCE.read_text(), "static bool netdaemon_process_table_e(")
    assert "packet.player_index()" in current_body and "packet.sequence()" in current_body
    assert "packet.payload_18(), packet.payload_10()," in current_body
    with pytest.raises(AssertionError):
        apply_table_e_read_migration(old.replace("who_2_idx(identity)", "who_2_idx(0)"))


def test_pinned_table_e_log_lookup_and_operand_reload_order():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    cs = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = {}
    for start,end in ((0x535DED,0x535E20),(0x535F45,0x535F73)):
        instructions.update({i.address:(i.mnemonic,i.op_str)
            for i in cs.disasm(image.read(Span(start,end)),start)})
    expected = {
        0x535DF9:("call","0x6262f0"),
        0x535DFE:("mov","eax, dword ptr [ebx + 0x10]"),
        0x535E06:("mov","dword ptr [ebp + 0xc], eax"),
        0x535E0A:("call","0x4e26b0"),
        0x535E0F:("mov","eax, dword ptr [ebx + 0xc]"),
        0x535F53:("call","0x6262f0"),
        0x535F58:("mov","eax, dword ptr [ebx + 0x14]"),
        0x535F5B:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x535F5E:("mov","edx, dword ptr [ebx + 0x18]"),
        0x535F64:("push","eax"),
        0x535F65:("push","ecx"),
        0x535F66:("push","edx"),
        0x535F67:("push","edi"),
        0x535F6E:("call","0x590470"),
    }
    for address,instruction in expected.items():
        assert instructions[address] == instruction
