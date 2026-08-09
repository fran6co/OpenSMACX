// ORIGINAL: 0x00529C00 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00529C00
// name           ?mash_planes@@YAXXZ
// size           615 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00529c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mash_planes@@YAXXZ  at 0x00529C00  (615 bytes)
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
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Win;

// ---- callees, declared and never defined (a definition would be inlined) ----
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

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void focus(int, int, int);
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

int __cdecl boom(int, int, int);
int __cdecl speed(int, int);
int __cdecl stack_check(int, int, int, int, int);
void __cdecl draw_tile(int, int, int);
void __cdecl kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009ab88c = (int *)0x009AB88C;

// Indexed table bases: the const-pointer spelling the scaffold already
// declared for these two addresses is folded to a literal under /O2 and
// loses the scale-4 lea the original uses, so fresh extern arrays are
// declared here instead (the scaffold's g_00952832/g_009ab88c above are
// simply left unreferenced).
extern int tbl_952832[];
extern unsigned char tbl_9ab88c[];
extern unsigned char tbl_94a379[];

void __cdecl mash_planes() {
    int idx = *g_009a64c8 - 1;
    if (idx < 0) return;

    short *rec = reinterpret_cast<short *>(&tbl_952832[idx * 13]);
    do {
        int type = *rec;
        int sub = tbl_9ab88c[type * 0x34];
        if (tbl_94a379[sub * 0x90] == 2 && tbl_94a379[sub * 0x90 + 1] != 0) {
            int spd = speed(idx, 0);
            int diff = spd - *reinterpret_cast<unsigned char *>(
                                 reinterpret_cast<char *>(rec) + 0x1e);
            if (diff >= 0 && (diff > 999 || diff != 0)) {
                int x = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 8);
                int y = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 0xa);
                int iVar4 = x * (*g_0068faf0) + (y >> 1);
                int recBase = *g_0094a30c + iVar4 * 0x2c;
                unsigned char flagByte = *reinterpret_cast<unsigned char *>(recBase + 8);
                bool bail = false;
                if ((flagByte & 1) != 0) {
                    int nibble = *reinterpret_cast<unsigned char *>(recBase + 2) & 0xf;
                    if (nibble < 8) bail = true;
                }
                if (!bail &&
                    (*reinterpret_cast<unsigned int *>(recBase + 8) & 0x40000) == 0 &&
                    stack_check(idx, 6, 0x80, -1, -1) == 0) {
                    unsigned char owner = *reinterpret_cast<unsigned char *>(
                                              reinterpret_cast<char *>(rec) + 4);
                    int curCiv = *g_00939284;
                    bool didFocus = false;
                    if ((int)owner == curCiv ||
                        (*reinterpret_cast<unsigned char *>(
                             reinterpret_cast<char *>(rec) + 0x1d) &
                         (unsigned char)(1 << curCiv)) != 0) {
                        int fy = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 0xa);
                        int fx = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 8);
                        reinterpret_cast<Console *>(g_009156b0)->focus(fy, fx, owner);
                        didFocus = true;
                    }
                    *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(rec) - 6) |= 4;
                    if (didFocus) {
                        int by = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 0xa);
                        int bx = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 8);
                        boom(by, bx, 0x80);
                    }

                    // Point/cost update against the type table.
                    int subT = *rec * 0x34;
                    int sub2 = tbl_9ab88c[subT];
                    unsigned char curPoint =
                        *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(rec) + 6);
                    signed char delta;
                    if (tbl_94a379[sub2 * 0x90 + 1] == 1 && sub2 == 6) {
                        signed char growth = (signed char)tbl_9ab88c[subT + 3];
                        delta = (signed char)(growth * 3);
                    } else {
                        int val;
                        if (tbl_9ab88c[subT + 6] == 0x0c) {
                            val = 1;
                        } else {
                            unsigned char growthU = tbl_9ab88c[subT + 3];
                            int u = growthU;
                            if (growthU == 0) u = 1;
                            else if (growthU > 100) u = 100;
                            val = u * 10;
                        }
                        curPoint =
                            *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(rec) + 6);
                        int diff2 = val - curPoint;
                        if (diff2 < 0) diff2 = 0;
                        else if (diff2 > 9999) diff2 = 9999;
                        delta = (signed char)diff2;
                    }
                    *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(rec) + 6) =
                        (unsigned char)(curPoint + delta);

                    // Same type table, decide kill vs draw_tile.
                    int subT2 = *rec * 0x34;
                    int val2;
                    if (tbl_9ab88c[subT2 + 6] == 0x0c) {
                        val2 = 1;
                    } else {
                        unsigned char growthU2 = tbl_9ab88c[subT2 + 3];
                        int u2 = growthU2;
                        if (growthU2 == 0) u2 = 1;
                        else if (growthU2 > 100) u2 = 100;
                        val2 = u2 * 10;
                    }
                    unsigned char curPoint2 =
                        *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(rec) + 6);
                    int delta2 = val2 - curPoint2;
                    if (delta2 <= 0) {
                        kill(idx);
                    } else {
                        *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(rec) + 0x1b) = 0;
                        int ty = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 0xa);
                        int tx = *reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 8);
                        draw_tile(ty, tx, 2);
                    }
                }
            }
        }
        rec = reinterpret_cast<short *>(reinterpret_cast<char *>(rec) - 0x34);
    } while (--idx >= 0);
}
