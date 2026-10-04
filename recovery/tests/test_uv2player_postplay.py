"""Actual-source movie text controls; fake APIs are not game runtime proof."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from recovery.tests.test_uv2player_directdraw import definition

ROOT = Path(__file__).resolve().parents[2]


def member():
    path = ROOT / 'src/uv2player.cpp'
    record, = (r for r in read_file(path) if r.address == 0x4BEF00)
    assert record.image_spans == ((0x4BEF00, 0x4BF380), (0x659D00, 0x659D55))
    return definition(path.read_text(), r'void UV2Player::show_post_movie_text\(\)')


def run(tmp_path, compiler, body):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip('host compiler unavailable')
    template = Path(__file__).with_name('uv2player_postplay_harness.cpp').read_text()
    assert template.count('// ACTUAL_MEMBER_GOES_HERE') == 1
    cpp, exe = tmp_path / 'postplay.cpp', tmp_path / 'postplay'
    cpp.write_text(template.replace('// ACTUAL_MEMBER_GOES_HERE', body))
    subprocess.run([executable, '-std=c++98', '-Wall', '-Wextra', '-Werror',
                    '-pedantic', '-fsanitize=address,undefined',
                    '-fno-sanitize-recover=all', str(cpp), '-o', str(exe)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0',
                                   UBSAN_OPTIONS='halt_on_error=1'))


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_postplay_member(tmp_path, compiler):
    result = run(tmp_path, compiler, member())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('old,new', [
    ('text_open("movlist", movie_name_) == 0', 'text_open("movlist", movie_name_) != 0'),
    ('StringAllocationBase allocator_scope;', ''),
    ('font_heights[index],\n                              font_styles[index]',
     'font_styles[index],\n                              font_heights[index]'),
    ('text_item_number() + 0xA', 'text_item_number()'),
    ('top + (bottom - drawn_height - top) / 2', 'top + (bottom - drawn_height - top) / 2 + 1'),
    ('help_lines.load(DatalinkHelpFile, help_line + 1, 0, 0);', 'help_lines.add_special(help_line);'),
    ('buffer_.set_write_raw_callback(0);', ''),
    ('DWORD now = timeGetTime();', 'DWORD now = start + static_cast<DWORD>(timeout);'),
])
def test_damage_controls(tmp_path, compiler, old, new):
    body = member()
    assert old in body
    result = run(tmp_path, compiler, body.replace(old, new, 1))
    assert result.returncode != 0, 'deliberate damage escaped actual-source control'
