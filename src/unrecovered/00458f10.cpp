// ORIGINAL: 0x00458F10 ?set_state@InfoWin@@QAEXHH@Z 0x00458F10-0x00458FEA FILE
// TRIED: 0.986 mnemonic similarity, one instruction off (`jns` vs `jge`) on `if ((v &= 0x80000007) < 0)` - VC6 inserts a fresh cmp/test instead of reusing the AND's flags no matter how the mask+compare is split or combined; flipping the a2==0 ternary arm order (else-first) fixed the two other divergences.
// working copy - scaffold materialised by --work
// size      218 bytes
// prototype void (__thiscall ?set_state@InfoWin@@QAEXHH@Z)(InfoWin* this, int, int)
// callers   3   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00456300 0x00458420 0x00458DC0 0x005E2DD3

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_state@InfoWin@@QAEXHH@Z  at 0x00458F10  (218 bytes)
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
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class InfoWin;
typedef char * LPSTR;
typedef void * LPVOID;
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


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0078d7f8 = (int *)0x0078D7F8;
static int *const g_007aec64 = (int *)0x007AEC64;

class InfoWin { public:
    void setup_text();
    void draw_text(int);
    int state_valid(int);
    void set_state(int, int);
};
void InfoWin::set_state(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int idx = *reinterpret_cast<int *>(self + 0x80);
    if (a1 < 0) {
        a1 = 0;
    }
    *reinterpret_cast<int *>(self + 0x88 + idx * 4) = a1;
    int valid = state_valid(a1);
    while (valid == 0) {
        int v = *reinterpret_cast<int *>(self + 0x88 + idx * 4) + 1;
        if ((v &= 0x80000007) < 0) {
            v = ((v - 1) | 0xfffffff8) + 1;
        }
        *reinterpret_cast<int *>(self + 0x88 + idx * 4) = v;
        valid = state_valid(v);
    }
    *reinterpret_cast<int *>(self + 0x94) = 0;
    int c9c8;
    if (a2 != 0) {
        c9c8 = 0;
    } else {
        c9c8 = *reinterpret_cast<int *>(self + 0x9c0) + 1;
    }
    *reinterpret_cast<int *>(self + 0x9c8) = c9c8;
    int c9cc;
    if (a2 != 0) {
        c9cc = 0;
    } else {
        c9cc = *reinterpret_cast<int *>(self + 0x9c4) + 1;
    }
    *reinterpret_cast<int *>(self + 0x9cc) = c9cc;
    *reinterpret_cast<int *>(self + 0xa8) = 0;

    RECT *rect = reinterpret_cast<RECT *>(self + ((*reinterpret_cast<int *>(self + 0x80) + 1) << 4));
    reinterpret_cast<Buffer *>(0x7aec64)->box_sprite(rect, reinterpret_cast<BoxSpriteParams *>(0x78d7f8));

    if (self + 0x88 != 0) {
        *reinterpret_cast<int *>(self + 0x9c8) = 0;
        *reinterpret_cast<int *>(self + 0x9cc) = 0;
    }
    setup_text();
    draw_text(*reinterpret_cast<int *>(self + 0x80));
}
