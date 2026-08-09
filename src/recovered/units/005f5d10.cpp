// ORIGINAL: 0x005F5D10 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F5D10
// name           ?on_key@Win@@QAEXIJHI@Z
// size           579 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5d10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_key@Win@@QAEXIJHI@Z  at 0x005F5D10  (579 bytes)
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
class Net;
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
class Net { public:
    int start_voice(long);
    int start_voice(unsigned long);
    void stop_voice();
};
void __cdecl do_all_chars();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 7, 23, 27
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
    virtual void slot027();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006692c8 = (int *)0x006692C8;
static int *const g_0066932c = (int *)0x0066932C;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_009b7ab8 = (int *)0x009B7AB8;
static int *const g_009b7b18 = (int *)0x009B7B18;
static int *const g_009b7b28 = (int *)0x009B7B28;
static int *const g_009be608 = (int *)0x009BE608;

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

    int key_down_event(int);
    int key_up_event(int);
    void on_key(unsigned int, long, int, unsigned int);
};

// Local shadow of the Win32 MSG struct (28 bytes) - the stack allocation the
// prologue reserves (`sub esp, 0x1c`) is exactly one of these and nothing
// else.
struct MsgT {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    long pt_x;
    long pt_y;
};

typedef short(__stdcall *GetKeyStateFn)(int);
typedef int(__stdcall *PeekMessageFn)(MsgT *, void *, unsigned int, unsigned int, unsigned int);
typedef int(__stdcall *Fn1)(void *);

// Second vtable shim: slot 23 (0x5c) returns a value that is tested here
// (the emitter's `VCall` declares it void), and slot 27 (0x6c) is called
// with one int argument (the emitter's slot is nullary) - both differ from
// what the emitter's own `VCall` covers, so this is the "second shim" its
// own comment allows.
class VCallArg { public:
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03();
    virtual void s04(); virtual void s05(); virtual void s06(); virtual void s07();
    virtual void s08(); virtual void s09(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual int s23();  // <-- used
    virtual void s24(); virtual void s25(); virtual void s26();
    virtual void s27(int);  // <-- used
};

void Win::on_key(unsigned int a1, long a2, int a3, unsigned int a4) {
    char *self = reinterpret_cast<char *>(this);
    (void)a3;
    (void)a4;

    GetKeyStateFn getKeyState = *reinterpret_cast<GetKeyStateFn *>(g_0066932c);
    unsigned int key = a1;
    if (getKeyState(0x10) & 0x8000) key |= 0x10000;
    if (getKeyState(0x11) & 0x8000) key |= 0x20000;
    if (getKeyState(0x12) & 0x8000) key |= 0x40000;

    char *net = *reinterpret_cast<char **>(g_009be608);
    if (net != 0) {
        PeekMessageFn peekMessage = *reinterpret_cast<PeekMessageFn *>(g_00669358);
        MsgT msg;
        if (key == *reinterpret_cast<unsigned int *>(net + 0x48)) {
            if (a2 == 0) {
                reinterpret_cast<Net *>(net)->stop_voice();
            } else {
                reinterpret_cast<Net *>(net)->start_voice(
                    *reinterpret_cast<unsigned long *>(net + 0x50));
            }
            if (!peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1)) {
                return;
            }
            do {
            } while (peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1));
            return;
        }
        if (key == *reinterpret_cast<unsigned int *>(net + 0x4c)) {
            if (a2 == 0) {
                reinterpret_cast<Net *>(net)->stop_voice();
            } else {
                reinterpret_cast<Net *>(net)->start_voice(
                    *reinterpret_cast<long *>(net + 0x54));
            }
            if (!peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1)) {
                return;
            }
            do {
            } while (peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1));
            return;
        }
    }

    if (a2 == 0) {
        if (*reinterpret_cast<unsigned int *>(self + 0x98) & 0x200000) {
            return;
        }
        if (*reinterpret_cast<unsigned char *>(self + 0x9c) & 8) {
            return;
        }
        if (*reinterpret_cast<int *>(self + 0xd4) != 0) {
            void *target;
            if (*reinterpret_cast<int *>(self + 0xcc) != 0) {
                target = *reinterpret_cast<void **>(*reinterpret_cast<char **>(self + 0xd0) + 4);
            } else {
                target = 0;
            }
            if (reinterpret_cast<Win *>(target)->key_up_event(key) != 0) {
                return;
            }
        }

        *reinterpret_cast<void **>(g_009b7ab8) = self;
        typedef void(__cdecl * OnKeyCb)(unsigned int);
        OnKeyCb cb = *reinterpret_cast<OnKeyCb *>(self + 0x434);
        if (cb != 0) {
            cb(key);
        }
        reinterpret_cast<VCallArg *>(self)->s27(key);

        char *next = *reinterpret_cast<char **>(self + 0x40);
        if (next == 0) {
            return;
        }
        if (reinterpret_cast<VCallArg *>(next)->s23() != 0) {
            return;
        }
        reinterpret_cast<VCall *>(next)->slot007();
        return;
    }

    *reinterpret_cast<unsigned int *>(g_009b7b18) = key;
    Fn1 fn = *reinterpret_cast<Fn1 *>(g_006692c8);
    if (fn(*reinterpret_cast<void **>(g_009b7b28))) {
        reinterpret_cast<Win *>(self)->key_down_event(key);
    }

    if (key >= 0x60 && key <= 0x6f) {
        PeekMessageFn peekMessage = *reinterpret_cast<PeekMessageFn *>(g_00669358);
        MsgT msg;
        if (!peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1)) {
            return;
        }
        do {
        } while (peekMessage(&msg, *reinterpret_cast<void **>(g_009b7b28), 0x102, 0x102, 1));
        return;
    }

    do_all_chars();
}
