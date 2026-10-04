"""Actual recovered menu body with service doubles, not GUI/game acceptance."""
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_diplomacy_menu(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    helper = definition(source, r'void read_diplomacy_menu_line\(\)')
    body = definition(source, r'int __cdecl diplomacy_menu\([^)]*\)')
    fixture = r'''
#include <cassert>
#include <cstring>
#define __cdecl
#define nullptr 0
struct Sprite {} portraits[8];
Sprite *FactionDiplomacyPortraits[8];
struct Player { int is_leader_female, rule_flags; char name_leader[24]; } Players[8];
struct Data { unsigned int diplo_treaties[8]; } PlayersData[8];
int PopupDialogFactionID, PluralityDefault, GenderDefault;
int DiploScratchFAD0, DiploScratchFAD4, DiploScratchFA94, DiploScratchF7DC;
int IsMultiplayerNet;
char StringTemp[80], PopupStartCaption[80], line[]="line", script[]="script";
char *TextBufferGetPtr=line, *ScriptPopupFile=script;
int lines, parses, closes, destructors, executions, open_result, council, picture;
int ids[16], id_count, mutation;
char *text_get() { ++lines; TextBufferGetPtr=line; return line; }
int parse_string(const char *s,char *d) {
  assert(s==TextBufferGetPtr && d==StringTemp && !d[0]);
  std::strcpy(d,s); ++parses;
  if (mutation && lines==4) DiploScratchFAD4=1;
  return 0;
}
int parse_says(int n,const char *s,int a,int b) {
  assert(a==-1 && b==-1);
  if (!n) { assert(s==Players[1].name_leader); assert(!PluralityDefault && GenderDefault==1); }
  else assert(n==1 && !std::strcmp(s,"pact"));
  return 0;
}
char *get_pact_hood(int a,int b) { assert(a==1 && b==2); static char s[]="pact"; return s; }
void diplomacy_caption(int a,int b) { assert(a==1 && b==2 && PopupDialogFactionID==2); }
int X_text_open(char *s,const char *section) { assert(s==script && !std::strcmp(section,"DIPLOMENU")); return open_result; }
void text_close() { assert(lines==9 && parses==9); ++closes; }
int can_call_council(int a,int b) { assert(a==1 && b==0); return council; }
int pop_wait() { return 0; }
struct Dialogs {
 void sprite(Sprite *s,char *t,int n) { assert(s==FactionDiplomacyPortraits[2] && !t && !n); }
 void item(char *s,int id) { assert(s==StringTemp && !std::strcmp(s,"line")); ids[id_count++]=id; }
};
struct Popup {
 Dialogs menu;
 ~Popup() { ++destructors; }
 void start(char *s,const char *label,int n,char *t,int flags) {
  assert(s==PopupStartCaption && !std::strcmp(label,"DIPLO") && n==-1 && !t && flags==0x100000);
 }
 bool picture_enabled() { return picture!=0; }
 void set_caller_picture(Sprite *s) { assert(s==(picture ? 0 : FactionDiplomacyPortraits[2])); }
 Dialogs &dialogs() { return menu; }
 int exec(int n,int(*callback)()) { assert(!n && callback==pop_wait && closes==1); ++executions; return 23; }
};
@@BODIES@@
void reset() {
 lines=parses=closes=destructors=executions=id_count=mutation=0;
 open_result=council=picture=IsMultiplayerNet=0;
 DiploScratchFAD0=DiploScratchFAD4=DiploScratchFA94=DiploScratchF7DC=0;
 std::memset(PlayersData,0,sizeof(PlayersData)); std::memset(Players,0,sizeof(Players));
 Players[1].is_leader_female=1;
 for(int i=0;i<8;++i) FactionDiplomacyPortraits[i]=&portraits[i];
 StringTemp[0]='x';
}
void expect(const int *values,int count) {
 assert(id_count==count); for(int i=0;i<count;++i) assert(ids[i]==values[i]);
 assert(lines==9 && parses==9 && closes==1 && destructors==1 && executions==1);
}
int main() {
 reset(); open_result=1; assert(diplomacy_menu(1,2)==0);
 assert(!lines && !closes && !executions && destructors==1);
 reset(); assert(diplomacy_menu(1,2)==23); const int basic[]={0,2}; expect(basic,2);
 reset(); picture=1; council=1; DiploScratchFAD0=DiploScratchFAD4=DiploScratchFA94=DiploScratchF7DC=1;
 PlayersData[2].diplo_treaties[1]=1; PlayersData[1].diplo_treaties[2]=7;
 assert(diplomacy_menu(1,2)==23); const int all[]={0,1,2,5,6,7,3,4}; expect(all,8);
 reset(); IsMultiplayerNet=1; council=1; DiploScratchFA94=DiploScratchF7DC=1;
 Players[2].rule_flags=0x80; PlayersData[1].diplo_treaties[2]=1;
 assert(diplomacy_menu(1,2)==23); const int net[]={0,2,4}; expect(net,3);
 reset(); mutation=1; assert(diplomacy_menu(1,2)==23); const int reread[]={0,2,2}; expect(reread,3);
}
'''
    variants = {'actual': body}
    replacements = {
        'wrong_forward_pair': ('PlayersData[faction_id_with].diplo_treaties[faction_id] & 1', 'PlayersData[faction_id].diplo_treaties[faction_id_with] & 1'),
        'wrong_reverse_pair': ('PlayersData[faction_id].diplo_treaties[faction_id_with] & 7', 'PlayersData[faction_id_with].diplo_treaties[faction_id] & 7'),
        'wrong_positive_gate': ('DiploScratchFA94 > 0', 'DiploScratchFA94 < 0'),
        'wrong_mode_line': ('DiploScratchFAD4 != 0', 'DiploScratchFAD4 == 0'),
        'lost_network_gate': ('IsMultiplayerNet == 0 && DiploScratchF7DC > 0', 'DiploScratchF7DC > 0'),
        'lost_alien_gate': ('(Players[faction_id_with].rule_flags & 0x80) == 0', 'true'),
        'lost_text_close': ('text_close();', '(void)0;'),
    }
    for name,(old,new) in replacements.items():
        assert body.count(old)==1
        variants[name]=body.replace(old,new)
    for name,candidate in variants.items():
        cpp,binary=tmp_path/(name+'.cpp'),tmp_path/name
        cpp.write_text(fixture.replace('@@BODIES@@',helper+'\n'+candidate))
        build=subprocess.run([compiler,'-std=c++98','-fsanitize=undefined','-fno-sanitize-recover=all',str(cpp),'-o',str(binary)],capture_output=True,text=True,timeout=45)
        assert build.returncode==0,build.stderr
        run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
        assert (run.returncode==0)==(name=='actual'),name+run.stderr
