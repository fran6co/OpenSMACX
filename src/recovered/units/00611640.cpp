// ORIGINAL: 0x00611640 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00611640
// name           ?pos_to_id@SpriteBox@@QAEXH@Z
// size           118 bytes
// measured tier  MISMATCH
// divergence     5
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00611640/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pos_to_id@SpriteBox@@QAEXH@Z  at 0x00611640  (118 bytes)
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

class SpriteBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int abs(int);

class SpriteBox { public:
    uint32_t field_0_;

    void pos_to_id(int);
};

extern "C" int __cdecl _abs(int);

// The scaffold declares pos_to_id returning void, but both exit paths set
// eax before `ret 4` (`mov eax,[eax+4]` / `xor eax,eax`) - it returns a
// value. The out-of-line definition of an already-declared member cannot
// change its return type, so this is a same-shaped shadow class (still a
// non-static member -> thiscall, matching `mov esi, ecx`) rather than a
// redeclaration of SpriteBox; the scaffold's own (void) member is left
// undefined. Calling it as `_abs` (matching the disassembly's real symbol)
// rather than the scaffold's plain `abs` also matters: `abs` gets treated
// as a pure intrinsic and the compiler CSEs away the second of the two
// identical `abs(a1)` calls the original genuinely makes twice.
class SpriteBoxR { public:
    int pos_to_id(int);
};

int SpriteBoxR::pos_to_id(int a1) {
    char *self = reinterpret_cast<char *>(this);

    if (a1 <= *reinterpret_cast<int *>(self + 0x34) - 1) {
        *reinterpret_cast<void **>(self + 0x30) = *reinterpret_cast<void **>(self + 0x2c);

        if (a1 < 0) {
            int n = _abs(a1);
            if (n <= *reinterpret_cast<int *>(self + 0x34)) {
                n = _abs(a1);
                if (n > 0) {
                    do {
                        --n;
                        *reinterpret_cast<void **>(self + 0x30) = *reinterpret_cast<void **>(
                            *reinterpret_cast<char **>(self + 0x30) + 0x10);
                    } while (n != 0);
                }
                *reinterpret_cast<int *>(self + 0x38) = *reinterpret_cast<int *>(self + 0x34) + a1;
            }
        } else {
            int n = a1;
            if (n > 0) {
                do {
                    --n;
                    *reinterpret_cast<void **>(self + 0x30) = *reinterpret_cast<void **>(
                        *reinterpret_cast<char **>(self + 0x30) + 0xc);
                } while (n != 0);
            }
            *reinterpret_cast<int *>(self + 0x38) = a1;
        }
    }

    if (*reinterpret_cast<int *>(self + 0x2c) != 0) {
        return *reinterpret_cast<int *>(*reinterpret_cast<char **>(self + 0x30) + 4);
    }
    return 0;
}
