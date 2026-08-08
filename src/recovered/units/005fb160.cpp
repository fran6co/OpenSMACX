// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FB160
// name           ?add_separator@Menu@@QAEHHH@Z
// size           86 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fb160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_separator@Menu@@QAEHHH@Z  at 0x005FB160  (86 bytes)
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

class Menu;
class PullDown;

// ---- callees, declared and never defined (a definition would be inlined) ----
class PullDown { public:
    int add_separator(int);
};

class Menu { public:
    int add_separator(int, int);
};

// CLOSEST reached: 0.84 mnemonic similarity, MISMATCH. Two residual
// divergences ruled out as backend choices rather than source-form bugs:
//  * two dependency-free prologue instructions (`xor eax,eax` /
//    `lea esi,[ecx+0xa38]`) issue in the opposite order from the original,
//    and likewise the loop-tail `inc eax` / `add esi,0x14` pair - an
//    INSTRUCTION SCHEDULING wall, not a source-shape problem: every
//    spelling tried (for/while/do-while, goto/break) reproduces it.
//  * the dead `if (i == -1) return 0xb;` guard (present verbatim in the
//    Ghidra decompilation too, and provably unreachable given the loop
//    bounds) gets branch-threaded by this backend straight to the
//    loop-exhaustion `return 0xb;` block instead of emitting its own
//    identical copy the way the original does - i.e. the ORIGINAL declines
//    an optimization this compiler takes. Not reproducible from source form
//    without duplicating the tail by hand, which would just be retyping
//    the target's bytes.
int Menu::add_separator(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int i;
    int *rec = reinterpret_cast<int *>(self + 0xa38);
    for (i = 0; i < 15; i++) {
        if (*rec == -1) {
            return 0xb;
        }
        if (*rec == a1) {
            goto matched;
        }
        rec = reinterpret_cast<int *>(reinterpret_cast<char *>(rec) + 0x14);
    }
    return 0xb;
matched:
    if (i == -1) {
        return 0xb;
    }
    (*reinterpret_cast<PullDown **>(self + 0xa48 + i * 0x14))->add_separator(a2);
    return 0;
}
