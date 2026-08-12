// ORIGINAL: 0x004482C0 BYTE_EXACT FILE
// name      ??1Effect@@QAE@XZ
// size      5 bytes
// spans     0x004482C0-0x004482C5
// prototype void (__thiscall ??1Effect@@QAE@XZ)(Effect* this)
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004482C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004482c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1Effect@@QAE@XZ  at 0x004482C0  (5 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

class Wave { public:
    ~Wave();
};

class Effect : public Wave { public:
    ~Effect();
};

Effect::~Effect() {
    // The original is a 5-byte jmp straight into ??1Wave@@QAE@XZ: Effect
    // derives from Wave, adds nothing, and has nothing else to tear down, so
    // the only work here is the (tail-called) base destructor.
}
