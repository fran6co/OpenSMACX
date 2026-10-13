"""Source witnesses for the pinned field-wise NetWin packet protocol.

Behavior controls separately exercise assigned fields and payloads. These
witnesses do not predict untouched stack bytes or establish native matching.
"""

from pathlib import Path

import pytest

from test_netwin_owner_behavior import extract_function
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("signature", (
    "static void pick_send_state()",
    "void NetWin::pick_faction(int",
    "void NetWin::pick_diff(int",
    "void NetWin::pick_map()",
    "int NetWin::pick_pass(int",
    "void NetWin::on_button_clicked(int",
))
def test_packet_constructors_do_not_blanket_initialize(signature):
    body = extract_function((ROOT / "src/netwin.cpp").read_text(), signature)
    assert "memset(" not in body


@pytest.mark.parametrize("signature,count", (
    ("void NetWin::pick_faction(int", 1),
    ("void NetWin::pick_diff(int", 1),
    ("void NetWin::on_button_clicked(int", 2),
))
def test_native_explicit_zero_store_is_not_lost_with_blanket_clear(signature, count):
    body = extract_function((ROOT / "src/netwin.cpp").read_text(), signature)
    assert body.count("packet.header_.field_8_ = 0;") == count


def test_password_reuses_word_opcode_packet_without_widened_store():
    body = extract_function((ROOT / "src/netwin.cpp").read_text(), "int NetWin::pick_pass(int")
    assert "NetWinLobbySinglePacket packet;" in body
    assert "packet.header_.opcode_ = 0x04;" in body
    assert "packet.header_.flags_ = 0x2F;" in body
    assert "packet.header_.field_8_ = 0;" in body
    assert "packet.header_.field_2_ =" not in body
    assert "packet.header_.field_C_ =" not in body


def test_packet_types_are_the_canonical_header_declarations():
    declarations = installed_packet_declarations()
    for name in (
        "NetWinLobbyPacketHeader", "NetWinLobbySinglePacket",
        "NetWinLobbyFullPacket", "NetWinLobbyLeavePacket",
        "NetWinSessionOptionsPacket", "NetWinSessionBlockPacket",
        "NetWinSessionLobbyPacket",
    ):
        assert f"struct {name} {{" in declarations
    source = (ROOT / "src/netwin.cpp").read_text()
    assert "struct NetWinLobbyPacketHeader {" not in source
    assert "struct NetWinLobbySinglePacket {" not in source
