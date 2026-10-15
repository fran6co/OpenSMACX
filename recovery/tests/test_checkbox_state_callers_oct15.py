"""Compile the actual Checkbox event handlers with typed host doubles."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/checkbox.cpp"

def extract(source, signature):
    start = source.find(signature)
    assert start >= 0, signature
    opening = source.find("{", start)
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{": depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0: return source[start:i + 1]
    raise AssertionError(signature)

def test_actual_checkbox_event_handlers(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler: pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    # The member bodies are extracted verbatim; the small list/window doubles
    # below provide only the pre-existing Dialog/StringStruct service APIs.
    redraw = extract(source, "void CheckBox::on_redraw()")
    down = extract(source, "void CheckBox::on_left_down(int x, int y)")
    prefix = r'''#include <cassert>
#include <cstdint>
struct Entry { int id; Entry *next; };
struct StringStruct {
 Entry *head_, *current_; int entry_count_, current_position_;
 void seek_id(int id) { if (!head_) return; current_=head_; current_position_=0;
  while (current_ && current_->id != id) { current_=current_->next; ++current_position_; } }
};
struct Dib { struct { int biWidth,biHeight; } bmiHeader; };
struct Win { virtual int on_dialog_back_draw(void *) { return 1; } };
struct GraphicWin : Win { int updates=0; void copy(void*,int,int,int,int,int,int){} void soft_update(){++updates;} };
static int mutate_nth_id_to_pos;
struct Dialog : GraphicWin {
 StringStruct item_list_; uint32_t selected_position_;
 uint32_t field_24_,field_28_,field_44_,field_48_,field_64_,field_68_,field_4C_,field_58_,field_F0_;
 int pos_to_id(int p) { if(!item_list_.head_) return -1; item_list_.current_=item_list_.head_; item_list_.current_position_=0;
  while(item_list_.current_ && item_list_.current_position_<p) {item_list_.current_=item_list_.current_->next; ++item_list_.current_position_;}
  return item_list_.current_?item_list_.current_->id:-1; }
 int id_to_pos(int id) { if(!item_list_.head_) return item_list_.current_position_; item_list_.current_=item_list_.head_; item_list_.current_position_=0;
  while(item_list_.current_ && item_list_.current_->id!=id) {item_list_.current_=item_list_.current_->next; ++item_list_.current_position_;}
  int result=item_list_.current_?item_list_.current_position_:-1;
  if(mutate_nth_id_to_pos && --mutate_nth_id_to_pos==0) { item_list_.current_=item_list_.head_; item_list_.current_position_=0; selected_position_=0; }
  return result; }
};
static Win *WinCallbackWindow;
typedef void (*WinNotificationCallback)(int);
static int callback_order,callback_value;
static StringStruct *callback_list; static uint32_t *callback_state; static Entry *mutated_head;
static void key_callback(int v){callback_order=1;callback_value=v;
 if(callback_list){callback_list->head_=mutated_head;callback_list->current_=mutated_head;callback_list->entry_count_=3;callback_list->current_position_=0;*callback_state=1u<<2;callback_list=0;}}
static void wrong_callback(int){callback_order=2;}
struct CheckBox : Dialog {
 Win *win_parent_=0; Dib dib_{}; uint32_t field_4_=0;
 WinNotificationCallback key_notification_callback_=0,double_click_callback_=0;
 int draws[32]{},states[32]{},focus[32]{},draw_count=0;
 void draw_item(int p,int s,int f){draws[draw_count]=p;states[draw_count]=s;focus[draw_count++]=f;}
 void set_state_id(int id,int value){item_list_.seek_id(id);uint32_t m=1u<<(uint32_t(item_list_.current_position_)&31u);if(value)selected_position_|=m;else selected_position_&=~m;}
 void on_redraw(); void on_left_down(int,int);
};
'''
    main = r'''int main() {
 CheckBox b; Win parent; b.win_parent_=&parent;
 Entry e[4]={{10,&e[1]},{20,&e[2]},{20,&e[3]},{30,0}};
 b.item_list_={e,e,4,0}; b.selected_position_=(1u<<1)|(1u<<3);
 b.field_4_=3;b.field_F0_=1;b.on_redraw();
 assert(b.draw_count==4 && b.states[0]==0 && b.states[1]==2 && b.focus[3]==1 && !b.focus[1]);
 assert(b.item_list_.current_==e && b.item_list_.current_position_==0);
 b.item_list_={e,e,4,0};b.selected_position_=1u;b.field_4_=0;
 b.field_64_=8;b.field_44_=2;b.field_68_=8;b.field_28_=100;b.field_4C_=1;b.field_58_=4;
 b.key_notification_callback_=key_callback;b.double_click_callback_=wrong_callback;callback_list=0;
 callback_order=0;b.draw_count=0;b.on_left_down(0,10);
 assert((b.selected_position_&(1u<<1)) && callback_order==1);
 assert(b.draw_count==2&&b.draws[0]==0&&b.states[0]==1&&!b.focus[0]&&b.draws[1]==1&&b.focus[1]);
 assert(b.field_4_==1&&b.item_list_.current_position_==1);
 // Selected click clears via the second search and still calls back/draws.
 b.draw_count=0;callback_order=0;b.on_left_down(0,10);
 assert((b.selected_position_&(1u<<1))==0 && b.draw_count==1 && b.draws[0]==1 && b.states[0]==0);
 assert(b.field_4_==1 && callback_order==1);
 // Duplicate ID: original compares IDs, so no old-row redraw or focus move.
 e[3].id=20;b.item_list_={e,e,4,0};b.selected_position_=0;b.field_4_=2;b.key_notification_callback_=0;b.draw_count=0;
 b.on_left_down(0,30);
 assert(b.draw_count==1&&b.draws[0]==1&&b.field_4_==2);
 assert(b.updates==3);
 // The callback replaces the list and state. The handler re-finds the ID,
 // draws at its new position, and does not overwrite the pre-callback focus.
 e[3].id=30;Entry changed[3]={{7,&changed[1]},{30,&changed[2]},{20,0}};
 b.item_list_={e,e,4,0};b.selected_position_=0;b.field_4_=3;
 b.key_notification_callback_=key_callback;callback_list=&b.item_list_;
 callback_state=&b.selected_position_;mutated_head=changed;b.draw_count=0;mutate_nth_id_to_pos=2;
 b.on_left_down(0,10);
 assert(callback_order==1 && callback_value==2);
 assert(b.field_4_==1 && b.draw_count==2 && b.draws[1]==2 && b.states[1]==4);
 assert(b.selected_position_==0 && b.item_list_.current_==changed && b.item_list_.current_position_==0);
 assert(b.updates==4);
 // Empty-list edge: seek_id and pos_to_id leave the seeded cursor position;
 // a selected bit is cleared there, and id_to_pos returns that stale position.
 b.item_list_={0,0,0,5};b.selected_position_=1u<<5;b.field_4_=0;
 b.key_notification_callback_=0;b.draw_count=0;b.on_left_down(0,10);
 assert(b.selected_position_==0 && b.item_list_.current_position_==5);
 assert(b.draw_count==1 && b.draws[0]==5 && b.states[0]==0);
 assert(b.updates==5);
}
'''
    methods = redraw + "\n" + down
    cpp = tmp_path / "caller.cpp"; exe = tmp_path / "caller"
    cpp.write_text(prefix + methods + main)
    subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror","-Wno-unused-variable",str(cpp),"-o",str(exe)],check=True,capture_output=True,text=True,timeout=30)
    subprocess.run([str(exe)],check=True,capture_output=True,text=True,timeout=10)
    # Product-source mutants must fail the behavior controls above.
    mutants = [
        methods.replace("Dialog::field_68_ + Dialog::field_44_", "Dialog::field_48_ + Dialog::field_28_",1),
        methods.replace("key_notification_callback_", "double_click_callback_",1),
        methods.replace("Dialog::pos_to_id(previous) != id", "previous != static_cast<int>(Dialog::id_to_pos(id))",1),
        methods.replace("field_4_ == static_cast<uint32_t>(position)", "Dialog::field_F0_ == static_cast<uint32_t>(position)",1),
        methods.replace("const int state_after_callback =\n        static_cast<int>(Dialog::selected_position_ & current_mask);\n    const int position_after_callback = Dialog::id_to_pos(id);", "const int position_after_callback = Dialog::id_to_pos(id);\n    const int state_after_callback =\n        static_cast<int>(Dialog::selected_position_ & current_mask);",1),
    ]
    for n, mutant in enumerate(mutants):
        test_cpp=tmp_path/f"mutant{n}.cpp"; test_exe=tmp_path/f"mutant{n}"
        test_cpp.write_text(prefix+mutant+main)
        subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror","-Wno-unused-variable",str(test_cpp),"-o",str(test_exe)],check=True,capture_output=True,text=True,timeout=30)
        result=subprocess.run([str(test_exe)],capture_output=True,text=True,timeout=10)
        assert result.returncode != 0, f"mutant {n} escaped"
