"""Actual production-row classification/rendering; not whole-game proof."""
from dataclasses import fields
from pathlib import Path
import json
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_prodpicker_behavior import extract_function,calculate_source

ROOT=Path(__file__).resolve().parents[2]
BASE='5acbd78b80262c7b99c49fa9dbefd4adbec949ec'
PIN='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def migration(text,*,reverse=False):
    patch=Path(__file__).with_name('prodpicker-display-id-oct11.patch').read_text()
    for hunk in patch.split('\n@@ ')[1:]:
        lines=hunk.splitlines(keepends=True)[1:]
        before=''.join(l[1:] for l in lines if l.startswith((' ','-')))
        after=''.join(l[1:] for l in lines if l.startswith((' ','+')))
        if reverse:before,after=after,before
        assert before and text.count(before)==1
        text=text.replace(before,after,1)
    return text


def restore_prodpicker_display_id(text):
    return migration(text,reverse=True) if 'uint32_t displayBits =' in text else text


def test_bounded_definition_originals_and_historical_notes():
    path=ROOT/'src/prodpicker.cpp'
    old=subprocess.check_output(['git','show',BASE+':src/prodpicker.cpp'],cwd=ROOT,text=True)
    expected=migration(old)
    assert path.read_text()==expected and restore_prodpicker_display_id(expected)==old
    for a,b in zip(read_text(old,path),read_text(expected,path),strict=True):
        for field in fields(a):
            if field.name not in {'line','region','origin'}:
                assert getattr(a,field.name)==getattr(b,field.name)
        lesson=lambda r:[(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in r.origin]
        assert lesson(a)==lesson(b)
    from match_recovery.tasks import Task
    task=Task.from_dict(json.loads((ROOT/'recovery/tasks/prodpicker-display-id-oct11.json').read_text()))
    assert task.base_revision==BASE and {s.address for s in task.subjects}=={0x4927C0}


def test_pinned_flag_negation_precedes_signed_icon_dispatch():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/'.opensmacx/game/terranx_original.exe'
    if not path.is_file():pytest.skip('pinned image unavailable')
    image=PeImage(path.read_bytes(),PIN)
    ins={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x4927C0,0x492F9F)),0x4927C0)}
    expected={0x492A2C:('mov','dword ptr [ebp - 0x24], edx'),
              0x492A45:('mov','eax, dword ptr [eax + 8]'),
              0x492A48:('test','al, 0x20'),0x492A4A:('jne','0x492a54'),
              0x492A4F:('neg','ecx'),0x492A51:('mov','dword ptr [ebp - 0x24], ecx'),
              0x492C00:('mov','ecx, dword ptr [ebp - 0x24]'),
              0x492C03:('test','ecx, ecx'),0x492C05:('jl','0x492e65')}
    for a,v in expected.items():assert ins[a]==v


def run(tmp_path,compiler_name,text,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip('host compiler unavailable')
    exe=tmp_path/'actual'
    built=subprocess.run([compiler,'-std=c++17','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-x','c++','-','-o',str(exe)],input=text,text=True,capture_output=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(exe)],cwd=tmp_path,capture_output=True,preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode==0)==(not damage),result.stderr


def draw_body():
    return extract_function((ROOT/'src/prodpicker.cpp').read_text(),'void ProdPicker::draw_item(int item, int update)')


def classification():
    body=draw_body()
    return body[body.index('    uint32_t displayBits ='):body.index('    StringStructPayload *name =')]


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','omit_negation','wrong_flag','signed_negation'])
def test_actual_classification_consumes_actual_calculate_rows_and_wraps(tmp_path,compiler_name,damage):
    fragment=classification()
    if damage=='omit_negation':fragment=fragment.replace('displayBits = 0U - displayBits;','displayBits = displayBits;')
    elif damage=='wrong_flag':fragment=fragment.replace('itemFlags & 0x20','itemFlags & 0x10')
    elif damage=='signed_negation':fragment=fragment.replace('0U - displayBits','static_cast<uint32_t>(-productionId)')
    helper='int display_id(int productionId,uint32_t itemFlags){\n'+fragment+'return displayId;\n}\n'
    source=calculate_source()
    source=source.replace('int main() {',helper+'int main() {',1)
    at=source.rfind('}')
    cases=r'''
    // Actual calculate's vehicle/facility rows retain positive stored IDs.
    assert(display_id(picker.prod_a_.ids[0],picker.prod_a_.flags[0])==7);
    assert(display_id(picker.prod_a_.ids[1],picker.prod_a_.flags[1])==-3);
    assert(picker.prod_a_.ids[0]==7 && picker.prod_a_.ids[1]==3);
    assert(display_id(0,0)==0 && display_id(0,0x20)==0);
    assert(display_id(-7,0)==7 && display_id(-7,0x20)==-7);
    assert(display_id(-2147483647-1,0)==-2147483647-1);
    assert(display_id(-2147483647-1,0x20)==-2147483647-1);
'''
    run(tmp_path,compiler_name,source[:at]+cases+source[at:],damage)


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','omit_negation','wrong_flag'])
def test_actual_complete_draw_method_selects_facility_project_or_vehicle(tmp_path,compiler_name,damage):
    body=draw_body()
    if damage=='omit_negation':body=body.replace('displayBits = 0U - displayBits;','displayBits = displayBits;')
    elif damage=='wrong_flag':body=body.replace('itemFlags & 0x20','itemFlags & 0x10')
    harness=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <initializer_list>
