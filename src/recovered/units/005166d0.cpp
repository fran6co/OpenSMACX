// ORIGINAL: 0x005166D0 FILE
// name      ?cursor_key@Console@@QAEXHHH@Z
// size      487 bytes
// spans     0x005166D0-0x005168B7
// prototype void (__thiscall ?cursor_key@Console@@QAEXHHH@Z)(Console* this, int, int, int)
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041A920 0x00467970 0x0046A550 0x004D2F00 0x004D61A0 0x004E3A50 0x004E3C60 0x0055BC00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005166D0
// measured tier  MISMATCH
// divergence     27
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005166d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?cursor_key@Console@@QAEXHHH@Z  at 0x005166D0  (487 bytes)
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

class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class BaseWin;
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
class MapWin;
class Menu;
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
class BaseWin { public:
    void zoom(int, int);
};
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void draw_base_dest(int);
    void draw_map(int);
};
int __cdecl base_at(int, int);
int __cdecl base_find(int, int, int);
int __cdecl spying(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00400000 = (int *)0x00400000;
static int *const g_005168b8 = (int *)0x005168B8;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093e944 = (int *)0x0093E944;
static int *const g_009a64c0 = (int *)0x009A64C0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void activate(int, int, int);
    void go_to(int, int, int);
    void cursor_key(int, int, int);
};

// Indexed by a register loaded from g_009392b8 - the pointer-constant form
// scaffolding emits folds the base into the `lea`, which /O2 turns into
// `add`. `extern T name[]` keeps the base-immediate + offset-register
// encoding the original uses.
extern int g_009392c0x[];
extern int g_00939340x[];
// Struct array, stride 0x134, indexed by a register (base * 0x134). Byte
// granularity so the field offsets (0 and 6) read as plain array indices.
extern unsigned char g_0097d044x[];

void Console::cursor_key(int a1, int a2, int a3) {
    switch (a2) {
    case 0x20: {
        int idx = *g_009392b8;
        int base = base_at(g_009392c0x[idx], g_00939340x[idx]);
        if (base >= 0) {
            if (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) {
                goto zoom_base;
            }
            {
                unsigned rec = base * 0x134;
                int id = g_0097d044x[rec];
                int player = *g_00939284;
                if (id == player ||
                    ((g_0097d044x[rec + 6] & (1 << player)) && spying(id))) {
                    goto zoom_base;
                }
            }
            goto skip_zoom_base;
            zoom_base:
            reinterpret_cast<BaseWin *>(g_006a7628)->zoom(base, 0);
            return;
            skip_zoom_base:;
        }
    }
    case 0x61:
        if (*g_0093e944 == 0) {
            int idx = *g_009392b8;
            activate(g_009392c0x[idx], g_00939340x[idx], 1);
            return;
        }
        break;
    case 0x62: {
        int idx = *g_009392b8;
        int base = base_find(g_009392c0x[idx], g_00939340x[idx], *g_00939284);
        if (base >= 0) {
            reinterpret_cast<BaseWin *>(g_006a7628)->zoom(base, 0);
            return;
        }
        break;
    }
    case 0x67:
        if (*g_0093a938 == 0 && *g_0093e944 == 0) {
            int idx = *g_009392b8;
            int base = base_at(g_009392c0x[idx], g_00939340x[idx]);
            if (base >= 0) {
                go_to(base, -1, 0);
                return;
            }
        }
        break;
    case 0x70: {
        int flag = *reinterpret_cast<int *>(reinterpret_cast<char *>(*reinterpret_cast<MapWin **>(g_007d3c3c)) + 0x1dd70) & 0x400000;
        int addr = reinterpret_cast<int>(g_007d3c3c);
        do {
            MapWin *m = *reinterpret_cast<MapWin **>(addr);
            if (m != 0 &&
                (addr == reinterpret_cast<int>(g_007d3c3c) ||
                 *reinterpret_cast<int *>(reinterpret_cast<char *>(m) + 0x1dd74) != 0)) {
                int f = *reinterpret_cast<int *>(reinterpret_cast<char *>(m) + 0x1dd70);
                if (flag == 0) {
                    f |= 0x400000;
                    *reinterpret_cast<int *>(reinterpret_cast<char *>(m) + 0x1dd70) = f;
                    m->draw_base_dest(1);
                } else {
                    f &= 0xffbfffff;
                    *reinterpret_cast<int *>(reinterpret_cast<char *>(m) + 0x1dd70) = f;
                    m->draw_map(1);
                }
            }
            addr += 4;
        } while (addr < reinterpret_cast<int>(g_007d3c5c));
        break;
    }
    }
}
