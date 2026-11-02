"""Actual staple body and pinned service-read phases, with projected storage.

The pin reads the rule flag after parse_says/has_fac and again after lock_base,
and reads ScriptPopupFile in each popup arm. Service doubles deliberately alter
those shared values. This does not prove the BaseWin layout or real GUI/network
callbacks; the initial compute_selected_base operation is a service boundary.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def actual():
    cpp = (ROOT / "src/basewin.cpp").read_text()
    body = braced(cpp, "void BaseWin::staple_the_bastards()")
    base = (ROOT / "src/base.h").read_text()
    faction = (ROOT / "src/faction.h").read_text()
    fields = []
    for name in ("faction_id_current", "nerve_staple_turns_left", "name_string"):
        declaration, = re.findall(r"^    (?:uint8_t " + name + r";|char " + name + r"\[25\];)$", base, re.M)
        fields.append(declaration)
    flags, = re.findall(r"^    int rule_flags;$", faction, re.M)
    social = braced(faction, "struct SocialEffect {") + ";"
    omniscient, = re.findall(r"^    STATE_OMNISCIENT_VIEW = (0x[0-9A-Fa-f]+),$", (ROOT / "src/game.h").read_text(), re.M)
    declarations = "struct Base {\n" + "\n".join(fields) + "\n};\nstruct Player {\n" + flags + "\n};\n" + social + "\nenum { STATE_OMNISCIENT_VIEW = " + omniscient + " };\n"
    return body, declarations


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
DECLARATIONS
Base Bases[4]; Base *BaseCurrent;
Player Players[4];
struct PlayerData { SocialEffect soc_effect_active; } PlayersData[4];
struct { int faction_id_; } ConsoleGlobal;
struct { struct { uint32_t game_state_; } fields_; } GameDataStateStorage;
int BaseIDCurrentSelected, IsMultiplayerNet;
char initial_script[]="INITIAL", locked_script[]="LOCKED";
char *ScriptPopupFile;
std::string events;
int mutation_phase, final_flags, lock_result, popup_result, facility;
int expected_alien, expected_case, unlocks, actions, messages, awaits;
bool change_script, change_selected, change_after_await;
const char *expected_script;
void mutate(int phase) {
    if (mutation_phase==phase) Players[2].rule_flags=final_flags;
}
int parse_says(int index, const char *name, int gender, int plurality) {
    assert(index==0 && name==Bases[1].name_string && gender==-1 && plurality==-1);
    events+='P'; mutate(1); return 0;
}
int has_fac(int id, int base, int flag) {
    assert(id==0x23 && base==1 && flag==0); events+='F'; mutate(2); return facility;
}
int popp(const char *script, const char *label, int flags,
         const char *picture, int (*callback)()) {
    events+='O';
    assert(script==expected_script && flags==0 && callback==0);
    const char *wanted_label=expected_case==0?"NERVESTAPLEHELLNO":
        expected_case==1?"NERVESTAPLED":"NERVESTAPLE";
    const char *wanted_picture=expected_case==0?
        (expected_alien?"Alopdir.pcx":"talent_sm.pcx"):
        (expected_alien?"ALdrone_sm.pcx":"drone_sm.pcx");
    assert(std::strcmp(label,wanted_label)==0);
    assert(std::strcmp(picture,wanted_picture)==0);
    if(change_selected) BaseIDCurrentSelected=3;
    return popup_result;
}
struct NetDaemon {
    int lock_base(int base, int target, int a, int b) {
        assert(base==1 && target==0 && a==-1 && b==-1);
        events+='L'; mutate(3);
        if(change_script) ScriptPopupFile=locked_script;
        return lock_result;
    }
    void unlock_base(int base) {
        assert(base==(change_after_await?2:change_selected?3:1));
        events+='U'; ++unlocks;
    }
    void await_exec(int mode) {
        assert(mode==1); events+='E'; ++awaits;
        if(change_after_await) BaseIDCurrentSelected=2;
    }
} NetDaemonNet;
int message_data(int op, int target, int base, int a, int b, int c) {
    assert(op==0x2414 && target==0 && base==(change_selected?3:1) && a==0 && b==0 && c==0);
    events+='M'; ++messages; return 0;
}
void action_staple(int base) {
    assert(base==(change_selected?3:1)); events+='A'; ++actions;
}
struct BaseWin {
    uint32_t focusedBaseID_;
    void compute_selected_base() { events+='C'; }
    void staple_the_bastards();
};
BODY
void run(int police, int stapled, int initial_flags, int phase, int replacement,
         bool locked_caption, int denied, int answer, bool network,
         const char *trace, bool selected=false, bool after_await=false,
         bool owner=true, bool omniscient=false, int fac=0) {
    std::memset(Bases,0,sizeof Bases); std::memset(Players,0,sizeof Players);
    std::memset(PlayersData,0,sizeof PlayersData);
    std::strcpy(Bases[1].name_string,"PINNED BASE");
    Bases[1].faction_id_current=2; Bases[1].nerve_staple_turns_left=stapled;
    BaseCurrent=&Bases[1]; ConsoleGlobal.faction_id_=owner?2:3;
    GameDataStateStorage.fields_.game_state_=omniscient?STATE_OMNISCIENT_VIEW:0;
    Players[2].rule_flags=initial_flags;
    PlayersData[2].soc_effect_active.values_[SocialEffect::POLICE]=police;
    BaseIDCurrentSelected=1; IsMultiplayerNet=network;
    ScriptPopupFile=initial_script; events.clear(); mutation_phase=phase;
    final_flags=replacement; lock_result=denied; popup_result=answer; facility=fac;
    change_script=locked_caption; change_selected=selected;
    change_after_await=after_await; unlocks=actions=messages=awaits=0;
    expected_alien=((phase?replacement:initial_flags)&0x80)!=0;
    expected_case=police+fac*2<0?0:stapled?1:2;
    expected_script=locked_caption?locked_script:initial_script;
    BaseWin win; win.focusedBaseID_=1; win.staple_the_bastards();
    assert(events==trace);
    assert(unlocks==(events.find('U')!=std::string::npos));
    assert(actions==(events.find('A')!=std::string::npos));
    assert(messages==(events.find('M')!=std::string::npos));
    assert(awaits==(events.find('E')!=std::string::npos));
}
int main() {
    // Both caption/picture arms and the two pre-lock read boundaries.
    run(-1,0,0,1,0x80,false,0,1,false,"CPFO");
    run(-1,0,0x80,2,0,false,0,1,false,"CPFO");
    run(0,1,0,1,0x80,false,0,1,false,"CPFO");
    run(0,1,0x80,2,0,false,0,1,false,"CPFO");
    // A successful lock is a further flag/script read boundary.
    run(0,0,0,3,0x80,true,0,0,false,"CPFLOU");
    run(0,0,0x80,3,0,true,0,1,false,"CPFLOAU",true);
    run(0,0,0,3,0x80,true,0,-3,true,"CPFLOMEU",true,true);
    run(0,0,0x80,3,0,true,0,0,true,"CPFLOU");
    run(0,0,0,3,0x80,true,1,1,true,"CPFL");
    // Existing permission, nonzero-answer and facility arithmetic quirks.
    run(0,0,0,0,0,false,0,1,false,"C",false,false,false,false);
    run(0,0,0,0,0,false,0,1,false,"CPFLOAU",false,false,false,true);
    run(-1,0,0,0,0,false,0,1,false,"CPFLOAU",false,false,true,false,1);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "early_flags", "cached_script"))
def test_actual_staple_service_read_phases(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body, declarations = actual()
    before = body
    if damage == "early_flags":
        assert body.count("Players[faction].rule_flags") == 3
        body = body.replace("Players[faction].rule_flags", "cached_flags")
        anchor = "    const int faction = BaseCurrent->faction_id_current;"
        assert body.count(anchor) == 1
        body = body.replace(anchor, anchor + "\n    const int cached_flags = Players[faction].rule_flags;")
    elif damage == "cached_script":
        assert body.count("popp(ScriptPopupFile,") == 6
        body = body.replace("popp(ScriptPopupFile,", "popp(cached_file,")
        anchor = "    if (police < 0) {"
        assert body.count(anchor) == 1
        body = body.replace(anchor, "    const char *cached_file = ScriptPopupFile;\n" + anchor)
    if damage is not None:
        assert body != before
    source = tmp_path / "actual_staple.cpp"
    source.write_text(HARNESS.replace("DECLARATIONS", declarations).replace("BODY", body))
    executable = tmp_path / "actual-staple"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode != 0) if damage else (ran.returncode == 0), ran.stdout + ran.stderr
