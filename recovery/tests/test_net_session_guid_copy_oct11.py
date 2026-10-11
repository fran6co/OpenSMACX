"""Two typed GUID copies; original session behavior remains unchanged."""
from dataclasses import fields
from pathlib import Path
import json
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_net_provider_guid_oct9 import _body

ROOT = Path(__file__).resolve().parents[2]
BASE = '18f93db66f4f21dc74122f759676e95aee5b3126'


def migration(text, *, reverse=False):
    fixture = Path(__file__).with_name('net-session-guid-copy-oct11.patch').read_text()
    for hunk in fixture.split('\n@@ ')[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ', '-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ', '+')))
        if reverse:
            before, after = after, before
        assert before and text.count(before) == 1
        text = text.replace(before, after, 1)
    return text


def restore_session_guid_copy(text):
    return migration(text, reverse=True) if '    allocation_node_.instance_guid_ = live->guidInstance;' in text else text


def test_exact_bounded_source_and_historical_continuity():
    path = ROOT / 'src/net_class.cpp'
    old = subprocess.check_output(['git', 'show', BASE + ':src/net_class.cpp'], cwd=ROOT, text=True)
    expected = migration(old)
    assert expected == path.read_text()
    assert migration(expected, reverse=True) == old
    before, after = read_text(old, path), read_text(expected, path)
    assert len(before) == len(after)
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name not in {'line', 'region', 'origin'}:
                assert getattr(a, field.name) == getattr(b, field.name)
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)


def test_native_task_and_typed_copy_order():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / 'recovery/tasks/net-session-guid-copy-oct11.json').read_text()))
    assert task.base_revision == BASE and {s.address for s in task.subjects} == {0x62EAA0}
    body = _body((ROOT / 'src/net_class.cpp').read_text(), 'NetAllocationNode *Net::create_session(')
    assert 'reinterpret_cast' not in body
    assert body.index('desc.dwFlags =') < body.index('desc.guidApplication = *guid_6E8_;') < body.index('desc.dwMaxPlayers =')
    assert body.index('GetSessionDesc(live, &size)') < body.index('allocation_node_.instance_guid_ = live->guidInstance;') < body.index('allocation_node_.session_flags_ =') < body.index('free(live);')


