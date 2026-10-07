"""Actual callback-cell users; host behavior, not original ABI/layout proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SLOTS = (
    ("MidiDeviceOpen", "MidiDeviceOpenSlot", 6),
    ("MidiDeviceClose", "MidiDeviceCloseSlot", 7),
    ("WaveInDeviceOpen", "WaveInDeviceOpenSlot", 8),
    ("WaveInDeviceClose", "WaveInDeviceCloseSlot", 9),
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


def harness(source):
    header = (ROOT / "src/sounddevice.h").read_text()
    aliases = "\n".join(line for line in header.splitlines()
                        if line.startswith("typedef ") and any(
                            f"{name})" in line for name, _, _ in SLOTS))
    assert aliases.count("typedef") == 4
    helpers = "\n".join(definition(source, f"inline {name} *&{helper}()")
                        for name, helper, _ in SLOTS)
    methods = "\n".join(definition(source, f"int {owner}::{method}(")
                        for owner in ("Midi_Device", "Wave_In_Device")
                        for method in ("create_device", "delete_device"))
    return r'''
#include <cassert>
#define __cdecl
struct SoundDeviceBase {};
void *SoundDllProcs[11] = {};
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
int open_first(void *slot, unsigned long flags) {
    ++opens; seen_flags = flags;
    *static_cast<void **>(slot) = &first;
    return -7;
}
int open_second(void *slot, unsigned long flags) {
    ++opens; seen_flags = flags;
    *static_cast<void **>(slot) = &second;
    return 31;
}
void close_first() { closes += 1; }
void close_second() { closes += 10; }
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
    for index, mutant in enumerate([source] + [
        source.replace(f"&SoundDllProcs[{slot}]", f"&SoundDllProcs[{slot - 1}]", 1)
        for _, _, slot in SLOTS
    ]):
        path = tmp_path / f"control-{index}.cpp"
        path.write_text(harness(mutant))
        binary = tmp_path / f"control-{index}"
        result = subprocess.run([executable, "-std=c++17", "-Wall", "-Wextra",
                                 "-Werror", str(path), "-o", str(binary)],
                                capture_output=True, text=True)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(binary)], capture_output=True, text=True)
        assert (result.returncode == 0) == (index == 0), result.stderr
