"""Actual AI editors retain their chosen faction across callback boundaries.

The pin keeps the editor_who result across all parsing and popup calls, then
reloads that faction's fields after the popup. Storage is projected with actual
member declarations; this is not an original layout, GUI or network ABI proof.
The bit-word examples cover the pinned priority masks and untouched bits, rather
than deriving expected results from the recovered implementation.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def production():
    header = (ROOT / "src/faction.h").read_text()
    fields = {}
    for owner, names in (
        ("Player", ("is_leader_female", "title_leader", "name_leader",
                    "noun_gender", "is_noun_plural", "noun_faction")),
        ("PlayerData", ("flags", "flags_ext", "ai_growth", "ai_tech",
                        "ai_wealth", "ai_power", "ai_fight")),
    ):
        declaration = braced(header, "struct " + owner + " {")
        selected = []
        for name in names:
            field, = re.findall(
                r"^\s*((?:BOOL|int|uint32_t|char) " + name +
                r"(?:\[\d+\])?;)", declaration, re.M)
            selected.append(field)
        fields[owner] = "struct " + owner + " {\n" + "\n".join(selected) + "\n};"
    source = (ROOT / "src/console_commands.cpp").read_text()
    bodies = {name: braced(source, "void Console::" + name + "()")
              for name in ("editor_personality", "editor_strategy")}
    return "\n".join(fields.values()), bodies


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
typedef int BOOL;
DECLARATIONS
Player Players[8]; PlayerData PlayersData[8];
int GenderDefault, PluralityDefault, PopupWriteCheckState;
int chosen, parser_calls, undo_calls, popup_calls, popup_result, answer;
bool strategy;
struct Console {
    int faction_id_;
    int editor_who(int original_faction, int first) {
        assert(original_faction==2 && first==1); return chosen;
    }
    void editor_personality();
    void editor_strategy();
} ConsoleGlobal;
void auto_undo() { ++undo_calls; }
int parse_says(int slot, const char *text, int gender, int plurality) {
    assert(slot==parser_calls && gender==-1 && plurality==-1);
    if(slot==0) {
        assert(text==Players[4].title_leader && GenderDefault==21 && PluralityDefault==0);
        Players[4].is_leader_female=22; ConsoleGlobal.faction_id_=5;
    } else if(slot==1) {
        assert(text==Players[4].name_leader && GenderDefault==22 && PluralityDefault==0);
        Players[4].noun_gender=31; Players[4].is_noun_plural=2;
        ConsoleGlobal.faction_id_=6;
    } else {
        assert(slot==2 && text==Players[4].noun_faction);
        assert(GenderDefault==31 && PluralityDefault==2);
        ConsoleGlobal.faction_id_=7;
    }
    ++parser_calls; return 0;
}
int x_pop_short(const char *label, int flags, void *callback) {
    ++popup_calls;
    assert(flags==0x41 && callback==0);
    assert(std::string(label)==(strategy?"AIEDITOR2":"AIEDITOR"));
    assert(static_cast<uint32_t>(PopupWriteCheckState)==
           (strategy?0x80001ad5u:0x800005adu));
    // The accepted path must preserve these fresh unrepresented bits rather
    // than the values used to stage the popup. The index still means faction4.
    PlayersData[4].flags=strategy?0x12345678u:0xc3f02468u;
    PlayersData[4].flags_ext=0xa5u;
    ConsoleGlobal.faction_id_=3;
    PopupWriteCheckState=answer;
    return popup_result;
}
BODIES
void reset(bool mode, int accepted, int result, int selected=4) {
    std::memset(Players,0,sizeof Players);
    std::memset(PlayersData,0,sizeof PlayersData);
    strategy=mode; chosen=selected; answer=accepted; popup_result=result;
    parser_calls=undo_calls=popup_calls=0; ConsoleGlobal.faction_id_=2;
    Players[4].is_leader_female=21;
    Players[4].noun_gender=17; Players[4].is_noun_plural=0;
    PlayerData &d=PlayersData[4];
    d.flags=mode?0x54200000u:0x01400011u;
    d.flags_ext=mode?0x55u:0x20u;
    d.ai_growth=5; d.ai_tech=0; d.ai_wealth=-7; d.ai_power=9; d.ai_fight=-2;
    PlayersData[3].flags=0x31415926u; PlayersData[3].flags_ext=0x27182818u;
    PopupWriteCheckState=static_cast<int>(mode?0x80001fffu:0x800007ffu);
}
void invoke() {
    if(strategy) ConsoleGlobal.editor_strategy();
    else ConsoleGlobal.editor_personality();
    assert(PlayersData[3].flags==0x31415926u && PlayersData[3].flags_ext==0x27182818u);
}
void personality(int bits, int fight, int growth, uint32_t flags, uint32_t extra) {
    reset(false,bits,0); invoke();
    assert(parser_calls==3 && popup_calls==1 && undo_calls==1);
    const PlayerData &d=PlayersData[4];
    assert(d.flags==flags && d.flags_ext==extra);
    assert(d.ai_fight==fight && d.ai_growth==growth);
    assert(d.ai_tech==growth && d.ai_wealth==growth && d.ai_power==growth);
}
int main() {
    personality(static_cast<int>(0x800007ffu),0,1,0xc3f02468u,0xa5u);
    personality(0x10,1,0,0xc0302468u,0x85u);
    personality(0x20,-1,0,0xc0302468u,0x85u);
    personality(0,0,0,0xc0302468u,0x85u);
    reset(false,0x7ff,-1); invoke();
    assert(PlayersData[4].flags==0xc3f02468u && PlayersData[4].flags_ext==0xa5u);
    assert(PlayersData[4].ai_growth==5 && PlayersData[4].ai_wealth==-7 && PlayersData[4].ai_fight==-2);
    reset(true,static_cast<int>(0x80001fffu),0); invoke();
    assert(PlayersData[4].flags==0xfe345678u && PlayersData[4].flags_ext==0xffu);
    reset(true,0,0); invoke();
    assert(PlayersData[4].flags==0x02145678u && PlayersData[4].flags_ext==0xa0u);
    reset(true,0x1081,0); invoke();
    assert(PlayersData[4].flags==0x06145678u && PlayersData[4].flags_ext==0xe1u);
    reset(true,0x1fff,-2); invoke();
    assert(PlayersData[4].flags==0x12345678u && PlayersData[4].flags_ext==0xa5u);
    reset(false,0,0,-1); invoke();
    assert(parser_calls==0 && popup_calls==0 && undo_calls==0);
    reset(true,0,0,-1); invoke();
    assert(parser_calls==0 && popup_calls==0 && undo_calls==0);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "live-global-faction", "stale-flags"))
def test_actual_ai_editors_keep_faction_and_reload_after_popup(
        compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    declarations, bodies = production()
    for name, body in list(bodies.items()):
        before = body
        member = "data." if "PlayerData &data = PlayersData[faction];" in body else "PlayersData[faction]."
        if damage == "live-global-faction":
            if member == "data.":
                anchor = "PlayerData &data = PlayersData[faction];"
                assert body.count(anchor) == 1
                body = body.replace(anchor, "PlayerData &data = PlayersData[ConsoleGlobal.faction_id_];")
            else:
                assert "PlayersData[faction]." in body
                body = body.replace("PlayersData[faction].", "PlayersData[ConsoleGlobal.faction_id_].")
        elif damage == "stale-flags":
            anchor = ('    if (x_pop_short("AIEDITOR",' if name == "editor_personality"
                      else '    const int result = x_pop_short("AIEDITOR2",')
            assert body.count(anchor) == 1
            body = body.replace(anchor,
                "    const uint32_t stale_flags = PlayersData[faction].flags;\n"
                "    const uint32_t stale_ext = PlayersData[faction].flags_ext;\n" + anchor)
            assert body.count(member + "flags &=") == 1
            assert body.count(member + "flags_ext &=") == 1
            body = body.replace(member + "flags &=", member + "flags = stale_flags &")
            body = body.replace(member + "flags_ext &=", member + "flags_ext = stale_ext &")
        if damage:
            assert body != before
        bodies[name] = body
    cpp = tmp_path / "ai-editors.cpp"
    cpp.write_text(HARNESS.replace("DECLARATIONS", declarations).replace(
        "BODIES", "\n".join(bodies.values())))
    exe = tmp_path / "ai-editors"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    (tmp_path / "run.stderr").write_text(ran.stderr)
    assert (ran.returncode != 0) if damage else (ran.returncode == 0), ran.stdout + ran.stderr
