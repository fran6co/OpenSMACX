// ORIGINAL: 0x005AD560 ?make_bases@ReplayWin@@QAEXXZ 0x005AD560-0x005AD607 FILE
// working copy - scaffold materialised by --work
// size      167 bytes
// prototype void (__thiscall ?make_bases@ReplayWin@@QAEXXZ)(ReplayWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00523DD0 0x00591B10 0x00591D60

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?make_bases@ReplayWin@@QAEXXZ  at 0x005AD560  (167 bytes)
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

class ReplayWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
void bit_set(int, int, int, int);
void owner_set(int, int, int);
void reset_territory();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00949884 = (int *)0x00949884;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0097d042 = (int *)0x0097D042;
static int *const g_009a64cc = (int *)0x009A64CC;

class ReplayWin { public:
    void make_bases();
};
void ReplayWin::make_bases() {
    char *self = reinterpret_cast<char *>(this);
    int *rec;
    int count;
    int i;
    short *psVar3;
    int *piVar4;
    int iVar2;

    rec = reinterpret_cast<int *>(*g_0094a30c + 8);
    count = *g_00949884;
    *g_009a64cc = 0;
    if (count > 0) {
        i = 0;
        do {
            i++;
            *rec &= 0xFFFFFFFE;
            rec += 0xb;
        } while (i < *g_00949884);
    }

    psVar3 = reinterpret_cast<short *>(g_0097d042);
    piVar4 = reinterpret_cast<int *>(self + 0xa20);
    iVar2 = 0x200;
    do {
        if (*piVar4 >= 0) {
            psVar3[-1] = (short)*piVar4;
            *psVar3 = (short)piVar4[1];
            *reinterpret_cast<unsigned char *>(psVar3 + 2) = 1;
            *reinterpret_cast<char *>(psVar3 + 1) = (char)piVar4[2];
            bit_set(psVar3[-1], *psVar3, 1, 1);
            owner_set(psVar3[-1], *psVar3, *reinterpret_cast<char *>(psVar3 + 1));
            psVar3 += 0x9a;
            (*g_009a64cc)++;
        }
        piVar4 += 3;
        iVar2--;
    } while (iVar2 != 0);

    reset_territory();
}
