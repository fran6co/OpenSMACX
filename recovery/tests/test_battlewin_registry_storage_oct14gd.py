"""Original registry identity and receiver-store evidence, not runtime proof."""
import hashlib
import os
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe",
))


def test_historical_receiver_identity_is_owned_by_registry_element():
    objects = read_objects(ROOT, PIN)
    parent, = [row for row in objects
               if row.symbol == "?InterfaceRegistry@@3PAPAVSubInterface@@A"]
    child, = [row for row in objects
              if row.symbol == "?BattleWinInitReceiver@@3PAVBattleWin@@A"]
    assert parent.span == Span(0x007AE7F0, 0x007AE820)
    assert child.origin.address == parent.origin.address + 10 * 4
    assert child.span is None  # Preserve the historical unbounded obligation.
    assert child.parent_symbol == parent.symbol
    assert child.member_path == ("[10]",)
    assert child.unit == parent.unit == "src/maininterface.cpp"
    header = (ROOT / "src/battlewin.h").read_text()
    assert "RECOVERY_BASE: v1 BattleWin SubInterface 0x0 0x00422EE0" in header
    assert "class BattleWin : public SubInterface" in header


def test_original_init_publishes_its_unadjusted_receiver_in_slot_ten():
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for address, expected in (
        (0x004219EB, ("mov", "esi, ecx")),
        (0x00421B0D, ("mov", "dword ptr [0x7ae818], esi")),
    ):
        instruction, = decoder.disasm(image.read(Span(address, address + 16)),
                                     address, count=1)
        assert (instruction.mnemonic, instruction.op_str) == expected
    assert image.read_initial(Span(0x007AE7F0, 0x007AE820)) == bytes(48)
