"""Pinned chat callback/order controls; no native layout or runtime proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from test_diplowin_exec_behavior import extract_function
ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
using std::strlen; using std::strcat;
struct Player { int is_leader_female; char name_leader[32]; };
static Player Players[8];
static int PluralityDefault, GenderDefault, DiploOffersDirty;
static struct Console { int faction_id_; } ConsoleGlobal;
static std::vector<int> events;
static std::string transcript;
static int transcript_faction, modal_calls, repeat_seen, key_seen;
struct EditBox {
 char text_[256];
 void set_text(const char *s) { assert(s==0 && DiploOffersDirty==1); events.push_back(3); text_[0]=0; }
 int on_key_click(int repeat,int key) { events.push_back(4); repeat_seen=repeat; key_seen=key; return -7; }
};
struct StringBox {
 void add(char *text,int faction,int mode) {
  assert(mode==1 && PluralityDefault==0 && GenderDefault==1 && DiploOffersDirty==0);
  events.push_back(2); transcript=text; transcript_faction=faction;
 }
};
struct DiploWin {
 int faction_id_with_; EditBox editBox_; StringBox stringBox_;
 int on_key_click(int,int);
 void release_modal() { ++modal_calls; events.push_back(5); }
};
static DiploWin *active;
static void message_chat(int code,int target,char *text) {
 assert(code==0x1E00 && target==7 && std::strcmp(text,"before")==0);
 events.push_back(1);
 // Original4425F5 samples console after this callback, including mutations.
 ConsoleGlobal.faction_id_=5; std::strcpy(active->editBox_.text_,"after");
}
static void CharUpperA(char *text) {
 for(;*text;++text) if(*text>='a' && *text<='z') *text-=32;
}
PRODUCT
int main() {
 DiploWin w; active=&w; w.faction_id_with_=7; ConsoleGlobal.faction_id_=2;
 std::strcpy(Players[2].name_leader,"old"); Players[2].is_leader_female=0;
 std::strcpy(Players[5].name_leader,"live"); Players[5].is_leader_female=1;
 std::strcpy(w.editBox_.text_,"before");
 DiploOffersDirty=0; PluralityDefault=13; GenderDefault=9;
 assert(w.on_key_click(11,13)==1);
 assert(events.size()==3 && events[0]==1 && events[1]==2 && events[2]==3);
 assert(transcript=="LIVE:  after" && transcript_faction==5);
 assert(DiploOffersDirty==1 && w.editBox_.text_[0]==0 && modal_calls==0);
 events.clear(); assert(w.on_key_click(19,13)==1);
 assert(events.size()==1 && events[0]==5 && modal_calls==1);
 events.clear(); assert(w.on_key_click(29,27)==1);
 assert(events.size()==1 && events[0]==5 && modal_calls==2);
 events.clear(); assert(w.on_key_click(-3,42)==1);
 assert(events.size()==1 && events[0]==4 && repeat_seen==-3 && key_seen==42);
}
'''
@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'faction_before_callback', 'dirty_after_clear', 'empty_guard'])
def test_chat_actual_source_callback_and_order(tmp_path, compiler, damage):
 path=shutil.which(compiler)
 assert path, f'required host compiler unavailable: {compiler}'
 source=extract_function((ROOT/'src/diplowin.cpp').read_text(), 'int DiploWin::on_key_click(int repeat, int key)')
 # Comments do not participate in the independent operation-order mutant.
 source='\n'.join(line for line in source.splitlines() if not line.lstrip().startswith('//'))
 if damage=='faction_before_callback':
  old='            message_chat(0x1E00, faction_id_with_, text);\n            const int faction = ConsoleGlobal.faction_id_;'
  new='            const int faction = ConsoleGlobal.faction_id_;\n            message_chat(0x1E00, faction_id_with_, text);'
 elif damage=='dirty_after_clear':
  old='            DiploOffersDirty = 1;\n            editBox_.set_text(0);'
  new='            editBox_.set_text(0);\n            DiploOffersDirty = 1;'
 elif damage=='empty_guard':
  old='text != 0 && strlen(text) != 0'; new='text != 0'
 if damage:
  assert source.count(old)==1, 'meaningful damage anchor changed'
  source=source.replace(old,new,1)
 cpp,exe=tmp_path/'chat.cpp',tmp_path/'chat'
 cpp.write_text(HARNESS.replace('PRODUCT',source))
 built=subprocess.run([path,'-std=c++98','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],capture_output=True,text=True)
 assert built.returncode==0, built.stdout+built.stderr
 result=subprocess.run([str(exe)],capture_output=True,text=True)
 assert (result.returncode==0)==(damage is None),result.stdout+result.stderr
