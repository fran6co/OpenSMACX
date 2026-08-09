// ORIGINAL: 0x00479F90 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00479F90
// name           ?init@MultiWin@@QAEXXZ
// size           1198 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00479f90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@MultiWin@@QAEXXZ  at 0x00479F90  (1198 bytes)
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
class BaseButton;
struct BorderSizing;
class Buffer;
class ButtonGroup;
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
class MultiWin;
typedef void * PVOID;
class Palette;
struct RECT;
class Scroll;
class Sound;
class Spot;
class Sprite;
class Strings;
class TexHeap;
class Time;
typedef unsigned int UINT_PTR;
class Wave;
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

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    void init(int);
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
    Buffer();
    int load_pcx(const char *, Palette *, int, int);
    ~Buffer();
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
};

class BaseButton { public:
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
    uint32_t field_A14_;
    uint32_t field_A18_;
    Time time1_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    Time time2_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    LPSTR name_;
    LPSTR bubble_text_;
    uint32_t color_;
    uint32_t bevel_upper_;
    uint32_t bevel_lower_;
    uint32_t bevel_thickness_;
    uint32_t field_A94_;
    uint32_t field_A98_;
    uint32_t field_A9C_;
    uint32_t field_AA0_;
    uint32_t field_AA4_;
    ButtonGroup * group_;
    uint32_t field_AAC_;
    uint32_t field_AB0_;
    uint32_t field_AB4_;
    int init(char *, int, int, int, int, int, Win *, int);
};

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
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
    int init(char *, int, int);
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
    int init(RECT *, char *, int, Win *, Menu *, BorderSizing *);
    void fill(int);
};

class Sound { public:
    void set_pan(int);
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
    int extract(Buffer *, int, int, int, int, int, TexHeap *);
};

class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};

class Wave { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    uint32_t field_8_;
    uint8_t memset_region_[0x24];
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    void * device_;
    uint32_t field_40_;
    Wave * chain_prev_;
    Wave * chain_next_;
    void * fname_;
    uint32_t field_50_;
    uint8_t flags_54_;
    uint8_t pad_55_[3];
    int32_t pitch_;
    float reverb_mix_;
    int32_t ms_length_;
    uint32_t start_time_;
    uint32_t group_slot_;
    void init(char *, unsigned long);
    void set_volume(int);
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
    int move(int, int);
    void show(int);
};


// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
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
static int *const g_0065642a = (int *)0x0065642A;
static int *const g_00676208 = (int *)0x00676208;
static int *const g_006867c4 = (int *)0x006867C4;
static int *const g_006867d0 = (int *)0x006867D0;
static int *const g_00691b2c = (int *)0x00691B2C;
static int *const g_00696d14 = (int *)0x00696D14;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007fff00 = (int *)0x007FFF00;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939ed8 = (int *)0x00939ED8;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009b7b1c = (int *)0x009B7B1C;
static int *const g_009b7b20 = (int *)0x009B7B20;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class MultiWin { public:
    void init();
};

extern unsigned char g_00946f58_arr[];

