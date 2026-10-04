"""Actual-owner C++98 branch controls; no whole-function/runtime acceptance."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import read_file
from recovery.tests.test_pact_ascent import definition

ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/faction.cpp').is_file())
FIXTURE = Path(__file__).with_name('propose_pact_control.cpp').read_text()


def actual_source():
    source = (ROOT / 'src/faction.cpp').read_text()
    return '\n\n'.join(definition(source, signature) for signature in (
        r'static int propose_pact_signed_bits\([^)]*\)',
        r'static int propose_pact_rank\([^)]*\)',
        r'static bool propose_pact_ahead_refusal\([^)]*\)',
        r'int __cdecl propose_pact\([^)]*\)',
    ))


@pytest.mark.parametrize('name', ('g++', 'clang++'))
def test_actual_pact_branch_callback_and_damage_controls(name):
    compiler = shutil.which(name)
    if not compiler:
        pytest.skip(name + ' unavailable')
    body = actual_source()
    mutations = (
        ('DiploFriction <= 2', 'DiploFriction > 2'),
        ('propose_pact_rank(otherFac) <= propose_pact_rank(myFac)',
         'propose_pact_rank(otherFac) >= propose_pact_rank(myFac)'),
        ('int cap_numerator = diplo_add(energySnapshot,',
         'int cap_numerator = diplo_add(PlayersData[otherFac].energy_reserves,'),
        ('PlayersData[otherFac].diplo_treaties[myFac]',
         'PlayersData[otherFac].diplo_treaties[myFac + otherFac]'),
        ('score = eligible ? 1 : 0;', 'score = 1;'),
        ('int delta = diplo_sub(0, value / 2);', 'int delta = diplo_sub(energySnapshot, value) / 2;'),
        ('int remaining = cap / 5;', 'int remaining = cap / 10;'),
        ('int delta2 = diplo_sub(0, diplo_add(sum, 4) / 5);', 'int delta2 = cap / 20;'),
        ('if (myFac != 0) PlayersData[myFac].energy_reserves',
         'if (true) PlayersData[myFac].energy_reserves'),
        ('if (result == 0) {', 'if (result != 0) {'),
        ('PlayersData[otherFac].diplo_patience[myFac] =',
         'PlayersData[myFac].diplo_patience[myFac] ='),
        ('pact_of_brotherhood(myFac, otherFac);', 'pact_of_brotherhood(otherFac, myFac);'),
        ('double_cross(myFac, threshold, otherFac);', 'double_cross(otherFac, threshold, myFac);'),
        ('set_treaty(otherFac, threshold, 0x40, 1);', 'set_treaty(myFac, otherFac, 0x40, 1);'),
        ('message_data(0x2447, 0, myFac, diplo_sub(0, cap), otherFac, cap);',
         'message_data(0x2447, 0, diplo_sub(0, cap), otherFac, myFac, 0);'),
    )
    variants = [('actual', body, True)]
    for index, (before, after) in enumerate(mutations):
        assert before in body, before
        variants.append(('damage_' + str(index), body.replace(before, after), False))
    repeat = body.replace('while (cap > energySnapshot)', 'if (cap > energySnapshot)')
    repeat = repeat.replace('if (cap < 0) { cap = 0; break; }', 'if (cap < 0) cap = 0;')
    repeat = repeat.replace('if (cap == 0) break;', '')
    variants.append(('damage_repeat', repeat, False))
    with tempfile.TemporaryDirectory(prefix='osmx-pact-control-') as directory:
        temporary = Path(directory)
        for label, candidate, good in variants:
            cpp = temporary / (label + '.cpp')
            binary = temporary / label
            cpp.write_text(FIXTURE.replace('// @@PROPOSE_PACT_ACTUAL_SOURCE@@', candidate))
            compiled = subprocess.run([compiler, '-std=c++98', '-O1',
                                       '-fsanitize=undefined', '-fno-sanitize-recover=all',
                                       str(cpp), '-o', str(binary)],
                                      capture_output=True, text=True, timeout=45)
            assert compiled.returncode == 0, label + compiled.stdout + compiled.stderr
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            if good:
                assert result.returncode == 0, result.stdout + result.stderr
            else:
                assert result.returncode != 0, 'undetected mutation: ' + label
                assert 'Assertion' in result.stderr or 'assertion' in result.stderr, result.stderr


def test_pact_owner_identity_and_genuine_storage():
    records = [r for r in read_file(ROOT / 'src/faction.cpp') if r.address == 0x543bc0]
    assert len(records) == 1
    record = records[0]
    assert record.name == '?propose_pact@@YAHHH@Z'
    assert list(record.image_spans) == [(0x543bc0, 0x544e36)]
    assert not record.byte_exact and record.ruled_out
    assert not (ROOT / 'src/unrecovered/00543bc0.cpp').exists()
    body = actual_source()
    assert '0x0093' not in body and 'reinterpret_cast' not in body
    assert 'propose_pact_row_word' not in body
    assert 'diplo_treaties[myFac]' in body and 'diplo_friction[myFac]' in body
    assert '"REJPACTHATE"' in body and '"REJPACTNOHELP"' in body
    assert 'char *__cdecl get_pacts(int faction_id, int other_id);' in (ROOT / 'src/game.h').read_text()
