// ORIGINAL: 0x004AF110 FILE
// working copy - scaffold materialised by --work
// name      ?maximize_effect@SocialWin@@QAEXH@Z
// size      199 bytes
// spans     0x004AF110-0x004AF1D7
// prototype void (__thiscall ?maximize_effect@SocialWin@@QAEXH@Z)(SocialWin* this, int effectID)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004AEBF0 0x005B4730 0x0062B870
// indirect  0x004AF1BE

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?maximize_effect@SocialWin@@QAEXH@Z  at 0x004AF110  (199 bytes)
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
class CheckButton;
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
struct MenuEntry;
typedef void * PVOID;
class PullDown;
struct PullDownItem;
struct RECT;
class Scroll;
class SocialWin;
class Spot;
class Sprite;
class SubInterface;
class Time;
typedef unsigned int UINT_PTR;
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

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
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
    uint32_t field_4BC_;
    uint8_t field_4C0_[0x4C];
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

class GraphicWin : public Win { public:
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

class BaseButton : public GraphicWin { public:
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
    int set(int, int);
};

class CheckButton : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t isToggled_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_A28_;
};

class FlatButton : public BaseButton { public:
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

struct MenuEntry {
    int id;
    char * text;
    uint8_t flags;
    uint8_t padding[3];
    char * mnemonic;
    PullDown * pull_down;
};

class Menu : public GraphicWin { public:
    void * proc_;
    int count_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    uint32_t field_A28_;
    Spot spot_;
    MenuEntry entries_[15];
};

struct PullDownItem {
    char * text;
    char * right_text;
    int id;
    uint32_t flags;
    char * mnemonic;
};

class PullDown : public GraphicWin { public:
    uint32_t field_A14_;
    PullDownItem items_[64];
    uint32_t field_F18_;
    uint32_t field_F1C_;
    int item_count_;
    int width_;
    int selected_;
    int visible_count_;
    Menu * menu_;
    uint8_t dirty_;
    uint8_t dirty_padding_[3];
    uint32_t field_F38_;
    uint32_t field_F3C_;
};

class Scroll : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    int border_color_;
    int range_minimum_;
    int range_maximum_;
    uint32_t range_reversed_;
    int position_;
    uint32_t field_A30_;
    uint32_t field_A34_;
    uint32_t field_A38_;
    uint32_t field_A3C_;
    int paging_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    RECT thumb_rect_;
    uint32_t color_;
    int bar_thickness_;
    Buffer * buffer_;
    uint32_t bevel_thickness_;
    uint32_t bevel_upper_;
    uint32_t bevel_lower_;
    uint32_t field_A74_;
    Sprite * field_A78_;
    Sprite * sprite_left1_;
    Sprite * sprite_left2_;
    Sprite * sprite_left3_;
    Sprite * sprite_up1_;
    Sprite * sprite_up2_;
    Sprite * sprite_up3_;
    Sprite * sprite_right1_;
    Sprite * sprite_right2_;
    Sprite * sprite_right3_;
    Sprite * sprite_down1_;
    Sprite * sprite_down2_;
    Sprite * sprite_down3_;
    FlatButton flat_button_left_;
    FlatButton flat_button_right_;
    uint32_t field_2144_;
    uint32_t field_2148_;
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
    uint32_t vtable_;
    uint32_t field_4_;
};

int society_avail(int, int, int);

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
static int *const g_0094b024 = (int *)0x0094B024;

class SocialWin : public GraphicWin, public SubInterface { public:
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
    Sprite socEffect_;
    Sprite socEffectOrig_;
    uint32_t netIncome_;
    uint32_t breakthroughTurns_;
    CheckButton energyLockButtons_[3];
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
    RECT tutRects1_[9];
    RECT tutRect1_;
    RECT tutRects2_[10];
    uint8_t tooltipSocEng_[0x10];
    RECT tutRect4_;
    RECT tutRects3_[18];
    uint8_t tooltipEffects_[0x10];
    uint8_t tooltipEconomy_[0x10];
    uint8_t tooltipLabs_[0x10];
    uint8_t tooltipPsych_[0x10];
    RECT tutRect9_;
    uint8_t tooltipFactionPower_[0x10];
    RECT tutRect11_;
    RECT tutRect12_;
    RECT tutRect13_;
    RECT tutRect14_;

    void UNK1(int, int);
    void maximize_effect(int);
};
void SocialWin::maximize_effect(int a1) {
    effectMaximize_ = a1;
    int catBase = 0;
    ButtonGroup *group = &buttonGroups_[0];
    for (int j = 0; j < 4; ++j) {
        int bestScore = 0;
        int bestIdx = 0;
        int subIdx = 0;
        int cat = 0;
        do {
            int avail = society_avail(j, cat, factionID_);
            if (avail != 0 && bestScore < g_0094b024[catBase + subIdx + a1]) {
                bestIdx = cat;
                bestScore = g_0094b024[catBase + subIdx + a1];
            }
            subIdx += 0xb;
            ++cat;
        } while (subIdx < 0x2c);
        group->set(bestIdx + j * 4, 1);
        UNK1(j, bestIdx);
        catBase += 0x35;
        ++group;
    }
    ((VCall *)this)->slot062();
    effectMaximize_ = -1;
}
