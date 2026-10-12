"""Actual queue-wrapper/FIFO controls; host layout is not the original ABI."""
from pathlib import Path
import os
import resource
import shutil
import signal
import subprocess

import pytest

from test_mapwin_cache import braced

ROOT = Path(__file__).resolve().parents[2]
BASE = "9296b9cb88ae14663527136e2d8c4c9b3102767c"
METHODS = {
    0x630A40: 'void *Net::sub_630a40(int key, int *length_out)',
    0x630980: 'void *Net::sub_630980(unsigned int *key_out, int *length_out)',
    0x6309C0: 'void *Net::sub_6309c0(int key, int *length_out)',
}
HISTORICAL_METHODS = METHODS.copy()
HISTORICAL_METHODS[0x630A40] = 'void *Net::sub_630a40(unsigned int *key, int *length_out)'
SYMBOLS = {
    0x630A40: '?sub_630a40@Net@@QAEPAXHPAH@Z',
    0x630980: '?sub_630980@Net@@QAEPAXPAIPAH@Z',
    0x6309C0: '?sub_6309c0@Net@@QAEPAXHPAH@Z',
}
HISTORICAL_SYMBOLS = dict(SYMBOLS)
HISTORICAL_SYMBOLS[0x630A40] = '?sub_630a40@Net@@QAEPAXPAIPAH@Z'


def test_only_three_interfaces_change_and_obligations_remain():
    from match_recovery.source import read_text
    path = ROOT / 'src/net_class.cpp'
    old = subprocess.check_output(['git','show',BASE+':src/net_class.cpp'],cwd=ROOT,text=True)
    current = path.read_text()
    # Preserve the old three-interface scope guard, first reversing only the
    # subsequent explicitly authorized transport closure against its own base.
    # This does not reset the historical BASE or exempt any unrelated body.
    from test_net_transport_queue_closure_oct12 import NET_METHODS
    import json
    task = json.loads((ROOT / 'recovery/tasks/net-transport-queue-closure-oct12.json').read_text())
    closure_base = subprocess.check_output(
        ['git', 'show', task['base_revision'] + ':src/net_class.cpp'], cwd=ROOT, text=True)
    normalized = current
    for signature in NET_METHODS:
        previous = signature
        if signature.startswith('void *Net::sub_630a40('):
            previous = HISTORICAL_METHODS[0x630A40]
        elif signature.startswith('void *Net::read_queue_key_until_timeout('):
            previous = 'void *Net::read_queue_key_until_timeout(unsigned int *key, int *length_out)'
        normalized = normalized.replace(braced(current, signature), braced(closure_base, previous), 1)
    # Only fresh coordinator-observed symbol updates are reversed in this
    # bridge; historical original names/spans/claims remain untouched.
    normalized = normalized.replace('?sub_630a40@Net@@QAEPAXHPAH@Z',
                                    '?sub_630a40@Net@@QAEPAXPAIPAH@Z')
    normalized = normalized.replace('?read_queue_key_until_timeout@Net@@QAEPAXHPAH@Z',
                                    '?read_queue_key_until_timeout@Net@@QAEPAXPAIPAH@Z')
    assert normalized == closure_base, 'outside the eight authorized Net/FIFO closure subjects changed'
    # Reverse only the independently bounded original player-ID correction.
    from test_net_polling_player_id_oct11 import restore_polling_player_id
    from test_net_session_guid_copy_oct11 import restore_session_guid_copy
    normalized = restore_polling_player_id(restore_session_guid_copy(normalized))
    for address, signature in HISTORICAL_METHODS.items():
        name = signature.split('Net::')[1].split('(')[0]
        previous = 'int Net::'+name+'(int first, int second)'
        normalized = normalized.replace(braced(closure_base,signature),braced(old,previous),1)
        normalized = normalized.replace(HISTORICAL_SYMBOLS[address],'?'+name+'@Net@@QAEHHH@Z')
        history = '// Historical seed prototype: int (__thiscall '+name+')(Net* this, int, int)\n'
        history += '// Historical catalogue: callers 1, call targets 1; not reachability proof.'
        catalogue = '// prototype int (__thiscall '+name+')(Net* this, int, int)\n'
        catalogue += '// callers   1   call targets   1'
        assert normalized.count(history)==1
        normalized=normalized.replace(history,catalogue)
    normalized = normalized.replace(
        '// Reads a matching message off the 0x10C queue: NetGetScratch is the buffer,\n'
        '// key supplies the routing-key filter and length_out receives the signed length.\n'
        '// The answer is the\n',
        '// Reads one message off the 0x10C queue: NetGetScratch is the buffer, `first`\n'
        "// receives the routing key and `second` the caller's int. The answer is the\n")
    assert normalized == old
    def identity(r):
        return (r.address,r.name,r.image_spans,r.byte_exact,r.semantic,
                r.source_role,r.source_role_reason,r.function_fragment)
    assert [identity(r) for r in read_text(old,path)] == [identity(r) for r in read_text(current,path)]
    rows = {r.address:r for r in read_text(current,path)}
    assert all(rows[a].symbol==s for a,s in SYMBOLS.items())
    header = (ROOT/'src/net_class.h').read_text()
    before = subprocess.check_output(['git','show',BASE+':src/net_class.h'],cwd=ROOT,text=True)
    header = header.replace('void *sub_630a40(int key, int *length_out);',
                            'void *sub_630a40(unsigned int *key, int *length_out);')
    header = header.replace('void *read_queue_key_until_timeout(int key, int *length_out);',
                            'void *read_queue_key_until_timeout(unsigned int *key, int *length_out);')
    for signature in HISTORICAL_METHODS.values():
        declaration = signature.replace('Net::','')+';'
        name = signature.split('Net::')[1].split('(')[0]
        assert header.count(declaration)==1
        header=header.replace(declaration,'int '+name+'(int first, int second);')
    assert header == before


