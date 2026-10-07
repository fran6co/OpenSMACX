#include <assert.h>
#include <stdint.h>

struct Font { int tag; };
struct Payload { char *text; };
struct Entry {
    Entry *next;
    Payload *payload;
    Payload *string_payload() { return payload; }
};
typedef Entry StringStructEntry;
struct StringList {
    Entry *head_;
    Entry *current_;
    int entry_count_;
    int current_position_;
    char *pending_text_;
    uint32_t field_20_;
    uint32_t field_24_;
    int add_result;
    int added_id;
    int add(int id) { added_id=id; return add_result; }
};

struct Dialog {
    uint32_t field_20_, field_24_, field_28_, field_2C_, field_30_;
    uint32_t field_34_, field_38_, field_3C_, field_40_, field_44_;
    uint32_t field_48_, field_4C_, field_50_, field_54_, field_58_;
    uint32_t field_5C_, field_60_, field_64_, field_68_;
    Font *font1_, *font2_, *font3_;
    uint32_t text_color_a_, text_color_2a_, text_color_3a_;
    uint32_t text_color_b_, text_color_2b_, text_color_3b_;
    uint32_t text_color_c_, text_color_2c_, text_color_3c_;
    uint32_t text_color_d_, text_color_2d_, text_color_3d_;
    StringList item_list_;
};

struct Buffer {
    int colors[12];
    Font *fonts[3];
    int color_calls;
    int font_calls;
    int widths[8];
    int width_calls;
    int line_height;
    uint32_t field_18_;
    int wrap_result;
    int wrap_calls;
    int wrap_width;
    StringList *mutated_list;
    int mutate_on_width;
    int set_text_color(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        colors[0]=a; colors[1]=b; colors[2]=c; colors[3]=d; ++color_calls; return 0;
    }
    int set_text_color2(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        colors[4]=a; colors[5]=b; colors[6]=c; colors[7]=d; ++color_calls; return 0;
    }
    int set_text_color3(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        colors[8]=a; colors[9]=b; colors[10]=c; colors[11]=d; ++color_calls; return 0;
    }
    int set_font(Font *a, Font *b, Font *c, int) {
        fonts[0]=a; fonts[1]=b; fonts[2]=c; ++font_calls; return 0;
    }
    int text_width(char *) {
        int result = widths[width_calls++];
        if (mutated_list && width_calls == mutate_on_width)
            mutated_list->entry_count_ = 1;
        return result;
    }
    int text_line_height() { return line_height; }
    int wrap_height(char *, int width) {
        ++wrap_calls; wrap_width=width; field_18_=static_cast<uint32_t>(wrap_result);
        return wrap_result + 1000; // Deliberately different from measured state.
    }
};
struct Win { uint32_t iFlags_; };
struct GraphicWin : Win, Buffer {};
struct ListBox : GraphicWin, Dialog {
    uint32_t field_C_, field_18_, field_1C_, field_20_, field_2C_;
    int calculate_dimensions();
    int item(char *text, int id);
};
int ScrollDefaultThickness;

/* PRODUCT_DEFINITION */
/* PRODUCT_ITEM_DEFINITION */

static void setup(ListBox &x, Entry &a, Entry &b, Payload *pa, Payload *pb) {
    Dialog &d = x;
    x.field_C_=0xFFFFFFFFu; x.field_1C_=0; x.field_20_=2; x.field_2C_=3;
    x.field_20_ = 2;
    x.item_list_.head_=&a;
    x.item_list_.current_=&a;
    x.item_list_.entry_count_=2;
    x.item_list_.current_position_=0;
    x.item_list_.pending_text_=0;
    x.item_list_.field_20_=0xFFFFFFFFu;
    x.item_list_.field_24_=0;
    x.item_list_.add_result=0;
    x.item_list_.added_id=-1;
    a.next=&b; a.payload=pa; b.next=&a; b.payload=pb;
    x.field_20_=2; x.iFlags_=0; x.field_C_=0xFFFFFFFFu;
    x.field_2C_=3;
    x.field_20_=2;
    d.field_20_=2; d.field_24_=d.field_28_=0; d.field_2C_=3;
    d.field_30_=0; d.field_34_=2; d.field_38_=1; d.field_3C_=1;
    d.field_40_=1; d.field_44_=1; d.field_48_=10; d.field_4C_=1;
    d.field_50_=2; d.field_54_=1; d.field_58_=0; d.field_5C_=30;
    d.field_60_=20; d.field_64_=0; d.field_68_=0;
    x.font1_=0; x.font2_=0; x.font3_=0;
    x.text_color_a_=11; x.text_color_b_=12; x.text_color_c_=13; x.text_color_d_=14;
    x.text_color_2a_=21; x.text_color_2b_=22; x.text_color_2c_=23; x.text_color_2d_=24;
    x.text_color_3a_=31; x.text_color_3b_=32; x.text_color_3c_=33; x.text_color_3d_=34;
    x.width_calls=0; x.widths[0]=7; x.widths[1]=9; x.line_height=8;
    x.mutated_list=0; x.mutate_on_width=0;
    x.wrap_calls=0; x.wrap_result=0; x.wrap_width=0;
    x.field_18_=99; // ListBox scroll state must not supply measured height.
    static_cast<Buffer &>(x).field_18_=0;
    x.color_calls=0; x.font_calls=0;
}

