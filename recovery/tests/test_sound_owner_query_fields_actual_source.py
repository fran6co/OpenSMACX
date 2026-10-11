"""Host controls for source-owned Sound/Wave_Device wrapped-device fields.

Complete product helpers and member functions are extracted verbatim. The
provider, its vtable bytes, pointer-to-member slots and 32-bit wrapper layouts
are deliberately host test doubles, not an SDK ABI or target-layout proof.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from test_spritelist_base_lifetime import extract_definition, repo_root


ROOT = repo_root()


WAVE_METHODS = (
    ("int Wave_Device::enable()", "int enable();"),
    ("int Wave_Device::disable()", "int disable();"),
    ("void Wave_Device::update_sound()", "void update_sound();"),
    ("int Wave_Device::get_ndevices()", "int get_ndevices();"),
    ("int Wave_Device::get_hw_mem_size()", "int get_hw_mem_size();"),
    ("int Wave_Device::get_rate()", "int get_rate();"),
    ("IDirectSound *Wave_Device::get_ds()", "IDirectSound *get_ds();"),
    ("bool Wave_Device::is_eax()", "bool is_eax();"),
    ("int Wave_Device::is_disabled()", "int is_disabled();"),
    ("int Wave_Device::stop_raw_dump()", "int stop_raw_dump();"),
    ("bool Wave_Device::is_3d()", "bool is_3d();"),
    ("void Wave_Device::set_rate(unsigned long rate)", "void set_rate(unsigned long);"),
    ("void Wave_Device::set_volume(unsigned long volume)", "void set_volume(unsigned long);"),
    ("int Wave_Device::set_hwnd(void *hwnd)", "int set_hwnd(void *);"),
)


def namespace_helpers(source: str, marker: str) -> str:
    point = source.index(marker)
    start = source.rfind("namespace {", 0, point)
    close = source.index("}  // namespace", point) + len("}  // namespace")
    assert start >= 0
    return source[start:close]


def product_host_program(sound_cpp: str, wave_cpp: str) -> str:
    sound_helpers = namespace_helpers(sound_cpp, "__forceinline SoundDeviceInterface *sound_device")
    sound_methods = "\n".join(
        extract_definition(sound_cpp, signature)
        for signature in ("int Sound::is_playing()", "int Sound::is_looping()", "int Sound::get_time()"))
    wave_methods = "\n".join(extract_definition(wave_cpp, signature)
                                 for signature, _ in WAVE_METHODS)
    declarations = "\n".join("    " + decl for _, decl in WAVE_METHODS)
    return r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#define __forceinline inline
typedef void *HWND;
struct EAX_REVERB_PROPERTIES;
#include "soundx_wave_device_abi.h"
typedef void *SoundDllProcsTable[11];
SoundDllProcsTable SoundDllProcs = {};
struct SoundFadeState {};
class SoundDeviceInterface {
public:
    virtual int is_playing() = 0;
    virtual int is_looping() = 0;
    virtual int get_time() = 0;
};
class Sound {
protected:
    void *device_;
public:
    Sound() : device_(0) {}
    void set_device(void *device) { device_ = device; }
    int is_playing();
    int is_looping();
    int get_time();
};
class SoundDeviceBase {
protected:
    void *device_14_;
public:
    SoundDeviceBase() : device_14_(0) {}
    void set_device(void *device) { device_14_ = device; }
};
class Wave_Device : public SoundDeviceBase {
public:
''' + declarations + r'''
};
''' + sound_helpers + "\n" + sound_methods + "\n" + wave_methods + r'''
static int calls = 0;
static int last_slot = -1;
static void *last_receiver = 0;
static int last_argument = 0;
static int answer = 0;
static int check(const char *name, bool value) {
    if (!value) std::fprintf(stderr, "FAIL: %s\n", name);
    return value ? 0 : 1;
}
static void record(int slot, void *receiver) {
    ++calls; last_slot = slot; last_receiver = receiver;
}
static void reset_trace() { calls = 0; last_slot = -1; last_receiver = 0; last_argument = 0; }

static int q_14(void *p) { record(0x14, p); return answer; }
static int q_58(void *p) { record(0x58, p); return answer; }
static int q_3c(void *p) { record(0x3C, p); return answer; }
struct IDirectSound {};
static IDirectSound direct_sound_answer;
static IDirectSound *q_70(void *p) { record(0x70, p); return &direct_sound_answer; }
static int q_68(void *p) { record(0x68, p); return answer; }
static int q_54(void *p) { record(0x54, p); return answer; }
static bool q_eax(void *p) { record(0x74, p); return answer != 0; }
static bool q_3d(void *p) { record(0x84, p); return answer != 0; }
static void d_60(void *p) { record(0x60, p); }
static void d_64(void *p) { record(0x64, p); }
static void d_34(void *p) { record(0x34, p); }
static void d_rate(void *p, uint32_t value) { record(0x38, p); last_argument = value; }
static void d_volume(void *p, uint32_t value) { record(0x20, p); last_argument = value; }
static int q_hwnd(void *p, HWND value) { record(0x6C, p); last_argument = (int)(intptr_t)value; return answer; }
struct SoundMock : SoundDeviceInterface {
    int is_playing() { record(0x5C, this); return answer; }
    int is_looping() { record(0x58, this); return answer; }
    int get_time() { record(0x74, this); return answer; }
};
union VtableBytes { void *align; void *slots[42]; };
struct Provider { void **vtable; };
static void install_wave(VtableBytes &t) {
    t.slots[0x60 / 4] = reinterpret_cast<void *>(&d_60);
    t.slots[0x64 / 4] = reinterpret_cast<void *>(&d_64);
    t.slots[0x34 / 4] = reinterpret_cast<void *>(&d_34);
    t.slots[0x14 / 4] = reinterpret_cast<void *>(&q_14);
    t.slots[0x58 / 4] = reinterpret_cast<void *>(&q_58);
    t.slots[0x3C / 4] = reinterpret_cast<void *>(&q_3c);
    t.slots[0x70 / 4] = reinterpret_cast<void *>(&q_70);
    t.slots[0x68 / 4] = reinterpret_cast<void *>(&q_68);
    t.slots[0x54 / 4] = reinterpret_cast<void *>(&q_54);
    t.slots[0x74 / 4] = reinterpret_cast<void *>(&q_eax);
    t.slots[0x84 / 4] = reinterpret_cast<void *>(&q_3d);
    t.slots[0x38 / 4] = reinterpret_cast<void *>(&d_rate);
    t.slots[0x20 / 4] = reinterpret_cast<void *>(&d_volume);
    t.slots[0x6C / 4] = reinterpret_cast<void *>(&q_hwnd);
}
int main() {
    int failures = 0;
    SoundMock sound_provider_storage;
    SoundDeviceInterface *sound_provider = &sound_provider_storage;
    Sound sound;
    failures += check("Sound null defaults", sound.is_playing() == 0 && sound.is_looping() == 0 && sound.get_time() == 0 && calls == 0);
    sound.set_device(sound_provider);
    answer = 73;
    reset_trace(); failures += check("Sound is_playing member source", sound.is_playing() == 73 && last_slot == 0x5C && last_receiver == sound_provider && calls == 1);
    reset_trace(); failures += check("Sound is_looping member source", sound.is_looping() == 73 && last_slot == 0x58 && last_receiver == sound_provider && calls == 1);
    reset_trace(); failures += check("Sound get_time member source", sound.get_time() == 73 && last_slot == 0x74 && last_receiver == sound_provider && calls == 1);

    VtableBytes wave_table = {};
    install_wave(wave_table);
    Provider wave_provider = { wave_table.slots };
    Wave_Device wave;
    reset_trace();
    failures += check("Wave_Device null defaults", wave.get_ndevices() == 0 && wave.get_hw_mem_size() == 0 && wave.get_rate() == 0 && wave.get_ds() == 0 && wave.is_disabled() == 1 && wave.stop_raw_dump() == 3 && wave.set_hwnd(reinterpret_cast<void *>(0x1234)) == 0x13 && calls == 0);
    wave.enable(); failures += check("enable dispatch null", calls == 0);
    wave.disable(); failures += check("disable dispatch null", calls == 0);
    wave.update_sound(); failures += check("update dispatch null", calls == 0);
    wave.set_rate(123); wave.set_volume(456); failures += check("argument dispatch null", calls == 0);

    wave.set_device(&wave_provider);
    answer = 91;
    reset_trace(); wave.enable(); failures += check("enable slot/receiver", calls == 1 && last_slot == 0x60 && last_receiver == &wave_provider);
    reset_trace(); wave.disable(); failures += check("disable slot/receiver", calls == 1 && last_slot == 0x64 && last_receiver == &wave_provider);
    reset_trace(); wave.update_sound(); failures += check("update slot/receiver", calls == 1 && last_slot == 0x34 && last_receiver == &wave_provider);
    reset_trace(); failures += check("query slots and results", wave.get_ndevices() == 91 && last_slot == 0x14 && last_receiver == &wave_provider && calls == 1);
    reset_trace(); failures += check("memory query", wave.get_hw_mem_size() == 91 && last_slot == 0x58);
    reset_trace(); failures += check("rate query", wave.get_rate() == 91 && last_slot == 0x3C);
    reset_trace(); failures += check("DirectSound query", wave.get_ds() == &direct_sound_answer && last_slot == 0x70);
    reset_trace(); failures += check("EAX byte query", wave.is_eax() && last_slot == 0x74 && last_receiver == &wave_provider && calls == 1);
    reset_trace(); failures += check("disabled query", wave.is_disabled() == 91 && last_slot == 0x68);
    reset_trace(); failures += check("raw dump query", wave.stop_raw_dump() == 91 && last_slot == 0x54);
    reset_trace(); failures += check("3D byte query", wave.is_3d() && last_slot == 0x84 && last_receiver == &wave_provider && calls == 1);
    answer = 0;
    reset_trace(); wave.set_rate(123); failures += check("rate argument path", calls == 1 && last_slot == 0x38 && last_receiver == &wave_provider && last_argument == 123);
    reset_trace(); wave.set_volume(456); failures += check("volume argument path", calls == 1 && last_slot == 0x20 && last_receiver == &wave_provider && last_argument == 456);
    reset_trace(); failures += check("hwnd argument/return path", wave.set_hwnd(reinterpret_cast<void *>(0x1234)) == 0 && calls == 1 && last_slot == 0x6C && last_receiver == &wave_provider && last_argument == 0x1234);
    return failures ? 1 : 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_owner_field_input_and_external_dispatch_behavior(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    sound_cpp = (ROOT / "src/sound.cpp").read_text()
    wave_cpp = (ROOT / "src/wave_device.cpp").read_text()
    sound_h = (ROOT / "src/sound.h").read_text()
    device_h = (ROOT / "src/sounddevice.h").read_text()
    assert "void *device_;" in sound_h and "void *device_14_;" in device_h
    assert "sound_device(device_)->is_playing()" in sound_cpp
    assert "sound_device(device_)->is_looping()" in sound_cpp
    assert "sound_device(device_)->get_time()" in sound_cpp
    assert "static_cast<WaveDeviceProviderCallView *>(device_14_)->enable()" in wave_cpp
    assert "static_cast<WaveDeviceProviderCallView *>(device_14_)->set_hwnd(static_cast<HWND>(hwnd))" in wave_cpp
    cpp = tmp_path / "sound_lifecycle_actual_owner_fields.cpp"
    exe = tmp_path / "sound_lifecycle_actual_owner_fields"
    (tmp_path / "soundx_wave_device_abi.h").write_text((ROOT / "src/soundx_wave_device_abi.h").read_text())
    cpp.write_text(product_host_program(sound_cpp, wave_cpp))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr
