// ORIGINAL: 0x004718C0 ?on_selected@MessageWin@@QAEXHH@Z 0x004718C0-0x00471D36;0x00655F60-0x00655F75 FILE
// size      1163 bytes
// prototype void (__thiscall ?on_selected@MessageWin@@QAEXHH@Z)(MessageWin* this, int, int)
// callers   1   call targets   16
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0041A920 0x0046B1F0 0x005147E0 0x005EC8A0 0x005ED240 0x005F8800 0x005F88A0 0x005F8920 0x005F89D0 0x005F9170 0x005F9F40 0x0060A020 0x0060C9B0 0x006169A0 0x0062B870 0x00644F3A
// indirect  0x00471C76 0x00471C88 0x00471CA0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004718C0
// measured tier  MISMATCH
// divergence     30
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004718c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_selected@MessageWin@@QAEXHH@Z  at 0x004718C0  (1163 bytes)
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
class BaseWin;
class Buffer;
class ButtonGroup;
class Console;
class Dialog;
struct DialogEntry;
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
class ListBox;
class MapWin;
class Menu;
class MessageWin;
typedef void * PVOID;
class PullDown;
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
class BaseWin { public:
    void zoom(int, int);
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
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void set_cursor(int, int);
};
class ListBox { public:
    uint32_t vbtable_pointer_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
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
    uint32_t graphic_vbase_adjust_;
    GraphicWin virtual_base_;
    uint32_t dialog_vbase_adjust_;
    Dialog dialog_;
    int update_changes();
    void set_selected_pos(int, int);
};
class MapWin { public:
    uint32_t vbtable_pointer_;
    void * owned_;
    uint8_t derived_tail_[0x21A6C - 0x8];
    GraphicWin virtual_base_;
    void set_center(int, int, int);
};
class PullDown { public:
    PullDown();
    int add_item(int, char *);
    int get_selected();
    int show(int, int, int (__cdecl *)());
    void init(Menu *);
    ~PullDown();
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
    void get_mouse_pos(int *, int *);
};
extern "C" int abs(int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00655f6b = (int *)0x00655F6B;
static int *const g_00675e50 = (int *)0x00675E50;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_007cd098 = (int *)0x007CD098;
static int *const g_007f685c = (int *)0x007F685C;
static int *const g_007f9f54 = (int *)0x007F9F54;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_009393d0 = (int *)0x009393D0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class MessageWin { public:
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
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    Font font_;
    uint8_t field_64_[0x4];
    uint8_t field_68_[0x36F0];

    void on_selected(int, int);
};

// Extra fixed-address tables this body needs beyond the emitted scaffold:
// a per-unit position record (word x, word y, byte owner) walked with a
// computed stride, mirrored from the disassembly at 0x00471A44-0x00471A5E.
extern short g_0097d040_tbl[];
extern short g_0097d042_tbl[];
extern unsigned char g_0097d044_tbl[];
static int *const g_007f6928 = (int *)0x007F6928;

void MessageWin::on_selected(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    PullDown pd;

    if (*g_007f9f54 != 0) {
        return;
    }

    int coordA, coordB;
    {
        void **vt = *reinterpret_cast<void ***>(g_009156b0);
        int voff = reinterpret_cast<int *>(vt)[1];
        reinterpret_cast<Win *>(reinterpret_cast<char *>(g_009156b0) + voff)->get_mouse_pos(&coordA, &coordB);
    }
    {
        void **vt = *reinterpret_cast<void ***>(g_009156b0);
        int voff = reinterpret_cast<int *>(vt)[1];
        reinterpret_cast<Win *>(reinterpret_cast<char *>(g_009156b0) + voff)->client_to_screen(&coordA, &coordB);
    }

    pd.init(0);
    pd.add_item(1, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*reinterpret_cast<int **>(g_009b90f8)) + 0x1130))));
    pd.add_item(2, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*reinterpret_cast<int **>(g_009b90f8)) + 0x1134))));
    pd.add_item(3, reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(
        *reinterpret_cast<int *>(reinterpret_cast<char *>(*reinterpret_cast<int **>(g_009b90f8)) + 0x1138))));

    pd.show(coordA, coordB, 0);

    int selected = pd.get_selected();
    int flags = (selected < 0x40)
        ? *reinterpret_cast<int *>(reinterpret_cast<char *>(&pd) + 0xa20 + selected * 20)
        : -1;
    int bit2 = flags & 2;

    if (flags & 1) {
        *g_007f9f54 = 1;
        if (a1 & 0x40000000) {
            unsigned int b = a1 & 0xbfffffff;
            if (static_cast<int>(b) >= 0) {
                int off = b * 308;
                if (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(g_0097d044_tbl) + off) ==
                    *g_00939284) {
                    short vx = *reinterpret_cast<short *>(reinterpret_cast<char *>(g_0097d040_tbl) + off);
                    short vy = *reinterpret_cast<short *>(reinterpret_cast<char *>(g_0097d042_tbl) + off);
                    reinterpret_cast<MapWin *>(g_009156b0)->set_center(vx, vy, 1);
                    if (*g_00939288 == 0) {
                        reinterpret_cast<Console *>(g_009156b0)->set_cursor(vx, vy);
                    }
                    reinterpret_cast<BaseWin *>(g_006a7628)->zoom(b, 0);
                }
            }
        } else if (!(a1 & 0x20000000)) {
            int magnitude = a1 & 0x7fffffff;
            int row = magnitude / *g_00949870;
            int col = magnitude % *g_00949870;
            if (row >= 0 && row < *g_00949874 && col >= 0 && col < *g_00949870) {
                int idx = *g_00939284;
                unsigned int packed = *reinterpret_cast<unsigned int *>(
                    reinterpret_cast<char *>(g_0096c9e0) + idx * 2099 * 4);
                bool doCenter;
                if (packed & 0x200) {
                    doCenter = true;
                } else {
                    int off2 = (*g_0068faf0 * row + (col >> 1)) * 11 * 4;
                    unsigned char testByte = *reinterpret_cast<unsigned char *>(
                        reinterpret_cast<char *>(g_0094a30c) + off2 + 4);
                    doCenter = (testByte & (1 << idx)) != 0;
                }
                if (doCenter) {
                    reinterpret_cast<MapWin *>(g_009156b0)->set_center(col, row, 1);
                    if (*g_00939288 == 0) {
                        reinterpret_cast<Console *>(g_009156b0)->set_cursor(col, row);
                    }
                }
            }
        }
    }

    if (bit2 != 0) {
        (*g_009393d0)--;

        char *p = *reinterpret_cast<char **>(self + 0x64);
        int off = *reinterpret_cast<int *>(p + 8);
        char *base = self + off;
        int n = *reinterpret_cast<int *>(base + 0x150);
        if (n <= *reinterpret_cast<int *>(base + 0x130) - 1) {
            *reinterpret_cast<char **>(base + 0x12c) = *reinterpret_cast<char **>(base + 0x128);
            if (n < 0) {
                int steps = abs(n);
                if (*reinterpret_cast<int *>(base + 0x130) >= steps) {
                    while (steps != 0) {
                        steps--;
                        *reinterpret_cast<char **>(base + 0x12c) =
                            *reinterpret_cast<char **>(*reinterpret_cast<char **>(base + 0x12c) + 0x10);
                    }
                    n = n + *reinterpret_cast<int *>(base + 0x130);
                    *reinterpret_cast<int *>(base + 0x134) = n;
                }
            } else {
                int steps = n;
                while (steps != 0) {
                    steps--;
                    *reinterpret_cast<char **>(base + 0x12c) =
                        *reinterpret_cast<char **>(*reinterpret_cast<char **>(base + 0x12c) + 0xc);
                }
                *reinterpret_cast<int *>(base + 0x134) = n;
            }
        }

        p = *reinterpret_cast<char **>(self + 0x64);
        off = *reinterpret_cast<int *>(p + 8);
        int decAmount;
        if (*reinterpret_cast<int *>(self + 0x128 + off) == 0) {
            decAmount = 0;
        } else {
            decAmount = *reinterpret_cast<int *>(
                *reinterpret_cast<int *>(*reinterpret_cast<char **>(self + off + 0x12c) + 8) + 0xc);
        }
        *reinterpret_cast<int *>(self + 0x84) -= decAmount;

        p = *reinterpret_cast<char **>(self + 0x64);
        off = *reinterpret_cast<int *>(p + 8);
        char *rec = self + 0x120 + off;
        if (*reinterpret_cast<int *>(rec + 8) != 0) {
            char *rec2 = *reinterpret_cast<char **>(rec + 0xc);
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(rec2 + 0x10) + 0xc) =
                *reinterpret_cast<int *>(rec2 + 0xc);
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(rec2 + 0xc) + 0x10) =
                *reinterpret_cast<int *>(rec2 + 0x10);
            char *rec3 = *reinterpret_cast<char **>(rec2 + 8);
            if (rec2 == *reinterpret_cast<char **>(rec + 8)) {
                *reinterpret_cast<char **>(rec + 8) = rec3;
            }
            *reinterpret_cast<char **>(rec + 0xc) = rec3;
            reinterpret_cast<VCall *>(rec)->slot000();
            if (rec3 != 0) {
                reinterpret_cast<VCall *>(rec3)->slot001();
            }
            *reinterpret_cast<char **>(rec2 + 8) = 0;
            reinterpret_cast<VCall *>(rec2)->slot001();
            int cnt = *reinterpret_cast<int *>(rec + 0x10) - 1;
            *reinterpret_cast<int *>(rec + 0x10) = cnt;
            if (cnt == 0) {
                *reinterpret_cast<char **>(rec + 8) = 0;
            }
            if (cnt - 1 < *reinterpret_cast<int *>(rec + 0x14)) {
                *reinterpret_cast<int *>(rec + 0x14) = cnt - 1;
            }
        }

        reinterpret_cast<ListBox *>(g_007f685c)->set_selected_pos(-1, 1);
        reinterpret_cast<ListBox *>(g_007f685c)->update_changes();
        if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_007f6928) +
            *reinterpret_cast<int *>(reinterpret_cast<char *>(*g_007f685c) + 8)) == 0) {
            reinterpret_cast<ButtonGroup *>(g_007cd098)->set(0x3e9, 1);
        }
    }

    reinterpret_cast<ListBox *>(self + 0x64)->set_selected_pos(-1, 1);
    *g_007f9f54 = 0;
}
