// ORIGINAL: 0x005DDBB0 ?wrap_right@Buffer@@QAEHPADHHH@Z 0x005DDBB0-0x005DDC8F FILE
// RULED-OUT: source order already matches Ghidra's (g=0; field18=0; field14=field10; y=a3); the original's `ebx/ebp` push order ahead of the field writes looks like pure register-scheduling, not reachable by reordering the C statements that produced it.
// working copy - scaffold materialised by --work
// size      223 bytes
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD3B0 0x006453E0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_right@Buffer@@QAEHPADHHH@Z  at 0x005DDBB0  (223 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

extern "C" unsigned int strlen(const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3a9c = (int *)0x009B3A9C;
static int *const g_009bb484 = (int *)0x009BB484;

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

    int find_line_break_l(char *, int *, int);
    int write_right_l(char *, int, int, int);
    int wrap_right(char *, int, int, int);
};
int Buffer::wrap_right(char * a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        return a3;
    }
    *g_009b3a9c = 0;
    *reinterpret_cast<int *>(self + 0x18) = 0;
    *reinterpret_cast<int *>(self + 0x14) = *reinterpret_cast<int *>(self + 0x10);
    int y = a3;
    char *text = a1;
    int width = a4;
    char *brk;
    do {
        a3 = width;
        if (text == 0) {
            brk = 0;
        } else {
            int len;
            *g_009b3a9c = 0;
            unsigned int slen = strlen(text);
            brk = reinterpret_cast<char *>(find_line_break_l(text, &a3, slen));
            if (brk == 0) {
                len = strlen(text);
            } else {
                len = (int)(brk - text);
            }
            write_right_l(text, a2, y, len);
        }
        *reinterpret_cast<int *>(self + 0x18) = *reinterpret_cast<int *>(self + 0x18) + 1;
        if (*reinterpret_cast<int *>(self + 0x52C) == 0) {
            *reinterpret_cast<int *>(self + 0x52C) = *g_009bb484;
        }
        int *font = *reinterpret_cast<int **>(self + 0x52C);
        int lineH;
        if (*font < 0) {
            lineH = font[3];
        } else {
            lineH = font[4] + *font;
        }
        y = y + lineH;
        int f14 = *reinterpret_cast<int *>(self + 0x14);
        if (f14 != 0) {
            *reinterpret_cast<int *>(self + 0x14) = 0;
            width = width - f14;
        }
        text = brk;
    } while (brk != 0);
    return y;
}
