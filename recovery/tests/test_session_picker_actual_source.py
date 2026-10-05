"""Bounded C++98 controls of the product SessionPicker bodies, not target ABI or full runtime proof."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/net_session_picker.cpp"


def function(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "normal"
    quote = ""
    escape = False
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if c == "\n":
                state = "normal"
        elif state == "block":
            if c == "*" and n == "/":
                state = "normal"
                i += 1
        elif state == "string":
            if escape:
                escape = False
            elif c == "\\":
                escape = True
            elif c == quote:
                state = "normal"
        else:
            if c == "/" and n == "/":
                state = "line"
                i += 1
            elif c == "/" and n == "*":
                state = "block"
                i += 1
            elif c in ("'", '"'):
                state, quote = "string", c
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        i += 1
    raise AssertionError("unterminated source function: " + signature)


def compile_and_run(tmp_path, code, name):
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("host C++ compiler unavailable")
    for index, compiler in enumerate(compilers):
        cpp = tmp_path / (name + str(index) + ".cpp")
        exe = tmp_path / (name + str(index))
        cpp.write_text(code)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                        "-Wno-unused-parameter", "-Wno-unused-variable",
                        str(cpp), "-o", str(exe)],
                       check=True, capture_output=True, text=True, timeout=60)
        subprocess.run([str(exe)], check=True, capture_output=True, text=True, timeout=30)


COMMON = r'''
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef unsigned long uintptr_t;
using std::string;
using std::vector;
struct RECT { int left, top, right, bottom; };
struct Win { virtual ~Win() {} };
static vector<string> events;
struct GraphicWin : Win {
  GraphicWin() { events.push_back("GraphicWin.ctor"); }
  virtual ~GraphicWin() { events.push_back("GraphicWin.dtor"); }
  void close() { events.push_back("GraphicWin.close"); }
};
struct Time {
  Time() { events.push_back("Time.ctor"); }
  ~Time() { events.push_back("Time.dtor"); }
  void close() { events.push_back("Time.close"); }
};
struct Heap {
  Heap() { events.push_back("Heap.ctor"); }
  ~Heap() { events.push_back("Heap.dtor"); }
};
struct ListBox {
  ListBox() { events.push_back("ListBox.ctor"); }
  ~ListBox() { events.push_back("ListBox.dtor"); }
  void close() { events.push_back("ListBox.close"); }
};
static int button_ctor_index;
struct PushButton {
  int id;
  PushButton() : id(++button_ctor_index) {
    char b[32]; std::sprintf(b, "PushButton%d.ctor", id); events.push_back(b);
  }
  ~PushButton() {
    char b[32]; std::sprintf(b, "PushButton%d.dtor", id); events.push_back(b);
  }
  void close() {
    char b[32]; std::sprintf(b, "PushButton%d.close", id); events.push_back(b);
  }
};
struct NetAllocationNode { char *password_; char *session_name_; };
struct AlphaNetLedgerEntry { NetAllocationNode *payload_; };
struct AlphaNetLedger {
  int entry_count_;
  int current_position_;
  AlphaNetLedgerEntry *head_;
  AlphaNetLedgerEntry *current_;
  AlphaNetLedger() : entry_count_(0), current_position_(0), head_(0), current_(0) {
    events.push_back("Ledger.ctor");
  }
  ~AlphaNetLedger() { events.push_back("Ledger.dtor"); }
  void remove_all() { events.push_back("Ledger.remove_all"); entry_count_=0; }
  void seek_pos(int pos) { current_position_=pos; current_=head_; }
  NetAllocationNode *next_entry() {
    current_=head_; current_position_=0;
    return current_ == 0 ? 0 : current_->payload_;
  }
};
struct NetSessionPickerTail {
  RECT panel_rect_, first_label_rect_;
  int field_3708_, field_370c_, field_3710_, field_3714_, field_3718_;
  RECT frame_rect_;
  int frame_color_, box_color_a_, box_color_b_, row_count_;
  int field_373c_, field_3740_, field_3744_, field_3748_;
  RECT rows_rect_, second_label_rect_;
  int row_height_, selected_row_color_, row_box_color_a_, row_box_color_b_;
  int boxes_per_row_, field_3780_;
  int selected_text_color_a_, selected_text_color_b_, selected_text_color_c_;
  int selected_text_color_d_, field_3794_, client_width_, client_height_;
  int rows_visible_, action_result_;
};
struct Net;
class NetSessionPickerWin : public GraphicWin {
 public:
  NetSessionPickerWin();
  virtual ~NetSessionPickerWin();
  void close();
  void refresh_player_list() { ++refresh_calls; }
  void refresh_if_focused();
  static void refresh_timer_callback(int);
  static int refresh_calls;
  Time refresh_timer_;
  Heap row_heap_;
  ListBox sessions_view_;
  PushButton create_button_, action_button_, cancel_button_;
  Net *net_;
  AlphaNetLedger sessions_;
  NetSessionPickerTail tail_;
};
int NetSessionPickerWin::refresh_calls=0;
Win *WinFocusWindow=0;
'''


def test_lifecycle_and_distinct_callback_identities_compile_actual_bodies(tmp_path):
    source = SOURCE.read_text()
    bodies = "\n".join(function(source, sig) for sig in (
        "NetSessionPickerWin::NetSessionPickerWin()",
        "NetSessionPickerWin::~NetSessionPickerWin()",
        "void NetSessionPickerWin::close()",
        "void __cdecl NetSessionPickerWin::refresh_timer_callback(int context)",
        "void NetSessionPickerWin::refresh_if_focused()",
    ))
    harness = COMMON + bodies + r'''
int main() {
  {
    NetSessionPickerWin owner;
    assert(events.size() >= 8);
    assert(events[0] == "GraphicWin.ctor");
    assert(events[1] == "Time.ctor");
    assert(events[2] == "Heap.ctor");
    assert(events[3] == "ListBox.ctor");
    assert(events[4] == "PushButton1.ctor");
    assert(events[5] == "PushButton2.ctor");
    assert(events[6] == "PushButton3.ctor");
    assert(events[7] == "Ledger.ctor");
    assert(owner.net_ == 0 && owner.tail_.row_count_ == 2);
    assert(owner.tail_.action_result_ == 0 && owner.tail_.boxes_per_row_ == 3);
    events.clear();
  }
  const char *expected[] = {
    "Time.close", "Ledger.remove_all", "ListBox.close",
    "PushButton1.close", "PushButton2.close", "PushButton3.close",
    "GraphicWin.close", "Ledger.dtor", "PushButton3.dtor",
    "PushButton2.dtor", "PushButton1.dtor", "ListBox.dtor",
    "Heap.dtor", "Time.dtor", "GraphicWin.dtor"
  };
  assert(events.size() == sizeof(expected)/sizeof(expected[0]));
  for (unsigned i=0; i<events.size(); ++i) assert(events[i] == expected[i]);

  NetSessionPickerWin owner;
  NetSessionPickerWin::refresh_calls=0;
  WinFocusWindow=0;
  NetSessionPickerWin::refresh_timer_callback(0);
  assert(NetSessionPickerWin::refresh_calls == 0);
  owner.refresh_if_focused();
  assert(NetSessionPickerWin::refresh_calls == 0);
  WinFocusWindow=&owner;
  owner.refresh_if_focused();
  assert(NetSessionPickerWin::refresh_calls == 1);
  WinFocusWindow=0;
  NetSessionPickerWin::refresh_timer_callback(1);
  assert(NetSessionPickerWin::refresh_calls == 1);
}
'''
    compile_and_run(tmp_path, harness, "picker-lifecycle-callback-")


CALLER_STUBS = r'''
struct Net;
static Heap *StringAllocationHeap=0;
struct Dialog { int field_4C_; };
struct Dialogs { void item(char *, int) {} };
struct EditGroup { void edit(char *, char *, int) { ++edits; }
  static int edits;
};
int EditGroup::edits=0;
struct DialogGroup : Dialog, Dialogs, EditGroup {};
static int exec_values[4];
static int exec_count;
static int poll_value;
static int pop_destructor_count;
static int popup_allocation_fails;
static int init_fail_call;
static int init_call_count;
static int enumerate_result;
static int join_result;
static int join_count;
static string join_player, join_password;
static int join_password_is_null;
static NetAllocationNode fixture_node;
static AlphaNetLedgerEntry fixture_entry;
static int mutate_password_on_credentials;
static char *password_after_credentials;
struct ParseBuffer { char str[128]; };
static ParseBuffer ParseStrBuffer[3];
static char *ParseStrBuffer0() { return ParseStrBuffer[0].str; }
struct BasePop {
  DialogGroup group_;
  ~BasePop() { ++pop_destructor_count; }
  int init(int, int) { ++init_call_count; return init_call_count==init_fail_call ? 9 : 0; }
  void string(char *) {}
  DialogGroup &dialogs() { return group_; }
  int exec(int, int) {
    int at=exec_count++;
    if (at==2 && mutate_password_on_credentials)
      fixture_node.password_=password_after_credentials;
    return exec_values[at];
  }
  int input_poll_state() { return poll_value; }
  void button(char *) {}
  void set_width(int) {}
};
static BasePop *PopupAllocHook() {
  return popup_allocation_fails ? 0 : new BasePop;
}
struct Net {
  int buffer_6E0_;
  Net() : buffer_6E0_(1) {}
  int enumerate_sessions(AlphaNetLedger *list, const char *) {
    if (enumerate_result) return enumerate_result;
    list->entry_count_=1; list->head_=&fixture_entry; list->current_=&fixture_entry;
    fixture_entry.payload_=&fixture_node;
    return 0;
  }
  NetAllocationNode *create_session(char *, int, char *) { return 0; }
  int join_session(NetAllocationNode *, char *player, char *password) {
    ++join_count; join_player=player ? player : "<null>";
    join_password_is_null=(password==0);
    join_password=password ? password : "<null>";
    return join_result;
  }
  int session_picker_dialog();
};
'''


def test_session_caller_reload_and_error_cleanup_from_actual_body(tmp_path):
    source = SOURCE.read_text()
    bodies = "\n".join(function(source, sig) for sig in (
        "struct SessionPickerAllocationHandoff",
        "NetSessionPickerWin::NetSessionPickerWin()",
        "NetSessionPickerWin::~NetSessionPickerWin()",
        "void NetSessionPickerWin::close()",
        "int Net::session_picker_dialog()",
    ))
    bodies = bodies.replace("struct SessionPickerAllocationHandoff {", "struct SessionPickerAllocationHandoff {")
    # The extracted declaration ends at its closing brace; preserve its source
    # semicolon when placed into the standalone host translation unit.
    helper = function(source, "struct SessionPickerAllocationHandoff") + ";"
    bodies = bodies.replace(function(source, "struct SessionPickerAllocationHandoff"), helper)
    # The staged constructor/close bodies are used by the real automatic local
    # in the caller. Remaining methods are only declarations for this harness.
    stubs = CALLER_STUBS.replace("int NetSessionPickerWin::close_event_count() { return 0; }", "")
    # A tiny Win/GraphicWin operation observer catches exactly one destructor
    # close path without copying candidate behavior into the fixture.
    stubs = stubs.replace("NetSessionPickerWin::NetSessionPickerWin();", "")
    harness = COMMON + stubs + bodies + r'''
static void reset_script() {
  events.clear(); button_ctor_index=0;
  exec_values[0]=1; exec_values[1]=0; exec_values[2]=0;
  exec_count=0; poll_value=0; pop_destructor_count=0; popup_allocation_fails=0;
  init_fail_call=0; init_call_count=0; enumerate_result=0; join_result=37;
  join_count=0; join_player.clear(); join_password.clear();
  join_password_is_null=0; mutate_password_on_credentials=0; EditGroup::edits=0;
  password_after_credentials=0; fixture_node.password_=0;
  fixture_node.session_name_=(char *)"lobby"; StringAllocationHeap=(Heap *)0x1234;
}
static int event_count(const char *needle) {
  int count=0;
  for (unsigned i=0; i<events.size(); ++i) if (events[i]==needle) ++count;
  return count;
}
int main() {
  Net net;
  reset_script(); popup_allocation_fails=1;
  assert(net.session_picker_dialog()==4);
  assert(event_count("GraphicWin.close")==1);
  assert(StringAllocationHeap==(Heap *)0x1234);
  assert(pop_destructor_count==0 && join_count==0);

  reset_script(); init_fail_call=1;
  assert(net.session_picker_dialog()==0);
  assert(event_count("GraphicWin.close")==1);
  assert(pop_destructor_count==1 && join_count==0);
  assert(StringAllocationHeap==(Heap *)0x1234);

  reset_script(); enumerate_result=1;
  assert(net.session_picker_dialog()==0);
  assert(event_count("GraphicWin.close")==1);
  assert(pop_destructor_count==1 && join_count==0);

  reset_script();
  strcpy(ParseStrBuffer[0].str,"Ariadne");
  strcpy(ParseStrBuffer[1].str,"door-key");
  mutate_password_on_credentials=1;
  password_after_credentials=(char *)"newly-required";
  assert(net.session_picker_dialog()==37);
  assert(event_count("GraphicWin.close")==1);
  assert(exec_count==3 && join_count==1);
  assert(join_player=="Ariadne" && join_password=="door-key");
  assert(join_password_is_null==0 && pop_destructor_count==1);
  assert(StringAllocationHeap==(Heap *)0x1234);

  reset_script();
  fixture_node.password_=(char *)"previously-required";
  exec_values[0]=1; exec_values[1]=0; exec_values[2]=0;
  mutate_password_on_credentials=1; password_after_credentials=0;
  strcpy(ParseStrBuffer[0].str,"Basil");
  strcpy(ParseStrBuffer[1].str,"ignored-old-secret");
  assert(net.session_picker_dialog()==37);
  assert(event_count("GraphicWin.close")==1);
  assert(join_count==1 && join_player=="Basil");
  assert(join_password_is_null==1 && join_password=="<null>");
  assert(EditGroup::edits==2);
  assert(pop_destructor_count==1);

  reset_script(); exec_values[0]=1; exec_values[1]=-1;
  assert(net.session_picker_dialog()==0);
  assert(event_count("GraphicWin.close")==1);
  assert(exec_count==2 && join_count==0 && pop_destructor_count==1);

  reset_script(); init_fail_call=2;
  assert(net.session_picker_dialog()==0);
  assert(event_count("GraphicWin.close")==1);
  assert(pop_destructor_count==1 && join_count==0);
}
'''
    # Remove the impossible declaration helper text and define the one OS CRT
    # spelling used by the recovered creation path even though this suite
    # exercises existing-session/error branches only.
    harness = harness.replace("static char *ParseStrBuffer0() { return ParseStrBuffer[0].str; }", "")
    harness = harness.replace("int NetSessionPickerWin::close_event_count() { return 0; }", "")
    harness = harness.replace("struct Net {", "static char *_itoa(int n, char *out, int) { std::sprintf(out, \"%d\", n); return out; }\nstruct Net {")
    compile_and_run(tmp_path, harness, "picker-session-caller-")
