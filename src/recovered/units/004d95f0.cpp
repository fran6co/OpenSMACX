// ORIGINAL: 0x004D95F0 FILE
// name      ?assemble@Console@@QAEXXZ
// size      896 bytes
// spans     0x004D95F0-0x004D9970
// prototype void (__thiscall ?assemble@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046B190 0x004D92D0 0x0050F650 0x0059DB40 0x0059E530 0x005BF3F0 0x00644F3A
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004D95F0
// measured tier  NO_COMPILE
// refusal        u004d95f0.cpp(1) : error C2653: 'Console' : is not a class or namespace name
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004d95f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?assemble@Console@@QAEXXZ  at 0x004D95F0  (896 bytes)
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
extern "C" int abs(int);
int __cdecl X_pop(const char *, int, int (__cdecl *)());
int __cdecl prefs_get(char *, int, int);
void __cdecl draw_map(int);
void __cdecl prefs_put(char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00688f0c = (int *)0x00688F0C;
static int *const g_00688f1c = (int *)0x00688F1C;
static int *const g_00688f2c = (int *)0x00688F2C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_00952832 = (int *)0x00952832;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009ab88c = (int *)0x009AB88C;
static int *const g_009ab892 = (int *)0x009AB892;
static int *const g_009bc06c = (int *)0x009BC06C;
static int *const g_009bc070 = (int *)0x009BC070;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void group(int);
    void clear_group();
    void assemble();
};

extern uint8_t g_0094a379[];

void Console::assemble() {
    char *self = reinterpret_cast<char *>(this);

    int result = prefs_get(reinterpret_cast<char *>(g_00688f0c), 7, 0);
    for (int i = 0; i < 0xc; ++i) {
        uint32_t bit = 1u << i;
        if (result & bit) {
            *reinterpret_cast<uint32_t *>(g_009bc06c) |= bit;
        } else {
            *reinterpret_cast<uint32_t *>(g_009bc06c) &= ~bit;
        }
    }

    if (X_pop(reinterpret_cast<const char *>(g_00688f1c), 0x41, 0) < 0) {
        return;
    }

    clear_group();

    uint32_t flags = 0;
    for (int j = 0; j < 0xc; ++j) {
        uint32_t bit = 1u << j;
        if (*reinterpret_cast<uint32_t *>(g_009bc06c) & bit) {
            flags |= bit;
        }
    }

    *reinterpret_cast<int32_t *>(self + 0x23d1c) = 1;

    int32_t best = -1;
    if (*g_009a64c8 > 0) {
        char *rec = reinterpret_cast<char *>(g_00952832);
        for (int32_t idx = 0; idx < *g_009a64c8; ++idx, rec += 0x34) {
            if (static_cast<uint32_t>(*reinterpret_cast<uint8_t *>(rec + 4)) !=
                *reinterpret_cast<uint32_t *>(self + 0x23bd4)) {
                continue;
            }

            if (flags & 1) {
                int16_t sVar2 = *reinterpret_cast<int16_t *>(rec - 8);
                int32_t iv8 = g_00939340[*g_009392b8];
                int32_t iv6 = abs(static_cast<int32_t>(*reinterpret_cast<int16_t *>(rec - 0xa)) -
                                   g_009392c0[*g_009392b8]);
                if ((*reinterpret_cast<uint8_t *>(g_0094988c) & 1) == 0 && iv6 > *g_0068faf0) {
                    iv6 = *g_00949870 - iv6;
                }
                iv8 = abs(static_cast<int32_t>(sVar2) - iv8);
                if (((iv8 + iv6) & ~1) > 4) {
                    continue;
                }
            }

            int16_t protoIdx = *reinterpret_cast<int16_t *>(rec);
            uint8_t abVal = reinterpret_cast<uint8_t *>(g_009ab88c)[protoIdx * 0x34];
            uint8_t chassisVal = g_0094a379[abVal * 0x90];
            uint8_t typeVal = reinterpret_cast<uint8_t *>(g_009ab892)[protoIdx * 0x34];

            bool ok =
                ((flags & 2) == 0 ||
                 (*reinterpret_cast<int8_t *>(rec + 7) != 1 &&
                  *reinterpret_cast<int8_t *>(rec + 7) != 2)) &&
                ((flags & 4) == 0 ||
                 ((*reinterpret_cast<uint32_t *>(rec - 6) & 0x1000200) != 0x1000200 &&
                  (*reinterpret_cast<uint32_t *>(rec - 6) & 0x4200) == 0)) &&
                ((flags & 8) == 0 || chassisVal != 0) &&
                ((flags & 0x10) == 0 || chassisVal != 1) &&
                ((flags & 0x20) == 0 || chassisVal != 2) &&
                ((flags & 0x40) == 0 ||
                 (typeVal != 0 && typeVal != 1 && typeVal != 5 && typeVal != 6)) &&
                ((flags & 0x80) == 0 || (typeVal != 2 && typeVal != 4)) &&
                ((flags & 0x100) == 0 || typeVal != 9) &&
                ((flags & 0x200) == 0 || typeVal != 0xb) &&
                ((flags & 0x400) == 0 ||
                 (typeVal != 8 && typeVal != 0xa && typeVal != 7 && typeVal != 0xc));

            if (ok) {
                *reinterpret_cast<uint32_t *>(rec - 6) |= 0x8000000;
                best = idx;
            }
        }
    }

    if (*g_009bc070 == 1) {
        if (best >= 0) {
            int16_t uVar3 = static_cast<int16_t>(g_009392c0[*g_009392b8]);
            int16_t uVar4 = static_cast<int16_t>(g_00939340[*g_009392b8]);
            char *bestRec = reinterpret_cast<char *>(g_00952832) + best * 0x34;
            *reinterpret_cast<uint8_t *>(bestRec + 7) = 0x18;
            *reinterpret_cast<int16_t *>(bestRec + 0xa) = uVar3;
            *reinterpret_cast<int16_t *>(bestRec + 0x12) = uVar4;
            *reinterpret_cast<uint32_t *>(bestRec - 6) &= 0xfcffbdff;
            group(best);
        }
    }

    if (*g_009bc070 == 2 && *g_009a64c8 > 0) {
        char *rec = reinterpret_cast<char *>(g_0095282c);
        int32_t count = *g_009a64c8;
        do {
            if (static_cast<uint32_t>(*reinterpret_cast<uint8_t *>(rec + 0xa)) ==
                    *reinterpret_cast<uint32_t *>(self + 0x23bd4) &&
                (*reinterpret_cast<uint32_t *>(rec) & 0x8000000) != 0) {
                *reinterpret_cast<uint8_t *>(rec + 0xd) = 0;
                *reinterpret_cast<uint32_t *>(rec) &= 0xfcffbdff;
            }
            rec += 0x34;
            --count;
        } while (count != 0);
    }

    draw_map(1);
    prefs_put(reinterpret_cast<char *>(g_00688f2c), static_cast<int>(flags), 0);
}
