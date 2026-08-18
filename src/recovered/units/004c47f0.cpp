// ORIGINAL: 0x004C47F0 ?UNK3@WorldWin@@QAEXXZ 0x004C47F0-0x004C499C FILE
// size      428 bytes
// prototype void (__thiscall ?UNK3@WorldWin@@QAEXXZ)(WorldWin* this)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C44E0 0x005E1A80 0x005E1BF0 0x005E3203
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004C47F0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c47f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@WorldWin@@QAEXXZ  at 0x004C47F0  (428 bytes)
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
struct RECT;
class Spot;
class WorldWin;

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
    int box(RECT *, int, int);
    void hline(int, int, int, int);
    void vline(int, int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007d3c40 = (int *)0x007D3C40;

class WorldWin { public:
    int tile_to_pixel(int, int, int *, int *);
    void UNK3();
};

// MISMATCH (#10: original 'lea' vs rebuilt 'mov' - close: mnemonic
// similarity 0.85, edit_count 11 of 143. Two tries got the shape right:
// (1) the two tile_to_pixel calls' int* out-params share ONE stack RECT
// whose four fields line up with the four locals the original spills
// (ebp-0x1c..ebp-4), which also fixed a completely frameless first attempt
// - the original needs the full 7-int (0x1c-byte) local set to earn its
// `push ebp` frame at all, and reusing rect fields directly (fewer live
// locals) compiled to a frameless `sub esp,N` function instead, off by the
// entire prologue from instruction 0. (2) `*g_007d3c40` inlined twice per
// call (no separate `wd` local) reproduced the original's CSE'd single
// load reused for both offset reads. What remains: the original computes
// BOTH address-of args (&rect.left, &rect.top) before touching
// `*g_007d3c40` at all (right-to-left argument evaluation finishing the
// trivial address args first); this rebuild's `*g_007d3c40 + offset`
// dereference schedules one `mov` ahead of the two `lea`s regardless of
// how the wd load is written inline vs hoisted. A second, smaller instance
// of the same class of gap sits around the `rect.left <= rect.right`
// branch (`dec`/`cmp`/store ordering into ebp-0xc/ebp-8/ebp-4). Both are
// INSTRUCTION SCHEDULING, not a logic or type error - ruled out two
// distinct source forms for the first one before stopping.
void WorldWin::UNK3() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x239d0) == 0) return;

    RECT rect;
    if (tile_to_pixel(*reinterpret_cast<int *>(*g_007d3c40 + 0x1dda4),
                       *reinterpret_cast<int *>(*g_007d3c40 + 0x1dda8),
                       reinterpret_cast<int *>(&rect.left), reinterpret_cast<int *>(&rect.top)) != 0) return;

    if (tile_to_pixel(*reinterpret_cast<int *>(*g_007d3c40 + 0x1ddd4) + *reinterpret_cast<int *>(*g_007d3c40 + 0x1ddcc) + *reinterpret_cast<int *>(*g_007d3c40 + 0x1dda4),
                       *reinterpret_cast<int *>(*g_007d3c40 + 0x1ddd8) + *reinterpret_cast<int *>(*g_007d3c40 + 0x1ddd0) + *reinterpret_cast<int *>(*g_007d3c40 + 0x1dda8),
                       reinterpret_cast<int *>(&rect.right), reinterpret_cast<int *>(&rect.bottom)) != 0) return;

    if (rect.left <= rect.right) {
        reinterpret_cast<Buffer *>(self + 0x444)->box(&rect, 0xfb, 0xfb);
        return;
    }

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    int a = rect.top;
    int b = rect.left;
    int c = rect.right - 1;
    int d = rect.bottom - 1;

    buf->hline(*reinterpret_cast<int *>(self + 0x239c4), c, a, 0xfb);
    buf->hline(*reinterpret_cast<int *>(self + 0x239c4), c - 1, d, 0xfb);
    buf->vline(c, a + 1, d, 0xfb);

    int v1;
    if (*reinterpret_cast<int *>(self + 0x239c4) != 0) {
        int p152c = *reinterpret_cast<int *>(self + 0x152c);
        int p4 = *reinterpret_cast<int *>(p152c + 4);
        int val = *reinterpret_cast<int *>(p4 + reinterpret_cast<int>(this) + 0x19f0);
        v1 = val + *reinterpret_cast<int *>(self + 0x239c4) - 1;
    } else {
        v1 = *reinterpret_cast<int *>(self + 0x4c4) - 1;
    }
    buf->hline(b + 1, v1, a, 0xfb);

    int v2;
    if (*reinterpret_cast<int *>(self + 0x239c4) != 0) {
        int p152c = *reinterpret_cast<int *>(self + 0x152c);
        int p4 = *reinterpret_cast<int *>(p152c + 4);
        int val = *reinterpret_cast<int *>(p4 + reinterpret_cast<int>(this) + 0x19f0);
        v2 = val + *reinterpret_cast<int *>(self + 0x239c4) - 1;
    } else {
        v2 = *reinterpret_cast<int *>(self + 0x4c4) - 1;
    }
    buf->hline(c, v2, d, 0xfb);
    buf->vline(c, a, d - 1, 0xfb);
}
