// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A920
// name           ?zoom@BaseWin@@QAEXHH@Z
// size           1312 bytes
// measured tier  MISMATCH
// divergence     51
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a920/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?zoom@BaseWin@@QAEXHH@Z  at 0x0041A920  (1312 bytes)
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
class NetDaemon;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class FX { public:
    uint8_t effects_[0x61 * 0x6C];
    uint32_t field_28EC_;
    void play(int);
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
    void redraw();
};
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
};
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
    int reset();
    int tut_win(GraphicWin *, const char *, int, int, Sprite *, int, int, int);
    void base_rect(RECT *, int *, int *);
    void do_base(RECT *, const char *, int);
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
    int is_visible();
};
int __cdecl ambience(int);
int __cdecl base_compute(int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl tut_check(int);
int __cdecl tut_check2(int);
void __cdecl bitmask(int, int *, int *);
void __cdecl set_base(int);
void __cdecl wait_2();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();  // <-- used
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682c78 = (int *)0x00682C78;
static int *const g_00682c84 = (int *)0x00682C84;
static int *const g_00682c90 = (int *)0x00682C90;
static int *const g_00682c9c = (int *)0x00682C9C;
static int *const g_00682ca4 = (int *)0x00682CA4;
static int *const g_00682cb4 = (int *)0x00682CB4;
static int *const g_00682cc0 = (int *)0x00682CC0;
static int *const g_00682cd0 = (int *)0x00682CD0;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_00689374 = (int *)0x00689374;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00749cf8 = (int *)0x00749CF8;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_008cc228 = (int *)0x008CC228;
static int *const g_0090e91c = (int *)0x0090E91C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096da3c = (int *)0x0096DA3C;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009b2068 = (int *)0x009B2068;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_expenses(int);
    void zoom(int, int);
};

extern int g_0097d0b8x[];
extern int g_0096da3cx[];
extern uint8_t g_0097d044x[];
extern uint8_t g_0097d0ccx[];

class VCall2 { public:
    virtual void slot000();
    virtual void slot001(int);  // <-- used, widened
};

