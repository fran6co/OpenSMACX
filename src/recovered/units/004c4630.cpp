// ORIGINAL: 0x004C4630 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C4630
// name           ?draw_box@WorldWin@@QAEXXZ
// size           439 bytes
// measured tier  MISMATCH
// divergence     29
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c4630/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_box@WorldWin@@QAEXXZ  at 0x004C4630  (439 bytes)
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
class WorldWin;

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
    int box(RECT *, int, int);
    void hline(int, int, int, int);
    void vline(int, int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00933454 = (int *)0x00933454;
static int *const g_00933458 = (int *)0x00933458;
static int *const g_0093345c = (int *)0x0093345C;
static int *const g_00933460 = (int *)0x00933460;
static int *const g_0093347c = (int *)0x0093347C;
static int *const g_00933480 = (int *)0x00933480;
static int *const g_00933484 = (int *)0x00933484;
static int *const g_00933488 = (int *)0x00933488;

class WorldWin { public:
    int tile_to_pixel(int, int, int *, int *);
    void draw_box();
};

void WorldWin::draw_box() {
    char *self = reinterpret_cast<char *>(this);

    if (*g_0093347c == 0) return;
    if (*g_0093345c != 0) return;
    if (*g_00933460 != 0) return;

    RECT rect;
    int r1 = tile_to_pixel(*g_00933454, *g_00933458, reinterpret_cast<int *>(&rect.left), reinterpret_cast<int *>(&rect.top));
    if (r1 != 0) return;

    int r2 = tile_to_pixel(*g_00933484 + *g_00933454 + *g_0093347c,
                            *g_00933488 + *g_00933480 + *g_00933458,
                            reinterpret_cast<int *>(&rect.right), reinterpret_cast<int *>(&rect.bottom));
    if (r2 != 0) return;

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    if (rect.left <= rect.right) {
        buf->box(&rect, 0xff, 0xff);
        return;
    }

    int a = rect.left;
    int b = rect.top;
    int c = rect.right;
    int d = rect.bottom;

    buf->hline(*reinterpret_cast<int *>(self + 0x239c4), c - 1, b, 0xff);
    buf->hline(*reinterpret_cast<int *>(self + 0x239c4), c - 2, d - 1, 0xff);
    buf->vline(c - 1, b + 1, d - 1, 0xff);

    int nightMode = *reinterpret_cast<int *>(self + 0x239c4);
    int x;
    if (nightMode != 0) {
        int *p1 = *reinterpret_cast<int **>(self + 0x152c);
        int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(p1) + 4);
        x = *reinterpret_cast<int *>(self + 0x19f0 + off) + nightMode - 1;
    } else {
        x = *reinterpret_cast<int *>(self + 0x4c4) - 1;
    }
    buf->hline(a + 1, x, b, 0xff);

    int nightMode2 = *reinterpret_cast<int *>(self + 0x239c4);
    int y;
    if (nightMode2 != 0) {
        int *p2 = *reinterpret_cast<int **>(self + 0x152c);
        int off2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(p2) + 4);
        y = *reinterpret_cast<int *>(self + 0x19f0 + off2) + nightMode2 - 1;
    } else {
        y = *reinterpret_cast<int *>(self + 0x4c4) - 1;
    }
    buf->hline(a, y, d - 1, 0xff);
    buf->vline(a, b, d - 2, 0xff);
}
