"""Full InfoWin and palette-initializer source bodies, host service controls.

The initializer's original signed pointer-to-int comparisons run in a non-PIE
host executable with the table below INT_MAX. GCC -fpermissive admits the
unchanged original spelling. This is not VC6, native Buffer/PCX/palette, x87,
invalid-pointer-path, byte-match or game-runtime acceptance. The companion
draw_text_x_lifetime control preserves the defined cross-row zero-budget case.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_infowin_draw_text_x_lifetime import function_source


ROOT = Path(__file__).resolve().parents[2]


def initializer():
    source = (ROOT / "src/interface_palette.cpp").read_text()
    start = source.index("void __cdecl textcolor_init()")
    return source[start:source.index("\n}",start)+2]


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <climits>
#include <vector>
#define __cdecl
struct RECT {int left,top,right,bottom;};
void require(bool b,const char*w){if(!b){fprintf(stderr,"WITNESS: %s\n",w);exit(19);}}
int TextColourTable[195],epoch,loads,sets,samples,closes;
int FakeZero[195],Disconnected[195];
struct Pair {int a,b;Pair(int x,int y):a(x),b(y){}};
std::vector<Pair> primary,secondary;
int writes,right_writes,resets,updates,arrows,mutate_secondary,reinit_on_write;
int InfoFactionTextColors[8],InfoFactionShadowColors[8];
struct GameDataFields {int faction_rankings_[9];};
struct GameDataState {GameDataFields fields_;} GameDataStateStorage;
int swatch(int x,int y){return x==18&&y==101?9:(epoch+3*x+5*y)%251;}
struct Buffer;
Buffer *sampler;
void __cdecl textcolor_init();
struct Buffer {
 ~Buffer(){if(this==sampler){++closes;sampler=0;}}
 int load_pcx(const char*name,int palette,int base,int count){
  require(strcmp(name,"text.pcx")==0&&palette==0&&base==10&&count==236,"initializer load contract");
  ++loads;sampler=this;return 17; // original intentionally ignores failure
 }
 int get_pixel(int x,int y){require(this==sampler&&sets==loads,"sample after palette publish");++samples;return swatch(x,y);}
 void set_font(void*,int,int,int){}
 void set_clip(RECT*){}
 void restore_clip(){}
 void update(RECT*,int){++updates;}
 int text_width(const char*s){return strlen(s)*3;}
 void set_text_color(int a,int b,int c,int d){
  require(c==1&&d==1,"primary flags");primary.push_back(Pair(a,b));
  if(mutate_secondary){TextColourTable[26]=901;TextColourTable[27]=902;mutate_secondary=0;}
 }
 void set_text_color2(int a,int b,int c,int d){require(c==1&&d==1,"secondary flags");secondary.push_back(Pair(a,b));}
 int write_l(char*,int x,int,int){++writes;if(reinit_on_write){reinit_on_write=0;epoch+=41;textcolor_init();}return x+7;}
 int write_right_l(char*,int,int,int,int){++right_writes;return 0;}
 void reset_text_state(){++resets;}
};
struct Palette {void set(){++sets;}} g_PALETTE1;
struct MainInterface:Buffer{} MainInterfaceGlobal;
struct Tutorial {void draw_arrow(){++arrows;}} TutorialWindow;
struct InfoWin {
 void*font_;int row_count_[2],visible_characters_[2],pane_states_[2];
 RECT pane_rects_[2],bounds_;
 char*row_text_[2][4];int row_color_[2][4],row_reveal_color_[2][4];
 int row_y_[2][4],row_x_[2][4],field_94_;
 void draw_text(int);
};
@INIT@
@DRAW@
int sampled_index(int index){int row=(index-12)/2;int c=swatch(index%2?18:8,21+16*row);return index%2&&c==9?-1:c;}
void fresh(){
 primary.clear();secondary.clear();writes=right_writes=resets=updates=arrows=mutate_secondary=reinit_on_write=0;
}
void pair_is(const std::vector<Pair>&v,int n,int a,int b,const char*w){
 require(n<(int)v.size()&&v[n].a==a&&v[n].b==b,w);
}
InfoWin setup(int budget=10){
 InfoWin win={};static char text[]="abcd";
 win.row_count_[0]=1;win.visible_characters_[0]=budget;
 win.pane_rects_[0].right=200;win.pane_rects_[0].bottom=100;
 win.row_text_[0][0]=text;win.row_x_[0][0]=20;return win;
}
int main(){
 require((uintptr_t)TextColourTable+sizeof(TextColourTable)<INT_MAX,"low nonPIE table address");
 for(int pass=0;pass<2;++pass){
  epoch=pass*31;textcolor_init();
  require(loads==pass+1&&sets==pass+1&&closes==pass+1&&sampler==0,"initializer lifetime and ignored failure");
  for(int i=22;i<30;++i)require(TextColourTable[i]==sampled_index(i),"initializer writes shared provider indices22..29");
  require(TextColourTable[23]==-1,"original transparent secondary swatch substitution");
  for(int reveal=0;reveal<2;++reveal){fresh();InfoWin win=setup();win.field_94_=reveal;
   win.draw_text(0);
   pair_is(primary,0,sampled_index(22+2*reveal),sampled_index(23+2*reveal),"normal/revealed primary provider");
   pair_is(secondary,0,sampled_index(26+2*reveal),sampled_index(27+2*reveal),"normal/revealed secondary provider");
   require(writes==1&&resets==0&&updates==1&&arrows==1,"full row path unchanged");
  }
 }
 fresh();InfoWin win=setup();mutate_secondary=1;win.draw_text(0);
 pair_is(secondary,0,901,902,"secondary reload after primary callback");
 // Prefix callback republishes the actual swatch provider before final-char
 // reads. The historical asymmetric pair is24/23, never24/25.
 fresh();win=setup(3);epoch=7;textcolor_init();int old22=sampled_index(22),old23=sampled_index(23);
 reinit_on_write=1;win.draw_text(0);
 pair_is(primary,0,old22,old23,"partial prefix old provider");
 pair_is(primary,1,sampled_index(24),sampled_index(23),"asymmetric final character24/23 live provider");
 pair_is(secondary,1,sampled_index(28),sampled_index(29),"final secondary28/29 live provider");
 require(writes==2&&resets==1,"positive partial row writes and reset");
 // Explicit and ranked colours retain their independent original routing.
 fresh();win=setup();win.row_color_[0][0]=61;win.row_reveal_color_[0][0]=62;win.draw_text(0);
 pair_is(primary,0,61,-1,"explicit normal row colour");require(secondary.empty(),"explicit no secondary publication");
 fresh();win.field_94_=1;win.draw_text(0);pair_is(primary,0,62,-1,"explicit revealed row colour");
 fresh();win=setup(3);win.row_color_[0][0]=61;win.row_reveal_color_[0][0]=62;win.draw_text(0);
 pair_is(primary,1,62,-1,"explicit partial final reveal colour");
 fresh();win=setup();static char empty[]="",rank[]="rank";win.row_count_[0]=2;
 win.row_text_[0][0]=empty;win.row_text_[0][1]=rank;win.pane_states_[0]=4;
 GameDataStateStorage.fields_.faction_rankings_[7]=2;InfoFactionTextColors[2]=71;InfoFactionShadowColors[2]=72;
 win.draw_text(0);pair_is(primary,0,71,72,"ranked faction colour unchanged");
 require(secondary.empty()&&right_writes==1&&writes==0,"ranked right alignment");
 fresh();win=setup();win.row_count_[1]=1;win.visible_characters_[1]=10;win.row_text_[1][0]=rank;
 win.draw_text(-1);require(writes==1,"shared row counter across panes retained");
 fresh();win=setup();win.visible_characters_[0]=-1;win.draw_text(0);
 require(primary.empty()&&writes==0&&updates==1,"negative budget skips rows but updates");
 require(samples>0,"actual initializer sampled pixels");
 puts("InfoWin shared palette controls passed");
}
'''


def run(tmp_path,damage=None):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("GCC host compiler required for unchanged pointer-to-int initializer")
    draw = function_source((ROOT / "src/infowin.cpp").read_text())
    if damage == "fake_zero":
        draw = draw.replace("TextColourTable[","FakeZero[")
    elif damage == "disconnected_copy":
        draw = draw.replace("TextColourTable[","Disconnected[")
        draw = draw.replace("    Buffer &buffer", "    static bool copied = false; if (!copied) { memcpy(Disconnected,TextColourTable,sizeof(Disconnected)); copied=true; }\n    Buffer &buffer",1)
    elif damage == "off_by_index":
        good = "TextColourTable[22 + primary * 2]"
        assert draw.count(good)==1
        draw = draw.replace(good,"TextColourTable[23 + primary * 2]")
    elif damage == "normalize_asymmetry":
        good = "TextColourTable[24], TextColourTable[23]"
        assert draw.count(good)==1
        draw = draw.replace(good,"TextColourTable[24], TextColourTable[25]")
    cpp,binary = tmp_path / "palette_view.cpp",tmp_path / "palette_view"
    cpp.write_text(HARNESS.replace("@INIT@",initializer()).replace("@DRAW@",draw))
    build = subprocess.run([compiler,"-std=c++98","-O0","-fpermissive","-fno-pie","-no-pie",
                            str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert build.returncode==0,build.stdout+build.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)


def test_actual_initializer_and_infowin_share_live_palette(tmp_path):
    result=run(tmp_path)
    assert result.returncode==0,result.stdout+result.stderr
    assert "InfoWin shared palette controls passed" in result.stdout


def test_actual_editor_reloads_same_palette_indices(tmp_path):
    from test_maininterface_map_metadata import run_control
    result=run_control(tmp_path)
    assert result.returncode==0,result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage",["fake_zero","disconnected_copy","off_by_index","normalize_asymmetry"])
def test_compiling_disconnected_or_wrong_colour_damage(tmp_path,damage):
    result=run(tmp_path,damage)
    witness="asymmetric final character24/23 live provider" if damage=="normalize_asymmetry" else "normal/revealed primary provider"
    assert result.returncode==19 and f"WITNESS: {witness}" in result.stderr,result.stdout+result.stderr


def test_complete_pinned_provider_and_consumer_spans():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    configured=os.environ.get("OPENSMACX_IMAGE")
    candidates=([Path(configured)] if configured else [])+[
        parent/".opensmacx/game/terranx_original.exe" for parent in (ROOT,*ROOT.parents)]
    path=next((p for p in candidates if p.is_file()),None)
    if path is None:pytest.skip("pinned proprietary image unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions={}
    for start,end in ((0x458420,0x4588A9),(0x4BA220,0x4BA5B3),(0x659BD0,0x659BE5),
                      (0x45B550,0x45BBF7)):
        cursor=start
        for ins in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start):
            assert ins.address==cursor
            cursor+=ins.size;instructions[ins.address]=(ins.mnemonic,ins.op_str)
        assert cursor==end
    anchors={
        0x45B59A:("mov","ecx, dword ptr [0x8c6bb4]"),
        0x45B5A3:("mov","eax, dword ptr [0x8c6bb8]"),
        0x4585A0:("mov","edx, dword ptr [0x8c6bc0]"),
        0x4585A6:("mov","eax, dword ptr [0x8c6bbc]"),
        0x4585B7:("mov","ecx, dword ptr [0x8c6bd0]"),
        0x4585BD:("mov","edx, dword ptr [0x8c6bcc]"),
        0x4585CB:("mov","eax, dword ptr [0x8c6bb8]"),
        0x4585D0:("mov","ecx, dword ptr [0x8c6bb4]"),
        0x4585E2:("mov","edx, dword ptr [0x8c6bc8]"),
        0x4585E8:("mov","eax, dword ptr [0x8c6bc4]"),
        0x4587A2:("mov","edx, dword ptr [0x8c6bb8]"),
        0x4587A8:("mov","eax, dword ptr [0x8c6bbc]"),
        0x4BA27A:("mov","esi, 0x8c6b90"),
        0x4BA296:("mov","dword ptr [esi - 4], eax"),
        0x4BA29E:("cmp","eax, 9"),
        0x4BA2A5:("mov","dword ptr [esi], 0xffffffff"),
        0x4BA2AB:("add","esi, 8"),
        0x4BA2B1:("cmp","esi, 0x8c6bd8"),
        0x4BA2B7:("jl","0x4ba27f"),
    }
    for address,expected in anchors.items():assert instructions[address]==expected,hex(address)
    assert [0x8C6B5C+index*4 for index in range(22,30)]==list(range(0x8C6BB4,0x8C6BD4,4))
