// ORIGINAL: 0x005C1FC0 ?UNK2@Fractal@@QAEHHHH@Z 0x005C1FC0-0x005C2020 FILE
// RULED-OUT: two UNK1 calls on shifted operands, `(r1*5+r2*2)*7 >> 8` then clamp to [0,100], matching Ghidra; compiles and matches through both calls and the arithmetic, diverges at #14 (lea vs mov) in the final clamp/pop sequence
// working copy - scaffold materialised by --work
// size      96 bytes
// prototype int (__thiscall ?UNK2@Fractal@@QAEHHHH@Z)(Fractal* this, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1F20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@Fractal@@QAEHHHH@Z  at 0x005C1FC0  (96 bytes)
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

class Fractal;

// ---- callees, declared and never defined (a definition would be inlined) ----

class Fractal { public:
    int UNK1(int, int);
    int UNK2(int, int, int);
};
int Fractal::UNK2(int a1, int a2, int a3) {
    int r1 = UNK1(a1 << 4, a2 << 4);
    int r2 = UNK1((a1 << 4) << a3, (a2 << 4) << a3);
    int result = (r1 * 5 + r2 * 2) * 7 >> 8;
    if (result < 0) {
        return 0;
    }
    if (result > 0x64) {
        return 0x64;
    }
    return result;
}
