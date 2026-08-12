// ORIGINAL: 0x00414B40 FILE
// name      ?draw_pop@BaseWin@@QAEXH@Z
// size      1365 bytes
// spans     0x00414B40-0x00415078;0x00651B76-0x00651B93
// prototype void (__thiscall ?draw_pop@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   1   call targets   15
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E4020 0x005D5630 0x005D5930 0x005D8000 0x005E2DD3 0x005E4B4A 0x005FA960 0x005FAA90 0x00618EA0 0x00618EE0 0x00618F40 0x00625730 0x00625740 0x00625750 0x00625770
// indirect  0x00414B81
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00414B40
// measured tier  MISMATCH
// divergence     35
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00414b40/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?draw_pop@BaseWin@@QAEXH@Z  at 0x00414B40  (1365 bytes)
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
struct BoxSpriteParams;
class Buffer;
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
class Random;
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

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
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
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int add(int, int, int, int, int, int);
    void kill_type(int);
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_clip(RECT *);
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
    Font();
    int init(char *, int, int);
    ~Font();
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
    void update(RECT *, GraphicWin *);
};
class Random { public:
    uint32_t seed_;
    Random();
    unsigned int get(int, int);
    void reseed(unsigned long);
    ~Random();
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
    int draw(Buffer *, int, int, int, int, int);
};
int __cdecl best_specialist();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00651b89 = (int *)0x00651B89;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_006729c0 = (int *)0x006729C0;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_007765c0 = (int *)0x007765C0;
static int *const g_0078cad0 = (int *)0x0078CAD0;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_00798cd0 = (int *)0x00798CD0;
static int *const g_007ad1b0 = (int *)0x007AD1B0;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec84 = (int *)0x007AEC84;
static int *const g_007aec88 = (int *)0x007AEC88;
static int *const g_007aec8c = (int *)0x007AEC8C;
static int *const g_007aec90 = (int *)0x007AEC90;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6488 = (int *)0x009A6488;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_pop(int);
};

extern uint8_t g_00946f58x[];
extern uint8_t g_007ad1b0x[];
extern uint8_t g_00798cd0x[];
extern uint8_t g_007765c0x[];
extern uint8_t g_0078cad0x[];

