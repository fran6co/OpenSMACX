"""Actual-source sensing controls; not native ABI or game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_spot_all_and_damage(name):
    compiler = shutil.which(name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    product = (ROOT / 'src/veh.cpp').read_text()
    context = product.index('struct SpotAllSourceContext')
    function = product.index('void __cdecl spot_all(', context)
    owner = product[context:function] + definition(product[function:], re.escape('void __cdecl spot_all('))
    owner += '\nstatic void spot_all_effect_candidate(int id, int toggle) { spot_all(id, toggle); }\n'
    fixture = Path(__file__).with_name('spot_all_control.cpp').read_text()
    assert fixture.count('// @@ACTUAL_SPOT_ALL_OWNER@@') == 1
    source = fixture.replace('// @@ACTUAL_SPOT_ALL_OWNER@@', owner)
    mutations = (
        ('WorldClimateWorldWin.GraphicWin::redraw();', 'if (MapWinTable[0] != 0) MapWinTable[0]->on_redraw();'),
        ('// SOURCE_FLAG_MUTATION', '// SOURCE_FLAG_MUTATION'),
        ('(PlayersData[source.faction_id]\n                         .diplo_treaties[detected_faction] & 1u) == 0',
         '(PlayersData[source.faction_id]\n                         .diplo_treaties[detected_faction] & 7u) == 0'),
    )
    variants = [source]
    for old, new in mutations:
        if old == '// SOURCE_FLAG_MUTATION':
            helper = definition(owner, re.escape('static int may_mark_spot_all_map_dirty('))
            assert helper.count('PFLAG_MAP_REVEALED') == 1
            damaged = owner.replace(helper, helper.replace('PFLAG_MAP_REVEALED', 'PFLAG_UNK_80'), 1)
        else:
            assert owner.count(old) == 1, (old, owner.count(old))
            damaged = owner.replace(old, new, 1)
        variants.append(fixture.replace('// @@ACTUAL_SPOT_ALL_OWNER@@', damaged))
    with tempfile.TemporaryDirectory(prefix='osmx-spot-owner-') as directory:
        cpp, binary = Path(directory) / 'control.cpp', Path(directory) / 'control'
        for index, variant in enumerate(variants):
            cpp.write_text(variant)
            built = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra',
                                    '-Werror', '-pedantic', '-I', str(Path(__file__).parent),
                                    str(cpp), '-o', str(binary)], capture_output=True, text=True, timeout=45)
            assert built.returncode == 0, built.stdout + built.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (index == 0), (name, index, run.stdout, run.stderr)
