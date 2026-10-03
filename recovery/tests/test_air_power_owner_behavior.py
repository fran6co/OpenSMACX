"""Actual-source air_power composition controls, not native ABI/game proof."""
from pathlib import Path
import shutil,subprocess
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT=Path(__file__).resolve().parents[2]
ANCHOR='// Air-unit movement, targeting and missile continuation helpers.\n'

def actual_owner():
    path=ROOT/'src/veh.cpp'; text=path.read_text()
    row=next(r for r in read_file(path) if r.address==0x5678c0)
    assert row.image_spans==((0x5678c0,0x56a89e),)
    assert row.region and 'BODY GOES HERE.' not in row.region
    body=_definition(row.region,'int __cdecl air_power(')
    start=text.index(ANCHOR); end=text.index(body,start)+len(body)
    result=text[start:end]
    assert '#include "air_power_' not in result
    return result

def control(tmp_path,compiler,owner):
    fixture=(Path(__file__).parent/'air_power_owner_control.cpp').read_text()
    assert fixture.count('// ACTUAL_SOURCE_OWNER')==1
    source=tmp_path/'control.cpp';source.write_text(fixture.replace('// ACTUAL_SOURCE_OWNER',owner))
    binary=tmp_path/'control'
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror',str(source),'-o',str(binary)],check=True,capture_output=True)
    return subprocess.run([str(binary)],capture_output=True)

@pytest.mark.parametrize('name',['g++','clang++'])
def test_actual_full_air_owner(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler: pytest.skip(name+' unavailable')
    result=control(tmp_path,compiler,actual_owner())
    assert result.returncode==0,result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new',[
    ('if (cost <= best_cost)','if (cost < best_cost)'),
    ('|| (PlayersData[faction_id].flags & 0x200u)','|| (PlayersData[faction_id].flags & 2u)'),
    ('live_iVar11 = speed(vehicle_id, 0) + live_iVar10;','live_iVar11 = live_iVar10;'),
    ('(LockEnableMask & faction_bit) != 0 && IsMultiplayerNet','(LockEnableMask & faction_bit) != 0 || IsMultiplayerNet'),
    ('a1, a4, a5, a3 != 0,','a1, a4, a5, a2 != 0,'),
    ('best_distance <= live_iVar11 / 2','best_distance < live_iVar11 / 2'),
    ('distance < best_distance)','distance <= best_distance)'),
])
def test_owner_damage_detected(tmp_path,old,new):
    compiler=shutil.which('g++')
    if not compiler: pytest.skip('g++ unavailable')
    owner=actual_owner();assert old in owner
    result=control(tmp_path,compiler,owner.replace(old,new,1))
    assert result.returncode<0,'damaged source must fail at runtime'
