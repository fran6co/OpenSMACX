// Disposable behavior control for the candidate, not an application/runtime proof.
#include <cassert>
#include <cstring>
#include <cctype>
#include <stdint.h>
#include <algorithm>
#include <vector>
typedef int LONG;
struct RECT { int left, top, right, bottom; };
static void UnionRect(RECT *out, const RECT *a, const RECT *b) {
    RECT result = {std::min(a->left,b->left),std::min(a->top,b->top),
                   std::max(a->right,b->right),std::max(a->bottom,b->bottom)};
    *out = result;
}
static std::vector<int> events;
struct Font {
    int unk_1_, height_, style_;
    Font():unk_1_(0),height_(0),style_(0) {}
    void init(const char *,int height,int style) {height_=height;style_=style;}
};
struct Style {};
static Style InterfaceBoxStyles[30];
static const char *DefaultFontFace = "control";
struct Buffer {
    Font *font_, *hyper_;
    int colors_[3];
    Buffer():font_(0),hyper_(0) {colors_[0]=colors_[1]=colors_[2]=0;}
    void box_sprite(RECT *,Style *) {}
    void set_font(Font *f,Font *,Font *,Font *hyper) {font_=f;hyper_=hyper;}
    void set_text_color(int color,int a,int b,int c) {assert(a==-1&&b==1&&c==1);colors_[0]=color;}
    void set_text_color2(int color,int a,int b,int c) {assert(a==-1&&b==1&&c==1);colors_[1]=color;}
    void set_text_color_hyper(int color,int a,int b,int c) {assert(a==-1&&b==1&&c==1);colors_[2]=color;}
};
struct Win {};
struct GraphicWin:Win,Buffer {
    RECT attached_; Win *parent_; int fill_;
    void init(int x,int y,int w,int h,int title,int flags,Win *parent,int a,int b) {
        assert(title==0&&flags==0x1000000&&a==0&&b==0);
        attached_.left=x;attached_.top=y;attached_.right=x+w;attached_.bottom=y+h;parent_=parent;
    }
    void fill(int color) {fill_=color;}
};
struct SubInterface {int field_4_;};
static SubInterface *registered;
static void register_iface_mode(int slot,SubInterface *iface) {assert(slot==8);assert(iface->field_4_==8);registered=iface;}
struct MainInterface:GraphicWin {
    int height_;RECT layout_,left_,right_;
    int panel_height() const {return height_;}
    const RECT &main_layout_rect() const {return layout_;}
    const RECT &main_frame_rect() const {return left_;}
    const RECT &right_frame_rect() const {return right_;}
};
static MainInterface MainInterfaceGlobal;
static GraphicWin ConsoleGlobal;
static int WinScreenWidth,WinScreenHeight;
static float CaviarCameraDistance=-10000.0f;
struct VOX_Matrix {float values[3][3];};
struct VOX_Vect {float values[3];};
static void caviar_apply_rotation(const float *angles,VOX_Matrix *out) {
    assert(angles[0]==0&&angles[1]==0);
    union {uint32_t bits;float value;} expected;expected.bits=0x3f48f5c3;
    assert(angles[2]==expected.value);
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)out->values[i][j]=float(i*3+j+1);
}
static void vox_matrix_vector_mul(const float (*matrix)[3],const float *input,float *out) {
    assert(matrix[2][0]==-7&&matrix[2][1]==-8&&matrix[2][2]==-9);
    assert(input[0]==0&&input[1]==0&&input[2]==1);
    out[0]=101;out[1]=102;out[2]=103;
}
struct Caviar {
    VOX_Matrix matrix_;VOX_Vect position_,light_vector_,shadow_vector_;int flag_;
    VOX_Vect &light_vector() {return light_vector_;}
    VOX_Vect &shadow_vector() {return shadow_vector_;}
    void init() {}
    void set_field_A4(int flag) {flag_=flag;}
    void set_camera_direct(VOX_Vect *position,VOX_Matrix *matrix) {position_=*position;matrix_=*matrix;}
    void set_scene_rotation(float angle,float y,float z) {
        union {uint32_t bits;float value;} expected;expected.bits=0x3f05f92d;
        assert(angle==expected.value&&y==0&&z==0);
    }
};
struct Heap {};
struct Dialog {
    int field_44_,field_B4_;RECT bounds_;Font *font_;GraphicWin *parent_;
    int x_,y_;
    void init(RECT *rect,Heap *heap) {assert(!heap);bounds_=*rect;events.push_back(20);}
    void set_dialog_font(Font *font,int a,int b) {assert(a==0&&b==0);font_=font;events.push_back(21);}
    void attach(GraphicWin *parent,int x,int y,int flags) {assert(flags==0x1000020);parent_=parent;x_=x;y_=y;events.push_back(22);}
};
struct ListBox:Dialog {
    int field_8_,field_2C_;
    void close() {events.push_back(19);}
};
struct FlatButton:Win,Buffer {
    int id_,x_,y_,width_,height_,redraws_,hides_;Win *parent_;
    FlatButton():redraws_(0),hides_(0) {}
    void init(const char *,int id,int x,int y,int w,int h,Win *parent,int flags) {
        assert(flags==0);id_=id;x_=x;y_=y;width_=w;height_=h;parent_=parent;events.push_back(100+id);
    }
    void set_border_styles(Style *a,Style *b,Style *c) {assert(a==&InterfaceBoxStyles[27]&&b==&InterfaceBoxStyles[28]&&c==&InterfaceBoxStyles[29]);}
    void redraw() {++redraws_;}
    void hide() {++hides_;}
};
struct ButtonGroup {
    std::vector<FlatButton *> entries_;
    void init(int a,int b) {assert(a==0&&b==0);events.push_back(23);}
    void add(FlatButton *entry) {entries_.push_back(entry);}
};
struct StringBox:Win,Buffer {
    int id_,x_,y_,width_,height_;Win *parent_;
    void init(int x,int y,int w,int h,Win *parent,int list,int flags) {
        assert(list==0&&flags==-1);x_=x;y_=y;width_=w;height_=h;parent_=parent;events.push_back(id_);
    }
};
static char StringTemp[1024];
static const char *label_get(int) {return "label";}
static void CharUpperA(char *text) {for(;*text;++text)*text=char(std::toupper(*text));}
struct Datalink:GraphicWin,SubInterface {
    int history_[1],field_2A34_,field_2A38_;
    RECT interface_update_rect_,category_rect_,illustration_rect_,title_rect_,description_rect_;
    RECT panel_rect_,entry_clip_rect_,entry_update_rect_,entry_frame_rect_,topic_button_bounds_[18];
    uint32_t field_10394_,field_10398_,field_1039C_,field_103A0_;
    uint32_t field_103E4_,field_103E8_,field_103EC_,field_103F0_;
    Font auxiliary_font10_,auxiliary_font11_,font1_,font3_,auxiliary_font4_,auxiliary_font5_;
    Font body_font_,secondary_body_font_,link_font_,auxiliary_font8_,font2_;
    Caviar caviar_;ListBox listBox_;ButtonGroup buttonGroup_;FlatButton topic_buttons_[18];
    StringBox blurb_box_,concept_box_,detail_box_;
    void init();
};
// @@BODIES@@
static void check_box(StringBox &box,int x,int y,int w,int h,Win *parent,Font *font,Font *hyper) {
    assert(box.x_==x&&box.y_==y&&box.width_==w&&box.height_==h&&box.parent_==parent);
    assert(box.font_==font&&box.hyper_==hyper);
    assert(box.colors_[0]==0x98&&box.colors_[1]==0x96&&box.colors_[2]==0x96);
}
int main() {
    for(int mode=0;mode<3;++mode) {
        events.clear();
        WinScreenWidth=mode==0?800:(mode==1?1024:799);WinScreenHeight=900;
        MainInterfaceGlobal.height_=80;
        RECT layout={11,22,333,444},left={10,20,110,320},right={200,30,300,340};
        MainInterfaceGlobal.layout_=layout;MainInterfaceGlobal.left_=left;MainInterfaceGlobal.right_=right;
        Datalink owner;owner.blurb_box_.id_=30;owner.concept_box_.id_=31;owner.detail_box_.id_=32;
        owner.init();int side=mode==0?0:138,content=mode==0?520:658;
        assert(registered==static_cast<SubInterface *>(&owner));
        assert(owner.history_[0]==0&&owner.field_2A34_==0&&owner.field_2A38_==0);
        assert(owner.title_rect_.left==239&&owner.description_rect_.left==239&&owner.field_10394_==239);
        assert(owner.attached_.left==(WinScreenWidth-800)/2&&owner.attached_.top==900-80-content);
        assert(owner.attached_.right-owner.attached_.left==832&&owner.attached_.bottom-owner.attached_.top==content);
        assert(owner.parent_==static_cast<Win *>(&ConsoleGlobal)&&owner.fill_==9);
        for(int i=0;i<18;++i) {
            FlatButton &b=owner.topic_buttons_[i];
            int x,y,w,h;
            if(i<8) {x=14;y=295-24*(7-i);w=92;h=22;}
            else if(i<16) {x=204;y=315-24*(i-8);w=92;h=22;}
            else {x=i==16?152:402;y=side+334;w=246;h=20;}
            assert(b.id_==i&&b.x_==x&&b.y_==y&&b.width_==w&&b.height_==h);
            assert(b.parent_==(i<16?static_cast<Win *>(&MainInterfaceGlobal):static_cast<Win *>(&owner)));
            assert(b.font_==&owner.auxiliary_font4_&&b.redraws_==1&&b.hides_==1);
        }
        assert(owner.buttonGroup_.entries_.size()==16);
        assert(owner.listBox_.field_8_==1&&owner.listBox_.field_2C_==0);
        assert(owner.listBox_.field_44_==0&&owner.listBox_.field_B4_==0);
        assert(owner.listBox_.x_==33&&owner.listBox_.y_==32);
        assert(owner.listBox_.font_==&owner.font2_&&owner.listBox_.parent_==static_cast<GraphicWin *>(&owner));
        assert(events.size()==26);
        for(int i=0;i<5;++i)assert(events[i]==19+i);
        for(int i=0;i<16;++i)assert(events[5+i]==100+i);
        assert(events[21]==117&&events[22]==116&&events[23]==30&&events[24]==31&&events[25]==32);
        check_box(owner.blurb_box_,14,25,316,416,&MainInterfaceGlobal,&owner.auxiliary_font8_,0);
        check_box(owner.concept_box_,242,39,525,side+281,&owner,&owner.body_font_,&owner.link_font_);
        check_box(owner.detail_box_,245,side/2+188,519,side-side/2+129,&owner,&owner.body_font_,&owner.link_font_);
        assert(owner.font2_.height_==(mode==0?12:14));
        assert(owner.auxiliary_font8_.height_==(mode==0?10:12));
        assert(owner.auxiliary_font8_.unk_1_==(mode==0?4:0));
        for(int axis=0;axis<3;++axis) {
            assert(owner.caviar_.matrix_.values[2][axis]==-(7+axis));
            assert(owner.caviar_.position_.values[axis]==float((7+axis)*10000));
            assert(owner.caviar_.light_vector_.values[axis]==(axis==2?1:0));
            assert(owner.caviar_.shadow_vector_.values[axis]==(axis==2?1:0));
        }
    }
}
