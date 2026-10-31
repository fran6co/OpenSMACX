"""Actual Console navigation/editor commands; host doubles do not prove ABI."""
from pathlib import Path
import shutil
import subprocess
import pytest
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/console_commands.cpp'


def body(text, signature):
    start=text.index(signature);end=text.index('{',start)+1;depth=1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]


HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl
struct Base {int16_t x,y;} Bases[2];
struct Fields {int base_current_count_;uint32_t game_state_,game_rules_;};
struct {Fields fields_;} GameDataStateStorage;
static int GoToPreviewActive,GoToSourceX,GoToSourceY;
static int InfoFactionTextColors[8]={0,101,202};
static int center_calls,line_calls,redraws,backs,shrink;
static int XAnswer=1,IsMultiplayerNet,IsMultiplayerPBEM;
static int IfaceIdToSlotStorage[2],ActiveIfaceIdStorage;
static bool flip_menu,flip_save;
static int shown,hidden,group_value,updates,saves,pops,prompts;
static std::vector<int> events;
struct Menu {void check(int n);};
struct Console {
 int faction_id_,selected_vehicle_;Menu alpha_menu_;
 static void goto_routine(int);void editor_on();
 void update_data(int n){assert(n==0);++updates;events.push_back(9);}
} ConsoleGlobal;
struct Plan {
 void clear_lines(){++backs;events.push_back(1);}
 void back_redraw(){events.push_back(2);}
 void on_redraw(){++redraws;events.push_back(3);}
 void mutate(){Bases[0].x=-13;Bases[0].y=34;ConsoleGlobal.faction_id_=2;
 GoToSourceX=1;GoToSourceY=2;if(shrink)GameDataStateStorage.fields_.base_current_count_=0;}
 void monocenter(int x,int y,int z){assert(x==11&&y==22&&z==0);++center_calls;mutate();}
 void opticenter(int sx,int sy,int x,int y,int z){assert(sx==7&&sy==9&&x==11&&y==22&&z==0);++center_calls;mutate();}
 void add_line(int sx,int sy,int x,int y,int color){++line_calls;
 assert(x==-13&&y==34&&color==202);
 if(GoToPreviewActive)assert(sx==1&&sy==2);else assert(sx==-13&&sy==34);}
} PlanWindow;
void Menu::check(int n){assert(n==17);events.push_back(5);
 if(flip_menu)GameDataStateStorage.fields_.game_state_=0x20;}
struct Message {void pop(const char *s,int ms,int z,void *p){assert(ms==5000&&z==0&&!p);++pops;
 if(!std::strcmp(s,"EDITORON"))events.push_back(1);
 else if(!std::strcmp(s,"EDITOROFF"))events.push_back(2);
 else {assert(!std::strcmp(s,"EDITORNET"));events.push_back(10);}}} NetMsgPrimary;
static int X_pop(const char *s,void *p){assert(!std::strcmp(s,"SCENEDITOR")&&!p);++prompts;return XAnswer;}
static void auto_save(){++saves;if(flip_save)GameDataStateStorage.fields_.game_state_=0x20;}
struct Flat {void show(int n){assert(n==0);++shown;events.push_back(4);}void hide(){++hidden;events.push_back(4);}};
struct Group {void set(int n,int state){assert(state==1);group_value=n;events.push_back(6);}};
struct Interface {Flat f;Group g;Flat &flat_button(int n){assert(n==2);return f;}
 Group &button_group(int n){assert(n==0);return g;}} MainInterfaceGlobal;
