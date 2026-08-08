// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004B19A0
// name           ?on_redraw@SocialWin@@QAEXXZ
// size           377 bytes
// measured tier  NO_COMPILE
// refusal        u004b19a0.cpp(211) : error C2079: 'buffer_' uses undefined class 'Buffer' u004b19a0.cpp(342) : error C2079: 'buffer_' uses undefined class 'Buffer'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b19a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_redraw@SocialWin@@QAEXXZ  at 0x004B19A0  (377 bytes)
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
struct BoxSpriteParams;
class Buffer;
class ButtonGroup;
class FlatButton;
class Font;
typedef void * HBITMAP;
struct HCURSOR;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
class Heap;
typedef char * LPSTR;
typedef void * LPVOID;
class MainInterface;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class SocialWin;
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

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
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

class SubInterface { public:
    uint32_t field_0_;
    uint32_t field_4_;
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
    int box_sprite(RECT *, BoxSpriteParams *);
    int set_clip(RECT *);
};
class MainInterface { public:
    void restore_back(int);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 62
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
    virtual void slot062();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669328 = (int *)0x00669328;
static int *const g_0078d5a0 = (int *)0x0078D5A0;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007aec64 = (int *)0x007AEC64;
static int *const g_007aec94 = (int *)0x007AEC94;

class SocialWin { public:
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
    SubInterface subIFace_;
    uint32_t field_A1C_;
    Spot spot_;
    uint32_t field_A2C_;
    uint32_t field_A30_;
    uint32_t field_A34_;
    uint32_t field_A38_;
    uint32_t field_A3C_;
    uint32_t field_A40_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    uint32_t field_A4C_;
    uint32_t field_A50_;
    uint32_t field_A54_;
    uint32_t field_A58_;
    uint32_t field_A5C_;
    uint32_t field_A60_;
    uint32_t field_A64_;
    uint32_t field_A68_;
    uint32_t field_A6C_;
    uint32_t field_A70_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    uint32_t field_A7C_;
    uint32_t field_A80_;
    uint32_t field_A84_;
    uint32_t field_A88_;
    uint32_t field_A8C_;
    uint32_t field_A90_;
    uint32_t field_A94_;
    uint32_t field_A98_;
    uint32_t field_A9C_;
    uint32_t field_AA0_;
    uint32_t field_AA4_;
    uint32_t field_AA8_;
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
    uint32_t field_AE0_;
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
    uint32_t field_B10_;
    uint32_t field_B14_;
    uint32_t field_B18_;
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
    uint32_t field_B4C_;
    uint32_t field_B50_;
    uint32_t field_B54_;
    uint32_t field_B58_;
    uint32_t field_B5C_;
    uint32_t field_B60_;
    uint32_t field_B64_;
    uint32_t field_B68_;
    uint32_t field_B6C_;
    uint32_t field_B70_;
    uint32_t field_B74_;
    uint32_t field_B78_;
    uint32_t field_B7C_;
    uint32_t field_B80_;
    uint32_t field_B84_;
    uint32_t field_B88_;
    uint32_t field_B8C_;
    uint32_t field_B90_;
    uint32_t field_B94_;
    uint32_t field_B98_;
    uint32_t field_B9C_;
    uint32_t field_BA0_;
    uint32_t field_BA4_;
    uint32_t field_BA8_;
    uint32_t field_BAC_;
    uint32_t field_BB0_;
    uint32_t field_BB4_;
    uint32_t field_BB8_;
    uint32_t field_BBC_;
    uint32_t field_BC0_;
    uint32_t field_BC4_;
    uint32_t field_BC8_;
    uint32_t field_BCC_;
    uint32_t field_BD0_;
    uint32_t field_BD4_;
    uint32_t field_BD8_;
    uint32_t field_BDC_;
    uint32_t field_BE0_;
    uint32_t field_BE4_;
    uint32_t field_BE8_;
    uint32_t field_BEC_;
    uint32_t field_BF0_;
    uint32_t field_BF4_;
    uint32_t field_BF8_;
    uint32_t field_BFC_;
    uint32_t field_C00_;
    uint32_t field_C04_;
    uint32_t field_C08_;
    uint32_t field_C0C_;
    uint32_t field_C10_;
    uint32_t field_C14_;
    uint32_t field_C18_;
    uint32_t field_C1C_;
    uint32_t field_C20_;
    uint32_t field_C24_;
    uint32_t field_C28_;
    uint32_t field_C2C_;
    uint32_t field_C30_;
    uint32_t field_C34_;
    uint32_t field_C38_;
    uint32_t field_C3C_;
    uint32_t field_C40_;
    uint32_t field_C44_;
    uint32_t field_C48_;
    uint32_t field_C4C_;
    uint32_t field_C50_;
    uint32_t field_C54_;
    uint32_t field_C58_;
    uint32_t field_C5C_;
    uint32_t field_C60_;
    uint32_t field_C64_;
    uint32_t field_C68_;
    uint32_t field_C6C_;
    uint32_t field_C70_;
    uint32_t field_C74_;
    uint32_t field_C78_;
    uint32_t field_C7C_;
    uint32_t field_C80_;
    uint32_t field_C84_;
    uint32_t field_C88_;
    uint32_t field_C8C_;
    uint32_t field_C90_;
    uint32_t field_C94_;
    uint32_t field_C98_;
    uint32_t field_C9C_;
    uint32_t field_CA0_;
    uint32_t field_CA4_;
    uint32_t field_CA8_;
    uint32_t field_CAC_;
    uint32_t field_CB0_;
    uint32_t field_CB4_;
    uint32_t field_CB8_;
    uint32_t field_CBC_;
    uint32_t field_CC0_;
    uint32_t field_CC4_;
    uint32_t field_CC8_;
    uint32_t field_CCC_;
    uint32_t field_CD0_;
    uint32_t field_CD4_;
    uint32_t field_CD8_;
    uint32_t field_CDC_;
    uint32_t field_CE0_;
    uint32_t field_CE4_;
    uint32_t field_CE8_;
    uint32_t field_CEC_;
    uint32_t field_CF0_;
    uint32_t factionID_;
    uint8_t socCategory_[0x10];
    uint8_t socEffect_[0x2C];
    uint8_t socEffectOrig_[0x2C];
    uint32_t netIncome_;
    uint32_t breakthroughTurns_;
    uint8_t energyLockButtons_[0x1E84];
    uint32_t field_2BEC_;
    uint32_t field_2BF0_;
    uint32_t field_2BF4_;
    uint32_t field_2BF8_;
    uint32_t field_2BFC_;
    uint32_t field_2C00_;
    uint32_t field_2C04_;
    uint32_t field_2C08_;
    uint32_t field_2C0C_;
    uint32_t field_2C10_;
    uint32_t field_2C14_;
    uint32_t field_2C18_;
    uint32_t field_2C1C_;
    uint32_t field_2C20_;
    uint32_t effectMaximize_;
    uint32_t field_2C28_;
    uint32_t field_2C2C_;
    uint32_t field_2C30_;
    uint32_t field_2C34_;
    uint32_t economyAllocOrig_;
    uint32_t psychAllocOrig_;
    uint32_t labsAllocOrig_;
    uint32_t economyAlloc3_;
    uint32_t psychAlloc3_;
    uint32_t labsAlloc3_;
    uint32_t economyAllocPending_;
    uint32_t psychAllocPending_;
    uint32_t labsAllocPending_;
    uint32_t field_2C5C_;
    uint8_t socEngModifierNums_[0x1B8];
    uint8_t effectIcons_[0xF78];
    uint8_t energyAllocLock_[0x58];
    uint8_t energyAllocArrow_[0x108];
    uint8_t energyAllocSlider_[0x2130];
    FlatButton flatButtons_[35];
    ButtonGroup buttonGroups_[4];
    ButtonGroup buttonGroup_;
    Font font1_;
    Font font2_;
    Font font3_;
    Font font4_;
    Font font5_;
    Font font6_;
    Font font7_;
    Font font8_;
    Font font9_;
    uint8_t tutRects1_[0x90];
    uint8_t tutRect1_[0x10];
    uint8_t tutRects2_[0xA0];
    uint8_t tooltipSocEng_[0x10];
    uint8_t tutRect4_[0x10];
    uint8_t tutRects3_[0x120];
    uint8_t tooltipEffects_[0x10];
    uint8_t tooltipEconomy_[0x10];
    uint8_t tooltipLabs_[0x10];
    uint8_t tooltipPsych_[0x10];
    uint8_t tutRect9_[0x10];
    uint8_t tooltipFactionPower_[0x10];
    uint8_t tutRect11_[0x10];
    uint8_t tutRect12_[0x10];
    uint8_t tutRect13_[0x10];
    uint8_t tutRect14_[0x10];

