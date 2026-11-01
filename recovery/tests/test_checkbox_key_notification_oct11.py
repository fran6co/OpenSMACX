"""Actual key-body callback controls; not VC6 layout or full-game proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.layout_source import parse_members
from match_recovery.source import read_text
from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
# This committed owner already retains the formerly separate shared cleanup
# identity at 0x0060FD90. It predates the current function-matching edits.
BASE = "7833cadff0e259ded81e36bca7e2692b26e22894"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _checkbox_records(source, path):
    rows = read_text(source, path)
    records = {row.address: row for row in rows}
    assert len(records) == len(rows), "duplicate original identities are forbidden"
    return records


def _assert_checkbox_identity_continuity(before, after, path):
    prior = _checkbox_records(before, path)
    current = _checkbox_records(after, path)
    assert len(prior) == 40
    assert prior.keys() == current.keys(), "all 40 historical identities must remain"
    for address, old in prior.items():
        new = current[address]
        assert (new.name, new.image_spans, new.mode) == (
            old.name, old.image_spans, old.mode
        ), f"identity/name/span/mode changed at 0x{address:08X}"
        assert not old.byte_exact or new.byte_exact, (
            f"historical BYTE_EXACT obligation demoted at 0x{address:08X}"
        )
        assert not old.semantic or new.semantic, (
            f"historical semantic claim demoted at 0x{address:08X}"
        )
        old_lessons = {(lesson.kind, lesson.key) for lesson in old.origin or ()}
        new_lessons = {(lesson.kind, lesson.key) for lesson in new.origin or ()}
        assert old_lessons <= new_lessons, (
            f"historical lesson categories/keys were lost at 0x{address:08X}"
        )


def _assert_key_member_is_typed_layout_member(header):
    rows = parse_members(header, "src/jackal/checkbox.h", PIN, address_space="pe:va")
    matching = [row for row in rows
                if row.class_name == "CheckBox"
                and row.member == "key_notification_callback_"]
    assert len(matching) == 1
    member = matching[0]
    assert (member.offset, member.evidence.address) == (0x8, 0x0060F8B0)
    assert member.class_type is member.class_pointer_type is member.class_array_type is None
    assert "RECOVERY_MEMBER: v1 CheckBox key_notification_callback_ 0x8 0x0060F8B0" in header
    assert "WinNotificationCallback key_notification_callback_;" in header
    return member


def test_focused_checkbox_member_lifecycle_and_historical_obligations():
    path = ROOT / "src/jackal/checkbox.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/jackal/checkbox.cpp"], cwd=ROOT, text=True)
    current = path.read_text()
    _assert_checkbox_identity_continuity(before, current, path)

    # The callback cell is an ordinary typed member, and every lifecycle reset
    # that previously cleared field_8_ still clears this same cell.
    header = (ROOT / "src/jackal/checkbox.h").read_text()
    _assert_key_member_is_typed_layout_member(header)
    assert "typedef void (__cdecl *WinNotificationCallback)(int selected_id);" in (ROOT / "src/win.h").read_text()
    constructor = extract(current, "CheckBox::CheckBox()")
    destructor = extract(current, "CheckBox::~CheckBox()")
    close = extract(current, "void CheckBox::close()")
    # Destruction delegates the identical reset/base-close sequence to close.
    assert destructor.count("close();") == 1
    assert "Dialog::close();" not in destructor
    assert "GraphicWin::close();" not in destructor
    for body in (constructor, close):
        assert body.count("key_notification_callback_ = 0;") == 1
        assert body.count("field_8_ = 0;") == 0
    for body in (close,):
        assert body.index("key_notification_callback_ = 0;") < body.index("Dialog::close();")
        assert body.index("Dialog::close();") < body.index("GraphicWin::close();")

    key_body = extract(current, "int CheckBox::on_key_down(")
    assert key_body.count("WinNotificationCallback callback = key_notification_callback_;") == 1
    assert key_body.index("if (callback != 0)") < key_body.index(
        "callback(static_cast<int>(box->selected_position_));")
    assert key_body.index("callback(static_cast<int>(box->selected_position_));") < key_body.index(
        "draw_item(static_cast<int>(field_4_),", key_body.index("callback(static_cast<int>"))
    assert key_body.index("soft_update();") > key_body.index(
        "callback(static_cast<int>(box->selected_position_));")


def test_identity_guard_rejects_missing_span_change_and_claim_demotion():
    path = ROOT / "src/jackal/checkbox.cpp"
    baseline = subprocess.check_output(["git", "show", BASE + ":src/jackal/checkbox.cpp"], cwd=ROOT, text=True)
    current = path.read_text()
    _assert_checkbox_identity_continuity(baseline, current, path)

    identity = "0x0060F8B0 ?on_key_down@CheckBox@@QAEHH@Z 0x0060F8B0-0x0060FA18"
    assert current.count(identity) == 1
    with pytest.raises(AssertionError, match="40 historical identities"):
        _assert_checkbox_identity_continuity(baseline, current.replace(identity, "", 1), path)
    changed_span = current.replace(identity, identity.replace("0x0060FA18", "0x0060FA17"), 1)
    with pytest.raises(AssertionError, match="identity/name/span/mode changed"):
        _assert_checkbox_identity_continuity(baseline, changed_span, path)
    ctor_marker = "0x0060E670-0x0060E735;0x00662F60-0x00662F84 BYTE_EXACT"
    assert current.count(ctor_marker) == 1
    demoted = current.replace(ctor_marker, ctor_marker[:-len(" BYTE_EXACT")], 1)
    with pytest.raises(AssertionError, match="demoted"):
        _assert_checkbox_identity_continuity(baseline, demoted, path)


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
    actual = (ROOT / "src/jackal/checkbox.cpp").read_text()
    body = extract(actual, "int CheckBox::on_key_down(")
    toggle = extract(actual, "void CheckBox::UNK1(int pos)")
    if damage == "wrong_argument":
        old = "callback(static_cast<int>(box->selected_position_));"
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
struct Dialog { struct { int entry_count_; } item_list_; int row_count_; };
struct CheckBox : Dialog {
    int field_4_, selected_position_, win_parent_;
    DECLARATION
    int on_key_down(int);
    void UNK1(int);
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
TOGGLE
BODY
int main() {
    CheckBox box={}; active=&box; box.win_parent_=77;
    box.item_list_.entry_count_=4; box.row_count_=2;
    box.field_4_=2; box.selected_position_=4; box.key_notification_callback_=notify;
    assert(box.on_key_down(0x20)==1);
    assert((events==std::vector<int>{0,2,1,3}));
    assert(box.selected_position_==0 && box.key_notification_callback_==0);
    events.clear(); assert(box.on_key_down(0xFF)==0);
    assert((events==std::vector<int>{0}));
    events.clear(); assert(box.on_key_down(0x20)==1);
    assert((events==std::vector<int>{0,1,3}));
}
'''.replace("CALLBACK_TYPE", callback_type).replace("DECLARATION", declaration).replace("TOGGLE", toggle).replace("BODY", body)
    # Null-callback second invocation toggles bit one; relax only that draw's
    # state expectation, while retaining the first callback-modified draw check.
    unit = unit.replace("assert(state==0);", "assert(state==(events.size()==3?0:2));")
    source, executable = tmp_path / "key.cpp", tmp_path / "key"
    source.write_text(unit)
    subprocess.run([binary, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)], check=True, capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage == "none"), result.stderr
