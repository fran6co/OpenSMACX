// ORIGINAL: 0x00642960 FILE
// RULED-OUT: the given nullary `int __cdecl sub_642960()` head - the body
//            reads a single stack arg at [esp+4] with no ebp frame (a
//            struct pointer) and never sets eax for a return; changed to
//            `void __cdecl sub_642960(void *a1)`.
//            MISMATCH #8 'push' vs 'mov' remains open.
// working copy - scaffold materialised by --work
// name      sub_642960
// size      139 bytes
// spans     0x00642960-0x006429EB
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_642960  at 0x00642960  (139 bytes)
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
extern "C" void __cdecl sub_642960(void *a1) {
    unsigned char *self = reinterpret_cast<unsigned char *>(a1);
    int *pRemain = reinterpret_cast<int *>(self + 0x16b4);
    int *pIndex = reinterpret_cast<int *>(self + 0x10);
    unsigned char **pBuf = reinterpret_cast<unsigned char **>(self + 8);
    unsigned char *pB0 = self + 0x16b0;
    unsigned char *pB1 = self + 0x16b1;

    if (*pRemain == 0x10) {
        (*pBuf)[*pIndex] = *pB0;
        int idx = *pIndex + 1;
        *pIndex = idx;
        (*pBuf)[idx] = *pB1;
        *pIndex = *pIndex + 1;
        *reinterpret_cast<unsigned short *>(self + 0x16b0) = 0;
        *pRemain = 0;
        return;
    }

    if (*pRemain > 7) {
        (*pBuf)[*pIndex] = *pB0;
        *pIndex = *pIndex + 1;
        *reinterpret_cast<unsigned short *>(self + 0x16b0) = (unsigned short)*pB1;
        *pRemain = *pRemain - 8;
    }
}
