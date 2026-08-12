// ORIGINAL: 0x0050EF50 FILE
// name      ?go_reset@@YAXXZ
// size      534 bytes
// spans     0x0050EF50-0x0050F166
// prototype 
// callers   7   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20 0x005EC7C0 0x005EDCD0 0x00616780
// indirect  0x0050EFF6 0x0050F014 0x0050F126 0x0050F144
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0050EF50
// measured tier  MISMATCH
// divergence     39
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0050ef50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?go_reset@@YAXXZ  at 0x0050EF50  (534 bytes)
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
class StatusWin;
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
class StatusWin { public:
    void set_title(int);
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
    int set_cursor(int);
    void hide();
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 4, 63
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();  // <-- used
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
    virtual void slot062();
    virtual void slot063();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068a5cc = (int *)0x0068A5CC;
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_007d3c5c = (int *)0x007D3C5C;
static int *const g_008c5568 = (int *)0x008C5568;
static int *const g_00937118 = (int *)0x00937118;
static int *const g_00939e60 = (int *)0x00939E60;
static int *const g_0093aa9c = (int *)0x0093AA9C;
static int *const g_0093abfc = (int *)0x0093ABFC;
static int *const g_009b7abc = (int *)0x009B7ABC;
static int *const g_009b7ac0 = (int *)0x009B7AC0;

// The two virtual calls dispatch on a pointer adjusted by a delta stored at
// vtable[1] of the raw window pointer (a vbase-style thunk this recovery
// cannot model as real C++ inheritance without the true class hierarchy) -
// so the adjustment is reproduced by hand, matching the disassembly's own
// two independent re-derivations instead of caching one local.
static inline char *adjusted(void *obj) {
    int delta = *reinterpret_cast<int *>(*reinterpret_cast<int *>(obj) + 4);
    return reinterpret_cast<char *>(obj) + delta;
}

void __cdecl go_reset() {
    reinterpret_cast<Win *>(g_00937118)->hide();
    reinterpret_cast<Time *>(g_00939e60)->close();

    int index = 0;
    int *slot = reinterpret_cast<int *>(g_007d3c3c);
    do {
        int win = *slot;
        if (win != 0 &&
            (slot == reinterpret_cast<int *>(g_007d3c3c) ||
             *reinterpret_cast<int *>(win + 0x1dd74) != 0) &&
            index != *g_0068a5cc &&
            *reinterpret_cast<int *>(win + 0x1dd80) != 0) {
            *reinterpret_cast<int *>(win + 0x1dd88) = 1;
            *reinterpret_cast<int *>(*slot + 0x1dd80) = 0;
            *reinterpret_cast<int *>(*slot + 0x1dd8c) = 0;
            *reinterpret_cast<int *>(*slot + 0x1dd84) = 0;
            *reinterpret_cast<int *>(*slot + 0x1de0c) = -1;

            char *sub = adjusted(reinterpret_cast<void *>(win));
            void *cached1 = *reinterpret_cast<void **>(g_009b7abc);
            if (cached1 == sub) {
                *reinterpret_cast<void **>(g_009b7abc) = 0;
                reinterpret_cast<VCall *>(sub)->slot004();
            }
            void *cached2 = *reinterpret_cast<void **>(g_009b7ac0);
            if (cached2 == sub) {
                *reinterpret_cast<void **>(g_009b7ac0) = 0;
            }
            reinterpret_cast<VCall *>(adjusted(reinterpret_cast<void *>(win)))->slot063();
            reinterpret_cast<StatusWin *>(g_008c5568)->set_title(0);
        }
        ++slot;
        ++index;
    } while (slot < reinterpret_cast<int *>(g_007d3c5c));

    int win0 = *reinterpret_cast<int *>(g_007d3c3c);
    if (*reinterpret_cast<int *>(win0 + 0x1ef4c) != 0 ||
        *reinterpret_cast<int *>(win0 + 0x1ef50) != 0x7f00) {
        reinterpret_cast<Win *>(adjusted(reinterpret_cast<void *>(win0)))->set_cursor(0x7f00);
        *reinterpret_cast<int *>(win0 + 0x1ef50) = 0x7f00;
        *reinterpret_cast<int *>(win0 + 0x1ef4c) = 0;
    }

    if (*g_0068a5cc >= 0) {
        int *entrySlot = reinterpret_cast<int *>(g_007d3c3c) + *g_0068a5cc;
        int win = *entrySlot;
        if ((*reinterpret_cast<void **>(win + 0x1ef4c) == g_0093aa9c ||
             *reinterpret_cast<void **>(win + 0x1ef4c) == g_0093abfc) &&
            (*reinterpret_cast<int *>(win + 0x1ef4c) != 0 ||
             *reinterpret_cast<int *>(win + 0x1ef50) != 0x7f00)) {
            reinterpret_cast<Win *>(adjusted(reinterpret_cast<void *>(win)))->set_cursor(0x7f00);
            *reinterpret_cast<int *>(win + 0x1ef50) = 0x7f00;
            *reinterpret_cast<int *>(win + 0x1ef4c) = 0;
        }
        *reinterpret_cast<int *>(*entrySlot + 0x1dd88) = 1;
        *reinterpret_cast<int *>(*entrySlot + 0x1dd80) = 0;
        *reinterpret_cast<int *>(*entrySlot + 0x1dd8c) = 0;
        *reinterpret_cast<int *>(*entrySlot + 0x1dd84) = 0;
        *reinterpret_cast<int *>(*entrySlot + 0x1de0c) = -1;

        char *sub = adjusted(reinterpret_cast<void *>(win));
        void *cached1 = *reinterpret_cast<void **>(g_009b7abc);
        if (cached1 == sub) {
            *reinterpret_cast<void **>(g_009b7abc) = 0;
            reinterpret_cast<VCall *>(sub)->slot004();
        }
        void *cached2 = *reinterpret_cast<void **>(g_009b7ac0);
        if (cached2 == sub) {
            *reinterpret_cast<void **>(g_009b7ac0) = 0;
        }
        reinterpret_cast<VCall *>(adjusted(reinterpret_cast<void *>(win)))->slot063();
        reinterpret_cast<StatusWin *>(g_008c5568)->set_title(0);
        *g_0068a5cc = -1;
    }
}
