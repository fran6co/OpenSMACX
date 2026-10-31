"""Pinned count reloads: 52E1C8/52E1E9/52E29E; retained packet slot 52E304."""
from pathlib import Path
import subprocess
import tempfile
from test_netdaemon_transfer_modal import extract
ROOT=Path(__file__).resolve().parents[2]
HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
struct AlphaNetProcessSlot { uint32_t pid_; char name_[32]; int8_t identity_; };
struct NetWinLobbyRow { int owner_,prefix_,ready_or_occupied_,field_2_,faction_or_color_,field_4_; } NetWinLobbyRows[16];
struct { int difficulty_; } NetWinSessionOptions;
struct Net { void send(void*p,int n,unsigned long id,int f){assert(*(uint16_t*)p==0x1f01&&n==32&&id==99&&f==1);} };
struct NetDaemon:Net {
 struct {int leading_words_[1],session_phase_;} primary_state_;
 int player_count_; AlphaNetProcessSlot process_slots_[16];
 unsigned long local_player_id(){return 17;} unsigned long host_player_id(){return 17;}
 char*get_player_name(unsigned long); void on_player_added(char*,unsigned long);
} *active;
char*NetDaemon::get_player_name(unsigned long id){assert(id==99);player_count_=3;static char s[]="name";return s;}
char*observed_copy(char*d,const char*s){assert(d==active->process_slots_[3].name_);char*r=std::strcpy(d,s);active->player_count_=4;return r;}
void parse_says(int,char*,int,int){}
struct Popup {void pop(const char*,int,int,void*){assert(active->process_slots_[4].identity_==4);active->player_count_=5;}} NetMsgPrimary;
struct Dialog {bool is_visible(){return false;}void on_redraw(){}} NetWinDialog;
int calls;
void send_all_player_packets(int t){assert(t==-1&&NetWinLobbyRows[5].prefix_==5);++calls;active->player_count_=6;}
void send_session_packet(int s,int t){assert(s==5&&t==-1);++calls;}
void send_session2_packet(int s,int t){assert(s==5&&t==-1);++calls;}
void send_faction_data(int s){assert(s==5);++calls;}
#define strcpy observed_copy
BODY
#undef strcpy
int main(){NetDaemon n={};active=&n;n.player_count_=1;n.process_slots_[1].pid_=17;NetWinSessionOptions.difficulty_=9;char name[]="join";n.on_player_added(name,99);assert(n.process_slots_[2].pid_==99&&std::strcmp(n.process_slots_[3].name_,"name")==0);assert(NetWinLobbyRows[4].prefix_==4&&NetWinLobbyRows[4].field_2_==9);assert(NetWinLobbyRows[5].prefix_==5&&NetWinLobbyRows[5].field_2_==9&&calls==4);n.player_count_=7;n.on_player_added(name,99);}
'''
def run(body):
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'c.cpp').write_text(HARNESS.replace('BODY',body));r=subprocess.run(['g++','-std=c++11','-O2',str(p/'c.cpp'),'-o',str(p/'c')],capture_output=True,text=True);assert r.returncode==0,r.stderr
  return subprocess.run([str(p/'c')],capture_output=True).returncode
def test_live_slots():
 assert run(extract((ROOT/'src/netdaemon.cpp').read_text(),'void NetDaemon::on_player_added('))==0
def test_packet_snapshot_damage():
 b=extract((ROOT/'src/netdaemon.cpp').read_text(),'void NetDaemon::on_player_added(');bad=b.replace('send_session_packet(slot, -1)','send_session_packet(player_count_, -1)');assert b!=bad and run(bad)!=0

def test_pinned_original_call_and_global_boundaries():
 from hashlib import sha256
 from match_recovery.source import read_file
 from match_recovery.source.asm import original_asm
 image=Path('/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe')
 assert sha256(image.read_bytes()).hexdigest()=='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
 records={r.address:r for r in read_file(ROOT/'src/netdaemon.cpp')}
 add='\n'.join(map(str,original_asm(records[0x52e160],image).lines))
 delete='\n'.join(map(str,original_asm(records[0x52e370],image).lines))
 for witness in ('0x0052E1C3','call 0x631a20','0x0052E1C8','0x0052E1E9','0x0052E29E','0x0052E304'):
  assert witness in add
 for witness in ('0x0052E460','[esi + 0x1bbc]','0x0052E46A','0x0052E3AC','[0x93e96c]','0x0052E42B','[0x93e8d0]','0x0052E479','[0x93e8c0]','0x0052E55E','jne 0x52e57e','0x0052E4FA','0x0052E5B5'):
  assert witness in delete
