// ORIGINAL: 0x0045C290 FILE
// name      ?restore_back@MainInterface@@QAEXH@Z
// size      259 bytes
// spans     0x0045C290-0x0045C393
// prototype void (__thiscall ?restore_back@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DA860
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C290
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?restore_back@MainInterface@@QAEXH@Z  at 0x0045C290  (259 bytes)
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
class MainInterface;
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
    int set_clip(RECT *);
    int tile(Buffer *, int, int, int, int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d6b0 = (int *)0x0078D6B0;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;
static int *const g_007d3930 = (int *)0x007D3930;

class MainInterface { public:
    void restore_back(int);
};

void MainInterface::restore_back(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        reinterpret_cast<Buffer *>(g_007aec64)->set_clip(reinterpret_cast<RECT *>(g_007aec94));
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd80);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(g_007aec64)->tile(reinterpret_cast<Buffer *>(*g_0078d6b0), x, y, x, y, w, h);
    }

    {
        int *r = *g_007d3930 != 0
            ? reinterpret_cast<int *>(self + 0xdd0)
            : reinterpret_cast<int *>(self + 0xdb0);
        int x = r[0];
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(g_007aec64)->tile(reinterpret_cast<Buffer *>(*g_0078d6b0), x, y, x, y, w, h);
    }

    {
        int *r = *g_007d3930 != 0
            ? reinterpret_cast<int *>(self + 0xdc0)
            : reinterpret_cast<int *>(self + 0xda0);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(g_007aec64)->tile(reinterpret_cast<Buffer *>(*g_0078d6b0), x, y, x, y, w, h);
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd90);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(g_007aec64)->tile(reinterpret_cast<Buffer *>(*g_0078d6b0), x, y, x, y, w, h);
    }
}
