// ORIGINAL: 0x00511800 ?turn_hints@Console@@QAEXXZ 0x00511800-0x00511BA8 FILE
// size      936 bytes
// prototype void (__thiscall ?turn_hints@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BA5F0 0x004BE650 0x004E3C60 0x005108A0 0x005BF330 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00511800
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00511800/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?turn_hints@Console@@QAEXXZ  at 0x00511800  (936 bytes)
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
    int tut_map(const char *, int, int, int, Sprite *, int, int, int);
};
int __cdecl X_pop(char *, const char *, int (__cdecl *)());
int __cdecl base_find(int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl tut_check(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a5e0 = (int *)0x0068A5E0;
static int *const g_0068a5ec = (int *)0x0068A5EC;
static int *const g_0068a5f8 = (int *)0x0068A5F8;
static int *const g_0068a604 = (int *)0x0068A604;
static int *const g_00691b14 = (int *)0x00691B14;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_008cc210 = (int *)0x008CC210;
static int *const g_008cc228 = (int *)0x008CC228;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void focus(int, int, int);
    void turn_hints();
};

// Indexed table bases: each address here is added to a RUNTIME-COMPUTED
// offset (a register), never used as a bare load/store, so it must be
// `extern` rather than a folded pointer constant - the const-pointer form
// gets its literal folded away under /O2 and the addressing instruction
// that carries the index disappears with it.
extern int tbl_939340[];
extern int tbl_9392c0[];
extern char tbl_97d040[];
extern char tbl_97d042[];
extern char tbl_97d053[];
extern char tbl_952832[];
extern char tbl_9ab868[];
extern char tbl_96c9f4[];
extern unsigned int tbl_96c9f8[];
extern char tbl_96da3c[];
extern char tbl_96da48[];
extern unsigned char tbl_96d238[];
extern unsigned char tbl_96d438[];
extern char tbl_946a50[];
extern char tbl_946a84[];
extern char tbl_946a9c[];
extern char tbl_946d34[];
extern char tbl_946d4c[];
extern char tbl_946d50[];

void Console::turn_hints() {
    if ((*reinterpret_cast<uint8_t *>(0x009A6490) & 0x20) == 0)
        return;
    if (*g_0093f660 != 0)
        return;
    if (*g_008cc210 != 0)
        return;

    int base = *g_00939284;
    int turn = *g_009a64d4;

    if (turn == 1 && tut_check(1) != 0) {
        int idx = *g_009392b8;
        int found = base_find(tbl_9392c0[idx], tbl_939340[idx], base);
        if (found >= 0) {
            parse_says(0, tbl_97d053 + found * 0x134, -1, -1);

            int off = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x23bdc);
            if (off >= 0) {
                int16_t nameIdx = *reinterpret_cast<int16_t *>(tbl_952832 + off * 0x34);
                parse_says(1, tbl_9ab868 + nameIdx * 0x34, -1, -1);

                int idx2 = *g_009392b8;
                int q1 = tbl_939340[idx2];
                int q2 = tbl_9392c0[idx2];
                *g_008cc228 = 0;
                reinterpret_cast<TutWin *>(g_008c6e68)->tut_map(
                    reinterpret_cast<const char *>(g_0068a5e0), q2, q1, -1, 0, 0x100, -1, -1);
            }
        }
        turn = *g_009a64d4;
    }

    bool secondBaseDone = false;
    if (turn >= 0x14) {
        int recOff = base * 0x20CC;
        if (*reinterpret_cast<int *>(tbl_96da3c + recOff) == 1 &&
            *reinterpret_cast<int *>(tbl_96da48 + recOff) >= 2 &&
            tbl_96d238[recOff] == 0 && tbl_96d438[recOff] == 0 &&
            tut_check(0x100000) != 0) {
            int idx = *g_009392b8;
            int found = base_find(tbl_9392c0[idx], tbl_939340[idx], base);
            if (found >= 0) {
                int byteOff = found * 0x134;
                int x = *reinterpret_cast<int16_t *>(tbl_97d040 + byteOff);
                int y = *reinterpret_cast<int16_t *>(tbl_97d042 + byteOff);
                focus(x, y, base);
                parse_says(0, tbl_97d053 + byteOff, -1, -1);
                x = *reinterpret_cast<int16_t *>(tbl_97d040 + byteOff);
                y = *reinterpret_cast<int16_t *>(tbl_97d042 + byteOff);
                reinterpret_cast<TutWin *>(g_008c6e68)->tut_map(
                    reinterpret_cast<const char *>(g_0068a5ec), x, y, -2 - found, 0, 0, -1, -1);
                *reinterpret_cast<int *>(tbl_96c9f4 + recOff) = *g_009a64d4;
                secondBaseDone = true;
            }
        }
    }

    if (!secondBaseDone) {
        int recOff = base * 0x20CC;
        int diff = turn - *reinterpret_cast<int *>(tbl_96c9f4 + recOff);
        if (diff >= 0x18 && tbl_96d238[recOff] == 0) {
            int level = *reinterpret_cast<int *>(tbl_96da3c + recOff);
            if (level > 1 && level <= 5) {
                int idx = *g_009392b8;
                int found = base_find(tbl_9392c0[idx], tbl_939340[idx], base);
                if (found >= 0) {
                    int byteOff = found * 0x134;
                    int x = *reinterpret_cast<int16_t *>(tbl_97d040 + byteOff);
                    int y = *reinterpret_cast<int16_t *>(tbl_97d042 + byteOff);
                    reinterpret_cast<TutWin *>(g_008c6e68)->tut_map(
                        reinterpret_cast<const char *>(g_0068a5f8), x, y, -2 - found, 0, 0, -1, -1);
                    *reinterpret_cast<int *>(tbl_96c9f4 + recOff) = *g_009a64d4;
                }
            }
        }
    }

    for (int slot = 1; slot < 8; ++slot) {
        if (slot == base)
            continue;
        unsigned int flags = tbl_96c9f8[2099 * base + slot];
        if ((flags & 8) == 0)
            continue;
        if ((flags & 0x10) != 0) {
            if (tut_check(0x80) == 0)
                return;

            int off2 = slot * 0x59C;
            *g_009bbff0 = 0;
            *g_009bbfec = *reinterpret_cast<int *>(tbl_946a50 + off2);
            parse_says(0, tbl_946a9c + off2, -1, -1);

            *g_009bbfec = *reinterpret_cast<int *>(tbl_946a50 + off2);
            *g_009bbff0 = 0;
            parse_says(1, tbl_946a84 + off2, -1, -1);

            *g_009bbfec = *reinterpret_cast<int *>(tbl_946d4c + off2);
            *g_009bbff0 = *reinterpret_cast<int *>(tbl_946d50 + off2);
            parse_says(2, tbl_946d34 + off2, -1, -1);

            char *tutor = *reinterpret_cast<char **>(g_00691b14);
            X_pop(tutor, reinterpret_cast<const char *>(g_0068a604), 0);
            return;
        }
    }
}
