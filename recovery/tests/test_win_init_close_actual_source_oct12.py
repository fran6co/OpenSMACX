"""Whole-method host controls for Win initialization and close.

Win storage declarations, callback declarations, BorderSizing, AutoSound and
WinNodeList come from current headers. The complete constructor, destructor,
init, close and rectangle converters come from product source. Dependency
doubles observe calls and allocation effects; this is not VC6 layout, unwind,
linked-preservation, native unsafe-path or game-runtime proof.
"""
from dataclasses import fields
import os
from pathlib import Path
import re
import resource
import shutil
import struct
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "29def3c84d9f261f0b29d368ccdda6677227d55f"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
INIT = "int Win::init(int x, int y, int width, int height, char * caption,"
CLOSE = "void Win::close()"


def definition(source, signature):
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), source)
    start = masked.index(signature)
    assert masked.find(signature, start + len(signature)) < 0
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        depth += (masked[end] == "{") - (masked[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated " + signature)


def test_only_two_bodies_change_and_all_source_obligations_survive():
    path = ROOT / "src/win.cpp"
    original = subprocess.check_output(["git", "show", BASE + ":src/win.cpp"],
                                       cwd=ROOT, text=True)
    current = path.read_text()
    restored = current
    for signature in (INIT, CLOSE):
        restored = restored.replace(definition(current, signature),
                                    definition(original, signature), 1)
    assert restored == original
    before = read_text(original, path)
    after = read_text(current, path)
    assert len(before) == len(after)
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name not in {"region", "line", "origin"}:
                assert getattr(a, field.name) == getattr(b, field.name), field.name
        assert [(x.kind, x.key, x.prose) for x in a.origin] == [
            (x.kind, x.key, x.prose) for x in b.origin]
    init = next(row for row in after if row.address == 0x5EBD80)
    assert init.image_spans == ((0x5EBD80, 0x5EC621), (0x662C80, 0x662CCC))


def test_pinned_original_branch_receivers_failures_and_lifecycle():
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                              ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE for original evidence")
    image = PeImage(path.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for span in (Span(0x5EBD80, 0x5EC621), Span(0x5EB640, 0x5EBC8A)):
        decoded = list(decoder.disasm(image.read(span), span.start))
        assert sum(row.size for row in decoded) == span.end - span.start
        instructions.update({row.address: (row.mnemonic, row.op_str) for row in decoded})
    anchors = {
        0x5EBF9D: ("cmp", "dword ptr [esi + 0x3fc], 0x96"),
        0x5EC12F: ("mov", "ebp, dword ptr [ebp]"),
        0x5EC0ED: ("lea", "ebp, [esi + 0x14c]"),
        0x5EC172: ("lea", "ebp, [esi + 0x14c]"),
        0x5EC17C: ("push", "ebp"),
        0x5EC195: ("call", "0x5ef050"),
        0x5EC19D: ("mov", "eax, dword ptr [esi + 0x150]"),
        0x5EC222: ("je", "0x5ec261"),
        0x5EC265: ("call", "dword ptr [eax + 0x108]"),
        0x5EC2A6: ("jne", "0x5ec2b4"),
        0x5EC44C: ("test", "eax, 0x100000"),
        0x5EC451: ("je", "0x5ec5fc"),
        0x5EC46F: ("test", "ah, 0x10"),
        0x5EC476: ("je", "0x5ec5fc"),
        0x5EC47C: ("mov", "eax, dword ptr [ebx + 0x98]"),
        0x5EC4EA: ("mov", "ecx, dword ptr [ebx + 0xcc]"),
        0x5EC50F: ("test", "ecx, ecx"),
        0x5EC511: ("je", "0x5ec5d7"),
        0x5EC540: ("mov", "dword ptr [edx + 0xc], eax"),
        0x5EC54F: ("test", "ecx, ecx"),
        0x5EC551: ("je", "0x5ec5d7"),
        0x5EC59B: ("mov", "ecx, dword ptr [ebx + 0xd0]"),
        0x5EC5D7: ("mov", "ecx, dword ptr [ebx + 0xd4]"),
        0x5EC5F6: ("mov", "dword ptr [esi + 0xc4], ebx"),
        0x5EC602: ("or", "al, 4"),
        0x5EC371: ("mov", "ecx, dword ptr [esi + 0x440]"),
        0x5EC37B: ("call", "dword ptr [edx + 4]"),
        0x5EC42C: ("mov", "ecx, dword ptr [esi + 0x43c]"),
        0x5EC436: ("call", "dword ptr [edx + 4]"),
        0x5EB86A: ("mov", "dword ptr [edi + 0xd8], eax"),
        0x5EB99A: ("mov", "dword ptr [esi + 0x184], ebp"),
        0x5EB9B7: ("cmp", "dword ptr [esi + 0xdc], ebp"),
        0x5EB9BD: ("jne", "0x5eba1d"),
        0x5EBB5A: ("mov", "dword ptr [esi + 0x15c], ebp"),
        0x5EBB6C: ("mov", "dword ptr [esi + 0x168], ebp"),
        0x5EBC80: ("call", "0x62bbf0"),
    }
    for address, expected in anchors.items():
        assert instructions[address] == expected, hex(address)
    # All six scalar-new unwind states retain their native support span. States
    # four/five free raw Scroll storage if construction throws, rather than
    # calling a completed Scroll destructor or retaining a placement-new leak.
    info = struct.unpack("<7I", image.read(Span(0x67F8E8, 0x67F904)))
    assert info == (0x19930520, 6, 0x67F908, 0, 0, 0, 0)
    unwind = struct.unpack("<12I", image.read(Span(0x67F908, 0x67F938)))
    assert unwind == tuple(value for handler in (
        0x662C80, 0x662C8B, 0x662C96, 0x662CA1, 0x662CAC, 0x662CB7)
        for value in (0xFFFFFFFF, handler))
    support = list(decoder.disasm(image.read(Span(0x662C80, 0x662CCC)), 0x662C80))
    assert sum(row.size for row in support) == 0x4C
    cold = {row.address: (row.mnemonic, row.op_str) for row in support}
    assert cold[0x662CAC] == ("mov", "eax, dword ptr [ebp + 4]")
    assert cold[0x662CB0] == ("call", "0x64557f")
    assert cold[0x662CB7] == ("mov", "eax, dword ptr [ebp + 4]")
    assert cold[0x662CBB] == ("call", "0x64557f")
    assert cold[0x662CC2] == ("mov", "eax, 0x67f8e8")


DEPENDENCIES = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#define __cdecl
typedef void *HWND;
typedef void *HDC;
typedef void *HCURSOR;
typedef void *LPVOID;
typedef char *LPSTR;
typedef int BOOL;
#define UNUSED(x)
struct RECT { int32_t left, top, right, bottom; };
struct Font {};
struct Sprite {};
class Win; class GraphicWin; class ImageButton; class Menu; class Scroll; class Sound;
static std::vector<std::string> events;
static std::vector<void *> allocations;
static int fail_size, fail_buffer, buffer_calls, buffer_live, throw_buffer;
static int caption_calls, paint_calls, rect_calls, heap_calls, fallback_calls;
static int menu_result, desired_visible, mouse_capture_calls;
static int scroll_alloc_calls,scroll_ctor_calls,scroll_live_allocations,scroll_dealloc_calls;
static int fail_scroll,throw_scroll;
static Win *active, *mutated_parent;
static void (*allocation_hook)(int);
static void (*buffer_hook)();
static void (*menu_hook)(Win *);
static void (*hide_hook)(Win *);
static void (*scroll_init_hook)(Scroll *,Win *,int);
static void *allocate(int size) {
  events.push_back("alloc:"+std::to_string(size));
  if(size==fail_size) return 0;
  void *p=std::malloc(size==0x1c ? sizeof(void*)*8 : size);
  assert(p); std::memset(p,0xA5,size==0x1c ? sizeof(void*)*8 : size);
  allocations.push_back(p); return p;
}
static void probe_free(void *p) {
  events.push_back("free");
  auto i=std::find(allocations.begin(),allocations.end(),p);
  assert(i!=allocations.end()); allocations.erase(i); std::free(p);
}
static void *mem_get(int size) {
  ++fallback_calls; void *p=allocate(size);
  if(allocation_hook) allocation_hook(size); return p;
}
struct Buffer {
  static void *operator new(std::size_t size) noexcept {
    int call=++buffer_calls; events.push_back("buffer:new");
    if(call==fail_buffer) return 0; return std::malloc(size);
  }
  static void operator delete(void *p) noexcept { std::free(p); }
  Buffer() { if(buffer_calls==throw_buffer) throw 8; ++buffer_live;
    if(buffer_hook) buffer_hook(); }
  virtual ~Buffer() { --buffer_live; events.push_back("buffer:delete"); }
};
#include "winnodelist.h"
#include "heap.h"
// The actual AutoSound declaration and Win storage follow below. No original
// offset accesses or replacement vtable tables are used in this host control.
'''


SUPPORT = r'''
class GraphicWin : public Win {};
class ImageButton : public GraphicWin {};
class Scroll : public GraphicWin {
 public:
  // A nonthrowing class allocation function gives the host new-expression the
  // explicit null-return policy used by the configured VC6 runtime. Host size
  // and exception machinery do not establish the original allocation ABI.
  static void *operator new(std::size_t size) noexcept {
    ++scroll_alloc_calls;events.push_back("scroll:allocate");
    if(scroll_alloc_calls==fail_scroll) return 0;
    ++scroll_live_allocations;return std::malloc(size);
  }
  static void operator delete(void *p) noexcept {
    --scroll_live_allocations;++scroll_dealloc_calls;
    events.push_back("scroll:deallocate");std::free(p);
  }
  Scroll() {
    ++scroll_ctor_calls;events.push_back("scroll:construct");
    if(scroll_ctor_calls==throw_scroll) throw 73;
    events.push_back("scroll:new");
  }
  ~Scroll() { events.push_back("scroll:delete"); }
  int init_horz_nc(int x,int y,int n,Win *p,int id) {
    assert(x==0 && p==active && id==-2); events.push_back("scroll:horz");
    if(scroll_init_hook) scroll_init_hook(this,p,4);
    (void)y;(void)n; return 0;
  }
  int init_vert_nc(int x,int y,int n,Win *p,int id) {
    assert(y==0 && p==active && id==-1); events.push_back("scroll:vert");
    if(scroll_init_hook) scroll_init_hook(this,p,8);
    (void)x;(void)n; return 0;
  }
};
class Menu : public GraphicWin {
 public:
  virtual int init(Win *owner) {
    events.push_back("menu:init"); if(menu_hook) menu_hook(owner); return menu_result;
  }
  virtual int requested_height() { return 6; }
};
static Win *WinPointerOwner1,*WinPointerOwner2,*WinPointerOwner3,*WinPointerOwner4;
static Win *WinBubbleCompanion,*WinHoverWindow,*WinCallbackWindow;
static int WinBubbleActive,WinRootCount;
static Win *WinRootWindows[40];
static RECT WinBubbleRect;
static HDC WinSharedHdc;
static int WinScreenWidth=640,WinScreenHeight=480,ScrollDefaultThickness=7;
static uint32_t WinStaticDefaults[10]={247,24,~0u,1,1,8,~0u,7,2,0};
static WinDynamicDefaultsTable WinDynamicDefaults={11,0,{22,33}};
static const char WinMsgTooManyChildren[]="children",WinMsgIncreaseMaxChildren[]="maxchild";
static const char WinMsgTooManyParents[]="parents",WinMsgIncreaseMaxParents[]="maxparent";
static int MessageBoxA(int,const char *,const char *,int) { throw 4; }
static int update_screen(RECT *,Win *) { events.push_back("screen"); return 0; }
static void flip(RECT *) { events.push_back("flip"); }
AutoSound::AutoSound() {}
void AutoSound::close() { events.push_back("sound:close"); }
Heap::Heap() {}
Heap::~Heap() {}
void *Heap::get(std::size_t size) {
  ++heap_calls; void *p=allocate(static_cast<int>(size));
  if(allocation_hook) allocation_hook(static_cast<int>(size)); return p;
}
void Win::hide() { events.push_back("hide"); if(hide_hook) hide_hook(this); }
void Win::show(int) { events.push_back("show"); }
int Win::is_visible() { return (iSomeFlag_&1)!=0; }
int Win::is_descendant(Win *candidate) {
  for(int i=0;i<child_count_;++i)
    if(children_[i]==candidate || children_[i]->is_descendant(candidate)) return 1;
  return 0;
}
void Win::do_caption_buttons() { ++caption_calls; events.push_back("caption"); }
void Win::set_rects() { ++rect_calls; events.push_back("rects"); }
void Win::on_nc_paint(RECT *,int) { ++paint_calls; events.push_back("paint"); }
void Win::on_lose_mouse_capture() { ++mouse_capture_calls; events.push_back("capture"); }
GraphicWin *Win::get_graphic_win() { return 0; }
'''


CASES = r'''
static void reset_observations() {
  events.clear(); fail_size=fail_buffer=throw_buffer=0; buffer_calls=0;
  caption_calls=paint_calls=rect_calls=heap_calls=fallback_calls=0;
  menu_result=desired_visible=mouse_capture_calls=0;
  scroll_alloc_calls=scroll_ctor_calls=scroll_dealloc_calls=fail_scroll=throw_scroll=0;
  allocation_hook=0;buffer_hook=0;menu_hook=0;hide_hook=0;scroll_init_hook=0;active=0;mutated_parent=0;
}
static void init_ok(Win &w,int flags=0,Win *parent=0,Menu *menu=0,char *text=0) {
  active=&w; assert(w.init(30,40,100,80,text,flags,parent,menu,0)==0);
  assert((w.iSomeFlag_&4)!=0);
}
static bool rectangle(RECT r,int a,int b,int c,int d) {
  return r.left==a&&r.top==b&&r.right==c&&r.bottom==d;
}
static WinDialogNode *node(Win *owner) {
  auto p=static_cast<WinDialogNode *>(allocate(0x1c));
  p->focus_owner_=owner;p->next_=p;p->previous_=p;p->allocation_=allocate(4);return p;
}
static void abandon_ring(Win &w) {
  w.list_.head_=w.list_.current_=0;w.list_.count_=w.list_.tail_=0;
  w.child_count_=0;
  for(void *p:allocations) std::free(p);allocations.clear();
}
struct PaintProbe : Win {
  GraphicWin *get_graphic_win() override {
    iSomeFlag_|=1;return reinterpret_cast<GraphicWin *>(this);
  }
};
static Win *allocation_parent,*allocation_child;
static WinDialogNode *replacement,*original_node;
static Heap heap_a,heap_b;
static int mutation;
static Win *poison_slots[150];
static void mutate_allocation(int size) {
  if(size==0x1c && mutation==1) {
    allocation_parent->list_.allocator_=&heap_b;
    allocation_child->win_parent_=mutated_parent;
  }
  if(size==4 && mutation==2) {
    original_node=allocation_parent->list_.head_;
    allocation_parent->list_.head_=replacement;
  }
  if(size==4 && mutation==3) {
    original_node=allocation_parent->list_.current_;
    allocation_parent->list_.current_=replacement;
  }
}
static void change_parent(Win *w) { w->win_parent_=mutated_parent; }
static void block_registration(Win *w) { w->iSomeFlag_|=8; }
static void poison_geometry(Win *w) {
  if(w==active) {
    w->child_count_=150;
    for(int i=0;i<150;++i) w->children_[i]=poison_slots[i];
    hide_hook=0;
  }
}
static void poison_after_close() {
  if(buffer_calls==4) { poison_geometry(active);buffer_hook=0; }
}
struct ReplacementScroll : Scroll {
  int shown=0;
  void show(int visible) override { assert(visible==0);++shown;events.push_back("replacement:show"); }
};
static ReplacementScroll *replacement_scroll;
static Scroll *displaced_scroll;
static void replace_scroll(Scroll *original,Win *owner,int axis) {
  displaced_scroll=original;
  if(axis==4) { assert(owner->scroll_horz_==original);owner->scroll_horz_=replacement_scroll; }
  else { assert(owner->scroll_vert_==original);owner->scroll_vert_=replacement_scroll; }
}
static void scroll_controls() {
  for(int axis:{4,8}) {
    { Win owner;reset_observations();fail_scroll=1;active=&owner;
      assert(owner.init(0,0,40,50,0,axis,0,0,0)==4);
      assert(scroll_alloc_calls==1 && scroll_ctor_calls==0 && scroll_live_allocations==0);
      assert(!owner.scroll_horz_ && !owner.scroll_vert_ && !(owner.iSomeFlag_&4));owner.close(); }
    { Win owner;reset_observations();throw_scroll=1;active=&owner;
      try {owner.init(0,0,40,50,0,axis,0,0,0);assert(false);}
      catch(int error) {assert(error==73);}
      assert(scroll_alloc_calls==1 && scroll_ctor_calls==1 && scroll_dealloc_calls==1);
      assert(scroll_live_allocations==0 && !owner.scroll_horz_ && !owner.scroll_vert_);
      assert(std::count(events.begin(),events.end(),"scroll:delete")==0);
      assert(events.back()=="scroll:deallocate");owner.close(); }
    { Win owner;reset_observations();replacement_scroll=new ReplacementScroll;
      events.clear();scroll_init_hook=replace_scroll;init_ok(owner,axis);
      assert(replacement_scroll->shown==1 && std::count(events.begin(),events.end(),"show")==0);
      assert((axis==4?owner.scroll_horz_:owner.scroll_vert_)==replacement_scroll);
      scroll_init_hook=0;owner.close();delete displaced_scroll;
      assert(scroll_live_allocations==0); }
  }
  { Win owner;reset_observations();throw_scroll=2;active=&owner;
    try {owner.init(0,0,40,50,0,12,0,0,0);assert(false);}
    catch(int error) {assert(error==73);}
    assert(owner.scroll_horz_ && !owner.scroll_vert_ && scroll_live_allocations==1);
    assert(scroll_dealloc_calls==1 && std::count(events.begin(),events.end(),"show")==1);
    owner.close();assert(scroll_live_allocations==0 && scroll_dealloc_calls==2); }
}
static void close_controls() {
  { Win parent,child;
    auto n=node(&child);parent.list_.head_=parent.list_.current_=n;
    parent.list_.count_=1;parent.list_.tail_=123;
    child.win_parent_=&parent;child.iFlags_=0x100000;
    child.palette_seed_cache_=123;child.cached_outer_rect_={9,10,11,12};
    child.close();
    assert(parent.list_.head_==0 && parent.list_.count_==0 && parent.list_.tail_==-1);
    assert(child.palette_seed_cache_==0 && rectangle(child.cached_outer_rect_,0,0,0,0));
    assert(child.poWinBase_==&child && child.win_parent_==0);
  }
  { Win parent,unlisted,listed;
    auto n=node(&listed);parent.list_.head_=parent.list_.current_=n;
    parent.list_.count_=1;parent.list_.tail_=123;
    unlisted.win_parent_=&parent;unlisted.iFlags_=0x100000;unlisted.close();
    assert(parent.list_.tail_==0 && parent.list_.count_==1);
    // Let real close release this list; the node owner is deliberately not a child.
    parent.close();assert(allocations.empty());
  }
  { Win owner;auto n=node(&owner);owner.list_.head_=owner.list_.current_=n;
    owner.list_.count_=1;owner.list_.allocator_=&heap_a;
    void *payload=n->allocation_;events.clear();owner.close();
    assert(owner.list_.head_==0 && owner.list_.count_==0 && owner.list_.tail_==0);
    assert(owner.list_.current_==n && n->allocation_==payload);
    assert(std::find(events.begin(),events.end(),"free")==events.end());
    abandon_ring(owner);
  }
  { Win owner;auto a=node(&owner),b=node(&owner);
    a->next_=b;a->previous_=b;b->next_=a;b->previous_=a;
    owner.list_.head_=a;owner.list_.current_=b;owner.list_.count_=2;
    events.clear();owner.close();
    assert(allocations.empty() && owner.list_.head_==0 && owner.list_.count_==0);
    assert(std::count(events.begin(),events.end(),"free")==4);
    assert(events.front()=="hide" && events.back()=="sound:close");
  }
  { Win owner;owner.list_.count_=9;owner.list_.tail_=7;owner.close();
    assert(owner.list_.count_==9 && owner.list_.tail_==0); }
  { Win owner;WinPointerOwner1=&owner;WinPointerOwner2=&owner;
    WinPointerOwner3=&owner;WinPointerOwner4=&owner;WinHoverWindow=&owner;
    WinBubbleCompanion=&owner;WinBubbleActive=1;events.clear();owner.close();
    assert(!WinPointerOwner1&&!WinPointerOwner2&&!WinPointerOwner3&&!WinPointerOwner4);
    assert(!WinHoverWindow&&!WinBubbleCompanion&&!WinBubbleActive);
    assert((events==std::vector<std::string>{"capture","screen","flip","hide","sound:close"}));
  }
  { Win parent;Win *a=new Win,*b=new Win;
    a->win_parent_=&parent;b->win_parent_=&parent;
    parent.children_[0]=a;parent.children_[1]=b;parent.child_count_=2;
    parent.close();
    // The original live-count forward walk skips shifted child b.
    assert(a->win_parent_==0 && b->win_parent_==&parent);
    delete a;delete b;
  }
  { events.clear(); Win *owner=new Win;owner->buffer1_=new Buffer;
    delete owner;
    assert((events==std::vector<std::string>{"buffer:new","hide","buffer:delete","sound:close","sound:close"}));
  }
}
static void init_controls() {
  { Win w;reset_observations();init_ok(w,0x10);
    assert(rectangle(w.client_rect_,30,40,130,120));
    assert(rectangle(w.outer_rect_,8,24,92,72));
    assert(rectangle(w.cached_outer_rect_,8,24,92,72));
    assert(w.field_16C_==24);w.close(); }
  { Win w;reset_observations();init_ok(w,0x810);
    assert(rectangle(w.client_rect_,22,16,138,128));
    assert(rectangle(w.outer_rect_,8,24,108,104));w.close(); }
  { Win w;reset_observations();init_ok(w,0);assert(w.field_16C_==8);w.close(); }
  { Win w;reset_observations();init_ok(w,0x20);assert(w.field_16C_==0);w.close(); }
  { Win parent,child;parent.outer_rect_={10,20,310,220};
    reset_observations();active=&child;
    assert(child.init(-5,-7,100,80,0,0,&parent,0,0)==0);
    assert(rectangle(child.outer_rect_,195,113,295,193));child.close(); }
  { Win parent,child;
    // A full parent would cause an unchecked array write. Do not execute that
    // unsafe path. A host Buffer constructor changes THIS count instead, using
    // valid detached receivers so the subsequent original close remains safe.
    for(auto &slot:poison_slots) slot=new Win;
    parent.child_count_=1;parent.children_[0]=&parent;
    reset_observations();active=&child;buffer_hook=poison_after_close;
    try { child.init(0,0,20,20,0,0,&parent,0,0); assert(false); }
    catch(int error) { assert(error==4); }
    assert(parent.child_count_==1);child.child_count_=0;parent.child_count_=0;
    for(auto slot:poison_slots) delete slot;
  }
  { Win w;reset_observations();fail_buffer=2;active=&w;
    assert(w.init(0,0,20,20,0,0,0,0,0)==4);
    assert(w.buffer1_!=0 && !w.buffer2_ && !(w.iSomeFlag_&4));w.close(); }
  { Win w;reset_observations();throw_buffer=2;active=&w;
    try { w.init(0,0,20,20,0,0,0,0,0);assert(false); }
    catch(int error) { assert(error==8); }
    assert(w.buffer1_!=0 && !w.buffer2_);w.close(); }
  { Win w;reset_observations();active=&w;
    assert(w.init(0,0,20,20,0,0x10000000,0,0,0)==3);w.close(); }
  { PaintProbe w;char caption[]="abc";reset_observations();fail_size=4;
    init_ok(w,0x10000000,0,0,caption);
    assert(!w.field_E0_ && paint_calls==0 && rect_calls==1);w.close(); }
  { PaintProbe w;char caption[]="abc";reset_observations();
    init_ok(w,0x10000000,0,0,caption);
    assert(std::string(w.field_E0_)=="abc" && paint_calls==1 && rect_calls==1);
    assert(std::find(events.begin(),events.end(),"paint")<std::find(events.begin(),events.end(),"rects"));w.close(); }
  { Win w;Menu menu;reset_observations();menu_result=17;active=&w;
    assert(w.init(0,0,20,20,0,0,0,&menu,0)==17);
    assert(w.iFlags_==0 && !w.buffer1_ && !(w.iSomeFlag_&4)); }
  { Win child;reset_observations();for(auto &root:WinRootWindows) root=0;
    WinRootCount=40;active=&child;
    try { child.init(0,0,20,20,0,0,0,0,0);assert(false); }
    catch(int error) { assert(error==4); }
    assert(WinRootCount==40 && child.iFlags_==0 && !child.buffer1_);WinRootCount=0; }
  for(int flags: {0,0x100000,0x300000}) {
    Win parent,child;reset_observations();init_ok(child,flags,&parent);
    assert(parent.list_.count_==(flags==0x100000?1:0));
    if(flags==0x100000) {
      auto n=parent.list_.head_;assert(n->unknown_00_==0xA5A5A5A5u);
      for(auto byte:n->unknown_14_) assert(byte==0xA5);
      for(int i=0;i<4;++i) assert(static_cast<unsigned char *>(n->allocation_)[i]==0xA5);
    }
    child.close();
  }
  { Win parent,child;Menu menu;reset_observations();menu_hook=block_registration;
    init_ok(child,0x100000,&parent,&menu);assert(parent.list_.count_==0);child.close(); }
  { Win parent,child;parent.iFlags_=0x1000;reset_observations();
    init_ok(child,0x100000,&parent);assert(parent.list_.count_==0);child.close(); }
  { Win child;reset_observations();init_ok(child,0x101000);
    assert(child.win_parent_==0);child.close(); }
  for(bool nonempty: {false,true}) for(int failure: {0x1c,4}) {
    Win parent,child;WinDialogNode *head=0;
    reset_observations();
    if(nonempty) {head=node(&parent);parent.list_.head_=parent.list_.current_=head;parent.list_.count_=1;}
    fail_size=failure;init_ok(child,0x100000,&parent);
    assert(child.win_parent_==&parent && parent.list_.count_==(nonempty?1:0));
    if(nonempty) {
      assert(parent.list_.current_==head && parent.list_.tail_==0);
      if(failure==0x1c) assert(head->next_==0);
      else assert(head->previous_->allocation_==0 && head->previous_->focus_owner_==&child);
    } else {
      assert(parent.list_.current_==0);
      if(failure==0x1c) assert(parent.list_.head_==0);
      else assert(parent.list_.head_->allocation_==0 && parent.list_.head_->focus_owner_==&child);
    }
    // Retain and inspect corrupt links, then detach only in the host cleanup.
    child.win_parent_=0;abandon_ring(parent);child.close();
  }
  for(int mode: {1,2,3}) {
    Win parent,child,other;Menu menu;
    reset_observations();allocation_parent=&parent;allocation_child=&child;
    mutated_parent=&other;parent.list_.allocator_=&heap_a;mutation=mode;
    if(mode==3) {
      auto head=node(&parent);parent.list_.head_=parent.list_.current_=head;parent.list_.count_=1;
    }
    replacement=node(&other);original_node=0;
    allocation_hook=mutate_allocation;menu_hook=change_parent;
    init_ok(child,0x100000,&parent,&menu);
    assert(child.win_parent_==&parent && other.list_.count_==0);
    if(mode==1) assert(heap_calls==2 && parent.list_.allocator_==&heap_b);
    if(mode==2) {
      assert(parent.list_.head_==replacement && parent.list_.current_==replacement);
      assert(replacement->allocation_!=0 && original_node->allocation_==reinterpret_cast<void*>(~uintptr_t(0)/255*165));
    }
    if(mode==3) {
      assert(parent.list_.count_==2 && parent.list_.current_==parent.list_.head_);
      assert(replacement->allocation_!=0 && original_node->allocation_==reinterpret_cast<void*>(~uintptr_t(0)/255*165));
    }
    allocation_hook=0;child.win_parent_=0;abandon_ring(parent);child.close();
  }
  { Win w;reset_observations();init_ok(w,0x0c);
    assert(std::count(events.begin(),events.end(),"scroll:horz")==1);
    assert(std::count(events.begin(),events.end(),"scroll:vert")==1);w.close(); }
}
int main() {
  reset_observations();close_controls();init_controls();scroll_controls();
  assert(allocations.empty() && buffer_live==0 && WinRootCount==0 && scroll_live_allocations==0);
}
'''


def program(damage=None):
    source = (ROOT / "src/win.cpp").read_text()
    header = (ROOT / "src/win.h").read_text()
    auto = definition((ROOT / "src/autosound.h").read_text(), "class AutoSound {") + ";"
    storage = header[header.index("  uint32_t iFlags_;"):header.index("\n public:", header.index("  uint32_t iFlags_;"))]
    callbacks = "\n".join(re.findall(r"^typedef .*;$", header, re.M))
    border = definition(header, "struct BorderSizing {") + ";"
    defaults = definition(header, "struct WinDynamicDefaultsTable {") + ";"
    init, close = definition(source, INIT), definition(source, CLOSE)
    changes = {
        "parent-limit": ("this->child_count_ == 150", "parent->child_count_ == 150"),
        "client-origin": ("int dx = -this->client_rect_.left;", "int dx = -this->outer_rect_.left;"),
        "wrong-rectangle": ("this->client_to_nonclient(&this->client_rect_);", "this->client_to_nonclient(&this->outer_rect_);"),
        "client-y-origin": ("            int dy = -this->client_rect_.top;", "            int dy = -this->outer_rect_.top;"),
        "caption-failure": ("goto init_set_rects;", "/* lost paint bypass */"),
        "border-polarity": ("else if ((this->iFlags_ & 0x20) == 0)", "else if ((this->iFlags_ & 0x20) != 0)"),
        "list-polarity": ("(fl & 0x100000) == 0", "(fl & 0x100000) != 0"),
        "null-skip": ("(fl & 0x1000) != 0", "(fl & 0x1000) == 0"),
        "live-parent": ("Win *parent_window = parent;", "Win *parent_window = this->win_parent_;"),
        "node-failure": ("goto init_registration_tail;", "return 0;"),
        "saved-parent": ("win_parent_ = parent_window;", "/* lost saved parent */"),
        "head-reload": ("parent_window->list_.head_->allocation_ = payload;", "newNode->allocation_ = payload;"),
        "current-reload": ("parent_window->list_.current_->allocation_ = payload;", "node->allocation_ = payload;"),
        "horizontal-placement": ("this->scroll_horz_ = new Scroll();", "void *raw = Scroll::operator new(sizeof(Scroll));\n            this->scroll_horz_ = raw ? ::new(raw) Scroll() : 0;"),
        "vertical-placement": ("this->scroll_vert_ = new Scroll();", "void *raw = Scroll::operator new(sizeof(Scroll));\n            this->scroll_vert_ = raw ? ::new(raw) Scroll() : 0;"),
        "horizontal-stale-show": ("this->scroll_horz_->show(0);", "sc->show(0);"),
        "vertical-stale-show": ("this->scroll_vert_->show(0);", "sc->show(0);"),
        "horizontal-direct-show": ("this->scroll_horz_->show(0);", "this->scroll_horz_->Win::show(0);"),
        "vertical-direct-show": ("this->scroll_vert_->show(0);", "this->scroll_vert_->Win::show(0);"),
        "close-index": ("parent->list_.tail_ = parent->list_.count_ - 1;", "/* lost index */"),
        "close-palette": ("palette_seed_cache_ = 0;", "/* lost palette reset */"),
        "close-allocator": ("list_.allocator_ == 0 &&", ""),
        "close-cache": ("cached_outer_rect_.left = 0;", "/* lost rectangle reset */"),
    }
    if damage:
        old, new = changes[damage]
        if damage.startswith("close-"):
            assert old in close;close=close.replace(old,new,1)
        else:
            assert old in init;init=init.replace(old,new,1)
    # Only the free dependency is instrumented; all product method bodies are
    # otherwise verbatim. Windows/CRT/Buffer/Scroll/Menu/Heap are host doubles.
    close = close.replace("free(", "probe_free(")
    # Use current product method declarations too, retaining virtualness and
    # parameter types. Dependency hook bodies alone are replaced by probes.
    win_class = definition(header, "class Win : public AutoSound {")
    declarations = []
    patterns = (
        r"Win\(\)", r"~Win\(\)", r"void close\(\)",
        r"int init\(int x,.*?BorderSizing \* border\)",
        r"virtual void hide\(\)", r"virtual void show\(int visible\)",
        r"virtual void on_lose_mouse_capture\(\)",
        r"virtual Scroll \*on_scroll_create\(\)",
        r"virtual Scroll \*on_scroll_delete\(Scroll \*scroll\)",
        r"virtual GraphicWin \*get_graphic_win\(\)",
        r"virtual void set_rects\(\)",
        r"virtual void on_nc_paint\(RECT \*area, int flags\)",
        r"void nonclient_to_client\(RECT \*rect\)",
        r"void client_to_nonclient\(RECT \* rect\)",
        r"int is_visible\(\)", r"int is_descendant\(Win \*candidate\)",
        r"void do_caption_buttons\(\)",
    )
    for pattern in patterns:
        found = re.findall(r"^\s*(" + pattern + r")\s*[;{]", win_class, re.M | re.S)
        assert len(found) == 1, pattern
        declarations.append(found[0] + ";")
    members = r'''
class Win : public AutoSound {
 public:
STORAGE
DECLARATIONS
};
Scroll *Win::on_scroll_create() { return 0; }
Scroll *Win::on_scroll_delete(Scroll *scroll) { return scroll; }
'''.replace("STORAGE", storage).replace("DECLARATIONS", "\n".join(declarations))
    result = DEPENDENCIES + auto + callbacks + border + defaults + members + SUPPORT
    for signature in ("Win::Win()", "Win::~Win()", "void Win::nonclient_to_client(RECT * rect)",
                      "void Win::client_to_nonclient(RECT * rect)"):
        result += "\n" + definition(source, signature)
    return result + "\n" + init + "\n" + close + CASES


def compile_run(tmp_path, compiler, damage=None):
    code = tmp_path / "control.cpp"
    binary = tmp_path / "control"
    code.write_text(program(damage))
    built = subprocess.run([compiler,"-std=c++17","-fcheck-new","-iquote",str(ROOT / "src"),
                            str(code),"-o",str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    return subprocess.run([str(binary)],capture_output=True,timeout=15,preexec_fn=no_core)


@pytest.mark.parametrize("compiler_name",["g++","clang++"])
def test_complete_actual_methods(tmp_path,compiler_name):
    compiler=shutil.which(compiler_name)
    if not compiler: pytest.skip("host compiler unavailable")
    ran=compile_run(tmp_path,compiler)
    assert ran.returncode==0,ran.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage",[
    "parent-limit","client-origin","wrong-rectangle","client-y-origin","caption-failure","border-polarity",
    "list-polarity","null-skip","live-parent","node-failure","saved-parent",
    "head-reload","current-reload","close-index","close-palette","close-allocator","close-cache",
    "horizontal-placement","vertical-placement","horizontal-stale-show","vertical-stale-show",
    "horizontal-direct-show","vertical-direct-show",
])
def test_controls_reject_executed_damage(tmp_path,damage):
    compiler=shutil.which("g++")
    if not compiler: pytest.skip("host compiler unavailable")
    ran=compile_run(tmp_path,compiler,damage)
    assert ran.returncode!=0,"damage escaped: "+damage
