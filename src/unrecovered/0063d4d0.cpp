// ORIGINAL: 0x0063D4D0 _deflateEnd 0x0063D4D0-0x0063D55F FILE BYTE_EXACT
// CORRECTED from sub_63d4d0
//   zlib 1.0.2 deflate.c, 143 bytes, byte-exact from upstream
// size      143 bytes
// prototype 
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x0063D4EC 0x0063D501 0x0063D516 0x0063D52B 0x0063D53D
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63d4d0  at 0x0063D4D0  (143 bytes)
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
extern "C" int __cdecl sub_63d4d0(int param_1) {
    typedef int (__cdecl *Fn)(int, int);

    if (param_1 != 0 && *reinterpret_cast<int *>(param_1 + 0x1c) != 0) {
        int v;
        v = *reinterpret_cast<int *>(*reinterpret_cast<int *>(param_1 + 0x1c) + 8);
        if (v != 0) {
            (*reinterpret_cast<Fn *>(param_1 + 0x24))(
                *reinterpret_cast<int *>(param_1 + 0x28), v);
        }
        v = *reinterpret_cast<int *>(*reinterpret_cast<int *>(param_1 + 0x1c) + 0x38);
        if (v != 0) {
            (*reinterpret_cast<Fn *>(param_1 + 0x24))(
                *reinterpret_cast<int *>(param_1 + 0x28), v);
        }
        v = *reinterpret_cast<int *>(*reinterpret_cast<int *>(param_1 + 0x1c) + 0x34);
        if (v != 0) {
            (*reinterpret_cast<Fn *>(param_1 + 0x24))(
                *reinterpret_cast<int *>(param_1 + 0x28), v);
        }
        v = *reinterpret_cast<int *>(*reinterpret_cast<int *>(param_1 + 0x1c) + 0x2c);
        if (v != 0) {
            (*reinterpret_cast<Fn *>(param_1 + 0x24))(
                *reinterpret_cast<int *>(param_1 + 0x28), v);
        }
        v = *reinterpret_cast<int *>(*reinterpret_cast<int *>(param_1 + 0x1c) + 4);
        (*reinterpret_cast<Fn *>(param_1 + 0x24))(
            *reinterpret_cast<int *>(param_1 + 0x28),
            *reinterpret_cast<int *>(param_1 + 0x1c));
        *reinterpret_cast<int *>(param_1 + 0x1c) = 0;
        return ((v != 0x71) - 1) & 0xfffffffd;
    }
    return -2;
}