void BaseWin::zoom(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0x40b00) = -1;
    reinterpret_cast<TutWin *>(g_008c6e68)->reset();
    *reinterpret_cast<int *>(self + 0x40b0c) = a1;
    *g_00689374 = -1;

    int prevBase = g_0097d0b8x[*reinterpret_cast<int *>(self + 0x40b0c) * 0x4d];
    set_base(*reinterpret_cast<int *>(self + 0x40b0c));
    base_compute(0);

    if (*g_0093f660 != 0 && *g_0093a938 == 0 &&
        prevBase != g_0097d0b8x[*reinterpret_cast<int *>(self + 0x40b0c) * 0x4d] &&
        reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1) == 0) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
    }

    *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x30) &= 0xffffffef;
    *reinterpret_cast<int *>(self + 0x40b28) = 1;
    *reinterpret_cast<int *>(self + 0x40b10) = 0;
    *reinterpret_cast<int *>(self + 0x40b14) = 0;
    *reinterpret_cast<int *>(self + 0x40b18) = 1;
    *reinterpret_cast<int *>(self + 0x40b34) =
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x50);

    reinterpret_cast<FX *>(g_00749cf8)->play(0xf);
    ambience(0x1e);

    if (reinterpret_cast<Win *>(self)->is_visible() == 0) {
        reinterpret_cast<VCall2 *>(self)->slot001(0);
    } else {
        reinterpret_cast<VCall *>(self)->slot062();
    }
    reinterpret_cast<GraphicWin *>(g_007ae820)->redraw();
    *reinterpret_cast<int *>(self + 0x1e670) = 0;

    RECT *rectPtr = 0;
    const char *msg = 0;
    bool doMessage = true;

    if (*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*g_0090ea30) + 4) == *g_00939284 &&
        g_0096da3cx[*g_00939284 * 0x833] == 1 && tut_check(2) != 0) {
        *g_008cc228 = 1;
        parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
        int outC, outD;
        reinterpret_cast<TutWin *>(g_008c6e68)->base_rect(reinterpret_cast<RECT *>(self + 0x40dac), &outC, &outD);

        int *objPtr = *reinterpret_cast<int **>(g_007d3c3c);
        GraphicWin *gwin = 0;
        if (objPtr != 0) {
            int *vtbl = *reinterpret_cast<int **>(objPtr);
            gwin = reinterpret_cast<GraphicWin *>(reinterpret_cast<char *>(objPtr) + vtbl[1]);
        }
        reinterpret_cast<TutWin *>(g_008c6e68)
            ->tut_win(gwin, reinterpret_cast<const char *>(g_00682c78), outC, outD, 0, 0x100, -1, -1);
        doMessage = false;
    } else if ((*reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*g_0090ea30) + 0x30) & 2) == 0 ||
               tut_check2(0x10) == 0 ||
               (*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x38) & 0x80000040) ==
                   0x80000040) {
        if (*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0xd0) < 0 &&
            tut_check2(0x20) != 0 &&
            *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x7c) != 0 &&
            (*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x38) & 0x80000040) !=
                0x80000040) {
            parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
            rectPtr = reinterpret_cast<RECT *>(self + 0x40b8c);
            msg = reinterpret_cast<const char *>(g_00682c90);
        } else {
            char b6 = *reinterpret_cast<char *>(reinterpret_cast<char *>(*g_0090ea30) + 6);
            int d4 = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0xd4);
            bool support = d4 < (b6 + 1) / 2 && *g_0090e91c >= 1 && (b6 >= 4 || d4 <= 0) &&
                           tut_check2(0x200) != 0;
            if (support) {
                parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
                rectPtr = reinterpret_cast<RECT *>(self + 0x40cdc);
                msg = reinterpret_cast<const char *>(g_00682c9c);
            } else {
                char b6b = *reinterpret_cast<char *>(reinterpret_cast<char *>(*g_0090ea30) + 6);
                int d4b = *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0xd4);
                bool lowMinerals = d4b <= 1 && *g_0090e91c < b6b / 2 && b6b >= 3 && tut_check2(0x40) != 0;
                if (lowMinerals) {
                    parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
                    short w0 = *reinterpret_cast<short *>(reinterpret_cast<char *>(*g_0090ea30) + 2);
                    short w1 = *reinterpret_cast<short *>(reinterpret_cast<char *>(*g_0090ea30) + 0);
                    int idx = (w0 * (*g_0068faf0) + (w1 >> 1)) * 11;
                    uint8_t val = reinterpret_cast<uint8_t *>(*g_0094a30c)[idx * 4] & 0xe0;
                    msg = (val < 0x60) ? reinterpret_cast<const char *>(g_00682ca4)
                                       : reinterpret_cast<const char *>(g_00682cb4);
                    rectPtr = reinterpret_cast<RECT *>(self + 0x40cdc);
                } else if (*reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0xe8) <=
                               *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_0090ea30) + 0xd8) ||
                           (*reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(*g_0090ea30) + 0x38) &
                            0x80000000) != 0) {
                    doMessage = false;
                } else {
                    bool skip = false;
                    for (int i = 0; i < *g_009a64cc; ++i) {
                        if (g_0097d044x[i * 0x134] ==
                            *reinterpret_cast<uint8_t *>(reinterpret_cast<char *>(*g_0090ea30) + 4)) {
                            int localC;
                            bitmask(1, &a1, &localC);
                            uint8_t flagByte = g_0097d0ccx[i * 0x134 + a1];
                            if ((flagByte & static_cast<uint8_t>(localC)) != 0) {
                                skip = true;
                                break;
                            }
                        }
                    }
                    bool ineffHQ = !skip && tut_check2(0x400) != 0;
                    if (ineffHQ) {
                        parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
                        draw_expenses(2);
                        rectPtr = reinterpret_cast<RECT *>(self + 0x40cbc);
                        msg = reinterpret_cast<const char *>(g_00682cc0);
                    } else if (tut_check2(0x80) != 0) {
                        parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
                        draw_expenses(2);
                        rectPtr = reinterpret_cast<RECT *>(self + 0x40cbc);
                        msg = reinterpret_cast<const char *>(g_00682cd0);
                    } else {
                        doMessage = false;
                    }
                }
            }
        }
        if (doMessage) {
            reinterpret_cast<TutWin *>(g_008c6e68)->do_base(rectPtr, msg, 0);
        }
    } else {
        *g_008cc228 = 6;
        parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
        int outC, outD;
        reinterpret_cast<TutWin *>(g_008c6e68)->base_rect(reinterpret_cast<RECT *>(self + 0x40dac), &outC, &outD);

        int *objPtr = *reinterpret_cast<int **>(g_007d3c3c);
        GraphicWin *gwin = 0;
        if (objPtr != 0) {
            int *vtbl = *reinterpret_cast<int **>(objPtr);
            gwin = reinterpret_cast<GraphicWin *>(reinterpret_cast<char *>(objPtr) + vtbl[1]);
        }
        reinterpret_cast<TutWin *>(g_008c6e68)
            ->tut_win(gwin, reinterpret_cast<const char *>(g_00682c84), outC, outD, 0, 0x100, -1, -1);
    }

    if (a2 != 0) {
        while (*reinterpret_cast<int *>(self + 0x40b28) != 0 &&
               reinterpret_cast<Win *>(self)->is_visible() != 0 && *g_009b2068 == 0) {
            wait_2();
        }
    }
}
