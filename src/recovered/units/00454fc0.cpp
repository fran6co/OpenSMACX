// ORIGINAL: 0x00454FC0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00454FC0
// name           ?load_cursors@@YAXXZ
// size           1075 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00454fc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_cursors@@YAXXZ  at 0x00454FC0  (1075 bytes)
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
class Sprite;
class TexHeap;

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
    int change_color(int, int);
    int get_pixel(int, int);
    int load_pcx(const char *, Palette *, int, int);
    void close();
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
    int extract(Buffer *, int, int, int, int, int, TexHeap *);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068511c = (int *)0x0068511C;
static int *const g_00685128 = (int *)0x00685128;
static int *const g_00685138 = (int *)0x00685138;
static int *const g_0075b180 = (int *)0x0075B180;
static int *const g_0075b230 = (int *)0x0075B230;
static int *const g_0075b710 = (int *)0x0075B710;
static int *const g_0075b844 = (int *)0x0075B844;
static int *const g_00776420 = (int *)0x00776420;
static int *const g_007764d0 = (int *)0x007764D0;
static int *const g_007765c0 = (int *)0x007765C0;
static int *const g_007766f4 = (int *)0x007766F4;
static int *const g_00779b78 = (int *)0x00779B78;
static int *const g_00779cac = (int *)0x00779CAC;
static int *const g_0078cad0 = (int *)0x0078CAD0;
static int *const g_0078cb80 = (int *)0x0078CB80;
static int *const g_00798668 = (int *)0x00798668;
static int *const g_00798cd0 = (int *)0x00798CD0;
static int *const g_00798e04 = (int *)0x00798E04;
static int *const g_007ad1b0 = (int *)0x007AD1B0;
static int *const g_007ad260 = (int *)0x007AD260;
static int *const g_0093aa70 = (int *)0x0093AA70;
static int *const g_0093ac80 = (int *)0x0093AC80;
static int *const g_009bc494 = (int *)0x009BC494;

extern int g_0093aa10[];
extern int g_0093aa40[];

void __cdecl load_cursors() {
    if (*g_009bc494 != 0) {
        return;
    }

    reinterpret_cast<Buffer *>(g_00798668)->load_pcx(
        reinterpret_cast<const char *>(g_0068511c), 0, 10, 0xec);

    int x = 1;
    char *cell = reinterpret_cast<char *>(g_0093aa70);
    int slot = 0;
    do {
        int hot_x = 0;
        for (int i = 0; i < 0x20; ++i) {
            if (reinterpret_cast<Buffer *>(g_00798668)->get_pixel(i + x, 0) == 5) {
                hot_x = i;
            }
        }
        int hot_y = 0;
        for (int j = 1; j < 0x20; ++j) {
            if (reinterpret_cast<Buffer *>(g_00798668)->get_pixel(x - 1, j) == 5) {
                hot_y = j - 1;
            }
        }
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 1, 0x20, 0x20, 0);
        cell += 0x2c;
        g_0093aa10[slot] = hot_x;
        g_0093aa40[slot] = hot_y;
        x += 0x21;
        ++slot;
    } while (cell < reinterpret_cast<char *>(g_0093ac80));

    reinterpret_cast<Buffer *>(g_00798668)->load_pcx(
        reinterpret_cast<const char *>(g_00685128), 0, 10, 0xec);

    x = 1;
    cell = reinterpret_cast<char *>(g_0078cad0);
    do {
        int n = 2;
        do {
            reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
            reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
            reinterpret_cast<Sprite *>(cell)->extract(
                reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x1f5, 0x26, 0x30, 0);
            x += 0x27;
            --n;
        } while (n != 0);
        cell += 0x2c;
    } while (cell < reinterpret_cast<char *>(g_0078cb80));

    cell = reinterpret_cast<char *>(g_007765c0);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x1f5, 0x26, 0x30, 0);
        cell += 0x2c;
        x += 0x27;
    } while (cell < reinterpret_cast<char *>(g_007766f4));

    x = 0x2c;
    cell = reinterpret_cast<char *>(g_00776420);
    do {
        int n = 2;
        do {
            reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
            reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
            reinterpret_cast<Sprite *>(cell)->extract(
                reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x1ca, 0x1e, 0x26, 0);
            x += 0x1f;
            --n;
        } while (n != 0);
        cell += 0x2c;
    } while (cell < reinterpret_cast<char *>(g_007764d0));

    cell = reinterpret_cast<char *>(g_00779b78);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x1ca, 0x1e, 0x26, 0);
        cell += 0x2c;
        x += 0x1f;
    } while (cell < reinterpret_cast<char *>(g_00779cac));

    reinterpret_cast<Buffer *>(g_00798668)->load_pcx(
        reinterpret_cast<const char *>(g_00685138), 0, 10, 0xec);

    x = 1;
    cell = reinterpret_cast<char *>(g_0075b180);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 1, 0x1e, 0x26, 0);
        cell += 0x2c;
        x += 0x1f;
    } while (cell < reinterpret_cast<char *>(g_0075b230));

    cell = reinterpret_cast<char *>(g_0075b710);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 1, 0x1e, 0x26, 0);
        cell += 0x2c;
        x += 0x1f;
    } while (cell < reinterpret_cast<char *>(g_0075b844));

    x = 1;
    cell = reinterpret_cast<char *>(g_007ad1b0);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x29, 0x26, 0x30, 0);
        cell += 0x2c;
        x += 0x27;
    } while (cell < reinterpret_cast<char *>(g_007ad260));

    cell = reinterpret_cast<char *>(g_00798cd0);
    do {
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x107, 0x109);
        reinterpret_cast<Buffer *>(g_00798668)->change_color(0x106, 0x109);
        reinterpret_cast<Sprite *>(cell)->extract(
            reinterpret_cast<Buffer *>(g_00798668), 0x109, x, 0x29, 0x26, 0x30, 0);
        cell += 0x2c;
        x += 0x27;
    } while (cell < reinterpret_cast<char *>(g_00798e04));

    reinterpret_cast<Buffer *>(g_00798668)->close();
}