static void set_dirty(){events.push_back(7);}
static void draw_map(int n){assert(n==1);events.push_back(8);}
struct GraphicWin {void redraw(){events.push_back(11);}};
struct World:GraphicWin {} WorldClimateWorldWin;
static void desktop_update(){events.push_back(12);}
/* GOTO */
/* EDITOR */
static void reset(){center_calls=line_calls=redraws=backs=shrink=0;
GoToSourceX=7;GoToSourceY=9;Bases[0]={11,22};GameDataStateStorage.fields_={1,0x20,0};
ConsoleGlobal.faction_id_=1;ConsoleGlobal.selected_vehicle_=17;
shown=hidden=group_value=updates=saves=pops=prompts=0;flip_menu=flip_save=false;
IsMultiplayerNet=IsMultiplayerPBEM=0;XAnswer=1;events.clear();}
int main(){
 for(int preview=0;preview<2;++preview){reset();GoToPreviewActive=preview;
 Console::goto_routine(0);assert(center_calls==1&&line_calls==1&&redraws==1);}
 reset();GoToPreviewActive=1;shrink=1;Console::goto_routine(0);
 assert(center_calls==1&&line_calls==0&&redraws==1);
 for(int id:{-1,1}){reset();Console::goto_routine(id);assert(!center_calls&&!line_calls&&redraws==1);
 assert((events==std::vector<int>{1,2,3}));}
 for(bool flip:{false,true}){reset();flip_menu=flip;ConsoleGlobal.editor_on();
 assert(shown==1&&!hidden&&updates==1&&pops==1);
 assert(group_value==(flip?1001:1002));assert(events.front()==1&&events.back()==9);}
 reset();GameDataStateStorage.fields_.game_state_=0xe0;ConsoleGlobal.editor_on();
 assert(!shown&&hidden==1&&group_value==1001&&updates==1&&events.front()==2);
 reset();GameDataStateStorage.fields_.game_state_=0;IsMultiplayerNet=1;ConsoleGlobal.editor_on();
 assert(pops==1&&!updates&&events==std::vector<int>{10});
 reset();GameDataStateStorage.fields_.game_state_=0;XAnswer=0;ConsoleGlobal.editor_on();
 assert(prompts==1&&!pops&&!updates&&!GameDataStateStorage.fields_.game_state_);
 reset();GameDataStateStorage.fields_.game_rules_=0x400;flip_save=true;ConsoleGlobal.editor_on();
 assert(saves==1&&events.front()==2&&hidden==1&&group_value==1001);
}
'''


def compile_run(tmp_path,compiler,text):
    cpp=tmp_path/'commands.cpp';exe=tmp_path/'commands'
    fixture=HARNESS.replace('/* GOTO */',body(text,'void __cdecl Console::goto_routine('))
    fixture=fixture.replace('/* EDITOR */',body(text,'void Console::editor_on()'))
    cpp.write_text(fixture)
    result=subprocess.run([compiler,'-std=c++11','-O2','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert result.returncode==0,result.stdout+result.stderr
    return subprocess.run([str(exe)],text=True,capture_output=True)


@pytest.mark.parametrize('compiler',('g++','clang++'))
def test_actual_live_navigation_and_editor_callbacks(compiler,tmp_path):
    if not shutil.which(compiler):pytest.skip(compiler+' unavailable')
    result=compile_run(tmp_path,compiler,SOURCE.read_text())
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize('damage',('cached-coordinates','wrong-selection'))
def test_callback_effect_damage_is_rejected(damage,tmp_path):
    if not shutil.which('g++'):pytest.skip('g++ unavailable')
    text=SOURCE.read_text()
    if damage=='cached-coordinates':
        start=text.index('void __cdecl Console::goto_routine(');end=text.index('// Superseded',start)
        section=text[start:end]
        section=section.replace('    if (GoToPreviewActive)',
            '    const int cached_x = Bases[base_id].x, cached_y = Bases[base_id].y;\n    if (GoToPreviewActive)',1)
        # Leave the center-call inputs intact and damage only the later add_line arguments.
        section=section.replace('PlanWindow.add_line(Bases[base_id].x, Bases[base_id].y,\n                        Bases[base_id].x, Bases[base_id].y,',
            'PlanWindow.add_line(cached_x, cached_y, cached_x, cached_y,',1)
        assert section!=text[start:end]
        text=text[:start]+section+text[end:]
    else:
        old='MainInterfaceGlobal.button_group(0).set(1002, 1);'
        assert text.count(old)==1
        text=text.replace(old,old.replace('1002','1001'),1)
    result=compile_run(tmp_path,'g++',text)
    assert result.returncode!=0


def test_pinned_center_calls_precede_live_coordinate_reloads():
    import os
    from hashlib import sha256
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_MEM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE",str(ROOT/".opensmacx/game/terranx_original.exe")))
    if not path.is_file():pytest.skip("pinned image unavailable")
    raw=path.read_bytes();pin="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest()==pin
    image=PeImage(raw,pin);decoder=Cs(CS_ARCH_X86,CS_MODE_32);decoder.detail=True
    for site,target,reads in ((0x4D6141,0x48B8E0,(0x4D614C,0x4D6153)),
                              (0x4D60D0,0x48B630,(0x4D60E6,0x4D60F4))):
        call=next(decoder.disasm(image.read(Span(site,site+5)),site))
        assert call.mnemonic=="call" and call.operands[0].imm==target
        displacements=[]
        for address in reads:
            instruction=next(decoder.disasm(image.read(Span(address,address+8)),address))
            assert instruction.mnemonic=="movsx"
            memory=instruction.operands[1]
            assert memory.type==X86_OP_MEM and memory.size==2
            displacements.append(memory.mem.disp)
        assert displacements==[0x97D042,0x97D040]
    guard=next(decoder.disasm(image.read(Span(0x4D60D5,0x4D60DB)),0x4D60D5))
    assert guard.mnemonic=="cmp" and guard.operands[1].mem.disp==0x9A64CC