    void draw_presets();
    void draw_social(int);
    void draw_faction_basal();
    void draw_info();
    void draw_overview();
    void draw_economy_alloc();
    void draw_labs_alloc();
    void draw_psych_alloc();
    void draw_mouseover();
    void on_redraw();
};

// NO_COMPILE, and NOT from this body: `tools/emit_translation_unit.py`
// (re-run fresh, not just the on-disk copy) emits `GraphicWin`/`Win` with a
// by-value `Buffer buffer_;` member BEFORE `class Buffer { ... };`'s own
// definition later in the same scaffold - C2079 ('buffer_' uses undefined
// class 'Buffer') at scaffold lines 211 and 342. Confirmed independent of
// the body: an EMPTY `void SocialWin::on_redraw() {}` hits the identical
// two errors, at the identical lines. Neither GraphicWin nor Win is
// referenced anywhere in this function or its callees - they are pulled in
// by the emitter's class-family grouping, whose topological sort orders
// Buffer after its own dependents for this particular subject. This is a
// toolchain-level wall (the emitter, which is out of scope to edit here),
// not a per-function translation gap. Left below is a faithful,
// self-consistent translation of the disassembly for whenever the scaffold
// bug is fixed.
//
// STRUCTURE, offsets in hex relative to `this` (SocialWin*), each read
// directly off the disassembly above:
//   member   | SocialWin | 0x444   | embedded Buffer (its own set_clip/
//            |           |         | box_sprite `this`) - `lea edi,[esi+0x444]`
//   member   | SocialWin | 0x1f260 | RECT, unioned with 0x1f270/0x1f280 into
//            |           |         | a padded box_sprite rect - three `lea`s
//            |           |         | feeding UnionRect, then +-3 on each edge
//   member   | SocialWin | 0x1f2b0 | RECT, same pattern with 0x1f2c0/0x1f2d0
//   member   | SocialWin | 0x1202c | array of polymorphic objects, byte
//            |           |         | stride 0xb4c, spanning to +0x7c44 with
//            |           |         | one element (+0x21e4) excluded and the
//            |           |         | index folded by -0xb4c past it - each
//            |           |         | dispatches virtual slot 62 (0xf8/4)
//   callgraph| -         | 0x669328| Win32 `UnionRect` import cell, called
//            |           |         | through as a raw __stdcall(RECT*,
//            |           |         | const RECT*, const RECT*) function
//            |           |         | pointer, not a catalogued symbol -
//            |           |         | `mov ebx,[0x669328]; ...; call ebx`
//            |           |         | with no `add esp` after (callee-pop)
//   emitter  | -         | -       | GraphicWin/Win's by-value `Buffer
//            |           |         | buffer_;` precedes `class Buffer`'s
//            |           |         | own definition in the emitted
//            |           |         | scaffold for this address - C2079 on
//            |           |         | an EMPTY body too, so it is the
//            |           |         | scaffold's class ordering, not this
//            |           |         | translation
//
// g_00669328 holds a plain function-pointer variable (the Win32 UnionRect
// import cell) - a single dereference, so the scaffold's pointer-constant
// spelling is right (plain load).
typedef int (__stdcall *UnionRectFn)(RECT *, const RECT *, const RECT *);

void SocialWin::on_redraw() {
    reinterpret_cast<Buffer *>(g_007aec64)->set_clip(reinterpret_cast<RECT *>(g_007aec94));

    char *self = reinterpret_cast<char *>(this);
    Buffer *buf = reinterpret_cast<Buffer *>(self + 0x444);
    buf->set_clip(reinterpret_cast<RECT *>(self + 0x444 + 0x30));

    reinterpret_cast<MainInterface *>(g_007ae820)->restore_back(1);

    UnionRectFn unionRect = *reinterpret_cast<UnionRectFn *>(g_00669328);

    RECT local14;
    unionRect(&local14, reinterpret_cast<RECT *>(self + 0x1f270),
              reinterpret_cast<RECT *>(self + 0x1f280));
    unionRect(&local14, &local14, reinterpret_cast<RECT *>(self + 0x1f260));
    local14.right += 3;
    local14.left -= 3;
    local14.bottom += 3;
    local14.top -= 3;
    reinterpret_cast<Buffer *>(g_007aec64)->box_sprite(
        &local14, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    unionRect(&local14, reinterpret_cast<RECT *>(self + 0x1f2b0),
              reinterpret_cast<RECT *>(self + 0x1f2d0));
    unionRect(&local14, &local14, reinterpret_cast<RECT *>(self + 0x1f2c0));
    local14.left -= 3;
    local14.right += 3;
    local14.bottom += 3;
    local14.top -= 3;
    buf->box_sprite(&local14, reinterpret_cast<BoxSpriteParams *>(g_0078d5a0));

    draw_presets();
    for (int i = 0; i < 4; i++) {
        draw_social(i);
    }
    draw_faction_basal();
    draw_info();
    draw_overview();
    draw_economy_alloc();
    draw_labs_alloc();
    draw_psych_alloc();
    draw_mouseover();

    for (int j = 0; j < 0x7c44; j += 0xb4c) {
        if (j != 0x21e4) {
            int idx = j;
            if (j >= 0x21e4) {
                idx = j - 0xb4c;
            }
            reinterpret_cast<VCall *>(self + 0x1202c + idx)->slot062();
        }
    }
}
