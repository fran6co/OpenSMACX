"""Actual Newgame preview and constructor-snapshot callback witnesses.

The doubles check original sequencing and operands, not the full Popup/SetupWin
ABI, original allocation sizes, CRT implementation or game runtime.
"""
from pathlib import Path
import os
import shutil
import subprocess
import pytest
import test_newgame_menu as menus

ROOT=Path(__file__).resolve().parents[2]


def body(signature):
    return menus.definition((ROOT/'src/newgame.cpp').read_text(),signature)


def run(tmp_path,compiler,text):
    cxx=shutil.which(compiler)
    if not cxx:pytest.skip(f'{compiler} unavailable')
    source=tmp_path/'control.cpp';exe=tmp_path/'control';source.write_text(text)
    built=subprocess.run([cxx,'-std=c++17','-O2','-Wall','-Wextra','-Werror',
                          '-fsanitize=undefined','-fno-sanitize-recover=all',
                          str(source),'-o',str(exe)],capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True)


PREVIEW=r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
struct Sprite{int id;};
struct Font{};Font normal,bold,italic;
Font *PopupMenuFont=&normal,*PopupMenuBoldFont=&bold,*PopupMenuItalicFont=&italic;
int ExpansionEnabled,ConfigFactionDetailMode,PopupModalActive,starts;
bool mutate_detail;
unsigned int picture_count;
char StringTemp[256];
std::vector<std::string> events;
struct Player{char filename[16],formal_name_faction[32];}Players[4];
struct Art{Sprite diplomacy;}FactionsArt[4];
struct Win{void set_caption(char *s){assert(std::strcmp(s,"formal")==0);events.push_back("caption");}};
struct GraphicWin:Win{void redraw(){events.push_back("redraw");}};
struct BasePop:GraphicWin{
 void exec(int n,void *p){assert(!n&&!p&&PopupModalActive==1);events.push_back("exec");PopupModalActive=17;}
};
struct SpriteBox{void sprite(Sprite *s,int a,int b){assert(s==&FactionsArt[1].diplomacy&&!a&&!b);events.push_back("sprite");}};
struct Dialogs:SpriteBox{};
struct Popup:BasePop{
 Dialogs rows;
 void close(){events.push_back("close");}
 void start(char *file,char *label,int a,int b,int flags,int owner){
  assert(file==Players[1].filename&&a==-1&&!b&&!owner);
  events.push_back(std::string("start:")+label);++starts;
  assert(flags==(starts==1?0x10500:0x80));
  if(mutate_detail&&starts==1)ConfigFactionDetailMode=1;
 }
 bool picture_enabled() const{return picture_count!=0;}
 void set_caller_picture(Sprite *s){assert(s==(picture_count?nullptr:&FactionsArt[1].diplomacy));events.push_back("picture");}
 Dialogs &dialogs(){return rows;}
 void set_string_font(Font *n,Font *b,Font *i,int z){assert(n==&normal&&b==&bold&&i==&italic&&!z);events.push_back("font");}
 void set_loc(int x,int y){assert(x==5&&y==0x136&&PopupModalActive==1);events.push_back("loc");}
}ConfigFactionPreview;
GraphicWin PickWinGlobal;
'''


@pytest.mark.parametrize('compiler',['g++','clang++'])
@pytest.mark.parametrize('mutation',[None,'label','cached_detail','picture','modal'])
def test_preview_live_callbacks_and_original_operands(tmp_path,compiler,mutation):
    source=body('void __cdecl thumb_routine(int faction)')
    if mutation=='label':
        assert source.count('strcat(StringTemp, "BLURB");')==1
        source=source.replace('strcat(StringTemp, "BLURB");','strcat(StringTemp, "DATALINKS1");',1)
    elif mutation=='cached_detail':
        anchor='    StringTemp[0] = 0;'
        assert anchor in source
        source=source.replace(anchor,'    const int cached = ConfigFactionDetailMode;\n'+anchor,1)
        anchor='    if (ConfigFactionDetailMode) {'
        assert source.count(anchor)==1
        source=source.replace(anchor,'    if (cached) {',1)
    elif mutation=='picture':
        anchor='? 0 : diplomacy'
        assert source.count(anchor)==1
        source=source.replace(anchor,'? diplomacy : 0',1)
    elif mutation=='modal':
        anchor='    PopupModalActive = 1;'
        assert source.count(anchor)==1
        source=source.replace(anchor,'    PopupModalActive = 0;',1)
    main=r'''
