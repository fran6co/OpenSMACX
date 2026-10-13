"""Host behavior controls over extracted PrefWin volume production methods.

Recording doubles stand in only for UI/audio services. These tests do not prove
the original ABI, native float/error effects, private RECT load ordering, or
complete game behavior. The music probe verifies the C++ virtual-delete
expression and order, not the original FactionAmbience vftable/class ABI.
"""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2]))
UNIT = ROOT / "src/prefwin.cpp"


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated source definition: {signature}")


def compile_run(tmp_path: Path, text: str, stem: str) -> subprocess.CompletedProcess[str]:
    compiler = (os.environ.get("OPENSMACX_HOST_CXX") or shutil.which("g++")
                or shutil.which("clang++"))
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    cpp, binary = tmp_path / f"{stem}.cpp", tmp_path / stem
    cpp.write_text(text)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, text=True)


def actual_source() -> str:
    return UNIT.read_text()


def label_functions(text: str) -> str:
    return "\n".join((braced(text, "int label_index(int byte_offset)"),
                      braced(text, "void PrefWin::change_volume_button_text(int button_id)")))


TEXT = r'''
#include <cassert>
#include <climits>
#include <string>
#include <vector>
std::vector<std::string> events;
int null_index=-1;
struct FlatButton {
 const char *tag; int selected=0; const char *name=nullptr;
 int selected_state()const{return selected;}
 void set_selected_state(int x){selected=x;events.push_back(std::string(tag)+".state:"+std::to_string(x));}
 void set_name(const char *x){name=x;events.push_back(x?std::string(tag)+".name:"+x:std::string(tag)+".name:<null>");}
};
struct PrefWin {
 FlatButton volumeMainToggle_{"main"},volumeVoiceToggle_{"voice"},volumeSFXToggle_{"sfx"},volumeMusicToggle_{"music"};
 void change_volume_button_text(int); void change_volume_button_state(int); void on_button_clicked(int); void redraw(); void destroy(); void hide_option_buttons(); void get_preferences();
};
const char *label_get(int i){events.push_back("label:"+std::to_string(i));static const char *s[]={"main-off","main-on","voice-off","voice-on","sfx-off","sfx-on","music-off","music-on"};if(i==null_index)return nullptr;return s[i-1080];}
'''


@pytest.mark.parametrize("button,selected,tag,index,label", [
    (0x55,0,"main",1080,"main-off"),(0x55,1,"main",1081,"main-on"),
    (0x57,0,"voice",1082,"voice-off"),(0x57,1,"voice",1083,"voice-on"),
    (0x56,0,"sfx",1084,"sfx-off"),(0x56,1,"sfx",1085,"sfx-on"),
    (0x58,0,"music",1086,"music-off"),(0x58,1,"music",1087,"music-on"),
])
def test_valid_id_selected_label_mapping(button,selected,tag,index,label,tmp_path):
    member={"main":"volumeMainToggle_","voice":"volumeVoiceToggle_","sfx":"volumeSFXToggle_","music":"volumeMusicToggle_"}[tag]
    main=f'''int main(){{PrefWin w;w.{member}.selected={selected};w.change_volume_button_text({button});
 assert(events.size()==2&&events[0]=="label:{index}"&&events[1]=="{tag}.name:{label}");
 assert(std::string(w.{member}.name)=="{label}");}}'''
    result=compile_run(tmp_path,TEXT+label_functions(actual_source())+main,f"label-{button:x}-{selected}")
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("invalid",[-(1<<31),-1,0,0x54,0x59,(1<<31)-1])
def test_invalid_signed_ids_are_noop(invalid,tmp_path):
    main=f'int main(){{PrefWin w;w.change_volume_button_text({invalid});assert(events.empty());}}'
    result=compile_run(tmp_path,TEXT+label_functions(actual_source())+main,f"invalid-{abs(invalid)}")
    assert result.returncode==0,result.stderr


def test_null_label_is_forwarded_unconditionally(tmp_path):
    main=r'''int main(){PrefWin w;w.volumeMainToggle_.selected=1;null_index=1081;w.change_volume_button_text(0x55);
 assert((events==std::vector<std::string>{"label:1081","main.name:<null>"}));}'''
    result=compile_run(tmp_path,TEXT+label_functions(actual_source())+main,"null-label")
    assert result.returncode==0,result.stderr


STATE = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
std::vector<std::string> events;
const uint32_t PREF_AV_VOLUME_MUSIC_TOGGLE=0x800;
struct FlatButton{const char *tag;int selected=0;const char *name=nullptr;int selected_state()const{return selected;}
 void set_selected_state(int x){selected=x;events.push_back(std::string(tag)+".state:"+std::to_string(x));}
 void set_name(const char*x){name=x;events.push_back(x?std::string(tag)+".name:"+x:std::string(tag)+".name:<null>");}};
