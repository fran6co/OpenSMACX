// ORIGINAL: 0x0065C001 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0065C001
// name           sub_65c001
// size           8 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0065c001/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_65c001  at 0x0065C001  (8 bytes)
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
// BaseButton's non-virtual destructor is the tail-jmp target. This 8-byte
// body is an MSVC C++ EH unwind funclet (AGENTS.md's "SEH frames" section;
// docs/BYTE_MATCH_ROUTE.md's "/GX is required, and free" section): it shares
// the ENCLOSING function's frame - no push ebp/mov ebp,esp here, and no
// epilogue - and the local it destroys sits at ebp-0x14 in that borrowed
// frame. A standalone top-level definition cannot reproduce "no prologue,
// ebp-relative access, no epilogue" together: VC6 always frames a named
// external symbol that touches ebp-relative storage. Ruled out: a genuine
// local `BaseButton*` forces a full prologue/epilogue (call, not jmp,
// instruction #0 `push` not `mov`); only a compile-time-constant `this`
// (p = 0) reaches a tail jmp, and then instruction #0 is `xor ecx,ecx`
// (33 C9) instead of `mov ecx,[ebp-0x14]` (8B 4D EC) - divergence at
// instruction index 0, verified MISMATCH, 7/8 bytes, mnemonics 1/2 in common.
class BaseButton {
public:
    ~BaseButton();
};

extern "C" void __cdecl sub_65c001() {
    BaseButton *p = 0;
    p->~BaseButton();
}
