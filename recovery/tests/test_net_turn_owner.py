"""Bounded native multiplayer-turn owner controls, not runtime acceptance."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = 'a61eaab05378c3e8ae791d630dd3bf1d5df3dd62'
SPECS = (
    (0x5290A0, 'src/unrecovered/005290a0.cpp', '?net_upkeep@@YAXXZ'),
    (0x529F30, 'src/recovered/units/00529f30.cpp', '?net_end_of_turn@@YAXXZ'),
    (0x52A2E0, 'src/unrecovered/0052a2e0.cpp', '?net_control_turn@@YAXXZ'),
)


@pytest.mark.parametrize('address,seed,symbol', SPECS)
def test_owner_preserves_complete_seed_identity_and_findings(address, seed, symbol):
    old, = read_text(subprocess.check_output(['git', 'show', BASE + ':' + seed], cwd=ROOT).decode(), ROOT / seed)
    current, = (r for r in read_file(ROOT / 'src/game.cpp') if r.address == address)
    for field in ('address', 'name', 'image_spans', 'byte_exact', 'semantic',
                  'levers', 'ruled_out', 'deferred', 'unrecoverable'):
        assert getattr(current, field) == getattr(old, field), field
    assert (current.symbol or current.name) == symbol


def test_native_turn_bodies_use_real_owner_dependencies_and_layout():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.codeview import class_layout
    from match_recovery.source_inventory import image_pin
    from match_recovery.model import Span
    from match_recovery.coff_replica import function_signature
    from match_recovery.object_source import read_objects
    from match_recovery.object_data import check_initial_data
    from recovery.environment.wine import VC6_ROOT
    original = ROOT / '.opensmacx/game/terranx_original.exe'
    if not original.is_file() or not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('pinned image, VC6 and Wine32 required')
    image = PeImage(original.read_bytes(), image_pin(ROOT))
    assert image.read_initial(Span(0x93A954, 0x93A958)) == bytes(4)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    literals = [b for b in read_objects(ROOT, image_pin(ROOT))
                if b.unit == 'src/game.cpp' and b.evidence is not None
                and b.symbol.startswith('_Net')
                and b.evidence.address in (0x5290A0, 0x529F30, 0x52A2E0)]
    assert len(literals) == 37
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / 'src/game.cpp', (), flags + settings.layout_flags))
        literal_results = check_initial_data(literals, {'src/game.cpp': obj}, image, {})
        assert len(literal_results) == len(literals)
        assert all(row['definition'] == 'pass' and row['content'] == 'pass'
                   for row in literal_results), literal_results
        def targets(symbol):
            function = obj.function(symbol)
            body, relocations, _ = obj.subject(function)
            assert len(body) > 100, symbol
            return [obj.symbols[int(r.target)].name for r in relocations]
        console = class_layout(obj, 'Console')
        assert console['members']['field_23BE4_'] == 0x23BE4
        assert console['members']['field_23BE8_'] == 0x23BE8
        assert 0x9156B0 + 0x23BE4 == 0x939294
        assert 0x9156B0 + 0x23BE8 == 0x939298
        prototype = class_layout(obj, 'VehPrototype')
        assert prototype['size'] == 0x34
        assert prototype['members']['flags'] == 0x30
        assert prototype['members']['obsolete_factions'] == 0x2C
        base = class_layout(obj, 'Base')
        assert base['size'] == 0x134 and base['members']['faction_id_current'] == 4
        upkeep = targets('?net_upkeep@@YAXXZ')
        for expected in ('?UpgradePromptAnswered@@3IA', '?TurnRedrawPending@@3HA',
                         '?CouncilPendingMessageArgs@@3PAHA', '?VehPrototypes@@3PAUVehPrototype@@A',
                         '?NetPeerTurnChecksums@@3PAY0BF@HA'):
            assert expected in upkeep, (expected, upkeep)
        end = targets('?net_end_of_turn@@YAXXZ')
        assert '__imp__timeGetTime@0' in end
        control = targets('?net_control_turn@@YAXXZ')
        assert '?net_upkeep@@YAXXZ' in control and '?net_end_of_turn@@YAXXZ' in control
        assert '?ConsoleGlobal@@3VConsole@@A' in control
        assert '?TurnControlWord93A954@@3HA' in control
        assert not any('NetTurnHandoffWord' in s.name or 'NetUpkeepLatch' in s.name
                       or 'NetUpkeepCouncilPayload' in s.name for s in obj.symbols.values())
        # This historical claim already misses on the unchanged owner source.
        # Compare fresh actual code/external-reference signatures, not an old
        # passing receipt. This is not whole-object or linked preservation.
        baseline_inputs = {unit: subprocess.check_output(['git', 'show', BASE + ':' + unit], cwd=ROOT)
                           for unit in ('src/game.cpp', 'src/game.h')}
        old = Coff(compiler.overlay(ROOT / 'src/game.cpp', baseline_inputs, (),
                                   flags + settings.layout_flags))
        assert function_signature(old, '?mon_enemy_destroyed@@YAXHH@Z') == function_signature(obj, '?mon_enemy_destroyed@@YAXHH@Z')
        # The stale StringList semantic symbol is unmeasurable before and
        # after this batch. Missing emission is not a pass or a demotion.
        string_old_inputs = {unit: subprocess.check_output(['git', 'show', BASE + ':' + unit], cwd=ROOT)
                             for unit in ('src/stringstruct.cpp', 'src/game.h')}
        current_strings = Coff(compiler(ROOT / 'src/stringstruct.cpp', (), flags))
        old_strings = Coff(compiler.overlay(ROOT / 'src/stringstruct.cpp', string_old_inputs, (), flags))
        missing = '?current_text@StringList@@QAEPADXZ'
        for strings in (current_strings, old_strings):
            assert not any(s.name == missing and s.section > 0 for s in strings.symbols.values())


HARNESS = ROOT / "recovery/tests/net_turn_harness.cpp"
OWNERS = {
    0x0052A2E0: ("src/game.cpp", "?net_control_turn@@YAXXZ",
                 "void __cdecl net_control_turn(", "EXTRACTED_NET_CONTROL_TURN"),
    0x00529F30: ("src/game.cpp", "?net_end_of_turn@@YAXXZ",
                 "void __cdecl net_end_of_turn(", "EXTRACTED_NET_END_OF_TURN"),
    0x005290A0: ("src/game.cpp", "?net_upkeep@@YAXXZ",
                 "void __cdecl net_upkeep(", "EXTRACTED_NET_UPKEEP"),
}

_helper_path = ROOT / "recovery/tests/test_game_logic_seed_batch_behavior.py"
_helper_spec = importlib.util.spec_from_file_location("net_turn_definition_helper",
                                                       _helper_path)
_helper = importlib.util.module_from_spec(_helper_spec)
_helper_spec.loader.exec_module(_helper)
_definition = _helper._definition


def _source_record(address):
    owner, decorated, signature, _marker = OWNERS[address]
    rows = [row for row in read_file(ROOT / owner)
            if row.address == address]
    assert len(rows) == 1, f"missing/duplicate mapped body {address:#x}"
    row = rows[0]
    assert row.mode in (Mode.FILE, Mode.BODY) and row.name == decorated
    assert row.region and not row.extract_error
    body = _definition(row.region, signature)
    assert "BODY GOES HERE." not in body
    return body


def _render(bodies):
    source = HARNESS.read_text()
    for address, (_owner, _decorated, _signature, marker) in OWNERS.items():
        token = "/*" + marker + "*/"
        assert source.count(token) == 1
        source = source.replace(token, bodies[address])
    return source


def _replace_once(body, old, new):
    assert body.count(old) == 1, f"mutation anchor is not unique: {old!r}"
    return body.replace(old, new)


def _run(tmp_path, bodies, compiler, scenario):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "net_turn_host.cpp"
    executable = tmp_path / "net_turn_host"
    source.write_text(_render(bodies))
    command = [compiler, "-std=c++98", "-Wall", "-Wextra",
               "-Wno-unused-function", "-Wno-unused-variable",
               "-Wno-unused-parameter", "-Wno-misleading-indentation",
               "-pedantic", "-fsanitize=address,undefined",
               "-fno-sanitize-recover=all", str(source), "-o", str(executable)]
    built = subprocess.run(command, text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable), scenario], text=True,
                          capture_output=True, timeout=5,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_net_turn_actual_bodies_and_damage_controls(tmp_path, compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    bodies = {address: _source_record(address) for address in OWNERS}
    scenarios = ("control", "end-turn", "end-turn-lock-mask",
                 "upkeep-clean", "upkeep-checksum")
    for index, scenario in enumerate(scenarios):
        result = _run(tmp_path / ("pristine-" + str(index)), bodies,
                      compiler_path, scenario)
        assert result.returncode == 0, result.stdout + result.stderr

    damages = []
    changed = dict(bodies)
    changed[0x005290A0] = _replace_once(
        changed[0x005290A0], "prototype_id < MaxVehProtoNum",
        "prototype_id < MaxVehProtoNum - 1")
    damages.append(("prototype-upper-bound", changed, "upkeep-clean"))

    changed = dict(bodies)
    changed[0x005290A0] = _replace_once(
        changed[0x005290A0], "++prototype_id", "prototype_id += 2")
    damages.append(("prototype-stride", changed, "upkeep-clean"))

    changed = dict(bodies)
    changed[0x005290A0] = _replace_once(
        changed[0x005290A0], "word - 5", "word - 4")
    damages.append(("checksum-cursor", changed, "upkeep-checksum"))

    changed = dict(bodies)
    changed[0x0052A2E0] = _replace_once(
        changed[0x0052A2E0], "NetDaemonNet.locks_.clear();", "")
    damages.append(("game-end-lock-reset", changed, "control"))

    changed = dict(bodies)
    changed[0x00529F30] = _replace_once(
        changed[0x00529F30], "ConsoleGlobal.field_23BE4_ = 0;",
        "ConsoleGlobal.field_23BE8_ = 0;")
    damages.append(("console-handoff-member", changed, "end-turn"))

    changed = dict(bodies)
    changed[0x00529F30] = _replace_once(
        changed[0x00529F30], "(player_lock.active_ & 1u) == 0",
        "(player_lock.active_ & 2u) == 0")
    damages.append(("playerlock-bit-mask", changed, "end-turn-lock-mask"))

    for index, (name, changed_bodies, scenario) in enumerate(damages):
        result = _run(tmp_path / ("damage-" + str(index)), changed_bodies,
                      compiler_path, scenario)
        assert result.returncode != 0, f"damage control survived: {name}"
