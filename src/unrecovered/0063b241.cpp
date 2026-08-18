// ORIGINAL: 0x0063B241 sub_63b241 0x0063B241-0x0063B2CB FILE
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63b241  at 0x0063B241  (138 bytes)
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
static int *const g_00698b70 = (int *)0x00698B70;
// Signature change: the disassembly reads 11 dwords off the stack starting
// at [esp+0x14] (past the 4 pushed callee-saved regs) and never sets eax
// before its plain `ret`, so this is void(11 ints), not int __cdecl().
extern "C" void __cdecl sub_63b241(int param_1, int param_2, int param_3, int param_4,
                                    int param_5, int param_6, int param_7, int param_8,
                                    int param_9, int param_10, int param_11) {
    if (param_5 > 0 && param_6 > 0) {
        unsigned char **watch = reinterpret_cast<unsigned char **>(g_00698b70);
        *watch = reinterpret_cast<unsigned char *>(param_1 + param_4 * param_10 + param_3);
        unsigned int uVar2 = 0;
        do {
            unsigned int uVar1 = 0;
            int iVar3 = param_5;
            unsigned char *pbVar4 = *watch;
            do {
                if (*reinterpret_cast<char *>((uVar1 >> 0x10) + param_2) != 0) {
                    *pbVar4 = *reinterpret_cast<unsigned char *>(static_cast<unsigned int>(*pbVar4) + param_11);
                }
                pbVar4 = pbVar4 + 1;
                uVar1 = uVar1 + param_7;
                iVar3 = iVar3 - 1;
            } while (iVar3 != 0);
            *watch = *watch + param_10;
            param_2 = param_2 + ((uVar2 + param_8) >> 0x10) * param_9;
            uVar2 = (uVar2 + param_8) & 0xffff;
            param_6 = param_6 - 1;
        } while (param_6 != 0);
    }
}
