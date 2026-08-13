// ORIGINAL: 0x004DA7E0 FILE
// working copy - scaffold materialised by --work
// name      ?UNK2@DipEdit@@QAEXH@Z
// size      135 bytes
// spans     0x004DA7E0-0x004DA867
// prototype void (__thiscall ?UNK2@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0055B820 0x0055BA80 0x0060EC10

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@DipEdit@@QAEXH@Z  at 0x004DA7E0  (135 bytes)
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

void agenda_off(int, int, int);
void agenda_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0096ca18 = (int *)0x0096CA18;

class DipEdit { public:
    void UNK2(int);
};
void DipEdit::UNK2(int a1) {
    char *self = (char *)this;
    CheckBox *cb = (CheckBox *)(self + 0x15a0);
    int state = cb->get_state_id(a1 | 0x80000000);
    int iVar1 = *(int *)(self + 0xa1c);
    int iVar2 = *(int *)(self + 0xa20);
    int idx = iVar2 + iVar1 * 2099;
    int *flags = g_0096ca18 + idx;
    if (state == 0) {
        if ((*flags & a1) != 0) {
            agenda_off(iVar1, iVar2, a1);
        }
    } else if ((*flags & a1) == 0) {
        agenda_on(iVar1, iVar2, a1);
    }
}
