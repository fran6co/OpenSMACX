// ORIGINAL: 0x00515F70 FILE
// name      ?show_movies@Console@@QAEXXZ
// size      1022 bytes
// spans     0x00515F70-0x00516291;0x0065D3F3-0x0065D4D0
// prototype void (__thiscall ?show_movies@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   23
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401060 0x00402970 0x00403BA0 0x004048A0 0x00404900 0x00406380 0x004066C0 0x00406910 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x006087A0 0x00608E10 0x00612A70 0x006169A0 0x00645470 0x00645550
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00515F70
// measured tier  NO_COMPILE
// refusal        u00515f70.cpp(157) : error C2079: 'spot_' uses undefined class 'Spot' u00515f70.cpp(266) : error C2079: 'heap_' uses undefined class 'Heap'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00515f70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?show_movies@Console@@QAEXXZ  at 0x00515F70  (1022 bytes)
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
class Console;
class Dialog;
struct DialogEntry;
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
class StringList;
class StringStruct;
struct StringStructEntry;
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

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
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
    void close();
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
    ~Dialog();
};
class Dialogs { public:
    int item(char *, int);
    ~Dialogs();
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
class StringList { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    void sort(int);
};
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
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
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _alloca_probe();
extern "C" int __cdecl fn_00402970();
extern "C" int __cdecl sub_4066c0();
void __cdecl amovie_project(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065d4c6 = (int *)0x0065D4C6;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006695c0 = (int *)0x006695C0;
static int *const g_006695c8 = (int *)0x006695C8;
static int *const g_0066974c = (int *)0x0066974C;
static int *const g_00669754 = (int *)0x00669754;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_006698cc = (int *)0x006698CC;
static int *const g_006698d4 = (int *)0x006698D4;
static int *const g_00669d50 = (int *)0x00669D50;
static int *const g_00669d58 = (int *)0x00669D58;
static int *const g_0067ba20 = (int *)0x0067BA20;
static int *const g_0068a954 = (int *)0x0068A954;
static int *const g_009a5888 = (int *)0x009A5888;
static int *const g_009a6488 = (int *)0x009A6488;
static int *const g_009b3374 = (int *)0x009B3374;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b8aa8 = (int *)0x009B8AA8;
static int *const g_009b90d8 = (int *)0x009B90D8;

class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;

    void show_movies();
};

// This method assembles and tears down a large "movie picker" overlay
// (Popup - real ctor call only; Popup's own external constructor builds
// all of its internal widgets) on every call. The internal layout of
// Popup (which widgets live at which offset - Scroll, two FlatButtons, a
// Spot, a Dialogs+embedded GraphicWin, a Dialog, a StringStruct, a Sprite,
// two more FlatButtons, a Heap, going by the ebp-relative "close" call
// sites in the disassembly) was not reconstructed field-by-field within
// this pass's time box the way 0x004921E0's FlatButton run or 0x005ADBF0's
// MapWin virtual base were: the offsets below are the raw ebp deltas from
// the disassembly, not sizes derived and cross-checked the way those two
// were. This is a faithful best-effort translation of the call sequence,
// arguments and order - not a byte-exact one. MISMATCH at instruction #2
// (first differing mnemonic 'mov' vs 'push') confirms the frame layout
// diverges immediately, as expected without the real Popup field widths.
void Console::show_movies() {
    Popup popup;
    char *base = reinterpret_cast<char *>(&popup);
    int result;

    do {
        popup.start((char *)0x9b8aa8, (const char *)0x68a954, -1, (char *)0, 0x42, (GraphicWin *)0);

        char *slot = reinterpret_cast<char *>(g_009a5888);
        for (int i = 0; slot < reinterpret_cast<char *>(g_009a6488); slot += 0x30, ++i) {
            *reinterpret_cast<char *>(g_009b86a0) = 0;
            int id = *reinterpret_cast<int *>(slot);
            const char *text = reinterpret_cast<const char *>(
                reinterpret_cast<Strings *>(g_009b90d8)->get(id));
            strcat(reinterpret_cast<char *>(g_009b86a0), text);
            reinterpret_cast<Dialogs *>(base + 0x21D0)->item(reinterpret_cast<char *>(g_009b86a0), i);
        }

        reinterpret_cast<StringList *>(base + 0x2150)->sort(0);

        result = reinterpret_cast<BasePop *>(base)->exec(0, (int (__cdecl *)())0);
        if (result < 0) {
            break;
        }
        amovie_project(result);
    } while (true);

    *reinterpret_cast<void **>(base) = g_006695c8;
    *reinterpret_cast<void **>(base + 0x444) = g_006695c0;
    popup.close();

    *reinterpret_cast<void **>(base + 0x3230) = g_00669d58;
    *reinterpret_cast<void **>(base + 0x3230 + 0x444) = g_00669d50;
    reinterpret_cast<Scroll *>(base + 0x3230)->close();

    *reinterpret_cast<void **>(base + 0x4828) = g_00669754;
    *reinterpret_cast<void **>(base + 0x4828 + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(base + 0x4828)->close();
    reinterpret_cast<BaseButton *>(base + 0x4828)->~BaseButton();

    *reinterpret_cast<void **>(base + 0x3CDC) = g_00669754;
    *reinterpret_cast<void **>(base + 0x3CDC + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(base + 0x3CDC)->close();
    reinterpret_cast<BaseButton *>(base + 0x3CDC)->~BaseButton();

    reinterpret_cast<GraphicWin *>(base + 0x3230)->~GraphicWin();

    *reinterpret_cast<void **>(base) = g_006698d4;
    *reinterpret_cast<void **>(base + 0x444) = g_006698cc;
    reinterpret_cast<BasePop *>(base)->close();

    reinterpret_cast<Spot *>(base + 0x3098)->~Spot();
    reinterpret_cast<Dialogs *>(base + 0x2358)->~Dialogs();
    reinterpret_cast<Dialog *>(base + 0x2D70)->~Dialog();
    reinterpret_cast<GraphicWin *>(base + 0x2358)->~GraphicWin();

    sub_4066c0();

    reinterpret_cast<StringStruct *>(base + 0x2150)->close();
    reinterpret_cast<Sprite *>(base + 0x2118)->close();

    *reinterpret_cast<void **>(base + 0x15A8) = g_00669754;
    *reinterpret_cast<void **>(base + 0x15A8 + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(base + 0x15A8)->close();
    reinterpret_cast<BaseButton *>(base + 0x15A8)->~BaseButton();

    *reinterpret_cast<void **>(base + 0xA5C) = g_00669754;
    *reinterpret_cast<void **>(base + 0xA5C + 0x444) = g_0066974c;
    reinterpret_cast<FlatButton *>(base + 0xA5C)->close();
    reinterpret_cast<BaseButton *>(base + 0xA5C)->~BaseButton();

    reinterpret_cast<Heap *>(base + 0xA28)->shutdown();
}
