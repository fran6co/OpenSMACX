// ORIGINAL: 0x004CEDE0 FILE
// name      ?action_give@@YAXHH@Z
// size      1433 bytes
// spans     0x004CEDE0-0x004CF379
// prototype void (__cdecl ?action_give@@YAXHH@Z)(int vehID, int protoID)
// callers   3   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046AEF0 0x0047A890 0x004E3B80 0x004E3C60 0x005108A0 0x00580860 0x00591B10 0x005BF310 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004CEDE0
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004cede0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_give@@YAXHH@Z  at 0x004CEDE0  (1433 bytes)
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
class NetMsg;
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
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void focus(int, int, int);
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
int __cdecl X_pop(const char *, int (__cdecl *)());
int __cdecl base_find(int, int);
int __cdecl base_find(int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl propose_proto(int, int, int, int, int, int, int, char *);
void __cdecl draw_radius(int, int, int, int);
void __cdecl owner_set(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068828c = (int *)0x0068828C;
static int *const g_0068829c = (int *)0x0068829C;
static int *const g_006882ac = (int *)0x006882AC;
static int *const g_006882b4 = (int *)0x006882B4;
static int *const g_006882c0 = (int *)0x006882C0;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095285a = (int *)0x0095285A;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab868 = (int *)0x009AB868;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab892 = (int *)0x009AB892;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;
static int *const g_009bc054 = (int *)0x009BC054;

// Indexed-table bases (base+register addressing) the emitter's immediate
// scan does not catch: `extern T name[];`, not a pointer constant, per the
// measured indexed-table-base lever.
extern char g_952828x[]; // per-unit record, stride 0x34 (shared with 0x004B43E0's record)
extern char g_9ab888x[]; // unit-type chassis record, stride 0x34: int at +0, bytes at +4..+7
extern char g_9ab892x[]; // per-(player,unit-type) availability table, stride 0x34
extern char g_9ab893x[]; // per-(player,unit-type) colour/flags table, stride 0x34
extern char g_94a379x[]; // per-unit-type record, stride 0x90, byte at +0
extern char g_946a50x[]; // per-faction diplomacy record, stride 0x59c: int at +0, +0x4c, +0x34
extern char g_946d34x[]; // per-faction diplomacy record, stride 0x59c: int at +0x18,+0x1c,+0x20
extern char g_94a30cx[]; // per-tile record, stride 0x2c
extern char g_96e35cx[]; // per-player x per-tile-terrain table, stride 0x20cc
extern char g_97d053x[]; // per-base record, stride 0x134

// WALL: this function's original ~40-way nested fallback search (four
// successive relaxed-criteria scans of the unit-type tables, each
// threading a shared "found at this depth" value through several scope
// exits) is reproduced here as independent sequential searches instead
// of the original's single register-carried value across nested loop
// breaks. Faithful in call sequence, offsets, and outcome per branch;
// not attempted at the register-allocation level (this function's size
// and branch count put an exact register-for-register match out of
// reach in this pass).
void __cdecl action_give(int a1, int a2) {
#define W(u, off) (*reinterpret_cast<short *>(g_952828x + (u) * 0x34 + (off)))
#define B(u, off) (*reinterpret_cast<unsigned char *>(g_952828x + (u) * 0x34 + (off)))
#define D(u, off) (*reinterpret_cast<unsigned int *>(g_952828x + (u) * 0x34 + (off)))

    int locX = W(a1, 2);
    int locY = W(a1, 0);
    unsigned char origOwner = B(a1, 0xe);

    if (a1 >= 0) {
        int head = W(a1, 0x32);
        int cur = a1;
        while (head >= 0) {
            cur = head;
            head = W(cur, 0x32);
        }

        for (; cur >= 0; cur = W(cur, 0x30)) {
            if (cur == a1 || (B(cur, 0x11) == 1 && W(cur, 0x14) == a1)) {
                int unitType = W(cur, 0xa);
                int destType;

                if (unitType < 0x40) {
                    destType = unitType;
                } else {
                    char *typeRec = g_9ab888x + unitType * 0x34;
                    destType = -1;
                    char *p = g_9ab888x + a2 * 0xd00;
                    for (int i = 0; i < 0x40; ++i, p += 0x34) {
                        if ((p[0xc] & 1) != 0 &&
                            p[0] == typeRec[4] && p[1] == typeRec[5] && p[2] == typeRec[6] &&
                            *reinterpret_cast<int *>(p - 4) == *reinterpret_cast<int *>(typeRec) &&
                            p[3] == typeRec[7]) {
                            int cand = a2 * 0x40 + i;
                            if (cand >= 0) {
                                destType = cand;
                            }
                            break;
                        }
                    }

                    if (destType < 0) {
                        destType = propose_proto(a2, typeRec[4], typeRec[5], typeRec[6],
                                                  *reinterpret_cast<int *>(typeRec), typeRec[7],
                                                  g_9ab893x[unitType * 0x34], 0);
                        if (destType < 0) {
                            int found2 = -1;
                            char *p2 = g_9ab888x + a2 * 0xd00;
                            for (int i = 0; i < 0x40; ++i, p2 += 0x34) {
                                if ((p2[0xc] & 1) != 0 && p2[0] == typeRec[4] && p2[1] == typeRec[5]) {
                                    found2 = a2 * 0x40 + i;
                                    break;
                                }
                            }
                            destType = found2;
                            if (destType < 0) {
                                int found3 = -1;
                                char *p3 = g_9ab892x + a2 * 0xd00;
                                for (int i = 0; i < 0x40; ++i, p3 += 0x34) {
                                    if ((p3[6] & 1) != 0 && p3[0] == g_9ab892x[unitType * 0x34]) {
                                        found3 = a2 * 0x40 + i;
                                        break;
                                    }
                                }
                                destType = found3;
                                if (destType < 0) {
                                    int found4 = -1;
                                    char *p4 = g_9ab888x + a2 * 0xd00;
                                    for (int i = 0; i < 0x40; ++i, p4 += 0x34) {
                                        if ((p4[0xc] & 1) != 0 && p4[0] == typeRec[4]) {
                                            found4 = a2 * 0x40 + i;
                                            break;
                                        }
                                    }
                                    destType = found4;
                                    if (destType < 0) {
                                        destType = 3 - ((g_94a379x[static_cast<unsigned char>(typeRec[4]) * 0x90] != 1) ? 1 : 0);
                                    }
                                }
                            }
                        }
                    }
                }

                B(cur, 0xe) = static_cast<unsigned char>(a2);
                W(cur, 0xa) = static_cast<short>(destType);
                B(cur, 0x11) = 0;
                D(cur, 4) = D(cur, 4) & 0xfcffbdff;

                int homeBase = base_find(locY, locX, a2);
                if (homeBase < 0) {
                    homeBase = -1;
                }
                W(cur, 0x2e) = static_cast<short>(homeBase);

                char *tile = g_94a30cx + ((*g_0068faf0) * locX + (locY >> 1)) * 0x2c;
                if ((tile[8] & 1) == 0 || (tile[2] & 0xf) > 7) {
                    owner_set(locY, locX, a2);
                }
            }
        }
    }

    draw_radius(locY, locX, 3, 2);

    if (a2 == *g_00939284) {
        parse_says(0, reinterpret_cast<char *>(g_009ab868) + W(a1, 0xa) * 0x34, -1, -1);
        int u11 = origOwner;
        *g_009bbff0 = 0;
        *g_009bbfec = *reinterpret_cast<int *>(g_946a50x + u11 * 0x167 * 4);
        parse_says(1, g_946a50x + u11 * 0x59c + 0x4c, -1, -1);
        *g_009bbfec = *reinterpret_cast<int *>(g_946a50x + u11 * 0x167 * 4);
        *g_009bbff0 = 0;
        parse_says(2, g_946a50x + u11 * 0x59c + 0x34, -1, -1);
        *g_009bbfec = *reinterpret_cast<int *>(g_946d34x + u11 * 0x59c + 0x18);
        *g_009bbff0 = *reinterpret_cast<int *>(g_946d34x + u11 * 0x59c + 0x1c);
        parse_says(3, g_946d34x + u11 * 0x59c, -1, -1);

        reinterpret_cast<Console *>(g_009156b0)->focus(locY, locX, u11);

        int hb = base_find(locY, locX);
        if (hb >= 0) {
            parse_says(4, g_97d053x + hb * 0x134, -1, -1);
            if (*g_009bc054 != 0 || ((1 << (origOwner & 0x1f)) & (*reinterpret_cast<unsigned char *>(g_009a64e8))) != 0) {
                reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_006882ac), 5000, 0, 0);
                return;
            }
            char *tile2 = g_94a30cx + ((*g_0068faf0) * locX + (locY >> 1)) * 0x2c;
            unsigned char terrain = *reinterpret_cast<unsigned char *>(tile2 + 3);
            if (g_96e35cx[a2 * 0x20cc + terrain] != 2 && g_9ab892x[W(a1, 0xa) * 0x34] != 2) {
                X_pop(reinterpret_cast<const char *>(g_0068829c), 0);
                return;
            }
            X_pop(reinterpret_cast<const char *>(g_0068828c), 0);
            return;
        }
    } else if (origOwner == *g_00939284) {
        *g_009bbfec = *reinterpret_cast<int *>(g_946d34x + a2 * 0x59c + 0x18);
        *g_009bbff0 = *reinterpret_cast<int *>(g_946d34x + a2 * 0x59c + 0x1c);
        parse_says(3, g_946d34x + a2 * 0x59c, -1, -1);
        if (a2 != 0) {
            reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_006882b4), 5000, 0, 0);
        } else {
            reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_006882c0), 5000, 0, 0);
        }
    }

#undef W
#undef B
#undef D
}
