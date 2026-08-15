// ORIGINAL: 0x00640BB0 BYTE_EXACT FILE
// LEVER: no explicit `eax` set before `ret` -> `void`, not `int`; the
//        contract's `int` plus a `return 0` adds a spurious `xor eax,eax`
// working copy - scaffold materialised by --work
// name      __tr_init
// CORRECTED from sub_640bb0
//   zlib 1.0.2 trees.c, 114 bytes, byte-exact from upstream
// size      114 bytes
// spans     0x00640BB0-0x00640C22
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640C30 0x00640E50

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_640bb0  at 0x00640BB0  (114 bytes)
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
extern "C" int __cdecl sub_640c30();
extern "C" int __cdecl sub_640e50();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00698ff8 = (int *)0x00698FF8;
static int *const g_00699010 = (int *)0x00699010;
static int *const g_00699028 = (int *)0x00699028;
// `mov eax, [esp+4]` reads the caller's stack arg (after a `call` that
// nets to zero stack movement) -> a plain __cdecl parameter.
extern "C" void __cdecl sub_640bb0(int a1) {
    sub_640c30();
    char *self = reinterpret_cast<char *>(a1);
    *reinterpret_cast<int *>(self + 0x16a4) = 0;
    *reinterpret_cast<int *>(self + 0xb0c) = a1 + 0x88;
    *reinterpret_cast<int *>(self + 0xb18) = a1 + 0x97c;
    *reinterpret_cast<int *>(self + 0xb14) = reinterpret_cast<int>(g_00698ff8);
    *reinterpret_cast<int *>(self + 0xb20) = reinterpret_cast<int>(g_00699010);
    *reinterpret_cast<int *>(self + 0xb24) = a1 + 0xa70;
    *reinterpret_cast<int *>(self + 0xb2c) = reinterpret_cast<int>(g_00699028);
    *reinterpret_cast<short *>(self + 0x16b0) = 0;
    *reinterpret_cast<int *>(self + 0x16b4) = 0;
    *reinterpret_cast<int *>(self + 0x16ac) = 8;
    typedef int (__cdecl *Fn1)(int);
    reinterpret_cast<Fn1>(sub_640e50)(a1);
}
