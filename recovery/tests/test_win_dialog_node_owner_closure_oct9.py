"""Typed source and real-header controls for the recovered dialog focus node."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import re


ROOT = Path(__file__).resolve().parents[2]


def test_complete_source_closure_uses_one_typed_node_and_virtual_dispatch():
    win = (ROOT / "src/win.cpp").read_text()
    editgroup = (ROOT / "src/editgroup.cpp").read_text()
    filewin = (ROOT / "src/jackal/filewin.cpp").read_text()
    header = (ROOT / "src/winnodelist.h").read_text()
    assert "struct WinDialogNode" in header
    assert "Win *focus_owner_;" in header and "void *allocation_;" in header
    assert "WinDialogNode *next_;" in header and "WinDialogNode *previous_;" in header
    assert "uint32_t unknown_00_;" in header and "uint8_t unknown_14_[8];" in header
    assert "WinNodeListFocusEntry" not in editgroup
    assert "FileWinFocusNode" not in filewin
    assert "call_slot_cc" not in win
    assert "newFocus->on_dialog_focus(0)" in win
    assert "target->on_dialog_focus(1)" in win
    assert "parent_window->list_.allocator_" in win
    assert "Heap *heap = parent_window->list_.allocator_" not in win
    for owner in (win, editgroup, filewin):
        assert "reinterpret_cast<char *>(list_.current_)" not in owner
        assert "reinterpret_cast<char *>(parent->list_.current_)" not in owner
        assert "static_cast<FileWinFocusNode" not in owner


def test_real_node_header_and_list_teardown_preserve_unknown_bytes():
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    program = r'''#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>
#include "winnodelist.h"
static_assert(std::is_trivial<WinDialogNode>::value, "node has no invented lifetime");
int main() {
  WinDialogNode *a = static_cast<WinDialogNode *>(std::malloc(sizeof(WinDialogNode)));
  WinDialogNode *b = static_cast<WinDialogNode *>(std::malloc(sizeof(WinDialogNode)));
  std::memset(a, 0xA5, sizeof(*a)); std::memset(b, 0x5A, sizeof(*b));
  a->next_ = b; a->previous_ = b; b->next_ = a; b->previous_ = a;
  alignas(WinNodeList) unsigned char storage[sizeof(WinNodeList)];
  WinNodeList *list = new (storage) WinNodeList;
  list->head_ = a; list->current_ = b; list->count_ = 2;
  list->allocator_ = reinterpret_cast<Heap *>(1);
  unsigned char a_prefix[sizeof(a->unknown_00_)], b_suffix[sizeof(b->unknown_14_)];
  std::memcpy(a_prefix, &a->unknown_00_, sizeof(a_prefix));
  std::memcpy(b_suffix, b->unknown_14_, sizeof(b_suffix));
  list->~WinNodeList();
  assert(list->head_ == 0 && list->count_ == 0 && list->tail_ == 0);
  assert(list->current_ == b);
  assert(std::memcmp(a_prefix, &a->unknown_00_, sizeof(a_prefix)) == 0);
  assert(std::memcmp(b_suffix, b->unknown_14_, sizeof(b_suffix)) == 0);
  std::free(a); std::free(b);
}'''
    with tempfile.TemporaryDirectory(prefix="win-dialog-node-") as temp:
        source, binary = Path(temp) / "control.cpp", Path(temp) / "control"
        source.write_text(program)
        subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(source), "-o", str(binary)],
                       cwd=ROOT, check=True, timeout=60)
        subprocess.run([str(binary)], check=True, timeout=10)


def test_actual_set_parent_dialog_insertion_failure_and_allocator_reread(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    source = (ROOT / "src/win.cpp").read_text()
    start = source.index("void Win::set_parent_dialog(Win *dialog)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated set_parent_dialog")
    harness = r'''#include <cassert>
#include <cstdlib>
#include <vector>
#include "winnodelist.h"
static std::vector<void *> allocations;
static int fail_size = -1;
static void *mem_get(std::size_t n) {
  if ((int)n == fail_size) return 0;
  void *p = std::malloc(n == 0x1c ? sizeof(WinDialogNode) : n); allocations.push_back(p); return p;
}
class Heap { public: Win *owner; int calls; bool mutate; void *get(std::size_t n); };
class Win { public: unsigned iFlags_; unsigned iSomeFlag_; Win *win_parent_; WinNodeList list_;
  Win():iFlags_(0),iSomeFlag_(0),win_parent_(0){} void set_parent_dialog(Win*);
};
void *Heap::get(std::size_t n) { ++calls; if (mutate) { owner->list_.allocator_ = 0; mutate = false; }
  return mem_get(n); }
''' + body + r'''
static void cleanup(Win &w) {
  w.list_.allocator_ = reinterpret_cast<Heap *>(1);
  for (void *p : allocations) std::free(p); allocations.clear();
}
int main() {
  { Win parent, child; Heap heap = {&parent,0,true}; parent.list_.allocator_=&heap;
    child.set_parent_dialog(&parent);
    assert(heap.calls == 1); assert(parent.list_.count_ == 1);
    assert(parent.list_.head_ == parent.list_.current_);
    assert(parent.list_.head_->focus_owner_ == &child);
    assert(parent.list_.head_->next_ == parent.list_.head_);
    assert(parent.list_.head_->allocation_ != 0); cleanup(parent); }
  { Win parent, child; fail_size=0x1c; child.set_parent_dialog(&parent);
    assert(parent.list_.head_ == 0 && parent.list_.count_ == 0 && child.win_parent_ == &parent);
    fail_size=-1; }
  { Win parent, child; fail_size=4; child.set_parent_dialog(&parent);
    assert(parent.list_.head_ != 0 && parent.list_.count_ == 0 && parent.list_.current_ == 0);
    assert(parent.list_.head_->focus_owner_ == &child && parent.list_.head_->allocation_ == 0);
    assert(parent.list_.head_->next_ == parent.list_.head_); cleanup(parent); fail_size=-1; }
  { Win parent, first, second; first.set_parent_dialog(&parent); WinDialogNode *head=parent.list_.head_;
    fail_size=0x1c; second.set_parent_dialog(&parent);
    assert(head->next_ == 0 && parent.list_.count_ == 1 && second.win_parent_ == &parent);
    cleanup(parent); fail_size=-1; }
  { Win parent, first, second; first.set_parent_dialog(&parent); WinDialogNode *head=parent.list_.head_;
    fail_size=4; second.set_parent_dialog(&parent); WinDialogNode *added=head->previous_;
    assert(parent.list_.count_ == 1 && parent.list_.current_ == head);
    assert(added->focus_owner_ == &second && added->allocation_ == 0);
    assert(added->next_ == head && head->previous_ == added); cleanup(parent); fail_size=-1; }
}'''
    path = tmp_path / "actual_set_parent.cpp"
    executable = tmp_path / "actual_set_parent"
    path.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(path), "-o",
                             str(executable)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

    # A cached-allocator mutation must fail the callback-rebind case above.
    mutant = body.replace("void Win::set_parent_dialog(Win *dialog) {",
                          "void Win::set_parent_dialog(Win *dialog) {\n Heap *cached_allocator = dialog->list_.allocator_;")
    mutant = mutant.replace("dialog->list_.allocator_", "cached_allocator")
    # Restore the injected initializer, which the broad replacement also sees.
    mutant = mutant.replace("Heap *cached_allocator = cached_allocator;",
                            "Heap *cached_allocator = dialog->list_.allocator_;")
    mutant_path = tmp_path / "mutant_set_parent.cpp"
    mutant_binary = tmp_path / "mutant_set_parent"
    mutant_path.write_text(harness.replace(body, mutant))
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(mutant_path), "-o",
                             str(mutant_binary)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(mutant_binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0


def test_actual_set_dialog_focus_reloads_node_after_virtual_callback(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    source = (ROOT / "src/win.cpp").read_text()
    start = source.index("void Win::set_dialog_focus(Win * window)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated set_dialog_focus")
    harness = r'''#include <cassert>
#include <vector>
#include "winnodelist.h"
static Win *WinPendingFocus;
static Win *parent_window;
static std::vector<int> events;
class Win { public: WinNodeList list_; virtual ~Win(){} virtual void on_dialog_focus(int){}
  void set_dialog_focus(Win*); };
class Probe: public Win { public: int id; void on_dialog_focus(int value) { events.push_back(id*10+value);
  if(id==1 && value==0) parent_window->list_.current_=parent_window->list_.head_->next_->next_; } };
''' + body + r'''
int main() { Win parent; Probe old_focus, requested, callback_focus;
  old_focus.id=1; requested.id=2; callback_focus.id=3; parent_window=&parent;
  WinDialogNode a,b,c; a.next_=&b; b.next_=&c; c.next_=&a;
  a.previous_=&c; b.previous_=&a; c.previous_=&b;
  a.focus_owner_=&old_focus; b.focus_owner_=&requested; c.focus_owner_=&callback_focus;
  parent.list_.head_=&a; parent.list_.current_=&a; parent.list_.count_=3;
  parent.set_dialog_focus(&requested);
  assert((events==std::vector<int>{10,31}));
  assert(WinPendingFocus==&old_focus && parent.list_.current_==&c);
  parent.list_.allocator_=reinterpret_cast<Heap*>(1);
}'''
    path = tmp_path / "actual_focus.cpp"
    executable = tmp_path / "actual_focus"
    path.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(path), "-o",
                             str(executable)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def test_actual_init_list_insertion_rereads_mutated_allocator(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    source = (ROOT / "src/win.cpp").read_text()
    start = source.index("        int initCount = parent_window->list_.count_;")
    end = source.index("\n    this->iSomeFlag_ |= 4;", start)
    insertion = source[start:end]
    harness = r'''#include <cassert>
#include <cstdlib>
#include <vector>
#include "winnodelist.h"
static std::vector<void*> allocations; static int heap_calls, malloc_calls;
static void *mem_get(std::size_t n) { ++malloc_calls; void *p=std::malloc(n==0x1c?sizeof(WinDialogNode):n);
  allocations.push_back(p); return p; }
class Heap { public: Win *owner; bool mutate; void *get(std::size_t n); };
class Win { public: WinNodeList list_; int register_in_parent(Win*); };
void *Heap::get(std::size_t n) { ++heap_calls; void *p=std::malloc(n==0x1c?sizeof(WinDialogNode):n);
  allocations.push_back(p); if(mutate) {owner->list_.allocator_=0; mutate=false;} return p; }
int Win::register_in_parent(Win *parent_window) {
  if (true) {
''' + insertion + r'''
  return 0;
}
static void cleanup(Win &w) { w.list_.allocator_=reinterpret_cast<Heap*>(1);
  for(void *p:allocations) std::free(p); allocations.clear(); }
int main() {
  { Win parent, child; Heap heap={&parent,true}; parent.list_.allocator_=&heap;
    child.register_in_parent(&parent); assert(heap_calls==1 && malloc_calls==1);
    assert(parent.list_.count_==1 && parent.list_.head_==parent.list_.current_);
    assert(parent.list_.head_->focus_owner_==&child && parent.list_.head_->allocation_!=0);
    cleanup(parent); }
  { Win parent, child; WinDialogNode head; head.next_=&head; head.previous_=&head;
    head.allocation_=reinterpret_cast<void*>(1); parent.list_.head_=&head; parent.list_.count_=1;
    child.register_in_parent(&parent); assert(parent.list_.count_==2);
    assert(head.next_->focus_owner_==&child && head.next_->next_==&head);
    assert(head.previous_==head.next_); cleanup(parent); }
}'''
    path = tmp_path / "actual_init_insert.cpp"
    executable = tmp_path / "actual_init_insert"
    path.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(path), "-o",
                             str(executable)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr


def test_actual_key_down_focus_result_and_accumulator_mutant(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    source = (ROOT / "src/win.cpp").read_text()
    start = source.index("int Win::key_down_event(int key)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated key_down_event")
    harness = r'''#include <cassert>
#include <cstdint>
#include "winnodelist.h"
class Win; typedef int (*WinKeyHookFn)(int);
static Win *WinCallbackWindow;
static int MapVirtualKeyA(unsigned int, unsigned int) { return 1; }
class Scroll { public: int on_key_click(int,int){return 0;} };
class Sound { public: int is_playing(){return 1;} void play(){} };
class Win { public: WinNodeList list_; unsigned iFlags_,iSomeFlag_; Scroll *scroll_vert_,*scroll_horz_;
  WinKeyHookFn key_hook_; Sound *val_15_; int down_result,recursive_result;
  Win():iFlags_(0),iSomeFlag_(0),scroll_vert_(0),scroll_horz_(0),key_hook_(0),val_15_(0),down_result(0),recursive_result(0){}
  virtual int key_down_event(int); virtual int on_key_down(int){return down_result;}
  int key_click_event(int,int){return 0;}
};
class FocusReceiver: public Win { public: int calls; FocusReceiver():calls(0){}
  int key_down_event(int) override {++calls; return recursive_result;} };
''' + body + r'''
static int hook_value(int){return 5;}
static void clean(Win &w){w.list_.allocator_=reinterpret_cast<Heap*>(1);}
int main(){
  { Win parent; FocusReceiver child; WinDialogNode node; node.focus_owner_=&child;
    parent.list_.head_=&node; parent.list_.current_=&node; parent.list_.count_=1;
    parent.down_result=9; child.recursive_result=0; assert(parent.key_down_event(7)==9);
    assert(child.calls==1 && WinCallbackWindow==&parent); clean(parent); }
  { Win parent; FocusReceiver child; WinDialogNode node; node.focus_owner_=&child;
    parent.list_.head_=&node; parent.list_.current_=&node; parent.list_.count_=1;
    child.recursive_result=1; parent.down_result=9; assert(parent.key_down_event(7)==1);
    assert(child.calls==1); clean(parent); }
  { Win parent; parent.down_result=9; assert(parent.key_down_event(7)==9);
    parent.key_hook_=&hook_value; assert(parent.key_down_event(7)==14); }
}'''
    path, executable = tmp_path / "actual_key_down.cpp", tmp_path / "actual_key_down"
    path.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(path), "-o",
                             str(executable)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

    mutant = body.replace("int ebx = 0;", "int ebx = static_cast<int>(reinterpret_cast<uintptr_t>(list_.current_->focus_owner_));", 1)
    assert mutant != body
    mutant_path, mutant_binary = tmp_path / "key_down_pointer_mutant.cpp", tmp_path / "key_down_pointer_mutant"
    mutant_path.write_text(harness.replace(body, mutant))
    result = subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(mutant_path), "-o",
                             str(mutant_binary)], cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(mutant_binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0
