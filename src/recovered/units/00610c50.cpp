// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00610C50
// name           ?draw_item@SpriteBox@@QAEXH@Z
// size           1003 bytes
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00610c50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_item@SpriteBox@@QAEXH@Z  at 0x00610C50  (1003 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class Sprite;
class SpriteBox;

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
    int box(int, int, int, int, int, int);
    int copy(Buffer *, int, int, int, int, int, int);
    int set_font(Font *, Font *, Font *, Font *);
    int write_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
    void set_text_color2(int, int, int, int);
    void set_text_color3(int, int, int, int);
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
    int draw(Buffer *, int, int, int, int, int);
};
extern "C" int abs(int);
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00696d18 = (int *)0x00696D18;
static int *const g_00696d1c = (int *)0x00696D1C;

class SpriteBox { public:
    uint32_t field_0_;

    void draw_item(int);
};

void SpriteBox::draw_item(int a1) {
    char *self = reinterpret_cast<char *>(this);

    int vt = *reinterpret_cast<int *>(self);
    int vtOff1 = *reinterpret_cast<int *>(vt + 4);
    if (*reinterpret_cast<int *>(self + vtOff1 + 0xc4) == 0)
        return;

    int vtOff2 = *reinterpret_cast<int *>(vt + 8);
    int divisor = *reinterpret_cast<int *>(self + vtOff2 + 0x58);
    int q = a1 / divisor;
    int r = a1 % divisor;
    int accX = *reinterpret_cast<int *>(self + vtOff2 + 0x64) + *reinterpret_cast<int *>(self + vtOff2 + 0x44);
    accX *= q;
    int accY = r * *reinterpret_cast<int *>(self + vtOff2 + 0x68) +
               (*reinterpret_cast<int *>(self + vtOff2 + 0x44) >> 1);
    int edi = accX;
    int ebx = accY;

    if (a1 > *reinterpret_cast<int *>(self + 0x34) - 1)
        return;

    *reinterpret_cast<int *>(self + 0x30) = *reinterpret_cast<int *>(self + 0x2c);
    int idx1;
    if (a1 >= 0) {
        if (a1 > 0) {
            int n = a1;
            do {
                int cur = *reinterpret_cast<int *>(self + 0x30);
                n--;
                *reinterpret_cast<int *>(self + 0x30) = *reinterpret_cast<int *>(cur + 0xc);
            } while (n != 0);
        }
        idx1 = a1;
    } else {
        if (abs(a1) > *reinterpret_cast<int *>(self + 0x34))
            return;
        int n = abs(a1);
        if (n > 0) {
            do {
                int cur = *reinterpret_cast<int *>(self + 0x30);
                n--;
                *reinterpret_cast<int *>(self + 0x30) = *reinterpret_cast<int *>(cur + 0x10);
            } while (n != 0);
        }
        idx1 = *reinterpret_cast<int *>(self + 0x34) + a1;
    }
    *reinterpret_cast<int *>(self + 0x38) = idx1;

    int vt2 = *reinterpret_cast<int *>(self);
    int vtOff2b = *reinterpret_cast<int *>(vt2 + 8);
    char *p2 = self + vtOff2b + 0xbc;
    int bound2 = *reinterpret_cast<int *>(p2 + 0x10) - 1;
    if (a1 > bound2)
        return;

    *reinterpret_cast<int *>(p2 + 0xc) = *reinterpret_cast<int *>(p2 + 0x8);
    int idx2;
    if (a1 >= 0) {
        if (a1 > 0) {
            int n = a1;
            do {
                int cur = *reinterpret_cast<int *>(p2 + 0xc);
                n--;
                *reinterpret_cast<int *>(p2 + 0xc) = *reinterpret_cast<int *>(cur + 0xc);
            } while (n != 0);
        }
        idx2 = a1;
    } else {
        if (abs(a1) > *reinterpret_cast<int *>(p2 + 0x10))
            return;
        int n = abs(a1);
        if (n > 0) {
            do {
                int cur = *reinterpret_cast<int *>(p2 + 0xc);
                n--;
                *reinterpret_cast<int *>(p2 + 0xc) = *reinterpret_cast<int *>(cur + 0x10);
            } while (n != 0);
        }
        idx2 = *reinterpret_cast<int *>(p2 + 0x10) + a1;
    }
    *reinterpret_cast<int *>(p2 + 0x14) = idx2;

    int vt3 = *reinterpret_cast<int *>(self);
    char *p3 = self + *reinterpret_cast<int *>(vt3 + 8);
    RECT localRect;
    localRect.left = *reinterpret_cast<int *>(self + 0x54) + ebx + 4;
    localRect.top = edi;
    localRect.right = *reinterpret_cast<int *>(p3 + 0x64) + ebx;
    localRect.bottom = *reinterpret_cast<int *>(p3 + 0x68) + edi;

    int bufPtr1;
    if (this != 0) {
        int vt4 = *reinterpret_cast<int *>(self);
        int vtOff1c = *reinterpret_cast<int *>(vt4 + 4);
        bufPtr1 = reinterpret_cast<int>(self + vtOff1c + 0x444);
    } else {
        bufPtr1 = 0;
    }
    int px = *reinterpret_cast<int *>(p3 + 0x24) + ebx;
    int py = *reinterpret_cast<int *>(p3 + 0x28) + edi;
    reinterpret_cast<Buffer *>(self + 0x444)->copy(
        reinterpret_cast<Buffer *>(bufPtr1), px, py, ebx, edi,
        *reinterpret_cast<int *>(p3 + 0x64), *reinterpret_cast<int *>(p3 + 0x68));

    int t1;
    if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
        int cur = *reinterpret_cast<int *>(self + 0x30);
        int a = *reinterpret_cast<int *>(cur + 8);
        t1 = *reinterpret_cast<int *>(a + 4);
    } else {
        t1 = 0;
    }
    int scaleN = *reinterpret_cast<int *>(self + 0x1c);
    int scaleD = *reinterpret_cast<int *>(self + 0x20);
    int val1 = (*reinterpret_cast<int *>(t1 + 0x1c) * scaleN) / scaleD;
    int vt5 = *reinterpret_cast<int *>(self);
    int vtOff2d = *reinterpret_cast<int *>(vt5 + 8);
    int base1 = *reinterpret_cast<int *>(self + vtOff2d + 0x68) - val1;
    edi += base1 / 2;

    int t2;
    if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
        int cur = *reinterpret_cast<int *>(self + 0x30);
        int a = *reinterpret_cast<int *>(cur + 8);
        t2 = *reinterpret_cast<int *>(a + 4);
    } else {
        t2 = 0;
    }
    int val2 = (*reinterpret_cast<int *>(t2 + 0x18) * scaleN) / scaleD;
    int base2 = *reinterpret_cast<int *>(self + 0x54) - val2;
    ebx += base2 / 2;

    if (*reinterpret_cast<int *>(self + 0x64) == 0) {
        int bufPtr2;
        if (this != 0) {
            int vt6 = *reinterpret_cast<int *>(self);
            int vtOff1d = *reinterpret_cast<int *>(vt6 + 4);
            bufPtr2 = reinterpret_cast<int>(self + vtOff1d + 0x444);
        } else {
            bufPtr2 = 0;
        }
        int t3;
        if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
            int cur = *reinterpret_cast<int *>(self + 0x30);
            int a = *reinterpret_cast<int *>(cur + 8);
            t3 = *reinterpret_cast<int *>(a + 4);
        } else {
            t3 = 0;
        }
        int byteVal = reinterpret_cast<Sprite *>(t3)->cTransparentIndex_;
        reinterpret_cast<Sprite *>(t3)->draw(reinterpret_cast<Buffer *>(bufPtr2), byteVal, ebx, edi, scaleN, scaleD);
    } else {
        *g_00696d18 = scaleN;
        *g_00696d1c = scaleD;

        int obj1;
        if (this != 0) {
            int vt7 = *reinterpret_cast<int *>(self);
            int vtOff1e = *reinterpret_cast<int *>(vt7 + 4);
            obj1 = reinterpret_cast<int>(self + vtOff1e);
        } else {
            obj1 = 0;
        }
        int t4;
        if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
            int cur = *reinterpret_cast<int *>(self + 0x30);
            t4 = *reinterpret_cast<int *>(cur + 4);
        } else {
            t4 = 0;
        }
        int t5;
        if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
            int cur = *reinterpret_cast<int *>(self + 0x30);
            int a = *reinterpret_cast<int *>(cur + 8);
            t5 = *reinterpret_cast<int *>(a + 4);
        } else {
            t5 = 0;
        }
        typedef int(__cdecl * DrawCallback)(int, int, int, int, int);
        DrawCallback cb = *reinterpret_cast<DrawCallback *>(self + 0x64);
        cb(t5, obj1, t4, ebx, edi);

        int vt8 = *reinterpret_cast<int *>(self);
        int vtOff2e = *reinterpret_cast<int *>(vt8 + 8);
        int vtOff1f = *reinterpret_cast<int *>(vt8 + 4);
        Buffer *buf1 = reinterpret_cast<Buffer *>(self + vtOff1f + 0x444);
        char *pf = self + vtOff2e;
        buf1->set_text_color(*reinterpret_cast<int *>(pf + 0x7c), *reinterpret_cast<int *>(pf + 0x88),
                              *reinterpret_cast<int *>(pf + 0x94), *reinterpret_cast<int *>(pf + 0xa0));

        int vt9 = *reinterpret_cast<int *>(self);
        int vtOff2f = *reinterpret_cast<int *>(vt9 + 8);
        int vtOff1g = *reinterpret_cast<int *>(vt9 + 4);
        Buffer *buf2 = reinterpret_cast<Buffer *>(self + vtOff1g + 0x444);
        char *pf2 = self + vtOff2f;
        buf2->set_text_color2(*reinterpret_cast<int *>(pf2 + 0x80), *reinterpret_cast<int *>(pf2 + 0x8c),
                               *reinterpret_cast<int *>(pf2 + 0x98), *reinterpret_cast<int *>(pf2 + 0xa4));

        int vt10 = *reinterpret_cast<int *>(self);
        int vtOff2g = *reinterpret_cast<int *>(vt10 + 8);
        int vtOff1h = *reinterpret_cast<int *>(vt10 + 4);
        Buffer *buf3 = reinterpret_cast<Buffer *>(self + vtOff1h + 0x444);
        char *pf3 = self + vtOff2g;
        buf3->set_text_color3(*reinterpret_cast<int *>(pf3 + 0x84), *reinterpret_cast<int *>(pf3 + 0x90),
                               *reinterpret_cast<int *>(pf3 + 0x9c), *reinterpret_cast<int *>(pf3 + 0xa8));

        int vt11 = *reinterpret_cast<int *>(self);
        int vtOff2h = *reinterpret_cast<int *>(vt11 + 8);
        int vtOff1i = *reinterpret_cast<int *>(vt11 + 4);
        Buffer *buf4 = reinterpret_cast<Buffer *>(self + vtOff1i + 0x444);
        char *pf4 = self + vtOff2h;
        buf4->set_font(reinterpret_cast<Font *>(*reinterpret_cast<int *>(pf4 + 0x70)),
                        reinterpret_cast<Font *>(*reinterpret_cast<int *>(pf4 + 0x74)),
                        reinterpret_cast<Font *>(*reinterpret_cast<int *>(pf4 + 0x78)), 0);
    }

    int vt12 = *reinterpret_cast<int *>(self);
    int vtOff2i = *reinterpret_cast<int *>(vt12 + 8);
    char *p4 = self + vtOff2i;
    if (*reinterpret_cast<int *>(p4 + 0xec) == a1 && *reinterpret_cast<int *>(self + 4) != 0) {
        int t6;
        if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
            int cur = *reinterpret_cast<int *>(self + 0x30);
            int a = *reinterpret_cast<int *>(cur + 8);
            t6 = *reinterpret_cast<int *>(a + 4);
        } else {
            t6 = 0;
        }
        int field8 = *reinterpret_cast<int *>(self + 8);
        int p4_68 = *reinterpret_cast<int *>(p4 + 0x68);
        edi++;
        int boxVal = (*reinterpret_cast<int *>(t6 + 0x18) * *reinterpret_cast<int *>(self + 0x1c)) /
                     *reinterpret_cast<int *>(self + 0x20);
        int vtOff1j = *reinterpret_cast<int *>(vt12 + 4);
        Buffer *buf5 = reinterpret_cast<Buffer *>(self + vtOff1j + 0x444);
        buf5->box(ebx, edi, boxVal + ebx - 1, p4_68 + edi - 3, field8, field8);
    }

    int vt13 = *reinterpret_cast<int *>(self);
    int vtOff2j = *reinterpret_cast<int *>(vt13 + 8);
    int hasCount2 = *reinterpret_cast<int *>(self + vtOff2j + 0xc4);
    char *p2b = self + vtOff2j + 0xbc;
    char *textPtr;
    if (hasCount2 != 0) {
        int cur = *reinterpret_cast<int *>(p2b + 0xc);
        int a = *reinterpret_cast<int *>(cur + 8);
        textPtr = reinterpret_cast<char *>(*reinterpret_cast<int *>(a + 4));
    } else {
        textPtr = 0;
    }
    int vtOff1k = *reinterpret_cast<int *>(vt13 + 4);
    Buffer *buf6 = reinterpret_cast<Buffer *>(self + vtOff1k + 0x444);
    if (textPtr != 0) {
        unsigned int len = strlen(textPtr);
        buf6->write_l(textPtr, &localRect, len);
    }
}