void BaseWin::draw_pop(int a1) {
    char *self = reinterpret_cast<char *>(this);

    Font font;
    Random rng;

    typedef unsigned long(__stdcall * TimeGetTimeFn)();
    TimeGetTimeFn timeGetTime = reinterpret_cast<TimeGetTimeFn>(*reinterpret_cast<void **>(g_00669368));
    rng.reseed(timeGetTime());

    RECT *winRect = reinterpret_cast<RECT *>(self + 0x40c5c);
    RECT clipRect = *winRect;
    RECT savedClip = {*g_007aec84, *g_007aec88, *g_007aec8c, *g_007aec90};

    reinterpret_cast<Buffer *>(g_007aec64)->set_clip(&clipRect);
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(winRect, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    int specialistCount[7];
    for (int z = 0; z < 7; ++z) specialistCount[z] = 0;

    font.init(*reinterpret_cast<char **>(g_00691b2c), 9, 1);

    Spot *spot = reinterpret_cast<Spot *>(self + 0x40eb4);
    spot->kill_type(1);

    int innerLeft = winRect->left + 3;
    int innerTop = winRect->top + 3;
    int innerRight = winRect->right - 3;
    int innerBottom = winRect->bottom - 3;
    RECT innerRect = {innerLeft, innerTop, innerRight, innerBottom};
    reinterpret_cast<Buffer *>(g_007aec64)->set_clip(&innerRect);

    char *curBase = reinterpret_cast<char *>(*g_0090ea30);
    int numSpecialists = curBase[6];
    int width = innerRight - innerLeft;
    int spacing;
    if (numSpecialists < 2) {
        spacing = width;
    } else {
        spacing = (innerBottom - innerLeft - width) / (numSpecialists - 1);
        if (spacing < 1 || width < 1) {
            spacing = 1;
        } else if (width < spacing) {
            spacing = width;
        }
    }
    int rightEdge = innerTop + width;
    int bottomEdge = innerLeft + width;

    if (*g_009a6488 == 0 || (g_00946f58x[curBase[4] * 0x59c] & 0x80) == 0) {
        int i = 0;
        int x = innerLeft;
        if (numSpecialists > 0) {
            do {
                curBase = reinterpret_cast<char *>(*g_0090ea30);
                int overtimeStart = *reinterpret_cast<int *>(curBase + 0x7c);
                int idx;
                int y;
                int buffer;
                if (i < numSpecialists - overtimeStart) {
                    idx = (*reinterpret_cast<int *>(curBase + 0x118) <= i) ? 1 : 0;
                    if ((numSpecialists - *reinterpret_cast<int *>(curBase + 0x11c)) - overtimeStart <= i) idx = 2;
                    if ((numSpecialists - *reinterpret_cast<int *>(curBase + 0x120)) - overtimeStart <= i) idx = 3;
                    int localY = rightEdge - innerTop;
                    y = (localY - 0x30) / 2 + innerTop;
                    int rnd = rng.get(0, 2);
                    uint8_t sprite = g_0078cad0x[(idx + rnd * 4) * 0x2c + 8];
                    reinterpret_cast<Sprite *>(g_0078cad0x + (idx + rnd * 4) * 0x2c)
                        ->draw(reinterpret_cast<Buffer *>(g_007aec64), sprite, x, y, 1, 1);
                    y = bottomEdge - x;
                    buffer = -1;
                    spot->add(buffer, 1, x, innerTop, y, localY);
                } else {
                    int specIdx = (overtimeStart - numSpecialists) + i;
                    unsigned int sIdx;
                    if (specIdx < 0x10) {
                        sIdx = (*reinterpret_cast<unsigned int *>(curBase + 0x84 + (specIdx >> 3) * 4) >>
                                ((specIdx & 7) << 2)) &
                               0xf;
                    } else {
                        sIdx = best_specialist();
                    }
                    specialistCount[sIdx] = specialistCount[sIdx] + 1;
                    int localY = rightEdge - innerTop;
                    uint8_t sprite = g_007ad1b0x[sIdx * 0x2c + 8];
                    reinterpret_cast<Sprite *>(g_007ad1b0x + sIdx * 0x2c)
                        ->draw(reinterpret_cast<Buffer *>(g_007aec64), sprite, x, (localY - 0x30) / 2 + innerTop, 1,
                               1);
                    y = bottomEdge - x;
                    reinterpret_cast<Spot *>(spot)->add(specIdx, 1, x, innerTop, y, localY);
                }
                x = x + spacing;
                bottomEdge = bottomEdge + spacing;
                ++i;
                numSpecialists = reinterpret_cast<char *>(*g_0090ea30)[6];
            } while (i < numSpecialists);
        }
    } else {
        int i = 0;
        int x = innerLeft;
        if (numSpecialists > 0) {
            do {
                curBase = reinterpret_cast<char *>(*g_0090ea30);
                unsigned int overtimeStart = *reinterpret_cast<unsigned int *>(curBase + 0x7c);
                int y;
                if (i < static_cast<int>(numSpecialists - overtimeStart)) {
                    unsigned int idx = (*reinterpret_cast<int *>(curBase + 0x118) <= i) ? 1 : 0;
                    if (static_cast<int>((numSpecialists - *reinterpret_cast<int *>(curBase + 0x11c)) -
                                          overtimeStart) <= i)
                        idx = 2;
                    if (static_cast<int>((numSpecialists - *reinterpret_cast<int *>(curBase + 0x120)) -
                                          overtimeStart) <= i)
                        idx = 3;
                    int localY = rightEdge - innerTop;
                    uint8_t sprite = g_00798cd0x[idx * 0x2c + 8];
                    reinterpret_cast<Sprite *>(g_00798cd0x + idx * 0x2c)
                        ->draw(reinterpret_cast<Buffer *>(g_007aec64), sprite, x, (localY - 0x30) / 2 + innerTop, 1,
                               1);
                    y = bottomEdge - x;
                    reinterpret_cast<Spot *>(spot)->add(static_cast<unsigned int>(-1), 1, x, innerTop, y, localY);
                } else {
                    unsigned int specIdx = (overtimeStart - numSpecialists) + i;
                    unsigned int sIdx;
                    if (static_cast<int>(specIdx) < 0x10) {
                        sIdx = (*reinterpret_cast<unsigned int *>(curBase + 0x84 + (specIdx >> 3) * 4) >>
                                ((specIdx & 7) << 2)) &
                               0xf;
                    } else {
                        sIdx = best_specialist();
                    }
                    specialistCount[sIdx] = specialistCount[sIdx] + 1;
                    int localY = rightEdge - innerTop;
                    uint8_t sprite = g_007765c0x[sIdx * 0x2c + 8];
                    reinterpret_cast<Sprite *>(g_007765c0x + sIdx * 0x2c)
                        ->draw(reinterpret_cast<Buffer *>(g_007aec64), sprite, x, (localY - 0x30) / 2 + innerTop, 1,
                               1);
                    y = bottomEdge - x;
                    reinterpret_cast<Spot *>(spot)->add(specIdx, 1, x, innerTop, y, localY);
                }
                x = x + spacing;
                bottomEdge = bottomEdge + spacing;
                ++i;
                numSpecialists = reinterpret_cast<char *>(*g_0090ea30)[6];
            } while (i < numSpecialists);
        }
    }

    if (a1 == 0) {
        reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(winRect);
    } else {
        reinterpret_cast<GraphicWin *>(g_007ae820)->update(winRect, 0);
    }

    reinterpret_cast<Buffer *>(g_007aec64)->set_clip(&savedClip);
}
