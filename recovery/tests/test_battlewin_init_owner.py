"""Bounded actual-owner geometry/lifetime controls, not full-game acceptance."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import Mode, State, read_file

ROOT = Path(__file__).resolve().parents[2]


def owner():
    path = ROOT / 'src/battlewin.cpp'
    record, = [r for r in read_file(path) if r.address == 0x4219E0]
    assert record.name == '?init@BattleWin@@QAEXXZ'
    assert record.symbol == '?init@BattleWin@@QAEXXZ'
    assert record.image_spans == ((0x4219E0, 0x421B1F),)
    assert record.mode == Mode.BODY and record.state == State.IMPLEMENTED
    text = path.read_text()
    start = text.index('void BattleWin::init() {')
    end = text.index('\n}', start) + 2
    return text[start:end]


def run(tmp_path, body):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host compiler unavailable')
    fixture = Path(__file__).with_name('battlewin_init_owner_control.cpp').read_text()
    assert fixture.count('/* ACTUAL OWNER BODY */') == 1
    source = tmp_path / 'owner.cpp'
    source.write_text(fixture.replace('/* ACTUAL OWNER BODY */', body))
    binary = tmp_path / 'owner'
    build = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra', '-Werror',
                            str(source), '-o', str(binary)], capture_output=True, text=True)
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_owner_geometry_and_temporary_lifetime(tmp_path):
    result = run(tmp_path, owner())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize('old,new', [
    ('Buffer scratch;', ';'),
    ('field_70_ = frame;', 'field_70_ = layout;'),
    ('field_4_ = 10;', 'field_4_ = 9;'),
    ('right2 + 3;', 'right2 + 4;'),
    ('InterfaceRegistry[10] = this;', 'InterfaceRegistry[10] = 0;'),
    ('InterfaceRegistry[10] = this;', 'InterfaceRegistry[9] = this;'),
    ('InterfaceRegistry[10] = this;', 'InterfaceRegistry[10] = this; InterfaceRegistry[11] = this;'),
])
def test_controls_reject_lifetime_geometry_and_receiver_mutants(tmp_path, old, new):
    body = owner()
    assert body.count(old) == 1
    assert run(tmp_path, body.replace(old, new)).returncode != 0


def test_real_source_receiver_and_typed_rectangles():
    implementation = (ROOT / 'src/battlewin.cpp').read_text()
    assert 'BattleWinInitReceiver' not in implementation
    assert 'InterfaceRegistry[10] = this;' in owner()
    source = (ROOT / 'src/game.cpp').read_text()
    assert 'g_BattleWin.init();' in source
    assert 'battlewin_init(static_cast<BattleWin *>(sub_interface_global()))' not in source
    header = (ROOT / 'src/battlewin.h').read_text()
    assert 'RECT field_70_;' in header and 'RECT field_90_;' in header
