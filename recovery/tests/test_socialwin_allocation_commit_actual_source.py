"""Actual allocation commit body, with callbacks that change the active row.

Host state controls supplement separate native layout/reference measurements.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    source = (ROOT / "src/socialwin.cpp").read_text()
    start = source.index("void SocialWin::on_iface_button_clicked(int button_id)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual allocation commit body")


FIXTURE = r'''
#include <cassert>
#include <cstdint>
struct SocialCategory { int value; };
struct SocialEffect { int value; };
struct PlayerData {
    SocialCategory soc_category_pending;
    SocialEffect soc_effect_pending;
    int se_alloc_psych,se_alloc_labs;
} PlayersData[10];
struct SocialWin;
SocialWin *active;
int BreakthroughTurnsGlobal,NetIncomeGlobal;
bool callbacks;
int expected_index,expected_delta,cap,limit_calls,allocation_calls,set_calls;
int calc_calls,energy_calls,redraw_calls,tutorial_calls;
uint32_t initial,clamped;
struct SocialWin {
    uint32_t factionID_,currentAlloc_[3],pendingAlloc_[3],field_2C5C_;
    SocialCategory socCategory_;
    SocialEffect socEffect_;
    int breakthroughTurns_,netIncome_;
    void on_iface_button_clicked(int);
    void allocate_energy(int index,int value) {
        assert(index==expected_index && allocation_calls++==0);
        assert(factionID_==static_cast<uint32_t>(callbacks?3:0));
        assert(value==static_cast<int>(callbacks?cap:(clamped>static_cast<uint32_t>(cap)?cap:clamped)));
        pendingAlloc_[0]=5;pendingAlloc_[1]=3;pendingAlloc_[2]=2;
        if(callbacks)factionID_=4;
    }
    void redraw() {
        assert(calc_calls==3 && energy_calls==1);++redraw_calls;
    }
};
struct Tutorial {
    void reset() {
        ++tutorial_calls;
        if(callbacks)active->factionID_=1;
    }
} TutorialWindow;
int energy_limit(int faction) {
    if(limit_calls++==0) {
        assert(faction==(callbacks?1:0));
        assert(active->currentAlloc_[expected_index]==clamped);
        if(callbacks) {
            active->currentAlloc_[expected_index]=static_cast<uint32_t>(cap+1);
            active->factionID_=2;
        }
    } else {
        assert(limit_calls==2 && faction==(callbacks?2:0));
        assert(active->field_2C5C_==1);
        if(callbacks)active->factionID_=3;
    }
    return cap;
}
void social_set(int faction) {
    assert(faction==(callbacks?4:0) && set_calls++==0);
    assert(PlayersData[faction].se_alloc_psych==3);
    assert(PlayersData[faction].se_alloc_labs==2);
    for(int i=0;i<3;++i)assert(active->currentAlloc_[i]==active->pendingAlloc_[i]);
    if(callbacks)active->factionID_=5;
}
void social_calc(SocialCategory *category,SocialEffect *effect,int faction,int a,int b) {
    assert(a==0 && b==0);
    if(calc_calls==0) {
        assert(faction==(callbacks?5:0));
        assert(category==&active->socCategory_ && effect==&active->socEffect_);
        if(callbacks)active->factionID_=6;
    } else if(calc_calls==1) {
        assert(faction==(callbacks?6:0) && category==&active->socCategory_);
        assert(effect==&PlayersData[faction].soc_effect_pending);
        if(callbacks)active->factionID_=7;
    } else {
        assert(calc_calls==2 && energy_calls==1 && faction==(callbacks?8:0));
        assert(category==&PlayersData[faction].soc_category_pending);
        assert(effect==&PlayersData[faction].soc_effect_pending);
        assert(active->breakthroughTurns_==42 && active->netIncome_==43);
    }
    ++calc_calls;
}
void energy_compute(int faction,int recompute) {
    assert(faction==(callbacks?7:0) && recompute==1 && energy_calls++==0);
    if(callbacks)active->factionID_=8;
    BreakthroughTurnsGlobal=42;NetIncomeGlobal=43;
}
ACTUAL_BODY
void run(int button,uint32_t value,int maximum,bool callback_changes) {
    callbacks=callback_changes;cap=maximum;initial=value;
    SocialWin window{};active=&window;
    expected_index=button<0x1f?0:(button<0x21?2:1);
    expected_delta=(button&1)?-1:1;
    window.currentAlloc_[0]=11;window.currentAlloc_[1]=12;window.currentAlloc_[2]=13;
    window.currentAlloc_[expected_index]=initial;
    clamped=initial+expected_delta;
    if(static_cast<int32_t>(clamped)<0)clamped=0;
    limit_calls=allocation_calls=set_calls=calc_calls=energy_calls=redraw_calls=tutorial_calls=0;
    for(int row=0;row<10;++row)PlayersData[row].se_alloc_psych=PlayersData[row].se_alloc_labs=-100;
    window.on_iface_button_clicked(button);
    assert(tutorial_calls==1 && allocation_calls==1 && set_calls==1);
    assert(calc_calls==3 && energy_calls==1 && redraw_calls==1);
    assert(limit_calls==((callbacks||clamped>static_cast<uint32_t>(cap))?2:1));
    assert(window.field_2C5C_==static_cast<uint32_t>(limit_calls==2));
    for(int row=0;row<10;++row)if(row!=(callbacks?4:0)) {
        assert(PlayersData[row].se_alloc_psych==-100 && PlayersData[row].se_alloc_labs==-100);
    }
}
int main() {
    for(int button=0x1d;button<=0x22;++button) {
        run(button,4,6,true);
        run(button,4,6,false);
        run(button,10,6,false);
    }
    run(0x1d,0,6,false);
    run(0x1e,0x7fffffffu,6,false);
    run(0x1e,0xffffffffu,6,false);
    SocialWin window{};active=&window;callbacks=false;
    int prior=tutorial_calls;
    window.on_iface_button_clicked(0x23);
    assert(tutorial_calls==prior+1);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "limit_before_change", "reverse_copy",
                                    "stale_second_calc", "stale_final_calc"])
def test_actual_allocation_commit_callback_rows(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = actual_body()
    before = body
    if damage == "limit_before_change":
        body = body.replace("    const int limit = energy_limit(factionID_);\n", "", 1)
        body = body.replace("    currentAlloc_[index] += delta;",
                            "    const int limit = energy_limit(factionID_);\n    currentAlloc_[index] += delta;", 1)
    elif damage == "reverse_copy":
        body = body.replace("currentAlloc_[allocation] = pendingAlloc_[allocation]",
                            "pendingAlloc_[allocation] = currentAlloc_[allocation]", 1)
    elif damage == "stale_second_calc":
        body = body.replace("    social_set(factionID_);",
                            "    PlayerData &cached = PlayersData[factionID_];\n    social_set(factionID_);", 1)
        body = body.replace("&PlayersData[factionID_].soc_effect_pending",
                            "&cached.soc_effect_pending", 1)
    elif damage == "stale_final_calc":
        body = body.replace("    energy_compute(factionID_, 1);",
                            "    PlayerData &cached = PlayersData[factionID_];\n    energy_compute(factionID_, 1);", 1)
        body = body.replace("&PlayersData[factionID_].soc_category_pending",
                            "&cached.soc_category_pending", 1)
        at = body.index("&cached.soc_category_pending")
        body = body[:at] + body[at:].replace("&PlayersData[factionID_].soc_effect_pending",
                                           "&cached.soc_effect_pending", 1)
    assert damage is None or body != before
    source = tmp_path / "socialwin-commit.cpp"
    binary = tmp_path / "socialwin-commit"
    source.write_text(FIXTURE.replace("ACTUAL_BODY", body))
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == (damage is None), ran.stdout + ran.stderr
