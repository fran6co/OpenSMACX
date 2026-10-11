"""Original cancel-word read via its actual owner, not an item-array underrun."""
from dataclasses import fields
from pathlib import Path
import json
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_prodpicker_behavior import extract_function

ROOT=Path(__file__).resolve().parents[2]
BASE='b0b6eb1d890534a3fc8120813164b9544bc42e44'
PIN='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def migration(text,*,reverse=False):
    patch=Path(__file__).with_name('prodpicker-cancel-member-oct11.patch').read_text()
    for hunk in patch.split('\n@@ ')[1:]:
        lines=hunk.splitlines(keepends=True)[1:]
        before=''.join(l[1:] for l in lines if l.startswith((' ','-')))
        after=''.join(l[1:] for l in lines if l.startswith((' ','+')))
        if reverse:before,after=after,before
        assert before and text.count(before)==1
        text=text.replace(before,after,1)
    return text


def restore_prodpicker_cancel_member(text):
    return migration(text,reverse=True) if 'switch (action - 1U)' in text else text


def test_bounded_source_and_original_obligations():
    path=ROOT/'src/prodpicker.cpp'
    before=subprocess.check_output(['git','show',BASE+':src/prodpicker.cpp'],cwd=ROOT,text=True)
    expected=migration(before)
    from test_prodpicker_display_id_oct11 import restore_prodpicker_display_id
    assert expected==restore_prodpicker_display_id(path.read_text()) and restore_prodpicker_cancel_member(expected)==before
    old,new=read_text(before,path),read_text(expected,path)
    assert len(old)==len(new)
    for a,b in zip(old,new):
        for field in fields(a):
            if field.name not in {'line','region','origin'}:
                assert getattr(a,field.name)==getattr(b,field.name)
        lesson=lambda r:[(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in r.origin]
        assert lesson(a)==lesson(b)
    from match_recovery.tasks import Task
    task=Task.from_dict(json.loads((ROOT/'recovery/tasks/prodpicker-cancel-member-oct11.json').read_text()))
    assert task.base_revision==BASE and {s.address for s in task.subjects}=={0x493EA0}


def test_pinned_cancel_index_id_layout_and_wrapping_dispatch():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/'.opensmacx/game/terranx_original.exe'
    if not path.is_file():pytest.skip('pinned image unavailable')
    image=PeImage(path.read_bytes(),PIN);ins={}
    for begin,end in ((0x493EA0,0x49419F),(0x5F9F40,0x5F9F5C),(0x5F8800,0x5F8893)):
        ins.update({i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(begin,end)),begin)})
    expected={
        0x4940C5:('call','0x5f9f40'),0x4940CA:('cmp','eax, 0x40'),
        0x4940CD:('jge','0x4940db'),
        0x4940CF:('lea','edx, [eax + eax*4]'),
        0x4940D2:('mov','eax, dword ptr [ebp + edx*4 - 0x52c]'),
        0x4940DE:('dec','eax'),0x4940DF:('cmp','eax, 4'),
        0x4940E2:('ja','0x49417a'),
        0x5F9F46:('cmp','eax, -1'),0x5F9F49:('je','0x5f9f58'),
        0x5F9F4E:('test','byte ptr [ecx + edx*4 + 0xa24], 2'),
        0x5F9F58:('or','eax, 0xffffffff'),
        0x5F8818:('lea','eax, [esi + 0xa1c]'),
        0x5F882A:('mov','dword ptr [eax + 4], 0xffffffff'),
        0x5F8837:('add','eax, 0x14'),
    }
    for a,v in expected.items():assert ins[a]==v
    # Frame-relative ID probe: menu base is ebp-F4C; slot-1 ID is
    # ebp-52C-14, or menu+A0C. Constructor corroborates item stride/start.
    assert -0x52C-0x14-(-0xF4C)==0xA0C
    assert 0xA18-0x14+8==0xA0C


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','skip_cancel','array_cancel','wrong_word','signed_subtract'])
def test_actual_click_and_getter_preserve_caption_word_read(tmp_path,compiler_name,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip('host compiler unavailable')
    body=extract_function((ROOT/'src/prodpicker.cpp').read_text(),'void ProdPicker::on_right_click(int x, int y)')
    getter=extract_function((ROOT/'src/pulldown.cpp').read_text(),'int PullDown::get_selected()')
    word=extract_function((ROOT/'src/graphicwin.cpp').read_text(),'uint32_t GraphicWin::field_A0C_value() const')
    if damage=='skip_cancel':body=body.replace('if (selectedItem < 0x40)','if (selectedItem >= 0 && selectedItem < 0x40)')
    elif damage=='array_cancel':body=body.replace('menu.field_A0C_value()','static_cast<uint32_t>(menu.item_id(-1))')
    elif damage=='wrong_word':body=body.replace('menu.field_A0C_value()','0xffffffffU')
    elif damage=='signed_subtract':body=body.replace('switch (action - 1U)','switch (static_cast<int32_t>(action) - 1)')
    harness=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <vector>
static int configured_slot,item_action,id_reads,word_reads,buttons,calcs,redraws,synchs;
static bool disabled;
static uint32_t callback_word;
struct GraphicWin {
 // Host storage is the target callback's 32-bit representation; never
 // invoke a fake callback. Native CodeView/layout checks are separate.
 uint32_t field_A0C_;
 uint32_t field_A0C_value() const;
};
struct Item{int id;uint32_t flags;};
struct PullDown:GraphicWin {
 int selected_;Item items_[64];
 PullDown(){field_A0C_=callback_word;selected_=configured_slot;
  for(auto &item:items_){item.id=item_action;item.flags=disabled?2:0;}}
 void init(){}void add_item(int,char*){}void show(int,int,int){}
 int get_selected();
 int item_id(int i){++id_reads;assert(i>=0 && i<64);return items_[i].id;}
};
struct Payload{uint32_t field_8_=0x20;};
typedef Payload StringStructPayload;
struct List{int entry_count_=1;Payload value;void seek_pos(int){}
 Payload *current_entry(){return &value;}int current_id(){return 3;}};
struct Proto{uint8_t weapon_id=0,obsolete_factions=0xff;int flags=1;}VehPrototypes[4];
struct WeaponRecord{int offense_rating=1;}Weapon[1];
struct Base{int faction_id_current=1;}base,*BaseCurrent=&base;
const int MaxVehProtoNum=4,PROTO_ACTIVE=1;int IsMultiplayerNet=0;
char *label_get(int){static char label[2];return label;}
void synch_proto(int id){assert(id==3);++synchs;}
struct ProdPicker {
 int field_A774_=0,field_B68_=0;List prod_a_;
 void on_left_click(int,int){}void client_to_screen(int*,int*){}
 void calculate(){++calcs;}void redraw(){++redraws;}
 void on_button_clicked(int button){buttons=button;}
 void on_right_click(int,int);
};
'''
    word=word.replace('    uint32_t value = 0;','    ++word_reads;\n    uint32_t value = 0;')
    cases=r'''
void check(int slot,uint32_t word,int action,bool disable){
 configured_slot=slot;callback_word=word;item_action=action;disabled=disable;
 id_reads=word_reads=calcs=redraws=synchs=0;buttons=99;
 for(auto &proto:VehPrototypes){proto.obsolete_factions=0xff;proto.flags=1;}
 VehPrototypes[2].flags=0;
 ProdPicker p;p.on_right_click(11,22);
 bool cancel=slot==-1 || (slot>=0 && slot<64 && disable);
 assert(word_reads==int(cancel) && id_reads==int(!cancel && slot<64));
 uint32_t native=cancel?word:slot<64?uint32_t(action):0xffffffffU;
 switch(native-1U){
 case 0:assert(synchs==1 && calcs==1 && redraws==1 && buttons==99);break;
 case 1:assert(buttons==0 && calcs==0);break;
 case 2:assert(calcs==1 && redraws==1 && buttons==99);
  for(int i=0;i<4;++i)assert(VehPrototypes[i].obsolete_factions==(i==2?0xff:0xfd));break;
 case 3:assert(buttons==5 && calcs==0);break;
 case 4:assert(buttons==-1 && calcs==0);break;
 default:assert(buttons==99 && calcs==0 && redraws==0 && synchs==0);break;
 }
}
int main(){
 // Controlled callback bit patterns test the read/arithmetic, not claims
 // that these tiny addresses are callable or occur in ordinary gameplay.
 for(uint32_t word:{0U,1U,2U,3U,4U,5U,0x5d69f0U,0x80000000U,0xffffffffU}){
  check(-1,word,2,false);check(3,word,2,true);
 }
 // Execute only get_selected's established slot domain. A corrupted
 // selected_=64 already makes that getter index outside its item array;
 // the caller's upper-bound instruction is checked separately above.
 for(int slot:{0,3,63})for(int action:{-1,0,1,2,3,4,5})check(slot,0x5d69f0U,action,false);
}
'''
    exe=tmp_path/'actual'
    built=subprocess.run([compiler,'-std=c++17','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-x','c++','-','-o',str(exe)],input=harness+word+getter+body+cases,text=True,capture_output=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(exe)],cwd=tmp_path,capture_output=True,preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode==0)==(not damage),result.stderr
