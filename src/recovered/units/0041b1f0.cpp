// ORIGINAL: 0x0041B1F0 ?on_mouse_move@BaseWin@@QAEXHH@Z 0x0041B1F0-0x0041B491 FILE
// size      673 bytes
// prototype void (__thiscall ?on_mouse_move@BaseWin@@QAEXHH@Z)(Win* this, int xCoord, int yCoord)
// callers   0   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00409100 0x0040E360 0x004E39D0 0x004E4020 0x004EC3B0 0x00530D50 0x00530E50 0x005EC740 0x005F7E90 0x005F8500 0x005FAB00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0041B1F0
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041b1f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@BaseWin@@QAEXHH@Z  at 0x0041B1F0  (673 bytes)
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
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
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

// Moved ahead of Buffer: the emitted scaffold placed the full Spot
// definition after Buffer used it by value (`Spot spot_;`), which VC6
// rejects as C2079 (undefined class). A fresh emit does not have this
// ordering bug; this respells the on-disk scaffold to match it (rule 5).
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
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
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
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
    int set_cursor(Sprite *, int, int);
    void __stdcall clear_bubble_text();
};
int __cdecl base_compute(int);
int __cdecl best_specialist();
void __cdecl set_base(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093e8c8 = (int *)0x0093E8C8;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a6488 = (int *)0x009A6488;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    int farm_get(int, int);
    void draw_production(int);
    void on_mouse_move(int, int);
};

// INDEXED TABLE BASE: these five addresses are combined with a
// register-computed offset before use (array index / sprite-table lookup),
// so they need extern arrays rather than the context's pointer constants.
extern int g_0097d0b8_arr[];
extern uint8_t g_00946f58_arr[];
extern uint8_t g_007764d0_arr[];
extern uint8_t g_00779b78_arr[];
extern uint8_t g_0075b180_arr[];
extern uint8_t g_0075b710_arr[];

void BaseWin::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (!reinterpret_cast<Win *>(self)->is_visible()) {
        return;
    }
    if (*g_0093f660 != 0 && *g_0093e8c8 != 0) {
        return;
    }

    reinterpret_cast<Win *>(self)->clear_bubble_text();
    reinterpret_cast<Win *>(self)->clear_bubble_text();

    if (*reinterpret_cast<int *>(self + 0x40b00) >= 0) {
        *reinterpret_cast<int *>(self + 0x40b00) = -1;
        *reinterpret_cast<int *>(self + 0x40b04) = 0;
        int idx = *reinterpret_cast<int *>(self + 0x40b0c);
        int old = g_0097d0b8_arr[idx * 0x4d];
        set_base(idx);
        base_compute(0);
        if (*g_0093f660 != 0 && *g_0093a938 == 0 &&
            old != g_0097d0b8_arr[idx * 0x4d]) {
            if (reinterpret_cast<NetDaemon *>(g_0093cd90)
                    ->lock_base(*g_00689370, 0, -1, -1) == 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
            }
        }
        this->draw_production(1);
    }

    if (*reinterpret_cast<int *>(self + 0x40b4c) != 0 &&
        *reinterpret_cast<int *>(self + 0x40b4c) != 1) {
        return;
    }
    if (*reinterpret_cast<int *>(self + 0x40b54) < 0) {
        return;
    }

    int loc_4, loc_8;
    int r = reinterpret_cast<Spot *>(self + 0x40ec0)->check(a1, a2, &loc_8, &loc_4);
    if (r < 0) {
        return;
    }

    int iVar1 = *reinterpret_cast<int *>(self + 0x40b54);
    int iVar3 = 1;

    if (*reinterpret_cast<int *>(self + 0x40b4c) == loc_4) {
        if (*reinterpret_cast<int *>(self + 0x40b4c) == 0) {
            int iVar4 = this->farm_get(*reinterpret_cast<int *>(self + 0x40b44),
                                        *reinterpret_cast<int *>(self + 0x40b48));
            if (iVar4 >= 0) {
                iVar4 = this->farm_get(a1, a2);
                int iVar5 = this->farm_get(*reinterpret_cast<int *>(self + 0x40b44),
                                            *reinterpret_cast<int *>(self + 0x40b48));
                *reinterpret_cast<unsigned int *>(self + 0x40b54) =
                    static_cast<unsigned int>(iVar5 != iVar4) + 1;
            }
        } else if (*reinterpret_cast<int *>(self + 0x40b4c) == 1) {
            *reinterpret_cast<unsigned int *>(self + 0x40b54) =
                static_cast<unsigned int>(loc_8 != *reinterpret_cast<int *>(self + 0x40b50)) + 1;
        }
    } else {
        *reinterpret_cast<int *>(self + 0x40b54) = 2;
    }

    if (*reinterpret_cast<int *>(self + 0x40b54) > 0 && iVar1 == 0) {
        if (*reinterpret_cast<int *>(self + 0x40b4c) == 1) {
            int u = *reinterpret_cast<int *>(self + 0x40b50);
            if (u < 0) {
                iVar3 = 1;
            } else if (u < 0x10) {
                char *base = *reinterpret_cast<char **>(g_0090ea30);
                iVar3 = ((*reinterpret_cast<int *>(base + 0x84 + (u >> 3) * 4)) >>
                         ((u & 7) << 2) & 0xf) + 4;
            } else {
                iVar3 = best_specialist() + 4;
            }
        }

        uint8_t *sprite;
        char *base2 = *reinterpret_cast<char **>(g_0090ea30);
        unsigned int flagidx = static_cast<unsigned char>(*(base2 + 4));
        if (*g_009a6488 == 0 || (g_00946f58_arr[flagidx * 0x59c] & 0x80) == 0) {
            if (iVar3 < 4) {
                sprite = g_007764d0_arr + iVar3 * 0x2c;
            } else {
                sprite = g_00779b78_arr + (iVar3 - 4) * 0x2c;
            }
        } else {
            if (iVar3 < 4) {
                sprite = g_0075b180_arr + iVar3 * 0x2c;
            } else {
                sprite = g_0075b710_arr + (iVar3 - 4) * 0x2c;
            }
        }
        reinterpret_cast<Win *>(self)->set_cursor(reinterpret_cast<Sprite *>(sprite), 7, 9);
        reinterpret_cast<Win *>(g_007ae820)->set_cursor(reinterpret_cast<Sprite *>(sprite), 7, 9);
    }
}
