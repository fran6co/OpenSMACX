// ORIGINAL: 0x004A5F40 FILE
// RULED-OUT: `check(a1, a2, &a1, &a2)` for the out-params (wrong operand
//            order); the push order decodes to check(a1,a2,&a2,&a1).
//            Explicit __thiscall function-pointer typedef for the vtable
//            slot at +0xE4 (C4234) - a member-function-pointer/union shim
//            compiles. MISMATCH #16 'inc' vs 'cmp' remains open.
// working copy - scaffold materialised by --work
// name      ?on_iface_left_click@ReportIf@@QAEXHH@Z
// size      150 bytes
// spans     0x004A5F40-0x004A5FD6
// prototype void (__thiscall ?on_iface_left_click@ReportIf@@QAEXHH@Z)(ReportIf* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004A3950 0x005FAB00
// indirect  0x004A5FB1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_left_click@ReportIf@@QAEXHH@Z  at 0x004A5F40  (150 bytes)
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

struct RECT;
class ReportIf;
class Spot;

// ---- callees, declared and never defined (a definition would be inlined) ----
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    struct SpotInternal {
        RECT rect;
        int type;
        int position;
    };
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
    int check(int, int, int *, int *);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_008846e8 = (int *)0x008846E8;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;

class ReportIf { public:
    void report(int, int, int);
    void on_iface_left_click(int, int);
};
extern int g_table_7ae778[];

class ShimE4 { public: void callE4(int, void *, void *); };
typedef void (ShimE4::*MemE4)(int, void *, void *);
union UE4 { void *raw; MemE4 mem; };

void ReportIf::on_iface_left_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int *field8 = reinterpret_cast<int *>(self + 8);

    if (*field8 != 7) {
        return;
    }

    Spot *spot = reinterpret_cast<Spot *>(self + 0x1e1bc);
    int result = spot->check(a1, a2, &a2, &a1);
    if (result == -1) {
        return;
    }

    *reinterpret_cast<int *>(self + 0x1e244) = a2;

    *g_008846e8 = 1;
    *g_00885f40 = 7;

    if (g_table_7ae778[*g_007d392c] != 5) {
        void *obj = g_00876478;
        void *vtbl = *reinterpret_cast<void **>(obj);
        UE4 u;
        u.raw = *reinterpret_cast<void **>(reinterpret_cast<char *>(vtbl) + 0xe4);
        (reinterpret_cast<ShimE4 *>(obj)->*u.mem)(0, g_005398e0, g_007ae820);
        return;
    }

    reinterpret_cast<ReportIf *>(g_00885f38)->report(7, *g_008a4160, 0);
}
