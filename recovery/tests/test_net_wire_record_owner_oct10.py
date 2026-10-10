"""Actual snapshot producer and bounded send prefix, not full Net/runtime proof."""

from pathlib import Path

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_net_receive_player_actual_source import HARNESS
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def broadcast_control():
    source = (ROOT / "src/net_class.cpp").read_text()
    record = extract(source, "struct NetPeerPlayerRecord") + ";"
    body = extract(source, "void Net::broadcast_player_table()")
    allocator = "void *mem_get(int size) { return malloc(static_cast<size_t>(size)); }"
    assert HARNESS.count(allocator) == 1
    prefix = HARNESS.replace(allocator,
        "void *mem_get(int size) { void *p=malloc(static_cast<size_t>(size)); "
        "memset(p,0xA5,static_cast<size_t>(size)); return p; }")
    return prefix + record + "\n" + body + r'''
int main(){
  for(int n: {0,1,2,16}) for(int length: {0,3,31,43}){
    Net net{};net.field_760_=7;net.field_764_=7;net.field_6DC_=n;
    sent_count=0;sent_size=0;
    for(int i=0;i!=16;++i){
      net.players_[i].field_0_=0x10203000+i;
      net.players_[i].flags_14_=static_cast<unsigned char>(0x85+i);
      memset(net.players_[i].name_and_tail_,0xBC,sizeof net.players_[i].name_and_tail_);
      memset(net.players_[i].name_and_tail_,'A'+i,length);
      if(length<43)net.players_[i].name_and_tail_[length]=0;
    }
    net.broadcast_player_table();
    if(!n){assert(!sent_count);continue;}
    assert(sent_count==1 && sent_size==n*44+4 && sent_packet[0]==n);
    assert(sent_packet[1]==0xA5 && sent_packet[2]==0xA5 && sent_packet[3]==0xA5);
    for(int i=0;i!=n;++i){
      const int base=4+i*40;uint32_t id,flags;
      memcpy(&id,sent_packet+base,4);memcpy(&flags,sent_packet+base+36,4);
      assert(id==static_cast<uint32_t>(0x10203000+i) && flags==static_cast<uint32_t>(0x85+i));
      assert(net.players_[i].name_and_tail_[31]==0);
      const int used=length<31?length:31;
      for(int k=0;k!=used;++k)assert(sent_packet[base+4+k]=='A'+i);
      assert(sent_packet[base+4+used]==0);
      for(int k=used+1;k!=32;++k)assert(sent_packet[base+4+k]==0xA5);
    }
    for(int k=4+n*40;k!=sent_size;++k)assert(sent_packet[k]==0xA5);
    net.field_764_=8;net.broadcast_player_table();assert(sent_count==1);
  }
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("* 0x2C + 4", "* 0x28 + 4"),
    ("*packet = static_cast<unsigned char>(count);", "*reinterpret_cast<unsigned int *>(packet) = count;"),
    ("player->name_and_tail_[0x1F] = 0;", "(void)player;"),
    ("entry->flags_ = flags;", "entry->flags_ = 0;"),
    ("++entry;", "(void)entry;")))
def test_actual_snapshot_wire_records(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, broadcast_control(), damage)


def envelope_control():
    source = (ROOT / "src/net_class.cpp").read_text()
    envelope = extract(source, "struct NetPacketEnvelope") + ";"
    body = extract(source, "int Net::send_group_flagged(")
    first = body.index("NetPacketEnvelope *const header")
    last = body.index("for (int clear_index", first)
    prefix = body[first:last]
    return r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
typedef uint16_t WORD;typedef uint32_t DWORD;typedef uint32_t DPID;
ENVELOPE
static_assert(sizeof(NetPacketEnvelope)==8,"wire envelope");
static unsigned char sent[32];static int sends;
struct DirectPlay {
    int Send(DPID from,DPID target,DWORD reliable,void *packet,DWORD size){
        assert(from==7 && target==9 && size==16 && reliable<=1);
        ++sends;std::memcpy(sent,packet,size);return 0;
    }
};
static DirectPlay provider;static DirectPlay *NetDirectPlay=&provider;
static DWORD field_E4_,voice_flags_D8_,field_760_=7;
int prepare(unsigned char *packet,unsigned int flags,unsigned short type,const void *data,int size){
    int allocation_size=size+12;long target=9;
PREFIX
    // Prefix-only witness: the original continues into the retry machinery.
    std::memcpy(sent,packet,allocation_size);std::free(packet);return 123;
}
int main(){
    const unsigned char payload[]={1,2,3,4};
    for(int reliable=0;reliable!=2;++reliable)for(int force=0;force!=2;++force){
        unsigned char *packet=static_cast<unsigned char *>(std::malloc(16));
        std::memset(packet,0xA5,16);field_E4_=0xffffffffu;
        voice_flags_D8_=force?0x10000000u:0;sends=0;
        int result=prepare(packet,reliable,0x10,payload,4);
        uint16_t type;uint32_t sequence;std::memcpy(&type,sent,2);std::memcpy(&sequence,sent+4,4);
        assert(sequence==0);
        assert(field_E4_==(reliable?0u:0xffffffffu));
        assert(type==(reliable && !force?0x14:0x10));
        assert(sent[2]==0xA5 && sent[3]==0xA5);
        assert(!std::memcmp(sent+8,payload,4));
        for(int i=12;i!=16;++i)assert(sent[i]==0xA5);
        assert(sends==(reliable && !force?0:1));
        assert(result==(reliable && !force?123:static_cast<int>(field_E4_)));
    }
}
'''.replace("ENVELOPE", envelope).replace("PREFIX", prefix)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("header->sequence_4_ = 0;", "header->field_2_ = 0;"),
    ("++field_E4_;", "(void)field_E4_;"),
    ("static_cast<unsigned short>(type | 4)", "static_cast<unsigned short>(type)"),
    ("memcpy(packet + 8, data", "memcpy(packet + 9, data")))
def test_actual_send_envelope_prefix(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, envelope_control(), damage)
