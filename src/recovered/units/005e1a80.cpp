// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005E1A80
// name           ?hline@Buffer@@QAEXHHHH@Z
// size           368 bytes
// measured tier  MISMATCH
// divergence     75
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005e1a80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hline@Buffer@@QAEXHHHH@Z  at 0x005E1A80  (368 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;

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

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void *memset(void *, int, unsigned int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 25, 32
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
    virtual void slot025();  // <-- used
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
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

    void hline(int, int, int, int);
};

// Extra shim (kept separate from the auto-generated VCall above so the
// nullary slots there stay untouched): the object at Buffer+0x58 is a
// DirectDraw-style surface interface. slot025 is IDirectDrawSurface::Lock
// (stdcall: this, RECT*, LPDDSURFACEDESC, DWORD flags, HANDLE event),
// slot032 is Unlock (this, LPVOID). Both are called through the vtable at
// [obj], matching `call dword ptr [edx+0x64]` / `[edx+0x80]`.
class VCallLock { public:
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
    virtual int __stdcall slot025(void *, void *, int, int);  // Lock
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual int __stdcall slot032(void *);  // Unlock
};

void Buffer::hline(int x1, int x2, int y, int color) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0x54) == 0
        && *reinterpret_cast<int *>(self + 0x58) == 0) return;
    if (y < *reinterpret_cast<int *>(self + 0x24)) return;
    if (y >= *reinterpret_cast<int *>(self + 0x2c)) return;
    if (x1 == x2) return;
    if (x1 > x2) { x1 ^= x2; x2 ^= x1; x1 ^= x2; }
    if (x1 >= *reinterpret_cast<int *>(self + 0x28)) return;
    if (x2 < *reinterpret_cast<int *>(self + 0x20)) return;
    if (x1 < *reinterpret_cast<int *>(self + 0x20)) x1 = *reinterpret_cast<int *>(self + 0x20);
    if (x2 >= *reinterpret_cast<int *>(self + 0x28)) x2 = *reinterpret_cast<int *>(self + 0x28) - 1;
    if (x1 >= *reinterpret_cast<int *>(self + 0x80)) return;
    if (y >= -*reinterpret_cast<int *>(self + 0x84)) return;

    if (*reinterpret_cast<int *>(self + 0x58) == 0) {
        *reinterpret_cast<int *>(self + 0x50) = *reinterpret_cast<int *>(self + 0x54);
        if (*reinterpret_cast<int *>(self + 0x54) == 0) return;
        ++*reinterpret_cast<int *>(self + 0x6c);
    } else if (*reinterpret_cast<int *>(self + 0x50) != 0) {
        ++*reinterpret_cast<int *>(self + 0x6c);
    } else {
        uint8_t desc[0x6c];
        *reinterpret_cast<int *>(desc) = 0x6c;
        int lockObj = *reinterpret_cast<int *>(self + 0x58);
        int result = reinterpret_cast<VCallLock *>(lockObj)->slot025(0, desc, 1, 0);
        if (result != 0) return;
        ++*reinterpret_cast<int *>(self + 0x6c);
        *reinterpret_cast<int *>(self + 0x4a8) = *reinterpret_cast<int *>(desc + 0x10);
        *reinterpret_cast<int *>(self + 0x50) = *reinterpret_cast<int *>(desc + 0x24);
    }

    if (*reinterpret_cast<int *>(self + 0x50) == 0) return;

    char *dest = reinterpret_cast<char *>(*reinterpret_cast<int *>(self + 0x50))
        + *reinterpret_cast<int *>(self + 0x4a8) * y + x1;
    if (dest == 0) return;
    memset(dest, color, x2 - x1 + 1);

    if (*reinterpret_cast<int *>(self + 0x58) == 0) {
        if (--*reinterpret_cast<int *>(self + 0x6c) > 0) return;
        *reinterpret_cast<int *>(self + 0x50) = 0;
        *reinterpret_cast<int *>(self + 0x6c) = 0;
    } else {
        int n = --*reinterpret_cast<int *>(self + 0x6c);
        if (*reinterpret_cast<int *>(self + 0x50) == 0) return;
        if (n > 0) return;
        int lockObj = *reinterpret_cast<int *>(self + 0x58);
        reinterpret_cast<VCallLock *>(lockObj)->slot032(
            reinterpret_cast<void *>(*reinterpret_cast<int *>(self + 0x50)));
        *reinterpret_cast<int *>(self + 0x50) = 0;
        *reinterpret_cast<int *>(self + 0x6c) = 0;
    }
}
