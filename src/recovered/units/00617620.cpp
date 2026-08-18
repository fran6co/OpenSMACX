// ORIGINAL: 0x00617620 ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z 0x00617620-0x0061766F FILE
// size      79 bytes
// prototype void (__thiscall ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z)(Caviar* this, Buffer*, int, int, int, int)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00617230 0x006465F0
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00617620
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00617620/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?render_mono@Caviar@@QAEXPAUBuffer@@HHHH@Z  at 0x00617620  (79 bytes)
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
class Caviar;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;

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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memset(void *, int, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b9c38 = (int *)0x009B9C38;

class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];

    int render(Buffer *, int, int, int);
    void render_mono(Buffer *, int, int, int, int);
};

// CLOSEST REACHED: MNEMONIC_ONLY, 78 rebuilt bytes vs 79 original.
// `#pragma function(memset)` is load-bearing: without it, /O2 sees the
// compile-time-constant 0x1800 size and expands the call into an
// inline `rep stosd` fill (86 bytes, MISMATCH) instead of calling
// memset - the pragma forces the real call the disassembly shows.
// The one remaining divergence is the trailing `&= ~8`: the original
// clears the flag with a full-width `and ecx, 0xfffffff7` (6 bytes),
// but this toolchain always narrows that mask to `and al, 0xf7`
// (2 bytes) once the upper 24 bits are provably 1s, regardless of
// whether the mask is spelled as a hex literal, `~8`, or `-9`, and
// regardless of routing the value through a named local first. The
// `|= 8` a few lines earlier narrows to `or al, 8` in EXACTLY the same
// shape as the original, which is what confirms this is the compiler's
// own peephole rather than a source-form choice available here.
#pragma function(memset)
void Caviar::render_mono(Buffer * a1, int a2, int a3, int a4, int a5) {
    memset(g_009b9c38, a4 & 0xff, 0x1800);
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<unsigned int *>(self + 4) |= 8;
    render(a1, a2, a3, a5);
    *reinterpret_cast<unsigned int *>(self + 4) &= 0xfffffff7;
}
