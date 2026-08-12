// ORIGINAL: 0x005FE950 FILE
// name      ?UNK3@Palette@@QAEXH@Z
// size      372 bytes
// spans     0x005FE950-0x005FEAC4
// prototype void (__thiscall ?UNK3@Palette@@QAEXH@Z)(Palette* this, int)
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DE8F0 0x00616200 0x00644EF2 0x0064557F 0x00645930
// indirect  0x005FE9FD 0x005FEA47
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FE950
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe950/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@Palette@@QAEXH@Z  at 0x005FE950  (372 bytes)
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
class Palette;
struct RECT;
class Spot;
class Time;
typedef unsigned int UINT_PTR;

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
    int sync_to_palette(Palette *);
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
    ~Time();
};
extern "C" void *memcpy(void *, const void *, unsigned int);
extern "C" void free(void *);
int __cdecl fn_0064557f();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 6
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006690a8 = (int *)0x006690A8;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8180 = (int *)0x009B8180;
static int *const g_009b8184 = (int *)0x009B8184;
static int *const g_009b8188 = (int *)0x009B8188;
static int *const g_009bc494 = (int *)0x009BC494;
static int *const g_009bc4a0 = (int *)0x009BC4A0;

class Palette { public:
    void UNK3(int);
};

typedef void(__stdcall *AnimatePaletteFn)(void *, unsigned int, unsigned int, void *);

// Second shim: the scaffold's VCall declares slot006 nullary, but this call
// site pushes 5 int-sized stack arguments after the vtable dispatch, so a
// same-shaped vtable with slot006 re-typed reaches the right call.
class VCall6 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006(int, int, int, int, int);
};

void Palette::UNK3(int a1) {
    char *self = reinterpret_cast<char *>(this);

    int i = 0;
    int *rec = reinterpret_cast<int *>(self + 0x404);
    do {
        if (a1 == *rec || *rec == -1) break;
        i++;
        rec += 4;
    } while (i < 5);

    char *iVar1 = self + i * 0x10;
    if (*reinterpret_cast<int *>(iVar1 + 0x404) == a1) {
        Time *timeObj = *reinterpret_cast<Time **>(iVar1 + 0x408);
        if (timeObj != 0) {
            timeObj->~Time();
            ::operator delete(timeObj);
            *reinterpret_cast<int *>(iVar1 + 0x408) = 0;
        }

        char *recSrc = self + (i + 0x41) * 0x10;
        int *piVar2 = reinterpret_cast<int *>(recSrc);
        memcpy(self + (*reinterpret_cast<unsigned char *>(iVar1 + 0x40c)) * 4,
               *reinterpret_cast<void **>(recSrc),
               (*reinterpret_cast<unsigned char *>(iVar1 + 0x40d)) << 2);

        if (*g_009bc494 == 0) {
            if (*g_009b8178 != 0) {
                *g_009b8180 = reinterpret_cast<int>(self);
                reinterpret_cast<Buffer *>(g_009b7490)->sync_to_palette(reinterpret_cast<Palette *>(self));
                if (*g_009b8188 == 0 && *g_009b8184 != *reinterpret_cast<int *>(self + 0x400)) {
                    reinterpret_cast<AnimatePaletteFn>(*g_006690a8)(
                        reinterpret_cast<void *>(*g_009b8178), 10, 0xec, self + 0x28);
                    *g_009b8184 = *reinterpret_cast<int *>(self + 0x400);
                }
            }
        } else if (*g_009bc4a0 != 0) {
            VCall6 *vc = reinterpret_cast<VCall6 *>(*g_009bc4a0);
            vc->slot006(reinterpret_cast<int>(vc), 0, 0, 0x100, reinterpret_cast<int>(self));
        }

        if (*piVar2 != 0) {
            free(reinterpret_cast<void *>(*piVar2));
            *piVar2 = 0;
        }

        if (i < 4) {
            int j = 4 - i;
            do {
                *reinterpret_cast<int *>(iVar1 + 0x404) = *reinterpret_cast<int *>(iVar1 + 0x414);
                *reinterpret_cast<char *>(iVar1 + 0x40c) = *reinterpret_cast<char *>(iVar1 + 0x41c);
                *reinterpret_cast<char *>(iVar1 + 0x40d) = *reinterpret_cast<char *>(iVar1 + 0x41d);
                *reinterpret_cast<int *>(iVar1 + 0x408) = *reinterpret_cast<int *>(iVar1 + 0x418);
                *piVar2 = *reinterpret_cast<int *>(self + (i + 0x42) * 0x10);
            } while (--j);
        }
    }
}
