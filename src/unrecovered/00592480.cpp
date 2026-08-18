// ORIGINAL: 0x00592480 sub_592480 0x00592480-0x00592550 FILE
// RULED-OUT: idx*0x2c direct multiply (compiler drops the lea/lea *11 decomposition); byte 0x66efbc/0x66f440 offset tables need a distinct extern (not caught by the fixed-global scanner since they're addressed as [reg+imm] with no separate mov).
// working copy - scaffold materialised by --work
// size      208 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_592480  at 0x00592480  (208 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;

extern int g_66efbc_arr[];
extern int g_66f440_arr[];

extern "C" int __cdecl sub_592480(int param_1, int param_2) {
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int recIdx;
    unsigned int *base;
    unsigned int *field;

    base = *(unsigned int **)g_0094a30c;
    iVar1 = *g_0068faf0 * param_2 + (param_1 >> 1);
    recIdx = iVar1 * 11;
    field = (unsigned int *)((char *)base + 8) + recIdx;
    *field = *field | 0x20;

    base = *(unsigned int **)g_0094a30c;
    iVar1 = *g_0068faf0 * param_2 + (param_1 >> 1);
    recIdx = iVar1 * 11;
    iVar4 = 0;
    field = (unsigned int *)((char *)base + 8) + recIdx;
    *field = *field & 0xfe1fffaf;

    iVar1 = *g_00949870;
    do {
        iVar2 = *(int *)((char *)g_66efbc_arr + iVar4) + param_1;
        if ((*(unsigned char *)g_0094988c & 1) == 0) {
            if (iVar2 < 0) {
                iVar2 = iVar2 + iVar1;
            } else if (iVar1 <= iVar2) {
                iVar2 = iVar2 - iVar1;
            }
        }
        iVar3 = *(int *)((char *)g_66f440_arr + iVar4) + param_2;
        if ((-1 < iVar3) && (iVar3 < *g_00949874) && (-1 < iVar2) && (iVar2 < iVar1)) {
            base = *(unsigned int **)g_0094a30c;
            iVar1 = *g_0068faf0 * iVar3 + (iVar2 >> 1);
            recIdx = iVar1 * 11;
            unsigned char *rec = (unsigned char *)base + recIdx * 4;
            rec[2] = rec[2] & 0xf;
            iVar1 = *g_00949870;
        }
        iVar4 = iVar4 + 4;
    } while (iVar4 < 0x54);
    return 0;
}
