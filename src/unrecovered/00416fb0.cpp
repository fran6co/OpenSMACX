// ORIGINAL: 0x00416FB0 FILE
// working copy - scaffold materialised by --work
// name      ?delete_queue@BaseWin@@QAEXH@Z
// size      140 bytes
// spans     0x00416FB0-0x0041703C
// prototype void (__thiscall ?delete_queue@BaseWin@@QAEXH@Z)(BaseWin* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00415080

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?delete_queue@BaseWin@@QAEXH@Z  at 0x00416FB0  (140 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090ea30 = (int *)0x0090EA30;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_queue(int);
    void delete_queue(int);
};
void BaseWin::delete_queue(int a1) {
    char *self = (char *)this;
    int *queue_count = (int *)(self + 0x40b2c);
    char *g = (char *)*g_0090ea30;
    if (a1 > *(int *)(g + 0x4c)) {
        int n = *queue_count - 1;
        *queue_count = n;
        int v = n;
        if (v <= 1) {
            v = 1;
        }
        *queue_count = v;
        return;
    }
    if (a1 < 9) {
        int off = a1 * 4 + 0x50;
        do {
            *(int *)(g + off) = *(int *)(g + off + 4);
            g = (char *)*g_0090ea30;
            off += 4;
        } while (off < 0x74);
    }
    *(int *)(g + 0x4c) -= 1;
    int cnt = *(int *)(g + 0x4c);
    if (cnt < *queue_count) {
        int v = cnt;
        if (v <= 1) {
            v = 1;
        }
        *queue_count = v;
    }
    draw_queue(1);
}
