// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0065BFBC
// name           sub_65bfbc
// size           11 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0065bfbc/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_65bfbc  at 0x0065BFBC  (11 bytes)
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

// BEST EFFORT, NOT BYTE_EXACT - see the agent report for this address.
// Same shape as 0x0065C001 (an MSVC C++ EH unwind funclet: `lea ecx,
// [ebp-0x5398]; jmp Popup::~Popup`, no prologue, no epilogue, borrowing the
// enclosing function's frame - the -0x5398 offset alone rules out this being
// its own small function). Ruled out the same way: a real `Popup*` local
// forces a full prologue/epilogue; a compile-time-constant `this` reaches a
// tail jmp but instruction #0 is `xor ecx,ecx` (33 C9) instead of `lea ecx,
// [ebp-0x5398]` (8D 8D 68 AC FF FF) - divergence at instruction index 0,
// verified MISMATCH, 7/11 bytes, mnemonics 1/2 in common.
class Popup {
public:
    ~Popup();
};

extern "C" void __cdecl sub_65bfbc() {
    Popup *p = 0;
    p->~Popup();
}
