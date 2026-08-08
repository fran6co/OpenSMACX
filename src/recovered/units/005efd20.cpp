// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005EFD20
// name           ?flip@Win@@QAAXPAURECT@@@Z
// size           1223 bytes
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005efd20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?flip@Win@@QAAXPAURECT@@@Z  at 0x005EFD20  (1223 bytes)
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
class Font;
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
class Palette;
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
    int fill(RECT *, int);
    int get_hdc();
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int sync_to_palette(Palette *);
    int write_cent_l(char *, RECT *, int);
    int write_cent_l(char *, int, int, int, int);
    void release_hdc(int);
    void set_text_color(int, int, int, int);
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
extern "C" int __cdecl strchr();
extern "C" unsigned int strlen(const char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 5, 11, 17, 22, 26
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();  // <-- used
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();  // <-- used
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();  // <-- used
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();  // <-- used
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669054 = (int *)0x00669054;
static int *const g_0066909c = (int *)0x0066909C;
static int *const g_006690b8 = (int *)0x006690B8;
static int *const g_0066927c = (int *)0x0066927C;
static int *const g_00669280 = (int *)0x00669280;
static int *const g_00669338 = (int *)0x00669338;
static int *const g_00696d2c = (int *)0x00696D2C;
static int *const g_00696d30 = (int *)0x00696D30;
static int *const g_009b3ab0 = (int *)0x009B3AB0;
static int *const g_009b6e38 = (int *)0x009B6E38;
static int *const g_009b6e3c = (int *)0x009B6E3C;
static int *const g_009b6e40 = (int *)0x009B6E40;
static int *const g_009b6e44 = (int *)0x009B6E44;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b74c0 = (int *)0x009B74C0;
static int *const g_009b7510 = (int *)0x009B7510;
static int *const g_009b7514 = (int *)0x009B7514;
static int *const g_009b7a2c = (int *)0x009B7A2C;
static int *const g_009b7a30 = (int *)0x009B7A30;
static int *const g_009b7a34 = (int *)0x009B7A34;
static int *const g_009b7a48 = (int *)0x009B7A48;
static int *const g_009b7a50 = (int *)0x009B7A50;
static int *const g_009b7a54 = (int *)0x009B7A54;
static int *const g_009b7a64 = (int *)0x009B7A64;
static int *const g_009b7ad8 = (int *)0x009B7AD8;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009b7b2c = (int *)0x009B7B2C;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8180 = (int *)0x009B8180;
static int *const g_009bc2d0 = (int *)0x009BC2D0;
static int *const g_009bc494 = (int *)0x009BC494;
static int *const g_009bc498 = (int *)0x009BC498;
static int *const g_009bc49c = (int *)0x009BC49C;

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

    void __stdcall release_hdc();
    void __cdecl flip(RECT *);
};

// Raw imports called through fixed IAT-style slots: this routine reaches
// GDI/DirectDraw entry points (GetDC/ReleaseDC/BitBlt-shaped calls) directly
// by address rather than through any declared C++ signature, so each slot
// is invoked through a plain stdcall function pointer of the arity its call
// site pushes.
typedef int(__stdcall *Fn0)();
typedef int(__stdcall *Fn1)(int);
typedef int(__stdcall *Fn2)(int, int);
typedef int(__stdcall *Fn3)(int, int, int);
typedef int(__stdcall *Fn8)(int, int, int, int, int, int, int, int);
typedef int(__stdcall *Fn9)(int, int, int, int, int, int, int, int, int);

// `strchr` is catalogued nullary (its arity did not settle), but this body
// calls it with the usual (const char*, int) - cast the address through the
// real CRT signature rather than declaring a second, conflicting prototype.
typedef char *(__cdecl *StrchrFn)(const char *, int);
static StrchrFn const call_strchr = reinterpret_cast<StrchrFn>(&strchr);

