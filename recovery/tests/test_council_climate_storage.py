"""Actual-source controls for the shared Council climate-state consumer."""
from pathlib import Path
import shutil
import subprocess

import pytest


def test_climate_step_threshold_direction_and_callback_reads(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (Path(__file__).resolve().parents[2] / "src/game.cpp").read_text()
    start = source.index("void __cdecl climactic_changes()")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated production body")
    harness = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
struct BaseSecretProject { int project_base[64]; int empath_guild() const { return -1; } };
#include "@@GAME_DATA_HEADER@@"
GameDataStateWindow GameDataStateStorage;
int &ClimateRiseStep = GameDataStateStorage.fields_.climate_rise_step_;
int &ClimateRiseLimit = GameDataStateStorage.fields_.climate_rise_limit_;
int &ClimateRiseAccum = GameDataStateStorage.fields_.climate_rise_accum_;
int &ClimateSeaLevelDelta = GameDataStateStorage.fields_.climate_sea_level_delta_;
int MapSeaLevel;
bool ChangeState;
char Script[]="SCRIPT", UpdatedScript[]="UPDATED";
char *ScriptPopupFile=Script;
std::vector<std::string> Events;
void world_climate() {
    Events.push_back("climate");
    if (ChangeState) {
        ClimateSeaLevelDelta=-8;
        ScriptPopupFile=UpdatedScript;
    }
}
void draw_map(int value) { assert(value==1); Events.push_back("draw"); }
int popp(char *file,const char *label,int value,const char *picture,void *callback) {
    assert(value==0 && !callback);
    assert(file==(ChangeState?UpdatedScript:Script));
    assert(!std::strcmp(picture,!std::strcmp(label,"SEARISING")?
                       "searis_sm.pcx":"seafal_sm.pcx"));
    Events.push_back(label); return 1;
}
'''
    harness += body
    harness += r'''
void reset(int delta,int step,int limit,int accum) {
    ClimateSeaLevelDelta=delta; ClimateRiseStep=step; ClimateRiseLimit=limit;
    ClimateRiseAccum=accum; MapSeaLevel=100; ChangeState=false;
    Events.clear(); ScriptPopupFile=Script;
}
int main() {
    reset(0,8,20,7); climactic_changes();
    assert(ClimateRiseAccum==7 && MapSeaLevel==100 && Events.empty());
    reset(3,8,20,7); climactic_changes();
    assert(ClimateRiseAccum==15 && ClimateSeaLevelDelta==3 && Events.empty());
    reset(3,8,20,12); climactic_changes();
    assert(ClimateRiseAccum==0 && ClimateSeaLevelDelta==2 && MapSeaLevel==101);
    assert((Events==std::vector<std::string>{"climate","draw","SEARISING"}));
    reset(-3,8,20,12); climactic_changes();
    assert(ClimateRiseAccum==0 && ClimateSeaLevelDelta==-2 && MapSeaLevel==99);
    assert((Events==std::vector<std::string>{"climate","draw","SEAFALLING"}));
    reset(3,50,20,0); climactic_changes();
    assert(ClimateRiseAccum==30 && ClimateSeaLevelDelta==2 && MapSeaLevel==101);
    assert(Events.size()==3); // Only one step per call, not a draining loop.
    reset(1,20,20,0); ChangeState=true; climactic_changes();
    assert(ClimateSeaLevelDelta==-8 && MapSeaLevel==101);
    assert(Events.back()=="SEARISING"); // Saved sign, freshly read popup file.
}
'''
    harness = harness.replace("@@GAME_DATA_HEADER@@", str(Path(__file__).resolve().parents[2] / "src/game_data_state_window.h"))
    unit = tmp_path / "climate.cpp"
    unit.write_text(harness)
    executable = tmp_path / "climate"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
