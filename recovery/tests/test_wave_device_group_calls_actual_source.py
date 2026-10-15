from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def repository_root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src").is_dir() and (candidate / "recovery").is_dir():
            return candidate
    raise AssertionError("could not find repository root")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1 : i]
    raise AssertionError(f"unterminated body: {signature}")


def extracted_bodies(root: Path) -> dict[str, str]:
    source = (root / "src/wave_device.cpp").read_text()
    signatures = {
        "set_group_volume": "int Wave_Device::set_group_volume(",
        "enable_group": "int Wave_Device::enable_group(",
        "disable_group": "int Wave_Device::disable_group(",
    }
    result = {name: body(source, signature) for name, signature in signatures.items()}
    assert "wave->set_volume(volume);" in result["set_group_volume"]
    assert "wave->load();" in result["enable_group"]
    assert "wave->unload();" in result["disable_group"]
    for value in result.values():
        assert "OriginalObject" not in value
        assert "vtable_slot" not in value
    return result


def harness(bodies: dict[str, str]) -> str:
    return f"""
#include <cstdint>
#include <cstring>

struct Wave;
struct WaveGroupNode {{
    WaveGroupNode *prev;
    WaveGroupNode *next;
    Wave *wave;
}};
struct WaveGroupList {{
    WaveGroupNode *head;
    WaveGroupNode *tail;
    WaveGroupNode *cursor;
    uint32_t count;
}};
struct Hook {{
    WaveGroupList *list;
    WaveGroupNode *replacement_cursor;
    bool mutate_cursor;
    uint8_t *enabled;
    int enabled_seen;
    uint32_t *group_volume;
    uint32_t group_volume_seen;
}};
struct Event {{ int op; int id; int value; }};
static Event events[32];
static int event_count;
static void record(int op, int id, int value) {{
    events[event_count++] = Event{{op, id, value}};
}}

struct Wave {{
    friend class Wave_Device;
    int id;
    uint32_t volume_;
    Hook *hook;
    virtual ~Wave() {{}}
    void after() {{
        if (hook == nullptr) return;
        if (hook->enabled != nullptr) hook->enabled_seen = *hook->enabled;
        if (hook->group_volume != nullptr) hook->group_volume_seen = *hook->group_volume;
        if (hook->mutate_cursor && hook->list != nullptr)
            hook->list->cursor = hook->replacement_cursor;
    }}
    virtual void set_volume(int value) {{ record(1, id, value); after(); }}
    virtual int load() {{ record(2, id, 0); after(); return 91; }}
    virtual int unload() {{ record(3, id, 0); after(); return 92; }}
}};
struct DerivedWave : Wave {{
    void set_volume(int value) override {{ record(11, id, value); after(); }}
    int load() override {{ record(12, id, 0); after(); return -7; }}
    int unload() override {{ record(13, id, 0); after(); return -8; }}
}};
struct WaveControlGroup {{
    uint8_t enabled;
    uint8_t pad[3];
    uint32_t volume;
    WaveGroupList list_;
}};
class Wave_Device {{
public:
    WaveControlGroup groups_[16];
    int set_group_volume(unsigned int a1, unsigned int a2) {{{bodies['set_group_volume']}}}
    int enable_group(unsigned int a1) {{{bodies['enable_group']}}}
    int disable_group(unsigned int a1) {{{bodies['disable_group']}}}
}};

static void reset_events() {{ event_count = 0; }}
static bool event(int index, int op, int id, int value) {{
    return index < event_count && events[index].op == op &&
           events[index].id == id && events[index].value == value;
}}
static int test_volume_bounds_and_live_cursor() {{
    Wave_Device d{{}};
    WaveGroupNode n1{{}}, n2{{}}, n3{{}};
    DerivedWave w1{{}};
    Wave w2{{}};
    Wave w3{{}};
    Hook h1{{}}, h3{{}};
    w1.id = 1; w1.volume_ = 0x80000003U; w1.hook = &h1;
    w2.id = 2; w2.volume_ = 8; w2.hook = nullptr;
    w3.id = 3; w3.volume_ = 9; w3.hook = &h3;
    n1.wave = &w1; n1.next = &n2; n2.wave = &w2; n2.next = &n3;
    n3.wave = &w3; n3.next = nullptr;
    d.groups_[0].volume = 99;
    WaveGroupList *list = &d.groups_[0].list_;
    list->head = &n1; list->tail = &n3; list->count = 3;
    h1.list = list; h1.mutate_cursor = true; h1.replacement_cursor = &n2;
    h1.group_volume = &d.groups_[0].volume;
    h3.group_volume = &d.groups_[0].volume;
    h3.list = list;
    reset_events();
    if (d.set_group_volume(16, 7) != 0xA || d.set_group_volume(0, 0x80) != 0xA) return 1;
    if (d.groups_[0].volume != 99 || event_count != 0) return 2;
    if (d.set_group_volume(0, 12) != 0 || d.groups_[0].volume != 12) return 3;
    if (event_count != 2) return 41;
    if (!event(0, 11, 1, (int)0x80000003U)) return 42;
    if (!event(1, 1, 3, 9)) return 43;
    if (h1.group_volume_seen != 12 || h3.group_volume_seen != 12) return 5;
    if (list->cursor != nullptr) return 6;
    return 0;
}}
static int test_enable_walk_and_post_store() {{
    Wave_Device d{{}};
    WaveGroupNode n1{{}}, n2{{}}, n3{{}};
    DerivedWave w1{{}};
    Wave w2{{}};
    DerivedWave w3{{}};
    Hook h1{{}}, h3{{}};
    w1.id = 4; w1.hook = &h1;
    w2.id = 5; w2.hook = nullptr;
    w3.id = 6; w3.hook = &h3;
    n1.wave = &w1; n1.next = &n2; n2.wave = &w2; n2.next = &n3;
    n3.wave = &w3;
    d.groups_[0].enabled = 0;
    WaveGroupList *list = &d.groups_[0].list_;
    list->head = &n1; list->tail = &n3; list->count = 3;
    h1.enabled = &d.groups_[0].enabled; h1.enabled_seen = -1;
    h1.list = list; h1.mutate_cursor = true; h1.replacement_cursor = &n2;
    h3.enabled = &d.groups_[0].enabled; h3.enabled_seen = -1; h3.list = list;
    reset_events();
    if (d.enable_group(16) != 0xA || d.enable_group(0) != 0) return 1;
    if (event_count != 2 || !event(0, 12, 4, 0) || !event(1, 12, 6, 0)) return 2;
    if (h1.enabled_seen != 0 || h3.enabled_seen != 0 || d.groups_[0].enabled != 1) return 3;
    if (list->cursor != nullptr) return 4;
    if (d.enable_group(0) != 0 || event_count != 2 || d.groups_[0].enabled != 1) return 5;
    d.groups_[1].enabled = 7;
    if (d.enable_group(1) != 0 || d.groups_[1].enabled != 7 || event_count != 2) return 6;
    return 0;
}}
static int test_disable_walk_and_post_store() {{
    Wave_Device d{{}};
    WaveGroupNode n1{{}}, n2{{}};
    DerivedWave w1{{}};
    Wave w2{{}};
    Hook h1{{}};
    w1.id = 7; w1.hook = &h1;
    w2.id = 8; w2.hook = nullptr;
    n1.wave = &w1; n1.next = &n2; n2.wave = &w2;
    d.groups_[0].enabled = 1;
    WaveGroupList *list = &d.groups_[0].list_;
    list->head = &n1; list->tail = &n2; list->count = 2;
    h1.enabled = &d.groups_[0].enabled; h1.enabled_seen = -1;
    h1.list = list;
    reset_events();
    if (d.disable_group(16) != 0xA || d.disable_group(0) != 0) return 1;
    if (event_count != 2 || !event(0, 13, 7, 0) || !event(1, 3, 8, 0)) return 2;
    if (h1.enabled_seen != 1 || d.groups_[0].enabled != 0) return 3;
    if (d.disable_group(0) != 0 || event_count != 2) return 4;
    return 0;
}}
int main() {{
    int result = test_volume_bounds_and_live_cursor(); if (result) return 10 + result;
    result = test_enable_walk_and_post_store(); if (result) return 30 + result;
    result = test_disable_walk_and_post_store(); if (result) return 50 + result;
    return 0;
}}
"""