struct Scroll{void set_pos(int){}};
struct PrefWin{
 FlatButton volumeMainToggle_{"main"},volumeVoiceToggle_{"voice"},volumeSFXToggle_{"sfx"},volumeMusicToggle_{"music"};
 Scroll volumeMainScroll_,volumeVoiceScroll_,volumeSFXScroll_,volumeMusicScroll_;
 uint32_t prefsAVPending_=0xFFFF,morePrefsAVPending_=0xFFFF,warningsPending_=0;
 uint32_t prefsBasicPending_=0,prefsAdvPending_=0,prefsAutoPending_=0,prefsMapPending_=0;
 uint32_t morePrefsBasicPending_=0,morePrefsAdvPending_=0,morePrefsAutoPending_=0,morePrefsMapPending_=0;
 int field_A5C_=0,field_A18_=0,field_A14_=4;
 void change_volume_button_text(int);void change_volume_button_state(int);void on_button_clicked(int);void redraw();void destroy();void hide_option_buttons();void get_preferences();
};
struct WaveDevice{void disable_group(int n){events.push_back("disable-group:"+std::to_string(n));}void enable_group(int n){events.push_back("enable-group:"+std::to_string(n));}void disable(){events.push_back("disable");}void enable(){events.push_back("enable");}void set_volume(int){}void set_group_volume(int,int){}} WaveDeviceGlobal;
struct MusicBase{virtual ~MusicBase(){events.push_back("base-dtor");}virtual void begin(){events.push_back("begin");}virtual void set_volume(int){}};
struct MusicProbe:MusicBase{~MusicProbe()override{events.push_back("derived-dtor");}};MusicBase *FactionMusic=nullptr;
struct Console{int faction_id_=3;} ConsoleGlobal;
void load_music(int id){events.push_back("load:"+std::to_string(id));FactionMusic=new MusicProbe;}
const char *label_get(int i){events.push_back("label:"+std::to_string(i));return "label";}
'''


def state_functions(text: str, click=False) -> str:
    signatures=["int label_index(int byte_offset)",
                "void PrefWin::change_volume_button_text(int button_id)",
                "void PrefWin::change_volume_button_state(int button_id)"]
    if click: signatures.append("void PrefWin::on_button_clicked(int button_id)")
    return "\n".join(braced(text,sig) for sig in signatures)


SUPPORT = r'''
struct Gamma{double exec(void*,float,int){return 1.0;}};struct Palette{void copy_from(void*){}}g_PALETTE1,g_PALETTE2;
float PrefWinGammaCorrection=1.0f,PrefWinGammaScale=100.0f;void prefs_put(const char*,int,int){}
uint32_t default_prefs(){return 0;}uint32_t default_prefs2(){return 0;}uint32_t default_warn(){return 0;}
void PrefWin::redraw(){events.push_back("redraw");}void PrefWin::destroy(){}void PrefWin::hide_option_buttons(){}void PrefWin::get_preferences(){}
'''


@pytest.mark.parametrize("initial,button,expected",[
 (0,0x57,["voice.state:1","main.state:1","label:1081","main.name:label","enable-group:0"]),
 (1,0x57,["voice.state:0","disable-group:0"]),
 (0,0x56,["sfx.state:1","main.state:1","label:1081","main.name:label","enable-group:1"]),
 (1,0x56,["disable-group:1","sfx.state:0"]),
])
def test_voice_sfx_side_effect_order(initial,button,expected,tmp_path):
    member="volumeVoiceToggle_" if button==0x57 else "volumeSFXToggle_"
    values=",".join('"'+x+'"' for x in expected)
    main=f'int main(){{PrefWin w;w.{member}.selected={initial};w.change_volume_button_state({button});assert((events==std::vector<std::string>{{{values}}}));}}'
    result=compile_run(tmp_path,STATE+state_functions(actual_source())+main,f"group-{button:x}-{initial}")
    assert result.returncode==0,result.stderr


def test_music_off_virtual_deletion_order(tmp_path):
    main=r'''int main(){PrefWin w;w.volumeMusicToggle_.selected=1;FactionMusic=new MusicProbe;w.change_volume_button_state(0x58);
 assert(FactionMusic==nullptr&&!w.volumeMusicToggle_.selected);
 assert((events==std::vector<std::string>{"derived-dtor","base-dtor","music.state:0"}));}'''
    result=compile_run(tmp_path,STATE+state_functions(actual_source())+main,"music-off")
    assert result.returncode==0,result.stderr


def test_music_on_order_and_pending_bit(tmp_path):
    main=r'''int main(){PrefWin w;w.volumeMusicToggle_.selected=0;w.volumeMainToggle_.selected=0;w.change_volume_button_state(0x58);
 assert(w.volumeMusicToggle_.selected&&w.volumeMainToggle_.selected);
 assert((events==std::vector<std::string>{"music.state:1","main.state:1","label:1081","main.name:label","load:3","begin"}));
 assert(w.prefsAVPending_&PREF_AV_VOLUME_MUSIC_TOGGLE);delete FactionMusic;FactionMusic=nullptr;}'''
    result=compile_run(tmp_path,STATE+state_functions(actual_source())+main,"music-on")
    assert result.returncode==0,result.stderr


def test_main_off_order_and_pending_masks(tmp_path):
    main=r'''int main(){PrefWin w;w.volumeMainToggle_.selected=1;w.change_volume_button_state(0x55);
 assert(!w.volumeMainToggle_.selected&&!w.volumeMusicToggle_.selected&&!w.volumeVoiceToggle_.selected&&!w.volumeSFXToggle_.selected);
 assert((events==std::vector<std::string>{"main.state:0","music.state:0","voice.state:0","sfx.state:0","disable","label:1086","music.name:label","label:1082","voice.name:label","label:1084","sfx.name:label"}));
 assert((w.prefsAVPending_&0xC00u)==0&&(w.morePrefsAVPending_&0x4000u)==0);}'''
    result=compile_run(tmp_path,STATE+state_functions(actual_source())+main,"main-off")
    assert result.returncode==0,result.stderr


def test_click_dispatch_runs_state_then_text_then_redraw(tmp_path):
    main=r'''int main(){PrefWin w;w.volumeMusicToggle_.selected=0;w.volumeMainToggle_.selected=1;w.on_button_clicked(0x58);
 assert((events==std::vector<std::string>{"music.state:1","load:3","begin","label:1087","music.name:label","redraw"}));
 delete FactionMusic;FactionMusic=nullptr;}'''
    result=compile_run(tmp_path,STATE+SUPPORT+state_functions(actual_source(),True)+main,"click-order")
    assert result.returncode==0,result.stderr


RECT=r'''
#include <cassert>
#include <cstdint>
struct RECT{int32_t left,top,right,bottom;};
int32_t PrefWinGeometryValue_8577D8=256,PrefWinGeometryValue_8577DC=287,PrefWinGeometryValue_8577E4=457,PrefWinGeometryValue_8577E0=92,PrefWinNarrowDisplayVerticalOffset=50;
int InterfaceBoxStyles[10],draws=0,style=-1;RECT last{};
struct Buffer{static void box_sprite(RECT*r,int*s){++draws;last=*r;style=int(s-InterfaceBoxStyles);}};
struct PrefWin{int field_A14_=0;void redraw_volume();};
'''


def test_rect_geometry_mode_and_style(tmp_path):
    body=braced(actual_source(),"void PrefWin::redraw_volume()")
    main=r'''int main(){PrefWin w;w.field_A14_=3;w.redraw_volume();assert(draws==0);w.field_A14_=4;w.redraw_volume();
 assert(draws==1&&style==9&&last.left==256&&last.top==237&&last.right==713&&last.bottom==329);
 w.field_A14_=5;w.redraw_volume();assert(draws==1);}'''
    result=compile_run(tmp_path,RECT+body+main,"rect")
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("kind",["label_offset","invalid_id","null_guard","delete_order","rect_style"])
def test_behavioral_mutants_are_detected(kind,tmp_path):
    text=actual_source()
    if kind in ("label_offset","invalid_id","null_guard"):
        changed=text
        if kind=="label_offset":
            old="offset = volumeSFXToggle_.selected_state() ? 0x10F4 : 0x10F0;"
            changed=text.replace(old,"offset = volumeSFXToggle_.selected_state() ? 0x10EC : 0x10E8;",1)
            assert changed!=text
            main=r'int main(){PrefWin w;w.change_volume_button_text(0x56);assert(events[0]=="label:1084");}'
        elif kind=="invalid_id":
            changed=text.replace("    default: return;","    default: button = &volumeMainToggle_; offset = 0x10E0; break;",1)
            assert changed!=text
            main=r'int main(){PrefWin w;w.change_volume_button_text(INT_MIN);assert(events.empty());}'
        else:
            body=braced(text,"void PrefWin::change_volume_button_text(int button_id)")
            wrong=body.replace("button->set_name(label_get(offset / 4));","if (label_get(offset / 4)) button->set_name(label_get(offset / 4));",1)
            assert wrong!=body
            changed=text.replace(body,wrong,1)
            main=r'int main(){PrefWin w;w.volumeMainToggle_.selected=1;null_index=1081;w.change_volume_button_text(0x55);assert(events.back()=="main.name:<null>");}'
        harness=TEXT+label_functions(changed)+main
    elif kind=="delete_order":
        body=braced(text,"void PrefWin::change_volume_button_state(int button_id)")
        wrong=body.replace("delete FactionMusic;\n          FactionMusic = nullptr;","FactionMusic = nullptr;\n          delete FactionMusic;",1)
        assert wrong!=body
        changed=text.replace(body,wrong,1)
        harness=STATE+state_functions(changed)+r'''int main(){PrefWin w;w.volumeMusicToggle_.selected=1;FactionMusic=new MusicProbe;w.change_volume_button_state(0x58);
 assert((events==std::vector<std::string>{"derived-dtor","base-dtor","music.state:0"}));}'''
    else:
        body=braced(text,"void PrefWin::redraw_volume()")
        wrong=body.replace("&InterfaceBoxStyles[9]","&InterfaceBoxStyles[8]",1)
        assert wrong!=body
        changed=text.replace(body,wrong,1)
        harness=RECT+braced(changed,"void PrefWin::redraw_volume()")+'int main(){PrefWin w;w.field_A14_=4;w.redraw_volume();assert(style==9);}'
    result=compile_run(tmp_path,harness,f"mutant-{kind}")
    assert result.returncode!=0
