"""Original polling ID selection; no timeout/error-path normalization."""
from dataclasses import fields
import json
import os
from pathlib import Path
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_net_polling_global_owner_actual_source_oct10 import _actual_sources, _harness

ROOT = Path(__file__).resolve().parents[2]
BASE = '57729afb89ce3ed036b3b26268213db23a86540f'


def migration(text, *, reverse=False):
    fixture = Path(__file__).with_name('net-polling-player-id-oct11.patch').read_text()
    hunks = fixture.split('\n@@ ')[1:]
    for hunk in reversed(hunks) if reverse else hunks:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ', '-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ', '+')))
        if reverse:
            before, after = after, before
        assert before and text.count(before) == 1
        text = text.replace(before, after, 1)
    return text


def restore_polling_player_id(text):
    return migration(text, reverse=True) if 'if (local_player_id_ == players_[1].field_0_) {' in text else text


def test_exact_bounded_source_and_historical_continuity():
    path = ROOT / 'src/net_class.cpp'
    old = subprocess.check_output(['git', 'show', BASE + ':src/net_class.cpp'], cwd=ROOT, text=True)
    expected = migration(old)
    from test_net_session_guid_copy_oct11 import restore_session_guid_copy
    assert expected == restore_session_guid_copy(path.read_text())
    assert migration(expected, reverse=True) == old
    before, after = read_text(old, path), read_text(expected, path)
    assert len(before) == len(after)
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name not in {'line', 'region', 'origin'}:
                assert getattr(a, field.name) == getattr(b, field.name)
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)


def test_pinned_id_discriminator_and_original_timeout_drop_order():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    if not path.is_file():
        pytest.skip('pinned image unavailable')
    image = PeImage(path.read_bytes(), '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions = {i.address: (i.mnemonic, i.op_str) for i in
        Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x6320E0, 0x632290)), 0x6320E0)}
    for address, instruction in {
        0x63216E: ('mov', 'eax, dword ptr [esi + 0x760]'),
        0x6321E2: ('cmp', 'eax, dword ptr [esi + 0x1ac]'),
        0x6321E8: ('jne', '0x632232'),
        0x6321EA: ('mov', 'edi, dword ptr [esi + 0x164]'),
        0x632204: ('jbe', '0x632280'),
        0x632206: ('mov', 'ecx, dword ptr [esi + 0x154]'),
        0x632211: ('call', '0x632350'),
        0x632218: ('jne', '0x632280'),
        0x63221C: ('mov', 'dword ptr [esi + 0x164], eax'),
        0x632225: ('mov', 'dword ptr [0x9be60c], 0'),
    }.items():
        assert instructions[address] == instruction
    assert 0x154 + 0x58 == 0x1AC
    assert 0x154 + 0x58 + 0x10 == 0x1BC


CASES = r'''
  // Row 1 ID, not its stamp, selects row-0-only reaping. Preserve empty
  // row-0 IDs, timeout equality and drop failures exactly as the original.
  for (int mode = 0; mode < 6; ++mode) {
    NetReapBusy = NetKeepAliveBusy = 0;
    HarnessNet one;
    one.local_player_id_ = mode == 5 ? 0 : 77;
    one.host_player_id_ = 88;
    one.voice_flags_D8_ = 0x40000;
    one.poll_time_DC_ = fake_now - 3000;
    one.players_[0].field_0_ = mode == 4 ? 0 : 123;
    one.players_[1].field_0_ = one.local_player_id_;
    one.players_[1].stamp_10_ = 0;
    one.players_[0].stamp_10_ = mode == 1 ? 0 : fake_now - (mode == 2 ? 100 : 101);
    one.drop_result = mode == 3 ? 1 : 0;
    const uint32_t original_stamp = one.players_[0].stamp_10_;
    one.check_polling();
    assert(one.sends == 1 && NetReapBusy == 0 && NetKeepAliveBusy == 0);
    assert(one.players_[1].stamp_10_ == 0);
    if (mode == 1 || mode == 2) {
      assert(one.drops == 0 && one.players_[0].stamp_10_ == original_stamp);
    } else {
      assert(one.drops == 1 && one.last_drop_id == one.players_[0].field_0_);
      assert(one.players_[0].stamp_10_ == (mode == 3 ? original_stamp : fake_now));
    }
  }
  // Coincidental timestamp equality must not select the one-player arm.
  NetReapBusy = NetKeepAliveBusy = 0;
  HarnessNet coincident;
  coincident.local_player_id_ = 77; coincident.host_player_id_ = 88;
  coincident.voice_flags_D8_ = 0x40000;
  coincident.poll_time_DC_ = fake_now - 3000;
  coincident.players_[0].field_0_ = 123;
  coincident.players_[0].stamp_10_ = fake_now - 101;
  coincident.players_[1].field_0_ = 456;
  coincident.players_[1].stamp_10_ = 77;
  coincident.check_polling();
  assert(coincident.drops == 1 && coincident.last_drop_id == 0xFFFFFFFFu);
  assert(coincident.players_[0].stamp_10_ == fake_now &&
         coincident.players_[1].stamp_10_ == fake_now && NetReapBusy == 0);
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'timestamp', 'wrong_row', 'timeout_equality', 'drop_target'])
def test_actual_row_id_selection_and_failure_paths(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    player, polling, abort, _ = _actual_sources()
    mutations = {
        'timestamp': ('local_player_id_ == players_[1].field_0_', 'local_player_id_ == players_[1].stamp_10_'),
        'wrong_row': ('local_player_id_ == players_[1].field_0_', 'local_player_id_ == players_[0].field_0_'),
        'timeout_equality': ('<= poll_timeout_E0_', '< poll_timeout_E0_'),
        'drop_target': ('drop_player(players_->field_0_, 0)', 'drop_player(players_[1].field_0_, 0)'),
    }
    if damage != 'none':
        before, after = mutations[damage]
        assert polling.count(before) == 1
        polling = polling.replace(before, after)
    source = _harness(player, polling, abort)
    source = source.replace('int sends, drops, reenter;', 'int sends, drops, reenter; uint32_t last_drop_id; int drop_result;')
    source = source.replace('drops(0), reenter(0)', 'drops(0), reenter(0), last_drop_id(0), drop_result(0)')
    source = source.replace('int HarnessNet::drop_player(uint32_t, int)', 'int HarnessNet::drop_player(uint32_t player_id, int)')
    source = source.replace('  ++drops;', '  ++drops; last_drop_id = player_id;')
    source = source.replace('  return 0;\n}\n' + polling, '  return drop_result;\n}\n' + polling)
    assert 'return drop_result;' in source
    marker = '  ExitTurnLoop = 0;\n  assert(actual_abort_answer(0)'
    assert source.count(marker) == 1
    source = source.replace(marker, CASES + marker)
    unit = tmp_path / 'polling.cpp'; executable = tmp_path / 'polling'
    unit.write_text(source)
    built = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra', str(unit), '-o', str(executable)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10, preexec_fn=no_core)
    assert (ran.returncode == 0) == (damage == 'none'), ran.stderr
