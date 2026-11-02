"""Actual packet-owner transport and bounded original/native call evidence.

These controls do not qualify the dispatcher, linked allocation or game runtime.
Native compilation is opt-in and must run under a coordinator compiler lease.
"""
from dataclasses import fields
import os
from pathlib import Path
import subprocess

import pytest
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from match_recovery.binary import RelocationKind
from match_recovery.model import Span
from match_recovery.pe_coff import Coff,PeImage
from match_recovery.source import read_file,read_text
from netdaemon_packet_source import BASE,historical_source
from netdaemon_packet_source import INTERFACE_OWNERS, _interface_blocks
from test_netdaemon_command_wire_actual_source_oct13ac import _cpp_tokens

ROOT=Path(__file__).resolve().parents[2]
PIN='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
CALLERS=(0x530D50,0x52E370,0x5310F0,0x530D00,0x530600,0x52E050,
    0x5328C0,0x5304A0,0x530AA0,0x530E50,0x531300,0x532140,
    0x532570,0x531780,0x532E00)


def test_spock_moved_blocks_are_complete_immutable_original_identities():
    from netdaemon_packet_source import _spock_blocks,SPOCK_OWNERS
    blocks,positions=_spock_blocks(ROOT)
    assert set(blocks)==set(SPOCK_OWNERS)
    assert len(set(positions.values()))==2
    assert all(blocks[address].count("// ORIGINAL: ")==1 for address in SPOCK_OWNERS)
    # The physical owner TU may add only includes and scalar declarations
    # around these transported blocks; _spock_blocks rejects every extra code
    # statement or detached annotation before returning these bodies.


def test_interface_moved_blocks_are_complete_immutable_parent_identities():
    blocks, positions, _ends = _interface_blocks(ROOT)
    assert set(blocks) == set(INTERFACE_OWNERS)
    assert len(positions) == 15
    ordered = sorted((address for address in positions if address != 0x0051F2C0),
                     key=positions.__getitem__)
    assert ordered == [address for address in INTERFACE_OWNERS if address != 0x0051F2C0]
    assert positions[0x0051F2C0] < positions[0x00593220]
    assert all(blocks[address].count("// ORIGINAL: ") == 1 for address in INTERFACE_OWNERS)
    assert all("Purpose:" in blocks[address] for address in INTERFACE_OWNERS if address != 0x0051F2C0)


def test_interface_tu_has_only_its_existing_netdaemon_oy_profile():
    cmake=(ROOT/'CMakeLists.txt').read_text()
    assert cmake.count('    src/netdaemon_interface.cpp\n') == 2
    sections=[section.split(')',1)[0] for section in cmake.split('set_source_files_properties(')[1:]]
    owner_sections=[section for section in sections if 'src/netdaemon_interface.cpp' in section]
    assert len(owner_sections)==1
    assert 'src/netdaemon.cpp' in owner_sections[0]
    assert 'COMPILE_OPTIONS "/Oy-"' in owner_sections[0]


def assert_native_rows_preserved():
    before=subprocess.check_output(['git','show',BASE+':src/netdaemon.cpp'],cwd=ROOT,text=True)
    old={r.address:r for r in read_text(before,ROOT/'src/netdaemon.cpp')}
    rows=[*read_file(ROOT/'src/netdaemon.cpp'),*read_file(ROOT/'src/netdaemon_message_data.cpp'),
          *read_file(ROOT/'src/netdaemon_spock.cpp'),*read_file(ROOT/'src/netdaemon_interface.cpp'),
          *read_file(ROOT/'src/netdaemon_vehicle.cpp')]
    current={r.address:r for r in rows}
    assert len(rows)==len(current)==len(old) and current.keys()==old.keys()
    for address,record in old.items():
        for field in fields(record):
            if field.name=='path' and address in (0x592EE0,0x592E10,0x593090,0x51F1B0,0x51F0C0,0x532140,0x532570,0x531780,*INTERFACE_OWNERS):
                assert record.path==ROOT/'src/netdaemon.cpp'
                expected_owner = ('src/netdaemon_message_data.cpp' if address in (0x592EE0,0x592E10,0x593090)
                                  else 'src/netdaemon_vehicle.cpp' if address in (0x532140,0x532570,0x531780)
                                  else 'src/netdaemon_spock.cpp' if address in (0x51F1B0,0x51F0C0)
                                  else 'src/netdaemon_interface.cpp')
                assert current[address].path==ROOT/expected_owner
                continue
            if field.name not in {'line','region','origin'}:
                assert getattr(current[address],field.name)==getattr(record,field.name),(address,field.name)
        def lessons(row):
            return [(x.kind,x.key,x.prose,tuple(line for _,line in x.lines)) for x in row.origin]
        assert lessons(current[address])==lessons(record)


