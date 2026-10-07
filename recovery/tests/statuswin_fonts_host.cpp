#include <assert.h>
#include <stdint.h>

struct RECT { int left, top, right, bottom; };
static int events[32];
static int event_count;
static int union_calls;
static int union_inputs[2][8];
static int WinScreenWidth;
static const char *DefaultFontFace;
static const char face_a[] = "face-a";
static const char face_b[] = "face-b";
static int mutate_width_after_first_init;
static int mutate_face_after_first_init;

struct Font {
    int unk_1_;
    int slot;
    int calls;
    int last_height;
    int last_style;
    const char *last_face;
    Font() : unk_1_(0), slot(0), calls(0), last_height(0), last_style(-1), last_face(0) {}
    int init(const char *face, int height, int style) {
        ++calls; last_height=height; last_style=style; last_face=face;
        events[event_count++]=10+slot;
        if (mutate_width_after_first_init && slot==0) WinScreenWidth=1024;
        if (mutate_face_after_first_init && slot==0) DefaultFontFace=face_b;
        return 0;
    }
};

struct MainInterface {
    RECT hit, right, left, last_update;
    int update_calls;
    MainInterface() : update_calls(0) {}
    const RECT &status_hit_rect() const { return hit; }
    const RECT &status_right_rect() const { return right; }
    const RECT &status_left_rect() const { return left; }
    void soft_update(const RECT *rect) {
        events[event_count++]=50; ++update_calls; last_update=*rect;
    }
} MainInterfaceGlobal;

int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    const int call=union_calls++;
    events[event_count++]=(call == 0) ? 40 : 41;
    union_inputs[call][0]=a->left; union_inputs[call][1]=a->top;
    union_inputs[call][2]=a->right; union_inputs[call][3]=a->bottom;
    union_inputs[call][4]=b->left; union_inputs[call][5]=b->top;
    union_inputs[call][6]=b->right; union_inputs[call][7]=b->bottom;
    out->left=a->left < b->left ? a->left : b->left;
    out->top=a->top < b->top ? a->top : b->top;
    out->right=a->right > b->right ? a->right : b->right;
    out->bottom=a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

struct StatusWin {
    Font font1_, font2_, font3_;
    int32_t font_size_;
    int draw_calls;
    int draw_args[5];
    StatusWin() : font_size_(-1), draw_calls(0) {
        font1_.slot=0; font2_.slot=1; font3_.slot=2;
    }
    void draw_status(int a, int b, int c, int d, int e) {
        events[event_count++]=30; ++draw_calls;
        draw_args[0]=a; draw_args[1]=b; draw_args[2]=c; draw_args[3]=d; draw_args[4]=e;
    }
    void UNK1();
    void on_redraw();
};

/* PRODUCT_UNK1 */
/* PRODUCT_ON_REDRAW */

static void reset_events() { event_count=0; union_calls=0; }

int main() {
    StatusWin status;
    WinScreenWidth=800; DefaultFontFace=face_a;
    mutate_width_after_first_init=1; mutate_face_after_first_init=1;
    status.UNK1();
    assert(status.font_size_==10);
    assert(status.font1_.calls==1 && status.font2_.calls==1 && status.font3_.calls==1);
    assert(status.font1_.last_height==10 && status.font2_.last_height==10 && status.font3_.last_height==10);
    assert(status.font1_.last_style==0 && status.font2_.last_style==2 && status.font3_.last_style==1);
    assert(status.font1_.last_face==face_a && status.font2_.last_face==face_b && status.font3_.last_face==face_b);
    assert(status.font1_.unk_1_==4 && status.font2_.unk_1_==4 && status.font3_.unk_1_==4);
    assert(event_count==3 && events[0]==10 && events[1]==11 && events[2]==12);

    reset_events();
    mutate_width_after_first_init=0; mutate_face_after_first_init=0;
    WinScreenWidth=1024; DefaultFontFace=face_a; status.font_size_=-1;
    MainInterfaceGlobal.hit.left=0; MainInterfaceGlobal.hit.top=0;
    MainInterfaceGlobal.hit.right=10; MainInterfaceGlobal.hit.bottom=10;
    MainInterfaceGlobal.right.left=10; MainInterfaceGlobal.right.top=-2;
    MainInterfaceGlobal.right.right=20; MainInterfaceGlobal.right.bottom=12;
    MainInterfaceGlobal.left.left=-5; MainInterfaceGlobal.left.top=1;
    MainInterfaceGlobal.left.right=2; MainInterfaceGlobal.left.bottom=9;
    status.on_redraw();
    assert(status.font_size_==12 && status.draw_calls==1);
    assert(status.draw_args[0]==-1 && status.draw_args[1]==-1 && status.draw_args[2]==-1 &&
           status.draw_args[3]==-1 && status.draw_args[4]==0);
    assert(event_count==7 && events[0]==10 && events[1]==11 && events[2]==12 &&
           events[3]==30 && events[4]==40 && events[5]==41 && events[6]==50);
    assert(union_inputs[0][0]==0 && union_inputs[0][2]==10 &&
           union_inputs[0][4]==10 && union_inputs[0][5]==-2);
    assert(union_inputs[1][0]==0 && union_inputs[1][1]==-2 &&
           union_inputs[1][4]==-5 && union_inputs[1][5]==1);
    assert(MainInterfaceGlobal.last_update.left==-5 && MainInterfaceGlobal.last_update.top==-2 &&
           MainInterfaceGlobal.last_update.right==20 && MainInterfaceGlobal.last_update.bottom==12);

    // Cached height suppresses all Font::init calls, but the redraw and both
    // ordered rectangle unions still execute.
    reset_events();
    WinScreenWidth=800; status.font_size_=10;
    status.font1_.unk_1_=1; status.font2_.unk_1_=2; status.font3_.unk_1_=3;
    status.on_redraw();
    assert(status.font1_.calls==2 && status.font2_.calls==2 && status.font3_.calls==2);
    assert(status.font1_.unk_1_==1 && status.font2_.unk_1_==2 && status.font3_.unk_1_==3);
    assert(event_count==4 && events[0]==30 && events[1]==40 && events[2]==41 && events[3]==50);
}
