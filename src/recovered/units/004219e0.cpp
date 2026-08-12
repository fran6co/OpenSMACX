// ORIGINAL: 0x004219E0 FILE
// name      ?init@BattleWin@@QAEXXZ
// size      319 bytes
// spans     0x004219E0-0x00421B1F
// prototype void (__thiscall ?init@BattleWin@@QAEXXZ)(BattleWin* this)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004219E0
// measured tier  MISMATCH
// divergence     17
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004219e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@BattleWin@@QAEXXZ  at 0x004219E0  (319 bytes)
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
class BattleWin;
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
    Buffer();
    ~Buffer();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007ae818 = (int *)0x007AE818;
static int *const g_007af5a0 = (int *)0x007AF5A0;
static int *const g_007af5a4 = (int *)0x007AF5A4;
static int *const g_007af5a8 = (int *)0x007AF5A8;
static int *const g_007af5ac = (int *)0x007AF5AC;
static int *const g_007af5e0 = (int *)0x007AF5E0;
static int *const g_007af5e4 = (int *)0x007AF5E4;
static int *const g_007af5e8 = (int *)0x007AF5E8;
static int *const g_007af5ec = (int *)0x007AF5EC;

class BattleWin { public:
    uint8_t unmapped_0_[0x30];
    uint8_t field_30_[0x10];
    uint8_t field_40_[0x10];
    uint8_t field_50_[0x10];
    uint8_t field_60_[0x10];
    uint8_t field_70_[0x10];
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint8_t field_90_[0x10];

    void init();
};

void BattleWin::init() {
    char *self = reinterpret_cast<char *>(this);

    Buffer buf;

    *reinterpret_cast<RECT *>(self + 0x70) = *reinterpret_cast<RECT *>(g_007af5a0);
    *reinterpret_cast<RECT *>(self + 0x90) = *reinterpret_cast<RECT *>(g_007af5e0);

    int left1 = *g_007af5e0 + 4;
    int top1 = *g_007af5e4 + 4;
    int right1 = (*g_007af5e8 - left1 - 1) / 2 - 3 + left1;
    int bottom1 = *g_007af5ec - 4;
    RECT *r30 = reinterpret_cast<RECT *>(self + 0x30);
    r30->left = left1;
    r30->top = top1;
    r30->right = right1;
    r30->bottom = bottom1;

    int delta1 = (right1 - left1) + 3;
    RECT *r40 = reinterpret_cast<RECT *>(self + 0x40);
    r40->left = left1 + delta1;
    r40->top = top1;
    r40->right = right1 + delta1;
    r40->bottom = bottom1;

    int left2 = *g_007af5a0 + 4;
    int top2 = *g_007af5a4 + 4;
    int right2 = (*g_007af5a8 - left2 - 1) / 2 - 3 + left2;
    int bottom2 = *g_007af5ac - 4;
    RECT *r50 = reinterpret_cast<RECT *>(self + 0x50);
    r50->left = left2;
    r50->top = top2;
    r50->right = right2;
    r50->bottom = bottom2;

    *reinterpret_cast<int *>(self + 4) = 10;

    RECT *r60 = reinterpret_cast<RECT *>(self + 0x60);
    r60->left = right2 + 3;
    r60->top = top2;
    r60->right = *g_007af5a8 - 4;
    r60->bottom = bottom2;

    *g_007ae818 = reinterpret_cast<int>(this);
}
