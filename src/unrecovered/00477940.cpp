// ORIGINAL: 0x00477940 ?mon_winning_unify@@YAXHH@Z 0x00477940-0x004779C7 FILE
// working copy - scaffold materialised by --work
// size      135 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_winning_unify@@YAXHH@Z  at 0x00477940  (135 bytes)
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
int game_year(int);
void monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a64d4 = (int *)0x009A64D4;
struct MonRecord {
    int f0_;
    int f4_;
    int f8_;
    int fC_;
    int f10_;
    int f14_;
    unsigned char pad_18_[0x1c0 - 0x18];
    int f1C0_;
    unsigned char pad_1C4_[0x4f4 - 0x1c4];
};

void __cdecl mon_winning_unify(int a1, int a2) {
    int year = game_year(*g_009a64d4);
    unsigned char flag = *(reinterpret_cast<unsigned char *>(g_00946f58) + a1 * 0x59c);
    if ((flag & 0x80) == 0) {
        MonRecord *rec = reinterpret_cast<MonRecord *>(0x0094CCE0) + a1;
        if (rec->f10_ == 0) {
            rec->f0_ = year;
            rec->fC_ = a1;
            rec->f14_ = 1;
            rec->f10_ = 1;
            rec->f8_ = a2;
            bool matches = (a1 == *g_00939284);
            rec->f1C0_ = 0xd;
            if (matches) {
                monument(1);
            }
        }
    }
}
