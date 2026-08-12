// ORIGINAL: 0x00423000 FILE
// name      ?colortables_init_faction@@YAXH@Z
// size      1422 bytes
// spans     0x00423000-0x00423566;0x00652BE2-0x00652C0A
// prototype 
// callers   1   call targets   20
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00422F20 0x005D7210 0x005D7410 0x005D7DE0 0x005E2210 0x005FE2A0 0x005FE2E0 0x005FEFF0 0x005FF470 0x00628380 0x006283E0 0x00628540 0x00628650 0x006287C0 0x00628810 0x00628DB0 0x00645470 0x006458C8 0x00645930 0x00645C70
// indirect  0x0042320F 0x00423287 0x00423306 0x0042332C 0x00423353
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00423000
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00423000/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?colortables_init_faction@@YAXH@Z  at 0x00423000  (1422 bytes)
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
class Filemap;
class Font;
typedef void * HANDLE;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
struct HSV;
typedef char * LPSTR;
typedef void * LPVOID;
struct PALETTEENTRY;
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
class Filemap { public:
    LPVOID map_view_addr_;
    HANDLE file_;
    HANDLE file_map_;
    uint32_t file_size_;
    Filemap();
    int create(char *, unsigned long, int);
    int open(char *, int);
    void close();
    void close(unsigned char *);
    ~Filemap();
};
class Palette { public:
    Palette();
    int create_table_from_color(int, unsigned char *, int, int, int, int);
    int get_nearest_palette_index(unsigned char, unsigned char, unsigned char, int);
    ~Palette();
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _ftol();
extern "C" int __cdecl memcmp();
extern "C" void *memcpy(void *, const void *, unsigned int);
int __cdecl compare_palettes();
void __cdecl RGB_to_HSV(PALETTEENTRY *, HSV *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00652c00 = (int *)0x00652C00;
static int *const g_00669130 = (int *)0x00669130;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_0066aa90 = (int *)0x0066AA90;
static int *const g_00673628 = (int *)0x00673628;
static int *const g_00682eec = (int *)0x00682EEC;
static int *const g_00682ef4 = (int *)0x00682EF4;
static int *const g_00682f00 = (int *)0x00682F00;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_006f077c = (int *)0x006F077C;
static int *const g_0078e970 = (int *)0x0078E970;
static int *const g_0079a7ec = (int *)0x0079A7EC;
static int *const g_00946a54 = (int *)0x00946A54;
static int *const g_0094c590 = (int *)0x0094C590;
static int *const g_009b86a0 = (int *)0x009B86A0;

// The per-faction record table is indexed by a1 (stride 1436 bytes), so
// the address itself does work: a plain `static T *const` (g_00946a54
// above) folds the literal into the load and loses the indexed addressing
// mode. A freshly named `extern T name[];` keeps it - the name need not
// match the address, since the comparison masks relocations either way.
extern char g_faction_table[];

// Time-boxed: only the setup (Palette/Filemap/Buffer construction,
// compare_palettes, the pcx filename build, and the load_pcx call) is
// transcribed. The pixel-sampling and color-table computation that
// follows - most of this 1422-byte function - is not; see the report.
void __cdecl colortables_init_faction(int a1) {
    Palette pal;
    Filemap fmap;
    Buffer buf;

    int cmp = compare_palettes();

    char *strbuf = reinterpret_cast<char *>(g_009b86a0);
    strbuf[0] = 0;
    char *rec = g_faction_table + a1 * 1436;
    strcat(strbuf, rec);
    strcat(strbuf, reinterpret_cast<char *>(g_00682eec));

    int rc = buf.load_pcx(strbuf, &pal, 0xa, 0xec);

    (void)cmp;
    (void)rc;
    (void)fmap;
}
