// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005D271D
// name           sub_5d271d
// size           68 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d271d/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d271d  at 0x005D271D  (68 bytes)
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

// Each source dword is 4 bytes b0..b3 (little-endian); each byte is
// written out twice adjacently to the destination (2x horizontal pixel
// doubling: b0,b0,b1,b1,b2,b2,b3,b3), 4 source bytes producing 8
// destination bytes per iteration.
//
// The original assembles each destination dword in a register via
// 8/16-bit sub-register moves (mov bl,ah / mov bh,ah / shl ebx,0x10 /
// mov bl,al / mov bh,al) before a single 32-bit store. Every arithmetic
// rendering tried here (shift-or splat, multiply-by-0x101 splat) makes
// VC6 emit `and`/`imul`/extra `shl`+`or` chains instead of reproducing
// that register shuffle, so this settles for the byte-store form below,
// which is shorter and closer in instruction count than the arithmetic
// attempts (26 mnemonics / 0.73 similarity, vs 34-43 / 0.58-0.6 for the
// splat versions) even though it stores 8 bytes individually rather
// than 2 dwords.
extern "C" void __cdecl sub_5d271d(unsigned int *src, unsigned char *dst,
                                    int count) {
    do {
        unsigned int word = *src;
        ++src;
        unsigned char b0 = static_cast<unsigned char>(word);
        unsigned char b1 = static_cast<unsigned char>(word >> 8);
        dst[0] = b0; dst[1] = b0; dst[2] = b1; dst[3] = b1;
        word >>= 16;
        unsigned char b2 = static_cast<unsigned char>(word);
        unsigned char b3 = static_cast<unsigned char>(word >> 8);
        dst[4] = b2; dst[5] = b2; dst[6] = b3; dst[7] = b3;
        dst += 8;
        count -= 4;
    } while (count != 0);
}

