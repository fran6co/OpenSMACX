// ORIGINAL: 0x0061A760 ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z 0x0061A760-0x0061AC53 FILE
// size      1267 bytes
// prototype 
// callers   2   call targets   3
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
// address        0x0061A760
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0061a760/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_upper_threshold_trans@Texture@@QAEXPAUBuffer@@PAUVert@@PAHHPAUVert@@H@Z  at 0x0061A760  (1267 bytes)
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
static int *const g_006972c4 = (int *)0x006972C4;
static int *const g_006972c8 = (int *)0x006972C8;
static int *const g_009bb490 = (int *)0x009BB490;
static int *const g_009bb49c = (int *)0x009BB49C;
static int *const g_009bb4a8 = (int *)0x009BB4A8;
static int *const g_009bb4ac = (int *)0x009BB4AC;
static int *const g_009bb4b0 = (int *)0x009BB4B0;
static int *const g_009bb4b4 = (int *)0x009BB4B4;
static int *const g_009bb4c0 = (int *)0x009BB4C0;
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
static int *const g_009bb504 = (int *)0x009BB504;
static int *const g_009bb508 = (int *)0x009BB508;
static int *const g_009bb50c = (int *)0x009BB50C;
static int *const g_009bb514 = (int *)0x009BB514;
static int *const g_009bb528 = (int *)0x009BB528;
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

    int __cdecl setup_edge(EdgeScan *, int);
    void draw_upper_threshold_trans(Buffer *, Vert *, int *, int, Vert *, int);
};

// Twelve ints per edge scanner, inferred from the field span each
// setup_edge() call touches (ebp-0x80..ebp-0x54 and ebp-0x44..ebp-0x18,
// both 0x30 bytes = 12 ints) - same shape as 0x006198E0's EdgeScanShadow.
struct EdgeScanShadow {
    int fields[12];
};

void Texture::draw_upper_threshold_trans(Buffer * a1, Vert * a2, int * a3, int a4, Vert * a5, int a6) {
    // a3/a4/a5/a6: the disassembly reads [ebp+8]/[ebp+0xc]/[ebp+0x10]/
    // [ebp+0x14] only - never [ebp+0x18] or [ebp+0x1c] - and the function
    // ends in `ret 0x10` (4 stack dwords), not the 0x18 six params would
    // need. So the real parameter list is 4 long, matching draw_trans's
    // shape exactly (Buffer*, Vert*, int, Vert*); see the PROPOSAL filed
    // for this address. Kept at the catalogued 6-param arity here only so
    // this stays the definition the scaffold's class declares; a3 is used
    // purely as a scalar loop count (never dereferenced), so its value is
    // read through a cast rather than as `*a3`, and a5/a6 go untouched.
    char *self = reinterpret_cast<char *>(this);
    int count = reinterpret_cast<int>(a3);
    int *verts = reinterpret_cast<int *>(a2);
    (void)a5;
    (void)a6;

    *g_009bb4ec = a1->get_data();
    *g_009bb4f4 = reinterpret_cast<int>(a2);
    *g_009bb544 = *reinterpret_cast<int *>(self);
    *g_009bb50c = *reinterpret_cast<int *>(self + 4);
    *g_009bb490 = a4;
    *g_009bb4c0 = *reinterpret_cast<int *>(self + 8);
    *g_009bb4b0 = count;
    *g_009bb514 = *g_009bb548;
    *g_009bb49c = *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 0x4a8);
    *g_009bb4a8 = *reinterpret_cast<int *>(self + 4);

    RECT *clip = reinterpret_cast<RECT *>(reinterpret_cast<char *>(a1) + 0x20);
    if (clip != 0) {
        *g_009bb534 = clip->right;
        *g_009bb4dc = clip->left;
        *g_009bb538 = clip->bottom;
        *g_009bb4e0 = clip->top;

        int min_y = 0x7fff, max_y = -0x7ffd;
        int min_x = 0x7fff, max_x = -0x7ffd;
        int top_idx = 0;

        if (count > 0) {
            int bottom_idx = 0;
            for (int i = 0; i < count; ++i) {
                int y = verts[i * 2 + 1];
                if (y < min_y) { min_y = y; top_idx = i; }
                if (y > max_y) { max_y = y; *g_009bb4b4 = bottom_idx = i; }
                int x = verts[i * 2];
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
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
                    if (right_ok) {
                        // Perspective-correct scanline fill with an extra
                        // "upper threshold" clip: a hand-tuned fixed-point
                        // DDA with a per-scanline FPU divide and a packed
                        // byte-level texel stepper, same shape as
                        // 0x006198E0's draw_trans. Reproducing that from
                        // plain C++ arithmetic is the WALL filed for this
                        // address - register allocation and strength
                        // reduction both apply to nearly every instruction
                        // past this point.
                    }
                }
            }
        }
    }
}
