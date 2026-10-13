"""Recovered picker ABI/state and original ordinal conventions, not GUI proof."""
from dataclasses import fields
from pathlib import Path
import json
import os
import re
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = '817b245ffa3aa2715ce985269e7078ece6ec3237'
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
PATHS = ('src/designwin.cpp', 'src/selectpartwin.cpp', 'src/selectpartwin.h')
MEMBERS = {'faction_id_':0x2B68, 'part_kind_':0x2B84,
           'chassis_id_':0x2B88, 'weapon_id_':0x2B8C, 'armor_id_':0x2B90,
           'ability_flags_':0x2B94, 'reactor_id_':0x2B98, 'palette_':0x4644}


def migration(text, relative, *, reverse=False):
    patch = Path(__file__).with_name('design-part-picker-contract-oct11.patch').read_text()
    section = next(p for p in patch.split('diff --git ')[1:]
                   if p.startswith('a/'+relative+' b/'+relative+'\n'))
    for hunk in section.split('\n@@ ')[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ','-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ','+')))
        if reverse:
            before, after = after, before
        assert before and text.count(before)==1
        text = text.replace(before,after,1)
    return text


def restore_design_picker_contract(text):
    return migration(text,'src/designwin.cpp',reverse=True) if 'protoReactorType_, SpotlightColorTable);' in text else text


@pytest.mark.parametrize('relative',PATHS)
def test_exact_bounded_source_and_original_obligations(relative):
    path = ROOT/relative
    old = subprocess.check_output(['git','show',BASE+':'+relative],cwd=ROOT,text=True)
    expected = migration(old,relative)
    assert migration(expected,relative,reverse=True)==old
    recipe_before,recipe_after = read_text(old,path),read_text(expected,path)
    assert len(recipe_before)==len(recipe_after)
    for before,after in zip(recipe_before,recipe_after):
        for field in fields(before):
            if field.name not in {'line','region','origin'}:
                assert getattr(before,field.name)==getattr(after,field.name)
        lessons = lambda r:[(l.kind,l.key,l.prose,tuple(t for _,t in l.lines))
                            for l in r.origin]
        assert lessons(before)==lessons(after)

    # The historical recipe remains an independently reversible migration.
    # Current owner files have since accumulated other reviewed migrations, so
    # comparing their whole text to this one recipe would reject valid source.
    original,current = read_text(old,path),read_text(path.read_text(),path)
    key = lambda r:(r.address,r.name,r.symbol)
    by_key = {key(r):r for r in current}
    assert len(original)==len(current) and set(map(key,original))==set(by_key)
    lessons = lambda r:[(l.kind,l.key,l.prose,tuple(t for _,t in l.lines))
                        for l in r.origin]
    for before in original:
        after = by_key[key(before)]
        for field in fields(before):
            if field.name not in {'line','region','origin','image_spans'}:
                assert getattr(before,field.name)==getattr(after,field.name)
        assert lessons(before)==lessons(after)
        if relative=='src/designwin.cpp' and key(before)==(
                0x43AC70, '?select_special@DesignWin@@QAEXH@Z', ''):
            # This independently reviewed cold-span recovery predates the
            # parent-surface task. No other historical span is waived.
            assert before.image_spans==((0x43AC70,0x43BDE9),(0x654410,0x654416))
            assert after.image_spans==((0x43AC70,0x43BDE9),(0x654410,0x6546C5))
        else:
            assert before.image_spans==after.image_spans


def test_source_owned_member_offsets_and_pointer_declaration():
    from match_recovery.layout_source import parse_members
    header=(ROOT/'src/selectpartwin.h').read_text()
    rows=parse_members(header,'src/selectpartwin.h',PIN,address_space='pe:va')
    assert {r.member:r.offset for r in rows if r.member in MEMBERS}==MEMBERS
    assert '  unsigned char *palette_;' in header
    assert 'sizeof(SelectPartWin) == 0x464C' in header
    task=json.loads((ROOT/'recovery/tasks/design-part-picker-contract-oct11.json').read_text())
    assert task['base_revision']==BASE
    assert {0x439F20,0x43A300,0x43A760,0x43AA00,0x433F70} <= {s['address'] for s in task['subjects']}


def test_pinned_argument_stores_palette_and_direct_ordinal_scan():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=Path(os.environ.get('OPENSMACX_IMAGE', ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned image unavailable')
    image=PeImage(path.read_bytes(),PIN)
    ins={}
    for begin,end in ((0x43AA00,0x43AC69),(0x433F70,0x43446F)):
        ins.update({i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(begin,end)),begin)})
    expected={
        0x43AA5E:('push','0x6feb7c'),
        0x43AA76:('mov','edi, dword ptr [esi + 0x1420c]'),
        0x43AA7D:('mov','edi, dword ptr [esi + 0x14208]'),
        0x43AA84:('mov','edi, dword ptr [esi + 0x14204]'),
        0x43AA8B:('mov','edi, dword ptr [esi + 0x14200]'),
        0x43AA92:('mov','edi, dword ptr [esi + 0x141fc]'),
        0x43AA99:('push','edx'),
        0x43AA9A:('mov','edx, dword ptr [esi + 0x141f4]'),
        0x43AAA0:('push','3'),0x43AAA2:('push','edx'),
        0x43AAA3:('push','ecx'),0x43AAA4:('push','eax'),
        0x43AAAB:('call','0x433f70'),
        0x43AAC1:('cmp','eax, edi'),0x43AAC5:('dec','eax'),
        0x433FF5:('mov','dword ptr [ebp - 0x1c], eax'),
        0x434006:('mov','dword ptr [ebx + 0x2b68], ecx'),
        0x43400F:('mov','dword ptr [ebx + 0x2b88], edx'),
        0x434018:('mov','dword ptr [ebx + 0x2b8c], ecx'),
        0x434021:('mov','dword ptr [ebx + 0x2b90], edx'),
        0x43402D:('mov','dword ptr [ebx + 0x2b84], eax'),
        0x434033:('mov','dword ptr [ebx + 0x2b94], ecx'),
        0x434039:('mov','dword ptr [ebx + 0x2b98], edx'),
        0x43403F:('mov','dword ptr [ebx + 0x4644], 0x6feb7c'),
        0x43404B:('dec','dword ptr [ebp + 0x18]'),
        0x434413:('inc','eax'),
        0x43445C:('mov','eax, dword ptr [ebp - 0x1c]'),
    }
    for address,instruction in expected.items():assert ins[address]==instruction


def run(tmp_path,compiler_name,source,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip('host compiler unavailable')
    exe=tmp_path/'actual'
    built=subprocess.run([compiler,'-std=c++17','-O2','-x','c++','-','-o',str(exe)],input=source,text=True,capture_output=True)
    assert built.returncode==0,built.stderr
    result=subprocess.run([str(exe)],cwd=tmp_path,capture_output=True,preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode==0)==(not damage),result.stderr


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','arguments','ordinal','palette'])
def test_actual_reactor_caller_with_distinct_state_and_cancel(tmp_path,compiler_name,damage):
    source=(ROOT/'src/designwin.cpp').read_text()
    body=extract(source,'void DesignWin::select_reactor()')
    helpers=source[source.index('template <typename Record>\nstatic int picker_row_before'):source.index('static const int DesignSelectableChassisCount')]
    if damage=='arguments':
        body=body.replace('outer_rect_.left, outer_rect_.top, factionID_, 3, row,\n        protoChassisType_, protoWeaponType_, protoArmorType_,\n        protoAbilityFlags_, protoReactorType_, SpotlightColorTable',
                          'protoReactorType_, protoAbilityFlags_, protoArmorType_, 3,\n        protoChassisType_, row, 3, factionID_, outer_rect_.top,\n        outer_rect_.left, SpotlightColorTable')
    elif damage=='ordinal':body=body.replace('picker_record_at(selected_row,','picker_record_at(selected_row - 1,')
    elif damage=='palette':body=body.replace('SpotlightColorTable);','0);')
    harness=r'''
#include <cassert>
#include <initializer_list>
struct Record {int preq_tech;} Reactor[4]={{-2},{0},{-1},{0}};
const int MaxReactorNum=4;
unsigned char SpotlightColorTable[256];
int reply, redraws, calls;
struct SelectPartWin {
 int exec(int x,int y,int faction,int kind,int selected,int chassis,int weapon,int armor,int abilities,int reactor,unsigned char *palette) {
  ++calls;assert(x==111 && y==222 && faction==3 && kind==3 && selected==1);
  assert(chassis==17 && weapon==23 && armor==31 && abilities==0x40000 && reactor==2);
  assert(palette==SpotlightColorTable);return reply;
 }
};
struct Rect{int left,top;} ;
struct DesignWin {
 Rect outer_rect_={111,222};int factionID_=3;
 int protoChassisType_=17,protoWeaponType_=23,protoArmorType_=31;
 int protoAbilityFlags_=0x40000,protoReactorType_=2;
 void select_reactor();
};
void redraw_design_picker_changes(DesignWin*){++redraws;}
'''
    cases=r'''
int main(){
 for(int result:{1,2,0,-1,3}){
  DesignWin win;reply=result;calls=redraws=0;win.select_reactor();
  // Saved ordinal 1 on cancel preserves record 2. Successful kind-3
  // result 2 selects record 3. Exhausted/negative scans return count;
  // do not normalize those original out-of-range results here.
  int expected=result==0?1:result==1?2:result==2?3:4;
  assert(win.protoReactorType_==expected && calls==1 && redraws==1);
 }
}
'''
    run(tmp_path,compiler_name,harness+helpers+body+cases,damage)


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','palette_argument','omit_decrement'])
def test_actual_exec_initialization_and_saved_cancel_selection(tmp_path,compiler_name,damage):
    body=extract((ROOT/'src/selectpartwin.cpp').read_text(),'int SelectPartWin::exec(')
    initialization=body[body.index('    (void)x;'):body.index('    caviar_.init();')]
    if damage=='palette_argument':initialization=initialization.replace('palette_ = SpotlightColorTable;','palette_ = palette;')
    elif damage=='omit_decrement':initialization=initialization.replace('--selected;',';')
    header=(ROOT/'src/selectpartwin.h').read_text()
    declarations='\n'.join(re.search(r'(?m)^  (?:int|unsigned char \*)\s*'+name+r';',header)[0] for name in MEMBERS)
    harness=r'''
#include <cassert>
unsigned char SpotlightColorTable[256],other[256];
struct Picker {
DECLARATIONS
 int initialize(int x,int y,int faction,int kind,int selected,int chassis,int weapon,int armor,int abilities,int reactor,unsigned char *palette) {
INITIALIZATION
  assert(faction_id_==3 && part_kind_==kind && chassis_id_==17 && weapon_id_==23 && armor_id_==31);
  assert(ability_flags_==0x40000 && reactor_id_==2 && palette_==SpotlightColorTable);
  assert(selected==saved_selection-(kind==3));return saved_selection;
 }
};
int main(){Picker p;for(int kind=0;kind<4;++kind){
 assert(p.initialize(111,222,3,kind,5,17,23,31,0x40000,2,other)==5);
 assert(p.initialize(111,222,3,kind,5,17,23,31,0x40000,2,0)==5);
}}
'''.replace('DECLARATIONS',declarations).replace('INITIALIZATION',initialization)
    run(tmp_path,compiler_name,harness,damage)