struct RECT{int left,top,right,bottom;};
struct BoxSpriteParams{int edge1_;}InterfaceBoxStyles[1];
struct Font{int unk_1_;int line_advance(){return 5;}};
struct StringStructPayload{char *text;uint32_t field_8_;};
struct StringList {
 int entry_count_=1,id=0;void *head_=nullptr;StringStructPayload payload;
 void seek_pos(int i){assert(i==0);}int current_id(){return id;}
 StringStructPayload *current_entry(){return &payload;}
};
int sprite_draws,vehicle_draws,domain,icon_index,fake_argument,render_calls,cost_calls;
struct Sprite{
 int family,index;int transparent_index(){return 0;}int width(){return 11;}
 void draw_mono(void*,int,int,int,int,int,int){++sprite_draws;domain=family;icon_index=index;}
}g_FACILITY_ICON_SPRITES[70],g_SECRET_PROJECT_ICON_SPRITES[64];
struct Caviar{
 void reset_objects(){}void set_scaling(float){}void set_scene_rotation(float,int,int){}
 void render(void*,int,int,int){++render_calls;}
};
int BaseIDCurrentSelected=2,MapWinCaptionScale=-1,MapWinCaptionDivisor=-1;
struct Base{int faction_id_current=2;}base,*BaseCurrent=&base;
char StringTemp[256];const float ProdPickerRotationRadiansPerDegree=0.0174533f;
const float ProdPickerVehicleScaleFactor=1.0f; // branch double, not numeric renderer proof
int mandate_color(int bits){return bits;}
int veh_fake(int id,int){fake_argument=id;return 77;}
int veh_draw(void*,int id,int,int,int,int,int){assert(id==77);++vehicle_draws;return 1;}
void vehdraw_construct_vehicle(int id,Caviar*){assert(id==77);}
void veh_cost(int,int,int *flags){++cost_calls;*flags=0;}
int prototype_factor(int){return 0;}char *label_get(int){static char text[2];return text;}
char *_itoa(int n,char *buffer,int){sprintf(buffer,"%d",n);return buffer;}
struct ProdPicker {
 int field_A77C_=0,field_A82C_=1,field_A810_=1,field_A81C_=80,field_A820_=70;
 uint32_t field_A774_=99,field_A818_=1,field_B64_=0;
 RECT rect2_;StringList prod_a_,prod_b_,prod_c_;Font font1_,font3_;Caviar caviar_;
 void set_clip(RECT*){}void box_sprite(RECT*,BoxSpriteParams*){}
 void set_text_color(int,int,int,int){}void set_font(Font*,int,int,int){}
 void wrap_cent(char*,RECT*){}int wrap_height(char*,int){return 5;}
 void write_cent_l(char*,RECT*,int){}void box(RECT*,int,int){}void soft_update(RECT*){}
 void draw_item(int,int);
};
'''
    cases=r'''
void check(int id,uint32_t flags){
 ProdPicker p;char name[]="ROW";char cost[]="COST";
 p.prod_a_.id=id;p.prod_a_.payload={name,flags};
 p.prod_c_.payload={cost,0};p.prod_b_.payload={cost,0};
 sprite_draws=vehicle_draws=render_calls=cost_calls=0;fake_argument=-999;
 p.draw_item(0,1);assert(p.prod_a_.id==id && p.prod_a_.payload.field_8_==flags);
 if(!(flags&0x20)){
  assert(sprite_draws==1 && vehicle_draws==0 && render_calls==0 && cost_calls==0);
  assert(domain==(id<0x46?0:1) && icon_index==(id<0x46?id:id-0x46));
 }else{
  assert(sprite_draws==0 && vehicle_draws==1 && render_calls==1 && cost_calls==1 && fake_argument==id);
  assert(MapWinCaptionScale==-1 && MapWinCaptionDivisor==-1);
 }
}
int main(){
 for(int i=0;i<70;++i)g_FACILITY_ICON_SPRITES[i]={0,i};
 for(int i=0;i<64;++i)g_SECRET_PROJECT_ICON_SPRITES[i]={1,i};
 check(3,0x10);check(0x45,0x10);check(0x46,0x10);check(0x65,0x10);
 check(7,0x20);check(0,0x20);
}
'''
    run(tmp_path,compiler_name,harness+body+cases,damage)
