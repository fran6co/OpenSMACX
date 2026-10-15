"""Typed dispatch for Wave's newly-created provider; host tests do not prove ABI."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_spritelist_base_lifetime import extract_definition


ROOT = Path(__file__).resolve().parents[2]


HARNESS = r'''
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

static std::vector<std::string> events;
static char *resolved_path = 0;
static int create_result = 0;
static void *created_device = 0;
static char *created_name = 0;
static int created_mode = -1;
static bool WaveDeviceReleaseGuard = true;
static const unsigned long computed_attribs = 0xA55AUL;

struct WavePlaybackDeviceInterface {
  std::vector<unsigned long> attribs;
  void set_attrib(unsigned long value) {
    events.push_back("set");
    attribs.push_back(value);
  }
};

static WavePlaybackDeviceInterface *wave_playback_device(void *device) {
  return static_cast<WavePlaybackDeviceInterface *>(device);
}

typedef int (*CreateSound)(void **, char *, int);
static int create_sound(void **out, char *name, int mode) {
  events.push_back("create");
  created_name = name;
  created_mode = mode;
  *out = created_device;
  return create_result;
}
static CreateSound SoundXOrdinal1CreateSound() { return &create_sound; }

static char *filefind_get(char *) {
  events.push_back("resolve");
  return resolved_path;
}

struct Wave {
  void *device_;
  uint32_t flags_54_;
  char *fname_;
  int loop_state_;
  int get_calls_;
  int attach_calls_;

  Wave() : device_(0), flags_54_(0), fname_(0), loop_state_(0),
           get_calls_(0), attach_calls_(0) {}
  ~Wave() { operator delete(fname_); }
  int get_attrib() {
    events.push_back("get");
    ++get_calls_;
    return static_cast<int>(computed_attribs);
  }
  void attach() { events.push_back("attach"); ++attach_calls_; }
  void set_loop_state(int value) {
    events.push_back("loop");
    loop_state_ = value;
  }
  int dyna_load(char *fname);
  void init(char *group_id, unsigned long flags);
};
'''


def actual_bodies(damage):
    source = (ROOT / "src/wave.cpp").read_text()
    dyna = extract_definition(source, "int Wave::dyna_load(char *fname) {")
    init = extract_definition(source, "void Wave::init(char *group_id, unsigned long flags) {")
    if damage == "dyna-mask":
        old = "wave_playback_device(device_)->set_attrib(\n                static_cast<unsigned long>(attribs));"
        assert old in dyna
        dyna = dyna.replace(old, "wave_playback_device(device_)->set_attrib(0UL);", 1)
    elif damage == "init-raw-mask":
        old = "wave_playback_device(device_)->set_attrib(flags);"
        assert old in init
        init = init.replace(old, "wave_playback_device(device_)->set_attrib(0UL);", 1)
    return dyna, init


MAIN = r'''
#define REQUIRE(test, code) do { if (!(test)) return code; } while (0)

int main() {
  char path[] = "wave.wav";
  WavePlaybackDeviceInterface provider;
  Wave w;

  // Existing-device, dead-hook, and creation-error paths never dispatch or attach.
  w.device_ = &provider;
  events.clear();
  REQUIRE(w.dyna_load(path) == 0xC, 1);
  REQUIRE(events.empty() && w.get_calls_ == 0 && w.attach_calls_ == 0, 2);
  w.device_ = 0;
  WaveDeviceReleaseGuard = false;
  REQUIRE(w.dyna_load(path) == 1 && events.empty(), 3);
  WaveDeviceReleaseGuard = true;
  create_result = 7;
  created_device = 0;
  REQUIRE(w.dyna_load(path) == 7, 4);
  REQUIRE(events.size() == 1 && events[0] == "create" &&
          w.get_calls_ == 0 && w.attach_calls_ == 0, 5);

  // Successful dynamic load computes before the typed setter, then attaches.
  events.clear();
  create_result = 0;
  created_device = &provider;
  provider.attribs.clear();
  REQUIRE(w.dyna_load(path) == 0, 6);
  REQUIRE(created_name == path && created_mode == 1, 7);
  REQUIRE(events.size() == 4 && events[0] == "create" &&
          events[1] == "get" && events[2] == "set" && events[3] == "attach", 8);
  REQUIRE(provider.attribs.size() == 1 &&
          provider.attribs[0] == computed_attribs, 9);
  REQUIRE(w.get_calls_ == 1 && w.attach_calls_ == 1, 10);

  // Streaming refusal and unresolved paths preserve state and avoid creation.
  Wave init_wave;
  init_wave.flags_54_ = 0x400;
  events.clear();
  init_wave.init(path, 4 | 0x10);
  REQUIRE(events.empty() && init_wave.flags_54_ == 0x400, 11);
  init_wave.init(path, 4 | 0x80);
  REQUIRE(events.empty() && init_wave.flags_54_ == 0x400, 12);
  resolved_path = 0;
  init_wave.init(path, 4);
  REQUIRE(events.size() == 1 && events[0] == "resolve" &&
          init_wave.flags_54_ == 0x400, 13);

  // A dead creation hook leaves a streaming wave unattached but does not
  // suppress the original local flag and loop updates.
  resolved_path = path;
  WaveDeviceReleaseGuard = false;
  events.clear();
  init_wave.init(path, 4 | 1 | 2);
  REQUIRE(events.size() == 2 && events[0] == "resolve" &&
          events[1] == "loop", 25);
  REQUIRE(init_wave.flags_54_ == (2 | 1) && init_wave.loop_state_ == 1 &&
          init_wave.device_ == 0, 26);
  WaveDeviceReleaseGuard = true;

  // A failed streaming create skips computed attributes and attach, but the
  // original post-create flag updates and loop-state call still run.
  resolved_path = path;
  create_result = 5;
  created_device = 0;
  events.clear();
  init_wave.init(path, 4 | 1 | 2);
  REQUIRE(events.size() == 3 && events[0] == "resolve" &&
          events[1] == "create" && events[2] == "loop", 23);
  REQUIRE(init_wave.flags_54_ == (2 | 1) && init_wave.loop_state_ == 1 &&
          init_wave.get_calls_ == 0 && init_wave.attach_calls_ == 0, 24);

  // New streaming provider gets computed attributes before attach, then raw
  // mode flags; local flags and loop state retain the original updates.
  char resolved[] = "resolved.wav";
  resolved_path = resolved;
  created_device = &provider;
  create_result = 0;
  provider.attribs.clear();
  events.clear();
  const unsigned long mode = 4 | 1 | 2 | 0x40 | 0x100;
  init_wave.init(path, mode);
  REQUIRE(created_name == resolved && created_mode == 1, 14);
  REQUIRE(events.size() == 7 && events[0] == "resolve" &&
          events[1] == "create" && events[2] == "get" &&
          events[3] == "set" && events[4] == "attach" &&
          events[5] == "set" && events[6] == "loop", 15);
  REQUIRE(provider.attribs.size() == 2 &&
          provider.attribs[0] == computed_attribs &&
          provider.attribs[1] == mode, 16);
  REQUIRE(init_wave.flags_54_ == (2 | 1 | 8 | 0x20) &&
          init_wave.loop_state_ == 1 && init_wave.attach_calls_ == 1, 17);

  // Existing streaming devices receive only the raw mode mask. A non-streaming
  // init does not send attributes, but still applies flags and loop state.
  Wave existing;
  existing.device_ = &provider;
  provider.attribs.clear();
  events.clear();
  existing.init(path, 4 | 1 | 2);
  REQUIRE(provider.attribs.size() == 1 && provider.attribs[0] == (4 | 1 | 2), 18);
  REQUIRE(events.size() == 3 && events[0] == "resolve" &&
          events[1] == "set" && events[2] == "loop", 19);
  REQUIRE(existing.flags_54_ == (2 | 1) && existing.loop_state_ == 1, 20);
  provider.attribs.clear();
  events.clear();
  existing.init(path, 1 | 2 | 0x10 | 0x80 | 0x100);
  REQUIRE(provider.attribs.empty(), 21);
  REQUIRE(existing.flags_54_ == (1 | 4 | 0x10 | 0x20) &&
          existing.loop_state_ == 1, 22);
  return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "dyna-mask", "init-raw-mask"))
def test_actual_created_provider_bodies_and_failing_mutants(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    dyna, init = actual_bodies(damage)
    source = tmp_path / "created_provider.cpp"
    source.write_text(HARNESS + "\n" + dyna + "\n" + init + "\n" + MAIN)
    exe = tmp_path / "created_provider"
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(exe)], check=True)
    result = subprocess.run([str(exe)])
    assert result.returncode == (0 if damage is None else 9 if damage == "dyna-mask" else 16)