void __cdecl Win::flip(RECT *a1) {
    if (*g_009b7a48 != 0)
        reinterpret_cast<Fn0>(*g_009b7a48)();

    Buffer *buf = reinterpret_cast<Buffer *>(g_009b7490);

    if (*g_009b7a50 == 0)
        goto AFTER_BANNER;

    if (a1 != 0)
        buf->set_clip(a1);

    {
        int32_t edge = *g_00696d30;
        buf->box(reinterpret_cast<RECT *>(g_009b6e38), edge, edge);

        RECT *box = reinterpret_cast<RECT *>(g_009b6e38);
        box->left += 1;
        box->top -= 1;
        box->right += 1;
        box->bottom -= 1;
        buf->fill(box, *g_009b7a64);

        buf->set_font(reinterpret_cast<Font *>(*g_009b7a54), 0, 0, 0);
        buf->set_text_color(*g_00696d2c, -1, 1, 1);

        char *text = reinterpret_cast<char *>(*g_009b7a50);
        char *split = call_strchr(text, 0x5e);
        if (split == 0) {
            char *msg = reinterpret_cast<char *>(*g_009b7a50);
            if (msg != 0)
                buf->write_cent_l(msg, box, static_cast<int>(strlen(msg)));
            goto AFTER_BANNER;
        }

        int32_t top = *g_009b6e3c + 1;
        char *line = text;
        for (;;) {
            char *next = call_strchr(line, 0x5e);
            if (next != 0)
                *next = 0;
            buf->write_cent_l(line, *g_009b6e38, top,
                               static_cast<int>(strlen(line)), 0);
            if (next == 0)
                break;
            *next = 0x5e;
            line = next + 1;
            top += 0;
        }
    }

AFTER_BANNER:
    {
        int32_t left = *g_009b6e38;
        int32_t top = *g_009b6e40;
        int32_t right = *g_009b6e3c;
        int32_t bottom = *g_009b6e44;
        *g_009b6e38 = left - 1;
        *g_009b6e40 = top + 1;
        *g_009b6e3c = right - 1;
        *g_009b6e44 = bottom + 1;
        if (a1 != 0)
            buf->set_clip(reinterpret_cast<RECT *>(g_009b74c0));
    }

    if (*g_009b7a2c != 0) {
        int32_t x = *g_009b7a34;
        int32_t y = *g_009b7a30;
        uint8_t frame = *reinterpret_cast<uint8_t *>(*g_009b7a2c + 8);
        reinterpret_cast<Sprite *>(*g_009b7a2c)
            ->draw(buf, frame, y, x, 1, 1);
    }

    if (*g_009bc498 == 0)
        goto DONE;

    if (*g_009b7ad8 == 0) {
        int32_t hCanvas = *g_009bc498;
        int32_t altHdc = *g_009bc49c;
        reinterpret_cast<VCall *>(hCanvas)->slot005();
        goto DONE;
    }

    if (a1 == 0) {
        int32_t hCanvas = *g_009bc498;
        int32_t altHdc = *g_009bc49c;
        reinterpret_cast<VCall *>(hCanvas)->slot005();
        goto DONE;
    } else {
        int32_t pt[2];
        int32_t ok = reinterpret_cast<Fn3>(*g_00669338)(
            reinterpret_cast<int32_t>(a1), reinterpret_cast<int32_t>(g_009bc2d0),
            reinterpret_cast<int32_t>(pt));
        if (ok == 0)
            return;
        reinterpret_cast<VCall *>(*g_009bc498)->slot005();
        goto DONE;
    }

DONE:
    if (*g_009b3ab0 != 0) {
        int32_t count = *g_009b3ab0;
        *g_009b3ab0 = count + 1;
    } else {
        typedef int (__stdcall *GetDcFn)(int);
        GetDcFn getDc = reinterpret_cast<GetDcFn>(*g_0066927c);
        int32_t hdc = getDc(*g_009b7b28);
        *g_009b7b2c = hdc;
        if (hdc != 0)
            *g_009b3ab0 = 1;
    }

    if (*g_009b7b2c == 0)
        return;

    {
        int32_t hdc = buf->get_hdc();
        if (hdc == 0) {
            int32_t count = *g_009b3ab0 - 1;
            *g_009b3ab0 = count;
            if (count != 0)
                return;
            if (*g_009bc498 != 0)
                reinterpret_cast<VCall *>(*g_009bc498)->slot005();
            *g_009b7b2c = hdc;
            return;
        }

        buf->sync_to_palette(reinterpret_cast<Palette *>(*g_009b8180));

        if (*g_009bc494 == 0) {
            if (*g_009b3ab0 != 0) {
                *g_009b3ab0 = *g_009b3ab0 + 1;
            } else if (*g_009bc498 != 0) {
                reinterpret_cast<VCall *>(*g_009bc498)->slot011();
                *g_009b3ab0 = 1;
            } else {
                typedef int (__stdcall *GetDcFn)(int);
                *g_009b7b2c = reinterpret_cast<GetDcFn>(*g_0066927c)(*g_009b7b28);
                if (*g_009b7b2c != 0)
                    *g_009b3ab0 = 1;
            }

            if (*g_009b7b2c != 0) {
                reinterpret_cast<Fn3>(*g_006690b8)(*g_009b7b2c, *g_009b8178, 0);
                reinterpret_cast<Fn1>(*g_0066909c)(*g_009b7b2c);
                release_hdc();
            }
        }
    }

    if (a1 == 0) {
        int32_t width = *g_009b7514;
        int32_t height = -*g_009b7510;
        reinterpret_cast<Fn9>(*g_00669054)(
            *g_009b7b2c, 0, 0, width, height, 0, 0, 0x1000000, 0xcc0020);
    } else {
        int32_t left = a1->left;
        int32_t top = a1->top;
        int32_t w = a1->right - left;
        int32_t h = a1->bottom - top;
        reinterpret_cast<Fn9>(*g_00669054)(
            *g_009b7b2c, left, top, w, h, left, top, 0x1000000, 0xcc0020);
    }

    {
        int32_t count = *g_009b3ab0 - 1;
        *g_009b3ab0 = count;
        if (count == 0) {
            if (*g_009bc498 != 0) {
                reinterpret_cast<VCall *>(*g_009bc498)->slot026();
            } else {
                reinterpret_cast<Fn2>(*g_00669280)(*g_009b7b28, *g_009b7b2c);
            }
            *g_009b7b2c = 0;
        }
    }
}
