"""Ignored actual-source behavior controls for the sound-device release edit."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
COMPILERS = ("g++", "clang++")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if line_comment:
            if ch == "\n":
                line_comment = False
        elif block_comment:
            if ch == "*" and nxt == "/":
                block_comment = False
                i += 1
        elif quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
        elif ch == "/" and nxt == "/":
            line_comment = True
            i += 1
        elif ch == "/" and nxt == "*":
            block_comment = True
            i += 1
        elif ch in "\"'":
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:i]
        i += 1
    raise AssertionError(f"unterminated body: {signature}")


def production_bodies():
    sound = (ROOT / "src/sounddevice.cpp").read_text()
    sound_header = (ROOT / "src/sounddevice.h").read_text()
    wave = (ROOT / "src/wave_device.cpp").read_text()
    return {
        "base": body(sound_header, "inline SoundDeviceBase::~SoundDeviceBase()"),
        "midi": body(sound, "int Midi_Device::release()"),
        "wave_in": body(sound, "int Wave_In_Device::release()"),
        "wave_in_delete": body(sound, "int Wave_In_Device::delete_device()"),
        "wave": body(wave, "int Wave_Device::release()"),
    }


def test_typed_dispatch_source_has_guards_rereads_and_wrapper_calls():
    b = production_bodies()
    assert "if (device)" in b["base"] and "SoundXDeviceAbi::release_device(device);" in b["base"]
    assert "vtable_slot<" not in b["base"]

    midi = b["midi"]
    assert "if (device_14_ != 0)" in midi
    assert "SoundXDeviceAbi::release_device(device_14_);" in midi
    after_release = midi.index("SoundXDeviceAbi::release_device(device_14_);")
    assert "if (device_14_ != 0)" in midi[after_release:]
    assert "SoundXOrdinal8MidiDeviceDelete()" in midi
    assert re.search(r"return 0;\s*$", midi)

    wave_in = b["wave_in"]
    assert "SoundXDeviceAbi::release_device(device_14_);" in wave_in
    assert wave_in.index("SoundXDeviceAbi::release_device(device_14_);") < wave_in.index("delete_device();")
    assert wave_in.index("delete_device();") < wave_in.index("device_14_ = 0;")
    assert "device_14_ != 0 && SoundXOrdinal10WaveInDeviceDelete() != 0" in b["wave_in_delete"]

    wave = b["wave"]
    assert "static_cast<WaveDeviceProviderCallView *>(device_14_)->release();" in wave
    assert wave.index("static_cast<WaveDeviceProviderCallView *>(device_14_)->release();") < wave.index("if (device_14_)", wave.index("static_cast<WaveDeviceProviderCallView *>(device_14_)->release();"))
    assert "SoundXOrdinal6WaveDeviceDelete()" in wave and "device_14_ = nullptr;" in wave
    assert re.search(r"return 0;\s*$", wave)


HARNESS_PREFIX = r'''#include <cassert>
#include <cstdint>
typedef void *HWND;
struct EAX_REVERB_PROPERTIES;
#include "soundx_wave_device_abi.h"

typedef int (*SoundXMidiDeviceDeleteProc)();
typedef int (*SoundXWaveInDeviceDeleteProc)();
typedef int (*SoundXWaveDeviceDeleteProc)();
static void *SoundDllProcs[11] = {};
static int midi_close_calls = 0, wave_in_close_calls = 0, wave_destroy_calls = 0;
static int midi_close_fn() { ++midi_close_calls; return -11; }
static int wave_in_close_fn() { ++wave_in_close_calls; return -12; }
static int wave_destroy_fn() { ++wave_destroy_calls; return -13; }
static SoundXMidiDeviceDeleteProc SoundXOrdinal8MidiDeviceDelete() {
  return reinterpret_cast<SoundXMidiDeviceDeleteProc>(SoundDllProcs[7]);
}
static SoundXWaveInDeviceDeleteProc SoundXOrdinal10WaveInDeviceDelete() {
  return reinterpret_cast<SoundXWaveInDeviceDeleteProc>(SoundDllProcs[9]);
}
static SoundXWaveDeviceDeleteProc SoundXOrdinal6WaveDeviceDelete() {
  return reinterpret_cast<SoundXWaveDeviceDeleteProc>(SoundDllProcs[5]);
}

struct SoundDeviceBase {
  void *device_14_;
  SoundDeviceBase() : device_14_(0) {}
  virtual int release() = 0;
  ~SoundDeviceBase();
};
struct Midi_Device : SoundDeviceBase { int release(); };
struct Wave_In_Device : SoundDeviceBase { int release(); virtual int delete_device(); };
struct Wave_Device : SoundDeviceBase { int release(); };

struct Backend {
  void **vptr;
  void **owner_slot;
  void *replacement;
  bool mutate_owner;
  int value, calls;
  Backend() : vptr(0), owner_slot(0), replacement(0), mutate_owner(false), value(0x51), calls(0) {}
  int release() {
    ++calls;
    if (mutate_owner && owner_slot) *owner_slot = replacement;
    return value;
  }
};
namespace SoundXDeviceAbi {
void release_device(void *device) { static_cast<Backend *>(device)->release(); }
}
// Host-only fake provider. Empty bodies exist only to satisfy the borrowed
// abstract call view; release alone participates in these behavior controls.
struct WaveProviderMock : WaveDeviceProviderCallView {
  Backend *backend;
  explicit WaveProviderMock(Backend *value) : backend(value) {}
  int reserved_00(uint32_t) override { return 0; }
  int reserved_04() override { return 0; }
  void reserved_08() override {}
  int init(void *, unsigned long) override { return 0; }
  void release() override { backend->release(); }
  int get_ndevices() override { return 0; }
  int select(unsigned long) override { return 0; }
  void get_description(unsigned long, char *, unsigned long) override {}
  void set_volume(uint32_t) override {}
  int reserved_24() override { return 0; }
  void reserved_28(uint32_t) override {}
  int reserved_2c(uint32_t) override { return 0; }
  int reserved_30() override { return 0; }
  void update_sound() override {}
  void set_rate(uint32_t) override {}
  int get_rate() override { return 0; }
  void reserved_40(uint32_t) override {}
  uint32_t reserved_44() override { return 0; }
  void suspend() override {}
  void restart() override {}
  int start_raw_dump(char *) override { return 0; }
  int stop_raw_dump() override { return 0; }
  int get_hw_mem_size() override { return 0; }
  void reserved_5c() override {}
  void enable() override {}
  void disable() override {}
  int is_disabled() override { return 0; }
  int set_hwnd(HWND) override { return 0; }
  IDirectSound *get_ds() override { return 0; }
  bool is_eax() override { return false; }
  int set_eax(EAX_REVERB_PROPERTIES *) override { return 0; }
  int set_eax(uint32_t) override { return 0; }
  int set_eax_mix(float) override { return 0; }
  bool is_3d() override { return false; }
  int set_listener_position(float, float, float) override { return 0; }
  int get_listener_position(float *, float *, float *) override { return 0; }
  int set_listener_xpos(float) override { return 0; }
  int get_listener_xpos(float *) override { return 0; }
  int set_listener_ypos(float) override { return 0; }
  int get_listener_ypos(float *) override { return 0; }
  int set_listener_zpos(float) override { return 0; }
  int get_listener_zpos(float *) override { return 0; }
};
struct WaveInProbe : Wave_In_Device {
  int deletes;
  WaveInProbe() : deletes(0) {}
  int delete_device() override { ++deletes; return Wave_In_Device::delete_device(); }
};
struct MidiProbe : Midi_Device {
  int nested_release_calls;
  MidiProbe() : nested_release_calls(0) {}
  int release() override {
    if (nested_release_calls++) return 0x77;
    return Midi_Device::release();
  }
};

'''


HARNESS_MAIN = r'''
int main() {
  // The base destructor uses its captured wrapped pointer. The backend clears
  // the live member during its callback. Observe the backend after ordinary
  // scope exit without reading the already-destroyed owner's fields.
  Backend base_backend;
  struct BaseProbe : SoundDeviceBase { int release() override { return 0; } };
  {
    BaseProbe owner;
    owner.device_14_ = &base_backend;
    base_backend.owner_slot = &owner.device_14_;
    base_backend.mutate_owner = true; // replacement defaults to null
  }
  assert(base_backend.calls == 1);
  Backend null_base_backend;
  {
    BaseProbe null_owner;
    null_base_backend.owner_slot = &null_owner.device_14_;
    null_base_backend.mutate_owner = true;
  }
  assert(null_base_backend.calls == 0); // null guard

  // Midi callback clears the field: close is skipped after the required reread.
  Backend midi_backend;
  MidiProbe midi;
  midi.device_14_ = &midi_backend;
  midi_backend.owner_slot = &midi.device_14_;
  midi_backend.mutate_owner = true;
  SoundDllProcs[7] = reinterpret_cast<void *>(&midi_close_fn);
  assert(midi.release() == 0 && midi_backend.calls == 1);
  assert(midi_close_calls == 0 && midi.device_14_ == 0);

  // A callback that rebinds the field causes the close hook to run on the
  // wrapper's post-callback path, then the wrapper clears the field.
  Backend midi_rebind_backend, midi_replacement;
  MidiProbe midi_rebind;
  midi_rebind.device_14_ = &midi_rebind_backend;
  midi_rebind_backend.owner_slot = &midi_rebind.device_14_;
  midi_rebind_backend.replacement = &midi_replacement;
  midi_rebind_backend.mutate_owner = true;
  assert(midi_rebind.release() == 0 && midi_close_calls == 1);
  assert(midi_rebind.device_14_ == 0);

  // Wave-In calls the virtual wrapper override, which delegates to the actual
  // delete_device body. A null callback result skips close; a rebound pointer
  // reaches the configured DLL close hook.
  SoundDllProcs[9] = reinterpret_cast<void *>(&wave_in_close_fn);
  Backend in_backend;
  WaveInProbe wave_in;
  wave_in.device_14_ = &in_backend;
  in_backend.owner_slot = &wave_in.device_14_;
  in_backend.mutate_owner = true;
  assert(wave_in.release() == 0 && in_backend.calls == 1);
  assert(wave_in.deletes == 1 && wave_in_close_calls == 0 && wave_in.device_14_ == 0);

  Backend in_rebind_backend, in_replacement;
  WaveInProbe wave_in_rebind;
  wave_in_rebind.device_14_ = &in_rebind_backend;
  in_rebind_backend.owner_slot = &wave_in_rebind.device_14_;
  in_rebind_backend.replacement = &in_replacement;
  in_rebind_backend.mutate_owner = true;
  assert(wave_in_rebind.release() == 0 && wave_in_rebind.deletes == 1);
  assert(wave_in_close_calls == 1 && wave_in_rebind.device_14_ == 0);

  // Wave_Device likewise rereads after backend release: null skips destruction,
  // rebound field invokes the hook and clears it.
  SoundDllProcs[5] = reinterpret_cast<void *>(&wave_destroy_fn);
  Backend wave_backend;
  WaveProviderMock wave_provider(&wave_backend);
  Wave_Device wave;
  wave.device_14_ = &wave_provider;
  wave_backend.owner_slot = &wave.device_14_;
  wave_backend.mutate_owner = true;
  assert(wave.release() == 0 && wave_backend.calls == 1);
  assert(wave_destroy_calls == 0 && wave.device_14_ == 0);

  Backend wave_rebind_backend, wave_replacement;
  WaveProviderMock wave_rebind_provider(&wave_rebind_backend);
  Wave_Device wave_rebind;
  wave_rebind.device_14_ = &wave_rebind_provider;
  wave_rebind_backend.owner_slot = &wave_rebind.device_14_;
  wave_rebind_backend.replacement = &wave_replacement;
  wave_rebind_backend.mutate_owner = true;
  assert(wave_rebind.release() == 0 && wave_destroy_calls == 1);
  assert(wave_rebind.device_14_ == 0);

  Midi_Device null_midi; assert(null_midi.release() == 0);
  Wave_In_Device null_in; assert(null_in.release() == 0);
  Wave_Device null_wave; assert(null_wave.release() == 0);
  assert(midi_close_calls == 1 && wave_in_close_calls == 1 && wave_destroy_calls == 1);
  return 0;
}
'''


def harness_source(bodies: dict[str, str], mutant: str | None = None) -> str:
    b = dict(bodies)
    if mutant == "reroute":
        b["midi"] = b["midi"].replace("SoundXDeviceAbi::release_device(device_14_);", "this->release();", 1)
    elif mutant == "drop_wavein_delete":
        b["wave_in"] = b["wave_in"].replace("delete_device();", "/* removed delete */", 1)
    elif mutant == "return_backend":
        b["midi"] = b["midi"].replace("return 0;", "return 0x51;", 1)
    elif mutant == "stale_reread":
        b["midi"] = "void *stale_device = device_14_;\n" + b["midi"]
        call = b["midi"].index("SoundXDeviceAbi::release_device(device_14_);")
        post = b["midi"][call:]
        post = post.replace("if (device_14_ != 0)", "if (stale_device != 0)", 1)
        b["midi"] = b["midi"][:call] + post

    return (
        HARNESS_PREFIX
        + "SoundDeviceBase::~SoundDeviceBase() {" + b["base"] + "}\n"
        + "int Midi_Device::release() {" + b["midi"] + "}\n"
        + "int Wave_In_Device::release() {" + b["wave_in"] + "}\n"
        + "int Wave_In_Device::delete_device() {" + b["wave_in_delete"] + "}\n"
        + "int Wave_Device::release() {" + b["wave"] + "}\n"
        + HARNESS_MAIN
    )


@pytest.mark.parametrize("compiler_name", COMPILERS)
def test_extracted_product_bodies_pass_callback_and_override_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = production_bodies()
    assert all("vtable_slot<" not in bodies[name]
               for name in ("base", "midi", "wave_in"))
    source = tmp_path / f"sounddevice_release_{compiler_name}.cpp"
    source.write_text(harness_source(bodies))
    binary = tmp_path / f"sounddevice_release_{compiler_name}"
    (tmp_path / "soundx_wave_device_abi.h").write_text((ROOT / "src/soundx_wave_device_abi.h").read_text())
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True, timeout=10)

    sanitized = tmp_path / f"sounddevice_release_{compiler_name}_ubsan"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(source), "-o", str(sanitized)], check=True)
    subprocess.run([str(sanitized)], check=True, timeout=10)


@pytest.mark.parametrize("compiler_name", COMPILERS)
@pytest.mark.parametrize("mutant", ("reroute", "drop_wavein_delete", "return_backend", "stale_reread"))
def test_compiled_behavior_controls_reject_targeted_mutants(tmp_path, compiler_name, mutant):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / f"sounddevice_release_{mutant}_{compiler_name}.cpp"
    source.write_text(harness_source(production_bodies(), mutant))
    (tmp_path / "soundx_wave_device_abi.h").write_text((ROOT / "src/soundx_wave_device_abi.h").read_text())
    binary = tmp_path / f"sounddevice_release_{mutant}_{compiler_name}"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    expected_failure = {
        "reroute": "midi_backend.calls == 1",
        "drop_wavein_delete": "wave_in.deletes == 1",
        "return_backend": "midi.release() == 0",
        "stale_reread": "midi_close_calls == 0",
    }[mutant]
    assert result.returncode != 0, f"{mutant} unexpectedly passed with {compiler_name}"
    assert expected_failure.encode() in result.stderr, (
        f"{mutant} failed for an unexpected reason with {compiler_name}: "
        f"{result.stderr.decode(errors='replace')}"
    )
