"""Actual packet builders; poison controls test writes, not original stack values."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "e33cd553388fbe62c77f0d71c629c47d84cd43fe"
SIGNATURES = {
    "raw": "void __cdecl send_raw_data(",
    "bulk": "void __cdecl message_big_data(",
    "landmark": "void __cdecl message_landmark(",
}
OLD_COMMENT = ("// The image stores the payload dwords out of order - 0x1C first, then the\n"
               "// stamp block, then 0x14 and 0x18 - and that order is kept.\n")
NEW_COMMENT = ("// Native payload stores are +0x10, +0x1C, sequence, +0x14, +0x18.\n"
               "// Null text leaves the entire text area unwritten; strcpy remains unchecked.\n")


def packet_declarations(source):
    start = source.index("// Native automatic wire records.")
    end = source.index("// ORIGINAL: 0x00592F50", start)
    return source[start:end]


def restore_packet_migration(source):
    """Reverse precisely this authorized migration for earlier TU continuity tests."""
    baseline = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    restored = source
    for signature in SIGNATURES.values():
        current_body = extract_definition(restored, signature)
        old_body = extract_definition(baseline, signature)
        restored = restored.replace(current_body, old_body, 1)
    restored = restored.replace(packet_declarations(restored), "", 1)
    assert restored.count(NEW_COMMENT) == 1
    return restored.replace(NEW_COMMENT, OLD_COMMENT, 1)


def test_only_three_packet_builders_and_passive_layouts_change():
    path = ROOT / "src/netdaemon.cpp"
    source = path.read_text()
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    assert restore_packet_migration(source) == old
    rows = read_text(source, path)
    assert write(source, rows) == source
    previous = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(previous)
    for row in rows:
        baseline = previous[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(baseline, field.name)
        # Only the independently corrected landmark explanatory comment changes.
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(baseline)
    for kind, signature in SIGNATURES.items():
        body = extract_definition(source, signature)
        record = "MessageLandmarkRecord" if kind == "landmark" else "MessageBigDataRecord"
        assert record + " message;" in body
        assert "message{}" not in body and "message =" not in body
        assert "memset" not in body
        assert "unresolved_2_" not in body
        assert "reinterpret_cast<int *>" not in body
    landmark = extract_definition(source, SIGNATURES["landmark"])
    writes = ("message.payload_10_ =", "message.payload_1C_ =", "message.sequence_ =",
              "message.payload_14_ =", "message.payload_18_ =")
    assert [landmark.index(x) for x in writes] == sorted(landmark.index(x) for x in writes)
    assert "if (text)" in landmark and "strcpy(message.text_, text);" in landmark
    bulk = extract_definition(source, SIGNATURES["bulk"])
    assert "if (count > 0)" in bulk and "i < count" in bulk
    assert "filled = count;" in bulk and "i < 0x20 - filled" in bulk


def control(kind):
    source = (ROOT / "src/netdaemon.cpp").read_text()
    declarations = packet_declarations(source)
    body = extract_definition(source, SIGNATURES[kind])
    record = "MessageLandmarkRecord" if kind == "landmark" else "MessageBigDataRecord"
    declaration = record + " message;"
    assert body.count(declaration) == 1
    # Test-only initialization exposes inadvertent stores to originally unwritten
    # bytes. Production has no such initialization; original values remain unknown.
    body = body.replace(declaration, declaration + "\n    std::memset(&message, 0xA5, sizeof(message));", 1)
    invocation = {
        "raw": "send_raw_data(opcode, target, values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);",
        "bulk": "message_big_data(opcode, target, count > 0 ? values : nullptr, count);",
        "landmark": "message_landmark(opcode, target, text, values[0], values[1], values[2], values[3]);",
    }[kind]
    return r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#define __cdecl
DECLARATIONS
static_assert(sizeof(int)==4,"native payload word");
struct Console { int faction_id_; } ConsoleGlobal;
struct LockState { uint32_t leading_words_[4]; };
static int NetBusyCount,time_calls,copy_calls,send_calls;
static int *live_payload;
static std::vector<unsigned char> sent;
static int received_target;
static uint32_t timeGetTime();
struct Daemon {
    LockState lock_state_[1];
    int send_message(char *message,uint32_t length,int target){
        assert(NetBusyCount==8+4*copy_calls); // Increment follows optional copy.
        sent.resize(length);std::memcpy(sent.data(),message,length);
        received_target=target;++send_calls;
        NetBusyCount+=2; // Decrement must use live state after transport returns.
        return -19;
    }
} NetDaemonNet;
static uint32_t timeGetTime(){
    assert(NetBusyCount==7 && time_calls==0);
    ++time_calls;
    ConsoleGlobal.faction_id_=99;
    NetDaemonNet.lock_state_[0].leading_words_[0]=0xFEDCBA98u;
    if(live_payload)live_payload[0]=-7654321;
    return 0xF1234567u;
}
static char *host_strcpy(char *destination,const char *source){
    assert(NetBusyCount==7 && time_calls==1);
    ++copy_calls;NetBusyCount+=4;
    return std::strcpy(destination,source); // No additional length guard.
}
#define strcpy host_strcpy
BODY
#undef strcpy
static void put(std::vector<unsigned char> &bytes,size_t offset,uint32_t value,int width=4){
    for(int i=0;i<width;++i)bytes[offset+i]=static_cast<unsigned char>(value>>(8*i));
}
int main(){
    const bool raw=KIND_RAW,bulk=KIND_BULK;
    const int opcode=static_cast<int>(0xABCD2307u),target=-33;
    for(int count:{-2147483647-1,-3,0,1,21,31,32})for(int text_case=0;text_case<4;++text_case){
        NetBusyCount=7;time_calls=copy_calls=send_calls=0;sent.clear();received_target=0;
        ConsoleGlobal.faction_id_=-7;NetDaemonNet.lock_state_[0].leading_words_[0]=0x12345678;
        int values[32];for(int i=0;i<32;++i)values[i]=1009+i*113;
        live_payload=bulk && count>0 ? values : nullptr;
        std::string storage=text_case==1?"":text_case==2?"faction landmark":std::string(31,'z');
        char *text=text_case ? &storage[0] : nullptr;
        INVOCATION
        assert(send_calls==1 && time_calls==1 && received_target==target);
        assert(copy_calls==(!raw && !bulk && text ? 1 : 0));
        assert(NetBusyCount==9+4*copy_calls);
        std::vector<unsigned char> expected(raw||bulk?0x90:0x40,0xA5);
        put(expected,0,static_cast<uint32_t>(opcode),2);
        put(expected,4,static_cast<uint32_t>(-7)); // Captured before time callback.
        put(expected,8,0xF1234567u);
        put(expected,12,0xFEDCBA98u); // Live read after time callback.
        if(raw){
            for(int i=0;i<8;++i)put(expected,0x10+i*4,static_cast<uint32_t>(values[i]));
        }else if(bulk){
            for(int i=0;i<32;++i)put(expected,0x10+i*4,i<count?static_cast<uint32_t>(values[i]):0);
        }else{
            for(int i=0;i<4;++i)put(expected,0x10+i*4,static_cast<uint32_t>(values[i]));
            if(text)std::memcpy(expected.data()+0x20,text,storage.size()+1);
        }
        assert(sent==expected); // Includes test-poisoned unwritten bytes.
    }
}
'''.replace("DECLARATIONS", declarations).replace("BODY", body).replace(
        "INVOCATION", invocation).replace("KIND_RAW", "true" if kind == "raw" else "false").replace(
        "KIND_BULK", "true" if kind == "bulk" else "false")


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("kind", ("raw", "bulk", "landmark"))
def test_actual_typed_packets_defined_paths_and_write_footprints(tmp_path, compiler, kind):
    program = control(kind)
    run_control(tmp_path, compiler, program, None)
    common = (
        ("message.faction_ = ConsoleGlobal.faction_id_;", "message.faction_ = 0;"),
        ("message.sequence_ = NetDaemonNet.lock_state_[0].leading_words_[0];", "message.sequence_ = 0x12345678;"),
        ("message.timestamp_ = timeGetTime();", "message.timestamp_ = timeGetTime(); message.unresolved_2_[0] = 0;"),
        ("    NetBusyCount++;", "    NetBusyCount += 2;"),
    )
    specific = {
        "raw": (
            ("message.data_[7] = payload_2C;", "message.data_[7] = payload_28;"),
            ("message.data_[0] = payload_10;", "message.data_[0] = payload_10; message.data_[31] = 0;"),
            ("0x90, target", "0x30, target"),
        ),
        "bulk": (
            ("if (count > 0)", "if (count > 1)"),
            ("i < count; i++", "i < count - 1; i++"),
            ("i < 0x20 - filled", "i < 0x1f - filled"),
        ),
        "landmark": (
            ("message.payload_1C_ = payload_1C;", "message.payload_1C_ = payload_18;"),
            ("if (text)", "if (text && text[0])"),
            ("strcpy(message.text_, text);", "strcpy(message.text_, text); message.text_[31] = 0;"),
            ("0x40, target", "0x20, target"),
        ),
    }
    for damage in common + specific[kind]:
        run_control(tmp_path, compiler, program, damage)


def test_original_transmitted_extents_and_unsafe_copy_instructions():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source.asm import Listing
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    ranges = ((0x592F50,0x592FE9,"0x90"), (0x592FF0,0x593089,"0x90"),
              (0x593110,0x593190,"0x40"))
    listings = []
    for start,end,length in ranges:
        lines = "\n".join(Listing(image.read(Span(start,end)),start).lines).lower()
        assert "sub esp, " + length in lines
        assert "push " + length in lines
        assert "call 0x532940" in lines
        listings.append(lines)
    assert "rep movsd" in listings[1] and "rep stosd" in listings[1]
    assert "call 0x645460" in listings[2] # The original unbounded strcpy call.
