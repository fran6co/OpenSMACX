// ORIGINAL: 0x0043D490 ?center_stack@DesignWin@@QAEXHH@Z 0x0043D490-0x0043D50B FILE
// working copy - scaffold materialised by --work
// size      123 bytes
// prototype void (__thiscall ?center_stack@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0043D510 0x005A63D0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?center_stack@DesignWin@@QAEXHH@Z  at 0x0043D490  (123 bytes)
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

class DesignWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
void proto_sort_2(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00945b40 = (int *)0x00945B40;
static int *const g_00945b44 = (int *)0x00945B44;

class DesignWin { public:
    void draw_stack();
    void center_stack(int, int);
};
extern int g_945b44_arr[];

void DesignWin::center_stack(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    proto_sort_2(*(int *)(self + 0x141f4));
    int n = *g_00945b40;
    int idx = *(int *)(self + 0xce4);
    if (idx < idx + n) {
        int rem = 0;
        bool found = false;
        do {
            rem = idx % n;
            if (a1 == g_945b44_arr[rem]) {
                found = true;
                break;
            }
            idx = idx + 1;
        } while (idx < *(int *)(self + 0xce4) + n);
        if (found) {
            int old_val = *(int *)(self + 0xce4);
            int new_val = (rem + n - 5) % n;
            *(int *)(self + 0xce4) = new_val;
            if (a2 != 0 && new_val != old_val) {
                draw_stack();
            }
        }
    }
}
