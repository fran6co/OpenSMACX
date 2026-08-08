// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00447DD0
// name           ?general@AAmbience@@QAEXXZ
// size           1187 bytes
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447dd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@AAmbience@@QAEXXZ  at 0x00447DD0  (1187 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00683c30 = (int *)0x00683C30;
static int *const g_00683c34 = (int *)0x00683C34;
static int *const g_00683c38 = (int *)0x00683C38;
static int *const g_00683c3c = (int *)0x00683C3C;
static int *const g_00683c40 = (int *)0x00683C40;
static int *const g_00683c44 = (int *)0x00683C44;
static int *const g_00683c48 = (int *)0x00683C48;
static int *const g_00683c4c = (int *)0x00683C4C;
static int *const g_00683c50 = (int *)0x00683C50;
static int *const g_0074dae8 = (int *)0x0074DAE8;
static int *const g_0074daf0 = (int *)0x0074DAF0;

class AAmbience { public:
    void general();
};

// ---- manual vtable dispatch (never a real C++ vtable) ----
// Every virtual call site in the original reloads the vtable pointer from
// [this] immediately before the call - never once cached and reused across a
// call - so these are macros expanded fresh at each call site rather than a
// helper function (a helper would also be inlined, and the original is not).
//
// VC6 RTM rejects an explicit `__thiscall` keyword on a FREE function
// pointer (error C4234: "'__thiscall' keyword reserved for future use" -
// that spelling was not accepted until a later service pack). A non-static,
// non-variadic member function pointer is thiscall on x86 MSVC without ever
// writing the keyword, so a dummy class with no bases and no virtuals
// (a "simple" class, whose member pointers MSVC represents as a bare code
// address - no vtable index, no this-adjustment) supplies the convention,
// and a union reinterprets the raw vtable-slot bytes as that member-pointer
// type. VThunk's methods are declared, never defined: only their TYPE is
// used, never their address or a call by name, so nothing needs them to
// exist at link time.
class VThunk { public:
    void c1(int);
    void c2(int, int);
    bool cb(int);
};
typedef void (VThunk::*VThunk_c1)(int);
typedef void (VThunk::*VThunk_c2)(int, int);
typedef bool (VThunk::*VThunk_cb)(int);
union Thunk1U { void *addr; VThunk_c1 fn; };
union Thunk2U { void *addr; VThunk_c2 fn; };
union ThunkBU { void *addr; VThunk_cb fn; };
typedef unsigned long (__stdcall *TimeGetTimeFn)();

#define VCALL1(off, a) (reinterpret_cast<VThunk*>(this)->*(reinterpret_cast<Thunk1U*>(*reinterpret_cast<uint8_t**>(this) + (off))->fn))(a)
#define VCALL2(off, a, b) (reinterpret_cast<VThunk*>(this)->*(reinterpret_cast<Thunk2U*>(*reinterpret_cast<uint8_t**>(this) + (off))->fn))(a, b)
#define VCALLB(off, a) (reinterpret_cast<VThunk*>(this)->*(reinterpret_cast<ThunkBU*>(*reinterpret_cast<uint8_t**>(this) + (off))->fn))(a)

void AAmbience::general() {
    int one = 1;
    int ten = 10;
    int counter = *g_00683c30 + 1;
    *g_00683c30 = counter;

    if (*g_00683c38 == one) {
        if (counter % 7 != 0) goto after_states;

        VCALL2(0x114, 0xe, *g_00683c3c);
        VCALL2(0x114, 0xf, *g_00683c3c);
        VCALL2(0x114, 0x10, *g_00683c3c);
        VCALL2(0x114, 0xd, *g_00683c3c);

        *g_00683c3c += *g_00683c40;
        VCALL1(0x10c, 0xe);
        VCALL1(0x10c, 2);
        VCALL1(0x10c, one);

        if (*g_00683c3c >= 300 || *g_00683c3c < -900) {
            *g_00683c40 = -*g_00683c40;
        }

        if (*g_00683c30 > ten) VCALL1(0x10c, 0xf);
        if (*g_00683c30 > 20) VCALL1(0x10c, 0x10);
        if (*g_00683c30 > 25) VCALL1(0x10c, 0xd);
    }

after_states:
    if (*g_00683c38 == 2) {
        VCALL1(0x10c, 0x1b);

        *g_00683c3c += *g_00683c40;
        if (*g_00683c3c >= 100 || *g_00683c3c < -600) {
            *g_00683c40 = -*g_00683c40;
        }

        *g_00683c44 += ten;
        *g_0074daf0 ^= one;
        if (*g_0074daf0) {
            VCALL1(0x10c, *g_00683c4c);
        } else {
            VCALL1(0x10c, *g_00683c48);
        }

        if (*g_00683c50 != 0x19) {
            VCALL1(0x10c, *g_00683c50);
        }

        int val48 = *g_00683c48 + 1;
        *g_00683c48 = val48;
        if (val48 > 0x14) *g_00683c48 = 0x12;

        int val4c = *g_00683c4c + 1;
        *g_00683c4c = val4c;
        if (val4c > 0x17) *g_00683c4c = 0x15;

        int val50 = *g_00683c50 + 1;
        *g_00683c50 = val50;
        if (val50 > 0x1a) *g_00683c50 = 0x18;
    }

    if (*g_00683c38 == 3) {
        int delta = *g_00683c40;
        int val = *g_0074dae8 + delta;
        *g_0074dae8 = val;
        if (val >= 100 || val < -600) {
            *g_00683c40 = -delta;
        }
        VCALL2(0x114, 8, val);
        VCALL2(0x114, 3, *g_0074dae8);
        VCALL2(0x114, 8, *g_0074dae8);
        VCALL2(0x114, 6, *g_0074dae8);
        VCALL2(0x114, 5, *g_0074dae8);
        VCALL2(0x114, 2, *g_0074dae8);
        VCALL2(0x114, 4, *g_0074dae8);

        if (*g_00683c30 == 5 || *g_00683c30 == 0xf) {
            VCALL1(0x10c, 5);
        }
    }

    if (*g_00683c34 > 5) {
        *g_00683c34 = one;
    } else {
        ++(*g_00683c34);
    }

    {
        unsigned long now = (*reinterpret_cast<TimeGetTimeFn*>(g_00669368))();
        int edi_val = now % *g_00683c34;
        if (edi_val == 3 || (edi_val == 8 && *g_0074dae8 >= 0)) {
            ++edi_val;
        }
        if (!VCALLB(0x118, edi_val)) {
            VCALL1(0x10c, edi_val);
        }
    }

    if (*g_00683c30 >= 0x28) {
        int phase = *g_00683c38;
        if (phase == one) {
            VCALL1(0x110, 0xd);
            VCALL1(0x110, 0xf);
            VCALL1(0x110, 0x10);
            *g_00683c38 = 2;
            VCALL2(0xb0, 0xe, 0);
            *g_00683c30 = 0;
            return;
        }
        if (phase == 2) {
            VCALL1(0x110, 0x12);
            VCALL1(0x110, 0x13);
            VCALL1(0x110, 0x14);
            VCALL1(0x110, 0x15);
            VCALL1(0x110, 0x16);
            VCALL1(0x110, 0x17);
            VCALL1(0x110, 0x18);
            VCALL1(0x110, 0x19);
            VCALL1(0x110, 0x1a);
            VCALL1(0x110, 0x1b);
            VCALL2(0xb0, 0xe, one);
            VCALL1(0x10c, 3);
            *g_00683c38 = 3;
            *g_00683c30 = 0;
            return;
        }
        if (phase == 3) {
            VCALL1(0x110, 1);
            VCALL1(0x110, 2);
            VCALL1(0x110, 3);
            VCALL1(0x110, 4);
            VCALL1(0x110, 5);
            VCALL1(0x110, 6);
            VCALL1(0x110, 7);
            VCALL1(0x110, 8);
            *g_00683c38 = one;
        }
        *g_00683c30 = 0;
    }
}
