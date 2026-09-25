"""Actual DiploPop lifetime and service controls with explicit host doubles.

Original43EFF0/440D50 and their unwind regions establish three Fonts, two
Times, two Flics, three FlatButtons, then the two constructor-body stores.
43F0F0/43F140/43F170/43F290/43F5C0 establish service order and typed dispatch.
The complete source member run, inheritance clause, relevant declarations,
method bodies and global owner are extracted unchanged. Host service sizes,
constructors and destructors are instruments, not native ABI/lifetime proof
for Font/Time/Flic/FlatButton/GraphicWin internals or the registry base.

Every observed member-constructor boundary can throw; only completed objects
unwind. Throwing destructors/service calls during teardown are not modelled.
Callback mutations are boundary stress controls, not claimed Win32 callbacks.
Poison is inspected as raw bytes, never read as an uninitialized scalar.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated actual source')


def one(text, pattern):
    found, = re.findall(pattern, text, re.M)
    return found


def actual():
    header = (ROOT / 'src/diplopop.h').read_text()
    source = (ROOT / 'src/diplopop.cpp').read_text()
    win = (ROOT / 'src/win.h').read_text()
    graphic = (ROOT / 'src/graphicwin.h').read_text()
    members = header[header.index('  Font font1_;'):header.index('\n};', header.index('  Font font1_;'))]
    methods = {name: braced(source, signature) for name, signature in (
        ('ctor', 'DiploPop::DiploPop()'), ('dtor', 'DiploPop::~DiploPop()'),
        ('close', 'void DiploPop::close()'), ('show', 'void DiploPop::show('),
        ('hide', 'void DiploPop::hide()'), ('stop', 'void DiploPop::stop()'),
        ('button', 'void DiploPop::on_button_clicked('))}
    declarations = '\n'.join((
        one(header, r'class DiploPop :[^\n]+\{') + ' public:',
        one(header, r'(?<!~)\bDiploPop\(\);'), one(header, r'~DiploPop\(\);'),
        *[one(header, pattern) for pattern in (
            r'void close\(\);', r'void hide\(\);', r'void stop\(\);',
            r'void show\(int a1\);', r'void on_button_clicked\(int a1\);')],
        members, '};', one(header, r'^extern DiploPop DiplomacyPopup;'),
        one(win, r'^extern Win \*WinModalFocus;')))
    base_declarations = '\n'.join((
        'struct Win {', one(win, r'int is_visible\(\);'),
        one(win, r'virtual void show\(int visible\);'),
        one(win, r'virtual void hide\(\);'),
        braced(win, 'virtual void on_button_clicked('), '};',
        'struct GraphicWin : Win { int id; GraphicWin(); ~GraphicWin();',
        one(graphic, r'uint32_t close\(\);'), '};'))
    owner = one(source, r'^DiploPop DiplomacyPopup;')
    return base_declarations, declarations, owner, methods


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>
#include <functional>
#include <type_traits>
// A fatal oracle failure must not start unrelated global teardown with an
// intentionally incomplete fixture. Normal main return still tests teardown.
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(105);}}
#define require(v) check((v),__LINE__)
struct RECT {int left,top,right,bottom;};
struct Event {char kind;int id;const void *pointer;};
std::vector<Event> events;
std::vector<char> services;
std::function<void(char)> callback;
int next_id=0,throw_at=-1;
bool poison_check=false,teardown_check=false;
unsigned char *construction_storage=nullptr;
void inspect_poison();
void inspect_reset();
int born(char kind,const void *pointer){
  const int id=next_id++;events.push_back({kind,id,pointer});
  if(poison_check)inspect_poison();
  if(id==throw_at)throw id;
  return id;
}
void died(char kind,int id,const void *pointer){
  if(teardown_check&&kind=='B')inspect_reset();
  events.push_back({static_cast<char>(kind+32),id,pointer});
}
void service(char kind){services.push_back(kind);if(callback)callback(kind);}
BASE_DECLARATIONS
struct IfaceRegistryEntry { void release_iface_mode(); };
struct Font {int id;Font():id(born('F',this)){}~Font(){died('F',id,this);}};
struct Time {int id;Time():id(born('T',this)){}~Time(){died('T',id,this);}void stop();};
struct Flic {int id;Flic():id(born('L',this)){}~Flic(){died('L',id,this);}void close();};
struct FlatButton {int id;FlatButton():id(born('B',this)){}~FlatButton(){died('B',id,this);}};
DECLARATIONS
DiploPop *active=nullptr;
Win *WinModalFocus=nullptr;
int visible_result=0;
static_assert(std::is_same<decltype(DiploPop::counterpart_faction_),int>::value,"signed counterpart");
static_assert(std::extent<decltype(DiploPop::flatButtons_)>::value==3,"three buttons");
void inspect_poison(){
  // offsetof on this non-standard-layout host projection is supported by the
  // selected host compilers; these offsets are not native layout assertions.
  const size_t offsets[]={offsetof(DiploPop,counterpart_faction_),offsetof(DiploPop,field_2154_)};
  for(size_t offset:offsets)for(size_t byte=0;byte<4;++byte)require(construction_storage[offset+byte]==0xa5);
}
void inspect_reset(){require(active->counterpart_faction_==-1&&active->field_20B0_==0&&active->field_2154_==1);}
GraphicWin::GraphicWin():id(born('G',this)){}
GraphicWin::~GraphicWin(){died('G',id,this);}
uint32_t GraphicWin::close(){require(this==static_cast<GraphicWin*>(active));service('C');return 19;}
int Win::is_visible(){require(this==static_cast<Win*>(active));service('V');return visible_result;}
void Win::hide(){require(this==static_cast<Win*>(active));service('H');}
void Win::show(int argument){require(this==static_cast<Win*>(active)&&argument==0);service('S');}
void IfaceRegistryEntry::release_iface_mode(){require(this==static_cast<IfaceRegistryEntry*>(active));service('I');}
void Time::stop(){require(this==&active->time1_||this==&active->time2_);service(this==&active->time1_?'1':'2');}
void Flic::close(){require(this==&active->flic1_||this==&active->flic2_);service(this==&active->flic1_?'a':'b');}
METHODS
const char kinds[]={'G','F','F','F','T','T','L','L','B','B','B'};
std::vector<const void*> member_addresses(DiploPop&p){return {
  static_cast<GraphicWin*>(&p),&p.font1_,&p.font2_,&p.font3_,&p.time1_,&p.time2_,
  &p.flic1_,&p.flic2_,&p.flatButtons_[0],&p.flatButtons_[1],&p.flatButtons_[2]};}
void construction_trace(const std::vector<const void*>&addresses,int attempted){
  require(events.size()>=static_cast<size_t>(attempted));
  for(int i=0;i<attempted;++i)require(events[i].kind==kinds[i]&&events[i].id==i&&events[i].pointer==addresses[i]);
}
void destruction_trace(const std::vector<const void*>&addresses,size_t begin,int completed){
  require(events.size()==begin+completed);
  for(int i=0;i<completed;++i){const int id=completed-1-i;const Event&e=events[begin+i];
    require(e.kind==kinds[id]+32&&e.id==id&&e.pointer==addresses[id]);}
}
// Registered before the extracted global, so this observer runs after the
// actual DiploPop global destructor, while event vectors are still alive.
std::vector<const void*> global_addresses;
bool shutdown_ready=false;
struct ShutdownCheck {~ShutdownCheck(){
  require(shutdown_ready);require(services==std::vector<char>({'1','2','V','H','C'}));
  destruction_trace(global_addresses,0,11);
}} shutdown_check;
OWNER

void clear_services(){services.clear();callback=nullptr;teardown_check=false;visible_result=1;}
void seed(DiploPop&p){active=&p;p.counterpart_faction_=7;p.field_20B0_=23;p.field_2154_=29;clear_services();}
struct Derived : DiploPop {
  void hide() override {require(this==active);service('D');}
  void show(int) override {require(false);}
};
struct Target : Win {
  int marker;Target(int id):marker(id){}
  void on_button_clicked(int value) override;
};
Target first(1),second(2);
int button_value=0;
void Target::on_button_clicked(int value){require(value==button_value);service(marker==1?'x':'y');WinModalFocus=&second;}
int main(){
  global_addresses=member_addresses(DiplomacyPopup);
  require(events.size()==11);construction_trace(global_addresses,11);
  require(DiplomacyPopup.counterpart_faction_==-1&&DiplomacyPopup.field_2154_==1);
  require(DiplomacyPopup.field_20B0_==0);
  alignas(DiploPop) unsigned char raw[sizeof(DiploPop)];
  std::vector<const void*> addresses;
  // A completed construction establishes member addresses for the SAME
  // storage, without manufacturing pre-lifetime member pointers.
  next_id=0;events.clear();construction_storage=raw;std::memset(raw,0xa5,sizeof raw);poison_check=true;
  DiploPop*p=new(raw)DiploPop;poison_check=false;active=p;addresses=member_addresses(*p);
  construction_trace(addresses,11);require(p->counterpart_faction_==-1&&p->field_2154_==1);
  require(p->field_20B0_==0xa5a5a5a5u);seed(*p);teardown_check=true;
  p->~DiploPop();require(services==std::vector<char>({'1','2','V','H','C'}));destruction_trace(addresses,11,11);
  teardown_check=false;
  // Throw at GraphicWin, every Font/Time/Flic and each FlatButton element.
  for(int failure=0;failure<11;++failure){
    next_id=0;throw_at=failure;events.clear();services.clear();std::memset(raw,0xa5,sizeof raw);poison_check=true;
    try{new(raw)DiploPop;require(false);}catch(int index){require(index==failure);}
    poison_check=false;require(services.empty());inspect_poison();
    construction_trace(addresses,failure+1);destruction_trace(addresses,failure+1,failure);
  }
  throw_at=-1;next_id=0;events.clear();std::memset(raw,0xa5,sizeof raw);p=new(raw)DiploPop;active=p;
  for(int visible : {0,1,-1,2})for(int faction : {-2,-1,0,7}){
    seed(*p);visible_result=visible;p->counterpart_faction_=faction;p->show(991);
    require(services==(visible==0&&faction!=-1?std::vector<char>{'V','S'}:std::vector<char>{'V'}));
    require(p->counterpart_faction_==faction&&p->field_20B0_==23&&p->field_2154_==29);
  }
  // Visibility is called first; its boundary can alter the later faction read.
  for(int after : {-1,3}){
    seed(*p);visible_result=0;p->counterpart_faction_=after==-1?3:-1;
    callback=[&](char kind){if(kind=='V')p->counterpart_faction_=after;};p->show(5);
    require(services==(after==-1?std::vector<char>{'V'}:std::vector<char>{'V','S'}));
  }
  seed(*p);callback=[&](char){require(p->counterpart_faction_==7&&p->field_20B0_==23&&p->field_2154_==29);};
  p->close();inspect_reset();require(services==std::vector<char>({'1','2','V','H','C'}));
  seed(*p);callback=[&](char kind){if(kind=='C'){p->counterpart_faction_=99;p->field_20B0_=99;p->field_2154_=99;}};
  p->close();inspect_reset();
  seed(*p);visible_result=0;p->close();require(services==std::vector<char>({'1','2','V','C'}));inspect_reset();
  seed(*p);callback=[&](char){require(p->counterpart_faction_==7);};p->stop();
  require(services==std::vector<char>({'I','b','a'}));require(p->counterpart_faction_==-1&&p->field_20B0_==23&&p->field_2154_==29);
  seed(*p);callback=[&](char kind){if(kind=='a')p->counterpart_faction_=99;};p->stop();require(p->counterpart_faction_==-1);
  for(int value : {-2147483647,-1,0,7,2147483647}){
    seed(*p);button_value=value;WinModalFocus=&first;p->on_button_clicked(value);require(services==std::vector<char>({'x'}));
    services.clear();p->on_button_clicked(value);require(services==std::vector<char>({'y'}));
    services.clear();WinModalFocus=nullptr;p->on_button_clicked(value);require(services.empty());
  }
  seed(*p);teardown_check=true;p->~DiploPop();teardown_check=false;
  // A further-derived hide participates in explicit close, but not in the
  // DiploPop destructor once C++ has changed the active dynamic type.
  next_id=0;events.clear();Derived*d=new Derived;seed(*d);d->DiploPop::close();
  require(services==std::vector<char>({'1','2','D','C'}));inspect_reset();
  seed(*d);visible_result=0;d->DiploPop::show(999);require(services==std::vector<char>({'V','S'}));
  seed(*d);teardown_check=true;delete d;require(services==std::vector<char>({'1','2','V','H','C'}));teardown_check=false;
  // Leave only the real global's teardown observations for the final checker.
  seed(DiplomacyPopup);events.clear();teardown_check=true;shutdown_ready=true;
}
'''


def run_host(tmp_path, bases, declarations, owner, methods):
    compiler=shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler unavailable')
    source=tmp_path/'lifecycle.cpp'
    source.write_text(HARNESS.replace('BASE_DECLARATIONS',bases).replace('DECLARATIONS',declarations)
                      .replace('METHODS','\n'.join(methods.values())).replace('OWNER',owner))
    binary=tmp_path/'lifecycle'
    compiled=subprocess.run([compiler,'-std=c++11','-O1','-g','-fsanitize=address,undefined',
                             '-fno-sanitize-recover=all',str(source),'-o',str(binary)],
                            capture_output=True,text=True,timeout=60)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=20,
                          env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))


def test_actual_diplopop_lifecycle(tmp_path):
    result=run_host(tmp_path,*actual())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize('method,before,after',[
    ('ctor','counterpart_faction_ = -1;','counterpart_faction_ = 0;'),
    ('ctor','field_2154_ = 1;','field_2154_ = 0;'),
    ('dtor','close();','(void)0;'),
    ('close','time1_.stop();','time2_.stop();'),
    ('close','time2_.stop();','(void)0;'),
    ('close','hide();','DiploPop::hide();'),
    ('close','GraphicWin::close();','Win::hide();'),
    ('close','counterpart_faction_ = -1;','counterpart_faction_ = 0;'),
    ('close','field_20B0_ = 0;','field_20B0_ = 1;'),
    ('close','field_2154_ = 1;','field_2154_ = 0;'),
    ('show','is_visible() == 0','is_visible() != 0'),
    ('show','counterpart_faction_ != -1','counterpart_faction_ > -1'),
    ('show','Win::show(0);','Win::show(1);'),
    ('stop','release_iface_mode();','(void)0;'),
    ('stop','flic2_.close();','flic1_.close();'),
    ('stop','flic1_.close();','(void)0;'),
    ('stop','counterpart_faction_ = -1;','counterpart_faction_ = 0;'),
    ('button','if (target)','if (!target)'),
    ('button','target->on_button_clicked(button_id);','target->Win::on_button_clicked(button_id);'),
    ('button','target->on_button_clicked(button_id);','target->on_button_clicked(0);'),
])
def test_diplopop_behavior_damage_is_observed(tmp_path,method,before,after):
    bases,declarations,owner,methods=actual()
    assert before in methods[method]
    methods[method]=methods[method].replace(before,after)
    result=run_host(tmp_path,bases,declarations,owner,methods)
    assert result.returncode==105 and 'assertion line' in result.stderr,result.stderr


@pytest.mark.parametrize('damage',['early_constructor','member_order','early_close','early_stop','cached_faction'])
def test_diplopop_lifetime_timing_damage_is_observed(tmp_path,damage):
    bases,declarations,owner,methods=actual()
    if damage=='early_constructor':
        methods['ctor']=methods['ctor'].replace('DiploPop::DiploPop() {','DiploPop::DiploPop() : counterpart_faction_(-1) {')
    elif damage=='member_order':
        declarations=declarations.replace('Time time1_;','Time swap_;').replace('Time time2_;','Time time1_;').replace('Time swap_;','Time time2_;')
    elif damage=='early_close':
        methods['close']=methods['close'].replace('time1_.stop();','counterpart_faction_ = -1; time1_.stop();')
    elif damage=='early_stop':
        methods['stop']=methods['stop'].replace('release_iface_mode();','counterpart_faction_ = -1; release_iface_mode();')
    else:
        methods['show']=methods['show'].replace('if (is_visible() == 0 && counterpart_faction_ != -1)',
              'const int cached = counterpart_faction_; if (is_visible() == 0 && cached != -1)')
    result=run_host(tmp_path,bases,declarations,owner,methods)
    assert result.returncode==105 and 'assertion line' in result.stderr,result.stderr
