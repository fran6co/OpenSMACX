"""Actual-source workshop controls; neither full-game nor linked acceptance."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = '5bc25d02ccc7c66c0cb3da679b8baeee66a0c5d0'
SEED = 'src/unrecovered/00438460.cpp'
OWNER = ROOT / 'src/designwin.cpp'
HARNESS = ROOT / 'recovery/tests/designwin_button_harness.cpp'
ADDRESS = 0x438460


def test_original_identity_spans_and_findings_retained():
    old, = read_text(subprocess.check_output(['git', 'show', BASE + ':' + SEED], cwd=ROOT).decode(), ROOT / SEED)
    current, = (r for r in read_file(OWNER) if r.address == ADDRESS)
    for field in ('address', 'name', 'image_spans', 'byte_exact', 'semantic',
                  'levers', 'ruled_out', 'deferred', 'unrecoverable'):
        assert getattr(current, field) == getattr(old, field), field
    assert current.mode == Mode.BODY and not current.extract_error


def test_native_workshop_uses_real_members_and_owner_dependencies():
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
        obj = Coff(compiler(OWNER, (), flags + settings.layout_flags))
        function = obj.function('?on_button_clicked@DesignWin@@UAEXH@Z')
        body, relocations, _ = obj.subject(function)
        assert len(body) > 1000
        targets = {obj.symbols[int(r.target)].name for r in relocations}
        for expected in ('?VehPrototypes@@3PAUVehPrototype@@A',
                         '?Weapon@@3PAURulesWeapon@@A', '?Armor@@3PAURulesArmor@@A',
                         '?UpgradePromptAnswered@@3IA', '?NetDaemonNet@@3VNetDaemon@@A',
                         '?DesignConsiderOverride@@3HA', '?clear_bunglist@@YAXH@Z'):
            assert expected in targets, (expected, sorted(targets))
        prototype = class_layout(obj, 'VehPrototype')
        assert prototype['size'] == 0x34
        assert prototype['members']['flags'] == 0x30
        assert prototype['members']['obsolete_factions'] == 0x2C
        design = class_layout(obj, 'DesignWin')
        assert design['members']['field_CDC_'] == 0xCDC
        assert design['members']['field_CE4_'] == 0xCE4
        assert '?consider_designs@@YAXI@Z' in targets  # still unfinished dependency
        assert not any('0x00438460' in line for line in _body().splitlines())


_spec = importlib.util.spec_from_file_location('designwin_definition_helper', ROOT / 'recovery/tests/test_game_logic_seed_batch_behavior.py')
_helper = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_helper)


def _body():
    row, = (r for r in read_file(OWNER) if r.address == ADDRESS)
    assert row.mode == Mode.BODY and row.region and not row.extract_error
    body = _helper._definition(row.region, 'void DesignWin::on_button_clicked(')
    assert 'BODY GOES HERE.' not in body and 'reinterpret_cast<pending>' not in body
    return body


def _run(directory, body, compiler, scenario):
    directory.mkdir(parents=True, exist_ok=True)
    source, executable = directory / 'workshop.cpp', directory / 'workshop'
    fixture = HARNESS.read_text()
    assert fixture.count('/* EXTRACTED_DESIGNWIN_BUTTON */') == 1
    source.write_text(fixture.replace('/* EXTRACTED_DESIGNWIN_BUTTON */', body))
    built = subprocess.run([compiler, '-std=c++11', '-fsanitize=address,undefined',
                            '-fno-sanitize-recover=all', str(source), '-o', str(executable)],
                           text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable), scenario], text=True, capture_output=True,
                          timeout=5, env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=0'})


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_product_handler_branches_and_damage_controls(tmp_path, compiler):
    compiler = shutil.which(compiler)
    if compiler is None:
        pytest.skip('host compiler unavailable')
    body = _body()
    for scenario in ('ordering', 'retire', 'retire-all', 'upgrade-filter', 'upgrade-alien'):
        result = _run(tmp_path / scenario, body, compiler, scenario)
        assert result.returncode == 0, result.stdout + result.stderr
    damages = (
        ('ordering', 'if (button_id == -2 || button_id == 8)', 'if (button_id == -2 || button_id == 9)'),
        ('upgrade-filter', 'if (old_weapon_rating > candidate_weapon_rating)', 'if (old_weapon_rating < candidate_weapon_rating)'),
        ('upgrade-alien', '"YESUPGRADE", 0, "alunit_sm.pcx"', '"YESUPGRADE", 0, "unit_sm.pcx"'),
        ('retire-all', '            const uint8_t faction_mask = static_cast<uint8_t>(1u << faction);', '            const uint8_t faction_mask = static_cast<uint8_t>(1u << factionID_);'),
        ('upgrade-filter', 'if (old_armor_rating > candidate_armor_rating)', 'if (old_armor_rating <= candidate_armor_rating)'),
        ('upgrade-filter', '''            if (candidate.plan != old_prototype.plan) {
                if (candidate.plan > 3 || old_prototype.plan > 3) {
                    continue;
                }
            }
''', ''),
    )
    for index, (scenario, old, new) in enumerate(damages):
        assert body.count(old) == 1
        changed = body.replace(old, new, 1)
        result = _run(tmp_path / ('damage-' + str(index)), changed, compiler, scenario)
        assert result.returncode != 0, 'Deliberate workshop damage went undetected'
