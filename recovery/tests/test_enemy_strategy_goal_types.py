"""Actual-body controls for the native Goal.type 8 and 9 predicates."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

from test_enemy_strategy_candidate import BODY, STUBS, extract_body


def case_source(goal_type: int, plan: int, include_closer: bool,
                damage_cache: bool=False) -> str:
    body=extract_body(BODY.read_text())
    if damage_cache:
        old="if (goal.type == AI_GOAL_COLONIZE) {"
        assert body.count(old)==1
        body=body.replace(old,"if (goal.type == AI_GOAL_LANDING_SITE) {",1)
    stubs=STUBS.replace(
        "static inline int world_site(int,int,int) { return 0; }",
        "static int world_site_calls=0,site_set_calls=0;\n"
        "static inline int world_site(int,int,int) { ++world_site_calls; return 1; }",
    ).replace(
        "static inline void site_set(int,int,int) {}",
        "static inline void site_set(int,int,int) { ++site_set_calls; }",
    ).replace("static const uint32_t BIT_BASE_RADIUS=0x10;\n","")
    other = r'''
    VehCurrentCount=2;
    Vehs[1].faction_id=1; Vehs[1].proto_id=8;
    Vehs[1].x=4; Vehs[1].y=3; Vehs[1].order=4; // not eligible for assignment
    Vehs[1].home_base_id=-1;
    VehPrototypes[8].plan=9; VehPrototypes[8].chassis_id=0;
    '''
    if not include_closer:
        other="VehCurrentCount=1;"
    return stubs+"\n"+body+f'''
int main() {{
    TurnCurrentNum=40; BaseCurrentCount=0; LockEnableMask=0;
    for(int i=0;i<8;i++) {{
        PlayersData[i].base_id_atk_target=-1;
        PlayersData[i].x_target=-1; PlayersData[i].y_target=-1;
    }}
    VehCurrentCount=1;
    Vehs[0].faction_id=1; Vehs[0].proto_id=7;
    Vehs[0].x=2; Vehs[0].y=3; Vehs[0].order=0;
    Vehs[0].home_base_id=-1;
    VehPrototypes[7].plan={plan}; VehPrototypes[7].chassis_id=0;
    Chassis[0].triad=TRIAD_LAND;
    MapTiles[(2>>1)+3*16].region=5;
    MapTiles[(2>>1)+3*16].climate=0x60;
    PlayersData[1].goals[0].type={goal_type};
    PlayersData[1].goals[0].priority=3;
    PlayersData[1].goals[0].x=14; PlayersData[1].goals[0].y=3;
    MapTiles[(14>>1)+3*16].region=5;
    MapTiles[(14>>1)+3*16].climate=0x60;
    {other}
    enemy_strategy(1);
    if ({goal_type} == AI_GOAL_COLONIZE) {{
        if (world_site_calls != 1 || site_set_calls != 1) return 1;
    }} else if (world_site_calls != 0 || site_set_calls != 0) return 2;
    if ({goal_type} == AI_GOAL_LANDING_SITE && Vehs[0].order != 0x18) return 3;
    if ({goal_type} == AI_GOAL_TERRAFORM_LAND && Vehs[0].order == 0x18) return 4;
    if ({goal_type} == AI_GOAL_TERRAFORM_LAND && {plan} != 9
        && Vehs[0].order == 0x18) return 5;
    return 0;
}}
'''


def run(compiler: str, goal_type: int, plan: int,
        include_closer: bool, damage_cache: bool=False) -> int:
    with tempfile.TemporaryDirectory(prefix="enemy-strategy-goal-type-") as temp:
        src=Path(temp)/"candidate.cpp"; exe=Path(temp)/"candidate"
        src.write_text(case_source(goal_type,plan,include_closer,damage_cache))
        subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror",
                        "-fsanitize=undefined","-fno-sanitize-recover=all",
                        str(src),"-o",str(exe)],check=True,timeout=60)
        return subprocess.run([str(exe)],timeout=30).returncode


def test_goal_type_discriminators_gcc() -> None:
    compiler=shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    assert run(compiler,8,8,False)==0
    assert run(compiler,41,9,True)==0
    assert run(compiler,9,9,True)==0
    assert run(compiler,9,8,False)==0
    assert run(compiler,8,8,False,True)!=0


def test_goal_type_discriminators_clang() -> None:
    compiler=shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    assert run(compiler,8,8,False)==0
    assert run(compiler,41,9,True)==0
    assert run(compiler,9,9,True)==0
    assert run(compiler,9,8,False)==0
    assert run(compiler,8,8,False,True)!=0

