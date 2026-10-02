"""Actual-source milestone producer controls, independent of seed scaffolds."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_monument_records_behavior import body

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("function,index,payload", [
    ("mon_tech_discovered", 1, "tech_id"),
    ("mon_secret_project", 5, "project_id"),
    ("mon_built_preserve", 12, None),
])
def test_milestone_producer(tmp_path, compiler_name, function, index, payload):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    header = (ROOT / "src/game.h").read_text()
    layout = header[header.index("struct MonumentRecord {"):
                    header.index("extern MonumentFaction FactionMonuments")]
    parameters = "int faction_id" + (f", int {payload}" if payload else "")
    implementation = body("src/game.cpp", f"void __cdecl {function}({parameters}) {{")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
enum { MaxPlayerNum=8, RFLAG_ALIEN=0x80, event_index=__INDEX__ };
__LAYOUT__
MonumentFaction FactionMonuments[8], expected[8];
struct Player { int rule_flags; } Players[8];
struct Console { int faction_id_; } ConsoleGlobal;
int TurnCurrentNum=21, years, notifications, changed_local=-1, set_alien=0;
uint32_t game_year(int turn) {
    assert(turn==21); ++years;
    if (changed_local!=-1) ConsoleGlobal.faction_id_=changed_local;
    if (set_alien) Players[3].rule_flags=RFLAG_ALIEN;
    return 2150;
}
void monument(int from_load) {
    assert(from_load==1); ++notifications;
    assert(FactionMonuments[3].latest_type_==event_index);
    assert(FactionMonuments[3].records_[event_index].achieved_==1);
}
__BODY__
void reset() {
    memset(FactionMonuments,0,sizeof FactionMonuments);
    memset(Players,0,sizeof Players);
    FactionMonuments[3].records_[event_index].word_8_=123;
    years=notifications=0; changed_local=-1; set_alien=0;
    ConsoleGlobal.faction_id_=3;
}
void invoke(int value) { __INVOKE__; }
void expect_record(int first, int flag, int value) {
    memcpy(expected,FactionMonuments,sizeof expected);
    MonumentRecord &r=expected[3].records_[event_index];
    r.achieved_=1; r.year_=2150; r.owner_faction_=first; r.word_14_=flag;
    __PAYLOAD_STORE__
    expected[3].latest_type_=event_index;
}
int main() {
    static_assert(sizeof(MonumentRecord)==0x38,"native record stride");
    static_assert(sizeof(MonumentFaction)==0x4F4,"native faction stride");
    reset(); expect_record(3,1,-7); invoke(-7);
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);
    assert(years==1 && notifications==1);
    invoke(99); // First event is retained, including original payload.
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);
    assert(years==2 && notifications==1);

    reset(); Players[3].rule_flags=RFLAG_ALIEN;
    memcpy(expected,FactionMonuments,sizeof expected); invoke(17);
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);
    assert(years==1 && notifications==0);

    reset(); set_alien=1; // Observe callbacks before evaluating the guard.
    memcpy(expected,FactionMonuments,sizeof expected); invoke(17);
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);
    assert(years==1 && notifications==0);

    reset();
    FactionMonuments[0].records_[event_index].achieved_=1;
    FactionMonuments[0].records_[event_index].word_14_=1;
    FactionMonuments[7].records_[event_index].achieved_=-1;
    FactionMonuments[7].records_[event_index].word_14_=-2;
    Players[7].rule_flags=RFLAG_ALIEN; // Peers are not filtered.
    expect_record(7,0,41); invoke(41);
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);

    reset(); FactionMonuments[1].records_[event_index].achieved_=1;
    expect_record(3,0,42); invoke(42); // No peer carries first-achiever state.
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);

    reset(); ConsoleGlobal.faction_id_=1; changed_local=3;
    expect_record(3,1,43); invoke(43);
    assert(memcmp(expected,FactionMonuments,sizeof expected)==0);
    assert(notifications==1);
    reset(); ConsoleGlobal.faction_id_=1; invoke(44);
    assert(notifications==0);
}
'''
    replacements = {
        "__INDEX__": str(index), "__LAYOUT__": layout, "__BODY__": implementation,
        "__INVOKE__": f"{function}(3" + (", value)" if payload else ")"),
        "__PAYLOAD_STORE__": "r.word_8_=value;" if payload else "(void)value;",
    }
    for key, value in replacements.items():
        harness = harness.replace(key, value)
    source = tmp_path / "milestone.cpp"
    executable = tmp_path / "milestone"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", "-fsanitize=address,undefined",
                    "-fno-omit-frame-pointer", "-no-pie", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    # No heap allocation; disable only ptrace-incompatible leak enumeration.
    subprocess.run([str(executable)], check=True, capture_output=True, text=True,
                   timeout=10, env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
