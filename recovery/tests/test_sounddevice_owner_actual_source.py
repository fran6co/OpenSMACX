from __future__ import annotations

from pathlib import Path

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]


def test_sound_device_source_retains_original_constructor_and_destructor_records():
    header = (ROOT / "src/sounddevice.h").read_text()
    source = (ROOT / "src/sounddevice.cpp").read_text()
    wave_header = (ROOT / "src/wave_device.h").read_text()
    wave_source = (ROOT / "src/wave_device.cpp").read_text()
    assert "class Midi_Device : public SoundDeviceBase" in header
    assert "class Wave_In_Device : public SoundDeviceBase" in header
    assert "class Wave_Device : public SoundDeviceBase" in wave_header
    for identity in (
        "0x004C4D70 sub_4c4d70",
        "0x004C5740 ??0Midi_Device@@QAE@XZ",
        "0x004C5780 ??1Midi_Device@@QAE@XZ",
        "0x004C5940 ??0Wave_In_Device@@QAE@XZ",
        "0x004C5980 ??1Wave_In_Device@@QAE@XZ",
    ):
        assert identity in source
    assert "0x004C4DD0 ??0Wave_Device@@QAE@XZ" in wave_source
    assert "0x004C4E60 ??1Wave_Device@@QAE@XZ" in wave_source
    records = read_text(source, "src/sounddevice.cpp")
    assert any(record.address == 0x4C4D70 for record in records)


def test_basepop_helper_uses_typed_owner_members_in_original_order():
    source = (ROOT / "src/basepop.cpp").read_text()
    helper = source[source.index("int __cdecl sub_48cf00(BasePop *owner, int vehicle)"):]
    assert "owner->picture_enabled()" in helper
    assert "owner->set_caller_picture" in helper
    assert "owner->dialogs().SpriteBox::sprite(" in helper
    assert helper.index("set_caller_picture") < helper.index("SpriteBox::sprite")
    assert helper.index("SpriteBox::sprite") < helper.index("set_redraw_callback")
    assert "&g_VEH_SPRITES[0]" in helper
    assert "veh_sprite_draw" in helper


def test_sound_owner_note_does_not_claim_backend_closure_complete():
    source = (ROOT / "src/basepop.cpp").read_text()
    assert "int __cdecl sub_48cf00(BasePop *owner, int vehicle)" in source
    # Backend/singleton consumers and Datalink bindings are separate owners;
    # the helper's source binding does not state those fixed-address surfaces
    # have been replaced.
    assert "0x0090DB3C" in (ROOT / "src/sounddevice.h").read_text()
