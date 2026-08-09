// ORIGINAL: 0x004CE210 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004CE210
// name           ?action_tectonic@@YAXHHH@Z
// size           1169 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ce210/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?action_tectonic@@YAXHHH@Z  at 0x004CE210  (1169 bytes)
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
class FX;
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

class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int, int, int);
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

int base_find(int, int);
int parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
int shoot_it(int, int, int, int, int);
int stack_fix(int);
int veh_at(int, int);
int whose_territory(int, int, int, int *, int);
void draw_map(int);
void kill(int);
void world_climate();
void world_raise_alt(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_0066f8c4 = (int *)0x0066F8C4;
static int *const g_0068820c = (int *)0x0068820C;
static int *const g_0068821c = (int *)0x0068821C;
static int *const g_0068822c = (int *)0x0068822C;
static int *const g_0068823c = (int *)0x0068823C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_0090d91c = (int *)0x0090D91C;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946138 = (int *)0x00946138;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_00952836 = (int *)0x00952836;
static int *const g_0096c9e1 = (int *)0x0096C9E1;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009ab88f = (int *)0x009AB88F;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;
extern char g_00952828[];  // per-faction struct, stride 0x34 bytes:
                            // +0x0 short, +0x2 short, +0xA short, +0xE byte
extern int g_00946a50[];
extern char g_00946a84[];
extern char g_00946a9c[];
extern int g_00946d4c[];
extern int g_00946d50[];
extern char g_00946d34[];

// Translated directly from the Ghidra hypothesis (spot-checked against the
// raw disassembly for the trickiest arithmetic: the per-faction struct at
// 0x952828 (stride 0x34: short/short/short/byte at +0,+2,+0xA,+0xE), the
// 0x2099-stride relation-flag tables, and the shift-amount bug where CL at
// the has_project-style guard is *g_00939284's low byte, not the earlier
// owner byte, because ecx gets reloaded in between).
// Scored MISMATCH, very close: divergence at instruction #4 (push vs mov,
// within the FX::play argument setup) and only 8 bytes of total size
// difference (1161 rebuilt vs 1169 original) - consistent with a
// REGISTER ALLOCATION or scheduling near-miss rather than a structural one.
void __cdecl action_tectonic(int a1, int a2, int a3) {
    int played_focus = 0;
    int shot_result = 0;
    int faction_off = a1 * 0x34;

    short fx_y = *reinterpret_cast<short *>(g_00952828 + faction_off + 2);
    short fx_x = *reinterpret_cast<short *>(g_00952828 + faction_off);
    reinterpret_cast<FX *>(g_00749cf8)->play(0x53, fx_x, fx_y);

    short base_idx_raw = *reinterpret_cast<short *>(g_00952828 + faction_off + 0xA);
    unsigned char land_id = *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_009ab88f) + base_idx_raw * 0x34);
    int raise_count = g_0066f8c4[land_id];

    int missile_base = base_find(a2, a3);

    unsigned char owner = *reinterpret_cast<unsigned char *>(g_00952828 + faction_off + 0xE);

    if (owner != *g_00939284 &&
        ((*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0096c9e1) + *g_00939284 * 0x20CC) & 2) ||
         (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0094a30c) + 4 +
              (*g_0068faf0 * a3 + (a2 >> 1)) * 0x2C) & (1 << (*g_00939284 & 0x1f)))) &&
        missile_base >= 0 &&
        *reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0097d044) + missile_base * 0x134) == *g_00939284) {
        reinterpret_cast<Console *>(g_009156b0)->focus(a2, a3, owner);
        played_focus = 1;
    }

    unsigned int enemy = owner;
    kill(a1);

    if (enemy != static_cast<unsigned int>(*g_00939284) && missile_base >= 0) {
        int rec = enemy * 0x168 - enemy;
        int recBytes = rec * 4;
        *g_009bbff0 = -1;
        *g_009bbfec = g_00946a50[rec];
        parse_says(0, g_00946a9c + recBytes, -1, -1);
        *g_009bbfec = g_00946a50[rec];
        *g_009bbff0 = -1;
        parse_says(1, g_00946a84 + recBytes, -1, -1);
        *g_009bbfec = g_00946d4c[rec];
        *g_009bbff0 = g_00946d50[rec];
        parse_says(2, g_00946d34 + recBytes, -1, -1);
        parse_says(3, reinterpret_cast<char *>(g_0097d053) + missile_base * 0x134, -1, -1);
        popp(*reinterpret_cast<char **>(g_00691b0c), reinterpret_cast<const char *>(g_0068821c),
             0, reinterpret_cast<const char *>(g_0068820c), 0);
        enemy = owner;
    }

    int hit[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    if (raise_count > 0) {
        int i = 0;
        do {
            int vx = g_0066efbc[i] + a2;
            if (!(*g_0094988c & 1)) {
                if (vx < 0) {
                    vx += *g_00949870;
                } else if (vx >= *g_00949870) {
                    vx -= *g_00949870;
                }
            }
            int vy = g_0066f440[i] + a3;
            if (vy >= 0 && vy < *g_00949874 && vx >= 0 && vx < *g_00949870) {
                int veh = veh_at(vx, vy);
                veh = stack_fix(veh);
                if (veh >= 0) {
                    unsigned int veh_owner = *reinterpret_cast<unsigned char *>(g_00952828 + veh * 0x34 + 0xE);
                    if (enemy != veh_owner &&
                        !(*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0096c9f8) +
                              veh_owner * 4 + enemy * 0x20CC) & 1) &&
                        hit[veh_owner] == 0) {
                        shot_result = shoot_it(enemy, veh_owner, a2, a3, 0);
                        if (shot_result != 0) {
                            break;
                        }
                        hit[veh_owner] = 1;
                    }
                }
            }
            ++i;
        } while (i < raise_count);
    }

    int territory = whose_territory(enemy, a2, a3, 0, 0);
    if (territory >= 0 && static_cast<unsigned int>(territory) != enemy && hit[territory] == 0) {
        shot_result = shoot_it(enemy, territory, a2, a3, 0);
    }

    if (shot_result == 0) {
        for (int n = land_id; n > 0; --n) {
            world_raise_alt(a2, a3);
        }
        world_climate();
        draw_map(1);

        if (played_focus) {
            parse_says(0, reinterpret_cast<char *>(g_0097d053) + missile_base * 0x134, -1, -1);
            *g_009bbff0 = -1;
            *g_009bbfec = g_00946a50[owner * 0x167];
            parse_says(1, g_00946a9c + owner * 0x59c, -1, -1);
            *g_009bbff0 = -1;
            *g_009bbfec = g_00946a50[owner * 0x167];
            parse_says(2, g_00946a84 + owner * 0x59c, -1, -1);
            *g_009bbfec = g_00946d4c[owner * 0x167];
            *g_009bbff0 = g_00946d50[owner * 0x167];
            parse_says(3, g_00946d34 + owner * 0x59c, -1, -1);
            popp(*reinterpret_cast<char **>(g_00691b0c), reinterpret_cast<const char *>(g_0068823c),
                 0, reinterpret_cast<const char *>(g_0068822c), 0);
        }

        g_00946138[owner] = g_00946138[owner] + 1;
        *g_0090d91c |= 4;
    }
}
