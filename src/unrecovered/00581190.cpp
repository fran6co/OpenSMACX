// ORIGINAL: 0x00581190 FILE
// working copy - scaffold materialised by --work
// name      ?add_abil@@YAXPAHHH@Z
// size      201 bytes
// spans     0x00581190-0x00581259
// prototype void (__cdecl ?add_abil@@YAXPAHHH@Z)(int*, int, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_abil@@YAXPAHHH@Z  at 0x00581190  (201 bytes)
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
bool has_tech(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009ab550 = (int *)0x009AB550;
void __cdecl add_abil(int * a1, int a2, int a3) {
    if (*a1 != 0) {
        return;
    }
    int bit = 0;
    while ((a2 & 1) == 0) {
        a2 = a2 >> 1;
        ++bit;
    }
    short *abilTable = (short *)0x009AB550;
    if (has_tech(abilTable[bit * 0xe], a3) == 0) {
        return;
    }
    int i = 0;
    int *count = (int *)((char *)0x00946f5c + a3 * 0x167 * 4);
    if (*count > 0) {
        int *rec = (int *)((char *)0x00946f80 + a3 * 0x167 * 4);
        do {
            if (rec[-8] == 0xf && has_tech(abilTable[rec[0] * 0xe], a3) != 0 && bit == rec[0]) {
                return;
            }
            ++i;
            ++rec;
        } while (i < *count);
    }
    *a1 |= 1 << bit;
}
