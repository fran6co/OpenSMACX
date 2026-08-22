// ORIGINAL: 0x005CDA30 sub_5cda30 0x005CDA30-0x005CDAC4 FILE
// TRIED: nothing forced the `push ecx`-as-spill-slot / `pop ecx` prologue trick the original uses for one local; plain locals compile 6 bytes shorter (142 vs 148), diverging early
// PROPOSAL: sub_5cda30(int,int,int) __stdcall -> sub_5cda30(int *param_1,
//           int param_2, int param_3, int param_4, unsigned int param_5)
//           __fastcall; ecx/edx are read as real incoming values (param_1
//           base pointer, param_2 stored to param_1[0x17e]) before either is
//           ever assigned, and `ret 0xc` only pops the 3 stack args - the
//           2 register args are fastcall's, not stdcall's.
// working copy - scaffold materialised by --work
// size      148 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D1278

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cda30  at 0x005CDA30  (148 bytes)
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
extern "C" int __cdecl sub_5d1278();
extern "C" int __cdecl sub_5d1278_real(int, unsigned int, int *);

extern "C" void __fastcall sub_5cda30(int *param_1, int param_2, int param_3, int param_4, unsigned int param_5) {
    int iVar2 = *param_1;
    param_1[0x17e] = param_2;
    param_1[0x17f] = 0;
    param_1[0x140] = 0;
    param_1[0x13f] = 0;
    param_1[0x13e] = 0;
    unsigned int local_4 = 0;
    if (*reinterpret_cast<int *>(iVar2 + 0x60) != 0) {
        do {
            unsigned int uVar1 = 0;
            if (*reinterpret_cast<int *>(iVar2 + 0x64) != 0) {
                iVar2 = param_3;
                do {
                    sub_5d1278_real(iVar2, param_5, param_1 + 0xfe);
                    uVar1 = uVar1 + 0x10;
                    iVar2 = iVar2 + 0x20;
                } while (uVar1 < *reinterpret_cast<unsigned int *>(*param_1 + 0x64));
            }
            iVar2 = *param_1;
            param_3 = param_3 + (param_5 >> 1) * 0x20;
            local_4 = local_4 + 0x10;
        } while (local_4 < *reinterpret_cast<unsigned int *>(iVar2 + 0x60));
    }
}
