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
    for name in ("wave_device.cpp", "wave_device.h", "soundx_wave_device_abi.h", "uv2player.cpp"):
        path = ROOT / "src" / name
        before = subprocess.check_output(["git", "show", BASE + ":src/" + name], cwd=ROOT, text=True)
        after = path.read_text()
        if name == "wave_device.cpp":
            expected = before.replace("int Wave_Device::get_ds()", "IDirectSound *Wave_Device::get_ds()")
            # Only authored binding/symbol facts are added; native body and
            # historical marker/lessons remain unchanged apart from the type.
            projected = "\n".join(line for line in after.split("\n") if not (
                line.startswith("// RECOVERY_BINDING: v1") and "borrowed DirectSound" in line
                or line == "// symbol    ?get_ds@Wave_Device@@QAEPAUIDirectSound@@XZ"))
            assert expected == projected
            assert definition(before, "int Wave_Device::get_ds()").replace("int Wave_Device::get_ds()", "IDirectSound *Wave_Device::get_ds()") == definition(after, "IDirectSound *Wave_Device::get_ds()")
        elif name == "wave_device.h":
            expected = before.replace("struct EAX_REVERB_PROPERTIES;", "struct EAX_REVERB_PROPERTIES;\nstruct IDirectSound;").replace("  int get_ds();", "  IDirectSound *get_ds();")
            assert expected == after
        elif name == "soundx_wave_device_abi.h":
            expected = before.replace("#pragma once", "#pragma once\nstruct IDirectSound;").replace("virtual int get_ds() = 0;", "virtual IDirectSound *get_ds() = 0;")
            assert expected == after
        else:
            expected = before.replace("media_.sound_device =\n        reinterpret_cast<IDirectSound *>(WaveDeviceGlobal.get_ds());", "media_.sound_device = WaveDeviceGlobal.get_ds();")
            assert expected == after
        old, new = read_text(before, path), read_text(after, path)
        assert [_continuity_fields(r) for r in old] == [_continuity_fields(r) for r in new]
        assert [(r.address, r.symbol) for r in old if r.address != 0x4C5190] == [(r.address, r.symbol) for r in new if r.address != 0x4C5190]
    row, = (r for r in read_text((ROOT / "src/wave_device.cpp").read_text(), ROOT / "src/wave_device.cpp") if r.address == 0x4C5190)
    assert row.symbol == "?get_ds@Wave_Device@@QAEPAUIDirectSound@@XZ" and row.byte_exact


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
