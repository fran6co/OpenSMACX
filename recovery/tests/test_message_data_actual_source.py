"""Compile production packet/routing code with controlled transport boundaries."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name('message_data_owner_host.cpp')


def product_bodies():
    path = ROOT/'src/netdaemon.cpp'
    rows = {r.address:r for r in read_file(path)}
    text = path.read_text()
    start = text.index('struct MessageDataRecord {')
    # Only the sender-owned record/layout assertions belong in this fixture.
    # Receiver wire views are independently extracted by their own controls.
    end = text.index('// Borrowed wire view',start)
    record = text[start:end]
    assert record.count('static_assert(') == 8
    lines = text.splitlines()
    send_line = next(n for n,line in enumerate(lines,1)
                     if line.startswith('int NetDaemon::send_message('))
    send_body = _extract_forward_text(lines,send_line)
    caller_line = next(n for n,line in enumerate(lines,1)
                       if line.startswith('uint32_t NetDaemon::unlock_veh()'))
    caller_body = _extract_forward_text(lines,caller_line)
    bodies = record + '\n' + send_body + '\n' + rows[0x592EE0].region + '\n' + caller_body
    assert all(rows[a].state.value=='implemented' and not rows[a].extract_error
               for a in (0x532940,0x592EE0))
    return bodies


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('mutation',['none','return','server','payload','offline','precedence','caller_result'])
def test_actual_message_packet_routing_and_return(tmp_path,compiler_name,mutation):
    compiler = shutil.which(compiler_name)
    if not compiler:pytest.skip(f'{compiler_name} unavailable')
    bodies = product_bodies()
    replacements = {
        'return':('    return result;', '    (void)result; return NetBusyCount;'),
        'server':('value = host_player_id();', 'value = target;'),
        'payload':('message.payload_18_ = payload_18;',
                   '(void)payload_18; message.payload_18_ = payload_14;'),
        'offline':('return multiplayer;', 'return -1;'),
        'precedence':('if (flags & 0x8000)', 'if (flags & 0x1000)'),
        'caller_result':('return residue;', 'return residue + 1;'),
    }
    if mutation!='none':
        old,new = replacements[mutation]
        assert old in bodies
        bodies = bodies.replace(old,new)
    fixture = FIXTURE.read_text()
    assert fixture.count('// ACTUAL_MESSAGE_DATA_BODIES')==1
    cpp = tmp_path/'actual.cpp'; binary = tmp_path/'actual'
    cpp.write_text(fixture.replace('// ACTUAL_MESSAGE_DATA_BODIES',bodies))
    subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],
                   check=True,capture_output=True,text=True,timeout=30)
    result = subprocess.run([str(binary)],capture_output=True,timeout=30)
    assert (result.returncode==0)==(mutation=='none'),result.stderr


def test_original_identities_claim_and_forwarder_retirement():
    rows = {r.address:r for r in read_file(ROOT/'src/netdaemon.cpp')}
    message,send = rows[0x592EE0],rows[0x532940]
    assert message.name=='?message_data@@YAXHHHHHH@Z'
    assert message.image_spans==((0x592EE0,0x592F4C),)
    assert message.symbol=='?message_data@@YAHHHHHHH@Z'
    assert not message.byte_exact and not message.semantic
    assert send.name=='?send_message@NetDaemon@@QAEXPADKH@Z'
    assert send.image_spans==((0x532940,0x532A44),)
    assert send.symbol=='?send_message@NetDaemon@@QAEHPADKH@Z' and send.byte_exact
    # Unwritten original bytes are retained, not silently zero-initialized.
    assert 'MessageDataRecord message;' in message.region
    assert 'unresolved_2_' not in message.region
    assert 'memset' not in message.region and 'memcpy' not in message.region
    assert 'message =' not in message.region and 'message{}' not in message.region
    caller = rows[0x5310F0].region
    assert 'message_data(' in caller and 'return residue;' in caller
