// ORIGINAL: 0x004379A0 ?on_redraw@DesignWin@@QAEXXZ 0x004379A0-0x00437B95 FILE
// size      501 bytes
// prototype void (__thiscall ?on_redraw@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004346A0 0x00434BC0 0x00434F40 0x00435400 0x00435800 0x00435B30 0x00435DE0 0x00436090 0x00439C00 0x0043D510 0x0043DD50 0x0045C290 0x005D8000 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x006169A0 0x006453E0 0x00645470
// indirect  0x00437A9C 0x00437B46 0x00437B57
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004379A0
// measured tier  MISMATCH
// divergence     9
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_clip(RECT *);
    int set_font(Font *, Font *, Font *, Font *);
    int write_cent_l(char *, RECT *, int);
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
// Underscore-prefixed, not the scaffold's bare `strcat`/`strlen` above: the
// catalogue's name for both call targets (0x00645470, 0x006453E0) is
// `_strcat`/`_strlen`, matching the disassembly's own comments. The bare
// ANSI names are VC6-recognized intrinsics and get expanded inline
// (`repne scasb` + `rep movsd/movsb`) instead of calling out, which also
// perturbs register allocation for the whole function - measured: fixing
// the name alone put `this` back in esi to match the original.
extern "C" char *_strcat(char *, const char *);
extern "C" unsigned int _strlen(const char *);
// Extern, not the fixed-address pointer constant above: the guard below
// tests this buffer's own address for non-null, and a literal `(char*)0xADDR`
// is a compile-time-nonzero constant that /O2 folds the test away for -
// exactly the "guard vanished entirely" failure mode. An extern symbol needs
// a relocation, so the compiler cannot fold it and the branch survives.
extern char g_009b86a0_arr[];

// MISMATCH, first divergence at mnemonic #9 (original 'call' vs rebuilt
// 'lea'), 501 vs 508 bytes: INSTRUCTION SCHEDULING wall. The rebuilt hoists
// `lea edi, [esi+0x444]` (computing the second Buffer, dead until after the
// first set_clip call) one call earlier than the original, which computes it
// only once the first set_clip has returned; every call target, argument,
// and register role from that point on lines up exactly, just shifted by
// this one reordering (confirmed instruction-by-instruction). Every
// box_sprite/set_clip/set_font/set_text_color/write_cent_l argument order,
// the two UnionRect calls and their argument roles (dst/src1/src2), the
// pre- and post-3-pixel RECT adjustments (shrink before the font draws,
// grow after), and the eleven draw_* dispatch calls are otherwise faithful
// to the disassembly.
void DesignWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(g_007aec64);
    Buffer *buf2 = reinterpret_cast<Buffer *>(self + 0x444);

    buf->set_clip(reinterpret_cast<RECT *>(g_007aec94));
    buf2->set_clip(reinterpret_cast<RECT *>(self + 0x444 + 0x30));
    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    buf->box_sprite(reinterpret_cast<RECT *>(self + 0xa8c),
                     reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    RECT local;
    int *src = reinterpret_cast<int *>(self + 0xa6c);
    local.left = src[0];
    local.top = src[1];
    local.right = src[2];
    local.bottom = src[3];
    buf->box_sprite(&local, reinterpret_cast<BoxSpriteParams *>(g_0078d618));

    local.left += 3;
    local.top += 3;
    local.right -= 3;
    local.bottom -= 3;

    char *msg = g_009b86a0_arr;
    *msg = 0;
    buf->set_font(reinterpret_cast<Font *>(g_007cfe8c), 0, 0, 0);

    int base = *reinterpret_cast<int *>(g_009b90f8);
    int arg = *reinterpret_cast<int *>(base + 0xa70);
    char *piece = reinterpret_cast<char *>(
        reinterpret_cast<Strings *>(g_009b90d8)->get(arg));
    _strcat(msg, piece);

    buf->set_text_color(0xf0, -1, 1, 1);
    (*reinterpret_cast<CharUpperAFn *>(g_0066931c))(msg);
    if (msg) {
        buf->write_cent_l(msg, &local, _strlen(msg));
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

    buf2->set_clip(reinterpret_cast<RECT *>(self + 0x444 + 0x30));

    UnionRectFn union_rect = *reinterpret_cast<UnionRectFn *>(g_00669328);
    union_rect(&local, reinterpret_cast<RECT *>(self + 0xbcc),
               reinterpret_cast<RECT *>(self + 0xb8c));
    union_rect(&local, &local, reinterpret_cast<RECT *>(self + 0xb9c));

    local.left -= 3;
    local.top -= 3;
    local.right += 3;
    local.bottom += 3;
    buf2->box_sprite(&local, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));
}
