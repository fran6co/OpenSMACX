// ORIGINAL: 0x004AECE0 ?reset@SocialWin@@QAEXXZ 0x004AECE0-0x004AEEF1 FILE
// size      529 bytes
// prototype void (__thiscall ?reset@SocialWin@@QAEXXZ)(SocialWin* this)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00445130 0x005B4210 0x005B4600 0x005B4790 0x005F7E90 0x0062B870
// indirect  0x004AEEE4
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004AECE0
// measured tier  MISMATCH
// divergence     19
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004aece0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?reset@SocialWin@@QAEXXZ  at 0x004AECE0  (529 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    int set(int, int);
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
void __cdecl energy_compute(int, int);
void __cdecl social_ai(int, int, int, int, int, int *);
void __cdecl social_calc(int *, int *, int, int, int);
void __cdecl social_set(int);

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
static int *const g_00749ae4 = (int *)0x00749AE4;
static int *const g_00749c0c = (int *)0x00749C0C;
static int *const g_0096cc08 = (int *)0x0096CC08;
static int *const g_0096cc2c = (int *)0x0096CC2C;
static int *const g_0096cd08 = (int *)0x0096CD08;

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

    void reset();
};

// EXTERN-SYMBOL LEVER: all four tables are reached through a computed row
// index (factionID_ * 2099, an lea-chain strength reduction of a row stride
// of 0x20CC bytes), not a plain load, so they need real linkage.
extern int g_0096cc08_sym[];
extern int g_0096cc2c_sym[];
extern int g_0096cd04_sym[];
extern int g_0096cd08_sym[];

void SocialWin::reset() {
    field_A1C_ = 0xffffffff;
    social_ai(factionID_, -1, -1, -1, -1, 0);

    ButtonGroup *bg = &buttonGroups_[0];
    field_2C28_ = 1;
    {
        int i = 0;
        int *cat = reinterpret_cast<int *>(socCategory_);
        do {
            int val = g_0096cc08_sym[factionID_ * 2099 + i];
            *cat = val;
            bg->set(val + i * 4, 1);
            ++i;
            ++cat;
            ++bg;
        } while (i < 4);
    }

    field_2C28_ = 0;
    {
        int i = 0;
        do {
            int val = g_0096cc2c_sym[factionID_ * 2099 + i];
            *reinterpret_cast<int *>(socEffectOrig_) = val;
            *reinterpret_cast<int *>(socEffect_) = val;
            ++i;
        } while (i < 0xb);
    }

    int row = factionID_ * 2099;
    int changed;
    if (g_0096cd04_sym[row] == static_cast<int>(psychAllocOrig_) &&
        g_0096cd08_sym[row] == static_cast<int>(labsAllocOrig_)) {
        changed = 0;
    } else {
        changed = 1;
    }
    g_0096cd04_sym[row] = psychAllocOrig_;
    g_0096cd08_sym[row] = labsAllocOrig_;

    {
        uint32_t *p = &economyAllocPending_;
        int n = 3;
        do {
            *p = p[-6];
            ++p;
        } while (--n);
    }

    if (changed) {
        social_set(factionID_);
        social_calc(reinterpret_cast<int *>(socCategory_), reinterpret_cast<int *>(socEffect_), factionID_, 0, 0);
        social_calc(reinterpret_cast<int *>(socCategory_), &g_0096cc2c_sym[row], factionID_, 0, 0);
    }

    energy_compute(factionID_, 1);
    breakthroughTurns_ = *g_00749ae4;
    netIncome_ = *g_00749c0c;

    social_calc(&g_0096cc08_sym[row], &g_0096cc2c_sym[row], factionID_, 0, 0);

    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
