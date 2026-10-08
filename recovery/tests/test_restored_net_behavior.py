"""Read-only source/pin witnesses for proposed Net fidelity restoration.

Defined normal-slot and no-send behavior belongs in extracted host controls.
These witnesses cover only branch/ordering facts for unsafe index 16 and
indeterminate reserved packet bytes; neither is executed or value-asserted.
"""

from pathlib import Path

import pytest


def find_root(start: Path) -> Path:
    for parent in start.resolve().parents:
        if (parent / "src/net_class.cpp").is_file() and (parent / "src/netwin.cpp").is_file():
            return parent
    raise AssertionError("cannot find OpenSMACX source root")


ROOT = find_root(Path(__file__))


def extract(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start : i + 1]
    raise AssertionError(f"unclosed body: {signature}")


def test_player_index16_skips_initialization_then_keeps_shared_effects():
    source = (ROOT / "src/net_class.cpp").read_text()
    body = extract(source, "int Net::sub_632cc0(char *text, int id, unsigned char flag)")
    assert "if (index == 16)" not in body
    assert "return 4;" not in body
    init_start = body.index("if (index < 16)")
    init_open = body.index("{", init_start)
    depth = 0
    init_end = None
    for i in range(init_open, len(body)):
        if body[i] == "{":
            depth += 1
        elif body[i] == "}":
            depth -= 1
            if depth == 0:
                init_end = i + 1
                break
    assert init_end is not None
    init = body[init_open:init_end]
    tail = body[init_end:]
    assert "player.field_0_ = static_cast<uint32_t>(id);" in init
    assert "player.flags_14_ = flag;" in init
    assert "strcat(player.name_and_tail_, text);" in init
    assert "field_764_ = static_cast<uint32_t>(id)" in init
    assert "++field_6DC_;" in tail
    assert "broadcast_player_table();" in tail
    assert tail.index("broadcast_player_table();") < tail.index("on_player_added(")
    assert "reinterpret_cast<char *>(&guid_6E8_) + 1" in tail
    assert "field_6D4_" in tail
    assert tail.index("on_player_added(") < tail.index("jackal_voice_rx_.add_player(")
    assert "index < 16 ? players_[index].field_0_ : field_6D4_" in tail


def test_pinned_player_index16_branch_and_callback_reread():
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image.is_file():
        pytest.skip("pinned original image unavailable")
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    pin = image_pin(ROOT)
    pe = PeImage(image.read_bytes(), pin)
    span = pe.read(Span(0x632CC0, 0x632DAA))
    # cmp esi,10; jl back to the slot test; otherwise jump over slot writes to
    # 0x632D2B. At that target the shared counter load begins immediately.
    assert span[0x23:0x2F] == bytes.fromhex("83 FE 10 7C E9 EB 41 83 FE 10 7D 3C")
    assert span[0x6B:0x71] == bytes.fromhex("8B 97 DC 06 00 00")
    # Callback is followed by a fresh field_0 read, then the add-player call.
    assert span[0xAB:0xAE] == bytes.fromhex("FF 50 04")
    assert span[0xAE:0xB4] == bytes.fromhex("8B 86 54 01 00 00")
    assert span[0xBD:0xC2] == bytes.fromhex("E8 FE 03 00 00")
    header = (ROOT / "src/net_class.h").read_text()
    assert "players_[16];  // 0x154, sixteen 0x58-byte slots to 0x6D4" in header
    assert "uint32_t field_6D4_;  // 0x6D4" in header
    assert "_GUID *guid_6E8_;  // 0x6E8" in header


def test_exec_options_packet_only_asserts_assigned_bytes():
    source = (ROOT / "src/netwin.cpp").read_text()
    body = extract(source, "int NetWin::exec()")
    start = body.index("NetWinSessionOptionsPacket packet;")
    send = body.index("NetDaemonNet.send(&packet, 0x28", start)
    packet_code = body[start:send]
    assert "memset(" not in packet_code
    assert "packet.opcode_ = 0x2F02;" in packet_code
    assert "packet.options_ = NetWinSessionOptions;" in packet_code

    # Host controls inspect assigned opcode/options only, never reserved bytes.
