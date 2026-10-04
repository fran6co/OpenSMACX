"""Actual owner-body controls; host dispatch does not prove native ABI/runtime."""
from pathlib import Path
import shutil
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]
SUBJECTS = ((0x5F1230, 'LRESULT', 'window_pos_changing'),
            (0x5F1280, 'LRESULT', 'window_pos_changed'),
            (0x5F12E0, 'void', 'window_sized'),
            (0x5F1310, 'void', 'window_nonclient_sized'))

def projected(damage=None):
    rows = read_file(ROOT / 'src/win.cpp')
    bodies = '\n\n'.join(_definition(next(r.region for r in rows if r.address == address),
                         result + ' __cdecl win_' + name + '(')
                         for address, result, name in SUBJECTS)
    if damage is not None:
        old, new = damage
        assert old in bodies
        bodies = bodies.replace(old, new, 1)
    fixture = Path(__file__).with_name('win_adapter_owner_control.cpp').read_text()
    assert fixture.count('// ACTUAL_SOURCE_BODIES') == 1
    return fixture.replace('// ACTUAL_SOURCE_BODIES', bodies)

@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_owner_adapters(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new', [
    ('HandleMain == hwnd', 'HandleMain != hwnd'),
    ('hwnd, 0x46, 0, parameter', 'hwnd, 0x47, 0, parameter'),
    ('window_data = owner->on_window_pos_changed', 'window_data = 1 + owner->on_window_pos_changed'),
    ('owner->on_size(flags, width, height)', 'owner->on_size(flags, height, width)'),
    ('owner->on_size_nc(flags, width, height)', 'owner->on_size(flags, width, height)'),
])
def test_adapter_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, 'damaged body must fail a behavioral witness'
