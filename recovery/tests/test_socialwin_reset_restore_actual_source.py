"""Bounded reset restoration/callback controls using the actual complete body.

These host boundary doubles check state routing, preserved saved allocations
and callback sequencing. Native layout, matching and runtime acceptance remain
separate measurements.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    source = (ROOT / "src/socialwin.cpp").read_text()
    start = source.index("void SocialWin::reset()")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual reset body")


FIXTURE = r'''
#include <cassert>
#include <cstdint>
#include <vector>
const int MaxSocialEffectNum=11;
struct SocialCategory { uint32_t models_[4]; };
struct SocialEffect { int values_[11]; };
struct PlayerData {
    SocialCategory soc_category_pending;
    SocialEffect soc_effect_pending;
    int se_alloc_psych,se_alloc_labs;
} PlayersData[10];
struct SocialWin;
SocialWin *active;
int BreakthroughTurnsGlobal,NetIncomeGlobal;
bool switch_factions,show_window,changed_case;
int group_calls,set_calls,calc_calls,energy_calls,redraw_calls;
uint32_t saved_psych,saved_labs;
struct ButtonGroup { int category; int set(int,int); };
struct SocialWin {
    uint32_t field_A1C_,field_2C28_,factionID_;
    SocialCategory socCategory_;
    SocialEffect socEffect_,socEffectOrig_;
    ButtonGroup buttonGroups_[4];
    uint32_t savedAlloc_[3],pendingAlloc_[3];
    int breakthroughTurns_,netIncome_;
    int is_visible() { assert(field_2C28_==0); return show_window; }
    void redraw() { assert(field_2C28_==0); ++redraw_calls; }
    void reset();
};
int current_after_groups() { return switch_factions?4:0; }
int after_set() { return switch_factions?5:0; }
int before_energy() { return switch_factions?(changed_case?7:4):0; }
int final_row() { return switch_factions?8:0; }
int ButtonGroup::set(int value,int repaint) {
    assert(active->field_2C28_==1 && category==group_calls && repaint==1);
    int row=switch_factions?category:0;
    assert(active->factionID_==static_cast<uint32_t>(row));
    int model=static_cast<int>(PlayersData[row].soc_category_pending.models_[category]);
    assert(value==model+category*4);
    assert(active->socCategory_.models_[category]==static_cast<uint32_t>(model));
    ++group_calls;
    if(switch_factions)active->factionID_=static_cast<uint32_t>(category+1);
    return 0;
}
void social_ai(int faction,int a,int b,int c,int d,SocialCategory *out) {
    assert(faction==0 && a==-1 && b==-1 && c==-1 && d==-1 && out==0);
    assert(active->field_A1C_==0xffffffffu);
}
void social_set(int faction) {
    assert(changed_case && faction==current_after_groups() && set_calls++==0);
    assert(PlayersData[faction].se_alloc_psych==static_cast<int>(saved_psych));
    assert(PlayersData[faction].se_alloc_labs==static_cast<int>(saved_labs));
    assert(active->savedAlloc_[1]==saved_psych && active->savedAlloc_[2]==saved_labs);
    if(switch_factions)active->factionID_=5;
}
void social_calc(SocialCategory *category,SocialEffect *effect,int faction,int a,int b) {
    assert(a==0 && b==0 && active->field_2C28_==0);
    if(changed_case && calc_calls<2) {
        if(calc_calls==0) {
            assert(faction==after_set());
            assert(category==&active->socCategory_ && effect==&active->socEffect_);
            if(switch_factions)active->factionID_=6;
        } else {
            int expected=switch_factions?6:0;
            assert(faction==expected && category==&active->socCategory_);
            assert(effect==&PlayersData[expected].soc_effect_pending);
            if(switch_factions)active->factionID_=7;
        }
    } else {
        assert(energy_calls==1 && faction==final_row());
        assert(category==&PlayersData[faction].soc_category_pending);
        assert(effect==&PlayersData[faction].soc_effect_pending);
        assert(active->breakthroughTurns_==42 && active->netIncome_==43);
    }
    ++calc_calls;
}
void energy_compute(int faction,int recompute) {
    assert(faction==before_energy() && recompute==1 && energy_calls++==0);
    assert(active->pendingAlloc_[0]==5 && active->pendingAlloc_[1]==saved_psych);
    assert(active->pendingAlloc_[2]==saved_labs);
    if(switch_factions)active->factionID_=8;
    BreakthroughTurnsGlobal=42;NetIncomeGlobal=43;
}
ACTUAL_BODY
void run(bool changed,bool callbacks,bool visible,bool bit_patterns) {
    SocialWin window{};active=&window;
    changed_case=changed;switch_factions=callbacks;show_window=visible;
    group_calls=set_calls=calc_calls=energy_calls=redraw_calls=0;
    saved_psych=bit_patterns?0x80000002u:3u;saved_labs=bit_patterns?0xffffffffu:2u;
    window.savedAlloc_[0]=5;window.savedAlloc_[1]=saved_psych;window.savedAlloc_[2]=saved_labs;
    for(int row=0;row<10;++row) {
        for(int category=0;category<4;++category)
            PlayersData[row].soc_category_pending.models_[category]=row*100+category;
        for(int i=0;i<11;++i)PlayersData[row].soc_effect_pending.values_[i]=row*1000+i;
        PlayersData[row].se_alloc_psych=static_cast<int>(saved_psych)+(changed?1:0);
        PlayersData[row].se_alloc_labs=static_cast<int>(saved_labs)+(changed?1:0);
    }
    for(int i=0;i<4;++i)window.buttonGroups_[i].category=i;
    window.reset();
    assert(group_calls==4 && set_calls==(changed?1:0));
    assert(calc_calls==(changed?3:1) && energy_calls==1 && redraw_calls==(visible?1:0));
    assert(window.field_A1C_==0xffffffffu && window.field_2C28_==0);
    assert(window.savedAlloc_[1]==saved_psych && window.savedAlloc_[2]==saved_labs);
    int row=current_after_groups();
    assert(PlayersData[row].se_alloc_psych==static_cast<int>(saved_psych));
    assert(PlayersData[row].se_alloc_labs==static_cast<int>(saved_labs));
    for(int i=0;i<11;++i) {
        assert(window.socEffect_.values_[i]==row*1000+i);
        assert(window.socEffectOrig_.values_[i]==row*1000+i);
    }
}
int main() {
    run(true,true,true,false);
    run(false,true,false,false);
    run(true,false,false,true);
    run(false,false,true,false);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "reverse_allocations", "stale_category",
                                    "stale_second_calc", "stale_final_calc",
                                    "inverted_change_test"])
def test_actual_reset_restoration_and_callback_rows(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = actual_body()
    before = body
    if damage == "reverse_allocations":
        body = body.replace(
            "PlayersData[factionID_].se_alloc_psych = static_cast<int>(savedAlloc_[1]);",
            "savedAlloc_[1] = static_cast<uint32_t>(PlayersData[factionID_].se_alloc_psych);", 1)
        body = body.replace(
            "PlayersData[factionID_].se_alloc_labs = static_cast<int>(savedAlloc_[2]);",
            "savedAlloc_[2] = static_cast<uint32_t>(PlayersData[factionID_].se_alloc_labs);", 1)
    elif damage == "stale_category":
        marker = "    for (int category = 0; category < 4; ++category) {"
        body = body.replace(marker, "    const SocialCategory &cached = PlayersData[factionID_].soc_category_pending;\n" + marker, 1)
        body = body.replace("PlayersData[factionID_].soc_category_pending.models_[category]",
                            "cached.models_[category]", 1)
    elif damage == "stale_second_calc":
        body = body.replace("        social_set(factionID_);",
                            "        PlayerData &cached = PlayersData[factionID_];\n        social_set(factionID_);", 1)
        body = body.replace("&PlayersData[factionID_].soc_effect_pending",
                            "&cached.soc_effect_pending", 1)
    elif damage == "stale_final_calc":
        body = body.replace("    energy_compute(factionID_, 1);",
                            "    PlayerData &cached_final = PlayersData[factionID_];\n    energy_compute(factionID_, 1);", 1)
        body = body.replace("&PlayersData[factionID_].soc_category_pending",
                            "&cached_final.soc_category_pending", 1)
        at = body.index("&cached_final.soc_category_pending")
        body = body[:at] + body[at:].replace("&PlayersData[factionID_].soc_effect_pending",
                                           "&cached_final.soc_effect_pending", 1)
    elif damage == "inverted_change_test":
        body = body.replace("if (allocations_changed)", "if (!allocations_changed)", 1)
    assert damage is None or body != before
    source = tmp_path / "socialwin-reset.cpp"
    binary = tmp_path / "socialwin-reset"
    source.write_text(FIXTURE.replace("ACTUAL_BODY", body))
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == (damage is None), ran.stdout + ran.stderr
