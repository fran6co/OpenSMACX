"""Actual map proposal chain with callback/state damage witnesses.

OPENSMACX_MAP_CALLCHAIN_SOURCE selects a private pre-application overlay for
coordinator review. Without it, extraction always uses the working product.
These service doubles establish bounded behavior, not native call visibility.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from recovery.tests import test_diplomacy_actions_coverage as owner
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


def replace_definition(text, pattern, replacement):
    body = definition(text, pattern)
    assert text.count(body) == 1
    return text.replace(body, replacement, 1)


def actual_chain():
    path = Path(os.environ.get('OPENSMACX_MAP_CALLCHAIN_SOURCE', ROOT / 'src/faction.cpp'))
    source = path.read_text()
    network = definition(source, r'void\s+__cdecl\s+net_maps\s*\(')
    helper = definition(source, r'void\s+__cdecl\s+propose_trade_maps\s*\(')
    caller = definition(source, r'void\s+__cdecl\s+make_a_proposal\s*\(')
    assert 'net_maps(faction_id, other_faction_id, 1);' in caller
    return network, helper, caller


def harness(network, helper, caller):
    fixture = owner._MAKE_PROPOSAL_FIXTURE
    assert fixture.count('#define __cdecl') == 1
    fixture = fixture.replace('#define __cdecl', '#define __cdecl\nint expected_network, expected_rejection, phase;')
    fixture = replace_definition(fixture, r'int\s+proposal_menu\s*\(', r'''int proposal_menu(int actor,int other) {
 assert(actor==1&&other==2);++menu_calls;
 // The original case7 must reset this after the menu returns.
 PopupDialogFactionID=7;return 1;
}''')
    fixture = replace_definition(fixture, r'void\s+log_say\s*\(', r'''void log_say(const char *label,int actor,int other,int zero) {
 assert(label==DiploLabelSendingTRADEMAPS&&actor==1&&other==2&&!zero);
 assert(phase==1&&PopupDialogFactionID==6);
 assert(!DiplomacyMapTransferChanged&&"TRANSFER_FLAG_IS_LATE");
 assert(static_cast<unsigned char>(PlayersData[2].diplo_patience[1])==128&&"PATIENCE_WRAP");
 phase=2;++log_count;
 // Selection of the network branch precedes this callback mutation.
 IsMultiplayerNet=0;
}''')
    fixture = replace_definition(fixture, r'void\s+message_data\s*\(', r'''void message_data(int op,int z,int actor,int other,int c,int d) {
 assert(op==0x2448&&!z&&actor==1&&other==2&&!c&&!d);
 assert(phase==2&&!IsMultiplayerNet&&PopupDialogFactionID==6);
 assert(!DiplomacyMapTransferChanged&&"TRANSFER_FLAG_IS_LATE");phase=3;++message_count;
}''')
    fixture, count = re.subn(r'struct Net \{.*?\} NetDaemonNet;', r'''struct Net {void await_diplo(int op) {
 assert(op==0x448&&phase==3&&!IsMultiplayerNet);
 assert(!DiplomacyMapTransferChanged&&"TRANSFER_FLAG_IS_LATE");phase=4;++awaits;
}} NetDaemonNet;''', fixture, count=1, flags=re.S)
    assert count == 1
    fixture = replace_definition(fixture, r'int\s+net_maps\s*\(', r'''void trade_maps(int actor,int other) {
 assert(actor==1&&other==2&&phase==1&&!IsMultiplayerNet&&PopupDialogFactionID==6);
 assert(!DiplomacyMapTransferChanged&&"TRANSFER_FLAG_IS_LATE");
 assert(static_cast<unsigned char>(PlayersData[2].diplo_patience[1])==128&&"PATIENCE_WRAP");
 phase=4;++map_calls;++PlayersData[2].energy_reserves;
}''')
    fixture = replace_definition(fixture, r'int\s+X_pops\s*\(', r'''int X_pops(const char *key,int flags,Sprite *sprite,int(*cb)()) {
 assert(PopupDialogFactionID==2&&"POST_MENU_TARGET");
 assert(flags==0x100000&&sprite==FactionDiplomacyPortraits[2]&&cb==pop_wait&&phase==0);
 assert(!std::strcmp(key,expected_rejection?"REJMAPS":"TRADEMAPS")&&"MAP_REPLY_BRANCH");
 assert(!DiplomacyMapTransferChanged&&"TRANSFER_FLAG_IS_LATE");
 phase=1;PopupDialogFactionID=6;
 // Network selection occurs after the popup callback, with live state.
 IsMultiplayerNet=expected_network;return -7;
}''')
    fixture = replace_definition(fixture, r'int\s+main\s*\(', r'''int main() {
 for(int scenario=0;scenario<4;++scenario) {
  reset();phase=0;expected_rejection=scenario<2;expected_network=scenario==2;
  proposal_menu_result=1;DiploScratchFA34=7;
  PlayersData[2].diplo_patience[1]=126;
  // Distinct directed rows make a reversed treaty lookup observable.
  PlayersData[1].diplo_treaties[2]=scenario>=2?1:0;
  PlayersData[2].diplo_treaties[1]=scenario>=2?0:1;
  DiploFriction=scenario==0?5:3;
  IsMultiplayerNet=!expected_network;
  make_a_proposal(1,2,0);
  assert(menu_calls==1&&popup_constructs==1&&popup_destroys==1);
  assert(PopupDialogFactionID==6);
  if(expected_rejection) {
   assert(phase==1&&!DiplomacyMapTransferChanged&&!log_count&&!message_count&&!awaits&&!map_calls);
   assert(static_cast<unsigned char>(PlayersData[2].diplo_patience[1])==126);
  } else if(expected_network) {
   assert(phase==4&&log_count==1&&message_count==1&&awaits==1&&!map_calls&&"COMPLETED_NETWORK_TRANSFER");
   assert(DiplomacyMapTransferChanged==1);
  } else {
   assert(phase==4&&map_calls==1&&!log_count&&!message_count&&!awaits);
   assert(PlayersData[2].energy_reserves==1&&DiplomacyMapTransferChanged==1);
  }
 }
 return 0;
}''')
    fixture = owner.host_fixture(fixture)
    return fixture.replace('// @@BODY@@', network + '\n' + helper + '\n' + caller, 1)


DAMAGE = {
    None: None,
    'missing_post_menu_target': 'POST_MENU_TARGET',
    'reversed_treaty_row': 'MAP_REPLY_BRANCH',
    'missing_network_wait': 'COMPLETED_NETWORK_TRANSFER',
    'early_transfer_flag': 'TRANSFER_FLAG_IS_LATE',
    'callback_network_reread': 'COMPLETED_NETWORK_TRANSFER',
    'wrong_patience_increment': 'PATIENCE_WRAP',
}


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', DAMAGE)
def test_actual_map_chain_callback_and_damage(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    assert compiler, 'required independent host compiler unavailable'
    network, helper, caller = actual_chain()
    before = network, helper, caller
    if damage == 'missing_post_menu_target':
        # Removing exactly the new statement reproduces the original baseline
        # dispatcher body, so this is the callback-mutation baseline witness.
        anchor = 'if (proposal_kind == 7) {\n        PopupDialogFactionID = other_faction_id;\n'
        caller = caller.replace(anchor, 'if (proposal_kind == 7) {\n', 1)
    elif damage == 'reversed_treaty_row':
        caller = caller.replace('PlayersData[faction_id].diplo_treaties[other_faction_id]',
                                'PlayersData[other_faction_id].diplo_treaties[faction_id]', 1)
    elif damage == 'missing_network_wait':
        caller = caller.replace('net_maps(faction_id, other_faction_id, 1);',
                                'net_maps(faction_id, other_faction_id, 0);', 1)
    elif damage == 'early_transfer_flag':
        caller = caller.replace('net_maps(faction_id, other_faction_id, 1);\n            DiplomacyMapTransferChanged = 1;',
                                'DiplomacyMapTransferChanged = 1;\n            net_maps(faction_id, other_faction_id, 1);', 1)
    elif damage == 'callback_network_reread':
        anchor = 'log_say(DiploLabelSendingTRADEMAPS, faction_id, faction_id_with, 0);'
        network = network.replace(anchor, anchor + '\nif (!IsMultiplayerNet) return;', 1)
    elif damage == 'wrong_patience_increment':
        caller = caller.replace('diplo_patience[faction_id] += 2;',
                                'diplo_patience[faction_id] += 1;', 1)
    assert ((network, helper, caller) != before) == (damage is not None), 'damage anchor drift'
    cpp = tmp_path / 'actual_map_chain.cpp'
    cpp.write_text(harness(network, helper, caller))
    exe = tmp_path / 'control'
    built = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    if damage is None:
        assert ran.returncode == 0, ran.stderr
    else:
        assert ran.returncode != 0 and DAMAGE[damage] in ran.stderr, ran.stderr
