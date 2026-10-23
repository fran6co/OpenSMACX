"""Actual timer-drain body and bounded direct-call boundary controls.

C++98 API doubles prove host ordering, not USER32, VC6 ABI, linked preservation
or game behavior. The separately selected native test requires fresh COFF.
"""
import os
from pathlib import Path
import shutil
import subprocess
from types import SimpleNamespace

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.binary import RelocationKind
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from test_infowin_setup_text import _function_end
from test_timer_close_recovery import timer_objects
from test_application_link_recovery import symbol_at

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
CALLERS = ((0x616260,0x6162CE,0x616299), (0x6162D0,0x616342,0x616309),
           (0x616350,0x616405,0x616389), (0x616410,0x6164CA,0x616449),
           (0x6164D0,0x616585,0x616509), (0x616590,0x61664D,0x6165C9),
           (0x616730,0x616772,0x61676C))


def actual():
    path = ROOT / "src/timer_messages.cpp"
    record, = (r for r in read_file(path) if r.address == 0x5FD370)
    assert record.image_spans == ((0x5FD370, 0x5FD3F2),)
    assert record.name == "?flush_timer@@YAXXZ" and record.byte_exact
    text = path.read_text()
    signature = "void __cdecl flush_timer()"
    assert text.count(signature) == 1
    start = text.index(signature)
    return text[start:_function_end(text, start)]


