// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F2AC0
// name           ?window_line_raw@Win@@QAEXHHHHHHI@Z
// size           375 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f2ac0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?window_line_raw@Win@@QAEXHHHHHHI@Z  at 0x005F2AC0  (375 bytes)
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
class Palette;
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
class Palette { public:
    void __cdecl set_active_window(Win *);
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 17, 26
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
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669048 = (int *)0x00669048;
static int *const g_0066904c = (int *)0x0066904C;
static int *const g_00669058 = (int *)0x00669058;
static int *const g_00669060 = (int *)0x00669060;
static int *const g_006690c4 = (int *)0x006690C4;
static int *const g_0066927c = (int *)0x0066927C;
static int *const g_00669280 = (int *)0x00669280;
static int *const g_009b3ab0 = (int *)0x009B3AB0;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009b7b2c = (int *)0x009B7B2C;
static int *const g_009b8180 = (int *)0x009B8180;
static int *const g_009bc498 = (int *)0x009BC498;

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

    void window_line_raw(int, int, int, int, int, int, unsigned int);
};

struct LocalLogPen {
    unsigned int style;
    int widthX;
    int widthY;
    unsigned int color;
};

typedef int (__stdcall *IfaceGetHdcProc)(void *, int *);
typedef int (__stdcall *IfaceReleaseHdcProc)(void *, int);
typedef HDC (__stdcall *GetDCProc)(void *);
typedef int (__stdcall *ReleaseDCProc)(void *, HDC);
typedef void *(__stdcall *CreatePenIndirectProc)(const LocalLogPen *);
typedef void *(__stdcall *SelectObjectProc)(HDC, void *);
typedef int (__stdcall *MoveToExProc)(HDC, int, int, void *);
typedef int (__stdcall *LineToProc)(HDC, int, int);
typedef int (__stdcall *DeleteObjectProc)(void *);

void Win::window_line_raw(int a1, int a2, int a3, int a4, int a5, int a6, unsigned int a7) {
    if (*g_009b3ab0 != 0) {
        *g_009b3ab0 = *g_009b3ab0 + 1;
    } else {
        void *iface = reinterpret_cast<void *>(*g_009bc498);
        if (iface != 0) {
            IfaceGetHdcProc fn = (*reinterpret_cast<IfaceGetHdcProc **>(iface))[17];
            fn(iface, g_009b7b2c);
        } else {
            GetDCProc getDC = *reinterpret_cast<GetDCProc *>(g_0066927c);
            *g_009b7b2c = reinterpret_cast<int>(getDC(reinterpret_cast<void *>(*g_009b7b28)));
        }
        if (*g_009b7b2c == 0) {
            return;
        }
        *g_009b3ab0 = 1;
    }
    if (*g_009b7b2c != 0) {
        char *self = reinterpret_cast<char *>(this);
        if (*reinterpret_cast<int *>(self + 0x184) != *reinterpret_cast<int *>(*g_009b8180 + 0x400)) {
            (*reinterpret_cast<Palette **>(g_009b8180))->set_active_window(this);
            *reinterpret_cast<int *>(self + 0x184) = *reinterpret_cast<int *>(*g_009b8180 + 0x400);
        }

        LocalLogPen pen;
        pen.style = a7;
        pen.widthX = a6;
        pen.color = (a5 & 0xffff) | 0x1000000;

        CreatePenIndirectProc createPen = *reinterpret_cast<CreatePenIndirectProc *>(g_0066904c);
        void *hpen = createPen(&pen);
        if (hpen != 0) {
            SelectObjectProc selectObj = *reinterpret_cast<SelectObjectProc *>(g_00669060);
            void *oldPen = selectObj(reinterpret_cast<HDC>(*g_009b7b2c), hpen);

            MoveToExProc moveTo = *reinterpret_cast<MoveToExProc *>(g_00669048);
            moveTo(reinterpret_cast<HDC>(*g_009b7b2c), a3, a4, 0);

            LineToProc lineTo = *reinterpret_cast<LineToProc *>(g_006690c4);
            lineTo(reinterpret_cast<HDC>(*g_009b7b2c), a1, a2);

            selectObj(reinterpret_cast<HDC>(*g_009b7b2c), oldPen);

            DeleteObjectProc deleteObj = *reinterpret_cast<DeleteObjectProc *>(g_00669058);
            deleteObj(hpen);

            *g_009b3ab0 = *g_009b3ab0 - 1;
            if (*g_009b3ab0 == 0) {
                void *iface2 = reinterpret_cast<void *>(*g_009bc498);
                if (iface2 != 0) {
                    IfaceReleaseHdcProc fn2 = (*reinterpret_cast<IfaceReleaseHdcProc **>(iface2))[26];
                    fn2(iface2, *g_009b7b2c);
                    *g_009b7b2c = 0;
                    return;
                }
                ReleaseDCProc releaseDC = *reinterpret_cast<ReleaseDCProc *>(g_00669280);
                releaseDC(reinterpret_cast<void *>(*g_009b7b28), reinterpret_cast<HDC>(*g_009b7b2c));
                *g_009b7b2c = 0;
            }
        }
    }
}
