// ORIGINAL: 0x004D24C0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004D24C0
// name           ?hold@Console@@QAEXHH@Z
// size           471 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d24c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hold@Console@@QAEXHH@Z  at 0x004D24C0  (471 bytes)
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
class BaseWin { public:
    void check_veh(int);
};
class NetDaemon { public:
    int lock_veh(int *, int, int, int, int);
    int order_veh(int, int, int);
    void await_synch();
    void unlock_veh();
};
void __cdecl draw_tile(int, int, int);
void __cdecl synch_veh(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab892 = (int *)0x009AB892;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void hold(int, int);
};

// Per-vehicle record array at 0x952828 (stride 0x34): short X @0, short Y
// @2, short unit-type-id @0xA, byte @0xE, byte state @0x11, short @0x1C.
// Per-unit-type-id record array at 0x9AB88C (stride 0x34): byte
// type-index @0, byte @6. Per-type-index record array at 0x94A379 (stride
// 0x90): byte @0, byte @1. All three are indexed by a register with the
// address itself doing the work (the disp32 base is folded straight into
// scale-4 SIB addressing) - extern arrays, not pointer constants.
extern char g_00952828_records[];
extern char g_009ab88c_records[];
extern char g_0094a379_records[];

// NOT BYTE_EXACT. Closest reached: 476/471 bytes, mnemonic similarity
// 0.78, edit_count 27. Written as one time-boxed pass given the
// function's size (471 bytes) and branch count; not iterated further.
// Logic transcribed instruction-by-instruction from the disassembly
// (goto labels named for their original addresses) rather than
// restructured, because the two chained OR conditions each have a
// non-tautological exit in the middle of their AND-chain (the "flag==0
// or nibble>=8" checks leave via a DIFFERENT label than falling off the
// end), which a flattened boolean would not reproduce. Cross-checked
// against Ghidra's reconstruction, which gets the two big conditions'
// boolean SHAPE right but obscures the exact fields being compared
// because it re-derives the same table index three separate times from
// three different multiplier spellings (0x1a-strided shorts, 0x34-strided
// bytes) that are all the SAME 0x34-byte record read from the
// disassembly's single `lea`-chain per table.
void Console::hold(int a1, int a2) {
    int lockOut = a1;
    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&lockOut, 0, -1, -1, 0) != 0) {
        goto L_end;
    }

    {
        short vehX = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0);
        short vehY = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 2);
        int xHalf = vehX >> 1;
        int index = *g_0068faf0 * vehY + xHalf;
        char *tile = *reinterpret_cast<char **>(g_0094a30c) + index * 0x2c;

        if ((tile[0] & 0xe0) >= 0x60) {
            goto L_256a;
        }
        {
            short unitId = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0xA);
            unsigned char typeIdx =
                *reinterpret_cast<unsigned char *>(g_009ab88c_records + unitId * 0x34);
            if (*reinterpret_cast<unsigned char *>(g_0094a379_records + typeIdx * 0x90) != 0) {
                goto L_256a;
            }
        }
        if ((tile[8] & 1) == 0) {
            goto L_25b9;
        }
        {
            int nibble = tile[2] & 0xf;
            if (nibble >= 8) {
                goto L_25b9;
            }
            if (nibble < 0) {
                goto L_25b9;
            }
        }

    L_256a:
        {
            short unitId = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0xA);
            unsigned char typeIdx =
                *reinterpret_cast<unsigned char *>(g_009ab88c_records + unitId * 0x34);
            if (*reinterpret_cast<unsigned char *>(g_0094a379_records + typeIdx * 0x90) != 2) {
                goto L_25cc;
            }
            if (*reinterpret_cast<unsigned char *>(g_0094a379_records + typeIdx * 0x90 + 1) == 0) {
                goto L_25cc;
            }
        }
        if ((tile[8] & 1) != 0) {
            int nibble = tile[2] & 0xf;
            if (nibble < 8) {
                goto L_25cc;
            }
        }
        if ((*reinterpret_cast<unsigned int *>(tile + 8) & 0x40000) != 0) {
            goto L_25cc;
        }

    L_25b9:
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        goto L_end;

    L_25cc:
        *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0x1C) =
            static_cast<short>(a2);
        *reinterpret_cast<unsigned char *>(g_00952828_records + a1 * 0x34 + 0x11) = 2;
        synch_veh(a1);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
        reinterpret_cast<BaseWin *>(g_006a7628)->check_veh(a1);
        draw_tile(vehX, vehY, 1);

        {
            short unitId2 = *reinterpret_cast<short *>(g_00952828_records + a1 * 0x34 + 0xA);
            if (*reinterpret_cast<unsigned char *>(g_009ab88c_records + unitId2 * 0x34 + 6) !=
                0xc) {
                goto L_end;
            }
            int index2 = *g_0068faf0 * vehY + xHalf;
            char *tile2 = *reinterpret_cast<char **>(g_0094a30c) + index2 * 0x2c;
            int state;
            if ((tile2[8] & 1) == 0) {
                state = -1;
            } else {
                state = tile2[2] & 0xf;
                if (state > 7) {
                    state = -1;
                }
            }
            unsigned char wanted =
                *reinterpret_cast<unsigned char *>(g_00952828_records + a1 * 0x34 + 0xE);
            if (static_cast<unsigned char>(state) == wanted) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->order_veh(a1, -1, 0);
            }
        }
    }

L_end:;
}
