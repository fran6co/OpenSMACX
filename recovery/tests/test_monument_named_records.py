"""Actual-source controls for facility/conquest milestone and name ownership."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_monument_records_behavior import body


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("function,index,member,offset", [
    ("mon_facility_built", 4, "facility_base_name_", "0x4C4"),
    ("mon_conquer_base", 7, "conquered_base_name_", "0x4DC"),
])
def test_named_milestone_native_flow(tmp_path, compiler_name, function, index, member, offset):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    header = (ROOT / "src/game.h").read_text()
    layout = header[header.index("struct MonumentRecord {"):
                    header.index("extern MonumentFaction FactionMonuments")]
    implementation = body("src/game.cpp", f"void __cdecl {function}(int faction_id, char *base_name) {{")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#define __cdecl
enum { MaxPlayerNum=8, RFLAG_ALIEN=0x80 };
__LAYOUT__
MonumentFaction FactionMonuments[8];
struct Player { int rule_flags; } Players[8];
struct Console { int faction_id_; } ConsoleGlobal;
int TurnCurrentNum=21, year_calls, popup_calls, change_local=-1;
uint32_t game_year(int turn) {
    assert(turn==21); ++year_calls;
    if (change_local!=-1) ConsoleGlobal.faction_id_=change_local;
    return 2150;
}
void monument(int from_load) { assert(from_load==1); ++popup_calls; }
__BODY__
void reset() {
    memset(FactionMonuments,0,sizeof FactionMonuments);
    memset(Players,0,sizeof Players);
    year_calls=popup_calls=0; change_local=-1; ConsoleGlobal.faction_id_=3;
}
void invoke(char *name) { __FUNCTION__(3,name); }
int main() {
    static_assert(sizeof(MonumentRecord)==0x38,"record stride");
    static_assert(sizeof(MonumentFaction)==0x4F4,"faction stride");
    static_assert(offsetof(MonumentFaction,__MEMBER__)==__OFFSET__,"name owner");
    char name[]="Milestone Base";
    reset();
    memset(FactionMonuments[3].__MEMBER__,0x7F,24);
    FactionMonuments[3].records_[1].year_=123;
    MonumentFaction expected=FactionMonuments[3];
    expected.records_[__INDEX__].year_=2150;
    expected.records_[__INDEX__].owner_faction_=3;
    expected.records_[__INDEX__].achieved_=1;
    expected.records_[__INDEX__].word_14_=1;
    expected.latest_type_=__INDEX__;
    strncpy(expected.__MEMBER__,name,24);
    invoke(name);
    assert(memcmp(&expected,&FactionMonuments[3],sizeof expected)==0);
    assert(popup_calls==1 && year_calls==1);
    invoke(name);
    assert(memcmp(&expected,&FactionMonuments[3],sizeof expected)==0);
    assert(popup_calls==1 && year_calls==2);

    reset(); Players[3].rule_flags=RFLAG_ALIEN;
    expected=FactionMonuments[3]; invoke(name);
    assert(year_calls==1 && popup_calls==0);
    assert(memcmp(&expected,&FactionMonuments[3],sizeof expected)==0);

    reset();
    FactionMonuments[0].records_[__INDEX__].achieved_=1;
    FactionMonuments[0].records_[__INDEX__].word_14_=1;
    FactionMonuments[7].records_[__INDEX__].achieved_=-1;
    FactionMonuments[7].records_[__INDEX__].word_14_=-2;
    Players[7].rule_flags=RFLAG_ALIEN; // Native includes alien peer records.
    invoke(name);
    assert(FactionMonuments[3].records_[__INDEX__].owner_faction_==7);
    assert(FactionMonuments[3].records_[__INDEX__].word_14_==0);

    reset(); FactionMonuments[1].records_[__INDEX__].achieved_=1;
    invoke(name);
    assert(FactionMonuments[3].records_[__INDEX__].owner_faction_==3);
    assert(FactionMonuments[3].records_[__INDEX__].word_14_==0);

    reset(); ConsoleGlobal.faction_id_=1; change_local=3;
    char long_name[]="123456789012345678901234EXTRA";
    expected=FactionMonuments[3];
    expected.records_[__INDEX__].year_=2150;
    expected.records_[__INDEX__].owner_faction_=3;
    expected.records_[__INDEX__].achieved_=1;
    expected.records_[__INDEX__].word_14_=1;
    expected.latest_type_=__INDEX__;
    memcpy(expected.__MEMBER__,long_name,24);
    invoke(long_name);
    assert(popup_calls==1 && FactionMonuments[3].__MEMBER__[23]=='4');
    assert(memcmp(&expected,&FactionMonuments[3],sizeof expected)==0);
    reset(); ConsoleGlobal.faction_id_=1; invoke(name);
    assert(popup_calls==0);
}
'''
    for old, new in {
        "__LAYOUT__": layout, "__BODY__": implementation,
        "__FUNCTION__": function, "__MEMBER__": member,
        "__INDEX__": str(index), "__OFFSET__": offset,
    }.items():
        harness = harness.replace(old, new)
    source = tmp_path / "monument.cpp"
    executable = tmp_path / "monument"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", "-fsanitize=address,undefined",
                    "-fno-omit-frame-pointer", "-no-pie", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    # LeakSanitizer cannot enumerate threads under the sandbox's ptrace boundary.
    # These controls allocate no heap; address/undefined checks remain enabled.
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    subprocess.run([str(executable)], check=True, capture_output=True, text=True,
                   timeout=10, env=environment)
