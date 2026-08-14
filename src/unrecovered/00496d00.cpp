// ORIGINAL: 0x00496D00 FILE
// RULED-OUT: a `switch` on the 9 cases (0 and 1 sharing `draw_intel`, since
//   Ghidra's decompile only lists case 0 and the jump table has 9 slots for
//   8 distinct handlers) does NOT tail-merge its identical per-case
//   trailing `set_clip` even though every case reaches it with the same
//   register state - unlike the if/else fold seen on 0x00615F50, VC6
//   apparently does not fold across switch jump-table targets, so the
//   straightforward switch already reproduces the disassembly's eight
//   separate copies. Caching `Buffer *buf = self+0x444` as a named local
//   swaps which register holds `this` vs `buf` relative to the original
//   (70.5%); leaving the cast un-cached (recomputed at each use) matches
//   the original's register roles and reaches 81.4% - the remaining diff
//   is that this source's compiler always folds `buf+0x30` back to the
//   constant `self+0x474` and computes it via `add` off `this`, where the
//   original computes it via `lea` off the already-live buffer register.
// working copy - scaffold materialised by --work
// name      ?on_redraw@ReportWin@@QAEXXZ
// size      228 bytes
// spans     0x00496D00-0x00496DE4
// prototype void (__thiscall ?on_redraw@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   0   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00496BB0 0x00498B30 0x0049BCD0 0x0049D540 0x0049EB10 0x0049F350 0x004A08E0 0x004A1140 0x004A2210 0x005D8000

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@ReportWin@@QAEXXZ  at 0x00496D00  (228 bytes)
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
class ReportWin;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
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
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
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
    int set_clip(RECT *);
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


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00496de4 = (int *)0x00496DE4;
static int *const g_00885f40 = (int *)0x00885F40;

class ReportWin { public:
    void reset();
    void draw_labs();
    void draw_energy();
    void draw_ops();
    void draw_projects();
    void draw_sat();
    void draw_security();
    void draw_score();
    void draw_intel();
    void on_redraw();
};
void ReportWin::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    reset();
    *reinterpret_cast<int *>(self + 0x59ac) = 0xffffffff;
    reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x5974));
    switch (*g_00885f40) {
    case 0:
    case 1:
        draw_intel();
        break;
    case 2:
        draw_labs();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 3:
        draw_energy();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 4:
        draw_ops();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 5:
        draw_projects();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 6:
        draw_sat();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 7:
        draw_security();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    case 8:
        draw_score();
        reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
        return;
    }
    reinterpret_cast<Buffer *>(self + 0x444)->set_clip(reinterpret_cast<RECT *>(self + 0x474));
}
