"""Actual MCI owner-body witnesses; host tests do not prove native ABI/runtime."""
from pathlib import Path
import shutil
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]

def projected(damage=None):
    rows = read_file(ROOT / 'src/alphamovie.cpp')
    bodies = '\n\n'.join(_definition(next(r.region for r in rows if r.address == address),
                                    'LRESULT __cdecl ' + name + '(')
                         for address, name in ((0x5FFE40, 'sub_5ffe40'), (0x5FFE80, 'sub_5ffe80')))
    if damage is not None:
        old, new = damage
        assert old in bodies
        bodies = bodies.replace(old, new, 1)
    fixture = Path(__file__).with_name('mci_message_owner_control.cpp').read_text()
    assert fixture.count('// ACTUAL_SOURCE_BODIES') == 1
    return fixture.replace('// ACTUAL_SOURCE_BODIES', bodies)

@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_mci_owner(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new', [
    ('forward_event != 0', 'forward_event == 0'),
    ('WM_KEYDOWN, virtual_key', 'WM_LBUTTONDOWN, virtual_key'),
    ('static_cast<uint16_t>(low_payload)', 'static_cast<uint16_t>(low_payload + 1)'),
    ('static_cast<uint16_t>(y)', 'static_cast<uint16_t>(y + 1)'),
    ('double_click != 0', 'double_click > 0'),
    ('WM_LBUTTONDBLCLK - WM_LBUTTONDOWN', 'WM_LBUTTONDBLCLK - WM_LBUTTONDOWN + 1'),
    ('message, keys,', 'message, keys + 1,'),
    ('DefWindowProcA(MciVideoCallbackWindow, WM_KEYDOWN', 'DefWindowProcA(0, WM_KEYDOWN'),
    ('return DefWindowProcA(MciVideoCallbackWindow, message, keys,\n                          static_cast<LPARAM>(packed));', 'DefWindowProcA(MciVideoCallbackWindow, message, keys,\n                          static_cast<LPARAM>(packed)); return 0;'),
])
def test_mci_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, 'damaged actual source must fail its runtime witness'
