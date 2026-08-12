// ORIGINAL: 0x00439F20 FILE
// name      ?select_chassis@DesignWin@@QAEXXZ
// size      1125 bytes
// spans     0x00439F20-0x0043A300;0x006541FC-0x00654281
// prototype void (__thiscall ?select_chassis@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   1   call targets   23
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004328B0 0x00433F70 0x004346A0 0x00434BC0 0x00434F40 0x00435400 0x00435800 0x00435B30 0x00435DE0 0x00436090 0x00439C00 0x00439D30 0x0043DD50 0x005D4DD0 0x005D4E40 0x005D7410 0x005D7470 0x00605370 0x00607040 0x00607DA0 0x00617020 0x00618EE0 0x00645550
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00439F20
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439f20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?select_chassis@DesignWin@@QAEXXZ  at 0x00439F20  (1125 bytes)
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
class Buffer;
class ButtonGroup;
class Caviar;
class DesignWin;
class FlatButton;
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
class SelectPartWin;
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

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
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
    void close();
    ~Buffer();
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
    ~BaseButton();
};
class Caviar { public:
    float scene_scale_;
    uint8_t unmapped_04_[0x2C - 0x04];
    int32_t field_2C_;
    int32_t field_30_;
    int32_t field_34_;
    uint8_t unmapped_38_[0x108 - 0x38];
    int32_t field_108_;
    uint8_t unmapped_10C_[0x13D0 - 0x10C];
    void close();
};
class FlatButton { public:
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
    uint32_t field_AB8_;
    uint32_t field_ABC_;
    uint32_t field_AC0_;
    uint32_t field_AC4_;
    uint32_t field_AC8_;
    uint32_t field_ACC_;
    uint32_t field_AD0_;
    uint32_t field_AD4_;
    uint32_t field_AD8_;
    uint32_t field_ADC_;
    Sprite * sprite0_;
    uint32_t field_AE4_;
    uint32_t field_AE8_;
    uint32_t field_AEC_;
    uint32_t field_AF0_;
    uint32_t field_AF4_;
    uint32_t field_AF8_;
    uint32_t field_AFC_;
    uint32_t field_B00_;
    uint32_t field_B04_;
    uint32_t field_B08_;
    uint32_t field_B0C_;
    Sprite * sprite1_;
    Sprite * sprite2_;
    Sprite * sprite3_;
    uint32_t field_B1C_;
    uint32_t field_B20_;
    uint32_t field_B24_;
    uint32_t field_B28_;
    uint32_t field_B2C_;
    uint32_t field_B30_;
    uint32_t field_B34_;
    uint32_t field_B38_;
    uint32_t field_B3C_;
    uint32_t field_B40_;
    uint32_t field_B44_;
    uint32_t field_B48_;
    void close();
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
    void close();
    ~GraphicWin();
};
class Scroll { public:
    void close();
};
class SelectPartWin { public:
    SelectPartWin();
    int exec(int, int, int, int, int, int, int, int, int, int, unsigned char *);
};
extern "C" int __cdecl _alloca_probe();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 63
class VCall { public:
    virtual void slot000();
    virtual void slot001();
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00654277 = (int *)0x00654277;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0066b2b0 = (int *)0x0066B2B0;
static int *const g_0066b2b8 = (int *)0x0066B2B8;
static int *const g_00674780 = (int *)0x00674780;
static int *const g_006feb7c = (int *)0x006FEB7C;
static int *const g_0094a3be = (int *)0x0094A3BE;
static int *const g_0094a8ce = (int *)0x0094A8CE;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void draw_info();
    void draw_chassis(int);
    void draw_weapon(int);
    void draw_armor(int);
    void draw_reactor(int);
    void draw_special_1(int);
    void draw_special_2(int);
    void draw_vehicle(int);
    void check_abil();
    void suggest_abil(int);
    void draw_flash();
    void select_chassis();
};

// Placement new: needed to construct SelectPartWin's local storage since it
// is opaque and the emitter never defines placement new for us.
inline void *operator new(unsigned int, void *p) { return p; }

