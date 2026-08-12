// ORIGINAL: 0x0050F170 FILE
// name      ?go_timer@@YAXH@Z
// size      596 bytes
// spans     0x0050F170-0x0050F3C4
// prototype 
// callers   1   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050EF50 0x005C0CB0 0x005C1A20 0x005D5250 0x005EC740 0x005EC8A0 0x005ED9D0 0x00616780
// indirect  0x0050F1C0 0x0050F3AE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0050F170
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0050f170/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?go_timer@@YAXH@Z  at 0x0050F170  (596 bytes)
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
class Buffer;
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
    void fill(int);
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
    void close();
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
    int set_cursor(Sprite *, int, int);
    void get_mouse_pos(int *, int *);
    void show(int);
};
int __cdecl veh_find(int, int, int, int);
int __cdecl veh_selectable(int);
void __cdecl go_reset();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 17
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
    virtual void slot017();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669330 = (int *)0x00669330;
static int *const g_0068a5cc = (int *)0x0068A5CC;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_00915620 = (int *)0x00915620;
static int *const g_00937118 = (int *)0x00937118;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_009392b8 = (int *)0x009392B8;
static int *const g_009392c0 = (int *)0x009392C0;
static int *const g_00939340 = (int *)0x00939340;
static int *const g_009393c0 = (int *)0x009393C0;
static int *const g_009393cc = (int *)0x009393CC;
static int *const g_00939e60 = (int *)0x00939E60;
static int *const g_0093aa10 = (int *)0x0093AA10;
static int *const g_0093aa40 = (int *)0x0093AA40;
static int *const g_0093aa70 = (int *)0x0093AA70;
static int *const g_009b7abc = (int *)0x009B7ABC;
static int *const g_009b7ac0 = (int *)0x009B7AC0;

// EXTERN-SYMBOL LEVER: every table below is reached through a
// register-scaled index (a veh id times a 0x34-byte row stride, or an
// index loaded from another global), not a plain load.
extern int g_007d3c3c_sym[];
extern int g_009392c0_sym[];
extern int g_00939340_sym[];
extern int g_0093aa10_sym[];
extern int g_0093aa40_sym[];
extern uint8_t g_0093aa70_sym[];
extern int16_t g_00952828_sym[];
extern uint8_t g_00952839_sym[];
extern int g_0095282c_sym[];

extern "C" short __stdcall GetAsyncKeyState(int);

// The `call [0x44]` at the end of the routine goes through an object
// reached by a virtual-base adjustment (this = *(*obj+4) + obj), not a
// declared Win member, so it gets its own vtable shim.
class VCallY { public:
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
    virtual void slot017(int, int);
};

void __cdecl go_timer(int a1) {
    reinterpret_cast<Time *>(g_00939e60)->close();
    if (*g_0068a5cc < 0) return;
    if (*g_00915620 != 0) return;

    int *rec = &g_007d3c3c_sym[*g_0068a5cc];
    int *savedRec = rec;

    char *vbase = *reinterpret_cast<char **>(rec);
    char *adjThis = vbase + *reinterpret_cast<int *>(*reinterpret_cast<int *>(vbase) + 4);
    int mouseY, mouseX;
    reinterpret_cast<Win *>(adjThis)->get_mouse_pos(&mouseY, &mouseX);

    short keyState = (*reinterpret_cast<short(__stdcall **)(int)>(g_00669330))(0x11);

    int mode = *g_0093928c;
    int row = *g_0093928c;

    if (static_cast<uint8_t>(keyState >> 8) == 0 || *g_00939288 != 0 || *g_009393cc != 0) {
        mode = (*g_009393cc != 0) ? 9 : 1;
    }
    int newVehId = *g_0093928c;

    if ((*reinterpret_cast<unsigned int *>(*rec + 0x1dd70) & 0x40000) == 0) {
        bool needSelect =
            (mode == 1 && *g_00939288 == 0 && *g_009393cc == 0 &&
             ((*g_0093928c < 0 ||
               (row = *g_0093928c * 0x34, g_00952828_sym[*g_0093928c * 26 + 0] != g_009392c0_sym[*g_009392b8]) ||
               g_00952828_sym[*g_0093928c * 26 + 1] != g_00939340_sym[*g_009392b8] ||
               veh_selectable(*g_0093928c) == 0 || g_00952839_sym[row] != 0) ||
              (rec = savedRec, newVehId = *g_0093928c, (g_0095282c_sym[mode * 13] & 0x4200) != 0)));

        if (needSelect) {
            newVehId = veh_find(g_009392c0_sym[*g_009392b8], g_00939340_sym[*g_009392b8], *g_00939284, -1);
            if (newVehId < 0 || veh_selectable(newVehId) == 0 || g_00952839_sym[newVehId * 0x34] != 0 ||
                (g_0095282c_sym[newVehId * 13] & 0x200) != 0 ||
                (rec = savedRec, (g_0095282c_sym[newVehId * 13] & 0x4000) == 0)) {
                go_reset();
                return;
            }
        }
    } else {
        mode = 9;
    }

    *g_0093928c = newVehId;
    int owner = *rec;
    uint8_t *cursorEntry = &g_0093aa70_sym[mode * 0x2c];

    if (*reinterpret_cast<uint8_t **>(owner + 0x1ef4c) != cursorEntry) {
        char *ovbase = *reinterpret_cast<char **>(owner);
        char *oAdjThis = ovbase + *reinterpret_cast<int *>(*reinterpret_cast<int *>(ovbase) + 4);
        reinterpret_cast<Win *>(oAdjThis)->set_cursor(
            reinterpret_cast<Sprite *>(cursorEntry), g_0093aa10_sym[mode], g_0093aa40_sym[mode]);
        *reinterpret_cast<uint8_t **>(owner + 0x1ef4c) = cursorEntry;
    }

    if (*g_009b7abc == 0) {
        *g_009b7ac0 = 0;
        *g_009b7abc = *reinterpret_cast<int *>(*reinterpret_cast<int *>(owner) + 4) + owner;
    }

    *g_009393c0 = mode;
    reinterpret_cast<GraphicWin *>(g_00937118)->fill(9);
    reinterpret_cast<Win *>(g_00937118)->show(0);
    *reinterpret_cast<int *>(owner + 0x1dd80) = 1;

    int flag = 1;
    if (*g_00939288 != 0 || *g_009393cc == 0) {
        flag = 0;
    }
    *reinterpret_cast<int *>(owner + 0x1dd84) = flag;

    char *fvbase = *reinterpret_cast<char **>(owner);
    char *fAdjThis = fvbase + *reinterpret_cast<int *>(*reinterpret_cast<int *>(fvbase) + 4);
    reinterpret_cast<VCallY *>(fAdjThis)->slot017(mouseY, mouseX);
}
