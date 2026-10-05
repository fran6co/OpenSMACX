"""Full-body control that the selected-region search scores faction 2 for faction 1."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

from test_enemy_strategy_candidate import BODY, STUBS, extract_body


def source(damage: bool=False) -> str:
    body=extract_body(BODY.read_text())
    if damage:
        old="for (int other = 1; other < MaxPlayerNum; ++other) {\n                if (other == faction_id) continue;"
        assert body.count(old)==1
        body=body.replace(old,"for (int other = 1; other < MaxPlayerNum; ++other) {\n                if (faction_id == 1) continue;",1)
    stubs=STUBS.replace(
        "static inline int contiguous(int,int,int,int) { return 0; }",
        "static inline int contiguous(int,int,int,int mode) { return mode == 0; }",
    ).replace(
        "static inline int base_find(int,int,int) { return -1; }",
        "static inline int base_find(int,int,int) { ++base_find_calls; BaseFindDist=5; return -1; }",
    ).replace(
        "static inline int base_find(int,int,int,int,int,int) { return -1; }",
        "static inline int base_find(int,int,int,int,int,int) { ++base_find_calls; return -1; }",
    )
    stubs=stubs.replace("int goal_calls=0,wipe_calls=0,reset_calls=0;",
                        "int goal_calls=0,wipe_calls=0,reset_calls=0,base_find_calls=0;")
    stubs=stubs.replace("static const uint32_t BIT_BASE_RADIUS=0x10;\n","")
    return stubs+"\n"+body+r'''
int main() {
    TurnCurrentNum=2; BaseCurrentCount=1; VehCurrentCount=1;
    LockEnableMask=0;
    for(int i=0;i<8;i++) {
        PlayersData[i].base_id_atk_target=-1;
        PlayersData[i].x_target=-1; PlayersData[i].y_target=-1;
    }
    Bases[0].x=2; Bases[0].y=3; Bases[0].faction_id_current=1;
    Bases[0].population_size=1;
    PlayersData[2].region_total_bases[5]=1;
    PlayersData[2].region_total_combat_vehs[5]=1;
    PlayersData[2].region_force_rating[5]=1;
    PlayersData[1].diplo_treaties[2]=0x0C18;
    Vehs[0].faction_id=1; Vehs[0].proto_id=7;
    Vehs[0].x=4; Vehs[0].y=3; Vehs[0].home_base_id=-1; Vehs[0].order=0;
    VehPrototypes[7].plan=1; VehPrototypes[7].chassis_id=0;
    Chassis[0].triad=TRIAD_LAND;
    MapTiles[(2>>1)+3*16].region=5; MapTiles[(2>>1)+3*16].climate=0x60;
    MapTiles[(4>>1)+3*16].region=5; MapTiles[(4>>1)+3*16].climate=0x60;
    Continents[5].tile_count=100;
    enemy_strategy(1);
    return base_find_calls > 0 ? 0 : 1;
}
'''


def run(compiler: str, damage: bool=False) -> int:
    with tempfile.TemporaryDirectory(prefix="enemy-strategy-faction1-region-") as tmp:
        src=Path(tmp)/"candidate.cpp"; exe=Path(tmp)/"candidate"
        src.write_text(source(damage))
        subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror",
                        "-fsanitize=undefined","-fno-sanitize-recover=all",
                        str(src),"-o",str(exe)],check=True,timeout=60)
        return subprocess.run([str(exe)],timeout=30).returncode


def test_faction_one_opponent_scan_gcc() -> None:
    compiler=shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    assert run(compiler)==0
    assert run(compiler,True)!=0


def test_faction_one_opponent_scan_clang() -> None:
    compiler=shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    assert run(compiler)==0
    assert run(compiler,True)!=0

