// ORIGINAL: 0x005BE2A0 tech_wizard_unk2 0x005BE2A0-0x005BE378 FILE
// TRIED: 6-param __cdecl void, if/else matching Ghidra's arithmetic structure gets 0.84 mnemonic similarity but the original saves ebx/esi (push) before the `al&1` test while ours defers it; register-save scheduling around the ternary not matched.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E4B9A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: tech_wizard_unk2  at 0x005BE2A0  (216 bytes)
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
class Sprite;

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

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
    int draw(Buffer *, int, int, int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691e6c = (int *)0x00691E6C;
static int *const g_00691e70 = (int *)0x00691E70;
static int *const g_00696d18 = (int *)0x00696D18;
static int *const g_00696d1c = (int *)0x00696D1C;
static int *const g_00759e28 = (int *)0x00759E28;
static int *const g_00759e30 = (int *)0x00759E30;
extern "C" void __cdecl tech_wizard_unk2(int a1, int a2, int a3, int a4, int a5, int a6) {
    if (a2 < 0x59) {
        int denom = *g_00691e6c;
        int div = *g_00691e70;
        int x_off;
        if ((a3 & 1) == 0) {
            x_off = 0;
        } else if (denom <= 0) {
            x_off = 0x6c;
        } else {
            x_off = (denom * 0xd9) / div;
        }
        int bx = a4 + x_off;

        int y_off;
        if (denom <= 0) {
            y_off = 0x6b;
        } else {
            y_off = (denom * 0xd7) / div;
        }
        int dy = a6 - y_off;
        int y = a5 + (dy / 2);

        if (denom > 0) {
            *g_00696d18 = denom;
            *g_00696d1c = div;
        } else {
            *g_00696d18 = 8;
            *g_00696d1c = 0x10;
        }

        Buffer *buf = a1 != 0 ? reinterpret_cast<Buffer *>(a1 + 0x444) : reinterpret_cast<Buffer *>(0);

        int index = a2 * 11;
        unsigned char transp = *(reinterpret_cast<unsigned char *>(g_00759e30) + index * 4);
        Sprite *spr = reinterpret_cast<Sprite *>(reinterpret_cast<char *>(g_00759e28) + index * 4);
        spr->draw(buf, transp, bx, y);

        *g_00696d18 = 1;
        *g_00696d1c = 1;
    }
}
