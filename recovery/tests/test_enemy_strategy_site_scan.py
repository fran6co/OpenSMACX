"""Focused actual-body control for enemy_strategy's nested site scan.

Host structs are doubles only; this checks the translated native loop behavior,
not target ABI, map generation, or full-game runtime.
"""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

from test_enemy_strategy_candidate import BODY, STUBS, extract_body


def scanner_source(damage: bool = False) -> str:
    stubs = STUBS.replace(
        "static inline int abstract_at(int x,int y) { return map_loc(x,y)->region; }",
        "static int AbstractRegions[5][7]={{0}};\n"
        "static inline int abstract_at(int x,int y) { return AbstractRegions[y][x]; }",
    ).replace(
        "static const uint32_t BIT_BASE_RADIUS=0x10;\n", ""
    ).replace(
        "static inline void add_goal(int,int,int,int,int,int) { ++goal_calls; }",
        "static int last_goal_type=-1,last_goal_x=-1,last_goal_y=-1;\n"
        "static inline void add_goal(int,int type,int,int x,int y,int) { "
        "++goal_calls; last_goal_type=type; last_goal_x=x; last_goal_y=y; }",
    )
    body = extract_body(BODY.read_text())
    if damage:
        old = "int next_x =\n                                    ray_x + VehMoveDeltaX[outer_direction];"
        new = "int next_x =\n                                    ray_x + VehMoveDeltaX[inner_direction];"
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    return stubs + "\n" + body + r'''
int main() {
    TurnCurrentNum=0;
    LockEnableMask=0;
    BaseCurrentCount=1;
    for (int i=0;i<8;i++) PlayersData[i].base_id_atk_target=-1;
    Bases[0].x=2; Bases[0].y=2; Bases[0].faction_id_current=1;
    Bases[0].population_size=1;
    Map *base_tile=map_loc(2,2);
    base_tile->region=5; base_tile->climate=0x60;
    PlayersData[1].energy_reserves=8000;
    Continents[5].tile_count=100;
    AbstractRegions[2][2]=5;

    // An off-diagonal inner start has a northward land ray ending at y=7;
    // the old single-direction walk sees only disallowed diagonal edges.
    // Every other eligible ray ends on tiles marked as disallowed edges.
    Map *start=map_loc(9,11);
    start->climate=0x60; start->bit=1; start->val2=0;
    Map *middle=map_loc(9,10);
    middle->climate=0x60; middle->bit=1; middle->val2=0;
    Map *inner_start=map_loc(9,9);
    inner_start->climate=0x60; inner_start->bit=1; inner_start->val2=0;
    Map *middle2=map_loc(9,8);
    middle2->climate=0x60; middle2->bit=1; middle2->val2=0;
    Map *edge=map_loc(9,7);
    edge->climate=0x60; edge->bit=0; edge->val2=0;
    enemy_strategy(1);
    if (last_goal_type != AI_GOAL_LANDING_SITE) return 1;
    if (last_goal_x != 9 || last_goal_y != 7) return 2;
    return 0;
}
'''


def run(compiler: str, damage: bool = False) -> int:
    with tempfile.TemporaryDirectory(prefix="enemy-strategy-site-scan-") as temp:
        source=Path(temp)/"candidate.cpp"
        binary=Path(temp)/"candidate"
        source.write_text(scanner_source(damage))
        subprocess.run(
            [compiler,"-std=c++98","-Wall","-Wextra","-Werror",
             "-fsanitize=undefined","-fno-sanitize-recover=all",
             str(source),"-o",str(binary)],
            check=True,timeout=60,
        )
        return subprocess.run([str(binary)],timeout=30).returncode


def test_nested_site_scan_gcc() -> None:
    compiler=shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    assert run(compiler) == 0
    assert run(compiler, damage=True) != 0


def test_nested_site_scan_clang() -> None:
    compiler=shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    assert run(compiler) == 0
    assert run(compiler, damage=True) != 0


