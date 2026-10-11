"""Host controls for the actual Wave_Device provider call bodies.

These controls exercise wrapper decisions and argument flow on the host ABI.
They are not evidence that a host compiler reproduces VC6 thiscall or the
provider's physical vtable ABI; the coordinator's opt-in VC6 probes own that.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


METHODS = (
    "int Wave_Device::enable()", "int Wave_Device::disable()",
    "void Wave_Device::suspend()", "void Wave_Device::restart()",
    "void Wave_Device::update_sound()", "int Wave_Device::get_ndevices()",
    "int Wave_Device::get_hw_mem_size()", "int Wave_Device::get_rate()",
    "IDirectSound *Wave_Device::get_ds()", "bool Wave_Device::is_eax()",
    "int Wave_Device::is_disabled()", "int Wave_Device::stop_raw_dump()",
    "bool Wave_Device::is_3d()", "void Wave_Device::set_rate(unsigned long rate)",
    "void Wave_Device::set_volume(unsigned long volume)",
    "int Wave_Device::set_hwnd(void *hwnd)",
    "int Wave_Device::get_description(unsigned long a1, char *a2, unsigned long a3)",
    "int Wave_Device::start_raw_dump(char *path)",
    "int Wave_Device::set_eax(EAX_REVERB_PROPERTIES *properties)",
    "int Wave_Device::set_eax(unsigned long properties)",
    "int Wave_Device::set_eax_mix(float mix)",
    "int Wave_Device::set_listener_position(float x, float y, float z)",
    "int Wave_Device::get_listener_position(float *x, float *y, float *z)",
    "int Wave_Device::set_listener_xpos(float x)",
    "int Wave_Device::get_listener_xpos(float *x)",
    "int Wave_Device::set_listener_ypos(float y)",
    "int Wave_Device::get_listener_ypos(float *y)",
    "int Wave_Device::set_listener_zpos(float z)",
    "int Wave_Device::get_listener_zpos(float *z)",
    "int Wave_Device::create_device(unsigned long a1)",
    "int Wave_Device::delete_device()", "int Wave_Device::init(void *group_id, unsigned long flags)",
    "int Wave_Device::release()",
)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_provider_call_bodies_preserve_guards_arguments_and_order(tmp_path,
                                                                         compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/wave_device.cpp").read_text()
    header = (ROOT / "src/soundx_wave_device_abi.h").read_text()
    sound_header = (ROOT / "src/sound.h").read_text()
    assert "virtual ~WaveDeviceProviderCallView" not in header
    assert "class WaveDeviceProviderCallView" in header
    selected = "\n".join(definition(source, item) for item in METHODS)
    assert "SoundXDeviceAbi::" not in selected
    assert "vtable_slot<" not in selected and "ORIGINAL(" not in selected
    assert "dispatch_wrapped_device" not in selected

    harness = r'''#include <cassert>
#include <cstdint>
#include <cstring>
typedef void *HWND;
struct EAX_REVERB_PROPERTIES;
#include "soundx_wave_device_abi.h"

static int calls[42];
static int value_result = 37;
static int init_result = 0;
static unsigned long word_arg;
static HWND hwnd_arg;
static char *path_arg;
static EAX_REVERB_PROPERTIES *properties_arg;
static float float_args[3];
static float *float_ptrs[3];
static void *owner_to_clear;
static int factory_calls, destroy_calls;
static void *factory_provider;
static int factory_result;
static int call_order;

struct ProviderObject { void **vptr; } provider;
struct IDirectSound {};
static IDirectSound direct_sound;
static IDirectSound *provider_direct_sound(void *self) {
  assert(self == &provider); ++calls[1]; return &direct_sound;
}
static void provider_noarg_void(void *self) {
  assert(self == &provider); ++calls[0];
}
static int provider_noarg_int(void *self) {
  assert(self == &provider); ++calls[1]; return value_result;
}
static bool provider_noarg_bool(void *self) {
  assert(self == &provider); ++calls[2]; return value_result != 0;
}
static int provider_word_int(void *self, unsigned long value) {
  assert(self == &provider); word_arg = value; ++calls[3]; return value_result;
}
static void provider_word_void(void *self, uint32_t value) {
  assert(self == &provider); word_arg = value; ++calls[4];
}
static int provider_hwnd(void *self, HWND value) {
  assert(self == &provider); hwnd_arg = value; ++calls[5]; return value_result;
}
static void provider_description(void *self, unsigned long a, char *b,
                                 unsigned long c) {
  assert(self == &provider); word_arg = a; path_arg = b; float_args[0] = (float)c;
  ++calls[6];
}
static int provider_path(void *self, char *value) {
  assert(self == &provider); path_arg = value; ++calls[7]; return value_result;
}
static int provider_properties(void *self, EAX_REVERB_PROPERTIES *value) {
  assert(self == &provider); properties_arg = value; ++calls[8]; return value_result;
}
static int provider_mix(void *self, float value) {
  assert(self == &provider); float_args[0] = value; ++calls[9]; return value_result;
}
static int provider_three_floats(void *self, float a, float b, float c) {
  assert(self == &provider); float_args[0] = a; float_args[1] = b;
  float_args[2] = c; ++calls[10]; return value_result;
}
static int provider_three_ptrs(void *self, float *a, float *b, float *c) {
  assert(self == &provider); float_ptrs[0] = a; float_ptrs[1] = b;
  float_ptrs[2] = c; ++calls[11]; return value_result;
}
static int factory(void **slot, unsigned long flags) {
  ++factory_calls; word_arg = flags; *slot = factory_provider; return factory_result;
}
static int destroy() { ++destroy_calls; return -19; }
static int provider_init(void *self, void *group, unsigned long flags) {
  assert(self == &provider); assert(group == reinterpret_cast<void *>(0x1234));
  word_arg = flags; ++calls[12]; call_order = call_order * 10 + 1;
  return init_result;
}
static void provider_release(void *self) {
  assert(self == &provider); ++calls[13]; call_order = call_order * 10 + 2;
  if (owner_to_clear) *static_cast<void **>(owner_to_clear) = 0;
}

struct Wave_Device {
  void *device_14_;
  int enable(); int disable(); void suspend(); void restart();
  void update_sound(); int get_ndevices(); int get_hw_mem_size(); int get_rate();
  IDirectSound *get_ds(); bool is_eax(); int is_disabled(); int stop_raw_dump(); bool is_3d();
  void set_rate(unsigned long); void set_volume(unsigned long); int set_hwnd(void *);
  int get_description(unsigned long, char *, unsigned long); int start_raw_dump(char *);
  int set_eax(EAX_REVERB_PROPERTIES *); int set_eax(unsigned long); int set_eax_mix(float);
  int set_listener_position(float, float, float); int get_listener_position(float *, float *, float *);
  int set_listener_xpos(float); int get_listener_xpos(float *);
  int set_listener_ypos(float); int get_listener_ypos(float *);
  int set_listener_zpos(float); int get_listener_zpos(float *);
  int create_device(unsigned long); int delete_device();
  int init(void *, unsigned long); int release();
};

'''
    aliases_start = sound_header.index("typedef int (__cdecl *SoundXCreateSoundProc)")
    aliases_end = sound_header.index("inline SoundXCreateSoundProc SoundXOrdinal1CreateSound()")
    aliases = sound_header[aliases_start:aliases_end]
    export_accessors = "\n".join(definition(sound_header, signature) for signature in (
        "inline SoundXWaveDeviceCreateProc SoundXOrdinal5WaveDeviceCreate()",
        "inline SoundXWaveDeviceDeleteProc SoundXOrdinal6WaveDeviceDelete()",
    ))
    harness += "#define __cdecl\ntypedef void *SoundDllProcsTable[11];\nSoundDllProcsTable SoundDllProcs = {};\n" + aliases + export_accessors
    # The table uses the actual declaration order and only provides host
    # callbacks at consumed slots. This deliberately is not a DLL provider.
    slot_setup = r'''
static void *table[42] = {};
static void setup_provider() {
  table[3] = reinterpret_cast<void *>(&provider_init);
  table[4] = reinterpret_cast<void *>(&provider_release);
  table[5] = reinterpret_cast<void *>(&provider_noarg_int);
  table[6] = reinterpret_cast<void *>(&provider_word_int);
  table[7] = reinterpret_cast<void *>(&provider_description);
  table[8] = reinterpret_cast<void *>(&provider_word_void);
  table[13] = reinterpret_cast<void *>(&provider_noarg_void);
  table[14] = reinterpret_cast<void *>(&provider_word_void);
  table[15] = reinterpret_cast<void *>(&provider_noarg_int);
  table[18] = reinterpret_cast<void *>(&provider_noarg_void);
  table[19] = reinterpret_cast<void *>(&provider_noarg_void);
  table[20] = reinterpret_cast<void *>(&provider_path);
  table[21] = reinterpret_cast<void *>(&provider_noarg_int);
  table[22] = reinterpret_cast<void *>(&provider_noarg_int);
  table[24] = reinterpret_cast<void *>(&provider_noarg_void);
  table[25] = reinterpret_cast<void *>(&provider_noarg_void);
  table[26] = reinterpret_cast<void *>(&provider_noarg_int);
  table[27] = reinterpret_cast<void *>(&provider_hwnd);
  table[28] = reinterpret_cast<void *>(&provider_direct_sound);
  table[29] = reinterpret_cast<void *>(&provider_noarg_bool);
      // Host compiler overload order is intentionally only a host harness;
      // coordinator VC6 probes establish the target physical offsets.
      table[30] = reinterpret_cast<void *>(&provider_properties);
      table[31] = reinterpret_cast<void *>(&provider_word_int);
  table[32] = reinterpret_cast<void *>(&provider_mix);
  table[33] = reinterpret_cast<void *>(&provider_noarg_bool);
  table[34] = reinterpret_cast<void *>(&provider_three_floats);
  table[35] = reinterpret_cast<void *>(&provider_three_ptrs);
  table[36] = reinterpret_cast<void *>(&provider_mix);
  table[37] = reinterpret_cast<void *>(&provider_three_ptrs);
  table[38] = reinterpret_cast<void *>(&provider_mix);
  table[39] = reinterpret_cast<void *>(&provider_three_ptrs);
  table[40] = reinterpret_cast<void *>(&provider_mix);
  table[41] = reinterpret_cast<void *>(&provider_three_ptrs);
  provider.vptr = table;
}
'''
    main = r'''
int main() {
  setup_provider();
  Wave_Device w = {};
  assert(w.get_ndevices() == 0 && w.get_rate() == 0 && w.get_ds() == 0);
  assert(w.get_hw_mem_size() == 0 && w.is_disabled() == 1 && w.stop_raw_dump() == 3);
  assert(!w.is_eax() && !w.is_3d() && w.set_hwnd(reinterpret_cast<void *>(1)) == 0x13);
  assert(w.start_raw_dump(0) == 3 && w.set_eax((EAX_REVERB_PROPERTIES *)0) == 0x14);
  assert(w.set_eax(1UL) == 0x14 && w.set_eax_mix(1.0f) == 0x14);
  assert(w.get_description(1, 0, 2) == 0 && calls[6] == 0);
  w.device_14_ = &provider;
  value_result = -17;
  assert(w.get_ndevices() == -17 && w.get_rate() == -17);
  assert(w.get_ds() == &direct_sound);
  assert(w.get_hw_mem_size() == -17 && w.is_disabled() == -17 && w.stop_raw_dump() == -17);
  assert(w.is_eax() && w.is_3d());
  const unsigned long raw = 0xfedcba98UL;
  w.set_rate(raw); assert(word_arg == static_cast<uint32_t>(raw));
  w.set_volume(0x87654321UL); assert(word_arg == 0x87654321U);
  assert(w.set_hwnd(reinterpret_cast<void *>(0x3456)) == -17);
  assert(hwnd_arg == reinterpret_cast<void *>(0x3456));
  char path[] = "sound.raw";
  assert(w.get_description(19, path, 23) == 0);
  assert(word_arg == 19 && path_arg == path && float_args[0] == 23.0f);
  assert(w.start_raw_dump(path) == -17 && path_arg == path);
  EAX_REVERB_PROPERTIES *props = reinterpret_cast<EAX_REVERB_PROPERTIES *>(0x7788);
  assert(w.set_eax(props) == -17 && properties_arg == props);
  assert(w.set_eax(0x89abcdefUL) == -17 && word_arg == 0x89abcdefUL);
  assert(w.set_eax_mix(-0.375f) == -17 && float_args[0] == -0.375f);
  float x = 1.25f, y = -2.5f, z = 3.75f;
  assert(w.set_listener_position(x, y, z) == -17);
  assert(float_args[0] == x && float_args[1] == y && float_args[2] == z);
  assert(w.get_listener_position(&x, &y, &z) == -17);
  assert(float_ptrs[0] == &x && float_ptrs[1] == &y && float_ptrs[2] == &z);
  assert(w.set_listener_xpos(x) == -17 && float_args[0] == x);
  assert(w.get_listener_xpos(&x) == -17 && float_ptrs[0] == &x);
  assert(w.set_listener_ypos(y) == -17 && float_args[0] == y);
  assert(w.get_listener_ypos(&y) == -17 && float_ptrs[0] == &y);
  assert(w.set_listener_zpos(z) == -17 && float_args[0] == z);
  assert(w.get_listener_zpos(&z) == -17 && float_ptrs[0] == &z);

  // Factory/init/error and release/destroy sequencing use the actual owner
  // bodies; test callbacks stand in for the external DLL only.
  SoundDllProcs[4] = reinterpret_cast<void *>(&factory);
  SoundDllProcs[5] = reinterpret_cast<void *>(&destroy);
  Wave_Device lifecycle = {};
  factory_provider = &provider; factory_result = 0; init_result = 0;
  call_order = 0;
  assert(lifecycle.init(reinterpret_cast<void *>(0x1234), 0x12345678UL) == 0);
  assert(factory_calls == 1 && word_arg == 0x12345678U && calls[12] == 1);
  assert(call_order == 1 && lifecycle.device_14_ == &provider);
  lifecycle.device_14_ = 0;
  init_result = -29;
  assert(lifecycle.init(reinterpret_cast<void *>(0x1234), 7) == -29);
  assert(destroy_calls == 1 && lifecycle.device_14_ == 0);
  lifecycle.device_14_ = &provider; owner_to_clear = &lifecycle.device_14_;
  call_order = 0;
  assert(lifecycle.release() == 0 && call_order == 2);
  assert(destroy_calls == 1 && lifecycle.device_14_ == 0);
  owner_to_clear = 0; lifecycle.device_14_ = &provider; call_order = 0;
  assert(lifecycle.release() == 0 && call_order == 2);
  assert(destroy_calls == 2 && lifecycle.device_14_ == 0);
}
'''
    cpp = harness + slot_setup + selected + main
    path = tmp_path / "wave_provider_actual.cpp"
    (tmp_path / "soundx_wave_device_abi.h").write_text(header)
    path.write_text(cpp)
    binary = tmp_path / "wave_provider_actual"
    result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                             "-Wno-unused-parameter",
                             str(path), "-o", str(binary)], capture_output=True,
                            text=True)
    assert result.returncode == 0, result.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stderr
