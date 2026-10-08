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
    # Loader export cells now have a source owner; that does not resolve the
    # separate singleton extents or the backend's higher virtual interface.
    header = (ROOT / "src/sounddevice.h").read_text()
    assert "WaveInDeviceDispatchView" not in header
    assert "void *device_14_;" in header
    assert "extern Midi_Device g_MIDI_DEVICE;" in header
    assert "extern Wave_In_Device g_WAVE_IN_DEVICE;" in header
    assert "Midi_Device *const MidiDeviceGlobal" not in header
    assert "Wave_In_Device *const WaveInDeviceGlobal" not in header


def test_real_device_singletons_have_source_storage_and_current_owner_calls():
    header = (ROOT / "src/sounddevice.h").read_text()
    owner = (ROOT / "src/sounddevice.cpp").read_text()
    sound = (ROOT / "src/sound.cpp").read_text()
    voice = (ROOT / "src/voice.cpp").read_text()
    loader = (ROOT / "src/sound_loader.cpp").read_text()

    assert "extern Midi_Device g_MIDI_DEVICE;" in header
    assert "extern Wave_In_Device g_WAVE_IN_DEVICE;" in header
    assert owner.count("Midi_Device g_MIDI_DEVICE;") == 1
    assert owner.count("Wave_In_Device g_WAVE_IN_DEVICE;") == 1
    assert "= (Midi_Device *)0x0090D950" not in header
    assert "= (Wave_In_Device *)0x0090DB50" not in header
    assert "RECOVERY_OBJECT: v1 0x0090D950 ?g_MIDI_DEVICE@@3VMidi_Device@@A" in owner
    assert "RECOVERY_OBJECT: v1 0x0090DB50 ?g_WAVE_IN_DEVICE@@3VWave_In_Device@@A" in owner

    init = sound[sound.index("int __cdecl init_sound("):]
    assert "g_MIDI_DEVICE.Midi_Device::init(window, 2);" in init
    assert "g_WAVE_IN_DEVICE.Wave_In_Device::init(window, backends);" in init
    assert init.index("g_MIDI_DEVICE.Midi_Device::init") < init.index(
        "g_WAVE_IN_DEVICE.Wave_In_Device::init")
    assert "g_WAVE_IN_DEVICE.start_record()" in voice
    assert "g_WAVE_IN_DEVICE.end_record()" in voice

    release = loader[loader.index("int __cdecl release_sound()"):]
    order = ["g_MIDI_DEVICE.release();", "WaveDeviceGlobal.release();",
             "g_WAVE_IN_DEVICE.release();", "unload_sound_dll();"]
    positions = [release.index(item) for item in order]
    assert positions == sorted(positions)
    assert "Midi_Device::Midi_Device()" in owner
    assert "Wave_In_Device::Wave_In_Device()" in owner
    # These checks establish source ownership/use sites, not native allocation
    # extents or the linked application's process-startup constructor order.
