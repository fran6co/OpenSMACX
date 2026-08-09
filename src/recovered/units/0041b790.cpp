// ORIGINAL: 0x0041B790 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041B790
// name           ?base_editor_fac@BaseWin@@QAEXXZ
// size           1262 bytes
// measured tier  NO_COMPILE
// refusal        u0041b790.cpp(152) : error C2079: 'spot_' uses undefined class 'Spot' u0041b790.cpp(241) : error C2079: 'heap_' uses undefined class 'Heap' u0041b790.cpp(891) :
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041b790/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_editor_fac@BaseWin@@QAEXXZ  at 0x0041B790  (1262 bytes)
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
class BasePop;
class BaseWin;
class Buffer;
class ButtonGroup;
class Dialogs;
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
class Popup;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
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
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    void shutdown();
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
class BasePop { public:
    int exec(int, int (__cdecl *)());
    int set_cancel_text(char *);
    int set_ok_text(char *);
    void close();
};
class Dialogs { public:
    int item(char *, int);
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
    ~FlatButton();
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
    ~GraphicWin();
};
class Popup { public:
    Popup();
    void close();
    void start(char *, const char *, int, char *, int, GraphicWin *);
};
class Scroll { public:
    void close();
};
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    ~Spot();
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
    void close();
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
bool __cdecl has_fac(int, int, int);
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _alloca_probe();
extern "C" int __cdecl sub_406820();
extern "C" int __cdecl sub_406af0();
int __cdecl facility_avail(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl bitmask(int, int *, int *);
void __cdecl set_fac(int, int, int);

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
static int *const g_00652554 = (int *)0x00652554;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_006698cc = (int *)0x006698CC;
static int *const g_006698d4 = (int *)0x006698D4;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_006730b0 = (int *)0x006730B0;
static int *const g_00682d9c = (int *)0x00682D9C;
static int *const g_00682da8 = (int *)0x00682DA8;
static int *const g_00682dac = (int *)0x00682DAC;
static int *const g_00682db0 = (int *)0x00682DB0;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_009a4b98 = (int *)0x009A4B98;
static int *const g_009a5798 = (int *)0x009A5798;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b8aa8 = (int *)0x009B8AA8;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void base_editor_fac();
};

// The original builds this dialog's widgets as local automatic-storage
// C++ objects (Popup/BasePop/Scroll/FlatButton/BaseButton/GraphicWin/
// Spot/Dialogs/Sprite/Heap), which is why VC6 emits the fs:[0] SEH
// prologue/epilogue and the 0x652xxx cleanup funclets in the
// disassembly. None of those classes has a real sizeof() pinned in the
// catalogue here (all opaque/zero-sized shells), so the compiler cannot
// be made to reconstruct that exact frame or fs:[0] chain - matches
// 0047E640's ruled-out approach for the same class of BaseWin/NetWin
// "editor dialog" method. This body follows the CALL SEQUENCE and every
// argument faithfully instead of chasing those bytes.
inline void *__cdecl operator new(unsigned int, void *p) { return p; }

// Base-record "already built" flags table, stride 0x134 bytes - a single
// indexed access, not a register-advanced walk, so the folded pointer
// constant is correct (matches 00440080's proven g_0097d0cc for this
// exact table).
static uint8_t *const g_0097d0cc = (uint8_t *)0x0097D0CC;

