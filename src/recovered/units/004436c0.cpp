// ORIGINAL: 0x004436C0 FILE
// name      ?on_redraw@DiploWin@@QAEXXZ
// size      1381 bytes
// spans     0x004436C0-0x00443C25
// prototype void (__thiscall ?on_redraw@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00442A10 0x00442DC0 0x00443170 0x0045C290 0x005882F0 0x005DAC70 0x005DACB0 0x005DACE0 0x005DD130 0x005E2DD3 0x006169A0 0x006453E0 0x00645470
// indirect  0x00443773 0x00443784 0x004437D3 0x004437E8
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004436C0
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004436c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@DiploWin@@QAEXXZ  at 0x004436C0  (1381 bytes)
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
class MainInterface;
struct RECT;
class Spot;
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
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
    void set_text_color2(int, int, int, int);
};
class MainInterface { public:
    void restore_back(int);
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
static int *const g_00669328 = (int *)0x00669328;
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_006839fc = (int *)0x006839FC;
static int *const g_00683a04 = (int *)0x00683A04;
static int *const g_00683a10 = (int *)0x00683A10;
static int *const g_00683a14 = (int *)0x00683A14;
static int *const g_007492d4 = (int *)0x007492D4;
static int *const g_007492d8 = (int *)0x007492D8;
static int *const g_0078d528 = (int *)0x0078D528;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d618 = (int *)0x0078D618;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007af5a0 = (int *)0x007AF5A0;
static int *const g_007af5a4 = (int *)0x007AF5A4;
static int *const g_007af5a8 = (int *)0x007AF5A8;
static int *const g_007af5ac = (int *)0x007AF5AC;
static int *const g_007af5b0 = (int *)0x007AF5B0;
static int *const g_007af5b4 = (int *)0x007AF5B4;
static int *const g_007af5b8 = (int *)0x007AF5B8;
static int *const g_007af5bc = (int *)0x007AF5BC;
static int *const g_007d3930 = (int *)0x007D3930;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bc054 = (int *)0x009BC054;

class DiploWin { public:
    uint8_t pad_0_[0xA1C];
    uint32_t field_a1c_;

    void draw_who();
    void draw_whom();
    void draw_offers();
    void on_redraw();
};

// Indexed table base: the address itself does work (scaled index), so
// a fresh extern array is needed instead of the scaffold's
// const-pointer spelling.
extern unsigned int g_x96c9f8[];

void DiploWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    RECT r;

    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    r.left = *g_007af5b0 + 4;
    r.top = *g_007af5b4 + 4;
    r.right = *g_007af5b8 - 4;
    r.bottom = *g_007af5bc - 4;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    r.left = *g_007af5a0 + 4;
    r.top = *g_007af5a4 + 4;
    r.right = *g_007af5a8 - 4;
    r.bottom = *g_007af5ac - 4;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    typedef int(__stdcall * UnionRectFn)(RECT *, const RECT *, const RECT *);
    UnionRectFn unionRect = reinterpret_cast<UnionRectFn>(*reinterpret_cast<void **>(g_00669328));

