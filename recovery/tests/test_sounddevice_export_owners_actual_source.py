"""Actual callback-cell users; host behavior, not original ABI/layout proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SLOTS = (
    ("SoundXMidiDeviceCreateProc", "SoundXOrdinal7MidiDeviceCreate", 6),
    ("SoundXMidiDeviceDeleteProc", "SoundXOrdinal8MidiDeviceDelete", 7),
    ("SoundXWaveInDeviceCreateProc", "SoundXOrdinal9WaveInDeviceCreate", 8),
    ("SoundXWaveInDeviceDeleteProc", "SoundXOrdinal10WaveInDeviceDelete", 9),
)


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def harness(source, header):
    start = header.index("typedef int (__cdecl *SoundXCreateSoundProc)")
    end = header.index("inline SoundXCreateSoundProc SoundXOrdinal1CreateSound()")
    aliases = header[start:end]
    helpers = "\n".join(definition(header, f"inline {name} {helper}()")
                        for name, helper, _ in SLOTS)
    methods = "\n".join(definition(source, f"int {owner}::{method}(")
                        for owner in ("Midi_Device", "Wave_In_Device")
                        for method in ("create_device", "delete_device"))
    return r'''
#include <cassert>
#define __cdecl
struct SoundDeviceBase {};
typedef void *SoundDllProcsTable[11];
SoundDllProcsTable SoundDllProcs = {};
''' + aliases + "\n" + helpers + r'''
struct Midi_Device {
    void *device_14_ = nullptr;
    int create_device(unsigned long);
    int delete_device();
};
struct Wave_In_Device {
    void *device_14_ = nullptr;
    int create_device(unsigned long);
    int delete_device();
};
''' + methods + r'''
SoundDeviceBase first, second;
unsigned long seen_flags;
int opens = 0, closes = 0;
int open_first(void **slot, unsigned long flags) {
    ++opens; seen_flags = flags;
    *static_cast<void **>(slot) = &first;
    return -7;
}
int open_second(void **slot, unsigned long flags) {
    ++opens; seen_flags = flags;
    *static_cast<void **>(slot) = &second;
    return 31;
}
int close_first() { closes += 1; return -1; }
int close_second() { closes += 10; return -2; }
template<class Owner> void run(int open_index, int close_index) {
    for (void *&entry : SoundDllProcs) entry = nullptr;
    Owner owner;
    opens = closes = 0;
    assert(owner.create_device(1) == 0x14 && !owner.device_14_);
    assert(owner.delete_device() == 0x14);
    SoundDllProcs[open_index] = reinterpret_cast<void *>(&open_first);
    assert(owner.create_device(0xffffffffUL) == -7);
    assert(seen_flags == 0xffffffffUL && owner.device_14_ == &first && opens == 1);
    assert(owner.create_device(2) == 0xc && opens == 1);
    assert(owner.delete_device() == 0x14 && owner.device_14_ == &first);
    SoundDllProcs[close_index] = reinterpret_cast<void *>(&close_first);
    assert(owner.delete_device() == 0 && closes == 1 && !owner.device_14_);
    SoundDllProcs[open_index] = reinterpret_cast<void *>(&open_second);
    SoundDllProcs[close_index] = reinterpret_cast<void *>(&close_second);
    assert(owner.create_device(37) == 31 && seen_flags == 37);
    assert(owner.device_14_ == &second && opens == 2);
    assert(owner.delete_device() == 0 && closes == 11 && !owner.device_14_);
    assert(owner.delete_device() == 0x14 && closes == 11);
}
int main() { run<Midi_Device>(6, 7); run<Wave_In_Device>(8, 9); }
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_live_loader_cells_and_wrong_slot_damage(tmp_path, compiler):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(f"{compiler} unavailable")
    source = (ROOT / "src/sounddevice.cpp").read_text()
    header = (ROOT / "src/sound.h").read_text()
    mutants = [header]
    for slot, wrong_slot in ((6, 8), (7, 9), (8, 6), (9, 7)):
        mutants.append(header.replace(f"SoundDllProcs[{slot}]",
                                      f"SoundDllProcs[{wrong_slot}]", 1))
    for index, mutant in enumerate(mutants):
        path = tmp_path / f"control-{index}.cpp"
        path.write_text(harness(source, mutant))
        binary = tmp_path / f"control-{index}"
        result = subprocess.run([executable, "-std=c++17", "-Wall", "-Wextra",
                                 "-Werror", str(path), "-o", str(binary)],
                                capture_output=True, text=True)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(binary)], capture_output=True, text=True)
        assert (result.returncode == 0) == (index == 0), result.stderr
