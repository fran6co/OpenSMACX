"""Actual artillery command with service doubles, not combat/runtime proof."""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]

def body():
    source = (ROOT / "src/veh.cpp").read_text()
    start = source.index("void __cdecl action_arty(int veh_id, int x, int y) {")
    return source[start:source.index("\n}", start) + 2]

HARNESS = r'''
#include <cassert>
#include <cstdlib>
#include <cstring>
#define __cdecl
struct Veh {int faction_id,x,y;} Vehs[8];
struct {int faction_id_;}ConsoleGlobal;
struct {unsigned diplo_treaties[8];}PlayersData[8];
struct {unsigned artillery_max_rng;}Rules;
const int RadiusRange[]={1,9,25,49,81,121,169,225,289};
int MapIsFlat,MapLongitude=10,MapLongitudeBounds=20,IsMultiplayerNet;
int ready=1,target=1,direction=4,lock_result,maybe_result,fight_result;
int calls,observed_mode,observed_id,opcode,notify,destroyed,unlocked,awaited;
struct NetDaemon {
 struct {int dword_[4];}lock_state_[2];
 int lock_veh(int*id,int mode,int x,int y,int zero){
  assert(mode==4&&x==2&&y==0&&zero==0);*id=3;return lock_result;}
 int maybe_lock(int*id,int a,int b,int x,int y,int mode){
  assert(a==0&&b==0&&x==2&&y==0&&mode==4);*id=3;return maybe_result;}
 void await_exec(int n){assert(n==1);++awaited;}
 void unlock_veh(){++unlocked;}
}NetDaemonNet;
struct Message {void pop(const char*key,int t,int zero,const char*p){
 assert(t==5000&&zero==0&&!p);notify=!std::strcmp(key,"UNITMOVED")?1:2;
}}message;Message*NetMsgPopup=&message;
int veh_ready(int){return ready;}
int veh_at(int,int){return target;}
int stack_fix(int n){return n;}
void action_destroy(int id,int bits,int x,int y){assert(id==0&&bits==0&&x==2&&y==0);++destroyed;}
void message_data(int op,int zero,int id,int x,int y,int end){
 assert(zero==0&&id==3&&x==2&&y==0&&end==0);opcode=op;}
void message_veh(int op,int id,int dir,int zero){assert(id==3&&dir==4&&zero==0);opcode=op;}
int radius_move(int x,int y,int,int,int range){assert(x==0&&y==0&&range==25);return direction;}
int battle_fight(int id,int dir,int a,int b,int*p){
 assert(dir==4&&a==1&&b==1&&!p);++calls;observed_id=id;
 observed_mode=NetDaemonNet.lock_state_[1].dword_[3];return fight_result;}
BODY
void reset(){std::memset(PlayersData,0,sizeof PlayersData);std::memset(Vehs,0,sizeof Vehs);
 Vehs[1].faction_id=1;ready=1;target=1;direction=4;lock_result=maybe_result=fight_result=0;
 MapIsFlat=1;IsMultiplayerNet=0;Rules.artillery_max_rng=2;
 calls=observed_mode=observed_id=opcode=notify=destroyed=unlocked=awaited=0;
 NetDaemonNet.lock_state_[1].dword_[3]=17;}
int main(){
 reset();ready=0;action_arty(0,2,0);assert(notify==1&&calls==0);
 reset();action_arty(0,6,0);assert(notify==2&&calls==0);
 reset();MapIsFlat=0;action_arty(0,15,0);assert(notify==0&&calls==1);
 reset();Vehs[1].faction_id=0;action_arty(0,2,0);assert(calls==0);
 reset();PlayersData[0].diplo_treaties[1]=1;action_arty(0,2,0);assert(calls==0);
 reset();PlayersData[0].diplo_treaties[1]=2;action_arty(0,2,0);assert(calls==1);
 reset();direction=-1;action_arty(0,2,0);assert(calls==0);
 reset();target=-1;action_arty(0,2,0);assert(destroyed==1&&calls==0);
 reset();target=-1;IsMultiplayerNet=1;action_arty(0,2,0);assert(opcode==0x2413&&awaited==1);
 reset();target=-1;IsMultiplayerNet=1;lock_result=1;action_arty(0,2,0);assert(opcode==0&&awaited==0);
 reset();action_arty(0,2,0);assert(calls==1&&observed_mode==3&&observed_id==0);
 reset();IsMultiplayerNet=1;maybe_result=1;action_arty(0,2,0);
 assert(calls==0&&NetDaemonNet.lock_state_[1].dword_[3]==17);
 reset();IsMultiplayerNet=1;fight_result=1;action_arty(0,2,0);
 assert(calls==1&&observed_mode==1&&observed_id==3&&unlocked==1&&opcode==0&&awaited==0);
 reset();IsMultiplayerNet=1;action_arty(0,2,0);
 assert(calls==1&&observed_mode==1&&observed_id==3&&unlocked==0&&opcode==0x2402&&awaited==1);
}
'''

def execute(tmp_path, candidate, label):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
                   input=HARNESS.replace("BODY", candidate), text=True,
                   capture_output=True, check=True)
    return subprocess.run([str(executable)], capture_output=True).returncode

def test_artillery_branch_and_network_protocol(tmp_path):
    assert execute(tmp_path, body(), "artillery") == 0

@pytest.mark.parametrize("before,after", [
    ("diplo_treaties[target_faction] & 1U", "diplo_treaties[target_faction] & 2U"),
    ("dword_[3] = 3;", "dword_[3] = 1;"),
    ("message_veh(0x2402", "message_veh(0x2413"),
])
def test_artillery_damage_is_detected(tmp_path, before, after):
    candidate = body()
    assert before in candidate
    assert execute(tmp_path, candidate.replace(before, after), "damage") != 0