    unionRect(&r, reinterpret_cast<RECT *>(self + 0xf5c), reinterpret_cast<RECT *>(self + 0xf6c));
    unionRect(&r, &r, reinterpret_cast<RECT *>(self + 0xf4c));
    r.left = r.left - 3;
    r.right = r.right + 3;
    r.bottom = r.bottom + 3;
    r.top = r.top - 3;

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    unionRect(&r, reinterpret_cast<RECT *>(self + 0xffc), reinterpret_cast<RECT *>(self + 0x100c));
    unionRect(&r, &r, reinterpret_cast<RECT *>(self + 0x101c));
    r.right = r.right + 3;
    r.left = r.left - 3;
    r.bottom = r.bottom + 3;
    r.top = r.top - 3;
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    r = *reinterpret_cast<RECT *>(self + 0xf9c);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d618));

    r.left = r.left + 3;
    r.right = r.right - 3;
    r.top = r.top + 3;
    r.bottom = r.bottom - 3;

    int fontId = reinterpret_cast<int(__cdecl *)(int, int, int, int, int)>(&find_font)(0xe, 1, 0, 0, 0);
    buf->set_font(reinterpret_cast<Font *>(fontId), 0, 0, 0);
    buf->set_text_color(0x99, -1, 1, 1);

    char *str1 = (*g_009bc054 == 0) ? reinterpret_cast<char *>(g_006839fc) : reinterpret_cast<char *>(g_007492d4);
    if (str1 != 0) {
        unsigned int len1 = strlen(str1);
        buf->write_cent_l(str1, &r, len1);
    }

    r = *reinterpret_cast<RECT *>(self + 0xfac);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    r = *reinterpret_cast<RECT *>(self + 0xfbc);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d618));
    r.left = r.left + 3;
    r.right = r.right - 3;
    r.top = r.top + 3;
    r.bottom = r.bottom - 3;

    char *str2 = (*g_009bc054 == 0) ? reinterpret_cast<char *>(g_00683a04) : reinterpret_cast<char *>(g_007492d8);
    if (str2 != 0) {
        unsigned int len2 = strlen(str2);
        buf->write_cent_l(str2, &r, len2);
    }

    r = *reinterpret_cast<RECT *>(self + 0xfcc);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    RECT *rf4c = reinterpret_cast<RECT *>(self + 0xf4c);
    r.top = *reinterpret_cast<int *>(self + 0xf50);
    r.left = rf4c->left;
    r.right = *reinterpret_cast<int *>(self + 0xf54);
    r.bottom = *reinterpret_cast<int *>(self + 0xf58);
    buf->box_sprite(&r, reinterpret_cast<BoxSpriteParams *>(g_0078d618));
    r.left = r.left + 3;
    r.right = r.right - 3;
    r.bottom = r.bottom - 3;
    r.top = r.top + 3;

    *reinterpret_cast<char *>(g_009b86a0) = 0;

    Strings *strs = reinterpret_cast<Strings *>(g_009b90d8);
    char *acc = reinterpret_cast<char *>(g_009b86a0);
    char *p90f8 = *reinterpret_cast<char **>(g_009b90f8);

    strcat(acc, reinterpret_cast<const char *>(strs->get(*reinterpret_cast<int *>(p90f8 + 0x748))));
    strcat(acc, reinterpret_cast<const char *>(g_00682820));
    strcat(acc, reinterpret_cast<const char *>(g_00682820));
    strcat(acc, reinterpret_cast<const char *>(g_00682e9c));

    strcat(acc, reinterpret_cast<const char *>(strs->get(*reinterpret_cast<int *>(p90f8 + 0x788))));
    strcat(acc, reinterpret_cast<const char *>(g_00682e94));
    strcat(acc, reinterpret_cast<const char *>(g_00683a10));

    unsigned int flags = g_x96c9f8[*reinterpret_cast<int *>(self + 0xab4) * 0x833 + *reinterpret_cast<int *>(self + 0xab8)];
    int colorCode;
    if ((flags & 1) != 0) {
        int fld = *reinterpret_cast<int *>(p90f8 + 0x34c);
        strcat(acc, reinterpret_cast<const char *>(strs->get(fld)));
        colorCode = 0xfa;
    } else if ((flags & 2) != 0) {
        int fld = *reinterpret_cast<int *>(p90f8 + 0x350);
        strcat(acc, reinterpret_cast<const char *>(strs->get(fld)));
        colorCode = 0xfa;
    } else if ((flags & 4) != 0) {
        int fld = *reinterpret_cast<int *>(p90f8 + 0x354);
        strcat(acc, reinterpret_cast<const char *>(strs->get(fld)));
        colorCode = 0xfa;
    } else if ((flags & 0x10) != 0) {
        int fld = *reinterpret_cast<int *>(p90f8 + 0x358);
        strcat(acc, reinterpret_cast<const char *>(strs->get(fld)));
        colorCode = 0xf9;
    } else {
        int fld = *reinterpret_cast<int *>(p90f8 + 0x784);
        strcat(acc, reinterpret_cast<const char *>(strs->get(fld)));
        colorCode = 0xfb;
    }
    buf->set_text_color2(colorCode, -1, 1, 1);

    strcat(acc, reinterpret_cast<const char *>(g_00683a14));
    strcat(acc, reinterpret_cast<const char *>(g_00682e98));

    if (acc != 0) {
        unsigned int len3 = strlen(acc);
        buf->write_cent_l(acc, &r, len3);
    }

    if (*g_007d3930 != 0) {
        reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(reinterpret_cast<RECT *>(self + 0xf3c), reinterpret_cast<BoxSpriteParams *>(g_0078d528));
        reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(reinterpret_cast<RECT *>(self + 0xf2c), reinterpret_cast<BoxSpriteParams *>(g_0078d528));
    }

    draw_who();
    draw_whom();
    draw_offers();
}
