// ORIGINAL: 0x004BAF20 ?tour7@TutWin@@QAEXXZ 0x004BAF20-0x004BB373 FILE
// size      1107 bytes
// prototype void (__thiscall ?tour7@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0043E960 0x0043E9A0 0x004BA9F0 0x004BC5A0 0x004BC690 0x004BDFE0 0x005F7E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BAF20
// measured tier  MISMATCH
// divergence     24
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004baf20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tour7@TutWin@@QAEXXZ  at 0x004BAF20  (1107 bytes)
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
class DesignWin;
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
class DesignWin { public:
    int exec(int, int);
    void shut_that_badboy_down();
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
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00687578 = (int *)0x00687578;
static int *const g_00687584 = (int *)0x00687584;
static int *const g_00687590 = (int *)0x00687590;
static int *const g_0068759c = (int *)0x0068759C;
static int *const g_006875ac = (int *)0x006875AC;
static int *const g_006875bc = (int *)0x006875BC;
static int *const g_006875c8 = (int *)0x006875C8;
static int *const g_006875d8 = (int *)0x006875D8;
static int *const g_006875e4 = (int *)0x006875E4;
static int *const g_006875f4 = (int *)0x006875F4;
static int *const g_00687604 = (int *)0x00687604;
static int *const g_00687610 = (int *)0x00687610;
static int *const g_00687620 = (int *)0x00687620;
static int *const g_00687630 = (int *)0x00687630;
static int *const g_0068763c = (int *)0x0068763C;
static int *const g_0068764c = (int *)0x0068764C;
static int *const g_0068765c = (int *)0x0068765C;
static int *const g_0071f2b0 = (int *)0x0071F2B0;
static int *const g_0071fd1c = (int *)0x0071FD1C;
static int *const g_0071fd5c = (int *)0x0071FD5C;
static int *const g_0071fd6c = (int *)0x0071FD6C;
static int *const g_0071fd7c = (int *)0x0071FD7C;
static int *const g_0071fd8c = (int *)0x0071FD8C;
static int *const g_0071fd9c = (int *)0x0071FD9C;
static int *const g_0071fdbc = (int *)0x0071FDBC;
static int *const g_0071fddc = (int *)0x0071FDDC;
static int *const g_0071fdec = (int *)0x0071FDEC;
static int *const g_0071fdfc = (int *)0x0071FDFC;
static int *const g_0071fe0c = (int *)0x0071FE0C;
static int *const g_0071fe3c = (int *)0x0071FE3C;
static int *const g_0071fe4c = (int *)0x0071FE4C;
static int *const g_0071fe5c = (int *)0x0071FE5C;
static int *const g_0071fe6c = (int *)0x0071FE6C;
static int *const g_0071fe7c = (int *)0x0071FE7C;
static int *const g_0071ff6c = (int *)0x0071FF6C;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_00939284 = (int *)0x00939284;

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

    void do_des(RECT *, const char *, int);
    void iface_rect(RECT *, int *, int *);
    void des_rect(RECT *, int *, int *);
    int tut_win(GraphicWin *, const char *, int, int, Sprite *, int, int, int);
    void tour7();
};

// The double-indirect "active window" resolve below reads a value from
// this global and chases its vtable (vtbl = *ptr; adjusted = ptr +
// *(int*)(vtbl+4)); the address of the global itself is a plain load, so
// the context's folded pointer constant is fine here.
static GraphicWin *resolve_win() {
    int *ptr = *reinterpret_cast<int **>(g_007d3c3c);
    if (ptr == 0) {
        return 0;
    }
    int *vtbl = reinterpret_cast<int *>(*ptr);
    int offset = *reinterpret_cast<int *>(reinterpret_cast<char *>(vtbl) + 4);
    return reinterpret_cast<GraphicWin *>(reinterpret_cast<char *>(ptr) + offset);
}

// do_des is declared returning void in the catalogue, but the call site
// tests eax afterward (`test eax,eax; je ...`) - it returns int. A shim
// with the same "this" and a corrected return type sidesteps the
// declared-void signature without touching the scaffold's declaration.
class TutWinDoDes { public:
    int do_des(RECT *, const char *, int);
};

void TutWin::tour7() {
    int wasVisible = reinterpret_cast<Win *>(g_0071f2b0)->is_visible();
    if (!wasVisible) {
        reinterpret_cast<DesignWin *>(g_0071f2b0)->exec(*g_00939284, -2);
    }

    int a3, a4;

    this->des_rect(reinterpret_cast<RECT *>(0x71fd5c), &a4, &a3);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x687578), a4,
                       a3, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fd6c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x687584), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fd7c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x687590), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fd8c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x68759c), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fd9c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875ac), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fddc), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875bc), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fdec), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875c8), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(0x71fdbc), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875d8), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(0x71ff6c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875e4), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->iface_rect(reinterpret_cast<RECT *>(0x71fd1c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x6875f4), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fe3c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x687604), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    this->des_rect(reinterpret_cast<RECT *>(0x71fe0c), &a3, &a4);
    if (this->tut_win(resolve_win(), reinterpret_cast<const char *>(0x687610), a3,
                       a4, reinterpret_cast<Sprite *>(0), 2, -1, -1) == 0) {
        goto fail;
    }

    if (reinterpret_cast<TutWinDoDes *>(this)->do_des(
            reinterpret_cast<RECT *>(0x71fe4c), reinterpret_cast<const char *>(0x687620),
            2) == 0) {
        goto fail;
    }
    if (reinterpret_cast<TutWinDoDes *>(this)->do_des(
            reinterpret_cast<RECT *>(0x71fe5c), reinterpret_cast<const char *>(0x687630),
            2) == 0) {
        goto fail;
    }
    if (reinterpret_cast<TutWinDoDes *>(this)->do_des(
            reinterpret_cast<RECT *>(0x71fe6c), reinterpret_cast<const char *>(0x68763c),
            2) == 0) {
        goto fail;
    }
    if (reinterpret_cast<TutWinDoDes *>(this)->do_des(
            reinterpret_cast<RECT *>(0x71fdfc), reinterpret_cast<const char *>(0x68764c),
            2) == 0) {
        goto fail;
    }
    if (reinterpret_cast<TutWinDoDes *>(this)->do_des(
            reinterpret_cast<RECT *>(0x71fe7c), reinterpret_cast<const char *>(0x68765c),
            2) == 0) {
        goto fail;
    }

fail:
    if (!wasVisible) {
        reinterpret_cast<DesignWin *>(g_0071f2b0)->shut_that_badboy_down();
    }
}
