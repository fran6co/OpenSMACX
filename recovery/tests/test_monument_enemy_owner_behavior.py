"""Actual monument producer witnesses, not native ABI or game-runtime proof."""
from pathlib import Path
import shutil
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]

def projected(damage=None):
    row = next(r for r in read_file(ROOT / 'src/game.cpp') if r.address == 0x477210)
    body = _definition(row.region, 'void __cdecl mon_enemy_destroyed(')
    if damage is not None:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    header = (ROOT / 'src/game.h').read_text()
    types = []
    for name in ('MonumentRecord', 'MonumentFaction'):
        start = header.index('struct ' + name + ' {')
        end = header.index('};', start) + 2
        types.append(header[start:end])
    fixture = Path(__file__).with_name('monument_enemy_owner_control.cpp').read_text()
    return fixture.replace('// ACTUAL_SOURCE_TYPES', '\n'.join(types)).replace('// ACTUAL_SOURCE_BODY', body)

@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_monument_owner(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new', [
    ('rule_flags & RFLAG_ALIEN', 'rule_flags & 0x40u'),
    ('target_faction == 0', 'target_faction != 0'),
    ('record.achieved_ != 0 ||', 'record.achieved_ == 0 ||'),
    ('other < 8', 'other < 7'),
    ('other != faction_id &&', 'other == faction_id &&'),
    ('owner_faction = target_faction;', 'owner_faction = faction_id;'),
    ('other_record.word_14_ != 0', 'other_record.word_14_ == 0'),
    ('record.word_14_ = 1;', 'record.word_14_ = 2;'),
    ('record.year_ = year;', 'record.year_ = year + 1;'),
    ('record.word_8_ = target_faction;', 'record.word_8_ = faction_id;'),
    ('faction.latest_type_ = 6;', 'faction.latest_type_ = 5;'),
    ('faction_id == ConsoleGlobal.faction_id_', 'faction_id != ConsoleGlobal.faction_id_'),
])
def test_monument_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, 'damaged actual body must fail a runtime witness'
