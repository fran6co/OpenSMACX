// ORIGINAL: 0x00462980 FILE
// name      ?calculate_dimensions@MapWin@@QAEXXZ
// size      1402 bytes
// spans     0x00462980-0x00462EFA
// prototype void (__thiscall ?calculate_dimensions@MapWin@@QAEXXZ)(MapWin* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618F40
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00462980
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00462980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?calculate_dimensions@MapWin@@QAEXXZ  at 0x00462980  (1402 bytes)
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
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class MapWin;
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
};

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
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
    int init(char *, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_007d3c28 = (int *)0x007D3C28;
static int *const g_007d3c2c = (int *)0x007D3C2C;
static int *const g_007d3c30 = (int *)0x007D3C30;
static int *const g_007d3c34 = (int *)0x007D3C34;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_00949884 = (int *)0x00949884;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_009a6494 = (int *)0x009A6494;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void calculate_dimensions();
};

void MapWin::calculate_dimensions() {
    char *self = reinterpret_cast<char *>(this);
#define I32(off) (*reinterpret_cast<int32_t *>(self + (off)))

    I32(0x1ddac) = 0;
    I32(0x1ddb0) = 0;

    int32_t v;
    if (*g_00691e6c > 0) {
        v = (*g_00691e6c * 50) / *g_00691e70;
    } else {
        v = ((I32(0x1dd98) + 0x10) * 50) / 16;
    }
    v = (v + 1) / 4;
    I32(0x1ddc8) = v;
    if (v < 1) {
        I32(0x1ddc8) = 1;
    }
    int32_t T = I32(0x1ddc8);
    int32_t c2 = T * 2;
    I32(0x1ddc4) = c2;
    int32_t c4a = c2 * 2;
    int32_t c8 = c2 * 4;
    I32(0x1ddbc) = c4a;
    I32(0x1ddc0) = c2;
    I32(0x1ddb4) = c8;
    I32(0x1ddb8) = T * 4;

    int32_t vbaseOff = *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(self) + 4);
    int32_t w = *reinterpret_cast<int32_t *>(self + vbaseOff + 0x4c4);
    int32_t h = *reinterpret_cast<int32_t *>(self + vbaseOff + 0x4c8);

    I32(0x1ddcc) = (w + c8 - 1) / c8;
    I32(0x1ddd0) = ((T * 4 - h) + c2 - 1) / (T * 4);
    I32(0x1ddd4) = (w + T * 4 + c8 - 1) / c8;
    I32(0x1ddd8) = ((T * 4 - h) + c2 - 1) / (T * 4);

    if (reinterpret_cast<void *>(this) == reinterpret_cast<void *>(g_009156b0)) {
        int32_t a = I32(0x1dda0);
        int32_t b = I32(0x1dd9c);
        *g_007d3c28 = b;
        *g_007d3c2c = a;
        *g_007d3c30 = b;
        *g_007d3c34 = a;
        int32_t f73 = I32(0x1ddcc);
        int32_t f74 = I32(0x1ddd0);
        int32_t e1 = (b - f73) + 4;
        int32_t e2 = (f73 + b) - 4;
        int32_t e3 = (a - f74) + 4;
        int32_t e4 = (a + f74) - 6;
        *g_007d3c28 = e1;
        *g_007d3c30 = e2;
        *g_007d3c2c = e3;
        *g_007d3c34 = e4;
    }

    int32_t halfW = (*g_00949870) / 2;
    if (I32(0x1ddcc) > halfW) {
        I32(0x1ddcc) = halfW;
        int32_t tileProduct = I32(0x1ddcc) * I32(0x1ddb4);
        I32(0x1ddd4) = halfW + 1;
        I32(0x1ddac) = (w - tileProduct) / 2;
    }
    int32_t halfH = (*g_00949874) / 2;
    if (I32(0x1ddd0) > halfH) {
        I32(0x1ddd0) = halfH;
        int32_t tileProduct2 = I32(0x1ddd0) * I32(0x1ddb8);
        I32(0x1ddd8) = halfH + 1;
        I32(0x1ddb0) = -((h + tileProduct2) / 2);
    }

    int32_t halfW2 = (*g_00949870) / 2;
    I32(0x1ddcc) = (I32(0x1ddcc) < halfW2) ? I32(0x1ddcc) : halfW2;
    int32_t halfW3 = (*g_00949870) / 2;
    I32(0x1ddd4) = (I32(0x1ddd4) < halfW3) ? I32(0x1ddd4) : halfW3;

    int32_t f74_ = I32(0x1ddd0);
    int32_t aVal = I32(0x1dda0);
    int32_t panYinit = aVal - f74_;
    I32(0x1dda8) = panYinit;
    int32_t altY = *g_00949874 + (2 - f74_) * 2;
    int32_t panY;
    if (panYinit < -2 || altY < -2) {
        panY = -2;
    } else if (altY < panYinit) {
        panY = altY;
    } else {
        panY = panYinit;
    }
    I32(0x1dda8) = panY;

    int32_t f73_ = I32(0x1ddcc);
    int32_t bVal = I32(0x1dd9c);
    int32_t panXinit = bVal - f73_;
    uint8_t wrapFlag = *reinterpret_cast<uint8_t *>(g_0094988c);
    int32_t panX;
    if ((wrapFlag & 1) == 0) {
        int32_t screenW = *g_00949870;
        if (panXinit < 0) {
            panX = panXinit + screenW;
        } else if (panXinit >= screenW) {
            panX = panXinit - screenW;
        } else {
            panX = panXinit;
        }
    } else {
        panX = panXinit;
    }
    I32(0x1dda4) = panX;

    if ((wrapFlag & 1) != 0) {
        int32_t altX = *g_00949870 + (2 - f73_) * 2;
        if (panX < -4 || altX < -4) {
            panX = -4;
        } else if (altX < panX) {
            panX = altX;
        }
        I32(0x1dda4) = panX;
    }

    I32(0x1dda4) = ((I32(0x1dda4) ^ I32(0x1dda8)) & 1) ^ I32(0x1dda4);

    int32_t fogW;
    if ((I32(0x1dd70) & 2) == 0 || I32(0x1dd98) < -10) {
        fogW = 0;
    } else if (*g_00691e6c < 1) {
        fogW = ((I32(0x1dd98) * 3 + 0x30) * 4) / 16;
    } else {
        fogW = (*g_00691e6c * 12) / *g_00691e70;
    }
    I32(0x1dddc) = fogW;

    int32_t fogH;
    if ((I32(0x1dd70) & 2) == 0 || I32(0x1dd98) < -10) {
        fogH = 0;
    } else if (*g_00691e6c < 1) {
        fogH = (I32(0x1dd98) * 4 + 0x40) / 16;
    } else {
        fogH = (*g_00691e6c * 4) / *g_00691e70;
    }
    I32(0x1dde0) = fogH;

    I32(0x1dd6c) = 0x19;
    if ((*g_009a6494 & 0x10000) != 0) {
        I32(0x1dd6c) = 8;
        I32(0x1dddc) = fogW / 3;
        I32(0x1dde0) = fogH / 3;
    }

    if (I32(0x1dd6c) != I32(0x1eec4)) {
        I32(0x1eec4) = I32(0x1dd6c);
        if (*reinterpret_cast<int32_t *>(self + 4) != 0) {
            int32_t count = *g_00949884;
            char *base = *reinterpret_cast<char **>(self + 4);
            for (int32_t ia = 0; ia < count; ++ia) {
                for (int32_t ja = 0; ja < 5; ++ja) {
                    base[ia * 0xc + ja + 4] = static_cast<char>(0xff);
                }
            }
        }
    }

    if (I32(0x1dd98) != I32(0x1eec0)) {
        if (*reinterpret_cast<int32_t *>(self + 4) != 0) {
            int32_t count = *g_00949884;
            char *base = *reinterpret_cast<char **>(self + 4);
            for (int32_t ib = 0; ib < count; ++ib) {
                for (int32_t jb = 0; jb < 4; ++jb) {
                    base[ib * 0xc + jb] = static_cast<char>(0xff);
                }
            }
            count = *g_00949884;
            base = *reinterpret_cast<char **>(self + 4);
            for (int32_t ic = 0; ic < count; ++ic) {
                for (int32_t jc = 0; jc < 5; ++jc) {
                    base[ic * 0xc + jc + 4] = static_cast<char>(0xff);
                }
            }
        }
        I32(0x1eec0) = I32(0x1dd98);

        int32_t fs1 = (I32(0x1ddb8) * 5) / 16;
        if (fs1 > 0xc) {
            fs1 = 0xc;
        }
        if (I32(0x1eef0) != fs1) {
            I32(0x1eef0) = fs1;
            reinterpret_cast<Font *>(self + 0x1eec8)->init(*reinterpret_cast<char **>(g_00691b2c), fs1, 0);
        }
        int32_t fs2 = (I32(0x1ddb8) * 5) / 16;
        if (fs2 > 0xc) {
            fs2 = 0xc;
        }
        if (I32(0x1ef48) != fs2) {
            I32(0x1ef48) = fs2;
            reinterpret_cast<Font *>(self + 0x1ef20)->init(*reinterpret_cast<char **>(g_00691b2c), fs2, 2);
        }
        int32_t fs3 = (I32(0x1ddb8) * 2) / 5;
        if (fs3 > 0xc) {
            fs3 = 0xc;
        }
        if (I32(0x1ef1c) != fs3) {
            I32(0x1ef1c) = fs3;
            reinterpret_cast<Font *>(self + 0x1eef4)->init(*reinterpret_cast<char **>(g_00691b2c), fs3, 1);
        }
    }

#undef I32
}
