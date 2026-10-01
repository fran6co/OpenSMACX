"""Actual-source behavior controls, not native matching/game acceptance."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
HARNESS = Path(__file__).with_name('interface_registry_wave_harness.cpp')


def bodies():
    result = []
    for source, address, signature in (
        ('src/reportif.cpp', 0x45D2E0, 'void SubInterface::delete_iface_mode()'),
        ('src/reportif.cpp', 0x45D310, 'void SubInterface::set_iface_mode()'),
        ('src/reportif.cpp', 0x45D380, 'void SubInterface::release_iface_mode()'),
        ('src/multiwin.cpp', 0x479900, 'void MultiWin::UNK2()')):
        records = [r for r in read_file(ROOT / source) if r.address == address]
        assert len(records) == 1
        text = (ROOT / source).read_text()
        matches = list(re.finditer(re.escape(signature) + r'\s*\{', text))
        assert len(matches) == 1
        start, opening = matches[0].start(), matches[0].end() - 1
        depth = 0
        for end in range(opening, len(text)):
            depth += (text[end] == '{') - (text[end] == '}')
            if depth == 0:
                body = text[start:end + 1]
                assert 'ORIGINAL(' not in body and 'original_method' not in body
                result.append(body)
                break
        else:
            raise AssertionError('Unclosed actual product body')
    return result


def run_host(tmp_path, damage=None):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('Host C++ compiler required')
    program = HARNESS.read_text()
    token = '// ACTUAL_PRODUCT_BODIES'
    assert program.count(token) == 1
    program = program.replace(token, '\n\n'.join(bodies()))
    if damage:
        before, after = damage
        assert program.count(before) == 1, before
        program = program.replace(before, after, 1)
    source, binary = tmp_path / 'registry-wave.cpp', tmp_path / 'registry-wave'
    source.write_text(program)
    built = subprocess.run(
        [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
         '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
         str(source), '-o', str(binary)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_four_owner_bodies(tmp_path):
    result = run_host(tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('damage', [
    ('mode != 10 || current == 0', 'mode != 10 || current != 0'),
    ('--depth;\n                    ActiveIfaceIdStorage = depth;\n                }\n            }',
     '--depth;\n                    ActiveIfaceIdStorage = depth;\n                    break;\n                }\n            }'),
    ('total += sprite4_.height() + footer_height + 0x0E;',
     'total += footer_height + 0x0E;'),
    ('StatusFlags |= 0x40;', 'StatusFlags &= ~0x40u;'),
    ('const int footer_height = sprite7_.height();\n    const int header_height = sprite3_.height();',
     'const int header_height = sprite3_.height();\n    const int footer_height = sprite7_.height();'),
])
def test_actual_source_damage_is_detected(tmp_path, damage):
    result = run_host(tmp_path, damage)
    assert result.returncode != 0, 'Source damage escaped actual-body controls'
