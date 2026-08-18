// ORIGINAL: 0x00458FF0 ?timer_change@InfoWin@@QAEXH@Z 0x00458FF0-0x00459141 FILE
// size      337 bytes
// prototype void (__thiscall ?timer_change@InfoWin@@QAEXH@Z)(InfoWin* this, int)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00456300 0x00458420 0x00458DC0 0x005E2DD3
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00458FF0
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00458ff0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?timer_change@InfoWin@@QAEXH@Z  at 0x00458FF0  (337 bytes)
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
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class InfoWin;
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
    int box_sprite(RECT *, BoxSpriteParams *);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d7f8 = (int *)0x0078D7F8;
// Indexed by `*g_007d392c` (`[eax*4 + 0x7ae778]`) - the address itself
// does the work, so this is `extern int[]`, not a pointer constant, per
// the indexed-table-base lever.
extern int g_007ae778[];
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007af174 = (int *)0x007AF174;
static int *const g_007af178 = (int *)0x007AF178;
static int *const g_007af17c = (int *)0x007AF17C;
static int *const g_007af180 = (int *)0x007AF180;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_009b7b1c = (int *)0x009B7B1C;

class InfoWin { public:
    void setup_text();
    void draw_text(int);
    int state_valid(int);
    void timer_change(int);
};

void InfoWin::timer_change(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (g_007ae778[*g_007d392c] != 0) {
        return;
    }
    int orig_idx = *reinterpret_cast<int *>(self + 0x80);
    *reinterpret_cast<int *>(self + 0x94) = 0;
    bool first = true;
    if (*g_009b7b1c == 800) {
        *reinterpret_cast<int *>(self + 0x80) = 0;
    } else {
        *reinterpret_cast<unsigned int *>(self + 0x80) =
            (static_cast<unsigned int>(orig_idx) - 1) & 1;
    }
    for (;;) {
        if (*reinterpret_cast<int *>(self + 0x90) >= 0) {
            unsigned int v = (*reinterpret_cast<unsigned int *>(self + 0x90) + 1) &
                              0x80000007u;
            if (static_cast<int>(v) < 0) {
                v = ((v - 1) | 0xfffffff8u) + 1;
            }
            int slot = *reinterpret_cast<int *>(self + 0x80);
            *reinterpret_cast<unsigned int *>(self + 0x88 + slot * 4) = v;
        } else {
            int use_idx = first ? orig_idx : *reinterpret_cast<int *>(self + 0x80);
            unsigned int v =
                (*reinterpret_cast<unsigned int *>(self + 0x88 + use_idx * 4) + 1) &
                0x80000007u;
            if (static_cast<int>(v) < 0) {
                v = ((v - 1) | 0xfffffff8u) + 1;
            }
            int slot = *reinterpret_cast<int *>(self + 0x80);
            *reinterpret_cast<unsigned int *>(self + 0x88 + slot * 4) = v;
        }
        *reinterpret_cast<int *>(self + 0x90) = -1;
        first = false;
        int slot = *reinterpret_cast<int *>(self + 0x80);
        int cur = *reinterpret_cast<int *>(self + 0x88 + slot * 4);
        if (state_valid(cur) != 0) {
            break;
        }
    }
    *g_007af174 = 0;
    *g_007af17c = 0;
    *g_007af178 = 0;
    *g_007af180 = 0;
    RECT *rect = reinterpret_cast<RECT *>(
        self + (*reinterpret_cast<int *>(self + 0x80) + 1) * 16);
    *reinterpret_cast<int *>(self + 0x9c8) = 0;
    *reinterpret_cast<int *>(self + 0x9cc) = 0;
    *reinterpret_cast<int *>(self + 0xa8) = 0;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(
        rect, reinterpret_cast<BoxSpriteParams *>(g_0078d7f8));
    int *arr_ptr = reinterpret_cast<int *>(self + 0x88);
    if (arr_ptr != 0) {
        *reinterpret_cast<int *>(self + 0x9c8) = 0;
        *reinterpret_cast<int *>(self + 0x9cc) = 0;
    }
    setup_text();
    draw_text(*reinterpret_cast<int *>(self + 0x80));
}
