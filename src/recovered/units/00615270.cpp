// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00615270
// name           ?on_redraw@EditBox@@QAEXXZ
// size           951 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00615270/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@EditBox@@QAEXXZ  at 0x00615270  (951 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class EditBox;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
};

class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
};

class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint8_t field_4BC_[80];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;
    int box(RECT *, int, int);
    int change_color(int, int, int, int, int, int);
    int set_clip(RECT *);
    int text_line_height();
    int text_width(char *, int);
    int write_l(char *, RECT *, int);
    void vline(int, int, int, int);
};
class GraphicWin { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
    void fill(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

class EditBox { public:
    uint8_t pad_0_[0xB48];
    uint32_t field_b48_;

    void on_redraw();
};

void EditBox::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    unsigned char flags = *reinterpret_cast<unsigned char *>(self + 0xb3c);

    char localBuf[256];
    if (flags & 2) {
        localBuf[0] = 0;
        strcat(localBuf, self + 0xa14);
        if (localBuf[0]) {
            char *p = localBuf;
            char c = *p;
            do {
                *p = '*';
                c = p[1];
                p++;
            } while (c);
        }
    }

    buf->set_clip(&buf->rect2_);
    reinterpret_cast<GraphicWin *>(self)->fill(*reinterpret_cast<int *>(self + 0xb1c));

    RECT r = *reinterpret_cast<RECT *>(self + 0x474);
    int count = *reinterpret_cast<int *>(self + 0xb30);
    for (int i = 0; i < count; i++) {
        buf->box(&r, *reinterpret_cast<int *>(self + 0xb28), *reinterpret_cast<int *>(self + 0xb2c));
        r.left++;
        r.right--;
        r.top++;
        r.bottom--;
    }

    int b30 = *reinterpret_cast<int *>(self + 0xb30);
    int b34 = *reinterpret_cast<int *>(self + 0xb34);
    int diff = b30 - b34;
    r.left -= diff;
    r.right += diff;
    r.top -= b30;
    r.bottom += b30;

    int lineHeight0 = buf->text_line_height();
    int vcenter = -(lineHeight0 + *reinterpret_cast<int *>(self + 0x4c8)) >> 1;

    if (*reinterpret_cast<int *>(self + 0xb48) != 0) {
        char *textPtr;
        int textLen;
        if (flags & 2) {
            textLen = *reinterpret_cast<int *>(self + 0xb44) - *reinterpret_cast<int *>(self + 0xb18);
            textPtr = localBuf;
        } else {
            int b18 = *reinterpret_cast<int *>(self + 0xb18);
            textLen = *reinterpret_cast<int *>(self + 0xb44) - b18;
            textPtr = self + 0xa14 + b18;
        }
        int w = buf->text_width(textPtr, textLen);
        int x = w + *reinterpret_cast<int *>(self + 0xb34);
        int color = *reinterpret_cast<int *>(self + 0x980);
        int h = buf->text_line_height();
        buf->vline(x, vcenter, h + vcenter, color);
    }

    buf->set_clip(&r);

    char *p2 = (flags & 2) ? localBuf : (self + 0xa14 + *reinterpret_cast<int *>(self + 0xb18));
    if (p2) {
        buf->write_l(p2, &r, strlen(p2));
    }

    int b40 = *reinterpret_cast<int *>(self + 0xb40);
    int b44 = *reinterpret_cast<int *>(self + 0xb44);
    if (b40 != b44) {
        char *wp;
        int wlen;
        int xpos;
        int b34_2 = *reinterpret_cast<int *>(self + 0xb34);
        int b18 = *reinterpret_cast<int *>(self + 0xb18);

        if (b40 > b44) {
            if (flags & 2) {
                wlen = b44 - b18;
                int w1 = buf->text_width(localBuf, wlen);
                xpos = w1 + b34_2;
                wp = localBuf;
            } else {
                wlen = b44 - b18;
                int w1 = buf->text_width(self + 0xa14 + b18, wlen);
                xpos = w1 + b34_2;
                wp = self + 0xa14 + b44;
            }
            wlen = b40 - b44;
        } else {
            int m = (b40 > b18) ? b40 : b18;
            if (flags & 2) {
                wlen = m - b18;
                int w1 = buf->text_width(localBuf, wlen);
                xpos = w1 + b34_2;
                wp = localBuf;
            } else {
                wlen = m - b18;
                int w1 = buf->text_width(self + 0xa14 + b18, wlen);
                xpos = w1 + b34_2;
                wp = self + 0xa14 + m;
            }
            wlen = b44 - b40;
        }

        int vx = buf->text_width(wp, wlen);

        int b24 = *reinterpret_cast<int *>(self + 0xb24);
        int color980 = *reinterpret_cast<int *>(self + 0x980);
        int color1 = (b24 == -1) ? color980 : b24;
        int b20 = *reinterpret_cast<int *>(self + 0xb20);
        int b1c = *reinterpret_cast<int *>(self + 0xb1c);
        int color2 = (b20 == -1) ? b1c : b20;
        int color1p1 = color1 + 1;

        int h1 = buf->text_line_height();
        buf->change_color(xpos, vcenter, vx, h1, b1c, color1p1);

        int h2 = buf->text_line_height();
        buf->change_color(xpos, vcenter, vx, h2, color980, color2);

        int h3 = buf->text_line_height();
        buf->change_color(xpos, vcenter, vx, h3, color1p1, color1);
    }
}
