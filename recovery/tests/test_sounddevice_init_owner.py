from __future__ import annotations

import shutil
import subprocess
import os
import shlex
from pathlib import Path

import pytest
from match_recovery.source import read_file
from match_recovery.source.model import State


SOURCE_ROOT = os.environ.get("OPENSMACX_SOURCE_ROOT")
ROOT = (Path(SOURCE_ROOT).resolve() if SOURCE_ROOT else
        next(p for p in Path(__file__).resolve().parents
             if (p / "src/sounddevice.cpp").is_file()))
SUBJECTS = {
    0x004C57A0: ("?init@Midi_Device@@QAEHPAXK@Z", "Midi_Device"),
    0x004C5A10: ("?init@Wave_In_Device@@QAEHPAXK@Z", "Wave_In_Device"),
}


def test_init_owner_bodies_are_extracted_from_installed_source(tmp_path):
    records = {r.address: r for r in read_file(ROOT / "src/sounddevice.cpp")}
    assert set(SUBJECTS) <= set(records)
    bodies = {}
    for address, (name, owner) in SUBJECTS.items():
        record = records[address]
        assert record.path == ROOT / "src/sounddevice.cpp"
        assert record.name == name
        assert record.state is State.IMPLEMENTED
        assert record.byte_exact
        assert record.image_spans == ((address, address + (68 if owner == "Midi_Device" else 61)),)
        assert record.region and f"0x{address:08X}" in record.region
        bodies[owner] = record.region

    midi, wave = bodies["Midi_Device"], bodies["Wave_In_Device"]
    header = (ROOT / "src/sounddevice.h").read_text()
    assert "SoundDeviceBase *device_14_;" in header
    assert "void *device_14_;" not in header
    for body in (midi, wave):
        assert "create_device(backends)" in body
        assert "device_14_->init(window, backends)" in body
        assert "delete_device();" in body
        assert "DeviceVCall" not in body
        assert "reinterpret_cast<uint8_t *>(this) + 0x14" not in body
        assert "restart()" not in body
    assert "device_14_ = nullptr;" in midi
    assert "device_14_ = nullptr;" not in wave

    configured = os.environ.get("CXX")
    if configured:
        command = shlex.split(configured)
        if not command or not shutil.which(command[0]):
            pytest.skip(f"configured CXX is unavailable: {configured}")
        compilers = [command]
    else:
        compilers = [[path] for name in ("c++", "clang++")
                     if (path := shutil.which(name))]
    if not compilers:
        pytest.skip("host C++ compiler unavailable; actual-source assertions passed")
    harness = r'''
#include <cassert>
struct SoundDeviceBase {
  SoundDeviceBase *device_14_ = 0;
  virtual int create_device(unsigned long) = 0;
  virtual int delete_device() = 0;
  virtual int init(void *, unsigned long) = 0;
};
struct Midi_Device : SoundDeviceBase {
  int init(void *window, unsigned long backends) override;
};
struct Wave_In_Device : SoundDeviceBase {
  int init(void *window, unsigned long backends) override;
};
struct FakeBackend : SoundDeviceBase {
  int result;
  void *window = 0;
  unsigned long backends = 0;
  explicit FakeBackend(int r) : result(r) {}
  int create_device(unsigned long) override { return 0; }
  int delete_device() override { return 0; }
  int init(void *w, unsigned long b) override { window = w; backends = b; return result; }
};
struct MidiProbe : Midi_Device {
  int make_result = 0, delete_result = 0;
  int creates = 0, deletes = 0;
  unsigned long create_flags = 0;
  int create_device(unsigned long b) override { ++creates; create_flags = b; return make_result; }
  int delete_device() override { ++deletes; return delete_result; }
};
struct WaveProbe : Wave_In_Device {
  int make_result = 0, delete_result = 0;
  int creates = 0, deletes = 0;
  unsigned long create_flags = 0;
  int create_device(unsigned long b) override { ++creates; create_flags = b; return make_result; }
  int delete_device() override { ++deletes; return delete_result; }
};
'''
    source = "\n".join((
        harness,
        bodies["Midi_Device"],
        bodies["Wave_In_Device"],
        r'''
int main() {
  int marker = 0;
  MidiProbe m; FakeBackend m_backend(0x31); m.device_14_ = &m_backend;
  m.delete_result = 0x99;
  assert(m.init(&marker, 0x12345678) == 0x31);
  assert(m.creates == 1 && m.deletes == 1 && m.create_flags == 0x12345678);
  assert(m_backend.window == &marker && m_backend.backends == 0x12345678);
  assert(m.device_14_ == 0); // unconditional wrapper clear on Midi failure

  WaveProbe w; FakeBackend w_backend(0x42); w.device_14_ = &w_backend;
  w.delete_result = 0x98;
  assert(w.init(&marker, 0x87654321) == 0x42);
  assert(w.creates == 1 && w.deletes == 1 && w.create_flags == 0x87654321);
  assert(w_backend.window == &marker && w_backend.backends == 0x87654321);
  assert(w.device_14_ == &w_backend); // Wave init adds no unconditional clear

  MidiProbe m_create_failure; m_create_failure.make_result = 0x13;
  assert(m_create_failure.init(&marker, 7) == 0x13);
  assert(m_create_failure.deletes == 0 && m_create_failure.creates == 1);
  WaveProbe w_create_failure; w_create_failure.make_result = 0x14;
  assert(w_create_failure.init(&marker, 8) == 0x14);
  assert(w_create_failure.deletes == 0 && w_create_failure.creates == 1);

  MidiProbe m_success; FakeBackend m_ok(0); m_success.device_14_ = &m_ok;
  assert(m_success.init(&marker, 9) == 0 && m_success.deletes == 0);
  WaveProbe w_success; FakeBackend w_ok(0); w_success.device_14_ = &w_ok;
  assert(w_success.init(&marker, 10) == 0 && w_success.deletes == 0);
}
''',
    ))
    cpp = tmp_path / "sounddevice_init_actual_source.cpp"
    cpp.write_text(source)
    for index, command in enumerate(compilers):
        exe = tmp_path / f"sounddevice_init_actual_source_{index}"
        subprocess.run([*command, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
