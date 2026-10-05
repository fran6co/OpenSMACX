"""Bounded typed helper migration controls, not whole-game acceptance."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = '9f7d39cdb1f16680ac232f60bab112bfbe678a3d'
SPECS = (
    (0x515430, 'src/statuswin.cpp', 'src/recovered/00515430.cpp', 'int __cdecl reset_status_and_initialize_desktop(', '?reset_status_and_initialize_desktop@@YAHXZ'),
    (0x58FB70, 'src/stringstruct.cpp', 'src/recovered/units/0058fb70.cpp', 'char *__cdecl uppercase_text(', '?uppercase_text@@YAPADPADH@Z'),
    (0x592D70, 'src/general.cpp', 'src/recovered/00592d70.cpp', 'int __cdecl set_scale_adjustment(', '?set_scale_adjustment@@YAHH@Z'),
    (0x592D90, 'src/general.cpp', 'src/recovered/00592d90.cpp', 'int __cdecl apply_scale_adjustment(', '?apply_scale_adjustment@@YAHH@Z'),
    (0x4396B0, 'src/maininterface.cpp', 'src/recovered/units/004396b0.cpp', 'void __stdcall main_interface_bubble_callback_4396b0(', '?main_interface_bubble_callback_4396b0@@YGXPADHH@Z'),
)
_spec = importlib.util.spec_from_file_location('owned_helper_definition', ROOT / 'recovery/tests/test_game_logic_seed_batch_behavior.py')
_helper = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_helper)


@pytest.mark.parametrize('address,owner,seed,signature,symbol', SPECS)
def test_complete_original_identity_and_findings_retained(address, owner, seed, signature, symbol):
    old, = read_text(subprocess.check_output(['git', 'show', BASE + ':' + seed], cwd=ROOT).decode(), ROOT / seed)
    current, = (r for r in read_file(ROOT / owner) if r.address == address)
    for field in ('address','name','image_spans','byte_exact','semantic','levers','ruled_out','deferred','unrecoverable'):
        assert getattr(current, field) == getattr(old, field), field
    assert current.mode == Mode.BODY and not current.extract_error


def _bodies():
    bodies = {}
    for address, owner, seed, signature, symbol in SPECS:
        row, = (r for r in read_file(ROOT / owner) if r.address == address)
        assert row.mode == Mode.BODY and row.region and not row.extract_error
        bodies[address] = _helper._definition(row.region, signature)
    return bodies


def test_native_helpers_use_real_owner_objects_and_sdk():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.source_inventory import image_pin
    from match_recovery.model import Span
    from match_recovery.object_source import read_objects
    from match_recovery.coff_replica import function_signature
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    image = PeImage((ROOT / '.opensmacx/game/terranx_original.exe').read_bytes(), image_pin(ROOT))
    assert image.read_initial(Span(0x945AEC, 0x945AF0)) == bytes(4)
    binding, = (b for b in read_objects(ROOT, image_pin(ROOT)) if b.origin.address == 0x945AEC)
    assert binding.symbol == '?StoredScaleAdjustment@@3HA' and binding.span is None
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for flags in settings.flag_sets:
        objects = {unit: Coff(compiler(ROOT / unit, (), flags)) for unit in sorted({s[1] for s in SPECS})}
        refs = {}
        for address, unit, seed, signature, symbol in SPECS:
            body, relocs, _ = objects[unit].subject(objects[unit].function(symbol))
            assert body
            refs[address] = {objects[unit].symbols[int(r.target)].name for r in relocs}
        assert '?StatusFlags@@3IA' in refs[0x515430]
        assert '?desktop_init@@YAHH@Z' in refs[0x515430]
        assert '__imp__CharUpperA@4' in refs[0x58FB70]
        assert '?StoredScaleAdjustment@@3HA' in refs[0x592D70]
        assert '?StoredScaleAdjustment@@3HA' in refs[0x592D90]
        assert '?MainInterfaceGlobal@@3VMainInterface@@A' in refs[0x4396B0]
        assert any('client_to_screen@' in s for s in refs[0x4396B0])
        assert any('set_bubble_text@Win@@' in s for s in refs[0x4396B0])
        # Existing restoration debt must be distinguished from this batch.
        # Fresh code/external-reference equality is bounded COFF evidence,
        # not retained linked-image or whole-object preservation.
        headers = {unit: subprocess.check_output(['git','show',BASE+':'+unit],cwd=ROOT)
                   for unit in ('src/game.h','src/general.h','src/statuswin.h','src/maininterface.h')}
        old_status_inputs = {**headers, 'src/statuswin.cpp': subprocess.check_output(['git','show',BASE+':src/statuswin.cpp'],cwd=ROOT)}
        old_status = Coff(compiler.overlay(ROOT / 'src/statuswin.cpp',old_status_inputs,(),flags))
        assert function_signature(old_status,'?arrow_right@StatusWin@@QAEXXZ') == function_signature(objects['src/statuswin.cpp'],'?arrow_right@StatusWin@@QAEXXZ')
        old_string_inputs = {**headers, 'src/stringstruct.cpp': subprocess.check_output(['git','show',BASE+':src/stringstruct.cpp'],cwd=ROOT)}
        old_strings = Coff(compiler.overlay(ROOT / 'src/stringstruct.cpp',old_string_inputs,(),flags))
        for strings in (old_strings,objects['src/stringstruct.cpp']):
            assert not any(s.name == '?current_text@StringList@@QAEPADXZ' and s.section > 0
                           for s in strings.symbols.values())


def _run(directory, bodies, compiler):
    directory.mkdir(parents=True, exist_ok=True)
    template = (ROOT / 'recovery/tests/owned_helper_harness.cpp').read_text()
    selected = '\n'.join(bodies[a] for a in (0x592D70,0x592D90,0x515430,0x4396B0))
    source, executable = directory / 'helpers.cpp', directory / 'helpers'
    source.write_text(template.replace('/* EXTRACTED_OWNED_HELPERS */', selected))
    built = subprocess.run([compiler,'-std=c++11','-fsanitize=address,undefined','-fno-sanitize-recover=all',str(source),'-o',str(executable)], text=True,capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)],text=True,capture_output=True,timeout=5,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})


@pytest.mark.parametrize('compiler', ['g++','clang++'])
def test_source_helpers_and_deliberate_damage(tmp_path, compiler):
    compiler = shutil.which(compiler)
    if compiler is None: pytest.skip('host compiler unavailable')
    bodies = _bodies()
    result = _run(tmp_path / 'pristine',bodies,compiler)
    assert result.returncode == 0, result.stdout + result.stderr
    damages = (
        (0x592D70,'if (value <= 0)','if (value == 0)'),
        (0x592D90,'+ 16u','+ 15u'),
        (0x592D90,'return product / 16;','return static_cast<int>(bits / 16u);'),
        (0x515430,'StatusFlags = 0;','StatusFlags = 1;'),
        (0x515430,'desktop_init(1);','desktop_init(2);'),
        (0x4396B0,'point.right = x;','point.right = y;'),
    )
    for index,(address,old,new) in enumerate(damages):
        assert bodies[address].count(old) == 1
        changed = dict(bodies)
        changed[address] = changed[address].replace(old,new,1)
        result = _run(tmp_path / ('damage-'+str(index)),changed,compiler)
        assert result.returncode != 0, 'Helper damage went undetected'
