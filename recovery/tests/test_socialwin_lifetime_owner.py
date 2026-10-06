"""Coupled SocialWin lifetime observations, not complete game acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = '10a77ef7f0c4668460bc3e5704f116457f21f6e2'
SEED = 'src/recovered/units/004b3c80.cpp'


def test_destructor_retains_original_identity_and_seed_findings():
    old, = read_text(subprocess.check_output(['git', 'show', BASE + ':' + SEED], cwd=ROOT).decode(), ROOT / SEED)
    current = next(r for r in read_file(ROOT / 'src/socialwin.cpp') if r.address == 0x4B3C80)
    for field in ('address', 'name', 'image_spans', 'byte_exact', 'semantic',
                  'levers', 'ruled_out', 'deferred', 'unrecoverable'):
        assert getattr(current, field) == getattr(old, field), field
    assert current.symbol == '??1SocialWin@@UAE@XZ'
    assert (ROOT / 'src/socialwin.cpp').read_text().count('SocialWin::~SocialWin()') == 1
    assert not (ROOT / SEED).exists()


def test_native_socialwin_layout_and_single_base_construction():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from match_recovery.codeview import class_layout
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / 'src/socialwin.cpp', (), flags + settings.layout_flags))
        layout = class_layout(obj, 'SocialWin')
        assert layout['members']['energyLockButtons_'] == 0xD6C
        assert layout['members']['field_2BF0_'] == 0x2BF0
        assert 'field_2BEC_' not in layout['members']
        # Every later lifetime range must retain its native start, rather than
        # concealing the array fix with a uniform shift or raw teardown.
        expected = {'spot_': 0xA20, 'socEngModifierNums_': 0x2C60,
                    'effectIcons_': 0x2E18, 'energyAllocLock_': 0x3D90,
                    'energyAllocArrow_': 0x3DE8, 'energyAllocSlider_': 0x3EF0,
                    'flatButtons_': 0x6020, 'buttonGroups_': 0x1EB84,
                    'buttonGroup_': 0x1EDD4, 'font1_': 0x1EE68,
                    'font9_': 0x1EFA8}
        assert {name: layout['members'][name] for name in expected} == expected
        array = layout['member_types']['energyLockButtons_']
        assert array['kind'] == 'array' and array['count'] == 3 and array['size'] == 0x1E84
        assert array['target'] == {'kind': 'class_value', 'class': 'CheckButton'}
        assert [(b['class'], b['offset'], b['virtual']) for b in layout['bases']] == [
            ('GraphicWin', 0, False), ('SubInterface', 0xA14, False)]
        def references(symbol):
            function = obj.function(symbol)
            section = obj.sections[function.section - 1]
            # Array teardown passes destructor callbacks as DIR32 operands;
            # they are real dependencies, not direct REL32 call instructions.
            return [obj.symbols[target].name for _, target, kind in section.relocations]
        ctor = references('??0SocialWin@@QAE@XZ')
        assert ctor.count('??0GraphicWin@@QAE@XZ') == 1
        dtor = references('??1SocialWin@@UAE@XZ')
        assert dtor.count('??1GraphicWin@@UAE@XZ') == 1
        assert dtor.count('??1Font@@QAE@XZ') == 9
        assert '??1CheckButton@@UAE@XZ' in dtor
        assert '?close@ButtonGroup@@QAEXXZ' in dtor
        init = references('?init@SocialWin@@QAEXH@Z')
        assert init.count('?init@CheckButton@@QAEHHHPAVGraphicWin@@H@Z') == 3
        # Historical code really had the two defects; this control ensures
        # the observations above distinguish the migration from its baseline.
        old_inputs = {unit: subprocess.check_output(['git', 'show', BASE + ':' + unit], cwd=ROOT)
                      for unit in ('src/socialwin.cpp', 'src/socialwin.h')}
        old = Coff(compiler.overlay(ROOT / 'src/socialwin.cpp', old_inputs, (),
                                   flags + settings.layout_flags))
        assert class_layout(old, 'SocialWin')['members']['energyLockButtons_'] == 0xD68
        function = old.function('??0SocialWin@@QAE@XZ')
        section = old.sections[function.section - 1]
        old_targets = [old.symbols[target].name for _, target, kind in section.relocations]
        assert old_targets.count('??0GraphicWin@@QAE@XZ') == 2
