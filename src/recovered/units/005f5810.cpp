// ORIGINAL: 0x005F5810 FILE
// name      ?on_nc_paint@Win@@QAEXPAURECT@@H@Z
// size      698 bytes
// spans     0x005F5810-0x005F5ACA
// prototype void (__thiscall ?on_nc_paint@Win@@QAEXPAURECT@@H@Z)(Win* this, RECT* rect, int)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005D84C0 0x005ED240 0x005F7E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F5810
// measured tier  MISMATCH
// divergence     28
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5810/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_nc_paint@Win@@QAEXPAURECT@@H@Z  at 0x005F5810  (698 bytes)
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
    int draw(Buffer *, int, int, int, int, int);
    int set_clip(RECT *);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b74c0 = (int *)0x009B74C0;
static int *const g_009b7a68 = (int *)0x009B7A68;
static int *const g_009b7a70 = (int *)0x009B7A70;
static int *const g_009b7a74 = (int *)0x009B7A74;

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

    void client_to_screen(int *, int *);
    int is_visible();
    void on_nc_paint(RECT *, int);
};

void Win::on_nc_paint(RECT * a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if ((*reinterpret_cast<uint8_t *>(self + 0x98) & 0x11) != 0 &&
        (*reinterpret_cast<uint8_t *>(self + 0x9c) & 1) != 0 &&
        (*reinterpret_cast<int *>(self + 0xc4) == 0 || this->is_visible() != 0)) {
        int stack8 = 0, stack4 = 0;
        this->client_to_screen(&stack8, &stack4);
        stack8 -= *reinterpret_cast<int *>(self + 0x13c);
        stack4 -= *reinterpret_cast<int *>(self + 0x140);

        if (a1 != 0) {
            a2 = -1;
            reinterpret_cast<Buffer *>(a1)->set_clip(a1);
        }

        int iVar1;
        if ((*reinterpret_cast<uint32_t *>(self + 0x98) & 0x10) == 0 ||
            (*reinterpret_cast<uint32_t *>(self + 0x98) & 0x400000) != 0) {
            iVar1 = *reinterpret_cast<int *>(self + 0x118);
        } else {
            iVar1 = *reinterpret_cast<int *>(self + 0x114);
        }

        if (*reinterpret_cast<int *>(self + 0xb4) != 0 && (a2 == -1 || a2 == 0)) {
            Buffer *buf;
            int x, y;
            if (*g_009b7a68 == 0) {
                buf = reinterpret_cast<Buffer *>(g_009b7490);
                x = stack8;
                y = stack4;
            } else {
                buf = reinterpret_cast<Buffer *>(*g_009b7a68);
                x = stack8 - *g_009b7a70;
                y = stack4 - *g_009b7a74;
            }
            reinterpret_cast<Buffer *>(g_009b7490)->draw(buf, *g_00696d14, x, y, 1, 1);
        }

        if (*reinterpret_cast<int *>(self + 0xc0) != 0 && (a2 == -1 || a2 == 2)) {
            int v = *reinterpret_cast<int *>(self + 0x11c);
            if (v == -1) {
                v = *reinterpret_cast<int *>(self + 0x118);
            }
            Buffer *buf;
            int x;
            if (*g_009b7a68 == 0) {
                v = (*reinterpret_cast<int *>(self + 0x158) -
                     *reinterpret_cast<int *>(self + 0x150)) - v;
                buf = reinterpret_cast<Buffer *>(g_009b7490);
                x = stack8;
            } else {
                v = ((*reinterpret_cast<int *>(self + 0x158) -
                      *reinterpret_cast<int *>(self + 0x150)) - *g_009b7a74) - v;
                buf = reinterpret_cast<Buffer *>(*g_009b7a68);
                x = stack8 - *g_009b7a70;
            }
            reinterpret_cast<Buffer *>(g_009b7490)->draw(buf, *g_00696d14, x, v + stack4, 1, 1);
        }

        if (*reinterpret_cast<int *>(self + 0xbc) != 0 && (a2 == -1 || a2 == 3)) {
            if (*g_009b7a68 == 0) {
                reinterpret_cast<Buffer *>(g_009b7490)
                    ->draw(reinterpret_cast<Buffer *>(g_009b7490), *g_00696d14, stack8,
                           stack4 + iVar1, 1, 1);
            } else {
                reinterpret_cast<Buffer *>(g_009b7490)
                    ->draw(reinterpret_cast<Buffer *>(*g_009b7a68), *g_00696d14,
                           stack8 - *g_009b7a70, (iVar1 - *g_009b7a74) + stack4, 1, 1);
            }
        }

        if (*reinterpret_cast<int *>(self + 0xb8) != 0 && (a2 == -1 || a2 == 1)) {
            Buffer *buf;
            int v;
            int yv = iVar1;
            if (*g_009b7a68 == 0) {
                v = (*reinterpret_cast<int *>(self + 0x154) -
                     *reinterpret_cast<int *>(self + 0x14c)) -
                    *reinterpret_cast<int *>(self + 0x118);
                buf = reinterpret_cast<Buffer *>(g_009b7490);
            } else {
                yv = iVar1 - *g_009b7a74;
                v = ((*reinterpret_cast<int *>(self + 0x154) -
                      *reinterpret_cast<int *>(self + 0x14c)) -
                     *reinterpret_cast<int *>(self + 0x118)) - *g_009b7a70;
                buf = reinterpret_cast<Buffer *>(*g_009b7a68);
            }
            reinterpret_cast<Buffer *>(g_009b7490)->draw(buf, *g_00696d14, v + stack8, yv + stack4, 1, 1);
        }

        reinterpret_cast<Buffer *>(g_009b7490)->set_clip(reinterpret_cast<RECT *>(g_009b74c0));
    }
}
