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

from test_sprite_list_lifecycle_actual_source import extract_definition, repo_root


ROOT = repo_root()


WAVE_METHODS = (
    ("int Wave_Device::enable()", "int enable();"),
    ("int Wave_Device::disable()", "int disable();"),
    ("void Wave_Device::update_sound()", "void update_sound();"),
    ("int Wave_Device::get_ndevices()", "int get_ndevices();"),
    ("int Wave_Device::get_hw_mem_size()", "int get_hw_mem_size();"),
    ("int Wave_Device::get_rate()", "int get_rate();"),
    ("int Wave_Device::get_ds()", "int get_ds();"),
    ("int Wave_Device::is_disabled()", "int is_disabled();"),
    ("int Wave_Device::stop_raw_dump()", "int stop_raw_dump();"),
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
    sound_helpers = namespace_helpers(sound_cpp, "typedef int(__fastcall *sound_device_query)")
    sound_methods = "\n".join(
        extract_definition(sound_cpp, signature)
        for signature in ("int Sound::is_playing()", "int Sound::is_looping()", "int Sound::get_time()"))
    wave_helpers = namespace_helpers(wave_cpp, "typedef void(__fastcall *device_vfn)")
    wave_methods = "\n".join(extract_definition(wave_cpp, signature)
                                 for signature, _ in WAVE_METHODS)
    declarations = "\n".join("    " + decl for _, decl in WAVE_METHODS)
    return r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#define __fastcall
#define __forceinline inline
#define ORIGINAL(p) reinterpret_cast<OriginalObject *>(p)

class OriginalObject {
public:
    int arg_rate(int value);
    int arg_volume(int value);
    int arg_hwnd(int value);
};
// Host-only adapter: 32-bit original byte offsets map to separate host slots
// so 64-bit function/pointer-to-member representations cannot overlap.
template<class Fn> Fn &vtable_slot(const void *object, unsigned long offset) {
    const unsigned char *const table = *reinterpret_cast<const unsigned char *const *>(object);
    return *reinterpret_cast<Fn *>(const_cast<unsigned char *>(table) + offset * 4);
}
typedef void *SoundDllProcsTable[11];
SoundDllProcsTable SoundDllProcs = {};
struct SoundFadeState {};
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
''' + sound_helpers + "\n" + sound_methods + "\n" + wave_helpers + "\n" + wave_methods + r'''
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

static int q_s_play(void *p) { record(0x5C, p); return answer; }
static int q_s_loop(void *p) { record(0x58, p); return answer; }
static int q_s_time(void *p) { record(0x74, p); return answer; }
static int q_14(void *p) { record(0x14, p); return answer; }
static int q_58(void *p) { record(0x58, p); return answer; }
static int q_3c(void *p) { record(0x3C, p); return answer; }
static int q_70(void *p) { record(0x70, p); return answer; }
static int q_68(void *p) { record(0x68, p); return answer; }
static int q_54(void *p) { record(0x54, p); return answer; }
static void d_60(void *p) { record(0x60, p); }
static void d_64(void *p) { record(0x64, p); }
static void d_34(void *p) { record(0x34, p); }

int OriginalObject::arg_rate(int value) {
    record(0x38, this); last_argument = value; return answer;
}
int OriginalObject::arg_volume(int value) {
    record(0x20, this); last_argument = value; return answer;
}
int OriginalObject::arg_hwnd(int value) {
    record(0x6C, this); last_argument = value; return answer;
}
union VtableBytes { void *align; unsigned char bytes[1024]; };
struct Provider { const unsigned char *vtable; };
template<class Function> static void install(VtableBytes &table, unsigned offset, Function fn) {
    std::memcpy(table.bytes + offset * 4, &fn, sizeof(fn));
}
static void install_arg(VtableBytes &table, unsigned offset,
                        int (OriginalObject::*fn)(int)) {
    std::memcpy(table.bytes + offset * 4, &fn, sizeof(fn));
}
static Provider make_provider(VtableBytes &table) { Provider p = { table.bytes }; return p; }
static void install_sound(VtableBytes &t) {
    install(t, 0x5C, &q_s_play); install(t, 0x58, &q_s_loop); install(t, 0x74, &q_s_time);
}
static void install_wave(VtableBytes &t) {
    install(t, 0x60, &d_60); install(t, 0x64, &d_64); install(t, 0x34, &d_34);
    install(t, 0x14, &q_14); install(t, 0x58, &q_58); install(t, 0x3C, &q_3c);
    install(t, 0x70, &q_70); install(t, 0x68, &q_68); install(t, 0x54, &q_54);
    install_arg(t, 0x38, &OriginalObject::arg_rate);
    install_arg(t, 0x20, &OriginalObject::arg_volume);
    install_arg(t, 0x6C, &OriginalObject::arg_hwnd);
}
int main() {
    int failures = 0;
    VtableBytes sound_table = {};
    Provider sound_provider = make_provider(sound_table);
    install_sound(sound_table);
    Sound sound;
    failures += check("Sound null defaults", sound.is_playing() == 0 && sound.is_looping() == 0 && sound.get_time() == 0 && calls == 0);
    sound.set_device(&sound_provider);
    answer = 73;
    reset_trace(); failures += check("Sound is_playing member source", sound.is_playing() == 73 && last_slot == 0x5C && last_receiver == &sound_provider && calls == 1);
    reset_trace(); failures += check("Sound is_looping member source", sound.is_looping() == 73 && last_slot == 0x58 && last_receiver == &sound_provider && calls == 1);
    reset_trace(); failures += check("Sound get_time member source", sound.get_time() == 73 && last_slot == 0x74 && last_receiver == &sound_provider && calls == 1);

    VtableBytes wave_table = {};
    Provider wave_provider = make_provider(wave_table);
    install_wave(wave_table);
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
    reset_trace(); failures += check("DirectSound query", wave.get_ds() == 91 && last_slot == 0x70);
    reset_trace(); failures += check("disabled query", wave.is_disabled() == 91 && last_slot == 0x68);
    reset_trace(); failures += check("raw dump query", wave.stop_raw_dump() == 91 && last_slot == 0x54);
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
    assert "query_sound_device(void *device" in sound_cpp
    assert "query_sound_device(device_, 0x5C)" in sound_cpp
    assert "wrapped_device(Wave_Device *self)" not in wave_cpp
    assert "dispatch_wrapped_device(device_14_, 0x60)" in wave_cpp
    assert "forward_to_wrapped_device(device_14_, 0x6C" in wave_cpp
    cpp = tmp_path / "sound_lifecycle_actual_owner_fields.cpp"
    exe = tmp_path / "sound_lifecycle_actual_owner_fields"
    cpp.write_text(product_host_program(sound_cpp, wave_cpp))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr
