"""The DirectPlay Receive out slot is a DPID object, not an int alias."""
from pathlib import Path
import os

import pytest

from test_caviar_draw_callbacks_actual_source import extract


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void Net::receive_directplay_messages(int player_id)"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def valid_receive_contract(body: str) -> bool:
    return all((
        "DPID working_id;" in body,
        "working_id = static_cast<DPID>(player_id);" in body,
        "NetDirectPlay->Receive(\n                &working_id, &counterpart, flags, NetGetScratch, &size);" in body,
        "reinterpret_cast<LPDPID>(&working_id)" not in body,
    ))


def test_receive_out_local_matches_directplay_type_and_keeps_method_identity():
    source = (ROOT / "src/net_class.cpp").read_text()
    header = (ROOT / "src/net_class.h").read_text()
    body = extract(source, SIGNATURE)

    assert valid_receive_contract(body)
    assert header.count("void receive_directplay_messages(int player_id);") == 1
    assert "// ORIGINAL: 0x00630B80 sub_630b80 0x00630B80-0x00630BE3" in source
    assert "// symbol    ?receive_directplay_messages@Net@@QAEXH@Z" in source
    assert "working_id = static_cast<DPID>(player_id);" in body
    assert body.index("working_id = static_cast<DPID>(player_id);") < body.index("NetDirectPlay->Receive(")


@pytest.mark.parametrize("before,after", [
    ("DPID working_id;", "int working_id;"),
    ("&working_id, &counterpart", "reinterpret_cast<LPDPID>(&working_id), &counterpart"),
    ("working_id = static_cast<DPID>(player_id);", "working_id = player_id;"),
])
def test_old_alias_or_implicit_conversion_is_rejected(before: str, after: str):
    source = (ROOT / "src/net_class.cpp").read_text()
    body = extract(source, SIGNATURE)
    assert body.count(before) == 1
    damaged = body.replace(before, after, 1)
    assert not valid_receive_contract(damaged)


def test_pinned_receive_call_uses_a_reinitialized_stack_dpid_out_slot():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    assert image_path.is_file(), f"pinned original unavailable: {image_path}"
    image = PeImage(image_path.read_bytes(), PIN)
    rows = {i.address: (i.mnemonic, i.op_str) for i in
            Cs(CS_ARCH_X86, CS_MODE_32).disasm(
                image.read(Span(0x630B80, 0x630BE3)), 0x630B80)}

    expected = {
        0x630B90: ("mov", "edi, dword ptr [esp + 0x14]"),
        0x630BB3: ("mov", "dword ptr [esp + 0xc], edi"),
        0x630BC9: ("lea", "edx, [esp + 0x14]"),
        0x630BCE: ("lea", "edx, [esp + 0x1c]"),
        0x630BD4: ("call", "dword ptr [ecx + 0x64]"),
        0x630BD9: ("je", "0x630b96"),
        0x630BE0: ("ret", "4"),
    }
    for address, instruction in expected.items():
        assert rows[address] == instruction

