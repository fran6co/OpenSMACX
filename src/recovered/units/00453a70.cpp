// ORIGINAL: 0x00453A70 FILE
// name      ?load_box_sprites@@YAXXZ
// size      1279 bytes
// spans     0x00453A70-0x00453F6F
// prototype 
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005D7670 0x005D7DE0 0x005DFF00 0x005E2210 0x005E39A0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00453A70
// measured tier  MISMATCH
// divergence     14
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00453a70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?load_box_sprites@@YAXXZ  at 0x00453A70  (1279 bytes)
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
struct ExtDirectDraw;
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
    int copy(Buffer *, int, int, int, int, int, int);
    int get_pixel(int, int);
    int init(int, int, int, ExtDirectDraw *);
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
static int *const g_00685014 = (int *)0x00685014;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_0075afd0 = (int *)0x0075AFD0;
static int *const g_0075b058 = (int *)0x0075B058;
static int *const g_0075c218 = (int *)0x0075C218;
static int *const g_0076dc30 = (int *)0x0076DC30;
static int *const g_007793f0 = (int *)0x007793F0;
static int *const g_00779478 = (int *)0x00779478;
static int *const g_0077aa70 = (int *)0x0077AA70;
static int *const g_007871d0 = (int *)0x007871D0;
static int *const g_00788038 = (int *)0x00788038;
static int *const g_007880bc = (int *)0x007880BC;
static int *const g_0078d52c = (int *)0x0078D52C;
static int *const g_007921e8 = (int *)0x007921E8;
static int *const g_00798668 = (int *)0x00798668;
static int *const g_007ab400 = (int *)0x007AB400;
static int *const g_007ac040 = (int *)0x007AC040;
static int *const g_007ac06c = (int *)0x007AC06C;
static int *const g_007ac1c0 = (int *)0x007AC1C0;
static int *const g_007acfb0 = (int *)0x007ACFB0;
static int *const g_007f67c8 = (int *)0x007F67C8;
static int *const g_00822610 = (int *)0x00822610;
static int *const g_00822718 = (int *)0x00822718;

// Eight sprite arrays walked by a SHARED index register in the nested loop
// below (`lea ecx,[edi+CONST]`): the INDEXED TABLE BASE lever - a plain
// address-holding pointer constant gets its `+edi` folded away under /O2,
// so each needs its own `extern` array instead.
extern char g_007ac290_arr[];
extern char g_0076dc98_arr[];
extern char g_0078ae58_arr[];
extern char g_0078cc60_arr[];
extern char g_00779cb0_arr[];
extern char g_007a6978_arr[];
extern char g_0078e0a0_arr[];
extern char g_0075b950_arr[];

// Every `Sprite::extract` call in this function shares its Buffer* (the
// canvas at g_00798668), its language/quality byte (g_00696d14) and a null
// TexHeap* - only the receiving Sprite and the four geometry ints vary.
static void do_extract(void *sprite, int p2, int p3, int p4, int p5) {
    reinterpret_cast<Sprite *>(sprite)->extract(
        reinterpret_cast<Buffer *>(g_00798668),
        *reinterpret_cast<unsigned char *>(g_00696d14),
        p2, p3, p4, p5, 0);
}

