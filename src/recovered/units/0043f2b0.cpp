// ORIGINAL: 0x0043F2B0 FILE
// name      ?start@DiploPop@@QAEXH@Z
// size      773 bytes
// spans     0x0043F2B0-0x0043F5B5
// prototype void (__thiscall ?start@DiploPop@@QAEXH@Z)(DiploPop* this, int)
// callers   1   call targets   10
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D310 0x0053A980 0x005D5930 0x006050F0 0x006162D0 0x00616650 0x00629E50 0x0062A070 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0043F2B0
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043f2b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?start@DiploPop@@QAEXH@Z  at 0x0043F2B0  (773 bytes)
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
class BasePop;
class Buffer;
class DiploPop;
class Flic;
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
class SubInterface;
class Time;
typedef unsigned int UINT_PTR;
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
class BasePop { public:
    void __cdecl set_def_external_window(GraphicWin *, int, int, int, int);
};
class Flic { public:
    int decode_frame(int, int);
    int init(Buffer *, char *, int, int, int);
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
    void soft_update(RECT *);
};
class SubInterface { public:
    uint32_t field_0_;
    uint32_t field_4_;
    void set_iface_mode();
};
class Time { public:
    int unk_tgl_;
    UINT_PTR id_event_;
    void * callback1_;
    void * callback2_;
    int cb_param2_;
    int cb_param1_;
    uint32_t count_;
    int unk_1_;
    uint32_t resolution_;
    int unk_2_;
    void init(void (__cdecl *)(int, int), int, int, int, int);
    void start();
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
void __cdecl diplomacy_check(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0043f200 = (int *)0x0043F200;
static int *const g_0043f250 = (int *)0x0043F250;
static int *const g_0068393c = (int *)0x0068393C;
static int *const g_00683944 = (int *)0x00683944;
static int *const g_00683948 = (int *)0x00683948;
static int *const g_0068394c = (int *)0x0068394C;
static int *const g_00683950 = (int *)0x00683950;
static int *const g_00683958 = (int *)0x00683958;
static int *const g_00683960 = (int *)0x00683960;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007d161c = (int *)0x007D161C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093fa74 = (int *)0x0093FA74;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b86a0 = (int *)0x009B86A0;

class DiploPop { public:
    uint8_t pad_0_[0x1074];
    uint32_t field_1074_;

    void start(int);
};

// The address itself does work below (fed into an sbb-mask select and
// used as register-indexed table bases), so these need real extern
// linkage rather than the folded pointer constants above.
extern int g_007ae820_sym;
extern int g_0078eecc[];
extern int g_0078ee74[];
extern int g_0078eea0[];
extern int g_00946a50[];
extern int g_00946f58[];

// set_def_external_window is declared __cdecl inside BasePop, but the
// call site pushes exactly its 5 declared arguments and nothing more -
// no separate receiver slot - so it is invoked as the equivalent free
// function rather than through the context's BasePop shell, which would
// add an unwanted extra `this` push.
void __cdecl set_def_external_window_free(GraphicWin *, int, int, int, int);

// Flic::init and Time::init share the base name "init" with matching
// arity (5), and mizuchi_declfix's callee-name rewrite is not class
// scoped - it clobbers whichever declaration comes first in the file
// with whichever callee row lands last. A same-signature shim under a
// non-colliding name sidesteps the collision entirely.
class FlicInitShim { public:
    int flic_init(Buffer *, char *, int, int, int);
};

void DiploPop::start(int a1) {
    char *self = reinterpret_cast<char *>(this);

    int winH = *reinterpret_cast<int *>(self + 0x4c8);
    int winW = *reinterpret_cast<int *>(self + 0x4c4);
    set_def_external_window_free(reinterpret_cast<GraphicWin *>(this), 0, 0,
                                  winW, -winH);

    int diffRes = (*g_009b7b1c != 0x320);
    diplomacy_check(*g_00939284, a1, 0);

    if (*reinterpret_cast<int *>(self + 0xa94) == a1) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();
    } else {
        *reinterpret_cast<int *>(self + 0xa94) = a1;
        *reinterpret_cast<void **>(self + 0x20b8) = g_007d161c;

        int base = a1 * 0x65c;
        *reinterpret_cast<char **>(self + 0x20b4) =
            reinterpret_cast<char *>(g_0078eecc) + base;
        *reinterpret_cast<char **>(self + 0x20bc) =
            reinterpret_cast<char *>(g_0078ee74) + base;
        *reinterpret_cast<char **>(self + 0x20c0) =
            reinterpret_cast<char *>(g_0078eea0) + base;

        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();

        *reinterpret_cast<char *>(g_009b86a0) = 0;
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(g_0068393c));

        char *puVar6;
        if (*g_009a6488 != 0 &&
            (*(reinterpret_cast<char *>(g_00946f58) + a1 * 0x59c) & 0x80) != 0) {
            puVar6 = reinterpret_cast<char *>(g_00683944);
        } else {
            puVar6 = reinterpret_cast<char *>(g_00683948);
            if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_00946a50) +
                                          a1 * 0x59c) != 0) {
                puVar6 = reinterpret_cast<char *>(g_0068394c);
            }
        }
        int idx = a1 * 0x59c;
        strcat(reinterpret_cast<char *>(g_009b86a0), puVar6);

        int pct = (*g_0093fa74 * 3) / 20;
        if (pct < 0) {
            pct = 0;
        } else if (pct > 2) {
            pct = 2;
        }

        char numbuf[80];
        typedef char *(__cdecl * ItoaFn)(int, char *, int);
        reinterpret_cast<ItoaFn>(&_itoa)(pct, numbuf, 10);
        strcat(reinterpret_cast<char *>(g_009b86a0), numbuf);
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(g_00683950));

        void *sel1 = (&g_007ae820_sym != 0)
                         ? reinterpret_cast<void *>(g_007aec64)
                         : 0;
        int p50 = (diffRes ? 0xa : 0) + 0x50;
        int p16 = (diffRes ? 4 : 0) + 0x16;

        int r1 = reinterpret_cast<FlicInitShim *>(self + 0x15cc)->flic_init(
            reinterpret_cast<Buffer *>(sel1), reinterpret_cast<char *>(g_009b86a0),
            p16, p50, 1);
        if (r1 != 0) {
            return;
        }

        *reinterpret_cast<uint8_t *>(self + 0x15cc) = 9;

        int span = *reinterpret_cast<int *>(self + 0x20f0) -
                   *reinterpret_cast<int *>(self + 0x20e8);
        int frameCount = *reinterpret_cast<int *>(self + 0x20e4) + 1;
        *reinterpret_cast<char *>(g_009b86a0) = 0;
        int frames = ((span + ((span >> 31) & 3)) >> 2) +
                     *reinterpret_cast<int *>(self + 0x20e8) + 1;

        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(g_00946a50) + idx + 4);
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(g_00683958));

        void *sel2 = (&g_007ae820_sym != 0)
                         ? reinterpret_cast<void *>(g_007aec64)
                         : 0;
        int r2 = reinterpret_cast<FlicInitShim *>(self + 0xae8)->flic_init(
            reinterpret_cast<Buffer *>(sel2), reinterpret_cast<char *>(g_009b86a0),
            frameCount, frames, 0);
        if (r2 != 0) {
            void *sel3 = (&g_007ae820_sym != 0)
                             ? reinterpret_cast<void *>(g_007aec64)
                             : 0;
            int r3 = reinterpret_cast<FlicInitShim *>(self + 0xae8)->flic_init(
                reinterpret_cast<Buffer *>(sel3),
                reinterpret_cast<char *>(g_00683960), frameCount, frames, 0);
            if (r3 != 0) {
                return;
            }
        }

        reinterpret_cast<Time *>(self + 0xac0)->init(
            reinterpret_cast<void (__cdecl *)(int, int)>(g_0043f250), 0,
            reinterpret_cast<int>(this), 2000, 5);

        uint32_t vsyncCount = *reinterpret_cast<uint32_t *>(self + 0x1ff0);
        if (vsyncCount > 5) {
            if (vsyncCount == 0) {
                vsyncCount = 100;
            }
            reinterpret_cast<Time *>(self + 0xa98)->init(
                reinterpret_cast<void (__cdecl *)(int, int)>(g_0043f200), 0,
                reinterpret_cast<int>(this), vsyncCount, 5);
        }
    }

    *reinterpret_cast<int *>(self + 0x20b0) = 0;
    reinterpret_cast<Time *>(self + 0xa98)->start();
    reinterpret_cast<Time *>(self + 0xac0)->start();

    int decP2 = *reinterpret_cast<int *>(self + 0x1098);
    int decP1 = *reinterpret_cast<int *>(self + 0x1094);
    reinterpret_cast<Flic *>(self + 0xae8)->decode_frame(decP1, decP2);

    reinterpret_cast<GraphicWin *>(g_007ae820)
        ->soft_update(reinterpret_cast<RECT *>(self + 0x1074));
}
