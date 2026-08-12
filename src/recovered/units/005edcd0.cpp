// ORIGINAL: 0x005EDCD0 FILE
// name      ?hide@Win@@QAEXXZ
// size      390 bytes
// spans     0x005EDCD0-0x005EDE56
// prototype void (__thiscall ?hide@Win@@QAEXXZ)(Win* this)
// callers   34   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECE20 0x005EFD20 0x005F4EC0 0x005F7320
// indirect  0x005EDD98 0x005EDDC5 0x005EDE33 0x005EDE44 0x005EDE4F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EDCD0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005edcd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hide@Win@@QAEXXZ  at 0x005EDCD0  (390 bytes)
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
void __cdecl recurse_zorder(Win *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 4, 7, 23, 58
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();  // <-- used
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot023();  // <-- used
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
    virtual void slot058();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669304 = (int *)0x00669304;
static int *const g_009b6e38 = (int *)0x009B6E38;
static int *const g_009b6e48 = (int *)0x009B6E48;
static int *const g_009b7a4c = (int *)0x009B7A4C;
static int *const g_009b7a50 = (int *)0x009B7A50;
static int *const g_009b7a6c = (int *)0x009B7A6C;
static int *const g_009b7a78 = (int *)0x009B7A78;
static int *const g_009b7abc = (int *)0x009B7ABC;
static int *const g_009b7ac0 = (int *)0x009B7AC0;
static int *const g_009b7acc = (int *)0x009B7ACC;
static int *const g_009b7ad0 = (int *)0x009B7AD0;
static int *const g_009b7ae0 = (int *)0x009B7AE0;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009b7b30 = (int *)0x009B7B30;
static int *const g_009b7b34 = (int *)0x009B7B34;
static int *const g_009bc4b0 = (int *)0x009BC4B0;

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

    int is_descendant(Win *);
    void __cdecl flip(RECT *);
    int __cdecl update_screen(RECT *, Win *);
    void hide();
};

// Slots 4,7,23,58 need real arities/return types; the scaffold's own VCall
// declares every slot nullary void, so a local shim (declaration order only)
// carries the ones this body actually calls.
class HideVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();  // <-- used, void
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();  // <-- used, void
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
  virtual int slot023();  // <-- used, returns int (tested with `test eax,eax`)
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
  virtual void slot058();  // <-- used, void
};

typedef void(__stdcall *ThreeArgFn)(int, int, int);

void Win::hide() {
    char *const self = reinterpret_cast<char *>(this);
    int32_t zero;
    int32_t focus_a, focus_b, chosen;
    Win **child;
    int32_t i;

    if (!(iSomeFlag_ & 1)) {
        return;
    }

    if (*g_009b7a4c == reinterpret_cast<int32_t>(this)) {
        zero = 0;
        if (*g_009b7a50 != zero) {
            *g_009b7a4c = zero;
            *g_009b7a50 = zero;
            update_screen(reinterpret_cast<RECT *>(g_009b6e38), 0);
            flip(reinterpret_cast<RECT *>(g_009b6e38));
        }
        goto converge;
    }
    zero = 0;

converge:
    focus_a = *g_009b7acc;
    focus_b = *g_009b7ad0;
    chosen = (focus_a != zero) ? focus_a : focus_b;
    if (chosen == reinterpret_cast<int32_t>(this)) {
        goto clear_focus;
    }
    if (focus_a == zero) {
        if (focus_b == zero) {
            goto skip_all;
        }
        goto search;
    }
    focus_b = focus_a;

search:
    if (focus_b == zero) {
        goto skip_all;
    }
    if (child_count_ <= 0) {
        goto skip_all;
    }
    child = children_;
    i = 0;
    for (;;) {
        if (reinterpret_cast<int32_t>(*child) == focus_b) {
            goto found;
        }
        if ((*child)->is_descendant(reinterpret_cast<Win *>(focus_b))) {
            goto found;
        }
        i++;
        child++;
        if (i >= child_count_) {
            goto skip_all;
        }
    }

found:
    zero = 0;
clear_focus:
    *g_009b7acc = zero;
    *g_009b7ad0 = zero;

skip_all:
    if (*g_009b7abc == reinterpret_cast<int32_t>(this)) {
        *g_009b7abc = 0;
        reinterpret_cast<HideVCall *>(this)->slot004();
    }
    if (*g_009b7ac0 == reinterpret_cast<int32_t>(this)) {
        *g_009b7ac0 = 0;
    }
    iSomeFlag_ &= 0xfffffffeu;
    if (*g_009b7ae0 == reinterpret_cast<int32_t>(this)) {
        reinterpret_cast<HideVCall *>(this)->slot058();
    }

    if (!(*reinterpret_cast<uint8_t *>(g_009bc4b0) & 1)) {
        return;
    }

    {
        int32_t total = *g_009b7b34;
        *g_009b7b30 = 0;
        if (total > 0) {
            int32_t target = *g_009b7a6c;
            Win **p = reinterpret_cast<Win **>(g_009b6e48);
            int32_t j = 0;
            for (;;) {
                if (target != 0 && target == reinterpret_cast<int32_t>(*p)) {
                    *g_009b7b30 = 0;
                    *g_009b7a78 = 0;
                }
                if ((*p)->iSomeFlag_ & 1) {
                    recurse_zorder(*p);
                    target = *g_009b7a6c;
                }
                total = *g_009b7b34;
                j++;
                p++;
                if (j >= total) {
                    break;
                }
            }
        }
    }

    (*reinterpret_cast<ThreeArgFn *>(g_00669304))(*g_009b7b28, 0, 0);

    {
        Win *other = *reinterpret_cast<Win **>(self + 0x10);
        if (other != 0) {
            if (reinterpret_cast<HideVCall *>(other)->slot023() == 0) {
                reinterpret_cast<HideVCall *>(other)->slot007();
            }
        }
    }
}
