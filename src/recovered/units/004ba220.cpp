// ORIGINAL: 0x004BA220 BYTE_EXACT FILE
// name      ?textcolor_init@@YAXXZ
// size      936 bytes
// spans     0x004BA220-0x004BA5B3;0x00659BD0-0x00659BE5
// prototype 
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005E2210 0x005FE460
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BA220
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ba220/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?textcolor_init@@YAXXZ  at 0x004BA220  (936 bytes)
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
class Palette;
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
    int get_pixel(int, int);
    int load_pcx(const char *, Palette *, int, int);
    ~Buffer();
};
class Palette { public:
    int set();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659bdb = (int *)0x00659BDB;
static int *const g_00678e38 = (int *)0x00678E38;
static int *const g_00687504 = (int *)0x00687504;
static int *const g_008c6b60 = (int *)0x008C6B60;
static int *const g_008c6b90 = (int *)0x008C6B90;
static int *const g_008c6bd8 = (int *)0x008C6BD8;
static int *const g_008c6bf8 = (int *)0x008C6BF8;
static int *const g_008c6c28 = (int *)0x008C6C28;
static int *const g_008c6c70 = (int *)0x008C6C70;
static int *const g_008c6ca8 = (int *)0x008C6CA8;
static int *const g_008c6d90 = (int *)0x008C6D90;
static int *const g_008c6de8 = (int *)0x008C6DE8;
static int *const g_008c6e08 = (int *)0x008C6E08;
static int *const g_008c6e30 = (int *)0x008C6E30;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_0094c590 = (int *)0x0094C590;

// EXTERN-SYMBOL LEVER: every table base/bound below is walked with pointer
// arithmetic and compared against another table's address, so the address
// itself does work; `extern int name[]` keeps the indexed-store form instead
// of folding to a bare literal the way the context's `static int *const`
// spelling would under /O2.
extern int g_008c6b60_tbl[];
extern int g_008c6b90_tbl[];
extern int g_008c6bd8_tbl[];
extern int g_008c6bf8_tbl[];
extern int g_008c6c28_tbl[];
extern int g_008c6c70_tbl[];
extern int g_008c6ca8_tbl[];
extern int g_008c6d90_tbl[];
extern int g_008c6de8_tbl[];
extern int g_008c6e08_tbl[];
extern int g_008c6e30_tbl[];
extern int g_008c6e68_tbl[];

void __cdecl textcolor_init() {
    Buffer buf;
    int c;
    int *p;
    int y;

    buf.load_pcx(reinterpret_cast<const char *>(g_00687504), 0, 0xa, 0xec);
    reinterpret_cast<Palette *>(g_0094c590)->set();

    y = 0x15;
    p = g_008c6b90_tbl;
    do {
        c = buf.get_pixel(8, y);
        p[-1] = c;
        c = buf.get_pixel(0x12, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6bd8_tbl);

    y = 0x24;
    p = g_008c6ca8_tbl;
    do {
        c = buf.get_pixel(0xcd, y);
        p[-1] = c;
        c = buf.get_pixel(0xd7, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6d90_tbl);

    y = 0x24;
    p = g_008c6d90_tbl;
    do {
        c = buf.get_pixel(0x173, y);
        p[-1] = c;
        c = buf.get_pixel(0x17d, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6de8_tbl);

    y = 0x24;
    p = g_008c6e30_tbl;
    do {
        c = buf.get_pixel(0x234, y);
        p[-1] = c;
        c = buf.get_pixel(0x23e, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6e68_tbl);

    y = 0xb0;
    p = g_008c6c70_tbl;
    do {
        c = buf.get_pixel(0x232, y);
        p[-1] = c;
        c = buf.get_pixel(0x23c, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6ca8_tbl);

    y = 0x25;
    p = g_008c6de8_tbl;
    do {
        c = buf.get_pixel(0x2ce, y);
        p[-1] = c;
        c = buf.get_pixel(0x2d8, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6e08_tbl);

    y = 0xbe;
    p = g_008c6bd8_tbl;
    do {
        c = buf.get_pixel(7, y);
        p[-1] = c;
        c = buf.get_pixel(0x11, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6bf8_tbl);

    y = 0x126;
    p = g_008c6bf8_tbl;
    do {
        c = buf.get_pixel(6, y);
        p[-1] = c;
        c = buf.get_pixel(0x10, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6c28_tbl);

    y = 0x11e;
    p = g_008c6e08_tbl;
    do {
        c = buf.get_pixel(0x175, y);
        p[-1] = c;
        c = buf.get_pixel(0x17f, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6e30_tbl);

    y = 0x19e;
    p = g_008c6b60_tbl;
    do {
        c = buf.get_pixel(0x175, y);
        p[-1] = c;
        c = buf.get_pixel(0x17f, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)g_008c6b90_tbl);

    y = 0x1a4;
    p = g_008c6c28_tbl;
    do {
        c = buf.get_pixel(8, y);
        p[-1] = c;
        c = buf.get_pixel(0x12, y);
        p[0] = c;
        c = buf.get_pixel(0x1c, y);
        p[1] = c;
        p += 3;
        y += 0x10;
    } while ((int)p < (int)g_008c6c70_tbl);
}
