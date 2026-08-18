// ORIGINAL: 0x00634DB0 ?add_filter@FileBox@@QAEHPADPAD@Z 0x00634DB0-0x00634E73 FILE
// RULED-OUT: byte-exact - original has no ebp frame at all (esp-relative addressing throughout); any local variable we declare makes VC6 spill differently and changes the prologue shape. Tried both a variable-per-step version and a minimal-locals version (reusing one char** for both strcat/append steps); neither changed the prologue divergence at instruction 0.
// working copy - scaffold materialised by --work
// size      195 bytes
// prototype int (__thiscall ?add_filter@FileBox@@QAEHPADPAD@Z)(FileBox* this, int8*, int8*)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_filter@FileBox@@QAEHPADPAD@Z  at 0x00634DB0  (195 bytes)
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

class FileBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

class FileBox { public:
    int add_filter(char *, char *);
};
int FileBox::add_filter(char * a1, char * a2) {
    if (a1 != 0 && a2 != 0) {
        char *self = reinterpret_cast<char *>(this);
        char **pos = reinterpret_cast<char **>(self + 0x410);
        char *cur = *pos;
        unsigned int len1 = strlen(a1);
        unsigned int len2 = strlen(a2);
        int remain = (0x104 - static_cast<int>(len1)) - static_cast<int>(len2);
        int used = (cur - self) - 0x30c;
        if (remain < used) {
            return 4;
        }
        strcat(cur, a2);
        unsigned int l = strlen(*pos);
        char *p = *pos + l + 1;
        *pos = p;
        *p = 0;
        strcat(*pos, a1);
        l = strlen(*pos);
        p = *pos + l + 1;
        *pos = p;
        *p = 0;
        return 0;
    }
    return 3;
}
