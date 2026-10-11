"""Session-status packets must update the original player-record prefix."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT = Path(__file__).resolve().parents[2]
BEFORE = '''    const int faction = *reinterpret_cast<int *>(message + 4);
    if (type == 0x1F00 || type == 0x4E03 || type == 0x4F00)
        daemon->primary_state_.leading_words_[1] = 1;

    unsigned char *status = daemon->process_slots_[faction].status_bytes_;
    status[0] = static_cast<unsigned char>(message[0x18]);
    status[2] = static_cast<unsigned char>(message[0x14]);
    status[1] = static_cast<unsigned char>(message[0x10]);
'''
AFTER = '''    if (type == 0x1F00 || type == 0x4F00)
        daemon->primary_state_.leading_words_[1] = 1;
    const int faction = *reinterpret_cast<int *>(message + 4);

    // Native 0x537BF5/0x537BFE/0x537C07 write +0x790/+0x792/+0x791.
    // Slot base +0x76C makes these +0x24/+0x26/+0x25, not the +0x28
    // transfer-status region. Preserve the original unchecked row index.
    AlphaNetProcessSlot &slot = daemon->process_slots_[faction];
    slot.identity_ = static_cast<int8_t>(message[0x18]);
    slot.field_26_ = static_cast<uint8_t>(message[0x14]);
    slot.field_25_ = static_cast<uint8_t>(message[0x10]);
'''


def apply_session_status_migration(source, reverse=False):
    before, after = (AFTER, BEFORE) if reverse else (BEFORE, AFTER)
    assert source.count(before) == 1
    return source.replace(before, after, 1)


def test_only_reviewed_session_stores_changed():
    before = subprocess.check_output(
        ["git", "show", "4c771e24f:src/netdaemon.cpp"], cwd=ROOT, text=True)
    assert apply_session_status_migration(before) == (ROOT / "src/netdaemon.cpp").read_text()


def test_native_flag_store_precedes_packet_index_read():
    body = extract((ROOT / "src/netdaemon.cpp").read_text(),
                   "static void netdaemon_handle_session_status(")
    assert body.index("leading_words_[1] = 1") < body.index("const int faction")


@pytest.mark.parametrize("damage", ["none", "wrong_offset", "wrong_order", "set_client_flag"])
def test_actual_session_stores_and_rebroadcast(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/netdaemon.cpp").read_text()
    body = "\n".join(extract(source, signature) for signature in (
        "static void netdaemon_rebroadcast_session_packet(",
        "static void netdaemon_handle_session_status("))
    mutations = {
        "wrong_offset": ("slot.identity_ =", "slot.status_bytes_[0] ="),
        "wrong_order": ("message[0x14]", "message[0x10]"),
        "set_client_flag": ("type == 0x1F00 || type == 0x4F00", "true"),
    }
    if damage != "none":
        before, after = mutations[damage]
        assert before in body
        body = body.replace(before, after, 1)
    slot = extract((ROOT / "src/alphanet.h").read_text(), "struct AlphaNetProcessSlot {") + ";"
    harness = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstddef>
@@SLOT@@
struct Primary { uint32_t leading_words_[3]; };
struct NetDaemon {
    Primary primary_state_;
    AlphaNetProcessSlot process_slots_[8];
    int sends;
    int selected;
    void send_message(char *message, unsigned long size, int recipient) {
        assert(size == 64 && recipient == 0);
        assert(*reinterpret_cast<uint16_t *>(message) == 0x4F00);
        assert(process_slots_[selected].identity_ == -2);
        assert(process_slots_[selected].field_26_ == 0xB7);
        assert(process_slots_[selected].field_25_ == 0xC8);
        ++sends;
    }
};
@@BODY@@
int main() {
    assert(sizeof(AlphaNetProcessSlot) == 0x19C);
    assert(offsetof(AlphaNetProcessSlot, identity_) == 0x24);
    assert(offsetof(AlphaNetProcessSlot, field_25_) == 0x25);
    assert(offsetof(AlphaNetProcessSlot, field_26_) == 0x26);
    const uint16_t types[] = {0x1F00, 0x2F00, 0x4F00};
    for (int index = 0; index < 8; ++index) for (int route = 0; route < 3; ++route) {
        NetDaemon daemon;
        std::memset(daemon.process_slots_, 0x5A, sizeof daemon.process_slots_);
        daemon.primary_state_.leading_words_[1] = 7;
        daemon.sends = 0; daemon.selected = index;
        uint32_t packet[16] = {0};
        char *message = reinterpret_cast<char *>(packet);
        std::memcpy(message, &types[route], 2);
        packet[1] = index;
        message[0x18] = static_cast<char>(0xFE);
        message[0x14] = static_cast<char>(0xB7);
        message[0x10] = static_cast<char>(0xC8);
        netdaemon_handle_session_status(&daemon, types[route], message, sizeof packet);
        assert(daemon.primary_state_.leading_words_[1] == (route == 1 ? 7u : 1u));
        assert(daemon.sends == (route == 1 ? 1 : 0));
        for (int row = 0; row < 8; ++row) {
            const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&daemon.process_slots_[row]);
            for (unsigned offset = 0; offset < sizeof(AlphaNetProcessSlot); ++offset) {
                unsigned expected = 0x5A;
                if (row == index && offset == 0x24) expected = 0xFE;
                if (row == index && offset == 0x25) expected = 0xC8;
                if (row == index && offset == 0x26) expected = 0xB7;
                assert(bytes[offset] == expected);
            }
        }
    }
}
'''
    cpp = tmp_path / "session.cpp"
    cpp.write_text(harness.replace("@@SLOT@@", slot).replace("@@BODY@@", body))
    exe = tmp_path / "session"
    subprocess.run([compiler, "-std=c++03", "-fsanitize=undefined,bounds",
                    "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, timeout=60)
    result = subprocess.run([str(exe)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_pinned_session_destination_and_wire_byte_order():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic, i.op_str) for i in
                    Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x537BD5, 0x537C37)), 0x537BD5)}
    expected = {
        0x537BD5: ("mov", "dword ptr [esi + 0x1b34], 1"),
        0x537BEF: ("mov", "cl, byte ptr [ebx + 0x18]"),
        0x537BF5: ("mov", "byte ptr [eax + 0x790], cl"),
        0x537BFB: ("mov", "dl, byte ptr [ebx + 0x14]"),
        0x537BFE: ("mov", "byte ptr [eax + 0x792], dl"),
        0x537C04: ("mov", "cl, byte ptr [ebx + 0x10]"),
        0x537C07: ("mov", "byte ptr [eax + 0x791], cl"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction
