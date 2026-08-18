// ORIGINAL: 0x0045EF10 ??0MainInterface@@QAE@XZ 0x0045EF10-0x0045F1BE;0x00655530-0x00655723 FILE
// size      1185 bytes
// prototype void (__thiscall ??0MainInterface@@QAE@XZ)(MainInterface* this)
// callers   1   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005E37E0 0x005FA860 0x006161D0 0x00618EA0 0x00629110 0x00629D60 0x006456E4 0x006457C2
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045EF10
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045ef10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??0MainInterface@@QAE@XZ  at 0x0045EF10  (1185 bytes)
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
class Flic;
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
class MainInterface;
class Menu;
typedef void * PVOID;
struct RECT;
class Scroll;
class Spot;
class Sprite;
class StringBox;
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

// Moved ahead of `Buffer`, which embeds it BY VALUE at `spot_` below - the
// on-disk scaffold had this definition further down (after Buffer), which
// is a C2079 ('spot_' uses undefined class 'Spot') under any compiler: a
// by-value member needs a complete type in scope at the point of use, not
// merely the forward declaration above.
class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    Spot();
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

class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
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
class Flic { public:
    Flic();
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
    Font();
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
    GraphicWin();
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
    Sprite();
};
class StringBox { public:
    StringBox();
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
    Time();
};
void __stdcall fn_006456e4(void*, unsigned int, int, void (*)(void*));
void __stdcall fn_006457c2(void*, unsigned int, int, void (*)(void*), void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00406850 = (int *)0x00406850;
static int *const g_00406880 = (int *)0x00406880;
static int *const g_004325b0 = (int *)0x004325B0;
static int *const g_004327a0 = (int *)0x004327A0;
static int *const g_005e37e0 = (int *)0x005E37E0;
static int *const g_00607cf0 = (int *)0x00607CF0;
static int *const g_00629d60 = (int *)0x00629D60;
static int *const g_0062b7c0 = (int *)0x0062B7C0;
static int *const g_00655719 = (int *)0x00655719;
static int *const g_0066a6e4 = (int *)0x0066A6E4;
static int *const g_0066c544 = (int *)0x0066C544;
static int *const g_0066c590 = (int *)0x0066C590;
static int *const g_0066c598 = (int *)0x0066C598;
static int *const g_00675680 = (int *)0x00675680;

class MainInterface { public:
    uint8_t pad_0_[0x1EACC];
    StringBox string_box_;

    MainInterface();
};

// Placement new: the only way to run one of the declared sub-object
// constructors AT an existing byte offset instead of allocating storage.
inline void *__cdecl operator new(unsigned int, void *p) { return p; }

typedef void (*CtorDtorFn)(void *);

MainInterface::MainInterface() {
    // Same shape as 0x00408490/BaseWin::BaseWin: a long run of sub-object
    // constructions guarded by a compiler-synthesized SEH try-frame (the
    // push -1 / push 0x655719 / fs:[0] chain in the prologue) whose cleanup
    // path lives in the EH-funclet cluster at 0x00655530-0x00655719. A
    // call-by-call translation does not make the compiler re-derive that
    // frame, so this was never expected to score BYTE_EXACT; bodied here
    // for coverage as a faithful in-order translation of the disassembly.
    char *self = reinterpret_cast<char *>(this);

    new (self) GraphicWin();

    fn_006457c2(self + 0xe00, 0xb4c, 0x2a,
                reinterpret_cast<CtorDtorFn>(g_00607cf0),
                reinterpret_cast<CtorDtorFn>(g_00406880));
    *reinterpret_cast<int *>(self + 0xa14) = reinterpret_cast<int>(g_0066a6e4);

    fn_006457c2(self + 0x1e878, 0x94, 4,
                reinterpret_cast<CtorDtorFn>(g_0062b7c0),
                reinterpret_cast<CtorDtorFn>(g_004325b0));

    new (self + 0x1eacc) StringBox();
    new (self + 0x2166c) Font();
    new (self + 0x21694) Font();
    new (self + 0x216bc) Font();

    fn_006457c2(self + 0x216f4, 0x2c, 4,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x217a4, 0x2c, 4,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21854, 0x2c, 2,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x218ac, 0x2c, 2,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21904, 0x2c, 0xa,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21abc, 0x2c, 0xa,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21c74, 0x2c, 6,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21d7c, 0x2c, 3,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x21e00, 0x2c, 0x4b,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x22ae4, 0x2c, 0x12,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));

    new (self + 0x22dfc) Sprite();

    fn_006457c2(self + 0x22e28, 0x2c, 6,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x22f30, 0x2c, 2,
                reinterpret_cast<CtorDtorFn>(g_005e37e0),
                reinterpret_cast<CtorDtorFn>(g_00406850));
    fn_006457c2(self + 0x22f88, 0xae4, 2,
                reinterpret_cast<CtorDtorFn>(g_00629d60),
                reinterpret_cast<CtorDtorFn>(g_004327a0));

    new (self + 0x24550) Flic();
    new (self + 0x25034) Spot();
    new (self + 0x25040) Time();
    new (self + 0x25068) Time();

    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066c598);
    *reinterpret_cast<int *>(self + 0x444) = reinterpret_cast<int>(g_0066c590);
    *reinterpret_cast<int *>(self + 0xa14) = reinterpret_cast<int>(g_0066c544);
}
