// ORIGINAL: 0x004D26A0 ?give_veh@Console@@QAEXH@Z 0x004D26A0-0x004D2ABC FILE
// size      1052 bytes
// prototype void (__thiscall ?give_veh@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   2   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x0048C0A0 0x004CEDE0 0x004E3EF0 0x00531020 0x005310F0 0x00531300 0x00592E10 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004D26A0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d26a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?give_veh@Console@@QAEXH@Z  at 0x004D26A0  (1052 bytes)
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
class NetDaemon;
class NetMsg;
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

class NetDaemon { public:
    int lock_veh(int *, int, int, int, int);
    void await_exec(int);
    void unlock_veh();
};

class NetMsg { public:
    void pop(const char *, int, int, const char *);
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

int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int __cdecl whose_territory(int, int, int, int *, int);
void __cdecl action_give(int, int);
void __cdecl message_veh(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006887e0 = (int *)0x006887E0;
static int *const g_006887ec = (int *)0x006887EC;
static int *const g_006887f8 = (int *)0x006887F8;
static int *const g_00688808 = (int *)0x00688808;
static int *const g_00688814 = (int *)0x00688814;
static int *const g_00688824 = (int *)0x00688824;
static int *const g_00688830 = (int *)0x00688830;
static int *const g_00688840 = (int *)0x00688840;
static int *const g_00688850 = (int *)0x00688850;
static int *const g_00688860 = (int *)0x00688860;
static int *const g_00688868 = (int *)0x00688868;
static int *const g_00688874 = (int *)0x00688874;
static int *const g_00688884 = (int *)0x00688884;
static int *const g_00688890 = (int *)0x00688890;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab88d = (int *)0x009AB88D;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void give_veh(int);
};

// Per-vehicle record table at 0x952828 (stride 0x34): short X@0, short Y@2,
// short unit-type-id@0xA, byte owner-faction@0xE. Same table documented in
// src/recovered/units/0041a620.cpp and 004d24c0.cpp.
extern char g_00952828_records[];

// Per-unit-type-id record table at 0x9AB88C (stride 0x34): byte@0 (used with
// g_0094a379_records, see 004d24c0.cpp) and a second byte@1 (0x9AB88D) used
// with g_0094ae68_records below.
extern char g_009ab88c_records[];

// Per-value-of-the-0x9AB88D-byte record at 0x94AE68 (stride 0x10): signed
// byte@0, sign tested for a "can carry/move" style flag.
extern signed char g_0094ae68_records[];

// Per-type-index record table at 0x94A379 (stride 0x90): byte class@0
// (0=land,1=sea,2=air per 004d24c0.cpp).
extern unsigned char g_0094a379_records[];

// Per-faction record table at 0x946D34 (stride 0x59C, same stride family as
// g_diplo_name_table@0x946A50 in DiploWin::draw_who): dword@0x18, dword@0x1C
// staged into g_009bbfec/g_009bbff0 before parse_says, record base used as a
// name string.
extern char g_00946d34_records[];

// Per-unit-type-id string table at 0x9AB868 (stride 0x34).
extern char g_009ab868_records[];

// Per-faction flag byte at 0x946F58 (stride 0x59C), bit 0x80 tested. Same
// table as g_00946f58_tbl in src/recovered/units/004d2de0.cpp.
extern unsigned char g_00946f58_records[];

// Diplomatic relation flags: dword per (my_faction, other_faction) pair,
// row stride 0x20CC bytes (2099 ints); bits 0x8 and 0x10 tested.
extern int g_0096c9f8_records[];

// g_0094a30c, g_00939284, g_0068faf0, g_0093f660, g_00691b0c are already
// declared by the scaffold (plain-load pointer constants); used as-is.

void Console::give_veh(int a1) {
    char *self = reinterpret_cast<char *>(this);

    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&a1, 4, -1, -1, 0) != 0) {
        return;
    }

    unsigned char owner = *reinterpret_cast<unsigned char *>(g_00952828_records + a1 * 0x34 + 0xE);
    if (owner != *g_00939284) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        return;
    }

    short vehY = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 2);
    short vehX = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0);
    int myFaction = *reinterpret_cast<int *>(self + 0x23bd4);
    int destFaction = whose_territory(*g_00939284, vehX, vehY, 0, 0);

    if (destFaction == myFaction || destFaction < 0 ||
        (g_0096c9f8_records[myFaction * 2099 + destFaction] & 8) == 0 ||
        (g_0096c9f8_records[myFaction * 2099 + destFaction] & 0x10) != 0) {
        short unitType = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0xA);
        unsigned char idxB =
            *reinterpret_cast<unsigned char *>(g_009ab88c_records + unitType * 0x34 + 1);
        signed char moveFlag = g_0094ae68_records[idxB * 0x10];
        if (unitType > 0x3f || (moveFlag >= 0 && unitType != 0xf)) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                reinterpret_cast<const char *>(g_006887e0), 5000, 0, 0);
            return;
        }
        destFaction = 0;
    }

    short unitType = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0xA);
    unsigned char idxA =
        *reinterpret_cast<unsigned char *>(g_009ab88c_records + unitType * 0x34);
    unsigned char unitClass = g_0094a379_records[idxA * 0x90];

    if (unitClass == 2) {
        int tileIndex = *g_0068faf0 * vehY + (vehX >> 1);
        char *tile = *reinterpret_cast<char **>(g_0094a30c) + tileIndex * 0x2c;
        bool blocked = (tile[8] & 1) == 0 || (tile[2] & 0xf) > 7;
        if (blocked && (*reinterpret_cast<unsigned int *>(tile + 8) & 0x40000) == 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
            reinterpret_cast<NetMsg *>(g_00805338)->pop(
                reinterpret_cast<const char *>(g_006887ec), 5000, 0, 0);
            return;
        }
    }

    unsigned char idxB2 =
        *reinterpret_cast<unsigned char *>(g_009ab88c_records + unitType * 0x34 + 1);
    signed char moveFlag2 = g_0094ae68_records[idxB2 * 0x10];

    char *msgA;
    char *msgB;
    int result;

    if (unitType < 0x40 && (moveFlag2 < 0 || unitType == 0xf)) {
        *reinterpret_cast<int *>(g_009bbfec) =
            *reinterpret_cast<int *>(g_00946d34_records + destFaction * 0x59c + 0x18);
        *reinterpret_cast<int *>(g_009bbff0) =
            *reinterpret_cast<int *>(g_00946d34_records + destFaction * 0x59c + 0x1c);
        parse_says(0, g_00946d34_records + destFaction * 0x59c, -1, -1);
        parse_says(1, g_009ab868_records + unitType * 0x34, -1, -1);

        if (destFaction != 0) {
            result = popp(*reinterpret_cast<char **>(g_00691b0c),
                          reinterpret_cast<const char *>(g_00688808), 0,
                          reinterpret_cast<const char *>(g_006887f8), 0);
            if (result == 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
                return;
            }
            if (result == 2) {
                destFaction = 0;
            }
            goto give_or_message;
        }
        msgB = reinterpret_cast<char *>(g_00688814);
        msgA = reinterpret_cast<char *>(g_00688824);
    } else {
        *reinterpret_cast<int *>(g_009bbfec) =
            *reinterpret_cast<int *>(g_00946d34_records + destFaction * 0x59c + 0x18);
        *reinterpret_cast<int *>(g_009bbff0) =
            *reinterpret_cast<int *>(g_00946d34_records + destFaction * 0x59c + 0x1c);
        parse_says(0, g_00946d34_records + destFaction * 0x59c, -1, -1);
        parse_says(1, g_009ab868_records + unitType * 0x34, -1, -1);

        if ((g_00946f58_records[myFaction * 0x59c] & 0x80) == 0) {
            if (unitClass == 2) {
                msgB = reinterpret_cast<char *>(g_00688868);
            } else {
                msgB = reinterpret_cast<char *>(g_00688874);
                if (unitClass != 1) {
                    msgB = reinterpret_cast<char *>(g_00688884);
                }
            }
            msgA = reinterpret_cast<char *>(g_00688890);
        } else if (unitClass == 2) {
            msgB = reinterpret_cast<char *>(g_00688830);
            msgA = reinterpret_cast<char *>(g_00688860);
        } else {
            msgB = reinterpret_cast<char *>(g_00688840);
            if (unitClass != 1) {
                msgB = reinterpret_cast<char *>(g_00688850);
            }
            msgA = reinterpret_cast<char *>(g_00688860);
        }
    }

    result = popp(*reinterpret_cast<char **>(g_00691b0c), msgA, 0, msgB, 0);
    if (result == 0) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        return;
    }

give_or_message:
    if (*g_0093f660 == 0) {
        action_give(a1, destFaction);
        return;
    }
    message_veh(0x2416, a1, destFaction, 0);
    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
}
