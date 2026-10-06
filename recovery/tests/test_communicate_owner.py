"""Actual-source communicate bounded controls; not native/game acceptance.

Product body, true SocialEffect declaration and arithmetic helpers come only
from current source. Services remain explicit doubles. One entry-only injection
permits bounded pact/loan/cleanup paths; removing it restores the body exactly.
Unsafe out-of-domain effect selectors/uninitialized paths are not certified.
"""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / 'src/faction.cpp').exists())
FIXTURE = ROOT / 'recovery/tests/communicate_owner_host_fixture.cpp'
HELPERS = ('communicate_i32_from_bits', 'communicate_wrapped_add', 'communicate_wrapped_sub',
           'communicate_wrapped_mul', 'communicate_pending_social_model',
           'communicate_sar_i32', 'communicate_imul_high_sar')

def braced(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    # Mask comments/literals to avoid deriving extent from braces in dialogue.
    masked = re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                    lambda m: ' ' * len(m.group()), source, flags=re.S)
    depth = 0
    for end in range(opening, len(source)):
        depth += (masked[end] == '{') - (masked[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual owner definition: ' + signature)

def actual_parts():
    source = (ROOT / 'src/faction.cpp').read_text()
    row, = [row for row in read_file(ROOT / 'src/faction.cpp') if row.address == 0x54FFD0]
    assert row.name == '?communicate@@YAXHHH@Z'
    assert row.image_spans == ((0x54FFD0, 0x558980), (0x65FF4F, 0x65FF64))
    assert not row.extract_error and 'BODY GOES HERE.' not in row.region
    function = braced(row.region, 'void __cdecl communicate(')
    support = '\n'.join(braced(source, 'static int ' + name + '(') for name in HELPERS)
    effect = braced((ROOT / 'src/faction.h').read_text(), 'struct SocialEffect {') + ';'
    assert 'int values_[11];' in effect
    return function, support, effect

@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'indexed-effect', 'loan-count'])
def test_actual_complete_communicate(compiler, damage, tmp_path):
    command = shutil.which(compiler)
    if command is None:
        pytest.skip('host compiler unavailable')
    function, support, effect = actual_parts()
    if damage == 'indexed-effect':
        good = 'PlayersData[faction_id].soc_effect_pending.values_[effect_index]'
        assert function.count(good) == 1
        function = function.replace(good, 'PlayersData[faction_id].soc_effect_pending.values_[0]')
    if damage == 'loan-count':
        good = 'net_loan(peer_id, faction_id, communicate_stack_2c, communicate_stack_c);'
        assert function.count(good) == 1
        function = function.replace(good, 'net_loan(peer_id, faction_id, communicate_stack_2c, 0);')
    fixture = FIXTURE.read_text()
    entry = fixture[fixture.index('// CONTROLLED_ENTRY_BEGIN'):fixture.index('// CONTROLLED_ENTRY_END')]
    anchor = 'communicate_stack_4c = 0; // native 0x5501A3'
    assert function.count(anchor) == 1
    instrumented = function.replace(anchor, anchor + '\n' + entry)
    assert instrumented.replace('\n' + entry, '', 1) == function
    fixture = fixture[:fixture.index('// CONTROLLED_ENTRY_BEGIN')] + fixture[fixture.index('// CONTROLLED_ENTRY_END') + len('// CONTROLLED_ENTRY_END'):]
    for marker, value in (('/*ACTUAL_COMMUNICATE_FUNCTION*/', instrumented),
                          ('/*ACTUAL_COMMUNICATE_SUPPORT*/', support),
                          ('/*ACTUAL_SOCIAL_EFFECT_OWNER*/', effect)):
        assert fixture.count(marker) == 1
        fixture = fixture.replace(marker, value)
    cpp, binary = tmp_path / 'actual.cpp', tmp_path / 'actual'
    cpp.write_text(fixture)
    built = subprocess.run([command, '-std=c++98', '-O1', '-w', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (run.returncode == 0) == (damage == 'none'), run.stdout + run.stderr
    if damage == 'none':
        assert 'communicate actual-body host fixtures: 20 passed' in run.stdout
