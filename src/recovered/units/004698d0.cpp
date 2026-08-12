// ORIGINAL: 0x004698D0 FILE
// name      ?draw_wars@MapWin@@QAEXXZ
// size      610 bytes
// spans     0x004698D0-0x00469B32
// prototype void (__thiscall ?draw_wars@MapWin@@QAEXXZ)(MapWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DAC70 0x005DACB0 0x005DCEA0 0x00619280 0x006453E0 0x00645470
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004698D0
// measured tier  NO_COMPILE
// refusal        u004698d0.cpp(191) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004698d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_wars@MapWin@@QAEXXZ  at 0x004698D0  (610 bytes)
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
    int set_font(Font *, Font *, Font *, Font *);
    int write_l(char *, int, int, int);
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
    int width(char *);
};
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_0068637c = (int *)0x0068637C;
static int *const g_0068f9f4 = (int *)0x0068F9F4;
static int *const g_0068fa10 = (int *)0x0068FA10;
static int *const g_009472ec = (int *)0x009472EC;
static int *const g_00949a30 = (int *)0x00949A30;
static int *const g_0096eac8 = (int *)0x0096EAC8;
static int *const g_0097d05c = (int *)0x0097D05C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b86a1 = (int *)0x009B86A1;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;

    void draw_wars();
};

// EXTERN-SYMBOL LEVER: these three tables are walked with a pointer that
// advances by a fixed stride (not a plain load/store, and the loop bound
// is a raw address compare too), so `static int *const` would let /O2
// fold the address literal and drop the addressing the walk needs.
extern int g_0068f9f4_arr[];   // per-faction color, stride 1 int (4 bytes)
extern int g_009472ec_arr[];   // per-faction record, stride 0x167 ints (0x59c bytes)
extern int g_0096eac8_arr[];  // per-row flags, stride 0x833 ints (0x20cc bytes)

// The `test edx,edx; je ...` after `mov edx, 0x9b86a0` tests the ADDRESS
// of this scratch buffer, not its contents. A `static int *const` would
// let /O2 fold the always-true test away entirely.
extern char g_009b86a0_buf[];

void MapWin::draw_wars() {
    char *self = reinterpret_cast<char *>(this);
    Font *font = reinterpret_cast<Font *>(self + 0x1eef4);
    int charWidth = font->width(reinterpret_cast<char *>(g_0068637c));

    int y = (font->unk_1_ < 0) ? font->line_height_ : (font->height_ + font->unk_1_);

    Buffer *buf = reinterpret_cast<Buffer *>(
        self + 0x444 + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 4));
    buf->set_font(font, 0, 0, 0);

    int *colorPtr = g_0068f9f4_arr;
    int *recPtr = g_009472ec_arr;
    int maxWidth = charWidth;

    char lastLetter;

    do {
        *g_009bbfec = *(recPtr - 1);
        *g_009bbff0 = *recPtr;
        g_009b86a0_buf[0] = 0;
        strcat(g_009b86a0_buf, reinterpret_cast<char *>(recPtr - 7));
        strcat(g_009b86a0_buf, reinterpret_cast<char *>(g_00682e94));

        buf = reinterpret_cast<Buffer *>(
            self + 0x444 + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 4));
        buf->set_text_color(*colorPtr, 0, 1, 1);

        unsigned int len = strlen(g_009b86a0_buf);
        int w = buf->write_l(g_009b86a0_buf, charWidth, y, len);
        if (maxWidth <= w) {
            maxWidth = w;
        }

        int lineAdv = (font->unk_1_ < 0) ? font->line_height_ : (font->height_ + font->unk_1_);
        recPtr += 0x167;
        y += lineAdv;
        colorPtr += 1;
    } while (recPtr < g_00949a30);

    y = (font->unk_1_ < 0) ? font->line_height_ : (font->height_ + font->unk_1_);

    int *rowFlags = g_0096eac8_arr;
    do {
        int x = maxWidth;
        colorPtr = g_0068f9f4_arr;
        int *flags = rowFlags;
        do {
            buf = reinterpret_cast<Buffer *>(
                self + 0x444 + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 4));
            buf->set_text_color(*colorPtr, 0, 1, 1);

            unsigned int f = static_cast<unsigned int>(*flags);
            if ((f & 8) == 0) {
                lastLetter = '-';
            } else if ((f & 0x10) != 0) {
                lastLetter = 'W';
            } else if ((f & 1) != 0) {
                lastLetter = 'P';
            } else if ((f & 2) != 0) {
                lastLetter = 'T';
            } else if ((f & 4) != 0) {
                lastLetter = 't';
            }

            g_009b86a0_buf[1] = 0;
            g_009b86a0_buf[0] = lastLetter;

            buf = reinterpret_cast<Buffer *>(
                self + 0x444 + *reinterpret_cast<int *>(*reinterpret_cast<int *>(self) + 4));
            if (g_009b86a0_buf) {
                unsigned int len = strlen(g_009b86a0_buf);
                buf->write_l(g_009b86a0_buf, x, y, len);
            }

            colorPtr += 1;
            x += charWidth;
            flags += 1;
        } while (colorPtr < g_0068fa10);

        int lineAdv = (font->unk_1_ < 0) ? font->line_height_ : (font->height_ + font->unk_1_);
        y += lineAdv;
        rowFlags += 0x833;
    } while (rowFlags < g_0097d05c);
}
