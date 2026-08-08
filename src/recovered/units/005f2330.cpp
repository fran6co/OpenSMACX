// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F2330
// name           ?OnLButtonDown@Win@@QAAXPAXJHHI@Z
// size           571 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f2330/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?OnLButtonDown@Win@@QAAXPAXJHHI@Z  at 0x005F2330  (571 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b6e48 = (int *)0x009B6E48;
static int *const g_009b7a6c = (int *)0x009B7A6C;
static int *const g_009b7a78 = (int *)0x009B7A78;
static int *const g_009b7a88 = (int *)0x009B7A88;
static int *const g_009b7a90 = (int *)0x009B7A90;
static int *const g_009b7aa4 = (int *)0x009B7AA4;
static int *const g_009b7aa8 = (int *)0x009B7AA8;
static int *const g_009b7aac = (int *)0x009B7AAC;
static int *const g_009b7ab0 = (int *)0x009B7AB0;
static int *const g_009b7ab4 = (int *)0x009B7AB4;
static int *const g_009b7b30 = (int *)0x009B7B30;
static int *const g_009b7b34 = (int *)0x009B7B34;

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

    int __cdecl get_mouse_window(int *, int *);
    void __cdecl OnLButtonDown(void *, long, int, int, unsigned int);
};

// No VCall shim is emitted for this subject at all - the emitter only
// synthesises one when it recognises the pattern, and did not here. Slots
// 75 (0x12c/4, two ints, returns int) and 81 (0x144/4, five ints) are
// declared fresh, following the same nullary-then-widen convention.
class VCallW { public:
    virtual void slot00(); virtual void slot01(); virtual void slot02();
    virtual void slot03(); virtual void slot04(); virtual void slot05();
    virtual void slot06(); virtual void slot07(); virtual void slot08();
    virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual void slot15(); virtual void slot16(); virtual void slot17();
    virtual void slot18(); virtual void slot19(); virtual void slot20();
    virtual void slot21(); virtual void slot22(); virtual void slot23();
    virtual void slot24(); virtual void slot25(); virtual void slot26();
    virtual void slot27(); virtual void slot28(); virtual void slot29();
    virtual void slot30(); virtual void slot31(); virtual void slot32();
    virtual void slot33(); virtual void slot34(); virtual void slot35();
    virtual void slot36(); virtual void slot37(); virtual void slot38();
    virtual void slot39(); virtual void slot40(); virtual void slot41();
    virtual void slot42(); virtual void slot43(); virtual void slot44();
    virtual void slot45(); virtual void slot46(); virtual void slot47();
    virtual void slot48(); virtual void slot49(); virtual void slot50();
    virtual void slot51(); virtual void slot52(); virtual void slot53();
    virtual void slot54(); virtual void slot55(); virtual void slot56();
    virtual void slot57(); virtual void slot58(); virtual void slot59();
    virtual void slot60(); virtual void slot61(); virtual void slot62();
    virtual void slot63(); virtual void slot64(); virtual void slot65();
    virtual void slot66(); virtual void slot67(); virtual void slot68();
    virtual void slot69(); virtual void slot70(); virtual void slot71();
    virtual void slot72(); virtual void slot73(); virtual void slot74();
    virtual int  slot75(int, int);          // <-- used, offset 0x12c
    virtual void slot76(); virtual void slot77(); virtual void slot78();
    virtual void slot79(); virtual void slot80();
    virtual void slot81(int, int, int, int, int);  // <-- used, offset 0x144
};

void __cdecl Win::OnLButtonDown(void * a1, long a2, int a3, int a4, unsigned int a5) {
    Win *w = reinterpret_cast<Win *>(get_mouse_window(&a3, &a4));
    int i = 0;
    if (w == 0) {
        goto tail;
    }
    if (*g_009b7aa4 == 0 && (w->iFlags_ & 0x4000000) == 0) {
        goto sendToOwner;
    }
    {
        int click = reinterpret_cast<VCallW *>(w)->slot75(a3, a4);
        if (click == 0) {
            goto sendToOwner;
        }

        Win *parent = w->win_parent_;
        int gcount;
        int idx = 0;
        if (parent != 0) {
            if ((w->iFlags_ & 0x2000000) != 0) {
                goto afterReorder;
            }
            int count = parent->child_count_;
            idx = 0;
            if (count > 0) {
                while (idx < count && parent->children_[idx] != w) {
                    ++idx;
                }
            }
            if (idx < count) {
                if (idx > 0) {
                    for (int k = idx; k > 0; --k) {
                        parent->children_[k] = parent->children_[k - 1];
                    }
                }
                parent->children_[0] = w;
            }

            *g_009b7b30 = 0;
            gcount = *g_009b7b34;
            if (gcount <= 0) {
                goto afterReorder;
            }
            {
                int matchTarget = *g_009b7a6c;
                Win **slot = reinterpret_cast<Win **>(g_009b6e48);
                i = 0;
                do {
                    if (matchTarget != 0 && matchTarget == reinterpret_cast<int>(*slot)) {
                        *g_009b7b30 = 0;
                        *g_009b7a78 = 0;
                    }
                    Win *cur = *slot;
                    if ((cur->iSomeFlag_ & 1) != 0) {
                        recurse_zorder(cur);
                        matchTarget = *g_009b7a6c;
                    }
                    gcount = *g_009b7b34;
                    ++i;
                    ++slot;
                } while (i < gcount);
            }
            goto joinTail;
        } else {
            if ((w->iFlags_ & 0x2000000) != 0) {
                goto afterReorder;
            }
            gcount = *g_009b7b34;
            idx = 0;
            if (gcount > 0) {
                Win **p = reinterpret_cast<Win **>(g_009b6e48);
                while (idx < gcount && *p != w) {
                    ++idx;
                    ++p;
                }
            }
            if (idx < gcount) {
                if (idx > 0) {
                    Win **q = reinterpret_cast<Win **>(g_009b6e48) + idx;
                    int k = idx;
                    do {
                        *q = *(q - 1);
                        --q;
                    } while (--k);
                }
                *reinterpret_cast<Win **>(g_009b6e48) = w;
            }

        afterReorder:
            *g_009b7b30 = 0;
            if (gcount <= 0) {
                goto joinTail;
            }
            {
                int matchTarget = *g_009b7a6c;
                Win **slot = reinterpret_cast<Win **>(g_009b6e48);
                i = 0;
                do {
                    if (matchTarget != 0 && matchTarget == reinterpret_cast<int>(*slot)) {
                        *g_009b7b30 = 0;
                        *g_009b7a78 = 0;
                    }
                    Win *cur = *slot;
                    if ((cur->iSomeFlag_ & 1) != 0) {
                        recurse_zorder(cur);
                        matchTarget = *g_009b7a6c;
                    }
                    gcount = *g_009b7b34;
                    ++i;
                    ++slot;
                } while (i < gcount);
            }
        }

    joinTail:
        if (click == 2 || (click > 9 && click <= 0x11)) {
            *g_009b7aac = reinterpret_cast<int>(w);
            *g_009b7aa8 = click;
            *g_009b7ab0 = a3;
            *g_009b7ab4 = a4;
            return;
        }
    }

sendToOwner:
    reinterpret_cast<VCallW *>(w)->slot81(a2, a3, a4, a5, *g_009b7aa4);

tail:
    if (*g_009b7a90 != 0) {
        reinterpret_cast<void(__cdecl *)(int, int)>(*g_009b7a90)(a3, a4);
    }
    if (*g_009b7a88 != 0) {
        reinterpret_cast<void(__cdecl *)()>(*g_009b7a88)();
    }
}
