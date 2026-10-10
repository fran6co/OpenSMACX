from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_function(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        ch = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        else:
            if ch == "/" and nxt == "/":
                state = "line_comment"
                index += 1
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                index += 1
            elif ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated C++ function {signature}")


def actual_axis_body():
    source = (ROOT / "src/win.cpp").read_text()
    init = extract_function(source,
                            "int Win::init(int x, int y, int width, int height, char * caption,")
    start = init.index("    if ((this->iFlags_ & 4) != 0) {")
    end = init.index("    this->do_caption_buttons();", start)
    body = init[start:end]
    return body.replace("operator new(0x214C)", "test_scroll_alloc(0x214C)")


def actual_close_scroll_body():
    source = (ROOT / "src/win.cpp").read_text()
    close = extract_function(source, "void Win::close()")
    start = close.index("    if ((scroll_vert_ != 0) && ((iFlags_ & 0x40) != 0)) {")
    end = close.index("    if ((iFlags_ & 0x100000) != 0) {", start)
    return close[start:end]


def test_win_scroll_types_and_buffer_fields_are_distinct():
    win_h = (ROOT / "src/win.h").read_text()
    win_cpp = (ROOT / "src/win.cpp").read_text()
    list_h = (ROOT / "src/listbox.h").read_text()
    dialogs_h = (ROOT / "src/dialogs.h").read_text()
    assert "Buffer *buffer3_;" in win_h and "Buffer *buffer4_;" in win_h
    assert "Scroll *scroll_vert_;" in win_h and "Scroll *scroll_horz_;" in win_h
    assert "virtual Scroll *on_scroll_create()" in win_h
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll)" in win_h
    assert "virtual Scroll *on_listbox_scroll_create()" in win_h
    assert "virtual Scroll *on_listbox_scroll_delete(Scroll *value = 0)" in win_h
    assert "virtual Scroll *on_scroll_create();" in list_h
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll);" in list_h
    assert "virtual Scroll *on_scroll_create();" in dialogs_h
    assert "virtual Scroll *on_scroll_delete(Scroll *scroll);" in dialogs_h
    assert "this->buffer3_ = reinterpret_cast<Buffer *>(this->on_scroll_create())" not in win_cpp
    assert "this->buffer4_ = reinterpret_cast<Buffer *>(this->on_scroll_create())" not in win_cpp


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_init_axis_branches_and_close_lifecycle(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host compiler unavailable")
    axis_body = actual_axis_body()
    close_body = actual_close_scroll_body()
    host = r'''#include <cassert>
#include <cstdlib>
#include <new>

static int ScrollDefaultThickness = 7;
static int allocation_calls;
static int fail_allocation_call;
static void *allocated[8];
static int allocated_count;
static int destruction_order[16];
static int destruction_count;
static int next_scroll_id;

static void *test_scroll_alloc(unsigned int size) {
    ++allocation_calls;
    if (allocation_calls == fail_allocation_call) return 0;
    void *value = std::malloc(size);
    allocated[allocated_count++] = value;
    return value;
}

struct RECT { int left, top, right, bottom; };
struct Scroll;
struct Buffer {};

struct Win {
    unsigned int iFlags_;
    Scroll *scroll_vert_;
    Scroll *scroll_horz_;
    Buffer *buffer1_;
    Buffer *buffer2_;
    Buffer *buffer3_;
    Buffer *buffer4_;
    RECT outer_rect_;
    int create_calls;
    int delete_calls;
    Scroll *create_results[2];
    Scroll *delete_results[2];
    Scroll *delete_arguments[2];
    Win() : iFlags_(0), scroll_vert_(0), scroll_horz_(0), buffer1_(0),
        buffer2_(0), buffer3_(0), buffer4_(0), outer_rect_(), create_calls(0),
        delete_calls(0) {
        create_results[0] = create_results[1] = 0;
        delete_results[0] = delete_results[1] = 0;
        delete_arguments[0] = delete_arguments[1] = 0;
    }
    virtual ~Win() {}
    virtual Scroll *on_scroll_create() {
        int index = create_calls++;
        assert(index < 2);
        return create_results[index];
    }
    virtual Scroll *on_scroll_delete(Scroll *value) {
        int index = delete_calls++;
        assert(index < 2);
        delete_arguments[index] = value;
        return delete_results[index];
    }
    int init_scrolls();
    void close_scrolls();
    void hide() {}
};

struct Scroll {
    int id;
    bool track_destroy;
    int horizontal_calls;
    int vertical_calls;
    int show_calls;
    int x, y, length, setting;
    Win *parent;
    Scroll() : id(++next_scroll_id), track_destroy(false), horizontal_calls(0), vertical_calls(0),
        show_calls(0), x(0), y(0), length(0), setting(0), parent(0) {}
    ~Scroll() {
        if (track_destroy) destruction_order[destruction_count++] = id;
    }
    int init_horz_nc(int a, int b, int c, Win *owner, int d) {
        ++horizontal_calls; x=a; y=b; length=c; parent=owner; setting=d;
        return 71;
    }
    int init_vert_nc(int a, int b, int c, Win *owner, int d) {
        ++vertical_calls; x=a; y=b; length=c; parent=owner; setting=d;
        return 72;
    }
    int show(int) { ++show_calls; return 73; }
};

int Win::init_scrolls() {
''' + axis_body + r'''
    return 0;
}

void Win::close_scrolls() {
    this->hide();
''' + close_body + r'''
}

static void release_allocations() {
    for (int i = 0; i < allocated_count; ++i) std::free(allocated[i]);
    allocated_count = 0;
    allocation_calls = 0;
    fail_allocation_call = 0;
    destruction_count = 0;
}

static void dimensions(Scroll *scroll, int h, int v, Win *owner,
                       int hx, int hy, int hlen, int vx, int vy, int vlen) {
    if (h) {
        assert(scroll->horizontal_calls == 1);
        assert(scroll->vertical_calls == 0);
        assert(scroll->x == hx && scroll->y == hy && scroll->length == hlen);
        assert(scroll->setting == -2 && scroll->parent == owner);
    }
    if (v) {
        assert(scroll->vertical_calls == 1);
        assert(scroll->horizontal_calls == 0);
        assert(scroll->x == vx && scroll->y == vy && scroll->length == vlen);
        assert(scroll->setting == -1 && scroll->parent == owner);
    }
    assert(scroll->show_calls == 1);
}

int main() {
    Buffer b1,b2,b3,b4;
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120};
        w.buffer1_=&b1; w.buffer2_=&b2; w.buffer3_=&b3; w.buffer4_=&b4;
        w.iFlags_=4;
        assert(w.init_scrolls()==0);
        assert(w.create_calls==0 && allocation_calls==1);
        assert(w.scroll_horz_ && !w.scroll_vert_);
        dimensions(w.scroll_horz_,1,0,&w,0,113,200,0,0,0);
        assert(w.buffer1_==&b1 && w.buffer2_==&b2 &&
               w.buffer3_==&b3 && w.buffer4_==&b4);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120};
        w.buffer3_=&b3; w.buffer4_=&b4; w.iFlags_=8;
        assert(w.init_scrolls()==0);
        assert(w.create_calls==0 && allocation_calls==1);
        assert(!w.scroll_horz_ && w.scroll_vert_);
        dimensions(w.scroll_vert_,0,1,&w,0,0,0,203,0,100);
        assert(w.buffer3_==&b3 && w.buffer4_==&b4);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=12;
        assert(w.init_scrolls()==0);
        assert(allocation_calls==2 && w.scroll_horz_ && w.scroll_vert_);
        dimensions(w.scroll_horz_,1,0,&w,0,113,193,0,0,0);
        dimensions(w.scroll_vert_,0,1,&w,0,0,0,203,0,93);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=0x4c;
        Scroll horizontal, vertical;
        w.create_results[0]=&horizontal; w.create_results[1]=&vertical;
        assert(w.init_scrolls()==0);
        assert(w.create_calls==2 && allocation_calls==0);
        assert(w.scroll_horz_==&horizontal && w.scroll_vert_==&vertical);
        assert(horizontal.horizontal_calls==0 && horizontal.vertical_calls==0 &&
               horizontal.show_calls==0);
        assert(vertical.horizontal_calls==0 && vertical.vertical_calls==0 &&
               vertical.show_calls==0);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=0x4c;
        Scroll vertical;
        w.create_results[0]=0; w.create_results[1]=&vertical;
        assert(w.init_scrolls()==0);
        assert(w.create_calls==2 && allocation_calls==1);
        assert(w.scroll_horz_ && w.scroll_vert_==&vertical);
        dimensions(w.scroll_horz_,1,0,&w,0,113,193,0,0,0);
        assert(vertical.horizontal_calls==0 && vertical.vertical_calls==0 &&
               vertical.show_calls==0);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=0x4c;
        Scroll horizontal;
        w.create_results[0]=&horizontal; w.create_results[1]=0;
        assert(w.init_scrolls()==0);
        assert(w.create_calls==2 && allocation_calls==1);
        assert(w.scroll_horz_==&horizontal && w.scroll_vert_);
        assert(horizontal.horizontal_calls==0 && horizontal.vertical_calls==0 &&
               horizontal.show_calls==0);
        dimensions(w.scroll_vert_,0,1,&w,0,0,0,203,0,93);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=4;
        Scroll existing;
        w.scroll_horz_=&existing; // without bit 0x40 the source reads the member
        assert(w.init_scrolls()==0);
        assert(w.create_calls==0 && allocation_calls==0);
        assert(w.scroll_horz_==&existing && existing.show_calls==0);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=12;
        fail_allocation_call=1;
        assert(w.init_scrolls()==4);
        assert(allocation_calls==1 && w.scroll_horz_==0 && w.scroll_vert_==0);
        release_allocations();
    }
    {
        Win w; w.outer_rect_ = RECT{10,20,210,120}; w.iFlags_=12;
        fail_allocation_call=2;
        assert(w.init_scrolls()==4);
        assert(allocation_calls==2 && w.scroll_horz_ && w.scroll_vert_==0);
        assert(w.scroll_horz_->horizontal_calls==1 && w.scroll_horz_->show_calls==1);
        release_allocations();
    }
    {
        Win w; w.iFlags_=0x40;
        Scroll vertical, horizontal;
        Scroll *replacement_v=new Scroll(); replacement_v->track_destroy=true;
        Scroll *replacement_h=new Scroll(); replacement_h->track_destroy=true;
        int replacement_v_id=replacement_v->id;
        int replacement_h_id=replacement_h->id;
        w.scroll_vert_=&vertical; w.scroll_horz_=&horizontal;
        w.delete_results[0]=replacement_v; w.delete_results[1]=replacement_h;
        w.close_scrolls();
        assert(w.delete_calls==2);
        assert(w.delete_arguments[0]==&vertical && w.delete_arguments[1]==&horizontal);
        assert(w.scroll_vert_==0 && w.scroll_horz_==0);
        assert(destruction_count==2);
        assert(destruction_order[0]==replacement_v_id &&
               destruction_order[1]==replacement_h_id);
        release_allocations();
    }
    {
        Win w; w.iFlags_=0x40;
        Scroll vertical, horizontal;
        w.scroll_vert_=&vertical; w.scroll_horz_=&horizontal;
        w.delete_results[0]=0; w.delete_results[1]=0;
        w.close_scrolls();
        assert(w.delete_calls==2 && w.delete_arguments[0]==&vertical &&
               w.delete_arguments[1]==&horizontal);
        assert(w.scroll_vert_==0 && w.scroll_horz_==0 && destruction_count==0);
        release_allocations();
    }
    {
        Win w; w.iFlags_=0;
        Scroll vertical, horizontal;
        w.scroll_vert_=new Scroll(); w.scroll_horz_=new Scroll();
        w.scroll_vert_->track_destroy=true; w.scroll_horz_->track_destroy=true;
        int vert_id=w.scroll_vert_->id, horz_id=w.scroll_horz_->id;
        w.close_scrolls();
        assert(w.delete_calls==0 && w.scroll_vert_==0 && w.scroll_horz_==0);
        assert(destruction_count==2 && destruction_order[0]==vert_id &&
               destruction_order[1]==horz_id);
        release_allocations();
    }
    {
        Win w; w.iFlags_=0x40;
        Scroll *vertical=new Scroll(); vertical->track_destroy=true;
        Scroll *horizontal=new Scroll(); horizontal->track_destroy=true;
        int vertical_id=vertical->id, horizontal_id=horizontal->id;
        w.scroll_vert_=vertical; w.scroll_horz_=horizontal;
        w.delete_results[0]=vertical; w.delete_results[1]=horizontal;
        w.close_scrolls();
        assert(w.delete_calls==2);
        assert(w.delete_arguments[0]==vertical && w.delete_arguments[1]==horizontal);
        assert(w.scroll_vert_==0 && w.scroll_horz_==0);
        assert(destruction_count==2 && destruction_order[0]==vertical_id &&
               destruction_order[1]==horizontal_id);
        w.close_scrolls();
        assert(w.delete_calls==2 && destruction_count==2);
        release_allocations();
    }
    return 0;
}
'''
    source = tmp_path / "win_scroll_lifecycle.cpp"
    source.write_text(host)
    binary = tmp_path / "win_scroll_lifecycle"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


def test_independent_vertical_branch_and_slot_pairs_are_source_anchored():
    body = actual_axis_body()
    horizontal_test = body.index("if ((this->iFlags_ & 4) != 0)")
    vertical_test = body.index("if ((this->iFlags_ & 8) != 0)")
    assert horizontal_test < vertical_test
    assert "this->scroll_horz_ = this->on_scroll_create();" in body
    assert "this->scroll_vert_ = this->on_scroll_create();" in body
    assert "this->buffer3_" not in body and "this->buffer4_" not in body
    assert "if ((this->iFlags_ & 4) != 0) {\n        if ((this->iFlags_ & 0x40)" in body
    assert "    }\n\n    if ((this->iFlags_ & 8) != 0) {" in body
    close = actual_close_scroll_body()
    assert close.index("on_scroll_delete(scroll_vert_)") < close.index("on_scroll_delete(scroll_horz_)")
