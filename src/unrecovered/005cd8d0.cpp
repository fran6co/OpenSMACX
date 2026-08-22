// ORIGINAL: 0x005CD8D0 sub_5cd8d0 0x005CD8D0-0x005CD97B FILE
// TRIED: the given `__stdcall(int,int,int)` head - disasm reads edx with no prior init (a real 2nd register param) and `push ecx` at entry is pure stack reservation, not `this` preservation; rewrote as `__fastcall(int*,int,int,int,uint)` matching Ghidra's 5-param read, with differently-named cdecl externs for the two nullary-scaffolded callees. MISMATCH #4 push/mov, prologue register shape only.
// working copy - scaffold materialised by --work
// size      171 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D1466 0x005D1842

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cd8d0  at 0x005CD8D0  (171 bytes)
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
extern "C" int __cdecl sub_5d1466();
extern "C" int __cdecl sub_5d1842();

extern "C" int __cdecl real_5d1466(int, unsigned int, int *);
extern "C" int __cdecl real_5d1842(int, unsigned int, int *);

extern "C" void __fastcall sub_5cd8d0(int *param_1, int param_2, int param_3, int param_4, unsigned int param_5) {
    int iVar1 = *param_1;
    param_1[0x140] = 0;
    param_1[0x13f] = 0;
    param_1[0x13e] = 0;
    param_1[0x17e] = param_2;
    param_1[0x17f] = 0;
    unsigned int local_4 = 0;
    if (*(unsigned int *)(iVar1 + 0x60) != 0) {
        do {
            unsigned int uVar2 = 0;
            int iVar3 = param_3;
            if (*(unsigned int *)(iVar1 + 100) != 0) {
                do {
                    if (param_1[0x10] == 0) {
                        real_5d1842(iVar3, param_5, param_1 + 0xfe);
                    } else {
                        real_5d1466(iVar3, param_5, param_1 + 0xfe);
                    }
                    uVar2 = uVar2 + 0x10;
                    iVar3 = iVar3 + 0x20;
                } while (uVar2 < *(unsigned int *)(*param_1 + 100));
            }
            param_3 = param_3 + (param_5 >> 1) * 0x20;
            iVar1 = *param_1;
            local_4 = local_4 + 0x10;
        } while (local_4 < *(unsigned int *)(iVar1 + 0x60));
    }
}
