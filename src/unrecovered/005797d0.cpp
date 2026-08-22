// ORIGINAL: 0x005797D0 ?site_at@@YAHHHH@Z 0x005797D0-0x00579832 FILE
// TRIED: single combined expression and explicit a1/a2 temp reads both match 44/46 mnemonics at 98/98 bytes; VC6 interleaves the push esi/mov esi(a2)/push edi/mov edi(a1) prologue differently (push,push,mov,mov instead) - register scheduling, not logic
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B50 0x005C4FD0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?site_at@@YAHHHH@Z  at 0x005797D0  (98 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int world_site(int, int, int);
void site_set(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
int __cdecl site_at(int a1, int a2, int a3) {
    int index = *g_0068faf0 * a2 + (a1 >> 1);
    char * record = reinterpret_cast<char *>(*g_0094a30c) + index * 0x2c;
    if ((*reinterpret_cast<unsigned int *>(record + 8) & 0x1000) != 0) {
        return 0;
    }
    unsigned int owner = static_cast<unsigned char>(record[2]) >> 4;
    if (owner == 0) {
        owner = world_site(a1, a2, a3);
        site_set(a1, a2, owner);
    }
    return owner;
}
