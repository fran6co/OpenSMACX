"""Evidence-backed borrowed COM pointer contract; not external DLL proof."""
from pathlib import Path
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "0f913ad9242021223960a85d6db0e9a57e17bf33"


def test_bounded_pointer_contract_and_original_claim_continuity():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    from test_wave_device_provider_owner_oct9 import definition

    # Compare only the recovered operation, not the former translation unit:
    # its owner, virtual symbol, and neighboring interface bodies may evolve.
    old_source = subprocess.check_output(
        ["git", "show", BASE + ":src/wave_device.cpp"], cwd=ROOT, text=True)
    current_source = (ROOT / "src/sounddevice.cpp").read_text()
    old_body = definition(old_source, "int Wave_Device::get_ds()")
    current_body = definition(current_source, "IDirectSound *Wave_Device::get_ds()")
    expected_body = old_body.replace(
        "int Wave_Device::get_ds()", "IDirectSound *Wave_Device::get_ds()")
    assert current_body == expected_body
    assert "if (device_14_)" in current_body
    assert "return static_cast<WaveDeviceProviderCallView *>(device_14_)->get_ds();" in current_body
    assert "return 0;" in current_body

    # The original identity, image span, claim, and authored lesson history
    # remain attached to 0x4C5190. The source owner and emitted virtual symbol
    # are allowed to change with the interface migration.
    old_rows = {row.address: row for row in read_text(old_source, ROOT / "src/wave_device.cpp")}
    current_rows = {row.address: row for row in read_text(current_source, ROOT / "src/sounddevice.cpp")}
    assert 0x4C5190 in old_rows and 0x4C5190 in current_rows
    assert _continuity_fields(old_rows[0x4C5190]) == _continuity_fields(current_rows[0x4C5190])

    # The wrapper exposes the pointer as a virtual operation, and its opaque
    # provider boundary uses the matching pointer-return slot declaration.
    wave_header = (ROOT / "src/wave_device.h").read_text()
    assert "virtual IDirectSound *get_ds();" in wave_header
    assert "struct IDirectSound;" in wave_header
    provider_header = (ROOT / "src/soundx_wave_device_abi.h").read_text()
    assert "virtual IDirectSound *get_ds() = 0;" in provider_header

    # UV2Player stores the borrowed pointer directly in MediaDescriptor; this
    # query itself neither adds a reference nor changes provider ownership.
    consumer = (ROOT / "src/uv2player.cpp").read_text()
    assert "media_.sound_device = WaveDeviceGlobal.get_ds();" in consumer


def test_original_provider_slot_and_directsound_consumer():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {i.address: (i.mnemonic, i.op_str) for start, end in ((0x4C5190, 0x4C519F), (0x4BEA50, 0x4BEDB5))
                    for i in decoder.disasm(image.read(Span(start, end)), start)}
    assert instructions[0x4C5190] == ("mov", "ecx, dword ptr [ecx + 0x14]")
    assert instructions[0x4C5199] == ("jmp", "dword ptr [eax + 0x70]")
    assert instructions[0x4C519C] == ("xor", "eax, eax")
    assert instructions[0x4C519E] == ("ret", "")
    assert instructions[0x4BEB76] == ("call", "0x4c5190")
    assert instructions[0x4BEB81] == ("mov", "dword ptr [esi + 0x17c], eax")
