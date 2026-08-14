// ORIGINAL: 0x004402A0 FILE
// RULED-OUT: RECT local via named struct field assigns, not raw offsets
// working copy - scaffold materialised by --work
// name      ?on_redraw@DiploPop@@QAEXXZ
// size      217 bytes
// spans     0x004402A0-0x00440379
// prototype void (__thiscall ?on_redraw@DiploPop@@QAEXXZ)(DiploPop* this)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043F5F0 0x0043F790 0x0043FA90 0x00440080 0x0045C290 0x005E2DD3
// indirect  0x0044030C 0x0044031D

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@DiploPop@@QAEXXZ  at 0x004402A0  (217 bytes)
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
struct BoxSpriteParams;
class Buffer;
class DiploPop;
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
    int box_sprite(RECT *, BoxSpriteParams *);
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

class MainInterface { public:
    void restore_back(int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669328 = (int *)0x00669328;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_0078d870 = (int *)0x0078D870;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007af2ac = (int *)0x007AF2AC;
static int *const g_007af2bc = (int *)0x007AF2BC;
static int *const g_007af5e0 = (int *)0x007AF5E0;
static int *const g_007af5e4 = (int *)0x007AF5E4;
static int *const g_007af5e8 = (int *)0x007AF5E8;
static int *const g_007af5ec = (int *)0x007AF5EC;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void draw_bioscan();
    void draw_portrait();
    void draw_info();
    void draw_hq();
    void on_redraw();
};
typedef int (__stdcall *UnionRectProc)(RECT *, RECT *, RECT *);

void DiploPop::on_redraw() {
    RECT rc;
    char *self = reinterpret_cast<char *>(this);

    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    rc.left = *g_007af5e0 + 4;
    rc.top = *g_007af5e4 + 4;
    rc.right = *g_007af5e8 - 4;
    rc.bottom = *g_007af5ec - 4;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(&rc, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    (*reinterpret_cast<UnionRectProc *>(g_00669328))(&rc, reinterpret_cast<RECT *>(g_007af2ac), reinterpret_cast<RECT *>(g_007af2bc));
    (*reinterpret_cast<UnionRectProc *>(g_00669328))(&rc, &rc, reinterpret_cast<RECT *>(self + 0x2144));

    rc.left -= 3;
    rc.right += 3;
    rc.bottom += 3;
    rc.top -= 3;

    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(&rc, reinterpret_cast<BoxSpriteParams *>(g_0078d870));

    draw_bioscan();
    draw_portrait();
    draw_info();
    draw_hq();
}
