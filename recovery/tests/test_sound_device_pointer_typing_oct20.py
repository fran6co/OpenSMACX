"""The shared SoundX provider cell uses the reconstructed Sound base."""
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]


def test_sound_cell_and_ordinal_one_output_use_common_sound_owner():
    sound_h = (ROOT / "src/sound.h").read_text()

    assert '#include "soundx_sound_abi.h"' not in sound_h
    assert re.search(r"Sound\s*\*device_;\s*// 0x3C", sound_h)
    assert re.search(
        r"typedef int \(__cdecl \*SoundXCreateSoundProc\)\(\s*"
        r"Sound\s*\*\*,\s*const char \*,\s*int\s*\);",
        sound_h,
    )
    assert not (ROOT / "src/soundx_sound_abi.h").exists()

    # These are separate ordinal contracts and remain opaque pointer outputs.
    assert "typedef int (__cdecl *SoundXWaveDeviceCreateProc)(void **, unsigned long);" in sound_h
    assert "typedef int (__cdecl *SoundXMidiDeviceCreateProc)(void **, unsigned long);" in sound_h
    assert "typedef int (__cdecl *SoundXWaveInDeviceCreateProc)(void **, unsigned long);" in sound_h
    assert "typedef int (__cdecl *SoundXDeleteSoundProc)(void *);" in sound_h


def test_all_ordinal_one_callers_write_through_the_typed_sound_cell():
    owners = (
        "sound.cpp", "wave.cpp", "midi.cpp", "ambience.cpp",
        "video_sound.cpp", "voice.cpp", "sound_loader.cpp",
    )
    calls = 0
    for owner in owners:
        source = (ROOT / "src" / owner).read_text()
        owner_calls = source.count("SoundXOrdinal1CreateSound()")
        calls += owner_calls
        if owner != "sound_loader.cpp":
            assert source.count("&device_") >= owner_calls
    assert calls == 13
    loader = (ROOT / "src/sound_loader.cpp").read_text()
    assert re.search(
        r"sub_4c6000\(Sound\s*\*\*device_slot",
        loader,
    )

    sound = (ROOT / "src/sound.cpp").read_text()
    assert "__forceinline Sound *sound_device(Sound *device)" in sound
    assert "sound_device(void *device)" not in sound

    wave = (ROOT / "src/wave.cpp").read_text()
    assert re.search(
        r"wave_playback_device\(\s*Sound\s*\*device\s*\)", wave
    )
    assert "static_cast<SoundDeviceInterface *>(device_)->unk_slot24()" not in wave


def test_extension_views_and_midi_children_use_the_shared_sound_hierarchy():
    sources = {
        name: (ROOT / "src" / name).read_text()
        for name in ("voice.h", "midi.h", "video.h")
    }
    assert "class WavePlaybackDeviceInterface : public Sound" in sources["voice.h"]
    assert "class VoiceRxDeviceInterface : public Sound" in sources["voice.h"]
    assert "class VoiceTxDeviceInterface : public Sound" in sources["voice.h"]
    assert "class Midi : public Sound" in sources["midi.h"]
    assert "class MidiDelegate" not in sources["midi.h"]
    assert "class VideoSoundDevice : public Sound" in sources["video.h"]

    midi = (ROOT / "src/midi.cpp").read_text()
    ambience = (ROOT / "src/ambience.cpp").read_text()
    assert "Midi *const child = static_cast<Midi *>(device_);" in midi
    assert "reinterpret_cast<Midi *>(device_)" not in midi
    assert "static_cast<Midi *>(device_)->set_mode(field_54_ & 1);" in ambience
    assert "reinterpret_cast<Midi *>(device_)" not in ambience
