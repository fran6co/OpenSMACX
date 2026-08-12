// ORIGINAL: 0x0051F1B0 BYTE_EXACT FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0051F1B0
// name           ?check_spock@@YAXXZ
// size           66 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0051f1b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?check_spock@@YAXXZ  at 0x0051F1B0  (66 bytes)

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
typedef char int8;
typedef unsigned char uint8;

class Lock;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Lock { public:
    int any_locks();
};
void __cdecl help_spock();

// ---- fixed globals this body references ----
static int *const g_0093a934 = (int *)0x0093A934;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093e230 = (int *)0x0093E230;
static int *const g_0093e8c0 = (int *)0x0093E8C0;
static int *const g_0093f670 = (int *)0x0093F670;
static int *const g_0093f674 = (int *)0x0093F674;

void __cdecl check_spock() {
    if (*g_0093e8c0 != 0 && *g_0093f670 != 0 && *g_0093a934 == 0 && *g_0093a938 == 0) {
        *g_0093f674 = 1;
        if (reinterpret_cast<Lock *>(g_0093e230)->any_locks() == 0) {
            help_spock();
        }
    }
}