def test_spock_and_packet_native_facts_remain_one_to_one():
    assert_native_rows_preserved()


def test_inverse_rehome_retains_complete_receiver_tokens_and_all_native_facts():
    before=subprocess.check_output(['git','show',BASE+':src/netdaemon.cpp'],cwd=ROOT,text=True)
    reconstructed=historical_source(ROOT)
    assert _cpp_tokens(reconstructed)==_cpp_tokens(before)
    assert_native_rows_preserved()
    cmake=(ROOT/'CMakeLists.txt').read_text()
    assert cmake.count('    src/netdaemon_message_data.cpp\n')==2
    for section in cmake.split('set_source_files_properties(')[1:]:
        section=section.split(')',1)[0]
        if 'src/netdaemon_message_data.cpp' in section:
            assert 'src/netdaemon.cpp' in section and 'COMPILE_OPTIONS "/Oy-"' in section
        if 'src/netdaemon_interface.cpp' in section:
            assert 'src/netdaemon.cpp' in section and 'COMPILE_OPTIONS "/Oy-"' in section


@pytest.mark.parametrize('damage',['sender-return','sender-callback-order','sender-extra-code',
    'sender-extra-identity','sender-orphan-symbol','record-field','record-extra-code',
    'record-orphan-context','receiver-duplicate-sender','spock-guard','spock-opcode',
    'spock-extra-code','spock-fake-state','spock-duplicate-marker','spock-orphan-fact',
    'interface-guard','interface-opcode','interface-extra-code','interface-extra-identity',
    'interface-orphan-fact','interface-extra-include','interface-order','spock-order',
    'vehicle-payload','lock-target','vehicle-record-extra','lock-record-extra',
    'vehicle-owner-code','vehicle-owner-extra-code','vehicle-owner-extra-identity','vehicle-owner-order'])
