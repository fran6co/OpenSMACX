// ORIGINAL: 0x0043D290 ?on_iface_right_click@DesignWin@@QAEXHH@Z 0x0043D290-0x0043D327 FILE
// working copy - scaffold materialised by --work
// size      151 bytes
// prototype void (__thiscall ?on_iface_right_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int xCoord, int yCoord)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDDE0 0x005ED240 0x005F83D0 0x005FAB00 0x006169A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_right_click@DesignWin@@QAEXHH@Z  at 0x0043D290  (151 bytes)
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
class Buffer;
class ButtonGroup;
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
struct MenuEntry;
typedef void * PVOID;
class Popup;
class PullDown;
struct PullDownItem;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
class Time;
class TutWin;
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
    void client_to_screen(int *, int *);
    void set_bubble_text(char *, RECT *);
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
    int check(int, int, int *, int *);
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

class BasePop : public GraphicWin { public:
    uint32_t field_A14_;
    uint32_t field_A18_;
    uint32_t field_A1C_;
    uint32_t field_A20_;
    uint32_t field_A24_;
    Heap heap_;
    uint32_t field_A3C_;
    uint32_t field_A40_;
    uint32_t field_A44_;
    uint32_t field_A48_;
    uint32_t field_A4C_;
    uint32_t field_A50_;
    uint32_t field_A54_;
    uint32_t field_A58_;
    FlatButton flat_button1_;
    FlatButton flat_button2_;
    uint32_t field_20F4_;
    uint32_t field_20F8_;
    uint32_t field_20FC_;
    uint32_t field_2100_;
    uint32_t field_2104_;
    uint32_t field_2108_;
    uint32_t field_210C_;
    uint32_t field_2110_;
    uint32_t field_2114_;
    Sprite sprite_;
    uint32_t field_2144_;
    uint32_t field_2148_;
    uint32_t field_214C_;
    uint32_t field_2150_;
    uint32_t field_2154_;
    uint32_t field_2158_;
    uint32_t field_215C_;
    uint32_t field_2160_;
    uint32_t field_2164_;
    uint32_t field_2168_;
    uint32_t field_216C_;
    uint32_t field_2170_;
    uint32_t field_2174_;
    uint32_t field_2178_;
    uint32_t field_217C_;
    uint32_t field_2180_;
    uint32_t field_2184_;
    uint32_t field_2188_;
    uint32_t field_218C_;
    uint32_t field_2190_;
    uint32_t field_2194_;
    uint32_t field_2198_;
    uint32_t field_219C_;
    uint32_t field_21A0_;
    uint32_t field_21A4_;
    uint32_t field_21A8_;
    uint32_t field_21AC_;
    uint32_t field_21B0_;
    uint32_t field_21B4_;
    uint32_t field_21B8_;
    uint32_t field_21BC_;
    uint32_t field_21C0_;
    uint32_t field_21C4_;
    uint32_t field_21C8_;
    uint32_t field_21CC_;
    uint8_t dialogs_[0xC94];
    uint32_t field_2E64_;
    uint32_t field_2E68_;
    uint32_t field_2E6C_;
    uint32_t field_2E70_;
    uint32_t field_2E74_;
    uint32_t field_2E78_;
    uint32_t field_2E7C_;
    uint32_t field_2E80_;
    uint32_t field_2E84_;
    uint32_t field_2E88_;
    uint32_t field_2E8C_;
    uint32_t field_2E90_;
    uint32_t field_2E94_;
    uint32_t field_2E98_;
    uint32_t field_2E9C_;
    uint32_t field_2EA0_;
    uint32_t field_2EA4_;
    uint32_t field_2EA8_;
    uint32_t field_2EAC_;
    uint32_t field_2EB0_;
    uint32_t field_2EB4_;
    uint32_t field_2EB8_;
    uint32_t field_2EBC_;
    uint32_t field_2EC0_;
    uint32_t field_2EC4_;
    uint32_t field_2EC8_;
    uint32_t field_2ECC_;
    uint32_t field_2ED0_;
    uint32_t field_2ED4_;
    uint32_t field_2ED8_;
    uint32_t field_2EDC_;
    uint32_t field_2EE0_;
    uint32_t field_2EE4_;
    uint32_t field_2EE8_;
    uint32_t field_2EEC_;
    uint32_t field_2EF0_;
    uint32_t field_2EF4_;
    uint32_t field_2EF8_;
    uint32_t field_2EFC_;
    uint32_t field_2F00_;
    uint32_t field_2F04_;
    uint32_t field_2F08_;
    uint32_t field_2F0C_;
    uint32_t field_2F10_;
    uint32_t field_2F14_;
    uint32_t field_2F18_;
    uint32_t field_2F1C_;
    uint32_t field_2F20_;
    uint32_t field_2F24_;
    uint32_t field_2F28_;
    uint32_t field_2F2C_;
    uint32_t field_2F30_;
    uint32_t field_2F34_;
    uint32_t field_2F38_;
    uint32_t field_2F3C_;
    uint32_t field_2F40_;
    uint32_t field_2F44_;
    uint32_t field_2F48_;
    uint32_t field_2F4C_;
    uint32_t field_2F50_;
    uint32_t field_2F54_;
    uint32_t field_2F58_;
    uint32_t field_2F5C_;
    uint32_t field_2F60_;
    uint32_t field_2F64_;
    uint32_t field_2F68_;
    uint32_t field_2F6C_;
    uint32_t field_2F70_;
    uint32_t field_2F74_;
    uint32_t field_2F78_;
    uint32_t field_2F7C_;
    uint32_t field_2F80_;
    uint32_t field_2F84_;
    uint32_t field_2F88_;
    uint32_t field_2F8C_;
    uint32_t field_2F90_;
    uint32_t field_2F94_;
    uint32_t field_2F98_;
    uint32_t field_2F9C_;
    uint32_t field_2FA0_;
    uint32_t field_2FA4_;
    uint32_t field_2FA8_;
    uint32_t field_2FAC_;
    uint32_t field_2FB0_;
    uint32_t field_2FB4_;
    uint32_t field_2FB8_;
    uint32_t field_2FBC_;
    uint32_t field_2FC0_;
    uint32_t field_2FC4_;
    uint32_t field_2FC8_;
    uint32_t field_2FCC_;
    uint32_t field_2FD0_;
    uint32_t field_2FD4_;
    uint32_t field_2FD8_;
    uint32_t field_2FDC_;
    uint32_t field_2FE0_;
    uint32_t field_2FE4_;
    uint32_t field_2FE8_;
    uint32_t field_2FEC_;
    uint32_t field_2FF0_;
    uint32_t field_2FF4_;
    uint32_t field_2FF8_;
    uint32_t field_2FFC_;
    uint32_t field_3000_;
    uint32_t field_3004_;
    uint32_t field_3008_;
    uint32_t field_300C_;
    uint32_t field_3010_;
    uint32_t field_3014_;
    uint32_t field_3018_;
    uint32_t field_301C_;
    uint32_t field_3020_;
    uint32_t field_3024_;
    uint32_t field_3028_;
    uint32_t field_302C_;
    uint32_t field_3030_;
    uint32_t field_3034_;
    uint32_t field_3038_;
    uint32_t field_303C_;
    uint32_t field_3040_;
    uint32_t field_3044_;
    uint32_t field_3048_;
    uint32_t field_304C_;
    uint32_t field_3050_;
    uint32_t field_3054_;
    uint32_t field_3058_;
    uint32_t field_305C_;
    uint32_t field_3060_;
    uint32_t field_3064_;
    uint32_t field_3068_;
    uint32_t field_306C_;
    uint32_t field_3070_;
    uint32_t field_3074_;
    uint32_t field_3078_;
    uint32_t field_307C_;
    uint32_t field_3080_;
    uint32_t field_3084_;
    uint32_t field_3088_;
    uint32_t field_308C_;
    uint32_t field_3090_;
    uint32_t field_3094_;
    Spot spot_;
    uint32_t field_30A4_;
    uint32_t field_30A8_;
    uint32_t field_30AC_;
    uint32_t field_30B0_;
    uint32_t field_30B4_;
    uint32_t field_30B8_;
    uint32_t field_30BC_;
    uint32_t field_30C0_;
    uint32_t field_30C4_;
    uint32_t field_30C8_;
    uint32_t field_30CC_;
    uint32_t field_30D0_;
    uint32_t field_30D4_;
    uint32_t field_30D8_;
    uint32_t field_30DC_;
    uint32_t field_30E0_;
    uint32_t field_30E4_;
    uint32_t field_30E8_;
    uint32_t field_30EC_;
    uint32_t field_30F0_;
    uint32_t field_30F4_;
    uint32_t loc_a_;
    uint32_t loc_b_;
    uint32_t field_3100_;
    uint32_t field_3104_;
    uint32_t field_3108_;
    uint32_t field_310C_;
    Font * string_font1_;
    Font * string_font2_;
    Font * string_font3_;
    Font * string_font4_;
    int string_color_a_;
    int string_color_2a_;
    int string_color_3a_;
    int string_color_hyper_a_;
    int string_color_b_;
    int string_color_2b_;
    int string_color_3b_;
    int string_color_hyper_b_;
    int string_color_c_;
    int string_color_2c_;
    int string_color_3c_;
    int string_color_hyperc_;
    int string_color_d_;
    int string_color_2d_;
    int string_color_3d_;
    int string_color_hyper_d_;
    uint32_t field_3160_;
    uint32_t field_3164_;
    uint32_t field_3168_;
    Font * button_font1_;
    Font * button_font2_;
    Font * button_font3_;
    uint8_t button_color_a_;
    uint8_t button_color_2a_;
    uint8_t button_color_3a_;
    uint8_t field_317B_;
    uint32_t button_color_b_;
    uint32_t button_color_2b_;
    uint32_t button_color_3b_;
    uint32_t button_color_c_;
    uint32_t button_color_2c_;
    uint32_t button_color_3c_;
    uint32_t button_color_d_;
    uint32_t button_color_2d_;
    uint32_t button_color_3d_;
    uint8_t field_31A0_;
    uint8_t field_31A1_;
    uint8_t field_31A2_;
    uint8_t field_31A3_;
    uint32_t field_31A4_;
    uint32_t field_31A8_;
    uint32_t field_31AC_;
    uint32_t field_31B0_;
    uint32_t field_31B4_;
    uint32_t field_31B8_;
    uint32_t field_31BC_;
    uint32_t field_31C0_;
    uint32_t field_31C4_;
    uint32_t field_31C8_;
    uint32_t field_31CC_;
    uint32_t field_31D0_;
    uint32_t field_31D4_;
    uint32_t field_31D8_;
    uint32_t field_31DC_;
    uint32_t field_31E0_;
    uint32_t field_31E4_;
    uint32_t field_31E8_;
    uint32_t field_31EC_;
    uint32_t field_31F0_;
    uint32_t field_31F4_;
    uint32_t field_31F8_;
    uint32_t field_31FC_;
    uint32_t field_3200_;
    uint32_t field_3204_;
    uint32_t field_3208_;
    uint32_t field_320C_;
    uint32_t field_3210_;
    uint32_t field_3214_;
    uint32_t field_3218_;
    uint32_t field_321C_;
    uint32_t field_3220_;
    LPSTR ok_text_;
    LPSTR cancel_text_;
    uint32_t field_322C_;
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

class Popup : public BasePop { public:
    Scroll scroll_;
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

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
};

class TutWin : public Popup { public:
    int32_t field_537C_;
    int32_t field_5380_;
    uint32_t field_5384_;
    uint32_t field_5388_;
    uint32_t field_538C_;
    uint32_t field_5390_;
    uint32_t field_5394_;
    uint32_t field_5398_;
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t field_53B0_[0x4];
    uint32_t field_53B4_;
    int32_t field_53B8_;
    uint32_t field_53BC_;
    uint32_t field_53C0_;
    int32_t field_53C4_;
    uint32_t field_53C8_;
    uint32_t field_53CC_;
    uint32_t field_53D0_;
    int32_t field_53D4_;
    int reset();
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class DesignWin { public:
    uint8_t pad_0_[0x12764];
    Spot spot_;

    void on_iface_right_click(int, int);
};
void DesignWin::on_iface_right_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<TutWin *>(g_008c6e68)->reset();

    int outA, outB;
    int check = reinterpret_cast<Spot *>(self + 0x11d44)->check(a1, a2, &outA, &outB);
    if (check != -1 && outA >= 0) {
        int tmp = a1;
        a1 = a2;
        a2 = tmp;

        int idx = *reinterpret_cast<int *>(*g_009b90f8 + 0xce8);
        char *str = (char *)reinterpret_cast<Strings *>(g_009b90d8)->get(idx);

        reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&a2, &a1);

        RECT rect;
        rect.left = a2;
        rect.top = a1;
        rect.right = a2;
        rect.bottom = a1;
        reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(str, &rect);
    }
}
