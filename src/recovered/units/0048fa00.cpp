// ORIGINAL: 0x0048FA00 FILE
// name      ?display@PrefWin@@QAEXH@Z
// size      709 bytes
// spans     0x0048FA00-0x0048FCC5
// prototype void (__thiscall ?display@PrefWin@@QAEXH@Z)(PrefWin* this, int)
// callers   6   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00403830 0x0045D440 0x00490B00 0x004918C0 0x005E2DD3 0x005F2CE0 0x0062B8A0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0048FA00
// measured tier  NO_COMPILE
// refusal        u0048fa00.cpp(210) : error C2079: 'buffer_' uses undefined class 'Buffer' u0048fa00.cpp(345) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048fa00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?display@PrefWin@@QAEXH@Z  at 0x0048FA00  (709 bytes)
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

class AlphaMenu;
class AutoSound;
struct BITMAPINFO;
typedef int BOOL;
class BaseButton;
struct BoxSpriteParams;
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
typedef void * PVOID;
class PrefWin;
struct RECT;
class Scroll;
class Spot;
class Sprite;
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

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
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
class AlphaMenu { public:
    void hide();
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
};
class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    int button_click(int);
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
    void __cdecl set_def_focus(Win *);
};
void __cdecl slide_show(GraphicWin *, int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 1, 57, 62
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
    virtual void slot057();  // <-- used
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_00686b7c = (int *)0x00686B7C;
static int *const g_00686b80 = (int *)0x00686B80;
static int *const g_00686b84 = (int *)0x00686B84;
static int *const g_006970dc = (int *)0x006970DC;
static int *const g_0078d690 = (int *)0x0078D690;
static int *const g_0078d7f8 = (int *)0x0078D7F8;
static int *const g_008577d8 = (int *)0x008577D8;
static int *const g_008577dc = (int *)0x008577DC;
static int *const g_008577e0 = (int *)0x008577E0;
static int *const g_008577e4 = (int *)0x008577E4;
static int *const g_008577f0 = (int *)0x008577F0;
static int *const g_00857884 = (int *)0x00857884;
static int *const g_00857888 = (int *)0x00857888;
static int *const g_00857898 = (int *)0x00857898;
static int *const g_0085789c = (int *)0x0085789C;
static int *const g_008578a4 = (int *)0x008578A4;
static int *const g_008578a8 = (int *)0x008578A8;
static int *const g_008578c0 = (int *)0x008578C0;
static int *const g_008578d8 = (int *)0x008578D8;
static int *const g_00872ca8 = (int *)0x00872CA8;
static int *const g_009380dc = (int *)0x009380DC;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a6498 = (int *)0x009A6498;

class PrefWin { public:
    void get_preferences();
    void change_volume_button_text(int);
    void display(int);
};

class VCall2 { public:
    virtual void slot000();
    virtual void slot001(int);
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
    virtual void slot057(int, int, int);
};

extern AlphaMenu g_009380dc_menu;
extern int g_0078d690_arr[];
extern int g_0078d7f8_arr[];
extern Win g_008578d8_win;
extern int g_005398e0_target;
extern ButtonGroup g_008577f0_group;

void PrefWin::display(int a1) {
    char *self = reinterpret_cast<char *>(this);
    g_009380dc_menu.hide();

    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);

    *reinterpret_cast<int32_t *>(self + 0xa20) = 0;
    *reinterpret_cast<int32_t *>(self + 0xa24) = 0;
    *reinterpret_cast<int32_t *>(self + 0xa28) = 0;
    *reinterpret_cast<int32_t *>(self + 0xa30) = *g_009a6490;
    *reinterpret_cast<int32_t *>(self + 0xa34) = *g_009a6494;
    *reinterpret_cast<int32_t *>(self + 0xa38) = *g_009a6490;
    *reinterpret_cast<int32_t *>(self + 0xa3c) = *g_009a6494;
    *reinterpret_cast<int32_t *>(self + 0xa40) = *g_009a6490;
    *reinterpret_cast<int32_t *>(self + 0xa44) = *g_009a6494;
    *reinterpret_cast<int32_t *>(self + 0xa4c) = *g_009a6490;
    *reinterpret_cast<int32_t *>(self + 0xa50) = *g_009a6494;
    *reinterpret_cast<int32_t *>(self + 0xa48) = *g_009a6498;
    *reinterpret_cast<int32_t *>(self + 0xa54) = *g_009a6490;
    *reinterpret_cast<int32_t *>(self + 0xa58) = *g_009a6494;

    RECT rect1;
    rect1.left = 0x17;
    rect1.top = 0;
    rect1.right = *g_00686b7c + 0x17;
    rect1.bottom = *g_00686b80;
    buf->box_sprite(&rect1, reinterpret_cast<BoxSpriteParams *>(g_0078d690_arr));

    RECT rect2;
    rect2.left = 0x1b;
    rect2.top = 4;
    rect2.right = *g_00686b7c + 0x13;
    rect2.bottom = *g_00686b80 - 4;
    buf->box_sprite(&rect2, reinterpret_cast<BoxSpriteParams *>(g_0078d690_arr));

    RECT rect3;
    rect3.left = *g_00857888;
    rect3.top = *g_00857884;
    rect3.right = *g_00686b84 + *g_00857888;
    rect3.bottom = *g_008578c0 + *g_00857884;
    buf->box_sprite(&rect3, reinterpret_cast<BoxSpriteParams *>(g_0078d7f8_arr));

    get_preferences();

    *reinterpret_cast<int32_t *>(self + 0x15820) = 0;
    if (*reinterpret_cast<int32_t *>(self + 0x17a04) != 0) {
        *reinterpret_cast<int32_t *>(self + 0x15820) = 1;
    }
    if (*reinterpret_cast<int32_t *>(self + 0x16eb8) != 0) {
        *reinterpret_cast<int32_t *>(self + 0x15820) = 1;
    }
    if (*reinterpret_cast<int32_t *>(self + 0x1636c) != 0) {
        *reinterpret_cast<int32_t *>(self + 0x15820) = 1;
    }

    change_volume_button_text(0x55);

    RECT rect4;
    rect4.left = *g_0085789c;
    rect4.top = *g_00857898;
    rect4.right = *g_008578a8 + *g_0085789c;
    rect4.bottom = *g_008578a4 + *g_00857898;
    buf->box_sprite(&rect4, reinterpret_cast<BoxSpriteParams *>(g_0078d7f8_arr));

    if (*reinterpret_cast<int32_t *>(self + 0xa14) == 4) {
        RECT rect5;
        rect5.left = *g_008577d8;
        rect5.top = *g_008577dc - *g_00872ca8;
        rect5.right = *g_008577e4 + *g_008577d8;
        rect5.bottom = *g_008577e0 + (*g_008577dc - *g_00872ca8);
        buf->box_sprite(&rect5, reinterpret_cast<BoxSpriteParams *>(g_0078d690_arr));
    }

    reinterpret_cast<VCall2 *>(self + 0x18684)->slot001(0);
    reinterpret_cast<VCall2 *>(self + 0x19d1c)->slot001(0);
    reinterpret_cast<VCall2 *>(self + 0x191d0)->slot001(0);

    if (a1 >= 0) {
        *reinterpret_cast<int32_t *>(self + 0xa18) = *reinterpret_cast<int32_t *>(self + 0xa14);
        *reinterpret_cast<int32_t *>(self + 0xa14) = a1;
        g_008577f0_group.button_click(a1);
    }

    reinterpret_cast<VCall *>(self)->slot062();

    slide_show(reinterpret_cast<GraphicWin *>(self), 0);

    reinterpret_cast<VCall2 *>(self)->slot001(3);

    reinterpret_cast<Win *>(self)->set_def_focus(&g_008578d8_win);

    reinterpret_cast<VCall2 *>(self)->slot057(0, reinterpret_cast<int>(&g_005398e0_target), 0);

    uint32_t v = *reinterpret_cast<uint32_t *>(g_006970dc);
    if ((~(*g_009a6490) & 0x20000000) != 0) {
        v |= 4;
    } else {
        v &= 0xfb;
    }
    *reinterpret_cast<uint32_t *>(g_006970dc) = v;

}
