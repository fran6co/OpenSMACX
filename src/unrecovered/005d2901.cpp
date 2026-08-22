// ORIGINAL: 0x005D2901 sub_5d2901 0x005D2901-0x005D29E1 FILE
// TRIED: extern int name[] for the six indexed lookup tables (same tables as the sibling sub_5d2834 at 0x005D2834), not the given `static int *const` - these are used as `table[idx]`, an indexed base. Signature widened to the IDA-guessed 4 params (int*,int*,int*,int) since the given head has none.
// TRIED: could not reproduce the 6-register push prologue (push ebx,ecx,edx,esi,edi,ebp - ecx/edx pushed as scratch stack slots, not preservation); same wall the sibling function at 0x005D2834 hit. Hoisting the shared table_7f2c+table_772c subexpression into named locals `a`/`b` (to force it live across both blend computations, matching the original's edx reuse) made similarity slightly worse (0.61 -> 0.60), not better.
// working copy - scaffold materialised by --work
// size      224 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2901  at 0x005D2901  (224 bytes)
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
static int *const g_009c6f2c = (int *)0x009C6F2C;
static int *const g_009c772c = (int *)0x009C772C;
static int *const g_009c7f2c = (int *)0x009C7F2C;
static int *const g_009c872c = (int *)0x009C872C;
static int *const g_009c8bf4 = (int *)0x009C8BF4;
static int *const g_009c900c = (int *)0x009C900C;
extern int g_9c6f2c_arr[];
extern int g_9c772c_arr[];
extern int g_9c7f2c_arr[];
extern int g_9c872c_arr[];
extern int g_9c8bf4_arr[];
extern int g_9c900c_arr[];

extern "C" int __cdecl sub_5d2901(int *param_1, int *param_2, int *param_3, int param_4) {
    unsigned int uVar3;
    unsigned int uVar4;
    unsigned int uVar5;

    do {
        int iVar1 = *param_1;
        int iVar2 = *param_2;
        param_1 = param_1 + 1;
        uVar3 = (unsigned int)(iVar1 + iVar2) >> 1;
        param_2 = param_2 + 1;
        uVar4 = uVar3 & 0x7f7f7f7f;

        uVar5 = g_9c7f2c_arr[uVar4 & 0xff] + g_9c772c_arr[(uVar4 >> 8) & 0xff] +
                g_9c6f2c_arr[uVar4 >> 0x18];
        uVar3 = g_9c7f2c_arr[uVar4 & 0xff] + g_9c772c_arr[(uVar4 >> 8) & 0xff] +
                g_9c6f2c_arr[(uVar4 >> 0x10) & 0xff];

        *param_3 = g_9c900c_arr[uVar5 & 0x1ff] + g_9c872c_arr[uVar5 >> 0x17] +
                   g_9c8bf4_arr[(uVar5 >> 0xb) & 0x1ff];
        param_4 = param_4 - 1;
        param_3[1] = g_9c8bf4_arr[(uVar3 >> 0xb) & 0x1ff] +
                     g_9c872c_arr[uVar3 >> 0x17] + g_9c900c_arr[uVar3 & 0x1ff];
        param_3 = param_3 + 2;
    } while (param_4 != 0);

    return 0;
}