void BaseWin::base_editor_fac() {
    unsigned char popupBuf[0x2000];
    unsigned char scrollBuf[0x400];
    unsigned char flatBtn1Buf[0x400];
    unsigned char flatBtn2Buf[0x400];
    unsigned char spotBuf[0x100];
    unsigned char dialogsBuf[0x400];
    unsigned char helperBuf[0x100];
    unsigned char helper2Buf[0x100];
    unsigned char spriteBuf[0x100];
    unsigned char flatBtn3Buf[0x400];
    unsigned char flatBtn4Buf[0x400];
    unsigned char heapBuf[0x100];

    char *self = reinterpret_cast<char *>(this);

    Popup *popup = new (popupBuf) Popup();
    BasePop *basePop = reinterpret_cast<BasePop *>(popup);

    int result;
    for (;;) {
        parse_says(0, reinterpret_cast<char *>(*g_0090ea30) + 0x13, -1, -1);
        *reinterpret_cast<int *>(g_006a721c) = 6;

        popup->start(reinterpret_cast<char *>(g_009b8aa8),
                     reinterpret_cast<const char *>(g_00682d9c), -1, 0, 0x42, 0);

        basePop->set_cancel_text(reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x438))));
        basePop->set_ok_text(reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0xfc0))));

        int facIdx = 1;
        const char *rec = reinterpret_cast<const char *>(g_009a4b98);
        while (rec != reinterpret_cast<const char *>(g_009a5798)) {
            *reinterpret_cast<uint8_t *>(g_009b86a0) = 0;
            int textId;

            if (has_fac(facIdx, *g_00689370, 0)) {
                textId = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0xd50);
            } else {
                strcat(reinterpret_cast<char *>(g_009b86a0),
                       reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
                           *reinterpret_cast<const int *>(rec))));
                strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682da8));

                int byteField = *reinterpret_cast<uint8_t *>(*reinterpret_cast<char **>(g_0090ea30) + 4);
                *reinterpret_cast<uint8_t *>(g_009b86a0) = 0;
                if (facility_avail(facIdx, byteField, *g_00689370, 0)) {
                    strcat(reinterpret_cast<char *>(g_009b86a0),
                           reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
                               *reinterpret_cast<const int *>(rec))));
                    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682dac));
                    textId = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x3dc);
                } else {
                    strcat(reinterpret_cast<char *>(g_009b86a0),
                           reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
                               *reinterpret_cast<const int *>(rec))));
                    strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682db0));
                    textId = *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0x4d8);
                }
            }

            strcat(reinterpret_cast<char *>(g_009b86a0),
                   reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(textId)));

            reinterpret_cast<Dialogs *>(dialogsBuf)->item(reinterpret_cast<char *>(g_009b86a0), facIdx);

            rec += 0x30;
            facIdx++;
        }

        result = basePop->exec(0, 0);
        *reinterpret_cast<int *>(g_006a721c) = 0;
        if (result < 0) {
            break;
        }

        int fieldAt40b0c = *reinterpret_cast<int *>(self + 0x40b0c);
        bool alreadyBuilt = false;
        if (result < 0x41) {
            int m_off, m_mask;
            bitmask(result, &m_off, &m_mask);
            uint8_t tblByte = g_0097d0cc[*g_00689370 * 0x134 + m_off];
            alreadyBuilt = (tblByte & m_mask) != 0;
        }
        set_fac(result, fieldAt40b0c, alreadyBuilt ? 0 : 1);
        reinterpret_cast<VCall *>(this)->slot062();
    }

    *reinterpret_cast<void **>(popupBuf + 0) = g_006695c8;
    *reinterpret_cast<void **>(popupBuf + 0x444) = g_006695c0;
    reinterpret_cast<Popup *>(popupBuf)->close();

    *reinterpret_cast<void **>(scrollBuf + 0) = g_00669d58;
    *reinterpret_cast<void **>(scrollBuf + 0x444) = g_00669d50;
    reinterpret_cast<Scroll *>(scrollBuf)->close();

    *reinterpret_cast<void **>(flatBtn1Buf + 0) = g_00669754;
    *reinterpret_cast<void **>(flatBtn1Buf + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(flatBtn1Buf)->close();
    reinterpret_cast<BaseButton *>(flatBtn1Buf)->~BaseButton();

    *reinterpret_cast<void **>(flatBtn2Buf + 0) = g_00669754;
    *reinterpret_cast<void **>(flatBtn2Buf + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(flatBtn2Buf)->close();
    reinterpret_cast<BaseButton *>(flatBtn2Buf)->~BaseButton();

    reinterpret_cast<GraphicWin *>(popupBuf)->~GraphicWin();

    *reinterpret_cast<void **>(popupBuf + 0) = g_006698d4;
    *reinterpret_cast<void **>(popupBuf + 0x444) = g_006698cc;
    reinterpret_cast<BasePop *>(popupBuf)->close();

    reinterpret_cast<Spot *>(spotBuf)->~Spot();
    (void)dialogsBuf;
    sub_406af0();
    (void)helperBuf;
    sub_406820();
    (void)helper2Buf;
    sub_406820();

    reinterpret_cast<Sprite *>(spriteBuf)->close();
    reinterpret_cast<FlatButton *>(flatBtn3Buf)->~FlatButton();
    reinterpret_cast<FlatButton *>(flatBtn4Buf)->~FlatButton();
    reinterpret_cast<Heap *>(heapBuf)->shutdown();

    reinterpret_cast<GraphicWin *>(popupBuf)->~GraphicWin();
}
