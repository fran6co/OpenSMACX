// ORIGINAL: 0x00454270 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00454270
// name           ?load_vehicles@@YAXXZ
// size           1427 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00454270/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_vehicles@@YAXXZ  at 0x00454270  (1427 bytes)
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
    int get_pixel(int, int);
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
    Sprite();
    int extract(Buffer *, int, int, int, int, int, TexHeap *);
    void close();
};
void __cdecl load_flags();
void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));
void __stdcall fn_006457c2(void*, unsigned int, int, void (*)(void*), void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_005e37e0 = (int *)0x005E37E0;
static int *const g_00655178 = (int *)0x00655178;
static int *const g_006753c8 = (int *)0x006753C8;
static int *const g_0068503c = (int *)0x0068503C;
static int *const g_00685044 = (int *)0x00685044;
static int *const g_0068504c = (int *)0x0068504C;
static int *const g_00685058 = (int *)0x00685058;
static int *const g_0068f9f0 = (int *)0x0068F9F0;
static int *const g_0068fa10 = (int *)0x0068FA10;
static int *const g_0068fa30 = (int *)0x0068FA30;
static int *const g_0068fa50 = (int *)0x0068FA50;
static int *const g_007777a0 = (int *)0x007777A0;
static int *const g_00798668 = (int *)0x00798668;
static int *const g_007a9b30 = (int *)0x007A9B30;
static int *const g_007ab3f0 = (int *)0x007AB3F0;
static int *const g_007ab5c0 = (int *)0x007AB5C0;
static int *const g_007ac0c0 = (int *)0x007AC0C0;
static int *const g_0094a37e = (int *)0x0094A37E;
static int *const g_0094a38e = (int *)0x0094A38E;
static int *const g_0094a88e = (int *)0x0094A88E;

void __cdecl load_vehicles() {
    Sprite singleSprite;

    Buffer *buf = reinterpret_cast<Buffer *>(0x798668);

    buf->load_pcx(reinterpret_cast<const char *>(0x68503c), 0, 0, 0x12);
    buf->change_color(0xfd, 0xff);

    int spriteBase = 0x7a9b30;
    int y = 2;
    do {
        int x = 2;
        int n = 0x10;
        do {
            reinterpret_cast<Sprite *>(spriteBase)->extract(buf, 0xff, x, y, 0x2b, 0x2f, 0);
            x = x + 0x2d;
            spriteBase = spriteBase + 0x2c;
            n = n - 1;
        } while (n != 0);
        y = y + 0x31;
    } while (spriteBase < 0x7ab3f0);

    buf->load_pcx(reinterpret_cast<const char *>(0x685044), 0, 0, 0x12);
    buf->change_color(0xfd, 0xff);

    y = 2;
    int rowBase = 0x7ab5c0;
    do {
        int x = 2;
        int n = 8;
        do {
            reinterpret_cast<Sprite *>(rowBase)->extract(buf, 0xff, x, y, 100, 0x96, 0);
            reinterpret_cast<Sprite *>(rowBase + 0x98)->extract(buf, 0xff, x, y + 0x98, 100, 0x96, 0);
            x = x + 0x66;
            n = n - 1;
        } while (n != 0);
        y = y + 0x130;
        rowBase = rowBase + 0x2c0;
    } while (rowBase < 0x7ac0c0);

    buf->load_pcx(reinterpret_cast<const char *>(0x68504c), 0, 10, 0xec);
    int colX = 0x244;
    for (int i = 0; i < 8; ++i) {
        if (i == 0) {
            *reinterpret_cast<int *>(0x68f9f0) = buf->get_pixel(colX, 0x17);
            *reinterpret_cast<int *>(0x68fa10) = buf->get_pixel(colX, 0x1f);
            *reinterpret_cast<int *>(0x68fa30) = buf->get_pixel(colX, 0x27);
            *reinterpret_cast<int *>(0x68fa50) = buf->get_pixel(colX, 0x2f);
            colX = colX + 8;
        }
    }

    buf->load_pcx(reinterpret_cast<const char *>(0x685058), 0, 10, 0xec);

    int outer = -9;
    int flagBase = 0x7777a0;
    int byteBase = 0x94a38e;
    int rowY = -0x2b3;
    do {
        if (outer + 9 > 8) {
            int count = 1;
            if (rowY == 0xe9 || rowY == 0x4f || rowY == 0x136 || rowY == 0x183) {
                count = 7;
            } else if (rowY == 0x1d0) {
                count = 3;
            }
            if (count != 0) {
                int spriteThis = flagBase;
                unsigned char *byteWalk = reinterpret_cast<unsigned char *>(byteBase);
                int colBase = 2;
                int remain = count;
                int curY = rowY;
                do {
                    int width = colBase;
                    if (outer == 9) {
                        width = 0x2cc;
                        curY = 0xe9;
                    }
                    int startX = width;

                    buf->change_color(0x107, 0x109);
                    buf->change_color(0x106, 0x109);
                    reinterpret_cast<Sprite *>(spriteThis)->extract(buf, 0x109, startX, curY, 100, 0x4b, 0);

                    byteWalk[-8] = 0;
                    byteWalk[-0x10] = 0;
                    byteWalk[8] = 0;
                    byteWalk[0] = 0;
                    byteWalk[0x18] = 0x9d;
                    byteWalk[0x10] = 0x9d;
                    byteWalk[0x28] = 0x9d;
                    byteWalk[0x20] = 0x9d;

                    for (int r1 = 0; r1 < 100; ++r1) {
                        int yLo = curY - 2;
                        if (yLo <= curY - 1) {
                            int xCol = startX + r1;
                            int xLo = curY - 3;
                            do {
                                buf->get_pixel(xCol, xLo);
                                if (buf->get_pixel(xCol, yLo) == 5) {
                                    byteWalk[-0x10] = static_cast<unsigned char>(r1);
                                }
                                if (buf->get_pixel(xCol, yLo) == 2) {
                                    byteWalk[0] = static_cast<unsigned char>(r1);
                                }
                                yLo = yLo + 1;
                                xLo = xLo + 1;
                            } while (yLo <= curY - 1);
                        }
                    }

                    int bandLo = curY - 2;
                    int bandHi = curY - 1;
                    for (int r2 = 0; r2 < 0x4b; ++r2) {
                        if (bandLo <= bandHi) {
                            int rowV = r2 + curY;
                            int colV = bandLo;
                            do {
                                if (buf->get_pixel(colV, rowV) == 5) {
                                    byteWalk[-8] = static_cast<unsigned char>(r2);
                                }
                                if (buf->get_pixel(colV, rowV) == 2) {
                                    byteWalk[8] = static_cast<unsigned char>(r2);
                                }
                                colV = colV + 1;
                            } while (colV <= bandHi);
                        }
                    }

                    spriteThis = spriteThis + 0x2c;
                    colBase = colBase + 0x66;
                    byteWalk = byteWalk + 1;
                    remain = remain - 1;
                } while (remain != 0);
            }
        }
        int nextOuter = outer + 1;
        rowY = rowY + 0x4d;
        byteBase = byteBase + 0x90;
        flagBase = flagBase + 0x160;
        int checkVal = outer + 10;
        outer = nextOuter;
        if (checkVal >= 0x13)
            break;
    } while (true);

    int col1 = 2;
    unsigned char *ptrA = reinterpret_cast<unsigned char *>(0x94a37e);
    unsigned char *ptrB = reinterpret_cast<unsigned char *>(0x94a386);
    do {
        int colHi = col1;
        for (int r3 = 0; r3 < 100; ++r3) {
            int yTop = colHi - 1;
            if (yTop <= colHi) {
                int xCol = r3 + 0x396;
                int yLo = colHi - 2;
                do {
                    buf->get_pixel(xCol, yLo);
                    if (buf->get_pixel(xCol, yTop) == 5) {
                        *ptrA = static_cast<unsigned char>(r3);
                    }
                    yTop = yTop + 1;
                    yLo = yLo + 1;
                } while (yTop <= colHi);
            }
        }
        for (int r4 = 0; r4 < 0x4b; ++r4) {
            int xRow = 0x394;
            do {
                if (buf->get_pixel(xRow, colHi + 1 + r4) == 5) {
                    *ptrB = static_cast<unsigned char>(r4);
                }
                xRow = xRow + 1;
            } while (xRow < 0x396);
        }
        ptrA = ptrA + 0x90;
        col1 = col1 + 0x4d;
        ptrB = ptrB + 0x90;
    } while (reinterpret_cast<int>(ptrA) < 0x94a88e);

    load_flags();
    reinterpret_cast<Buffer *>(0x798668)->close();
    reinterpret_cast<Buffer *>(0x798668)->close();
    singleSprite.close();
}
