// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00514990
// name           ?quit_game@Console@@QAEXXZ
// size           412 bytes
// measured tier  MISMATCH
// divergence     48
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00514990/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?quit_game@Console@@QAEXXZ  at 0x00514990  (412 bytes)
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

class AlphaNet;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Console;
class Font;
class GraphicWin;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class Lock;
class Menu;
class NetDaemon;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

class AutoSound { public:
    PVOID vtable_;
    int val_1_;
    int val_2_;
    int val_3_;
    int val_4_;
    int val_5_;
    int val_6_;
    int val_7_;
    int val_8_;
    int val_9_;
    int val_10_;
    int val_11_;
    int val_12_;
    int val_13_;
    int val_14_;
    int val_15_;
    int val_16_;
    int val_17_;
    int val_18_;
    int val_19_;
    int val_20_;
    int val_21_;
    int val_22_;
    int val_23_;
    int val_24_;
    int val_25_;
    int val_26_;
    int val_27_;
    int val_28_;
    int val_29_;
    int val_30_;
    int val_31_;
    int val_32_;
    int val_33_;
    int val_34_;
    int val_35_;
    int val_36_;
    int val_37_;
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

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

class GraphicWin { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
    Buffer buffer_;
    uint32_t field_9CC_;
    uint32_t field_9D0_;
    uint32_t field_9D4_;
    uint32_t field_9D8_;
    uint32_t field_9DC_;
    uint32_t field_9E0_;
    uint32_t field_9E4_;
    uint32_t field_9E8_;
    uint32_t field_9EC_;
    uint32_t field_9F0_;
    uint32_t field_9F4_;
    uint32_t field_9F8_;
    uint32_t field_9FC_;
    uint32_t field_A00_;
    uint32_t field_A04_;
    uint32_t poCanvas_;
    uint32_t field_A0C_;
    uint32_t field_A10_;
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
};

class Win { public:
    AutoSound auto_sound_;
    uint32_t iFlags_;
    uint32_t iSomeFlag_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t poWinBase_;
    uint32_t iVertScaleDenom_;
    uint32_t iVertScaleNum_;
    Buffer * buffer1_;
    Buffer * buffer2_;
    Buffer * buffer3_;
    Buffer * buffer4_;
    Win * win_parent_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    Heap heap_;
    Menu * menu_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    RECT outer_rect_;
    RECT client_rect_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    Sprite * cursor_sprite_;
    uint32_t field_18C_;
    uint32_t field_190_;
    HCURSOR * cursor_handle_;
    int cursor_name_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    Win * children_[150];
    int child_count_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    Scroll * scroll_vert_;
    Scroll * scroll_horz_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class AlphaNet { public:
    uint32_t alignment_;
    uint8_t data_[0x149C];
    void close();
};
class Lock { public:
    void clear();
};
class NetDaemon { public:
    void hang_up();
};
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl bit_count(int);
int __cdecl save_game(int);
void __cdecl auto_save();
void __cdecl log_say(char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a7d4 = (int *)0x0068A7D4;
static int *const g_0068a7e4 = (int *)0x0068A7E4;
static int *const g_0068a7f0 = (int *)0x0068A7F0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e230 = (int *)0x0093E230;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093e8c4 = (int *)0x0093E8C4;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093e8cc = (int *)0x0093E8CC;
static int *const g_0093e8d0 = (int *)0x0093E8D0;
static int *const g_0093e8d4 = (int *)0x0093E8D4;
static int *const g_0093e8d8 = (int *)0x0093E8D8;
static int *const g_0093e8dc = (int *)0x0093E8DC;
static int *const g_0093e8e0 = (int *)0x0093E8E0;
static int *const g_0093e8e4 = (int *)0x0093E8E4;
static int *const g_0093e8e8 = (int *)0x0093E8E8;
static int *const g_0093e8ec = (int *)0x0093E8EC;
static int *const g_0093e8f0 = (int *)0x0093E8F0;
static int *const g_0093e8f8 = (int *)0x0093E8F8;
static int *const g_0093e8fc = (int *)0x0093E8FC;
static int *const g_0093e908 = (int *)0x0093E908;
static int *const g_0093e90c = (int *)0x0093E90C;
static int *const g_0093e910 = (int *)0x0093E910;
static int *const g_0093e930 = (int *)0x0093E930;
static int *const g_0093e934 = (int *)0x0093E934;
static int *const g_0093e938 = (int *)0x0093E938;
static int *const g_0093e93c = (int *)0x0093E93C;
static int *const g_0093e940 = (int *)0x0093E940;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_0093e950 = (int *)0x0093E950;
static int *const g_0093e954 = (int *)0x0093E954;
static int *const g_0093e958 = (int *)0x0093E958;
static int *const g_0093e95c = (int *)0x0093E95C;
static int *const g_0093e960 = (int *)0x0093E960;
static int *const g_0093e964 = (int *)0x0093E964;
static int *const g_0093e968 = (int *)0x0093E968;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009b2068 = (int *)0x009B2068;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void quit_game();
};

// Indexed (register-incremented) zero-fill loop over 8 dwords starting at
// 0x93E910 - the lever wants an extern array here, not the scaffold's fixed
// pointer, so the addressing stays a base+index rather than folding.
extern int g_e910_block[];

void Console::quit_game() {
    const char *prompt;
    if (*g_0093f660 == 0 && *g_0093a95c == 0) {
        goto really_quit;
    }
    if (bit_count(*g_009a64e8 & 0xff) > 1) {
        prompt = reinterpret_cast<const char *>(g_0068a7d4);
    } else {
    really_quit:
        prompt = reinterpret_cast<const char *>(g_0068a7e4);
    }

    if (X_pop(prompt, 0) == 0) {
        return;
    }

    if (*g_0093a95c != 0) {
        *reinterpret_cast<unsigned char *>(g_009a64e8) &= ~(1 << *g_00939284);
        save_game(0);
    }

    if ((*g_009a649c & 0x400) && *g_0093a95c == 0) {
        auto_save();
    }

    if (*g_0093f660 != 0) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->hang_up();

        int *p = g_e910_block;
        for (int i = 8; i != 0; i--) {
            *p = 0;
            p++;
        }

        *g_0093e8c0 = 0;
        *g_0093e8c4 = 0;
        *g_0093e8c8 = 0;
        *g_0093e8cc = 0;
        *g_0093e8f0 = 0;
        *g_0093e8ec = 0;
        *g_0093e8fc = 0;
        *g_0093e8f8 = 0;
        *g_0093e90c = 0;
        *g_0093e938 = 0;
        *g_0093e930 = 0;
        *g_0093e934 = 0;
        *g_0093e95c = 0;
        *g_0093e958 = 0;
        *g_0093e8e0 = 0;
        *g_0093e8e4 = 0;
        *g_0093e8e8 = 0;
        *g_0093e954 = 0;
        *g_0093e950 = 0;
        *g_0093e940 = 0;
        *g_0093e93c = 0;
        *g_0093e944 = 0;
        *g_0093e8d0 = 0;
        *g_0093e8d4 = 0;
        *g_0093e968 = 0;
        *g_0093e964 = 0;
        *g_0093e8d8 = 0;
        *g_0093e8dc = 0;
        *g_0093e960 = 0xff;
        *g_0093e908 = 0;

        reinterpret_cast<Lock *>(g_0093e230)->clear();
        reinterpret_cast<AlphaNet *>(g_0093cd90)->close();
        *g_0093f660 = 0;
    }

    *g_009b2068 = 1;
    log_say(const_cast<char *>(reinterpret_cast<const char *>(g_0068a7f0)), 0, 0, 0);
}
