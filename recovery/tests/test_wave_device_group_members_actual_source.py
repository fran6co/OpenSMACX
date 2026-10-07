from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
CANDIDATE = ROOT


def body(source: str, signature: str) -> str:
    begin = source.index(signature)
    opening = source.index("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1 : index]
    raise AssertionError(signature)


def source_bodies() -> dict[str, str]:
    device = (CANDIDATE / "src/wave_device.cpp").read_text()
    wave = (CANDIDATE / "src/wave.cpp").read_text()
    result = {
        "get": body(device, "int Wave_Device::get_group_volume("),
        "add": body(device, "int Wave_Device::add_to_group("),
        "pull": body(device, "int Wave_Device::pull_from_group("),
        "set": body(wave, "void Wave::set_volume("),
    }
    assert "groups_[group].volume" in result["get"]
    assert "wave->group_slot_ = group;" in result["add"]
    assert "a1->group_slot_ = 0x10;" in result["pull"]
    assert "WaveDeviceGlobal.groups_[group_slot_].volume" in result["set"]
    assert "reinterpret_cast<uint8_t *>" not in result["add"] + result["pull"]
    assert "WaveDeviceGroupVolumes" not in result["set"]
    return result


def harness(bodies: dict[str, str]) -> str:
    return f"""
#include <cstdint>
#include <cstring>
#include <new>

struct Wave;
struct WaveGroupNode {{ WaveGroupNode *prev; WaveGroupNode *next; Wave *wave; }};
struct WaveGroupList {{
  WaveGroupNode *head; WaveGroupNode *tail; WaveGroupNode *cursor; uint32_t count;
  void insert(Wave *);
}};
struct WaveControlGroup {{
  uint8_t enabled; uint8_t pad[3]; uint32_t volume;
  WaveGroupNode *head; WaveGroupNode *tail; WaveGroupNode *cursor; uint32_t count;
}};
struct OriginalObject {{
  int observed;
  void set(int value) {{ observed = value; }}
}};
template<class Method> Method vtable_slot(void *, int) {{ return &OriginalObject::set; }}
#define ORIGINAL(pointer) static_cast<OriginalObject *>(pointer)

static bool fail_insert;
static uint32_t slot_seen_by_insert;
class Wave_Device;
class Wave_Device {{
  friend class Wave;
public:
  WaveControlGroup groups_[16];
  int get_group_volume(unsigned int);
  int add_to_group(unsigned int, Wave *);
  int pull_from_group(Wave *);
}};
extern Wave_Device WaveDeviceGlobal;
class Wave {{
  friend class Wave_Device;
  uint32_t group_slot_;
  uint32_t volume_;
  OriginalObject *device_;
public:
  void set_volume(int volume);
  uint32_t slot() const {{ return group_slot_; }}
  void slot(uint32_t value) {{ group_slot_ = value; }}
  void device(OriginalObject *value) {{ device_ = value; }}
}};
void WaveGroupList::insert(Wave *wave) {{
  slot_seen_by_insert = wave->slot();
  if (fail_insert) throw std::bad_alloc();
  WaveGroupNode *node = new WaveGroupNode{{tail, nullptr, wave}};
  if (tail) tail->next = node; else head = node;
  tail = node;
  ++count;
}}
void Wave::set_volume(int volume) {{{bodies['set']}}}
int Wave_Device::get_group_volume(unsigned int group) {{{bodies['get']}}}
int Wave_Device::add_to_group(unsigned int group, Wave *wave) {{{bodies['add']}}}
int Wave_Device::pull_from_group(Wave *a1) {{{bodies['pull']}}}
Wave_Device WaveDeviceGlobal;

static int test_add_order_and_errors() {{
  Wave_Device d{{}}; Wave w{{}}; w.slot(9);
  if (d.add_to_group(16, &w) != 0xA || d.add_to_group(0, nullptr) != 0xA) return 1;
  if (d.add_to_group(2, &w) != 0 || slot_seen_by_insert != 9 || w.slot() != 2) return 2;
  Wave failing{{}}; failing.slot(7); fail_insert = true;
  try {{ (void)d.add_to_group(3, &failing); return 3; }} catch (const std::bad_alloc &) {{}}
  if (slot_seen_by_insert != 7 || failing.slot() != 7) return 4;
  return 0;
}}
static int test_pull_order_and_list_edges() {{
  Wave_Device d{{}};
  if (d.pull_from_group(nullptr) != 0xA) return 1;
  Wave invalid{{}}; invalid.slot(16); if (d.pull_from_group(&invalid) != 0xA) return 2;
  Wave a{{}}, b{{}}, c{{}}, absent{{}}; a.slot(0); b.slot(0); c.slot(0); absent.slot(0);
  WaveGroupNode *n1 = new WaveGroupNode{{nullptr, nullptr, &a}};
  WaveGroupNode *n2 = new WaveGroupNode{{n1, nullptr, &b}};
  WaveGroupNode *n3 = new WaveGroupNode{{n2, nullptr, &c}};
  n1->next = n2; n2->next = n3;
  auto &g = d.groups_[0]; g.head = n1; g.tail = n3; g.cursor = n2; g.count = 3;
  if (d.pull_from_group(&b) != 0 || b.slot() != 16) return 3;
  if (g.head != n1 || n1->next != n3 || n3->prev != n1 || g.cursor != n3 || g.count != 2) return 4;
  if (d.pull_from_group(&absent) != 0 || absent.slot() != 16 || g.count != 2) return 5;
  if (d.pull_from_group(&c) != 0 || c.slot() != 16) return 6;
  if (g.tail != n1 || g.cursor != nullptr || g.count != 1 || n1->next != nullptr) return 7;
  delete n1;
  return 0;
}}
static int test_unsigned_group_volume_and_bit_return() {{
  Wave_Device &d = WaveDeviceGlobal; d.groups_[0].volume = 0xC0000000U;
  const int returned = d.get_group_volume(0);
  uint32_t returned_bits = 0; std::memcpy(&returned_bits, &returned, sizeof(returned));
  if (returned_bits != 0xC0000000U || d.get_group_volume(16) != 0) return 1;
  Wave w{{}}; OriginalObject device{{0}}; w.slot(0); w.device(&device);
  w.set_volume(63);
  const int expected = static_cast<int>(static_cast<int64_t>(
      (63.0 * (1.0 / 127.0)) * static_cast<double>(0xC0000000U)));
  if (device.observed != expected || device.observed <= 0) return 2;
  w.slot(16); w.set_volume(63);
  if (device.observed != 63) return 3;
  return 0;
}}
int main() {{
  int rc = test_add_order_and_errors(); if (rc) return 10 + rc;
  rc = test_pull_order_and_list_edges(); if (rc) return 30 + rc;
  rc = test_unsigned_group_volume_and_bit_return(); if (rc) return 50 + rc;
  return 0;
}}
"""


def test_actual_bodies_preserve_group_semantics() -> None:
    bodies = source_bodies()
    wave_header = (CANDIDATE / "src/wave.h").read_text()
    device_header = (CANDIDATE / "src/wave_device.h").read_text()
    assert "friend class Wave_Device;" in wave_header
    assert "friend class Wave;" in device_header
    assert "uint32_t group_slot_;" in wave_header
    assert "WaveControlGroup groups_[16];" in device_header
    assert "WaveDevicePullFromGroup" not in wave_header
    assert "WaveDeviceGroupVolumes" not in wave_header
    for marker in (
        "// ORIGINAL: 0x004C5380 ?get_group_volume@Wave_Device@@QAEHI@Z",
        "// ORIGINAL: 0x004C5240 ?add_to_group@Wave_Device@@QAEHIPAUWave@@@Z",
        "// ORIGINAL: 0x004C5280 ?pull_from_group@Wave_Device@@QAEHPAUWave@@@Z",
        "// ORIGINAL: 0x004C7130 ?set_volume@Wave@@QAEXH@Z",
    ):
        assert marker in (CANDIDATE / "src/wave_device.cpp").read_text() + (CANDIDATE / "src/wave.cpp").read_text()
    compilers = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]
    assert compilers
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="osmx-wave-group-owner-") as tmp:
            binary = Path(tmp) / "controls"
            built = subprocess.run(
                [compiler, "-std=c++11", "-O0", "-x", "c++", "-", "-o", str(binary)],
                input=harness(bodies), text=True, capture_output=True, timeout=45,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], check=False, timeout=10)
            assert ran.returncode == 0, (compiler, ran.returncode)
