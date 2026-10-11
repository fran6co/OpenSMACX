"""Faithful selection/cancellation branches, not intentional game bug fixes."""
from dataclasses import fields
from pathlib import Path
import json
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from councwin_owner_control_fixture import LEFT_CLICK_HARNESS, adapt_body, extract_function

ROOT = Path(__file__).resolve().parents[2]
BASE = 'bf8268f4621fc58fc2e8233b0e09f561d9e90e81'


def migration(text, *, reverse=False):
    fixture = Path(__file__).with_name('council-selection-paths-oct11.patch').read_text()
    for hunk in fixture.split('\n@@ ')[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ','-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ','+')))
        if reverse:
            before, after = after, before
        assert before and text.count(before)==1
        text = text.replace(before,after,1)
    return text


def restore_council_selection_paths(text):
    return migration(text,reverse=True) if '        selection = y;\n' in text else text


def test_bounded_actual_source_and_historical_continuity():
    path = ROOT/'src/councwin.cpp'
    old = subprocess.check_output(['git','show',BASE+':src/councwin.cpp'],cwd=ROOT,text=True)
    expected = migration(old)
    assert expected==path.read_text() and restore_council_selection_paths(expected)==old
    before,after = read_text(old,path),read_text(expected,path)
    assert len(before)==len(after)
    for a,b in zip(before,after):
        for field in fields(a):
            if field.name not in {'line','region','origin'}:
                assert getattr(a,field.name)==getattr(b,field.name)
        lessons = lambda r: [(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in r.origin]
        assert lessons(a)==lessons(b)
    from match_recovery.tasks import Task
    task=Task.from_dict(json.loads((ROOT/'recovery/tasks/council-selection-paths-oct11.json').read_text()))
    assert task.base_revision==BASE and {s.address for s in task.subjects}=={0x425050}


def test_pinned_original_argument_and_branch_specific_cancel_paths():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/'.opensmacx/game/terranx_original.exe'
    if not path.is_file():
        pytest.skip('pinned image unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    ins={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x425050,0x425B92)),0x425050)}
    expected={
        0x42511F:('je','0x4253ca'),
        0x4253CA:('mov','esi, dword ptr [ebp + 0xc]'),
        0x425251:('or','esi, 0xffffffff'),
        0x425257:('sub','esi, eax'),
        0x425259:('mov','dword ptr [ebp + 0xc], esi'),
        0x42525C:('je','0x4259b8'),
        0x425262:('jmp','0x42535f'),
        0x42534A:('call','0x602600'),
        0x42534F:('mov','esi, eax'),
        0x425351:('test','esi, esi'),
        0x425356:('jl','0x4259b5'),
        0x425362:('cmp','esi, 0x270f'),
        0x425368:('je','0x4259b8'),
        0x4256E3:('mov','ecx, dword ptr [ebp + 0xc]'),
        0x4259B5:('mov','ebx, dword ptr [ebp - 0x10]'),
        0x4259B8:('push','edi'),
        0x4259B9:('mov','ecx, ebx'),
        0x4259BB:('call','0x425db0'),
    }
    for address,instruction in expected.items():
        assert ins[address]==instruction


CASES = r'''
int main() {
  CouncWin win; ConsoleGlobal.faction_id_=1;
  // Government modal cancellation: only this producer treats negatives
  // as cancel; do not propagate that rule to WHICHVOTE or original y.
  win.spot_.faction=0; mode=0;
  for(int result: {-1,-7,0x270f}) {
    events.clear(); LockEnableMask=(1u<<1)|(1u<<8); exec_result=result;
    win.on_left_click(12,23);
    assert(events.back()=="close");
    assert(events[events.size()-2]=="leader:0");
    for(auto &e:events) assert(e.find("buy:")!=0);
  }
  events.clear(); exec_result=0;
  win.on_left_click(12,23);
  assert(last_buy_actor==1);
  bool bought_zero=false; for(auto &e:events) bought_zero |= e=="buy:0";
  assert(bought_zero);

  // WHICHVOTE's zero/sentinel cancellation is distinct from its negatives.
  win.spot_.faction=2; LockEnableMask=(1u<<1)|(1u<<10);
  for(int result: {-1,-10000}) {
    events.clear(); prompt_result=result;
    win.on_left_click(12,23);
    assert((events==std::vector<std::string>{"xpop:WHICHVOTE","leader:2","close"}));
  }
  events.clear(); prompt_result=1; win.on_left_click(12,23);
  assert(events[1]=="buy:-2");

  // Dead-target y sentinel cancels before mode observation/A40 writes.
  LockEnableMask=(1u<<1)|(1u<<9);win.spot_.option=17;
  mode=1;mode_reads=0;win.field_A40_=91;win.field_A44_=1u<<1;
  events.clear();win.on_left_click(12,0x270f);
  assert((events==std::vector<std::string>{"leader:2","close"}));
  assert(mode_reads==0 && win.field_A40_==91);

  // Model a concurrent alive-mask update precisely at the mode read, so
  // the shared tail consumes the dead-target branch's saved y value.
  // This is an observation schedule, not ordinary gameplay reachability.
  for(int y: {23,-2}) {
    events.clear();LockEnableMask=(1u<<1)|(1u<<9);revive=true;mode_reads=0;
    win.field_A44_=1u<<1;win.on_left_click(12,y);
    assert(mode_reads==1);
    bool bought=false;for(auto &e:events) bought |= e=="buy:"+std::to_string(y);
    assert(bought && events.back()=="close");
  }
}
'''


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['','y','government','sentinel','generic_negative'])
def test_actual_selection_and_cancellation_branches(tmp_path,compiler_name,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    source=(ROOT/'src/councwin.cpp').read_text()
    body=adapt_body(extract_function(source,'void CouncWin::on_left_click(int x, int y)'))
    if damage=='y':
        body=body.replace('selection = y;','selection = 17;')
    elif damage=='government':
        body=body.replace('if (selection < 0) {','if (false) {')
    elif damage=='sentinel':
        body=body.replace('if (selection == 0x270f) {','if (false) {')
    elif damage=='generic_negative':
        body=body.replace('if (selection == 0x270f) {','if (selection < 0 || selection == 0x270f) {')
    fixture=LEFT_CLICK_HARNESS.split('BODY')[0]
    fixture=fixture.replace('int IsMultiplayerNet = 0;',r'''
int mode=0,mode_reads=0;bool revive=false;
struct ModeRead {
 operator int() { ++mode_reads;if(revive){LockEnableMask|=1u<<10;revive=false;}return mode; }
} IsMultiplayerNet;
''')
    fixture=fixture.replace('int prompt_result = 0;','int prompt_result = 0; int exec_result=0;')
    fixture=fixture.replace('return 0x270f;','return exec_result;')
    fixture=fixture.replace('NetMsg *NetMsgPopup = &net_msg;','NetMsg *NetMsgPopup = &net_msg; NetMsg NetMsgPrimary;')
    executable=tmp_path/'actual'
    result=subprocess.run([compiler,'-std=c++17','-O2','-x','c++','-','-o',str(executable)],input=fixture+body+CASES,text=True,capture_output=True)
    assert result.returncode==0,result.stderr
    run=subprocess.run([str(executable)],cwd=tmp_path,capture_output=True,preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (run.returncode==0)==(not damage),run.stderr
