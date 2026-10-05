"""Discriminating actual-body controls for native foreign plan-9 gating."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

from test_enemy_strategy_candidate import BODY, STUBS, extract_body


def make_source(triad: int, climate: int, bit: int, val2: int,
                lock_mask: int, visibility: int, move_marker: int,
                state: int) -> str:
    stubs = STUBS.replace(
        "static inline void add_goal(int,int,int,int,int,int) { ++goal_calls; }",
        "static int added_type=-1,added_priority=-1;\n"
        "static inline void add_goal(int,int type,int priority,int,int,int) { "
        "++goal_calls; added_type=type; added_priority=priority; }",
    )
    body = extract_body(BODY.read_text())
    return stubs + "\n" + body + f'''
int main() {{
    TurnCurrentNum=0; LockEnableMask={lock_mask};
    BaseCurrentCount=0; VehCurrentCount=1;
    for(int i=0;i<8;i++) {{
        PlayersData[i].base_id_atk_target=-1;
        PlayersData[i].x_target=-1; PlayersData[i].y_target=-1;
    }}
    PlayersData[1].diplo_treaties[2]=0x8;
    Vehs[0].faction_id=2; Vehs[0].proto_id=9;
    Vehs[0].x=2; Vehs[0].y=3; Vehs[0].visibility={visibility};
    Vehs[0].move_to_ai_type={move_marker}; Vehs[0].state={state};
    VehPrototypes[9].plan=9; VehPrototypes[9].chassis_id=0;
    Chassis[0].triad={triad};
    Map *tile=map_loc(2,3); tile->climate={climate};
    tile->bit={bit}; tile->val2={val2};
    enemy_strategy(1);
    return goal_calls == 1 && added_type == 6 ? 0 : 1;
}}
'''


def run(compiler: str, **case: int) -> int:
    with tempfile.TemporaryDirectory(prefix="enemy-strategy-foreign-plan9-") as temp:
        source = Path(temp) / "candidate.cpp"
        binary = Path(temp) / "candidate"
        source.write_text(make_source(**case))
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=undefined", "-fno-sanitize-recover=all",
             str(source), "-o", str(binary)], check=True, timeout=60
        )
        return subprocess.run([str(binary)], timeout=30).returncode


TRIAD_CASE = dict(triad=1, climate=0, bit=0, val2=0,
                  lock_mask=0, visibility=2, move_marker=0, state=0)
TILE_CASE = dict(triad=1, climate=0, bit=1, val2=7,
                 lock_mask=0, visibility=2, move_marker=0, state=0)
VISIBLE_CASE = dict(triad=1, climate=0, bit=0, val2=0,
                    lock_mask=0, visibility=2, move_marker=0, state=0)


def test_plan9_requires_sea_ocean_path_gcc() -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    assert run(compiler, **TRIAD_CASE) == 0
    assert run(compiler, **dict(TRIAD_CASE, triad=0)) != 0
    assert run(compiler, **dict(TRIAD_CASE, climate=0x60)) != 0


def test_plan9_requires_sea_ocean_path_clang() -> None:
    compiler = shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    assert run(compiler, **TRIAD_CASE) == 0
    assert run(compiler, **dict(TRIAD_CASE, triad=0)) != 0
    assert run(compiler, **dict(TRIAD_CASE, climate=0x60)) != 0


def test_plan9_base_owner_and_visibility_gates_gcc() -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    assert run(compiler, **TILE_CASE) != 0
    assert run(compiler, **dict(TILE_CASE, val2=8)) == 0
    assert run(compiler, **VISIBLE_CASE) == 0
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, lock_mask=2)) != 0
    # Actor move marker bypasses visibility unless both native state bits are set.
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, move_marker=1)) == 0
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, move_marker=1,
                               state=0x440)) != 0


def test_plan9_base_owner_and_visibility_gates_clang() -> None:
    compiler = shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    assert run(compiler, **TILE_CASE) != 0
    assert run(compiler, **dict(TILE_CASE, val2=8)) == 0
    assert run(compiler, **VISIBLE_CASE) == 0
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, lock_mask=2)) != 0
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, move_marker=1)) == 0
    assert run(compiler, **dict(VISIBLE_CASE, visibility=0, move_marker=1,
                               state=0x440)) != 0
