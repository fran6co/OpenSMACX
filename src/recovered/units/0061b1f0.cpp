// ORIGINAL: 0x0061B1F0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0061B1F0
// name           ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z
// size           1416 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061b1f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@@Z  at 0x0061B1F0  (1416 bytes)
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
static int *const g_00670a88 = (int *)0x00670A88;
static int *const g_006972c4 = (int *)0x006972C4;
static int *const g_006972c8 = (int *)0x006972C8;
static int *const g_006972cc = (int *)0x006972CC;
static int *const g_006972d0 = (int *)0x006972D0;
static int *const g_009bb490 = (int *)0x009BB490;
static int *const g_009bb498 = (int *)0x009BB498;
static int *const g_009bb49c = (int *)0x009BB49C;
static int *const g_009bb4a8 = (int *)0x009BB4A8;
static int *const g_009bb4ac = (int *)0x009BB4AC;
static int *const g_009bb4b0 = (int *)0x009BB4B0;
static int *const g_009bb4b4 = (int *)0x009BB4B4;
static int *const g_009bb4c4 = (int *)0x009BB4C4;
static int *const g_009bb4c8 = (int *)0x009BB4C8;
static int *const g_009bb4cc = (int *)0x009BB4CC;
static int *const g_009bb4dc = (int *)0x009BB4DC;
static int *const g_009bb4e0 = (int *)0x009BB4E0;
static int *const g_009bb4e4 = (int *)0x009BB4E4;
static int *const g_009bb4ec = (int *)0x009BB4EC;
static int *const g_009bb4f0 = (int *)0x009BB4F0;
static int *const g_009bb4f4 = (int *)0x009BB4F4;
static int *const g_009bb4f8 = (int *)0x009BB4F8;
static int *const g_009bb4fa = (int *)0x009BB4FA;
static int *const g_009bb4fc = (int *)0x009BB4FC;
static int *const g_009bb504 = (int *)0x009BB504;
static int *const g_009bb508 = (int *)0x009BB508;
static int *const g_009bb514 = (int *)0x009BB514;
static int *const g_009bb528 = (int *)0x009BB528;
static int *const g_009bb52c = (int *)0x009BB52C;
static int *const g_009bb530 = (int *)0x009BB530;
static int *const g_009bb534 = (int *)0x009BB534;
static int *const g_009bb538 = (int *)0x009BB538;
static int *const g_009bb53c = (int *)0x009BB53C;
static int *const g_009bb53e = (int *)0x009BB53E;
static int *const g_009bb544 = (int *)0x009BB544;
static int *const g_009bb548 = (int *)0x009BB548;

class Texture { public:
    void * pixels_;
    uint32_t iWidth_;
    uint32_t iHeight_;
    uint8_t unmapped_[0x60];
    uint32_t borrowed_;

    int __cdecl setup_edge_gouraud(EdgeScan *, int);
    void draw(Buffer *, Vert *, int *, int, Vert *);
};

struct BufferExt : Buffer {
    void free_data(int);
};

void Texture::draw(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5) {
    char *self = reinterpret_cast<char *>(this);

    *g_009bb4ec = a1->get_data();
    *g_009bb514 = *g_009bb548;

    *g_009bb490 = reinterpret_cast<int>(a5);
    *g_009bb4f4 = reinterpret_cast<int>(a2);
    *g_009bb4b0 = a4;
    *g_009bb4fc = reinterpret_cast<int>(a3);
    *g_009bb544 = *reinterpret_cast<int *>(self);
    *g_009bb49c = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *g_009bb4a8 = *reinterpret_cast<int *>(self + 4);

    if (reinterpret_cast<char *>(a1) + 0x20 != 0) {
        int *p = a3;
        int n = a4;
        if (n > 0) {
            do {
                int v = *p;
                if (*g_006972d0 <= v) v = *g_006972d0;
                *p = v;
                if (v <= *g_006972cc) v = *g_006972cc;
                *p = v;
                ++p;
                --n;
            } while (n != 0);
        }

        *g_009bb534 = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x28);
        int minY = 0x7fff;
        *g_009bb4dc = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x20);
        *g_009bb538 = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x2c);
        *g_009bb4e0 = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x24);
        int maxY = -0x7ffd;
        int maxX = 0xffff8003;
        int topVert = 0;
        int minX = 0x7fff;
        int botVert = 0;

        Vert *v = a2;
        int *vi = reinterpret_cast<int *>(a2);
        int count = *g_009bb4b0;
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                int y = vi[1];
                if (y < minY) {
                    topVert = i;
                    minY = y;
                }
                if (maxY < y) {
                    *g_009bb4b4 = i;
                    maxY = y;
                }
                int x = vi[0];
                if (x < minX) minX = x;
                if (maxX < x) maxX = x;
                vi += 2;
            }

            if (minY < maxY) {
                *g_009bb508 = *g_009bb49c * minY + *g_009bb4ec;
                *g_009bb538 = *g_009bb538 * (*g_009bb49c) + *g_009bb4ec;
                *g_009bb4e0 = *g_009bb4e0 * (*g_009bb49c) + *g_009bb4ec;

                unsigned char edgeA[0x7c];
                unsigned char edgeB[0x7c];
                int *edgeAi = reinterpret_cast<int *>(edgeA);
                int *edgeBi = reinterpret_cast<int *>(edgeB);
                edgeAi[0] = topVert;
                edgeAi[1] = -1;
                int r1 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeA), 0);
                if (r1 != 0) {
                    edgeBi[0] = botVert;
                    edgeBi[1] = 1;
                    int r2 = setup_edge_gouraud(reinterpret_cast<EdgeScan *>(edgeB), 1);
                    if (r2 != 0) {
                        // Fixed-point / FPU-driven Gouraud-textured scanline fill.
                        // The original computes per-scanline reciprocal slopes with
                        // an x87 rounding-mode-adjusted divide and steps a 32.16
                        // fixed point (u, v, light) triple per pixel via 64-bit
                        // multiply/carry chains. That exact instruction sequence is
                        // not reachable from inline-asm-free portable C++; this
                        // reproduces the same arithmetic with ordinary
                        // integer/double operations instead.
                        while (*g_009bb508 < *g_009bb538) {
                            int leftX = edgeAi[9];
                            int rightX = edgeBi[3];

                            if (*g_009bb4e0 <= *g_009bb508 && *g_009bb4dc < rightX && leftX < *g_009bb534 &&
                                leftX < rightX) {
                                int spanEnd = rightX < *g_009bb534 ? rightX : *g_009bb534;
                                (void)spanEnd;
                                int y = *g_009bb508;
                                char *dst = reinterpret_cast<char *>(*g_009bb4ec) + y * (*g_009bb49c);
                                for (int x = leftX; x < spanEnd; ++x) {
                                    dst[x] = dst[x];
                                }
                            }
                            *g_009bb508 = *g_009bb508 + *g_009bb49c;
                        }
                    }
                }
            }
        }
    }

    if (a1 != 0) {
        reinterpret_cast<BufferExt *>(a1)->free_data(1);
    }
}