def test_pinned_complete_guid_load_store_sequences():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / '.opensmacx/game/terranx_original.exe'
    if not path.is_file():
        pytest.skip('pinned image unavailable')
    image = PeImage(path.read_bytes(), '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions = {i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x62EAA0,0x62ED2F)),0x62EAA0)}
    expected = {
        0x62EB67:'eax, dword ptr [esi + 0x6e8]',
        0x62EB7C:'ecx, dword ptr [eax]', 0x62EB80:'dword ptr [esp + 0x2c], ecx',
        0x62EB84:'edx, dword ptr [eax + 4]', 0x62EB87:'dword ptr [esp + 0x30], edx',
        0x62EB8B:'ecx, dword ptr [eax + 8]', 0x62EB8E:'dword ptr [esp + 0x34], ecx',
        0x62EB92:'edx, dword ptr [eax + 0xc]', 0x62EB99:'dword ptr [esp + 0x38], edx',
        0x62ECD7:'eax, dword ptr [ecx]', 0x62ECD9:'dword ptr [edx], eax',
        0x62ECDB:'eax, dword ptr [ecx + 4]', 0x62ECDE:'dword ptr [edx + 4], eax',
        0x62ECE1:'eax, dword ptr [ecx + 8]', 0x62ECE4:'dword ptr [edx + 8], eax',
        0x62ECEB:'ecx, dword ptr [ecx + 0xc]', 0x62ECEE:'dword ptr [edx + 0xc], ecx',
    }
    for address, operands in expected.items():
        assert instructions[address] == ('mov', operands)
    assert instructions[0x62ECCD] == ('lea', 'ecx, [ebp + 8]')
    assert instructions[0x62ECD0] == ('lea', 'edx, [esi + 0x70c]')


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['', 'application', 'instance'])
def test_actual_session_guid_values(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip('host compiler unavailable')
    body = _body((ROOT / 'src/net_class.cpp').read_text(), 'NetAllocationNode *Net::create_session(')
    if damage == 'application':
        body = body.replace('desc.guidApplication = *guid_6E8_;', 'desc.guidApplication = NetSerialServiceGuid;')
    elif damage == 'instance':
        body = body.replace('allocation_node_.instance_guid_ = live->guidInstance;', 'allocation_node_.instance_guid_ = live->guidApplication;')
    harness = r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdint>
struct GUID { uint32_t Data1; uint16_t Data2, Data3; unsigned char Data4[8]; };
struct DPSESSIONDESC2 {
  uint32_t dwSize, dwFlags;
  GUID guidInstance, guidApplication;
  uint32_t dwMaxPlayers, dwCurrentPlayers;
  char *lpszSessionNameA, *lpszPasswordA;
  uint32_t dwReserved1, dwReserved2, dwUser1, dwUser2, dwUser3, dwUser4;
};
struct Caps { uint32_t dwSize; unsigned char rest[36]; } NetServiceCaps;
static const int DPSESSION_MIGRATEHOST=4, DPOPEN_CREATE=2;
static const GUID NetSerialServiceGuid={1,2,3,{4}}, NetModemServiceGuid={5,6,7,{8}};
static const GUID instance={0xFEDCBA98,0x7654,0x3210,{255,1,128,3,127,5,64,7}};
struct DirectPlay {
  DPSESSIONDESC2 opened{};
  int Open(DPSESSIONDESC2 *desc,int) { opened=*desc; return 0; }
  int Close() { return 0; }
  int GetCaps(Caps *,int) { return 0; }
  int GetSessionDesc(void *out,unsigned long *size) {
    *size=sizeof(DPSESSIONDESC2);
    if(out) { auto *d=static_cast<DPSESSIONDESC2 *>(out); *d=opened; d->guidInstance=instance; }
    return 0;
  }
} *NetDirectPlay;
void *mem_get(int bytes) { return malloc(bytes); }
struct NetAllocationNode {
  char *session_name_=nullptr, *password_=nullptr;
  GUID instance_guid_{};
  unsigned int session_flags_{}, max_players_{}, current_players_{};
};
struct Net {
  GUID service_provider_guid_{};
  GUID *guid_6E8_{};
  NetAllocationNode allocation_node_;
  unsigned int field_44_{}, voice_flags_D8_{};
  NetAllocationNode *create_session(char *,int,char *);
};
'''
    main = r'''
int main() {
  GUID app={0x11223344,0x5566,0x7788,{9,10,11,12,13,14,15,16}};
  DirectPlay dp; NetDirectPlay=&dp;
  Net net; net.guid_6E8_=&app; net.field_44_=19;
  char name[]="session", password[]="secret";
  auto *node=net.create_session(name,7,password);
  assert(node==&net.allocation_node_);
  assert(memcmp(&dp.opened.guidApplication,&app,sizeof(GUID))==0);
  assert(memcmp(&node->instance_guid_,&instance,sizeof(GUID))==0);
  assert(dp.opened.dwMaxPlayers==7 && dp.opened.dwUser1==19);
  assert(node->session_flags_==4 && node->max_players_==7 && node->current_players_==0);
  assert(strcmp(node->session_name_,name)==0 && strcmp(node->password_,password)==0);
  assert(net.voice_flags_D8_==0x10000);
  free(node->session_name_); free(node->password_);
}
'''
    source = tmp_path / 'actual.cpp'
    source.write_text(harness + body + main)
    executable = tmp_path / 'actual'
    subprocess.run([compiler,'-std=c++17','-O2',str(source),'-o',str(executable)],check=True,capture_output=True)
    result = subprocess.run([str(executable)],capture_output=True,cwd=tmp_path,
                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode == 0) == (not damage)
