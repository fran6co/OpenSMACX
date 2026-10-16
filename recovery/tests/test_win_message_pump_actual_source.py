"""Host controls for the actual Win message-pump owner.

Bodies are extracted from current src/temp.* or, after an owner move, from
src/win.*. This is behavior regression coverage only, not native ABI proof.
"""

from pathlib import Path
import os
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
HOST_COMPILERS = [path for executable in ("g++", "clang++")
                  if (path := shutil.which(executable)) is not None]


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def _one_body(name, signature, paths):
    found = []
    for path in paths:
        text = path.read_text()
        if signature in text:
            found.append(_body(text, signature))
    assert len(found) == 1, f"expected one current owner of {name}, found {len(found)}"
    return found[0]


def _actual_pump_bodies():
    cpp_paths = [ROOT / "src/win.cpp"]
    header_paths = [ROOT / "src/win.h"]
    return "\n".join([
        _one_body("do_draw", "MEASURED inline BOOL __cdecl do_draw()", header_paths),
        _one_body("do_keyboard", "MEASURED inline BOOL __cdecl do_keyboard()", header_paths),
        _one_body("do_all_non_input", "void __cdecl do_all_non_input()", cpp_paths),
        _one_body("do_non_input", "BOOL __cdecl do_non_input()", cpp_paths),
        _one_body("do_all_draws", "void __cdecl do_all_draws()", cpp_paths),
        _one_body("do_all_keyboard", "void __cdecl do_all_keyboard()", cpp_paths),
        _one_body("do_all_tasks(Win*)", "void __cdecl do_all_tasks(Win *window)", cpp_paths),
    ])


def test_non_input_pump_is_an_ordinary_win_cpp_definition():
    cpp = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    signature = "void __cdecl do_all_non_input()"
    assert cpp.count(signature) == 1
    assert header.count(signature) == 1
    assert "MEASURED inline void __cdecl do_all_non_input()" not in header
    assert _body(cpp, signature).endswith("}")


