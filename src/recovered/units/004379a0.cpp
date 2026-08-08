// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004379A0
// name           ?on_redraw@DesignWin@@QAEXXZ
// size           501 bytes
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004379a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@DesignWin@@QAEXXZ  at 0x004379A0  (501 bytes)
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
class DesignWin;
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
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
    void set_text_color(int, int, int, int);
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00669328 = (int *)0x00669328;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d618 = (int *)0x0078D618;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007cfe8c = (int *)0x007CFE8C;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_info();
    void draw_chassis(int);
    void draw_weapon(int);
    void draw_armor(int);
    void draw_reactor(int);
    void draw_special_1(int);
    void draw_special_2(int);
    void draw_vehicle(int);
    void check_abil();
    void draw_stack();
    void draw_flash();
    void on_redraw();
};

typedef char *(__stdcall *CharUpperAFn)(char *);
typedef int (__stdcall *UnionRectFn)(RECT *, const RECT *, const RECT *);

// MISMATCH, first divergence at mnemonic #11 (original 'mov ecx,edi' vs
// rebuilt 'lea ecx,[esi+0x444]'), 536 vs 501 bytes: REGISTER ALLOCATION /
// INSTRUCTION SCHEDULING wall. The embedded Buffer at self+0x444 (`buf`) is
// held live in edi across the whole function in the original and its
// address reused with a plain `mov`; the rebuilt recomputes it with `lea`
// at at least one use site instead of caching the register across the
// intervening calls. Tried both a `char *` and a typed `Buffer *` local for
// `buf` (both reached the identical divergence), so this is not a source-
// typing issue - ruled out. Every call, argument, and struct-field write
// below is otherwise faithful to the disassembly and to the Ghidra
// hypothesis (which drops `this` on every thiscall, as documented).
void DesignWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *screen = reinterpret_cast<Buffer *>(g_007aec64);

    screen->set_clip(reinterpret_cast<RECT *>(g_007aec94));

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_clip(reinterpret_cast<RECT *>(reinterpret_cast<char *>(buf) + 0x30));

    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    screen->box_sprite(reinterpret_cast<RECT *>(self + 0xa8c),
                        reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    RECT local_14 = *reinterpret_cast<RECT *>(self + 0xa6c);
    screen->box_sprite(&local_14, reinterpret_cast<BoxSpriteParams *>(g_0078d618));

    local_14.left += 3;
    local_14.top += 3;
    local_14.right -= 3;
    local_14.bottom -= 3;
    *reinterpret_cast<char *>(g_009b86a0) = 0;
    screen->set_font(reinterpret_cast<Font *>(g_007cfe8c), 0, 0, 0);

    strcat(reinterpret_cast<char *>(g_009b86a0),
           reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
               *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0xa70))));

    screen->set_text_color(0xf0, -1, 1, 1);

    (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
    if (reinterpret_cast<char *>(g_009b86a0)) {
        screen->write_cent_l(reinterpret_cast<char *>(g_009b86a0), &local_14,
                              strlen(reinterpret_cast<char *>(g_009b86a0)));
    }

    check_abil();
    draw_chassis(0);
    draw_weapon(0);
    draw_armor(0);
    draw_reactor(0);
    draw_special_1(0);
    draw_special_2(0);
    draw_vehicle(0);
    draw_info();
    draw_stack();
    draw_flash();

    buf->set_clip(reinterpret_cast<RECT *>(reinterpret_cast<char *>(buf) + 0x30));

    (*reinterpret_cast<UnionRectFn *>(g_00669328))(&local_14, reinterpret_cast<RECT *>(self + 0xbcc),
               reinterpret_cast<RECT *>(self + 0xb8c));
    (*reinterpret_cast<UnionRectFn *>(g_00669328))(&local_14, &local_14,
               reinterpret_cast<RECT *>(self + 0xb9c));

    local_14.left -= 3;
    local_14.top -= 3;
    local_14.right += 3;
    local_14.bottom += 3;
    buf->box_sprite(&local_14, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
}