void DesignWin::select_chassis() {
    char *self = reinterpret_cast<char *>(this);
    unsigned char spwStorage[0x4670];
    char *spw = reinterpret_cast<char *>(spwStorage);
    new (spw) SelectPartWin();

    int32_t count = 0;
    int32_t idx = 0;
    {
        int32_t addr = 0x94a3be;
        do {
            if (*reinterpret_cast<int16_t *>(addr) >= -1) {
                if (idx == *reinterpret_cast<int32_t *>(self + 0x141fc))
                    goto LOOP1_DONE;
                count++;
            }
            addr += 0x90;
            idx++;
        } while (addr < 0x94a8ce);
    }
LOOP1_DONE:

    int32_t rectVals[4];
    rectVals[0] = *reinterpret_cast<int32_t *>(self + 0x14c);
    rectVals[1] = *reinterpret_cast<int32_t *>(self + 0x150);
    rectVals[2] = *reinterpret_cast<int32_t *>(self + 0x154);
    rectVals[3] = *reinterpret_cast<int32_t *>(self + 0x158);

    int32_t execResult = reinterpret_cast<SelectPartWin *>(spw)->exec(
        rectVals[0], rectVals[1],
        *reinterpret_cast<int32_t *>(self + 0x141f4), 0, count,
        *reinterpret_cast<int32_t *>(self + 0x141fc),
        *reinterpret_cast<int32_t *>(self + 0x14200),
        *reinterpret_cast<int32_t *>(self + 0x14204),
        *reinterpret_cast<int32_t *>(self + 0x14208),
        *reinterpret_cast<int32_t *>(self + 0x1420c),
        reinterpret_cast<unsigned char *>(g_006feb7c));

    int32_t newIdx = 0;
    {
        int32_t addr = 0x94a3be;
        do {
            if (*reinterpret_cast<int16_t *>(addr) >= -1) {
                if (execResult == 0)
                    goto LOOP2_DONE;
                execResult--;
            }
            addr += 0x90;
            newIdx++;
        } while (addr < 0x94a8ce);
    }
LOOP2_DONE:

    int32_t oldIdx = *reinterpret_cast<int32_t *>(self + 0x141fc);
    int32_t oldBase = oldIdx * 144;
    uint8_t oldChassisType = *reinterpret_cast<uint8_t *>(oldBase + 0x94a379);

    int32_t newBase = newIdx * 144;
    *reinterpret_cast<int32_t *>(self + 0x141fc) = newIdx;

    if (*reinterpret_cast<uint8_t *>(newBase + 0x94a37d) == 0) {
        int32_t armor = *reinterpret_cast<int32_t *>(self + 0x14200);
        *reinterpret_cast<int32_t *>(self + 0x14204) = 0;
        if (*reinterpret_cast<uint8_t *>((armor << 4) + 0x94ae6a) >= 2)
            *reinterpret_cast<int32_t *>(self + 0x14200) = 0;
        armor = *reinterpret_cast<int32_t *>(self + 0x14200);
        if (armor != 0x10 && armor != 0x18 && armor != 0x19)
            *reinterpret_cast<int32_t *>(self + 0x14200) = 0x17;
    } else {
        int32_t armor = *reinterpret_cast<int32_t *>(self + 0x14200);
        if (*reinterpret_cast<uint8_t *>((armor << 4) + 0x94ae68) >= 0x63 ||
            armor == 0x17 || armor == 0x18 || armor == 0x19) {
            *reinterpret_cast<int32_t *>(self + 0x14200) = 0;
        }
    }

    uint8_t newChassisType = *reinterpret_cast<uint8_t *>(newBase + 0x94a379);
    if (newChassisType != oldChassisType) {
        if (newChassisType != 0) {
            suggest_abil(2);
            int32_t curIdx = *reinterpret_cast<int32_t *>(self + 0x141fc);
            int32_t curBase = curIdx * 144;
            if (*reinterpret_cast<uint8_t *>(curBase + 0x94a379) == 2) {
                int32_t reactor = *reinterpret_cast<int32_t *>(self + 0x14200);
                if (*reinterpret_cast<uint8_t *>((reactor << 4) + 0x94ae6a) == 0xb)
                    *reinterpret_cast<int32_t *>(self + 0x14200) = 0;
                reactor = *reinterpret_cast<int32_t *>(self + 0x14200);
                if (*reinterpret_cast<uint8_t *>((reactor << 4) + 0x94ae6a) == 9) {
                    if (*reinterpret_cast<uint8_t *>(curBase + 0x94a37a) != 0)
                        *reinterpret_cast<int32_t *>(self + 0x14200) = 0;
                }
            }
        } else {
            int32_t abilFlags = *reinterpret_cast<int32_t *>(self + 0x14208);
            if ((abilFlags & 2) != 0) {
                abilFlags &= ~2;
                *reinterpret_cast<int32_t *>(self + 0x14208) = abilFlags;
                int32_t *arr = reinterpret_cast<int32_t *>(self + 0x14218);
                arr[0] = -1;
                arr[1] = -1;
                for (int32_t bit = 0; bit < 0x1d; bit++) {
                    int32_t flags = *reinterpret_cast<int32_t *>(self + 0x14208);
                    if ((flags & (1 << bit)) != 0) {
                        int32_t slot;
                        for (slot = 0; slot < 2; slot++) {
                            if (arr[slot] < 0)
                                break;
                        }
                        if (slot < 2)
                            arr[slot] = bit;
                    }
                }
            }
        }
    }

    check_abil();
    draw_chassis(0);
    draw_armor(0);
    draw_reactor(0);
    draw_weapon(0);
    draw_vehicle(0);
    draw_special_1(0);
    draw_special_2(0);
    draw_flash();
    draw_info();
    reinterpret_cast<VCall *>(this)->slot063();

    // Teardown of the local SelectPartWin (mirrors the Scroll/FlatButton
    // pair shape seen on the other GraphicWin-derived widgets: two vtable
    // slots 0x444 apart get reset to the current layer's vtable before
    // each non-virtual `close`/dtor call).
    *reinterpret_cast<int32_t *>(spw) = *g_0066b2b8;
    *reinterpret_cast<int32_t *>(spw + 0x444) = *g_0066b2b0;
    reinterpret_cast<GraphicWin *>(spw)->close();

    char *bufferObj = spw + 0x406c;
    reinterpret_cast<Buffer *>(bufferObj)->close();

    reinterpret_cast<Font *>(spw + 0x461c)->~Font();
    reinterpret_cast<Font *>(spw + 0x45f4)->~Font();
    reinterpret_cast<Buffer *>(bufferObj)->~Buffer();
    reinterpret_cast<Caviar *>(spw + 0x2b9c)->close();

    char *scroll = spw + 0xa14;
    *reinterpret_cast<int32_t *>(scroll) = *g_00669d58;
    *reinterpret_cast<int32_t *>(scroll + 0x444) = *g_00669d50;
    reinterpret_cast<Scroll *>(scroll)->close();

    char *btn1 = spw + 0x200c;
    *reinterpret_cast<int32_t *>(btn1) = *g_00669754;
    *reinterpret_cast<int32_t *>(btn1 + 0x444) = *g_0066974c;
    reinterpret_cast<FlatButton *>(btn1)->close();
    reinterpret_cast<BaseButton *>(btn1)->~BaseButton();

    char *btn2 = spw + 0x14c0;
    *reinterpret_cast<int32_t *>(btn2) = *g_00669754;
    *reinterpret_cast<int32_t *>(btn2 + 0x444) = *g_0066974c;
    reinterpret_cast<FlatButton *>(btn2)->close();
    reinterpret_cast<BaseButton *>(btn2)->~BaseButton();

    reinterpret_cast<GraphicWin *>(scroll)->~GraphicWin();
    reinterpret_cast<GraphicWin *>(spw)->~GraphicWin();
}
