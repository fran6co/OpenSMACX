// ORIGINAL: 0x0052E900 FILE
// name      ?send_game@NetDaemon@@QAEXXZ
// size      1075 bytes
// spans     0x0052E900-0x0052ED33
// prototype void (__thiscall ?send_game@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00 0x00534400 0x00630A00
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E900
// measured tier  NO_COMPILE
// refusal        u0052e900.cpp(73) : error C2664: 'synch' : cannot convert parameter 1 from 'const int' to 'struct int16' Source or target has incomplete type u0052e900.cpp(75) 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e900/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_game@NetDaemon@@QAEXXZ  at 0x0052E900  (1075 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
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

struct MessageFactionData;
struct id;
struct int16;
struct int8;
struct uDelay;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Net { public:
    int get(unsigned int*, unsigned int*);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_009469f8 = (int *)0x009469F8;
static int *const g_009a64d4 = (int *)0x009A64D4;

class NetDaemon { public:
    void synch(int16, int id, int, int, int8*, int uDelay, int16);
    void process_message(char*, unsigned long, int);
    void send_game();
};

void NetDaemon::send_game() {
    unsigned long a, b;
    unsigned long c, d;
    unsigned long e, f;
    unsigned long id;

    synch(0, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(2, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(4, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(5, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    int i = 0;
    int off = 0;
    do {
        synch(6, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = ((Net *)g_0093cd90)->get(&c, &d);
            if (id == 0) break;
            process_message((char *)id, d, c);
        }

        synch(7, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = ((Net *)g_0093cd90)->get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        synch(0xf, off, 0x40, 0, 0, 1, 0x2101);
        for (;;) {
            id = ((Net *)g_0093cd90)->get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        off += 0x40;
        i++;
    } while (off < 0x200);

    synch(0x29, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x12, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x14, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x19, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (*g_009a64d4 != 0) {
        synch(0x20, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = ((Net *)g_0093cd90)->get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x21, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (*g_009469f8 != 0) {
        synch(0x22, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = ((Net *)g_0093cd90)->get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x24, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x15, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = ((Net *)g_0093cd90)->get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }
}
