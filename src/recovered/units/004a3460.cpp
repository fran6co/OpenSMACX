// ORIGINAL: 0x004A3460 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004A3460
// name           ?init@ReportIf@@QAEXXZ
// size           1244 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a3460/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@ReportIf@@QAEXXZ  at 0x004A3460  (1244 bytes)
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
class ReportIf;
class ReportWin;
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
    void init(int);
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
    Buffer();
    int load_pcx(const char *, Palette *, int, int);
    ~Buffer();
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

class ReportWin { public:
    void init();
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

extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
int find_font(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065958b = (int *)0x0065958B;
static int *const g_00678988 = (int *)0x00678988;
static int *const g_00687268 = (int *)0x00687268;
static int *const g_0068726c = (int *)0x0068726C;
static int *const g_00687274 = (int *)0x00687274;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_0078f554 = (int *)0x0078F554;
static int *const g_0078f5d8 = (int *)0x0078F5D8;
static int *const g_007ae804 = (int *)0x007AE804;
static int *const g_007af5a0 = (int *)0x007AF5A0;
static int *const g_007af5a4 = (int *)0x007AF5A4;
static int *const g_007af5a8 = (int *)0x007AF5A8;
static int *const g_007af5ac = (int *)0x007AF5AC;
static int *const g_007af5b0 = (int *)0x007AF5B0;
static int *const g_007af5b4 = (int *)0x007AF5B4;
static int *const g_007af5b8 = (int *)0x007AF5B8;
static int *const g_007af5bc = (int *)0x007AF5BC;
static int *const g_007af5c0 = (int *)0x007AF5C0;
static int *const g_007af5c4 = (int *)0x007AF5C4;
static int *const g_007af5c8 = (int *)0x007AF5C8;
static int *const g_007af5cc = (int *)0x007AF5CC;
static int *const g_007af5d0 = (int *)0x007AF5D0;
static int *const g_007af5d4 = (int *)0x007AF5D4;
static int *const g_007af5d8 = (int *)0x007AF5D8;
static int *const g_007af5dc = (int *)0x007AF5DC;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_0087be80 = (int *)0x0087BE80;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b86a0 = (int *)0x009B86A0;

class ReportIf { public:
    void init();
};
extern "C" char *__cdecl my_itoa(int, char *, int);
extern char tbl_78f554[];
extern char tbl_78f5d8[];


void ReportIf::init() {
    char *self = reinterpret_cast<char *>(this);
    Buffer localBuffer;

    *reinterpret_cast<int *>(self + 0x1e240) = 0;
    *reinterpret_cast<int *>(self + 0x1e244) = 4;
    *reinterpret_cast<int *>(self + 4) = 5;
    *g_007ae804 = reinterpret_cast<int>(self);

    reinterpret_cast<Spot *>(self + 0x1e1bc)->init(0x100);

    int *recTable = reinterpret_cast<int *>(self + 0x1e258);
    for (int i = 0; i < 0x200; ++i) {
        recTable[-1] = i;
        recTable[0] = 0;
        recTable[1] = i;
        recTable[2] = 0;
        recTable += 4;
    }

    *g_0087be80 = 0;
    Sprite *sprites4 = reinterpret_cast<Sprite *>(self + 0x20254);
    char *strBuf = reinterpret_cast<char *>(g_009b86a0);
    for (int j = 0; j < 4; ++j) {
        *strBuf = 0;
        strcat(strBuf, reinterpret_cast<char *>(g_00687268));
        char numBuf[80];
        my_itoa(j, numBuf, 10);
        strcat(strBuf, numBuf);
        strcat(strBuf, reinterpret_cast<char *>(g_0068726c));
        localBuffer.load_pcx(strBuf, 0, 10, 0xec);
        sprites4[j].extract(&localBuffer, *reinterpret_cast<unsigned char *>(g_00696d14),
                            0, 0, 0xc, 0xc, 0);
    }

    localBuffer.load_pcx(reinterpret_cast<char *>(g_00687274), 0, 10, 0xec);

    Sprite *singleSprite = reinterpret_cast<Sprite *>(self + 0x4f94);
    int y = 0;
    int rowCount = 7;
    do {
        int x = 0;
        int colCount = 3;
        do {
            singleSprite->extract(&localBuffer, *reinterpret_cast<unsigned char *>(g_00696d14),
                                  y, x, 0x1e, 0x24, 0);
            x += 0x23;
            colCount--;
        } while (colCount != 0);
        y += 0x1d;
        rowCount--;
    } while (rowCount != 0);

    *reinterpret_cast<int *>(self + 0x20) = *g_007af5a0;
    *reinterpret_cast<int *>(self + 0x24) = *g_007af5a4;
    *reinterpret_cast<int *>(self + 0x28) = *g_007af5a8;
    *reinterpret_cast<int *>(self + 0x2c) = *g_007af5ac;
    *reinterpret_cast<int *>(self + 0x30) = *g_007af5b0;
    *reinterpret_cast<int *>(self + 0x34) = *g_007af5b4;
    *reinterpret_cast<int *>(self + 0x38) = *g_007af5b8;
    *reinterpret_cast<int *>(self + 0x3c) = *g_007af5bc;
    *reinterpret_cast<int *>(self + 0x40) = *g_007af5d0;
    *reinterpret_cast<int *>(self + 0x44) = *g_007af5d4;
    *reinterpret_cast<int *>(self + 0x48) = *g_007af5d8;
    *reinterpret_cast<int *>(self + 0x4c) = *g_007af5dc;
    *reinterpret_cast<int *>(self + 0x50) = *g_007af5c0;
    *reinterpret_cast<int *>(self + 0x54) = *g_007af5c4;
    *reinterpret_cast<int *>(self + 0x58) = *g_007af5c8;
    *reinterpret_cast<int *>(self + 0x5c) = *g_007af5cc;

    bool wide = (*g_009b7b1c == 800);
    if (wide) {
        *reinterpret_cast<int *>(self + 100) = 0x25;
        *reinterpret_cast<int *>(self + 0x60) = 0;
        *reinterpret_cast<int *>(self + 0x68) = 0x4a;
        *reinterpret_cast<int *>(self + 0x6c) = 0x6f;
        *reinterpret_cast<int *>(self + 0x70) = 0x94;
        *reinterpret_cast<int *>(self + 0x74) = 0xb9;
        *reinterpret_cast<int *>(self + 0x78) = 0xde;
        *reinterpret_cast<int *>(self + 0x1e230) = 0;
    } else {
        *reinterpret_cast<int *>(self + 0x60) = 0;
        *reinterpret_cast<int *>(self + 100) = 0x2a;
        *reinterpret_cast<int *>(self + 0x68) = 0x54;
        *reinterpret_cast<int *>(self + 0x6c) = 0x7e;
        *reinterpret_cast<int *>(self + 0x70) = 0xa8;
        *reinterpret_cast<int *>(self + 0x74) = 0xd2;
        *reinterpret_cast<int *>(self + 0x78) = 0xfc;
        *reinterpret_cast<int *>(self + 0x1e230) = 1;
    }

    int col = 0;
    int base = 0;
    int outerIdx = 0x14cc;
    do {
        int inner = 0;
        col++;
        do {
            char *ptr;
            if (col == *g_00939284 && *reinterpret_cast<int *>(self + 0x1e230) != 0) {
                ptr = tbl_78f5d8 + ((inner == 0 ? 1 : 0) + base) * 0x2c;
            } else {
                ptr = tbl_78f554 + (base + inner) * 0x2c;
            }
            int idx = outerIdx + inner;
            inner++;
            *reinterpret_cast<char **>(self + idx * 4) = ptr;
        } while (inner < 3);
        outerIdx += 3;
        base += 0x25;
    } while (outerIdx < 0x14e1);

    int wideFlag = *reinterpret_cast<int *>(self + 0x1e230);
    int y0 = (wideFlag != 0 ? 1 : 0) + 0x10 + *reinterpret_cast<int *>(self + 0x40);
    int rows = 7;
    *reinterpret_cast<int *>(self + 0xa2cc) =
        ((*reinterpret_cast<int *>(self + 0x4c) + *reinterpret_cast<int *>(self + 0x44)) -
         *reinterpret_cast<int *>(self + 0x4fb0)) / 2;
    int step = *reinterpret_cast<int *>(self + 0x4fac);
    int *row = reinterpret_cast<int *>(self + 0xa298);
    do {
        *row = y0;
        y0 += (wideFlag != 0 ? 3 : 0) + 9 + step;
        row++;
        rows--;
    } while (rows != 0);

    int n2 = 8;
    int *slots = reinterpret_cast<int *>(self + 0x60);
    do {
        n2--;
        *slots = *slots + (*reinterpret_cast<int *>(self + 0x1e230) != 0 ? 0x2b : 0) + 0x2e +
                 *reinterpret_cast<int *>(self + 0x50);
        slots++;
    } while (n2 != 0);

    *reinterpret_cast<int *>(self + 0x7c) = *reinterpret_cast<int *>(self + 0x5c) - 0x28;

    *reinterpret_cast<int *>(self + 0x1e180) = find_font(10, 0);
    *reinterpret_cast<int *>(self + 0x1e184) = find_font(0xc, 0);
    *reinterpret_cast<int *>(self + 0x1e188) = find_font(0xc, 2);
    *reinterpret_cast<int *>(self + 0x1e18c) = find_font(0xc, 1);
    *reinterpret_cast<int *>(self + 0x1e190) = find_font(0xe, 0);
    *reinterpret_cast<int *>(self + 0x1e194) = find_font(0xe, 2);
    *reinterpret_cast<int *>(self + 0x1e198) = find_font(0xe, 1);
    *reinterpret_cast<int *>(self + 0x1e19c) = find_font(0x10, 0);
    *reinterpret_cast<int *>(self + 0x1e1a0) = find_font(0x10, 2);
    *reinterpret_cast<int *>(self + 0x1e1a4) = find_font(0x10, 1);
    *reinterpret_cast<int *>(self + 0x1e1a8) = find_font(0xb, 0);
    *reinterpret_cast<int *>(self + 0x1e1ac) = find_font(0x1a, 0);
    *reinterpret_cast<int *>(self + 0x1e1b0) = find_font(0x12, 2);
    *reinterpret_cast<int *>(self + 0x1e1b4) = find_font(0x16, 0);
    *reinterpret_cast<int *>(self + 0x1e1b8) = find_font(0x20, 0);

    int *fontRec = *reinterpret_cast<int **>(self + 0x1e184);
    int lineH;
    if (fontRec[0] < 0)
        lineH = fontRec[3];
    else
        lineH = fontRec[4] + fontRec[0];
    *reinterpret_cast<int *>(self + 0x1c) = lineH;

    reinterpret_cast<ReportWin *>(self)->init();
}
