// ORIGINAL: 0x004BAA70 ?tour6@TutWin@@QAEXXZ 0x004BAA70-0x004BAF12 FILE
// size      1186 bytes
// prototype void (__thiscall ?tour6@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0040F0F0 0x00411980 0x00412AA0 0x00415DD0 0x0041A920 0x004BDFE0 0x005ED240 0x005F7E90 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BAA70
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004baa70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tour6@TutWin@@QAEXXZ  at 0x004BAA70  (1186 bytes)
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
class TutWin;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class BaseWin { public:
    void draw_energy_alloc(int);
    void draw_expenses(int);
    void draw_farm(int);
    void focus();
    void zoom(int, int);
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
    int is_visible();
    void client_to_screen(int *, int *);
};
int __cdecl parse_says(int, char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687510 = (int *)0x00687510;
static int *const g_0068751c = (int *)0x0068751C;
static int *const g_00687528 = (int *)0x00687528;
static int *const g_00687534 = (int *)0x00687534;
static int *const g_00687540 = (int *)0x00687540;
static int *const g_0068754c = (int *)0x0068754C;
static int *const g_00687558 = (int *)0x00687558;
static int *const g_00687564 = (int *)0x00687564;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_006c0df4 = (int *)0x006C0DF4;
static int *const g_006e8254 = (int *)0x006E8254;
static int *const g_006e8258 = (int *)0x006E8258;
static int *const g_006e825c = (int *)0x006E825C;
static int *const g_006e8260 = (int *)0x006E8260;
static int *const g_006e8284 = (int *)0x006E8284;
static int *const g_006e8288 = (int *)0x006E8288;
static int *const g_006e828c = (int *)0x006E828C;
static int *const g_006e8290 = (int *)0x006E8290;
static int *const g_006e8334 = (int *)0x006E8334;
static int *const g_006e8338 = (int *)0x006E8338;
static int *const g_006e833c = (int *)0x006E833C;
static int *const g_006e8340 = (int *)0x006E8340;
static int *const g_006e8404 = (int *)0x006E8404;
static int *const g_006e8408 = (int *)0x006E8408;
static int *const g_006e840c = (int *)0x006E840C;
static int *const g_006e8410 = (int *)0x006E8410;
static int *const g_006e8414 = (int *)0x006E8414;
static int *const g_006e8418 = (int *)0x006E8418;
static int *const g_006e841c = (int *)0x006E841C;
static int *const g_006e8420 = (int *)0x006E8420;
static int *const g_006e8454 = (int *)0x006E8454;
static int *const g_006e8458 = (int *)0x006E8458;
static int *const g_006e845c = (int *)0x006E845C;
static int *const g_006e8460 = (int *)0x006E8460;
static int *const g_006e8464 = (int *)0x006E8464;
static int *const g_006e8468 = (int *)0x006E8468;
static int *const g_006e846c = (int *)0x006E846C;
static int *const g_006e8470 = (int *)0x006E8470;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a64cc = (int *)0x009A64CC;

class TutWin { public:
    uint8_t unmapped_0_[0x537C];
    int32_t field_537C_;
    int32_t field_5380_;
    uint8_t unmapped_5384_[0x539C - 0x5384];
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t unmapped_53B0_[0x53B8 - 0x53B0];
    int32_t field_53B8_;
    uint8_t unmapped_53BC_[0x53C4 - 0x53BC];
    int32_t field_53C4_;
    uint8_t unmapped_53C8_[0x53D4 - 0x53C8];
    int32_t field_53D4_;

    int tut_win(GraphicWin *, const char *, int, int, Sprite *, int, int, int);
    void tour6();
};

// Walked with a computed stride starting from this literal address, so the
// address itself does work (pointer increment) - needs a real relocation,
// not the scaffold's folded constant.
extern uint8_t g_0097d044_arr[];

static GraphicWin *tut_target() {
    void *obj = reinterpret_cast<void *>(*g_007d3c3c);
    if (obj == 0) {
        return 0;
    }
    int vtbl = *reinterpret_cast<int *>(obj);
    int adj = *reinterpret_cast<int *>(vtbl + 4);
    return reinterpret_cast<GraphicWin *>(reinterpret_cast<char *>(obj) + adj);
}

void TutWin::tour6() {
    if (!reinterpret_cast<Win *>(g_006a7628)->is_visible()) {
        if (*g_009a64cc > 0) {
            int i = 0;
            uint8_t *p = g_0097d044_arr;
            do {
                if (*p == *g_00939284) {
                    reinterpret_cast<BaseWin *>(g_006a7628)->zoom(i, 0);
                    break;
                }
                i++;
                p += 0x134;
            } while (i < *g_009a64cc);
        }
        if (!reinterpret_cast<Win *>(g_006a7628)->is_visible()) {
            return;
        }
    }

    reinterpret_cast<BaseWin *>(g_006a7628)->focus();
    parse_says(0, reinterpret_cast<char *>(*g_0090ea30 + 0x13), -1, -1);

    int x, y;
    x = *g_006e8284 + (*g_006e828c - *g_006e8284) / 2;
    y = *g_006e8288 + (*g_006e8290 - *g_006e8288) / 2;
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
    if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_00687510), *g_006e8284 + 5, y, 0,
                 2, -1, -1)) {
        return;
    }

    x = *g_006e8254 + (*g_006e825c - *g_006e8254) / 2;
    y = *g_006e8258 + (*g_006e8260 - *g_006e8258) / 2;
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
    if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_0068751c), x, y, 0, 2, -1, -1)) {
        return;
    }

    x = *g_006e8454 + (*g_006e845c - *g_006e8454) / 2;
    y = *g_006e8458 + (*g_006e8460 - *g_006e8458) / 2;
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
    if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_00687528), x, y, 0, 2, -1, -1)) {
        return;
    }

    x = *g_006e8414 + (*g_006e841c - *g_006e8414) / 2;
    y = *g_006e8418 + (*g_006e8420 - *g_006e8418) / 2;
    reinterpret_cast<Win *>(g_006a7628)->client_to_screen(&x, &y);
    if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_00687534), x, y, 0, 2, -1, -1)) {
        return;
    }

    x = *g_006e8284 + (*g_006e828c - *g_006e8284) / 2;
    y = *g_006e8288 + (*g_006e8290 - *g_006e8288) / 2;
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&x, &y);
    if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_00687540), *g_006e8284 + 5, y, 0,
                 2, -1, -1)) {
        return;
    }

    reinterpret_cast<BaseWin *>(g_006a7628)->draw_energy_alloc(2);

    if (!reinterpret_cast<Win *>(g_006c0df4)->is_visible()) {
        x = *g_006e8334 + (*g_006e833c - *g_006e8334) / 2;
        y = *g_006e8338 + (*g_006e8340 - *g_006e8338) / 2;
        reinterpret_cast<Win *>(g_006a7628)->client_to_screen(&x, &y);
        if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_0068754c), x, y, 0, 2, -1,
                     -1)) {
            return;
        }
    } else {
        x = *g_006e8464 + (*g_006e846c - *g_006e8464) / 2;
        y = *g_006e8468 + (*g_006e8470 - *g_006e8468) / 2;
        reinterpret_cast<Win *>(g_006a7628)->client_to_screen(&x, &y);
        if (!tut_win(tut_target(), reinterpret_cast<const char *>(g_00687558), x, y, 0, 2, -1,
                     -1)) {
            return;
        }
    }

    reinterpret_cast<BaseWin *>(g_006a7628)->draw_expenses(1);
    reinterpret_cast<BaseWin *>(g_006a7628)->draw_farm(2);

    x = *g_006e8404 + (*g_006e840c - *g_006e8404) / 2;
    y = *g_006e8408 + (*g_006e8410 - *g_006e8408) / 2;
    reinterpret_cast<Win *>(g_006a7628)->client_to_screen(&x, &y);
    if (tut_win(tut_target(), reinterpret_cast<const char *>(g_00687564), x, y, 0, 2, -1, -1)) {
        reinterpret_cast<BaseWin *>(g_006a7628)->draw_farm(1);
    }
}
