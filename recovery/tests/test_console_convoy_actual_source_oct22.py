"""Host controls for the actual Console::convoy body.

The fixture extracts the currently mapped product definition on every run and
substitutes only external game/network services. It is not proof of VC6 ABI,
original runtime behavior beyond the pinned witnesses, or whole-game recovery.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ADDRESS = 0x004D8FA0
OWNER = "src/console_commands.cpp"
SIGNATURE = "void Console::convoy(int veh_id)"
WRITE = "NetDaemonNet.channels_[0].leading_words_[3] = 3;"


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / OWNER).is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX worktree")


def _actual_body() -> str:
    root = _root()
    source = (root / OWNER).read_text(encoding="utf-8")
    rows = [row for row in read_file(root / OWNER) if row.address == ADDRESS]
    assert len(rows) == 1
    row = rows[0]
    assert row.name == "?convoy@Console@@QAEXH@Z"
    assert Path(row.path).resolve() == (root / OWNER).resolve()
    assert source.count("// ORIGINAL: 0x004D8FA0") == 1
    assert row.region.count("// RECOVERY_BINDING: v1 ") == 1
    body = extract_definition(source, SIGNATURE)
    assert body in row.region
    assert body.count(WRITE) == 1
    assert body.index(WRITE) < body.index("supply_options(veh_id, base_id);")
    return body


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#define __cdecl
#define LPSTR char *

static std::vector<std::string> events;
static void mark(const char *name) { events.push_back(name); }
static void expect(const std::vector<std::string> &wanted) { assert(events == wanted); }

struct LockEntry { uint32_t leading_words_[4]; };
struct NetDaemonDouble {
    LockEntry channels_[2];
    int lock_result;
    int locked_id;
    int unlock_count;
    int lock_veh(int *id, int mode, int x, int y, int flags) {
        mark("lock");
        assert(mode == 0 && x == -1 && y == -1 && flags == 0);
        if (lock_result == 0) *id = locked_id;
        return lock_result;
    }
    void unlock_veh() { mark("unlock"); ++unlock_count; }
    void await_synch() { mark("await"); }
};
static NetDaemonDouble NetDaemonNet;

struct Veh { int x, y, home_base_id, order; uint8_t order_auto_type; };
static Veh Vehs[8];
struct Base { const char *name_string; };
static Base Bases[8];
struct ResourceName { char *name_singular; char *name_plural; };
static char singular[3][8] = {{'s','0',0},{'s','1',0},{'s','2',0}};
static char plural[3][8] = {{'p','0',0},{'p','1',0},{'p','2',0}};
static ResourceName Resource[3];

class Console {
public:
    int faction_id_;
    void convoy(int veh_id);
};

static int configured_base = -1;
static int convoy_result = 1;
static int popup_choice = -1;
static int supply_mutate_word3 = 0;
static int supply_seen_word3 = -999;
static int supply_seen_vehicle = -999;
static int supply_seen_base = -999;
static int resource_yield_calls;
static int parse_num_calls;
static int parse_say_calls;
static int parse_says_calls;
static int synch_calls;

static int base_at(int x, int y) {
    mark("base_at"); assert(x == 31 && y == 47); return configured_base;
}
static void supply_options(int veh_id, int base_id) {
    mark("supply");
    supply_seen_word3 = static_cast<int>(NetDaemonNet.channels_[0].leading_words_[3]);
    supply_seen_vehicle = veh_id; supply_seen_base = base_id;
    if (supply_mutate_word3)
        NetDaemonNet.channels_[0].leading_words_[3] = 0xA11CE;
}
static int can_convoy(int x, int y) {
    mark("can_convoy"); assert(x == 31 && y == 47); return convoy_result;
}
struct NetMessage { void pop(const char *s, int, int, int) {
    mark("pop"); assert(std::string(s) == "CANTCONVOY");
} };
static NetMessage NetMsgPrimary;
static int resource_yield(int resource, int faction, int home, int x, int y) {
    mark("yield");
    assert(resource == resource_yield_calls && resource < 3);
    assert(faction == 6 && home == 4 && x == 31 && y == 47);
    ++resource_yield_calls; return resource == 1 ? 1 : resource + 2;
}
static void parse_num(int resource, int amount) {
    mark("parse_num"); assert(resource == parse_num_calls);
    assert(amount == (resource == 1 ? 1 : resource + 2)); ++parse_num_calls;
}
static void parse_say(int resource, char *name, int a, int b) {
    mark("parse_say"); assert(resource == parse_say_calls && a == -1 && b == -1);
    assert(name == (resource == 1 ? singular[resource] : plural[resource]));
    ++parse_say_calls;
}
static void parse_says(int slot, const char *name, int a, int b) {
    mark("parse_says"); assert(slot == 3 && std::string(name) == "base-four");
    assert(a == -1 && b == -1); ++parse_says_calls;
}
static int x_pop_short(const char *label, int flags, int extra) {
    mark("popup"); assert(std::string(label) == "SUPPLYEXTRACT");
    assert(flags == 0x40 && extra == 0); return popup_choice;
}
static void synch_veh(int id) {
    mark("synch");
    assert(id == 2 && Vehs[id].order == 3 && Vehs[id].order_auto_type == popup_choice);
    ++synch_calls;
}

/* ACTUAL_CONVOY_BODY */

static void reset() {
    events.clear();
    NetDaemonNet = NetDaemonDouble{};
    NetDaemonNet.lock_result = 0; NetDaemonNet.locked_id = 2;
    NetDaemonNet.unlock_count = 0;
    for (int channel = 0; channel < 2; ++channel)
        for (int word = 0; word < 4; ++word)
            NetDaemonNet.channels_[channel].leading_words_[word] =
                0x510000u + static_cast<uint32_t>(channel * 0x100 + word * 7);
    configured_base = -1; convoy_result = 1; popup_choice = -1;
    supply_mutate_word3 = 0; supply_seen_word3 = -999;
    supply_seen_vehicle = supply_seen_base = -999;
    resource_yield_calls = parse_num_calls = parse_say_calls = parse_says_calls = 0;
    synch_calls = 0;
    Vehs[2] = Veh{31, 47, 4, 22, 9}; Vehs[3] = Veh{5, 6, 7, 8, 9};
    Bases[4].name_string = "base-four";
    for (int i = 0; i < 3; ++i) Resource[i] = ResourceName{singular[i], plural[i]};
}

static void check_only_convoy_word(int expected) {
    for (int channel = 0; channel < 2; ++channel) {
        for (int word = 0; word < 4; ++word) {
            const uint32_t initial = 0x510000u +
                static_cast<uint32_t>(channel * 0x100 + word * 7);
            const uint32_t wanted = channel == 0 && word == 3
                ? static_cast<uint32_t>(expected) : initial;
            assert(NetDaemonNet.channels_[channel].leading_words_[word] == wanted);
        }
    }
}

static void lock_rejection() {
    reset(); Console c = {6}; NetDaemonNet.lock_result = 1;
    const int before = NetDaemonNet.channels_[0].leading_words_[3];
    c.convoy(1);
    expect({"lock"}); assert(NetDaemonNet.unlock_count == 0);
    assert(NetDaemonNet.channels_[0].leading_words_[3] == static_cast<uint32_t>(before));
}
static void base_supply_callback_order() {
    reset(); Console c = {6}; configured_base = 5; supply_mutate_word3 = 1;
    c.convoy(1);
    expect({"lock", "base_at", "supply", "unlock"});
    assert(supply_seen_word3 == 3 && supply_seen_vehicle == 2 && supply_seen_base == 5);
    assert(NetDaemonNet.unlock_count == 1);
    check_only_convoy_word(0xA11CE); // callback mutation remains; no later duplicate store
    assert(Vehs[2].order == 22 && Vehs[2].order_auto_type == 9);
}
static void no_home_has_no_action_write() {
    reset(); Console c = {6}; Vehs[2].home_base_id = -1;
    c.convoy(1);
    expect({"lock", "base_at", "unlock"});
    check_only_convoy_word(static_cast<int>(0x510000u + 3u * 7u));
    assert(NetDaemonNet.unlock_count == 1);
}
static void cannot_convoy_has_no_action_write() {
    reset(); Console c = {6}; convoy_result = 0;
    c.convoy(1);
    expect({"lock", "base_at", "can_convoy", "pop", "unlock"});
    check_only_convoy_word(static_cast<int>(0x510000u + 3u * 7u));
    assert(resource_yield_calls == 0 && parse_num_calls == 0 &&
           parse_say_calls == 0 && parse_says_calls == 0 && synch_calls == 0);
    assert(NetDaemonNet.unlock_count == 1);
}
static void extraction_cancel_preserves_order_and_unlocks() {
    reset(); Console c = {6}; popup_choice = -1;
    c.convoy(1);
    expect({"lock", "base_at", "can_convoy", "yield", "parse_num", "parse_say",
            "yield", "parse_num", "parse_say", "yield", "parse_num", "parse_say",
            "parse_says", "popup", "unlock"});
    assert(resource_yield_calls == 3 && parse_num_calls == 3 && parse_say_calls == 3 &&
           parse_says_calls == 1 && synch_calls == 0);
    check_only_convoy_word(static_cast<int>(0x510000u + 3u * 7u));
    assert(Vehs[2].order == 22 && Vehs[2].order_auto_type == 9);
    assert(NetDaemonNet.unlock_count == 1);
}
static void extraction_accept_sets_order_then_synchs() {
    reset(); Console c = {6}; popup_choice = 2;
    c.convoy(1);
    assert(events.size() == 17);
    assert(events[12] == "parse_says" && events[13] == "popup" && events[14] == "synch" &&
           events[15] == "await" && events[16] == "unlock");
    assert(Vehs[2].order == 3 && Vehs[2].order_auto_type == 2);
    assert(resource_yield_calls == 3 && parse_num_calls == 3 && parse_say_calls == 3 &&
           parse_says_calls == 1 && synch_calls == 1);
    check_only_convoy_word(static_cast<int>(0x510000u + 3u * 7u));
    assert(NetDaemonNet.unlock_count == 1);
}
int main() {
    lock_rejection(); base_supply_callback_order(); no_home_has_no_action_write();
    cannot_convoy_has_no_action_write(); extraction_cancel_preserves_order_and_unlocks();
    extraction_accept_sets_order_then_synchs();
}
'''


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler")


def _compile_and_run(body: str, compiler: str, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = HARNESS.replace("/* ACTUAL_CONVOY_BODY */", body)
    cpp = tmp_path / "convoy.cpp"
    exe = tmp_path / "convoy"
    cpp.write_text(source, encoding="utf-8")
    # C++11 is used for concise fixture-only vector/aggregate setup; the
    # extracted production body itself uses only the project's older C++ forms.
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True, capture_output=True,
    )
    assert built.returncode == 0, f"host fixture must compile before runtime assertion: {built.stderr}"
    assert exe.is_file(), "successful host compile did not produce executable"
    return subprocess.run([str(exe)], text=True, capture_output=True)


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_convoy_body_and_semantic_controls(compiler: str, tmp_path: Path) -> None:
    body = _actual_body()
    baseline = _compile_and_run(body, compiler, tmp_path / "baseline")
    assert baseline.returncode == 0, baseline.stderr

    damage = {
        "missing-store": body.replace(WRITE, "", 1),
        "wrong-value": body.replace(WRITE, WRITE.replace("= 3", "= 4"), 1),
        "wrong-channel": body.replace(
            "channels_[0].leading_words_[3]", "channels_[1].leading_words_[3]", 1),
        "after-supply-callback": body.replace(
            f"        {WRITE}\n        supply_options(veh_id, base_id);",
            f"        supply_options(veh_id, base_id);\n        {WRITE}", 1),
        "unconditional-before-base-branch": body.replace(
            "    if (base_id >= 0) {",
            f"    {WRITE}\n    if (base_id >= 0) {{", 1),
        "stale-pre-lock-receiver": body.replace("Veh &vehicle = Vehs[veh_id];",
                                                   "Veh &vehicle = Vehs[1];", 1),
        "missing-base-unlock": body.replace(
            "        NetDaemonNet.unlock_veh();\n        return;",
            "        return;", 1),
    }
    for name, mutant in damage.items():
        assert mutant != body, name
        result = _compile_and_run(mutant, compiler, tmp_path / name)
        assert result.returncode != 0, f"{name} mutant survived under {compiler}"
