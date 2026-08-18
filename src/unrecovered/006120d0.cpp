// ORIGINAL: 0x006120D0 ?set_default@EditGroup@@QAEHPADH@Z 0x006120D0-0x00612144 FILE
// RULED-OUT: both `if (a1 == 0) {...null...} else-fallthrough {...set...}` and the inverted `if (a1 != 0) {...set...} else {...null...}` diverge immediately at instr #1 (original 'push ebx' first in the prologue vs rebuilt 'mov') - a register-allocation order artifact, not a control-flow one.
// working copy - scaffold materialised by --work
// size      116 bytes
// prototype int (__thiscall ?set_default@EditGroup@@QAEHPADH@Z)(EditGroup* this, int8*, int)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_default@EditGroup@@QAEHPADH@Z  at 0x006120D0  (116 bytes)
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

class EditGroup;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);
extern "C" void free(void *);
void * mem_get(int);

class EditGroup { public:
    int set_default(char *, int);
};
int EditGroup::set_default(char * a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        char *field = *reinterpret_cast<char **>(self + 0x2c + a2 * 4);
        if (field != 0) {
            *field = 0;
        }
        return 0;
    }
    char *old = *reinterpret_cast<char **>(self + 0x2c + a2 * 4);
    if (old != 0) {
        free(old);
    }
    unsigned int len = strlen(a1);
    char *buf = static_cast<char *>(mem_get(len + 1));
    *reinterpret_cast<char **>(self + 0x2c + a2 * 4) = buf;
    if (buf == 0) {
        return 4;
    }
    *buf = 0;
    strcat(*reinterpret_cast<char **>(self + 0x2c + a2 * 4), a1);
    return 0;
}
