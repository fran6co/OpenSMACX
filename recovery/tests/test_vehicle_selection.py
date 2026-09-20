"""Live pick_top_veh controls over bounded, well-formed vehicle stacks.

Pinned 559466 tests DH,42h (state mask 4200h, not the old seed's 420000h).
5594C6 preserves the first equal-scoring entry; 5594E3 returns the original
input if every entry is suppressed. These host tests extract the actual body
and record declarations. Host pointer widths in RulesChassis are not the
native layout proof; cycles, corrupt indices and whole-game rendering are out
of scope.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def without_comments(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source declaration/body")


def live_body():
    source = without_comments((ROOT / "src/veh.cpp").read_text())
    match, = re.finditer(r"int\s+__cdecl\s+pick_top_veh\(int veh_id\)\s*\{", source)
    return braced(source, match.start())


def declarations():
    header = without_comments((ROOT / "src/veh.h").read_text())
    records = [braced(header, header.index("struct " + name + " {")) + ";"
               for name in ("Veh", "VehPrototype", "RulesChassis")]
    console = without_comments((ROOT / "src/console.h").read_text())
    fields = []
    for name in ("faction_id_", "selected_vehicle_", "cursor_active_", "field_23BE4_"):
        match, = re.finditer(r"\b(?:u?int32_t|int)\s+" + name + r"\s*;", console)
        fields.append(match.group())
    return "\n".join(records) + "\nstruct Console {" + "\n".join(fields) + "} ConsoleGlobal;"


HARNESS = r"""
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
void check(bool ok) { if (!ok) std::exit(91); }
// DECLARATIONS
Veh Vehs[8];
VehPrototype VehPrototypes[8];
RulesChassis Chassis[8];
int VehDrawSuppressedVehicle = -1;
// BODY

void reset(const int *order, int count, int owners, int triads) {
    std::memset(Vehs,0,sizeof(Vehs));
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Chassis,0,sizeof(Chassis));
    std::memset(&ConsoleGlobal,0,sizeof(ConsoleGlobal));
    ConsoleGlobal.faction_id_ = 3;
    ConsoleGlobal.selected_vehicle_ = -1;
    VehDrawSuppressedVehicle = -1;
    for (int i=0; i<8; ++i)
        Vehs[i].prev_veh_id_stack = Vehs[i].next_veh_id_stack = -1;
    for (int pos=0; pos<count; ++pos) {
        const int id = order[pos], proto = 7-id, chassis = (2*id)%7;
        Vehs[id].prev_veh_id_stack = pos ? order[pos-1] : -1;
        Vehs[id].next_veh_id_stack = pos+1<count ? order[pos+1] : -1;
        Vehs[id].faction_id = (owners & (1<<pos)) ? 3 : 5;
        Vehs[id].proto_id = proto;
        VehPrototypes[proto].chassis_id = chassis;
        Chassis[chassis].triad = triads%3;
        triads /= 3;
    }
}

// The reference consumes the independently supplied ordered stack, never
// candidate next/previous traversal. Select priority first, otherwise the
// earliest maximum rank among unsuppressed nodes, falling back to input.
int reference(const int *order, int count, int input) {
    if (input < 0) return input;
    int ranks[3], highest = -1;
    for (int pos=0; pos<count; ++pos) {
        const int id = order[pos];
        ranks[pos] = -1;
        if (id == VehDrawSuppressedVehicle) continue;
        const Veh &v = Vehs[id];
        const bool own = v.faction_id == ConsoleGlobal.faction_id_;
        const bool state_allows = !(v.state & (uint32_t(0x42)<<8));
        if (id == ConsoleGlobal.selected_vehicle_ && own && state_allows &&
            ConsoleGlobal.cursor_active_ && ConsoleGlobal.field_23BE4_) return id;
        ranks[pos] = int(Chassis[VehPrototypes[v.proto_id].chassis_id].triad) + (own ? 4 : 0);
        highest = std::max(highest,ranks[pos]);
    }
    for (int pos=0; pos<count; ++pos)
        if (ranks[pos] >= 0 && ranks[pos] == highest) return order[pos];
    return input;
}

void verify(const int *order, int count, int input) {
    unsigned char vehicles[sizeof(Vehs)], prototypes[sizeof(VehPrototypes)];
    unsigned char chassis[sizeof(Chassis)], console[sizeof(ConsoleGlobal)];
    std::memcpy(vehicles,Vehs,sizeof(Vehs));
    std::memcpy(prototypes,VehPrototypes,sizeof(VehPrototypes));
    std::memcpy(chassis,Chassis,sizeof(Chassis));
    std::memcpy(console,&ConsoleGlobal,sizeof(ConsoleGlobal));
    const int suppressed = VehDrawSuppressedVehicle;
    const int expected = reference(order,count,input);
    check(pick_top_veh(input) == expected);
    check(std::memcmp(vehicles,Vehs,sizeof(Vehs)) == 0);
    check(std::memcmp(prototypes,VehPrototypes,sizeof(VehPrototypes)) == 0);
    check(std::memcmp(chassis,Chassis,sizeof(Chassis)) == 0);
    check(std::memcmp(console,&ConsoleGlobal,sizeof(ConsoleGlobal)) == 0);
    check(VehDrawSuppressedVehicle == suppressed);
}

