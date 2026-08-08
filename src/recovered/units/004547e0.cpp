// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004547E0
// name           ?load_icons@@YAXXZ
// size           1324 bytes
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004547e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_icons@@YAXXZ  at 0x004547E0  (1324 bytes)
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
    int change_color(int, int);
    int load_pcx(const char *, Palette *, int, int);
    void close();
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
static int *const g_00685064 = (int *)0x00685064;
static int *const g_00685074 = (int *)0x00685074;
static int *const g_00685084 = (int *)0x00685084;
static int *const g_0075ad88 = (int *)0x0075AD88;
static int *const g_0075ae20 = (int *)0x0075AE20;
static int *const g_0075aed0 = (int *)0x0075AED0;
static int *const g_0075af10 = (int *)0x0075AF10;
static int *const g_0075af68 = (int *)0x0075AF68;
static int *const g_0075af94 = (int *)0x0075AF94;
static int *const g_0075b450 = (int *)0x0075B450;
static int *const g_0075b710 = (int *)0x0075B710;
static int *const g_00776728 = (int *)0x00776728;
static int *const g_007796b0 = (int *)0x007796B0;
static int *const g_0078a520 = (int *)0x0078A520;
static int *const g_0078de30 = (int *)0x0078DE30;
static int *const g_0078e040 = (int *)0x0078E040;
static int *const g_00791d88 = (int *)0x00791D88;
static int *const g_00791f98 = (int *)0x00791F98;
static int *const g_00798668 = (int *)0x00798668;
static int *const g_0079a5e8 = (int *)0x0079A5E8;
static int *const g_007a72a0 = (int *)0x007A72A0;
static int *const g_007a7820 = (int *)0x007A7820;

void __cdecl load_icons() {
    Buffer *buf = reinterpret_cast<Buffer *>(g_00798668);

    buf->load_pcx(reinterpret_cast<const char *>(g_00685064), 0, 0xa, 0xec);

    int base1 = 0xd;
    Sprite *sprite = reinterpret_cast<Sprite *>(g_0075b450);
    for (; reinterpret_cast<int>(sprite) < 0x75b710; base1 += 0x17) {
        int icon1 = 1;
        for (int j = 0; j < 4; j++) {
            buf->change_color(0x107, 0x109);
            buf->change_color(0x106, 0x109);
            sprite->extract(buf, 0x109, icon1, base1, 0x16, 0x16, 0);
            icon1 += 0x17;
            sprite++;
        }
    }

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_0078a520)->extract(buf, 0x109, 1, 0xae, 0x16, 0x16, 0);

    sprite = reinterpret_cast<Sprite *>(g_0075af10);
    for (int icon2 = 0xdc; reinterpret_cast<int>(sprite) < 0x75af68; icon2 += 0x17) {
        buf->change_color(0x107, 0x109);
        buf->change_color(0x106, 0x109);
        sprite->extract(buf, 0x109, 1, icon2, 0x16, 0x16, 0);
        sprite++;
    }

    sprite = reinterpret_cast<Sprite *>(g_007a72a0);
    for (int base3 = 0x130; reinterpret_cast<int>(sprite) < 0x7a7820; base3 += 0x29) {
        int icon3 = 0xae;
        for (int j = 0; j < 8; j++) {
            buf->change_color(0x107, 0x109);
            buf->change_color(0x106, 0x109);
            sprite->extract(buf, 0x109, icon3, base3, 0x28, 0x28, 0);
            icon3 += 0x29;
            sprite++;
        }
    }

    buf->load_pcx(reinterpret_cast<const char *>(g_00685074), 0, 0xa, 0xec);
    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_00791f98)->extract(buf, 0x109, 0x300, 0x94, 0x1c, 0x2a, 0);

    buf->load_pcx(reinterpret_cast<const char *>(g_00685084), 0, 0xa, 0xec);
    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_00776728)->extract(buf, 0x109, 1, 1, 0x64, 0x32, 0);

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_0075ad88)->extract(buf, 0x109, 0x2e, 0x12f, 0x21, 0x1e, 0);

    sprite = reinterpret_cast<Sprite *>(g_00791d88);
    int base4 = 1;
    int icon4 = 0x68;
    do {
        for (int j = 0; j < 3; j++) {
            buf->change_color(0x107, 0x109);
            buf->change_color(0x106, 0x109);
            sprite->extract(buf, 0x109, icon4, base4, 0x18, 0x18, 0);
            icon4 += 0x19;
            sprite++;
        }
        base4 += 0x19;
        icon4 = 0x68;
    } while (reinterpret_cast<int>(sprite) < 0x791f98);

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_0079a5e8)->extract(buf, 0x109, icon4, 0x65, 0x12, 0x12, 0);

    sprite = reinterpret_cast<Sprite *>(g_0078de30);
    int base5 = 1;
    int icon5 = 0xc0;
    do {
        for (int j = 0; j < 3; j++) {
            buf->change_color(0x107, 0x109);
            buf->change_color(0x106, 0x109);
            sprite->extract(buf, 0x109, icon5, base5, 0xa, 0xa, 0);
            icon5 += 0xb;
            sprite++;
        }
        base5 += 0xb;
        icon5 = 0xc0;
    } while (reinterpret_cast<int>(sprite) < 0x78e040);

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_007796b0)->extract(buf, 0x109, 0xe1, 1, 0xa, 0xa, 0);

    sprite = reinterpret_cast<Sprite *>(g_0075ae20);
    for (int icon6 = 0x140; reinterpret_cast<int>(sprite) < 0x75aed0; icon6 += 0x10) {
        buf->change_color(0x107, 0x109);
        buf->change_color(0x106, 0x109);
        sprite->extract(buf, 0x109, icon6, 1, 0xf, 0xf, 0);
        sprite++;
    }

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_0075af68)->extract(buf, 0x109, 0x140, 0x14, 0x10, 0x10, 0);

    buf->change_color(0x107, 0x109);
    buf->change_color(0x106, 0x109);
    reinterpret_cast<Sprite *>(g_0075af94)->extract(buf, 0x109, 0x151, 0x14, 0x10, 0x10, 0);

    buf->close();
}
