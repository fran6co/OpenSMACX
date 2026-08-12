// ORIGINAL: 0x004D5D30 FILE
// name      ?oblit@Console@@QAEXH@Z
// size      564 bytes
// spans     0x004D5D30-0x004D5F64
// prototype void (__thiscall ?oblit@Console@@QAEXH@Z)(Console* this, int vehID)
// callers   2   call targets   13
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x0048C0A0 0x004CD4E0 0x004E3A50 0x0052DC70 0x00530AA0 0x00531020 0x005310F0 0x00531300 0x00592E10 0x005AC060 0x005C1B70 0x00625EC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004D5D30
// measured tier  MISMATCH
// divergence     131
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d5d30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?oblit@Console@@QAEXH@Z  at 0x004D5D30  (564 bytes)
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
class NetDaemon { public:
    int add_global();
    int lock_veh(int *, int, int, int, int);
    void await_exec(int);
    void unlock_veh();
};
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
int __cdecl base_at(int, int);
int __cdecl is_objective(int);
int __cdecl not_my_turn();
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int __cdecl veh_ready(int);
void __cdecl action_oblit(int, int);
void __cdecl message_veh(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00688ccc = (int *)0x00688CCC;
static int *const g_00688cd4 = (int *)0x00688CD4;
static int *const g_00688ce0 = (int *)0x00688CE0;
static int *const g_00688cec = (int *)0x00688CEC;
static int *const g_00688cfc = (int *)0x00688CFC;
static int *const g_00688d10 = (int *)0x00688D10;
static int *const g_00688d20 = (int *)0x00688D20;
static int *const g_00688d28 = (int *)0x00688D28;
static int *const g_00688d30 = (int *)0x00688D30;
static int *const g_00688d40 = (int *)0x00688D40;
static int *const g_00688d48 = (int *)0x00688D48;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00949860 = (int *)0x00949860;
static int *const g_0097d044 = (int *)0x0097D044;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void oblit(int);
};

// EXTERN-SYMBOL LEVER: all four tables are reached through a computed,
// register-scaled row index (idx*0x34, idx*0x134, faction*0x59c), not a
// plain load, so they need real linkage rather than a folded literal.
extern int16_t g_00952828[];
extern uint8_t g_00952836[];
extern uint8_t g_0097d044_sym[];
extern uint8_t g_00946f58_sym[];

void Console::oblit(int a1) {
    int faction = g_00952836[a1 * 52];
    int x = g_00952828[a1 * 26 + 0];
    int y = g_00952828[a1 * 26 + 1];

    if (not_my_turn()) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_00688ccc), 0x1388, 0, 0);
        return;
    }
    if (!veh_ready(a1)) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_00688cd4), 0x1388, 0, 0);
        return;
    }
    int baseId = base_at(x, y);
    if (baseId < 0) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_00688ce0), 0x1388, 0, 0);
        return;
    }
    if (g_0097d044_sym[baseId * 0x134] != faction) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_00688cec), 0x1388, 0, 0);
        return;
    }
    if (is_objective(baseId)) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(reinterpret_cast<const char *>(g_00688cfc), 0x1388, 0, 0);
        return;
    }

    if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_veh(&a1, 4, -1, -1, 0) == 0) {
        baseId = base_at(x, y);
        if (baseId >= 0 && g_0097d044_sym[baseId * 0x134] == faction) {
            parse_says(0, reinterpret_cast<char *>(&g_0097d044_sym[baseId * 0x134]) + 0xf, -1, -1);

            const char *msg;
            const char *pcx;
            if ((g_00946f58_sym[faction * 0x59c] & 0x80) == 0) {
                msg = reinterpret_cast<const char *>(g_00688d40);
                if (*g_00949860 == 0) {
                    msg = reinterpret_cast<const char *>(g_00688d48);
                }
                pcx = reinterpret_cast<const char *>(g_00688d30);
            } else {
                msg = reinterpret_cast<const char *>(g_00688d20);
                if (*g_00949860 == 0) {
                    msg = reinterpret_cast<const char *>(g_00688d28);
                }
                pcx = reinterpret_cast<const char *>(g_00688d10);
            }

            if (popp(reinterpret_cast<char *>(g_00691b0c), msg, 0, pcx, 0) == 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_veh();
                return;
            }
            if (reinterpret_cast<NetDaemon *>(g_0093cd90)->add_global() == 0) {
                if (*g_0093f660 != 0) {
                    message_veh(0x2409, a1, baseId, 0);
                    reinterpret_cast<NetDaemon *>(g_0093cd90)->await_exec(1);
                    return;
                }
                action_oblit(a1, baseId);
            }
        }
    }
}
