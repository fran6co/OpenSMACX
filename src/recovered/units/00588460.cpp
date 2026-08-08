// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00588460
// name           ?popups_normal@@YAXXZ
// size           914 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00588460/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?popups_normal@@YAXXZ  at 0x00588460  (914 bytes)
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
class Dialog;
struct DialogEntry;
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

class ButtonGroup { public:
    BaseButton * buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
};

struct DialogEntry {
    uint32_t vtable;
    int id;
    void * payload;
    DialogEntry * next;
    DialogEntry * previous;
    uint32_t secondary_vtable;
    void * heap;
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
    int __cdecl set_def_font(Font *, Font *, Font *);
    void __cdecl set_def_text_color(int, int, int, int);
};
class BasePop { public:
    int __cdecl set_def_button_font(Font *, Font *, Font *);
    int __cdecl set_def_string_font(Font *, Font *, Font *, Font *);
    void __cdecl set_def_button_color(int, int, int, int);
    void __cdecl set_def_button_color2(int, int, int, int);
    void __cdecl set_def_button_color3(int, int, int, int);
    void __cdecl set_def_string_color(int, int, int, int);
    void __cdecl set_def_string_color2(int, int, int, int);
    void __cdecl set_def_string_color3(int, int, int, int);
    void __cdecl set_def_string_color_hyper(int, int, int, int);
};
class Dialog { public:
    LPVOID vtable_;
    Heap heap_;
    Heap * heap_ptr_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    uint32_t text_color_a_;
    uint32_t text_color_2a_;
    uint32_t text_color_3a_;
    uint32_t text_color_b_;
    uint32_t text_color_2b_;
    uint32_t text_color_3b_;
    uint32_t text_color_c_;
    uint32_t text_color_2c_;
    uint32_t text_color_3c_;
    uint32_t text_color_d_;
    uint32_t text_color_2d_;
    uint32_t text_color_3d_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    DialogEntry * entry_head_;
    DialogEntry * current_entry_;
    int entry_count_;
    int entry_position_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    int selected_position_;
    uint32_t field_F0_;
    int __cdecl set_def_dialog_font(Font *, Font *, Font *);
    void __cdecl set_def_dialog_text_color(int, int, int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a5a0 = (int *)0x0068A5A0;
static int *const g_0068a5a4 = (int *)0x0068A5A4;
static int *const g_0068a5a8 = (int *)0x0068A5A8;
static int *const g_0068a5ac = (int *)0x0068A5AC;
static int *const g_0068a5b8 = (int *)0x0068A5B8;
static int *const g_0068a5bc = (int *)0x0068A5BC;
static int *const g_0068a5c0 = (int *)0x0068A5C0;
static int *const g_0068a5c4 = (int *)0x0068A5C4;
static int *const g_00696d34 = (int *)0x00696D34;
static int *const g_00696d38 = (int *)0x00696D38;
static int *const g_00696d3c = (int *)0x00696D3C;
static int *const g_00696d40 = (int *)0x00696D40;
static int *const g_00696d44 = (int *)0x00696D44;
static int *const g_00696d48 = (int *)0x00696D48;
static int *const g_00696d4c = (int *)0x00696D4C;
static int *const g_00696d50 = (int *)0x00696D50;
static int *const g_00696e08 = (int *)0x00696E08;
static int *const g_00696e0c = (int *)0x00696E0C;
static int *const g_00696e10 = (int *)0x00696E10;
static int *const g_00696e14 = (int *)0x00696E14;
static int *const g_00696e18 = (int *)0x00696E18;
static int *const g_00696e1c = (int *)0x00696E1C;
static int *const g_00696e30 = (int *)0x00696E30;
static int *const g_00696e34 = (int *)0x00696E34;
static int *const g_00696e38 = (int *)0x00696E38;
static int *const g_00696ed0 = (int *)0x00696ED0;
static int *const g_00696ed4 = (int *)0x00696ED4;
static int *const g_00696ee0 = (int *)0x00696EE0;
static int *const g_00696f54 = (int *)0x00696F54;
static int *const g_00696f58 = (int *)0x00696F58;
static int *const g_00696f5c = (int *)0x00696F5C;
static int *const g_0069702c = (int *)0x0069702C;
static int *const g_00697034 = (int *)0x00697034;
static int *const g_00697038 = (int *)0x00697038;
static int *const g_0069703c = (int *)0x0069703C;
static int *const g_00697040 = (int *)0x00697040;
static int *const g_0069704c = (int *)0x0069704C;
static int *const g_00697050 = (int *)0x00697050;
static int *const g_00697054 = (int *)0x00697054;
static int *const g_00697058 = (int *)0x00697058;
static int *const g_006970a8 = (int *)0x006970A8;
static int *const g_006970e0 = (int *)0x006970E0;
static int *const g_006970e8 = (int *)0x006970E8;
static int *const g_006970ec = (int *)0x006970EC;
static int *const g_006971f4 = (int *)0x006971F4;
static int *const g_006971f8 = (int *)0x006971F8;
static int *const g_006a721c = (int *)0x006A721C;
static int *const g_00759e20 = (int *)0x00759E20;
static int *const g_0075ade8 = (int *)0x0075ADE8;
static int *const g_00779644 = (int *)0x00779644;
static int *const g_00791d88 = (int *)0x00791D88;
static int *const g_00791db4 = (int *)0x00791DB4;
static int *const g_00791de0 = (int *)0x00791DE0;
static int *const g_00791e0c = (int *)0x00791E0C;
static int *const g_00791e38 = (int *)0x00791E38;
static int *const g_00791e64 = (int *)0x00791E64;
static int *const g_00791e90 = (int *)0x00791E90;
static int *const g_00791ebc = (int *)0x00791EBC;
static int *const g_00791ee8 = (int *)0x00791EE8;
static int *const g_00791f14 = (int *)0x00791F14;
static int *const g_00791f40 = (int *)0x00791F40;
static int *const g_00791f6c = (int *)0x00791F6C;
static int *const g_0079a5e8 = (int *)0x0079A5E8;
static int *const g_008c6bf4 = (int *)0x008C6BF4;
static int *const g_008c6bfc = (int *)0x008C6BFC;
static int *const g_008c6c04 = (int *)0x008C6C04;
static int *const g_008c6c0c = (int *)0x008C6C0C;
static int *const g_008c6c14 = (int *)0x008C6C14;
static int *const g_008c6c1c = (int *)0x008C6C1C;
static int *const g_009403d8 = (int *)0x009403D8;
static int *const g_0094575c = (int *)0x0094575C;
static int *const g_00945760 = (int *)0x00945760;
static int *const g_0094576c = (int *)0x0094576C;
static int *const g_009457ec = (int *)0x009457EC;
static int *const g_00945808 = (int *)0x00945808;
static int *const g_0094580c = (int *)0x0094580C;
static int *const g_009b7af8 = (int *)0x009B7AF8;
static int *const g_009b7afc = (int *)0x009B7AFC;
static int *const g_009b7b60 = (int *)0x009B7B60;
static int *const g_009b7b64 = (int *)0x009B7B64;
static int *const g_009b7b68 = (int *)0x009B7B68;
static int *const g_009b7b8c = (int *)0x009B7B8C;
static int *const g_009b7b90 = (int *)0x009B7B90;
static int *const g_009b8d88 = (int *)0x009B8D88;
static int *const g_009b8d8c = (int *)0x009B8D8C;
static int *const g_009b8d90 = (int *)0x009B8D90;
static int *const g_009b8d94 = (int *)0x009B8D94;
static int *const g_009b8db4 = (int *)0x009B8DB4;
static int *const g_009b8db8 = (int *)0x009B8DB8;
static int *const g_009b8dcc = (int *)0x009B8DCC;
static int *const g_009b8dd4 = (int *)0x009B8DD4;
static int *const g_009b8de8 = (int *)0x009B8DE8;
static int *const g_009b8dec = (int *)0x009B8DEC;
static int *const g_009b8df0 = (int *)0x009B8DF0;
static int *const g_009b8df4 = (int *)0x009B8DF4;
static int *const g_009b8df8 = (int *)0x009B8DF8;
static int *const g_009b8dfc = (int *)0x009B8DFC;
static int *const g_009b8e00 = (int *)0x009B8E00;
static int *const g_009b8e04 = (int *)0x009B8E04;
static int *const g_009b8e08 = (int *)0x009B8E08;
static int *const g_009b8e0c = (int *)0x009B8E0C;
static int *const g_009b8e10 = (int *)0x009B8E10;
static int *const g_009b8e14 = (int *)0x009B8E14;
static int *const g_009b8e1c = (int *)0x009B8E1C;
static int *const g_009b8e2c = (int *)0x009B8E2C;
static int *const g_009b8e30 = (int *)0x009B8E30;
static int *const g_009b8ecc = (int *)0x009B8ECC;
static int *const g_009b8ed0 = (int *)0x009B8ED0;
static int *const g_009b8ed4 = (int *)0x009B8ED4;
static int *const g_009b8ed8 = (int *)0x009B8ED8;
static int *const g_009b8ee0 = (int *)0x009B8EE0;
static int *const g_009b8fc4 = (int *)0x009B8FC4;
static int *const g_009b90b8 = (int *)0x009B90B8;
static int *const g_009b90bc = (int *)0x009B90BC;

// The catalogued class members (BasePop/Dialog/BaseButton) mangle with no
// `this`: every call site below pushes exactly the explicit-parameter count
// and nothing more (verified against 0x005889C0/popups_medium, which shares
// three of these callees byte-for-byte). Declared as plain __cdecl
// functions so the emitted call matches what the disassembly actually shows.
int __cdecl fn_006048c0(Font *, Font *, Font *, Font *); // set_def_string_font
int __cdecl fn_00609d20(Font *, Font *, Font *);         // set_def_dialog_font
int __cdecl fn_006049c0(Font *, Font *, Font *);         // set_def_button_font
void __cdecl fn_006049f0(int, int, int, int);            // set_def_button_color
void __cdecl fn_00604a20(int, int, int, int);            // set_def_button_color2
void __cdecl fn_00604a50(int, int, int, int);            // set_def_button_color3
void __cdecl fn_00604900(int, int, int, int);            // set_def_string_color
void __cdecl fn_00604930(int, int, int, int);            // set_def_string_color2
void __cdecl fn_00604960(int, int, int, int);            // set_def_string_color3
void __cdecl fn_00604990(int, int, int, int);            // set_def_string_color_hyper
void __cdecl fn_00607420(int, int, int, int);            // set_def_text_color (BaseButton)
int __cdecl fn_006074b0(Font *, Font *, Font *);          // set_def_font (BaseButton)
void __cdecl fn_00609d50(int, int, int, int, int);        // set_def_dialog_text_color (Dialog)

void __cdecl popups_normal() {
    Font *f1 = *reinterpret_cast<Font **>(g_00945760);
    Font *f2 = *reinterpret_cast<Font **>(g_0094575c);
    Font *f3 = *reinterpret_cast<Font **>(g_009403d8);
    *g_006a721c = 0;
    *g_009b8dcc = 0x1000080;
    *g_009b8d88 = 0x1a;
    *g_009b8d8c = 0x3a;
    *g_009b8d90 = 0x32;
    *g_009b8d94 = 3;
    fn_006048c0(f3, f2, f1, 0);

    f1 = *reinterpret_cast<Font **>(g_00945760);
    f2 = *reinterpret_cast<Font **>(g_0094575c);
    f3 = *reinterpret_cast<Font **>(g_009403d8);
    fn_00609d20(f3, f2, f1);

    f1 = *reinterpret_cast<Font **>(g_00945808);
    f2 = *reinterpret_cast<Font **>(g_0094580c);
    f3 = *reinterpret_cast<Font **>(g_0094576c);
    fn_006049c0(f3, f2, f1);

    int32_t v_a0 = *g_0068a5a0;
    int32_t v_a4 = *g_0068a5a4;
    int32_t v_ac = *g_0068a5ac;
    *g_00696d50 = v_a0;
    uint8_t v_b8_lo = *reinterpret_cast<uint8_t *>(g_0068a5b8);
    *g_00696ed0 = 0;
    *g_00696ed4 = 4;
    *g_00696d34 = 0;
    *g_009b7afc = v_ac;
    *g_009b7af8 = v_a4 & 0xff;
    *g_00696d3c = 0;
    *g_00696d40 = 1;
    *g_00696d44 = 1;
    *reinterpret_cast<uint8_t *>(g_00696f54) = v_b8_lo;
    fn_006049f0(0, -1, 1, 1);
    fn_00604a20(0, -1, 1, 1);
    fn_00604a50(0, -1, 1, 1);

    int32_t v_a4_2 = *g_0068a5a4;
    fn_00604900(v_a4_2, -1, 1, 1);
    int32_t v_a4_3 = *g_0068a5a4;
    fn_00604930(v_a4_3, -1, 1, 1);
    int32_t v_a4_4 = *g_0068a5a4;
    fn_00604960(v_a4_4, -1, 1, 1);
    fn_00604990(2, -1, 1, 1);

    int32_t v_b8_1 = *g_0068a5b8;
    int32_t v_bc_1 = *g_0068a5bc;
    *g_009b8db8 = v_b8_1;
    *g_009b7b8c = v_b8_1;
    int32_t v_a4_5 = *g_0068a5a4;
    int32_t v_c0_1 = *g_0068a5c0;
    *g_009b7b90 = v_a4_5;
    int32_t v_c4 = *g_0068a5c4;
    *g_00696e38 = v_c4;
    int32_t v_779644 = *g_00779644;
    *g_00696d4c = v_779644;
    int32_t v_75ade8 = *g_0075ade8;
    *g_00696d38 = v_75ade8;
    int32_t v_759e20 = *g_00759e20;
    *g_00696ee0 = 0;
    *g_00696f58 = v_bc_1;
    *g_00696f5c = v_c0_1;
    *g_009b8db4 = 0;
    *g_00696e30 = 0;
    *g_00696e34 = 0;
    *g_00696d48 = v_759e20;
    *g_00697058 = 2;
    *g_00697050 = v_bc_1;
    *g_00697054 = v_c0_1;
    fn_00607420(0, -1, 1, 1);

    int32_t v_b8_2 = *g_0068a5b8;
    *g_0069704c = v_b8_2;
    *g_009b8e2c = 0;
    fn_006074b0(0, 0, 0);

    int32_t v_a8 = *g_0068a5a8;
    int32_t v_8c6bfc = *g_008c6bfc;
    int32_t v_8c6bf4 = *g_008c6bf4;
    *g_009b8e30 = v_a8;
    int32_t v_8c6c04 = *g_008c6c04;
    *g_009b7b60 = v_8c6bfc;
    int32_t v_8c6c14 = *g_008c6c14;
    *g_00696e08 = v_8c6bf4;
    int32_t v_8c6c0c = *g_008c6c0c;
    *g_009b7b64 = v_8c6c04;
    int32_t v_9457ec = *g_009457ec;
    *g_00696e10 = v_8c6c14;
    int32_t v_8c6c1c = *g_008c6c1c;
    *g_00696e0c = v_8c6c0c;
    *g_00696e14 = v_8c6c1c;
    *g_00696e18 = v_8c6c1c;
    *g_00696e1c = 1;
    *g_009b7b68 = v_9457ec;
    *g_006970a8 = 0;
    fn_00609d50(0xe0, -1, 1, 1, 0);

    int32_t v_bc_2 = *g_0068a5bc;
    int32_t v_c0_2 = *g_0068a5c0;
    *g_009b8ecc = v_bc_2;
    *g_009b8ed0 = v_c0_2;
    *g_006970e0 = v_bc_2;
    int32_t v_b8_3 = *g_0068a5b8;
    *g_009b8ee0 = v_c0_2;
    int32_t v_41 = 0x41;
    *g_009b8ed4 = 1;
    *g_009b8ed8 = -1;
    *g_006970ec = v_b8_3;
    *g_006970e8 = v_41;
    *g_006971f4 = v_b8_3;
    int32_t v_a0_2 = *g_0068a5a0;
    int32_t v_ac_2 = *g_0068a5ac;
    *g_006971f8 = 0;
    *g_00697040 = 0;
    *g_0069702c = -1;
    *g_0069703c = v_41;
    *g_00697034 = v_41;
    *g_00697038 = v_41;
    *g_009b8df4 = reinterpret_cast<int32_t>(g_00791d88);
    *g_009b8df8 = reinterpret_cast<int32_t>(g_00791db4);
    *g_009b8dfc = reinterpret_cast<int32_t>(g_00791de0);
    *g_009b8e0c = reinterpret_cast<int32_t>(g_00791e0c);
    *g_009b8e10 = reinterpret_cast<int32_t>(g_00791e38);
    *g_009b8e14 = reinterpret_cast<int32_t>(g_00791e64);
    *g_009b8de8 = reinterpret_cast<int32_t>(g_00791e90);
    *g_009b8dec = reinterpret_cast<int32_t>(g_00791ebc);
    *g_009b8df0 = reinterpret_cast<int32_t>(g_00791ee8);
    *g_009b8e00 = reinterpret_cast<int32_t>(g_00791f14);
    *g_009b8e04 = reinterpret_cast<int32_t>(g_00791f40);
    *g_009b8e08 = reinterpret_cast<int32_t>(g_00791f6c);
    *g_009b8e1c = reinterpret_cast<int32_t>(g_0079a5e8);
    *g_009b8dd4 = 0x14;
    *g_009b8fc4 = v_a0_2;
    *g_009b90b8 = v_a0_2;
    *g_009b90bc = v_ac_2;
}