def test_inverse_projection_rejects_changed_or_hidden_new_owner_inputs(damage):
    sender=(ROOT/'src/netdaemon_message_data.cpp').read_text()
    header=(ROOT/'src/netdaemon_message_record.h').read_text()
    receiver=(ROOT/'src/netdaemon.cpp').read_text()
    spock=(ROOT/'src/netdaemon_spock.cpp').read_text()
    interface=(ROOT/'src/netdaemon_interface.cpp').read_text()
    vehicle=(ROOT/'src/netdaemon_vehicle.cpp').read_text()
    if damage=='vehicle-owner-code':
        vehicle=vehicle.replace('NetDaemonWaitTicks += waited;', 'NetDaemonWaitTicks -= waited;', 1)
    elif damage=='vehicle-owner-extra-code': vehicle+='\nint unexpected_vehicle_storage;\n'
    elif damage=='vehicle-owner-extra-identity':
        vehicle=vehicle.replace('// ORIGINAL: 0x00532140', '// ORIGINAL: 0x00532140\n// ORIGINAL: 0x00532140', 1)
    elif damage=='vehicle-owner-order':
        from netdaemon_packet_source import _vehicle_blocks
        blocks = _vehicle_blocks(ROOT)
        first, second = blocks[0x00532140], blocks[0x00532570]
        vehicle=vehicle.replace(first, '/* VEHICLE_ORDER_SLOT */', 1)
        vehicle=vehicle.replace(second, first, 1).replace('/* VEHICLE_ORDER_SLOT */', second, 1)
    elif damage=='vehicle-payload':
        sender=sender.replace('message.field_14_ = payload_14;', 'message.field_14_ = payload_10;', 1)
    elif damage=='lock-target':
        sender=sender.replace('message.field_18_ = target;', 'message.field_18_ = payload_18;', 1)
    elif damage=='vehicle-record-extra':
        header=header.replace('struct MessageVehicleRecord {', 'struct MessageVehicleRecord { int extra_;', 1)
    elif damage=='lock-record-extra':
        header=header.replace('struct MessageLockRecord {', 'struct MessageLockRecord { int extra_;', 1)
    elif damage=='sender-return': sender=sender.replace('return result;','return 0;',1)
    elif damage=='sender-callback-order':
        sender=sender.replace('message.timestamp_ = timeGetTime();','message.timestamp_ = 0;',1)
    elif damage=='sender-extra-code': sender+='\nint unexpected_packet_storage;\n'
    elif damage=='sender-extra-identity':
        marker=next(s for s in sender.splitlines() if s.startswith('// ORIGINAL:'))
        sender=marker+'\n'+sender
    elif damage=='sender-orphan-symbol': sender='// symbol    _unexpected\n'+sender
    elif damage=='record-field': header=header.replace('int payload_1C_;','int payload_1C_; int extra_;',1)
    elif damage=='record-extra-code': header='int unexpected_packet_storage;\n'+header
    elif damage=='record-orphan-context': header='// compile_context src/netdaemon.cpp\n'+header
    elif damage=='receiver-duplicate-sender': receiver+='\nint __cdecl message_data() {}\n'
    elif damage=='spock-guard': spock=spock.replace('SpockNotifyPending != 0', 'SpockNotifyPending == 0', 1)
    elif damage=='spock-opcode': spock=spock.replace('NetDaemonNet.synch(0x12,', 'NetDaemonNet.synch(0x13,', 1)
    elif damage=='spock-extra-code': spock+='\nint unexpected_spock_state;\n'
    elif damage=='spock-fake-state': spock=spock.replace('#include "netdaemon.h"', '#include "netdaemon.h"\nint SpockNotifyPending;', 1)
    elif damage=='spock-duplicate-marker':
        marker='// ORIGINAL: 0x0051F1B0'
        spock=spock.replace(marker, marker+'\n'+marker, 1)
    elif damage=='spock-orphan-fact': spock='// symbol    _unexpected_spock\n'+spock
    elif damage=='interface-guard': interface=interface.replace('IsMultiplayerNet != 0', 'IsMultiplayerNet == 0', 1)
    elif damage=='interface-opcode': interface=interface.replace('NetDaemonNet.synch(0x11,', 'NetDaemonNet.synch(0x13,', 1)
    elif damage=='interface-extra-code': interface+='\nint unexpected_interface_state;\n'
    elif damage=='interface-extra-identity':
        marker='// ORIGINAL: 0x00593220'
        interface=interface.replace(marker, marker+'\n'+marker, 1)
    elif damage=='interface-orphan-fact': interface='// symbol    _unexpected_interface\n'+interface
    elif damage=='interface-extra-include': interface=interface.replace('#include "game.h"', '#include "game.h"\n#include "win.h"', 1)
    elif damage=='interface-order':
        blocks, _positions, _ends = _interface_blocks(ROOT)
        first, second = blocks[0x0051F2C0], blocks[0x00593220]
        assert interface.count(first) == interface.count(second) == 1
        interface=interface.replace(first, '/* TRANSPORT_ORDER_SLOT */', 1)
        interface=interface.replace(second, first, 1).replace('/* TRANSPORT_ORDER_SLOT */', second, 1)
    elif damage=='spock-order':
        from netdaemon_packet_source import _spock_blocks
        blocks, _positions = _spock_blocks(ROOT)
        first, second = blocks[0x0051F1B0], blocks[0x0051F0C0]
        assert spock.count(first) == spock.count(second) == 1
        spock=spock.replace(first, '/* TRANSPORT_ORDER_SLOT */', 1)
        spock=spock.replace(second, first, 1).replace('/* TRANSPORT_ORDER_SLOT */', second, 1)
    with pytest.raises(AssertionError):
        historical_source(ROOT,receiver_text=receiver,sender_text=sender,header_text=header,
                          spock_text=spock,interface_text=interface,vehicle_text=vehicle)


