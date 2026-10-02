"""Actual-source SDK message-pump controls, not native game/runtime proof."""
from pathlib import Path
import subprocess
import tempfile


def test_complete_win_input_pump_family():
    source = (Path(__file__).resolve().parents[2] / "src/win.cpp").read_text()
    names = ["pump_key_message", "pump_all_key_messages", "pump_mouse_move_message",
             "pump_mouse_button_message", "pump_sound_message", "pump_all_sound_messages",
             "do_all_sounds"]
    bodies = []
    for name in names:
        start = source.index("__cdecl " + name + "()")
        start = source.rfind("\n", 0, start) + 1
        opening = source.index("{", start)
        depth = 1
        end = opening + 1
        while depth:
            depth += (source[end] == "{") - (source[end] == "}")
            end += 1
        bodies.append(source[start:end])
    prefix = r'''
#include <cassert>
#include <vector>
#include <string>
#define __cdecl
const unsigned WM_KEYDOWN=0x100, WM_SYSDEADCHAR=0x108, WM_MOUSEMOVE=0x200;
const unsigned WM_LBUTTONDOWN=0x201, WM_LBUTTONUP=0x202;
const unsigned WM_RBUTTONDOWN=0x204, WM_RBUTTONUP=0x205, WM_MBUTTONDBLCLK=0x209;
const unsigned WM_USER=0x400, PM_REMOVE=1;
struct MSG { void *hwnd; unsigned message; };
std::vector<MSG> Queue;
std::vector<std::string> Trace;
unsigned MsgStatus=0;
bool Reentrant=false;
void *Window1=reinterpret_cast<void *>(1), *Window2=reinterpret_cast<void *>(2);
void do_video() { Trace.push_back("V"); }
void check_net() { Trace.push_back("C"); }
void do_net() { Trace.push_back("N"); }
int PeekMessageA(MSG *message, void *window, unsigned low, unsigned high, unsigned flags) {
    assert(flags == PM_REMOVE);
    Trace.push_back("P");
    for (unsigned i=0; i<Queue.size(); ++i) {
        if ((!window || Queue[i].hwnd==window) && Queue[i].message>=low && Queue[i].message<=high) {
            *message=Queue[i]; Queue.erase(Queue.begin()+i); return 1;
        }
    }
    return 0;
}
int TranslateMessage(const MSG *) { Trace.push_back("T"); return 0; }
long DispatchMessageA(const MSG *) {
    Trace.push_back("D");
    MsgStatus=77;
    if (Reentrant) { Queue.push_back(MSG{Window1,0x108}); Reentrant=false; }
    return 0;
}
std::string trace() { std::string s; for (unsigned i=0;i<Trace.size();++i) s+=Trace[i]; return s; }
void reset() { Queue.clear(); Trace.clear(); MsgStatus=99; Reentrant=false; }
void post(unsigned message, void *window=Window1) { Queue.push_back(MSG{window,message}); }
'''
    checks = r'''
int main() {
    reset(); assert(pump_key_message()==0); assert(trace()=="VCP" && MsgStatus==99);
    reset(); post(0x109); post(0x108); assert(pump_key_message()==1);
    assert(trace()=="VCPTD" && Queue.size()==1 && Queue[0].message==0x109 && MsgStatus==77);
    reset(); pump_all_key_messages(); assert(trace()=="VCPN" && MsgStatus==99);
    reset(); post(0x100); Reentrant=true; pump_all_key_messages();
    assert(trace()=="VCPTDVCPTDVCPN" && Queue.empty() && MsgStatus==77);
    reset(); post(0x201); assert(pump_mouse_move_message()==0); assert(trace()=="P" && Queue.size()==1);
    reset(); post(0x200); assert(pump_mouse_move_message()==1); assert(trace()=="PTD" && MsgStatus==77);
    reset(); assert(pump_mouse_button_message()==0); assert(trace()=="VCP" && MsgStatus==99);
    reset(); post(0x20A); assert(pump_mouse_button_message()==0); assert(Queue.size()==1);
    reset(); post(0x201); post(0x202,Window2); post(0x202);
    assert(pump_mouse_button_message()==1); assert(trace()=="VCPTDPTD");
    assert(Queue.size()==1 && Queue[0].hwnd==Window2);
    reset(); post(0x201); assert(pump_mouse_button_message()==1); assert(trace()=="VCPTDP");
    reset(); post(0x204); post(0x205); assert(pump_mouse_button_message()==1);
    assert(trace()=="VCPTDPTD" && Queue.empty());
    reset(); post(0x209); assert(pump_mouse_button_message()==1); assert(trace()=="VCPTD");
    reset(); post(0x400); post(0x402); assert(pump_sound_message()==0);
    assert(trace()=="P" && Queue.size()==2 && MsgStatus==99);
    reset(); post(0x401); assert(pump_sound_message()==1); assert(trace()=="PTDC" && MsgStatus==77);
    reset(); pump_all_sound_messages(); assert(trace()=="P" && MsgStatus==99);
    reset(); post(0x401); post(0x402); post(0x401); pump_all_sound_messages();
    assert(trace()=="PTDCPTDCP" && Queue.size()==1 && Queue[0].message==0x402 && MsgStatus==77);
    reset(); do_all_sounds(); assert(trace().empty() && MsgStatus==0);
}
'''
    with tempfile.TemporaryDirectory(prefix="win-input-pump-host-") as directory:
        binary = str(Path(directory) / "control")
        compiled = subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                                   "-x", "c++", "-", "-o", binary],
                                  input=prefix+"\n".join(bodies)+checks, text=True,
                                  capture_output=True, timeout=30)
        assert compiled.returncode == 0, compiled.stderr
        ran = subprocess.run([binary], capture_output=True, text=True, timeout=30)
        assert ran.returncode == 0, ran.stdout + ran.stderr