def compile_and_run(compiler: str, source: str) -> int:
    with tempfile.TemporaryDirectory(prefix="osmx-wave-api-") as temporary:
        executable = Path(temporary) / "controls"
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-x", "c++", "-", "-o", str(executable)],
            input=source,
            text=True,
            capture_output=True,
            timeout=45,
        )
        assert compiled.returncode == 0, compiled.stderr
        return subprocess.run([str(executable)], check=False, timeout=10).returncode


def host_compilers() -> list[str]:
    return [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


def test_group_walks_execute_actual_wave_apis() -> None:
    root = repository_root()
    bodies = extracted_bodies(root)
    wave_header = (root / "src/wave.h").read_text()
    sound_header = (root / "src/sound.h").read_text()
    device_header = (root / "src/wave_device.h").read_text()
    assert "class Wave : public Sound" in wave_header
    assert "void set_volume(int volume);" in wave_header
    assert "virtual int load();" in wave_header
    assert "int unload();" in wave_header
    assert "virtual int unload();" in sound_header
    assert "friend class Wave_Device;" in sound_header
    assert "uint32_t volume_;       // 0x04" in sound_header
    assert "Wave *wave;" in device_header
    assert "class Effect : public Wave" in wave_header
    source = harness(bodies)
    compilers = host_compilers()
    assert compilers, "g++ or clang++ host compiler required"
    for compiler in compilers:
        assert compile_and_run(compiler, source) == 0


@pytest.mark.parametrize(
    ("function", "old", "new"),
    [
        ("set_group_volume", "wave->set_volume(volume);", "wave->set_volume(volume + 1);"),
        ("enable_group", "wave->load();", "wave->unload();"),
        ("disable_group", "wave->unload();", "wave->load();"),
    ],
)
def test_wrong_wave_operation_mutants_fail_behavior_control(function: str, old: str, new: str) -> None:
    root = repository_root()
    bodies = extracted_bodies(root)
    assert bodies[function].count(old) == 1
    bodies[function] = bodies[function].replace(old, new)
    source = harness(bodies)
    compilers = host_compilers()
    assert compilers, "g++ or clang++ host compiler required"
    for compiler in compilers:
        assert compile_and_run(compiler, source) != 0
