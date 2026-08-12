// ORIGINAL: 0x00442DC0 FILE
// name      ?draw_whom@DiploWin@@QAEXXZ
// size      905 bytes
// spans     0x00442DC0-0x00443149
// prototype void (__thiscall ?draw_whom@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   1   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005882F0 0x005DA4D0 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005E4B4A 0x006169A0 0x006453E0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00442DC0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00442dc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_whom@DiploWin@@QAEXXZ  at 0x00442DC0  (905 bytes)
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
struct BoxSpriteParams;
class Buffer;
class DiploWin;
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
class Strings;

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
    int box_sprite(RECT *, BoxSpriteParams *);
    int map_colors(RECT *, unsigned char *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
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
int __cdecl find_font(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0044314c = (int *)0x0044314C;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_0068fa30 = (int *)0x0068FA30;
static int *const g_0068fa50 = (int *)0x0068FA50;
static int *const g_006f107c = (int *)0x006F107C;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d618 = (int *)0x0078D618;
static int *const g_0078eecc = (int *)0x0078EECC;
static int *const g_0078eee8 = (int *)0x0078EEE8;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class DiploWin { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void draw_whom();
};

typedef char *(__stdcall *CharUpperFn)(char *);

extern char mz_00946a50[];
extern char mz_0078eecc[];

void DiploWin::draw_whom() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0xab4) <= 0) {
        return;
    }
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_font(reinterpret_cast<Font *>(find_font(0xe, 1)), 0, 0, 0);

    *reinterpret_cast<char *>(g_009b86a0) = 0;
    int idx = *reinterpret_cast<int *>(self + 0xab8);
    char *player = mz_00946a50 + idx * 0x59c;
    *g_009bbff0 = 0;
    *g_009bbfec = *reinterpret_cast<int *>(player);
    strcat(reinterpret_cast<char *>(g_009b86a0), player + 0x4c);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));

    idx = *reinterpret_cast<int *>(self + 0xab8);
    player = mz_00946a50 + idx * 0x59c;
    *g_009bbff0 = 0;
    *g_009bbfec = *reinterpret_cast<int *>(player);
    strcat(reinterpret_cast<char *>(g_009b86a0), player + 0x34);

    idx = *reinterpret_cast<int *>(self + 0xab8);
    buf->set_text_color(g_0068fa30[idx], g_0068fa50[idx], 1, 1);

    // The original reuses one 20-byte local slot for both rects in turn
    // (same ebp-relative offsets throughout); mirrored here with one `rect`.
    RECT rect = *reinterpret_cast<RECT *>(self + 0xf8c);
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d618));
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;
    (*reinterpret_cast<CharUpperFn *>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
    if (reinterpret_cast<char *>(g_009b86a0) != 0) {
        buf->write_cent_l(reinterpret_cast<char *>(g_009b86a0), &rect,
                           strlen(reinterpret_cast<char *>(g_009b86a0)));
    }

    rect = *reinterpret_cast<RECT *>(self + 0xf6c);
    buf->box_sprite(&rect, reinterpret_cast<BoxSpriteParams *>(g_0078d528));

    idx = *reinterpret_cast<int *>(self + 0xab8);
    Sprite *spr = reinterpret_cast<Sprite *>(mz_0078eecc + idx * 0x65c);
    int spriteHeight = spr->iHeight_;
    int top = rect.top + 1;
    int right = rect.right;
    int spriteWidth = spr->iWidth_;
    int left = right - spriteWidth - 1;
    rect.top = top;
    rect.left = left;
    rect.right = spriteWidth + left;
    rect.bottom = spriteHeight + top;
    Buffer *target = this ? buf : 0;
    int flag = static_cast<unsigned char>(spr->cTransparentIndex_);
    spr->draw(target, flag, left, top, 1, 1);

    rect.left += 4;
    rect.bottom -= 4;
    rect.right -= 4;
    rect.top = rect.bottom - 0x14;
    buf->set_font(reinterpret_cast<Font *>(find_font(0xe, 1)), 0, 0, 0);
    *reinterpret_cast<char *>(g_009b86a0) = 0;

    int result;
    switch (*reinterpret_cast<int *>(self + 0xa28)) {
    case 0:
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x73c);
        break;
    case 1:
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x740);
        break;
    case 2:
        buf->set_text_color(*g_0068fa30, *g_0068fa50, 1, 1);
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x744);
        break;
    case 3:
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x780);
        break;
    case 4:
        buf->set_text_color(*g_0068fa30, *g_0068fa50, 1, 1);
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x968);
        break;
    case 5:
        buf->set_text_color(*g_0068fa30, *g_0068fa50, 1, 1);
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x95c);
        break;
    case 6:
        buf->set_text_color(0xfb, 0, 1, 1);
        result = *reinterpret_cast<int *>(*g_009b90f8 + 0x960);
        break;
    default:
        goto after_switch;
    }
    result = reinterpret_cast<Strings *>(g_009b90d8)->get(result);
    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(result));

after_switch:
    (*reinterpret_cast<CharUpperFn *>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
    buf->map_colors(&rect, reinterpret_cast<unsigned char *>(g_006f107c));
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;
    if (reinterpret_cast<char *>(g_009b86a0) != 0) {
        buf->write_cent_l(reinterpret_cast<char *>(g_009b86a0), &rect,
                           strlen(reinterpret_cast<char *>(g_009b86a0)));
    }
}
