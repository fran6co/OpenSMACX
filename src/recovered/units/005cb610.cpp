// ORIGINAL: 0x005CB610 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005CB610
// name           sub_5cb610
// size           66 bytes
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005cb610/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cb610  at 0x005CB610  (66 bytes)
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

extern "C" int __cdecl sub_5cb610(int *p) {
    // Frameless (/Oy) is the flag set the harness selects as best; the
    // /Oy- variant keeps ebp and never gets closer. The remaining
    // divergence is a scheduler swap: this compile always orders
    // `mov edx,[ecx+0x98]` before `xor eax,eax` here, while the original
    // has `xor eax,eax` first - both are independent of each other and
    // every rephrasing of `p[0x26] != 6` (direct, through a named local,
    // negated via `!(...==6)`) reproduces the same swapped order, so it
    // is the backend's instruction scheduler, not a fixable source form.
    if (*p != 0) {
        if (*reinterpret_cast<int *>(*p + 0x3d0) != 0) {
            return p[0x26] != 6;
        }
        if (p[0x26] == 2 || p[0x26] == 3) {
            return 1;
        }
    }
    return 0;
}

