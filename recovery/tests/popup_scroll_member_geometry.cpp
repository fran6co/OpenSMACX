#include <assert.h>
#include <stdint.h>
#define __cdecl
struct RECT { int left, top, right, bottom; };
struct Dib { struct Header { int biWidth; } bmiHeader; };
class Sprite {
    int value_;
public:
    Sprite():value_(1) {}
    void set_height(int value) { value_=value; }
    int height() const { return value_; }
};
class MainInterface {
    Sprite sprites1_[4];
public:
    // ACTUAL_MAIN_ACCESSOR
} MainInterfaceGlobal;
class ListBox {
    unsigned int field_1C_;
public:
    explicit ListBox(unsigned int count):field_1C_(count) {}
    // ACTUAL_CAPACITY_ACCESSOR
};
class Buffer {
protected:
    RECT rect2_;
    Dib dib_;
};
struct Log { int normal,init,hide,show,left,top,width,height; } log_state;
class Scroll {
public:
    void init(int left,int y,int width,int height,void*,int id,int flags) {
        assert(y==21 && id==-1 && flags==1);
        ++log_state.init;log_state.left=left;log_state.width=width;log_state.height=height;
    }
    void hide() { ++log_state.hide; }
};
class Popup:public Buffer {
    unsigned int field_30AC_;
    Scroll scroll_;
    bool visible_;
public:
    Popup(unsigned int mode,bool visible):field_30AC_(mode),visible_(visible) {
        rect2_.left=5;rect2_.top=10;rect2_.right=305;rect2_.bottom=200;
        dib_.bmiHeader.biWidth=320;
    }
    void scroll_normal() { ++log_state.normal; }
    bool is_visible() { return visible_; }
    int on_listbox_scroll_create();
};
void scroll_show(Popup*,Scroll*) { ++log_state.show; }
// ACTUAL_SOURCE_BODY
int main() {
    ListBox list(37);assert(list.visible_item_capacity()==37);
    MainInterfaceGlobal.scroll_frame_sprite(0).set_height(13);
    MainInterfaceGlobal.scroll_frame_sprite(1).set_height(23);
    MainInterfaceGlobal.scroll_frame_sprite(2).set_height(17);
    MainInterfaceGlobal.scroll_frame_sprite(3).set_height(71);
    Popup quiet(0,false);quiet.on_listbox_scroll_create();
    assert(log_state.normal==1 && log_state.init==1 && log_state.hide==1 && log_state.show==0);
    assert(log_state.left==277 && log_state.width==24 && log_state.height==112);
    MainInterfaceGlobal.scroll_frame_sprite(1).set_height(29);
    Popup visible(0,true);visible.on_listbox_scroll_create();
    assert(log_state.init==2 && log_state.hide==2 && log_state.show==1);
    assert(log_state.height==107);
    Popup bypass(10000000,true);assert(bypass.on_listbox_scroll_create()==0);
    assert(log_state.init==2 && log_state.normal==2 && log_state.hide==2 && log_state.show==1);
    return 0;
}
