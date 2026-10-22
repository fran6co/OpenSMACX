"""Actual-source air_power entry controls, not native ABI/game proof."""
from pathlib import Path
import shutil,subprocess
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT=Path(__file__).resolve().parents[2]

def actual_owner():
    path=ROOT/'src/veh.cpp'; text=path.read_text()
    row=next(r for r in read_file(path) if r.address==0x5678c0)
    assert row.image_spans==((0x5678c0,0x56a89e),)
    assert row.region and 'BODY GOES HERE.' not in row.region
    body=_definition(row.region,'int __cdecl air_power(')
    assert text.count(body)==1
    assert body in row.region
    return body

def control(tmp_path,compiler,owner):
    fixture=(Path(__file__).parent/'air_power_owner_control.cpp').read_text()
    assert fixture.count('// ACTUAL_SOURCE_OWNER')==1
    source=tmp_path/'control.cpp';source.write_text(fixture.replace('// ACTUAL_SOURCE_OWNER',owner))
    binary=tmp_path/'control'
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror','-Wno-unused-but-set-variable',str(source),'-o',str(binary)],check=True,capture_output=True,timeout=45)
    return subprocess.run([str(binary)],capture_output=True,timeout=15)

@pytest.mark.parametrize('name',['g++','clang++'])
def test_actual_full_air_owner(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler: pytest.skip(name+' unavailable')
    result=control(tmp_path,compiler,actual_owner())
    assert result.returncode==0,result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new',[
    ('if (cost <= best_cost)','if (cost < best_cost)'),
    ('(PlayersData[faction_id].flags & 0x200u) != 0','(PlayersData[faction_id].flags & 2u) != 0'),
    ('live_iVar11 = speed(a1, 0) + live_iVar10;','live_iVar11 = live_iVar10;'),
    ('(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0 && IsMultiplayerNet',
     '(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0 || IsMultiplayerNet'),
    ('if ((live_cVar21 == 0 || live_cVar21 == 6)\n            && attacker.order != ORDER_MOVE_TO)',
     'if ((live_cVar21 == 0 || live_cVar21 == 6)\n            && attacker.order == ORDER_MOVE_TO)'),
    ('if (best_distance <= live_iVar11 / 2)','if (best_distance < live_iVar11 / 2)'),
    ('veh_drop(lifted, x, y);','veh_drop(lifted + 1, x, y);'),
])
def test_owner_damage_detected(tmp_path,old,new):
    compiler=shutil.which('g++')
    if not compiler: pytest.skip('g++ unavailable')
    owner=actual_owner();assert old in owner
    # The tie comparator occurs in two distinct original selection loops; mutate
    # both so the later-equal-target control exercises the full decision.
    changed=owner.replace(old,new) if old == 'if (cost <= best_cost)' else owner.replace(old,new,1)
    result=control(tmp_path,compiler,changed)
    assert result.returncode<0,'damaged source must fail at runtime'
