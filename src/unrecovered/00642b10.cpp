// ORIGINAL: 0x00642B10 _inflate_blocks_reset 0x00642B10-0x00642BA2 FILE BYTE_EXACT
// symbol    _sub_642b10
// working copy - scaffold materialised by --work
// CORRECTED from sub_642b10
//   zlib 1.0.2 infblock.c, 146 bytes, byte-exact from upstream
// size      146 bytes
// prototype 
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644100 0x00644910
// indirect  0x00642B3E 0x00642B94

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_642b10  at 0x00642B10  (146 bytes)
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
extern "C" int __cdecl sub_644100();
extern "C" int __cdecl sub_644910();
extern "C" int __cdecl call_644910(int, int);
extern "C" int __cdecl call_644100(int, int);

extern "C" void __cdecl sub_642b10(int *param_1, int param_2, int *param_3) {
    if (param_1[0xd] != 0) {
        *param_3 = param_1[0xe];
    }
    if (param_1[0] == 4 || param_1[0] == 5) {
        typedef int (__cdecl *VtableFn)(int, int);
        VtableFn fn = *reinterpret_cast<VtableFn *>(param_2 + 0x24);
        fn(*reinterpret_cast<int *>(param_2 + 0x28), param_1[3]);
    }
    if (param_1[0] == 6) {
        call_644910(param_1[3], param_2);
        call_644100(param_1[2], param_2);
        call_644100(param_1[1], param_2);
    }
    param_1[0] = 0;
    param_1[0xc] = param_1[9];
    param_1[0xb] = param_1[9];
    param_1[7] = 0;
    param_1[8] = 0;
    if (param_1[0xd] != 0) {
        typedef int (__cdecl *Fn3)(int, int, int);
        Fn3 fn2 = reinterpret_cast<Fn3>(param_1[0xd]);
        int result = fn2(0, 0, 0);
        param_1[0xe] = result;
        *reinterpret_cast<int *>(param_2 + 0x30) = result;
    }
}
