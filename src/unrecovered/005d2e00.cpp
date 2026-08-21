// ORIGINAL: 0x005D2E00 sub_5d2e00 0x005D2E00-0x005D2E6E FILE BYTE_EXACT
// symbol    ?sub_5d2e00@@YIXHHHHD@Z
// working copy - scaffold materialised by --work
// size      110 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2e00  at 0x005D2E00  (110 bytes)
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
// `lea eax, [ecx+edx]` with no stack access for the first two operands is
// __fastcall (ecx=a1, edx=a2); three more come off the stack, the last a
// single byte - matches `ret 0xc` (3 stack slots popped by the callee).
void __fastcall sub_5d2e00(int a1, int a2, int a3, int a4, char a5) {
    char *src = reinterpret_cast<char *>(a1 + a2);
    int stride = a3 * 8;
    char *dst = reinterpret_cast<char *>(a4 + 6);
    int count = 8;
    do {
        dst[-4] = src[0] + a5;
        dst[0] = src[4] + a5;
        dst[4] = src[8] + a5;
        dst[8] = src[0xc] + a5;
        dst[0xc] = src[0x10] + a5;
        dst[0x10] = src[0x14] + a5;
        dst[0x14] = src[0x18] + a5;
        dst[0x18] = src[0x1c] + a5;
        src = src + stride;
        count = count - 1;
        dst = dst + 0x20;
    } while (count != 0);
}