int main() {
    ScrollDefaultThickness=5;
    ListBox empty;
    empty.item_list_.entry_count_=0;
    empty.field_C_=77;
    assert(empty.calculate_dimensions()==7);
    assert(empty.field_C_==77);

    Entry a,b; Payload pa,pb; char one[]="one", two[]="two";
    pa.text=one; pb.text=two;
    ListBox x;
    setup(x,a,b,&pa,&pb);
    Font f1,f2,f3; x.font1_=&f1; x.font2_=&f2; x.font3_=&f3;
    assert(x.calculate_dimensions()==0);
    assert(x.color_calls==3 && x.font_calls==1);
    assert(x.colors[0]==11 && x.colors[1]==12 && x.colors[2]==13 && x.colors[3]==14);
    assert(x.colors[4]==21 && x.colors[5]==22 && x.colors[6]==23 && x.colors[7]==24);
    assert(x.colors[8]==31 && x.colors[9]==32 && x.colors[10]==33 && x.colors[11]==34);
    assert(x.fonts[0]==&f1 && x.fonts[1]==&f2 && x.fonts[2]==&f3);
    assert(x.width_calls==2);
    assert(x.item_list_.current_==&a && x.item_list_.current_position_==0);
    assert(x.field_C_==0 && x.field_1C_==2);

    // The condition reloads entry_count_ after text_width callbacks.
    setup(x,a,b,&pa,&pb);
    Buffer &buffer=static_cast<Buffer &>(static_cast<GraphicWin &>(x));
    buffer.mutated_list=&x.item_list_;
    buffer.mutate_on_width=1;
    assert(x.calculate_dimensions()==0);
    assert(x.width_calls==1 && x.item_list_.entry_count_==1);

    // Automatic width mode (Dialog flags bit 1 clear) computes one column and
    // retains the minimum-width clamp.
    setup(x,a,b,&pa,&pb);
    static_cast<Dialog &>(x).field_20_=0;
    x.field_2C_=20;
    x.calculate_dimensions();
    assert(static_cast<Dialog &>(x).field_54_==1);
    assert(static_cast<int32_t>(static_cast<Dialog &>(x).field_5C_)>=20);

    // Fixed height/columns with signed vertical overflow and the no-adjust flag.
    setup(x,a,b,&pa,&pb);
    x.field_20_=3; x.field_30_=1; x.field_4C_=2; x.field_50_=1;
    x.field_58_=1; x.field_68_=2; x.iFlags_=0;
    assert(x.calculate_dimensions()==0);
    assert(x.field_C_==1 && x.field_60_==16);

    // The 0x40 flag suppresses the global adjustment while retaining capacity.
    setup(x,a,b,&pa,&pb);
    x.field_20_=3; x.field_30_=1; x.field_4C_=2; x.field_50_=1;
    x.field_58_=1; x.field_68_=2; x.iFlags_=0x40;
    assert(x.calculate_dimensions()==0);
    assert(x.field_1C_==x.field_54_*x.field_58_);
    assert(x.field_C_==1 && x.field_60_==11);

    // Null text returns before measuring, height accumulation or insertion.
    setup(x,a,b,&pa,&pb);
    int old_total=x.field_20_;
    assert(x.item(0,55)==3);
    assert(x.field_20_==static_cast<uint32_t>(old_total));
    assert(x.wrap_calls==0 && x.item_list_.pending_text_==0);

    // Nonpositive available width skips wrap_height and contributes one row.
    setup(x,a,b,&pa,&pb);
    static_cast<Dialog &>(x).field_2C_=ScrollDefaultThickness;
    x.field_20_=0xFFFFFFFEu;
    x.item_list_.add_result=19;
    assert(x.item(one,77)==19);
    assert(x.wrap_calls==0 && x.field_20_==0xFFFFFFFFu);
    assert(x.item_list_.pending_text_==one);
    assert(x.item_list_.field_20_==0 && x.item_list_.field_24_==1);
    assert(x.item_list_.added_id==77);

    // Positive available width uses wrap_height's Buffer field_18_, and the
    // height accumulator wraps as a dword before add's return is propagated.
    setup(x,a,b,&pa,&pb);
    static_cast<Dialog &>(x).field_2C_=20;
    x.field_20_=0xFFFFFFFEu;
    x.wrap_result=3;
    x.item_list_.add_result=23;
    assert(x.item(two,88)==23);
    assert(x.wrap_calls==1 && x.wrap_width==15);
    assert(x.field_20_==1);
    assert(x.item_list_.pending_text_==two);
    assert(x.item_list_.field_20_==0 && x.item_list_.field_24_==3);
    assert(x.item_list_.added_id==88);
    assert(x.field_18_==99); // Insertion does not change ListBox scroll state.

    // Invalid negative geometry still uses the pinned signed division; do
    // not silently reinterpret the shared thickness or column count unsigned.
    setup(x,a,b,&pa,&pb);
    ScrollDefaultThickness=-5;
    x.field_4C_=static_cast<uint32_t>(-2);
    assert(x.calculate_dimensions()==0);
    assert(x.field_1C_==static_cast<uint32_t>(-4));
    assert(x.field_64_==static_cast<uint32_t>(-4));
    assert(x.field_C_==2);
}
