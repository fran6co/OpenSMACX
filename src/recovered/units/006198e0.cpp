// ORIGINAL: 0x006198E0 FILE
// name      ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z
// size      1183 bytes
// spans     0x006198E0-0x00619D7F
// prototype void (__thiscall ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z)(Texture* this, Buffer*, Vert*, int, Vert*)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3 0x00624D80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006198E0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006198e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@@Z  at 0x006198E0  (1183 bytes)
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
struct EdgeScan;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;
class Texture;
struct Vert;

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
    int get_data();
    void free_data(int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00670a84 = (int *)0x00670A84;
static int *const g_009bb490 = (int *)0x009BB490;
static int *const g_009bb49c = (int *)0x009BB49C;
static int *const g_009bb4a8 = (int *)0x009BB4A8;
static int *const g_009bb4b0 = (int *)0x009BB4B0;
static int *const g_009bb4b4 = (int *)0x009BB4B4;
static int *const g_009bb4c0 = (int *)0x009BB4C0;
static int *const g_009bb4c4 = (int *)0x009BB4C4;
static int *const g_009bb4c8 = (int *)0x009BB4C8;
static int *const g_009bb4ca = (int *)0x009BB4CA;
static int *const g_009bb4cc = (int *)0x009BB4CC;
static int *const g_009bb4dc = (int *)0x009BB4DC;
static int *const g_009bb4e0 = (int *)0x009BB4E0;
static int *const g_009bb4e4 = (int *)0x009BB4E4;
static int *const g_009bb4e6 = (int *)0x009BB4E6;
static int *const g_009bb4ec = (int *)0x009BB4EC;
static int *const g_009bb4f0 = (int *)0x009BB4F0;
static int *const g_009bb4f4 = (int *)0x009BB4F4;
static int *const g_009bb4f8 = (int *)0x009BB4F8;
static int *const g_009bb4fa = (int *)0x009BB4FA;
static int *const g_009bb508 = (int *)0x009BB508;
static int *const g_009bb50c = (int *)0x009BB50C;
static int *const g_009bb514 = (int *)0x009BB514;
static int *const g_009bb530 = (int *)0x009BB530;
static int *const g_009bb534 = (int *)0x009BB534;
static int *const g_009bb538 = (int *)0x009BB538;
static int *const g_009bb53c = (int *)0x009BB53C;
static int *const g_009bb53e = (int *)0x009BB53E;
static int *const g_009bb544 = (int *)0x009BB544;

class Texture { public:
    void * pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    uint8_t unmapped_[0x60];
    uint32_t borrowed_;

    int __cdecl setup_edge(EdgeScan *, int);
    void draw_trans(Buffer *, Vert *, int, Vert *);
};

// Twelve ints per edge scanner, inferred from the field span each
// setup_edge() call touches (ebp-0x78..ebp-0x4c and ebp-0x3c..ebp-0x10,
// both 0x30 bytes = 12 ints) - see the STRUCTURE note filed for this
// address. EdgeScan itself stays opaque (forward-declared only), so this
// shadow is passed through a reinterpret_cast, matching the "shadow
// struct" allowance for a local whose real layout the scaffold omits.
struct EdgeScanShadow {
    int fields[12];
};

void Texture::draw_trans(Buffer * a1, Vert * a2, int a3, Vert * a4) {
    char *self = reinterpret_cast<char *>(this);
    int *verts = reinterpret_cast<int *>(a2);

    *g_009bb4ec = a1->get_data();
    *g_009bb4f4 = reinterpret_cast<int>(a2);
    *g_009bb544 = *reinterpret_cast<int *>(self);
    *g_009bb50c = *reinterpret_cast<int *>(self + 4);
    *g_009bb4c0 = *reinterpret_cast<int *>(self + 8);
    *g_009bb4b0 = a3;
    *g_009bb490 = reinterpret_cast<int>(a4);
    *g_009bb49c = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *g_009bb4a8 = *reinterpret_cast<int *>(self + 4);

    RECT *clip = reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x20);
    if (clip != 0) {
        *g_009bb534 = clip->right;
        *g_009bb4dc = clip->left;
        *g_009bb538 = clip->bottom;
        *g_009bb4e0 = clip->top;

        int min_y = 0x7fff, max_y = -0x7ffd;
        int top_idx = 0;

        if (a3 > 0) {
            int bottom_idx = 0;
            for (int i = 0; i < a3; ++i) {
                int y = verts[i * 2 + 1];
                if (y < min_y) { min_y = y; top_idx = i; }
                if (y > max_y) { max_y = y; *g_009bb4b4 = bottom_idx = i; }
            }

            if (min_y < max_y) {
                *g_009bb508 = *g_009bb49c * min_y + *g_009bb4ec;
                *g_009bb538 = *g_009bb538 * *g_009bb49c + *g_009bb4ec;
                *g_009bb4e0 = *g_009bb4e0 * *g_009bb49c + *g_009bb4ec;

                EdgeScanShadow left_edge;
                int left_ok = setup_edge(reinterpret_cast<EdgeScan *>(&left_edge), top_idx);
                if (left_ok) {
                    EdgeScanShadow right_edge;
                    int right_ok = setup_edge(reinterpret_cast<EdgeScan *>(&right_edge), top_idx);
                    if (right_ok && *reinterpret_cast<int *>(self + 0x6c) != 0) {
                        // Perspective-correct scanline fill: a hand-tuned
                        // fixed-point (8.24) DDA with a per-scanline FPU
                        // divide for the perspective correction and a
                        // packed byte-level texel stepper (shl/adc across
                        // combined integer/fraction register halves) in the
                        // pixel loop. Reproducing that from plain C++
                        // arithmetic is the WALL filed for this address:
                        // register allocation AND strength reduction both
                        // apply to nearly every instruction past this point,
                        // so no source-level rewrite closes the gap.
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        a1->free_data(1);
    }
}
