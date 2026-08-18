// ORIGINAL: 0x004DA870 ?UNK3@DipEdit@@QAEXH@Z 0x004DA870-0x004DA8F4 FILE
// working copy - scaffold materialised by --work
// size      132 bytes
// prototype void (__thiscall ?UNK3@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0055BB30 0x0060EC10

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@DipEdit@@QAEXH@Z  at 0x004DA870  (132 bytes)
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

class CheckBox;
class DipEdit;

// ---- callees, declared and never defined (a definition would be inlined) ----
class CheckBox { public:
    int get_state_id(int);
};

void set_treaty(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0096c9f8 = (int *)0x0096C9F8;

class DipEdit { public:
    void UNK3(int);
};
void DipEdit::UNK3(int a1) {
    char *self = reinterpret_cast<char *>(this);
    CheckBox *cb = reinterpret_cast<CheckBox *>(self + 0x15a0);
    int r = cb->get_state_id(a1);
    int f1 = *reinterpret_cast<int *>(self + 0xa1c);
    int f2 = *reinterpret_cast<int *>(self + 0xa20);
    int *flag = reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (f2 + f1) * 4 + f1 * 0x20c8);
    if (r == 0) {
        if ((*flag & a1) != 0) {
            set_treaty(f1, f2, a1, 0);
        }
    } else {
        if ((*flag & a1) == 0) {
            set_treaty(f1, f2, a1, 1);
        }
    }
}