int main(){
 for(int expansion=0;expansion<2;++expansion)for(int detail=0;detail<2;++detail)
 for(int mutation=0;mutation<2;++mutation)for(unsigned int count:{0u,1u,0xffffffffu}){
  ExpansionEnabled=expansion;ConfigFactionDetailMode=detail;mutate_detail=mutation;picture_count=count;
  starts=0;events.clear();PopupModalActive=31;
  std::strcpy(Players[1].filename,"FACTION");std::strcpy(Players[1].formal_name_faction,"formal");
  thumb_routine(1);
  std::vector<std::string> expected;
  if(expansion)expected.push_back("redraw");
  expected.push_back(detail?"start:DATALINKS1":"start:BLURB");
  if(detail||mutation)expected.push_back("start:DATALINKS2");
  for(const char *tag:{"picture","sprite","caption","font","loc","exec"})expected.push_back(tag);
  assert(events==expected&&PopupModalActive==0);
 }
 ExpansionEnabled=1;events.clear();PopupModalActive=31;std::strcpy(Players[1].filename,"JENN282");
 thumb_routine(1);assert((events==std::vector<std::string>{"redraw","close"})&&PopupModalActive==31);
}
'''
    result=run(tmp_path,compiler,PREVIEW+source+main)
    assert (result.returncode==0)==(mutation is None),result.stderr


@pytest.mark.parametrize('compiler',['g++','clang++'])
@pytest.mark.parametrize('mutation',[False,True])
def test_map_menu_refreshes_snapshot_after_popup_constructor(tmp_path,compiler,mutation):
    source=menus.projected('map_menu')
    assert 'snapshot_probe = &opening_window_snapshot_' in source
    if mutation:
        anchor='    setup.opening_window_snapshot_ = opening_window();'
        assert source.count(anchor)==1
        source=source.replace(anchor,'',1)
    result=run(tmp_path,compiler,source)
    assert (result.returncode==0)==(not mutation),result.stderr


def test_pinned_original_preview_labels_picture_and_map_snapshot():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not image.is_file():pytest.skip('pinned original unavailable')
    pe=PeImage(image.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    thumb={i.address:(i.mnemonic,i.op_str) for i in md.disasm(pe.read(Span(0x5898a0,0x589a24)),0x5898a0)}
    for address,string in [(0x68f5b8,b'JENN282\0'),(0x68f5c0,b'DATALINKS1\0'),(0x68f5cc,b'BLURB\0'),(0x68f5d4,b'DATALINKS2\0')]:
        assert pe.read(Span(address,address+len(string)))==string
    assert thumb[0x5898fb]==('push','0x68f5c0')
    assert thumb[0x589902]==('push','0x68f5cc')
    assert thumb[0x58994c]==('je','0x589982')
    assert thumb[0x5899ab]==('mov','dword ptr [0x942524], ecx')
    assert thumb[0x5899f8]==('mov','dword ptr [0x9469fc], 1')
    menu={i.address:(i.mnemonic,i.op_str) for i in md.disasm(pe.read(Span(0x58d6b0,0x58d72f)),0x58d6b0)}
    assert menu[0x58d6ed]==('call','0x4048a0')
    assert menu[0x58d6f2]==('mov','eax, dword ptr [0x945824]')
    assert menu[0x58d6ff]==('mov','dword ptr [ebp - 0xc6c], eax')
