// ORIGINAL: 0x0041D500 FILE
// name      ?on_button_clicked@BaseWin@@QAEXH@Z
// size      1388 bytes
// spans     0x0041D500-0x0041DA6C
// prototype void (__thiscall ?on_button_clicked@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   31
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00412AA0 0x00415080 0x00415F70 0x00417040 0x00418DF0 0x00419A90 0x0041A010 0x0041A160 0x0041A2B0 0x0041B4F0 0x0041C7A0 0x004216F0 0x00445650 0x0045D380 0x004BDDE0 0x004E39D0 0x004EC3B0 0x00515400 0x00530D50 0x00530E50 0x005BFE90 0x005C01D0 0x005C0260 0x005D5A70 0x005F7E90 0x005FCBB0 0x006074E0 0x00607C80 0x00616730 0x006169A0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041D500
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041d500/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_clicked@BaseWin@@QAEXH@Z  at 0x0041D500  (1388 bytes)
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
class NetDaemon;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class Strings;
class SubInterface;
class Time;
class TutWin;
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

// ---- callees, declared and never defined (a definition would be inlined) ----
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
    void stop();
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
    int set_name(char *);
    void set(int);
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
    void redraw();
};
class Console { public:
    uint8_t derived_storage_[0x23D94];
    GraphicWin virtual_base_;
    void center();
};
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
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
class SubInterface { public:
    uint32_t field_0_;
    uint32_t field_4_;
    void release_iface_mode();
};
class TutWin { public:
    uint8_t unmapped_0_[0x537C];
    int32_t field_537C_;
    int32_t field_5380_;
    uint8_t unmapped_5384_[0x539C - 0x5384];
    int32_t field_539C_;
    uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
    int32_t field_53A4_;
    int32_t field_53A8_;
    int32_t field_53AC_;
    uint8_t unmapped_53B0_[0x53B8 - 0x53B0];
    int32_t field_53B8_;
    uint8_t unmapped_53BC_[0x53C4 - 0x53BC];
    int32_t field_53C4_;
    uint8_t unmapped_53C8_[0x53D4 - 0x53C8];
    int32_t field_53D4_;
    int reset();
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
extern "C" char *strcat(char *, const char *);
extern "C" int __stdcall sub_4216f0(int, int, int);
int __cdecl base_compute(int);
int __cdecl veh_at(int, int);
void __cdecl do_all_draws();
void __cdecl ok_callback();
void __cdecl set_base(int);
void __cdecl veh_demote(int);
void __cdecl veh_promote(int);

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
static int *const g_0041da6c = (int *)0x0041DA6C;
static int *const g_0066931c = (int *)0x0066931C;
static int *const g_00689370 = (int *)0x00689370;
static int *const g_006a7628 = (int *)0x006A7628;
static int *const g_006a803c = (int *)0x006A803C;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_008c6e68 = (int *)0x008C6E68;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_009156b0 = (int *)0x009156B0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0097d0b8 = (int *)0x0097D0B8;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_expenses(int);
    void draw_queue(int);
    void gov_on(int);
    void production(int, int);
    void hurry();
    int governor(int);
    void prev();
    void next();
    void rename();
    void staple_the_bastards();
    void base_editor();
    void on_button_clicked(int);
};

// Per-base flags dword, indexed by a register-scaled base id (baseId*0x4d
// dwords = baseId*0x134 bytes) - the indexed-table-base lever applies.
extern int32_t g_0097d0b8_tbl[];
// Tutorial-stage table, same shape, register-indexed by *g_007d392c.
extern int32_t g_007ae778_tbl[];
// Unit record table (same family as 0x00952832 elsewhere): a short field
// at +0x26 from the 0x00952832 base, register-scaled index.
extern int16_t g_00952858[];
extern int16_t g_00952828[];  // x, stride 0x1a shorts
extern int16_t g_0095282a[];  // y, stride 0x1a shorts

