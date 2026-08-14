// ORIGINAL: 0x005CD980 FILE
// working copy - scaffold materialised by --work
// name      sub_5cd980
// size      172 bytes
// spans     0x005CD980-0x005CDA2C
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D1654 0x005D1A30

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cd980  at 0x005CD980  (172 bytes)
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
extern "C" int __cdecl sub_5d1654();
extern "C" int __cdecl sub_5d1a30();
// IDA guessed __stdcall(int,int,int); the disassembly reads a2 straight out
// of edx with no matching stack load, which is the __fastcall ABI (ecx,edx,
// then stack) - and `ret 0xc` pops exactly the 3 remaining stack ints.
// The scaffold's own callee declarations are nullary (redeclaring is
// C2733), so the real 3-int signature is declared under a different name.
extern "C" int __fastcall sub_5d1654_args(int, unsigned int, int *);
extern "C" int __fastcall sub_5d1a30_args(int, unsigned int, int *);

extern "C" void __fastcall sub_5cd980(int *a1, int a2, int a3, int a4,
                                       unsigned int a5) {
    int base;
    unsigned int outer;
    int inner;
    int cursor;

    base = *a1;
    a1[0x140] = 0;
    a1[0x13f] = 0;
    a1[0x13e] = 0;
    a1[0x17e] = a2;
    a1[0x17f] = 0;
    outer = 0;
    if (*(int *)(base + 0x60) != 0) {
        do {
            inner = 0;
            cursor = a3;
            if (*(int *)(base + 100) != 0) {
                do {
                    if (a1[0x10] == 0) {
                        sub_5d1a30_args(cursor, a5, a1 + 0xfe);
                    } else {
                        sub_5d1654_args(cursor, a5, a1 + 0xfe);
                    }
                    inner += 0x10;
                    cursor += 0x40;
                } while ((unsigned int)inner < *(unsigned int *)(*a1 + 100));
            }
            a3 = a3 + (int)((a5 >> 2) * 0x40);
            base = *a1;
            outer += 0x10;
        } while (outer < *(unsigned int *)(base + 0x60));
    }
}