def test_queue_uses_only_the_shared_message_state_interface():
    queue = (ROOT / "src/timer_messages.cpp").read_text()
    win = (ROOT / "src/win.h").read_text()
    state = (ROOT / "src/win_message_state.h").read_text()
    assert '#include "win_message_state.h"' in queue
    assert '#include "win.h"' not in queue
    # Win's established public interface and the lightweight queue interface
    # declare the same one externally defined object, never separate storage.
    assert win.count('extern uint32_t MsgStatus;') == 1
    assert state.count('extern uint32_t MsgStatus;') == 1
    assert '#include <stdint.h>' in state
    assert 'MEASURED' not in state and 'class Win' not in state


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#define __cdecl
#define NULL 0
typedef unsigned int UINT;
typedef void* HWND;
struct MSG { HWND hwnd; UINT message; int wParam,lParam,time,x,y; };
enum {WM_USER=1024,PM_REMOVE=1};
unsigned int MsgStatus;
std::vector<MSG> queue;
std::string events;
MSG* last[2]; MSG saved[2];
int depth, recurse, enqueue, translate_count, dispatch_count,nested_returned;
void require(bool ok,const char*w){if(!ok){fprintf(stderr,"WITNESS: %s\n",w);exit(43);}}
void flush_timer();
MSG message(int id,UINT type=WM_USER+1){MSG m={0,type,id,100+id,200+id,300+id,400+id};return m;}
bool same(const MSG&a,const MSG&b){return a.hwnd==b.hwnd&&a.message==b.message&&a.wParam==b.wParam&&a.lParam==b.lParam&&a.time==b.time&&a.x==b.x&&a.y==b.y;}
int PeekMessage(MSG*m,HWND h,UINT lo,UINT hi,UINT flags){
 require(h==NULL&&lo==WM_USER+1&&hi==WM_USER+1&&flags==PM_REMOVE,"precise filtered PeekMessage arguments");
 events+='P';
 if(events.size()==1)require(MsgStatus==0xA500003F,"OR preserves existing MsgStatus high bits");
 if(depth)require(MsgStatus==0x83F,"nested loop ORs its live entry state");
 if(nested_returned&&!depth)require(MsgStatus==0,"outer loop observes nested cleared status");
 for(unsigned int i=0;i<queue.size();++i)if(queue[i].message==WM_USER+1){
  *m=queue[i];queue.erase(queue.begin()+i);last[depth]=m;saved[depth]=*m;
  if(depth)require(m!=last[0],"nested loop owns distinct MSG storage");
  return 7;
 }
 return 0;
}
int TranslateMessage(const MSG*m){
 require(m==last[depth]&&same(*m,saved[depth]),"Translate uses exact removed MSG");
 require(events[events.size()-1]=='P',"Translate precedes Dispatch for each removal");
 events+='T';++translate_count;return 0;
}
long DispatchMessage(const MSG*m){
 require(m==last[depth]&&same(*m,saved[depth]),"Dispatch uses exact translated MSG");
 require(events[events.size()-1]=='T',"Translate precedes Dispatch for each removal");
 events+='D';++dispatch_count;
 if(enqueue){enqueue=0;queue.push_back(message(9));}
 if(recurse&&!depth){
  recurse=0;MsgStatus=0x800;depth=1;flush_timer();depth=0;
  require(MsgStatus==0,"nested flush clears rather than restores MsgStatus");
  nested_returned=1;
 }
 return -17;
}
// ACTUAL
void reset(){queue.clear();events.clear();MsgStatus=0xA5000000;depth=0;recurse=enqueue=nested_returned=0;translate_count=dispatch_count=0;last[0]=last[1]=0;}
void finish(const char* trace,int count){
 require(events==trace,"complete queue-drain event order");
 require(translate_count==count&&dispatch_count==count,"API return values do not truncate draining");
 require(MsgStatus==0,"final MsgStatus is zero not prior state");
}
int main(){
 reset();flush_timer();finish("P",0);
 reset();queue.push_back(message(1));queue.push_back(message(8,WM_USER));queue.push_back(message(2));
 flush_timer();finish("PTDPTDP",2);require(queue.size()==1&&queue[0].wParam==8,"unrelated messages remain queued");
 reset();queue.push_back(message(1));enqueue=1;flush_timer();finish("PTDPTDP",2);
 reset();queue.push_back(message(1));queue.push_back(message(2));queue.push_back(message(3));recurse=1;
 flush_timer();finish("PTDPTDPTDPP",3);
 return 0;
}
'''


def run_host(tmp_path, compiler, body):
    path = tmp_path / "drain.cpp"
    path.write_text(HARNESS.replace("// ACTUAL", body))
    output = tmp_path / "drain"
    result = subprocess.run([compiler,"-std=c++98","-O0",str(path),"-o",str(output)],capture_output=True,text=True)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(output)],capture_output=True,text=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_complete_flush_body(tmp_path, compiler):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    result = run_host(tmp_path,compiler,actual())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("old,new,witness", [
    ("MsgStatus |= 0x3F;","MsgStatus = 0x3F;","OR preserves"),
    ("NULL, WM_USER + 1, WM_USER + 1, PM_REMOVE","(HWND)1, WM_USER + 1, WM_USER + 1, PM_REMOVE","precise filtered"),
    ("WM_USER + 1, WM_USER + 1","WM_USER, WM_USER + 1","precise filtered"),
    ("WM_USER + 1, WM_USER + 1","WM_USER + 1, WM_USER + 2","precise filtered"),
    ("PM_REMOVE)","0)","precise filtered"),
    ("TranslateMessage(&msg);\n        DispatchMessage(&msg);","DispatchMessage(&msg);\n        TranslateMessage(&msg);","Translate precedes"),
    ("while (PeekMessage","if (PeekMessage","complete queue-drain"),
    ("TranslateMessage(&msg);","if (!TranslateMessage(&msg)) break;","complete queue-drain"),
    ("DispatchMessage(&msg);","if (DispatchMessage(&msg)) break;","complete queue-drain"),
    ("MsgStatus |= 0x3F;","unsigned int prior = MsgStatus; MsgStatus |= 0x3F;","final MsgStatus"),
])
def test_compiling_flush_damage_controls(tmp_path,old,new,witness):
    body = actual()
    assert body.count(old) == 1
    body = body.replace(old,new)
    if "prior =" in new: body = body.replace("MsgStatus = 0;","MsgStatus = prior;")
    result = run_host(tmp_path,"g++",body)
    assert result.returncode == 43, result.stderr
    assert "WITNESS: " + witness in result.stderr


def decode_complete(data,start,end):
    rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(data,start))
    cursor=start
    for row in rows:
        assert row.address==cursor,"complete contiguous pinned decode"
        cursor+=row.size
    assert cursor==end,"complete contiguous pinned decode"
    return {row.address:row for row in rows}


def test_pinned_complete_spans_and_message_boundary_anchors():
    paths=[Path(os.environ["OPENSMACX_IMAGE"])] if "OPENSMACX_IMAGE" in os.environ else [p/".opensmacx/game/terranx_original.exe" for p in (ROOT,*ROOT.parents)]
    path=next((p for p in paths if p.is_file()),None)
    if path is None: pytest.skip("proprietary pinned image unavailable")
    image=PeImage(path.read_bytes(),PIN)
    rows=decode_complete(image.read(Span(0x5FD370,0x5FD3F2)),0x5FD370,0x5FD3F2)
    assert len(rows)==44
    for address,text in {0x5FD370:"ecx, dword ptr [0x9b7b9c]",0x5FD37D:"ecx, 0x3f",0x5FD381:"esi, dword ptr [0x669358]",0x5FD3A3:"ebx, dword ptr [0x669340]",0x5FD3AA:"edi, dword ptr [0x66935c]",0x5FD3D9:"dword ptr [0x9b7b9c], eax",0x5FD3E3:"dword ptr [0x9b7b9c], 0"}.items():
        assert rows[address].op_str==text
    for address in (0x5FD39C,0x5FD3B5,0x5FD3BC,0x5FD3D1): assert rows[address].mnemonic=="call"
    assert rows[0x5FD3D5].mnemonic=="jne" and rows[0x5FD3D5].op_str=="0x5fd3b0"
    assert rows[0x5FD3A0].mnemonic=="je" and rows[0x5FD3A0].op_str=="0x5fd3e3"
    for address,operand in ((0x5FD387,"1"),(0x5FD389,"0x401"),(0x5FD38E,"0x401"),(0x5FD393,"0"),
                            (0x5FD3BE,"1"),(0x5FD3C0,"0x401"),(0x5FD3C5,"0x401"),(0x5FD3CE,"0")):
        assert rows[address].mnemonic=="push" and rows[address].op_str==operand
    for start,end,edge in CALLERS:
        row=decode_complete(image.read(Span(start,end)),start,end)[edge]
        assert row.mnemonic==("jmp" if start==0x616730 else "call")
        assert row.op_str=="0x5fd370"


def check_boundary(code,relocations,symbols,flush,kind):
    """Only this subject's direct rel32 edge; never helper-existence closure."""
    rows=decode_complete(code,0,len(code))
    edges=[]
    for relocation in relocations:
        name=symbols[int(relocation.target)].name
        assert not any(s in name for s in ("MsgStatus","PeekMessage","TranslateMessage","DispatchMessage")),"timer subject must not own message loop"
        if name==flush:
            assert relocation.kind==RelocationKind.RELATIVE32,"flush reference must be relative32"
            row=rows.get(relocation.offset-1)
            assert row is not None and row.mnemonic in ("call","jmp") and row.size==5,"flush reference must be direct branch"
            assert code[relocation.offset:relocation.offset+4]==b"\0\0\0\0","flush branch must target exact entry"
            edges.append(row.mnemonic)
    assert edges==[kind],"exactly one required flush call boundary"


def test_native_all_seven_subjects_keep_direct_flush_boundary(timer_objects):
    _,_,objects=timer_objects
    flush=symbol_at("src/timer_messages.cpp",0x5FD370)
    for obj in objects["src/jackal/time.cpp"]:
        for start,_,_ in CALLERS:
            code,relocations,_=obj.subject(obj.function(symbol_at("src/jackal/time.cpp",start)))
            check_boundary(code,relocations,obj.symbols,flush,"jmp" if start==0x616730 else "call")


@pytest.mark.parametrize("code,relocs,kind,witness",[
    (b"\xc3",[],"call","exactly one"),
    (b"\xe9\0\0\0\0",[(1,0)],"call","exactly one"),
    (b"\xe8\0\0\0\0\xe8\0\0\0\0",[(1,0),(6,0)],"call","exactly one"),
    (b"\xb8\0\0\0\0",[(1,0)],"call","direct branch"),
    (b"\xe8\x04\0\0\0",[(1,0)],"call","target exact entry"),
    (b"\xe8\0\0\0\0\xb8\0\0\0\0",[(1,0),(6,1)],"call","must not own message loop"),
])
def test_direct_boundary_edge_loss_controls(code,relocs,kind,witness):
    symbols={0:SimpleNamespace(name="flush"),1:SimpleNamespace(name="__imp__PeekMessageA@20")}
    with pytest.raises(AssertionError,match=witness):
        check_boundary(code,[SimpleNamespace(offset=o,target=t,kind=RelocationKind.RELATIVE32) for o,t in relocs],symbols,"flush",kind)


@pytest.mark.parametrize("opcode,kind",[(b"\xe8","call"),(b"\xe9","jmp")])
def test_direct_boundary_positive_controls(opcode,kind):
    check_boundary(opcode+b"\0\0\0\0",[SimpleNamespace(offset=1,target=0,kind=RelocationKind.RELATIVE32)],{0:SimpleNamespace(name="flush")},"flush",kind)


def test_direct_boundary_rejects_absolute_relocation():
    with pytest.raises(AssertionError,match="relative32"):
        check_boundary(b"\xe8\0\0\0\0",[SimpleNamespace(offset=1,target=0,kind=RelocationKind.ABSOLUTE32)],
                       {0:SimpleNamespace(name="flush")},"flush","call")
