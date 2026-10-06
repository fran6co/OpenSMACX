"""Original identity controls for the compiler-owned VoiceRx allocation tail.

These inspect installed source and optional original bytes, not a proposal or
an acceptance receipt. Current compiled lifetime verification remains separate.
"""
import os
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "recovery/project.json").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def test_installed_tail_retains_original_identity_and_restoration_obligation():
    records = read_file(ROOT / "src/net_class.cpp")
    tail, = [record for record in records if record.address == 0x633720]
    assert tail.name == "j_VoiceRx__VoiceRx_dt"
    assert tail.image_spans == ((0x633720, 0x633725),)
    assert tail.mode.value == "file" and tail.byte_exact
    assert tail.symbol == "??1VoiceRxAllocation@@UAE@XZ"
    assert tail.levers
    assert not (ROOT / "src/unrecovered/00633720.cpp").exists()
    caller, = [record for record in records if record.address == 0x6336D0]
    assert caller.symbol == "??_GVoiceRxAllocation@@UAEPAXI@Z"
    target, = [record for record in read_file(ROOT / "src/voice.cpp")
               if record.address == 0x4C8A50]
    assert target.symbol == "??1VoiceRx@@UAE@XZ"


@pytest.mark.skipif(os.environ.get("OPENSMACX_LIFETIME_ASSET_TESTS") != "1",
                    reason="requires opt-in pinned original executable")
def test_pinned_deleting_caller_and_tail_target_are_distinct_entries():
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("original executable is unavailable")
    image = PeImage(path.read_bytes(), PIN)
    for start, opcode, target in ((0x6336D3, 0xE8, 0x633720),
                                   (0x633720, 0xE9, 0x4C8A50)):
        raw = image.read(Span(start, start + 5))
        assert raw[0] == opcode
        assert start + 5 + int.from_bytes(raw[1:], "little", signed=True) == target
