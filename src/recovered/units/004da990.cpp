// ORIGINAL: 0x004DA990 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004DA990
// name           ?read_check@DipEdit@@QAEXXZ
// size           1035 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004da990/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?read_check@DipEdit@@QAEXXZ  at 0x004DA990  (1035 bytes)
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
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

// ---- callees, declared and never defined (a definition would be inlined) ----
class CheckBox { public:
    int get_state_id(int);
};
void __cdecl set_agenda(int factionID, int factionIDWith, int agenda, int set);
void __cdecl set_treaty(int, int, int, int);
void __cdecl treaty_off(int, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096ca18 = (int *)0x0096CA18;

class DipEdit { public:
    void UNK2(int);
    void UNK3(int);
    void UNK4(int);
    void read_check();
};

void DipEdit::read_check() {
    char *self = reinterpret_cast<char *>(this);
    int a, b, state;

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(8);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 8)) {
            treaty_on(b, a, 8);
        }
    } else if (reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 8) {
        treaty_off(b, a, 8);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(0x10);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 0x10)) {
            treaty_on(b, a, 0x10);
        }
    } else if (reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 0x10) {
        treaty_off(b, a, 0x10);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(4);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 4)) {
            treaty_on(b, a, 4);
        }
    } else if (reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 4) {
        treaty_off(b, a, 4);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(2);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 2)) {
            treaty_on(b, a, 2);
        }
    } else if (reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 2) {
        treaty_off(b, a, 2);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(1);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 1)) {
            treaty_on(b, a, 1);
        }
    } else if (reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 1) {
        treaty_off(b, a, 1);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(0x1000);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (a + b) * 4 + b * 0x20c8) & 0x1000)) {
            set_treaty(b, a, 0x1000, 1);
        }
    } else if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (a + b) * 4 + b * 0x20c8) & 0x1000) {
        set_treaty(b, a, 0x1000, 0);
    }

    state = reinterpret_cast<CheckBox *>(self + 0x15a0)->get_state_id(0x2000);
    a = *reinterpret_cast<int *>(self + 0xa20);
    b = *reinterpret_cast<int *>(self + 0xa1c);
    if (state != 0) {
        if (!(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (a + b) * 4 + b * 0x20c8) & 0x2000)) {
            set_treaty(b, a, 0x2000, 1);
        }
    } else if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (a + b) * 4 + b * 0x20c8) & 0x2000) {
        set_treaty(b, a, 0x2000, 0);
    }

    UNK3(0x20);
    UNK3(0x2000000);
    UNK3(0x400);
    UNK3(0x40000);
    UNK4(8);
    UNK2(0x2000);

    b = *reinterpret_cast<int *>(self + 0xa1c);
    a = *reinterpret_cast<int *>(self + 0xa20);
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9f8) + (a + b) * 4 + b * 0x20c8) & 0x2000000) {
        treaty_on(b, a, 1);
        a = *reinterpret_cast<int *>(self + 0xa20);
        b = *reinterpret_cast<int *>(self + 0xa1c);
        set_treaty(b, a, 0xc20, 0);
        a = *reinterpret_cast<int *>(self + 0xa20);
        b = *reinterpret_cast<int *>(self + 0xa1c);
        set_agenda(b, a, 8, 0);
    }

    b = *reinterpret_cast<int *>(self + 0xa1c);
    a = *reinterpret_cast<int *>(self + 0xa20);
    if (reinterpret_cast<unsigned char *>(g_0096ca18)[(a + b) * 4 + b * 0x20c8] & 8) {
        treaty_on(b, a, 0x10);
    }

    b = *reinterpret_cast<int *>(self + 0xa1c);
    a = *reinterpret_cast<int *>(self + 0xa20);
    if (!(reinterpret_cast<unsigned char *>(g_0096c9f8)[(a + b) * 4 + b * 0x20c8] & 0x20)) {
        set_agenda(b, a, 0xc00, 0);
    }
}
