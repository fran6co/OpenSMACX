"""Cursor bounds controls using the actual session-picker source bodies.

The host fixture preserves list cursor state and picker call outcomes. It does
not establish native ABI, compiler output, or complete game behavior.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from test_session_picker_actual_source import function as extract_definition


ROOT = Path(__file__).resolve().parents[2]
PICKER_CPP = ROOT / "src/net_session_picker.cpp"
NET_CLASS_CPP = ROOT / "src/net_class.cpp"
ALPHANET_CPP = ROOT / "src/alphanet_records.cpp"


def actual_sources():
    picker = PICKER_CPP.read_text(encoding="utf-8")
    net_class = NET_CLASS_CPP.read_text(encoding="utf-8")
    alphanet = ALPHANET_CPP.read_text(encoding="utf-8")
    helper_signature = "template<class List> inline int seek_owned_list(List *list, int position)"

    join = extract_definition(picker, "int NetSessionPickerWin::join_selected_session()")
    button = extract_definition(picker, "void NetSessionPickerWin::on_button_clicked(int action)")
    shared_seek = extract_definition(net_class, helper_signature)
    alpha_seek = extract_definition(alphanet, helper_signature)
    assert " ".join(shared_seek.split()) == " ".join(alpha_seek.split()), (
        "AlphaNet's duplicated seek helper has diverged from the shared list helper"
    )
    alpha_wrapper = extract_definition(
        alphanet, "int AlphaNetStringStruct::seek_pos(int position)"
    )
    return join, button, shared_seek, alpha_wrapper


HARNESS = r'''#include <cassert>
#include <cstdlib>

typedef int (*AbsFnT)(int);

struct NetAllocationNode { int tag_; };
struct AlphaNetLedgerEntry {
  NetAllocationNode *payload_;
  AlphaNetLedgerEntry *next_;
  AlphaNetLedgerEntry *previous_;
};
struct AlphaNetStringStruct {
  typedef AlphaNetLedgerEntry Entry;
  typedef NetAllocationNode Payload;
  Entry *head_;
  Entry *current_;
  int entry_count_;
  int current_position_;
  int seek_pos(int position);
};
struct AlphaNetLedger : AlphaNetStringStruct {};

ACTUAL_SEEK_HELPER
ACTUAL_SEEK_WRAPPER

struct Dialog { int selected_position_; };
struct ListBox : Dialog {};
struct NetSessionPickerTail { int action_result_; };
struct Net {
  NetAllocationNode *last_session_;
  int join_result_;
  int join_calls_;
  Net() : last_session_(0), join_result_(0), join_calls_(0) {}
  int join_session(NetAllocationNode *session, char *, char *) {
    last_session_ = session;
    ++join_calls_;
    return join_result_;
  }
};
struct ParseBuffer { char str[64]; };
ParseBuffer ParseStrBuffer[1] = {{{0}}};
int popup_result = 0;
int list_selection = 0;
int pop_caption_value_title(char *, char *, int, int, int) {
  return popup_result;
}
int x_pop_caption_value_flags(const char *, const char *, int, int, int) {
  return list_selection;
}

struct NetSessionPickerWin {
  ListBox sessions_view_;
  AlphaNetLedger sessions_;
  Net *net_;
  NetSessionPickerTail tail_;
  int release_calls_;
  int connect_result_;
  int join_selected_session();
  void on_button_clicked(int action);
  void release_modal() { ++release_calls_; }
  int connect_session() { return connect_result_; }
  NetSessionPickerWin() : net_(0), release_calls_(0), connect_result_(0) {
    tail_.action_result_ = -9;
  }
};

ACTUAL_JOIN
ACTUAL_BUTTON

static NetAllocationNode payloads[3];
static AlphaNetLedgerEntry entries[3];

static void reset_case(NetSessionPickerWin *picker, Net *net, int row) {
  for (int index = 0; index != 3; ++index) {
    payloads[index].tag_ = index;
    entries[index].payload_ = &payloads[index];
    entries[index].next_ = &entries[(index + 1) % 3];
    entries[index].previous_ = &entries[(index + 2) % 3];
  }
  picker->net_ = net;
  picker->sessions_view_.selected_position_ = row;
  picker->sessions_.head_ = &entries[0];
  picker->sessions_.current_ = &entries[1];
  picker->sessions_.entry_count_ = 3;
  picker->sessions_.current_position_ = 71;
  picker->tail_.action_result_ = -9;
  picker->release_calls_ = 0;
  picker->connect_result_ = 0;
  net->last_session_ = 0;
  net->join_result_ = 0;
  net->join_calls_ = 0;
  popup_result = 0;
  list_selection = 0;
}

static void run_case(bool button_path, int row, bool cancel,
                     int expected_cursor, int expected_position,
                     int expected_payload) {
  NetSessionPickerWin picker;
  Net net;
  reset_case(&picker, &net, row);
  if (cancel) {
    if (button_path) list_selection = -1;
    else popup_result = -1;
  }

  if (button_path) {
    picker.on_button_clicked(-1);
  } else {
    const int result = picker.join_selected_session();
    assert(result == 1);
  }

  if (cancel) {
    assert(net.join_calls_ == 0);
    assert(picker.sessions_.current_ == &entries[1]);
    assert(picker.sessions_.current_position_ == 71);
  } else {
    assert(net.join_calls_ == 1);
    assert(net.last_session_ == &payloads[expected_payload]);
    assert(picker.sessions_.current_ == &entries[expected_cursor]);
    assert(picker.sessions_.current_position_ == expected_position);
    if (button_path) assert(picker.tail_.action_result_ == 0);
  }
  assert(picker.release_calls_ == 0);
}

static void run_controls() {
  for (int path = 0; path != 2; ++path) {
    const bool button = path != 0;
    // Above-count selection retains the preexisting cursor and position.
    run_case(button, 3, false, 1, 71, 1);
    // Too-negative selection seeds head_, but leaves current_position_ alone.
    run_case(button, -4, false, 0, 71, 0);
    // Valid forward and backward positions select their ring entries.
    run_case(button, 2, false, 2, 2, 2);
    run_case(button, -1, false, 2, 2, 2);
    // Both modal-cancel routes leave both cursor fields untouched.
    run_case(button, 2, true, 1, 71, 1);
  }
}

int main() { run_controls(); }
'''


def fixture(join: str, button: str, helper: str, wrapper: str) -> str:
    return (HARNESS.replace("ACTUAL_SEEK_HELPER", helper)
            .replace("ACTUAL_SEEK_WRAPPER", wrapper)
            .replace("ACTUAL_JOIN", join)
            .replace("ACTUAL_BUTTON", button))


def compile_and_run(tmp_path: Path, source: str, compiler: str, label: str) -> int:
    cpp = tmp_path / (label + ".cpp")
    executable = tmp_path / label
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        check=False, capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], check=False, capture_output=True,
                          timeout=20).returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damaged_path", ("join", "button"))
def test_actual_picker_cursor_bounds_and_preseed_damage(tmp_path, compiler_name,
                                                       damaged_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    join, button, helper, wrapper = actual_sources()
    assert compile_and_run(tmp_path, fixture(join, button, helper, wrapper),
                           compiler, "actual") == 0

    anchor = "Dialog &dialog = static_cast<Dialog &>(sessions_view_);"
    damaged_join = join.replace(
        anchor, anchor + "\n    sessions_.current_ = sessions_.head_;", 1)
    damaged_button = button.replace(
        anchor, anchor + "\n  sessions_.current_ = sessions_.head_;", 1)
    assert damaged_join != join and damaged_button != button
    assert compile_and_run(
        tmp_path,
        fixture(damaged_join if damaged_path == "join" else join,
                damaged_button if damaged_path == "button" else button,
                helper, wrapper),
        compiler,
        "unconditional-preseed-mutant",
    ) != 0


def find_pinned_image() -> Path | None:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        # An explicit missing/wrong input is an error, not permission to fall
        # back to another image and report the configured witness as passing.
        return Path(configured)
    for parent in (ROOT, *ROOT.parents):
        for path in (parent / ".opensmacx/game/terranx_original.exe",
                     parent / "game/terranx_original.exe"):
            if path.is_file():
                return path
    return None


def test_pinned_cursor_bound_branches_and_store_order():
    image_path = find_pinned_image()
    if image_path is None:
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    capstone = pytest.importorskip("capstone")
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    pin = next(line.split()[1] for line in
               (ROOT / "src/IMAGE_PIN").read_text(encoding="utf-8").splitlines()
               if line.startswith("sha256 "))
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == pin
    image = PeImage(image_bytes, pin)
    disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    spans = (
        (Span(0x0063C6D0, 0x0063C7B5), {
            0x0063C70F: ("cmp", "edi, edx"),
            0x0063C711: ("jg", "0x63c779"),
            0x0063C71B: ("mov", "dword ptr [esi + 0x36b4], eax"),
        }),
        (Span(0x0063C3A0, 0x0063C4D6), {
            0x0063C411: ("cmp", "edi, eax"),
            0x0063C413: ("jg", "0x63c47b"),
            0x0063C41D: ("mov", "dword ptr [esi + 0x36b4], ecx"),
        }),
    )
    for span, expected in spans:
        instructions = list(disassembler.disasm(image.read(span), span.start))
        assert instructions
        assert instructions[-1].address + instructions[-1].size == span.end
        anchors = {instruction.address: (instruction.mnemonic, instruction.op_str)
                   for instruction in instructions}
        for address, instruction in expected.items():
            assert anchors[address] == instruction

