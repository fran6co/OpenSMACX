"""Actual-source message-pump controls; not native ABI or game acceptance."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_bodies():
    source = (ROOT / "src/win.cpp").read_text()
    bodies = []
    for name, kind in [("do_char", "int"), ("do_mouse", "int"),
                       ("do_all_mouse", "void"), ("flush_keyboard", "void"),
                       ("flush_mouse", "void")]:
        start = source.index(f"{kind} __cdecl {name}() {{")
        opening = source.index("{", start)
        depth = 0
        for end in range(opening, len(source)):
            depth += (source[end] == "{") - (source[end] == "}")
            if depth == 0:
                bodies.append(source[start:end + 1])
                break
        else:
            raise AssertionError(f"unterminated {name}")
    return "\n".join(bodies)


HARNESS = r'''
#include <cassert>
#include <vector>
#define __cdecl
constexpr int WM_CHAR=0x102, WM_MOUSEMOVE=0x200, WM_MBUTTONDBLCLK=0x209;
constexpr int WM_KEYFIRST=0x100, WM_KEYLAST=0x108, PM_REMOVE=1;
struct MSG { unsigned message; };
struct Win {};
Win owner1, owner2;
Win *WinPointerOwner1, *WinPointerOwner2;
int MsgStatus, mode, video_calls, check_calls, net_calls, dispatch_calls, translates;
std::vector<unsigned> queue;
std::vector<char> events;
void do_video() { ++video_calls; events.push_back('v'); if(mode==2) assert(MsgStatus==8); }
void check_net() {
    ++check_calls; events.push_back('c');
    if(mode==2) assert(MsgStatus==8);
    if(mode==3) assert(queue.size()==2 && !WinPointerOwner1 && !WinPointerOwner2);
}
void do_net() { ++net_calls; events.push_back('n'); assert(MsgStatus==0); }
int PeekMessageA(MSG *msg, int hwnd, unsigned first, unsigned last, int remove) {
    assert(hwnd==0 && remove==1); events.push_back('p');
    if(mode==2) assert(MsgStatus==8);
    if(mode==3) assert(WinPointerOwner1==&owner1 && WinPointerOwner2==&owner2);
    for(auto it=queue.begin(); it!=queue.end(); ++it) {
        if(*it>=first && *it<=last) { msg->message=*it; queue.erase(it); return 1; }
    }
    return 0;
}
int TranslateMessage(MSG*) { ++translates; events.push_back('t'); return 1; }
int DispatchMessageA(MSG*) {
    ++dispatch_calls; events.push_back('d'); if(mode==2) MsgStatus=99; return 17;
}
void reset(int next_mode, std::vector<unsigned> messages) {
    mode=next_mode; queue=messages; events.clear();
    video_calls=check_calls=net_calls=dispatch_calls=translates=0;
    WinPointerOwner1=&owner1; WinPointerOwner2=&owner2; MsgStatus=41;
}
'''

CASES = r'''
int main() {
    reset(0,{0x101,0x102,0x102,0x200});
    assert(do_char()==1 && queue==std::vector<unsigned>({0x101,0x102,0x200}));
    assert(events==std::vector<char>({'v','c','p','t','d'}));
    assert(do_char()==1); assert(do_char()==0);
    assert(dispatch_calls==2 && translates==2 && net_calls==0);
    reset(0,{0x102,0x209,0x20a});
    assert(do_mouse()==1 && queue==std::vector<unsigned>({0x102,0x20a}));
    assert(do_mouse()==0 && dispatch_calls==1 && video_calls==2 && check_calls==2);
    reset(2,{0x102,0x200,0x209,0x20a});
    do_all_mouse();
    assert(queue==std::vector<unsigned>({0x102,0x20a}));
    assert(events==std::vector<char>({'v','c','p','t','d','v','c','p','t','d','v','c','p','n'}));
    assert(video_calls==3 && check_calls==3 && dispatch_calls==2 && net_calls==1);
    reset(2,{}); do_all_mouse();
    assert(events==std::vector<char>({'v','c','p','n'}));
    reset(3,{0xff,0x100,0x102,0x108,0x109}); flush_keyboard();
    assert(queue==std::vector<unsigned>({0xff,0x109}));
    assert(check_calls==1 && !video_calls && !net_calls && !dispatch_calls && !translates);
    assert(events==std::vector<char>({'p','p','p','p','c'}));
    reset(3,{0x1ff,0x200,0x209,0x20a}); flush_mouse();
    assert(queue==std::vector<unsigned>({0x1ff,0x20a}));
    assert(check_calls==1 && !video_calls && !net_calls && !dispatch_calls && !translates);
    assert(events==std::vector<char>({'p','p','p','c'}));
    reset(3,{0xff,0x109}); flush_keyboard();
    assert(events==std::vector<char>({'p','c'}));
    reset(3,{0x1ff,0x20a}); flush_mouse();
    assert(events==std::vector<char>({'p','c'}));
}
'''


def run_host(tmp_path, bodies):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "pumps.cpp"
    source.write_text(HARNESS + bodies + CASES)
    executable = tmp_path / "pumps"
    compiled = subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_actual_input_pumps(tmp_path):
    result = run_host(tmp_path, actual_bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("WM_CHAR, WM_CHAR", "WM_KEYFIRST, WM_KEYLAST"),
    ("WM_MBUTTONDBLCLK, PM_REMOVE", "WM_MBUTTONDBLCLK + 1, PM_REMOVE"),
    ("WM_CHAR, WM_CHAR, PM_REMOVE", "WM_CHAR, WM_CHAR, 0"),
    ("WM_KEYFIRST, 0x108", "WM_KEYFIRST, 0x107"),
    ("            MsgStatus = 8;", "            MsgStatus = 0;"),
    ("WinPointerOwner2 = nullptr;", "WinPointerOwner2 = &owner2;"),
])
def test_input_pump_damage_controls(tmp_path, before, after):
    bodies = actual_bodies()
    assert before in bodies
    result = run_host(tmp_path, bodies.replace(before, after, 1))
    assert result.returncode != 0, "control failed to detect damaged source"