void MultiWin::init() {
    char *const self = reinterpret_cast<char *>(this);
    Buffer buf;

    *reinterpret_cast<int *>(self + 0xa4c) = 0;
    *reinterpret_cast<int *>(self + 0xa50) = 0;
    *reinterpret_cast<int *>(self + 0xa48) = 0;

    reinterpret_cast<Font *>(self + 0x285c)->init(
        *reinterpret_cast<char **>(g_00691b2c), 10, 0);
    reinterpret_cast<Font *>(self + 0x2884)->init(
        *reinterpret_cast<char **>(g_00691b2c), 0xe, 1);
    reinterpret_cast<Spot *>(self + 0x28ac)->init(0x10);

    buf.load_pcx(reinterpret_cast<char *>(g_006867c4), 0, 10, 0xec);

    unsigned char frame = *reinterpret_cast<unsigned char *>(g_00696d14);
    reinterpret_cast<Sprite *>(self + 0x10e4)->extract(&buf, frame, 1, 1, 0x8a, 6, 0);
    reinterpret_cast<Sprite *>(self + 0x1110)->extract(&buf, frame, 1, 8, 0x8a, 0x1c, 0);
    reinterpret_cast<Sprite *>(self + 0x113c)->extract(&buf, frame, 1, 0x25, 0x8a, 0x12, 0);
    reinterpret_cast<Sprite *>(self + 0x1198)->extract(&buf, frame, 1, 0x38, 0x8a, 0x18, 0);
    reinterpret_cast<Sprite *>(self + 0x116c)->extract(&buf, frame, 1, 0x51, 0x8a, 0x12, 0);

    char *slot = self + 0x1060;
    int x = 1;
    for (int i = 0; i < 3; ++i) {
        int y = x + (i == 1 ? 0x1e : 0);
        reinterpret_cast<Sprite *>(slot)->extract(&buf, frame, 0x106, y, 0xe, 0xe, 0);
        x += 0xf;
        slot += 0x2c;
    }

    char *btn = self + 0xfdc;
    x = 1;
    for (int j = 0; j < 3; ++j) {
        reinterpret_cast<Sprite *>(btn)->extract(&buf, frame, 0x8c, x, 0x79, 0x12, 0);
        x += 0x13;
        btn += 0x2c;
    }

    int y = *reinterpret_cast<int *>(self + 0x1158) * 7 +
             *reinterpret_cast<int *>(self + 0x11b4) +
             *reinterpret_cast<int *>(self + 0x1100);
    if (*g_0093f660 != 0) {
        y += 0xe + *reinterpret_cast<int *>(self + 0x11b4) +
             *reinterpret_cast<int *>(self + 0x112c);
    }

    *reinterpret_cast<int *>(self + 0xa38) = 0;
    *reinterpret_cast<int *>(self + 0xa3c) = 0;
    *reinterpret_cast<int *>(self + 0xa40) = 0x8a;
    *reinterpret_cast<int *>(self + 0xa44) = y;

    *reinterpret_cast<unsigned int *>(g_00939ed8) |= 0x40;
    reinterpret_cast<GraphicWin *>(this)->init(
        reinterpret_cast<RECT *>(self + 0xa38), 0, 0x1000000,
        reinterpret_cast<Win *>(g_007ae820), 0, 0);
    reinterpret_cast<GraphicWin *>(this)->fill(9);

    int height = *reinterpret_cast<int *>(self + 0xff8);
    int top = y - (*reinterpret_cast<int *>(self + 0x11b4) + height) / 2;

    if (*g_0093f660 != 0) {
        int str = reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(
                *reinterpret_cast<char **>(g_009b90f8) + 0x440));
        BaseButton *const b1 = reinterpret_cast<BaseButton *>(self + 0x1d10);
        b1->init(reinterpret_cast<char *>(str), 0, 10, top,
                 *reinterpret_cast<int *>(self + 0xff4),
                 *reinterpret_cast<int *>(self + 0xff8),
                 reinterpret_cast<Win *>(this), 0);
        reinterpret_cast<VCall *>(b1)->slot002();
        *reinterpret_cast<int *>(self + 0x27e4) = reinterpret_cast<int>(self + 0xfdc);
        *reinterpret_cast<int *>(self + 0x27e8) = reinterpret_cast<int>(self + 0x1008);
        *reinterpret_cast<int *>(self + 0x27ec) = reinterpret_cast<int>(self + 0x1034);
        *reinterpret_cast<int *>(self + 0x27cc) = 0x65;
        *reinterpret_cast<int *>(self + 0x27d0) = 0x41;
        *reinterpret_cast<int *>(self + 0x27d4) = 0x41;
        reinterpret_cast<VCall *>(b1)->slot062();
        top -= *reinterpret_cast<int *>(self + 0x11b4);
    }
    top -= *reinterpret_cast<int *>(self + 0x1158);

    int faction = *g_00939284;
    if (*g_009a6488 == 0 || (g_00946f58_arr[faction * 0x59c] & 0x80) == 0) {
        int str = reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(
                *reinterpret_cast<char **>(g_009b90f8) + 0x7e8));
        BaseButton *const b2 = reinterpret_cast<BaseButton *>(self + 0x11c4);
        b2->init(reinterpret_cast<char *>(str), 1, 10, top,
                 *reinterpret_cast<int *>(self + 0xff4),
                 *reinterpret_cast<int *>(self + 0xff8),
                 reinterpret_cast<Win *>(this), 0);
        reinterpret_cast<VCall *>(b2)->slot002();
        *reinterpret_cast<int *>(self + 0x1c98) = reinterpret_cast<int>(self + 0xfdc);
        *reinterpret_cast<int *>(self + 0x1c9c) = reinterpret_cast<int>(self + 0x1008);
        *reinterpret_cast<int *>(self + 0x1ca0) = reinterpret_cast<int>(self + 0x1034);
        *reinterpret_cast<int *>(self + 0x1c80) = 0x65;
        *reinterpret_cast<int *>(self + 0x1c84) = 0x41;
        *reinterpret_cast<int *>(self + 0x1c88) = 0x41;
        reinterpret_cast<VCall *>(b2)->slot062();
    }

    int moveX, moveY;
    if (*g_009b7b1c == 800) {
        moveY = *reinterpret_cast<int *>(self + 0x4c8) + 0x166;
        moveX = 800 - *reinterpret_cast<int *>(self + 0x4c4);
    } else {
        moveY = *reinterpret_cast<int *>(self + 0x4c8) - 0xff + *g_009b7b20;
        moveX = (*g_009b7b1c + 0x400) / 2 - *reinterpret_cast<int *>(self + 0x4c4);
    }
    reinterpret_cast<Win *>(this)->move(moveX, moveY);
    *reinterpret_cast<int *>(self + 0xa24) = 0;

    if (*g_0093f660 != 0) {
        reinterpret_cast<Win *>(this)->show(0);
    }

    reinterpret_cast<Wave *>(g_007fff00)->init(
        reinterpret_cast<char *>(g_006867d0), 4);
    reinterpret_cast<Sound *>(g_007fff00)->set_pan(0x20);
    reinterpret_cast<Wave *>(g_007fff00)->set_volume(0x28);
}