def test_pinned_direct_pump_callers_use_the_full_mapped_spans():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for caller-boundary verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    # Addresses are instruction witnesses; the containing extents come from
    # the native source reader's ORIGINAL mappings, not hand-cut decode windows.
    witnesses = (
        ("src/path.cpp", 0x0059C520, (0x0059C773,)),
        ("src/map.cpp", 0x005C3F70, (0x005C4038, 0x005C40A5)),
    )
    for relative_path, owner_address, call_sites in witnesses:
        records = read_file(ROOT / relative_path)
        record, = [row for row in records if row.address == owner_address]
        assert len(record.image_spans) == 1
        low, high = record.image_spans[0]
        assert low == owner_address
        instructions = list(decoder.disasm(image.read(Span(low, high)), low))
        cursor = low
        by_address = {}
        for instruction in instructions:
            assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
            by_address[instruction.address] = instruction
            cursor += instruction.size
        assert cursor == high
        for call_site in call_sites:
            instruction = by_address[call_site]
            assert instruction.mnemonic == "call"
            assert instruction.operands[0].imm == 0x005FCB20


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
#define MEASURED
using BOOL = int;
struct Win {};
struct MSG { void *hwnd; unsigned message; unsigned time; };
constexpr unsigned WM_NULL=0x000, WM_INPUT=0x0ff, WM_KEYDOWN=0x100;
constexpr unsigned WM_PAINT=0x00f, WM_CREATE=0x001, WM_UNICHAR=0x109;
constexpr unsigned WM_KEYLAST=0x108, UNICODE_NOCHAR=0xffff;
constexpr unsigned PM_NOREMOVE=0, PM_REMOVE=1;
Win owner_a, owner_b;
Win *WinPointerOwner1, *WinPointerOwner2;
uint32_t MsgStatus;
struct Entry { void *hwnd; unsigned message; unsigned time; };
std::vector<Entry> queue;
std::vector<char> events;
std::vector<unsigned> net_statuses;
int dispatches, translates;
void do_video() { events.push_back('V'); }
void check_net() { events.push_back('C'); }
void do_net() {
    events.push_back('N');
    net_statuses.push_back(MsgStatus);
}
BOOL __cdecl do_non_input();
BOOL __cdecl do_draw();
BOOL __cdecl do_keyboard();
void __cdecl do_all_draws();
void __cdecl do_all_keyboard();
int PeekMessageA(MSG *out, void *hwnd, unsigned low, unsigned high, unsigned flags) {
    events.push_back('P');
    for (unsigned i=0; i<queue.size(); ++i) {
        Entry const row=queue[i];
        if ((!hwnd || hwnd==row.hwnd) && row.message>=low && row.message<=high) {
            *out=MSG{row.hwnd,row.message,row.time};
            if (flags==PM_REMOVE) queue.erase(queue.begin()+i);
            return 1;
        }
    }
    return 0;
}
int PeekMessage(MSG *out, void *hwnd, unsigned low, unsigned high, unsigned flags) {
    return PeekMessageA(out, hwnd, low, high, flags);
}
int TranslateMessage(MSG const *) { ++translates; events.push_back('T'); return 1; }
long DispatchMessageA(MSG const *) { ++dispatches; events.push_back('D'); return 0; }
long DispatchMessage(MSG const *message) { return DispatchMessageA(message); }
void reset() {
    queue.clear(); events.clear(); dispatches=translates=0;
    net_statuses.clear();
    WinPointerOwner1=&owner_a; WinPointerOwner2=&owner_b;
    MsgStatus=77;
}
void post(unsigned message, unsigned time, void *hwnd=&owner_a) {
    queue.push_back(Entry{hwnd,message,time});
}
'''


CASES = r'''
int main() {
    reset();
    post(0x41,20); post(0x120,10); post(0x20b,5);
    assert(do_non_input()==1);
    assert(queue.size()==2 && queue[0].message==0x120 && queue[1].message==0x20b);
    assert(translates==1 && dispatches==1);
    assert(events==std::vector<char>({'V','C','N','P','P','T','D'}));
    assert(net_statuses==std::vector<unsigned>({77}));

    reset(); post(0x200,4); post(0x120,10); post(0x20b,5);
    assert(do_non_input()==1);
    assert(queue.size()==2 && queue[0].message==0x200 && queue[1].message==0x20b);

    reset(); post(0x200,4); post(0x20b,5);
    assert(do_non_input()==1);
    assert(queue.size()==1 && queue[0].message==0x200);

    reset(); post(0x41,1);
    do_all_non_input();
    assert(queue.empty() && translates==1 && dispatches==1 && MsgStatus==0);
    assert(net_statuses==std::vector<unsigned>({32,32,0}));
    assert(events.back()=='C' && events[events.size()-2]=='N');

    reset(); post(WM_PAINT,2); post(WM_PAINT,1);
    do_all_draws();
    assert(queue.empty() && translates==2 && dispatches==2 && MsgStatus==0);
    assert(net_statuses==std::vector<unsigned>({1,1,1}));
    assert(events.back()=='P');

    reset(); post(WM_KEYDOWN,1); post(WM_KEYLAST,2);
    post(WM_UNICHAR,3);
    do_all_keyboard();
    assert(queue.size()==1 && queue[0].message==WM_UNICHAR);
    assert(translates==2 && dispatches==2 && MsgStatus==0);
    assert(net_statuses==std::vector<unsigned>({0}));
    assert(events.back()=='N');

    reset(); WinPointerOwner1=nullptr; WinPointerOwner2=&owner_b;
    do_all_tasks(&owner_b);
    assert(WinPointerOwner1==nullptr && WinPointerOwner2==nullptr);
    assert(MsgStatus==0 && events==std::vector<char>({'N'}));
    assert(net_statuses==std::vector<unsigned>({0}));
}
'''


@pytest.mark.parametrize("compiler", HOST_COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_message_pump_bodies_and_order(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    source = tmp_path / "message_pump.cpp"
    source.write_text(HARNESS + _actual_pump_bodies() + CASES)
    binary = tmp_path / "message_pump"
    compiled = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                               "-Werror", str(source), "-o", str(binary)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", HOST_COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
@pytest.mark.parametrize(("before", "after"), [
    ("WM_NULL, WM_INPUT, PM_NOREMOVE", "WM_NULL, 0x40, PM_NOREMOVE"),
    ("MsgStatus = 1;", "MsgStatus = 0;"),
    ("WinPointerOwner2 = 0;", "WinPointerOwner2 = window;"),
])
def test_pump_control_rejects_behavior_mutants(tmp_path, compiler, before, after):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    bodies = _actual_pump_bodies()
    assert before in bodies
    source = tmp_path / "mutant.cpp"
    source.write_text(HARNESS + bodies.replace(before, after, 1) + CASES)
    binary = tmp_path / "mutant"
    compiled = subprocess.run([compiler, "-std=c++17", str(source), "-o", str(binary)],
                              capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode != 0, "control failed to detect the changed pump behavior"