def test_pinned_complete_caller_spans_retain_original_packet_builder_calls():
    paths=[Path(os.environ['OPENSMACX_IMAGE'])] if os.environ.get('OPENSMACX_IMAGE') else [p/'.opensmacx/game/terranx_original.exe' for p in (ROOT,*ROOT.parents)]
    path=next((p for p in paths if p.is_file()),None)
    if path is None: pytest.skip('pinned proprietary image unavailable')
    image=PeImage(path.read_bytes(),PIN)
    rows={r.address:r for r in read_file(ROOT/'src/netdaemon.cpp')}
    rows.update({r.address:r for r in read_file(ROOT/'src/netdaemon_message_data.cpp')})
    rows.update({r.address:r for r in read_file(ROOT/'src/netdaemon_vehicle.cpp')})
    for address in (*CALLERS,0x592EE0):
        row=rows[address]
        assert len(row.image_spans)==1
        start,end=row.image_spans[0]
        instructions=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start))
        cursor=start
        for instruction in instructions:
            assert instruction.address==cursor
            cursor+=instruction.size
        assert cursor==end and instructions[-1].mnemonic=='ret'
        if address!=0x592EE0:
            assert any(i.mnemonic=='call' and i.op_str=='0x592ee0' for i in instructions)
        else:
            assert len(instructions)==33
            assert any(i.address==0x592F3D and i.mnemonic=='call' and i.op_str=='0x532940' for i in instructions)


@pytest.fixture(scope='module')
def native_packet_objects():
    if os.environ.get('OPENSMACX_NATIVE_PACKET_BOUNDARY')!='1':
        pytest.skip('requires explicitly leased native compiler epoch')
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    settings=Measurement.read(ROOT)
    compiler=Compiler(ROOT,profile=settings.compiler_profile)
    return {unit:[Coff(compiler(ROOT/unit,(),flags+settings.layout_flags))
                  for flags in settings.flag_sets]
            for unit in ('src/netdaemon.cpp','src/netdaemon_message_data.cpp','src/netdaemon_vehicle.cpp')}


def edges(obj,symbol,target):
    code,relocations,_=obj.subject(obj.function(symbol))
    found=[]
    for relocation in relocations:
        if obj.symbols[int(relocation.target)].name!=target: continue
        assert relocation.kind==RelocationKind.RELATIVE32
        assert relocation.offset>=1 and code[relocation.offset-1] in (0xE8,0xE9)
        assert code[relocation.offset:relocation.offset+4]==b'\0\0\0\0'
        found.append(relocation.offset)
    return found


def test_native_direct_packet_boundaries_and_real_sender_target(native_packet_objects):
    receiver={r.address:r for unit in ('src/netdaemon.cpp','src/netdaemon_vehicle.cpp')
              for r in read_file(ROOT/unit)}
    sender=next(r for r in read_file(ROOT/'src/netdaemon_message_data.cpp') if r.address==0x592EE0)
    target=sender.symbol or sender.name
    send=receiver[0x532940].symbol or receiver[0x532940].name
    for unit in ('src/netdaemon.cpp','src/netdaemon_vehicle.cpp'):
        for obj in native_packet_objects[unit]:
            # synch's independently outlined source helper remains a separate
            # physical boundary; it is not an invented second original identity.
            for address in (*[a for a in CALLERS if a!=0x532E00],0x534400):
                row=receiver[address]
                if row.path != ROOT/unit: continue
                assert edges(obj,row.symbol or row.name,target),hex(address)
    for obj in native_packet_objects['src/netdaemon_message_data.cpp']:
        assert len(edges(obj,target,send))==1