def test_pinned_trio_call_arguments_and_pointer_result():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path=Path(os.environ.get('OPENSMACX_IMAGE', ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():
        pytest.skip('pinned executable unavailable')
    assert path.is_absolute()
    image=PeImage(path.read_bytes(),image_pin(ROOT))
    for start,call,target,tail in ((0x630980,0x6309A4,0x634200,0x6309A9),
                                  (0x6309C0,0x6309EC,0x6340C0,0x6309F1),
                                  (0x630A40,0x630A6C,0x633D90,0x630A71)):
        rows={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(
            image.read(Span(start,tail+12)),start)}
        assert rows[call]==('call',hex(target))
        assert rows[call-9]==('push','edx')
        assert rows[call-8]==('push','0')
        assert rows[call-6]==('push','eax')
        assert rows[call-5]==('push','0x9bc4bc')
        assert rows[tail]==('neg','eax')
        assert rows[tail+2]==('sbb','eax, eax')
        assert rows[tail+4]==('and','eax, 0x9bc4bc')
        assert rows[tail+9]==('ret','8')


def actual_bodies():
    wrapper=(ROOT/'src/net_class.cpp').read_text()
    fifo=(ROOT/'src/net_fifo.cpp').read_text()
    return '\n'.join(braced(wrapper,s) for s in METHODS.values())+'\n'+ '\n'.join(
        braced(fifo,s) for s in (
            'int NetFifo::get(void *data, int key, int *type, int *len)',
            'int NetFifo::peek_head(void *data, unsigned int *key_out, int *caller_value_out, int *size_out)',
            'int NetFifo::peek_key(void *data, unsigned int key, int *caller_value_out, int *size_out)'))


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <initializer_list>
unsigned char NetGetScratch[0x2000];
void *NetDirectPlay;
int locks,unlocks;
void EnterCriticalSection(int*) { ++locks; }
void LeaveCriticalSection(int*) { ++unlocks; }
struct NetFifo {
    struct NetFifoNode {
        int caller_value_; uint32_t key_; int result_;
        void *data_; uint32_t data_size_; NetFifoNode *next_;
    };
    NetFifoNode *head_,*tail_; uint32_t count_; int lock_;
    int get(void*,int,int*,int*);
    int peek_head(void*,unsigned int*,int*,int*);
    int peek_key(void*,unsigned int,int*,int*);
};
struct Net {
    NetFifo queue_10c_;
    void *sub_630a40(int,int*);
    void *sub_630980(unsigned int*,int*);
    void *sub_6309c0(int,int*);
};
BODIES
NetFifo::NetFifoNode *node(unsigned int key,int result) {
    auto *n=static_cast<NetFifo::NetFifoNode*>(malloc(sizeof(NetFifo::NetFifoNode)));
    n->caller_value_=73; n->key_=key; n->result_=result; n->next_=0;
    n->data_=malloc(3);memcpy(n->data_,"abc",3);n->data_size_=3;
    return n;
}
void release(NetFifo &q) {
    while(q.head_) {auto *n=q.head_;q.head_=n->next_;free(n->data_);free(n);}
}
int main() {
    int token;NetDirectPlay=&token;
    for(int operation=0;operation<3;++operation)for(int result : {0,1,-1,INT_MIN})
        for(int omit_size=0;omit_size<2;++omit_size) {
            auto *n=node(0x80000001u,result);
            Net net={{n,n,1,0}};unsigned int key=0x80000001u;int size=-999;
            memset(NetGetScratch,0xcc,sizeof(NetGetScratch));locks=unlocks=0;
            void *answer=operation==0 ? net.sub_630a40(INT_MIN+1,omit_size?0:&size) :
                operation==1 ? net.sub_630980(&key,omit_size?0:&size) :
                net.sub_6309c0(INT_MIN+1,omit_size?0:&size);
            assert(answer==(result ? NetGetScratch : 0));
            assert(key==0x80000001u && size==(omit_size?-999:3));
            assert(memcmp(NetGetScratch,"abc",3)==0 && NetGetScratch[3]==0xcc);
            assert(locks==1 && unlocks==1);
            assert(net.queue_10c_.count_==(operation==0?0u:1u));
            assert(net.queue_10c_.head_==(operation==0?0:n));
            release(net.queue_10c_);
        }
    Net empty={{0,0,0,0}};unsigned int key=5;int size=-999;
    assert(!empty.sub_630a40(5,&size));
    assert(!empty.sub_630980(&key,&size));
    assert(!empty.sub_6309c0(5,&size));
    assert(key==5 && size==-999);
    for(int disconnected=0;disconnected<2;++disconnected) {
        auto *n=node(9,1);Net net={{n,n,1,0}};locks=unlocks=0;
        NetDirectPlay=disconnected?0:&token;
        if(disconnected) {
            assert(!net.sub_630a40(5,&size));assert(!net.sub_630980(&key,&size));
            assert(!net.sub_6309c0(9,&size));
        } else {
            assert(!net.sub_630a40(0,&size));assert(!net.sub_630980(0,&size));
            assert(!net.sub_6309c0(0,&size));
        }
        assert(!locks && !unlocks && net.queue_10c_.head_==n && size==-999);
        release(net.queue_10c_);
    }
    NetDirectPlay=&token;
    auto *n=node(9,1);Net net={{n,n,1,0}};key=8;size=-999;
    assert(!net.sub_630a40(8,&size) && key==8 && size==-999 && net.queue_10c_.head_==n);
    assert(!net.sub_6309c0(8,&size) && size==-999);
    key=0;
    assert(net.sub_630980(&key,&size)==NetGetScratch && key==9 && size==3);
    release(net.queue_10c_);
}
'''


@pytest.mark.parametrize('compiler_name',('g++','clang++'))
@pytest.mark.parametrize('damage',(None,'return','keyed-operation','peek-operation'))
def test_actual_wrappers_and_fifo_behavior(compiler_name,damage,tmp_path):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name+' unavailable')
    bodies=actual_bodies()
    if damage:
        old,new={
            'return':('return got != 0 ? NetGetScratch : 0;','return got > 0 ? NetGetScratch : 0;'),
            'keyed-operation':('queue_10c_.get(NetGetScratch, key, 0, length_out)',
                               'queue_10c_.peek_key(NetGetScratch, static_cast<unsigned int>(key), 0, length_out)'),
            'peek-operation':('queue_10c_.peek_head(NetGetScratch, key_out, 0, length_out)',
                              'queue_10c_.get(NetGetScratch, static_cast<int>(*key_out), 0, length_out)'),
        }[damage]
        assert old in bodies
        bodies=bodies.replace(old,new)
    path=tmp_path/'queue.cpp';path.write_text(HARNESS.replace('BODIES',bodies))
    binary=tmp_path/'queue'
    subprocess.run([compiler,'-std=c++11','-fsanitize=undefined',
                    '-fno-sanitize-recover=all',str(path),'-o',str(binary)],
                   check=True,capture_output=True)
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    result=subprocess.run([str(binary)],capture_output=True,preexec_fn=no_core)
    assert result.returncode==(0 if damage is None else -signal.SIGABRT),result.stderr
    if damage:
        assert b'assert' in result.stderr.lower(),result.stderr