int main() {
    const int ids[3] = {1,4,6};
    // All permutations of stacks of one to three nodes, all binary ownership
    // assignments, all land/sea/air assignments, every starting node and each
    // possible suppression (including none). Sparse ids expose index/position
    // confusion and distinct proto/chassis indices exercise both indirections.
    for (int count=1; count<=3; ++count) {
        int order[3] = {ids[0],ids[1],ids[2]};
        int configurations = 1;
        for (int i=0; i<count; ++i) configurations *= 3;
        do {
            for (int owners=0; owners<(1<<count); ++owners)
                for (int triads=0; triads<configurations; ++triads) {
                    reset(order,count,owners,triads);
                    for (int suppress=-1; suppress<count; ++suppress) {
                        VehDrawSuppressedVehicle = suppress<0 ? -1 : order[suppress];
                        for (int start=0; start<count; ++start) verify(order,count,order[start]);
                    }
                }
        } while (std::next_permutation(order,order+count));
    }
    const int order[3] = {6,1,4};
    const uint32_t states[] = {0,0x200,0x4000,0x4200,0x420000,0x42,0x80000000,0xFFFFBDFF};
    const uint32_t flags[] = {0,1,0x80000000};
    for (int owners=0; owners<8; ++owners)
        for (int selected=0; selected<3; ++selected)
            for (unsigned state=0; state<sizeof(states)/sizeof(states[0]); ++state)
                for (int cursor=0; cursor<3; ++cursor)
                    for (int enabled=0; enabled<3; ++enabled) {
                        reset(order,3,owners,11); // head air, middle land, tail sea
                        ConsoleGlobal.selected_vehicle_ = order[selected];
                        ConsoleGlobal.cursor_active_ = flags[cursor];
                        ConsoleGlobal.field_23BE4_ = flags[enabled];
                        Vehs[order[selected]].state = states[state];
                        for (int suppress=-1; suppress<3; ++suppress) {
                            VehDrawSuppressedVehicle = suppress<0 ? -1 : order[suppress];
                            for (int start=0; start<3; ++start) verify(order,3,order[start]);
                        }
                    }
    reset(order,3,7,0);
    ConsoleGlobal.selected_vehicle_ = 7; // outside the stack
    ConsoleGlobal.cursor_active_ = ConsoleGlobal.field_23BE4_ = 1;
    VehDrawSuppressedVehicle = -2; // only -1 is the named sentinel, but no id equals -2
    verify(order,3,1);
    const int negatives[] = {-1,-7,std::numeric_limits<int>::min()};
    for (int i=0; i<3; ++i) verify(order,3,negatives[i]);
    // Original byte fields are zero-extended, including unusual high values.
    reset(order,3,0,0);
    ConsoleGlobal.faction_id_ = 255;
    Vehs[1].faction_id = 255;
    verify(order,3,4);
    Chassis[VehPrototypes[Vehs[4].proto_id].chassis_id].triad = 255;
    verify(order,3,1);
    return 0;
}
"""


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required for bounded behavioral controls")
    source, executable = tmp_path / "selection.cpp", tmp_path / "selection"
    source.write_text(HARNESS.replace("// DECLARATIONS", declarations()).replace("// BODY", body))
    result = subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined",
        "-fno-sanitize-recover=undefined",str(source),"-o",str(executable)],
        text=True,capture_output=True,timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(executable)],text=True,capture_output=True,timeout=10)


def test_selection_native_identity():
    record, = (r for r in read_file(ROOT / "src/veh.cpp") if r.address == 0x5593E0)
    assert record.image_spans == ((0x5593E0,0x5594EB),)


def test_live_vehicle_selection(tmp_path):
    result = run_host(tmp_path,live_body())
    assert result.returncode == 0, result.stdout + result.stderr


def replace_once(body, old, new):
    assert body.count(old) == 1, old
    return body.replace(old,new)


@pytest.mark.parametrize("damage", [
    "wrong_mask", "last_tie", "no_head_climb", "enemy_priority", "ignore_suppression",
    "negative_zero", "ignore_enable", "no_owner_bonus", "mutate_input",
])
def test_selection_runtime_damage(tmp_path,damage):
    body = live_body()
    changes = {
        "wrong_mask": ("state & 0x4200", "state & 0x420000"),
        "last_tie": ("score > best_score", "score >= best_score"),
        "no_head_climb": ("while (previous >= 0)", "while (false && previous >= 0)"),
        "enemy_priority": ("&& Vehs[index].faction_id == faction", "&& true"),
        "ignore_suppression": ("VehDrawSuppressedVehicle == -1 || index != VehDrawSuppressedVehicle", "true"),
        "negative_zero": ("if (veh_id < 0) {\n        return best;", "if (veh_id < 0) {\n        return 0;"),
        "ignore_enable": ("&& ConsoleGlobal.field_23BE4_ != 0", "&& true"),
        "no_owner_bonus": ("? 4 : 0", "? 0 : 0"),
        "mutate_input": ("int index = veh_id;", "Vehs[veh_id].state ^= 1;\n    int index = veh_id;"),
    }
    body = replace_once(body,*changes[damage])
    result = run_host(tmp_path,body)
    # Compiler errors, signals and UBSan traps cannot satisfy a damage control.
    assert result.returncode == 91, result.stdout + result.stderr
