// ORIGINAL: 0x004D8A70 FILE
// name      ?automate@Console@@QAEXHH@Z
// size      601 bytes
// spans     0x004D8A70-0x004D8CC9
// prototype void (__thiscall ?automate@Console@@QAEXHH@Z)(Console* this, int vehID, uint32_t orderAutoType)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004CBAA0 0x00510490 0x00531020 0x005310F0 0x00531480 0x00593220 0x005B9F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004D8A70
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d8a70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?automate@Console@@QAEXHH@Z  at 0x004D8A70  (601 bytes)
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
    void await_synch();
    void unlock_veh();
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

bool __cdecl has_tech(int, int);
int __cdecl action_home(int, int);
void __cdecl synch_veh(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691940 = (int *)0x00691940;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_009ab88c = (int *)0x009AB88C;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void cursor_on(int, int);
    void automate(int, int);
};

// EXTERN-SYMBOL LEVER: every table below is indexed with a register-scaled
// offset (`[reg*N + imm]` or `[reg + imm]` where reg already holds a
// computed record*stride byte offset), not a plain load/store, so a
// `static int *const` would let /O2 fold the address literal and drop the
// addressing mode. Extern arrays keep it. Vehicle-record fields share a
// 0x34-byte stride; unit-type fields share a 0x34-byte stride too;
// faction-category fields use a 0x90-byte stride.
extern short g_00952828_arr[];  // veh.x           (index a1*0x1a)
extern short g_0095282a_arr[];  // veh.y           (index a1*0x1a)
extern int   g_0095282c_arr[];  // veh.flags       (index a1*0xd)
extern short g_00952832_arr[];  // veh.unit_type   (index a1*0x1a)
extern char  g_00952836_arr[];  // veh.tech_req    (index a1*0x34)
extern char  g_00952839_arr[];  // veh.state1      (index a1*0x34)
extern char  g_0095283a_arr[];  // veh.state2      (index a1*0x34)
extern short g_0095283c_arr[];  // veh.altX        (index a1*0x1a)
extern short g_0095283e_arr[];  // veh.dispX       (index a1*0x1a)
extern short g_00952844_arr[];  // veh.altY        (index a1*0x1a)
extern short g_00952846_arr[];  // veh.dispY       (index a1*0x1a)
extern char  g_0095284e_arr[];  // veh.order       (index a1*0x34)

extern char g_009ab88c_arr[];   // unittype.factionCat  (index unitType*0x34)
extern char g_009ab88d_arr[];   // unittype.cat2        (index unitType*0x34)
extern char g_009ab892_arr[];   // unittype.triggerType (index unitType*0x34)

extern char g_0094a379_arr[];   // faction-cat table field0 (index fac*0x90)
extern char g_0094a37a_arr[];   // faction-cat table field1 (index fac*0x90)
extern char g_0094ae68_arr[];   // table (index fac2*0x10)

void Console::automate(int a1, int a2) {
    int unitType = g_00952832_arr[a1 * 0x1a];

    bool goLock = false;
    if (g_009ab892_arr[unitType * 0x34] == 9) {
        if (a2 == 2 || a2 == 1) {
            unsigned char fac = g_009ab88c_arr[unitType * 0x34];
            if (g_0094a379_arr[fac * 0x90] != 0) {
                return;
            }
            int tileIdx = g_0095282a_arr[a1 * 0x1a] * (*g_0068faf0) +
                          (g_00952828_arr[a1 * 0x1a] >> 1);
            unsigned char *tile = reinterpret_cast<unsigned char *>(*g_0094a30c) + tileIdx * 0x2c;
            if ((*tile & 0xe0) < 0x60) {
                return;
            }
        }
        if (a2 == 2) {
            if (!has_tech(*g_00691940, g_00952836_arr[a1 * 0x34])) {
                return;
            }
            goLock = true;
        }
        if (!goLock && (a2 == 5 || a2 == 4)) {
            int tileIdx = g_0095282a_arr[a1 * 0x1a] * (*g_0068faf0) +
                          (g_00952828_arr[a1 * 0x1a] >> 1);
            unsigned char *tile = reinterpret_cast<unsigned char *>(*g_0094a30c) + tileIdx * 0x2c;
            if ((tile[8] & 1) != 0 && (tile[2] & 0xf) < 8) {
                return;
            }
        }
    }

    if (!goLock && a2 == 10) {
        if (g_0094a379_arr[static_cast<unsigned char>(g_009ab88c_arr[unitType * 0x34]) * 0x90] != 2) {
            return;
        }
        if (g_0094ae68_arr[static_cast<unsigned char>(g_009ab88d_arr[unitType * 0x34]) * 0x10] == 0) {
            return;
        }
        g_00952839_arr[a1 * 0x34] = 0;
        reinterpret_cast<Console *>(g_009156b0)->cursor_on(0xb, -1);
        return;
    }

    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&a1, 0, -1, -1, 0) == 0) {
        if (a2 == 0xb) {
            g_0095283e_arr[a1 * 0x1a] = g_00952828_arr[a1 * 0x1a];
            g_00952846_arr[a1 * 0x1a] = g_0095282a_arr[a1 * 0x1a];
            int ut = g_00952832_arr[a1 * 0x1a];
            if (g_0094a37a_arr[static_cast<unsigned char>(g_009ab88c_arr[ut * 0x34]) * 0x90] != 0) {
                action_home(a1, 0);
                if (g_00952839_arr[a1 * 0x34] == 0x18) {
                    g_0095283e_arr[a1 * 0x1a] = g_0095283c_arr[a1 * 0x1a];
                    g_00952846_arr[a1 * 0x1a] = g_00952844_arr[a1 * 0x1a];
                }
                g_00952839_arr[a1 * 0x34] = 0;
            }
        }
        g_0095284e_arr[a1 * 0x34] = static_cast<char>(a2);
        g_00952839_arr[a1 * 0x34] = 0;
        g_0095282c_arr[a1 * 0xd] = (g_0095282c_arr[a1 * 0xd] & 0xfcffffff) | 0x200;
        g_0095283a_arr[a1 * 0x34] = 0;
        synch_veh(a1);
        reinterpret_cast<NetDaemon *>(g_0093cd90)->await_synch();
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
    }
}
