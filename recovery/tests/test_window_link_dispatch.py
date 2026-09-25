"""Whole actual Win::on_l_button_down, after coordinator's original-backed fix.

Self-contained host control for the recovered hyperlink path. No product
body rewriting: both complete click and markup writer are extracted unchanged.
Original5F63C0..5F654E: argument1 gates double-click, argument4 unused,
argument5 chooses capture/slot28 versus38 and is forwarded to fallback.

Bring-to-top/z-order/parent focus/Spot/font/modal services remain doubles.
Related-window integer fields val17/26 are zero (their nonzero 32-bit object
address branch is not claimed). The actual 32-bit function-pointer hook is
exercised with a non-PIE host function whose address is checked representable;
this is not portable callback ABI/native layout/full GUI proof. Owner store
order has no intervening observable callback and is not independently proved.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if not depth:
            return source[start:end + 1]
    raise AssertionError('unterminated source')


def actual_source():
    header = {n: (ROOT / 'src' / (n + '.h')).read_text() for n in
              ('win', 'graphicwin', 'stringbox', 'newtechwin', 'buffer', 'font', 'game')}
    body = {n: (ROOT / 'src' / (n + '.cpp')).read_text() for n in
            ('win', 'stringbox', 'newtechwin', 'buffer')}
    out = {}
    for n, cls in [('win', 'Win'), ('graphicwin', 'GraphicWin'),
                   ('stringbox', 'StringBox'), ('newtechwin', 'NewTechWin')]:
        out[n.upper() + '_BASE'], = re.findall(r'^class ' + cls + r'\s*:[^{]+\{', header[n], re.M)
    for n in ('stringbox', 'newtechwin'):
        out[n.upper() + '_DECL'], = re.findall(r'^  virtual int on_link_clicked\([^;]+;', header[n], re.M)
    out['LINK_DEFAULT'] = braced(header['graphicwin'], 'virtual int on_link_clicked(')
    out['PARENT_DECL'], = re.findall(r'^  Win \*win_parent_;', header['win'], re.M)
    out['WRITER_DECL'], = re.findall(r'^  int write_multi_font_raw_l\([^;]+;', header['buffer'], re.M)
    out['BUFFER_FIELDS'] = '\n'.join(re.findall(r'^  (?:uint32_t (?:link_count_|write_font_slot_|markup_pending_|markup_enabled_)|Spot spot_|void \*owned_\[20\]|Font \*font[14]_);', header['buffer'], re.M))
    assert len(out['BUFFER_FIELDS'].splitlines()) == 8
    out['FONT_FIELDS'] = '\n'.join(re.findall(r'^  int (?:unk_1_|height_|line_height_);', header['font'], re.M))
    assert len(out['FONT_FIELDS'].splitlines()) == 3
    out['NETWORK_DECL'], = re.findall(r'^extern BOOL IsMultiplayerNet;', header['game'], re.M)
    out['NETWORK_DECL'] = out['NETWORK_DECL'].replace('extern ', '')
    out['PENDING_DEF'], = re.findall(r'^static int PendingTechnologyLink = -1;', body['newtechwin'], re.M)
    for name, unit, sig in [('STRING_BODY','stringbox','int StringBox::on_link_clicked('),
                            ('TECH_BODY','newtechwin','int NewTechWin::on_link_clicked('),
                            ('WRITER_BODY','buffer','int Buffer::write_multi_font_raw_l(')]:
        out[name] = braced(body[unit], sig)
    complete = braced(body['win'], 'void Win::on_l_button_down(')
    suffix = complete[complete.index('    GraphicWin *hitTest ='):]
    out['CLICK_BODY'] = 'void Win::click_suffix(int x,int y,int from_parent) {\n' + suffix
    return out


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <functional>
typedef char *LPSTR;typedef int BOOL;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion %d\n",line);std::_Exit(109);}}
#define require(x) ::check((x),__LINE__)
struct Font { FONT_FIELDS };
struct Region {int id,type,x,y,w,h;};
struct Spot {
 std::vector<Region> regions;int result=0,index=0,checks=0,last_x=0,last_y=0;
 std::function<void()> on_check;
 int add(int id,int type,int x,int y,int w,int h){regions.push_back({id,type,x,y,w,h});return 0;}
 int check(int x,int y,int *id,int *type){require(type==nullptr);++checks;last_x=x;last_y=y;if(on_check)on_check();*id=index;return result;}
};
int warnings=0,allocations=0,raw_calls=0;
int MessageBoxA(void*,const char*,const char*,int){++warnings;return 0;}
void *mem_get(int count){require(count>0);++allocations;return std::malloc(count);}
struct AutoSound {virtual ~AutoSound(){}};
class GraphicWin;
class Buffer {
public:
 BUFFER_FIELDS
 WRITER_DECL
 Buffer():link_count_(0),write_font_slot_(0),markup_pending_(0),markup_enabled_(0),font1_(nullptr),font4_(nullptr){for(auto &p:owned_)p=nullptr;}
 ~Buffer(){for(auto p:owned_)std::free(p);}
 int write_raw_l(char *,int x,int,int count){require(count>=0);++raw_calls;return x+count;}
};
WIN_BASE public:
 PARENT_DECL
 Win():win_parent_(nullptr){}
 virtual GraphicWin *get_graphic_win(){return nullptr;}
 virtual void release_modal(){}
 int fallback_count=0,fall_x=0,fall_y=0,fall_dbl=0;
 void left_down_event(int x,int y,int dbl){++fallback_count;fall_x=x;fall_y=y;fall_dbl=dbl;}
 void click_suffix(int,int,int);
};
GRAPHICWIN_BASE public:
 LINK_DEFAULT
 GraphicWin *get_graphic_win() override{return this;}
};
STRINGBOX_BASE public: STRINGBOX_DECL };
NEWTECHWIN_BASE public: NEWTECHWIN_DECL };
NETWORK_DECL
PENDING_DEF
STRING_BODY
TECH_BODY
CLICK_BODY
WRITER_BODY
struct Receiver:GraphicWin {
 GraphicWin *answer=this;int queries=0,calls=0,last_id=0,return_value=0;char *last_text=nullptr;
 std::function<void()> query_action,link_action;
 GraphicWin *get_graphic_win() override{++queries;if(query_action)query_action();return answer;}
 int on_link_clicked(int id,char *text) override{++calls;last_id=id;last_text=text;if(link_action)link_action();return return_value;}
};
struct Technology:NewTechWin {int releases=0,observed=0;void release_modal()override{++releases;observed=PendingTechnologyLink;PendingTechnologyLink=99;}};
void string_tests(){
 StringBox box;Receiver first,second,unrelated;char text[]="route";
 require(box.on_link_clicked(4,text)==1);
 box.win_parent_=&first;first.answer=nullptr;require(box.on_link_clicked(4,text)==1&&first.queries==1&&first.calls==0);
 first.answer=&unrelated;first.return_value=-7;
 require(box.on_link_clicked(4,text)==-7&&first.calls==1&&unrelated.calls==0&&first.last_id==4&&first.last_text==text);
 first.query_action=[&]{box.win_parent_=&second;};second.return_value=83;
 require(box.on_link_clicked(-2,nullptr)==83&&second.calls==1&&second.last_id==-2&&second.last_text==nullptr&&first.calls==1);
 require(unrelated.queries==0&&second.queries==0);
}
void click_tests(){
 char old_text[]="old",fresh[]="fresh";
 for(int enabled: {0,1})for(int hit: {-1,0,4})for(int verdict: {-3,0,6}){
  Receiver own,unrelated;own.answer=enabled?&unrelated:nullptr;own.return_value=verdict;
  own.spot_.result=hit;own.spot_.index=2;
  // Owned allocations remain real and distinct; none of these synthetic cells
  // are freed twice or formed outside the admitted original twenty-slot table.
  for(int i=0;i<4;++i){own.owned_[i]=std::malloc(16);std::strcpy(static_cast<char*>(own.owned_[i]),old_text);}
  own.spot_.on_check=[&]{std::strcpy(static_cast<char*>(own.owned_[2]),fresh);};
  own.click_suffix(11,-4,9);
  bool dispatch=enabled&&hit>=0;
  require(own.queries==1&&unrelated.queries==0&&unrelated.calls==0&&unrelated.spot_.checks==0);
  require(own.spot_.checks==enabled&&own.calls==int(dispatch));
  require(own.fallback_count==int(!dispatch||verdict!=0));
  if(dispatch)require(own.last_id==2&&own.last_text==own.owned_[2]&&std::strcmp(own.last_text,"fresh")==0);
  if(enabled)require(own.spot_.last_x==11&&own.spot_.last_y==-4);
  if(own.fallback_count)require(own.fall_x==11&&own.fall_y==-4&&own.fall_dbl==9);
 }
}
void tech_tests(){
 require(PendingTechnologyLink==-1);
 for(int network: {-1,0,3})for(int has_text: {0,1}){
  Technology t;IsMultiplayerNet=network;PendingTechnologyLink=-31;char text[]=" -27junk";
  int result=t.on_link_clicked(123,has_text?text:nullptr);bool admitted=network==0&&has_text;
  require(result==(admitted?0:1)&&t.releases==int(admitted));
  if(admitted)require(t.observed==-27&&PendingTechnologyLink==99);else require(PendingTechnologyLink==-31);
 }
 Technology t;IsMultiplayerNet=0;char invalid[]="nonnumeric";require(t.on_link_clicked(-19,invalid)==0&&t.observed==0);
}
void markup_tests(){
 Font regular{},link{};regular.unk_1_=-1;regular.height_=5;regular.line_height_=7;
 link.unk_1_=2;link.height_=11;link.line_height_=20;
 for(int font_mode: {0,1,2}){
  Receiver own;own.markup_enabled_=1;own.font1_=&regular;own.font4_=font_mode==0?nullptr:&link;
  if(font_mode==2)link.unk_1_=-1;else link.unk_1_=2;
  char text[]="$LINK<A=17>/$LINK<BC=-4>";
  int old_alloc=allocations;require(own.write_multi_font_raw_l(text,10,30,std::strlen(text))==14);
  require(own.link_count_==2&&allocations-old_alloc==2&&own.spot_.regions.size()==2);
  require(own.owned_[0]&&own.owned_[1]&&!own.owned_[2]);
  require(std::strcmp(static_cast<char*>(own.owned_[0]),"17")==0&&std::strcmp(static_cast<char*>(own.owned_[1]),"-4")==0);
  int height=font_mode==0?7:font_mode==1?13:20;
  const Region &a=own.spot_.regions[0],&b=own.spot_.regions[1];
  require(a.id==0&&a.type==0&&a.x==10&&a.y==30&&a.w==1&&a.h==height);
  require(b.id==1&&b.type==0&&b.x==12&&b.y==30&&b.w==2&&b.h==height);
  for(int id=0;id<2;++id){own.spot_.index=own.spot_.regions[id].id;own.click_suffix(0,0,0);require(own.last_id==id&&own.last_text==own.owned_[id]);}
 }
 Receiver plain;char raw[]="$LINK<X=9>";int before=allocations;require(plain.write_multi_font_raw_l(raw,3,4,10)==13&&allocations==before&&plain.spot_.regions.empty());
 require(warnings==0);
}
int main(){string_tests();click_tests();tech_tests();markup_tests();}
'''


MUTANTS = [
 ('cached_parent','STRING_BODY','static_cast<GraphicWin *>(win_parent_)','static_cast<GraphicWin *>(parent)'),
 ('getter_receiver','STRING_BODY','static_cast<GraphicWin *>(win_parent_)','parent->get_graphic_win()'),
 ('lost_return','STRING_BODY','return static_cast<GraphicWin *>(win_parent_)->on_link_clicked(id, link);','static_cast<GraphicWin *>(win_parent_)->on_link_clicked(id, link); return 0;'),
 ('wrong_id','STRING_BODY','on_link_clicked(id, link)','on_link_clicked(id + 1, link)'),
 ('wrong_text','STRING_BODY','on_link_clicked(id, link)','on_link_clicked(id, 0)'),
 ('rejected_value','STRING_BODY','return 1;','return 0;'),
 ('own_receiver','CLICK_BODY','static_cast<GraphicWin *>(this)->on_link_clicked','hitTest->on_link_clicked'),
 ('own_spot','CLICK_BODY','static_cast<GraphicWin *>(this)->Buffer::spot_','hitTest->Buffer::spot_'),
 ('own_text','CLICK_BODY','Buffer::owned_[idx]','Buffer::owned_[idx + 1]'),
 ('hit_boundary','CLICK_BODY','check(x, y, &idx, 0) < 0','check(x, y, &idx, 0) <= 0'),
 ('callback_result','CLICK_BODY','])) != 0','])) == 0'),
 ('fallback_args','CLICK_BODY','left_down_event(x, y, from_parent)','left_down_event(y, x, from_parent)'),
 ('network_gate','TECH_BODY','IsMultiplayerNet == 0','IsMultiplayerNet != 0'),
 ('success_return','TECH_BODY','return 0;','return 1;'),
 ('rejection_return','TECH_BODY','return 1;','return 0;'),
 ('modal_order','TECH_BODY','PendingTechnologyLink = atoi(link);\n        release_modal();','release_modal();\n        PendingTechnologyLink = atoi(link);'),
 ('no_modal','TECH_BODY','release_modal();','/* omitted */'),
 ('wrong_parse','TECH_BODY','atoi(link)','17'),
 ('producer_index','WRITER_BODY','owned_[link_count_ - 1] = kept;','owned_[link_count_] = kept;'),
 ('spot_index','WRITER_BODY','spot_.add(link_count_ - 1,','spot_.add(link_count_,'),
 ('font_fallback','WRITER_BODY','font4_ ? font4_ : font1_','font1_'),
 ('font_negative','WRITER_BODY','link_font->line_height_','link_font->height_'),
 ('font_positive','WRITER_BODY','link_font->height_ + link_font->unk_1_','link_font->height_'),
 ('markup_off','WRITER_BODY','if (!markup_enabled_)','if (markup_enabled_)'),
]


def run_case(tmp_path, mutation=None):
    compiler=shutil.which('clang++') or shutil.which('g++')
    if not compiler: pytest.skip('host C++ compiler unavailable')
    parts=actual_source()
    if mutation:
        _,key,old,new=mutation
        assert old in parts[key], (key,old)
        parts[key]=parts[key].replace(old,new)
    source=HARNESS
    for key in sorted(parts,key=len,reverse=True): source=source.replace(key,parts[key])
    unit=tmp_path/'link.cpp';unit.write_text(source);exe=tmp_path/'link'
    compile_result=subprocess.run([compiler,'-std=c++17','-O1','-g','-fsanitize=address,undefined',
        '-fno-sanitize-recover=all','-fno-omit-frame-pointer',str(unit),'-o',str(exe)],capture_output=True,text=True)
    assert compile_result.returncode==0,compile_result.stderr
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    result=subprocess.run([str(exe)],capture_output=True,text=True,env=env)
    assert result.returncode==(109 if mutation else 0),(result.returncode,result.stdout,result.stderr)
    if mutation: assert 'assertion ' in result.stderr
    assert 'AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr


def test_actual_link_flow(tmp_path): run_case(tmp_path)


@pytest.mark.parametrize('mutation',MUTANTS,ids=[m[0] for m in MUTANTS])
def test_compiling_damage_controls(tmp_path,mutation): run_case(tmp_path,mutation)


def actual_parts():
    parts = actual_source()
    # Adapt only the synthetic suffix wrapper parameter to the genuine renamed
    # signature; its extracted statement sequence is unchanged.
    parts['CLICK_BODY'] = parts['CLICK_BODY'].replace('int dbl) {', 'int from_parent) {', 1)
    header = (ROOT/'src/win.h').read_text()
    autosound = (ROOT/'src/autosound.h').read_text()
    source = (ROOT/'src/win.cpp').read_text()
    parts['WHOLE_BODY'] = braced(source, 'void Win::on_l_button_down(')
    parts['WHOLE_DECL'], = re.findall(r'^  virtual void on_l_button_down\([^;]+;', header, re.M)
    parts['WINDOW_FIELDS'] = '\n'.join(re.findall(r'^  uint32_t (?:iFlags_|iSomeFlag_|field_40C_);', header, re.M))
    assert len(parts['WINDOW_FIELDS'].splitlines()) == 3
    parts['RELATED_FIELDS'] = '\n'.join(re.findall(r'^  int val_(?:17|26)_;', autosound, re.M))
    assert len(parts['RELATED_FIELDS'].splitlines()) == 2
    parts['PROCESS_FIELDS'] = '\n'.join(re.findall(r'^extern (?:Win \*(?:WinPointerOwner1|WinPointerOwner2|WinCallbackWindow)|int (?:WinSizingFlag|WinRootCount));', header, re.M)).replace('extern ', '')
    assert len(parts['PROCESS_FIELDS'].splitlines()) == 5
    return parts


EXTRA_DECLS = r'''
 WINDOW_FIELDS
 RELATED_FIELDS
 WHOLE_DECL
 int tag=0;
 std::function<void(Win*)> top_action,focus_action;
 std::function<void()> order_action;
 std::function<void(int,int)> double_action,other_action;
 void bring_child_to_top(Win*);
 virtual void set_dialog_focus(Win*);
 void update_zorder();
 virtual void on_left_double_click(int,int);
 virtual void vslot_38(int,int);
 virtual int vslot_23(){require(false);return 0;}
 virtual void vslot_07(){require(false);}
'''

EXTRA_BODIES = r'''
struct Event {int kind,receiver,x,y; bool operator==(const Event&o)const{return kind==o.kind&&receiver==o.receiver&&x==o.x&&y==o.y;}};
std::vector<Event> events;
PROCESS_FIELDS
Win *WinRootWindows[8];
void Win::bring_child_to_top(Win*target){events.push_back({1,tag,target->tag,0});if(top_action)top_action(target);}
void Win::set_dialog_focus(Win*target){events.push_back({2,tag,target->tag,0});if(focus_action)focus_action(target);}
void Win::update_zorder(){events.push_back({3,tag,0,0});if(order_action)order_action();}
void Win::on_left_double_click(int x,int y){events.push_back({28,tag,x,y});if(double_action)double_action(x,y);}
void Win::vslot_38(int x,int y){events.push_back({38,tag,x,y});if(other_action)other_action(x,y);}
WHOLE_BODY
std::function<void(int,int)> hook_action;
void hook(int x,int y){require(WinCallbackWindow!=nullptr);events.push_back({100,WinCallbackWindow->tag,x,y});if(hook_action)hook_action(x,y);}
void reset(Win&w){
 events.clear();hook_action=nullptr;w.iFlags_=0;w.iSomeFlag_=0;w.field_40C_=0;w.val_17_=w.val_26_=0;
 w.tag=10;w.win_parent_=nullptr;w.fallback_count=0;
 w.top_action=nullptr;w.focus_action=nullptr;w.order_action=nullptr;w.double_action=nullptr;w.other_action=nullptr;
 WinRootCount=0;for(auto&p:WinRootWindows)p=nullptr;WinPointerOwner1=WinPointerOwner2=WinCallbackWindow=nullptr;WinSizingFlag=-9;
}
void call(Win&w,long twice,int x,int y,unsigned unused,int parent){w.on_l_button_down(twice,x,y,unused,parent);}
void routing_cases(){
 for(long twice:{0L,1L,-1L})for(unsigned unused:{0U,1U,0x80000000U})for(int parent:{0,1,-7}){
  Win w;reset(w);w.iFlags_=0x2000000;call(w,twice,31,-8,unused,parent);
  require(WinPointerOwner1==(parent==0?&w:nullptr)&&WinPointerOwner2==(parent==0?nullptr:&w));
  require(WinSizingFlag==1&&w.fallback_count==1&&w.fall_x==31&&w.fall_y==-8&&w.fall_dbl==parent);
  if(twice)require(events==std::vector<Event>({{parent==0?28:38,10,31,-8}}));else require(events.empty());
  require(WinCallbackWindow==((twice&&parent==0)?&w:nullptr));
 }
 for(uint32_t gate:{0U,0x200U,0x200000U})for(uint32_t state:{0U,8U}){
  Win w;reset(w);w.iFlags_=0x2000000|gate;w.iSomeFlag_=state;
  call(w,1,2,9,123,0);require(events.size()==((gate==0&&state==0)?1U:0U));
 }
}
void root_and_parent_cases(){
 for(int count:{-3,0,1,4})for(int position:{-1,0,2}){
  Win w,a,b,c,d;reset(w);WinRootCount=count;WinRootWindows[0]=&a;WinRootWindows[1]=&b;WinRootWindows[2]=&c;WinRootWindows[3]=&d;
  if(position>=0&&position<count)WinRootWindows[position]=&w;
  std::vector<Win*> expected(WinRootWindows,WinRootWindows+4);
  if(position>=0&&position<count){for(int i=position;i>0;--i)expected[i]=expected[i-1];expected[0]=&w;}
  call(w,0,4,5,0,0);require(events==std::vector<Event>({{3,10,0,0}}));
  for(int i=0;i<4;++i)require(WinRootWindows[i]==expected[i]);
 }
 Win w,first,replacement;reset(w);first.tag=21;replacement.tag=22;w.win_parent_=&first;
 first.top_action=[&](Win*target){require(target==&w);w.win_parent_=&replacement;};
 replacement.focus_action=[&](Win*target){require(target==&w);require(!WinPointerOwner1&&!WinPointerOwner2&&WinSizingFlag==-9);w.iFlags_|=0x200;};
 call(w,1,3,5,0,0);require(events==std::vector<Event>({{1,21,10,0},{2,22,10,0}}));
 reset(w);w.win_parent_=&first;first.top_action=[&](Win*){w.win_parent_=nullptr;};call(w,0,3,5,0,0);
 require(events==std::vector<Event>({{1,21,10,0}}));
 reset(w);w.order_action=[&]{w.win_parent_=&replacement;};replacement.focus_action=nullptr;
 call(w,0,3,5,0,0);require(events==std::vector<Event>({{3,10,0,0},{2,22,10,0}}));
 reset(w);w.iFlags_=0x2000000;call(w,0,3,5,0,0);require(events.empty());
}
void hook_cases(){
 auto address=reinterpret_cast<uintptr_t>(&hook);require(address<=INT32_MAX);
 for(int parent:{0,1}){
  Win w;reset(w);w.iFlags_=0x2000000;w.field_40C_=static_cast<uint32_t>(address);
  hook_action=[&](int x,int y){require(x==17&&y==-4&&WinCallbackWindow==&w&&WinPointerOwner1==&w&&WinSizingFlag==1);
    w.iFlags_|=0x200;w.iSomeFlag_|=8;w.field_40C_=0;};
  call(w,1,17,-4,7,parent);
  if(parent==0)require(events==std::vector<Event>({{100,10,17,-4},{28,10,17,-4}}));
  else require(events==std::vector<Event>({{38,10,17,-4}}));
 }
}
void whole_link_case(){
 Receiver own,unrelated;reset(own);own.answer=&unrelated;own.iFlags_=0x2000000;
 Font font{};font.unk_1_=-1;font.line_height_=6;own.font1_=&font;own.markup_enabled_=1;
 char markup[]="$LINK<A=29>";require(own.write_multi_font_raw_l(markup,1,2,std::strlen(markup))==2);
 require(own.spot_.regions.size()==1&&own.spot_.regions[0].id==0&&std::strcmp(static_cast<char*>(own.owned_[0]),"29")==0);
 own.spot_.index=0;own.return_value=0;
 own.double_action=[&](int,int){require(WinPointerOwner1==&own);own.spot_.result=0;};
 own.spot_.on_check=[&]{void*replacement=std::malloc(8);std::strcpy(static_cast<char*>(replacement),"fresh");std::free(own.owned_[0]);own.owned_[0]=replacement;};
 call(own,1,9,-6,99,0);require(events==std::vector<Event>({{28,10,9,-6}}));
 require(own.calls==1&&own.last_id==0&&own.last_text==own.owned_[0]&&std::strcmp(own.last_text,"fresh")==0);
 require(own.fallback_count==0&&unrelated.calls==0&&unrelated.spot_.checks==0);
 own.return_value=-4;call(own,0,7,8,0,-2);require(own.fallback_count==1&&own.fall_dbl==-2);
}
'''


def build(tmp_path, mutation=None):
    parts=actual_parts()
    if mutation:
        changes=mutation if isinstance(mutation,list) else [mutation]
        for before,after in changes:
            assert before in parts['WHOLE_BODY'],before
            parts['WHOLE_BODY']=parts['WHOLE_BODY'].replace(before,after)
    harness=HARNESS.replace('#include <cstdint>', '#include <cstdint>\n#define __cdecl')
    harness=harness.replace(' void click_suffix(int,int,int);', ' void click_suffix(int,int,int);\n'+EXTRA_DECLS)
    harness=harness.replace('int main(){string_tests();click_tests();tech_tests();markup_tests();}',
                            EXTRA_BODIES+'\nint main(){routing_cases();root_and_parent_cases();hook_cases();whole_link_case();}')
    for key in sorted(parts,key=len,reverse=True):harness=harness.replace(key,parts[key])
    unit=tmp_path/'whole_click.cpp';unit.write_text(harness);exe=tmp_path/'whole_click'
    compiler=shutil.which('clang++') or shutil.which('g++')
    if not compiler:pytest.skip('host C++ compiler required')
    result=subprocess.run([compiler,'-std=c++17','-O1','-g','-fno-pie','-no-pie',
      '-fsanitize=address,undefined','-fno-sanitize-recover=all',str(unit),'-o',str(exe)],capture_output=True,text=True,timeout=45)
    assert result.returncode==0,result.stderr
    result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,
      env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
    assert result.returncode==(109 if mutation else 0),(result.returncode,result.stderr)
    if mutation:assert 'assertion ' in result.stderr
    assert 'AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr


def test_whole_actual_click(tmp_path):build(tmp_path)


WHOLE_MUTATIONS = {
 'fourth_not_fifth': ('unsigned int, int from_parent)', 'unsigned int keys, int from_parent)'),
 'invert_capture_and_dispatch': ('if (from_parent == 0)', 'if (from_parent != 0)'),
 'first_argument_gate': ('if (double_click != 0 &&', 'if (from_parent != 0 &&'),
 'left_xy': ('on_left_double_click(x, y);', 'on_left_double_click(y, x);'),
 'other_xy': ('vslot_38(x, y);', 'vslot_38(y, x);'),
 'parent_reload': ('\n    child = win_parent_;', '\n    /* stale child */'),
 'root_raise_disabled': ('(iFlags_ & 0x2000000) == 0', '(iFlags_ & 0x2000000) != 0'),
 'root_shift': ('*q = *(q - 1);', '*q = this;'),
 'no_zorder': ('this->update_zorder();', '/* missing zorder */'),
 'no_focus': ('reinterpret_cast<Win *>(child)->set_dialog_focus(this);', '/* missing focus */'),
 'wrong_capture': ('WinPointerOwner1 = this;', 'WinPointerOwner2 = this;'),
 'no_sizing': ('WinSizingFlag = 1;', 'WinSizingFlag = 0;'),
 'missing_flag_bit': ('iFlags_ & 0x200200', 'iFlags_ & 0x200'),
 'wrong_state_bit': ('iSomeFlag_ & 8', 'iSomeFlag_ & 4'),
 'no_callback_owner': ('WinCallbackWindow = this;', '/* missing publication */'),
 'no_callback': ('reinterpret_cast<void(__cdecl *)(int, int)>(fp)(x, y);', '/* missing callback */'),
 'regate_after_callback': ('this->on_left_double_click(x, y);', 'if ((iFlags_ & 0x200200)==0) this->on_left_double_click(x, y);'),
}


@pytest.mark.parametrize('name',tuple(WHOLE_MUTATIONS))
def test_whole_compiling_damage(tmp_path,name):
    mutation=WHOLE_MUTATIONS[name]
    if name=='fourth_not_fifth':
        # Naming the genuinely unused fourth parameter is part of this damage.
        build(tmp_path,[mutation,('if (from_parent == 0)', 'if (keys == 0)')])
    else:build(tmp_path,mutation)
