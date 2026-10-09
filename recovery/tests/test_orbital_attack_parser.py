"""Exercise the actual orbital body, including distinct faction grammar fields.

The host harness adapts only the 32-bit pointer-to-parser-slot conversion to
intptr_t; native VC6 compilation remains a separate coordinator check.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def orbital_body():
    source = (ROOT / "src/veh.cpp").read_text()
    match = re.search(r"void __cdecl action_sat_attack\([^;{]*\{", source)
    assert match
    depth = 1
    end = match.end()
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[match.start():end].replace("reinterpret_cast<int>(", "reinterpret_cast<intptr_t>(")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
struct Player { int is_leader_female, noun_gender, is_noun_plural;
 char title_leader[32],name_leader[32],noun_faction[32];
 char adj_name_faction[32],formal_name_faction[32]; };
struct PlayerData { unsigned diplo_treaties[8],satellites_[4]; int sat_odp_deployed; };
Player Players[8]; PlayerData PlayersData[8];
struct Console { int faction_id_; } ConsoleGlobal;
int GenderDefault,PluralityDefault;
struct FacilityRow { char *name; } Facility[69];
char title[]="title", facility[]="facility";
char *ScriptPopupFile=title;
struct Report { int field_8_; void on_redraw() {} } ReportIfGlobal;
int IfaceIdToSlotStorage[8],ActiveIfaceIdStorage;
const char *slots[8]; int slot_calls[8]; int crosses;
void double_cross(int,int,int) { ++crosses; }
void parse_says(int slot,char *text,int,int) { slots[slot]=text; ++slot_calls[slot]; }
void parse_say(int slot,intptr_t text,int,int) { slots[slot]=reinterpret_cast<char *>(text); ++slot_calls[slot]; }
void set_fac(int,int,int) {}
int popp(char *,const char *,int,const char *,int) { return 0; }
int rand() { return 0; }
// BODY
int main() {
 std::strcpy(Players[2].adj_name_faction,"target adjective");
 std::strcpy(Players[2].formal_name_faction,"different formal name");
 ConsoleGlobal.faction_id_=1;
 PlayersData[1].diplo_treaties[2]=0x10;
 PlayersData[2].satellites_[0]=2;
 Facility[65].name=facility;
 action_sat_attack(1,2,0,0);
 assert(slot_calls[7]==1);
 assert(slots[7]==Players[2].adj_name_faction);
 assert(std::strcmp(slots[7],"target adjective")==0);
 assert(PlayersData[2].satellites_[0]==1);
 assert(PlayersData[1].sat_odp_deployed==1);
 assert(crosses==0);
 // Neither participant is local: parser state must remain untouched.
 ConsoleGlobal.faction_id_=3;
 action_sat_attack(1,2,0,0);
 assert(slot_calls[7]==1);
 assert(PlayersData[2].satellites_[0]==0);
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "wrong_member", "missing_slot"])
def test_live_orbital_parser(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    body = orbital_body()
    line = "parse_says(7, Players[target].adj_name_faction, -1, -1);"
    assert body.count(line) == 1
    if damage == "wrong_member":
        body = body.replace(line, line.replace("adj_name_faction", "formal_name_faction"))
    elif damage == "missing_slot":
        body = body.replace(line, "")
    source = tmp_path / "orbital.cpp"
    source.write_text(HARNESS.replace("// BODY", body))
    binary = tmp_path / "orbital"
    compilation = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", str(source), "-o", str(binary)],
        capture_output=True, text=True,
    )
    assert compilation.returncode == 0, compilation.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    if damage is None:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0
        assert "Assertion" in result.stderr or "assertion" in result.stderr