void BaseWin::on_button_clicked(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int baseId = *reinterpret_cast<int *>(self + 0x40b0c);
    int prevFlags = g_0097d0b8_tbl[baseId * 0x4d];

    set_base(baseId);
    base_compute(0);

    if (*g_0093f660 != 0 && *g_0093a938 == 0 &&
        prevFlags != g_0097d0b8_tbl[baseId * 0x4d]) {
        if (reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1) == 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
        }
    }

    reinterpret_cast<TutWin *>(g_008c6e68)->reset();

    if (g_007ae778_tbl[*g_007d392c] != 3) {
        return;
    }

    ok_callback();

    if (a1 > 0x3f1) {
        if (a1 != 0x3f2) {
            return;
        }
        int v = veh_at(*reinterpret_cast<int16_t *>(*g_0090ea30),
                        *reinterpret_cast<int16_t *>(*g_0090ea30 + 2));
        if (v < 0) {
            return;
        }
        veh_demote(v);
        reinterpret_cast<VCall *>(this)->slot062();
        return;
    }

    if (a1 == 0x3f1) {
        int v = veh_at(*reinterpret_cast<int16_t *>(*g_0090ea30),
                        *reinterpret_cast<int16_t *>(*g_0090ea30 + 2));
        if (v < 0) {
            return;
        }
        int idx = v;
        int16_t link = g_00952858[idx * 0x1a];
        while (link < 0) {
            idx = link;
            link = g_00952858[idx * 0x1a];
        }
        veh_promote(idx);
        reinterpret_cast<VCall *>(this)->slot062();
        return;
    }

    unsigned idx = static_cast<unsigned>(a1 + 2);
    if (idx > 0x17) {
        return;
    }

    switch (idx) {
    case 0: {
        int queue = *reinterpret_cast<int *>(*g_0090ea30 + 0x4c);
        queue++;
        if (queue >= 0xa) {
            queue = 9;
        }
        *reinterpret_cast<int *>(self + 0x40b2c) = queue;
        draw_queue(1);
        if (reinterpret_cast<Win *>(self + 0xa1c)->is_visible() == 0) {
            production(*reinterpret_cast<int *>(self + 0x40b2c), 0);
        }
        return;
    }
    case 1: {
        int wasVisible = reinterpret_cast<Win *>(g_006a7628)->is_visible();
        *reinterpret_cast<int *>(self + 0x40b28) = 0;
        reinterpret_cast<Time *>(self + 0x1d148)->stop();
        if (reinterpret_cast<Win *>(g_006a7628)->is_visible() != 0) {
            reinterpret_cast<SubInterface *>(g_006a803c)->release_iface_mode();
        }
        do_all_draws();
        if (wasVisible != 0 && *g_00939288 != 0 && *g_0093928c >= 0) {
            int recIdx = *g_0093928c;
            int16_t x = g_00952828[recIdx * 0x1a];
            int16_t y = g_0095282a[recIdx * 0x1a];
            if (sub_4216f0(x, y, 2) == 0) {
                reinterpret_cast<Console *>(g_009156b0)->center();
            }
        }
        return;
    }
    case 2:
        rename();
        return;
    case 3:
        production(0, 0);
        return;
    case 4:
        hurry();
        return;
    case 5: {
        int flags = *reinterpret_cast<int *>(self + 0x38);
        if ((flags & 0x80000000) != 0) {
            *reinterpret_cast<int *>(self + 0x38) = flags & 0x7fffffff;
            reinterpret_cast<VCall *>(this)->slot062();
            reinterpret_cast<GraphicWin *>(g_007ae820)->redraw();
        } else {
            gov_on(0);
        }
        return;
    }
    case 6:
        governor(0);
        return;
    case 7:
        gov_on(0x1000000);
        return;
    case 8:
        gov_on(0x2000000);
        return;
    case 9:
        gov_on(0x4000000);
        return;
    case 10:
        gov_on(0x8000000);
        return;
    case 11:
        if (*reinterpret_cast<int *>(self + 0x40afc) != 0) {
            *reinterpret_cast<int *>(self + 0x40afc) = 0;
            reinterpret_cast<VCall *>(this)->slot062();
        }
        return;
    case 12:
        if (*reinterpret_cast<int *>(self + 0x40afc) != 2) {
            *reinterpret_cast<int *>(self + 0x40afc) = 2;
            reinterpret_cast<VCall *>(this)->slot062();
        }
        return;
    case 13:
        if (*reinterpret_cast<int *>(self + 0x40afc) != 1) {
            *reinterpret_cast<int *>(self + 0x40afc) = 1;
            reinterpret_cast<VCall *>(this)->slot062();
        }
        return;
    case 14: {
        BaseButton *nameButton = reinterpret_cast<BaseButton *>(self + 0x197cc);
        *reinterpret_cast<uint8_t *>(g_009b86a0) = 0;
        bool named = *reinterpret_cast<int *>(self + 0x1a1e4) != 0;
        int textId = named
            ? *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0xa4c)
            : *reinterpret_cast<int *>(*reinterpret_cast<char **>(g_009b90f8) + 0xa50);
        strcat(reinterpret_cast<char *>(g_009b86a0),
               reinterpret_cast<char *>(reinterpret_cast<Strings *>(g_009b90d8)->get(textId)));
        (*reinterpret_cast<int (__cdecl **)(char *)>(g_0066931c))(reinterpret_cast<char *>(g_009b86a0));
        nameButton->set_name(reinterpret_cast<char *>(g_009b86a0));
        nameButton->set(named ? 0 : 1);
        draw_expenses(1);
        return;
    }
    case 15:
        prev();
        return;
    case 16:
        next();
        return;
    case 17:
        if (*g_00939284 == static_cast<int>(*reinterpret_cast<uint8_t *>(*g_0090ea30 + 4)) ||
            (*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80) != 0) {
            staple_the_bastards();
        }
        return;
    case 18:
        if (*g_00939284 == static_cast<int>(*reinterpret_cast<uint8_t *>(*g_0090ea30 + 4)) ||
            (*reinterpret_cast<uint8_t *>(g_009a64c0) & 0x80) != 0) {
            base_editor();
        }
        return;
    default:
        return;
    }
}
