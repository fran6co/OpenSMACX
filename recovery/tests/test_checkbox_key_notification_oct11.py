"""Actual key-body callback controls; not VC6 layout or full-game proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_caviar_draw_callbacks_actual_source import extract
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "04ed99ab1a48ca7f91a87c828bb1da1f140e0941"


def test_complete_source_change_and_historical_obligations():
    path = ROOT / "src/jackal/checkbox.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/jackal/checkbox.cpp"], cwd=ROOT, text=True)
    assert before.count("    field_8_ = 0;") == 3
    expected = before.replace("    field_8_ = 0;", "    key_notification_callback_ = 0;")
    old = "    WinNotificationCallback callback =\n        reinterpret_cast<WinNotificationCallback>(field_8_);"
    assert expected.count(old) == 1
    expected = expected.replace(old, "    WinNotificationCallback callback = key_notification_callback_;")
    current = path.read_text()
    assert current == expected
    prior, after = read_text(before, path), read_text(current, path)
    assert [(r.address, _continuity_fields(r), r.symbol, r.function_fragment) for r in prior] == [
        (r.address, _continuity_fields(r), r.symbol, r.function_fragment) for r in after]
    header = (ROOT / "src/jackal/checkbox.h").read_text()
    previous_header = subprocess.check_output(["git", "show", BASE + ":src/jackal/checkbox.h"], cwd=ROOT, text=True)
    old_declaration = "  uint32_t field_8_;"
    assert previous_header.count(old_declaration) == 1
    new_declaration = (
        "  // Key handling loads and invokes this cell at original 0x0060F9BD/0x0060F9D1.\n"
        "  // Registration/producer remains unknown; the selected-position word is passed.\n"
        "  // RECOVERY_MEMBER: v1 CheckBox key_notification_callback_ 0x8 0x0060F8B0\n"
        "  WinNotificationCallback key_notification_callback_;  // 0x0008")
    assert header == previous_header.replace(old_declaration, new_declaration, 1)
    assert "WinNotificationCallback key_notification_callback_;" in header
    assert "RECOVERY_MEMBER: v1 CheckBox key_notification_callback_ 0x8 0x0060F8B0" in header


def test_pinned_null_gate_argument_and_caller_cleanup():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x60F8B0, 0x60FA18)), 0x60F8B0)}
    for address, instruction in {
        0x60F9BD: ("mov", "eax, dword ptr [esi - 0x14]"),
        0x60F9C0: ("test", "eax, eax"),
        0x60F9C2: ("je", "0x60f9d6"),
        0x60F9C9: ("mov", "ecx, dword ptr [edx + edi + 0xec]"),
        0x60F9D0: ("push", "ecx"),
        0x60F9D1: ("call", "eax"),
        0x60F9D3: ("add", "esp, 4"),
    }.items():
        assert instructions[address] == instruction


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("none", "wrong_argument", "lost_callback"))
def test_actual_key_body_notification_order_and_post_callback_reads(compiler, damage, tmp_path):
    binary = shutil.which(compiler)
    if binary is None:
        pytest.skip("host compiler unavailable")
    body = extract((ROOT / "src/jackal/checkbox.cpp").read_text(), "int CheckBox::on_key_down(")
    if damage == "wrong_argument":
        old = "callback(static_cast<int>(selected_position_));"
        assert body.count(old) == 1
        body = body.replace(old, "callback(static_cast<int>(field_4_));", 1)
    elif damage == "lost_callback":
        old = "WinNotificationCallback callback = key_notification_callback_;"
        assert body.count(old) == 1
        body = body.replace(old, "WinNotificationCallback callback = 0;", 1)
    header = (ROOT / "src/jackal/checkbox.h").read_text()
    declaration = re.search(r"WinNotificationCallback key_notification_callback_;", header).group()
    callback_type = re.search(r"typedef void \(__cdecl \*WinNotificationCallback\)\(int selected_id\);", (ROOT / "src/win.h").read_text()).group()
    unit = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
CALLBACK_TYPE
static std::vector<int> events;
static int WinCallbackWindow;
struct Dialog { struct { int entry_count_; } item_list_; uint32_t field_58_; };
struct CheckBox : Dialog {
    uint32_t field_4_, selected_position_; int win_parent_;
    DECLARATION
    int on_key_down(int);
    void draw_item(int index,int state,int phase) {
        events.push_back(phase);
        if (phase==1) { assert(index==1); assert(state==0); }
    }
    void soft_update() { events.push_back(3); }
};
static CheckBox *active;
static void notify(int value) {
    assert(value==0); assert(WinCallbackWindow==77);
    assert(events.size()==1 && events[0]==0);
    events.push_back(2); active->field_4_=1;
    active->key_notification_callback_=0;
}
BODY
int main() {
    CheckBox box={}; active=&box; box.win_parent_=77;
    box.item_list_.entry_count_=4; box.field_58_=2;
    box.field_4_=2; box.selected_position_=4; box.key_notification_callback_=notify;
    assert(box.on_key_down(0x20)==1);
    assert((events==std::vector<int>{0,2,1,3}));
    assert(box.selected_position_==0 && box.key_notification_callback_==0);
    events.clear(); assert(box.on_key_down(0xFF)==0);
    assert((events==std::vector<int>{0}));
    events.clear(); assert(box.on_key_down(0x20)==1);
    assert((events==std::vector<int>{0,1,3}));
}
'''.replace("CALLBACK_TYPE", callback_type).replace("DECLARATION", declaration).replace("BODY", body)
    # Null-callback second invocation toggles bit one; relax only that draw's
    # state expectation, while retaining the first callback-modified draw check.
    unit = unit.replace("assert(state==0);", "assert(state==(events.size()==3?0:2));")
    source, executable = tmp_path / "key.cpp", tmp_path / "key"
    source.write_text(unit)
    subprocess.run([binary, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)], check=True, capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage == "none"), result.stderr
