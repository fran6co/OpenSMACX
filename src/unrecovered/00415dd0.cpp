// ORIGINAL: 0x00415DD0 ?focus@BaseWin@@QAEXXZ 0x00415DD0-0x00415E52 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      130 bytes
// prototype void (__thiscall ?focus@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   4   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004E39D0 0x004EC3B0 0x00530D50 0x00530E50

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?focus@BaseWin@@QAEXXZ  at 0x00415DD0  (130 bytes)
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

class BaseWin;
class NetDaemon;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int lock_base(int, int, int, int);
    void unlock_base(int);
};

int base_compute(int);
void set_base(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0097d0b8 = (int *)0x0097D0B8;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void focus();
};
void BaseWin::focus() {
    char *self = reinterpret_cast<char *>(this);
    int idx = *reinterpret_cast<int *>(self + 0x40b0c);
    int old = g_0097d0b8[idx * 0x4d];
    set_base(idx);
    base_compute(0);
    if (*g_0093f660 != 0 && *g_0093a938 == 0) {
        int idx2 = *reinterpret_cast<int *>(self + 0x40b0c);
        if (old != g_0097d0b8[idx2 * 0x4d]) {
            int r = reinterpret_cast<NetDaemon *>(g_0093cd90)->lock_base(*g_00689370, 0, -1, -1);
            if (r == 0) {
                reinterpret_cast<NetDaemon *>(g_0093cd90)->unlock_base(*g_00689370);
            }
        }
    }
}
