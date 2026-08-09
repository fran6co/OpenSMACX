// ORIGINAL: 0x004DC230 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004DC230
// name           ?editor_rules@Console@@QAEXXZ
// size           741 bytes
// measured tier  NO_COMPILE
// refusal        u004dc230.cpp(420) : error C2027: use of undefined type 'Console' u004dc230.cpp(38) : see declaration of 'Console'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004dc230/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_rules@Console@@QAEXXZ  at 0x004DC230  (741 bytes)
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
int __cdecl X_pop(const char *, int, int (__cdecl *)());
int __cdecl default_rules();
void __cdecl auto_undo();
void __cdecl prefs_save(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00688fcc = (int *)0x00688FCC;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_00800000 = (int *)0x00800000;
static int *const g_0094b470 = (int *)0x0094B470;
static int *const g_0094b474 = (int *)0x0094B474;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009bc06c = (int *)0x009BC06C;
static int *const g_009bc070 = (int *)0x009BC070;

void Console::editor_rules() {
    auto_undo();

    while (true) {
        int ecx = *g_009a649c;
        int edx = *g_009a64c0;
        unsigned int eax = (unsigned int)ecx;
        edx = edx & 0xfe7fffff;

        eax = eax >> 0xb;
        eax = eax & 1;
        *g_009a64c0 = edx;

        // Test cl & 2
        if ((ecx & 2) != 0) {
            eax = eax | 2;
        } else {
            eax = eax & 0xfd;
        }

        // Test cl & 8
        if ((ecx & 8) != 0) {
            eax = eax | 4;
        } else {
            eax = eax & 0xfb;
        }

        // Test cl & 4
        if ((ecx & 4) != 0) {
            eax = eax | 8;
        } else {
            eax = eax & 0xf7;
        }

        // Test cl & 0x10 (ch & 0x10 - bit 12 of ecx)
        if ((ecx & 0x1000) != 0) {
            eax = eax | 0x10;
        } else {
            eax = eax & 0xef;
        }

        // Test cl & 1
        if ((ecx & 1) != 0) {
            eax = eax | 0x20;
        } else {
            eax = eax & 0xdf;
        }

        // Test cl & 0x10 (bit 4)
        if ((ecx & 0x10) != 0) {
            eax = eax | 0x40;
        } else {
            eax = eax & 0xbf;
        }

        // Test cl & 0x20 (bit 5)
        if ((ecx & 0x20) != 0) {
            eax = eax | 0x80;
        } else {
            eax = eax & 0x7f;
        }

        // Test ch & 0x40 (bit 14 of ecx)
        if ((ecx & 0x4000) != 0) {
            eax = eax | 0x100;
        } else {
            eax = eax & 0xfeff;
        }

        // Test ch & 2 (bit 9 of ecx)
        if ((ecx & 0x200) != 0) {
            eax = eax | 0x200;
        } else {
            eax = eax & 0xfdff;
        }

        // Test cl & 0x40 (bit 6 of ecx)
        if ((ecx & 0x40) != 0) {
            eax = eax | 0x400;
        } else {
            eax = eax & 0xfbff;
        }

        // Test ~ecx & 0x100 (bit 8 inverted)
        if ((~ecx & 0x100) != 0) {
            eax = eax | 0x800;
        } else {
            eax = eax & 0xf7ff;
        }

        // Test ch & 0x20 (bit 13 of ecx)
        if ((ecx & 0x2000) != 0) {
            eax = eax | 0x1000;
        } else {
            eax = eax & 0xefff;
        }

        // Test ch & 0x80 (bit 15 of ecx)
        if ((ecx & 0x8000) != 0) {
            eax = eax | 0x2000;
        } else {
            eax = eax & 0xdfff;
        }

        // Test cl & 0x80 (bit 7 of ecx)
        if ((ecx & 0x80) != 0) {
            eax = eax | 0x4000;
        } else {
            eax = eax & 0xbfff;
        }

        // Test ch & 4 (bit 10 of ecx)
        if ((ecx & 0x400) != 0) {
            eax = eax | 0x8000;
        } else {
            eax = eax & 0x7fff;
        }

        // Test edx & 0x800000
        if ((edx & 0x800000) != 0) {
            eax = eax | 0x10000;
        } else {
            eax = eax & 0xfffeffff;
        }

        // Test edx & 0x1000000
        if ((edx & 0x1000000) != 0) {
            eax = eax | 0x20000;
        } else {
            eax = eax & 0xfffdffff;
        }

        *g_009bc06c = (int)eax;
        *g_006a721c = 6;
        int result = X_pop((const char *)g_00688fcc, 0x41, 0);
        *g_006a721c = 0;

        if (result < 0) {
            break;
        }

        ecx = *g_009a649c;
        edx = *g_009a64c0;
        int val = *g_009bc070;

        ecx = ecx & 0xffff0000;
        edx = edx & 0xfe7fffff;

        *g_009a649c = ecx;
        *g_009a64c0 = edx;

        if (val == 0) {
            eax = (unsigned int)*g_009bc06c;

            if ((eax & 1) != 0) {
                edx = *g_009a649c;
                edx = edx | 0x800;
                *g_009a649c = edx;
            }

            if ((eax & 2) != 0) {
                *g_009a649c = *g_009a649c | 2;
            }

            if ((eax & 4) != 0) {
                *g_009a649c = *g_009a649c | 8;
            }

            if ((eax & 8) != 0) {
                *g_009a649c = *g_009a649c | 4;
            }

            if ((eax & 0x10) != 0) {
                edx = *g_009a649c;
                edx = edx | 0x1000;
                *g_009a649c = edx;
            }

            if ((eax & 0x20) != 0) {
                *g_009a649c = *g_009a649c | 1;
            }

            if ((eax & 0x40) != 0) {
                *g_009a649c = *g_009a649c | 0x10;
            }

            if ((eax & 0x80) != 0) {
                *g_009a649c = *g_009a649c | 0x20;
            }

            if ((eax & 0x100) != 0) {
                edx = *g_009a649c;
                edx = edx | 0x4000;
                *g_009a649c = edx;
            }

            if ((eax & 0x200) != 0) {
                *g_009a649c = *g_009a649c | 0x200;
            }

            if ((eax & 0x400) != 0) {
                *g_009a649c = *g_009a649c | 0x40;
            }

            if ((eax & 0x800) == 0) {
                edx = *g_009a649c;
                edx = edx | 0x100;
                *g_009a649c = edx;
            }

            if ((eax & 0x1000) != 0) {
                *g_009a649c = *g_009a649c | 0x2000;
            }

            if ((eax & 0x2000) != 0) {
                *g_009a649c = *g_009a649c | 0x8000;
            }

            if ((eax & 0x4000) != 0) {
                *g_009a649c = *g_009a649c | 0x80;
            }

            if ((eax & 0x8000) != 0) {
                *g_009a649c = *g_009a649c | 0x400;
            }

            if ((eax & 0x10000) != 0) {
                edx = *g_009a64c0;
                edx = edx | 0x800000;
                *g_009a64c0 = edx;
            }

            if ((eax & 0x20000) != 0) {
                edx = *g_009a64c0;
                edx = edx | 0x1000000;
                *g_009a64c0 = edx;
            }

            edx = *g_009a649c;
            ecx = *g_009a64c0;
            ecx = ecx & 0x1800000;
            *g_0094b470 = edx;
            *g_0094b474 = ecx;
            prefs_save(0);
            return;
        } else {
            eax = (unsigned int)default_rules();
            ecx = *g_009a649c;
            ecx = ecx | (int)eax;
            *g_009a649c = ecx;
            *g_0094b470 = ecx;

            edx = *g_009a64c0;
            edx = edx & 0x1800000;
            *g_0094b474 = edx;
        }
    }
}
