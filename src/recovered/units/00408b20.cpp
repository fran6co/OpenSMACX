// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00408B20
// name           ?draw_psych@BaseWin@@QAEXXZ
// size           901 bytes
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00408b20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_psych@BaseWin@@QAEXXZ  at 0x00408B20  (901 bytes)
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
class BaseWin;
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
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, int, int, int, int);
    void set_text_color(int, int, int, int);
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
    Font();
    int init(char *, int, int);
    ~Font();
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
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
void __cdecl bitmask(int, int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00408e94 = (int *)0x00408E94;
static int *const g_00651581 = (int *)0x00651581;
static int *const g_00672428 = (int *)0x00672428;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_00776438 = (int *)0x00776438;
static int *const g_0077643c = (int *)0x0077643C;
static int *const g_008c6d24 = (int *)0x008C6D24;
static int *const g_008c6d28 = (int *)0x008C6D28;
static int *const g_0090e920 = (int *)0x0090E920;
static int *const g_0090e94c = (int *)0x0090E94C;
static int *const g_0090e984 = (int *)0x0090E984;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_0096cc40 = (int *)0x0096CC40;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void psych_row(int, int, int, int, int, int, int);
    void draw_psych();
};

// Register-scaled index arrays: 0x90e984/0x90e920/0x90e94c indexed by the
// row counter (dup-check via byte offset, draw call via [reg*4+addr]), and
// 0x96cc40 / 0x97d0cc indexed by computed values - the indexed-table lever
// applies to all four, not the scaffold's folded pointer literal.
extern int g_x90e984[];
extern int g_x90e920[];
extern int g_x90e94c[];
extern int g_x96cc40[];
extern unsigned char g_x7d0cc[];

void BaseWin::draw_psych() {
    Font font;
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    RECT *oldClipSrc = reinterpret_cast<RECT *>(self + 0x464);
    RECT savedClip;
    savedClip.left = oldClipSrc->left;
    savedClip.top = oldClipSrc->top;
    savedClip.right = oldClipSrc->right;
    savedClip.bottom = oldClipSrc->bottom;

    RECT *newClip = reinterpret_cast<RECT *>(self + 0x40b8c);
    buf->set_clip(newClip);
    reinterpret_cast<GraphicWin *>(this)->fill(0);

    font.init(reinterpret_cast<char *>(*g_00691b2c), 0xc, 1);

    char *cfgPtr = reinterpret_cast<char *>(*g_0090ea30);
    int flagByte = *reinterpret_cast<unsigned char *>(cfgPtr + 4);
    int left = newClip->left;
    int top = newClip->top;
    int right = newClip->right;
    int bottom = newClip->bottom;
    int width = right - left;
    (void)bottom;

    int gVal = *g_0077643c;
    int colCount = gVal + 1;

    buf->set_font(&font, 0, 0, 0);
    *g_00691e6c = 0x1e;
    *g_00691e70 = 0x26;

    int rowCount = 5;
    int xBase = (colCount / 2) - (font.height_ / 2);
    int y = top + 2;

    int row = 0;
    for (;;) {
        if (row != 0) {
            if (row < 5) {
                int idx = row * 4 - 4;
                do {
                    if ((*reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e984) + idx + 4) !=
                         *reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e984) + idx)) ||
                        (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e920) + idx + 4) !=
                         *reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e920) + idx)) ||
                        (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e94c) + idx + 4) !=
                         *reinterpret_cast<int *>(reinterpret_cast<char *>(g_x90e94c) + idx)))
                        goto row_check;
                    idx += 4;
                    row += 1;
                } while (idx < 0x10);
            }
            goto loop_tail;
        }
    row_check:
        if (row >= 5) goto loop_tail;

        {
            *reinterpret_cast<char *>(g_009b86a0) = 0;
            int half3 = width / 2;
            int xEdge = half3 - 4;
            int x2 = right - xEdge - 2;

            int uVar3 = 0;
            switch (row) {
            case 0:
                if ((cfgPtr[7] == '\0') || (cfgPtr[5] == cfgPtr[4])) {
                    if (cfgPtr[8] == '\0') {
                        uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x508);
                    } else {
                        uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xf2c);
                    }
                } else {
                    uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0xf28);
                }
                break;
            case 1:
                uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x50c);
                break;
            case 2:
                uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x510);
                break;
            case 3: {
                int rowStride = *g_00689370;
                int idxOut = 0;
                int maskOut = 0;
                bitmask(0x23, &idxOut, &maskOut);
                unsigned char byteVal = g_x7d0cc[idxOut + rowStride * 0x134];
                byteVal = static_cast<unsigned char>(byteVal & static_cast<unsigned char>(maskOut));
                int boolMask = (byteVal != 0) ? 1 : 0;
                int tableVal = g_x96cc40[flagByte * 2099];
                int val2 = tableVal + boolMask * 2;
                int strIdx = ((val2 <= -2) ? 1 : 0) + 0x145;
                int finalVal = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + strIdx * 4);
                int strPtr = reinterpret_cast<Strings *>(g_009b90d8)->get(finalVal);
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(strPtr));
                goto after_switch;
            }
            case 4:
                uVar3 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_009b90f8) + 0x51c);
                break;
            }

            {
                int strPtr = reinterpret_cast<Strings *>(g_009b90d8)->get(uVar3);
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(strPtr));
            }
        after_switch:
            buf->set_text_color(*g_008c6d24, *g_008c6d28, 1, 1);

            char *msg = reinterpret_cast<char *>(g_009b86a0);
            if (msg) {
                unsigned int len = strlen(msg);
                int xCenter = xBase + y - 2;
                buf->write_cent_l(msg, x2, xCenter, y, len);
            }

            this->psych_row(left + 2, y, y, *g_00776438 + 1,
                                g_x90e984[row], g_x90e920[row], g_x90e94c[row]);
        }
    loop_tail:
        y += colCount;
        row += 1;
        rowCount -= 1;
        if (rowCount == 0) {
            *g_00691e6c = -1;
            *g_00691e70 = -1;
            buf->set_clip(&savedClip);
            return;
        }
    }
}
