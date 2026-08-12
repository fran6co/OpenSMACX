// ORIGINAL: 0x00605D90 FILE
// name      ?on_mouse_move@Scroll@@QAEXHH@Z
// size      1099 bytes
// spans     0x00605D90-0x006061DB
// prototype void (__thiscall ?on_mouse_move@Scroll@@QAEXHH@Z)(Scroll* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EC8A0 0x005EC960 0x005FA7E0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00605D90
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00605d90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_mouse_move@Scroll@@QAEXHH@Z  at 0x00605D90  (1099 bytes)
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
    int get_lbutton_state();
    void get_mouse_pos(int *, int *);
};

int in_box(int, int, RECT *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 49, 62
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
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
    virtual void slot049();  // <-- used
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
static int *const g_00669314 = (int *)0x00669314;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_009b7ab8 = (int *)0x009B7AB8;
static int *const g_009b7acc = (int *)0x009B7ACC;
static int *const g_009b7ad0 = (int *)0x009B7AD0;
static int *const g_009b7ad4 = (int *)0x009B7AD4;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009b8ddc = (int *)0x009B8DDC;

class Scroll { public:
    void on_mouse_move(int, int);
};
extern "C" unsigned long __stdcall timeGetTime();
extern "C" int __stdcall PostMessageA(void *, unsigned int, unsigned int, long);
typedef void (__cdecl *Callback2)(int, int);

class VCall49 { public:
    virtual void s00();
    virtual void s01();
    virtual void s02();
    virtual void s03();
    virtual void s04();
    virtual void s05();
    virtual void s06();
    virtual void s07();
    virtual void s08();
    virtual void s09();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void s17();
    virtual void s18();
    virtual void s19();
    virtual void s20();
    virtual void s21();
    virtual void s22();
    virtual void s23();
    virtual void s24();
    virtual void s25();
    virtual void s26();
    virtual void s27();
    virtual void s28();
    virtual void s29();
    virtual void s30();
    virtual void s31();
    virtual void s32();
    virtual void s33();
    virtual void s34();
    virtual void s35();
    virtual void s36();
    virtual void s37();
    virtual void s38();
    virtual void s39();
    virtual void s40();
    virtual void s41();
    virtual void s42();
    virtual void s43();
    virtual void s44();
    virtual void s45();
    virtual void s46();
    virtual void s47();
    virtual void s48();
    virtual void s49(int, int);
};

class VCall23 { public:
    virtual void t00();
    virtual void t01();
    virtual void t02();
    virtual void t03();
    virtual void t04();
    virtual void t05();
    virtual void t06();
    virtual void t07();
    virtual void t08();
    virtual void t09();
    virtual void t10();
    virtual void t11();
    virtual void t12();
    virtual void t13();
    virtual void t14();
    virtual void t15();
    virtual void t16();
    virtual void t17();
    virtual void t18();
    virtual void t19();
    virtual void t20();
    virtual void t21();
    virtual void t22();
    virtual int t23();
};

void Scroll::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0xc4) == 0)
        return;

    int active = *g_009b7acc;
    if (active == 0)
        active = *g_009b7ad0;
    if (active != reinterpret_cast<int>(this))
        return;
    if (*g_009b7ad4 == 0)
        return;

    RECT localRect;
    localRect.left = 0;
    localRect.top = 0;
    localRect.right = *reinterpret_cast<int *>(self + 0x47c) - *reinterpret_cast<int *>(self + 0x474);
    localRect.bottom = *reinterpret_cast<int *>(self + 0x480) - *reinterpret_cast<int *>(self + 0x478);
    if (in_box(a1, a2, &localRect) == 0)
        return;

    *g_009b8ddc = 0;
    int savedPos = *reinterpret_cast<int *>(self + 0xa2c);

    if (*reinterpret_cast<int *>(self + 0xa44) == 0) {
        int extent = *reinterpret_cast<int *>(self + 0xa54) - *reinterpret_cast<int *>(self + 0xa4c);
        int notchFlag = *reinterpret_cast<int *>(self + 0xa1c);
        int denom = extent * -3;
        if (notchFlag != -1)
            denom -= 2;

        int newPos;
        if (-*reinterpret_cast<int *>(self + 0x4c8) < *reinterpret_cast<int *>(self + 0x4c4)) {
            int mx = a1 - extent;
            if (notchFlag != -1)
                mx -= 1;
            int rangeMax = *reinterpret_cast<int *>(self + 0xa24);
            int rangeMin = *reinterpret_cast<int *>(self + 0xa20);
            newPos = (mx - (extent >> 1)) * (rangeMax - rangeMin) /
                     (denom + *reinterpret_cast<int *>(self + 0x4c4)) + rangeMin;
            *reinterpret_cast<int *>(self + 0xa3c) = mx;
        } else {
            int my = a2 - extent;
            if (notchFlag != -1)
                my += 1;
            int rangeMax = *reinterpret_cast<int *>(self + 0xa24);
            int rangeMin = *reinterpret_cast<int *>(self + 0xa20);
            *reinterpret_cast<int *>(self + 0xa3c) = my;
            newPos = (my - (extent >> 1)) * (rangeMax - rangeMin) /
                     (denom - *reinterpret_cast<int *>(self + 0x4c8)) + rangeMin;
        }

        int rangeMax = *reinterpret_cast<int *>(self + 0xa24);
        int rangeMin = *reinterpret_cast<int *>(self + 0xa20);
        if (newPos > rangeMax)
            newPos = rangeMax;
        if (newPos < rangeMin)
            newPos = rangeMin;
        *reinterpret_cast<int *>(self + 0xa2c) = newPos;
    } else {
        unsigned long now = timeGetTime();
        int lastTime = *reinterpret_cast<int *>(self + 0xa38);
        if (lastTime != 0) {
            if (static_cast<int>(now - lastTime) < *reinterpret_cast<int *>(self + 0xa30) * 2) {
                if (reinterpret_cast<Win *>(this)->get_lbutton_state() == 0)
                    return;
                reinterpret_cast<Win *>(this)->get_mouse_pos(&a1, &a2);
                PostMessageA(reinterpret_cast<void *>(*g_009b7b28), 0x200, 0,
                            (a2 << 16) | (a1 & 0xffff));
                return;
            }
        }
        if (lastTime == 0) {
            *reinterpret_cast<unsigned long *>(self + 0xa38) = timeGetTime();
        }

        if (-*reinterpret_cast<int *>(self + 0x4c8) < *reinterpret_cast<int *>(self + 0x4c4)) {
            if (a1 > *reinterpret_cast<int *>(self + 0xa54) &&
                *reinterpret_cast<int *>(self + 0xa44) > 0) {
                int lo = *reinterpret_cast<int *>(self + 0xa24);
                int next = *reinterpret_cast<int *>(self + 0xa2c) + *reinterpret_cast<int *>(self + 0xa40);
                *reinterpret_cast<int *>(self + 0xa2c) = next;
                if (lo < next)
                    *reinterpret_cast<int *>(self + 0xa2c) = lo;
            } else if (a1 < *reinterpret_cast<int *>(self + 0xa4c) &&
                       *reinterpret_cast<int *>(self + 0xa44) < 0) {
                int hi = *reinterpret_cast<int *>(self + 0xa20);
                int next = *reinterpret_cast<int *>(self + 0xa2c) - *reinterpret_cast<int *>(self + 0xa40);
                *reinterpret_cast<int *>(self + 0xa2c) = next;
                if (next < hi)
                    *reinterpret_cast<int *>(self + 0xa2c) = hi;
            }
        } else if (a2 > *reinterpret_cast<int *>(self + 0xa58) &&
                   *reinterpret_cast<int *>(self + 0xa44) > 0) {
            int lo = *reinterpret_cast<int *>(self + 0xa24);
            int next = *reinterpret_cast<int *>(self + 0xa2c) + *reinterpret_cast<int *>(self + 0xa40);
            *reinterpret_cast<int *>(self + 0xa2c) = next;
            if (lo < next)
                *reinterpret_cast<int *>(self + 0xa2c) = lo;
        } else if (a2 < *reinterpret_cast<int *>(self + 0xa50) &&
                   *reinterpret_cast<int *>(self + 0xa44) < 0) {
            int hi = *reinterpret_cast<int *>(self + 0xa20);
            int next = *reinterpret_cast<int *>(self + 0xa2c) - *reinterpret_cast<int *>(self + 0xa40);
            *reinterpret_cast<int *>(self + 0xa2c) = next;
            if (next < hi)
                *reinterpret_cast<int *>(self + 0xa2c) = hi;
        }

        reinterpret_cast<Win *>(this)->get_mouse_pos(&a1, &a2);
        if (reinterpret_cast<Win *>(this)->get_lbutton_state() != 0) {
            PostMessageA(reinterpret_cast<void *>(*g_009b7b28), 0x200, 0,
                        (a2 << 16) | (a1 & 0xffff));
        }
    }

    *g_009b7ab8 = *reinterpret_cast<int *>(self + 0xc4);
    int newVal = *reinterpret_cast<int *>(self + 0xa2c);
    if (savedPos != newVal) {
        int target = *reinterpret_cast<int *>(self + 0xc4);
        Callback2 cb = *reinterpret_cast<Callback2 *>(self + 0x2148);
        if (*reinterpret_cast<int *>(self + 0xa28) == 0) {
            reinterpret_cast<VCall49 *>(target)->s49(
                *reinterpret_cast<int *>(self + 0xa18), newVal);
            if (cb)
                cb(*reinterpret_cast<int *>(self + 0xa18),
                   *reinterpret_cast<int *>(self + 0xa2c));
        } else {
            reinterpret_cast<VCall49 *>(target)->s49(
                *reinterpret_cast<int *>(self + 0xa18),
                (*reinterpret_cast<int *>(self + 0xa20) - newVal) +
                    *reinterpret_cast<int *>(self + 0xa24));
            if (cb) {
                int iVar2b = (*reinterpret_cast<int *>(self + 0xa20) -
                             *reinterpret_cast<int *>(self + 0xa2c)) +
                             *reinterpret_cast<int *>(self + 0xa24);
                cb(*reinterpret_cast<int *>(self + 0xa18), iVar2b);
            }
        }
    }

    int *piVar1 = *reinterpret_cast<int **>(*reinterpret_cast<int *>(self + 0xc4) + 0x84);
    if (piVar1 != 0) {
        if (reinterpret_cast<VCall23 *>(piVar1)->t23() == 0)
            reinterpret_cast<VCall *>(piVar1)->slot007();
    }
    reinterpret_cast<VCall *>(this)->slot062();
}
