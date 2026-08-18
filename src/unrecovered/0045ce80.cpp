// ORIGINAL: 0x0045CE80 ?on_iface_right_click@MainInterface@@QAEXHH@Z 0x0045CE80-0x0045CF38 FILE BYTE_EXACT
// LEVER: g_008c5568/g_007ad2a0 used directly as the "this" pointer for a member call (plain const-pointer global, not indexed) reproduces `mov ecx, imm`.
// working copy - scaffold materialised by --work
// size      184 bytes
// prototype void (__thiscall ?on_iface_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00458D40 0x004B9010 0x005FA7E0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_iface_right_click@MainInterface@@QAEXHH@Z  at 0x0045CE80  (184 bytes)
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

class InfoWin;
class MainInterface;
struct RECT;
class StatusWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class InfoWin { public:
    void on_right_click(int, int);
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class StatusWin { public:
    void on_right_click(int, int, int);
};

int in_box(int, int, RECT *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_007ad2a0 = (int *)0x007AD2A0;
static int *const g_007af4ec = (int *)0x007AF4EC;
static int *const g_007af4fc = (int *)0x007AF4FC;
static int *const g_007af50c = (int *)0x007AF50C;
static int *const g_008c5568 = (int *)0x008C5568;

class MainInterface { public:
    void on_iface_right_click(int, int);
};
void MainInterface::on_iface_right_click(int a1, int a2) {
    if (in_box(a1, a2, reinterpret_cast<RECT *>(g_007af4fc))) {
        reinterpret_cast<StatusWin *>(g_008c5568)->on_right_click(a1, a2, 0);
        return;
    }
    if (in_box(a1, a2, reinterpret_cast<RECT *>(g_007af50c))) {
        reinterpret_cast<StatusWin *>(g_008c5568)->on_right_click(a1, a2, 1);
        return;
    }
    if (in_box(a1, a2, reinterpret_cast<RECT *>(g_007af4ec))) {
        reinterpret_cast<StatusWin *>(g_008c5568)->on_right_click(a1, a2, 0);
        return;
    }
    if (in_box(a1, a2, reinterpret_cast<RECT *>(g_007ad2a0))) {
        char *self = reinterpret_cast<char *>(this);
        if (*reinterpret_cast<int *>(self + 0x1dee8) == 0x3e9) {
            reinterpret_cast<InfoWin *>(g_007ad2a0)->on_right_click(a1, a2);
        }
    }
}
