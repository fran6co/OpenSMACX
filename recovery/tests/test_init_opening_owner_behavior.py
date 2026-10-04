"""Actual source controls; test doubles are not native ABI/game proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]


def projected(damage=None):
    row = next(r for r in read_file(ROOT / 'src/game.cpp') if r.address == 0x589B60)
    assert row.image_spans == ((0x589B60, 0x589D29), (0x66026A, 0x66028A))
    body = _definition(row.region, 'int __cdecl init_opening(')
    if damage is not None:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    header = (ROOT / 'src/buffer.h').read_text()
    getters = []
    for name in ('pixel_width', 'pixel_height'):
        matches = re.findall(r'int ' + name + r'\(\) const \{[^\n]*\}', header)
        assert len(matches) == 1
        getters.append(matches[0])
    fixture = Path(__file__).with_name('init_opening_owner_control.cpp').read_text()
    return fixture.replace('// ACTUAL_BUFFER_GETTERS', '\n'.join(getters)).replace('// ACTUAL_SOURCE_BODY', body)


def run(tmp_path, compiler, source):
    path = tmp_path / 'control.cpp'
    path.write_text(source)
    binary = tmp_path / 'control'
    command = [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
               '-fcheck-new', str(path), '-o', str(binary)]
    if 'clang' in compiler:
        command.insert(1, '-Wno-new-returns-null')
    subprocess.run(command, check=True, capture_output=True)
    return subprocess.run([str(binary)], capture_output=True)


@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_opening_owner(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')


@pytest.mark.parametrize('old,new', [
    ('return 4;', 'return 7;'),
    ('return result;', 'return 0;'),
    ('copy_window->pixel_height();', '-copy_window->pixel_height();'),
    ('local_buffer.pixel_height(), 0, 0', '-local_buffer.pixel_height(), 0, 0'),
    ('OpeningWindow->on_update();', 'copy_window->on_update();'),
    ('BasePopDefaultField322C = static_cast<Win *>(OpeningWindow);', 'BasePopDefaultField322C = &ConsoleGlobal;'),
    ('10, 0xEC', '10, 0'),
])
def test_opening_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, 'damaged live source must fail a runtime witness'


def test_opening_storage_is_source_owned():
    header = (ROOT / 'src/game.h').read_text()
    accessor = _definition(header, 'inline GraphicWin *&opening_window(')
    assert 'return OpeningWindow;' in accessor
    assert 'reinterpret_cast' not in accessor
    assert '0x00945824' not in accessor
    source = (ROOT / 'src/game.cpp').read_text()
    assert source.count('GraphicWin *OpeningWindow = 0;') == 1