void __cdecl load_box_sprites() {
    Buffer *canvas = reinterpret_cast<Buffer *>(g_00798668);

    canvas->load_pcx(reinterpret_cast<const char *>(g_00685014), 0, 0xa, 0xec);

    do_extract(g_007ac040, 0xf7, 0x47, 0xf, 0x13);
    do_extract(g_007ac06c, 0x107, 0x47, 0xf, 0x13);
    do_extract(g_007f67c8, 0x267, 0x212, 0x10, 0xf);

    {
        int y = 0x216;
        char *sprite = reinterpret_cast<char *>(g_00788038);
        do {
            do_extract(sprite, y, 0x219, 7, 0xd);
            sprite += 0x2c;
            y += 8;
        } while (sprite < reinterpret_cast<char *>(g_007880bc));
    }

    int id = 0xe5;
    int idx = 0;
    int *rec = reinterpret_cast<int *>(g_0078d52c);
    Buffer *buf = reinterpret_cast<Buffer *>(g_0075c218);
    do {
        int x = 0x21;
        int id2 = id + 0x20;
        do {
            do_extract(g_00798668, x - 0x20, id, 6, 6);
            do_extract(g_00798668, x, id, 6, 6);
            do_extract(g_00798668, x - 0x20, id2, 6, 6);
            do_extract(g_00798668, x, id2, 6, 6);
            do_extract(g_00798668, x - 0x19, id, 0x18, 6);
            do_extract(g_00798668, x - 0x19, id2, 0x18, 6);
            do_extract(g_00798668, x - 0x20, id + 7, 6, 0x18);
            do_extract(g_00798668, x, id + 7, 6, 0x18);

            buf->init(0x18, 0x18, 0, 0);
            canvas->copy(buf, x - 0x19, id + 7, 0, 0, 0x18, 0x18);

            rec[-1] = reinterpret_cast<int>(g_007ac290_arr + idx);
            rec[0] = reinterpret_cast<int>(g_0076dc98_arr + idx);
            rec[1] = reinterpret_cast<int>(g_0078ae58_arr + idx);
            rec[2] = reinterpret_cast<int>(g_0078cc60_arr + idx);
            rec[3] = reinterpret_cast<int>(g_00779cb0_arr + idx);
            rec[4] = reinterpret_cast<int>(g_007a6978_arr + idx);
            rec[5] = reinterpret_cast<int>(g_0078e0a0_arr + idx);
            rec[6] = reinterpret_cast<int>(g_0075b950_arr + idx);

            if (canvas->get_pixel(0, 0) == 9) {
                rec[7] = 0;
                buf->close();
            } else {
                rec[7] = reinterpret_cast<int>(buf);
            }
            rec[8] = 7;

            x += 0x27;
            idx += 0x2c;
            buf = reinterpret_cast<Buffer *>(reinterpret_cast<char *>(buf) + 0x588);
            rec += 10;
        } while (x < 0x96);
        id += 0x27;
    } while (reinterpret_cast<char *>(buf) < reinterpret_cast<char *>(g_0076dc30));

    do_extract(g_007921e8, 0x128, 0x2c7, 0x3d, 0x2c);
    do_extract(g_007793f0, 0x166, 0x2c7, 0x3d, 0x2c);
    do_extract(g_00779478, 0x1a4, 0x2c7, 0x3d, 0x2c);
    do_extract(g_007acfb0, 0x128, 0x2f4, 0x3d, 0x2c);
    do_extract(g_0075b058, 0x1a4, 0x2f4, 0x3d, 0x2c);
    do_extract(g_007ac1c0, 0x128, 0x321, 0x3d, 0x2c);
    do_extract(g_007871d0, 0x166, 0x321, 0x3d, 0x2c);
    do_extract(g_007ab400, 0x1a4, 0x321, 0x3d, 0x2c);

    Buffer *small = reinterpret_cast<Buffer *>(g_0077aa70);
    small->init(0x3d, 0x2c, 0, 0);
    canvas->copy(small, 0x166, 0x2f4, 0, 0, 0x3d, 0x2c);

    {
        int y = 0x8c;
        char *sprite = reinterpret_cast<char *>(g_0075afd0);
        do {
            do_extract(sprite, y, 0x25, 7, 0xd);
            sprite += 0x2c;
            y += 8;
        } while (y < 0xa4);
    }

    {
        int x = 0x187;
        char *sprite = reinterpret_cast<char *>(g_00822610);
        do {
            int y = 0x236;
            int count = 3;
            do {
                do_extract(sprite, x, y, 0x1c, 0x22);
                y += 0x23;
                sprite += 0x2c;
                count--;
            } while (count != 0);
            x += 0x1d;
        } while (sprite < reinterpret_cast<char *>(g_00822718));
    }

    canvas->close();
}
