"""Actual source convenience bodies; host controls are not native ABI proof."""
from pathlib import Path
import shutil
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]

def projected(damage=None):
    bodies = []
    damaged = False
    for address, relative, signature in (
        (0x47A6F0, 'src/spot.cpp', 'int Spot::add(int position, int type, RECT *rect)'),
        (0x47A720, 'src/buffer.cpp', 'int Buffer::write_cent_l(LPSTR text, RECT *rect)')):
        row = next(r for r in read_file(ROOT / relative) if r.address == address)
        body = _definition(row.region, signature)
        if damage is not None and damage[0] in body:
            old, new = damage
            body = body.replace(old, new, 1)
            damaged = True
        bodies.append(body)
    fixture = Path(__file__).with_name('rectangle_owner_overloads_control.cpp').read_text()
    source = '\n'.join(bodies)
    if damage is not None:
        assert damaged
    return fixture.replace('// ACTUAL_SOURCE_BODIES', source)

@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_rectangle_owners(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new', [
    ('rect->right - rect->left', 'rect->right'),
    ('rect->bottom - rect->top', 'rect->bottom'),
    ('return 0;', 'return 1;'),
    ('static_cast<int>(strlen(text))', 'static_cast<int>(strlen(text)) + 1'),
])
def test_rectangle_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0
