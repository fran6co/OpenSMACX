"""Actual-source controls for shared monument record ownership and milestones."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(path, signature):
    text = (ROOT / path).read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated {signature}")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
#define __cdecl
enum { MaxPlayerNum=8, RFLAG_ALIEN=0x80 };
__LAYOUT__
MonumentFaction FactionMonuments[MaxPlayerNum];
struct Player { int rule_flags; } Players[MaxPlayerNum];
struct Console { int faction_id_; } ConsoleGlobal;
int TurnCurrentNum, year_value, year_calls, popup_calls, change_local=-1;
uint32_t game_year(int turn) {
    assert(turn==TurnCurrentNum); ++year_calls;
    if (change_local!=-1) ConsoleGlobal.faction_id_=change_local;
    return static_cast<uint32_t>(year_value);
}
void monument(int from_load) { assert(from_load==1); ++popup_calls; }
__BODIES__
void reset() {
    std::memset(FactionMonuments,0,sizeof FactionMonuments);
    std::memset(Players,0,sizeof Players);
    ConsoleGlobal.faction_id_=3; TurnCurrentNum=21; year_value=2150;
    year_calls=popup_calls=0; change_local=-1;
}
int main() {
    static_assert(offsetof(MonumentRecord,achieved_)==0x10,"native flag");
    static_assert(offsetof(MonumentRecord,word_14_)==0x14,"native first flag");
    static_assert(offsetof(MonumentFaction,latest_type_)==0x498,"native latest type");
    reset(); mon_secrets_of_tech(3);
    MonumentRecord &secret=FactionMonuments[3].records_[2];
    assert(year_calls==1 && popup_calls==1);
    assert(secret.year_==2150 && secret.owner_faction_==3);
    assert(secret.achieved_==1 && secret.word_14_==1);
    assert(secret.word_4_==-1 && secret.word_8_==-1);
    assert(FactionMonuments[3].latest_type_==2);
    year_value=2160; mon_secrets_of_tech(3);
    assert(secret.word_8_==2160 && secret.word_4_==-1 && secret.year_==2150);
    year_value=2170; mon_secrets_of_tech(3);
    assert(secret.word_8_==2160 && secret.word_4_==2170);
    year_value=2180; mon_secrets_of_tech(3);
    assert(secret.word_8_==2160 && secret.word_4_==2170 && popup_calls==4);

    reset(); Players[3].rule_flags=RFLAG_ALIEN;
    MonumentFaction before=FactionMonuments[3]; mon_secrets_of_tech(3);
    assert(year_calls==1 && popup_calls==0);
    assert(std::memcmp(&before,&FactionMonuments[3],sizeof before)==0);

    void (*triggers[5])(int)={mon_prototype_built,mon_naval_unit_built,
      mon_air_unit_built,mon_native_life_bred,mon_first_in_space};
    const int types[5]={3,8,9,10,11};
    for (int trigger=0;trigger<5;++trigger) {
      const int type=types[trigger];
      reset(); triggers[trigger](3);
      const MonumentRecord &record=FactionMonuments[3].records_[type];
      assert(record.year_==2150 && record.owner_faction_==3);
      assert(record.achieved_==1 && record.word_14_==1);
      assert(FactionMonuments[3].latest_type_==type && popup_calls==1);
      before=FactionMonuments[3]; triggers[trigger](3);
      assert(year_calls==2 && popup_calls==1);
      assert(std::memcmp(&before,&FactionMonuments[3],sizeof before)==0);
      reset(); Players[3].rule_flags=RFLAG_ALIEN; triggers[trigger](3);
      assert(year_calls==1 && popup_calls==0);
      assert(FactionMonuments[3].records_[type].achieved_==0);
      reset();
      FactionMonuments[1].records_[type].achieved_=1;
      FactionMonuments[1].records_[type].word_14_=1;
      FactionMonuments[7].records_[type].achieved_=-1;
      FactionMonuments[7].records_[type].word_14_=-2;
      triggers[trigger](3);
      assert(FactionMonuments[3].records_[type].owner_faction_==7);
      assert(FactionMonuments[3].records_[type].word_14_==0);
      assert(FactionMonuments[3].latest_type_==type);
      reset(); ConsoleGlobal.faction_id_=1; triggers[trigger](3);
      assert(FactionMonuments[3].latest_type_==type && popup_calls==0);
    }

    reset();
    FactionMonuments[1].records_[2].achieved_=1;
    FactionMonuments[1].records_[2].word_14_=1;
    FactionMonuments[7].records_[2].achieved_=-4;
    FactionMonuments[7].records_[2].word_14_=-2;
    mon_secrets_of_tech(3);
    assert(FactionMonuments[3].records_[2].owner_faction_==7);
    assert(FactionMonuments[3].records_[2].word_14_==0);
    reset(); FactionMonuments[1].records_[2].achieved_=1;
    mon_secrets_of_tech(3);
    assert(FactionMonuments[3].records_[2].owner_faction_==3);
    assert(FactionMonuments[3].records_[2].word_14_==0);
    reset(); ConsoleGlobal.faction_id_=1; change_local=3; mon_secrets_of_tech(3);
    assert(popup_calls==1); // Local receiver is read after game_year.

    reset(); mon_winning_unify(3,6);
    MonumentRecord &unify=FactionMonuments[3].records_[13];
    assert(unify.year_==2150 && unify.owner_faction_==3 && unify.word_8_==6);
    assert(unify.word_14_==1 && unify.achieved_==1);
    assert(FactionMonuments[3].latest_type_==13 && popup_calls==1);
    before=FactionMonuments[3]; mon_winning_unify(3,2);
    assert(std::memcmp(&before,&FactionMonuments[3],sizeof before)==0);
    assert(year_calls==2 && popup_calls==1);
    reset(); Players[3].rule_flags=RFLAG_ALIEN; mon_winning_trans(3,6);
    assert(year_calls==1 && popup_calls==0 && FactionMonuments[3].latest_type_==0);
    reset(); mon_winning_trans(3,6);
    const MonumentRecord &trans=FactionMonuments[3].records_[14];
    assert(trans.year_==2150 && trans.word_8_==6 && trans.owner_faction_==3);
    assert(trans.word_14_==1 && trans.achieved_==1);
    assert(FactionMonuments[3].latest_type_==14 && popup_calls==1);

    reset(); FactionMonuments[3].records_[15].year_=0;
    FactionMonuments[3].records_[15].achieved_=1;
    FactionMonuments[3].records_[16].year_=123;
    mon_killed_faction(3,5);
    const MonumentRecord &killed=FactionMonuments[3].records_[16];
    assert(killed.year_==2150 && killed.word_8_==5 && killed.owner_faction_==3);
    assert(killed.achieved_==1 && killed.word_14_==1);
    assert(FactionMonuments[3].latest_type_==16 && popup_calls==1);
    assert(FactionMonuments[3].records_[15].year_==0);
    reset(); Players[3].rule_flags=RFLAG_ALIEN; mon_killed_faction(3,5);
    assert(FactionMonuments[3].records_[15].achieved_==1 && popup_calls==0);
    reset(); for (int index=15;index<21;++index)
      FactionMonuments[3].records_[index].achieved_=1;
    before=FactionMonuments[3]; mon_killed_faction(3,5);
    assert(year_calls==1 && popup_calls==0);
    assert(std::memcmp(&before,&FactionMonuments[3],sizeof before)==0);
}
"""


def test_shared_monument_records_and_native_milestone_flow(tmp_path):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    header = (ROOT / "src/game.h").read_text()
    layout = header[header.index("struct MonumentRecord {"):
                    header.index("extern MonumentFaction FactionMonuments")]
    bodies = [body("src/game.cpp", "void __cdecl mon_secrets_of_tech(int faction_id) {")]
    bodies += [body("src/leaf_recoveries.cpp", f"void __cdecl {name}(int faction_id, int counterpart) {{")
               for name in ("mon_winning_unify", "mon_winning_trans", "mon_killed_faction")]
    bodies += [body("src/leaf_recoveries.cpp", f"void __cdecl {name}(int faction_id) {{")
               for name in ("mon_prototype_built", "mon_naval_unit_built", "mon_air_unit_built",
                            "mon_native_life_bred", "mon_first_in_space")]
    source = tmp_path / "monuments.cpp"
    executable = tmp_path / "monuments"
    source.write_text(HARNESS.replace("__LAYOUT__", layout).replace("__BODIES__", "\n".join(bodies)))
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)
